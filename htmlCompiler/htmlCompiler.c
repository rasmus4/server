// HTML compiler that allows includes and outputs the result as a C array.
// Includes are done with <!--INCLUDE(src/main.js)--> or with /*INCLUDE(src/main.js)*/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

static char *buffer = NULL;
static int bufferLength = 0;

static int replaceWithFile(int replaceIndex, int replaceLength, char *fileName, int fileNameLength) {
    int status;
    char *fileNameZ = malloc(fileNameLength + 1);
    if (!fileNameZ) {
        status = -1;
        goto cleanup_none;
    }
    memcpy(fileNameZ, fileName, fileNameLength);
    fileNameZ[fileNameLength] = '\0';

    FILE *handle = fopen(fileNameZ, "r");
    if (!handle) {
        status = -2;
        goto cleanup_fileNameZ;
    }

    if (fseek(handle, 0, SEEK_END) != 0) {
        status = -3;
        goto cleanup_handle;
    }
    int length = ftell(handle);
    if (length < 0) {
        status = -4;
        goto cleanup_handle;
    }
    rewind(handle);

    int newBufferLength = bufferLength + (length - replaceLength);
    if (newBufferLength > bufferLength) {
        char *newBuffer = realloc(buffer, newBufferLength + 1);
        if (!newBuffer) {
            status = -5;
            goto cleanup_handle;
        }
        buffer = newBuffer;
    }
    memmove(&buffer[replaceIndex + length], &buffer[replaceIndex + replaceLength], bufferLength - (replaceLength + replaceIndex));
    buffer[newBufferLength] = '\0';
    bufferLength = newBufferLength;
    int readSize = fread(&buffer[replaceIndex], 1, length, handle);
    if (readSize != length) {
        status = -6;
        goto cleanup_handle;
    }
    status = 0;
    cleanup_handle:
    fclose(handle);
    cleanup_fileNameZ:
    free(fileNameZ);
    cleanup_none:
    return status;
}

static int writeToFile(char *fileName, char *content, int32_t contentLength) {
    int status;
    FILE *handle = fopen(fileName, "w");
    if (!handle) {
        status = -1;
        goto cleanup_none;
    }
    if (fwrite(content, 1, contentLength, handle) != (size_t)contentLength) {
        status = -2;
        goto cleanup_handle;
    };
    status = 0;
    cleanup_handle:
    fclose(handle);
    cleanup_none:
    return status;
}

static int writeHeaderOutput(char *fileName, char *arrayName) {
    int status;
    char start[] = "#pragma once\n#include <stdint.h>\nstatic uint8_t ";
    char afterName[] = "[] = {";
    char betweenBytes[] = ",";
    char end[] = "};\n";

    int32_t maxLength = (
        (sizeof(start) - 1) +
        strlen(arrayName) +
        (sizeof(afterName) - 1) +
        (3 + (sizeof(betweenBytes) - 1)) * (bufferLength - 1) +
        (sizeof(end) - 1) +
        1
    );

    char *outBuffer = malloc(maxLength);
    if (!outBuffer) {
        status = -1;
        goto cleanup_none;
    }
    outBuffer[0] = '\0';
    strcat(outBuffer, start);
    strcat(outBuffer, arrayName);
    strcat(outBuffer, afterName);

    for (int32_t i = 0; i < bufferLength; ++i) {
        sprintf(&outBuffer[strlen(outBuffer)], "%" PRIu8, (uint8_t)buffer[i]);
        if (i != bufferLength - 1) strcat(outBuffer, betweenBytes);
    }
    strcat(outBuffer, end);

    status = writeToFile(fileName, outBuffer, strlen(outBuffer));
    if (status < 0) {
        status = -2;
        goto cleanup_outBuffer;
    }

    status = 0;
    cleanup_outBuffer:
    free(outBuffer);
    cleanup_none:
    return status;
}

static int handleInclude(char *includeStartPattern, char *includeEndPattern) {
    int status;
    char *includeStart = strstr(buffer, includeStartPattern);
    if (!includeStart) {
        status = 1;
        goto cleanup_none;
    }
    char *includeEnd = strstr(includeStart, includeEndPattern);
    if (!includeEnd) {
        printf("Error: Unclosed include\n");
        status = -1;
        goto cleanup_none;
    }
    char *name = &includeStart[strlen(includeStartPattern)];
    int32_t nameLength = (int32_t)(includeEnd - name);
    status = replaceWithFile((int32_t)(includeStart - buffer), (int32_t)(includeEnd + strlen(includeEndPattern) - includeStart), name, nameLength);
    if (status < 0) {
        printf("Error: Failed to include file %.*s (%d)\n", nameLength, name, status);
        status = -2;
        goto cleanup_none;
    }
    status = 0;
    cleanup_none:
    return status;
}

int main(int argc, char **argv) {
    int status;
    if (argc != 3) {
        printf("Usage: %s infile.html outname\n", argv[0]);
        status = 1;
        goto cleanup_none;
    }
    status = replaceWithFile(0, 0, argv[1], strlen(argv[1]));
    if (status < 0) {
        printf("Error: Failed to load initial file %s (%d)\n", argv[1], status);
        status = 1;
        goto cleanup_none;
    }

    int complete = 0;
    while (!complete) {
        complete = 1;
        status = handleInclude("<!--INCLUDE(", ")-->");
        if (status < 0) {
            printf("Error: Failed to handle Html include (%d)\n", status);
            status = 1;
            goto cleanup_buffer;
        }
        complete &= status;
        status = handleInclude("/*INCLUDE(", ")*/");
        if (status < 0) {
            printf("Error: Failed to handle Javascript/Css include (%d)\n", status);
            status = 1;
            goto cleanup_buffer;
        }
        complete &= status;
    }

    int32_t outNameLength = strlen(argv[2]);
    char *outName = malloc(outNameLength + 6); // 6 enough for ".html" and ".h".
    if (!outName) {
        printf("Error: Failed to allocate memory\n");
        status = 1;
        goto cleanup_buffer;
    }
    memcpy(outName, argv[2], outNameLength);
    memcpy(&outName[outNameLength], ".html", 6);
    status = writeToFile(outName, buffer, bufferLength);
    if (status < 0) {
        printf("Error: Failed to write html output (%d)\n", status);
        status = 1;
        goto cleanup_outName;
    }

    memcpy(&outName[outNameLength], ".h", 3);
    status = writeHeaderOutput(outName, argv[2]);
    if (status < 0) {
        printf("Error: Failed to write header output (%d)\n", status);
        status = 1;
        goto cleanup_outName;
    }
    status = 0;
    cleanup_outName:
    free(outName);
    cleanup_buffer:
    free(buffer);
    cleanup_none:
    return status;
}
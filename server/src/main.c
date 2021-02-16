#include "server.h"
#include "fileResponse.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "generatedHtml.h"

struct server server;

int onConnect(void *data, struct server_client *client) {
    printf("onConnect\n");
    uint32_t version = 1;
    if (server_sendWebsocketMessage(&server, client, (uint8_t *)&version, 4, false) < 0) return -1;
    uint8_t view = 3;
    if (server_sendWebsocketMessage(&server, client, &view, 1, false) < 0) return -1;
    return 0;
}

int onDisconnect(void *data, struct server_client *client) {
    printf("onDisconnect\n");
    return 0;
}

int onMessage(void *data, struct server_client *client, uint8_t *message, int32_t messageLength, bool isText) {
    printf("Got websocket packet!! %.*s\n", (int)messageLength, message);
    //server_sendWebsocketMessage(&server, client, message, messageLength, isText);
    return 0;
}

int main_createFileResponse(struct fileResponse *response) {
    int status;

    int32_t digits = 1;
    int32_t magnitude = 10;
    while (sizeof(generatedHtml) >= magnitude) {
        ++digits;
        magnitude *= 10;
    }
    char responseHttpStart[] = "HTTP/1.1 200 OK\r\nContent-Length:";
    char responseHttpEnd[] = "\r\n\r\n";
    int32_t responseLength = (sizeof(responseHttpStart) - 1) + digits + (sizeof(responseHttpEnd) - 1) + sizeof(generatedHtml);

    uint8_t *responseBuffer = malloc(responseLength);
    if (!responseBuffer) {
        status = -1;
        goto cleanup_none;
    }

    memcpy(responseBuffer, responseHttpStart, sizeof(responseHttpStart) - 1);

    uint8_t *responsePos = &responseBuffer[sizeof(responseHttpStart) - 1];
    int32_t remainingLength = sizeof(generatedHtml);
    while (magnitude >= 10) {
        magnitude /= 10;
        int32_t digitValue = remainingLength / magnitude;
        remainingLength -= digitValue * magnitude;
        *responsePos = '0' + digitValue;
        ++responsePos;
    }

    memcpy(responsePos, responseHttpEnd, sizeof(responseHttpEnd) - 1);
    responsePos += (sizeof(responseHttpEnd) - 1);

    memcpy(responsePos, generatedHtml, sizeof(generatedHtml));

    fileResponse_init(
        response,
        (uint8_t *)"",
        0,
        responseBuffer,
        responseLength
    );
    status = 0;
    cleanup_none:
    return status;
}

int main(int argc, char **argv) {
    int status;
    struct fileResponse response;
    status = main_createFileResponse(&response);
    if (status < 0) {
        printf("Failed to create html response (%d)\n", status);
        status = 1;
        goto cleanup_none;
    }

    struct server_callbacks callbacks;
    server_callbacks_init(
        &callbacks,
        NULL,
        onConnect,
        onDisconnect,
        onMessage
    );

    status = server_init(&server, &response, 1, callbacks);
    if (status < 0) {
        printf("Server initialization failed (%d)\n", status);
        status = 1;
        goto cleanup_callbacks;
    }
    printf("Server initialized!\n");

    status = server_run(&server);
    if (status < 0) {
        printf("Server ran into error (%d)\n", status);
        status = 1;
        goto cleanup_server;
    }
    status = 0;
    cleanup_server:
    server_deinit(&server);
    cleanup_callbacks:
    server_callbacks_deinit(&callbacks);
    //cleanup_response:
    free(response.response);
    fileResponse_deinit(&response);
    cleanup_none:
    return status;
}
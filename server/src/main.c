#include "server.h"
#include "fileResponse.h"

// TODO rewrite this
#define MAX_RESPONSES 2048
#define MAX_RESPONSE_PATH_LENGTH 128
struct fileResponse responses[MAX_RESPONSES];
int32_t responsesLength;

int load_responses(void) {
    uint8_t file_path[MAX_RESPONSE_PATH_LENGTH] = "./public/";
    uint8_t *file_name_start = &file_path[9];
    DIR *response_directory = opendir((char *)file_path);
    if (response_directory == NULL) {
        printf("Error opening directory \"./responses\"\n");
        return -1;
    }

    struct dirent* entry;
    responsesLength = 0;
    for (;;) {
        entry = readdir(response_directory);
        if (entry == NULL) {
            break;
        }
        if (entry->d_type == DT_DIR) {
            continue;
        }
        if (responsesLength == MAX_RESPONSES) {
            printf("Reached limit of %d responses\n", MAX_RESPONSES);
            break;
        }
        uint8_t *path_pos = file_name_start;
        uint8_t *file_name_pos = (uint8_t *)entry->d_name;
        while (*file_name_pos != 0) {
            *path_pos = *file_name_pos;
            ++path_pos;
            ++file_name_pos;
            if (path_pos > file_path + MAX_RESPONSE_PATH_LENGTH) {
                printf("File name too long %s\n", entry->d_name);
                continue;
            }
        }
        *path_pos = 0;
        int32_t urlLength = path_pos - file_name_start;
        printf("%s\n", file_path);
        FILE *entry_file = fopen((char *)file_path, "rb");
        if (entry_file == NULL) {
            printf("Failed to open response file\n");
            continue;
        }
        fseek(entry_file, 0, SEEK_END);
        long file_size = ftell(entry_file);
        if (file_size < 0) {
            printf("Failed to determine file size\n");
            continue;
        }
        rewind(entry_file);
        int32_t digit_order = 10;
        int32_t digits = 1;
        while (file_size >= digit_order) {
            ++digits;
            digit_order *= 10;
        }
        int32_t response_length = 32 + digits + 4 + file_size; 
        responses[responsesLength].response = malloc(response_length);
        uint8_t *response_pos = responses[responsesLength].response;
        if (response_pos == NULL) {
            printf("Failed to allocate memory for response file: %s\n", entry->d_name);
            continue;
        };
        uint8_t *start_response_pos = (uint8_t *)"HTTP/1.1 200 OK\r\nContent-Length:";
        uint8_t *start_response_end = start_response_pos + 32;
        while (start_response_pos < start_response_end) {
            *response_pos = *start_response_pos;
            ++response_pos;
            ++start_response_pos; 
        }
        int32_t size_copy = file_size;
        while (digit_order > 9) {
            digit_order /= 10;
            int digit_value = size_copy / digit_order;
            size_copy -= digit_value * digit_order;
            *response_pos = '0' + digit_value;
            printf("%c\n", *response_pos);
            ++response_pos;
        }
        *response_pos = '\r';
        *(++response_pos) = '\n';
        *(++response_pos) = '\r';
        *(++response_pos) = '\n';
        ++response_pos;
        if (fread(response_pos, 1, file_size, entry_file) != (size_t)file_size) {
            printf("Failed to read whole response file: %s\n", entry->d_name);
            continue;
        }
        fclose(entry_file);
        responses[responsesLength].responseLength = response_length;
        responses[responsesLength].url = (uint8_t *)entry->d_name;
        responses[responsesLength].urlLength = urlLength;
        ++responsesLength;
    }
    return 0;
}

struct server server;

int main(int argc, char **argv) {
    if (load_responses() < 0) {
        printf("load_responses failed\n");
        return 1;
    }

    int status = server_init(&server, responses, responsesLength);
    if (status < 0) {
        printf("server_init failed (%d)\n", status);
        return 1;
    }
    printf("Server initialized!\n");

    status = server_run(&server);
    if (status < 0) {
        printf("server_run failed (%d)\n", status);
        return 1;
    }
    return 0;
}
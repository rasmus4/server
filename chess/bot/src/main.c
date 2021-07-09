#include "client.h"
#include "bots/retarded.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static union {
    struct retarded retarded;
} main_botData;
static struct client main_client;

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s BOT [ROOM_ID]\n", argv[0]);
        return 1;
    }
    int32_t roomId = -1;
    if (argc > 2) roomId = atoi(argv[2]);

    struct clientCallbacks callbacks;
    if (strcmp(argv[1], "retarded") == 0) {
        clientCallbacks_create(&callbacks, &main_botData.retarded, retarded_makeMove);
    } else {
        printf("Unknown bot: %s\n", argv[1]);
        return 1;
    }

    client_create(&main_client, &callbacks);
    int status = client_run(&main_client, "127.0.0.1", 8089, roomId);
    printf("Status: %d\n", status);
    return 0;
}
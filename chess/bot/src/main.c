#include "client.h"
#include "bots/retarded.h"

#include <stdio.h>
#include <stdlib.h>

static struct client client;

int main(int argc, char **argv) {
    int32_t roomId = -1;
    if (argc > 1) roomId = atoi(argv[1]);

    struct clientCallbacks callbacks;
    clientCallbacks_create(&callbacks, NULL, retarded_makeMove);

    client_create(&client, &callbacks);
    int status = client_run(&client, "127.0.0.1", 8089, roomId);
    printf("Status: %d\n", status);
    return 0;
}
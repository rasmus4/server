#include "client.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    int32_t roomId = -1;
    if (argc > 1) roomId = atoi(argv[1]);

    struct client client;
    client_create(&client);
    int status = client_run(&client, "127.0.0.1", 8089, roomId);
    printf("Status: %d\n", status);
    return 0;
}
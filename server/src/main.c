#include <stdio.h>
#include "server.h"

int main(int argc, char **argv) {
    struct server server;
    int status = server_init(&server);
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
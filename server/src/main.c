#include "chess/chess.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static struct chess main_chess;

int main(int argc, char **argv) {
    int status;
    // TODO: Better seed...
    srand(time(NULL));

    status = chess_init(&main_chess);
    if (status < 0) {
        printf("Failed to initialize chess (%d)\n", status);
        status = 1;
        goto cleanup_none;
    }

    status = chess_run(&main_chess);
    if (status < 0) {
        printf("Chess ran into error (%d)\n", status);
        status = 1;
        goto cleanup_chess;
    }
    status = 0;
    cleanup_chess:
    chess_deinit(&main_chess);
    cleanup_none:
    return status;
}
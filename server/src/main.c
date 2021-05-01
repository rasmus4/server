#include "chess/chess.h"

#include "timespec.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <sched.h>
#include <sys/prctl.h>

static struct chess main_chess;

int main(int argc, char **argv) {
    int status;

    struct timespec currentTime;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    unsigned int seed = timespec_toNanoseconds(currentTime) % UINT_MAX;
    srand(seed);

    struct sched_param schedparm = {
        .sched_priority = 99
    };
	if (sched_setscheduler(0, SCHED_FIFO, &schedparm) < 0) {
        printf("Note: Could not set scheduler priority (run as root)\n");
    }

	if (prctl(PR_SET_TIMERSLACK, 1) < 0) {
        printf("Failed to set timer slack\n");
        status = 1;
        goto cleanup_none;
    }

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
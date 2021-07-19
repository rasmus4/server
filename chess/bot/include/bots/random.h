#pragma once

#include <stdint.h>
#include <stdbool.h>

struct random_move {
    int32_t from;
    int32_t to;
};

struct random {
    uint8_t board[144]; // 12 * 12
    struct random_move moves[256];
    int32_t numMoves;
};

static int random_makeMove(void *data, bool isHost, uint8_t *board, int32_t lastMoveFrom, int32_t lastMoveTo, int32_t *moveFrom, int32_t *moveTo);
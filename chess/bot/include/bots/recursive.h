#pragma once

#include <stdint.h>
#include <stdbool.h>

struct recursive_move {
    int32_t from;
    int32_t to;
    int32_t score;
};

struct recursive {
    uint8_t board[144]; // 12 * 12
    struct recursive_move moves[256];
    int32_t numMoves;
};

static int recursive_makeMove(void *data, bool isHost, uint8_t *board, int32_t lastMoveFrom, int32_t lastMoveTo, int32_t *moveFrom, int32_t *moveTo);
#pragma once

struct retarded {
    int dummy;
};

static int retarded_makeMove(void *data, bool isHost, uint8_t *board, int32_t lastMoveFrom, int32_t lastMoveTo, int32_t *moveFrom, int32_t *moveTo);
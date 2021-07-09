#pragma once

#include <stdint.h>
#include <stdbool.h>

struct clientCallbacks {
    void *data;
    int (*makeMove)(void *data, bool isHost, uint8_t *board, int32_t lastMoveFrom, int32_t lastMoveTo, int32_t *moveFrom, int32_t *moveTo);
};

static inline void clientCallbacks_create(
    struct clientCallbacks *self,
    void *data,
    int (*makeMove)(void *data, bool isHost, uint8_t *board, int32_t lastMoveFrom, int32_t lastMoveTo, int32_t *moveFrom, int32_t *moveTo)
);
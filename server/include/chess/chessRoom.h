#pragma once

#include <stdint.h>

struct chessClient;

struct chessRoom {
    int32_t index;
    int32_t roomId;
    struct chessClient *host; // NULL if room empty.
    struct chessClient *guest; // NULL if none.
    uint8_t board[64];
};

static inline void chessRoom_create(struct chessRoom *self, int32_t index);

static inline void chessRoom_open(struct chessRoom *self, struct chessClient *host, int32_t roomId);
static inline void chessRoom_addGuest(struct chessRoom *self, struct chessClient *guest);

static inline bool chessRoom_isOpen(struct chessRoom *self);
static inline bool chessRoom_isFull(struct chessRoom *self);

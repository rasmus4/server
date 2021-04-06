#pragma once

#include <stdint.h>

struct chessClient;

struct chessRoom {
    int32_t roomId;
    struct chessClient *host; // NULL if room empty.
    struct chessClient *guest; // NULL if none.
};

static void chessRoom_create(struct chessRoom *self, int32_t roomId);

static inline void chessRoom_setHost(struct chessRoom *self, struct chessClient *host);
static inline void chessRoom_setGuest(struct chessRoom *self, struct chessClient *guest);

static inline bool chessRoom_isEmpty(struct chessRoom *self);
static inline bool chessRoom_isFull(struct chessRoom *self);

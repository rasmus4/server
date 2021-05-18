#pragma once

#include <stdbool.h>
#include <stdint.h>

struct chessRoom;
struct chess;
struct server_client;

struct chessClient {
    struct server_client *client;
    struct chessRoom *room; // NULL if none.
};

static void chessClient_create(struct chessClient *self, struct server_client *client);

static inline void chessClient_setRoom(struct chessClient *self, struct chessRoom *room);
static inline void chessClient_unsetRoom(struct chessClient *self);
static inline bool chessClient_inRoom(struct chessClient *self);
static inline bool chessClient_isHost(struct chessClient *self);

#define chessClient_writeState_MAX 85
// Returns length written, max `chessClient_writeState_MAX` bytes.
static int32_t chessClient_writeState(struct chessClient *self, uint8_t *buffer);
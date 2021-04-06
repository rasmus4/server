#pragma once

#include <stdbool.h>

struct chessRoom;
struct chess;
struct server_client;

struct chessClient {
    struct server_client *client;
    struct chessRoom *room; // NULL if none.
};

static void chessClient_create(struct chessClient *self, struct server_client *client);

static inline void chessClient_setRoom(struct chessClient *self, struct chessRoom *room);
static inline bool chessClient_inRoom(struct chessClient *self);

static int chessClient_sendState(struct chessClient *self, struct chess *chess);
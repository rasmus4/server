#pragma once
#include "server.h"
#include "fileResponse.h"

#include <stdint.h>

struct chess_client {
    struct server_client *client;
    int32_t room; // -1 if none.
};

static void chess_client_create(struct chess_client *self, struct server_client *client);

struct chess_room {
    int32_t host;
};

struct chess {
    struct server server;
    struct fileResponse response;
    struct chess_client clients[server_MAX_CLIENTS]; // Same indices as server.
    struct chess_room rooms[server_MAX_CLIENTS];
};

static int chess_init(struct chess *self);
static void chess_deinit(struct chess *self);

static int chess_run(struct chess *self);
#pragma once

#include "server.h"
#include "fileResponse.h"

struct chess {
    struct server server;
    struct fileResponse response;
};

static int chess_init(struct chess *self);
static void chess_deinit(struct chess *self);

static int chess_run(struct chess *self);
struct chess {
    struct server server;
    struct fileResponse response;
    struct chessClient clients[server_MAX_CLIENTS]; // Same indices as server.
    struct chessRoom rooms[server_MAX_CLIENTS];
};

// Assumes srand has been called.
static int chess_init(struct chess *self);
static void chess_deinit(struct chess *self);

static int chess_run(struct chess *self);
static void chessClient_create(struct chessClient *self, struct serverClient *client) {
    self->serverClient = client;
    chessClient_unsetRoom(self);
}

static inline void chessClient_setRoom(struct chessClient *self, struct chessRoom *room) {
    self->room = room;
    self->move = room->numMoves;
}

static inline void chessClient_unsetRoom(struct chessClient *self) {
    self->room = NULL;
}

static inline bool chessClient_inRoom(struct chessClient *self) {
    return self->room;
}

static inline bool chessClient_isHost(struct chessClient *self) {
    assert(self->room);
    return self->room->host.client == self;
}

static inline bool chessClient_isGuest(struct chessClient *self) {
    assert(self->room);
    return self->room->guest.client == self;
}

static inline bool chessClient_isSpectator(struct chessClient *self) {
    assert(self->room);

    if (chessClient_isHost(self)) return false;
    if (chessRoom_isFull(self->room)) return !chessClient_isGuest(self);
    // Not host and room not full, must be spectator.
    return true;
}

static inline void chessClient_onNewMove(struct chessClient *self) {
    if (self->move == self->room->numMoves - 1) ++self->move;
}

static inline int chessClient_scrollMove(struct chessClient *self, bool forward) {
    int32_t newMove = self->move + (forward ? 1 : -1);
    if (newMove > self->room->numMoves || newMove < 0) return 1;
    self->move = newMove;
    return 0;
}

static int32_t chessClient_writeState(struct chessClient *self, uint8_t *buffer) {
    if (!chessClient_inRoom(self)) {
        buffer[0] = protocol_HOME;
        return 1;
    }
    if (chessRoom_isFull(self->room)) {
        bool hostPov = !chessClient_isGuest(self); // Host pov for spectators too.
        buffer[0] = protocol_CHESS;
        buffer[1] = chessRoom_isHostsTurn(self->room) ? 1 : 0;
        buffer[2] = (uint8_t)chessRoom_winner(self->room);

        struct chessRoom_move currentMove = chessRoom_getMove(self->room, self->move, hostPov);
        buffer[3] = (uint8_t)currentMove.fromIndex;
        buffer[4] = (uint8_t)currentMove.toIndex;

        int64_t timeSpent = chessRoom_timeSpent(self->room, hostPov);
        int64_t opponentTimeSpent = chessRoom_timeSpent(self->room, !hostPov);
        memcpy(&buffer[5], &timeSpent, 8);
        memcpy(&buffer[13], &opponentTimeSpent, 8);

        chessRoom_getBoard(self->room, self->move, hostPov, &buffer[21]);
        return chessClient_writeState_MAX;
    }
    buffer[0] = protocol_ROOM;
    memcpy(&buffer[1], &self->room->roomId, 4);
    return 5;
}

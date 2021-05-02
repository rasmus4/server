#include "chess/chessClient.h"
#include "chess/chess.h"
#include "chess/protocol.h"
#include "chess/chessRoom.h"

#include <stdbool.h>
#include <string.h>
#include <assert.h>

static void chessClient_create(struct chessClient *self, struct server_client *client) {
    self->client = client;
    chessClient_unsetRoom(self);
}

static inline void chessClient_setRoom(struct chessClient *self, struct chessRoom *room) {
    self->room = room;
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

static int32_t chessClient_writeState(struct chessClient *self, struct chess *chess, uint8_t *buffer) {
    if (!chessClient_inRoom(self)) {
        buffer[0] = protocol_HOME;
        return 1;
    }
    if (chessRoom_isFull(self->room)) {
        bool isHost = chessClient_isHost(self);
        buffer[0] = protocol_CHESS;
        buffer[1] = chessRoom_isHostsTurn(self->room) ? 1 : 0;
        buffer[2] = chessRoom_winner(self->room);
        buffer[3] = chessRoom_lastMoveFromIndex(self->room, isHost);
        buffer[4] = chessRoom_lastMoveToIndex(self->room, isHost);
        int64_t timeSpent = chessRoom_timeSpent(self->room, isHost);
        int64_t opponentTimeSpent = chessRoom_timeSpent(self->room, !isHost);
        memcpy(&buffer[5], &timeSpent, 8);
        memcpy(&buffer[13], &opponentTimeSpent, 8);
        if (isHost) {
            memcpy(&buffer[21], &self->room->board[0], 64);
        } else {
            // Flip the board for black.
            for (int i = 0; i < 64; ++i) {
                buffer[21 + 63 - i] = self->room->board[i];
            }
        }
        return chessClient_writeState_MAX;
    }
    buffer[0] = protocol_ROOM;
    memcpy(&buffer[1], &self->room->roomId, 4);
    return 5;
}
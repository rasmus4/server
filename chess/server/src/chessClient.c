#include "include/chessClient.h"
#include "include/chess.h"
#include "include/protocol.h"
#include "include/chessRoom.h"

#include <stdbool.h>
#include <string.h>
#include <assert.h>

static void chessClient_create(struct chessClient *self, struct serverClient *client) {
    self->serverClient = client;
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

static inline bool chessClient_isGuest(struct chessClient *self) {
    assert(self->room);
    return self->room->guest.client == self;
}

static inline bool chessClient_isSpectator(struct chessClient *self) {
    assert(self->room);
    // If we are not host we must either be guest or spectator in the room.
    // In both cases there exists a guest (can only spectate full games).
    return !chessClient_isHost(self) && !chessClient_isGuest(self);
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
        buffer[2] = chessRoom_winner(self->room);
        buffer[3] = chessRoom_lastMoveFromIndex(self->room, hostPov);
        buffer[4] = chessRoom_lastMoveToIndex(self->room, hostPov);
        int64_t timeSpent = chessRoom_timeSpent(self->room, hostPov);
        int64_t opponentTimeSpent = chessRoom_timeSpent(self->room, !hostPov);
        memcpy(&buffer[5], &timeSpent, 8);
        memcpy(&buffer[13], &opponentTimeSpent, 8);
        if (hostPov) {
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
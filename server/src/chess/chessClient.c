#include "chess/chessClient.h"
#include "chess/chess.h"
#include "chess/protocol.h"
#include "chess/chessRoom.h"

#include <stdbool.h>
#include <string.h>
#include <assert.h>

static void chessClient_create(struct chessClient *self, struct server_client *client) {
    self->client = client;
    self->room = NULL;
}

static inline void chessClient_setRoom(struct chessClient *self, struct chessRoom *room) {
    self->room = room;
}

static inline bool chessClient_inRoom(struct chessClient *self) {
    return self->room;
}

static inline bool chessClient_isHost(struct chessClient *self) {
    assert(self->room);
    return self->room->host == self;
}

static int chessClient_sendState(struct chessClient *self, struct chess *chess) {
    if (!chessClient_inRoom(self)) {
        uint8_t buffer[1] = { protocol_HOME };
        if (server_sendWebsocketMessage(&chess->server, self->client, buffer, sizeof(buffer), false) < 0) return -1;
    } else if (chessRoom_isFull(self->room)) {
        uint8_t buffer[66] = {0};
        buffer[0] = protocol_CHESS;
        buffer[1] = chessClient_isHost(self) ? 1 : 0;
        memcpy(&buffer[2], &self->room->board[0], 64);
        if (server_sendWebsocketMessage(&chess->server, self->client, buffer, sizeof(buffer), false) < 0) return -2;
    } else {
        uint8_t buffer[5] = { protocol_ROOM };
        memcpy(&buffer[1], &self->room->roomId, 4);
        if (server_sendWebsocketMessage(&chess->server, self->client, buffer, sizeof(buffer), false) < 0) return -3;
    }
    return 0;
}
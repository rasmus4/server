#include "chess/chessClient.h"
#include "chess/chess.h"
#include "chess/protocol.h"
#include "chess/chessRoom.h"

#include <stdbool.h>

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

static int chessClient_sendState(struct chessClient *self, struct chess *chess) {
    if (!chessClient_inRoom(self)) {
        uint8_t buffer[1] = { protocol_HOME };
        if (server_sendWebsocketMessage(&chess->server, self->client, buffer, sizeof(buffer), false) < 0) return -1;
    } else if (chessRoom_isFull(self->room)) {
        uint8_t buffer[66] = {0};
        buffer[0] = protocol_CHESS;
        buffer[1] = 1;
        buffer[2] = buffer[9] = protocol_ROOK;
        buffer[3] = buffer[8] = protocol_KNIGHT;
        buffer[4] = buffer[7] = protocol_BISHOP;
        buffer[5] = protocol_QUEEN;
        buffer[6] = protocol_KING;
        for (int i = 0; i < 8; ++i) buffer[10 + i] = protocol_PAWN;

        buffer[58] = buffer[65] = protocol_ROOK | protocol_WHITE_FLAG;
        buffer[59] = buffer[64] = protocol_KNIGHT | protocol_WHITE_FLAG;
        buffer[60] = buffer[63] = protocol_BISHOP | protocol_WHITE_FLAG;
        buffer[61] = protocol_QUEEN | protocol_WHITE_FLAG;
        buffer[62] = protocol_KING | protocol_WHITE_FLAG;
        for (int i = 0; i < 8; ++i) buffer[50 + i] = protocol_PAWN | protocol_WHITE_FLAG;
        if (server_sendWebsocketMessage(&chess->server, self->client, buffer, sizeof(buffer), false) < 0) return -2;
    } else {
        uint8_t buffer[5] = { protocol_ROOM };
        buffer[1] = self->room->roomId & 0xFF;
        buffer[2] = (self->room->roomId >> 8) & 0xFF;
        buffer[3] = (self->room->roomId >> 16) & 0xFF;
        buffer[4] = (self->room->roomId >> 24) & 0xFF;
        if (server_sendWebsocketMessage(&chess->server, self->client, buffer, sizeof(buffer), false) < 0) return -3;
    }
    return 0;
}
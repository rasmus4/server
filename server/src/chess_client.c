#include "chess.h"
#include "protocol.h"

#include <stdbool.h>

static void chess_client_create(struct chess_client *self, struct server_client *client) {
    self->client = client;
    self->room = -1;
}

static int chess_client_sendState(struct chess_client *self, struct chess *chess) {
    if (self->room == -1) {
        uint8_t buffer[1] = { protocol_HOME };
        if (server_sendWebsocketMessage(&chess->server, self->client, buffer, sizeof(buffer), false) < 0) return -1;
    } else {
        /*uint8_t buffer[66] = {0};
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
        if (server_sendWebsocketMessage(&SELF->server, client, buffer, 66, false) < 0) return -1;*/
    }
    return 0;
}

static int chess_client_onMessage(struct chess_client *self, uint8_t *message, uint32_t messageLength) {
    return 0;
}
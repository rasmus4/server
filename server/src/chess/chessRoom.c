#include "chess/chess.h"
#include "chess/chessClient.h"
#include "chess/protocol.h"

#include <stdbool.h>
#include <stdlib.h>

static void chessRoom_initBoard(struct chessRoom *self) {
    self->board[0] = self->board[7] = protocol_ROOK;
    self->board[1] = self->board[6] = protocol_KNIGHT;
    self->board[2] = self->board[5] = protocol_BISHOP;
    self->board[3] = protocol_QUEEN;
    self->board[4] = protocol_KING;
    for (int i = 0; i < 8; ++i) self->board[8 + i] = protocol_PAWN;

    self->board[56] = self->board[63] = protocol_ROOK | protocol_WHITE_FLAG;
    self->board[57] = self->board[62] = protocol_KNIGHT | protocol_WHITE_FLAG;
    self->board[58] = self->board[61] = protocol_BISHOP | protocol_WHITE_FLAG;
    self->board[59] = protocol_QUEEN | protocol_WHITE_FLAG;
    self->board[60] = protocol_KING | protocol_WHITE_FLAG;
    for (int i = 0; i < 8; ++i) self->board[48 + i] = protocol_PAWN | protocol_WHITE_FLAG;
}

static inline void chessRoom_create(struct chessRoom *self, int32_t index) {
    self->index = index;
    self->host = NULL;
    self->guest = NULL;
}

static inline void chessRoom_open(struct chessRoom *self, struct chessClient *host, int32_t roomId) {
    self->host = host;
    self->roomId = roomId;
    chessRoom_initBoard(self);
    chessClient_setRoom(host, self);
}

static inline void chessRoom_addGuest(struct chessRoom *self, struct chessClient *guest) {
    self->guest = guest;
    chessClient_setRoom(guest, self);
}

static inline bool chessRoom_isOpen(struct chessRoom *self) {
    return self->host;
}

static inline bool chessRoom_isFull(struct chessRoom *self) {
    return self->guest; 
}
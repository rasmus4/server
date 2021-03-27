#pragma once

#define protocol_VERSION 1

enum protocol_clientOp {
    protocol_MOVE = 1,
    protocol_CREATE = 2,
    protocol_JOIN = 3,
    protocol_BACK = 4
};

enum protocol_serverOp {
    protocol_HOME = 1,
    protocol_ROOM = 2,
    protocol_CHESS = 3
};

enum protocol_pieces {
    protocol_PAWN = 1,
    protocol_BISHOP = 2,
    protocol_KNIGHT = 3,
    protocol_ROOK = 4,
    protocol_QUEEN = 5,
    protocol_KING = 6,
    protocol_PIECE_MASK = 0x7F,
    protocol_WHITE_FLAG = 0x80
};

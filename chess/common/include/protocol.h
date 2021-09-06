#define protocol_VERSION 4

enum protocol_clientOp {
    protocol_MOVE = 0,
    protocol_CREATE = 1,
    protocol_JOIN = 2,
    protocol_BACK = 3,
    protocol_SPECTATE = 4,
    protocol_SCROLL = 5
};

enum protocol_serverOp {
    protocol_HOME = 0,
    protocol_ROOM = 1,
    protocol_CHESS = 2
};

enum protocol_winner {
    protocol_NO_WIN = 0,
    protocol_WHITE_WIN = 1,
    protocol_BLACK_WIN = 2
};

enum protocol_pieces {
    protocol_NO_PIECE = 0,
    protocol_PAWN = 1,
    protocol_BISHOP = 2,
    protocol_KNIGHT = 3,
    protocol_ROOK = 4,
    protocol_QUEEN = 5,
    protocol_KING = 6,
    protocol_PIECE_MASK = 0x3F,
    protocol_WHITE_FLAG = 0x80,
    protocol_BLACK_FLAG = 0x40
};

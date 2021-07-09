const protocolVersion = 3;

const ProtocolClientOp = {
    // Chess
    MOVE: 0,
    // Home
    CREATE: 1,
    JOIN: 2,
    BACK: 3,
    SPECTATE: 4,
    SCROLL: 5
};

const ProtocolServerOp = {
    HOME: 0,
    ROOM: 1,
    CHESS: 2
};

const ProtocolWinner = {
    NO_WIN: 0,
    WHITE_WIN: 1,
    BLACK_WIN: 2
};

const ProtocolPieces = {
    NO_PIECE: 0,
    PAWN: 1,
    BISHOP: 2,
    KNIGHT: 3,
    ROOK: 4,
    QUEEN: 5,
    KING: 6,
    PIECE_MASK: 0x7F,
    WHITE_FLAG: 0x80
};
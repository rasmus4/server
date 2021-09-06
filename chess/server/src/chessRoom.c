static void chessRoom_initBoard(struct chessRoom *self) {
    self->board[0] = self->board[7] = protocol_ROOK | protocol_WHITE_FLAG;
    self->board[1] = self->board[6] = protocol_KNIGHT | protocol_WHITE_FLAG;
    self->board[2] = self->board[5] = protocol_BISHOP | protocol_WHITE_FLAG;
    self->board[3] = protocol_QUEEN | protocol_WHITE_FLAG;
    self->board[4] = protocol_KING | protocol_WHITE_FLAG;
    for (int i = 0; i < 8; ++i) self->board[8 + i] = protocol_PAWN | protocol_WHITE_FLAG;

    for (int i = 16; i < 48; ++i) self->board[i] = protocol_NO_PIECE;

    self->board[56] = self->board[63] = protocol_ROOK | protocol_BLACK_FLAG;
    self->board[57] = self->board[62] = protocol_KNIGHT | protocol_BLACK_FLAG;
    self->board[58] = self->board[61] = protocol_BISHOP | protocol_BLACK_FLAG;
    self->board[59] = protocol_QUEEN | protocol_BLACK_FLAG;
    self->board[60] = protocol_KING | protocol_BLACK_FLAG;
    for (int i = 0; i < 8; ++i) self->board[48 + i] = protocol_PAWN | protocol_BLACK_FLAG;
}

static inline int32_t chessRoom_convertIndex(int32_t index, bool hostPov) {
    return hostPov ? index : 63 - index;
}

static inline int32_t chessRoom_xyToIndex(int32_t x, int32_t y, bool hostPov) {
    return chessRoom_convertIndex(y * 8 + x, hostPov);
}

static struct chessRoom_move chessRoom_getMove(struct chessRoom *self, int32_t move, bool hostPov) {
    assert(move >= 0 && move <= self->numMoves);
    struct chessRoom_move convertedMove;
    if (move == 0) {
        convertedMove = (struct chessRoom_move) {
            .fromIndex = -1,
            .toIndex = -1
        };
    } else {
        --move;
        convertedMove = (struct chessRoom_move) {
            .fromIndex = chessRoom_convertIndex(self->moves[move].fromIndex, hostPov),
            .toIndex = chessRoom_convertIndex(self->moves[move].toIndex, hostPov),
            .piece = self->moves[move].piece,
            .replacePiece = self->moves[move].replacePiece
        };
    }
    return convertedMove;
}

static void chessRoom_getBoard(struct chessRoom *self, int32_t move, bool hostPov, uint8_t *outBoard) {
    assert(move >= 0 && move <= self->numMoves);
    memcpy(&outBoard[0], &self->board[0], 64);
    for (int32_t current = self->numMoves - 1; current >= move; --current) {
        struct chessRoom_move *move = &self->moves[current];
        outBoard[move->fromIndex] = move->piece;
        outBoard[move->toIndex] = move->replacePiece;
    }
    if (!hostPov) {
        // Reverse board.
        for (int32_t i = 0; i < 64 / 2; ++i) {
            uint8_t temp = outBoard[i];
            outBoard[i] = outBoard[63 - i];
            outBoard[63 - i] = temp;
        }
    }
}

static inline bool chessRoom_diagonalAndFree(struct chessRoom *self, int32_t fromX, int32_t fromY, int32_t toX, int32_t toY, bool hostPov) {
    if (abs(toX - fromX) != abs(toY - fromY)) return false;
    int32_t signX = toX > fromX ? 1 : -1;
    int32_t signY = toY > fromY ? 1 : -1;
    for (fromX += signX, fromY += signY; fromX != toX; fromX += signX, fromY += signY) {
        if (self->board[chessRoom_xyToIndex(fromX, fromY, hostPov)] != protocol_NO_PIECE) return false;
    }
    return true;
}

static inline bool chessRoom_straightAndFree(struct chessRoom *self, int32_t fromX, int32_t fromY, int32_t toX, int32_t toY, bool hostPov) {
    if (toX != fromX && toY == fromY) {
        int signX = toX > fromX ? 1 : -1;
        for (fromX += signX; fromX != toX; fromX += signX) {
            if (self->board[chessRoom_xyToIndex(fromX, fromY, hostPov)] != protocol_NO_PIECE) return false;
        }
    } else if (toY != fromY && toX == fromX) {
        int signY = toY > fromY ? 1 : -1;
        for (fromY += signY; fromY != toY; fromY += signY) {
            if (self->board[chessRoom_xyToIndex(fromX, fromY, hostPov)] != protocol_NO_PIECE) return false;
        }
    } else return false;
    return true;
}

static inline int32_t chessRoom_distance(int32_t fromX, int32_t fromY, int32_t toX, int32_t toY) {
    int32_t dxAbs = abs(toX - fromX);
    int32_t dyAbs = abs(toY - fromY);
    if (dxAbs > dyAbs) return dxAbs;
    return dyAbs;
}

static inline void chessRoom_create(struct chessRoom *self, int32_t index) {
    self->index = index;
    self->host.client = NULL;
    self->guest.client = NULL;
}

static int chessRoom_open(struct chessRoom *self, struct chessClient *host, int32_t roomId, struct server *server) {
    self->host.client = host;
    self->roomId = roomId;
    self->spectators = NULL;
    self->numSpectators = 0;
    if (server_createTimer(server, &self->secondTimerHandle) < 0) return -1;

    self->numMoves = 0;
    self->moves = malloc(sizeof(self->moves[0])); // Need to be able to hold last move at least.
    if (self->moves == NULL) goto cleanup_secondTimer;
    return 0;

    cleanup_secondTimer:
    server_destroyTimer(self->secondTimerHandle);
    return -2;
}

static void chessRoom_close(struct chessRoom *self) {
    self->host.client = NULL;
    self->guest.client = NULL;
    free(self->moves);
    server_destroyTimer(self->secondTimerHandle);
    assert(self->numSpectators == 0);
    free(self->spectators);
}

static void chessRoom_start(struct chessRoom *self, struct chessClient *guest) {
    self->guest.client = guest;
    self->winner = protocol_NO_WIN;
    self->hostsTurn = true;
    self->host.timeSpent = 0;
    self->guest.timeSpent = 0;

    chessRoom_initBoard(self);
}

static int chessRoom_addSpectator(struct chessRoom *self, struct chessClient *spectator) {
    if (self->numSpectators == (INT32_MAX / sizeof(self->spectators[0]))) return -1; // Never know ;)
    struct chessClient **newSpectators = realloc(self->spectators, (size_t)((self->numSpectators + 1) * (int32_t)sizeof(self->spectators[0])));
    if (newSpectators == NULL) return -2;

    newSpectators[self->numSpectators] = spectator;
    self->spectators = newSpectators;
    ++self->numSpectators;
    return 0;
}

static void chessRoom_removeSpectator(struct chessRoom *self, struct chessClient *spectator) {
    struct chessClient **current = &self->spectators[0];
    for (;; ++current) {
        assert(current < &self->spectators[self->numSpectators]);
        if (*current == spectator) {
            --self->numSpectators;
            *current = self->spectators[self->numSpectators];
            if (self->numSpectators == 0) return; // Probably not worth freeing completely.

            struct chessClient **newSpectators = realloc(self->spectators, (size_t)(self->numSpectators * (int32_t)sizeof(self->spectators[0])));
            if (newSpectators != NULL) self->spectators = newSpectators;
            return;
        }
    }
}

static inline bool chessRoom_isOpen(struct chessRoom *self) {
    return self->host.client;
}

static inline bool chessRoom_isFull(struct chessRoom *self) {
    return self->guest.client;
}

static inline bool chessRoom_isHostsTurn(struct chessRoom *self) {
    return self->hostsTurn;
}

static inline enum protocol_winner chessRoom_winner(struct chessRoom *self) {
    return self->winner;
}

static bool chessRoom_isMoveValid(struct chessRoom *self, int32_t fromIndex, int32_t toIndex, bool hostPov) {
    if (self->winner != protocol_NO_WIN) return false;

    if (
        fromIndex < 0 || fromIndex > 63 ||
        toIndex < 0 || toIndex > 63
    ) return false;

    if (fromIndex == toIndex) return false;

    uint8_t piece = self->board[chessRoom_convertIndex(fromIndex, hostPov)];
    if (piece == protocol_NO_PIECE) return false;
    bool hostsPiece = piece & protocol_WHITE_FLAG;
    if (hostsPiece != hostPov) return false;

    uint8_t destPiece = self->board[chessRoom_convertIndex(toIndex, hostPov)];
    if ((piece & (protocol_WHITE_FLAG | protocol_BLACK_FLAG)) == (destPiece & (protocol_WHITE_FLAG | protocol_BLACK_FLAG))) return false; // Can't take your own pieces.

    int32_t fromX = fromIndex % 8;
    int32_t fromY = fromIndex / 8;
    int32_t toX = toIndex % 8;
    int32_t toY = toIndex / 8;

    switch (piece & protocol_PIECE_MASK) {
        case protocol_KING: return chessRoom_distance(fromX, fromY, toX, toY) == 1;
        case protocol_QUEEN: return (
            chessRoom_diagonalAndFree(self, fromX, fromY, toX, toY, hostPov) ||
            chessRoom_straightAndFree(self, fromX, fromY, toX, toY, hostPov)
        );
        case protocol_BISHOP: return chessRoom_diagonalAndFree(self, fromX, fromY, toX, toY, hostPov);
        case protocol_ROOK: return chessRoom_straightAndFree(self, fromX, fromY, toX, toY, hostPov);
        case protocol_KNIGHT: {
            int32_t dxAbs = abs(toX - fromX);
            int32_t dyAbs = abs(toY - fromY);
            return (
                (dxAbs == 1 && dyAbs == 2) ||
                (dxAbs == 2 && dyAbs == 1)
            );
        }
        case protocol_PAWN: {
            if (toY - fromY != 1) return false;
            int32_t dx = toX - fromX;
            if (dx == 0) return destPiece == protocol_NO_PIECE;
            if (dx == -1 || dx == 1) return destPiece != protocol_NO_PIECE;
            return false;
        }
        default: UNREACHABLE;
    }
}

static inline void chessRoom_updateMoves(struct chessRoom *self, struct chessRoom_move *lastMove) {
    if (self->numMoves != 0) { // Space for first move is guaranteed from `chessRoom_open`.
        struct chessRoom_move *newMoves;
        if (
            self->numMoves == INT32_MAX || // ...
            (newMoves = realloc(self->moves, (size_t)((self->numMoves + 1) * (int64_t)sizeof(self->moves[0])))) == NULL
        ) {
            // Forget oldest move.
            memmove(&self->moves[0], &self->moves[1], (size_t)((self->numMoves - 1) * (int64_t)sizeof(self->moves[0])));
            self->moves[self->numMoves - 1] = *lastMove;
            return;
        }
        self->moves = newMoves;
    }
    self->moves[self->numMoves] = *lastMove;
    ++self->numMoves;
    chessClient_onNewMove(self->host.client);
    chessClient_onNewMove(self->guest.client);
    for (int32_t i = 0; i < self->numSpectators; ++i) {
        chessClient_onNewMove(self->spectators[i]);
    }
}

static void chessRoom_doMove(struct chessRoom *self, int32_t fromIndex, int32_t toIndex, bool hostPov) {
    struct timespec currentTimespec;
    clock_gettime(CLOCK_MONOTONIC, &currentTimespec);
    int64_t currentTime = timespec_toNanoseconds(currentTimespec);

    // Initialise timer if first move.
    if (self->numMoves == 0) {
        self->host.lastUpdate = currentTime;
    }

    struct chessRoom_move move;
    move.fromIndex = chessRoom_convertIndex(fromIndex, hostPov);
    move.piece = self->board[move.fromIndex];
    move.toIndex = chessRoom_convertIndex(toIndex, hostPov);
    move.replacePiece = self->board[move.toIndex];

    int32_t newPiece;
    if (toIndex >= 56 && (move.piece & protocol_PIECE_MASK) == protocol_PAWN) {
        newPiece = protocol_QUEEN | (move.piece & ~protocol_PIECE_MASK); // Promotion
    } else {
        newPiece = move.piece;
    }

    self->board[move.toIndex] = (uint8_t)newPiece;
    self->board[move.fromIndex] = protocol_NO_PIECE;
    chessRoom_updateMoves(self, &move);

    if ((move.replacePiece & protocol_PIECE_MASK) == protocol_KING) { // Win
        if (move.replacePiece & protocol_WHITE_FLAG) self->winner = protocol_BLACK_WIN;
        else self->winner = protocol_WHITE_WIN;
    }

    chessRoom_updateTimeSpent(self, currentTime);

    int64_t timeSpent;
    if (self->hostsTurn) {
        self->guest.lastUpdate = currentTime;
        timeSpent = self->guest.timeSpent;
        self->hostsTurn = false;
    } else {
        self->host.lastUpdate = currentTime;
        timeSpent = self->host.timeSpent;
        self->hostsTurn = true;
    }
    if (chessRoom_winner(self) == protocol_NO_WIN) {
        struct itimerspec spec = {
            .it_interval.tv_sec = 1,
            .it_value = timespec_fromNanoseconds(currentTime + (int64_t)1000000000 - (timeSpent % 1000000000))
        };
        server_startTimer(self->secondTimerHandle, &spec, true);
    } else {
        server_stopTimer(self->secondTimerHandle);
    }
}

static inline void chessRoom_updateTimeSpent(struct chessRoom *self, int64_t currentTime) {
    if (self->hostsTurn) {
        self->host.timeSpent += (currentTime - self->host.lastUpdate);
        self->host.lastUpdate = currentTime;
    } else {
        self->guest.timeSpent += (currentTime - self->guest.lastUpdate);
        self->guest.lastUpdate = currentTime;
    }
}

static inline int64_t chessRoom_timeSpent(struct chessRoom *self, bool hostPov) {
    if (hostPov) return self->host.timeSpent;
    return self->guest.timeSpent;
}

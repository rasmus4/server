#include "bots/recursive.h"

#include <assert.h>
#include <stdlib.h>
#include <limits.h>

#define recursive_CONVERT_INDEX(INDEX) (INDEX + 26 + 4 * (INDEX / 8))
#define recursive_CONVERT_INDEX_BACK(INDEX) (INDEX - 18 - 4 * (INDEX / 12))

#define recursive_DEPTH 5

static int32_t recursive_pieceValues[7] = {
    0, // No piece
    1, // Pawn
    3, // Bishop
    3, // Knight
    5, // Rook
    9, // Queen
};

// Transform board to 12x12 with 2 wide borders, and pretend we are always white.
static void recursive_transformBoard(struct recursive *self, bool isHost, uint8_t *board) {
    // Make borders
    for (int32_t i = 0; i < 12; ++i) {
        // Bottom
        self->board[i] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
        self->board[12 + i] = protocol_WHITE_FLAG  | protocol_BLACK_FLAG;
        // Top
        self->board[12 * 10 + i] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
        self->board[12 * 11 + i] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
        // Left
        self->board[i * 12] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
        self->board[i * 12 + 1] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
        // Right
        self->board[i * 12 + 10] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
        self->board[i * 12 + 11] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
    }

    for (int32_t i = 0; i < 64; ++i) {
        uint8_t piece = board[i];
        if (!isHost && piece != protocol_NO_PIECE) {
            piece ^= (protocol_WHITE_FLAG | protocol_BLACK_FLAG);
        }
        self->board[recursive_CONVERT_INDEX(i)] = piece;
    }
}

static void recursive_dumpBoard(struct recursive *self) {
    for (int32_t y = 11; y >= 0; --y) {
        for (int32_t x = 0; x < 12; ++x) {
            uint8_t piece = self->board[y * 12 + x];
            char symbol;
            switch (piece & protocol_PIECE_MASK) {
                case protocol_KING:   symbol = 'k'; break;
                case protocol_QUEEN:  symbol = 'q'; break;
                case protocol_BISHOP: symbol = 'b'; break;
                case protocol_KNIGHT: symbol = 'n'; break;
                case protocol_ROOK:   symbol = 'r'; break;
                case protocol_PAWN:   symbol = 'p'; break;
                case protocol_NO_PIECE: {
                    if (piece & protocol_WHITE_FLAG) symbol = 'X';
                    else symbol = ' ';
                    goto done;
                }
                default: UNREACHABLE;
            }
            if (piece & protocol_WHITE_FLAG) {
                symbol -= 32;
            }
            done:
            printf("%c", symbol);
        }
        printf("\n");
    }
}

#define UP 12
#define DOWN (-12)
#define RIGHT 1
#define LEFT (-1)

#define recursive_TRY_WHITE_MOVE \
    if ((self->board[testIndex] & protocol_WHITE_FLAG) == 0) { \
        int32_t SCORE = recursive_evaluateWhiteMove(self, index, testIndex, remainingDepth - 1); \
        if (SCORE > best) best = SCORE; \
    }

#define recursive_TRY_WHITE_MOVES(OFFSET) \
    for (testIndex = index + OFFSET; (self->board[testIndex] & protocol_WHITE_FLAG) == 0; testIndex += OFFSET) { \
        int32_t SCORE = recursive_evaluateWhiteMove(self, index, testIndex, remainingDepth - 1); \
        if (SCORE > best) best = SCORE; \
        if (self->board[testIndex] != 0) break; \
    }

#define recursive_TRY_BLACK_MOVE \
    if ((self->board[testIndex] & protocol_BLACK_FLAG) == 0) { \
        int32_t SCORE = recursive_evaluateBlackMove(self, index, testIndex, remainingDepth - 1); \
        if (SCORE < best) best = SCORE; \
    }

#define recursive_TRY_BLACK_MOVES(OFFSET) \
    for (testIndex = index + OFFSET; (self->board[testIndex] & protocol_BLACK_FLAG) == 0; testIndex += OFFSET) { \
        int32_t SCORE = recursive_evaluateBlackMove(self, index, testIndex, remainingDepth - 1); \
        if (SCORE < best) best = SCORE; \
        if (self->board[testIndex] != 0) break; \
    }

static int32_t recursive_evaluateWhiteMove(struct recursive *self, int32_t from, int32_t to, int32_t remainingDepth);

static int32_t recursive_evaluateBlackMove(struct recursive *self, int32_t from, int32_t to, int32_t remainingDepth) {
    uint8_t takenPiece = self->board[to];
    if (takenPiece == (protocol_KING | protocol_WHITE_FLAG)) return -10000;
    int32_t pieceValue = recursive_pieceValues[takenPiece & protocol_PIECE_MASK];
    uint8_t originalPiece = self->board[from];
    if (originalPiece == (protocol_PAWN | protocol_BLACK_FLAG) && to < recursive_CONVERT_INDEX(8)) {
        self->board[from] = protocol_QUEEN | protocol_BLACK_FLAG;
        pieceValue += 8;
    }
    if (remainingDepth == 0) return -pieceValue;

    // Find best response for white.
    self->board[to] = self->board[from];
    self->board[from] = 0;
    int32_t best = INT32_MIN;
    for (int32_t i = 0; i < 64; ++i) {
        int32_t index = recursive_CONVERT_INDEX(i);
        uint8_t piece = self->board[index];
        if ((piece & protocol_WHITE_FLAG) == 0) continue;

        int32_t testIndex;
        switch (piece & protocol_PIECE_MASK) {
            case protocol_KING: {
                testIndex = index + DOWN + LEFT;
                recursive_TRY_WHITE_MOVE
                testIndex = index + DOWN;
                recursive_TRY_WHITE_MOVE
                testIndex = index + DOWN + RIGHT;
                recursive_TRY_WHITE_MOVE
                testIndex = index + LEFT;
                recursive_TRY_WHITE_MOVE
                testIndex = index + RIGHT;
                recursive_TRY_WHITE_MOVE
                testIndex = index + UP + LEFT;
                recursive_TRY_WHITE_MOVE
                testIndex = index + UP;
                recursive_TRY_WHITE_MOVE
                testIndex = index + UP + RIGHT;
                recursive_TRY_WHITE_MOVE
                break;
            }
            case protocol_QUEEN:
            case protocol_BISHOP: {
                recursive_TRY_WHITE_MOVES(UP + LEFT)
                recursive_TRY_WHITE_MOVES(DOWN + LEFT)
                recursive_TRY_WHITE_MOVES(UP + RIGHT)
                recursive_TRY_WHITE_MOVES(DOWN + RIGHT)
                if ((piece & protocol_PIECE_MASK) == protocol_BISHOP) break;
            }
            case protocol_ROOK: {
                recursive_TRY_WHITE_MOVES(UP)
                recursive_TRY_WHITE_MOVES(DOWN)
                recursive_TRY_WHITE_MOVES(RIGHT)
                recursive_TRY_WHITE_MOVES(LEFT)
                break;
            }
            case protocol_KNIGHT: {
                testIndex = index + 2 * LEFT + DOWN;
                recursive_TRY_WHITE_MOVE
                testIndex = index + 2 * RIGHT + DOWN;
                recursive_TRY_WHITE_MOVE
                testIndex = index + 2 * RIGHT + UP;
                recursive_TRY_WHITE_MOVE
                testIndex = index + 2 * LEFT + UP;
                recursive_TRY_WHITE_MOVE
                testIndex = index + 2 * DOWN + LEFT;
                recursive_TRY_WHITE_MOVE
                testIndex = index + 2 * DOWN + RIGHT;
                recursive_TRY_WHITE_MOVE
                testIndex = index + 2 * UP + RIGHT;
                recursive_TRY_WHITE_MOVE
                testIndex = index + 2 * UP + LEFT;
                recursive_TRY_WHITE_MOVE
                break;
            }
            case protocol_PAWN: {
                testIndex = index + UP;
                if (self->board[testIndex] == 0) {
                    int32_t moveScore = recursive_evaluateWhiteMove(self, index, testIndex, remainingDepth - 1);
                    if (moveScore > best) best = moveScore;
                }

                testIndex = index + UP + LEFT;
                if (self->board[testIndex] != 0 && (self->board[testIndex] & protocol_WHITE_FLAG) == 0) {
                    int32_t moveScore = recursive_evaluateWhiteMove(self, index, testIndex, remainingDepth - 1);
                    if (moveScore > best) best = moveScore;
                }

                testIndex = index + UP + RIGHT;
                if (self->board[testIndex] != 0 && (self->board[testIndex] & protocol_WHITE_FLAG) == 0) {
                    int32_t moveScore = recursive_evaluateWhiteMove(self, index, testIndex, remainingDepth - 1);
                    if (moveScore > best) best = moveScore;
                }
                break;
            }
            default: UNREACHABLE;
        }
    }
    self->board[from] = originalPiece;
    self->board[to] = takenPiece;
    return best - pieceValue;
}

static int32_t recursive_evaluateWhiteMove(struct recursive *self, int32_t from, int32_t to, int32_t remainingDepth) {
    uint8_t takenPiece = self->board[to];
    if (takenPiece == (protocol_KING | protocol_BLACK_FLAG)) return 10000;
    int32_t pieceValue = recursive_pieceValues[takenPiece & protocol_PIECE_MASK];
    uint8_t originalPiece = self->board[from];
    if (originalPiece == (protocol_PAWN | protocol_WHITE_FLAG) && to >= recursive_CONVERT_INDEX(56)) {
        self->board[from] = protocol_QUEEN | protocol_WHITE_FLAG;
        pieceValue += 8;
    }
    if (remainingDepth == 0) return pieceValue;

    // Find best response for black.
    self->board[to] = self->board[from];
    self->board[from] = 0;
    int32_t best = INT32_MAX;
    for (int32_t i = 0; i < 64; ++i) {
        int32_t index = recursive_CONVERT_INDEX(i);
        uint8_t piece = self->board[index];
        if ((piece & protocol_BLACK_FLAG) == 0) continue;

        int32_t testIndex;
        switch (piece & protocol_PIECE_MASK) {
            case protocol_KING: {
                testIndex = index + DOWN + LEFT;
                recursive_TRY_BLACK_MOVE
                testIndex = index + DOWN;
                recursive_TRY_BLACK_MOVE
                testIndex = index + DOWN + RIGHT;
                recursive_TRY_BLACK_MOVE
                testIndex = index + LEFT;
                recursive_TRY_BLACK_MOVE
                testIndex = index + RIGHT;
                recursive_TRY_BLACK_MOVE
                testIndex = index + UP + LEFT;
                recursive_TRY_BLACK_MOVE
                testIndex = index + UP;
                recursive_TRY_BLACK_MOVE
                testIndex = index + UP + RIGHT;
                recursive_TRY_BLACK_MOVE
                break;
            }
            case protocol_QUEEN:
            case protocol_BISHOP: {
                recursive_TRY_BLACK_MOVES(UP + LEFT)
                recursive_TRY_BLACK_MOVES(DOWN + LEFT)
                recursive_TRY_BLACK_MOVES(UP + RIGHT)
                recursive_TRY_BLACK_MOVES(DOWN + RIGHT)
                if ((piece & protocol_PIECE_MASK) == protocol_BISHOP) break;
            }
            case protocol_ROOK: {
                recursive_TRY_BLACK_MOVES(UP)
                recursive_TRY_BLACK_MOVES(DOWN)
                recursive_TRY_BLACK_MOVES(RIGHT)
                recursive_TRY_BLACK_MOVES(LEFT)
                break;
            }
            case protocol_KNIGHT: {
                testIndex = index + 2 * LEFT + DOWN;
                recursive_TRY_BLACK_MOVE
                testIndex = index + 2 * RIGHT + DOWN;
                recursive_TRY_BLACK_MOVE
                testIndex = index + 2 * RIGHT + UP;
                recursive_TRY_BLACK_MOVE
                testIndex = index + 2 * LEFT + UP;
                recursive_TRY_BLACK_MOVE
                testIndex = index + 2 * DOWN + LEFT;
                recursive_TRY_BLACK_MOVE
                testIndex = index + 2 * DOWN + RIGHT;
                recursive_TRY_BLACK_MOVE
                testIndex = index + 2 * UP + RIGHT;
                recursive_TRY_BLACK_MOVE
                testIndex = index + 2 * UP + LEFT;
                recursive_TRY_BLACK_MOVE
                break;
            }
            case protocol_PAWN: {
                testIndex = index + DOWN;
                if (self->board[testIndex] == 0) {
                    int32_t moveScore = recursive_evaluateBlackMove(self, index, testIndex, remainingDepth - 1);
                    if (moveScore < best) best = moveScore;
                }

                testIndex = index + DOWN + LEFT;
                if (self->board[testIndex] != 0 && (self->board[testIndex] & protocol_BLACK_FLAG) == 0) {
                    int32_t moveScore = recursive_evaluateBlackMove(self, index, testIndex, remainingDepth - 1);
                    if (moveScore < best) best = moveScore;
                }

                testIndex = index + DOWN + RIGHT;
                if (self->board[testIndex] != 0 && (self->board[testIndex] & protocol_BLACK_FLAG) == 0) {
                    int32_t moveScore = recursive_evaluateBlackMove(self, index, testIndex, remainingDepth - 1);
                    if (moveScore < best) best = moveScore;
                }
                break;
            }
            default: UNREACHABLE;
        }
    }
    self->board[from] = originalPiece;
    self->board[to] = takenPiece;
    return best + pieceValue;
}

#define recursive_TRY_MOVE \
    if ((self->board[testIndex] & protocol_WHITE_FLAG) == 0) { \
        self->moves[self->numMoves++] = (struct recursive_move) { \
            .from = index, \
            .to = testIndex, \
            .score = recursive_evaluateWhiteMove(self, index, testIndex, recursive_DEPTH) \
        }; \
    }

static void recursive_kingMoves(struct recursive *self, int32_t index) {
    int32_t testIndex;
    testIndex = index + DOWN + LEFT;
    recursive_TRY_MOVE
    testIndex = index + DOWN;
    recursive_TRY_MOVE
    testIndex = index + DOWN + RIGHT;
    recursive_TRY_MOVE
    testIndex = index + LEFT;
    recursive_TRY_MOVE
    testIndex = index + RIGHT;
    recursive_TRY_MOVE
    testIndex = index + UP + LEFT;
    recursive_TRY_MOVE
    testIndex = index + UP;
    recursive_TRY_MOVE
    testIndex = index + UP + RIGHT;
    recursive_TRY_MOVE
}

static void recursive_knightMoves(struct recursive *self, int32_t index) {
    int32_t testIndex;
    testIndex = index + 2 * LEFT + DOWN;
    recursive_TRY_MOVE
    testIndex = index + 2 * RIGHT + DOWN;
    recursive_TRY_MOVE
    testIndex = index + 2 * RIGHT + UP;
    recursive_TRY_MOVE
    testIndex = index + 2 * LEFT + UP;
    recursive_TRY_MOVE
    testIndex = index + 2 * DOWN + LEFT;
    recursive_TRY_MOVE
    testIndex = index + 2 * DOWN + RIGHT;
    recursive_TRY_MOVE
    testIndex = index + 2 * UP + RIGHT;
    recursive_TRY_MOVE
    testIndex = index + 2 * UP + LEFT;
    recursive_TRY_MOVE
}

static void recursive_pawnMoves(struct recursive *self, int32_t index) {
    int32_t testIndex = index + UP;
    if (self->board[testIndex] == 0) {
        self->moves[self->numMoves++] = (struct recursive_move) {
            .from = index,
            .to = testIndex,
            .score = recursive_evaluateWhiteMove(self, index, testIndex, recursive_DEPTH)
        };
    }

    testIndex = index + UP + LEFT;
    if (self->board[testIndex] != 0 && (self->board[testIndex] & protocol_WHITE_FLAG) == 0) {
        self->moves[self->numMoves++] = (struct recursive_move) {
            .from = index,
            .to = testIndex,
            .score = recursive_evaluateWhiteMove(self, index, testIndex, recursive_DEPTH)
        };
    }

    testIndex = index + UP + RIGHT;
    if (self->board[testIndex] != 0 && (self->board[testIndex] & protocol_WHITE_FLAG) == 0) {
        self->moves[self->numMoves++] = (struct recursive_move) {
            .from = index,
            .to = testIndex,
            .score = recursive_evaluateWhiteMove(self, index, testIndex, recursive_DEPTH)
        };
    }
}

#define recursive_TRY_MOVES(OFFSET) \
    for (testIndex = index + OFFSET; (self->board[testIndex] & protocol_WHITE_FLAG) == 0; testIndex += OFFSET) { \
        self->moves[self->numMoves++] = (struct recursive_move) { \
            .from = index, \
            .to = testIndex, \
            .score = recursive_evaluateWhiteMove(self, index, testIndex, recursive_DEPTH) \
        }; \
        if (self->board[testIndex] != 0) break; \
    }

static void recursive_rookMoves(struct recursive *self, int32_t index) {
    int32_t testIndex;
    recursive_TRY_MOVES(UP)
    recursive_TRY_MOVES(DOWN)
    recursive_TRY_MOVES(RIGHT)
    recursive_TRY_MOVES(LEFT)
}

static void recursive_bishopMoves(struct recursive *self, int32_t index) {
    int32_t testIndex;
    recursive_TRY_MOVES(UP + LEFT)
    recursive_TRY_MOVES(DOWN + LEFT)
    recursive_TRY_MOVES(UP + RIGHT)
    recursive_TRY_MOVES(DOWN + RIGHT)
}

#undef UP
#undef DOWN
#undef RIGHT
#undef LEFT

#define SELF ((struct recursive *)data)

static int recursive_makeMove(void *data, bool isHost, uint8_t *board, int32_t lastMoveFrom, int32_t lastMoveTo, int32_t *moveFrom, int32_t *moveTo) {
    recursive_transformBoard(SELF, isHost, board);
    recursive_dumpBoard(SELF);

    SELF->numMoves = 0;
    for (int32_t i = 0; i < 64; ++i) {
        int32_t index = recursive_CONVERT_INDEX(i);
        uint8_t piece = SELF->board[index];
        if (piece & protocol_WHITE_FLAG) {
            switch (piece & protocol_PIECE_MASK) {
                case protocol_KING: {
                    recursive_kingMoves(SELF, index);
                    break;
                }
                case protocol_QUEEN: {
                    recursive_rookMoves(SELF, index);
                    // fallthrough
                }
                case protocol_BISHOP: {
                    recursive_bishopMoves(SELF, index);
                    break;
                }
                case protocol_ROOK: {
                    recursive_rookMoves(SELF, index);
                    break;
                }
                case protocol_KNIGHT: {
                    recursive_knightMoves(SELF, index);
                    break;
                }
                case protocol_PAWN: {
                    recursive_pawnMoves(SELF, index);
                    break;
                }
                default: UNREACHABLE;
            }
        }
    }
    printf("Found %d moves\n", SELF->numMoves);
    if (SELF->numMoves == 0) return -1;

    int32_t bestScore = INT32_MIN;
    int32_t numBestMoves = 0;
    for (int32_t i = 0; i < SELF->numMoves; ++i) {
        if (SELF->moves[i].score > bestScore) {
            bestScore = SELF->moves[i].score;
        }
    }

    for (int32_t i = 0; i < SELF->numMoves; ++i) {
        if (SELF->moves[i].score == bestScore) {
            ++numBestMoves;
            printf("Best move: %d->%d (%d)\n", recursive_CONVERT_INDEX_BACK(SELF->moves[i].from), recursive_CONVERT_INDEX_BACK(SELF->moves[i].to), bestScore);
        }
    }

    int32_t moveIndex = rand() % numBestMoves;
    for (int32_t i = 0, j = 0; i < SELF->numMoves; ++i) {
        if (SELF->moves[i].score == bestScore) {
            if (j == moveIndex) {
                *moveFrom = recursive_CONVERT_INDEX_BACK(SELF->moves[i].from);
                *moveTo = recursive_CONVERT_INDEX_BACK(SELF->moves[i].to);
                return 0;
            }
            ++j;
        }
    }
    return -1;
}

#undef SELF
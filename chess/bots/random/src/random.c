#include "random.h"
#include "protocol.h"

#include <assert.h>
#include <stdlib.h>

struct random_move {
    int32_t from;
    int32_t to;
};

static uint8_t random_board[144]; // 12 * 12
static struct random_move random_moves[256];
static int32_t random_numMoves;

#define random_CONVERT_INDEX(INDEX) (INDEX + 26 + 4 * (INDEX / 8))
#define random_CONVERT_INDEX_BACK(INDEX) (INDEX - 18 - 4 * (INDEX / 12))

// Transform board to 12x12 with 2 wide borders, and pretend we are always white.
static void random_transformBoard(bool isHost, uint8_t *board) {
    // Make borders
    for (int32_t i = 0; i < 12; ++i) {
        // Bottom
        random_board[i] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
        random_board[12 + i] = protocol_WHITE_FLAG  | protocol_BLACK_FLAG;
        // Top
        random_board[12 * 10 + i] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
        random_board[12 * 11 + i] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
        // Left
        random_board[i * 12] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
        random_board[i * 12 + 1] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
        // Right
        random_board[i * 12 + 10] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
        random_board[i * 12 + 11] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
    }

    for (int32_t i = 0; i < 64; ++i) {
        uint8_t piece = board[i];
        if (!isHost && piece != protocol_NO_PIECE) {
            piece ^= (protocol_WHITE_FLAG | protocol_BLACK_FLAG);
        }
        random_board[random_CONVERT_INDEX(i)] = piece;
    }
}

static void random_dumpBoard() {
    for (int32_t y = 11; y >= 0; --y) {
        for (int32_t x = 0; x < 12; ++x) {
            uint8_t piece = random_board[y * 12 + x];
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

#define random_TRY_MOVE \
    if ((random_board[testIndex] & protocol_WHITE_FLAG) == 0) { \
        random_moves[random_numMoves++] = (struct random_move) { \
            .from = index, \
            .to = testIndex \
        }; \
    }

#define UP 12
#define DOWN (-12)
#define RIGHT 1
#define LEFT (-1)

static void random_kingMoves(int32_t index) {
    int32_t testIndex;
    testIndex = index + DOWN + LEFT;
    random_TRY_MOVE
    testIndex = index + DOWN;
    random_TRY_MOVE
    testIndex = index + DOWN + RIGHT;
    random_TRY_MOVE
    testIndex = index + LEFT;
    random_TRY_MOVE
    testIndex = index + RIGHT;
    random_TRY_MOVE
    testIndex = index + UP + LEFT;
    random_TRY_MOVE
    testIndex = index + UP;
    random_TRY_MOVE
    testIndex = index + UP + RIGHT;
    random_TRY_MOVE
}

static void random_knightMoves(int32_t index) {
    int32_t testIndex;
    testIndex = index + 2 * LEFT + DOWN;
    random_TRY_MOVE
    testIndex = index + 2 * RIGHT + DOWN;
    random_TRY_MOVE
    testIndex = index + 2 * RIGHT + UP;
    random_TRY_MOVE
    testIndex = index + 2 * LEFT + UP;
    random_TRY_MOVE
    testIndex = index + 2 * DOWN + LEFT;
    random_TRY_MOVE
    testIndex = index + 2 * DOWN + RIGHT;
    random_TRY_MOVE
    testIndex = index + 2 * UP + RIGHT;
    random_TRY_MOVE
    testIndex = index + 2 * UP + LEFT;
    random_TRY_MOVE
}

static void random_pawnMoves(int32_t index) {
    uint8_t piece;
    piece = random_board[index + UP];
    if (piece == 0) {
        random_moves[random_numMoves++] = (struct random_move) {
            .from = index,
            .to = index + UP
        };
    }

    piece = random_board[index + UP + LEFT];
    if (piece != 0 && (piece & protocol_WHITE_FLAG) == 0) {
        random_moves[random_numMoves++] = (struct random_move) {
            .from = index,
            .to = index + UP + LEFT
        };
    }

    piece = random_board[index + UP + RIGHT];
    if (piece != 0 && (piece & protocol_WHITE_FLAG) == 0) {
        random_moves[random_numMoves++] = (struct random_move) {
            .from = index,
            .to = index + UP + RIGHT
        };
    }
}

#define random_TRY_MOVES(OFFSET) \
    for (testIndex = index + OFFSET; (random_board[testIndex] & protocol_WHITE_FLAG) == 0; testIndex += OFFSET) { \
        random_moves[random_numMoves++] = (struct random_move) { \
            .from = index, \
            .to = testIndex \
        }; \
        if (random_board[testIndex] != 0) break; \
    }

static void random_rookMoves(int32_t index) {
    int32_t testIndex;
    random_TRY_MOVES(UP)
    random_TRY_MOVES(DOWN)
    random_TRY_MOVES(RIGHT)
    random_TRY_MOVES(LEFT)
}

static void random_bishopMoves(int32_t index) {
    int32_t testIndex;
    random_TRY_MOVES(UP + LEFT)
    random_TRY_MOVES(DOWN + LEFT)
    random_TRY_MOVES(UP + RIGHT)
    random_TRY_MOVES(DOWN + RIGHT)
}

#undef UP
#undef DOWN
#undef RIGHT
#undef LEFT

static int random_makeMove(bool isHost, uint8_t *board, int32_t lastMoveFrom, int32_t lastMoveTo, int32_t *moveFrom, int32_t *moveTo) {
    random_transformBoard(isHost, board);
    random_dumpBoard();
    random_numMoves = 0;
    for (int32_t i = 0; i < 64; ++i) {
        int32_t index = random_CONVERT_INDEX(i);
        uint8_t piece = random_board[index];
        if (piece & protocol_WHITE_FLAG) {
            switch (piece & protocol_PIECE_MASK) {
                case protocol_KING: {
                    random_kingMoves(index);
                    break;
                }
                case protocol_QUEEN: {
                    random_rookMoves(index);
                    // fallthrough
                }
                case protocol_BISHOP: {
                    random_bishopMoves(index);
                    break;
                }
                case protocol_ROOK: {
                    random_rookMoves(index);
                    break;
                }
                case protocol_KNIGHT: {
                    random_knightMoves(index);
                    break;
                }
                case protocol_PAWN: {
                    random_pawnMoves(index);
                    break;
                }
                default: UNREACHABLE;
            }
        }
    }
    printf("Found %d moves\n", random_numMoves);
    if (random_numMoves == 0) return -1;
    int32_t moveIndex = rand() % random_numMoves;
    struct random_move *move = &random_moves[moveIndex];
    *moveFrom = random_CONVERT_INDEX_BACK(move->from);
    *moveTo = random_CONVERT_INDEX_BACK(move->to);
    return 0;
}

#include "bots/random.h"

#include <assert.h>
#include <stdlib.h>

#define random_CONVERT_INDEX(index) (index + 26 + 4 * (index / 8))
#define random_CONVERT_INDEX_BACK(index) (index - 18 - 4 * (index / 12))

// Transform board to 12x12 with 2 wide borders, and pretend we are always white.
static void random_transformBoard(struct random *self, bool isHost, uint8_t *board) {
    // Make borders
    for (int32_t i = 0; i < 12; ++i) {
        // Bottom
        self->board[i] = protocol_WHITE_FLAG;
        self->board[12 + i] = protocol_WHITE_FLAG;
        // Top
        self->board[12 * 10 + i] = protocol_WHITE_FLAG;
        self->board[12 * 11 + i] = protocol_WHITE_FLAG;
        // Left
        self->board[i * 12] = protocol_WHITE_FLAG;
        self->board[i * 12 + 1] = protocol_WHITE_FLAG;
        // Right
        self->board[i * 12 + 10] = protocol_WHITE_FLAG;
        self->board[i * 12 + 11] = protocol_WHITE_FLAG;
    }

    for (int32_t i = 0; i < 64; ++i) {
        uint8_t piece = board[i];
        if (!isHost && piece != protocol_NO_PIECE) {
            piece ^= protocol_WHITE_FLAG;
        }
        self->board[random_CONVERT_INDEX(i)] = piece;
    }
}

static void random_dumpBoard(struct random *self) {
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

#define random_TRY_MOVE \
    if ((self->board[testIndex] & protocol_WHITE_FLAG) == 0) { \
        self->moves[self->numMoves++] = (struct random_move) { \
            .from = index, \
            .to = testIndex \
        }; \
    }

#define UP 12
#define DOWN (-12)
#define RIGHT 1
#define LEFT (-1)

static void random_kingMoves(struct random *self, int32_t index) {
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

static void random_knightMoves(struct random *self, int32_t index) {
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

#undef UP
#undef DOWN
#undef RIGHT
#undef LEFT

#define SELF ((struct random *)data)

static int random_makeMove(void *data, bool isHost, uint8_t *board, int32_t lastMoveFrom, int32_t lastMoveTo, int32_t *moveFrom, int32_t *moveTo) {
    random_transformBoard(SELF, isHost, board);
    random_dumpBoard(SELF);
    SELF->numMoves = 0;
    for (int32_t i = 0; i < 64; ++i) {
        int32_t index = random_CONVERT_INDEX(i);
        uint8_t piece = SELF->board[index];
        if (piece & protocol_WHITE_FLAG) {
            switch (piece & protocol_PIECE_MASK) {
                case protocol_KING: {
                    random_kingMoves(SELF, index);
                    break;
                }
                case protocol_QUEEN: {
                    break;
                }
                case protocol_BISHOP: {
                    break;
                }
                case protocol_ROOK: {
                    break;
                }
                case protocol_KNIGHT: {
                    random_knightMoves(SELF, index);
                    break;
                }
                case protocol_PAWN: {
                    break;
                }
                default: UNREACHABLE;
            }
        }
    }
    printf("Found %d moves\n", SELF->numMoves);
    if (SELF->numMoves == 0) return -1;
    int32_t moveIndex = rand() % SELF->numMoves;
    struct random_move *move = &SELF->moves[moveIndex];
    *moveFrom = random_CONVERT_INDEX_BACK(move->from);
    *moveTo = random_CONVERT_INDEX_BACK(move->to);
    return 0;
}

#undef SELF
#include "common.h"
#include "protocol.h"

static void common_transformBoard(bool isHost, uint8_t *board) {
    // Make borders
    for (int32_t i = 0; i < 12; ++i) {
        // Bottom
        common_board[i] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
        common_board[12 + i] = protocol_WHITE_FLAG  | protocol_BLACK_FLAG;
        // Top
        common_board[12 * 10 + i] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
        common_board[12 * 11 + i] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
        // Left
        common_board[i * 12] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
        common_board[i * 12 + 1] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
        // Right
        common_board[i * 12 + 10] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
        common_board[i * 12 + 11] = protocol_WHITE_FLAG | protocol_BLACK_FLAG;
    }

    for (int32_t i = 0; i < 64; ++i) {
        uint8_t piece = board[i];
        if (!isHost && piece != protocol_NO_PIECE) {
            piece ^= (protocol_WHITE_FLAG | protocol_BLACK_FLAG);
        }
        common_board[common_CONVERT_INDEX(i)] = piece;
    }
}

static void common_dumpBoard(void) {
    for (int32_t y = 11; y >= 0; --y) {
        for (int32_t x = 0; x < 12; ++x) {
            uint8_t piece = common_board[y * 12 + x];
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

#define common_TRY_MOVE \
    if ((common_board[testIndex] & protocol_WHITE_FLAG) == 0) { \
        common_moves[common_numMoves++] = (struct common_move) { \
            .from = index, \
            .to = testIndex \
        }; \
    }

#define common_TRY_MOVES(OFFSET) \
    for (testIndex = index + OFFSET; (common_board[testIndex] & protocol_WHITE_FLAG) == 0; testIndex += OFFSET) { \
        common_moves[common_numMoves++] = (struct common_move) { \
            .from = index, \
            .to = testIndex \
        }; \
        if (common_board[testIndex] != 0) break; \
    }

#define UP 12
#define DOWN (-12)
#define RIGHT 1
#define LEFT (-1)

static void common_findMoves(void) {
    common_numMoves = 0;
    for (int32_t i = 0; i < 64; ++i) {
        int32_t index = common_CONVERT_INDEX(i);
        uint8_t piece = common_board[index];
        if (piece & protocol_WHITE_FLAG) {
            int32_t testIndex;
            switch (piece & protocol_PIECE_MASK) {
                case protocol_KING: {
                    testIndex = index + DOWN + LEFT;
                    common_TRY_MOVE
                    testIndex = index + DOWN;
                    common_TRY_MOVE
                    testIndex = index + DOWN + RIGHT;
                    common_TRY_MOVE
                    testIndex = index + LEFT;
                    common_TRY_MOVE
                    testIndex = index + RIGHT;
                    common_TRY_MOVE
                    testIndex = index + UP + LEFT;
                    common_TRY_MOVE
                    testIndex = index + UP;
                    common_TRY_MOVE
                    testIndex = index + UP + RIGHT;
                    common_TRY_MOVE
                    break;
                }
                case protocol_QUEEN:
                case protocol_BISHOP: {
                    common_TRY_MOVES(UP + LEFT)
                    common_TRY_MOVES(DOWN + LEFT)
                    common_TRY_MOVES(UP + RIGHT)
                    common_TRY_MOVES(DOWN + RIGHT)
                    if ((piece & protocol_PIECE_MASK) == protocol_BISHOP) break;
                }
                case protocol_ROOK: {
                    common_TRY_MOVES(UP)
                    common_TRY_MOVES(DOWN)
                    common_TRY_MOVES(RIGHT)
                    common_TRY_MOVES(LEFT)
                    break;
                }
                case protocol_KNIGHT: {
                    testIndex = index + 2 * LEFT + DOWN;
                    common_TRY_MOVE
                    testIndex = index + 2 * RIGHT + DOWN;
                    common_TRY_MOVE
                    testIndex = index + 2 * RIGHT + UP;
                    common_TRY_MOVE
                    testIndex = index + 2 * LEFT + UP;
                    common_TRY_MOVE
                    testIndex = index + 2 * DOWN + LEFT;
                    common_TRY_MOVE
                    testIndex = index + 2 * DOWN + RIGHT;
                    common_TRY_MOVE
                    testIndex = index + 2 * UP + RIGHT;
                    common_TRY_MOVE
                    testIndex = index + 2 * UP + LEFT;
                    common_TRY_MOVE
                    break;
                }
                case protocol_PAWN: {
                    testIndex = index + UP;
                    if (common_board[testIndex] == 0) {
                        common_moves[common_numMoves++] = (struct common_move) {
                            .from = index,
                            .to = testIndex
                        };
                    }

                    testIndex = index + UP + LEFT;
                    if (common_board[testIndex] != 0 && (common_board[testIndex] & protocol_WHITE_FLAG) == 0) {
                        common_moves[common_numMoves++] = (struct common_move) {
                            .from = index,
                            .to = testIndex
                        };
                    }

                    testIndex = index + UP + RIGHT;
                    if (common_board[testIndex] != 0 && (common_board[testIndex] & protocol_WHITE_FLAG) == 0) {
                        common_moves[common_numMoves++] = (struct common_move) {
                            .from = index,
                            .to = testIndex
                        };
                    }
                    break;
                }
                default: UNREACHABLE;
            }
        }
    }
}

#undef UP
#undef DOWN
#undef RIGHT
#undef LEFT
#include "recursive.h"
#include "common.h"
#include "protocol.h"

#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

static int32_t recursive_evaluateWhiteMove(int32_t from, int32_t to, int32_t remainingDepth);

#define recursive_DEPTH 5

static const int32_t recursive_pieceValues[7] = {
    0, // No piece
    1, // Pawn
    3, // Bishop
    3, // Knight
    5, // Rook
    9, // Queen
};

static int32_t whitePieces[16];
static int32_t whiteNumPieces;
static int32_t blackPieces[16];
static int32_t blackNumPieces;

static int32_t pieceIndices[144];

static void recursive_initPieces(void) {
    memset(&pieceIndices, 0, sizeof(pieceIndices));
    whiteNumPieces = 0;
    blackNumPieces = 0;
    for (int32_t i = 0; i < 64; ++i) {
        int32_t index = common_CONVERT_INDEX(i);
        uint8_t piece = common_board[index];
        if ((piece & protocol_WHITE_FLAG) != 0) {
            pieceIndices[index] = whiteNumPieces;
            whitePieces[whiteNumPieces++] = index;
        } else if ((piece & protocol_BLACK_FLAG) != 0) {
            pieceIndices[index] = blackNumPieces;
            blackPieces[blackNumPieces++] = index;
        }
    }
}

#define recursive_TRY_WHITE_MOVE \
    if ((common_board[testIndex] & protocol_WHITE_FLAG) == 0) { \
        int32_t SCORE = recursive_evaluateWhiteMove(i, testIndex, remainingDepth - 1); \
        if (SCORE > best) best = SCORE; \
    }

#define recursive_TRY_WHITE_MOVES(OFFSET) \
    for (testIndex = index + OFFSET; (common_board[testIndex] & protocol_WHITE_FLAG) == 0; testIndex += OFFSET) { \
        int32_t SCORE = recursive_evaluateWhiteMove(i, testIndex, remainingDepth - 1); \
        if (SCORE > best) best = SCORE; \
        if (common_board[testIndex] != 0) break; \
    }

#define recursive_TRY_BLACK_MOVE \
    if ((common_board[testIndex] & protocol_BLACK_FLAG) == 0) { \
        int32_t SCORE = recursive_evaluateBlackMove(i, testIndex, remainingDepth - 1); \
        if (SCORE < best) best = SCORE; \
    }

#define recursive_TRY_BLACK_MOVES(OFFSET) \
    for (testIndex = index + OFFSET; (common_board[testIndex] & protocol_BLACK_FLAG) == 0; testIndex += OFFSET) { \
        int32_t SCORE = recursive_evaluateBlackMove(i, testIndex, remainingDepth - 1); \
        if (SCORE < best) best = SCORE; \
        if (common_board[testIndex] != 0) break; \
    }

#define UP 12
#define DOWN (-12)
#define RIGHT 1
#define LEFT (-1)

static int32_t recursive_evaluateBlackMove(int32_t blackPiece, int32_t to, int32_t remainingDepth) {
    int32_t from = blackPieces[blackPiece];

    uint8_t takenPiece = common_board[to];
    if (takenPiece == (protocol_KING | protocol_WHITE_FLAG)) return -10000;

    int32_t pieceValue = recursive_pieceValues[takenPiece & protocol_PIECE_MASK];
    uint8_t originalPiece = common_board[from];
    if (originalPiece == (protocol_PAWN | protocol_BLACK_FLAG) && to < common_CONVERT_INDEX(8)) {
        pieceValue += 8;
        if (remainingDepth == 0) return -pieceValue;
        common_board[from] = protocol_QUEEN | protocol_BLACK_FLAG;
    } else if (remainingDepth == 0) return -pieceValue;

    int32_t pieceIndex = pieceIndices[to];
    if (takenPiece != 0) {
        whitePieces[pieceIndex] = whitePieces[--whiteNumPieces];
        pieceIndices[whitePieces[pieceIndex]] = pieceIndex;
    }

    // Find best response for white.
    common_board[to] = common_board[from];
    common_board[from] = 0;
    pieceIndices[to] = blackPiece;
    blackPieces[blackPiece] = to;
    int32_t best = INT32_MIN;
    for (int32_t i = 0; i < whiteNumPieces; ++i) {
        int32_t index = whitePieces[i];
        uint8_t piece = common_board[index];
        assert(piece & protocol_WHITE_FLAG);

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
                if (common_board[testIndex] == 0) {
                    int32_t moveScore = recursive_evaluateWhiteMove(i, testIndex, remainingDepth - 1);
                    if (moveScore > best) best = moveScore;
                }

                testIndex = index + UP + LEFT;
                if (common_board[testIndex] != 0 && (common_board[testIndex] & protocol_WHITE_FLAG) == 0) {
                    int32_t moveScore = recursive_evaluateWhiteMove(i, testIndex, remainingDepth - 1);
                    if (moveScore > best) best = moveScore;
                }

                testIndex = index + UP + RIGHT;
                if (common_board[testIndex] != 0 && (common_board[testIndex] & protocol_WHITE_FLAG) == 0) {
                    int32_t moveScore = recursive_evaluateWhiteMove(i, testIndex, remainingDepth - 1);
                    if (moveScore > best) best = moveScore;
                }
                break;
            }
            default: UNREACHABLE;
        }
    }
    if (takenPiece != 0) {
        pieceIndices[whitePieces[pieceIndex]] = whiteNumPieces;
        whitePieces[whiteNumPieces++] = whitePieces[pieceIndex];
        whitePieces[pieceIndex] = to;
    }
    common_board[to] = takenPiece;
    pieceIndices[to] = pieceIndex;
    common_board[from] = originalPiece;
    blackPieces[blackPiece] = from;
    return best - pieceValue;
}

static int32_t recursive_evaluateWhiteMove(int32_t whitePiece, int32_t to, int32_t remainingDepth) {
    int32_t from = whitePieces[whitePiece];

    uint8_t takenPiece = common_board[to];
    if (takenPiece == (protocol_KING | protocol_BLACK_FLAG)) return 10000;

    int32_t pieceValue = recursive_pieceValues[takenPiece & protocol_PIECE_MASK];
    uint8_t originalPiece = common_board[from];
    if (originalPiece == (protocol_PAWN | protocol_WHITE_FLAG) && to >= common_CONVERT_INDEX(56)) {
        pieceValue += 8;
        if (remainingDepth == 0) return pieceValue;
        common_board[from] = protocol_QUEEN | protocol_WHITE_FLAG;
    } else if (remainingDepth == 0) return pieceValue;

    int32_t pieceIndex = pieceIndices[to];
    if (takenPiece != 0) {
        blackPieces[pieceIndex] = blackPieces[--blackNumPieces];
        pieceIndices[blackPieces[pieceIndex]] = pieceIndex;
    }

    // Find best response for black.
    common_board[to] = common_board[from];
    common_board[from] = 0;
    pieceIndices[to] = whitePiece;
    whitePieces[whitePiece] = to;
    int32_t best = INT32_MAX;
    for (int32_t i = 0; i < blackNumPieces; ++i) {
        int32_t index = blackPieces[i];
        int32_t piece = common_board[index];
        assert(piece & protocol_BLACK_FLAG);

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
                if (common_board[testIndex] == 0) {
                    int32_t moveScore = recursive_evaluateBlackMove(i, testIndex, remainingDepth - 1);
                    if (moveScore < best) best = moveScore;
                }

                testIndex = index + DOWN + LEFT;
                if (common_board[testIndex] != 0 && (common_board[testIndex] & protocol_BLACK_FLAG) == 0) {
                    int32_t moveScore = recursive_evaluateBlackMove(i, testIndex, remainingDepth - 1);
                    if (moveScore < best) best = moveScore;
                }

                testIndex = index + DOWN + RIGHT;
                if (common_board[testIndex] != 0 && (common_board[testIndex] & protocol_BLACK_FLAG) == 0) {
                    int32_t moveScore = recursive_evaluateBlackMove(i, testIndex, remainingDepth - 1);
                    if (moveScore < best) best = moveScore;
                }
                break;
            }
            default: UNREACHABLE;
        }
    }
    if (takenPiece != 0) {
        pieceIndices[blackPieces[pieceIndex]] = blackNumPieces;
        blackPieces[blackNumPieces++] = blackPieces[pieceIndex];
        blackPieces[pieceIndex] = to;
    }
    common_board[to] = takenPiece;
    pieceIndices[to] = pieceIndex;
    common_board[from] = originalPiece;
    whitePieces[whitePiece] = from;
    return best + pieceValue;
}

#undef UP
#undef DOWN
#undef RIGHT
#undef LEFT

static int recursive_makeMove(bool isHost, uint8_t *board, int32_t lastMoveFrom, int32_t lastMoveTo, int32_t *moveFrom, int32_t *moveTo) {
    common_transformBoard(isHost, board);
    recursive_initPieces();
    common_dumpBoard();
    common_findMoves();

    printf("Found %d moves\n", common_numMoves);
    if (common_numMoves == 0) return -1;

    int32_t scores[common_MAX_MOVES];

    int32_t bestScore = INT32_MIN;
    int32_t numBestMoves = 0;
    for (int32_t i = 0; i < common_numMoves; ++i) {
        scores[i] = recursive_evaluateWhiteMove(pieceIndices[common_moves[i].from], common_moves[i].to, recursive_DEPTH);
        if (scores[i] > bestScore) {
            bestScore = scores[i];
        }
    }

    for (int32_t i = 0; i < common_numMoves; ++i) {
        if (scores[i] == bestScore) {
            ++numBestMoves;
            printf("Best move: %d->%d (%d)\n", common_CONVERT_INDEX_BACK(common_moves[i].from), common_CONVERT_INDEX_BACK(common_moves[i].to), bestScore);
        }
    }

    int32_t moveIndex = rand() % numBestMoves;
    for (int32_t i = 0, j = 0; i < common_numMoves; ++i) {
        if (scores[i] == bestScore) {
            if (j == moveIndex) {
                *moveFrom = common_CONVERT_INDEX_BACK(common_moves[i].from);
                *moveTo = common_CONVERT_INDEX_BACK(common_moves[i].to);
                return 0;
            }
            ++j;
        }
    }
    return -1;
}

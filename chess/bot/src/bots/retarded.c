#include "bots/retarded.h"

#include <assert.h>

static int retarded_tryMove(bool isHost, uint8_t *board, int32_t fromIndex, int32_t *moveTo, int32_t deltaX, int32_t deltaY) {
    int32_t newX = fromIndex % 8 + deltaX;
    int32_t newY = fromIndex / 8 + deltaY;

    if (newX < 0 || newX > 7 || newY < 0 || newY > 7) return 0;

    int32_t newIndex = newY * 8 + newX;

    uint8_t piece = board[newIndex];
    if (piece == 0 || (bool)(piece & protocol_WHITE_FLAG) != isHost) {
        *moveTo = newIndex;
        return 1;
    }
    return 0;
}

static int retarded_tryMovePawn(bool isHost, uint8_t *board, int32_t fromIndex, int32_t *moveTo, int32_t deltaX, int32_t deltaY) {
    int32_t newX = fromIndex % 8 + deltaX;
    int32_t newY = fromIndex / 8 + deltaY;

    if (newX < 0 || newX > 7 || newY < 0 || newY > 7) return 0;

    int32_t newIndex = newY * 8 + newX;

    uint8_t piece = board[newIndex];
    if (
        (deltaX == 0 && piece == 0) ||
        (deltaX != 0 && (bool)(piece & protocol_WHITE_FLAG) != isHost && piece != 0)
    ) {
        *moveTo = newIndex;
        return 1;
    }
    return 0;
}

static int retarded_makeMove(void *data, bool isHost, uint8_t *board, int32_t lastMoveFrom, int32_t lastMoveTo, int32_t *moveFrom, int32_t *moveTo) {
    for (int32_t i = 0; i < 64; ++i) {
        uint8_t piece = board[i];
        if (piece != 0 && (bool)(piece & protocol_WHITE_FLAG) == isHost) {
            int success = 0;
            *moveFrom = i;
            switch (piece & protocol_PIECE_MASK) {
                case protocol_KING:
                case protocol_QUEEN: {
                    success |= retarded_tryMove(isHost, board, i, moveTo, 0, -1);
                    success |= retarded_tryMove(isHost, board, i, moveTo, 0, 1);
                    success |= retarded_tryMove(isHost, board, i, moveTo, 1, 0);
                    success |= retarded_tryMove(isHost, board, i, moveTo, -1, 0);

                    success |= retarded_tryMove(isHost, board, i, moveTo, -1, -1);
                    success |= retarded_tryMove(isHost, board, i, moveTo, 1, -1);
                    success |= retarded_tryMove(isHost, board, i, moveTo, -1, 1);
                    success |= retarded_tryMove(isHost, board, i, moveTo, 1, 1);
                    break;
                }
                case protocol_BISHOP: {
                    success |= retarded_tryMove(isHost, board, i, moveTo, -1, -1);
                    success |= retarded_tryMove(isHost, board, i, moveTo, 1, -1);
                    success |= retarded_tryMove(isHost, board, i, moveTo, -1, 1);
                    success |= retarded_tryMove(isHost, board, i, moveTo, 1, 1);
                    break;
                }
                case protocol_ROOK: {
                    success |= retarded_tryMove(isHost, board, i, moveTo, 0, -1);
                    success |= retarded_tryMove(isHost, board, i, moveTo, 0, 1);
                    success |= retarded_tryMove(isHost, board, i, moveTo, 1, 0);
                    success |= retarded_tryMove(isHost, board, i, moveTo, -1, 0);
                    break;
                }
                case protocol_KNIGHT: {
                    success |= retarded_tryMove(isHost, board, i, moveTo, -1, 2);
                    success |= retarded_tryMove(isHost, board, i, moveTo, -1, -2);
                    success |= retarded_tryMove(isHost, board, i, moveTo, 1, 2);
                    success |= retarded_tryMove(isHost, board, i, moveTo, 1, -2);

                    success |= retarded_tryMove(isHost, board, i, moveTo, 2, -1);
                    success |= retarded_tryMove(isHost, board, i, moveTo, -2, -1);
                    success |= retarded_tryMove(isHost, board, i, moveTo, 2, 1);
                    success |= retarded_tryMove(isHost, board, i, moveTo, -2, 1);
                    break;
                }
                case protocol_PAWN: {
                    success |= retarded_tryMovePawn(isHost, board, i, moveTo, 0, 1);
                    success |= retarded_tryMovePawn(isHost, board, i, moveTo, 1, 1);
                    success |= retarded_tryMovePawn(isHost, board, i, moveTo, -1, 1);
                    break;
                }
                default: UNREACHABLE;
            }
            if (success) goto madeMove;
        }
    }
    return -1;
    madeMove:
    return 0;
}
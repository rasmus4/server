#include "random.h"
#include "common.h"
#include "protocol.h"

#include <assert.h>
#include <stdlib.h>

static int random_makeMove(bool isHost, uint8_t *board, int32_t lastMoveFrom, int32_t lastMoveTo, int32_t *moveFrom, int32_t *moveTo) {
    common_transformBoard(isHost, board);
    common_dumpBoard();
    common_findMoves();

    printf("Found %d moves\n", common_numMoves);
    if (common_numMoves == 0) return -1;
    int32_t moveIndex = rand() % common_numMoves;
    struct common_move *move = &common_moves[moveIndex];
    *moveFrom = common_CONVERT_INDEX_BACK(move->from);
    *moveTo = common_CONVERT_INDEX_BACK(move->to);
    return 0;
}

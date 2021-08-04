#include "recursive.h"
#include "protocol.h"
#include "timespec.h"

#include <time.h>
#include <stdio.h>
#include <inttypes.h>

#define BOARD(X, Y) main_board[63 - ((Y - 1) * 8 + (X - 1))]
#define WHITE(PIECE) (PIECE | protocol_WHITE_FLAG)
#define BLACK(PIECE) (PIECE | protocol_BLACK_FLAG)

#define TIME(FUNCTION) \
    { \
        clock_gettime(CLOCK_MONOTONIC, &startTime); \
        int STATUS = FUNCTION(false, &main_board[0], 18, 26, &moveFrom, &moveTo); \
        if (STATUS < 0) { \
            printf("%s failed to make move\n", #FUNCTION); \
            return -1; \
        } \
        clock_gettime(CLOCK_MONOTONIC, &endTime); \
        int64_t delta = timespec_toNanoseconds(endTime) - timespec_toNanoseconds(startTime); \
        printf("%s took: %" PRId64 ".%09" PRId64 "\n", #FUNCTION, delta / 1000000000, delta % 1000000000); \
    }


static uint8_t main_board[64];

int main(int argc, char **argv) {
    BOARD(1, 1) = WHITE(protocol_ROOK);
    BOARD(2, 1) = WHITE(protocol_KNIGHT);
    BOARD(3, 1) = WHITE(protocol_BISHOP);
    BOARD(4, 1) = WHITE(protocol_QUEEN);
    BOARD(6, 1) = WHITE(protocol_KING);
    BOARD(8, 1) = WHITE(protocol_ROOK);

    BOARD(1, 2) = WHITE(protocol_PAWN);
    BOARD(2, 2) = WHITE(protocol_PAWN);
    BOARD(6, 2) = WHITE(protocol_PAWN);
    BOARD(7, 2) = WHITE(protocol_BISHOP);
    BOARD(8, 2) = WHITE(protocol_PAWN);

    BOARD(4, 3) = WHITE(protocol_PAWN);
    BOARD(7, 3) = WHITE(protocol_PAWN);

    BOARD(3, 4) = WHITE(protocol_PAWN);
    BOARD(4, 4) = WHITE(protocol_KNIGHT);
    BOARD(5, 4) = WHITE(protocol_PAWN);

    BOARD(1, 5) = BLACK(protocol_QUEEN);
    BOARD(7, 5) = BLACK(protocol_PAWN);

    BOARD(3, 6) = BLACK(protocol_PAWN);
    BOARD(4, 6) = BLACK(protocol_PAWN);

    BOARD(1, 7) = BLACK(protocol_PAWN);
    BOARD(2, 7) = BLACK(protocol_PAWN);
    BOARD(5, 7) = BLACK(protocol_PAWN);
    BOARD(6, 7) = BLACK(protocol_PAWN);
    BOARD(8, 7) = BLACK(protocol_PAWN);

    BOARD(1, 8) = BLACK(protocol_ROOK);
    BOARD(2, 8) = BLACK(protocol_KNIGHT);
    BOARD(3, 8) = BLACK(protocol_BISHOP);
    BOARD(5, 8) = BLACK(protocol_KING);
    BOARD(6, 8) = BLACK(protocol_BISHOP);
    BOARD(7, 8) = BLACK(protocol_KNIGHT);
    BOARD(8, 8) = BLACK(protocol_ROOK);

    struct timespec startTime;
    struct timespec endTime;
    int32_t moveFrom;
    int32_t moveTo;

    TIME(recursive_makeMove)
    return 0;
}

#undef BOARD
#undef WHITE
#undef BLACK
#undef TIME
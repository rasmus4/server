#include "include/chess.h"
#include "include/chessClient.h"
#include "include/protocol.h"
#include "include/server.h"
#include "include/timespec.h"

#include <stdbool.h>
#include <stdlib.h>

static void chessRoom_initBoard(struct chessRoom *self) {
    self->board[0] = self->board[7] = protocol_ROOK  | protocol_WHITE_FLAG;
    self->board[1] = self->board[6] = protocol_KNIGHT  | protocol_WHITE_FLAG;
    self->board[2] = self->board[5] = protocol_BISHOP  | protocol_WHITE_FLAG;
    self->board[3] = protocol_QUEEN  | protocol_WHITE_FLAG;
    self->board[4] = protocol_KING  | protocol_WHITE_FLAG;
    for (int i = 0; i < 8; ++i) self->board[8 + i] = protocol_PAWN  | protocol_WHITE_FLAG;

    for (int i = 16; i < 48; ++i) self->board[i] = protocol_NO_PIECE;

    self->board[56] = self->board[63] = protocol_ROOK;
    self->board[57] = self->board[62] = protocol_KNIGHT;
    self->board[58] = self->board[61] = protocol_BISHOP;
    self->board[59] = protocol_QUEEN;
    self->board[60] = protocol_KING;
    for (int i = 0; i < 8; ++i) self->board[48 + i] = protocol_PAWN;

    self->lastMoveFromIndex = -1;
    self->lastMoveToIndex = -1;
}

static inline int32_t chessRoom_convertIndex(int32_t index, bool hostsPov) {
    return hostsPov ? index : 63 - index;
}

static inline int32_t chessRoom_xyToIndex(int32_t x, int32_t y, bool hostsPov) {
    return chessRoom_convertIndex(y * 8 + x, hostsPov);
}

static inline int32_t chessRoom_lastMoveFromIndex(struct chessRoom *self, bool hostsPov) {
    return chessRoom_convertIndex(self->lastMoveFromIndex, hostsPov);
}

static inline int32_t chessRoom_lastMoveToIndex(struct chessRoom *self, bool hostsPov) {
    return chessRoom_convertIndex(self->lastMoveToIndex, hostsPov);
}

static inline bool chessRoom_diagonalAndFree(struct chessRoom *self, int32_t fromX, int32_t fromY, int32_t toX, int32_t toY, bool hostsPov) {
    if (abs(toX - fromX) != abs(toY - fromY)) return false;
    int32_t signX = toX > fromX ? 1 : -1;
    int32_t signY = toY > fromY ? 1 : -1;
    for (fromX += signX, fromY += signY; fromX != toX; fromX += signX, fromY += signY) {
        if (self->board[chessRoom_xyToIndex(fromX, fromY, hostsPov)] != protocol_NO_PIECE) return false;
    }
    return true;
}

static inline bool chessRoom_straightAndFree(struct chessRoom *self, int32_t fromX, int32_t fromY, int32_t toX, int32_t toY, bool hostsPov) {
    if (toX != fromX && toY == fromY) {
        int signX = toX > fromX ? 1 : -1;
        for (fromX += signX; fromX != toX; fromX += signX) {
            if (self->board[chessRoom_xyToIndex(fromX, fromY, hostsPov)] != protocol_NO_PIECE) return false;
        }
    } else if (toY != fromY && toX == fromX) {
        int signY = toY > fromY ? 1 : -1;
        for (fromY += signY; fromY != toY; fromY += signY) {
            if (self->board[chessRoom_xyToIndex(fromX, fromY, hostsPov)] != protocol_NO_PIECE) return false;
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
    self->secondTimerHandle = 0;
}

static inline void chessRoom_open(struct chessRoom *self, struct chessClient *host, int32_t roomId) {
    self->host.client = host;
    self->roomId = roomId;
}

static inline void chessRoom_close(struct chessRoom *self) {
    self->host.client = NULL;
    self->guest.client = NULL;
    if (self->secondTimerHandle != 0) {
        server_destroyTimer(self->secondTimerHandle);
        self->secondTimerHandle = 0;
    }
}

static int chessRoom_start(struct chessRoom *self, struct chessClient *guest, struct server *server) {
    if (server_createTimer(server, &self->secondTimerHandle) < 0) return -1;

    self->guest.client = guest;
    self->winner = protocol_NO_WIN;
    self->hostsTurn = true;
    chessRoom_initBoard(self);

    self->host.timeSpent = 0;
    self->guest.timeSpent = 0;

    struct timespec currentTimespec;
    clock_gettime(CLOCK_MONOTONIC, &currentTimespec);
    self->host.lastUpdate = timespec_toNanoseconds(currentTimespec);

    struct itimerspec spec = {
        .it_interval.tv_sec = 1,
        .it_value.tv_sec = currentTimespec.tv_sec + 1,
        .it_value.tv_nsec = currentTimespec.tv_nsec
    };
    server_startTimer(self->secondTimerHandle, &spec, true);
    return 0;
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

static bool chessRoom_isMoveValid(struct chessRoom *self, int32_t fromX, int32_t fromY, int32_t toX, int32_t toY, bool hostsPov) {
    if (self->winner != protocol_NO_WIN) return false;

    if (
        fromX > 7 || fromX < 0 ||
        fromY > 7 || fromY < 0 ||
        toX > 7   || toX < 0   ||
        toY > 7   || toY < 0
    ) return false;

    if (fromX == toX && fromY == toY) return false;

    uint8_t piece = chessRoom_pieceAt(self, fromX, fromY, hostsPov);
    if (piece == protocol_NO_PIECE) return false;
    bool hostsPiece = piece & protocol_WHITE_FLAG;
    if (hostsPiece != hostsPov) return false;

    uint8_t destPiece = chessRoom_pieceAt(self, toX, toY, hostsPov);
    if (destPiece != protocol_NO_PIECE) {
        bool hostsDestPiece = destPiece & protocol_WHITE_FLAG;
        if (hostsDestPiece == hostsPov) return false; // Can't take ur own pieces.
    }

    switch (piece & protocol_PIECE_MASK) {
        case protocol_KING: return chessRoom_distance(fromX, fromY, toX, toY) == 1;
        case protocol_QUEEN: return (
            chessRoom_diagonalAndFree(self, fromX, fromY, toX, toY, hostsPov) ||
            chessRoom_straightAndFree(self, fromX, fromY, toX, toY, hostsPov)
        );
        case protocol_BISHOP: return chessRoom_diagonalAndFree(self, fromX, fromY, toX, toY, hostsPov);
        case protocol_ROOK: return chessRoom_straightAndFree(self, fromX, fromY, toX, toY, hostsPov);
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
        default: return false;
    }
}

static void chessRoom_doMove(struct chessRoom *self, int32_t fromX, int32_t fromY, int32_t toX, int32_t toY, bool hostsPov) {
    int32_t fromIndex = chessRoom_xyToIndex(fromX, fromY, hostsPov);

    uint8_t piece = self->board[fromIndex];
    uint8_t destPiece;
    if (toY == 7 && (piece & protocol_PIECE_MASK) == protocol_PAWN) destPiece = protocol_QUEEN | (piece & protocol_WHITE_FLAG); // Promotion
    else destPiece = piece;

    int32_t toIndex = chessRoom_xyToIndex(toX, toY, hostsPov);
    if ((self->board[toIndex] & protocol_PIECE_MASK) == protocol_KING) { // Win
        if (self->board[toIndex] & protocol_WHITE_FLAG) self->winner = protocol_BLACK_WIN;
        else self->winner = protocol_WHITE_WIN;
    }
    self->board[toIndex] = destPiece;
    self->board[fromIndex] = protocol_NO_PIECE;
    self->lastMoveFromIndex = fromIndex;
    self->lastMoveToIndex = toIndex;

    struct timespec currentTimespec;
    clock_gettime(CLOCK_MONOTONIC, &currentTimespec);
    int64_t currentTime = timespec_toNanoseconds(currentTimespec);

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
    struct itimerspec spec = {
        .it_interval.tv_sec = 1,
        .it_value = timespec_fromNanoseconds(currentTime + (int64_t)1000000000 - (timeSpent % 1000000000))
    };
    server_startTimer(self->secondTimerHandle, &spec, true);
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

static inline int64_t chessRoom_timeSpent(struct chessRoom *self, bool hostsPov) {
    if (hostsPov) return self->host.timeSpent;
    return self->guest.timeSpent;
}

static uint8_t chessRoom_pieceAt(struct chessRoom *self, int32_t x, int32_t y, bool hostsPov) {
    return self->board[chessRoom_xyToIndex(x, y, hostsPov)];
}
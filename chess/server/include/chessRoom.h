#pragma once
#include "include/protocol.h"

#include <stdint.h>
#include <time.h>

struct chessClient;
struct server;

struct chessRoom_clientInfo {
    struct chessClient *client; // NULL if none.
    int64_t lastUpdate;
    int64_t timeSpent;
};

struct chessRoom {
    int32_t index;
    int32_t roomId;
    struct chessRoom_clientInfo host;
    struct chessRoom_clientInfo guest;
    int32_t *spectators;
    int32_t numSpectators;
    uint8_t board[64];
    enum protocol_winner winner;
    int32_t lastMoveFromIndex;
    int32_t lastMoveToIndex;
    int secondTimerHandle;
    bool hostsTurn;
};

static inline void chessRoom_create(struct chessRoom *self, int32_t index);

static inline void chessRoom_open(struct chessRoom *self, struct chessClient *host, int32_t roomId);
static inline void chessRoom_close(struct chessRoom *self);
static int chessRoom_start(struct chessRoom *self, struct chessClient *guest, struct server *server);
static int chessRoom_addSpectator(struct chessRoom *self, int32_t clientIndex);
static void chessRoom_removeSpectator(struct chessRoom *self, int32_t clientIndex);

static inline bool chessRoom_isOpen(struct chessRoom *self);
static inline bool chessRoom_isFull(struct chessRoom *self);
static inline bool chessRoom_isHostsTurn(struct chessRoom *self);
static inline enum protocol_winner chessRoom_winner(struct chessRoom *self);
static inline int32_t chessRoom_lastMoveFromIndex(struct chessRoom *self, bool hostsPov);
static inline int32_t chessRoom_lastMoveToIndex(struct chessRoom *self, bool hostsPov);
static inline void chessRoom_updateTimeSpent(struct chessRoom *self, int64_t currentTime);
static inline int64_t chessRoom_timeSpent(struct chessRoom *self, bool hostsPov);

// Validates coordinates.
static bool chessRoom_isMoveValid(struct chessRoom *self, int32_t fromX, int32_t fromY, int32_t toX, int32_t toY, bool hostsPov);

static void chessRoom_doMove(struct chessRoom *self, int32_t fromX, int32_t fromY, int32_t toX, int32_t toY, bool hostsPov);
static uint8_t chessRoom_pieceAt(struct chessRoom *self, int32_t x, int32_t y, bool hostsPov);
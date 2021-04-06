#include "chess/chess.h"

#include <stdbool.h>
#include <stdlib.h>

static void chessRoom_create(struct chessRoom *self, int32_t roomId) {
    self->host = NULL;
    self->guest = NULL;
    self->roomId = roomId;
}

static inline void chessRoom_setHost(struct chessRoom *self, struct chessClient *host) {
    self->host = host;
}

static inline void chessRoom_setGuest(struct chessRoom *self, struct chessClient *guest) {
    self->guest = guest;
}

static inline bool chessRoom_isEmpty(struct chessRoom *self) {
    return !self->host;
}

static inline bool chessRoom_isFull(struct chessRoom *self) {
    return self->guest; 
}
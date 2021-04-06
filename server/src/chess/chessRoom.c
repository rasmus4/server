#include "chess/chess.h"

#include <stdbool.h>
#include <stdlib.h>

static inline void chessRoom_create(struct chessRoom *self, int32_t index) {
    self->index = index;
    self->host = NULL;
    self->guest = NULL;
}

static inline void chessRoom_open(struct chessRoom *self, struct chessClient *host, int32_t roomId) {
    self->host = host;
    self->roomId = roomId;
}

static inline void chessRoom_setGuest(struct chessRoom *self, struct chessClient *guest) {
    self->guest = guest;
}

static inline bool chessRoom_isOpen(struct chessRoom *self) {
    return self->host;
}

static inline bool chessRoom_isFull(struct chessRoom *self) {
    return self->guest; 
}
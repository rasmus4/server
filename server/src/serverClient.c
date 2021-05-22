#include "serverClient.h"

static inline void serverClient_init(struct serverClient *self, int32_t index) {
    self->fd = -1;
    self->index = index;
}

static inline void serverClient_deinit(struct serverClient *self) {
    if (self->fd >= 0) {
        serverClient_close(self);
    }
}

static inline void serverClient_open(struct serverClient *self, int fd) {
    self->fd = fd;
    self->receiveLength = 0;
    self->isWebsocket = false;
}

static inline void serverClient_close(struct serverClient *self) {
    close(self->fd);
    self->fd = -1;
}
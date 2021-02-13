#include "server.h"

static inline void server_client_init(struct server_client *self) {
    self->fd = -1;
}
static inline void server_client_deinit(struct server_client *self) {
    if (self->fd >= 0) {
        server_client_close(self);
    }
}

static inline void server_client_open(struct server_client *self, int fd) {
    self->fd = fd;
    self->receiveLength = 0;
    self->isWebsocket = false;
}

static inline void server_client_close(struct server_client *self) {
    close(self->fd);
    self->fd = -1;
}
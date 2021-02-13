#include "server.h"

static inline void server_callbacks_init(
    struct server_callbacks *self,
    void *data,
    int (*onConnect)(void *data, struct server_client *client),
    int (*onDisconnect)(void *data, struct server_client *client),
    int (*onMessage)(void *data, struct server_client *client, uint8_t *message, int32_t messageLength, bool isText)
) {
    self->data = data;
    self->onConnect = onConnect;
    self->onDisconnect = onDisconnect;
    self->onMessage = onMessage;
}

static inline void server_callbacks_deinit(struct server_callbacks *self) {}
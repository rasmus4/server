#include "server/serverCallbacks.h"

static inline void serverCallbacks_create(
    struct serverCallbacks *self,
    void *data,
    int (*onConnect)(void *data, struct server_client *client),
    void (*onDisconnect)(void *data, struct server_client *client),
    int (*onMessage)(void *data, struct server_client *client, uint8_t *message, int32_t messageLength, bool isText)
) {
    self->data = data;
    self->onConnect = onConnect;
    self->onDisconnect = onDisconnect;
    self->onMessage = onMessage;
}
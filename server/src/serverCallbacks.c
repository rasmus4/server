#include "include/serverCallbacks.h"

static inline void serverCallbacks_create(
    struct serverCallbacks *self,
    void *data,
    int (*onConnect)(void *data, struct serverClient *client),
    void (*onDisconnect)(void *data, struct serverClient *client),
    int (*onMessage)(void *data, struct serverClient *client, uint8_t *message, int32_t messageLength, bool isText),
    void (*onTimer)(void *data, int *timerHandle, uint64_t expirations)
) {
    self->data = data;
    self->onConnect = onConnect;
    self->onDisconnect = onDisconnect;
    self->onMessage = onMessage;
    self->onTimer = onTimer;
}
#pragma once

#include "clientCallbacks.h"

#include <stdint.h>
#include <stdbool.h>

#define client_RECEIVE_BUFFER_SIZE 1024
#define client_SEND_BUFFER_SIZE 1024

struct client_gameState {
    bool wasHostsTurn;
    bool isHost;
};

struct client {
    int socketFd;
    uint8_t receiveBuffer[client_RECEIVE_BUFFER_SIZE];
    uint8_t sendBuffer[client_SEND_BUFFER_SIZE];
    int32_t received;
    struct client_gameState state;
    struct clientCallbacks callbacks;
};

static inline void client_create(struct client *self, struct clientCallbacks *callbacks);
static int client_run(struct client *self, char *address, int32_t port, int32_t roomId);
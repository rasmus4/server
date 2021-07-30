#pragma once

#include <stdint.h>
#include <stdbool.h>

#define client_RECEIVE_BUFFER_SIZE 1024
#define client_SEND_BUFFER_SIZE 1024

typedef int (*client_makeMove)(bool isHost, uint8_t *board, int32_t lastMoveFrom, int32_t lastMoveTo, int32_t *moveFrom, int32_t *moveTo);

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
    client_makeMove makeMove;
};

static inline void client_create(struct client *self, client_makeMove makeMove);
static int client_run(struct client *self, char *address, int32_t port, int32_t roomId);
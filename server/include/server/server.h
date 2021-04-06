#pragma once
#include "fileResponse.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <stdbool.h>

#define server_RECEIVE_BUFFER_SIZE 4096
#define server_MAX_EPOLL_EVENTS 64
#define server_MAX_CLIENTS 256

struct server_client {
    int fd;
    uint8_t receiveBuffer[server_RECEIVE_BUFFER_SIZE];
    int32_t receiveLength;
    int index;
    bool isWebsocket;
};

struct server_callbacks {
    void *data;
    int (*onConnect)(void *data, struct server_client *client); // Non-zero return prevents the connection.
    void (*onDisconnect)(void *data, struct server_client *client);
    int (*onMessage)(void *data, struct server_client *client, uint8_t *message, int32_t messageLength, bool isText); // Non-zero return closes connection.
};

static inline void server_callbacks_create(
    struct server_callbacks *self,
    void *data,
    int (*onConnect)(void *data, struct server_client *client),
    void (*onDisconnect)(void *data, struct server_client *client),
    int (*onMessage)(void *data, struct server_client *client, uint8_t *message, int32_t messageLength, bool isText)
);

struct server {
    int listenSocketFd;
    int epollFd;
    struct epoll_event epollEvents[server_MAX_EPOLL_EVENTS];
    struct server_client clients[server_MAX_CLIENTS];
    struct fileResponse *fileResponses;
    int32_t fileResponsesLength;
    struct server_callbacks callbacks;
    uint8_t scratchSpace[1024];
};

static int server_init(
    struct server *self,
    struct fileResponse *fileResponses,
    int32_t fileResponsesLength,
    struct server_callbacks *callbacks // Copied
);
static inline void server_deinit(struct server *self);
static int server_run(struct server *self);

// Should not be used in a callback for the client in question, return non-zero instead.
// Note: Will instantly call onDisconnect for the client.
static void server_closeClient(struct server *self, struct server_client *client);

static int server_sendWebsocketMessage(struct server *self, struct server_client *client, uint8_t *message, int32_t messageLength, bool isText);
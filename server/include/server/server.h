#pragma once
#include "fileResponse.h"
#include "serverCallbacks.h"

#include <sys/epoll.h>
#include <sys/timerfd.h>
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

struct server {
    int listenSocketFd;
    int epollFd;
    struct epoll_event epollEvents[server_MAX_EPOLL_EVENTS];
    struct server_client clients[server_MAX_CLIENTS];
    struct fileResponse *fileResponses;
    int32_t fileResponsesLength;
    struct serverCallbacks callbacks;
    uint8_t scratchSpace[1024];
};

static int server_init(
    struct server *self,
    struct fileResponse *fileResponses,
    int32_t fileResponsesLength,
    struct serverCallbacks *callbacks // Copied
);
static inline void server_deinit(struct server *self);
static int server_run(struct server *self);

// Should not be used in a callback for the client in question, return non-zero instead.
// Note: Will instantly call onDisconnect for the client.
static void server_closeClient(struct server *self, struct server_client *client);

static int server_sendWebsocketMessage(struct server *self, struct server_client *client, uint8_t *message, int32_t messageLength, bool isText);

// `*timerHandle` will be a negative number.
static int server_createTimer(struct server *self, int *timerHandle);
// See timerfd_settime() for `value`.
static inline int server_setTimer(int timerHandle, struct itimerspec *value);
static inline void server_closeTimer(int timerHandle);
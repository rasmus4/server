#pragma once
#include "fileResponse.h"

#define server_RECEIVE_BUFFER_SIZE 400024
#define server_MAX_EPOLL_EVENTS 64
#define server_MAX_CLIENTS 10

struct server_client {
    int fd;
    uint8_t receiveBuffer[server_RECEIVE_BUFFER_SIZE];
    int32_t receiveLength;
    bool isWebsocket;
};

struct server {
    int listenSocketFd;
    int epollFd;
    struct epoll_event epollEvents[server_MAX_EPOLL_EVENTS];
    struct server_client clients[server_MAX_CLIENTS];
    struct fileResponse *fileResponses;
    int32_t fileResponsesLength;
    uint8_t scratchSpace[1024];
};

static int server_init(struct server *self, struct fileResponse *fileResponses, int32_t fileResponsesLength);
#define server_DEINIT(SELF)
static int server_run(struct server *self);
#pragma once
#include "fileResponse.h"

#include <sys/epoll.h>
#include <unistd.h>
#include <stdbool.h>

#define server_RECEIVE_BUFFER_SIZE 1024
#define server_MAX_EPOLL_EVENTS 64
#define server_MAX_CLIENTS 1024

struct server_client {
    int fd;
    char receiveBuffer[server_RECEIVE_BUFFER_SIZE];
    int receiveLength;
    bool isWebsocket;
};

static inline void server_client_init_invalid(struct server_client *self) {
    self->fd = -1;
}

static inline void server_client_init(struct server_client *self, int fd) {
    self->fd = fd;
    self->receiveLength = 0;
    self->isWebsocket = false;
}
static inline void server_client_deinit(struct server_client *self) {
    if (self->fd >= 0) {
        close(self->fd);
        self->fd = -1;
    }
}

struct server {
    int listenSocketFd;
    int epollFd;
    struct epoll_event epollEvents[server_MAX_EPOLL_EVENTS];
    struct server_client clients[server_MAX_CLIENTS];
    struct fileResponse *fileResponses;
    int fileResponsesLength;
    char scratchSpace[1024];
};

static int server_init(struct server *self, struct fileResponse *fileResponses, int fileResponsesLength);
#define server_DEINIT(SELF)
static int server_run(struct server *self);
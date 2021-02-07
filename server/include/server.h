#pragma once

#include <sys/epoll.h>
#include <unistd.h>

#define server_RECEIVE_BUFFER_SIZE 1024
#define server_MAX_EPOLL_EVENTS 64
#define server_MAX_CLIENTS 1024

struct server_client {
    int fd;
    char receiveBuffer[server_RECEIVE_BUFFER_SIZE];
    int receiveLength;
};
#define server_client_INIT_INVALID(SELF) (SELF).fd = -1
#define server_client_INIT(SELF, FD) \
    (SELF).fd = FD; \
    (SELF).receiveLength = 0
#define server_client_DEINIT(SELF) \
    if ((SELF).fd != -1) { \
        close((SELF).fd); \
        (SELF).fd = -1; \
    }

struct server {
    int listenSocketFd;
    int epollFd;
    struct epoll_event epollEvents[server_MAX_EPOLL_EVENTS];
    struct server_client clients[server_MAX_CLIENTS];
};

static int server_init(struct server *self);
#define server_DEINIT(SELF)
static int server_run(struct server *self);
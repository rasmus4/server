#include "server.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>

static int server_init(struct server *self, struct fileResponse *fileResponses, int fileResponsesLength) {
    self->fileResponses = fileResponses;
    self->fileResponsesLength = fileResponsesLength;

    self->listenSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (self->listenSocketFd < 0) return -1;    

    int enable = 1;
    if (setsockopt(self->listenSocketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) return -2;

    struct sockaddr_in listenAddr;
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_addr.s_addr = INADDR_ANY;
    listenAddr.sin_port = htons(8080);

    if (bind(self->listenSocketFd, (struct sockaddr *)&listenAddr, sizeof(listenAddr)) < 0) return -3;

    if (listen(self->listenSocketFd, 128) < 0) return -4;

    self->epollFd = epoll_create1(0);
    if (self->epollFd < 0) return -5;

    struct epoll_event listenSocketEvent;
    listenSocketEvent.events = EPOLLIN;
    listenSocketEvent.data.ptr = &self->listenSocketFd;
    if (
        epoll_ctl(
            self->epollFd,
            EPOLL_CTL_ADD,
            self->listenSocketFd,
            &listenSocketEvent
        ) < 0
    ) return -6;

    for (int i = 0; i < server_MAX_CLIENTS; ++i) {
        server_client_init_invalid(&self->clients[i]);
    }

    return 0;
}

static int server_acceptSocket(struct server *self) {
    int status = 0;

    int newSocketFd = accept(self->listenSocketFd, NULL, NULL);
    if (newSocketFd < 0) goto cleanup0;

    int currentFlags = fcntl(newSocketFd, F_GETFL, 0);
    if (currentFlags == -1) goto cleanup1;

    if (fcntl(newSocketFd, F_SETFL, currentFlags | O_NONBLOCK) == -1) goto cleanup1;

    // Find empty client spot
    for (int i = 0; i < server_MAX_CLIENTS; ++i) {
        if (self->clients[i].fd < 0) {
            struct epoll_event newSocketEvent;
            newSocketEvent.events = EPOLLIN;
            newSocketEvent.data.ptr = &self->clients[i];

            if (epoll_ctl(self->epollFd, EPOLL_CTL_ADD, newSocketFd, &newSocketEvent) < 0) goto cleanup1;

            server_client_init(&self->clients[i], newSocketFd);
            return 0;
        }
    }
    // Max clients reached
    close(newSocketFd);
    return 1;

    cleanup1:
    close(newSocketFd);
    cleanup0:
    return -1;
}

static int server_findLineEnd(char *buffer, int index, int end) {
    for (; index + 1 < end; ++index) {
        if (buffer[index] == '\r' && buffer[index + 1] == '\n') return index;
    }
    return -1;
}

static int server_handleRequest(struct server *self, struct server_client *client) {
    int lineEnd = server_findLineEnd(client->receiveBuffer, 0, client->receiveLength);
    if (lineEnd < 0) return 1;
#define server_GET_SLASH_LEN 5
    bool isGet = (lineEnd >= server_GET_SLASH_LEN && memcmp(client->receiveBuffer, "GET /", server_GET_SLASH_LEN) == 0);

    int currentLine = lineEnd;
    for (;;) {
        int lineEnd = server_findLineEnd(client->receiveBuffer, currentLine, client->receiveLength);
        if (lineEnd < 0) return 1;

        int lineLength = lineEnd - currentLine;
        if (lineLength == 0) break;
    }

    if (isGet) {
        char *urlStart = &client->receiveBuffer[server_GET_SLASH_LEN];
        int urlLength = 0;
        for (;;) {
            if (urlLength >= (lineEnd - server_GET_SLASH_LEN)) break;
            if (urlStart[urlLength] == ' ') break;
            ++urlLength;
        }
        for (int i = 0; i < self->fileResponsesLength; ++i) {
            if (
                self->fileResponses[i].urlLength == urlLength &&
                memcmp(self->fileResponses[i].url, urlStart, urlLength) == 0
            ) {
                send(client->fd, self->fileResponses[i].response, self->fileResponses[i].responseLength, 0);
                goto foundResponse;
            }
        }
    }
    char *response = "HTTP/1.1 404 Not Found\r\nContent-Length:0\r\n\r\n";
    send(client->fd, response, 44, 0);
    foundResponse:

    server_client_deinit(client);
    return 0;
}

static int server_handleClient(struct server *self, struct server_client *client) {
    int remainingBuffer = server_RECEIVE_BUFFER_SIZE - client->receiveLength;
    if (remainingBuffer <= 0) {
        server_client_deinit(client);
        return -1;
    }

    char *receivePosition = &client->receiveBuffer[client->receiveLength];
    ssize_t recvLength = recv(client->fd, receivePosition, remainingBuffer, 0);
    if (recvLength < 0) {
        server_client_deinit(client);
        return -2;
    } else if (recvLength == 0) {
        server_client_deinit(client);
        return 1;
    } else {
        client->receiveLength += recvLength;
        printf("Got: %.*s\n", client->receiveLength, client->receiveBuffer);
        int status = server_handleRequest(self, client);
    }
    return 0;
}

static int server_run(struct server *self) {
    for (;;) {
        int numEvents = epoll_wait(self->epollFd, self->epollEvents, server_MAX_EPOLL_EVENTS, -1);
        for (int i = 0; i < numEvents; ++i) {
            int eventFd = *((int *)self->epollEvents[i].data.ptr);
            if (eventFd == self->listenSocketFd) {
                int status = server_acceptSocket(self);
                if (status < 0) printf("Error accepting client socket! (%d)\n", status);
                else printf("Accepted client socket! (%d)\n", status);
            } else {
                struct server_client *client = (struct server_client *)self->epollEvents[i].data.ptr;
                int status = server_handleClient(self, client);
                if (status < 0) printf("Error handling client! (%d)\n", status);
                else printf("Handled client! (%d)\n", status);
            }
        }
    }
    return 0;
}
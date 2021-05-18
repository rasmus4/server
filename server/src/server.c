#include "server_client.c"

#include "server.h"
#include "base64.h"
#include "sha1.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define server_WEBSOCKET_ACCEPT_START \
    "HTTP/1.1 101 Switching Protocols\r\n" \
    "Upgrade: websocket\r\n" \
    "Connection: Upgrade\r\n" \
    "Sec-WebSocket-Accept: "
#define server_WEBSOCKET_ACCEPT_START_LEN (34 + 20 + 21 + 22)

static int server_init(
    struct server *self,
    struct fileResponse *fileResponses,
    int32_t fileResponsesLength,
    struct serverCallbacks *callbacks
) {
    self->fileResponses = fileResponses;
    self->fileResponsesLength = fileResponsesLength;
    self->callbacks = *callbacks;

    self->listenSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (self->listenSocketFd < 0) return -1;

    int enable = 1;
    if (setsockopt(self->listenSocketFd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) return -2;
    if (setsockopt(self->listenSocketFd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable)) < 0) return -3;

    struct sockaddr_in listenAddr;
    listenAddr.sin_family = AF_INET;
    listenAddr.sin_addr.s_addr = INADDR_ANY;
    listenAddr.sin_port = htons(8089);

    if (bind(self->listenSocketFd, (struct sockaddr *)&listenAddr, sizeof(listenAddr)) < 0) return -4;

    if (listen(self->listenSocketFd, 128) < 0) return -5;

    self->epollFd = epoll_create1(0);
    if (self->epollFd < 0) return -6;

    struct epoll_event listenSocketEvent = {
        .events = EPOLLIN,
        .data.ptr = &self->listenSocketFd
    };
    if (epoll_ctl(self->epollFd, EPOLL_CTL_ADD, self->listenSocketFd, &listenSocketEvent) < 0) return -7;

    for (int32_t i = 0; i < server_MAX_CLIENTS; ++i) {
        server_client_init(&self->clients[i], i);
    }

    return 0;
}

static inline void server_deinit(struct server *self) {
    for (int32_t i = 0; i < server_MAX_CLIENTS; ++i) {
        server_client_deinit(&self->clients[i]);
    }
}

static int server_acceptSocket(struct server *self) {
    int newSocketFd = accept(self->listenSocketFd, NULL, NULL);
    if (newSocketFd < 0) goto cleanup_none;

    int currentFlags = fcntl(newSocketFd, F_GETFL, 0);
    if (currentFlags == -1) goto cleanup_newSocketFd;

    if (fcntl(newSocketFd, F_SETFL, currentFlags | O_NONBLOCK) == -1) goto cleanup_newSocketFd;

    int enable = 1;
    if (setsockopt(newSocketFd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable)) < 0) goto cleanup_newSocketFd;

    // Find empty client spot
    for (int i = 0; i < server_MAX_CLIENTS; ++i) {
        if (self->clients[i].fd < 0) {
            struct epoll_event newSocketEvent;
            newSocketEvent.events = EPOLLIN;
            newSocketEvent.data.ptr = &self->clients[i];

            if (epoll_ctl(self->epollFd, EPOLL_CTL_ADD, newSocketFd, &newSocketEvent) < 0) goto cleanup_newSocketFd;

            server_client_open(&self->clients[i], newSocketFd);
            return 0;
        }
    }
    // Max clients reached
    close(newSocketFd);
    return 1;

    cleanup_newSocketFd:
    close(newSocketFd);
    cleanup_none:
    return -1;
}

static int32_t server_findLineEnd(uint8_t *buffer, int32_t index, int32_t end) {
    for (; index + 1 < end; ++index) {
        if (buffer[index] == '\r' && buffer[index + 1] == '\n') return index;
    }
    return -1;
}

static int server_sendWebsocketMessage(struct server *self, struct server_client *client, uint8_t *message, int32_t messageLength, bool isText) {
    if (isText) self->scratchSpace[0] = 0x81;
    else self->scratchSpace[0] = 0x82;

    int32_t headerLength;
    if (messageLength < 126) {
        self->scratchSpace[1] = messageLength;
        headerLength = 2;
    } else {
        self->scratchSpace[1] = 126;
        self->scratchSpace[2] = messageLength >> 8;
        self->scratchSpace[3] = messageLength & 0xFF;
        headerLength = 4;
    }

    struct iovec iov[2] = {
        {
            .iov_base = &self->scratchSpace[0],
            .iov_len = headerLength
        }, {
            .iov_base = message,
            .iov_len = messageLength
        }
    };
    struct msghdr msg = {
        .msg_iov = &iov[0],
        .msg_iovlen = 2,
    };
    if (sendmsg(client->fd, &msg, 0) != headerLength + messageLength) return -1;
    return 0;
}

// Returns 1 if the connection should be kept.
static int server_handleWebsocket(struct server *self, struct server_client *client) {
    if (client->receiveLength < 2) return 1; // No space for even the payload length.
    bool fin = client->receiveBuffer[0] & 0x80;
    if (!fin) return -1; // TODO support fragmentation.

    int opcode = client->receiveBuffer[0] & 0x0F;
    bool masking = client->receiveBuffer[1] & 0x80;
    if (!masking) return -2;

    int64_t payloadLength = client->receiveBuffer[1] & 0x7F;
    int32_t maskingKeyIndex;
    if (payloadLength == 126) {
        if (client->receiveLength < 4) return 1;
        payloadLength = (
            ((uint64_t)client->receiveBuffer[2] << 8) |
            ((uint64_t)client->receiveBuffer[3])
        );
        maskingKeyIndex = 4;
    } else if (payloadLength == 127) {
        if (client->receiveLength < 10) return 1;
        payloadLength = (
            ((uint64_t)client->receiveBuffer[2] << 54) |
            ((uint64_t)client->receiveBuffer[3] << 48) |
            ((uint64_t)client->receiveBuffer[4] << 40) |
            ((uint64_t)client->receiveBuffer[5] << 32) |
            ((uint64_t)client->receiveBuffer[6] << 24) |
            ((uint64_t)client->receiveBuffer[7] << 16) |
            ((uint64_t)client->receiveBuffer[8] << 8) |
            ((uint64_t)client->receiveBuffer[9])
        );
        maskingKeyIndex = 10;
    } else maskingKeyIndex = 2;
    int32_t payloadStart = maskingKeyIndex + 4;
    if (client->receiveLength - payloadStart < payloadLength) return 1;

    for (int32_t i = 0; i < payloadLength; ++i) {
        client->receiveBuffer[payloadStart + i] ^= client->receiveBuffer[maskingKeyIndex + (i % 4)];
    }

    switch (opcode) {
        case 0x1:   // Text
        case 0x2: { // Binary
            if (self->callbacks.onMessage(self->callbacks.data, client, &client->receiveBuffer[payloadStart], payloadLength, opcode & 0x1) != 0) return -3;
            break;
        }
        case 0x8: { // Close
            // TODO should probably send a close frame.
            return 0;
        }
        case 0x9: { // Ping
            // TODO reply
            break;
        }
        case 0xA: { // Pong
            // TODO callback?
            break;
        }
        default: { // TODO continuation frames
            return -3;
        }
    }
    client->receiveLength = 0;
    return 1;
}

// Returns 1 if the connection should be kept.
static int server_handleHttpRequest(struct server *self, struct server_client *client) {
    printf("Got: %.*s\n", client->receiveLength, client->receiveBuffer);

    int32_t lineEnd = server_findLineEnd(client->receiveBuffer, 0, client->receiveLength);
    if (lineEnd < 0) return 1;
#define server_GET_SLASH_LEN 5
    bool isGet = (lineEnd >= server_GET_SLASH_LEN && memcmp(client->receiveBuffer, "GET /", server_GET_SLASH_LEN) == 0);
    int32_t websocketKeyStart = 0;
    int32_t websocketKeyLength;

    int32_t currentLine = lineEnd + 2;
    for (;;) {
        int32_t lineEnd = server_findLineEnd(client->receiveBuffer, currentLine, client->receiveLength);
        if (lineEnd < 0) return 1;

        int32_t lineLength = lineEnd - currentLine;
        if (lineLength == 0) break;

        if (lineLength > 18 && memcmp(&client->receiveBuffer[currentLine], "Sec-WebSocket-Key:", 18) == 0) {
            int32_t i = currentLine + 18;
            for (; i < currentLine + lineLength; ++i) {
                if (websocketKeyStart == 0) {
                    if (client->receiveBuffer[i] != ' ') websocketKeyStart = i;
                } else {
                    if (client->receiveBuffer[i] == ' ') break;
                }
            }
            websocketKeyLength = i - websocketKeyStart;
            if (websocketKeyLength != 24) websocketKeyStart = 0;
        }
        currentLine = lineEnd + 2;
    }

    if (isGet) {
        uint8_t *urlStart = &client->receiveBuffer[server_GET_SLASH_LEN];
        int32_t urlLength = 0;
        for (;;) {
            if (urlLength >= (lineEnd - server_GET_SLASH_LEN)) break;
            if (urlStart[urlLength] == ' ') break;
            ++urlLength;
        }

        if (websocketKeyStart != 0) {
            // Respond to websocket upgrade request.
            memcpy(self->scratchSpace, &client->receiveBuffer[websocketKeyStart], 24);
            memcpy(&self->scratchSpace[24], "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", 36);

            SHA1Context sha1Context;
            SHA1Reset(&sha1Context);
            SHA1Input(&sha1Context, self->scratchSpace, 24 + 36);
            SHA1Result(&sha1Context, &self->scratchSpace[512]);

            memcpy(&self->scratchSpace[0], server_WEBSOCKET_ACCEPT_START, server_WEBSOCKET_ACCEPT_START_LEN);
            int32_t base64Len = base64_encode(&self->scratchSpace[512], SHA1HashSize, &self->scratchSpace[server_WEBSOCKET_ACCEPT_START_LEN]);
            memcpy(&self->scratchSpace[server_WEBSOCKET_ACCEPT_START_LEN + base64Len], "\r\n\r\n", 4);

            printf("Full Response: %.*s", (int)(server_WEBSOCKET_ACCEPT_START_LEN + base64Len + 4), self->scratchSpace);
            client->receiveLength = 0;

            int32_t len = server_WEBSOCKET_ACCEPT_START_LEN + base64Len + 4;
            if (send(client->fd, self->scratchSpace, len, MSG_NOSIGNAL) != len) return -1;
            if (self->callbacks.onConnect(self->callbacks.data, client) != 0) return -2;
            // Only set this if the callback accepts the new connection.
            client->isWebsocket = true;
            return 1;
        }

        // Respond to normal GET request.
        for (int32_t i = 0; i < self->fileResponsesLength; ++i) {
            if (
                self->fileResponses[i].urlLength == urlLength &&
                memcmp(self->fileResponses[i].url, urlStart, urlLength) == 0
            ) {
                if (send(client->fd, self->fileResponses[i].response, self->fileResponses[i].responseLength, MSG_NOSIGNAL) != self->fileResponses[i].responseLength) return -3;
                return 0;
            }
        }
    }
    // We don't have any useful response, just send 404.
    if (send(client->fd, "HTTP/1.1 404 Not Found\r\nContent-Length:0\r\n\r\n", 44, MSG_NOSIGNAL) != 44) return -4;
    return 0;
}

static void server_closeClient(struct server *self, struct server_client *client) {
    if (client->isWebsocket) self->callbacks.onDisconnect(self->callbacks.data, client);
    server_client_close(client);
}

static int server_handleClient(struct server *self, struct server_client *client) {
    int32_t remainingBuffer = server_RECEIVE_BUFFER_SIZE - client->receiveLength;
    if (remainingBuffer <= 0) {
        server_closeClient(self, client);
        return -1;
    }

    uint8_t *receivePosition = &client->receiveBuffer[client->receiveLength];
    ssize_t recvLength = recv(client->fd, receivePosition, remainingBuffer, 0);
    if (recvLength < 0) {
        server_closeClient(self, client);
        return -2;
    } else if (recvLength == 0) {
        server_closeClient(self, client);
        return 1;
    } else {
        client->receiveLength += recvLength;

        int status;
        if (client->isWebsocket) status = server_handleWebsocket(self, client);
        else status = server_handleHttpRequest(self, client);

        if (status != 1) { // Connection no longer in progress.
            server_closeClient(self, client);
            if (status < 0) return -3;
        }
    }
    return 0;
}

static int server_createTimer(struct server *self, int *timerHandle) {
    int fd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (fd < 0) return -1;

    struct epoll_event timerEvent = {
        .events = EPOLLIN,
        .data.ptr = timerHandle
    };
    if (epoll_ctl(self->epollFd, EPOLL_CTL_ADD, fd, &timerEvent) < 0) return -2;

    *timerHandle = -fd; // Fd 0 is reserved for stdin, so this always gives a negative number.
    return 0;
}

static inline void server_startTimer(int timerHandle, struct itimerspec *value, bool absolute) {
    int flags = absolute ? TFD_TIMER_ABSTIME : 0;
    timerfd_settime(-timerHandle, flags, value, NULL);
}

static inline void server_stopTimer(int timerHandle) {
    struct itimerspec value = {0};
    timerfd_settime(-timerHandle, 0, &value, NULL);
}

static inline void server_destroyTimer(int timerHandle) {
    close(-timerHandle);
}

static int server_run(struct server *self) {
    for (;;) {
        struct epoll_event event;
        if (epoll_wait(self->epollFd, &event, 1, -1) < 0) return -1;
        int eventFd = *((int *)event.data.ptr);
        if (eventFd < 0) {
            uint64_t expirations;
            if (read(-eventFd, &expirations, 8) <= 0) return -2;
            self->callbacks.onTimer(self->callbacks.data, event.data.ptr, expirations);
        } else if (eventFd == self->listenSocketFd) {
            int status = server_acceptSocket(self);
            if (status < 0) printf("Error accepting client socket! (%d)\n", status);
            else printf("Accepted client socket! (%d)\n", status);
        } else {
            struct server_client *client = (struct server_client *)event.data.ptr;
            int status = server_handleClient(self, client);
            if (status < 0) printf("Error handling client! (%d)\n", status);
        }
    }
    return 0;
}
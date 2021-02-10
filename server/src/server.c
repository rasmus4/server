#include "server.h"
#include "base64.h"
#include "sha1.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <fcntl.h>

#define server_WEBSOCKET_ACCEPT_START \
    "HTTP/1.1 101 Switching Protocols\r\n" \
    "Upgrade: websocket\r\n" \
    "Connection: Upgrade\r\n" \
    "Sec-WebSocket-Accept: "
#define server_WEBSOCKET_ACCEPT_START_LEN (34 + 20 + 21 + 22)

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

static inline void server_client_setIsWebsocket(struct server_client *self) {
    self->isWebsocket = true;
}

static inline void server_client_setReceiveLength(struct server_client *self, int receiveLength) {
    self->receiveLength = receiveLength;
}

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

static int server_sendWebsocket(struct server *self, struct server_client *client, char *message, int messageLength) {
    if (messageLength >= 1000) return -1;
    // TODO opcode?
    self->scratchSpace[0] = 0x80 | 0x01;
    int payloadIndex;
    if (messageLength < 126) {
        self->scratchSpace[1] = messageLength;
        payloadIndex = 2;
    } else {
        self->scratchSpace[1] = 126;
        self->scratchSpace[2] = messageLength >> 8;
        self->scratchSpace[3] = messageLength & 0xFF;
        payloadIndex = 4;
    }
    memcpy(&self->scratchSpace[payloadIndex], message, messageLength);

    int totalLen = payloadIndex + messageLength;
    if (send(client->fd, self->scratchSpace, totalLen, 0) != totalLen) return -1;
    
    return 0;
}

static int server_handleWebsocket(struct server *self, struct server_client *client) {
    if (client->receiveLength < 2) return 1; // No space for even the payload length.
    bool fin = client->receiveBuffer[0] & 0x80;
    if (!fin) return -1; // TODO support fragmentation.

    int opcode = client->receiveBuffer[0] & 0x0F;
    bool masking = client->receiveBuffer[1] & 0x80;
    if (!masking) return -1;

    long long payloadLen = client->receiveBuffer[1] & 0x7F;
    int maskingKeyIndex;
    if (payloadLen == 126) {
        if (client->receiveLength < 4) return 1;
        payloadLen = (
            ((unsigned long long)((unsigned char)client->receiveBuffer[2]) << 8) |
            ((unsigned long long)(unsigned char)client->receiveBuffer[3])
        );
        maskingKeyIndex = 4;
    } else if (payloadLen == 127) {
        if (client->receiveLength < 10) return 1;
        payloadLen = (
            ((unsigned long long)((unsigned char)client->receiveBuffer[2]) << 54) |
            ((unsigned long long)((unsigned char)client->receiveBuffer[3]) << 48) |
            ((unsigned long long)((unsigned char)client->receiveBuffer[4]) << 40) |
            ((unsigned long long)((unsigned char)client->receiveBuffer[5]) << 32) |
            ((unsigned long long)((unsigned char)client->receiveBuffer[6]) << 24) |
            ((unsigned long long)((unsigned char)client->receiveBuffer[7]) << 16) |
            ((unsigned long long)((unsigned char)client->receiveBuffer[8]) << 8) |
            ((unsigned long long)((unsigned char)client->receiveBuffer[9]))
        );
        maskingKeyIndex = 10;
    } else maskingKeyIndex = 2;
    int payloadStart = maskingKeyIndex + 4;
    if (client->receiveLength - payloadStart < payloadLen) return 1;

    for (int i = 0; i < payloadLen; ++i) {
        client->receiveBuffer[payloadStart + i] ^= client->receiveBuffer[maskingKeyIndex + (i % 4)];
    }

    if (opcode == 0x8) {
        // TODO should probably send a close frame.
        server_client_deinit(client);
        return 0;
    }
    printf("Got websocket packet!! %.*s\n", (int)payloadLen, &client->receiveBuffer[payloadStart]);
    server_sendWebsocket(self, client, &client->receiveBuffer[payloadStart], (int)payloadLen);
    server_client_setReceiveLength(client, 0);
    return 0;
}

static int server_handleRequest(struct server *self, struct server_client *client) {
    if (client->isWebsocket) {
        if (server_handleWebsocket(self, client) < 0) {
            server_client_deinit(client);
            return -1;
        }
        
        return 0;
    }
    printf("Got: %.*s\n", client->receiveLength, client->receiveBuffer);

    int lineEnd = server_findLineEnd(client->receiveBuffer, 0, client->receiveLength);
    if (lineEnd < 0) return 1;
#define server_GET_SLASH_LEN 5
    bool isGet = (lineEnd >= server_GET_SLASH_LEN && memcmp(client->receiveBuffer, "GET /", server_GET_SLASH_LEN) == 0);
    int websocketKeyStart = 0;
    int websocketKeyLength;

    int currentLine = lineEnd + 2;
    for (;;) {
        int lineEnd = server_findLineEnd(client->receiveBuffer, currentLine, client->receiveLength);
        if (lineEnd < 0) return 1;

        int lineLength = lineEnd - currentLine;
        if (lineLength == 0) break;

        if (lineLength > 18 && memcmp(&client->receiveBuffer[currentLine], "Sec-WebSocket-Key:", 18) == 0) {
            int i = currentLine + 18;
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
        char *urlStart = &client->receiveBuffer[server_GET_SLASH_LEN];
        int urlLength = 0;
        for (;;) {
            if (urlLength >= (lineEnd - server_GET_SLASH_LEN)) break;
            if (urlStart[urlLength] == ' ') break;
            ++urlLength;
        }

        if (websocketKeyStart != 0) {
            memcpy(self->scratchSpace, &client->receiveBuffer[websocketKeyStart], 24);
            memcpy(&self->scratchSpace[24], "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", 36);

            SHA1Context sha1Context;
            SHA1Reset(&sha1Context);
            SHA1Input(&sha1Context, self->scratchSpace, 24 + 36);
            SHA1Result(&sha1Context, &self->scratchSpace[512]);

            size_t base64Len;
            char *base64Encoded = base64_encode(&self->scratchSpace[512], SHA1HashSize, &base64Len);
            if (base64Encoded == NULL) goto handledRequest;
            
            memcpy(self->scratchSpace, server_WEBSOCKET_ACCEPT_START, server_WEBSOCKET_ACCEPT_START_LEN);
            memcpy(&self->scratchSpace[server_WEBSOCKET_ACCEPT_START_LEN], base64Encoded, server_WEBSOCKET_ACCEPT_START_LEN);
            memcpy(&self->scratchSpace[server_WEBSOCKET_ACCEPT_START_LEN + base64Len], "\r\n\r\n", 4);

            printf("Full Response: %.*s", (int)(server_WEBSOCKET_ACCEPT_START_LEN + base64Len + 4), self->scratchSpace);
            server_client_setIsWebsocket(client);
            server_client_setReceiveLength(client, 0);

            int len = server_WEBSOCKET_ACCEPT_START_LEN + base64Len + 4;
            if (send(client->fd, self->scratchSpace, len, 0) != len) {
                server_client_deinit(client);
                return -2;
            }
            return 0;
        } else {
            for (int i = 0; i < self->fileResponsesLength; ++i) {
                if (
                    self->fileResponses[i].urlLength == urlLength &&
                    memcmp(self->fileResponses[i].url, urlStart, urlLength) == 0
                ) {
                    send(client->fd, self->fileResponses[i].response, self->fileResponses[i].responseLength, 0);
                    goto handledRequest;
                }
            }
        }
    }
    char *response = "HTTP/1.1 404 Not Found\r\nContent-Length:0\r\n\r\n";
    send(client->fd, response, 44, 0);

    handledRequest:
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
        if (server_handleRequest(self, client) < 0) {
            return -3;   
        }
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
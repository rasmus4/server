#include "client.h"
#include "../../server/include/protocol.h"

#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

static const uint8_t client_GET_REQUEST[] = "GET /chess HTTP/1.1\r\nSec-WebSocket-Key: AQIDBAUGBwgJCgsMDQ4PEC==\r\n\r\n";

static inline void client_create(struct client *self, struct clientCallbacks *callbacks) {
    self->received = 0;
    self->callbacks = *callbacks;
}

// Return is same as recv().
static int client_receive(struct client *self) {
    int status = recv(self->socketFd, &self->receiveBuffer[0], client_RECEIVE_BUFFER_SIZE - self->received, 0);
    if (status > 0) self->received += status;
    return status;
}

static int client_receiveWebsocket(struct client *self, uint8_t **payload, int32_t *length) {
    while (self->received < 2) if (client_receive(self) <= 0) return -1;
    // Assumes no mask, no fragmentation and single byte payload length.
    *payload = &self->receiveBuffer[2];
    *length = self->receiveBuffer[1] & 0x7F;
    while (self->received < 2 + *length) if (client_receive(self) <= 0) return -2;
    return 0;
}

static inline void client_ack(struct client *self, int32_t length) {
    self->received -= length;
    memmove(&self->receiveBuffer[0], &self->receiveBuffer[length], self->received);
}

static inline void client_ackWebsocket(struct client *self) {
    client_ack(self, 2 + (self->receiveBuffer[1] & 0x7F));
}

static int client_sendWebsocket(struct client *self, int32_t length) {
    uint8_t frame[6] = { 0x82, 0x80 | length, 0, 0, 0, 0 };

    struct iovec iov[2] = {
        {
            .iov_base = &frame[0],
            .iov_len = 6
        }, {
            .iov_base = &self->sendBuffer[0],
            .iov_len = length
        }
    };
    struct msghdr msg = {
        .msg_iov = &iov[0],
        .msg_iovlen = 2,
    };
    if (sendmsg(self->socketFd, &msg, MSG_NOSIGNAL) != 6 + length) return -1;
    return 0;
}

static int client_onChessUpdate(struct client *self, uint8_t *payload, int32_t length) {
    uint8_t winner = payload[2];
    if (winner != protocol_NO_WIN) return 0; // Game already over.

    bool hostsTurn = payload[1];
    if (hostsTurn == self->state.wasHostsTurn) return 0; // No turn change.
    self->state.wasHostsTurn = hostsTurn;

    if (self->state.isHost != hostsTurn) return 0; // Not our turn.

    int32_t moveFrom;
    int32_t moveTo;
    int status = self->callbacks.makeMove(self->callbacks.data, self->state.isHost, &payload[21], payload[3], payload[4], &moveFrom, &moveTo);
    if (status < 0) {
        printf("Bot failed to make a move: %d\n", status);
        return 0;
    }

    printf(
        "Bot tries to move from %d to %d. (%d, %d)->(%d, %d)\n",
        moveFrom,
        moveTo,
        moveFrom % 8,
        moveFrom / 8,
        moveTo % 8,
        moveTo / 8
    );
    self->sendBuffer[0] = protocol_MOVE;
    self->sendBuffer[1] = moveFrom;
    self->sendBuffer[2] = moveTo;

    if (client_sendWebsocket(self, 3) < 0) return -2;
    return 0;
}

static int client_run(struct client *self, char *address, int32_t port, int32_t roomId) {
    self->state.isHost = roomId < 0;
    self->state.wasHostsTurn = false;

    self->socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (self->socketFd < 0) return -1;

    int enable = 1;
    int status = setsockopt(self->socketFd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
    if (status < 0) {
        status = -2;
        goto cleanup_socket;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    status = inet_pton(AF_INET, address, &serverAddr.sin_addr.s_addr);
    if (status < 0) {
        status = -3;
        goto cleanup_socket;
    }

    status = connect(self->socketFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    if (status < 0) {
        status = -4;
        goto cleanup_socket;
    }

    if (send(self->socketFd, &client_GET_REQUEST[0], sizeof(client_GET_REQUEST) - 1, MSG_NOSIGNAL) != sizeof(client_GET_REQUEST) - 1) {
        status = -5;
        goto cleanup_socket;
    }

    // Wait for response.
    int32_t responseLength;
    for (;;) {
        status = client_receive(self);
        if (status <= 0) {
            status = -6;
            goto cleanup_socket;
        }

        for (int32_t i = 0; i + 3 < self->received; ++i) {
            if (
                self->receiveBuffer[i] == '\r' &&
                self->receiveBuffer[i + 1] == '\n' &&
                self->receiveBuffer[i + 2] == '\r' &&
                self->receiveBuffer[i + 3] == '\n'
            ) {
                responseLength = i + 4; // Assume no body.
                goto gotResponse;
            }
        }
    }
    gotResponse:
    printf("Http response:\n%.*s", responseLength, &self->receiveBuffer[0]);
    client_ack(self, responseLength);

    // Get protocol version.
    uint8_t *payload;
    int32_t length;
    if (
        client_receiveWebsocket(self, &payload, &length) < 0 ||
        length != 4
    ) {
        status = -7;
        goto cleanup_socket;
    }
    int32_t protocolVersion;
    memcpy(&protocolVersion, &payload[0], 4);
    client_ackWebsocket(self);

    if (protocolVersion != protocol_VERSION) {
        status = -8;
        goto cleanup_socket;
    }

    // Run main loop.
    for (;;) {
        if (client_receiveWebsocket(self, &payload, &length) < 0) {
            status = -9;
            goto cleanup_socket;
        }
        switch (payload[0]) {
            case protocol_HOME: {
                printf("Home view!\n");
                if (self->state.isHost) {
                    self->sendBuffer[0] = protocol_CREATE;
                    if (client_sendWebsocket(self, 1) < 0) {
                        status = -10;
                        goto cleanup_socket;
                    }
                } else {
                    self->sendBuffer[0] = protocol_JOIN;
                    memcpy(&self->sendBuffer[1], &roomId, 4);
                    if (client_sendWebsocket(self, 5) < 0) {
                        status = -11;
                        goto cleanup_socket;
                    }
                }
                break;
            }
            case protocol_ROOM: {
                int32_t id;
                memcpy(&id, &payload[1], 4);
                printf("Created room with id: %d\n", id);
                break;
            }
            case protocol_CHESS: {
                status = client_onChessUpdate(self, payload, length);
                if (status < 0) {
                    printf("Chess update failed: %d\n", status);
                    status = -12;
                    goto cleanup_socket;
                }
                break;
            }
        }
        client_ackWebsocket(self);
    }
    status = 0;

    cleanup_socket:
    close(self->socketFd);
    return status;
}
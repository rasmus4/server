#include "chess.h"
#include "protocol.h"
#include "fileResponse.h"
#include "timespec.h"
#include "generatedHtml.h"

#include <stdlib.h>

static int chess_sendClientState(struct chess *self, struct chessClient *chessClient) {
    static uint8_t buffer[chessClient_writeState_MAX];
    int32_t length = chessClient_writeState(chessClient, &buffer[0]);
    if (server_sendWebsocketMessage(&self->server, chessClient->client, &buffer[0], length, false) < 0) return -1;
    return 0;
}

static void chess_createRoom(struct chess *self, struct chessClient *chessClient) {
    struct chessRoom *room = &self->rooms[0];
    // Atleast one room is guaranteed to be empty.
    for (;; ++room) {
        if (!chessRoom_isOpen(room)) break;
    }
    // Note: Relies on server_MAX_CLIENTS being power of 2!
    int randomPart = rand() & ~(server_MAX_CLIENTS - 1);
    chessRoom_open(room, chessClient, randomPart | room->index);
    chessClient_setRoom(chessClient, room);
}

static void chess_leaveRoom(struct chess *self, struct chessClient *chessClient) {
    struct chessRoom *room = chessClient->room;
    if (chessRoom_isFull(room)) {
        struct chessClient *opponent = chessClient_isHost(chessClient) ? room->guest.client : room->host.client;
        chessClient_unsetRoom(opponent);
        if (chess_sendClientState(self, opponent) < 0) {
            server_closeClient(&self->server, opponent->client);
        }
    }
    chessClient_unsetRoom(chessClient);
    chessRoom_close(room);
}

static int chess_handleCreate(struct chess *self, struct chessClient *chessClient, uint8_t *message, int32_t messageLength) {
    if (chessClient_inRoom(chessClient)) return -1;
    chess_createRoom(self, chessClient);
    if (chess_sendClientState(self, chessClient) < 0) return -2;
    return 0;
}

static int chess_handleJoin(struct chess *self, struct chessClient *chessClient, uint8_t *message, int32_t messageLength) {
    if (messageLength != 5) return -1;
    if (chessClient_inRoom(chessClient)) return -2;
    int32_t roomId;
    memcpy(&roomId, &message[1], 4);

    struct chessRoom *room = &self->rooms[0];
    struct chessRoom *roomsEnd = &self->rooms[server_MAX_CLIENTS];
    for (; room != roomsEnd; ++room) {
        if (
            chessRoom_isOpen(room) &&
            room->roomId == roomId
        ) goto found;
    }
    return 0; // Doesn't exist.
    found:
    if (chessRoom_isFull(room)) return 0; // Already full.

    if (chessRoom_start(room, chessClient, &self->server) < 0) return 0; // Not the client's fault.
    chessClient_setRoom(chessClient, room);

    if (chess_sendClientState(self, room->host.client) < 0) {
        server_closeClient(&self->server, room->host.client->client);
    }
    if (chess_sendClientState(self, chessClient) < 0) return -3;
    return 0;
}

static int chess_handleMove(struct chess *self, struct chessClient *chessClient, uint8_t *message, int32_t messageLength) {
    if (messageLength != 5) return -1;
    if (!chessClient_inRoom(chessClient)) return -2;

    struct chessRoom *room = chessClient->room;
    if (!chessRoom_isFull(room)) return -3;

    bool isHost = chessClient_isHost(chessClient);
    if (isHost != chessRoom_isHostsTurn(chessClient->room)) return 0; // Not players turn.

    int32_t fromX = message[1];
    int32_t fromY = message[2];
    int32_t toX = message[3];
    int32_t toY = message[4];

    if (chessRoom_isMoveValid(room, fromX, fromY, toX, toY, isHost)) {
        chessRoom_doMove(room, fromX, fromY, toX, toY, isHost);

        struct chessClient *opponent = isHost ? room->guest.client : room->host.client;
        if (chess_sendClientState(self, opponent) < 0) {
            server_closeClient(&self->server, opponent->client);
        }
        if (chess_sendClientState(self, chessClient) < 0) return -4;
    }
    return 0;
}

static int chess_handleBack(struct chess *self, struct chessClient *chessClient, uint8_t *message, int32_t messageLength) {
    if (!chessClient_inRoom(chessClient)) return -1;
    chess_leaveRoom(self, chessClient);
    if (chess_sendClientState(self, chessClient) < 0) return -2;
    return 0;
}

#define SELF ((struct chess *)(self))

static int chess_onConnect(void *self, struct server_client *client) {
    // Complete handshake.
    uint32_t version = protocol_VERSION;
    if (server_sendWebsocketMessage(&SELF->server, client, (uint8_t *)&version, 4, false) < 0) return -1;

    struct chessClient *chessClient = &SELF->clients[client->index];
    chessClient_create(chessClient, client);
    if (chess_sendClientState(SELF, chessClient) < 0) return -2;
    return 0;
}

static void chess_onDisconnect(void *self, struct server_client *client) {
    struct chessClient *chessClient = &SELF->clients[client->index];
    if (!chessClient_inRoom(chessClient)) return;
    chess_leaveRoom(self, chessClient);
}

static int chess_onMessage(void *self, struct server_client *client, uint8_t *message, int32_t messageLength, bool isText) {
    struct chessClient *chessClient = &SELF->clients[client->index];

    if (messageLength < 1) return -1;
    int status;
    switch (message[0]) {
        case protocol_CREATE: {
            status = chess_handleCreate(SELF, chessClient, message, messageLength);
            if (status < 0) printf("Error creating room: %d\n", status);
            break;
        }
        case protocol_JOIN: {
            status = chess_handleJoin(SELF, chessClient, message, messageLength);
            if (status < 0) printf("Error joining room: %d\n", status);
            break;
        }
        case protocol_MOVE: {
            status = chess_handleMove(SELF, chessClient, message, messageLength);
            if (status < 0) printf("Error moving piece: %d\n", status);
            break;
        }
        case protocol_BACK: {
            status = chess_handleBack(SELF, chessClient, message, messageLength);
            if (status < 0) printf("Error going back: %d\n", status);
            break;
        }
        default: {
            status = -1;
            break;
        }
    }
    return status;
}

static void chess_onTimer(void *self, int *timerHandle, uint64_t expirations) {
    struct timespec currentTimespec;
    clock_gettime(CLOCK_MONOTONIC, &currentTimespec);

    struct chessRoom *room = &SELF->rooms[0];
    for (;; ++room) {
        if (timerHandle == &room->secondTimerHandle) break;
    }
    int64_t currentTime = timespec_toNanoseconds(currentTimespec);
    chessRoom_updateTimeSpent(room, currentTime);

    if (chess_sendClientState(SELF, room->host.client) < 0) {
        server_closeClient(&SELF->server, room->host.client->client);
    }
    if (chess_sendClientState(SELF, room->guest.client) < 0) {
        server_closeClient(&SELF->server, room->guest.client->client);
    }
}

#undef SELF

static int chess_initFileResponse(struct chess *self) {
    int status;

    int32_t digits = 1;
    int32_t magnitude = 10;
    while ((int32_t)sizeof(generatedHtml) >= magnitude) {
        ++digits;
        magnitude *= 10;
    }
    char responseHttpStart[] = "HTTP/1.1 200 OK\r\nContent-Length:";
    char responseHttpEnd[] = "\r\n\r\n";
    int32_t responseLength = (sizeof(responseHttpStart) - 1) + digits + (sizeof(responseHttpEnd) - 1) + sizeof(generatedHtml);

    uint8_t *responseBuffer = malloc(responseLength);
    if (!responseBuffer) {
        status = -1;
        goto cleanup_none;
    }

    memcpy(responseBuffer, responseHttpStart, sizeof(responseHttpStart) - 1);

    uint8_t *responsePos = &responseBuffer[sizeof(responseHttpStart) - 1];
    int32_t remainingLength = sizeof(generatedHtml);
    while (magnitude >= 10) {
        magnitude /= 10;
        int32_t digitValue = remainingLength / magnitude;
        remainingLength -= digitValue * magnitude;
        *responsePos = '0' + digitValue;
        ++responsePos;
    }

    memcpy(responsePos, responseHttpEnd, sizeof(responseHttpEnd) - 1);
    responsePos += (sizeof(responseHttpEnd) - 1);

    memcpy(responsePos, generatedHtml, sizeof(generatedHtml));

    fileResponse_create(
        &self->response,
        (uint8_t *)"",
        0,
        responseBuffer,
        responseLength
    );
    status = 0;
    cleanup_none:
    return status;
}

static inline void chess_deinitFileResponse(struct chess *self) {
    free(self->response.response);
}

static int chess_init(struct chess *self) {
    int status;

    for (int i = 0; i < server_MAX_CLIENTS; ++i) {
        chessRoom_create(&self->rooms[i], i);
    }

    if (chess_initFileResponse(self) < 0) {
        status = -1;
        goto cleanup_none;
    }

    struct serverCallbacks callbacks;
    serverCallbacks_create(
        &callbacks,
        self,
        chess_onConnect,
        chess_onDisconnect,
        chess_onMessage,
        chess_onTimer
    );

    if (server_init(&self->server, &self->response, 1, &callbacks) < 0) {
        status = -2;
        goto cleanup_response;
    }
    return 0;

    cleanup_response:
    chess_deinitFileResponse(self);
    cleanup_none:
    return status;
}

static void chess_deinit(struct chess *self) {
    chess_deinitFileResponse(self);
    server_deinit(&self->server);
}

static int chess_run(struct chess *self) {
    return server_run(&self->server, false);
}
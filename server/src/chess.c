#include "chess_client.c"
#include "chess_room.c"

#include "chess.h"

#include "fileResponse.h"
#include "protocol.h"
#include "generatedHtml.h"

#define SELF ((struct chess *)(self))

static int chess_onConnect(void *self, struct server_client *client) {
    // Complete handshake.
    uint32_t version = protocol_VERSION;
    if (server_sendWebsocketMessage(&SELF->server, client, (uint8_t *)&version, 4, false) < 0) return -1;

    struct chess_client *chessClient = &SELF->clients[client->index];
    chess_client_create(chessClient, client);
    if (chess_client_sendState(chessClient, SELF) < 0) return -2;

    printf("onConnect\n");
    return 0;
}

static int chess_onDisconnect(void *self, struct server_client *client) {
    printf("onDisconnect\n");
    return 0;
}

static int chess_onMessage(void *self, struct server_client *client, uint8_t *message, int32_t messageLength, bool isText) {
    printf("onMessage %.*s\n", (int)messageLength, message);
    struct chess_client *chessClient = &SELF->clients[client->index];
    if (chess_client_onMessage(chessClient, message, messageLength) < 0) return -1;
    return 0;
}

static int chess_initFileResponse(struct chess *self) {
    int status;

    int32_t digits = 1;
    int32_t magnitude = 10;
    while (sizeof(generatedHtml) >= magnitude) {
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
    if (chess_initFileResponse(self) < 0) {
        status = -1;
        goto cleanup_none;
    }

    struct server_callbacks callbacks;
    server_callbacks_create(
        &callbacks,
        self,
        chess_onConnect,
        chess_onDisconnect,
        chess_onMessage
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
    return server_run(&self->server);
}
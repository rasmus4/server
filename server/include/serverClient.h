#pragma once

#define serverClientRECEIVE_BUFFER_SIZE 4096

struct ServerClient {
    int fd;
    uint8_t receiveBuffer[serverClientRECEIVE_BUFFER_SIZE];
    int32_t receiveLength;
    int index;
    bool isWebsocket;
};
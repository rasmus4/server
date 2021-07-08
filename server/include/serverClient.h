#pragma once

#define serverClient_RECEIVE_BUFFER_SIZE 4096

struct serverClient {
    int fd;
    uint8_t receiveBuffer[serverClient_RECEIVE_BUFFER_SIZE];
    int32_t receiveLength;
    int32_t index;
    bool isWebsocket;
};
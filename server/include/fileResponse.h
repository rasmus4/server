#pragma once

struct fileResponse {
    uint8_t *url;
    int32_t urlLength;
    uint8_t *response;
    int32_t responseLength;
};

static inline void fileResponse_init(
    struct fileResponse *self,
    uint8_t *url,
    int32_t urlLength,
    uint8_t *response,
    int32_t responseLength
) {
    self->url = url;
    self->urlLength = urlLength;
    self->response = response;
    self->responseLength = responseLength;
}

#define fileResponse_DEINIT(SELF)

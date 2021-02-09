#pragma once

struct fileResponse {
    char *url;
    int urlLength;
    char *response;
    int responseLength;
};

static inline void fileResponse_init(
    struct fileResponse *self,
    char *url,
    int urlLength,
    char *response,
    int responseLength
) {
    self->url = url;
    self->urlLength = urlLength;
    self->response = response;
    self->responseLength = responseLength;
}

#define fileResponse_DEINIT(SELF)

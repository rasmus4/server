#pragma once

#include <stdint.h>

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
);

static inline void fileResponse_deinit(struct fileResponse *self);

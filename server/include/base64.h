#pragma once

#include <stdint.h>

static inline int32_t base64_encodeLength(int32_t inputLength);
static int32_t base64_encode(uint8_t *input, int32_t inputLength, uint8_t *output);

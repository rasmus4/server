#pragma once
#include "include/serverClient.h"

static inline void serverClient_init(struct serverClient *self, int32_t index);
static inline void serverClient_deinit(struct serverClient *self);
static inline void serverClient_open(struct serverClient *self, int fd);
static inline void serverClient_close(struct serverClient *self);
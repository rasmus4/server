#pragma once

#include <stdint.h>
#include <stdbool.h>

static int random_makeMove(bool isHost, uint8_t *board, int32_t lastMoveFrom, int32_t lastMoveTo, int32_t *moveFrom, int32_t *moveTo);
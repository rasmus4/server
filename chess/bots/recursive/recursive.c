#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <assert.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "../../common/include/timespec.h"
#include "../../common/include/protocol.h"
#include "../common/include/common.h"
#include "../client/include/client.h"
#include "include/recursive.h"

#include "src/main.c"
#include "src/recursive.c"
#include "../common/src/common.c"
#include "../client/src/client.c"

#define server_RECEIVE_BUFFER_SIZE 4096
#define server_MAX_CLIENTS 256

struct server {
    int listenSocketFd;
    int epollFd;
    int sha1SocketFd;
    int sha1InstanceFd;
    struct serverClient clients[server_MAX_CLIENTS];
    struct fileResponse *fileResponses;
    int32_t fileResponsesLength;
    struct serverCallbacks callbacks;
    uint8_t scratchSpace[512];
};

static int server_init(
    struct server *self,
    struct fileResponse *fileResponses,
    int32_t fileResponsesLength,
    struct serverCallbacks *callbacks // Copied
);
static inline void server_deinit(struct server *self);
static int server_run(struct server *self, bool busyWaiting);

// Should not be used in a callback for the client in question, return non-zero instead.
// Note: Will instantly call onDisconnect for the client.
static void server_closeClient(struct server *self, struct serverClient *client);

static int server_sendWebsocketMessage(struct server *self, struct serverClient *client, uint8_t *message, int32_t messageLength, bool isText);

// `*timerHandle` will be a negative number.
static int server_createTimer(struct server *self, int *timerHandle);
// See timerfd_settime(), `new_value`.
static inline void server_startTimer(int timerHandle, struct itimerspec *value, bool absolute);
static inline void server_stopTimer(int timerHandle);
static inline void server_destroyTimer(int timerHandle);

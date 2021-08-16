struct serverClient;

struct serverCallbacks {
    void *data;
    int (*onConnect)(void *data, struct serverClient *client); // Non-zero return prevents the connection.
    void (*onDisconnect)(void *data, struct serverClient *client);
    int (*onMessage)(void *data, struct serverClient *client, uint8_t *message, int32_t messageLength, bool isText); // Non-zero return closes connection.
    void (*onTimer)(void *data, int *timerHandle, uint64_t expirations); // `timerHandle` is the same pointer that was passed to `server_createTimer`.
                                                                         // `expirations` is number of expirations since last callback, at least 1.
};

static inline void serverCallbacks_create(
    struct serverCallbacks *self,
    void *data,
    int (*onConnect)(void *data, struct serverClient *client),
    void (*onDisconnect)(void *data, struct serverClient *client),
    int (*onMessage)(void *data, struct serverClient *client, uint8_t *message, int32_t messageLength, bool isText),
    void (*onTimer)(void *data, int *timerHandle, uint64_t expirations)
);
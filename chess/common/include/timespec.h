static inline int64_t timespec_toNanoseconds(struct timespec timespec) {
    return (int64_t)1000000000 * timespec.tv_sec + timespec.tv_nsec;
}

static inline struct timespec timespec_fromNanoseconds(int64_t nanoseconds) {
    return (struct timespec) {
        .tv_sec = nanoseconds / 1000000000,
        .tv_nsec = nanoseconds % 1000000000
    };
}

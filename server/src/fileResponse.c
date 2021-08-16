static inline void fileResponse_create(
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
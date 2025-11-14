#include <stdint.h>

static uint32_t heap_offset = 1024;

__attribute__((export_name("alloc")))
int32_t alloc(int32_t size) {
    uint32_t cur = heap_offset;
    heap_offset += (size + 7) & ~7;
    return (int32_t)cur;
}

__attribute__((export_name("dealloc")))
void dealloc(int32_t ptr, int32_t size) {
    (void)ptr; (void)size;
}

extern char *handler(const char *req);

__attribute__((export_name("handle_request")))
int64_t handle_request(int32_t req_ptr, int32_t req_len) {
    int32_t str_ptr = alloc(req_len + 1);
    char *dst = (char *)(uintptr_t)str_ptr;
    char *src = (char *)(uintptr_t)req_ptr;
    for (int i = 0; i < req_len; i++) dst[i] = src[i];
    dst[req_len] = '\0';

    char *resp = handler((const char *)str_ptr);
    if (!resp) return 0;

    int32_t len = 0;
    while (resp[len] != '\0') len++;

    uint64_t packed = ((uint64_t)(uintptr_t)resp << 32) | (uint32_t)len;
    return (int64_t)packed;
}

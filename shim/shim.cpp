// shim_cpp.cpp
#include <cstdint>
typedef unsigned int u32;
typedef unsigned long long u64;

// Forward declaration (no extern "C")
char* handler(const char* req);

static u32 heap_offset = 1024;

extern "C" int32_t alloc(int32_t size) {
    u32 cur = heap_offset;
    heap_offset += (size + 7) & ~7;
    return (int32_t)cur;
}

extern "C" long long handle_request(int32_t req_ptr, int32_t req_len) {
    const char* req = (const char*)(uintptr_t)req_ptr;
    char* resp = handler(req);
    int len = 0; while (resp[len]) len++;
    return ((long long)(uintptr_t)resp << 32) | len;
}

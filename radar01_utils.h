#ifndef __RADAR01_UTILS_H__
#define __RADAR01_UTILS_H__
#include "linux_common.h"

#define RADAR01_DEBUG_ENABLE 1
#define TRUE 1
#define FALSE 0
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))
#define BILLION 1000000000L
#define FTRACE_TIME_STAMP(id) \
    do {                      \
        syscall(id);          \
    } while (0)
#define debug_print(...)          \
    do {                          \
        if (RADAR01_DEBUG_ENABLE) \
            printf(__VA_ARGS__);  \
    } while (0)

#define errExit(msg) \
    do {             \
        perror(msg); \
        abort();     \
    } while (0)


void debug_hex_dump(char *str, uint8_t *pSrcBufVA, int SrcBufLen);

#endif  // __RADAR01_UTILS_H__
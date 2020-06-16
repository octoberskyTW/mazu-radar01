#ifndef __RADAR01_UTILS_H__
#define __RADAR01_UTILS_H__


#define errExit(msg) \
    do {             \
        perror(msg); \
        abort();     \
    } while (0)


#endif  // __RADAR01_UTILS_H__
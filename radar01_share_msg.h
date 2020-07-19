#ifndef __RADAR01_SHARE_MSG_H__
#define __RADAR01_SHARE_MSG_H__

#include <stdint.h>
#define JSON_SZ 1024

struct radar01_json_entry_t {
    int length;
    char payload[JSON_SZ];
};
#endif  //  __RADAR01_SHARE_MSG_H__

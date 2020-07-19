#ifndef __RADAR01_SHARE_MSG_H__
#define __RADAR01_SHARE_MSG_H__

#include <stdint.h>
struct radar01_json_entry_t {
    int length;
    char payload[1024];
};
#endif  //  __RADAR01_SHARE_MSG_H__

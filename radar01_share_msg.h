#ifndef __RADAR01_SHARE_MSG_H__
#define __RADAR01_SHARE_MSG_H__

#include <stdint.h>

struct radar01_ringbuf_entry_t {
    uint32_t frameNumber;
    uint32_t numDetectedObj;
    float x_pos[32];
    float y_pos[32];
    float z_pos[32];
    float velocity[32];
    int16_t snr[32];
    int16_t noise[32];
};
#endif  //  __RADAR01_SHARE_MSG_H__

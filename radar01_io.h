#ifndef __RADAR01_IO_H__
#define __RADAR01_IO_H__
#include <net/if.h>
#include <stdint.h>

struct radar01_io_block_info {
    char portname[IFNAMSIZ];
    int32_t dev_fd;
};

#endif  //  __RADAR01_IO_H__
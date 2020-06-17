#ifndef __RADAR01_IO_H__
#define __RADAR01_IO_H__
#include <errno.h> /* Error number definitions */
#include <fcntl.h> /* File control definitions */
#include <net/if.h>
#include <stdint.h>
#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>  /* malloc/free */
#include <string.h>  /* String function definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <unistd.h>  /* UNIX standard function definitions */

struct radar01_io_info_t {
    char ifname[IFNAMSIZ];
    int dss_fd;
};
int radar01_io_init(char *ifname, void **priv_data);
void radar01_io_deinit(void **priv_data);
int radar01_data_recv(int fd, uint8_t *rx_buff, int buff_size);
#endif  //  __RADAR01_IO_H__
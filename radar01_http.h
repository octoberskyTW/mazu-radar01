#ifndef __RADAR01_HTTP_H__
#define __RADAR01_HTTP_H__
#include "linux_common.h"
struct radar01_http_info_t {
    struct sockaddr_in http_addr;
    int client_fd;  // http client socket fd
    /* http://49.159.114.50:10002/2020test/2020test?data=[{"x":"0.02","y":"0.01","value":"20"}]
     */
    char sever_url[256];
    int net_port;
};

int radar01_http_socket_init(char *ifname, void **priv_data);
int radar01_http_socket_deinit(void **priv_data);

#endif  // __RADAR01_HTTP_H__
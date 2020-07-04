#ifndef __RADAR01_HTTP_H__
#define __RADAR01_HTTP_H__
#include "linux_common.h"
#include "ringbuffer.h"

struct radar01_http_conn_t {
    int sockfd;  // http client socket fd
    size_t offs;
    int flags;
};

struct radar01_http_user_t {
    struct sockaddr_in http_addr;
    /* http://49.159.114.50:10002/2020test/2020test?data=[{"x":"0.02","y":"0.01","value":"20"}]
     */
    char sever_url[256];
    int net_port;
};

// int radar01_http_socket_init(char *ifname, void **priv_data);
// int radar01_http_socket_deinit(void **priv_data);
int radar01_http_user_init(char *ifname, void *priv_data);
int http_connect_server(int efd,
                        struct radar01_http_conn_t *hc,
                        struct sockaddr_in *http_addr);
int radar01_http_send(int fd, char *tx_buff, int frame_len);
int radar01_http_recv(int fd, char *rx_buff, int buff_size);
int http_ring_dequeue(struct ringbuffer_t *rbuf, void *payload, uint32_t size);

#endif  // __RADAR01_HTTP_H__
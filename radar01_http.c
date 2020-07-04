#include "radar01_http.h"
#include <fcntl.h>
#include <sys/epoll.h>
#include "linux_common.h"
#include "radar01_utils.h"

int http_connect_server(int efd,
                        struct radar01_http_conn_t *hc,
                        struct sockaddr_in *http_addr)
{
    int rc = 0;
    if ((hc->sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
        printf("[%s:%d] Require socket failed. [ERROR]: %s\n", __FUNCTION__,
               __LINE__, strerror(errno));
        rc = -1;
        goto failed_open_socket;
    }
    hc->offs = 0;
    hc->flags = 0;
    int retry_cnt = 0;
    do {
        rc = connect(hc->sockfd, (struct sockaddr *) http_addr,
                     sizeof(struct sockaddr_in));
        if (rc == 0)
            break;
        if (rc < 0 || errno == EAGAIN || errno == EINPROGRESS) {
            retry_cnt++;
            sleep(1);
            fprintf(stderr, "[%s:%d] Connection retry %d errno: %s\n",
                    __FUNCTION__, __LINE__, retry_cnt, strerror(errno));
        }
    } while (retry_cnt < 5);

    if (rc && errno != EINPROGRESS) {
        fprintf(stderr, "[%s:%d] Connect fail at socket(%d). [ERROR]: %s\n",
                __FUNCTION__, __LINE__, hc->sockfd, strerror(errno));
        rc = -1;
        goto failed_connect;
    }

    struct epoll_event reg_evt = {
        .events = EPOLLOUT,  // Send packet first;
        .data.ptr = hc,
    };

    if ((rc = epoll_ctl(efd, EPOLL_CTL_ADD, hc->sockfd, &reg_evt)) < 0) {
        printf("[%s:%d] Fail to control epoll. [ERROR]: %s\n", __FUNCTION__,
               __LINE__, strerror(errno));
        goto failed_connect;
    }
    printf("Connect success !! \n");
    return rc;
failed_connect:
    close(hc->sockfd);
failed_open_socket:
    return rc;
}



int radar01_http_user_init(char *ifname, void *priv_data)
{
    int rc = 0;
    struct radar01_http_user_t *hu = (struct radar01_http_user_t *) priv_data;
    /*Get address and port from strings "192.168.1.1:7788"*/
    strncpy(&hu->sever_url[0], ifname, 256);
    char *saveptr = NULL;
    char delim = ':';
    char *token = strtok_r(&hu->sever_url[0], &delim, &saveptr);
    hu->http_addr.sin_family = AF_INET;
    hu->http_addr.sin_addr.s_addr = inet_addr(token);
    token = strtok_r(NULL, &delim, &saveptr);
    if (token)
        hu->net_port = atoi(token);
    hu->http_addr.sin_port = htons(hu->net_port);
    printf("%s:%d\n", hu->sever_url, hu->net_port);
    return rc;
}

int radar01_http_send(int fd, char *tx_buff, int frame_len)
{
    int offset = 0;
    while (offset < frame_len) {
        int wdlen;
        if ((wdlen = send(fd, tx_buff + offset, frame_len - offset, 0)) < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                wdlen = 0;
            } else {
                fprintf(stderr, "%s: %s\n", __FUNCTION__, strerror(errno));
                return -1;
            }
        }
        offset += wdlen;
    }
    return offset;
}

int radar01_http_recv(int fd, char *rx_buff, int buff_size)
{
    int offset = 0;
    memset(rx_buff, 0, buff_size);
    while (offset < buff_size) {
        int rdlen = 0;
        if ((rdlen = recv(fd, rx_buff + offset, buff_size - offset,
                          MSG_DONTWAIT)) < 0) {
            if (rdlen == -1 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
                rdlen = 0;
                break;
            } else {
                printf("[%s:%d] rdlen = %d, %s\n", __func__, __LINE__, rdlen,
                       strerror(errno));
                break;
            }
        } else if (rdlen == 0) {
            /* socket peer has performed an orderly shutdown */
            break;
        }
        offset += rdlen;
    }
    return offset;
}


int http_ring_dequeue(struct ringbuffer_t *rbuf, void *payload, uint32_t size)
{
    uint8_t *rxcell = NULL;
    rb_pop(rbuf, (void **) &rxcell);
    if (rxcell == NULL)
        goto empty;
    memcpy(payload, rxcell, size);
    radar01_free_mem((void **) &rxcell);
    debug_hex_dump("http_ring_dequeue", payload, size);
    return size;
empty:
    return 0;
}
#if 0
int radar01_http_socket_init(char *ifname, void **priv_data)
{
    int rc = 0;
    struct radar01_http_info_t *hp;
    hp = (struct radar01_http_info_t *) malloc(
        sizeof(struct radar01_http_info_t));
    if (hp == NULL) {
        printf("[%s:%d] Memory allocate fail. [ERROR]: %s\n", __FUNCTION__,
               __LINE__, strerror(errno));
        rc = -1;
        goto failed_malloc;
    }
    *priv_data = hp;
    if ((hp->client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[%s:%d] Require socket failed. [ERROR]: %s\n", __FUNCTION__,
               __LINE__, strerror(errno));
        rc = -1;
        goto failed_open_socket;
    }
    /*Get address and port from strings "192.168.1.1:7788"*/
    strncpy(&hp->sever_url[0], ifname, 256);
    char *saveptr = NULL;
    char delim = ':';
    char *token = strtok_r(&hp->sever_url[0], &delim, &saveptr);
    hp->http_addr.sin_family = AF_INET;
    hp->http_addr.sin_addr.s_addr = inet_addr(token);
    token = strtok_r(NULL, &delim, &saveptr);
    if (token)
        hp->net_port = atoi(token);
    hp->http_addr.sin_port = htons(hp->net_port);
    /* Try to Connect to server*/
    // int setflag = fcntl(hp->client_fd, F_GETFL);
    // setflag = setflag | O_NONBLOCK;
    // fcntl(hp->client_fd, F_SETFL, setflag);

    int retry_cnt = 0;
    do {
        rc = connect(hp->client_fd, (struct sockaddr *) &hp->http_addr,
                     sizeof(hp->http_addr));
        if (rc == 0)
            break;
        if (rc < 0 || errno == EINPROGRESS) {
            retry_cnt++;
            sleep(2);
            printf("[%s:%d] Connection retry %d errno: %s\n", __FUNCTION__,
                   __LINE__, retry_cnt, strerror(errno));
        }
    } while (retry_cnt < 5);

    if (rc < 0) {
        printf("[%s:%d] Connect to %s:%d failed. Abort http init. errno: %s\n",
               __FUNCTION__, __LINE__, hp->sever_url, hp->net_port,
               strerror(errno));
        goto failed_connect;
    }
    printf("[%s:%d] HTTP Connection Create on %s\n", __FUNCTION__, __LINE__,
           ifname);
    return rc;
failed_connect:
    close(hp->client_fd);
failed_open_socket:
    if (hp) {
        free(hp);
        *priv_data = NULL;
    }
failed_malloc:
    return rc;
}

int radar01_http_socket_deinit(void **priv_data)
{
    struct radar01_http_info_t *hp = *priv_data;
    if (hp) {
        close(hp->client_fd);
        printf("Closing %s:%d \n", hp->sever_url, hp->net_port);
        free(hp);
        *priv_data = NULL;
    }
}
#endif
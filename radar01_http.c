#include "radar01_http.h"
#include "linux_common.h"

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
    int retry_cnt = 0;
    do {
        rc = connect(hp->client_fd, (struct sockaddr *) &hp->http_addr,
                     sizeof(hp->http_addr));
        if (rc < 0) {
            retry_cnt++;
            sleep(2);
            printf("[%s:%d] Connection retry %d errno: %s\n", __FUNCTION__,
                   __LINE__, retry_cnt, strerror(errno));
        }
    } while (rc && errno == EINTR && retry_cnt < 5);

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
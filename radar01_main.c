#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <sys/epoll.h>
#include "radar01_http.h"
#include "radar01_io.h"
#include "radar01_tlv.h"
#include "radar01_utils.h"
#include "vender/dpif_pointcloud.h"
#include "vender/mmw_output.h"


#define EPOLL_SIZE 256
#define BUF_SIZE 512
#define EPOLL_RUN_TIMEOUT 1000

#define MSG_HEADER_LENS (sizeof(MmwDemo_output_message_header))

static void server_err(const char *str)
{
    perror(str);
    exit(-1);
}

struct radar01_io_info_t *iwr1642_dss_blk;
struct radar01_io_info_t *iwr1642_mss_blk;
struct radar01_http_info_t *arstu_server_blk;

static int exit_i = 0;
static void signal_exit(int signal)
{
    (void) signal;
    exit_i++;
}

static int radar01_receive_process(int fd,
                                   uint8_t *rx_buff,
                                   int max_size,
                                   int epoll_fd,
                                   struct epoll_event *ev_recv)
{
    int offset = 0;
    MmwDemo_output_message_header msg_header = {0};
    static uint16_t magicWord[4] = {0x0102, 0x0304, 0x0506, 0x0708};
    offset = radar01_data_recv(fd, rx_buff, MSG_HEADER_LENS);
    debug_hex_dump("DSS Header ", rx_buff, offset);
    if (memcmp(rx_buff, magicWord, 8) != 0) {
        printf("[%s] Header Magic number not match Drop it !!\n", __FUNCTION__);
        return -1;
    }
    memcpy(&msg_header, rx_buff, MSG_HEADER_LENS);
    max_size = (int) msg_header.totalPacketLen < max_size
                   ? (int) msg_header.totalPacketLen
                   : max_size;
    while (offset < max_size) {
        int rdlen = 0;
        int epoll_events_count;
        if ((epoll_events_count =
                 epoll_wait(epoll_fd, ev_recv, 1024, EPOLL_RUN_TIMEOUT)) < 0)
            printf("[%s:%d] Remain packet received fail. [ERROR]: %s\n",
                   __FUNCTION__, __LINE__, strerror(errno));

        for (int i = 0; i < epoll_events_count; i++) {
            if (ev_recv[i].data.fd != fd)
                continue;
            rdlen = radar01_data_recv(fd, rx_buff + offset, max_size - offset);
            offset += rdlen;
        }
    }  // while
    debug_hex_dump("DSS Header+Content ", rx_buff, max_size);
    if (offset != max_size)
        printf(
            "[%s:%d] [WARNING] Received pkt size not patch. %d, expected %d \n",
            __FUNCTION__, __LINE__, offset, max_size);
    return offset;
}

struct device_worker_info {
    int epoll_fd;
    struct epoll_event ev_recv[EPOLL_SIZE];
    int dss_fd;
    uint8_t data_buff[1024];
    struct radar01_message_data_t Cartesian;
};

void *device_worker(void *v_param)
{
    struct device_worker_info *winfo;
    winfo = (struct device_worker_info *) v_param;
    while (!exit_i) {
        int epoll_events_count;
        if ((epoll_events_count =
                 epoll_wait(winfo->epoll_fd, &winfo->ev_recv[0], EPOLL_SIZE,
                            EPOLL_RUN_TIMEOUT)) < 0)
            server_err("Fail to wait epoll");
        // printf("epoll event count: %d\n", epoll_events_count);
        //        clock_t start_time = clock();
        for (int i = 0; i < epoll_events_count; i++) {
            /* EPOLLIN event for listener (new client connection) */
            if (winfo->ev_recv[i].data.fd == winfo->dss_fd) {
                int size = radar01_receive_process(
                    winfo->dss_fd, &winfo->data_buff[0], 1024, winfo->epoll_fd,
                    &winfo->ev_recv[0]);
                if (size > 0) {
                    radar01_process_message(&winfo->data_buff[0], size,
                                            &winfo->Cartesian);
                    radar01_Cartesian_info_dump(&winfo->Cartesian);
                }

            } else {
                // /* EPOLLIN event for others (new incoming message from
                // client)
                //  */
                // if (handle_message_from_client(events[i].data.fd, &list) < 0)
                //     server_err("Handle message from client", &list);
            }
        }
    }
    return NULL;
}

struct http_worker_info {
    int epoll_fd;
    struct epoll_event ev_recv[EPOLL_SIZE];
    int http_fd;
    uint8_t data_buff[1024];
};

/* ToDo: Use epoll socket */
void *http_worker(void *v_param)
{
    struct http_worker_info *winfo;
    winfo = (struct http_worker_info *) v_param;
    char buffer[1024] = {0};
    int offset = 0;
    snprintf(buffer, 1024, "GET /2020test/2020test?");
    offset = strlen(buffer);
    snprintf(buffer + offset, 1024,
             "data=[{\"x\":\"7788\",\"y\":\"5566\",\"value\":\"1818\"}] "
             "HTTP/1.0\r\n\r\n");

    printf("SENDING: %s", buffer);
    printf("===\n");

    // For this trivial demo just assume write() sends all bytes in one go and
    // is not interrupted

    write(winfo->http_fd, buffer, strlen(buffer));

    int len = read(winfo->http_fd, &winfo->data_buff[0], 999);
    winfo->data_buff[len] = '\0';
    printf("%s\n", winfo->data_buff);
    memset(winfo->data_buff, 0, 1024);
    len = read(winfo->http_fd, &winfo->data_buff[0], 999);
    winfo->data_buff[len] = '\0';
    printf("%s\n", winfo->data_buff);
    return NULL;
}



int main(int argc, char const *argv[])
{
    int rc = 0;
    /*Signal Handleer*/
    __sighandler_t ret = signal(SIGINT, signal_exit);
    if (ret == SIG_ERR) {
        perror("signal(SIGINT, handler)");
        exit(0);
    }

    ret = signal(SIGTERM, signal_exit);
    if (ret == SIG_ERR) {
        perror("signal(SIGTERM, handler)");
        exit(0);
    }
    /* code */
    struct device_worker_info *dev_worker;
    dev_worker = calloc(1, sizeof(struct device_worker_info));
    if (!dev_worker)
        return -1;
    rc = radar01_io_init("/dev/ttyACM1", (void *) &iwr1642_dss_blk);
    uint8_t is_device_worker_ready = 0;
    if (rc < 0) {
        is_device_worker_ready = 0;
        printf("[Warning] Fail to radar01_io_init. skipped\n");
        iwr1642_dss_blk = NULL;
        if (dev_worker)
            free(dev_worker);
    } else {
        is_device_worker_ready = 1;
    }

    int ep_event = 0;
    if (argc > 1)
        if (strcmp(argv[1], "-et") == 0) {
            ep_event = EPOLLET;
        }
    /*Device IO init */
    int epoll_fd;
    static struct epoll_event ev;
    if ((epoll_fd = epoll_create(EPOLL_SIZE)) < 0)
        server_err("Fail to create epoll");
    if (is_device_worker_ready) {
        ev.events = EPOLLIN | ep_event;
        ev.data.fd = iwr1642_dss_blk->dss_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, iwr1642_dss_blk->dss_fd, &ev) <
            0)
            server_err("Fail to control epoll");
        printf("Listener (fd=%d) was added to epoll.\n", epoll_fd);
        dev_worker->epoll_fd = epoll_fd;
        dev_worker->dss_fd = iwr1642_dss_blk->dss_fd;
    }

    /* HTTP CLient Init*/
    struct http_worker_info *http_winfo;
    http_winfo = calloc(1, sizeof(struct http_worker_info));
    uint8_t is_hp_worker_ready = 0;
    if (http_winfo) {
        rc = radar01_http_socket_init("49.159.114.50:10002",
                                      (void *) &arstu_server_blk);
        if (rc < 0)
            is_hp_worker_ready = 0;
        else {
            http_winfo->http_fd = arstu_server_blk->client_fd;
            is_hp_worker_ready = 1;
        }
    }
    if (is_device_worker_ready)
        printf("Frame Seq, Obj_index, x, y, z, velocity, snr, noise\n");

    pthread_t dev_tid0;
    if (is_device_worker_ready) {
        rc = pthread_create(&dev_tid0, 0, &device_worker, (void *) dev_worker);
        if (rc < 0) {
            printf("[ERROR] Device Thread create fail rc = %d\n", rc);
            goto exit_0;
        }
    }

    pthread_t hp_tid1;
    if (is_hp_worker_ready) {
        rc = pthread_create(&hp_tid1, 0, &http_worker, (void *) http_winfo);
        if (rc < 0) {
            printf("[ERROR] HTTP Thread create fail rc = %d\n", rc);
            goto exit_0;
        }
    }
    void *cancel_hook = NULL;
    if (is_device_worker_ready)
        pthread_join(dev_tid0, &cancel_hook);
    if (is_hp_worker_ready)
        pthread_join(hp_tid1, &cancel_hook);

exit_0:
    radar01_io_deinit((void *) &iwr1642_dss_blk);
    radar01_http_socket_deinit((void *) &arstu_server_blk);
    close(epoll_fd);
    exit(0);
}

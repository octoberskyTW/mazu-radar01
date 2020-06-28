#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <sys/epoll.h>
#include "radar01_io.h"
#include "radar01_tlv.h"
#include "radar01_utils.h"
#include "vender/dpif_pointcloud.h"
#include "vender/mmw_output.h"


#define EPOLL_SIZE 256
#define BUF_SIZE 512
#define EPOLL_RUN_TIMEOUT -1

#define MSG_HEADER_LENS (sizeof(MmwDemo_output_message_header))

static void server_err(const char *str)
{
    perror(str);
    exit(-1);
}

static int exit_i = 0;
static void signal_exit(int signal)
{
    (void) signal;
    exit_i++;
}

struct radar01_io_info_t *iwr1642_dss_blk;
struct radar01_io_info_t *iwr1642_mss_blk;

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

void *http_worker(void *v_param)
{
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
    if (rc < 0) {
        server_err("Fail to radar01_io_init");
    }

    int ep_event = 0;
    if (argc > 1)
        if (strcmp(argv[1], "-et") == 0) {
            ep_event = EPOLLET;
        }

    int epoll_fd;
    if ((epoll_fd = epoll_create(EPOLL_SIZE)) < 0)
        server_err("Fail to create epoll");

    static struct epoll_event ev;
    ev.events = EPOLLIN | ep_event;
    ev.data.fd = iwr1642_dss_blk->dss_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, iwr1642_dss_blk->dss_fd, &ev) < 0)
        server_err("Fail to control epoll");
    printf("Listener (fd=%d) was added to epoll.\n", epoll_fd);
    dev_worker->epoll_fd = epoll_fd;
    dev_worker->dss_fd = iwr1642_dss_blk->dss_fd;
    printf("Frame Seq, Obj_index, x, y, z, velocity, snr, noise\n");

    pthread_t dev_tid0;
    rc = pthread_create(&dev_tid0, 0, &device_worker, (void *) dev_worker);
    if (rc < 0) {
        printf("[ERROR] Device Thread create fail rc = %d\n", rc);
        goto exit_0;
    }

    void *cancel_hook = NULL;
    pthread_join(dev_tid0, &cancel_hook);
exit_0:
    radar01_io_deinit((void *) &iwr1642_dss_blk);
    close(epoll_fd);
    exit(0);
}

#include <stdio.h>
#include <sys/epoll.h>
#include "radar01_io.h"
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

struct radar01_io_info_t *iwr1642_dss_blk;
struct radar01_io_info_t *iwr1642_mss_blk;

static int radar01_header_parser(int fd, uint8_t *rx_buff, int buff_size)
{
    int offset;
    MmwDemo_output_message_header msg_header = {0};
    MmwDemo_output_message_tl tlv = {0};
    DPIF_PointCloudCartesian vector = {0};
    offset = radar01_data_recv(fd, rx_buff, MSG_HEADER_LENS);
    if (offset == MSG_HEADER_LENS) {
        static uint16_t magicWord[4] = {0x0102, 0x0304, 0x0506, 0x0708};
        if (memcmp(rx_buff, magicWord, 8) == 0) {
            if (offset > 0) {
                debug_hex_dump("DSS Header ", rx_buff, offset);
                memcpy(&msg_header, rx_buff, MSG_HEADER_LENS);
            }
            // Workaround here
            msg_header.totalPacketLen = 128;
            while (offset < msg_header.totalPacketLen) {
                int rdlen = 0;
                rdlen = radar01_data_recv(fd, rx_buff + offset,
                                          msg_header.totalPacketLen - offset);
                if (rdlen < 0) {
                    if (errno == EINTR || errno == EAGAIN ||
                        errno == EWOULDBLOCK) {
                        rdlen = 0;
                    } else {
                        printf("%s:%d: rdlen = %d, %s\n", __func__, __LINE__,
                               rdlen, strerror(errno));
                        return -1;
                    }
                } else if (rdlen == 0) {
                    break;
                }
                offset += rdlen;
            }  // while
            debug_hex_dump("DSS Header+Content ", rx_buff,
                           msg_header.totalPacketLen);
            uint32_t cur = MSG_HEADER_LENS;
            memcpy(&tlv, rx_buff + cur, sizeof(tlv));
            cur += sizeof(tlv);
            int obj_nums = tlv.length / sizeof(DPIF_PointCloudCartesian);
            for (int i = 0; i < 3; i++) {
                memcpy(&vector, rx_buff + cur,
                       sizeof(DPIF_PointCloudCartesian));
                printf("Object%d, %f, %f, %f, %f \n", i, vector.x, vector.y,
                       vector.z, vector.velocity);
                cur += sizeof(DPIF_PointCloudCartesian);
            }
        }
    }
    return offset;
}

int main(int argc, char const *argv[])
{
    int rc = 0;
    /* code */
    uint8_t *data_buff = (uint8_t *) calloc(1, 1024);
    if (!data_buff)
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
    static struct epoll_event ev_recv[EPOLL_SIZE];
    if ((epoll_fd = epoll_create(EPOLL_SIZE)) < 0)
        server_err("Fail to create epoll");

    static struct epoll_event ev;
    ev.events = EPOLLIN | ep_event;
    ev.data.fd = iwr1642_dss_blk->dss_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, iwr1642_dss_blk->dss_fd, &ev) < 0)
        server_err("Fail to control epoll");
    printf("Listener (fd=%d) was added to epoll.\n", epoll_fd);

    printf("Object i, x, y, z, velocity\n");
    while (1) {
        int epoll_events_count;
        if ((epoll_events_count = epoll_wait(epoll_fd, ev_recv, EPOLL_SIZE,
                                             EPOLL_RUN_TIMEOUT)) < 0)
            server_err("Fail to wait epoll");
        // printf("epoll event count: %d\n", epoll_events_count);
        //        clock_t start_time = clock();
        for (int i = 0; i < epoll_events_count; i++) {
            /* EPOLLIN event for listener (new client connection) */
            if (ev_recv[i].data.fd == iwr1642_dss_blk->dss_fd) {
                radar01_header_parser(iwr1642_dss_blk->dss_fd, data_buff, 1024);
            } else {
                // /* EPOLLIN event for others (new incoming message from
                // client)
                //  */
                // if (handle_message_from_client(events[i].data.fd, &list) < 0)
                //     server_err("Handle message from client", &list);
            }
        }
    }
    radar01_io_deinit((void *) &iwr1642_dss_blk);
    close(epoll_fd);
    exit(0);
}

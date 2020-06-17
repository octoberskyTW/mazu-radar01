#include <stdio.h>
#include <sys/epoll.h>
#include "radar01_io.h"
#include "radar01_utils.h"



#define EPOLL_SIZE 256
#define BUF_SIZE 512
#define EPOLL_RUN_TIMEOUT -1

static void server_err(const char *str)
{
    perror(str);
    exit(-1);
}

struct radar01_io_info_t *iwr1642_dss_blk;
struct radar01_io_info_t *iwr1642_mss_blk;
int main(int UNUSED(argc), char const UNUSED(*argv[]))
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


    int epoll_fd;
    static struct epoll_event events[EPOLL_SIZE];
    if ((epoll_fd = epoll_create(EPOLL_SIZE)) < 0)
        server_err("Fail to create epoll");

    static struct epoll_event ev = {.events = EPOLLIN | EPOLLET};
    ev.data.fd = iwr1642_dss_blk->dss_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, iwr1642_dss_blk->dss_fd, &ev) < 0)
        server_err("Fail to control epoll");
    printf("Listener (fd=%d) was added to epoll.\n", epoll_fd);


    while (1) {
        int size = 0;
        int epoll_events_count;
        if ((epoll_events_count = epoll_wait(epoll_fd, events, EPOLL_SIZE,
                                             EPOLL_RUN_TIMEOUT)) < 0)
            server_err("Fail to wait epoll");
        printf("epoll event count: %d\n", epoll_events_count);
        //        clock_t start_time = clock();
        for (int i = 0; i < epoll_events_count; i++) {
            /* EPOLLIN event for listener (new client connection) */
            if (events[i].data.fd == iwr1642_dss_blk->dss_fd) {
                size =
                    radar01_data_recv(iwr1642_dss_blk->dss_fd, data_buff, 1024);
                if (size > 0) {
                    debug_hex_dump("DSS dump ", data_buff, size);
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
    radar01_io_deinit((void *) &iwr1642_dss_blk);
    close(epoll_fd);
    exit(0);
}

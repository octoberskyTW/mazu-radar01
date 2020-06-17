#include "radar01_io.h"
#include "radar01_utils.h"

static int open_port(char *portname)
{
    int fd;
    fd = open(portname, O_RDWR | O_NOCTTY);
    if (fd < 0) {
        return fd;
    } else {
        printf("Open %s success.\n", portname);
    }
    return (fd);
}

static int set_interface_attribs(int fd, int speed, int parity)
{
    struct termios options;
    int rc = 0, setflag;
    // Get the current options for the port
    if ((rc = tcgetattr(fd, &options)) < 0) {
        printf("[%s:%d] Fail to get attr, rc = %d [ERROR]: %s\n", __FUNCTION__,
               __LINE__, rc, strerror(errno));
        return rc;
    }

    cfsetospeed(&options, (speed_t) speed);
    cfsetispeed(&options, (speed_t) speed);
    /* Enable the receiver and set local mode...*/
    options.c_cflag |= (CLOCAL | CREAD);

    /* Setting the Character Size */
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    options.c_cflag &= ~(PARENB | PARODD);
    options.c_cflag |= parity;
    options.c_cflag &= ~CSTOPB;  // 1 stop bits

    /*Setting Hardware Flow Control*/
    options.c_cflag &= ~CRTSCTS;

    /* Local Options: Choosing Raw Input*/
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

    /* Input Options: Setting Software Flow Control */
    options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_iflag &= ~(INLCR | ICRNL);

    /* Output Options: Choosing Raw Output */
    options.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    options.c_cc[VMIN] = 0;       // read doesn't block
    options.c_cc[VTIME] = 0.001;  // 0.5 seconds read timeout


    // Set the new attributes
    if ((rc = tcsetattr(fd, TCSANOW, &options)) < 0) {
        printf("[%s:%d] Fail to set attr, rc = %d [ERROR]: %s\n", __FUNCTION__,
               __LINE__, rc, strerror(errno));
        return rc;
    }

    setflag = fcntl(fd, F_GETFL);
    setflag = setflag | O_NONBLOCK;
    rc = fcntl(fd, F_SETFL, setflag);
    if (rc != 0) {
        printf("[%s:%d] Fail to set attr, rc = %d [ERROR]: %s\n", __FUNCTION__,
               __LINE__, rc, strerror(errno));
    }
    return rc;
}

int radar01_io_init(char *ifname, void **priv_data)
{
    int rc = 0;
    struct radar01_io_info_t *dev_info;
    dev_info =
        (struct radar01_io_info_t *) malloc(sizeof(struct radar01_io_info_t));
    if (dev_info == NULL) {
        printf("[%s:%d] Memory allocate fail. [ERROR]: %s\n", __FUNCTION__,
               __LINE__, strerror(errno));
        rc = -1;
        goto failed_malloc;
    }
    *priv_data = dev_info;
    strncpy(dev_info->ifname, ifname, IFNAMSIZ);
    dev_info->dss_fd = open_port(dev_info->ifname);
    if (dev_info->dss_fd < 0) {
        printf("[%s:%d] Get %s fail. [ERROR]: %s\n", __FUNCTION__, __LINE__,
               dev_info->ifname, strerror(errno));
        rc = -1;
        goto failed_port;
    }
    rc = set_interface_attribs(dev_info->dss_fd, B921600, 0);
    if (rc != 0) {
        goto failed_set_attr;
    }
    return rc;
failed_set_attr:
failed_port:
    if (dev_info) {
        free(dev_info);
        *priv_data = NULL;
    }

failed_malloc:
    return rc;
}

int radar01_data_recv(int fd, uint8_t *rx_buff, int buff_size)
{
    int offset = 0;
    memset(rx_buff, 0, buff_size);
    while (offset < buff_size) {
        int rdlen = 0;
        if ((rdlen = read(fd, rx_buff + offset, buff_size - offset)) < 0) {
            if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
                rdlen = 0;
            } else {
                printf("%s:%d: rdlen = %d, %s\n", __func__, __LINE__, rdlen,
                       strerror(errno));
                return -1;
            }
        } else if (rdlen == 0) {
            break;
        }
        offset += rdlen;
    }
    return offset;
}

void radar01_io_deinit(void **priv_data)
{
    struct radar01_io_info_t *dev_info = *priv_data;
    if (dev_info) {
        close(dev_info->dss_fd);
        printf("Closing %s \n", dev_info->ifname);
        free(dev_info);
        *priv_data = NULL;
    }
}
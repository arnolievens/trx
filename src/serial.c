/**
 * @author      : Arno Lievens (arnolievens@gmail.com)
 * @created     : 20/05/2021
 * @filename    : serial.c
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <termios.h>
#include <unistd.h>

#include "serial.h"

int fd;
struct termios oldtty;

int serial_init(const portsettings_t* portsettings)
{
    if (!portsettings->port) {
        fprintf(stderr, "please provide serial port\n");
        return -1;
    }

    if (!portsettings->baudrate) {
        fprintf(stderr, "please specify baudrate\n");
        return -1;
    }

    /* try to open file descriptor */
    if ((fd = open(portsettings->port, O_RDWR | O_NOCTTY | O_NDELAY)) == -1) {
        fprintf(stderr, "invalid serial port: %s: %s\n",
                portsettings->port,
                strerror(errno));
        return -1;
    }

    /* succes, set port to blocking */
    fcntl(fd, F_SETFL, 0);//FNDELAY);

    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        fprintf(stderr, "error reading port settings: %s\n", strerror(errno));
        return -1;
    }

    oldtty = tty;

    /* set IO speed */
    cfsetospeed(&tty, portsettings->baudrate);
    cfsetispeed(&tty, portsettings->baudrate);

    /* ignore Wsign because termios.c_cflag is unsigned but the macros are */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"

    /* set port paratmeters */
    tty.c_cflag |=                  CLOCAL | CREAD;
    /* ?? */
    tty.c_cflag &=                  ~CSIZE;
    /* 8-bit characters */
    tty.c_cflag |=                  CS8;
    /* no parity bit */
    tty.c_cflag &=                  ~PARENB;
    /* only need 1 stop bit */
    tty.c_cflag &=                  ~CSTOPB;
    /* no hardware flowcontrol  */
    /* tty.c_cflag &=               ~CRTSCTS; */
    /* canonical input */
    tty.c_lflag |=                  ICANON | ISIG;
    /* NON-canonical input!! <] */
    /* tty.c_lflag &=               ~(ICANON | ISIG);*/
    /* echo */
    tty.c_lflag &=                  ~(ECHO | ECHOE | ECHONL | IEXTEN);
    /* preserve carriage return */
    /* tty.c_iflag &=               ~IGNCR; */
    /* ?? */
    tty.c_iflag &=                  ~INPCK;
    /* ?? */
    tty.c_iflag &=                  ~(INLCR | ICRNL | IUCLC | IMAXBEL);
    /* SW fdlow-control */
    tty.c_iflag &=                  ~(IXON | IXOFF | IXANY);
    /* ?? */
    tty.c_oflag &=                  ~OPOST;
    /* alt eol characters */
    tty.c_cc[VEOL] =                0;
    tty.c_cc[VEOL2] =               0;
    tty.c_cc[VEOF] =                0x04;
    /* non-blocking behaviour */
    tty.c_cc[VTIME] =               0;
    tty.c_cc[VMIN] =                0;
#pragma GCC diagnostic pop

    if (tcsetattr(fd, TCSANOW, &tty) == -1) {
        fprintf(stderr, "error setting serial port settings: %s\n",
                strerror(errno));
        return -1;
    }
    return 0;
}

/* TODO "fd" should go in portsettings struct */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
int serial_tx(const portsettings_t* portsettings, const char *cmd)
{
#pragma GCC diagnostic pop
    int len = (int)strlen(cmd);
    int n = (int)write(fd, cmd, (size_t)len);
    write(fd, "\r", 1);

    if (n != len) {
        fprintf(stderr, "sent only %i bytes out of %i\n", n, len);
    }

    /* wait until output buffer is empty */
    tcdrain(fd);
    return n;
}

int serial_rx(const portsettings_t* portsettings, char *buf, size_t size)
{
    ssize_t n = 0;
    fd_set set;
    FD_ZERO(&set);
    FD_SET(fd, &set);

    *buf = '\0';

    struct timeval timeout = {
        .tv_sec = 0,
        .tv_usec = (long)(1000000.0 * portsettings->timeout),
    };

    switch (select(fd+1, &set, NULL, NULL, &timeout)) {

        /* error select() */
        case -1:
            fprintf(stderr, "error selecting port: %s\n" , strerror(errno));
            break;

        /* timeout occured - return 0 but empty buffer */
        case 0:
            return 0;

        /* available for reading */
        default: {

            /* read from serial port */
            n = read(fd, buf, size-1);
            if (n > 0) {
                /* trim CRLF */
                *(strchr(buf, '\n')) = '\0';
                *(strchr(buf, '\r')) = '\0';
                return 0;

            /* read() error */
            } else if (n < 0) {
                fprintf(stderr, "error reading port: %s\n" , strerror(errno));
                break;

            /* read should never return 0 as read is canonical (blocking) */
            } else {
                printf("read() returned 0, this should not happen??\n");
                break;
            }
        }
    }
    return -1;
}

int serial_die(void) {

    if (tcsetattr(fd, TCSANOW, &oldtty) == -1) {
        fprintf(stderr, "error resetting serial port settings: %s\n",
                strerror(errno));
        return -1;
    }

    if (fd) close(fd);
    return 0;
}

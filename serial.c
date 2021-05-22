/**
 * @author      : Arno Lievens (arnolievens@gmail.com)
 * @created     : 20/05/2021
 * @filename    : serial.c
 */

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>

#include "serial.h"

int fd;

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

    /* set IO speed */
    cfsetospeed(&tty, portsettings->baudrate);
    cfsetispeed(&tty, portsettings->baudrate);

    /* set port paratmeters */
    tty.c_cflag |= CLOCAL | CREAD;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    /* tty.c_cflag &= ~CRTSCTS;    [> no hardware flowcontrol <] */

    tty.c_lflag |= ICANON | ISIG;  /* canonical input */
    /* tty.c_lflag &= ~(ICANON | ISIG);  [> NON-canonical input!! <] */

    tty.c_lflag &= ~(ECHO | ECHOE | ECHONL | IEXTEN);

    //tty.c_iflag &= ~IGNCR;  /* preserve carriage return */
    tty.c_iflag &= ~INPCK;
    tty.c_iflag &= ~(INLCR | ICRNL | IUCLC | IMAXBEL);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);   /* no SW flowcontrol */

    tty.c_oflag &= ~OPOST;

    tty.c_cc[VEOL] = 0;
    tty.c_cc[VEOL2] = 0;
    tty.c_cc[VEOF] = 0x04;

    tty.c_cc[VTIME]    = 0;     /* inter-character timer unused */
    tty.c_cc[VMIN]     = 0;     /* blocking read until 1 character arrives */

    // write settings to port
    if (tcsetattr(fd, TCSANOW, &tty) == -1) {
        fprintf(stderr, "error setting serial port settings: %s\n",
                strerror(errno));
        return -1;
    }
    return 1;
}

int serial_tx(const portsettings_t* portsettings, const char *cmd) {
    int len = strlen(cmd);
    ssize_t n = write(fd, cmd, len);
    write(fd, "\r", 1);
    if (n != len) {
        fprintf(stderr, "sent only %li bytes out of %i\n", n, len);
    }
    tcdrain(fd); /* wait until output buffer is empty */
    return n;
}

int serial_rx(const portsettings_t* portsettings, char *buf, size_t size)
{
    ssize_t n = 0;
    fd_set set;
    FD_ZERO(&set); /* clear the set */
    FD_SET(fd, &set); /* add our file descriptor to the set */

    struct timeval timeout;
    timeout.tv_usec = 0;
    timeout.tv_sec  = (int)portsettings->wait;

    *buf = '\0';

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

void serial_die(void) {
    if (fd) close(fd);
}

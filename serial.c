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

#include "serial.h"

int fd;

int serial_init(const portsettings_t* portsettings)
{
    /* try to open file descriptor */
    if ((fd = open(portsettings->port, O_RDWR | O_NOCTTY | O_NDELAY)) == -1) {
        fprintf(stderr, "unable to open %s: %s\n",
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

    /* tty.c_lflag |= ICANON | ISIG;  [> canonical input <] */
    tty.c_lflag &= ~(ICANON | ISIG);  /* NON-canonical input!! */

    tty.c_lflag &= ~(ECHO | ECHOE | ECHONL | IEXTEN);

    //tty.c_iflag &= ~IGNCR;  /* preserve carriage return */
    tty.c_iflag &= ~INPCK;
    tty.c_iflag &= ~(INLCR | ICRNL | IUCLC | IMAXBEL);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);   /* no SW flowcontrol */

    tty.c_oflag &= ~OPOST;

    tty.c_cc[VEOL] = 0;
    tty.c_cc[VEOL2] = 0;
    tty.c_cc[VEOF] = 0x04;

    tty.c_cc[VTIME]    = portsettings->wait * 10;     /* inter-character timer unused */
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
    char *eol = NULL;
    *buf = '\0';

    while (!eol) {

        ssize_t n = read(fd, buf, size - 1);

        if (n > 0) {
            eol = strchr(buf, '\n');
            buf += n;
            size -= n;

        } else if (n < 0) {
            fprintf(stderr, "Error reading from serial port\n");
            *buf = '\0';
            return -1;

        } else {
            return -1;
        }
    }
    /* trim \n char */
    *eol = '\0';
    return 0;
}

void serial_die(void) {
    if (fd) close(fd);
}




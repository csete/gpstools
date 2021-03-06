/*
 * Copyright (c) 2014, Alexandru Csete <oz9aec@gmail.com>
 * All rights reserved.
 *
 * This software is licensed under the terms and conditions of the
 * Simplified BSD License. See license.txt for details.
 *
 */
#include <errno.h>
#include <fcntl.h>              /* O_WRONLY */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>

#include "common.h"

/* Print an array of chars as HEX numbers */
inline void print_buffer_hex(int fd, const uint8_t * buf, unsigned int len)
{
    int             i;

    fprintf(stderr, "%d:", fd);

    for (i = 0; i < len; i++)
        fprintf(stderr, " %02X", buf[i]);

    fprintf(stderr, "\n");
}

inline void print_buffer_txt(int fd, const uint8_t * buf, unsigned int len)
{
    int             i;

    fprintf(stderr, "%d:", fd);

    for (i = 0; i < len; i++)
        fprintf(stderr, "%c", buf[i]);

    fprintf(stderr, "\n");
}


static int _int_to_serial_speed(speed)
{
    switch (speed)
    {
    case 1200:
        return B1200;
    case 2400:
        return B2400;
    case 4800:
        return B4800;
    default:
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    }
}

/* Configure serial interface to raw mode with specified attributes */
int gps_open(const char *port, int speed, int blocking)
{
    int             gps_fd;
    struct termios  tty;


    gps_fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (gps_fd == -1)
    {
        fprintf(stderr, "Error opening uart %s: %d (%s)\n", port, errno,
                strerror(errno));
        return -1;
    }

    memset(&tty, 0, sizeof(tty));
    if (tcgetattr(gps_fd, &tty) != 0)
    {
        fprintf(stderr, "error %d from tcgetattr", errno);
        return -1;
    }

    cfsetospeed(&tty, _int_to_serial_speed(speed));
    cfsetispeed(&tty, _int_to_serial_speed(speed));

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars
    // disable IGNBRK for mismatched speed tests; otherwise receive break
    // as \000 chars
    tty.c_iflag &= ~IGNBRK;     // disable break processing
    tty.c_lflag = 0;            // no signaling chars, no echo,
    // no canonical processing

    /* no remapping, no delays */
    tty.c_oflag = 0;

    /* 0.5 sec read timeout */
    tty.c_cc[VMIN] = blocking ? 1 : 0;
    tty.c_cc[VTIME] = 5;

    /* shut off xon/xoff ctrl */
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);

    /* ignore modem controls and enable reading */
    tty.c_cflag |= (CLOCAL | CREAD);

    /* parity */
    tty.c_cflag &= ~(PARENB | PARODD);
    //tty.c_cflag |= parity;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(gps_fd, TCSANOW, &tty) != 0)
    {
        fprintf(stderr, "error %d from tcsetattr", errno);
        return -1;
    }

    return gps_fd;
}


int read_data(int fd, struct xfr_buf *buffer)
{
    uint8_t        *buf = buffer->data;
    int             type = MSG_TYPE_INCOMPLETE;
    size_t          num;

    /* read data */
    num = read(fd, &buf[buffer->wridx], RDBUF_SIZE - buffer->wridx);

    if (num > 0)
    {
        buffer->wridx += num;

        /* There is at least one character in the buffer */
        if (buf[0] == '$')
        {
            if (buf[buffer->wridx - 1] == 0x0A)
                type = MSG_TYPE_VALID;
            else
                type = MSG_TYPE_INCOMPLETE;
        }
        else if ((buf[0] == 0x0A) && (buffer->wridx == 1))
        {
            /* single line feed character -> ignore */
            buffer->wridx = 0;
            type = MSG_TYPE_INCOMPLETE;
        }
        else if ((buf[0] == 0x00) && (buffer->wridx == 1))
        {
            type = MSG_TYPE_EOF;
        }
        else
        {
            type = MSG_TYPE_INVALID;
        }
    }
    else if (num == 0)
    {
        type = MSG_TYPE_EOF;
        fprintf(stderr, "Received EOF from FD %d\n", fd);
    }
    else
    {
        type = MSG_TYPE_INVALID;
        fprintf(stderr, "Error reading from FD %d: %d: %s\n", fd, errno,
                strerror(errno));
    }

    return type;
}

uint64_t time_ms(void)
{
    struct timeval  tval;

    gettimeofday(&tval, NULL);

    return 1e3 * tval.tv_sec + 1e-3 * tval.tv_usec;
}

uint64_t time_us(void)
{
    struct timeval  tval;

    gettimeofday(&tval, NULL);

    return 1e6 * tval.tv_sec + tval.tv_usec;
}

/*
 * Copyright (c) 2014, Alexandru Csete <oz9aec@gmail.com>
 * All rights reserved.
 *
 * This software is licensed under the terms and conditions of the
 * Simplified BSD License. See license.txt for details.
 *
 */
#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdint.h>

/* Use 1 = debug, 0 = release */
#define DEBUG 1

/* Read buffer size */
#define RDBUF_SIZE 2048

/* convenience struct for data transfers */
struct xfr_buf {
    uint8_t         data[RDBUF_SIZE];
    int             wridx;      /* next available write slot. */
    uint64_t        valid_pkts; /* number of valid packets */
    uint64_t        invalid_pkts;       /* number of invalid packets */
};


#define MSG_TYPE_INVALID        -1
#define MSG_TYPE_INCOMPLETE     0
#define MSG_TYPE_VALID          1
#define MSG_TYPE_EOF            2

/** Read data from file descriptor.
 *  @param  fd      The file descriptor.
 *  @param  buffer  Pointer to the serial_buffer structure to use.
 *  @returns The packet type if the packet is complete.
 *
 * This function will read all available data from the UART and put the
 * data into the buffer starting at index buffer->wridx. When the read
 * is finished buffer->wridx will again point to the first available
 * slot in buffer, which mneans that it is also equal to the number of
 * bytes in the buffer.
 *
 * If the last byte read was 0xFD the packet is considered complete
 * and the function will return the packet type. The caller can use this
 * information to decide what to do with the data.
 *
 * If the last byte is not 0xFD then the read is only partial and the
 * function returns MSG_TYPE_INCOMPLETE to indicate this to the caller.
 */
int             read_data(int fd, struct xfr_buf *buffer);

int             gps_open(const char *port, int speed, int blocking);

/** Get current time in milliseconds. */
uint64_t        time_ms(void);

/** Get current time in microseconds. */
uint64_t        time_us(void);

#endif

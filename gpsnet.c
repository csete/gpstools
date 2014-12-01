/*
 * Copyright (c) 2014, Alexandru Csete <oz9aec@gmail.com>
 * All rights reserved.
 *
 * This software is licensed under the terms and conditions of the
 * Simplified BSD License. See license.txt for details.
 *
 */
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>           // PRId64 and PRIu64
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"


#define NUM_FDS 10

static char    *uart = NULL;    /* UART port */
static int      speed = 4800;   /* serial speed */
static int      port = 45000;   /* Network port */
static int      keep_running = 1;


void signal_handler(int signo)
{
    if (signo == SIGINT)
        fprintf(stderr, "\nCaught SIGINT\n");
    else if (signo == SIGTERM)
        fprintf(stderr, "\nCaught SIGTERM\n");
    else
        fprintf(stderr, "\nCaught signal: %d\n", signo);

    keep_running = 0;
}

static void help(void)
{
    static const char help_string[] =
        "\n Usage: gpsnet [options]\n"
        "\n Possible options are:\n"
        "\n"
        "  -d dev    Serial device (default is /dev/ttyUSB0).\n"
        "  -s speed  Serial speed (default is 4800 baud).\n"
        "  -p port   Network port number (default is 45000).\n"
        "  -h        This help message.\n\n";

    fprintf(stderr, "%s", help_string);
}

/* Parse command line options */
static void parse_options(int argc, char **argv)
{
    int             option;

    if (argc > 1)
    {
        while ((option = getopt(argc, argv, "d:s:p:h")) != -1)
        {
            switch (option)
            {
            case 'd':
                uart = strdup(optarg);
                break;

            case 's':
                speed = atoi(optarg);
                break;

            case 'p':
                port = atoi(optarg);
                break;

            case 'h':
                help();
                exit(EXIT_SUCCESS);

            default:
                help();
                exit(EXIT_FAILURE);
            }
        }
    }
}

/* send GPS data to active network sockets */
static void send_gps_data(struct xfr_buf *gps_buf, struct pollfd *poll_fds,
                          int num_fd)
{
    int             i;
    ssize_t         written;

    for (i = 2; i < num_fd; i++)
    {
        if (poll_fds[i].fd != -1)
        {
            written = write(poll_fds[i].fd, gps_buf->data, gps_buf->wridx);
            if (written != gps_buf->wridx)
                fprintf(stderr, "Wrote %zd instead of %d bytes to FD=%d\n",
                        written, gps_buf->wridx, poll_fds[i].fd);
        }
    }
}

/* get index of first available fd */
static int get_idle_fd_index(struct pollfd *poll_fds, int num_fd)
{
    int             i;

    for (i = 2; i < NUM_FDS; i++)
    {
        if (poll_fds[i].fd == -1)
            return i;
    }

    return -1;
}

int main(int argc, char **argv)
{
    int             exit_code = EXIT_FAILURE;
    int             sock_fd, gps_fd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t       cli_addr_len;

    struct pollfd   poll_fds[NUM_FDS];
    int             i;

    struct xfr_buf  gps_buf, net_buf;

    gps_buf.wridx = 0;
    gps_buf.valid_pkts = 0;
    gps_buf.invalid_pkts = 0;

    net_buf.wridx = 0;
    net_buf.valid_pkts = 0;
    net_buf.invalid_pkts = 0;

    parse_options(argc, argv);

    /* setup signal handler */
    if (signal(SIGINT, signal_handler) == SIG_ERR)
        printf("Warning: Can't catch SIGINT\n");
    if (signal(SIGTERM, signal_handler) == SIG_ERR)
        printf("Warning: Can't catch SIGTERM\n");

    /* open GPS port */
    if (uart == NULL)
        uart = strdup("/dev/ttyO1");
    gps_fd = gps_open(uart, speed, 0);
    if (gps_fd == -1)
        exit(EXIT_FAILURE);

    /* open and configure network interface */
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1)
    {
        fprintf(stderr, "Error creating socket: %d: %s\n", errno,
                strerror(errno));
        goto cleanup;
    }

    int             yes = 1;

    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
        fprintf(stderr, "Error setting SO_REUSEADDR: %d: %s\n", errno,
                strerror(errno));

    /* bind socket to host address */
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if (bind(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        fprintf(stderr, "bind() error: %d: %s\n", errno, strerror(errno));
        goto cleanup;
    }

    if (listen(sock_fd, 1) == -1)
    {
        fprintf(stderr, "listen() error: %d: %s\n", errno, strerror(errno));
        goto cleanup;
    }

    memset(&cli_addr, 0, sizeof(struct sockaddr_in));
    cli_addr_len = sizeof(cli_addr);


    /* GPS input */
    poll_fds[0].fd = gps_fd;
    poll_fds[0].events = POLLIN;

    /* network socket (listening for connections) */
    poll_fds[1].fd = sock_fd;
    poll_fds[1].events = POLLIN;

    /* network sockets to clients (when connected) */
    for (i = 2; i < NUM_FDS; i++)
        poll_fds[i].fd = -1;


    while (keep_running)
    {
        if (poll(poll_fds, NUM_FDS, 100) < 0)
            continue;

        /* service network clients */
        for (i = 2; i < NUM_FDS; i++)
        {
            if ((poll_fds[i].fd != -1) && (poll_fds[i].revents & POLLIN))
            {
                switch (read_data(poll_fds[i].fd, &net_buf))
                {
                case MSG_TYPE_EOF:
                    fprintf(stderr, "Connection closed (FD=%d)\n",
                            poll_fds[i].fd);
                    close(poll_fds[i].fd);
                    poll_fds[i].fd = -1;
                    poll_fds[i].events = 0;
                    break;
                }

                net_buf.wridx = 0;
            }
        }

        /* check if there are any new connections pending */
        if (poll_fds[1].revents & POLLIN)
        {
            int             new;
            int             fd_idx = -1;

            new = accept(sock_fd, (struct sockaddr *)&cli_addr, &cli_addr_len);
            if (new == -1)
            {
                fprintf(stderr, "accept() error: %d: %s\n", errno,
                        strerror(errno));
                goto cleanup;
            }

            fprintf(stderr, "New connection from %s\n",
                    inet_ntoa(cli_addr.sin_addr));

            if ((fd_idx = get_idle_fd_index(&poll_fds[0], NUM_FDS)) > 1)
            {
                fprintf(stderr, "Connection accepted (FD=%d)\n", new);
                poll_fds[fd_idx].fd = new;
                poll_fds[fd_idx].events = POLLIN;
            }
            else
            {
                fprintf(stderr, "Connection refused. Got fd_idx = %d\n",
                        fd_idx);
                close(new);
            }
        }

        /* read data from gps */
        if (poll_fds[0].revents & POLLIN)
        {
            switch (read_data(poll_fds[0].fd, &gps_buf))
            {
            case MSG_TYPE_INVALID:
                gps_buf.invalid_pkts++;
                gps_buf.wridx = 0;
                break;

            case MSG_TYPE_INCOMPLETE:
                break;

            case MSG_TYPE_VALID:
                send_gps_data(&gps_buf, &poll_fds[0], NUM_FDS);
#if DEBUG
                gps_buf.data[gps_buf.wridx] = 0;
                fprintf(stderr, "Valid GPS data: %s", gps_buf.data);
#endif
                gps_buf.valid_pkts++;
                gps_buf.wridx = 0;
                break;

            case MSG_TYPE_EOF:
                fprintf(stderr, "Got EOF from GPS!\n");
                gps_buf.wridx = 0;
                keep_running = 0;
                break;

            default:
                fprintf(stderr, "WTF?\n");
                gps_buf.wridx = 0;
                break;
            }
        }

        //usleep(10000);
    }

    fprintf(stderr, "Shutting down...\n");
    exit_code = EXIT_SUCCESS;

  cleanup:
    for (i = 0; i < NUM_FDS; i++)
        if (poll_fds[i].fd != -1)
            close(poll_fds[i].fd);

    fprintf(stderr, "    Valid messages: %" PRIu64 "\n", gps_buf.valid_pkts);
    fprintf(stderr, "  Invalid messages: %" PRIu64 "\n", gps_buf.invalid_pkts);

    exit(exit_code);
}

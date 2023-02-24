/*
 * inetcat.c -- simple netcat-like tool that enables service access to iOS devices
 *
 * Copyright (C) 2017 Adrien Guinet <adrien@guinet.me>
 *
 * Based on iproxy which is based upon iTunnel source code, Copyright (c)
 * 2008 Jing Su.  http://www.cs.toronto.edu/~jingsu/itunnel/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define TOOL_NAME "inetcat"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#else
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <signal.h>
#endif

#include "usbmuxd.h"
#include "socket.h"

static int debug_level = 0;

static size_t read_data_socket(int fd, uint8_t* buf, size_t bufsize)
{
#ifdef WIN32
    u_long bytesavailable = 0;
    if (fd == STDIN_FILENO) {
        bytesavailable = bufsize;
    } else if (ioctlsocket(fd, FIONREAD, &bytesavailable) != 0) {
        perror("ioctlsocket FIONREAD failed");
        exit(1);
    }
#else
    size_t bytesavailable = 0;
    if (ioctl(fd, FIONREAD, &bytesavailable) != 0) {
        perror("ioctl FIONREAD failed");
        exit(1);
    }
#endif
    size_t bufread = (bytesavailable >= bufsize) ? bufsize:bytesavailable;
    ssize_t ret = read(fd, buf, bufread);
    if (ret < 0) {
        perror("read failed");
        exit(1);
    }
    return (size_t)ret;
}

static void print_usage(int argc, char **argv, int is_error)
{
    char *name = NULL;
    name = strrchr(argv[0], '/');
    fprintf(is_error ? stderr : stdout, "Usage: %s [OPTIONS] DEVICE_PORT\n", (name ? name + 1: argv[0]));
    fprintf(is_error ? stderr : stdout,
        "\n" \
        "Opens a read/write interface via STDIN/STDOUT to a TCP port on a usbmux device.\n" \
        "\n" \
        "OPTIONS:\n" \
        "  -u, --udid UDID    target specific device by UDID\n" \
        "  -n, --network      connect to network device\n" \
        "  -l, --local        connect to USB device (default)\n" \
        "  -h, --help         prints usage information\n" \
        "  -d, --debug        increase debug level\n" \
        "  -v, --version      prints version information\n" \
        "\n" \
        "Homepage:    <" PACKAGE_URL ">\n"
        "Bug Reports: <" PACKAGE_BUGREPORT ">\n"
    );
}

int main(int argc, char **argv)
{
    const struct option longopts[] = {
        { "debug", no_argument, NULL, 'd' },
        { "help", no_argument, NULL, 'h' },
        { "udid", required_argument, NULL, 'u' },
        { "local", no_argument, NULL, 'l' },
        { "network", no_argument, NULL, 'n' },
        { "version", no_argument, NULL, 'v' },
        { NULL, 0, NULL, 0}
    };

    char* device_udid = NULL;
    static enum usbmux_lookup_options lookup_opts = 0;

    int c = 0;
    while ((c = getopt_long(argc, argv, "dhu:lnv", longopts, NULL)) != -1) {
        switch (c) {
        case 'd':
            libusbmuxd_set_debug_level(++debug_level);
            break;
        case 'u':
            if (!*optarg) {
                fprintf(stderr, "ERROR: UDID must not be empty!\n");
                print_usage(argc, argv, 1);
                return 2;
            }
            free(device_udid);
            device_udid = strdup(optarg);
            break;
        case 'l':
            lookup_opts |= DEVICE_LOOKUP_USBMUX;
            break;
        case 'n':
            lookup_opts |= DEVICE_LOOKUP_NETWORK;
            break;
        case 'h':
            print_usage(argc, argv, 0);
            return 0;
        case 'v':
            printf("%s %s\n", TOOL_NAME, PACKAGE_VERSION);
            return 0;
        default:
            print_usage(argc, argv, 1);
            return 2;
        }
    }

    if (lookup_opts == 0) {
        lookup_opts = DEVICE_LOOKUP_USBMUX;
    }

    argc -= optind;
    argv += optind;

    if (argc < 1) {
        print_usage(argc + optind, argv - optind, 1);
        return 2;
    }

    int device_port = atoi(argv[0]);
    if (!device_port) {
        fprintf(stderr, "Invalid device_port specified!\n");
        return -EINVAL;
    }

#ifndef WIN32
    signal(SIGPIPE, SIG_IGN);
#endif

    usbmuxd_device_info_t *dev_list = NULL;
    usbmuxd_device_info_t *dev = NULL;
    usbmuxd_device_info_t muxdev;

    if (device_udid) {
        if (usbmuxd_get_device(device_udid, &muxdev, lookup_opts) > 0) {
            dev = &muxdev;
        }
    } else {
        int count;
        if ((count = usbmuxd_get_device_list(&dev_list)) < 0) {
            printf("Connecting to usbmuxd failed, terminating.\n");
            free(dev_list);
            return 1;
        }

        if (dev_list == NULL || dev_list[0].handle == 0) {
            fprintf(stderr, "No connected device found, terminating.\n");
            free(dev_list);
            return 1;
        }

        int i;
        for (i = 0; i < count; i++) {
            if (dev_list[i].conn_type == CONNECTION_TYPE_USB && (lookup_opts & DEVICE_LOOKUP_USBMUX)) {
                dev = &(dev_list[i]);
                break;
            } else if (dev_list[i].conn_type == CONNECTION_TYPE_NETWORK && (lookup_opts & DEVICE_LOOKUP_NETWORK)) {
                dev = &(dev_list[i]);
                break;
            }
        }
    }

    if (dev == NULL || dev->handle == 0) {
        fprintf(stderr, "No connected/matching device found, disconnecting client.\n");
        free(dev_list);
        return 1;
    }

    int devfd = -1;
    if (dev->conn_type == CONNECTION_TYPE_NETWORK) {
        unsigned char saddr_[32];
        memset(saddr_, '\0', sizeof(saddr_));
        struct sockaddr* saddr = (struct sockaddr*)&saddr_[0];
        if (((char*)dev->conn_data)[1] == 0x02) { // AF_INET
            saddr->sa_family = AF_INET;
            memcpy(&saddr->sa_data[0], (char*)dev->conn_data+2, 14);
        }
        else if (((char*)dev->conn_data)[1] == 0x1E) { //AF_INET6 (bsd)
#ifdef AF_INET6
            saddr->sa_family = AF_INET6;
            memcpy(&saddr->sa_data[0], (char*)dev->conn_data+2, 26);
#else
            fprintf(stderr, "ERROR: Got an IPv6 address but this system doesn't support IPv6\n");
            free(dev_list);
            return 1;
#endif
        }
        else {
            fprintf(stderr, "Unsupported address family 0x%02x\n", ((char*)dev->conn_data)[1]);
            free(dev_list);
            return 1;
        }
        char addrtxt[48];
        addrtxt[0] = '\0';
        if (!socket_addr_to_string(saddr, addrtxt, sizeof(addrtxt))) {
            fprintf(stderr, "Failed to convert network address: %d (%s)\n", errno, strerror(errno));
        }
	devfd = socket_connect_addr(saddr, device_port);
    } else if (dev->conn_type == CONNECTION_TYPE_USB) {
        devfd = usbmuxd_connect(dev->handle, device_port);
    }
    free(dev_list);
    if (devfd < 0) {
        fprintf(stderr, "Error connecting to device: %s\n", strerror(errno));
        return 1;
    }

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    FD_SET(devfd, &fds);

    int ret = 0;
    uint8_t buf[4096];
    while (1) {
        fd_set read_fds = fds;
        int ret_sel = select(devfd+1, &read_fds, NULL, NULL, NULL);
        if (ret_sel < 0) {
            perror("select");
            ret = 1;
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &read_fds)) {
            size_t n = read_data_socket(STDIN_FILENO, buf, sizeof(buf));
            if (n == 0) {
                break;
            }
            write(devfd, buf, n);
        }

        if (FD_ISSET(devfd, &read_fds)) {
            size_t n = read_data_socket(devfd, buf, sizeof(buf));
            if (n == 0) {
                break;
            }
            write(STDOUT_FILENO, buf, n);
        }
    }

    socket_close(devfd);
    return ret;
}

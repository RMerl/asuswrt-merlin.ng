/*
 * iproxy.c -- proxy that enables tcp service access to iOS devices
 *
 * Copyright (C) 2009-2020 Nikias Bassen <nikias@gmx.li>
 * Copyright (C) 2014      Martin Szulecki <m.szulecki@libimobiledevice.org>
 * Copyright (C) 2009      Paul Sladen <libiphone@paul.sladen.org>
 *
 * Based upon iTunnel source code, Copyright (c) 2008 Jing Su.
 * http://www.cs.toronto.edu/~jingsu/itunnel/
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

#define TOOL_NAME "iproxy"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
typedef unsigned int socklen_t;
#else
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
#include <signal.h>
#endif
#include "socket.h"
#include "usbmuxd.h"

#ifndef ETIMEDOUT
#define ETIMEDOUT 138
#endif

static int debug_level = 0;

struct client_data {
	int fd;
	int sfd;
	char* udid;
	enum usbmux_lookup_options lookup_opts;
	uint16_t device_port;
};

#define CDATA_FREE(x) if (x) { \
	if (x->fd > 0) socket_close(x->fd); \
	if (x->sfd > 0) socket_close(x->sfd); \
	free(x->udid); \
	free(x); \
}

static void *acceptor_thread(void *arg)
{
	char buffer[32768];
	struct client_data *cdata = (struct client_data*)arg;
	usbmuxd_device_info_t *dev_list = NULL;
	usbmuxd_device_info_t *dev = NULL;
	usbmuxd_device_info_t muxdev;
	int count;

	if (!cdata) {
		fprintf(stderr, "invalid client_data provided!\n");
		return NULL;
	}

	if (cdata->udid) {
		if (usbmuxd_get_device(cdata->udid, &muxdev, cdata->lookup_opts) > 0) {
			dev = &muxdev;
		}
	} else {
		if ((count = usbmuxd_get_device_list(&dev_list)) < 0) {
			printf("Connecting to usbmuxd failed, terminating.\n");
			free(dev_list);
			CDATA_FREE(cdata);
			return NULL;
		}

		if (dev_list == NULL || dev_list[0].handle == 0) {
			printf("No connected device found, terminating.\n");
			free(dev_list);
			CDATA_FREE(cdata);
			return NULL;
		}

		int i;
		for (i = 0; i < count; i++) {
			if (dev_list[i].conn_type == CONNECTION_TYPE_USB && (cdata->lookup_opts & DEVICE_LOOKUP_USBMUX)) {
				dev = &(dev_list[i]);
				break;
			} else if (dev_list[i].conn_type == CONNECTION_TYPE_NETWORK && (cdata->lookup_opts & DEVICE_LOOKUP_NETWORK)) {
				dev = &(dev_list[i]);
				break;
			}
		}
	}

	if (dev == NULL || dev->handle == 0) {
		printf("No connected/matching device found, disconnecting client.\n");
		free(dev_list);
		CDATA_FREE(cdata);
		return NULL;
	}

	cdata->sfd = -1;
	if (dev->conn_type == CONNECTION_TYPE_NETWORK) {
		struct sockaddr_storage saddr_storage;
		struct sockaddr* saddr = (struct sockaddr*)&saddr_storage;

		if (((char*)dev->conn_data)[1] == 0x02) { // AF_INET
			saddr->sa_family = AF_INET;
			memcpy(&saddr->sa_data[0], (char*)dev->conn_data + 2, 14);
		}
		else if (((char*)dev->conn_data)[1] == 0x1E) { //AF_INET6 (bsd)
#ifdef AF_INET6
			saddr->sa_family = AF_INET6;
			/* copy the address and the host dependent scope id */
			memcpy(&saddr->sa_data[0], (char*)dev->conn_data + 2, 26);
#else
			fprintf(stderr, "ERROR: Got an IPv6 address but this system doesn't support IPv6\n");
			CDATA_FREE(cdata);
			return NULL;
#endif
		}
		else {
			fprintf(stderr, "Unsupported address family 0x%02x\n", ((char*)dev->conn_data)[1]);
			CDATA_FREE(cdata);
			return NULL;
		}
		char addrtxt[48];
		addrtxt[0] = '\0';
		if (!socket_addr_to_string(saddr, addrtxt, sizeof(addrtxt))) {
			fprintf(stderr, "Failed to convert network address: %d (%s)\n", errno, strerror(errno));
		}
		fprintf(stdout, "Requesting connecion to NETWORK device %s (serial: %s), port %d\n", addrtxt, dev->udid, cdata->device_port);
		cdata->sfd = socket_connect_addr(saddr, cdata->device_port);
	} else if (dev->conn_type == CONNECTION_TYPE_USB) {
		fprintf(stdout, "Requesting connecion to USB device handle %d (serial: %s), port %d\n", dev->handle, dev->udid, cdata->device_port);

		cdata->sfd = usbmuxd_connect(dev->handle, cdata->device_port);
	}
	free(dev_list);
	if (cdata->sfd < 0) {
		fprintf(stderr, "Error connecting to device: %s\n", strerror(errno));
	} else {
		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(cdata->fd, &fds);
		FD_SET(cdata->sfd, &fds);

		while (1) {
			fd_set read_fds = fds;
			int ret_sel = select(cdata->sfd+1, &read_fds, NULL, NULL, NULL);
			if (ret_sel < 0) {
				perror("select");
				break;
			}
			if (FD_ISSET(cdata->fd, &read_fds)) {
				int r = socket_receive_timeout(cdata->fd, buffer, sizeof(buffer), 0, 100);
				if (r <= 0) {
					break;
				}
				int sent = 0;
				while (sent < r) {
					int s = socket_send(cdata->sfd, buffer+sent, r-sent);
					if (s <= 0) {
						break;
					}
					sent += s;
				}
			}
			if (FD_ISSET(cdata->sfd, &read_fds)) {
				int r = socket_receive_timeout(cdata->sfd, buffer, sizeof(buffer), 0, 100);
				if (r <= 0) {
					break;
				}
				int sent = 0;
				while (sent < r) {
					int s = socket_send(cdata->fd, buffer+sent, r-sent);
					if (s <= 0) {
						break;
					}
					sent += s;
				}
			}
		}
	}

	CDATA_FREE(cdata);

	return NULL;
}

static void print_usage(int argc, char **argv, int is_error)
{
	char *name = NULL;
	name = strrchr(argv[0], '/');
	fprintf(is_error ? stderr : stdout, "Usage: %s [OPTIONS] LOCAL_PORT:DEVICE_PORT [LOCAL_PORT2:DEVICE_PORT2 ...]\n", (name ? name+1 : argv[0]));
	fprintf(is_error ? stderr : stdout,
		"\n" \
		"Proxy that binds local TCP ports to be forwarded to the specified ports on a usbmux device.\n" \
		"\n" \
		"OPTIONS:\n" \
		"  -u, --udid UDID    target specific device by UDID\n" \
		"  -n, --network      connect to network device\n" \
		"  -l, --local        connect to USB device (default)\n" \
		"  -s, --source ADDR  source address for listening socket (default 127.0.0.1)\n" \
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
	char* device_udid = NULL;
	char* source_addr = NULL;
	uint16_t listen_port[16];
	uint16_t device_port[16];
#ifdef AF_INET6
#define MAX_LISTEN_NUM 32
#else
#define MAX_LISTEN_NUM 16
#endif
	struct listen_sock {
		int fd;
		int index;
	} listen_sock[MAX_LISTEN_NUM];
	int num_listen = 0;
	int num_pairs = 0;
	int i = 0;
	enum usbmux_lookup_options lookup_opts = 0;

	const struct option longopts[] = {
		{ "debug", no_argument, NULL, 'd' },
		{ "help", no_argument, NULL, 'h' },
		{ "udid", required_argument, NULL, 'u' },
		{ "local", no_argument, NULL, 'l' },
		{ "network", no_argument, NULL, 'n' },
		{ "source", required_argument, NULL, 's' },
		{ "version", no_argument, NULL, 'v' },
		{ NULL, 0, NULL, 0}
	};
	int c = 0;
	while ((c = getopt_long(argc, argv, "dhu:lns:v", longopts, NULL)) != -1) {
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
		case 's':
			if (!*optarg) {
				fprintf(stderr, "ERROR: source address must not be empty!\n");
				print_usage(argc, argv, 1);
				return 2;
			}
			free(source_addr);
			source_addr = strdup(optarg);
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

	if (argc == 0) {
		fprintf(stderr, "ERROR: Not enough parameters. Need at least one pair of ports.\n");
		print_usage(argc + optind, argv - optind, 1);
		free(device_udid);
		free(source_addr);
		return 2;
	}

	if (argc == 2 && (strchr(argv[0], ':') == NULL) && (strchr(argv[1], ':') == NULL)) {
		/* support old-style port pair specification */
		char* endp = NULL;
		listen_port[0] = (uint16_t)strtol(argv[0], &endp, 10);
		if (!listen_port[0] || *endp != '\0') {
			fprintf(stderr, "Invalid listen port specified in argument '%s'!\n", argv[0]);
			free(device_udid);
			free(source_addr);
			return -EINVAL;
		}
		endp = NULL;
		device_port[0] = (uint16_t)strtol(argv[1], &endp, 10);
		if (!device_port[0] || *endp != '\0') {
			fprintf(stderr, "Invalid device port specified in argument '%s'!\n", argv[1]);
			free(device_udid);
			free(source_addr);
			return -EINVAL;
		}
		num_pairs = 1;
	} else {
		/* new style, colon-separated local:device port pairs */
		for (i = 0; i < argc; i++) {
			char* endp = NULL;
			listen_port[i] = (uint16_t)strtol(argv[i], &endp, 10);
			if (!listen_port[i] || (*endp != ':')) {
				fprintf(stderr, "Invalid listen port specified in argument '%s'!\n", argv[i]);
				free(device_udid);
				free(source_addr);
				return -EINVAL;
			}
			device_port[i] = (uint16_t)strtol(endp+1, &endp, 10);
			if (!device_port[i] || (*endp != '\0')) {
				fprintf(stderr, "Invalid device port specified in argument '%s'!\n", argv[i+1]);
				free(device_udid);
				free(source_addr);
				return -EINVAL;
			}
		}
		num_pairs = argc;
	}

	if (num_pairs > 16) {
		fprintf(stderr, "ERROR: Too many LOCAL:DEVICE port pairs. Maximum is 16.\n");
		return -1;
	}

#ifndef WIN32
	signal(SIGPIPE, SIG_IGN);
#endif

	// first create the listening sockets
	for (i = 0; i < num_pairs; i++) {
		printf("Creating listening port %d for device port %d\n", listen_port[i], device_port[i]);
		if (!source_addr) {
			listen_sock[num_listen].fd = socket_create("127.0.0.1", listen_port[i]);
			if (listen_sock[num_listen].fd < 0) {
				int j;
				fprintf(stderr, "Error creating socket for listen port %u: %s\n", listen_port[i], strerror(errno));
				free(source_addr);
				free(device_udid);
				for (j = num_listen; j >= 0; j--) {
					socket_close(listen_sock[j].fd);
				}
				return -errno;
			}
			listen_sock[num_listen].index = i;
			num_listen++;
#if defined(AF_INET6)
			listen_sock[num_listen].fd = socket_create("::1", listen_port[i]);
			if (listen_sock[num_listen].fd < 0) {
				int j;
				fprintf(stderr, "Error creating socket for listen port %u: %s\n", listen_port[i], strerror(errno));
				free(source_addr);
				free(device_udid);
				for (j = num_listen; j >= 0; j--) {
					socket_close(listen_sock[j].fd);
				}
				return -errno;
			}
			listen_sock[num_listen].index = i;
			num_listen++;
#endif
		} else {
			listen_sock[num_listen].fd = socket_create(source_addr, listen_port[i]);
			if (listen_sock[num_listen].fd < 0) {
				int j;
				fprintf(stderr, "Error creating socket for listen port %u: %s\n", listen_port[i], strerror(errno));
				free(source_addr);
				free(device_udid);
				for (j = num_listen; j >= 0; j--) {
					socket_close(listen_sock[j].fd);
				}
				return -errno;
			}
			listen_sock[num_listen].index = i;
			num_listen++;
		}
	}

	// make them non-blocking and add them to fd set
	fd_set fds;
	FD_ZERO(&fds);
	for (i = 0; i < num_listen; i++) {
#ifdef WIN32
		u_long l_yes = 1;
		ioctlsocket(listen_sock[i].fd, FIONBIO, &l_yes);
#else
		int flags = fcntl(listen_sock[i].fd, F_GETFL, 0);
		fcntl(listen_sock[i].fd, F_SETFL, flags | O_NONBLOCK);
#endif
		FD_SET(listen_sock[i].fd, &fds);
	}

	// main loop
	while (1) {
		printf("waiting for connection\n");
		fd_set read_fds = fds;
		int ret_sel = select(listen_sock[num_listen-1].fd+1, &read_fds, NULL, NULL, NULL);
		if (ret_sel < 0) {
			perror("select");
			break;
		}
		for (i = 0; i < num_listen; i++) {
			if (FD_ISSET(listen_sock[i].fd, &read_fds)) {
#ifdef WIN32
				HANDLE acceptor = NULL;
#else
				pthread_t acceptor;
#endif
				struct client_data *cdata;
				int c_sock = socket_accept(listen_sock[i].fd, listen_port[listen_sock[i].index]);
				if (c_sock < 0) {
					fprintf(stderr, "accept: %s\n", strerror(errno));
					break;
				} else {
					printf("New connection for %d->%d, fd = %d\n", listen_port[listen_sock[i].index], device_port[listen_sock[i].index], c_sock);
					cdata = (struct client_data*)malloc(sizeof(struct client_data));
					if (!cdata) {
						socket_close(c_sock);
						fprintf(stderr, "ERROR: Out of memory\n");
						free(device_udid);
						return -1;
					}
					cdata->fd = c_sock;
					cdata->sfd = -1;
					cdata->udid = (device_udid) ? strdup(device_udid) : NULL;
					cdata->lookup_opts = lookup_opts;
					cdata->device_port = device_port[listen_sock[i].index];
#ifdef WIN32
					acceptor = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)acceptor_thread, cdata, 0, NULL);
					CloseHandle(acceptor);
#else
					pthread_create(&acceptor, NULL, acceptor_thread, cdata);
					pthread_detach(acceptor);
#endif
				}
			}
		}
	}

	for (i = 0; i < num_listen; i++) {
		socket_close(listen_sock[i].fd);
	}

	free(device_udid);
	free(source_addr);

	return 0;
}

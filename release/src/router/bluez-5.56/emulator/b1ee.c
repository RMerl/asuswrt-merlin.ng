// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2012  Intel Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>

#include <netdb.h>
#include <arpa/inet.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"

#include "src/shared/mainloop.h"

#define DEFAULT_HOST_PORT	"45550"		/* 0xb1ee */
#define DEFAULT_SNIFFER_PORT	"45551"		/* 0xb1ef */

static int sniffer_fd;
static int server_fd;
static int vhci_fd;

static void usage(void)
{
	printf("b1ee - Bluetooth device testing tool over internet\n"
		"Usage:\n");
	printf("\tb1ee [options] <host>\n");
	printf("options:\n"
		"\t-p, --port <port>          Specify the server port\n"
		"\t-s, --sniffer-port <port>  Specify the sniffer port\n"
		"\t-v, --version              Show version information\n"
		"\t-h, --help                 Show help options\n");
}

static const struct option main_options[] = {
	{ "port",		required_argument,	NULL, 'p' },
	{ "sniffer-port",	required_argument,	NULL, 's' },
	{ "version",		no_argument,		NULL, 'v' },
	{ "help",		no_argument,		NULL, 'h' },
	{ }
};

static char *set_port(char *str)
{
	char *c;

	if (str == NULL || str[0] == '\0')
		return NULL;

	for (c = str; *c != '\0'; c++)
		if (isdigit(*c) == 0)
			return NULL;

	if (atol(str) > 65535)
		return NULL;

	return strdup(str);
}

static void sniffer_read_callback(int fd, uint32_t events, void *user_data)
{
	static uint8_t buf[4096];
	ssize_t len;

	if (events & (EPOLLERR | EPOLLHUP))
		return;

again:
	len = recv(fd, buf, sizeof(buf), MSG_DONTWAIT);
	if (len < 0) {
		if (errno == EAGAIN)
			goto again;
		return;
	}

	printf("Sniffer received: %zi bytes\n", len);
}

static uint8_t *server_pkt_data;
static uint8_t server_pkt_type;
static uint16_t server_pkt_expect;
static uint16_t server_pkt_len;
static uint16_t server_pkt_offset;

static void server_read_callback(int fd, uint32_t events, void *user_data)
{
	static uint8_t buf[4096];
	uint8_t *ptr = buf;
	ssize_t len;
	uint16_t count;

	if (events & (EPOLLERR | EPOLLHUP))
		return;

again:
	len = recv(fd, buf + server_pkt_offset,
			sizeof(buf) - server_pkt_offset, MSG_DONTWAIT);
	if (len < 0) {
		if (errno == EAGAIN)
			goto again;
		return;
	}

	count = server_pkt_offset + len;

	while (count > 0) {
		hci_event_hdr *evt_hdr;

		if (!server_pkt_data) {
			server_pkt_type = ptr[0];

			switch (server_pkt_type) {
			case HCI_EVENT_PKT:
				if (count < HCI_EVENT_HDR_SIZE + 1) {
					server_pkt_offset += len;
					return;
				}
				evt_hdr = (hci_event_hdr *) (ptr + 1);
				server_pkt_expect = HCI_EVENT_HDR_SIZE +
							evt_hdr->plen + 1;
				server_pkt_data = malloc(server_pkt_expect);
				server_pkt_len = 0;
				break;
			default:
				fprintf(stderr, "Unknown packet from server\n");
				return;
			}

			server_pkt_offset = 0;
		}

		if (count >= server_pkt_expect) {
			ssize_t written;

			memcpy(server_pkt_data + server_pkt_len,
						ptr, server_pkt_expect);
			ptr += server_pkt_expect;
			count -= server_pkt_expect;

			written = write(vhci_fd, server_pkt_data,
					server_pkt_len + server_pkt_expect);
			if (written != server_pkt_len + server_pkt_expect)
				fprintf(stderr, "Write to /dev/vhci failed\n");

			free(server_pkt_data);
			server_pkt_data = NULL;
		} else {
			memcpy(server_pkt_data + server_pkt_len, ptr, count);
			server_pkt_len += count;
			server_pkt_expect -= count;
			count = 0;
		}
	}
}

static void vhci_read_callback(int fd, uint32_t events, void *user_data)
{
	unsigned char buf[4096];
	ssize_t len, written;

	if (events & (EPOLLERR | EPOLLHUP))
		return;

	len = read(fd, buf, sizeof(buf));
	if (len < 0)
		return;

	written = write(server_fd, buf, len);
	if (written != len)
		fprintf(stderr, "Write to server failed\n");
}

static void signal_callback(int signum, void *user_data)
{
	switch (signum) {
	case SIGINT:
	case SIGTERM:
		mainloop_quit();
		break;
	}
}

static int do_connect(const char *node, const char *service)
{
	struct addrinfo hints;
	struct addrinfo *info, *res;
	int err, fd = -1;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	err = getaddrinfo(node, service, &hints, &res);
	if (err) {
		perror(gai_strerror(err));
		exit(1);
	}

	for (info = res; info; info = info->ai_next) {
		char str[INET6_ADDRSTRLEN];

		inet_ntop(info->ai_family, info->ai_addr->sa_data,
							str, sizeof(str));

		fd = socket(info->ai_family, info->ai_socktype,
						info->ai_protocol);
		if (fd < 0)
			continue;

		printf("Trying to connect to %s on port %s\n", str, service);

		if (connect(fd, res->ai_addr, res->ai_addrlen) < 0) {
			perror("Failed to connect");
			close(fd);
			continue;
		}

		printf("Successfully connected to %s on port %s\n",
							str, service);
		break;
	}

	freeaddrinfo(res);

	if (res == NULL)
		exit(1);

	return fd;
}

int main(int argc, char *argv[])
{
	const char sniff_cmd[] = { 0x01, 0x00,
					0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	char *server_port = NULL, *sniffer_port = NULL;
	int ret = EXIT_FAILURE;
	ssize_t written;

	for (;;) {
		int opt;

		opt = getopt_long(argc, argv, "s:p:vh", main_options, NULL);
		if (opt < 0)
			break;

		switch (opt) {
		case 'p':
			server_port = set_port(optarg);
			if (server_port == NULL)
				goto usage;

			break;
		case 's':
			sniffer_port = set_port(optarg);
			if (sniffer_port == NULL)
				goto usage;

			break;
		case 'v':
			printf("%s\n", VERSION);
			ret = EXIT_SUCCESS;
			goto done;
		case 'h':
			ret = EXIT_SUCCESS;
			goto usage;
		default:
			goto usage;
		}
	}

	argc = argc - optind;
	argv = argv + optind;
	optind = 0;

	if (argv[0] == NULL || argv[0][0] == '\0')
		goto usage;

	server_fd = do_connect(argv[0], server_port ? : DEFAULT_HOST_PORT);
	sniffer_fd = do_connect(argv[0],
				sniffer_port ? : DEFAULT_SNIFFER_PORT);

	written = write(sniffer_fd, sniff_cmd, sizeof(sniff_cmd));
	if (written < 0)
		perror("Failed to enable sniffer");

	vhci_fd = open("/dev/vhci", O_RDWR | O_NONBLOCK);
	if (vhci_fd < 0) {
		perror("Failed to /dev/vhci");
		close(server_fd);
		exit(1);
	}

	mainloop_init();

	mainloop_add_fd(sniffer_fd, EPOLLIN, sniffer_read_callback, NULL, NULL);
	mainloop_add_fd(server_fd, EPOLLIN, server_read_callback, NULL, NULL);
	mainloop_add_fd(vhci_fd, EPOLLIN, vhci_read_callback, NULL, NULL);

	ret = mainloop_run_with_signal(signal_callback, NULL);

	goto done;

usage:
	usage();
done:
	free(server_port);
	free(sniffer_port);

	return ret;
}

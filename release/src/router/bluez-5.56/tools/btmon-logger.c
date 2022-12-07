// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2017-2018  Codecoup
 *  Copyright (C) 2011-2014  Intel Corporation
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <libgen.h>
#include <errno.h>

#include <linux/capability.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"

#include "src/shared/util.h"
#include "src/shared/mainloop.h"
#include "src/shared/btsnoop.h"

#define MONITOR_INDEX_NONE 0xffff

struct monitor_hdr {
	uint16_t opcode;
	uint16_t index;
	uint16_t len;
} __attribute__ ((packed));

static struct btsnoop *btsnoop_file = NULL;

static void data_callback(int fd, uint32_t events, void *user_data)
{
	uint8_t buf[BTSNOOP_MAX_PACKET_SIZE];
	unsigned char control[64];
	struct monitor_hdr hdr;
	struct msghdr msg;
	struct iovec iov[2];

	if (events & (EPOLLERR | EPOLLHUP)) {
		mainloop_exit_failure();
		return;
	}

	iov[0].iov_base = &hdr;
	iov[0].iov_len = sizeof(hdr);
	iov[1].iov_base = buf;
	iov[1].iov_len = sizeof(buf);

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;
	msg.msg_control = control;
	msg.msg_controllen = sizeof(control);

	while (1) {
		struct cmsghdr *cmsg;
		struct timeval *tv = NULL;
		struct timeval ctv;
		uint16_t opcode, index, pktlen;
		ssize_t len;

		len = recvmsg(fd, &msg, MSG_DONTWAIT);
		if (len < 0)
			break;

		if (len < (ssize_t) sizeof(hdr))
			break;

		for (cmsg = CMSG_FIRSTHDR(&msg); cmsg != NULL;
					cmsg = CMSG_NXTHDR(&msg, cmsg)) {
			if (cmsg->cmsg_level != SOL_SOCKET)
				continue;

			if (cmsg->cmsg_type == SCM_TIMESTAMP) {
				memcpy(&ctv, CMSG_DATA(cmsg), sizeof(ctv));
				tv = &ctv;
			}
		}

		opcode = le16_to_cpu(hdr.opcode);
		index  = le16_to_cpu(hdr.index);
		pktlen = le16_to_cpu(hdr.len);

		btsnoop_write_hci(btsnoop_file, tv, index, opcode, 0, buf,
									pktlen);
	}
}

static bool open_monitor_channel(void)
{
	struct sockaddr_hci addr;
	int fd, opt = 1;

	fd = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
	if (fd < 0) {
		perror("Failed to open monitor channel");
		return false;
	}

	memset(&addr, 0, sizeof(addr));
	addr.hci_family = AF_BLUETOOTH;
	addr.hci_dev = HCI_DEV_NONE;
	addr.hci_channel = HCI_CHANNEL_MONITOR;

	if (bind(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("Failed to bind monitor channel");
		close(fd);
		return false;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_TIMESTAMP, &opt, sizeof(opt)) < 0) {
		perror("Failed to enable timestamps");
		close(fd);
		return false;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_PASSCRED, &opt, sizeof(opt)) < 0) {
		perror("Failed to enable credentials");
		close(fd);
		return false;
	}

	mainloop_add_fd(fd, EPOLLIN, data_callback, NULL, NULL);

	return true;
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

extern int capget(struct __user_cap_header_struct *header,
					struct __user_cap_data_struct *data);
extern int capset(struct __user_cap_header_struct *header,
				const struct __user_cap_data_struct *data);

static void drop_capabilities(void)
{
	struct __user_cap_header_struct header;
	struct __user_cap_data_struct cap;
	unsigned int mask;
	int err;

	header.version = _LINUX_CAPABILITY_VERSION_3;
	header.pid = 0;

	err = capget(&header, &cap);
	if (err) {
		perror("Unable to get current capabilities");
		return;
	}

	/* not needed anymore since monitor socket is already open */
	mask = ~CAP_TO_MASK(CAP_NET_RAW);

	cap.effective &= mask;
	cap.permitted &= mask;
	cap.inheritable &= mask;

	err = capset(&header, &cap);
	if (err)
		perror("Failed to set capabilities");
}

static void usage(void)
{
	printf("btmon-logger - Bluetooth monitor\n"
		"Usage:\n");
	printf("\tbtmon-logger [options]\n");
	printf("options:\n"
		"\t-b, --basename <path>  Save traces in specified path\n"
		"\t-p, --parents          Create basename parent directories\n"
		"\t-l, --limit <limit>    Limit traces file size (rotate)\n"
		"\t-c, --count <count>    Limit number of rotated files\n"
		"\t-v, --version          Show version\n"
		"\t-h, --help             Show help options\n");
}

static const struct option main_options[] = {
	{ "basename",	required_argument,	NULL, 'b' },
	{ "parents",	no_argument,		NULL, 'p' },
	{ "limit",	required_argument,	NULL, 'l' },
	{ "count",	required_argument,	NULL, 'c' },
	{ "version",	no_argument,		NULL, 'v' },
	{ "help",	no_argument,		NULL, 'h' },
	{ }
};

static int create_dir(const char *filename)
{
	char *dirc;
	char *dir;
	char *p;
	int err = 0;

	/* get base directory */
	dirc = strdup(filename);
	dir = dirname(dirc);

	p = dir;

	/* preserve leading / if present */
	if (*p == '/')
		p++;

	/* create any intermediate directories */
	p = strchrnul(p, '/');
	while (*p) {
		/* cut directory path */
		*p = '\0';

		if (mkdir(dir, 0700) < 0 && errno != EEXIST) {
			err = errno;
			goto done;
		}

		/* restore directory path */
		*p = '/';
		p = strchrnul(p + 1, '/');
	}

	/* create leaf directory */
	if (mkdir(dir, 0700) < 0 && errno != EEXIST)
		err = errno;

done:
	free(dirc);

	if (err)
		printf("Failed to create parent directories for %s\n",
								filename);

	return err;
}

int main(int argc, char *argv[])
{
	const char *path = "hci.log";
	unsigned long max_count = 0;
	size_t size_limit = 0;
	bool parents = false;
	int exit_status;
	char *endptr;

	mainloop_init();

	mainloop_sd_notify("STATUS=Starting up");

	while (true) {
		int opt;

		opt = getopt_long(argc, argv, "b:l:c:vhp", main_options,
									NULL);
		if (opt < 0)
			break;

		switch (opt) {
		case 'b':
			path = optarg;
			if (strlen(path) > PATH_MAX) {
				fprintf(stderr, "Too long path\n");
				return EXIT_FAILURE;
			}
			break;
		case 'l':
			size_limit = strtoul(optarg, &endptr, 10);

			if (size_limit == ULONG_MAX) {
				fprintf(stderr, "Invalid limit\n");
				return EXIT_FAILURE;
			}

			if (*endptr != '\0') {
				if (*endptr == 'K' || *endptr == 'k') {
					size_limit *= 1024;
				} else if (*endptr == 'M' || *endptr == 'm') {
					size_limit *= 1024 * 1024;
				} else {
					fprintf(stderr, "Invalid limit\n");
					return EXIT_FAILURE;
				}
			}

			/* limit this to reasonable size */
			if (size_limit < 4096) {
				fprintf(stderr, "Too small limit value\n");
				return EXIT_FAILURE;
			}
			break;
		case 'c':
			max_count = strtoul(optarg, &endptr, 10);
			break;
		case 'p':
			if (getppid() != 1) {
				fprintf(stderr, "Parents option allowed only "
						"when running as a service\n");
				return EXIT_FAILURE;
			}

			parents = true;
			break;
		case 'v':
			printf("%s\n", VERSION);
			return EXIT_SUCCESS;
		case 'h':
			usage();
			return EXIT_SUCCESS;
		default:
			return EXIT_FAILURE;
		}
	}

	if (argc - optind > 0) {
		fprintf(stderr, "Invalid command line parameters\n");
		return EXIT_FAILURE;
	}

	if (!open_monitor_channel())
		return EXIT_FAILURE;

	if (parents && create_dir(path) < 0)
		return EXIT_FAILURE;

	btsnoop_file = btsnoop_create(path, size_limit, max_count,
							BTSNOOP_FORMAT_MONITOR);
	if (!btsnoop_file)
		return EXIT_FAILURE;

	drop_capabilities();

	printf("Bluetooth monitor logger ver %s\n", VERSION);

	mainloop_sd_notify("STATUS=Running");
	mainloop_sd_notify("READY=1");

	exit_status = mainloop_run_with_signal(signal_callback, NULL);

	mainloop_sd_notify("STATUS=Quitting");

	btsnoop_unref(btsnoop_file);

	return exit_status;
}

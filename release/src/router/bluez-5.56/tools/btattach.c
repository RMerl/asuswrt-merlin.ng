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
#include <termios.h>
#include <sys/ioctl.h>
#include <poll.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"

#include "hciattach.h"
#include "monitor/bt.h"
#include "src/shared/mainloop.h"
#include "src/shared/timeout.h"
#include "src/shared/util.h"
#include "src/shared/tty.h"
#include "src/shared/hci.h"

static int open_serial(const char *path, unsigned int speed, bool flowctl)
{
	struct termios ti;
	int fd, saved_ldisc, ldisc = N_HCI;

	fd = open(path, O_RDWR | O_NOCTTY);
	if (fd < 0) {
		perror("Failed to open serial port");
		return -1;
	}

	if (tcflush(fd, TCIOFLUSH) < 0) {
		perror("Failed to flush serial port");
		close(fd);
		return -1;
	}

	if (ioctl(fd, TIOCGETD, &saved_ldisc) < 0) {
		perror("Failed get serial line discipline");
		close(fd);
		return -1;
	}

	/* Switch TTY to raw mode */
	memset(&ti, 0, sizeof(ti));
	cfmakeraw(&ti);

	ti.c_cflag |= (speed | CLOCAL | CREAD);

	if (flowctl) {
		/* Set flow control */
		ti.c_cflag |= CRTSCTS;
	}

	if (tcsetattr(fd, TCSANOW, &ti) < 0) {
		perror("Failed to set serial port settings");
		close(fd);
		return -1;
	}

	if (ioctl(fd, TIOCSETD, &ldisc) < 0) {
		perror("Failed set serial line discipline");
		close(fd);
		return -1;
	}

	printf("Switched line discipline from %d to %d\n", saved_ldisc, ldisc);

	return fd;
}

static void local_version_callback(const void *data, uint8_t size,
							void *user_data)
{
	const struct bt_hci_rsp_read_local_version *rsp = data;

	printf("Manufacturer: %u\n", le16_to_cpu(rsp->manufacturer));
}

static int attach_proto(const char *path, unsigned int proto,
			unsigned int speed, bool flowctl, unsigned int flags)
{
	int fd, dev_id;

	fd = open_serial(path, speed, flowctl);
	if (fd < 0)
		return -1;

	if (ioctl(fd, HCIUARTSETFLAGS, flags) < 0) {
		perror("Failed to set flags");
		close(fd);
		return -1;
	}

	if (ioctl(fd, HCIUARTSETPROTO, proto) < 0) {
		perror("Failed to set protocol");
		close(fd);
		return -1;
	}

	dev_id = ioctl(fd, HCIUARTGETDEVICE);
	if (dev_id < 0) {
		perror("Failed to get device id");
		close(fd);
		return -1;
	}

	printf("Device index %d attached\n", dev_id);

	if (flags & (1 << HCI_UART_RAW_DEVICE)) {
		unsigned int attempts = 6;
		struct bt_hci *hci;

		while (attempts-- > 0) {
			hci = bt_hci_new_user_channel(dev_id);
			if (hci)
				break;

			usleep(250 * 1000);
		}

		if (!hci) {
			fprintf(stderr, "Failed to open HCI user channel\n");
			close(fd);
			return -1;
		}

		bt_hci_send(hci, BT_HCI_CMD_READ_LOCAL_VERSION, NULL, 0,
					local_version_callback, hci,
					(bt_hci_destroy_func_t) bt_hci_unref);
	}

	return fd;
}

static void uart_callback(int fd, uint32_t events, void *user_data)
{
	printf("UART callback handling\n");
}

static void signal_callback(int signum, void *user_data)
{
	static bool terminated = false;

	switch (signum) {
	case SIGINT:
	case SIGTERM:
		if (!terminated) {
			mainloop_quit();
			terminated = true;
		}
		break;
	}
}
static void usage(void)
{
	printf("btattach - Bluetooth serial utility\n"
		"Usage:\n");
	printf("\tbtattach [options]\n");
	printf("options:\n"
		"\t-B, --bredr <device>   Attach Primary controller\n"
		"\t-A, --amp <device>     Attach AMP controller\n"
		"\t-P, --protocol <proto> Specify protocol type\n"
		"\t-S, --speed <baudrate> Specify which baudrate to use\n"
		"\t-N, --noflowctl        Disable flow control\n"
		"\t-h, --help             Show help options\n");
}

static const struct option main_options[] = {
	{ "bredr",    required_argument, NULL, 'B' },
	{ "amp",      required_argument, NULL, 'A' },
	{ "protocol", required_argument, NULL, 'P' },
	{ "speed",    required_argument, NULL, 'S' },
	{ "noflowctl",no_argument,       NULL, 'N' },
	{ "version",  no_argument,       NULL, 'v' },
	{ "help",     no_argument,       NULL, 'h' },
	{ }
};

static const struct {
	const char *name;
	unsigned int id;
} proto_table[] = {
	{ "h4",    HCI_UART_H4    },
	{ "bcsp",  HCI_UART_BCSP  },
	{ "3wire", HCI_UART_3WIRE },
	{ "h4ds",  HCI_UART_H4DS  },
	{ "ll",    HCI_UART_LL    },
	{ "ath3k", HCI_UART_ATH3K },
	{ "intel", HCI_UART_INTEL },
	{ "bcm",   HCI_UART_BCM   },
	{ "qca",   HCI_UART_QCA   },
	{ "ag6xx", HCI_UART_AG6XX },
	{ "nokia", HCI_UART_NOKIA },
	{ "mrvl",  HCI_UART_MRVL  },
	{ }
};

int main(int argc, char *argv[])
{
	const char *bredr_path = NULL, *amp_path = NULL, *proto = NULL;
	bool flowctl = true, raw_device = false;
	int exit_status, count = 0, proto_id = HCI_UART_H4;
	unsigned int speed = B115200;

	for (;;) {
		int opt;

		opt = getopt_long(argc, argv, "B:A:P:S:NRvh",
						main_options, NULL);
		if (opt < 0)
			break;

		switch (opt) {
		case 'B':
			bredr_path = optarg;
			break;
		case 'A':
			amp_path = optarg;
			break;
		case 'P':
			proto = optarg;
			break;
		case 'S':
			speed = tty_get_speed(atoi(optarg));
			if (!speed) {
				fprintf(stderr, "Invalid speed: %s\n", optarg);
				return EXIT_FAILURE;
			}
			break;
		case 'N':
			flowctl = false;
			break;
		case 'R':
			raw_device = true;
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

	mainloop_init();

	if (proto) {
		unsigned int i;

		for (i = 0; proto_table[i].name; i++) {
			if (!strcmp(proto_table[i].name, proto)) {
				proto_id = proto_table[i].id;
				break;
			}
		}

		if (!proto_table[i].name) {
			fprintf(stderr, "Invalid protocol\n");
			return EXIT_FAILURE;
		}
	}

	if (bredr_path) {
		unsigned long flags;
		int fd;

		printf("Attaching Primary controller to %s\n", bredr_path);

		flags = (1 << HCI_UART_RESET_ON_INIT);

		if (raw_device)
			flags = (1 << HCI_UART_RAW_DEVICE);

		fd = attach_proto(bredr_path, proto_id, speed, flowctl, flags);
		if (fd >= 0) {
			mainloop_add_fd(fd, 0, uart_callback, NULL, NULL);
			count++;
		}
	}

	if (amp_path) {
		unsigned long flags;
		int fd;

		printf("Attaching AMP controller to %s\n", amp_path);

		flags = (1 << HCI_UART_RESET_ON_INIT) |
			(1 << HCI_UART_CREATE_AMP);

		if (raw_device)
			flags = (1 << HCI_UART_RAW_DEVICE);

		fd = attach_proto(amp_path, proto_id, speed, flowctl, flags);
		if (fd >= 0) {
			mainloop_add_fd(fd, 0, uart_callback, NULL, NULL);
			count++;
		}
	}

	if (count < 1) {
		fprintf(stderr, "No controller attached\n");
		return EXIT_FAILURE;
	}

	exit_status = mainloop_run_with_signal(signal_callback, NULL);

	return exit_status;
}

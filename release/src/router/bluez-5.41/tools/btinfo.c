/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011-2012  Intel Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "monitor/bt.h"
#include "src/shared/mainloop.h"
#include "src/shared/timeout.h"
#include "src/shared/util.h"
#include "src/shared/hci.h"

#define BTPROTO_HCI	1

struct hci_dev_stats {
	uint32_t err_rx;
	uint32_t err_tx;
	uint32_t cmd_tx;
	uint32_t evt_rx;
	uint32_t acl_tx;
	uint32_t acl_rx;
	uint32_t sco_tx;
	uint32_t sco_rx;
	uint32_t byte_rx;
	uint32_t byte_tx;
};

struct hci_dev_info {
	uint16_t dev_id;
	char     name[8];
	uint8_t  bdaddr[6];
	uint32_t flags;
	uint8_t  type;
	uint8_t  features[8];
	uint32_t pkt_type;
	uint32_t link_policy;
	uint32_t link_mode;
	uint16_t acl_mtu;
	uint16_t acl_pkts;
	uint16_t sco_mtu;
	uint16_t sco_pkts;
	struct   hci_dev_stats stat;
};

#define HCIDEVUP	_IOW('H', 201, int)
#define HCIDEVDOWN	_IOW('H', 202, int)
#define HCIGETDEVINFO	_IOR('H', 211, int)

#define HCI_UP		(1 << 0)

#define HCI_PRIMARY	0x00
#define HCI_AMP		0x01

static struct hci_dev_info hci_info;
static uint8_t hci_type;
static struct bt_hci *hci_dev;

static bool reset_on_init = false;
static bool reset_on_shutdown = false;

static bool shutdown_timeout(void *user_data)
{
	mainloop_quit();

	return false;
}

static void shutdown_complete(const void *data, uint8_t size, void *user_data)
{
	unsigned int id = PTR_TO_UINT(user_data);

	timeout_remove(id);
	mainloop_quit();
}

static void shutdown_device(void)
{
	unsigned int id;

	bt_hci_flush(hci_dev);

	if (reset_on_shutdown) {
		id = timeout_add(5000, shutdown_timeout, NULL, NULL);

		bt_hci_send(hci_dev, BT_HCI_CMD_RESET, NULL, 0,
				shutdown_complete, UINT_TO_PTR(id), NULL);
	} else
		mainloop_quit();
}

static void local_version_callback(const void *data, uint8_t size,
							void *user_data)
{
	const struct bt_hci_rsp_read_local_version *rsp = data;

	printf("HCI version: %u\n", rsp->hci_ver);
	printf("HCI revision: %u\n", le16_to_cpu(rsp->hci_rev));

	switch (hci_type) {
	case HCI_PRIMARY:
		printf("LMP version: %u\n", rsp->lmp_ver);
		printf("LMP subversion: %u\n", le16_to_cpu(rsp->lmp_subver));
		break;
	case HCI_AMP:
		printf("PAL version: %u\n", rsp->lmp_ver);
		printf("PAL subversion: %u\n", le16_to_cpu(rsp->lmp_subver));
		break;
	}

	printf("Manufacturer: %u\n", le16_to_cpu(rsp->manufacturer));
}

static void local_commands_callback(const void *data, uint8_t size,
							void *user_data)
{
	shutdown_device();
}

static void local_features_callback(const void *data, uint8_t size,
							void *user_data)
{
	bt_hci_send(hci_dev, BT_HCI_CMD_READ_LOCAL_COMMANDS, NULL, 0,
					local_commands_callback, NULL, NULL);
}

static bool cmd_local(int argc, char *argv[])
{
	if (reset_on_init)
		bt_hci_send(hci_dev, BT_HCI_CMD_RESET, NULL, 0,
						NULL, NULL, NULL);

	bt_hci_send(hci_dev, BT_HCI_CMD_READ_LOCAL_VERSION, NULL, 0,
					local_version_callback, NULL, NULL);

	bt_hci_send(hci_dev, BT_HCI_CMD_READ_LOCAL_FEATURES, NULL, 0,
					local_features_callback, NULL, NULL);

	return true;
}

typedef bool (*cmd_func_t)(int argc, char *argv[]);

static const struct {
	const char *name;
	cmd_func_t func;
	const char *help;
} cmd_table[] = {
	{ "local", cmd_local, "Print local controller details" },
	{ }
};

static void signal_callback(int signum, void *user_data)
{
	static bool terminated = false;

	switch (signum) {
	case SIGINT:
	case SIGTERM:
		if (!terminated) {
			shutdown_device();
			terminated = true;
		}
		break;
	}
}

static void usage(void)
{
	int i;

	printf("btinfo - Bluetooth device testing tool\n"
		"Usage:\n");
	printf("\tbtinfo [options] <command>\n");
	printf("options:\n"
		"\t-i, --index <num>      Use specified controller\n"
		"\t-h, --help             Show help options\n");
	printf("commands:\n");
	for (i = 0; cmd_table[i].name; i++)
		printf("\t%-25s%s\n", cmd_table[i].name, cmd_table[i].help);
}

static const struct option main_options[] = {
	{ "index",   required_argument, NULL, 'i' },
	{ "reset",   no_argument,       NULL, 'r' },
	{ "raw",     no_argument,       NULL, 'R' },
	{ "version", no_argument,       NULL, 'v' },
	{ "help",    no_argument,       NULL, 'h' },
	{ }
};

int main(int argc, char *argv[])
{
	cmd_func_t func = NULL;
	uint16_t index = 0;
	const char *str;
	bool use_raw = false;
	bool power_down = false;
	sigset_t mask;
	int fd, i, exit_status;

	for (;;) {
		int opt;

		opt = getopt_long(argc, argv, "i:rRvh", main_options, NULL);
		if (opt < 0)
			break;

		switch (opt) {
		case 'i':
			if (strlen(optarg) > 3 && !strncmp(optarg, "hci", 3))
				str = optarg + 3;
			else
				str = optarg;
			if (!isdigit(*str)) {
				usage();
				return EXIT_FAILURE;
			}
			index = atoi(str);
			break;
		case 'r':
			reset_on_init = true;
			break;
		case 'R':
			use_raw = true;
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

	if (argc - optind < 1) {
		fprintf(stderr, "Missing command argument\n");
		return EXIT_FAILURE;
	}

	for (i = 0; cmd_table[i].name; i++) {
		if (!strcmp(cmd_table[i].name, argv[optind])) {
			func = cmd_table[i].func;
			break;
		}
	}

	if (!func) {
		fprintf(stderr, "Unsupported command specified\n");
		return EXIT_FAILURE;
	}

	mainloop_init();

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);

	mainloop_set_signal(&mask, signal_callback, NULL, NULL);

	printf("Bluetooth information utility ver %s\n", VERSION);

	fd = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
	if (fd < 0) {
		perror("Failed to open HCI raw socket");
		return EXIT_FAILURE;
	}

	memset(&hci_info, 0, sizeof(hci_info));
	hci_info.dev_id = index;

	if (ioctl(fd, HCIGETDEVINFO, (void *) &hci_info) < 0) {
		perror("Failed to get HCI device information");
		close(fd);
		return EXIT_FAILURE;
	}

	if (use_raw && !(hci_info.flags & HCI_UP)) {
		printf("Powering on controller\n");

		if (ioctl(fd, HCIDEVUP, hci_info.dev_id) < 0) {
			perror("Failed to power on controller");
			close(fd);
			return EXIT_FAILURE;
		}

		power_down = true;
	}

	close(fd);

	hci_type = (hci_info.type & 0x30) >> 4;

	if (use_raw) {
		hci_dev = bt_hci_new_raw_device(index);
		if (!hci_dev) {
			fprintf(stderr, "Failed to open HCI raw device\n");
			return EXIT_FAILURE;
		}
	} else {
		hci_dev = bt_hci_new_user_channel(index);
		if (!hci_dev) {
			fprintf(stderr, "Failed to open HCI user channel\n");
			return EXIT_FAILURE;
		}

		reset_on_init = true;
		reset_on_shutdown = true;
	}

	if (!func(argc - optind - 1, argv + optind + 1)) {
		bt_hci_unref(hci_dev);
		return EXIT_FAILURE;
	}

	exit_status = mainloop_run();

	bt_hci_unref(hci_dev);

	if (use_raw && power_down) {
		fd = socket(AF_BLUETOOTH, SOCK_RAW | SOCK_CLOEXEC, BTPROTO_HCI);
		if (fd >= 0) {
			printf("Powering down controller\n");

			if (ioctl(fd, HCIDEVDOWN, hci_info.dev_id) < 0)
				perror("Failed to power down controller");

			close(fd);
		}
	}

	return exit_status;
}

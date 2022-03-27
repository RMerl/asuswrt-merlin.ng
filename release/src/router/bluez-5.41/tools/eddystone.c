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
#include <fcntl.h>
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

static int urandom_fd;
static struct bt_hci *hci_dev;

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
	uint8_t enable = 0x00;
	unsigned int id;

	bt_hci_flush(hci_dev);

	id = timeout_add(5000, shutdown_timeout, NULL, NULL);

	bt_hci_send(hci_dev, BT_HCI_CMD_LE_SET_ADV_ENABLE,
					&enable, 1, NULL, NULL, NULL);

	bt_hci_send(hci_dev, BT_HCI_CMD_RESET, NULL, 0,
				shutdown_complete, UINT_TO_PTR(id), NULL);
}

static void set_random_address(void)
{
	struct bt_hci_cmd_le_set_random_address cmd;
	ssize_t len;

	len = read(urandom_fd, cmd.addr, sizeof(cmd.addr));
	if (len < 0 || len != sizeof(cmd.addr)) {
		fprintf(stderr, "Failed to read random data\n");
		return;
	}

	/* Clear top most significant bits */
	cmd.addr[5] &= 0x3f;

	bt_hci_send(hci_dev, BT_HCI_CMD_LE_SET_RANDOM_ADDRESS,
					&cmd, sizeof(cmd), NULL, NULL, NULL);
}

static void set_adv_parameters(void)
{
	struct bt_hci_cmd_le_set_adv_parameters cmd;

	cmd.min_interval = cpu_to_le16(0x0800);
	cmd.max_interval = cpu_to_le16(0x0800);
	cmd.type = 0x03;		/* Non-connectable advertising */
	cmd.own_addr_type = 0x01;	/* Use random address */
	cmd.direct_addr_type = 0x00;
	memset(cmd.direct_addr, 0, 6);
	cmd.channel_map = 0x07;
	cmd.filter_policy = 0x00;

	bt_hci_send(hci_dev, BT_HCI_CMD_LE_SET_ADV_PARAMETERS,
					&cmd, sizeof(cmd), NULL, NULL, NULL);
}

static void set_adv_enable(void)
{
	uint8_t enable = 0x01;

	bt_hci_send(hci_dev, BT_HCI_CMD_LE_SET_ADV_ENABLE,
					&enable, 1, NULL, NULL, NULL);
}

static void adv_data_callback(const void *data, uint8_t size,
							void *user_data)
{
	uint8_t status = *((uint8_t *) data);

	if (status) {
		fprintf(stderr, "Failed to set advertising data\n");
		shutdown_device();
		return;
	}

	set_random_address();
	set_adv_parameters();
	set_adv_enable();
}

static void adv_tx_power_callback(const void *data, uint8_t size,
							void *user_data)
{
	const struct bt_hci_rsp_le_read_adv_tx_power *rsp = data;
	struct bt_hci_cmd_le_set_adv_data cmd;

	if (rsp->status) {
		fprintf(stderr, "Failed to read advertising TX power\n");
		shutdown_device();
		return;
	}

	cmd.data[0] = 0x02;		/* Field length */
	cmd.data[1] = 0x01;		/* Flags */
	cmd.data[2] |= 0x04;		/* BR/EDR Not Supported */

	cmd.data[3] = 0x03;		/* Field length */
	cmd.data[4] = 0x03;		/* 16-bit Service UUID list */
	cmd.data[5] = 0xaa;		/* Eddystone UUID */
	cmd.data[6] = 0xfe;

	cmd.data[7] = 0x0c;		/* Field length */
	cmd.data[8] = 0x16;		/* 16-bit Service UUID data */
	cmd.data[9] = 0xaa;		/* Eddystone UUID */
	cmd.data[10] = 0xfe;
	cmd.data[11] = 0x10;		/* Eddystone-URL frame type */
	cmd.data[12] = 0x00;		/* Calibrated Tx power at 0m */
	cmd.data[13] = 0x00;		/* URL Scheme Prefix http://www. */
	cmd.data[14] = 'b';
	cmd.data[15] = 'l';
	cmd.data[16] = 'u';
	cmd.data[17] = 'e';
	cmd.data[18] = 'z';
	cmd.data[19] = 0x01;		/* .org/ */

	cmd.data[20] = 0x00;		/* Field terminator */
	memset(cmd.data + 21, 0, 9);

	cmd.len = 1 + cmd.data[0] + 1 + cmd.data[3] + 1 + cmd.data[7];

	bt_hci_send(hci_dev, BT_HCI_CMD_LE_SET_ADV_DATA, &cmd, sizeof(cmd),
					adv_data_callback, NULL, NULL);
}

static void local_features_callback(const void *data, uint8_t size,
							void *user_data)
{
	const struct bt_hci_rsp_read_local_features *rsp = data;

	if (rsp->status) {
		fprintf(stderr, "Failed to read local features\n");
		shutdown_device();
		return;
	}

	if (!(rsp->features[4] & 0x40)) {
		fprintf(stderr, "Controller without Low Energy support\n");
		shutdown_device();
		return;
	}

	bt_hci_send(hci_dev, BT_HCI_CMD_LE_READ_ADV_TX_POWER, NULL, 0,
					adv_tx_power_callback, NULL, NULL);
}

static void start_eddystone(void)
{
	bt_hci_send(hci_dev, BT_HCI_CMD_RESET, NULL, 0, NULL, NULL, NULL);

	bt_hci_send(hci_dev, BT_HCI_CMD_READ_LOCAL_FEATURES, NULL, 0,
					local_features_callback, NULL, NULL);
}

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
	printf("eddystone - Low Energy Eddystone testing tool\n"
		"Usage:\n");
	printf("\teddystone [options]\n");
	printf("Options:\n"
		"\t-i, --index <num>      Use specified controller\n"
		"\t-h, --help             Show help options\n");
}

static const struct option main_options[] = {
	{ "index",   required_argument, NULL, 'i' },
	{ "version", no_argument,       NULL, 'v' },
	{ "help",    no_argument,       NULL, 'h' },
	{ }
};

int main(int argc, char *argv[])
{
	uint16_t index = 0;
	const char *str;
	sigset_t mask;
	int exit_status;

	for (;;) {
		int opt;

		opt = getopt_long(argc, argv, "i:vh", main_options, NULL);
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

	urandom_fd = open("/dev/urandom", O_RDONLY);
	if (urandom_fd < 0) {
		fprintf(stderr, "Failed to open /dev/urandom device\n");
		return EXIT_FAILURE;
	}

	mainloop_init();

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);

	mainloop_set_signal(&mask, signal_callback, NULL, NULL);

	printf("Low Energy Eddystone utility ver %s\n", VERSION);

	hci_dev = bt_hci_new_user_channel(index);
	if (!hci_dev) {
		fprintf(stderr, "Failed to open HCI user channel\n");
		exit_status = EXIT_FAILURE;
		goto done;
	}

	start_eddystone();

	exit_status = mainloop_run();

	bt_hci_unref(hci_dev);

done:
	close(urandom_fd);

	return exit_status;
}

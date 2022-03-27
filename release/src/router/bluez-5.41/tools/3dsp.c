/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Intel Corporation. All rights reserved.
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
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "monitor/bt.h"
#include "src/shared/mainloop.h"
#include "src/shared/timeout.h"
#include "src/shared/util.h"
#include "src/shared/hci.h"

#define LT_ADDR 0x01
#define PKT_TYPE 0x0008		/* 0x0008 = EDR + DM1, 0xff1e = BR only */
#define SERVICE_DATA 0x00

struct broadcast_message {
	uint32_t frame_sync_instant;
	uint16_t bluetooth_clock_phase;
	uint16_t left_open_offset;
	uint16_t left_close_offset;
	uint16_t right_open_offset;
	uint16_t right_close_offset;
	uint16_t frame_sync_period;
	uint8_t  frame_sync_period_fraction;
} __attribute__ ((packed));

struct brcm_evt_sync_train_received {
	uint8_t  status;
	uint8_t  bdaddr[6];
	uint32_t offset;
	uint8_t  map[10];
	uint8_t  service_data;
	uint8_t  lt_addr;
	uint32_t instant;
	uint16_t interval;
} __attribute__ ((packed));

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

static void inquiry_started(const void *data, uint8_t size, void *user_data)
{
	uint8_t status = *((uint8_t *) data);

	if (status) {
		printf("Failed to search for 3D display\n");
		shutdown_device();
		return;
	}

	printf("Searching for 3D display\n");
}

static void start_inquiry(void)
{
	struct bt_hci_cmd_inquiry cmd;

	cmd.lap[0] = 0x33;
	cmd.lap[1] = 0x8b;
	cmd.lap[2] = 0x9e;
	cmd.length = 0x08;
	cmd.num_resp = 0x00;

	bt_hci_send(hci_dev, BT_HCI_CMD_INQUIRY, &cmd, sizeof(cmd),
						inquiry_started, NULL, NULL);
}

static void set_slave_broadcast_receive(const void *data, uint8_t size,
							void *user_data)
{
	printf("Slave broadcast receiption enabled\n");
}

static void sync_train_received(const void *data, uint8_t size,
							void *user_data)
{
	const struct bt_hci_evt_sync_train_received *evt = data;
	struct bt_hci_cmd_set_slave_broadcast_receive cmd;

	if (evt->status) {
		printf("Failed to synchronize with 3D display\n");
		start_inquiry();
		return;
	}

	if (evt->lt_addr != LT_ADDR) {
		printf("Ignoring synchronization for non 3D display\n");
		return;
	}

	cmd.enable = 0x01;
	memcpy(cmd.bdaddr, evt->bdaddr, 6);
	cmd.lt_addr = evt->lt_addr;
	cmd.interval = evt->interval;
	cmd.offset = evt->offset;
	cmd.instant = evt->instant;
	cmd.timeout = cpu_to_le16(0xfffe);
	cmd.accuracy = 250;
	cmd.skip = 20;
	cmd.pkt_type = cpu_to_le16(PKT_TYPE);
	memcpy(cmd.map, evt->map, 10);

	bt_hci_send(hci_dev, BT_HCI_CMD_SET_SLAVE_BROADCAST_RECEIVE,
				&cmd, sizeof(cmd),
				set_slave_broadcast_receive, NULL, NULL);
}

static void brcm_sync_train_received(const void *data, uint8_t size,
							void *user_data)
{
	const struct brcm_evt_sync_train_received *evt = data;
	struct bt_hci_cmd_set_slave_broadcast_receive cmd;

	if (evt->status) {
		printf("Failed to synchronize with 3D display\n");
		start_inquiry();
		return;
	}

	if (evt->lt_addr != LT_ADDR) {
		printf("Ignoring synchronization for non 3D display\n");
		return;
	}

	cmd.enable = 0x01;
	memcpy(cmd.bdaddr, evt->bdaddr, 6);
	cmd.lt_addr = evt->lt_addr;
	cmd.interval = evt->interval;
	cmd.offset = evt->offset;
	cmd.instant = evt->instant;
	cmd.timeout = cpu_to_le16(0xfffe);
	cmd.accuracy = 250;
	cmd.skip = 20;
	cmd.pkt_type = cpu_to_le16(PKT_TYPE);
	memcpy(cmd.map, evt->map, 10);

	bt_hci_send(hci_dev, BT_HCI_CMD_SET_SLAVE_BROADCAST_RECEIVE,
				&cmd, sizeof(cmd),
				set_slave_broadcast_receive, NULL, NULL);
}

static void truncated_page_complete(const void *data, uint8_t size,
							void *user_data)
{
	const struct bt_hci_evt_truncated_page_complete *evt = data;
	struct bt_hci_cmd_receive_sync_train cmd;

	if (evt->status) {
		printf("Failed to contact 3D display\n");
		shutdown_device();
		return;
	}

	printf("Attempt to synchronize with 3D display\n");

	memcpy(cmd.bdaddr, evt->bdaddr, 6);
	cmd.timeout = cpu_to_le16(0x4000);
	cmd.window = cpu_to_le16(0x0100);
	cmd.interval = cpu_to_le16(0x0080);

	bt_hci_send(hci_dev, BT_HCI_CMD_RECEIVE_SYNC_TRAIN, &cmd, sizeof(cmd),
							NULL, NULL, NULL);
}

static void slave_broadcast_timeout(const void *data, uint8_t size,
							void *user_data)
{
	const struct bt_hci_evt_slave_broadcast_timeout *evt = data;
	struct bt_hci_cmd_receive_sync_train cmd;

	printf("Re-synchronizing with 3D display\n");

	memcpy(cmd.bdaddr, evt->bdaddr, 6);
	cmd.timeout = cpu_to_le16(0x4000);
	cmd.window = cpu_to_le16(0x0100);
	cmd.interval = cpu_to_le16(0x0080);

	bt_hci_send(hci_dev, BT_HCI_CMD_RECEIVE_SYNC_TRAIN, &cmd, sizeof(cmd),
							NULL, NULL, NULL);
}

static void slave_broadcast_receive(const void *data, uint8_t size,
							void *user_data)
{
	const struct bt_hci_evt_slave_broadcast_receive *evt = data;
	struct bt_hci_cmd_read_clock cmd;

	if (evt->status != 0x00)
		return;

	if (le32_to_cpu(evt->clock) != 0x00000000)
		return;

	cmd.handle = cpu_to_le16(0x0000);
	cmd.type = 0x00;

	bt_hci_send(hci_dev, BT_HCI_CMD_READ_CLOCK, &cmd, sizeof(cmd),
							NULL, NULL, NULL);
}

static void ext_inquiry_result(const void *data, uint8_t size, void *user_data)
{
	const struct bt_hci_evt_ext_inquiry_result *evt = data;

	if (evt->dev_class[0] != 0x3c || evt->dev_class[1] != 0x04
					|| evt->dev_class[2] != 0x08)
		return;

	if (evt->data[0]) {
		struct bt_hci_cmd_truncated_page cmd;

		printf("Found 3D display\n");

		bt_hci_send(hci_dev, BT_HCI_CMD_INQUIRY_CANCEL, NULL, 0,
							NULL, NULL, NULL);

		memcpy(cmd.bdaddr, evt->bdaddr, 6);
		cmd.pscan_rep_mode = evt->pscan_rep_mode;
		cmd.clock_offset = evt->clock_offset;

		bt_hci_send(hci_dev, BT_HCI_CMD_TRUNCATED_PAGE,
					&cmd, sizeof(cmd), NULL, NULL, NULL);
	}
}

static void inquiry_complete(const void *data, uint8_t size, void *user_data)
{
	printf("No 3D display found\n");

	start_inquiry();
}

static void read_local_version(const void *data, uint8_t size, void *user_data)
{
	const struct bt_hci_rsp_read_local_version *rsp = data;

	if (rsp->status) {
		printf("Failed to read local version information\n");
		shutdown_device();
		return;
	}

	if (rsp->manufacturer == 15) {
		printf("Enabling receiver workaround for Broadcom\n");

		bt_hci_register(hci_dev, BT_HCI_EVT_SYNC_TRAIN_RECEIVED,
					brcm_sync_train_received, NULL, NULL);
	} else {
		bt_hci_register(hci_dev, BT_HCI_EVT_SYNC_TRAIN_RECEIVED,
					sync_train_received, NULL, NULL);
	}
}

static void start_glasses(void)
{
	uint8_t evtmask1[] = { 0x03, 0xe0, 0x00, 0x00, 0x02, 0x40, 0x00, 0x00 };
	uint8_t evtmask2[] = { 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00 };
	uint8_t inqmode = 0x02;

	if (reset_on_init) {
		bt_hci_send(hci_dev, BT_HCI_CMD_RESET, NULL, 0,
							NULL, NULL, NULL);
		bt_hci_send(hci_dev, BT_HCI_CMD_SET_EVENT_MASK, evtmask1, 8,
							NULL, NULL, NULL);
	}

	bt_hci_send(hci_dev, BT_HCI_CMD_READ_LOCAL_VERSION, NULL, 0,
					read_local_version, NULL, NULL);

	bt_hci_send(hci_dev, BT_HCI_CMD_SET_EVENT_MASK_PAGE2, evtmask2, 8,
							NULL, NULL, NULL);
	bt_hci_send(hci_dev, BT_HCI_CMD_WRITE_INQUIRY_MODE, &inqmode, 1,
							NULL, NULL, NULL);

	bt_hci_register(hci_dev, BT_HCI_EVT_INQUIRY_COMPLETE,
						inquiry_complete, NULL, NULL);
	bt_hci_register(hci_dev, BT_HCI_EVT_EXT_INQUIRY_RESULT,
						ext_inquiry_result, NULL, NULL);

	bt_hci_register(hci_dev, BT_HCI_EVT_TRUNCATED_PAGE_COMPLETE,
					truncated_page_complete, NULL, NULL);
	bt_hci_register(hci_dev, BT_HCI_EVT_SLAVE_BROADCAST_TIMEOUT,
					slave_broadcast_timeout, NULL, NULL);
	bt_hci_register(hci_dev, BT_HCI_EVT_SLAVE_BROADCAST_RECEIVE,
					slave_broadcast_receive, NULL, NULL);

	start_inquiry();
}

static bool sync_train_active = false;

static void sync_train_complete(const void *data, uint8_t size,
							void *user_data)
{
	sync_train_active = false;
}

static void start_sync_train(void)
{
	struct bt_hci_cmd_write_sync_train_params cmd;

	if (sync_train_active)
		return;

	printf("Starting new synchronization train\n");

	cmd.min_interval = cpu_to_le16(0x0050);
	cmd.max_interval = cpu_to_le16(0x00a0);
	cmd.timeout = cpu_to_le32(0x0002ee00);		/* 120 sec */
	cmd.service_data = SERVICE_DATA;

	bt_hci_send(hci_dev, BT_HCI_CMD_WRITE_SYNC_TRAIN_PARAMS,
					&cmd, sizeof(cmd), NULL, NULL, NULL);

	bt_hci_send(hci_dev, BT_HCI_CMD_START_SYNC_TRAIN, NULL, 0,
							NULL, NULL, NULL);

	sync_train_active = true;
}

static void conn_request(const void *data, uint8_t size, void *user_data)
{
	const struct bt_hci_evt_conn_request *evt = data;
	struct bt_hci_cmd_accept_conn_request cmd;

	printf("Incoming connection from 3D glasses\n");

	memcpy(cmd.bdaddr, evt->bdaddr, 6);
	cmd.role = 0x00;

	bt_hci_send(hci_dev, BT_HCI_CMD_ACCEPT_CONN_REQUEST, &cmd, sizeof(cmd),
							NULL, NULL, NULL);

	start_sync_train();
}

static void slave_page_response_timeout(const void *data, uint8_t size,
							void *user_data)
{
	printf("Incoming truncated page received\n");

	start_sync_train();
}

static void slave_broadcast_channel_map_change(const void *data, uint8_t size,
								void *user_data)
{
	printf("Broadcast channel map changed\n");

	start_sync_train();
}

static void inquiry_resp_tx_power(const void *data, uint8_t size,
							void *user_data)
{
	const struct bt_hci_rsp_read_inquiry_resp_tx_power *rsp = data;
	struct bt_hci_cmd_write_ext_inquiry_response cmd;
	uint8_t inqdata[] = { 0x03, 0x3d, 0x03, 0x43, 0x02, 0x0a, 0x00, 0x00 };
	uint8_t devclass[] = { 0x3c, 0x04, 0x08 };
	uint8_t scanmode = 0x03;

	inqdata[6] = (uint8_t) rsp->level;

	cmd.fec = 0x00;
	memset(cmd.data, 0, sizeof(cmd.data));
	memcpy(cmd.data, inqdata, sizeof(inqdata));

	bt_hci_send(hci_dev, BT_HCI_CMD_WRITE_EXT_INQUIRY_RESPONSE,
					&cmd, sizeof(cmd), NULL, NULL, NULL);

	bt_hci_send(hci_dev, BT_HCI_CMD_WRITE_CLASS_OF_DEV, devclass, 3,
							NULL, NULL, NULL);
	bt_hci_send(hci_dev, BT_HCI_CMD_WRITE_SCAN_ENABLE, &scanmode, 1,
							NULL, NULL, NULL);
}

static void read_clock(const void *data, uint8_t size, void *user_data)
{
	const struct bt_hci_rsp_read_clock *rsp = data;
	struct broadcast_message msg;
	uint8_t bcastdata[sizeof(msg) + 3] = { LT_ADDR, 0x03, 0x11, };

	if (rsp->status) {
		printf("Failed to read local clock information\n");
		shutdown_device();
		return;
	}

	msg.frame_sync_instant = rsp->clock;
	msg.bluetooth_clock_phase = rsp->accuracy;
	msg.left_open_offset = cpu_to_le16(50);
	msg.left_close_offset = cpu_to_le16(300);
	msg.right_open_offset = cpu_to_le16(350);
	msg.right_close_offset = cpu_to_le16(600);
	msg.frame_sync_period = cpu_to_le16(650);
	msg.frame_sync_period_fraction = 0;
	memcpy(bcastdata + 3, &msg, sizeof(msg));

	bt_hci_send(hci_dev, BT_HCI_CMD_SET_SLAVE_BROADCAST_DATA,
			bcastdata, sizeof(bcastdata), NULL, NULL, NULL);
}

static void set_slave_broadcast(const void *data, uint8_t size, void *user_data)
{
	const struct bt_hci_rsp_set_slave_broadcast *rsp = data;
	struct bt_hci_cmd_read_clock cmd;

	if (rsp->status) {
		printf("Failed to set slave broadcast transmission\n");
		shutdown_device();
		return;
	}

	cmd.handle = cpu_to_le16(0x0000);
	cmd.type = 0x00;

	bt_hci_send(hci_dev, BT_HCI_CMD_READ_CLOCK, &cmd, sizeof(cmd),
						read_clock, NULL, NULL);
}

static void start_display(void)
{
	struct bt_hci_cmd_set_slave_broadcast cmd;
	uint8_t evtmask1[] = { 0x1c, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	uint8_t evtmask2[] = { 0x00, 0xc0, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00 };
	uint8_t sspmode = 0x01;
	uint8_t ltaddr = LT_ADDR;

	if (reset_on_init) {
		bt_hci_send(hci_dev, BT_HCI_CMD_RESET, NULL, 0,
							NULL, NULL, NULL);
		bt_hci_send(hci_dev, BT_HCI_CMD_SET_EVENT_MASK, evtmask1, 8,
							NULL, NULL, NULL);
	}

	bt_hci_send(hci_dev, BT_HCI_CMD_SET_EVENT_MASK_PAGE2, evtmask2, 8,
							NULL, NULL, NULL);
	bt_hci_send(hci_dev, BT_HCI_CMD_WRITE_SIMPLE_PAIRING_MODE, &sspmode, 1,
							NULL, NULL, NULL);
	bt_hci_send(hci_dev, BT_HCI_CMD_SET_RESERVED_LT_ADDR, &ltaddr, 1,
							NULL, NULL, NULL);
	bt_hci_send(hci_dev, BT_HCI_CMD_READ_SYNC_TRAIN_PARAMS, NULL, 0,
							NULL, NULL, NULL);

	bt_hci_register(hci_dev, BT_HCI_EVT_CONN_REQUEST,
						conn_request, NULL, NULL);

	bt_hci_register(hci_dev, BT_HCI_EVT_SLAVE_PAGE_RESPONSE_TIMEOUT,
				slave_page_response_timeout, NULL, NULL);
	bt_hci_register(hci_dev, BT_HCI_EVT_SLAVE_BROADCAST_CHANNEL_MAP_CHANGE,
				slave_broadcast_channel_map_change, NULL, NULL);
	bt_hci_register(hci_dev, BT_HCI_EVT_SYNC_TRAIN_COMPLETE,
					sync_train_complete, NULL, NULL);

	bt_hci_send(hci_dev, BT_HCI_CMD_READ_INQUIRY_RESP_TX_POWER, NULL, 0,
					inquiry_resp_tx_power, NULL, NULL);

	cmd.enable = 0x01;
	cmd.lt_addr = LT_ADDR;
	cmd.lpo_allowed = 0x01;
	cmd.pkt_type = cpu_to_le16(PKT_TYPE);
	cmd.min_interval = cpu_to_le16(0x0050);		/* 50 ms */
	cmd.max_interval = cpu_to_le16(0x00a0);		/* 100 ms */
	cmd.timeout = cpu_to_le16(0xfffe);

	bt_hci_send(hci_dev, BT_HCI_CMD_SET_SLAVE_BROADCAST, &cmd, sizeof(cmd),
					set_slave_broadcast, NULL, NULL);
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
	printf("3dsp - 3D Synchronization Profile testing\n"
		"Usage:\n");
	printf("\t3dsp [options]\n");
	printf("options:\n"
		"\t-D, --display          Use display role\n"
		"\t-G, --glasses          Use glasses role\n"
		"\t-i, --index <num>      Use specified controller\n"
		"\t-h, --help             Show help options\n");
}

static const struct option main_options[] = {
	{ "display", no_argument,       NULL, 'D' },
	{ "glasses", no_argument,       NULL, 'G' },
	{ "index",   required_argument, NULL, 'i' },
	{ "raw",     no_argument,       NULL, 'r' },
	{ "version", no_argument,       NULL, 'v' },
	{ "help",    no_argument,       NULL, 'h' },
	{ }
};

int main(int argc, char *argv[])
{
	bool display_role = false, glasses_role = false;
	uint16_t index = 0;
	const char *str;
	bool use_raw = false;
	sigset_t mask;
	int exit_status;

	for (;;) {
		int opt;

		opt = getopt_long(argc, argv, "DGi:rvh", main_options, NULL);
		if (opt < 0)
			break;

		switch (opt) {
		case 'D':
			display_role = true;
			break;
		case 'G':
			glasses_role = true;
			break;
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

	if (argc - optind > 0) {
		fprintf(stderr, "Invalid command line parameters\n");
		return EXIT_FAILURE;
	}

	if (display_role == glasses_role) {
		fprintf(stderr, "Specify either display or glasses role\n");
		return EXIT_FAILURE;
	}

	mainloop_init();

	sigemptyset(&mask);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);

	mainloop_set_signal(&mask, signal_callback, NULL, NULL);

	printf("3D Synchronization Profile testing ver %s\n", VERSION);

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

	if (display_role)
		start_display();
	else if (glasses_role)
		start_glasses();

	exit_status = mainloop_run();

	bt_hci_unref(hci_dev);

	return exit_status;
}

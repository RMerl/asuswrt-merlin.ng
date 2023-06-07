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

#include <getopt.h>

#include "lib/bluetooth.h"
#include "lib/mgmt.h"

#include "monitor/bt.h"
#include "src/shared/mainloop.h"
#include "src/shared/util.h"
#include "src/shared/mgmt.h"
#include "src/shared/hci.h"
#include "src/shared/crypto.h"

#define PEER_ADDR_TYPE	0x00
#define PEER_ADDR	"\x00\x00\x00\x00\x00\x00"

#define ADV_IRK		"\x69\x30\xde\xc3\x8f\x84\x74\x14" \
			"\xe1\x23\x99\xc1\xca\x9a\xc3\x31"
#define SCAN_IRK	"\xfa\x73\x09\x11\x3f\x03\x37\x0f" \
			"\xf4\xf9\x93\x1e\xf9\xa3\x63\xa6"

static struct mgmt *mgmt;
static uint16_t index1 = MGMT_INDEX_NONE;
static uint16_t index2 = MGMT_INDEX_NONE;

static struct bt_crypto *crypto;
static struct bt_hci *adv_dev;
static struct bt_hci *scan_dev;

static void print_rpa(const uint8_t addr[6])
{
	printf("  Address:  %02x:%02x:%02x:%02x:%02x:%02x\n",
					addr[5], addr[4], addr[3],
					addr[2], addr[1], addr[0]);
	printf("    Random: %02x%02x%02x\n", addr[3], addr[4], addr[5]);
	printf("    Hash:   %02x%02x%02x\n", addr[0], addr[1], addr[2]);
}

static void scan_le_adv_report(const void *data, uint8_t size,
							void *user_data)
{
	const struct bt_hci_evt_le_adv_report *evt = data;

	if (evt->addr_type == 0x01 && (evt->addr[5] & 0xc0) == 0x40) {
		uint8_t hash[3], irk[16];

		memcpy(irk, ADV_IRK, 16);
		bt_crypto_ah(crypto, irk, evt->addr + 3, hash);

		if (!memcmp(evt->addr, hash, 3)) {
			printf("Received advertising report\n");
			print_rpa(evt->addr);

			memcpy(irk, ADV_IRK, 16);
			bt_crypto_ah(crypto, irk, evt->addr + 3, hash);

			printf("  -> Computed hash: %02x%02x%02x\n",
						hash[0], hash[1], hash[2]);

			mainloop_quit();
		}
	}
}

static void scan_le_meta_event(const void *data, uint8_t size,
							void *user_data)
{
	uint8_t evt_code = ((const uint8_t *) data)[0];

	switch (evt_code) {
	case BT_HCI_EVT_LE_ADV_REPORT:
		scan_le_adv_report(data + 1, size - 1, user_data);
		break;
	}
}

static void scan_enable_callback(const void *data, uint8_t size,
							void *user_data)
{
}

static void adv_enable_callback(const void *data, uint8_t size,
							void *user_data)
{
	struct bt_hci_cmd_le_set_scan_parameters cmd4;
	struct bt_hci_cmd_le_set_scan_enable cmd5;

	cmd4.type = 0x00;		/* Passive scanning */
	cmd4.interval = cpu_to_le16(0x0010);
	cmd4.window = cpu_to_le16(0x0010);
	cmd4.own_addr_type = 0x00;	/* Use public address */
	cmd4.filter_policy = 0x00;

	bt_hci_send(scan_dev, BT_HCI_CMD_LE_SET_SCAN_PARAMETERS,
					&cmd4, sizeof(cmd4), NULL, NULL, NULL);

	cmd5.enable = 0x01;
	cmd5.filter_dup = 0x01;

	bt_hci_send(scan_dev, BT_HCI_CMD_LE_SET_SCAN_ENABLE,
					&cmd5, sizeof(cmd5),
					scan_enable_callback, NULL, NULL);
}

static void adv_le_evtmask_callback(const void *data, uint8_t size,
							void *user_data)
{
	struct bt_hci_cmd_le_set_resolv_timeout cmd0;
	struct bt_hci_cmd_le_add_to_resolv_list cmd1;
	struct bt_hci_cmd_le_set_resolv_enable cmd2;
	struct bt_hci_cmd_le_set_random_address cmd3;
	struct bt_hci_cmd_le_set_adv_parameters cmd4;
	struct bt_hci_cmd_le_set_adv_enable cmd5;

	cmd0.timeout = cpu_to_le16(0x0384);

	bt_hci_send(adv_dev, BT_HCI_CMD_LE_SET_RESOLV_TIMEOUT,
					&cmd0, sizeof(cmd0), NULL, NULL, NULL);

	cmd1.addr_type = PEER_ADDR_TYPE;
	memcpy(cmd1.addr, PEER_ADDR, 6);
	memset(cmd1.peer_irk, 0, 16);
	memcpy(cmd1.local_irk, ADV_IRK, 16);

	bt_hci_send(adv_dev, BT_HCI_CMD_LE_ADD_TO_RESOLV_LIST,
					&cmd1, sizeof(cmd1), NULL, NULL, NULL);

	cmd2.enable = 0x01;

	bt_hci_send(adv_dev, BT_HCI_CMD_LE_SET_RESOLV_ENABLE,
					&cmd2, sizeof(cmd2), NULL, NULL, NULL);

	bt_crypto_random_bytes(crypto, cmd3.addr + 3, 3);
	cmd3.addr[5] &= 0x3f;	/* Clear two most significant bits */
	cmd3.addr[5] |= 0x40;	/* Set second most significant bit */
	bt_crypto_ah(crypto, cmd1.local_irk, cmd3.addr + 3, cmd3.addr);

	bt_hci_send(adv_dev, BT_HCI_CMD_LE_SET_RANDOM_ADDRESS,
					&cmd3, sizeof(cmd3), NULL, NULL, NULL);

	printf("Setting advertising address\n");
	print_rpa(cmd3.addr);

	cmd4.min_interval = cpu_to_le16(0x0800);
	cmd4.max_interval = cpu_to_le16(0x0800);
	cmd4.type = 0x03;		/* Non-connectable advertising */
	cmd4.own_addr_type = 0x03;	/* Local IRK, random address fallback */
	cmd4.direct_addr_type = PEER_ADDR_TYPE;
	memcpy(cmd4.direct_addr, PEER_ADDR, 6);
	cmd4.channel_map = 0x07;
	cmd4.filter_policy = 0x00;

	bt_hci_send(adv_dev, BT_HCI_CMD_LE_SET_ADV_PARAMETERS,
					&cmd4, sizeof(cmd4), NULL, NULL, NULL);

	cmd5.enable = 0x01;

	bt_hci_send(adv_dev, BT_HCI_CMD_LE_SET_ADV_ENABLE,
					&cmd5, sizeof(cmd5),
					adv_enable_callback, NULL, NULL);
}

static void adv_le_features_callback(const void *data, uint8_t size,
							void *user_data)
{
	const struct bt_hci_rsp_le_read_local_features *rsp = data;
	uint8_t evtmask[] = { 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00 };

	if (rsp->status) {
		fprintf(stderr, "Failed to read local LE features\n");
		mainloop_exit_failure();
		return;
	}

	bt_hci_send(adv_dev, BT_HCI_CMD_LE_SET_EVENT_MASK, evtmask, 8,
					adv_le_evtmask_callback, NULL, NULL);
}

static void adv_features_callback(const void *data, uint8_t size,
							void *user_data)
{
	const struct bt_hci_rsp_read_local_features *rsp = data;
	uint8_t evtmask[] = { 0x90, 0xe8, 0x04, 0x02, 0x00, 0x80, 0x00, 0x20 };

	if (rsp->status) {
		fprintf(stderr, "Failed to read local features\n");
		mainloop_exit_failure();
		return;
	}

	if (!(rsp->features[4] & 0x40)) {
		fprintf(stderr, "Controller without Low Energy support\n");
		mainloop_exit_failure();
		return;
	}

	bt_hci_send(adv_dev, BT_HCI_CMD_SET_EVENT_MASK, evtmask, 8,
							NULL, NULL, NULL);

	bt_hci_send(adv_dev, BT_HCI_CMD_LE_READ_LOCAL_FEATURES, NULL, 0,
					adv_le_features_callback, NULL, NULL);
}

static void scan_le_evtmask_callback(const void *data, uint8_t size,
							void *user_data)
{
	bt_hci_send(adv_dev, BT_HCI_CMD_RESET, NULL, 0, NULL, NULL, NULL);

	bt_hci_send(adv_dev, BT_HCI_CMD_READ_LOCAL_FEATURES, NULL, 0,
					adv_features_callback, NULL, NULL);
}

static void scan_le_features_callback(const void *data, uint8_t size,
							void *user_data)
{
	const struct bt_hci_rsp_le_read_local_features *rsp = data;
	uint8_t evtmask[] = { 0xff, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00 };

	if (rsp->status) {
		fprintf(stderr, "Failed to read local LE features\n");
		mainloop_exit_failure();
		return;
	}

	bt_hci_send(adv_dev, BT_HCI_CMD_LE_SET_EVENT_MASK, evtmask, 8,
					scan_le_evtmask_callback, NULL, NULL);
}

static void scan_features_callback(const void *data, uint8_t size,
							void *user_data)
{
	const struct bt_hci_rsp_read_local_features *rsp = data;
	uint8_t evtmask[] = { 0x90, 0xe8, 0x04, 0x02, 0x00, 0x80, 0x00, 0x20 };

	if (rsp->status) {
		fprintf(stderr, "Failed to read local features\n");
		mainloop_exit_failure();
		return;
	}

	if (!(rsp->features[4] & 0x40)) {
		fprintf(stderr, "Controller without Low Energy support\n");
		mainloop_exit_failure();
		return;
	}

	bt_hci_send(scan_dev, BT_HCI_CMD_SET_EVENT_MASK, evtmask, 8,
							NULL, NULL, NULL);

	bt_hci_send(scan_dev, BT_HCI_CMD_LE_READ_LOCAL_FEATURES, NULL, 0,
					scan_le_features_callback, NULL, NULL);
}

static void read_index_list(uint8_t status, uint16_t len, const void *param,
							void *user_data)
{
	const struct mgmt_rp_read_index_list *rp = param;
	uint16_t count;
	int i;

	if (status) {
		fprintf(stderr, "Reading index list failed: %s\n",
						mgmt_errstr(status));
		mainloop_exit_failure();
		return;
	}

	count = le16_to_cpu(rp->num_controllers);

	if (count < 2) {
		fprintf(stderr, "At least 2 controllers are required\n");
		mainloop_exit_failure();
		return;
	}

	for (i = 0; i < count; i++) {
		uint16_t index = cpu_to_le16(rp->index[i]);

		if (index < index1)
			index1 = index;
	}

	for (i = 0; i < count; i++) {
		uint16_t index = cpu_to_le16(rp->index[i]);

		if (index < index2 && index > index1)
			index2 = index;
	}

	printf("Selecting index %u for advertiser\n", index1);
	printf("Selecting index %u for scanner\n", index2);

	crypto = bt_crypto_new();
	if (!crypto) {
		fprintf(stderr, "Failed to open crypto interface\n");
		mainloop_exit_failure();
		return;
	}

	adv_dev = bt_hci_new_user_channel(index1);
	if (!adv_dev) {
		fprintf(stderr, "Failed to open HCI for advertiser\n");
		mainloop_exit_failure();
		return;
	}

	scan_dev = bt_hci_new_user_channel(index2);
	if (!scan_dev) {
		fprintf(stderr, "Failed to open HCI for scanner\n");
		mainloop_exit_failure();
		return;
	}

	bt_hci_register(scan_dev, BT_HCI_EVT_LE_META_EVENT,
					scan_le_meta_event, NULL, NULL);

	bt_hci_send(scan_dev, BT_HCI_CMD_RESET, NULL, 0, NULL, NULL, NULL);

	bt_hci_send(scan_dev, BT_HCI_CMD_READ_LOCAL_FEATURES, NULL, 0,
					scan_features_callback, NULL, NULL);
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

static void usage(void)
{
	printf("advtest - Advertising testing\n"
		"Usage:\n");
	printf("\tadvtest [options]\n");
	printf("options:\n"
		"\t-h, --help             Show help options\n");
}

static const struct option main_options[] = {
	{ "version",   no_argument,       NULL, 'v' },
	{ "help",      no_argument,       NULL, 'h' },
	{ }
};

int main(int argc ,char *argv[])
{
	int exit_status;

	for (;;) {
		int opt;

		opt = getopt_long(argc, argv, "vh", main_options, NULL);
		if (opt < 0)
			break;

		switch (opt) {
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

	mgmt = mgmt_new_default();
	if (!mgmt) {
		fprintf(stderr, "Failed to open management socket\n");
		return EXIT_FAILURE;
	}

	if (!mgmt_send(mgmt, MGMT_OP_READ_INDEX_LIST,
					MGMT_INDEX_NONE, 0, NULL,
					read_index_list, NULL, NULL)) {
		fprintf(stderr, "Failed to read index list\n");
		exit_status = EXIT_FAILURE;
		goto done;
	}

	exit_status = mainloop_run_with_signal(signal_callback, NULL);

	bt_hci_unref(adv_dev);
	bt_hci_unref(scan_dev);

	bt_crypto_unref(crypto);

done:
	mgmt_unref(mgmt);

	return exit_status;
}

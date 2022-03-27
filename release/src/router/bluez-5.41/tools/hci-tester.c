/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013  Intel Corporation. All rights reserved.
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

#include <stdlib.h>
#include <string.h>

#include "monitor/bt.h"
#include "src/shared/hci.h"
#include "src/shared/util.h"
#include "src/shared/ecc.h"
#include "src/shared/tester.h"

struct user_data {
	const void *test_data;
	uint16_t index_ut;
	uint16_t index_lt;
	struct bt_hci *hci_ut;		/* Upper Tester / IUT */
	struct bt_hci *hci_lt;		/* Lower Tester / Reference */

	uint8_t bdaddr_ut[6];
	uint8_t bdaddr_lt[6];
	uint16_t handle_ut;
};

struct le_keys {
	uint8_t remote_sk[32];
	uint8_t local_pk[64];
} key_test_data;

static void swap_buf(const uint8_t *src, uint8_t *dst, uint16_t len)
{
	int i;

	for (i = 0; i < len; i++)
		dst[len - 1 - i] = src[i];
}

static void test_debug(const char *str, void *user_data)
{
	tester_debug("%s", str);
}

static void test_pre_setup_lt_address(const void *data, uint8_t size,
							void *user_data)
{
	struct user_data *user = tester_get_data();
	const struct bt_hci_rsp_read_bd_addr *rsp = data;

	if (rsp->status) {
		tester_warn("Read lower tester address failed (0x%02x)",
								rsp->status);
		tester_pre_setup_failed();
		return;
	}

	memcpy(user->bdaddr_lt, rsp->bdaddr, 6);

	tester_pre_setup_complete();
}

static void test_pre_setup_lt_complete(const void *data, uint8_t size,
							void *user_data)
{
	struct user_data *user = tester_get_data();
	uint8_t status = *((uint8_t *) data);

	if (status) {
		tester_warn("Reset lower tester failed (0x%02x)", status);
		tester_pre_setup_failed();
		return;
	}

	if (!bt_hci_send(user->hci_lt, BT_HCI_CMD_READ_BD_ADDR, NULL, 0,
				test_pre_setup_lt_address, NULL, NULL)) {
		tester_warn("Failed to read lower tester address");
		tester_pre_setup_failed();
		return;
	}
}

static void test_pre_setup_ut_address(const void *data, uint8_t size,
							void *user_data)
{
	struct user_data *user = tester_get_data();
	const struct bt_hci_rsp_read_bd_addr *rsp = data;

	if (rsp->status) {
		tester_warn("Read upper tester address failed (0x%02x)",
								rsp->status);
		tester_pre_setup_failed();
		return;
	}

	memcpy(user->bdaddr_ut, rsp->bdaddr, 6);

	user->hci_lt = bt_hci_new_user_channel(user->index_lt);
	if (!user->hci_lt) {
		tester_warn("Failed to setup lower tester user channel");
		tester_pre_setup_failed();
		return;
	}

	if (!bt_hci_send(user->hci_lt, BT_HCI_CMD_RESET, NULL, 0,
				test_pre_setup_lt_complete, NULL, NULL)) {
		tester_warn("Failed to reset lower tester");
		tester_pre_setup_failed();
		return;
	}
}

static void test_pre_setup_ut_complete(const void *data, uint8_t size,
							void *user_data)
{
	struct user_data *user = tester_get_data();
	uint8_t status = *((uint8_t *) data);

	if (status) {
		tester_warn("Reset upper tester failed (0x%02x)", status);
		tester_pre_setup_failed();
		return;
	}

	if (user->index_lt == 0xffff) {
		tester_pre_setup_complete();
		return;
	}

	if (!bt_hci_send(user->hci_ut, BT_HCI_CMD_READ_BD_ADDR, NULL, 0,
				test_pre_setup_ut_address, NULL, NULL)) {
		tester_warn("Failed to read upper tester address");
		tester_pre_setup_failed();
		return;
	}
}

static void test_pre_setup(const void *test_data)
{
	struct user_data *user = tester_get_data();

	user->hci_ut = bt_hci_new_user_channel(user->index_ut);
	if (!user->hci_ut) {
		tester_warn("Failed to setup upper tester user channel");
		tester_pre_setup_failed();
		return;
	}

	if (!bt_hci_send(user->hci_ut, BT_HCI_CMD_RESET, NULL, 0,
				test_pre_setup_ut_complete, NULL, NULL)) {
		tester_warn("Failed to reset upper tester");
		tester_pre_setup_failed();
		return;
	}
}

static void test_post_teardown(const void *test_data)
{
	struct user_data *user = tester_get_data();

	bt_hci_unref(user->hci_lt);
	user->hci_lt = NULL;

	bt_hci_unref(user->hci_ut);
	user->hci_ut = NULL;

	tester_post_teardown_complete();
}

static void user_data_free(void *data)
{
	struct user_data *user = data;

	free(user);
}

#define test_hci(name, data, setup, func, teardown) \
	do { \
		struct user_data *user; \
		user = calloc(1, sizeof(struct user_data)); \
		if (!user) \
			break; \
		user->test_data = data; \
		user->index_ut = 0; \
		user->index_lt = 1; \
		tester_add_full(name, data, \
				test_pre_setup, setup, func, teardown, \
				test_post_teardown, 30, user, user_data_free); \
	} while (0)

#define test_hci_local(name, data, setup, func) \
	do { \
		struct user_data *user; \
		user = calloc(1, sizeof(struct user_data)); \
		if (!user) \
			break; \
		user->test_data = data; \
		user->index_ut = 0; \
		user->index_lt = 0xffff; \
		tester_add_full(name, data, \
				test_pre_setup, setup, func, NULL, \
				test_post_teardown, 30, user, user_data_free); \
	} while (0)

static void setup_features_complete(const void *data, uint8_t size,
							void *user_data)
{
	const struct bt_hci_rsp_read_local_features *rsp = data;

	if (rsp->status) {
		tester_warn("Failed to get HCI features (0x%02x)", rsp->status);
		tester_setup_failed();
		return;
	}

	tester_setup_complete();
}

static void setup_features(const void *test_data)
{
	struct user_data *user = tester_get_data();

	if (!bt_hci_send(user->hci_ut, BT_HCI_CMD_READ_LOCAL_FEATURES, NULL, 0,
					setup_features_complete, NULL, NULL)) {
		tester_warn("Failed to send HCI features command");
		tester_setup_failed();
		return;
	}
}

static void test_reset(const void *test_data)
{
	tester_test_passed();
}

static void test_command_complete(const void *data, uint8_t size,
							void *user_data)
{
	uint8_t status = *((uint8_t *) data);

	if (status) {
		tester_warn("HCI command failed (0x%02x)", status);
		tester_test_failed();
		return;
	}

	tester_test_passed();
}

static void test_command(uint16_t opcode)
{
	struct user_data *user = tester_get_data();

	if (!bt_hci_send(user->hci_ut, opcode, NULL, 0,
					test_command_complete, NULL, NULL)) {
		tester_warn("Failed to send HCI command 0x%04x", opcode);
		tester_test_failed();
		return;
	}
}

static void test_read_local_version_information(const void *test_data)
{
	test_command(BT_HCI_CMD_READ_LOCAL_VERSION);
}

static void test_read_local_supported_commands(const void *test_data)
{
	test_command(BT_HCI_CMD_READ_LOCAL_COMMANDS);
}

static void test_read_local_supported_features(const void *test_data)
{
	test_command(BT_HCI_CMD_READ_LOCAL_FEATURES);
}

static void test_local_extended_features_complete(const void *data,
						uint8_t size, void *user_data)
{
	const struct bt_hci_rsp_read_local_ext_features *rsp = data;

	if (rsp->status) {
		tester_warn("Failed to get HCI extended features (0x%02x)",
								rsp->status);
		tester_test_failed();
		return;
	}

	tester_test_passed();
}

static void test_read_local_extended_features(const void *test_data)
{
	struct user_data *user = tester_get_data();
	struct bt_hci_cmd_read_local_ext_features cmd;

	cmd.page = 0x00;

	if (!bt_hci_send(user->hci_ut, BT_HCI_CMD_READ_LOCAL_EXT_FEATURES,
					&cmd, sizeof(cmd),
					test_local_extended_features_complete,
								NULL, NULL)) {
		tester_warn("Failed to send HCI extended features command");
		tester_test_failed();
		return;
	}
}

static void test_read_buffer_size(const void *test_data)
{
	test_command(BT_HCI_CMD_READ_BUFFER_SIZE);
}

static void test_read_country_code(const void *test_data)
{
	test_command(BT_HCI_CMD_READ_COUNTRY_CODE);
}

static void test_read_bd_addr(const void *test_data)
{
	test_command(BT_HCI_CMD_READ_BD_ADDR);
}

static void test_read_local_supported_codecs(const void *test_data)
{
	test_command(BT_HCI_CMD_READ_LOCAL_CODECS);
}

static void test_le_read_white_list_size(const void *test_data)
{
	test_command(BT_HCI_CMD_LE_READ_WHITE_LIST_SIZE);
}

static void test_le_clear_white_list(const void *test_data)
{
	test_command(BT_HCI_CMD_LE_CLEAR_WHITE_LIST);
}

static void test_le_encrypt_complete(const void *data, uint8_t size,
								void *user_data)
{
	const struct bt_hci_rsp_le_encrypt *rsp = data;
	uint8_t sample[16] = {
		0x7d, 0xf7, 0x6b, 0x0c, 0x1a, 0xb8, 0x99, 0xb3,
		0x3e, 0x42, 0xf0, 0x47, 0xb9, 0x1b, 0x54, 0x6f
	};
	uint8_t enc_data[16];

	if (rsp->status) {
		tester_warn("Failed HCI LE Encrypt (0x%02x)", rsp->status);
		tester_test_failed();
		return;
	}

	swap_buf(rsp->data, enc_data, 16);
	util_hexdump('>', enc_data, 16, test_debug, NULL);

	if (!memcmp(sample, enc_data, 16))
		tester_test_passed();
	else
		tester_test_failed();
}

/* Data are taken from RFC 4493 Test Vectors */
static void test_le_encrypt(const void *test_data)
{
	struct user_data *user = tester_get_data();
	struct bt_hci_cmd_le_encrypt cmd;
	uint8_t key[16] = {
		0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6,
		0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c
	};
	uint8_t plaintext[16] = { 0 };

	/* Swap bytes since our interface has LE interface, opposed to
	 * common crypto interface
	 */
	swap_buf(key, cmd.key, 16);
	swap_buf(plaintext, cmd.plaintext, 16);

	util_hexdump('<', cmd.key, 16, test_debug, NULL);
	util_hexdump('<', cmd.plaintext, 16, test_debug, NULL);

	if (!bt_hci_send(user->hci_ut, BT_HCI_CMD_LE_ENCRYPT, &cmd, sizeof(cmd),
					test_le_encrypt_complete, NULL, NULL)) {
		tester_warn("Failed to send HCI LE Encrypt command");
		tester_test_failed();
		return;
	}

}

static void test_le_rand(const void *test_data)
{
	test_command(BT_HCI_CMD_LE_RAND);
}

static void test_le_read_local_pk_complete(const void *data, uint8_t size,
								void *user_data)
{
	const uint8_t *event = data;
	const struct bt_hci_evt_le_read_local_pk256_complete *evt;
	struct le_keys *keys = user_data;

	if (*event != BT_HCI_EVT_LE_READ_LOCAL_PK256_COMPLETE) {
		tester_warn("Failed Read Local PK256 command");
		tester_test_failed();
		return;
	}

	evt = (void *)(event + 1);
	if (evt->status) {
		tester_warn("HCI Read Local PK complete failed (0x%02x)",
								evt->status);
		tester_test_failed();
		return;
	}

	memcpy(keys->local_pk, evt->local_pk256, 64);

	util_hexdump('>', evt->local_pk256, 64, test_debug, NULL);

	tester_test_passed();
}

static void test_le_read_local_pk_status(const void *data, uint8_t size,
							void *user_data)
{
	uint8_t status = *((uint8_t *) data);

	if (status) {
		tester_warn("Failed to send DHKey gen cmd (0x%02x)", status);
		tester_test_failed();
		return;
	}
}

static void test_le_read_local_pk(const void *test_data)
{
	struct user_data *user = tester_get_data();

	bt_hci_register(user->hci_ut, BT_HCI_EVT_LE_META_EVENT,
				test_le_read_local_pk_complete,
				(void *)test_data, NULL);

	if (!bt_hci_send(user->hci_ut, BT_HCI_CMD_LE_READ_LOCAL_PK256, NULL,
				0, test_le_read_local_pk_status,
				NULL, NULL)) {
		tester_warn("Failed to send HCI LE Read Local PK256 command");
		tester_test_failed();
		return;
	}
}

static void setup_le_read_local_pk_complete(const void *data, uint8_t size,
								void *user_data)
{
	const uint8_t *event = data;
	const struct bt_hci_evt_le_read_local_pk256_complete *evt;
	struct le_keys *keys = user_data;

	if (*event != BT_HCI_EVT_LE_READ_LOCAL_PK256_COMPLETE) {
		tester_warn("Failed Read Local PK256 command");
		tester_setup_failed();
		return;
	}

	evt = (void *)(event + 1);
	if (evt->status) {
		tester_warn("HCI Read Local PK complete failed (0x%02x)",
								evt->status);
		tester_setup_failed();
		return;
	}

	memcpy(keys->local_pk, evt->local_pk256, 64);

	util_hexdump('>', evt->local_pk256, 64, test_debug, NULL);

	tester_setup_complete();
}

static void setup_le_read_local_pk_status(const void *data, uint8_t size,
							void *user_data)
{
	uint8_t status = *((uint8_t *) data);

	if (status) {
		tester_warn("Failed to send DHKey gen cmd (0x%02x)", status);
		tester_setup_failed();
		return;
	}
}

static void setup_le_generate_dhkey(const void *test_data)
{
	struct user_data *user = tester_get_data();

	bt_hci_register(user->hci_ut, BT_HCI_EVT_LE_META_EVENT,
				setup_le_read_local_pk_complete,
				(void *)test_data, NULL);

	if (!bt_hci_send(user->hci_ut, BT_HCI_CMD_LE_READ_LOCAL_PK256, NULL,
				0, setup_le_read_local_pk_status,
				NULL, NULL)) {
		tester_warn("Failed to send HCI LE Read Local PK256 command");
		tester_setup_failed();
		return;
	}
}

static void test_le_generate_dhkey_complete(const void *data, uint8_t size,
								void *user_data)
{
	const uint8_t *event = data;
	const struct bt_hci_evt_le_generate_dhkey_complete *evt;
	struct le_keys *keys = user_data;
	uint8_t dhkey[32];

	if (*event != BT_HCI_EVT_LE_GENERATE_DHKEY_COMPLETE) {
		tester_warn("Failed DHKey generation command");
		tester_test_failed();
		return;
	}

	evt = (void *)(event + 1);
	if (evt->status) {
		tester_warn("HCI Generate DHKey complete failed (0x%02x)",
								evt->status);
		tester_test_failed();
		return;
	}

	util_hexdump('>', evt->dhkey, 32, test_debug, NULL);


	util_hexdump('S', keys->remote_sk, 32, test_debug, NULL);
	util_hexdump('P', keys->local_pk, 64, test_debug, NULL);

	/* Generate DHKey ourself with local public key and remote
	 * private key we got when generated public / private key
	 * pair for BT_HCI_CMD_LE_GENERATE_DHKEY argument.
	 */
	ecdh_shared_secret(keys->local_pk, keys->remote_sk, dhkey);

	util_hexdump('D', dhkey, 32, test_debug, NULL);

	if (!memcmp(dhkey, evt->dhkey, 32))
		tester_test_passed();
	else
		tester_test_failed();
}

static void test_le_generate_dhkey_status(const void *data, uint8_t size,
							void *user_data)
{
	uint8_t status = *((uint8_t *) data);

	if (status) {
		tester_warn("Failed to send DHKey gen cmd (0x%02x)", status);
		tester_test_failed();
		return;
	}
}

static void test_le_generate_dhkey(const void *test_data)
{
	struct user_data *user = tester_get_data();
	struct bt_hci_cmd_le_generate_dhkey cmd;
	struct le_keys *keys = (void *)test_data;

	ecc_make_key(cmd.remote_pk256, keys->remote_sk);

	/* Unregister handler for META event */
	bt_hci_unregister(user->hci_ut, 1);

	bt_hci_register(user->hci_ut, BT_HCI_EVT_LE_META_EVENT,
				test_le_generate_dhkey_complete, keys,
				NULL);

	if (!bt_hci_send(user->hci_ut, BT_HCI_CMD_LE_GENERATE_DHKEY, &cmd,
				sizeof(cmd), test_le_generate_dhkey_status,
				NULL, NULL)) {
		tester_warn("Failed to send HCI LE Encrypt command");
		tester_test_failed();
		return;
	}

}

static void test_inquiry_complete(const void *data, uint8_t size,
							void *user_data)
{
	const struct bt_hci_evt_inquiry_complete *evt = data;

	if (evt->status) {
		tester_warn("HCI inquiry complete failed (0x%02x)",
							evt->status);
		tester_test_failed();
		return;
	}

	tester_test_passed();
}

static void test_inquiry_status(const void *data, uint8_t size,
							void *user_data)
{
	uint8_t status = *((uint8_t *) data);

	if (status) {
		tester_warn("HCI inquiry command failed (0x%02x)", status);
		tester_test_failed();
		return;
	}
}

static void test_inquiry_liac(const void *test_data)
{
	struct user_data *user = tester_get_data();
	struct bt_hci_cmd_inquiry cmd;

	bt_hci_register(user->hci_ut, BT_HCI_EVT_INQUIRY_COMPLETE,
					test_inquiry_complete, NULL, NULL);

	cmd.lap[0] = 0x00;
	cmd.lap[1] = 0x8b;
	cmd.lap[2] = 0x9e;
	cmd.length = 0x08;
	cmd.num_resp = 0x00;

	if (!bt_hci_send(user->hci_ut, BT_HCI_CMD_INQUIRY, &cmd, sizeof(cmd),
					test_inquiry_status, NULL, NULL)) {
		tester_warn("Failed to send HCI inquiry command");
		tester_test_failed();
		return;
	}
}

static void setup_lt_connectable_complete(const void *data, uint8_t size,
							void *user_data)
{
	uint8_t status = *((uint8_t *) data);

	if (status) {
		tester_warn("Failed to set HCI scan enable (0x%02x)", status);
		tester_setup_failed();
		return;
	}

	tester_setup_complete();
}

static void setup_lt_connect_request_accept(const void *data, uint8_t size,
							void *user_data)
{
	struct user_data *user = tester_get_data();
	const struct bt_hci_evt_conn_request *evt = data;
	struct bt_hci_cmd_accept_conn_request cmd;

	memcpy(cmd.bdaddr, evt->bdaddr, 6);
	cmd.role = 0x01;

	if (!bt_hci_send(user->hci_lt, BT_HCI_CMD_ACCEPT_CONN_REQUEST,
					&cmd, sizeof(cmd), NULL, NULL, NULL)) {
		tester_warn("Failed to send HCI accept connection command");
		return;
	}
}

static void setup_lt_connectable(const void *test_data)
{
	struct user_data *user = tester_get_data();
	struct bt_hci_cmd_write_scan_enable cmd;

	bt_hci_register(user->hci_lt, BT_HCI_EVT_CONN_REQUEST,
				setup_lt_connect_request_accept, NULL, NULL);

	cmd.enable = 0x02;

	if (!bt_hci_send(user->hci_lt, BT_HCI_CMD_WRITE_SCAN_ENABLE,
				&cmd, sizeof(cmd),
				setup_lt_connectable_complete, NULL, NULL)) {
		tester_warn("Failed to send HCI scan enable command");
		tester_setup_failed();
		return;
	}
}

static void test_create_connection_disconnect(void *user_data)
{
	tester_test_passed();
}

static void test_create_connection_complete(const void *data, uint8_t size,
							void *user_data)
{
	struct user_data *user = tester_get_data();
	const struct bt_hci_evt_conn_complete *evt = data;

	if (evt->status) {
		tester_warn("HCI create connection complete failed (0x%02x)",
								evt->status);
		tester_test_failed();
		return;
	}

	user->handle_ut = le16_to_cpu(evt->handle);

	tester_wait(2, test_create_connection_disconnect, NULL);
}

static void test_create_connection_status(const void *data, uint8_t size,
							void *user_data)
{
	uint8_t status = *((uint8_t *) data);

	if (status) {
		tester_warn("HCI create connection command failed (0x%02x)",
								status);
		tester_test_failed();
		return;
	}
}

static void test_create_connection(const void *test_data)
{
	struct user_data *user = tester_get_data();
	struct bt_hci_cmd_create_conn cmd;

	bt_hci_register(user->hci_ut, BT_HCI_EVT_CONN_COMPLETE,
				test_create_connection_complete, NULL, NULL);

	memcpy(cmd.bdaddr, user->bdaddr_lt, 6);
	cmd.pkt_type = cpu_to_le16(0x0008);
	cmd.pscan_rep_mode = 0x02;
	cmd.pscan_mode = 0x00;
	cmd.clock_offset = cpu_to_le16(0x0000);
	cmd.role_switch = 0x01;

	if (!bt_hci_send(user->hci_ut, BT_HCI_CMD_CREATE_CONN,
						&cmd, sizeof(cmd),
						test_create_connection_status,
								NULL, NULL)) {
		tester_warn("Failed to send HCI create connection command");
		tester_test_failed();
		return;
	}
}

static void teardown_timeout(void *user_data)
{
	tester_teardown_complete();
}

static void teardown_disconnect_status(const void *data, uint8_t size,
							void *user_data)
{
	uint8_t status = *((uint8_t *) data);

	if (status) {
		tester_warn("HCI disconnect failed (0x%02x)", status);
		tester_teardown_failed();
		return;
	}

	tester_wait(1, teardown_timeout, NULL);
}

static void teardown_connection(const void *test_data)
{
	struct user_data *user = tester_get_data();
	struct bt_hci_cmd_disconnect cmd;

	cmd.handle = cpu_to_le16(user->handle_ut);
	cmd.reason = 0x13;

	if (!bt_hci_send(user->hci_ut, BT_HCI_CMD_DISCONNECT,
						&cmd, sizeof(cmd),
						teardown_disconnect_status,
								NULL, NULL)) {
		tester_warn("Failed to send HCI disconnect command");
		tester_test_failed();
		return;
	}
}

static void test_adv_report(const void *data, uint8_t size, void *user_data)
{
	struct user_data *user = tester_get_data();
	uint8_t subevent = *((uint8_t *) data);
	const struct bt_hci_evt_le_adv_report *lar = data + 1;

	switch (subevent) {
	case BT_HCI_EVT_LE_ADV_REPORT:
		if (!memcmp(lar->addr, user->bdaddr_ut, 6))
			tester_setup_complete();
		break;
	}
}

static void setup_advertising_initiated(const void *test_data)
{
	struct user_data *user = tester_get_data();
	struct bt_hci_cmd_set_event_mask sem;
	struct bt_hci_cmd_le_set_event_mask lsem;
	struct bt_hci_cmd_le_set_scan_enable lsse;
	struct bt_hci_cmd_le_set_adv_parameters lsap;
	struct bt_hci_cmd_le_set_adv_enable lsae;

	bt_hci_register(user->hci_lt, BT_HCI_EVT_LE_META_EVENT,
					test_adv_report, NULL, NULL);

	memset(sem.mask, 0, 8);
	sem.mask[1] |= 0x20;	/* Command Complete */
	sem.mask[1] |= 0x40;	/* Command Status */
	sem.mask[7] |= 0x20;	/* LE Meta */

	bt_hci_send(user->hci_lt, BT_HCI_CMD_SET_EVENT_MASK,
					&sem, sizeof(sem), NULL, NULL, NULL);

	memset(lsem.mask, 0, 8);
	lsem.mask[0] |= 0x02;	/* LE Advertising Report */

	bt_hci_send(user->hci_lt, BT_HCI_CMD_LE_SET_EVENT_MASK,
					&lsem, sizeof(lsem), NULL, NULL, NULL);

	lsse.enable = 0x01;
	lsse.filter_dup = 0x00;

	bt_hci_send(user->hci_lt, BT_HCI_CMD_LE_SET_SCAN_ENABLE,
					&lsse, sizeof(lsse), NULL, NULL, NULL);

	lsap.min_interval = cpu_to_le16(0x0800);
	lsap.max_interval = cpu_to_le16(0x0800);
	lsap.type = 0x03;
	lsap.own_addr_type = 0x00;
	lsap.direct_addr_type = 0x00;
	memset(lsap.direct_addr, 0, 6);
	lsap.channel_map = 0x07;
	lsap.filter_policy = 0x00;

	bt_hci_send(user->hci_ut, BT_HCI_CMD_LE_SET_ADV_PARAMETERS,
					&lsap, sizeof(lsap), NULL, NULL, NULL);

	lsae.enable = 0x01;

	bt_hci_send(user->hci_ut, BT_HCI_CMD_LE_SET_ADV_ENABLE,
					&lsae, sizeof(lsae), NULL, NULL, NULL);
}

static void test_reset_in_advertising_state_timeout(void *user_data)
{
	struct user_data *user = tester_get_data();
	struct bt_hci_cmd_le_set_adv_enable lsae;
	struct bt_hci_cmd_le_set_scan_enable lsse;

	lsae.enable = 0x00;

	bt_hci_send(user->hci_ut, BT_HCI_CMD_LE_SET_ADV_ENABLE,
					&lsae, sizeof(lsae), NULL, NULL, NULL);

	lsse.enable = 0x00;
	lsse.filter_dup = 0x00;

	bt_hci_send(user->hci_lt, BT_HCI_CMD_LE_SET_SCAN_ENABLE,
					&lsse, sizeof(lsse), NULL, NULL, NULL);

	tester_test_passed();
}

static void test_reset_in_advertising_state(const void *test_data)
{
	struct user_data *user = tester_get_data();

	bt_hci_send(user->hci_ut, BT_HCI_CMD_RESET, NULL, 0, NULL, NULL, NULL);

	tester_wait(5, test_reset_in_advertising_state_timeout, NULL);
}

int main(int argc, char *argv[])
{
	tester_init(&argc, &argv);

	test_hci_local("Reset", NULL, NULL, test_reset);

	test_hci_local("Read Local Version Information", NULL, NULL,
				test_read_local_version_information);
	test_hci_local("Read Local Supported Commands", NULL, NULL,
				test_read_local_supported_commands);
	test_hci_local("Read Local Supported Features", NULL, NULL,
				test_read_local_supported_features);
	test_hci_local("Read Local Extended Features", NULL,
				setup_features,
				test_read_local_extended_features);
	test_hci_local("Read Buffer Size", NULL, NULL,
				test_read_buffer_size);
	test_hci_local("Read Country Code", NULL, NULL,
				test_read_country_code);
	test_hci_local("Read BD_ADDR", NULL, NULL,
				test_read_bd_addr);
	test_hci_local("Read Local Supported Codecs", NULL, NULL,
				test_read_local_supported_codecs);

	test_hci_local("LE Read White List Size", NULL, NULL,
				test_le_read_white_list_size);
	test_hci_local("LE Clear White List", NULL, NULL,
				test_le_clear_white_list);
	test_hci_local("LE Encrypt", NULL, NULL,
				test_le_encrypt);
	test_hci_local("LE Rand", NULL, NULL,
				test_le_rand);
	test_hci_local("LE Read Local PK", &key_test_data, NULL,
				test_le_read_local_pk);
	test_hci_local("LE Generate DHKey", &key_test_data,
				setup_le_generate_dhkey,
				test_le_generate_dhkey);

	test_hci_local("Inquiry (LIAC)", NULL, NULL, test_inquiry_liac);

	test_hci("Create Connection", NULL,
				setup_lt_connectable,
				test_create_connection,
				teardown_connection);

	test_hci("TP/DSU/BV-02-C Reset in Advertising State", NULL,
				setup_advertising_initiated,
				test_reset_in_advertising_state, NULL);

	return tester_run();
}

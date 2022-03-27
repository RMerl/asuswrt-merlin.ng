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
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/socket.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/mgmt.h"

#include "monitor/bt.h"
#include "emulator/bthost.h"
#include "emulator/hciemu.h"

#include "src/shared/crypto.h"
#include "src/shared/ecc.h"
#include "src/shared/tester.h"
#include "src/shared/mgmt.h"

#define SMP_CID 0x0006

struct test_data {
	const void *test_data;
	struct mgmt *mgmt;
	uint16_t mgmt_index;
	struct hciemu *hciemu;
	enum hciemu_type hciemu_type;
	unsigned int io_id;
	uint8_t ia[6];
	uint8_t ia_type;
	uint8_t ra[6];
	uint8_t ra_type;
	bool out;
	uint16_t handle;
	size_t counter;
	struct bt_crypto *crypto;
	uint8_t tk[16];
	uint8_t prnd[16];
	uint8_t rrnd[16];
	uint8_t pcnf[16];
	uint8_t preq[7];
	uint8_t prsp[7];
	uint8_t ltk[16];
	uint8_t remote_pk[64];
	uint8_t local_pk[64];
	uint8_t local_sk[32];
	uint8_t dhkey[32];
	int unmet_conditions;
};

struct smp_req_rsp {
	const void *send;
	uint16_t send_len;
	const void *expect;
	uint16_t expect_len;
};

struct smp_data {
	const struct smp_req_rsp *req;
	size_t req_count;
	bool mitm;
	uint16_t expect_hci_command;
	const void *expect_hci_param;
	uint8_t expect_hci_len;
	const void * (*expect_hci_func)(uint8_t *len);
	bool sc;
};

static void mgmt_debug(const char *str, void *user_data)
{
	const char *prefix = user_data;

	tester_print("%s%s", prefix, str);
}

static void read_info_callback(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();
	const struct mgmt_rp_read_info *rp = param;
	char addr[18];
	uint16_t manufacturer;
	uint32_t supported_settings, current_settings;

	tester_print("Read Info callback");
	tester_print("  Status: 0x%02x", status);

	if (status || !param) {
		tester_pre_setup_failed();
		return;
	}

	ba2str(&rp->bdaddr, addr);
	manufacturer = btohs(rp->manufacturer);
	supported_settings = btohl(rp->supported_settings);
	current_settings = btohl(rp->current_settings);

	tester_print("  Address: %s", addr);
	tester_print("  Version: 0x%02x", rp->version);
	tester_print("  Manufacturer: 0x%04x", manufacturer);
	tester_print("  Supported settings: 0x%08x", supported_settings);
	tester_print("  Current settings: 0x%08x", current_settings);
	tester_print("  Class: 0x%02x%02x%02x",
			rp->dev_class[2], rp->dev_class[1], rp->dev_class[0]);
	tester_print("  Name: %s", rp->name);
	tester_print("  Short name: %s", rp->short_name);

	if (strcmp(hciemu_get_address(data->hciemu), addr)) {
		tester_pre_setup_failed();
		return;
	}

	tester_pre_setup_complete();
}

static void index_added_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();

	tester_print("Index Added callback");
	tester_print("  Index: 0x%04x", index);

	data->mgmt_index = index;

	mgmt_send(data->mgmt, MGMT_OP_READ_INFO, data->mgmt_index, 0, NULL,
					read_info_callback, NULL, NULL);
}

static void index_removed_callback(uint16_t index, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();

	tester_print("Index Removed callback");
	tester_print("  Index: 0x%04x", index);

	if (index != data->mgmt_index)
		return;

	mgmt_unregister_index(data->mgmt, data->mgmt_index);

	mgmt_unref(data->mgmt);
	data->mgmt = NULL;

	tester_post_teardown_complete();
}

static void read_index_list_callback(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();

	tester_print("Read Index List callback");
	tester_print("  Status: 0x%02x", status);

	if (status || !param) {
		tester_pre_setup_failed();
		return;
	}

	mgmt_register(data->mgmt, MGMT_EV_INDEX_ADDED, MGMT_INDEX_NONE,
					index_added_callback, NULL, NULL);

	mgmt_register(data->mgmt, MGMT_EV_INDEX_REMOVED, MGMT_INDEX_NONE,
					index_removed_callback, NULL, NULL);

	data->hciemu = hciemu_new(data->hciemu_type);
	if (!data->hciemu) {
		tester_warn("Failed to setup HCI emulation");
		tester_pre_setup_failed();
	}

	tester_print("New hciemu instance created");
}

static void test_pre_setup(const void *test_data)
{
	struct test_data *data = tester_get_data();

	data->crypto = bt_crypto_new();
	if (!data->crypto) {
		tester_warn("Failed to setup crypto");
		tester_pre_setup_failed();
		return;
	}

	data->mgmt = mgmt_new_default();
	if (!data->mgmt) {
		tester_warn("Failed to setup management interface");
		bt_crypto_unref(data->crypto);
		tester_pre_setup_failed();
		return;
	}

	if (tester_use_debug())
		mgmt_set_debug(data->mgmt, mgmt_debug, "mgmt: ", NULL);

	mgmt_send(data->mgmt, MGMT_OP_READ_INDEX_LIST, MGMT_INDEX_NONE, 0, NULL,
					read_index_list_callback, NULL, NULL);
}

static void test_post_teardown(const void *test_data)
{
	struct test_data *data = tester_get_data();

	if (data->io_id > 0) {
		g_source_remove(data->io_id);
		data->io_id = 0;
	}

	if (data->crypto) {
		bt_crypto_unref(data->crypto);
		data->crypto = NULL;
	}

	hciemu_unref(data->hciemu);
	data->hciemu = NULL;
}

static void test_data_free(void *test_data)
{
	struct test_data *data = test_data;

	free(data);
}

static void test_add_condition(struct test_data *data)
{
	data->unmet_conditions++;

	tester_print("Test condition added, total %d", data->unmet_conditions);
}

static void test_condition_complete(struct test_data *data)
{
	data->unmet_conditions--;

	tester_print("Test condition complete, %d left",
						data->unmet_conditions);

	if (data->unmet_conditions > 0)
		return;

	tester_test_passed();
}

#define test_smp(name, data, setup, func) \
	do { \
		struct test_data *user; \
		user = calloc(1, sizeof(struct test_data)); \
		if (!user) \
			break; \
		user->hciemu_type = HCIEMU_TYPE_BREDRLE; \
		user->test_data = data; \
		tester_add_full(name, data, \
				test_pre_setup, setup, func, NULL, \
				test_post_teardown, 2, user, test_data_free); \
	} while (0)

static const uint8_t smp_nval_req_1[] = { 0x0b, 0x00 };
static const uint8_t smp_nval_req_1_rsp[] = { 0x05, 0x07 };

static const struct smp_req_rsp nval_req_1[] = {
	{ smp_nval_req_1, sizeof(smp_nval_req_1),
			smp_nval_req_1_rsp, sizeof(smp_nval_req_1_rsp) },
};

static const struct smp_data smp_server_nval_req_1_test = {
	.req = nval_req_1,
	.req_count = G_N_ELEMENTS(nval_req_1),
};

static const uint8_t smp_nval_req_2[7] = { 0x01 };
static const uint8_t smp_nval_req_2_rsp[] = { 0x05, 0x06 };

static const struct smp_req_rsp srv_nval_req_1[] = {
	{ smp_nval_req_2, sizeof(smp_nval_req_2),
			smp_nval_req_2_rsp, sizeof(smp_nval_req_2_rsp) },
};

static const struct smp_data smp_server_nval_req_2_test = {
	.req = srv_nval_req_1,
	.req_count = G_N_ELEMENTS(srv_nval_req_1),
};

static const uint8_t smp_nval_req_3[] = { 0x01, 0xff };
static const uint8_t smp_nval_req_3_rsp[] = { 0x05, 0x0a };

static const struct smp_req_rsp srv_nval_req_2[] = {
	{ smp_nval_req_3, sizeof(smp_nval_req_3),
			smp_nval_req_3_rsp, sizeof(smp_nval_req_3_rsp) },
};

static const struct smp_data smp_server_nval_req_3_test = {
	.req = srv_nval_req_2,
	.req_count = G_N_ELEMENTS(srv_nval_req_2),
};

static const uint8_t smp_basic_req_1[] = {	0x01,	/* Pairing Request */
						0x03,	/* NoInputNoOutput */
						0x00,	/* OOB Flag */
						0x01,	/* Bonding - no MITM */
						0x10,	/* Max key size */
						0x05,	/* Init. key dist. */
						0x05,	/* Rsp. key dist. */
};
static const uint8_t smp_basic_req_1_rsp[] = {	0x02,	/* Pairing Response */
						0x03,	/* NoInputNoOutput */
						0x00,	/* OOB Flag */
						0x01,	/* Bonding - no MITM */
						0x10,	/* Max key size */
						0x05,	/* Init. key dist. */
						0x05,	/* Rsp. key dist. */
};

static const uint8_t smp_confirm_req_1[17] = { 0x03 };
static const uint8_t smp_random_req_1[17] = { 0x04 };

static const struct smp_req_rsp srv_basic_req_1[] = {
	{ smp_basic_req_1, sizeof(smp_basic_req_1),
			smp_basic_req_1_rsp, sizeof(smp_basic_req_1_rsp) },
	{ smp_confirm_req_1, sizeof(smp_confirm_req_1),
			smp_confirm_req_1, sizeof(smp_confirm_req_1) },
	{ smp_random_req_1, sizeof(smp_random_req_1),
			smp_random_req_1, sizeof(smp_random_req_1) },
};

static const struct smp_data smp_server_basic_req_1_test = {
	.req = srv_basic_req_1,
	.req_count = G_N_ELEMENTS(srv_basic_req_1),
};

static const struct smp_req_rsp cli_basic_req_1[] = {
	{ NULL, 0, smp_basic_req_1, sizeof(smp_basic_req_1) },
	{ smp_basic_req_1_rsp, sizeof(smp_basic_req_1_rsp),
			smp_confirm_req_1, sizeof(smp_confirm_req_1) },
	{ smp_confirm_req_1, sizeof(smp_confirm_req_1),
			smp_random_req_1, sizeof(smp_random_req_1) },
	{ smp_random_req_1, sizeof(smp_random_req_1), NULL, 0 },
};

static const struct smp_data smp_client_basic_req_1_test = {
	.req = cli_basic_req_1,
	.req_count = G_N_ELEMENTS(cli_basic_req_1),
};

static const uint8_t smp_basic_req_2[] = {	0x01,	/* Pairing Request */
						0x04,	/* NoInputNoOutput */
						0x00,	/* OOB Flag */
						0x05,	/* Bonding - MITM */
						0x10,	/* Max key size */
						0x05,	/* Init. key dist. */
						0x05,	/* Rsp. key dist. */
};

static const struct smp_req_rsp cli_basic_req_2[] = {
	{ NULL, 0, smp_basic_req_2, sizeof(smp_basic_req_2) },
	{ smp_basic_req_1_rsp, sizeof(smp_basic_req_1_rsp),
			smp_confirm_req_1, sizeof(smp_confirm_req_1) },
	{ smp_confirm_req_1, sizeof(smp_confirm_req_1),
			smp_random_req_1, sizeof(smp_random_req_1) },
	{ smp_random_req_1, sizeof(smp_random_req_1), NULL, 0 },
};

static const struct smp_data smp_client_basic_req_2_test = {
	.req = cli_basic_req_2,
	.req_count = G_N_ELEMENTS(cli_basic_req_1),
	.mitm = true,
};

static void user_confirm_request_callback(uint16_t index, uint16_t length,
							const void *param,
							void *user_data)
{
	const struct mgmt_ev_user_confirm_request *ev = param;
	struct test_data *data = tester_get_data();
	struct mgmt_cp_user_confirm_reply cp;

	memset(&cp, 0, sizeof(cp));
	memcpy(&cp.addr, &ev->addr, sizeof(cp.addr));

	mgmt_reply(data->mgmt, MGMT_OP_USER_CONFIRM_REPLY,
			data->mgmt_index, sizeof(cp), &cp, NULL, NULL, NULL);
}

static const uint8_t smp_sc_req_1[] = {	0x01,	/* Pairing Request */
					0x03,	/* NoInputNoOutput */
					0x00,	/* OOB Flag */
					0x09,	/* Bonding - no MITM, SC */
					0x10,	/* Max key size */
					0x0d,	/* Init. key dist. */
					0x0d,	/* Rsp. key dist. */
};

static const struct smp_req_rsp cli_sc_req_1[] = {
	{ NULL, 0, smp_sc_req_1, sizeof(smp_sc_req_1) },
	{ smp_basic_req_1_rsp, sizeof(smp_basic_req_1_rsp),
			smp_confirm_req_1, sizeof(smp_confirm_req_1) },
	{ smp_confirm_req_1, sizeof(smp_confirm_req_1),
			smp_random_req_1, sizeof(smp_random_req_1) },
	{ smp_random_req_1, sizeof(smp_random_req_1), NULL, 0 },
};

static const struct smp_data smp_client_sc_req_1_test = {
	.req = cli_sc_req_1,
	.req_count = G_N_ELEMENTS(cli_sc_req_1),
	.sc = true,
};

static const uint8_t smp_sc_rsp_1[] = {	0x02,	/* Pairing Response */
					0x03,	/* NoInputNoOutput */
					0x00,	/* OOB Flag */
					0x09,	/* Bonding - no MITM, SC */
					0x10,	/* Max key size */
					0x0d,	/* Init. key dist. */
					0x0d,	/* Rsp. key dist. */
};

static const uint8_t smp_sc_pk[65] = { 0x0c };

static const struct smp_req_rsp cli_sc_req_2[] = {
	{ NULL, 0, smp_sc_req_1, sizeof(smp_sc_req_1) },
	{ smp_sc_rsp_1, sizeof(smp_sc_rsp_1), smp_sc_pk, sizeof(smp_sc_pk) },
	{ smp_sc_pk, sizeof(smp_sc_pk), NULL, 0 },
	{ smp_confirm_req_1, sizeof(smp_confirm_req_1),
				smp_random_req_1, sizeof(smp_random_req_1) },
	{ smp_random_req_1, sizeof(smp_random_req_1), NULL, 0 },
};

static const struct smp_data smp_client_sc_req_2_test = {
	.req = cli_sc_req_2,
	.req_count = G_N_ELEMENTS(cli_sc_req_2),
	.sc = true,
};

static void client_connectable_complete(uint16_t opcode, uint8_t status,
					const void *param, uint8_t len,
					void *user_data)
{
	if (opcode != BT_HCI_CMD_LE_SET_ADV_ENABLE)
		return;

	tester_print("Client set connectable status 0x%02x", status);

	if (status)
		tester_setup_failed();
	else
		tester_setup_complete();
}

static void setup_powered_client_callback(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost;

	if (status != MGMT_STATUS_SUCCESS) {
		tester_setup_failed();
		return;
	}

	tester_print("Controller powered on");

	bthost = hciemu_client_get_host(data->hciemu);
	bthost_set_cmd_complete_cb(bthost, client_connectable_complete, data);
	bthost_set_adv_enable(bthost, 0x01);
}

static void make_pk(struct test_data *data)
{
	if (!ecc_make_key(data->local_pk, data->local_sk)) {
		tester_print("Failed to general local ECDH keypair");
		tester_setup_failed();
		return;
	}
}

static void setup_powered_client(const void *test_data)
{
	struct test_data *data = tester_get_data();
	const struct smp_data *smp = data->test_data;
	unsigned char param[] = { 0x01 };

	tester_print("Powering on controller");

	mgmt_send(data->mgmt, MGMT_OP_SET_LE, data->mgmt_index,
				sizeof(param), param, NULL, NULL, NULL);
	mgmt_send(data->mgmt, MGMT_OP_SET_BONDABLE, data->mgmt_index,
				sizeof(param), param, NULL, NULL, NULL);
	if (smp->sc) {
		mgmt_send(data->mgmt, MGMT_OP_SET_SSP, data->mgmt_index,
				sizeof(param), param, NULL, NULL, NULL);
		mgmt_send(data->mgmt, MGMT_OP_SET_SECURE_CONN,
				data->mgmt_index, sizeof(param), param, NULL,
				NULL, NULL);
		make_pk(data);
	}

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
			sizeof(param), param, setup_powered_client_callback,
			NULL, NULL);
}

static void pair_device_complete(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	if (status != MGMT_STATUS_SUCCESS) {
		tester_warn("Pairing failed: %s", mgmt_errstr(status));
		return;
	}

	tester_print("Pairing succeedded");
}

static const void *get_pdu(const uint8_t *pdu)
{
	struct test_data *data = tester_get_data();
	const struct smp_data *smp = data->test_data;
	uint8_t opcode = pdu[0];
	static uint8_t buf[65];

	switch (opcode) {
	case 0x01: /* Pairing Request */
		memcpy(data->preq, pdu, sizeof(data->preq));
		break;
	case 0x02: /* Pairing Response */
		memcpy(data->prsp, pdu, sizeof(data->prsp));
		break;
	case 0x03: /* Pairing Confirm */
		buf[0] = pdu[0];
		if (smp->sc)
			bt_crypto_f4(data->crypto, data->local_pk,
					data->remote_pk, data->prnd, 0,
					&buf[1]);
		else
			bt_crypto_c1(data->crypto, data->tk, data->prnd,
					data->prsp, data->preq, data->ia_type,
					data->ia, data->ra_type, data->ra,
					&buf[1]);
		return buf;
	case 0x04: /* Pairing Random */
		buf[0] = pdu[0];
		memcpy(&buf[1], data->prnd, 16);
		return buf;
	case 0x0c: /* Public Key */
		buf[0] = pdu[0];
		memcpy(&buf[1], data->local_pk, 64);
		return buf;
	default:
		break;
	}

	return pdu;
}

static bool verify_random(const uint8_t rnd[16])
{
	struct test_data *data = tester_get_data();
	uint8_t confirm[16];

	if (!bt_crypto_c1(data->crypto, data->tk, data->rrnd, data->prsp,
					data->preq, data->ia_type, data->ia,
					data->ra_type, data->ra, confirm))
		return false;

	if (memcmp(data->pcnf, confirm, sizeof(data->pcnf) != 0)) {
		tester_warn("Confirmation values don't match");
		return false;
	}

	if (data->out) {
		struct bthost *bthost = hciemu_client_get_host(data->hciemu);
		bt_crypto_s1(data->crypto, data->tk, data->rrnd, data->prnd,
								data->ltk);
		bthost_le_start_encrypt(bthost, data->handle, data->ltk);
	} else {
		bt_crypto_s1(data->crypto, data->tk, data->prnd, data->rrnd,
								data->ltk);
	}

	return true;
}

static bool sc_random(struct test_data *test_data)
{
	return true;
}

static void smp_server(const void *data, uint16_t len, void *user_data)
{
	struct test_data *test_data = user_data;
	struct bthost *bthost = hciemu_client_get_host(test_data->hciemu);
	const struct smp_data *smp = test_data->test_data;
	const struct smp_req_rsp *req;
	uint8_t opcode;
	const void *pdu;

	if (len < 1) {
		tester_warn("Received too small SMP PDU");
		goto failed;
	}

	opcode = *((const uint8_t *) data);

	tester_print("Received SMP opcode 0x%02x", opcode);

	if (test_data->counter >= smp->req_count) {
		test_condition_complete(test_data);
		return;
	}

	req = &smp->req[test_data->counter++];
	if (!req->expect)
		goto next;

	if (req->expect_len != len) {
		tester_warn("Unexpected SMP PDU length (%u != %u)",
							len, req->expect_len);
		goto failed;
	}

	switch (opcode) {
	case 0x01: /* Pairing Request */
		memcpy(test_data->preq, data, sizeof(test_data->preq));
		break;
	case 0x02: /* Pairing Response */
		memcpy(test_data->prsp, data, sizeof(test_data->prsp));
		break;
	case 0x03: /* Pairing Confirm */
		memcpy(test_data->pcnf, data + 1, 16);
		goto next;
	case 0x04: /* Pairing Random */
		memcpy(test_data->rrnd, data + 1, 16);
		if (smp->sc) {
			if (!sc_random(test_data))
				goto failed;
		} else {
			if (!verify_random(data + 1))
				goto failed;
		}
		goto next;
	case 0x0c: /* Public Key */
		memcpy(test_data->remote_pk, data + 1, 64);
		ecdh_shared_secret(test_data->remote_pk, test_data->local_sk,
							test_data->dhkey);
		goto next;
	default:
		break;
	}

	if (memcmp(req->expect, data, len) != 0) {
		tester_warn("Unexpected SMP PDU");
		goto failed;
	}

next:
	while (true) {
		if (smp->req_count == test_data->counter) {
			test_condition_complete(test_data);
			break;
		}

		req = &smp->req[test_data->counter];

		pdu = get_pdu(req->send);
		bthost_send_cid(bthost, test_data->handle, SMP_CID, pdu,
								req->send_len);
		if (req->expect)
			break;
		else
			test_data->counter++;
	}

	return;

failed:
	tester_test_failed();
}

static void command_hci_callback(uint16_t opcode, const void *param,
					uint8_t length, void *user_data)
{
	struct test_data *data = user_data;
	const struct smp_data *smp = data->test_data;
	const void *expect_hci_param = smp->expect_hci_param;
	uint8_t expect_hci_len = smp->expect_hci_len;

	tester_print("HCI Command 0x%04x length %u", opcode, length);

	if (opcode != smp->expect_hci_command)
		return;

	if (smp->expect_hci_func)
		expect_hci_param = smp->expect_hci_func(&expect_hci_len);

	if (length != expect_hci_len) {
		tester_warn("Invalid parameter size for HCI command");
		tester_test_failed();
		return;
	}

	if (memcmp(param, expect_hci_param, length) != 0) {
		tester_warn("Unexpected HCI command parameter value");
		tester_test_failed();
		return;
	}

	test_condition_complete(data);
}

static void smp_new_conn(uint16_t handle, void *user_data)
{
	struct test_data *data = user_data;
	const struct smp_data *smp = data->test_data;
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);
	const struct smp_req_rsp *req;
	const void *pdu;

	tester_print("New SMP client connection with handle 0x%04x", handle);

	data->handle = handle;

	bthost_add_cid_hook(bthost, handle, SMP_CID, smp_server, data);

	if (smp->req_count == data->counter)
		return;

	req = &smp->req[data->counter];

	if (!req->send)
		return;

	tester_print("Sending SMP PDU");

	pdu = get_pdu(req->send);
	bthost_send_cid(bthost, handle, SMP_CID, pdu, req->send_len);

	if (!req->expect)
		test_condition_complete(data);
}

static void init_bdaddr(struct test_data *data)
{
	const uint8_t *master_bdaddr, *client_bdaddr;

	master_bdaddr = hciemu_get_master_bdaddr(data->hciemu);
	if (!master_bdaddr) {
		tester_warn("No master bdaddr");
		tester_test_failed();
		return;
	}

	client_bdaddr = hciemu_get_client_bdaddr(data->hciemu);
	if (!client_bdaddr) {
		tester_warn("No client bdaddr");
		tester_test_failed();
		return;
	}

	data->ia_type = LE_PUBLIC_ADDRESS;
	data->ra_type = LE_PUBLIC_ADDRESS;

	if (data->out) {
		memcpy(data->ia, client_bdaddr, sizeof(data->ia));
		memcpy(data->ra, master_bdaddr, sizeof(data->ra));
	} else {
		memcpy(data->ia, master_bdaddr, sizeof(data->ia));
		memcpy(data->ra, client_bdaddr, sizeof(data->ra));
	}
}

static void test_client(const void *test_data)
{
	struct test_data *data = tester_get_data();
	const struct smp_data *smp = data->test_data;
	struct mgmt_cp_pair_device cp;
	struct bthost *bthost;

	init_bdaddr(data);

	bthost = hciemu_client_get_host(data->hciemu);
	bthost_set_connect_cb(bthost, smp_new_conn, data);
	test_add_condition(data);

	if (smp->expect_hci_command) {
		tester_print("Registering HCI command callback");
		hciemu_add_master_post_command_hook(data->hciemu,
						command_hci_callback, data);
		test_add_condition(data);
	}

	memcpy(&cp.addr.bdaddr, data->ra, sizeof(data->ra));
	cp.addr.type = BDADDR_LE_PUBLIC;
	if (smp->mitm)
		cp.io_cap = 0x04; /* KeyboardDisplay */
	else
		cp.io_cap = 0x03; /* NoInputNoOutput */

	mgmt_send(data->mgmt, MGMT_OP_PAIR_DEVICE, data->mgmt_index,
			sizeof(cp), &cp, pair_device_complete, NULL, NULL);

	tester_print("Pairing in progress");
}

static void setup_powered_server_callback(uint8_t status, uint16_t length,
					const void *param, void *user_data)
{
	if (status != MGMT_STATUS_SUCCESS) {
		tester_setup_failed();
		return;
	}

	tester_print("Controller powered on");

	tester_setup_complete();
}

static void setup_powered_server(const void *test_data)
{
	struct test_data *data = tester_get_data();
	const struct smp_data *smp = data->test_data;
	unsigned char param[] = { 0x01 };

	mgmt_register(data->mgmt, MGMT_EV_USER_CONFIRM_REQUEST,
			data->mgmt_index, user_confirm_request_callback,
			data, NULL);

	tester_print("Powering on controller");

	mgmt_send(data->mgmt, MGMT_OP_SET_LE, data->mgmt_index,
				sizeof(param), param, NULL, NULL, NULL);
	mgmt_send(data->mgmt, MGMT_OP_SET_BONDABLE, data->mgmt_index,
				sizeof(param), param, NULL, NULL, NULL);
	mgmt_send(data->mgmt, MGMT_OP_SET_CONNECTABLE, data->mgmt_index,
				sizeof(param), param, NULL, NULL, NULL);
	mgmt_send(data->mgmt, MGMT_OP_SET_ADVERTISING, data->mgmt_index,
				sizeof(param), param, NULL, NULL, NULL);
	if (smp->sc) {
		mgmt_send(data->mgmt, MGMT_OP_SET_SECURE_CONN,
				data->mgmt_index, sizeof(param), param, NULL,
				NULL, NULL);
		make_pk(data);
	}

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
			sizeof(param), param, setup_powered_server_callback,
			NULL, NULL);
}

static void test_server(const void *test_data)
{
	struct test_data *data = tester_get_data();
	const struct smp_data *smp = data->test_data;
	struct bthost *bthost;

	data->out = true;

	init_bdaddr(data);

	bthost = hciemu_client_get_host(data->hciemu);
	bthost_set_connect_cb(bthost, smp_new_conn, data);
	test_add_condition(data);

	bthost_hci_connect(bthost, data->ra, BDADDR_LE_PUBLIC);

	if (smp->expect_hci_command) {
		tester_print("Registering HCI command callback");
		hciemu_add_master_post_command_hook(data->hciemu,
						command_hci_callback, data);
		test_add_condition(data);
	}
}

int main(int argc, char *argv[])
{
	tester_init(&argc, &argv);

	test_smp("SMP Server - Basic Request 1",
					&smp_server_basic_req_1_test,
					setup_powered_server, test_server);
	test_smp("SMP Server - Invalid Request 1",
					&smp_server_nval_req_1_test,
					setup_powered_server, test_server);
	test_smp("SMP Server - Invalid Request 2",
					&smp_server_nval_req_2_test,
					setup_powered_server, test_server);
	test_smp("SMP Server - Invalid Request 3",
					&smp_server_nval_req_3_test,
					setup_powered_server, test_server);

	test_smp("SMP Client - Basic Request 1",
					&smp_client_basic_req_1_test,
					setup_powered_client, test_client);
	test_smp("SMP Client - Basic Request 2",
					&smp_client_basic_req_2_test,
					setup_powered_client, test_client);

	test_smp("SMP Client - SC Request 1",
					&smp_client_sc_req_1_test,
					setup_powered_client, test_client);
	test_smp("SMP Client - SC Request 2",
					&smp_client_sc_req_2_test,
					setup_powered_client, test_client);

	return tester_run();
}

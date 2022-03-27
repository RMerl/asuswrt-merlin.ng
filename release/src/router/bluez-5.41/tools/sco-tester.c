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

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sco.h"
#include "lib/mgmt.h"

#include "monitor/bt.h"
#include "emulator/bthost.h"
#include "emulator/hciemu.h"

#include "src/shared/tester.h"
#include "src/shared/mgmt.h"

struct test_data {
	const void *test_data;
	struct mgmt *mgmt;
	uint16_t mgmt_index;
	struct hciemu *hciemu;
	enum hciemu_type hciemu_type;
	unsigned int io_id;
	bool disable_esco;
};

struct sco_client_data {
	int expect_err;
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

	data->hciemu = hciemu_new(HCIEMU_TYPE_BREDRLE);
	if (!data->hciemu) {
		tester_warn("Failed to setup HCI emulation");
		tester_pre_setup_failed();
		return;
	}

	tester_print("New hciemu instance created");

	if (data->disable_esco) {
		uint8_t *features;

		tester_print("Disabling eSCO packet type support");

		features = hciemu_get_features(data->hciemu);
		if (features)
			features[3] &= ~0x80;
	}
}

static void test_pre_setup(const void *test_data)
{
	struct test_data *data = tester_get_data();

	data->mgmt = mgmt_new_default();
	if (!data->mgmt) {
		tester_warn("Failed to setup management interface");
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

	hciemu_unref(data->hciemu);
	data->hciemu = NULL;
}

static void test_data_free(void *test_data)
{
	struct test_data *data = test_data;

	if (data->io_id > 0)
		g_source_remove(data->io_id);

	free(data);
}

#define test_sco_full(name, data, setup, func, _disable_esco) \
	do { \
		struct test_data *user; \
		user = malloc(sizeof(struct test_data)); \
		if (!user) \
			break; \
		user->hciemu_type = HCIEMU_TYPE_BREDRLE; \
		user->io_id = 0; \
		user->test_data = data; \
		user->disable_esco = _disable_esco; \
		tester_add_full(name, data, \
				test_pre_setup, setup, func, NULL, \
				test_post_teardown, 2, user, test_data_free); \
	} while (0)

#define test_sco(name, data, setup, func) \
	test_sco_full(name, data, setup, func, false)

#define test_sco_11(name, data, setup, func) \
	test_sco_full(name, data, setup, func, true)

static const struct sco_client_data connect_success = {
	.expect_err = 0
};

static const struct sco_client_data connect_failure = {
	.expect_err = EOPNOTSUPP
};

static void client_connectable_complete(uint16_t opcode, uint8_t status,
					const void *param, uint8_t len,
					void *user_data)
{
	if (opcode != BT_HCI_CMD_WRITE_SCAN_ENABLE)
		return;

	tester_print("Client set connectable status 0x%02x", status);

	if (status)
		tester_setup_failed();
	else
		tester_setup_complete();
}

static void setup_powered_callback(uint8_t status, uint16_t length,
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
	bthost_write_scan_enable(bthost, 0x03);
}

static void setup_powered(const void *test_data)
{
	struct test_data *data = tester_get_data();
	unsigned char param[] = { 0x01 };

	tester_print("Powering on controller");

	mgmt_send(data->mgmt, MGMT_OP_SET_CONNECTABLE, data->mgmt_index,
					sizeof(param), param,
					NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_SET_SSP, data->mgmt_index,
				sizeof(param), param, NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_SET_LE, data->mgmt_index,
				sizeof(param), param, NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
					sizeof(param), param,
					setup_powered_callback, NULL, NULL);
}

static void test_framework(const void *test_data)
{
	tester_test_passed();
}

static void test_socket(const void *test_data)
{
	int sk;

	sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_SCO);
	if (sk < 0) {
		tester_warn("Can't create socket: %s (%d)", strerror(errno),
									errno);
		tester_test_failed();
		return;
	}

	close(sk);

	tester_test_passed();
}

static void test_getsockopt(const void *test_data)
{
	int sk, err;
	socklen_t len;
	struct bt_voice voice;

	sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_SCO);
	if (sk < 0) {
		tester_warn("Can't create socket: %s (%d)", strerror(errno),
									errno);
		tester_test_failed();
		return;
	}

	len = sizeof(voice);
	memset(&voice, 0, len);

	err = getsockopt(sk, SOL_BLUETOOTH, BT_VOICE, &voice, &len);
	if (err < 0) {
		tester_warn("Can't get socket option : %s (%d)",
							strerror(errno), errno);
		tester_test_failed();
		goto end;
	}

	if (voice.setting != BT_VOICE_CVSD_16BIT) {
		tester_warn("Invalid voice setting");
		tester_test_failed();
		goto end;
	}

	tester_test_passed();

end:
	close(sk);
}

static void test_setsockopt(const void *test_data)
{
	int sk, err;
	socklen_t len;
	struct bt_voice voice;

	sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_SCO);
	if (sk < 0) {
		tester_warn("Can't create socket: %s (%d)", strerror(errno),
									errno);
		tester_test_failed();
		goto end;
	}


	len = sizeof(voice);
	memset(&voice, 0, len);

	err = getsockopt(sk, SOL_BLUETOOTH, BT_VOICE, &voice, &len);
	if (err < 0) {
		tester_warn("Can't get socket option : %s (%d)",
							strerror(errno), errno);
		tester_test_failed();
		goto end;
	}

	if (voice.setting != BT_VOICE_CVSD_16BIT) {
		tester_warn("Invalid voice setting");
		tester_test_failed();
		goto end;
	}

	memset(&voice, 0, sizeof(voice));
	voice.setting = BT_VOICE_TRANSPARENT;

	err = setsockopt(sk, SOL_BLUETOOTH, BT_VOICE, &voice, sizeof(voice));
	if (err < 0) {
		tester_warn("Can't set socket option : %s (%d)",
							strerror(errno), errno);
		tester_test_failed();
		goto end;
	}

	len = sizeof(voice);
	memset(&voice, 0, len);

	err = getsockopt(sk, SOL_BLUETOOTH, BT_VOICE, &voice, &len);
	if (err < 0) {
		tester_warn("Can't get socket option : %s (%d)",
							strerror(errno), errno);
		tester_test_failed();
		goto end;
	}

	if (voice.setting != BT_VOICE_TRANSPARENT) {
		tester_warn("Invalid voice setting");
		tester_test_failed();
		goto end;
	}

	tester_test_passed();

end:
	close(sk);
}

static int create_sco_sock(struct test_data *data)
{
	const uint8_t *master_bdaddr;
	struct sockaddr_sco addr;
	int sk, err;

	sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET | SOCK_NONBLOCK,
								BTPROTO_SCO);
	if (sk < 0) {
		err = -errno;
		tester_warn("Can't create socket: %s (%d)", strerror(errno),
									errno);
		return err;
	}

	master_bdaddr = hciemu_get_master_bdaddr(data->hciemu);
	if (!master_bdaddr) {
		tester_warn("No master bdaddr");
		return -ENODEV;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sco_family = AF_BLUETOOTH;
	bacpy(&addr.sco_bdaddr, (void *) master_bdaddr);

	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		err = -errno;
		tester_warn("Can't bind socket: %s (%d)", strerror(errno),
									errno);
		close(sk);
		return err;
	}

	return sk;
}

static int connect_sco_sock(struct test_data *data, int sk)
{
	const uint8_t *client_bdaddr;
	struct sockaddr_sco addr;
	int err;

	client_bdaddr = hciemu_get_client_bdaddr(data->hciemu);
	if (!client_bdaddr) {
		tester_warn("No client bdaddr");
		return -ENODEV;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sco_family = AF_BLUETOOTH;
	bacpy(&addr.sco_bdaddr, (void *) client_bdaddr);

	err = connect(sk, (struct sockaddr *) &addr, sizeof(addr));
	if (err < 0 && !(errno == EAGAIN || errno == EINPROGRESS)) {
		err = -errno;
		tester_warn("Can't connect socket: %s (%d)", strerror(errno),
									errno);
		return err;
	}

	return 0;
}

static gboolean sco_connect_cb(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{
	struct test_data *data = tester_get_data();
	const struct sco_client_data *scodata = data->test_data;
	int err, sk_err, sk;
	socklen_t len = sizeof(sk_err);

	data->io_id = 0;

	sk = g_io_channel_unix_get_fd(io);

	if (getsockopt(sk, SOL_SOCKET, SO_ERROR, &sk_err, &len) < 0)
		err = -errno;
	else
		err = -sk_err;

	if (err < 0)
		tester_warn("Connect failed: %s (%d)", strerror(-err), -err);
	else
		tester_print("Successfully connected");

	if (-err != scodata->expect_err)
		tester_test_failed();
	else
		tester_test_passed();

	return FALSE;
}

static void test_connect(const void *test_data)
{
	struct test_data *data = tester_get_data();
	GIOChannel *io;
	int sk;

	sk = create_sco_sock(data);
	if (sk < 0) {
		tester_test_failed();
		return;
	}

	if (connect_sco_sock(data, sk) < 0) {
		close(sk);
		tester_test_failed();
		return;
	}

	io = g_io_channel_unix_new(sk);
	g_io_channel_set_close_on_unref(io, TRUE);

	data->io_id = g_io_add_watch(io, G_IO_OUT, sco_connect_cb, NULL);

	g_io_channel_unref(io);

	tester_print("Connect in progress");
}

static void test_connect_transp(const void *test_data)
{
	struct test_data *data = tester_get_data();
	const struct sco_client_data *scodata = data->test_data;
	int sk, err;
	struct bt_voice voice;

	sk = create_sco_sock(data);
	if (sk < 0) {
		tester_test_failed();
		return;
	}

	memset(&voice, 0, sizeof(voice));
	voice.setting = BT_VOICE_TRANSPARENT;

	err = setsockopt(sk, SOL_BLUETOOTH, BT_VOICE, &voice, sizeof(voice));
	if (err < 0) {
		tester_warn("Can't set socket option : %s (%d)",
							strerror(errno), errno);
		tester_test_failed();
		goto end;
	}

	err = connect_sco_sock(data, sk);

	tester_warn("Connect returned %s (%d), expected %s (%d)",
			strerror(-err), -err,
			strerror(scodata->expect_err), scodata->expect_err);

	if (-err != scodata->expect_err)
		tester_test_failed();
	else
		tester_test_passed();

end:
	close(sk);
}

int main(int argc, char *argv[])
{
	tester_init(&argc, &argv);

	test_sco("Basic Framework - Success", NULL, setup_powered,
							test_framework);

	test_sco("Basic SCO Socket - Success", NULL, setup_powered,
							test_socket);

	test_sco("Basic SCO Get Socket Option - Success", NULL, setup_powered,
							test_getsockopt);

	test_sco("Basic SCO Set Socket Option - Success", NULL, setup_powered,
							test_setsockopt);

	test_sco("eSCO CVSD - Success", &connect_success, setup_powered,
							test_connect);

	test_sco("eSCO mSBC - Success", &connect_success, setup_powered,
							test_connect_transp);

	test_sco_11("SCO CVSD 1.1 - Success", &connect_success, setup_powered,
							test_connect);

	test_sco_11("SCO mSBC 1.1 - Failure", &connect_failure, setup_powered,
							test_connect_transp);

	return tester_run();
}

/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
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
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <stdbool.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/rfcomm.h"
#include "lib/mgmt.h"

#include "monitor/bt.h"
#include "emulator/bthost.h"
#include "emulator/hciemu.h"

#include "src/shared/tester.h"
#include "src/shared/mgmt.h"

struct test_data {
	struct mgmt *mgmt;
	uint16_t mgmt_index;
	struct hciemu *hciemu;
	enum hciemu_type hciemu_type;
	const void *test_data;
	unsigned int io_id;
	uint16_t conn_handle;
};

struct rfcomm_client_data {
	uint8_t server_channel;
	uint8_t client_channel;
	int expected_connect_err;
	const uint8_t *send_data;
	const uint8_t *read_data;
	uint16_t data_len;
};

struct rfcomm_server_data {
	uint8_t server_channel;
	uint8_t client_channel;
	bool expected_status;
	const uint8_t *send_data;
	const uint8_t *read_data;
	uint16_t data_len;
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

	if (data->io_id > 0) {
		g_source_remove(data->io_id);
		data->io_id = 0;
	}

	hciemu_unref(data->hciemu);
	data->hciemu = NULL;
}

static void test_data_free(void *test_data)
{
	struct test_data *data = test_data;

	free(data);
}

static void client_connectable_complete(uint16_t opcode, uint8_t status,
					const void *param, uint8_t len,
					void *user_data)
{
	switch (opcode) {
	case BT_HCI_CMD_WRITE_SCAN_ENABLE:
		break;
	default:
		return;
	}

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
	bthost_write_scan_enable(bthost, 0x03);
}

static void setup_powered_client(const void *test_data)
{
	struct test_data *data = tester_get_data();
	unsigned char param[] = { 0x01 };

	tester_print("Powering on controller");

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
			sizeof(param), param, setup_powered_client_callback,
			NULL, NULL);
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
	unsigned char param[] = { 0x01 };

	tester_print("Powering on controller");

	mgmt_send(data->mgmt, MGMT_OP_SET_CONNECTABLE, data->mgmt_index,
				sizeof(param), param,
				NULL, NULL, NULL);

	mgmt_send(data->mgmt, MGMT_OP_SET_POWERED, data->mgmt_index,
			sizeof(param), param, setup_powered_server_callback,
			NULL, NULL);
}

const struct rfcomm_client_data connect_success = {
	.server_channel = 0x0c,
	.client_channel = 0x0c
};

const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};

const struct rfcomm_client_data connect_send_success = {
	.server_channel = 0x0c,
	.client_channel = 0x0c,
	.data_len = sizeof(data),
	.send_data = data
};

const struct rfcomm_client_data connect_read_success = {
	.server_channel = 0x0c,
	.client_channel = 0x0c,
	.data_len = sizeof(data),
	.read_data = data
};

const struct rfcomm_client_data connect_nval = {
	.server_channel = 0x0c,
	.client_channel = 0x0e,
	.expected_connect_err = -ECONNREFUSED
};

const struct rfcomm_server_data listen_success = {
	.server_channel = 0x0c,
	.client_channel = 0x0c,
	.expected_status = true
};

const struct rfcomm_server_data listen_send_success = {
	.server_channel = 0x0c,
	.client_channel = 0x0c,
	.expected_status = true,
	.data_len = sizeof(data),
	.send_data = data
};

const struct rfcomm_server_data listen_read_success = {
	.server_channel = 0x0c,
	.client_channel = 0x0c,
	.expected_status = true,
	.data_len = sizeof(data),
	.read_data = data
};

const struct rfcomm_server_data listen_nval = {
	.server_channel = 0x0c,
	.client_channel = 0x0e,
	.expected_status = false
};

static void test_basic(const void *test_data)
{
	int sk;

	sk = socket(PF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	if (sk < 0) {
		tester_warn("Can't create socket: %s (%d)", strerror(errno),
									errno);
		tester_test_failed();
		return;
	}

	close(sk);

	tester_test_passed();
}

static int create_rfcomm_sock(bdaddr_t *address, uint8_t channel)
{
	int sk;
	struct sockaddr_rc addr;

	sk = socket(PF_BLUETOOTH, SOCK_STREAM | SOCK_NONBLOCK, BTPROTO_RFCOMM);

	memset(&addr, 0, sizeof(addr));
	addr.rc_family = AF_BLUETOOTH;
	addr.rc_channel = channel;
	bacpy(&addr.rc_bdaddr, address);

	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		close(sk);
		return -1;
	}

	return sk;
}

static int connect_rfcomm_sock(int sk, const bdaddr_t *bdaddr, uint8_t channel)
{
	struct sockaddr_rc addr;
	int err;

	memset(&addr, 0, sizeof(addr));
	addr.rc_family = AF_BLUETOOTH;
	bacpy(&addr.rc_bdaddr, bdaddr);
	addr.rc_channel = htobs(channel);

	err = connect(sk, (struct sockaddr *) &addr, sizeof(addr));
	if (err < 0 && !(errno == EAGAIN || errno == EINPROGRESS))
		return err;

	return 0;
}

static gboolean client_received_data(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{
	struct test_data *data = tester_get_data();
	const struct rfcomm_client_data *cli = data->test_data;
	int sk;
	ssize_t ret;
	char buf[248];

	sk = g_io_channel_unix_get_fd(io);

	ret = read(sk, buf, cli->data_len);
	if (cli->data_len != ret) {
		tester_test_failed();
		return false;
	}

	if (memcmp(cli->read_data, buf, cli->data_len))
		tester_test_failed();
	else
		tester_test_passed();

	return false;
}

static gboolean rc_connect_cb(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{
	struct test_data *data = tester_get_data();
	const struct rfcomm_client_data *cli = data->test_data;
	socklen_t len = sizeof(int);
	int sk, err, sk_err;

	tester_print("Connected");

	data->io_id = 0;

	sk = g_io_channel_unix_get_fd(io);

	if (getsockopt(sk, SOL_SOCKET, SO_ERROR, &sk_err, &len) < 0)
		err = -errno;
	else
		err = -sk_err;

	if (cli->expected_connect_err && err == cli->expected_connect_err) {
		tester_test_passed();
		return false;
	}

	if (cli->send_data) {
		ssize_t ret;

		tester_print("Writing %u bytes of data", cli->data_len);

		ret = write(sk, cli->send_data, cli->data_len);
		if (cli->data_len != ret) {
			tester_warn("Failed to write %u bytes: %s (%d)",
					cli->data_len, strerror(errno), errno);
			tester_test_failed();
		}

		return false;
	} else if (cli->read_data) {
		g_io_add_watch(io, G_IO_IN, client_received_data, NULL);
		bthost_send_rfcomm_data(hciemu_client_get_host(data->hciemu),
						data->conn_handle,
						cli->client_channel,
						cli->read_data, cli->data_len);
		return false;
	}

	if (err < 0)
		tester_test_failed();
	else
		tester_test_passed();

	return false;
}

static void client_hook_func(const void *data, uint16_t len,
							void *user_data)
{
	struct test_data *test_data = tester_get_data();
	const struct rfcomm_client_data *cli = test_data->test_data;
	ssize_t ret;

	tester_print("bthost received %u bytes of data", len);

	if (cli->data_len != len) {
		tester_test_failed();
		return;
	}

	ret = memcmp(cli->send_data, data, len);
	if (ret)
		tester_test_failed();
	else
		tester_test_passed();
}

static void server_hook_func(const void *data, uint16_t len,
							void *user_data)
{
	struct test_data *test_data = tester_get_data();
	const struct rfcomm_server_data *srv = test_data->test_data;
	ssize_t ret;

	if (srv->data_len != len) {
		tester_test_failed();
		return;
	}

	ret = memcmp(srv->send_data, data, len);
	if (ret)
		tester_test_failed();
	else
		tester_test_passed();
}

static void rfcomm_connect_cb(uint16_t handle, uint16_t cid,
						void *user_data, bool status)
{
	struct test_data *data = tester_get_data();
	const struct rfcomm_client_data *cli = data->test_data;
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);

	if (cli->send_data)
		bthost_add_rfcomm_chan_hook(bthost, handle,
						cli->client_channel,
						client_hook_func, NULL);
	else if (cli->read_data)
		data->conn_handle = handle;
}

static void test_connect(const void *test_data)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);
	const struct rfcomm_client_data *cli = data->test_data;
	const uint8_t *client_addr, *master_addr;
	GIOChannel *io;
	int sk;

	bthost_add_l2cap_server(bthost, 0x0003, NULL, NULL);
	bthost_add_rfcomm_server(bthost, cli->server_channel,
						rfcomm_connect_cb, NULL);

	master_addr = hciemu_get_master_bdaddr(data->hciemu);
	client_addr = hciemu_get_client_bdaddr(data->hciemu);

	sk = create_rfcomm_sock((bdaddr_t *) master_addr, 0);

	if (connect_rfcomm_sock(sk, (const bdaddr_t *) client_addr,
					cli->client_channel) < 0) {
		close(sk);
		tester_test_failed();
		return;
	}

	io = g_io_channel_unix_new(sk);
	g_io_channel_set_close_on_unref(io, TRUE);

	data->io_id = g_io_add_watch(io, G_IO_OUT, rc_connect_cb, NULL);

	g_io_channel_unref(io);

	tester_print("Connect in progress %d", sk);
}

static gboolean server_received_data(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{
	struct test_data *data = tester_get_data();
	const struct rfcomm_server_data *srv = data->test_data;
	char buf[1024];
	ssize_t ret;
	int sk;

	sk = g_io_channel_unix_get_fd(io);

	ret = read(sk, buf, srv->data_len);
	if (ret != srv->data_len) {
		tester_test_failed();
		return false;
	}

	if (memcmp(buf, srv->read_data, srv->data_len))
		tester_test_failed();
	else
		tester_test_passed();

	return false;
}

static gboolean rfcomm_listen_cb(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{
	struct test_data *data = tester_get_data();
	const struct rfcomm_server_data *srv = data->test_data;
	int sk, new_sk;

	data->io_id = 0;

	sk = g_io_channel_unix_get_fd(io);

	new_sk = accept(sk, NULL, NULL);
	if (new_sk < 0) {
		tester_test_failed();
		return false;
	}

	if (srv->send_data) {
		ssize_t ret;

		ret = write(new_sk, srv->send_data, srv->data_len);
		if (ret != srv->data_len)
			tester_test_failed();

		close(new_sk);
		return false;
	} else if (srv->read_data) {
		GIOChannel *new_io;

		new_io = g_io_channel_unix_new(new_sk);
		g_io_channel_set_close_on_unref(new_io, TRUE);

		data->io_id = g_io_add_watch(new_io, G_IO_IN,
						server_received_data, NULL);

		g_io_channel_unref(new_io);
		return false;
	}

	close(new_sk);

	tester_test_passed();

	return false;
}

static void connection_cb(uint16_t handle, uint16_t cid, void *user_data,
								bool status)
{
	struct test_data *data = tester_get_data();
	const struct rfcomm_server_data *srv = data->test_data;
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);

	if (srv->read_data) {
		data->conn_handle = handle;
		bthost_send_rfcomm_data(bthost, data->conn_handle,
						srv->client_channel,
						srv->read_data, srv->data_len);
		return;
	} else if (srv->data_len) {
		return;
	}

	if (srv->expected_status == status)
		tester_test_passed();
	else
		tester_test_failed();
}

static void client_new_conn(uint16_t handle, void *user_data)
{
	struct test_data *data = tester_get_data();
	const struct rfcomm_server_data *srv = data->test_data;
	struct bthost *bthost;

	bthost = hciemu_client_get_host(data->hciemu);
	bthost_add_rfcomm_chan_hook(bthost, handle, srv->client_channel,
						server_hook_func, NULL);
	bthost_connect_rfcomm(bthost, handle, srv->client_channel,
						connection_cb, NULL);
}

static void test_server(const void *test_data)
{
	struct test_data *data = tester_get_data();
	const struct rfcomm_server_data *srv = data->test_data;
	const uint8_t *master_addr;
	struct bthost *bthost;
	GIOChannel *io;
	int sk;

	master_addr = hciemu_get_master_bdaddr(data->hciemu);

	sk = create_rfcomm_sock((bdaddr_t *) master_addr, srv->server_channel);
	if (sk < 0) {
		tester_test_failed();
		return;
	}

	if (listen(sk, 5) < 0) {
		tester_warn("listening on socket failed: %s (%u)",
				strerror(errno), errno);
		tester_test_failed();
		close(sk);
		return;
	}

	io = g_io_channel_unix_new(sk);
	g_io_channel_set_close_on_unref(io, TRUE);

	data->io_id = g_io_add_watch(io, G_IO_IN, rfcomm_listen_cb, NULL);
	g_io_channel_unref(io);

	tester_print("Listening for connections");

	bthost = hciemu_client_get_host(data->hciemu);
	bthost_set_connect_cb(bthost, client_new_conn, data);

	bthost_hci_connect(bthost, master_addr, BDADDR_BREDR);
}

#define test_rfcomm(name, data, setup, func) \
	do { \
		struct test_data *user; \
		user = malloc(sizeof(struct test_data)); \
		if (!user) \
			break; \
		user->hciemu_type = HCIEMU_TYPE_BREDR; \
		user->test_data = data; \
		user->io_id = 0; \
		tester_add_full(name, data, \
				test_pre_setup, setup, func, NULL, \
				test_post_teardown, 2, user, test_data_free); \
	} while (0)

int main(int argc, char *argv[])
{
	tester_init(&argc, &argv);

	test_rfcomm("Basic RFCOMM Socket - Success", NULL,
					setup_powered_client, test_basic);
	test_rfcomm("Basic RFCOMM Socket Client - Success", &connect_success,
					setup_powered_client, test_connect);
	test_rfcomm("Basic RFCOMM Socket Client - Write Success",
				&connect_send_success, setup_powered_client,
				test_connect);
	test_rfcomm("Basic RFCOMM Socket Client - Read Success",
				&connect_read_success, setup_powered_client,
				test_connect);
	test_rfcomm("Basic RFCOMM Socket Client - Conn Refused",
			&connect_nval, setup_powered_client, test_connect);
	test_rfcomm("Basic RFCOMM Socket Server - Success", &listen_success,
					setup_powered_server, test_server);
	test_rfcomm("Basic RFCOMM Socket Server - Write Success",
				&listen_send_success, setup_powered_server,
				test_server);
	test_rfcomm("Basic RFCOMM Socket Server - Read Success",
				&listen_read_success, setup_powered_server,
				test_server);
	test_rfcomm("Basic RFCOMM Socket Server - Conn Refused", &listen_nval,
					setup_powered_server, test_server);

	return tester_run();
}

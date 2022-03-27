/*
 * Copyright (C) 2014 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

#include "emulator/bthost.h"
#include "src/shared/tester.h"
#include "src/shared/queue.h"
#include "tester-main.h"

static struct queue *list; /* List of socket test cases */

static bt_bdaddr_t bdaddr_dummy = {
	.address = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55}
};

static int got_fd_result = -1;

static struct bt_action_data btsock_param_socktype_0 = {
	.addr = &bdaddr_dummy,
	.sock_type = 0,
	.channel = 1,
	.service_uuid = NULL,
	.service_name = "Test service",
	.flags = 0,
	.fd = &got_fd_result,
};

static struct bt_action_data btsock_param_socktype_l2cap = {
	.addr = &bdaddr_dummy,
	.sock_type = BTSOCK_L2CAP,
	.channel = 1,
	.service_uuid = NULL,
	.service_name = "Test service",
	.flags = 0,
	.fd = &got_fd_result,
};

static struct bt_action_data btsock_param_channel_0 = {
	.addr = &bdaddr_dummy,
	.sock_type = BTSOCK_RFCOMM,
	.channel = 0,
	.service_uuid = NULL,
	.service_name = "Test service",
	.flags = 0,
	.fd = &got_fd_result,
};

static struct bt_action_data btsock_param = {
	.addr = &bdaddr_dummy,
	.sock_type = BTSOCK_RFCOMM,
	.channel = 1,
	.service_uuid = NULL,
	.service_name = "Test service",
	.flags = 0,
	.fd = &got_fd_result,
};

static struct bt_action_data btsock_param_inv_bdaddr = {
	.addr = NULL,
	.sock_type = BTSOCK_RFCOMM,
	.channel = 1,
	.service_uuid = NULL,
	.service_name = "Test service",
	.flags = 0,
	.fd = &got_fd_result,
};

static bt_bdaddr_t emu_remote_bdaddr_val = {
	.address = { 0x00, 0xaa, 0x01, 0x01, 0x00, 0x00 },
};
static bt_property_t prop_emu_remote_bdadr = {
	.type = BT_PROPERTY_BDADDR,
	.val = &emu_remote_bdaddr_val,
	.len = sizeof(emu_remote_bdaddr_val),
};
static bt_property_t prop_emu_remotes_default_set[] = {
	{ BT_PROPERTY_BDADDR, sizeof(emu_remote_bdaddr_val),
						&emu_remote_bdaddr_val },
};

static struct bt_action_data btsock_param_emu_bdaddr = {
	.addr = &emu_remote_bdaddr_val,
	.sock_type = BTSOCK_RFCOMM,
	.channel = 1,
	.service_uuid = NULL,
	.service_name = "Test service",
	.flags = 0,
	.fd = &got_fd_result,
};

static struct emu_set_l2cap_data l2cap_setup_data = {
	.psm = 0x0003,
	.func = NULL,
	.user_data = NULL,
};

static struct bt_action_data prop_emu_remote_bdaddr_req = {
	.addr = &emu_remote_bdaddr_val,
	.prop_type = BT_PROPERTY_BDADDR,
	.prop = &prop_emu_remote_bdadr,
};

static void socket_listen_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct bt_action_data *action_data = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	*action_data->fd = -1;

	step->action_status = data->if_sock->listen(action_data->sock_type,
						action_data->service_name,
						action_data->service_uuid,
						action_data->channel,
						action_data->fd,
						action_data->flags);

	schedule_action_verification(step);
}

static void socket_connect_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct bt_action_data *action_data = current_data_step->set_data;
	struct step *step;
	int status;

	*action_data->fd = -1;

	status = data->if_sock->connect(action_data->addr,
						action_data->sock_type,
						action_data->service_uuid,
						action_data->channel,
						action_data->fd,
						action_data->flags);

	tester_print("status %d sock_fd %d", status, *action_data->fd);

	if (!status)
		return;

	step = g_new0(struct step, 1);
	step->action_status = status;

	schedule_action_verification(step);
}

static gboolean socket_chan_cb(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{
	int sock_fd = g_io_channel_unix_get_fd(io);
	struct step *step = g_new0(struct step, 1);
	int channel, len;

	tester_print("%s", __func__);

	if (cond & G_IO_HUP) {
		tester_warn("Socket %d hang up", sock_fd);

		step->action_status = BT_STATUS_FAIL;
		goto done;
	}

	if (cond & (G_IO_ERR | G_IO_NVAL)) {
		tester_warn("Socket error: sock %d cond %d", sock_fd, cond);

		step->action_status = BT_STATUS_FAIL;
		goto done;
	}

	len = read(sock_fd, &channel, sizeof(channel));
	if (len != sizeof(channel)) {
		tester_warn("Socket read failed");

		step->action_status = BT_STATUS_FAIL;
		goto done;
	}

	tester_print("read correct channel: %d", channel);

	step->action_status = BT_STATUS_SUCCESS;

done:
	schedule_action_verification(step);
	return FALSE;
}

static void socket_read_fd_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct bt_action_data *action_data = current_data_step->set_data;
	GIOChannel *io;

	io = g_io_channel_unix_new(*action_data->fd);
	g_io_channel_set_close_on_unref(io, TRUE);

	g_io_add_watch(io, G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
							socket_chan_cb, NULL);

	g_io_channel_unref(io);
}

static void socket_verify_fd_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct bt_action_data *action_data = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	if (!*action_data->fd) {
		step->action_status = BT_STATUS_FAIL;
		goto done;
	}

	step->action_status = (fcntl(*action_data->fd, F_GETFD) < 0) ?
					BT_STATUS_FAIL : BT_STATUS_SUCCESS;

done:
	schedule_action_verification(step);
}

static void socket_verify_channel_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct bt_action_data *action_data = current_data_step->set_data;
	int channel, len;
	struct step *step = g_new0(struct step, 1);

	if (!*action_data->fd) {
		tester_warn("Ups no action_data->fd");

		step->action_status = BT_STATUS_FAIL;
		goto done;
	}

	len = read(*action_data->fd, &channel, sizeof(channel));
	if (len != sizeof(channel) || channel != action_data->channel) {
		tester_warn("Ups bad channel");

		step->action_status = BT_STATUS_FAIL;
		goto done;
	}

	step->action_status = BT_STATUS_SUCCESS;

done:
	schedule_action_verification(step);
}

static void socket_close_channel_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	struct bt_action_data *action_data = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	if (!*action_data->fd) {
		tester_warn("Ups no action_data->fd");

		step->action_status = BT_STATUS_FAIL;
		goto done;
	}

	close(*action_data->fd);
	*action_data->fd = -1;

	step->action_status = BT_STATUS_SUCCESS;

done:
	schedule_action_verification(step);
}

static struct test_case test_cases[] = {
	TEST_CASE_BREDRLE("Socket Init",
		ACTION_SUCCESS(dummy_action, NULL),
	),
	TEST_CASE_BREDRLE("Socket Listen - Invalid: sock_type 0",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION(BT_STATUS_PARM_INVALID, socket_listen_action,
						&btsock_param_socktype_0),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Socket Listen - Invalid: sock_type L2CAP",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION(BT_STATUS_UNSUPPORTED, socket_listen_action,
						&btsock_param_socktype_l2cap),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Socket Listen - Invalid: chan, uuid",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION(BT_STATUS_PARM_INVALID, socket_listen_action,
						&btsock_param_channel_0),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Socket Listen - Check returned fd valid",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(socket_listen_action, &btsock_param),
		ACTION_SUCCESS(socket_verify_fd_action, &btsock_param),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Socket Listen - Check returned channel",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(socket_listen_action, &btsock_param),
		ACTION_SUCCESS(socket_verify_fd_action, &btsock_param),
		ACTION_SUCCESS(socket_verify_channel_action, &btsock_param),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Socket Listen - Close and Listen again",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(socket_listen_action, &btsock_param),
		ACTION_SUCCESS(socket_verify_fd_action, &btsock_param),
		ACTION_SUCCESS(socket_verify_channel_action, &btsock_param),
		ACTION_SUCCESS(socket_close_channel_action, &btsock_param),
		ACTION_SUCCESS(socket_listen_action, &btsock_param),
		ACTION_SUCCESS(socket_verify_fd_action, &btsock_param),
		ACTION_SUCCESS(socket_verify_channel_action, &btsock_param),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Socket Listen - Invalid: double Listen",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(socket_listen_action, &btsock_param),
		ACTION_SUCCESS(socket_verify_fd_action, &btsock_param),
		ACTION_SUCCESS(socket_verify_channel_action, &btsock_param),
		ACTION(BT_STATUS_BUSY, socket_listen_action, &btsock_param),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Socket Connect - Invalid: sock_type 0",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION(BT_STATUS_PARM_INVALID, socket_connect_action,
						&btsock_param_socktype_0),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Socket Connect - Invalid: sock_type L2CAP",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION(BT_STATUS_UNSUPPORTED, socket_connect_action,
						&btsock_param_socktype_l2cap),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Socket Connect - Invalid: chan, uuid",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION(BT_STATUS_PARM_INVALID, socket_connect_action,
						&btsock_param_channel_0),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Socket Connect - Invalid: bdaddr",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION(BT_STATUS_PARM_INVALID, socket_connect_action,
						&btsock_param_inv_bdaddr),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Socket Connect - Check returned fd valid",
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(bt_create_bond_action,
						&prop_emu_remote_bdaddr_req),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDING,
						&prop_emu_remote_bdadr, 1),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_set, 1),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDED,
						&prop_emu_remote_bdadr, 1),
		CALLBACK_DEVICE_PROPS(NULL, 0),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &l2cap_setup_data),
		ACTION_SUCCESS(emu_add_rfcomm_server_action,
						&btsock_param_emu_bdaddr),
		ACTION_SUCCESS(socket_connect_action, &btsock_param_emu_bdaddr),
		ACTION_SUCCESS(socket_verify_fd_action,
						&btsock_param_emu_bdaddr),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("Socket Connect - Check returned chann",
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(bt_create_bond_action,
						&prop_emu_remote_bdaddr_req),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDING,
						&prop_emu_remote_bdadr, 1),
		CALLBACK_DEVICE_FOUND(prop_emu_remotes_default_set, 1),
		CALLBACK_BOND_STATE(BT_BOND_STATE_BONDED,
						&prop_emu_remote_bdadr, 1),
		CALLBACK_DEVICE_PROPS(NULL, 0),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &l2cap_setup_data),
		ACTION_SUCCESS(emu_add_rfcomm_server_action,
						&btsock_param_emu_bdaddr),
		ACTION_SUCCESS(socket_connect_action, &btsock_param_emu_bdaddr),
		ACTION_SUCCESS(socket_verify_fd_action,
						&btsock_param_emu_bdaddr),
		ACTION_SUCCESS(socket_verify_channel_action,
						&btsock_param_emu_bdaddr),
		ACTION_SUCCESS(socket_read_fd_action, &btsock_param_emu_bdaddr),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
};

struct queue *get_socket_tests(void)
{
	uint16_t i = 0;

	list = queue_new();

	for (; i < sizeof(test_cases) / sizeof(test_cases[0]); ++i)
		queue_push_tail(list, &test_cases[i]);

	return list;
}

void remove_socket_tests(void)
{
	queue_destroy(list, NULL);
}

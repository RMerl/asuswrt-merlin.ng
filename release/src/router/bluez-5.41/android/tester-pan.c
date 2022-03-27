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

#include <stdbool.h>

#include "emulator/bthost.h"
#include "lib/bluetooth.h"
#include "android/utils.h"
#include "src/shared/tester.h"
#include "src/shared/queue.h"
#include "tester-main.h"

static struct queue *list; /* List of pan test cases */

#define pan_conn_req_pdu 0x01, 0x01, 0x02, 0x11, 0x16, 0x11, 0x15
#define pan_conn_rsp_pdu 0x01, 0x02, 0x00, 0x00

static const struct pdu_set pdus[] = {
	{ raw_pdu(pan_conn_req_pdu), raw_pdu(pan_conn_rsp_pdu) },
	{ end_pdu, end_pdu },
};

static struct emu_l2cap_cid_data cid_data = {
	.pdu = pdus,
};

static struct emu_set_l2cap_data l2cap_setup_data = {
	.psm = 15,
	.func = tester_generic_connect_cb,
	.user_data = &cid_data,
};

static void pan_connect_action(void)
{
	struct test_data *data = tester_get_data();
	const uint8_t *pan_addr = hciemu_get_client_bdaddr(data->hciemu);
	struct step *step = g_new0(struct step, 1);
	bt_bdaddr_t bdaddr;

	bdaddr2android((const bdaddr_t *) pan_addr, &bdaddr);

	step->action_status = data->if_pan->connect(&bdaddr,
					BTPAN_ROLE_PANU, BTPAN_ROLE_PANNAP);

	schedule_action_verification(step);
}

static void pan_disconnect_action(void)
{
	struct test_data *data = tester_get_data();
	const uint8_t *pan_addr = hciemu_get_client_bdaddr(data->hciemu);
	struct step *step = g_new0(struct step, 1);
	bt_bdaddr_t bdaddr;

	bdaddr2android((const bdaddr_t *) pan_addr, &bdaddr);

	step->action_status = data->if_pan->disconnect(&bdaddr);

	schedule_action_verification(step);
}

static void pan_get_local_role_action(void)
{
	struct test_data *data = tester_get_data();
	const uint8_t *pan_addr = hciemu_get_client_bdaddr(data->hciemu);
	struct step *step = g_new0(struct step, 1);
	bt_bdaddr_t bdaddr;
	int role;

	bdaddr2android((const bdaddr_t *) pan_addr, &bdaddr);

	role = data->if_pan->get_local_role();
	if (role == BTPAN_ROLE_PANU)
		step->action_status = BT_STATUS_SUCCESS;
	else
		step->action_status = BT_STATUS_FAIL;

	schedule_action_verification(step);
}

static void pan_enable_nap_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_pan->enable(BTPAN_ROLE_PANNAP);

	schedule_action_verification(step);
}

static void pan_enable_panu_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_pan->enable(BTPAN_ROLE_PANU);

	schedule_action_verification(step);
}

static void pan_enable_none_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_pan->enable(BTPAN_ROLE_NONE);

	schedule_action_verification(step);
}

static struct test_case test_cases[] = {
	TEST_CASE_BREDRLE("PAN Init",
		ACTION_SUCCESS(dummy_action, NULL),
	),
	TEST_CASE_BREDRLE("PAN Connect - Success",
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &l2cap_setup_data),
		ACTION_SUCCESS(pan_connect_action, NULL),
		CALLBACK_PAN_CONN_STATE(CB_PAN_CONNECTION_STATE,
					BT_STATUS_SUCCESS,
					BTPAN_STATE_CONNECTING,
					BTPAN_ROLE_PANU, BTPAN_ROLE_PANNAP),
		CALLBACK_PAN_CTRL_STATE(CB_PAN_CONTROL_STATE, BT_STATUS_SUCCESS,
					BTPAN_STATE_ENABLED, BTPAN_ROLE_PANU),
		CALLBACK_PAN_CONN_STATE(CB_PAN_CONNECTION_STATE,
					BT_STATUS_SUCCESS,
					BTPAN_STATE_CONNECTED,
					BTPAN_ROLE_PANU, BTPAN_ROLE_PANNAP),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_PAN_CONN_STATE(CB_PAN_CONNECTION_STATE,
					BT_STATUS_SUCCESS,
					BTPAN_STATE_DISCONNECTED,
					BTPAN_ROLE_PANU, BTPAN_ROLE_PANNAP),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("PAN Disconnect - Success",
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &l2cap_setup_data),
		ACTION_SUCCESS(pan_connect_action, NULL),
		CALLBACK_PAN_CONN_STATE(CB_PAN_CONNECTION_STATE,
					BT_STATUS_SUCCESS,
					BTPAN_STATE_CONNECTING,
					BTPAN_ROLE_PANU, BTPAN_ROLE_PANNAP),
		CALLBACK_PAN_CTRL_STATE(CB_PAN_CONTROL_STATE, BT_STATUS_SUCCESS,
					BTPAN_STATE_ENABLED, BTPAN_ROLE_PANU),
		CALLBACK_PAN_CONN_STATE(CB_PAN_CONNECTION_STATE,
					BT_STATUS_SUCCESS,
					BTPAN_STATE_CONNECTED,
					BTPAN_ROLE_PANU, BTPAN_ROLE_PANNAP),
		ACTION_SUCCESS(pan_disconnect_action, NULL),
		CALLBACK_PAN_CONN_STATE(CB_PAN_CONNECTION_STATE,
					BT_STATUS_SUCCESS,
					BTPAN_STATE_DISCONNECTED,
					BTPAN_ROLE_PANU, BTPAN_ROLE_PANNAP),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("PAN GetLocalRole - Success",
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &l2cap_setup_data),
		ACTION_SUCCESS(pan_connect_action, NULL),
		CALLBACK_PAN_CONN_STATE(CB_PAN_CONNECTION_STATE,
					BT_STATUS_SUCCESS,
					BTPAN_STATE_CONNECTING,
					BTPAN_ROLE_PANU, BTPAN_ROLE_PANNAP),
		CALLBACK_PAN_CTRL_STATE(CB_PAN_CONTROL_STATE, BT_STATUS_SUCCESS,
					BTPAN_STATE_ENABLED, BTPAN_ROLE_PANU),
		CALLBACK_PAN_CONN_STATE(CB_PAN_CONNECTION_STATE,
					BT_STATUS_SUCCESS,
					BTPAN_STATE_CONNECTED,
					BTPAN_ROLE_PANU, BTPAN_ROLE_PANNAP),
		ACTION_SUCCESS(pan_get_local_role_action, NULL),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_PAN_CONN_STATE(CB_PAN_CONNECTION_STATE,
					BT_STATUS_SUCCESS,
					BTPAN_STATE_DISCONNECTED,
					BTPAN_ROLE_PANU, BTPAN_ROLE_PANNAP),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("PAN Enable NAP - Success",
		ACTION_SUCCESS(pan_enable_nap_action, NULL),
		CALLBACK_PAN_CTRL_STATE(CB_PAN_CONTROL_STATE, BT_STATUS_SUCCESS,
					BTPAN_STATE_ENABLED, BTPAN_ROLE_PANNAP),
	),
	TEST_CASE_BREDRLE("PAN Enable PANU - Success",
		ACTION(BT_STATUS_UNSUPPORTED, pan_enable_panu_action, NULL),
	),
	TEST_CASE_BREDRLE("PAN Enable NONE - Success",
		ACTION_SUCCESS(pan_enable_nap_action, NULL),
		CALLBACK_PAN_CTRL_STATE(CB_PAN_CONTROL_STATE, BT_STATUS_SUCCESS,
					BTPAN_STATE_ENABLED, BTPAN_ROLE_PANNAP),
		ACTION_SUCCESS(pan_enable_none_action, NULL),
		CALLBACK_PAN_CTRL_STATE(CB_PAN_CONTROL_STATE, BT_STATUS_SUCCESS,
					BTPAN_STATE_DISABLED, BTPAN_ROLE_NONE),
	),
};

struct queue *get_pan_tests(void)
{
	uint16_t i = 0;

	list = queue_new();

	for (; i < sizeof(test_cases) / sizeof(test_cases[0]); ++i)
		queue_push_tail(list, &test_cases[i]);

	return list;
}

void remove_pan_tests(void)
{
	queue_destroy(list, NULL);
}

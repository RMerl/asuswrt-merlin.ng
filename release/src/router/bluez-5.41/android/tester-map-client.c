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
#include "src/shared/tester.h"
#include "src/shared/queue.h"
#include "tester-main.h"

static struct queue *list = NULL; /* List of map client test cases */

#define INST0_ID 0
#define INST1_ID 1

#define sdp_rsp_pdu	0x07, \
			0x00, 0x00, \
			0x00, 0xb5, \
			0x00, 0xb2, \
			0x35, 0xb0, 0x36, 0x00, 0x56, 0x09, 0x00, 0x00, 0x0a, \
			0x00, 0x01, 0x00, 0x09, 0x09, 0x00, 0x01, 0x35, 0x03, \
			0x19, 0x11, 0x32, 0x09, 0x00, 0x04, 0x35, 0x11, 0x35, \
			0x03, 0x19, 0x01, 0x00, 0x35, 0x05, 0x19, 0x00, 0x03, \
			0x08, 0x04, 0x35, 0x03, 0x19, 0x00, 0x08, 0x09, 0x00, \
			0x05, 0x35, 0x03, 0x19, 0x10, 0x02, 0x09, 0x00, 0x09, \
			0x35, 0x08, 0x35, 0x06, 0x19, 0x11, 0x34, 0x09, 0x01, \
			0x01, 0x09, 0x01, 0x00, 0x25, 0x0c, 0x4d, 0x41, 0x50, \
			0x20, 0x53, 0x4d, 0x53, 0x2f, 0x4d, 0x4d, 0x53, 0x00, \
			0x09, 0x03, 0x15, 0x08, 0x00, 0x09, 0x03, 0x16, 0x08, \
			0x0e, 0x36, 0x00, 0x54, 0x09, 0x00, 0x00, 0x0a, 0x00, \
			0x01, 0x00, 0x0a, 0x09, 0x00, 0x01, 0x35, 0x03, 0x19, \
			0x11, 0x32, 0x09, 0x00, 0x04, 0x35, 0x11, 0x35, 0x03, \
			0x19, 0x01, 0x00, 0x35, 0x05, 0x19, 0x00, 0x03, 0x08, \
			0x05, 0x35, 0x03, 0x19, 0x00, 0x08, 0x09, 0x00, 0x05, \
			0x35, 0x03, 0x19, 0x10, 0x02, 0x09, 0x00, 0x09, 0x35, \
			0x08, 0x35, 0x06, 0x19, 0x11, 0x34, 0x09, 0x01, 0x01, \
			0x09, 0x01, 0x00, 0x25, 0x0a, 0x4d, 0x41, 0x50, 0x20, \
			0x45, 0x4d, 0x41, 0x49, 0x4c, 0x00, 0x09, 0x03, 0x15, \
			0x08, 0x01, 0x09, 0x03, 0x16, 0x08, 0x01, \
			0x00

static const struct pdu_set pdus[] = {
	{ end_pdu, raw_pdu(sdp_rsp_pdu) },
	{ end_pdu, end_pdu },
};

static struct emu_l2cap_cid_data cid_data = {
	.pdu = pdus,
};

static bt_bdaddr_t emu_remote_bdaddr_val = {
	.address = { 0x00, 0xaa, 0x01, 0x01, 0x00, 0x00 },
};

static struct emu_set_l2cap_data l2cap_sdp_setup_data = {
	.psm = 1,
	.func = tester_generic_connect_cb,
	.user_data = &cid_data,
};

/* TODO define all parameters according to specification document */
static btmce_mas_instance_t remote_map_inst_sms_mms_email_val[] = {
	{ INST0_ID, 4, 14, "MAP SMS/MMS" },
	{ INST1_ID, 5, 1, "MAP EMAIL" },
};

static void map_client_cid_hook_cb(const void *data, uint16_t len,
								void *user_data)
{
	/* TODO extend if needed */
}

static void map_client_conn_cb(uint16_t handle, void *user_data)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);

	tester_print("New connection with handle 0x%04x", handle);

	if (data->hciemu_type == HCIEMU_TYPE_BREDR) {
		tester_warn("Not handled device type.");
		return;
	}

	cid_data.cid = 0x0040;
	cid_data.handle = handle;

	bthost_add_cid_hook(bthost, handle, cid_data.cid,
					map_client_cid_hook_cb, &cid_data);
}

static void map_client_get_instances_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *current_data_step = queue_peek_head(data->steps);
	bt_bdaddr_t *bd_addr = current_data_step->set_data;
	struct step *step = g_new0(struct step, 1);

	step->action_status =
		data->if_map_client->get_remote_mas_instances(bd_addr);

	schedule_action_verification(step);
}

static struct test_case test_cases[] = {
	TEST_CASE_BREDRLE("MAP Client Init", ACTION_SUCCESS(dummy_action, NULL),
	),
	TEST_CASE_BREDRLE("MAP Client - Get mas instances success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_sdp_setup_data),
		ACTION_SUCCESS(emu_set_connect_cb_action, map_client_conn_cb),
		ACTION_SUCCESS(map_client_get_instances_action,
							&emu_remote_bdaddr_val),
		CALLBACK_MAP_CLIENT_REMOTE_MAS_INSTANCE(BT_STATUS_SUCCESS, NULL,
					2, remote_map_inst_sms_mms_email_val),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
};

struct queue *get_map_client_tests(void)
{
	uint16_t i = 0;

	list = queue_new();

	for (; i < sizeof(test_cases) / sizeof(test_cases[0]); ++i)
		queue_push_tail(list, &test_cases[i]);

	return list;
}

void remove_map_client_tests(void)
{
	queue_destroy(list, NULL);
}

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
#include "src/shared/util.h"
#include "src/shared/tester.h"
#include "src/shared/queue.h"
#include "lib/bluetooth.h"
#include "android/utils.h"
#include "tester-main.h"

static struct queue *list;

#define req_dsc 0x00, 0x01
#define rsp_dsc 0x02, 0x01, 0x04, 0x08
#define req_get 0x10, 0x02, 0x04
#define rsp_get 0x12, 0x02, 0x01, 0x00, 0x07, 0x06, 0x00, \
						0x00, 0xff, 0xff, 0x02, 0x40
#define req_cfg 0x20, 0x03, 0x04, 0x04, 0x01, 0x00, 0x07, \
					0x06, 0x00, 0x00, 0x21, 0x15, 0x02, \
					0x40
#define rsp_cfg 0x22, 0x03
#define req_open 0x30, 0x06, 0x04
#define rsp_open 0x32, 0x06
#define req_close 0x40, 0x08, 0x04
#define rsp_close 0x42, 0x08
#define req_start 0x40, 0x07, 0x04
#define rsp_start 0x42, 0x07
#define req_suspend 0x50, 0x09, 0x04
#define rsp_suspend 0x52, 0x09

static const struct pdu_set pdus[] = {
	{ raw_pdu(req_dsc), raw_pdu(rsp_dsc) },
	{ raw_pdu(req_get), raw_pdu(rsp_get) },
	{ raw_pdu(req_cfg), raw_pdu(rsp_cfg) },
	{ raw_pdu(req_open), raw_pdu(rsp_open) },
	{ raw_pdu(req_close), raw_pdu(rsp_close) },
	{ raw_pdu(req_start), raw_pdu(rsp_start) },
	{ raw_pdu(req_suspend), raw_pdu(rsp_suspend) },
	{ end_pdu, end_pdu },
};

static struct emu_l2cap_cid_data cid_data = {
	.pdu = pdus,
};

static void a2dp_connect_request_cb(uint16_t handle, uint16_t cid,
							void *user_data)
{
	struct emu_l2cap_cid_data *cid_data = user_data;

	if (cid_data->handle)
		return;

	cid_data->handle = handle;
	cid_data->cid = cid;

	tester_handle_l2cap_data_exchange(cid_data);
}

static struct emu_set_l2cap_data l2cap_setup_data = {
	.psm = 25,
	.func = a2dp_connect_request_cb,
	.user_data = &cid_data,
};

static void a2dp_connect_action(void)
{
	struct test_data *data = tester_get_data();
	const uint8_t *addr = hciemu_get_client_bdaddr(data->hciemu);
	struct step *step = g_new0(struct step, 1);
	bt_bdaddr_t bdaddr;

	cid_data.handle = 0;
	cid_data.cid = 0;

	bdaddr2android((const bdaddr_t *) addr, &bdaddr);

	step->action_status = data->if_a2dp->connect(&bdaddr);

	schedule_action_verification(step);
}

static void a2dp_disconnect_action(void)
{
	struct test_data *data = tester_get_data();
	const uint8_t *addr = hciemu_get_client_bdaddr(data->hciemu);
	struct step *step = g_new0(struct step, 1);
	bt_bdaddr_t bdaddr;

	bdaddr2android((const bdaddr_t *) addr, &bdaddr);

	step->action_status = data->if_a2dp->disconnect(&bdaddr);

	schedule_action_verification(step);
}

static void audio_resume_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);
	int err;

	err = data->audio->open_output_stream(data->audio,
						0,
						AUDIO_DEVICE_OUT_ALL_A2DP,
						AUDIO_OUTPUT_FLAG_NONE,
						NULL,
						&data->if_stream, NULL);
	if (err < 0) {
		step->action_status = BT_STATUS_FAIL;
		goto done;
	}

	/* Write something to force resume */
	data->if_stream->write(data->if_stream, &err, sizeof(err));

done:
	schedule_action_verification(step);
}

static void audio_suspend_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);

	data->if_stream->common.standby(&data->if_stream->common);

	schedule_action_verification(step);
}

static struct test_case test_cases[] = {
	TEST_CASE_BREDRLE("A2DP Init",
		ACTION_SUCCESS(dummy_action, NULL),
	),
	TEST_CASE_BREDRLE("A2DP Connect - Success",
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &l2cap_setup_data),
		ACTION_SUCCESS(a2dp_connect_action, NULL),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTING),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTED),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_DISCONNECTED),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("A2DP Disconnect - Success",
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &l2cap_setup_data),
		ACTION_SUCCESS(a2dp_connect_action, NULL),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTING),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTED),
		ACTION_SUCCESS(a2dp_disconnect_action, NULL),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_DISCONNECTING),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_DISCONNECTED),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("A2DP Resume - Success",
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &l2cap_setup_data),
		ACTION_SUCCESS(a2dp_connect_action, NULL),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTING),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTED),
		ACTION_SUCCESS(audio_resume_action, NULL),
		CALLBACK_AV_AUDIO_STATE(CB_A2DP_AUDIO_STATE,
					BTAV_AUDIO_STATE_STARTED),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_DISCONNECTED),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("A2DP Suspend - Success",
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &l2cap_setup_data),
		ACTION_SUCCESS(a2dp_connect_action, NULL),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTING),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTED),
		ACTION_SUCCESS(audio_resume_action, NULL),
		CALLBACK_AV_AUDIO_STATE(CB_A2DP_AUDIO_STATE,
					BTAV_AUDIO_STATE_STARTED),
		ACTION_SUCCESS(audio_suspend_action, NULL),
		CALLBACK_AV_AUDIO_STATE(CB_A2DP_AUDIO_STATE,
					BTAV_AUDIO_STATE_STOPPED),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_DISCONNECTED),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
};

struct queue *get_a2dp_tests(void)
{
	uint16_t i = 0;

	list = queue_new();

	for (; i < sizeof(test_cases) / sizeof(test_cases[0]); ++i)
		queue_push_tail(list, &test_cases[i]);

	return list;
}

void remove_a2dp_tests(void)
{
	queue_destroy(list, NULL);
}

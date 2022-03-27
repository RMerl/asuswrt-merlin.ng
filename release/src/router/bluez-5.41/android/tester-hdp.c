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

#include <stdlib.h>
#include <stdbool.h>

#include "emulator/bthost.h"
#include "lib/bluetooth.h"
#include "android/utils.h"
#include "src/shared/tester.h"
#include "src/shared/queue.h"
#include "tester-main.h"

typedef enum {
	HDP_APP_SINK_RELIABLE,
	HDP_APP_SINK_STREAM,
	HDP_APP_SOURCE_RELIABLE,
	HDP_APP_SOURCE_STREAM,
} hdp_app_reg_type;

#define hdp_rsp_pdu	0x07, \
			0x00, 0x00, \
			0x01, 0xc8, \
			0x01, 0xc5, \
			0x36, 0x01, 0xc2, 0x36, 0x01, 0xbf, 0x09, 0x00, 0x00, \
			0x0a, 0x00, 0x01, 0x00, 0x00, 0x09, 0x00, 0x01, 0x35, \
			0x03, 0x19, 0x14, 0x01, 0x09, 0x00, 0x04, 0x35, 0x10, \
			0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x10, 0x01, 0x35, \
			0x06, 0x19, 0x00, 0x1e, 0x09, 0x01, 0x00, 0x09, 0x00, \
			0x09, 0x35, 0x08, 0x35, 0x06, 0x19, 0x14, 0x00, 0x09, \
			0x01, 0x01, 0x09, 0x00, 0x0d, 0x35, 0x0f, 0x35, 0x0d, \
			0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x10, 0x03, 0x35, \
			0x03, 0x19, 0x00, 0x1f, 0x09, 0x01, 0x00, 0x25, 0x03, \
			0x48, 0x44, 0x50, 0x09, 0x01, 0x01, 0x25, 0x28, 0x43, \
			0x6f, 0x6c, 0x6c, 0x65, 0x63, 0x74, 0x2c, 0x20, 0x64, \
			0x69, 0x73, 0x70, 0x6c, 0x61, 0x79, 0x2c, 0x20, 0x61, \
			0x6e, 0x64, 0x20, 0x72, 0x65, 0x8b, 0x6c, 0x61, 0x79, \
			0x20, 0x68, 0x65, 0x61, 0x6c, 0x74, 0x68, 0x20, 0x64, \
			0x61, 0x74, 0x61, 0x09, 0x01, 0x02, 0x25, 0x0d, 0x42, \
			0x4c, 0x55, 0x45, 0x54, 0x4f, 0x4f, 0x54, 0x48, 0x20, \
			0x53, 0x49, 0x47, 0x09, 0x02, 0x00, 0x36, 0x01, 0x22, \
			0x35, 0x18, 0x08, 0x01, 0x09, 0x10, 0x04, 0x08, 0x00, \
			0x25, 0x0f, 0x50, 0x75, 0x6c, 0x73, 0x65, 0x20, 0x4f, \
			0x78, 0x69, 0x6d, 0x65, 0x74, 0x65, 0x72, 0x0d, 0x35, \
			0x20, 0x08, 0x02, 0x09, 0x10, 0x07, 0x08, 0x00, 0x25, \
			0x17, 0x42, 0x6c, 0x6f, 0x6f, 0x64, 0x20, 0x50, 0x72, \
			0x65, 0x73, 0x73, 0x75, 0x72, 0x65, 0x20, 0x4d, 0x6f, \
			0x6e, 0x69, 0x74, 0x6f, 0x72, 0x0d, 0x35, 0x1a, 0x08, \
			0x03, 0x09, 0x10, 0x08, 0x08, 0x00, 0x25, 0x11, 0x42, \
			0x6f, 0x64, 0x79, 0x20, 0x54, 0x68, 0x65, 0x72, 0x6d, \
			0x6f, 0x6d, 0x65, 0x74, 0x65, 0x72, 0x0d, 0x35, 0x1e, \
			0x08, 0x04, 0x09, 0x10, 0x0f, 0x08, 0x00, 0x25, 0x15, \
			0x42, 0x6f, 0x64, 0x79, 0x20, 0x57, 0x65, 0x69, 0x67, \
			0x68, 0x74, 0x20, 0x53, 0x63, 0x61, 0x6c, 0x65, 0x09, \
			0x09, 0x09, 0x0d, 0x35, 0x17, 0x08, 0x05, 0x09, 0x10, \
			0x11, 0x08, 0x00, 0x25, 0x0e, 0x47, 0x6c, 0x75, 0x63, \
			0x6f, 0x73, 0x65, 0x20, 0x4d, 0x65, 0x74, 0x65, 0x72, \
			0x0d, 0x35, 0x18, 0x08, 0x06, 0x09, 0x10, 0x04, 0x08, \
			0x01, 0x25, 0x0f, 0x50, 0x75, 0x6c, 0x73, 0x65, 0x20, \
			0x4f, 0x78, 0x69, 0x6d, 0x65, 0x74, 0x65, 0x72, 0x0d, \
			0x35, 0x20, 0x08, 0x07, 0x09, 0x10, 0x07, 0x08, 0x01, \
			0x25, 0x17, 0x42, 0x6c, 0x6f, 0x6f, 0x64, 0x20, 0x50, \
			0x72, 0x65, 0x73, 0x73, 0x75, 0x72, 0x65, 0x20, 0x4d, \
			0x6f, 0x6e, 0x69, 0x74, 0x6f, 0x72, 0x0d, 0x35, 0x1a, \
			0x08, 0x08, 0x09, 0x10, 0x08, 0x08, 0x01, 0x25, 0x11, \
			0x42, 0x6f, 0x64, 0x79, 0x20, 0x54, 0x68, 0x65, 0x72, \
			0x6d, 0x6f, 0x6d, 0x65, 0x74, 0x65, 0x72, 0x0d, 0x35, \
			0x1e, 0x08, 0x09, 0x09, 0x10, 0x0f, 0x08, 0x01, 0x25, \
			0x15, 0x42, 0x6f, 0x64, 0x79, 0x20, 0x57, 0x65, 0x69, \
			0x67, 0x68, 0x74, 0x20, 0x53, 0x63, 0x61, 0x6c, 0x65, \
			0x09, 0x09, 0x09, 0x0d, 0x35, 0x17, 0x08, 0x0a, 0x09, \
			0x10, 0x11, 0x08, 0x01, 0x25, 0x0e, 0x47, 0x6c, 0x75, \
			0x63, 0x6f, 0x73, 0x65, 0x20, 0x4d, 0x65, 0x74, 0x65, \
			0x72, 0x0d, 0x09, 0x03, 0x01, 0x08, 0x01, 0x09, 0x03, \
			0x02, 0x08, 0x00, \
			0x00

static const struct pdu_set sdp_pdus[] = {
	{ end_pdu, raw_pdu(hdp_rsp_pdu) },
	{ end_pdu, end_pdu },
};

static struct emu_l2cap_cid_data sdp_cid_data = {
	.pdu = sdp_pdus,
	.is_sdp = TRUE,
};

static struct emu_l2cap_cid_data ctrl_cid_data;
static struct emu_l2cap_cid_data data_cid_data;

static struct queue *list; /* List of hdp test cases */

static bthl_reg_param_t *create_app(hdp_app_reg_type type)
{
	bthl_reg_param_t *reg;
	bthl_mdep_cfg_t mdep1, mdep2;

	reg = malloc(sizeof(bthl_reg_param_t));
	reg->application_name = "bluez-android";
	reg->provider_name = "Bluez";
	reg->srv_name = "bluez-hdp";
	reg->srv_desp = "health-device-profile";

	mdep1.data_type = 4100;
	mdep1.mdep_description = "pulse-oximeter";

	mdep2.data_type = 4100;
	mdep2.mdep_description = "pulse-oximeter";

	switch (type) {
	case HDP_APP_SINK_RELIABLE:
		reg->number_of_mdeps = 1;
		mdep1.mdep_role = BTHL_MDEP_ROLE_SINK;
		mdep1.channel_type = BTHL_CHANNEL_TYPE_RELIABLE;
		reg->mdep_cfg = malloc(reg->number_of_mdeps *
						sizeof(bthl_mdep_cfg_t));
		reg->mdep_cfg[0] = mdep1;
		break;

	case HDP_APP_SINK_STREAM:
		reg->number_of_mdeps = 2;

		mdep1.mdep_role = BTHL_MDEP_ROLE_SINK;
		mdep1.channel_type = BTHL_CHANNEL_TYPE_RELIABLE;

		mdep2.mdep_role = BTHL_MDEP_ROLE_SINK;
		mdep2.channel_type = BTHL_CHANNEL_TYPE_STREAMING;

		reg->mdep_cfg = malloc(reg->number_of_mdeps *
						sizeof(bthl_mdep_cfg_t));
		reg->mdep_cfg[0] = mdep1;
		reg->mdep_cfg[1] = mdep2;
		break;

	case HDP_APP_SOURCE_RELIABLE:
		reg->number_of_mdeps = 1;

		mdep1.mdep_role = BTHL_MDEP_ROLE_SOURCE;
		mdep1.channel_type = BTHL_CHANNEL_TYPE_RELIABLE;

		reg->mdep_cfg = malloc(reg->number_of_mdeps *
						sizeof(bthl_mdep_cfg_t));
		reg->mdep_cfg[0] = mdep1;
		break;

	case HDP_APP_SOURCE_STREAM:
		reg->number_of_mdeps = 2;

		mdep1.mdep_role = BTHL_MDEP_ROLE_SOURCE;
		mdep1.channel_type = BTHL_CHANNEL_TYPE_RELIABLE;

		mdep2.mdep_role = BTHL_MDEP_ROLE_SOURCE;
		mdep2.channel_type = BTHL_CHANNEL_TYPE_STREAMING;

		reg->mdep_cfg = malloc(reg->number_of_mdeps *
						sizeof(bthl_mdep_cfg_t));
		reg->mdep_cfg[0] = mdep1;
		reg->mdep_cfg[1] = mdep2;
		break;
	}


	return reg;
}

static void hdp_register_sink_reliable_app_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);
	int app_id = 0;
	bthl_reg_param_t *reg;

	reg = create_app(HDP_APP_SINK_RELIABLE);
	step->action_status = data->if_hdp->register_application(reg, &app_id);

	schedule_action_verification(step);
	free(reg->mdep_cfg);
	free(reg);
}

static void hdp_register_sink_stream_app_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);
	int app_id = 0;
	bthl_reg_param_t *reg;

	reg = create_app(HDP_APP_SINK_STREAM);
	step->action_status = data->if_hdp->register_application(reg, &app_id);

	schedule_action_verification(step);
	free(reg->mdep_cfg);
	free(reg);
}

static void hdp_register_source_reliable_app_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);
	int app_id = 0;
	bthl_reg_param_t *reg;

	reg = create_app(HDP_APP_SOURCE_RELIABLE);
	step->action_status = data->if_hdp->register_application(reg, &app_id);

	schedule_action_verification(step);
	free(reg->mdep_cfg);
	free(reg);
}

static void hdp_register_source_stream_app_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);
	int app_id = 0;
	bthl_reg_param_t *reg;

	reg = create_app(HDP_APP_SOURCE_STREAM);
	step->action_status = data->if_hdp->register_application(reg, &app_id);

	schedule_action_verification(step);
	free(reg->mdep_cfg);
	free(reg);
}

static void hdp_unregister_app_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_hdp->unregister_application(1);

	schedule_action_verification(step);
}

static void mcap_ctrl_cid_hook_cb(const void *data, uint16_t len,
							void *user_data)
{
	struct test_data *t_data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(t_data->hciemu);
	struct emu_l2cap_cid_data *cid_data = user_data;
	uint8_t crt_rsp[5], del_rsp[4], config;
	uint8_t opcode = ((uint8_t *) data)[0];
	static bool reliable = false;

	switch (opcode) {
	case 0x01: /* MD_CREATE_MDL_REQ */
		crt_rsp[0] = 0x02; /* MD_CREATE_MDL_RSP */
		crt_rsp[1] = 0x00; /* Response code - Success */
		crt_rsp[2] = ((uint8_t *) data)[1]; /* mdlid */
		crt_rsp[3] = ((uint8_t *) data)[2];
		config = ((uint8_t *) data)[4];

		if (config == 0x00) {
			if (!reliable) {
				crt_rsp[4] = 0x01;
				reliable = true;
			} else {
				crt_rsp[4] = 0x02;
				reliable = false;
			}
		} else {
			crt_rsp[4] = config;
		}

		bthost_send_cid(bthost, cid_data->handle,
					cid_data->cid,
					crt_rsp, sizeof(crt_rsp));
		break;
	case 0x03: /* MD_RECONNECT_MDL_REQ */
	case 0x05: /* MD_ABORT_MDL_REQ */
		break;
	case 0x07: /* MD_DELETE_MDL_REQ */
		del_rsp[0] = 0x08; /* MD_DELETE_MDL_RSP */
		del_rsp[1] = 0x00; /* Response code - Success */
		del_rsp[2] = ((uint8_t *) data)[1]; /* mdlid */
		del_rsp[3] = ((uint8_t *) data)[2];
		bthost_send_cid(bthost, cid_data->handle,
					cid_data->cid,
					del_rsp, sizeof(del_rsp));
		break;
	}
}

static void mcap_ctrl_connect_cb(uint16_t handle, uint16_t cid, void *user_data)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);
	struct emu_l2cap_cid_data *cid_data = user_data;

	cid_data->handle = handle;
	cid_data->cid = cid;

	bthost_add_cid_hook(bthost, handle, cid, mcap_ctrl_cid_hook_cb,
								cid_data);
}

/* Emulate SDP (PSM = 1) */
static struct emu_set_l2cap_data l2cap_setup_sdp_data = {
	.psm = 1,
	.func = tester_generic_connect_cb,
	.user_data = &sdp_cid_data,
};

/* Emulate Control Channel (PSM = 0x1001) */
static struct emu_set_l2cap_data l2cap_setup_cc_data = {
	.psm = 0x1001,
	.func = mcap_ctrl_connect_cb,
	.user_data = &ctrl_cid_data,
};

/* Emulate Data Channel (PSM = 0x1003) */
static struct emu_set_l2cap_data l2cap_setup_dc_data = {
	.psm = 0x1003,
	.func = tester_generic_connect_cb,
	.user_data = &data_cid_data,
};

static void hdp_connect_source_reliable_action(void)
{
	struct test_data *data = tester_get_data();
	const uint8_t *hid_addr = hciemu_get_client_bdaddr(data->hciemu);
	struct step *step = g_new0(struct step, 1);
	bt_bdaddr_t bdaddr;
	int app_id, channel_id, mdep_cfg_index;

	bdaddr2android((const bdaddr_t *) hid_addr, &bdaddr);
	app_id = 1;
	mdep_cfg_index = 0;
	channel_id = 0;
	step->action_status = data->if_hdp->connect_channel(app_id, &bdaddr,
						mdep_cfg_index, &channel_id);

	schedule_action_verification(step);
}

static void hdp_destroy_source_reliable_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_hdp->destroy_channel(1);
	schedule_action_verification(step);
}

static void hdp_connect_sink_reliable_action(void)
{
	struct test_data *data = tester_get_data();
	const uint8_t *hid_addr = hciemu_get_client_bdaddr(data->hciemu);
	struct step *step = g_new0(struct step, 1);
	bt_bdaddr_t bdaddr;
	int app_id, channel_id, mdep_cfg_index;

	bdaddr2android((const bdaddr_t *) hid_addr, &bdaddr);
	app_id = 1;
	mdep_cfg_index = 0;
	channel_id = 0;
	step->action_status = data->if_hdp->connect_channel(app_id, &bdaddr,
						mdep_cfg_index, &channel_id);

	schedule_action_verification(step);
}

static void hdp_connect_sink_stream_action(void)
{
	struct test_data *data = tester_get_data();
	const uint8_t *hid_addr = hciemu_get_client_bdaddr(data->hciemu);
	struct step *step = g_new0(struct step, 1);
	bt_bdaddr_t bdaddr;
	int app_id, channel_id, mdep_cfg_index;

	bdaddr2android((const bdaddr_t *) hid_addr, &bdaddr);
	app_id = 1;
	mdep_cfg_index = 1;
	channel_id = 0;
	step->action_status = data->if_hdp->connect_channel(app_id, &bdaddr,
						mdep_cfg_index, &channel_id);

	schedule_action_verification(step);
}

static void hdp_destroy_sink_reliable_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_hdp->destroy_channel(1);
	schedule_action_verification(step);
}

static void hdp_destroy_sink_stream_action(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_hdp->destroy_channel(2);
	schedule_action_verification(step);
}

static struct test_case test_cases[] = {
	TEST_CASE_BREDRLE("HDP Init",
		ACTION_SUCCESS(dummy_action, NULL),
	),
	TEST_CASE_BREDRLE("HDP Register Sink Reliable Application",
		ACTION_SUCCESS(hdp_register_sink_reliable_app_action, NULL),
		CALLBACK_HDP_APP_REG_STATE(CB_HDP_APP_REG_STATE, 1,
					BTHL_APP_REG_STATE_REG_SUCCESS),
	),
	TEST_CASE_BREDRLE("HDP Register Sink Stream Application",
		ACTION_SUCCESS(hdp_register_sink_stream_app_action, NULL),
		CALLBACK_HDP_APP_REG_STATE(CB_HDP_APP_REG_STATE, 1,
					BTHL_APP_REG_STATE_REG_SUCCESS),
	),
	TEST_CASE_BREDRLE("HDP Register Source Reliable Application",
		ACTION_SUCCESS(hdp_register_source_reliable_app_action, NULL),
		CALLBACK_HDP_APP_REG_STATE(CB_HDP_APP_REG_STATE, 1,
					BTHL_APP_REG_STATE_REG_SUCCESS),
	),
	TEST_CASE_BREDRLE("HDP Register Source Stream Application",
		ACTION_SUCCESS(hdp_register_source_stream_app_action, NULL),
		CALLBACK_HDP_APP_REG_STATE(CB_HDP_APP_REG_STATE, 1,
					BTHL_APP_REG_STATE_REG_SUCCESS),
	),
	TEST_CASE_BREDRLE("HDP Unegister Application",
		ACTION_SUCCESS(hdp_register_source_stream_app_action, NULL),
		CALLBACK_HDP_APP_REG_STATE(CB_HDP_APP_REG_STATE, 1,
					BTHL_APP_REG_STATE_REG_SUCCESS),
		ACTION_SUCCESS(hdp_unregister_app_action, NULL),
		CALLBACK_HDP_APP_REG_STATE(CB_HDP_APP_REG_STATE, 1,
					BTHL_APP_REG_STATE_DEREG_SUCCESS),
	),
	TEST_CASE_BREDRLE("HDP Connect Source Reliable Channel",
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_sdp_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_cc_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_dc_data),
		ACTION_SUCCESS(hdp_register_source_reliable_app_action, NULL),
		CALLBACK_HDP_APP_REG_STATE(CB_HDP_APP_REG_STATE, 1,
					BTHL_APP_REG_STATE_REG_SUCCESS),
		ACTION_SUCCESS(hdp_connect_source_reliable_action, NULL),
		CALLBACK_HDP_CHANNEL_STATE(CB_HDP_CHANNEL_STATE, 1, 1, 0,
						BTHL_CONN_STATE_CONNECTING),
		CALLBACK_HDP_CHANNEL_STATE(CB_HDP_CHANNEL_STATE, 1, 1, 0,
						BTHL_CONN_STATE_CONNECTED),
	),
	TEST_CASE_BREDRLE("HDP Destroy Source Reliable Channel",
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_sdp_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_cc_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_dc_data),
		ACTION_SUCCESS(hdp_register_source_reliable_app_action, NULL),
		CALLBACK_HDP_APP_REG_STATE(CB_HDP_APP_REG_STATE, 1,
					BTHL_APP_REG_STATE_REG_SUCCESS),
		ACTION_SUCCESS(hdp_connect_source_reliable_action, NULL),
		CALLBACK_HDP_CHANNEL_STATE(CB_HDP_CHANNEL_STATE, 1, 1, 0,
						BTHL_CONN_STATE_CONNECTING),
		CALLBACK_HDP_CHANNEL_STATE(CB_HDP_CHANNEL_STATE, 1, 1, 0,
						BTHL_CONN_STATE_CONNECTED),
		ACTION_SUCCESS(hdp_destroy_source_reliable_action, NULL),
		CALLBACK_HDP_CHANNEL_STATE(CB_HDP_CHANNEL_STATE, 1, 1, 0,
						BTHL_CONN_STATE_DESTROYED),
	),
	TEST_CASE_BREDRLE("HDP Connect Sink Streaming Channel",
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_sdp_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_cc_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_dc_data),
		ACTION_SUCCESS(hdp_register_sink_stream_app_action, NULL),
		CALLBACK_HDP_APP_REG_STATE(CB_HDP_APP_REG_STATE, 1,
					BTHL_APP_REG_STATE_REG_SUCCESS),
		ACTION_SUCCESS(hdp_connect_sink_reliable_action, NULL),
		CALLBACK_HDP_CHANNEL_STATE(CB_HDP_CHANNEL_STATE, 1, 1, 0,
						BTHL_CONN_STATE_CONNECTING),
		CALLBACK_HDP_CHANNEL_STATE(CB_HDP_CHANNEL_STATE, 1, 1, 0,
						BTHL_CONN_STATE_CONNECTED),
		ACTION_SUCCESS(hdp_connect_sink_stream_action, NULL),
		CALLBACK_HDP_CHANNEL_STATE(CB_HDP_CHANNEL_STATE, 1, 2, 1,
						BTHL_CONN_STATE_CONNECTED),
	),
	TEST_CASE_BREDRLE("HDP Destroy Sink Streaming Channel",
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_sdp_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_cc_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_dc_data),
		ACTION_SUCCESS(hdp_register_sink_stream_app_action, NULL),
		CALLBACK_HDP_APP_REG_STATE(CB_HDP_APP_REG_STATE, 1,
					BTHL_APP_REG_STATE_REG_SUCCESS),
		ACTION_SUCCESS(hdp_connect_sink_reliable_action, NULL),
		CALLBACK_HDP_CHANNEL_STATE(CB_HDP_CHANNEL_STATE, 1, 1, 0,
						BTHL_CONN_STATE_CONNECTING),
		CALLBACK_HDP_CHANNEL_STATE(CB_HDP_CHANNEL_STATE, 1, 1, 0,
						BTHL_CONN_STATE_CONNECTED),
		ACTION_SUCCESS(hdp_connect_sink_stream_action, NULL),
		CALLBACK_HDP_CHANNEL_STATE(CB_HDP_CHANNEL_STATE, 1, 2, 1,
						BTHL_CONN_STATE_CONNECTED),
		ACTION_SUCCESS(hdp_destroy_sink_reliable_action, NULL),
		CALLBACK_HDP_CHANNEL_STATE(CB_HDP_CHANNEL_STATE, 1, 1, 0,
						BTHL_CONN_STATE_DESTROYED),
		ACTION_SUCCESS(hdp_destroy_sink_stream_action, NULL),
		CALLBACK_HDP_CHANNEL_STATE(CB_HDP_CHANNEL_STATE, 1, 2, 1,
						BTHL_CONN_STATE_DESTROYED),
	),
};

struct queue *get_hdp_tests(void)
{
	uint16_t i = 0;

	list = queue_new();

	for (; i < sizeof(test_cases) / sizeof(test_cases[0]); ++i)
		queue_push_tail(list, &test_cases[i]);

	return list;
}

void remove_hdp_tests(void)
{
	queue_destroy(list, NULL);
}

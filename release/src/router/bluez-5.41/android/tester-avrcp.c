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

#define AVRCP_GET_ELEMENT_ATTRIBUTES	0x20
#define AVRCP_GET_PLAY_STATUS		0x30
#define AVRCP_REGISTER_NOTIFICATION	0x31

#define sdp_rsp_pdu	0x07, \
			0x00, 0x00, \
			0x00, 0x7f, \
			0x00, 0x7c, \
			0x36, 0x00, 0x79, 0x36, 0x00, 0x3b, 0x09, 0x00, 0x00, \
			0x0a, 0x00, 0x01, 0x00, 0x04, 0x09, 0x00, 0x01, 0x35, \
			0x06, 0x19, 0x11, 0x0e, 0x19, 0x11, 0x0f, 0x09, 0x00, \
			0x04, 0x35, 0x10, 0x35, 0x06, 0x19, 0x01, 0x00, 0x09, \
			0x00, 0x17, 0x35, 0x06, 0x19, 0x00, 0x17, 0x09, 0x01, \
			0x03, 0x09, 0x00, 0x09, 0x35, 0x08, 0x35, 0x06, 0x19, \
			0x11, 0x0e, 0x09, 0x01, 0x00, 0x09, 0x03, 0x11, 0x09, \
			0x00, 0x01, 0x36, 0x00, 0x38, 0x09, 0x00, 0x00, 0x0a, \
			0x00, 0x01, 0x00, 0x05, 0x09, 0x00, 0x01, 0x35, 0x03, \
			0x19, 0x11, 0x0c, 0x09, 0x00, 0x04, 0x35, 0x10, 0x35, \
			0x06, 0x19, 0x01, 0x00, 0x09, 0x00, 0x17, 0x35, 0x06, \
			0x19, 0x00, 0x17, 0x09, 0x01, 0x03, 0x09, 0x00, 0x09, \
			0x35, 0x08, 0x35, 0x06, 0x19, 0x11, 0x0e, 0x09, 0x01, \
			0x04, 0x09, 0x03, 0x11, 0x09, 0x00, 0x02, \
			0x00

static const struct pdu_set sdp_pdus[] = {
	{ end_pdu, raw_pdu(sdp_rsp_pdu) },
	{ end_pdu, end_pdu },
};

static struct emu_l2cap_cid_data sdp_data = {
	.pdu = sdp_pdus,
	.is_sdp = TRUE,
};

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

#define req_play_status 0x00, 0x11, 0x0e, 0x01, 0x48, 0x00, 0x00, 0x19, 0x58, \
							0x30, 0x00, 0x00, 0x00
#define rsp_play_status 0x02, 0x11, 0x0e, 0x0c, 0x48, 0x00, 0x00, 0x19, 0x58, \
			0x30, 0x00, 0x00, 0x09, 0xbb, 0xbb, 0xbb, 0xbb, 0xaa, \
							0xaa, 0xaa, 0xaa, 0x00

#define req_track_notif 0x00, 0x11, 0x0e, 0x03, 0x48, 0x00, 0x00, 0x19, 0x58, \
			0x31, 0x00, 0x00, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00

#define rsp_track_notif 0x00, 0x11, 0x0e, 0x0F, 0x48, 0x00, 0x00, 0x19, 0x58, \
			0x31, 0x00, 0x00, 0x09, 0x02, 0xFF, 0xFF, 0xFF, 0xFF, \
							0xFF, 0xFF, 0xFF, 0xFF

#define req_position_notif 0x00, 0x11, 0x0e, 0x03, 0x48, 0x00, 0x00, 0x19, \
				0x58, 0x31, 0x00, 0x00, 0x05, 0x05, 0x00, \
							0x00, 0x00, 0x00

#define rsp_position_notif 0x00, 0x11, 0x0e, 0x0F, 0x48, 0x00, 0x00, 0x19, \
				0x58, 0x31, 0x00, 0x00, 0x04, 0x05, 0xFF, \
							0xFF, 0xFF, 0xFF

#define req_status_notif 0x00, 0x11, 0x0e, 0x03, 0x48, 0x00, 0x00, 0x19, \
				0x58, 0x31, 0x00, 0x00, 0x05, 0x01, 0x00, \
							0x00, 0x00, 0x00

#define rsp_status_notif 0x00, 0x11, 0x0e, 0x0D, 0x48, 0x00, 0x00, 0x19, \
				0x58, 0x31, 0x00, 0x00, 0x01, 0x01, 0x00

#define req_ele_attr 0x00, 0x11, 0x0e, 0x01, 0x48, 0x00, 0x00, 0x19, 0x58, \
			0x20, 0x00, 0x00, 0x0D, 0x00, 0x00, 0x00, 0x00, \
			0x02, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x07

#define rsp_ele_attr 0x02, 0x11, 0x0e, 0x0c, 0x48, 0x00, 0x00, 0x19, 0x58, \
			0x20, 0x00, 0x00, 0x2a, 0x02, 0x00, 0x00, 0x00, 0x01, \
			0x00, 0x6a, 0x00, 0x13, 0x47, 0x69, 0x76, 0x65, 0x20, \
			0x50, 0x65, 0x61, 0x63, 0x65, 0x20, 0x61, 0x20, 0x43, \
			0x68, 0x61, 0x6e, 0x63, 0x65, 0x00, 0x00, 0x00, 0x07, \
			0x00, 0x6a, 0x00, 0x06, 0x31, 0x30, 0x33, 0x30, 0x30, \
									0x30

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

static struct emu_l2cap_cid_data a2dp_data = {
	.pdu = pdus,
};

static struct emu_l2cap_cid_data avrcp_data;

static btrc_element_attr_val_t ele_attrs[2] = {
	{
	.attr_id = BTRC_MEDIA_ATTR_TITLE,
	.text = {0x47, 0x69, 0x76, 0x65, 0x20, 0x50, 0x65, 0x61, 0x63, 0x65,
			 0x20, 0x61, 0x20, 0x43, 0x68, 0x61, 0x6e, 0x63, 0x65}
	},
	{
	.attr_id = BTRC_MEDIA_ATTR_PLAYING_TIME,
	.text = {0x31, 0x30, 0x33, 0x30, 0x30, 0x30}
	}
};

static btrc_element_attr_val_t exp_attrs[2];

static void print_avrcp(const char *str, void *user_data)
{
	tester_debug("avrcp: %s", str);
}

static void avrcp_cid_hook_cb(const void *data, uint16_t len, void *user_data)
{
	struct step *step;
	uint8_t pdu, event;

	util_hexdump('>', data, len, print_avrcp, NULL);

	pdu = ((uint8_t *) data)[9];
	switch (pdu) {
	case AVRCP_GET_PLAY_STATUS:
		step = g_new0(struct step, 1);
		step->callback = CB_AVRCP_PLAY_STATUS_RSP;
		step->callback_result.song_length = get_be32(data + 13);
		step->callback_result.song_position = get_be32(data + 17);
		step->callback_result.play_status = ((uint8_t *) data)[21];
		schedule_callback_verification(step);
		break;
	case AVRCP_REGISTER_NOTIFICATION:
		event = ((uint8_t *) data)[13];
		switch (event) {
		case 0x01:
			step = g_new0(struct step, 1);
			step->callback = CB_AVRCP_REG_NOTIF_RSP;
			step->callback_result.play_status =
							((uint8_t *) data)[14];
			schedule_callback_verification(step);
			break;

		case 0x02:
			step = g_new0(struct step, 1);
			step->callback = CB_AVRCP_REG_NOTIF_RSP;
			step->callback_result.rc_index = get_be64(data + 14);
			schedule_callback_verification(step);
			break;

		case 0x05:
			step = g_new0(struct step, 1);
			step->callback = CB_AVRCP_REG_NOTIF_RSP;
			step->callback_result.song_position =
							get_be32(data + 14);
			schedule_callback_verification(step);
			break;
		}
		break;
	case AVRCP_GET_ELEMENT_ATTRIBUTES:
		step = g_new0(struct step, 1);
		step->callback = CB_AVRCP_GET_ATTR_RSP;
		step->callback_result.num_of_attrs = ((uint8_t *) data)[13];

		memset(exp_attrs, 0, 2 * sizeof(btrc_element_attr_val_t));
		exp_attrs[0].attr_id = get_be16(data + 16);
		memcpy(exp_attrs[0].text, data + 22, 19);
		exp_attrs[1].attr_id = get_be16(data + 43);
		memcpy(exp_attrs[1].text, data + 49, 6);
		step->callback_result.attrs = exp_attrs;
		schedule_callback_verification(step);
		break;
	}
}

static void avrcp_connect_request_cb(uint16_t handle, uint16_t cid,
							void *user_data)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);
	struct emu_l2cap_cid_data *cid_data = user_data;

	cid_data->handle = handle;
	cid_data->cid = cid;

	bthost_add_cid_hook(bthost, handle, cid, avrcp_cid_hook_cb, cid_data);
}

static struct emu_set_l2cap_data avrcp_setup_data = {
	.psm = 23,
	.func = avrcp_connect_request_cb,
	.user_data = &avrcp_data,
};

static void a2dp_connect_request_cb(uint16_t handle, uint16_t cid,
							void *user_data)
{
	struct emu_l2cap_cid_data *cid_data = user_data;

	if (cid_data->handle)
		return;

	cid_data->handle = handle;
	cid_data->cid = cid;
	avrcp_data.handle = handle;
	avrcp_data.cid = cid;

	tester_handle_l2cap_data_exchange(cid_data);
}

static struct emu_set_l2cap_data a2dp_setup_data = {
	.psm = 25,
	.func = a2dp_connect_request_cb,
	.user_data = &a2dp_data,
};

static struct emu_set_l2cap_data sdp_setup_data = {
	.psm = 1,
	.func = tester_generic_connect_cb,
	.user_data = &sdp_data,
};

static void avrcp_connect_action(void)
{
	struct test_data *data = tester_get_data();
	const uint8_t *addr = hciemu_get_client_bdaddr(data->hciemu);
	struct step *step = g_new0(struct step, 1);
	bt_bdaddr_t bdaddr;

	sdp_data.handle = 0;
	sdp_data.cid = 0;

	a2dp_data.handle = 0;
	a2dp_data.cid = 0;

	avrcp_data.handle = 0;
	avrcp_data.cid = 0;

	bdaddr2android((const bdaddr_t *) addr, &bdaddr);

	step->action_status = data->if_a2dp->connect(&bdaddr);

	schedule_action_verification(step);
}

static void avrcp_disconnect_action(void)
{
	struct test_data *data = tester_get_data();
	const uint8_t *addr = hciemu_get_client_bdaddr(data->hciemu);
	struct step *step = g_new0(struct step, 1);
	bt_bdaddr_t bdaddr;

	bdaddr2android((const bdaddr_t *) addr, &bdaddr);

	step->action_status = data->if_a2dp->disconnect(&bdaddr);

	schedule_action_verification(step);
}

static void avrcp_get_play_status_req(void)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);
	const struct iovec pdu = raw_pdu(req_play_status);
	struct step *step = g_new0(struct step, 1);

	bthost_send_cid_v(bthost, avrcp_data.handle, avrcp_data.cid, &pdu, 1);
	step->action_status = BT_STATUS_SUCCESS;
	schedule_action_verification(step);
}

static void avrcp_get_play_status_rsp(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_avrcp->get_play_status_rsp(0x00,
						0xbbbbbbbb, 0xaaaaaaaa);
	schedule_action_verification(step);
}

static void avrcp_reg_notif_track_changed_req(void)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);
	const struct iovec pdu = raw_pdu(req_track_notif);
	struct step *step = g_new0(struct step, 1);

	bthost_send_cid_v(bthost, avrcp_data.handle, avrcp_data.cid, &pdu, 1);
	step->action_status = BT_STATUS_SUCCESS;
	schedule_action_verification(step);
}

static void avrcp_reg_notif_track_changed_rsp(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);
	uint64_t track;
	btrc_register_notification_t reg;

	track = 0xffffffffffffffff;
	memcpy(reg.track, &track, sizeof(btrc_uid_t));
	step->action_status = data->if_avrcp->register_notification_rsp(
							BTRC_EVT_TRACK_CHANGE,
					BTRC_NOTIFICATION_TYPE_INTERIM, &reg);

	schedule_action_verification(step);
}

static void avrcp_reg_notif_play_position_changed_req(void)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);
	const struct iovec pdu = raw_pdu(req_position_notif);
	struct step *step = g_new0(struct step, 1);

	bthost_send_cid_v(bthost, avrcp_data.handle, avrcp_data.cid, &pdu, 1);
	step->action_status = BT_STATUS_SUCCESS;
	schedule_action_verification(step);
}

static void avrcp_reg_notif_play_position_changed_rsp(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);
	btrc_register_notification_t reg;

	reg.song_pos = 0xffffffff;
	step->action_status = data->if_avrcp->register_notification_rsp(
						BTRC_EVT_PLAY_POS_CHANGED,
					BTRC_NOTIFICATION_TYPE_INTERIM, &reg);

	schedule_action_verification(step);
}

static void avrcp_reg_notif_play_status_changed_req(void)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);
	const struct iovec pdu = raw_pdu(req_status_notif);
	struct step *step = g_new0(struct step, 1);

	bthost_send_cid_v(bthost, avrcp_data.handle, avrcp_data.cid, &pdu, 1);
	step->action_status = BT_STATUS_SUCCESS;
	schedule_action_verification(step);
}

static void avrcp_reg_notif_play_status_changed_rsp(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);
	btrc_register_notification_t reg;

	reg.play_status = BTRC_PLAYSTATE_STOPPED;
	step->action_status = data->if_avrcp->register_notification_rsp(
						BTRC_EVT_PLAY_STATUS_CHANGED,
					BTRC_NOTIFICATION_TYPE_CHANGED, &reg);

	schedule_action_verification(step);
}

static void avrcp_get_element_attributes_req(void)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);
	const struct iovec pdu = raw_pdu(req_ele_attr);
	struct step *step = g_new0(struct step, 1);

	bthost_send_cid_v(bthost, avrcp_data.handle, avrcp_data.cid, &pdu, 1);
	step->action_status = BT_STATUS_SUCCESS;
	schedule_action_verification(step);
}

static void avrcp_get_element_attributes_rsp(void)
{
	struct test_data *data = tester_get_data();
	struct step *step = g_new0(struct step, 1);

	step->action_status = data->if_avrcp->get_element_attr_rsp(2,
								ele_attrs);

	schedule_action_verification(step);
}

static struct test_case test_cases[] = {
	TEST_CASE_BREDRLE("AVRCP Init",
		ACTION_SUCCESS(dummy_action, NULL),
	),
	TEST_CASE_BREDRLE("AVRCP Connect - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &sdp_setup_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &a2dp_setup_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &avrcp_setup_data),
		ACTION_SUCCESS(avrcp_connect_action, NULL),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTING),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTED),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("AVRCP Disconnect - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &sdp_setup_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &a2dp_setup_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &avrcp_setup_data),
		ACTION_SUCCESS(avrcp_connect_action, NULL),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTING),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTED),
		ACTION_SUCCESS(avrcp_disconnect_action, NULL),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_DISCONNECTING),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_DISCONNECTED),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("AVRCP GetPlayStatus - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &sdp_setup_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &a2dp_setup_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &avrcp_setup_data),
		ACTION_SUCCESS(avrcp_connect_action, NULL),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTING),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTED),
		ACTION_SUCCESS(avrcp_get_play_status_req, NULL),
		CALLBACK(CB_AVRCP_PLAY_STATUS_REQ),
		ACTION_SUCCESS(avrcp_get_play_status_rsp, NULL),
		CALLBACK_RC_PLAY_STATUS(CB_AVRCP_PLAY_STATUS_RSP, 0xbbbbbbbb,
							0xaaaaaaaa, 0x00),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("AVRCP RegNotifTrackChanged - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &sdp_setup_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &a2dp_setup_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &avrcp_setup_data),
		ACTION_SUCCESS(avrcp_connect_action, NULL),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTING),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTED),
		ACTION_SUCCESS(avrcp_reg_notif_track_changed_req, NULL),
		CALLBACK(CB_AVRCP_REG_NOTIF_REQ),
		ACTION_SUCCESS(avrcp_reg_notif_track_changed_rsp, NULL),
		CALLBACK_RC_REG_NOTIF_TRACK_CHANGED(CB_AVRCP_REG_NOTIF_RSP,
							0xffffffffffffffff),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("AVRCP RegNotifPlayPositionChanged - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &sdp_setup_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &a2dp_setup_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &avrcp_setup_data),
		ACTION_SUCCESS(avrcp_connect_action, NULL),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTING),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTED),
		ACTION_SUCCESS(avrcp_reg_notif_play_position_changed_req, NULL),
		CALLBACK(CB_AVRCP_REG_NOTIF_REQ),
		ACTION_SUCCESS(avrcp_reg_notif_play_position_changed_rsp, NULL),
		CALLBACK_RC_REG_NOTIF_POSITION_CHANGED(CB_AVRCP_REG_NOTIF_RSP,
								0xffffffff),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("AVRCP RegNotifPlayStatusChanged - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &sdp_setup_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &a2dp_setup_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &avrcp_setup_data),
		ACTION_SUCCESS(avrcp_connect_action, NULL),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTING),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTED),
		ACTION_SUCCESS(avrcp_reg_notif_play_status_changed_req, NULL),
		CALLBACK(CB_AVRCP_REG_NOTIF_REQ),
		ACTION_SUCCESS(avrcp_reg_notif_play_status_changed_rsp, NULL),
		CALLBACK_RC_REG_NOTIF_STATUS_CHANGED(CB_AVRCP_REG_NOTIF_RSP,
									0x00),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("AVRCP GetElementAttributes - Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &sdp_setup_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &a2dp_setup_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action, &avrcp_setup_data),
		ACTION_SUCCESS(avrcp_connect_action, NULL),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTING),
		CALLBACK_AV_CONN_STATE(CB_A2DP_CONN_STATE,
					BTAV_CONNECTION_STATE_CONNECTED),
		ACTION_SUCCESS(avrcp_get_element_attributes_req, NULL),
		CALLBACK(CB_AVRCP_GET_ATTR_REQ),
		ACTION_SUCCESS(avrcp_get_element_attributes_rsp, NULL),
		CALLBACK_RC_GET_ELEMENT_ATTRIBUTES(CB_AVRCP_GET_ATTR_RSP, 2,
								ele_attrs),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
};

struct queue *get_avrcp_tests(void)
{
	uint16_t i = 0;

	list = queue_new();

	for (; i < sizeof(test_cases) / sizeof(test_cases[0]); ++i)
		queue_push_tail(list, &test_cases[i]);

	return list;
}

void remove_avrcp_tests(void)
{
	queue_destroy(list, NULL);
}

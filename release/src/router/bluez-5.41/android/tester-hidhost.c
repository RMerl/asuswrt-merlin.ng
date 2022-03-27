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
#include "lib/bluetooth.h"
#include "android/utils.h"
#include "tester-main.h"

#define HID_GET_REPORT_PROTOCOL		0x60
#define HID_GET_BOOT_PROTOCOL		0x61
#define HID_SET_REPORT_PROTOCOL		0x70
#define HID_SET_BOOT_PROTOCOL		0x71

#define HID_SET_INPUT_REPORT		0x51
#define HID_SET_OUTPUT_REPORT		0x52
#define HID_SET_FEATURE_REPORT		0x53

#define HID_SEND_DATA			0xa2

#define HID_GET_INPUT_REPORT		0x49
#define HID_GET_OUTPUT_REPORT		0x4a
#define HID_GET_FEATURE_REPORT		0x4b

#define HID_MODE_DEFAULT		0x00
#define HID_MODE_BREDR			0x01
#define HID_MODE_LE			0x02

#define HID_EXPECTED_REPORT_SIZE	0x02

#define HID_VIRTUAL_CABLE_UNPLUG	0x15

static struct queue *list; /* List of hidhost test cases */

#define did_req_pdu	0x06, \
			0x00, 0x00, \
			0x00, 0x0f, \
			0x35, 0x03, \
			0x19, 0x12, 0x00, 0xff, 0xff, 0x35, 0x05, 0x0a, 0x00, \
			0x00, 0xff, 0xff, 0x00

#define did_rsp_pdu	0x07, \
			0x00, 0x00, \
			0x00, 0x4f, \
			0x00, 0x4c, \
			0x35, 0x4a, 0x35, 0x48, 0x09, 0x00, 0x00, 0x0a, 0x00, \
			0x01, 0x00, 0x00, 0x09, 0x00, 0x01, 0x35, 0x03, 0x19, \
			0x12, 0x00, 0x09, 0x00, 0x05, 0x35, 0x03, 0x19, 0x10, \
			0x02, 0x09, 0x00, 0x09, 0x35, 0x08, 0x35, 0x06, 0x19, \
			0x12, 0x00, 0x09, 0x01, 0x03, 0x09, 0x02, 0x00, 0x09, \
			0x01, 0x03, 0x09, 0x02, 0x01, 0x09, 0x1d, 0x6b, 0x09, \
			0x02, 0x02, 0x09, 0x02, 0x46, 0x09, 0x02, 0x03, 0x09, \
			0x05, 0x0e, 0x09, 0x02, 0x04, 0x28, 0x01, 0x09, 0x02, \
			0x05, 0x09, 0x00, 0x02, \
			0x00

#define hid_req_pdu	0x06, \
			0x00, 0x01, \
			0x00, 0x0f, \
			0x35, 0x03, \
			0x19, 0x11, 0x24, 0xff, 0xff, 0x35, 0x05, 0x0a, 0x00, \
			0x00, 0xff, 0xff, 0x00

#define hid_rsp_pdu	0x07, \
			0x00, 0x01, \
			0x01, 0x71, \
			0x01, 0x6E, \
			0x36, 0x01, 0x6b, 0x36, 0x01, 0x68, 0x09, 0x00, 0x00, \
			0x0a, 0x00, 0x01, 0x00, 0x00, 0x09, 0x00, 0x01, 0x35, \
			0x03, 0x19, 0x11, 0x24, 0x09, 0x00, 0x04, 0x35, 0x0d, \
			0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x00, 0x11, 0x35, \
			0x03, 0x19, 0x00, 0x11, 0x09, 0x00, 0x05, 0x35, 0x03, \
			0x19, 0x10, 0x02, 0x09, 0x00, 0x06, 0x35, 0x09, 0x09, \
			0x65, 0x6e, 0x09, 0x00, 0x6a, 0x09, 0x01, 0x00, 0x09, \
			0x00, 0x09, 0x35, 0x08, 0x35, 0x06, 0x19, 0x11, 0x24, \
			0x09, 0x01, 0x00, 0x09, 0x00, 0x0d, 0x35, 0x0f, 0x35, \
			0x0d, 0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x00, 0x13, \
			0x35, 0x03, 0x19, 0x00, 0x11, 0x09, 0x01, 0x00, 0x25, \
			0x1e, 0x4c, 0x6f, 0x67, 0x69, 0x74, 0x65, 0x63, 0x68, \
			0x20, 0x42, 0x6c, 0x75, 0x65, 0x74, 0x6f, 0x6f, 0x74, \
			0x68, 0x20, 0x4d, 0x6f, 0x75, 0x73, 0x65, 0x20, 0x4d, \
			0x35, 0x35, 0x35, 0x62, 0x09, 0x01, 0x01, 0x25, 0x0f, \
			0x42, 0x6c, 0x75, 0x65, 0x74, 0x6f, 0x6f, 0x74, 0x68, \
			0x20, 0x4d, 0x6f, 0x75, 0x73, 0x65, 0x09, 0x01, 0x02, \
			0x25, 0x08, 0x4c, 0x6f, 0x67, 0x69, 0x74, 0x65, 0x63, \
			0x68, 0x09, 0x02, 0x00, 0x09, 0x01, 0x00, 0x09, 0x02, \
			0x01, 0x09, 0x01, 0x11, 0x09, 0x02, 0x02, 0x08, 0x80, \
			0x09, 0x02, 0x03, 0x08, 0x21, 0x09, 0x02, 0x04, 0x28, \
			0x01, 0x09, 0x02, 0x05, 0x28, 0x01, 0x09, 0x02, 0x06, \
			0x35, 0x74, 0x35, 0x72, 0x08, 0x22, 0x25, 0x6e, 0x05, \
			0x01, 0x09, 0x02, 0xa1, 0x01, 0x85, 0x02, 0x09, 0x01, \
			0xa1, 0x00, 0x05, 0x09, 0x19, 0x01, 0x29, 0x08, 0x15, \
			0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x08, 0x81, 0x02, \
			0x05, 0x01, 0x09, 0x30, 0x09, 0x31, 0x16, 0x01, 0xf8, \
			0x26, 0xff, 0x07, 0x75, 0x0c, 0x95, 0x02, 0x81, 0x06, \
			0x09, 0x38, 0x15, 0x81, 0x25, 0x7f, 0x75, 0x08, 0x95, \
			0x01, 0x81, 0x06, 0x05, 0x0c, 0x0a, 0x38, 0x02, 0x81, \
			0x06, 0x05, 0x09, 0x19, 0x09, 0x29, 0x10, 0x15, 0x00, \
			0x25, 0x01, 0x95, 0x08, 0x75, 0x01, 0x81, 0x02, 0xc0, \
			0xc0, 0x06, 0x00, 0xff, 0x09, 0x01, 0xa1, 0x01, 0x85, \
			0x10, 0x75, 0x08, 0x95, 0x06, 0x15, 0x00, 0x26, 0xff, \
			0x00, 0x09, 0x01, 0x81, 0x00, 0x09, 0x01, 0x91, 0x00, \
			0xc0, 0x09, 0x02, 0x07, 0x35, 0x08, 0x35, 0x06, 0x09, \
			0x04, 0x09, 0x09, 0x01, 0x00, 0x09, 0x02, 0x08, 0x28, \
			0x00, 0x09, 0x02, 0x09, 0x28, 0x01, 0x09, 0x02, 0x0a, \
			0x28, 0x01, 0x09, 0x02, 0x0b, 0x09, 0x01, 0x00, 0x09, \
			0x02, 0x0c, 0x09, 0x0c, 0x80, 0x09, 0x02, 0x0d, 0x28, \
			0x00, 0x09, 0x02, 0x0e, 0x28, 0x01, \
			0x00

static const struct pdu_set sdp_pdus[] = {
	{ raw_pdu(did_req_pdu), raw_pdu(did_rsp_pdu) },
	{ raw_pdu(hid_req_pdu), raw_pdu(hid_rsp_pdu) },
	{ end_pdu, end_pdu },
};

static struct emu_l2cap_cid_data sdp_cid_data = {
	.pdu = sdp_pdus,
	.is_sdp = TRUE,
};

#define hid_keyboard_rsp_pdu	0x07, \
			0x00, 0x01, \
			0x02, 0x04, \
			0x02, 0x01, \
			0x36, 0x01, 0xfe, 0x36, 0x01, 0x93, 0x09, 0x00, 0x00, \
			0x0a, 0x00, 0x01, 0x00, 0x00, 0x09, 0x00, 0x01, 0x35, \
			0x03, 0x19, 0x11, 0x24, 0x09, 0x00, 0x04, 0x35, 0x0d, \
			0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x00, 0x11, 0x35, \
			0x03, 0x19, 0x00, 0x11, 0x09, 0x00, 0x06, 0x35, 0x09, \
			0x09, 0x65, 0x6e, 0x09, 0x00, 0x6a, 0x09, 0x01, 0x00, \
			0x09, 0x00, 0x09, 0x35, 0x08, 0x35, 0x06, 0x19, 0x11, \
			0x24, 0x09, 0x01, 0x00, 0x09, 0x00, 0x0d, 0x35, 0x0f, \
			0x35, 0x0d, 0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x00, \
			0x13, 0x35, 0x03, 0x19, 0x00, 0x11, 0x09, 0x01, 0x00, \
			0x25, 0x10, 0x53, 0x41, 0x4d, 0x53, 0x55, 0x4e, 0x47, \
			0x20, 0x4b, 0x65, 0x79, 0x62, 0x6f, 0x61, 0x72, 0x64, \
			0x09, 0x01, 0x01, 0x25, 0x08, 0x4b, 0x65, 0x79, 0x62, \
			0x6f, 0x61, 0x72, 0x64, 0x09, 0x01, 0x02, 0x25, 0x0d, \
			0x43, 0x53, 0x52, 0x20, 0x48, 0x49, 0x44, 0x45, 0x6e, \
			0x67, 0x69, 0x6e, 0x65, 0x09, 0x02, 0x00, 0x09, 0x01, \
			0x00, 0x09, 0x02, 0x01, 0x09, 0x01, 0x11, 0x09, 0x02, \
			0x02, 0x08, 0x40, 0x09, 0x02, 0x03, 0x08, 0x23, 0x09, \
			0x02, 0x04, 0x28, 0x01, 0x09, 0x02, 0x05, 0x28, 0x01, \
			0x09, 0x02, 0x06, 0x35, 0xb7, 0x35, 0xb5, 0x08, 0x22, \
			0x25, 0xb1, 0x05, 0x01, 0x09, 0x06, 0xa1, 0x01, 0x05, \
			0x07, 0x85, 0x01, 0x19, 0xe0, 0x29, 0xe7, 0x15, 0x00, \
			0x25, 0x01, 0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x95, \
			0x01, 0x75, 0x08, 0x81, 0x01, 0x95, 0x05, 0x75, 0x01, \
			0x05, 0x08, 0x85, 0x01, 0x19, 0x01, 0x29, 0x05, 0x91, \
			0x02, 0x95, 0x01, 0x75, 0x03, 0x91, 0x03, 0x95, 0x06, \
			0x75, 0x08, 0x15, 0x00, 0x25, 0x65, 0x05, 0x07, 0x19, \
			0x00, 0x29, 0x6f, 0x81, 0x00, 0xc0, 0x05, 0x0c, 0x09, \
			0x01, 0xa1, 0x01, 0x85, 0x02, 0x05, 0x0c, 0x15, 0x00, \
			0x25, 0x01, 0x75, 0x01, 0x95, 0x18, 0x09, 0xe2, 0x09, \
			0xea, 0x09, 0xe9, 0x09, 0xb7, 0x09, 0xcd, 0x0a, 0x23, \
			0x02, 0x0a, 0x8a, 0x01, 0x0a, 0x21, 0x02, 0x75, 0x01, \
			0x95, 0x03, 0x81, 0x02, 0x75, 0x01, 0x95, 0x05, 0x81, \
			0x01, 0x05, 0x08, 0x85, 0xff, 0x95, 0x01, 0x75, 0x02, \
			0x09, 0x24, 0x09, 0x26, 0x81, 0x02, 0x75, 0x06, 0x81, \
			0x01, 0xc0, 0x06, 0x7f, 0xff, 0x09, 0x01, 0xa1, 0x01, \
			0x85, 0x03, 0x15, 0x00, 0x25, 0x01, 0x09, 0xb9, 0x09, \
			0xb5, 0x09, 0xba, 0x09, 0xbb, 0x09, 0xbc, 0x09, 0xbd, \
			0x09, 0xb6, 0x09, 0xb7, 0x75, 0x01, 0x95, 0x06, 0x81, \
			0x02, 0x75, 0x01, 0x95, 0x02, 0x81, 0x01, 0xc0, 0x09, \
			0x02, 0x07, 0x35, 0x08, 0x35, 0x06, 0x09, 0x04, 0x09, \
			0x09, 0x01, 0x00, 0x09, 0x02, 0x08, 0x28, 0x00, 0x09, \
			0x02, 0x09, 0x28, 0x01, 0x09, 0x02, 0x0a, 0x28, 0x01, \
			0x09, 0x02, 0x0b, 0x09, 0x01, 0x00, 0x09, 0x02, 0x0c, \
			0x09, 0x1f, 0x40, 0x09, 0x02, 0x0d, 0x28, 0x00, 0x09, \
			0x02, 0x0e, 0x28, 0x01, 0x36, 0x00, 0x65, 0x09, 0x00, \
			0x00, 0x0a, 0x00, 0x01, 0x00, 0x01, 0x09, 0x00, 0x01, \
			0x35, 0x03, 0x19, 0x12, 0x00, 0x09, 0x00, 0x04, 0x35, \
			0x0d, 0x35, 0x06, 0x19, 0x01, 0x00, 0x09, 0x00, 0x01, \
			0x35, 0x03, 0x19, 0x00, 0x01, 0x09, 0x00, 0x06, 0x35, \
			0x09, 0x09, 0x65, 0x6e, 0x09, 0x00, 0x6a, 0x09, 0x01, \
			0x00, 0x09, 0x00, 0x09, 0x35, 0x08, 0x35, 0x06, 0x19, \
			0x12, 0x00, 0x09, 0x01, 0x00, 0x09, 0x01, 0x01, 0x25, \
			0x00, 0x09, 0x02, 0x00, 0x09, 0x01, 0x00, 0x09, 0x02, \
			0x01, 0x09, 0x23, 0x3d, 0x09, 0x02, 0x02, 0x09, 0x01, \
			0x3d, 0x09, 0x02, 0x03, 0x09, 0x00, 0x00, 0x09, 0x02, \
			0x04, 0x28, 0x01, 0x09, 0x02, 0x05, 0x09, 0x00,	0x02, \
			0x00

static const struct pdu_set sdp_kb_pdus[] = {
	{ raw_pdu(did_req_pdu), raw_pdu(did_rsp_pdu) },
	{ raw_pdu(hid_req_pdu), raw_pdu(hid_keyboard_rsp_pdu) },
	{ end_pdu, end_pdu },
};

static struct emu_l2cap_cid_data sdp_kb_cid_data = {
	.pdu = sdp_kb_pdus,
	.is_sdp = TRUE,
};

static struct emu_l2cap_cid_data ctrl_cid_data;
static struct emu_l2cap_cid_data intr_cid_data;

static void hid_prepare_reply_protocol_mode(struct emu_l2cap_cid_data *cid_data)
{
	struct test_data *t_data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(t_data->hciemu);
	const struct iovec pdu = raw_pdu(0xa0, 0x00);

	bthost_send_cid_v(bthost, cid_data->handle, cid_data->cid, &pdu, 1);
}

static void hid_prepare_reply_report(struct emu_l2cap_cid_data *cid_data)
{
	struct test_data *t_data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(t_data->hciemu);
	const struct iovec pdu = raw_pdu(0xa2, 0x01, 0x00);

	bthost_send_cid_v(bthost, cid_data->handle, cid_data->cid, &pdu, 1);
}

static void hid_ctrl_cid_hook_cb(const void *data, uint16_t len,
							void *user_data)
{
	struct emu_l2cap_cid_data *cid_data = user_data;
	uint8_t header = ((uint8_t *) data)[0];
	struct step *step;

	switch (header) {
	case HID_GET_REPORT_PROTOCOL:
	case HID_GET_BOOT_PROTOCOL:
	case HID_SET_REPORT_PROTOCOL:
	case HID_SET_BOOT_PROTOCOL:
		hid_prepare_reply_protocol_mode(cid_data);
		break;
	case HID_GET_INPUT_REPORT:
	case HID_GET_OUTPUT_REPORT:
	case HID_GET_FEATURE_REPORT:
		hid_prepare_reply_report(cid_data);
		break;
	/*
	 * HID device doesnot reply for this commads, so reaching pdu's
	 * to hid device means assuming test passed
	 */
	case HID_SET_INPUT_REPORT:
	case HID_SET_OUTPUT_REPORT:
	case HID_SET_FEATURE_REPORT:
	case HID_SEND_DATA:
		/* Successfully verify sending data step */
		step = g_new0(struct step, 1);

		step->callback = CB_EMU_CONFIRM_SEND_DATA;

		schedule_callback_verification(step);
		break;
	case HID_VIRTUAL_CABLE_UNPLUG:
		step = g_new0(struct step, 1);

		step->callback = CB_EMU_CONNECTION_REJECTED;

		schedule_callback_verification(step);
		break;
	}
}
static void hid_ctrl_connect_cb(uint16_t handle, uint16_t cid, void *user_data)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);
	struct emu_l2cap_cid_data *cid_data = user_data;

	cid_data->handle = handle;
	cid_data->cid = cid;

	bthost_add_cid_hook(bthost, handle, cid, hid_ctrl_cid_hook_cb,
								cid_data);
}

static void hid_intr_cid_hook_cb(const void *data, uint16_t len,
							void *user_data)
{
	uint8_t header = ((uint8_t *) data)[0];
	struct step *step;

	switch (header) {
	case HID_SEND_DATA:
		/* Successfully verify sending data step */
		step = g_new0(struct step, 1);

		step->callback = CB_EMU_CONFIRM_SEND_DATA;

		schedule_callback_verification(step);
		break;
	}
}
static void hid_intr_connect_cb(uint16_t handle, uint16_t cid, void *user_data)
{
	struct test_data *data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);
	struct emu_l2cap_cid_data *cid_data = user_data;

	cid_data->handle = handle;
	cid_data->cid = cid;

	bthost_add_cid_hook(bthost, handle, cid, hid_intr_cid_hook_cb,
								cid_data);
}

static bt_scan_mode_t setprop_scan_mode_conn_val = BT_SCAN_MODE_CONNECTABLE;

static bt_property_t prop_test_scan_mode_conn = {
	.type = BT_PROPERTY_ADAPTER_SCAN_MODE,
	.val = &setprop_scan_mode_conn_val,
	.len = sizeof(setprop_scan_mode_conn_val),
};

/* Emulate SDP (PSM = 1) */
static struct emu_set_l2cap_data l2cap_setup_sdp_data = {
	.psm = 1,
	.func = tester_generic_connect_cb,
	.user_data = &sdp_cid_data,
};

static struct emu_set_l2cap_data l2cap_setup_kb_sdp_data = {
	.psm = 1,
	.func = tester_generic_connect_cb,
	.user_data = &sdp_kb_cid_data,
};

/* Emulate Control Channel (PSM = 17) */
static struct emu_set_l2cap_data l2cap_setup_cc_data = {
	.psm = 17,
	.func = hid_ctrl_connect_cb,
	.user_data = &ctrl_cid_data,
};

/* Emulate Interrupt Channel (PSM = 19) */
static struct emu_set_l2cap_data l2cap_setup_ic_data = {
	.psm = 19,
	.func = hid_intr_connect_cb,
	.user_data = &intr_cid_data,
};

static void hidhost_connect_action(void)
{
	struct test_data *data = tester_get_data();
	const uint8_t *hid_addr = hciemu_get_client_bdaddr(data->hciemu);
	struct step *step = g_new0(struct step, 1);
	bt_bdaddr_t bdaddr;

	bdaddr2android((const bdaddr_t *) hid_addr, &bdaddr);

	step->action_status = data->if_hid->connect(&bdaddr);

	schedule_action_verification(step);
}

static void hidhost_disconnect_action(void)
{
	struct test_data *data = tester_get_data();
	const uint8_t *hid_addr = hciemu_get_client_bdaddr(data->hciemu);
	struct step *step = g_new0(struct step, 1);
	bt_bdaddr_t bdaddr;

	bdaddr2android((const bdaddr_t *) hid_addr, &bdaddr);

	step->action_status = data->if_hid->disconnect(&bdaddr);

	schedule_action_verification(step);
}

static void hidhost_virtual_unplug_action(void)
{
	struct test_data *data = tester_get_data();
	const uint8_t *hid_addr = hciemu_get_client_bdaddr(data->hciemu);
	struct step *step = g_new0(struct step, 1);
	bt_bdaddr_t bdaddr;

	bdaddr2android((const bdaddr_t *) hid_addr, &bdaddr);

	step->action_status = data->if_hid->virtual_unplug(&bdaddr);

	schedule_action_verification(step);
}

static void hidhost_get_protocol_action(void)
{
	struct test_data *data = tester_get_data();
	const uint8_t *hid_addr = hciemu_get_client_bdaddr(data->hciemu);
	struct step *step = g_new0(struct step, 1);
	bt_bdaddr_t bdaddr;

	bdaddr2android((const bdaddr_t *) hid_addr, &bdaddr);

	step->action_status = data->if_hid->get_protocol(&bdaddr,
							BTHH_REPORT_MODE);

	schedule_action_verification(step);
}

static void hidhost_set_protocol_action(void)
{
	struct test_data *data = tester_get_data();
	const uint8_t *hid_addr = hciemu_get_client_bdaddr(data->hciemu);
	struct step *step = g_new0(struct step, 1);
	bt_bdaddr_t bdaddr;

	bdaddr2android((const bdaddr_t *) hid_addr, &bdaddr);

	step->action_status = data->if_hid->set_protocol(&bdaddr,
							BTHH_REPORT_MODE);

	schedule_action_verification(step);
}

static void hidhost_get_report_action(void)
{
	struct test_data *data = tester_get_data();
	const uint8_t *hid_addr = hciemu_get_client_bdaddr(data->hciemu);
	struct step *step = g_new0(struct step, 1);
	bt_bdaddr_t bdaddr;

	bdaddr2android((const bdaddr_t *) hid_addr, &bdaddr);

	step->action_status = data->if_hid->get_report(&bdaddr,
							BTHH_INPUT_REPORT, 1,
							20);

	schedule_action_verification(step);
}

static void hidhost_set_report_action(void)
{
	struct test_data *data = tester_get_data();
	const uint8_t *hid_addr = hciemu_get_client_bdaddr(data->hciemu);
	struct step *step = g_new0(struct step, 1);
	char *buf = "fe0201";
	bt_bdaddr_t bdaddr;

	bdaddr2android((const bdaddr_t *) hid_addr, &bdaddr);

	step->action_status = data->if_hid->send_data(&bdaddr, buf);
	schedule_action_verification(step);
}

static void hidhost_send_data_action(void)
{
	struct test_data *data = tester_get_data();
	const uint8_t *hid_addr = hciemu_get_client_bdaddr(data->hciemu);
	struct step *step = g_new0(struct step, 1);
	char *buf = "010101";
	bt_bdaddr_t bdaddr;

	bdaddr2android((const bdaddr_t *) hid_addr, &bdaddr);

	step->action_status = data->if_hid->set_report(&bdaddr,
							BTHH_INPUT_REPORT, buf);
	schedule_action_verification(step);
}

static void client_l2cap_rsp(uint8_t code, const void *data, uint16_t len,
							void *user_data)
{
	struct test_data *t_data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(t_data->hciemu);
	struct emu_l2cap_cid_data *cid_data = user_data;
	const uint16_t *psm = data;
	const struct iovec con_req = raw_pdu(0x13, 0x00,	/* PSM */
						0x41, 0x00);	/* Source CID */

	if (len < sizeof(*psm)) {
		tester_warn("Invalid l2cap response.");
		return;
	}

	switch (*psm) {
	case 0x40:
		bthost_add_cid_hook(bthost, cid_data->handle, 0x40,
						hid_ctrl_cid_hook_cb, cid_data);

		bthost_l2cap_req(bthost, cid_data->handle, 0x02,
					con_req.iov_base, con_req.iov_len,
					client_l2cap_rsp, cid_data);
		break;
	case 0x41:
		bthost_add_cid_hook(bthost, cid_data->handle, 0x41,
						hid_intr_cid_hook_cb, cid_data);
		break;
	default:
		break;
	}
}

static void hidhost_conn_cb(uint16_t handle, void *user_data)
{
	const struct iovec con_req = raw_pdu(0x11, 0x00,	/* PSM */
						0x40, 0x00);	/* Source CID */

	struct test_data *data = tester_get_data();
	struct bthost *bthost = hciemu_client_get_host(data->hciemu);

	if (data->hciemu_type == HCIEMU_TYPE_BREDR) {
		tester_warn("Not handled device type.");
		return;
	}

	ctrl_cid_data.cid = 0x40;
	ctrl_cid_data.handle = handle;

	tester_print("Sending L2CAP Request from remote");

	bthost_l2cap_req(bthost, handle, 0x02, con_req.iov_base,
					con_req.iov_len, client_l2cap_rsp,
					&ctrl_cid_data);
}

static struct test_case test_cases[] = {
	TEST_CASE_BREDRLE("HidHost Init",
		ACTION_SUCCESS(dummy_action, NULL),
	),
	TEST_CASE_BREDRLE("HidHost Connect Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_sdp_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_cc_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_ic_data),
		ACTION_SUCCESS(hidhost_connect_action, NULL),
		CALLBACK_STATE(CB_HH_CONNECTION_STATE,
						BTHH_CONN_STATE_CONNECTED),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_HH_CONNECTION_STATE,
						BTHH_CONN_STATE_DISCONNECTED),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("HidHost Disconnect Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_sdp_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_cc_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_ic_data),
		ACTION_SUCCESS(hidhost_connect_action, NULL),
		CALLBACK_STATE(CB_HH_CONNECTION_STATE,
						BTHH_CONN_STATE_CONNECTED),
		ACTION_SUCCESS(hidhost_disconnect_action, NULL),
		CALLBACK_STATE(CB_HH_CONNECTION_STATE,
						BTHH_CONN_STATE_DISCONNECTED),
	),
	TEST_CASE_BREDRLE("HidHost VirtualUnplug Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_sdp_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_cc_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_ic_data),
		ACTION_SUCCESS(hidhost_connect_action, NULL),
		CALLBACK_STATE(CB_HH_CONNECTION_STATE,
						BTHH_CONN_STATE_CONNECTED),
		ACTION_SUCCESS(hidhost_virtual_unplug_action, NULL),
		CALLBACK_STATE(CB_HH_CONNECTION_STATE,
						BTHH_CONN_STATE_DISCONNECTED),
	),
	TEST_CASE_BREDRLE("HidHost GetProtocol Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_sdp_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_cc_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_ic_data),
		ACTION_SUCCESS(hidhost_connect_action, NULL),
		CALLBACK_STATE(CB_HH_CONNECTION_STATE,
						BTHH_CONN_STATE_CONNECTED),
		ACTION_SUCCESS(hidhost_get_protocol_action, NULL),
		CALLBACK_HH_MODE(CB_HH_PROTOCOL_MODE, BTHH_OK, HID_MODE_BREDR),
	),
	TEST_CASE_BREDRLE("HidHost SetProtocol Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_sdp_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_cc_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_ic_data),
		ACTION_SUCCESS(hidhost_connect_action, NULL),
		CALLBACK_STATE(CB_HH_CONNECTION_STATE,
						BTHH_CONN_STATE_CONNECTED),
		ACTION_SUCCESS(hidhost_set_protocol_action, NULL),
		CALLBACK_HH_MODE(CB_HH_PROTOCOL_MODE, BTHH_OK, HID_MODE_BREDR),
	),
	TEST_CASE_BREDRLE("HidHost GetReport Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_sdp_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_cc_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_ic_data),
		ACTION_SUCCESS(hidhost_connect_action, NULL),
		CALLBACK_STATE(CB_HH_CONNECTION_STATE,
						BTHH_CONN_STATE_CONNECTED),
		ACTION_SUCCESS(hidhost_get_report_action, NULL),
		CALLBACK_HHREPORT(CB_HH_GET_REPORT, BTHH_OK,
						HID_EXPECTED_REPORT_SIZE),
	),
	TEST_CASE_BREDRLE("HidHost SetReport Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_sdp_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_cc_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_ic_data),
		ACTION_SUCCESS(hidhost_connect_action, NULL),
		CALLBACK_STATE(CB_HH_CONNECTION_STATE,
						BTHH_CONN_STATE_CONNECTING),
		CALLBACK_STATE(CB_HH_CONNECTION_STATE,
						BTHH_CONN_STATE_CONNECTED),
		ACTION_SUCCESS(hidhost_set_report_action, NULL),
		CALLBACK(CB_EMU_CONFIRM_SEND_DATA),
	),
	TEST_CASE_BREDRLE("HidHost SendData Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_sdp_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_cc_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_ic_data),
		ACTION_SUCCESS(hidhost_connect_action, NULL),
		CALLBACK_STATE(CB_HH_CONNECTION_STATE,
						BTHH_CONN_STATE_CONNECTED),
		ACTION_SUCCESS(hidhost_send_data_action, NULL),
		CALLBACK(CB_EMU_CONFIRM_SEND_DATA),
	),
	TEST_CASE_BREDRLE("HidHost Connect Encrypted Success",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_set_ssp_mode_action, NULL),
		ACTION_SUCCESS(set_default_ssp_request_handler, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
						&l2cap_setup_kb_sdp_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_cc_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_ic_data),
		ACTION_SUCCESS(hidhost_connect_action, NULL),
		CALLBACK(CB_EMU_ENCRYPTION_ENABLED),
		CALLBACK_STATE(CB_HH_CONNECTION_STATE,
						BTHH_CONN_STATE_CONNECTED),
		ACTION_SUCCESS(hidhost_send_data_action, NULL),
		ACTION_SUCCESS(bluetooth_disable_action, NULL),
		CALLBACK_STATE(CB_HH_CONNECTION_STATE,
						BTHH_CONN_STATE_DISCONNECTED),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_OFF),
	),
	TEST_CASE_BREDRLE("HidHost Reject Unknown Remote Connection",
		ACTION_SUCCESS(bluetooth_enable_action, NULL),
		CALLBACK_STATE(CB_BT_ADAPTER_STATE_CHANGED, BT_STATE_ON),
		ACTION_SUCCESS(bt_set_property_action,
						&prop_test_scan_mode_conn),
		CALLBACK_ADAPTER_PROPS(&prop_test_scan_mode_conn, 1),
		ACTION_SUCCESS(emu_setup_powered_remote_action, NULL),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
						&l2cap_setup_kb_sdp_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_cc_data),
		ACTION_SUCCESS(emu_add_l2cap_server_action,
							&l2cap_setup_ic_data),
		/* Trigger incoming connection */
		ACTION_SUCCESS(emu_set_connect_cb_action, hidhost_conn_cb),
		ACTION_SUCCESS(emu_remote_connect_hci_action, NULL),
		CALLBACK(CB_EMU_CONNECTION_REJECTED),
	),
};

struct queue *get_hidhost_tests(void)
{
	uint16_t i = 0;

	list = queue_new();

	for (; i < sizeof(test_cases) / sizeof(test_cases[0]); ++i)
		queue_push_tail(list, &test_cases[i]);

	return list;
}

void remove_hidhost_tests(void)
{
	queue_destroy(list, NULL);
}

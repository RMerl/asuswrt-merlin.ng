/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"
#include "src/sdp-client.h"
#include "src/shared/hfp.h"
#include "src/shared/queue.h"
#include "src/shared/util.h"
#include "btio/btio.h"
#include "ipc.h"
#include "ipc-common.h"
#include "src/log.h"
#include "utils.h"

#include "bluetooth.h"
#include "hal-msg.h"
#include "handsfree-client.h"
#include "sco.h"

#define HFP_HF_CHANNEL 7

#define HFP_HF_FEAT_ECNR	0x00000001
#define HFP_HF_FEAT_3WAY	0x00000002
#define HFP_HF_FEAT_CLI		0x00000004
#define HFP_HF_FEAT_VR		0x00000008
#define HFP_HF_FEAT_RVC		0x00000010
#define HFP_HF_FEAT_ECS		0x00000020
#define HFP_HF_FEAT_ECC		0x00000040
#define HFP_HF_FEAT_CODEC	0x00000080
#define HFP_HF_FEAT_HF_IND	0x00000100
#define HFP_HF_FEAT_ESCO_S4_T2	0x00000200

#define HFP_AG_FEAT_3WAY	0x00000001
#define HFP_AG_FEAT_ECNR	0x00000002
#define HFP_AG_FEAT_VR		0x00000004
#define HFP_AG_FEAT_INBAND	0x00000008
#define HFP_AG_FEAT_VTAG	0x00000010
#define HFP_AG_FEAT_REJ_CALL	0x00000020
#define HFP_AG_FEAT_ECS		0x00000040
#define HFP_AG_FEAT_ECC		0x00000080
#define HFP_AG_FEAT_EXT_ERR	0x00000100
#define HFP_AG_FEAT_CODEC	0x00000200

#define HFP_HF_FEATURES (HFP_HF_FEAT_ECNR | HFP_HF_FEAT_3WAY |\
				HFP_HF_FEAT_CLI | HFP_HF_FEAT_VR |\
				HFP_HF_FEAT_RVC | HFP_HF_FEAT_ECS |\
				HFP_HF_FEAT_ECC)

#define CVSD_OFFSET 0
#define MSBC_OFFSET 1
#define CODECS_COUNT (MSBC_OFFSET + 1)

#define CODEC_ID_CVSD 0x01
#define CODEC_ID_MSBC 0x02

#define MAX_NUMBER_LEN 33
#define MAX_OPERATOR_NAME_LEN 17

enum hfp_indicator {
	HFP_INDICATOR_SERVICE = 0,
	HFP_INDICATOR_CALL,
	HFP_INDICATOR_CALLSETUP,
	HFP_INDICATOR_CALLHELD,
	HFP_INDICATOR_SIGNAL,
	HFP_INDICATOR_ROAM,
	HFP_INDICATOR_BATTCHG,
	HFP_INDICATOR_LAST
};

typedef void (*ciev_func_t)(uint8_t val);

struct indicator {
	uint8_t index;
	uint32_t min;
	uint32_t max;
	uint32_t val;
	ciev_func_t cb;
};

struct hfp_codec {
	uint8_t type;
	bool local_supported;
	bool remote_supported;
};

struct device {
	bdaddr_t bdaddr;
	struct hfp_hf *hf;
	uint8_t state;
	uint8_t audio_state;

	uint8_t negotiated_codec;
	uint32_t features;
	struct hfp_codec codecs[2];

	struct indicator ag_ind[HFP_INDICATOR_LAST];

	uint32_t chld_features;
};

static const struct hfp_codec codecs_defaults[] = {
	{ CODEC_ID_CVSD, true, false},
	{ CODEC_ID_MSBC, false, false},
};

static bdaddr_t adapter_addr;

static struct ipc *hal_ipc = NULL;

static uint32_t hfp_hf_features = 0;
static uint32_t hfp_hf_record_id = 0;
static struct queue *devices = NULL;
static GIOChannel *hfp_hf_server = NULL;

static struct bt_sco *sco = NULL;

static struct device *find_default_device(void)
{
	return queue_peek_head(devices);
}

static bool match_by_bdaddr(const void *data, const void *user_data)
{
	const bdaddr_t *addr1 = data;
	const bdaddr_t *addr2 = user_data;

	return !bacmp(addr1, addr2);
}

static struct device *find_device(const bdaddr_t *addr)
{
	return queue_find(devices, match_by_bdaddr, addr);
}

static void init_codecs(struct device *dev)
{
	memcpy(&dev->codecs, codecs_defaults, sizeof(dev->codecs));

	if (hfp_hf_features & HFP_HF_FEAT_CODEC)
		dev->codecs[MSBC_OFFSET].local_supported = true;
}

static struct device *device_create(const bdaddr_t *bdaddr)
{
	struct device *dev;

	dev = new0(struct device, 1);

	bacpy(&dev->bdaddr, bdaddr);
	dev->state = HAL_HF_CLIENT_CONN_STATE_DISCONNECTED;
	dev->audio_state = HAL_HF_CLIENT_AUDIO_STATE_DISCONNECTED;

	init_codecs(dev);

	queue_push_tail(devices, dev);

	return dev;
}

static struct device *get_device(const bdaddr_t *addr)
{
	struct device *dev;

	dev = find_device(addr);
	if (dev)
		return dev;

	/* We do support only one device as for now */
	if (queue_isempty(devices))
		return device_create(addr);

	return NULL;
}

static void device_set_state(struct device *dev, uint8_t state)
{
	struct hal_ev_hf_client_conn_state ev;
	char address[18];

	if (dev->state == state)
		return;

	memset(&ev, 0, sizeof(ev));

	dev->state = state;

	ba2str(&dev->bdaddr, address);
	DBG("device %s state %u", address, state);

	bdaddr2android(&dev->bdaddr, ev.bdaddr);
	ev.state = state;

	ev.chld_feat = dev->chld_features;
	ev.peer_feat = dev->features;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
				HAL_EV_HF_CLIENT_CONN_STATE, sizeof(ev), &ev);
}

static void device_destroy(struct device *dev)
{
	device_set_state(dev, HAL_HF_CLIENT_CONN_STATE_DISCONNECTED);
	queue_remove(devices, dev);

	if (dev->hf)
		hfp_hf_unref(dev->hf);

	free(dev);
}

static void handle_disconnect(const void *buf, uint16_t len)
{
	const struct hal_cmd_hf_client_disconnect *cmd = buf;
	struct device *dev;
	uint32_t status;
	bdaddr_t bdaddr;
	char addr[18];

	DBG("");

	android2bdaddr(&cmd->bdaddr, &bdaddr);

	ba2str(&bdaddr, addr);
	DBG("Disconnect %s", addr);

	dev = get_device(&bdaddr);
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	if (dev->state == HAL_HF_CLIENT_CONN_STATE_DISCONNECTED) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	if (dev->state == HAL_HF_CLIENT_CONN_STATE_DISCONNECTING) {
		status = HAL_STATUS_SUCCESS;
		goto done;
	}

	if (dev->state == HAL_HF_CLIENT_CONN_STATE_CONNECTING) {
		device_destroy(dev);
		status = HAL_STATUS_SUCCESS;
		goto done;
	}

	status = hfp_hf_disconnect(dev->hf) ? HAL_STATUS_SUCCESS :
							HAL_STATUS_FAILED;

	if (status)
		device_set_state(dev, HAL_HF_CLIENT_CONN_STATE_DISCONNECTING);

done:

	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_DISCONNECT, status);
}

static void set_audio_state(struct device *dev, uint8_t state)
{
	struct hal_ev_hf_client_audio_state ev;
	char address[18];

	if (dev->audio_state == state)
		return;

	dev->audio_state = state;

	ba2str(&dev->bdaddr, address);
	DBG("device %s audio state %u", address, state);

	bdaddr2android(&dev->bdaddr, ev.bdaddr);
	ev.state = state;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
				HAL_EV_HF_CLIENT_AUDIO_STATE, sizeof(ev), &ev);
}

static void bcc_cb(enum hfp_result result, enum hfp_error cme_err,
							void *user_data)
{
	struct device *dev = user_data;

	if (result != HFP_RESULT_OK)
		set_audio_state(dev, HAL_HF_CLIENT_AUDIO_STATE_DISCONNECTED);
}

static bool codec_negotiation_supported(struct device *dev)
{
	return (dev->features & HFP_AG_FEAT_CODEC) &&
			(hfp_hf_features & HFP_HF_FEAT_CODEC);
}

static bool connect_sco(struct device *dev)
{
	if (codec_negotiation_supported(dev))
		return hfp_hf_send_command(dev->hf, bcc_cb, dev,
								"AT+BCC");

	return bt_sco_connect(sco, &dev->bdaddr, BT_VOICE_CVSD_16BIT);
}

static void handle_connect_audio(const void *buf, uint16_t len)
{
	const struct hal_cmd_hf_client_connect_audio *cmd = (void *) buf;
	struct device *dev;
	uint8_t status;
	bdaddr_t bdaddr;

	DBG("");

	android2bdaddr(&cmd->bdaddr, &bdaddr);

	dev = find_device(&bdaddr);
	if (!dev || dev->state != HAL_HF_CLIENT_CONN_STATE_SLC_CONNECTED ||
		dev->audio_state != HAL_HF_CLIENT_AUDIO_STATE_DISCONNECTED) {
		error("hf-client: Cannot create SCO, check SLC or audio state");
		status = HAL_STATUS_FAILED;
		goto done;
	}

	if (connect_sco(dev)) {
		status = HAL_STATUS_SUCCESS;
		set_audio_state(dev, HAL_HF_CLIENT_AUDIO_STATE_CONNECTING);
	} else {
		status = HAL_STATUS_FAILED;
	}

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
					HAL_OP_HF_CLIENT_CONNECT_AUDIO, status);
}

static void handle_disconnect_audio(const void *buf, uint16_t len)
{
	const struct hal_cmd_hf_client_disconnect_audio *cmd = (void *) buf;
	struct device *dev;
	uint8_t status;
	bdaddr_t bdaddr;

	DBG("");

	android2bdaddr(&cmd->bdaddr, &bdaddr);

	dev = find_device(&bdaddr);
	if (!dev ||
		dev->audio_state == HAL_HF_CLIENT_AUDIO_STATE_DISCONNECTED) {
		error("hf-client: Device not found or audio not connected");
		status = HAL_STATUS_FAILED;
		goto done;
	}

	bt_sco_disconnect(sco);
	status = HAL_STATUS_SUCCESS;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
				HAL_OP_HF_CLIENT_DISCONNECT_AUDIO, status);
}

static void cmd_complete_cb(enum hfp_result result, enum hfp_error cme_err,
							void *user_data)
{
	struct hal_ev_hf_client_command_complete ev;

	DBG("");
	memset(&ev, 0, sizeof(ev));

	switch (result) {
	case HFP_RESULT_OK:
		ev.type = HAL_HF_CLIENT_CMD_COMP_OK;
		break;
	case HFP_RESULT_NO_CARRIER:
		ev.type = HAL_HF_CLIENT_CMD_COMP_ERR_NO_CARRIER;
		break;
	case HFP_RESULT_ERROR:
		ev.type = HAL_HF_CLIENT_CMD_COMP_ERR;
		break;
	case HFP_RESULT_BUSY:
		ev.type = HAL_HF_CLIENT_CMD_COMP_ERR_BUSY;
		break;
	case HFP_RESULT_NO_ANSWER:
		ev.type = HAL_HF_CLIENT_CMD_COMP_ERR_NO_ANSWER;
		break;
	case HFP_RESULT_DELAYED:
		ev.type = HAL_HF_CLIENT_CMD_COMP_ERR_DELAYED;
		break;
	case HFP_RESULT_BLACKLISTED:
		ev.type = HAL_HF_CLIENT_CMD_COMP_ERR_BACKLISTED;
		break;
	case HFP_RESULT_CME_ERROR:
		ev.type = HAL_HF_CLIENT_CMD_COMP_ERR_CME;
		ev.cme = cme_err;
		break;
	case HFP_RESULT_CONNECT:
	case HFP_RESULT_RING:
	case HFP_RESULT_NO_DIALTONE:
	default:
		error("hf-client: Unknown error code %d", result);
		ev.type = HAL_HF_CLIENT_CMD_COMP_ERR;
		break;
	}

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_EV_CLIENT_COMMAND_COMPLETE, sizeof(ev), &ev);
}

static void handle_start_vr(const void *buf, uint16_t len)
{
	struct device *dev;
	uint8_t status;

	DBG("");

	dev = find_default_device();
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	if (hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL, "AT+BVRA=1"))
		status = HAL_STATUS_SUCCESS;
	else
		status = HAL_STATUS_FAILED;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_START_VR, status);
}

static void handle_stop_vr(const void *buf, uint16_t len)
{
	struct device *dev;
	uint8_t status;

	DBG("");

	dev = find_default_device();
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	if (hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL, "AT+BVRA=0"))
		status = HAL_STATUS_SUCCESS;
	else
		status = HAL_STATUS_FAILED;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_STOP_VR, status);
}

static void handle_volume_control(const void *buf, uint16_t len)
{
	const struct hal_cmd_hf_client_volume_control *cmd = buf;
	struct device *dev;
	uint8_t status;
	uint8_t vol;
	bool ret;

	DBG("");

	dev = find_default_device();
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	/*
	 * Volume is in the range 0-15. Make sure we send correct value
	 * to remote device
	 */
	vol = cmd->volume > 15 ? 15 : cmd->volume;

	switch (cmd->type) {
	case HF_CLIENT_VOLUME_TYPE_SPEAKER:
		ret = hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL,
							"AT+VGS=%u", vol);
		break;
	case HF_CLIENT_VOLUME_TYPE_MIC:
		ret = hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL,
							"AT+VGM=%u", vol);
		break;
	default:
		ret = false;
		break;
	}

	status = ret ? HAL_STATUS_SUCCESS : HAL_STATUS_FAILED;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
					HAL_OP_HF_CLIENT_VOLUME_CONTROL,
					status);
}

static void handle_dial(const void *buf, uint16_t len)
{
	const struct hal_cmd_hf_client_dial *cmd = buf;
	struct device *dev;
	uint8_t status;
	bool ret;

	DBG("");

	if (len != sizeof(*cmd) + cmd->number_len)
		goto failed;

	dev = find_default_device();
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	if (cmd->number_len > 0) {
		if (cmd->number[cmd->number_len - 1] != '\0')
			goto failed;

		DBG("Dialing %s", cmd->number);

		ret = hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL,
							"ATD%s;", cmd->number);
	} else {
		DBG("Redialing");

		ret = hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL,
								"AT+BLDN");
	}

	status =  ret ? HAL_STATUS_SUCCESS : HAL_STATUS_FAILED;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
						HAL_OP_HF_CLIENT_DIAL, status);

	return;

failed:
	error("Malformed number data, size (%u bytes), terminating", len);
	raise(SIGTERM);
}

static void handle_dial_memory(const void *buf, uint16_t len)
{
	const struct hal_cmd_hf_client_dial_memory *cmd = buf;
	struct device *dev;
	uint8_t status;

	DBG("");

	dev = find_default_device();
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	/* For some reason location in BT HAL is int. Therefore that check */
	if (cmd->location < 0) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	if (hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL , "ATD>%d;",
								cmd->location))
		status = HAL_STATUS_SUCCESS;
	else
		status = HAL_STATUS_FAILED;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
					HAL_OP_HF_CLIENT_DIAL_MEMORY, status);
}

static void handle_call_action(const void *buf, uint16_t len)
{
	const struct hal_cmd_hf_client_call_action *cmd = buf;
	struct device *dev;
	uint8_t status;
	bool ret;

	DBG("");

	dev = find_default_device();
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	switch (cmd->action) {
	case HAL_HF_CLIENT_ACTION_CHLD_0:
		ret = hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL,
								"AT+CHLD=0");
		break;
	case HAL_HF_CLIENT_ACTION_CHLD_1:
		ret = hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL,
								"AT+CHLD=1");
		break;
	case HAL_HF_CLIENT_ACTION_CHLD_2:
		ret = hfp_hf_send_command(dev->hf, cmd_complete_cb,
							NULL, "AT+CHLD=2");
		break;
	case HAL_HF_CLIENT_ACTION_CHLD_3:
		ret = hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL,
								"AT+CHLD=3");
		break;
	case HAL_HF_CLIENT_ACTION_CHLD_4:
		ret = hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL,
								"AT+CHLD=4");
		break;
	case HAL_HF_CLIENT_ACTION_CHLD_1x:
		/* Index is int in BT HAL. Let's be paranoid here */
		if (cmd->index <= 0)
			ret = false;
		else
			ret = hfp_hf_send_command(dev->hf, cmd_complete_cb,
					NULL, "AT+CHLD=1%d", cmd->index);
		break;
	case HAL_HF_CLIENT_ACTION_CHLD_2x:
		/* Index is int in BT HAL. Let's be paranoid here */
		if (cmd->index <= 0)
			ret = false;
		else
			ret = hfp_hf_send_command(dev->hf, cmd_complete_cb,
					NULL, "AT+CHLD=2%d", cmd->index);
		break;
	case HAL_HF_CLIENT_ACTION_ATA:
		ret = hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL,
									"ATA");
		break;
	case HAL_HF_CLIENT_ACTION_CHUP:
		ret = hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL,
								"AT+CHUP");
		break;
	case HAL_HF_CLIENT_ACTION_BRTH_0:
		ret = hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL,
								"AT+BTRH=0");
		break;
	case HAL_HF_CLIENT_ACTION_BRTH_1:
		ret = hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL,
								"AT+BTRH=1");
		break;
	case HAL_HF_CLIENT_ACTION_BRTH_2:
		ret = hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL,
								"AT+BTRH=2");
		break;
	default:
		error("hf-client: Unknown action %d", cmd->action);
		ret = false;
		break;
	}

	status = ret ? HAL_STATUS_SUCCESS : HAL_STATUS_FAILED;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
					HAL_OP_HF_CLIENT_CALL_ACTION, status);
}

static void handle_query_current_calls(const void *buf, uint16_t len)
{
	struct device *dev;
	uint8_t status;

	DBG("");

	dev = find_default_device();
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	if (hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL, "AT+CLCC"))
		status = HAL_STATUS_SUCCESS;
	else
		status = HAL_STATUS_FAILED;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
					HAL_OP_HF_CLIENT_QUERY_CURRENT_CALLS,
					status);
}

static void handle_query_operator_name(const void *buf, uint16_t len)
{
	struct device *dev;
	uint8_t status;

	DBG("");

	dev = find_default_device();
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	if (hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL, "AT+COPS?"))
		status = HAL_STATUS_SUCCESS;
	else
		status = HAL_STATUS_FAILED;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
					HAL_OP_HF_CLIENT_QUERY_OPERATOR_NAME,
					status);
}

static void handle_retrieve_subscr_info(const void *buf, uint16_t len)
{
	struct device *dev;
	uint8_t status;

	DBG("");

	dev = find_default_device();
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	if (hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL, "AT+CNUM"))
		status = HAL_STATUS_SUCCESS;
	else
		status = HAL_STATUS_FAILED;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
					HAL_OP_HF_CLIENT_RETRIEVE_SUBSCR_INFO,
					status);
}

static void handle_send_dtmf(const void *buf, uint16_t len)
{
	const struct hal_cmd_hf_client_send_dtmf *cmd = buf;
	struct device *dev;
	uint8_t status;

	dev = find_default_device();
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	if (hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL, "AT+VTS=%c",
							(char) cmd->tone))
		status = HAL_STATUS_SUCCESS;
	else
		status = HAL_STATUS_FAILED;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
					HAL_OP_HF_CLIENT_SEND_DTMF, status);
}

static void handle_get_last_vc_tag_num(const void *buf, uint16_t len)
{
	struct device *dev;
	uint8_t status;

	dev = find_default_device();
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	if (hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL, "AT+BINP=1"))
		status = HAL_STATUS_SUCCESS;
	else
		status = HAL_STATUS_FAILED;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_OP_HF_CLIENT_GET_LAST_VOICE_TAG_NUM, status);
}

static void disconnect_watch(void *user_data)
{
	DBG("");

	device_destroy(user_data);
}

static void slc_error(struct device *dev)
{
	error("hf-client: Could not create SLC - dropping connection");
	hfp_hf_disconnect(dev->hf);
}

static void set_chld_feat(struct device *dev, char *feat)
{
	DBG(" %s", feat);

	if (strcmp(feat, "0") == 0)
		dev->chld_features |= HAL_HF_CLIENT_CHLD_FEAT_REL;
	else if (strcmp(feat, "1") == 0)
		dev->chld_features |= HAL_HF_CLIENT_CHLD_FEAT_REL_ACC;
	else if (strcmp(feat, "1x") == 0)
		dev->chld_features |= HAL_HF_CLIENT_CHLD_FEAT_REL_X;
	else if (strcmp(feat, "2") == 0)
		dev->chld_features |= HAL_HF_CLIENT_CHLD_FEAT_HOLD_ACC;
	else if (strcmp(feat, "2x") == 0)
		dev->chld_features |= HAL_HF_CLIENT_CHLD_FEAT_PRIV_X;
	else if (strcmp(feat, "3") == 0)
		dev->chld_features |= HAL_HF_CLIENT_CHLD_FEAT_MERGE;
	else if (strcmp(feat, "4") == 0)
		dev->chld_features |= HAL_HF_CLIENT_CHLD_FEAT_MERGE_DETACH;
}

static void get_local_codecs_string(struct device *dev, char *buf,
								uint8_t len)
{
	int i;
	uint8_t offset;

	memset(buf, 0, len);
	offset = 0;

	for (i = 0; i < CODECS_COUNT; i++) {
		char c[8];
		int l;

		if (!dev->codecs[i].local_supported)
			continue;

		memset(c, 0, sizeof(c));

		l = sprintf(c, "%d,", dev->codecs[i].type);

		if (l > (len - offset - 1)) {
			error("hf-client: Codecs cannot fit into buffer");
			return;
		}

		strcat(&buf[offset], c);
		offset += l;
	}
}

static void bvra_cb(struct hfp_context *context, void *user_data)
{
	struct hal_ev_hf_client_vr_state ev;
	unsigned int val;

	if (!hfp_context_get_number(context, &val) || val > 1)
		return;

	ev.state = val ? HAL_HF_CLIENT_VR_STARTED : HAL_HF_CLIENT_VR_STOPPED;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
				HAL_EV_HF_CLIENT_VR_STATE, sizeof(ev), &ev);
}

static void vgm_cb(struct hfp_context *context, void *user_data)
{
	struct hal_ev_hf_client_volume_changed ev;
	unsigned int val;

	if (!hfp_context_get_number(context, &val) || val > 15)
		return;

	ev.type = HF_CLIENT_VOLUME_TYPE_MIC;
	ev.volume = val;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
				HAL_EV_HF_CLIENT_VR_STATE, sizeof(ev), &ev);
}

static void vgs_cb(struct hfp_context *context, void *user_data)
{
	struct hal_ev_hf_client_volume_changed ev;
	unsigned int val;

	if (!hfp_context_get_number(context, &val) || val > 15)
		return;

	ev.type = HF_CLIENT_VOLUME_TYPE_SPEAKER;
	ev.volume = val;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_EV_CLIENT_VOLUME_CHANGED, sizeof(ev), &ev);
}

static void brth_cb(struct hfp_context *context, void *user_data)
{
	struct hal_ev_hf_client_response_and_hold_status ev;
	unsigned int val;

	DBG("");

	if (!hfp_context_get_number(context, &val) ||
			val > HAL_HF_CLIENT_RESP_AND_HOLD_STATUS_REJECT) {
		error("hf-client: incorrect BTRH response ");
		return;
	}

	ev.status = val;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
				HAL_EV_HF_CLIENT_RESPONSE_AND_HOLD_STATUS,
				sizeof(ev), &ev);
}

static void clcc_cb(struct hfp_context *context, void *user_data)
{
	uint8_t buf[IPC_MTU];
	struct hal_ev_hf_client_current_call *ev = (void *) buf;
	unsigned int val;

	DBG("");

	memset(buf, 0, sizeof(buf));

	if (!hfp_context_get_number(context, &val)) {
		error("hf-client: Could not get index");
		return;
	}

	ev->index = val;

	if (!hfp_context_get_number(context, &val) ||
				val > HAL_HF_CLIENT_DIRECTION_INCOMING) {
		error("hf-client: Could not get direction");
		return;
	}

	ev->direction = val;

	if (!hfp_context_get_number(context, &val) ||
			val > HAL_HF_CLIENT_CALL_STATE_HELD_BY_RESP_AND_HOLD) {
		error("hf-client: Could not get callstate");
		return;
	}

	ev->call_state = val;

	/* Next field is MODE but Android is not interested in this. Skip it */
	if (!hfp_context_get_number(context, &val)) {
		error("hf-client: Could not get mode");
		return;
	}

	if (!hfp_context_get_number(context, &val) || val > 1) {
		error("hf-client: Could not get multiparty");
		return;
	}

	ev->multiparty = val;

	if (hfp_context_get_string(context, (char *) &ev->number[0],
								MAX_NUMBER_LEN))
		ev->number_len = strlen((char *) ev->number) + 1;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
					HAL_EV_HF_CLIENT_CURRENT_CALL,
					sizeof(*ev) + ev->number_len, ev);
}

static void ciev_cb(struct hfp_context *context, void *user_data)
{
	struct device *dev = user_data;
	unsigned int index, val;
	int i;

	DBG("");

	if (!hfp_context_get_number(context, &index))
		return;

	if (!hfp_context_get_number(context, &val))
		return;

	for (i = 0; i < HFP_INDICATOR_LAST; i++) {
		if (dev->ag_ind[i].index != index)
			continue;

		if (dev->ag_ind[i].cb) {
			dev->ag_ind[i].val = val;
			dev->ag_ind[i].cb(val);
			return;
		}
	}
}

static void cnum_cb(struct hfp_context *context, void *user_data)
{
	uint8_t buf[IPC_MTU];
	struct hal_ev_hf_client_subscriber_service_info *ev = (void *) buf;
	unsigned int service;

	DBG("");

	/* Alpha field is empty string, just skip it */
	hfp_context_skip_field(context);

	if (!hfp_context_get_string(context, (char *) &ev->name[0],
							MAX_NUMBER_LEN)) {
		error("hf-client: Could not get number");
		return;
	}

	ev->name_len = strlen((char *) &ev->name[0]) + 1;

	/* Type is not used in Android */
	hfp_context_skip_field(context);

	/* Speed field is empty string, just skip it */
	hfp_context_skip_field(context);

	if (!hfp_context_get_number(context, &service))
		return;

	switch (service) {
	case 4:
		ev->type = HAL_HF_CLIENT_SUBSCR_TYPE_VOICE;
		break;
	case 5:
		ev->type = HAL_HF_CLIENT_SUBSCR_TYPE_FAX;
		break;
	default:
		ev->type = HAL_HF_CLIENT_SUBSCR_TYPE_UNKNOWN;
		break;
	}

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
					HAL_EV_CLIENT_SUBSCRIBER_SERVICE_INFO,
					sizeof(*ev) + ev->name_len, ev);
}

static void cops_cb(struct hfp_context *context, void *user_data)
{
	uint8_t buf[IPC_MTU];
	struct hal_ev_hf_client_operator_name *ev = (void *) buf;
	unsigned int format;

	DBG("");

	/* Not interested in mode */
	hfp_context_skip_field(context);

	if (!hfp_context_get_number(context, &format))
		return;

	if (format != 0)
		info("hf-client: Not correct string format in +COSP");

	if (!hfp_context_get_string(context, (char *) &ev->name[0],
						MAX_OPERATOR_NAME_LEN)) {
		error("hf-client: incorrect COPS response");
		return;
	}

	ev->name_len = strlen((char *) &ev->name[0]) + 1;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
					HAL_EV_HF_CLIENT_OPERATOR_NAME,
					sizeof(*ev) + ev->name_len, ev);
}

static void binp_cb(struct hfp_context *context, void *user_data)
{
	uint8_t buf[IPC_MTU];
	struct hal_ev_hf_client_last_void_call_tag_num *ev = (void *) buf;
	char number[33];

	DBG("");

	if (!hfp_context_get_string(context, number, sizeof(number))) {
		error("hf-client: incorrect COPS response");
		return;
	}

	ev->number_len = strlen(number) + 1;
	memcpy(ev->number, number, ev->number_len);

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
					HAL_EV_CLIENT_LAST_VOICE_CALL_TAG_NUM,
					sizeof(*ev) + ev->number_len, ev);
}

static bool is_codec_supported_localy(struct device *dev, uint8_t codec)
{
	int i;

	for (i = 0; i < CODECS_COUNT; i++) {
		if (dev->codecs[i].type != codec)
			continue;

		return dev->codecs[i].local_supported;
	}

	return false;
}

static void bcs_resp(enum hfp_result result, enum hfp_error cme_err,
							void *user_data)
{
	if (result != HFP_RESULT_OK)
		error("hf-client: Error on AT+BCS (err=%u)", result);
}

static void bcs_cb(struct hfp_context *context, void *user_data)
{
	struct device *dev = user_data;
	unsigned int codec;
	char codecs_string[8];

	DBG("");

	if (!hfp_context_get_number(context, &codec))
		goto failed;

	if (!is_codec_supported_localy(dev, codec))
		goto failed;

	dev->negotiated_codec = codec;

	hfp_hf_send_command(dev->hf, bcs_resp, dev, "AT+BCS=%u", codec);

	return;

failed:
	error("hf-client: Could not get codec");

	get_local_codecs_string(dev, codecs_string, sizeof(codecs_string));

	hfp_hf_send_command(dev->hf, bcs_resp, dev, "AT+BCS=%s", codecs_string);
}

static void slc_completed(struct device *dev)
{
	int i;
	struct indicator *ag_ind;

	DBG("");

	ag_ind = dev->ag_ind;

	device_set_state(dev, HAL_HF_CLIENT_CONN_STATE_SLC_CONNECTED);

	/* Notify Android with indicators */
	for (i = 0; i < HFP_INDICATOR_LAST; i++) {
		if (!ag_ind[i].cb)
			continue;

		ag_ind[i].cb(ag_ind[i].val);
	}

	/* TODO: register unsolicited results handlers */

	hfp_hf_register(dev->hf, bvra_cb, "+BRVA", dev, NULL);
	hfp_hf_register(dev->hf, vgm_cb, "+VGM", dev, NULL);
	hfp_hf_register(dev->hf, vgs_cb, "+VGS", dev, NULL);
	hfp_hf_register(dev->hf, brth_cb, "+BTRH", dev, NULL);
	hfp_hf_register(dev->hf, clcc_cb, "+CLCC", dev, NULL);
	hfp_hf_register(dev->hf, ciev_cb, "+CIEV", dev, NULL);
	hfp_hf_register(dev->hf, cops_cb, "+COPS", dev, NULL);
	hfp_hf_register(dev->hf, cnum_cb, "+CNUM", dev, NULL);
	hfp_hf_register(dev->hf, binp_cb, "+BINP", dev, NULL);
	hfp_hf_register(dev->hf, bcs_cb, "+BCS", dev, NULL);

	if (!hfp_hf_send_command(dev->hf, cmd_complete_cb, NULL, "AT+COPS=3,0"))
		info("hf-client: Could not send AT+COPS=3,0");
}

static void slc_chld_cb(struct hfp_context *context, void *user_data)
{
	struct device *dev = user_data;
	char feat[3];

	if (!hfp_context_open_container(context))
		goto failed;

	while (hfp_context_get_unquoted_string(context, feat, sizeof(feat)))
		set_chld_feat(dev, feat);

	if (!hfp_context_close_container(context))
		goto failed;

	return;

failed:
	error("hf-client: Error on CHLD response");
	slc_error(dev);
}

static void slc_chld_resp(enum hfp_result result, enum hfp_error cme_err,
							void *user_data)
{
	struct device *dev = user_data;

	DBG("");

	hfp_hf_unregister(dev->hf, "+CHLD");

	if (result != HFP_RESULT_OK) {
		error("hf-client: CHLD error: %d", result);
		slc_error(dev);
		return;
	}

	slc_completed(dev);
}

static void slc_cmer_resp(enum hfp_result result, enum hfp_error cme_err,
							void *user_data)
{
	struct device *dev = user_data;

	DBG("");

	if (result != HFP_RESULT_OK) {
		error("hf-client: CMER error: %d", result);
		goto failed;
	}

	/* Continue with SLC creation */
	if (!(dev->features & HFP_AG_FEAT_3WAY)) {
		slc_completed(dev);
		return;
	}

	if (!hfp_hf_register(dev->hf, slc_chld_cb, "+CHLD", dev, NULL)) {
		error("hf-client: Could not register +CHLD");
		goto failed;
	}

	if (!hfp_hf_send_command(dev->hf, slc_chld_resp, dev, "AT+CHLD=?")) {
		error("hf-client: Could not send AT+CHLD");
		goto failed;
	}

	return;

failed:
	slc_error(dev);
}

static void set_indicator_value(uint8_t index, unsigned int val,
						struct indicator *ag_ind)
{
	int i;

	for (i = 0; i < HFP_INDICATOR_LAST; i++) {
		if (index != ag_ind[i].index)
			continue;

		ag_ind[i].val = val;
		ag_ind[i].cb(val);
		return;
	}
}

static void slc_cind_status_cb(struct hfp_context *context,
							void *user_data)
{
	struct device *dev = user_data;
	uint8_t index = 1;

	DBG("");

	while (hfp_context_has_next(context)) {
		uint32_t val;

		if (!hfp_context_get_number(context, &val)) {
			error("hf-client: Error on CIND status response");
			return;
		}

		set_indicator_value(index++, val, dev->ag_ind);
	}
}

static void slc_cind_status_resp(enum hfp_result result,
							enum hfp_error cme_err,
							void *user_data)
{
	struct device *dev = user_data;

	DBG("");

	hfp_hf_unregister(dev->hf, "+CIND");

	if (result != HFP_RESULT_OK) {
		error("hf-client: CIND error: %d", result);
		goto failed;
	}

	/* Continue with SLC creation */
	if (!hfp_hf_send_command(dev->hf, slc_cmer_resp, dev,
							"AT+CMER=3,0,0,1")) {
		error("hf-client: Counld not send AT+CMER");
		goto failed;
	}

	return;

failed:
	slc_error(dev);
}

static void slc_cind_resp(enum hfp_result result, enum hfp_error cme_err,
							void *user_data)
{
	struct device *dev = user_data;

	DBG("");

	hfp_hf_unregister(dev->hf, "+CIND");

	if (result != HFP_RESULT_OK) {
		error("hf-client: CIND error: %d", result);
		goto failed;
	}

	/* Continue with SLC creation */
	if (!hfp_hf_register(dev->hf, slc_cind_status_cb, "+CIND", dev,
								NULL)) {
		error("hf-client: Counld not register +CIND");
		goto failed;
	}

	if (!hfp_hf_send_command(dev->hf, slc_cind_status_resp, dev,
								"AT+CIND?")) {
		error("hf-client: Counld not send AT+CIND?");
		goto failed;
	}

	return;

failed:
	slc_error(dev);
}

static void ciev_service_cb(uint8_t val)
{
	struct hal_ev_hf_client_net_state ev;

	DBG("");

	if (val > HAL_HF_CLIENT_NET_ROAMING_TYPE_ROAMING) {
		error("hf-client: Incorrect state %u:", val);
		return;
	}

	ev.state = val;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
				HAL_EV_HF_CLIENT_NET_STATE, sizeof(ev), &ev);
}

static void ciev_call_cb(uint8_t val)
{
	struct hal_ev_hf_client_call_indicator ev;

	DBG("");

	if (val > HAL_HF_CLIENT_CALL_IND_CALL_IN_PROGERSS) {
		error("hf-client: Incorrect call state %u:", val);
		return;
	}

	ev.call = val;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_EV_HF_CLIENT_CALL_INDICATOR, sizeof(ev), &ev);
}

static void ciev_callsetup_cb(uint8_t val)
{
	struct hal_ev_hf_client_call_setup_indicator ev;

	DBG("");

	if (val > HAL_HF_CLIENT_CALL_SETUP_ALERTING) {
		error("hf-client: Incorrect call setup state %u:", val);
		return;
	}

	ev.call_setup = val;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
					HAL_EV_HF_CLIENT_CALL_SETUP_INDICATOR,
					sizeof(ev), &ev);
}

static void ciev_callheld_cb(uint8_t val)
{
	struct hal_ev_hf_client_call_held_indicator ev;

	DBG("");

	if (val > HAL_HF_CLIENT_CALL_SETUP_IND_HOLD) {
		error("hf-client: Incorrect call held state %u:", val);
		return;
	}

	ev.call_held = val;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
					HAL_EV_HF_CLIENT_CALL_HELD_INDICATOR,
					sizeof(ev), &ev);
}

static void ciev_signal_cb(uint8_t val)
{
	struct hal_ev_hf_client_net_signal_strength ev;

	DBG("");

	if (val > 5) {
		error("hf-client: Incorrect signal value %u:", val);
		return;
	}

	ev.signal_strength = val;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
					HAL_EV_HF_CLIENT_NET_SIGNAL_STRENGTH,
					sizeof(ev), &ev);
}

static void ciev_roam_cb(uint8_t val)
{
	struct hal_ev_hf_client_net_roaming_type ev;

	DBG("");

	if (val > HAL_HF_CLIENT_NET_ROAMING_TYPE_ROAMING) {
		error("hf-client: Incorrect roaming state %u:", val);
		return;
	}

	ev.state = val;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
					HAL_EV_HF_CLIENT_NET_ROAMING_TYPE,
					sizeof(ev), &ev);
}

static void ciev_battchg_cb(uint8_t val)
{
	struct hal_ev_hf_client_battery_level ev;

	DBG("");

	if (val > 5) {
		error("hf-client: Incorrect battery charge value %u:", val);
		return;
	}

	ev.battery_level = val;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
			HAL_EV_HF_CLIENT_BATTERY_LEVEL, sizeof(ev), &ev);
}

static void set_indicator_parameters(uint8_t index, const char *indicator,
						unsigned int min,
						unsigned int max,
						struct indicator *ag_ind)
{
	DBG("%s, %i", indicator, index);

	/* TODO: Verify min/max values ? */

	if (strcmp("service", indicator) == 0) {
		ag_ind[HFP_INDICATOR_SERVICE].index = index;
		ag_ind[HFP_INDICATOR_SERVICE].min = min;
		ag_ind[HFP_INDICATOR_SERVICE].max = max;
		ag_ind[HFP_INDICATOR_SERVICE].cb = ciev_service_cb;
		return;
	}

	if (strcmp("call", indicator) == 0) {
		ag_ind[HFP_INDICATOR_CALL].index = index;
		ag_ind[HFP_INDICATOR_CALL].min = min;
		ag_ind[HFP_INDICATOR_CALL].max = max;
		ag_ind[HFP_INDICATOR_CALL].cb = ciev_call_cb;
		return;
	}

	if (strcmp("callsetup", indicator) == 0) {
		ag_ind[HFP_INDICATOR_CALLSETUP].index = index;
		ag_ind[HFP_INDICATOR_CALLSETUP].min = min;
		ag_ind[HFP_INDICATOR_CALLSETUP].max = max;
		ag_ind[HFP_INDICATOR_CALLSETUP].cb = ciev_callsetup_cb;
		return;
	}

	if (strcmp("callheld", indicator) == 0) {
		ag_ind[HFP_INDICATOR_CALLHELD].index = index;
		ag_ind[HFP_INDICATOR_CALLHELD].min = min;
		ag_ind[HFP_INDICATOR_CALLHELD].max = max;
		ag_ind[HFP_INDICATOR_CALLHELD].cb = ciev_callheld_cb;
		return;
	}

	if (strcmp("signal", indicator) == 0) {
		ag_ind[HFP_INDICATOR_SIGNAL].index = index;
		ag_ind[HFP_INDICATOR_SIGNAL].min = min;
		ag_ind[HFP_INDICATOR_SIGNAL].max = max;
		ag_ind[HFP_INDICATOR_SIGNAL].cb = ciev_signal_cb;
		return;
	}

	if (strcmp("roam", indicator) == 0) {
		ag_ind[HFP_INDICATOR_ROAM].index = index;
		ag_ind[HFP_INDICATOR_ROAM].min = min;
		ag_ind[HFP_INDICATOR_ROAM].max = max;
		ag_ind[HFP_INDICATOR_ROAM].cb = ciev_roam_cb;
		return;
	}

	if (strcmp("battchg", indicator) == 0) {
		ag_ind[HFP_INDICATOR_BATTCHG].index = index;
		ag_ind[HFP_INDICATOR_BATTCHG].min = min;
		ag_ind[HFP_INDICATOR_BATTCHG].max = max;
		ag_ind[HFP_INDICATOR_BATTCHG].cb = ciev_battchg_cb;
		return;
	}

	error("hf-client: Unknown indicator: %s", indicator);
}

static void slc_cind_cb(struct hfp_context *context, void *user_data)
{
	struct device *dev = user_data;
	int index = 1;

	DBG("");

	while (hfp_context_has_next(context)) {
		char name[255];
		unsigned int min, max;

		/* e.g ("callsetup",(0-3)) */
		if (!hfp_context_open_container(context))
			break;

		if (!hfp_context_get_string(context, name, sizeof(name))) {
			error("hf-client: Could not get string");
			goto failed;
		}

		if (!hfp_context_open_container(context)) {
			error("hf-client: Could not open container");
			goto failed;
		}

		if (!hfp_context_get_range(context, &min, &max)) {
			if (!hfp_context_get_number(context, &min)) {
				error("hf-client: Could not get number");
				goto failed;
			}

			if (!hfp_context_get_number(context, &max)) {
				error("hf-client: Could not get number");
				goto failed;
			}
		}

		if (!hfp_context_close_container(context)) {
			error("hf-client: Could not close container");
			goto failed;
		}

		if (!hfp_context_close_container(context)) {
			error("hf-client: Could not close container");
			goto failed;
		}

		set_indicator_parameters(index, name, min, max, dev->ag_ind);
		index++;
	}

	return;

failed:
	error("hf-client: Error on CIND response");
	slc_error(dev);
}

static void slc_bac_resp(enum hfp_result result, enum hfp_error cme_err,
							void *user_data)
{
	struct device *dev = user_data;

	DBG("");

	if (result != HFP_RESULT_OK)
		goto failed;

	/* Continue with SLC creation */
	if (!hfp_hf_register(dev->hf, slc_cind_cb, "+CIND", dev, NULL)) {
		error("hf-client: Could not register for +CIND");
		goto failed;
	}

	if (!hfp_hf_send_command(dev->hf, slc_cind_resp, dev, "AT+CIND=?"))
		goto failed;

	return;

failed:
	error("hf-client: Error on BAC response");
	slc_error(dev);
}

static bool send_supported_codecs(struct device *dev)
{
	char codecs_string[8];
	char bac[16];

	memset(bac, 0, sizeof(bac));

	strcpy(bac, "AT+BAC=");

	get_local_codecs_string(dev, codecs_string, sizeof(codecs_string));
	strcat(bac, codecs_string);

	return hfp_hf_send_command(dev->hf, slc_bac_resp, dev, bac);
}

static void slc_brsf_cb(struct hfp_context *context, void *user_data)
{
	unsigned int feat;
	struct device *dev = user_data;

	DBG("");

	if (hfp_context_get_number(context, &feat))
		dev->features = feat;
}

static void slc_brsf_resp(enum hfp_result result, enum hfp_error cme_err,
							void *user_data)
{
	struct device *dev = user_data;

	hfp_hf_unregister(dev->hf, "+BRSF");

	if (result != HFP_RESULT_OK) {
		error("hf-client: BRSF error: %d", result);
		goto failed;
	}

	/* Continue with SLC creation */
	if (codec_negotiation_supported(dev)) {
		if (send_supported_codecs(dev))
			return;

		error("hf-client: Could not send BAC command");
		goto failed;
	}

	/* No WBS on remote side. Continue with indicators */
	if (!hfp_hf_register(dev->hf, slc_cind_cb, "+CIND", dev, NULL)) {
		error("hf-client: Could not register for +CIND");
		goto failed;
	}

	if (!hfp_hf_send_command(dev->hf, slc_cind_resp, dev, "AT+CIND=?")) {
		error("hf-client: Could not send AT+CIND command");
		goto failed;
	}

	return;

failed:
	slc_error(dev);
}

static bool create_slc(struct device *dev)
{
	DBG("");

	if (!hfp_hf_register(dev->hf, slc_brsf_cb, "+BRSF", dev, NULL))
		return false;

	return hfp_hf_send_command(dev->hf, slc_brsf_resp, dev, "AT+BRSF=%u",
							hfp_hf_features);
}

static void connect_cb(GIOChannel *chan, GError *err, gpointer user_data)
{
	struct device *dev = user_data;

	DBG("");

	if (err) {
		error("hf-client: connect failed (%s)", err->message);
		goto failed;
	}

	dev->hf = hfp_hf_new(g_io_channel_unix_get_fd(chan));
	if (!dev->hf) {
		error("hf-client: Could not create hfp io");
		goto failed;
	}

	g_io_channel_set_close_on_unref(chan, FALSE);

	hfp_hf_set_close_on_unref(dev->hf, true);
	hfp_hf_set_disconnect_handler(dev->hf, disconnect_watch, dev, NULL);

	if (!create_slc(dev)) {
		error("hf-client: Could not start SLC creation");
		hfp_hf_disconnect(dev->hf);
		goto failed;
	}

	device_set_state(dev, HAL_HF_CLIENT_CONN_STATE_CONNECTED);

	return;

failed:
	g_io_channel_shutdown(chan, TRUE, NULL);
	device_destroy(dev);
}

static void sdp_hfp_search_cb(sdp_list_t *recs, int err, gpointer data)
{
	sdp_list_t *protos, *classes;
	struct device *dev = data;
	GError *gerr = NULL;
	GIOChannel *io;
	uuid_t uuid;
	int channel;

	DBG("");

	if (err < 0) {
		error("hf-client: unable to get SDP record: %s",
							strerror(-err));
		goto failed;
	}

	if (!recs || !recs->data) {
		info("hf-client: no HFP SDP records found");
		goto failed;
	}

	if (sdp_get_service_classes(recs->data, &classes) < 0 || !classes) {
		error("hf-client: unable to get service classes from record");
		goto failed;
	}

	/* TODO read remote version? */

	memcpy(&uuid, classes->data, sizeof(uuid));
	sdp_list_free(classes, free);

	if (!sdp_uuid128_to_uuid(&uuid) || uuid.type != SDP_UUID16 ||
			uuid.value.uuid16 != HANDSFREE_AGW_SVCLASS_ID) {
		error("hf-client: invalid service record or not HFP");
		goto failed;
	}

	if (sdp_get_access_protos(recs->data, &protos) < 0) {
		error("hf-client: unable to get access protocols from record");
		sdp_list_free(classes, free);
		goto failed;
	}

	channel = sdp_get_proto_port(protos, RFCOMM_UUID);
	sdp_list_foreach(protos, (sdp_list_func_t) sdp_list_free, NULL);
	sdp_list_free(protos, NULL);
	if (channel <= 0) {
		error("hf-client: unable to get RFCOMM channel from record");
		goto failed;
	}

	io = bt_io_connect(connect_cb, dev, NULL, &gerr,
				BT_IO_OPT_SOURCE_BDADDR, &adapter_addr,
				BT_IO_OPT_DEST_BDADDR, &dev->bdaddr,
				BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_MEDIUM,
				BT_IO_OPT_CHANNEL, channel,
				BT_IO_OPT_INVALID);
	if (!io) {
		error("hf-client: unable to connect: %s", gerr->message);
		g_error_free(gerr);
		goto failed;
	}

	g_io_channel_unref(io);
	return;

failed:
	device_destroy(dev);
}

static int sdp_search_hfp(struct device *dev)
{
	uuid_t uuid;

	sdp_uuid16_create(&uuid, HANDSFREE_AGW_SVCLASS_ID);

	return bt_search_service(&adapter_addr, &dev->bdaddr, &uuid,
					sdp_hfp_search_cb, dev, NULL, 0);
}

static void handle_connect(const void *buf, uint16_t len)
{
	struct device *dev;
	const struct hal_cmd_hf_client_connect *cmd = buf;
	uint32_t status;
	bdaddr_t bdaddr;
	char addr[18];

	DBG("");

	android2bdaddr(&cmd->bdaddr, &bdaddr);

	ba2str(&bdaddr, addr);
	DBG("connecting to %s", addr);

	dev = get_device(&bdaddr);
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	if (dev->state != HAL_HF_CLIENT_CONN_STATE_DISCONNECTED) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	if (sdp_search_hfp(dev) < 0) {
		status = HAL_STATUS_FAILED;
		device_destroy(dev);
		goto done;
	}

	device_set_state(dev, HAL_HF_CLIENT_CONN_STATE_CONNECTING);

	status = HAL_STATUS_SUCCESS;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT,
					HAL_OP_HF_CLIENT_CONNECT, status);
}

static void confirm_cb(GIOChannel *chan, gpointer data)
{
	struct device *dev;
	char address[18];
	bdaddr_t bdaddr;
	GError *err = NULL;

	bt_io_get(chan, &err,
			BT_IO_OPT_DEST, address,
			BT_IO_OPT_DEST_BDADDR, &bdaddr,
			BT_IO_OPT_INVALID);
	if (err) {
		error("hf-client: confirm failed (%s)", err->message);
		g_error_free(err);
		goto drop;
	}

	DBG("Incoming connection from %s", address);

	dev = get_device(&bdaddr);
	if (!dev) {
		error("hf-client: There is other AG connected");
		goto drop;
	}

	if (dev->state != HAL_HF_CLIENT_CONN_STATE_DISCONNECTED) {
		/* TODO: Handle colision */
		error("hf-client: Connections is up or ongoing ?");
		goto drop;
	}

	device_set_state(dev, HAL_HF_CLIENT_CONN_STATE_CONNECTING);

	if (!bt_io_accept(chan, connect_cb, dev, NULL, NULL)) {
		error("hf-client: failed to accept connection");
		device_destroy(dev);
		goto drop;
	}

	return;

drop:
	g_io_channel_shutdown(chan, TRUE, NULL);
}

static const struct ipc_handler cmd_handlers[] = {
	/* HAL_OP_HF_CLIENT_CONNECT */
	{ handle_connect, false,
				sizeof(struct hal_cmd_hf_client_connect) },
	/* HAL_OP_HF_CLIENT_DISCONNECT */
	{ handle_disconnect, false,
				sizeof(struct hal_cmd_hf_client_disconnect) },
	/* HAL_OP_HF_CLIENT_CONNECT_AUDIO */
	{ handle_connect_audio, false,
			sizeof(struct hal_cmd_hf_client_connect_audio) },
	/* HAL_OP_HF_CLIENT_DISCONNECT_AUDIO */
	{ handle_disconnect_audio, false,
			sizeof(struct hal_cmd_hf_client_disconnect_audio) },
	/* define HAL_OP_HF_CLIENT_START_VR */
	{ handle_start_vr, false, 0 },
	/* define HAL_OP_HF_CLIENT_STOP_VR */
	{ handle_stop_vr, false, 0 },
	/* HAL_OP_HF_CLIENT_VOLUME_CONTROL */
	{ handle_volume_control, false,
			sizeof(struct hal_cmd_hf_client_volume_control) },
	/* HAL_OP_HF_CLIENT_DIAL */
	{ handle_dial, true, sizeof(struct hal_cmd_hf_client_dial) },
	/* HAL_OP_HF_CLIENT_DIAL_MEMORY */
	{ handle_dial_memory, false,
				sizeof(struct hal_cmd_hf_client_dial_memory) },
	/* HAL_OP_HF_CLIENT_CALL_ACTION */
	{ handle_call_action, false,
				sizeof(struct hal_cmd_hf_client_call_action) },
	/* HAL_OP_HF_CLIENT_QUERY_CURRENT_CALLS */
	{ handle_query_current_calls, false, 0 },
	/* HAL_OP_HF_CLIENT_QUERY_OPERATOR_NAME */
	{ handle_query_operator_name, false, 0 },
	/* HAL_OP_HF_CLIENT_RETRIEVE_SUBSCR_INFO */
	{ handle_retrieve_subscr_info, false, 0 },
	/* HAL_OP_HF_CLIENT_SEND_DTMF */
	{ handle_send_dtmf, false,
				sizeof(struct hal_cmd_hf_client_send_dtmf) },
	/* HAL_OP_HF_CLIENT_GET_LAST_VOICE_TAG_NUM */
	{ handle_get_last_vc_tag_num, false, 0 },
};

static sdp_record_t *hfp_hf_record(void)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, svclass_uuid, ga_svclass_uuid;
	uuid_t l2cap_uuid, rfcomm_uuid;
	sdp_profile_desc_t profile;
	sdp_list_t *aproto, *proto[2];
	sdp_record_t *record;
	sdp_data_t *channel, *features;
	uint16_t sdpfeat;
	uint8_t ch = HFP_HF_CHANNEL;

	record = sdp_record_alloc();
	if (!record)
		return NULL;

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(record, root);

	sdp_uuid16_create(&svclass_uuid, HANDSFREE_SVCLASS_ID);
	svclass_id = sdp_list_append(NULL, &svclass_uuid);
	sdp_uuid16_create(&ga_svclass_uuid, GENERIC_AUDIO_SVCLASS_ID);
	svclass_id = sdp_list_append(svclass_id, &ga_svclass_uuid);
	sdp_set_service_classes(record, svclass_id);

	sdp_uuid16_create(&profile.uuid, HANDSFREE_PROFILE_ID);
	profile.version = 0x0106;
	pfseq = sdp_list_append(NULL, &profile);
	sdp_set_profile_descs(record, pfseq);

	sdp_uuid16_create(&l2cap_uuid, L2CAP_UUID);
	proto[0] = sdp_list_append(NULL, &l2cap_uuid);
	apseq = sdp_list_append(NULL, proto[0]);

	sdp_uuid16_create(&rfcomm_uuid, RFCOMM_UUID);
	proto[1] = sdp_list_append(NULL, &rfcomm_uuid);
	channel = sdp_data_alloc(SDP_UINT8, &ch);
	proto[1] = sdp_list_append(proto[1], channel);
	apseq = sdp_list_append(apseq, proto[1]);

	/* Codec Negotiation bit in SDP feature is different then in BRSF */
	sdpfeat = hfp_hf_features & 0x0000003F;
	if (hfp_hf_features & HFP_HF_FEAT_CODEC)
		sdpfeat |= 0x00000020;
	else
		sdpfeat &= ~0x00000020;

	features = sdp_data_alloc(SDP_UINT16, &sdpfeat);
	sdp_attr_add(record, SDP_ATTR_SUPPORTED_FEATURES, features);

	aproto = sdp_list_append(NULL, apseq);
	sdp_set_access_protos(record, aproto);

	sdp_set_info_attr(record, "Hands-Free unit", NULL, NULL);

	sdp_data_free(channel);
	sdp_list_free(proto[0], NULL);
	sdp_list_free(proto[1], NULL);
	sdp_list_free(apseq, NULL);
	sdp_list_free(pfseq, NULL);
	sdp_list_free(aproto, NULL);
	sdp_list_free(root, NULL);
	sdp_list_free(svclass_id, NULL);

	return record;
}

static bool enable_hf_client(void)
{
	sdp_record_t *rec;
	GError *err = NULL;

	hfp_hf_server =  bt_io_listen(NULL, confirm_cb, NULL, NULL, &err,
					BT_IO_OPT_SOURCE_BDADDR, &adapter_addr,
					BT_IO_OPT_CHANNEL, HFP_HF_CHANNEL,
					BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_MEDIUM,
					BT_IO_OPT_INVALID);
	if (!hfp_hf_server) {
		error("hf-client: Failed to listen on Handsfree rfcomm: %s",
								err->message);
		g_error_free(err);
		return false;
	}

	hfp_hf_features = HFP_HF_FEATURES;

	rec = hfp_hf_record();
	if (!rec) {
		error("hf-client: Could not create service record");
		goto failed;
	}

	if (bt_adapter_add_record(rec, 0) < 0) {
		error("hf-client: Failed to register service record");
		sdp_record_free(rec);
		goto failed;
	}

	hfp_hf_record_id = rec->handle;

	return true;

failed:
	g_io_channel_shutdown(hfp_hf_server, TRUE, NULL);
	g_io_channel_unref(hfp_hf_server);
	hfp_hf_server = NULL;

	return false;
}

static void cleanup_hfp_hf(void)
{
	if (hfp_hf_server) {
		g_io_channel_shutdown(hfp_hf_server, TRUE, NULL);
		g_io_channel_unref(hfp_hf_server);
		hfp_hf_server = NULL;
	}

	if (hfp_hf_record_id > 0) {
		bt_adapter_remove_record(hfp_hf_record_id);
		hfp_hf_record_id = 0;
	}

	if (sco) {
		bt_sco_unref(sco);
		sco = NULL;
	}
}

static bool confirm_sco_cb(const bdaddr_t *addr, uint16_t *voice_settings)
{
	struct device *dev;

	DBG("");

	dev = find_device(addr);
	if (!dev || dev->state != HAL_HF_CLIENT_CONN_STATE_SLC_CONNECTED) {
		error("hf-client: No device or SLC not ready");
		return false;
	}

	set_audio_state(dev, HAL_HF_CLIENT_AUDIO_STATE_CONNECTING);

	if (codec_negotiation_supported(dev) &&
			dev->negotiated_codec != CODEC_ID_CVSD)
		*voice_settings = BT_VOICE_TRANSPARENT;
	else
		*voice_settings = BT_VOICE_CVSD_16BIT;

	return true;
}

static void connect_sco_cb(enum sco_status status, const bdaddr_t *addr)
{
	struct device *dev;
	uint8_t audio_state;

	DBG("SCO Status %u", status);

	/* Device shall be there, just sanity check */
	dev = find_device(addr);
	if (!dev) {
		error("hf-client: There is no device?");
		return;
	}

	if (status != SCO_STATUS_OK) {
		audio_state = HAL_HF_CLIENT_AUDIO_STATE_DISCONNECTED;
		goto done;
	}

	if (dev->negotiated_codec == CODEC_ID_MSBC)
		audio_state = HAL_HF_CLIENT_AUDIO_STATE_CONNECTED_MSBC;
	else
		audio_state = HAL_HF_CLIENT_AUDIO_STATE_CONNECTED;

done:
	set_audio_state(dev, audio_state);
}

static void disconnect_sco_cb(const bdaddr_t *addr)
{
	struct device *dev;

	DBG("");

	dev = find_device(addr);
	if (!dev) {
		error("hf-client: No device");
		return;
	}

	set_audio_state(dev, HAL_HF_CLIENT_AUDIO_STATE_DISCONNECTED);
}

bool bt_hf_client_register(struct ipc *ipc, const bdaddr_t *addr)
{
	DBG("");

	devices = queue_new();

	bacpy(&adapter_addr, addr);

	if (!enable_hf_client())
		goto failed;

	sco = bt_sco_new(addr);
	if (!sco) {
		error("hf-client: Cannot create SCO. HFP AG is in use ?");
		goto failed;
	}

	bt_sco_set_confirm_cb(sco, confirm_sco_cb);
	bt_sco_set_connect_cb(sco, connect_sco_cb);
	bt_sco_set_disconnect_cb(sco, disconnect_sco_cb);

	hal_ipc = ipc;
	ipc_register(hal_ipc, HAL_SERVICE_ID_HANDSFREE_CLIENT, cmd_handlers,
						G_N_ELEMENTS(cmd_handlers));

	return true;

failed:
	cleanup_hfp_hf();
	queue_destroy(devices, free);
	devices = NULL;

	return false;
}

void bt_hf_client_unregister(void)
{
	DBG("");

	cleanup_hfp_hf();

	queue_destroy(devices, (void *) device_destroy);
	devices = NULL;

	ipc_unregister(hal_ipc, HAL_SERVICE_ID_HANDSFREE);
	hal_ipc = NULL;
}

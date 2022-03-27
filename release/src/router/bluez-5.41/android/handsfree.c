/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2013-2014  Intel Corporation. All rights reserved.
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
#include "src/uuid-helper.h"
#include "src/shared/hfp.h"
#include "src/shared/queue.h"
#include "src/shared/util.h"
#include "btio/btio.h"
#include "hal-msg.h"
#include "ipc-common.h"
#include "ipc.h"
#include "handsfree.h"
#include "bluetooth.h"
#include "src/log.h"
#include "utils.h"
#include "sco-msg.h"
#include "sco.h"

#define HSP_AG_CHANNEL 12
#define HFP_AG_CHANNEL 13

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

#define HFP_HF_FEAT_ECNR	0x00000001
#define HFP_HF_FEAT_3WAY	0x00000002
#define HFP_HF_FEAT_CLI		0x00000004
#define HFP_HF_FEAT_VR		0x00000008
#define HFP_HF_FEAT_RVC		0x00000010
#define HFP_HF_FEAT_ECS		0x00000020
#define HFP_HF_FEAT_ECC		0x00000040
#define HFP_HF_FEAT_CODEC	0x00000080

#define HFP_AG_FEATURES (HFP_AG_FEAT_3WAY | HFP_AG_FEAT_ECNR |\
				HFP_AG_FEAT_VR | HFP_AG_FEAT_REJ_CALL |\
				HFP_AG_FEAT_ECS | HFP_AG_FEAT_EXT_ERR)

#define HFP_AG_CHLD "0,1,2,3"

/* offsets in indicators table, should be incremented when sending CIEV */
#define IND_SERVICE	0
#define IND_CALL	1
#define IND_CALLSETUP	2
#define IND_CALLHELD	3
#define IND_SIGNAL	4
#define IND_ROAM	5
#define IND_BATTCHG	6
#define IND_COUNT	(IND_BATTCHG + 1)

#define RING_TIMEOUT 2

#define CVSD_OFFSET 0
#define MSBC_OFFSET 1
#define CODECS_COUNT (MSBC_OFFSET + 1)

#define CODEC_ID_CVSD 0x01
#define CODEC_ID_MSBC 0x02

struct indicator {
	const char *name;
	int min;
	int max;
	int val;
	bool always_active;
	bool active;
};

struct hfp_codec {
	uint8_t type;
	bool local_supported;
	bool remote_supported;
};

struct hf_device {
	bdaddr_t bdaddr;
	uint8_t state;
	uint8_t audio_state;
	uint32_t features;

	bool clip_enabled;
	bool cmee_enabled;
	bool ccwa_enabled;
	bool indicators_enabled;
	struct indicator inds[IND_COUNT];
	int num_active;
	int num_held;
	int setup_state;
	guint call_hanging_up;

	uint8_t negotiated_codec;
	uint8_t proposed_codec;
	struct hfp_codec codecs[CODECS_COUNT];

	guint ring;
	char *clip;
	bool hsp;

	struct hfp_gw *gw;
	guint delay_sco;
};

static const struct indicator inds_defaults[] = {
		{ "service",   0, 1, 0, false, true },
		{ "call",      0, 1, 0, true, true },
		{ "callsetup", 0, 3, 0, true, true },
		{ "callheld",  0, 2, 0, true, true },
		{ "signal",    0, 5, 0, false, true },
		{ "roam",      0, 1, 0, false, true },
		{ "battchg",   0, 5, 0, false, true },
};

static const struct hfp_codec codecs_defaults[] = {
	{ CODEC_ID_CVSD, true, false},
	{ CODEC_ID_MSBC, false, false},
};

static struct queue *devices = NULL;

static uint32_t hfp_ag_features = 0;

static bdaddr_t adapter_addr;

static struct ipc *hal_ipc = NULL;
static struct ipc *sco_ipc = NULL;

static uint32_t hfp_record_id = 0;
static GIOChannel *hfp_server = NULL;

static uint32_t hsp_record_id = 0;
static GIOChannel *hsp_server = NULL;

static struct bt_sco *sco = NULL;

static unsigned int max_hfp_clients = 0;

static void set_state(struct hf_device *dev, uint8_t state)
{
	struct hal_ev_handsfree_conn_state ev;
	char address[18];

	if (dev->state == state)
		return;

	dev->state = state;

	ba2str(&dev->bdaddr, address);
	DBG("device %s state %u", address, state);

	bdaddr2android(&dev->bdaddr, ev.bdaddr);
	ev.state = state;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
				HAL_EV_HANDSFREE_CONN_STATE, sizeof(ev), &ev);
}

static void set_audio_state(struct hf_device *dev, uint8_t state)
{
	struct hal_ev_handsfree_audio_state ev;
	char address[18];

	if (dev->audio_state == state)
		return;

	dev->audio_state = state;

	ba2str(&dev->bdaddr, address);
	DBG("device %s audio state %u", address, state);

	bdaddr2android(&dev->bdaddr, ev.bdaddr);
	ev.state = state;

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
				HAL_EV_HANDSFREE_AUDIO_STATE, sizeof(ev), &ev);
}

static void init_codecs(struct hf_device *dev)
{
	memcpy(dev->codecs, codecs_defaults, sizeof(dev->codecs));

	if (hfp_ag_features & HFP_AG_FEAT_CODEC)
		dev->codecs[MSBC_OFFSET].local_supported = true;
}

static struct hf_device *device_create(const bdaddr_t *bdaddr)
{
	struct hf_device *dev;

	dev = new0(struct hf_device, 1);

	bacpy(&dev->bdaddr, bdaddr);
	dev->setup_state = HAL_HANDSFREE_CALL_STATE_IDLE;
	dev->state = HAL_EV_HANDSFREE_CONN_STATE_DISCONNECTED;
	dev->audio_state = HAL_EV_HANDSFREE_AUDIO_STATE_DISCONNECTED;

	memcpy(dev->inds, inds_defaults, sizeof(dev->inds));

	init_codecs(dev);

	queue_push_head(devices, dev);

	return dev;
}

static void device_destroy(struct hf_device *dev)
{
	hfp_gw_unref(dev->gw);

	if (dev->delay_sco)
		g_source_remove(dev->delay_sco);

	if (dev->audio_state == HAL_EV_HANDSFREE_AUDIO_STATE_CONNECTED)
		bt_sco_disconnect(sco);

	if (dev->ring)
		g_source_remove(dev->ring);

	g_free(dev->clip);

	if (dev->call_hanging_up)
		g_source_remove(dev->call_hanging_up);

	set_audio_state(dev, HAL_EV_HANDSFREE_AUDIO_STATE_DISCONNECTED);
	set_state(dev, HAL_EV_HANDSFREE_CONN_STATE_DISCONNECTED);

	queue_remove(devices, dev);
	free(dev);
}

static bool match_by_bdaddr(const void *data, const void *match_data)
{
	const struct hf_device *dev = data;
	const bdaddr_t *addr = match_data;

	return !bacmp(&dev->bdaddr, addr);
}

static struct hf_device *find_device(const bdaddr_t *bdaddr)
{
	if (!bacmp(bdaddr, BDADDR_ANY))
		return queue_peek_head(devices);

	return queue_find(devices, match_by_bdaddr, bdaddr);
}

static struct hf_device *get_device(const bdaddr_t *bdaddr)
{
	struct hf_device *dev;

	dev = find_device(bdaddr);
	if (dev)
		return dev;

	if (queue_length(devices) == max_hfp_clients)
		return NULL;

	return device_create(bdaddr);
}

static void disconnect_watch(void *user_data)
{
	struct hf_device *dev = user_data;

	DBG("");

	device_destroy(dev);
}

static void at_cmd_unknown(const char *command, void *user_data)
{
	struct hf_device *dev = user_data;
	uint8_t buf[IPC_MTU];
	struct hal_ev_handsfree_unknown_at *ev = (void *) buf;

	bdaddr2android(&dev->bdaddr, ev->bdaddr);

	/* copy while string including terminating NULL */
	ev->len = strlen(command) + 1;

	if (ev->len > IPC_MTU - sizeof(*ev)) {
		hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
		return;
	}

	memcpy(ev->buf, command, ev->len);

	ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
			HAL_EV_HANDSFREE_UNKNOWN_AT, sizeof(*ev) + ev->len, ev);
}

static void at_cmd_vgm(struct hfp_context *context,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct hf_device *dev = user_data;
	struct hal_ev_handsfree_volume ev;
	unsigned int val;

	DBG("");

	switch (type) {
	case HFP_GW_CMD_TYPE_SET:
		if (!hfp_context_get_number(context, &val) || val > 15)
			break;

		if (hfp_context_has_next(context))
			break;

		ev.type = HAL_HANDSFREE_VOLUME_TYPE_MIC;
		ev.volume = val;
		bdaddr2android(&dev->bdaddr, ev.bdaddr);

		ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
				HAL_EV_HANDSFREE_VOLUME, sizeof(ev), &ev);

		/* Framework is not replying with result for AT+VGM */
		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);
		return;
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_TEST:
	case HFP_GW_CMD_TYPE_COMMAND:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void at_cmd_vgs(struct hfp_context *context,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct hf_device *dev = user_data;
	struct hal_ev_handsfree_volume ev;
	unsigned int val;

	DBG("");

	switch (type) {
	case HFP_GW_CMD_TYPE_SET:
		if (!hfp_context_get_number(context, &val) || val > 15)
			break;

		if (hfp_context_has_next(context))
			break;

		ev.type = HAL_HANDSFREE_VOLUME_TYPE_SPEAKER;
		ev.volume = val;
		bdaddr2android(&dev->bdaddr, ev.bdaddr);

		ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
				HAL_EV_HANDSFREE_VOLUME, sizeof(ev), &ev);

		/* Framework is not replying with result for AT+VGS */
		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);
		return;
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_TEST:
	case HFP_GW_CMD_TYPE_COMMAND:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void at_cmd_cops(struct hfp_context *context,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct hf_device *dev = user_data;
	struct hal_ev_handsfree_cops ev;
	unsigned int val;

	switch (type) {
	case HFP_GW_CMD_TYPE_SET:
		if (!hfp_context_get_number(context, &val) || val != 3)
			break;

		if (!hfp_context_get_number(context, &val) || val != 0)
			break;

		if (hfp_context_has_next(context))
			break;

		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);
		return;
	case HFP_GW_CMD_TYPE_READ:
		bdaddr2android(&dev->bdaddr, ev.bdaddr);

		ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
					HAL_EV_HANDSFREE_COPS, sizeof(ev), &ev);
		return;
	case HFP_GW_CMD_TYPE_TEST:
	case HFP_GW_CMD_TYPE_COMMAND:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void at_cmd_bia(struct hfp_context *context,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct hf_device *dev = user_data;
	unsigned int val, i, def;
	bool tmp[IND_COUNT];

	DBG("");

	switch (type) {
	case HFP_GW_CMD_TYPE_SET:
		for (i = 0; i < IND_COUNT; i++)
			tmp[i] = dev->inds[i].active;

		i = 0;

		do {
			def = (i < IND_COUNT) ? dev->inds[i].active : 0;

			if (!hfp_context_get_number_default(context, &val, def))
				goto failed;

			if (val > 1)
				goto failed;

			if (i < IND_COUNT) {
				tmp[i] = val || dev->inds[i].always_active;
				i++;
			}
		} while (hfp_context_has_next(context));

		for (i = 0; i < IND_COUNT; i++)
			dev->inds[i].active = tmp[i];

		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);
		return;
	case HFP_GW_CMD_TYPE_TEST:
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_COMMAND:
		break;
	}

failed:
	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void at_cmd_a(struct hfp_context *context,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct hf_device *dev = user_data;
	struct hal_ev_handsfree_answer ev;

	DBG("");

	switch (type) {
	case HFP_GW_CMD_TYPE_COMMAND:
		if (hfp_context_has_next(context))
			break;

		bdaddr2android(&dev->bdaddr, ev.bdaddr);

		ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
				HAL_EV_HANDSFREE_ANSWER, sizeof(ev), &ev);

		/* Framework is not replying with result for ATA */
		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);
		return;
	case HFP_GW_CMD_TYPE_SET:
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_TEST:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void at_cmd_d(struct hfp_context *context,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct hf_device *dev = user_data;
	char buf[IPC_MTU];
	struct hal_ev_handsfree_dial *ev = (void *) buf;
	int cnt;

	DBG("");

	switch (type) {
	case HFP_GW_CMD_TYPE_SET:
		if (!hfp_context_get_unquoted_string(context,
						(char *) ev->number, 255))
			break;

		bdaddr2android(&dev->bdaddr, ev->bdaddr);

		ev->number_len = strlen((char *) ev->number);

		if (ev->number[ev->number_len - 1] != ';')
			break;

		if (ev->number[0] == '>')
			cnt = strspn((char *) ev->number + 1, "0123456789") + 1;
		else
			cnt = strspn((char *) ev->number, "0123456789ABC*#+");

		if (cnt != ev->number_len - 1)
			break;

		ev->number_len++;

		ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
					HAL_EV_HANDSFREE_DIAL,
					sizeof(*ev) + ev->number_len, ev);
		return;
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_TEST:
	case HFP_GW_CMD_TYPE_COMMAND:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void at_cmd_ccwa(struct hfp_context *context,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct hf_device *dev = user_data;
	unsigned int val;

	DBG("");

	switch (type) {
	case HFP_GW_CMD_TYPE_SET:
		if (!hfp_context_get_number(context, &val) || val > 1)
			break;

		if (hfp_context_has_next(context))
			break;

		dev->ccwa_enabled = val;

		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);
		return;
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_TEST:
	case HFP_GW_CMD_TYPE_COMMAND:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void at_cmd_chup(struct hfp_context *context,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct hf_device *dev = user_data;
	struct hal_ev_handsfree_hangup ev;

	DBG("");

	switch (type) {
	case HFP_GW_CMD_TYPE_COMMAND:
		if (hfp_context_has_next(context))
			break;

		bdaddr2android(&dev->bdaddr, ev.bdaddr);

		ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
				HAL_EV_HANDSFREE_HANGUP, sizeof(ev), &ev);

		/* Framework is not replying with result for AT+CHUP */
		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);
		return;
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_TEST:
	case HFP_GW_CMD_TYPE_SET:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void at_cmd_clcc(struct hfp_context *context,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct hf_device *dev = user_data;
	struct hal_ev_handsfree_clcc ev;

	DBG("");

	switch (type) {
	case HFP_GW_CMD_TYPE_COMMAND:
		if (hfp_context_has_next(context))
			break;

		bdaddr2android(&dev->bdaddr, ev.bdaddr);

		ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
					HAL_EV_HANDSFREE_CLCC, sizeof(ev), &ev);
		return;
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_TEST:
	case HFP_GW_CMD_TYPE_SET:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void at_cmd_cmee(struct hfp_context *context,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct hf_device *dev = user_data;
	unsigned int val;

	DBG("");

	switch (type) {
	case HFP_GW_CMD_TYPE_SET:
		if (!hfp_context_get_number(context, &val) || val > 1)
			break;

		if (hfp_context_has_next(context))
			break;

		dev->cmee_enabled = val;

		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);
		return;
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_TEST:
	case HFP_GW_CMD_TYPE_COMMAND:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void at_cmd_clip(struct hfp_context *context,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct hf_device *dev = user_data;
	unsigned int val;

	DBG("");

	switch (type) {
	case HFP_GW_CMD_TYPE_SET:
		if (!hfp_context_get_number(context, &val) || val > 1)
			break;

		if (hfp_context_has_next(context))
			break;

		dev->clip_enabled = val;

		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);
		return;
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_TEST:
	case HFP_GW_CMD_TYPE_COMMAND:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void at_cmd_vts(struct hfp_context *context,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct hf_device *dev = user_data;
	struct hal_ev_handsfree_dtmf ev;
	char str[2];

	DBG("");

	switch (type) {
	case HFP_GW_CMD_TYPE_SET:
		if (!hfp_context_get_unquoted_string(context, str, 2))
			break;

		if (!((str[0] >= '0' && str[0] <= '9') ||
				(str[0] >= 'A' && str[0] <= 'D') ||
				str[0] == '*' || str[0] == '#'))
			break;

		if (hfp_context_has_next(context))
			break;

		bdaddr2android(&dev->bdaddr, ev.bdaddr);
		ev.tone = str[0];

		ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
					HAL_EV_HANDSFREE_DTMF, sizeof(ev), &ev);

		/* Framework is not replying with result for AT+VTS */
		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);
		return;
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_TEST:
	case HFP_GW_CMD_TYPE_COMMAND:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void at_cmd_cnum(struct hfp_context *context,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct hf_device *dev = user_data;
	struct hal_ev_handsfree_cnum ev;

	DBG("");

	switch (type) {
	case HFP_GW_CMD_TYPE_COMMAND:
		if (hfp_context_has_next(context))
			break;

		bdaddr2android(&dev->bdaddr, ev.bdaddr);

		ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
					HAL_EV_HANDSFREE_CNUM, sizeof(ev), &ev);
		return;
	case HFP_GW_CMD_TYPE_SET:
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_TEST:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void at_cmd_binp(struct hfp_context *context,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct hf_device *dev = user_data;

	DBG("");

	/* TODO */

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void at_cmd_bldn(struct hfp_context *context,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct hf_device *dev = user_data;
	struct hal_ev_handsfree_dial ev;

	DBG("");

	switch (type) {
	case HFP_GW_CMD_TYPE_COMMAND:
		if (hfp_context_has_next(context))
			break;

		bdaddr2android(&dev->bdaddr, ev.bdaddr);
		ev.number_len = 0;

		ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
					HAL_EV_HANDSFREE_DIAL, sizeof(ev), &ev);
		return;
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_TEST:
	case HFP_GW_CMD_TYPE_SET:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void at_cmd_bvra(struct hfp_context *context,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct hf_device *dev = user_data;
	struct hal_ev_handsfree_vr_state ev;
	unsigned int val;

	DBG("");

	switch (type) {
	case HFP_GW_CMD_TYPE_SET:
		if (!hfp_context_get_number(context, &val) || val > 1)
			break;

		if (hfp_context_has_next(context))
			break;

		if (val)
			ev.state = HAL_HANDSFREE_VR_STARTED;
		else
			ev.state = HAL_HANDSFREE_VR_STOPPED;

		bdaddr2android(&dev->bdaddr, ev.bdaddr);

		ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
					HAL_EV_HANDSFREE_VR, sizeof(ev), &ev);
		return;
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_TEST:
	case HFP_GW_CMD_TYPE_COMMAND:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void at_cmd_nrec(struct hfp_context *context,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct hf_device *dev = user_data;
	struct hal_ev_handsfree_nrec ev;
	unsigned int val;

	DBG("");

	switch (type) {
	case HFP_GW_CMD_TYPE_SET:
		/*
		 * Android HAL defines start and stop parameter for NREC
		 * callback, but spec allows HF to only disable AG's NREC
		 * feature for SLC duration. Follow spec here.
		 */
		if (!hfp_context_get_number(context, &val) || val != 0)
			break;

		if (hfp_context_has_next(context))
			break;

		ev.nrec = HAL_HANDSFREE_NREC_STOP;
		bdaddr2android(&dev->bdaddr, ev.bdaddr);

		ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
					HAL_EV_HANDSFREE_NREC, sizeof(ev), &ev);

		/* Framework is not replying with context for AT+NREC */
		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);
		return;
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_TEST:
	case HFP_GW_CMD_TYPE_COMMAND:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void at_cmd_bsir(struct hfp_context *context,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct hf_device *dev = user_data;

	DBG("");

	/* TODO */

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void at_cmd_btrh(struct hfp_context *context,
				enum hfp_gw_cmd_type type, void *user_data)
{
	struct hf_device *dev = user_data;

	DBG("");

	/* TODO */

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void disconnect_sco_cb(const bdaddr_t *addr)
{
	struct hf_device *dev;

	DBG("");

	dev = find_device(addr);
	if (!dev) {
		error("handsfree: Could not find device");
		return;
	}

	set_audio_state(dev, HAL_EV_HANDSFREE_AUDIO_STATE_DISCONNECTED);
}

static void select_codec(struct hf_device *dev, uint8_t codec_type)
{
	uint8_t type = CODEC_ID_CVSD;
	int i;

	if (codec_type > 0) {
		type = codec_type;
		goto done;
	}

	for (i = CODECS_COUNT - 1; i >= CVSD_OFFSET; i--) {
		if (!dev->codecs[i].local_supported)
			continue;

		if (!dev->codecs[i].remote_supported)
			continue;

		type = dev->codecs[i].type;
		break;
	}

done:
	dev->proposed_codec = type;

	hfp_gw_send_info(dev->gw, "+BCS: %u", type);
}

static bool codec_negotiation_supported(struct hf_device *dev)
{
	return (dev->features & HFP_HF_FEAT_CODEC) &&
			(hfp_ag_features & HFP_AG_FEAT_CODEC);
}

static void connect_sco_cb(enum sco_status status, const bdaddr_t *addr)
{
	struct hf_device *dev;

	dev = find_device(addr);
	if (!dev) {
		error("handsfree: Connect sco failed, no device?");
		return;
	}

	if (status == SCO_STATUS_OK) {
		set_audio_state(dev, HAL_EV_HANDSFREE_AUDIO_STATE_CONNECTED);
		return;
	}

	/* Try fallback to CVSD first */
	if (codec_negotiation_supported(dev) &&
				dev->negotiated_codec != CODEC_ID_CVSD) {
		info("handsfree: trying fallback with CVSD");
		select_codec(dev, CODEC_ID_CVSD);
		return;
	}

	error("handsfree: audio connect failed");
	set_audio_state(dev, HAL_EV_HANDSFREE_AUDIO_STATE_DISCONNECTED);
}

static bool connect_sco(struct hf_device *dev)
{
	uint16_t voice_settings;

	if (codec_negotiation_supported(dev) &&
			dev->negotiated_codec != CODEC_ID_CVSD)
		voice_settings = BT_VOICE_TRANSPARENT;
	else
		voice_settings = BT_VOICE_CVSD_16BIT;

	if (!bt_sco_connect(sco, &dev->bdaddr, voice_settings))
		return false;

	set_audio_state(dev, HAL_EV_HANDSFREE_AUDIO_STATE_CONNECTING);

	return true;
}

static gboolean connect_sco_delayed(void *data)
{
	struct hf_device *dev = data;

	DBG("");

	dev->delay_sco = 0;

	if (connect_sco(dev))
		return FALSE;

	/*
	 * we try connect to negotiated codec. If it fails, and it isn't
	 * CVSD codec, try connect CVSD
	 */
	if (dev->negotiated_codec != CODEC_ID_CVSD)
		select_codec(dev, CODEC_ID_CVSD);

	return FALSE;
}

static void at_cmd_bcc(struct hfp_context *result, enum hfp_gw_cmd_type type,
								void *user_data)
{
	struct hf_device *dev = user_data;

	DBG("");

	switch (type) {
	case HFP_GW_CMD_TYPE_COMMAND:
		if (!codec_negotiation_supported(dev))
			break;

		if (hfp_context_has_next(result))
			break;

		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);

		/* we haven't negotiated codec, start selection */
		if (!dev->negotiated_codec) {
			select_codec(dev, 0);
			return;
		}

		/* Delay SCO connection so that OK response is send first */
		if (dev->delay_sco == 0)
			dev->delay_sco = g_idle_add(connect_sco_delayed, dev);
		return;
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_TEST:
	case HFP_GW_CMD_TYPE_SET:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void at_cmd_bcs(struct hfp_context *result, enum hfp_gw_cmd_type type,
								void *user_data)
{
	struct hf_device *dev = user_data;
	struct hal_ev_handsfree_wbs ev;
	unsigned int val;

	DBG("");

	switch (type) {
	case HFP_GW_CMD_TYPE_SET:
		if (!hfp_context_get_number(result, &val))
			break;

		if (hfp_context_has_next(result))
			break;

		/* Remote replied with other codec. Reply with error */
		if (dev->proposed_codec != val) {
			dev->proposed_codec = 0;
			break;
		}

		ev.wbs = val;
		bdaddr2android(&dev->bdaddr, ev.bdaddr);

		ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
					HAL_EV_HANDSFREE_WBS, sizeof(ev), &ev);

		dev->proposed_codec = 0;
		dev->negotiated_codec = val;

		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);

		/*
		 * Delay SCO connection so that OK response is send first,
		 * then connect with negotiated parameters.
		 */
		if (dev->delay_sco == 0)
			dev->delay_sco = g_idle_add(connect_sco_delayed, dev);
		return;
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_TEST:
	case HFP_GW_CMD_TYPE_COMMAND:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void at_cmd_ckpd(struct hfp_context *result, enum hfp_gw_cmd_type type,
								void *user_data)
{
	struct hf_device *dev = user_data;
	struct hal_ev_handsfree_hsp_key_press ev;
	unsigned int val;

	DBG("");

	switch (type) {
	case HFP_GW_CMD_TYPE_SET:
		if (!hfp_context_get_number(result, &val) || val != 200)
			break;

		if (hfp_context_has_next(result))
			break;

		bdaddr2android(&dev->bdaddr, ev.bdaddr);

		ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
						HAL_EV_HANDSFREE_HSP_KEY_PRESS,
						sizeof(ev), &ev);

		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);
		return;
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_TEST:
	case HFP_GW_CMD_TYPE_COMMAND:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);
}

static void register_post_slc_at(struct hf_device *dev)
{
	hfp_gw_set_command_handler(dev->gw, at_cmd_unknown, dev, NULL);

	if (dev->hsp) {
		hfp_gw_register(dev->gw, at_cmd_ckpd, "+CKPD", dev, NULL);
		hfp_gw_register(dev->gw, at_cmd_vgs, "+VGS", dev, NULL);
		hfp_gw_register(dev->gw, at_cmd_vgm, "+VGM", dev, NULL);
		return;
	}

	hfp_gw_register(dev->gw, at_cmd_a, "A", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_d, "D", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_ccwa, "+CCWA", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_chup, "+CHUP", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_clcc, "+CLCC", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_cops, "+COPS", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_cmee, "+CMEE", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_clip, "+CLIP", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_vts, "+VTS", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_cnum, "+CNUM", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_bia, "+BIA", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_binp, "+BINP", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_bldn, "+BLDN", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_bvra, "+BVRA", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_nrec, "+NREC", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_vgs, "+VGS", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_vgm, "+VGM", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_bsir, "+BSIR", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_btrh, "+BTRH", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_bcc, "+BCC", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_bcs, "+BCS", dev, NULL);
}

static void at_cmd_cmer(struct hfp_context *result, enum hfp_gw_cmd_type type,
								void *user_data)
{
	struct hf_device *dev = user_data;
	unsigned int val;

	switch (type) {
	case HFP_GW_CMD_TYPE_SET:
		/* mode must be =3 */
		if (!hfp_context_get_number(result, &val) || val != 3)
			break;

		/* keyp is don't care */
		if (!hfp_context_get_number(result, &val))
			break;

		/* disp is don't care */
		if (!hfp_context_get_number(result, &val))
			break;

		/* ind must be 0 or 1 */
		if (!hfp_context_get_number(result, &val) || val > 1)
			break;

		dev->indicators_enabled = val;

		/* skip bfr if present */
		hfp_context_get_number(result, &val);

		if (hfp_context_has_next(result))
			break;

		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);

		if (dev->features & HFP_HF_FEAT_3WAY)
			return;

		register_post_slc_at(dev);
		set_state(dev, HAL_EV_HANDSFREE_CONN_STATE_SLC_CONNECTED);
		return;
	case HFP_GW_CMD_TYPE_TEST:
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_COMMAND:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);

	if (dev->state != HAL_EV_HANDSFREE_CONN_STATE_SLC_CONNECTED)
		hfp_gw_disconnect(dev->gw);
}

static void at_cmd_cind(struct hfp_context *result, enum hfp_gw_cmd_type type,
								void *user_data)
{
	struct hf_device *dev = user_data;
	struct hal_ev_handsfree_cind ev;
	char *buf, *ptr;
	int len;
	unsigned int i;

	switch (type) {
	case HFP_GW_CMD_TYPE_TEST:

		/*
		 * If device supports Codec Negotiation, AT+BAC should be
		 * received first
		 */
		if (codec_negotiation_supported(dev) &&
				!dev->codecs[CVSD_OFFSET].remote_supported)
			break;

		len = strlen("+CIND:") + 1;

		for (i = 0; i < IND_COUNT; i++) {
			len += strlen("(\"\",(X,X)),");
			len += strlen(dev->inds[i].name);
		}

		buf = g_malloc(len);

		ptr = buf + sprintf(buf, "+CIND:");

		for (i = 0; i < IND_COUNT; i++) {
			ptr += sprintf(ptr, "(\"%s\",(%d%c%d)),",
					dev->inds[i].name,
					dev->inds[i].min,
					dev->inds[i].max == 1 ? ',' : '-',
					dev->inds[i].max);
		}

		ptr--;
		*ptr = '\0';

		hfp_gw_send_info(dev->gw, "%s", buf);
		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);

		g_free(buf);
		return;
	case HFP_GW_CMD_TYPE_READ:
		bdaddr2android(&dev->bdaddr, ev.bdaddr);

		ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
					HAL_EV_HANDSFREE_CIND, sizeof(ev), &ev);
		return;
	case HFP_GW_CMD_TYPE_SET:
	case HFP_GW_CMD_TYPE_COMMAND:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);

	if (dev->state != HAL_EV_HANDSFREE_CONN_STATE_SLC_CONNECTED)
		hfp_gw_disconnect(dev->gw);
}

static void at_cmd_brsf(struct hfp_context *result, enum hfp_gw_cmd_type type,
								void *user_data)
{
	struct hf_device *dev = user_data;
	unsigned int feat;

	switch (type) {
	case HFP_GW_CMD_TYPE_SET:
		if (!hfp_context_get_number(result, &feat))
			break;

		if (hfp_context_has_next(result))
			break;

		/* TODO verify features */
		dev->features = feat;

		hfp_gw_send_info(dev->gw, "+BRSF: %u", hfp_ag_features);
		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);
		return;
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_TEST:
	case HFP_GW_CMD_TYPE_COMMAND:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);

	if (dev->state != HAL_EV_HANDSFREE_CONN_STATE_SLC_CONNECTED)
		hfp_gw_disconnect(dev->gw);
}

static void at_cmd_chld(struct hfp_context *result, enum hfp_gw_cmd_type type,
								void *user_data)
{
	struct hf_device *dev = user_data;
	struct hal_ev_handsfree_chld ev;
	unsigned int val;

	DBG("");

	switch (type) {
	case HFP_GW_CMD_TYPE_SET:
		if (!hfp_context_get_number(result, &val) || val > 3)
			break;

		/* No ECC support */
		if (hfp_context_has_next(result))
			break;

		/* value match HAL type */
		ev.chld = val;
		bdaddr2android(&dev->bdaddr, ev.bdaddr);

		ipc_send_notif(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
					HAL_EV_HANDSFREE_CHLD, sizeof(ev), &ev);
		return;
	case HFP_GW_CMD_TYPE_TEST:
		hfp_gw_send_info(dev->gw, "+CHLD: (%s)", HFP_AG_CHLD);
		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);

		register_post_slc_at(dev);
		set_state(dev, HAL_EV_HANDSFREE_CONN_STATE_SLC_CONNECTED);
		return;
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_COMMAND:
		break;
	}

	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);

	if (dev->state != HAL_EV_HANDSFREE_CONN_STATE_SLC_CONNECTED)
		hfp_gw_disconnect(dev->gw);
}

static struct hfp_codec *find_codec_by_type(struct hf_device *dev, uint8_t type)
{
	int i;

	for (i = 0; i < CODECS_COUNT; i++)
		if (type == dev->codecs[i].type)
			return &dev->codecs[i];

	return NULL;
}

static void at_cmd_bac(struct hfp_context *result, enum hfp_gw_cmd_type type,
								void *user_data)
{
	struct hf_device *dev = user_data;
	unsigned int val;

	DBG("");

	switch (type) {
	case HFP_GW_CMD_TYPE_SET:
		if (!codec_negotiation_supported(dev))
			goto failed;

		/* set codecs to defaults */
		init_codecs(dev);
		dev->negotiated_codec = 0;

		/*
		 * At least CVSD mandatory codec must exist
		 * HFP V1.6 4.34.1
		 */
		if (!hfp_context_get_number(result, &val) ||
							val != CODEC_ID_CVSD)
			goto failed;

		dev->codecs[CVSD_OFFSET].remote_supported = true;

		if (hfp_context_get_number(result, &val)) {
			if (val != CODEC_ID_MSBC)
				goto failed;

			dev->codecs[MSBC_OFFSET].remote_supported = true;
		}

		while (hfp_context_has_next(result)) {
			struct hfp_codec *codec;

			if (!hfp_context_get_number(result, &val))
				goto failed;

			codec = find_codec_by_type(dev, val);
			if (!codec)
				continue;

			codec->remote_supported = true;
		}

		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);

		if (dev->proposed_codec)
			select_codec(dev, 0);
		return;
	case HFP_GW_CMD_TYPE_TEST:
	case HFP_GW_CMD_TYPE_READ:
	case HFP_GW_CMD_TYPE_COMMAND:
		break;
	}

failed:
	hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);

	if (dev->state != HAL_EV_HANDSFREE_CONN_STATE_SLC_CONNECTED)
		hfp_gw_disconnect(dev->gw);
}

static void register_slc_at(struct hf_device *dev)
{
	hfp_gw_register(dev->gw, at_cmd_brsf, "+BRSF", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_cind, "+CIND", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_cmer, "+CMER", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_chld, "+CHLD", dev, NULL);
	hfp_gw_register(dev->gw, at_cmd_bac, "+BAC", dev, NULL);
}

static void connect_cb(GIOChannel *chan, GError *err, gpointer user_data)
{
	struct hf_device *dev = user_data;

	DBG("");

	if (err) {
		error("handsfree: connect failed (%s)", err->message);
		goto failed;
	}

	dev->gw = hfp_gw_new(g_io_channel_unix_get_fd(chan));
	if (!dev->gw)
		goto failed;

	g_io_channel_set_close_on_unref(chan, FALSE);

	hfp_gw_set_close_on_unref(dev->gw, true);
	hfp_gw_set_disconnect_handler(dev->gw, disconnect_watch, dev, NULL);

	if (dev->hsp) {
		register_post_slc_at(dev);
		set_state(dev, HAL_EV_HANDSFREE_CONN_STATE_CONNECTED);
		set_state(dev, HAL_EV_HANDSFREE_CONN_STATE_SLC_CONNECTED);
		return;
	}

	register_slc_at(dev);
	set_state(dev, HAL_EV_HANDSFREE_CONN_STATE_CONNECTED);
	return;

failed:
	g_io_channel_shutdown(chan, TRUE, NULL);
	device_destroy(dev);
}

static void confirm_cb(GIOChannel *chan, gpointer data)
{
	char address[18];
	bdaddr_t bdaddr;
	GError *err = NULL;
	struct hf_device *dev;

	bt_io_get(chan, &err,
			BT_IO_OPT_DEST, address,
			BT_IO_OPT_DEST_BDADDR, &bdaddr,
			BT_IO_OPT_INVALID);
	if (err) {
		error("handsfree: confirm failed (%s)", err->message);
		g_error_free(err);
		goto drop;
	}

	DBG("incoming connect from %s", address);

	dev = get_device(&bdaddr);
	if (!dev) {
		error("handsfree: Failed to get device object for %s", address);
		goto drop;
	}

	if (dev->state != HAL_EV_HANDSFREE_CONN_STATE_DISCONNECTED) {
		info("handsfree: refusing connection from %s", address);
		goto drop;
	}

	if (!bt_io_accept(chan, connect_cb, dev, NULL, NULL)) {
		error("handsfree: failed to accept connection");
		device_destroy(dev);
		goto drop;
	}

	dev->hsp = GPOINTER_TO_INT(data);

	set_state(dev, HAL_EV_HANDSFREE_CONN_STATE_CONNECTING);

	return;

drop:
	g_io_channel_shutdown(chan, TRUE, NULL);
}

static void sdp_hsp_search_cb(sdp_list_t *recs, int err, gpointer data)
{
	struct hf_device *dev = data;
	sdp_list_t *protos;
	GError *gerr = NULL;
	GIOChannel *io;
	uuid_t class;
	int channel;

	DBG("");

	if (err < 0) {
		error("handsfree: unable to get SDP record: %s",
								strerror(-err));
		goto fail;
	}

	sdp_uuid16_create(&class, HEADSET_SVCLASS_ID);

	/* Find record with proper service class */
	for (; recs; recs = recs->next) {
		sdp_record_t *rec = recs->data;

		if (rec && !sdp_uuid_cmp(&rec->svclass, &class))
			break;
	}

	if (!recs || !recs->data) {
		info("handsfree: no valid HSP SDP records found");
		goto fail;
	}

	if (sdp_get_access_protos(recs->data, &protos) < 0) {
		error("handsfree: unable to get access protocols from record");
		goto fail;
	}

	/* TODO read remote version? */
	/* TODO read volume control support */

	channel = sdp_get_proto_port(protos, RFCOMM_UUID);
	sdp_list_foreach(protos, (sdp_list_func_t) sdp_list_free, NULL);
	sdp_list_free(protos, NULL);
	if (channel <= 0) {
		error("handsfree: unable to get RFCOMM channel from record");
		goto fail;
	}

	io = bt_io_connect(connect_cb, dev, NULL, &gerr,
				BT_IO_OPT_SOURCE_BDADDR, &adapter_addr,
				BT_IO_OPT_DEST_BDADDR, &dev->bdaddr,
				BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_MEDIUM,
				BT_IO_OPT_CHANNEL, channel,
				BT_IO_OPT_INVALID);
	if (!io) {
		error("handsfree: unable to connect: %s", gerr->message);
		g_error_free(gerr);
		goto fail;
	}

	dev->hsp = true;

	g_io_channel_unref(io);
	return;

fail:
	device_destroy(dev);
}

static int sdp_search_hsp(struct hf_device *dev)
{
	uuid_t uuid;

	sdp_uuid16_create(&uuid, HEADSET_SVCLASS_ID);

	return bt_search_service(&adapter_addr, &dev->bdaddr, &uuid,
					sdp_hsp_search_cb, dev, NULL, 0);
}

static void sdp_hfp_search_cb(sdp_list_t *recs, int err, gpointer data)
{
	struct hf_device *dev = data;
	sdp_list_t *protos;
	GError *gerr = NULL;
	GIOChannel *io;
	uuid_t class;
	int channel;

	DBG("");

	if (err < 0) {
		error("handsfree: unable to get SDP record: %s",
								strerror(-err));
		goto fail;
	}

	sdp_uuid16_create(&class, HANDSFREE_SVCLASS_ID);

	/* Find record with proper service class */
	for (; recs; recs = recs->next) {
		sdp_record_t *rec = recs->data;

		if (rec && !sdp_uuid_cmp(&rec->svclass, &class))
			break;
	}

	if (!recs || !recs->data) {
		info("handsfree: no HFP SDP records found, trying HSP");

		if (sdp_search_hsp(dev) < 0) {
			error("handsfree: HSP SDP search failed");
			goto fail;
		}

		return;
	}

	if (sdp_get_access_protos(recs->data, &protos) < 0) {
		error("handsfree: unable to get access protocols from record");
		goto fail;
	}

	channel = sdp_get_proto_port(protos, RFCOMM_UUID);
	sdp_list_foreach(protos, (sdp_list_func_t) sdp_list_free, NULL);
	sdp_list_free(protos, NULL);
	if (channel <= 0) {
		error("handsfree: unable to get RFCOMM channel from record");
		goto fail;
	}

	/* TODO read remote version? */

	io = bt_io_connect(connect_cb, dev, NULL, &gerr,
				BT_IO_OPT_SOURCE_BDADDR, &adapter_addr,
				BT_IO_OPT_DEST_BDADDR, &dev->bdaddr,
				BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_MEDIUM,
				BT_IO_OPT_CHANNEL, channel,
				BT_IO_OPT_INVALID);
	if (!io) {
		error("handsfree: unable to connect: %s", gerr->message);
		g_error_free(gerr);
		goto fail;
	}

	g_io_channel_unref(io);
	return;

fail:
	device_destroy(dev);
}

static int sdp_search_hfp(struct hf_device *dev)
{
	uuid_t uuid;

	sdp_uuid16_create(&uuid, HANDSFREE_SVCLASS_ID);

	return bt_search_service(&adapter_addr, &dev->bdaddr, &uuid,
					sdp_hfp_search_cb, dev, NULL, 0);
}

static void handle_connect(const void *buf, uint16_t len)
{
	const struct hal_cmd_handsfree_connect *cmd = buf;
	struct hf_device *dev;
	char addr[18];
	uint8_t status;
	bdaddr_t bdaddr;
	int ret;

	DBG("");

	android2bdaddr(&cmd->bdaddr, &bdaddr);

	dev = get_device(&bdaddr);
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	if (dev->state != HAL_EV_HANDSFREE_CONN_STATE_DISCONNECTED) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	ba2str(&bdaddr, addr);
	DBG("connecting to %s", addr);

	/* prefer HFP over HSP */
	ret = hfp_server ? sdp_search_hfp(dev) : sdp_search_hsp(dev);
	if (ret < 0) {
		error("handsfree: SDP search failed");
		device_destroy(dev);
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	set_state(dev, HAL_EV_HANDSFREE_CONN_STATE_CONNECTING);

	status = HAL_STATUS_SUCCESS;

failed:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
					HAL_OP_HANDSFREE_CONNECT, status);
}

static void handle_disconnect(const void *buf, uint16_t len)
{
	const struct hal_cmd_handsfree_disconnect *cmd = buf;
	struct hf_device *dev;
	bdaddr_t bdaddr;
	uint8_t status;

	DBG("");

	android2bdaddr(cmd->bdaddr, &bdaddr);

	dev = find_device(&bdaddr);
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	if (dev->state == HAL_EV_HANDSFREE_CONN_STATE_DISCONNECTED) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	if (dev->state == HAL_EV_HANDSFREE_CONN_STATE_DISCONNECTING) {
		status = HAL_STATUS_SUCCESS;
		goto failed;
	}

	if (dev->state == HAL_EV_HANDSFREE_CONN_STATE_CONNECTING) {
		device_destroy(dev);
	} else {
		set_state(dev, HAL_EV_HANDSFREE_CONN_STATE_DISCONNECTING);
		hfp_gw_disconnect(dev->gw);
	}

	status = HAL_STATUS_SUCCESS;

failed:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
					HAL_OP_HANDSFREE_DISCONNECT, status);
}

static bool disconnect_sco(struct hf_device *dev)
{
	if (dev->audio_state == HAL_EV_HANDSFREE_AUDIO_STATE_DISCONNECTED ||
		dev->audio_state == HAL_EV_HANDSFREE_AUDIO_STATE_DISCONNECTING)
		return false;

	bt_sco_disconnect(sco);
	set_audio_state(dev, HAL_EV_HANDSFREE_AUDIO_STATE_DISCONNECTING);

	return true;
}

static bool connect_audio(struct hf_device *dev)
{
	if (dev->audio_state != HAL_EV_HANDSFREE_AUDIO_STATE_DISCONNECTED)
		return false;

	/* we haven't negotiated codec, start selection */
	if (codec_negotiation_supported(dev) && !dev->negotiated_codec) {
		select_codec(dev, 0);
		return true;
	}

	return connect_sco(dev);
}

static void handle_connect_audio(const void *buf, uint16_t len)
{
	const struct hal_cmd_handsfree_connect_audio *cmd = buf;
	struct hf_device *dev;
	bdaddr_t bdaddr;
	uint8_t status;

	DBG("");

	android2bdaddr(cmd->bdaddr, &bdaddr);

	dev = find_device(&bdaddr);
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	if (dev->audio_state != HAL_EV_HANDSFREE_AUDIO_STATE_DISCONNECTED) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	status = connect_audio(dev) ? HAL_STATUS_SUCCESS : HAL_STATUS_FAILED;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
				HAL_OP_HANDSFREE_CONNECT_AUDIO, status);
}

static void handle_disconnect_audio(const void *buf, uint16_t len)
{
	const struct hal_cmd_handsfree_disconnect_audio *cmd = buf;
	struct hf_device *dev;
	bdaddr_t bdaddr;
	uint8_t status;

	DBG("");

	android2bdaddr(cmd->bdaddr, &bdaddr);

	dev = find_device(&bdaddr);
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	status = disconnect_sco(dev) ? HAL_STATUS_SUCCESS : HAL_STATUS_FAILED;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
				HAL_OP_HANDSFREE_DISCONNECT_AUDIO, status);
}

static void handle_start_vr(const void *buf, uint16_t len)
{
	const struct hal_cmd_handsfree_start_vr *cmd = buf;
	struct hf_device *dev;
	bdaddr_t bdaddr;
	uint8_t status;

	DBG("");

	android2bdaddr(cmd->bdaddr, &bdaddr);

	dev = find_device(&bdaddr);
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	if (dev->features & HFP_HF_FEAT_VR) {
		hfp_gw_send_info(dev->gw, "+BVRA: 1");
		status = HAL_STATUS_SUCCESS;
	} else {
		status = HAL_STATUS_FAILED;
	}

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
					HAL_OP_HANDSFREE_START_VR, status);
}

static void handle_stop_vr(const void *buf, uint16_t len)
{
	const struct hal_cmd_handsfree_stop_vr *cmd = buf;
	struct hf_device *dev;
	bdaddr_t bdaddr;
	uint8_t status;

	DBG("");

	android2bdaddr(cmd->bdaddr, &bdaddr);

	dev = find_device(&bdaddr);
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	if (dev->features & HFP_HF_FEAT_VR) {
		hfp_gw_send_info(dev->gw, "+BVRA: 0");
		status = HAL_STATUS_SUCCESS;
	} else {
		status = HAL_STATUS_FAILED;
	}

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
				HAL_OP_HANDSFREE_STOP_VR, status);
}

static void handle_volume_control(const void *buf, uint16_t len)
{
	const struct hal_cmd_handsfree_volume_control *cmd = buf;
	struct hf_device *dev;
	uint8_t status, volume;
	bdaddr_t bdaddr;

	DBG("type=%u volume=%u", cmd->type, cmd->volume);

	android2bdaddr(cmd->bdaddr, &bdaddr);

	dev = find_device(&bdaddr);
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	volume = cmd->volume > 15 ? 15 : cmd->volume;

	switch (cmd->type) {
	case HAL_HANDSFREE_VOLUME_TYPE_MIC:
		hfp_gw_send_info(dev->gw, "+VGM: %u", volume);

		status = HAL_STATUS_SUCCESS;
		break;
	case HAL_HANDSFREE_VOLUME_TYPE_SPEAKER:
		hfp_gw_send_info(dev->gw, "+VGS: %u", volume);

		status = HAL_STATUS_SUCCESS;
		break;
	default:
		status = HAL_STATUS_FAILED;
		break;
	}

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
				HAL_OP_HANDSFREE_VOLUME_CONTROL, status);
}

static void update_indicator(struct hf_device *dev, int ind, uint8_t val)
{
	DBG("ind=%u new=%u old=%u", ind, val, dev->inds[ind].val);

	if (dev->inds[ind].val == val)
		return;

	dev->inds[ind].val = val;

	if (!dev->indicators_enabled)
		return;

	if (!dev->inds[ind].active)
		return;

	/* indicator numbers in CIEV start from 1 */
	hfp_gw_send_info(dev->gw, "+CIEV: %u,%u", ind + 1, val);
}

static void device_status_notif(void *data, void *user_data)
{
	struct hf_device *dev = data;
	struct hal_cmd_handsfree_device_status_notif *cmd = user_data;

	update_indicator(dev, IND_SERVICE, cmd->state);
	update_indicator(dev, IND_ROAM, cmd->type);
	update_indicator(dev, IND_SIGNAL, cmd->signal);
	update_indicator(dev, IND_BATTCHG, cmd->battery);
}

static void handle_device_status_notif(const void *buf, uint16_t len)
{
	const struct hal_cmd_handsfree_device_status_notif *cmd = buf;
	uint8_t status;

	DBG("");

	if (queue_isempty(devices)) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	/* Cast cmd to void as queue api needs that */
	queue_foreach(devices, device_status_notif, (void *) cmd);

	status = HAL_STATUS_SUCCESS;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
				HAL_OP_HANDSFREE_DEVICE_STATUS_NOTIF, status);
}

static void handle_cops(const void *buf, uint16_t len)
{
	const struct hal_cmd_handsfree_cops_response *cmd = buf;
	struct hf_device *dev;
	bdaddr_t bdaddr;
	uint8_t status;

	if (len != sizeof(*cmd) + cmd->len ||
			(cmd->len != 0 && cmd->buf[cmd->len - 1] != '\0')) {
		error("Invalid cops response command, terminating");
		raise(SIGTERM);
		return;
	}

	DBG("");

	android2bdaddr(cmd->bdaddr, &bdaddr);

	dev = find_device(&bdaddr);
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	hfp_gw_send_info(dev->gw, "+COPS: 0,0,\"%.16s\"",
					cmd->len ? (char *) cmd->buf : "");

	hfp_gw_send_result(dev->gw, HFP_RESULT_OK);

	status = HAL_STATUS_SUCCESS;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
				HAL_OP_HANDSFREE_COPS_RESPONSE, status);
}

static unsigned int get_callsetup(uint8_t state)
{
	switch (state) {
	case HAL_HANDSFREE_CALL_STATE_INCOMING:
		return 1;
	case HAL_HANDSFREE_CALL_STATE_DIALING:
		return 2;
	case HAL_HANDSFREE_CALL_STATE_ALERTING:
		return 3;
	default:
		return 0;
	}
}

static void handle_cind(const void *buf, uint16_t len)
{
	const struct hal_cmd_handsfree_cind_response *cmd = buf;
	struct hf_device *dev;
	bdaddr_t bdaddr;
	uint8_t status;

	DBG("");

	android2bdaddr(cmd->bdaddr, &bdaddr);

	dev = find_device(&bdaddr);
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	/* HAL doesn't provide indicators values so need to convert here */
	dev->inds[IND_SERVICE].val = cmd->svc;
	dev->inds[IND_CALL].val = !!(cmd->num_active + cmd->num_held);
	dev->inds[IND_CALLSETUP].val = get_callsetup(cmd->state);
	dev->inds[IND_CALLHELD].val = cmd->num_held ?
						(cmd->num_active ? 1 : 2) : 0;
	dev->inds[IND_SIGNAL].val = cmd->signal;
	dev->inds[IND_ROAM].val = cmd->roam;
	dev->inds[IND_BATTCHG].val = cmd->batt_chg;

	/* Order must match indicators_defaults table */
	hfp_gw_send_info(dev->gw, "+CIND: %u,%u,%u,%u,%u,%u,%u",
						dev->inds[IND_SERVICE].val,
						dev->inds[IND_CALL].val,
						dev->inds[IND_CALLSETUP].val,
						dev->inds[IND_CALLHELD].val,
						dev->inds[IND_SIGNAL].val,
						dev->inds[IND_ROAM].val,
						dev->inds[IND_BATTCHG].val);

	hfp_gw_send_result(dev->gw, HFP_RESULT_OK);

	status = HAL_STATUS_SUCCESS;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
				HAL_OP_HANDSFREE_CIND_RESPONSE, status);
}

static void handle_formatted_at_resp(const void *buf, uint16_t len)
{
	const struct hal_cmd_handsfree_formatted_at_response *cmd = buf;
	struct hf_device *dev;
	bdaddr_t bdaddr;
	uint8_t status;

	DBG("");

	if (len != sizeof(*cmd) + cmd->len ||
			(cmd->len != 0 && cmd->buf[cmd->len - 1] != '\0')) {
		error("Invalid formatted AT response command, terminating");
		raise(SIGTERM);
		return;
	}

	android2bdaddr(cmd->bdaddr, &bdaddr);

	dev = find_device(&bdaddr);
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	hfp_gw_send_info(dev->gw, "%s", cmd->len ? (char *) cmd->buf : "");

	status = HAL_STATUS_SUCCESS;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
			HAL_OP_HANDSFREE_FORMATTED_AT_RESPONSE, status);
}

static void handle_at_resp(const void *buf, uint16_t len)
{
	const struct hal_cmd_handsfree_at_response *cmd = buf;
	struct hf_device *dev;
	bdaddr_t bdaddr;
	uint8_t status;

	DBG("");

	android2bdaddr(cmd->bdaddr, &bdaddr);

	dev = find_device(&bdaddr);
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	if (cmd->response == HAL_HANDSFREE_AT_RESPONSE_OK)
		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);
	else if (dev->cmee_enabled)
		hfp_gw_send_error(dev->gw, cmd->error);
	else
		hfp_gw_send_result(dev->gw, HFP_RESULT_ERROR);

	status = HAL_STATUS_SUCCESS;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
					HAL_OP_HANDSFREE_AT_RESPONSE, status);
}

static void handle_clcc_resp(const void *buf, uint16_t len)
{
	const struct hal_cmd_handsfree_clcc_response *cmd = buf;
	struct hf_device *dev;
	uint8_t status;
	bdaddr_t bdaddr;
	char *number;

	if (len != sizeof(*cmd) + cmd->number_len || (cmd->number_len != 0 &&
				cmd->number[cmd->number_len - 1] != '\0')) {
		error("Invalid CLCC response command, terminating");
		raise(SIGTERM);
		return;
	}

	DBG("");

	android2bdaddr(cmd->bdaddr, &bdaddr);

	dev = find_device(&bdaddr);
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	if (!cmd->index) {
		hfp_gw_send_result(dev->gw, HFP_RESULT_OK);

		status = HAL_STATUS_SUCCESS;
		goto done;
	}

	number = cmd->number_len ? (char *) cmd->number : "";

	switch (cmd->state) {
	case HAL_HANDSFREE_CALL_STATE_INCOMING:
	case HAL_HANDSFREE_CALL_STATE_WAITING:
	case HAL_HANDSFREE_CALL_STATE_ACTIVE:
	case HAL_HANDSFREE_CALL_STATE_HELD:
	case HAL_HANDSFREE_CALL_STATE_DIALING:
	case HAL_HANDSFREE_CALL_STATE_ALERTING:
		if (cmd->type == HAL_HANDSFREE_CALL_ADDRTYPE_INTERNATIONAL &&
							number[0] != '+')
			hfp_gw_send_info(dev->gw,
					"+CLCC: %u,%u,%u,%u,%u,\"+%s\",%u",
					cmd->index, cmd->dir, cmd->state,
					cmd->mode, cmd->mpty, number,
					cmd->type);
		else
			hfp_gw_send_info(dev->gw,
					"+CLCC: %u,%u,%u,%u,%u,\"%s\",%u",
					cmd->index, cmd->dir, cmd->state,
					cmd->mode, cmd->mpty, number,
					cmd->type);

		status = HAL_STATUS_SUCCESS;
		break;
	case HAL_HANDSFREE_CALL_STATE_IDLE:
	default:
		status = HAL_STATUS_FAILED;
		break;
	}

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
				HAL_OP_HANDSFREE_CLCC_RESPONSE, status);
}

static gboolean ring_cb(gpointer user_data)
{
	struct hf_device *dev = user_data;

	hfp_gw_send_info(dev->gw, "RING");

	if (dev->clip_enabled && dev->clip)
		hfp_gw_send_info(dev->gw, "%s", dev->clip);

	return TRUE;
}

static void phone_state_dialing(struct hf_device *dev, int num_active,
								int num_held)
{
	if (dev->call_hanging_up) {
		g_source_remove(dev->call_hanging_up);
		dev->call_hanging_up = 0;
	}

	update_indicator(dev, IND_CALLSETUP, 2);

	if (num_active == 0 && num_held > 0)
		update_indicator(dev, IND_CALLHELD, 2);

	if (dev->num_active == 0 && dev->num_held == 0)
		connect_audio(dev);
}

static void phone_state_alerting(struct hf_device *dev, int num_active,
								int num_held)
{
	if (dev->call_hanging_up) {
		g_source_remove(dev->call_hanging_up);
		dev->call_hanging_up = 0;
	}

	update_indicator(dev, IND_CALLSETUP, 3);
}

static void phone_state_waiting(struct hf_device *dev, int num_active,
					int num_held, uint8_t type,
					const uint8_t *number, int number_len)
{
	char *num;

	if (!dev->ccwa_enabled)
		return;

	num = number_len ? (char *) number : "";

	if (type == HAL_HANDSFREE_CALL_ADDRTYPE_INTERNATIONAL && num[0] != '+')
		hfp_gw_send_info(dev->gw, "+CCWA: \"+%s\",%u", num, type);
	else
		hfp_gw_send_info(dev->gw, "+CCWA: \"%s\",%u", num, type);

	update_indicator(dev, IND_CALLSETUP, 1);
}

static void phone_state_incoming(struct hf_device *dev, int num_active,
					int num_held, uint8_t type,
					const uint8_t *number, int number_len)
{
	char *num;

	if (dev->setup_state == HAL_HANDSFREE_CALL_STATE_INCOMING) {
		if (dev->num_active != num_active ||
						dev->num_held != num_held) {
			if (dev->num_active == num_held &&
						dev->num_held == num_active)
				return;
			/*
			 * calls changed while waiting call ie. due to
			 * termination of active call
			 */
			update_indicator(dev, IND_CALLHELD,
					num_held ? (num_active ? 1 : 2) : 0);
			update_indicator(dev, IND_CALL,
						!!(num_active + num_held));
		}

		return;
	}

	if (dev->call_hanging_up)
		return;

	if (num_active > 0 || num_held > 0) {
		phone_state_waiting(dev, num_active, num_held, type, number,
								number_len);
		return;
	}

	update_indicator(dev, IND_CALLSETUP, 1);

	num = number_len ? (char *) number : "";

	if (type == HAL_HANDSFREE_CALL_ADDRTYPE_INTERNATIONAL && num[0] != '+')
		dev->clip = g_strdup_printf("+CLIP: \"+%s\",%u", num, type);
	else
		dev->clip = g_strdup_printf("+CLIP: \"%s\",%u", num, type);

	/* send first RING */
	ring_cb(dev);

	dev->ring = g_timeout_add_seconds_full(G_PRIORITY_DEFAULT,
							RING_TIMEOUT, ring_cb,
							dev, NULL);
	if (!dev->ring) {
		g_free(dev->clip);
		dev->clip = NULL;
	}
}

static gboolean hang_up_cb(gpointer user_data)
{
	struct hf_device *dev = user_data;

	DBG("");

	dev->call_hanging_up = 0;

	return FALSE;
}

static void phone_state_idle(struct hf_device *dev, int num_active,
								int num_held)
{
	if (dev->ring) {
		g_source_remove(dev->ring);
		dev->ring = 0;

		if (dev->clip) {
			g_free(dev->clip);
			dev->clip = NULL;
		}
	}

	switch (dev->setup_state) {
	case HAL_HANDSFREE_CALL_STATE_INCOMING:
		if (num_active > dev->num_active) {
			update_indicator(dev, IND_CALL, 1);

			if (dev->num_active == 0 && dev->num_held == 0)
				connect_audio(dev);
		}

		if (num_held >= dev->num_held && num_held != 0)
			update_indicator(dev, IND_CALLHELD, 1);

		update_indicator(dev, IND_CALLSETUP, 0);

		if (num_active == 0 && num_held == 0 &&
				num_active == dev->num_active &&
				num_held == dev->num_held)
			dev->call_hanging_up = g_timeout_add(800, hang_up_cb,
									dev);
		break;
	case HAL_HANDSFREE_CALL_STATE_DIALING:
	case HAL_HANDSFREE_CALL_STATE_ALERTING:
		if (num_active > dev->num_active)
			update_indicator(dev, IND_CALL, 1);

		update_indicator(dev, IND_CALLHELD,
					num_held ? (num_active ? 1 : 2) : 0);

		update_indicator(dev, IND_CALLSETUP, 0);

		/* disconnect SCO if we hang up while dialing or alerting */
		if (num_active == 0 && num_held == 0)
			disconnect_sco(dev);
		break;
	case HAL_HANDSFREE_CALL_STATE_IDLE:
		if (dev->call_hanging_up) {
			g_source_remove(dev->call_hanging_up);
			dev->call_hanging_up = 0;
			return;
		}

		/* check if calls swapped */
		if (num_held != 0 && num_active != 0 &&
				dev->num_active == num_held &&
				dev->num_held == num_active) {
			/* TODO better way for forcing indicator */
			dev->inds[IND_CALLHELD].val = 0;
		} else if ((num_active > 0 || num_held > 0) &&
						dev->num_active == 0 &&
						dev->num_held == 0) {
			/*
			 * If number of active or held calls change but there
			 * was no call setup change this means that there were
			 * calls present when headset was connected.
			 */
			connect_audio(dev);
		} else if (num_active == 0 && num_held == 0) {
			disconnect_sco(dev);
		}

		update_indicator(dev, IND_CALLHELD,
					num_held ? (num_active ? 1 : 2) : 0);
		update_indicator(dev, IND_CALL, !!(num_active + num_held));
		update_indicator(dev, IND_CALLSETUP, 0);

		/* If call was terminated due to carrier lost send NO CARRIER */
		if (num_active == 0 && num_held == 0 &&
				dev->inds[IND_SERVICE].val == 0 &&
				(dev->num_active > 0 || dev->num_held > 0))
			hfp_gw_send_info(dev->gw, "NO CARRIER");

		break;
	default:
		DBG("unhandled state %u", dev->setup_state);
		break;
	}
}

static void phone_state_change(void *data, void *user_data)
{
	struct hf_device *dev = data;
	struct hal_cmd_handsfree_phone_state_change *cmd = user_data;

	switch (cmd->state) {
	case HAL_HANDSFREE_CALL_STATE_DIALING:
		phone_state_dialing(dev, cmd->num_active, cmd->num_held);
		break;
	case HAL_HANDSFREE_CALL_STATE_ALERTING:
		phone_state_alerting(dev, cmd->num_active, cmd->num_held);
		break;
	case HAL_HANDSFREE_CALL_STATE_INCOMING:
		phone_state_incoming(dev, cmd->num_active, cmd->num_held,
						cmd->type, cmd->number,
						cmd->number_len);
		break;
	case HAL_HANDSFREE_CALL_STATE_IDLE:
		phone_state_idle(dev, cmd->num_active, cmd->num_held);
		break;
	default:
		DBG("unhandled new state %u (current state %u)", cmd->state,
							dev->setup_state);

		return;
	}

	dev->num_active = cmd->num_active;
	dev->num_held = cmd->num_held;
	dev->setup_state = cmd->state;

}

static void handle_phone_state_change(const void *buf, uint16_t len)
{
	const struct hal_cmd_handsfree_phone_state_change *cmd = buf;
	uint8_t status;

	if (len != sizeof(*cmd) + cmd->number_len || (cmd->number_len != 0 &&
				cmd->number[cmd->number_len - 1] != '\0')) {
		error("Invalid phone state change command, terminating");
		raise(SIGTERM);
		return;
	}

	DBG("active=%u hold=%u state=%u", cmd->num_active, cmd->num_held,
								cmd->state);

	if (queue_isempty(devices)) {
		status = HAL_STATUS_FAILED;
		goto failed;
	}

	/* Cast cmd to void as queue api needs that */
	queue_foreach(devices, phone_state_change, (void *) cmd);

	status = HAL_STATUS_SUCCESS;

failed:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
				HAL_OP_HANDSFREE_PHONE_STATE_CHANGE, status);
}

static void handle_configure_wbs(const void *buf, uint16_t len)
{
	const struct hal_cmd_handsfree_configure_wbs *cmd = buf;
	struct hf_device *dev;
	bdaddr_t bdaddr;
	uint8_t status;

	if (!(hfp_ag_features & HFP_AG_FEAT_CODEC)) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	android2bdaddr(cmd->bdaddr, &bdaddr);

	dev = find_device(&bdaddr);
	if (!dev) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	if (dev->audio_state != HAL_EV_HANDSFREE_AUDIO_STATE_DISCONNECTED) {
		status = HAL_STATUS_FAILED;
		goto done;
	}

	switch (cmd->config) {
	case HAL_HANDSFREE_WBS_NO:
		dev->codecs[MSBC_OFFSET].local_supported = false;
		break;
	case HAL_HANDSFREE_WBS_YES:
		dev->codecs[MSBC_OFFSET].local_supported = true;
		break;
	case HAL_HANDSFREE_WBS_NONE:
		/* TODO */
	default:
		status = HAL_STATUS_FAILED;
		goto done;
	}

	/*
	 * cleanup negotiated codec if WBS support was changed, it will be
	 * renegotiated on next audio connection based on currently supported
	 * codecs
	 */
	dev->negotiated_codec = 0;
	status = HAL_STATUS_SUCCESS;

done:
	ipc_send_rsp(hal_ipc, HAL_SERVICE_ID_HANDSFREE,
					HAL_OP_HANDSFREE_CONFIGURE_WBS, status);
}

static const struct ipc_handler cmd_handlers[] = {
	/* HAL_OP_HANDSFREE_CONNECT */
	{ handle_connect, false,
		sizeof(struct hal_cmd_handsfree_connect) },
	/* HAL_OP_HANDSFREE_DISCONNECT */
	{ handle_disconnect, false,
		sizeof(struct hal_cmd_handsfree_disconnect) },
	/* HAL_OP_HANDSFREE_CONNECT_AUDIO */
	{ handle_connect_audio, false,
		sizeof(struct hal_cmd_handsfree_connect_audio) },
	/* HAL_OP_HANDSFREE_DISCONNECT_AUDIO */
	{ handle_disconnect_audio, false,
		sizeof(struct hal_cmd_handsfree_disconnect_audio) },
	/* define HAL_OP_HANDSFREE_START_VR */
	{ handle_start_vr, false, sizeof(struct hal_cmd_handsfree_start_vr) },
	/* define HAL_OP_HANDSFREE_STOP_VR */
	{ handle_stop_vr, false, sizeof(struct hal_cmd_handsfree_stop_vr) },
	/* HAL_OP_HANDSFREE_VOLUME_CONTROL */
	{ handle_volume_control, false,
		sizeof(struct hal_cmd_handsfree_volume_control) },
	/* HAL_OP_HANDSFREE_DEVICE_STATUS_NOTIF */
	{ handle_device_status_notif, false,
		sizeof(struct hal_cmd_handsfree_device_status_notif) },
	/* HAL_OP_HANDSFREE_COPS_RESPONSE */
	{ handle_cops, true,
		sizeof(struct hal_cmd_handsfree_cops_response) },
	/* HAL_OP_HANDSFREE_CIND_RESPONSE */
	{ handle_cind, false,
		sizeof(struct hal_cmd_handsfree_cind_response) },
	/* HAL_OP_HANDSFREE_FORMATTED_AT_RESPONSE */
	{ handle_formatted_at_resp, true,
		sizeof(struct hal_cmd_handsfree_formatted_at_response) },
	/* HAL_OP_HANDSFREE_AT_RESPONSE */
	{ handle_at_resp, false,
		sizeof(struct hal_cmd_handsfree_at_response) },
	/* HAL_OP_HANDSFREE_CLCC_RESPONSE */
	{ handle_clcc_resp, true,
		sizeof(struct hal_cmd_handsfree_clcc_response) },
	/* HAL_OP_HANDSFREE_PHONE_STATE_CHANGE */
	{ handle_phone_state_change, true,
		sizeof(struct hal_cmd_handsfree_phone_state_change) },
	/* HAL_OP_HANDSFREE_CONFIGURE_WBS */
	{ handle_configure_wbs, false,
		sizeof(struct hal_cmd_handsfree_configure_wbs) },
};

static sdp_record_t *headset_ag_record(void)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, svclass_uuid, ga_svclass_uuid;
	uuid_t l2cap_uuid, rfcomm_uuid;
	sdp_profile_desc_t profile;
	sdp_list_t *aproto, *proto[2];
	sdp_record_t *record;
	sdp_data_t *channel;
	uint8_t netid = 0x01;
	sdp_data_t *network;
	uint8_t ch = HSP_AG_CHANNEL;

	record = sdp_record_alloc();
	if (!record)
		return NULL;

	network = sdp_data_alloc(SDP_UINT8, &netid);
	if (!network) {
		sdp_record_free(record);
		return NULL;
	}

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(record, root);

	sdp_uuid16_create(&svclass_uuid, HEADSET_AGW_SVCLASS_ID);
	svclass_id = sdp_list_append(NULL, &svclass_uuid);
	sdp_uuid16_create(&ga_svclass_uuid, GENERIC_AUDIO_SVCLASS_ID);
	svclass_id = sdp_list_append(svclass_id, &ga_svclass_uuid);
	sdp_set_service_classes(record, svclass_id);

	sdp_uuid16_create(&profile.uuid, HEADSET_PROFILE_ID);
	profile.version = 0x0102;
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

	aproto = sdp_list_append(NULL, apseq);
	sdp_set_access_protos(record, aproto);

	sdp_set_info_attr(record, "Voice Gateway", NULL, NULL);

	sdp_attr_add(record, SDP_ATTR_EXTERNAL_NETWORK, network);

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

static bool confirm_sco_cb(const bdaddr_t *addr, uint16_t *voice_settings)
{
	char address[18];
	struct hf_device *dev;

	ba2str(addr, address);

	DBG("incoming SCO connection from %s", address);

	dev = find_device(addr);
	if (!dev || dev->state != HAL_EV_HANDSFREE_CONN_STATE_SLC_CONNECTED) {
		error("handsfree: audio connection from %s rejected", address);
		return false;
	}

	/* If HF initiate SCO there must be no WBS used */
	*voice_settings = 0;

	set_audio_state(dev, HAL_EV_HANDSFREE_AUDIO_STATE_CONNECTING);
	return true;
}

static bool enable_hsp_ag(void)
{
	sdp_record_t *rec;
	GError *err = NULL;

	DBG("");

	hsp_server =  bt_io_listen(NULL, confirm_cb, GINT_TO_POINTER(true),
					NULL, &err,
					BT_IO_OPT_SOURCE_BDADDR, &adapter_addr,
					BT_IO_OPT_CHANNEL, HSP_AG_CHANNEL,
					BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_MEDIUM,
					BT_IO_OPT_INVALID);
	if (!hsp_server) {
		error("Failed to listen on Headset rfcomm: %s", err->message);
		g_error_free(err);
		return false;
	}

	rec = headset_ag_record();
	if (!rec) {
		error("Failed to allocate Headset record");
		goto failed;
	}

	if (bt_adapter_add_record(rec, 0) < 0) {
		error("Failed to register Headset record");
		sdp_record_free(rec);
		goto failed;
	}

	hsp_record_id = rec->handle;
	return true;

failed:
	g_io_channel_shutdown(hsp_server, TRUE, NULL);
	g_io_channel_unref(hsp_server);
	hsp_server = NULL;

	return false;
}

static void cleanup_hsp_ag(void)
{
	if (hsp_server) {
		g_io_channel_shutdown(hsp_server, TRUE, NULL);
		g_io_channel_unref(hsp_server);
		hsp_server = NULL;
	}

	if (hsp_record_id > 0) {
		bt_adapter_remove_record(hsp_record_id);
		hsp_record_id = 0;
	}
}

static sdp_record_t *hfp_ag_record(void)
{
	sdp_list_t *svclass_id, *pfseq, *apseq, *root;
	uuid_t root_uuid, svclass_uuid, ga_svclass_uuid;
	uuid_t l2cap_uuid, rfcomm_uuid;
	sdp_profile_desc_t profile;
	sdp_list_t *aproto, *proto[2];
	sdp_record_t *record;
	sdp_data_t *channel, *features;
	uint8_t netid = 0x01;
	uint16_t sdpfeat;
	sdp_data_t *network;
	uint8_t ch = HFP_AG_CHANNEL;

	record = sdp_record_alloc();
	if (!record)
		return NULL;

	network = sdp_data_alloc(SDP_UINT8, &netid);
	if (!network) {
		sdp_record_free(record);
		return NULL;
	}

	sdp_uuid16_create(&root_uuid, PUBLIC_BROWSE_GROUP);
	root = sdp_list_append(NULL, &root_uuid);
	sdp_set_browse_groups(record, root);

	sdp_uuid16_create(&svclass_uuid, HANDSFREE_AGW_SVCLASS_ID);
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
	sdpfeat = hfp_ag_features & 0x0000003F;
	if (hfp_ag_features & HFP_AG_FEAT_CODEC)
		sdpfeat |= 0x00000020;
	else
		sdpfeat &= ~0x00000020;

	features = sdp_data_alloc(SDP_UINT16, &sdpfeat);
	sdp_attr_add(record, SDP_ATTR_SUPPORTED_FEATURES, features);

	aproto = sdp_list_append(NULL, apseq);
	sdp_set_access_protos(record, aproto);

	sdp_set_info_attr(record, "Hands-Free Audio Gateway", NULL, NULL);

	sdp_attr_add(record, SDP_ATTR_EXTERNAL_NETWORK, network);

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

static bool enable_hfp_ag(void)
{
	sdp_record_t *rec;
	GError *err = NULL;

	DBG("");

	if (hfp_server)
		return false;

	hfp_server =  bt_io_listen(NULL, confirm_cb, GINT_TO_POINTER(false),
					NULL, &err,
					BT_IO_OPT_SOURCE_BDADDR, &adapter_addr,
					BT_IO_OPT_CHANNEL, HFP_AG_CHANNEL,
					BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_MEDIUM,
					BT_IO_OPT_INVALID);
	if (!hfp_server) {
		error("Failed to listen on Handsfree rfcomm: %s", err->message);
		g_error_free(err);
		return false;
	}

	rec = hfp_ag_record();
	if (!rec) {
		error("Failed to allocate Handsfree record");
		goto failed;
	}

	if (bt_adapter_add_record(rec, 0) < 0) {
		error("Failed to register Handsfree record");
		sdp_record_free(rec);
		goto failed;
	}

	hfp_record_id = rec->handle;
	return true;

failed:
	g_io_channel_shutdown(hfp_server, TRUE, NULL);
	g_io_channel_unref(hfp_server);
	hfp_server = NULL;

	return false;
}

static void cleanup_hfp_ag(void)
{
	if (hfp_server) {
		g_io_channel_shutdown(hfp_server, TRUE, NULL);
		g_io_channel_unref(hfp_server);
		hfp_server = NULL;
	}

	if (hfp_record_id > 0) {
		bt_adapter_remove_record(hfp_record_id);
		hfp_record_id = 0;
	}
}

static void bt_sco_get_fd(const void *buf, uint16_t len)
{
	const struct sco_cmd_get_fd *cmd = buf;
	struct sco_rsp_get_fd rsp;
	struct hf_device *dev;
	bdaddr_t bdaddr;
	int fd;

	DBG("");

	android2bdaddr(cmd->bdaddr, &bdaddr);

	dev = find_device(&bdaddr);
	if (!dev || !bt_sco_get_fd_and_mtu(sco, &fd, &rsp.mtu))
		goto failed;

	DBG("fd %d mtu %u", fd, rsp.mtu);

	ipc_send_rsp_full(sco_ipc, SCO_SERVICE_ID, SCO_OP_GET_FD,
							sizeof(rsp), &rsp, fd);

	return;

failed:
	ipc_send_rsp(sco_ipc, SCO_SERVICE_ID, SCO_OP_STATUS, SCO_STATUS_FAILED);
}

static const struct ipc_handler sco_handlers[] = {
	/* SCO_OP_GET_FD */
	{ bt_sco_get_fd, false, sizeof(struct sco_cmd_get_fd) }
};

static void bt_sco_unregister(void)
{
	DBG("");

	ipc_cleanup(sco_ipc);
	sco_ipc = NULL;
}

static bool bt_sco_register(ipc_disconnect_cb disconnect)
{
	DBG("");

	sco_ipc = ipc_init(BLUEZ_SCO_SK_PATH, sizeof(BLUEZ_SCO_SK_PATH),
				SCO_SERVICE_ID, false, disconnect, NULL);
	if (!sco_ipc)
		return false;

	ipc_register(sco_ipc, SCO_SERVICE_ID, sco_handlers,
						G_N_ELEMENTS(sco_handlers));

	return true;
}

bool bt_handsfree_register(struct ipc *ipc, const bdaddr_t *addr, uint8_t mode,
								int max_clients)
{
	DBG("mode 0x%x max_clients %d", mode, max_clients);

	bacpy(&adapter_addr, addr);

	if (max_clients < 1)
		return false;

	devices = queue_new();

	if (!enable_hsp_ag())
		goto failed_queue;

	sco = bt_sco_new(addr);
	if (!sco)
		goto failed_hsp;

	bt_sco_set_confirm_cb(sco, confirm_sco_cb);
	bt_sco_set_connect_cb(sco, connect_sco_cb);
	bt_sco_set_disconnect_cb(sco, disconnect_sco_cb);

	if (mode == HAL_MODE_HANDSFREE_HSP_ONLY)
		goto done;

	hfp_ag_features = HFP_AG_FEATURES;

	if (mode == HAL_MODE_HANDSFREE_HFP_WBS)
		hfp_ag_features |= HFP_AG_FEAT_CODEC;

	if (enable_hfp_ag())
		goto done;

	bt_sco_unref(sco);
	sco = NULL;
	hfp_ag_features = 0;
failed_hsp:
	cleanup_hsp_ag();
failed_queue:
	queue_destroy(devices, NULL);
	devices = NULL;

	return false;

done:
	hal_ipc = ipc;
	ipc_register(hal_ipc, HAL_SERVICE_ID_HANDSFREE, cmd_handlers,
						G_N_ELEMENTS(cmd_handlers));

	bt_sco_register(NULL);

	max_hfp_clients = max_clients;

	return true;
}

void bt_handsfree_unregister(void)
{
	DBG("");

	bt_sco_unregister();
	ipc_unregister(hal_ipc, HAL_SERVICE_ID_HANDSFREE);
	hal_ipc = NULL;

	cleanup_hfp_ag();
	cleanup_hsp_ag();
	bt_sco_unref(sco);
	sco = NULL;

	hfp_ag_features = 0;

	queue_destroy(devices, (queue_destroy_func_t) device_destroy);
	devices = NULL;

	max_hfp_clients = 0;
}

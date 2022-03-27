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
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <cutils/properties.h>

#include "hal-log.h"
#include "hal.h"
#include "hal-msg.h"
#include "ipc-common.h"
#include "hal-ipc.h"

static const bthf_client_callbacks_t *cbs = NULL;

static bool interface_ready(void)
{
	return cbs != NULL;
}

static void handle_conn_state(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hf_client_conn_state *ev = buf;

	if (cbs->connection_state_cb)
		cbs->connection_state_cb(ev->state, ev->peer_feat,
						ev->chld_feat,
						(bt_bdaddr_t *) ev->bdaddr);
}

static void handle_audio_state(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hf_client_audio_state *ev = buf;

	if (cbs->audio_state_cb)
		cbs->audio_state_cb(ev->state, (bt_bdaddr_t *) (ev->bdaddr));
}

static void handle_vr_state(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hf_client_vr_state *ev = buf;

	if (cbs->vr_cmd_cb)
		cbs->vr_cmd_cb(ev->state);
}

static void handle_network_state(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hf_client_net_state *ev = buf;

	if (cbs->network_state_cb)
		cbs->network_state_cb(ev->state);
}

static void handle_network_roaming(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hf_client_net_roaming_type *ev = buf;

	if (cbs->network_roaming_cb)
		cbs->network_roaming_cb(ev->state);
}

static void handle_network_signal(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hf_client_net_signal_strength *ev = buf;

	if (cbs->network_signal_cb)
		cbs->network_signal_cb(ev->signal_strength);
}

static void handle_battery_level(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hf_client_battery_level *ev = buf;

	if (cbs->battery_level_cb)
		cbs->battery_level_cb(ev->battery_level);
}

static void handle_operator_name(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hf_client_operator_name *ev = buf;
	uint16_t name_len = ev->name_len;
	char *name = NULL;

	if (len != sizeof(*ev) + name_len ||
		(name_len != 0 && ev->name[name_len - 1] != '\0')) {
		error("invalid operator name, aborting");
		exit(EXIT_FAILURE);
	}

	if (name_len)
		name = (char *) ev->name;

	if (cbs->current_operator_cb)
		cbs->current_operator_cb(name);
}

static void handle_call(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hf_client_call_indicator *ev = buf;

	if (cbs->call_cb)
		cbs->call_cb(ev->call);
}

static void handle_call_setup(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hf_client_call_setup_indicator *ev = buf;

	if (cbs->callsetup_cb)
		cbs->callsetup_cb(ev->call_setup);
}

static void handle_call_held(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hf_client_call_held_indicator *ev = buf;

	if (cbs->callheld_cb)
		cbs->callheld_cb(ev->call_held);
}

static void handle_response_and_hold(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hf_client_response_and_hold_status *ev = buf;

	if (cbs->resp_and_hold_cb)
		cbs->resp_and_hold_cb(ev->status);
}

static void handle_clip(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hf_client_calling_line_ident *ev = buf;
	uint16_t num_len = ev->number_len;
	char *number = NULL;

	if (len != sizeof(*ev) + num_len ||
		(num_len != 0 && ev->number[num_len - 1] != '\0')) {
		error("invalid  clip, aborting");
		exit(EXIT_FAILURE);
	}

	if (num_len)
		number = (char *) ev->number;

	if (cbs->clip_cb)
		cbs->clip_cb(number);
}

static void handle_call_waiting(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hf_client_call_waiting *ev = buf;
	uint16_t num_len = ev->number_len;
	char *number = NULL;

	if (len != sizeof(*ev) + num_len ||
		(num_len != 0 && ev->number[num_len - 1] != '\0')) {
		error("invalid call waiting, aborting");
		exit(EXIT_FAILURE);
	}

	if (num_len)
		number = (char *) ev->number;

	if (cbs->call_waiting_cb)
		cbs->call_waiting_cb(number);
}

static void handle_current_calls(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hf_client_current_call *ev = buf;
	uint16_t num_len = ev->number_len;
	char *number = NULL;

	if (len != sizeof(*ev) + num_len ||
		(num_len != 0 && ev->number[num_len - 1] != '\0')) {
		error("invalid current calls, aborting");
		exit(EXIT_FAILURE);
	}

	if (num_len)
		number = (char *) ev->number;

	if (cbs->current_calls_cb)
		cbs->current_calls_cb(ev->index, ev->direction, ev->call_state,
							ev->multiparty, number);
}

static void handle_volume_change(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hf_client_volume_changed *ev = buf;

	if (cbs->volume_change_cb)
		cbs->volume_change_cb(ev->type, ev->volume);
}

static void handle_command_cmp(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hf_client_command_complete *ev = buf;

	if (cbs->cmd_complete_cb)
		cbs->cmd_complete_cb(ev->type, ev->cme);
}

static void handle_subscriber_info(void *buf, uint16_t len, int fd)
{
	const struct hal_ev_hf_client_subscriber_service_info *ev = buf;
	uint16_t name_len = ev->name_len;
	char *name = NULL;

	if (len != sizeof(*ev) + name_len ||
		(name_len != 0 && ev->name[name_len - 1] != '\0')) {
		error("invalid sunscriber info, aborting");
		exit(EXIT_FAILURE);
	}

	if (name_len)
		name = (char *) ev->name;

	if (cbs->subscriber_info_cb)
		cbs->subscriber_info_cb(name, ev->type);
}

static void handle_in_band_ringtone(void *buf, uint16_t len, int fd)
{
	struct hal_ev_hf_client_inband_settings *ev = buf;

	if (cbs->in_band_ring_tone_cb)
		cbs->in_band_ring_tone_cb(ev->state);
}

static void handle_last_voice_tag_number(void *buf, uint16_t len, int fd)
{
	const struct hal_ev_hf_client_last_void_call_tag_num *ev = buf;
	char *number = NULL;
	uint16_t num_len = ev->number_len;

	if (len != sizeof(*ev) + num_len ||
		(num_len != 0 && ev->number[num_len - 1] != '\0')) {
		error("invalid voice tag, aborting");
		exit(EXIT_FAILURE);
	}

	if (num_len)
		number = (char *) ev->number;

	if (cbs->last_voice_tag_number_callback)
		cbs->last_voice_tag_number_callback(number);
}

static void handle_ring_indication(void *buf, uint16_t len, int fd)
{
	if (cbs->ring_indication_cb)
		cbs->ring_indication_cb();
}

/*
 * handlers will be called from notification thread context,
 * index in table equals to 'opcode - HAL_MINIMUM_EVENT'
 */
static const struct hal_ipc_handler ev_handlers[] = {
	/* HAL_EV_HF_CLIENT_CONN_STATE */
	{ handle_conn_state, false,
				sizeof(struct hal_ev_hf_client_conn_state) },
	/* HAL_EV_HF_CLIENT_AUDIO_STATE */
	{ handle_audio_state, false,
				sizeof(struct hal_ev_hf_client_audio_state) },
	/* HAL_EV_HF_CLIENT_VR_STATE */
	{ handle_vr_state, false, sizeof(struct hal_ev_hf_client_vr_state) },
	/*HAL_EV_HF_CLIENT_NET_STATE */
	{ handle_network_state, false,
				sizeof(struct hal_ev_hf_client_net_state)},
	/*HAL_EV_HF_CLIENT_NET_ROAMING_TYPE */
	{ handle_network_roaming, false,
			sizeof(struct hal_ev_hf_client_net_roaming_type) },
	/* HAL_EV_HF_CLIENT_NET_SIGNAL_STRENGTH */
	{ handle_network_signal, false,
			sizeof(struct hal_ev_hf_client_net_signal_strength) },
	/* HAL_EV_HF_CLIENT_BATTERY_LEVEL */
	{ handle_battery_level, false,
			sizeof(struct hal_ev_hf_client_battery_level) },
	/* HAL_EV_HF_CLIENT_OPERATOR_NAME */
	{ handle_operator_name, true,
			sizeof(struct hal_ev_hf_client_operator_name) },
	/* HAL_EV_HF_CLIENT_CALL_INDICATOR */
	{ handle_call, false,
			sizeof(struct hal_ev_hf_client_call_indicator) },
	/* HAL_EV_HF_CLIENT_CALL_SETUP_INDICATOR */
	{ handle_call_setup, false,
		sizeof(struct hal_ev_hf_client_call_setup_indicator) },
	/* HAL_EV_HF_CLIENT_CALL_HELD_INDICATOR */
	{ handle_call_held, false,
			sizeof(struct hal_ev_hf_client_call_held_indicator) },
	/* HAL_EV_HF_CLIENT_RESPONSE_AND_HOLD_STATUS */
	{ handle_response_and_hold, false,
		sizeof(struct hal_ev_hf_client_response_and_hold_status) },
	/* HAL_EV_HF_CLIENT_CALLING_LINE_IDENT */
	{ handle_clip, true,
			sizeof(struct hal_ev_hf_client_calling_line_ident) },
	/* HAL_EV_HF_CLIENT_CALL_WAITING */
	{ handle_call_waiting, true,
			sizeof(struct hal_ev_hf_client_call_waiting) },
	/* HAL_EV_HF_CLIENT_CURRENT_CALL */
	{ handle_current_calls, true,
			sizeof(struct hal_ev_hf_client_current_call) },
	/* HAL_EV_CLIENT_VOLUME_CHANGED */
	{ handle_volume_change, false,
			sizeof(struct hal_ev_hf_client_volume_changed) },
	/* HAL_EV_CLIENT_COMMAND_COMPLETE */
	{ handle_command_cmp, false,
			sizeof(struct hal_ev_hf_client_command_complete) },
	/* HAL_EV_CLIENT_SUBSCRIBER_SERVICE_INFO */
	{ handle_subscriber_info, true,
		sizeof(struct hal_ev_hf_client_subscriber_service_info) },
	/* HAL_EV_CLIENT_INBAND_SETTINGS */
	{ handle_in_band_ringtone, false,
		sizeof(struct hal_ev_hf_client_inband_settings) },
	/* HAL_EV_CLIENT_LAST_VOICE_CALL_TAG_NUM */
	{ handle_last_voice_tag_number, true,
		sizeof(struct hal_ev_hf_client_last_void_call_tag_num) },
	/* HAL_EV_CLIENT_RING_INDICATION */
	{ handle_ring_indication, false, 0 },
};

static bt_status_t init(bthf_client_callbacks_t *callbacks)
{
	struct hal_cmd_register_module cmd;
	int ret;

	DBG("");

	if (interface_ready())
		return BT_STATUS_DONE;

	cbs = callbacks;

	hal_ipc_register(HAL_SERVICE_ID_HANDSFREE_CLIENT, ev_handlers,
				sizeof(ev_handlers)/sizeof(ev_handlers[0]));

	cmd.service_id = HAL_SERVICE_ID_HANDSFREE_CLIENT;
	cmd.mode = HAL_MODE_DEFAULT;
	cmd.max_clients = 1;

	ret = hal_ipc_cmd(HAL_SERVICE_ID_CORE, HAL_OP_REGISTER_MODULE,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
	if (ret != BT_STATUS_SUCCESS) {
		cbs = NULL;
		hal_ipc_unregister(HAL_SERVICE_ID_HANDSFREE_CLIENT);
	}

	return ret;
}

static bt_status_t hf_client_connect(bt_bdaddr_t *bd_addr)
{
	struct hal_cmd_hf_client_connect cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!bd_addr)
		return BT_STATUS_PARM_INVALID;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE_CLIENT,
				HAL_OP_HF_CLIENT_CONNECT, sizeof(cmd), &cmd,
				NULL, NULL, NULL);
}

static bt_status_t disconnect(bt_bdaddr_t *bd_addr)
{
	struct hal_cmd_hf_client_disconnect cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!bd_addr)
		return BT_STATUS_PARM_INVALID;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE_CLIENT,
				HAL_OP_HF_CLIENT_DISCONNECT, sizeof(cmd), &cmd,
				NULL, NULL, NULL);
}

static bt_status_t connect_audio(bt_bdaddr_t *bd_addr)
{
	struct hal_cmd_hf_client_connect_audio cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!bd_addr)
		return BT_STATUS_PARM_INVALID;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE_CLIENT,
				HAL_OP_HF_CLIENT_CONNECT_AUDIO, sizeof(cmd),
				&cmd, NULL, NULL, NULL);
}

static bt_status_t disconnect_audio(bt_bdaddr_t *bd_addr)
{
	struct hal_cmd_hf_client_disconnect_audio cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!bd_addr)
		return BT_STATUS_PARM_INVALID;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE_CLIENT,
				HAL_OP_HF_CLIENT_DISCONNECT_AUDIO, sizeof(cmd),
				&cmd, NULL, NULL, NULL);
}

static bt_status_t start_voice_recognition(void)
{
	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE_CLIENT,
				HAL_OP_HF_CLIENT_START_VR, 0, NULL, NULL, NULL,
				NULL);
}

static bt_status_t stop_voice_recognition(void)
{
	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE_CLIENT,
				HAL_OP_HF_CLIENT_STOP_VR, 0, NULL, NULL, NULL,
				NULL);
}

static bt_status_t volume_control(bthf_client_volume_type_t type,
								int volume)
{
	struct hal_cmd_hf_client_volume_control cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	cmd.type = type;
	cmd.volume = volume;

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE_CLIENT,
				HAL_OP_HF_CLIENT_VOLUME_CONTROL, sizeof(cmd),
				&cmd, NULL, NULL, NULL);
}

static bt_status_t dial(const char *number)
{
	char buf[IPC_MTU];
	struct hal_cmd_hf_client_dial *cmd = (void *) buf;
	size_t len;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (number) {
		cmd->number_len = strlen(number) + 1;
		memcpy(cmd->number, number, cmd->number_len);
	} else {
		cmd->number_len = 0;
	}

	len = sizeof(*cmd) + cmd->number_len;

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE_CLIENT,
				HAL_OP_HF_CLIENT_DIAL, len, cmd, NULL, NULL,
				NULL);
}

static bt_status_t dial_memory(int location)
{
	struct hal_cmd_hf_client_dial_memory cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	cmd.location = location;

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE_CLIENT,
					HAL_OP_HF_CLIENT_DIAL_MEMORY,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static bt_status_t call_action(bthf_client_call_action_t action, int index)
{
	struct hal_cmd_hf_client_call_action cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	cmd.action = action;
	cmd.index = index;

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE_CLIENT,
				HAL_OP_HF_CLIENT_CALL_ACTION, sizeof(cmd), &cmd,
				NULL, NULL, NULL);
}

static bt_status_t query_current_calls(void)
{
	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE_CLIENT,
				HAL_OP_HF_CLIENT_QUERY_CURRENT_CALLS, 0, NULL,
				NULL, NULL, NULL);
}

static bt_status_t query_operator_name(void)
{
	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE_CLIENT,
				HAL_OP_HF_CLIENT_QUERY_OPERATOR_NAME, 0, NULL,
				NULL, NULL, NULL);
}

static bt_status_t retrieve_subsr_info(void)
{
	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE_CLIENT,
				HAL_OP_HF_CLIENT_RETRIEVE_SUBSCR_INFO, 0, NULL,
				NULL, NULL, NULL);
}

static bt_status_t send_dtmf(char tone)
{
	struct hal_cmd_hf_client_send_dtmf cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	cmd.tone = tone;

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE_CLIENT,
				HAL_OP_HF_CLIENT_SEND_DTMF, sizeof(cmd), &cmd,
				NULL, NULL, NULL);
}

static bt_status_t request_last_voice_tag_number(void)
{
	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE_CLIENT,
					HAL_OP_HF_CLIENT_GET_LAST_VOICE_TAG_NUM,
					0, NULL, NULL, NULL, NULL);
}

static void cleanup(void)
{
	struct hal_cmd_unregister_module cmd;

	DBG("");

	if (!interface_ready())
		return;

	cmd.service_id = HAL_SERVICE_ID_HANDSFREE_CLIENT;

	hal_ipc_cmd(HAL_SERVICE_ID_CORE, HAL_OP_UNREGISTER_MODULE,
					sizeof(cmd), &cmd, NULL, NULL, NULL);

	hal_ipc_unregister(HAL_SERVICE_ID_HANDSFREE_CLIENT);

	cbs = NULL;
}

static bthf_client_interface_t iface = {
	.size = sizeof(iface),
	.init = init,
	.connect = hf_client_connect,
	.disconnect = disconnect,
	.connect_audio = connect_audio,
	.disconnect_audio = disconnect_audio,
	.start_voice_recognition = start_voice_recognition,
	.stop_voice_recognition = stop_voice_recognition,
	.volume_control = volume_control,
	.dial = dial,
	.dial_memory = dial_memory,
	.handle_call_action = call_action,
	.query_current_calls = query_current_calls,
	.query_current_operator_name = query_operator_name,
	.retrieve_subscriber_info = retrieve_subsr_info,
	.send_dtmf = send_dtmf,
	.request_last_voice_tag_number = request_last_voice_tag_number,
	.cleanup = cleanup
};

bthf_client_interface_t *bt_get_hf_client_interface(void)
{
	return &iface;
}

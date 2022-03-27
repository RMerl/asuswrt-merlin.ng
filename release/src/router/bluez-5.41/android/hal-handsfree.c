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
#include "hal-utils.h"

static const bthf_callbacks_t *cbs = NULL;

static bool interface_ready(void)
{
	return cbs != NULL;
}

static void handle_conn_state(void *buf, uint16_t len, int fd)
{
	struct hal_ev_handsfree_conn_state *ev = buf;

	if (cbs->connection_state_cb)
		cbs->connection_state_cb(ev->state,
						(bt_bdaddr_t *) (ev->bdaddr));
}

static void handle_audio_state(void *buf, uint16_t len, int fd)
{
	struct hal_ev_handsfree_audio_state *ev = buf;

	if (cbs->audio_state_cb)
		cbs->audio_state_cb(ev->state, (bt_bdaddr_t *) (ev->bdaddr));
}

static void handle_vr_state(void *buf, uint16_t len, int fd)
{
	struct hal_ev_handsfree_vr_state *ev = buf;

	if (cbs->vr_cmd_cb)
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
		cbs->vr_cmd_cb(ev->state, (bt_bdaddr_t *) (ev->bdaddr));
#else
		cbs->vr_cmd_cb(ev->state);
#endif
}

static void handle_answer(void *buf, uint16_t len, int fd)
{
	if (cbs->answer_call_cmd_cb) {
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
		struct hal_ev_handsfree_answer *ev = buf;

		cbs->answer_call_cmd_cb((bt_bdaddr_t *) (ev->bdaddr));
#else
		cbs->answer_call_cmd_cb();
#endif
	}
}

static void handle_hangup(void *buf, uint16_t len, int fd)
{
	if (cbs->hangup_call_cmd_cb) {
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
		struct hal_ev_handsfree_hangup *ev = buf;

		cbs->hangup_call_cmd_cb((bt_bdaddr_t *) (ev->bdaddr));
#else
		cbs->hangup_call_cmd_cb();
#endif
	}
}

static void handle_volume(void *buf, uint16_t len, int fd)
{
	struct hal_ev_handsfree_volume *ev = buf;

	if (cbs->volume_cmd_cb)
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
		cbs->volume_cmd_cb(ev->type, ev->volume,
						(bt_bdaddr_t *) (ev->bdaddr));
#else
		cbs->volume_cmd_cb(ev->type, ev->volume);
#endif
}

static void handle_dial(void *buf, uint16_t len, int fd)
{
	struct hal_ev_handsfree_dial *ev = buf;
	uint16_t num_len = ev->number_len;
	char *number = NULL;

	if (len != sizeof(*ev) + num_len ||
			(num_len != 0 && ev->number[num_len - 1] != '\0')) {
		error("invalid dial event, aborting");
		exit(EXIT_FAILURE);
	}

	if (!cbs->dial_call_cmd_cb)
		return;

	if (ev->number_len)
		number = (char *) ev->number;

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	cbs->dial_call_cmd_cb(number, (bt_bdaddr_t *) (ev->bdaddr));
#else
	cbs->dial_call_cmd_cb(number);
#endif
}

static void handle_dtmf(void *buf, uint16_t len, int fd)
{
	struct hal_ev_handsfree_dtmf *ev = buf;

	if (cbs->dtmf_cmd_cb)
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
		cbs->dtmf_cmd_cb(ev->tone, (bt_bdaddr_t *) (ev->bdaddr));
#else
		cbs->dtmf_cmd_cb(ev->tone);
#endif
}

static void handle_nrec(void *buf, uint16_t len, int fd)
{
	struct hal_ev_handsfree_nrec *ev = buf;

	if (cbs->nrec_cmd_cb)
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
		cbs->nrec_cmd_cb(ev->nrec, (bt_bdaddr_t *) (ev->bdaddr));
#else
		cbs->nrec_cmd_cb(ev->nrec);
#endif
}

static void handle_wbs(void *buf, uint16_t len, int fd)
{
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	struct hal_ev_handsfree_wbs *ev = buf;

	if (cbs->wbs_cb)
		cbs->wbs_cb(ev->wbs, (bt_bdaddr_t *) (ev->bdaddr));
#endif
}

static void handle_chld(void *buf, uint16_t len, int fd)
{
	struct hal_ev_handsfree_chld *ev = buf;

	if (cbs->chld_cmd_cb)
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
		cbs->chld_cmd_cb(ev->chld, (bt_bdaddr_t *) (ev->bdaddr));
#else
		cbs->chld_cmd_cb(ev->chld);
#endif
}

static void handle_cnum(void *buf, uint16_t len, int fd)
{
	if (cbs->cnum_cmd_cb) {
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
		struct hal_ev_handsfree_cnum *ev = buf;

		cbs->cnum_cmd_cb((bt_bdaddr_t *) (ev->bdaddr));
#else
		cbs->cnum_cmd_cb(NULL);
#endif
	}
}

static void handle_cind(void *buf, uint16_t len, int fd)
{
	if (cbs->cind_cmd_cb) {
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
		struct hal_ev_handsfree_cind *ev = buf;

		cbs->cind_cmd_cb((bt_bdaddr_t *) (ev->bdaddr));
#else
		cbs->cind_cmd_cb();
#endif
	}
}

static void handle_cops(void *buf, uint16_t len, int fd)
{
	if (cbs->cops_cmd_cb) {
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
		struct hal_ev_handsfree_cops *ev = buf;

		cbs->cops_cmd_cb((bt_bdaddr_t *) (ev->bdaddr));
#else
		cbs->cops_cmd_cb();
#endif
	}
}

static void handle_clcc(void *buf, uint16_t len, int fd)
{
	if (cbs->clcc_cmd_cb) {
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
		struct hal_ev_handsfree_clcc *ev = buf;

		cbs->clcc_cmd_cb((bt_bdaddr_t *) (ev->bdaddr));
#else
		cbs->clcc_cmd_cb();
#endif
	}
}

static void handle_unknown_at(void *buf, uint16_t len, int fd)
{
	struct hal_ev_handsfree_unknown_at *ev = buf;

	if (len != sizeof(*ev) + ev->len ||
			(ev->len != 0 && ev->buf[ev->len - 1] != '\0')) {
		error("invalid unknown command event, aborting");
		exit(EXIT_FAILURE);
	}

	if (cbs->unknown_at_cmd_cb)
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
		cbs->unknown_at_cmd_cb((char *) ev->buf,
						(bt_bdaddr_t *) (ev->bdaddr));
#else
		cbs->unknown_at_cmd_cb((char *) ev->buf);
#endif
}

static void handle_hsp_key_press(void *buf, uint16_t len, int fd)
{
	if (cbs->key_pressed_cmd_cb) {
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
		struct hal_ev_handsfree_hsp_key_press *ev = buf;

		cbs->key_pressed_cmd_cb((bt_bdaddr_t *) (ev->bdaddr));
#else
		cbs->key_pressed_cmd_cb();
#endif
	}
}

/*
 * handlers will be called from notification thread context,
 * index in table equals to 'opcode - HAL_MINIMUM_EVENT'
 */
static const struct hal_ipc_handler ev_handlers[] = {
	/* HAL_EV_HANDSFREE_CONN_STATE */
	{ handle_conn_state, false,
				sizeof(struct hal_ev_handsfree_conn_state) },
	/* HAL_EV_HANDSFREE_AUDIO_STATE */
	{ handle_audio_state, false,
				sizeof(struct hal_ev_handsfree_audio_state) },
	/* HAL_EV_HANDSFREE_VR */
	{ handle_vr_state, false, sizeof(struct hal_ev_handsfree_vr_state) },
	/* HAL_EV_HANDSFREE_ANSWER */
	{ handle_answer, false, sizeof(struct hal_ev_handsfree_answer) },
	/* HAL_EV_HANDSFREE_HANGUP */
	{ handle_hangup, false, sizeof(struct hal_ev_handsfree_hangup) },
	/* HAL_EV_HANDSFREE_VOLUME */
	{ handle_volume, false, sizeof(struct hal_ev_handsfree_volume) },
	/* HAL_EV_HANDSFREE_DIAL */
	{ handle_dial, true, sizeof(struct hal_ev_handsfree_dial) },
	/* HAL_EV_HANDSFREE_DTMF */
	{ handle_dtmf, false, sizeof(struct hal_ev_handsfree_dtmf) },
	/* HAL_EV_HANDSFREE_NREC */
	{ handle_nrec, false, sizeof(struct hal_ev_handsfree_nrec) },
	/* HAL_EV_HANDSFREE_CHLD */
	{ handle_chld, false, sizeof(struct hal_ev_handsfree_chld) },
	/* HAL_EV_HANDSFREE_CNUM */
	{ handle_cnum, false, sizeof(struct hal_ev_handsfree_cnum) },
	/* HAL_EV_HANDSFREE_CIND */
	{ handle_cind, false, sizeof(struct hal_ev_handsfree_cind) },
	/* HAL_EV_HANDSFREE_COPS */
	{ handle_cops, false, sizeof(struct hal_ev_handsfree_cops) },
	/* HAL_EV_HANDSFREE_CLCC */
	{ handle_clcc, false, sizeof(struct hal_ev_handsfree_clcc) },
	/* HAL_EV_HANDSFREE_UNKNOWN_AT */
	{ handle_unknown_at, true, sizeof(struct hal_ev_handsfree_unknown_at) },
	/* HAL_EV_HANDSFREE_HSP_KEY_PRESS */
	{ handle_hsp_key_press, false,
				sizeof(struct hal_ev_handsfree_hsp_key_press) },
	/* HAL_EV_HANDSFREE_WBS */
	{ handle_wbs, false, sizeof(struct hal_ev_handsfree_wbs) },
};

static uint8_t get_mode(void)
{
	char value[PROPERTY_VALUE_MAX];

	if (get_config("handsfree", value, NULL) > 0) {
		if (!strcasecmp(value, "hfp"))
			return HAL_MODE_HANDSFREE_HFP;

		if (!strcasecmp(value, "hfp_wbs"))
			return HAL_MODE_HANDSFREE_HFP_WBS;
	}

	return HAL_MODE_HANDSFREE_HSP_ONLY;
}

static bt_status_t init_real(bthf_callbacks_t *callbacks, int max_hf_clients)
{
	struct hal_cmd_register_module cmd;
	int ret;

	DBG("");

	if (interface_ready())
		return BT_STATUS_DONE;

	cbs = callbacks;

	hal_ipc_register(HAL_SERVICE_ID_HANDSFREE, ev_handlers,
				sizeof(ev_handlers)/sizeof(ev_handlers[0]));

	cmd.service_id = HAL_SERVICE_ID_HANDSFREE;
	cmd.mode = get_mode();
	cmd.max_clients = max_hf_clients;

	ret = hal_ipc_cmd(HAL_SERVICE_ID_CORE, HAL_OP_REGISTER_MODULE,
					sizeof(cmd), &cmd, NULL, NULL, NULL);

	if (ret != BT_STATUS_SUCCESS) {
		cbs = NULL;
		hal_ipc_unregister(HAL_SERVICE_ID_HANDSFREE);
	}

	return ret;
}

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static bt_status_t init(bthf_callbacks_t *callbacks, int max_hf_clients)
{
	return init_real(callbacks, max_hf_clients);
}
#else
static bt_status_t init(bthf_callbacks_t *callbacks)
{
	return init_real(callbacks, 1);
}
#endif

static bt_status_t handsfree_connect(bt_bdaddr_t *bd_addr)
{
	struct hal_cmd_handsfree_connect cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!bd_addr)
		return BT_STATUS_PARM_INVALID;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE, HAL_OP_HANDSFREE_CONNECT,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static bt_status_t disconnect(bt_bdaddr_t *bd_addr)
{
	struct hal_cmd_handsfree_disconnect cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!bd_addr)
		return BT_STATUS_PARM_INVALID;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE,
				HAL_OP_HANDSFREE_DISCONNECT, sizeof(cmd), &cmd,
				NULL, NULL, NULL);
}

static bt_status_t connect_audio(bt_bdaddr_t *bd_addr)
{
	struct hal_cmd_handsfree_connect_audio cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!bd_addr)
		return BT_STATUS_PARM_INVALID;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE,
				HAL_OP_HANDSFREE_CONNECT_AUDIO, sizeof(cmd),
				&cmd, NULL, NULL, NULL);
}

static bt_status_t disconnect_audio(bt_bdaddr_t *bd_addr)
{
	struct hal_cmd_handsfree_disconnect_audio cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!bd_addr)
		return BT_STATUS_PARM_INVALID;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE,
				HAL_OP_HANDSFREE_DISCONNECT_AUDIO, sizeof(cmd),
				&cmd, NULL, NULL, NULL);
}

static bt_status_t start_voice_recognition_real(bt_bdaddr_t *bd_addr)
{
	struct hal_cmd_handsfree_start_vr cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	memset(&cmd, 0, sizeof(cmd));

	if (bd_addr)
		memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE, HAL_OP_HANDSFREE_START_VR,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static bt_status_t start_voice_recognition(bt_bdaddr_t *bd_addr)
{
	return start_voice_recognition_real(bd_addr);
}
#else
static bt_status_t start_voice_recognition(void)
{
	return start_voice_recognition_real(NULL);
}
#endif

static bt_status_t stop_voice_recognition_real(bt_bdaddr_t *bd_addr)
{
	struct hal_cmd_handsfree_stop_vr cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	memset(&cmd, 0, sizeof(cmd));

	if (bd_addr)
		memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE, HAL_OP_HANDSFREE_STOP_VR,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static bt_status_t stop_voice_recognition(bt_bdaddr_t *bd_addr)
{
	return stop_voice_recognition_real(bd_addr);
}
#else
static bt_status_t stop_voice_recognition(void)
{
	return stop_voice_recognition_real(NULL);
}
#endif

static bt_status_t volume_control_real(bthf_volume_type_t type, int volume,
							bt_bdaddr_t *bd_addr)
{
	struct hal_cmd_handsfree_volume_control cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	memset(&cmd, 0, sizeof(cmd));

	cmd.type = type;
	cmd.volume = volume;

	if (bd_addr)
		memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE,
				HAL_OP_HANDSFREE_VOLUME_CONTROL, sizeof(cmd),
				&cmd, NULL, NULL, NULL);
}

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static bt_status_t volume_control(bthf_volume_type_t type, int volume,
							bt_bdaddr_t *bd_addr)
{
	return volume_control_real(type, volume, bd_addr);
}
#else
static bt_status_t volume_control(bthf_volume_type_t type, int volume)
{
	return volume_control_real(type, volume, NULL);
}
#endif

static bt_status_t device_status_notification(bthf_network_state_t state,
						bthf_service_type_t type,
						int signal, int battery)
{
	struct hal_cmd_handsfree_device_status_notif cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	cmd.state = state;
	cmd.type = type;
	cmd.signal = signal;
	cmd.battery = battery;

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE,
					HAL_OP_HANDSFREE_DEVICE_STATUS_NOTIF,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static bt_status_t cops_response_real(const char *cops, bt_bdaddr_t *bd_addr)
{
	char buf[IPC_MTU];
	struct hal_cmd_handsfree_cops_response *cmd = (void *) buf;
	size_t len;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!cops)
		return BT_STATUS_PARM_INVALID;

	memset(cmd, 0, sizeof(*cmd));

	if (bd_addr)
		memcpy(cmd->bdaddr, bd_addr, sizeof(cmd->bdaddr));

	/* Size of cmd.buf */
	cmd->len = strlen(cops) + 1;
	memcpy(cmd->buf, cops, cmd->len);

	len = sizeof(*cmd) + cmd->len;

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE,
						HAL_OP_HANDSFREE_COPS_RESPONSE,
						len, cmd, NULL, NULL, NULL);
}

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static bt_status_t cops_response(const char *cops, bt_bdaddr_t *bd_addr)
{
	return cops_response_real(cops, bd_addr);
}
#else
static bt_status_t cops_response(const char *cops)
{
	return cops_response_real(cops, NULL);
}
#endif

static bt_status_t cind_response_real(int svc, int num_active, int num_held,
					bthf_call_state_t state, int signal,
					int roam, int batt_chg,
					bt_bdaddr_t *bd_addr)
{
	struct hal_cmd_handsfree_cind_response cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	memset(&cmd, 0, sizeof(cmd));

	if (bd_addr)
		memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	cmd.svc = svc;
	cmd.num_active = num_active;
	cmd.num_held = num_held;
	cmd.state = state;
	cmd.signal = signal;
	cmd.roam = roam;
	cmd.batt_chg = batt_chg;

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE,
					HAL_OP_HANDSFREE_CIND_RESPONSE,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static bt_status_t cind_response(int svc, int num_active, int num_held,
					bthf_call_state_t state, int signal,
					int roam, int batt_chg,
					bt_bdaddr_t *bd_addr)
{
	return cind_response_real(svc, num_active, num_held, state, signal,
						roam, batt_chg, bd_addr);
}
#else
static bt_status_t cind_response(int svc, int num_active, int num_held,
					bthf_call_state_t state, int signal,
					int roam, int batt_chg)
{
	return cind_response_real(svc, num_active, num_held, state, signal,
						roam, batt_chg, NULL);
}
#endif

static bt_status_t formatted_at_response_real(const char *rsp,
							bt_bdaddr_t *bd_addr)
{
	char buf[IPC_MTU];
	struct hal_cmd_handsfree_formatted_at_response *cmd = (void *) buf;
	size_t len;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!rsp)
		return BT_STATUS_PARM_INVALID;

	memset(cmd, 0, sizeof(*cmd));

	if (bd_addr)
		memcpy(cmd->bdaddr, bd_addr, sizeof(cmd->bdaddr));

	cmd->len = strlen(rsp) + 1;
	memcpy(cmd->buf, rsp, cmd->len);

	len = sizeof(*cmd) + cmd->len;

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE,
					HAL_OP_HANDSFREE_FORMATTED_AT_RESPONSE,
					len, cmd, NULL, NULL, NULL);
}

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static bt_status_t formatted_at_response(const char *rsp, bt_bdaddr_t *bd_addr)
{
	return formatted_at_response_real(rsp, bd_addr);
}
#else
static bt_status_t formatted_at_response(const char *rsp)
{
	return formatted_at_response_real(rsp, NULL);
}
#endif

static bt_status_t at_response_real(bthf_at_response_t response, int error,
							bt_bdaddr_t *bd_addr)
{
	struct hal_cmd_handsfree_at_response cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (bd_addr)
		memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));

	memset(&cmd, 0, sizeof(cmd));

	cmd.response = response;
	cmd.error = error;

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE,
					HAL_OP_HANDSFREE_AT_RESPONSE,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static bt_status_t at_response(bthf_at_response_t response, int error,
							bt_bdaddr_t *bd_addr)
{
	return at_response_real(response, error, bd_addr);
}
#else
static bt_status_t at_response(bthf_at_response_t response, int error)
{
	return at_response_real(response, error, NULL);
}
#endif

static bt_status_t clcc_response_real(int index, bthf_call_direction_t dir,
					bthf_call_state_t state,
					bthf_call_mode_t mode,
					bthf_call_mpty_type_t mpty,
					const char *number,
					bthf_call_addrtype_t type,
					bt_bdaddr_t *bd_addr)
{
	char buf[IPC_MTU];
	struct hal_cmd_handsfree_clcc_response *cmd = (void *) buf;
	size_t len;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	memset(cmd, 0, sizeof(*cmd));

	if (bd_addr)
		memcpy(cmd->bdaddr, bd_addr, sizeof(cmd->bdaddr));

	cmd->index = index;
	cmd->dir = dir;
	cmd->state = state;
	cmd->mode = mode;
	cmd->mpty = mpty;
	cmd->type = type;

	if (number) {
		cmd->number_len = strlen(number) + 1;
		memcpy(cmd->number, number, cmd->number_len);
	} else {
		cmd->number_len = 0;
	}

	len = sizeof(*cmd) + cmd->number_len;

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE,
						HAL_OP_HANDSFREE_CLCC_RESPONSE,
						len, cmd, NULL, NULL, NULL);
}

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static bt_status_t clcc_response(int index, bthf_call_direction_t dir,
					bthf_call_state_t state,
					bthf_call_mode_t mode,
					bthf_call_mpty_type_t mpty,
					const char *number,
					bthf_call_addrtype_t type,
					bt_bdaddr_t *bd_addr)
{
	return clcc_response_real(index, dir, state, mode, mpty, number, type,
								bd_addr);
}
#else
static bt_status_t clcc_response(int index, bthf_call_direction_t dir,
					bthf_call_state_t state,
					bthf_call_mode_t mode,
					bthf_call_mpty_type_t mpty,
					const char *number,
					bthf_call_addrtype_t type)
{
	return clcc_response_real(index, dir, state, mode, mpty, number, type,
									NULL);
}
#endif

static bt_status_t phone_state_change(int num_active, int num_held,
					bthf_call_state_t state,
					const char *number,
					bthf_call_addrtype_t type)
{
	char buf[IPC_MTU];
	struct hal_cmd_handsfree_phone_state_change *cmd = (void *) buf;
	size_t len;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	cmd->num_active = num_active;
	cmd->num_held = num_held;
	cmd->state = state;
	cmd->type = type;

	if (number) {
		cmd->number_len = strlen(number) + 1;
		memcpy(cmd->number, number, cmd->number_len);
	} else {
		cmd->number_len = 0;
	}

	len = sizeof(*cmd) + cmd->number_len;

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE,
					HAL_OP_HANDSFREE_PHONE_STATE_CHANGE,
					len, cmd, NULL, NULL, NULL);
}

static void cleanup(void)
{
	struct hal_cmd_unregister_module cmd;

	DBG("");

	if (!interface_ready())
		return;

	cmd.service_id = HAL_SERVICE_ID_HANDSFREE;

	hal_ipc_cmd(HAL_SERVICE_ID_CORE, HAL_OP_UNREGISTER_MODULE,
					sizeof(cmd), &cmd, NULL, NULL, NULL);

	hal_ipc_unregister(HAL_SERVICE_ID_HANDSFREE);

	cbs = NULL;
}

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static bt_status_t configure_wbs(bt_bdaddr_t *bd_addr, bthf_wbs_config_t config)
{
	struct hal_cmd_handsfree_configure_wbs cmd;

	DBG("%u", config);

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!bd_addr)
		return BT_STATUS_PARM_INVALID;

	memcpy(cmd.bdaddr, bd_addr, sizeof(cmd.bdaddr));
	cmd.config = config;

	return hal_ipc_cmd(HAL_SERVICE_ID_HANDSFREE,
					HAL_OP_HANDSFREE_CONFIGURE_WBS,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}
#endif

static bthf_interface_t iface = {
	.size = sizeof(iface),
	.init = init,
	.connect = handsfree_connect,
	.disconnect = disconnect,
	.connect_audio = connect_audio,
	.disconnect_audio = disconnect_audio,
	.start_voice_recognition = start_voice_recognition,
	.stop_voice_recognition = stop_voice_recognition,
	.volume_control = volume_control,
	.device_status_notification = device_status_notification,
	.cops_response = cops_response,
	.cind_response = cind_response,
	.formatted_at_response = formatted_at_response,
	.at_response = at_response,
	.clcc_response = clcc_response,
	.phone_state_change = phone_state_change,
	.cleanup = cleanup,
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	.configure_wbs = configure_wbs,
#endif
};

bthf_interface_t *bt_get_handsfree_interface(void)
{
	return &iface;
}

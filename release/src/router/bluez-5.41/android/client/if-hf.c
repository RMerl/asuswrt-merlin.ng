/*
 * Copyright (C) 2013 Intel Corporation
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

#include "if-main.h"
#include "../hal-utils.h"

const bthf_interface_t *if_hf = NULL;

SINTMAP(bthf_at_response_t, -1, "(unknown)")
	DELEMENT(BTHF_AT_RESPONSE_ERROR),
	DELEMENT(BTHF_AT_RESPONSE_OK),
ENDMAP

SINTMAP(bthf_connection_state_t, -1, "(unknown)")
	DELEMENT(BTHF_CONNECTION_STATE_DISCONNECTED),
	DELEMENT(BTHF_CONNECTION_STATE_CONNECTING),
	DELEMENT(BTHF_CONNECTION_STATE_CONNECTED),
	DELEMENT(BTHF_CONNECTION_STATE_SLC_CONNECTED),
	DELEMENT(BTHF_CONNECTION_STATE_DISCONNECTING),
ENDMAP

SINTMAP(bthf_audio_state_t, -1, "(unknown)")
	DELEMENT(BTHF_AUDIO_STATE_DISCONNECTED),
	DELEMENT(BTHF_AUDIO_STATE_CONNECTING),
	DELEMENT(BTHF_AUDIO_STATE_CONNECTED),
	DELEMENT(BTHF_AUDIO_STATE_DISCONNECTING),
ENDMAP

SINTMAP(bthf_vr_state_t, -1, "(unknown)")
	DELEMENT(BTHF_VR_STATE_STOPPED),
	DELEMENT(BTHF_VR_STATE_STARTED),
ENDMAP

SINTMAP(bthf_volume_type_t, -1, "(unknown)")
	DELEMENT(BTHF_VOLUME_TYPE_SPK),
	DELEMENT(BTHF_VOLUME_TYPE_MIC),
ENDMAP

SINTMAP(bthf_nrec_t, -1, "(unknown)")
	DELEMENT(BTHF_NREC_STOP),
	DELEMENT(BTHF_NREC_START),
ENDMAP

SINTMAP(bthf_chld_type_t, -1, "(unknown)")
	DELEMENT(BTHF_CHLD_TYPE_RELEASEHELD),
	DELEMENT(BTHF_CHLD_TYPE_RELEASEACTIVE_ACCEPTHELD),
	DELEMENT(BTHF_CHLD_TYPE_HOLDACTIVE_ACCEPTHELD),
	DELEMENT(BTHF_CHLD_TYPE_ADDHELDTOCONF),
ENDMAP

/* Network Status */
SINTMAP(bthf_network_state_t, -1, "(unknown)")
	DELEMENT(BTHF_NETWORK_STATE_NOT_AVAILABLE),
	DELEMENT(BTHF_NETWORK_STATE_AVAILABLE),
ENDMAP

/* Service type */
SINTMAP(bthf_service_type_t, -1, "(unknown)")
	DELEMENT(BTHF_SERVICE_TYPE_HOME),
	DELEMENT(BTHF_SERVICE_TYPE_ROAMING),
ENDMAP

SINTMAP(bthf_call_state_t, -1, "(unknown)")
	DELEMENT(BTHF_CALL_STATE_ACTIVE),
	DELEMENT(BTHF_CALL_STATE_HELD),
	DELEMENT(BTHF_CALL_STATE_DIALING),
	DELEMENT(BTHF_CALL_STATE_ALERTING),
	DELEMENT(BTHF_CALL_STATE_INCOMING),
	DELEMENT(BTHF_CALL_STATE_WAITING),
	DELEMENT(BTHF_CALL_STATE_IDLE),
ENDMAP

SINTMAP(bthf_call_direction_t, -1, "(unknown)")
	DELEMENT(BTHF_CALL_DIRECTION_OUTGOING),
	DELEMENT(BTHF_CALL_DIRECTION_INCOMING),
ENDMAP

SINTMAP(bthf_call_mode_t, -1, "(unknown)")
	DELEMENT(BTHF_CALL_TYPE_VOICE),
	DELEMENT(BTHF_CALL_TYPE_DATA),
	DELEMENT(BTHF_CALL_TYPE_FAX),
ENDMAP

SINTMAP(bthf_call_mpty_type_t, -1, "(unknown)")
	DELEMENT(BTHF_CALL_MPTY_TYPE_SINGLE),
	DELEMENT(BTHF_CALL_MPTY_TYPE_MULTI),
ENDMAP

SINTMAP(bthf_call_addrtype_t, -1, "(unknown)")
	DELEMENT(BTHF_CALL_ADDRTYPE_UNKNOWN),
	DELEMENT(BTHF_CALL_ADDRTYPE_INTERNATIONAL),
ENDMAP

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
SINTMAP(bthf_wbs_config_t, -1, "(unknown)")
	DELEMENT(BTHF_WBS_NONE),
	DELEMENT(BTHF_WBS_NO),
	DELEMENT(BTHF_WBS_YES),
ENDMAP
#endif

/* Callbacks */

static char last_addr[MAX_ADDR_STR_LEN];

/*
 * Callback for connection state change.
 * state will have one of the values from BtHfConnectionState
 */
static void connection_state_cb(bthf_connection_state_t state,
							bt_bdaddr_t *bd_addr)
{
	haltest_info("%s: state=%s bd_addr=%s\n", __func__,
					bthf_connection_state_t2str(state),
					bt_bdaddr_t2str(bd_addr, last_addr));
}

/*
 * Callback for audio connection state change.
 * state will have one of the values from BtHfAudioState
 */
static void audio_state_cb(bthf_audio_state_t state, bt_bdaddr_t *bd_addr)
{
	haltest_info("%s: state=%s bd_addr=%s\n", __func__,
					bthf_audio_state_t2str(state),
					bt_bdaddr_t2str(bd_addr, last_addr));
}

/*
 * Callback for VR connection state change.
 * state will have one of the values from BtHfVRState
 */
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static void vr_cmd_cb(bthf_vr_state_t state, bt_bdaddr_t *bd_addr)
{
	haltest_info("%s: state=%s bd_addr=%s\n", __func__,
					bthf_vr_state_t2str(state),
					bt_bdaddr_t2str(bd_addr, last_addr));
}
#else
static void vr_cmd_cb(bthf_vr_state_t state)
{
	haltest_info("%s: state=%s\n", __func__, bthf_vr_state_t2str(state));
}
#endif

/* Callback for answer incoming call (ATA) */
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static void answer_call_cmd_cb(bt_bdaddr_t *bd_addr)
{
	haltest_info("%s: bd_addr=%s\n", __func__,
					bt_bdaddr_t2str(bd_addr, last_addr));
}
#else
static void answer_call_cmd_cb(void)
{
	haltest_info("%s\n", __func__);
}
#endif

/* Callback for disconnect call (AT+CHUP) */
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static void hangup_call_cmd_cb(bt_bdaddr_t *bd_addr)
{
	haltest_info("%s: bd_addr=%s\n", __func__,
					bt_bdaddr_t2str(bd_addr, last_addr));
}
#else
static void hangup_call_cmd_cb(void)
{
	haltest_info("%s\n", __func__);
}
#endif

/*
 * Callback for disconnect call (AT+CHUP)
 * type will denote Speaker/Mic gain (BtHfVolumeControl).
 */
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static void volume_cmd_cb(bthf_volume_type_t type, int volume,
							bt_bdaddr_t *bd_addr)
{
	haltest_info("%s: type=%s volume=%d bd_addr=%s\n", __func__,
					bthf_volume_type_t2str(type), volume,
					bt_bdaddr_t2str(bd_addr, last_addr));
}
#else
static void volume_cmd_cb(bthf_volume_type_t type, int volume)
{
	haltest_info("%s: type=%s volume=%d\n", __func__,
					bthf_volume_type_t2str(type), volume);
}
#endif

/*
 * Callback for dialing an outgoing call
 * If number is NULL, redial
 */
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static void dial_call_cmd_cb(char *number, bt_bdaddr_t *bd_addr)
{
	haltest_info("%s: number=%s bd_addr=%s\n", __func__, number,
					bt_bdaddr_t2str(bd_addr, last_addr));
}
#else
static void dial_call_cmd_cb(char *number)
{
	haltest_info("%s: number=%s\n", __func__, number);
}
#endif

/*
 * Callback for sending DTMF tones
 * tone contains the dtmf character to be sent
 */
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static void dtmf_cmd_cb(char tone, bt_bdaddr_t *bd_addr)
{
	haltest_info("%s: tone=%d bd_addr=%s\n", __func__, tone,
					bt_bdaddr_t2str(bd_addr, last_addr));
}
#else
static void dtmf_cmd_cb(char tone)
{
	haltest_info("%s: tone=%d\n", __func__, tone);
}
#endif

/*
 * Callback for enabling/disabling noise reduction/echo cancellation
 * value will be 1 to enable, 0 to disable
 */
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static void nrec_cmd_cb(bthf_nrec_t nrec, bt_bdaddr_t *bd_addr)
{
	haltest_info("%s: nrec=%s bd_addr=%s\n", __func__,
					bthf_nrec_t2str(nrec),
					bt_bdaddr_t2str(bd_addr, last_addr));
}
#else
static void nrec_cmd_cb(bthf_nrec_t nrec)
{
	haltest_info("%s: nrec=%s\n", __func__, bthf_nrec_t2str(nrec));
}
#endif

/*
 * Callback for call hold handling (AT+CHLD)
 * value will contain the call hold command (0, 1, 2, 3)
 */
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static void chld_cmd_cb(bthf_chld_type_t chld, bt_bdaddr_t *bd_addr)
{
	haltest_info("%s: chld=%s bd_addr=%s\n", __func__,
					bthf_chld_type_t2str(chld),
					bt_bdaddr_t2str(bd_addr, last_addr));
}
#else
static void chld_cmd_cb(bthf_chld_type_t chld)
{
	haltest_info("%s: chld=%s\n", __func__, bthf_chld_type_t2str(chld));
}
#endif

/* Callback for CNUM (subscriber number) */
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static void cnum_cmd_cb(bt_bdaddr_t *bd_addr)
{
	haltest_info("%s: bd_addr=%s\n", __func__,
					bt_bdaddr_t2str(bd_addr, last_addr));
}
#else
static void cnum_cmd_cb(void)
{
	haltest_info("%s\n", __func__);
}
#endif

/* Callback for indicators (CIND) */
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static void cind_cmd_cb(bt_bdaddr_t *bd_addr)
{
	haltest_info("%s: bd_addr=%s\n", __func__,
					bt_bdaddr_t2str(bd_addr, last_addr));
}
#else
static void cind_cmd_cb(void)
{
	haltest_info("%s\n", __func__);
}
#endif

/* Callback for operator selection (COPS) */
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static void cops_cmd_cb(bt_bdaddr_t *bd_addr)
{
	haltest_info("%s: bd_addr=%s\n", __func__,
					bt_bdaddr_t2str(bd_addr, last_addr));
}
#else
static void cops_cmd_cb(void)
{
	haltest_info("%s\n", __func__);
}
#endif

/* Callback for call list (AT+CLCC) */
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static void clcc_cmd_cb(bt_bdaddr_t *bd_addr)
{
	haltest_info("%s: bd_addr=%s\n", __func__,
					bt_bdaddr_t2str(bd_addr, last_addr));
}
#else
static void clcc_cmd_cb(void)
{
	haltest_info("%s\n", __func__);
}
#endif

/*
 * Callback for unknown AT command recd from HF
 * at_string will contain the unparsed AT string
 */
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static void unknown_at_cmd_cb(char *at_string, bt_bdaddr_t *bd_addr)
{
	haltest_info("%s: at_string=%s bd_addr=%s\n", __func__, at_string,
					bt_bdaddr_t2str(bd_addr, last_addr));
}
#else
static void unknown_at_cmd_cb(char *at_string)
{
	haltest_info("%s: at_string=%s\n", __func__, at_string);
}
#endif

/* Callback for keypressed (HSP) event. */
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static void key_pressed_cmd_cb(bt_bdaddr_t *bd_addr)
{
	haltest_info("%s: bd_addr=%s\n", __func__,
					bt_bdaddr_t2str(bd_addr, last_addr));
}
#else
static void key_pressed_cmd_cb(void)
{
	haltest_info("%s\n", __func__);
}
#endif

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static void wbs_cb(bthf_wbs_config_t wbs, bt_bdaddr_t *bd_addr)
{
	haltest_info("%s: bd_addr=%s\n", __func__,
					bt_bdaddr_t2str(bd_addr, last_addr));
}
#endif

static bthf_callbacks_t hf_cbacks = {
	.size = sizeof(hf_cbacks),
	.connection_state_cb = connection_state_cb,
	.audio_state_cb = audio_state_cb,
	.vr_cmd_cb = vr_cmd_cb,
	.answer_call_cmd_cb = answer_call_cmd_cb,
	.hangup_call_cmd_cb = hangup_call_cmd_cb,
	.volume_cmd_cb = volume_cmd_cb,
	.dial_call_cmd_cb = dial_call_cmd_cb,
	.dtmf_cmd_cb = dtmf_cmd_cb,
	.nrec_cmd_cb = nrec_cmd_cb,
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	.wbs_cb = wbs_cb,
#endif
	.chld_cmd_cb = chld_cmd_cb,
	.cnum_cmd_cb = cnum_cmd_cb,
	.cind_cmd_cb = cind_cmd_cb,
	.cops_cmd_cb = cops_cmd_cb,
	.clcc_cmd_cb = clcc_cmd_cb,
	.unknown_at_cmd_cb = unknown_at_cmd_cb,
	.key_pressed_cmd_cb = key_pressed_cmd_cb,
};

/* init */

static void init_p(int argc, const char **argv)
{
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	int max_hf_clients;
#endif

	RETURN_IF_NULL(if_hf);

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	if (argc <= 2)
		max_hf_clients = 1;
	else
		max_hf_clients = atoi(argv[2]);

	EXEC(if_hf->init, &hf_cbacks, max_hf_clients);
#else
	EXEC(if_hf->init, &hf_cbacks);
#endif
}

/* connect */

static void connect_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 3) {
		*user = NULL;
		*enum_func = enum_devices;
	}
}

static void connect_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;

	RETURN_IF_NULL(if_hf);
	VERIFY_ADDR_ARG(2, &addr);

	EXEC(if_hf->connect, &addr);
}

/* disconnect */

/*
 * This completion function will be used for several methods
 * returning recently connected address
 */
static void connected_addr_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 3) {
		*user = last_addr;
		*enum_func = enum_one_string;
	}
}

/* Map completion to connected_addr_c */
#define disconnect_c connected_addr_c

static void disconnect_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;

	RETURN_IF_NULL(if_hf);
	VERIFY_ADDR_ARG(2, &addr);

	EXEC(if_hf->disconnect, &addr);
}

/* create an audio connection */

/* Map completion to connected_addr_c */
#define connect_audio_c connected_addr_c

static void connect_audio_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;

	RETURN_IF_NULL(if_hf);
	VERIFY_ADDR_ARG(2, &addr);

	EXEC(if_hf->connect_audio, &addr);
}

/* close the audio connection */

/* Map completion to connected_addr_c */
#define disconnect_audio_c connected_addr_c

static void disconnect_audio_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;

	RETURN_IF_NULL(if_hf);
	VERIFY_ADDR_ARG(2, &addr);

	EXEC(if_hf->disconnect_audio, &addr);
}

/* start voice recognition */

static void start_voice_recognition_p(int argc, const char **argv)
{
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	bt_bdaddr_t addr;
#endif

	RETURN_IF_NULL(if_hf);

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	VERIFY_ADDR_ARG(2, &addr);

	EXEC(if_hf->start_voice_recognition, &addr);
#else
	EXEC(if_hf->start_voice_recognition);
#endif
}

/* stop voice recognition */

static void stop_voice_recognition_p(int argc, const char **argv)
{
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	bt_bdaddr_t addr;
#endif

	RETURN_IF_NULL(if_hf);

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	VERIFY_ADDR_ARG(2, &addr);

	EXEC(if_hf->stop_voice_recognition, &addr);
#else
	EXEC(if_hf->stop_voice_recognition);
#endif
}

/* volume control */

static void volume_control_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 3) {
		*user = TYPE_ENUM(bthf_volume_type_t);
		*enum_func = enum_defines;
	}
}

static void volume_control_p(int argc, const char **argv)
{
	bthf_volume_type_t type;
	int volume;
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	bt_bdaddr_t addr;
#endif

	RETURN_IF_NULL(if_hf);

	/* volume type */
	if (argc <= 2) {
		haltest_error("No volume type specified\n");
		return;
	}
	type = str2bthf_volume_type_t(argv[2]);

	/* volume */
	if (argc <= 3) {
		haltest_error("No volume specified\n");
		return;
	}
	volume = atoi(argv[3]);

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	VERIFY_ADDR_ARG(4, &addr);

	EXEC(if_hf->volume_control, type, volume, &addr);
#else
	EXEC(if_hf->volume_control, type, volume);
#endif
}

/* Combined device status change notification */

static void device_status_notification_c(int argc, const char **argv,
							enum_func *enum_func,
							void **user)
{
	if (argc == 3) {
		*user = TYPE_ENUM(bthf_network_state_t);
		*enum_func = enum_defines;
	} else if (argc == 4) {
		*user = TYPE_ENUM(bthf_service_type_t);
		*enum_func = enum_defines;
	}
}

static void device_status_notification_p(int argc, const char **argv)
{
	bthf_network_state_t ntk_state;
	bthf_service_type_t svc_type;
	int signal;
	int batt_chg;

	RETURN_IF_NULL(if_hf);

	/* network state */
	if (argc <= 2) {
		haltest_error("No network state specified\n");
		return;
	}
	ntk_state = str2bthf_network_state_t(argv[2]);

	/* service type */
	if (argc <= 3) {
		haltest_error("No service type specified\n");
		return;
	}
	svc_type = str2bthf_service_type_t(argv[3]);

	/* signal */
	if (argc <= 4) {
		haltest_error("No signal specified\n");
		return;
	}
	signal = atoi(argv[4]);

	/* batt_chg */
	if (argc <= 5) {
		haltest_error("No batt_chg specified\n");
		return;
	}
	batt_chg = atoi(argv[5]);

	EXEC(if_hf->device_status_notification, ntk_state, svc_type, signal,
								batt_chg);
}

/* Response for COPS command */

static void cops_response_p(int argc, const char **argv)
{
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	bt_bdaddr_t addr;
#endif

	RETURN_IF_NULL(if_hf);

	/* response */
	if (argc <= 2) {
		haltest_error("No cops specified\n");
		return;
	}

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	VERIFY_ADDR_ARG(3, &addr);

	EXEC(if_hf->cops_response, argv[2], &addr);
#else
	EXEC(if_hf->cops_response, argv[2]);
#endif
}

/* Response for CIND command */

static void cind_response_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 6) {
		*user = TYPE_ENUM(bthf_call_state_t);
		*enum_func = enum_defines;
	}
}

static void cind_response_p(int argc, const char **argv)
{
	int svc;
	int num_active;
	int num_held;
	bthf_call_state_t call_setup_state;
	int signal;
	int roam;
	int batt_chg;
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	bt_bdaddr_t addr;
#endif

	RETURN_IF_NULL(if_hf);

	/* svc */
	if (argc <= 2) {
		haltest_error("No service specified\n");
		return;
	}
	svc = atoi(argv[2]);

	/* num active */
	if (argc <= 3) {
		haltest_error("No num active specified\n");
		return;
	}
	num_active = atoi(argv[3]);

	/* num held */
	if (argc <= 4) {
		haltest_error("No num held specified\n");
		return;
	}
	num_held = atoi(argv[4]);

	/* call setup state */
	if (argc <= 5) {
		haltest_error("No call setup state specified\n");
		return;
	}
	call_setup_state = str2bthf_call_state_t(argv[5]);

	/* signal */
	if (argc <= 6) {
		haltest_error("No signal specified\n");
		return;
	}
	signal = atoi(argv[6]);

	/* roam */
	if (argc <= 7) {
		haltest_error("No roam specified\n");
		return;
	}
	roam = atoi(argv[7]);

	/* batt_chg */
	if (argc <= 8) {
		haltest_error("No batt_chg specified\n");
		return;
	}
	batt_chg = atoi(argv[8]);

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	VERIFY_ADDR_ARG(9, &addr);

	EXEC(if_hf->cind_response, svc, num_active, num_held, call_setup_state,
						signal, roam, batt_chg, &addr);
#else
	EXEC(if_hf->cind_response, svc, num_active, num_held, call_setup_state,
							signal, roam, batt_chg);
#endif
}

/* Pre-formatted AT response, typically in response to unknown AT cmd */

static void formatted_at_response_p(int argc, const char **argv)
{
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	bt_bdaddr_t addr;
#endif

	RETURN_IF_NULL(if_hf);

	/* response */
	if (argc <= 2) {
		haltest_error("No response specified\n");
		return;
	}

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	VERIFY_ADDR_ARG(3, &addr);

	EXEC(if_hf->formatted_at_response, argv[2], &addr);
#else
	EXEC(if_hf->formatted_at_response, argv[2]);
#endif
}

/* at_response */

static void at_response_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 3) {
		*user = TYPE_ENUM(bthf_at_response_t);
		*enum_func = enum_defines;
	}
}

static void at_response_p(int argc, const char **argv)
{
	bthf_at_response_t response_code;
	int error_code;
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	bt_bdaddr_t addr;
#endif

	RETURN_IF_NULL(if_hf);

	/* response type */
	if (argc <= 2) {
		haltest_error("No response specified\n");
		return;
	}
	response_code = str2bthf_at_response_t(argv[2]);

	/* error code */
	if (argc <= 3)
		error_code = 0;
	else
		error_code = atoi(argv[3]);

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	VERIFY_ADDR_ARG(4, &addr);

	EXEC(if_hf->at_response, response_code, error_code, &addr);
#else
	EXEC(if_hf->at_response, response_code, error_code);
#endif
}

/* response for CLCC command */

static void clcc_response_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 4) {
		*user = TYPE_ENUM(bthf_call_direction_t);
		*enum_func = enum_defines;
	} else if (argc == 5) {
		*user = TYPE_ENUM(bthf_call_state_t);
		*enum_func = enum_defines;
	} else if (argc == 6) {
		*user = TYPE_ENUM(bthf_call_mode_t);
		*enum_func = enum_defines;
	} else if (argc == 7) {
		*user = TYPE_ENUM(bthf_call_mpty_type_t);
		*enum_func = enum_defines;
	} else if (argc == 9) {
		*user = TYPE_ENUM(bthf_call_addrtype_t);
		*enum_func = enum_defines;
	}
}

static void clcc_response_p(int argc, const char **argv)
{
	int index;
	bthf_call_direction_t dir;
	bthf_call_state_t state;
	bthf_call_mode_t mode;
	bthf_call_mpty_type_t mpty;
	const char *number;
	bthf_call_addrtype_t type;
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	bt_bdaddr_t addr;
#endif

	RETURN_IF_NULL(if_hf);

	/* index */
	if (argc <= 2) {
		haltest_error("No index specified\n");
		return;
	}
	index = atoi(argv[2]);

	/* direction */
	if (argc <= 3) {
		haltest_error("No direction specified\n");
		return;
	}
	dir = str2bthf_call_direction_t(argv[3]);

	/* call state */
	if (argc <= 4) {
		haltest_error("No call state specified\n");
		return;
	}
	state = str2bthf_call_state_t(argv[4]);

	/* call mode */
	if (argc <= 5) {
		haltest_error("No mode specified\n");
		return;
	}
	mode = str2bthf_call_mode_t(argv[5]);

	/* call mpty type */
	if (argc <= 6) {
		haltest_error("No mpty type specified\n");
		return;
	}
	mpty = str2bthf_call_mpty_type_t(argv[6]);

	/* number */
	if (argc <= 7) {
		haltest_error("No number specified\n");
		return;
	}
	number = argv[7];

	/* call mpty type */
	if (argc <= 8) {
		haltest_error("No address type specified\n");
		return;
	}
	type = str2bthf_call_addrtype_t(argv[8]);

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	VERIFY_ADDR_ARG(9, &addr);

	EXEC(if_hf->clcc_response, index, dir, state, mode, mpty, number,
								type, &addr);
#else
	EXEC(if_hf->clcc_response, index, dir, state, mode, mpty, number,
									type);
#endif
}

#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
static void configure_wbs_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 4) {
		*user = TYPE_ENUM(bthf_wbs_config_t);
		*enum_func = enum_defines;
	}
}

static void configure_wbs_p(int argc, const char **argv)
{
	bthf_wbs_config_t wbs;
	bt_bdaddr_t addr;

	RETURN_IF_NULL(if_hf);

	if (argc <= 3) {
		haltest_error("Too few parameters specified\n");
		return;
	}

	VERIFY_ADDR_ARG(2, &addr);
	wbs = str2bthf_wbs_config_t(argv[3]);

	EXEC(if_hf->configure_wbs, &addr, wbs);
}
#endif

/* phone state change */

static void phone_state_change_c(int argc, const char **argv,
					enum_func *enum_func, void **user)
{
	if (argc == 5) {
		*user = TYPE_ENUM(bthf_call_state_t);
		*enum_func = enum_defines;
	} else if (argc == 7) {
		*user = TYPE_ENUM(bthf_call_addrtype_t);
		*enum_func = enum_defines;
	}
}

static void phone_state_change_p(int argc, const char **argv)
{
	int num_active;
	int num_held;
	bthf_call_state_t call_setup_state;
	const char *number;
	bthf_call_addrtype_t type;

	RETURN_IF_NULL(if_hf);

	/* num_active */
	if (argc <= 2) {
		haltest_error("No num_active specified\n");
		return;
	}
	num_active = atoi(argv[2]);

	/* num_held */
	if (argc <= 3) {
		haltest_error("No num_held specified\n");
		return;
	}
	num_held = atoi(argv[3]);

	/* setup state */
	if (argc <= 4) {
		haltest_error("No call setup state specified\n");
		return;
	}
	call_setup_state = str2bthf_call_state_t(argv[4]);

	/* number */
	if (argc <= 5) {
		haltest_error("No number specified\n");
		return;
	}
	number = argv[5];

	/* call mpty type */
	if (argc <= 6) {
		haltest_error("No address type specified\n");
		return;
	}
	type = str2bthf_call_addrtype_t(argv[6]);

	EXEC(if_hf->phone_state_change, num_active, num_held, call_setup_state,
								number, type);
}

/* cleanup */

static void cleanup_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_hf);

	EXECV(if_hf->cleanup);
	if_hf = NULL;
}

static struct method methods[] = {
#if ANDROID_VERSION >= PLATFORM_VER(5, 0, 0)
	STD_METHODH(init, "[<max_hf_clients>]"),
	STD_METHODH(start_voice_recognition, "<addr>"),
	STD_METHODH(stop_voice_recognition, "<addr>"),
	STD_METHODCH(volume_control, "<vol_type> <volume> <addr>"),
	STD_METHODH(cops_response, "<cops string> <addr>"),
	STD_METHODCH(cind_response,
			"<svc> <num_active> <num_held> <setup_state> <signal> "
			"<roam> <batt_chg> <addr>"),
	STD_METHODH(formatted_at_response, "<at_response> <addr>"),
	STD_METHODCH(at_response, "<response_code> [<error_code> <bdaddr>]"),
	STD_METHODCH(clcc_response,
			"<index> <direction> <state> <mode> <mpty> <number> "
			"<type> <addr>"),
	STD_METHODCH(configure_wbs, "<addr> <wbs config>"),
#else
	STD_METHOD(init),
	STD_METHOD(start_voice_recognition),
	STD_METHOD(stop_voice_recognition),
	STD_METHODCH(volume_control, "<vol_type> <volume>"),
	STD_METHODH(cops_response, "<cops string>"),
	STD_METHODCH(cind_response,
			"<svc> <num_active> <num_held> <setup_state> <signal> "
			"<roam> <batt_chg>"),
	STD_METHODH(formatted_at_response, "<at_response>"),
	STD_METHODCH(at_response, "<response_code> [<error_code>]"),
	STD_METHODCH(clcc_response,
			"<index> <direction> <state> <mode> <mpty> <number> "
			"<type>"),
#endif
	STD_METHODCH(connect, "<addr>"),
	STD_METHODCH(disconnect, "<addr>"),
	STD_METHODCH(connect_audio, "<addr>"),
	STD_METHODCH(disconnect_audio, "<addr>"),
	STD_METHODCH(device_status_notification,
			"<ntk_state> <svt_type> <signal> <batt_chg>"),
	STD_METHODCH(phone_state_change,
			"<num_active> <num_held> <setup_state> <number> "
			"<type>"),
	STD_METHOD(cleanup),
	END_METHOD
};

const struct interface hf_if = {
	.name = "handsfree",
	.methods = methods
};

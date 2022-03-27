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

#include "if-main.h"
#include "../hal-utils.h"

const bthf_client_interface_t *if_hf_client = NULL;

static char last_addr[MAX_ADDR_STR_LEN];

SINTMAP(bthf_client_connection_state_t, -1, "(unknown)")
	DELEMENT(BTHF_CLIENT_CONNECTION_STATE_DISCONNECTED),
	DELEMENT(BTHF_CLIENT_CONNECTION_STATE_CONNECTING),
	DELEMENT(BTHF_CLIENT_CONNECTION_STATE_CONNECTED),
	DELEMENT(BTHF_CLIENT_CONNECTION_STATE_SLC_CONNECTED),
	DELEMENT(BTHF_CLIENT_CONNECTION_STATE_DISCONNECTING),
ENDMAP

SINTMAP(bthf_client_audio_state_t, -1, "(unknown)")
	DELEMENT(BTHF_CLIENT_AUDIO_STATE_DISCONNECTED),
	DELEMENT(BTHF_CLIENT_AUDIO_STATE_CONNECTING),
	DELEMENT(BTHF_CLIENT_AUDIO_STATE_CONNECTED),
	DELEMENT(BTHF_CLIENT_AUDIO_STATE_CONNECTED_MSBC),
ENDMAP

SINTMAP(bthf_client_vr_state_t, -1, "(unknown)")
	DELEMENT(BTHF_CLIENT_VR_STATE_STOPPED),
	DELEMENT(BTHF_CLIENT_VR_STATE_STARTED),
ENDMAP

SINTMAP(bthf_client_network_state_t, -1, "(unknown)")
	DELEMENT(BTHF_CLIENT_NETWORK_STATE_NOT_AVAILABLE),
	DELEMENT(BTHF_CLIENT_NETWORK_STATE_AVAILABLE),
ENDMAP

SINTMAP(bthf_client_service_type_t, -1, "(unknown)")
	DELEMENT(BTHF_CLIENT_SERVICE_TYPE_HOME),
	DELEMENT(BTHF_CLIENT_SERVICE_TYPE_ROAMING),
ENDMAP

SINTMAP(bthf_client_call_t, -1, "(unknown)")
	DELEMENT(BTHF_CLIENT_CALL_NO_CALLS_IN_PROGRESS),
	DELEMENT(BTHF_CLIENT_CALL_CALLS_IN_PROGRESS),
ENDMAP

SINTMAP(bthf_client_callsetup_t, -1, "(unknown)")
	DELEMENT(BTHF_CLIENT_CALLSETUP_NONE),
	DELEMENT(BTHF_CLIENT_CALLSETUP_INCOMING),
	DELEMENT(BTHF_CLIENT_CALLSETUP_OUTGOING),
	DELEMENT(BTHF_CLIENT_CALLSETUP_ALERTING),
ENDMAP

SINTMAP(bthf_client_callheld_t, -1, "(unknown)")
	DELEMENT(BTHF_CLIENT_CALLHELD_NONE),
	DELEMENT(BTHF_CLIENT_CALLHELD_HOLD_AND_ACTIVE),
	DELEMENT(BTHF_CLIENT_CALLHELD_HOLD),
ENDMAP

SINTMAP(bthf_client_resp_and_hold_t, -1, "(unknown)")
	DELEMENT(BTHF_CLIENT_RESP_AND_HOLD_HELD),
	DELEMENT(BTRH_CLIENT_RESP_AND_HOLD_ACCEPT),
	DELEMENT(BTRH_CLIENT_RESP_AND_HOLD_REJECT),
ENDMAP

SINTMAP(bthf_client_call_direction_t, -1, "(unknown)")
	DELEMENT(BTHF_CLIENT_CALL_DIRECTION_OUTGOING),
	DELEMENT(BTHF_CLIENT_CALL_DIRECTION_INCOMING),
ENDMAP

SINTMAP(bthf_client_call_state_t, -1, "(unknown)")
	DELEMENT(BTHF_CLIENT_CALL_STATE_ACTIVE),
	DELEMENT(BTHF_CLIENT_CALL_STATE_HELD),
	DELEMENT(BTHF_CLIENT_CALL_STATE_DIALING),
	DELEMENT(BTHF_CLIENT_CALL_STATE_ALERTING),
	DELEMENT(BTHF_CLIENT_CALL_STATE_INCOMING),
	DELEMENT(BTHF_CLIENT_CALL_STATE_WAITING),
	DELEMENT(BTHF_CLIENT_CALL_STATE_HELD_BY_RESP_HOLD),
ENDMAP

SINTMAP(bthf_client_call_mpty_type_t, -1, "(unknown)")
	DELEMENT(BTHF_CLIENT_CALL_MPTY_TYPE_SINGLE),
	DELEMENT(BTHF_CLIENT_CALL_MPTY_TYPE_MULTI),
ENDMAP

SINTMAP(bthf_client_volume_type_t, -1, "(unknown)")
	DELEMENT(BTHF_CLIENT_VOLUME_TYPE_SPK),
	DELEMENT(BTHF_CLIENT_VOLUME_TYPE_MIC),
ENDMAP

SINTMAP(bthf_client_cmd_complete_t, -1, "(unknown)")
	DELEMENT(BTHF_CLIENT_CMD_COMPLETE_OK),
	DELEMENT(BTHF_CLIENT_CMD_COMPLETE_ERROR),
	DELEMENT(BTHF_CLIENT_CMD_COMPLETE_ERROR_NO_CARRIER),
	DELEMENT(BTHF_CLIENT_CMD_COMPLETE_ERROR_BUSY),
	DELEMENT(BTHF_CLIENT_CMD_COMPLETE_ERROR_NO_ANSWER),
	DELEMENT(BTHF_CLIENT_CMD_COMPLETE_ERROR_DELAYED),
	DELEMENT(BTHF_CLIENT_CMD_COMPLETE_ERROR_BLACKLISTED),
	DELEMENT(BTHF_CLIENT_CMD_COMPLETE_ERROR_CME),
ENDMAP

SINTMAP(bthf_client_subscriber_service_type_t, -1, "(unknown)")
	DELEMENT(BTHF_CLIENT_SERVICE_UNKNOWN),
	DELEMENT(BTHF_CLIENT_SERVICE_VOICE),
	DELEMENT(BTHF_CLIENT_SERVICE_FAX),
ENDMAP

SINTMAP(bthf_client_in_band_ring_state_t, -1, "(unknown)")
	DELEMENT(BTHF_CLIENT_IN_BAND_RINGTONE_NOT_PROVIDED),
	DELEMENT(BTHF_CLIENT_IN_BAND_RINGTONE_PROVIDED),
ENDMAP

SINTMAP(bthf_client_call_action_t, -1, "(unknown)")
	DELEMENT(BTHF_CLIENT_CALL_ACTION_CHLD_0),
	DELEMENT(BTHF_CLIENT_CALL_ACTION_CHLD_1),
	DELEMENT(BTHF_CLIENT_CALL_ACTION_CHLD_2),
	DELEMENT(BTHF_CLIENT_CALL_ACTION_CHLD_3),
	DELEMENT(BTHF_CLIENT_CALL_ACTION_CHLD_4),
	DELEMENT(BTHF_CLIENT_CALL_ACTION_CHLD_1x),
	DELEMENT(BTHF_CLIENT_CALL_ACTION_CHLD_2x),
	DELEMENT(BTHF_CLIENT_CALL_ACTION_ATA),
	DELEMENT(BTHF_CLIENT_CALL_ACTION_CHUP),
	DELEMENT(BTHF_CLIENT_CALL_ACTION_BTRH_0),
	DELEMENT(BTHF_CLIENT_CALL_ACTION_BTRH_1),
	DELEMENT(BTHF_CLIENT_CALL_ACTION_BTRH_2),
ENDMAP

/* Callbacks */

static char features_str[512];

static const char *pear_features_t2str(int feat)
{
	memset(features_str, 0, sizeof(features_str));

	sprintf(features_str, "BTHF_CLIENT_PEER_FEAT_3WAY: %s,\n"
			"BTHF_CLIENT_PEER_FEAT_ECNR: %s,\n"
			"BTHF_CLIENT_PEER_FEAT_VREC: %s,\n"
			"BTHF_CLIENT_PEER_FEAT_INBAND: %s,\n"
			"BTHF_CLIENT_PEER_FEAT_VTAG: %s,\n"
			"BTHF_CLIENT_PEER_FEAT_REJECT: %s,\n"
			"BTHF_CLIENT_PEER_FEAT_ECS: %s,\n"
			"BTHF_CLIENT_PEER_FEAT_ECC: %s,\n"
			"BTHF_CLIENT_PEER_FEAT_EXTERR: %s,\n"
			"BTHF_CLIENT_PEER_FEAT_CODEC: %s,\n",
			feat & BTHF_CLIENT_PEER_FEAT_3WAY ? "True" : "False",
			feat & BTHF_CLIENT_PEER_FEAT_ECNR ? "True" : "False",
			feat & BTHF_CLIENT_PEER_FEAT_VREC ? "True" : "False",
			feat & BTHF_CLIENT_PEER_FEAT_INBAND ? "True" : "False",
			feat & BTHF_CLIENT_PEER_FEAT_VTAG ? "True" : "False",
			feat & BTHF_CLIENT_PEER_FEAT_REJECT ? "True" : "False",
			feat & BTHF_CLIENT_PEER_FEAT_ECS ? "True" : "False",
			feat & BTHF_CLIENT_PEER_FEAT_ECC ? "True" : "False",
			feat & BTHF_CLIENT_PEER_FEAT_EXTERR ? "True" : "False",
			feat & BTHF_CLIENT_PEER_FEAT_CODEC ? "True" : "False");

	return features_str;
}

static const char *chld_features_t2str(int feat)
{
	memset(features_str, 0, sizeof(features_str));

	sprintf(features_str,
		"BTHF_CLIENT_CHLD_FEAT_REL: %s,\n"
		"BTHF_CLIENT_CHLD_FEAT_REL_ACC: %s,\n"
		"BTHF_CLIENT_CHLD_FEAT_REL_X: %s,\n"
		"BTHF_CLIENT_CHLD_FEAT_HOLD_ACC: %s,\n"
		"BTHF_CLIENT_CHLD_FEAT_PRIV_X: %s,\n"
		"BTHF_CLIENT_CHLD_FEAT_MERGE: %s,\n"
		"BTHF_CLIENT_CHLD_FEAT_MERGE_DETACH: %s,\n",
		feat & BTHF_CLIENT_CHLD_FEAT_REL ? "True" : "False",
		feat & BTHF_CLIENT_CHLD_FEAT_REL_ACC ? "True" : "False",
		feat & BTHF_CLIENT_CHLD_FEAT_REL_X ? "True" : "False",
		feat & BTHF_CLIENT_CHLD_FEAT_HOLD_ACC ? "True" : "False",
		feat & BTHF_CLIENT_CHLD_FEAT_PRIV_X ? "True" : "False",
		feat & BTHF_CLIENT_CHLD_FEAT_MERGE ? "True" : "False",
		feat & BTHF_CLIENT_CHLD_FEAT_MERGE_DETACH ? "True" : "False");

	return features_str;
}

/* Callback for connection state change. */
static void hf_client_connection_state_callback(
					bthf_client_connection_state_t state,
					unsigned int peer_feat,
					unsigned int chld_feat,
					bt_bdaddr_t *bd_addr)
{
	haltest_info("%s: state=%s bd_addr=%s\n", __func__,
				bthf_client_connection_state_t2str(state),
				bt_bdaddr_t2str(bd_addr, last_addr));

	if (state != BTHF_CLIENT_CONNECTION_STATE_CONNECTED)
		return;

	haltest_info("\tpeer_features%s\n", pear_features_t2str(peer_feat));
	haltest_info("\tchld_feat=%s\n", chld_features_t2str(chld_feat));
}

/* Callback for audio connection state change. */
static void hf_client_audio_state_callback(bthf_client_audio_state_t state,
							bt_bdaddr_t *bd_addr)
{
	haltest_info("%s: state=%s bd_addr=%s\n", __func__,
				bthf_client_audio_state_t2str(state),
				bt_bdaddr_t2str(bd_addr, last_addr));
}

/* Callback for VR connection state change. */
static void hf_client_vr_cmd_callback(bthf_client_vr_state_t state)
{
	haltest_info("%s: vr_state=%s\n", __func__,
					bthf_client_vr_state_t2str(state));
}

/* Callback for network state change */
static void hf_client_network_state_callback(bthf_client_network_state_t state)
{
	haltest_info("%s: network_state=%s\n", __func__,
					bthf_client_network_state_t2str(state));
}

/* Callback for network roaming status change */
static void hf_client_network_roaming_callback(bthf_client_service_type_t type)
{
	haltest_info("%s: service_type=%s\n", __func__,
					bthf_client_service_type_t2str(type));
}

/* Callback for signal strength indication */
static void hf_client_network_signal_callback(int signal_strength)
{
	haltest_info("%s: signal strength=%d\n", __func__, signal_strength);
}

/* Callback for battery level indication */
static void hf_client_battery_level_callback(int battery_level)
{
	haltest_info("%s: battery_lvl=%d\n", __func__, battery_level);
}

/* Callback for current operator name */
static void hf_client_current_operator_callback(const char *name)
{
	haltest_info("%s: operator_name=%s\n", __func__, name);
}

/* Callback for call indicator */
static void hf_client_call_callback(bthf_client_call_t call)
{
	haltest_info("%s: call_state=%s\n", __func__,
						bthf_client_call_t2str(call));
}

/* Callback for callsetup indicator */
static void hf_client_callsetup_callback(bthf_client_callsetup_t callsetup)
{
	haltest_info("%s: callsetup=%s\n", __func__,
					bthf_client_callsetup_t2str(callsetup));
}

/* Callback for callheld indicator */
static void hf_client_callheld_callback(bthf_client_callheld_t callheld)
{
	haltest_info("%s: callheld=%s\n", __func__,
					bthf_client_callheld_t2str(callheld));
}

/* Callback for response and hold */
static void hf_client_resp_and_hold_callback(
				bthf_client_resp_and_hold_t resp_and_hold)
{
	haltest_info("%s: resp_and_hold=%s\n", __func__,
				bthf_client_resp_and_hold_t2str(resp_and_hold));
}

/* Callback for Calling Line Identification notification */
static void hf_client_clip_callback(const char *number)
{
	haltest_info("%s: number=%s\n", __func__, number);
}

/* Callback for Call Waiting notification */
static void hf_client_call_waiting_callback(const char *number)
{
	haltest_info("%s: number=%s\n", __func__, number);
}

/* Callback for listing current calls. Can be called multiple time. */
static void hf_client_current_calls_callback(int index,
					bthf_client_call_direction_t dir,
					bthf_client_call_state_t state,
					bthf_client_call_mpty_type_t mpty,
					const char *number)
{
	haltest_info("%s: index=%d, direction=%s, state=%s, m_party=%s\n",
					__func__, index,
					bthf_client_call_direction_t2str(dir),
					bthf_client_call_state_t2str(state),
					bthf_client_call_mpty_type_t2str(mpty));

	if (number)
		haltest_info("%s: number=%s\n", __func__, number);
}

/* Callback for audio volume change */
static void hf_client_volume_change_callback(bthf_client_volume_type_t type,
								int volume)
{
	haltest_info("%s: vol_type=%s, value=%d\n", __func__,
				bthf_client_volume_type_t2str(type), volume);
}

/* Callback for command complete event */
static void hf_client_cmd_complete_callback(bthf_client_cmd_complete_t type,
									int cme)
{
	haltest_info("%s: type=%s, cme=%d\n", __func__,
				bthf_client_cmd_complete_t2str(type), cme);
}

/* Callback for subscriber information */
static void hf_client_subscriber_info_callback(const char *name,
				bthf_client_subscriber_service_type_t type)
{
	haltest_info("%s: name=%s, type=%s\n", __func__, name,
			bthf_client_subscriber_service_type_t2str(type));
}

/* Callback for in-band ring tone settings */
static void hf_client_in_band_ring_tone_callback(
				bthf_client_in_band_ring_state_t state)
{
	haltest_info("%s: state=%s\n", __func__,
				bthf_client_in_band_ring_state_t2str(state));
}

/* Callback for requested number from AG */
static void hf_client_last_voice_tag_number_callback(const char *number)
{
	haltest_info("%s: number=%s\n", __func__, number);
}

/* Callback for sending ring indication to app */
static void hf_client_ring_indication_callback(void)
{
	haltest_info("%s\n", __func__);
}

static bthf_client_callbacks_t hf_client_cbacks = {
	.size = sizeof(hf_client_cbacks),
	.connection_state_cb = hf_client_connection_state_callback,
	.audio_state_cb = hf_client_audio_state_callback,
	.vr_cmd_cb = hf_client_vr_cmd_callback,
	.network_state_cb = hf_client_network_state_callback,
	.network_roaming_cb = hf_client_network_roaming_callback,
	.network_signal_cb = hf_client_network_signal_callback,
	.battery_level_cb = hf_client_battery_level_callback,
	.current_operator_cb = hf_client_current_operator_callback,
	.call_cb = hf_client_call_callback,
	.callsetup_cb = hf_client_callsetup_callback,
	.callheld_cb = hf_client_callheld_callback,
	.resp_and_hold_cb = hf_client_resp_and_hold_callback,
	.clip_cb = hf_client_clip_callback,
	.call_waiting_cb = hf_client_call_waiting_callback,
	.current_calls_cb = hf_client_current_calls_callback,
	.volume_change_cb = hf_client_volume_change_callback,
	.cmd_complete_cb = hf_client_cmd_complete_callback,
	.subscriber_info_cb = hf_client_subscriber_info_callback,
	.in_band_ring_tone_cb = hf_client_in_band_ring_tone_callback,
	.last_voice_tag_number_callback =
				hf_client_last_voice_tag_number_callback,
	.ring_indication_cb = hf_client_ring_indication_callback,
};

/* init */
static void init_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_hf_client);

	EXEC(if_hf_client->init, &hf_client_cbacks);
}

static void connect_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 3) {
		*user = NULL;
		*enum_func = enum_devices;
	}
}

/* connect to audio gateway */
static void connect_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;

	RETURN_IF_NULL(if_hf_client);
	VERIFY_ADDR_ARG(2, &addr);

	EXEC(if_hf_client->connect, &addr);
}

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

/* disconnect from audio gateway */
static void disconnect_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;

	RETURN_IF_NULL(if_hf_client);
	VERIFY_ADDR_ARG(2, &addr);

	EXEC(if_hf_client->disconnect, &addr);
}

static void connect_audio_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 3) {
		*user = NULL;
		*enum_func = enum_devices;
	}
}

/* create an audio connection */
static void connect_audio_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;

	RETURN_IF_NULL(if_hf_client);
	VERIFY_ADDR_ARG(2, &addr);

	EXEC(if_hf_client->connect_audio, &addr);
}

/* Map completion to connected_addr_c */
#define disconnect_audio_c connected_addr_c

/* close the audio connection */
static void disconnect_audio_p(int argc, const char **argv)
{
	bt_bdaddr_t addr;

	RETURN_IF_NULL(if_hf_client);
	VERIFY_ADDR_ARG(2, &addr);

	EXEC(if_hf_client->disconnect_audio, &addr);
}

/* start voice recognition */
static void start_voice_recognition_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_hf_client);

	EXEC(if_hf_client->start_voice_recognition);
}

/* stop voice recognition */
static void stop_voice_recognition_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_hf_client);

	EXEC(if_hf_client->stop_voice_recognition);
}

static void volume_control_c(int argc, const char **argv, enum_func *enum_func,
								void **user)
{
	if (argc == 3) {
		*user = TYPE_ENUM(bthf_client_volume_type_t);
		*enum_func = enum_defines;
	}
}

/* volume control */
static void volume_control_p(int argc, const char **argv)
{
	bthf_client_volume_type_t type;
	int volume;

	RETURN_IF_NULL(if_hf_client);

	/* volume type */
	if (argc <= 2) {
		haltest_error("No volume type specified\n");
		return;
	}
	type = str2bthf_client_volume_type_t(argv[2]);

	/* volume */
	if (argc <= 3) {
		haltest_error("No volume specified\n");
		return;
	}
	volume = atoi(argv[3]);

	EXEC(if_hf_client->volume_control, type, volume);
}

/* place a call with number a number */
static void dial_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_hf_client);

	/* number string */
	if (argc <= 2) {
		haltest_info("Number not specified. Redial\n");
		EXEC(if_hf_client->dial, NULL);
		return;
	}

	EXEC(if_hf_client->dial, argv[2]);
}

/* place a call with number specified by location (speed dial) */
static void dial_memory_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_hf_client);

	/* memory index */
	if (argc <= 2) {
		haltest_error("No memory index specified\n");
		return;
	}

	EXEC(if_hf_client->dial_memory, atoi(argv[2]));
}

static void handle_call_action_c(int argc, const char **argv,
					enum_func *enum_func, void **user)
{
	if (argc == 3) {
		*user = TYPE_ENUM(bthf_client_call_action_t);
		*enum_func = enum_defines;
	}
}

/* perform specified call related action */
static void handle_call_action_p(int argc, const char **argv)
{
	bthf_client_call_action_t action;
	int index = 0;

	RETURN_IF_NULL(if_hf_client);

	/* action */
	if (argc <= 2) {
		haltest_error("No action specified\n");
		return;
	}
	action = str2bthf_client_call_action_t(argv[2]);

	/* call index */
	if (action == BTHF_CLIENT_CALL_ACTION_CHLD_1x ||
				action == BTHF_CLIENT_CALL_ACTION_CHLD_2x) {
		if (argc <= 3) {
			haltest_error("No call index specified\n");
			return;
		}
		index = atoi(argv[3]);
	}

	EXEC(if_hf_client->handle_call_action, action, index);
}

/* query list of current calls */
static void query_current_calls_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_hf_client);

	EXEC(if_hf_client->query_current_calls);
}

/* query name of current selected operator */
static void query_current_operator_name_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_hf_client);

	EXEC(if_hf_client->query_current_operator_name);
}

/* Retrieve subscriber information */
static void retrieve_subscriber_info_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_hf_client);

	EXEC(if_hf_client->retrieve_subscriber_info);
}

/* Send DTMF code*/
static void send_dtmf_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_hf_client);

	EXEC(if_hf_client->send_dtmf, *argv[2]);
}

/* Request a phone number from AG corresponding to last voice tag recorded */
static void request_last_voice_tag_number_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_hf_client);

	EXEC(if_hf_client->request_last_voice_tag_number);
}

/* Closes the interface. */
static void cleanup_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_hf_client);

	EXECV(if_hf_client->cleanup);
	if_hf_client = NULL;
}

static struct method methods[] = {
	STD_METHOD(init),
	STD_METHODCH(connect, "<addr>"),
	STD_METHODCH(disconnect, "<addr>"),
	STD_METHODCH(connect_audio, "<addr>"),
	STD_METHODCH(disconnect_audio, "<addr>"),
	STD_METHOD(start_voice_recognition),
	STD_METHOD(stop_voice_recognition),
	STD_METHODCH(volume_control, "<volume_type> <value>"),
	STD_METHODH(dial, "<destination_number>"),
	STD_METHODH(dial_memory, "<memory_location>"),
	STD_METHODCH(handle_call_action, "<call_action> <call_index>"),
	STD_METHOD(query_current_calls),
	STD_METHOD(query_current_operator_name),
	STD_METHOD(retrieve_subscriber_info),
	STD_METHODH(send_dtmf, "<code>"),
	STD_METHOD(request_last_voice_tag_number),
	STD_METHOD(cleanup),
	END_METHOD
};

const struct interface hf_client_if = {
	.name = "handsfree_client",
	.methods = methods
};

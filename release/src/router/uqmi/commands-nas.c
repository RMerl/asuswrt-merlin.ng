/*
 * uqmi -- tiny QMI support implementation
 *
 * Copyright (C) 2014-2015 Felix Fietkau <nbd@openwrt.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "qmi-message.h"

static struct qmi_nas_set_system_selection_preference_request sel_req;
static struct	{
	bool mcc_is_set;
	bool mnc_is_set;
} plmn_code_flag;

#define cmd_nas_do_set_system_selection_cb no_cb
static enum qmi_cmd_result
cmd_nas_do_set_system_selection_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_nas_set_system_selection_preference_request(msg, &sel_req);
	return QMI_CMD_REQUEST;
}

static enum qmi_cmd_result
do_sel_network(void)
{
	static bool use_sel_req = false;

	if (!use_sel_req) {
		use_sel_req = true;
		uqmi_add_command(NULL, __UQMI_COMMAND_nas_do_set_system_selection);
	}

	return QMI_CMD_DONE;
}

#define cmd_nas_set_network_modes_cb no_cb
static enum qmi_cmd_result
cmd_nas_set_network_modes_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	static const struct {
		const char *name;
		QmiNasRatModePreference val;
	} modes[] = {
		{ "cdma", QMI_NAS_RAT_MODE_PREFERENCE_CDMA_1X | QMI_NAS_RAT_MODE_PREFERENCE_CDMA_1XEVDO },
		{ "td-scdma", QMI_NAS_RAT_MODE_PREFERENCE_TD_SCDMA },
		{ "gsm", QMI_NAS_RAT_MODE_PREFERENCE_GSM },
		{ "umts", QMI_NAS_RAT_MODE_PREFERENCE_UMTS },
		{ "lte", QMI_NAS_RAT_MODE_PREFERENCE_LTE },
	};
	QmiNasRatModePreference val = 0;
	char *word;
	int i;

	for (word = strtok(arg, ",");
	     word;
	     word = strtok(NULL, ",")) {
		bool found = false;

		for (i = 0; i < ARRAY_SIZE(modes); i++) {
			if (strcmp(word, modes[i].name) != 0 &&
				strcmp(word, "all") != 0)
				continue;

			val |= modes[i].val;
			found = true;
		}

		if (!found) {
			uqmi_add_error("Invalid network mode");
			return QMI_CMD_EXIT;
		}
	}

	qmi_set(&sel_req, mode_preference, val);
	return do_sel_network();
}

#define cmd_nas_set_network_preference_cb no_cb
static enum qmi_cmd_result
cmd_nas_set_network_preference_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	QmiNasGsmWcdmaAcquisitionOrderPreference pref = QMI_NAS_GSM_WCDMA_ACQUISITION_ORDER_PREFERENCE_AUTOMATIC;

	if (!strcmp(arg, "gsm"))
		pref = QMI_NAS_GSM_WCDMA_ACQUISITION_ORDER_PREFERENCE_GSM;
	else if (!strcmp(arg, "wcdma"))
		pref = QMI_NAS_GSM_WCDMA_ACQUISITION_ORDER_PREFERENCE_WCDMA;

	qmi_set(&sel_req, gsm_wcdma_acquisition_order_preference, pref);
	return do_sel_network();
}

#define cmd_nas_set_roaming_cb no_cb
static enum qmi_cmd_result
cmd_nas_set_roaming_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	QmiNasRoamingPreference pref;

	if (!strcmp(arg, "any"))
		pref = QMI_NAS_ROAMING_PREFERENCE_ANY;
	else if (!strcmp(arg, "only"))
		pref = QMI_NAS_ROAMING_PREFERENCE_NOT_OFF;
	else if (!strcmp(arg, "off"))
		pref = QMI_NAS_ROAMING_PREFERENCE_OFF;
	else
		return uqmi_add_error("Invalid argument");

	qmi_set(&sel_req, roaming_preference, pref);
	return do_sel_network();
}

#define cmd_nas_set_mcc_cb no_cb
static enum qmi_cmd_result
cmd_nas_set_mcc_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	char *err;
	int value = strtoul(arg, &err, 10);
	if (err && *err) {
		uqmi_add_error("Invalid MCC value");
		return QMI_CMD_EXIT;
	}

	sel_req.data.network_selection_preference.mcc = value;
	plmn_code_flag.mcc_is_set = true;
	return QMI_CMD_DONE;
}

#define cmd_nas_set_mnc_cb no_cb
static enum qmi_cmd_result
cmd_nas_set_mnc_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	char *err;
	int value = strtoul(arg, &err, 10);
	if (err && *err) {
		uqmi_add_error("Invalid MNC value");
		return QMI_CMD_EXIT;
	}

	sel_req.data.network_selection_preference.mnc = value;
	plmn_code_flag.mnc_is_set = true;
	return QMI_CMD_DONE;
}

#define cmd_nas_set_plmn_cb no_cb
static enum qmi_cmd_result
cmd_nas_set_plmn_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	sel_req.set.network_selection_preference = 1;
	sel_req.data.network_selection_preference.mode = QMI_NAS_NETWORK_SELECTION_PREFERENCE_AUTOMATIC;

	if (!plmn_code_flag.mcc_is_set && plmn_code_flag.mnc_is_set) {
		uqmi_add_error("No MCC value");
		return QMI_CMD_EXIT;
	}

	if (plmn_code_flag.mcc_is_set && sel_req.data.network_selection_preference.mcc) {
		if (!plmn_code_flag.mnc_is_set) {
			uqmi_add_error("No MNC value");
			return QMI_CMD_EXIT;
		} else {
			sel_req.data.network_selection_preference.mode = QMI_NAS_NETWORK_SELECTION_PREFERENCE_MANUAL;
		}
	}

	return do_sel_network();
}

#define cmd_nas_initiate_network_register_cb no_cb
static enum qmi_cmd_result
cmd_nas_initiate_network_register_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	static struct qmi_nas_initiate_network_register_request register_req = {
		QMI_INIT(action, QMI_NAS_NETWORK_REGISTER_TYPE_AUTOMATIC)
	};

	qmi_set_nas_initiate_network_register_request(msg, &register_req);
	return QMI_CMD_REQUEST;
}

static void
cmd_nas_get_signal_info_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_nas_get_signal_info_response res;
	void *c;

	qmi_parse_nas_get_signal_info_response(msg, &res);

	c = blobmsg_open_table(&status, NULL);
	if (res.set.cdma_signal_strength) {
		blobmsg_add_string(&status, "type", "cdma");
		blobmsg_add_u32(&status, "rssi", (int32_t) res.data.cdma_signal_strength.rssi);
		blobmsg_add_u32(&status, "ecio", (int32_t) res.data.cdma_signal_strength.ecio);
	}

	if (res.set.hdr_signal_strength) {
		blobmsg_add_string(&status, "type", "hdr");
		blobmsg_add_u32(&status, "rssi", (int32_t) res.data.hdr_signal_strength.rssi);
		blobmsg_add_u32(&status, "ecio", (int32_t) res.data.hdr_signal_strength.ecio);
		blobmsg_add_u32(&status, "io", res.data.hdr_signal_strength.io);
	}

	if (res.set.gsm_signal_strength) {
		blobmsg_add_string(&status, "type", "gsm");
		blobmsg_add_u32(&status, "signal", (int32_t) res.data.gsm_signal_strength);
	}

	if (res.set.wcdma_signal_strength) {
		blobmsg_add_string(&status, "type", "wcdma");
		blobmsg_add_u32(&status, "rssi", (int32_t) res.data.wcdma_signal_strength.rssi);
		blobmsg_add_u32(&status, "ecio", (int32_t) res.data.wcdma_signal_strength.ecio);
	}

	if (res.set.lte_signal_strength) {
		blobmsg_add_string(&status, "type", "lte");
		blobmsg_add_u32(&status, "rssi", (int32_t) res.data.lte_signal_strength.rssi);
		blobmsg_add_u32(&status, "rsrq", (int32_t) res.data.lte_signal_strength.rsrq);
		blobmsg_add_u32(&status, "rsrp", (int32_t) res.data.lte_signal_strength.rsrp);
		blobmsg_add_u32(&status, "snr", (int32_t) res.data.lte_signal_strength.snr);
	}

	if (res.set.tdma_signal_strength) {
		blobmsg_add_string(&status, "type", "tdma");
		blobmsg_add_u32(&status, "signal", (int32_t) res.data.tdma_signal_strength);
	}

	blobmsg_close_table(&status, c);
}

static enum qmi_cmd_result
cmd_nas_get_signal_info_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_nas_get_signal_info_request(msg);
	return QMI_CMD_REQUEST;
}

static void
cmd_nas_get_serving_system_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_nas_get_serving_system_response res;
	static const char *reg_states[] = {
		[QMI_NAS_REGISTRATION_STATE_NOT_REGISTERED] = "not_registered",
		[QMI_NAS_REGISTRATION_STATE_REGISTERED] = "registered",
		[QMI_NAS_REGISTRATION_STATE_NOT_REGISTERED_SEARCHING] = "searching",
		[QMI_NAS_REGISTRATION_STATE_REGISTRATION_DENIED] = "registering_denied",
		[QMI_NAS_REGISTRATION_STATE_UNKNOWN] = "unknown",
	};
	void *c;

	qmi_parse_nas_get_serving_system_response(msg, &res);

	c = blobmsg_open_table(&status, NULL);
	if (res.set.serving_system) {
		int state = res.data.serving_system.registration_state;

		if (state > QMI_NAS_REGISTRATION_STATE_UNKNOWN)
			state = QMI_NAS_REGISTRATION_STATE_UNKNOWN;

		blobmsg_add_string(&status, "registration", reg_states[state]);
	}
	if (res.set.current_plmn) {
		blobmsg_add_u32(&status, "plmn_mcc", res.data.current_plmn.mcc);
		blobmsg_add_u32(&status, "plmn_mnc", res.data.current_plmn.mnc);
		if (res.data.current_plmn.description)
			blobmsg_add_string(&status, "plmn_description", res.data.current_plmn.description);
	}

	if (res.set.roaming_indicator)
		blobmsg_add_u8(&status, "roaming", !res.data.roaming_indicator);

	blobmsg_close_table(&status, c);
}

static enum qmi_cmd_result
cmd_nas_get_serving_system_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_nas_get_serving_system_request(msg);
	return QMI_CMD_REQUEST;
}

static void
cmd_nas_network_scan_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	static struct qmi_nas_network_scan_response res;
	const char *network_status[] = {
		"current_serving",
		"available",
		"home",
		"roaming",
		"forbidden",
		"not_forbidden",
		"preferred",
		"not_preferred",
	};
	const char *radio[] = {
		[QMI_NAS_RADIO_INTERFACE_NONE] = "none",
		[QMI_NAS_RADIO_INTERFACE_CDMA_1X] = "cdma-1x",
		[QMI_NAS_RADIO_INTERFACE_CDMA_1XEVDO] = "cdma-1x_evdo",
		[QMI_NAS_RADIO_INTERFACE_AMPS] = "amps",
		[QMI_NAS_RADIO_INTERFACE_GSM] = "gsm",
		[QMI_NAS_RADIO_INTERFACE_UMTS] = "umts",
		[QMI_NAS_RADIO_INTERFACE_LTE] = "lte",
		[QMI_NAS_RADIO_INTERFACE_TD_SCDMA] = "td-scdma",
	};
	void *t, *c, *info, *stat;
	int i, j;

	qmi_parse_nas_network_scan_response(msg, &res);

	t = blobmsg_open_table(&status, NULL);

	c = blobmsg_open_array(&status, "network_info");
	for (i = 0; i < res.data.network_information_n; i++) {
		info = blobmsg_open_table(&status, NULL);
		blobmsg_add_u32(&status, "mcc", res.data.network_information[i].mcc);
		blobmsg_add_u32(&status, "mnc", res.data.network_information[i].mnc);
		if (res.data.network_information[i].description)
			blobmsg_add_string(&status, "description", res.data.network_information[i].description);
		stat = blobmsg_open_array(&status, "status");
		for (j = 0; j < ARRAY_SIZE(network_status); j++) {
			if (!(res.data.network_information[i].network_status & (1 << j)))
				continue;

			blobmsg_add_string(&status, NULL, network_status[j]);
		}
		blobmsg_close_array(&status, stat);
		blobmsg_close_table(&status, info);
	}
	blobmsg_close_array(&status, c);

	c = blobmsg_open_array(&status, "radio_access_technology");
	for (i = 0; i < res.data.radio_access_technology_n; i++) {
		const char *r = "unknown";
		int r_i = res.data.radio_access_technology[i].radio_interface;

		info = blobmsg_open_table(&status, NULL);
		blobmsg_add_u32(&status, "mcc", res.data.radio_access_technology[i].mcc);
		blobmsg_add_u32(&status, "mnc", res.data.radio_access_technology[i].mnc);
		if (r_i >= 0 && r_i < ARRAY_SIZE(radio))
			r = radio[r_i];

		blobmsg_add_string(&status, "radio", r);
		blobmsg_close_table(&status, info);
	}
	blobmsg_close_array(&status, c);

	blobmsg_close_table(&status, t);
}

static enum qmi_cmd_result
cmd_nas_network_scan_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	struct qmi_nas_network_scan_request sreq = {
		QMI_INIT(network_type,
	             QMI_NAS_NETWORK_SCAN_TYPE_GSM |
	             QMI_NAS_NETWORK_SCAN_TYPE_UMTS |
	             QMI_NAS_NETWORK_SCAN_TYPE_LTE |
	             QMI_NAS_NETWORK_SCAN_TYPE_TD_SCDMA),
	};

	qmi_set_nas_network_scan_request(msg, &sreq);
	return QMI_CMD_REQUEST;
}

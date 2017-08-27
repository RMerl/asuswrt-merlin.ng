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

static struct {
	QmiDmsUimPinId pin_id;
	char* pin;
	char* new_pin;
	char* puk;
} dms_req_data;

static void cmd_dms_get_capabilities_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	void *t, *networks;
	int i;
	struct qmi_dms_get_capabilities_response res;
	const char *radio_cap[] = {
		[QMI_DMS_RADIO_INTERFACE_CDMA20001X] = "cdma1x",
		[QMI_DMS_RADIO_INTERFACE_EVDO] = "cdma1xevdo",
		[QMI_DMS_RADIO_INTERFACE_GSM] = "gsm",
		[QMI_DMS_RADIO_INTERFACE_UMTS] = "umts",
		[QMI_DMS_RADIO_INTERFACE_LTE] = "lte",
	};
	const char *service_cap[] = {
		[QMI_DMS_DATA_SERVICE_CAPABILITY_NONE] = "none",
		[QMI_DMS_DATA_SERVICE_CAPABILITY_CS] = "cs",
		[QMI_DMS_DATA_SERVICE_CAPABILITY_PS] = "ps",
		[QMI_DMS_DATA_SERVICE_CAPABILITY_SIMULTANEOUS_CS_PS] = "simultaneous_cs_ps",
		[QMI_DMS_DATA_SERVICE_CAPABILITY_NON_SIMULTANEOUS_CS_PS] = "non_simultaneous_cs_ps",
	};

	qmi_parse_dms_get_capabilities_response(msg, &res);

	t = blobmsg_open_table(&status, NULL);

	blobmsg_add_u32(&status, "max_tx_channel_rate", (int32_t) res.data.info.max_tx_channel_rate);
	blobmsg_add_u32(&status, "max_rx_channel_rate", (int32_t) res.data.info.max_rx_channel_rate);
	if ((int)res.data.info.data_service_capability >= 0 && res.data.info.data_service_capability < ARRAY_SIZE(service_cap))
		blobmsg_add_string(&status, "data_service", service_cap[res.data.info.data_service_capability]);

	if(res.data.info.sim_capability == QMI_DMS_SIM_CAPABILITY_NOT_SUPPORTED)
		blobmsg_add_string(&status, "sim", "not supported");
	else if(res.data.info.sim_capability == QMI_DMS_SIM_CAPABILITY_SUPPORTED)
		blobmsg_add_string(&status, "sim", "supported");

	networks = blobmsg_open_array(&status, "networks");
	for (i = 0; i < res.data.info.radio_interface_list_n; i++) {
		if ((int)res.data.info.radio_interface_list[i] >= 0 && res.data.info.radio_interface_list[i] < ARRAY_SIZE(radio_cap))
			blobmsg_add_string(&status, NULL, radio_cap[res.data.info.radio_interface_list[i]]);
		else
			blobmsg_add_string(&status, NULL, "unknown");
	}
	blobmsg_close_array(&status, networks);

	blobmsg_close_table(&status, t);
}

static enum qmi_cmd_result
cmd_dms_get_capabilities_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_dms_get_capabilities_request(msg);
	return QMI_CMD_REQUEST;
}

static const char *get_pin_status(int status)
{
	static const char *pin_status[] = {
		[QMI_DMS_UIM_PIN_STATUS_NOT_INITIALIZED] = "not_initialized",
		[QMI_DMS_UIM_PIN_STATUS_ENABLED_NOT_VERIFIED] = "not_verified",
		[QMI_DMS_UIM_PIN_STATUS_ENABLED_VERIFIED] = "verified",
		[QMI_DMS_UIM_PIN_STATUS_DISABLED] = "disabled",
		[QMI_DMS_UIM_PIN_STATUS_BLOCKED] = "blocked",
		[QMI_DMS_UIM_PIN_STATUS_PERMANENTLY_BLOCKED] = "permanently_blocked",
		[QMI_DMS_UIM_PIN_STATUS_UNBLOCKED] = "unblocked",
		[QMI_DMS_UIM_PIN_STATUS_CHANGED] = "changed",
	};
	const char *res = "Unknown";

	if (status < ARRAY_SIZE(pin_status) && pin_status[status])
		res = pin_status[status];

	return res;
}

static void cmd_dms_get_pin_status_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_dms_uim_get_pin_status_response res;
	void *c;

	qmi_parse_dms_uim_get_pin_status_response(msg, &res);
	c = blobmsg_open_table(&status, NULL);
	if (res.set.pin1_status) {
		blobmsg_add_string(&status, "pin1_status", get_pin_status(res.data.pin1_status.current_status));
		blobmsg_add_u32(&status, "pin1_verify_tries", (int32_t) res.data.pin1_status.verify_retries_left);
		blobmsg_add_u32(&status, "pin1_unblock_tries", (int32_t) res.data.pin1_status.unblock_retries_left);
	}
	if (res.set.pin2_status) {
		blobmsg_add_string(&status, "pin2_status", get_pin_status(res.data.pin2_status.current_status));
		blobmsg_add_u32(&status, "pin2_verify_tries", (int32_t) res.data.pin2_status.verify_retries_left);
		blobmsg_add_u32(&status, "pin2_unblock_tries", (int32_t) res.data.pin2_status.unblock_retries_left);
	}
	blobmsg_close_table(&status, c);
}

static enum qmi_cmd_result
cmd_dms_get_pin_status_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_dms_uim_get_pin_status_request(msg);
	return QMI_CMD_REQUEST;
}

#define cmd_dms_verify_pin1_cb no_cb
static enum qmi_cmd_result
cmd_dms_verify_pin1_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	struct qmi_dms_uim_verify_pin_request data = {
		QMI_INIT_SEQUENCE(info,
			.pin_id = QMI_DMS_UIM_PIN_ID_PIN,
			.pin = arg
		)
	};
	qmi_set_dms_uim_verify_pin_request(msg, &data);
	return QMI_CMD_REQUEST;
}

#define cmd_dms_verify_pin2_cb no_cb
static enum qmi_cmd_result
cmd_dms_verify_pin2_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	struct qmi_dms_uim_verify_pin_request data = {
		QMI_INIT_SEQUENCE(info,
			.pin_id = QMI_DMS_UIM_PIN_ID_PIN2,
			.pin = arg
		)
	};
	qmi_set_dms_uim_verify_pin_request(msg, &data);
	return QMI_CMD_REQUEST;
}

#define cmd_dms_set_pin_cb no_cb
static enum qmi_cmd_result
cmd_dms_set_pin_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	dms_req_data.pin = arg;
	return QMI_CMD_DONE;
}

static enum qmi_cmd_result
cmd_dms_set_pin_protection_prepare(struct qmi_msg *msg, char *arg)
{
	if (!dms_req_data.pin) {
		uqmi_add_error("Missing argument");
		return QMI_CMD_EXIT;
	}

	int is_enabled;
	if (strcasecmp(arg, "disabled") == 0)
		is_enabled = false;
	else if (strcasecmp(arg, "enabled") == 0)
		is_enabled = true;
	else {
		uqmi_add_error("Invalid value (valid: disabled, enabled)");
		return QMI_CMD_EXIT;
	}

	struct qmi_dms_uim_set_pin_protection_request dms_pin_protection_req = {
		QMI_INIT_SEQUENCE(info,
			.pin_id = dms_req_data.pin_id
		),
		QMI_INIT_PTR(info.pin, dms_req_data.pin),
		QMI_INIT_PTR(info.protection_enabled, is_enabled)
	};

	qmi_set_dms_uim_set_pin_protection_request(msg, &dms_pin_protection_req);
	return QMI_CMD_REQUEST;
}

#define cmd_dms_set_pin1_protection_cb no_cb
static enum qmi_cmd_result
cmd_dms_set_pin1_protection_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	dms_req_data.pin_id = QMI_DMS_UIM_PIN_ID_PIN;
	return cmd_dms_set_pin_protection_prepare(msg, arg);
}

#define cmd_dms_set_pin2_protection_cb no_cb
static enum qmi_cmd_result
cmd_dms_set_pin2_protection_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	dms_req_data.pin_id = QMI_DMS_UIM_PIN_ID_PIN2;
	return cmd_dms_set_pin_protection_prepare(msg, arg);
}

static enum qmi_cmd_result
cmd_dms_change_pin_prepare(struct qmi_msg *msg, char *arg)
{
	if (!dms_req_data.pin || !dms_req_data.new_pin) {
		uqmi_add_error("Missing argument");
		return QMI_CMD_EXIT;
	}

	struct qmi_dms_uim_change_pin_request dms_change_pin_req = {
		QMI_INIT_SEQUENCE(info,
			.pin_id = dms_req_data.pin_id
		),
		QMI_INIT_PTR(info.old_pin, dms_req_data.pin),
		QMI_INIT_PTR(info.new_pin, dms_req_data.new_pin)
	};

	qmi_set_dms_uim_change_pin_request(msg, &dms_change_pin_req);
	return QMI_CMD_REQUEST;
}

#define cmd_dms_change_pin1_cb no_cb
static enum qmi_cmd_result
cmd_dms_change_pin1_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	dms_req_data.pin_id = QMI_DMS_UIM_PIN_ID_PIN;
	return cmd_dms_change_pin_prepare(msg, arg);
}

#define cmd_dms_change_pin2_cb no_cb
static enum qmi_cmd_result
cmd_dms_change_pin2_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	dms_req_data.pin_id = QMI_DMS_UIM_PIN_ID_PIN2;
	return cmd_dms_change_pin_prepare(msg, arg);
}

#define cmd_dms_set_new_pin_cb no_cb
static enum qmi_cmd_result
cmd_dms_set_new_pin_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	dms_req_data.new_pin = arg;
	return QMI_CMD_DONE;
}

#define cmd_dms_set_puk_cb no_cb
static enum qmi_cmd_result
cmd_dms_set_puk_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	dms_req_data.puk = arg;
	return QMI_CMD_DONE;
}

#define cmd_dms_unblock_pin1_cb no_cb
static enum qmi_cmd_result
cmd_dms_unblock_pin1_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	if (!dms_req_data.puk || !dms_req_data.new_pin) {
		uqmi_add_error("Missing argument");
		return QMI_CMD_EXIT;
	}

	struct qmi_dms_uim_unblock_pin_request dms_unlock_pin_req = {
		QMI_INIT_SEQUENCE(info,
				.pin_id = QMI_DMS_UIM_PIN_ID_PIN
			),
		QMI_INIT_PTR(info.puk, dms_req_data.puk),
		QMI_INIT_PTR(info.new_pin, dms_req_data.new_pin)
		};

	qmi_set_dms_uim_unblock_pin_request(msg, &dms_unlock_pin_req);
	return QMI_CMD_REQUEST;
}

#define cmd_dms_unblock_pin2_cb no_cb
static enum qmi_cmd_result
cmd_dms_unblock_pin2_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	if (!dms_req_data.puk || !dms_req_data.new_pin) {
		uqmi_add_error("Missing argument");
		return QMI_CMD_EXIT;
	}

	struct qmi_dms_uim_unblock_pin_request dms_unlock_pin_req = {
		QMI_INIT_SEQUENCE(info,
			.pin_id = QMI_DMS_UIM_PIN_ID_PIN2
		),
		QMI_INIT_PTR(info.puk, dms_req_data.puk),
		QMI_INIT_PTR(info.new_pin, dms_req_data.new_pin)
	};

	qmi_set_dms_uim_unblock_pin_request(msg, &dms_unlock_pin_req);
	return QMI_CMD_REQUEST;
}

static void cmd_dms_get_iccid_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_dms_uim_get_iccid_response res;

	qmi_parse_dms_uim_get_iccid_response(msg, &res);
	if (res.data.iccid)
		blobmsg_add_string(&status, NULL, res.data.iccid);
}

static enum qmi_cmd_result
cmd_dms_get_iccid_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_dms_uim_get_iccid_request(msg);
	return QMI_CMD_REQUEST;
}

static void cmd_dms_get_imsi_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_dms_uim_get_imsi_response res;

	qmi_parse_dms_uim_get_imsi_response(msg, &res);
	if (res.data.imsi)
		blobmsg_add_string(&status, NULL, res.data.imsi);
}

static enum qmi_cmd_result
cmd_dms_get_imsi_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_dms_uim_get_imsi_request(msg);
	return QMI_CMD_REQUEST;
}

static void cmd_dms_get_msisdn_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_dms_get_msisdn_response res;

	qmi_parse_dms_get_msisdn_response(msg, &res);
	if (res.data.msisdn)
		blobmsg_add_string(&status, NULL, res.data.msisdn);
}

static enum qmi_cmd_result
cmd_dms_get_msisdn_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_dms_get_msisdn_request(msg);
	return QMI_CMD_REQUEST;
}

static void cmd_dms_get_imei_cb(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg)
{
	struct qmi_dms_get_ids_response res;

	qmi_parse_dms_get_ids_response(msg, &res);
	if (res.data.imei)
		blobmsg_add_string(&status, NULL, res.data.imei);
}

static enum qmi_cmd_result
cmd_dms_get_imei_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_dms_get_ids_request(msg);
	return QMI_CMD_REQUEST;
}

#define cmd_dms_reset_cb no_cb
static enum qmi_cmd_result
cmd_dms_reset_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_dms_reset_request(msg);
	return QMI_CMD_REQUEST;
}

#define cmd_dms_set_operating_mode_cb no_cb
static enum qmi_cmd_result
cmd_dms_set_operating_mode_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	static const char *modes[] = {
		[QMI_DMS_OPERATING_MODE_ONLINE] = "online",
		[QMI_DMS_OPERATING_MODE_LOW_POWER] = "low_power",
		[QMI_DMS_OPERATING_MODE_FACTORY_TEST] = "factory_test",
		[QMI_DMS_OPERATING_MODE_OFFLINE] = "offline",
		[QMI_DMS_OPERATING_MODE_RESET] = "reset",
		[QMI_DMS_OPERATING_MODE_SHUTTING_DOWN] = "shutting_down",
		[QMI_DMS_OPERATING_MODE_PERSISTENT_LOW_POWER] = "persistent_low_power",
		[QMI_DMS_OPERATING_MODE_MODE_ONLY_LOW_POWER] = "mode_only_low_power",
	};
	static struct qmi_dms_set_operating_mode_request sreq = {
		QMI_INIT(mode, QMI_DMS_OPERATING_MODE_ONLINE),
	};
	int i;

	for (i = 0; i < ARRAY_SIZE(modes); i++) {
		if (!modes[i])
			continue;

		if (strcmp(arg, modes[i]) != 0)
			continue;

		sreq.data.mode = i;
		qmi_set_dms_set_operating_mode_request(msg, &sreq);
		return QMI_CMD_REQUEST;
	}

	return uqmi_add_error("Invalid argument");
}

#define cmd_dms_set_fcc_authentication_cb no_cb
static enum qmi_cmd_result
cmd_dms_set_fcc_authentication_prepare(struct qmi_dev *qmi, struct qmi_request *req, struct qmi_msg *msg, char *arg)
{
	qmi_set_dms_set_fcc_authentication_request(msg);
	return QMI_CMD_REQUEST;
}

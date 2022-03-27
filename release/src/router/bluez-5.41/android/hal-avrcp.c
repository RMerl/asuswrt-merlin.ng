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

#include "hal-utils.h"
#include "hal-log.h"
#include "hal.h"
#include "hal-msg.h"
#include "ipc-common.h"
#include "hal-ipc.h"

static const btrc_callbacks_t *cbs = NULL;

static bool interface_ready(void)
{
	return cbs != NULL;
}

static void handle_remote_features(void *buf, uint16_t len, int fd)
{
	struct hal_ev_avrcp_remote_features *ev = buf;

	if (cbs->remote_features_cb)
		cbs->remote_features_cb((bt_bdaddr_t *) (ev->bdaddr),
								ev->features);
}

static void handle_get_play_status(void *buf, uint16_t len, int fd)
{
	if (cbs->get_play_status_cb)
		cbs->get_play_status_cb();
}

static void handle_list_player_attrs(void *buf, uint16_t len, int fd)
{
	if (cbs->list_player_app_attr_cb)
		cbs->list_player_app_attr_cb();
}

static void handle_list_player_values(void *buf, uint16_t len, int fd)
{
	struct hal_ev_avrcp_list_player_values *ev = buf;

	if (cbs->list_player_app_values_cb)
		cbs->list_player_app_values_cb(ev->attr);
}

static void handle_get_player_values(void *buf, uint16_t len, int fd)
{
	struct hal_ev_avrcp_get_player_values *ev = buf;
	btrc_player_attr_t attrs[4];
	int i;

	if (!cbs->get_player_app_value_cb)
		return;

	/* Convert uint8_t array to btrc_player_attr_t array */
	for (i = 0; i < ev->number; i++)
		attrs[i] = ev->attrs[i];

	cbs->get_player_app_value_cb(ev->number, attrs);
}

static void handle_get_player_attrs_text(void *buf, uint16_t len, int fd)
{
	struct hal_ev_avrcp_get_player_attrs_text *ev = buf;
	btrc_player_attr_t attrs[4];
	int i;

	if (!cbs->get_player_app_attrs_text_cb)
		return;

	/* Convert uint8_t array to btrc_player_attr_t array */
	for (i = 0; i < ev->number; i++)
		attrs[i] = ev->attrs[i];

	cbs->get_player_app_attrs_text_cb(ev->number, attrs);
}

static void handle_get_player_values_text(void *buf, uint16_t len, int fd)
{
	struct hal_ev_avrcp_get_player_values_text *ev = buf;

	if (cbs->get_player_app_values_text_cb)
		cbs->get_player_app_values_text_cb(ev->attr, ev->number,
								ev->values);
}

static void handle_set_player_value(void *buf, uint16_t len, int fd)
{
	struct hal_ev_avrcp_set_player_values *ev = buf;
	struct hal_avrcp_player_attr_value *attrs;
	btrc_player_settings_t values;
	int i;

	if (!cbs->set_player_app_value_cb)
		return;

	attrs = (struct hal_avrcp_player_attr_value *) ev->attrs;

	/* Convert to btrc_player_settings_t */
	values.num_attr = ev->number;
	for (i = 0; i < ev->number; i++) {
		values.attr_ids[i] = attrs[i].attr;
		values.attr_values[i] = attrs[i].value;
	}

	cbs->set_player_app_value_cb(&values);
}

static void handle_get_element_attrs(void *buf, uint16_t len, int fd)
{
	struct hal_ev_avrcp_get_element_attrs *ev = buf;
	btrc_media_attr_t attrs[BTRC_MAX_APP_SETTINGS];
	int i;

	if (!cbs->get_element_attr_cb)
		return;

	/* Convert uint8_t array to btrc_media_attr_t array */
	for (i = 0; i < ev->number; i++)
		attrs[i] = ev->attrs[i];

	cbs->get_element_attr_cb(ev->number, attrs);
}

static void handle_register_notification(void *buf, uint16_t len, int fd)
{
	struct hal_ev_avrcp_register_notification *ev = buf;

	if (cbs->register_notification_cb)
		cbs->register_notification_cb(ev->event, ev->param);
}

static void handle_volume_changed(void *buf, uint16_t len, int fd)
{
	struct hal_ev_avrcp_volume_changed *ev = buf;

	if (cbs->volume_change_cb)
		cbs->volume_change_cb(ev->volume, ev->type);
}

static void handle_passthrough_cmd(void *buf, uint16_t len, int fd)
{
	struct hal_ev_avrcp_passthrough_cmd *ev = buf;

	if (cbs->passthrough_cmd_cb)
		cbs->passthrough_cmd_cb(ev->id, ev->state);
}

/*
 * handlers will be called from notification thread context,
 * index in table equals to 'opcode - HAL_MINIMUM_EVENT'
 */
static const struct hal_ipc_handler ev_handlers[] = {
	/* HAL_EV_AVRCP_REMOTE_FEATURES */
	{ handle_remote_features, false,
			sizeof(struct hal_ev_avrcp_remote_features) },
	/* HAL_EV_AVRCP_GET_PLAY_STATUS */
	{ handle_get_play_status, false, 0 },
	/* HAL_EV_AVRCP_LIST_PLAYER_ATTRS */
	{ handle_list_player_attrs, false, 0 },
	/* HAL_EV_AVRCP_LIST_PLAYER_VALUES */
	{ handle_list_player_values, false,
			sizeof(struct hal_ev_avrcp_list_player_values) },
	/* HAL_EV_AVRCP_GET_PLAYER_VALUES */
	{ handle_get_player_values, true,
			sizeof(struct hal_ev_avrcp_get_player_values) },
	/* HAL_EV_AVRCP_GET_PLAYER_ATTRS_TEXT */
	{ handle_get_player_attrs_text, true,
			sizeof(struct hal_ev_avrcp_get_player_attrs_text) },
	/* HAL_EV_AVRCP_GET_PLAYER_VALUES_TEXT */
	{ handle_get_player_values_text, true,
			sizeof(struct hal_ev_avrcp_get_player_values_text) },
	/* HAL_EV_AVRCP_SET_PLAYER_VALUES */
	{ handle_set_player_value, true,
			sizeof(struct hal_ev_avrcp_set_player_values) },
	/* HAL_EV_AVRCP_GET_ELEMENT_ATTRS */
	{ handle_get_element_attrs, true,
			sizeof(struct hal_ev_avrcp_get_element_attrs) },
	/* HAL_EV_AVRCP_REGISTER_NOTIFICATION */
	{ handle_register_notification, false,
			sizeof(struct hal_ev_avrcp_register_notification) },
	/* HAL_EV_AVRCP_VOLUME_CHANGED */
	{ handle_volume_changed, false,
			sizeof(struct hal_ev_avrcp_volume_changed) },
	/* HAL_EV_AVRCP_PASSTHROUGH_CMD */
	{ handle_passthrough_cmd, false,
			sizeof(struct hal_ev_avrcp_passthrough_cmd) },
};

static bt_status_t init(btrc_callbacks_t *callbacks)
{
	struct hal_cmd_register_module cmd;
	int ret;

	DBG("");

	if (interface_ready())
		return BT_STATUS_DONE;

	cbs = callbacks;

	hal_ipc_register(HAL_SERVICE_ID_AVRCP, ev_handlers,
				sizeof(ev_handlers) / sizeof(ev_handlers[0]));

	cmd.service_id = HAL_SERVICE_ID_AVRCP;
	cmd.mode = HAL_MODE_DEFAULT;
	cmd.max_clients = 1;

	ret = hal_ipc_cmd(HAL_SERVICE_ID_CORE, HAL_OP_REGISTER_MODULE,
					sizeof(cmd), &cmd, NULL, NULL, NULL);

	if (ret != BT_STATUS_SUCCESS) {
		cbs = NULL;
		hal_ipc_unregister(HAL_SERVICE_ID_AVRCP);
	}

	return ret;
}

static bt_status_t get_play_status_rsp(btrc_play_status_t status,
					uint32_t song_len, uint32_t song_pos)
{
	struct hal_cmd_avrcp_get_play_status cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	cmd.status = status;
	cmd.duration = song_len;
	cmd.position = song_pos;

	return hal_ipc_cmd(HAL_SERVICE_ID_AVRCP, HAL_OP_AVRCP_GET_PLAY_STATUS,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static bt_status_t list_player_app_attr_rsp(int num_attr,
						btrc_player_attr_t *p_attrs)
{
	char buf[IPC_MTU];
	struct hal_cmd_avrcp_list_player_attrs *cmd = (void *) buf;
	size_t len;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (num_attr < 0)
		return BT_STATUS_PARM_INVALID;

	len = sizeof(*cmd) + num_attr;
	if (len > IPC_MTU)
		return BT_STATUS_PARM_INVALID;

	cmd->number = num_attr;
	memcpy(cmd->attrs, p_attrs, num_attr);

	return hal_ipc_cmd(HAL_SERVICE_ID_AVRCP,
					HAL_OP_AVRCP_LIST_PLAYER_ATTRS,
					len, cmd, NULL, NULL, NULL);
}

static bt_status_t list_player_app_value_rsp(int num_val, uint8_t *p_vals)
{
	char buf[IPC_MTU];
	struct hal_cmd_avrcp_list_player_values *cmd = (void *) buf;
	size_t len;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (num_val < 0)
		return BT_STATUS_PARM_INVALID;

	len = sizeof(*cmd) + num_val;

	if (len > IPC_MTU)
		return BT_STATUS_PARM_INVALID;

	cmd->number = num_val;
	memcpy(cmd->values, p_vals, num_val);

	return hal_ipc_cmd(HAL_SERVICE_ID_AVRCP,
					HAL_OP_AVRCP_LIST_PLAYER_VALUES,
					len, cmd, NULL, NULL, NULL);
}

static bt_status_t get_player_app_value_rsp(btrc_player_settings_t *p_vals)
{
	char buf[IPC_MTU];
	struct hal_cmd_avrcp_get_player_attrs *cmd = (void *) buf;
	size_t len, attrs_len;
	int i;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (!p_vals)
		return BT_STATUS_PARM_INVALID;

	attrs_len = p_vals->num_attr *
				sizeof(struct hal_avrcp_player_attr_value);
	len = sizeof(*cmd) + attrs_len;

	if (len > IPC_MTU)
		return BT_STATUS_PARM_INVALID;

	cmd->number = p_vals->num_attr;

	for (i = 0; i < p_vals->num_attr; i++) {
		cmd->attrs[i].attr = p_vals->attr_ids[i];
		cmd->attrs[i].value = p_vals->attr_values[i];
	}

	return hal_ipc_cmd(HAL_SERVICE_ID_AVRCP,
					HAL_OP_AVRCP_GET_PLAYER_ATTRS,
					len, cmd, NULL, NULL, NULL);
}

static int write_text(uint8_t *ptr, uint8_t id, uint8_t *text, size_t *len)
{
	struct hal_avrcp_player_setting_text *value = (void *) ptr;
	size_t attr_len = sizeof(*value);

	if (attr_len + *len > IPC_MTU)
		return 0;

	value->id = id;
	value->len = strnlen((const char *) text, BTRC_MAX_ATTR_STR_LEN);

	*len += attr_len;

	if (value->len + *len > IPC_MTU)
		value->len = IPC_MTU - *len;

	memcpy(value->text, text, value->len);

	*len += value->len;

	return attr_len + value->len;
}

static uint8_t write_player_setting_text(uint8_t *ptr, uint8_t num_attr,
					btrc_player_setting_text_t *p_attrs,
					size_t *len)
{
	int i;

	for (i = 0; i < num_attr && *len < IPC_MTU; i++) {
		int ret;

		ret = write_text(ptr, p_attrs[i].id, p_attrs[i].text, len);
		if (ret == 0)
			break;

		ptr += ret;
	}

	return i;
}

static bt_status_t get_player_app_attr_text_rsp(int num_attr,
					btrc_player_setting_text_t *p_attrs)
{
	char buf[IPC_MTU];
	struct hal_cmd_avrcp_get_player_attrs_text *cmd = (void *) buf;
	uint8_t *ptr;
	size_t len;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (num_attr < 0 || num_attr > BTRC_MAX_APP_SETTINGS)
		return BT_STATUS_PARM_INVALID;

	len = sizeof(*cmd);
	ptr = (uint8_t *) &cmd->attrs[0];
	cmd->number = write_player_setting_text(ptr, num_attr, p_attrs, &len);

	return hal_ipc_cmd(HAL_SERVICE_ID_AVRCP,
					HAL_OP_AVRCP_GET_PLAYER_ATTRS_TEXT,
					len, cmd, NULL, NULL, NULL);
}

static bt_status_t get_player_app_value_text_rsp(int num_val,
					btrc_player_setting_text_t *p_vals)
{
	char buf[IPC_MTU];
	struct hal_cmd_avrcp_get_player_values_text *cmd = (void *) buf;
	uint8_t *ptr;
	size_t len;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	if (num_val < 0)
		return BT_STATUS_PARM_INVALID;

	len = sizeof(*cmd);
	ptr = (uint8_t *) &cmd->values[0];
	cmd->number = write_player_setting_text(ptr, num_val, p_vals, &len);

	return hal_ipc_cmd(HAL_SERVICE_ID_AVRCP,
					HAL_OP_AVRCP_GET_PLAYER_VALUES_TEXT,
					len, cmd, NULL, NULL, NULL);
}

static uint8_t write_element_attr_text(uint8_t *ptr, uint8_t num_attr,
					btrc_element_attr_val_t *p_attrs,
					size_t *len)
{
	int i;

	for (i = 0; i < num_attr && *len < IPC_MTU; i++) {
		int ret;

		ret = write_text(ptr, p_attrs[i].attr_id, p_attrs[i].text, len);
		if (ret == 0)
			break;

		ptr += ret;
	}

	return i;
}

static bt_status_t get_element_attr_rsp(uint8_t num_attr,
					btrc_element_attr_val_t *p_attrs)
{
	char buf[IPC_MTU];
	struct hal_cmd_avrcp_get_element_attrs_text *cmd = (void *) buf;
	size_t len;
	uint8_t *ptr;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	len = sizeof(*cmd);
	ptr = (uint8_t *) &cmd->values[0];
	cmd->number = write_element_attr_text(ptr, num_attr, p_attrs, &len);

	return hal_ipc_cmd(HAL_SERVICE_ID_AVRCP,
					HAL_OP_AVRCP_GET_ELEMENT_ATTRS_TEXT,
					len, cmd, NULL, NULL, NULL);
}

static bt_status_t set_player_app_value_rsp(btrc_status_t rsp_status)
{
	struct hal_cmd_avrcp_set_player_attrs_value cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	cmd.status = rsp_status;

	return hal_ipc_cmd(HAL_SERVICE_ID_AVRCP,
					HAL_OP_AVRCP_SET_PLAYER_ATTRS_VALUE,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static bt_status_t play_status_changed_rsp(btrc_notification_type_t type,
						btrc_play_status_t *play_status)
{
	char buf[IPC_MTU];
	struct hal_cmd_avrcp_register_notification *cmd = (void *) buf;
	size_t len;

	cmd->event = BTRC_EVT_PLAY_STATUS_CHANGED;
	cmd->type = type;
	cmd->len = 1;
	memcpy(cmd->data, play_status, cmd->len);

	len = sizeof(*cmd) + cmd->len;

	return hal_ipc_cmd(HAL_SERVICE_ID_AVRCP,
					HAL_OP_AVRCP_REGISTER_NOTIFICATION,
					len, cmd, NULL, NULL, NULL);
}

static bt_status_t track_change_rsp(btrc_notification_type_t type,
							btrc_uid_t *track)
{
	char buf[IPC_MTU];
	struct hal_cmd_avrcp_register_notification *cmd = (void *) buf;
	size_t len;

	cmd->event = BTRC_EVT_TRACK_CHANGE;
	cmd->type = type;
	cmd->len = sizeof(*track);
	memcpy(cmd->data, track, cmd->len);

	len = sizeof(*cmd) + cmd->len;

	return hal_ipc_cmd(HAL_SERVICE_ID_AVRCP,
					HAL_OP_AVRCP_REGISTER_NOTIFICATION,
					len, cmd, NULL, NULL, NULL);
}

static bt_status_t track_reached_end_rsp(btrc_notification_type_t type)
{
	struct hal_cmd_avrcp_register_notification cmd;

	cmd.event = BTRC_EVT_TRACK_REACHED_END;
	cmd.type = type;
	cmd.len = 0;

	return hal_ipc_cmd(HAL_SERVICE_ID_AVRCP,
					HAL_OP_AVRCP_REGISTER_NOTIFICATION,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static bt_status_t track_reached_start_rsp(btrc_notification_type_t type)
{
	struct hal_cmd_avrcp_register_notification cmd;

	cmd.event = BTRC_EVT_TRACK_REACHED_START;
	cmd.type = type;
	cmd.len = 0;

	return hal_ipc_cmd(HAL_SERVICE_ID_AVRCP,
					HAL_OP_AVRCP_REGISTER_NOTIFICATION,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static bt_status_t play_pos_changed_rsp(btrc_notification_type_t type,
							uint32_t *song_pos)
{
	char buf[IPC_MTU];
	struct hal_cmd_avrcp_register_notification *cmd = (void *) buf;
	size_t len;

	cmd->event = BTRC_EVT_PLAY_POS_CHANGED;
	cmd->type = type;
	cmd->len = sizeof(*song_pos);
	memcpy(cmd->data, song_pos, cmd->len);

	len = sizeof(*cmd) + cmd->len;

	return hal_ipc_cmd(HAL_SERVICE_ID_AVRCP,
					HAL_OP_AVRCP_REGISTER_NOTIFICATION,
					len, cmd, NULL, NULL, NULL);
}

static bt_status_t settings_changed_rsp(btrc_notification_type_t type,
					btrc_player_settings_t *player_setting)
{
	char buf[IPC_MTU];
	struct hal_cmd_avrcp_register_notification *cmd = (void *) buf;
	struct hal_avrcp_player_attr_value *attrs;
	size_t len, param_len;
	int i;

	param_len = player_setting->num_attr * sizeof(*attrs);
	len = sizeof(*cmd) + param_len;

	if (len > IPC_MTU)
		return BT_STATUS_PARM_INVALID;

	cmd->event = BTRC_EVT_APP_SETTINGS_CHANGED;
	cmd->type = type;
	cmd->len = param_len;

	attrs = (struct hal_avrcp_player_attr_value *) &cmd->data[0];
	for (i = 0; i < player_setting->num_attr; i++) {
		attrs[i].attr = player_setting->attr_ids[i];
		attrs[i].value = player_setting->attr_values[i];
	}

	return hal_ipc_cmd(HAL_SERVICE_ID_AVRCP,
					HAL_OP_AVRCP_REGISTER_NOTIFICATION,
					len, cmd, NULL, NULL, NULL);
}

static bt_status_t register_notification_rsp(btrc_event_id_t event_id,
					btrc_notification_type_t type,
					btrc_register_notification_t *p_param)
{
	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	switch (event_id) {
	case BTRC_EVT_PLAY_STATUS_CHANGED:
		return play_status_changed_rsp(type, &p_param->play_status);
	case BTRC_EVT_TRACK_CHANGE:
		return track_change_rsp(type, &p_param->track);
	case BTRC_EVT_TRACK_REACHED_END:
		return track_reached_end_rsp(type);
	case BTRC_EVT_TRACK_REACHED_START:
		return track_reached_start_rsp(type);
	case BTRC_EVT_PLAY_POS_CHANGED:
		return play_pos_changed_rsp(type, &p_param->song_pos);
	case BTRC_EVT_APP_SETTINGS_CHANGED:
		return settings_changed_rsp(type, &p_param->player_setting);
	default:
		return BT_STATUS_PARM_INVALID;
	}
}

static bt_status_t set_volume(uint8_t volume)
{
	struct hal_cmd_avrcp_set_volume cmd;

	DBG("");

	if (!interface_ready())
		return BT_STATUS_NOT_READY;

	cmd.value = volume;

	return hal_ipc_cmd(HAL_SERVICE_ID_AVRCP, HAL_OP_AVRCP_SET_VOLUME,
					sizeof(cmd), &cmd, NULL, NULL, NULL);
}

static void cleanup(void)
{
	struct hal_cmd_unregister_module cmd;

	DBG("");

	if (!interface_ready())
		return;

	cmd.service_id = HAL_SERVICE_ID_AVRCP;

	hal_ipc_cmd(HAL_SERVICE_ID_CORE, HAL_OP_UNREGISTER_MODULE,
					sizeof(cmd), &cmd, NULL, NULL, NULL);

	hal_ipc_unregister(HAL_SERVICE_ID_AVRCP);

	cbs = NULL;
}

static btrc_interface_t iface = {
	.size = sizeof(iface),
	.init = init,
	.get_play_status_rsp = get_play_status_rsp,
	.list_player_app_attr_rsp = list_player_app_attr_rsp,
	.list_player_app_value_rsp = list_player_app_value_rsp,
	.get_player_app_value_rsp = get_player_app_value_rsp,
	.get_player_app_attr_text_rsp = get_player_app_attr_text_rsp,
	.get_player_app_value_text_rsp = get_player_app_value_text_rsp,
	.get_element_attr_rsp = get_element_attr_rsp,
	.set_player_app_value_rsp = set_player_app_value_rsp,
	.register_notification_rsp = register_notification_rsp,
	.set_volume = set_volume,
	.cleanup = cleanup
};

btrc_interface_t *bt_get_avrcp_interface(void)
{
	return &iface;
}

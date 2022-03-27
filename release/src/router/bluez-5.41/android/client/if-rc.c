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

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <hardware/bluetooth.h>
#include <hardware/bt_hh.h>

#include "if-main.h"
#include "pollhandler.h"
#include "../hal-utils.h"

const btrc_interface_t *if_rc = NULL;

SINTMAP(btrc_play_status_t, -1, "(unknown)")
	DELEMENT(BTRC_PLAYSTATE_STOPPED),
	DELEMENT(BTRC_PLAYSTATE_PLAYING),
	DELEMENT(BTRC_PLAYSTATE_PAUSED),
	DELEMENT(BTRC_PLAYSTATE_FWD_SEEK),
	DELEMENT(BTRC_PLAYSTATE_REV_SEEK),
	DELEMENT(BTRC_PLAYSTATE_ERROR),
ENDMAP

SINTMAP(btrc_media_attr_t, -1, "(unknown)")
	DELEMENT(BTRC_MEDIA_ATTR_TITLE),
	DELEMENT(BTRC_MEDIA_ATTR_ARTIST),
	DELEMENT(BTRC_MEDIA_ATTR_ALBUM),
	DELEMENT(BTRC_MEDIA_ATTR_TRACK_NUM),
	DELEMENT(BTRC_MEDIA_ATTR_NUM_TRACKS),
	DELEMENT(BTRC_MEDIA_ATTR_GENRE),
	DELEMENT(BTRC_MEDIA_ATTR_PLAYING_TIME),
ENDMAP

SINTMAP(btrc_status_t, -1, "(unknown)")
	DELEMENT(BTRC_STS_BAD_CMD),
	DELEMENT(BTRC_STS_BAD_PARAM),
	DELEMENT(BTRC_STS_NOT_FOUND),
	DELEMENT(BTRC_STS_INTERNAL_ERR),
	DELEMENT(BTRC_STS_NO_ERROR),
ENDMAP

SINTMAP(btrc_event_id_t, -1, "(unknown)")
	DELEMENT(BTRC_EVT_PLAY_STATUS_CHANGED),
	DELEMENT(BTRC_EVT_TRACK_CHANGE),
	DELEMENT(BTRC_EVT_TRACK_REACHED_END),
	DELEMENT(BTRC_EVT_TRACK_REACHED_START),
	DELEMENT(BTRC_EVT_PLAY_POS_CHANGED),
	DELEMENT(BTRC_EVT_APP_SETTINGS_CHANGED),
ENDMAP

SINTMAP(btrc_notification_type_t, -1, "(unknown)")
	DELEMENT(BTRC_NOTIFICATION_TYPE_INTERIM),
	DELEMENT(BTRC_NOTIFICATION_TYPE_CHANGED),
ENDMAP

static char last_addr[MAX_ADDR_STR_LEN];

static void remote_features_cb(bt_bdaddr_t *bd_addr,
					btrc_remote_features_t features)
{
	haltest_info("%s: remote_bd_addr=%s features=%u\n", __func__,
				bt_bdaddr_t2str(bd_addr, last_addr), features);
}

static void get_play_status_cb(void)
{
	haltest_info("%s\n", __func__);
}

static void list_player_app_attr_cb(void)
{
	haltest_info("%s\n", __func__);
}

static void list_player_app_values_cb(btrc_player_attr_t attr_id)
{
	haltest_info("%s, attr_id=%d\n", __func__, attr_id);
}

static void get_player_app_value_cb(uint8_t num_attr,
						btrc_player_attr_t *p_attrs)
{
	int i;

	haltest_info("%s, num_attr=%d\n", __func__, num_attr);

	for (i = 0; i < num_attr; i++)
		haltest_info("attribute=%u\n", p_attrs[i]);
}

static void get_player_app_attrs_text_cb(uint8_t num_attr,
						btrc_player_attr_t *p_attrs)
{
	int i;

	haltest_info("%s, num_attr=%d\n", __func__, num_attr);

	for (i = 0; i < num_attr; i++)
		haltest_info("attribute=%u\n", p_attrs[i]);

}

static void get_player_app_values_text_cb(uint8_t attr_id, uint8_t num_val,
								uint8_t *p_vals)
{
	haltest_info("%s, attr_id=%d num_val=%d values=%p\n", __func__,
						attr_id, num_val, p_vals);
}

static void set_player_app_value_cb(btrc_player_settings_t *p_vals)
{
	int i;

	haltest_info("%s, num_attr=%u\n", __func__, p_vals->num_attr);

	for (i = 0; i < p_vals->num_attr; i++)
		haltest_info("attr id=%u, values=%u\n", p_vals->attr_ids[i],
							p_vals->attr_values[i]);
}

static void get_element_attr_cb(uint8_t num_attr, btrc_media_attr_t *attrs)
{
	uint8_t i;

	haltest_info("%s, num_of_attributes=%d\n", __func__, num_attr);

	for (i = 0; i < num_attr; i++)
		haltest_info("attr id=%s\n", btrc_media_attr_t2str(attrs[i]));
}

static void register_notification_cb(btrc_event_id_t event_id, uint32_t param)
{
	haltest_info("%s, event=%u param=%u\n", __func__, event_id, param);
}

static void volume_change_cb(uint8_t volume, uint8_t ctype)
{
	haltest_info("%s, volume=%d ctype=%d\n", __func__, volume, ctype);
}

static void passthrough_cmd_cb(int id, int key_state)
{
	haltest_info("%s, id=%d key_state=%d\n", __func__, id, key_state);
}

static btrc_callbacks_t rc_cbacks = {
	.size = sizeof(rc_cbacks),
	.remote_features_cb = remote_features_cb,
	.get_play_status_cb = get_play_status_cb,
	.list_player_app_attr_cb = list_player_app_attr_cb,
	.list_player_app_values_cb = list_player_app_values_cb,
	.get_player_app_value_cb = get_player_app_value_cb,
	.get_player_app_attrs_text_cb = get_player_app_attrs_text_cb,
	.get_player_app_values_text_cb = get_player_app_values_text_cb,
	.set_player_app_value_cb = set_player_app_value_cb,
	.get_element_attr_cb = get_element_attr_cb,
	.register_notification_cb = register_notification_cb,
	.volume_change_cb = volume_change_cb,
	.passthrough_cmd_cb = passthrough_cmd_cb,
};

/* init */

static void init_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_rc);

	EXEC(if_rc->init, &rc_cbacks);
}

/* get_play_status_rsp */

static void get_play_status_rsp_c(int argc, const char **argv,
					enum_func *enum_func, void **user)
{
	if (argc == 3) {
		*user = TYPE_ENUM(btrc_play_status_t);
		*enum_func = enum_defines;
	}
}

static void get_play_status_rsp_p(int argc, const char **argv)
{
	btrc_play_status_t play_status;
	uint32_t song_len, song_pos;

	RETURN_IF_NULL(if_rc);

	if (argc <= 2) {
		haltest_error("No play status specified");
		return;
	}

	if (argc <= 3) {
		haltest_error("No song length specified");
		return;
	}

	if (argc <= 4) {
		haltest_error("No song position specified");
		return;
	}

	play_status = str2btrc_play_status_t(argv[2]);
	song_len = (uint32_t) atoi(argv[3]);
	song_pos = (uint32_t) atoi(argv[4]);

	EXEC(if_rc->get_play_status_rsp, play_status, song_len, song_pos);
}

/* get_element_attr_rsp */

static void get_element_attr_rsp_c(int argc, const char **argv,
					enum_func *enum_func, void **user)
{
	if (argc == 4) {
		*user = TYPE_ENUM(btrc_media_attr_t);
		*enum_func = enum_defines;
	}
}

static void get_element_attr_rsp_p(int argc, const char **argv)
{
	uint8_t num_attr;
	btrc_element_attr_val_t attrs;

	RETURN_IF_NULL(if_rc);

	if (argc <= 2) {
		haltest_error("No number of attributes specified");
		return;
	}

	if (argc <= 4) {
		haltest_error("No attr id and value specified");
		return;
	}

	num_attr = (uint8_t) atoi(argv[2]);
	attrs.attr_id = str2btrc_media_attr_t(argv[3]);
	strcpy((char *)attrs.text, argv[4]);

	EXEC(if_rc->get_element_attr_rsp, num_attr, &attrs);
}

/* set_volume */

static void set_volume_c(int argc, const char **argv,
					enum_func *enum_func, void **user)
{
}

static void set_volume_p(int argc, const char **argv)
{
	uint8_t volume;

	RETURN_IF_NULL(if_rc);

	if (argc <= 2) {
		haltest_error("No volume specified");
		return;
	}

	volume = (uint8_t) atoi(argv[2]);

	EXEC(if_rc->set_volume, volume);
}

/* set_player_app_value_rsp */

static void set_player_app_value_rsp_c(int argc, const char **argv,
					enum_func *enum_func, void **user)
{
	if (argc == 3) {
		*user = TYPE_ENUM(btrc_status_t);
		*enum_func = enum_defines;
	}
}

static void set_player_app_value_rsp_p(int argc, const char **argv)
{
	btrc_status_t rsp_status;

	RETURN_IF_NULL(if_rc);

	if (argc <= 2) {
		haltest_error("No response status specified");
		return;
	}

	rsp_status = str2btrc_status_t(argv[2]);

	EXEC(if_rc->set_player_app_value_rsp, rsp_status);
}

/* register_notification_rsp */

static void register_notification_rsp_c(int argc, const char **argv,
					enum_func *enum_func, void **user)
{
	if (argc == 3) {
		*user = TYPE_ENUM(btrc_event_id_t);
		*enum_func = enum_defines;
	}

	if (argc == 4) {
		*user = TYPE_ENUM(btrc_notification_type_t);
		*enum_func = enum_defines;
	}
}

static void register_notification_rsp_p(int argc, const char **argv)
{
	btrc_event_id_t event_id;
	btrc_notification_type_t type;
	btrc_register_notification_t reg;
	uint32_t song_pos;
	uint64_t track;

	RETURN_IF_NULL(if_rc);

	memset(&reg, 0, sizeof(reg));
	event_id = str2btrc_event_id_t(argv[2]);
	type = str2btrc_notification_type_t(argv[3]);

	switch (event_id) {
	case BTRC_EVT_PLAY_STATUS_CHANGED:
		reg.play_status = str2btrc_play_status_t(argv[4]);
		break;

	case BTRC_EVT_TRACK_CHANGE:
		track = strtoull(argv[5], NULL, 10);
		memcpy(reg.track, &track, sizeof(btrc_uid_t));
		break;

	case BTRC_EVT_TRACK_REACHED_END:
	case BTRC_EVT_TRACK_REACHED_START:
		break;

	case BTRC_EVT_PLAY_POS_CHANGED:
		song_pos = strtoul(argv[4], NULL, 10);
		memcpy(&reg.song_pos, &song_pos, sizeof(uint32_t));
		break;

	case BTRC_EVT_APP_SETTINGS_CHANGED:
		haltest_error("not supported");
		return;
	}

	EXEC(if_rc->register_notification_rsp, event_id, type, &reg);
}

/* cleanup */

static void cleanup_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_rc);

	EXECV(if_rc->cleanup);
	if_rc = NULL;
}

static struct method methods[] = {
	STD_METHOD(init),
	STD_METHODCH(get_play_status_rsp,
					"<play_status> <song_len> <song_pos>"),
	STD_METHODCH(get_element_attr_rsp, "<num_attr> <attrs_id> <value>"),
	STD_METHODCH(set_player_app_value_rsp, "<rsp_status>"),
	STD_METHODCH(set_volume, "<volume>"),
	STD_METHODCH(register_notification_rsp,
			"<event_id> <type> <respective_data...>\n"
			"BTRC_EVT_PLAY_STATUS_CHANGED <type> <play_status>\n"
			"BTRC_EVT_TRACK_CHANGE <type> <track>\n"
			"BTRC_EVT_TRACK_REACHED_END <type>\n"
			"BTRC_EVT_TRACK_REACHED_START <type>\n"
			"BTRC_EVT_PLAY_POS_CHANGED <type> <song_pos>\n"),
	STD_METHOD(cleanup),
	END_METHOD
};

const struct interface rc_if = {
	.name = "rc",
	.methods = methods
};

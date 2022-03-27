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

/* Control PDU ids */
#define AVRCP_GET_CAPABILITIES		0x10
#define AVRCP_LIST_PLAYER_ATTRIBUTES	0X11
#define AVRCP_LIST_PLAYER_VALUES	0x12
#define AVRCP_GET_CURRENT_PLAYER_VALUE	0x13
#define AVRCP_SET_PLAYER_VALUE		0x14
#define AVRCP_GET_PLAYER_ATTRIBUTE_TEXT	0x15
#define AVRCP_GET_PLAYER_VALUE_TEXT	0x16
#define AVRCP_DISPLAYABLE_CHARSET	0x17
#define AVRCP_CT_BATTERY_STATUS		0x18
#define AVRCP_GET_ELEMENT_ATTRIBUTES	0x20
#define AVRCP_GET_PLAY_STATUS		0x30
#define AVRCP_REGISTER_NOTIFICATION	0x31
#define AVRCP_REQUEST_CONTINUING	0x40
#define AVRCP_ABORT_CONTINUING		0x41
#define AVRCP_SET_ABSOLUTE_VOLUME	0x50
#define AVRCP_SET_ADDRESSED_PLAYER	0x60
#define AVRCP_SET_BROWSED_PLAYER	0x70
#define AVRCP_GET_FOLDER_ITEMS		0x71
#define AVRCP_CHANGE_PATH		0x72
#define AVRCP_GET_ITEM_ATTRIBUTES	0x73
#define AVRCP_PLAY_ITEM			0x74
#define AVRCP_SEARCH			0x80
#define AVRCP_ADD_TO_NOW_PLAYING	0x90
#define AVRCP_GENERAL_REJECT		0xA0

/* Notification events */
#define AVRCP_EVENT_STATUS_CHANGED		0x01
#define AVRCP_EVENT_TRACK_CHANGED		0x02
#define AVRCP_EVENT_TRACK_REACHED_END		0x03
#define AVRCP_EVENT_TRACK_REACHED_START		0x04
#define AVRCP_EVENT_PLAYBACK_POS_CHANGED	0x05
#define AVRCP_EVENT_SETTINGS_CHANGED		0x08
#define AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED	0x09
#define AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED	0x0a
#define AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED	0x0b
#define AVRCP_EVENT_UIDS_CHANGED		0x0c
#define AVRCP_EVENT_VOLUME_CHANGED		0x0d
#define AVRCP_EVENT_LAST			AVRCP_EVENT_VOLUME_CHANGED

/* Status codes */
#define AVRCP_STATUS_INVALID_COMMAND		0x00
#define AVRCP_STATUS_INVALID_PARAM		0x01
#define AVRCP_STATUS_PARAM_NOT_FOUND		0x02
#define AVRCP_STATUS_INTERNAL_ERROR		0x03
#define AVRCP_STATUS_SUCCESS			0x04
#define AVRCP_STATUS_UID_CHANGED		0x05
#define AVRCP_STATUS_NOT_DIRECTORY		0x08
#define AVRCP_STATUS_DOES_NOT_EXIST		0x09
#define AVRCP_STATUS_INVALID_SCOPE		0x0a
#define AVRCP_STATUS_OUT_OF_BOUNDS		0x0b
#define AVRCP_STATUS_INVALID_PLAYER_ID		0x11
#define AVRCP_STATUS_PLAYER_NOT_BROWSABLE	0x12
#define AVRCP_STATUS_NO_AVAILABLE_PLAYERS	0x15
#define AVRCP_STATUS_ADDRESSED_PLAYER_CHANGED	0x16

/* Capabilities for AVRCP_GET_CAPABILITIES pdu */
#define CAP_COMPANY_ID				0x02
#define CAP_EVENTS_SUPPORTED			0x03

/* Player Attributes */
#define AVRCP_ATTRIBUTE_ILEGAL			0x00
#define AVRCP_ATTRIBUTE_EQUALIZER		0x01
#define AVRCP_ATTRIBUTE_REPEAT_MODE		0x02
#define AVRCP_ATTRIBUTE_SHUFFLE			0x03
#define AVRCP_ATTRIBUTE_SCAN			0x04
#define AVRCP_ATTRIBUTE_LAST			AVRCP_ATTRIBUTE_SCAN

/* equalizer values */
#define AVRCP_EQUALIZER_OFF		0x01
#define AVRCP_EQUALIZER_ON		0x02

/* repeat mode values */
#define AVRCP_REPEAT_MODE_OFF		0x01
#define AVRCP_REPEAT_MODE_SINGLE	0x02
#define AVRCP_REPEAT_MODE_ALL		0x03
#define AVRCP_REPEAT_MODE_GROUP		0x04

/* shuffle values */
#define AVRCP_SHUFFLE_OFF		0x01
#define AVRCP_SHUFFLE_ALL		0x02
#define AVRCP_SHUFFLE_GROUP		0x03

/* scan values */
#define AVRCP_SCAN_OFF			0x01
#define AVRCP_SCAN_ALL			0x02
#define AVRCP_SCAN_GROUP		0x03

/* media attributes */
#define AVRCP_MEDIA_ATTRIBUTE_ILLEGAL	0x00
#define AVRCP_MEDIA_ATTRIBUTE_TITLE	0x01
#define AVRCP_MEDIA_ATTRIBUTE_ARTIST	0x02
#define AVRCP_MEDIA_ATTRIBUTE_ALBUM	0x03
#define AVRCP_MEDIA_ATTRIBUTE_TRACK	0x04
#define AVRCP_MEDIA_ATTRIBUTE_N_TRACKS	0x05
#define AVRCP_MEDIA_ATTRIBUTE_GENRE	0x06
#define AVRCP_MEDIA_ATTRIBUTE_DURATION	0x07
#define AVRCP_MEDIA_ATTRIBUTE_LAST	AVRCP_MEDIA_ATTRIBUTE_DURATION

/* Media Scope */
#define AVRCP_MEDIA_PLAYER_LIST			0x00
#define AVRCP_MEDIA_PLAYER_VFS			0x01
#define AVRCP_MEDIA_SEARCH			0x02
#define AVRCP_MEDIA_NOW_PLAYING			0x03

/* SDP features */
#define AVRCP_FEATURE_CATEGORY_1	0x0001
#define AVRCP_FEATURE_CATEGORY_2	0x0002
#define AVRCP_FEATURE_CATEGORY_3	0x0004
#define AVRCP_FEATURE_CATEGORY_4	0x0008
#define AVRCP_FEATURE_PLAYER_SETTINGS	0x0010
#define AVRCP_FEATURE_GROUP_NAVIGATION	0x0020
#define AVRCP_FEATURE_BROWSING		0x0040
#define AVRCP_FEATURE_MULTIPLE_PLAYERS	0x0080

/* Company IDs for vendor dependent commands */
#define IEEEID_BTSIG		0x001958

struct avrcp;

struct avrcp_control_ind {
	int (*get_capabilities) (struct avrcp *session, uint8_t transaction,
							void *user_data);
	int (*list_attributes) (struct avrcp *session, uint8_t transaction,
							void *user_data);
	int (*get_attribute_text) (struct avrcp *session, uint8_t transaction,
					uint8_t number, uint8_t *attrs,
					void *user_data);
	int (*list_values) (struct avrcp *session, uint8_t transaction,
					uint8_t attr, void *user_data);
	int (*get_value_text) (struct avrcp *session, uint8_t transaction,
					uint8_t attr, uint8_t number,
					uint8_t *values, void *user_data);
	int (*get_value) (struct avrcp *session, uint8_t transaction,
					uint8_t number, uint8_t *attrs,
					void *user_data);
	int (*set_value) (struct avrcp *session, uint8_t transaction,
					uint8_t number, uint8_t *attrs,
					uint8_t *values, void *user_data);
	int (*get_play_status) (struct avrcp *session, uint8_t transaction,
					void *user_data);
	int (*get_element_attributes) (struct avrcp *session,
					uint8_t transaction, uint64_t uid,
					uint8_t number, uint32_t *attrs,
					void *user_data);
	int (*register_notification) (struct avrcp *session,
					uint8_t transaction, uint8_t event,
					uint32_t interval, void *user_data);
	int (*set_volume) (struct avrcp *session, uint8_t transaction,
					uint8_t volume, void *user_data);
	int (*set_addressed) (struct avrcp *session, uint8_t transaction,
					uint16_t id, void *user_data);
	int (*set_browsed) (struct avrcp *session, uint8_t transaction,
					uint16_t id, void *user_data);
	int (*get_folder_items) (struct avrcp *session, uint8_t transaction,
					uint8_t scope, uint32_t start,
					uint32_t end, uint16_t number,
					uint32_t *attrs, void *user_data);
	int (*change_path) (struct avrcp *session, uint8_t transaction,
					uint16_t counter, uint8_t direction,
					uint64_t uid, void *user_data);
	int (*get_item_attributes) (struct avrcp *session, uint8_t transaction,
					uint8_t scope, uint64_t uid,
					uint16_t counter, uint8_t number,
					uint32_t *attrs, void *user_data);
	int (*play_item) (struct avrcp *session, uint8_t transaction,
					uint8_t scope, uint64_t uid,
					uint16_t counter, void *user_data);
	int (*search) (struct avrcp *session, uint8_t transaction,
					const char *string, void *user_data);
	int (*add_to_now_playing) (struct avrcp *session, uint8_t transaction,
					uint8_t scope, uint64_t uid,
					uint16_t counter, void *user_data);
};

struct avrcp_control_cfm {
	void (*get_capabilities) (struct avrcp *session, int err,
					uint8_t number, uint8_t *params,
					void *user_data);
	void (*list_attributes) (struct avrcp *session, int err,
					uint8_t number, uint8_t *attrs,
					void *user_data);
	void (*get_attribute_text) (struct avrcp *session, int err,
					uint8_t number, uint8_t *attrs,
					char **text, void *user_data);
	void (*list_values) (struct avrcp *session, int err,
					uint8_t number, uint8_t *values,
					void *user_data);
	void (*get_value_text) (struct avrcp *session, int err,
					uint8_t number, uint8_t *values,
					char **text, void *user_data);
	void (*get_value) (struct avrcp *session, int err,
					uint8_t number, uint8_t *attrs,
					uint8_t *values, void *user_data);
	void (*set_value) (struct avrcp *session, int err, void *user_data);
	void (*get_play_status) (struct avrcp *session, int err,
					uint8_t status, uint32_t position,
					uint32_t duration, void *user_data);
	void (*get_element_attributes) (struct avrcp *session, int err,
					uint8_t number, uint32_t *attrs,
					char **text, void *user_data);
	bool (*register_notification) (struct avrcp *session, int err,
					uint8_t code, uint8_t event,
					void *params, void *user_data);
	void (*set_volume) (struct avrcp *session, int err, uint8_t volume,
					void *user_data);
	void (*set_addressed) (struct avrcp *session, int err,
					void *user_data);
	void (*set_browsed) (struct avrcp *session, int err,
					uint16_t counter, uint32_t items,
					char *path, void *user_data);
	void (*get_folder_items) (struct avrcp *session, int err,
					uint16_t counter, uint16_t number,
					uint8_t *params, void *user_data);
	void (*change_path) (struct avrcp *session, int err,
					uint32_t items, void *user_data);
	void (*get_item_attributes) (struct avrcp *session, int err,
					uint8_t number, uint32_t *attrs,
					char **text, void *user_data);
	void (*play_item) (struct avrcp *session, int err, void *user_data);
	void (*search) (struct avrcp *session, int err, uint16_t counter,
					uint32_t items, void *user_data);
	void (*add_to_now_playing) (struct avrcp *session, int err,
					void *user_data);
};

struct avrcp_passthrough_handler {
	uint8_t op;
	bool (*func) (struct avrcp *session, bool pressed, void *user_data);
};

typedef void (*avrcp_destroy_cb_t) (void *user_data);

struct avrcp *avrcp_new(int fd, size_t imtu, size_t omtu, uint16_t version);
void avrcp_shutdown(struct avrcp *session);
void avrcp_set_destroy_cb(struct avrcp *session, avrcp_destroy_cb_t cb,
							void *user_data);
int avrcp_connect_browsing(struct avrcp *session, int fd, size_t imtu,
								size_t omtu);

void avrcp_register_player(struct avrcp *session,
				const struct avrcp_control_ind *ind,
				const struct avrcp_control_cfm *cfm,
				void *user_data);
void avrcp_set_passthrough_handlers(struct avrcp *session,
			const struct avrcp_passthrough_handler *handlers,
			void *user_data);
int avrcp_init_uinput(struct avrcp *session, const char *name,
							const char *address);
int avrcp_send(struct avrcp *session, uint8_t transaction, uint8_t code,
					uint8_t subunit, uint8_t pdu_id,
					const struct iovec *iov, int iov_cnt);
int avrcp_get_capabilities(struct avrcp *session, uint8_t param);
int avrcp_register_notification(struct avrcp *session, uint8_t event,
							uint32_t interval);
int avrcp_list_player_attributes(struct avrcp *session);
int avrcp_get_player_attribute_text(struct avrcp *session, uint8_t number,
							uint8_t *attrs);
int avrcp_list_player_values(struct avrcp *session, uint8_t attr);
int avrcp_get_player_value_text(struct avrcp *session, uint8_t attr,
					uint8_t number, uint8_t *values);
int avrcp_set_player_value(struct avrcp *session, uint8_t number,
					uint8_t *attrs, uint8_t *values);
int avrcp_get_current_player_value(struct avrcp *session, uint8_t number,
							uint8_t *attrs);
int avrcp_get_play_status(struct avrcp *session);
int avrcp_set_volume(struct avrcp *session, uint8_t volume);
int avrcp_get_element_attributes(struct avrcp *session);
int avrcp_set_addressed_player(struct avrcp *session, uint16_t player_id);
int avrcp_set_browsed_player(struct avrcp *session, uint16_t player_id);
int avrcp_get_folder_items(struct avrcp *session, uint8_t scope,
				uint32_t start, uint32_t end, uint8_t number,
				uint32_t *attrs);
int avrcp_change_path(struct avrcp *session, uint8_t direction, uint64_t uid,
							uint16_t counter);
int avrcp_get_item_attributes(struct avrcp *session, uint8_t scope,
				uint64_t uid, uint16_t counter, uint8_t number,
				uint32_t *attrs);
int avrcp_play_item(struct avrcp *session, uint8_t scope, uint64_t uid,
							uint16_t counter);
int avrcp_search(struct avrcp *session, const char *string);
int avrcp_add_to_now_playing(struct avrcp *session, uint8_t scope, uint64_t uid,
							uint16_t counter);

int avrcp_get_capabilities_rsp(struct avrcp *session, uint8_t transaction,
					uint8_t number, uint8_t *events);
int avrcp_list_player_attributes_rsp(struct avrcp *session, uint8_t transaction,
					uint8_t number, uint8_t *attrs);
int avrcp_get_player_attribute_text_rsp(struct avrcp *session,
					uint8_t transaction, uint8_t number,
					uint8_t *attrs, const char **text);
int avrcp_list_player_values_rsp(struct avrcp *session, uint8_t transaction,
					uint8_t number, uint8_t *values);
int avrcp_get_play_status_rsp(struct avrcp *session, uint8_t transaction,
				uint32_t position, uint32_t duration,
				uint8_t status);
int avrcp_get_player_values_text_rsp(struct avrcp *session,
					uint8_t transaction, uint8_t number,
					uint8_t *values, const char **text);
int avrcp_get_current_player_value_rsp(struct avrcp *session,
					uint8_t transaction, uint8_t number,
					uint8_t *attrs, uint8_t *values);
int avrcp_set_player_value_rsp(struct avrcp *session, uint8_t transaction);
int avrcp_get_element_attrs_rsp(struct avrcp *session, uint8_t transaction,
					uint8_t *params, size_t params_len);
int avrcp_register_notification_rsp(struct avrcp *session, uint8_t transaction,
					uint8_t code, uint8_t event,
					void *data, size_t len);
int avrcp_set_volume_rsp(struct avrcp *session, uint8_t transaction,
							uint8_t volume);
int avrcp_set_addressed_player_rsp(struct avrcp *session, uint8_t transaction,
							uint8_t status);
int avrcp_set_browsed_player_rsp(struct avrcp *session, uint8_t transaction,
					uint8_t status, uint16_t counter,
					uint32_t items, uint8_t depth,
					const char **folders);
int avrcp_get_folder_items_rsp(struct avrcp *session, uint8_t transaction,
					uint8_t status, uint16_t counter,
					uint8_t number, uint8_t *type,
					uint16_t *len, uint8_t **params);
int avrcp_change_path_rsp(struct avrcp *session, uint8_t transaction,
						uint8_t status, uint32_t items);
int avrcp_get_item_attributes_rsp(struct avrcp *session, uint8_t transaction,
					uint8_t status, uint8_t number,
					uint32_t *attrs, const char **text);
int avrcp_play_item_rsp(struct avrcp *session, uint8_t transaction,
					uint8_t status);
int avrcp_search_rsp(struct avrcp *session, uint8_t transaction, uint8_t status,
					uint16_t counter, uint32_t items);
int avrcp_add_to_now_playing_rsp(struct avrcp *session, uint8_t transaction,
								uint8_t status);

int avrcp_send_passthrough(struct avrcp *session, uint32_t vendor, uint8_t op);

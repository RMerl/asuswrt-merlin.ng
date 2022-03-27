/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/* player attributes */
#define AVRCP_ATTRIBUTE_ILEGAL		0x00
#define AVRCP_ATTRIBUTE_EQUALIZER	0x01
#define AVRCP_ATTRIBUTE_REPEAT_MODE	0x02
#define AVRCP_ATTRIBUTE_SHUFFLE		0x03
#define AVRCP_ATTRIBUTE_SCAN		0x04
#define AVRCP_ATTRIBUTE_LAST		AVRCP_ATTRIBUTE_SCAN

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

/* play status */
#define AVRCP_PLAY_STATUS_STOPPED	0x00
#define AVRCP_PLAY_STATUS_PLAYING	0x01
#define AVRCP_PLAY_STATUS_PAUSED	0x02
#define AVRCP_PLAY_STATUS_FWD_SEEK	0x03
#define AVRCP_PLAY_STATUS_REV_SEEK	0x04
#define AVRCP_PLAY_STATUS_ERROR		0xFF

/* Notification events */
#define AVRCP_EVENT_STATUS_CHANGED		0x01
#define AVRCP_EVENT_TRACK_CHANGED		0x02
#define AVRCP_EVENT_TRACK_REACHED_END		0x03
#define AVRCP_EVENT_TRACK_REACHED_START		0x04
#define AVRCP_EVENT_PLAYBACK_POS_CHANGED	0x05
#define AVRCP_EVENT_SETTINGS_CHANGED		0x08
#define AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED	0x0a
#define AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED	0x0b
#define AVRCP_EVENT_UIDS_CHANGED		0x0c
#define AVRCP_EVENT_VOLUME_CHANGED		0x0d
#define AVRCP_EVENT_LAST			AVRCP_EVENT_VOLUME_CHANGED

struct avrcp_player_cb {
	GList *(*list_settings) (void *user_data);
	const char *(*get_setting) (const char *key, void *user_data);
	int (*set_setting) (const char *key, const char *value,
							void *user_data);
	uint64_t (*get_uid) (void *user_data);
	const char *(*get_metadata) (const char *key, void *user_data);
	GList *(*list_metadata) (void *user_data);
	const char *(*get_status) (void *user_data);
	uint32_t (*get_position) (void *user_data);
	uint32_t (*get_duration) (void *user_data);
	const char *(*get_name) (void *user_data);
	void (*set_volume) (uint8_t volume, struct btd_device *dev,
							void *user_data);
	bool (*play) (void *user_data);
	bool (*stop) (void *user_data);
	bool (*pause) (void *user_data);
	bool (*next) (void *user_data);
	bool (*previous) (void *user_data);
};

int avrcp_set_volume(struct btd_device *dev, uint8_t volume, bool notify);

struct avrcp_player *avrcp_register_player(struct btd_adapter *adapter,
						struct avrcp_player_cb *cb,
						void *user_data,
						GDestroyNotify destroy);
void avrcp_unregister_player(struct avrcp_player *player);

void avrcp_player_event(struct avrcp_player *player, uint8_t id,
							const void *data);


size_t avrcp_handle_vendor_reject(uint8_t *code, uint8_t *operands);
size_t avrcp_browsing_general_reject(uint8_t *operands);

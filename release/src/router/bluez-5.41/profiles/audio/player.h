/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2007  Nokia Corporation
 *  Copyright (C) 2004-2009  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2012-2012  Intel Corporation
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

typedef enum {
	PLAYER_ITEM_TYPE_AUDIO,
	PLAYER_ITEM_TYPE_VIDEO,
	PLAYER_ITEM_TYPE_FOLDER,
	PLAYER_ITEM_TYPE_INVALID,
} player_item_type_t;

typedef enum {
	PLAYER_FOLDER_TYPE_MIXED,
	PLAYER_FOLDER_TYPE_TITLES,
	PLAYER_FOLDER_TYPE_ALBUMS,
	PLAYER_FOLDER_TYPE_ARTISTS,
	PLAYER_FOLDER_TYPE_GENRES,
	PLAYER_FOLDER_TYPE_PLAYLISTS,
	PLAYER_FOLDER_TYPE_YEARS,
	PLAYER_FOLDER_TYPE_INVALID,
} player_folder_type_t;

struct media_player;
struct media_item;

struct media_player_callback {
	bool (*set_setting) (struct media_player *mp, const char *key,
				const char *value, void *user_data);
	int (*play) (struct media_player *mp, void *user_data);
	int (*pause) (struct media_player *mp, void *user_data);
	int (*stop) (struct media_player *mp, void *user_data);
	int (*next) (struct media_player *mp, void *user_data);
	int (*previous) (struct media_player *mp, void *user_data);
	int (*fast_forward) (struct media_player *mp, void *user_data);
	int (*rewind) (struct media_player *mp, void *user_data);
	int (*list_items) (struct media_player *mp, const char *name,
				uint32_t start, uint32_t end, void *user_data);
	int (*change_folder) (struct media_player *mp, const char *path,
						uint64_t uid, void *user_data);
	int (*search) (struct media_player *mp, const char *string,
						void *user_data);
	int (*play_item) (struct media_player *mp, const char *name,
					uint64_t uid, void *user_data);
	int (*add_to_nowplaying) (struct media_player *mp, const char *name,
					uint64_t uid, void *user_data);
	int (*total_items) (struct media_player *mp, const char *name,
						void *user_data);
};

struct media_player *media_player_controller_create(const char *path,
								uint16_t id);
const char *media_player_get_path(struct media_player *mp);
void media_player_destroy(struct media_player *mp);
void media_player_set_duration(struct media_player *mp, uint32_t duration);
void media_player_set_position(struct media_player *mp, uint32_t position);
void media_player_set_setting(struct media_player *mp, const char *key,
							const char *value);
const char *media_player_get_status(struct media_player *mp);
void media_player_set_status(struct media_player *mp, const char *status);
void media_player_set_metadata(struct media_player *mp,
				struct media_item *item, const char *key,
				void *data, size_t len);
void media_player_set_type(struct media_player *mp, const char *type);
void media_player_set_subtype(struct media_player *mp, const char *subtype);
void media_player_set_name(struct media_player *mp, const char *name);
void media_player_set_browsable(struct media_player *mp, bool enabled);
void media_player_set_searchable(struct media_player *mp, bool enabled);
void media_player_set_folder(struct media_player *mp, const char *path,
								uint32_t items);
void media_player_set_playlist(struct media_player *mp, const char *name);
struct media_item *media_player_set_playlist_item(struct media_player *mp,
								uint64_t uid);

struct media_item *media_player_create_folder(struct media_player *mp,
						const char *name,
						player_folder_type_t type,
						uint64_t uid);
struct media_item *media_player_create_item(struct media_player *mp,
						const char *name,
						player_item_type_t type,
						uint64_t uid);

void media_item_set_playable(struct media_item *item, bool value);
void media_player_list_complete(struct media_player *mp, GSList *items,
								int err);
void media_player_change_folder_complete(struct media_player *player,
						const char *path, int ret);
void media_player_search_complete(struct media_player *mp, int ret);
void media_player_total_items_complete(struct media_player *mp,
						uint32_t num_of_items);

void media_player_set_callbacks(struct media_player *mp,
				const struct media_player_callback *cbs,
				void *user_data);

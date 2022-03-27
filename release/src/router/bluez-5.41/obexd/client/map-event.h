/*
 *
 *  OBEX
 *
 *  Copyright (C) 2013  BMW Car IT GmbH. All rights reserved.
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

struct obc_session;

enum map_event_type {
	MAP_ET_NEW_MESSAGE,
	MAP_ET_DELIVERY_SUCCESS,
	MAP_ET_SENDING_SUCCESS,
	MAP_ET_DELIVERY_FAILURE,
	MAP_ET_SENDING_FAILURE,
	MAP_ET_MEMORY_FULL,
	MAP_ET_MEMORY_AVAILABLE,
	MAP_ET_MESSAGE_DELETED,
	MAP_ET_MESSAGE_SHIFT
};

struct map_event {
	enum map_event_type type;
	uint64_t handle;
	char *folder;
	char *old_folder;
	char *msg_type;
	char *datetime;
	char *subject;
	char *sender_name;
	char *priority;
};

/* Handle notification in map client.
 *
 * event: Event report.
 *
 * Callback shall be called for every received event.
 */
typedef void (*map_event_cb) (struct map_event *event, void *user_data);

/* Registers client notification handler callback for events that are
 * addressed to the given mas instance id for the given device.
 */
bool map_register_event_handler(struct obc_session *session, int mas_id,
					map_event_cb cb, void *user_data);

/* Unregisters client notification handler callback.
 */
void map_unregister_event_handler(struct obc_session *session, int mas_id);

/* Dispatch notification to a registered notification handler callback.
 */
void map_dispatch_event(int mas_id, const char *device,
						struct map_event *event);

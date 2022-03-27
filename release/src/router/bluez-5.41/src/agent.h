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

struct agent;

typedef void (*agent_cb) (struct agent *agent, DBusError *err,
				void *user_data);

typedef void (*agent_pincode_cb) (struct agent *agent, DBusError *err,
					const char *pincode, void *user_data);

typedef void (*agent_passkey_cb) (struct agent *agent, DBusError *err,
					uint32_t passkey, void *user_data);

struct agent *agent_ref(struct agent *agent);
void agent_unref(struct agent *agent);

struct agent *agent_get(const char *owner);

int agent_authorize_service(struct agent *agent, const char *path,
				const char *uuid, agent_cb cb,
				void *user_data, GDestroyNotify destroy);

int agent_request_pincode(struct agent *agent, struct btd_device *device,
				agent_pincode_cb cb, gboolean secure,
				void *user_data, GDestroyNotify destroy);

int agent_request_passkey(struct agent *agent, struct btd_device *device,
				agent_passkey_cb cb, void *user_data,
				GDestroyNotify destroy);

int agent_request_confirmation(struct agent *agent, struct btd_device *device,
				uint32_t passkey, agent_cb cb,
				void *user_data, GDestroyNotify destroy);

int agent_request_authorization(struct agent *agent, struct btd_device *device,
						agent_cb cb, void *user_data,
						GDestroyNotify destroy);

int agent_display_passkey(struct agent *agent, struct btd_device *device,
				uint32_t passkey, uint16_t entered);

int agent_display_pincode(struct agent *agent, struct btd_device *device,
				const char *pincode, agent_cb cb,
				void *user_data, GDestroyNotify destroy);

int agent_cancel(struct agent *agent);

uint8_t agent_get_io_capability(struct agent *agent);

void btd_agent_init(void);
void btd_agent_cleanup(void);

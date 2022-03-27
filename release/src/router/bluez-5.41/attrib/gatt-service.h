/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011  Nokia Corporation
 *  Copyright (C) 2011  Marcel Holtmann <marcel@holtmann.org>
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
	GATT_OPT_INVALID = 0,

	/* bt_uuid_t* value */
	GATT_OPT_CHR_UUID,

	/* a uint16 value */
	GATT_OPT_CHR_UUID16,

	GATT_OPT_CHR_PROPS,
	GATT_OPT_CHR_VALUE_CB,
	GATT_OPT_CHR_AUTHENTICATION,
	GATT_OPT_CHR_AUTHORIZATION,

	/* Get attribute handle for characteristic value */
	GATT_OPT_CHR_VALUE_GET_HANDLE,

	/* Get handle for ccc attribute */
	GATT_OPT_CCC_GET_HANDLE,

	/* arguments for authentication/authorization */
	GATT_CHR_VALUE_READ,
	GATT_CHR_VALUE_WRITE,
	GATT_CHR_VALUE_BOTH,
} gatt_option;

typedef enum {
	ATTRIB_READ,
	ATTRIB_WRITE,
} attrib_event_t;

gboolean gatt_service_add(struct btd_adapter *adapter, uint16_t uuid,
					bt_uuid_t *svc_uuid, gatt_option opt1, ...);

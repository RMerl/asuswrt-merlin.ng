/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012 Texas Instruments Corporation
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

/* Requirements for read/write operations */
enum {
	ATT_NONE,		/* No restrictions */
	ATT_AUTHENTICATION,	/* Authentication required */
	ATT_AUTHORIZATION,	/* Authorization required */
	ATT_NOT_PERMITTED,	/* Operation not permitted */
};

struct attribute {
	uint16_t handle;
	bt_uuid_t uuid;
	int read_req;		/* Read requirement */
	int write_req;		/* Write requirement */
	uint8_t (*read_cb)(struct attribute *a, struct btd_device *device,
							gpointer user_data);
	uint8_t (*write_cb)(struct attribute *a, struct btd_device *device,
							gpointer user_data);
	gpointer cb_user_data;
	size_t len;
	uint8_t *data;
};

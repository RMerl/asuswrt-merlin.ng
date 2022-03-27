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

struct enabled {
	gboolean linkloss;
	gboolean pathloss;
	gboolean findme;
};

int monitor_register_linkloss(struct btd_device *device,
						struct enabled *enabled,
						struct gatt_primary *linkloss);
int monitor_register_txpower(struct btd_device *device,
						struct enabled *enabled,
						struct gatt_primary *txpower);
int monitor_register_immediate(struct btd_device *device,
						struct enabled *enabled,
						struct gatt_primary *immediate);

void monitor_unregister_linkloss(struct btd_device *device);
void monitor_unregister_txpower(struct btd_device *device);
void monitor_unregister_immediate(struct btd_device *device);

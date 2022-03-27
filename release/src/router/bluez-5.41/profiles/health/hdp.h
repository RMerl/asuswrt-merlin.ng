/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2010 GSyC/LibreSoft, Universidad Rey Juan Carlos.
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

int hdp_adapter_register(struct btd_adapter *btd_adapter);
void hdp_adapter_unregister(struct btd_adapter *btd_adapter);

int hdp_device_register(struct btd_device *device);
void hdp_device_unregister(struct btd_device *device);

int hdp_manager_start(void);
void hdp_manager_stop(void);

gboolean hdp_set_mcl_cb(struct hdp_device *device, GError **err);

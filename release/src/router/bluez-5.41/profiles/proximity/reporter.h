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

#define PROXIMITY_REPORTER_INTERFACE "org.bluez.ProximityReporter1"

#define IMMEDIATE_ALERT_SVC_UUID	0x1802
#define LINK_LOSS_SVC_UUID		0x1803
#define TX_POWER_SVC_UUID		0x1804
#define ALERT_LEVEL_CHR_UUID		0x2A06
#define POWER_LEVEL_CHR_UUID		0x2A07

enum {
	NO_ALERT = 0x00,
	MILD_ALERT = 0x01,
	HIGH_ALERT = 0x02,
};

void reporter_device_remove(struct btd_service *service);
int reporter_device_probe(struct btd_service *service);

int reporter_adapter_probe(struct btd_profile *p, struct btd_adapter *adapter);
void reporter_adapter_remove(struct btd_profile *p,
						struct btd_adapter *adapter);

const char *get_alert_level_string(uint8_t level);

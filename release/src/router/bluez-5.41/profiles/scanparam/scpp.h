/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Intel Corporation. All rights reserved.
 *
 *
 *  This library is free software; you can rescpptribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is scpptributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

struct bt_scpp;

struct bt_scpp *bt_scpp_new(void *primary);

struct bt_scpp *bt_scpp_ref(struct bt_scpp *scan);
void bt_scpp_unref(struct bt_scpp *scan);

bool bt_scpp_attach(struct bt_scpp *scan, void *gatt);
void bt_scpp_detach(struct bt_scpp *scan);

bool bt_scpp_set_interval(struct bt_scpp *scan, uint16_t value);
bool bt_scpp_set_window(struct bt_scpp *scan, uint16_t value);

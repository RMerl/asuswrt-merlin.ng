/* SPDX-License-Identifier: LGPL-2.1-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2019  Intel Corporation. All rights reserved.
 *
 *
 */

bool manager_dbus_init(struct l_dbus *dbus);
void manager_scan_cancel(struct mesh_node *node);

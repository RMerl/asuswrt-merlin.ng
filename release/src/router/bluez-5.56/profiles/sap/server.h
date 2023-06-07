/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2010 ST-Ericsson SA
 *
 */

int sap_server_register(struct btd_adapter *adapter);
void sap_server_unregister(const char *path);

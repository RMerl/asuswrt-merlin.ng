/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

char *bt_modalias(uint16_t source, uint16_t vendor,
					uint16_t product, uint16_t version);
char *bt_uuid2string(uuid_t *uuid);
char *bt_name2string(const char *string);
int bt_string2uuid(uuid_t *uuid, const char *string);

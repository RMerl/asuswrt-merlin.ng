/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011  Nokia Corporation
 *
 *
 */

int interactive(const char *src, const char *dst, const char *dst_type,
								int psm);
GIOChannel *gatt_connect(const char *src, const char *dst,
			const char *dst_type, const char *sec_level,
			int psm, int mtu, BtIOConnect connect_cb,
			GError **gerr);
size_t gatt_attr_data_from_string(const char *str, uint8_t **data);

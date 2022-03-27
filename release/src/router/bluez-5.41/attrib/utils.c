/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2011  Nokia Corporation
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/hci.h"
#include "lib/hci_lib.h"
#include "lib/sdp.h"
#include "lib/uuid.h"

#include "btio/btio.h"
#include "att.h"
#include "gattrib.h"
#include "gatt.h"
#include "gatttool.h"

GIOChannel *gatt_connect(const char *src, const char *dst,
				const char *dst_type, const char *sec_level,
				int psm, int mtu, BtIOConnect connect_cb,
				GError **gerr)
{
	GIOChannel *chan;
	bdaddr_t sba, dba;
	uint8_t dest_type;
	GError *tmp_err = NULL;
	BtIOSecLevel sec;

	str2ba(dst, &dba);

	/* Local adapter */
	if (src != NULL) {
		if (!strncmp(src, "hci", 3))
			hci_devba(atoi(src + 3), &sba);
		else
			str2ba(src, &sba);
	} else
		bacpy(&sba, BDADDR_ANY);

	/* Not used for BR/EDR */
	if (strcmp(dst_type, "random") == 0)
		dest_type = BDADDR_LE_RANDOM;
	else
		dest_type = BDADDR_LE_PUBLIC;

	if (strcmp(sec_level, "medium") == 0)
		sec = BT_IO_SEC_MEDIUM;
	else if (strcmp(sec_level, "high") == 0)
		sec = BT_IO_SEC_HIGH;
	else
		sec = BT_IO_SEC_LOW;

	if (psm == 0)
		chan = bt_io_connect(connect_cb, NULL, NULL, &tmp_err,
				BT_IO_OPT_SOURCE_BDADDR, &sba,
				BT_IO_OPT_SOURCE_TYPE, BDADDR_LE_PUBLIC,
				BT_IO_OPT_DEST_BDADDR, &dba,
				BT_IO_OPT_DEST_TYPE, dest_type,
				BT_IO_OPT_CID, ATT_CID,
				BT_IO_OPT_SEC_LEVEL, sec,
				BT_IO_OPT_INVALID);
	else
		chan = bt_io_connect(connect_cb, NULL, NULL, &tmp_err,
				BT_IO_OPT_SOURCE_BDADDR, &sba,
				BT_IO_OPT_DEST_BDADDR, &dba,
				BT_IO_OPT_PSM, psm,
				BT_IO_OPT_IMTU, mtu,
				BT_IO_OPT_SEC_LEVEL, sec,
				BT_IO_OPT_INVALID);

	if (tmp_err) {
		g_propagate_error(gerr, tmp_err);
		return NULL;
	}

	return chan;
}

size_t gatt_attr_data_from_string(const char *str, uint8_t **data)
{
	char tmp[3];
	size_t size, i;

	size = strlen(str) / 2;
	*data = g_try_malloc0(size);
	if (*data == NULL)
		return 0;

	tmp[2] = '\0';
	for (i = 0; i < size; i++) {
		memcpy(tmp, str + (i * 2), 2);
		(*data)[i] = (uint8_t) strtol(tmp, NULL, 16);
	}

	return size;
}

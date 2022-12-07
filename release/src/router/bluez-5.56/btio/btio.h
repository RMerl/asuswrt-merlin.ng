/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2009-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2009-2010  Nokia Corporation
 *
 *
 */
#ifndef BT_IO_H
#define BT_IO_H

#include <glib.h>

#define BT_IO_ERROR bt_io_error_quark()

GQuark bt_io_error_quark(void);

typedef enum {
	BT_IO_OPT_INVALID = 0,
	BT_IO_OPT_SOURCE,
	BT_IO_OPT_SOURCE_BDADDR,
	BT_IO_OPT_SOURCE_TYPE,
	BT_IO_OPT_DEST,
	BT_IO_OPT_DEST_BDADDR,
	BT_IO_OPT_DEST_TYPE,
	BT_IO_OPT_DEFER_TIMEOUT,
	BT_IO_OPT_SEC_LEVEL,
	BT_IO_OPT_KEY_SIZE,
	BT_IO_OPT_CHANNEL,
	BT_IO_OPT_SOURCE_CHANNEL,
	BT_IO_OPT_DEST_CHANNEL,
	BT_IO_OPT_PSM,
	BT_IO_OPT_CID,
	BT_IO_OPT_MTU,
	BT_IO_OPT_OMTU,
	BT_IO_OPT_IMTU,
	BT_IO_OPT_MASTER,
	BT_IO_OPT_HANDLE,
	BT_IO_OPT_CLASS,
	BT_IO_OPT_MODE,
	BT_IO_OPT_FLUSHABLE,
	BT_IO_OPT_PRIORITY,
	BT_IO_OPT_VOICE,
	BT_IO_OPT_PHY,
} BtIOOption;

typedef enum {
	BT_IO_SEC_SDP = 0,
	BT_IO_SEC_LOW,
	BT_IO_SEC_MEDIUM,
	BT_IO_SEC_HIGH,
} BtIOSecLevel;

typedef enum {
	BT_IO_MODE_BASIC = 0,
	BT_IO_MODE_ERTM,
	BT_IO_MODE_STREAMING,
	BT_IO_MODE_LE_FLOWCTL,
	BT_IO_MODE_EXT_FLOWCTL
} BtIOMode;

typedef void (*BtIOConfirm)(GIOChannel *io, gpointer user_data);

typedef void (*BtIOConnect)(GIOChannel *io, GError *err, gpointer user_data);

gboolean bt_io_accept(GIOChannel *io, BtIOConnect connect, gpointer user_data,
					GDestroyNotify destroy, GError **err);

gboolean bt_io_set(GIOChannel *io, GError **err, BtIOOption opt1, ...);

gboolean bt_io_get(GIOChannel *io, GError **err, BtIOOption opt1, ...);

GIOChannel *bt_io_connect(BtIOConnect connect, gpointer user_data,
				GDestroyNotify destroy, GError **gerr,
				BtIOOption opt1, ...);

GIOChannel *bt_io_listen(BtIOConnect connect, BtIOConfirm confirm,
				gpointer user_data, GDestroyNotify destroy,
				GError **err, BtIOOption opt1, ...);

#endif

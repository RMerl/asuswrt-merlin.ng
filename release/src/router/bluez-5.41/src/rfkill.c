/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
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
#include <config.h>
#endif

#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"

#include "log.h"
#include "adapter.h"
#include "hcid.h"

enum rfkill_type {
	RFKILL_TYPE_ALL = 0,
	RFKILL_TYPE_WLAN,
	RFKILL_TYPE_BLUETOOTH,
	RFKILL_TYPE_UWB,
	RFKILL_TYPE_WIMAX,
	RFKILL_TYPE_WWAN,
};

enum rfkill_operation {
	RFKILL_OP_ADD = 0,
	RFKILL_OP_DEL,
	RFKILL_OP_CHANGE,
	RFKILL_OP_CHANGE_ALL,
};

struct rfkill_event {
	uint32_t idx;
	uint8_t  type;
	uint8_t  op;
	uint8_t  soft;
	uint8_t  hard;
};

static gboolean rfkill_event(GIOChannel *chan,
				GIOCondition cond, gpointer data)
{
	unsigned char buf[32];
	struct rfkill_event *event = (void *) buf;
	struct btd_adapter *adapter;
	char sysname[PATH_MAX];
	ssize_t len;
	int fd, id;

	if (cond & (G_IO_NVAL | G_IO_HUP | G_IO_ERR))
		return FALSE;

	fd = g_io_channel_unix_get_fd(chan);

	memset(buf, 0, sizeof(buf));

	len = read(fd, buf, sizeof(buf));
	if (len < 0) {
		if (errno == EAGAIN)
			return TRUE;
		return FALSE;
	}

	if (len != sizeof(struct rfkill_event))
		return TRUE;

	DBG("RFKILL event idx %u type %u op %u soft %u hard %u",
					event->idx, event->type, event->op,
						event->soft, event->hard);

	if (event->soft || event->hard)
		return TRUE;

	if (event->op != RFKILL_OP_CHANGE)
		return TRUE;

	if (event->type != RFKILL_TYPE_BLUETOOTH &&
					event->type != RFKILL_TYPE_ALL)
		return TRUE;

	snprintf(sysname, sizeof(sysname) - 1,
			"/sys/class/rfkill/rfkill%u/name", event->idx);

	fd = open(sysname, O_RDONLY);
	if (fd < 0)
		return TRUE;

	memset(sysname, 0, sizeof(sysname));

	if (read(fd, sysname, sizeof(sysname) - 1) < 4) {
		close(fd);
		return TRUE;
	}

	close(fd);

	if (g_str_has_prefix(sysname, "hci") == FALSE)
		return TRUE;

	id = atoi(sysname + 3);
	if (id < 0)
		return TRUE;

	adapter = adapter_find_by_id(id);
	if (!adapter)
		return TRUE;

	DBG("RFKILL unblock for hci%d", id);

	btd_adapter_restore_powered(adapter);

	return TRUE;
}

static guint watch = 0;

void rfkill_init(void)
{
	int fd;
	GIOChannel *channel;

	fd = open("/dev/rfkill", O_RDWR);
	if (fd < 0) {
		error("Failed to open RFKILL control device");
		return;
	}

	channel = g_io_channel_unix_new(fd);
	g_io_channel_set_close_on_unref(channel, TRUE);

	watch = g_io_add_watch(channel,
				G_IO_IN | G_IO_NVAL | G_IO_HUP | G_IO_ERR,
				rfkill_event, NULL);

	g_io_channel_unref(channel);
}

void rfkill_exit(void)
{
	if (watch == 0)
		return;

	g_source_remove(watch);
	watch = 0;
}

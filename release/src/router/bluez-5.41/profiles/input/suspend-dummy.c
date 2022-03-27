/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2012  Nordic Semiconductor Inc.
 *  Copyright (C) 2012  Instituto Nokia de Tecnologia - INdT
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <glib.h>

#include "src/log.h"
#include "suspend.h"

#define HOG_SUSPEND_FIFO	"/tmp/hogsuspend"

static suspend_event suspend_cb = NULL;
static resume_event resume_cb = NULL;
static guint watch = 0;

static int fifo_open(void);

static gboolean read_fifo(GIOChannel *io, GIOCondition cond, gpointer user_data)
{
	char buffer[12];
	gsize offset, left, bread;
	GIOStatus iostatus;

	if (cond & (G_IO_ERR | G_IO_HUP)) {
		/*
		 * Both ends needs to be open simultaneously before proceeding
		 * any input or output operation. When the remote closes the
		 * channel, hup signal is received on this end.
		 */
		fifo_open();
		return FALSE;
	}

	offset = 0;
	left = sizeof(buffer) - 1;
	memset(buffer, 0, sizeof(buffer));

	do {
		iostatus = g_io_channel_read_chars(io, &buffer[offset], left,
								&bread, NULL);

		offset += bread;
		left -= bread;
		if (left == 0)
			break;
	} while (iostatus == G_IO_STATUS_NORMAL);

	if (g_ascii_strncasecmp("suspend", buffer, 7) == 0)
		suspend_cb();
	else if (g_ascii_strncasecmp("resume", buffer, 6) == 0)
		resume_cb();

	return TRUE;
}

static int fifo_open(void)
{
	GIOCondition condition = G_IO_IN | G_IO_ERR | G_IO_HUP;
	GIOChannel *fifoio;
	int fd;

	fd = open(HOG_SUSPEND_FIFO, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		int err = -errno;
		error("Can't open FIFO (%s): %s(%d)", HOG_SUSPEND_FIFO,
							strerror(-err), -err);
		return err;
	}

	fifoio = g_io_channel_unix_new(fd);
	g_io_channel_set_close_on_unref(fifoio, TRUE);

	watch = g_io_add_watch(fifoio, condition, read_fifo, NULL);

	g_io_channel_unref(fifoio);

	return 0;
}

int suspend_init(suspend_event suspend, resume_event resume)
{
	struct stat st;
	int ret;

	DBG("");

	suspend_cb = suspend;
	resume_cb = resume;

	if (stat(HOG_SUSPEND_FIFO, &st) == 0) {
		if (!S_ISFIFO(st.st_mode)) {
			error("Unexpected non-FIFO %s file", HOG_SUSPEND_FIFO);
			return -EIO;
		}

		if (unlink(HOG_SUSPEND_FIFO) < 0) {
			int err = -errno;
			error("Failed to remove FIFO (%s): %s (%d)",
				HOG_SUSPEND_FIFO, strerror(-err), -err);
			return err;
		}
	}

	if (mkfifo(HOG_SUSPEND_FIFO, S_IRUSR | S_IWUSR) < 0) {
		int err = -errno;

		error("Can't create FIFO (%s): %s (%d)", HOG_SUSPEND_FIFO,
							strerror(-err), -err);
		return err;
	}

	DBG("Created suspend-dummy FIFO on %s", HOG_SUSPEND_FIFO);

	ret = fifo_open();
	if (ret < 0)
		unlink(HOG_SUSPEND_FIFO);

	return ret;
}

void suspend_exit(void)
{
	if (watch > 0) {
		g_source_remove(watch);
		watch = 0;
	}

	unlink(HOG_SUSPEND_FIFO);
}

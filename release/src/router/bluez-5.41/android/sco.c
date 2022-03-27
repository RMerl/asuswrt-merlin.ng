/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014 Intel Corporation. All rights reserved.
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

#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "btio/btio.h"
#include "src/log.h"
#include "src/shared/util.h"

#include "sco.h"

struct bt_sco {
	int ref_count;

	GIOChannel *server_io;

	GIOChannel *io;
	guint watch;

	bdaddr_t local_addr;
	bdaddr_t remote_addr;

	bt_sco_confirm_func_t confirm_cb;
	bt_sco_conn_func_t connect_cb;
	bt_sco_disconn_func_t disconnect_cb;
};

/* We support only one sco for the moment */
static bool sco_in_use = false;

static void clear_remote_address(struct bt_sco *sco)
{
	memset(&sco->remote_addr, 0, sizeof(bdaddr_t));
}

static gboolean disconnect_watch(GIOChannel *chan, GIOCondition cond,
							gpointer user_data)
{
	struct bt_sco *sco = user_data;

	g_io_channel_shutdown(sco->io, TRUE, NULL);
	g_io_channel_unref(sco->io);
	sco->io = NULL;

	DBG("");

	sco->watch = 0;

	if (sco->disconnect_cb)
		sco->disconnect_cb(&sco->remote_addr);

	clear_remote_address(sco);

	return FALSE;
}

static void connect_sco_cb(GIOChannel *chan, GError *err, gpointer user_data)
{
	struct bt_sco *sco = user_data;

	DBG("");

	/* Lets unref connecting io */
	if (sco->io) {
		g_io_channel_unref(sco->io);
		sco->io = NULL;
	}

	if (err) {
		error("sco: Audio connect failed (%s)", err->message);

		/*
		 * Connect_sco_cb is called only when connect_cb is in place
		 * Therefore it is safe to call it
		 */
		sco->connect_cb(SCO_STATUS_ERROR, &sco->remote_addr);

		clear_remote_address(sco);

		return;
	}

	g_io_channel_set_close_on_unref(chan, TRUE);

	sco->io = g_io_channel_ref(chan);
	sco->watch = g_io_add_watch(chan, G_IO_ERR | G_IO_HUP | G_IO_NVAL,
							disconnect_watch, sco);

	/* It is safe to call it here */
	sco->connect_cb(SCO_STATUS_OK, &sco->remote_addr);
}

static void confirm_sco_cb(GIOChannel *chan, gpointer user_data)
{
	char address[18];
	bdaddr_t bdaddr;
	GError *err = NULL;
	struct bt_sco *sco = user_data;
	uint16_t voice_settings;

	DBG("");

	bt_io_get(chan, &err,
			BT_IO_OPT_DEST, address,
			BT_IO_OPT_DEST_BDADDR, &bdaddr,
			BT_IO_OPT_INVALID);
	if (err) {
		error("sco: audio confirm failed (%s)", err->message);
		g_error_free(err);
		goto drop;
	}

	if (!sco->confirm_cb || !sco->connect_cb) {
		error("sco: Connect and/or confirm callback not registered ");
		goto drop;
	}

	/* Check if there is SCO */
	if (sco->io) {
		error("sco: SCO is in progress");
		goto drop;
	}

	if (!sco->confirm_cb(&bdaddr, &voice_settings)) {
		error("sco: Audio connection from %s rejected", address);
		goto drop;
	}

	bacpy(&sco->remote_addr, &bdaddr);

	DBG("Incoming SCO connection from %s, voice settings 0x%x", address,
								voice_settings);

	err = NULL;
	bt_io_set(chan, &err, BT_IO_OPT_VOICE, voice_settings,
							BT_IO_OPT_INVALID);
	if (err) {
		error("sco: Could not set voice settings (%s)", err->message);
		g_error_free(err);
		goto drop;
	}

	if (!bt_io_accept(chan, connect_sco_cb, sco, NULL, NULL)) {
		error("sco: Failed to accept audio connection");
		goto drop;
	}

	sco->io = g_io_channel_ref(chan);

	return;

drop:
	g_io_channel_shutdown(chan, TRUE, NULL);
}

static bool sco_listen(struct bt_sco *sco)
{
	GError *err = NULL;

	if (!sco)
		return false;

	sco->server_io = bt_io_listen(NULL, confirm_sco_cb, sco, NULL, &err,
							BT_IO_OPT_SOURCE_BDADDR,
							&sco->local_addr,
							BT_IO_OPT_INVALID);
	if (!sco->server_io) {
		error("sco: Failed to listen on SCO: %s", err->message);
		g_error_free(err);
		return false;
	}

	return true;
}

struct bt_sco *bt_sco_new(const bdaddr_t *local_bdaddr)
{
	struct bt_sco *sco;

	if (!local_bdaddr)
		return NULL;

	/* For now we support only one SCO connection per time */
	if (sco_in_use)
		return NULL;

	sco = new0(struct bt_sco, 1);
	if (!sco)
		return NULL;

	bacpy(&sco->local_addr, local_bdaddr);

	if (!sco_listen(sco)) {
		free(sco);
		return NULL;
	}

	sco_in_use  = true;

	return bt_sco_ref(sco);
}

struct bt_sco *bt_sco_ref(struct bt_sco *sco)
{
	if (!sco)
		return NULL;

	__sync_fetch_and_add(&sco->ref_count, 1);

	return sco;
}

static void sco_free(struct bt_sco *sco)
{
	if (sco->server_io) {
		g_io_channel_shutdown(sco->server_io, TRUE, NULL);
		g_io_channel_unref(sco->server_io);
	}

	if (sco->io) {
		g_io_channel_shutdown(sco->io, TRUE, NULL);
		g_io_channel_unref(sco->io);
	}

	g_free(sco);
	sco_in_use  = false;
}

void bt_sco_unref(struct bt_sco *sco)
{
	DBG("");

	if (!sco)
		return;

	if (__sync_sub_and_fetch(&sco->ref_count, 1))
		return;

	sco_free(sco);
}

bool bt_sco_connect(struct bt_sco *sco, const bdaddr_t *addr,
							uint16_t voice_settings)
{
	GIOChannel *io;
	GError *gerr = NULL;

	DBG("");

	if (!sco || !sco->connect_cb || !addr) {
		error("sco: Incorrect parameters or missing connect_cb");
		return false;
	}

	/* Check if we have connection in progress */
	if (sco->io) {
		error("sco: Connection already in progress");
		return false;
	}

	io = bt_io_connect(connect_sco_cb, sco, NULL, &gerr,
				BT_IO_OPT_SOURCE_BDADDR, &sco->local_addr,
				BT_IO_OPT_DEST_BDADDR, addr,
				BT_IO_OPT_VOICE, voice_settings,
				BT_IO_OPT_INVALID);

	if (!io) {
		error("sco: unable to connect audio: %s", gerr->message);
		g_error_free(gerr);
		return false;
	}

	sco->io = io;

	bacpy(&sco->remote_addr, addr);

	return true;
}

void bt_sco_disconnect(struct bt_sco *sco)
{
	if (!sco)
		return;

	if (sco->io)
		g_io_channel_shutdown(sco->io, TRUE, NULL);
}

bool bt_sco_get_fd_and_mtu(struct bt_sco *sco, int *fd, uint16_t *mtu)
{
	GError *err;

	if (!sco->io || !fd || !mtu)
		return false;

	err = NULL;
	if (!bt_io_get(sco->io, &err, BT_IO_OPT_MTU, mtu, BT_IO_OPT_INVALID)) {
			error("Unable to get MTU: %s\n", err->message);
			g_clear_error(&err);
			return false;
		}

	*fd = g_io_channel_unix_get_fd(sco->io);

	return true;
}

void bt_sco_set_confirm_cb(struct bt_sco *sco,
					bt_sco_confirm_func_t func)
{
	sco->confirm_cb = func;
}

void bt_sco_set_connect_cb(struct bt_sco *sco, bt_sco_conn_func_t func)
{
	sco->connect_cb = func;
}

void bt_sco_set_disconnect_cb(struct bt_sco *sco,
						bt_sco_disconn_func_t func)
{
	sco->disconnect_cb = func;
}

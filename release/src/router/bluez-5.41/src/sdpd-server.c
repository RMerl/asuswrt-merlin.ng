/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2001-2002  Nokia Corporation
 *  Copyright (C) 2002-2003  Maxim Krasnyansky <maxk@qualcomm.com>
 *  Copyright (C) 2002-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2002-2003  Stephen Crane <steve.crane@rococosoft.com>
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
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/l2cap.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"

#include "log.h"
#include "sdpd.h"

static guint l2cap_id = 0, unix_id = 0;
static int l2cap_sock = -1, unix_sock = -1;

/*
 * SDP server initialization on startup includes creating the
 * l2cap and unix sockets over which discovery and registration clients
 * access us respectively
 */
static int init_server(uint16_t mtu, int master, int compat)
{
	struct l2cap_options opts;
	struct sockaddr_l2 l2addr;
	struct sockaddr_un unaddr;
	socklen_t optlen;

	/* Register the public browse group root */
	register_public_browse_group();

	/* Register the SDP server's service record */
	register_server_service();

	/* Create L2CAP socket */
	l2cap_sock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (l2cap_sock < 0) {
		error("opening L2CAP socket: %s", strerror(errno));
		return -1;
	}

	memset(&l2addr, 0, sizeof(l2addr));
	l2addr.l2_family = AF_BLUETOOTH;
	bacpy(&l2addr.l2_bdaddr, BDADDR_ANY);
	l2addr.l2_psm = htobs(SDP_PSM);

	if (bind(l2cap_sock, (struct sockaddr *) &l2addr, sizeof(l2addr)) < 0) {
		error("binding L2CAP socket: %s", strerror(errno));
		return -1;
	}

	if (master) {
		int opt = L2CAP_LM_MASTER;
		if (setsockopt(l2cap_sock, SOL_L2CAP, L2CAP_LM, &opt, sizeof(opt)) < 0) {
			error("setsockopt: %s", strerror(errno));
			return -1;
		}
	}

	if (mtu > 0) {
		memset(&opts, 0, sizeof(opts));
		optlen = sizeof(opts);

		if (getsockopt(l2cap_sock, SOL_L2CAP, L2CAP_OPTIONS, &opts, &optlen) < 0) {
			error("getsockopt: %s", strerror(errno));
			return -1;
		}

		opts.omtu = mtu;
		opts.imtu = mtu;

		if (setsockopt(l2cap_sock, SOL_L2CAP, L2CAP_OPTIONS, &opts, sizeof(opts)) < 0) {
			error("setsockopt: %s", strerror(errno));
			return -1;
		}
	}

	if (listen(l2cap_sock, 5) < 0) {
		error("listen: %s", strerror(errno));
		return -1;
	}

	if (!compat) {
		unix_sock = -1;
		return 0;
	}

	/* Create local Unix socket */
	unix_sock = socket(PF_UNIX, SOCK_STREAM, 0);
	if (unix_sock < 0) {
		error("opening UNIX socket: %s", strerror(errno));
		return -1;
	}

	memset(&unaddr, 0, sizeof(unaddr));
	unaddr.sun_family = AF_UNIX;
	strcpy(unaddr.sun_path, SDP_UNIX_PATH);

	unlink(unaddr.sun_path);

	if (bind(unix_sock, (struct sockaddr *) &unaddr, sizeof(unaddr)) < 0) {
		error("binding UNIX socket: %s", strerror(errno));
		return -1;
	}

	if (listen(unix_sock, 5) < 0) {
		error("listen UNIX socket: %s", strerror(errno));
		return -1;
	}

	chmod(SDP_UNIX_PATH, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

	return 0;
}

static gboolean io_session_event(GIOChannel *chan, GIOCondition cond, gpointer data)
{
	sdp_pdu_hdr_t hdr;
	uint8_t *buf;
	int sk, len, size;

	if (cond & G_IO_NVAL)
		return FALSE;

	sk = g_io_channel_unix_get_fd(chan);

	if (cond & (G_IO_HUP | G_IO_ERR)) {
		sdp_svcdb_collect_all(sk);
		return FALSE;
	}

	len = recv(sk, &hdr, sizeof(sdp_pdu_hdr_t), MSG_PEEK);
	if (len != sizeof(sdp_pdu_hdr_t)) {
		sdp_svcdb_collect_all(sk);
		return FALSE;
	}

	size = sizeof(sdp_pdu_hdr_t) + ntohs(hdr.plen);
	buf = malloc(size);
	if (!buf)
		return TRUE;

	len = recv(sk, buf, size, 0);
	/* Check here only that the received message is not empty.
	 * Incorrect length of message should be processed later
	 * inside handle_request() in order to produce ErrorResponse.
	 */
	if (len <= 0) {
		sdp_svcdb_collect_all(sk);
		free(buf);
		return FALSE;
	}

	handle_request(sk, buf, len);

	return TRUE;
}

static gboolean io_accept_event(GIOChannel *chan, GIOCondition cond, gpointer data)
{
	GIOChannel *io;
	int nsk;

	if (cond & (G_IO_HUP | G_IO_ERR | G_IO_NVAL))
		return FALSE;

	if (data == &l2cap_sock) {
		struct sockaddr_l2 addr;
		socklen_t len = sizeof(addr);

		nsk = accept(l2cap_sock, (struct sockaddr *) &addr, &len);
	} else if (data == &unix_sock) {
		struct sockaddr_un addr;
		socklen_t len = sizeof(addr);

		nsk = accept(unix_sock, (struct sockaddr *) &addr, &len);
	} else
		return FALSE;

	if (nsk < 0) {
		error("Can't accept connection: %s", strerror(errno));
		return TRUE;
	}

	io = g_io_channel_unix_new(nsk);
	g_io_channel_set_close_on_unref(io, TRUE);

	g_io_add_watch(io, G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
					io_session_event, data);

	g_io_channel_unref(io);

	return TRUE;
}

int start_sdp_server(uint16_t mtu, uint32_t flags)
{
	int compat = flags & SDP_SERVER_COMPAT;
	int master = flags & SDP_SERVER_MASTER;
	GIOChannel *io;

	info("Starting SDP server");

	if (init_server(mtu, master, compat) < 0) {
		error("Server initialization failed");
		return -1;
	}

	io = g_io_channel_unix_new(l2cap_sock);
	g_io_channel_set_close_on_unref(io, TRUE);

	l2cap_id = g_io_add_watch(io, G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
					io_accept_event, &l2cap_sock);
	g_io_channel_unref(io);

	if (compat && unix_sock > fileno(stderr)) {
		io = g_io_channel_unix_new(unix_sock);
		g_io_channel_set_close_on_unref(io, TRUE);

		unix_id = g_io_add_watch(io,
					G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
					io_accept_event, &unix_sock);
		g_io_channel_unref(io);
	}

	return 0;
}

void stop_sdp_server(void)
{
	info("Stopping SDP server");

	sdp_svcdb_reset();

	if (unix_id > 0)
		g_source_remove(unix_id);

	if (l2cap_id > 0)
		g_source_remove(l2cap_id);

	l2cap_id = unix_id = 0;
	l2cap_sock = unix_sock = -1;
}

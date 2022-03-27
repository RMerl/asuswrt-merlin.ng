/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2009-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2009-2010  Nokia Corporation
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

#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/l2cap.h"
#include "lib/rfcomm.h"
#include "lib/sco.h"

#include "btio.h"

#ifndef BT_FLUSHABLE
#define BT_FLUSHABLE	8
#endif

#define ERROR_FAILED(gerr, str, err) \
		g_set_error(gerr, BT_IO_ERROR, err, \
				str ": %s (%d)", strerror(err), err)

#define DEFAULT_DEFER_TIMEOUT 30

typedef enum {
	BT_IO_L2CAP,
	BT_IO_RFCOMM,
	BT_IO_SCO,
	BT_IO_INVALID,
} BtIOType;

struct set_opts {
	bdaddr_t src;
	bdaddr_t dst;
	BtIOType type;
	uint8_t src_type;
	uint8_t dst_type;
	int defer;
	int sec_level;
	uint8_t channel;
	uint16_t psm;
	uint16_t cid;
	uint16_t mtu;
	uint16_t imtu;
	uint16_t omtu;
	int master;
	uint8_t mode;
	int flushable;
	uint32_t priority;
	uint16_t voice;
};

struct connect {
	BtIOConnect connect;
	gpointer user_data;
	GDestroyNotify destroy;
};

struct accept {
	BtIOConnect connect;
	gpointer user_data;
	GDestroyNotify destroy;
};

struct server {
	BtIOConnect connect;
	BtIOConfirm confirm;
	gpointer user_data;
	GDestroyNotify destroy;
};

static BtIOType bt_io_get_type(GIOChannel *io, GError **gerr)
{
	int sk = g_io_channel_unix_get_fd(io);
	int domain, proto, err;
	socklen_t len;

	domain = 0;
	len = sizeof(domain);
	err = getsockopt(sk, SOL_SOCKET, SO_DOMAIN, &domain, &len);
	if (err < 0) {
		ERROR_FAILED(gerr, "getsockopt(SO_DOMAIN)", errno);
		return BT_IO_INVALID;
	}

	if (domain != AF_BLUETOOTH) {
		g_set_error(gerr, BT_IO_ERROR, EINVAL,
				"BtIO socket domain not AF_BLUETOOTH");
		return BT_IO_INVALID;
	}

	proto = 0;
	len = sizeof(proto);
	err = getsockopt(sk, SOL_SOCKET, SO_PROTOCOL, &proto, &len);
	if (err < 0) {
		ERROR_FAILED(gerr, "getsockopt(SO_PROTOCOL)", errno);
		return BT_IO_INVALID;
	}

	switch (proto) {
	case BTPROTO_RFCOMM:
		return BT_IO_RFCOMM;
	case BTPROTO_SCO:
		return BT_IO_SCO;
	case BTPROTO_L2CAP:
		return BT_IO_L2CAP;
	default:
		g_set_error(gerr, BT_IO_ERROR, EINVAL,
					"Unknown BtIO socket type");
		return BT_IO_INVALID;
	}
}

static void server_remove(struct server *server)
{
	if (server->destroy)
		server->destroy(server->user_data);
	g_free(server);
}

static void connect_remove(struct connect *conn)
{
	if (conn->destroy)
		conn->destroy(conn->user_data);
	g_free(conn);
}

static void accept_remove(struct accept *accept)
{
	if (accept->destroy)
		accept->destroy(accept->user_data);
	g_free(accept);
}

static gboolean check_nval(GIOChannel *io)
{
	struct pollfd fds;

	memset(&fds, 0, sizeof(fds));
	fds.fd = g_io_channel_unix_get_fd(io);
	fds.events = POLLNVAL;

	if (poll(&fds, 1, 0) > 0 && (fds.revents & POLLNVAL))
		return TRUE;

	return FALSE;
}

static gboolean accept_cb(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{
	struct accept *accept = user_data;
	GError *gerr = NULL;

	/* If the user aborted this accept attempt */
	if ((cond & G_IO_NVAL) || check_nval(io))
		return FALSE;

	if (cond & (G_IO_HUP | G_IO_ERR)) {
		int err, sk_err, sock = g_io_channel_unix_get_fd(io);
		socklen_t len = sizeof(sk_err);

		if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &sk_err, &len) < 0)
			err = -errno;
		else
			err = -sk_err;

		if (err < 0)
			ERROR_FAILED(&gerr, "HUP or ERR on socket", -err);
	}

	accept->connect(io, gerr, accept->user_data);

	g_clear_error(&gerr);

	return FALSE;
}

static gboolean connect_cb(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{
	struct connect *conn = user_data;
	GError *gerr = NULL;
	int err, sk_err, sock;
	socklen_t len = sizeof(sk_err);

	/* If the user aborted this connect attempt */
	if ((cond & G_IO_NVAL) || check_nval(io))
		return FALSE;

	sock = g_io_channel_unix_get_fd(io);

	if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &sk_err, &len) < 0)
		err = -errno;
	else
		err = -sk_err;

	if (err < 0)
		ERROR_FAILED(&gerr, "connect error", -err);

	conn->connect(io, gerr, conn->user_data);

	g_clear_error(&gerr);

	return FALSE;
}

static gboolean server_cb(GIOChannel *io, GIOCondition cond,
							gpointer user_data)
{
	struct server *server = user_data;
	int srv_sock, cli_sock;
	GIOChannel *cli_io;

	/* If the user closed the server */
	if ((cond & G_IO_NVAL) || check_nval(io))
		return FALSE;

	srv_sock = g_io_channel_unix_get_fd(io);

	cli_sock = accept(srv_sock, NULL, NULL);
	if (cli_sock < 0)
		return TRUE;

	cli_io = g_io_channel_unix_new(cli_sock);

	g_io_channel_set_close_on_unref(cli_io, TRUE);
	g_io_channel_set_flags(cli_io, G_IO_FLAG_NONBLOCK, NULL);

	if (server->confirm)
		server->confirm(cli_io, server->user_data);
	else
		server->connect(cli_io, NULL, server->user_data);

	g_io_channel_unref(cli_io);

	return TRUE;
}

static void server_add(GIOChannel *io, BtIOConnect connect,
				BtIOConfirm confirm, gpointer user_data,
				GDestroyNotify destroy)
{
	struct server *server;
	GIOCondition cond;

	server = g_new0(struct server, 1);
	server->connect = connect;
	server->confirm = confirm;
	server->user_data = user_data;
	server->destroy = destroy;

	cond = G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL;
	g_io_add_watch_full(io, G_PRIORITY_DEFAULT, cond, server_cb, server,
					(GDestroyNotify) server_remove);
}

static void connect_add(GIOChannel *io, BtIOConnect connect,
				gpointer user_data, GDestroyNotify destroy)
{
	struct connect *conn;
	GIOCondition cond;

	conn = g_new0(struct connect, 1);
	conn->connect = connect;
	conn->user_data = user_data;
	conn->destroy = destroy;

	cond = G_IO_OUT | G_IO_ERR | G_IO_HUP | G_IO_NVAL;
	g_io_add_watch_full(io, G_PRIORITY_DEFAULT, cond, connect_cb, conn,
					(GDestroyNotify) connect_remove);
}

static void accept_add(GIOChannel *io, BtIOConnect connect, gpointer user_data,
							GDestroyNotify destroy)
{
	struct accept *accept;
	GIOCondition cond;

	accept = g_new0(struct accept, 1);
	accept->connect = connect;
	accept->user_data = user_data;
	accept->destroy = destroy;

	cond = G_IO_OUT | G_IO_ERR | G_IO_HUP | G_IO_NVAL;
	g_io_add_watch_full(io, G_PRIORITY_DEFAULT, cond, accept_cb, accept,
					(GDestroyNotify) accept_remove);
}

static int l2cap_bind(int sock, const bdaddr_t *src, uint8_t src_type,
				uint16_t psm, uint16_t cid, GError **err)
{
	struct sockaddr_l2 addr;

	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, src);

	if (cid)
		addr.l2_cid = htobs(cid);
	else
		addr.l2_psm = htobs(psm);

	addr.l2_bdaddr_type = src_type;

	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		int error = -errno;
		ERROR_FAILED(err, "l2cap_bind", errno);
		return error;
	}

	return 0;
}

static int l2cap_connect(int sock, const bdaddr_t *dst, uint8_t dst_type,
						uint16_t psm, uint16_t cid)
{
	int err;
	struct sockaddr_l2 addr;

	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, dst);
	if (cid)
		addr.l2_cid = htobs(cid);
	else
		addr.l2_psm = htobs(psm);

	addr.l2_bdaddr_type = dst_type;

	err = connect(sock, (struct sockaddr *) &addr, sizeof(addr));
	if (err < 0 && !(errno == EAGAIN || errno == EINPROGRESS))
		return -errno;

	return 0;
}

static int l2cap_set_master(int sock, int master)
{
	int flags;
	socklen_t len;

	len = sizeof(flags);
	if (getsockopt(sock, SOL_L2CAP, L2CAP_LM, &flags, &len) < 0)
		return -errno;

	if (master) {
		if (flags & L2CAP_LM_MASTER)
			return 0;
		flags |= L2CAP_LM_MASTER;
	} else {
		if (!(flags & L2CAP_LM_MASTER))
			return 0;
		flags &= ~L2CAP_LM_MASTER;
	}

	if (setsockopt(sock, SOL_L2CAP, L2CAP_LM, &flags, sizeof(flags)) < 0)
		return -errno;

	return 0;
}

static int rfcomm_set_master(int sock, int master)
{
	int flags;
	socklen_t len;

	len = sizeof(flags);
	if (getsockopt(sock, SOL_RFCOMM, RFCOMM_LM, &flags, &len) < 0)
		return -errno;

	if (master) {
		if (flags & RFCOMM_LM_MASTER)
			return 0;
		flags |= RFCOMM_LM_MASTER;
	} else {
		if (!(flags & RFCOMM_LM_MASTER))
			return 0;
		flags &= ~RFCOMM_LM_MASTER;
	}

	if (setsockopt(sock, SOL_RFCOMM, RFCOMM_LM, &flags, sizeof(flags)) < 0)
		return -errno;

	return 0;
}

static int l2cap_set_lm(int sock, int level)
{
	int lm_map[] = {
		0,
		L2CAP_LM_AUTH,
		L2CAP_LM_AUTH | L2CAP_LM_ENCRYPT,
		L2CAP_LM_AUTH | L2CAP_LM_ENCRYPT | L2CAP_LM_SECURE,
	}, opt = lm_map[level];

	if (setsockopt(sock, SOL_L2CAP, L2CAP_LM, &opt, sizeof(opt)) < 0)
		return -errno;

	return 0;
}

static int rfcomm_set_lm(int sock, int level)
{
	int lm_map[] = {
		0,
		RFCOMM_LM_AUTH,
		RFCOMM_LM_AUTH | RFCOMM_LM_ENCRYPT,
		RFCOMM_LM_AUTH | RFCOMM_LM_ENCRYPT | RFCOMM_LM_SECURE,
	}, opt = lm_map[level];

	if (setsockopt(sock, SOL_RFCOMM, RFCOMM_LM, &opt, sizeof(opt)) < 0)
		return -errno;

	return 0;
}

static gboolean set_sec_level(int sock, BtIOType type, int level, GError **err)
{
	struct bt_security sec;
	int ret;

	if (level < BT_SECURITY_LOW || level > BT_SECURITY_HIGH) {
		g_set_error(err, BT_IO_ERROR, EINVAL,
				"Valid security level range is %d-%d",
				BT_SECURITY_LOW, BT_SECURITY_HIGH);
		return FALSE;
	}

	memset(&sec, 0, sizeof(sec));
	sec.level = level;

	if (setsockopt(sock, SOL_BLUETOOTH, BT_SECURITY, &sec,
							sizeof(sec)) == 0)
		return TRUE;

	if (errno != ENOPROTOOPT) {
		ERROR_FAILED(err, "setsockopt(BT_SECURITY)", errno);
		return FALSE;
	}

	if (type == BT_IO_L2CAP)
		ret = l2cap_set_lm(sock, level);
	else
		ret = rfcomm_set_lm(sock, level);

	if (ret < 0) {
		ERROR_FAILED(err, "setsockopt(LM)", -ret);
		return FALSE;
	}

	return TRUE;
}

static int l2cap_get_lm(int sock, int *sec_level)
{
	int opt;
	socklen_t len;

	len = sizeof(opt);
	if (getsockopt(sock, SOL_L2CAP, L2CAP_LM, &opt, &len) < 0)
		return -errno;

	*sec_level = 0;

	if (opt & L2CAP_LM_AUTH)
		*sec_level = BT_SECURITY_LOW;
	if (opt & L2CAP_LM_ENCRYPT)
		*sec_level = BT_SECURITY_MEDIUM;
	if (opt & L2CAP_LM_SECURE)
		*sec_level = BT_SECURITY_HIGH;

	return 0;
}

static int rfcomm_get_lm(int sock, int *sec_level)
{
	int opt;
	socklen_t len;

	len = sizeof(opt);
	if (getsockopt(sock, SOL_RFCOMM, RFCOMM_LM, &opt, &len) < 0)
		return -errno;

	*sec_level = 0;

	if (opt & RFCOMM_LM_AUTH)
		*sec_level = BT_SECURITY_LOW;
	if (opt & RFCOMM_LM_ENCRYPT)
		*sec_level = BT_SECURITY_MEDIUM;
	if (opt & RFCOMM_LM_SECURE)
		*sec_level = BT_SECURITY_HIGH;

	return 0;
}

static gboolean get_sec_level(int sock, BtIOType type, int *level,
								GError **err)
{
	struct bt_security sec;
	socklen_t len;
	int ret;

	memset(&sec, 0, sizeof(sec));
	len = sizeof(sec);
	if (getsockopt(sock, SOL_BLUETOOTH, BT_SECURITY, &sec, &len) == 0) {
		*level = sec.level;
		return TRUE;
	}

	if (errno != ENOPROTOOPT) {
		ERROR_FAILED(err, "getsockopt(BT_SECURITY)", errno);
		return FALSE;
	}

	if (type == BT_IO_L2CAP)
		ret = l2cap_get_lm(sock, level);
	else
		ret = rfcomm_get_lm(sock, level);

	if (ret < 0) {
		ERROR_FAILED(err, "getsockopt(LM)", -ret);
		return FALSE;
	}

	return TRUE;
}

static int l2cap_set_flushable(int sock, gboolean flushable)
{
	int f;

	f = flushable;
	if (setsockopt(sock, SOL_BLUETOOTH, BT_FLUSHABLE, &f, sizeof(f)) < 0)
		return -errno;

	return 0;
}

static int set_priority(int sock, uint32_t prio)
{
	if (setsockopt(sock, SOL_SOCKET, SO_PRIORITY, &prio, sizeof(prio)) < 0)
		return -errno;

	return 0;
}

static gboolean get_key_size(int sock, int *size, GError **err)
{
	struct bt_security sec;
	socklen_t len;

	memset(&sec, 0, sizeof(sec));
	len = sizeof(sec);
	if (getsockopt(sock, SOL_BLUETOOTH, BT_SECURITY, &sec, &len) == 0) {
		*size = sec.key_size;
		return TRUE;
	}

	return FALSE;
}

static gboolean set_l2opts(int sock, uint16_t imtu, uint16_t omtu,
						uint8_t mode, GError **err)
{
	struct l2cap_options l2o;
	socklen_t len;

	memset(&l2o, 0, sizeof(l2o));
	len = sizeof(l2o);
	if (getsockopt(sock, SOL_L2CAP, L2CAP_OPTIONS, &l2o, &len) < 0) {
		ERROR_FAILED(err, "getsockopt(L2CAP_OPTIONS)", errno);
		return FALSE;
	}

	if (imtu)
		l2o.imtu = imtu;
	if (omtu)
		l2o.omtu = omtu;
	if (mode)
		l2o.mode = mode;

	if (setsockopt(sock, SOL_L2CAP, L2CAP_OPTIONS, &l2o, sizeof(l2o)) < 0) {
		ERROR_FAILED(err, "setsockopt(L2CAP_OPTIONS)", errno);
		return FALSE;
	}

	return TRUE;
}

static gboolean set_le_imtu(int sock, uint16_t imtu, GError **err)
{
	if (setsockopt(sock, SOL_BLUETOOTH, BT_RCVMTU, &imtu,
							sizeof(imtu)) < 0) {
		ERROR_FAILED(err, "setsockopt(BT_RCVMTU)", errno);
		return FALSE;
	}

	return TRUE;
}

static gboolean l2cap_set(int sock, uint8_t src_type, int sec_level,
				uint16_t imtu, uint16_t omtu, uint8_t mode,
				int master, int flushable, uint32_t priority,
				GError **err)
{
	if (imtu || omtu || mode) {
		gboolean ret;

		if (src_type == BDADDR_BREDR)
			ret = set_l2opts(sock, imtu, omtu, mode, err);
		else
			ret = set_le_imtu(sock, imtu, err);

		if (!ret)
			return ret;
	}

	if (master >= 0 && l2cap_set_master(sock, master) < 0) {
		ERROR_FAILED(err, "l2cap_set_master", errno);
		return FALSE;
	}

	if (flushable >= 0 && l2cap_set_flushable(sock, flushable) < 0) {
		ERROR_FAILED(err, "l2cap_set_flushable", errno);
		return FALSE;
	}

	if (priority > 0 && set_priority(sock, priority) < 0) {
		ERROR_FAILED(err, "set_priority", errno);
		return FALSE;
	}

	if (sec_level && !set_sec_level(sock, BT_IO_L2CAP, sec_level, err))
		return FALSE;

	return TRUE;
}

static int rfcomm_bind(int sock,
		const bdaddr_t *src, uint8_t channel, GError **err)
{
	struct sockaddr_rc addr;

	memset(&addr, 0, sizeof(addr));
	addr.rc_family = AF_BLUETOOTH;
	bacpy(&addr.rc_bdaddr, src);
	addr.rc_channel = channel;

	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		int error = -errno;
		ERROR_FAILED(err, "rfcomm_bind", errno);
		return error;
	}

	return 0;
}

static int rfcomm_connect(int sock, const bdaddr_t *dst, uint8_t channel)
{
	int err;
	struct sockaddr_rc addr;

	memset(&addr, 0, sizeof(addr));
	addr.rc_family = AF_BLUETOOTH;
	bacpy(&addr.rc_bdaddr, dst);
	addr.rc_channel = channel;

	err = connect(sock, (struct sockaddr *) &addr, sizeof(addr));
	if (err < 0 && !(errno == EAGAIN || errno == EINPROGRESS))
		return -errno;

	return 0;
}

static gboolean rfcomm_set(int sock, int sec_level, int master, GError **err)
{
	if (sec_level && !set_sec_level(sock, BT_IO_RFCOMM, sec_level, err))
		return FALSE;

	if (master >= 0 && rfcomm_set_master(sock, master) < 0) {
		ERROR_FAILED(err, "rfcomm_set_master", errno);
		return FALSE;
	}

	return TRUE;
}

static int sco_bind(int sock, const bdaddr_t *src, GError **err)
{
	struct sockaddr_sco addr;

	memset(&addr, 0, sizeof(addr));
	addr.sco_family = AF_BLUETOOTH;
	bacpy(&addr.sco_bdaddr, src);

	if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		int error = -errno;
		ERROR_FAILED(err, "sco_bind", errno);
		return error;
	}

	return 0;
}

static int sco_connect(int sock, const bdaddr_t *dst)
{
	struct sockaddr_sco addr;
	int err;

	memset(&addr, 0, sizeof(addr));
	addr.sco_family = AF_BLUETOOTH;
	bacpy(&addr.sco_bdaddr, dst);

	err = connect(sock, (struct sockaddr *) &addr, sizeof(addr));
	if (err < 0 && !(errno == EAGAIN || errno == EINPROGRESS))
		return -errno;

	return 0;
}

static gboolean sco_set(int sock, uint16_t mtu, uint16_t voice, GError **err)
{
	struct sco_options sco_opt;
	struct bt_voice bt_voice;
	socklen_t len;

	if (!mtu)
		goto voice;

	len = sizeof(sco_opt);
	memset(&sco_opt, 0, len);
	if (getsockopt(sock, SOL_SCO, SCO_OPTIONS, &sco_opt, &len) < 0) {
		ERROR_FAILED(err, "getsockopt(SCO_OPTIONS)", errno);
		return FALSE;
	}

	sco_opt.mtu = mtu;
	if (setsockopt(sock, SOL_SCO, SCO_OPTIONS, &sco_opt,
						sizeof(sco_opt)) < 0) {
		ERROR_FAILED(err, "setsockopt(SCO_OPTIONS)", errno);
		return FALSE;
	}

voice:
	if (!voice)
		return TRUE;

	memset(&bt_voice, 0, sizeof(bt_voice));
	bt_voice.setting = voice;
	if (setsockopt(sock, SOL_BLUETOOTH, BT_VOICE, &bt_voice,
						sizeof(bt_voice)) < 0) {
		ERROR_FAILED(err, "setsockopt(BT_VOICE)", errno);
		return FALSE;
	}

	return TRUE;
}

static gboolean parse_set_opts(struct set_opts *opts, GError **err,
						BtIOOption opt1, va_list args)
{
	BtIOOption opt = opt1;
	const char *str;

	memset(opts, 0, sizeof(*opts));

	/* Set defaults */
	opts->type = BT_IO_SCO;
	opts->defer = DEFAULT_DEFER_TIMEOUT;
	opts->master = -1;
	opts->mode = L2CAP_MODE_BASIC;
	opts->flushable = -1;
	opts->priority = 0;
	opts->src_type = BDADDR_BREDR;
	opts->dst_type = BDADDR_BREDR;

	while (opt != BT_IO_OPT_INVALID) {
		switch (opt) {
		case BT_IO_OPT_SOURCE:
			str = va_arg(args, const char *);
			str2ba(str, &opts->src);
			break;
		case BT_IO_OPT_SOURCE_BDADDR:
			bacpy(&opts->src, va_arg(args, const bdaddr_t *));
			break;
		case BT_IO_OPT_SOURCE_TYPE:
			opts->src_type = va_arg(args, int);
			break;
		case BT_IO_OPT_DEST:
			str2ba(va_arg(args, const char *), &opts->dst);
			break;
		case BT_IO_OPT_DEST_BDADDR:
			bacpy(&opts->dst, va_arg(args, const bdaddr_t *));
			break;
		case BT_IO_OPT_DEST_TYPE:
			opts->dst_type = va_arg(args, int);
			break;
		case BT_IO_OPT_DEFER_TIMEOUT:
			opts->defer = va_arg(args, int);
			break;
		case BT_IO_OPT_SEC_LEVEL:
			opts->sec_level = va_arg(args, int);
			break;
		case BT_IO_OPT_CHANNEL:
			opts->type = BT_IO_RFCOMM;
			opts->channel = va_arg(args, int);
			break;
		case BT_IO_OPT_PSM:
			opts->type = BT_IO_L2CAP;
			opts->psm = va_arg(args, int);
			break;
		case BT_IO_OPT_CID:
			opts->type = BT_IO_L2CAP;
			opts->cid = va_arg(args, int);
			break;
		case BT_IO_OPT_MTU:
			opts->mtu = va_arg(args, int);
			opts->imtu = opts->mtu;
			opts->omtu = opts->mtu;
			break;
		case BT_IO_OPT_OMTU:
			opts->omtu = va_arg(args, int);
			if (!opts->mtu)
				opts->mtu = opts->omtu;
			break;
		case BT_IO_OPT_IMTU:
			opts->imtu = va_arg(args, int);
			if (!opts->mtu)
				opts->mtu = opts->imtu;
			break;
		case BT_IO_OPT_MASTER:
			opts->master = va_arg(args, gboolean);
			break;
		case BT_IO_OPT_MODE:
			opts->mode = va_arg(args, int);
			break;
		case BT_IO_OPT_FLUSHABLE:
			opts->flushable = va_arg(args, gboolean);
			break;
		case BT_IO_OPT_PRIORITY:
			opts->priority = va_arg(args, int);
			break;
		case BT_IO_OPT_VOICE:
			opts->voice = va_arg(args, int);
			break;
		case BT_IO_OPT_INVALID:
		case BT_IO_OPT_KEY_SIZE:
		case BT_IO_OPT_SOURCE_CHANNEL:
		case BT_IO_OPT_DEST_CHANNEL:
		case BT_IO_OPT_HANDLE:
		case BT_IO_OPT_CLASS:
		default:
			g_set_error(err, BT_IO_ERROR, EINVAL,
					"Unknown option %d", opt);
			return FALSE;
		}

		opt = va_arg(args, int);
	}

	return TRUE;
}

static gboolean get_src(int sock, void *src, socklen_t len, GError **err)
{
	socklen_t olen;

	memset(src, 0, len);
	olen = len;
	if (getsockname(sock, src, &olen) < 0) {
		ERROR_FAILED(err, "getsockname", errno);
		return FALSE;
	}

	return TRUE;
}

static gboolean get_dst(int sock, void *dst, socklen_t len, GError **err)
{
	socklen_t olen;

	memset(dst, 0, len);
	olen = len;
	if (getpeername(sock, dst, &olen) < 0) {
		ERROR_FAILED(err, "getpeername", errno);
		return FALSE;
	}

	return TRUE;
}

static int l2cap_get_info(int sock, uint16_t *handle, uint8_t *dev_class)
{
	struct l2cap_conninfo info;
	socklen_t len;

	len = sizeof(info);
	if (getsockopt(sock, SOL_L2CAP, L2CAP_CONNINFO, &info, &len) < 0)
		return -errno;

	if (handle)
		*handle = info.hci_handle;

	if (dev_class)
		memcpy(dev_class, info.dev_class, 3);

	return 0;
}

static int l2cap_get_flushable(int sock, gboolean *flushable)
{
	int f;
	socklen_t len;

	f = 0;
	len = sizeof(f);
	if (getsockopt(sock, SOL_BLUETOOTH, BT_FLUSHABLE, &f, &len) < 0)
		return -errno;

	if (f)
		*flushable = TRUE;
	else
		*flushable = FALSE;

	return 0;
}

static int get_priority(int sock, uint32_t *prio)
{
	socklen_t len;

	len = sizeof(*prio);
	if (getsockopt(sock, SOL_SOCKET, SO_PRIORITY, prio, &len) < 0)
		return -errno;

	return 0;
}

static gboolean l2cap_get(int sock, GError **err, BtIOOption opt1,
								va_list args)
{
	BtIOOption opt = opt1;
	struct sockaddr_l2 src, dst;
	struct l2cap_options l2o;
	int flags;
	uint8_t dev_class[3];
	uint16_t handle = 0;
	socklen_t len;
	gboolean flushable = FALSE, have_dst = FALSE;
	uint32_t priority;

	if (!get_src(sock, &src, sizeof(src), err))
		return FALSE;

	memset(&l2o, 0, sizeof(l2o));

	if (src.l2_bdaddr_type != BDADDR_BREDR) {
		len = sizeof(l2o.imtu);
		if (getsockopt(sock, SOL_BLUETOOTH, BT_RCVMTU,
						&l2o.imtu, &len) == 0)
			goto parse_opts;

		/* Non-LE CoC enabled kernels will return one of these
		 * in which case we need to fall back to L2CAP_OPTIONS.
		 */
		if (errno != EPROTONOSUPPORT && errno != ENOPROTOOPT) {
			ERROR_FAILED(err, "getsockopt(BT_RCVMTU)", errno);
			return FALSE;
		}
	}

	len = sizeof(l2o);
	if (getsockopt(sock, SOL_L2CAP, L2CAP_OPTIONS, &l2o, &len) < 0) {
		ERROR_FAILED(err, "getsockopt(L2CAP_OPTIONS)", errno);
		return FALSE;
	}

parse_opts:
	while (opt != BT_IO_OPT_INVALID) {
		switch (opt) {
		case BT_IO_OPT_SOURCE:
			ba2str(&src.l2_bdaddr, va_arg(args, char *));
			break;
		case BT_IO_OPT_SOURCE_BDADDR:
			bacpy(va_arg(args, bdaddr_t *), &src.l2_bdaddr);
			break;
		case BT_IO_OPT_DEST:
			if (!have_dst)
				have_dst = get_dst(sock, &dst, sizeof(dst),
									err);
			if (!have_dst)
				return FALSE;
			ba2str(&dst.l2_bdaddr, va_arg(args, char *));
			break;
		case BT_IO_OPT_DEST_BDADDR:
			if (!have_dst)
				have_dst = get_dst(sock, &dst, sizeof(dst),
									err);
			if (!have_dst)
				return FALSE;
			bacpy(va_arg(args, bdaddr_t *), &dst.l2_bdaddr);
			break;
		case BT_IO_OPT_DEST_TYPE:
			if (!have_dst)
				have_dst = get_dst(sock, &dst, sizeof(dst),
									err);
			if (!have_dst)
				return FALSE;
			*(va_arg(args, uint8_t *)) = dst.l2_bdaddr_type;
			break;
		case BT_IO_OPT_DEFER_TIMEOUT:
			len = sizeof(int);
			if (getsockopt(sock, SOL_BLUETOOTH, BT_DEFER_SETUP,
					va_arg(args, int *), &len) < 0) {
				ERROR_FAILED(err, "getsockopt(DEFER_SETUP)",
									errno);
				return FALSE;
			}
			break;
		case BT_IO_OPT_SEC_LEVEL:
			if (!get_sec_level(sock, BT_IO_L2CAP,
						va_arg(args, int *), err))
				return FALSE;
			break;
		case BT_IO_OPT_KEY_SIZE:
			if (!get_key_size(sock, va_arg(args, int *), err))
				return FALSE;
			break;
		case BT_IO_OPT_PSM:
			if (src.l2_psm) {
				*(va_arg(args, uint16_t *)) = btohs(src.l2_psm);
				break;
			}

			if (!have_dst)
				have_dst = get_dst(sock, &dst, sizeof(dst),
									err);
			if (!have_dst)
				return FALSE;

			*(va_arg(args, uint16_t *)) = btohs(dst.l2_psm);
			break;
		case BT_IO_OPT_CID:
			if (src.l2_cid) {
				*(va_arg(args, uint16_t *)) = btohs(src.l2_cid);
				break;
			}

			if (!have_dst)
				have_dst = get_dst(sock, &dst, sizeof(dst),
									err);
			if (!have_dst)
				return FALSE;

			*(va_arg(args, uint16_t *)) = btohs(dst.l2_cid);
			break;
		case BT_IO_OPT_OMTU:
			if (src.l2_bdaddr_type == BDADDR_BREDR) {
				*(va_arg(args, uint16_t *)) = l2o.omtu;
				break;
			}

			len = sizeof(l2o.omtu);
			if (getsockopt(sock, SOL_BLUETOOTH, BT_SNDMTU,
							&l2o.omtu, &len) < 0) {
				ERROR_FAILED(err, "getsockopt(BT_SNDMTU)",
									errno);
				return FALSE;
			}

			*(va_arg(args, uint16_t *)) = l2o.omtu;
			break;
		case BT_IO_OPT_IMTU:
			*(va_arg(args, uint16_t *)) = l2o.imtu;
			break;
		case BT_IO_OPT_MASTER:
			len = sizeof(flags);
			if (getsockopt(sock, SOL_L2CAP, L2CAP_LM, &flags,
								&len) < 0) {
				ERROR_FAILED(err, "getsockopt(L2CAP_LM)",
									errno);
				return FALSE;
			}
			*(va_arg(args, gboolean *)) =
				(flags & L2CAP_LM_MASTER) ? TRUE : FALSE;
			break;
		case BT_IO_OPT_HANDLE:
			if (l2cap_get_info(sock, &handle, dev_class) < 0) {
				ERROR_FAILED(err, "L2CAP_CONNINFO", errno);
				return FALSE;
			}
			*(va_arg(args, uint16_t *)) = handle;
			break;
		case BT_IO_OPT_CLASS:
			if (l2cap_get_info(sock, &handle, dev_class) < 0) {
				ERROR_FAILED(err, "L2CAP_CONNINFO", errno);
				return FALSE;
			}
			memcpy(va_arg(args, uint8_t *), dev_class, 3);
			break;
		case BT_IO_OPT_MODE:
			*(va_arg(args, uint8_t *)) = l2o.mode;
			break;
		case BT_IO_OPT_FLUSHABLE:
			if (l2cap_get_flushable(sock, &flushable) < 0) {
				ERROR_FAILED(err, "get_flushable", errno);
				return FALSE;
			}
			*(va_arg(args, gboolean *)) = flushable;
			break;
		case BT_IO_OPT_PRIORITY:
			if (get_priority(sock, &priority) < 0) {
				ERROR_FAILED(err, "get_priority", errno);
				return FALSE;
			}
			*(va_arg(args, uint32_t *)) = priority;
			break;
		case BT_IO_OPT_INVALID:
		case BT_IO_OPT_SOURCE_TYPE:
		case BT_IO_OPT_CHANNEL:
		case BT_IO_OPT_SOURCE_CHANNEL:
		case BT_IO_OPT_DEST_CHANNEL:
		case BT_IO_OPT_MTU:
		case BT_IO_OPT_VOICE:
		default:
			g_set_error(err, BT_IO_ERROR, EINVAL,
					"Unknown option %d", opt);
			return FALSE;
		}

		opt = va_arg(args, int);
	}

	return TRUE;
}

static int rfcomm_get_info(int sock, uint16_t *handle, uint8_t *dev_class)
{
	struct rfcomm_conninfo info;
	socklen_t len;

	len = sizeof(info);
	if (getsockopt(sock, SOL_RFCOMM, RFCOMM_CONNINFO, &info, &len) < 0)
		return -errno;

	if (handle)
		*handle = info.hci_handle;

	if (dev_class)
		memcpy(dev_class, info.dev_class, 3);

	return 0;
}

static gboolean rfcomm_get(int sock, GError **err, BtIOOption opt1,
								va_list args)
{
	BtIOOption opt = opt1;
	struct sockaddr_rc src, dst;
	gboolean have_dst = FALSE;
	int flags;
	socklen_t len;
	uint8_t dev_class[3];
	uint16_t handle = 0;

	if (!get_src(sock, &src, sizeof(src), err))
		return FALSE;

	while (opt != BT_IO_OPT_INVALID) {
		switch (opt) {
		case BT_IO_OPT_SOURCE:
			ba2str(&src.rc_bdaddr, va_arg(args, char *));
			break;
		case BT_IO_OPT_SOURCE_BDADDR:
			bacpy(va_arg(args, bdaddr_t *), &src.rc_bdaddr);
			break;
		case BT_IO_OPT_DEST:
			if (!have_dst)
				have_dst = get_dst(sock, &dst, sizeof(dst),
									err);
			if (!have_dst)
				return FALSE;
			ba2str(&dst.rc_bdaddr, va_arg(args, char *));
			break;
		case BT_IO_OPT_DEST_BDADDR:
			if (!have_dst)
				have_dst = get_dst(sock, &dst, sizeof(dst),
									err);
			if (!have_dst)
				return FALSE;
			bacpy(va_arg(args, bdaddr_t *), &dst.rc_bdaddr);
			break;
		case BT_IO_OPT_DEFER_TIMEOUT:
			len = sizeof(int);
			if (getsockopt(sock, SOL_BLUETOOTH, BT_DEFER_SETUP,
					va_arg(args, int *), &len) < 0) {
				ERROR_FAILED(err, "getsockopt(DEFER_SETUP)",
									errno);
				return FALSE;
			}
			break;
		case BT_IO_OPT_SEC_LEVEL:
			if (!get_sec_level(sock, BT_IO_RFCOMM,
						va_arg(args, int *), err))
				return FALSE;
			break;
		case BT_IO_OPT_CHANNEL:
			if (src.rc_channel) {
				*(va_arg(args, uint8_t *)) = src.rc_channel;
				break;
			}

			if (!have_dst)
				have_dst = get_dst(sock, &dst, sizeof(dst),
									err);
			if (!have_dst)
				return FALSE;

			*(va_arg(args, uint8_t *)) = dst.rc_channel;
			break;
		case BT_IO_OPT_SOURCE_CHANNEL:
			*(va_arg(args, uint8_t *)) = src.rc_channel;
			break;
		case BT_IO_OPT_DEST_CHANNEL:
			if (!have_dst)
				have_dst = get_dst(sock, &dst, sizeof(dst),
									err);
			if (!have_dst)
				return FALSE;

			*(va_arg(args, uint8_t *)) = dst.rc_channel;
			break;
		case BT_IO_OPT_MASTER:
			len = sizeof(flags);
			if (getsockopt(sock, SOL_RFCOMM, RFCOMM_LM, &flags,
								&len) < 0) {
				ERROR_FAILED(err, "getsockopt(RFCOMM_LM)",
									errno);
				return FALSE;
			}
			*(va_arg(args, gboolean *)) =
				(flags & RFCOMM_LM_MASTER) ? TRUE : FALSE;
			break;
		case BT_IO_OPT_HANDLE:
			if (rfcomm_get_info(sock, &handle, dev_class) < 0) {
				ERROR_FAILED(err, "RFCOMM_CONNINFO", errno);
				return FALSE;
			}
			*(va_arg(args, uint16_t *)) = handle;
			break;
		case BT_IO_OPT_CLASS:
			if (rfcomm_get_info(sock, &handle, dev_class) < 0) {
				ERROR_FAILED(err, "RFCOMM_CONNINFO", errno);
				return FALSE;
			}
			memcpy(va_arg(args, uint8_t *), dev_class, 3);
			break;
		case BT_IO_OPT_SOURCE_TYPE:
		case BT_IO_OPT_DEST_TYPE:
		case BT_IO_OPT_KEY_SIZE:
		case BT_IO_OPT_PSM:
		case BT_IO_OPT_CID:
		case BT_IO_OPT_MTU:
		case BT_IO_OPT_OMTU:
		case BT_IO_OPT_IMTU:
		case BT_IO_OPT_MODE:
		case BT_IO_OPT_FLUSHABLE:
		case BT_IO_OPT_PRIORITY:
		case BT_IO_OPT_VOICE:
		case BT_IO_OPT_INVALID:
		default:
			g_set_error(err, BT_IO_ERROR, EINVAL,
					"Unknown option %d", opt);
			return FALSE;
		}

		opt = va_arg(args, int);
	}

	return TRUE;
}

static int sco_get_info(int sock, uint16_t *handle, uint8_t *dev_class)
{
	struct sco_conninfo info;
	socklen_t len;

	len = sizeof(info);
	if (getsockopt(sock, SOL_SCO, SCO_CONNINFO, &info, &len) < 0)
		return -errno;

	if (handle)
		*handle = info.hci_handle;

	if (dev_class)
		memcpy(dev_class, info.dev_class, 3);

	return 0;
}

static gboolean sco_get(int sock, GError **err, BtIOOption opt1, va_list args)
{
	BtIOOption opt = opt1;
	struct sockaddr_sco src, dst;
	struct sco_options sco_opt;
	socklen_t len;
	uint8_t dev_class[3];
	uint16_t handle = 0;

	len = sizeof(sco_opt);
	memset(&sco_opt, 0, len);
	if (getsockopt(sock, SOL_SCO, SCO_OPTIONS, &sco_opt, &len) < 0) {
		ERROR_FAILED(err, "getsockopt(SCO_OPTIONS)", errno);
		return FALSE;
	}

	if (!get_src(sock, &src, sizeof(src), err))
		return FALSE;

	if (!get_dst(sock, &dst, sizeof(dst), err))
		return FALSE;

	while (opt != BT_IO_OPT_INVALID) {
		switch (opt) {
		case BT_IO_OPT_SOURCE:
			ba2str(&src.sco_bdaddr, va_arg(args, char *));
			break;
		case BT_IO_OPT_SOURCE_BDADDR:
			bacpy(va_arg(args, bdaddr_t *), &src.sco_bdaddr);
			break;
		case BT_IO_OPT_DEST:
			ba2str(&dst.sco_bdaddr, va_arg(args, char *));
			break;
		case BT_IO_OPT_DEST_BDADDR:
			bacpy(va_arg(args, bdaddr_t *), &dst.sco_bdaddr);
			break;
		case BT_IO_OPT_MTU:
		case BT_IO_OPT_IMTU:
		case BT_IO_OPT_OMTU:
			*(va_arg(args, uint16_t *)) = sco_opt.mtu;
			break;
		case BT_IO_OPT_HANDLE:
			if (sco_get_info(sock, &handle, dev_class) < 0) {
				ERROR_FAILED(err, "SCO_CONNINFO", errno);
				return FALSE;
			}
			*(va_arg(args, uint16_t *)) = handle;
			break;
		case BT_IO_OPT_CLASS:
			if (sco_get_info(sock, &handle, dev_class) < 0) {
				ERROR_FAILED(err, "SCO_CONNINFO", errno);
				return FALSE;
			}
			memcpy(va_arg(args, uint8_t *), dev_class, 3);
			break;
		case BT_IO_OPT_SOURCE_TYPE:
		case BT_IO_OPT_DEST_TYPE:
		case BT_IO_OPT_DEFER_TIMEOUT:
		case BT_IO_OPT_SEC_LEVEL:
		case BT_IO_OPT_KEY_SIZE:
		case BT_IO_OPT_CHANNEL:
		case BT_IO_OPT_SOURCE_CHANNEL:
		case BT_IO_OPT_DEST_CHANNEL:
		case BT_IO_OPT_PSM:
		case BT_IO_OPT_CID:
		case BT_IO_OPT_MASTER:
		case BT_IO_OPT_MODE:
		case BT_IO_OPT_FLUSHABLE:
		case BT_IO_OPT_PRIORITY:
		case BT_IO_OPT_VOICE:
		case BT_IO_OPT_INVALID:
		default:
			g_set_error(err, BT_IO_ERROR, EINVAL,
					"Unknown option %d", opt);
			return FALSE;
		}

		opt = va_arg(args, int);
	}

	return TRUE;
}

static gboolean get_valist(GIOChannel *io, BtIOType type, GError **err,
						BtIOOption opt1, va_list args)
{
	int sock;

	sock = g_io_channel_unix_get_fd(io);

	switch (type) {
	case BT_IO_L2CAP:
		return l2cap_get(sock, err, opt1, args);
	case BT_IO_RFCOMM:
		return rfcomm_get(sock, err, opt1, args);
	case BT_IO_SCO:
		return sco_get(sock, err, opt1, args);
	case BT_IO_INVALID:
	default:
		g_set_error(err, BT_IO_ERROR, EINVAL,
				"Unknown BtIO type %d", type);
		return FALSE;
	}
}

gboolean bt_io_accept(GIOChannel *io, BtIOConnect connect, gpointer user_data,
					GDestroyNotify destroy, GError **err)
{
	int sock;
	char c;
	struct pollfd pfd;

	sock = g_io_channel_unix_get_fd(io);

	memset(&pfd, 0, sizeof(pfd));
	pfd.fd = sock;
	pfd.events = POLLOUT;

	if (poll(&pfd, 1, 0) < 0) {
		ERROR_FAILED(err, "poll", errno);
		return FALSE;
	}

	if (!(pfd.revents & POLLOUT)) {
		if (read(sock, &c, 1) < 0) {
			ERROR_FAILED(err, "read", errno);
			return FALSE;
		}
	}

	accept_add(io, connect, user_data, destroy);

	return TRUE;
}

gboolean bt_io_set(GIOChannel *io, GError **err, BtIOOption opt1, ...)
{
	va_list args;
	gboolean ret;
	struct set_opts opts;
	int sock;
	BtIOType type;

	va_start(args, opt1);
	ret = parse_set_opts(&opts, err, opt1, args);
	va_end(args);

	if (!ret)
		return ret;

	type = bt_io_get_type(io, err);
	if (type == BT_IO_INVALID)
		return FALSE;

	sock = g_io_channel_unix_get_fd(io);

	switch (type) {
	case BT_IO_L2CAP:
		return l2cap_set(sock, opts.src_type, opts.sec_level, opts.imtu,
					opts.omtu, opts.mode, opts.master,
					opts.flushable, opts.priority, err);
	case BT_IO_RFCOMM:
		return rfcomm_set(sock, opts.sec_level, opts.master, err);
	case BT_IO_SCO:
		return sco_set(sock, opts.mtu, opts.voice, err);
	case BT_IO_INVALID:
	default:
		g_set_error(err, BT_IO_ERROR, EINVAL,
				"Unknown BtIO type %d", type);
		return FALSE;
	}

}

gboolean bt_io_get(GIOChannel *io, GError **err, BtIOOption opt1, ...)
{
	va_list args;
	gboolean ret;
	BtIOType type;

	type = bt_io_get_type(io, err);
	if (type == BT_IO_INVALID)
		return FALSE;

	va_start(args, opt1);
	ret = get_valist(io, type, err, opt1, args);
	va_end(args);

	return ret;
}

static GIOChannel *create_io(gboolean server, struct set_opts *opts,
								GError **err)
{
	int sock;
	GIOChannel *io;

	switch (opts->type) {
	case BT_IO_L2CAP:
		sock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
		if (sock < 0) {
			ERROR_FAILED(err, "socket(SEQPACKET, L2CAP)", errno);
			return NULL;
		}
		if (l2cap_bind(sock, &opts->src, opts->src_type,
				server ? opts->psm : 0, opts->cid, err) < 0)
			goto failed;
		if (!l2cap_set(sock, opts->src_type, opts->sec_level,
				opts->imtu, opts->omtu, opts->mode,
				opts->master, opts->flushable, opts->priority,
				err))
			goto failed;
		break;
	case BT_IO_RFCOMM:
		sock = socket(PF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
		if (sock < 0) {
			ERROR_FAILED(err, "socket(STREAM, RFCOMM)", errno);
			return NULL;
		}
		if (rfcomm_bind(sock, &opts->src,
					server ? opts->channel : 0, err) < 0)
			goto failed;
		if (!rfcomm_set(sock, opts->sec_level, opts->master, err))
			goto failed;
		break;
	case BT_IO_SCO:
		sock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_SCO);
		if (sock < 0) {
			ERROR_FAILED(err, "socket(SEQPACKET, SCO)", errno);
			return NULL;
		}
		if (sco_bind(sock, &opts->src, err) < 0)
			goto failed;
		if (!sco_set(sock, opts->mtu, opts->voice, err))
			goto failed;
		break;
	case BT_IO_INVALID:
	default:
		g_set_error(err, BT_IO_ERROR, EINVAL,
				"Unknown BtIO type %d", opts->type);
		return NULL;
	}

	io = g_io_channel_unix_new(sock);

	g_io_channel_set_close_on_unref(io, TRUE);
	g_io_channel_set_flags(io, G_IO_FLAG_NONBLOCK, NULL);

	return io;

failed:
	close(sock);

	return NULL;
}

GIOChannel *bt_io_connect(BtIOConnect connect, gpointer user_data,
				GDestroyNotify destroy, GError **gerr,
				BtIOOption opt1, ...)
{
	GIOChannel *io;
	va_list args;
	struct set_opts opts;
	int err, sock;
	gboolean ret;

	va_start(args, opt1);
	ret = parse_set_opts(&opts, gerr, opt1, args);
	va_end(args);

	if (ret == FALSE)
		return NULL;

	io = create_io(FALSE, &opts, gerr);
	if (io == NULL)
		return NULL;

	sock = g_io_channel_unix_get_fd(io);

	switch (opts.type) {
	case BT_IO_L2CAP:
		err = l2cap_connect(sock, &opts.dst, opts.dst_type,
							opts.psm, opts.cid);
		break;
	case BT_IO_RFCOMM:
		err = rfcomm_connect(sock, &opts.dst, opts.channel);
		break;
	case BT_IO_SCO:
		err = sco_connect(sock, &opts.dst);
		break;
	case BT_IO_INVALID:
	default:
		g_set_error(gerr, BT_IO_ERROR, EINVAL,
					"Unknown BtIO type %d", opts.type);
		return NULL;
	}

	if (err < 0) {
		ERROR_FAILED(gerr, "connect", -err);
		g_io_channel_unref(io);
		return NULL;
	}

	connect_add(io, connect, user_data, destroy);

	return io;
}

GIOChannel *bt_io_listen(BtIOConnect connect, BtIOConfirm confirm,
				gpointer user_data, GDestroyNotify destroy,
				GError **err, BtIOOption opt1, ...)
{
	GIOChannel *io;
	va_list args;
	struct set_opts opts;
	int sock;
	gboolean ret;

	va_start(args, opt1);
	ret = parse_set_opts(&opts, err, opt1, args);
	va_end(args);

	if (ret == FALSE)
		return NULL;

	io = create_io(TRUE, &opts, err);
	if (io == NULL)
		return NULL;

	sock = g_io_channel_unix_get_fd(io);

	if (confirm)
		setsockopt(sock, SOL_BLUETOOTH, BT_DEFER_SETUP, &opts.defer,
							sizeof(opts.defer));

	if (listen(sock, 5) < 0) {
		ERROR_FAILED(err, "listen", errno);
		g_io_channel_unref(io);
		return NULL;
	}

	server_add(io, connect, confirm, user_data, destroy);

	return io;
}

GQuark bt_io_error_quark(void)
{
	return g_quark_from_static_string("bt-io-error-quark");
}

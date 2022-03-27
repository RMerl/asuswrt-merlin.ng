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

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <net/if.h>
#include <linux/sockios.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/l2cap.h"
#include "lib/bnep.h"
#include "lib/uuid.h"

#include "src/log.h"
#include "src/shared/util.h"
#include "btio/btio.h"

#include "bnep.h"

#define CON_SETUP_RETRIES      3
#define CON_SETUP_TO           9

static int ctl;

struct __service_16 {
	uint16_t dst;
	uint16_t src;
} __attribute__ ((packed));

struct bnep {
	GIOChannel	*io;
	uint16_t	src;
	uint16_t	dst;
	bdaddr_t	dst_addr;
	char	iface[16];
	guint	attempts;
	guint	setup_to;
	guint	watch;
	bnep_connect_cb	conn_cb;
	void	*conn_data;
	bnep_disconnect_cb disconn_cb;
	void	*disconn_data;
};

int bnep_init(void)
{
	ctl = socket(PF_BLUETOOTH, SOCK_RAW, BTPROTO_BNEP);
	if (ctl < 0) {
		int err = -errno;

		if (err == -EPROTONOSUPPORT)
			warn("kernel lacks bnep-protocol support");
		else
			error("bnep: Failed to open control socket: %s (%d)",
							strerror(-err), -err);

		return err;
	}

	return 0;
}

int bnep_cleanup(void)
{
	close(ctl);
	return 0;
}

static int bnep_conndel(const bdaddr_t *dst)
{
	struct bnep_conndel_req req;

	memset(&req, 0, sizeof(req));
	baswap((bdaddr_t *)&req.dst, dst);
	req.flags = 0;
	if (ioctl(ctl, BNEPCONNDEL, &req) < 0) {
		int err = -errno;
		error("bnep: Failed to kill connection: %s (%d)",
							strerror(-err), -err);
		return err;
	}
	return 0;
}

static int bnep_connadd(int sk, uint16_t role, char *dev)
{
	struct bnep_connadd_req req;

	memset(&req, 0, sizeof(req));
	strncpy(req.device, dev, 16);
	req.device[15] = '\0';

	req.sock = sk;
	req.role = role;
	req.flags = (1 << BNEP_SETUP_RESPONSE);
	if (ioctl(ctl, BNEPCONNADD, &req) < 0) {
		int err = -errno;
		error("bnep: Failed to add device %s: %s(%d)",
						dev, strerror(-err), -err);
		return err;
	}

	strncpy(dev, req.device, 16);
	return 0;
}

static uint32_t bnep_getsuppfeat(void)
{
	uint32_t feat;

	if (ioctl(ctl, BNEPGETSUPPFEAT, &feat) < 0)
		feat = 0;

	DBG("supported features: 0x%x", feat);

	return feat;
}

static int bnep_if_up(const char *devname)
{
	struct ifreq ifr;
	int sk, err = 0;

	sk = socket(AF_INET, SOCK_DGRAM, 0);

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, devname, IF_NAMESIZE - 1);

	ifr.ifr_flags |= IFF_UP;
	ifr.ifr_flags |= IFF_MULTICAST;

	if (ioctl(sk, SIOCSIFFLAGS, (void *) &ifr) < 0) {
		err = -errno;
		error("bnep: Could not bring up %s: %s(%d)",
						devname, strerror(-err), -err);
	}

	close(sk);

	return err;
}

static int bnep_if_down(const char *devname)
{
	struct ifreq ifr;
	int sk, err = 0;

	sk = socket(AF_INET, SOCK_DGRAM, 0);

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, devname, IF_NAMESIZE - 1);

	ifr.ifr_flags &= ~IFF_UP;

	/* Bring down the interface */
	if (ioctl(sk, SIOCSIFFLAGS, (void *) &ifr) < 0) {
		err = -errno;
		error("bnep: Could not bring down %s: %s(%d)",
						devname, strerror(-err), -err);
	}

	close(sk);

	return err;
}

static gboolean bnep_watchdog_cb(GIOChannel *chan, GIOCondition cond,
								gpointer data)
{
	struct bnep *session = data;

	if (session->disconn_cb)
		session->disconn_cb(session->disconn_data);

	return FALSE;
}

static gboolean bnep_setup_cb(GIOChannel *chan, GIOCondition cond,
								gpointer data)
{
	struct bnep *session = data;
	struct bnep_control_rsp *rsp;
	struct timeval timeo;
	char pkt[BNEP_MTU];
	ssize_t r;
	int sk;

	if (cond & G_IO_NVAL)
		return FALSE;

	if (session->setup_to > 0) {
		g_source_remove(session->setup_to);
		session->setup_to = 0;
	}

	if (cond & (G_IO_HUP | G_IO_ERR)) {
		error("bnep: Hangup or error on l2cap server socket");
		goto failed;
	}

	sk = g_io_channel_unix_get_fd(chan);
	memset(pkt, 0, BNEP_MTU);
	r = read(sk, pkt, sizeof(pkt) - 1);
	if (r < 0) {
		error("bnep: IO Channel read error");
		goto failed;
	}

	if (r == 0) {
		error("bnep: No packet received on l2cap socket");
		goto failed;
	}

	errno = EPROTO;

	if ((size_t) r < sizeof(*rsp)) {
		error("bnep: Packet received is not bnep type");
		goto failed;
	}

	rsp = (void *) pkt;
	if (rsp->type != BNEP_CONTROL) {
		error("bnep: Packet received is not bnep type");
		goto failed;
	}

	if (rsp->ctrl != BNEP_SETUP_CONN_RSP)
		return TRUE;

	r = ntohs(rsp->resp);
	if (r != BNEP_SUCCESS) {
		error("bnep: failed");
		goto failed;
	}

	memset(&timeo, 0, sizeof(timeo));
	timeo.tv_sec = 0;
	setsockopt(sk, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo));

	sk = g_io_channel_unix_get_fd(session->io);
	if (bnep_connadd(sk, session->src, session->iface) < 0)
		goto failed;

	if (bnep_if_up(session->iface) < 0) {
		bnep_conndel(&session->dst_addr);
		goto failed;
	}

	session->watch = g_io_add_watch(session->io,
					G_IO_ERR | G_IO_HUP | G_IO_NVAL,
					(GIOFunc) bnep_watchdog_cb, session);
	g_io_channel_unref(session->io);
	session->io = NULL;

	session->conn_cb(session->iface, 0, session->conn_data);

	return FALSE;

failed:
	session->conn_cb(NULL, -EIO, session->conn_data);

	return FALSE;
}

static int bnep_setup_conn_req(struct bnep *session)
{
	struct bnep_setup_conn_req *req;
	struct __service_16 *s;
	unsigned char pkt[BNEP_MTU];
	int fd;

	/* Send request */
	req = (void *) pkt;
	req->type = BNEP_CONTROL;
	req->ctrl = BNEP_SETUP_CONN_REQ;
	req->uuid_size = 2;     /* 16bit UUID */
	s = (void *) req->service;
	s->src = htons(session->src);
	s->dst = htons(session->dst);

	fd = g_io_channel_unix_get_fd(session->io);
	if (write(fd, pkt, sizeof(*req) + sizeof(*s)) < 0) {
		error("bnep: connection req send failed: %s", strerror(errno));
		return -errno;
	}

	session->attempts++;

	return 0;
}

static gboolean bnep_conn_req_to(gpointer user_data)
{
	struct bnep *session = user_data;

	if (session->attempts == CON_SETUP_RETRIES) {
		error("bnep: Too many bnep connection attempts");
	} else {
		error("bnep: connection setup TO, retrying...");
		if (bnep_setup_conn_req(session) == 0)
			return TRUE;
	}

	session->conn_cb(NULL, -ETIMEDOUT, session->conn_data);

	return FALSE;
}

struct bnep *bnep_new(int sk, uint16_t local_role, uint16_t remote_role,
								char *iface)
{
	struct bnep *session;
	int dup_fd;

	dup_fd = dup(sk);
	if (dup_fd < 0)
		return NULL;

	session = g_new0(struct bnep, 1);
	session->io = g_io_channel_unix_new(dup_fd);
	session->src = local_role;
	session->dst = remote_role;
	strncpy(session->iface, iface, 16);
	session->iface[15] = '\0';

	g_io_channel_set_close_on_unref(session->io, TRUE);
	session->watch = g_io_add_watch(session->io,
				G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
					(GIOFunc) bnep_setup_cb, session);

	return session;
}

void bnep_free(struct bnep *session)
{
	if (!session)
		return;

	if (session->io) {
		g_io_channel_shutdown(session->io, FALSE, NULL);
		g_io_channel_unref(session->io);
		session->io = NULL;
	}

	if (session->watch > 0) {
		g_source_remove(session->watch);
		session->watch = 0;
	}

	g_free(session);
}

int bnep_connect(struct bnep *session, bnep_connect_cb conn_cb,
					bnep_disconnect_cb disconn_cb,
					void *conn_data, void *disconn_data)
{
	GError *gerr = NULL;
	int err;

	if (!session || !conn_cb || !disconn_cb)
		return -EINVAL;

	session->attempts = 0;
	session->conn_cb = conn_cb;
	session->disconn_cb = disconn_cb;
	session->conn_data = conn_data;
	session->disconn_data = disconn_data;

	bt_io_get(session->io, &gerr, BT_IO_OPT_DEST_BDADDR, &session->dst_addr,
							BT_IO_OPT_INVALID);
	if (gerr) {
		error("bnep: connect failed: %s", gerr->message);
		g_error_free(gerr);
		return -EINVAL;
	}

	err = bnep_setup_conn_req(session);
	if (err < 0)
		return err;

	session->setup_to = g_timeout_add_seconds(CON_SETUP_TO,
						bnep_conn_req_to, session);
	return 0;
}

void bnep_disconnect(struct bnep *session)
{
	if (!session)
		return;

	if (session->watch > 0) {
		g_source_remove(session->watch);
		session->watch = 0;
	}

	if (session->io) {
		g_io_channel_unref(session->io);
		session->io = NULL;
	}

	bnep_if_down(session->iface);
	bnep_conndel(&session->dst_addr);
}

static int bnep_add_to_bridge(const char *devname, const char *bridge)
{
	int ifindex;
	struct ifreq ifr;
	int sk, err = 0;

	if (!devname || !bridge)
		return -EINVAL;

	ifindex = if_nametoindex(devname);

	sk = socket(AF_INET, SOCK_STREAM, 0);
	if (sk < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, bridge, IFNAMSIZ - 1);
	ifr.ifr_ifindex = ifindex;

	if (ioctl(sk, SIOCBRADDIF, &ifr) < 0) {
		err = -errno;
		error("bnep: Can't add %s to the bridge %s: %s(%d)",
					devname, bridge, strerror(-err), -err);
	} else {
		info("bnep: bridge %s: interface %s added", bridge, devname);
	}

	close(sk);

	return err;
}

static int bnep_del_from_bridge(const char *devname, const char *bridge)
{
	int ifindex;
	struct ifreq ifr;
	int sk, err = 0;

	if (!devname || !bridge)
		return -EINVAL;

	ifindex = if_nametoindex(devname);

	sk = socket(AF_INET, SOCK_STREAM, 0);
	if (sk < 0)
		return -1;

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, bridge, IFNAMSIZ - 1);
	ifr.ifr_ifindex = ifindex;

	if (ioctl(sk, SIOCBRDELIF, &ifr) < 0) {
		err = -errno;
		error("bnep: Can't delete %s from the bridge %s: %s(%d)",
					devname, bridge, strerror(-err), -err);
	} else {
		info("bnep: bridge %s: interface %s removed", bridge, devname);
	}

	close(sk);

	return err;
}

static ssize_t bnep_send_ctrl_rsp(int sk, uint8_t ctrl, uint16_t resp)
{
	ssize_t sent;

	switch (ctrl) {
	case BNEP_CMD_NOT_UNDERSTOOD: {
		struct bnep_ctrl_cmd_not_understood_cmd rsp;

		rsp.type = BNEP_CONTROL;
		rsp.ctrl = ctrl;
		rsp.unkn_ctrl = (uint8_t) resp;

		sent = send(sk, &rsp, sizeof(rsp), 0);
		break;
	}
	case BNEP_FILTER_MULT_ADDR_RSP:
	case BNEP_FILTER_NET_TYPE_RSP:
	case BNEP_SETUP_CONN_RSP: {
		struct bnep_control_rsp rsp;

		rsp.type = BNEP_CONTROL;
		rsp.ctrl = ctrl;
		rsp.resp = htons(resp);

		sent = send(sk, &rsp, sizeof(rsp), 0);
		break;
	}
	default:
		error("bnep: wrong response type");
		sent = -1;
		break;
	}

	return sent;
}

static uint16_t bnep_setup_decode(int sk, struct bnep_setup_conn_req *req,
								uint16_t *dst)
{
	const uint8_t bt_base[] = { 0x00, 0x00, 0x10, 0x00, 0x80, 0x00,
					0x00, 0x80, 0x5F, 0x9B, 0x34, 0xFB };
	uint16_t src;
	uint8_t *dest, *source;
	uint32_t val;

	if (((req->type != BNEP_CONTROL) &&
		(req->type != (BNEP_CONTROL | BNEP_EXT_HEADER)))  ||
					req->ctrl != BNEP_SETUP_CONN_REQ)
		return BNEP_CONN_NOT_ALLOWED;

	dest = req->service;
	source = req->service + req->uuid_size;

	switch (req->uuid_size) {
	case 2: /* UUID16 */
		*dst = get_be16(dest);
		src = get_be16(source);
		break;
	case 16: /* UUID128 */
		/* Check that the bytes in the UUID, except the service ID
		 * itself, are correct. The service ID is checked in
		 * bnep_setup_chk(). */
		if (memcmp(&dest[4], bt_base, sizeof(bt_base)) != 0)
			return BNEP_CONN_INVALID_DST;
		if (memcmp(&source[4], bt_base, sizeof(bt_base)) != 0)
			return BNEP_CONN_INVALID_SRC;

		/* Intentional no-break */

	case 4: /* UUID32 */
		val = get_be32(dest);
		if (val > 0xffff)
			return BNEP_CONN_INVALID_DST;

		*dst = val;

		val = get_be32(source);
		if (val > 0xffff)
			return BNEP_CONN_INVALID_SRC;

		src = val;
		break;
	default:
		return BNEP_CONN_INVALID_SVC;
	}

	/* Allowed PAN Profile scenarios */
	switch (*dst) {
	case BNEP_SVC_NAP:
	case BNEP_SVC_GN:
		if (src == BNEP_SVC_PANU)
			return BNEP_SUCCESS;
		return BNEP_CONN_INVALID_SRC;
	case BNEP_SVC_PANU:
		if (src == BNEP_SVC_PANU || src == BNEP_SVC_GN ||
							src == BNEP_SVC_NAP)
			return BNEP_SUCCESS;

		return BNEP_CONN_INVALID_SRC;
	}

	return BNEP_CONN_INVALID_DST;
}

static int bnep_server_add_legacy(int sk, uint16_t dst, char *bridge,
					char *iface, const bdaddr_t *addr,
					uint8_t *setup_data, int len)
{
	int err, n;
	uint16_t rsp;

	n = read(sk, setup_data, len);
	if (n != len) {
		err = -EIO;
		rsp = BNEP_CONN_NOT_ALLOWED;
		goto reply;
	}

	err = bnep_connadd(sk, dst, iface);
	if (err < 0) {
		rsp = BNEP_CONN_NOT_ALLOWED;
		goto reply;
	}

	err = bnep_add_to_bridge(iface, bridge);
	if (err < 0) {
		bnep_conndel(addr);
		rsp = BNEP_CONN_NOT_ALLOWED;
		goto reply;
	}

	err = bnep_if_up(iface);
	if (err < 0) {
		bnep_del_from_bridge(iface, bridge);
		bnep_conndel(addr);
		rsp = BNEP_CONN_NOT_ALLOWED;
		goto reply;
	}

	rsp = BNEP_SUCCESS;

reply:
	if (bnep_send_ctrl_rsp(sk, BNEP_SETUP_CONN_RSP, rsp) < 0) {
		err = -errno;
		error("bnep: send ctrl rsp error: %s (%d)", strerror(-err),
									-err);
	}

	return err;
}

int bnep_server_add(int sk, char *bridge, char *iface, const bdaddr_t *addr,
						uint8_t *setup_data, int len)
{
	int err;
	uint32_t feat;
	uint16_t rsp, dst;
	struct bnep_setup_conn_req *req = (void *) setup_data;

	/* Highest known Control command ID
	 * is BNEP_FILTER_MULT_ADDR_RSP = 0x06 */
	if (req->type == BNEP_CONTROL &&
					req->ctrl > BNEP_FILTER_MULT_ADDR_RSP) {
		error("bnep: cmd not understood");
		err = bnep_send_ctrl_rsp(sk, BNEP_CMD_NOT_UNDERSTOOD,
								req->ctrl);
		if (err < 0)
			error("send not understood ctrl rsp error: %s (%d)",
							strerror(errno), errno);

		return err;
	}

	/* Processing BNEP_SETUP_CONNECTION_REQUEST_MSG */
	rsp = bnep_setup_decode(sk, req, &dst);
	if (rsp != BNEP_SUCCESS) {
		err = -rsp;
		error("bnep: error while decoding setup connection request: %d",
									rsp);
		goto failed;
	}

	feat = bnep_getsuppfeat();

	/*
	 * Take out setup data if kernel doesn't support handling it, especially
	 * setup request. If kernel would have set session flags, they should
	 * be checked and handled respectively.
	 */
	if (!feat || !(feat & (1 << BNEP_SETUP_RESPONSE)))
		return bnep_server_add_legacy(sk, dst, bridge, iface, addr,
							setup_data, len);

	err = bnep_connadd(sk, dst, iface);
	if (err < 0) {
		rsp = BNEP_CONN_NOT_ALLOWED;
		goto failed;
	}

	err = bnep_add_to_bridge(iface, bridge);
	if (err < 0)
		goto failed_conn;

	err = bnep_if_up(iface);
	if (err < 0)
		goto failed_bridge;

	return 0;

failed_bridge:
	bnep_del_from_bridge(iface, bridge);

failed_conn:
	bnep_conndel(addr);

	return err;

failed:
	if (bnep_send_ctrl_rsp(sk, BNEP_SETUP_CONN_RSP, rsp) < 0) {
		err = -errno;
		error("bnep: send ctrl rsp error: %s (%d)", strerror(-err),
									-err);
	}

	return err;
}

void bnep_server_delete(char *bridge, char *iface, const bdaddr_t *addr)
{
	if (!bridge || !iface || !addr)
		return;

	bnep_del_from_bridge(iface, bridge);
	bnep_if_down(iface);
	bnep_conndel(addr);
}

/*
 * WPA Supplicant / UNIX domain socket -based control interface
 * Copyright (c) 2004-2014, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

/*
 * ***************************************************************************
 * *  Mediatek Inc.
 * * 4F, No. 2 Technology 5th Rd.
 * * Science-based Industrial Park
 * * Hsin-chu, Taiwan, R.O.C.
 * *
 * * (c) Copyright 2002-2018, Mediatek, Inc.
 * *
 * * All rights reserved. Mediatek's source code is an unpublished work and the
 * * use of a copyright notice does not imply otherwise. This source code
 * * contains confidential trade secret material of Ralink Tech. Any attemp
 * * or participation in deciphering, decoding, reverse engineering or in any
 * * way altering the source code is stricitly prohibited, unless the prior
 * * written consent of Mediatek, Inc. is obtained.
 * ***************************************************************************
 *
 *  Module Name:
 *  UNIX domain socket -based control interface
 *
 *  Abstract:
 *  UNIX domain socket -based control interface
 *
 *  Revision History:
 *  Who         When          What
 *  --------    ----------    -----------------------------------------
 *  Neelansh.M   2018/05/02     Derived from  WPA Supplicant / UNIX domain
								socket -based control interface
 * */

#include "includes.h"
#include <sys/un.h>
#include <sys/stat.h>
#include <grp.h>
#include <stddef.h>
#include <unistd.h>
#include <fcntl.h>
#ifdef __linux__
#include <sys/ioctl.h>
#endif /* __linux__ */

#include "utils/common.h"
#include "utils/eloop.h"
#include "utils/list.h"
#include "client_db.h"
#include "mapd_i.h"
#include "ctrl_iface.h"

/* Per-interface ctrl_iface */

/**
 * struct wpa_ctrl_dst - Internal data structure of control interface monitors
 *
 * This structure is used to store information about registered control
 * interface monitors into struct wpa_supplicant. This data is private to
 * ctrl_iface_unix.c and should not be touched directly from other files.
 */

static int mapd_ctrl_iface_attach(struct dl_list *ctrl_dst,
					    struct sockaddr_un *from,
					    socklen_t fromlen)
{
	struct mapd_ctrl_dst *dst;
	char addr_txt[200];

	dst = os_zalloc(sizeof(*dst));
	if (dst == NULL)
		return -1;
	os_memcpy(&dst->addr, from, sizeof(struct sockaddr_un));
	dst->addrlen = fromlen;
	dst->debug_level = MSG_INFO;
	dl_list_add(ctrl_dst, &dst->list);
	printf_encode(addr_txt, sizeof(addr_txt),
		      (u8 *) from->sun_path,
		      fromlen - offsetof(struct sockaddr_un, sun_path));
	mapd_printf(MSG_DEBUG, "CTRL_IFACE monitor attached %s", addr_txt);
	return 0;
}


static int mapd_ctrl_iface_detach(struct dl_list *ctrl_dst,
					    struct sockaddr_un *from,
					    socklen_t fromlen)
{
	struct mapd_ctrl_dst *dst;

	dl_list_for_each(dst, ctrl_dst, struct mapd_ctrl_dst, list) {
		if (fromlen == dst->addrlen &&
		    os_memcmp(from->sun_path, dst->addr.sun_path,
			      fromlen - offsetof(struct sockaddr_un, sun_path))
		    == 0) {
			char addr_txt[200];
			printf_encode(addr_txt, sizeof(addr_txt),
				      (u8 *) from->sun_path,
				      fromlen -
				      offsetof(struct sockaddr_un, sun_path));
			mapd_printf(MSG_DEBUG, "CTRL_IFACE monitor detached %s",
				   addr_txt);
			dl_list_del(&dst->list);
			os_free(dst);
			return 0;
		}
	}
	return -1;
}

static void mapd_ctrl_sock_debug(const char *title, int sock, const char *buf,
		size_t len)
{
#ifdef __linux__
	socklen_t optlen;
	int sndbuf, outq;
	int level = MSG_MSGDUMP;

	if (len >= 5 && os_strncmp(buf, "PONG\n", 5) == 0)
		level = MSG_EXCESSIVE;

	optlen = sizeof(sndbuf);
	sndbuf = 0;
	if (getsockopt(sock, SOL_SOCKET, SO_SNDBUF, &sndbuf, &optlen) < 0)
		sndbuf = -1;

	if (ioctl(sock, TIOCOUTQ, &outq) < 0)
		outq = -1;

	mapd_printf(level,
			"CTRL-DEBUG: %s: sock=%d sndbuf=%d outq=%d send_len=%d",
			title, sock, sndbuf, outq, (int) len);
#endif /* __linux__ */
}
static int mapd_global_ctrl_iface_open_sock(struct mapd_global *global,
		struct ctrl_iface_global_priv *priv);

static int mapd_ctrl_iface_global_reinit(struct mapd_global *global,
					 struct ctrl_iface_global_priv *priv)
{
	int res;

	if (priv->sock <= 0)
		return -1;

	eloop_unregister_read_sock(priv->sock);
	close(priv->sock);
	priv->sock = -1;
	res = mapd_global_ctrl_iface_open_sock(global, priv);
	if (res < 0)
		return -1;
	return priv->sock;
}


/* Global ctrl_iface */

static void mapd_global_ctrl_iface_receive(int sock, void *eloop_ctx,
		void *sock_ctx)
{
	struct mapd_global *global = eloop_ctx;
	struct ctrl_iface_global_priv *priv = sock_ctx;
	char buf[4096] = {0};
	int res;
	struct sockaddr_storage from;
	socklen_t fromlen = sizeof(from);
	char *reply = NULL, *reply_buf = NULL;
	size_t reply_len;

	res = recvfrom(sock, buf, sizeof(buf) - 1, 0,
			(struct sockaddr *) &from, &fromlen);
	if (res < 0) {
		mapd_printf(MSG_ERROR, "recvfrom(ctrl_iface): %s",
				strerror(errno));
		return;
	}
	buf[res] = '\0';
	if (os_strcmp(buf, "ATTACH") == 0) {
		if (mapd_ctrl_iface_attach(&priv->ctrl_dst, (struct sockaddr_un *)&from,
							 fromlen))
			reply_len = 1;
		else {
			reply_len = 2;
		}
	} else if (os_strcmp(buf, "DETACH") == 0) {
		if (mapd_ctrl_iface_detach(&priv->ctrl_dst, (struct sockaddr_un *)&from,
							 fromlen))
			reply_len = 1;
		else
			reply_len = 2;
	} else {
		reply_buf = mapd_global_ctrl_iface_process(
				global, buf, &reply_len);
		reply = reply_buf;
	}
	/*
	 * There could be some password/key material in the commapd, so
	 * clear the buffer explicitly now that it is not needed
	 * anymore.
	 */
	os_memset(buf, 0, res);

	if (!reply && reply_len == 1) {
		reply = "FAIL\n";
		reply_len = 5;
	} else if (!reply && reply_len == 2) {
		reply = "OK\n";
		reply_len = 3;
	}

	if (reply) {
		mapd_ctrl_sock_debug("global_ctrl_sock-sendto",
				sock, reply, reply_len);
		if (sendto(sock, reply, reply_len, 0, (struct sockaddr *) &from,
					fromlen) < 0) {
			mapd_printf(MSG_ERROR, "ctrl_iface sendto failed: %s",
					strerror(errno));
		}
	}
	os_free(reply_buf);
}


static int mapd_global_ctrl_iface_open_sock(struct mapd_global *global,
		struct ctrl_iface_global_priv *priv)
{
	struct sockaddr_un addr;
	const char *ctrl = global->params.ctrl_interface;
	int flags;

	mapd_printf(MSG_INFO, "Global control interface '%s'", ctrl);

	priv->sock = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (priv->sock < 0) {
		mapd_printf(MSG_ERROR, "socket(PF_UNIX): %s", strerror(errno));
		goto fail;
	}

	os_memset(&addr, 0, sizeof(addr));
#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
	addr.sun_len = sizeof(addr);
#endif /* __FreeBSD__ */
	addr.sun_family = AF_UNIX;

	if (os_strncmp(ctrl, "@abstract:", 10) == 0) {
		addr.sun_path[0] = '\0';
		os_strlcpy(addr.sun_path + 1, ctrl + 10,
				sizeof(addr.sun_path) - 1);
		if (bind(priv->sock, (struct sockaddr *) &addr, sizeof(addr)) <
				0) {
			mapd_printf(MSG_ERROR, "supp-global-ctrl-iface-init: "
					"bind(PF_UNIX;%s) failed: %s",
					ctrl, strerror(errno));
			goto fail;
		}
		mapd_printf(MSG_DEBUG, "Using Abstract control socket '%s'",
				ctrl + 10);
		goto havesock;
	}

	os_strlcpy(addr.sun_path, ctrl, sizeof(addr.sun_path));
	if (bind(priv->sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		mapd_printf(MSG_INFO, "supp-global-ctrl-iface-init(%s) (will try fixup): bind(PF_UNIX): %s",
				ctrl, strerror(errno));
		if (connect(priv->sock, (struct sockaddr *) &addr,
					sizeof(addr)) < 0) {
			mapd_printf(MSG_DEBUG, "ctrl_iface exists, but does not"
					" allow connections - assuming it was left"
					"over from forced program termination");
			if (unlink(ctrl) < 0) {
				mapd_printf(MSG_ERROR,
						"Could not unlink existing ctrl_iface socket '%s': %s",
						ctrl, strerror(errno));
				goto fail;
			}
			if (bind(priv->sock, (struct sockaddr *) &addr,
						sizeof(addr)) < 0) {
				mapd_printf(MSG_ERROR, "supp-glb-iface-init: bind(PF_UNIX;%s): %s",
						ctrl, strerror(errno));
				goto fail;
			}
			mapd_printf(MSG_DEBUG, "Successfully replaced leftover "
					"ctrl_iface socket '%s'",
					ctrl);
		} else {
			mapd_printf(MSG_INFO, "ctrl_iface exists and seems to "
					"be in use - cannot override it");
			mapd_printf(MSG_INFO, "Delete '%s' manually if it is "
					"not used anymore",
					ctrl);
			goto fail;
		}
	}

	mapd_printf(MSG_DEBUG, "Using UNIX control socket '%s'", ctrl);

	if (global->params.ctrl_interface_group) {
		char *gid_str = global->params.ctrl_interface_group;
		gid_t gid = 0;
		struct group *grp;
		char *endp;

		grp = getgrnam(gid_str);
		if (grp) {
			gid = grp->gr_gid;
			mapd_printf(MSG_DEBUG, "ctrl_interface_group=%d"
					" (from group name '%s')",
					(int) gid, gid_str);
		} else {
			/* Group name not found - try to parse this as gid */
			gid = strtol(gid_str, &endp, 10);
			if (*gid_str == '\0' || *endp != '\0') {
				mapd_printf(MSG_ERROR, "CTRL: Invalid group "
						"'%s'", gid_str);
				goto fail;
			}
			mapd_printf(MSG_DEBUG, "ctrl_interface_group=%d",
					(int) gid);
		}
		if (chown(ctrl, -1, gid) < 0) {
			mapd_printf(MSG_ERROR,
					"chown[global_ctrl_interface=%s,gid=%d]: %s",
					ctrl, (int) gid, strerror(errno));
			goto fail;
		}

		if (chmod(ctrl, S_IRWXU | S_IRWXG) < 0) {
			mapd_printf(MSG_ERROR,
					"chmod[global_ctrl_interface=%s]: %s",
					ctrl, strerror(errno));
			goto fail;
		}
	} else {
		if (chmod(ctrl, S_IRWXU) < 0) {
			mapd_printf(MSG_DEBUG,
					"chmod[global_ctrl_interface=%s](S_IRWXU): %s",
					ctrl, strerror(errno));
			/* continue anyway since group change was not required
			 */
		}
	}

havesock:

	/*
	 * Make socket non-blocking so that we don't hang forever if
	 * target dies unexpectedly.
	 */
	flags = fcntl(priv->sock, F_GETFL);
	if (flags >= 0) {
		flags |= O_NONBLOCK;
		if (fcntl(priv->sock, F_SETFL, flags) < 0) {
			mapd_printf(MSG_INFO, "fcntl(ctrl, O_NONBLOCK): %s",
					strerror(errno));
			/* Not fatal, continue on.*/
		}
	}

	eloop_register_read_sock(priv->sock,
			mapd_global_ctrl_iface_receive,
			global, priv);

	return 0;

fail:
	if (priv->sock >= 0) {
		close(priv->sock);
		priv->sock = -1;
	}
	return -1;
}
static mapd_msg_cb_func mapd_msg_cb = NULL;

void mapd_msg_register_cb(mapd_msg_cb_func func)
{
	mapd_msg_cb = func;
}

static void mapd_ctrl_iface_msg_cb(void *ctx, const char *txt, size_t len)
{
	struct mapd_global *global = ctx;

	if (global == NULL)
		return;

	struct ctrl_iface_global_priv *priv = global->ctrl_iface;
	if (!dl_list_empty(&priv->ctrl_dst)) {
		mapd_ctrl_iface_send(global,
					       priv->sock,
					       &priv->ctrl_dst,
					       txt, len,
					       priv);
	}
	
}

struct ctrl_iface_global_priv *mapd_global_ctrl_iface_init(struct mapd_global *global)
{
	struct ctrl_iface_global_priv *priv;

	priv = os_zalloc(sizeof(*priv));
	if (priv == NULL)
		return NULL;
	dl_list_init(&priv->ctrl_dst);
	dl_list_init(&priv->msg_queue);
	priv->global = global;
	priv->sock = -1;

	if (global->params.ctrl_interface == NULL)
		return priv;

	if (mapd_global_ctrl_iface_open_sock(global, priv) < 0) {
		os_free(priv);
		return NULL;
	}
	mapd_msg_register_cb(mapd_ctrl_iface_msg_cb);
	return priv;
}

void mapd_global_ctrl_iface_deinit(struct ctrl_iface_global_priv *priv)
{

	if (priv->sock >= 0) {
		eloop_unregister_read_sock(priv->sock);
		close(priv->sock);
	}
	if (priv->global->params.ctrl_interface)
		unlink(priv->global->params.ctrl_interface);
	os_free(priv);
}

/**
 * mapd_ctrl_iface_send - Send a control interface packet to monitors
 * @sock: Local socket fd
 * @ctrl_dst: List of attached listeners
 * @buf: Message data
 * @len: Message length
 *
 * Send a packet to all monitor programs attached to the control interface.
 */
void mapd_ctrl_iface_send(struct mapd_global *global,
					   int sock,
					   struct dl_list *ctrl_dst,
					   const char *buf,
					   size_t len,
					   struct ctrl_iface_global_priv *gp)
{
	struct mapd_ctrl_dst *dst, *next;
	int idx;
	struct msghdr msg;
	struct iovec io[5];

	if (sock < 0 || dl_list_empty(ctrl_dst))
		return;

	idx = 0;
	io[idx].iov_base = (char *) buf;
	io[idx].iov_len = len;
	idx++;
	os_memset(&msg, 0, sizeof(msg));
	msg.msg_iov = io;
	msg.msg_iovlen = idx;

	dl_list_for_each_safe(dst, next, ctrl_dst, struct mapd_ctrl_dst, list) {
		int _errno;
		char addr_txt[200];

		printf_encode(addr_txt, sizeof(addr_txt),
			      (u8 *) dst->addr.sun_path, dst->addrlen -
			      offsetof(struct sockaddr_un, sun_path));
		msg.msg_name = (void *) &dst->addr;
		msg.msg_namelen = dst->addrlen;
		if (sendmsg(sock, &msg, MSG_DONTWAIT) >= 0) {
			mapd_printf(MSG_DEBUG, "CTRL_IFACE monitor sent successfully to %s",
				   addr_txt);
			dst->errors = 0;
			continue;
		}

		_errno = errno;
		mapd_printf(MSG_DEBUG, "CTRL_IFACE monitor[%s]: %d - %s",
			   addr_txt, errno, strerror(errno));
		dst->errors++;

		if (dst->errors > 10 || _errno == ENOENT || _errno == EPERM) {
			mapd_printf(MSG_INFO, "CTRL_IFACE: Detach monitor %s that cannot receive messages",
				addr_txt);
			mapd_ctrl_iface_detach(ctrl_dst, &dst->addr,
							 dst->addrlen);
		}

		if (_errno == ENOBUFS || _errno == EAGAIN) {
			/*
			 * The socket send buffer could be full. This may happen
			 * if client programs are not receiving their pending
			 * messages. Close and reopen the socket as a workaround
			 * to avoid getting stuck being unable to send any new
			 * responses.
			 */
			if (gp)
				sock = mapd_ctrl_iface_global_reinit(
					global, gp);
			else
				break;
			if (sock < 0) {
				mapd_printf(MSG_DEBUG,
					"Failed to reinitialize ctrl_iface socket");
				break;
			}
		}
	}
}

void mapd_msg_ctrl(void *ctx, const char *fmt, ...)
{
	va_list ap;
	char *buf;
	int buflen;
	int len;

	if (!mapd_msg_cb)
		return;

	va_start(ap, fmt);
	buflen = vsnprintf(NULL, 0, fmt, ap) + 1;
	va_end(ap);

	buf = os_malloc(buflen);
	if (buf == NULL) {
		mapd_printf(MSG_ERROR, "mapd_msg_ctrl: Failed to allocate "
			   "message buffer");
		return;
	}
	va_start(ap, fmt);
	len = vsnprintf(buf, buflen, fmt, ap);
	va_end(ap);
	mapd_msg_cb(ctx, buf, len);
	os_free(buf);
}



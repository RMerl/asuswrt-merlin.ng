/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2004-2011  Marcel Holtmann <marcel@holtmann.org>
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

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"

#include "btio/btio.h"
#include "log.h"
#include "sdp-client.h"

/* Number of seconds to keep a sdp_session_t in the cache */
#define CACHE_TIMEOUT 2

struct cached_sdp_session {
	bdaddr_t src;
	bdaddr_t dst;
	sdp_session_t *session;
	guint timer;
	guint io_id;
};

static GSList *cached_sdp_sessions = NULL;

static void cleanup_cached_session(struct cached_sdp_session *cached)
{
	cached_sdp_sessions = g_slist_remove(cached_sdp_sessions, cached);
	sdp_close(cached->session);
	g_free(cached);
}

static gboolean cached_session_expired(gpointer user_data)
{
	struct cached_sdp_session *cached = user_data;

	g_source_remove(cached->io_id);
	cleanup_cached_session(cached);

	return FALSE;
}

static sdp_session_t *get_cached_sdp_session(const bdaddr_t *src,
							const bdaddr_t *dst)
{
	GSList *l;

	for (l = cached_sdp_sessions; l != NULL; l = l->next) {
		struct cached_sdp_session *c = l->data;
		sdp_session_t *session;

		if (bacmp(&c->src, src) || bacmp(&c->dst, dst))
			continue;

		g_source_remove(c->timer);
		g_source_remove(c->io_id);

		session = c->session;

		cached_sdp_sessions = g_slist_remove(cached_sdp_sessions, c);
		g_free(c);

		return session;
	}

	return NULL;
}

static gboolean disconnect_watch(GIOChannel *chan, GIOCondition cond,
							gpointer user_data)
{
	struct cached_sdp_session *cached = user_data;

	g_source_remove(cached->timer);
	cleanup_cached_session(cached);

	return FALSE;
}

static void cache_sdp_session(bdaddr_t *src, bdaddr_t *dst,
						sdp_session_t *session)
{
	struct cached_sdp_session *cached;
	int sk;
	GIOChannel *chan;

	cached = g_new0(struct cached_sdp_session, 1);

	bacpy(&cached->src, src);
	bacpy(&cached->dst, dst);

	cached->session = session;

	cached_sdp_sessions = g_slist_append(cached_sdp_sessions, cached);

	cached->timer = g_timeout_add_seconds(CACHE_TIMEOUT,
						cached_session_expired,
						cached);

	/* Watch the connection state during cache timeout */
	sk = sdp_get_socket(session);
	chan = g_io_channel_unix_new(sk);

	cached->io_id = g_io_add_watch(chan, G_IO_HUP | G_IO_ERR | G_IO_NVAL,
						disconnect_watch, cached);

	g_io_channel_unref(chan);
}

struct search_context {
	bdaddr_t		src;
	bdaddr_t		dst;
	sdp_session_t		*session;
	bt_callback_t		cb;
	bt_destroy_t		destroy;
	gpointer		user_data;
	uuid_t			uuid;
	guint			io_id;
};

static GSList *context_list = NULL;

static void search_context_cleanup(struct search_context *ctxt)
{
	context_list = g_slist_remove(context_list, ctxt);

	if (ctxt->destroy)
		ctxt->destroy(ctxt->user_data);

	g_free(ctxt);
}

static void search_completed_cb(uint8_t type, uint16_t status,
			uint8_t *rsp, size_t size, void *user_data)
{
	struct search_context *ctxt = user_data;
	sdp_list_t *recs = NULL;
	int scanned, seqlen = 0, bytesleft = size;
	uint8_t dataType;
	int err = 0;

	if (status || type != SDP_SVC_SEARCH_ATTR_RSP) {
		err = -EPROTO;
		goto done;
	}

	scanned = sdp_extract_seqtype(rsp, bytesleft, &dataType, &seqlen);
	if (!scanned || !seqlen)
		goto done;

	rsp += scanned;
	bytesleft -= scanned;
	do {
		sdp_record_t *rec;
		int recsize;

		recsize = 0;
		rec = sdp_extract_pdu(rsp, bytesleft, &recsize);
		if (!rec)
			break;

		if (!recsize) {
			sdp_record_free(rec);
			break;
		}

		scanned += recsize;
		rsp += recsize;
		bytesleft -= recsize;

		recs = sdp_list_append(recs, rec);
	} while (scanned < (ssize_t) size && bytesleft > 0);

done:
	cache_sdp_session(&ctxt->src, &ctxt->dst, ctxt->session);

	if (ctxt->cb)
		ctxt->cb(recs, err, ctxt->user_data);

	if (recs)
		sdp_list_free(recs, (sdp_free_func_t) sdp_record_free);

	search_context_cleanup(ctxt);
}

static gboolean search_process_cb(GIOChannel *chan, GIOCondition cond,
							gpointer user_data)
{
	struct search_context *ctxt = user_data;

	if (cond & (G_IO_ERR | G_IO_HUP | G_IO_NVAL)) {
		sdp_close(ctxt->session);
		ctxt->session = NULL;

		if (ctxt->cb)
			ctxt->cb(NULL, -EIO, ctxt->user_data);

		search_context_cleanup(ctxt);
		return FALSE;
	}

	/* If sdp_process fails it calls search_completed_cb */
	if (sdp_process(ctxt->session) < 0)
		return FALSE;

	return TRUE;
}

static gboolean connect_watch(GIOChannel *chan, GIOCondition cond,
							gpointer user_data)
{
	struct search_context *ctxt = user_data;
	sdp_list_t *search, *attrids;
	uint32_t range = 0x0000ffff;
	socklen_t len;
	int sk, err, sk_err = 0;

	sk = g_io_channel_unix_get_fd(chan);
	ctxt->io_id = 0;

	len = sizeof(sk_err);
	if (getsockopt(sk, SOL_SOCKET, SO_ERROR, &sk_err, &len) < 0)
		err = -errno;
	else
		err = -sk_err;

	if (err != 0)
		goto failed;

	if (sdp_set_notify(ctxt->session, search_completed_cb, ctxt) < 0) {
		err = -EIO;
		goto failed;
	}

	search = sdp_list_append(NULL, &ctxt->uuid);
	attrids = sdp_list_append(NULL, &range);
	if (sdp_service_search_attr_async(ctxt->session,
				search, SDP_ATTR_REQ_RANGE, attrids) < 0) {
		sdp_list_free(attrids, NULL);
		sdp_list_free(search, NULL);
		err = -EIO;
		goto failed;
	}

	sdp_list_free(attrids, NULL);
	sdp_list_free(search, NULL);

	/* Set callback responsible for update the internal SDP transaction */
	ctxt->io_id = g_io_add_watch(chan,
				G_IO_IN | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				search_process_cb, ctxt);
	return FALSE;

failed:
	sdp_close(ctxt->session);
	ctxt->session = NULL;

	if (ctxt->cb)
		ctxt->cb(NULL, err, ctxt->user_data);

	search_context_cleanup(ctxt);

	return FALSE;
}

static int create_search_context(struct search_context **ctxt,
					const bdaddr_t *src,
					const bdaddr_t *dst,
					uuid_t *uuid, uint16_t flags)
{
	sdp_session_t *s;
	GIOChannel *chan;
	uint32_t prio = 1;
	int sk;

	if (!ctxt)
		return -EINVAL;

	s = get_cached_sdp_session(src, dst);
	if (!s)
		s = sdp_connect(src, dst, SDP_NON_BLOCKING | flags);

	if (!s)
		return -errno;

	*ctxt = g_try_malloc0(sizeof(struct search_context));
	if (!*ctxt) {
		sdp_close(s);
		return -ENOMEM;
	}

	bacpy(&(*ctxt)->src, src);
	bacpy(&(*ctxt)->dst, dst);
	(*ctxt)->session = s;
	(*ctxt)->uuid = *uuid;

	sk = sdp_get_socket(s);
	/* Set low priority for the SDP connection not to interfere with
	 * other potential traffic.
	 */
	if (setsockopt(sk, SOL_SOCKET, SO_PRIORITY, &prio, sizeof(prio)) < 0)
		warn("Setting SDP priority failed: %s (%d)",
						strerror(errno), errno);

	chan = g_io_channel_unix_new(sk);
	(*ctxt)->io_id = g_io_add_watch(chan,
				G_IO_OUT | G_IO_HUP | G_IO_ERR | G_IO_NVAL,
				connect_watch, *ctxt);
	g_io_channel_unref(chan);

	return 0;
}

int bt_search_service(const bdaddr_t *src, const bdaddr_t *dst,
			uuid_t *uuid, bt_callback_t cb, void *user_data,
			bt_destroy_t destroy, uint16_t flags)
{
	struct search_context *ctxt = NULL;
	int err;

	if (!cb)
		return -EINVAL;

	err = create_search_context(&ctxt, src, dst, uuid, flags);
	if (err < 0)
		return err;

	ctxt->cb	= cb;
	ctxt->destroy	= destroy;
	ctxt->user_data	= user_data;

	context_list = g_slist_append(context_list, ctxt);

	return 0;
}

static int find_by_bdaddr(gconstpointer data, gconstpointer user_data)
{
	const struct search_context *ctxt = data, *search = user_data;
	int ret;

	ret = bacmp(&ctxt->src, &search->src);
	if (ret != 0)
		return ret;

	return bacmp(&ctxt->dst, &search->dst);
}

int bt_cancel_discovery(const bdaddr_t *src, const bdaddr_t *dst)
{
	struct search_context match, *ctxt;
	GSList *l;

	memset(&match, 0, sizeof(match));
	bacpy(&match.src, src);
	bacpy(&match.dst, dst);

	/* Ongoing SDP Discovery */
	l = g_slist_find_custom(context_list, &match, find_by_bdaddr);
	if (l == NULL)
		return -ENOENT;

	ctxt = l->data;

	if (!ctxt->session)
		return -ENOTCONN;

	if (ctxt->io_id)
		g_source_remove(ctxt->io_id);

	if (ctxt->session)
		sdp_close(ctxt->session);

	search_context_cleanup(ctxt);

	return 0;
}

void bt_clear_cached_session(const bdaddr_t *src, const bdaddr_t *dst)
{
	sdp_session_t *session;

	session = get_cached_sdp_session(src, dst);
	if (session)
		sdp_close(session);
}

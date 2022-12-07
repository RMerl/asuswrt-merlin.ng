// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/sdp_lib.h"
#include "lib/uuid.h"

#include "btio/btio.h"
#include "src/btd.h"
#include "src/log.h"
#include "src/shared/util.h"
#include "src/shared/queue.h"
#include "src/adapter.h"
#include "src/device.h"

#include "avdtp.h"
#include "sink.h"
#include "source.h"

#define AVDTP_PSM 25

#define MAX_SEID 0x3E
static unsigned int seids;

#ifndef MAX
# define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

#define AVDTP_DISCOVER				0x01
#define AVDTP_GET_CAPABILITIES			0x02
#define AVDTP_SET_CONFIGURATION			0x03
#define AVDTP_GET_CONFIGURATION			0x04
#define AVDTP_RECONFIGURE			0x05
#define AVDTP_OPEN				0x06
#define AVDTP_START				0x07
#define AVDTP_CLOSE				0x08
#define AVDTP_SUSPEND				0x09
#define AVDTP_ABORT				0x0A
#define AVDTP_SECURITY_CONTROL			0x0B
#define AVDTP_GET_ALL_CAPABILITIES		0x0C
#define AVDTP_DELAY_REPORT			0x0D

#define AVDTP_PKT_TYPE_SINGLE			0x00
#define AVDTP_PKT_TYPE_START			0x01
#define AVDTP_PKT_TYPE_CONTINUE			0x02
#define AVDTP_PKT_TYPE_END			0x03

#define AVDTP_MSG_TYPE_COMMAND			0x00
#define AVDTP_MSG_TYPE_GEN_REJECT		0x01
#define AVDTP_MSG_TYPE_ACCEPT			0x02
#define AVDTP_MSG_TYPE_REJECT			0x03

#define REQ_TIMEOUT 6
#define SUSPEND_TIMEOUT 10
#define ABORT_TIMEOUT 2
#define DISCONNECT_TIMEOUT 1
#define START_TIMEOUT 1

#if __BYTE_ORDER == __LITTLE_ENDIAN

struct avdtp_common_header {
	uint8_t message_type:2;
	uint8_t packet_type:2;
	uint8_t transaction:4;
} __attribute__ ((packed));

struct avdtp_single_header {
	uint8_t message_type:2;
	uint8_t packet_type:2;
	uint8_t transaction:4;
	uint8_t signal_id:6;
	uint8_t rfa0:2;
} __attribute__ ((packed));

struct avdtp_start_header {
	uint8_t message_type:2;
	uint8_t packet_type:2;
	uint8_t transaction:4;
	uint8_t no_of_packets;
	uint8_t signal_id:6;
	uint8_t rfa0:2;
} __attribute__ ((packed));

struct avdtp_continue_header {
	uint8_t message_type:2;
	uint8_t packet_type:2;
	uint8_t transaction:4;
} __attribute__ ((packed));

struct seid_info {
	uint8_t rfa0:1;
	uint8_t inuse:1;
	uint8_t seid:6;
	uint8_t rfa2:3;
	uint8_t type:1;
	uint8_t media_type:4;
} __attribute__ ((packed));

struct seid {
	uint8_t rfa0:2;
	uint8_t seid:6;
} __attribute__ ((packed));

#elif __BYTE_ORDER == __BIG_ENDIAN

struct avdtp_common_header {
	uint8_t transaction:4;
	uint8_t packet_type:2;
	uint8_t message_type:2;
} __attribute__ ((packed));

struct avdtp_single_header {
	uint8_t transaction:4;
	uint8_t packet_type:2;
	uint8_t message_type:2;
	uint8_t rfa0:2;
	uint8_t signal_id:6;
} __attribute__ ((packed));

struct avdtp_start_header {
	uint8_t transaction:4;
	uint8_t packet_type:2;
	uint8_t message_type:2;
	uint8_t no_of_packets;
	uint8_t rfa0:2;
	uint8_t signal_id:6;
} __attribute__ ((packed));

struct avdtp_continue_header {
	uint8_t transaction:4;
	uint8_t packet_type:2;
	uint8_t message_type:2;
} __attribute__ ((packed));

struct seid_info {
	uint8_t seid:6;
	uint8_t inuse:1;
	uint8_t rfa0:1;
	uint8_t media_type:4;
	uint8_t type:1;
	uint8_t rfa2:3;
} __attribute__ ((packed));

struct seid {
	uint8_t seid:6;
	uint8_t rfa0:2;
} __attribute__ ((packed));

#else
#error "Unknown byte order"
#endif

/* packets */

struct discover_resp {
	struct seid_info seps[0];
} __attribute__ ((packed));

struct getcap_resp {
	uint8_t caps[0];
} __attribute__ ((packed));

struct start_req {
	struct seid first_seid;
	struct seid other_seids[0];
} __attribute__ ((packed));

struct suspend_req {
	struct seid first_seid;
	struct seid other_seids[0];
} __attribute__ ((packed));

struct seid_rej {
	uint8_t error;
} __attribute__ ((packed));

struct conf_rej {
	uint8_t category;
	uint8_t error;
} __attribute__ ((packed));

#if __BYTE_ORDER == __LITTLE_ENDIAN

struct seid_req {
	uint8_t rfa0:2;
	uint8_t acp_seid:6;
} __attribute__ ((packed));

struct setconf_req {
	uint8_t rfa0:2;
	uint8_t acp_seid:6;
	uint8_t rfa1:2;
	uint8_t int_seid:6;

	uint8_t caps[0];
} __attribute__ ((packed));

struct stream_rej {
	uint8_t rfa0:2;
	uint8_t acp_seid:6;
	uint8_t error;
} __attribute__ ((packed));

struct reconf_req {
	uint8_t rfa0:2;
	uint8_t acp_seid:6;

	uint8_t serv_cap;
	uint8_t serv_cap_len;

	uint8_t caps[0];
} __attribute__ ((packed));

struct delay_req {
	uint8_t rfa0:2;
	uint8_t acp_seid:6;
	uint16_t delay;
} __attribute__ ((packed));

#elif __BYTE_ORDER == __BIG_ENDIAN

struct seid_req {
	uint8_t acp_seid:6;
	uint8_t rfa0:2;
} __attribute__ ((packed));

struct setconf_req {
	uint8_t acp_seid:6;
	uint8_t rfa0:2;
	uint8_t int_seid:6;
	uint8_t rfa1:2;

	uint8_t caps[0];
} __attribute__ ((packed));

struct stream_rej {
	uint8_t acp_seid:6;
	uint8_t rfa0:2;
	uint8_t error;
} __attribute__ ((packed));

struct reconf_req {
	uint8_t acp_seid:6;
	uint8_t rfa0:2;

	uint8_t serv_cap;
	uint8_t serv_cap_len;

	uint8_t caps[0];
} __attribute__ ((packed));

struct delay_req {
	uint8_t acp_seid:6;
	uint8_t rfa0:2;
	uint16_t delay;
} __attribute__ ((packed));

#else
#error "Unknown byte order"
#endif

struct in_buf {
	gboolean active;
	int no_of_packets;
	uint8_t transaction;
	uint8_t message_type;
	uint8_t signal_id;
	uint8_t buf[1024];
	uint8_t data_size;
};

struct pending_req {
	uint8_t transaction;
	uint8_t signal_id;
	void *data;
	size_t data_size;
	struct avdtp_stream *stream; /* Set if the request targeted a stream */
	guint timeout;
	gboolean collided;
};

struct avdtp_remote_sep {
	uint8_t seid;
	uint8_t type;
	uint8_t media_type;
	struct avdtp_service_capability *codec;
	gboolean delay_reporting;
	bool discovered;
	GSList *caps; /* of type struct avdtp_service_capability */
	struct avdtp_stream *stream;
	avdtp_remote_sep_destroy_t destroy;
	void *user_data;
};

struct avdtp_local_sep {
	avdtp_state_t state;
	struct avdtp_stream *stream;
	struct seid_info info;
	uint8_t codec;
	gboolean delay_reporting;
	GSList *caps;
	struct avdtp_sep_ind *ind;
	struct avdtp_sep_cfm *cfm;
	void *user_data;
};

struct stream_callback {
	avdtp_stream_state_cb cb;
	void *user_data;
	unsigned int id;
};

struct avdtp_state_callback {
	avdtp_session_state_cb cb;
	struct btd_device *dev;
	unsigned int id;
	void *user_data;
};

struct discover_callback {
	unsigned int id;
	avdtp_discover_cb_t cb;
	void *user_data;
};

struct avdtp_stream {
	GIOChannel *io;
	uint16_t imtu;
	uint16_t omtu;
	struct avdtp *session;
	struct avdtp_local_sep *lsep;
	uint8_t rseid;
	GSList *caps;
	GSList *callbacks;
	struct avdtp_service_capability *codec;
	guint io_id;		/* Transport GSource ID */
	guint timer;		/* Waiting for other side to close or open
				 * the transport channel */
	gboolean open_acp;	/* If we are in ACT role for Open */
	gboolean close_int;	/* If we are in INT role for Close */
	gboolean abort_int;	/* If we are in INT role for Abort */
	guint start_timer;	/* Wait START command timer */
	gboolean delay_reporting;
	uint16_t delay;		/* AVDTP 1.3 Delay Reporting feature */
	gboolean starting;	/* only valid while sep state == OPEN */
};

/* Structure describing an AVDTP connection between two devices */

struct avdtp {
	unsigned int ref;

	uint16_t version;

	struct queue *lseps;
	struct btd_device *device;

	avdtp_session_state_t state;

	GIOChannel *io;
	guint io_id;

	GSList *seps; /* Elements of type struct avdtp_remote_sep * */

	GSList *streams; /* Elements of type struct avdtp_stream * */

	GSList *req_queue; /* Elements of type struct pending_req * */
	GSList *prio_queue; /* Same as req_queue but is processed before it */

	struct avdtp_stream *pending_open;
	GIOChannel *pending_open_io;

	uint32_t phy;
	uint16_t imtu;
	uint16_t omtu;

	struct in_buf in;

	char *buf;

	struct discover_callback *discover;
	struct pending_req *req;

	guint dc_timer;
	int dc_timeout;

	/* Attempt stream setup instead of disconnecting */
	gboolean stream_setup;
};

static GSList *state_callbacks = NULL;

static int send_request(struct avdtp *session, gboolean priority,
			struct avdtp_stream *stream, uint8_t signal_id,
			void *buffer, size_t size);
static gboolean avdtp_parse_resp(struct avdtp *session,
					struct avdtp_stream *stream,
					uint8_t transaction, uint8_t signal_id,
					void *buf, int size);
static gboolean avdtp_parse_rej(struct avdtp *session,
					struct avdtp_stream *stream,
					uint8_t transaction, uint8_t signal_id,
					void *buf, int size);
static int process_queue(struct avdtp *session);
static void avdtp_sep_set_state(struct avdtp *session,
				struct avdtp_local_sep *sep,
				avdtp_state_t state);

static const char *avdtp_statestr(avdtp_state_t state)
{
	switch (state) {
	case AVDTP_STATE_IDLE:
		return "IDLE";
	case AVDTP_STATE_CONFIGURED:
		return "CONFIGURED";
	case AVDTP_STATE_OPEN:
		return "OPEN";
	case AVDTP_STATE_STREAMING:
		return "STREAMING";
	case AVDTP_STATE_CLOSING:
		return "CLOSING";
	case AVDTP_STATE_ABORTING:
		return "ABORTING";
	default:
		return "<unknown state>";
	}
}

static gboolean try_send(int sk, void *data, size_t len)
{
	int err;

	do {
		err = send(sk, data, len, 0);
	} while (err < 0 && errno == EINTR);

	if (err < 0) {
		error("send: %s (%d)", strerror(errno), errno);
		return FALSE;
	} else if ((size_t) err != len) {
		error("try_send: complete buffer not sent (%d/%zu bytes)",
								err, len);
		return FALSE;
	}

	return TRUE;
}

static gboolean avdtp_send(struct avdtp *session, uint8_t transaction,
				uint8_t message_type, uint8_t signal_id,
				void *data, size_t len)
{
	unsigned int cont_fragments, sent;
	struct avdtp_start_header start;
	struct avdtp_continue_header cont;
	int sock;

	if (session->io == NULL) {
		error("avdtp_send: session is closed");
		return FALSE;
	}

	sock = g_io_channel_unix_get_fd(session->io);

	/* Single packet - no fragmentation */
	if (sizeof(struct avdtp_single_header) + len <= session->omtu) {
		struct avdtp_single_header single;

		memset(&single, 0, sizeof(single));

		single.transaction = transaction;
		single.packet_type = AVDTP_PKT_TYPE_SINGLE;
		single.message_type = message_type;
		single.signal_id = signal_id;

		memcpy(session->buf, &single, sizeof(single));
		memcpy(session->buf + sizeof(single), data, len);

		return try_send(sock, session->buf, sizeof(single) + len);
	}

	/* Check if there is enough space to start packet */
	if (session->omtu < sizeof(start)) {
		error("No enough space to fragment packet");
		return FALSE;
	}

	/* Count the number of needed fragments */
	cont_fragments = (len - (session->omtu - sizeof(start))) /
					(session->omtu - sizeof(cont)) + 1;

	DBG("%zu bytes split into %d fragments", len, cont_fragments + 1);

	/* Send the start packet */
	memset(&start, 0, sizeof(start));
	start.transaction = transaction;
	start.packet_type = AVDTP_PKT_TYPE_START;
	start.message_type = message_type;
	start.no_of_packets = cont_fragments + 1;
	start.signal_id = signal_id;

	memcpy(session->buf, &start, sizeof(start));
	memcpy(session->buf + sizeof(start), data,
					session->omtu - sizeof(start));

	if (!try_send(sock, session->buf, session->omtu))
		return FALSE;

	DBG("first packet with %zu bytes sent", session->omtu - sizeof(start));

	sent = session->omtu - sizeof(start);

	/* Send the continue fragments and the end packet */
	while (sent < len) {
		int left, to_copy;

		left = len - sent;
		if (left + sizeof(cont) > session->omtu) {
			cont.packet_type = AVDTP_PKT_TYPE_CONTINUE;
			to_copy = session->omtu - sizeof(cont);
			DBG("sending continue with %d bytes", to_copy);
		} else {
			cont.packet_type = AVDTP_PKT_TYPE_END;
			to_copy = left;
			DBG("sending end with %d bytes", to_copy);
		}

		cont.transaction = transaction;
		cont.message_type = message_type;

		memcpy(session->buf, &cont, sizeof(cont));
		memcpy(session->buf + sizeof(cont), data + sent, to_copy);

		if (!try_send(sock, session->buf, to_copy + sizeof(cont)))
			return FALSE;

		sent += to_copy;
	}

	return TRUE;
}

static void pending_req_free(void *data)
{
	struct pending_req *req = data;

	if (req->timeout)
		g_source_remove(req->timeout);
	g_free(req->data);
	g_free(req);
}

static void close_stream(struct avdtp_stream *stream)
{
	int sock;

	if (stream->io == NULL)
		return;

	sock = g_io_channel_unix_get_fd(stream->io);

	shutdown(sock, SHUT_RDWR);

	g_io_channel_shutdown(stream->io, FALSE, NULL);

	g_io_channel_unref(stream->io);
	stream->io = NULL;
}

static gboolean stream_close_timeout(gpointer user_data)
{
	struct avdtp_stream *stream = user_data;

	DBG("Timed out waiting for peer to close the transport channel");

	stream->timer = 0;

	close_stream(stream);

	return FALSE;
}

static gboolean stream_open_timeout(gpointer user_data)
{
	struct avdtp_stream *stream = user_data;

	DBG("Timed out waiting for peer to open the transport channel");

	stream->timer = 0;

	stream->session->pending_open = NULL;

	if (stream->session->pending_open_io) {
		g_io_channel_unref(stream->session->pending_open_io);
		stream->session->pending_open_io = NULL;
	}

	avdtp_abort(stream->session, stream);

	return FALSE;
}

static void stream_set_timer(struct avdtp_stream *stream, guint timeout,
							GSourceFunc func)
{
	if (stream->timer)
		g_source_remove(stream->timer);

	stream->timer = g_timeout_add_seconds(timeout, func, stream);
}

static void stream_set_pending_open(struct avdtp_stream *stream, GIOChannel *io)
{
	stream->open_acp = TRUE;
	stream->session->pending_open = stream;
	stream->session->pending_open_io = io;
	stream_set_timer(stream, REQ_TIMEOUT, stream_open_timeout);
}

void avdtp_error_init(struct avdtp_error *err, uint8_t category, int id)
{
	err->category = category;

	if (category == AVDTP_ERRNO)
		err->err.posix_errno = id;
	else
		err->err.error_code = id;
}

uint8_t avdtp_error_category(struct avdtp_error *err)
{
	return err->category;
}

int avdtp_error_error_code(struct avdtp_error *err)
{
	assert(err->category != AVDTP_ERRNO);
	return err->err.error_code;
}

int avdtp_error_posix_errno(struct avdtp_error *err)
{
	assert(err->category == AVDTP_ERRNO);
	return err->err.posix_errno;
}

static struct avdtp_stream *find_stream_by_rseid(struct avdtp *session,
							uint8_t rseid)
{
	GSList *l;

	for (l = session->streams; l != NULL; l = g_slist_next(l)) {
		struct avdtp_stream *stream = l->data;

		if (stream->rseid == rseid)
			return stream;
	}

	return NULL;
}

static struct avdtp_remote_sep *find_remote_sep(GSList *seps, uint8_t seid)
{
	GSList *l;

	for (l = seps; l != NULL; l = g_slist_next(l)) {
		struct avdtp_remote_sep *sep = l->data;

		if (sep->seid == seid)
			return sep;
	}

	return NULL;
}

static void avdtp_set_state(struct avdtp *session,
					avdtp_session_state_t new_state)
{
	GSList *l;
	avdtp_session_state_t old_state = session->state;

	session->state = new_state;

	for (l = state_callbacks; l != NULL;) {
		struct avdtp_state_callback *cb = l->data;

		l = g_slist_next(l);

		if (session->device != cb->dev)
			continue;

		cb->cb(cb->dev, session, old_state, new_state, cb->user_data);
	}
}

static void stream_free(void *data)
{
	struct avdtp_stream *stream = data;
	struct avdtp_remote_sep *rsep;

	stream->lsep->info.inuse = 0;
	stream->lsep->stream = NULL;

	rsep = find_remote_sep(stream->session->seps, stream->rseid);
	if (rsep)
		rsep->stream = NULL;

	if (stream->timer)
		g_source_remove(stream->timer);

	if (stream->io)
		close_stream(stream);

	if (stream->io_id)
		g_source_remove(stream->io_id);

	g_slist_free_full(stream->callbacks, g_free);
	g_slist_free_full(stream->caps, g_free);

	g_free(stream);
}

static gboolean transport_cb(GIOChannel *chan, GIOCondition cond,
				gpointer data)
{
	struct avdtp_stream *stream = data;
	struct avdtp_local_sep *sep = stream->lsep;

	if (stream->close_int && sep->cfm && sep->cfm->close)
		sep->cfm->close(stream->session, sep, stream, NULL,
				sep->user_data);

	if (!(cond & G_IO_NVAL))
		close_stream(stream);

	stream->io_id = 0;

	if (!stream->abort_int)
		avdtp_sep_set_state(stream->session, sep, AVDTP_STATE_IDLE);

	return FALSE;
}

static int get_send_buffer_size(int sk)
{
	int size;
	socklen_t optlen = sizeof(size);

	if (getsockopt(sk, SOL_SOCKET, SO_SNDBUF, &size, &optlen) < 0) {
		int err = -errno;
		error("getsockopt(SO_SNDBUF) failed: %s (%d)", strerror(-err),
									-err);
		return err;
	}

	/*
	 * Doubled value is returned by getsockopt since kernel uses that
	 * space for its own purposes (see man 7 socket, bookkeeping overhead
	 * for SO_SNDBUF).
	 */
	return size / 2;
}

static int set_send_buffer_size(int sk, int size)
{
	socklen_t optlen = sizeof(size);

	if (setsockopt(sk, SOL_SOCKET, SO_SNDBUF, &size, optlen) < 0) {
		int err = -errno;
		error("setsockopt(SO_SNDBUF) failed: %s (%d)", strerror(-err),
									-err);
		return err;
	}

	return 0;
}

static void handle_transport_connect(struct avdtp *session, GIOChannel *io,
					uint16_t imtu, uint16_t omtu)
{
	struct avdtp_stream *stream = session->pending_open;
	struct avdtp_local_sep *sep = stream->lsep;
	int sk, buf_size, min_buf_size;
	GError *err = NULL;

	session->pending_open = NULL;

	if (stream->timer) {
		g_source_remove(stream->timer);
		stream->timer = 0;
	}

	if (io == NULL) {
		if (!stream->open_acp && sep->cfm && sep->cfm->open) {
			struct avdtp_error err;
			avdtp_error_init(&err, AVDTP_ERRNO, EIO);
			sep->cfm->open(session, sep, NULL, &err,
					sep->user_data);
		}
		return;
	}

	if (stream->io == NULL)
		stream->io = g_io_channel_ref(io);

	stream->omtu = omtu;
	stream->imtu = imtu;

	/* Apply special settings only if local SEP is of type SRC */
	if (sep->info.type != AVDTP_SEP_TYPE_SOURCE)
		goto proceed;

	bt_io_set(stream->io, &err, BT_IO_OPT_FLUSHABLE, TRUE,
							BT_IO_OPT_INVALID);
	if (err != NULL) {
		error("Enabling flushable packets failed: %s", err->message);
		g_error_free(err);
	} else
		DBG("Flushable packets enabled");

	sk = g_io_channel_unix_get_fd(stream->io);
	buf_size = get_send_buffer_size(sk);
	if (buf_size < 0)
		goto proceed;

	DBG("sk %d, omtu %d, send buffer size %d", sk, omtu, buf_size);
	min_buf_size = omtu * 2;
	if (buf_size < min_buf_size) {
		DBG("send buffer size to be increassed to %d",
				min_buf_size);
		set_send_buffer_size(sk, min_buf_size);
	}

proceed:
	if (!stream->open_acp && sep->cfm && sep->cfm->open)
		sep->cfm->open(session, sep, stream, NULL, sep->user_data);

	avdtp_sep_set_state(session, sep, AVDTP_STATE_OPEN);

	stream->io_id = g_io_add_watch(io, G_IO_ERR | G_IO_HUP | G_IO_NVAL,
					(GIOFunc) transport_cb, stream);

	/* Release pending IO */
	if (session->pending_open_io) {
		g_io_channel_unref(session->pending_open_io);
		session->pending_open_io = NULL;
	}
}

static int pending_req_cmp(gconstpointer a, gconstpointer b)
{
	const struct pending_req *req = a;
	const struct avdtp_stream *stream = b;

	if (req->stream == stream)
		return 0;

	return -1;
}

static void cleanup_queue(struct avdtp *session, struct avdtp_stream *stream)
{
	GSList *l;
	struct pending_req *req;

	while ((l = g_slist_find_custom(session->prio_queue, stream,
							pending_req_cmp))) {
		req = l->data;
		pending_req_free(req);
		session->prio_queue = g_slist_remove(session->prio_queue, req);
	}

	while ((l = g_slist_find_custom(session->req_queue, stream,
							pending_req_cmp))) {
		req = l->data;
		pending_req_free(req);
		session->req_queue = g_slist_remove(session->req_queue, req);
	}
}

static void handle_unanswered_req(struct avdtp *session,
						struct avdtp_stream *stream)
{
	struct pending_req *req;
	struct avdtp_local_sep *lsep;
	struct avdtp_error err;

	if (session->req->signal_id == AVDTP_ABORT) {
		/* Avoid freeing the Abort request here */
		DBG("handle_unanswered_req: Abort req, returning");
		session->req->stream = NULL;
		return;
	}

	req = session->req;
	session->req = NULL;

	avdtp_error_init(&err, AVDTP_ERRNO, EIO);

	lsep = stream->lsep;

	switch (req->signal_id) {
	case AVDTP_RECONFIGURE:
		error("No reply to Reconfigure request");
		if (lsep && lsep->cfm && lsep->cfm->reconfigure)
			lsep->cfm->reconfigure(session, lsep, stream, &err,
						lsep->user_data);
		break;
	case AVDTP_OPEN:
		error("No reply to Open request");
		if (lsep && lsep->cfm && lsep->cfm->open)
			lsep->cfm->open(session, lsep, stream, &err,
					lsep->user_data);
		break;
	case AVDTP_START:
		error("No reply to Start request");
		if (lsep && lsep->cfm && lsep->cfm->start)
			lsep->cfm->start(session, lsep, stream, &err,
						lsep->user_data);
		break;
	case AVDTP_SUSPEND:
		error("No reply to Suspend request");
		if (lsep && lsep->cfm && lsep->cfm->suspend)
			lsep->cfm->suspend(session, lsep, stream, &err,
						lsep->user_data);
		break;
	case AVDTP_CLOSE:
		error("No reply to Close request");
		if (lsep && lsep->cfm && lsep->cfm->close)
			lsep->cfm->close(session, lsep, stream, &err,
						lsep->user_data);
		break;
	case AVDTP_SET_CONFIGURATION:
		error("No reply to SetConfiguration request");
		if (lsep && lsep->cfm && lsep->cfm->set_configuration)
			lsep->cfm->set_configuration(session, lsep, stream,
							&err, lsep->user_data);
	}

	pending_req_free(req);
}

static void avdtp_sep_set_state(struct avdtp *session,
				struct avdtp_local_sep *sep,
				avdtp_state_t state)
{
	struct avdtp_stream *stream = sep->stream;
	avdtp_state_t old_state;
	struct avdtp_error err, *err_ptr = NULL;
	GSList *l;

	if (!stream) {
		error("Error changing sep state: stream not available");
		return;
	}

	if (sep->state == state) {
		avdtp_error_init(&err, AVDTP_ERRNO, EIO);
		DBG("stream state change failed: %s", avdtp_strerror(&err));
		err_ptr = &err;
	} else {
		err_ptr = NULL;
		DBG("stream state changed: %s -> %s",
				avdtp_statestr(sep->state),
				avdtp_statestr(state));
	}

	old_state = sep->state;
	sep->state = state;

	switch (state) {
	case AVDTP_STATE_CONFIGURED:
		if (sep->info.type == AVDTP_SEP_TYPE_SINK)
			avdtp_delay_report(session, stream, stream->delay);
		break;
	case AVDTP_STATE_OPEN:
		stream->starting = FALSE;
		break;
	case AVDTP_STATE_STREAMING:
		if (stream->start_timer) {
			g_source_remove(stream->start_timer);
			stream->start_timer = 0;
		}
		stream->open_acp = FALSE;
		break;
	case AVDTP_STATE_CLOSING:
	case AVDTP_STATE_ABORTING:
		if (stream->start_timer) {
			g_source_remove(stream->start_timer);
			stream->start_timer = 0;
		}
		break;
	case AVDTP_STATE_IDLE:
		if (stream->start_timer) {
			g_source_remove(stream->start_timer);
			stream->start_timer = 0;
		}
		if (session->pending_open == stream)
			handle_transport_connect(session, NULL, 0, 0);
		if (session->req && session->req->stream == stream)
			handle_unanswered_req(session, stream);
		/* Remove pending commands for this stream from the queue */
		cleanup_queue(session, stream);
		session->streams = g_slist_remove(session->streams, stream);
		break;
	default:
		break;
	}

	l = stream->callbacks;
	while (l != NULL) {
		struct stream_callback *cb = l->data;
		l = g_slist_next(l);
		cb->cb(stream, old_state, state, err_ptr, cb->user_data);
	}

	if (state == AVDTP_STATE_IDLE)
		stream_free(stream);
}

static void sep_free(gpointer data)
{
	struct avdtp_remote_sep *sep = data;

	if (sep->destroy)
		sep->destroy(sep->user_data);

	g_slist_free_full(sep->caps, g_free);
	g_free(sep);
}

static void remove_disappeared(void *data, void *user_data)
{
	struct avdtp_remote_sep *sep = data;
	struct avdtp *session = user_data;

	if (sep->discovered)
		return;

	DBG("seid %d disappeared", sep->seid);

	session->seps = g_slist_remove(session->seps, sep);

	sep_free(sep);
}

static void finalize_discovery(struct avdtp *session, int err)
{
	struct discover_callback *discover = session->discover;
	struct avdtp_error avdtp_err;

	if (!discover)
		return;

	avdtp_error_init(&avdtp_err, AVDTP_ERRNO, err);

	if (discover->id > 0)
		g_source_remove(discover->id);

	if (!err)
		g_slist_foreach(session->seps, remove_disappeared, session);

	if (discover->cb)
		discover->cb(session, session->seps, err ? &avdtp_err : NULL,
							discover->user_data);
	g_free(discover);
	session->discover = NULL;
}

static void release_stream(struct avdtp_stream *stream, struct avdtp *session)
{
	struct avdtp_local_sep *sep = stream->lsep;

	if (sep->cfm && sep->cfm->abort &&
				(sep->state != AVDTP_STATE_ABORTING ||
							stream->abort_int))
		sep->cfm->abort(session, sep, stream, NULL, sep->user_data);

	avdtp_sep_set_state(session, sep, AVDTP_STATE_IDLE);
}

static void remove_disconnect_timer(struct avdtp *session)
{
	if (!session->dc_timer)
		return;

	g_source_remove(session->dc_timer);
	session->dc_timer = 0;
	session->stream_setup = FALSE;

	/* Release disconnect timer reference */
	avdtp_unref(session);
}

static void avdtp_free(void *data)
{
	struct avdtp *session = data;

	DBG("%p", session);

	g_slist_free_full(session->streams, stream_free);

	if (session->io) {
		g_io_channel_shutdown(session->io, FALSE, NULL);
		g_io_channel_unref(session->io);
	}

	if (session->io_id) {
		g_source_remove(session->io_id);
		session->io_id = 0;
	}

	if (session->req)
		pending_req_free(session->req);

	g_slist_free_full(session->req_queue, pending_req_free);
	g_slist_free_full(session->prio_queue, pending_req_free);
	g_slist_free_full(session->seps, sep_free);

	g_free(session->buf);

	btd_device_unref(session->device);
	g_free(session);
}

static void connection_lost(struct avdtp *session, int err)
{
	char address[18];

	session = avdtp_ref(session);

	ba2str(device_get_address(session->device), address);
	DBG("Disconnected from %s", address);

	g_slist_foreach(session->streams, (GFunc) release_stream, session);
	session->streams = NULL;

	finalize_discovery(session, err);

	avdtp_set_state(session, AVDTP_SESSION_STATE_DISCONNECTED);

	avdtp_unref(session);
}

static gboolean disconnect_timeout(gpointer user_data)
{
	struct avdtp *session = user_data;
	struct btd_service *service;
	gboolean stream_setup;

	session->dc_timer = 0;

	stream_setup = session->stream_setup;
	session->stream_setup = FALSE;

	service = btd_device_get_service(session->device, A2DP_SINK_UUID);
	if (service && stream_setup) {
		sink_setup_stream(service, session);
		goto done;
	}

	service = btd_device_get_service(session->device, A2DP_SOURCE_UUID);
	if (service && stream_setup) {
		source_setup_stream(service, session);
		goto done;
	}

	connection_lost(session, ETIMEDOUT);

done:
	/* Release disconnect timer reference */
	avdtp_unref(session);

	return FALSE;
}

static void set_disconnect_timer(struct avdtp *session)
{
	/* Take a ref while disconnect timer is active */
	avdtp_ref(session);

	DBG("timeout %d", session->dc_timeout);

	if (!session->dc_timeout)
		session->dc_timer = g_idle_add(disconnect_timeout, session);
	else
		session->dc_timer = g_timeout_add_seconds(session->dc_timeout,
							disconnect_timeout,
							session);
}

void avdtp_unref(struct avdtp *session)
{
	if (!session)
		return;

	session->ref--;

	DBG("%p: ref=%d", session, session->ref);

	if (session->ref > 0)
		return;

	switch (session->state) {
	case AVDTP_SESSION_STATE_CONNECTED:
		/* Only set disconnect timer if there are local endpoints
		 * otherwise disconnect immediately.
		 */
		if (queue_isempty(session->lseps))
			connection_lost(session, ECONNRESET);
		else
			set_disconnect_timer(session);
		break;
	case AVDTP_SESSION_STATE_CONNECTING:
		connection_lost(session, ECONNABORTED);
		break;
	case AVDTP_SESSION_STATE_DISCONNECTED:
	default:
		avdtp_free(session);
		break;
	}
}

struct avdtp *avdtp_ref(struct avdtp *session)
{
	session->ref++;
	DBG("%p: ref=%d", session, session->ref);
	remove_disconnect_timer(session);
	return session;
}

static bool match_by_seid(const void *data, const void *user_data)
{
	const struct avdtp_local_sep *sep = data;
	uint8_t seid = PTR_TO_UINT(user_data);

	return sep->info.seid == seid;
}

static struct avdtp_local_sep *find_local_sep_by_seid(struct avdtp *session,
								uint8_t seid)
{
	return queue_find(session->lseps, match_by_seid, INT_TO_PTR(seid));
}

struct avdtp_remote_sep *avdtp_find_remote_sep(struct avdtp *session,
						struct avdtp_local_sep *lsep)
{
	GSList *l;

	if (lsep->info.inuse)
		return NULL;

	for (l = session->seps; l != NULL; l = g_slist_next(l)) {
		struct avdtp_remote_sep *sep = l->data;
		struct avdtp_service_capability *cap;
		struct avdtp_media_codec_capability *codec_data;

		/* Type must be different: source <-> sink */
		if (sep->type == lsep->info.type)
			continue;

		if (sep->media_type != lsep->info.media_type)
			continue;

		if (!sep->codec)
			continue;

		cap = sep->codec;
		codec_data = (void *) cap->data;

		if (codec_data->media_codec_type != lsep->codec)
			continue;

		if (lsep->ind && lsep->ind->match_codec)
			if (!lsep->ind->match_codec(session, codec_data,
							lsep->user_data))
				continue;

		if (sep->stream == NULL)
			return sep;
	}

	return NULL;
}

static GSList *caps_to_list(uint8_t *data, int size,
				struct avdtp_service_capability **codec,
				gboolean *delay_reporting)
{
	GSList *caps;
	int processed;

	if (delay_reporting)
		*delay_reporting = FALSE;

	for (processed = 0, caps = NULL; processed + 2 <= size;) {
		struct avdtp_service_capability *cap;
		uint8_t length, category;

		category = data[0];
		length = data[1];

		if (processed + 2 + length > size) {
			error("Invalid capability data in getcap resp");
			break;
		}

		cap = g_malloc(sizeof(struct avdtp_service_capability) +
					length);
		memcpy(cap, data, 2 + length);

		processed += 2 + length;
		data += 2 + length;

		caps = g_slist_append(caps, cap);

		if (category == AVDTP_MEDIA_CODEC &&
				length >=
				sizeof(struct avdtp_media_codec_capability))
			*codec = cap;
		else if (category == AVDTP_DELAY_REPORTING && delay_reporting)
			*delay_reporting = TRUE;
	}

	return caps;
}

static gboolean avdtp_unknown_cmd(struct avdtp *session, uint8_t transaction,
							uint8_t signal_id)
{
	return avdtp_send(session, transaction, AVDTP_MSG_TYPE_GEN_REJECT,
							signal_id, NULL, 0);
}

static void copy_seps(void *data, void *user_data)
{
	struct avdtp_local_sep *sep = data;
	struct seid_info **p = user_data;

	memcpy(*p, &sep->info, sizeof(struct seid_info));
	*p = *p + 1;
}

static gboolean avdtp_discover_cmd(struct avdtp *session, uint8_t transaction,
							void *buf, int size)
{
	unsigned int rsp_size, sep_count;
	struct seid_info *seps, *p;
	gboolean ret;

	sep_count = queue_length(session->lseps);

	if (sep_count == 0) {
		uint8_t err = AVDTP_NOT_SUPPORTED_COMMAND;
		return avdtp_send(session, transaction, AVDTP_MSG_TYPE_REJECT,
					AVDTP_DISCOVER, &err, sizeof(err));
	}

	rsp_size = sep_count * sizeof(struct seid_info);

	seps = g_new0(struct seid_info, sep_count);
	p = seps;

	queue_foreach(session->lseps, copy_seps, &p);

	ret = avdtp_send(session, transaction, AVDTP_MSG_TYPE_ACCEPT,
				AVDTP_DISCOVER, seps, rsp_size);
	g_free(seps);

	return ret;
}

static gboolean avdtp_getcap_cmd(struct avdtp *session, uint8_t transaction,
					struct seid_req *req, unsigned int size,
					gboolean get_all)
{
	GSList *l, *caps;
	struct avdtp_local_sep *sep = NULL;
	unsigned int rsp_size;
	uint8_t err, buf[1024], *ptr = buf;
	uint8_t cmd;

	cmd = get_all ? AVDTP_GET_ALL_CAPABILITIES : AVDTP_GET_CAPABILITIES;

	if (size < sizeof(struct seid_req)) {
		err = AVDTP_BAD_LENGTH;
		goto failed;
	}

	sep = find_local_sep_by_seid(session, req->acp_seid);
	if (!sep) {
		err = AVDTP_BAD_ACP_SEID;
		goto failed;
	}

	if (!sep->ind->get_capability(session, sep, get_all, &caps,
							&err, sep->user_data))
		goto failed;

	for (l = caps, rsp_size = 0; l != NULL; l = g_slist_next(l)) {
		struct avdtp_service_capability *cap = l->data;

		if (rsp_size + cap->length + 2 > sizeof(buf))
			break;

		memcpy(ptr, cap, cap->length + 2);
		rsp_size += cap->length + 2;
		ptr += cap->length + 2;

		g_free(cap);
	}

	g_slist_free(caps);

	return avdtp_send(session, transaction, AVDTP_MSG_TYPE_ACCEPT, cmd,
								buf, rsp_size);

failed:
	return avdtp_send(session, transaction, AVDTP_MSG_TYPE_REJECT, cmd,
							&err, sizeof(err));
}

static void setconf_cb(struct avdtp *session, struct avdtp_stream *stream,
						struct avdtp_error *err)
{
	struct conf_rej rej;
	struct avdtp_local_sep *sep;

	if (err != NULL) {
		rej.error = AVDTP_UNSUPPORTED_CONFIGURATION;
		rej.category = err->err.error_code;
		avdtp_send(session, session->in.transaction,
				AVDTP_MSG_TYPE_REJECT, AVDTP_SET_CONFIGURATION,
				&rej, sizeof(rej));
		stream_free(stream);
		return;
	}

	if (!avdtp_send(session, session->in.transaction, AVDTP_MSG_TYPE_ACCEPT,
					AVDTP_SET_CONFIGURATION, NULL, 0)) {
		stream_free(stream);
		return;
	}

	sep = stream->lsep;
	sep->stream = stream;
	sep->info.inuse = 1;
	session->streams = g_slist_append(session->streams, stream);

	avdtp_sep_set_state(session, sep, AVDTP_STATE_CONFIGURED);
}

static gboolean avdtp_setconf_cmd(struct avdtp *session, uint8_t transaction,
				struct setconf_req *req, unsigned int size)
{
	struct conf_rej rej;
	struct avdtp_local_sep *sep;
	struct avdtp_stream *stream;
	uint8_t err, category = 0x00;
	struct btd_service *service;
	GSList *l;

	if (size < sizeof(struct setconf_req)) {
		error("Too short getcap request");
		return FALSE;
	}

	sep = find_local_sep_by_seid(session, req->acp_seid);
	if (!sep) {
		err = AVDTP_BAD_ACP_SEID;
		goto failed;
	}

	if (sep->stream) {
		err = AVDTP_SEP_IN_USE;
		goto failed;
	}

	switch (sep->info.type) {
	case AVDTP_SEP_TYPE_SOURCE:
		service = btd_device_get_service(session->device,
							A2DP_SINK_UUID);
		if (service == NULL) {
			btd_device_add_uuid(session->device, A2DP_SINK_UUID);
			service = btd_device_get_service(session->device,
							A2DP_SINK_UUID);
			if (service == NULL) {
				error("Unable to get a audio sink object");
				err = AVDTP_BAD_STATE;
				goto failed;
			}
		}
		break;
	case AVDTP_SEP_TYPE_SINK:
		service = btd_device_get_service(session->device,
							A2DP_SOURCE_UUID);
		if (service == NULL) {
			btd_device_add_uuid(session->device, A2DP_SOURCE_UUID);
			service = btd_device_get_service(session->device,
							A2DP_SOURCE_UUID);
			if (service == NULL) {
				error("Unable to get a audio source object");
				err = AVDTP_BAD_STATE;
				goto failed;
			}
		}
		break;
	}

	stream = g_new0(struct avdtp_stream, 1);
	stream->session = session;
	stream->lsep = sep;
	stream->rseid = req->int_seid;
	stream->caps = caps_to_list(req->caps,
					size - sizeof(struct setconf_req),
					&stream->codec,
					&stream->delay_reporting);

	/* Verify that the Media Transport capability's length = 0. Reject otherwise */
	for (l = stream->caps; l != NULL; l = g_slist_next(l)) {
		struct avdtp_service_capability *cap = l->data;

		if (cap->category == AVDTP_MEDIA_TRANSPORT && cap->length != 0) {
			err = AVDTP_BAD_MEDIA_TRANSPORT_FORMAT;
			goto failed_stream;
		}
	}

	if (stream->delay_reporting && session->version < 0x0103)
		session->version = 0x0103;

	if (sep->ind && sep->ind->set_configuration) {
		if (!sep->ind->set_configuration(session, sep, stream,
							stream->caps,
							setconf_cb,
							sep->user_data)) {
			err = AVDTP_UNSUPPORTED_CONFIGURATION;
			category = 0x00;
			goto failed_stream;
		}
	} else {
		if (!avdtp_send(session, transaction, AVDTP_MSG_TYPE_ACCEPT,
					AVDTP_SET_CONFIGURATION, NULL, 0)) {
			stream_free(stream);
			return FALSE;
		}

		sep->stream = stream;
		sep->info.inuse = 1;
		session->streams = g_slist_append(session->streams, stream);

		avdtp_sep_set_state(session, sep, AVDTP_STATE_CONFIGURED);
	}

	return TRUE;

failed_stream:
	stream_free(stream);
failed:
	rej.error = err;
	rej.category = category;
	return avdtp_send(session, transaction, AVDTP_MSG_TYPE_REJECT,
				AVDTP_SET_CONFIGURATION, &rej, sizeof(rej));
}

static gboolean avdtp_getconf_cmd(struct avdtp *session, uint8_t transaction,
					struct seid_req *req, int size)
{
	GSList *l;
	struct avdtp_local_sep *sep = NULL;
	int rsp_size;
	uint8_t err;
	uint8_t buf[1024];
	uint8_t *ptr = buf;

	if (size < (int) sizeof(struct seid_req)) {
		error("Too short getconf request");
		return FALSE;
	}

	memset(buf, 0, sizeof(buf));

	sep = find_local_sep_by_seid(session, req->acp_seid);
	if (!sep) {
		err = AVDTP_BAD_ACP_SEID;
		goto failed;
	}
	if (!sep->stream || !sep->stream->caps) {
		err = AVDTP_UNSUPPORTED_CONFIGURATION;
		goto failed;
	}

	for (l = sep->stream->caps, rsp_size = 0; l != NULL; l = g_slist_next(l)) {
		struct avdtp_service_capability *cap = l->data;

		if (rsp_size + cap->length + 2 > (int) sizeof(buf))
			break;

		memcpy(ptr, cap, cap->length + 2);
		rsp_size += cap->length + 2;
		ptr += cap->length + 2;
	}

	return avdtp_send(session, transaction, AVDTP_MSG_TYPE_ACCEPT,
				AVDTP_GET_CONFIGURATION, buf, rsp_size);

failed:
	return avdtp_send(session, transaction, AVDTP_MSG_TYPE_REJECT,
				AVDTP_GET_CONFIGURATION, &err, sizeof(err));
}

static gboolean avdtp_reconf_cmd(struct avdtp *session, uint8_t transaction,
					struct seid_req *req, int size)
{
	struct conf_rej rej;

	rej.error = AVDTP_NOT_SUPPORTED_COMMAND;
	rej.category = 0x00;

	return avdtp_send(session, transaction, AVDTP_MSG_TYPE_REJECT,
					AVDTP_RECONFIGURE, &rej, sizeof(rej));
}

static void check_seid_collision(struct pending_req *req, uint8_t id)
{
	struct seid_req *seid = req->data;

	if (seid->acp_seid == id)
		req->collided = TRUE;
}

static void check_start_collision(struct pending_req *req, uint8_t id)
{
	struct start_req *start = req->data;
	struct seid *seid = &start->first_seid;
	int count = 1 + req->data_size - sizeof(struct start_req);
	int i;

	for (i = 0; i < count; i++, seid++) {
		if (seid->seid == id) {
			req->collided = TRUE;
			return;
		}
	}
}

static void check_suspend_collision(struct pending_req *req, uint8_t id)
{
	struct suspend_req *suspend = req->data;
	struct seid *seid = &suspend->first_seid;
	int count = 1 + req->data_size - sizeof(struct suspend_req);
	int i;

	for (i = 0; i < count; i++, seid++) {
		if (seid->seid == id) {
			req->collided = TRUE;
			return;
		}
	}
}

static void avdtp_check_collision(struct avdtp *session, uint8_t cmd,
					struct avdtp_stream *stream)
{
	struct pending_req *req = session->req;

	if (req == NULL || (req->signal_id != cmd && cmd != AVDTP_ABORT))
		return;

	if (cmd == AVDTP_ABORT)
		cmd = req->signal_id;

	switch (cmd) {
	case AVDTP_OPEN:
	case AVDTP_CLOSE:
		check_seid_collision(req, stream->rseid);
		break;
	case AVDTP_START:
		check_start_collision(req, stream->rseid);
		break;
	case AVDTP_SUSPEND:
		check_suspend_collision(req, stream->rseid);
		break;
	}
}

static gboolean avdtp_open_cmd(struct avdtp *session, uint8_t transaction,
				struct seid_req *req, unsigned int size)
{
	struct avdtp_local_sep *sep;
	struct avdtp_stream *stream;
	uint8_t err;

	if (size < sizeof(struct seid_req)) {
		error("Too short abort request");
		return FALSE;
	}

	sep = find_local_sep_by_seid(session, req->acp_seid);
	if (!sep) {
		err = AVDTP_BAD_ACP_SEID;
		goto failed;
	}

	if (sep->state != AVDTP_STATE_CONFIGURED) {
		err = AVDTP_BAD_STATE;
		goto failed;
	}

	stream = sep->stream;

	/* Check if the stream is pending and there is an IO set already */
	if (stream == session->pending_open && session->pending_open_io) {
		handle_transport_connect(session, session->pending_open_io,
						stream->imtu, stream->omtu);
		return avdtp_send(session, transaction, AVDTP_MSG_TYPE_ACCEPT,
							AVDTP_OPEN, NULL, 0);
	}

	if (sep->ind && sep->ind->open && !session->pending_open) {
		if (!sep->ind->open(session, sep, stream, &err,
					sep->user_data))
			goto failed;
	}

	avdtp_check_collision(session, AVDTP_OPEN, stream);

	if (!avdtp_send(session, transaction, AVDTP_MSG_TYPE_ACCEPT,
						AVDTP_OPEN, NULL, 0))
		return FALSE;

	if (!session->pending_open)
		stream_set_pending_open(stream, NULL);

	return TRUE;

failed:
	return avdtp_send(session, transaction, AVDTP_MSG_TYPE_REJECT,
				AVDTP_OPEN, &err, sizeof(err));
}

static gboolean avdtp_start_cmd(struct avdtp *session, uint8_t transaction,
				struct start_req *req, unsigned int size)
{
	struct avdtp_local_sep *sep;
	struct avdtp_stream *stream;
	struct stream_rej rej;
	struct seid *seid;
	uint8_t err, failed_seid;
	int seid_count, i;

	if (size < sizeof(struct start_req)) {
		error("Too short start request");
		return FALSE;
	}

	seid_count = 1 + size - sizeof(struct start_req);

	seid = &req->first_seid;

	for (i = 0; i < seid_count; i++, seid++) {
		failed_seid = seid->seid;

		sep = find_local_sep_by_seid(session, seid->seid);
		if (!sep || !sep->stream) {
			err = AVDTP_BAD_ACP_SEID;
			goto failed;
		}

		stream = sep->stream;

		/* Also reject start cmd if state is not open */
		if (sep->state != AVDTP_STATE_OPEN) {
			err = AVDTP_BAD_STATE;
			goto failed;
		}
		stream->starting = TRUE;

		if (sep->ind && sep->ind->start) {
			if (!sep->ind->start(session, sep, stream, &err,
						sep->user_data))
				goto failed;
		}

		avdtp_check_collision(session, AVDTP_START, stream);

		avdtp_sep_set_state(session, sep, AVDTP_STATE_STREAMING);
	}

	return avdtp_send(session, transaction, AVDTP_MSG_TYPE_ACCEPT,
						AVDTP_START, NULL, 0);

failed:
	DBG("Rejecting (%d)", err);
	memset(&rej, 0, sizeof(rej));
	rej.acp_seid = failed_seid;
	rej.error = err;
	return avdtp_send(session, transaction, AVDTP_MSG_TYPE_REJECT,
				AVDTP_START, &rej, sizeof(rej));
}

static gboolean avdtp_close_cmd(struct avdtp *session, uint8_t transaction,
				struct seid_req *req, unsigned int size)
{
	struct avdtp_local_sep *sep;
	struct avdtp_stream *stream;
	uint8_t err;

	if (size < sizeof(struct seid_req)) {
		error("Too short close request");
		return FALSE;
	}

	sep = find_local_sep_by_seid(session, req->acp_seid);
	if (!sep || !sep->stream) {
		err = AVDTP_BAD_ACP_SEID;
		goto failed;
	}

	if (sep->state != AVDTP_STATE_OPEN &&
			sep->state != AVDTP_STATE_STREAMING) {
		err = AVDTP_BAD_STATE;
		goto failed;
	}

	stream = sep->stream;

	if (sep->ind && sep->ind->close) {
		if (!sep->ind->close(session, sep, stream, &err,
					sep->user_data))
			goto failed;
	}

	avdtp_check_collision(session, AVDTP_CLOSE, stream);

	avdtp_sep_set_state(session, sep, AVDTP_STATE_CLOSING);

	session->dc_timeout = DISCONNECT_TIMEOUT;

	if (!avdtp_send(session, transaction, AVDTP_MSG_TYPE_ACCEPT,
						AVDTP_CLOSE, NULL, 0))
		return FALSE;

	stream->timer = g_timeout_add_seconds(REQ_TIMEOUT,
					stream_close_timeout,
					stream);

	return TRUE;

failed:
	return avdtp_send(session, transaction, AVDTP_MSG_TYPE_REJECT,
					AVDTP_CLOSE, &err, sizeof(err));
}

static gboolean avdtp_suspend_cmd(struct avdtp *session, uint8_t transaction,
				struct suspend_req *req, unsigned int size)
{
	struct avdtp_local_sep *sep;
	struct avdtp_stream *stream;
	struct stream_rej rej;
	struct seid *seid;
	uint8_t err, failed_seid;
	int seid_count, i;

	if (size < sizeof(struct suspend_req)) {
		error("Too short suspend request");
		return FALSE;
	}

	seid_count = 1 + size - sizeof(struct suspend_req);

	seid = &req->first_seid;

	for (i = 0; i < seid_count; i++, seid++) {
		failed_seid = seid->seid;

		sep = find_local_sep_by_seid(session, seid->seid);
		if (!sep || !sep->stream) {
			err = AVDTP_BAD_ACP_SEID;
			goto failed;
		}

		stream = sep->stream;

		if (sep->state != AVDTP_STATE_STREAMING) {
			err = AVDTP_BAD_STATE;
			goto failed;
		}

		if (sep->ind && sep->ind->suspend) {
			if (!sep->ind->suspend(session, sep, stream, &err,
						sep->user_data))
				goto failed;
		}

		avdtp_check_collision(session, AVDTP_SUSPEND, stream);

		avdtp_sep_set_state(session, sep, AVDTP_STATE_OPEN);
	}

	return avdtp_send(session, transaction, AVDTP_MSG_TYPE_ACCEPT,
						AVDTP_SUSPEND, NULL, 0);

failed:
	memset(&rej, 0, sizeof(rej));
	rej.acp_seid = failed_seid;
	rej.error = err;
	return avdtp_send(session, transaction, AVDTP_MSG_TYPE_REJECT,
				AVDTP_SUSPEND, &rej, sizeof(rej));
}

static gboolean avdtp_abort_cmd(struct avdtp *session, uint8_t transaction,
				struct seid_req *req, unsigned int size)
{
	struct avdtp_local_sep *sep;
	uint8_t err;
	gboolean ret;

	if (size < sizeof(struct seid_req)) {
		error("Too short abort request");
		return FALSE;
	}

	sep = find_local_sep_by_seid(session, req->acp_seid);
	if (!sep || !sep->stream)
		return TRUE;

	if (sep->ind && sep->ind->abort)
		sep->ind->abort(session, sep, sep->stream, &err,
							sep->user_data);

	avdtp_check_collision(session, AVDTP_ABORT, sep->stream);

	ret = avdtp_send(session, transaction, AVDTP_MSG_TYPE_ACCEPT,
						AVDTP_ABORT, NULL, 0);
	if (ret) {
		avdtp_sep_set_state(session, sep, AVDTP_STATE_ABORTING);
		session->dc_timeout = DISCONNECT_TIMEOUT;
	}

	return ret;
}

static gboolean avdtp_secctl_cmd(struct avdtp *session, uint8_t transaction,
					struct seid_req *req, int size)
{
	return avdtp_unknown_cmd(session, transaction, AVDTP_SECURITY_CONTROL);
}

static gboolean avdtp_delayreport_cmd(struct avdtp *session,
					uint8_t transaction,
					struct delay_req *req,
					unsigned int size)
{
	struct avdtp_local_sep *sep;
	struct avdtp_stream *stream;
	uint8_t err;

	if (size < sizeof(struct delay_req)) {
		error("Too short delay report request");
		return FALSE;
	}

	sep = find_local_sep_by_seid(session, req->acp_seid);
	if (!sep || !sep->stream) {
		err = AVDTP_BAD_ACP_SEID;
		goto failed;
	}

	stream = sep->stream;

	if (sep->state != AVDTP_STATE_CONFIGURED &&
					sep->state != AVDTP_STATE_OPEN &&
					sep->state != AVDTP_STATE_STREAMING) {
		err = AVDTP_BAD_STATE;
		goto failed;
	}

	stream->delay = ntohs(req->delay);

	if (sep->ind && sep->ind->delayreport) {
		if (!sep->ind->delayreport(session, sep, stream->rseid,
						stream->delay, &err,
						sep->user_data))
			goto failed;
	}

	return avdtp_send(session, transaction, AVDTP_MSG_TYPE_ACCEPT,
						AVDTP_DELAY_REPORT, NULL, 0);

failed:
	return avdtp_send(session, transaction, AVDTP_MSG_TYPE_REJECT,
					AVDTP_DELAY_REPORT, &err, sizeof(err));
}

static gboolean avdtp_parse_cmd(struct avdtp *session, uint8_t transaction,
				uint8_t signal_id, void *buf, int size)
{
	switch (signal_id) {
	case AVDTP_DISCOVER:
		DBG("Received DISCOVER_CMD");
		return avdtp_discover_cmd(session, transaction, buf, size);
	case AVDTP_GET_CAPABILITIES:
		DBG("Received  GET_CAPABILITIES_CMD");
		return avdtp_getcap_cmd(session, transaction, buf, size,
									FALSE);
	case AVDTP_GET_ALL_CAPABILITIES:
		DBG("Received  GET_ALL_CAPABILITIES_CMD");
		return avdtp_getcap_cmd(session, transaction, buf, size, TRUE);
	case AVDTP_SET_CONFIGURATION:
		DBG("Received SET_CONFIGURATION_CMD");
		return avdtp_setconf_cmd(session, transaction, buf, size);
	case AVDTP_GET_CONFIGURATION:
		DBG("Received GET_CONFIGURATION_CMD");
		return avdtp_getconf_cmd(session, transaction, buf, size);
	case AVDTP_RECONFIGURE:
		DBG("Received RECONFIGURE_CMD");
		return avdtp_reconf_cmd(session, transaction, buf, size);
	case AVDTP_OPEN:
		DBG("Received OPEN_CMD");
		return avdtp_open_cmd(session, transaction, buf, size);
	case AVDTP_START:
		DBG("Received START_CMD");
		return avdtp_start_cmd(session, transaction, buf, size);
	case AVDTP_CLOSE:
		DBG("Received CLOSE_CMD");
		return avdtp_close_cmd(session, transaction, buf, size);
	case AVDTP_SUSPEND:
		DBG("Received SUSPEND_CMD");
		return avdtp_suspend_cmd(session, transaction, buf, size);
	case AVDTP_ABORT:
		DBG("Received ABORT_CMD");
		return avdtp_abort_cmd(session, transaction, buf, size);
	case AVDTP_SECURITY_CONTROL:
		DBG("Received SECURITY_CONTROL_CMD");
		return avdtp_secctl_cmd(session, transaction, buf, size);
	case AVDTP_DELAY_REPORT:
		DBG("Received DELAY_REPORT_CMD");
		return avdtp_delayreport_cmd(session, transaction, buf, size);
	default:
		DBG("Received unknown request id %u", signal_id);
		return avdtp_unknown_cmd(session, transaction, signal_id);
	}
}

enum avdtp_parse_result { PARSE_ERROR, PARSE_FRAGMENT, PARSE_SUCCESS };

static enum avdtp_parse_result avdtp_parse_data(struct avdtp *session,
							void *buf, size_t size)
{
	struct avdtp_common_header *header = buf;
	struct avdtp_single_header *single = (void *) session->buf;
	struct avdtp_start_header *start = (void *) session->buf;
	void *payload;
	gsize payload_size;

	switch (header->packet_type) {
	case AVDTP_PKT_TYPE_SINGLE:
		if (size < sizeof(*single)) {
			error("Received too small single packet (%zu bytes)", size);
			return PARSE_ERROR;
		}
		if (session->in.active) {
			error("SINGLE: Invalid AVDTP packet fragmentation");
			return PARSE_ERROR;
		}

		payload = session->buf + sizeof(*single);
		payload_size = size - sizeof(*single);

		session->in.active = TRUE;
		session->in.data_size = 0;
		session->in.no_of_packets = 1;
		session->in.transaction = header->transaction;
		session->in.message_type = header->message_type;
		session->in.signal_id = single->signal_id;

		break;
	case AVDTP_PKT_TYPE_START:
		if (size < sizeof(*start)) {
			error("Received too small start packet (%zu bytes)", size);
			return PARSE_ERROR;
		}
		if (session->in.active) {
			error("START: Invalid AVDTP packet fragmentation");
			return PARSE_ERROR;
		}

		session->in.active = TRUE;
		session->in.data_size = 0;
		session->in.transaction = header->transaction;
		session->in.message_type = header->message_type;
		session->in.no_of_packets = start->no_of_packets;
		session->in.signal_id = start->signal_id;

		payload = session->buf + sizeof(*start);
		payload_size = size - sizeof(*start);

		break;
	case AVDTP_PKT_TYPE_CONTINUE:
		if (size < sizeof(struct avdtp_continue_header)) {
			error("Received too small continue packet (%zu bytes)",
									size);
			return PARSE_ERROR;
		}
		if (!session->in.active) {
			error("CONTINUE: Invalid AVDTP packet fragmentation");
			return PARSE_ERROR;
		}
		if (session->in.transaction != header->transaction) {
			error("Continue transaction id doesn't match");
			return PARSE_ERROR;
		}
		if (session->in.no_of_packets <= 1) {
			error("Too few continue packets");
			return PARSE_ERROR;
		}

		payload = session->buf + sizeof(struct avdtp_continue_header);
		payload_size = size - sizeof(struct avdtp_continue_header);

		break;
	case AVDTP_PKT_TYPE_END:
		if (size < sizeof(struct avdtp_continue_header)) {
			error("Received too small end packet (%zu bytes)", size);
			return PARSE_ERROR;
		}
		if (!session->in.active) {
			error("END: Invalid AVDTP packet fragmentation");
			return PARSE_ERROR;
		}
		if (session->in.transaction != header->transaction) {
			error("End transaction id doesn't match");
			return PARSE_ERROR;
		}
		if (session->in.no_of_packets > 1) {
			error("Got an end packet too early");
			return PARSE_ERROR;
		}

		payload = session->buf + sizeof(struct avdtp_continue_header);
		payload_size = size - sizeof(struct avdtp_continue_header);

		break;
	default:
		error("Invalid AVDTP packet type 0x%02X", header->packet_type);
		return PARSE_ERROR;
	}

	if (session->in.data_size + payload_size >
					sizeof(session->in.buf)) {
		error("Not enough incoming buffer space!");
		return PARSE_ERROR;
	}

	memcpy(session->in.buf + session->in.data_size, payload, payload_size);
	session->in.data_size += payload_size;

	if (session->in.no_of_packets > 1) {
		session->in.no_of_packets--;
		DBG("Received AVDTP fragment. %d to go",
						session->in.no_of_packets);
		return PARSE_FRAGMENT;
	}

	session->in.active = FALSE;

	return PARSE_SUCCESS;
}

static gboolean session_cb(GIOChannel *chan, GIOCondition cond,
				gpointer data)
{
	struct avdtp *session = data;
	struct avdtp_common_header *header;
	ssize_t size;
	int fd;

	DBG("");

	if (cond & G_IO_NVAL)
		return FALSE;

	header = (void *) session->buf;

	if (cond & (G_IO_HUP | G_IO_ERR))
		goto failed;

	fd = g_io_channel_unix_get_fd(chan);
	size = read(fd, session->buf, session->imtu);
	if (size < 0) {
		error("IO Channel read error");
		goto failed;
	}

	if ((size_t) size < sizeof(struct avdtp_common_header)) {
		error("Received too small packet (%zu bytes)", size);
		goto failed;
	}

	switch (avdtp_parse_data(session, session->buf, size)) {
	case PARSE_ERROR:
		goto failed;
	case PARSE_FRAGMENT:
		return TRUE;
	case PARSE_SUCCESS:
		break;
	}

	if (session->in.message_type == AVDTP_MSG_TYPE_COMMAND) {
		if (!avdtp_parse_cmd(session, session->in.transaction,
					session->in.signal_id,
					session->in.buf,
					session->in.data_size)) {
			error("Unable to handle command. Disconnecting");
			goto failed;
		}

		if (session->req && session->req->collided) {
			DBG("Collision detected");
			goto next;
		}

		return TRUE;
	}

	if (session->req == NULL) {
		error("No pending request, ignoring message");
		return TRUE;
	}

	if (header->transaction != session->req->transaction) {
		error("Transaction label doesn't match");
		return TRUE;
	}

	if (session->in.signal_id != session->req->signal_id) {
		error("Response signal doesn't match");
		return TRUE;
	}

	g_source_remove(session->req->timeout);
	session->req->timeout = 0;

	switch (header->message_type) {
	case AVDTP_MSG_TYPE_ACCEPT:
		if (!avdtp_parse_resp(session, session->req->stream,
						session->in.transaction,
						session->in.signal_id,
						session->in.buf,
						session->in.data_size)) {
			error("Unable to parse accept response");
			goto failed;
		}
		break;
	case AVDTP_MSG_TYPE_REJECT:
		if (!avdtp_parse_rej(session, session->req->stream,
						session->in.transaction,
						session->in.signal_id,
						session->in.buf,
						session->in.data_size)) {
			error("Unable to parse reject response");
			goto failed;
		}
		break;
	case AVDTP_MSG_TYPE_GEN_REJECT:
		error("Received a General Reject message");
		break;
	default:
		error("Unknown message type 0x%02X", header->message_type);
		break;
	}

next:
	pending_req_free(session->req);
	session->req = NULL;

	process_queue(session);

	return TRUE;

failed:
	connection_lost(session, EIO);

	return FALSE;
}

static uint16_t get_version(struct avdtp *session)
{
	const sdp_record_t *rec;
	sdp_list_t *protos;
	sdp_data_t *proto_desc;
	uint16_t ver = 0x0000;

	rec = btd_device_get_record(session->device, A2DP_SINK_UUID);
	if (!rec)
		rec = btd_device_get_record(session->device, A2DP_SOURCE_UUID);

	if (!rec)
		return ver;

	if (sdp_get_access_protos(rec, &protos) < 0)
		return ver;

	proto_desc = sdp_get_proto_desc(protos, AVDTP_UUID);
	if (proto_desc && proto_desc->dtd == SDP_UINT16)
		ver = proto_desc->val.uint16;

	sdp_list_foreach(protos, (sdp_list_func_t) sdp_list_free, NULL);
	sdp_list_free(protos, NULL);

	return ver;
}

static void avdtp_connect_cb(GIOChannel *chan, GError *err, gpointer user_data)
{
	struct avdtp *session = user_data;
	char address[18];
	int err_no = EIO;

	if (err) {
		err_no = err->code;
		error("%s", err->message);
		goto failed;
	}

	if (!session->io)
		session->io = g_io_channel_ref(chan);

	/* Check if kernel supports reading packet types */
	bt_io_get(chan, NULL, BT_IO_OPT_PHY, &session->phy, BT_IO_OPT_INVALID);

	bt_io_get(chan, &err,
			BT_IO_OPT_OMTU, &session->omtu,
			BT_IO_OPT_IMTU, &session->imtu,
			BT_IO_OPT_INVALID);
	if (err) {
		err_no = err->code;
		error("%s", err->message);
		goto failed;
	}

	ba2str(device_get_address(session->device), address);
	DBG("AVDTP: connected %s channel to %s",
			session->pending_open ? "transport" : "signaling",
			address);

	if (session->state == AVDTP_SESSION_STATE_CONNECTING) {
		DBG("AVDTP imtu=%u, omtu=%u", session->imtu, session->omtu);

		session->buf = g_malloc0(MAX(session->imtu, session->omtu));
		avdtp_set_state(session, AVDTP_SESSION_STATE_CONNECTED);

		if (session->io_id)
			g_source_remove(session->io_id);

		session->io_id = g_io_add_watch(chan,
						G_IO_IN | G_IO_ERR | G_IO_HUP
						| G_IO_NVAL,
						(GIOFunc) session_cb, session);

		if (session->stream_setup)
			set_disconnect_timer(session);
	} else if (session->pending_open)
		handle_transport_connect(session, chan, session->imtu,
								session->omtu);
	else
		goto failed;

	process_queue(session);

	return;

failed:
	if (session->pending_open) {
		struct avdtp_stream *stream = session->pending_open;

		handle_transport_connect(session, NULL, 0, 0);

		if (avdtp_abort(session, stream) < 0)
			avdtp_sep_set_state(session, stream->lsep,
						AVDTP_STATE_IDLE);
	} else
		connection_lost(session, err_no);
}

struct avdtp *avdtp_new(GIOChannel *chan, struct btd_device *device,
							struct queue *lseps)
{
	struct avdtp *session;

	session = g_new0(struct avdtp, 1);

	session->device = btd_device_ref(device);
	/* We don't use avdtp_set_state() here since this isn't a state change
	 * but just setting of the initial state */
	session->state = AVDTP_SESSION_STATE_DISCONNECTED;
	session->lseps = lseps;

	session->version = get_version(session);

	if (!chan)
		return session;

	avdtp_set_state(session, AVDTP_SESSION_STATE_CONNECTING);

	btd_device_add_uuid(device, ADVANCED_AUDIO_UUID);

	session->io = g_io_channel_ref(chan);
	session->io_id = g_io_add_watch(chan, G_IO_ERR | G_IO_HUP | G_IO_NVAL,
					(GIOFunc) session_cb, session);

	/* This is so that avdtp_connect_cb will know to do the right thing
	 * with respect to the disconnect timer */
	session->stream_setup = TRUE;

	session->dc_timeout = DISCONNECT_TIMEOUT;

	avdtp_connect_cb(chan, NULL, session);

	return session;
}

uint16_t avdtp_get_version(struct avdtp *session)
{
	return session->version;
}

static GIOChannel *l2cap_connect(struct avdtp *session, BtIOMode mode)
{
	GError *err = NULL;
	GIOChannel *io;
	const bdaddr_t *src;
	src = btd_adapter_get_address(device_get_adapter(session->device));

	if (session->phy)
		io = bt_io_connect(avdtp_connect_cb, session,
					NULL, &err,
					BT_IO_OPT_SOURCE_BDADDR, src,
					BT_IO_OPT_DEST_BDADDR,
					device_get_address(session->device),
					BT_IO_OPT_PSM, AVDTP_PSM,
					BT_IO_OPT_MODE, mode,
					BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_MEDIUM,
					/* Set Input MTU to 0 to auto-tune */
					BT_IO_OPT_IMTU, 0,
					BT_IO_OPT_INVALID);
	else
		io = bt_io_connect(avdtp_connect_cb, session,
					NULL, &err,
					BT_IO_OPT_SOURCE_BDADDR, src,
					BT_IO_OPT_DEST_BDADDR,
					device_get_address(session->device),
					BT_IO_OPT_PSM, AVDTP_PSM,
					BT_IO_OPT_MODE, mode,
					BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_MEDIUM,
					BT_IO_OPT_INVALID);
	if (!io) {
		error("%s", err->message);
		g_error_free(err);
		return NULL;
	}

	return io;
}

static void queue_request(struct avdtp *session, struct pending_req *req,
			gboolean priority)
{
	if (priority)
		session->prio_queue = g_slist_append(session->prio_queue, req);
	else
		session->req_queue = g_slist_append(session->req_queue, req);
}

static uint8_t req_get_seid(struct pending_req *req)
{
	if (req->signal_id == AVDTP_DISCOVER)
		return 0;

	return ((struct seid_req *) (req->data))->acp_seid;
}

static int cancel_request(struct avdtp *session, int err)
{
	struct pending_req *req;
	struct seid_req sreq;
	struct avdtp_local_sep *lsep;
	struct avdtp_stream *stream;
	uint8_t seid;
	struct avdtp_error averr;

	req = session->req;
	session->req = NULL;

	avdtp_error_init(&averr, AVDTP_ERRNO, err);

	seid = req_get_seid(req);
	if (seid)
		stream = find_stream_by_rseid(session, seid);
	else
		stream = NULL;

	if (stream)
		lsep = stream->lsep;
	else
		lsep = NULL;

	switch (req->signal_id) {
	case AVDTP_RECONFIGURE:
		error("Reconfigure: %s (%d)", strerror(err), err);
		if (lsep && lsep->cfm && lsep->cfm->reconfigure)
			lsep->cfm->reconfigure(session, lsep, stream, &averr,
						lsep->user_data);
		break;
	case AVDTP_OPEN:
		error("Open: %s (%d)", strerror(err), err);
		if (lsep && lsep->cfm && lsep->cfm->open)
			lsep->cfm->open(session, lsep, stream, &averr,
					lsep->user_data);
		break;
	case AVDTP_START:
		error("Start: %s (%d)", strerror(err), err);
		if (lsep && lsep->cfm && lsep->cfm->start) {
			lsep->cfm->start(session, lsep, stream, &averr,
						lsep->user_data);
			if (stream)
				stream->starting = FALSE;
		}
		break;
	case AVDTP_SUSPEND:
		error("Suspend: %s (%d)", strerror(err), err);
		if (lsep && lsep->cfm && lsep->cfm->suspend)
			lsep->cfm->suspend(session, lsep, stream, &averr,
						lsep->user_data);
		break;
	case AVDTP_CLOSE:
		error("Close: %s (%d)", strerror(err), err);
		if (lsep && lsep->cfm && lsep->cfm->close) {
			lsep->cfm->close(session, lsep, stream, &averr,
						lsep->user_data);
			if (stream)
				stream->close_int = FALSE;
		}
		break;
	case AVDTP_SET_CONFIGURATION:
		error("SetConfiguration: %s (%d)", strerror(err), err);
		if (lsep && lsep->cfm && lsep->cfm->set_configuration)
			lsep->cfm->set_configuration(session, lsep, stream,
							&averr, lsep->user_data);
		break;
	case AVDTP_DISCOVER:
		error("Discover: %s (%d)", strerror(err), err);
		goto failed;
	case AVDTP_GET_CAPABILITIES:
		error("GetCapabilities: %s (%d)", strerror(err), err);
		goto failed;
	case AVDTP_ABORT:
		error("Abort: %s (%d)", strerror(err), err);
		goto failed;
	}

	if (!stream)
		goto failed;

	memset(&sreq, 0, sizeof(sreq));
	sreq.acp_seid = seid;

	err = send_request(session, TRUE, stream, AVDTP_ABORT, &sreq,
				sizeof(sreq));
	if (err < 0) {
		error("Unable to send abort request");
		goto failed;
	}

	stream->abort_int = TRUE;

	goto done;

failed:
	connection_lost(session, err);
done:
	pending_req_free(req);
	return err;
}

static gboolean request_timeout(gpointer user_data)
{
	struct avdtp *session = user_data;

	cancel_request(session, ETIMEDOUT);

	return FALSE;
}

static int send_req(struct avdtp *session, gboolean priority,
			struct pending_req *req)
{
	static int transaction = 0;
	int err, timeout;

	if (session->state == AVDTP_SESSION_STATE_DISCONNECTED) {
		BtIOMode mode = btd_opts.avdtp.session_mode;

		session->io = l2cap_connect(session, mode);
		if (!session->io) {
			/* Report disconnection anyways, as the other layers
			 * are using this state for cleanup.
			 */
			avdtp_set_state(session,
					AVDTP_SESSION_STATE_DISCONNECTED);
			err = -EIO;
			goto failed;
		}
		avdtp_set_state(session, AVDTP_SESSION_STATE_CONNECTING);
	}

	if (session->state < AVDTP_SESSION_STATE_CONNECTED ||
			session->req != NULL) {
		queue_request(session, req, priority);
		return 0;
	}

	req->transaction = transaction++;
	transaction %= 16;

	/* FIXME: Should we retry to send if the buffer
	was not totally sent or in case of EINTR? */
	if (!avdtp_send(session, req->transaction, AVDTP_MSG_TYPE_COMMAND,
				req->signal_id, req->data, req->data_size)) {
		err = -EIO;
		goto failed;
	}

	session->req = req;

	switch (req->signal_id) {
	case AVDTP_ABORT:
		timeout = ABORT_TIMEOUT;
		break;
	case AVDTP_SUSPEND:
		timeout = SUSPEND_TIMEOUT;
		break;
	default:
		timeout = REQ_TIMEOUT;
	}

	req->timeout = g_timeout_add_seconds(timeout, request_timeout, session);
	return 0;

failed:
	g_free(req->data);
	g_free(req);
	return err;
}

static int send_request(struct avdtp *session, gboolean priority,
			struct avdtp_stream *stream, uint8_t signal_id,
			void *buffer, size_t size)
{
	struct pending_req *req;

	if (stream && stream->abort_int && signal_id != AVDTP_ABORT) {
		DBG("Unable to send requests while aborting");
		return -EINVAL;
	}

	req = g_new0(struct pending_req, 1);
	req->signal_id = signal_id;
	req->data = g_malloc(size);
	memcpy(req->data, buffer, size);
	req->data_size = size;
	req->stream = stream;

	return send_req(session, priority, req);
}

static gboolean avdtp_discover_resp(struct avdtp *session,
					struct discover_resp *resp, int size)
{
	int sep_count, i;
	uint8_t getcap_cmd;
	int ret = 0;
	gboolean getcap_pending = FALSE;

	if (session->version >= 0x0103)
		getcap_cmd = AVDTP_GET_ALL_CAPABILITIES;
	else
		getcap_cmd = AVDTP_GET_CAPABILITIES;

	sep_count = size / sizeof(struct seid_info);

	for (i = 0; i < sep_count; i++) {
		struct avdtp_remote_sep *sep;
		struct avdtp_stream *stream;
		struct seid_req req;

		DBG("seid %d type %d media %d in use %d",
				resp->seps[i].seid, resp->seps[i].type,
				resp->seps[i].media_type, resp->seps[i].inuse);

		stream = find_stream_by_rseid(session, resp->seps[i].seid);

		sep = find_remote_sep(session->seps, resp->seps[i].seid);
		if (sep && sep->type == resp->seps[i].type &&
				sep->media_type == resp->seps[i].media_type &&
				sep->codec) {
			sep->discovered = true;
			continue;
		}

		if (resp->seps[i].inuse && !stream)
			continue;

		sep = g_new0(struct avdtp_remote_sep, 1);
		session->seps = g_slist_append(session->seps, sep);

		sep->stream = stream;
		sep->seid = resp->seps[i].seid;
		sep->type = resp->seps[i].type;
		sep->media_type = resp->seps[i].media_type;
		sep->discovered = true;

		memset(&req, 0, sizeof(req));
		req.acp_seid = sep->seid;

		ret = send_request(session, TRUE, NULL, getcap_cmd,
							&req, sizeof(req));
		if (ret < 0)
			break;
		getcap_pending = TRUE;
	}

	if (!getcap_pending)
		finalize_discovery(session, -ret);

	return TRUE;
}

static gboolean avdtp_get_capabilities_resp(struct avdtp *session,
						struct getcap_resp *resp,
						unsigned int size)
{
	struct avdtp_remote_sep *sep;
	uint8_t seid;

	/* Check for minimum required packet size includes:
	 *   1. getcap resp header
	 *   2. media transport capability (2 bytes)
	 *   3. media codec capability type + length (2 bytes)
	 *   4. the actual media codec elements
	 * */
	if (size < (sizeof(struct getcap_resp) + 4 +
				sizeof(struct avdtp_media_codec_capability))) {
		error("Too short getcap resp packet");
		return FALSE;
	}

	seid = ((struct seid_req *) session->req->data)->acp_seid;

	sep = find_remote_sep(session->seps, seid);

	DBG("seid %d type %d media %d", sep->seid,
					sep->type, sep->media_type);

	if (sep->caps) {
		g_slist_free_full(sep->caps, g_free);
		sep->caps = NULL;
		sep->codec = NULL;
		sep->delay_reporting = FALSE;
	}

	sep->caps = caps_to_list(resp->caps, size - sizeof(struct getcap_resp),
					&sep->codec, &sep->delay_reporting);

	return TRUE;
}

static gboolean avdtp_set_configuration_resp(struct avdtp *session,
						struct avdtp_stream *stream,
						struct avdtp_single_header *resp,
						int size)
{
	struct avdtp_local_sep *sep = stream->lsep;

	if (sep->cfm && sep->cfm->set_configuration)
		sep->cfm->set_configuration(session, sep, stream, NULL,
						sep->user_data);

	avdtp_sep_set_state(session, sep, AVDTP_STATE_CONFIGURED);

	return TRUE;
}

static gboolean avdtp_reconfigure_resp(struct avdtp *session,
					struct avdtp_stream *stream,
					struct avdtp_single_header *resp, int size)
{
	return TRUE;
}

static gboolean avdtp_open_resp(struct avdtp *session, struct avdtp_stream *stream,
				struct seid_rej *resp, int size)
{
	struct avdtp_local_sep *sep = stream->lsep;
	BtIOMode mode = btd_opts.avdtp.stream_mode;

	stream->io = l2cap_connect(session, mode);
	if (!stream->io) {
		avdtp_sep_set_state(session, sep, AVDTP_STATE_IDLE);
		return FALSE;
	}

	session->pending_open = stream;

	return TRUE;
}

static gboolean avdtp_start_resp(struct avdtp *session,
					struct avdtp_stream *stream,
					struct seid_rej *resp, int size)
{
	struct avdtp_local_sep *sep = stream->lsep;

	if (sep->cfm && sep->cfm->start)
		sep->cfm->start(session, sep, stream, NULL, sep->user_data);

	/* We might be in STREAMING already if both sides send START_CMD at the
	 * same time and the one in SNK role doesn't reject it as it should */
	if (sep->state != AVDTP_STATE_STREAMING)
		avdtp_sep_set_state(session, sep, AVDTP_STATE_STREAMING);

	return TRUE;
}

static gboolean avdtp_close_resp(struct avdtp *session,
					struct avdtp_stream *stream,
					struct seid_rej *resp, int size)
{
	struct avdtp_local_sep *sep = stream->lsep;

	avdtp_sep_set_state(session, sep, AVDTP_STATE_CLOSING);

	close_stream(stream);

	return TRUE;
}

static gboolean avdtp_suspend_resp(struct avdtp *session,
					struct avdtp_stream *stream,
					void *data, int size)
{
	struct avdtp_local_sep *sep = stream->lsep;

	avdtp_sep_set_state(session, sep, AVDTP_STATE_OPEN);

	if (sep->cfm && sep->cfm->suspend)
		sep->cfm->suspend(session, sep, stream, NULL, sep->user_data);

	return TRUE;
}

static gboolean avdtp_abort_resp(struct avdtp *session,
					struct avdtp_stream *stream,
					struct seid_rej *resp, int size)
{
	struct avdtp_local_sep *sep = stream->lsep;

	avdtp_sep_set_state(session, sep, AVDTP_STATE_ABORTING);

	if (sep->cfm && sep->cfm->abort)
		sep->cfm->abort(session, sep, stream, NULL, sep->user_data);

	avdtp_sep_set_state(session, sep, AVDTP_STATE_IDLE);

	return TRUE;
}

static gboolean avdtp_delay_report_resp(struct avdtp *session,
					struct avdtp_stream *stream,
					void *data, int size)
{
	struct avdtp_local_sep *sep = stream->lsep;

	if (sep->cfm && sep->cfm->delay_report)
		sep->cfm->delay_report(session, sep, stream, NULL, sep->user_data);

	return TRUE;
}

static gboolean avdtp_parse_resp(struct avdtp *session,
					struct avdtp_stream *stream,
					uint8_t transaction, uint8_t signal_id,
					void *buf, int size)
{
	struct pending_req *next;
	const char *get_all = "";

	if (session->prio_queue)
		next = session->prio_queue->data;
	else if (session->req_queue)
		next = session->req_queue->data;
	else
		next = NULL;

	switch (signal_id) {
	case AVDTP_DISCOVER:
		DBG("DISCOVER request succeeded");
		return avdtp_discover_resp(session, buf, size);
	case AVDTP_GET_ALL_CAPABILITIES:
		get_all = "ALL_";
		/* fall through */
	case AVDTP_GET_CAPABILITIES:
		DBG("GET_%sCAPABILITIES request succeeded", get_all);
		if (!avdtp_get_capabilities_resp(session, buf, size))
			return FALSE;
		if (!(next && (next->signal_id == AVDTP_GET_CAPABILITIES ||
				next->signal_id == AVDTP_GET_ALL_CAPABILITIES)))
			finalize_discovery(session, 0);
		return TRUE;
	}

	/* The remaining commands require an existing stream so bail out
	 * here if the stream got unexpectedly disconnected */
	if (!stream) {
		DBG("AVDTP: stream was closed while waiting for reply");
		return TRUE;
	}

	switch (signal_id) {
	case AVDTP_SET_CONFIGURATION:
		DBG("SET_CONFIGURATION request succeeded");
		return avdtp_set_configuration_resp(session, stream,
								buf, size);
	case AVDTP_RECONFIGURE:
		DBG("RECONFIGURE request succeeded");
		return avdtp_reconfigure_resp(session, stream, buf, size);
	case AVDTP_OPEN:
		DBG("OPEN request succeeded");
		return avdtp_open_resp(session, stream, buf, size);
	case AVDTP_SUSPEND:
		DBG("SUSPEND request succeeded");
		return avdtp_suspend_resp(session, stream, buf, size);
	case AVDTP_START:
		DBG("START request succeeded");
		return avdtp_start_resp(session, stream, buf, size);
	case AVDTP_CLOSE:
		DBG("CLOSE request succeeded");
		return avdtp_close_resp(session, stream, buf, size);
	case AVDTP_ABORT:
		DBG("ABORT request succeeded");
		return avdtp_abort_resp(session, stream, buf, size);
	case AVDTP_DELAY_REPORT:
		DBG("DELAY_REPORT request succeeded");
		return avdtp_delay_report_resp(session, stream, buf, size);
	}

	error("Unknown signal id in accept response: %u", signal_id);
	return TRUE;
}

static gboolean seid_rej_to_err(struct seid_rej *rej, unsigned int size,
					struct avdtp_error *err)
{
	if (size < sizeof(struct seid_rej)) {
		error("Too small packet for seid_rej");
		return FALSE;
	}

	avdtp_error_init(err, 0x00, rej->error);

	return TRUE;
}

static gboolean conf_rej_to_err(struct conf_rej *rej, unsigned int size,
				struct avdtp_error *err)
{
	if (size < sizeof(struct conf_rej)) {
		error("Too small packet for conf_rej");
		return FALSE;
	}

	avdtp_error_init(err, rej->category, rej->error);

	return TRUE;
}

static gboolean stream_rej_to_err(struct stream_rej *rej, unsigned int size,
					struct avdtp_error *err,
					uint8_t *acp_seid)
{
	if (size < sizeof(struct stream_rej)) {
		error("Too small packet for stream_rej");
		return FALSE;
	}

	avdtp_error_init(err, 0x00, rej->error);

	if (acp_seid)
		*acp_seid = rej->acp_seid;

	return TRUE;
}

static gboolean avdtp_parse_rej(struct avdtp *session,
					struct avdtp_stream *stream,
					uint8_t transaction, uint8_t signal_id,
					void *buf, int size)
{
	struct avdtp_error err;
	uint8_t acp_seid;
	struct avdtp_local_sep *sep = stream ? stream->lsep : NULL;

	switch (signal_id) {
	case AVDTP_DISCOVER:
		if (!seid_rej_to_err(buf, size, &err))
			return FALSE;
		error("DISCOVER request rejected: %s (%d)",
				avdtp_strerror(&err), err.err.error_code);
		return TRUE;
	case AVDTP_GET_CAPABILITIES:
	case AVDTP_GET_ALL_CAPABILITIES:
		if (!seid_rej_to_err(buf, size, &err))
			return FALSE;
		error("GET_CAPABILITIES request rejected: %s (%d)",
				avdtp_strerror(&err), err.err.error_code);
		return TRUE;
	case AVDTP_OPEN:
		if (!seid_rej_to_err(buf, size, &err))
			return FALSE;
		error("OPEN request rejected: %s (%d)",
				avdtp_strerror(&err), err.err.error_code);
		if (sep && sep->cfm && sep->cfm->open)
			sep->cfm->open(session, sep, stream, &err,
					sep->user_data);
		return TRUE;
	case AVDTP_SET_CONFIGURATION:
		if (!conf_rej_to_err(buf, size, &err))
			return FALSE;
		error("SET_CONFIGURATION request rejected: %s (%d)",
				avdtp_strerror(&err), err.err.error_code);
		if (sep && sep->cfm && sep->cfm->set_configuration)
			sep->cfm->set_configuration(session, sep, stream,
							&err, sep->user_data);
		return TRUE;
	case AVDTP_RECONFIGURE:
		if (!conf_rej_to_err(buf, size, &err))
			return FALSE;
		error("RECONFIGURE request rejected: %s (%d)",
				avdtp_strerror(&err), err.err.error_code);
		if (sep && sep->cfm && sep->cfm->reconfigure)
			sep->cfm->reconfigure(session, sep, stream, &err,
						sep->user_data);
		return TRUE;
	case AVDTP_START:
		if (!stream_rej_to_err(buf, size, &err, &acp_seid))
			return FALSE;
		error("START request rejected: %s (%d)",
				avdtp_strerror(&err), err.err.error_code);
		if (sep && sep->cfm && sep->cfm->start) {
			sep->cfm->start(session, sep, stream, &err,
					sep->user_data);
			stream->starting = FALSE;
		}
		return TRUE;
	case AVDTP_SUSPEND:
		if (!stream_rej_to_err(buf, size, &err, &acp_seid))
			return FALSE;
		error("SUSPEND request rejected: %s (%d)",
				avdtp_strerror(&err), err.err.error_code);
		if (sep && sep->cfm && sep->cfm->suspend)
			sep->cfm->suspend(session, sep, stream, &err,
						sep->user_data);
		return TRUE;
	case AVDTP_CLOSE:
		if (!stream_rej_to_err(buf, size, &err, &acp_seid))
			return FALSE;
		error("CLOSE request rejected: %s (%d)",
				avdtp_strerror(&err), err.err.error_code);
		if (sep && sep->cfm && sep->cfm->close) {
			sep->cfm->close(session, sep, stream, &err,
					sep->user_data);
			stream->close_int = FALSE;
		}
		return TRUE;
	case AVDTP_ABORT:
		if (!stream_rej_to_err(buf, size, &err, &acp_seid))
			return FALSE;
		error("ABORT request rejected: %s (%d)",
				avdtp_strerror(&err), err.err.error_code);
		if (sep && sep->cfm && sep->cfm->abort)
			sep->cfm->abort(session, sep, stream, &err,
					sep->user_data);
		return FALSE;
	case AVDTP_DELAY_REPORT:
		if (!stream_rej_to_err(buf, size, &err, &acp_seid))
			return FALSE;
		error("DELAY_REPORT request rejected: %s (%d)",
				avdtp_strerror(&err), err.err.error_code);
		if (sep && sep->cfm && sep->cfm->delay_report)
			sep->cfm->delay_report(session, sep, stream, &err,
							sep->user_data);
		return TRUE;
	default:
		error("Unknown reject response signal id: %u", signal_id);
		return TRUE;
	}
}

struct avdtp_service_capability *avdtp_stream_get_codec(
						struct avdtp_stream *stream)
{
	GSList *l;

	for (l = stream->caps; l; l = l->next) {
		struct avdtp_service_capability *cap = l->data;

		if (cap->category == AVDTP_MEDIA_CODEC)
			return cap;
	}

	return NULL;
}

static gboolean avdtp_stream_has_capability(struct avdtp_stream *stream,
					struct avdtp_service_capability *cap)
{
	GSList *l;
	struct avdtp_service_capability *stream_cap;

	for (l = stream->caps; l; l = g_slist_next(l)) {
		stream_cap = l->data;

		if (stream_cap->category != cap->category ||
			stream_cap->length != cap->length)
			continue;

		if (memcmp(stream_cap->data, cap->data, cap->length) == 0)
			return TRUE;
	}

	return FALSE;
}

gboolean avdtp_stream_has_capabilities(struct avdtp_stream *stream,
					GSList *caps)
{
	for (; caps; caps = g_slist_next(caps)) {
		struct avdtp_service_capability *cap = caps->data;

		if (!avdtp_stream_has_capability(stream, cap))
			return FALSE;
	}

	return TRUE;
}

struct avdtp_remote_sep *avdtp_stream_get_remote_sep(
						struct avdtp_stream *stream)
{
	GSList *l;

	for (l = stream->session->seps; l; l = l->next) {
		struct avdtp_remote_sep *sep = l->data;

		if (sep->seid == stream->rseid)
			return sep;
	}

	return NULL;
}

gboolean avdtp_stream_set_transport(struct avdtp_stream *stream, int fd,
						size_t imtu, size_t omtu)
{
	GIOChannel *io = g_io_channel_unix_new(fd);

	if (stream != stream->session->pending_open) {
		uint8_t err;

		if (stream->session->pending_open)
			goto failed;

		/* Attempt to Open there is no pending stream set yet */
		if (stream->lsep->ind && stream->lsep->ind->open) {
			if (!stream->lsep->ind->open(stream->session,
						stream->lsep,
						stream, &err,
						stream->lsep->user_data))
				goto failed;
		}

		stream_set_pending_open(stream, io);
		stream->imtu = imtu;
		stream->omtu = omtu;

		return TRUE;
	}

	handle_transport_connect(stream->session, io, imtu, omtu);

	g_io_channel_unref(io);

	return TRUE;

failed:
	g_io_channel_unref(io);
	return FALSE;
}

gboolean avdtp_stream_get_transport(struct avdtp_stream *stream, int *sock,
					uint16_t *imtu, uint16_t *omtu,
					GSList **caps)
{
	if (stream->io == NULL)
		return FALSE;

	if (sock)
		*sock = g_io_channel_unix_get_fd(stream->io);

	if (omtu)
		*omtu = stream->omtu;

	if (imtu)
		*imtu = stream->imtu;

	if (caps)
		*caps = stream->caps;

	return TRUE;
}

static int process_queue(struct avdtp *session)
{
	GSList **queue, *l;
	struct pending_req *req;

	if (session->req)
		return 0;

	if (session->prio_queue)
		queue = &session->prio_queue;
	else
		queue = &session->req_queue;

	if (!*queue)
		return 0;

	l = *queue;
	req = l->data;

	*queue = g_slist_remove(*queue, req);

	return send_req(session, FALSE, req);
}

uint8_t avdtp_get_seid(struct avdtp_remote_sep *sep)
{
	return sep->seid;
}

uint8_t avdtp_get_type(struct avdtp_remote_sep *sep)
{
	return sep->type;
}

struct avdtp_service_capability *avdtp_get_codec(struct avdtp_remote_sep *sep)
{
	return sep->codec;
}

bool avdtp_get_delay_reporting(struct avdtp_remote_sep *sep)
{
	return sep->delay_reporting;
}

struct avdtp_service_capability *avdtp_service_cap_new(uint8_t category,
							void *data, int length)
{
	struct avdtp_service_capability *cap;

	if (category < AVDTP_MEDIA_TRANSPORT || category > AVDTP_DELAY_REPORTING)
		return NULL;

	cap = g_malloc(sizeof(struct avdtp_service_capability) + length);
	cap->category = category;
	cap->length = length;
	memcpy(cap->data, data, length);

	return cap;
}

struct avdtp_remote_sep *avdtp_register_remote_sep(struct avdtp *session,
							uint8_t seid,
							uint8_t type,
							GSList *caps,
							bool delay_reporting)
{
	struct avdtp_remote_sep *sep;
	GSList *l;

	sep = find_remote_sep(session->seps, seid);
	if (sep)
		return sep;

	sep = g_new0(struct avdtp_remote_sep, 1);
	session->seps = g_slist_append(session->seps, sep);
	sep->seid = seid;
	sep->type = type;
	sep->media_type = AVDTP_MEDIA_TYPE_AUDIO;
	sep->caps = caps;
	sep->delay_reporting = delay_reporting;

	for (l = caps; l; l = g_slist_next(l)) {
		struct avdtp_service_capability *cap = l->data;

		if (cap->category == AVDTP_MEDIA_CODEC)
			sep->codec = cap;
	}

	DBG("seid %d type %d media %d delay_reporting %s", sep->seid, sep->type,
				sep->media_type,
				sep->delay_reporting ? "true" : "false");

	return sep;
}

void avdtp_remote_sep_set_destroy(struct avdtp_remote_sep *sep, void *user_data,
					avdtp_remote_sep_destroy_t destroy)
{
	if (!sep)
		return;

	sep->user_data = user_data;
	sep->destroy = destroy;
}

static gboolean process_discover(gpointer data)
{
	struct avdtp *session = data;

	session->discover->id = 0;

	finalize_discovery(session, 0);

	return FALSE;
}

int avdtp_discover(struct avdtp *session, avdtp_discover_cb_t cb,
			void *user_data)
{
	int err;

	if (session->discover)
		return -EBUSY;

	session->discover = g_new0(struct discover_callback, 1);

	if (session->seps) {
		session->discover->cb = cb;
		session->discover->user_data = user_data;
		session->discover->id = g_idle_add(process_discover, session);
		return 0;
	}

	err = send_request(session, FALSE, NULL, AVDTP_DISCOVER, NULL, 0);
	if (err == 0) {
		session->discover->cb = cb;
		session->discover->user_data = user_data;
	}

	return err;
}

gboolean avdtp_stream_remove_cb(struct avdtp *session,
				struct avdtp_stream *stream,
				unsigned int id)
{
	GSList *l;
	struct stream_callback *cb;

	if (!stream)
		return FALSE;

	for (cb = NULL, l = stream->callbacks; l != NULL; l = l->next) {
		struct stream_callback *tmp = l->data;
		if (tmp && tmp->id == id) {
			cb = tmp;
			break;
		}
	}

	if (!cb)
		return FALSE;

	stream->callbacks = g_slist_remove(stream->callbacks, cb);
	g_free(cb);

	return TRUE;
}

unsigned int avdtp_stream_add_cb(struct avdtp *session,
					struct avdtp_stream *stream,
					avdtp_stream_state_cb cb, void *data)
{
	struct stream_callback *stream_cb;
	static unsigned int id = 0;

	stream_cb = g_new(struct stream_callback, 1);
	stream_cb->cb = cb;
	stream_cb->user_data = data;
	stream_cb->id = ++id;

	stream->callbacks = g_slist_append(stream->callbacks, stream_cb);

	return stream_cb->id;
}

int avdtp_get_configuration(struct avdtp *session, struct avdtp_stream *stream)
{
	struct seid_req req;

	if (session->state < AVDTP_SESSION_STATE_CONNECTED)
		return -EINVAL;

	memset(&req, 0, sizeof(req));
	req.acp_seid = stream->rseid;

	return send_request(session, FALSE, stream, AVDTP_GET_CONFIGURATION,
							&req, sizeof(req));
}

static void copy_capabilities(gpointer data, gpointer user_data)
{
	struct avdtp_service_capability *src_cap = data;
	struct avdtp_service_capability *dst_cap;
	GSList **l = user_data;

	dst_cap = avdtp_service_cap_new(src_cap->category, src_cap->data,
					src_cap->length);

	*l = g_slist_append(*l, dst_cap);
}

int avdtp_set_configuration(struct avdtp *session,
				struct avdtp_remote_sep *rsep,
				struct avdtp_local_sep *lsep,
				GSList *caps,
				struct avdtp_stream **stream)
{
	struct setconf_req *req;
	struct avdtp_stream *new_stream;
	unsigned char *ptr;
	int err, caps_len;
	struct avdtp_service_capability *cap;
	GSList *l;

	if (session->state != AVDTP_SESSION_STATE_CONNECTED)
		return -ENOTCONN;

	if (!(lsep && rsep))
		return -EINVAL;

	DBG("%p: int_seid=%u, acp_seid=%u", session,
			lsep->info.seid, rsep->seid);

	new_stream = g_new0(struct avdtp_stream, 1);
	new_stream->session = session;
	new_stream->lsep = lsep;
	new_stream->rseid = rsep->seid;

	if (rsep->delay_reporting && lsep->delay_reporting) {
		struct avdtp_service_capability *delay_reporting;

		delay_reporting = avdtp_service_cap_new(AVDTP_DELAY_REPORTING,
								NULL, 0);
		caps = g_slist_append(caps, delay_reporting);
		new_stream->delay_reporting = TRUE;
	}

	g_slist_foreach(caps, copy_capabilities, &new_stream->caps);

	/* Calculate total size of request */
	for (l = caps, caps_len = 0; l != NULL; l = g_slist_next(l)) {
		cap = l->data;
		caps_len += cap->length + 2;
	}

	req = g_malloc0(sizeof(struct setconf_req) + caps_len);

	req->int_seid = lsep->info.seid;
	req->acp_seid = rsep->seid;

	/* Copy the capabilities into the request */
	for (l = caps, ptr = req->caps; l != NULL; l = g_slist_next(l)) {
		cap = l->data;
		memcpy(ptr, cap, cap->length + 2);
		ptr += cap->length + 2;
	}

	err = send_request(session, FALSE, new_stream,
				AVDTP_SET_CONFIGURATION, req,
				sizeof(struct setconf_req) + caps_len);
	if (err < 0)
		stream_free(new_stream);
	else {
		lsep->info.inuse = 1;
		lsep->stream = new_stream;
		rsep->stream = new_stream;
		session->streams = g_slist_append(session->streams, new_stream);
		if (stream)
			*stream = new_stream;
		session->dc_timeout = DISCONNECT_TIMEOUT;
	}

	g_free(req);

	return err;
}

int avdtp_open(struct avdtp *session, struct avdtp_stream *stream)
{
	struct seid_req req;

	if (!g_slist_find(session->streams, stream))
		return -EINVAL;

	if (stream->lsep->state > AVDTP_STATE_CONFIGURED)
		return -EINVAL;

	memset(&req, 0, sizeof(req));
	req.acp_seid = stream->rseid;

	return send_request(session, FALSE, stream, AVDTP_OPEN,
							&req, sizeof(req));
}

static gboolean start_timeout(gpointer user_data)
{
	struct avdtp_stream *stream = user_data;
	struct avdtp *session = stream->session;

	stream->open_acp = FALSE;

	if (avdtp_start(session, stream) < 0)
		error("wait_timeout: avdtp_start failed");

	stream->start_timer = 0;

	return FALSE;
}

int avdtp_start(struct avdtp *session, struct avdtp_stream *stream)
{
	struct start_req req;
	int ret;

	if (!g_slist_find(session->streams, stream))
		return -EINVAL;

	if (stream->lsep->state != AVDTP_STATE_OPEN)
		return -EINVAL;

	/* Recommendation 12:
	 *  If the RD has configured and opened a stream it is also responsible
	 *  to start the streaming via GAVDP_START.
	 */
	if (stream->open_acp) {
		/* If timer already active wait it */
		if (stream->start_timer)
			return 0;

		stream->start_timer = g_timeout_add_seconds(START_TIMEOUT,
								start_timeout,
								stream);
		return 0;
	}

	if (stream->close_int == TRUE) {
		error("avdtp_start: rejecting start since close is initiated");
		return -EINVAL;
	}

	if (stream->starting == TRUE) {
		DBG("stream already started");
		return -EINPROGRESS;
	}

	memset(&req, 0, sizeof(req));
	req.first_seid.seid = stream->rseid;

	ret = send_request(session, FALSE, stream, AVDTP_START,
							&req, sizeof(req));
	if (ret == 0)
		stream->starting = TRUE;

	return ret;
}

int avdtp_close(struct avdtp *session, struct avdtp_stream *stream,
		gboolean immediate)
{
	struct seid_req req;
	int ret;

	if (!g_slist_find(session->streams, stream))
		return -EINVAL;

	if (stream->lsep->state < AVDTP_STATE_OPEN)
		return -EINVAL;

	if (stream->close_int == TRUE) {
		error("avdtp_close: rejecting since close is already initiated");
		return -EINVAL;
	}

	if (immediate && session->req && stream == session->req->stream)
		return avdtp_abort(session, stream);

	memset(&req, 0, sizeof(req));
	req.acp_seid = stream->rseid;

	ret = send_request(session, FALSE, stream, AVDTP_CLOSE,
							&req, sizeof(req));
	if (ret == 0) {
		stream->close_int = TRUE;
		session->dc_timeout = 0;
	}

	return ret;
}

int avdtp_suspend(struct avdtp *session, struct avdtp_stream *stream)
{
	struct seid_req req;

	if (!g_slist_find(session->streams, stream))
		return -EINVAL;

	if (stream->lsep->state <= AVDTP_STATE_OPEN || stream->close_int)
		return -EINVAL;

	memset(&req, 0, sizeof(req));
	req.acp_seid = stream->rseid;

	return send_request(session, FALSE, stream, AVDTP_SUSPEND,
							&req, sizeof(req));
}

int avdtp_abort(struct avdtp *session, struct avdtp_stream *stream)
{
	struct seid_req req;
	int ret;

	if (!stream && session->discover) {
		/* Don't call cb since it being aborted */
		session->discover->cb = NULL;
		finalize_discovery(session, ECANCELED);
		return -EALREADY;
	}

	if (!g_slist_find(session->streams, stream))
		return -EINVAL;

	if (stream->lsep->state == AVDTP_STATE_ABORTING)
		return -EINVAL;

	avdtp_sep_set_state(session, stream->lsep, AVDTP_STATE_ABORTING);

	if (session->req && stream == session->req->stream)
		return cancel_request(session, ECANCELED);

	memset(&req, 0, sizeof(req));
	req.acp_seid = stream->rseid;

	ret = send_request(session, TRUE, stream, AVDTP_ABORT,
							&req, sizeof(req));
	if (ret == 0) {
		stream->abort_int = TRUE;
		session->dc_timeout = 0;
	}

	return ret;
}

int avdtp_delay_report(struct avdtp *session, struct avdtp_stream *stream,
							uint16_t delay)
{
	struct delay_req req;

	if (!g_slist_find(session->streams, stream))
		return -EINVAL;

	if (stream->lsep->state != AVDTP_STATE_CONFIGURED &&
				stream->lsep->state != AVDTP_STATE_STREAMING)
		return -EINVAL;

	if (!stream->delay_reporting || session->version < 0x0103)
		return -EINVAL;

	stream->delay = delay;

	memset(&req, 0, sizeof(req));
	req.acp_seid = stream->rseid;
	req.delay = htons(delay);

	return send_request(session, TRUE, stream, AVDTP_DELAY_REPORT,
							&req, sizeof(req));
}

struct avdtp_local_sep *avdtp_register_sep(struct queue *lseps, uint8_t type,
						uint8_t media_type,
						uint8_t codec_type,
						gboolean delay_reporting,
						struct avdtp_sep_ind *ind,
						struct avdtp_sep_cfm *cfm,
						void *user_data)
{
	struct avdtp_local_sep *sep;
	uint8_t seid = util_get_uid(&seids, MAX_SEID);

	if (!seid)
		return NULL;

	sep = g_new0(struct avdtp_local_sep, 1);

	sep->state = AVDTP_STATE_IDLE;
	sep->info.seid = seid;
	sep->info.type = type;
	sep->info.media_type = media_type;
	sep->codec = codec_type;
	sep->ind = ind;
	sep->cfm = cfm;
	sep->user_data = user_data;
	sep->delay_reporting = delay_reporting;

	DBG("SEP %p registered: type:%d codec:%d seid:%d", sep,
			sep->info.type, sep->codec, sep->info.seid);

	if (!queue_push_tail(lseps, sep)) {
		g_free(sep);
		return NULL;
	}

	return sep;
}

int avdtp_unregister_sep(struct queue *lseps, struct avdtp_local_sep *sep)
{
	if (!sep)
		return -EINVAL;

	if (sep->stream)
		release_stream(sep->stream, sep->stream->session);

	DBG("SEP %p unregistered: type:%d codec:%d seid:%d", sep,
			sep->info.type, sep->codec, sep->info.seid);

	util_clear_uid(&seids, sep->info.seid);
	queue_remove(lseps, sep);
	g_free(sep);

	return 0;
}

const char *avdtp_strerror(struct avdtp_error *err)
{
	if (err->category == AVDTP_ERRNO)
		return strerror(err->err.posix_errno);

	switch(err->err.error_code) {
	case AVDTP_BAD_HEADER_FORMAT:
		return "Bad Header Format";
	case AVDTP_BAD_LENGTH:
		return "Bad Packet Length";
	case AVDTP_BAD_ACP_SEID:
		return "Bad Acceptor SEID";
	case AVDTP_SEP_IN_USE:
		return "Stream End Point in Use";
	case AVDTP_SEP_NOT_IN_USE:
		return "Stream End Point Not in Use";
	case AVDTP_BAD_SERV_CATEGORY:
		return "Bad Service Category";
	case AVDTP_BAD_PAYLOAD_FORMAT:
		return "Bad Payload format";
	case AVDTP_NOT_SUPPORTED_COMMAND:
		return "Command Not Supported";
	case AVDTP_INVALID_CAPABILITIES:
		return "Invalid Capabilities";
	case AVDTP_BAD_RECOVERY_TYPE:
		return "Bad Recovery Type";
	case AVDTP_BAD_MEDIA_TRANSPORT_FORMAT:
		return "Bad Media Transport Format";
	case AVDTP_BAD_RECOVERY_FORMAT:
		return "Bad Recovery Format";
	case AVDTP_BAD_ROHC_FORMAT:
		return "Bad Header Compression Format";
	case AVDTP_BAD_CP_FORMAT:
		return "Bad Content Protection Format";
	case AVDTP_BAD_MULTIPLEXING_FORMAT:
		return "Bad Multiplexing Format";
	case AVDTP_UNSUPPORTED_CONFIGURATION:
		return "Configuration not supported";
	case AVDTP_BAD_STATE:
		return "Bad State";
	default:
		return "Unknown error";
	}
}

avdtp_state_t avdtp_sep_get_state(struct avdtp_local_sep *sep)
{
	return sep->state;
}

uint8_t avdtp_sep_get_seid(struct avdtp_local_sep *sep)
{
	return sep->info.seid;
}

struct btd_adapter *avdtp_get_adapter(struct avdtp *session)
{
	if (!session)
		return NULL;

	return device_get_adapter(session->device);
}

struct btd_device *avdtp_get_device(struct avdtp *session)
{
	if (!session)
		return NULL;

	return session->device;
}

gboolean avdtp_has_stream(struct avdtp *session, struct avdtp_stream *stream)
{
	return g_slist_find(session->streams, stream) ? TRUE : FALSE;
}

unsigned int avdtp_add_state_cb(struct btd_device *dev,
				avdtp_session_state_cb cb, void *user_data)
{
	struct avdtp_state_callback *state_cb;
	static unsigned int id = 0;

	state_cb = g_new(struct avdtp_state_callback, 1);
	state_cb->cb = cb;
	state_cb->dev = dev;
	state_cb->id = ++id;
	state_cb->user_data = user_data;

	state_callbacks = g_slist_append(state_callbacks, state_cb);

	return state_cb->id;
}

gboolean avdtp_remove_state_cb(unsigned int id)
{
	GSList *l;

	for (l = state_callbacks; l != NULL; l = l->next) {
		struct avdtp_state_callback *cb = l->data;
		if (cb && cb->id == id) {
			state_callbacks = g_slist_remove(state_callbacks, cb);
			g_free(cb);
			return TRUE;
		}
	}

	return FALSE;
}

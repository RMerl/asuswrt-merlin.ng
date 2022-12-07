// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2011  Texas Instruments, Inc.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>

#include <glib.h>

#include "lib/sdp.h"

#include "src/log.h"
#include "src/uinput.h"

#include "avctp.h"

/*
 * AV/C Panel 1.23, page 76:
 * command with the pressed value is valid for two seconds
 */
#define AVC_PRESS_TIMEOUT	2

#define QUIRK_NO_RELEASE 1 << 0

/* Message types */
#define AVCTP_COMMAND		0
#define AVCTP_RESPONSE		1

/* Packet types */
#define AVCTP_PACKET_SINGLE	0
#define AVCTP_PACKET_START	1
#define AVCTP_PACKET_CONTINUE	2
#define AVCTP_PACKET_END	3

#if __BYTE_ORDER == __LITTLE_ENDIAN

struct avctp_header {
	uint8_t ipid:1;
	uint8_t cr:1;
	uint8_t packet_type:2;
	uint8_t transaction:4;
	uint16_t pid;
} __attribute__ ((packed));

struct avc_header {
	uint8_t code:4;
	uint8_t _hdr0:4;
	uint8_t subunit_id:3;
	uint8_t subunit_type:5;
	uint8_t opcode;
} __attribute__ ((packed));

#elif __BYTE_ORDER == __BIG_ENDIAN

struct avctp_header {
	uint8_t transaction:4;
	uint8_t packet_type:2;
	uint8_t cr:1;
	uint8_t ipid:1;
	uint16_t pid;
} __attribute__ ((packed));

struct avc_header {
	uint8_t _hdr0:4;
	uint8_t code:4;
	uint8_t subunit_type:5;
	uint8_t subunit_id:3;
	uint8_t opcode;
} __attribute__ ((packed));

#else
#error "Unknown byte order"
#endif

struct avctp_control_req {
	struct avctp_pending_req *p;
	uint8_t code;
	uint8_t subunit;
	uint8_t op;
	struct iovec *iov;
	int iov_cnt;
	avctp_rsp_cb func;
	void *user_data;
};

struct avctp_browsing_req {
	struct avctp_pending_req *p;
	struct iovec *iov;
	int iov_cnt;
	avctp_browsing_rsp_cb func;
	void *user_data;
};

typedef int (*avctp_process_cb) (void *data);

struct avctp_pending_req {
	struct avctp_channel *chan;
	uint8_t transaction;
	guint timeout;
	int err;
	avctp_process_cb process;
	void *data;
	avctp_destroy_cb_t destroy;
};

struct avctp_channel {
	struct avctp *session;
	GIOChannel *io;
	uint8_t transaction;
	guint watch;
	uint16_t imtu;
	uint16_t omtu;
	uint8_t *buffer;
	GSList *handlers;
	struct avctp_pending_req *p;
	GQueue *queue;
	GSList *processed;
	guint process_id;
	avctp_destroy_cb_t destroy;
};

struct key_pressed {
	uint8_t op;
	uint8_t *params;
	size_t params_len;
	guint timer;
};

struct avctp {
	unsigned int ref;
	int uinput;

	unsigned int passthrough_id;
	unsigned int unit_id;
	unsigned int subunit_id;

	struct avctp_channel *control;
	struct avctp_channel *browsing;

	struct avctp_passthrough_handler *handler;

	uint8_t key_quirks[256];
	struct key_pressed key;
	uint16_t version;

	avctp_destroy_cb_t destroy;
	void *data;
};

struct avctp_passthrough_handler {
	avctp_passthrough_cb cb;
	void *user_data;
	unsigned int id;
};

struct avctp_pdu_handler {
	uint8_t opcode;
	avctp_control_pdu_cb cb;
	void *user_data;
	unsigned int id;
};

struct avctp_browsing_pdu_handler {
	avctp_browsing_pdu_cb cb;
	void *user_data;
	unsigned int id;
	avctp_destroy_cb_t destroy;
};

static struct {
	const char *name;
	uint8_t avc;
	uint16_t uinput;
} key_map[] = {
	{ "SELECT",		AVC_SELECT,		KEY_SELECT },
	{ "UP",			AVC_UP,			KEY_UP },
	{ "DOWN",		AVC_DOWN,		KEY_DOWN },
	{ "LEFT",		AVC_LEFT,		KEY_LEFT },
	{ "RIGHT",		AVC_RIGHT,		KEY_RIGHT },
	{ "ROOT MENU",		AVC_ROOT_MENU,		KEY_MENU },
	{ "CONTENTS MENU",	AVC_CONTENTS_MENU,	KEY_PROGRAM },
	{ "FAVORITE MENU",	AVC_FAVORITE_MENU,	KEY_FAVORITES },
	{ "EXIT",		AVC_EXIT,		KEY_EXIT },
	{ "ON DEMAND MENU",	AVC_ON_DEMAND_MENU,	KEY_MENU },
	{ "APPS MENU",		AVC_APPS_MENU,		KEY_MENU },
	{ "0",			AVC_0,			KEY_0 },
	{ "1",			AVC_1,			KEY_1 },
	{ "2",			AVC_2,			KEY_2 },
	{ "3",			AVC_3,			KEY_3 },
	{ "4",			AVC_4,			KEY_4 },
	{ "5",			AVC_5,			KEY_5 },
	{ "6",			AVC_6,			KEY_6 },
	{ "7",			AVC_7,			KEY_7 },
	{ "8",			AVC_8,			KEY_8 },
	{ "9",			AVC_9,			KEY_9 },
	{ "DOT",		AVC_DOT,		KEY_DOT },
	{ "ENTER",		AVC_ENTER,		KEY_ENTER },
	{ "CHANNEL UP",		AVC_CHANNEL_UP,		KEY_CHANNELUP },
	{ "CHANNEL DOWN",	AVC_CHANNEL_DOWN,	KEY_CHANNELDOWN },
	{ "CHANNEL PREVIOUS",	AVC_CHANNEL_PREVIOUS,	KEY_LAST },
	{ "INPUT SELECT",	AVC_INPUT_SELECT,	KEY_CONFIG },
	{ "INFO",		AVC_INFO,		KEY_INFO },
	{ "HELP",		AVC_HELP,		KEY_HELP },
	{ "POWER",		AVC_POWER,		KEY_POWER2 },
	{ "VOLUME UP",		AVC_VOLUME_UP,		KEY_VOLUMEUP },
	{ "VOLUME DOWN",	AVC_VOLUME_DOWN,	KEY_VOLUMEDOWN },
	{ "MUTE",		AVC_MUTE,		KEY_MUTE },
	{ "PLAY",		AVC_PLAY,		KEY_PLAYCD },
	{ "STOP",		AVC_STOP,		KEY_STOPCD },
	{ "PAUSE",		AVC_PAUSE,		KEY_PAUSECD },
	{ "FORWARD",		AVC_FORWARD,		KEY_NEXTSONG },
	{ "BACKWARD",		AVC_BACKWARD,		KEY_PREVIOUSSONG },
	{ "RECORD",		AVC_RECORD,		KEY_RECORD },
	{ "REWIND",		AVC_REWIND,		KEY_REWIND },
	{ "FAST FORWARD",	AVC_FAST_FORWARD,	KEY_FASTFORWARD },
	{ "LIST",		AVC_LIST,		KEY_LIST },
	{ "F1",			AVC_F1,			KEY_F1 },
	{ "F2",			AVC_F2,			KEY_F2 },
	{ "F3",			AVC_F3,			KEY_F3 },
	{ "F4",			AVC_F4,			KEY_F4 },
	{ "F5",			AVC_F5,			KEY_F5 },
	{ "F6",			AVC_F6,			KEY_F6 },
	{ "F7",			AVC_F7,			KEY_F7 },
	{ "F8",			AVC_F8,			KEY_F8 },
	{ "F9",			AVC_F9,			KEY_F9 },
	{ "RED",		AVC_RED,		KEY_RED },
	{ "GREEN",		AVC_GREEN,		KEY_GREEN },
	{ "BLUE",		AVC_BLUE,		KEY_BLUE },
	{ "YELLOW",		AVC_YELLOW,		KEY_YELLOW },
	{ NULL }
};

static gboolean process_queue(gpointer user_data);
static gboolean avctp_passthrough_rsp(struct avctp *session, uint8_t code,
					uint8_t subunit, uint8_t *operands,
					size_t operand_count, void *user_data);

static int send_event(int fd, uint16_t type, uint16_t code, int32_t value)
{
	struct uinput_event event;
	int err;

	memset(&event, 0, sizeof(event));
	event.type	= type;
	event.code	= code;
	event.value	= value;

	do {
		err = write(fd, &event, sizeof(event));
	} while (err < 0 && errno == EINTR);

	if (err < 0) {
		err = -errno;
		error("send_event: %s (%d)", strerror(-err), -err);
	}

	return err;
}

static void send_key(int fd, uint16_t key, int pressed)
{
	send_event(fd, EV_KEY, key, pressed);
	send_event(fd, EV_SYN, SYN_REPORT, 0);
}

static gboolean auto_release(gpointer user_data)
{
	struct avctp *session = user_data;

	session->key.timer = 0;

	DBG("AV/C: key press timeout");

	send_key(session->uinput, session->key.op, 0);

	return FALSE;
}

static ssize_t handle_panel_passthrough(struct avctp *session,
					uint8_t transaction, uint8_t *code,
					uint8_t *subunit, uint8_t *operands,
					size_t operand_count, void *user_data)
{
	struct avctp_passthrough_handler *handler = session->handler;
	const char *status;
	int pressed, i;

	if (*code != AVC_CTYPE_CONTROL || *subunit != AVC_SUBUNIT_PANEL) {
		*code = AVC_CTYPE_REJECTED;
		return operand_count;
	}

	if (operand_count == 0)
		goto done;

	if (operands[0] & 0x80) {
		status = "released";
		pressed = 0;
	} else {
		status = "pressed";
		pressed = 1;
	}

	if (session->key.timer == 0 && handler != NULL) {
		if (handler->cb(session, operands[0] & 0x7F,
						pressed, handler->user_data))
			goto done;
	}

	if (session->uinput < 0) {
		DBG("AV/C: uinput not initialized");
		*code = AVC_CTYPE_NOT_IMPLEMENTED;
		return 0;
	}

	for (i = 0; key_map[i].name != NULL; i++) {
		uint8_t key_quirks;

		if ((operands[0] & 0x7F) != key_map[i].avc)
			continue;

		DBG("AV/C: %s %s", key_map[i].name, status);

		key_quirks = session->key_quirks[key_map[i].avc];

		if (key_quirks & QUIRK_NO_RELEASE) {
			if (!pressed) {
				DBG("AV/C: Ignoring release");
				break;
			}

			DBG("AV/C: treating key press as press + release");
			send_key(session->uinput, key_map[i].uinput, 1);
			send_key(session->uinput, key_map[i].uinput, 0);
			break;
		}

		if (pressed) {
			if (session->key.timer > 0) {
				g_source_remove(session->key.timer);
				send_key(session->uinput, session->key.op, 0);
			}

			session->key.op = key_map[i].uinput;
			session->key.timer = g_timeout_add_seconds(
							AVC_PRESS_TIMEOUT,
							auto_release,
							session);
		} else if (session->key.timer > 0) {
			g_source_remove(session->key.timer);
			session->key.timer = 0;
		}

		send_key(session->uinput, key_map[i].uinput, pressed);
		break;
	}

	if (key_map[i].name == NULL) {
		DBG("AV/C: unknown button 0x%02X %s",
						operands[0] & 0x7F, status);
		*code = AVC_CTYPE_NOT_IMPLEMENTED;
		return operand_count;
	}

done:
	*code = AVC_CTYPE_ACCEPTED;
	return operand_count;
}

static ssize_t handle_unit_info(struct avctp *session,
					uint8_t transaction, uint8_t *code,
					uint8_t *subunit, uint8_t *operands,
					size_t operand_count, void *user_data)
{
	if (*code != AVC_CTYPE_STATUS) {
		*code = AVC_CTYPE_REJECTED;
		return 0;
	}

	*code = AVC_CTYPE_STABLE;

	/*
	 * The first operand should be 0x07 for the UNITINFO response.
	 * Neither AVRCP (section 22.1, page 117) nor AVC Digital
	 * Interface Command Set (section 9.2.1, page 45) specs
	 * explain this value but both use it
	 */
	if (operand_count >= 1)
		operands[0] = 0x07;
	if (operand_count >= 2)
		operands[1] = AVC_SUBUNIT_PANEL << 3;

	DBG("reply to AVC_OP_UNITINFO");

	return operand_count;
}

static ssize_t handle_subunit_info(struct avctp *session,
					uint8_t transaction, uint8_t *code,
					uint8_t *subunit, uint8_t *operands,
					size_t operand_count, void *user_data)
{
	if (*code != AVC_CTYPE_STATUS) {
		*code = AVC_CTYPE_REJECTED;
		return 0;
	}

	*code = AVC_CTYPE_STABLE;

	/*
	 * The first operand should be 0x07 for the UNITINFO response.
	 * Neither AVRCP (section 22.1, page 117) nor AVC Digital
	 * Interface Command Set (section 9.2.1, page 45) specs
	 * explain this value but both use it
	 */
	if (operand_count >= 2)
		operands[1] = AVC_SUBUNIT_PANEL << 3;

	DBG("reply to AVC_OP_SUBUNITINFO");

	return operand_count;
}

static struct avctp_pdu_handler *find_handler(GSList *list, uint8_t opcode)
{
	for (; list; list = list->next) {
		struct avctp_pdu_handler *handler = list->data;

		if (handler->opcode == opcode)
			return handler;
	}

	return NULL;
}

static void pending_destroy(gpointer data, gpointer user_data)
{
	struct avctp_pending_req *req = data;

	if (req->destroy)
		req->destroy(req->data);

	if (req->timeout > 0)
		g_source_remove(req->timeout);

	g_free(req);
}

static void avctp_channel_destroy(struct avctp_channel *chan)
{
	g_io_channel_shutdown(chan->io, TRUE, NULL);
	g_io_channel_unref(chan->io);

	if (chan->watch)
		g_source_remove(chan->watch);

	if (chan->p)
		pending_destroy(chan->p, NULL);

	if (chan->process_id > 0)
		g_source_remove(chan->process_id);

	if (chan->destroy)
		chan->destroy(chan);

	g_free(chan->buffer);
	g_queue_foreach(chan->queue, pending_destroy, NULL);
	g_queue_free(chan->queue);
	g_slist_foreach(chan->processed, pending_destroy, NULL);
	g_slist_free(chan->processed);
	g_slist_free_full(chan->handlers, g_free);
	g_free(chan);
}

static int avctp_send(struct avctp_channel *control, uint8_t transaction,
				uint8_t cr, uint8_t code,
				uint8_t subunit, uint8_t opcode,
				const struct iovec *iov, int iov_cnt)
{
	struct avctp_header avctp;
	struct avc_header avc;
	struct msghdr msg;
	int sk, err = 0;
	struct iovec pdu[iov_cnt + 2];
	int i;
	size_t len = sizeof(avctp) + sizeof(avc);

	DBG("");

	pdu[0].iov_base = &avctp;
	pdu[0].iov_len  = sizeof(avctp);
	pdu[1].iov_base = &avc;
	pdu[1].iov_len  = sizeof(avc);

	for (i = 0; i < iov_cnt; i++) {
		pdu[i + 2].iov_base = iov[i].iov_base;
		pdu[i + 2].iov_len  = iov[i].iov_len;
		len += iov[i].iov_len;
	}

	if (control->omtu < len)
		return -EOVERFLOW;

	sk = g_io_channel_unix_get_fd(control->io);

	memset(&avctp, 0, sizeof(avctp));

	avctp.transaction = transaction;
	avctp.packet_type = AVCTP_PACKET_SINGLE;
	avctp.cr = cr;
	avctp.pid = htons(AV_REMOTE_SVCLASS_ID);

	memset(&avc, 0, sizeof(avc));

	avc.code = code;
	avc.subunit_type = subunit;
	avc.opcode = opcode;

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = pdu;
	msg.msg_iovlen = iov_cnt + 2;

	if (sendmsg(sk, &msg, 0) < 0)
		err = -errno;

	return err;
}

static int avctp_browsing_send(struct avctp_channel *browsing,
				uint8_t transaction, uint8_t cr,
				const struct iovec *iov, int iov_cnt)
{
	struct avctp_header avctp;
	struct msghdr msg;
	struct iovec pdu[iov_cnt + 1];
	int sk, err = 0;
	int i;
	size_t len = sizeof(avctp);

	for (i = 0; i < iov_cnt; i++) {
		pdu[i + 1].iov_base = iov[i].iov_base;
		pdu[i + 1].iov_len  = iov[i].iov_len;
		len += iov[i].iov_len;
	}

	pdu[0].iov_base = &avctp;
	pdu[0].iov_len  = sizeof(avctp);

	if (browsing->omtu < len)
		return -EOVERFLOW;

	sk = g_io_channel_unix_get_fd(browsing->io);

	memset(&avctp, 0, sizeof(avctp));

	avctp.transaction = transaction;
	avctp.packet_type = AVCTP_PACKET_SINGLE;
	avctp.cr = cr;
	avctp.pid = htons(AV_REMOTE_SVCLASS_ID);

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = pdu;
	msg.msg_iovlen = iov_cnt + 1;

	if (sendmsg(sk, &msg, 0) < 0)
		err = -errno;

	return err;
}

static void control_req_destroy(void *data)
{
	struct avctp_control_req *req = data;
	struct avctp_pending_req *p = req->p;
	struct avctp *session = p->chan->session;
	int i;

	if (p->err == 0 || req->func == NULL)
		goto done;

	req->func(session, AVC_CTYPE_REJECTED, req->subunit, NULL, 0,
							req->user_data);

done:
	for (i = 0; i < req->iov_cnt; i++)
		g_free(req->iov[i].iov_base);

	g_free(req->iov);
	g_free(req);
}

static void browsing_req_destroy(void *data)
{
	struct avctp_browsing_req *req = data;
	struct avctp_pending_req *p = req->p;
	struct avctp *session = p->chan->session;
	int i;

	if (p->err == 0 || req->func == NULL)
		goto done;

	req->func(session, NULL, 0, req->user_data);

done:
	for (i = 0; i < req->iov_cnt; i++)
		g_free(req->iov[i].iov_base);

	g_free(req->iov);
	g_free(req);
}

static gboolean req_timeout(gpointer user_data)
{
	struct avctp_channel *chan = user_data;
	struct avctp_pending_req *p = chan->p;

	DBG("transaction %u", p->transaction);

	p->timeout = 0;
	p->err = -ETIMEDOUT;

	pending_destroy(p, NULL);
	chan->p = NULL;

	if (chan->process_id == 0)
		chan->process_id = g_idle_add(process_queue, chan);

	return FALSE;
}

static int process_control(void *data)
{
	struct avctp_control_req *req = data;
	struct avctp_pending_req *p = req->p;

	return avctp_send(p->chan, p->transaction, AVCTP_COMMAND, req->code,
				req->subunit, req->op, req->iov, req->iov_cnt);
}

static int process_browsing(void *data)
{
	struct avctp_browsing_req *req = data;
	struct avctp_pending_req *p = req->p;

	return avctp_browsing_send(p->chan, p->transaction, AVCTP_COMMAND,
						req->iov, req->iov_cnt);
}

static gboolean process_queue(void *user_data)
{
	struct avctp_channel *chan = user_data;
	struct avctp_pending_req *p = chan->p;

	chan->process_id = 0;

	if (p != NULL)
		return FALSE;

	while ((p = g_queue_pop_head(chan->queue))) {

		if (p->process(p->data) == 0)
			break;

		pending_destroy(p, NULL);
	}

	if (p == NULL)
		return FALSE;

	chan->p = p;
	p->timeout = g_timeout_add_seconds(2, req_timeout, chan);

	return FALSE;

}

static struct avctp *avctp_ref(struct avctp *session)
{
	__sync_fetch_and_add(&session->ref, 1);

	DBG("%p: ref=%d", session, session->ref);

	return session;
}

static void avctp_unref(struct avctp *session)
{
	DBG("%p: ref=%d", session, session->ref);

	if (__sync_sub_and_fetch(&session->ref, 1))
		return;

	if (session->browsing)
		avctp_channel_destroy(session->browsing);

	if (session->control)
		avctp_channel_destroy(session->control);

	if (session->destroy)
		session->destroy(session->data);

	g_free(session->handler);

	if (session->key.timer > 0)
		g_source_remove(session->key.timer);

	if (session->uinput >= 0) {
		DBG("AVCTP: closing uinput");

		ioctl(session->uinput, UI_DEV_DESTROY);
		close(session->uinput);
		session->uinput = -1;
	}

	g_free(session);
}

static void control_response(struct avctp_channel *control,
					struct avctp_header *avctp,
					struct avc_header *avc,
					uint8_t *operands,
					size_t operand_count)
{
	struct avctp_pending_req *p = control->p;
	struct avctp_control_req *req;
	GSList *l;

	if (p && p->transaction == avctp->transaction) {
		control->processed = g_slist_prepend(control->processed, p);

		if (p->timeout > 0) {
			g_source_remove(p->timeout);
			p->timeout = 0;
		}

		control->p = NULL;

		if (control->process_id == 0)
			control->process_id = g_idle_add(process_queue,
								control);
	}

	avctp_ref(control->session);

	for (l = control->processed; l; l = l->next) {
		p = l->data;
		req = p->data;

		if (p->transaction != avctp->transaction)
			continue;

		if (req->func && req->func(control->session, avc->code,
						avc->subunit_type,
						operands, operand_count,
						req->user_data))
			break;

		control->processed = g_slist_remove(control->processed, p);
		pending_destroy(p, NULL);

		break;
	}

	avctp_unref(control->session);
}

static void browsing_response(struct avctp_channel *browsing,
					struct avctp_header *avctp,
					uint8_t *operands,
					size_t operand_count)
{
	struct avctp_pending_req *p = browsing->p;
	struct avctp_browsing_req *req;
	GSList *l;

	if (p && p->transaction == avctp->transaction) {
		browsing->processed = g_slist_prepend(browsing->processed, p);

		if (p->timeout > 0) {
			g_source_remove(p->timeout);
			p->timeout = 0;
		}

		browsing->p = NULL;

		if (browsing->process_id == 0)
			browsing->process_id = g_idle_add(process_queue,
								browsing);
	}

	avctp_ref(browsing->session);

	for (l = browsing->processed; l; l = l->next) {
		p = l->data;
		req = p->data;

		if (p->transaction != avctp->transaction)
			continue;

		if (req->func && req->func(browsing->session, operands,
						operand_count, req->user_data))
			break;

		browsing->processed = g_slist_remove(browsing->processed, p);
		pending_destroy(p, NULL);

		break;
	}

	avctp_unref(browsing->session);
}

static gboolean session_browsing_cb(GIOChannel *chan, GIOCondition cond,
				gpointer data)
{
	struct avctp *session = data;
	struct avctp_channel *browsing = session->browsing;
	uint8_t *buf = browsing->buffer;
	uint8_t *operands;
	struct avctp_header *avctp;
	int sock, ret, packet_size, operand_count;
	struct avctp_browsing_pdu_handler *handler;

	if (cond & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
		goto failed;

	sock = g_io_channel_unix_get_fd(chan);

	ret = read(sock, buf, browsing->imtu);
	if (ret <= 0)
		goto failed;

	if (ret < AVCTP_HEADER_LENGTH) {
		error("Too small AVCTP packet");
		goto failed;
	}

	avctp = (struct avctp_header *) buf;

	if (avctp->packet_type != AVCTP_PACKET_SINGLE) {
		error("Invalid packet type");
		goto failed;
	}

	operands = buf + AVCTP_HEADER_LENGTH;
	ret -= AVCTP_HEADER_LENGTH;
	operand_count = ret;

	if (avctp->cr == AVCTP_RESPONSE) {
		browsing_response(browsing, avctp, operands, operand_count);
		return TRUE;
	}

	packet_size = AVCTP_HEADER_LENGTH;
	avctp->cr = AVCTP_RESPONSE;

	handler = g_slist_nth_data(browsing->handlers, 0);
	if (handler == NULL) {
		DBG("handler not found");
		/* FIXME: Add general reject */
		/* packet_size += avrcp_browsing_general_reject(operands); */
		goto send;
	}

	ret = handler->cb(session, avctp->transaction, operands, operand_count,
							handler->user_data);
	if (ret < 0) {
		if (ret == -EAGAIN)
			return TRUE;
		goto failed;
	}

	packet_size += ret;

send:
	if (packet_size != 0) {
		ret = write(sock, buf, packet_size);
		if (ret != packet_size)
			goto failed;
	}

	return TRUE;

failed:
	DBG("AVCTP Browsing: disconnected");

	if (session->browsing) {
		avctp_channel_destroy(session->browsing);
		session->browsing = NULL;
	}

	return FALSE;
}

static gboolean session_cb(GIOChannel *chan, GIOCondition cond, gpointer data)
{
	struct avctp *session = data;
	struct avctp_channel *control = session->control;
	uint8_t *buf = control->buffer;
	uint8_t *operands, code, subunit;
	struct avctp_header *avctp;
	struct avc_header *avc;
	int packet_size, operand_count, sock;
	struct avctp_pdu_handler *handler;
	ssize_t ret;

	if (cond & (G_IO_ERR | G_IO_HUP | G_IO_NVAL))
		goto failed;

	sock = g_io_channel_unix_get_fd(chan);

	ret = read(sock, buf, control->imtu);
	if (ret <= 0)
		goto failed;

	if (ret < AVCTP_HEADER_LENGTH) {
		error("Too small AVCTP packet");
		goto failed;
	}

	avctp = (struct avctp_header *) buf;

	ret -= AVCTP_HEADER_LENGTH;
	if (ret < AVC_HEADER_LENGTH) {
		error("Too small AVC packet");
		goto failed;
	}

	avc = (struct avc_header *) (buf + AVCTP_HEADER_LENGTH);

	ret -= AVC_HEADER_LENGTH;

	operands = (uint8_t *) avc + AVC_HEADER_LENGTH;
	operand_count = ret;

	if (avctp->cr == AVCTP_RESPONSE) {
		control_response(control, avctp, avc, operands, operand_count);
		return TRUE;
	}

	packet_size = AVCTP_HEADER_LENGTH + AVC_HEADER_LENGTH;
	avctp->cr = AVCTP_RESPONSE;

	if (avctp->packet_type != AVCTP_PACKET_SINGLE) {
		avc->code = AVC_CTYPE_NOT_IMPLEMENTED;
		goto done;
	}

	if (avctp->pid != htons(AV_REMOTE_SVCLASS_ID)) {
		avctp->ipid = 1;
		packet_size = AVCTP_HEADER_LENGTH;
		goto done;
	}

	handler = find_handler(control->handlers, avc->opcode);
	if (!handler) {
		DBG("handler not found for 0x%02x", avc->opcode);
		avc->code = AVC_CTYPE_REJECTED;
		goto done;
	}

	code = avc->code;
	subunit = avc->subunit_type;

	ret = handler->cb(session, avctp->transaction, &code,
					&subunit, operands, operand_count,
					handler->user_data);
	if (ret < 0) {
		if (ret == -EAGAIN)
			return TRUE;
		goto failed;
	}

	packet_size += ret;
	avc->code = code;
	avc->subunit_type = subunit;

done:
	ret = write(sock, buf, packet_size);
	if (ret != packet_size)
		goto failed;

	return TRUE;

failed:
	DBG("AVCTP session %p got disconnected", session);
	avctp_shutdown(session);
	return FALSE;
}

static int uinput_create(const char *name)
{
	struct uinput_dev dev;
	int fd, err, i;

	fd = open("/dev/uinput", O_RDWR);
	if (fd < 0) {
		fd = open("/dev/input/uinput", O_RDWR);
		if (fd < 0) {
			fd = open("/dev/misc/uinput", O_RDWR);
			if (fd < 0) {
				err = -errno;
				error("Can't open input device: %s (%d)",
							strerror(-err), -err);
				return err;
			}
		}
	}

	memset(&dev, 0, sizeof(dev));
	if (name)
		strncpy(dev.name, name, UINPUT_MAX_NAME_SIZE - 1);

	dev.id.bustype = BUS_BLUETOOTH;
	dev.id.vendor  = 0x0000;
	dev.id.product = 0x0000;
	dev.id.version = 0x0000;

	if (write(fd, &dev, sizeof(dev)) < 0) {
		err = -errno;
		error("Can't write device information: %s (%d)",
						strerror(-err), -err);
		close(fd);
		return err;
	}

	ioctl(fd, UI_SET_EVBIT, EV_KEY);
	ioctl(fd, UI_SET_EVBIT, EV_REL);
	ioctl(fd, UI_SET_EVBIT, EV_REP);
	ioctl(fd, UI_SET_EVBIT, EV_SYN);

	for (i = 0; key_map[i].name != NULL; i++)
		ioctl(fd, UI_SET_KEYBIT, key_map[i].uinput);

	if (ioctl(fd, UI_DEV_CREATE, NULL) < 0) {
		err = -errno;
		error("Can't create uinput device: %s (%d)",
						strerror(-err), -err);
		close(fd);
		return err;
	}

	return fd;
}

int avctp_init_uinput(struct avctp *session, const char *name,
							const char *address)
{
	if (g_str_equal(name, "Nokia CK-20W")) {
		session->key_quirks[AVC_FORWARD] |= QUIRK_NO_RELEASE;
		session->key_quirks[AVC_BACKWARD] |= QUIRK_NO_RELEASE;
		session->key_quirks[AVC_PLAY] |= QUIRK_NO_RELEASE;
		session->key_quirks[AVC_PAUSE] |= QUIRK_NO_RELEASE;
	}

	session->uinput = uinput_create(address);
	if (session->uinput < 0) {
		error("AVCTP: failed to init uinput for %s", address);
		return session->uinput;
	}

	return 0;
}

static struct avctp_channel *avctp_channel_create(struct avctp *session, int fd,
						size_t imtu, size_t omtu,
						avctp_destroy_cb_t destroy)
{
	struct avctp_channel *chan;

	chan = g_new0(struct avctp_channel, 1);
	chan->session = session;
	chan->io = g_io_channel_unix_new(fd);
	chan->queue = g_queue_new();
	chan->imtu = imtu;
	chan->omtu = omtu;
	chan->buffer = g_malloc0(MAX(imtu, omtu));
	chan->destroy = destroy;

	return chan;
}

static void handler_free(void *data)
{
	struct avctp_browsing_pdu_handler *handler = data;

	if (handler->destroy)
		handler->destroy(handler->user_data);

	g_free(data);
}

static void avctp_destroy_browsing(void *data)
{
	struct avctp_channel *chan = data;

	g_slist_free_full(chan->handlers, handler_free);

	chan->handlers = NULL;
}

static struct avctp_pending_req *pending_create(struct avctp_channel *chan,
						avctp_process_cb process,
						void *data,
						avctp_destroy_cb_t destroy)
{
	struct avctp_pending_req *p;
	GSList *l, *tmp;

	if (!chan->processed)
		goto done;

	tmp = g_slist_copy(chan->processed);

	/* Find first unused transaction id */
	for (l = tmp; l; l = g_slist_next(l)) {
		struct avctp_pending_req *req = l->data;

		if (req->transaction == chan->transaction) {
			chan->transaction++;
			chan->transaction %= 16;
			tmp = g_slist_delete_link(tmp, l);
			l = tmp;
		}
	}

	g_slist_free(tmp);

done:
	p = g_new0(struct avctp_pending_req, 1);
	p->chan = chan;
	p->transaction = chan->transaction;
	p->process = process;
	p->data = data;
	p->destroy = destroy;

	chan->transaction++;
	chan->transaction %= 16;

	return p;
}

static int avctp_send_req(struct avctp *session, uint8_t code, uint8_t subunit,
			uint8_t opcode, const struct iovec *iov, int iov_cnt,
			avctp_rsp_cb func, void *user_data)
{
	struct avctp_channel *control = session->control;
	struct avctp_pending_req *p;
	struct avctp_control_req *req;
	struct iovec *pdu;
	int i;

	if (control == NULL)
		return -ENOTCONN;

	pdu = g_new0(struct iovec, iov_cnt);

	for (i = 0; i < iov_cnt; i++) {
		pdu[i].iov_len = iov[i].iov_len;
		pdu[i].iov_base = g_memdup(iov[i].iov_base, iov[i].iov_len);
	}

	req = g_new0(struct avctp_control_req, 1);
	req->code = code;
	req->subunit = subunit;
	req->op = opcode;
	req->func = func;
	req->iov = pdu;
	req->iov_cnt = iov_cnt;
	req->user_data = user_data;

	p = pending_create(control, process_control, req, control_req_destroy);

	req->p = p;

	g_queue_push_tail(control->queue, p);

	if (control->process_id == 0)
		control->process_id = g_idle_add(process_queue, control);

	return 0;
}

int avctp_send_browsing_req(struct avctp *session,
				const struct iovec *iov, int iov_cnt,
				avctp_browsing_rsp_cb func, void *user_data)
{
	struct avctp_channel *browsing = session->browsing;
	struct avctp_pending_req *p;
	struct avctp_browsing_req *req;
	struct iovec *pdu;
	int i;

	if (browsing == NULL)
		return -ENOTCONN;

	pdu = g_new0(struct iovec, iov_cnt);

	for (i = 0; i < iov_cnt; i++) {
		pdu[i].iov_len = iov[i].iov_len;
		pdu[i].iov_base = g_memdup(iov[i].iov_base, iov[i].iov_len);
	}

	req = g_new0(struct avctp_browsing_req, 1);
	req->func = func;
	req->iov = pdu;
	req->iov_cnt = iov_cnt;
	req->user_data = user_data;

	p = pending_create(browsing, process_browsing, req,
			browsing_req_destroy);

	req->p = p;

	g_queue_push_tail(browsing->queue, p);

	/* Connection did not complete, delay process of the request */
	if (browsing->watch == 0)
		return 0;

	if (browsing->process_id == 0)
		browsing->process_id = g_idle_add(process_queue, browsing);

	return 0;
}

int avctp_send_browsing(struct avctp *session, uint8_t transaction,
					const struct iovec *iov, int iov_cnt)
{
	struct avctp_channel *browsing = session->browsing;

	if (browsing == NULL)
		return -ENOTCONN;

	return avctp_browsing_send(browsing, transaction, AVCTP_RESPONSE,
								iov, iov_cnt);
}

static const char *op2str(uint8_t op)
{
	int i;

	for (i = 0; key_map[i].name != NULL; i++) {
		if ((op & 0x7F) == key_map[i].avc)
			return key_map[i].name;
	}

	return "UNKNOWN";
}

static int avctp_passthrough_press(struct avctp *session, uint8_t op,
					uint8_t *params, size_t params_len)
{
	struct iovec iov[2];
	int iov_cnt;
	uint8_t operands[2];

	DBG("%s", op2str(op));

	iov[0].iov_base = operands;
	iov[0].iov_len = sizeof(operands);

	/* Button pressed */
	operands[0] = op & 0x7f;

	if (params_len > 0) {
		iov[1].iov_base = params;
		iov[1].iov_len = params_len;
		iov_cnt = 2;
		operands[1] = params_len;
	} else {
		iov_cnt = 1;
		operands[1] = 0;
	}

	return avctp_send_req(session, AVC_CTYPE_CONTROL,
				AVC_SUBUNIT_PANEL, AVC_OP_PASSTHROUGH,
				iov, iov_cnt, avctp_passthrough_rsp, NULL);
}

static int avctp_passthrough_release(struct avctp *session, uint8_t op,
					uint8_t *params, size_t params_len)
{
	struct iovec iov[2];
	int iov_cnt;
	uint8_t operands[2];

	DBG("%s", op2str(op));

	iov[0].iov_base = operands;
	iov[0].iov_len = sizeof(operands);

	/* Button released */
	operands[0] = op | 0x80;

	if (params_len > 0) {
		iov[1].iov_base = params;
		iov[1].iov_len = params_len;
		iov_cnt = 2;
		operands[1] = params_len;
	} else {
		iov_cnt = 1;
		operands[1] = 0;
	}

	return avctp_send_req(session, AVC_CTYPE_CONTROL,
				AVC_SUBUNIT_PANEL, AVC_OP_PASSTHROUGH,
				iov, iov_cnt, NULL, NULL);
}

static gboolean repeat_timeout(gpointer user_data)
{
	struct avctp *session = user_data;

	avctp_passthrough_release(session, session->key.op, session->key.params,
						session->key.params_len);
	avctp_passthrough_press(session, session->key.op, session->key.params,
						session->key.params_len);

	return TRUE;
}

static void release_pressed(struct avctp *session)
{
	avctp_passthrough_release(session, session->key.op, session->key.params,
						session->key.params_len);

	if (session->key.timer > 0)
		g_source_remove(session->key.timer);

	session->key.timer = 0;
}

static bool set_pressed(struct avctp *session, uint8_t op, uint8_t *params,
							size_t params_len)
{
	if (session->key.timer > 0) {
		if (session->key.op == op)
			return TRUE;
		release_pressed(session);
	}

	if (op != AVC_FAST_FORWARD && op != AVC_REWIND)
		return FALSE;

	session->key.op = op;
	session->key.params = params;
	session->key.params_len = params_len;
	session->key.timer = g_timeout_add_seconds(AVC_PRESS_TIMEOUT,
							repeat_timeout,
							session);

	return TRUE;
}

static gboolean avctp_passthrough_rsp(struct avctp *session, uint8_t code,
					uint8_t subunit, uint8_t *operands,
					size_t operand_count, void *user_data)
{
	uint8_t *params;
	size_t params_len;

	DBG("code 0x%02x operand_count %zd", code, operand_count);

	if (code != AVC_CTYPE_ACCEPTED)
		return FALSE;

	if (operands[0] == AVC_VENDOR_UNIQUE) {
		params = &operands[2];
		params_len = operand_count - 2;
	} else {
		params = NULL;
		params_len = 0;
	}

	if (set_pressed(session, operands[0], params, params_len))
		return FALSE;

	avctp_passthrough_release(session, operands[0], params, params_len);

	return FALSE;
}

int avctp_send_passthrough(struct avctp *session, uint8_t op, uint8_t *params,
							size_t params_len)
{
	/* Auto release if key pressed */
	if (session->key.timer > 0)
		release_pressed(session);

	return avctp_passthrough_press(session, op, params, params_len);
}

int avctp_send_vendor(struct avctp *session, uint8_t transaction,
				uint8_t code, uint8_t subunit,
				const struct iovec *iov, int iov_cnt)
{
	struct avctp_channel *control = session->control;

	if (control == NULL)
		return -ENOTCONN;

	return avctp_send(control, transaction, AVCTP_RESPONSE, code, subunit,
						AVC_OP_VENDORDEP, iov, iov_cnt);
}

int avctp_send_vendor_req(struct avctp *session, uint8_t code, uint8_t subunit,
					const struct iovec *iov, int iov_cnt,
					avctp_rsp_cb func, void *user_data)
{
	return avctp_send_req(session, code, subunit, AVC_OP_VENDORDEP, iov,
						iov_cnt, func, user_data);
}

unsigned int avctp_register_passthrough_handler(struct avctp *session,
						avctp_passthrough_cb cb,
						void *user_data)
{
	struct avctp_channel *control = session->control;
	struct avctp_passthrough_handler *handler;
	static unsigned int id = 0;

	if (control == NULL || session->handler != NULL)
		return 0;

	handler = g_new(struct avctp_passthrough_handler, 1);
	handler->cb = cb;
	handler->user_data = user_data;
	handler->id = ++id;

	session->handler = handler;

	return handler->id;
}

bool avctp_unregister_passthrough_handler(struct avctp *session,
							unsigned int id)
{
	if (session->handler == NULL)
		return false;

	if (session->handler->id != id)
		return false;

	g_free(session->handler);
	session->handler = NULL;
	return true;
}

unsigned int avctp_register_pdu_handler(struct avctp *session, uint8_t opcode,
						avctp_control_pdu_cb cb,
						void *user_data)
{
	struct avctp_channel *control = session->control;
	struct avctp_pdu_handler *handler;
	static unsigned int id = 0;

	if (control == NULL)
		return 0;

	handler = find_handler(control->handlers, opcode);
	if (handler)
		return 0;

	handler = g_new(struct avctp_pdu_handler, 1);
	handler->opcode = opcode;
	handler->cb = cb;
	handler->user_data = user_data;
	handler->id = ++id;

	control->handlers = g_slist_append(control->handlers, handler);

	return handler->id;
}

unsigned int avctp_register_browsing_pdu_handler(struct avctp *session,
						avctp_browsing_pdu_cb cb,
						void *user_data,
						avctp_destroy_cb_t destroy)
{
	struct avctp_channel *browsing = session->browsing;
	struct avctp_browsing_pdu_handler *handler;
	static unsigned int id = 0;

	if (browsing == NULL)
		return 0;

	if (browsing->handlers != NULL)
		return 0;

	handler = g_new(struct avctp_browsing_pdu_handler, 1);
	handler->cb = cb;
	handler->user_data = user_data;
	handler->id = ++id;
	handler->destroy = destroy;

	browsing->handlers = g_slist_append(browsing->handlers, handler);

	return handler->id;
}

bool avctp_unregister_pdu_handler(struct avctp *session, unsigned int id)
{
	struct avctp_channel *control = session->control;
	GSList *l;

	if (!control)
		return false;

	for (l = control->handlers; l; l = g_slist_next(l)) {
		struct avctp_pdu_handler *handler = l->data;

		if (handler->id != id)
			continue;

		control->handlers = g_slist_remove(control->handlers, handler);
		g_free(handler);
		return true;
	}

	return false;
}

bool avctp_unregister_browsing_pdu_handler(struct avctp *session,
							unsigned int id)
{
	struct avctp_channel *browsing = session->browsing;
	GSList *l;

	if (browsing == NULL)
		return false;

	for (l = browsing->handlers; l; l = g_slist_next(l)) {
		struct avctp_browsing_pdu_handler *handler = l->data;

		if (handler->id != id)
			continue;

		browsing->handlers = g_slist_remove(browsing->handlers,
								handler);
		g_free(handler);
		return true;
	}

	return false;
}

struct avctp *avctp_new(int fd, size_t imtu, size_t omtu, uint16_t version)
{
	struct avctp *session;
	struct avctp_channel *control;
	GIOCondition cond = G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL;

	session = g_new0(struct avctp, 1);
	session->version = version;

	control = avctp_channel_create(session, fd, imtu, omtu, NULL);
	if (!control) {
		g_free(session);
		return NULL;
	}

	session->uinput = -1;
	session->control = control;
	session->passthrough_id = avctp_register_pdu_handler(session,
						AVC_OP_PASSTHROUGH,
						handle_panel_passthrough,
						NULL);
	session->unit_id = avctp_register_pdu_handler(session,
						AVC_OP_UNITINFO,
						handle_unit_info,
						NULL);
	session->subunit_id = avctp_register_pdu_handler(session,
						AVC_OP_SUBUNITINFO,
						handle_subunit_info,
						NULL);

	control->watch = g_io_add_watch(session->control->io, cond,
						(GIOFunc) session_cb, session);

	return avctp_ref(session);
}

int avctp_connect_browsing(struct avctp *session, int fd, size_t imtu,
								size_t omtu)
{
	struct avctp_channel *browsing;
	GIOCondition cond = G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL;

	if (session->browsing)
		return -EISCONN;

	browsing = avctp_channel_create(session, fd, imtu, omtu,
						avctp_destroy_browsing);
	if (!browsing)
		return -EINVAL;

	session->browsing = browsing;
	browsing->watch = g_io_add_watch(session->browsing->io, cond,
					(GIOFunc) session_browsing_cb, session);

	return 0;
}

void avctp_set_destroy_cb(struct avctp *session, avctp_destroy_cb_t cb,
							void *user_data)
{
	session->destroy = cb;
	session->data = user_data;
}

void avctp_shutdown(struct avctp *session)
{
	if (!session)
		return;

	avctp_unref(session);
}

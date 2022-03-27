/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2006-2010  Nokia Corporation
 *  Copyright (C) 2004-2010  Marcel Holtmann <marcel@holtmann.org>
 *  Copyright (C) 2011  Texas Instruments, Inc.
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

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>

#include <glib.h>

#include "lib/bluetooth.h"
#include "lib/sdp.h"
#include "lib/l2cap.h"
#include "lib/uuid.h"

#include "btio/btio.h"
#include "src/adapter.h"
#include "src/device.h"
#include "src/log.h"
#include "src/error.h"
#include "src/uinput.h"

#include "avctp.h"
#include "avrcp.h"

/* AV/C Panel 1.23, page 76:
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
#define AVCTP_HEADER_LENGTH 3

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
#define AVCTP_HEADER_LENGTH 3

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

struct avctp_state_callback {
	avctp_state_cb cb;
	struct btd_device *dev;
	unsigned int id;
	void *user_data;
};

struct avctp_server {
	struct btd_adapter *adapter;
	GIOChannel *control_io;
	GIOChannel *browsing_io;
	GSList *sessions;
};

struct avctp_control_req {
	struct avctp_pending_req *p;
	uint8_t code;
	uint8_t subunit;
	uint8_t op;
	uint8_t *operands;
	uint16_t operand_count;
	avctp_rsp_cb func;
	void *user_data;
};

struct avctp_browsing_req {
	struct avctp_pending_req *p;
	uint8_t *operands;
	uint16_t operand_count;
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
	GDestroyNotify destroy;
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
	GDestroyNotify destroy;
};

struct key_pressed {
	uint16_t op;
	guint timer;
};

struct avctp {
	struct avctp_server *server;
	struct btd_device *device;

	avctp_state_t state;

	int uinput;

	guint auth_id;
	unsigned int passthrough_id;
	unsigned int unit_id;
	unsigned int subunit_id;

	struct avctp_channel *control;
	struct avctp_channel *browsing;

	struct avctp_passthrough_handler *handler;

	uint8_t key_quirks[256];
	struct key_pressed key;
	bool initiator;
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
	GDestroyNotify destroy;
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

static GSList *callbacks = NULL;
static GSList *servers = NULL;

static void auth_cb(DBusError *derr, void *user_data);
static gboolean process_queue(gpointer user_data);
static gboolean avctp_passthrough_rsp(struct avctp *session, uint8_t code,
					uint8_t subunit, uint8_t transaction,
					uint8_t *operands, size_t operand_count,
					void *user_data);

static int send_event(int fd, uint16_t type, uint16_t code, int32_t value)
{
	struct uinput_event event;

	memset(&event, 0, sizeof(event));
	event.type	= type;
	event.code	= code;
	event.value	= value;

	return write(fd, &event, sizeof(event));
}

static void send_key(int fd, uint16_t key, int pressed)
{
	if (fd < 0)
		return;

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

static void handle_press(struct avctp *session, uint16_t op)
{
	if (session->key.timer > 0) {
		g_source_remove(session->key.timer);

		/* Only auto release if keys are different */
		if (session->key.op == op)
			goto done;

		send_key(session->uinput, session->key.op, 0);
	}

	session->key.op = op;

	send_key(session->uinput, op, 1);

done:
	session->key.timer = g_timeout_add_seconds(AVC_PRESS_TIMEOUT,
							auto_release, session);
}

static void handle_release(struct avctp *session, uint16_t op)
{
	if (session->key.timer > 0) {
		g_source_remove(session->key.timer);
		session->key.timer = 0;
	}

	send_key(session->uinput, op, 0);
}

static size_t handle_panel_passthrough(struct avctp *session,
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

		if (pressed)
			handle_press(session, key_map[i].uinput);
		else
			handle_release(session, key_map[i].uinput);

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

static size_t handle_unit_info(struct avctp *session,
					uint8_t transaction, uint8_t *code,
					uint8_t *subunit, uint8_t *operands,
					size_t operand_count, void *user_data)
{
	if (*code != AVC_CTYPE_STATUS) {
		*code = AVC_CTYPE_REJECTED;
		return 0;
	}

	*code = AVC_CTYPE_STABLE;

	/* The first operand should be 0x07 for the UNITINFO response.
	 * Neither AVRCP (section 22.1, page 117) nor AVC Digital
	 * Interface Command Set (section 9.2.1, page 45) specs
	 * explain this value but both use it */
	if (operand_count >= 1)
		operands[0] = 0x07;
	if (operand_count >= 2)
		operands[1] = AVC_SUBUNIT_PANEL << 3;

	DBG("reply to AVC_OP_UNITINFO");

	return operand_count;
}

static size_t handle_subunit_info(struct avctp *session,
					uint8_t transaction, uint8_t *code,
					uint8_t *subunit, uint8_t *operands,
					size_t operand_count, void *user_data)
{
	if (*code != AVC_CTYPE_STATUS) {
		*code = AVC_CTYPE_REJECTED;
		return 0;
	}

	*code = AVC_CTYPE_STABLE;

	/* The first operand should be 0x07 for the UNITINFO response.
	 * Neither AVRCP (section 22.1, page 117) nor AVC Digital
	 * Interface Command Set (section 9.2.1, page 45) specs
	 * explain this value but both use it */
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

static void avctp_disconnected(struct avctp *session)
{
	struct avctp_server *server;

	if (!session)
		return;

	if (session->browsing)
		avctp_channel_destroy(session->browsing);

	if (session->control)
		avctp_channel_destroy(session->control);

	if (session->auth_id != 0) {
		btd_cancel_authorization(session->auth_id);
		session->auth_id = 0;
	}

	if (session->key.timer > 0)
		g_source_remove(session->key.timer);

	if (session->uinput >= 0) {
		char address[18];

		ba2str(device_get_address(session->device), address);
		DBG("AVCTP: closing uinput for %s", address);

		ioctl(session->uinput, UI_DEV_DESTROY);
		close(session->uinput);
		session->uinput = -1;
	}

	server = session->server;
	server->sessions = g_slist_remove(server->sessions, session);
	btd_device_unref(session->device);
	g_free(session);
}

static void avctp_set_state(struct avctp *session, avctp_state_t new_state,
									int err)
{
	GSList *l;
	avctp_state_t old_state = session->state;

	session->state = new_state;

	for (l = callbacks; l != NULL; l = l->next) {
		struct avctp_state_callback *cb = l->data;

		if (cb->dev && cb->dev != session->device)
			continue;

		cb->cb(session->device, old_state, new_state, err,
								cb->user_data);
	}

	switch (new_state) {
	case AVCTP_STATE_DISCONNECTED:
		DBG("AVCTP Disconnected");
		avctp_disconnected(session);
		break;
	case AVCTP_STATE_CONNECTING:
		DBG("AVCTP Connecting");
		break;
	case AVCTP_STATE_CONNECTED:
		DBG("AVCTP Connected");
		break;
	case AVCTP_STATE_BROWSING_CONNECTING:
		DBG("AVCTP Browsing Connecting");
		break;
	case AVCTP_STATE_BROWSING_CONNECTED:
		DBG("AVCTP Browsing Connected");
		break;
	default:
		error("Invalid AVCTP state %d", new_state);
		return;
	}
}

static int avctp_send(struct avctp_channel *control, uint8_t transaction,
				uint8_t cr, uint8_t code,
				uint8_t subunit, uint8_t opcode,
				uint8_t *operands, size_t operand_count)
{
	struct avctp_header *avctp;
	struct avc_header *avc;
	struct msghdr msg;
	struct iovec iov[2];
	int sk, err = 0;

	iov[0].iov_base = control->buffer;
	iov[0].iov_len  = sizeof(*avctp) + sizeof(*avc);
	iov[1].iov_base = operands;
	iov[1].iov_len  = operand_count;

	if (control->omtu < (iov[0].iov_len + iov[1].iov_len))
		return -EOVERFLOW;

	sk = g_io_channel_unix_get_fd(control->io);

	memset(control->buffer, 0, iov[0].iov_len);

	avctp = (void *) control->buffer;
	avc = (void *) avctp + sizeof(*avctp);

	avctp->transaction = transaction;
	avctp->packet_type = AVCTP_PACKET_SINGLE;
	avctp->cr = cr;
	avctp->pid = htons(AV_REMOTE_SVCLASS_ID);

	avc->code = code;
	avc->subunit_type = subunit;
	avc->opcode = opcode;

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	if (sendmsg(sk, &msg, 0) < 0)
		err = -errno;

	return err;
}

static int avctp_browsing_send(struct avctp_channel *browsing,
				uint8_t transaction, uint8_t cr,
				uint8_t *operands, size_t operand_count)
{
	struct avctp_header *avctp;
	struct msghdr msg;
	struct iovec iov[2];
	int sk, err = 0;

	iov[0].iov_base = browsing->buffer;
	iov[0].iov_len  = sizeof(*avctp);
	iov[1].iov_base = operands;
	iov[1].iov_len  = operand_count;

	if (browsing->omtu < (iov[0].iov_len + iov[1].iov_len))
		return -EOVERFLOW;

	sk = g_io_channel_unix_get_fd(browsing->io);

	memset(browsing->buffer, 0, iov[0].iov_len);

	avctp = (void *) browsing->buffer;

	avctp->transaction = transaction;
	avctp->packet_type = AVCTP_PACKET_SINGLE;
	avctp->cr = cr;
	avctp->pid = htons(AV_REMOTE_SVCLASS_ID);

	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = iov;
	msg.msg_iovlen = 2;

	if (sendmsg(sk, &msg, 0) < 0)
		err = -errno;

	return err;
}

static void control_req_destroy(void *data)
{
	struct avctp_control_req *req = data;
	struct avctp_pending_req *p = req->p;
	struct avctp *session = p->chan->session;

	if (p->err == 0 || req->func == NULL)
		goto done;

	req->func(session, AVC_CTYPE_REJECTED, req->subunit, p->transaction,
						NULL, 0, req->user_data);

done:
	g_free(req->operands);
	g_free(req);
}

static void browsing_req_destroy(void *data)
{
	struct avctp_browsing_req *req = data;
	struct avctp_pending_req *p = req->p;
	struct avctp *session = p->chan->session;

	if (p->err == 0 || req->func == NULL)
		goto done;

	req->func(session, NULL, 0, req->user_data);

done:
	g_free(req->operands);
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
					req->subunit, req->op,
					req->operands, req->operand_count);
}

static int process_browsing(void *data)
{
	struct avctp_browsing_req *req = data;
	struct avctp_pending_req *p = req->p;

	return avctp_browsing_send(p->chan, p->transaction, AVCTP_COMMAND,
					req->operands, req->operand_count);
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

	for (l = control->processed; l; l = l->next) {
		p = l->data;
		req = p->data;

		if (p->transaction != avctp->transaction)
			continue;

		if (req->func && req->func(control->session, avc->code,
					avc->subunit_type, p->transaction,
					operands, operand_count,
					req->user_data))
			return;

		control->processed = g_slist_remove(control->processed, p);
		pending_destroy(p, NULL);

		return;
	}
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

	for (l = browsing->processed; l; l = l->next) {
		p = l->data;
		req = p->data;

		if (p->transaction != avctp->transaction)
			continue;

		if (req->func && req->func(browsing->session, operands,
						operand_count, req->user_data))
			return;

		browsing->processed = g_slist_remove(browsing->processed, p);
		pending_destroy(p, NULL);

		return;
	}
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

	avctp = (struct avctp_header *) buf;

	if (avctp->packet_type != AVCTP_PACKET_SINGLE)
		goto failed;

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
		packet_size += avrcp_browsing_general_reject(operands);
		goto send;
	}

	packet_size += handler->cb(session, avctp->transaction,
						operands, operand_count,
						handler->user_data);

send:
	if (packet_size != 0) {
		ret = write(sock, buf, packet_size);
		if (ret != packet_size)
			goto failed;
	}

	return TRUE;

failed:
	DBG("AVCTP Browsing: disconnected");
	avctp_set_state(session, AVCTP_STATE_CONNECTED, 0);

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
	int ret, packet_size, operand_count, sock;
	struct avctp_pdu_handler *handler;

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
		packet_size += avrcp_handle_vendor_reject(&code, operands);
		avc->code = code;
		goto done;
	}

	code = avc->code;
	subunit = avc->subunit_type;

	packet_size += handler->cb(session, avctp->transaction, &code,
					&subunit, operands, operand_count,
					handler->user_data);

	avc->code = code;
	avc->subunit_type = subunit;

done:
	ret = write(sock, buf, packet_size);
	if (ret != packet_size)
		goto failed;

	return TRUE;

failed:
	DBG("AVCTP session %p got disconnected", session);
	avctp_set_state(session, AVCTP_STATE_DISCONNECTED, -EIO);
	return FALSE;
}

static int uinput_create(char *name)
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

	send_event(fd, EV_REP, REP_DELAY, 300);

	return fd;
}

static void init_uinput(struct avctp *session)
{
	char address[18], name[248 + 1];

	device_get_name(session->device, name, sizeof(name));
	if (g_str_equal(name, "Nokia CK-20W")) {
		session->key_quirks[AVC_FORWARD] |= QUIRK_NO_RELEASE;
		session->key_quirks[AVC_BACKWARD] |= QUIRK_NO_RELEASE;
		session->key_quirks[AVC_PLAY] |= QUIRK_NO_RELEASE;
		session->key_quirks[AVC_PAUSE] |= QUIRK_NO_RELEASE;
	}

	ba2str(device_get_address(session->device), address);
	session->uinput = uinput_create(address);
	if (session->uinput < 0)
		error("AVRCP: failed to init uinput for %s", address);
	else
		DBG("AVRCP: uinput initialized for %s", address);
}

static struct avctp_channel *avctp_channel_create(struct avctp *session,
							GIOChannel *io,
							GDestroyNotify destroy)
{
	struct avctp_channel *chan;

	chan = g_new0(struct avctp_channel, 1);
	chan->session = session;
	chan->io = g_io_channel_ref(io);
	chan->queue = g_queue_new();
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

static void avctp_connect_browsing_cb(GIOChannel *chan, GError *err,
							gpointer data)
{
	struct avctp *session = data;
	struct avctp_channel *browsing = session->browsing;
	char address[18];
	uint16_t imtu, omtu;
	GError *gerr = NULL;

	if (err) {
		error("Browsing: %s", err->message);
		goto fail;
	}

	bt_io_get(chan, &gerr,
			BT_IO_OPT_DEST, &address,
			BT_IO_OPT_IMTU, &imtu,
			BT_IO_OPT_OMTU, &omtu,
			BT_IO_OPT_INVALID);
	if (gerr) {
		error("%s", gerr->message);
		g_io_channel_shutdown(chan, TRUE, NULL);
		g_io_channel_unref(chan);
		g_error_free(gerr);
		goto fail;
	}

	DBG("AVCTP Browsing: connected to %s", address);

	if (browsing == NULL) {
		browsing = avctp_channel_create(session, chan,
						avctp_destroy_browsing);
		session->browsing = browsing;
	}

	browsing->imtu = imtu;
	browsing->omtu = omtu;
	browsing->buffer = g_malloc0(MAX(imtu, omtu));
	browsing->watch = g_io_add_watch(session->browsing->io,
				G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
				(GIOFunc) session_browsing_cb, session);

	avctp_set_state(session, AVCTP_STATE_BROWSING_CONNECTED, 0);

	/* Process any request that was pending the connection to complete */
	if (browsing->process_id == 0 && !g_queue_is_empty(browsing->queue))
		browsing->process_id = g_idle_add(process_queue, browsing);

	return;

fail:
	avctp_set_state(session, AVCTP_STATE_CONNECTED, 0);

	if (session->browsing) {
		avctp_channel_destroy(session->browsing);
		session->browsing = NULL;
	}
}

static void avctp_connect_cb(GIOChannel *chan, GError *err, gpointer data)
{
	struct avctp *session = data;
	char address[18];
	uint16_t imtu, omtu;
	GError *gerr = NULL;

	if (err) {
		avctp_set_state(session, AVCTP_STATE_DISCONNECTED, -EIO);
		error("%s", err->message);
		return;
	}

	bt_io_get(chan, &gerr,
			BT_IO_OPT_DEST, &address,
			BT_IO_OPT_IMTU, &imtu,
			BT_IO_OPT_IMTU, &omtu,
			BT_IO_OPT_INVALID);
	if (gerr) {
		avctp_set_state(session, AVCTP_STATE_DISCONNECTED, -EIO);
		error("%s", gerr->message);
		g_error_free(gerr);
		return;
	}

	DBG("AVCTP: connected to %s", address);

	if (session->control == NULL)
		session->control = avctp_channel_create(session, chan, NULL);

	session->control->imtu = imtu;
	session->control->omtu = omtu;
	session->control->buffer = g_malloc0(MAX(imtu, omtu));
	session->control->watch = g_io_add_watch(session->control->io,
				G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL,
				(GIOFunc) session_cb, session);

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

	init_uinput(session);

	avctp_set_state(session, AVCTP_STATE_CONNECTED, 0);
}

static void auth_cb(DBusError *derr, void *user_data)
{
	struct avctp *session = user_data;
	GError *err = NULL;

	session->auth_id = 0;

	if (session->control->watch > 0) {
		g_source_remove(session->control->watch);
		session->control->watch = 0;
	}

	if (derr && dbus_error_is_set(derr)) {
		error("Access denied: %s", derr->message);
		avctp_set_state(session, AVCTP_STATE_DISCONNECTED, -EIO);
		return;
	}

	if (!bt_io_accept(session->control->io, avctp_connect_cb, session,
								NULL, &err)) {
		error("bt_io_accept: %s", err->message);
		g_error_free(err);
		avctp_set_state(session, AVCTP_STATE_DISCONNECTED, -EIO);
	}
}

static struct avctp_server *find_server(GSList *list, struct btd_adapter *a)
{
	for (; list; list = list->next) {
		struct avctp_server *server = list->data;

		if (server->adapter == a)
			return server;
	}

	return NULL;
}

static struct avctp *find_session(GSList *list, struct btd_device *device)
{
	for (; list != NULL; list = g_slist_next(list)) {
		struct avctp *s = list->data;

		if (s->device == device)
			return s;
	}

	return NULL;
}

static struct avctp *avctp_get_internal(struct btd_device *device)
{
	struct avctp_server *server;
	struct avctp *session;

	server = find_server(servers, device_get_adapter(device));
	if (server == NULL)
		return NULL;

	session = find_session(server->sessions, device);
	if (session)
		return session;

	session = g_new0(struct avctp, 1);

	session->server = server;
	session->device = btd_device_ref(device);
	session->state = AVCTP_STATE_DISCONNECTED;
	session->uinput = -1;

	server->sessions = g_slist_append(server->sessions, session);

	return session;
}

static void avctp_control_confirm(struct avctp *session, GIOChannel *chan,
						struct btd_device *dev)
{
	const bdaddr_t *src;
	const bdaddr_t *dst;

	if (session->control != NULL) {
		error("Control: Refusing unexpected connect");
		g_io_channel_shutdown(chan, TRUE, NULL);

		/*
		 * Close AVCTP channel if remote tried connect
		 * at the same time
		 * AVRCP SPEC V1.5 4.1.1 Connection Establishment
		 */
		avctp_set_state(session, AVCTP_STATE_DISCONNECTED, -EAGAIN);
		return;
	}

	avctp_set_state(session, AVCTP_STATE_CONNECTING, 0);
	session->control = avctp_channel_create(session, chan, NULL);

	src = btd_adapter_get_address(device_get_adapter(dev));
	dst = device_get_address(dev);

	session->auth_id = btd_request_authorization(src, dst,
							AVRCP_REMOTE_UUID,
							auth_cb, session);
	if (session->auth_id == 0)
		goto drop;

	session->control->watch = g_io_add_watch(chan, G_IO_ERR | G_IO_HUP |
						G_IO_NVAL, session_cb, session);
	return;

drop:
	avctp_set_state(session, AVCTP_STATE_DISCONNECTED, -EIO);
}

static void avctp_browsing_confirm(struct avctp *session, GIOChannel *chan,
						struct btd_device *dev)
{
	GError *err = NULL;

	if (session->control == NULL || session->browsing != NULL) {
		error("Browsing: Refusing unexpected connect");
		g_io_channel_shutdown(chan, TRUE, NULL);
		return;
	}

	if (bt_io_accept(chan, avctp_connect_browsing_cb, session, NULL,
								&err)) {
		avctp_set_state(session, AVCTP_STATE_BROWSING_CONNECTING, 0);
		return;
	}

	error("Browsing: %s", err->message);
	g_error_free(err);

	return;
}

static void avctp_confirm_cb(GIOChannel *chan, gpointer data)
{
	struct avctp *session;
	char address[18];
	bdaddr_t src, dst;
	GError *err = NULL;
	uint16_t psm;
	struct btd_device *device;

	bt_io_get(chan, &err,
			BT_IO_OPT_SOURCE_BDADDR, &src,
			BT_IO_OPT_DEST_BDADDR, &dst,
			BT_IO_OPT_DEST, address,
			BT_IO_OPT_PSM, &psm,
			BT_IO_OPT_INVALID);
	if (err) {
		error("%s", err->message);
		g_error_free(err);
		g_io_channel_shutdown(chan, TRUE, NULL);
		return;
	}

	DBG("AVCTP: incoming connect from %s", address);

	device = btd_adapter_find_device(adapter_find(&src), &dst,
								BDADDR_BREDR);
	if (!device)
		return;

	session = avctp_get_internal(device);
	if (session == NULL)
		return;

	if (btd_device_get_service(device, AVRCP_REMOTE_UUID) == NULL)
		btd_device_add_uuid(device, AVRCP_REMOTE_UUID);

	if (btd_device_get_service(device, AVRCP_TARGET_UUID) == NULL)
		btd_device_add_uuid(device, AVRCP_TARGET_UUID);

	switch (psm) {
	case AVCTP_CONTROL_PSM:
		avctp_control_confirm(session, chan, device);
		break;
	case AVCTP_BROWSING_PSM:
		avctp_browsing_confirm(session, chan, device);
		break;
	}

	return;
}

static GIOChannel *avctp_server_socket(const bdaddr_t *src, gboolean master,
						uint8_t mode, uint16_t psm)
{
	GError *err = NULL;
	GIOChannel *io;

	io = bt_io_listen(NULL, avctp_confirm_cb, NULL,
				NULL, &err,
				BT_IO_OPT_SOURCE_BDADDR, src,
				BT_IO_OPT_PSM, psm,
				BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_MEDIUM,
				BT_IO_OPT_MASTER, master,
				BT_IO_OPT_MODE, mode,
				BT_IO_OPT_INVALID);
	if (!io) {
		error("%s", err->message);
		g_error_free(err);
	}

	return io;
}

int avctp_register(struct btd_adapter *adapter, gboolean master)
{
	struct avctp_server *server;
	const bdaddr_t *src = btd_adapter_get_address(adapter);

	server = g_new0(struct avctp_server, 1);

	server->control_io = avctp_server_socket(src, master, L2CAP_MODE_BASIC,
							AVCTP_CONTROL_PSM);
	if (!server->control_io) {
		g_free(server);
		return -1;
	}
	server->browsing_io = avctp_server_socket(src, master, L2CAP_MODE_ERTM,
							AVCTP_BROWSING_PSM);
	if (!server->browsing_io) {
		if (server->control_io) {
			g_io_channel_shutdown(server->control_io, TRUE, NULL);
			g_io_channel_unref(server->control_io);
			server->control_io = NULL;
		}
		g_free(server);
		return -1;
	}

	server->adapter = btd_adapter_ref(adapter);

	servers = g_slist_append(servers, server);

	return 0;
}

void avctp_unregister(struct btd_adapter *adapter)
{
	struct avctp_server *server;

	server = find_server(servers, adapter);
	if (!server)
		return;

	while (server->sessions)
		avctp_disconnected(server->sessions->data);

	servers = g_slist_remove(servers, server);

	g_io_channel_shutdown(server->browsing_io, TRUE, NULL);
	g_io_channel_unref(server->browsing_io);
	server->browsing_io = NULL;

	g_io_channel_shutdown(server->control_io, TRUE, NULL);
	g_io_channel_unref(server->control_io);
	btd_adapter_unref(server->adapter);
	g_free(server);
}

static struct avctp_pending_req *pending_create(struct avctp_channel *chan,
						avctp_process_cb process,
						void *data,
						GDestroyNotify destroy)
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

static int avctp_send_req(struct avctp *session, uint8_t code,
				uint8_t subunit, uint8_t opcode,
				uint8_t *operands, size_t operand_count,
				avctp_rsp_cb func, void *user_data)
{
	struct avctp_channel *control = session->control;
	struct avctp_pending_req *p;
	struct avctp_control_req *req;

	if (control == NULL)
		return -ENOTCONN;

	req = g_new0(struct avctp_control_req, 1);
	req->code = code;
	req->subunit = subunit;
	req->op = opcode;
	req->func = func;
	req->operands = g_memdup(operands, operand_count);
	req->operand_count = operand_count;
	req->user_data = user_data;

	p = pending_create(control, process_control, req, control_req_destroy);

	req->p = p;

	g_queue_push_tail(control->queue, p);

	if (control->process_id == 0)
		control->process_id = g_idle_add(process_queue, control);

	return 0;
}

int avctp_send_browsing_req(struct avctp *session,
				uint8_t *operands, size_t operand_count,
				avctp_browsing_rsp_cb func, void *user_data)
{
	struct avctp_channel *browsing = session->browsing;
	struct avctp_pending_req *p;
	struct avctp_browsing_req *req;

	if (browsing == NULL)
		return -ENOTCONN;

	req = g_new0(struct avctp_browsing_req, 1);
	req->func = func;
	req->operands = g_memdup(operands, operand_count);
	req->operand_count = operand_count;
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

static const char *op2str(uint8_t op)
{
	int i;

	for (i = 0; key_map[i].name != NULL; i++) {
		if ((op & 0x7F) == key_map[i].avc)
			return key_map[i].name;
	}

	return "UNKNOWN";
}

static int avctp_passthrough_press(struct avctp *session, uint8_t op)
{
	uint8_t operands[2];

	DBG("%s", op2str(op));

	/* Button pressed */
	operands[0] = op & 0x7f;
	operands[1] = 0;

	return avctp_send_req(session, AVC_CTYPE_CONTROL,
				AVC_SUBUNIT_PANEL, AVC_OP_PASSTHROUGH,
				operands, sizeof(operands),
				avctp_passthrough_rsp, NULL);
}

static int avctp_passthrough_release(struct avctp *session, uint8_t op)
{
	uint8_t operands[2];

	DBG("%s", op2str(op));

	/* Button released */
	operands[0] = op | 0x80;
	operands[1] = 0;

	return avctp_send_req(session, AVC_CTYPE_CONTROL,
				AVC_SUBUNIT_PANEL, AVC_OP_PASSTHROUGH,
				operands, sizeof(operands),
				NULL, NULL);
}

static gboolean repeat_timeout(gpointer user_data)
{
	struct avctp *session = user_data;

	avctp_passthrough_release(session, session->key.op);
	avctp_passthrough_press(session, session->key.op);

	return TRUE;
}

static void release_pressed(struct avctp *session)
{
	avctp_passthrough_release(session, session->key.op);

	if (session->key.timer > 0)
		g_source_remove(session->key.timer);

	session->key.timer = 0;
}

static bool set_pressed(struct avctp *session, uint8_t op)
{
	if (session->key.timer > 0) {
		if (session->key.op == op)
			return TRUE;
		release_pressed(session);
	}

	if (op != AVC_FAST_FORWARD && op != AVC_REWIND)
		return FALSE;

	session->key.op = op;
	session->key.timer = g_timeout_add_seconds(AVC_PRESS_TIMEOUT,
							repeat_timeout,
							session);

	return TRUE;
}

static gboolean avctp_passthrough_rsp(struct avctp *session, uint8_t code,
					uint8_t subunit, uint8_t transaction,
					uint8_t *operands, size_t operand_count,
					void *user_data)
{
	if (code != AVC_CTYPE_ACCEPTED)
		return FALSE;

	if (set_pressed(session, operands[0]))
		return FALSE;

	avctp_passthrough_release(session, operands[0]);

	return FALSE;
}

int avctp_send_passthrough(struct avctp *session, uint8_t op)
{
	/* Auto release if key pressed */
	if (session->key.timer > 0)
		release_pressed(session);

	return avctp_passthrough_press(session, op);
}

int avctp_send_vendordep(struct avctp *session, uint8_t transaction,
				uint8_t code, uint8_t subunit,
				uint8_t *operands, size_t operand_count)
{
	struct avctp_channel *control = session->control;

	if (control == NULL)
		return -ENOTCONN;

	return avctp_send(control, transaction, AVCTP_RESPONSE, code, subunit,
					AVC_OP_VENDORDEP, operands, operand_count);
}

int avctp_send_vendordep_req(struct avctp *session, uint8_t code,
					uint8_t subunit, uint8_t *operands,
					size_t operand_count,
					avctp_rsp_cb func, void *user_data)
{
	return avctp_send_req(session, code, subunit, AVC_OP_VENDORDEP,
						operands, operand_count,
						func, user_data);
}

unsigned int avctp_add_state_cb(struct btd_device *dev, avctp_state_cb cb,
							void *user_data)
{
	struct avctp_state_callback *state_cb;
	static unsigned int id = 0;

	state_cb = g_new(struct avctp_state_callback, 1);
	state_cb->cb = cb;
	state_cb->dev = dev;
	state_cb->id = ++id;
	state_cb->user_data = user_data;

	callbacks = g_slist_append(callbacks, state_cb);

	return state_cb->id;
}

gboolean avctp_remove_state_cb(unsigned int id)
{
	GSList *l;

	for (l = callbacks; l != NULL; l = l->next) {
		struct avctp_state_callback *cb = l->data;
		if (cb && cb->id == id) {
			callbacks = g_slist_remove(callbacks, cb);
			g_free(cb);
			return TRUE;
		}
	}

	return FALSE;
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

bool avctp_unregister_passthrough_handler(unsigned int id)
{
	GSList *l;

	for (l = servers; l; l = l->next) {
		struct avctp_server *server = l->data;
		GSList *s;

		for (s = server->sessions; s; s = s->next) {
			struct avctp *session = s->data;

			if (session->handler == NULL)
				continue;

			if (session->handler->id == id) {
				g_free(session->handler);
				session->handler = NULL;
				return true;
			}
		}
	}

	return false;
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
						GDestroyNotify destroy)
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

gboolean avctp_unregister_pdu_handler(unsigned int id)
{
	GSList *l;

	for (l = servers; l; l = l->next) {
		struct avctp_server *server = l->data;
		GSList *s;

		for (s = server->sessions; s; s = s->next) {
			struct avctp *session = s->data;
			struct avctp_channel *control = session->control;
			GSList *h;

			if (control == NULL)
				continue;

			for (h = control->handlers; h; h = h->next) {
				struct avctp_pdu_handler *handler = h->data;

				if (handler->id != id)
					continue;

				control->handlers = g_slist_remove(
							control->handlers,
							handler);
				g_free(handler);
				return TRUE;
			}
		}
	}

	return FALSE;
}

gboolean avctp_unregister_browsing_pdu_handler(unsigned int id)
{
	GSList *l;

	for (l = servers; l; l = l->next) {
		struct avctp_server *server = l->data;
		GSList *s;

		for (s = server->sessions; s; s = s->next) {
			struct avctp *session = s->data;
			struct avctp_channel *browsing = session->browsing;
			GSList *h;

			if (browsing == NULL)
				continue;

			for (h = browsing->handlers; h; h = h->next) {
				struct avctp_browsing_pdu_handler *handler =
								h->data;

				if (handler->id != id)
					continue;

				browsing->handlers = g_slist_remove(
							browsing->handlers,
							handler);
				g_free(handler);
				return TRUE;
			}
		}
	}

	return FALSE;
}

struct avctp *avctp_connect(struct btd_device *device)
{
	struct avctp *session;
	GError *err = NULL;
	GIOChannel *io;
	const bdaddr_t *src;

	session = avctp_get_internal(device);
	if (!session)
		return NULL;

	if (session->state > AVCTP_STATE_DISCONNECTED)
		return session;

	avctp_set_state(session, AVCTP_STATE_CONNECTING, 0);

	src = btd_adapter_get_address(session->server->adapter);

	io = bt_io_connect(avctp_connect_cb, session, NULL, &err,
				BT_IO_OPT_SOURCE_BDADDR, src,
				BT_IO_OPT_DEST_BDADDR,
				device_get_address(session->device),
				BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_MEDIUM,
				BT_IO_OPT_PSM, AVCTP_CONTROL_PSM,
				BT_IO_OPT_INVALID);
	if (err) {
		avctp_set_state(session, AVCTP_STATE_DISCONNECTED, -EIO);
		error("%s", err->message);
		g_error_free(err);
		return NULL;
	}

	session->control = avctp_channel_create(session, io, NULL);
	session->initiator = true;
	g_io_channel_unref(io);

	return session;
}

int avctp_connect_browsing(struct avctp *session)
{
	const bdaddr_t *src;
	GError *err = NULL;
	GIOChannel *io;

	if (session->state != AVCTP_STATE_CONNECTED)
		return -ENOTCONN;

	if (session->browsing != NULL)
		return 0;

	avctp_set_state(session, AVCTP_STATE_BROWSING_CONNECTING, 0);

	src = btd_adapter_get_address(session->server->adapter);

	io = bt_io_connect(avctp_connect_browsing_cb, session, NULL, &err,
				BT_IO_OPT_SOURCE_BDADDR, src,
				BT_IO_OPT_DEST_BDADDR,
				device_get_address(session->device),
				BT_IO_OPT_SEC_LEVEL, BT_IO_SEC_MEDIUM,
				BT_IO_OPT_PSM, AVCTP_BROWSING_PSM,
				BT_IO_OPT_MODE, L2CAP_MODE_ERTM,
				BT_IO_OPT_INVALID);
	if (err) {
		error("%s", err->message);
		g_error_free(err);
		return -EIO;
	}

	session->browsing = avctp_channel_create(session, io,
						avctp_destroy_browsing);
	g_io_channel_unref(io);

	return 0;
}

void avctp_disconnect(struct avctp *session)
{
	if (session->state == AVCTP_STATE_DISCONNECTED)
		return;

	avctp_set_state(session, AVCTP_STATE_DISCONNECTED, -EIO);
}

struct avctp *avctp_get(struct btd_device *device)
{
	return avctp_get_internal(device);
}

bool avctp_is_initiator(struct avctp *session)
{
	return session->initiator;
}

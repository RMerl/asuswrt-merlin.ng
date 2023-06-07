// SPDX-License-Identifier: LGPL-2.1-or-later
/*
 *
 *  BlueZ - Bluetooth protocol stack for Linux
 *
 *  Copyright (C) 2014  Google Inc.
 *
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include "src/shared/io.h"
#include "src/shared/queue.h"
#include "src/shared/util.h"
#include "src/shared/timeout.h"
#include "lib/bluetooth.h"
#include "lib/l2cap.h"
#include "lib/uuid.h"
#include "src/shared/att.h"
#include "src/shared/crypto.h"

#define ATT_MIN_PDU_LEN			1  /* At least 1 byte for the opcode. */
#define ATT_OP_CMD_MASK			0x40
#define ATT_OP_SIGNED_MASK		0x80
#define ATT_TIMEOUT_INTERVAL		30000  /* 30000 ms */

/* Length of signature in write signed packet */
#define BT_ATT_SIGNATURE_LEN		12

struct att_send_op;

struct bt_att_chan {
	struct bt_att *att;
	int fd;
	struct io *io;
	uint8_t type;
	int sec_level;			/* Only used for non-L2CAP */

	struct queue *queue;		/* Channel dedicated queue */

	struct att_send_op *pending_req;
	struct att_send_op *pending_ind;
	bool writer_active;

	bool in_req;			/* There's a pending incoming request */

	uint8_t *buf;
	uint16_t mtu;
};

struct bt_att {
	int ref_count;
	bool close_on_unref;
	struct queue *chans;
	uint8_t enc_size;
	uint16_t mtu;			/* Biggest possible MTU */

	struct queue *notify_list;	/* List of registered callbacks */
	struct queue *disconn_list;	/* List of disconnect handlers */

	unsigned int next_send_id;	/* IDs for "send" ops */
	unsigned int next_reg_id;	/* IDs for registered callbacks */

	struct queue *req_queue;	/* Queued ATT protocol requests */
	struct queue *ind_queue;	/* Queued ATT protocol indications */
	struct queue *write_queue;	/* Queue of PDUs ready to send */
	bool in_disc;			/* Cleanup queues on disconnect_cb */

	bt_att_timeout_func_t timeout_callback;
	bt_att_destroy_func_t timeout_destroy;
	void *timeout_data;

	uint8_t debug_level;
	bt_att_debug_func_t debug_callback;
	bt_att_destroy_func_t debug_destroy;
	void *debug_data;

	struct bt_crypto *crypto;

	struct sign_info *local_sign;
	struct sign_info *remote_sign;
};

struct sign_info {
	uint8_t key[16];
	bt_att_counter_func_t counter;
	void *user_data;
};

enum att_op_type {
	ATT_OP_TYPE_REQ,
	ATT_OP_TYPE_RSP,
	ATT_OP_TYPE_CMD,
	ATT_OP_TYPE_IND,
	ATT_OP_TYPE_NFY,
	ATT_OP_TYPE_CONF,
	ATT_OP_TYPE_UNKNOWN,
};

static const struct {
	uint8_t opcode;
	enum att_op_type type;
} att_opcode_type_table[] = {
	{ BT_ATT_OP_ERROR_RSP,			ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_MTU_REQ,			ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_MTU_RSP,			ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_FIND_INFO_REQ,		ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_FIND_INFO_RSP,		ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_FIND_BY_TYPE_REQ,		ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_FIND_BY_TYPE_RSP,		ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_READ_BY_TYPE_REQ,		ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_READ_BY_TYPE_RSP,		ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_READ_REQ,			ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_READ_RSP,			ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_READ_BLOB_REQ,		ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_READ_BLOB_RSP,		ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_READ_MULT_REQ,		ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_READ_MULT_RSP,		ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_READ_BY_GRP_TYPE_REQ,	ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_READ_BY_GRP_TYPE_RSP,	ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_WRITE_REQ,			ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_WRITE_RSP,			ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_WRITE_CMD,			ATT_OP_TYPE_CMD },
	{ BT_ATT_OP_SIGNED_WRITE_CMD,		ATT_OP_TYPE_CMD },
	{ BT_ATT_OP_PREP_WRITE_REQ,		ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_PREP_WRITE_RSP,		ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_EXEC_WRITE_REQ,		ATT_OP_TYPE_REQ },
	{ BT_ATT_OP_EXEC_WRITE_RSP,		ATT_OP_TYPE_RSP },
	{ BT_ATT_OP_HANDLE_NFY,			ATT_OP_TYPE_NFY },
	{ BT_ATT_OP_HANDLE_NFY_MULT,		ATT_OP_TYPE_NFY },
	{ BT_ATT_OP_HANDLE_IND,			ATT_OP_TYPE_IND },
	{ BT_ATT_OP_HANDLE_CONF,		ATT_OP_TYPE_CONF },
	{ }
};

static enum att_op_type get_op_type(uint8_t opcode)
{
	int i;

	for (i = 0; att_opcode_type_table[i].opcode; i++) {
		if (att_opcode_type_table[i].opcode == opcode)
			return att_opcode_type_table[i].type;
	}

	if (opcode & ATT_OP_CMD_MASK)
		return ATT_OP_TYPE_CMD;

	return ATT_OP_TYPE_UNKNOWN;
}

static const struct {
	uint8_t req_opcode;
	uint8_t rsp_opcode;
} att_req_rsp_mapping_table[] = {
	{ BT_ATT_OP_MTU_REQ,			BT_ATT_OP_MTU_RSP },
	{ BT_ATT_OP_FIND_INFO_REQ,		BT_ATT_OP_FIND_INFO_RSP},
	{ BT_ATT_OP_FIND_BY_TYPE_REQ,		BT_ATT_OP_FIND_BY_TYPE_RSP },
	{ BT_ATT_OP_READ_BY_TYPE_REQ,		BT_ATT_OP_READ_BY_TYPE_RSP },
	{ BT_ATT_OP_READ_REQ,			BT_ATT_OP_READ_RSP },
	{ BT_ATT_OP_READ_BLOB_REQ,		BT_ATT_OP_READ_BLOB_RSP },
	{ BT_ATT_OP_READ_MULT_REQ,		BT_ATT_OP_READ_MULT_RSP },
	{ BT_ATT_OP_READ_BY_GRP_TYPE_REQ,	BT_ATT_OP_READ_BY_GRP_TYPE_RSP },
	{ BT_ATT_OP_WRITE_REQ,			BT_ATT_OP_WRITE_RSP },
	{ BT_ATT_OP_PREP_WRITE_REQ,		BT_ATT_OP_PREP_WRITE_RSP },
	{ BT_ATT_OP_EXEC_WRITE_REQ,		BT_ATT_OP_EXEC_WRITE_RSP },
	{ }
};

static uint8_t get_req_opcode(uint8_t rsp_opcode)
{
	int i;

	for (i = 0; att_req_rsp_mapping_table[i].rsp_opcode; i++) {
		if (att_req_rsp_mapping_table[i].rsp_opcode == rsp_opcode)
			return att_req_rsp_mapping_table[i].req_opcode;
	}

	return 0;
}

struct att_send_op {
	unsigned int id;
	unsigned int timeout_id;
	enum att_op_type type;
	uint8_t opcode;
	void *pdu;
	uint16_t len;
	bt_att_response_func_t callback;
	bt_att_destroy_func_t destroy;
	void *user_data;
};

static void destroy_att_send_op(void *data)
{
	struct att_send_op *op = data;

	if (op->timeout_id)
		timeout_remove(op->timeout_id);

	if (op->destroy)
		op->destroy(op->user_data);

	free(op->pdu);
	free(op);
}

static void cancel_att_send_op(void *data)
{
	struct att_send_op *op = data;

	if (op->destroy)
		op->destroy(op->user_data);

	op->user_data = NULL;
	op->callback = NULL;
	op->destroy = NULL;
}

struct att_notify {
	unsigned int id;
	uint16_t opcode;
	bt_att_notify_func_t callback;
	bt_att_destroy_func_t destroy;
	void *user_data;
};

static void destroy_att_notify(void *data)
{
	struct att_notify *notify = data;

	if (notify->destroy)
		notify->destroy(notify->user_data);

	free(notify);
}

static bool match_notify_id(const void *a, const void *b)
{
	const struct att_notify *notify = a;
	unsigned int id = PTR_TO_UINT(b);

	return notify->id == id;
}

struct att_disconn {
	unsigned int id;
	bool removed;
	bt_att_disconnect_func_t callback;
	bt_att_destroy_func_t destroy;
	void *user_data;
};

static void destroy_att_disconn(void *data)
{
	struct att_disconn *disconn = data;

	if (disconn->destroy)
		disconn->destroy(disconn->user_data);

	free(disconn);
}

static bool match_disconn_id(const void *a, const void *b)
{
	const struct att_disconn *disconn = a;
	unsigned int id = PTR_TO_UINT(b);

	return disconn->id == id;
}

static void att_log(struct bt_att *att, uint8_t level, const char *format,
								...)
{
	va_list va;

	if (att->debug_level < level)
		return;

	va_start(va, format);
	util_debug_va(att->debug_callback, att->debug_data, format, va);
	va_end(va);
}

#define att_debug(_att, _format, _arg...) \
	att_log(_att, BT_ATT_DEBUG, _format, ## _arg)

#define att_verbose(_att, _format, _arg...) \
	att_log(_att, BT_ATT_DEBUG_VERBOSE, _format, ## _arg)

static void att_hexdump(struct bt_att *att, char dir, const void *data,
							size_t len)
{
	if (att->debug_level < 2)
		return;

	util_hexdump(dir, data, len, att->debug_callback, att->debug_data);
}

static bool encode_pdu(struct bt_att *att, struct att_send_op *op,
					const void *pdu, uint16_t length)
{
	uint16_t pdu_len = 1;
	struct sign_info *sign = att->local_sign;
	uint32_t sign_cnt;

	if (sign && (op->opcode & ATT_OP_SIGNED_MASK))
		pdu_len += BT_ATT_SIGNATURE_LEN;

	if (length && pdu)
		pdu_len += length;

	if (pdu_len > att->mtu)
		return false;

	op->len = pdu_len;
	op->pdu = malloc(op->len);
	if (!op->pdu)
		return false;

	((uint8_t *) op->pdu)[0] = op->opcode;
	if (pdu_len > 1)
		memcpy(op->pdu + 1, pdu, length);

	if (!sign || !(op->opcode & ATT_OP_SIGNED_MASK) || !att->crypto)
		return true;

	if (!sign->counter(&sign_cnt, sign->user_data))
		goto fail;

	if ((bt_crypto_sign_att(att->crypto, sign->key, op->pdu, 1 + length,
				sign_cnt, &((uint8_t *) op->pdu)[1 + length])))
		return true;

	att_debug(att, "ATT unable to generate signature");

fail:
	free(op->pdu);
	return false;
}

static struct att_send_op *create_att_send_op(struct bt_att *att,
						uint8_t opcode,
						const void *pdu,
						uint16_t length,
						bt_att_response_func_t callback,
						void *user_data,
						bt_att_destroy_func_t destroy)
{
	struct att_send_op *op;
	enum att_op_type type;

	if (length && !pdu)
		return NULL;

	type = get_op_type(opcode);
	if (type == ATT_OP_TYPE_UNKNOWN)
		return NULL;

	/* If the opcode corresponds to an operation type that does not elicit a
	 * response from the remote end, then no callback should have been
	 * provided, since it will never be called.
	 */
	if (callback && type != ATT_OP_TYPE_REQ && type != ATT_OP_TYPE_IND)
		return NULL;

	/* Similarly, if the operation does elicit a response then a callback
	 * must be provided.
	 */
	if (!callback && (type == ATT_OP_TYPE_REQ || type == ATT_OP_TYPE_IND))
		return NULL;

	op = new0(struct att_send_op, 1);
	op->type = type;
	op->opcode = opcode;
	op->callback = callback;
	op->destroy = destroy;
	op->user_data = user_data;

	if (!encode_pdu(att, op, pdu, length)) {
		free(op);
		return NULL;
	}

	return op;
}

static struct att_send_op *pick_next_send_op(struct bt_att_chan *chan)
{
	struct bt_att *att = chan->att;
	struct att_send_op *op;

	/* Check if there is anything queued on the channel */
	op = queue_pop_head(chan->queue);
	if (op)
		return op;

	/* See if any operations are already in the write queue */
	op = queue_peek_head(att->write_queue);
	if (op && op->len <= chan->mtu)
		return queue_pop_head(att->write_queue);

	/* If there is no pending request, pick an operation from the
	 * request queue.
	 */
	if (!chan->pending_req) {
		op = queue_peek_head(att->req_queue);
		if (op && op->len <= chan->mtu)
			return queue_pop_head(att->req_queue);
	}

	/* There is either a request pending or no requests queued. If there is
	 * no pending indication, pick an operation from the indication queue.
	 */
	if (!chan->pending_ind) {
		op = queue_peek_head(att->ind_queue);
		if (op && op->len <= chan->mtu)
			return queue_pop_head(att->ind_queue);
	}

	return NULL;
}

static void disc_att_send_op(void *data)
{
	struct att_send_op *op = data;

	if (op->callback)
		op->callback(BT_ATT_OP_ERROR_RSP, NULL, 0, op->user_data);

	destroy_att_send_op(op);
}

struct timeout_data {
	struct bt_att_chan *chan;
	unsigned int id;
};

static bool timeout_cb(void *user_data)
{
	struct timeout_data *timeout = user_data;
	struct bt_att_chan *chan = timeout->chan;
	struct bt_att *att = chan->att;
	struct att_send_op *op = NULL;

	if (chan->pending_req && chan->pending_req->id == timeout->id) {
		op = chan->pending_req;
		chan->pending_req = NULL;
	} else if (chan->pending_ind && chan->pending_ind->id == timeout->id) {
		op = chan->pending_ind;
		chan->pending_ind = NULL;
	}

	if (!op)
		return false;

	att_debug(att, "(chan %p) Operation timed out: 0x%02x", chan,
						op->opcode);

	if (att->timeout_callback)
		att->timeout_callback(op->id, op->opcode, att->timeout_data);

	op->timeout_id = 0;
	disc_att_send_op(op);

	/*
	 * Directly terminate the connection as required by the ATT protocol.
	 * This should trigger an io disconnect event which will clean up the
	 * io and notify the upper layer.
	 */
	io_shutdown(chan->io);

	return false;
}

static void write_watch_destroy(void *user_data)
{
	struct bt_att_chan *chan = user_data;

	chan->writer_active = false;
}

static ssize_t bt_att_chan_write(struct bt_att_chan *chan, uint8_t opcode,
					const void *pdu, uint16_t len)
{
	struct bt_att *att = chan->att;
	ssize_t ret;
	struct iovec iov;

	iov.iov_base = (void *) pdu;
	iov.iov_len = len;

	att_verbose(att, "(chan %p) ATT op 0x%02x", chan, opcode);

	ret = io_send(chan->io, &iov, 1);
	if (ret < 0) {
		att_debug(att, "(chan %p) write failed: %s", chan,
						strerror(-ret));
		return ret;
	}

	if (att->debug_level)
		util_hexdump('<', pdu, ret, att->debug_callback,
						att->debug_data);

	return ret;
}

static bool can_write_data(struct io *io, void *user_data)
{
	struct bt_att_chan *chan = user_data;
	struct att_send_op *op;
	struct timeout_data *timeout;

	op = pick_next_send_op(chan);
	if (!op)
		return false;

	if (!bt_att_chan_write(chan, op->opcode, op->pdu, op->len)) {
		if (op->callback)
			op->callback(BT_ATT_OP_ERROR_RSP, NULL, 0,
							op->user_data);
		destroy_att_send_op(op);
		return true;
	}

	/* Based on the operation type, set either the pending request or the
	 * pending indication. If it came from the write queue, then there is
	 * no need to keep it around.
	 */
	switch (op->type) {
	case ATT_OP_TYPE_REQ:
		chan->pending_req = op;
		break;
	case ATT_OP_TYPE_IND:
		chan->pending_ind = op;
		break;
	case ATT_OP_TYPE_RSP:
		/* Set in_req to false to indicate that no request is pending */
		chan->in_req = false;
		/* fall through */
	case ATT_OP_TYPE_CMD:
	case ATT_OP_TYPE_NFY:
	case ATT_OP_TYPE_CONF:
	case ATT_OP_TYPE_UNKNOWN:
	default:
		destroy_att_send_op(op);
		return true;
	}

	timeout = new0(struct timeout_data, 1);
	timeout->chan = chan;
	timeout->id = op->id;
	op->timeout_id = timeout_add(ATT_TIMEOUT_INTERVAL, timeout_cb,
								timeout, free);

	/* Return true as there may be more operations ready to write. */
	return true;
}

static void wakeup_chan_writer(void *data, void *user_data)
{
	struct bt_att_chan *chan = data;
	struct bt_att *att = chan->att;

	if (chan->writer_active)
		return;

	/* Set the write handler only if there is anything that can be sent
	 * at all.
	 */
	if (queue_isempty(chan->queue) && queue_isempty(att->write_queue)) {
		if ((chan->pending_req || queue_isempty(att->req_queue)) &&
			(chan->pending_ind || queue_isempty(att->ind_queue)))
			return;
	}

	if (!io_set_write_handler(chan->io, can_write_data, chan,
							write_watch_destroy))
		return;

	chan->writer_active = true;
}

static void wakeup_writer(struct bt_att *att)
{
	queue_foreach(att->chans, wakeup_chan_writer, NULL);
}

static void disconn_handler(void *data, void *user_data)
{
	struct att_disconn *disconn = data;
	int err = PTR_TO_INT(user_data);

	if (disconn->removed)
		return;

	if (disconn->callback)
		disconn->callback(err, disconn->user_data);
}

static void bt_att_chan_free(void *data)
{
	struct bt_att_chan *chan = data;

	if (chan->pending_req)
		destroy_att_send_op(chan->pending_req);

	if (chan->pending_ind)
		destroy_att_send_op(chan->pending_ind);

	queue_destroy(chan->queue, destroy_att_send_op);

	io_destroy(chan->io);

	free(chan->buf);
	free(chan);
}

static bool disconnect_cb(struct io *io, void *user_data)
{
	struct bt_att_chan *chan = user_data;
	struct bt_att *att = chan->att;
	int err;
	socklen_t len;

	len = sizeof(err);

	if (getsockopt(chan->fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0) {
		att_debug(att, "(chan %p) Failed to obtain disconnect "
				"error: %s", chan, strerror(errno));
		err = 0;
	}

	att_debug(att, "Channel %p disconnected: %s", chan, strerror(err));

	/* Dettach channel */
	queue_remove(att->chans, chan);

	if (chan->pending_req) {
		disc_att_send_op(chan->pending_req);
		chan->pending_req = NULL;
	}

	if (chan->pending_ind) {
		disc_att_send_op(chan->pending_ind);
		chan->pending_ind = NULL;
	}

	bt_att_chan_free(chan);

	/* Don't run disconnect callback if there are channels left */
	if (!queue_isempty(att->chans))
		return false;

	bt_att_ref(att);

	att->in_disc = true;

	/* Notify request callbacks */
	queue_remove_all(att->req_queue, NULL, NULL, disc_att_send_op);
	queue_remove_all(att->ind_queue, NULL, NULL, disc_att_send_op);
	queue_remove_all(att->write_queue, NULL, NULL, disc_att_send_op);

	att->in_disc = false;

	queue_foreach(att->disconn_list, disconn_handler, INT_TO_PTR(err));

	bt_att_unregister_all(att);
	bt_att_unref(att);

	return false;
}

static int bt_att_chan_get_security(struct bt_att_chan *chan)
{
	struct bt_security sec;
	socklen_t len;

	if (chan->type == BT_ATT_LOCAL)
		return chan->sec_level;

	memset(&sec, 0, sizeof(sec));
	len = sizeof(sec);
	if (getsockopt(chan->fd, SOL_BLUETOOTH, BT_SECURITY, &sec, &len) < 0)
		return -EIO;

	return sec.level;
}

static bool bt_att_chan_set_security(struct bt_att_chan *chan, int level)
{
	struct bt_security sec;

	if (chan->type == BT_ATT_LOCAL) {
		chan->sec_level = level;
		return true;
	}

	memset(&sec, 0, sizeof(sec));
	sec.level = level;

	if (setsockopt(chan->fd, SOL_BLUETOOTH, BT_SECURITY, &sec,
							sizeof(sec)) < 0)
		return false;

	return true;
}

static bool change_security(struct bt_att_chan *chan, uint8_t ecode)
{
	int security;

	if (chan->sec_level != BT_ATT_SECURITY_AUTO)
		return false;

	security = bt_att_chan_get_security(chan);

	if (ecode == BT_ATT_ERROR_INSUFFICIENT_ENCRYPTION &&
					security < BT_ATT_SECURITY_MEDIUM) {
		security = BT_ATT_SECURITY_MEDIUM;
	} else if (ecode == BT_ATT_ERROR_AUTHENTICATION) {
		if (security < BT_ATT_SECURITY_MEDIUM)
			security = BT_ATT_SECURITY_MEDIUM;
		else if (security < BT_ATT_SECURITY_HIGH)
			security = BT_ATT_SECURITY_HIGH;
		else if (security < BT_ATT_SECURITY_FIPS)
			security = BT_ATT_SECURITY_FIPS;
		else
			return false;
	} else {
		return false;
	}

	return bt_att_chan_set_security(chan, security);
}

static bool handle_error_rsp(struct bt_att_chan *chan, uint8_t *pdu,
					ssize_t pdu_len, uint8_t *opcode)
{
	struct bt_att *att = chan->att;
	const struct bt_att_pdu_error_rsp *rsp;
	struct att_send_op *op = chan->pending_req;

	if (pdu_len != sizeof(*rsp)) {
		*opcode = 0;
		return false;
	}

	rsp = (void *) pdu;

	*opcode = rsp->opcode;

	/* Attempt to change security */
	if (!change_security(chan, rsp->ecode))
		return false;

	/* Remove timeout_id if outstanding */
	if (op->timeout_id) {
		timeout_remove(op->timeout_id);
		op->timeout_id = 0;
	}

	att_debug(att, "(chan %p) Retrying operation %p", chan, op);

	chan->pending_req = NULL;

	/* Push operation back to request queue */
	return queue_push_head(att->req_queue, op);
}

static void handle_rsp(struct bt_att_chan *chan, uint8_t opcode, uint8_t *pdu,
								ssize_t pdu_len)
{
	struct bt_att *att = chan->att;
	struct att_send_op *op = chan->pending_req;
	uint8_t req_opcode;
	uint8_t rsp_opcode;
	uint8_t *rsp_pdu = NULL;
	uint16_t rsp_pdu_len = 0;

	/*
	 * If no request is pending, then the response is unexpected. Disconnect
	 * the bearer.
	 */
	if (!op) {
		att_debug(att, "(chan %p) Received unexpected ATT response",
								chan);
		io_shutdown(chan->io);
		return;
	}

	/*
	 * If the received response doesn't match the pending request, or if
	 * the request is malformed, end the current request with failure.
	 */
	if (opcode == BT_ATT_OP_ERROR_RSP) {
		/* Return if error response cause a retry */
		if (handle_error_rsp(chan, pdu, pdu_len, &req_opcode)) {
			wakeup_chan_writer(chan, NULL);
			return;
		}
	} else if (!(req_opcode = get_req_opcode(opcode)))
		goto fail;

	if (req_opcode != op->opcode)
		goto fail;

	rsp_opcode = opcode;

	if (pdu_len > 0) {
		rsp_pdu = pdu;
		rsp_pdu_len = pdu_len;
	}

	goto done;

fail:
	att_debug(att, "(chan %p) Failed to handle response PDU; opcode: "
			"0x%02x", chan, opcode);

	rsp_opcode = BT_ATT_OP_ERROR_RSP;

done:
	if (op->callback)
		op->callback(rsp_opcode, rsp_pdu, rsp_pdu_len, op->user_data);

	destroy_att_send_op(op);
	chan->pending_req = NULL;

	wakeup_chan_writer(chan, NULL);
}

static void handle_conf(struct bt_att_chan *chan, uint8_t *pdu, ssize_t pdu_len)
{
	struct bt_att *att = chan->att;
	struct att_send_op *op = chan->pending_ind;

	/*
	 * Disconnect the bearer if the confirmation is unexpected or the PDU is
	 * invalid.
	 */
	if (!op || pdu_len) {
		att_debug(att, "(chan %p) Received unexpected/invalid ATT "
				"confirmation", chan);
		io_shutdown(chan->io);
		return;
	}

	if (op->callback)
		op->callback(BT_ATT_OP_HANDLE_CONF, NULL, 0, op->user_data);

	destroy_att_send_op(op);
	chan->pending_ind = NULL;

	wakeup_chan_writer(chan, NULL);
}

struct notify_data {
	uint8_t opcode;
	uint8_t *pdu;
	ssize_t pdu_len;
	bool handler_found;
};

static bool opcode_match(uint8_t opcode, uint8_t test_opcode)
{
	enum att_op_type op_type = get_op_type(test_opcode);

	if (opcode == BT_ATT_ALL_REQUESTS && (op_type == ATT_OP_TYPE_REQ ||
						op_type == ATT_OP_TYPE_CMD))
		return true;

	return opcode == test_opcode;
}

static void respond_not_supported(struct bt_att *att, uint8_t opcode)
{
	struct bt_att_pdu_error_rsp pdu;

	pdu.opcode = opcode;
	pdu.handle = 0x0000;
	pdu.ecode = BT_ATT_ERROR_REQUEST_NOT_SUPPORTED;

	bt_att_send(att, BT_ATT_OP_ERROR_RSP, &pdu, sizeof(pdu), NULL, NULL,
									NULL);
}

static bool handle_signed(struct bt_att *att, uint8_t *pdu, ssize_t pdu_len)
{
	uint8_t *signature;
	uint32_t sign_cnt;
	struct sign_info *sign;
	uint8_t opcode = pdu[0];

	/* Check if there is enough data for a signature */
	if (pdu_len < 3 + BT_ATT_SIGNATURE_LEN)
		goto fail;

	sign = att->remote_sign;
	if (!sign)
		goto fail;

	signature = pdu + (pdu_len - BT_ATT_SIGNATURE_LEN);
	sign_cnt = get_le32(signature);

	/* Validate counter */
	if (!sign->counter(&sign_cnt, sign->user_data))
		goto fail;

	/* Verify received signature */
	if (!bt_crypto_verify_att_sign(att->crypto, sign->key, pdu, pdu_len))
		goto fail;

	return true;

fail:
	att_debug(att, "ATT failed to verify signature: 0x%02x", opcode);

	return false;
}

static void handle_notify(struct bt_att_chan *chan, uint8_t *pdu,
							ssize_t pdu_len)
{
	struct bt_att *att = chan->att;
	const struct queue_entry *entry;
	bool found;
	uint8_t opcode = pdu[0];

	bt_att_ref(att);

	found = false;
	entry = queue_get_entries(att->notify_list);

	while (entry) {
		struct att_notify *notify = entry->data;

		entry = entry->next;

		if (!opcode_match(notify->opcode, opcode))
			continue;

		if ((opcode & ATT_OP_SIGNED_MASK) && att->crypto) {
			if (!handle_signed(att, pdu, pdu_len))
				return;
			pdu_len -= BT_ATT_SIGNATURE_LEN;
		}

		/* BLUETOOTH CORE SPECIFICATION Version 5.1 | Vol 3, Part G
		 * page 2370
		 *
		 * 4.3.1 Exchange MTU
		 *
		 * This sub-procedure shall not be used on a BR/EDR physical
		 * link since the MTU size is negotiated using L2CAP channel
		 * configuration procedures.
		 */
		if (bt_att_get_link_type(att) == BT_ATT_BREDR) {
			switch (opcode) {
			case BT_ATT_OP_MTU_REQ:
				goto not_supported;
			}
		}

		found = true;

		if (notify->callback)
			notify->callback(chan, opcode, pdu + 1, pdu_len - 1,
							notify->user_data);

		/* callback could remove all entries from notify list */
		if (queue_isempty(att->notify_list))
			break;
	}

not_supported:
	/*
	 * If this was not a command and no handler was registered for it,
	 * respond with "Not Supported"
	 */
	if (!found && get_op_type(opcode) != ATT_OP_TYPE_CMD)
		respond_not_supported(att, opcode);

	bt_att_unref(att);
}

static bool can_read_data(struct io *io, void *user_data)
{
	struct bt_att_chan *chan = user_data;
	struct bt_att *att = chan->att;
	uint8_t opcode;
	uint8_t *pdu;
	ssize_t bytes_read;

	bytes_read = read(chan->fd, chan->buf, chan->mtu);
	if (bytes_read < 0)
		return false;

	att_verbose(att, "(chan %p) ATT received: %zd", chan, bytes_read);

	att_hexdump(att, '>', chan->buf, bytes_read);

	if (bytes_read < ATT_MIN_PDU_LEN)
		return true;

	pdu = chan->buf;
	opcode = pdu[0];

	bt_att_ref(att);

	/* Act on the received PDU based on the opcode type */
	switch (get_op_type(opcode)) {
	case ATT_OP_TYPE_RSP:
		att_verbose(att, "(chan %p) ATT response received: 0x%02x",
				chan, opcode);
		handle_rsp(chan, opcode, pdu + 1, bytes_read - 1);
		break;
	case ATT_OP_TYPE_CONF:
		att_verbose(att, "(chan %p) ATT confirmation received: 0x%02x",
				chan, opcode);
		handle_conf(chan, pdu + 1, bytes_read - 1);
		break;
	case ATT_OP_TYPE_REQ:
		/*
		 * If a request is currently pending, then the sequential
		 * protocol was violated. Disconnect the bearer, which will
		 * promptly notify the upper layer via disconnect handlers.
		 */
		if (chan->in_req) {
			att_debug(att, "(chan %p) Received request while "
					"another is pending: 0x%02x",
					chan, opcode);
			io_shutdown(chan->io);
			bt_att_unref(chan->att);

			return false;
		}

		chan->in_req = true;
		/* fall through */
	case ATT_OP_TYPE_CMD:
	case ATT_OP_TYPE_NFY:
	case ATT_OP_TYPE_UNKNOWN:
	case ATT_OP_TYPE_IND:
		/* fall through */
	default:
		/* For all other opcodes notify the upper layer of the PDU and
		 * let them act on it.
		 */
		att_debug(att, "(chan %p) ATT PDU received: 0x%02x", chan,
							opcode);
		handle_notify(chan, pdu, bytes_read);
		break;
	}

	bt_att_unref(att);

	return true;
}

static bool is_io_l2cap_based(int fd)
{
	int domain;
	int proto;
	int err;
	socklen_t len;

	domain = 0;
	len = sizeof(domain);
	err = getsockopt(fd, SOL_SOCKET, SO_DOMAIN, &domain, &len);
	if (err < 0)
		return false;

	if (domain != AF_BLUETOOTH)
		return false;

	proto = 0;
	len = sizeof(proto);
	err = getsockopt(fd, SOL_SOCKET, SO_PROTOCOL, &proto, &len);
	if (err < 0)
		return false;

	return proto == BTPROTO_L2CAP;
}

static void bt_att_free(struct bt_att *att)
{
	bt_crypto_unref(att->crypto);

	if (att->timeout_destroy)
		att->timeout_destroy(att->timeout_data);

	if (att->debug_destroy)
		att->debug_destroy(att->debug_data);

	free(att->local_sign);
	free(att->remote_sign);

	queue_destroy(att->req_queue, NULL);
	queue_destroy(att->ind_queue, NULL);
	queue_destroy(att->write_queue, NULL);
	queue_destroy(att->notify_list, NULL);
	queue_destroy(att->disconn_list, NULL);
	queue_destroy(att->chans, bt_att_chan_free);

	free(att);
}

static uint16_t io_get_mtu(int fd)
{
	socklen_t len;
	struct l2cap_options l2o;

	len = sizeof(l2o);
	if (!getsockopt(fd, SOL_L2CAP, L2CAP_OPTIONS, &l2o, &len))
		return l2o.omtu;

	if (!getsockopt(fd, SOL_BLUETOOTH, BT_SNDMTU, &l2o.omtu, &len))
		return l2o.omtu;

	return 0;
}

static uint8_t io_get_type(int fd)
{
	struct sockaddr_l2 src;
	socklen_t len;

	if (!is_io_l2cap_based(fd))
		return BT_ATT_LOCAL;

	len = sizeof(src);
	memset(&src, 0, len);
	if (getsockname(fd, (void *)&src, &len) < 0)
		return -errno;

	if (src.l2_bdaddr_type == BDADDR_BREDR)
		return BT_ATT_BREDR;

	return BT_ATT_LE;
}

static struct bt_att_chan *bt_att_chan_new(int fd, uint8_t type)
{
	struct bt_att_chan *chan;

	if (fd < 0)
		return NULL;

	chan = new0(struct bt_att_chan, 1);
	chan->fd = fd;

	chan->io = io_new(fd);
	if (!chan->io)
		goto fail;

	if (!io_set_read_handler(chan->io, can_read_data, chan, NULL))
		goto fail;

	if (!io_set_disconnect_handler(chan->io, disconnect_cb, chan, NULL))
		goto fail;

	chan->type = type;
	switch (chan->type) {
	case BT_ATT_LOCAL:
		chan->sec_level = BT_ATT_SECURITY_LOW;
		/* fall through */
	case BT_ATT_LE:
		chan->mtu = BT_ATT_DEFAULT_LE_MTU;
		break;
	default:
		chan->mtu = io_get_mtu(chan->fd);
	}

	if (chan->mtu < BT_ATT_DEFAULT_LE_MTU)
		goto fail;

	chan->buf = malloc(chan->mtu);
	if (!chan->buf)
		goto fail;

	chan->queue = queue_new();

	return chan;

fail:
	bt_att_chan_free(chan);

	return NULL;
}

static void bt_att_attach_chan(struct bt_att *att, struct bt_att_chan *chan)
{
	/* Push to head as EATT channels have higher priority */
	queue_push_head(att->chans, chan);
	chan->att = att;

	if (chan->mtu > att->mtu)
		att->mtu = chan->mtu;

	io_set_close_on_destroy(chan->io, att->close_on_unref);

	att_debug(att, "Channel %p attached", chan);

	wakeup_chan_writer(chan, NULL);
}

struct bt_att *bt_att_new(int fd, bool ext_signed)
{
	struct bt_att *att;
	struct bt_att_chan *chan;

	chan = bt_att_chan_new(fd, io_get_type(fd));
	if (!chan)
		return NULL;

	att = new0(struct bt_att, 1);
	att->chans = queue_new();
	att->mtu = chan->mtu;

	/* crypto is optional, if not available leave it NULL */
	if (!ext_signed)
		att->crypto = bt_crypto_new();

	att->req_queue = queue_new();
	att->ind_queue = queue_new();
	att->write_queue = queue_new();
	att->notify_list = queue_new();
	att->disconn_list = queue_new();

	bt_att_attach_chan(att, chan);

	return bt_att_ref(att);
}

struct bt_att *bt_att_ref(struct bt_att *att)
{
	if (!att)
		return NULL;

	__sync_fetch_and_add(&att->ref_count, 1);

	return att;
}

void bt_att_unref(struct bt_att *att)
{
	if (!att)
		return;

	if (__sync_sub_and_fetch(&att->ref_count, 1))
		return;

	bt_att_unregister_all(att);
	bt_att_cancel_all(att);

	bt_att_free(att);
}

bool bt_att_set_close_on_unref(struct bt_att *att, bool do_close)
{
	const struct queue_entry *entry;

	if (!att)
		return false;

	att->close_on_unref = do_close;

	for (entry = queue_get_entries(att->chans); entry;
						entry = entry->next) {
		struct bt_att_chan *chan = entry->data;

		if (!io_set_close_on_destroy(chan->io, do_close))
			return false;
	}

	return true;
}

int bt_att_attach_fd(struct bt_att *att, int fd)
{
	struct bt_att_chan *chan;

	if (!att || fd < 0)
		return -EINVAL;

	chan = bt_att_chan_new(fd, BT_ATT_EATT);
	if (!chan)
		return -EINVAL;

	bt_att_attach_chan(att, chan);

	return 0;
}

int bt_att_get_fd(struct bt_att *att)
{
	struct bt_att_chan *chan;

	if (!att)
		return -1;

	if (queue_isempty(att->chans))
		return -ENOTCONN;

	chan = queue_peek_tail(att->chans);

	return chan->fd;
}

int bt_att_get_channels(struct bt_att *att)
{
	if (!att)
		return 0;

	return queue_length(att->chans);
}

bool bt_att_set_debug(struct bt_att *att, uint8_t level,
			bt_att_debug_func_t callback, void *user_data,
			bt_att_destroy_func_t destroy)
{
	if (!att)
		return false;

	if (att->debug_destroy)
		att->debug_destroy(att->debug_data);

	att->debug_level = level;
	att->debug_callback = callback;
	att->debug_destroy = destroy;
	att->debug_data = user_data;

	return true;
}

uint16_t bt_att_get_mtu(struct bt_att *att)
{
	if (!att)
		return 0;

	return att->mtu;
}

bool bt_att_set_mtu(struct bt_att *att, uint16_t mtu)
{
	struct bt_att_chan *chan;
	void *buf;

	if (!att)
		return false;

	if (mtu < BT_ATT_DEFAULT_LE_MTU)
		return false;

	/* Original channel is always the last */
	chan = queue_peek_tail(att->chans);
	if (!chan)
		return -ENOTCONN;

	buf = malloc(mtu);
	if (!buf)
		return false;

	free(chan->buf);

	chan->mtu = mtu;
	chan->buf = buf;

	if (chan->mtu > att->mtu)
		att->mtu = chan->mtu;

	return true;
}

uint8_t bt_att_get_link_type(struct bt_att *att)
{
	struct bt_att_chan *chan;

	if (!att)
		return -EINVAL;

	chan = queue_peek_tail(att->chans);
	if (!chan)
		return -ENOTCONN;

	return chan->type;
}

bool bt_att_set_timeout_cb(struct bt_att *att, bt_att_timeout_func_t callback,
						void *user_data,
						bt_att_destroy_func_t destroy)
{
	if (!att)
		return false;

	if (att->timeout_destroy)
		att->timeout_destroy(att->timeout_data);

	att->timeout_callback = callback;
	att->timeout_destroy = destroy;
	att->timeout_data = user_data;

	return true;
}

unsigned int bt_att_register_disconnect(struct bt_att *att,
					bt_att_disconnect_func_t callback,
					void *user_data,
					bt_att_destroy_func_t destroy)
{
	struct att_disconn *disconn;

	if (!att || queue_isempty(att->chans))
		return 0;

	disconn = new0(struct att_disconn, 1);
	disconn->callback = callback;
	disconn->destroy = destroy;
	disconn->user_data = user_data;

	if (att->next_reg_id < 1)
		att->next_reg_id = 1;

	disconn->id = att->next_reg_id++;

	if (!queue_push_tail(att->disconn_list, disconn)) {
		free(disconn);
		return 0;
	}

	return disconn->id;
}

bool bt_att_unregister_disconnect(struct bt_att *att, unsigned int id)
{
	struct att_disconn *disconn;

	if (!att || !id)
		return false;

	/* Check if disconnect is running */
	if (queue_isempty(att->chans)) {
		disconn = queue_find(att->disconn_list, match_disconn_id,
							UINT_TO_PTR(id));
		if (!disconn)
			return false;

		disconn->removed = true;
		return true;
	}

	disconn = queue_remove_if(att->disconn_list, match_disconn_id,
							UINT_TO_PTR(id));
	if (!disconn)
		return false;

	destroy_att_disconn(disconn);
	return true;
}

unsigned int bt_att_send(struct bt_att *att, uint8_t opcode,
				const void *pdu, uint16_t length,
				bt_att_response_func_t callback, void *user_data,
				bt_att_destroy_func_t destroy)
{
	struct att_send_op *op;
	bool result;

	if (!att || queue_isempty(att->chans))
		return 0;

	op = create_att_send_op(att, opcode, pdu, length, callback, user_data,
								destroy);
	if (!op)
		return 0;

	if (att->next_send_id < 1)
		att->next_send_id = 1;

	op->id = att->next_send_id++;

	/* Add the op to the correct queue based on its type */
	switch (op->type) {
	case ATT_OP_TYPE_REQ:
		result = queue_push_tail(att->req_queue, op);
		break;
	case ATT_OP_TYPE_IND:
		result = queue_push_tail(att->ind_queue, op);
		break;
	case ATT_OP_TYPE_CMD:
	case ATT_OP_TYPE_NFY:
	case ATT_OP_TYPE_UNKNOWN:
	case ATT_OP_TYPE_RSP:
	case ATT_OP_TYPE_CONF:
	default:
		result = queue_push_tail(att->write_queue, op);
		break;
	}

	if (!result) {
		free(op->pdu);
		free(op);
		return 0;
	}

	wakeup_writer(att);

	return op->id;
}

unsigned int bt_att_chan_send(struct bt_att_chan *chan, uint8_t opcode,
				const void *pdu, uint16_t len,
				bt_att_response_func_t callback,
				void *user_data,
				bt_att_destroy_func_t destroy)
{
	struct att_send_op *op;

	if (!chan || !chan->att)
		return -EINVAL;

	op = create_att_send_op(chan->att, opcode, pdu, len, callback,
						user_data, destroy);
	if (!op)
		return -EINVAL;

	if (!queue_push_tail(chan->queue, op)) {
		free(op->pdu);
		free(op);
		return 0;
	}

	wakeup_chan_writer(chan, NULL);

	return op->id;
}

static bool match_op_id(const void *a, const void *b)
{
	const struct att_send_op *op = a;
	unsigned int id = PTR_TO_UINT(b);

	return op->id == id;
}

bool bt_att_chan_cancel(struct bt_att_chan *chan, unsigned int id)
{
	struct att_send_op *op;

	if (chan->pending_req && chan->pending_req->id == id) {
		/* Don't cancel the pending request; remove it's handlers */
		cancel_att_send_op(chan->pending_req);
		return true;
	}

	if (chan->pending_ind && chan->pending_ind->id == id) {
		/* Don't cancel the pending indication; remove it's handlers. */
		cancel_att_send_op(chan->pending_ind);
		return true;
	}

	op = queue_remove_if(chan->queue, match_op_id, UINT_TO_PTR(id));
	if (!op)
		return false;

	destroy_att_send_op(op);

	wakeup_chan_writer(chan, NULL);

	return true;
}

static bool bt_att_disc_cancel(struct bt_att *att, unsigned int id)
{
	struct att_send_op *op;

	op = queue_find(att->req_queue, match_op_id, UINT_TO_PTR(id));
	if (op)
		goto done;

	op = queue_find(att->ind_queue, match_op_id, UINT_TO_PTR(id));
	if (op)
		goto done;

	op = queue_find(att->write_queue, match_op_id, UINT_TO_PTR(id));

done:
	if (!op)
		return false;

	/* Just cancel since disconnect_cb will be cleaning up */
	cancel_att_send_op(op);

	return true;
}

bool bt_att_cancel(struct bt_att *att, unsigned int id)
{
	const struct queue_entry *entry;
	struct att_send_op *op;

	if (!att || !id)
		return false;

	/* Lookuo request on each channel first */
	for (entry = queue_get_entries(att->chans); entry;
						entry = entry->next) {
		struct bt_att_chan *chan = entry->data;

		if (bt_att_chan_cancel(chan, id))
			return true;
	}

	if (att->in_disc)
		return bt_att_disc_cancel(att, id);

	op = queue_remove_if(att->req_queue, match_op_id, UINT_TO_PTR(id));
	if (op)
		goto done;

	op = queue_remove_if(att->ind_queue, match_op_id, UINT_TO_PTR(id));
	if (op)
		goto done;

	op = queue_remove_if(att->write_queue, match_op_id, UINT_TO_PTR(id));
	if (op)
		goto done;

	if (!op)
		return false;

done:
	destroy_att_send_op(op);

	wakeup_writer(att);

	return true;
}

bool bt_att_cancel_all(struct bt_att *att)
{
	const struct queue_entry *entry;

	if (!att)
		return false;

	queue_remove_all(att->req_queue, NULL, NULL, destroy_att_send_op);
	queue_remove_all(att->ind_queue, NULL, NULL, destroy_att_send_op);
	queue_remove_all(att->write_queue, NULL, NULL, destroy_att_send_op);

	for (entry = queue_get_entries(att->chans); entry;
						entry = entry->next) {
		struct bt_att_chan *chan = entry->data;

		if (chan->pending_req)
			/* Don't cancel the pending request; remove it's
			 * handlers
			 */
			cancel_att_send_op(chan->pending_req);

		if (chan->pending_ind)
			/* Don't cancel the pending request; remove it's
			 * handlers
			 */
			cancel_att_send_op(chan->pending_ind);
	}

	return true;
}

static uint8_t att_ecode_from_error(int err)
{
	/*
	 * If the error fits in a single byte, treat it as an ATT protocol
	 * error as is. Since "0" is not a valid ATT protocol error code, we map
	 * that to UNLIKELY below.
	 */
	if (err > 0 && err < UINT8_MAX)
		return err;

	/*
	 * Since we allow UNIX errnos, map them to appropriate ATT protocol
	 * and "Common Profile and Service" error codes.
	 */
	switch (err) {
	case -ENOENT:
		return BT_ATT_ERROR_INVALID_HANDLE;
	case -ENOMEM:
		return BT_ATT_ERROR_INSUFFICIENT_RESOURCES;
	case -EALREADY:
		return BT_ERROR_ALREADY_IN_PROGRESS;
	case -EOVERFLOW:
		return BT_ERROR_OUT_OF_RANGE;
	}

	return BT_ATT_ERROR_UNLIKELY;
}

int bt_att_chan_send_error_rsp(struct bt_att_chan *chan, uint8_t opcode,
						uint16_t handle, int error)
{
	struct bt_att_pdu_error_rsp pdu;
	uint8_t ecode;

	if (!chan || !chan->att || !opcode)
		return -EINVAL;

	ecode = att_ecode_from_error(error);

	memset(&pdu, 0, sizeof(pdu));

	pdu.opcode = opcode;
	put_le16(handle, &pdu.handle);
	pdu.ecode = ecode;

	return bt_att_chan_send_rsp(chan, BT_ATT_OP_ERROR_RSP, &pdu,
							sizeof(pdu));
}

unsigned int bt_att_register(struct bt_att *att, uint8_t opcode,
						bt_att_notify_func_t callback,
						void *user_data,
						bt_att_destroy_func_t destroy)
{
	struct att_notify *notify;

	if (!att || !callback || queue_isempty(att->chans))
		return 0;

	notify = new0(struct att_notify, 1);
	notify->opcode = opcode;
	notify->callback = callback;
	notify->destroy = destroy;
	notify->user_data = user_data;

	if (att->next_reg_id < 1)
		att->next_reg_id = 1;

	notify->id = att->next_reg_id++;

	if (!queue_push_tail(att->notify_list, notify)) {
		free(notify);
		return 0;
	}

	return notify->id;
}

bool bt_att_unregister(struct bt_att *att, unsigned int id)
{
	struct att_notify *notify;

	if (!att || !id)
		return false;

	notify = queue_remove_if(att->notify_list, match_notify_id,
							UINT_TO_PTR(id));
	if (!notify)
		return false;

	destroy_att_notify(notify);
	return true;
}

bool bt_att_unregister_all(struct bt_att *att)
{
	if (!att)
		return false;

	queue_remove_all(att->notify_list, NULL, NULL, destroy_att_notify);
	queue_remove_all(att->disconn_list, NULL, NULL, destroy_att_disconn);

	return true;
}

int bt_att_get_security(struct bt_att *att, uint8_t *enc_size)
{
	struct bt_att_chan *chan;
	int ret;

	if (!att)
		return -EINVAL;

	chan = queue_peek_tail(att->chans);
	if (!chan)
		return -ENOTCONN;

	ret = bt_att_chan_get_security(chan);
	if (ret < 0)
		return ret;

	if (enc_size)
		*enc_size = att->enc_size;

	return ret;
}

bool bt_att_set_security(struct bt_att *att, int level)
{
	struct bt_att_chan *chan;

	if (!att || level < BT_ATT_SECURITY_AUTO ||
						level > BT_ATT_SECURITY_HIGH)
		return false;

	chan = queue_peek_tail(att->chans);
	if (!chan)
		return -ENOTCONN;

	return bt_att_chan_set_security(chan, level);
}

void bt_att_set_enc_key_size(struct bt_att *att, uint8_t enc_size)
{
	if (!att)
		return;

	att->enc_size = enc_size;
}

static bool sign_set_key(struct sign_info **sign, uint8_t key[16],
				bt_att_counter_func_t func, void *user_data)
{
	if (!(*sign))
		*sign = new0(struct sign_info, 1);

	(*sign)->counter = func;
	(*sign)->user_data = user_data;
	memcpy((*sign)->key, key, 16);

	return true;
}

bool bt_att_set_local_key(struct bt_att *att, uint8_t sign_key[16],
				bt_att_counter_func_t func, void *user_data)
{
	if (!att)
		return false;

	return sign_set_key(&att->local_sign, sign_key, func, user_data);
}

bool bt_att_set_remote_key(struct bt_att *att, uint8_t sign_key[16],
				bt_att_counter_func_t func, void *user_data)
{
	if (!att)
		return false;

	return sign_set_key(&att->remote_sign, sign_key, func, user_data);
}

bool bt_att_has_crypto(struct bt_att *att)
{
	if (!att)
		return false;

	return att->crypto ? true : false;
}

// SPDX-License-Identifier: GPL-2.0
/*
 * (C) 2006 - Cambridge University
 * (C) 2020 - EPAM Systems Inc.
 *
 * File: xenbus.c [1]
 * Author: Steven Smith (sos22@cam.ac.uk)
 * Changes: Grzegorz Milos (gm281@cam.ac.uk)
 * Changes: John D. Ramsdell
 *
 * Date: Jun 2006, changes Aug 2006
 *
 * Description: Minimal implementation of xenbus
 *
 * [1] - http://xenbits.xen.org/gitweb/?p=mini-os.git;a=summary
 */

#include <common.h>
#include <log.h>

#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <asm/xen/system.h>

#include <linux/bug.h>
#include <linux/compat.h>

#include <xen/events.h>
#include <xen/hvm.h>
#include <xen/xenbus.h>

#include <xen/interface/io/xs_wire.h>

#define map_frame_virt(v)	(v << PAGE_SHIFT)

#define SCNd16			"d"

/* Wait for reply time out, ms */
#define WAIT_XENBUS_TO_MS	5000
/* Polling time out, ms */
#define WAIT_XENBUS_POLL_TO_MS	1

static struct xenstore_domain_interface *xenstore_buf;

static char *errmsg(struct xsd_sockmsg *rep);

u32 xenbus_evtchn;

struct write_req {
	const void *data;
	unsigned int len;
};

static void memcpy_from_ring(const void *r, void *d, int off, int len)
{
	int c1, c2;
	const char *ring = r;
	char *dest = d;

	c1 = min(len, XENSTORE_RING_SIZE - off);
	c2 = len - c1;
	memcpy(dest, ring + off, c1);
	memcpy(dest + c1, ring, c2);
}

/**
 * xenbus_get_reply() - Receive reply from xenbus
 * @req_reply: reply message structure
 *
 * Wait for reply message event from the ring and copy received message
 * to input xsd_sockmsg structure. Repeat until full reply is
 * proceeded.
 *
 * Return: false - timeout
 *	   true - reply is received
 */
static bool xenbus_get_reply(struct xsd_sockmsg **req_reply)
{
	struct xsd_sockmsg msg;
	unsigned int prod = xenstore_buf->rsp_prod;

again:
	if (!wait_event_timeout(NULL, prod != xenstore_buf->rsp_prod,
				WAIT_XENBUS_TO_MS)) {
		printk("%s: wait_event timeout\n", __func__);
		return false;
	}

	prod = xenstore_buf->rsp_prod;
	if (xenstore_buf->rsp_prod - xenstore_buf->rsp_cons < sizeof(msg))
		goto again;

	rmb();
	memcpy_from_ring(xenstore_buf->rsp, &msg,
			 MASK_XENSTORE_IDX(xenstore_buf->rsp_cons),
			 sizeof(msg));

	if (xenstore_buf->rsp_prod - xenstore_buf->rsp_cons < sizeof(msg) + msg.len)
		goto again;

	/* We do not support and expect any Xen bus wathes. */
	BUG_ON(msg.type == XS_WATCH_EVENT);

	*req_reply = malloc(sizeof(msg) + msg.len);
	memcpy_from_ring(xenstore_buf->rsp, *req_reply,
			 MASK_XENSTORE_IDX(xenstore_buf->rsp_cons),
			 msg.len + sizeof(msg));
	mb();
	xenstore_buf->rsp_cons += msg.len + sizeof(msg);

	wmb();
	notify_remote_via_evtchn(xenbus_evtchn);
	return true;
}

char *xenbus_switch_state(xenbus_transaction_t xbt, const char *path,
			  XenbusState state)
{
	char *current_state;
	char *msg = NULL;
	char *msg2 = NULL;
	char value[2];
	XenbusState rs;
	int xbt_flag = 0;
	int retry = 0;

	do {
		if (xbt == XBT_NIL) {
			msg = xenbus_transaction_start(&xbt);
			if (msg)
				goto exit;
			xbt_flag = 1;
		}

		msg = xenbus_read(xbt, path, &current_state);
		if (msg)
			goto exit;

		rs = (XenbusState)(current_state[0] - '0');
		free(current_state);
		if (rs == state) {
			msg = NULL;
			goto exit;
		}

		snprintf(value, 2, "%d", state);
		msg = xenbus_write(xbt, path, value);

exit:
		if (xbt_flag) {
			msg2 = xenbus_transaction_end(xbt, 0, &retry);
			xbt = XBT_NIL;
		}
		if (msg == NULL && msg2 != NULL)
			msg = msg2;
		else
			free(msg2);
	} while (retry);

	return msg;
}

char *xenbus_wait_for_state_change(const char *path, XenbusState *state)
{
	for (;;) {
		char *res, *msg;
		XenbusState rs;

		msg = xenbus_read(XBT_NIL, path, &res);
		if (msg)
			return msg;

		rs = (XenbusState)(res[0] - 48);
		free(res);

		if (rs == *state) {
			wait_event_timeout(NULL, false, WAIT_XENBUS_POLL_TO_MS);
		} else {
			*state = rs;
			break;
		}
	}
	return NULL;
}

/* Send data to xenbus.  This can block.  All of the requests are seen
 * by xenbus as if sent atomically.  The header is added
 * automatically, using type %type, req_id %req_id, and trans_id
 * %trans_id.
 */
static void xb_write(int type, int req_id, xenbus_transaction_t trans_id,
		     const struct write_req *req, int nr_reqs)
{
	XENSTORE_RING_IDX prod;
	int r;
	int len = 0;
	const struct write_req *cur_req;
	int req_off;
	int total_off;
	int this_chunk;
	struct xsd_sockmsg m = {
		.type = type,
		.req_id = req_id,
		.tx_id = trans_id
	};
	struct write_req header_req = {
		&m,
		sizeof(m)
	};

	for (r = 0; r < nr_reqs; r++)
		len += req[r].len;
	m.len = len;
	len += sizeof(m);

	cur_req = &header_req;

	BUG_ON(len > XENSTORE_RING_SIZE);
	prod = xenstore_buf->req_prod;
	/* We are running synchronously, so it is a bug if we do not
	 * have enough room to send a message: please note that a message
	 * can occupy multiple slots in the ring buffer.
	 */
	BUG_ON(prod + len - xenstore_buf->req_cons > XENSTORE_RING_SIZE);

	total_off = 0;
	req_off = 0;
	while (total_off < len) {
		this_chunk = min(cur_req->len - req_off,
				 XENSTORE_RING_SIZE - MASK_XENSTORE_IDX(prod));
		memcpy((char *)xenstore_buf->req + MASK_XENSTORE_IDX(prod),
		       (char *)cur_req->data + req_off, this_chunk);
		prod += this_chunk;
		req_off += this_chunk;
		total_off += this_chunk;
		if (req_off == cur_req->len) {
			req_off = 0;
			if (cur_req == &header_req)
				cur_req = req;
			else
				cur_req++;
		}
	}

	BUG_ON(req_off != 0);
	BUG_ON(total_off != len);
	BUG_ON(prod > xenstore_buf->req_cons + XENSTORE_RING_SIZE);

	/* Remote must see entire message before updating indexes */
	wmb();

	xenstore_buf->req_prod += len;

	/* Send evtchn to notify remote */
	notify_remote_via_evtchn(xenbus_evtchn);
}

/* Send a message to xenbus, in the same fashion as xb_write, and
 * block waiting for a reply.  The reply is malloced and should be
 * freed by the caller.
 */
struct xsd_sockmsg *xenbus_msg_reply(int type,
				     xenbus_transaction_t trans,
				     struct write_req *io,
				     int nr_reqs)
{
	struct xsd_sockmsg *rep;

	/* We do not use request identifier which is echoed in daemon's response. */
	xb_write(type, 0, trans, io, nr_reqs);
	/* Now wait for the message to arrive. */
	if (!xenbus_get_reply(&rep))
		return NULL;
	return rep;
}

static char *errmsg(struct xsd_sockmsg *rep)
{
	char *res;

	if (!rep) {
		char msg[] = "No reply";
		size_t len = strlen(msg) + 1;

		return memcpy(malloc(len), msg, len);
	}
	if (rep->type != XS_ERROR)
		return NULL;
	res = malloc(rep->len + 1);
	memcpy(res, rep + 1, rep->len);
	res[rep->len] = 0;
	free(rep);
	return res;
}

/* List the contents of a directory.  Returns a malloc()ed array of
 * pointers to malloc()ed strings.  The array is NULL terminated.  May
 * block.
 */
char *xenbus_ls(xenbus_transaction_t xbt, const char *pre, char ***contents)
{
	struct xsd_sockmsg *reply, *repmsg;
	struct write_req req[] = { { pre, strlen(pre) + 1 } };
	int nr_elems, x, i;
	char **res, *msg;

	repmsg = xenbus_msg_reply(XS_DIRECTORY, xbt, req, ARRAY_SIZE(req));
	msg = errmsg(repmsg);
	if (msg) {
		*contents = NULL;
		return msg;
	}
	reply = repmsg + 1;
	for (x = nr_elems = 0; x < repmsg->len; x++)
		nr_elems += (((char *)reply)[x] == 0);
	res = malloc(sizeof(res[0]) * (nr_elems + 1));
	for (x = i = 0; i < nr_elems; i++) {
		int l = strlen((char *)reply + x);

		res[i] = malloc(l + 1);
		memcpy(res[i], (char *)reply + x, l + 1);
		x += l + 1;
	}
	res[i] = NULL;
	free(repmsg);
	*contents = res;
	return NULL;
}

char *xenbus_read(xenbus_transaction_t xbt, const char *path, char **value)
{
	struct write_req req[] = { {path, strlen(path) + 1} };
	struct xsd_sockmsg *rep;
	char *res, *msg;

	rep = xenbus_msg_reply(XS_READ, xbt, req, ARRAY_SIZE(req));
	msg = errmsg(rep);
	if (msg) {
		*value = NULL;
		return msg;
	}
	res = malloc(rep->len + 1);
	memcpy(res, rep + 1, rep->len);
	res[rep->len] = 0;
	free(rep);
	*value = res;
	return NULL;
}

char *xenbus_write(xenbus_transaction_t xbt, const char *path,
				   const char *value)
{
	struct write_req req[] = {
		{path, strlen(path) + 1},
		{value, strlen(value)},
	};
	struct xsd_sockmsg *rep;
	char *msg;

	rep = xenbus_msg_reply(XS_WRITE, xbt, req, ARRAY_SIZE(req));
	msg = errmsg(rep);
	if (msg)
		return msg;
	free(rep);
	return NULL;
}

char *xenbus_rm(xenbus_transaction_t xbt, const char *path)
{
	struct write_req req[] = { {path, strlen(path) + 1} };
	struct xsd_sockmsg *rep;
	char *msg;

	rep = xenbus_msg_reply(XS_RM, xbt, req, ARRAY_SIZE(req));
	msg = errmsg(rep);
	if (msg)
		return msg;
	free(rep);
	return NULL;
}

char *xenbus_get_perms(xenbus_transaction_t xbt, const char *path, char **value)
{
	struct write_req req[] = { {path, strlen(path) + 1} };
	struct xsd_sockmsg *rep;
	char *res, *msg;

	rep = xenbus_msg_reply(XS_GET_PERMS, xbt, req, ARRAY_SIZE(req));
	msg = errmsg(rep);
	if (msg) {
		*value = NULL;
		return msg;
	}
	res = malloc(rep->len + 1);
	memcpy(res, rep + 1, rep->len);
	res[rep->len] = 0;
	free(rep);
	*value = res;
	return NULL;
}

#define PERM_MAX_SIZE 32
char *xenbus_set_perms(xenbus_transaction_t xbt, const char *path,
		       domid_t dom, char perm)
{
	char value[PERM_MAX_SIZE];
	struct write_req req[] = {
		{path, strlen(path) + 1},
		{value, 0},
	};
	struct xsd_sockmsg *rep;
	char *msg;

	snprintf(value, PERM_MAX_SIZE, "%c%hu", perm, dom);
	req[1].len = strlen(value) + 1;
	rep = xenbus_msg_reply(XS_SET_PERMS, xbt, req, ARRAY_SIZE(req));
	msg = errmsg(rep);
	if (msg)
		return msg;
	free(rep);
	return NULL;
}

char *xenbus_transaction_start(xenbus_transaction_t *xbt)
{
	/* Xenstored becomes angry if you send a length 0 message, so just
	 * shove a nul terminator on the end
	 */
	struct write_req req = { "", 1};
	struct xsd_sockmsg *rep;
	char *err;

	rep = xenbus_msg_reply(XS_TRANSACTION_START, 0, &req, 1);
	err = errmsg(rep);
	if (err)
		return err;
	sscanf((char *)(rep + 1), "%lu", xbt);
	free(rep);
	return NULL;
}

char *xenbus_transaction_end(xenbus_transaction_t t, int abort, int *retry)
{
	struct xsd_sockmsg *rep;
	struct write_req req;
	char *err;

	*retry = 0;

	req.data = abort ? "F" : "T";
	req.len = 2;
	rep = xenbus_msg_reply(XS_TRANSACTION_END, t, &req, 1);
	err = errmsg(rep);
	if (err) {
		if (!strcmp(err, "EAGAIN")) {
			*retry = 1;
			free(err);
			return NULL;
		} else {
			return err;
		}
	}
	free(rep);
	return NULL;
}

int xenbus_read_integer(const char *path)
{
	char *res, *buf;
	int t;

	res = xenbus_read(XBT_NIL, path, &buf);
	if (res) {
		printk("Failed to read %s.\n", path);
		free(res);
		return -1;
	}
	sscanf(buf, "%d", &t);
	free(buf);
	return t;
}

int xenbus_read_uuid(const char *path, unsigned char uuid[16])
{
	char *res, *buf;

	res = xenbus_read(XBT_NIL, path, &buf);
	if (res) {
		printk("Failed to read %s.\n", path);
		free(res);
		return 0;
	}
	if (strlen(buf) != ((2 * 16) + 4) /* 16 hex bytes and 4 hyphens */
	    || sscanf(buf,
		      "%2hhx%2hhx%2hhx%2hhx-"
		      "%2hhx%2hhx-"
		      "%2hhx%2hhx-"
		      "%2hhx%2hhx-"
		      "%2hhx%2hhx%2hhx%2hhx%2hhx%2hhx",
		      uuid, uuid + 1, uuid + 2, uuid + 3,
		      uuid + 4, uuid + 5, uuid + 6, uuid + 7,
		      uuid + 8, uuid + 9, uuid + 10, uuid + 11,
		      uuid + 12, uuid + 13, uuid + 14, uuid + 15) != 16) {
		printk("Xenbus path %s value %s is not a uuid!\n", path, buf);
		free(buf);
		return 0;
	}
	free(buf);
	return 1;
}

char *xenbus_printf(xenbus_transaction_t xbt,
		    const char *node, const char *path,
		    const char *fmt, ...)
{
#define BUFFER_SIZE 256
	char fullpath[BUFFER_SIZE];
	char val[BUFFER_SIZE];
	va_list args;

	BUG_ON(strlen(node) + strlen(path) + 1 >= BUFFER_SIZE);
	sprintf(fullpath, "%s/%s", node, path);
	va_start(args, fmt);
	vsprintf(val, fmt, args);
	va_end(args);
	return xenbus_write(xbt, fullpath, val);
}

domid_t xenbus_get_self_id(void)
{
	char *dom_id;
	domid_t ret;

	BUG_ON(xenbus_read(XBT_NIL, "domid", &dom_id));
	sscanf(dom_id, "%"SCNd16, &ret);

	return ret;
}

void init_xenbus(void)
{
	u64 v;

	debug("%s\n", __func__);
	if (hvm_get_parameter(HVM_PARAM_STORE_EVTCHN, &v))
		BUG();
	xenbus_evtchn = v;

	if (hvm_get_parameter(HVM_PARAM_STORE_PFN, &v))
		BUG();
	xenstore_buf = (struct xenstore_domain_interface *)map_frame_virt(v);
}

void fini_xenbus(void)
{
	debug("%s\n", __func__);
}

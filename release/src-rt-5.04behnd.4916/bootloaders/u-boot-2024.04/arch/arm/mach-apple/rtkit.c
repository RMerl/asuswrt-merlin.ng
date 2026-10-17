// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021 Mark Kettenis <kettenis@openbsd.org>
 * (C) Copyright 2021 Copyright The Asahi Linux Contributors
 */

#include <common.h>
#include <mailbox.h>
#include <malloc.h>

#include <asm/arch/rtkit.h>
#include <linux/apple-mailbox.h>
#include <linux/bitfield.h>

#define APPLE_RTKIT_EP_MGMT 0
#define APPLE_RTKIT_EP_CRASHLOG	1
#define APPLE_RTKIT_EP_SYSLOG 2
#define APPLE_RTKIT_EP_DEBUG 3
#define APPLE_RTKIT_EP_IOREPORT 4
#define APPLE_RTKIT_EP_TRACEKIT 10

/* Messages for management endpoint. */
#define APPLE_RTKIT_MGMT_TYPE GENMASK(59, 52)

#define APPLE_RTKIT_MGMT_PWR_STATE GENMASK(15, 0)

#define APPLE_RTKIT_MGMT_HELLO 1
#define APPLE_RTKIT_MGMT_HELLO_REPLY 2
#define APPLE_RTKIT_MGMT_HELLO_MINVER GENMASK(15, 0)
#define APPLE_RTKIT_MGMT_HELLO_MAXVER GENMASK(31, 16)

#define APPLE_RTKIT_MGMT_STARTEP 5
#define APPLE_RTKIT_MGMT_STARTEP_EP GENMASK(39, 32)
#define APPLE_RTKIT_MGMT_STARTEP_FLAG BIT(1)

#define APPLE_RTKIT_MGMT_SET_IOP_PWR_STATE 6
#define APPLE_RTKIT_MGMT_SET_IOP_PWR_STATE_ACK 7

#define APPLE_RTKIT_MGMT_EPMAP 8
#define APPLE_RTKIT_MGMT_EPMAP_LAST BIT(51)
#define APPLE_RTKIT_MGMT_EPMAP_BASE GENMASK(34, 32)
#define APPLE_RTKIT_MGMT_EPMAP_BITMAP GENMASK(31, 0)

#define APPLE_RTKIT_MGMT_EPMAP_REPLY 8
#define APPLE_RTKIT_MGMT_EPMAP_REPLY_MORE BIT(0)

#define APPLE_RTKIT_MIN_SUPPORTED_VERSION 11
#define APPLE_RTKIT_MAX_SUPPORTED_VERSION 12

/* Messages for internal endpoints. */
#define APPLE_RTKIT_BUFFER_REQUEST 1
#define APPLE_RTKIT_BUFFER_REQUEST_SIZE GENMASK(51, 44)
#define APPLE_RTKIT_BUFFER_REQUEST_IOVA GENMASK(41, 0)

#define TIMEOUT_1SEC_US 1000000

struct apple_rtkit {
	struct mbox_chan *chan;
	void *cookie;
	apple_rtkit_shmem_setup shmem_setup;
	apple_rtkit_shmem_destroy shmem_destroy;

	struct apple_rtkit_buffer syslog_buffer;
	struct apple_rtkit_buffer crashlog_buffer;
	struct apple_rtkit_buffer ioreport_buffer;
};

struct apple_rtkit *apple_rtkit_init(struct mbox_chan *chan, void *cookie,
				     apple_rtkit_shmem_setup shmem_setup,
				     apple_rtkit_shmem_destroy shmem_destroy)
{
	struct apple_rtkit *rtk;

	rtk = calloc(sizeof(*rtk), 1);
	if (!rtk)
		return NULL;

	rtk->chan = chan;
	rtk->cookie = cookie;
	rtk->shmem_setup = shmem_setup;
	rtk->shmem_destroy = shmem_destroy;

	return rtk;
}

void apple_rtkit_free(struct apple_rtkit *rtk)
{
	if (rtk->shmem_destroy) {
		if (rtk->syslog_buffer.buffer)
			rtk->shmem_destroy(rtk->cookie, &rtk->syslog_buffer);
		if (rtk->crashlog_buffer.buffer)
			rtk->shmem_destroy(rtk->cookie, &rtk->crashlog_buffer);
		if (rtk->ioreport_buffer.buffer)
			rtk->shmem_destroy(rtk->cookie, &rtk->ioreport_buffer);
	}
	free(rtk);
}

static int rtkit_handle_buf_req(struct apple_rtkit *rtk, int endpoint, struct apple_mbox_msg *msg)
{
	struct apple_rtkit_buffer *buf;
	size_t num_4kpages;
	int ret;

	num_4kpages = FIELD_GET(APPLE_RTKIT_BUFFER_REQUEST_SIZE, msg->msg0);

	if (num_4kpages == 0) {
		printf("%s: unexpected request for buffer without size\n", __func__);
		return -1;
	}

	switch (endpoint) {
	case APPLE_RTKIT_EP_CRASHLOG:
		buf = &rtk->crashlog_buffer;
		break;
	case APPLE_RTKIT_EP_SYSLOG:
		buf = &rtk->syslog_buffer;
		break;
	case APPLE_RTKIT_EP_IOREPORT:
		buf = &rtk->ioreport_buffer;
		break;
	default:
		printf("%s: unexpected endpoint %d\n", __func__, endpoint);
		return -1;
	}

	buf->dva = FIELD_GET(APPLE_RTKIT_BUFFER_REQUEST_IOVA, msg->msg0);
	buf->size = num_4kpages << 12;
	buf->is_mapped = false;

	if (rtk->shmem_setup) {
		ret = rtk->shmem_setup(rtk->cookie, buf);
		if (ret < 0) {
			printf("%s: shmen_setup failed for endpoint %d\n", __func__,
			       endpoint);
			return ret;
		}
	}

	if (!buf->is_mapped) {
		msg->msg0 = FIELD_PREP(APPLE_RTKIT_MGMT_TYPE, APPLE_RTKIT_BUFFER_REQUEST) |
				FIELD_PREP(APPLE_RTKIT_BUFFER_REQUEST_SIZE, num_4kpages) |
				FIELD_PREP(APPLE_RTKIT_BUFFER_REQUEST_IOVA, buf->dva);
		msg->msg1 = endpoint;

		return mbox_send(rtk->chan, msg);
	}

	return 0;
}

int apple_rtkit_boot(struct apple_rtkit *rtk)
{
	struct apple_mbox_msg msg;
	int endpoints[256];
	int nendpoints = 0;
	int endpoint;
	int min_ver, max_ver, want_ver;
	int msgtype, pwrstate;
	u64 reply;
	u32 bitmap, base;
	int i, ret;

	/* Wakup the IOP. */
	msg.msg0 = FIELD_PREP(APPLE_RTKIT_MGMT_TYPE, APPLE_RTKIT_MGMT_SET_IOP_PWR_STATE) |
		FIELD_PREP(APPLE_RTKIT_MGMT_PWR_STATE, APPLE_RTKIT_PWR_STATE_ON);
	msg.msg1 = APPLE_RTKIT_EP_MGMT;
	ret = mbox_send(rtk->chan, &msg);
	if (ret < 0)
		return ret;

	/* Wait for protocol version negotiation message. */
	ret = mbox_recv(rtk->chan, &msg, TIMEOUT_1SEC_US);
	if (ret < 0)
		return ret;

	endpoint = msg.msg1;
	msgtype = FIELD_GET(APPLE_RTKIT_MGMT_TYPE, msg.msg0);
	if (endpoint != APPLE_RTKIT_EP_MGMT) {
		printf("%s: unexpected endpoint %d\n", __func__, endpoint);
		return -EINVAL;
	}
	if (msgtype != APPLE_RTKIT_MGMT_HELLO) {
		printf("%s: unexpected message type %d\n", __func__, msgtype);
		return -EINVAL;
	}

	min_ver = FIELD_GET(APPLE_RTKIT_MGMT_HELLO_MINVER, msg.msg0);
	max_ver = FIELD_GET(APPLE_RTKIT_MGMT_HELLO_MAXVER, msg.msg0);
	want_ver = min(APPLE_RTKIT_MAX_SUPPORTED_VERSION, max_ver);

	if (min_ver > APPLE_RTKIT_MAX_SUPPORTED_VERSION) {
		printf("%s: firmware min version %d is too new\n",
		       __func__, min_ver);
		return -ENOTSUPP;
	}

	if (max_ver < APPLE_RTKIT_MIN_SUPPORTED_VERSION) {
		printf("%s: firmware max version %d is too old\n",
		       __func__, max_ver);
		return -ENOTSUPP;
	}

	/* Ack version. */
	msg.msg0 = FIELD_PREP(APPLE_RTKIT_MGMT_TYPE, APPLE_RTKIT_MGMT_HELLO_REPLY) |
		FIELD_PREP(APPLE_RTKIT_MGMT_HELLO_MINVER, want_ver) |
		FIELD_PREP(APPLE_RTKIT_MGMT_HELLO_MAXVER, want_ver);
	msg.msg1 = APPLE_RTKIT_EP_MGMT;
	ret = mbox_send(rtk->chan, &msg);
	if (ret < 0)
		return ret;

wait_epmap:
	/* Wait for endpoint map message. */
	ret = mbox_recv(rtk->chan, &msg, TIMEOUT_1SEC_US);
	if (ret < 0)
		return ret;

	endpoint = msg.msg1;
	msgtype = FIELD_GET(APPLE_RTKIT_MGMT_TYPE, msg.msg0);
	if (endpoint != APPLE_RTKIT_EP_MGMT) {
		printf("%s: unexpected endpoint %d\n", __func__, endpoint);
		return -EINVAL;
	}
	if (msgtype != APPLE_RTKIT_MGMT_EPMAP) {
		printf("%s: unexpected message type %d\n", __func__, msgtype);
		return -EINVAL;
	}

	bitmap = FIELD_GET(APPLE_RTKIT_MGMT_EPMAP_BITMAP, msg.msg0);
	base = FIELD_GET(APPLE_RTKIT_MGMT_EPMAP_BASE, msg.msg0);
	for (i = 0; i < 32; i++) {
		if (bitmap & (1U << i))
			endpoints[nendpoints++] = base * 32 + i;
	}

	/* Ack endpoint map. */
	reply = FIELD_PREP(APPLE_RTKIT_MGMT_TYPE, APPLE_RTKIT_MGMT_EPMAP_REPLY) |
		FIELD_PREP(APPLE_RTKIT_MGMT_EPMAP_BASE, base);
	if (msg.msg0 & APPLE_RTKIT_MGMT_EPMAP_LAST)
		reply |= APPLE_RTKIT_MGMT_EPMAP_LAST;
	else
		reply |= APPLE_RTKIT_MGMT_EPMAP_REPLY_MORE;
	msg.msg0 = reply;
	msg.msg1 = APPLE_RTKIT_EP_MGMT;
	ret = mbox_send(rtk->chan, &msg);
	if (ret < 0)
		return ret;

	if (reply & APPLE_RTKIT_MGMT_EPMAP_REPLY_MORE)
		goto wait_epmap;

	for (i = 0; i < nendpoints; i++) {
		/* Start only necessary endpoints. The syslog endpoint is
		 * particularly noisy and its message can't easily be handled
		 * within U-Boot.
		 */
		switch (endpoints[i]) {
		case APPLE_RTKIT_EP_MGMT:
		case APPLE_RTKIT_EP_SYSLOG:
		case APPLE_RTKIT_EP_DEBUG:
		case APPLE_RTKIT_EP_TRACEKIT:
			continue;
		default:
			break;
		}

		/* Request endpoint. */
		msg.msg0 = FIELD_PREP(APPLE_RTKIT_MGMT_TYPE, APPLE_RTKIT_MGMT_STARTEP) |
			FIELD_PREP(APPLE_RTKIT_MGMT_STARTEP_EP, endpoints[i]) |
			APPLE_RTKIT_MGMT_STARTEP_FLAG;
		msg.msg1 = APPLE_RTKIT_EP_MGMT;
		ret = mbox_send(rtk->chan, &msg);
		if (ret < 0)
			return ret;
	}

	pwrstate = APPLE_RTKIT_PWR_STATE_SLEEP;
	while (pwrstate != APPLE_RTKIT_PWR_STATE_ON) {
		ret = mbox_recv(rtk->chan, &msg, TIMEOUT_1SEC_US);
		if (ret < 0)
			return ret;

		endpoint = msg.msg1;
		msgtype = FIELD_GET(APPLE_RTKIT_MGMT_TYPE, msg.msg0);

		if (endpoint == APPLE_RTKIT_EP_CRASHLOG ||
		    endpoint == APPLE_RTKIT_EP_SYSLOG ||
		    endpoint == APPLE_RTKIT_EP_IOREPORT) {
			if (msgtype == APPLE_RTKIT_BUFFER_REQUEST) {
				ret = rtkit_handle_buf_req(rtk, endpoint, &msg);
				if (ret < 0)
					return ret;
				continue;
			}
		}

		if (endpoint == APPLE_RTKIT_EP_IOREPORT) {
			// these two messages have to be ack-ed for proper startup
			if (msgtype == 0xc || msgtype == 0x8) {
				ret = mbox_send(rtk->chan, &msg);
				if (ret < 0)
					return ret;
				continue;
			}
		}

		if (endpoint != APPLE_RTKIT_EP_MGMT) {
			printf("%s: unexpected endpoint %d\n", __func__, endpoint);
			return -EINVAL;
		}
		if (msgtype != APPLE_RTKIT_MGMT_SET_IOP_PWR_STATE_ACK) {
			printf("%s: unexpected message type %d\n", __func__, msgtype);
			return -EINVAL;
		}

		pwrstate = FIELD_GET(APPLE_RTKIT_MGMT_PWR_STATE, msg.msg0);
	}

	return 0;
}

int apple_rtkit_shutdown(struct apple_rtkit *rtk, int pwrstate)
{
	struct apple_mbox_msg msg;
	int ret;

	msg.msg0 = FIELD_PREP(APPLE_RTKIT_MGMT_TYPE, APPLE_RTKIT_MGMT_SET_IOP_PWR_STATE) |
		FIELD_PREP(APPLE_RTKIT_MGMT_PWR_STATE, pwrstate);
	msg.msg1 = APPLE_RTKIT_EP_MGMT;
	ret = mbox_send(rtk->chan, &msg);
	if (ret < 0)
		return ret;

	ret = mbox_recv(rtk->chan, &msg, TIMEOUT_1SEC_US);
	if (ret < 0)
		return ret;

	return 0;
}

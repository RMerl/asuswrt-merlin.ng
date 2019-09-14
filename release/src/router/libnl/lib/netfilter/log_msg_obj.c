/*
 * lib/netfilter/log_msg_obj.c	Netfilter Log Object
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2007 Philip Craig <philipc@snapgear.com>
 * Copyright (c) 2007 Secure Computing Corporation
 */

#include <netlink-private/netlink.h>
#include <netlink/netfilter/nfnl.h>
#include <netlink/netfilter/netfilter.h>
#include <netlink/netfilter/log_msg.h>

/** @cond SKIP */
#define LOG_MSG_ATTR_FAMILY		(1UL << 0)
#define LOG_MSG_ATTR_HWPROTO		(1UL << 1)
#define LOG_MSG_ATTR_HOOK		(1UL << 2)
#define LOG_MSG_ATTR_MARK		(1UL << 3)
#define LOG_MSG_ATTR_TIMESTAMP		(1UL << 4)
#define LOG_MSG_ATTR_INDEV		(1UL << 5)
#define LOG_MSG_ATTR_OUTDEV		(1UL << 6)
#define LOG_MSG_ATTR_PHYSINDEV		(1UL << 7)
#define LOG_MSG_ATTR_PHYSOUTDEV		(1UL << 8)
#define LOG_MSG_ATTR_HWADDR		(1UL << 9)
#define LOG_MSG_ATTR_PAYLOAD		(1UL << 10)
#define LOG_MSG_ATTR_PREFIX		(1UL << 11)
#define LOG_MSG_ATTR_UID		(1UL << 12)
#define LOG_MSG_ATTR_GID		(1UL << 13)
#define LOG_MSG_ATTR_SEQ		(1UL << 14)
#define LOG_MSG_ATTR_SEQ_GLOBAL		(1UL << 15)
/** @endcond */

static void log_msg_free_data(struct nl_object *c)
{
	struct nfnl_log_msg *msg = (struct nfnl_log_msg *) c;

	if (msg == NULL)
		return;

	free(msg->log_msg_payload);
	free(msg->log_msg_prefix);
}

static int log_msg_clone(struct nl_object *_dst, struct nl_object *_src)
{
	struct nfnl_log_msg *dst = (struct nfnl_log_msg *) _dst;
	struct nfnl_log_msg *src = (struct nfnl_log_msg *) _src;
	int err;

	if (src->log_msg_payload) {
		err = nfnl_log_msg_set_payload(dst, src->log_msg_payload,
					       src->log_msg_payload_len);
		if (err < 0)
			goto errout;
	}

	if (src->log_msg_prefix) {
		err = nfnl_log_msg_set_prefix(dst, src->log_msg_prefix);
		if (err < 0)
			goto errout;
	}

	return 0;
errout:
	return err;
}

static void log_msg_dump(struct nl_object *a, struct nl_dump_params *p)
{
	struct nfnl_log_msg *msg = (struct nfnl_log_msg *) a;
	struct nl_cache *link_cache;
	char buf[64];

	link_cache = nl_cache_mngt_require_safe("route/link");

	nl_new_line(p);

	if (msg->ce_mask & LOG_MSG_ATTR_PREFIX)
		nl_dump(p, "%s", msg->log_msg_prefix);

	if (msg->ce_mask & LOG_MSG_ATTR_INDEV) {
		if (link_cache)
			nl_dump(p, "IN=%s ",
				rtnl_link_i2name(link_cache,
						 msg->log_msg_indev,
						 buf, sizeof(buf)));
		else
			nl_dump(p, "IN=%d ", msg->log_msg_indev);
	}

	if (msg->ce_mask & LOG_MSG_ATTR_PHYSINDEV) {
		if (link_cache)
			nl_dump(p, "PHYSIN=%s ",
				rtnl_link_i2name(link_cache,
						 msg->log_msg_physindev,
						 buf, sizeof(buf)));
		else
			nl_dump(p, "IN=%d ", msg->log_msg_physindev);
	}

	if (msg->ce_mask & LOG_MSG_ATTR_OUTDEV) {
		if (link_cache)
			nl_dump(p, "OUT=%s ",
				rtnl_link_i2name(link_cache,
						 msg->log_msg_outdev,
						 buf, sizeof(buf)));
		else
			nl_dump(p, "OUT=%d ", msg->log_msg_outdev);
	}

	if (msg->ce_mask & LOG_MSG_ATTR_PHYSOUTDEV) {
		if (link_cache)
			nl_dump(p, "PHYSOUT=%s ",
				rtnl_link_i2name(link_cache,
						 msg->log_msg_physoutdev,
						 buf, sizeof(buf)));
		else
			nl_dump(p, "PHYSOUT=%d ", msg->log_msg_physoutdev);
	}

	if (msg->ce_mask & LOG_MSG_ATTR_HWADDR) {
		int i;

		nl_dump(p, "MAC");
		for (i = 0; i < msg->log_msg_hwaddr_len; i++)
			nl_dump(p, "%c%02x", i?':':'=', msg->log_msg_hwaddr[i]);
		nl_dump(p, " ");
	}

	/* FIXME: parse the payload to get iptables LOG compatible format */

	if (msg->ce_mask & LOG_MSG_ATTR_FAMILY)
		nl_dump(p, "FAMILY=%s ",
			nl_af2str(msg->log_msg_family, buf, sizeof(buf)));

	if (msg->ce_mask & LOG_MSG_ATTR_HWPROTO)
		nl_dump(p, "HWPROTO=%s ",
			nl_ether_proto2str(ntohs(msg->log_msg_hwproto),
					   buf, sizeof(buf)));

	if (msg->ce_mask & LOG_MSG_ATTR_HOOK)
		nl_dump(p, "HOOK=%s ",
			nfnl_inet_hook2str(msg->log_msg_hook,
					   buf, sizeof(buf)));

	if (msg->ce_mask & LOG_MSG_ATTR_MARK)
		nl_dump(p, "MARK=%u ", msg->log_msg_mark);

	if (msg->ce_mask & LOG_MSG_ATTR_PAYLOAD)
		nl_dump(p, "PAYLOADLEN=%d ", msg->log_msg_payload_len);

	if (msg->ce_mask & LOG_MSG_ATTR_UID)
		nl_dump(p, "UID=%u ", msg->log_msg_uid);

	if (msg->ce_mask & LOG_MSG_ATTR_GID)
		nl_dump(p, "GID=%u ", msg->log_msg_gid);

	if (msg->ce_mask & LOG_MSG_ATTR_SEQ)
		nl_dump(p, "SEQ=%d ", msg->log_msg_seq);

	if (msg->ce_mask & LOG_MSG_ATTR_SEQ_GLOBAL)
		nl_dump(p, "SEQGLOBAL=%d ", msg->log_msg_seq_global);

	nl_dump(p, "\n");

	if (link_cache)
		nl_cache_put(link_cache);
}

/**
 * @name Allocation/Freeing
 * @{
 */

struct nfnl_log_msg *nfnl_log_msg_alloc(void)
{
	return (struct nfnl_log_msg *) nl_object_alloc(&log_msg_obj_ops);
}

void nfnl_log_msg_get(struct nfnl_log_msg *msg)
{
	nl_object_get((struct nl_object *) msg);
}

void nfnl_log_msg_put(struct nfnl_log_msg *msg)
{
	nl_object_put((struct nl_object *) msg);
}

/** @} */

/**
 * @name Attributes
 * @{
 */

void nfnl_log_msg_set_family(struct nfnl_log_msg *msg, uint8_t family)
{
	msg->log_msg_family = family;
	msg->ce_mask |= LOG_MSG_ATTR_FAMILY;
}

uint8_t nfnl_log_msg_get_family(const struct nfnl_log_msg *msg)
{
	if (msg->ce_mask & LOG_MSG_ATTR_FAMILY)
		return msg->log_msg_family;
	else
		return AF_UNSPEC;
}

void nfnl_log_msg_set_hwproto(struct nfnl_log_msg *msg, uint16_t hwproto)
{
	msg->log_msg_hwproto = hwproto;
	msg->ce_mask |= LOG_MSG_ATTR_HWPROTO;
}

int nfnl_log_msg_test_hwproto(const struct nfnl_log_msg *msg)
{
	return !!(msg->ce_mask & LOG_MSG_ATTR_HWPROTO);
}

uint16_t nfnl_log_msg_get_hwproto(const struct nfnl_log_msg *msg)
{
	return msg->log_msg_hwproto;
}

void nfnl_log_msg_set_hook(struct nfnl_log_msg *msg, uint8_t hook)
{
	msg->log_msg_hook = hook;
	msg->ce_mask |= LOG_MSG_ATTR_HOOK;
}

int nfnl_log_msg_test_hook(const struct nfnl_log_msg *msg)
{
	return !!(msg->ce_mask & LOG_MSG_ATTR_HOOK);
}

uint8_t nfnl_log_msg_get_hook(const struct nfnl_log_msg *msg)
{
	return msg->log_msg_hook;
}

void nfnl_log_msg_set_mark(struct nfnl_log_msg *msg, uint32_t mark)
{
	msg->log_msg_mark = mark;
	msg->ce_mask |= LOG_MSG_ATTR_MARK;
}

int nfnl_log_msg_test_mark(const struct nfnl_log_msg *msg)
{
	return !!(msg->ce_mask & LOG_MSG_ATTR_MARK);
}

uint32_t nfnl_log_msg_get_mark(const struct nfnl_log_msg *msg)
{
	return msg->log_msg_mark;
}

void nfnl_log_msg_set_timestamp(struct nfnl_log_msg *msg, struct timeval *tv)
{
	msg->log_msg_timestamp.tv_sec = tv->tv_sec;
	msg->log_msg_timestamp.tv_usec = tv->tv_usec;
	msg->ce_mask |= LOG_MSG_ATTR_TIMESTAMP;
}

const struct timeval *nfnl_log_msg_get_timestamp(const struct nfnl_log_msg *msg)
{
	if (!(msg->ce_mask & LOG_MSG_ATTR_TIMESTAMP))
		return NULL;
	return &msg->log_msg_timestamp;
}

void nfnl_log_msg_set_indev(struct nfnl_log_msg *msg, uint32_t indev)
{
	msg->log_msg_indev = indev;
	msg->ce_mask |= LOG_MSG_ATTR_INDEV;
}

uint32_t nfnl_log_msg_get_indev(const struct nfnl_log_msg *msg)
{
	return msg->log_msg_indev;
}

void nfnl_log_msg_set_outdev(struct nfnl_log_msg *msg, uint32_t outdev)
{
	msg->log_msg_outdev = outdev;
	msg->ce_mask |= LOG_MSG_ATTR_OUTDEV;
}

uint32_t nfnl_log_msg_get_outdev(const struct nfnl_log_msg *msg)
{
	return msg->log_msg_outdev;
}

void nfnl_log_msg_set_physindev(struct nfnl_log_msg *msg, uint32_t physindev)
{
	msg->log_msg_physindev = physindev;
	msg->ce_mask |= LOG_MSG_ATTR_PHYSINDEV;
}

uint32_t nfnl_log_msg_get_physindev(const struct nfnl_log_msg *msg)
{
	return msg->log_msg_physindev;
}

void nfnl_log_msg_set_physoutdev(struct nfnl_log_msg *msg, uint32_t physoutdev)
{
	msg->log_msg_physoutdev = physoutdev;
	msg->ce_mask |= LOG_MSG_ATTR_PHYSOUTDEV;
}

uint32_t nfnl_log_msg_get_physoutdev(const struct nfnl_log_msg *msg)
{
	return msg->log_msg_physoutdev;
}

void nfnl_log_msg_set_hwaddr(struct nfnl_log_msg *msg, uint8_t *hwaddr, int len)
{
	if (len > sizeof(msg->log_msg_hwaddr))
		len = sizeof(msg->log_msg_hwaddr);
	msg->log_msg_hwaddr_len = len;
	memcpy(msg->log_msg_hwaddr, hwaddr, len);
	msg->ce_mask |= LOG_MSG_ATTR_HWADDR;
}

const uint8_t *nfnl_log_msg_get_hwaddr(const struct nfnl_log_msg *msg, int *len)
{
	if (!(msg->ce_mask & LOG_MSG_ATTR_HWADDR)) {
		*len = 0;
		return NULL;
	}

	*len = msg->log_msg_hwaddr_len;
	return msg->log_msg_hwaddr;
}

int nfnl_log_msg_set_payload(struct nfnl_log_msg *msg, uint8_t *payload, int len)
{
	free(msg->log_msg_payload);
	msg->log_msg_payload = malloc(len);
	if (!msg->log_msg_payload)
		return -NLE_NOMEM;

	memcpy(msg->log_msg_payload, payload, len);
	msg->log_msg_payload_len = len;
	msg->ce_mask |= LOG_MSG_ATTR_PAYLOAD;
	return 0;
}

const void *nfnl_log_msg_get_payload(const struct nfnl_log_msg *msg, int *len)
{
	if (!(msg->ce_mask & LOG_MSG_ATTR_PAYLOAD)) {
		*len = 0;
		return NULL;
	}

	*len = msg->log_msg_payload_len;
	return msg->log_msg_payload;
}

int nfnl_log_msg_set_prefix(struct nfnl_log_msg *msg, void *prefix)
{
	free(msg->log_msg_prefix);
	msg->log_msg_prefix = strdup(prefix);
	if (!msg->log_msg_prefix)
		return -NLE_NOMEM;

	msg->ce_mask |= LOG_MSG_ATTR_PREFIX;
	return 0;
}

const char *nfnl_log_msg_get_prefix(const struct nfnl_log_msg *msg)
{
	return msg->log_msg_prefix;
}

void nfnl_log_msg_set_uid(struct nfnl_log_msg *msg, uint32_t uid)
{
	msg->log_msg_uid = uid;
	msg->ce_mask |= LOG_MSG_ATTR_UID;
}

int nfnl_log_msg_test_uid(const struct nfnl_log_msg *msg)
{
	return !!(msg->ce_mask & LOG_MSG_ATTR_UID);
}

uint32_t nfnl_log_msg_get_uid(const struct nfnl_log_msg *msg)
{
	return msg->log_msg_uid;
}

void nfnl_log_msg_set_gid(struct nfnl_log_msg *msg, uint32_t gid)
{
	msg->log_msg_gid = gid;
	msg->ce_mask |= LOG_MSG_ATTR_GID;
}

int nfnl_log_msg_test_gid(const struct nfnl_log_msg *msg)
{
	return !!(msg->ce_mask & LOG_MSG_ATTR_GID);
}

uint32_t nfnl_log_msg_get_gid(const struct nfnl_log_msg *msg)
{
	return msg->log_msg_gid;
}


void nfnl_log_msg_set_seq(struct nfnl_log_msg *msg, uint32_t seq)
{
	msg->log_msg_seq = seq;
	msg->ce_mask |= LOG_MSG_ATTR_SEQ;
}

int nfnl_log_msg_test_seq(const struct nfnl_log_msg *msg)
{
	return !!(msg->ce_mask & LOG_MSG_ATTR_SEQ);
}

uint32_t nfnl_log_msg_get_seq(const struct nfnl_log_msg *msg)
{
	return msg->log_msg_seq;
}

void nfnl_log_msg_set_seq_global(struct nfnl_log_msg *msg, uint32_t seq_global)
{
	msg->log_msg_seq_global = seq_global;
	msg->ce_mask |= LOG_MSG_ATTR_SEQ_GLOBAL;
}

int nfnl_log_msg_test_seq_global(const struct nfnl_log_msg *msg)
{
	return !!(msg->ce_mask & LOG_MSG_ATTR_SEQ_GLOBAL);
}

uint32_t nfnl_log_msg_get_seq_global(const struct nfnl_log_msg *msg)
{
	return msg->log_msg_seq_global;
}

/** @} */

struct nl_object_ops log_msg_obj_ops = {
	.oo_name		= "netfilter/log_msg",
	.oo_size		= sizeof(struct nfnl_log_msg),
	.oo_free_data		= log_msg_free_data,
	.oo_clone		= log_msg_clone,
	.oo_dump = {
	    [NL_DUMP_LINE]	= log_msg_dump,
	    [NL_DUMP_DETAILS]	= log_msg_dump,
	    [NL_DUMP_STATS]	= log_msg_dump,
	},
};

/** @} */

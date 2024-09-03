/*
 * lib/netfilter/queue_msg_obj.c	Netfilter Queue Message Object
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2007, 2008 Patrick McHardy <kaber@trash.net>
 */

#include <netlink-private/netlink.h>
#include <netlink/netfilter/nfnl.h>
#include <netlink/netfilter/netfilter.h>
#include <netlink/netfilter/queue_msg.h>
#include <linux/netfilter.h>

/** @cond SKIP */
#define QUEUE_MSG_ATTR_GROUP		(1UL << 0)
#define QUEUE_MSG_ATTR_FAMILY		(1UL << 1)
#define QUEUE_MSG_ATTR_PACKETID		(1UL << 2)
#define QUEUE_MSG_ATTR_HWPROTO		(1UL << 3)
#define QUEUE_MSG_ATTR_HOOK		(1UL << 4)
#define QUEUE_MSG_ATTR_MARK		(1UL << 5)
#define QUEUE_MSG_ATTR_TIMESTAMP	(1UL << 6)
#define QUEUE_MSG_ATTR_INDEV		(1UL << 7)
#define QUEUE_MSG_ATTR_OUTDEV		(1UL << 8)
#define QUEUE_MSG_ATTR_PHYSINDEV	(1UL << 9)
#define QUEUE_MSG_ATTR_PHYSOUTDEV	(1UL << 10)
#define QUEUE_MSG_ATTR_HWADDR		(1UL << 11)
#define QUEUE_MSG_ATTR_PAYLOAD		(1UL << 12)
#define QUEUE_MSG_ATTR_VERDICT		(1UL << 13)
/** @endcond */

static void nfnl_queue_msg_free_data(struct nl_object *c)
{
	struct nfnl_queue_msg *msg = (struct nfnl_queue_msg *) c;

	if (msg == NULL)
		return;

	free(msg->queue_msg_payload);
}

static int nfnl_queue_msg_clone(struct nl_object *_dst, struct nl_object *_src)
{
	struct nfnl_queue_msg *dst = (struct nfnl_queue_msg *) _dst;
	struct nfnl_queue_msg *src = (struct nfnl_queue_msg *) _src;
	int err;

	if (src->queue_msg_payload) {
		err = nfnl_queue_msg_set_payload(dst, src->queue_msg_payload,
						 src->queue_msg_payload_len);
		if (err < 0)
			goto errout;
	}

	return 0;
errout:
	return err;
}

static void nfnl_queue_msg_dump(struct nl_object *a, struct nl_dump_params *p)
{
	struct nfnl_queue_msg *msg = (struct nfnl_queue_msg *) a;
	struct nl_cache *link_cache;
	char buf[64];

	link_cache = nl_cache_mngt_require_safe("route/link");

	nl_new_line(p);

	if (msg->ce_mask & QUEUE_MSG_ATTR_GROUP)
		nl_dump(p, "GROUP=%u ", msg->queue_msg_group);

	if (msg->ce_mask & QUEUE_MSG_ATTR_INDEV) {
		if (link_cache)
			nl_dump(p, "IN=%s ",
				rtnl_link_i2name(link_cache,
						 msg->queue_msg_indev,
						 buf, sizeof(buf)));
		else
			nl_dump(p, "IN=%d ", msg->queue_msg_indev);
	}

	if (msg->ce_mask & QUEUE_MSG_ATTR_PHYSINDEV) {
		if (link_cache)
			nl_dump(p, "PHYSIN=%s ",
				rtnl_link_i2name(link_cache,
						 msg->queue_msg_physindev,
						 buf, sizeof(buf)));
		else
			nl_dump(p, "IN=%d ", msg->queue_msg_physindev);
	}

	if (msg->ce_mask & QUEUE_MSG_ATTR_OUTDEV) {
		if (link_cache)
			nl_dump(p, "OUT=%s ",
				rtnl_link_i2name(link_cache,
						 msg->queue_msg_outdev,
						 buf, sizeof(buf)));
		else
			nl_dump(p, "OUT=%d ", msg->queue_msg_outdev);
	}

	if (msg->ce_mask & QUEUE_MSG_ATTR_PHYSOUTDEV) {
		if (link_cache)
			nl_dump(p, "PHYSOUT=%s ",
				rtnl_link_i2name(link_cache,
						 msg->queue_msg_physoutdev,
						 buf, sizeof(buf)));
		else
			nl_dump(p, "PHYSOUT=%d ", msg->queue_msg_physoutdev);
	}

	if (msg->ce_mask & QUEUE_MSG_ATTR_HWADDR) {
		int i;

		nl_dump(p, "MAC");
		for (i = 0; i < msg->queue_msg_hwaddr_len; i++)
			nl_dump(p, "%c%02x", i?':':'=',
				msg->queue_msg_hwaddr[i]);
		nl_dump(p, " ");
	}

	if (msg->ce_mask & QUEUE_MSG_ATTR_FAMILY)
		nl_dump(p, "FAMILY=%s ",
			nl_af2str(msg->queue_msg_family, buf, sizeof(buf)));

	if (msg->ce_mask & QUEUE_MSG_ATTR_HWPROTO)
		nl_dump(p, "HWPROTO=%s ",
			nl_ether_proto2str(ntohs(msg->queue_msg_hwproto),
					   buf, sizeof(buf)));

	if (msg->ce_mask & QUEUE_MSG_ATTR_HOOK)
		nl_dump(p, "HOOK=%s ",
			nfnl_inet_hook2str(msg->queue_msg_hook,
					   buf, sizeof(buf)));

	if (msg->ce_mask & QUEUE_MSG_ATTR_MARK)
		nl_dump(p, "MARK=%d ", msg->queue_msg_mark);

	if (msg->ce_mask & QUEUE_MSG_ATTR_PAYLOAD)
		nl_dump(p, "PAYLOADLEN=%d ", msg->queue_msg_payload_len);

	if (msg->ce_mask & QUEUE_MSG_ATTR_PACKETID)
		nl_dump(p, "PACKETID=%u ", msg->queue_msg_packetid);

	if (msg->ce_mask & QUEUE_MSG_ATTR_VERDICT)
		nl_dump(p, "VERDICT=%s ",
			nfnl_verdict2str(msg->queue_msg_verdict,
					 buf, sizeof(buf)));

	nl_dump(p, "\n");

	if (link_cache)
		nl_cache_put(link_cache);
}

/**
 * @name Allocation/Freeing
 * @{
 */

struct nfnl_queue_msg *nfnl_queue_msg_alloc(void)
{
	return (struct nfnl_queue_msg *) nl_object_alloc(&queue_msg_obj_ops);
}

void nfnl_queue_msg_get(struct nfnl_queue_msg *msg)
{
	nl_object_get((struct nl_object *) msg);
}

void nfnl_queue_msg_put(struct nfnl_queue_msg *msg)
{
	nl_object_put((struct nl_object *) msg);
}

/** @} */

/**
 * @name Attributes
 * @{
 */

void nfnl_queue_msg_set_group(struct nfnl_queue_msg *msg, uint16_t group)
{
	msg->queue_msg_group = group;
	msg->ce_mask |= QUEUE_MSG_ATTR_GROUP;
}

int nfnl_queue_msg_test_group(const struct nfnl_queue_msg *msg)
{
	return !!(msg->ce_mask & QUEUE_MSG_ATTR_GROUP);
}

uint16_t nfnl_queue_msg_get_group(const struct nfnl_queue_msg *msg)
{
	return msg->queue_msg_group;
}

/**
* Set the protocol family
* @arg msg         NF queue message
* @arg family      AF_XXX  address family  example: AF_INET, AF_UNIX, etc
*/
void nfnl_queue_msg_set_family(struct nfnl_queue_msg *msg, uint8_t family)
{
	msg->queue_msg_family = family;
	msg->ce_mask |= QUEUE_MSG_ATTR_FAMILY;
}

int nfnl_queue_msg_test_family(const struct nfnl_queue_msg *msg)
{
	return !!(msg->ce_mask & QUEUE_MSG_ATTR_FAMILY);
}

uint8_t nfnl_queue_msg_get_family(const struct nfnl_queue_msg *msg)
{
	if (msg->ce_mask & QUEUE_MSG_ATTR_FAMILY)
		return msg->queue_msg_family;
	else
		return AF_UNSPEC;
}

void nfnl_queue_msg_set_packetid(struct nfnl_queue_msg *msg, uint32_t packetid)
{
	msg->queue_msg_packetid = packetid;
	msg->ce_mask |= QUEUE_MSG_ATTR_PACKETID;
}

int nfnl_queue_msg_test_packetid(const struct nfnl_queue_msg *msg)
{
	return !!(msg->ce_mask & QUEUE_MSG_ATTR_PACKETID);
}

uint32_t nfnl_queue_msg_get_packetid(const struct nfnl_queue_msg *msg)
{
	return msg->queue_msg_packetid;
}

void nfnl_queue_msg_set_hwproto(struct nfnl_queue_msg *msg, uint16_t hwproto)
{
	msg->queue_msg_hwproto = hwproto;
	msg->ce_mask |= QUEUE_MSG_ATTR_HWPROTO;
}

int nfnl_queue_msg_test_hwproto(const struct nfnl_queue_msg *msg)
{
	return !!(msg->ce_mask & QUEUE_MSG_ATTR_HWPROTO);
}

uint16_t nfnl_queue_msg_get_hwproto(const struct nfnl_queue_msg *msg)
{
	return msg->queue_msg_hwproto;
}

void nfnl_queue_msg_set_hook(struct nfnl_queue_msg *msg, uint8_t hook)
{
	msg->queue_msg_hook = hook;
	msg->ce_mask |= QUEUE_MSG_ATTR_HOOK;
}

int nfnl_queue_msg_test_hook(const struct nfnl_queue_msg *msg)
{
	return !!(msg->ce_mask & QUEUE_MSG_ATTR_HOOK);
}

uint8_t nfnl_queue_msg_get_hook(const struct nfnl_queue_msg *msg)
{
	return msg->queue_msg_hook;
}

void nfnl_queue_msg_set_mark(struct nfnl_queue_msg *msg, uint32_t mark)
{
	msg->queue_msg_mark = mark;
	msg->ce_mask |= QUEUE_MSG_ATTR_MARK;
}

int nfnl_queue_msg_test_mark(const struct nfnl_queue_msg *msg)
{
	return !!(msg->ce_mask & QUEUE_MSG_ATTR_MARK);
}

uint32_t nfnl_queue_msg_get_mark(const struct nfnl_queue_msg *msg)
{
	return msg->queue_msg_mark;
}

void nfnl_queue_msg_set_timestamp(struct nfnl_queue_msg *msg,
				  struct timeval *tv)
{
	msg->queue_msg_timestamp.tv_sec = tv->tv_sec;
	msg->queue_msg_timestamp.tv_usec = tv->tv_usec;
	msg->ce_mask |= QUEUE_MSG_ATTR_TIMESTAMP;
}

int nfnl_queue_msg_test_timestamp(const struct nfnl_queue_msg *msg)
{
	return !!(msg->ce_mask & QUEUE_MSG_ATTR_TIMESTAMP);
}

const struct timeval *nfnl_queue_msg_get_timestamp(const struct nfnl_queue_msg *msg)
{
	if (!(msg->ce_mask & QUEUE_MSG_ATTR_TIMESTAMP))
		return NULL;
	return &msg->queue_msg_timestamp;
}

void nfnl_queue_msg_set_indev(struct nfnl_queue_msg *msg, uint32_t indev)
{
	msg->queue_msg_indev = indev;
	msg->ce_mask |= QUEUE_MSG_ATTR_INDEV;
}

int nfnl_queue_msg_test_indev(const struct nfnl_queue_msg *msg)
{
	return !!(msg->ce_mask & QUEUE_MSG_ATTR_INDEV);
}

uint32_t nfnl_queue_msg_get_indev(const struct nfnl_queue_msg *msg)
{
	return msg->queue_msg_indev;
}

void nfnl_queue_msg_set_outdev(struct nfnl_queue_msg *msg, uint32_t outdev)
{
	msg->queue_msg_outdev = outdev;
	msg->ce_mask |= QUEUE_MSG_ATTR_OUTDEV;
}

int nfnl_queue_msg_test_outdev(const struct nfnl_queue_msg *msg)
{
	return !!(msg->ce_mask & QUEUE_MSG_ATTR_OUTDEV);
}

uint32_t nfnl_queue_msg_get_outdev(const struct nfnl_queue_msg *msg)
{
	return msg->queue_msg_outdev;
}

void nfnl_queue_msg_set_physindev(struct nfnl_queue_msg *msg,
				  uint32_t physindev)
{
	msg->queue_msg_physindev = physindev;
	msg->ce_mask |= QUEUE_MSG_ATTR_PHYSINDEV;
}

int nfnl_queue_msg_test_physindev(const struct nfnl_queue_msg *msg)
{
	return !!(msg->ce_mask & QUEUE_MSG_ATTR_PHYSINDEV);
}

uint32_t nfnl_queue_msg_get_physindev(const struct nfnl_queue_msg *msg)
{
	return msg->queue_msg_physindev;
}

void nfnl_queue_msg_set_physoutdev(struct nfnl_queue_msg *msg,
				   uint32_t physoutdev)
{
	msg->queue_msg_physoutdev = physoutdev;
	msg->ce_mask |= QUEUE_MSG_ATTR_PHYSOUTDEV;
}

int nfnl_queue_msg_test_physoutdev(const struct nfnl_queue_msg *msg)
{
	return !!(msg->ce_mask & QUEUE_MSG_ATTR_PHYSOUTDEV);
}

uint32_t nfnl_queue_msg_get_physoutdev(const struct nfnl_queue_msg *msg)
{
	return msg->queue_msg_physoutdev;
}

void nfnl_queue_msg_set_hwaddr(struct nfnl_queue_msg *msg, uint8_t *hwaddr,
			       int len)
{
	if (len > sizeof(msg->queue_msg_hwaddr))
		len = sizeof(msg->queue_msg_hwaddr);

	msg->queue_msg_hwaddr_len = len;
	memcpy(msg->queue_msg_hwaddr, hwaddr, len);
	msg->ce_mask |= QUEUE_MSG_ATTR_HWADDR;
}

int nfnl_queue_msg_test_hwaddr(const struct nfnl_queue_msg *msg)
{
	return !!(msg->ce_mask & QUEUE_MSG_ATTR_HWADDR);
}

const uint8_t *nfnl_queue_msg_get_hwaddr(const struct nfnl_queue_msg *msg,
					 int *len)
{
	if (!(msg->ce_mask & QUEUE_MSG_ATTR_HWADDR)) {
		*len = 0;
		return NULL;
	}

	*len = msg->queue_msg_hwaddr_len;
	return msg->queue_msg_hwaddr;
}

int nfnl_queue_msg_set_payload(struct nfnl_queue_msg *msg, uint8_t *payload,
			       int len)
{
	free(msg->queue_msg_payload);
	msg->queue_msg_payload = malloc(len);
	if (!msg->queue_msg_payload)
		return -NLE_NOMEM;

	memcpy(msg->queue_msg_payload, payload, len);
	msg->queue_msg_payload_len = len;
	msg->ce_mask |= QUEUE_MSG_ATTR_PAYLOAD;
	return 0;
}

int nfnl_queue_msg_test_payload(const struct nfnl_queue_msg *msg)
{
	return !!(msg->ce_mask & QUEUE_MSG_ATTR_PAYLOAD);
}

const void *nfnl_queue_msg_get_payload(const struct nfnl_queue_msg *msg, int *len)
{
	if (!(msg->ce_mask & QUEUE_MSG_ATTR_PAYLOAD)) {
		*len = 0;
		return NULL;
	}

	*len = msg->queue_msg_payload_len;
	return msg->queue_msg_payload;
}

/**
* Return the number of items matching a filter in the cache
* @arg msg        queue msg
* @arg verdict    NF_DROP, NF_ACCEPT, NF_REPEAT, etc
*/
void nfnl_queue_msg_set_verdict(struct nfnl_queue_msg *msg,
				unsigned int verdict)
{
	msg->queue_msg_verdict = verdict;
	msg->ce_mask |= QUEUE_MSG_ATTR_VERDICT;
}

int nfnl_queue_msg_test_verdict(const struct nfnl_queue_msg *msg)
{
	return !!(msg->ce_mask & QUEUE_MSG_ATTR_VERDICT);
}

unsigned int nfnl_queue_msg_get_verdict(const struct nfnl_queue_msg *msg)
{
	return msg->queue_msg_verdict;
}

static const struct trans_tbl nfnl_queue_msg_attrs[] = {
	__ADD(QUEUE_MSG_ATTR_GROUP,		group)
	__ADD(QUEUE_MSG_ATTR_FAMILY,		family)
	__ADD(QUEUE_MSG_ATTR_PACKETID,		packetid)
	__ADD(QUEUE_MSG_ATTR_HWPROTO,		hwproto)
	__ADD(QUEUE_MSG_ATTR_HOOK,		hook)
	__ADD(QUEUE_MSG_ATTR_MARK,		mark)
	__ADD(QUEUE_MSG_ATTR_TIMESTAMP,		timestamp)
	__ADD(QUEUE_MSG_ATTR_INDEV,		indev)
	__ADD(QUEUE_MSG_ATTR_OUTDEV,		outdev)
	__ADD(QUEUE_MSG_ATTR_PHYSINDEV,		physindev)
	__ADD(QUEUE_MSG_ATTR_PHYSOUTDEV,	physoutdev)
	__ADD(QUEUE_MSG_ATTR_HWADDR,		hwaddr)
	__ADD(QUEUE_MSG_ATTR_PAYLOAD,		payload)
	__ADD(QUEUE_MSG_ATTR_VERDICT,		verdict)
};

static char *nfnl_queue_msg_attrs2str(int attrs, char *buf, size_t len)
{
	return __flags2str(attrs, buf, len, nfnl_queue_msg_attrs,
			   ARRAY_SIZE(nfnl_queue_msg_attrs));
}

/** @} */

struct nl_object_ops queue_msg_obj_ops = {
	.oo_name		= "netfilter/queuemsg",
	.oo_size		= sizeof(struct nfnl_queue_msg),
	.oo_free_data		= nfnl_queue_msg_free_data,
	.oo_clone		= nfnl_queue_msg_clone,
	.oo_dump = {
	    [NL_DUMP_LINE]	= nfnl_queue_msg_dump,
	    [NL_DUMP_DETAILS]	= nfnl_queue_msg_dump,
	    [NL_DUMP_STATS]	= nfnl_queue_msg_dump,
	},
	.oo_attrs2str		= nfnl_queue_msg_attrs2str,
};

/** @} */

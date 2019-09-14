/*
 * lib/netfilter/queue_obj.c	Netfilter Queue
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2007, 2008 Patrick McHardy <kaber@trash.net>
 */

/**
 * @ingroup nfnl
 * @defgroup queue Queue
 * @brief
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink/netfilter/nfnl.h>
#include <netlink/netfilter/queue.h>

/** @cond SKIP */
#define QUEUE_ATTR_GROUP		(1UL << 0)
#define QUEUE_ATTR_MAXLEN		(1UL << 1)
#define QUEUE_ATTR_COPY_MODE		(1UL << 2)
#define QUEUE_ATTR_COPY_RANGE		(1UL << 3)
/** @endcond */


static void nfnl_queue_dump(struct nl_object *a, struct nl_dump_params *p)
{
	struct nfnl_queue *queue = (struct nfnl_queue *) a;
	char buf[64];

	nl_new_line(p);

	if (queue->ce_mask & QUEUE_ATTR_GROUP)
		nl_dump(p, "group=%u ", queue->queue_group);

	if (queue->ce_mask & QUEUE_ATTR_MAXLEN)
		nl_dump(p, "maxlen=%u ", queue->queue_maxlen);

	if (queue->ce_mask & QUEUE_ATTR_COPY_MODE)
		nl_dump(p, "copy_mode=%s ",
			nfnl_queue_copy_mode2str(queue->queue_copy_mode,
						 buf, sizeof(buf)));

	if (queue->ce_mask & QUEUE_ATTR_COPY_RANGE)
		nl_dump(p, "copy_range=%u ", queue->queue_copy_range);

	nl_dump(p, "\n");
}

static const struct trans_tbl copy_modes[] = {
	__ADD(NFNL_QUEUE_COPY_NONE,	none)
	__ADD(NFNL_QUEUE_COPY_META,	meta)
	__ADD(NFNL_QUEUE_COPY_PACKET,	packet)
};

char *nfnl_queue_copy_mode2str(enum nfnl_queue_copy_mode copy_mode, char *buf,
			       size_t len)
{
	return __type2str(copy_mode, buf, len, copy_modes,
			   ARRAY_SIZE(copy_modes));
}

enum nfnl_queue_copy_mode nfnl_queue_str2copy_mode(const char *name)
{
	return __str2type(name, copy_modes, ARRAY_SIZE(copy_modes));
}

/**
 * @name Allocation/Freeing
 * @{
 */

struct nfnl_queue *nfnl_queue_alloc(void)
{
	return (struct nfnl_queue *) nl_object_alloc(&queue_obj_ops);
}

void nfnl_queue_get(struct nfnl_queue *queue)
{
	nl_object_get((struct nl_object *) queue);
}

void nfnl_queue_put(struct nfnl_queue *queue)
{
	nl_object_put((struct nl_object *) queue);
}

/** @} */

/**
 * @name Attributes
 * @{
 */

void nfnl_queue_set_group(struct nfnl_queue *queue, uint16_t group)
{
	queue->queue_group = group;
	queue->ce_mask |= QUEUE_ATTR_GROUP;
}

int nfnl_queue_test_group(const struct nfnl_queue *queue)
{
	return !!(queue->ce_mask & QUEUE_ATTR_GROUP);
}

uint16_t nfnl_queue_get_group(const struct nfnl_queue *queue)
{
	return queue->queue_group;
}

void nfnl_queue_set_maxlen(struct nfnl_queue *queue, uint32_t maxlen)
{
	queue->queue_maxlen = maxlen;
	queue->ce_mask |= QUEUE_ATTR_MAXLEN;
}

int nfnl_queue_test_maxlen(const struct nfnl_queue *queue)
{
	return !!(queue->ce_mask & QUEUE_ATTR_MAXLEN);
}

uint32_t nfnl_queue_get_maxlen(const struct nfnl_queue *queue)
{
	return queue->queue_maxlen;
}

void nfnl_queue_set_copy_mode(struct nfnl_queue *queue, enum nfnl_queue_copy_mode mode)
{
	queue->queue_copy_mode = mode;
	queue->ce_mask |= QUEUE_ATTR_COPY_MODE;
}

int nfnl_queue_test_copy_mode(const struct nfnl_queue *queue)
{
	return !!(queue->ce_mask & QUEUE_ATTR_COPY_MODE);
}

enum nfnl_queue_copy_mode nfnl_queue_get_copy_mode(const struct nfnl_queue *queue)
{
	return queue->queue_copy_mode;
}

void nfnl_queue_set_copy_range(struct nfnl_queue *queue, uint32_t copy_range)
{
	queue->queue_copy_range = copy_range;
	queue->ce_mask |= QUEUE_ATTR_COPY_RANGE;
}

int nfnl_queue_test_copy_range(const struct nfnl_queue *queue)
{
	return !!(queue->ce_mask & QUEUE_ATTR_COPY_RANGE);
}

uint32_t nfnl_queue_get_copy_range(const struct nfnl_queue *queue)
{
	return queue->queue_copy_range;
}

static int nfnl_queue_compare(struct nl_object *_a, struct nl_object *_b,
			      uint32_t attrs, int flags)
{
	struct nfnl_queue *a = (struct nfnl_queue *) _a;
	struct nfnl_queue *b = (struct nfnl_queue *) _b;
	int diff = 0;

#define NFNL_QUEUE_DIFF(ATTR, EXPR) \
	ATTR_DIFF(attrs, QUEUE_ATTR_##ATTR, a, b, EXPR)
#define NFNL_QUEUE_DIFF_VAL(ATTR, FIELD) \
	NFNL_QUEUE_DIFF(ATTR, a->FIELD != b->FIELD)

	diff |= NFNL_QUEUE_DIFF_VAL(GROUP,	queue_group);
	diff |= NFNL_QUEUE_DIFF_VAL(MAXLEN,	queue_maxlen);
	diff |= NFNL_QUEUE_DIFF_VAL(COPY_MODE,	queue_copy_mode);
	diff |= NFNL_QUEUE_DIFF_VAL(COPY_RANGE,	queue_copy_range);

#undef NFNL_QUEUE_DIFF
#undef NFNL_QUEUE_DIFF_VAL

	return diff;
}

static const struct trans_tbl nfnl_queue_attrs[] = {
	__ADD(QUEUE_ATTR_GROUP,		group)
	__ADD(QUEUE_ATTR_MAXLEN,	maxlen)
	__ADD(QUEUE_ATTR_COPY_MODE,	copy_mode)
	__ADD(QUEUE_ATTR_COPY_RANGE,	copy_range)
};

static char *nfnl_queue_attrs2str(int attrs, char *buf, size_t len)
{
	return __flags2str(attrs, buf, len, nfnl_queue_attrs,
			   ARRAY_SIZE(nfnl_queue_attrs));
}

/** @} */

struct nl_object_ops queue_obj_ops = {
	.oo_name		= "netfilter/queue",
	.oo_size		= sizeof(struct nfnl_queue),
	.oo_dump = {
	    [NL_DUMP_LINE]	= nfnl_queue_dump,
	    [NL_DUMP_DETAILS]	= nfnl_queue_dump,
	    [NL_DUMP_STATS]	= nfnl_queue_dump,
	},
	.oo_compare		= nfnl_queue_compare,
	.oo_attrs2str		= nfnl_queue_attrs2str,
	.oo_id_attrs		= QUEUE_ATTR_GROUP,
};

/** @} */

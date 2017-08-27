/*
 * lib/netfilter/log_obj.c	Netfilter Log Object
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2007 Philip Craig <philipc@snapgear.com>
 * Copyright (c) 2007 Secure Computing Corporation
 * Copyright (c) 2008 Patrick McHardy <kaber@trash.net>
 */

#include <netlink-local.h>
#include <netlink/netfilter/nfnl.h>
#include <netlink/netfilter/log.h>

/** @cond SKIP */
#define LOG_ATTR_GROUP			(1UL << 0)
#define LOG_ATTR_COPY_MODE		(1UL << 1)
#define LOG_ATTR_COPY_RANGE		(1UL << 3)
#define LOG_ATTR_FLUSH_TIMEOUT		(1UL << 4)
#define LOG_ATTR_ALLOC_SIZE		(1UL << 5)
#define LOG_ATTR_QUEUE_THRESHOLD	(1UL << 6)

/** @endcond */

static void nfnl_log_dump(struct nl_object *a, struct nl_dump_params *p)
{
	struct nfnl_log *log = (struct nfnl_log *) a;
	char buf[64];

	nl_new_line(p);

	if (log->ce_mask & LOG_ATTR_GROUP)
		nl_dump(p, "group=%u ", log->log_group);

	if (log->ce_mask & LOG_ATTR_COPY_MODE)
		nl_dump(p, "copy_mode=%s ",
			nfnl_log_copy_mode2str(log->log_copy_mode,
					       buf, sizeof(buf)));

	if (log->ce_mask & LOG_ATTR_COPY_RANGE)
		nl_dump(p, "copy_range=%u ", log->log_copy_range);

	if (log->ce_mask & LOG_ATTR_FLUSH_TIMEOUT)
		nl_dump(p, "flush_timeout=%u ", log->log_flush_timeout);

	if (log->ce_mask & LOG_ATTR_ALLOC_SIZE)
		nl_dump(p, "alloc_size=%u ", log->log_alloc_size);

	if (log->ce_mask & LOG_ATTR_QUEUE_THRESHOLD)
		nl_dump(p, "queue_threshold=%u ", log->log_queue_threshold);

	nl_dump(p, "\n");
}

static const struct trans_tbl copy_modes[] = {
	__ADD(NFNL_LOG_COPY_NONE,	none)
	__ADD(NFNL_LOG_COPY_META,	meta)
	__ADD(NFNL_LOG_COPY_PACKET,	packet)
};

char *nfnl_log_copy_mode2str(enum nfnl_log_copy_mode copy_mode, char *buf,
			     size_t len)
{
	return __type2str(copy_mode, buf, len, copy_modes,
			  ARRAY_SIZE(copy_modes));
}

enum nfnl_log_copy_mode nfnl_log_str2copy_mode(const char *name)
{
	return __str2type(name, copy_modes, ARRAY_SIZE(copy_modes));
}

/**
 * @name Allocation/Freeing
 * @{
 */

struct nfnl_log *nfnl_log_alloc(void)
{
	return (struct nfnl_log *) nl_object_alloc(&log_obj_ops);
}

void nfnl_log_get(struct nfnl_log *log)
{
	nl_object_get((struct nl_object *) log);
}

void nfnl_log_put(struct nfnl_log *log)
{
	nl_object_put((struct nl_object *) log);
}

/** @} */

/**
 * @name Attributes
 * @{
 */

void nfnl_log_set_group(struct nfnl_log *log, uint16_t group)
{
	log->log_group = group;
	log->ce_mask |= LOG_ATTR_GROUP;
}

int nfnl_log_test_group(const struct nfnl_log *log)
{
	return !!(log->ce_mask & LOG_ATTR_GROUP);
}

uint16_t nfnl_log_get_group(const struct nfnl_log *log)
{
	return log->log_group;
}

void nfnl_log_set_copy_mode(struct nfnl_log *log, enum nfnl_log_copy_mode mode)
{
	log->log_copy_mode = mode;
	log->ce_mask |= LOG_ATTR_COPY_MODE;
}

int nfnl_log_test_copy_mode(const struct nfnl_log *log)
{
	return !!(log->ce_mask & LOG_ATTR_COPY_MODE);
}

enum nfnl_log_copy_mode nfnl_log_get_copy_mode(const struct nfnl_log *log)
{
	return log->log_copy_mode;
}

void nfnl_log_set_copy_range(struct nfnl_log *log, uint32_t copy_range)
{
	log->log_copy_range = copy_range;
	log->ce_mask |= LOG_ATTR_COPY_RANGE;
}

int nfnl_log_test_copy_range(const struct nfnl_log *log)
{
	return !!(log->ce_mask & LOG_ATTR_COPY_RANGE);
}

uint32_t nfnl_log_get_copy_range(const struct nfnl_log *log)
{
	return log->log_copy_range;
}

void nfnl_log_set_flush_timeout(struct nfnl_log *log, uint32_t timeout)
{
	log->log_flush_timeout = timeout;
	log->ce_mask |= LOG_ATTR_FLUSH_TIMEOUT;
}

int nfnl_log_test_flush_timeout(const struct nfnl_log *log)
{
	return !!(log->ce_mask & LOG_ATTR_FLUSH_TIMEOUT);
}

uint32_t nfnl_log_get_flush_timeout(const struct nfnl_log *log)
{
	return log->log_flush_timeout;
}

void nfnl_log_set_alloc_size(struct nfnl_log *log, uint32_t alloc_size)
{
	log->log_alloc_size = alloc_size;
	log->ce_mask |= LOG_ATTR_ALLOC_SIZE;
}

int nfnl_log_test_alloc_size(const struct nfnl_log *log)
{
	return !!(log->ce_mask & LOG_ATTR_ALLOC_SIZE);
}

uint32_t nfnl_log_get_alloc_size(const struct nfnl_log *log)
{
	return log->log_alloc_size;
}

void nfnl_log_set_queue_threshold(struct nfnl_log *log, uint32_t threshold)
{
	log->log_queue_threshold = threshold;
	log->ce_mask |= LOG_ATTR_QUEUE_THRESHOLD;
}

int nfnl_log_test_queue_threshold(const struct nfnl_log *log)
{
	return !!(log->ce_mask & LOG_ATTR_QUEUE_THRESHOLD);
}

uint32_t nfnl_log_get_queue_threshold(const struct nfnl_log *log)
{
	return log->log_queue_threshold;
}

/* We don't actually use the flags for anything yet since the
 * nfnetlog_log interface truly sucks - it only contains the
 * flag value, but not mask, so we would have to make assumptions
 * about the supported flags.
 */
void nfnl_log_set_flags(struct nfnl_log *log, unsigned int flags)
{
	log->log_flags |= flags;
	log->log_flag_mask |= flags;
}

void nfnl_log_unset_flags(struct nfnl_log *log, unsigned int flags)
{
	log->log_flags &= ~flags;
	log->log_flag_mask |= flags;
}

static const struct trans_tbl log_flags[] = {
	__ADD(NFNL_LOG_FLAG_SEQ,	seq)
	__ADD(NFNL_LOG_FLAG_SEQ_GLOBAL,	seq_global)
};

char *nfnl_log_flags2str(unsigned int flags, char *buf, size_t len)
{
	return __flags2str(flags, buf, len, log_flags, ARRAY_SIZE(log_flags));
}

unsigned int nfnl_log_str2flags(const char *name)
{
	return __str2flags(name, log_flags, ARRAY_SIZE(log_flags));
}

static int nfnl_log_compare(struct nl_object *_a, struct nl_object *_b,
			    uint32_t attrs, int flags)
{
	struct nfnl_log *a = (struct nfnl_log *) _a;
	struct nfnl_log *b = (struct nfnl_log *) _b;
	int diff = 0;

#define NFNL_LOG_DIFF(ATTR, EXPR) \
	ATTR_DIFF(attrs, LOG_ATTR_##ATTR, a, b, EXPR)
#define NFNL_LOG_DIFF_VAL(ATTR, FIELD) \
	NFNL_LOG_DIFF(ATTR, a->FIELD != b->FIELD)

	diff |= NFNL_LOG_DIFF_VAL(GROUP,		log_group);
	diff |= NFNL_LOG_DIFF_VAL(COPY_MODE,		log_copy_mode);
	diff |= NFNL_LOG_DIFF_VAL(COPY_RANGE,		log_copy_range);
	diff |= NFNL_LOG_DIFF_VAL(FLUSH_TIMEOUT,	log_flush_timeout);
	diff |= NFNL_LOG_DIFF_VAL(ALLOC_SIZE,		log_alloc_size);
	diff |= NFNL_LOG_DIFF_VAL(QUEUE_THRESHOLD,	log_queue_threshold);

#undef NFNL_LOG_DIFF
#undef NFNL_LOG_DIFF_VAL

	return diff;
}

static const struct trans_tbl nfnl_log_attrs[] = {
	__ADD(LOG_ATTR_GROUP,		group)
	__ADD(LOG_ATTR_COPY_MODE,	copy_mode)
	__ADD(LOG_ATTR_COPY_RANGE,	copy_range)
	__ADD(LOG_ATTR_FLUSH_TIMEOUT,	flush_timeout)
	__ADD(LOG_ATTR_ALLOC_SIZE,	alloc_size)
	__ADD(LOG_ATTR_QUEUE_THRESHOLD, queue_threshold)
};

static char *nfnl_log_attrs2str(int attrs, char *buf, size_t len)
{
	return __flags2str(attrs, buf, len, nfnl_log_attrs,
			   ARRAY_SIZE(nfnl_log_attrs));
}

/** @} */

struct nl_object_ops log_obj_ops = {
	.oo_name		= "netfilter/log",
	.oo_size		= sizeof(struct nfnl_log),
	.oo_dump = {
	    [NL_DUMP_LINE]	= nfnl_log_dump,
	    [NL_DUMP_DETAILS]	= nfnl_log_dump,
	    [NL_DUMP_STATS]	= nfnl_log_dump,
	},
	.oo_compare		= nfnl_log_compare,
	.oo_attrs2str		= nfnl_log_attrs2str,
	.oo_id_attrs		= LOG_ATTR_GROUP,
};

/** @} */

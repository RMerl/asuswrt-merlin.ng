/*
 * lib/fib_lookup/request.c	FIB Lookup Request
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup fib_lookup
 * @defgroup flreq Request
 * @brief
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink/netlink.h>
#include <netlink/attr.h>
#include <netlink/utils.h>
#include <netlink/object.h>
#include <netlink/fib_lookup/request.h>

static struct nl_object_ops request_obj_ops;

/** @cond SKIP */
#define REQUEST_ATTR_ADDR	0x01
#define REQUEST_ATTR_FWMARK	0x02
#define REQUEST_ATTR_TOS	0x04
#define REQUEST_ATTR_SCOPE	0x08
#define REQUEST_ATTR_TABLE	0x10
/** @endcond */

static void request_free_data(struct nl_object *obj)
{
	struct flnl_request *req = REQUEST_CAST(obj);

	if (req)
		nl_addr_put(req->lr_addr);
}

static int request_clone(struct nl_object *_dst, struct nl_object *_src)
{
	struct flnl_request *dst = nl_object_priv(_dst);
	struct flnl_request *src = nl_object_priv(_src);

	if (src->lr_addr)
		if (!(dst->lr_addr = nl_addr_clone(src->lr_addr)))
			return -NLE_NOMEM;

	return 0;
}

static int request_compare(struct nl_object *_a, struct nl_object *_b,
			   uint32_t attrs, int flags)
{
	struct flnl_request *a = (struct flnl_request *) _a;
	struct flnl_request *b = (struct flnl_request *) _b;
	int diff = 0;

#define REQ_DIFF(ATTR, EXPR) ATTR_DIFF(attrs, REQUEST_ATTR_##ATTR, a, b, EXPR)

	diff |= REQ_DIFF(FWMARK,	a->lr_fwmark != b->lr_fwmark);
	diff |= REQ_DIFF(TOS,		a->lr_tos != b->lr_tos);
	diff |= REQ_DIFF(SCOPE,		a->lr_scope != b->lr_scope);
	diff |= REQ_DIFF(TABLE,		a->lr_table != b->lr_table);
	diff |= REQ_DIFF(ADDR,		nl_addr_cmp(a->lr_addr, b->lr_addr));

#undef REQ_DIFF

	return diff;
}


/**
 * @name Lookup Request Creation/Deletion
 * @{
 */

struct flnl_request *flnl_request_alloc(void)
{
	return REQUEST_CAST(nl_object_alloc(&request_obj_ops));
}

/** @} */

/**
 * @name Attributes
 * @{
 */

void flnl_request_set_fwmark(struct flnl_request *req, uint64_t fwmark)
{
	req->lr_fwmark = fwmark;
	req->ce_mask |= REQUEST_ATTR_FWMARK;
}

uint64_t flnl_request_get_fwmark(struct flnl_request *req)
{
	if (req->ce_mask & REQUEST_ATTR_FWMARK)
		return req->lr_fwmark;
	else
		return UINT_LEAST64_MAX;
}

void flnl_request_set_tos(struct flnl_request *req, int tos)
{
	req->lr_tos = tos;
	req->ce_mask |= REQUEST_ATTR_TOS;
}

int flnl_request_get_tos(struct flnl_request *req)
{
	if (req->ce_mask & REQUEST_ATTR_TOS)
		return req->lr_tos;
	else
		return -1;
}

void flnl_request_set_scope(struct flnl_request *req, int scope)
{
	req->lr_scope = scope;
	req->ce_mask |= REQUEST_ATTR_SCOPE;
}

int flnl_request_get_scope(struct flnl_request *req)
{
	if (req->ce_mask & REQUEST_ATTR_SCOPE)
		return req->lr_scope;
	else
		return -1;
}

void flnl_request_set_table(struct flnl_request *req, int table)
{
	req->lr_table = table;
	req->ce_mask |= REQUEST_ATTR_TABLE;
}

int flnl_request_get_table(struct flnl_request *req)
{
	if (req->ce_mask & REQUEST_ATTR_TABLE)
		return req->lr_table;
	else
		return -1;
}

int flnl_request_set_addr(struct flnl_request *req, struct nl_addr *addr)
{
	if (addr->a_family != AF_INET)
		return -NLE_AF_NOSUPPORT;

	if (req->lr_addr)
		nl_addr_put(req->lr_addr);

	nl_addr_get(addr);
	req->lr_addr = addr;

	req->ce_mask |= REQUEST_ATTR_ADDR;

	return 0;
}

struct nl_addr *flnl_request_get_addr(struct flnl_request *req)
{
	if (req->ce_mask & REQUEST_ATTR_ADDR)
		return req->lr_addr;
	else
		return NULL;
}

/** @} */

static struct nl_object_ops request_obj_ops = {
	.oo_name		= "fib_lookup/request",
	.oo_size		= sizeof(struct flnl_request),
	.oo_free_data		= request_free_data,
	.oo_clone		= request_clone,
	.oo_compare		= request_compare,
	.oo_id_attrs		= ~0,
};

/** @} */

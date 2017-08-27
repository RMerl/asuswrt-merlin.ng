/*
 * lib/fib_lookup/lookup.c	FIB Lookup
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @defgroup fib_lookup FIB Lookup
 * @brief
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/attr.h>
#include <netlink/utils.h>
#include <netlink/object.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/route.h>
#include <netlink/fib_lookup/request.h>
#include <netlink/fib_lookup/lookup.h>

/** @cond SKIP */
static struct nl_cache_ops fib_lookup_ops;
static struct nl_object_ops result_obj_ops;

/* not exported so far */
struct fib_result_nl {
	uint32_t	fl_addr;   /* To be looked up*/ 
	uint32_t	fl_fwmark; 
	unsigned char	fl_tos;
	unsigned char   fl_scope;
	unsigned char   tb_id_in;

	unsigned char   tb_id;      /* Results */
	unsigned char	prefixlen;
	unsigned char	nh_sel;
	unsigned char	type;
	unsigned char	scope;
	int             err;      
};
/** @endcond */

static void result_free_data(struct nl_object *obj)
{
	struct flnl_result *res = nl_object_priv(obj);

	if (res && res->fr_req)
		nl_object_put(OBJ_CAST(res->fr_req));
}

static int result_clone(struct nl_object *_dst, struct nl_object *_src)
{
	struct flnl_result *dst = nl_object_priv(_dst);
	struct flnl_result *src = nl_object_priv(_src);

	if (src->fr_req)
		if (!(dst->fr_req = (struct flnl_request *)
				nl_object_clone(OBJ_CAST(src->fr_req))))
			return -NLE_NOMEM;
	
	return 0;
}

static int result_msg_parser(struct nl_cache_ops *ops, struct sockaddr_nl *who,
			     struct nlmsghdr *n, struct nl_parser_param *pp)
{
	struct flnl_result *res;
	struct fib_result_nl *fr;
	struct nl_addr *addr;
	int err = -NLE_INVAL;

	res = flnl_result_alloc();
	if (!res)
		goto errout;

	res->ce_msgtype = n->nlmsg_type;

	res->fr_req = flnl_request_alloc();
	if (!res->fr_req)
		goto errout;

	fr = nlmsg_data(n);
	addr = nl_addr_build(AF_INET, &fr->fl_addr, 4);
	if (!addr)
		goto errout;
	err = flnl_request_set_addr(res->fr_req, addr);
	nl_addr_put(addr);
	if (err < 0)
		goto errout;

	flnl_request_set_fwmark(res->fr_req, fr->fl_fwmark);
	flnl_request_set_tos(res->fr_req, fr->fl_tos);
	flnl_request_set_scope(res->fr_req, fr->fl_scope);
	flnl_request_set_table(res->fr_req, fr->tb_id_in);

	res->fr_table_id = fr->tb_id;
	res->fr_prefixlen = fr->prefixlen;
	res->fr_nh_sel = fr->nh_sel;
	res->fr_type = fr->type;
	res->fr_scope = fr->scope;
	res->fr_error = fr->err;

	err = pp->pp_cb((struct nl_object *) res, pp);
	if (err < 0)
		goto errout;

	/* REAL HACK, fib_lookup doesn't support ACK nor does it
	 * send a DONE message, enforce end of message stream
	 * after just the first message */
	err = NL_STOP;

errout:
	flnl_result_put(res);
	return err;
}

static void result_dump_line(struct nl_object *obj, struct nl_dump_params *p)
{
	struct flnl_result *res = (struct flnl_result *) obj;
	char buf[128];

	nl_dump_line(p, "table %s prefixlen %u next-hop-selector %u\n",
		rtnl_route_table2str(res->fr_table_id, buf, sizeof(buf)),
		res->fr_prefixlen, res->fr_nh_sel);
	nl_dump_line(p, "type %s ",
		     nl_rtntype2str(res->fr_type, buf, sizeof(buf)));
	nl_dump(p, "scope %s error %s (%d)\n",
		rtnl_scope2str(res->fr_scope, buf, sizeof(buf)),
		strerror(-res->fr_error), res->fr_error);
}

static void result_dump_details(struct nl_object *obj, struct nl_dump_params *p)
{
	result_dump_line(obj, p);
}

static int result_compare(struct nl_object *_a, struct nl_object *_b,
			uint32_t attrs, int flags)
{
	return 0;
}

/**
 * @name Allocation/Freeing
 * @{
 */

struct flnl_result *flnl_result_alloc(void)
{
	return (struct flnl_result *) nl_object_alloc(&result_obj_ops);
}

void flnl_result_put(struct flnl_result *res)
{
	nl_object_put((struct nl_object *) res);
}

/** @} */

/**
 * @name Cache Management
 * @{
 */

/**
 * Allocate lookup result cache.
 *
 * Allocates a new lookup result cache and initializes it properly.
 *
 * @note Free the memory after usage using nl_cache_destroy_and_free().
 * @return Newly allocated cache or NULL if an error occured.
 */
struct nl_cache *flnl_result_alloc_cache(void)
{
	return nl_cache_alloc(&fib_lookup_ops);
}

/** @} */

/**
 * @name Lookup
 * @{
 */

/**
 * Builds a netlink request message to do a lookup
 * @arg req		Requested match.
 * @arg flags		additional netlink message flags
 * @arg result		Result pointer
 *
 * Builds a new netlink message requesting a change of link attributes.
 * The netlink message header isn't fully equipped with all relevant
 * fields and must be sent out via nl_send_auto_complete() or
 * supplemented as needed.
 * \a old must point to a link currently configured in the kernel
 * and \a tmpl must contain the attributes to be changed set via
 * \c rtnl_link_set_* functions.
 *
 * @return 0 on success or a negative error code.
 */
int flnl_lookup_build_request(struct flnl_request *req, int flags,
			      struct nl_msg **result)
{
	struct nl_msg *msg;
	struct nl_addr *addr;
	uint64_t fwmark;
	int tos, scope, table;
	struct fib_result_nl fr = {0};

	fwmark = flnl_request_get_fwmark(req);
	tos = flnl_request_get_tos(req);
	scope = flnl_request_get_scope(req);
	table = flnl_request_get_table(req);

	fr.fl_fwmark = fwmark != UINT_LEAST64_MAX ? fwmark : 0;
	fr.fl_tos = tos >= 0 ? tos : 0;
	fr.fl_scope = scope >= 0 ? scope : RT_SCOPE_UNIVERSE;
	fr.tb_id_in = table >= 0 ? table : RT_TABLE_UNSPEC;

	addr = flnl_request_get_addr(req);
	if (!addr)
		return -NLE_MISSING_ATTR;

	fr.fl_addr = *(uint32_t *) nl_addr_get_binary_addr(addr);

	msg = nlmsg_alloc_simple(0, flags);
	if (!msg)
		return -NLE_NOMEM;

	if (nlmsg_append(msg, &fr, sizeof(fr), NLMSG_ALIGNTO) < 0)
		goto errout;

	*result = msg;
	return 0;

errout:
	nlmsg_free(msg);
	return -NLE_MSGSIZE;
}

/**
 * Perform FIB Lookup
 * @arg sk		Netlink socket.
 * @arg req		Lookup request object.
 * @arg cache		Cache for result.
 *
 * Builds a netlink message to request a FIB lookup, waits for the
 * reply and adds the result to the specified cache.
 *
 * @return 0 on success or a negative error code.
 */
int flnl_lookup(struct nl_sock *sk, struct flnl_request *req,
		struct nl_cache *cache)
{
	struct nl_msg *msg;
	int err;

	if ((err = flnl_lookup_build_request(req, 0, &msg)) < 0)
		return err;

	err = nl_send_auto_complete(sk, msg);
	nlmsg_free(msg);
	if (err < 0)
		return err;

	return nl_cache_pickup(sk, cache);
}

/** @} */

/**
 * @name Attribute Access
 * @{
 */

int flnl_result_get_table_id(struct flnl_result *res)
{
	return res->fr_table_id;
}

int flnl_result_get_prefixlen(struct flnl_result *res)
{
	return res->fr_prefixlen;
}

int flnl_result_get_nexthop_sel(struct flnl_result *res)
{
	return res->fr_nh_sel;
}

int flnl_result_get_type(struct flnl_result *res)
{
	return res->fr_type;
}

int flnl_result_get_scope(struct flnl_result *res)
{
	return res->fr_scope;
}

int flnl_result_get_error(struct flnl_result *res)
{
	return res->fr_error;
}

/** @} */

static struct nl_object_ops result_obj_ops = {
	.oo_name		= "fib_lookup/result",
	.oo_size		= sizeof(struct flnl_result),
	.oo_free_data		= result_free_data,
	.oo_clone		= result_clone,
	.oo_dump = {
	    [NL_DUMP_LINE]	= result_dump_line,
	    [NL_DUMP_DETAILS]	= result_dump_details,
	},
	.oo_compare		= result_compare,
};

static struct nl_cache_ops fib_lookup_ops = {
	.co_name		= "fib_lookup/fib_lookup",
	.co_hdrsize		= sizeof(struct fib_result_nl),
	.co_msgtypes		= {
					{ 0, NL_ACT_UNSPEC, "any" },
					END_OF_MSGTYPES_LIST,
				  },
	.co_protocol		= NETLINK_FIB_LOOKUP,
	.co_msg_parser		= result_msg_parser,
	.co_obj_ops		= &result_obj_ops,
};

static void __init fib_lookup_init(void)
{
	nl_cache_mngt_register(&fib_lookup_ops);
}

static void __exit fib_lookup_exit(void)
{
	nl_cache_mngt_unregister(&fib_lookup_ops);
}

/** @} */

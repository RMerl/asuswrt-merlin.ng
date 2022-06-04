/*
 * lib/idiag/idiagnl_req_obj.c Inet Diag Request Object
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Sassano Systems LLC <joe@sassanosystems.com>
 */

#include <netlink-private/netlink.h>
#include <netlink/idiag/req.h>
#include <linux/inet_diag.h>

/**
 * @ingroup idiag
 * @defgroup idiagnl_req Inet Diag Requests
 *
 * @details
 * @idiagnl_doc{idiagnl_req, Inet Diag Request Documentation}
 * @{
 */
struct idiagnl_req *idiagnl_req_alloc(void)
{
	return (struct idiagnl_req *) nl_object_alloc(&idiagnl_req_obj_ops);
}

void idiagnl_req_get(struct idiagnl_req *req)
{
	nl_object_get((struct nl_object *) req);
}

void idiagnl_req_put(struct idiagnl_req *req)
{
	nl_object_put((struct nl_object *) req);
}

/**
 * @name Attributes
 * @{
 */

uint8_t idiagnl_req_get_family(const struct idiagnl_req *req)
{
	return req->idiag_family;
}

void idiagnl_req_set_family(struct idiagnl_req *req, uint8_t family)
{
	req->idiag_family = family;
}

uint8_t idiagnl_req_get_ext(const struct idiagnl_req *req)
{
	return req->idiag_ext;
}

void idiagnl_req_set_ext(struct idiagnl_req *req, uint8_t ext)
{
	req->idiag_ext = ext;
}

uint32_t idiagnl_req_get_ifindex(const struct idiagnl_req *req)
{
	return req->idiag_ifindex;
}

void idiagnl_req_set_ifindex(struct idiagnl_req *req, uint32_t ifindex)
{
	req->idiag_states = ifindex;
}

uint32_t idiagnl_req_get_states(const struct idiagnl_req *req)
{
	return req->idiag_states;
}

void idiagnl_req_set_states(struct idiagnl_req *req, uint32_t states)
{
	req->idiag_states = states;
}

uint32_t idiagnl_req_get_dbs(const struct idiagnl_req *req)
{
	return req->idiag_dbs;
}

void idiagnl_req_set_dbs(struct idiagnl_req *req, uint32_t dbs)
{
	req->idiag_dbs = dbs;
}

struct nl_addr *idiagnl_req_get_src(const struct idiagnl_req *req)
{
	return req->idiag_src;
}

int idiagnl_req_set_src(struct idiagnl_req *req, struct nl_addr *addr)
{
	if (req->idiag_src)
		nl_addr_put(req->idiag_src);

	nl_addr_get(addr);
	req->idiag_src = addr;

	return 0;
}

struct nl_addr *idiagnl_req_get_dst(const struct idiagnl_req *req)
{
	return req->idiag_dst;
}

int idiagnl_req_set_dst(struct idiagnl_req *req, struct nl_addr *addr)
{
	if (req->idiag_dst)
		nl_addr_put(req->idiag_dst);

	nl_addr_get(addr);
	req->idiag_dst = addr;

	return 0;
}

/** @} */

static void idiag_req_dump_line(struct nl_object *a, struct nl_dump_params *p)
{
	struct idiagnl_req *req = (struct idiagnl_req *) a;
	char buf[64] = { 0 };

	nl_dump_line(p, "%s ", nl_af2str(req->idiag_family, buf, sizeof(buf)));
	nl_dump(p, "src %s ", nl_addr2str(req->idiag_src, buf, sizeof(buf)));
	nl_dump(p, "dst %s ", nl_addr2str(req->idiag_dst, buf, sizeof(buf)));
	nl_dump(p, "iif %d ", req->idiag_ifindex);
	nl_dump(p, "\n");
}

static void idiag_req_dump_details(struct nl_object *a, struct nl_dump_params *p)
{
	struct idiagnl_req *req = (struct idiagnl_req *) a;
	char buf[64];

	nl_dump_line(p, "    ");
	nl_dump(p, "%s ", nl_af2str(req->idiag_family, buf, sizeof(buf)));
	nl_dump(p, "exts %s ",
			idiagnl_exts2str(req->idiag_ext, buf, sizeof(buf)));
	nl_dump(p, "src %s ", nl_addr2str(req->idiag_src, buf, sizeof(buf)));
	nl_dump(p, "dst %s ", nl_addr2str(req->idiag_dst, buf, sizeof(buf)));
	nl_dump(p, "iif %d ", req->idiag_ifindex);
	nl_dump(p, "states %s ", idiagnl_state2str(req->idiag_states, buf,
				sizeof(buf)));
	nl_dump(p, "dbs %d", req->idiag_dbs);
	nl_dump(p, "\n");
}

static void idiag_req_dump_stats(struct nl_object *obj, struct nl_dump_params *p)
{
	idiag_req_dump_details(obj, p);
}

static void idiagnl_req_free(struct nl_object *a)
{
	struct idiagnl_req *req = (struct idiagnl_req *) a;
	if (a == NULL)
		return;

	nl_addr_put(req->idiag_src);
	nl_addr_put(req->idiag_dst);
}

static int idiagnl_req_clone(struct nl_object *_dst, struct nl_object *_src)
{
	struct idiagnl_req *dst = (struct idiagnl_req *) _dst;
	struct idiagnl_req *src = (struct idiagnl_req *) _src;

	if (src->idiag_src)
		if (!(dst->idiag_src = nl_addr_clone(src->idiag_src)))
			return -NLE_NOMEM;

	if (src->idiag_dst)
		if (!(dst->idiag_dst = nl_addr_clone(src->idiag_dst)))
			return -NLE_NOMEM;

	return 0;
}

int idiagnl_req_parse(struct nlmsghdr *nlh, struct idiagnl_req **result)
{
	struct idiagnl_req *req = NULL;
	struct inet_diag_req *raw_req = NULL;
	struct nl_addr *src = NULL, *dst = NULL;
	int err = 0;

	req = idiagnl_req_alloc();
	if (!req)
		goto errout_nomem;

	raw_req = nlmsg_data(nlh);
	req->idiag_family = raw_req->idiag_family;
	req->idiag_ext = raw_req->idiag_ext;
	req->idiag_states = raw_req->idiag_states;
	req->idiag_dbs = raw_req->idiag_dbs;
	req->idiag_ifindex = raw_req->id.idiag_if;

	dst = nl_addr_build(raw_req->idiag_family, raw_req->id.idiag_dst,
			sizeof(raw_req->id.idiag_dst));
	if (!dst)
		goto errout_nomem;

	err = idiagnl_req_set_dst(req, dst);
	if (err < 0)
		goto errout;

	nl_addr_put(dst);

	src = nl_addr_build(raw_req->idiag_family, raw_req->id.idiag_src,
			sizeof(raw_req->id.idiag_src));
	if (!src)
		goto errout_nomem;

	err = idiagnl_req_set_src(req, src);
	if (err < 0)
		goto errout;

	nl_addr_put(src);

	*result = req;
	return 0;

errout:
	idiagnl_req_put(req);
	return err;

errout_nomem:
	err = -NLE_NOMEM;
	goto errout;
}

/** @cond SKIP */
struct nl_object_ops idiagnl_req_obj_ops = {
	.oo_name		  = "idiag/idiag_req",
	.oo_size		  = sizeof(struct idiagnl_req),
	.oo_free_data		  = idiagnl_req_free,
	.oo_clone		  = idiagnl_req_clone,
	.oo_dump		  = {
		[NL_DUMP_LINE]	  = idiag_req_dump_line,
		[NL_DUMP_DETAILS] = idiag_req_dump_details,
		[NL_DUMP_STATS]	  = idiag_req_dump_stats,
	},
};
/** @endcond */

/** @} */

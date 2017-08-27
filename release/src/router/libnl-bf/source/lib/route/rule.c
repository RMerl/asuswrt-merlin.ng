/*
 * lib/route/rule.c          Routing Rules
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2010 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup rtnl
 * @defgroup rule Routing Rules
 * @brief
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/rule.h>
#include <inttypes.h>

/** @cond SKIP */
#define RULE_ATTR_FAMILY	0x0001
#define RULE_ATTR_TABLE		0x0002
#define RULE_ATTR_ACTION	0x0004
#define RULE_ATTR_FLAGS		0x0008
#define RULE_ATTR_IIFNAME	0x0010
#define RULE_ATTR_OIFNAME	0x0020
#define RULE_ATTR_PRIO		0x0040
#define RULE_ATTR_MARK		0x0080
#define RULE_ATTR_MASK		0x0100
#define RULE_ATTR_GOTO		0x0200
#define RULE_ATTR_SRC		0x0400
#define RULE_ATTR_DST		0x0800
#define RULE_ATTR_DSFIELD	0x1000
#define RULE_ATTR_FLOW		0x2000

static struct nl_cache_ops rtnl_rule_ops;
static struct nl_object_ops rule_obj_ops;
/** @endcond */

static void rule_free_data(struct nl_object *c)
{
	struct rtnl_rule *rule = nl_object_priv(c);

	if (!rule)
		return;

	nl_addr_put(rule->r_src);
	nl_addr_put(rule->r_dst);
}

static int rule_clone(struct nl_object *_dst, struct nl_object *_src)
{
	struct rtnl_rule *dst = nl_object_priv(_dst);
	struct rtnl_rule *src = nl_object_priv(_src);

	if (src->r_src)
		if (!(dst->r_src = nl_addr_clone(src->r_src)))
			return -NLE_NOMEM;

	if (src->r_dst)
		if (!(dst->r_dst = nl_addr_clone(src->r_dst)))
			return -NLE_NOMEM;

	return 0;
}

static struct nla_policy rule_policy[FRA_MAX+1] = {
	[FRA_TABLE]	= { .type = NLA_U32 },
	[FRA_IIFNAME]	= { .type = NLA_STRING, .maxlen = IFNAMSIZ },
	[FRA_OIFNAME]	= { .type = NLA_STRING, .maxlen = IFNAMSIZ },
	[FRA_PRIORITY]	= { .type = NLA_U32 },
	[FRA_FWMARK]	= { .type = NLA_U32 },
	[FRA_FWMASK]	= { .type = NLA_U32 },
	[FRA_GOTO]	= { .type = NLA_U32 },
	[FRA_FLOW]	= { .type = NLA_U32 },
};

static int rule_msg_parser(struct nl_cache_ops *ops, struct sockaddr_nl *who,
			   struct nlmsghdr *n, struct nl_parser_param *pp)
{
	struct rtnl_rule *rule;
	struct fib_rule_hdr *frh;
	struct nlattr *tb[FRA_MAX+1];
	int err = 1, family;

	rule = rtnl_rule_alloc();
	if (!rule) {
		err = -NLE_NOMEM;
		goto errout;
	}

	rule->ce_msgtype = n->nlmsg_type;
	frh = nlmsg_data(n);

	err = nlmsg_parse(n, sizeof(*frh), tb, FRA_MAX, rule_policy);
	if (err < 0)
		goto errout;

	rule->r_family = family = frh->family;
	rule->r_table = frh->table;
	rule->r_action = frh->action;
	rule->r_flags = frh->flags;

	rule->ce_mask = (RULE_ATTR_FAMILY | RULE_ATTR_TABLE | RULE_ATTR_ACTION |
			 RULE_ATTR_FLAGS);

	/* ipv4 only */
	if (frh->tos) {
		rule->r_dsfield = frh->tos;
		rule->ce_mask |= RULE_ATTR_DSFIELD;
	}

	if (tb[FRA_TABLE]) {
		rule->r_table = nla_get_u32(tb[FRA_TABLE]);
		rule->ce_mask |= RULE_ATTR_TABLE;
	}

	if (tb[FRA_IIFNAME]) {
		nla_strlcpy(rule->r_iifname, tb[FRA_IIFNAME], IFNAMSIZ);
		rule->ce_mask |= RULE_ATTR_IIFNAME;
	}

	if (tb[FRA_OIFNAME]) {
		nla_strlcpy(rule->r_oifname, tb[FRA_OIFNAME], IFNAMSIZ);
		rule->ce_mask |= RULE_ATTR_OIFNAME;
	}

	if (tb[FRA_PRIORITY]) {
		rule->r_prio = nla_get_u32(tb[FRA_PRIORITY]);
		rule->ce_mask |= RULE_ATTR_PRIO;
	}

	if (tb[FRA_FWMARK]) {
		rule->r_mark = nla_get_u32(tb[FRA_FWMARK]);
		rule->ce_mask |= RULE_ATTR_MARK;
	}

	if (tb[FRA_FWMASK]) {
		rule->r_mask = nla_get_u32(tb[FRA_FWMASK]);
		rule->ce_mask |= RULE_ATTR_MASK;
	}

	if (tb[FRA_GOTO]) {
		rule->r_goto = nla_get_u32(tb[FRA_GOTO]);
		rule->ce_mask |= RULE_ATTR_GOTO;
	}

	if (tb[FRA_SRC]) {
		if (!(rule->r_src = nl_addr_alloc_attr(tb[FRA_SRC], family)))
			goto errout_enomem;

		nl_addr_set_prefixlen(rule->r_src, frh->src_len);
		rule->ce_mask |= RULE_ATTR_SRC;
	}

	if (tb[FRA_DST]) {
		if (!(rule->r_dst = nl_addr_alloc_attr(tb[FRA_DST], family)))
			goto errout_enomem;
		nl_addr_set_prefixlen(rule->r_dst, frh->dst_len);
		rule->ce_mask |= RULE_ATTR_DST;
	}

	/* ipv4 only */
	if (tb[FRA_FLOW]) {
		rule->r_flow = nla_get_u32(tb[FRA_FLOW]);
		rule->ce_mask |= RULE_ATTR_FLOW;
	}

	err = pp->pp_cb((struct nl_object *) rule, pp);
errout:
	rtnl_rule_put(rule);
	return err;

errout_enomem:
	err = -NLE_NOMEM;
	goto errout;
}

static int rule_request_update(struct nl_cache *c, struct nl_sock *h)
{
	return nl_rtgen_request(h, RTM_GETRULE, AF_UNSPEC, NLM_F_DUMP);
}

static void rule_dump_line(struct nl_object *o, struct nl_dump_params *p)
{
	struct rtnl_rule *r = (struct rtnl_rule *) o;
	char buf[128];

	nl_dump_line(p, "%8d ", (r->ce_mask & RULE_ATTR_PRIO) ? r->r_prio : 0);
	nl_dump(p, "%s ", nl_af2str(r->r_family, buf, sizeof(buf)));

	if (r->ce_mask & RULE_ATTR_SRC)
		nl_dump(p, "from %s ",
			nl_addr2str(r->r_src, buf, sizeof(buf)));

	if (r->ce_mask & RULE_ATTR_DST)
		nl_dump(p, "to %s ",
			nl_addr2str(r->r_dst, buf, sizeof(buf)));

	if (r->ce_mask & RULE_ATTR_DSFIELD)
		nl_dump(p, "tos %u ", r->r_dsfield);

	if (r->ce_mask & (RULE_ATTR_MARK | RULE_ATTR_MASK))
		nl_dump(p, "mark %#x/%#x", r->r_mark, r->r_mask);

	if (r->ce_mask & RULE_ATTR_IIFNAME)
		nl_dump(p, "iif %s ", r->r_iifname);

	if (r->ce_mask & RULE_ATTR_OIFNAME)
		nl_dump(p, "oif %s ", r->r_oifname);

	if (r->ce_mask & RULE_ATTR_TABLE)
		nl_dump(p, "lookup %s ",
			rtnl_route_table2str(r->r_table, buf, sizeof(buf)));

	if (r->ce_mask & RULE_ATTR_FLOW)
		nl_dump(p, "flow %s ",
			rtnl_realms2str(r->r_flow, buf, sizeof(buf)));

	if (r->ce_mask & RULE_ATTR_GOTO)
		nl_dump(p, "goto %u ", r->r_goto);

	if (r->ce_mask & RULE_ATTR_ACTION)
		nl_dump(p, "action %s",
			nl_rtntype2str(r->r_action, buf, sizeof(buf)));

	nl_dump(p, "\n");
}

static void rule_dump_details(struct nl_object *obj, struct nl_dump_params *p)
{
	rule_dump_line(obj, p);
}

static void rule_dump_stats(struct nl_object *obj, struct nl_dump_params *p)
{
	rule_dump_details(obj, p);
}

#define RULE_ATTR_FLAGS		0x0008

static int rule_compare(struct nl_object *_a, struct nl_object *_b,
			uint32_t attrs, int flags)
{
	struct rtnl_rule *a = (struct rtnl_rule *) _a;
	struct rtnl_rule *b = (struct rtnl_rule *) _b;
	int diff = 0;

#define RULE_DIFF(ATTR, EXPR) ATTR_DIFF(attrs, RULE_ATTR_##ATTR, a, b, EXPR)

	diff |= RULE_DIFF(FAMILY,	a->r_family != b->r_family);
	diff |= RULE_DIFF(TABLE,	a->r_table != b->r_table);
	diff |= RULE_DIFF(ACTION,	a->r_action != b->r_action);
	diff |= RULE_DIFF(IIFNAME,	strcmp(a->r_iifname, b->r_iifname));
	diff |= RULE_DIFF(OIFNAME,	strcmp(a->r_oifname, b->r_oifname));
	diff |= RULE_DIFF(PRIO,		a->r_prio != b->r_prio);
	diff |= RULE_DIFF(MARK,		a->r_mark != b->r_mark);
	diff |= RULE_DIFF(MASK,		a->r_mask != b->r_mask);
	diff |= RULE_DIFF(GOTO,		a->r_goto != b->r_goto);
	diff |= RULE_DIFF(SRC,		nl_addr_cmp(a->r_src, b->r_src));
	diff |= RULE_DIFF(DST,		nl_addr_cmp(a->r_dst, b->r_dst));
	diff |= RULE_DIFF(DSFIELD,	a->r_dsfield != b->r_dsfield);
	diff |= RULE_DIFF(FLOW,		a->r_flow != b->r_flow);
	
#undef RULE_DIFF

	return diff;
}

static const struct trans_tbl rule_attrs[] = {
	__ADD(RULE_ATTR_FAMILY, family)
	__ADD(RULE_ATTR_TABLE, table)
	__ADD(RULE_ATTR_ACTION, action)
	__ADD(RULE_ATTR_IIFNAME, iifname)
	__ADD(RULE_ATTR_OIFNAME, oifname)
	__ADD(RULE_ATTR_PRIO, prio)
	__ADD(RULE_ATTR_MARK, mark)
	__ADD(RULE_ATTR_MASK, mask)
	__ADD(RULE_ATTR_GOTO, goto)
	__ADD(RULE_ATTR_SRC, src)
	__ADD(RULE_ATTR_DST, dst)
	__ADD(RULE_ATTR_DSFIELD, dsfield)
	__ADD(RULE_ATTR_FLOW, flow)
};

static char *rule_attrs2str(int attrs, char *buf, size_t len)
{
	return __flags2str(attrs, buf, len, rule_attrs,
			   ARRAY_SIZE(rule_attrs));
}

/**
 * @name Allocation/Freeing
 * @{
 */

struct rtnl_rule *rtnl_rule_alloc(void)
{
	return (struct rtnl_rule *) nl_object_alloc(&rule_obj_ops);
}

void rtnl_rule_put(struct rtnl_rule *rule)
{
	nl_object_put((struct nl_object *) rule);
}

/** @} */

/**
 * @name Cache Management
 * @{
 */

/**
 * Build a rule cache including all rules currently configured in the kernel.
 * @arg sock		Netlink socket.
 * @arg family		Address family or AF_UNSPEC.
 * @arg result		Pointer to store resulting cache.
 *
 * Allocates a new rule cache, initializes it properly and updates it
 * to include all rules currently configured in the kernel.
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_rule_alloc_cache(struct nl_sock *sock, int family,
			  struct nl_cache **result)
{
	struct nl_cache * cache;
	int err;

	if (!(cache = nl_cache_alloc(&rtnl_rule_ops)))
		return -NLE_NOMEM;

	cache->c_iarg1 = family;

	if (sock && (err = nl_cache_refill(sock, cache)) < 0) {
		free(cache);
		return err;
	}

	*result = cache;
	return 0;
}

/** @} */

/**
 * @name Rule Addition
 * @{
 */

static int build_rule_msg(struct rtnl_rule *tmpl, int cmd, int flags,
			  struct nl_msg **result)
{
	struct nl_msg *msg;
	struct fib_rule_hdr frh = {
		.family = tmpl->r_family,
		.table = tmpl->r_table,
		.action = tmpl->r_action,
		.flags = tmpl->r_flags,
		.tos = tmpl->r_dsfield,
	};

	if (!(tmpl->ce_mask & RULE_ATTR_FAMILY))
		return -NLE_MISSING_ATTR;

	msg = nlmsg_alloc_simple(cmd, flags);
	if (!msg)
		return -NLE_NOMEM;

	if (tmpl->ce_mask & RULE_ATTR_SRC) 
		frh.src_len = nl_addr_get_prefixlen(tmpl->r_src);

	if (tmpl->ce_mask & RULE_ATTR_DST)
		frh.dst_len = nl_addr_get_prefixlen(tmpl->r_dst);

	if (nlmsg_append(msg, &frh, sizeof(frh), NLMSG_ALIGNTO) < 0)
		goto nla_put_failure;

	/* Additional table attribute replacing the 8bit in the header, was
	 * required to allow more than 256 tables. */
	NLA_PUT_U32(msg, FRA_TABLE, tmpl->r_table);

	if (tmpl->ce_mask & RULE_ATTR_SRC)
		NLA_PUT_ADDR(msg, FRA_SRC, tmpl->r_src);

	if (tmpl->ce_mask & RULE_ATTR_DST) 
		NLA_PUT_ADDR(msg, FRA_DST, tmpl->r_dst);

	if (tmpl->ce_mask & RULE_ATTR_IIFNAME)
		NLA_PUT_STRING(msg, FRA_IIFNAME, tmpl->r_iifname);

	if (tmpl->ce_mask & RULE_ATTR_OIFNAME)
		NLA_PUT_STRING(msg, FRA_OIFNAME, tmpl->r_oifname);

	if (tmpl->ce_mask & RULE_ATTR_PRIO)
		NLA_PUT_U32(msg, FRA_PRIORITY, tmpl->r_prio);

	if (tmpl->ce_mask & RULE_ATTR_MARK)
		NLA_PUT_U32(msg, FRA_FWMARK, tmpl->r_mark);

	if (tmpl->ce_mask & RULE_ATTR_MASK)
		NLA_PUT_U32(msg, FRA_FWMASK, tmpl->r_mask);

	if (tmpl->ce_mask & RULE_ATTR_GOTO)
		NLA_PUT_U32(msg, FRA_GOTO, tmpl->r_goto);

	if (tmpl->ce_mask & RULE_ATTR_FLOW)
		NLA_PUT_U32(msg, FRA_FLOW, tmpl->r_flow);


	*result = msg;
	return 0;

nla_put_failure:
	nlmsg_free(msg);
	return -NLE_MSGSIZE;
}

/**
 * Build netlink request message to add a new rule
 * @arg tmpl		template with data of new rule
 * @arg flags		additional netlink message flags
 * @arg result		Result pointer
 *
 * Builds a new netlink message requesting a addition of a new
 * rule. The netlink message header isn't fully equipped with
 * all relevant fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed. \a tmpl must contain the attributes of the new
 * address set via \c rtnl_rule_set_* functions.
 * 
 * @return 0 on success or a negative error code.
 */
int rtnl_rule_build_add_request(struct rtnl_rule *tmpl, int flags,
				struct nl_msg **result)
{
	return build_rule_msg(tmpl, RTM_NEWRULE, NLM_F_CREATE | flags,
			      result);
}

/**
 * Add a new rule
 * @arg sk		Netlink socket.
 * @arg tmpl		template with requested changes
 * @arg flags		additional netlink message flags
 *
 * Builds a netlink message by calling rtnl_rule_build_add_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been fullfilled.
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_rule_add(struct nl_sock *sk, struct rtnl_rule *tmpl, int flags)
{
	struct nl_msg *msg;
	int err;
	
	if ((err = rtnl_rule_build_add_request(tmpl, flags, &msg)) < 0)
		return err;

	err = nl_send_auto_complete(sk, msg);
	nlmsg_free(msg);
	if (err < 0)
		return err;

	return wait_for_ack(sk);
}

/** @} */

/**
 * @name Rule Deletion
 * @{
 */

/**
 * Build a netlink request message to delete a rule
 * @arg rule		rule to delete
 * @arg flags		additional netlink message flags
 * @arg result		Result pointer
 *
 * Builds a new netlink message requesting a deletion of a rule.
 * The netlink message header isn't fully equipped with all relevant
 * fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed. \a rule must point to an existing
 * address.
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_rule_build_delete_request(struct rtnl_rule *rule, int flags,
				   struct nl_msg **result)
{
	return build_rule_msg(rule, RTM_DELRULE, flags, result);
}

/**
 * Delete a rule
 * @arg sk		Netlink socket.
 * @arg rule		rule to delete
 * @arg flags		additional netlink message flags
 *
 * Builds a netlink message by calling rtnl_rule_build_delete_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been fullfilled.
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_rule_delete(struct nl_sock *sk, struct rtnl_rule *rule, int flags)
{
	struct nl_msg *msg;
	int err;
	
	if ((err = rtnl_rule_build_delete_request(rule, flags, &msg)) < 0)
		return err;

	err = nl_send_auto_complete(sk, msg);
	nlmsg_free(msg);
	if (err < 0)
		return err;

	return wait_for_ack(sk);
}

/** @} */

/**
 * @name Attribute Modification
 * @{
 */

void rtnl_rule_set_family(struct rtnl_rule *rule, int family)
{
	rule->r_family = family;
	rule->ce_mask |= RULE_ATTR_FAMILY;
}

int rtnl_rule_get_family(struct rtnl_rule *rule)
{
	if (rule->ce_mask & RULE_ATTR_FAMILY)
		return rule->r_family;
	else
		return AF_UNSPEC;
}

void rtnl_rule_set_prio(struct rtnl_rule *rule, uint32_t prio)
{
	rule->r_prio = prio;
	rule->ce_mask |= RULE_ATTR_PRIO;
}

uint32_t rtnl_rule_get_prio(struct rtnl_rule *rule)
{
	return rule->r_prio;
}

void rtnl_rule_set_mark(struct rtnl_rule *rule, uint32_t mark)
{
	rule->r_mark = mark;
	rule->ce_mask |= RULE_ATTR_MARK;
}

uint32_t rtnl_rule_get_mark(struct rtnl_rule *rule)
{
	return rule->r_mark;
}

void rtnl_rule_set_mask(struct rtnl_rule *rule, uint32_t mask)
{
	rule->r_mask = mask;
	rule->ce_mask |= RULE_ATTR_MASK;
}

uint32_t rtnl_rule_get_mask(struct rtnl_rule *rule)
{
	return rule->r_mask;
}

void rtnl_rule_set_table(struct rtnl_rule *rule, uint32_t table)
{
	rule->r_table = table;
	rule->ce_mask |= RULE_ATTR_TABLE;
}

uint32_t rtnl_rule_get_table(struct rtnl_rule *rule)
{
	return rule->r_table;
}

void rtnl_rule_set_dsfield(struct rtnl_rule *rule, uint8_t dsfield)
{
	rule->r_dsfield = dsfield;
	rule->ce_mask |= RULE_ATTR_DSFIELD;
}

uint8_t rtnl_rule_get_dsfield(struct rtnl_rule *rule)
{
	return rule->r_dsfield;
}

static inline int __assign_addr(struct rtnl_rule *rule, struct nl_addr **pos,
			        struct nl_addr *new, int flag)
{
	if (rule->ce_mask & RULE_ATTR_FAMILY) {
		if (new->a_family != rule->r_family)
			return -NLE_AF_MISMATCH;
	} else
		rule->r_family = new->a_family;

	if (*pos)
		nl_addr_put(*pos);

	nl_addr_get(new);
	*pos = new;

	rule->ce_mask |= (flag | RULE_ATTR_FAMILY);

	return 0;
}

int rtnl_rule_set_src(struct rtnl_rule *rule, struct nl_addr *src)
{
	return __assign_addr(rule, &rule->r_src, src, RULE_ATTR_SRC);
}

struct nl_addr *rtnl_rule_get_src(struct rtnl_rule *rule)
{
	return rule->r_src;
}

int rtnl_rule_set_dst(struct rtnl_rule *rule, struct nl_addr *dst)
{
	return __assign_addr(rule, &rule->r_dst, dst, RULE_ATTR_DST);
}

struct nl_addr *rtnl_rule_get_dst(struct rtnl_rule *rule)
{
	return rule->r_dst;
}

int rtnl_rule_set_iif(struct rtnl_rule *rule, const char *dev)
{
	if (strlen(dev) > IFNAMSIZ-1)
		return -NLE_RANGE;

	strcpy(rule->r_iifname, dev);
	rule->ce_mask |= RULE_ATTR_IIFNAME;
	return 0;
}

char *rtnl_rule_get_iif(struct rtnl_rule *rule)
{
	if (rule->ce_mask & RULE_ATTR_IIFNAME)
		return rule->r_iifname;
	else
		return NULL;
}

int rtnl_rule_set_oif(struct rtnl_rule *rule, const char *dev)
{
	if (strlen(dev) > IFNAMSIZ-1)
		return -NLE_RANGE;

	strcpy(rule->r_oifname, dev);
	rule->ce_mask |= RULE_ATTR_OIFNAME;
	return 0;
}

char *rtnl_rule_get_oif(struct rtnl_rule *rule)
{
	if (rule->ce_mask & RULE_ATTR_OIFNAME)
		return rule->r_oifname;
	else
		return NULL;
}

void rtnl_rule_set_action(struct rtnl_rule *rule, uint8_t action)
{
	rule->r_action = action;
	rule->ce_mask |= RULE_ATTR_ACTION;
}

uint8_t rtnl_rule_get_action(struct rtnl_rule *rule)
{
	return rule->r_action;
}

void rtnl_rule_set_realms(struct rtnl_rule *rule, uint32_t realms)
{
	rule->r_flow = realms;
	rule->ce_mask |= RULE_ATTR_FLOW;
}

uint32_t rtnl_rule_get_realms(struct rtnl_rule *rule)
{
	return rule->r_flow;
}

void rtnl_rule_set_goto(struct rtnl_rule *rule, uint32_t ref)
{
	rule->r_goto = ref;
	rule->ce_mask |= RULE_ATTR_GOTO;
}

uint32_t rtnl_rule_get_goto(struct rtnl_rule *rule)
{
	return rule->r_goto;
}

/** @} */

static struct nl_object_ops rule_obj_ops = {
	.oo_name		= "route/rule",
	.oo_size		= sizeof(struct rtnl_rule),
	.oo_free_data		= rule_free_data,
	.oo_clone		= rule_clone,
	.oo_dump = {
	    [NL_DUMP_LINE]	= rule_dump_line,
	    [NL_DUMP_DETAILS]	= rule_dump_details,
	    [NL_DUMP_STATS]	= rule_dump_stats,
	},
	.oo_compare		= rule_compare,
	.oo_attrs2str		= rule_attrs2str,
	.oo_id_attrs		= ~0,
};

static struct nl_cache_ops rtnl_rule_ops = {
	.co_name		= "route/rule",
	.co_hdrsize		= sizeof(struct fib_rule_hdr),
	.co_msgtypes		= {
					{ RTM_NEWRULE, NL_ACT_NEW, "new" },
					{ RTM_DELRULE, NL_ACT_DEL, "del" },
					{ RTM_GETRULE, NL_ACT_GET, "get" },
					END_OF_MSGTYPES_LIST,
				  },
	.co_protocol		= NETLINK_ROUTE,
	.co_request_update	= rule_request_update,
	.co_msg_parser		= rule_msg_parser,
	.co_obj_ops		= &rule_obj_ops,
};

static void __init rule_init(void)
{
	nl_cache_mngt_register(&rtnl_rule_ops);
}

static void __exit rule_exit(void)
{
	nl_cache_mngt_unregister(&rtnl_rule_ops);
}

/** @} */

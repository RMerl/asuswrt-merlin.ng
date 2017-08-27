/*
 * lib/route/cls/u32.c		u32 classifier
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2011 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2005-2006 Petr Gotthard <petr.gotthard@siemens.com>
 * Copyright (c) 2005-2006 Siemens AG Oesterreich
 */

/**
 * @ingroup cls
 * @defgroup cls_u32 Universal 32-bit Classifier
 *
 * @{
 */

#include <netlink-local.h>
#include <netlink-tc.h>
#include <netlink/netlink.h>
#include <netlink/attr.h>
#include <netlink/utils.h>
#include <netlink/route/tc-api.h>
#include <netlink/route/classifier.h>
#include <netlink/route/cls/u32.h>

/** @cond SKIP */
#define U32_ATTR_DIVISOR      0x001
#define U32_ATTR_HASH         0x002
#define U32_ATTR_CLASSID      0x004
#define U32_ATTR_LINK         0x008
#define U32_ATTR_PCNT         0x010
#define U32_ATTR_SELECTOR     0x020
#define U32_ATTR_ACTION       0x040
#define U32_ATTR_POLICE       0x080
#define U32_ATTR_INDEV        0x100
/** @endcond */

static inline struct tc_u32_sel *u32_selector(struct rtnl_u32 *u)
{
	return (struct tc_u32_sel *) u->cu_selector->d_data;
}

static inline struct tc_u32_sel *u32_selector_alloc(struct rtnl_u32 *u)
{
	if (!u->cu_selector)
		u->cu_selector = nl_data_alloc(NULL, sizeof(struct tc_u32_sel));

	return u32_selector(u);
}

static struct nla_policy u32_policy[TCA_U32_MAX+1] = {
	[TCA_U32_DIVISOR]	= { .type = NLA_U32 },
	[TCA_U32_HASH]		= { .type = NLA_U32 },
	[TCA_U32_CLASSID]	= { .type = NLA_U32 },
	[TCA_U32_LINK]		= { .type = NLA_U32 },
	[TCA_U32_INDEV]		= { .type = NLA_STRING,
				    .maxlen = IFNAMSIZ },
	[TCA_U32_SEL]		= { .minlen = sizeof(struct tc_u32_sel) },
	[TCA_U32_PCNT]		= { .minlen = sizeof(struct tc_u32_pcnt) },
};

static int u32_msg_parser(struct rtnl_tc *tc, void *data)
{
	struct rtnl_u32 *u = data;
	struct nlattr *tb[TCA_U32_MAX + 1];
	int err;

	err = tca_parse(tb, TCA_U32_MAX, tc, u32_policy);
	if (err < 0)
		return err;

	if (tb[TCA_U32_DIVISOR]) {
		u->cu_divisor = nla_get_u32(tb[TCA_U32_DIVISOR]);
		u->cu_mask |= U32_ATTR_DIVISOR;
	}

	if (tb[TCA_U32_SEL]) {
		u->cu_selector = nl_data_alloc_attr(tb[TCA_U32_SEL]);
		if (!u->cu_selector)
			goto errout_nomem;
		u->cu_mask |= U32_ATTR_SELECTOR;
	}

	if (tb[TCA_U32_HASH]) {
		u->cu_hash = nla_get_u32(tb[TCA_U32_HASH]);
		u->cu_mask |= U32_ATTR_HASH;
	}

	if (tb[TCA_U32_CLASSID]) {
		u->cu_classid = nla_get_u32(tb[TCA_U32_CLASSID]);
		u->cu_mask |= U32_ATTR_CLASSID;
	}

	if (tb[TCA_U32_LINK]) {
		u->cu_link = nla_get_u32(tb[TCA_U32_LINK]);
		u->cu_mask |= U32_ATTR_LINK;
	}

	if (tb[TCA_U32_ACT]) {
		u->cu_act = nl_data_alloc_attr(tb[TCA_U32_ACT]);
		if (!u->cu_act)
			goto errout_nomem;
		u->cu_mask |= U32_ATTR_ACTION;
	}

	if (tb[TCA_U32_POLICE]) {
		u->cu_police = nl_data_alloc_attr(tb[TCA_U32_POLICE]);
		if (!u->cu_police)
			goto errout_nomem;
		u->cu_mask |= U32_ATTR_POLICE;
	}

	if (tb[TCA_U32_PCNT]) {
		struct tc_u32_sel *sel;
		int pcnt_size;

		if (!tb[TCA_U32_SEL]) {
			err = -NLE_MISSING_ATTR;
			goto errout;
		}
		
		sel = u->cu_selector->d_data;
		pcnt_size = sizeof(struct tc_u32_pcnt) +
				(sel->nkeys * sizeof(uint64_t));
		if (nla_len(tb[TCA_U32_PCNT]) < pcnt_size) {
			err = -NLE_INVAL;
			goto errout;
		}

		u->cu_pcnt = nl_data_alloc_attr(tb[TCA_U32_PCNT]);
		if (!u->cu_pcnt)
			goto errout_nomem;
		u->cu_mask |= U32_ATTR_PCNT;
	}

	if (tb[TCA_U32_INDEV]) {
		nla_strlcpy(u->cu_indev, tb[TCA_U32_INDEV], IFNAMSIZ);
		u->cu_mask |= U32_ATTR_INDEV;
	}

	return 0;

errout_nomem:
	err = -NLE_NOMEM;
errout:
	return err;
}

static void u32_free_data(struct rtnl_tc *tc, void *data)
{
	struct rtnl_u32 *u = data;

	nl_data_free(u->cu_selector);
	nl_data_free(u->cu_act);
	nl_data_free(u->cu_police);
	nl_data_free(u->cu_pcnt);
}

static int u32_clone(void *_dst, void *_src)
{
	struct rtnl_u32 *dst = _dst, *src = _src;

	if (src->cu_selector &&
	    !(dst->cu_selector = nl_data_clone(src->cu_selector)))
		return -NLE_NOMEM;

	if (src->cu_act && !(dst->cu_act = nl_data_clone(src->cu_act)))
		return -NLE_NOMEM;

	if (src->cu_police && !(dst->cu_police = nl_data_clone(src->cu_police)))
		return -NLE_NOMEM;

	if (src->cu_pcnt && !(dst->cu_pcnt = nl_data_clone(src->cu_pcnt)))
		return -NLE_NOMEM;

	return 0;
}

static void u32_dump_line(struct rtnl_tc *tc, void *data,
			  struct nl_dump_params *p)
{
	struct rtnl_u32 *u = data;
	char buf[32];
	
	if (!u)
		return;

	if (u->cu_mask & U32_ATTR_DIVISOR)
		nl_dump(p, " divisor %u", u->cu_divisor);
	else if (u->cu_mask & U32_ATTR_CLASSID)
		nl_dump(p, " target %s",
			rtnl_tc_handle2str(u->cu_classid, buf, sizeof(buf)));
}

static void print_selector(struct nl_dump_params *p, struct tc_u32_sel *sel,
			   struct rtnl_u32 *u)
{
	int i;
	struct tc_u32_key *key;

	if (sel->hmask || sel->hoff) {
		/* I guess this will never be used since the kernel only
		 * exports the selector if no divisor is set but hash offset
		 * and hash mask make only sense in hash filters with divisor
		 * set */
		nl_dump(p, " hash at %u & 0x%x", sel->hoff, sel->hmask);
	}

	if (sel->flags & (TC_U32_OFFSET | TC_U32_VAROFFSET)) {
		nl_dump(p, " offset at %u", sel->off);

		if (sel->flags & TC_U32_VAROFFSET)
			nl_dump(p, " variable (at %u & 0x%x) >> %u",
				sel->offoff, ntohs(sel->offmask), sel->offshift);
	}

	if (sel->flags) {
		int flags = sel->flags;
		nl_dump(p, " <");

#define PRINT_FLAG(f) if (flags & TC_U32_##f) { \
	flags &= ~TC_U32_##f; nl_dump(p, #f "%s", flags ? "," : ""); }

		PRINT_FLAG(TERMINAL);
		PRINT_FLAG(OFFSET);
		PRINT_FLAG(VAROFFSET);
		PRINT_FLAG(EAT);
#undef PRINT_FLAG

		nl_dump(p, ">");
	}
		
	
	for (i = 0; i < sel->nkeys; i++) {
		key = (struct tc_u32_key *) ((char *) sel + sizeof(*sel)) + i;

		nl_dump(p, "\n");
		nl_dump_line(p, "      match key at %s%u ",
			key->offmask ? "nexthdr+" : "", key->off);

		if (key->offmask)
			nl_dump(p, "[0x%u] ", key->offmask);

		nl_dump(p, "& 0x%08x == 0x%08x", ntohl(key->mask), ntohl(key->val));

		if (p->dp_type == NL_DUMP_STATS &&
		    (u->cu_mask & U32_ATTR_PCNT)) {
			struct tc_u32_pcnt *pcnt = u->cu_pcnt->d_data;
			nl_dump(p, " successful %" PRIu64, pcnt->kcnts[i]);
		}
	}
}

static void u32_dump_details(struct rtnl_tc *tc, void *data,
			     struct nl_dump_params *p)
{
	struct rtnl_u32 *u = data;
	struct tc_u32_sel *s;

	if (!u)
		return;

	if (!(u->cu_mask & U32_ATTR_SELECTOR)) {
		nl_dump(p, "no-selector\n");
		return;
	}
	
	s = u->cu_selector->d_data;

	nl_dump(p, "nkeys %u ", s->nkeys);

	if (u->cu_mask & U32_ATTR_HASH)
		nl_dump(p, "ht key 0x%x hash 0x%u",
			TC_U32_USERHTID(u->cu_hash), TC_U32_HASH(u->cu_hash));

	if (u->cu_mask & U32_ATTR_LINK)
		nl_dump(p, "link %u ", u->cu_link);

	if (u->cu_mask & U32_ATTR_INDEV)
		nl_dump(p, "indev %s ", u->cu_indev);

	print_selector(p, s, u);
	nl_dump(p, "\n");

#if 0	
#define U32_ATTR_ACTION       0x040
#define U32_ATTR_POLICE       0x080

	struct nl_data   act;
	struct nl_data   police;
#endif
}

static void u32_dump_stats(struct rtnl_tc *tc, void *data,
			   struct nl_dump_params *p)
{
	struct rtnl_u32 *u = data;

	if (!u)
		return;

	if (u->cu_mask & U32_ATTR_PCNT) {
		struct tc_u32_pcnt *pc = u->cu_pcnt->d_data;
		nl_dump(p, "\n");
		nl_dump_line(p, "    hit %8llu count %8llu\n",
			     pc->rhit, pc->rcnt);
	}
}

static int u32_msg_fill(struct rtnl_tc *tc, void *data, struct nl_msg *msg)
{
	struct rtnl_u32 *u = data;

	if (!u)
		return 0;
	
	if (u->cu_mask & U32_ATTR_DIVISOR)
		NLA_PUT_U32(msg, TCA_U32_DIVISOR, u->cu_divisor);

	if (u->cu_mask & U32_ATTR_HASH)
		NLA_PUT_U32(msg, TCA_U32_HASH, u->cu_hash);

	if (u->cu_mask & U32_ATTR_CLASSID)
		NLA_PUT_U32(msg, TCA_U32_CLASSID, u->cu_classid);

	if (u->cu_mask & U32_ATTR_LINK)
		NLA_PUT_U32(msg, TCA_U32_LINK, u->cu_link);

	if (u->cu_mask & U32_ATTR_SELECTOR)
		NLA_PUT_DATA(msg, TCA_U32_SEL, u->cu_selector);

	if (u->cu_mask & U32_ATTR_ACTION)
		NLA_PUT_DATA(msg, TCA_U32_ACT, u->cu_act);

	if (u->cu_mask & U32_ATTR_POLICE)
		NLA_PUT_DATA(msg, TCA_U32_POLICE, u->cu_police);

	if (u->cu_mask & U32_ATTR_INDEV)
		NLA_PUT_STRING(msg, TCA_U32_INDEV, u->cu_indev);

	return 0;

nla_put_failure:
	return -NLE_NOMEM;
}

/**
 * @name Attribute Modifications
 * @{
 */

void rtnl_u32_set_handle(struct rtnl_cls *cls, int htid, int hash,
			 int nodeid)
{
	uint32_t handle = (htid << 20) | (hash << 12) | nodeid;

	rtnl_tc_set_handle((struct rtnl_tc *) cls, handle );
}
 
int rtnl_u32_set_classid(struct rtnl_cls *cls, uint32_t classid)
{
	struct rtnl_u32 *u;

	if (!(u = rtnl_tc_data(TC_CAST(cls))))
		return -NLE_NOMEM;
	
	u->cu_classid = classid;
	u->cu_mask |= U32_ATTR_CLASSID;

	return 0;
}

/** @} */

/**
 * @name Selector Modifications
 * @{
 */

int rtnl_u32_set_flags(struct rtnl_cls *cls, int flags)
{
	struct tc_u32_sel *sel;
	struct rtnl_u32 *u;

	if (!(u = rtnl_tc_data(TC_CAST(cls))))
		return -NLE_NOMEM;

	sel = u32_selector_alloc(u);
	if (!sel)
		return -NLE_NOMEM;

	sel->flags |= flags;
	u->cu_mask |= U32_ATTR_SELECTOR;

	return 0;
}

/**
 * Append new 32-bit key to the selector
 *
 * @arg cls	classifier to be modifier
 * @arg val	value to be matched (network byte-order)
 * @arg mask	mask to be applied before matching (network byte-order)
 * @arg off	offset, in bytes, to start matching
 * @arg offmask	offset mask
 *
 * General selectors define the pattern, mask and offset the pattern will be
 * matched to the packet contents. Using the general selectors you can match
 * virtually any single bit in the IP (or upper layer) header.
 *
*/
int rtnl_u32_add_key(struct rtnl_cls *cls, uint32_t val, uint32_t mask,
		     int off, int offmask)
{
	struct tc_u32_sel *sel;
	struct rtnl_u32 *u;
	int err;

	if (!(u = rtnl_tc_data(TC_CAST(cls))))
		return -NLE_NOMEM;

	sel = u32_selector_alloc(u);
	if (!sel)
		return -NLE_NOMEM;

	err = nl_data_append(u->cu_selector, NULL, sizeof(struct tc_u32_key));
	if (err < 0)
		return err;

	/* the selector might have been moved by realloc */
	sel = u32_selector(u);

	sel->keys[sel->nkeys].mask = mask;
	sel->keys[sel->nkeys].val = val & mask;
	sel->keys[sel->nkeys].off = off;
	sel->keys[sel->nkeys].offmask = offmask;
	sel->nkeys++;
	u->cu_mask |= U32_ATTR_SELECTOR;

	return 0;
}

int rtnl_u32_add_key_uint8(struct rtnl_cls *cls, uint8_t val, uint8_t mask,
			   int off, int offmask)
{
	int shift = 24 - 8 * (off & 3);

	return rtnl_u32_add_key(cls, htonl((uint32_t)val << shift),
				htonl((uint32_t)mask << shift),
				off & ~3, offmask);
}

/**
 * Append new selector key to match a 16-bit number
 *
 * @arg cls	classifier to be modified
 * @arg val	value to be matched (host byte-order)
 * @arg mask	mask to be applied before matching (host byte-order)
 * @arg off	offset, in bytes, to start matching
 * @arg offmask	offset mask
*/
int rtnl_u32_add_key_uint16(struct rtnl_cls *cls, uint16_t val, uint16_t mask,
			    int off, int offmask)
{
	int shift = ((off & 3) == 0 ? 16 : 0);
	if (off % 2)
		return -NLE_INVAL;

	return rtnl_u32_add_key(cls, htonl((uint32_t)val << shift),
				htonl((uint32_t)mask << shift),
				off & ~3, offmask);
}

/**
 * Append new selector key to match a 32-bit number
 *
 * @arg cls	classifier to be modified
 * @arg val	value to be matched (host byte-order)
 * @arg mask	mask to be applied before matching (host byte-order)
 * @arg off	offset, in bytes, to start matching
 * @arg offmask	offset mask
*/
int rtnl_u32_add_key_uint32(struct rtnl_cls *cls, uint32_t val, uint32_t mask,
			    int off, int offmask)
{
	return rtnl_u32_add_key(cls, htonl(val), htonl(mask),
				off & ~3, offmask);
}

int rtnl_u32_add_key_in_addr(struct rtnl_cls *cls, struct in_addr *addr,
			     uint8_t bitmask, int off, int offmask)
{
	uint32_t mask = 0xFFFFFFFF << (32 - bitmask);
	return rtnl_u32_add_key(cls, addr->s_addr, htonl(mask), off, offmask);
}

int rtnl_u32_add_key_in6_addr(struct rtnl_cls *cls, struct in6_addr *addr,
			      uint8_t bitmask, int off, int offmask)
{
	int i, err;

	for (i = 1; i <= 4; i++) {
		if (32 * i - bitmask <= 0) {
			if ((err = rtnl_u32_add_key(cls, addr->s6_addr32[i-1],
						0xFFFFFFFF, off+4*(i-1), offmask)) < 0)
				return err;
		}
		else if (32 * i - bitmask < 32) {
			uint32_t mask = 0xFFFFFFFF << (32 * i - bitmask);
			if ((err = rtnl_u32_add_key(cls, addr->s6_addr32[i-1],
						htonl(mask), off+4*(i-1), offmask)) < 0)
				return err;
		}
		/* otherwise, if (32*i - bitmask >= 32) no key is generated */
	}

	return 0;
}

/** @} */

static struct rtnl_tc_ops u32_ops = {
	.to_kind		= "u32",
	.to_type		= RTNL_TC_TYPE_CLS,
	.to_size		= sizeof(struct rtnl_u32),
	.to_msg_parser		= u32_msg_parser,
	.to_free_data		= u32_free_data,
	.to_clone		= u32_clone,
	.to_msg_fill		= u32_msg_fill,
	.to_dump = {
	    [NL_DUMP_LINE]	= u32_dump_line,
	    [NL_DUMP_DETAILS]	= u32_dump_details,
	    [NL_DUMP_STATS]	= u32_dump_stats,
	},
};

static void __init u32_init(void)
{
	rtnl_tc_register(&u32_ops);
}

static void __exit u32_exit(void)
{
	rtnl_tc_unregister(&u32_ops);
}

/** @} */

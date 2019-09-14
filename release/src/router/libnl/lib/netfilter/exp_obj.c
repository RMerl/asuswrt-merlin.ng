/*
 * lib/netfilter/exp_obj.c	Conntrack Expectation Object
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2007 Philip Craig <philipc@snapgear.com>
 * Copyright (c) 2007 Secure Computing Corporation
 * Copyright (c) 2012 Rich Fought <rich.fought@watchguard.com>
 */

#include <sys/types.h>
#include <netinet/in.h>
#include <linux/netfilter/nfnetlink_conntrack.h>
#include <linux/netfilter/nf_conntrack_common.h>
#include <linux/netfilter/nf_conntrack_tcp.h>

#include <netlink-private/netlink.h>
#include <netlink/netfilter/nfnl.h>
#include <netlink/netfilter/exp.h>

// The 32-bit attribute mask in the common object header isn't
// big enough to handle all attributes of an expectation.  So
// we'll for sure specify optional attributes + parent attributes
// that are required for valid object comparison.  Comparison of
// these parent attributes will include nested attributes.

/** @cond SKIP */
#define EXP_ATTR_FAMILY			(1UL << 0) // 8-bit
#define EXP_ATTR_TIMEOUT		(1UL << 1) // 32-bit
#define EXP_ATTR_ID			(1UL << 2) // 32-bit
#define EXP_ATTR_HELPER_NAME		(1UL << 3) // string
#define EXP_ATTR_ZONE			(1UL << 4) // 16-bit
#define EXP_ATTR_FLAGS			(1UL << 5) // 32-bit
#define EXP_ATTR_CLASS			(1UL << 6) // 32-bit
#define EXP_ATTR_FN			(1UL << 7) // String
// Tuples
#define EXP_ATTR_EXPECT_IP_SRC		(1UL << 8)
#define EXP_ATTR_EXPECT_IP_DST		(1UL << 9)
#define EXP_ATTR_EXPECT_L4PROTO_NUM	(1UL << 10)
#define EXP_ATTR_EXPECT_L4PROTO_PORTS	(1UL << 11)
#define EXP_ATTR_EXPECT_L4PROTO_ICMP	(1UL << 12)
#define EXP_ATTR_MASTER_IP_SRC		(1UL << 13)
#define EXP_ATTR_MASTER_IP_DST		(1UL << 14)
#define EXP_ATTR_MASTER_L4PROTO_NUM	(1UL << 15)
#define EXP_ATTR_MASTER_L4PROTO_PORTS	(1UL << 16)
#define EXP_ATTR_MASTER_L4PROTO_ICMP	(1UL << 17)
#define EXP_ATTR_MASK_IP_SRC		(1UL << 18)
#define EXP_ATTR_MASK_IP_DST		(1UL << 19)
#define EXP_ATTR_MASK_L4PROTO_NUM	(1UL << 20)
#define EXP_ATTR_MASK_L4PROTO_PORTS	(1UL << 21)
#define EXP_ATTR_MASK_L4PROTO_ICMP	(1UL << 22)
#define EXP_ATTR_NAT_IP_SRC		(1UL << 23)
#define EXP_ATTR_NAT_IP_DST		(1UL << 24)
#define EXP_ATTR_NAT_L4PROTO_NUM	(1UL << 25)
#define EXP_ATTR_NAT_L4PROTO_PORTS	(1UL << 26)
#define EXP_ATTR_NAT_L4PROTO_ICMP	(1UL << 27)
#define EXP_ATTR_NAT_DIR		(1UL << 28)
/** @endcond */

static void exp_free_data(struct nl_object *c)
{
	struct nfnl_exp *exp = (struct nfnl_exp *) c;

	if (exp == NULL)
		return;

	nl_addr_put(exp->exp_expect.src);
	nl_addr_put(exp->exp_expect.dst);
	nl_addr_put(exp->exp_master.src);
	nl_addr_put(exp->exp_master.dst);
	nl_addr_put(exp->exp_mask.src);
	nl_addr_put(exp->exp_mask.dst);
	nl_addr_put(exp->exp_nat.src);
	nl_addr_put(exp->exp_nat.dst);

	free(exp->exp_fn);
	free(exp->exp_helper_name);
}

static int exp_clone(struct nl_object *_dst, struct nl_object *_src)
{
	struct nfnl_exp *dst = (struct nfnl_exp *) _dst;
	struct nfnl_exp *src = (struct nfnl_exp *) _src;
	struct nl_addr *addr;

	// Expectation
	if (src->exp_expect.src) {
		addr = nl_addr_clone(src->exp_expect.src);
		if (!addr)
			return -NLE_NOMEM;
		dst->exp_expect.src = addr;
	}

	if (src->exp_expect.dst) {
		addr = nl_addr_clone(src->exp_expect.dst);
		if (!addr)
			return -NLE_NOMEM;
		dst->exp_expect.dst = addr;
	}

	// Master CT
	if (src->exp_master.src) {
		addr = nl_addr_clone(src->exp_master.src);
		if (!addr)
			return -NLE_NOMEM;
		dst->exp_master.src = addr;
	}

	if (src->exp_master.dst) {
		addr = nl_addr_clone(src->exp_master.dst);
		if (!addr)
			return -NLE_NOMEM;
		dst->exp_master.dst = addr;
	}

	// Mask
	if (src->exp_mask.src) {
		addr = nl_addr_clone(src->exp_mask.src);
		if (!addr)
			return -NLE_NOMEM;
		dst->exp_mask.src = addr;
	}

	if (src->exp_mask.dst) {
		addr = nl_addr_clone(src->exp_mask.dst);
		if (!addr)
			return -NLE_NOMEM;
		dst->exp_mask.dst = addr;
	}

    // NAT
	if (src->exp_nat.src) {
		addr = nl_addr_clone(src->exp_nat.src);
		if (!addr)
			return -NLE_NOMEM;
		dst->exp_nat.src = addr;
	}

	if (src->exp_nat.dst) {
		addr = nl_addr_clone(src->exp_nat.dst);
		if (!addr)
			return -NLE_NOMEM;
		dst->exp_nat.dst = addr;
	}

	if (src->exp_fn)
		dst->exp_fn = strdup(src->exp_fn);

	if (src->exp_helper_name)
		dst->exp_helper_name = strdup(src->exp_helper_name);

	return 0;
}

static void dump_addr(struct nl_dump_params *p, struct nl_addr *addr, int port)
{
	char buf[64];

	if (addr)
		nl_dump(p, "%s", nl_addr2str(addr, buf, sizeof(buf)));

	if (port)
		nl_dump(p, ":%u ", port);
	else if (addr)
		nl_dump(p, " ");
}

static void dump_icmp(struct nl_dump_params *p, struct nfnl_exp *exp, int tuple)
{
	if (nfnl_exp_test_icmp(exp, tuple)) {

		nl_dump(p, "icmp type %d ", nfnl_exp_get_icmp_type(exp, tuple));

		nl_dump(p, "code %d ", nfnl_exp_get_icmp_code(exp, tuple));

		nl_dump(p, "id %d ", nfnl_exp_get_icmp_id(exp, tuple));
	}
}

static void exp_dump_tuples(struct nfnl_exp *exp, struct nl_dump_params *p)
{
	struct nl_addr *tuple_src, *tuple_dst;
	int tuple_sport, tuple_dport;
	int i = 0;
	char buf[64];

	for (i = NFNL_EXP_TUPLE_EXPECT; i < NFNL_EXP_TUPLE_MAX; i++) {
		tuple_src = NULL;
		tuple_dst = NULL;
		tuple_sport = 0;
		tuple_dport = 0;

		// Test needed for NAT case
		if (nfnl_exp_test_src(exp, i))
			tuple_src = nfnl_exp_get_src(exp, i);
		if (nfnl_exp_test_dst(exp, i))
			tuple_dst = nfnl_exp_get_dst(exp, i);

        // Don't have tests for individual ports/types/codes/ids,
		if (nfnl_exp_test_l4protonum(exp, i)) {
			nl_dump(p, "%s ",
				nl_ip_proto2str(nfnl_exp_get_l4protonum(exp, i), buf, sizeof(buf)));
		}

		if (nfnl_exp_test_ports(exp, i)) {
			tuple_sport = nfnl_exp_get_src_port(exp, i);
			tuple_dport = nfnl_exp_get_dst_port(exp, i);
		}

		dump_addr(p, tuple_src, tuple_sport);
		dump_addr(p, tuple_dst, tuple_dport);
		dump_icmp(p, exp, 0);
	}

	if (nfnl_exp_test_nat_dir(exp))
		nl_dump(p, "nat dir %s ", exp->exp_nat_dir);

}

/* FIXME Compatible with /proc/net/nf_conntrack */
static void exp_dump_line(struct nl_object *a, struct nl_dump_params *p)
{
	struct nfnl_exp *exp = (struct nfnl_exp *) a;

	nl_new_line(p);

	exp_dump_tuples(exp, p);

	nl_dump(p, "\n");
}

static void exp_dump_details(struct nl_object *a, struct nl_dump_params *p)
{
	struct nfnl_exp *exp = (struct nfnl_exp *) a;
	char buf[64];
	int fp = 0;

	exp_dump_line(a, p);

	nl_dump(p, "    id 0x%x ", exp->exp_id);
	nl_dump_line(p, "family %s ",
		nl_af2str(exp->exp_family, buf, sizeof(buf)));

	if (nfnl_exp_test_timeout(exp)) {
		uint64_t timeout_ms = nfnl_exp_get_timeout(exp) * 1000UL;
		nl_dump(p, "timeout %s ",
			nl_msec2str(timeout_ms, buf, sizeof(buf)));
	}

	if (nfnl_exp_test_helper_name(exp))
		nl_dump(p, "helper %s ", exp->exp_helper_name);

	if (nfnl_exp_test_fn(exp))
		nl_dump(p, "fn %s ", exp->exp_fn);

	if (nfnl_exp_test_class(exp))
		nl_dump(p, "class %u ", nfnl_exp_get_class(exp));

	if (nfnl_exp_test_zone(exp))
		nl_dump(p, "zone %u ", nfnl_exp_get_zone(exp));

	if (nfnl_exp_test_flags(exp))
		nl_dump(p, "<");
#define PRINT_FLAG(str) \
	{ nl_dump(p, "%s%s", fp++ ? "," : "", (str)); }

	if (exp->exp_flags & NF_CT_EXPECT_PERMANENT)
		PRINT_FLAG("PERMANENT");
	if (exp->exp_flags & NF_CT_EXPECT_INACTIVE)
		PRINT_FLAG("INACTIVE");
	if (exp->exp_flags & NF_CT_EXPECT_USERSPACE)
		PRINT_FLAG("USERSPACE");
#undef PRINT_FLAG

	if (nfnl_exp_test_flags(exp))
		nl_dump(p, ">");

	nl_dump(p, "\n");
}

static int exp_cmp_l4proto_ports (union nfnl_exp_protodata *a, union nfnl_exp_protodata *b) {
	// Must return 0 for match, 1 for mismatch
	int d = 0;
	d = ( (a->port.src != b->port.src) ||
		  (a->port.dst != b->port.dst) );

	return d;
}

static int exp_cmp_l4proto_icmp (union nfnl_exp_protodata *a, union nfnl_exp_protodata *b) {
    // Must return 0 for match, 1 for mismatch
	int d = 0;
	d = ( (a->icmp.code != b->icmp.code) ||
		  (a->icmp.type != b->icmp.type) ||
		  (a->icmp.id != b->icmp.id) );

	return d;
}

static int exp_compare(struct nl_object *_a, struct nl_object *_b,
							uint32_t attrs, int flags)
{
	struct nfnl_exp *a = (struct nfnl_exp *) _a;
	struct nfnl_exp *b = (struct nfnl_exp *) _b;
	int diff = 0;

#define EXP_DIFF(ATTR, EXPR) ATTR_DIFF(attrs, EXP_ATTR_##ATTR, a, b, EXPR)
#define EXP_DIFF_VAL(ATTR, FIELD) EXP_DIFF(ATTR, a->FIELD != b->FIELD)
#define EXP_DIFF_STRING(ATTR, FIELD) EXP_DIFF(ATTR, (strcmp(a->FIELD, b->FIELD) != 0))
#define EXP_DIFF_ADDR(ATTR, FIELD) \
		((flags & LOOSE_COMPARISON) \
		? EXP_DIFF(ATTR, nl_addr_cmp_prefix(a->FIELD, b->FIELD)) \
		: EXP_DIFF(ATTR, nl_addr_cmp(a->FIELD, b->FIELD)))
#define EXP_DIFF_L4PROTO_PORTS(ATTR, FIELD) \
		EXP_DIFF(ATTR, exp_cmp_l4proto_ports(&(a->FIELD), &(b->FIELD)))
#define EXP_DIFF_L4PROTO_ICMP(ATTR, FIELD) \
		EXP_DIFF(ATTR, exp_cmp_l4proto_icmp(&(a->FIELD), &(b->FIELD)))

		diff |= EXP_DIFF_VAL(FAMILY,			exp_family);
		diff |= EXP_DIFF_VAL(TIMEOUT,			exp_timeout);
		diff |= EXP_DIFF_VAL(ID,			exp_id);
		diff |= EXP_DIFF_VAL(ZONE,			exp_zone);
		diff |= EXP_DIFF_VAL(CLASS,			exp_class);
		diff |= EXP_DIFF_VAL(FLAGS,			exp_flags);
		diff |= EXP_DIFF_VAL(NAT_DIR,			exp_nat_dir);

		diff |= EXP_DIFF_STRING(FN,			exp_fn);
		diff |= EXP_DIFF_STRING(HELPER_NAME,		exp_helper_name);

		diff |= EXP_DIFF_ADDR(EXPECT_IP_SRC,			exp_expect.src);
		diff |= EXP_DIFF_ADDR(EXPECT_IP_DST,			exp_expect.dst);
		diff |= EXP_DIFF_VAL(EXPECT_L4PROTO_NUM,		exp_expect.proto.l4protonum);
		diff |= EXP_DIFF_L4PROTO_PORTS(EXPECT_L4PROTO_PORTS,	exp_expect.proto.l4protodata);
		diff |= EXP_DIFF_L4PROTO_ICMP(EXPECT_L4PROTO_ICMP,	exp_expect.proto.l4protodata);

		diff |= EXP_DIFF_ADDR(MASTER_IP_SRC,			exp_master.src);
		diff |= EXP_DIFF_ADDR(MASTER_IP_DST,			exp_master.dst);
		diff |= EXP_DIFF_VAL(MASTER_L4PROTO_NUM,		exp_master.proto.l4protonum);
		diff |= EXP_DIFF_L4PROTO_PORTS(MASTER_L4PROTO_PORTS,	exp_master.proto.l4protodata);
		diff |= EXP_DIFF_L4PROTO_ICMP(MASTER_L4PROTO_ICMP,	exp_master.proto.l4protodata);

		diff |= EXP_DIFF_ADDR(MASK_IP_SRC,			exp_mask.src);
		diff |= EXP_DIFF_ADDR(MASK_IP_DST,			exp_mask.dst);
		diff |= EXP_DIFF_VAL(MASK_L4PROTO_NUM,			exp_mask.proto.l4protonum);
		diff |= EXP_DIFF_L4PROTO_PORTS(MASK_L4PROTO_PORTS,	exp_mask.proto.l4protodata);
		diff |= EXP_DIFF_L4PROTO_ICMP(MASK_L4PROTO_ICMP,	exp_mask.proto.l4protodata);

		diff |= EXP_DIFF_ADDR(NAT_IP_SRC,			exp_nat.src);
		diff |= EXP_DIFF_ADDR(NAT_IP_DST,			exp_nat.dst);
		diff |= EXP_DIFF_VAL(NAT_L4PROTO_NUM,			exp_nat.proto.l4protonum);
		diff |= EXP_DIFF_L4PROTO_PORTS(NAT_L4PROTO_PORTS,	exp_nat.proto.l4protodata);
		diff |= EXP_DIFF_L4PROTO_ICMP(NAT_L4PROTO_ICMP,		exp_nat.proto.l4protodata);

#undef EXP_DIFF
#undef EXP_DIFF_VAL
#undef EXP_DIFF_STRING
#undef EXP_DIFF_ADDR
#undef EXP_DIFF_L4PROTO_PORTS
#undef EXP_DIFF_L4PROTO_ICMP

	return diff;
}

// CLI arguments?
static const struct trans_tbl exp_attrs[] = {
	__ADD(EXP_ATTR_FAMILY,				family)
	__ADD(EXP_ATTR_TIMEOUT,				timeout)
	__ADD(EXP_ATTR_ID,				id)
	__ADD(EXP_ATTR_HELPER_NAME,			helpername)
	__ADD(EXP_ATTR_ZONE,				zone)
	__ADD(EXP_ATTR_CLASS,				class)
	__ADD(EXP_ATTR_FLAGS,				flags)
	__ADD(EXP_ATTR_FN,				function)
	__ADD(EXP_ATTR_EXPECT_IP_SRC,			expectipsrc)
	__ADD(EXP_ATTR_EXPECT_IP_DST,			expectipdst)
	__ADD(EXP_ATTR_EXPECT_L4PROTO_NUM,		expectprotonum)
	__ADD(EXP_ATTR_EXPECT_L4PROTO_PORTS,		expectports)
	__ADD(EXP_ATTR_EXPECT_L4PROTO_ICMP,		expecticmp)
	__ADD(EXP_ATTR_MASTER_IP_SRC,			masteripsrc)
	__ADD(EXP_ATTR_MASTER_IP_DST,			masteripdst)
	__ADD(EXP_ATTR_MASTER_L4PROTO_NUM,		masterprotonum)
	__ADD(EXP_ATTR_MASTER_L4PROTO_PORTS,		masterports)
	__ADD(EXP_ATTR_MASTER_L4PROTO_ICMP,		mastericmp)
	__ADD(EXP_ATTR_MASK_IP_SRC,			maskipsrc)
	__ADD(EXP_ATTR_MASK_IP_DST,			maskipdst)
	__ADD(EXP_ATTR_MASK_L4PROTO_NUM,		maskprotonum)
	__ADD(EXP_ATTR_MASK_L4PROTO_PORTS,		maskports)
	__ADD(EXP_ATTR_MASK_L4PROTO_ICMP,		maskicmp)
	__ADD(EXP_ATTR_NAT_IP_SRC,			natipsrc)
	__ADD(EXP_ATTR_NAT_IP_DST,			natipdst)
	__ADD(EXP_ATTR_NAT_L4PROTO_NUM,			natprotonum)
	__ADD(EXP_ATTR_NAT_L4PROTO_PORTS,		natports)
	__ADD(EXP_ATTR_NAT_L4PROTO_ICMP,		naticmp)
	__ADD(EXP_ATTR_NAT_DIR,				natdir)
};

static char *exp_attrs2str(int attrs, char *buf, size_t len)
{
	return __flags2str(attrs, buf, len, exp_attrs, ARRAY_SIZE(exp_attrs));
}

/**
 * @name Allocation/Freeing
 * @{
 */

struct nfnl_exp *nfnl_exp_alloc(void)
{
	return (struct nfnl_exp *) nl_object_alloc(&exp_obj_ops);
}

void nfnl_exp_get(struct nfnl_exp *exp)
{
	nl_object_get((struct nl_object *) exp);
}

void nfnl_exp_put(struct nfnl_exp *exp)
{
	nl_object_put((struct nl_object *) exp);
}

/** @} */

/**
 * @name Attributes
 * @{
 */

void nfnl_exp_set_family(struct nfnl_exp *exp, uint8_t family)
{
	exp->exp_family = family;
	exp->ce_mask |= EXP_ATTR_FAMILY;
}

uint8_t nfnl_exp_get_family(const struct nfnl_exp *exp)
{
	if (exp->ce_mask & EXP_ATTR_FAMILY)
		return exp->exp_family;
	else
		return AF_UNSPEC;
}

void nfnl_exp_set_flags(struct nfnl_exp *exp, uint32_t flags)
{
	exp->exp_flags |= flags;
	exp->ce_mask |= EXP_ATTR_FLAGS;
}

int nfnl_exp_test_flags(const struct nfnl_exp *exp)
{
	return !!(exp->ce_mask & EXP_ATTR_FLAGS);
}

void nfnl_exp_unset_flags(struct nfnl_exp *exp, uint32_t flags)
{
	exp->exp_flags &= ~flags;
	exp->ce_mask |= EXP_ATTR_FLAGS;
}

uint32_t nfnl_exp_get_flags(const struct nfnl_exp *exp)
{
	return exp->exp_flags;
}

static const struct trans_tbl flag_table[] = {
	__ADD(IPS_EXPECTED, expected)
	__ADD(IPS_SEEN_REPLY, seen_reply)
	__ADD(IPS_ASSURED, assured)
};

char * nfnl_exp_flags2str(int flags, char *buf, size_t len)
{
	return __flags2str(flags, buf, len, flag_table,
			   ARRAY_SIZE(flag_table));
}

int nfnl_exp_str2flags(const char *name)
{
	return __str2flags(name, flag_table, ARRAY_SIZE(flag_table));
}

void nfnl_exp_set_timeout(struct nfnl_exp *exp, uint32_t timeout)
{
	exp->exp_timeout = timeout;
	exp->ce_mask |= EXP_ATTR_TIMEOUT;
}

int nfnl_exp_test_timeout(const struct nfnl_exp *exp)
{
	return !!(exp->ce_mask & EXP_ATTR_TIMEOUT);
}

uint32_t nfnl_exp_get_timeout(const struct nfnl_exp *exp)
{
	return exp->exp_timeout;
}

void nfnl_exp_set_id(struct nfnl_exp *exp, uint32_t id)
{
	exp->exp_id = id;
	exp->ce_mask |= EXP_ATTR_ID;
}

int nfnl_exp_test_id(const struct nfnl_exp *exp)
{
	return !!(exp->ce_mask & EXP_ATTR_ID);
}

uint32_t nfnl_exp_get_id(const struct nfnl_exp *exp)
{
	return exp->exp_id;
}

int nfnl_exp_set_helper_name(struct nfnl_exp *exp, void *name)
{
	free(exp->exp_helper_name);
	exp->exp_helper_name = strdup(name);
	if (!exp->exp_helper_name)
		return -NLE_NOMEM;

	exp->ce_mask |= EXP_ATTR_HELPER_NAME;
	return 0;
}

int  nfnl_exp_test_helper_name(const struct nfnl_exp *exp)
{
	return !!(exp->ce_mask & EXP_ATTR_HELPER_NAME);
}

const char * nfnl_exp_get_helper_name(const struct nfnl_exp *exp)
{
	return exp->exp_helper_name;
}

void nfnl_exp_set_zone(struct nfnl_exp *exp, uint16_t zone)
{
	exp->exp_zone = zone;
	exp->ce_mask |= EXP_ATTR_ZONE;
}

int nfnl_exp_test_zone(const struct nfnl_exp *exp)
{
	return !!(exp->ce_mask & EXP_ATTR_ZONE);
}

uint16_t nfnl_exp_get_zone(const struct nfnl_exp *exp)
{
	return exp->exp_zone;
}

void nfnl_exp_set_class(struct nfnl_exp *exp, uint32_t class)
{
	exp->exp_class = class;
	exp->ce_mask |= EXP_ATTR_CLASS;
}

int nfnl_exp_test_class(const struct nfnl_exp *exp)
{
	return !!(exp->ce_mask & EXP_ATTR_CLASS);
}

uint32_t nfnl_exp_get_class(const struct nfnl_exp *exp)
{
	return exp->exp_class;
}

int nfnl_exp_set_fn(struct nfnl_exp *exp, void *fn)
{
	free(exp->exp_fn);
	exp->exp_fn = strdup(fn);
	if (!exp->exp_fn)
		return -NLE_NOMEM;

	exp->ce_mask |= EXP_ATTR_FN;
	return 0;
}

int nfnl_exp_test_fn(const struct nfnl_exp *exp)
{
	return !!(exp->ce_mask & EXP_ATTR_FN);
}

const char * nfnl_exp_get_fn(const struct nfnl_exp *exp)
{
	return exp->exp_fn;
}

void nfnl_exp_set_nat_dir(struct nfnl_exp *exp, uint8_t nat_dir)
{
	exp->exp_nat_dir = nat_dir;
	exp->ce_mask |= EXP_ATTR_NAT_DIR;
}

int nfnl_exp_test_nat_dir(const struct nfnl_exp *exp)
{
	return !!(exp->ce_mask & EXP_ATTR_NAT_DIR);
}

uint8_t nfnl_exp_get_nat_dir(const struct nfnl_exp *exp)
{
	return exp->exp_nat_dir;
}

#define EXP_GET_TUPLE(e, t) \
	(t == NFNL_EXP_TUPLE_MASTER) ? \
		&(e->exp_master) : \
	(t == NFNL_EXP_TUPLE_MASK) ? \
		&(e->exp_mask) : \
	(t == NFNL_EXP_TUPLE_NAT) ? \
		&(e->exp_nat) : &(exp->exp_expect)

static int exp_get_src_attr(int tuple)
{
	int attr = 0;

	switch (tuple) {
		case NFNL_EXP_TUPLE_MASTER:
			attr = EXP_ATTR_MASTER_IP_SRC;
			break;
		case NFNL_EXP_TUPLE_MASK:
			attr = EXP_ATTR_MASK_IP_SRC;
			break;
		case NFNL_EXP_TUPLE_NAT:
			attr = EXP_ATTR_NAT_IP_SRC;
			break;
		case NFNL_EXP_TUPLE_EXPECT:
		default :
			attr = EXP_ATTR_EXPECT_IP_SRC;
	}

	return attr;
}

static int exp_get_dst_attr(int tuple)
{
	int attr = 0;

	switch (tuple) {
		case NFNL_EXP_TUPLE_MASTER:
			attr = EXP_ATTR_MASTER_IP_DST;
			break;
		case NFNL_EXP_TUPLE_MASK:
			attr = EXP_ATTR_MASK_IP_DST;
			break;
		case NFNL_EXP_TUPLE_NAT:
			attr = EXP_ATTR_NAT_IP_DST;
			break;
		case NFNL_EXP_TUPLE_EXPECT:
		default :
			attr = EXP_ATTR_EXPECT_IP_DST;
	}

	return attr;
}


static int exp_set_addr(struct nfnl_exp *exp, struct nl_addr *addr,
                          int attr, struct nl_addr ** exp_addr)
{
	if (exp->ce_mask & EXP_ATTR_FAMILY) {
		if (addr->a_family != exp->exp_family)
			return -NLE_AF_MISMATCH;
	} else
		nfnl_exp_set_family(exp, addr->a_family);

	if (*exp_addr)
		nl_addr_put(*exp_addr);

	nl_addr_get(addr);
	*exp_addr = addr;
	exp->ce_mask |= attr;

	return 0;
}

int nfnl_exp_set_src(struct nfnl_exp *exp, int tuple, struct nl_addr *addr)
{
	struct nfnl_exp_dir *dir = EXP_GET_TUPLE(exp, tuple);

	return exp_set_addr(exp, addr, exp_get_src_attr(tuple), &dir->src);
}

int nfnl_exp_set_dst(struct nfnl_exp *exp, int tuple, struct nl_addr *addr)
{
	struct nfnl_exp_dir *dir = EXP_GET_TUPLE(exp, tuple);

	return exp_set_addr(exp, addr, exp_get_dst_attr(tuple), &dir->dst);
}

int nfnl_exp_test_src(const struct nfnl_exp *exp, int tuple)
{
	return !!(exp->ce_mask & exp_get_src_attr(tuple));
}

int nfnl_exp_test_dst(const struct nfnl_exp *exp, int tuple)
{
	return !!(exp->ce_mask & exp_get_dst_attr(tuple));
}

struct nl_addr *nfnl_exp_get_src(const struct nfnl_exp *exp, int tuple)
{
	const struct nfnl_exp_dir *dir = EXP_GET_TUPLE(exp, tuple);

	if (!(exp->ce_mask & exp_get_src_attr(tuple)))
		return NULL;
	return dir->src;
}

struct nl_addr *nfnl_exp_get_dst(const struct nfnl_exp *exp, int tuple)
{
	const struct nfnl_exp_dir *dir = EXP_GET_TUPLE(exp, tuple);

	if (!(exp->ce_mask & exp_get_dst_attr(tuple)))
		return NULL;
	return dir->dst;
}

static int exp_get_l4protonum_attr(int tuple)
{
	int attr = 0;

	switch (tuple) {
		case NFNL_EXP_TUPLE_MASTER:
			attr = EXP_ATTR_MASTER_L4PROTO_NUM;
			break;
		case NFNL_EXP_TUPLE_MASK:
			attr = EXP_ATTR_MASK_L4PROTO_NUM;
			break;
		case NFNL_EXP_TUPLE_NAT:
			attr = EXP_ATTR_NAT_L4PROTO_NUM;
			break;
		case NFNL_EXP_TUPLE_EXPECT:
		default :
			attr = EXP_ATTR_EXPECT_L4PROTO_NUM;
	}

	return attr;
}

void nfnl_exp_set_l4protonum(struct nfnl_exp *exp, int tuple, uint8_t l4protonum)
{
	struct nfnl_exp_dir *dir = EXP_GET_TUPLE(exp, tuple);

	dir->proto.l4protonum = l4protonum;
	exp->ce_mask |= exp_get_l4protonum_attr(tuple);
}

int nfnl_exp_test_l4protonum(const struct nfnl_exp *exp, int tuple)
{
	return !!(exp->ce_mask & exp_get_l4protonum_attr(tuple));
}

uint8_t nfnl_exp_get_l4protonum(const struct nfnl_exp *exp, int tuple)
{
	const struct nfnl_exp_dir *dir = EXP_GET_TUPLE(exp, tuple);
	return dir->proto.l4protonum;
}

static int exp_get_l4ports_attr(int tuple)
{
	int attr = 0;

	switch (tuple) {
		case NFNL_EXP_TUPLE_MASTER:
			attr = EXP_ATTR_MASTER_L4PROTO_PORTS;
			break;
		case NFNL_EXP_TUPLE_MASK:
			attr = EXP_ATTR_MASK_L4PROTO_PORTS;
			break;
		case NFNL_EXP_TUPLE_NAT:
			attr = EXP_ATTR_NAT_L4PROTO_PORTS;
			break;
		case NFNL_EXP_TUPLE_EXPECT:
		default :
			attr = EXP_ATTR_EXPECT_L4PROTO_PORTS;
	}

	return attr;
}

void nfnl_exp_set_ports(struct nfnl_exp *exp, int tuple, uint16_t srcport, uint16_t dstport)
{
	struct nfnl_exp_dir *dir = EXP_GET_TUPLE(exp, tuple);

	dir->proto.l4protodata.port.src = srcport;
	dir->proto.l4protodata.port.dst = dstport;

	exp->ce_mask |= exp_get_l4ports_attr(tuple);
}

int nfnl_exp_test_ports(const struct nfnl_exp *exp, int tuple)
{
	return !!(exp->ce_mask & exp_get_l4ports_attr(tuple));
}

uint16_t nfnl_exp_get_src_port(const struct nfnl_exp *exp, int tuple)
{
	const struct nfnl_exp_dir *dir = EXP_GET_TUPLE(exp, tuple);
	return dir->proto.l4protodata.port.src;
}

uint16_t nfnl_exp_get_dst_port(const struct nfnl_exp *exp, int tuple)
{
	const struct nfnl_exp_dir *dir = EXP_GET_TUPLE(exp, tuple);

	return dir->proto.l4protodata.port.dst;
}

static int exp_get_l4icmp_attr(int tuple)
{
	int attr = 0;

	switch (tuple) {
		case NFNL_EXP_TUPLE_MASTER:
			attr = EXP_ATTR_MASTER_L4PROTO_ICMP;
			break;
		case NFNL_EXP_TUPLE_MASK:
			attr = EXP_ATTR_MASK_L4PROTO_ICMP;
			break;
		case NFNL_EXP_TUPLE_NAT:
			attr = EXP_ATTR_NAT_L4PROTO_ICMP;
			break;
		case NFNL_EXP_TUPLE_EXPECT:
		default :
			attr = EXP_ATTR_EXPECT_L4PROTO_ICMP;
	}

	return attr;
}

void nfnl_exp_set_icmp(struct nfnl_exp *exp, int tuple, uint16_t id, uint8_t type, uint8_t code)
{
	struct nfnl_exp_dir *dir = EXP_GET_TUPLE(exp, tuple);

	dir->proto.l4protodata.icmp.id = id;
	dir->proto.l4protodata.icmp.type = type;
	dir->proto.l4protodata.icmp.code = code;

	exp->ce_mask |= exp_get_l4icmp_attr(tuple);
}

int nfnl_exp_test_icmp(const struct nfnl_exp *exp, int tuple)
{
	int attr = exp_get_l4icmp_attr(tuple);
	return !!(exp->ce_mask & attr);
}

uint16_t nfnl_exp_get_icmp_id(const struct nfnl_exp *exp, int tuple)
{
	const struct nfnl_exp_dir *dir = EXP_GET_TUPLE(exp, tuple);

	return dir->proto.l4protodata.icmp.id;
}

uint8_t nfnl_exp_get_icmp_type(const struct nfnl_exp *exp, int tuple)
{
	const struct nfnl_exp_dir *dir = EXP_GET_TUPLE(exp, tuple);

	return dir->proto.l4protodata.icmp.type;
}

uint8_t nfnl_exp_get_icmp_code(const struct nfnl_exp *exp, int tuple)
{
	const struct nfnl_exp_dir *dir = EXP_GET_TUPLE(exp, tuple);

	return dir->proto.l4protodata.icmp.code;
}

/** @} */

struct nl_object_ops exp_obj_ops = {
	.oo_name	= "netfilter/exp",
	.oo_size	= sizeof(struct nfnl_exp),
	.oo_free_data   = exp_free_data,
	.oo_clone	= exp_clone,
	.oo_dump = {
		[NL_DUMP_LINE]		= exp_dump_line,
		[NL_DUMP_DETAILS]	= exp_dump_details,
	},
	.oo_compare	= exp_compare,
	.oo_attrs2str	= exp_attrs2str,
};

/** @} */

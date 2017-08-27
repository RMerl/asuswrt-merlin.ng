/*
 * lib/route/route_obj.c	Route Object
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup route
 * @defgroup route_obj Route Object
 *
 * @par Attributes
 * @code
 * Name                                           Default
 * -------------------------------------------------------------
 * routing table                                  RT_TABLE_MAIN
 * scope                                          RT_SCOPE_NOWHERE
 * tos                                            0
 * protocol                                       RTPROT_STATIC
 * prio                                           0
 * family                                         AF_UNSPEC
 * type                                           RTN_UNICAST
 * iif                                            NULL
 * @endcode
 *
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/utils.h>
#include <netlink/data.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/route.h>
#include <netlink/route/link.h>
#include <netlink/route/nexthop.h>

/** @cond SKIP */
#define ROUTE_ATTR_FAMILY    0x000001
#define ROUTE_ATTR_TOS       0x000002
#define ROUTE_ATTR_TABLE     0x000004
#define ROUTE_ATTR_PROTOCOL  0x000008
#define ROUTE_ATTR_SCOPE     0x000010
#define ROUTE_ATTR_TYPE      0x000020
#define ROUTE_ATTR_FLAGS     0x000040
#define ROUTE_ATTR_DST       0x000080
#define ROUTE_ATTR_SRC       0x000100
#define ROUTE_ATTR_IIF       0x000200
#define ROUTE_ATTR_OIF       0x000400
#define ROUTE_ATTR_GATEWAY   0x000800
#define ROUTE_ATTR_PRIO      0x001000
#define ROUTE_ATTR_PREF_SRC  0x002000
#define ROUTE_ATTR_METRICS   0x004000
#define ROUTE_ATTR_MULTIPATH 0x008000
#define ROUTE_ATTR_REALMS    0x010000
#define ROUTE_ATTR_CACHEINFO 0x020000
/** @endcond */

static void route_constructor(struct nl_object *c)
{
	struct rtnl_route *r = (struct rtnl_route *) c;

	r->rt_family = AF_UNSPEC;
	r->rt_scope = RT_SCOPE_NOWHERE;
	r->rt_table = RT_TABLE_MAIN;
	r->rt_protocol = RTPROT_STATIC;
	r->rt_type = RTN_UNICAST;

	nl_init_list_head(&r->rt_nexthops);
}

static void route_free_data(struct nl_object *c)
{
	struct rtnl_route *r = (struct rtnl_route *) c;
	struct rtnl_nexthop *nh, *tmp;

	if (r == NULL)
		return;

	nl_addr_put(r->rt_dst);
	nl_addr_put(r->rt_src);
	nl_addr_put(r->rt_pref_src);

	nl_list_for_each_entry_safe(nh, tmp, &r->rt_nexthops, rtnh_list) {
		rtnl_route_remove_nexthop(r, nh);
		rtnl_route_nh_free(nh);
	}
}

static int route_clone(struct nl_object *_dst, struct nl_object *_src)
{
	struct rtnl_route *dst = (struct rtnl_route *) _dst;
	struct rtnl_route *src = (struct rtnl_route *) _src;
	struct rtnl_nexthop *nh, *new;

	if (src->rt_dst)
		if (!(dst->rt_dst = nl_addr_clone(src->rt_dst)))
			return -NLE_NOMEM;

	if (src->rt_src)
		if (!(dst->rt_src = nl_addr_clone(src->rt_src)))
			return -NLE_NOMEM;

	if (src->rt_pref_src)
		if (!(dst->rt_pref_src = nl_addr_clone(src->rt_pref_src)))
			return -NLE_NOMEM;

	nl_init_list_head(&dst->rt_nexthops);
	nl_list_for_each_entry(nh, &src->rt_nexthops, rtnh_list) {
		new = rtnl_route_nh_clone(nh);
		if (!new)
			return -NLE_NOMEM;

		rtnl_route_add_nexthop(dst, new);
	}

	return 0;
}

static void route_dump_line(struct nl_object *a, struct nl_dump_params *p)
{
	struct rtnl_route *r = (struct rtnl_route *) a;
	int cache = 0, flags;
	char buf[64];

	if (r->rt_flags & RTM_F_CLONED)
		cache = 1;

	nl_dump_line(p, "%s ", nl_af2str(r->rt_family, buf, sizeof(buf)));

	if (cache)
		nl_dump(p, "cache ");

	if (!(r->ce_mask & ROUTE_ATTR_DST) ||
	    nl_addr_get_len(r->rt_dst) == 0)
		nl_dump(p, "default ");
	else
		nl_dump(p, "%s ", nl_addr2str(r->rt_dst, buf, sizeof(buf)));

	if (r->ce_mask & ROUTE_ATTR_TABLE && !cache)
		nl_dump(p, "table %s ",
			rtnl_route_table2str(r->rt_table, buf, sizeof(buf)));

	if (r->ce_mask & ROUTE_ATTR_TYPE)
		nl_dump(p, "type %s ",
			nl_rtntype2str(r->rt_type, buf, sizeof(buf)));

	if (r->ce_mask & ROUTE_ATTR_TOS && r->rt_tos != 0)
		nl_dump(p, "tos %#x ", r->rt_tos);

	if (r->ce_mask & ROUTE_ATTR_MULTIPATH) {
		struct rtnl_nexthop *nh;

		nl_list_for_each_entry(nh, &r->rt_nexthops, rtnh_list) {
			p->dp_ivar = NH_DUMP_FROM_ONELINE;
			rtnl_route_nh_dump(nh, p);
		}
	}

	flags = r->rt_flags & ~(RTM_F_CLONED);
	if (r->ce_mask & ROUTE_ATTR_FLAGS && flags) {

		nl_dump(p, "<");

#define PRINT_FLAG(f) if (flags & RTNH_F_##f) { \
		flags &= ~RTNH_F_##f; nl_dump(p, #f "%s", flags ? "," : ""); }
		PRINT_FLAG(DEAD);
		PRINT_FLAG(ONLINK);
		PRINT_FLAG(PERVASIVE);
#undef PRINT_FLAG

#define PRINT_FLAG(f) if (flags & RTM_F_##f) { \
		flags &= ~RTM_F_##f; nl_dump(p, #f "%s", flags ? "," : ""); }
		PRINT_FLAG(NOTIFY);
		PRINT_FLAG(EQUALIZE);
		PRINT_FLAG(PREFIX);
#undef PRINT_FLAG

#define PRINT_FLAG(f) if (flags & RTCF_##f) { \
		flags &= ~RTCF_##f; nl_dump(p, #f "%s", flags ? "," : ""); }
		PRINT_FLAG(NOTIFY);
		PRINT_FLAG(REDIRECTED);
		PRINT_FLAG(DOREDIRECT);
		PRINT_FLAG(DIRECTSRC);
		PRINT_FLAG(DNAT);
		PRINT_FLAG(BROADCAST);
		PRINT_FLAG(MULTICAST);
		PRINT_FLAG(LOCAL);
#undef PRINT_FLAG

		nl_dump(p, ">");
	}

	nl_dump(p, "\n");
}

static void route_dump_details(struct nl_object *a, struct nl_dump_params *p)
{
	struct rtnl_route *r = (struct rtnl_route *) a;
	struct nl_cache *link_cache;
	char buf[128];
	int i;

	link_cache = nl_cache_mngt_require("route/link");

	route_dump_line(a, p);
	nl_dump_line(p, "    ");

	if (r->ce_mask & ROUTE_ATTR_PREF_SRC)
		nl_dump(p, "preferred-src %s ",
			nl_addr2str(r->rt_pref_src, buf, sizeof(buf)));

	if (r->ce_mask & ROUTE_ATTR_SCOPE && r->rt_scope != RT_SCOPE_NOWHERE)
		nl_dump(p, "scope %s ",
			rtnl_scope2str(r->rt_scope, buf, sizeof(buf)));

	if (r->ce_mask & ROUTE_ATTR_PRIO)
		nl_dump(p, "priority %#x ", r->rt_prio);

	if (r->ce_mask & ROUTE_ATTR_PROTOCOL)
		nl_dump(p, "protocol %s ",
			rtnl_route_proto2str(r->rt_protocol, buf, sizeof(buf)));

	if (r->ce_mask & ROUTE_ATTR_IIF) {
		if (link_cache) {
			nl_dump(p, "iif %s ",
				rtnl_link_i2name(link_cache, r->rt_iif,
						 buf, sizeof(buf)));
		} else
			nl_dump(p, "iif %d ", r->rt_iif);
	}

	if (r->ce_mask & ROUTE_ATTR_SRC)
		nl_dump(p, "src %s ", nl_addr2str(r->rt_src, buf, sizeof(buf)));

	nl_dump(p, "\n");

	if (r->ce_mask & ROUTE_ATTR_MULTIPATH) {
		struct rtnl_nexthop *nh;

		nl_list_for_each_entry(nh, &r->rt_nexthops, rtnh_list) {
			nl_dump_line(p, "    ");
			p->dp_ivar = NH_DUMP_FROM_DETAILS;
			rtnl_route_nh_dump(nh, p);
			nl_dump(p, "\n");
		}
	}

	if ((r->ce_mask & ROUTE_ATTR_CACHEINFO) && r->rt_cacheinfo.rtci_error) {
		nl_dump_line(p, "    cacheinfo error %d (%s)\n",
			r->rt_cacheinfo.rtci_error,
			strerror(-r->rt_cacheinfo.rtci_error));
	}

	if (r->ce_mask & ROUTE_ATTR_METRICS) {
		nl_dump_line(p, "    metrics [");
		for (i = 0; i < RTAX_MAX; i++)
			if (r->rt_metrics_mask & (1 << i))
				nl_dump(p, "%s %u ",
					rtnl_route_metric2str(i+1,
							      buf, sizeof(buf)),
					r->rt_metrics[i]);
		nl_dump(p, "]\n");
	}
}

static void route_dump_stats(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_route *route = (struct rtnl_route *) obj;

	route_dump_details(obj, p);

	if (route->ce_mask & ROUTE_ATTR_CACHEINFO) {
		struct rtnl_rtcacheinfo *ci = &route->rt_cacheinfo;

		nl_dump_line(p, "    used %u refcnt %u last-use %us "
				"expires %us\n",
			     ci->rtci_used, ci->rtci_clntref,
			     ci->rtci_last_use / nl_get_user_hz(),
			     ci->rtci_expires / nl_get_user_hz());
	}
}

static int route_compare(struct nl_object *_a, struct nl_object *_b,
			uint32_t attrs, int flags)
{
	struct rtnl_route *a = (struct rtnl_route *) _a;
	struct rtnl_route *b = (struct rtnl_route *) _b;
	struct rtnl_nexthop *nh_a, *nh_b;
	int i, diff = 0, found;

#define ROUTE_DIFF(ATTR, EXPR) ATTR_DIFF(attrs, ROUTE_ATTR_##ATTR, a, b, EXPR)

	diff |= ROUTE_DIFF(FAMILY,	a->rt_family != b->rt_family);
	diff |= ROUTE_DIFF(TOS,		a->rt_tos != b->rt_tos);
	diff |= ROUTE_DIFF(TABLE,	a->rt_table != b->rt_table);
	diff |= ROUTE_DIFF(PROTOCOL,	a->rt_protocol != b->rt_protocol);
	diff |= ROUTE_DIFF(SCOPE,	a->rt_scope != b->rt_scope);
	diff |= ROUTE_DIFF(TYPE,	a->rt_type != b->rt_type);
	diff |= ROUTE_DIFF(PRIO,	a->rt_prio != b->rt_prio);
	diff |= ROUTE_DIFF(DST,		nl_addr_cmp(a->rt_dst, b->rt_dst));
	diff |= ROUTE_DIFF(SRC,		nl_addr_cmp(a->rt_src, b->rt_src));
	diff |= ROUTE_DIFF(IIF,		a->rt_iif != b->rt_iif);
	diff |= ROUTE_DIFF(PREF_SRC,	nl_addr_cmp(a->rt_pref_src,
						    b->rt_pref_src));

	if (flags & LOOSE_COMPARISON) {
		nl_list_for_each_entry(nh_b, &b->rt_nexthops, rtnh_list) {
			found = 0;
			nl_list_for_each_entry(nh_a, &a->rt_nexthops,
					       rtnh_list) {
				if (!rtnl_route_nh_compare(nh_a, nh_b,
							nh_b->ce_mask, 1)) {
					found = 1;
					break;
				}
			}

			if (!found)
				goto nh_mismatch;
		}

		for (i = 0; i < RTAX_MAX - 1; i++) {
			if (a->rt_metrics_mask & (1 << i) &&
			    (!(b->rt_metrics_mask & (1 << i)) ||
			     a->rt_metrics[i] != b->rt_metrics[i]))
				ROUTE_DIFF(METRICS, 1);
		}

		diff |= ROUTE_DIFF(FLAGS,
			  (a->rt_flags ^ b->rt_flags) & b->rt_flag_mask);
	} else {
		if (a->rt_nr_nh != a->rt_nr_nh)
			goto nh_mismatch;

		/* search for a dup in each nh of a */
		nl_list_for_each_entry(nh_a, &a->rt_nexthops, rtnh_list) {
			found = 0;
			nl_list_for_each_entry(nh_b, &b->rt_nexthops,
					       rtnh_list) {
				if (!rtnl_route_nh_compare(nh_a, nh_b, ~0, 0)) {
					found = 1;
					break;
				}
			}
			if (!found)
				goto nh_mismatch;
		}

		/* search for a dup in each nh of b, covers case where a has
		 * dupes itself */
		nl_list_for_each_entry(nh_b, &b->rt_nexthops, rtnh_list) {
			found = 0;
			nl_list_for_each_entry(nh_a, &a->rt_nexthops,
					       rtnh_list) {
				if (!rtnl_route_nh_compare(nh_a, nh_b, ~0, 0)) {
					found = 1;
					break;
				}
			}
			if (!found)
				goto nh_mismatch;
		}

		for (i = 0; i < RTAX_MAX - 1; i++) {
			if ((a->rt_metrics_mask & (1 << i)) ^
			    (b->rt_metrics_mask & (1 << i)))
				diff |= ROUTE_DIFF(METRICS, 1);
			else
				diff |= ROUTE_DIFF(METRICS,
					a->rt_metrics[i] != b->rt_metrics[i]);
		}

		diff |= ROUTE_DIFF(FLAGS, a->rt_flags != b->rt_flags);
	}

out:
	return diff;

nh_mismatch:
	diff |= ROUTE_DIFF(MULTIPATH, 1);
	goto out;

#undef ROUTE_DIFF
}

static const struct trans_tbl route_attrs[] = {
	__ADD(ROUTE_ATTR_FAMILY, family)
	__ADD(ROUTE_ATTR_TOS, tos)
	__ADD(ROUTE_ATTR_TABLE, table)
	__ADD(ROUTE_ATTR_PROTOCOL, protocol)
	__ADD(ROUTE_ATTR_SCOPE, scope)
	__ADD(ROUTE_ATTR_TYPE, type)
	__ADD(ROUTE_ATTR_FLAGS, flags)
	__ADD(ROUTE_ATTR_DST, dst)
	__ADD(ROUTE_ATTR_SRC, src)
	__ADD(ROUTE_ATTR_IIF, iif)
	__ADD(ROUTE_ATTR_OIF, oif)
	__ADD(ROUTE_ATTR_GATEWAY, gateway)
	__ADD(ROUTE_ATTR_PRIO, prio)
	__ADD(ROUTE_ATTR_PREF_SRC, pref_src)
	__ADD(ROUTE_ATTR_METRICS, metrics)
	__ADD(ROUTE_ATTR_MULTIPATH, multipath)
	__ADD(ROUTE_ATTR_REALMS, realms)
	__ADD(ROUTE_ATTR_CACHEINFO, cacheinfo)
};

static char *route_attrs2str(int attrs, char *buf, size_t len)
{
	return __flags2str(attrs, buf, len, route_attrs,
			   ARRAY_SIZE(route_attrs));
}

/**
 * @name Allocation/Freeing
 * @{
 */

struct rtnl_route *rtnl_route_alloc(void)
{
	return (struct rtnl_route *) nl_object_alloc(&route_obj_ops);
}

void rtnl_route_get(struct rtnl_route *route)
{
	nl_object_get((struct nl_object *) route);
}

void rtnl_route_put(struct rtnl_route *route)
{
	nl_object_put((struct nl_object *) route);
}

/** @} */

/**
 * @name Attributes
 * @{
 */

void rtnl_route_set_table(struct rtnl_route *route, uint32_t table)
{
	route->rt_table = table;
	route->ce_mask |= ROUTE_ATTR_TABLE;
}

uint32_t rtnl_route_get_table(struct rtnl_route *route)
{
	return route->rt_table;
}

void rtnl_route_set_scope(struct rtnl_route *route, uint8_t scope)
{
	route->rt_scope = scope;
	route->ce_mask |= ROUTE_ATTR_SCOPE;
}

uint8_t rtnl_route_get_scope(struct rtnl_route *route)
{
	return route->rt_scope;
}

void rtnl_route_set_tos(struct rtnl_route *route, uint8_t tos)
{
	route->rt_tos = tos;
	route->ce_mask |= ROUTE_ATTR_TOS;
}

uint8_t rtnl_route_get_tos(struct rtnl_route *route)
{
	return route->rt_tos;
}

void rtnl_route_set_protocol(struct rtnl_route *route, uint8_t protocol)
{
	route->rt_protocol = protocol;
	route->ce_mask |= ROUTE_ATTR_PROTOCOL;
}

uint8_t rtnl_route_get_protocol(struct rtnl_route *route)
{
	return route->rt_protocol;
}

void rtnl_route_set_priority(struct rtnl_route *route, uint32_t prio)
{
	route->rt_prio = prio;
	route->ce_mask |= ROUTE_ATTR_PRIO;
}

uint32_t rtnl_route_get_priority(struct rtnl_route *route)
{
	return route->rt_prio;
}

int rtnl_route_set_family(struct rtnl_route *route, uint8_t family)
{
	if (family != AF_INET && family != AF_INET6 && family != AF_DECnet)
		return -NLE_AF_NOSUPPORT;

	route->rt_family = family;
	route->ce_mask |= ROUTE_ATTR_FAMILY;

	return 0;
}

uint8_t rtnl_route_get_family(struct rtnl_route *route)
{
	return route->rt_family;
}

int rtnl_route_set_dst(struct rtnl_route *route, struct nl_addr *addr)
{
	if (route->ce_mask & ROUTE_ATTR_FAMILY) {
		if (addr->a_family != route->rt_family)
			return -NLE_AF_MISMATCH;
	} else
		route->rt_family = addr->a_family;

	if (route->rt_dst)
		nl_addr_put(route->rt_dst);

	nl_addr_get(addr);
	route->rt_dst = addr;
	
	route->ce_mask |= (ROUTE_ATTR_DST | ROUTE_ATTR_FAMILY);

	return 0;
}

struct nl_addr *rtnl_route_get_dst(struct rtnl_route *route)
{
	return route->rt_dst;
}

int rtnl_route_set_src(struct rtnl_route *route, struct nl_addr *addr)
{
	if (addr->a_family == AF_INET)
		return -NLE_SRCRT_NOSUPPORT;

	if (route->ce_mask & ROUTE_ATTR_FAMILY) {
		if (addr->a_family != route->rt_family)
			return -NLE_AF_MISMATCH;
	} else
		route->rt_family = addr->a_family;

	if (route->rt_src)
		nl_addr_put(route->rt_src);

	nl_addr_get(addr);
	route->rt_src = addr;
	route->ce_mask |= (ROUTE_ATTR_SRC | ROUTE_ATTR_FAMILY);

	return 0;
}

struct nl_addr *rtnl_route_get_src(struct rtnl_route *route)
{
	return route->rt_src;
}

int rtnl_route_set_type(struct rtnl_route *route, uint8_t type)
{
	if (type > RTN_MAX)
		return -NLE_RANGE;

	route->rt_type = type;
	route->ce_mask |= ROUTE_ATTR_TYPE;

	return 0;
}

uint8_t rtnl_route_get_type(struct rtnl_route *route)
{
	return route->rt_type;
}

void rtnl_route_set_flags(struct rtnl_route *route, uint32_t flags)
{
	route->rt_flag_mask |= flags;
	route->rt_flags |= flags;
	route->ce_mask |= ROUTE_ATTR_FLAGS;
}

void rtnl_route_unset_flags(struct rtnl_route *route, uint32_t flags)
{
	route->rt_flag_mask |= flags;
	route->rt_flags &= ~flags;
	route->ce_mask |= ROUTE_ATTR_FLAGS;
}

uint32_t rtnl_route_get_flags(struct rtnl_route *route)
{
	return route->rt_flags;
}

int rtnl_route_set_metric(struct rtnl_route *route, int metric, uint32_t value)
{
	if (metric > RTAX_MAX || metric < 1)
		return -NLE_RANGE;

	route->rt_metrics[metric - 1] = value;

	if (!(route->rt_metrics_mask & (1 << (metric - 1)))) {
		route->rt_nmetrics++;
		route->rt_metrics_mask |= (1 << (metric - 1));
	}

	route->ce_mask |= ROUTE_ATTR_METRICS;

	return 0;
}

int rtnl_route_unset_metric(struct rtnl_route *route, int metric)
{
	if (metric > RTAX_MAX || metric < 1)
		return -NLE_RANGE;

	if (route->rt_metrics_mask & (1 << (metric - 1))) {
		route->rt_nmetrics--;
		route->rt_metrics_mask &= ~(1 << (metric - 1));
	}

	return 0;
}

int rtnl_route_get_metric(struct rtnl_route *route, int metric, uint32_t *value)
{
	if (metric > RTAX_MAX || metric < 1)
		return -NLE_RANGE;

	if (!(route->rt_metrics_mask & (1 << (metric - 1))))
		return -NLE_OBJ_NOTFOUND;

	if (value)
		*value = route->rt_metrics[metric - 1];

	return 0;
}

int rtnl_route_set_pref_src(struct rtnl_route *route, struct nl_addr *addr)
{
	if (route->ce_mask & ROUTE_ATTR_FAMILY) {
		if (addr->a_family != route->rt_family)
			return -NLE_AF_MISMATCH;
	} else
		route->rt_family = addr->a_family;

	if (route->rt_pref_src)
		nl_addr_put(route->rt_pref_src);

	nl_addr_get(addr);
	route->rt_pref_src = addr;
	route->ce_mask |= (ROUTE_ATTR_PREF_SRC | ROUTE_ATTR_FAMILY);

	return 0;
}

struct nl_addr *rtnl_route_get_pref_src(struct rtnl_route *route)
{
	return route->rt_pref_src;
}

void rtnl_route_set_iif(struct rtnl_route *route, int ifindex)
{
	route->rt_iif = ifindex;
	route->ce_mask |= ROUTE_ATTR_IIF;
}

int rtnl_route_get_iif(struct rtnl_route *route)
{
	return route->rt_iif;
}

void rtnl_route_add_nexthop(struct rtnl_route *route, struct rtnl_nexthop *nh)
{
	nl_list_add_tail(&nh->rtnh_list, &route->rt_nexthops);
	route->rt_nr_nh++;
	route->ce_mask |= ROUTE_ATTR_MULTIPATH;
}

void rtnl_route_remove_nexthop(struct rtnl_route *route, struct rtnl_nexthop *nh)
{
	if (route->ce_mask & ROUTE_ATTR_MULTIPATH) {
		route->rt_nr_nh--;
		nl_list_del(&nh->rtnh_list);
	}
}

struct nl_list_head *rtnl_route_get_nexthops(struct rtnl_route *route)
{
	if (route->ce_mask & ROUTE_ATTR_MULTIPATH)
		return &route->rt_nexthops;

	return NULL;
}

int rtnl_route_get_nnexthops(struct rtnl_route *route)
{
	if (route->ce_mask & ROUTE_ATTR_MULTIPATH)
		return route->rt_nr_nh;

	return 0;
}

void rtnl_route_foreach_nexthop(struct rtnl_route *r,
                                void (*cb)(struct rtnl_nexthop *, void *),
                                void *arg)
{
	struct rtnl_nexthop *nh;
    
	if (r->ce_mask & ROUTE_ATTR_MULTIPATH) {
		nl_list_for_each_entry(nh, &r->rt_nexthops, rtnh_list) {
                        cb(nh, arg);
		}
	}
}

struct rtnl_nexthop *rtnl_route_nexthop_n(struct rtnl_route *r, int n)
{
	struct rtnl_nexthop *nh;
	int i;
    
	if (r->ce_mask & ROUTE_ATTR_MULTIPATH && r->rt_nr_nh > n) {
		i = 0;
		nl_list_for_each_entry(nh, &r->rt_nexthops, rtnh_list) {
                        if (i == n) return nh;
			i++;
		}
	}
        return NULL;
}

/** @} */

/**
 * @name Utilities
 * @{
 */

/**
 * Guess scope of a route object.
 * @arg route		Route object.
 *
 * Guesses the scope of a route object, based on the following rules:
 * @code
 *   1) Local route -> local scope
 *   2) At least one nexthop not directly connected -> universe scope
 *   3) All others -> link scope
 * @endcode
 *
 * @return Scope value.
 */
int rtnl_route_guess_scope(struct rtnl_route *route)
{
	if (route->rt_type == RTN_LOCAL)
		return RT_SCOPE_HOST;

	if (!nl_list_empty(&route->rt_nexthops)) {
		struct rtnl_nexthop *nh;

		/*
		 * Use scope uiniverse if there is at least one nexthop which
		 * is not directly connected
		 */
		nl_list_for_each_entry(nh, &route->rt_nexthops, rtnh_list) {
			if (nh->rtnh_gateway)
				return RT_SCOPE_UNIVERSE;
		}
	}

	return RT_SCOPE_LINK;
}

/** @} */

static struct nla_policy route_policy[RTA_MAX+1] = {
	[RTA_IIF]	= { .type = NLA_U32 },
	[RTA_OIF]	= { .type = NLA_U32 },
	[RTA_PRIORITY]	= { .type = NLA_U32 },
	[RTA_FLOW]	= { .type = NLA_U32 },
	[RTA_CACHEINFO]	= { .minlen = sizeof(struct rta_cacheinfo) },
	[RTA_METRICS]	= { .type = NLA_NESTED },
	[RTA_MULTIPATH]	= { .type = NLA_NESTED },
};

static int parse_multipath(struct rtnl_route *route, struct nlattr *attr)
{
	struct rtnl_nexthop *nh = NULL;
	struct rtnexthop *rtnh = nla_data(attr);
	size_t tlen = nla_len(attr);
	int err;

	while (tlen >= sizeof(*rtnh) && tlen >= rtnh->rtnh_len) {
		nh = rtnl_route_nh_alloc();
		if (!nh)
			return -NLE_NOMEM;

		rtnl_route_nh_set_weight(nh, rtnh->rtnh_hops);
		rtnl_route_nh_set_ifindex(nh, rtnh->rtnh_ifindex);
		rtnl_route_nh_set_flags(nh, rtnh->rtnh_flags);

		if (rtnh->rtnh_len > sizeof(*rtnh)) {
			struct nlattr *ntb[RTA_MAX + 1];

			err = nla_parse(ntb, RTA_MAX, (struct nlattr *)
					RTNH_DATA(rtnh),
					rtnh->rtnh_len - sizeof(*rtnh),
					route_policy);
			if (err < 0)
				goto errout;

			if (ntb[RTA_GATEWAY]) {
				struct nl_addr *addr;

				addr = nl_addr_alloc_attr(ntb[RTA_GATEWAY],
							  route->rt_family);
				if (!addr) {
					err = -NLE_NOMEM;
					goto errout;
				}

				rtnl_route_nh_set_gateway(nh, addr);
				nl_addr_put(addr);
			}

			if (ntb[RTA_FLOW]) {
				uint32_t realms;
				
				realms = nla_get_u32(ntb[RTA_FLOW]);
				rtnl_route_nh_set_realms(nh, realms);
			}
		}

		rtnl_route_add_nexthop(route, nh);
		tlen -= RTNH_ALIGN(rtnh->rtnh_len);
		rtnh = RTNH_NEXT(rtnh);
	}

	err = 0;
errout:
	if (err && nh)
		rtnl_route_nh_free(nh);

	return err;
}

int rtnl_route_parse(struct nlmsghdr *nlh, struct rtnl_route **result)
{
	struct rtmsg *rtm;
	struct rtnl_route *route;
	struct nlattr *tb[RTA_MAX + 1];
	struct nl_addr *src = NULL, *dst = NULL, *addr;
	struct rtnl_nexthop *old_nh = NULL;
	int err, family;

	route = rtnl_route_alloc();
	if (!route) {
		err = -NLE_NOMEM;
		goto errout;
	}

	route->ce_msgtype = nlh->nlmsg_type;

	err = nlmsg_parse(nlh, sizeof(struct rtmsg), tb, RTA_MAX, route_policy);
	if (err < 0)
		goto errout;

	rtm = nlmsg_data(nlh);
	route->rt_family = family = rtm->rtm_family;
	route->rt_tos = rtm->rtm_tos;
	route->rt_table = rtm->rtm_table;
	route->rt_type = rtm->rtm_type;
	route->rt_scope = rtm->rtm_scope;
	route->rt_protocol = rtm->rtm_protocol;
	route->rt_flags = rtm->rtm_flags;

	route->ce_mask |= ROUTE_ATTR_FAMILY | ROUTE_ATTR_TOS |
			  ROUTE_ATTR_TABLE | ROUTE_ATTR_TYPE |
			  ROUTE_ATTR_SCOPE | ROUTE_ATTR_PROTOCOL |
			  ROUTE_ATTR_FLAGS;

	if (tb[RTA_DST]) {
		if (!(dst = nl_addr_alloc_attr(tb[RTA_DST], family)))
			goto errout_nomem;
	} else {
		if (!(dst = nl_addr_alloc(0)))
			goto errout_nomem;
		nl_addr_set_family(dst, rtm->rtm_family);
	}

	nl_addr_set_prefixlen(dst, rtm->rtm_dst_len);
	err = rtnl_route_set_dst(route, dst);
	if (err < 0)
		goto errout;

	nl_addr_put(dst);

	if (tb[RTA_SRC]) {
		if (!(src = nl_addr_alloc_attr(tb[RTA_SRC], family)))
			goto errout_nomem;
	} else if (rtm->rtm_src_len)
		if (!(src = nl_addr_alloc(0)))
			goto errout_nomem;

	if (src) {
		nl_addr_set_prefixlen(src, rtm->rtm_src_len);
		rtnl_route_set_src(route, src);
		nl_addr_put(src);
	}

	if (tb[RTA_TABLE])
		rtnl_route_set_table(route, nla_get_u32(tb[RTA_TABLE]));

	if (tb[RTA_IIF])
		rtnl_route_set_iif(route, nla_get_u32(tb[RTA_IIF]));

	if (tb[RTA_PRIORITY])
		rtnl_route_set_priority(route, nla_get_u32(tb[RTA_PRIORITY]));

	if (tb[RTA_PREFSRC]) {
		if (!(addr = nl_addr_alloc_attr(tb[RTA_PREFSRC], family)))
			goto errout_nomem;
		rtnl_route_set_pref_src(route, addr);
		nl_addr_put(addr);
	}

	if (tb[RTA_METRICS]) {
		struct nlattr *mtb[RTAX_MAX + 1];
		int i;

		err = nla_parse_nested(mtb, RTAX_MAX, tb[RTA_METRICS], NULL);
		if (err < 0)
			goto errout;

		for (i = 1; i <= RTAX_MAX; i++) {
			if (mtb[i] && nla_len(mtb[i]) >= sizeof(uint32_t)) {
				uint32_t m = nla_get_u32(mtb[i]);
				if (rtnl_route_set_metric(route, i, m) < 0)
					goto errout;
			}
		}
	}

	if (tb[RTA_MULTIPATH])
		if ((err = parse_multipath(route, tb[RTA_MULTIPATH])) < 0)
			goto errout;

	if (tb[RTA_CACHEINFO]) {
		nla_memcpy(&route->rt_cacheinfo, tb[RTA_CACHEINFO],
			   sizeof(route->rt_cacheinfo));
		route->ce_mask |= ROUTE_ATTR_CACHEINFO;
	}

	if (tb[RTA_OIF]) {
		if (!old_nh && !(old_nh = rtnl_route_nh_alloc()))
			goto errout;

		rtnl_route_nh_set_ifindex(old_nh, nla_get_u32(tb[RTA_OIF]));
	}

	if (tb[RTA_GATEWAY]) {
		if (!old_nh && !(old_nh = rtnl_route_nh_alloc()))
			goto errout;

		if (!(addr = nl_addr_alloc_attr(tb[RTA_GATEWAY], family)))
			goto errout_nomem;

		rtnl_route_nh_set_gateway(old_nh, addr);
		nl_addr_put(addr);
	}

	if (tb[RTA_FLOW]) {
		if (!old_nh && !(old_nh = rtnl_route_nh_alloc()))
			goto errout;

		rtnl_route_nh_set_realms(old_nh, nla_get_u32(tb[RTA_FLOW]));
	}

	if (old_nh) {
		if (route->rt_nr_nh == 0) {
			/* If no nexthops have been provided via RTA_MULTIPATH
			 * we add it as regular nexthop to maintain backwards
			 * compatibility */
			rtnl_route_add_nexthop(route, old_nh);
		} else {
			/* Kernel supports new style nexthop configuration,
			 * verify that it is a duplicate and discard nexthop. */
			struct rtnl_nexthop *first;

			first = nl_list_first_entry(&route->rt_nexthops,
						    struct rtnl_nexthop,
						    rtnh_list);
			if (!first)
				BUG();

			if (rtnl_route_nh_compare(old_nh, first,
						  old_nh->ce_mask, 0)) {
				err = -NLE_INVAL;
				goto errout;
			}

			rtnl_route_nh_free(old_nh);
		}
	}

	*result = route;
	return 0;

errout:
	rtnl_route_put(route);
	return err;

errout_nomem:
	err = -NLE_NOMEM;
	goto errout;
}

int rtnl_route_build_msg(struct nl_msg *msg, struct rtnl_route *route)
{
	int i;
	struct nlattr *metrics;
	struct rtmsg rtmsg = {
		.rtm_family = route->rt_family,
		.rtm_tos = route->rt_tos,
		.rtm_table = route->rt_table,
		.rtm_protocol = route->rt_protocol,
		.rtm_scope = route->rt_scope,
		.rtm_type = route->rt_type,
		.rtm_flags = route->rt_flags,
	};

	if (route->rt_dst == NULL)
		return -NLE_MISSING_ATTR;

	rtmsg.rtm_dst_len = nl_addr_get_prefixlen(route->rt_dst);
	if (route->rt_src)
		rtmsg.rtm_src_len = nl_addr_get_prefixlen(route->rt_src);


	if (rtmsg.rtm_scope == RT_SCOPE_NOWHERE)
		rtmsg.rtm_scope = rtnl_route_guess_scope(route);

	if (nlmsg_append(msg, &rtmsg, sizeof(rtmsg), NLMSG_ALIGNTO) < 0)
		goto nla_put_failure;

	/* Additional table attribute replacing the 8bit in the header, was
	 * required to allow more than 256 tables. */
	NLA_PUT_U32(msg, RTA_TABLE, route->rt_table);

	if (nl_addr_get_len(route->rt_dst))
		NLA_PUT_ADDR(msg, RTA_DST, route->rt_dst);
	NLA_PUT_U32(msg, RTA_PRIORITY, route->rt_prio);

	if (route->ce_mask & ROUTE_ATTR_SRC)
		NLA_PUT_ADDR(msg, RTA_SRC, route->rt_src);

	if (route->ce_mask & ROUTE_ATTR_PREF_SRC)
		NLA_PUT_ADDR(msg, RTA_PREFSRC, route->rt_pref_src);

	if (route->ce_mask & ROUTE_ATTR_IIF)
		NLA_PUT_U32(msg, RTA_IIF, route->rt_iif);

	if (route->rt_nmetrics > 0) {
		uint32_t val;

		metrics = nla_nest_start(msg, RTA_METRICS);
		if (metrics == NULL)
			goto nla_put_failure;

		for (i = 1; i <= RTAX_MAX; i++) {
			if (!rtnl_route_get_metric(route, i, &val))
				NLA_PUT_U32(msg, i, val);
		}

		nla_nest_end(msg, metrics);
	}

	if (rtnl_route_get_nnexthops(route) == 1) {
		struct rtnl_nexthop *nh;

		nh = rtnl_route_nexthop_n(route, 0);
		if (nh->rtnh_gateway)
			NLA_PUT_ADDR(msg, RTA_GATEWAY, nh->rtnh_gateway);
		if (nh->rtnh_ifindex)
			NLA_PUT_U32(msg, RTA_OIF, nh->rtnh_ifindex);
		if (nh->rtnh_realms)
			NLA_PUT_U32(msg, RTA_FLOW, nh->rtnh_realms);
	} else if (rtnl_route_get_nnexthops(route) > 1) {
		struct nlattr *multipath;
		struct rtnl_nexthop *nh;

		if (!(multipath = nla_nest_start(msg, RTA_MULTIPATH)))
			goto nla_put_failure;

		nl_list_for_each_entry(nh, &route->rt_nexthops, rtnh_list) {
			struct rtnexthop *rtnh;

			rtnh = nlmsg_reserve(msg, sizeof(*rtnh), NLMSG_ALIGNTO);
			if (!rtnh)
				goto nla_put_failure;

			rtnh->rtnh_flags = nh->rtnh_flags;
			rtnh->rtnh_hops = nh->rtnh_weight;
			rtnh->rtnh_ifindex = nh->rtnh_ifindex;

			if (nh->rtnh_gateway)
				NLA_PUT_ADDR(msg, RTA_GATEWAY,
					     nh->rtnh_gateway);

			if (nh->rtnh_realms)
				NLA_PUT_U32(msg, RTA_FLOW, nh->rtnh_realms);

			rtnh->rtnh_len = nlmsg_tail(msg->nm_nlh) -
						(void *) rtnh;
		}

		nla_nest_end(msg, multipath);
	}

	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

/** @cond SKIP */
struct nl_object_ops route_obj_ops = {
	.oo_name		= "route/route",
	.oo_size		= sizeof(struct rtnl_route),
	.oo_constructor		= route_constructor,
	.oo_free_data		= route_free_data,
	.oo_clone		= route_clone,
	.oo_dump = {
	    [NL_DUMP_LINE]	= route_dump_line,
	    [NL_DUMP_DETAILS]	= route_dump_details,
	    [NL_DUMP_STATS]	= route_dump_stats,
	},
	.oo_compare		= route_compare,
	.oo_attrs2str		= route_attrs2str,
	.oo_id_attrs		= (ROUTE_ATTR_FAMILY | ROUTE_ATTR_TOS |
				   ROUTE_ATTR_TABLE | ROUTE_ATTR_DST),
};
/** @endcond */

/** @} */

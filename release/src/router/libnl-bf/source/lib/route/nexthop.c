/*
 * lib/route/nexthop.c	Routing Nexthop
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup route_obj
 * @defgroup nexthop Nexthop
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/route.h>

/** @cond SKIP */
#define NH_ATTR_FLAGS   0x000001
#define NH_ATTR_WEIGHT  0x000002
#define NH_ATTR_IFINDEX 0x000004
#define NH_ATTR_GATEWAY 0x000008
#define NH_ATTR_REALMS  0x000010
/** @endcond */

/**
 * @name Allocation/Freeing
 * @{
 */

struct rtnl_nexthop *rtnl_route_nh_alloc(void)
{
	struct rtnl_nexthop *nh;

	nh = calloc(1, sizeof(*nh));
	if (!nh)
		return NULL;

	nl_init_list_head(&nh->rtnh_list);

	return nh;
}

struct rtnl_nexthop *rtnl_route_nh_clone(struct rtnl_nexthop *src)
{
	struct rtnl_nexthop *nh;

	nh = rtnl_route_nh_alloc();
	if (!nh)
		return NULL;

	nh->rtnh_flags = src->rtnh_flags;
	nh->rtnh_flag_mask = src->rtnh_flag_mask;
	nh->rtnh_weight = src->rtnh_weight;
	nh->rtnh_ifindex = src->rtnh_ifindex;
	nh->ce_mask = src->ce_mask;

	if (src->rtnh_gateway) {
		nh->rtnh_gateway = nl_addr_clone(src->rtnh_gateway);
		if (!nh->rtnh_gateway) {
			free(nh);
			return NULL;
		}
	}

	return nh;
}

void rtnl_route_nh_free(struct rtnl_nexthop *nh)
{
	nl_addr_put(nh->rtnh_gateway);
	free(nh);
}

/** @} */

int rtnl_route_nh_compare(struct rtnl_nexthop *a, struct rtnl_nexthop *b,
			  uint32_t attrs, int loose)
{
	int diff = 0;

#define NH_DIFF(ATTR, EXPR) ATTR_DIFF(attrs, NH_ATTR_##ATTR, a, b, EXPR)

	diff |= NH_DIFF(IFINDEX,	a->rtnh_ifindex != b->rtnh_ifindex);
	diff |= NH_DIFF(WEIGHT,		a->rtnh_weight != b->rtnh_weight);
	diff |= NH_DIFF(REALMS,		a->rtnh_realms != b->rtnh_realms);
	diff |= NH_DIFF(GATEWAY,	nl_addr_cmp(a->rtnh_gateway,
						    b->rtnh_gateway));

	if (loose)
		diff |= NH_DIFF(FLAGS,
			  (a->rtnh_flags ^ b->rtnh_flags) & b->rtnh_flag_mask);
	else
		diff |= NH_DIFF(FLAGS, a->rtnh_flags != b->rtnh_flags);
	
#undef NH_DIFF

	return diff;
}

static void nh_dump_line(struct rtnl_nexthop *nh, struct nl_dump_params *dp)
{
	struct nl_cache *link_cache;
	char buf[128];

	link_cache = nl_cache_mngt_require("route/link");

	nl_dump(dp, "via");

	if (nh->ce_mask & NH_ATTR_GATEWAY)
		nl_dump(dp, " %s", nl_addr2str(nh->rtnh_gateway,
						   buf, sizeof(buf)));

	if(nh->ce_mask & NH_ATTR_IFINDEX) {
		if (link_cache) {
			nl_dump(dp, " dev %s",
				rtnl_link_i2name(link_cache,
						 nh->rtnh_ifindex,
						 buf, sizeof(buf)));
		} else
			nl_dump(dp, " dev %d", nh->rtnh_ifindex);
	}

	nl_dump(dp, " ");
}

static void nh_dump_details(struct rtnl_nexthop *nh, struct nl_dump_params *dp)
{
	struct nl_cache *link_cache;
	char buf[128];

	link_cache = nl_cache_mngt_require("route/link");

	nl_dump(dp, "nexthop");

	if (nh->ce_mask & NH_ATTR_GATEWAY)
		nl_dump(dp, " via %s", nl_addr2str(nh->rtnh_gateway,
						   buf, sizeof(buf)));

	if(nh->ce_mask & NH_ATTR_IFINDEX) {
		if (link_cache) {
			nl_dump(dp, " dev %s",
				rtnl_link_i2name(link_cache,
						 nh->rtnh_ifindex,
						 buf, sizeof(buf)));
		} else
			nl_dump(dp, " dev %d", nh->rtnh_ifindex);
	}

	if (nh->ce_mask & NH_ATTR_WEIGHT)
		nl_dump(dp, " weight %u", nh->rtnh_weight);

	if (nh->ce_mask & NH_ATTR_REALMS)
		nl_dump(dp, " realm %04x:%04x",
			RTNL_REALM_FROM(nh->rtnh_realms),
			RTNL_REALM_TO(nh->rtnh_realms));

	if (nh->ce_mask & NH_ATTR_FLAGS)
		nl_dump(dp, " <%s>", rtnl_route_nh_flags2str(nh->rtnh_flags,
							buf, sizeof(buf)));
}

void rtnl_route_nh_dump(struct rtnl_nexthop *nh, struct nl_dump_params *dp)
{
	switch (dp->dp_type) {
	case NL_DUMP_LINE:
		nh_dump_line(nh, dp);
		break;

	case NL_DUMP_DETAILS:
	case NL_DUMP_STATS:
		if (dp->dp_ivar == NH_DUMP_FROM_DETAILS)
			nh_dump_details(nh, dp);
		break;

	default:
		break;
	}
}

/**
 * @name Attributes
 * @{
 */

void rtnl_route_nh_set_weight(struct rtnl_nexthop *nh, uint8_t weight)
{
	nh->rtnh_weight = weight;
	nh->ce_mask |= NH_ATTR_WEIGHT;
}

uint8_t rtnl_route_nh_get_weight(struct rtnl_nexthop *nh)
{
	return nh->rtnh_weight;
}

void rtnl_route_nh_set_ifindex(struct rtnl_nexthop *nh, int ifindex)
{
	nh->rtnh_ifindex = ifindex;
	nh->ce_mask |= NH_ATTR_IFINDEX;
}

int rtnl_route_nh_get_ifindex(struct rtnl_nexthop *nh)
{
	return nh->rtnh_ifindex;
}	

void rtnl_route_nh_set_gateway(struct rtnl_nexthop *nh, struct nl_addr *addr)
{
	struct nl_addr *old = nh->rtnh_gateway;

	if (addr) {
		nh->rtnh_gateway = nl_addr_get(addr);
		nh->ce_mask |= NH_ATTR_GATEWAY;
	} else {
		nh->ce_mask &= ~NH_ATTR_GATEWAY;
		nh->rtnh_gateway = NULL;
	}

	if (old)
		nl_addr_put(old);
}

struct nl_addr *rtnl_route_nh_get_gateway(struct rtnl_nexthop *nh)
{
	return nh->rtnh_gateway;
}

void rtnl_route_nh_set_flags(struct rtnl_nexthop *nh, unsigned int flags)
{
	nh->rtnh_flag_mask |= flags;
	nh->rtnh_flags |= flags;
	nh->ce_mask |= NH_ATTR_FLAGS;
}

void rtnl_route_nh_unset_flags(struct rtnl_nexthop *nh, unsigned int flags)
{
	nh->rtnh_flag_mask |= flags;
	nh->rtnh_flags &= ~flags;
	nh->ce_mask |= NH_ATTR_FLAGS;
}

unsigned int rtnl_route_nh_get_flags(struct rtnl_nexthop *nh)
{
	return nh->rtnh_flags;
}

void rtnl_route_nh_set_realms(struct rtnl_nexthop *nh, uint32_t realms)
{
	nh->rtnh_realms = realms;
	nh->ce_mask |= NH_ATTR_REALMS;
}

uint32_t rtnl_route_nh_get_realms(struct rtnl_nexthop *nh)
{
	return nh->rtnh_realms;
}

/** @} */

/**
 * @name Nexthop Flags Translations
 * @{
 */

static const struct trans_tbl nh_flags[] = {
	__ADD(RTNH_F_DEAD, dead)
	__ADD(RTNH_F_PERVASIVE, pervasive)
	__ADD(RTNH_F_ONLINK, onlink)
};

char *rtnl_route_nh_flags2str(int flags, char *buf, size_t len)
{
	return __flags2str(flags, buf, len, nh_flags, ARRAY_SIZE(nh_flags));
}

int rtnl_route_nh_str2flags(const char *name)
{
	return __str2flags(name, nh_flags, ARRAY_SIZE(nh_flags));
}

/** @} */

/** @} */

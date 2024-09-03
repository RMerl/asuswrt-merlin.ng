/*
 * lib/route/link/vxlan.c	VXLAN Link Info
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Yasunobu Chiba <yasu@dsl.gr.jp>
 */

/**
 * @ingroup link
 * @defgroup vxlan VXLAN
 * Virtual eXtensible Local Area Network link module
 *
 * @details
 * \b Link Type Name: "vxlan"
 *
 * @route_doc{link_vxlan, VXLAN Documentation}
 *
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink/netlink.h>
#include <netlink/attr.h>
#include <netlink/utils.h>
#include <netlink/object.h>
#include <netlink/route/rtnl.h>
#include <netlink-private/route/link/api.h>
#include <netlink/route/link/vxlan.h>

#include <linux/if_link.h>

/** @cond SKIP */
#define VXLAN_HAS_ID	(1<<0)
#define VXLAN_HAS_GROUP	(1<<1)
#define VXLAN_HAS_LINK	(1<<2)
#define VXLAN_HAS_LOCAL	(1<<3)
#define VXLAN_HAS_TTL	(1<<4)
#define VXLAN_HAS_TOS	(1<<5)
#define VXLAN_HAS_LEARNING	(1<<6)
#define VXLAN_HAS_AGEING	(1<<7)
#define VXLAN_HAS_LIMIT		(1<<8)
#define VXLAN_HAS_PORT_RANGE	(1<<9)
#define VXLAN_HAS_PROXY	(1<<10)
#define VXLAN_HAS_RSC	(1<<11)
#define VXLAN_HAS_L2MISS	(1<<12)
#define VXLAN_HAS_L3MISS	(1<<13)

struct vxlan_info
{
	uint32_t		vxi_id;
	uint32_t		vxi_group;
	uint32_t		vxi_link;
	uint32_t		vxi_local;
	uint8_t			vxi_ttl;
	uint8_t			vxi_tos;
	uint8_t			vxi_learning;
	uint32_t		vxi_ageing;
	uint32_t		vxi_limit;
	struct ifla_vxlan_port_range	vxi_port_range;
	uint8_t			vxi_proxy;
	uint8_t			vxi_rsc;
	uint8_t			vxi_l2miss;
	uint8_t			vxi_l3miss;
	uint32_t		vxi_mask;
};

/** @endcond */

static struct nla_policy vxlan_policy[IFLA_VXLAN_MAX+1] = {
	[IFLA_VXLAN_ID]	= { .type = NLA_U32 },
	[IFLA_VXLAN_GROUP] = { .minlen = sizeof(uint32_t) },
	[IFLA_VXLAN_LINK] = { .type = NLA_U32 },
	[IFLA_VXLAN_LOCAL] = { .minlen = sizeof(uint32_t) },
	[IFLA_VXLAN_TTL] = { .type = NLA_U8 },
	[IFLA_VXLAN_TOS] = { .type = NLA_U8 },
	[IFLA_VXLAN_LEARNING] = { .type = NLA_U8 },
	[IFLA_VXLAN_AGEING] = { .type = NLA_U32 },
	[IFLA_VXLAN_LIMIT] = { .type = NLA_U32 },
	[IFLA_VXLAN_PORT_RANGE] = { .minlen = sizeof(struct ifla_vxlan_port_range) },
	[IFLA_VXLAN_PROXY] = { .type = NLA_U8 },
	[IFLA_VXLAN_RSC] = { .type = NLA_U8 },
	[IFLA_VXLAN_L2MISS] = { .type = NLA_U8 },
	[IFLA_VXLAN_L3MISS] = { .type = NLA_U8 },
};

static int vxlan_alloc(struct rtnl_link *link)
{
	struct vxlan_info *vxi;

	if ((vxi = calloc(1, sizeof(*vxi))) == NULL)
		return -NLE_NOMEM;

	link->l_info = vxi;

	return 0;
}

static int vxlan_parse(struct rtnl_link *link, struct nlattr *data,
		      struct nlattr *xstats)
{
	struct nlattr *tb[IFLA_VXLAN_MAX+1];
	struct vxlan_info *vxi;
	int err;

	NL_DBG(3, "Parsing VXLAN link info");

	if ((err = nla_parse_nested(tb, IFLA_VXLAN_MAX, data, vxlan_policy)) < 0)
		goto errout;

	if ((err = vxlan_alloc(link)) < 0)
		goto errout;

	vxi = link->l_info;

	if (tb[IFLA_VXLAN_ID]) {
		vxi->vxi_id = nla_get_u32(tb[IFLA_VXLAN_ID]);
		vxi->vxi_mask |= VXLAN_HAS_ID;
	}

	if (tb[IFLA_VXLAN_GROUP]) {
		nla_memcpy(&vxi->vxi_group, tb[IFLA_VXLAN_GROUP],
				   sizeof(vxi->vxi_group));
		vxi->vxi_mask |= VXLAN_HAS_GROUP;
	}

	if (tb[IFLA_VXLAN_LINK]) {
		vxi->vxi_link = nla_get_u32(tb[IFLA_VXLAN_LINK]);
		vxi->vxi_mask |= VXLAN_HAS_LINK;
	}

	if (tb[IFLA_VXLAN_LOCAL]) {
		nla_memcpy(&vxi->vxi_local, tb[IFLA_VXLAN_LOCAL],
				   sizeof(vxi->vxi_local));
		vxi->vxi_mask |= VXLAN_HAS_LOCAL;
	}

	if (tb[IFLA_VXLAN_TTL]) {
		vxi->vxi_ttl = nla_get_u8(tb[IFLA_VXLAN_TTL]);
		vxi->vxi_mask |= VXLAN_HAS_TTL;
	}

	if (tb[IFLA_VXLAN_TOS]) {
		vxi->vxi_tos = nla_get_u8(tb[IFLA_VXLAN_TOS]);
		vxi->vxi_mask |= VXLAN_HAS_TOS;
	}

	if (tb[IFLA_VXLAN_LEARNING]) {
		vxi->vxi_learning = nla_get_u8(tb[IFLA_VXLAN_LEARNING]);
		vxi->vxi_mask |= VXLAN_HAS_LEARNING;
	}

	if (tb[IFLA_VXLAN_AGEING]) {
		vxi->vxi_ageing = nla_get_u32(tb[IFLA_VXLAN_AGEING]);
		vxi->vxi_mask |= VXLAN_HAS_AGEING;
	}

	if (tb[IFLA_VXLAN_LIMIT]) {
		vxi->vxi_limit = nla_get_u32(tb[IFLA_VXLAN_LIMIT]);
		vxi->vxi_mask |= VXLAN_HAS_LIMIT;
	}

	if (tb[IFLA_VXLAN_PORT_RANGE]) {
		nla_memcpy(&vxi->vxi_port_range, tb[IFLA_VXLAN_PORT_RANGE],
				   sizeof(vxi->vxi_port_range));
		vxi->vxi_mask |= VXLAN_HAS_PORT_RANGE;
	}

	if (tb[IFLA_VXLAN_PROXY]) {
		vxi->vxi_proxy = nla_get_u8(tb[IFLA_VXLAN_PROXY]);
		vxi->vxi_mask |= VXLAN_HAS_PROXY;
	}

	if (tb[IFLA_VXLAN_RSC]) {
		vxi->vxi_rsc = nla_get_u8(tb[IFLA_VXLAN_RSC]);
		vxi->vxi_mask |= VXLAN_HAS_RSC;
	}

	if (tb[IFLA_VXLAN_L2MISS]) {
		vxi->vxi_l2miss = nla_get_u8(tb[IFLA_VXLAN_L2MISS]);
		vxi->vxi_mask |= VXLAN_HAS_L2MISS;
	}

	if (tb[IFLA_VXLAN_L3MISS]) {
		vxi->vxi_l3miss = nla_get_u8(tb[IFLA_VXLAN_L3MISS]);
		vxi->vxi_mask |= VXLAN_HAS_L3MISS;
	}

	err = 0;

errout:
	return err;
}

static void vxlan_free(struct rtnl_link *link)
{
	struct vxlan_info *vxi = link->l_info;

	free(vxi);
	link->l_info = NULL;
}

static void vxlan_dump_line(struct rtnl_link *link, struct nl_dump_params *p)
{
	struct vxlan_info *vxi = link->l_info;

	nl_dump(p, "vxlan-id %u", vxi->vxi_id);
}

static void vxlan_dump_details(struct rtnl_link *link, struct nl_dump_params *p)
{
	struct vxlan_info *vxi = link->l_info;
	char *name, addr[INET_ADDRSTRLEN];

	nl_dump_line(p, "    vxlan-id %u\n", vxi->vxi_id);

	if (vxi->vxi_mask & VXLAN_HAS_GROUP) {
		nl_dump(p, "      group ");
		if(inet_ntop(AF_INET, &vxi->vxi_group, addr, sizeof(addr)))
			nl_dump_line(p, "%s\n", addr);
		else
			nl_dump_line(p, "%#x\n", ntohs(vxi->vxi_group));
	}

	if (vxi->vxi_mask & VXLAN_HAS_LINK) {
		nl_dump(p, "      link ");
		name = rtnl_link_get_name(link);
		if (name)
			nl_dump_line(p, "%s\n", name);
		else
			nl_dump_line(p, "%u\n", vxi->vxi_link);
	}

	if (vxi->vxi_mask & VXLAN_HAS_LOCAL) {
		nl_dump(p, "      local ");
		if(inet_ntop(AF_INET, &vxi->vxi_local, addr, sizeof(addr)))
			nl_dump_line(p, "%s\n", addr);
		else
			nl_dump_line(p, "%#x\n", ntohs(vxi->vxi_local));
	}

	if (vxi->vxi_mask & VXLAN_HAS_TTL) {
		nl_dump(p, "      ttl ");
		if(vxi->vxi_ttl)
			nl_dump_line(p, "%u\n", vxi->vxi_ttl);
		else
			nl_dump_line(p, "inherit\n");
	}

	if (vxi->vxi_mask & VXLAN_HAS_TOS) {
		nl_dump(p, "      tos ");
		if (vxi->vxi_tos == 1)
			nl_dump_line(p, "inherit\n", vxi->vxi_tos);
		else
			nl_dump_line(p, "%#x\n", vxi->vxi_tos);
	}

	if (vxi->vxi_mask & VXLAN_HAS_LEARNING) {
		nl_dump(p, "      learning ");
		if (vxi->vxi_learning)
			nl_dump_line(p, "enabled (%#x)\n", vxi->vxi_learning);
		else
			nl_dump_line(p, "disabled\n");
	}

	if (vxi->vxi_mask & VXLAN_HAS_AGEING) {
		nl_dump(p, "      ageing ");
		if (vxi->vxi_ageing)
			nl_dump_line(p, "%u seconds\n", vxi->vxi_ageing);
		else
			nl_dump_line(p, "disabled\n");
	}

	if (vxi->vxi_mask & VXLAN_HAS_LIMIT) {
		nl_dump(p, "      limit ");
		if (vxi->vxi_limit)
			nl_dump_line(p, "%u\n", vxi->vxi_limit);
		else
			nl_dump_line(p, "unlimited\n");
	}

	if (vxi->vxi_mask & VXLAN_HAS_PORT_RANGE)
		nl_dump_line(p, "      port range %u - %u\n",
					 ntohs(vxi->vxi_port_range.low),
					 ntohs(vxi->vxi_port_range.high));

	if (vxi->vxi_mask & VXLAN_HAS_PROXY) {
		nl_dump(p, "      proxy ");
		if (vxi->vxi_proxy)
			nl_dump_line(p, "enabled (%#x)\n", vxi->vxi_proxy);
		else
			nl_dump_line(p, "disabled\n");
	}

	if (vxi->vxi_mask & VXLAN_HAS_RSC) {
		nl_dump(p, "      rsc ");
		if (vxi->vxi_rsc)
			nl_dump_line(p, "enabled (%#x)\n", vxi->vxi_rsc);
		else
			nl_dump_line(p, "disabled\n");
	}

	if (vxi->vxi_mask & VXLAN_HAS_L2MISS) {
		nl_dump(p, "      l2miss ");
		if (vxi->vxi_l2miss)
			nl_dump_line(p, "enabled (%#x)\n", vxi->vxi_l2miss);
		else
			nl_dump_line(p, "disabled\n");
	}

	if (vxi->vxi_mask & VXLAN_HAS_L3MISS) {
		nl_dump(p, "      l3miss ");
		if (vxi->vxi_l3miss)
			nl_dump_line(p, "enabled (%#x)\n", vxi->vxi_l3miss);
		else
			nl_dump_line(p, "disabled\n");
	}
}

static int vxlan_clone(struct rtnl_link *dst, struct rtnl_link *src)
{
	struct vxlan_info *vdst, *vsrc = src->l_info;
	int err;

	dst->l_info = NULL;
	if ((err = rtnl_link_set_type(dst, "vxlan")) < 0)
		return err;
	vdst = dst->l_info;

	if (!vdst || !vsrc)
		return -NLE_NOMEM;

	memcpy(vdst, vsrc, sizeof(struct vxlan_info));

	return 0;
}

static int vxlan_put_attrs(struct nl_msg *msg, struct rtnl_link *link)
{
	struct vxlan_info *vxi = link->l_info;
	struct nlattr *data;

	if (!(data = nla_nest_start(msg, IFLA_INFO_DATA)))
		return -NLE_MSGSIZE;

	if (vxi->vxi_mask & VXLAN_HAS_ID)
		NLA_PUT_U32(msg, IFLA_VXLAN_ID, vxi->vxi_id);

	if (vxi->vxi_mask & VXLAN_HAS_GROUP)
		NLA_PUT(msg, IFLA_VXLAN_GROUP, sizeof(vxi->vxi_group), &vxi->vxi_group);

	if (vxi->vxi_mask & VXLAN_HAS_LINK)
		NLA_PUT_U32(msg, IFLA_VXLAN_LINK, vxi->vxi_link);

	if (vxi->vxi_mask & VXLAN_HAS_LOCAL)
		NLA_PUT(msg, IFLA_VXLAN_LOCAL, sizeof(vxi->vxi_local), &vxi->vxi_local);

	if (vxi->vxi_mask & VXLAN_HAS_TTL)
		NLA_PUT_U8(msg, IFLA_VXLAN_TTL, vxi->vxi_ttl);

	if (vxi->vxi_mask & VXLAN_HAS_TOS)
		NLA_PUT_U8(msg, IFLA_VXLAN_TOS, vxi->vxi_tos);

	if (vxi->vxi_mask & VXLAN_HAS_LEARNING)
		NLA_PUT_U8(msg, IFLA_VXLAN_LEARNING, vxi->vxi_learning);

	if (vxi->vxi_mask & VXLAN_HAS_AGEING)
		NLA_PUT_U32(msg, IFLA_VXLAN_AGEING, vxi->vxi_ageing);

	if (vxi->vxi_mask & VXLAN_HAS_LIMIT)
		NLA_PUT_U32(msg, IFLA_VXLAN_LIMIT, vxi->vxi_limit);

	if (vxi->vxi_mask & VXLAN_HAS_PORT_RANGE)
		NLA_PUT(msg, IFLA_VXLAN_PORT_RANGE, sizeof(vxi->vxi_port_range),
				&vxi->vxi_port_range);

	if (vxi->vxi_mask & VXLAN_HAS_PROXY)
		NLA_PUT_U8(msg, IFLA_VXLAN_PROXY, vxi->vxi_proxy);

	if (vxi->vxi_mask & VXLAN_HAS_RSC)
		NLA_PUT_U8(msg, IFLA_VXLAN_RSC, vxi->vxi_rsc);

	if (vxi->vxi_mask & VXLAN_HAS_L2MISS)
		NLA_PUT_U8(msg, IFLA_VXLAN_L2MISS, vxi->vxi_l2miss);

	if (vxi->vxi_mask & VXLAN_HAS_L3MISS)
		NLA_PUT_U8(msg, IFLA_VXLAN_L3MISS, vxi->vxi_l3miss);

	nla_nest_end(msg, data);

nla_put_failure:

	return 0;
}

static struct rtnl_link_info_ops vxlan_info_ops = {
	.io_name		= "vxlan",
	.io_alloc		= vxlan_alloc,
	.io_parse		= vxlan_parse,
	.io_dump = {
	    [NL_DUMP_LINE]	= vxlan_dump_line,
	    [NL_DUMP_DETAILS]	= vxlan_dump_details,
	},
	.io_clone		= vxlan_clone,
	.io_put_attrs		= vxlan_put_attrs,
	.io_free		= vxlan_free,
};

/** @cond SKIP */
#define IS_VXLAN_LINK_ASSERT(link) \
	if ((link)->l_info_ops != &vxlan_info_ops) { \
		APPBUG("Link is not a vxlan link. set type \"vxlan\" first."); \
		return -NLE_OPNOTSUPP; \
	}
/** @endcond */

/**
 * @name VXLAN Object
 * @{
 */

/**
 * Allocate link object of type VXLAN
 *
 * @return Allocated link object or NULL.
 */
struct rtnl_link *rtnl_link_vxlan_alloc(void)
{
	struct rtnl_link *link;
	int err;

	if (!(link = rtnl_link_alloc()))
		return NULL;

	if ((err = rtnl_link_set_type(link, "vxlan")) < 0) {
		rtnl_link_put(link);
		return NULL;
	}

	return link;
}

/**
 * Check if link is a VXLAN link
 * @arg link		Link object
 *
 * @return True if link is a VXLAN link, otherwise false is returned.
 */
int rtnl_link_is_vxlan(struct rtnl_link *link)
{
	return link->l_info_ops && !strcmp(link->l_info_ops->io_name, "vxlan");
}

/**
 * Set VXLAN Network Identifier
 * @arg link		Link object
 * @arg id		VXLAN network identifier (or VXLAN segment identifier)
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_set_id(struct rtnl_link *link, uint32_t id)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	if (id > VXLAN_ID_MAX)
		return -NLE_INVAL;

	vxi->vxi_id = id;
	vxi->vxi_mask |= VXLAN_HAS_ID;

	return 0;
}

/**
 * Get VXLAN Network Identifier
 * @arg link		Link object
 * @arg id			Pointer to store network identifier
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_get_id(struct rtnl_link *link, uint32_t *id)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	if(!id)
		return -NLE_INVAL;

	if (vxi->vxi_mask & VXLAN_HAS_ID)
		*id = vxi->vxi_id;
	else
		return -NLE_AGAIN;

	return 0;
}

/**
 * Set VXLAN multicast IP address
 * @arg link		Link object
 * @arg addr		Multicast IP address to join
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_set_group(struct rtnl_link *link, struct nl_addr *addr)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	if ((nl_addr_get_family(addr) != AF_INET) ||
		(nl_addr_get_len(addr) != sizeof(vxi->vxi_group)))
		return -NLE_INVAL;

	memcpy(&vxi->vxi_group, nl_addr_get_binary_addr(addr),
		   sizeof(vxi->vxi_group));
	vxi->vxi_mask |= VXLAN_HAS_GROUP;

	return 0;
}

/**
 * Get VXLAN multicast IP address
 * @arg link		Link object
 * @arg addr		Pointer to store multicast IP address
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_get_group(struct rtnl_link *link, struct nl_addr **addr)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	if (!addr)
		return -NLE_INVAL;

	if (!(vxi->vxi_mask & VXLAN_HAS_GROUP))
		return -NLE_AGAIN;

	*addr = nl_addr_build(AF_INET, &vxi->vxi_group, sizeof(vxi->vxi_group));

	return 0;
}

/**
 * Set physical device to use for VXLAN
 * @arg link		Link object
 * @arg index		Interface index
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_set_link(struct rtnl_link *link, uint32_t index)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	vxi->vxi_link = index;
	vxi->vxi_mask |= VXLAN_HAS_LINK;

	return 0;
}

/**
 * Get physical device to use for VXLAN
 * @arg link		Link object
 * @arg index		Pointer to store interface index
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_get_link(struct rtnl_link *link, uint32_t *index)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	if (!index)
		return -NLE_INVAL;

	if (!(vxi->vxi_mask & VXLAN_HAS_LINK))
		return -NLE_AGAIN;

	*index = vxi->vxi_link;

	return 0;
}

/**
 * Set source address to use for VXLAN
 * @arg link		Link object
 * @arg addr		Local address
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_set_local(struct rtnl_link *link, struct nl_addr *addr)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	if ((nl_addr_get_family(addr) != AF_INET) ||
		(nl_addr_get_len(addr) != sizeof(vxi->vxi_local)))
		return -NLE_INVAL;

	memcpy(&vxi->vxi_local, nl_addr_get_binary_addr(addr),
		   sizeof(vxi->vxi_local));
	vxi->vxi_mask |= VXLAN_HAS_LOCAL;

	return 0;
}

/**
 * Get source address to use for VXLAN
 * @arg link		Link object
 * @arg addr		Pointer to store local address
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_get_local(struct rtnl_link *link, struct nl_addr **addr)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	if (!addr)
		return -NLE_INVAL;

	if (!(vxi->vxi_mask & VXLAN_HAS_LOCAL))
		return -NLE_AGAIN;

	*addr = nl_addr_build(AF_INET, &vxi->vxi_local, sizeof(vxi->vxi_local));

	return 0;
}

/**
 * Set IP TTL value to use for VXLAN
 * @arg link		Link object
 * @arg ttl			TTL value
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_set_ttl(struct rtnl_link *link, uint8_t ttl)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	vxi->vxi_ttl = ttl;
	vxi->vxi_mask |= VXLAN_HAS_TTL;

	return 0;
}

/**
 * Get IP TTL value to use for VXLAN
 * @arg link		Link object
 *
 * @return TTL value on success or a negative error code
 */
int rtnl_link_vxlan_get_ttl(struct rtnl_link *link)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	if (!(vxi->vxi_mask & VXLAN_HAS_TTL))
		return -NLE_AGAIN;

	return vxi->vxi_ttl;
}

/**
 * Set IP ToS value to use for VXLAN
 * @arg link		Link object
 * @arg tos		ToS value
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_set_tos(struct rtnl_link *link, uint8_t tos)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	vxi->vxi_tos = tos;
	vxi->vxi_mask |= VXLAN_HAS_TOS;

	return 0;
}

/**
 * Get IP ToS value to use for VXLAN
 * @arg link		Link object
 *
 * @return ToS value on success or a negative error code
 */
int rtnl_link_vxlan_get_tos(struct rtnl_link *link)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	if (!(vxi->vxi_mask & VXLAN_HAS_TOS))
		return -NLE_AGAIN;

	return vxi->vxi_tos;
}

/**
 * Set VXLAN learning status
 * @arg link		Link object
 * @arg learning	Learning status value
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_set_learning(struct rtnl_link *link, uint8_t learning)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	vxi->vxi_learning = learning;
	vxi->vxi_mask |= VXLAN_HAS_LEARNING;

	return 0;
}

/**
 * Get VXLAN learning status
 * @arg link		Link object
 *
 * @return Learning status value on success or a negative error code
 */
int rtnl_link_vxlan_get_learning(struct rtnl_link *link)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	if (!(vxi->vxi_mask & VXLAN_HAS_LEARNING))
		return -NLE_AGAIN;

	return vxi->vxi_learning;
}

/**
 * Enable VXLAN address learning
 * @arg link		Link object
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_enable_learning(struct rtnl_link *link)
{
	return rtnl_link_vxlan_set_learning(link, 1);
}

/**
 * Disable VXLAN address learning
 * @arg link		Link object
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_disable_learning(struct rtnl_link *link)
{
	return rtnl_link_vxlan_set_learning(link, 0);
}

/**
 * Set expiration timer value to use for VXLAN
 * @arg link		Link object
 * @arg expiry		Expiration timer value
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_set_ageing(struct rtnl_link *link, uint32_t expiry)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	vxi->vxi_ageing = expiry;
	vxi->vxi_mask |= VXLAN_HAS_AGEING;

	return 0;
}

/**
 * Get expiration timer value to use for VXLAN
 * @arg link		Link object
 * @arg expiry		Pointer to store expiration timer value
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_get_ageing(struct rtnl_link *link, uint32_t *expiry)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	if (!expiry)
		return -NLE_INVAL;

	if (vxi->vxi_mask & VXLAN_HAS_AGEING)
		*expiry = vxi->vxi_ageing;
	else
		return -NLE_AGAIN;

	return 0;
}

/**
 * Set maximum number of forwarding database entries to use for VXLAN
 * @arg link		Link object
 * @arg limit		Maximum number
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_set_limit(struct rtnl_link *link, uint32_t limit)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	vxi->vxi_limit = limit;
	vxi->vxi_mask |= VXLAN_HAS_LIMIT;

	return 0;
}

/**
 * Get maximum number of forwarding database entries to use for VXLAN
 * @arg link		Link object
 * @arg limit		Pointer to store maximum number
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_get_limit(struct rtnl_link *link, uint32_t *limit)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	if (!limit)
		return -NLE_INVAL;

	if (vxi->vxi_mask & VXLAN_HAS_LIMIT)
		*limit = vxi->vxi_limit;
	else
		return -NLE_AGAIN;

	return 0;
}

/**
 * Set range of UDP port numbers to use for VXLAN
 * @arg link		Link object
 * @arg range		Port number range
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_set_port_range(struct rtnl_link *link,
								   struct ifla_vxlan_port_range *range)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	if (!range)
		return -NLE_INVAL;

	memcpy(&vxi->vxi_port_range, range, sizeof(vxi->vxi_port_range));
	vxi->vxi_mask |= VXLAN_HAS_PORT_RANGE;

	return 0;
}

/**
 * Get range of UDP port numbers to use for VXLAN
 * @arg link		Link object
 * @arg range		Pointer to store port range
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_get_port_range(struct rtnl_link *link,
								   struct ifla_vxlan_port_range *range)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	if (!range)
		return -NLE_INVAL;

	if (vxi->vxi_mask & VXLAN_HAS_PORT_RANGE)
		memcpy(range, &vxi->vxi_port_range, sizeof(*range));
	else
		return -NLE_AGAIN;

	return 0;
}

/**
 * Set ARP proxy status to use for VXLAN
 * @arg link		Link object
 * @arg proxy		Status value
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_set_proxy(struct rtnl_link *link, uint8_t proxy)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	vxi->vxi_proxy = proxy;
	vxi->vxi_mask |= VXLAN_HAS_PROXY;

	return 0;
}

/**
 * Get ARP proxy status to use for VXLAN
 * @arg link		Link object
 *
 * @return Status value on success or a negative error code
 */
int rtnl_link_vxlan_get_proxy(struct rtnl_link *link)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	if (!(vxi->vxi_mask & VXLAN_HAS_PROXY))
		return -NLE_AGAIN;

	return vxi->vxi_proxy;
}

/**
 * Enable ARP proxy
 * @arg link		Link object
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_enable_proxy(struct rtnl_link *link)
{
	return rtnl_link_vxlan_set_proxy(link, 1);
}

/**
 * Disable ARP proxy
 * @arg link		Link object
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_disable_proxy(struct rtnl_link *link)
{
	return rtnl_link_vxlan_set_proxy(link, 0);
}

/**
 * Set Route Short Circuit status to use for VXLAN
 * @arg link		Link object
 * @arg rsc			Status value
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_set_rsc(struct rtnl_link *link, uint8_t rsc)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	vxi->vxi_rsc = rsc;
	vxi->vxi_mask |= VXLAN_HAS_RSC;

	return 0;
}

/**
 * Get Route Short Circuit status to use for VXLAN
 * @arg link		Link object
 *
 * @return Status value on success or a negative error code
 */
int rtnl_link_vxlan_get_rsc(struct rtnl_link *link)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	if (!(vxi->vxi_mask & VXLAN_HAS_RSC))
		return -NLE_AGAIN;

	return vxi->vxi_rsc;
}

/**
 * Enable Route Short Circuit
 * @arg link		Link object
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_enable_rsc(struct rtnl_link *link)
{
	return rtnl_link_vxlan_set_rsc(link, 1);
}

/**
 * Disable Route Short Circuit
 * @arg link		Link object
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_disable_rsc(struct rtnl_link *link)
{
	return rtnl_link_vxlan_set_rsc(link, 0);
}

/**
 * Set netlink LLADDR miss notification status to use for VXLAN
 * @arg link		Link object
 * @arg miss		Status value
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_set_l2miss(struct rtnl_link *link, uint8_t miss)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	vxi->vxi_l2miss = miss;
	vxi->vxi_mask |= VXLAN_HAS_L2MISS;

	return 0;
}

/**
 * Get netlink LLADDR miss notification status to use for VXLAN
 * @arg link		Link object
 *
 * @return Status value on success or a negative error code
 */
int rtnl_link_vxlan_get_l2miss(struct rtnl_link *link)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	if (!(vxi->vxi_mask & VXLAN_HAS_L2MISS))
		return -NLE_AGAIN;

	return vxi->vxi_l2miss;
}

/**
 * Enable netlink LLADDR miss notifications
 * @arg link		Link object
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_enable_l2miss(struct rtnl_link *link)
{
	return rtnl_link_vxlan_set_l2miss(link, 1);
}

/**
 * Disable netlink LLADDR miss notifications
 * @arg link		Link object
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_disable_l2miss(struct rtnl_link *link)
{
	return rtnl_link_vxlan_set_l2miss(link, 0);
}

/**
 * Set netlink IP ADDR miss notification status to use for VXLAN
 * @arg link		Link object
 * @arg miss		Status value
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_set_l3miss(struct rtnl_link *link, uint8_t miss)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	vxi->vxi_l3miss = miss;
	vxi->vxi_mask |= VXLAN_HAS_L3MISS;

	return 0;
}

/**
 * Get netlink IP ADDR miss notification status to use for VXLAN
 * @arg link		Link object
 *
 * @return Status value on success or a negative error code
 */
int rtnl_link_vxlan_get_l3miss(struct rtnl_link *link)
{
	struct vxlan_info *vxi = link->l_info;

	IS_VXLAN_LINK_ASSERT(link);

	if (!(vxi->vxi_mask & VXLAN_HAS_L3MISS))
		return -NLE_AGAIN;

	return vxi->vxi_l3miss;
}

/**
 * Enable netlink IP DDR miss notifications
 * @arg link		Link object
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_enable_l3miss(struct rtnl_link *link)
{
	return rtnl_link_vxlan_set_l3miss(link, 1);
}

/**
 * Disable netlink IP ADDR miss notifications
 * @arg link		Link object
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_vxlan_disable_l3miss(struct rtnl_link *link)
{
	return rtnl_link_vxlan_set_l3miss(link, 0);
}

/** @} */

static void __init vxlan_init(void)
{
	rtnl_link_register_info(&vxlan_info_ops);
}

static void __exit vxlan_exit(void)
{
	rtnl_link_unregister_info(&vxlan_info_ops);
}

/** @} */

/*
 * lib/route/link/ip6tnl.c        IP6TNL Link Info
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License as published by the Free Software Foundation version 2.1
 *      of the License.
 *
 * Copyright (c) 2014 Susant Sahani <susant@redhat.com>
 */

/**
 * @ingroup link
 * @defgroup ip6tnl IP6TNL
 * ip6tnl link module
 *
 * @details
 * \b Link Type Name: "ip6tnl"
 *
 * @route_doc{link_ip6tnl, IP6TNL Documentation}
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
#include <linux/if_tunnel.h>
#include <netinet/in.h>

#define IP6_TNL_ATTR_LINK          (1 << 0)
#define IP6_TNL_ATTR_LOCAL         (1 << 1)
#define IP6_TNL_ATTR_REMOTE        (1 << 2)
#define IP6_TNL_ATTR_TTL           (1 << 3)
#define IP6_TNL_ATTR_TOS           (1 << 4)
#define IP6_TNL_ATTR_ENCAPLIMIT    (1 << 5)
#define IP6_TNL_ATTR_FLAGS         (1 << 6)
#define IP6_TNL_ATTR_PROTO         (1 << 7)
#define IP6_TNL_ATTR_FLOWINFO      (1 << 8)

struct ip6_tnl_info
{
	uint8_t                 ttl;
	uint8_t                 tos;
	uint8_t                 encap_limit;
	uint8_t                 proto;
	uint32_t                flags;
	uint32_t                link;
	uint32_t                flowinfo;
	struct in6_addr         local;
	struct in6_addr         remote;
	uint32_t                ip6_tnl_mask;
};

static struct nla_policy ip6_tnl_policy[IFLA_IPTUN_MAX + 1] = {
	[IFLA_IPTUN_LINK]         = { .type = NLA_U32 },
	[IFLA_IPTUN_LOCAL]        = { .minlen = sizeof(struct in6_addr) },
	[IFLA_IPTUN_REMOTE]       = { .minlen = sizeof(struct in6_addr) },
	[IFLA_IPTUN_TTL]          = { .type = NLA_U8 },
	[IFLA_IPTUN_TOS]          = { .type = NLA_U8 },
	[IFLA_IPTUN_ENCAP_LIMIT]  = { .type = NLA_U8 },
	[IFLA_IPTUN_FLOWINFO]     = { .type = NLA_U32 },
	[IFLA_IPTUN_FLAGS]        = { .type = NLA_U32 },
	[IFLA_IPTUN_PROTO]        = { .type = NLA_U8 },
};

static int ip6_tnl_alloc(struct rtnl_link *link)
{
	struct ip6_tnl_info *ip6_tnl;

	ip6_tnl = calloc(1, sizeof(*ip6_tnl));
	if (!ip6_tnl)
		return -NLE_NOMEM;

	link->l_info = ip6_tnl;

	return 0;
}

static int ip6_tnl_parse(struct rtnl_link *link, struct nlattr *data,
			 struct nlattr *xstats)
{
	struct nlattr *tb[IFLA_IPTUN_MAX + 1];
	struct ip6_tnl_info *ip6_tnl;
	int err;

	NL_DBG(3, "Parsing IP6_TNL link info");

	err = nla_parse_nested(tb, IFLA_IPTUN_MAX, data, ip6_tnl_policy);
	if (err < 0)
		goto errout;

	err = ip6_tnl_alloc(link);
	if (err < 0)
		goto errout;

	ip6_tnl = link->l_info;

	if (tb[IFLA_IPTUN_LINK]) {
		ip6_tnl->link = nla_get_u32(tb[IFLA_IPTUN_LINK]);
		ip6_tnl->ip6_tnl_mask |= IP6_TNL_ATTR_LINK;
	}

	if (tb[IFLA_IPTUN_LOCAL]) {
		nla_memcpy(&ip6_tnl->local, tb[IFLA_IPTUN_LOCAL], sizeof(struct in6_addr));
		ip6_tnl->ip6_tnl_mask |= IP6_TNL_ATTR_LOCAL;
	}

	if (tb[IFLA_IPTUN_REMOTE]) {
		nla_memcpy(&ip6_tnl->remote, tb[IFLA_IPTUN_REMOTE], sizeof(struct in6_addr));
		ip6_tnl->ip6_tnl_mask |= IP6_TNL_ATTR_REMOTE;
	}

	if (tb[IFLA_IPTUN_TTL]) {
		ip6_tnl->ttl = nla_get_u8(tb[IFLA_IPTUN_TTL]);
		ip6_tnl->ip6_tnl_mask |= IP6_TNL_ATTR_TTL;
	}

	if (tb[IFLA_IPTUN_TOS]) {
		ip6_tnl->tos = nla_get_u8(tb[IFLA_IPTUN_TOS]);
		ip6_tnl->ip6_tnl_mask |= IP6_TNL_ATTR_TOS;
	}

	if (tb[IFLA_IPTUN_ENCAP_LIMIT]) {
		ip6_tnl->encap_limit = nla_get_u8(tb[IFLA_IPTUN_ENCAP_LIMIT]);
		ip6_tnl->ip6_tnl_mask |= IP6_TNL_ATTR_ENCAPLIMIT;
	}

	if (tb[IFLA_IPTUN_FLAGS]) {
		ip6_tnl->flags = nla_get_u32(tb[IFLA_IPTUN_FLAGS]);
		ip6_tnl->ip6_tnl_mask |= IP6_TNL_ATTR_FLAGS;
	}

	if (tb[IFLA_IPTUN_FLOWINFO]) {
		ip6_tnl->flowinfo = nla_get_u32(tb[IFLA_IPTUN_FLOWINFO]);
		ip6_tnl->ip6_tnl_mask |= IP6_TNL_ATTR_FLOWINFO;
	}

	if (tb[IFLA_IPTUN_PROTO]) {
		ip6_tnl->proto = nla_get_u8(tb[IFLA_IPTUN_PROTO]);
		ip6_tnl->ip6_tnl_mask |= IP6_TNL_ATTR_PROTO;
	}

	err = 0;

errout:
	return err;
}

static int ip6_tnl_put_attrs(struct nl_msg *msg, struct rtnl_link *link)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;
	struct nlattr *data;

	data = nla_nest_start(msg, IFLA_INFO_DATA);
	if (!data)
		return -NLE_MSGSIZE;

	if (ip6_tnl->ip6_tnl_mask & IP6_TNL_ATTR_LINK)
		NLA_PUT_U32(msg, IFLA_IPTUN_LINK, ip6_tnl->link);

	if (ip6_tnl->ip6_tnl_mask & IP6_TNL_ATTR_LOCAL)
		NLA_PUT(msg, IFLA_IPTUN_LOCAL, sizeof(struct in6_addr), &ip6_tnl->local);

	if (ip6_tnl->ip6_tnl_mask & IP6_TNL_ATTR_REMOTE)
		NLA_PUT(msg, IFLA_IPTUN_REMOTE, sizeof(struct in6_addr), &ip6_tnl->remote);

	if (ip6_tnl->ip6_tnl_mask & IP6_TNL_ATTR_TTL)
		NLA_PUT_U8(msg, IFLA_IPTUN_TTL, ip6_tnl->ttl);

	if (ip6_tnl->ip6_tnl_mask & IP6_TNL_ATTR_TOS)
		NLA_PUT_U8(msg, IFLA_IPTUN_TOS, ip6_tnl->tos);

	if (ip6_tnl->ip6_tnl_mask & IP6_TNL_ATTR_ENCAPLIMIT)
		NLA_PUT_U8(msg, IFLA_IPTUN_ENCAP_LIMIT, ip6_tnl->encap_limit);

	if (ip6_tnl->ip6_tnl_mask & IP6_TNL_ATTR_FLAGS)
		NLA_PUT_U32(msg, IFLA_IPTUN_FLAGS, ip6_tnl->flags);

	if (ip6_tnl->ip6_tnl_mask & IP6_TNL_ATTR_FLOWINFO)
		NLA_PUT_U32(msg, IFLA_IPTUN_FLOWINFO, ip6_tnl->flowinfo);

	/* kernel crashes if this attribure is missing  temporary fix */
	if (ip6_tnl->ip6_tnl_mask & IP6_TNL_ATTR_PROTO)
		NLA_PUT_U8(msg, IFLA_IPTUN_PROTO, ip6_tnl->proto);
	else
		NLA_PUT_U8(msg, IFLA_IPTUN_PROTO, 0);

	nla_nest_end(msg, data);

nla_put_failure:
	return 0;
}

static void ip6_tnl_free(struct rtnl_link *link)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;

	free(ip6_tnl);
	link->l_info = NULL;
}

static void ip6_tnl_dump_line(struct rtnl_link *link, struct nl_dump_params *p)
{
	nl_dump(p, "ip6_tnl : %s", link->l_name);
}

static void ip6_tnl_dump_details(struct rtnl_link *link, struct nl_dump_params *p)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;
	char *name, addr[INET6_ADDRSTRLEN];

	if (ip6_tnl->ip6_tnl_mask & IP6_TNL_ATTR_LINK) {
		nl_dump(p, "      link ");
		name = rtnl_link_get_name(link);
		if (name)
			nl_dump_line(p, "%s\n", name);
		else
			nl_dump_line(p, "%u\n", ip6_tnl->link);
	}

	if (ip6_tnl->ip6_tnl_mask & IP6_TNL_ATTR_LOCAL) {
		nl_dump(p, "      local ");

		if(inet_ntop(AF_INET6, &ip6_tnl->local, addr, INET6_ADDRSTRLEN))
			nl_dump_line(p, "%s\n", addr);
		else
			nl_dump_line(p, "%#x\n", ip6_tnl->local);
	}

	if (ip6_tnl->ip6_tnl_mask & IP6_TNL_ATTR_REMOTE) {
		nl_dump(p, "      remote ");

		if(inet_ntop(AF_INET6, &ip6_tnl->remote, addr, INET6_ADDRSTRLEN))
			nl_dump_line(p, "%s\n", addr);
		else
			nl_dump_line(p, "%#x\n", ip6_tnl->remote);
	}

	if (ip6_tnl->ip6_tnl_mask & IP6_TNL_ATTR_TTL) {
		nl_dump(p, "      ttl ");
		nl_dump_line(p, "%d\n", ip6_tnl->ttl);
	}

	if (ip6_tnl->ip6_tnl_mask & IP6_TNL_ATTR_TOS) {
		nl_dump(p, "      tos ");
		nl_dump_line(p, "%d\n", ip6_tnl->tos);
	}

	if (ip6_tnl->ip6_tnl_mask & IP6_TNL_ATTR_ENCAPLIMIT) {
		nl_dump(p, "      encaplimit ");
		nl_dump_line(p, "%d\n", ip6_tnl->encap_limit);
	}

	if (ip6_tnl->ip6_tnl_mask & IP6_TNL_ATTR_FLAGS) {
		nl_dump(p, "      flags ");
		nl_dump_line(p, " (%x)\n", ip6_tnl->flags);
	}

	if (ip6_tnl->ip6_tnl_mask & IP6_TNL_ATTR_FLOWINFO) {
		nl_dump(p, "      flowinfo ");
		nl_dump_line(p, " (%x)\n", ip6_tnl->flowinfo);
	}

	if (ip6_tnl->ip6_tnl_mask & IP6_TNL_ATTR_PROTO) {
		nl_dump(p, "    proto   ");
		nl_dump_line(p, " (%x)\n", ip6_tnl->proto);
	}
}

static int ip6_tnl_clone(struct rtnl_link *dst, struct rtnl_link *src)
{
	struct ip6_tnl_info *ip6_tnl_dst, *ip6_tnl_src = src->l_info;
	int err;

	dst->l_info = NULL;

	err = rtnl_link_set_type(dst, "ip6tnl");
	if (err < 0)
		return err;

	ip6_tnl_dst = dst->l_info;

	if (!ip6_tnl_dst || !ip6_tnl_src)
		BUG();

	memcpy(ip6_tnl_dst, ip6_tnl_src, sizeof(struct ip6_tnl_info));

	return 0;
}

static struct rtnl_link_info_ops ip6_tnl_info_ops = {
	.io_name                = "ip6tnl",
	.io_alloc               = ip6_tnl_alloc,
	.io_parse               = ip6_tnl_parse,
	.io_dump = {
		[NL_DUMP_LINE]  = ip6_tnl_dump_line,
		[NL_DUMP_DETAILS] = ip6_tnl_dump_details,
	},
	.io_clone               = ip6_tnl_clone,
	.io_put_attrs           = ip6_tnl_put_attrs,
	.io_free                = ip6_tnl_free,
};

#define IS_IP6_TNL_LINK_ASSERT(link)\
	if ((link)->l_info_ops != &ip6_tnl_info_ops) {\
		APPBUG("Link is not a ip6_tnl link. set type \"ip6tnl\" first.");\
		return -NLE_OPNOTSUPP;\
	}

struct rtnl_link *rtnl_link_ip6_tnl_alloc(void)
{
	struct rtnl_link *link;
	int err;

	link = rtnl_link_alloc();
	if (!link)
		return NULL;

	err = rtnl_link_set_type(link, "ip6tnl");
	if (err < 0) {
		rtnl_link_put(link);
		return NULL;
	}

	return link;
}

/**
 * Check if link is a IP6_TNL link
 * @arg link            Link object
 *
 * @return True if link is a IP6_TNL link, otherwise false is returned.
 */
int rtnl_link_is_ip6_tnl(struct rtnl_link *link)
{
	return link->l_info_ops && !strcmp(link->l_info_ops->io_name, "ip6tnl");
}

/**
 * Create a new ip6_tnl tunnel device
 * @arg sock            netlink socket
 * @arg name            name of the tunnel device
 *
 * Creates a new ip6_tnl tunnel device in the kernel
 * @return 0 on success or a negative error code
 */
int rtnl_link_ip6_tnl_add(struct nl_sock *sk, const char *name)
{
	struct rtnl_link *link;
	int err;

	link = rtnl_link_ip6_tnl_alloc();
	if (!link)
		return -NLE_NOMEM;

	if(name)
		rtnl_link_set_name(link, name);

	err = rtnl_link_add(sk, link, NLM_F_CREATE);
	rtnl_link_put(link);

	return err;
}

/**
 * Set IP6_TNL tunnel interface index
 * @arg link            Link object
 * @arg index           interface index
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ip6_tnl_set_link(struct rtnl_link *link, uint32_t index)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;

	IS_IP6_TNL_LINK_ASSERT(link);

	ip6_tnl->link = index;
	ip6_tnl->ip6_tnl_mask |= IP6_TNL_ATTR_LINK;

	return 0;
}

/**
 * Get IP6_TNL tunnel interface index
 * @arg link            Link object
 *
 * @return interface index value
 */
uint32_t rtnl_link_ip6_tnl_get_link(struct rtnl_link *link)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;

	IS_IP6_TNL_LINK_ASSERT(link);

	return ip6_tnl->link;
}

/**
 * Set IP6_TNL tunnel local address
 * @arg link            Link object
 * @arg addr            local address
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ip6_tnl_set_local(struct rtnl_link *link, struct in6_addr *addr)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;

	IS_IP6_TNL_LINK_ASSERT(link);

	memcpy(&ip6_tnl->local, addr, sizeof(struct in6_addr));
	ip6_tnl->ip6_tnl_mask |= IP6_TNL_ATTR_LOCAL;

	return 0;
}

/**
 * Get IP6_TNL tunnel local address
 * @arg link            Link object
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ip6_tnl_get_local(struct rtnl_link *link, struct in6_addr *addr)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;

	IS_IP6_TNL_LINK_ASSERT(link);

	memcpy(addr, &ip6_tnl->local, sizeof(struct in6_addr));

	return 0;
}

/**
 * Set IP6_TNL tunnel remote address
 * @arg link            Link object
 * @arg remote          remote address
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ip6_tnl_set_remote(struct rtnl_link *link, struct in6_addr *addr)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;

	IS_IP6_TNL_LINK_ASSERT(link);

	memcpy(&ip6_tnl->remote, addr, sizeof(struct in6_addr));
	ip6_tnl->ip6_tnl_mask |= IP6_TNL_ATTR_REMOTE;

	return 0;
}

/**
 * Get IP6_TNL tunnel remote address
 * @arg link            Link object
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ip6_tnl_get_remote(struct rtnl_link *link, struct in6_addr *addr)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;

	IS_IP6_TNL_LINK_ASSERT(link);

	memcpy(addr, &ip6_tnl->remote, sizeof(struct in6_addr));

	return 0;
}

/**
 * Set IP6_TNL tunnel ttl
 * @arg link            Link object
 * @arg ttl             tunnel ttl
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ip6_tnl_set_ttl(struct rtnl_link *link, uint8_t ttl)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;

	IS_IP6_TNL_LINK_ASSERT(link);

	ip6_tnl->ttl = ttl;
	ip6_tnl->ip6_tnl_mask |= IP6_TNL_ATTR_TTL;

	return 0;
}

/**
 * Get IP6_TNL tunnel ttl
 * @arg link            Link object
 *
 * @return ttl value
 */
uint8_t rtnl_link_ip6_tnl_get_ttl(struct rtnl_link *link)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;

	IS_IP6_TNL_LINK_ASSERT(link);

	return ip6_tnl->ttl;
}

/**
 * Set IP6_TNL tunnel tos
 * @arg link            Link object
 * @arg tos             tunnel tos
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ip6_tnl_set_tos(struct rtnl_link *link, uint8_t tos)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;

	IS_IP6_TNL_LINK_ASSERT(link);

	ip6_tnl->tos = tos;
	ip6_tnl->ip6_tnl_mask |= IP6_TNL_ATTR_TOS;

	return 0;
}

/**
 * Get IP6_TNL tunnel tos
 * @arg link            Link object
 *
 * @return tos value
 */
uint8_t rtnl_link_ip6_tnl_get_tos(struct rtnl_link *link)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;

	IS_IP6_TNL_LINK_ASSERT(link);

	return ip6_tnl->tos;
}

/**
 * Set IP6_TNL tunnel encap limit
 * @arg link            Link object
 * @arg encap_limit     encaplimit value
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ip6_tnl_set_encaplimit(struct rtnl_link *link, uint8_t encap_limit)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;

	IS_IP6_TNL_LINK_ASSERT(link);

	ip6_tnl->encap_limit = encap_limit;
	ip6_tnl->ip6_tnl_mask |= IP6_TNL_ATTR_ENCAPLIMIT;

	return 0;
}

/**
 * Get IP6_TNL encaplimit
 * @arg link            Link object
 *
 * @return encaplimit value
 */
uint8_t rtnl_link_ip6_tnl_get_encaplimit(struct rtnl_link *link)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;

	IS_IP6_TNL_LINK_ASSERT(link);

	return ip6_tnl->encap_limit;
}

/**
 * Set IP6_TNL tunnel flowinfo
 * @arg link            Link object
 * @arg flowinfo        flowinfo value
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ip6_tnl_set_flowinfo(struct rtnl_link *link, uint32_t flowinfo)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;

	IS_IP6_TNL_LINK_ASSERT(link);

	ip6_tnl->flowinfo = flowinfo;
	ip6_tnl->ip6_tnl_mask |= IP6_TNL_ATTR_FLOWINFO;

	return 0;
}

/**
 * Get IP6_TNL flowinfo
 * @arg link            Link object
 *
 * @return flowinfo value
 */
uint32_t rtnl_link_ip6_tnl_get_flowinfo(struct rtnl_link *link)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;

	IS_IP6_TNL_LINK_ASSERT(link);

	return ip6_tnl->flowinfo;
}

/**
 * Set IP6_TNL tunnel flags
 * @arg link            Link object
 * @arg flags           tunnel flags
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ip6_tnl_set_flags(struct rtnl_link *link, uint32_t flags)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;

	IS_IP6_TNL_LINK_ASSERT(link);

	ip6_tnl->flags = flags;
	ip6_tnl->ip6_tnl_mask |= IP6_TNL_ATTR_FLAGS;

	return 0;
}

/**
 * Get IP6_TNL path flags
 * @arg link            Link object
 *
 * @return flags value
 */
uint32_t rtnl_link_ip6_tnl_get_flags(struct rtnl_link *link)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;

	IS_IP6_TNL_LINK_ASSERT(link);

	return ip6_tnl->flags;
}

/**
 * Set IP6_TNL tunnel proto
 * @arg link            Link object
 * @arg proto           tunnel proto
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ip6_tnl_set_proto(struct rtnl_link *link, uint8_t proto)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;

	IS_IP6_TNL_LINK_ASSERT(link);

	ip6_tnl->proto = proto;
	ip6_tnl->ip6_tnl_mask |= IP6_TNL_ATTR_PROTO;

	return 0;
}

/**
 * Get IP6_TNL proto
 * @arg link            Link object
 *
 * @return proto value
 */
uint8_t rtnl_link_ip6_tnl_get_proto(struct rtnl_link *link)
{
	struct ip6_tnl_info *ip6_tnl = link->l_info;

	IS_IP6_TNL_LINK_ASSERT(link);

	return ip6_tnl->proto;
}

static void __init ip6_tnl_init(void)
{
	rtnl_link_register_info(&ip6_tnl_info_ops);
}

static void __exit ip6_tnl_exit(void)
{
	rtnl_link_unregister_info(&ip6_tnl_info_ops);
}

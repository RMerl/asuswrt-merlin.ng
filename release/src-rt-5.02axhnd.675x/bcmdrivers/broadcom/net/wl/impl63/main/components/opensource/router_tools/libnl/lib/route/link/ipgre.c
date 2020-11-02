/*
 * lib/route/link/ipgre.c        IPGRE Link Info
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
 * @defgroup ipgre IPGRE
 * ipgre link module
 *
 * @details
 * \b Link Type Name: "ipgre"
 *
 * @route_doc{link_ipgre, IPGRE Documentation}
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

#define IPGRE_ATTR_LINK          (1 << 0)
#define IPGRE_ATTR_IFLAGS        (1 << 1)
#define IPGRE_ATTR_OFLAGS        (1 << 2)
#define IPGRE_ATTR_IKEY          (1 << 3)
#define IPGRE_ATTR_OKEY          (1 << 4)
#define IPGRE_ATTR_LOCAL         (1 << 5)
#define IPGRE_ATTR_REMOTE        (1 << 6)
#define IPGRE_ATTR_TTL           (1 << 7)
#define IPGRE_ATTR_TOS           (1 << 8)
#define IPGRE_ATTR_PMTUDISC      (1 << 9)

struct ipgre_info
{
	uint8_t    ttl;
	uint8_t    tos;
	uint8_t    pmtudisc;
	uint16_t   iflags;
	uint16_t   oflags;
	uint32_t   ikey;
	uint32_t   okey;
	uint32_t   link;
	uint32_t   local;
	uint32_t   remote;
	uint32_t   ipgre_mask;
};

static  struct nla_policy ipgre_policy[IFLA_GRE_MAX + 1] = {
	[IFLA_GRE_LINK]     = { .type = NLA_U32 },
	[IFLA_GRE_IFLAGS]   = { .type = NLA_U16 },
	[IFLA_GRE_OFLAGS]   = { .type = NLA_U16 },
	[IFLA_GRE_IKEY]     = { .type = NLA_U32 },
	[IFLA_GRE_OKEY]     = { .type = NLA_U32 },
	[IFLA_GRE_LOCAL]    = { .type = NLA_U32 },
	[IFLA_GRE_REMOTE]   = { .type = NLA_U32 },
	[IFLA_GRE_TTL]      = { .type = NLA_U8 },
	[IFLA_GRE_TOS]      = { .type = NLA_U8 },
	[IFLA_GRE_PMTUDISC] = { .type = NLA_U8 },
};

static int ipgre_alloc(struct rtnl_link *link)
{
	struct ipgre_info *ipgre;

	ipgre = calloc(1, sizeof(*ipgre));
	if (!ipgre)
		return -NLE_NOMEM;

	link->l_info = ipgre;

	return 0;
}

static int ipgre_parse(struct rtnl_link *link, struct nlattr *data,
		       struct nlattr *xstats)
{
	struct nlattr *tb[IFLA_IPTUN_MAX + 1];
	struct ipgre_info *ipgre;
	int err;

	NL_DBG(3, "Parsing IPGRE link info");

	err = nla_parse_nested(tb, IFLA_GRE_MAX, data, ipgre_policy);
	if (err < 0)
		goto errout;

	err = ipgre_alloc(link);
	if (err < 0)
		goto errout;

	ipgre = link->l_info;

	if (tb[IFLA_GRE_LINK]) {
		ipgre->link = nla_get_u32(tb[IFLA_GRE_LINK]);
		ipgre->ipgre_mask |= IPGRE_ATTR_LINK;
	}

	if (tb[IFLA_GRE_IFLAGS]) {
		ipgre->iflags = nla_get_u16(tb[IFLA_GRE_IFLAGS]);
		ipgre->ipgre_mask |= IPGRE_ATTR_IFLAGS;
	}

	if (tb[IFLA_GRE_OFLAGS]) {
		ipgre->oflags = nla_get_u16(tb[IFLA_GRE_OFLAGS]);
		ipgre->ipgre_mask |= IPGRE_ATTR_OFLAGS;
	}

	if (tb[IFLA_GRE_IKEY]) {
		ipgre->ikey = nla_get_u32(tb[IFLA_GRE_IKEY]);
		ipgre->ipgre_mask |= IPGRE_ATTR_IKEY;
	}

	if (tb[IFLA_GRE_OKEY]) {
		ipgre->okey = nla_get_u32(tb[IFLA_GRE_OKEY]);
		ipgre->ipgre_mask |= IPGRE_ATTR_OKEY;
	}

	if (tb[IFLA_GRE_LOCAL]) {
		ipgre->local = nla_get_u32(tb[IFLA_GRE_LOCAL]);
		ipgre->ipgre_mask |= IPGRE_ATTR_LOCAL;
	}

	if (tb[IFLA_GRE_LOCAL]) {
		ipgre->remote = nla_get_u32(tb[IFLA_GRE_LOCAL]);
		ipgre->ipgre_mask |= IPGRE_ATTR_REMOTE;
	}

	if (tb[IFLA_GRE_TTL]) {
		ipgre->ttl = nla_get_u8(tb[IFLA_GRE_TTL]);
		ipgre->ipgre_mask |= IPGRE_ATTR_TTL;
	}

	if (tb[IFLA_GRE_TOS]) {
		ipgre->tos = nla_get_u8(tb[IFLA_GRE_TOS]);
		ipgre->ipgre_mask |= IPGRE_ATTR_TOS;
	}

	if (tb[IFLA_GRE_PMTUDISC]) {
		ipgre->pmtudisc = nla_get_u8(tb[IFLA_GRE_PMTUDISC]);
		ipgre->ipgre_mask |= IPGRE_ATTR_PMTUDISC;
	}

	err = 0;

 errout:
	return err;
}

static int ipgre_put_attrs(struct nl_msg *msg, struct rtnl_link *link)
{
	struct ipgre_info *ipgre = link->l_info;
	struct nlattr *data;

	data = nla_nest_start(msg, IFLA_INFO_DATA);
	if (!data)
		return -NLE_MSGSIZE;

	if (ipgre->ipgre_mask & IPGRE_ATTR_LINK)
		NLA_PUT_U32(msg, IFLA_GRE_LINK, ipgre->link);

	if (ipgre->ipgre_mask & IFLA_GRE_IFLAGS)
		NLA_PUT_U16(msg, IFLA_GRE_IFLAGS, ipgre->iflags);

	if (ipgre->ipgre_mask & IFLA_GRE_OFLAGS)
		NLA_PUT_U16(msg, IFLA_GRE_OFLAGS, ipgre->oflags);

	if (ipgre->ipgre_mask & IPGRE_ATTR_IKEY)
		NLA_PUT_U32(msg, IFLA_GRE_IKEY, ipgre->ikey);

	if (ipgre->ipgre_mask & IPGRE_ATTR_OKEY)
		NLA_PUT_U32(msg, IFLA_GRE_OKEY, ipgre->okey);

	if (ipgre->ipgre_mask & IPGRE_ATTR_LOCAL)
		NLA_PUT_U32(msg, IFLA_GRE_LOCAL, ipgre->local);

	if (ipgre->ipgre_mask & IPGRE_ATTR_REMOTE)
		NLA_PUT_U32(msg, IFLA_GRE_REMOTE, ipgre->remote);

	if (ipgre->ipgre_mask & IPGRE_ATTR_TTL)
		NLA_PUT_U8(msg, IFLA_GRE_TTL, ipgre->ttl);

	if (ipgre->ipgre_mask & IPGRE_ATTR_TOS)
		NLA_PUT_U8(msg, IFLA_GRE_TOS, ipgre->tos);

	if (ipgre->ipgre_mask & IPGRE_ATTR_PMTUDISC)
		NLA_PUT_U8(msg, IFLA_GRE_PMTUDISC, ipgre->pmtudisc);

	nla_nest_end(msg, data);

 nla_put_failure:

	return 0;
}

static void ipgre_free(struct rtnl_link *link)
{
	struct ipgre_info *ipgre = link->l_info;

	free(ipgre);
	link->l_info = NULL;
}

static void ipgre_dump_line(struct rtnl_link *link, struct nl_dump_params *p)
{
	nl_dump(p, "ipgre : %s", link->l_name);
}

static void ipgre_dump_details(struct rtnl_link *link, struct nl_dump_params *p)
{
	struct ipgre_info *ipgre = link->l_info;
	char *name, addr[INET_ADDRSTRLEN];

	if (ipgre->ipgre_mask & IPGRE_ATTR_LINK) {
		nl_dump(p, "      link ");
		name = rtnl_link_get_name(link);
		if (name)
			nl_dump_line(p, "%s\n", name);
		else
			nl_dump_line(p, "%u\n", ipgre->link);
	}

	if (ipgre->ipgre_mask & IPGRE_ATTR_IFLAGS) {
		nl_dump(p, "      iflags ");
		nl_dump_line(p, "%x\n", ipgre->iflags);
	}

	if (ipgre->ipgre_mask & IPGRE_ATTR_OFLAGS) {
		nl_dump(p, "      oflags ");
		nl_dump_line(p, "%x\n", ipgre->oflags);
	}

	if (ipgre->ipgre_mask & IPGRE_ATTR_IKEY) {
		nl_dump(p, "    ikey   ");
		nl_dump_line(p, "%x\n",ipgre->ikey);
	}

	if (ipgre->ipgre_mask & IPGRE_ATTR_OKEY) {
		nl_dump(p, "      okey ");
		nl_dump_line(p, "%x\n", ipgre->okey);
	}

	if (ipgre->ipgre_mask & IPGRE_ATTR_LOCAL) {
		nl_dump(p, "      local ");
		if(inet_ntop(AF_INET, &ipgre->local, addr, sizeof(addr)))
			nl_dump_line(p, "%s\n", addr);
		else
			nl_dump_line(p, "%#x\n", ntohs(ipgre->local));
	}

	if (ipgre->ipgre_mask & IPGRE_ATTR_REMOTE) {
		nl_dump(p, "      remote ");
		if(inet_ntop(AF_INET, &ipgre->remote, addr, sizeof(addr)))
			nl_dump_line(p, "%s\n", addr);
		else
			nl_dump_line(p, "%#x\n", ntohs(ipgre->remote));
	}

	if (ipgre->ipgre_mask & IPGRE_ATTR_TTL) {
		nl_dump(p, "      ttl ");
		nl_dump_line(p, "%u\n", ipgre->ttl);
	}

	if (ipgre->ipgre_mask & IPGRE_ATTR_TOS) {
		nl_dump(p, "      tos ");
		nl_dump_line(p, "%u\n", ipgre->tos);
	}

	if (ipgre->ipgre_mask & IPGRE_ATTR_PMTUDISC) {
		nl_dump(p, "      pmtudisc ");
		nl_dump_line(p, "enabled (%#x)\n", ipgre->pmtudisc);
	}
}

static int ipgre_clone(struct rtnl_link *dst, struct rtnl_link *src)
{
	struct ipgre_info *ipgre_dst, *ipgre_src = src->l_info;
	int err;

	dst->l_info = NULL;

	err = rtnl_link_set_type(dst, "gre");
	if (err < 0)
		return err;

	ipgre_dst = dst->l_info;

	if (!ipgre_dst || !ipgre_src)
		BUG();

	memcpy(ipgre_dst, ipgre_src, sizeof(struct ipgre_info));

	return 0;
}

static struct rtnl_link_info_ops ipgre_info_ops = {
	.io_name                = "gre",
	.io_alloc               = ipgre_alloc,
	.io_parse               = ipgre_parse,
	.io_dump = {
		[NL_DUMP_LINE]  = ipgre_dump_line,
		[NL_DUMP_DETAILS] = ipgre_dump_details,
	},
	.io_clone               = ipgre_clone,
	.io_put_attrs           = ipgre_put_attrs,
	.io_free                = ipgre_free,
};

#define IS_IPGRE_LINK_ASSERT(link)                                          \
        if ((link)->l_info_ops != &ipgre_info_ops) {                        \
                APPBUG("Link is not a ipgre link. set type \"gre\" first.");\
                return -NLE_OPNOTSUPP;                                      \
        }

struct rtnl_link *rtnl_link_ipgre_alloc(void)
{
	struct rtnl_link *link;
	int err;

	link = rtnl_link_alloc();
	if (!link)
		return NULL;

	err = rtnl_link_set_type(link, "gre");
	if (err < 0) {
		rtnl_link_put(link);
		return NULL;
	}

	return link;
}

/**
 * Check if link is a IPGRE link
 * @arg link            Link object
 *
 * @return True if link is a IPGRE link, otherwise 0 is returned.
 */
int rtnl_link_is_ipgre(struct rtnl_link *link)
{
	return link->l_info_ops && !strcmp(link->l_info_ops->io_name, "gre");
}
/**
 * Create a new ipip tunnel device
 * @arg sock            netlink socket
 * @arg name            name of the tunnel deviceL
 *
 * Creates a new ipip tunnel device in the kernel
 * @return 0 on success or a negative error code
 */
int rtnl_link_ipgre_add(struct nl_sock *sk, const char *name)
{
	struct rtnl_link *link;
	int err;

	link = rtnl_link_ipgre_alloc();
	if (!link)
		return -NLE_NOMEM;

	if(name)
		rtnl_link_set_name(link, name);

	err = rtnl_link_add(sk, link, NLM_F_CREATE);
	rtnl_link_put(link);

	return err;
}
/**
 * Set IPGRE tunnel interface index
 * @arg link            Link object
 * @arg index           interface index
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ipgre_set_link(struct rtnl_link *link,  uint32_t index)
{
	struct ipgre_info *ipgre = link->l_info;

	IS_IPGRE_LINK_ASSERT(link);

	ipgre->link = index;
	ipgre->ipgre_mask |= IPGRE_ATTR_LINK;

	return 0;
}

/**
 * Get IPGRE tunnel interface index
 * @arg link            Link object
 *
 * @return interface index
 */
uint32_t rtnl_link_ipgre_get_link(struct rtnl_link *link)
{
	struct ipgre_info *ipgre = link->l_info;

	IS_IPGRE_LINK_ASSERT(link);

	return ipgre->link;
}

/**
 * Set IPGRE tunnel set iflags
 * @arg link            Link object
 * @arg iflags          gre iflags
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ipgre_set_iflags(struct rtnl_link *link, uint16_t iflags)
{
	struct ipgre_info *ipgre = link->l_info;

	IS_IPGRE_LINK_ASSERT(link);

	ipgre->iflags = iflags;
	ipgre->ipgre_mask |= IPGRE_ATTR_IFLAGS;

	return 0;
}

/**
 * Get IPGRE tunnel iflags
 * @arg link            Link object
 *
 * @return iflags
 */
uint16_t rtnl_link_ipgre_get_iflags(struct rtnl_link *link)
{
	struct ipgre_info *ipgre = link->l_info;

	IS_IPGRE_LINK_ASSERT(link);

	return ipgre->iflags;
}

/**
 * Set IPGRE tunnel set oflags
 * @arg link            Link object
 * @arg iflags          gre oflags
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ipgre_set_oflags(struct rtnl_link *link, uint16_t oflags)
{
	struct ipgre_info *ipgre = link->l_info;

	IS_IPGRE_LINK_ASSERT(link);

	ipgre->oflags = oflags;
	ipgre->ipgre_mask |= IPGRE_ATTR_OFLAGS;

	return 0;
}

/**
 * Get IPGRE tunnel oflags
 * @arg link            Link object
 *
 * @return oflags
 */
uint16_t rtnl_link_ipgre_get_oflags(struct rtnl_link *link)
{
	struct ipgre_info *ipgre = link->l_info;

	IS_IPGRE_LINK_ASSERT(link);

	return ipgre->oflags;
}

/**
 * Set IPGRE tunnel set ikey
 * @arg link            Link object
 * @arg ikey            gre ikey
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ipgre_set_ikey(struct rtnl_link *link, uint32_t ikey)
{
	struct ipgre_info *ipgre = link->l_info;

	IS_IPGRE_LINK_ASSERT(link);

	ipgre->ikey = ikey;
	ipgre->ipgre_mask |= IPGRE_ATTR_IKEY;

	return 0;
}

/**
 * Get IPGRE tunnel ikey
 * @arg link            Link object
 *
 * @return ikey
 */
uint32_t rtnl_link_ipgre_get_ikey(struct rtnl_link *link)
{
	struct ipgre_info *ipgre = link->l_info;

	IS_IPGRE_LINK_ASSERT(link);

	return ipgre->ikey;
}

/**
 * Set IPGRE tunnel set okey
 * @arg link            Link object
 * @arg okey            gre okey
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ipgre_set_okey(struct rtnl_link *link, uint32_t okey)
{
	struct ipgre_info *ipgre = link->l_info;

	IS_IPGRE_LINK_ASSERT(link);

	ipgre->okey = okey;
	ipgre->ipgre_mask |= IPGRE_ATTR_OKEY;

	return 0;
}

/**
 * Get IPGRE tunnel okey
 * @arg link            Link object
 *
 * @return okey value
 */
uint32_t rtnl_link_ipgre_get_okey(struct rtnl_link *link)
{
	struct ipgre_info *ipgre = link->l_info;

	IS_IPGRE_LINK_ASSERT(link);

	return ipgre->okey;
}

/**
 * Set IPGRE tunnel local address
 * @arg link            Link object
 * @arg addr            local address
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ipgre_set_local(struct rtnl_link *link, uint32_t addr)
{
	struct ipgre_info *ipgre = link->l_info;

	IS_IPGRE_LINK_ASSERT(link);

	ipgre->local = addr;
	ipgre->ipgre_mask |= IPGRE_ATTR_LOCAL;

	return 0;
}

/**
 * Get IPGRE tunnel local address
 * @arg link            Link object
 *
 * @return local address
 */
uint32_t rtnl_link_ipgre_get_local(struct rtnl_link *link)
{
	struct ipgre_info *ipgre = link->l_info;

	IS_IPGRE_LINK_ASSERT(link);

	return ipgre->local;
}

/**
 * Set IPGRE tunnel remote address
 * @arg link            Link object
 * @arg remote          remote address
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ipgre_set_remote(struct rtnl_link *link, uint32_t remote)
{
	struct ipgre_info *ipgre = link->l_info;

	IS_IPGRE_LINK_ASSERT(link);

	ipgre->remote = remote;
	ipgre->ipgre_mask |= IPGRE_ATTR_REMOTE;

	return 0;
}

/**
 * Get IPGRE tunnel remote address
 * @arg link            Link object
 *
 * @return remote address  on success or a negative error code
 */
uint32_t rtnl_link_ipgre_get_remote(struct rtnl_link *link)
{
	struct ipgre_info *ipgre = link->l_info;

	IS_IPGRE_LINK_ASSERT(link);

	return ipgre->remote;
}

/**
 * Set IPGRE tunnel ttl
 * @arg link            Link object
 * @arg ttl             tunnel ttl
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ipgre_set_ttl(struct rtnl_link *link, uint8_t ttl)
{
	struct ipgre_info *ipgre = link->l_info;

	IS_IPGRE_LINK_ASSERT(link);

	ipgre->ttl = ttl;
	ipgre->ipgre_mask |= IPGRE_ATTR_TTL;

	return 0;
}

/**
 * Set IPGRE tunnel ttl
 * @arg link            Link object
 *
 * @return ttl value
 */
uint8_t rtnl_link_ipgre_get_ttl(struct rtnl_link *link)
{
	struct ipgre_info *ipgre = link->l_info;

	IS_IPGRE_LINK_ASSERT(link);

	return ipgre->ttl;
}

/**
 * Set IPGRE tunnel tos
 * @arg link            Link object
 * @arg tos             tunnel tos
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ipgre_set_tos(struct rtnl_link *link, uint8_t tos)
{
	struct ipgre_info *ipgre = link->l_info;

	IS_IPGRE_LINK_ASSERT(link);

	ipgre->tos = tos;
	ipgre->ipgre_mask |= IPGRE_ATTR_TOS;

	return 0;
}

/**
 * Get IPGRE tunnel tos
 * @arg link            Link object
 *
 * @return tos value
 */
uint8_t rtnl_link_ipgre_get_tos(struct rtnl_link *link)
{
	struct ipgre_info *ipgre = link->l_info;

	IS_IPGRE_LINK_ASSERT(link);

	return ipgre->tos;
}

/**
 * Set IPGRE tunnel path MTU discovery
 * @arg link            Link object
 * @arg pmtudisc        path MTU discovery
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_ipgre_set_pmtudisc(struct rtnl_link *link, uint8_t pmtudisc)
{
	struct ipgre_info *ipgre = link->l_info;

	IS_IPGRE_LINK_ASSERT(link);

	ipgre->pmtudisc = pmtudisc;
	ipgre->ipgre_mask |= IPGRE_ATTR_PMTUDISC;

	return 0;
}

/**
 * Get IPGRE path MTU discovery
 * @arg link            Link object
 *
 * @return pmtudisc value
 */
uint8_t rtnl_link_get_pmtudisc(struct rtnl_link *link)
{
	struct ipgre_info *ipgre = link->l_info;

	IS_IPGRE_LINK_ASSERT(link);

	return ipgre->pmtudisc;
}

static void __init ipgre_init(void)
{
	rtnl_link_register_info(&ipgre_info_ops);
}

static void __exit ipgre_exit(void)
{
	rtnl_link_unregister_info(&ipgre_info_ops);
}

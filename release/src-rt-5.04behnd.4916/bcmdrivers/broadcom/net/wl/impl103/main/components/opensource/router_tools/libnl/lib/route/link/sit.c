/*
 * lib/route/link/sit.c        SIT Link Info
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
 * @defgroup sit SIT
 * sit link module
 *
 * @details
 * \b Link Type Name: "sit"
 *
 * @route_doc{link_sit, SIT Documentation}
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

#define SIT_ATTR_LINK          (1 << 0)
#define SIT_ATTR_LOCAL         (1 << 1)
#define SIT_ATTR_REMOTE        (1 << 2)
#define SIT_ATTR_TTL           (1 << 3)
#define SIT_ATTR_TOS           (1 << 4)
#define SIT_ATTR_PMTUDISC      (1 << 5)
#define SIT_ATTR_FLAGS         (1 << 6)
#define SIT_ATTR_PROTO         (1 << 7)

struct sit_info
{
	uint8_t    ttl;
	uint8_t    tos;
	uint8_t    pmtudisc;
	uint8_t    proto;
	uint16_t   flags;
	uint32_t   link;
	uint32_t   local;
	uint32_t   remote;
	uint32_t   sit_mask;
};

static struct nla_policy sit_policy[IFLA_IPTUN_MAX + 1] = {
	[IFLA_IPTUN_LINK]       = { .type = NLA_U32 },
	[IFLA_IPTUN_LOCAL]      = { .type = NLA_U32 },
	[IFLA_IPTUN_REMOTE]     = { .type = NLA_U32 },
	[IFLA_IPTUN_TTL]        = { .type = NLA_U8 },
	[IFLA_IPTUN_TOS]        = { .type = NLA_U8 },
	[IFLA_IPTUN_PMTUDISC]   = { .type = NLA_U8 },
	[IFLA_IPTUN_FLAGS]      = { .type = NLA_U16 },
	[IFLA_IPTUN_PROTO]      = { .type = NLA_U8 },
};

static int sit_alloc(struct rtnl_link *link)
{
	struct sit_info *sit;

	sit = calloc(1, sizeof(*sit));
	if (!sit)
		return -NLE_NOMEM;

	link->l_info = sit;

	return 0;
}

static int sit_parse(struct rtnl_link *link, struct nlattr *data,
		     struct nlattr *xstats)
{
	struct nlattr *tb[IFLA_IPTUN_MAX + 1];
	struct sit_info *sit;
	int err;

	NL_DBG(3, "Parsing SIT link info");

	err = nla_parse_nested(tb, IFLA_IPTUN_MAX, data, sit_policy);
	if (err < 0)
		goto errout;

	err = sit_alloc(link);
	if (err < 0)
		goto errout;

	sit = link->l_info;

	if (tb[IFLA_IPTUN_LINK]) {
		sit->link = nla_get_u32(tb[IFLA_IPTUN_LINK]);
		sit->sit_mask |= SIT_ATTR_LINK;
	}

	if (tb[IFLA_IPTUN_LOCAL]) {
		sit->local = nla_get_u32(tb[IFLA_IPTUN_LOCAL]);
		sit->sit_mask |= SIT_ATTR_LOCAL;
	}

	if (tb[IFLA_IPTUN_REMOTE]) {
		sit->remote = nla_get_u32(tb[IFLA_IPTUN_REMOTE]);
		sit->sit_mask |= SIT_ATTR_REMOTE;
	}

	if (tb[IFLA_IPTUN_TTL]) {
		sit->ttl = nla_get_u8(tb[IFLA_IPTUN_TTL]);
		sit->sit_mask |= SIT_ATTR_TTL;
	}

	if (tb[IFLA_IPTUN_TOS]) {
		sit->tos = nla_get_u8(tb[IFLA_IPTUN_TOS]);
		sit->sit_mask |= SIT_ATTR_TOS;
	}

	if (tb[IFLA_IPTUN_PMTUDISC]) {
		sit->pmtudisc = nla_get_u8(tb[IFLA_IPTUN_PMTUDISC]);
		sit->sit_mask |= SIT_ATTR_PMTUDISC;
	}

	if (tb[IFLA_IPTUN_FLAGS]) {
		sit->flags = nla_get_u16(tb[IFLA_IPTUN_FLAGS]);
		sit->sit_mask |= SIT_ATTR_FLAGS;
	}

	if (tb[IFLA_IPTUN_PROTO]) {
		sit->proto = nla_get_u8(tb[IFLA_IPTUN_PROTO]);
		sit->sit_mask |= SIT_ATTR_PROTO;
	}

	err = 0;

 errout:
	return err;
}

static int sit_put_attrs(struct nl_msg *msg, struct rtnl_link *link)
{
	struct sit_info *sit = link->l_info;
	struct nlattr *data;

	data = nla_nest_start(msg, IFLA_INFO_DATA);
	if (!data)
		return -NLE_MSGSIZE;

	if (sit->sit_mask & SIT_ATTR_LINK)
		NLA_PUT_U32(msg, IFLA_IPTUN_LINK, sit->link);

	if (sit->sit_mask & SIT_ATTR_LOCAL)
		NLA_PUT_U32(msg, IFLA_IPTUN_LOCAL, sit->local);

	if (sit->sit_mask & SIT_ATTR_REMOTE)
		NLA_PUT_U32(msg, IFLA_IPTUN_REMOTE, sit->remote);

	if (sit->sit_mask & SIT_ATTR_TTL)
		NLA_PUT_U8(msg, IFLA_IPTUN_TTL, sit->ttl);

	if (sit->sit_mask & SIT_ATTR_TOS)
		NLA_PUT_U8(msg, IFLA_IPTUN_TOS, sit->tos);

	if (sit->sit_mask & SIT_ATTR_PMTUDISC)
		NLA_PUT_U8(msg, IFLA_IPTUN_PMTUDISC, sit->pmtudisc);

	if (sit->sit_mask & SIT_ATTR_FLAGS)
		NLA_PUT_U16(msg, IFLA_IPTUN_FLAGS, sit->flags);

	if (sit->sit_mask & SIT_ATTR_PROTO)
		NLA_PUT_U8(msg, IFLA_IPTUN_PROTO, sit->proto);

	nla_nest_end(msg, data);

nla_put_failure:

	return 0;
}

static void sit_free(struct rtnl_link *link)
{
	struct sit_info *sit = link->l_info;

	free(sit);
	link->l_info = NULL;
}

static void sit_dump_line(struct rtnl_link *link, struct nl_dump_params *p)
{
	nl_dump(p, "sit : %s", link->l_name);
}

static void sit_dump_details(struct rtnl_link *link, struct nl_dump_params *p)
{
	struct sit_info *sit = link->l_info;
	char *name, addr[INET_ADDRSTRLEN];

	if (sit->sit_mask & SIT_ATTR_LINK) {
		nl_dump(p, "      link ");
		name = rtnl_link_get_name(link);
		if (name)
			nl_dump_line(p, "%s\n", name);
		else
			nl_dump_line(p, "%u\n", sit->link);
	}

	if (sit->sit_mask & SIT_ATTR_LOCAL) {
		nl_dump(p, "      local ");
		if(inet_ntop(AF_INET, &sit->local, addr, sizeof(addr)))
			nl_dump_line(p, "%s\n", addr);
		else
			nl_dump_line(p, "%#x\n", ntohs(sit->local));
	}

	if (sit->sit_mask & SIT_ATTR_REMOTE) {
		nl_dump(p, "      remote ");
		if(inet_ntop(AF_INET, &sit->remote, addr, sizeof(addr)))
			nl_dump_line(p, "%s\n", addr);
		else
			nl_dump_line(p, "%#x\n", ntohs(sit->remote));
	}

	if (sit->sit_mask & SIT_ATTR_TTL) {
		nl_dump(p, "      ttl ");
		nl_dump_line(p, "%u\n", sit->ttl);
	}

	if (sit->sit_mask & SIT_ATTR_TOS) {
		nl_dump(p, "      tos ");
		nl_dump_line(p, "%u\n", sit->tos);
	}

	if (sit->sit_mask & SIT_ATTR_FLAGS) {
		nl_dump(p, "      flags ");
		nl_dump_line(p, " (%x)\n", sit->flags);
	}

	if (sit->sit_mask & SIT_ATTR_PROTO) {
		nl_dump(p, "    proto   ");
		nl_dump_line(p, " (%x)\n", sit->proto);
	}
}

static int sit_clone(struct rtnl_link *dst, struct rtnl_link *src)
{
	struct sit_info *sit_dst, *sit_src = src->l_info;
	int err;

	dst->l_info = NULL;

	err = rtnl_link_set_type(dst, "sit");
	if (err < 0)
		return err;

	sit_dst = dst->l_info;

	if (!sit_dst || !sit_src)
		return -NLE_NOMEM;

	memcpy(sit_dst, sit_src, sizeof(struct sit_info));

	return 0;
}

static struct rtnl_link_info_ops sit_info_ops = {
	.io_name                = "sit",
	.io_alloc               = sit_alloc,
	.io_parse               = sit_parse,
	.io_dump = {
		[NL_DUMP_LINE]  = sit_dump_line,
		[NL_DUMP_DETAILS] = sit_dump_details,
	},
	.io_clone               = sit_clone,
	.io_put_attrs           = sit_put_attrs,
	.io_free                = sit_free,
};

#define IS_SIT_LINK_ASSERT(link)                                           \
        if ((link)->l_info_ops != &sit_info_ops) {                         \
                APPBUG("Link is not a sit link. set type \"sit\" first."); \
                return -NLE_OPNOTSUPP;                                     \
        }

struct rtnl_link *rtnl_link_sit_alloc(void)
{
	struct rtnl_link *link;
	int err;

	link = rtnl_link_alloc();
	if (!link)
		return NULL;

	err = rtnl_link_set_type(link, "sit");
	if (err < 0) {
		rtnl_link_put(link);
		return NULL;
	}

	return link;
}

/**
 * Check if link is a SIT link
 * @arg link            Link object
 *
 * @return True if link is a SIT link, otherwise false is returned.
 */
int rtnl_link_is_sit(struct rtnl_link *link)
{
	return link->l_info_ops && !strcmp(link->l_info_ops->io_name, "sit");
}

/**
 * Create a new sit tunnel device
 * @arg sock            netlink socket
 * @arg name            name of the tunnel device
 *
 * Creates a new sit tunnel device in the kernel
 * @return 0 on success or a negative error code
 */
int rtnl_link_sit_add(struct nl_sock *sk, const char *name)
{
	struct rtnl_link *link;
	int err;

	link = rtnl_link_sit_alloc();
	if (!link)
		return -NLE_NOMEM;

	if(name)
		rtnl_link_set_name(link, name);

	err = rtnl_link_add(sk, link, NLM_F_CREATE);
	rtnl_link_put(link);

	return err;
}

/**
 * Set SIT tunnel interface index
 * @arg link            Link object
 * @arg index           interface index
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_sit_set_link(struct rtnl_link *link,  uint32_t index)
{
	struct sit_info *sit = link->l_info;

	IS_SIT_LINK_ASSERT(link);

	sit->link = index;
	sit->sit_mask |= SIT_ATTR_LINK;

	return 0;
}

/**
 * Get SIT tunnel interface index
 * @arg link            Link object
 *
 * @return interface index value
 */
uint32_t rtnl_link_sit_get_link(struct rtnl_link *link)
{
	struct sit_info *sit = link->l_info;

	IS_SIT_LINK_ASSERT(link);

	return sit->link;
}

/**
 * Set SIT tunnel local address
 * @arg link            Link object
 * @arg addr            local address
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_sit_set_local(struct rtnl_link *link, uint32_t addr)
{
	struct sit_info *sit = link->l_info;

	IS_SIT_LINK_ASSERT(link);

	sit->local = addr;
	sit->sit_mask |= SIT_ATTR_LOCAL;

	return 0;
}

/**
 * Get SIT tunnel local address
 * @arg link            Link object
 *
 * @return local address value
 */
uint32_t rtnl_link_sit_get_local(struct rtnl_link *link)
{
	struct sit_info *sit = link->l_info;

	IS_SIT_LINK_ASSERT(link);

	return sit->local;
}

/**
 * Set SIT tunnel remote address
 * @arg link            Link object
 * @arg remote          remote address
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_sit_set_remote(struct rtnl_link *link, uint32_t addr)
{
	struct sit_info *sit = link->l_info;

	IS_SIT_LINK_ASSERT(link);

	sit->remote = addr;
	sit->sit_mask |= SIT_ATTR_REMOTE;

	return 0;
}

/**
 * Get SIT tunnel remote address
 * @arg link            Link object
 *
 * @return remote address
 */
uint32_t rtnl_link_sit_get_remote(struct rtnl_link *link)
{
	struct sit_info *sit = link->l_info;

	IS_SIT_LINK_ASSERT(link);

	return sit->remote;
}

/**
 * Set SIT tunnel ttl
 * @arg link            Link object
 * @arg ttl             tunnel ttl
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_sit_set_ttl(struct rtnl_link *link, uint8_t ttl)
{
	struct sit_info *sit = link->l_info;

	IS_SIT_LINK_ASSERT(link);

	sit->ttl = ttl;
	sit->sit_mask |= SIT_ATTR_TTL;

	return 0;
}

/**
 * Get SIT tunnel ttl
 * @arg link            Link object
 *
 * @return ttl value
 */
uint8_t rtnl_link_sit_get_ttl(struct rtnl_link *link)
{
	struct sit_info *sit = link->l_info;

	IS_SIT_LINK_ASSERT(link);

	return sit->ttl;
}

/**
 * Set SIT tunnel tos
 * @arg link            Link object
 * @arg tos             tunnel tos
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_sit_set_tos(struct rtnl_link *link, uint8_t tos)
{
	struct sit_info *sit = link->l_info;

	IS_SIT_LINK_ASSERT(link);

	sit->tos = tos;
	sit->sit_mask |= SIT_ATTR_TOS;

	return 0;
}

/**
 * Get SIT tunnel tos
 * @arg link            Link object
 *
 * @return tos value
 */
uint8_t rtnl_link_sit_get_tos(struct rtnl_link *link)
{
	struct sit_info *sit = link->l_info;

	IS_SIT_LINK_ASSERT(link);

	return sit->tos;
}

/**
 * Set SIT tunnel path MTU discovery
 * @arg link            Link object
 * @arg pmtudisc        path MTU discovery
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_sit_set_pmtudisc(struct rtnl_link *link, uint8_t pmtudisc)
{
	struct sit_info *sit = link->l_info;

	IS_SIT_LINK_ASSERT(link);

	sit->pmtudisc = pmtudisc;
	sit->sit_mask |= SIT_ATTR_PMTUDISC;

	return 0;
}

/**
 * Get SIT path MTU discovery
 * @arg link            Link object
 *
 * @return pmtudisc value
 */
uint8_t rtnl_link_sit_get_pmtudisc(struct rtnl_link *link)
{
	struct sit_info *sit = link->l_info;

	IS_SIT_LINK_ASSERT(link);

	return sit->pmtudisc;
}

/**
 * Set SIT tunnel flags
 * @arg link            Link object
 * @arg flags           tunnel flags
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_sit_set_flags(struct rtnl_link *link, uint16_t flags)
{
	struct sit_info *sit = link->l_info;

	IS_SIT_LINK_ASSERT(link);

	sit->flags = flags;
	sit->sit_mask |= SIT_ATTR_FLAGS;

	return 0;
}

/**
 * Get SIT path flags
 * @arg link            Link object
 *
 * @return flags value
 */
uint16_t rtnl_link_sit_get_flags(struct rtnl_link *link)
{
	struct sit_info *sit = link->l_info;

	IS_SIT_LINK_ASSERT(link);

	return sit->flags;
}

/**
 * Set SIT tunnel proto
 * @arg link            Link object
 * @arg proto           tunnel proto
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_sit_set_proto(struct rtnl_link *link, uint8_t proto)
{
	struct sit_info *sit = link->l_info;

	IS_SIT_LINK_ASSERT(link);

	sit->proto = proto;
	sit->sit_mask |= SIT_ATTR_PROTO;

	return 0;
}

/**
 * Get SIT proto
 * @arg link            Link object
 *
 * @return proto value
 */
uint8_t rtnl_link_sit_get_proto(struct rtnl_link *link)
{
	struct sit_info *sit = link->l_info;

	IS_SIT_LINK_ASSERT(link);

	return sit->proto;
}

static void __init sit_init(void)
{
	rtnl_link_register_info(&sit_info_ops);
}

static void __exit sit_exit(void)
{
	rtnl_link_unregister_info(&sit_info_ops);
}

/*
 * lib/route/addr.c		Addresses
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2012 Thomas Graf <tgraf@suug.ch>
 * Copyright (c) 2003-2006 Baruch Even <baruch@ev-en.org>,
 *                         Mediatrix Telecom, inc. <ericb@mediatrix.com>
 */

/**
 * @ingroup rtnl
 * @defgroup rtaddr Addresses
 * @brief
 *
 * @note The maximum size of an address label is IFNAMSIZ.
 *
 * @note The address may not contain a prefix length if the peer address
 *       has been specified already.
 *
 * @par 1) Address Addition
 * @code
 * // Allocate an empty address object to be filled out with the attributes
 * // of the new address.
 * struct rtnl_addr *addr = rtnl_addr_alloc();
 *
 * // Fill out the mandatory attributes of the new address. Setting the
 * // local address will automatically set the address family and the
 * // prefix length to the correct values.
 * rtnl_addr_set_ifindex(addr, ifindex);
 * rtnl_addr_set_local(addr, local_addr);
 *
 * // The label of the address can be specified, currently only supported
 * // by IPv4 and DECnet.
 * rtnl_addr_set_label(addr, "mylabel");
 *
 * // The peer address can be specified if necessary, in either case a peer
 * // address will be sent to the kernel in order to fullfil the interface
 * // requirements. If none is set, it will equal the local address.
 * // Note: Real peer addresses are only supported by IPv4 for now.
 * rtnl_addr_set_peer(addr, peer_addr);
 *
 * // In case you want to have the address have a scope other than global
 * // it may be overwritten using rtnl_addr_set_scope(). The scope currently
 * // cannot be set for IPv6 addresses.
 * rtnl_addr_set_scope(addr, rtnl_str2scope("site"));
 *
 * // Broadcast address may be specified using the relevant
 * // functions, the address family will be verified if one of the other
 * // addresses has been set already. Currently only works for IPv4.
 * rtnl_addr_set_broadcast(addr, broadcast_addr);
 *
 * // Build the netlink message and send it to the kernel, the operation will
 * // block until the operation has been completed. Alternatively the required
 * // netlink message can be built using rtnl_addr_build_add_request() to be
 * // sent out using nl_send_auto_complete().
 * rtnl_addr_add(sk, addr, 0);
 *
 * // Free the memory
 * rtnl_addr_put(addr);
 * @endcode
 *
 * @par 2) Address Deletion
 * @code
 * // Allocate an empty address object to be filled out with the attributes
 * // matching the address to be deleted. Alternatively a fully equipped
 * // address object out of a cache can be used instead.
 * struct rtnl_addr *addr = rtnl_addr_alloc();
 *
 * // The only mandatory parameter besides the address family is the interface
 * // index the address is on, i.e. leaving out all other parameters will
 * // result in all addresses of the specified address family interface tuple
 * // to be deleted.
 * rtnl_addr_set_ifindex(addr, ifindex);
 *
 * // Specyfing the address family manually is only required if neither the
 * // local nor peer address have been specified.
 * rtnl_addr_set_family(addr, AF_INET);
 *
 * // Specyfing the local address is optional but the best choice to delete
 * // specific addresses.
 * rtnl_addr_set_local(addr, local_addr);
 *
 * // The label of the address can be specified, currently only supported
 * // by IPv4 and DECnet.
 * rtnl_addr_set_label(addr, "mylabel");
 *
 * // The peer address can be specified if necessary, in either case a peer
 * // address will be sent to the kernel in order to fullfil the interface
 * // requirements. If none is set, it will equal the local address.
 * // Note: Real peer addresses are only supported by IPv4 for now.
 * rtnl_addr_set_peer(addr, peer_addr);
 *
 * // Build the netlink message and send it to the kernel, the operation will
 * // block until the operation has been completed. Alternatively the required
 * // netlink message can be built using rtnl_addr_build_delete_request()
 * // to be sent out using nl_send_auto_complete().
 * rtnl_addr_delete(sk, addr, 0);
 *
 * // Free the memory
 * rtnl_addr_put(addr);
 * @endcode
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink/netlink.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/addr.h>
#include <netlink/route/route.h>
#include <netlink/route/link.h>
#include <netlink/utils.h>

/** @cond SKIP */
#define ADDR_ATTR_FAMILY	0x0001
#define ADDR_ATTR_PREFIXLEN	0x0002
#define ADDR_ATTR_FLAGS		0x0004
#define ADDR_ATTR_SCOPE		0x0008
#define ADDR_ATTR_IFINDEX	0x0010
#define ADDR_ATTR_LABEL		0x0020
#define ADDR_ATTR_CACHEINFO	0x0040
#define ADDR_ATTR_PEER		0x0080
#define ADDR_ATTR_LOCAL		0x0100
#define ADDR_ATTR_BROADCAST	0x0200
#define ADDR_ATTR_MULTICAST	0x0400
#define ADDR_ATTR_ANYCAST	0x0800

static struct nl_cache_ops rtnl_addr_ops;
static struct nl_object_ops addr_obj_ops;
/** @endcond */

static void addr_constructor(struct nl_object *obj)
{
	struct rtnl_addr *addr = nl_object_priv(obj);

	addr->a_scope = RT_SCOPE_NOWHERE;
}

static void addr_free_data(struct nl_object *obj)
{
	struct rtnl_addr *addr = nl_object_priv(obj);

	if (!addr)
		return;

	nl_addr_put(addr->a_peer);
	nl_addr_put(addr->a_local);
	nl_addr_put(addr->a_bcast);
	nl_addr_put(addr->a_multicast);
	nl_addr_put(addr->a_anycast);
	rtnl_link_put(addr->a_link);
}

static int addr_clone(struct nl_object *_dst, struct nl_object *_src)
{
	struct rtnl_addr *dst = nl_object_priv(_dst);
	struct rtnl_addr *src = nl_object_priv(_src);

	if (src->a_link) {
		nl_object_get(OBJ_CAST(src->a_link));
		dst->a_link = src->a_link;
	}

	if (src->a_peer)
		if (!(dst->a_peer = nl_addr_clone(src->a_peer)))
			return -NLE_NOMEM;
	
	if (src->a_local)
		if (!(dst->a_local = nl_addr_clone(src->a_local)))
			return -NLE_NOMEM;

	if (src->a_bcast)
		if (!(dst->a_bcast = nl_addr_clone(src->a_bcast)))
			return -NLE_NOMEM;

	if (src->a_multicast)
		if (!(dst->a_multicast = nl_addr_clone(src->a_multicast)))
			return -NLE_NOMEM;

	if (src->a_anycast)
		if (!(dst->a_anycast = nl_addr_clone(src->a_anycast)))
			return -NLE_NOMEM;

	return 0;
}

static struct nla_policy addr_policy[IFA_MAX+1] = {
	[IFA_LABEL]	= { .type = NLA_STRING,
			    .maxlen = IFNAMSIZ },
	[IFA_CACHEINFO]	= { .minlen = sizeof(struct ifa_cacheinfo) },
};

static int addr_msg_parser(struct nl_cache_ops *ops, struct sockaddr_nl *who,
			   struct nlmsghdr *nlh, struct nl_parser_param *pp)
{
	struct rtnl_addr *addr;
	struct ifaddrmsg *ifa;
	struct nlattr *tb[IFA_MAX+1];
	int err, family;
	struct nl_cache *link_cache;
	struct nl_addr *plen_addr = NULL;

	addr = rtnl_addr_alloc();
	if (!addr)
		return -NLE_NOMEM;

	addr->ce_msgtype = nlh->nlmsg_type;

	err = nlmsg_parse(nlh, sizeof(*ifa), tb, IFA_MAX, addr_policy);
	if (err < 0)
		goto errout;

	ifa = nlmsg_data(nlh);
	addr->a_family = family = ifa->ifa_family;
	addr->a_prefixlen = ifa->ifa_prefixlen;
	addr->a_scope = ifa->ifa_scope;
	addr->a_flags = tb[IFA_FLAGS] ? nla_get_u32(tb[IFA_FLAGS]) :
					ifa->ifa_flags;
	addr->a_ifindex = ifa->ifa_index;

	addr->ce_mask = (ADDR_ATTR_FAMILY | ADDR_ATTR_PREFIXLEN |
			 ADDR_ATTR_FLAGS | ADDR_ATTR_SCOPE | ADDR_ATTR_IFINDEX);

	if (tb[IFA_LABEL]) {
		nla_strlcpy(addr->a_label, tb[IFA_LABEL], IFNAMSIZ);
		addr->ce_mask |= ADDR_ATTR_LABEL;
	}

	/* IPv6 only */
	if (tb[IFA_CACHEINFO]) {
		struct ifa_cacheinfo *ca;
		
		ca = nla_data(tb[IFA_CACHEINFO]);
		addr->a_cacheinfo.aci_prefered = ca->ifa_prefered;
		addr->a_cacheinfo.aci_valid = ca->ifa_valid;
		addr->a_cacheinfo.aci_cstamp = ca->cstamp;
		addr->a_cacheinfo.aci_tstamp = ca->tstamp;
		addr->ce_mask |= ADDR_ATTR_CACHEINFO;
	}

	if (tb[IFA_LOCAL]) {
		addr->a_local = nl_addr_alloc_attr(tb[IFA_LOCAL], family);
		if (!addr->a_local)
			goto errout_nomem;
		addr->ce_mask |= ADDR_ATTR_LOCAL;
		plen_addr = addr->a_local;
	}

	if (tb[IFA_ADDRESS]) {
		struct nl_addr *a;

		a = nl_addr_alloc_attr(tb[IFA_ADDRESS], family);
		if (!a)
			goto errout_nomem;

		/* IPv6 sends the local address as IFA_ADDRESS with
		 * no IFA_LOCAL, IPv4 sends both IFA_LOCAL and IFA_ADDRESS
		 * with IFA_ADDRESS being the peer address if they differ */
		if (!tb[IFA_LOCAL] || !nl_addr_cmp(a, addr->a_local)) {
			nl_addr_put(addr->a_local);
			addr->a_local = a;
			addr->ce_mask |= ADDR_ATTR_LOCAL;
		} else {
			addr->a_peer = a;
			addr->ce_mask |= ADDR_ATTR_PEER;
		}

		plen_addr = a;
	}

	if (plen_addr)
		nl_addr_set_prefixlen(plen_addr, addr->a_prefixlen);

	/* IPv4 only */
	if (tb[IFA_BROADCAST]) {
		addr->a_bcast = nl_addr_alloc_attr(tb[IFA_BROADCAST], family);
		if (!addr->a_bcast)
			goto errout_nomem;

		addr->ce_mask |= ADDR_ATTR_BROADCAST;
	}

	/* IPv6 only */
	if (tb[IFA_MULTICAST]) {
		addr->a_multicast = nl_addr_alloc_attr(tb[IFA_MULTICAST],
						       family);
		if (!addr->a_multicast)
			goto errout_nomem;

		addr->ce_mask |= ADDR_ATTR_MULTICAST;
	}

	/* IPv6 only */
	if (tb[IFA_ANYCAST]) {
		addr->a_anycast = nl_addr_alloc_attr(tb[IFA_ANYCAST],
						       family);
		if (!addr->a_anycast)
			goto errout_nomem;

		addr->ce_mask |= ADDR_ATTR_ANYCAST;
	}

	if ((link_cache = __nl_cache_mngt_require("route/link"))) {
		struct rtnl_link *link;

		if ((link = rtnl_link_get(link_cache, addr->a_ifindex))) {
			rtnl_addr_set_link(addr, link);

			/* rtnl_addr_set_link incs refcnt */
			rtnl_link_put(link);
		}
	}

	err = pp->pp_cb((struct nl_object *) addr, pp);
errout:
	rtnl_addr_put(addr);

	return err;

errout_nomem:
	err = -NLE_NOMEM;
	goto errout;
}

static int addr_request_update(struct nl_cache *cache, struct nl_sock *sk)
{
	return nl_rtgen_request(sk, RTM_GETADDR, AF_UNSPEC, NLM_F_DUMP);
}

static void addr_dump_line(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_addr *addr = (struct rtnl_addr *) obj;
	struct nl_cache *link_cache;
	char buf[128];

	link_cache = nl_cache_mngt_require_safe("route/link");

	if (addr->ce_mask & ADDR_ATTR_LOCAL)
		nl_dump_line(p, "%s",
			nl_addr2str(addr->a_local, buf, sizeof(buf)));
	else
		nl_dump_line(p, "none");

	if (addr->ce_mask & ADDR_ATTR_PEER)
		nl_dump(p, " peer %s",
			nl_addr2str(addr->a_peer, buf, sizeof(buf)));

	nl_dump(p, " %s ", nl_af2str(addr->a_family, buf, sizeof(buf)));

	if (link_cache)
		nl_dump(p, "dev %s ",
			rtnl_link_i2name(link_cache, addr->a_ifindex,
					 buf, sizeof(buf)));
	else
		nl_dump(p, "dev %d ", addr->a_ifindex);

	nl_dump(p, "scope %s",
		rtnl_scope2str(addr->a_scope, buf, sizeof(buf)));

	rtnl_addr_flags2str(addr->a_flags, buf, sizeof(buf));
	if (buf[0])
		nl_dump(p, " <%s>", buf);

	nl_dump(p, "\n");

	if (link_cache)
		nl_cache_put(link_cache);
}

static void addr_dump_details(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_addr *addr = (struct rtnl_addr *) obj;
	char buf[128];

	addr_dump_line(obj, p);

	if (addr->ce_mask & (ADDR_ATTR_LABEL | ADDR_ATTR_BROADCAST |
			     ADDR_ATTR_MULTICAST)) {
		nl_dump_line(p, "  ");

		if (addr->ce_mask & ADDR_ATTR_LABEL)
			nl_dump(p, " label %s", addr->a_label);

		if (addr->ce_mask & ADDR_ATTR_BROADCAST)
			nl_dump(p, " broadcast %s",
				nl_addr2str(addr->a_bcast, buf, sizeof(buf)));

		if (addr->ce_mask & ADDR_ATTR_MULTICAST)
			nl_dump(p, " multicast %s",
				nl_addr2str(addr->a_multicast, buf,
					      sizeof(buf)));

		if (addr->ce_mask & ADDR_ATTR_ANYCAST)
			nl_dump(p, " anycast %s",
				nl_addr2str(addr->a_anycast, buf,
					      sizeof(buf)));

		nl_dump(p, "\n");
	}

	if (addr->ce_mask & ADDR_ATTR_CACHEINFO) {
		struct rtnl_addr_cacheinfo *ci = &addr->a_cacheinfo;

		nl_dump_line(p, "   valid-lifetime %s",
			     ci->aci_valid == 0xFFFFFFFFU ? "forever" :
			     nl_msec2str(ci->aci_valid * 1000,
					   buf, sizeof(buf)));

		nl_dump(p, " preferred-lifetime %s\n",
			ci->aci_prefered == 0xFFFFFFFFU ? "forever" :
			nl_msec2str(ci->aci_prefered * 1000,
				      buf, sizeof(buf)));

		nl_dump_line(p, "   created boot-time+%s ",
			     nl_msec2str(addr->a_cacheinfo.aci_cstamp * 10,
					   buf, sizeof(buf)));
		    
		nl_dump(p, "last-updated boot-time+%s\n",
			nl_msec2str(addr->a_cacheinfo.aci_tstamp * 10,
				      buf, sizeof(buf)));
	}
}

static void addr_dump_stats(struct nl_object *obj, struct nl_dump_params *p)
{
	addr_dump_details(obj, p);
}

static int addr_compare(struct nl_object *_a, struct nl_object *_b,
			uint32_t attrs, int flags)
{
	struct rtnl_addr *a = (struct rtnl_addr *) _a;
	struct rtnl_addr *b = (struct rtnl_addr *) _b;
	int diff = 0;

#define ADDR_DIFF(ATTR, EXPR) ATTR_DIFF(attrs, ADDR_ATTR_##ATTR, a, b, EXPR)

	diff |= ADDR_DIFF(IFINDEX,	a->a_ifindex != b->a_ifindex);
	diff |= ADDR_DIFF(FAMILY,	a->a_family != b->a_family);
	diff |= ADDR_DIFF(SCOPE,	a->a_scope != b->a_scope);
	diff |= ADDR_DIFF(LABEL,	strcmp(a->a_label, b->a_label));
	diff |= ADDR_DIFF(PEER,		nl_addr_cmp(a->a_peer, b->a_peer));
	diff |= ADDR_DIFF(LOCAL,	nl_addr_cmp(a->a_local, b->a_local));
	diff |= ADDR_DIFF(MULTICAST,	nl_addr_cmp(a->a_multicast,
						    b->a_multicast));
	diff |= ADDR_DIFF(BROADCAST,	nl_addr_cmp(a->a_bcast, b->a_bcast));
	diff |= ADDR_DIFF(ANYCAST,	nl_addr_cmp(a->a_anycast, b->a_anycast));

	if (flags & LOOSE_COMPARISON)
		diff |= ADDR_DIFF(FLAGS,
				  (a->a_flags ^ b->a_flags) & b->a_flag_mask);
	else
		diff |= ADDR_DIFF(FLAGS, a->a_flags != b->a_flags);

#undef ADDR_DIFF

	return diff;
}

static const struct trans_tbl addr_attrs[] = {
	__ADD(ADDR_ATTR_FAMILY, family)
	__ADD(ADDR_ATTR_PREFIXLEN, prefixlen)
	__ADD(ADDR_ATTR_FLAGS, flags)
	__ADD(ADDR_ATTR_SCOPE, scope)
	__ADD(ADDR_ATTR_IFINDEX, ifindex)
	__ADD(ADDR_ATTR_LABEL, label)
	__ADD(ADDR_ATTR_CACHEINFO, cacheinfo)
	__ADD(ADDR_ATTR_PEER, peer)
	__ADD(ADDR_ATTR_LOCAL, local)
	__ADD(ADDR_ATTR_BROADCAST, broadcast)
	__ADD(ADDR_ATTR_MULTICAST, multicast)
};

static char *addr_attrs2str(int attrs, char *buf, size_t len)
{
	return __flags2str(attrs, buf, len, addr_attrs,
			   ARRAY_SIZE(addr_attrs));
}

/**
 * @name Allocation/Freeing
 * @{
 */

struct rtnl_addr *rtnl_addr_alloc(void)
{
	return (struct rtnl_addr *) nl_object_alloc(&addr_obj_ops);
}

void rtnl_addr_put(struct rtnl_addr *addr)
{
	nl_object_put((struct nl_object *) addr);
}

/** @} */

/**
 * @name Cache Management
 * @{
 */

int rtnl_addr_alloc_cache(struct nl_sock *sk, struct nl_cache **result)
{
	return nl_cache_alloc_and_fill(&rtnl_addr_ops, sk, result);
}

/**
 * Search address in cache
 * @arg cache		Address cache
 * @arg ifindex		Interface index of address
 * @arg addr		Local address part
 *
 * Searches address cache previously allocated with rtnl_addr_alloc_cache()
 * for an address with a matching local address.
 *
 * The reference counter is incremented before returning the address, therefore
 * the reference must be given back with rtnl_addr_put() after usage.
 *
 * @return Address object or NULL if no match was found.
 */
struct rtnl_addr *rtnl_addr_get(struct nl_cache *cache, int ifindex,
				struct nl_addr *addr)
{
	struct rtnl_addr *a;

	if (cache->c_ops != &rtnl_addr_ops)
		return NULL;

	nl_list_for_each_entry(a, &cache->c_items, ce_list) {
		if (ifindex && a->a_ifindex != ifindex)
			continue;

		if (a->ce_mask & ADDR_ATTR_LOCAL &&
		    !nl_addr_cmp(a->a_local, addr)) {
			nl_object_get((struct nl_object *) a);
			return a;
		}
	}

	return NULL;
}

/** @} */

static int build_addr_msg(struct rtnl_addr *tmpl, int cmd, int flags,
			  struct nl_msg **result)
{
	struct nl_msg *msg;
	struct ifaddrmsg am = {
		.ifa_family = tmpl->a_family,
		.ifa_index = tmpl->a_ifindex,
		.ifa_prefixlen = tmpl->a_prefixlen,
		.ifa_flags = tmpl->a_flags,
	};

	if (tmpl->ce_mask & ADDR_ATTR_SCOPE)
		am.ifa_scope = tmpl->a_scope;
	else {
		/* compatibility hack */
		if (tmpl->a_family == AF_INET &&
		    tmpl->ce_mask & ADDR_ATTR_LOCAL &&
		    *((char *) nl_addr_get_binary_addr(tmpl->a_local)) == 127)
			am.ifa_scope = RT_SCOPE_HOST;
		else
			am.ifa_scope = RT_SCOPE_UNIVERSE;
	}

	msg = nlmsg_alloc_simple(cmd, flags);
	if (!msg)
		return -NLE_NOMEM;

	if (nlmsg_append(msg, &am, sizeof(am), NLMSG_ALIGNTO) < 0)
		goto nla_put_failure;

	if (tmpl->ce_mask & ADDR_ATTR_LOCAL)
		NLA_PUT_ADDR(msg, IFA_LOCAL, tmpl->a_local);

	if (tmpl->ce_mask & ADDR_ATTR_PEER)
		NLA_PUT_ADDR(msg, IFA_ADDRESS, tmpl->a_peer);
	else if (tmpl->ce_mask & ADDR_ATTR_LOCAL)
		NLA_PUT_ADDR(msg, IFA_ADDRESS, tmpl->a_local);

	if (tmpl->ce_mask & ADDR_ATTR_LABEL)
		NLA_PUT_STRING(msg, IFA_LABEL, tmpl->a_label);

	if (tmpl->ce_mask & ADDR_ATTR_BROADCAST)
		NLA_PUT_ADDR(msg, IFA_BROADCAST, tmpl->a_bcast);

	if (tmpl->ce_mask & ADDR_ATTR_CACHEINFO) {
		struct ifa_cacheinfo ca = {
			.ifa_valid = tmpl->a_cacheinfo.aci_valid,
			.ifa_prefered = tmpl->a_cacheinfo.aci_prefered,
		};

		NLA_PUT(msg, IFA_CACHEINFO, sizeof(ca), &ca);
	}

	if (tmpl->a_flags & ~0xFF) {
		/* only set the IFA_FLAGS attribute, if they actually contain additional
		 * flags that are not already set to am.ifa_flags.
		 *
		 * Older kernels refuse RTM_NEWADDR and RTM_NEWROUTE messages with EINVAL
		 * if they contain unknown netlink attributes. See net/core/rtnetlink.c, which
		 * was fixed by kernel commit 661d2967b3f1b34eeaa7e212e7b9bbe8ee072b59.
		 *
		 * With this workaround, libnl will function correctly with older kernels,
		 * unless there is a new libnl user that wants to set these flags. In this
		 * case it's up to the user to workaround this issue. */
		NLA_PUT_U32(msg, IFA_FLAGS, tmpl->a_flags);
	}

	*result = msg;
	return 0;

nla_put_failure:
	nlmsg_free(msg);
	return -NLE_MSGSIZE;
}

/**
 * @name Addition
 * @{
 */

/**
 * Build netlink request message to request addition of new address
 * @arg addr		Address object representing the new address.
 * @arg flags		Additional netlink message flags.
 * @arg result		Pointer to store resulting message.
 *
 * Builds a new netlink message requesting the addition of a new
 * address. The netlink message header isn't fully equipped with
 * all relevant fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed.
 *
 * Minimal required attributes:
 *   - interface index (rtnl_addr_set_ifindex())
 *   - local address (rtnl_addr_set_local())
 *
 * The scope will default to universe except for loopback addresses in
 * which case a host scope is used if not specified otherwise.
 *
 * @note Free the memory after usage using nlmsg_free().
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_addr_build_add_request(struct rtnl_addr *addr, int flags,
				struct nl_msg **result)
{
	uint32_t required = ADDR_ATTR_IFINDEX | ADDR_ATTR_FAMILY |
		       ADDR_ATTR_PREFIXLEN | ADDR_ATTR_LOCAL;

	if ((addr->ce_mask & required) != required)
		return -NLE_MISSING_ATTR;
	
	return build_addr_msg(addr, RTM_NEWADDR, NLM_F_CREATE | flags, result);
}

/**
 * Request addition of new address
 * @arg sk		Netlink socket.
 * @arg addr		Address object representing the new address.
 * @arg flags		Additional netlink message flags.
 *
 * Builds a netlink message by calling rtnl_addr_build_add_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been fullfilled.
 *
 * @see rtnl_addr_build_add_request()
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_addr_add(struct nl_sock *sk, struct rtnl_addr *addr, int flags)
{
	struct nl_msg *msg;
	int err;

	if ((err = rtnl_addr_build_add_request(addr, flags, &msg)) < 0)
		return err;

	err = nl_send_auto_complete(sk, msg);
	nlmsg_free(msg);
	if (err < 0)
		return err;

	return wait_for_ack(sk);
}

/** @} */

/**
 * @name Deletion
 * @{
 */

/**
 * Build a netlink request message to request deletion of an address
 * @arg addr		Address object to be deleteted.
 * @arg flags		Additional netlink message flags.
 * @arg result		Pointer to store resulting message.
 *
 * Builds a new netlink message requesting a deletion of an address.
 * The netlink message header isn't fully equipped with all relevant
 * fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed.
 *
 * Minimal required attributes:
 *   - interface index (rtnl_addr_set_ifindex())
 *   - address family (rtnl_addr_set_family())
 *
 * Optional attributes:
 *   - local address (rtnl_addr_set_local())
 *   - label (rtnl_addr_set_label(), IPv4/DECnet only)
 *   - peer address (rtnl_addr_set_peer(), IPv4 only)
 *
 * @note Free the memory after usage using nlmsg_free().
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_addr_build_delete_request(struct rtnl_addr *addr, int flags,
				   struct nl_msg **result)
{
	uint32_t required = ADDR_ATTR_IFINDEX | ADDR_ATTR_FAMILY;

	if ((addr->ce_mask & required) != required)
		return -NLE_MISSING_ATTR;

	return build_addr_msg(addr, RTM_DELADDR, flags, result);
}

/**
 * Request deletion of an address
 * @arg sk		Netlink socket.
 * @arg addr		Address object to be deleted.
 * @arg flags		Additional netlink message flags.
 *
 * Builds a netlink message by calling rtnl_addr_build_delete_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been fullfilled.
 *
 * @see rtnl_addr_build_delete_request();
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_addr_delete(struct nl_sock *sk, struct rtnl_addr *addr, int flags)
{
	struct nl_msg *msg;
	int err;

	if ((err = rtnl_addr_build_delete_request(addr, flags, &msg)) < 0)
		return err;

	err = nl_send_auto_complete(sk, msg);
	nlmsg_free(msg);
	if (err < 0)
		return err;

	return wait_for_ack(sk);
}

/** @} */

/**
 * @name Attributes
 * @{
 */

int rtnl_addr_set_label(struct rtnl_addr *addr, const char *label)
{
	if (strlen(label) > sizeof(addr->a_label) - 1)
		return -NLE_RANGE;

	strcpy(addr->a_label, label);
	addr->ce_mask |= ADDR_ATTR_LABEL;

	return 0;
}

char *rtnl_addr_get_label(struct rtnl_addr *addr)
{
	if (addr->ce_mask & ADDR_ATTR_LABEL)
		return addr->a_label;
	else
		return NULL;
}

void rtnl_addr_set_ifindex(struct rtnl_addr *addr, int ifindex)
{
	addr->a_ifindex = ifindex;
	addr->ce_mask |= ADDR_ATTR_IFINDEX;
}

int rtnl_addr_get_ifindex(struct rtnl_addr *addr)
{
	return addr->a_ifindex;
}

void rtnl_addr_set_link(struct rtnl_addr *addr, struct rtnl_link *link)
{
	rtnl_link_put(addr->a_link);

	if (!link)
		return;

	nl_object_get(OBJ_CAST(link));
	addr->a_link = link;
	addr->a_ifindex = link->l_index;
	addr->ce_mask |= ADDR_ATTR_IFINDEX;
}

struct rtnl_link *rtnl_addr_get_link(struct rtnl_addr *addr)
{
	if (addr->a_link) {
		nl_object_get(OBJ_CAST(addr->a_link));
		return addr->a_link;
	}

	return NULL;
}

void rtnl_addr_set_family(struct rtnl_addr *addr, int family)
{
	addr->a_family = family;
	addr->ce_mask |= ADDR_ATTR_FAMILY;
}

int rtnl_addr_get_family(struct rtnl_addr *addr)
{
	return addr->a_family;
}

/**
 * Set the prefix length / netmask
 * @arg addr		Address
 * @arg prefixlen	Length of prefix (netmask)
 *
 * Modifies the length of the prefix. If the address object contains a peer
 * address the prefix length will apply to it, otherwise the prefix length
 * will apply to the local address of the address.
 *
 * If the address object contains a peer or local address the corresponding
 * `struct nl_addr` will be updated with the new prefix length.
 *
 * @note Specifying a length of 0 will remove the prefix length alltogether.
 *
 * @see rtnl_addr_get_prefixlen()
 */
void rtnl_addr_set_prefixlen(struct rtnl_addr *addr, int prefixlen)
{
	addr->a_prefixlen = prefixlen;

	if (prefixlen)
		addr->ce_mask |= ADDR_ATTR_PREFIXLEN;
	else
		addr->ce_mask &= ~ADDR_ATTR_PREFIXLEN;

	/*
	 * The prefix length always applies to the peer address if
	 * a peer address is present.
	 */
	if (addr->a_peer)
		nl_addr_set_prefixlen(addr->a_peer, prefixlen);
	else if (addr->a_local)
		nl_addr_set_prefixlen(addr->a_local, prefixlen);
}

int rtnl_addr_get_prefixlen(struct rtnl_addr *addr)
{
	return addr->a_prefixlen;
}

void rtnl_addr_set_scope(struct rtnl_addr *addr, int scope)
{
	addr->a_scope = scope;
	addr->ce_mask |= ADDR_ATTR_SCOPE;
}

int rtnl_addr_get_scope(struct rtnl_addr *addr)
{
	return addr->a_scope;
}

void rtnl_addr_set_flags(struct rtnl_addr *addr, unsigned int flags)
{
	addr->a_flag_mask |= flags;
	addr->a_flags |= flags;
	addr->ce_mask |= ADDR_ATTR_FLAGS;
}

void rtnl_addr_unset_flags(struct rtnl_addr *addr, unsigned int flags)
{
	addr->a_flag_mask |= flags;
	addr->a_flags &= ~flags;
	addr->ce_mask |= ADDR_ATTR_FLAGS;
}

unsigned int rtnl_addr_get_flags(struct rtnl_addr *addr)
{
	return addr->a_flags;
}

static inline int __assign_addr(struct rtnl_addr *addr, struct nl_addr **pos,
			        struct nl_addr *new, int flag)
{
	if (new) {
		if (addr->ce_mask & ADDR_ATTR_FAMILY) {
			if (new->a_family != addr->a_family)
				return -NLE_AF_MISMATCH;
		} else
			addr->a_family = new->a_family;

		if (*pos)
			nl_addr_put(*pos);

		*pos = nl_addr_get(new);
		addr->ce_mask |= (flag | ADDR_ATTR_FAMILY);
	} else {
		if (*pos)
			nl_addr_put(*pos);

		*pos = NULL;
		addr->ce_mask &= ~flag;
	}

	return 0;
}

int rtnl_addr_set_local(struct rtnl_addr *addr, struct nl_addr *local)
{
	int err;

	/* Prohibit local address with prefix length if peer address is present */
	if ((addr->ce_mask & ADDR_ATTR_PEER) && local &&
	    nl_addr_get_prefixlen(local))
		return -NLE_INVAL;

	err = __assign_addr(addr, &addr->a_local, local, ADDR_ATTR_LOCAL);
	if (err < 0)
		return err;

	/* Never overwrite the prefix length if a peer address is present */
	if (!(addr->ce_mask & ADDR_ATTR_PEER))
		rtnl_addr_set_prefixlen(addr, local ? nl_addr_get_prefixlen(local) : 0);

	return 0;
}

struct nl_addr *rtnl_addr_get_local(struct rtnl_addr *addr)
{
	return addr->a_local;
}

int rtnl_addr_set_peer(struct rtnl_addr *addr, struct nl_addr *peer)
{
	int err;

	if (peer && peer->a_family != AF_INET)
		return -NLE_AF_NOSUPPORT;

	err = __assign_addr(addr, &addr->a_peer, peer, ADDR_ATTR_PEER);
	if (err < 0)
		return err;

	rtnl_addr_set_prefixlen(addr, peer ? nl_addr_get_prefixlen(peer) : 0);

	return 0;
}

struct nl_addr *rtnl_addr_get_peer(struct rtnl_addr *addr)
{
	return addr->a_peer;
}

int rtnl_addr_set_broadcast(struct rtnl_addr *addr, struct nl_addr *bcast)
{
	if (bcast && bcast->a_family != AF_INET)
		return -NLE_AF_NOSUPPORT;

	return __assign_addr(addr, &addr->a_bcast, bcast, ADDR_ATTR_BROADCAST);
}

struct nl_addr *rtnl_addr_get_broadcast(struct rtnl_addr *addr)
{
	return addr->a_bcast;
}

int rtnl_addr_set_multicast(struct rtnl_addr *addr, struct nl_addr *multicast)
{
	if (multicast && multicast->a_family != AF_INET6)
		return -NLE_AF_NOSUPPORT;

	return __assign_addr(addr, &addr->a_multicast, multicast,
			     ADDR_ATTR_MULTICAST);
}

struct nl_addr *rtnl_addr_get_multicast(struct rtnl_addr *addr)
{
	return addr->a_multicast;
}

int rtnl_addr_set_anycast(struct rtnl_addr *addr, struct nl_addr *anycast)
{
	if (anycast && anycast->a_family != AF_INET6)
		return -NLE_AF_NOSUPPORT;

	return __assign_addr(addr, &addr->a_anycast, anycast,
			     ADDR_ATTR_ANYCAST);
}

struct nl_addr *rtnl_addr_get_anycast(struct rtnl_addr *addr)
{
	return addr->a_anycast;
}

uint32_t rtnl_addr_get_valid_lifetime(struct rtnl_addr *addr)
{
	if (addr->ce_mask & ADDR_ATTR_CACHEINFO)
		return addr->a_cacheinfo.aci_valid;
	else
		return 0xFFFFFFFFU;
}

void rtnl_addr_set_valid_lifetime(struct rtnl_addr *addr, uint32_t lifetime)
{
	addr->a_cacheinfo.aci_valid = lifetime;
	addr->ce_mask |= ADDR_ATTR_CACHEINFO;
}

uint32_t rtnl_addr_get_preferred_lifetime(struct rtnl_addr *addr)
{
	if (addr->ce_mask & ADDR_ATTR_CACHEINFO)
		return addr->a_cacheinfo.aci_prefered;
	else
		return 0xFFFFFFFFU;
}

void rtnl_addr_set_preferred_lifetime(struct rtnl_addr *addr, uint32_t lifetime)
{
	addr->a_cacheinfo.aci_prefered = lifetime;
	addr->ce_mask |= ADDR_ATTR_CACHEINFO;
}

uint32_t rtnl_addr_get_create_time(struct rtnl_addr *addr)
{
	return addr->a_cacheinfo.aci_cstamp;
}

uint32_t rtnl_addr_get_last_update_time(struct rtnl_addr *addr)
{
	return addr->a_cacheinfo.aci_tstamp;
}

/** @} */

/**
 * @name Flags Translations
 * @{
 */

static const struct trans_tbl addr_flags[] = {
	__ADD(IFA_F_SECONDARY, secondary)
	__ADD(IFA_F_NODAD, nodad)
	__ADD(IFA_F_OPTIMISTIC, optimistic)
	__ADD(IFA_F_HOMEADDRESS, homeaddress)
	__ADD(IFA_F_DEPRECATED, deprecated)
	__ADD(IFA_F_TENTATIVE, tentative)
	__ADD(IFA_F_PERMANENT, permanent)
	__ADD(IFA_F_MANAGETEMPADDR, mngtmpaddr)
	__ADD(IFA_F_NOPREFIXROUTE, noprefixroute)
};

char *rtnl_addr_flags2str(int flags, char *buf, size_t size)
{
	return __flags2str(flags, buf, size, addr_flags,
			   ARRAY_SIZE(addr_flags));
}

int rtnl_addr_str2flags(const char *name)
{
	return __str2flags(name, addr_flags, ARRAY_SIZE(addr_flags));
}

/** @} */

static struct nl_object_ops addr_obj_ops = {
	.oo_name		= "route/addr",
	.oo_size		= sizeof(struct rtnl_addr),
	.oo_constructor		= addr_constructor,
	.oo_free_data		= addr_free_data,
	.oo_clone		= addr_clone,
	.oo_dump = {
	    [NL_DUMP_LINE] 	= addr_dump_line,
	    [NL_DUMP_DETAILS]	= addr_dump_details,
	    [NL_DUMP_STATS]	= addr_dump_stats,
	},
	.oo_compare		= addr_compare,
	.oo_attrs2str		= addr_attrs2str,
	.oo_id_attrs		= (ADDR_ATTR_FAMILY | ADDR_ATTR_IFINDEX |
				   ADDR_ATTR_LOCAL | ADDR_ATTR_PREFIXLEN),
};

static struct nl_af_group addr_groups[] = {
	{ AF_INET,	RTNLGRP_IPV4_IFADDR },
	{ AF_INET6,	RTNLGRP_IPV6_IFADDR },
	{ END_OF_GROUP_LIST },
};

static struct nl_cache_ops rtnl_addr_ops = {
	.co_name		= "route/addr",
	.co_hdrsize		= sizeof(struct ifaddrmsg),
	.co_msgtypes		= {
					{ RTM_NEWADDR, NL_ACT_NEW, "new" },
					{ RTM_DELADDR, NL_ACT_DEL, "del" },
					{ RTM_GETADDR, NL_ACT_GET, "get" },
					END_OF_MSGTYPES_LIST,
				  },
	.co_protocol		= NETLINK_ROUTE,
	.co_groups		= addr_groups,
	.co_request_update      = addr_request_update,
	.co_msg_parser          = addr_msg_parser,
	.co_obj_ops		= &addr_obj_ops,
};

static void __init addr_init(void)
{
	nl_cache_mngt_register(&rtnl_addr_ops);
}

static void __exit addr_exit(void)
{
	nl_cache_mngt_unregister(&rtnl_addr_ops);
}

/** @} */

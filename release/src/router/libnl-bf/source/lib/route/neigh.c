/*
 * lib/route/neigh.c	Neighbours
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup rtnl
 * @defgroup neigh Neighbours
 * @brief
 *
 * The neighbour table establishes bindings between protocol addresses and
 * link layer addresses for hosts sharing the same physical link. This
 * module allows you to access and manipulate the content of these tables.
 *
 * @par Neighbour States
 * @code
 * NUD_INCOMPLETE
 * NUD_REACHABLE
 * NUD_STALE
 * NUD_DELAY
 * NUD_PROBE
 * NUD_FAILED
 * NUD_NOARP
 * NUD_PERMANENT
 * @endcode
 *
 * @par Neighbour Flags
 * @code
 * NTF_USE
 * NTF_PROXY
 * NTF_ROUTER
 * @endcode
 *
 * @par Neighbour Identification
 * A neighbour is uniquely identified by the attributes listed below, whenever
 * you refer to an existing neighbour all of the attributes must be set.
 * Neighbours from caches automatically have all required attributes set.
 *   - interface index (rtnl_neigh_set_ifindex())
 *   - destination address (rtnl_neigh_set_dst())
 *
 * @par Changeable Attributes
 * \anchor neigh_changeable
 *  - state (rtnl_neigh_set_state())
 *  - link layer address (rtnl_neigh_set_lladdr())
 *
 * @par Required Caches for Dumping
 * In order to dump neighbour attributes you must provide the following
 * caches via nl_cache_provide()
 *  - link cache holding all links
 *
 * @par TODO
 *   - Document proxy settings
 *   - Document states and their influence
 *
 * @par 1) Retrieving information about configured neighbours
 * @code
 * // The first step is to retrieve a list of all available neighbour within
 * // the kernel and put them into a cache.
 * struct nl_cache *cache = rtnl_neigh_alloc_cache(sk);
 *
 * // Neighbours can then be looked up by the interface and destination
 * // address:
 * struct rtnl_neigh *neigh = rtnl_neigh_get(cache, ifindex, dst_addr);
 * 
 * // After successful usage, the object must be given back to the cache
 * rtnl_neigh_put(neigh);
 * @endcode
 *
 * @par 2) Adding new neighbours
 * @code
 * // Allocate an empty neighbour handle to be filled out with the attributes
 * // of the new neighbour.
 * struct rtnl_neigh *neigh = rtnl_neigh_alloc();
 *
 * // Fill out the attributes of the new neighbour
 * rtnl_neigh_set_ifindex(neigh, ifindex);
 * rtnl_neigh_set_dst(neigh, dst_addr);
 * rtnl_neigh_set_state(neigh, rtnl_neigh_str2state("permanent"));
 *
 * // Build the netlink message and send it to the kernel, the operation will
 * // block until the operation has been completed. Alternatively the required
 * // netlink message can be built using rtnl_neigh_build_add_request()
 * // to be sent out using nl_send_auto_complete().
 * rtnl_neigh_add(sk, neigh, NLM_F_CREATE);
 *
 * // Free the memory
 * rtnl_neigh_put(neigh);
 * @endcode
 *
 * @par 3) Deleting an existing neighbour
 * @code
 * // Allocate an empty neighbour object to be filled out with the attributes
 * // matching the neighbour to be deleted. Alternatively a fully equipped
 * // neighbour object out of a cache can be used instead.
 * struct rtnl_neigh *neigh = rtnl_neigh_alloc();
 *
 * // Neighbours are uniquely identified by their interface index and
 * // destination address, you may fill out other attributes but they
 * // will have no influence.
 * rtnl_neigh_set_ifindex(neigh, ifindex);
 * rtnl_neigh_set_dst(neigh, dst_addr);
 *
 * // Build the netlink message and send it to the kernel, the operation will
 * // block until the operation has been completed. Alternatively the required
 * // netlink message can be built using rtnl_neigh_build_delete_request()
 * // to be sent out using nl_send_auto_complete().
 * rtnl_neigh_delete(sk, neigh, 0);
 *
 * // Free the memory
 * rtnl_neigh_put(neigh);
 * @endcode
 *
 * @par 4) Changing neighbour attributes
 * @code
 * // Allocate an empty neighbour object to be filled out with the attributes
 * // matching the neighbour to be changed and the new parameters. Alternatively
 * // a fully equipped modified neighbour object out of a cache can be used.
 * struct rtnl_neigh *neigh = rtnl_neigh_alloc();
 *
 * // Identify the neighbour to be changed by its interface index and
 * // destination address
 * rtnl_neigh_set_ifindex(neigh, ifindex);
 * rtnl_neigh_set_dst(neigh, dst_addr);
 *
 * // The link layer address may be modified, if so it is wise to change
 * // its state to "permanent" in order to avoid having it overwritten.
 * rtnl_neigh_set_lladdr(neigh, lladdr);
 *
 * // Secondly the state can be modified allowing normal neighbours to be
 * // converted into permanent entries or to manually confirm a neighbour.
 * rtnl_neigh_set_state(neigh, state);
 *
 * // Build the netlink message and send it to the kernel, the operation will
 * // block until the operation has been completed. Alternatively the required
 * // netlink message can be built using rtnl_neigh_build_change_request()
 * // to be sent out using nl_send_auto_complete().
 * rtnl_neigh_add(sk, neigh, NLM_F_REPLACE);
 *
 * // Free the memory
 * rtnl_neigh_put(neigh);
 * @endcode
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/neighbour.h>
#include <netlink/route/link.h>

/** @cond SKIP */
#define NEIGH_ATTR_FLAGS        0x01
#define NEIGH_ATTR_STATE        0x02
#define NEIGH_ATTR_LLADDR       0x04
#define NEIGH_ATTR_DST          0x08
#define NEIGH_ATTR_CACHEINFO    0x10
#define NEIGH_ATTR_IFINDEX      0x20
#define NEIGH_ATTR_FAMILY       0x40
#define NEIGH_ATTR_TYPE         0x80
#define NEIGH_ATTR_PROBES       0x100

static struct nl_cache_ops rtnl_neigh_ops;
static struct nl_object_ops neigh_obj_ops;
/** @endcond */

static void neigh_free_data(struct nl_object *c)
{
	struct rtnl_neigh *neigh = nl_object_priv(c);

	if (!neigh)
		return;

	nl_addr_put(neigh->n_lladdr);
	nl_addr_put(neigh->n_dst);
}

static int neigh_clone(struct nl_object *_dst, struct nl_object *_src)
{
	struct rtnl_neigh *dst = nl_object_priv(_dst);
	struct rtnl_neigh *src = nl_object_priv(_src);

	if (src->n_lladdr)
		if (!(dst->n_lladdr = nl_addr_clone(src->n_lladdr)))
			return -NLE_NOMEM;

	if (src->n_dst)
		if (!(dst->n_dst = nl_addr_clone(src->n_dst)))
			return -NLE_NOMEM;

	return 0;
}

static int neigh_compare(struct nl_object *_a, struct nl_object *_b,
			uint32_t attrs, int flags)
{
	struct rtnl_neigh *a = (struct rtnl_neigh *) _a;
	struct rtnl_neigh *b = (struct rtnl_neigh *) _b;
	int diff = 0;

#define NEIGH_DIFF(ATTR, EXPR) ATTR_DIFF(attrs, NEIGH_ATTR_##ATTR, a, b, EXPR)

	diff |= NEIGH_DIFF(IFINDEX,	a->n_ifindex != b->n_ifindex);
	diff |= NEIGH_DIFF(FAMILY,	a->n_family != b->n_family);
	diff |= NEIGH_DIFF(TYPE,	a->n_type != b->n_type);
	diff |= NEIGH_DIFF(LLADDR,	nl_addr_cmp(a->n_lladdr, b->n_lladdr));
	diff |= NEIGH_DIFF(DST,		nl_addr_cmp(a->n_dst, b->n_dst));

	if (flags & LOOSE_COMPARISON) {
		diff |= NEIGH_DIFF(STATE,
				  (a->n_state ^ b->n_state) & b->n_state_mask);
		diff |= NEIGH_DIFF(FLAGS,
				  (a->n_flags ^ b->n_flags) & b->n_flag_mask);
	} else {
		diff |= NEIGH_DIFF(STATE, a->n_state != b->n_state);
		diff |= NEIGH_DIFF(FLAGS, a->n_flags != b->n_flags);
	}

#undef NEIGH_DIFF

	return diff;
}

static const struct trans_tbl neigh_attrs[] = {
	__ADD(NEIGH_ATTR_FLAGS, flags)
	__ADD(NEIGH_ATTR_STATE, state)
	__ADD(NEIGH_ATTR_LLADDR, lladdr)
	__ADD(NEIGH_ATTR_DST, dst)
	__ADD(NEIGH_ATTR_CACHEINFO, cacheinfo)
	__ADD(NEIGH_ATTR_IFINDEX, ifindex)
	__ADD(NEIGH_ATTR_FAMILY, family)
	__ADD(NEIGH_ATTR_TYPE, type)
	__ADD(NEIGH_ATTR_PROBES, probes)
};

static char *neigh_attrs2str(int attrs, char *buf, size_t len)
{
	return __flags2str(attrs, buf, len, neigh_attrs,
			   ARRAY_SIZE(neigh_attrs));
}

static struct nla_policy neigh_policy[NDA_MAX+1] = {
	[NDA_CACHEINFO]	= { .minlen = sizeof(struct nda_cacheinfo) },
	[NDA_PROBES]	= { .type = NLA_U32 },
};

static int neigh_msg_parser(struct nl_cache_ops *ops, struct sockaddr_nl *who,
			    struct nlmsghdr *n, struct nl_parser_param *pp)
{
	struct rtnl_neigh *neigh;
	int err;

	if ((err = rtnl_neigh_parse(n, &neigh)) < 0)
		return err;

	err = pp->pp_cb((struct nl_object *) neigh, pp);

	rtnl_neigh_put(neigh);
	return err;
}


int rtnl_neigh_parse(struct nlmsghdr *n, struct rtnl_neigh **result)
{
	struct rtnl_neigh *neigh;
	struct nlattr *tb[NDA_MAX + 1];
	struct ndmsg *nm;
	int err;

	neigh = rtnl_neigh_alloc();
	if (!neigh) {
		err = -NLE_NOMEM;
		goto errout;
	}

	neigh->ce_msgtype = n->nlmsg_type;
	nm = nlmsg_data(n);

	err = nlmsg_parse(n, sizeof(*nm), tb, NDA_MAX, neigh_policy);
	if (err < 0)
		goto errout;

	neigh->n_family  = nm->ndm_family;
	neigh->n_ifindex = nm->ndm_ifindex;
	neigh->n_state   = nm->ndm_state;
	neigh->n_flags   = nm->ndm_flags;
	neigh->n_type    = nm->ndm_type;

	neigh->ce_mask |= (NEIGH_ATTR_FAMILY | NEIGH_ATTR_IFINDEX |
			   NEIGH_ATTR_STATE | NEIGH_ATTR_FLAGS |
			   NEIGH_ATTR_TYPE);

	if (tb[NDA_LLADDR]) {
		neigh->n_lladdr = nl_addr_alloc_attr(tb[NDA_LLADDR], AF_UNSPEC);
		if (!neigh->n_lladdr) {
			err = -NLE_NOMEM;
			goto errout;
		}
		nl_addr_set_family(neigh->n_lladdr,
				   nl_addr_guess_family(neigh->n_lladdr));
		neigh->ce_mask |= NEIGH_ATTR_LLADDR;
	}

	if (tb[NDA_DST]) {
		neigh->n_dst = nl_addr_alloc_attr(tb[NDA_DST], neigh->n_family);
		if (!neigh->n_dst) {
			err = -NLE_NOMEM;
			goto errout;
		}
		neigh->ce_mask |= NEIGH_ATTR_DST;
	}

	if (tb[NDA_CACHEINFO]) {
		struct nda_cacheinfo *ci = nla_data(tb[NDA_CACHEINFO]);

		neigh->n_cacheinfo.nci_confirmed = ci->ndm_confirmed;
		neigh->n_cacheinfo.nci_used = ci->ndm_used;
		neigh->n_cacheinfo.nci_updated = ci->ndm_updated;
		neigh->n_cacheinfo.nci_refcnt = ci->ndm_refcnt;
		
		neigh->ce_mask |= NEIGH_ATTR_CACHEINFO;
	}

	if (tb[NDA_PROBES]) {
		neigh->n_probes = nla_get_u32(tb[NDA_PROBES]);
		neigh->ce_mask |= NEIGH_ATTR_PROBES;
	}

	*result = neigh;
	return 0;

errout:
	rtnl_neigh_put(neigh);
	return err;
}

static int neigh_request_update(struct nl_cache *c, struct nl_sock *h)
{
	return nl_rtgen_request(h, RTM_GETNEIGH, AF_UNSPEC, NLM_F_DUMP);
}


static void neigh_dump_line(struct nl_object *a, struct nl_dump_params *p)
{
	char dst[INET6_ADDRSTRLEN+5], lladdr[INET6_ADDRSTRLEN+5];
	struct rtnl_neigh *n = (struct rtnl_neigh *) a;
	struct nl_cache *link_cache;
	char state[128], flags[64];

	link_cache = nl_cache_mngt_require("route/link");

	nl_dump_line(p, "%s ", nl_addr2str(n->n_dst, dst, sizeof(dst)));

	if (link_cache)
		nl_dump(p, "dev %s ",
			rtnl_link_i2name(link_cache, n->n_ifindex,
					 state, sizeof(state)));
	else
		nl_dump(p, "dev %d ", n->n_ifindex);

	if (n->ce_mask & NEIGH_ATTR_LLADDR)
		nl_dump(p, "lladdr %s ",
			nl_addr2str(n->n_lladdr, lladdr, sizeof(lladdr)));

	rtnl_neigh_state2str(n->n_state, state, sizeof(state));
	rtnl_neigh_flags2str(n->n_flags, flags, sizeof(flags));

	if (state[0])
		nl_dump(p, "<%s", state);
	if (flags[0])
		nl_dump(p, "%s%s", state[0] ? "," : "<", flags);
	if (state[0] || flags[0])
		nl_dump(p, ">");
	nl_dump(p, "\n");
}

static void neigh_dump_details(struct nl_object *a, struct nl_dump_params *p)
{
	char rtn_type[32];
	struct rtnl_neigh *n = (struct rtnl_neigh *) a;
	int hz = nl_get_user_hz();

	neigh_dump_line(a, p);

	nl_dump_line(p, "    refcnt %u type %s confirmed %u used "
				"%u updated %u\n",
		n->n_cacheinfo.nci_refcnt,
		nl_rtntype2str(n->n_type, rtn_type, sizeof(rtn_type)),
		n->n_cacheinfo.nci_confirmed/hz,
		n->n_cacheinfo.nci_used/hz, n->n_cacheinfo.nci_updated/hz);
}

static void neigh_dump_stats(struct nl_object *a, struct nl_dump_params *p)
{
	neigh_dump_details(a, p);
}

/**
 * @name Neighbour Object Allocation/Freeage
 * @{
 */

struct rtnl_neigh *rtnl_neigh_alloc(void)
{
	return (struct rtnl_neigh *) nl_object_alloc(&neigh_obj_ops);
}

void rtnl_neigh_put(struct rtnl_neigh *neigh)
{
	nl_object_put((struct nl_object *) neigh);
}

/** @} */

/**
 * @name Neighbour Cache Managament
 * @{
 */

/**
 * Build a neighbour cache including all neighbours currently configured in the kernel.
 * @arg sock		Netlink socket.
 * @arg result		Pointer to store resulting cache.
 *
 * Allocates a new neighbour cache, initializes it properly and updates it
 * to include all neighbours currently configured in the kernel.
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_neigh_alloc_cache(struct nl_sock *sock, struct nl_cache **result)
{
	return nl_cache_alloc_and_fill(&rtnl_neigh_ops, sock, result);
}

/**
 * Look up a neighbour by interface index and destination address
 * @arg cache		neighbour cache
 * @arg ifindex		interface index the neighbour is on
 * @arg dst		destination address of the neighbour
 * @return neighbour handle or NULL if no match was found.
 */
struct rtnl_neigh * rtnl_neigh_get(struct nl_cache *cache, int ifindex,
				   struct nl_addr *dst)
{
	struct rtnl_neigh *neigh;

	nl_list_for_each_entry(neigh, &cache->c_items, ce_list) {
		if (neigh->n_ifindex == ifindex &&
		    !nl_addr_cmp(neigh->n_dst, dst)) {
			nl_object_get((struct nl_object *) neigh);
			return neigh;
		}
	}

	return NULL;
}

/** @} */

/**
 * @name Neighbour Addition
 * @{
 */

static int build_neigh_msg(struct rtnl_neigh *tmpl, int cmd, int flags,
			   struct nl_msg **result)
{
	struct nl_msg *msg;
	struct ndmsg nhdr = {
		.ndm_ifindex = tmpl->n_ifindex,
		.ndm_state = NUD_PERMANENT,
	};

	if (!(tmpl->ce_mask & NEIGH_ATTR_DST))
		return -NLE_MISSING_ATTR;

	nhdr.ndm_family = nl_addr_get_family(tmpl->n_dst);

	if (tmpl->ce_mask & NEIGH_ATTR_FLAGS)
		nhdr.ndm_flags = tmpl->n_flags;

	if (tmpl->ce_mask & NEIGH_ATTR_STATE)
		nhdr.ndm_state = tmpl->n_state;

	msg = nlmsg_alloc_simple(cmd, flags);
	if (!msg)
		return -NLE_NOMEM;

	if (nlmsg_append(msg, &nhdr, sizeof(nhdr), NLMSG_ALIGNTO) < 0)
		goto nla_put_failure;

	NLA_PUT_ADDR(msg, NDA_DST, tmpl->n_dst);

	if (tmpl->ce_mask & NEIGH_ATTR_LLADDR)
		NLA_PUT_ADDR(msg, NDA_LLADDR, tmpl->n_lladdr);

	*result = msg;
	return 0;

nla_put_failure:
	nlmsg_free(msg);
	return -NLE_MSGSIZE;
}

/**
 * Build netlink request message to add a new neighbour
 * @arg tmpl		template with data of new neighbour
 * @arg flags		additional netlink message flags
 * @arg result		Pointer to store resulting message.
 *
 * Builds a new netlink message requesting a addition of a new
 * neighbour. The netlink message header isn't fully equipped with
 * all relevant fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed. \a tmpl must contain the attributes of the new
 * neighbour set via \c rtnl_neigh_set_* functions.
 * 
 * The following attributes must be set in the template:
 *  - Interface index (rtnl_neigh_set_ifindex())
 *  - State (rtnl_neigh_set_state())
 *  - Destination address (rtnl_neigh_set_dst())
 *  - Link layer address (rtnl_neigh_set_lladdr())
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_neigh_build_add_request(struct rtnl_neigh *tmpl, int flags,
				 struct nl_msg **result)
{
	return build_neigh_msg(tmpl, RTM_NEWNEIGH, flags, result);
}

/**
 * Add a new neighbour
 * @arg sk		Netlink socket.
 * @arg tmpl		template with requested changes
 * @arg flags		additional netlink message flags
 *
 * Builds a netlink message by calling rtnl_neigh_build_add_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been fullfilled.
 *
 * The following attributes must be set in the template:
 *  - Interface index (rtnl_neigh_set_ifindex())
 *  - State (rtnl_neigh_set_state())
 *  - Destination address (rtnl_neigh_set_dst())
 *  - Link layer address (rtnl_neigh_set_lladdr())
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_neigh_add(struct nl_sock *sk, struct rtnl_neigh *tmpl, int flags)
{
	int err;
	struct nl_msg *msg;
	
	if ((err = rtnl_neigh_build_add_request(tmpl, flags, &msg)) < 0)
		return err;

	err = nl_send_auto_complete(sk, msg);
	nlmsg_free(msg);
	if (err < 0)
		return err;

	return wait_for_ack(sk);
}

/** @} */

/**
 * @name Neighbour Deletion
 * @{
 */

/**
 * Build a netlink request message to delete a neighbour
 * @arg neigh		neighbour to delete
 * @arg flags		additional netlink message flags
 * @arg result		Pointer to store resulting message.
 *
 * Builds a new netlink message requesting a deletion of a neighbour.
 * The netlink message header isn't fully equipped with all relevant
 * fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed. \a neigh must point to an existing
 * neighbour.
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_neigh_build_delete_request(struct rtnl_neigh *neigh, int flags,
				    struct nl_msg **result)
{
	return build_neigh_msg(neigh, RTM_DELNEIGH, flags, result);
}

/**
 * Delete a neighbour
 * @arg sk		Netlink socket.
 * @arg neigh		neighbour to delete
 * @arg flags		additional netlink message flags
 *
 * Builds a netlink message by calling rtnl_neigh_build_delete_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been fullfilled.
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_neigh_delete(struct nl_sock *sk, struct rtnl_neigh *neigh,
		      int flags)
{
	struct nl_msg *msg;
	int err;
	
	if ((err = rtnl_neigh_build_delete_request(neigh, flags, &msg)) < 0)
		return err;

	err = nl_send_auto_complete(sk, msg);
	nlmsg_free(msg);
	if (err < 0)
		return err;

	return wait_for_ack(sk);
}

/** @} */

/**
 * @name Neighbour States Translations
 * @{
 */

static const struct trans_tbl neigh_states[] = {
	__ADD(NUD_INCOMPLETE, incomplete)
	__ADD(NUD_REACHABLE, reachable)
	__ADD(NUD_STALE, stale)
	__ADD(NUD_DELAY, delay)
	__ADD(NUD_PROBE, probe)
	__ADD(NUD_FAILED, failed)
	__ADD(NUD_NOARP, norarp)
	__ADD(NUD_PERMANENT, permanent)
};

char * rtnl_neigh_state2str(int state, char *buf, size_t len)
{
	return __flags2str(state, buf, len, neigh_states,
	    ARRAY_SIZE(neigh_states));
}

int rtnl_neigh_str2state(const char *name)
{
	return __str2type(name, neigh_states, ARRAY_SIZE(neigh_states));
}

/** @} */

/**
 * @name Neighbour Flags Translations
 * @{
 */

static const struct trans_tbl neigh_flags[] = {
	__ADD(NTF_USE, use)
	__ADD(NTF_PROXY, proxy)
	__ADD(NTF_ROUTER, router)
};

char * rtnl_neigh_flags2str(int flags, char *buf, size_t len)
{
	return __flags2str(flags, buf, len, neigh_flags,
	    ARRAY_SIZE(neigh_flags));
}

int rtnl_neigh_str2flag(const char *name)
{
	return __str2type(name, neigh_flags, ARRAY_SIZE(neigh_flags));
}

/** @} */

/**
 * @name Attributes
 * @{
 */

void rtnl_neigh_set_state(struct rtnl_neigh *neigh, int state)
{
	neigh->n_state_mask |= state;
	neigh->n_state |= state;
	neigh->ce_mask |= NEIGH_ATTR_STATE;
}

int rtnl_neigh_get_state(struct rtnl_neigh *neigh)
{
	if (neigh->ce_mask & NEIGH_ATTR_STATE)
		return neigh->n_state;
	else
		return -1;
}

void rtnl_neigh_unset_state(struct rtnl_neigh *neigh, int state)
{
	neigh->n_state_mask |= state;
	neigh->n_state &= ~state;
	neigh->ce_mask |= NEIGH_ATTR_STATE;
}

void rtnl_neigh_set_flags(struct rtnl_neigh *neigh, unsigned int flags)
{
	neigh->n_flag_mask |= flags;
	neigh->n_flags |= flags;
	neigh->ce_mask |= NEIGH_ATTR_FLAGS;
}

unsigned int rtnl_neigh_get_flags(struct rtnl_neigh *neigh)
{
	return neigh->n_flags;
}

void rtnl_neigh_unset_flags(struct rtnl_neigh *neigh, unsigned int flags)
{
	neigh->n_flag_mask |= flags;
	neigh->n_flags &= ~flags;
	neigh->ce_mask |= NEIGH_ATTR_FLAGS;
}

void rtnl_neigh_set_ifindex(struct rtnl_neigh *neigh, int ifindex)
{
	neigh->n_ifindex = ifindex;
	neigh->ce_mask |= NEIGH_ATTR_IFINDEX;
}

int rtnl_neigh_get_ifindex(struct rtnl_neigh *neigh)
{
	return neigh->n_ifindex;
}

static inline int __assign_addr(struct rtnl_neigh *neigh, struct nl_addr **pos,
			        struct nl_addr *new, int flag, int nocheck)
{
	if (!nocheck) {
		if (neigh->ce_mask & NEIGH_ATTR_FAMILY) {
			if (new->a_family != neigh->n_family)
				return -NLE_AF_MISMATCH;
		} else {
			neigh->n_family = new->a_family;
			neigh->ce_mask |= NEIGH_ATTR_FAMILY;
		}
	}

	if (*pos)
		nl_addr_put(*pos);

	nl_addr_get(new);
	*pos = new;

	neigh->ce_mask |= flag;

	return 0;
}

void rtnl_neigh_set_lladdr(struct rtnl_neigh *neigh, struct nl_addr *addr)
{
	__assign_addr(neigh, &neigh->n_lladdr, addr, NEIGH_ATTR_LLADDR, 1);
}

struct nl_addr *rtnl_neigh_get_lladdr(struct rtnl_neigh *neigh)
{
	if (neigh->ce_mask & NEIGH_ATTR_LLADDR)
		return neigh->n_lladdr;
	else
		return NULL;
}

int rtnl_neigh_set_dst(struct rtnl_neigh *neigh, struct nl_addr *addr)
{
	return __assign_addr(neigh, &neigh->n_dst, addr,
			     NEIGH_ATTR_DST, 0);
}

struct nl_addr *rtnl_neigh_get_dst(struct rtnl_neigh *neigh)
{
	if (neigh->ce_mask & NEIGH_ATTR_DST)
		return neigh->n_dst;
	else
		return NULL;
}

void rtnl_neigh_set_family(struct rtnl_neigh *neigh, int family)
{
	neigh->n_family = family;
	neigh->ce_mask |= NEIGH_ATTR_FAMILY;
}

int rtnl_neigh_get_family(struct rtnl_neigh *neigh)
{
	return neigh->n_family;
}

void rtnl_neigh_set_type(struct rtnl_neigh *neigh, int type)
{
	neigh->n_type = type;
	neigh->ce_mask = NEIGH_ATTR_TYPE;
}

int rtnl_neigh_get_type(struct rtnl_neigh *neigh)
{
	if (neigh->ce_mask & NEIGH_ATTR_TYPE)
		return neigh->n_type;
	else
		return -1;
}

/** @} */

static struct nl_object_ops neigh_obj_ops = {
	.oo_name		= "route/neigh",
	.oo_size		= sizeof(struct rtnl_neigh),
	.oo_free_data		= neigh_free_data,
	.oo_clone		= neigh_clone,
	.oo_dump = {
	    [NL_DUMP_LINE]	= neigh_dump_line,
	    [NL_DUMP_DETAILS]	= neigh_dump_details,
	    [NL_DUMP_STATS]	= neigh_dump_stats,
	},
	.oo_compare		= neigh_compare,
	.oo_attrs2str		= neigh_attrs2str,
	.oo_id_attrs		= (NEIGH_ATTR_IFINDEX | NEIGH_ATTR_DST | NEIGH_ATTR_FAMILY),
};

static struct nl_af_group neigh_groups[] = {
	{ AF_UNSPEC, RTNLGRP_NEIGH },
	{ END_OF_GROUP_LIST },
};

static struct nl_cache_ops rtnl_neigh_ops = {
	.co_name		= "route/neigh",
	.co_hdrsize		= sizeof(struct ndmsg),
	.co_msgtypes		= {
					{ RTM_NEWNEIGH, NL_ACT_NEW, "new" },
					{ RTM_DELNEIGH, NL_ACT_DEL, "del" },
					{ RTM_GETNEIGH, NL_ACT_GET, "get" },
					END_OF_MSGTYPES_LIST,
				  },
	.co_protocol		= NETLINK_ROUTE,
	.co_groups		= neigh_groups,
	.co_request_update	= neigh_request_update,
	.co_msg_parser		= neigh_msg_parser,
	.co_obj_ops		= &neigh_obj_ops,
};

static void __init neigh_init(void)
{
	nl_cache_mngt_register(&rtnl_neigh_ops);
}

static void __exit neigh_exit(void)
{
	nl_cache_mngt_unregister(&rtnl_neigh_ops);
}

/** @} */

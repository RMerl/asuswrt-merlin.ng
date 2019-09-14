/*
 * lib/route/link/bridge.c	AF_BRIDGE link support
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010-2013 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup link
 * @defgroup bridge Bridging
 *
 * @details
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink/netlink.h>
#include <netlink/attr.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/link/bridge.h>
#include <netlink-private/route/link/api.h>
#include <linux/if_bridge.h>

/** @cond SKIP */
#define BRIDGE_ATTR_PORT_STATE		(1 << 0)
#define BRIDGE_ATTR_PRIORITY		(1 << 1)
#define BRIDGE_ATTR_COST		(1 << 2)
#define BRIDGE_ATTR_FLAGS		(1 << 3)

#define PRIV_FLAG_NEW_ATTRS		(1 << 0)

struct bridge_data
{
	uint8_t			b_port_state;
	uint8_t			b_priv_flags; /* internal flags */
	uint16_t		b_priority;
	uint32_t		b_cost;
	uint32_t		b_flags;
	uint32_t		b_flags_mask;
	uint32_t                ce_mask; /* HACK to support attr macros */
};

static struct rtnl_link_af_ops bridge_ops;

#define IS_BRIDGE_LINK_ASSERT(link) \
	if (!rtnl_link_is_bridge(link)) { \
		APPBUG("A function was expecting a link object of type bridge."); \
		return -NLE_OPNOTSUPP; \
	}

static inline struct bridge_data *bridge_data(struct rtnl_link *link)
{
	return rtnl_link_af_data(link, &bridge_ops);
}

static void *bridge_alloc(struct rtnl_link *link)
{
	return calloc(1, sizeof(struct bridge_data));
}

static void *bridge_clone(struct rtnl_link *link, void *data)
{
	struct bridge_data *bd;

	if ((bd = bridge_alloc(link)))
		memcpy(bd, data, sizeof(*bd));

	return bd;
}

static void bridge_free(struct rtnl_link *link, void *data)
{
	free(data);
}

static struct nla_policy br_attrs_policy[IFLA_BRPORT_MAX+1] = {
	[IFLA_BRPORT_STATE]		= { .type = NLA_U8 },
	[IFLA_BRPORT_PRIORITY]		= { .type = NLA_U16 },
	[IFLA_BRPORT_COST]		= { .type = NLA_U32 },
	[IFLA_BRPORT_MODE]		= { .type = NLA_U8 },
	[IFLA_BRPORT_GUARD]		= { .type = NLA_U8 },
	[IFLA_BRPORT_PROTECT]		= { .type = NLA_U8 },
	[IFLA_BRPORT_FAST_LEAVE]	= { .type = NLA_U8 },
};

static void check_flag(struct rtnl_link *link, struct nlattr *attrs[],
		       int type, int flag)
{
	if (attrs[type] && nla_get_u8(attrs[type]))
		rtnl_link_bridge_set_flags(link, flag);
}

static int bridge_parse_protinfo(struct rtnl_link *link, struct nlattr *attr,
				 void *data)
{
	struct bridge_data *bd = data;
	struct nlattr *br_attrs[IFLA_BRPORT_MAX+1];
	int err;

	/* Backwards compatibility */
	if (!nla_is_nested(attr)) {
		if (nla_len(attr) < 1)
			return -NLE_RANGE;

		bd->b_port_state = nla_get_u8(attr);
		bd->ce_mask |= BRIDGE_ATTR_PORT_STATE;

		return 0;
	}

	if ((err = nla_parse_nested(br_attrs, IFLA_BRPORT_MAX, attr,
	     br_attrs_policy)) < 0)
		return err;

	bd->b_priv_flags |= PRIV_FLAG_NEW_ATTRS;

	if (br_attrs[IFLA_BRPORT_STATE]) {
		bd->b_port_state = nla_get_u8(br_attrs[IFLA_BRPORT_STATE]);
		bd->ce_mask |= BRIDGE_ATTR_PORT_STATE;
	}

	if (br_attrs[IFLA_BRPORT_PRIORITY]) {
		bd->b_priority = nla_get_u16(br_attrs[IFLA_BRPORT_PRIORITY]);
		bd->ce_mask |= BRIDGE_ATTR_PRIORITY;
	}

	if (br_attrs[IFLA_BRPORT_COST]) {
		bd->b_cost = nla_get_u32(br_attrs[IFLA_BRPORT_COST]);
		bd->ce_mask |= BRIDGE_ATTR_COST;
	}

	check_flag(link, br_attrs, IFLA_BRPORT_MODE, RTNL_BRIDGE_HAIRPIN_MODE);
	check_flag(link, br_attrs, IFLA_BRPORT_GUARD, RTNL_BRIDGE_BPDU_GUARD);
	check_flag(link, br_attrs, IFLA_BRPORT_PROTECT, RTNL_BRIDGE_ROOT_BLOCK);
	check_flag(link, br_attrs, IFLA_BRPORT_FAST_LEAVE, RTNL_BRIDGE_FAST_LEAVE);

	return 0;
}

static void bridge_dump_details(struct rtnl_link *link,
				struct nl_dump_params *p, void *data)
{
	struct bridge_data *bd = data;

	nl_dump_line(p, "    bridge: ");

	if (bd->ce_mask & BRIDGE_ATTR_PORT_STATE)
		nl_dump(p, "port-state %u ", bd->b_port_state);

	if (bd->ce_mask & BRIDGE_ATTR_PRIORITY)
		nl_dump(p, "prio %u ", bd->b_priority);

	if (bd->ce_mask & BRIDGE_ATTR_COST)
		nl_dump(p, "cost %u ", bd->b_cost);

	nl_dump(p, "\n");
}

static int bridge_compare(struct rtnl_link *_a, struct rtnl_link *_b,
			  int family, uint32_t attrs, int flags)
{
	struct bridge_data *a = bridge_data(_a);
	struct bridge_data *b = bridge_data(_b);
	int diff = 0;

#define BRIDGE_DIFF(ATTR, EXPR) ATTR_DIFF(attrs, BRIDGE_ATTR_##ATTR, a, b, EXPR)
	diff |= BRIDGE_DIFF(PORT_STATE,	a->b_port_state != b->b_port_state);
	diff |= BRIDGE_DIFF(PRIORITY, a->b_priority != b->b_priority);
	diff |= BRIDGE_DIFF(COST, a->b_cost != b->b_cost);

	if (flags & LOOSE_COMPARISON)
		diff |= BRIDGE_DIFF(FLAGS,
				  (a->b_flags ^ b->b_flags) & b->b_flags_mask);
	else
		diff |= BRIDGE_DIFF(FLAGS, a->b_flags != b->b_flags);
#undef BRIDGE_DIFF

	return diff;
}
/** @endcond */

/**
 * Allocate link object of type bridge
 *
 * @return Allocated link object or NULL.
 */
struct rtnl_link *rtnl_link_bridge_alloc(void)
{
	struct rtnl_link *link;
	int err;

	if (!(link = rtnl_link_alloc()))
		return NULL;

	if ((err = rtnl_link_set_type(link, "bridge")) < 0) {
		rtnl_link_put(link);
		return NULL;
	}

	return link;
}
		
/** 
 * Create a new kernel bridge device
 * @arg sk              netlink socket
 * @arg name            name of the bridge device or NULL
 *
 * Creates a new bridge device in the kernel. If no name is
 * provided, the kernel will automatically pick a name of the
 * form "type%d" (e.g. bridge0, vlan1, etc.)
 *
 * @return 0 on success or a negative error code
*/
int rtnl_link_bridge_add(struct nl_sock *sk, const char *name)
{
	int err;
	struct rtnl_link *link;

	if (!(link = rtnl_link_bridge_alloc()))
		return -NLE_NOMEM;

	if(name)
		rtnl_link_set_name(link, name);

	err = rtnl_link_add(sk, link, NLM_F_CREATE);
	rtnl_link_put(link);

	return err;
}

/**
 * Check if a link is a bridge
 * @arg link		Link object
 *
 * @return 1 if the link is a bridge, 0 otherwise.
 */
int rtnl_link_is_bridge(struct rtnl_link *link)
{
	return link->l_family == AF_BRIDGE &&
	       link->l_af_ops == &bridge_ops;
}

/**
 * Check if bridge has extended information
 * @arg link		Link object of type bridge
 *
 * Checks if the bridge object has been constructed based on
 * information that is only available in newer kernels. This
 * affectes the following functions:
 *  - rtnl_link_bridge_get_cost()
 *  - rtnl_link_bridge_get_priority()
 *  - rtnl_link_bridge_get_flags()
 *
 * @return 1 if extended information is available, otherwise 0 is returned.
 */
int rtnl_link_bridge_has_ext_info(struct rtnl_link *link)
{
	struct bridge_data *bd;

	if (!rtnl_link_is_bridge(link))
		return 0;

	bd = bridge_data(link);
	return !!(bd->b_priv_flags & PRIV_FLAG_NEW_ATTRS);
}

/**
 * Set Spanning Tree Protocol (STP) port state
 * @arg link		Link object of type bridge
 * @arg state		New STP port state
 *
 * The value of state must be one of the following:
 *   - BR_STATE_DISABLED
 *   - BR_STATE_LISTENING
 *   - BR_STATE_LEARNING
 *   - BR_STATE_FORWARDING
 *   - BR_STATE_BLOCKING
 *
 * @see rtnl_link_bridge_get_port_state()
 *
 * @return 0 on success or a negative error code.
 * @retval -NLE_OPNOTSUPP Link is not a bridge
 * @retval -NLE_INVAL Invalid state value (0..BR_STATE_BLOCKING)
 */
int rtnl_link_bridge_set_port_state(struct rtnl_link *link, uint8_t state)
{
	struct bridge_data *bd = bridge_data(link);

	IS_BRIDGE_LINK_ASSERT(link);

	if (state > BR_STATE_BLOCKING)
		return -NLE_INVAL;

	bd->b_port_state = state;
	bd->ce_mask |= BRIDGE_ATTR_PORT_STATE;

	return 0;
}

/**
 * Get Spanning Tree Protocol (STP) port state
 * @arg link		Link object of type bridge
 *
 * @see rtnl_link_bridge_set_port_state()
 *
 * @return The STP port state or a negative error code.
 * @retval -NLE_OPNOTSUPP Link is not a bridge
 */
int rtnl_link_bridge_get_port_state(struct rtnl_link *link)
{
	struct bridge_data *bd = bridge_data(link);

	IS_BRIDGE_LINK_ASSERT(link);

	return bd->b_port_state;
}

/**
 * Set priority
 * @arg link		Link object of type bridge
 * @arg prio		Bridge priority
 *
 * @see rtnl_link_bridge_get_priority()
 *
 * @return 0 on success or a negative error code.
 * @retval -NLE_OPNOTSUPP Link is not a bridge
 */
int rtnl_link_bridge_set_priority(struct rtnl_link *link, uint16_t prio)
{
	struct bridge_data *bd = bridge_data(link);

	IS_BRIDGE_LINK_ASSERT(link);

	bd->b_priority = prio;
	bd->ce_mask |= BRIDGE_ATTR_PRIORITY;

	return 0;
}

/**
 * Get priority
 * @arg link		Link object of type bridge
 *
 * @see rtnl_link_bridge_set_priority()
 *
 * @return 0 on success or a negative error code.
 * @retval -NLE_OPNOTSUPP Link is not a bridge
 */
int rtnl_link_bridge_get_priority(struct rtnl_link *link)
{
	struct bridge_data *bd = bridge_data(link);

	IS_BRIDGE_LINK_ASSERT(link);

	return bd->b_priority;
}

/**
 * Set Spanning Tree Protocol (STP) path cost
 * @arg link		Link object of type bridge
 * @arg cost		New STP path cost value
 *
 * @see rtnl_link_bridge_get_cost()
 *
 * @return The bridge priority or a negative error code.
 * @retval -NLE_OPNOTSUPP Link is not a bridge
 */
int rtnl_link_bridge_set_cost(struct rtnl_link *link, uint32_t cost)
{
	struct bridge_data *bd = bridge_data(link);

	IS_BRIDGE_LINK_ASSERT(link);

	bd->b_cost = cost;
	bd->ce_mask |= BRIDGE_ATTR_COST;

	return 0;
}

/**
 * Get Spanning Tree Protocol (STP) path cost
 * @arg link		Link object of type bridge
 * @arg cost		Pointer to store STP cost value
 *
 * @see rtnl_link_bridge_set_cost()
 *
 * @return 0 on success or a negative error code.
 * @retval -NLE_OPNOTSUPP Link is not a bridge
 * @retval -NLE_INVAL `cost` is not a valid pointer
 */
int rtnl_link_bridge_get_cost(struct rtnl_link *link, uint32_t *cost)
{
	struct bridge_data *bd = bridge_data(link);

	IS_BRIDGE_LINK_ASSERT(link);

	if (!cost)
		return -NLE_INVAL;

	*cost = bd->b_cost;

	return 0;
}

/**
 * Unset flags
 * @arg link		Link object of type bridge
 * @arg flags		Bridging flags to unset
 *
 * @see rtnl_link_bridge_set_flags()
 * @see rtnl_link_bridge_get_flags()
 *
 * @return 0 on success or a negative error code.
 * @retval -NLE_OPNOTSUPP Link is not a bridge
 */
int rtnl_link_bridge_unset_flags(struct rtnl_link *link, unsigned int flags)
{
	struct bridge_data *bd = bridge_data(link);

	IS_BRIDGE_LINK_ASSERT(link);

	bd->b_flags_mask |= flags;
	bd->b_flags &= ~flags;
	bd->ce_mask |= BRIDGE_ATTR_FLAGS;

	return 0;
}

/**
 * Set flags
 * @arg link		Link object of type bridge
 * @arg flags		Bridging flags to set
 *
 * Valid flags are:
 *   - RTNL_BRIDGE_HAIRPIN_MODE
 *   - RTNL_BRIDGE_BPDU_GUARD
 *   - RTNL_BRIDGE_ROOT_BLOCK
 *   - RTNL_BRIDGE_FAST_LEAVE
 *
 * @see rtnl_link_bridge_unset_flags()
 * @see rtnl_link_bridge_get_flags()
 *
 * @return 0 on success or a negative error code.
 * @retval -NLE_OPNOTSUPP Link is not a bridge
 */
int rtnl_link_bridge_set_flags(struct rtnl_link *link, unsigned int flags)
{
	struct bridge_data *bd = bridge_data(link);

	IS_BRIDGE_LINK_ASSERT(link);

	bd->b_flags_mask |= flags;
	bd->b_flags |= flags;
	bd->ce_mask |= BRIDGE_ATTR_FLAGS;

	return 0;
}

/**
 * Get flags
 * @arg link		Link object of type bridge
 *
 * @see rtnl_link_bridge_set_flags()
 * @see rtnl_link_bridge_unset_flags()
 *
 * @return Flags or a negative error code.
 * @retval -NLE_OPNOTSUPP Link is not a bridge
 */
int rtnl_link_bridge_get_flags(struct rtnl_link *link)
{
	struct bridge_data *bd = bridge_data(link);

	IS_BRIDGE_LINK_ASSERT(link);

	return bd->b_flags;
}

static const struct trans_tbl bridge_flags[] = {
	__ADD(RTNL_BRIDGE_HAIRPIN_MODE, hairpin_mode)
	__ADD(RTNL_BRIDGE_BPDU_GUARD, 	bpdu_guard)
	__ADD(RTNL_BRIDGE_ROOT_BLOCK,	root_block)
	__ADD(RTNL_BRIDGE_FAST_LEAVE,	fast_leave)
};

/**
 * @name Flag Translation
 * @{
 */

char *rtnl_link_bridge_flags2str(int flags, char *buf, size_t len)
{
	return __flags2str(flags, buf, len, bridge_flags, ARRAY_SIZE(bridge_flags));
}

int rtnl_link_bridge_str2flags(const char *name)
{
	return __str2flags(name, bridge_flags, ARRAY_SIZE(bridge_flags));
}

/** @} */

static struct rtnl_link_af_ops bridge_ops = {
	.ao_family			= AF_BRIDGE,
	.ao_alloc			= &bridge_alloc,
	.ao_clone			= &bridge_clone,
	.ao_free			= &bridge_free,
	.ao_parse_protinfo		= &bridge_parse_protinfo,
	.ao_dump[NL_DUMP_DETAILS]	= &bridge_dump_details,
	.ao_compare			= &bridge_compare,
};

static void __init bridge_init(void)
{
	rtnl_link_af_register(&bridge_ops);
}

static void __exit bridge_exit(void)
{
	rtnl_link_af_unregister(&bridge_ops);
}

/** @} */

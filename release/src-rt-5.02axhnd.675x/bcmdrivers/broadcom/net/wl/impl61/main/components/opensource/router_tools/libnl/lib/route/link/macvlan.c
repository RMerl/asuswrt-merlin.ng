/*
 * lib/route/link/macvlan.c	MACVLAN Link Info
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Michael Braun <michael-dev@fami-braun.de>
 */

/**
 * @ingroup link
 * @defgroup macvlan MACVLAN
 * MAC-based Virtual LAN link module
 *
 * @details
 * \b Link Type Name: "macvlan"
 *
 * @route_doc{link_macvlan, MACVLAN Documentation}
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
#include <netlink/route/link/macvlan.h>

#include <linux/if_link.h>

/** @cond SKIP */
#define MACVLAN_HAS_MODE	(1<<0)
#define MACVLAN_HAS_FLAGS	(1<<1)

struct macvlan_info
{
	uint32_t		mvi_mode;
	uint16_t		mvi_flags; // there currently is only one flag and kernel has no flags_mask yet
	uint32_t		mvi_mask;
};

/** @endcond */

static struct nla_policy macvlan_policy[IFLA_MACVLAN_MAX+1] = {
	[IFLA_MACVLAN_MODE]	= { .type = NLA_U32 },
	[IFLA_MACVLAN_FLAGS]	= { .type = NLA_U16 },
};

static int macvlan_alloc(struct rtnl_link *link)
{
	struct macvlan_info *mvi;

	if ((mvi = calloc(1, sizeof(*mvi))) == NULL)
		return -NLE_NOMEM;

	link->l_info = mvi;

	return 0;
}

static int macvlan_parse(struct rtnl_link *link, struct nlattr *data,
                         struct nlattr *xstats)
{
	struct nlattr *tb[IFLA_MACVLAN_MAX+1];
	struct macvlan_info *mvi;
	int err;

	NL_DBG(3, "Parsing MACVLAN link info");

	if ((err = nla_parse_nested(tb, IFLA_MACVLAN_MAX, data, macvlan_policy)) < 0)
		goto errout;

	if ((err = macvlan_alloc(link)) < 0)
		goto errout;

	mvi = link->l_info;

	if (tb[IFLA_MACVLAN_MODE]) {
		mvi->mvi_mode = nla_get_u32(tb[IFLA_MACVLAN_MODE]);
		mvi->mvi_mask |= MACVLAN_HAS_MODE;
	}

	if (tb[IFLA_MACVLAN_FLAGS]) {
		mvi->mvi_mode = nla_get_u16(tb[IFLA_MACVLAN_FLAGS]);
		mvi->mvi_mask |= MACVLAN_HAS_FLAGS;
	}

	err = 0;
errout:
	return err;
}

static void macvlan_free(struct rtnl_link *link)
{
	free(link->l_info);
	link->l_info = NULL;
}

static void macvlan_dump(struct rtnl_link *link, struct nl_dump_params *p)
{
	char buf[64];
	struct macvlan_info *mvi = link->l_info;

	if (mvi->mvi_mask & MACVLAN_HAS_MODE) {
		rtnl_link_macvlan_mode2str(mvi->mvi_mode, buf, sizeof(buf));
		nl_dump(p, "macvlan-mode %s", buf);
	}

	if (mvi->mvi_mask & MACVLAN_HAS_FLAGS) {
		rtnl_link_macvlan_flags2str(mvi->mvi_flags, buf, sizeof(buf));
		nl_dump(p, "macvlan-flags %s", buf);
	}
}

static int macvlan_clone(struct rtnl_link *dst, struct rtnl_link *src)
{
	struct macvlan_info *vdst, *vsrc = src->l_info;
	int err;

	dst->l_info = NULL;
	if ((err = rtnl_link_set_type(dst, "macvlan")) < 0)
		return err;
	vdst = dst->l_info;

	if (!vdst || !vsrc)
		return -NLE_NOMEM;

	memcpy(vdst, vsrc, sizeof(struct macvlan_info));

	return 0;
}

static int macvlan_put_attrs(struct nl_msg *msg, struct rtnl_link *link)
{
	struct macvlan_info *mvi = link->l_info;
	struct nlattr *data;

	if (!(data = nla_nest_start(msg, IFLA_INFO_DATA)))
		return -NLE_MSGSIZE;

	if (mvi->mvi_mask & MACVLAN_HAS_MODE)
		NLA_PUT_U32(msg, IFLA_MACVLAN_MODE, mvi->mvi_mode);

	if (mvi->mvi_mask & MACVLAN_HAS_FLAGS)
		NLA_PUT_U16(msg, IFLA_MACVLAN_FLAGS, mvi->mvi_flags);

	nla_nest_end(msg, data);

nla_put_failure:

	return 0;
}

static struct rtnl_link_info_ops macvlan_info_ops = {
	.io_name		= "macvlan",
	.io_alloc		= macvlan_alloc,
	.io_parse		= macvlan_parse,
	.io_dump = {
	    [NL_DUMP_LINE]	= macvlan_dump,
	    [NL_DUMP_DETAILS]	= macvlan_dump,
	},
	.io_clone		= macvlan_clone,
	.io_put_attrs		= macvlan_put_attrs,
	.io_free		= macvlan_free,
};

/** @cond SKIP */
#define IS_MACVLAN_LINK_ASSERT(link) \
	if ((link)->l_info_ops != &macvlan_info_ops) { \
		APPBUG("Link is not a macvlan link. set type \"macvlan\" first."); \
		return -NLE_OPNOTSUPP; \
	}
/** @endcond */

/**
 * @name MACVLAN Object
 * @{
 */

/**
 * Allocate link object of type MACVLAN
 *
 * @return Allocated link object or NULL.
 */
struct rtnl_link *rtnl_link_macvlan_alloc(void)
{
	struct rtnl_link *link;
	int err;

	if (!(link = rtnl_link_alloc()))
		return NULL;

	if ((err = rtnl_link_set_type(link, "macvlan")) < 0) {
		rtnl_link_put(link);
		return NULL;
	}

	return link;
}

/**
 * Check if link is a MACVLAN link
 * @arg link		Link object
 *
 * @return True if link is a MACVLAN link, otherwise false is returned.
 */
int rtnl_link_is_macvlan(struct rtnl_link *link)
{
	return link->l_info_ops && !strcmp(link->l_info_ops->io_name, "macvlan");
}

/**
 * Set MACVLAN MODE
 * @arg link		Link object
 * @arg mode		MACVLAN mode
 *
 * @return 0 on success or a negative error code
 */
int rtnl_link_macvlan_set_mode(struct rtnl_link *link, uint32_t mode)
{
	struct macvlan_info *mvi = link->l_info;

	IS_MACVLAN_LINK_ASSERT(link);

	mvi->mvi_mode = mode;
	mvi->mvi_mask |= MACVLAN_HAS_MODE;

	return 0;
}

/**
 * Get MACVLAN Mode
 * @arg link		Link object
 *
 * @return MACVLAN mode, 0 if not set or a negative error code.
 */
uint32_t rtnl_link_macvlan_get_mode(struct rtnl_link *link)
{
	struct macvlan_info *mvi = link->l_info;

	IS_MACVLAN_LINK_ASSERT(link);

	if (mvi->mvi_mask & MACVLAN_HAS_MODE)
		return mvi->mvi_mode;
	else
		return 0;
}

/**
 * Set MACVLAN flags
 * @arg link		Link object
 * @arg flags		MACVLAN flags
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_macvlan_set_flags(struct rtnl_link *link, uint16_t flags)
{
	struct macvlan_info *mvi = link->l_info;

	IS_MACVLAN_LINK_ASSERT(link);

	mvi->mvi_flags |= flags;
	mvi->mvi_mask |= MACVLAN_HAS_FLAGS;

	return 0;
}

/**
 * Unset MACVLAN flags
 * @arg link		Link object
 * @arg flags		MACVLAN flags
 *
 * Note: kernel currently only has a single flag and lacks flags_mask to
 * indicate which flags shall be changed (it always all).
 *
 * @return 0 on success or a negative error code.
 */
int rtnl_link_macvlan_unset_flags(struct rtnl_link *link, uint16_t flags)
{
	struct macvlan_info *mvi = link->l_info;

	IS_MACVLAN_LINK_ASSERT(link);

	mvi->mvi_flags &= ~flags;
	mvi->mvi_mask |= MACVLAN_HAS_FLAGS;

	return 0;
}

/**
 * Get MACVLAN flags
 * @arg link		Link object
 *
 * @return MACVLAN flags, 0 if none set, or a negative error code.
 */
uint16_t rtnl_link_macvlan_get_flags(struct rtnl_link *link)
{
	struct macvlan_info *mvi = link->l_info;

	IS_MACVLAN_LINK_ASSERT(link);

	return mvi->mvi_flags;
}

/** @} */

static const struct trans_tbl macvlan_flags[] = {
	__ADD(MACVLAN_FLAG_NOPROMISC, nopromisc)
};

static const struct trans_tbl macvlan_modes[] = {
	__ADD(MACVLAN_MODE_PRIVATE, private)
	__ADD(MACVLAN_MODE_VEPA, vepa)
	__ADD(MACVLAN_MODE_BRIDGE, bridge)
	__ADD(MACVLAN_MODE_PASSTHRU, passthru)
};

/**
 * @name Flag Translation
 * @{
 */

char *rtnl_link_macvlan_flags2str(int flags, char *buf, size_t len)
{
	return __flags2str(flags, buf, len, macvlan_flags, ARRAY_SIZE(macvlan_flags));
}

int rtnl_link_macvlan_str2flags(const char *name)
{
	return __str2flags(name, macvlan_flags, ARRAY_SIZE(macvlan_flags));
}

/** @} */

/**
 * @name Mode Translation
 * @{
 */

char *rtnl_link_macvlan_mode2str(int mode, char *buf, size_t len)
{
	return __type2str(mode, buf, len, macvlan_modes, ARRAY_SIZE(macvlan_modes));
}

int rtnl_link_macvlan_str2mode(const char *name)
{
	return __str2type(name, macvlan_modes, ARRAY_SIZE(macvlan_modes));
}

/** @} */

static void __init macvlan_init(void)
{
	rtnl_link_register_info(&macvlan_info_ops);
}

static void __exit macvlan_exit(void)
{
	rtnl_link_unregister_info(&macvlan_info_ops);
}

/** @} */

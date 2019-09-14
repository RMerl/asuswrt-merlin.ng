/*
 * lib/genl/ctrl.c		Generic Netlink Controller
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2012 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup genl
 * @defgroup genl_ctrl Controller (Resolver)
 *
 * Resolves Generic Netlink family names to numeric identifiers.
 *
 * The controller is a component in the kernel that resolves Generic Netlink
 * family names to their numeric identifiers. This module provides functions
 * to query the controller to access the resolving functionality.
 * @{
 */

#include <netlink-private/genl.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/mngt.h>
#include <netlink/genl/ctrl.h>
#include <netlink/utils.h>

/** @cond SKIP */
#define CTRL_VERSION		0x0001

static struct nl_cache_ops genl_ctrl_ops;

static int ctrl_request_update(struct nl_cache *c, struct nl_sock *h)
{
	return genl_send_simple(h, GENL_ID_CTRL, CTRL_CMD_GETFAMILY,
				CTRL_VERSION, NLM_F_DUMP);
}

static struct nla_policy ctrl_policy[CTRL_ATTR_MAX+1] = {
	[CTRL_ATTR_FAMILY_ID]	= { .type = NLA_U16 },
	[CTRL_ATTR_FAMILY_NAME]	= { .type = NLA_STRING,
				    .maxlen = GENL_NAMSIZ },
	[CTRL_ATTR_VERSION]	= { .type = NLA_U32 },
	[CTRL_ATTR_HDRSIZE]	= { .type = NLA_U32 },
	[CTRL_ATTR_MAXATTR]	= { .type = NLA_U32 },
	[CTRL_ATTR_OPS]		= { .type = NLA_NESTED },
	[CTRL_ATTR_MCAST_GROUPS] = { .type = NLA_NESTED },
};

static struct nla_policy family_op_policy[CTRL_ATTR_OP_MAX+1] = {
	[CTRL_ATTR_OP_ID]	= { .type = NLA_U32 },
	[CTRL_ATTR_OP_FLAGS]	= { .type = NLA_U32 },
};

static struct nla_policy family_grp_policy[CTRL_ATTR_MCAST_GRP_MAX+1] = {
	[CTRL_ATTR_MCAST_GRP_NAME] = { .type = NLA_STRING },
	[CTRL_ATTR_MCAST_GRP_ID]   = { .type = NLA_U32 },
};

static int parse_mcast_grps(struct genl_family *family, struct nlattr *grp_attr)
{
	struct nlattr *nla;
	int remaining, err;

	if (!grp_attr)
		BUG();

	nla_for_each_nested(nla, grp_attr, remaining) {
		struct nlattr *tb[CTRL_ATTR_MCAST_GRP_MAX+1];
		int id;
		const char * name;

		err = nla_parse_nested(tb, CTRL_ATTR_MCAST_GRP_MAX, nla,
				       family_grp_policy);
		if (err < 0)
			goto errout;

		if (tb[CTRL_ATTR_MCAST_GRP_ID] == NULL) {
			err = -NLE_MISSING_ATTR;
			goto errout;
		}
		id = nla_get_u32(tb[CTRL_ATTR_MCAST_GRP_ID]);

		if (tb[CTRL_ATTR_MCAST_GRP_NAME] == NULL) {
			err = -NLE_MISSING_ATTR;
			goto errout;
		}
		name = nla_get_string(tb[CTRL_ATTR_MCAST_GRP_NAME]);

		err = genl_family_add_grp(family, id, name);
		if (err < 0)
			goto errout;
	}

	err = 0;

errout:
	return err;
}

static int ctrl_msg_parser(struct nl_cache_ops *ops, struct genl_cmd *cmd,
			   struct genl_info *info, void *arg)
{
	struct genl_family *family;
	struct nl_parser_param *pp = arg;
	int err;

	family = genl_family_alloc();
	if (family == NULL) {
		err = -NLE_NOMEM;
		goto errout;
	}

	if (info->attrs[CTRL_ATTR_FAMILY_NAME] == NULL) {
		err = -NLE_MISSING_ATTR;
		goto errout;
	}

	if (info->attrs[CTRL_ATTR_FAMILY_ID] == NULL) {
		err = -NLE_MISSING_ATTR;
		goto errout;
	}

	family->ce_msgtype = info->nlh->nlmsg_type;
	genl_family_set_id(family,
			   nla_get_u16(info->attrs[CTRL_ATTR_FAMILY_ID]));
	genl_family_set_name(family,
		     nla_get_string(info->attrs[CTRL_ATTR_FAMILY_NAME]));

	if (info->attrs[CTRL_ATTR_VERSION]) {
		uint32_t version = nla_get_u32(info->attrs[CTRL_ATTR_VERSION]);
		genl_family_set_version(family, version);
	}

	if (info->attrs[CTRL_ATTR_HDRSIZE]) {
		uint32_t hdrsize = nla_get_u32(info->attrs[CTRL_ATTR_HDRSIZE]);
		genl_family_set_hdrsize(family, hdrsize);
	}

	if (info->attrs[CTRL_ATTR_MAXATTR]) {
		uint32_t maxattr = nla_get_u32(info->attrs[CTRL_ATTR_MAXATTR]);
		genl_family_set_maxattr(family, maxattr);
	}

	if (info->attrs[CTRL_ATTR_OPS]) {
		struct nlattr *nla, *nla_ops;
		int remaining;

		nla_ops = info->attrs[CTRL_ATTR_OPS];
		nla_for_each_nested(nla, nla_ops, remaining) {
			struct nlattr *tb[CTRL_ATTR_OP_MAX+1];
			int flags = 0, id;

			err = nla_parse_nested(tb, CTRL_ATTR_OP_MAX, nla,
					       family_op_policy);
			if (err < 0)
				goto errout;

			if (tb[CTRL_ATTR_OP_ID] == NULL) {
				err = -NLE_MISSING_ATTR;
				goto errout;
			}
			
			id = nla_get_u32(tb[CTRL_ATTR_OP_ID]);

			if (tb[CTRL_ATTR_OP_FLAGS])
				flags = nla_get_u32(tb[CTRL_ATTR_OP_FLAGS]);

			err = genl_family_add_op(family, id, flags);
			if (err < 0)
				goto errout;

		}
	}
	
	if (info->attrs[CTRL_ATTR_MCAST_GROUPS]) {
		err = parse_mcast_grps(family, info->attrs[CTRL_ATTR_MCAST_GROUPS]);
		if (err < 0)
			goto errout;
	}

	err = pp->pp_cb((struct nl_object *) family, pp);
errout:
	genl_family_put(family);
	return err;
}

/**
 * process responses from from the query sent by genl_ctrl_probe_by_name 
 * @arg nl_msg		Returned message.
 * @arg name		genl_family structure to fill out.
 *
 * Process returned messages, filling out the missing informatino in the
 * genl_family structure
 *
 * @return Indicator to keep processing frames or not
 *
 */
static int probe_response(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb[CTRL_ATTR_MAX+1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct genl_family *ret = (struct genl_family *)arg;

	if (genlmsg_parse(nlh, 0, tb, CTRL_ATTR_MAX, ctrl_policy))
		return NL_SKIP;

	if (tb[CTRL_ATTR_FAMILY_ID])
		genl_family_set_id(ret, nla_get_u16(tb[CTRL_ATTR_FAMILY_ID]));

	if (tb[CTRL_ATTR_MCAST_GROUPS])
		if (parse_mcast_grps(ret, tb[CTRL_ATTR_MCAST_GROUPS]) < 0)
			return NL_SKIP;

	return NL_STOP;
}

/**
 * Look up generic netlink family by family name querying the kernel directly
 * @arg sk		Socket.
 * @arg name		Family name.
 *
 * Directly query's the kernel for a given family name.  The caller will own a
 * reference on the returned object which needsd to be given back after usage
 * using genl_family_put.
 *
 * Note: This API call differs from genl_ctrl_search_by_name in that it querys
 * the kernel directly, alowing for module autoload to take place to resolve the
 * family request. Using an nl_cache prevents that operation
 *
 * @return Generic netlink family object or NULL if no match was found.
 */
static struct genl_family *genl_ctrl_probe_by_name(struct nl_sock *sk,
						   const char *name)
{
	struct nl_msg *msg;
	struct genl_family *ret;
	struct nl_cb *cb, *orig;
	int rc;

	ret = genl_family_alloc();
	if (!ret)
		goto out;

	genl_family_set_name(ret, name);

	msg = nlmsg_alloc();
	if (!msg)
		goto out_fam_free;

	if (!(orig = nl_socket_get_cb(sk)))
		goto out_msg_free;

	cb = nl_cb_clone(orig);
	nl_cb_put(orig);
	if (!cb)
		goto out_msg_free;

	if (!genlmsg_put(msg, NL_AUTO_PORT, NL_AUTO_SEQ, GENL_ID_CTRL,
			0, 0, CTRL_CMD_GETFAMILY, 1)) {
		BUG();
		goto out_cb_free;
	}

	if (nla_put_string(msg, CTRL_ATTR_FAMILY_NAME, name) < 0)
		goto out_cb_free;

	rc = nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, probe_response,
		       (void *) ret);
	if (rc < 0)
		goto out_cb_free;

	rc = nl_send_auto_complete(sk, msg);
	if (rc < 0)
		goto out_cb_free;

	rc = nl_recvmsgs(sk, cb);
	if (rc < 0)
		goto out_cb_free;

	/* If search was successful, request may be ACKed after data */
	rc = wait_for_ack(sk);
	if (rc < 0)
		goto out_cb_free;

	if (genl_family_get_id(ret) != 0) {
		nlmsg_free(msg);
		nl_cb_put(cb);
		return ret;
	}

out_cb_free:
	nl_cb_put(cb);
out_msg_free:
	nlmsg_free(msg);
out_fam_free:
	genl_family_put(ret);
	ret = NULL;
out:
	return ret;
}


/** @endcond */

/**
 * @name Controller Cache
 *
 * The controller cache allows to keep a local copy of the list of all
 * kernel side registered Generic Netlink families to quickly resolve
 * multiple Generic Netlink family names without requiring to communicate
 * with the kernel for each resolving iteration. 
 *
 * @{
 */

/**
 * Allocate a new controller cache
 * @arg sk		Generic Netlink socket
 * @arg result		Pointer to store resulting cache
 *
 * Allocates a new cache mirroring the state of the controller and stores it
 * in \c *result. The allocated cache will contain a list of all currently
 * registered kernel side Generic Netlink families. The cache is meant to be
 * used to resolve family names locally.
 *
 * @return 0 on success or a negative error code.
 */
int genl_ctrl_alloc_cache(struct nl_sock *sk, struct nl_cache **result)
{
	return nl_cache_alloc_and_fill(&genl_ctrl_ops, sk, result);
}

/**
 * Search controller cache for a numeric address match
 * @arg cache		Controller cache
 * @arg id		Numeric family identifier.
 *
 * Searches a previously allocated controller cache and looks for an entry
 * that matches the specified numeric family identifier \c id.  If a match
 * is found successfully, the reference count of the matching object is
 * increased by one before the objet is returned.
 *
 * @see genl_ctrl_alloc_cache()
 * @see genl_ctrl_search_by_name()
 * @see genl_family_put()
 *
 * @return Generic Netlink family object or NULL if no match was found.
 */
struct genl_family *genl_ctrl_search(struct nl_cache *cache, int id)
{
	struct genl_family *fam;

	if (cache->c_ops != &genl_ctrl_ops)
		BUG();

	nl_list_for_each_entry(fam, &cache->c_items, ce_list) {
		if (fam->gf_id == id) {
			nl_object_get((struct nl_object *) fam);
			return fam;
		}
	}

	return NULL;
}

/**
 * Search controller cache for a family name match
 * @arg cache		Controller cache
 * @arg name		Name of Generic Netlink family
 *
 * Searches a previously allocated controller cache and looks for an entry
 * that matches the specified family \c name. If a match is found successfully,
 * the reference count of the matching object is increased by one before the
 * objet is returned.
 *
 * @see genl_ctrl_alloc_cache()
 * @see genl_ctrl_search()
 * @see genl_family_put()
 *
 * @return Generic Netlink family object or NULL if no match was found.
 */
struct genl_family *genl_ctrl_search_by_name(struct nl_cache *cache,
					     const char *name)
{
	struct genl_family *fam;

	if (cache->c_ops != &genl_ctrl_ops)
		BUG();

	nl_list_for_each_entry(fam, &cache->c_items, ce_list) {
		if (!strcmp(name, fam->gf_name)) {
			nl_object_get((struct nl_object *) fam);
			return fam;
		}
	}

	return NULL;
}

/** @} */

/**
 * @name Direct Resolvers
 *
 * These functions communicate directly with the kernel and do not require
 * a cache to be kept up to date.
 *
 * @{
 */

/**
 * Resolve Generic Netlink family name to numeric identifier
 * @arg sk		Generic Netlink socket.
 * @arg name		Name of Generic Netlink family
 *
 * Resolves the Generic Netlink family name to the corresponding numeric
 * family identifier. This function queries the kernel directly, use
 * genl_ctrl_search_by_name() if you need to resolve multiple names.
 *
 * @see genl_ctrl_search_by_name()
 *
 * @return The numeric family identifier or a negative error code.
 */
int genl_ctrl_resolve(struct nl_sock *sk, const char *name)
{
	struct genl_family *family;
	int err;

	family = genl_ctrl_probe_by_name(sk, name);
	if (family == NULL) {
		err = -NLE_OBJ_NOTFOUND;
		goto errout;
	}

	err = genl_family_get_id(family);
	genl_family_put(family);
errout:
	return err;
}

static int genl_ctrl_grp_by_name(const struct genl_family *family,
				 const char *grp_name)
{
	struct genl_family_grp *grp;

	nl_list_for_each_entry(grp, &family->gf_mc_grps, list) {
		if (!strcmp(grp->name, grp_name)) {
			return grp->id;
		}
	}

	return -NLE_OBJ_NOTFOUND;
}

/**
 * Resolve Generic Netlink family group name
 * @arg sk		Generic Netlink socket
 * @arg family_name	Name of Generic Netlink family
 * @arg grp_name	Name of group to resolve
 *
 * Looks up the family object and resolves the group name to the numeric
 * group identifier.
 *
 * @return Numeric group identifier or a negative error code.
 */
int genl_ctrl_resolve_grp(struct nl_sock *sk, const char *family_name,
			  const char *grp_name)
{

	struct genl_family *family;
	int err;

	family = genl_ctrl_probe_by_name(sk, family_name);
	if (family == NULL) {
		err = -NLE_OBJ_NOTFOUND;
		goto errout;
	}

	err = genl_ctrl_grp_by_name(family, grp_name);
	genl_family_put(family);
errout:
	return err;
}

/** @} */

/** @cond SKIP */
static struct genl_cmd genl_cmds[] = {
	{
		.c_id		= CTRL_CMD_NEWFAMILY,
		.c_name		= "NEWFAMILY" ,
		.c_maxattr	= CTRL_ATTR_MAX,
		.c_attr_policy	= ctrl_policy,
		.c_msg_parser	= ctrl_msg_parser,
	},
	{
		.c_id		= CTRL_CMD_DELFAMILY,
		.c_name		= "DELFAMILY" ,
	},
	{
		.c_id		= CTRL_CMD_GETFAMILY,
		.c_name		= "GETFAMILY" ,
	},
	{
		.c_id		= CTRL_CMD_NEWOPS,
		.c_name		= "NEWOPS" ,
	},
	{
		.c_id		= CTRL_CMD_DELOPS,
		.c_name		= "DELOPS" ,
	},
};

static struct genl_ops genl_ops = {
	.o_cmds			= genl_cmds,
	.o_ncmds		= ARRAY_SIZE(genl_cmds),
};

extern struct nl_object_ops genl_family_ops;

static struct nl_cache_ops genl_ctrl_ops = {
	.co_name		= "genl/family",
	.co_hdrsize		= GENL_HDRSIZE(0),
	.co_msgtypes		= GENL_FAMILY(GENL_ID_CTRL, "nlctrl"),
	.co_genl		= &genl_ops,
	.co_protocol		= NETLINK_GENERIC,
	.co_request_update      = ctrl_request_update,
	.co_obj_ops		= &genl_family_ops,
};

static void __init ctrl_init(void)
{
	genl_register(&genl_ctrl_ops);
}

static void __exit ctrl_exit(void)
{
	genl_unregister(&genl_ctrl_ops);
}
/** @endcond */

/** @} */

/*
 * lib/genl/mngt.c		Generic Netlink Management
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
 * @defgroup genl_mngt Family and Command Registration
 *
 * Registering Generic Netlink Families and Commands
 *
 * @{
 */

#include <netlink-private/genl.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/mngt.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/utils.h>

/** @cond SKIP */

static NL_LIST_HEAD(genl_ops_list);

static struct genl_cmd *lookup_cmd(struct genl_ops *ops, int cmd_id)
{
	struct genl_cmd *cmd;
	int i;

	for (i = 0; i < ops->o_ncmds; i++) {
		cmd = &ops->o_cmds[i];
		if (cmd->c_id == cmd_id)
			return cmd;
	}

	return NULL;
}

static int cmd_msg_parser(struct sockaddr_nl *who, struct nlmsghdr *nlh,
			  struct genl_ops *ops, struct nl_cache_ops *cache_ops, void *arg)
{
	int err;
	struct genlmsghdr *ghdr;
	struct genl_cmd *cmd;

	ghdr = genlmsg_hdr(nlh);

	if (!(cmd = lookup_cmd(ops, ghdr->cmd))) {
		err = -NLE_MSGTYPE_NOSUPPORT;
		goto errout;
	}

	if (cmd->c_msg_parser == NULL)
		err = -NLE_OPNOTSUPP;
	else {
		struct nlattr *tb[cmd->c_maxattr + 1];
		struct genl_info info = {
			.who = who,
			.nlh = nlh,
			.genlhdr = ghdr,
			.userhdr = genlmsg_user_hdr(ghdr),
			.attrs = tb,
		};

		err = nlmsg_parse(nlh, GENL_HDRSIZE(ops->o_hdrsize), tb, cmd->c_maxattr,
				  cmd->c_attr_policy);
		if (err < 0)
			goto errout;

		err = cmd->c_msg_parser(cache_ops, cmd, &info, arg);
	}
errout:
	return err;

}

static int genl_msg_parser(struct nl_cache_ops *ops, struct sockaddr_nl *who,
			   struct nlmsghdr *nlh, struct nl_parser_param *pp)
{
	if (ops->co_genl == NULL)
		BUG();

	return cmd_msg_parser(who, nlh, ops->co_genl, ops, pp);
}

static struct genl_ops *lookup_family(int family)
{
	struct genl_ops *ops;

	nl_list_for_each_entry(ops, &genl_ops_list, o_list) {
		if (ops->o_id == family)
			return ops;
	}

	return NULL;
}

static struct genl_ops *lookup_family_by_name(const char *name)
{
	struct genl_ops *ops;

	nl_list_for_each_entry(ops, &genl_ops_list, o_list) {
		if (!strcmp(ops->o_name, name))
			return ops;
	}

	return NULL;
}

char *genl_op2name(int family, int op, char *buf, size_t len)
{
	struct genl_ops *ops;
	int i;

	if ((ops = lookup_family(family))) {
		for (i = 0; i < ops->o_ncmds; i++) {
			struct genl_cmd *cmd;
			cmd = &ops->o_cmds[i];

			if (cmd->c_id == op) {
				strncpy(buf, cmd->c_name, len - 1);
				return buf;
			}
		}
	}

	strncpy(buf, "unknown", len - 1);
	return NULL;
}

/** @endcond */

/**
 * @name Registration
 * @{
 */

/**
 * Register Generic Netlink family and associated commands
 * @arg ops		Generic Netlink family definition
 *
 * Registers the specified Generic Netlink family definition together with
 * all associated commands. After registration, received Generic Netlink
 * messages can be passed to genl_handle_msg() which will validate the
 * messages, look for a matching command and call the respective callback
 * function automatically.
 *
 * @note Consider using genl_register() if the family is used to implement a
 *       cacheable type.
 *
 * @see genl_unregister_family();
 * @see genl_register();
 *
 * @return 0 on success or a negative error code.
 */
int genl_register_family(struct genl_ops *ops)
{
	if (!ops->o_name)
		return -NLE_INVAL;

	if (ops->o_cmds && ops->o_ncmds <= 0)
		return -NLE_INVAL;

	if (ops->o_id && lookup_family(ops->o_id))
		return -NLE_EXIST;

	if (lookup_family_by_name(ops->o_name))
		return -NLE_EXIST;

	nl_list_add_tail(&ops->o_list, &genl_ops_list);

	return 0;
}

/**
 * Unregister Generic Netlink family
 * @arg ops		Generic Netlink family definition
 *
 * Unregisters a family and all associated commands that were previously
 * registered using genl_register_family().
 *
 * @see genl_register_family()
 *
 * @return 0 on success or a negative error code.
 */
int genl_unregister_family(struct genl_ops *ops)
{
	nl_list_del(&ops->o_list);

	return 0;
}

/**
 * Run a received message through the demultiplexer
 * @arg msg		Generic Netlink message
 * @arg arg		Argument passed on to the message handler callback
 *
 * @return 0 on success or a negative error code.
 */
int genl_handle_msg(struct nl_msg *msg, void *arg)
{
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct genl_ops *ops;

	if (!genlmsg_valid_hdr(nlh, 0))
		return -NLE_INVAL;

	if (!(ops = lookup_family(nlh->nlmsg_type)))
		return -NLE_MSGTYPE_NOSUPPORT;

	return cmd_msg_parser(nlmsg_get_src(msg), nlh, ops, NULL, arg);
}

/** @} */

/**
 * @name Registration of Cache Operations
 * @{
 */

/**
 * Register Generic Netlink family backed cache
 * @arg ops		Cache operations definition
 *
 * Same as genl_register_family() but additionally registers the specified
 * cache operations using nl_cache_mngt_register() and associates it with
 * the Generic Netlink family.
 *
 * @see genl_register_family()
 *
 * @return 0 on success or a negative error code.
 */
int genl_register(struct nl_cache_ops *ops)
{
	int err;

	if (ops->co_protocol != NETLINK_GENERIC) {
		err = -NLE_PROTO_MISMATCH;
		goto errout;
	}

	if (ops->co_hdrsize < GENL_HDRSIZE(0)) {
		err = -NLE_INVAL;
		goto errout;
	}

	if (ops->co_genl == NULL) {
		err = -NLE_INVAL;
		goto errout;
	}

	ops->co_genl->o_cache_ops = ops;
	ops->co_genl->o_hdrsize = ops->co_hdrsize - GENL_HDRLEN;
	ops->co_genl->o_name = ops->co_msgtypes[0].mt_name;
	ops->co_genl->o_id = ops->co_msgtypes[0].mt_id;
	ops->co_msg_parser = genl_msg_parser;

	if ((err = genl_register_family(ops->co_genl)) < 0)
		goto errout;

	err = nl_cache_mngt_register(ops);
errout:
	return err;
}

/**
 * Unregister cache based Generic Netlink family
 * @arg ops		Cache operations definition
 */
void genl_unregister(struct nl_cache_ops *ops)
{
	if (!ops)
		return;

	nl_cache_mngt_unregister(ops);

	genl_unregister_family(ops->co_genl);
}

/** @} */

/** @cond SKIP */
static int __genl_ops_resolve(struct nl_cache *ctrl, struct genl_ops *ops)
{
	struct genl_family *family;

	family = genl_ctrl_search_by_name(ctrl, ops->o_name);
	if (family != NULL) {
		ops->o_id = genl_family_get_id(family);

		if (ops->o_cache_ops)
			ops->o_cache_ops->co_msgtypes[0].mt_id = ops->o_id;

		genl_family_put(family);

		return 0;
	}

	return -NLE_OBJ_NOTFOUND;
}

int genl_resolve_id(struct genl_ops *ops)
{
	struct nl_sock *sk;
	int err = 0;

	/* Check if resolved already */
	if (ops->o_id != GENL_ID_GENERATE)
		return 0;

	if (!ops->o_name)
		return -NLE_INVAL;

	if (!(sk = nl_socket_alloc()))
		return -NLE_NOMEM;

	if ((err = genl_connect(sk)) < 0)
		goto errout_free;

	err = genl_ops_resolve(sk, ops);

errout_free:
	nl_socket_free(sk);

	return err;
}
/** @endcond */

/**
 * @name Resolving the name of registered families
 * @{
 */

/**
 * Resolve a single Generic Netlink family
 * @arg sk		Generic Netlink socket
 * @arg ops		Generic Netlink family definition
 *
 * Resolves the family name to its numeric identifier.
 *
 * @return 0 on success or a negative error code.
 */
int genl_ops_resolve(struct nl_sock *sk, struct genl_ops *ops)
{
	struct nl_cache *ctrl;
	int err;

	if ((err = genl_ctrl_alloc_cache(sk, &ctrl)) < 0)
		goto errout;

	err = __genl_ops_resolve(ctrl, ops);

	nl_cache_free(ctrl);
errout:
	return err;
}

/**
 * Resolve all registered Generic Netlink families
 * @arg sk		Generic Netlink socket
 *
 * Walks through all local Generic Netlink families that have been registered
 * using genl_register() and resolves the name of each family to the
 * corresponding numeric identifier.
 *
 * @see genl_register()
 * @see genl_ops_resolve()
 *
 * @return 0 on success or a negative error code.
 */
int genl_mngt_resolve(struct nl_sock *sk)
{
	struct nl_cache *ctrl;
	struct genl_ops *ops;
	int err = 0;

	if ((err = genl_ctrl_alloc_cache(sk, &ctrl)) < 0)
		goto errout;

	nl_list_for_each_entry(ops, &genl_ops_list, o_list) {
		err = __genl_ops_resolve(ctrl, ops);
	}

	nl_cache_free(ctrl);
errout:
	return err;
}

/** @} */

/** @} */

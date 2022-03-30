/*
 * netlink/genl/mngt.h		Generic Netlink Management
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2012 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_GENL_MNGT_H_
#define NETLINK_GENL_MNGT_H_

#include <netlink/netlink.h>
#include <netlink/attr.h>
#include <netlink/list.h>

#ifdef __cplusplus
extern "C" {
#endif

struct nl_cache_ops;

/**
 * @ingroup genl_mngt
 * @struct genl_info netlink/genl/mngt.h
 *
 * Informative structure passed on to message parser callbacks
 *
 * This structure is passed on to all message parser callbacks and contains
 * information about the sender of the message as well as pointers to all
 * relevant sections of the parsed message.
 *
 * @see genl_cmd::c_msg_parser
 */
struct genl_info
{
	/** Socket address of sender */
	struct sockaddr_nl *    who;

	/** Pointer to Netlink message header */
	struct nlmsghdr *       nlh;

	/** Pointer to Generic Netlink message header */
	struct genlmsghdr *     genlhdr;

	/** Pointer to user header */
	void *                  userhdr;

	/** Pointer to array of parsed attributes */
	struct nlattr **        attrs;
};

/**
 * @ingroup genl_mngt
 * @struct genl_cmd netlink/genl/mngt.h
 *
 * Definition of a Generic Netlink command.
 *
 * This structure is used to define the list of available commands on the
 * receiving side.
 *
 * @par Example:
 * @code
 * static struct genl_cmd foo_cmds[] = {
 * 	{
 * 		.c_id		= FOO_CMD_NEW,
 * 		.c_name		= "NEWFOO" ,
 * 		.c_maxattr	= FOO_ATTR_MAX,
 * 		.c_attr_policy	= foo_policy,
 * 		.c_msg_parser	= foo_msg_parser,
 * 	},
 * 	{
 * 		.c_id		= FOO_CMD_DEL,
 * 		.c_name		= "DELFOO" ,
 * 	},
 * };
 *
 * static struct genl_ops my_genl_ops = {
 * 	[...]
 * 	.o_cmds			= foo_cmds,
 * 	.o_ncmds		= ARRAY_SIZE(foo_cmds),
 * };
 * @endcode
 */
struct genl_cmd
{
	/** Numeric command identifier (required) */
	int			c_id;

	/** Human readable name  (required) */
	char *			c_name;

	/** Maximum attribute identifier that the command is prepared to handle. */
	int			c_maxattr;

	/** Called whenever a message for this command is received */
	int		      (*c_msg_parser)(struct nl_cache_ops *,
					      struct genl_cmd *,
					      struct genl_info *, void *);

	/** Attribute validation policy, enforced before the callback is called */
	struct nla_policy *	c_attr_policy;
};

/**
 * @ingroup genl_mngt
 * @struct genl_ops netlink/genl/mngt.h
 *
 * Definition of a Generic Netlink family
 *
 * @par Example:
 * @code
 * static struct genl_cmd foo_cmds[] = {
 * 	[...]
 * };
 *
 * static struct genl_ops my_genl_ops = {
 * 	.o_name			= "foo",
 * 	.o_hdrsize		= sizeof(struct my_hdr),
 * 	.o_cmds			= foo_cmds,
 * 	.o_ncmds		= ARRAY_SIZE(foo_cmds),
 * };
 *
 * if ((err = genl_register_family(&my_genl_ops)) < 0)
 * 	// ERROR
 * @endcode
 *
 * @see genl_cmd
 */
struct genl_ops
{
	/** Length of user header */
	unsigned int		o_hdrsize;

	/** Numeric identifier, automatically filled in by genl_ops_resolve() */
	int			o_id;

	/** Human readable name, used by genl_ops_resolve() to resolve numeric id */
	char *			o_name;

	/**
	 * If registered via genl_register(), will point to the related
	 * cache operations.
	 */
	struct nl_cache_ops *	o_cache_ops;

	/** Optional array defining the available Generic Netlink commands */
	struct genl_cmd	*	o_cmds;

	/** Number of elements in \c o_cmds array */
	int			o_ncmds;

	/**
	 * @private
	 * Used internally to link together all registered operations.
	 */
	struct nl_list_head	o_list;
};

extern int		genl_register_family(struct genl_ops *);
extern int		genl_unregister_family(struct genl_ops *);
extern int		genl_handle_msg(struct nl_msg *, void *);

extern int		genl_register(struct nl_cache_ops *);
extern void		genl_unregister(struct nl_cache_ops *);

extern int		genl_ops_resolve(struct nl_sock *, struct genl_ops *);
extern int		genl_mngt_resolve(struct nl_sock *);

#ifdef __cplusplus
}
#endif

#endif

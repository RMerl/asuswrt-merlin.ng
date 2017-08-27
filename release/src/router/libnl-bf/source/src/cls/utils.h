/*
 * src/cls-utils.h     Classifier Helpers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation version 2 of the License.
 *
 * Copyright (c) 2008-2009 Thomas Graf <tgraf@suug.ch>
 */

#ifndef __CLS_UTILS_H_
#define __CLS_UTILS_H_

#include "../utils.h"
#include <netlink/route/classifier-modules.h>
#include <netlink/route/cls/ematch.h>

struct cls_module
{
	const char *		name;
	struct rtnl_cls_ops *	ops;
	void		      (*parse_argv)(struct rtnl_cls *, int, char **);
	struct nl_list_head	list;
};

extern struct cls_module *lookup_cls_mod(struct rtnl_cls_ops *);
extern void register_cls_module(struct cls_module *);
extern void unregister_cls_module(struct cls_module *);

struct ematch_module
{
	int kind;
	struct rtnl_ematch_ops *ops;
	void (*parse_argv)(struct rtnl_ematch *, int, char **);
	struct nl_list_head list;
};

extern struct ematch_module *lookup_ematch_mod(struct rtnl_ematch_ops *);
extern void register_ematch_module(struct ematch_module *);
extern void unregister_ematch_module(struct ematch_module *);

extern struct rtnl_cls *nlt_alloc_cls(void);
extern void parse_dev(struct rtnl_cls *, struct nl_cache *, char *);
extern void parse_prio(struct rtnl_cls *, char *);
extern void parse_parent(struct rtnl_cls *, char *);
extern void parse_handle(struct rtnl_cls *, char *);
extern void parse_proto(struct rtnl_cls *, char *);

extern int parse_ematch_syntax(const char *, struct rtnl_ematch_tree **);

#endif

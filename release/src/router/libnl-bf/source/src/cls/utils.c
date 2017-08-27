/*
 * src/cls-utils.c     Classifier Helpers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License as
 *	published by the Free Software Foundation version 2 of the License.
 *
 * Copyright (c) 2008-2009 Thomas Graf <tgraf@suug.ch>
 */

#include "utils.h"

struct rtnl_cls *nlt_alloc_cls(void)
{
	struct rtnl_cls *cls;

	cls = rtnl_cls_alloc();
	if (!cls)
		fatal(ENOMEM, "Unable to allocate classifier object");

	return cls;
}

void parse_dev(struct rtnl_cls *cls, struct nl_cache *link_cache, char *arg)
{
	int ival;

	if (!(ival = rtnl_link_name2i(link_cache, arg)))
		fatal(ENOENT, "Link \"%s\" does not exist", arg);

	rtnl_cls_set_ifindex(cls, ival);
}
  
void parse_prio(struct rtnl_cls *cls, char *arg)
{
	uint32_t prio = parse_u32(arg);
	rtnl_cls_set_prio(cls, prio);
}

void parse_parent(struct rtnl_cls *cls, char *arg)
{
	uint32_t parent;
	int err;

	if ((err = rtnl_tc_str2handle(arg, &parent)) < 0)
		fatal(err, "Unable to parse handle \"%s\": %s",
		      arg, nl_geterror(err));

	rtnl_cls_set_parent(cls, parent);
}

void parse_handle(struct rtnl_cls *cls, char *arg)
{
	uint32_t handle;
	int err;

	if ((err = rtnl_tc_str2handle(arg, &handle)) < 0)
		fatal(err, "Unable to parse handle \"%s\": %s",
		      arg, nl_geterror(err));

	rtnl_cls_set_handle(cls, handle);
}

void parse_proto(struct rtnl_cls *cls, char *arg)
{
	int proto = nl_str2ether_proto(arg);
	if (proto < 0)
		fatal(proto, "Unable to parse protocol \"%s\": %s",
		      arg, nl_geterror(proto));
	rtnl_cls_set_protocol(cls, proto);
}

static NL_LIST_HEAD(cls_modules);

struct cls_module *lookup_cls_mod(struct rtnl_cls_ops *ops)
{
	struct cls_module *mod;

	nl_list_for_each_entry(mod, &cls_modules, list) {
		if (mod->ops == ops)
			return mod;
	}

	return NULL;
}

void register_cls_module(struct cls_module *mod)
{
	struct rtnl_cls_ops *ops;

	if (!(ops = __rtnl_cls_lookup_ops(mod->name)))
		fatal(ENOENT, "Could not locate classifier module \"%s\"",
			mod->name);

	if (lookup_cls_mod(ops) != NULL)
		fatal(EEXIST, "Duplicate classifier module registration.");

	mod->ops = ops;
	nl_list_add_tail(&mod->list, &cls_modules);
}

void unregister_cls_module(struct cls_module *mod)
{
	nl_list_del(&mod->list);
}

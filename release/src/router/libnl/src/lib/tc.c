/*
 * src/lib/tc.c     CLI Traffic Control Helpers
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink/cli/utils.h>
#include <netlink/cli/tc.h>
#include <netlink-private/route/tc-api.h>

/**
 * @ingroup cli
 * @defgroup cli_tc Traffic Control
 * @{
 */
void nl_cli_tc_parse_dev(struct rtnl_tc *tc, struct nl_cache *link_cache, char *name)
{
	struct rtnl_link *link;
	
	link = rtnl_link_get_by_name(link_cache, name);
	if (!link)
		nl_cli_fatal(ENOENT, "Link \"%s\" does not exist.", name);

	rtnl_tc_set_link(tc, link);
	rtnl_link_put(link);
}

void nl_cli_tc_parse_parent(struct rtnl_tc *tc, char *arg)
{
	uint32_t parent;
	int err;

	if ((err = rtnl_tc_str2handle(arg, &parent)) < 0)
		nl_cli_fatal(err, "Unable to parse handle \"%s\": %s",
		      arg, nl_geterror(err));

	rtnl_tc_set_parent(tc, parent);
}

void nl_cli_tc_parse_handle(struct rtnl_tc *tc, char *arg, int create)
{
	uint32_t handle, parent;
	int err;

	parent = rtnl_tc_get_parent(tc);

	if ((err = rtnl_tc_str2handle(arg, &handle)) < 0) {
		if (err == -NLE_OBJ_NOTFOUND && create)
			err = rtnl_classid_generate(arg, &handle, parent);

		if (err < 0)
			nl_cli_fatal(err, "Unable to parse handle \"%s\": %s",
				     arg, nl_geterror(err));
	}

	rtnl_tc_set_handle(tc, handle);
}

void nl_cli_tc_parse_mtu(struct rtnl_tc *tc, char *arg)
{
	rtnl_tc_set_mtu(tc, nl_cli_parse_u32(arg));
}

void nl_cli_tc_parse_mpu(struct rtnl_tc *tc, char *arg)
{
	rtnl_tc_set_mpu(tc, nl_cli_parse_u32(arg));
}

void nl_cli_tc_parse_overhead(struct rtnl_tc *tc, char *arg)
{
	rtnl_tc_set_overhead(tc, nl_cli_parse_u32(arg));
}

void nl_cli_tc_parse_kind(struct rtnl_tc *tc, char *arg)
{
	rtnl_tc_set_kind(tc, arg);
}

void nl_cli_tc_parse_linktype(struct rtnl_tc *tc, char *arg)
{
	int type;

	if ((type = nl_str2llproto(arg)) < 0)
		nl_cli_fatal(type, "Unable to parse linktype \"%s\": %s",
			arg, nl_geterror(type));

	rtnl_tc_set_linktype(tc, type);
}

static NL_LIST_HEAD(tc_modules);

static struct nl_cli_tc_module *__nl_cli_tc_lookup(struct rtnl_tc_ops *ops)
{
	struct nl_cli_tc_module *tm;

	nl_list_for_each_entry(tm, &tc_modules, tm_list)
		if (tm->tm_ops == ops)
			return tm;

	return NULL;
}

struct nl_cli_tc_module *nl_cli_tc_lookup(struct rtnl_tc_ops *ops)
{
	struct nl_cli_tc_module *tm;

	if ((tm = __nl_cli_tc_lookup(ops)))
		return tm;

	switch (ops->to_type) {
	case RTNL_TC_TYPE_QDISC:
	case RTNL_TC_TYPE_CLASS:
		nl_cli_load_module("cli/qdisc", ops->to_kind);
		break;

	case RTNL_TC_TYPE_CLS:
		nl_cli_load_module("cli/cls", ops->to_kind);
		break;

	default:
		nl_cli_fatal(EINVAL, "BUG: unhandled TC object type %d",
				ops->to_type);
	}

	if (!(tm = __nl_cli_tc_lookup(ops)))  {
		nl_cli_fatal(EINVAL, "Application bug: The shared library for "
			"the tc object \"%s\" was successfully loaded but it "
			"seems that module did not register itself",
			ops->to_kind);
	}

	return tm;
}

void nl_cli_tc_register(struct nl_cli_tc_module *tm)
{
	struct rtnl_tc_ops *ops;

	if (!(ops = rtnl_tc_lookup_ops(tm->tm_type, tm->tm_name))) {
		nl_cli_fatal(ENOENT, "Unable to register CLI TC module "
		"\"%s\": No matching libnl TC module found.", tm->tm_name);
	}

	if (__nl_cli_tc_lookup(ops)) {
		nl_cli_fatal(EEXIST, "Unable to register CLI TC module "
		"\"%s\": Module already registered.", tm->tm_name);
	}

	tm->tm_ops = ops;

	nl_list_add_tail(&tm->tm_list, &tc_modules);
}

void nl_cli_tc_unregister(struct nl_cli_tc_module *tm)
{
	nl_list_del(&tm->tm_list);
}


/** @} */

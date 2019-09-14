/*
 * lib/route/route_utils.c	Routing Utilities
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup route
 * @defgroup route_utils Utilities
 * @brief Routing Utility Functions
 *
 *
 * @par 1) Translating Routing Table Names
 * @code
 * // libnl is only aware of the de facto standard routing table names.
 * // Additional name <-> identifier associations have to be read in via
 * // a configuration file, f.e. /etc/iproute2/rt_tables
 * err = rtnl_route_read_table_names("/etc/iproute2/rt_tables");
 *
 * // Translating a table name to its idenfier
 * int table = rtnl_route_str2table("main");
 *
 * // ... and the other way around.
 * char buf[32];
 * printf("Name: %s\n",
 *        rtnl_route_table2str(table, buf, sizeof(buf)));
 * @endcode
 *
 *
 *
 *
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/route.h>
	
/**
 * @name Routing Table Identifier Translations
 * @{
 */

static NL_LIST_HEAD(table_names);

static int add_routing_table_name(long id, const char *name)
{
	return __trans_list_add(id, name, &table_names);
}

static void __init init_routing_table_names(void)
{
	add_routing_table_name(RT_TABLE_UNSPEC, "unspec");
	add_routing_table_name(RT_TABLE_COMPAT, "compat");
	add_routing_table_name(RT_TABLE_DEFAULT, "default");
	add_routing_table_name(RT_TABLE_MAIN, "main");
	add_routing_table_name(RT_TABLE_LOCAL, "local");
};

static void __exit release_routing_table_names(void)
{
	__trans_list_clear(&table_names);
}

int rtnl_route_read_table_names(const char *path)
{
	__trans_list_clear(&table_names);

	return __nl_read_num_str_file(path, &add_routing_table_name);
}

char *rtnl_route_table2str(int table, char *buf, size_t size)
{
	return __list_type2str(table, buf, size, &table_names);
}

int rtnl_route_str2table(const char *name)
{
	return __list_str2type(name, &table_names);
}


/** @} */

/**
 * @name Routing Protocol Translations
 * @{
 */

static NL_LIST_HEAD(proto_names);

static int add_proto_name(long id, const char *name)
{
	return __trans_list_add(id, name, &proto_names);
}

static void __init init_proto_names(void)
{
	add_proto_name(RTPROT_UNSPEC, "unspec");
	add_proto_name(RTPROT_REDIRECT, "redirect");
	add_proto_name(RTPROT_KERNEL, "kernel");
	add_proto_name(RTPROT_BOOT, "boot");
	add_proto_name(RTPROT_STATIC, "static");
};

static void __exit release_proto_names(void)
{
	__trans_list_clear(&proto_names);
}

int rtnl_route_read_protocol_names(const char *path)
{
	__trans_list_clear(&proto_names);

	return __nl_read_num_str_file(path, &add_proto_name);
}

char *rtnl_route_proto2str(int proto, char *buf, size_t size)
{
	return __list_type2str(proto, buf, size, &proto_names);
}

int rtnl_route_str2proto(const char *name)
{
	return __list_str2type(name, &proto_names);
}

/** @} */

/**
 * @name Routing Metrices Translations
 * @{
 */

static const struct trans_tbl route_metrices[] = {
	__ADD(RTAX_UNSPEC, unspec)
	__ADD(RTAX_LOCK, lock)
	__ADD(RTAX_MTU, mtu)
	__ADD(RTAX_WINDOW, window)
	__ADD(RTAX_RTT, rtt)
	__ADD(RTAX_RTTVAR, rttvar)
	__ADD(RTAX_SSTHRESH, ssthresh)
	__ADD(RTAX_CWND, cwnd)
	__ADD(RTAX_ADVMSS, advmss)
	__ADD(RTAX_REORDERING, reordering)
	__ADD(RTAX_HOPLIMIT, hoplimit)
	__ADD(RTAX_INITCWND, initcwnd)
	__ADD(RTAX_FEATURES, features)
};

char *rtnl_route_metric2str(int metric, char *buf, size_t size)
{
	return __type2str(metric, buf, size, route_metrices,
			  ARRAY_SIZE(route_metrices));
}

int rtnl_route_str2metric(const char *name)
{
	return __str2type(name, route_metrices, ARRAY_SIZE(route_metrices));
}

/** @} */

/** @} */

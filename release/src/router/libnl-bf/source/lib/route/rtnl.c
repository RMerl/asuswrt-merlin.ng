/*
 * lib/route/rtnl.c		Routing Netlink
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @defgroup rtnl Routing Family
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/rtnl.h>

/**
 * @name Sending
 * @{
 */

/**
 * Send routing netlink request message
 * @arg sk		Netlink socket.
 * @arg type		Netlink message type.
 * @arg family		Address family.
 * @arg flags		Additional netlink message flags.
 *
 * Fills out a routing netlink request message and sends it out
 * using nl_send_simple().
 *
 * @return 0 on success or a negative error code.
 */
int nl_rtgen_request(struct nl_sock *sk, int type, int family, int flags)
{
	struct rtgenmsg gmsg = {
		.rtgen_family = family,
	};

	return nl_send_simple(sk, type, flags, &gmsg, sizeof(gmsg));
}

/** @} */

/**
 * @name Routing Type Translations
 * @{
 */

static const struct trans_tbl rtntypes[] = {
	__ADD(RTN_UNSPEC,unspec)
	__ADD(RTN_UNICAST,unicast)
	__ADD(RTN_LOCAL,local)
	__ADD(RTN_BROADCAST,broadcast)
	__ADD(RTN_ANYCAST,anycast)
	__ADD(RTN_MULTICAST,multicast)
	__ADD(RTN_BLACKHOLE,blackhole)
	__ADD(RTN_UNREACHABLE,unreachable)
	__ADD(RTN_PROHIBIT,prohibit)
	__ADD(RTN_THROW,throw)
	__ADD(RTN_NAT,nat)
	__ADD(RTN_XRESOLVE,xresolve)
};

char *nl_rtntype2str(int type, char *buf, size_t size)
{
	return __type2str(type, buf, size, rtntypes, ARRAY_SIZE(rtntypes));
}

int nl_str2rtntype(const char *name)
{
	return __str2type(name, rtntypes, ARRAY_SIZE(rtntypes));
}

/** @} */

/**
 * @name Scope Translations
 * @{
 */

static const struct trans_tbl scopes[] = {
	__ADD(255,nowhere)
	__ADD(254,host)
	__ADD(253,link)
	__ADD(200,site)
	__ADD(0,global)
};

char *rtnl_scope2str(int scope, char *buf, size_t size)
{
	return __type2str(scope, buf, size, scopes, ARRAY_SIZE(scopes));
}

int rtnl_str2scope(const char *name)
{
	return __str2type(name, scopes, ARRAY_SIZE(scopes));
}

/** @} */

/**
 * @name Realms Translations
 * @{
 */

char * rtnl_realms2str(uint32_t realms, char *buf, size_t len)
{
	int from = RTNL_REALM_FROM(realms);
	int to = RTNL_REALM_TO(realms);

	snprintf(buf, len, "%d/%d", from, to);

	return buf;
}

/** @} */

/** @} */

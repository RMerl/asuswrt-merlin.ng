/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012
 * Joe Hershberger, National Instruments, joe.hershberger@ni.com
 */

#ifndef __ENV_CALLBACK_H__
#define __ENV_CALLBACK_H__

#include <env_flags.h>
#include <linker_lists.h>
#include <search.h>

#define ENV_CALLBACK_VAR ".callbacks"

/* Board configs can define additional static callback bindings */
#ifndef CFG_ENV_CALLBACK_LIST_STATIC
#define CFG_ENV_CALLBACK_LIST_STATIC
#endif

#ifdef CONFIG_SILENT_CONSOLE
#define SILENT_CALLBACK "silent:silent,"
#else
#define SILENT_CALLBACK
#endif

#ifdef CONFIG_REGEX
#define ENV_DOT_ESCAPE "\\"
#else
#define ENV_DOT_ESCAPE
#endif

#ifdef CONFIG_CMD_DNS
#define DNS_CALLBACK "dnsip:dnsip,"
#else
#define DNS_CALLBACK
#endif

#ifdef CONFIG_NET
#define NET_CALLBACKS \
	"bootfile:bootfile," \
	"ipaddr:ipaddr," \
	"gatewayip:gatewayip," \
	"netmask:netmask," \
	"serverip:serverip," \
	"nvlan:nvlan," \
	"vlan:vlan," \
	DNS_CALLBACK \
	"eth" ETHADDR_WILDCARD "addr:ethaddr,"
#else
#define NET_CALLBACKS
#endif

#ifdef CONFIG_IPV6
#define NET6_CALLBACKS \
	"ip6addr:ip6addr," \
	"serverip6:serverip6," \
	"gatewayip6:gatewayip6,"
#else
#define NET6_CALLBACKS
#endif

#ifdef CONFIG_BOOTSTD_FULL
#define BOOTSTD_CALLBACK \
	"bootmeths:bootmeths," \
	"bootargs:bootargs,"
#else
#define BOOTSTD_CALLBACK
#endif

/*
 * This list of callback bindings is static, but may be overridden by defining
 * a new association in the ".callbacks" environment variable.
 */
#define ENV_CALLBACK_LIST_STATIC ENV_DOT_ESCAPE ENV_CALLBACK_VAR ":callbacks," \
	ENV_DOT_ESCAPE ENV_FLAGS_VAR ":flags," \
	"baudrate:baudrate," \
	NET_CALLBACKS \
	NET6_CALLBACKS \
	BOOTSTD_CALLBACK \
	"loadaddr:loadaddr," \
	SILENT_CALLBACK \
	"stdin:console,stdout:console,stderr:console," \
	"serial#:serialno," \
	CFG_ENV_CALLBACK_LIST_STATIC

#ifndef CONFIG_SPL_BUILD
void env_callback_init(struct env_entry *var_entry);
#else
static inline void env_callback_init(struct env_entry *var_entry)
{
}
#endif

#endif /* __ENV_CALLBACK_H__ */

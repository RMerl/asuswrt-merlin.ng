/*
 * lib/netfilter/netfilter.c    Netfilter Generic Functions
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008 Patrick McHardy <kaber@trash.net>
 */

#include <netlink-private/netlink.h>
#include <netlink/netfilter/netfilter.h>
#include <linux/netfilter.h>

static const struct trans_tbl nfnl_verdicts[] = {
	__ADD(NF_DROP,		NF_DROP)
	__ADD(NF_ACCEPT,	NF_ACCEPT)
	__ADD(NF_STOLEN,	NF_STOLEN)
	__ADD(NF_QUEUE,		NF_QUEUE)
	__ADD(NF_REPEAT,	NF_REPEAT)
	__ADD(NF_STOP,		NF_STOP)
};

char *nfnl_verdict2str(unsigned int verdict, char *buf, size_t len)
{
	return __type2str(verdict, buf, len, nfnl_verdicts,
			  ARRAY_SIZE(nfnl_verdicts));
}

unsigned int nfnl_str2verdict(const char *name)
{
	return __str2type(name, nfnl_verdicts, ARRAY_SIZE(nfnl_verdicts));
}

static const struct trans_tbl nfnl_inet_hooks[] = {
	__ADD(NF_INET_PRE_ROUTING,	NF_INET_PREROUTING)
	__ADD(NF_INET_LOCAL_IN,		NF_INET_LOCAL_IN)
	__ADD(NF_INET_FORWARD,		NF_INET_FORWARD)
	__ADD(NF_INET_LOCAL_OUT,	NF_INET_LOCAL_OUT)
	__ADD(NF_INET_POST_ROUTING,	NF_INET_POST_ROUTING)
};

char *nfnl_inet_hook2str(unsigned int hook, char *buf, size_t len)
{
	return __type2str(hook, buf, len, nfnl_inet_hooks,
			  ARRAY_SIZE(nfnl_inet_hooks));
}

unsigned int nfnl_str2inet_hook(const char *name)
{
	return __str2type(name, nfnl_inet_hooks, ARRAY_SIZE(nfnl_inet_hooks));
}

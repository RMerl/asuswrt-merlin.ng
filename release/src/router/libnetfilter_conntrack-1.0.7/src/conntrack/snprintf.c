/*
 * (C) 2005-2011 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include "internal/internal.h"

/* these arrays are used by snprintf_default.c and snprintf_xml.c */
const char *const l3proto2str[AF_MAX] = {
	[AF_INET]			= "ipv4",
	[AF_INET6]			= "ipv6",
};

const char *const proto2str[IPPROTO_MAX] = {
	[IPPROTO_TCP]			= "tcp",
	[IPPROTO_UDP]			= "udp",
	[IPPROTO_UDPLITE]		= "udplite",
	[IPPROTO_ICMP]			= "icmp",
	[IPPROTO_ICMPV6]		= "icmpv6",
	[IPPROTO_SCTP]			= "sctp",
	[IPPROTO_GRE]			= "gre",
	[IPPROTO_DCCP]			= "dccp",
};

const char *const states[TCP_CONNTRACK_MAX] = {
	[TCP_CONNTRACK_NONE]		= "NONE",
	[TCP_CONNTRACK_SYN_SENT]	= "SYN_SENT",
	[TCP_CONNTRACK_SYN_RECV]	= "SYN_RECV",
	[TCP_CONNTRACK_ESTABLISHED]	= "ESTABLISHED",
	[TCP_CONNTRACK_FIN_WAIT]	= "FIN_WAIT",
	[TCP_CONNTRACK_CLOSE_WAIT]	= "CLOSE_WAIT",
	[TCP_CONNTRACK_LAST_ACK]	= "LAST_ACK",
	[TCP_CONNTRACK_TIME_WAIT]	= "TIME_WAIT",
	[TCP_CONNTRACK_CLOSE]		= "CLOSE",
	[TCP_CONNTRACK_SYN_SENT2]	= "SYN_SENT2",
};

const char *const sctp_states[SCTP_CONNTRACK_MAX] = {
	[SCTP_CONNTRACK_NONE]		= "NONE",
	[SCTP_CONNTRACK_CLOSED]		= "CLOSED",
	[SCTP_CONNTRACK_COOKIE_WAIT]	= "COOKIE_WAIT",
	[SCTP_CONNTRACK_COOKIE_ECHOED]	= "COOKIE_ECHOED",
	[SCTP_CONNTRACK_ESTABLISHED]	= "ESTABLISHED",
	[SCTP_CONNTRACK_SHUTDOWN_SENT]	= "SHUTDOWN_SENT",
	[SCTP_CONNTRACK_SHUTDOWN_RECD]	= "SHUTDOWN_RECD",
	[SCTP_CONNTRACK_SHUTDOWN_ACK_SENT] = "SHUTDOWN_ACK_SENT",
};

const char *const dccp_states[DCCP_CONNTRACK_MAX] = {
	[DCCP_CONNTRACK_NONE]		= "NONE",
	[DCCP_CONNTRACK_REQUEST]	= "REQUEST",
	[DCCP_CONNTRACK_RESPOND]	= "RESPOND",
	[DCCP_CONNTRACK_PARTOPEN]	= "PARTOPEN",
	[DCCP_CONNTRACK_OPEN]		= "OPEN",
	[DCCP_CONNTRACK_CLOSEREQ]	= "CLOSEREQ",
	[DCCP_CONNTRACK_CLOSING]	= "CLOSING",
	[DCCP_CONNTRACK_TIMEWAIT]	= "TIMEWAIT",
	[DCCP_CONNTRACK_IGNORE]		= "IGNORE",
	[DCCP_CONNTRACK_INVALID]	= "INVALID",
};

int __snprintf_conntrack(char *buf,
			 unsigned int len,
			 const struct nf_conntrack *ct,
			 unsigned int type,
			 unsigned int msg_output,
			 unsigned int flags,
			 struct nfct_labelmap *map)
{
	int size;

	switch(msg_output) {
	case NFCT_O_DEFAULT:
		size = __snprintf_conntrack_default(buf, len, ct, type, flags, map);
		break;
	case NFCT_O_XML:
		size = __snprintf_conntrack_xml(buf, len, ct, type, flags, map);
		break;
	default:
		errno = ENOENT;
		return -1;
	}

	/* NULL terminated string */
	buf[size+1 > len ? len-1 : size] = '\0';

	return size;
}

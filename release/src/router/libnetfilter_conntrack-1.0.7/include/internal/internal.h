/*
 * WARNING: Do *NOT* ever include this file, only for internal use!
 * 	    Use the set/get API in order to set/get the conntrack attributes
 */

#ifndef __LIBNETFILTER_CONNTRACK_INTERNAL__
#define __LIBNETFILTER_CONNTRACK_INTERNAL__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include <netinet/in.h>

#include <libnfnetlink/libnfnetlink.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_tcp.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_sctp.h>
#include <libnetfilter_conntrack/libnetfilter_conntrack_dccp.h>

#include "internal/object.h"
#include "internal/prototypes.h"
#include "internal/types.h"
#include "internal/extern.h"
#include "internal/bitops.h"

#ifndef IPPROTO_SCTP
#define IPPROTO_SCTP 132
#endif

#ifndef IPPROTO_UDPLITE
#define IPPROTO_UDPLITE 136
#endif

#ifndef IPPROTO_DCCP
#define IPPROTO_DCCP 33
#endif

#define BUFFER_SIZE(ret, size, len, offset)		\
	size += ret;					\
	if (ret > len)					\
		ret = len;				\
	offset += ret;					\
	len -= ret;

#define TS_ORIG								\
({									\
	((1 << ATTR_ORIG_IPV4_SRC) | (1 << ATTR_ORIG_IPV4_DST) |	\
	 (1 << ATTR_ORIG_IPV6_SRC) | (1 << ATTR_ORIG_IPV6_DST) |	\
	 (1 << ATTR_ORIG_PORT_SRC) | (1 << ATTR_ORIG_PORT_DST) | 	\
	 (1 << ATTR_ORIG_L3PROTO)  | (1 << ATTR_ORIG_L4PROTO)  | 	\
	 (1 << ATTR_ICMP_TYPE)	   | (1 << ATTR_ICMP_CODE)     | 	\
	 (1 << ATTR_ICMP_ID));						\
})

#define TS_REPL								\
({									\
	((1 << ATTR_REPL_IPV4_SRC) | (1 << ATTR_REPL_IPV4_DST) | 	\
	 (1 << ATTR_REPL_IPV6_SRC) | (1 << ATTR_REPL_IPV6_DST) | 	\
	 (1 << ATTR_REPL_PORT_SRC) | (1 << ATTR_REPL_PORT_DST) | 	\
	 (1 << ATTR_REPL_L3PROTO)  | (1 << ATTR_REPL_L4PROTO)  |	\
	 (1 << ATTR_ICMP_TYPE)	   | (1 << ATTR_ICMP_CODE)     | 	\
	 (1 << ATTR_ICMP_ID));						\
})

#define TUPLE_SET(dir) (dir == __DIR_ORIG ? TS_ORIG : TS_REPL)

#define likely(x)       __builtin_expect((x),1)
#define unlikely(x)     __builtin_expect((x),0)

#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC    1000000000L
#endif

/* extracted from include/linux/netfilter/nf_conntrack_tcp.h .*/
struct nf_ct_tcp_flags {
	uint8_t flags;
	uint8_t mask;
};

#define NFCT_BITMASK_AND	0
#define NFCT_BITMASK_OR		1

#endif

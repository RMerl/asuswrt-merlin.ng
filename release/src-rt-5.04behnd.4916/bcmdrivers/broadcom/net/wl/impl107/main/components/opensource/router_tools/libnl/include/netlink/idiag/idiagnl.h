/*
 * netlink/idiag/idiagnl.h		Inetdiag Netlink
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2013 Sassano Systems LLC <joe@sassanosystems.com>
 */

#ifndef NETLINK_IDIAGNL_H_
#define NETLINK_IDIAGNL_H_

#include <netlink/netlink.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Inet Diag message types
 */
#define IDIAG_TCPDIAG_GETSOCK	18
#define IDIAG_DCCPDIAG_GETSOCK	19
#define IDIAG_GETSOCK_MAX	24

/**
 * Socket state identifiers
 * @ingroup idiag
 */
enum {
	IDIAG_SS_UNKNOWN,
	IDIAG_SS_ESTABLISHED,
	IDIAG_SS_SYN_SENT,
	IDIAG_SS_SYN_RECV,
	IDIAG_SS_FIN_WAIT1,
	IDIAG_SS_FIN_WAIT2,
	IDIAG_SS_TIME_WAIT,
	IDIAG_SS_CLOSE,
	IDIAG_SS_CLOSE_WAIT,
	IDIAG_SS_LAST_ACK,
	IDIAG_SS_LISTEN,
	IDIAG_SS_CLOSING,
	IDIAG_SS_MAX
};

/**
 * Macro to represent all socket states.
 * @ingroup idiag
 */
#define IDIAG_SS_ALL ((1<<IDIAG_SS_MAX)-1)

/**
 * Inet Diag extended attributes
 * @ingroup idiag
 */
enum {
	IDIAG_ATTR_NONE,
	IDIAG_ATTR_MEMINFO,
	IDIAG_ATTR_INFO,
	IDIAG_ATTR_VEGASINFO,
	IDIAG_ATTR_CONG,
	IDIAG_ATTR_TOS,
	IDIAG_ATTR_TCLASS,
	IDIAG_ATTR_SKMEMINFO,
	IDIAG_ATTR_SHUTDOWN,
	IDIAG_ATTR_MAX,
};

/**
 * Macro to represent all socket attributes.
 * @ingroup idiag
 */
#define IDIAG_ATTR_ALL ((1<<IDIAG_ATTR_MAX)-1)

/**
 * Socket memory info identifiers
 * @ingroup idiag
 */
enum {
	IDIAG_SK_MEMINFO_RMEM_ALLOC,
	IDIAG_SK_MEMINFO_RCVBUF,
	IDIAG_SK_MEMINFO_WMEM_ALLOC,
	IDIAG_SK_MEMINFO_SNDBUF,
	IDIAG_SK_MEMINFO_FWD_ALLOC,
	IDIAG_SK_MEMINFO_WMEM_QUEUED,
	IDIAG_SK_MEMINFO_OPTMEM,
	IDIAG_SK_MEMINFO_BACKLOG,

	IDIAG_SK_MEMINFO_VARS,
};

/**
 * Socket timer indentifiers
 * @ingroupd idiag
 */
enum {
	IDIAG_TIMER_OFF,
	IDIAG_TIMER_ON,
	IDIAG_TIMER_KEEPALIVE,
	IDIAG_TIMER_TIMEWAIT,
	IDIAG_TIMER_PERSIST,
	IDIAG_TIMER_UNKNOWN,
};

extern char *	idiagnl_state2str(int, char *, size_t);
extern int	idiagnl_str2state(const char *);

extern int	idiagnl_connect(struct nl_sock *);
extern int	idiagnl_send_simple(struct nl_sock *, int, uint8_t, uint16_t,
                                    uint16_t);

extern char *		idiagnl_timer2str(int, char *, size_t);
extern int		idiagnl_str2timer(const char *);
extern char *		idiagnl_attrs2str(int, char *, size_t);
extern char *		idiagnl_tcpstate2str(uint8_t, char *, size_t);
extern char *		idiagnl_tcpopts2str(uint8_t, char *, size_t);
extern char *		idiagnl_shutdown2str(uint8_t, char *, size_t);
extern char *		idiagnl_exts2str(uint8_t, char *, size_t);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NETLINK_IDIAGNL_H_ */

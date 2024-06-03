/*
 * netlink/utils.h		Utility Functions
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2012 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_UTILS_H_
#define NETLINK_UTILS_H_

#include <netlink/netlink.h>
#include <netlink/list.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name Probability Constants
 * @{
 */

/**
 * Lower probability limit
 * @ingroup utils
 */
#define NL_PROB_MIN 0x0

/**
 * Upper probability limit nl_dump_type
 * @ingroup utils
 */
#define NL_PROB_MAX 0xffffffff

/** @} */

enum {
	NL_BYTE_RATE,
	NL_BIT_RATE,
};

/* unit pretty-printing */
extern double	nl_cancel_down_bytes(unsigned long long, char **);
extern double	nl_cancel_down_bits(unsigned long long, char **);
extern int	nl_rate2str(unsigned long long, int, char *, size_t);
extern double	nl_cancel_down_us(uint32_t, char **);

/* generic unit translations */
extern long	nl_size2int(const char *);
extern char *	nl_size2str(const size_t, char *, const size_t);
extern long	nl_prob2int(const char *);

/* time translations */
extern int	nl_get_user_hz(void);
extern int	nl_get_psched_hz(void);
extern uint32_t	nl_us2ticks(uint32_t);
extern uint32_t	nl_ticks2us(uint32_t);
extern int	nl_str2msec(const char *, uint64_t *);
extern char *	nl_msec2str(uint64_t, char *, size_t);

/* link layer protocol translations */
extern char *	nl_llproto2str(int, char *, size_t);
extern int	nl_str2llproto(const char *);

/* ethernet protocol translations */
extern char *	nl_ether_proto2str(int, char *, size_t);
extern int	nl_str2ether_proto(const char *);

/* IP protocol translations */
extern char *	nl_ip_proto2str(int, char *, size_t);
extern int	nl_str2ip_proto(const char *);

/* Dumping helpers */
extern void	nl_new_line(struct nl_dump_params *);
extern void	nl_dump(struct nl_dump_params *, const char *, ...);
extern void	nl_dump_line(struct nl_dump_params *, const char *, ...);

enum {
	NL_CAPABILITY_NONE,

	/**
	 * rtnl_route_build_msg() no longer guesses the route scope
	 * if explicitly set to RT_SCOPE_NOWHERE.
	 * @ingroup utils
	 */
	NL_CAPABILITY_ROUTE_BUILD_MSG_SET_SCOPE         = 1,
#define NL_CAPABILITY_ROUTE_BUILD_MSG_SET_SCOPE NL_CAPABILITY_ROUTE_BUILD_MSG_SET_SCOPE

	/**
	 * rtnl_link_veth_get_peer() now returns a reference that is owned by the
	 * caller and must be released by the caller with rtnl_link_put().
	 */
	NL_CAPABILITY_ROUTE_LINK_VETH_GET_PEER_OWN_REFERENCE = 2,
#define NL_CAPABILITY_ROUTE_LINK_VETH_GET_PEER_OWN_REFERENCE NL_CAPABILITY_ROUTE_LINK_VETH_GET_PEER_OWN_REFERENCE

	/**
	 * rtnl_u32_add_action() and rtnl_basic_add_action() now grab a reference to act
	 * caller are free to release its own
	 */
	NL_CAPABILITY_ROUTE_LINK_CLS_ADD_ACT_OWN_REFERENCE = 3,
#define NL_CAPABILITY_ROUTE_LINK_CLS_ADD_ACT_OWN_REFERENCE NL_CAPABILITY_ROUTE_LINK_CLS_ADD_ACT_OWN_REFERENCE

	/**
	 * Indicate that the local port is unspecified until the user accesses
	 * it (via nl_socket_get_local_port()) or until nl_connect(). More importantly,
	 * if the port is left unspecified, nl_connect() will retry generating another
	 * port when bind() fails with ADDRINUSE.
	 */
	NL_CAPABILITY_NL_CONNECT_RETRY_GENERATE_PORT_ON_ADDRINUSE = 4,
#define NL_CAPABILITY_NL_CONNECT_RETRY_GENERATE_PORT_ON_ADDRINUSE NL_CAPABILITY_NL_CONNECT_RETRY_GENERATE_PORT_ON_ADDRINUSE

	__NL_CAPABILITY_MAX
#define NL_CAPABILITY_MAX                               (__NL_CAPABILITY_MAX - 1)
};
int nl_has_capability (int capability);

#ifdef __cplusplus
}
#endif

#endif

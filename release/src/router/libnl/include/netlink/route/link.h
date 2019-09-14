/*
 * netlink/route/link.h		Links (Interfaces)
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2012 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_LINK_H_
#define NETLINK_LINK_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/addr.h>
#include <linux/if.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct rtnl_link link.h "netlink/route/link.h"
 * @brief Link object
 * @implements nl_object
 * @ingroup link
 *
 * @copydoc private_struct
 */
struct rtnl_link;

/**
 * @ingroup link
 */
typedef enum {
	RTNL_LINK_RX_PACKETS,		/*!< Packets received */
	RTNL_LINK_TX_PACKETS,		/*!< Packets sent */
	RTNL_LINK_RX_BYTES,		/*!< Bytes received */
	RTNL_LINK_TX_BYTES,		/*!< Bytes sent */
	RTNL_LINK_RX_ERRORS,		/*!< Receive errors */
	RTNL_LINK_TX_ERRORS,		/*!< Send errors */
	RTNL_LINK_RX_DROPPED,		/*!< Received packets dropped */
	RTNL_LINK_TX_DROPPED,		/*!< Packets dropped during transmit */
	RTNL_LINK_RX_COMPRESSED,	/*!< Compressed packets received */
	RTNL_LINK_TX_COMPRESSED,	/*!< Compressed packets sent */
	RTNL_LINK_RX_FIFO_ERR,		/*!< Receive FIFO errors */
	RTNL_LINK_TX_FIFO_ERR,		/*!< Send FIFO errors */
	RTNL_LINK_RX_LEN_ERR,		/*!< Length errors */
	RTNL_LINK_RX_OVER_ERR,		/*!< Over errors */
	RTNL_LINK_RX_CRC_ERR,		/*!< CRC errors */
	RTNL_LINK_RX_FRAME_ERR,		/*!< Frame errors */
	RTNL_LINK_RX_MISSED_ERR,	/*!< Missed errors */
	RTNL_LINK_TX_ABORT_ERR,		/*!< Aborted errors */
	RTNL_LINK_TX_CARRIER_ERR,	/*!< Carrier errors */
	RTNL_LINK_TX_HBEAT_ERR,		/*!< Heartbeat errors */
	RTNL_LINK_TX_WIN_ERR,		/*!< Window errors */
	RTNL_LINK_COLLISIONS,		/*!< Send collisions */
	RTNL_LINK_MULTICAST,		/*!< Multicast */
	RTNL_LINK_IP6_INPKTS,		/*!< IPv6 SNMP InReceives */
	RTNL_LINK_IP6_INHDRERRORS,	/*!< IPv6 SNMP InHdrErrors */
	RTNL_LINK_IP6_INTOOBIGERRORS,	/*!< IPv6 SNMP InTooBigErrors */
	RTNL_LINK_IP6_INNOROUTES,	/*!< IPv6 SNMP InNoRoutes */
	RTNL_LINK_IP6_INADDRERRORS,	/*!< IPv6 SNMP InAddrErrors */
	RTNL_LINK_IP6_INUNKNOWNPROTOS,	/*!< IPv6 SNMP InUnknownProtos */
	RTNL_LINK_IP6_INTRUNCATEDPKTS,	/*!< IPv6 SNMP InTruncatedPkts */
	RTNL_LINK_IP6_INDISCARDS,	/*!< IPv6 SNMP InDiscards */
	RTNL_LINK_IP6_INDELIVERS,	/*!< IPv6 SNMP InDelivers */
	RTNL_LINK_IP6_OUTFORWDATAGRAMS,	/*!< IPv6 SNMP OutForwDatagrams */
	RTNL_LINK_IP6_OUTPKTS,		/*!< IPv6 SNMP OutRequests */
	RTNL_LINK_IP6_OUTDISCARDS,	/*!< IPv6 SNMP OutDiscards */
	RTNL_LINK_IP6_OUTNOROUTES,	/*!< IPv6 SNMP OutNoRoutes */
	RTNL_LINK_IP6_REASMTIMEOUT,	/*!< IPv6 SNMP ReasmTimeout */
	RTNL_LINK_IP6_REASMREQDS,	/*!< IPv6 SNMP ReasmReqds */
	RTNL_LINK_IP6_REASMOKS,		/*!< IPv6 SNMP ReasmOKs */
	RTNL_LINK_IP6_REASMFAILS,	/*!< IPv6 SNMP ReasmFails */
	RTNL_LINK_IP6_FRAGOKS,		/*!< IPv6 SNMP FragOKs */
	RTNL_LINK_IP6_FRAGFAILS,	/*!< IPv6 SNMP FragFails */
	RTNL_LINK_IP6_FRAGCREATES,	/*!< IPv6 SNMP FragCreates */
	RTNL_LINK_IP6_INMCASTPKTS,	/*!< IPv6 SNMP InMcastPkts */
	RTNL_LINK_IP6_OUTMCASTPKTS,	/*!< IPv6 SNMP OutMcastPkts */
	RTNL_LINK_IP6_INBCASTPKTS,	/*!< IPv6 SNMP InBcastPkts */
	RTNL_LINK_IP6_OUTBCASTPKTS,	/*!< IPv6 SNMP OutBcastPkts */
	RTNL_LINK_IP6_INOCTETS,		/*!< IPv6 SNMP InOctets */
	RTNL_LINK_IP6_OUTOCTETS,	/*!< IPv6 SNMP OutOctets */
	RTNL_LINK_IP6_INMCASTOCTETS,	/*!< IPv6 SNMP InMcastOctets */
	RTNL_LINK_IP6_OUTMCASTOCTETS,	/*!< IPv6 SNMP OutMcastOctets */
	RTNL_LINK_IP6_INBCASTOCTETS,	/*!< IPv6 SNMP InBcastOctets */
	RTNL_LINK_IP6_OUTBCASTOCTETS,	/*!< IPv6 SNMP OutBcastOctets */
	RTNL_LINK_ICMP6_INMSGS,		/*!< ICMPv6 SNMP InMsgs */
	RTNL_LINK_ICMP6_INERRORS,	/*!< ICMPv6 SNMP InErrors */
	RTNL_LINK_ICMP6_OUTMSGS,	/*!< ICMPv6 SNMP OutMsgs */
	RTNL_LINK_ICMP6_OUTERRORS,	/*!< ICMPv6 SNMP OutErrors */
	RTNL_LINK_ICMP6_CSUMERRORS,	/*!< ICMPv6 SNMP InCsumErrors */
	RTNL_LINK_IP6_CSUMERRORS,	/*!< IPv6 SNMP InCsumErrors */
	RTNL_LINK_IP6_NOECTPKTS,	/*!< IPv6 SNMP InNoECTPkts */
	RTNL_LINK_IP6_ECT1PKTS,		/*!< IPv6 SNMP InECT1Pkts */
	RTNL_LINK_IP6_ECT0PKTS,		/*!< IPv6 SNMP InECT0Pkts */
	RTNL_LINK_IP6_CEPKTS,		/*!< IPv6 SNMP InCEPkts */
	__RTNL_LINK_STATS_MAX,
} rtnl_link_stat_id_t;

#define RTNL_LINK_STATS_MAX (__RTNL_LINK_STATS_MAX - 1)

extern struct nla_policy rtln_link_policy[];

extern struct rtnl_link *rtnl_link_alloc(void);
extern void	rtnl_link_put(struct rtnl_link *);

extern int	rtnl_link_alloc_cache(struct nl_sock *, int, struct nl_cache **);
extern struct rtnl_link *rtnl_link_get(struct nl_cache *, int);
extern struct rtnl_link *rtnl_link_get_by_name(struct nl_cache *, const char *);


extern int	rtnl_link_build_add_request(struct rtnl_link *, int,
					    struct nl_msg **);
extern int	rtnl_link_add(struct nl_sock *, struct rtnl_link *, int);
extern int	rtnl_link_build_change_request(struct rtnl_link *,
					       struct rtnl_link *, int,
					       struct nl_msg **);
extern int	rtnl_link_change(struct nl_sock *, struct rtnl_link *,
				 struct rtnl_link *, int);

extern int	rtnl_link_build_delete_request(const struct rtnl_link *,
					       struct nl_msg **);
extern int	rtnl_link_delete(struct nl_sock *, const struct rtnl_link *);
extern int	rtnl_link_build_get_request(int, const char *,
					    struct nl_msg **);
extern int	rtnl_link_get_kernel(struct nl_sock *, int, const char *,
				     struct rtnl_link **);

/* Name <-> Index Translations */
extern char * 	rtnl_link_i2name(struct nl_cache *, int, char *, size_t);
extern int	rtnl_link_name2i(struct nl_cache *, const char *);

/* Name <-> Statistic Translations */
extern char *	rtnl_link_stat2str(int, char *, size_t);
extern int	rtnl_link_str2stat(const char *);

/* Link Flags Translations */
extern char *	rtnl_link_flags2str(int, char *, size_t);
extern int	rtnl_link_str2flags(const char *);

extern char *	rtnl_link_operstate2str(uint8_t, char *, size_t);
extern int	rtnl_link_str2operstate(const char *);

extern char *	rtnl_link_mode2str(uint8_t, char *, size_t);
extern int	rtnl_link_str2mode(const char *);

/* Carrier State Translations */
extern char *	rtnl_link_carrier2str(uint8_t, char *, size_t);
extern int	rtnl_link_str2carrier(const char *);

/* Access Functions */
extern void	rtnl_link_set_qdisc(struct rtnl_link *, const char *);
extern char *	rtnl_link_get_qdisc(struct rtnl_link *);

extern void	rtnl_link_set_name(struct rtnl_link *, const char *);
extern char *	rtnl_link_get_name(struct rtnl_link *);

extern void	rtnl_link_set_group(struct rtnl_link *, uint32_t);
extern uint32_t	rtnl_link_get_group(struct rtnl_link *);

extern void	rtnl_link_set_flags(struct rtnl_link *, unsigned int);
extern void	rtnl_link_unset_flags(struct rtnl_link *, unsigned int);
extern unsigned int rtnl_link_get_flags(struct rtnl_link *);

extern void	rtnl_link_set_mtu(struct rtnl_link *, unsigned int);
extern unsigned int rtnl_link_get_mtu(struct rtnl_link *);

extern void	rtnl_link_set_txqlen(struct rtnl_link *, unsigned int);
extern unsigned int rtnl_link_get_txqlen(struct rtnl_link *);

extern void	rtnl_link_set_ifindex(struct rtnl_link *, int);
extern int	rtnl_link_get_ifindex(struct rtnl_link *);

extern void	rtnl_link_set_family(struct rtnl_link *, int);
extern int	rtnl_link_get_family(struct rtnl_link *);

extern void	rtnl_link_set_arptype(struct rtnl_link *, unsigned int);
extern unsigned int rtnl_link_get_arptype(struct rtnl_link *);

extern void	rtnl_link_set_addr(struct rtnl_link *, struct nl_addr *);
extern struct nl_addr *rtnl_link_get_addr(struct rtnl_link *);

extern void	rtnl_link_set_broadcast(struct rtnl_link *, struct nl_addr *);
extern struct nl_addr *rtnl_link_get_broadcast(struct rtnl_link *);

extern void	rtnl_link_set_link(struct rtnl_link *, int);
extern int	rtnl_link_get_link(struct rtnl_link *);

extern void	rtnl_link_set_master(struct rtnl_link *, int);
extern int	rtnl_link_get_master(struct rtnl_link *);

extern void	rtnl_link_set_carrier(struct rtnl_link *, uint8_t);
extern uint8_t	rtnl_link_get_carrier(struct rtnl_link *);

extern void	rtnl_link_set_operstate(struct rtnl_link *, uint8_t);
extern uint8_t	rtnl_link_get_operstate(struct rtnl_link *);

extern void	rtnl_link_set_linkmode(struct rtnl_link *, uint8_t);
extern uint8_t	rtnl_link_get_linkmode(struct rtnl_link *);

extern const char *	rtnl_link_get_ifalias(struct rtnl_link *);
extern void		rtnl_link_set_ifalias(struct rtnl_link *, const char *);

extern int		rtnl_link_get_num_vf(struct rtnl_link *, uint32_t *);

extern uint64_t rtnl_link_get_stat(struct rtnl_link *, rtnl_link_stat_id_t);
extern int	rtnl_link_set_stat(struct rtnl_link *, rtnl_link_stat_id_t,
				   const uint64_t);

extern int	rtnl_link_set_type(struct rtnl_link *, const char *);
extern char *	rtnl_link_get_type(struct rtnl_link *);

extern void	rtnl_link_set_promiscuity(struct rtnl_link *, uint32_t);
extern uint32_t	rtnl_link_get_promiscuity(struct rtnl_link *);

extern void	rtnl_link_set_num_tx_queues(struct rtnl_link *, uint32_t);
extern uint32_t	rtnl_link_get_num_tx_queues(struct rtnl_link *);

extern void	rtnl_link_set_num_rx_queues(struct rtnl_link *, uint32_t);
extern uint32_t	rtnl_link_get_num_rx_queues(struct rtnl_link *);

extern struct nl_data *	rtnl_link_get_phys_port_id(struct rtnl_link *);

extern void	rtnl_link_set_ns_fd(struct rtnl_link *, int);
extern int	rtnl_link_get_ns_fd(struct rtnl_link *);
extern void	rtnl_link_set_ns_pid(struct rtnl_link *, pid_t);
extern pid_t	rtnl_link_get_ns_pid(struct rtnl_link *);

extern int	rtnl_link_enslave_ifindex(struct nl_sock *, int, int);
extern int	rtnl_link_enslave(struct nl_sock *, struct rtnl_link *,
				  struct rtnl_link *);
extern int	rtnl_link_release_ifindex(struct nl_sock *, int);
extern int	rtnl_link_release(struct nl_sock *, struct rtnl_link *);
extern int	rtnl_link_fill_info(struct nl_msg *, struct rtnl_link *);
extern int	rtnl_link_info_parse(struct rtnl_link *, struct nlattr **);


/* deprecated */
extern int	rtnl_link_set_info_type(struct rtnl_link *, const char *) __attribute__((deprecated));
extern char *	rtnl_link_get_info_type(struct rtnl_link *) __attribute__((deprecated));
extern void	rtnl_link_set_weight(struct rtnl_link *, unsigned int) __attribute__((deprecated));
extern unsigned int rtnl_link_get_weight(struct rtnl_link *) __attribute__((deprecated));


#ifdef __cplusplus
}
#endif

#endif

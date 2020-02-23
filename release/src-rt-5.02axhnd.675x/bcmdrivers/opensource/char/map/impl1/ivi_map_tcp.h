/*************************************************************************
 *
 * ivi_map_tcp.c :
 *
 * This file is the header file for the 'ivi_map_tcp.c' file.
 *
 * Copyright (C) 2013 CERNET Network Center
 * All rights reserved.
 * 
 * Design and coding: 
 *   Xing Li <xing@cernet.edu.cn> 
 *	 Congxiao Bao <congxiao@cernet.edu.cn>
 *   Guoliang Han <bupthgl@gmail.com>
 * 	 Yuncheng Zhu <haoyu@cernet.edu.cn>
 * 	 Wentao Shang <wentaoshang@gmail.com>
 * 	 
 * 
 * Contributions:
 *
 * This file is part of MAP-T/MAP-E Kernel Module.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * You should have received a copy of the GNU General Public License 
 * along with MAP-T/MAP-E Kernel Module. If not, see 
 * <http://www.gnu.org/licenses/>.
 *
 * For more versions, please send an email to <bupthgl@gmail.com> to
 * obtain an password to access the svn server.
 *
 * LIC: GPLv2
 *
 ************************************************************************/

#ifndef IVI_MAP_TCP_H
#define IVI_MAP_TCP_H

#include <linux/module.h>
#include <linux/time.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/tcp.h>
#include <asm/unaligned.h>
#include <net/tcp.h>
//#include "a.h"

#include "ivi_config.h"
#include "ivi_map.h"

/* map list structure */
struct tcp_map_list {
	spinlock_t lock;
	struct     hlist_head out_chain[IVI_HTABLE_SIZE];   // Map table from oldport to newport
	struct     hlist_head in_chain[IVI_HTABLE_SIZE];    // Map table from newport to oldport
	struct     hlist_head dest_chain[IVI_HTABLE_SIZE];   // Map table with destination and newport
	int        size;                                     // Number of mappings in the list
	int        port_num;                                 // Number of MAP ports allocated in the map list
	int        state_seq;                                // Sequence number of the mapping(never decreased)                                  
	__be16     last_alloc_port;                         // Save the last allocated port number
};

// Packet flow direction
typedef enum _PACKET_DIR {
	PACKET_DIR_LOCAL = 0,  // Sent from local to remote
	PACKET_DIR_REMOTE,     // Sent from remote to local
	PACKET_DIR_MAX
} PACKET_DIR, *PPACKET_DIR;

typedef enum _TCP_STATUS {
	TCP_STATUS_NONE = 0,      // Initial state: 0
	TCP_STATUS_SYN_SENT,      // SYN only packet sent: 1
	TCP_STATUS_SYN_RECV,      // SYN-ACK packet sent: 2
	TCP_STATUS_ESTABLISHED,   // ACK packet sent: 3
	TCP_STATUS_FIN_WAIT,      // FIN packet sent: 4
	TCP_STATUS_CLOSE_WAIT,    // ACK sent after FIN received: 5
	TCP_STATUS_LAST_ACK,      // FIN sent after FIN received: 6
	TCP_STATUS_TIME_WAIT,     // Last ACK sent: 7
	TCP_STATUS_CLOSE,         // Connection closed: 8
	TCP_STATUS_SYN_SENT2,     // SYN only packet received after SYN sent, simultaneous open: 9
	TCP_STATUS_MAX,
	TCP_STATUS_IGNORE
} TCP_STATUS, *PTCP_STATUS;

typedef struct _TCP_STATE_INFO {
	u_int32_t  End;
	u_int32_t  MaxEnd;
	u_int32_t  MaxWindow;
	u_int32_t  MaxAck;
	u_int8_t   Scale;
	u_int8_t   Options;
} TCP_STATE_INFO, *PTCP_STATE_INFO;

typedef struct _TCP_STATE_CONTEXT {
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	uint32_t          blog_key[2];
	struct timeval evict_time;  // flow evict time from flow cache 
#endif
	struct hlist_node out_node;  // Inserted to out_chain
	struct hlist_node in_node;   // Inserted to in_chain
	struct hlist_node dest_node;   // Inserted to dest_chain
	
	int state_seq;
	
	// Indexes pointing back to port hash table
	__be32            oldaddr;
	__be16            oldport;
	__be32            dstaddr;
	__be16            dstport;
	__be16            newport;

	// TCP state info
	TCP_STATE_INFO    Seen[PACKET_DIR_MAX];     // Seen[0] for local state, Seen[1] for remote state
	struct timeval    StateSetTime;    // The time when the current state is set
	unsigned int      StateTimeOut;    // Timeout value for the current state
	TCP_STATUS        Status;
	// For detecting retransmitted packets
	PACKET_DIR        LastDir;
	u_int8_t          RetransCount;
	u_int8_t          LastControlBits;
	u_int32_t         LastWindow;
	u_int32_t         LastSeq;
	u_int32_t         LastAck;
	u_int32_t         LastEnd;
} TCP_STATE_CONTEXT, *PTCP_STATE_CONTEXT;

extern struct tcp_map_list tcp_list;
extern struct hlist_node *pf_state;
extern struct hlist_node *tcp_state;

extern void init_tcp_map_list(void);

extern void refresh_tcp_map_list(int);

extern void free_tcp_map_list(void);

extern int port_reserve(__be16);

/* mapping operations */
extern int get_outflow_tcp_map_port(__be32 oldaddr, __be16 oldp, __be32 dstaddr, __be16 dstp, u16 ratio, u16 adjacent, u16 offset, struct tcphdr *th, __u32 len, __be16 *newp, struct sk_buff *skb);
extern int get_inflow_tcp_map_port(__be16 newp, __be32 dstaddr, __be16 dstp, struct tcphdr *th, __u32 len, __be32 *oldaddr, __be16 *oldp, struct sk_buff *skb);

extern int ivi_map_tcp_init(void);
extern void ivi_map_tcp_exit(void);

#endif /* IVI_MAP_TCP_H */

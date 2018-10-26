/*************************************************************************
 *
 * ivi_map.h :
 *
 * This file is the header file for the 'ivi_map.c' file,
 * which contains all the system header files and definitions
 * used in the 'ivi_map.c' file.
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

#ifndef IVI_MAP_H
#define IVI_MAP_H

#include <linux/module.h>
#include <linux/time.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/brcm_dll.h>

#include "ivi_config.h"
#if 0
#include "ivi_map_tcp.h"
#endif

/* map entry structure */
struct map_tuple {
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	uint32_t blog_key[2];
	struct timeval evict_time;  // flow evict time from flow cache 
#endif
	struct hlist_node out_node;  // Inserted to out_chain
	struct hlist_node in_node;   // Inserted to in_chain
	struct hlist_node dest_node;   // Inserted to dest_chain
	__be32 oldaddr;
	__be16 oldport;
	__be32 dstaddr;
	__be16 newport;
	struct timeval timer;
};

/* map list structure */
struct map_list {
	spinlock_t lock;
	struct hlist_head out_chain[IVI_HTABLE_SIZE];  // Map table from oldport to newport
	struct hlist_head in_chain[IVI_HTABLE_SIZE];   // Map table from newport to oldport
	struct hlist_head dest_chain[IVI_HTABLE_SIZE]; // Map table with destination and newport
	int size;
	int port_num;            // Number of MAP ports allocated in the map list
	__be16 last_alloc_port;  // Save the last allocate port number
	time_t timeout;
	int type;
};

#define MAPFRAG_HTABLE_SIZE 64
#define MAPFRAG_MAX_ENTRIES 256
#define MAPFRAG_IX_INVALID 0
#define MAPFRAG_NULL ((MapFrag_t*)NULL)
/* map list to map fragmented IPv6 to LAN nated address */
typedef struct mapfrag_t {
	struct dll_t node;
	struct mapfrag_t *chain_p;

	u32 idx;
	u32 ipid;
	struct in6_addr v6addr;
	__be32 v4addr;
	struct timeval timer;
} __attribute__ ((packed)) MapFrag_t;

extern u32 mapfrag_lookup( const struct in6_addr *v6addr, u32 ipid );
extern void mapfrag_get( u32 idx, __be32 * v4addr, struct timeval *timer );
extern void mapfrag_set( u32 idx, __be32 v4addr );
extern void mapfrag_delete( u32 ipid );
extern int init_mapfrag_list( time_t timeout );


/* global map list variables */
extern u16 hgw_ratio;
extern u16 hgw_offset;
extern u16 hgw_suffix;
extern u16 hgw_adjacent;

extern struct map_list tcp_list;
extern struct map_list udp_list;
extern struct map_list icmp_list;


/* list operations */
extern void refresh_map_list(struct map_list *list);
extern void free_map_list(struct map_list *list);

/* mapping operations */
extern int get_outflow_map_port(struct map_list *list, __be32 oldaddr, __be16 oldp, __be32 dstaddr, u16 ratio, u16 adjacent, u16 offset, __be16 *newp, struct sk_buff *skb);
extern int get_inflow_map_port(struct map_list *list, __be16 newp, __be32 dstaddr, __be32* oldaddr, __be16 *oldp, struct sk_buff *skb);

extern int ivi_map_init(void);
extern void ivi_map_exit(void);

#endif /* IVI_MAP_H */

/*************************************************************************
 *
 * ivi_map.c :
 *
 * This file defines the generic mapping list data structure and basic 
 * operations, which will be used in other modules. 'ivi_map' module 
 * will be installed first when running './control start' command.
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

#include "ivi_map.h"
#include "ivi_portmap.h"
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif

struct map_list tcp_list;
struct map_list udp_list;
struct map_list icmp_list;

typedef struct {
    MapFrag_t      * htable[ MAPFRAG_HTABLE_SIZE ];
    MapFrag_t        etable[ MAPFRAG_MAX_ENTRIES ];

    Dll_t         frlist;
    time_t        timeout;
} __attribute__((aligned(16))) MapFragment_t;

MapFragment_t mapfragment;    /* Global map fragment context */


/* ratio and offset together indicate the port pool range */
u16 hgw_ratio = 1;

u16 hgw_offset = 0;

u16 hgw_suffix = 0;

u16 hgw_adjacent = 1024; // draft-ietf-softwire-map-05 specifies the default PSID offset is 6.

/* list operations */

// Get current size of the list, must be protected by spin lock when calling this function
static inline int get_list_port_num(struct map_list *list)
{
	return list->port_num;
}

// Init list
static void init_map_list(struct map_list *list, time_t timeout, int type)
{
	int i;
	spin_lock_init(&list->lock);
	for (i = 0; i < IVI_HTABLE_SIZE; i++) {
		INIT_HLIST_HEAD(&list->out_chain[i]);
		INIT_HLIST_HEAD(&list->in_chain[i]);
		INIT_HLIST_HEAD(&list->dest_chain[i]);
	}
	list->size = 0;
	list->port_num = 0;
	list->portmap_num = 0;
	list->last_alloc_port = 0;
	list->timeout = timeout;
	list->type = type;
}

// Check whether a newport is in use now, must be protected by spin lock when calling this function
static int port_in_use(__be16 port, struct map_list *list)
{
	int ret = 0;
	int hash;
	struct map_tuple *iter;

	hash = port_hashfn(port);
	if (!hlist_empty(&list->in_chain[hash])) {
		hlist_for_each_entry(iter, &list->in_chain[hash], in_node) {
			if (iter->newport == port) {
				ret = 1;
				break;
			}
		}
	}

	if (ret == 0)
		ret = mapportmap_port(port, (1<<list->type));

	return ret;
}

// Add a new map, the pointer to the new map_tuple is returned on success, must be protected by spin lock when calling this function
static struct map_tuple* add_new_map(u32 portmapidx, __be32 oldaddr, __be16 oldp, __be32 dstaddr, __be16 newp, struct map_list *list)
{
	struct map_tuple *map;
	int hash;
	map = (struct map_tuple*)kmalloc(sizeof(struct map_tuple), GFP_ATOMIC);
	if (map == NULL) {
		printk(KERN_ERR "add_new_map: kmalloc failed for map_tuple.\n");
		return NULL;
	}

	map->portmapidx = portmapidx;
	map->oldaddr = oldaddr;
	map->oldport = oldp;
	map->dstaddr = dstaddr;
	map->newport = newp;
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
    map->blog_key[BLOG_PARAM1_MAP_DIR_US] = BLOG_KEY_FC_INVALID;
    map->blog_key[BLOG_PARAM1_MAP_DIR_DS] = BLOG_KEY_FC_INVALID;
#endif
	do_gettimeofday(&map->timer);
	
	hash = v4addr_port_hashfn(oldaddr, oldp);
	hlist_add_head(&map->out_node, &list->out_chain[hash]);
	hash = port_hashfn(newp);
	hlist_add_head(&map->in_node, &list->in_chain[hash]);
	hash = v4addr_port_hashfn(dstaddr, 0);
	hlist_add_head(&map->dest_node, &list->dest_chain[hash]);
	
	list->size++;
	
	return map;
}

// Refresh the timer for each map_tuple, must NOT acquire spin lock when calling this function
void refresh_map_list(struct map_list *list, u32 portmapidx)
{
	struct map_tuple *iter, *i0;
	struct hlist_node *loop, *l0;
	struct timeval now;
	time_t delta;
	int i, flag;	
	do_gettimeofday(&now);
	
	spin_lock_bh(&list->lock);
	if (portmapidx != MAPPORTMAP_IX_INVALID && list->portmap_num == 0) {
		spin_unlock_bh(&list->lock);
		return;
	}
	// Iterate all the map_tuple through out_chain only, in_chain contains the same info.
	for (i = 0; i < IVI_HTABLE_SIZE; i++) {
		hlist_for_each_entry_safe(iter, loop, &list->out_chain[i], out_node) {
			if (portmapidx == MAPPORTMAP_IX_INVALID)
				delta = now.tv_sec - iter->timer.tv_sec;
			else if (portmapidx != iter->portmapidx)
				continue;

			if (portmapidx != MAPPORTMAP_IX_INVALID || delta >= list->timeout) {
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
				if (portmapidx == MAPPORTMAP_IX_INVALID) {
					blog_lock();
					if (iter->blog_key[BLOG_PARAM1_MAP_DIR_US] != BLOG_KEY_FC_INVALID || 
					    iter->blog_key[BLOG_PARAM1_MAP_DIR_DS] != BLOG_KEY_FC_INVALID) {
						if (blog_query(QUERY_MAP_TUPLE, (void*)iter, 
						            iter->blog_key[BLOG_PARAM1_MAP_DIR_US],
						            iter->blog_key[BLOG_PARAM1_MAP_DIR_DS], 0)) {
							blog_unlock();
							continue;
						}
					}
					else {
						// flow cache flow might have disassociated itself from map tuple.
						if (iter->evict_time.tv_sec) {
							iter->timer.tv_sec = iter->evict_time.tv_sec;
							delta = now.tv_sec - iter->timer.tv_sec;
							if (delta < list->timeout) {
								blog_unlock();
								continue;
							}
						}
					}
					blog_unlock();
				}
#endif

#ifdef IVI_DEBUG_MAP
				printk(KERN_INFO "refresh_map_list: %s map " NIP4_FMT ":%d -> " NIP4_FMT " ------> %d on out_chain[%d]\n", portmapidx == MAPPORTMAP_IX_INVALID ? "time out" : "delete", NIP4(iter->oldaddr), iter->oldport, NIP4(iter->dstaddr), iter->newport, i);
#endif

				hlist_del(&iter->out_node);
				hlist_del(&iter->in_node);
				hlist_del(&iter->dest_node);
				list->size--;

				flag = 0; // indicating whether list->port_num needs to be substracted by 1.
 				hlist_for_each_entry_safe(i0, l0, &list->in_chain[port_hashfn(iter->newport)], in_node) {
 					if (i0->newport == iter->newport) {
#ifdef IVI_DEBUG_MAP
 						printk(KERN_INFO "refresh_map_list: newport %d is still used by someone(" NIP4_FMT ":%d -> " NIP4_FMT "). port_num is still %d\n", iter->newport, NIP4(i0->oldaddr), i0->oldport, NIP4(i0->dstaddr), list->port_num);
#endif
 						flag = 1;
 						break;
 					}
 				}
 				if (!flag) {
 					list->port_num--;
#ifdef IVI_DEBUG_MAP
 					printk(KERN_INFO "refresh_map_list: port_num is decreased by 1 to %d(%d)\n", list->port_num, iter->newport);
#endif
 				}

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
				blog_lock();
				if (iter->blog_key[BLOG_PARAM1_MAP_DIR_US] != BLOG_KEY_FC_INVALID || 
				    iter->blog_key[BLOG_PARAM1_MAP_DIR_DS] != BLOG_KEY_FC_INVALID) {
					blog_notify(DESTROY_MAP_TUPLE, (void*)iter, 
					            iter->blog_key[BLOG_PARAM1_MAP_DIR_US],
					            iter->blog_key[BLOG_PARAM1_MAP_DIR_DS]);
				}
				blog_unlock();
#endif

				kfree(iter);

				if (portmapidx != MAPPORTMAP_IX_INVALID && --list->portmap_num == 0) {
					spin_unlock_bh(&list->lock);
					return;
				}
			}
		}
	}
	spin_unlock_bh(&list->lock);
}

// Clear the entire list, must NOT acquire spin lock when calling this function
void free_map_list(struct map_list *list)
{
	struct map_tuple *iter;
	struct hlist_node *loop;
	int i;
	
	spin_lock_bh(&list->lock);
	// Iterate all the map_tuple through out_chain only, in_chain contains the same info.
	for (i = 0; i < IVI_HTABLE_SIZE; i++) {
		hlist_for_each_entry_safe(iter, loop, &list->out_chain[i], out_node) {		
			hlist_del(&iter->out_node);
			hlist_del(&iter->in_node);
			hlist_del(&iter->dest_node);
			list->size--;
			
#ifdef IVI_DEBUG_MAP
			printk(KERN_INFO "free_map_list: delete map " NIP4_FMT ":%d -> " NIP4_FMT " ------> %d on out_chain[%d]\n", NIP4(iter->oldaddr), iter->oldport, NIP4(iter->dstaddr), iter->newport, i);
#endif
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
            blog_lock();
            if (iter->blog_key[BLOG_PARAM1_MAP_DIR_US] != BLOG_KEY_FC_INVALID || 
                iter->blog_key[BLOG_PARAM1_MAP_DIR_DS] != BLOG_KEY_FC_INVALID) {
                blog_notify(DESTROY_MAP_TUPLE, (void*)iter, 
                            iter->blog_key[BLOG_PARAM1_MAP_DIR_US],
                            iter->blog_key[BLOG_PARAM1_MAP_DIR_DS]);
			}
            blog_unlock();
#endif

			kfree(iter);
		}
	}
	list->port_num = 0;
	list->portmap_num = 0;
	spin_unlock_bh(&list->lock);
}

/* mapping operations */

// Get mapped port for outflow packet, input and output are in host byte order, return -1 if failed
int get_outflow_map_port(struct map_list *list, __be32 oldaddr, __be16 oldp, __be32 dstaddr, u16 ratio, u16 adjacent, u16 offset, __be16 *newp, struct sk_buff *skb)
{
	int hash, reusing, status, start_port;
	__be16 retport;
	struct map_tuple *multiplex_state;
	struct map_tuple *iter;
	struct map_tuple *tuple = NULL;
	struct hlist_node *loop;
		
	*newp = 0;
	reusing = 0;
	status = 0;
	retport = 0;
	ratio = fls(ratio) - 1;
	adjacent = fls(adjacent) - 1;
	start_port = ((1 << (ratio + adjacent)) > 1024) ? 1 << (ratio + adjacent) : 1024; // the ports below start_port are reserved for system ports.
	
	refresh_map_list(list, MAPPORTMAP_IX_INVALID);
	spin_lock_bh(&list->lock);
	
	hash = v4addr_port_hashfn(oldaddr, oldp);
	if (!hlist_empty(&list->out_chain[hash])) {
		hlist_for_each_entry(iter, &list->out_chain[hash], out_node) {
			if (iter->oldport == oldp && iter->oldaddr == oldaddr) {
				if (iter->dstaddr == dstaddr) {	
					retport = iter->newport;
					do_gettimeofday(&iter->timer);
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
                    blog_link(MAP_TUPLE, blog_ptr(skb), (void*)iter, BLOG_PARAM1_MAP_DIR_US, 0);
#endif
#ifdef IVI_DEBUG_MAP
					//printk(KERN_INFO "get_outflow_map_port: find map " NIP4_FMT ":%d -> " NIP4_FMT " ------> %d on out_chain[%d]\n", NIP4(iter->oldaddr), iter->oldport, NIP4(iter->dstaddr), iter->newport, hash);
#endif
					goto out;
				}
				else if (reusing == 0) { // src addr & port same, while dest addr & port different: reuse the mapped port (Endpoint-independent)
					retport = iter->newport;
					reusing = 1;	
#ifdef IVI_DEBUG_MAP	
					printk(KERN_INFO "get_outflow_map_port: port %d can be multiplexed with source address " NIP4_FMT ":%d\n", retport, NIP4(oldaddr), oldp);
#endif
				}

			}
		}
	}
	
	if (retport == 0 && reusing == 0) {		
		__be16 rover_j, rover_k;	
		int dsthash, i, rand_j, chance;
		struct hlist_node *loop0;
		
		status = 0;
		chance = UDP_MAX_LOOP_NUM;
			
		// Now we have to find a mapping whose src & dest are both different to multiplex:
		dsthash = v4addr_port_hashfn(dstaddr, 0);
		while (1) { // we want to generate an integer between [1, 31]
			get_random_bytes(&rand_j, sizeof(int));
			rand_j = (rand_j >= 0) ? rand_j : -rand_j;
			rand_j -= (rand_j >> 5) << 5;
			if (rand_j) break;
		}
			
		/* hash is a random number between [0,31] except dsthash, so MAYBE its newport can be multiplexed because 
		   dest_chain[hash] is impossible to have the same destination with this packet.*/
		hash = (dsthash + rand_j >= 32) ? (dsthash + rand_j - 32) : (dsthash + rand_j); 
		
		for (i = 0; i < 31 && chance > 0; i++) {		
			if (!hlist_empty(&list->dest_chain[hash])) {
				hlist_for_each_entry_safe(multiplex_state, loop0, &list->dest_chain[hash], dest_node) {
					retport = multiplex_state->newport;
					status = 1;
					
					/* don't worry:) we have to check whether this port has been multiplexed by another 
					   connection with the same destination */
					if (!hlist_empty(&list->dest_chain[dsthash])) {
						hlist_for_each_entry_safe(iter, loop, &list->dest_chain[dsthash], dest_node) {
							if (iter->dstaddr == dstaddr && iter->newport == retport) {
								status = 0; // this port cannot be multiplexed
								break;
							}
						}
					}			
					if (status == 1) { // this port can be multiplexed
#ifdef IVI_DEBUG_MAP		
						printk(KERN_INFO "get_outflow_map_port: multiplex port %d on dest_chain[%d], round %d\n", retport, hash, i + 1);
#endif
						i = 31; // go directly to create a new mapping
						break;
					}
				}				
				if (status == 0) {
					//printk(KERN_DEBUG "ooops, you have only %d chance left now~\n", chance);
					chance--;
				}
			}
			else {
				if (++hash >= 32)      
					hash = 0;
				if (hash == dsthash) {
					if (++hash >= 32)
						hash = 0;
				}
			}
		}
		
		if (status == 0) {
			// If it's so lucky to reach here, we have to generate a new port	
			if (get_list_port_num(list) >= ((65536 - start_port)>>ratio)) {
				spin_unlock_bh(&list->lock);
#ifdef IVI_DEBUG_MAP
				printk(KERN_INFO "get_outflow_map_port: map list full.\n");
#endif
				return -1;
			}

			if (ratio == 0)
				retport = oldp; // In 1:1 mapping mode, use old port directly.
				
			else {
				int remaining;
				__be16 low, high;
				
				low = (__u16)((start_port - 1) >> (ratio + adjacent)) + 1;
				high = (__u16)(65536 >> (ratio + adjacent)) - 1;
				remaining = (high - low) + 1;
			
				if (list->last_alloc_port != 0) {
					rover_j = list->last_alloc_port >> (ratio + adjacent);
					rover_k = list->last_alloc_port - ((list->last_alloc_port >> adjacent) << adjacent) + 1;
					if (rover_k == (1 << adjacent)) {
						rover_j++;
						rover_k = 0;
						if (rover_j > high)
							rover_j = low;
					}
				} else {
					rover_j = low;
					rover_k = 0;
				}
			
				do { 
					retport = (rover_j << (ratio + adjacent)) + (offset << adjacent) + rover_k;
					
					if (!port_in_use(retport, list))
						break;
					
					rover_k++;
					if (rover_k == (1 << adjacent)) {
						rover_j++;
						remaining--;
						rover_k = 0;
						if (rover_j > high)
							rover_j = low;
					}
				} while (remaining > 0);
				
				if (remaining <= 0) {
					spin_unlock_bh(&list->lock);
#ifdef IVI_DEBUG_MAP
					printk(KERN_INFO "get_outflow_map_port: failed to assign a new map port for " NIP4_FMT ":%d -> " NIP4_FMT "\n", NIP4(oldaddr), oldp, NIP4(dstaddr));
#endif
					return -1;
				}
			}
		}
	}
	
	tuple = add_new_map(MAPPORTMAP_IX_INVALID, oldaddr, oldp, dstaddr, retport, list);

	if (tuple == NULL) {
		spin_unlock_bh(&list->lock);
		return -1;
	}

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
    blog_link(MAP_TUPLE, blog_ptr(skb), (void*)tuple, BLOG_PARAM1_MAP_DIR_US, 0);
#endif
	
	if (status == 0 && reusing == 0) { // we generated a new mapping port
		list->last_alloc_port = retport;
		list->port_num++;
	}
	
#ifdef IVI_DEBUG_MAP
	printk(KERN_INFO "add_new_map: add new map (" NIP4_FMT ":%d -> " NIP4_FMT " -------> %d), list_len = %d, port_num = %d\n", NIP4(oldaddr), oldp, NIP4(dstaddr), retport, list->size, list->port_num);
#endif
		
out:
	*newp = retport;
	spin_unlock_bh(&list->lock);
	return (retport == 0 ? -1 : 0);
}

// Get mapped port and address for inflow packet, input and output are in host bypt order, return -1 if failed
int get_inflow_map_port(struct map_list *list, __be16 newp, __be32 dstaddr, __be32* oldaddr, __be16 *oldp, struct sk_buff *skb)
{
	struct map_tuple *iter;
	int ret, hash;
		
	refresh_map_list(list, MAPPORTMAP_IX_INVALID);
	spin_lock_bh(&list->lock);
	
	ret = 1;
	*oldp = 0;
	*oldaddr = 0;
	
	hash = port_hashfn(newp);
	hlist_for_each_entry(iter, &list->in_chain[hash], in_node) {
		if (iter->newport == newp && iter->dstaddr == dstaddr) {
			*oldaddr = iter->oldaddr;
			*oldp = iter->oldport;
			do_gettimeofday(&iter->timer);
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
            blog_link(MAP_TUPLE, blog_ptr(skb), (void*)iter, BLOG_PARAM1_MAP_DIR_DS, 0);
#endif
#ifdef IVI_DEBUG_MAP
			//printk(KERN_INFO "get_inflow_map_port: find map " NIP4_FMT ":%d -> " NIP4_FMT 
			//                 " ------> %d on in_chain[%d]\n", NIP4(iter->oldaddr), 
			//                 iter->oldport, NIP4(iter->dstaddr), iter->newport, hash);
#endif
			ret = 0;
			break;
		}		
	}
	
	if (ret == 1) {	// fail to find a mapping either in list.
		u32 idx, oldp0 = 0;
#ifdef IVI_DEBUG_MAP
		printk(KERN_INFO "get_inflow_map_port: in_chain[%d] empty.\n", hash);
#endif
		
		idx = mapportmap_lookup(oldaddr, dstaddr, newp, &oldp0, (1<<list->type), MAPPORTMAP_MODE_FIND);
		if (idx != MAPPORTMAP_IX_INVALID) {
			struct map_tuple *tuple = NULL;

			*oldp = oldp0;
			if ((tuple=add_new_map(idx, *oldaddr, *oldp, dstaddr, newp, list)) != NULL) {
				list->last_alloc_port = newp;
				list->portmap_num++;
				list->port_num++;
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
				blog_link(MAP_TUPLE, blog_ptr(skb), (void*)tuple, BLOG_PARAM1_MAP_DIR_DS, 0);
#endif
	
			}
			else {
				printk("fail add new map for portmap case\n");
				ret = -1;
			}
		}
		else
			ret = -1;
	}
	
	spin_unlock_bh(&list->lock);
	return ret;
}

static MapFrag_t * mapfrag_alloc( void )
{
    MapFrag_t * frag_p = MAPFRAG_NULL;

    if (unlikely(dll_empty(&mapfragment.frlist)))
    {
        return frag_p;
    }

    if (likely(!dll_empty(&mapfragment.frlist)))
    {
        frag_p = (MapFrag_t*)dll_head_p(&mapfragment.frlist);
        dll_delete(&frag_p->node);
        dll_init(&frag_p->node);
    }

    return frag_p;
}

static inline u32 _hash( u32 hash_val )
{
    hash_val ^= ( hash_val >> 16 );
    hash_val ^= ( hash_val >>  8 );
    hash_val ^= ( hash_val >>  3 );

    return ( hash_val );
}

static inline u32 _mapfrag_hash( u32 ipid )
{
    u32 hashix;

    hashix = _hash(ipid);

    return hashix % MAPFRAG_HTABLE_SIZE;
}

static inline u32 _mapfrag_match( const MapFrag_t *frag_p,
                                  u32 ipid, 
                                  const struct in6_addr *v6addr )
{
    return ( (frag_p->ipid == ipid) && 
             !memcmp((const void *)&(frag_p->v6addr), (const void *)v6addr, sizeof(struct in6_addr)) );
}

static void mapfrag_hashin( MapFrag_t * frag_p, u32 hashix )
{
    frag_p->chain_p = mapfragment.htable[ hashix ];
    mapfragment.htable[ hashix ] = frag_p;
}

static u32 mapfrag_new( const struct in6_addr *v6addr, u32 ipid, u32 hashix )
{
    MapFrag_t * frag_p;

    frag_p = mapfrag_alloc();
    if ( unlikely(frag_p == MAPFRAG_NULL) )
    {
        return MAPFRAG_IX_INVALID;
    }

    memcpy((void *)&(frag_p->v6addr), (const void *)v6addr, sizeof(struct in6_addr));
    frag_p->ipid = ipid;
    mapfrag_hashin(frag_p, hashix);
   	do_gettimeofday(&frag_p->timer);

    return frag_p->idx;
}

static void mapfrag_unhash(MapFrag_t * frag_p, u32 hashix)
{
    register MapFrag_t * hFrag_p = mapfragment.htable[hashix];

    if ( unlikely(hFrag_p == MAPFRAG_NULL) )
    {
        goto mapfrag_notfound;
    }

    if ( likely(hFrag_p == frag_p) )                /* At head */
    {
        mapfragment.htable[ hashix ] = frag_p->chain_p;  /* Delete at head */
    }
    else
    {
        u32 found = 0;

        /* Traverse the single linked hash collision chain */
        for ( hFrag_p = mapfragment.htable[ hashix ];
              likely(hFrag_p->chain_p != MAPFRAG_NULL);
              hFrag_p = hFrag_p->chain_p )
        {
            if ( hFrag_p->chain_p == frag_p )
            {
                hFrag_p->chain_p = frag_p->chain_p;
                found = 1;
                break;
            }
        }

        if ( unlikely(found == 0) )
        {
            goto mapfrag_notfound;
        }
    }

mapfrag_notfound:
    return; /* SUCCESS */
}

static void mapfrag_free( MapFrag_t * frag_p )
{
    frag_p->ipid = 0;
    frag_p->v4addr = 0;
    memset((void *)&(frag_p->v6addr), 0, sizeof(struct in6_addr));
    frag_p->timer.tv_sec = 0;
    frag_p->timer.tv_usec = 0;
    frag_p->chain_p = MAPFRAG_NULL;

    dll_prepend(&mapfragment.frlist, &frag_p->node);
}

void mapfrag_delete( u32 idx )
{
    MapFrag_t * frag_p;
    u32 hashix;

    frag_p = &mapfragment.etable[idx];
    hashix = _mapfrag_hash(frag_p->ipid);

    mapfrag_unhash(frag_p, hashix);
    mapfrag_free(frag_p);
}

static void refresh_mapfrag_list( void )
{
	struct timeval now;
	time_t delta;
    MapFrag_t * frag_p;
	int id;	
	do_gettimeofday(&now);
	
    for ( id=MAPFRAG_IX_INVALID; id < MAPFRAG_MAX_ENTRIES; id++ ) {
        frag_p = &mapfragment.etable[id];

        if ( frag_p->timer.tv_sec == 0 )
            continue;
		else {
			delta = now.tv_sec - frag_p->timer.tv_sec;
			if (delta >= mapfragment.timeout) {
				/* idle too long! */
				mapfrag_delete(id);
			}
		}
    }
}

u32 mapfrag_lookup( const struct in6_addr *v6addr, u32 ipid )
{
    MapFrag_t * frag_p;
    u32 idx;
    u32 hashix;

	refresh_mapfrag_list();
    hashix = _mapfrag_hash(ipid);

    for ( frag_p = mapfragment.htable[ hashix ]; frag_p != MAPFRAG_NULL;
          frag_p = frag_p->chain_p)
    {
        if (likely( _mapfrag_match(frag_p, ipid, v6addr) ))
        {
    		do_gettimeofday(&frag_p->timer);
            return frag_p->idx;
        }
    }

    idx = mapfrag_new(v6addr, ipid, hashix);

    return idx;
}

void mapfrag_get( u32 idx, __be32 *v4addr, struct timeval *timer )
{
    MapFrag_t * frag_p;

    frag_p = &mapfragment.etable[idx];
    *v4addr = frag_p->v4addr;
    timer->tv_sec = frag_p->timer.tv_sec;
    timer->tv_usec = frag_p->timer.tv_usec;

    return;
}

void mapfrag_set( u32 idx, __be32 v4addr )
{
    MapFrag_t * frag_p;

    frag_p = &mapfragment.etable[idx];
    frag_p->v4addr = v4addr;

    return;
}

int init_mapfrag_list( time_t timeout )
{
    register int id;
    MapFrag_t * frag_p;

    memset( (void*)&mapfragment, 0, sizeof(MapFragment_t) );

    /* Initialize list */
    dll_init( &mapfragment.frlist );

    /* Initialize each entry and insert into free list */
    for ( id=MAPFRAG_IX_INVALID; id < MAPFRAG_MAX_ENTRIES; id++ )
    {
        frag_p = &mapfragment.etable[id];
        frag_p->idx = id;

        if ( unlikely(id == MAPFRAG_IX_INVALID) )
            continue;           /* Exclude this entry from the free list */

        dll_append(&mapfragment.frlist, &frag_p->node);/* Insert into free list */
    }

    mapfragment.timeout = timeout;
    
    return 0;
}

int ivi_map_init(void) {
	init_map_list(&tcp_list, 15, MAPPORTMAP_PROTO_TCP);
	init_map_list(&udp_list, 15, MAPPORTMAP_PROTO_UDP);
	init_map_list(&icmp_list, 15, MAPPORTMAP_PROTO_ICMP);
	init_mapfrag_list(15);
#ifdef IVI_DEBUG
	printk(KERN_DEBUG "IVI: ivi_map loaded.\n");
#endif 
	return 0;
}

void ivi_map_exit(void) {
	free_map_list(&tcp_list);
	free_map_list(&udp_list);
	free_map_list(&icmp_list);
#ifdef IVI_DEBUG
	printk(KERN_DEBUG "IVI: ivi_map unloaded.\n");
#endif
}

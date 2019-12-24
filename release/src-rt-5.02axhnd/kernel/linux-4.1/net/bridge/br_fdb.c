/*
 *	Forwarding database
 *	Linux ethernet bridge
 *
 *	Authors:
 *	Lennert Buytenhek		<buytenh@gnu.org>
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version
 *	2 of the License, or (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/rculist.h>
#include <linux/spinlock.h>
#include <linux/times.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/jhash.h>
#include <linux/random.h>
#include <linux/slab.h>
#include <linux/atomic.h>
#include <asm/unaligned.h>
#include <linux/if_vlan.h>
#include "br_private.h"
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
#include <linux/blog.h>
#endif
#if defined(CONFIG_BCM_KF_LOG)
#include <linux/bcm_log.h>
#endif

#if defined(CONFIG_BCM_KF_RUNNER)
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#if defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE)
#include "br_fp.h"
#include "br_fp_hooks.h"
#endif /* CONFIG_BCM_RDPA_BRIDGE || CONFIG_BCM_RDPA_BRIDGE_MODULE */
#endif /* CONFIG_BCM_RDPA || CONFIG_BCM_RDPA_MODULE */
#endif /* CONFIG_BCM_KF_RUNNER */

#if defined(CONFIG_BCM_KF_WL)
#include <linux/module.h>
int (*fdb_check_expired_wl_hook)(unsigned char *addr) = NULL;
int (*fdb_check_expired_dhd_hook)(unsigned char *addr) = NULL;
#endif


static struct kmem_cache *br_fdb_cache __read_mostly;
static struct net_bridge_fdb_entry *fdb_find(struct hlist_head *head,
					     const unsigned char *addr,
					     __u16 vid);
static int fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		      const unsigned char *addr, u16 vid);
static void fdb_notify(struct net_bridge *br,
		       const struct net_bridge_fdb_entry *, int);

static u32 fdb_salt __read_mostly;

int __init br_fdb_init(void)
{
	br_fdb_cache = kmem_cache_create("bridge_fdb_cache",
					 sizeof(struct net_bridge_fdb_entry),
					 0,
					 SLAB_HWCACHE_ALIGN, NULL);
	if (!br_fdb_cache)
		return -ENOMEM;

	get_random_bytes(&fdb_salt, sizeof(fdb_salt));
	return 0;
}

void br_fdb_fini(void)
{
	kmem_cache_destroy(br_fdb_cache);
}


/* if topology_changing then use forward_delay (default 15 sec)
 * otherwise keep longer (default 5 minutes)
 */
static inline unsigned long hold_time(const struct net_bridge *br)
{
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	/* Seems one timer constant in bridge code can serve several different purposes. As we use forward_delay=0,
	if the code left unchanged, every entry in fdb will expire immidately after a topology change and every packet
	will flood the local ports for a period of bridge_max_age. This will result in low throughput after boot up. 
	So we decoulpe this timer from forward_delay. */
	return br->topology_change ? (15*HZ) : br->ageing_time;
#else
	return br->topology_change ? br->forward_delay : br->ageing_time;
#endif
}

static inline int has_expired(const struct net_bridge *br,
				  const struct net_bridge_fdb_entry *fdb)
{
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	blog_lock();
	if (fdb->fdb_key != BLOG_FDB_KEY_INVALID)
		blog_query(QUERY_BRIDGEFDB, (void*)fdb, fdb->fdb_key, 0, 0);
	blog_unlock();
#endif
	return !fdb->is_static &&
		time_before_eq(fdb->updated + hold_time(br), jiffies);
}

static inline int br_mac_hash(const unsigned char *mac, __u16 vid)
{
	/* use 1 byte of OUI and 3 bytes of NIC */
	u32 key = get_unaligned((u32 *)(mac + 2));
#if defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)
	vid = 0;
#endif
	return jhash_2words(key, vid, fdb_salt) & (BR_HASH_SIZE - 1);
}

static void fdb_rcu_free(struct rcu_head *head)
{
	struct net_bridge_fdb_entry *ent
		= container_of(head, struct net_bridge_fdb_entry, rcu);
	kmem_cache_free(br_fdb_cache, ent);
}

/* When a static FDB entry is added, the mac address from the entry is
 * added to the bridge private HW address list and all required ports
 * are then updated with the new information.
 * Called under RTNL.
 */
static void fdb_add_hw_addr(struct net_bridge *br, const unsigned char *addr)
{
	int err;
	struct net_bridge_port *p;

	ASSERT_RTNL();

	list_for_each_entry(p, &br->port_list, list) {
		if (!br_promisc_port(p)) {
			err = dev_uc_add(p->dev, addr);
			if (err)
				goto undo;
		}
	}

	return;
undo:
	list_for_each_entry_continue_reverse(p, &br->port_list, list) {
		if (!br_promisc_port(p))
			dev_uc_del(p->dev, addr);
	}
}

/* When a static FDB entry is deleted, the HW address from that entry is
 * also removed from the bridge private HW address list and updates all
 * the ports with needed information.
 * Called under RTNL.
 */
static void fdb_del_hw_addr(struct net_bridge *br, const unsigned char *addr)
{
	struct net_bridge_port *p;

	ASSERT_RTNL();

	list_for_each_entry(p, &br->port_list, list) {
		if (!br_promisc_port(p))
			dev_uc_del(p->dev, addr);
	}
}

#if defined(CONFIG_BCM_KF_BRIDGE_MAC_FDB_LIMIT) && defined(CONFIG_BCM_BRIDGE_MAC_FDB_LIMIT)
static int fdb_limit_port_max_check(struct net_bridge_port *port)
{
	if (port->max_port_fdb_entries != 0) {
		/* Check per port max limit */
		if ((port->num_port_fdb_entries+1) > port->max_port_fdb_entries)
			return -1;
	}
	return 0;    
}

/*
return 0 if the new learned mac will be one of the reserved mac
return -1 if reserved mac num is not set, or the mac will occupy the non-reserved place*/
static int fdb_limit_port_min_check(struct net_bridge_port *port)
{
	if (port->min_port_fdb_entries == 0) 
        return -1;
    /* Check per port min limit */
    if ((port->num_port_fdb_entries+1) > port->min_port_fdb_entries)
	    return -1;

	return 0;    
}

static int fdb_limit_bridge_check(struct net_bridge *br)
{
	if (br->max_br_fdb_entries != 0) {
		/* Check per br limit */
		if ((br->used_br_fdb_entries+1) > br->max_br_fdb_entries)
			return -1;
	}

	return 0;
}

static int fdb_limit_check(struct net_bridge *br, struct net_bridge_port *port)
{
    /*if excceeds port max, return fail*/
    if(fdb_limit_port_max_check(port))
        return -1;

    /*else if still in port reserved range, return success*/
    if(0 == fdb_limit_port_min_check(port))
        return 0;
    
    /*else depend on bridge max check
    br->used_br_fdb_entries need to be checked only when port reserved range has been excceeded*/
    return fdb_limit_bridge_check(br);
}

static int fdb_limit_mac_move_check(struct net_bridge *br, struct net_bridge_port *from, struct net_bridge_port *to)
{
    /*if excceed port max, return fail*/
    if(fdb_limit_port_max_check(to))
        return -1;
           
    /*else if the mac has already excceeded the from port's reserved places
    which means the bridge still has place for the mac*/
    if (from->num_port_fdb_entries > from->min_port_fdb_entries)
        return 0;
    
    /*else if still in to port reserved range, return success*/
    if(0 == fdb_limit_port_min_check(to))
        return 0;
    
    /*else depend on bridge max check
    br->used_br_fdb_entries need to be checked only when port reserved range has been excceeded*/   
    return fdb_limit_bridge_check(br);    
}

static void fdb_limit_update(struct net_bridge *br, struct net_bridge_port *port, int isAdd)
{
	if (isAdd) {
		port->num_port_fdb_entries++;
		if (port->num_port_fdb_entries > port->min_port_fdb_entries)
			br->used_br_fdb_entries++;
	}
	else {
		BUG_ON(!port->num_port_fdb_entries);
		port->num_port_fdb_entries--;
		if (port->num_port_fdb_entries >= port->min_port_fdb_entries)
			br->used_br_fdb_entries--;
	}        	
}
#endif

static void fdb_delete(struct net_bridge *br, struct net_bridge_fdb_entry *f)
{
	if (f->is_static)
		fdb_del_hw_addr(br, f->addr.addr);
#if defined(CONFIG_BCM_KF_NETFILTER)
	br->num_fdb_entries--;
#endif

#if defined(CONFIG_BCM_KF_BRIDGE_MAC_FDB_LIMIT) && defined(CONFIG_BCM_BRIDGE_MAC_FDB_LIMIT)
	if (f->is_local == 0) {
		fdb_limit_update(br, f->dst, 0);
	}
#endif

#if defined(CONFIG_BCM_KF_RUNNER)
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#if defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE)
	if (!f->is_local) /* Do not remove local MAC to the Runner  */
		br_fp_hook(BR_FP_FDB_REMOVE, f, NULL);
#endif /* CONFIG_BCM_RDPA_BRIDGE || CONFIG_BCM_RDPA_BRIDGE_MODULE */
#endif /* CONFIG_BCM_RDPA || CONFIG_BCM_RDPA_MODULE */
#endif /* CONFIG_BCM_KF_RUNNER */

	hlist_del_rcu(&f->hlist);
	fdb_notify(br, f, RTM_DELNEIGH);
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	blog_lock();
	if (f->fdb_key != BLOG_FDB_KEY_INVALID)
		blog_notify(DESTROY_BRIDGEFDB, (void*)f, f->fdb_key, 0);
	blog_unlock();
#endif
	call_rcu(&f->rcu, fdb_rcu_free);
}

/* Delete a local entry if no other port had the same address. */
static void fdb_delete_local(struct net_bridge *br,
			     const struct net_bridge_port *p,
			     struct net_bridge_fdb_entry *f)
{
	const unsigned char *addr = f->addr.addr;
	u16 vid = f->vlan_id;
	struct net_bridge_port *op;

	/* Maybe another port has same hw addr? */
	list_for_each_entry(op, &br->port_list, list) {
		if (op != p && ether_addr_equal(op->dev->dev_addr, addr) &&
		    (!vid || nbp_vlan_find(op, vid))) {
			f->dst = op;
			f->added_by_user = 0;
			return;
		}
	}

	/* Maybe bridge device has same hw addr? */
	if (p && ether_addr_equal(br->dev->dev_addr, addr) &&
	    (!vid || br_vlan_find(br, vid))) {
		f->dst = NULL;
		f->added_by_user = 0;
		return;
	}

	fdb_delete(br, f);
}

void br_fdb_find_delete_local(struct net_bridge *br,
			      const struct net_bridge_port *p,
			      const unsigned char *addr, u16 vid)
{
	struct hlist_head *head = &br->hash[br_mac_hash(addr, vid)];
	struct net_bridge_fdb_entry *f;

	spin_lock_bh(&br->hash_lock);
	f = fdb_find(head, addr, vid);
	if (f && f->is_local && !f->added_by_user && f->dst == p)
		fdb_delete_local(br, p, f);
	spin_unlock_bh(&br->hash_lock);
}

void br_fdb_changeaddr(struct net_bridge_port *p, const unsigned char *newaddr)
{
	struct net_bridge *br = p->br;
	struct net_port_vlans *pv = nbp_get_vlan_info(p);
	bool no_vlan = !pv;
	int i;
	u16 vid;

	spin_lock_bh(&br->hash_lock);

	/* Search all chains since old address/hash is unknown */
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct hlist_node *h;
		hlist_for_each(h, &br->hash[i]) {
			struct net_bridge_fdb_entry *f;

			f = hlist_entry(h, struct net_bridge_fdb_entry, hlist);
			if (f->dst == p && f->is_local && !f->added_by_user) {
				/* delete old one */
				fdb_delete_local(br, p, f);

				/* if this port has no vlan information
				 * configured, we can safely be done at
				 * this point.
				 */
				if (no_vlan)
					goto insert;
			}
		}
	}

insert:
	/* insert new address,  may fail if invalid address or dup. */
	fdb_insert(br, p, newaddr, 0);

	if (no_vlan)
		goto done;

	/* Now add entries for every VLAN configured on the port.
	 * This function runs under RTNL so the bitmap will not change
	 * from under us.
	 */
	for_each_set_bit(vid, pv->vlan_bitmap, VLAN_N_VID)
		fdb_insert(br, p, newaddr, vid);

done:
	spin_unlock_bh(&br->hash_lock);
}

void br_fdb_change_mac_address(struct net_bridge *br, const u8 *newaddr)
{
	struct net_bridge_fdb_entry *f;
	struct net_port_vlans *pv;
	u16 vid = 0;

	spin_lock_bh(&br->hash_lock);

	/* If old entry was unassociated with any port, then delete it. */
	f = __br_fdb_get(br, br->dev->dev_addr, 0);
	if (f && f->is_local && !f->dst)
		fdb_delete_local(br, NULL, f);

	fdb_insert(br, NULL, newaddr, 0);

	/* Now remove and add entries for every VLAN configured on the
	 * bridge.  This function runs under RTNL so the bitmap will not
	 * change from under us.
	 */
	pv = br_get_vlan_info(br);
	if (!pv)
		goto out;

	for_each_set_bit_from(vid, pv->vlan_bitmap, VLAN_N_VID) {
		f = __br_fdb_get(br, br->dev->dev_addr, vid);
		if (f && f->is_local && !f->dst)
			fdb_delete_local(br, NULL, f);
		fdb_insert(br, NULL, newaddr, vid);
	}
out:
	spin_unlock_bh(&br->hash_lock);
}

void br_fdb_cleanup(unsigned long _data)
{
	struct net_bridge *br = (struct net_bridge *)_data;
	unsigned long delay = hold_time(br);
	unsigned long next_timer = jiffies + br->ageing_time;
	int i;

	spin_lock(&br->hash_lock);
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct net_bridge_fdb_entry *f;
		struct hlist_node *n;

		hlist_for_each_entry_safe(f, n, &br->hash[i], hlist) {
			unsigned long this_timer;
			if (f->is_static)
				continue;
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
			blog_lock();
			if (f->fdb_key != BLOG_FDB_KEY_INVALID)
				blog_query(QUERY_BRIDGEFDB, (void*)f, f->fdb_key, 0, 0);
			blog_unlock();
#endif
			this_timer = f->updated + delay;
			if (time_before_eq(this_timer, jiffies))
#if defined(CONFIG_BCM_KF_RUNNER) || defined(CONFIG_BCM_KF_WL)
			{
				int flag = 0;

#if (defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)) && (defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE))
				br_fp_hook(BR_FP_FDB_CHECK_AGE, f, &flag);
#endif /* CONFIG_BCM_RDPA && CONFIG_BCM_RDPA_BRIDGE && CONFIG_BCM_RDPA_BRIDGE_MODULE */
				if (flag
#if defined(CONFIG_BCM_KF_WL)
				    || (fdb_check_expired_wl_hook && (fdb_check_expired_wl_hook(f->addr.addr) == 0))
				    || (fdb_check_expired_dhd_hook && (fdb_check_expired_dhd_hook(f->addr.addr) == 0))
#endif
				    )
				{
					f->updated = jiffies;
				}
				else
					fdb_delete(br, f);
			}
#else
				fdb_delete(br, f);
#endif
			else if (time_before(this_timer, next_timer))
				next_timer = this_timer;
		}
	}
	spin_unlock(&br->hash_lock);

	mod_timer(&br->gc_timer, round_jiffies_up(next_timer));
}

/* Completely flush all dynamic entries in forwarding database.*/
void br_fdb_flush(struct net_bridge *br)
{
	int i;

	spin_lock_bh(&br->hash_lock);
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct net_bridge_fdb_entry *f;
		struct hlist_node *n;
		hlist_for_each_entry_safe(f, n, &br->hash[i], hlist) {
			if (!f->is_static)
#if defined(CONFIG_BCM_KF_RUNNER) && (defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)) && (defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE))
			{
				int flag = 0;

				br_fp_hook(BR_FP_FDB_CHECK_AGE, f, &flag);
				if (flag) {
					f->updated = jiffies;
				}
				else
				{
					fdb_delete(br, f);
				}
			}
#else /* CONFIG_BCM_KF_RUNNER && CONFIG_BCM_RUNNER && (CONFIG_BCM_RDPA_BRIDGE || CONFIG_BCM_RDPA_BRIDGE_MODULE) */
				fdb_delete(br, f);
#endif /* CONFIG_BCM_KF_RUNNER && CONFIG_BCM_RUNNER && (CONFIG_BCM_RDPA_BRIDGE || CONFIG_BCM_RDPA_BRIDGE_MODULE) */
		}
	}
	spin_unlock_bh(&br->hash_lock);
}

/* Flush all entries referring to a specific port.
 * if do_all is set also flush static entries
 */
void br_fdb_delete_by_port(struct net_bridge *br,
			   const struct net_bridge_port *p,
			   int do_all)
{
	int i;

	spin_lock_bh(&br->hash_lock);
	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct hlist_node *h, *g;

		hlist_for_each_safe(h, g, &br->hash[i]) {
			struct net_bridge_fdb_entry *f
				= hlist_entry(h, struct net_bridge_fdb_entry, hlist);
			if (f->dst != p)
				continue;

			if (f->is_static && !do_all)
				continue;

			if (f->is_local)
				fdb_delete_local(br, p, f);
			else
				fdb_delete(br, f);
		}
	}
	spin_unlock_bh(&br->hash_lock);
}

#if defined(CONFIG_BCM_KF_BRIDGE_MAC_FDB_LIMIT) && defined(CONFIG_BCM_BRIDGE_MAC_FDB_LIMIT)
/* Set FDB limit
   lmtType  0: Bridge limit
            1: Port limit */
int br_set_fdb_limit(struct net_bridge *br, 
						struct net_bridge_port *p,
						int lmt_type,
						int is_min,
						int fdb_limit)
{
	int new_used_fdb;
	
	if((br == NULL) || ((p == NULL) && lmt_type))
		return -EINVAL;

	if(fdb_limit == 0) {
		/* Disable limit */
		if(lmt_type == 0) {
			br->max_br_fdb_entries = 0;
		}
		else if(is_min) {
			if (p->num_port_fdb_entries < p->min_port_fdb_entries) {
				new_used_fdb = br->used_br_fdb_entries - p->min_port_fdb_entries;
				new_used_fdb += p->num_port_fdb_entries;
				br->used_br_fdb_entries = new_used_fdb;
			}
			p->min_port_fdb_entries = 0;
		}
		else {
			p->max_port_fdb_entries = 0;
		}
	}
	else {
		if(lmt_type == 0) {
			if(br->used_br_fdb_entries > fdb_limit) 
				return -EINVAL;
			br->max_br_fdb_entries = fdb_limit;
		}
		else if(is_min) {
			new_used_fdb = max(p->num_port_fdb_entries, p->min_port_fdb_entries);
			new_used_fdb = br->used_br_fdb_entries - new_used_fdb;
			new_used_fdb += max(p->num_port_fdb_entries, fdb_limit);
			if ( (br->max_br_fdb_entries != 0) &&
				(new_used_fdb > br->max_br_fdb_entries) )
				return -EINVAL;

			p->min_port_fdb_entries = fdb_limit;
			br->used_br_fdb_entries = new_used_fdb;
		}
		else {
			if(p->num_port_fdb_entries > fdb_limit)
				return -EINVAL;
			p->max_port_fdb_entries = fdb_limit;
		}
	}
	return 0;
}
#endif

/* No locking or refcounting, assumes caller has rcu_read_lock */
#if defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)
struct net_bridge_fdb_entry *__br_fdb_get(struct net_bridge *br,
					  const unsigned char *addr,
					  __u16 vid __attribute__((unused)))
{
	struct net_bridge_fdb_entry *fdb;

	hlist_for_each_entry_rcu(fdb,
				&br->hash[br_mac_hash(addr, vid)], hlist) {
		if (ether_addr_equal(fdb->addr.addr, addr)) {
			if (unlikely(has_expired(br, fdb)))
				break;
			return fdb;
		}
	}

	return NULL;
}
#else 
struct net_bridge_fdb_entry *__br_fdb_get(struct net_bridge *br,
					  const unsigned char *addr,
					  __u16 vid)
{
	struct net_bridge_fdb_entry *fdb;

	hlist_for_each_entry_rcu(fdb,
				&br->hash[br_mac_hash(addr, vid)], hlist) {
		if (ether_addr_equal(fdb->addr.addr, addr) &&
		    fdb->vlan_id == vid) {
			if (unlikely(has_expired(br, fdb)))
				break;
			return fdb;
		}
	}

	return NULL;
}
#endif

struct net_device *bcmfc_br_fdbdev_get(void *br,
					  const unsigned char *addr,
					  __u16 vid)
{
	struct net_bridge_fdb_entry *fdb;

	if (unlikely(!br || !addr)) {
		printk("%s: br %p addr %p\n", __FUNCTION__, br, addr);
		return NULL;
	}

	rcu_read_lock();
	hlist_for_each_entry_rcu(fdb,
		&((struct net_bridge *)br)->hash[br_mac_hash(addr, vid)], hlist) {
		if (unlikely(!fdb)) {
			printk("%s: fdb is null\n", __FUNCTION__);
			continue;
		}
		if (ether_addr_equal(fdb->addr.addr, addr) &&
		    fdb->vlan_id == vid) {
			rcu_read_unlock();
			return fdb->dst->dev;
		}
	}
	
	rcu_read_unlock();
	return NULL;
}

#if IS_ENABLED(CONFIG_ATM_LANE)
/* Interface used by ATM LANE hook to test
 * if an addr is on some other bridge port */
int br_fdb_test_addr(struct net_device *dev, unsigned char *addr)
{
	struct net_bridge_fdb_entry *fdb;
	struct net_bridge_port *port;
	int ret;

	rcu_read_lock();
	port = br_port_get_rcu(dev);
	if (!port)
		ret = 0;
	else {
		fdb = __br_fdb_get(port->br, addr, 0);
		ret = fdb && fdb->dst && fdb->dst->dev != dev &&
			fdb->dst->state == BR_STATE_FORWARDING;
	}
	rcu_read_unlock();

	return ret;
}
#endif /* CONFIG_ATM_LANE */

/*
 * Fill buffer with forwarding table records in
 * the API format.
 */
int br_fdb_fillbuf(struct net_bridge *br, void *buf,
		   unsigned long maxnum, unsigned long skip)
{
	struct __fdb_entry *fe = buf;
	int i, num = 0;
	struct net_bridge_fdb_entry *f;

	memset(buf, 0, maxnum*sizeof(struct __fdb_entry));

	rcu_read_lock();
	for (i = 0; i < BR_HASH_SIZE; i++) {
		hlist_for_each_entry_rcu(f, &br->hash[i], hlist) {
			if (num >= maxnum)
				goto out;

			if (has_expired(br, f))
				continue;

			/* ignore pseudo entry for local MAC address */
			if (!f->dst)
				continue;

			if (skip) {
				--skip;
				continue;
			}

			/* convert from internal format to API */
			memcpy(fe->mac_addr, f->addr.addr, ETH_ALEN);

			/* due to ABI compat need to split into hi/lo */
			fe->port_no = f->dst->port_no;
			fe->port_hi = f->dst->port_no >> 8;

			fe->is_local = f->is_local;
			if (!f->is_static)
				fe->ageing_timer_value = jiffies_delta_to_clock_t(jiffies - f->updated);
#if defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)
			fe->vid = f->vlan_id;
#endif
			++fe;
			++num;
		}
	}

 out:
	rcu_read_unlock();

	return num;
}

#if defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)
static struct net_bridge_fdb_entry *fdb_find(struct hlist_head *head,
					     const unsigned char *addr,
					     __u16 vid __attribute__((unused)))
{
	struct net_bridge_fdb_entry *fdb;

	hlist_for_each_entry(fdb, head, hlist) {
		if (ether_addr_equal(fdb->addr.addr, addr))
			return fdb;
	}
	return NULL;
}

static struct net_bridge_fdb_entry *fdb_find_rcu(struct hlist_head *head,
						 const unsigned char *addr,
						 __u16 vid __attribute__((unused)))
{
	struct net_bridge_fdb_entry *fdb;

	hlist_for_each_entry_rcu(fdb, head, hlist) {
		if (ether_addr_equal(fdb->addr.addr, addr))
			return fdb;
	}
	return NULL;
}

#else
static struct net_bridge_fdb_entry *fdb_find(struct hlist_head *head,
					     const unsigned char *addr,
					     __u16 vid)
{
	struct net_bridge_fdb_entry *fdb;

	hlist_for_each_entry(fdb, head, hlist) {
		if (ether_addr_equal(fdb->addr.addr, addr) &&
		    fdb->vlan_id == vid)
			return fdb;
	}
	return NULL;
}

static struct net_bridge_fdb_entry *fdb_find_rcu(struct hlist_head *head,
						 const unsigned char *addr,
						 __u16 vid)
{
	struct net_bridge_fdb_entry *fdb;

	hlist_for_each_entry_rcu(fdb, head, hlist) {
		if (ether_addr_equal(fdb->addr.addr, addr) &&
		    fdb->vlan_id == vid)
			return fdb;
	}
	return NULL;
}
#endif /* defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION) */

#if defined(CONFIG_BCM_KF_NETFILTER)
static struct net_bridge_fdb_entry *fdb_create(struct net_bridge *br,
					       struct hlist_head *head,
					       struct net_bridge_port *source,
					       const unsigned char *addr,
					       __u16 vid)
#else
static struct net_bridge_fdb_entry *fdb_create(struct hlist_head *head,
					       struct net_bridge_port *source,
					       const unsigned char *addr,
					       __u16 vid)
#endif
{
	struct net_bridge_fdb_entry *fdb;

#if defined(CONFIG_BCM_KF_NETFILTER)
	if (br->num_fdb_entries >= BR_MAX_FDB_ENTRIES)
		return NULL;
#endif
	fdb = kmem_cache_alloc(br_fdb_cache, GFP_ATOMIC);
	if (fdb) {
		memcpy(fdb->addr.addr, addr, ETH_ALEN);
		fdb->dst = source;
		fdb->vlan_id = vid;
		fdb->is_local = 0;
		fdb->is_static = 0;
		fdb->added_by_user = 0;
		fdb->added_by_external_learn = 0;
		fdb->updated = fdb->used = jiffies;
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
		fdb->fdb_key = BLOG_FDB_KEY_INVALID;
#endif

#if defined(CONFIG_BCM_KF_NETFILTER)
		br->num_fdb_entries++;
#endif

		hlist_add_head_rcu(&fdb->hlist, head);
	}
	return fdb;
}

static int fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		  const unsigned char *addr, u16 vid)
{
	struct hlist_head *head = &br->hash[br_mac_hash(addr, vid)];
	struct net_bridge_fdb_entry *fdb;

	if (!is_valid_ether_addr(addr))
		return -EINVAL;

	fdb = fdb_find(head, addr, vid);
	if (fdb) {
		/* it is okay to have multiple ports with same
		 * address, just use the first one.
		 */
		if (fdb->is_local)
			return 0;
		br_warn(br, "adding interface %s with same address "
		       "as a received packet\n",
		       source ? source->dev->name : br->dev->name);
		fdb_delete(br, fdb);
	}

#if defined(CONFIG_BCM_KF_NETFILTER)
	fdb = fdb_create(br, head, source, addr, vid);
#else
	fdb = fdb_create(head, source, addr, vid);
#endif
	if (!fdb)
		return -ENOMEM;

	fdb->is_local = fdb->is_static = 1;
	fdb_add_hw_addr(br, addr);
	fdb_notify(br, fdb, RTM_NEWNEIGH);
	return 0;
}

/* Add entry for local address of interface */
int br_fdb_insert(struct net_bridge *br, struct net_bridge_port *source,
		  const unsigned char *addr, u16 vid)
{
	int ret;

	vid = BROADSTREAM_IQOS_ENABLE() ? 0 : vid;
	spin_lock_bh(&br->hash_lock);
	ret = fdb_insert(br, source, addr, vid);
	spin_unlock_bh(&br->hash_lock);
	return ret;
}

void br_fdb_update(struct net_bridge *br, struct net_bridge_port *source,
		   const unsigned char *addr, u16 vid, bool added_by_user)
{
	struct hlist_head *head = &br->hash[br_mac_hash(addr, vid)];
	struct net_bridge_fdb_entry *fdb;
	bool fdb_modified = false;

	/* some users want to always flood. */
	if (hold_time(br) == 0)
		return;

	/* ignore packets unless we are using this port */
	if (!(source->state == BR_STATE_LEARNING ||
	      source->state == BR_STATE_FORWARDING))
		return;

	fdb = fdb_find_rcu(head, addr, vid);
	if (likely(fdb)) {
		/* attempt to update an entry for a local interface */
		if (unlikely(fdb->is_local)) {
			if (net_ratelimit())
				br_warn(br, "received packet on %s with "
					"own address as source address\n",
					source->dev->name);
#if defined(CONFIG_BCM_KF_BRIDGE_STATIC_FDB_MOVE)
		} else if ( likely (fdb->is_static == 0)  ) {
			/* don't allow static fdb entries to move */
#else
		} else {
#endif
#if defined(CONFIG_BCM_KF_RUNNER)
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#if defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE)
			struct net_bridge_port *fdb_dst = fdb->dst;
#if defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)
			unsigned int fdb_vid = fdb->vlan_id;
#endif /* CONFIG_BCM_KF_VLAN_AGGREGATION && CONFIG_BCM_VLAN_AGGREGATION */
#endif /* CONFIG_BCM_RDPA_BRIDGE || CONFIG_BCM_RDPA_BRIDGE_MODULE */
#endif /* CONFIG_BCM_RUNNER */
#endif /* CONFIG_BCM_KF_RUNNER */

#if defined(CONFIG_BCM_KF_NETFILTER)
			/* In case of MAC move - let ethernet driver clear switch ARL */
			if (fdb->dst && fdb->dst->port_no != source->port_no) {
				bcmFun_t *ethswClearArlFun;

				/* Get the switch clear ARL function pointer */
				ethswClearArlFun =  bcmFun_get(BCM_FUN_IN_ENET_CLEAR_ARL_ENTRY);
				if ( ethswClearArlFun ) {
					ethswClearArlFun((void*)addr);
				}
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
				blog_lock();
				/* Also flush the associated entries in accelerators */
				if (fdb->fdb_key != BLOG_FDB_KEY_INVALID)
					blog_notify(DESTROY_BRIDGEFDB, (void*)fdb, fdb->fdb_key, 0);
				blog_unlock();
#endif
			}
#endif /* CONFIG_BCM_KF_NETFILTER */
			/* fastpath: update of existing entry */
			if (unlikely(source != fdb->dst)) {
#if defined(CONFIG_BCM_KF_BRIDGE_MAC_FDB_LIMIT) && defined(CONFIG_BCM_BRIDGE_MAC_FDB_LIMIT)
				/* Check that mac can be learned on new port */
				if (fdb_limit_mac_move_check(br, fdb->dst, source) != 0)
				{
					return;
				}
				/* Modify both old and new port counter */
				fdb_limit_update(br, fdb->dst, 0);
				fdb_limit_update(br, source, 1);
#endif /* CONFIG_BCM_KF_BRIDGE_MAC_FDB_LIMIT && CONFIG_BCM_BRIDGE_MAC_FDB_LIMIT */
				fdb->dst = source;
				fdb_modified = true;
			}
			fdb->updated = jiffies;
#if defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)
			fdb->vlan_id = vid;
#endif /* CONFIG_BCM_KF_VLAN_AGGREGATION && CONFIG_BCM_VLAN_AGGREGATION */
#if defined(CONFIG_BCM_KF_RUNNER)
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#if defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE)
#if defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)
			/*  Do not update FastPath if the the source still == dst and vid is same */
			if (fdb_dst != source || fdb_vid != vid)
				br_fp_hook(BR_FP_FDB_MODIFY, fdb, NULL);
#else
			/*  Do not update FastPath if the the source still == dst */
			if (fdb_dst != source)
				br_fp_hook(BR_FP_FDB_MODIFY, fdb, NULL);
#endif /* CONFIG_BCM_KF_VLAN_AGGREGATION && CONFIG_BCM_VLAN_AGGREGATION */
#endif /* CONFIG_BCM_RDPA_BRIDGE || CONFIG_BCM_RDPA_BRIDGE_MODULE */
#endif /* CONFIG_BCM_RUNNER */
#endif /* CONFIG_BCM_KF_RUNNER */

			if (unlikely(added_by_user))
				fdb->added_by_user = 1;
			if (unlikely(fdb_modified))
				fdb_notify(br, fdb, RTM_NEWNEIGH);
		}
	} else {
		spin_lock(&br->hash_lock);
		if (likely(!fdb_find(head, addr, vid))) {
#if defined(CONFIG_BCM_KF_BRIDGE_MAC_FDB_LIMIT) && defined(CONFIG_BCM_BRIDGE_MAC_FDB_LIMIT)
			fdb = NULL; 
			if (fdb_limit_check(br, source) == 0)
#endif
#if defined(CONFIG_BCM_KF_NETFILTER)
			fdb = fdb_create(br, head, source, addr, vid);
#else
			fdb = fdb_create(head, source, addr, vid);
#endif
			if (fdb) {
				if (unlikely(added_by_user))
					fdb->added_by_user = 1;
				fdb_notify(br, fdb, RTM_NEWNEIGH);
#if defined(CONFIG_BCM_KF_NETFILTER)
				/* In case of new MAC - let ethernet driver clear switch ARL */
				if (fdb->dst) {
					bcmFun_t *ethswClearArlFun;
					/* Get the switch clear ARL function pointer */
					ethswClearArlFun =  bcmFun_get(BCM_FUN_IN_ENET_CLEAR_ARL_ENTRY);
					if ( ethswClearArlFun ) {
						struct net_device *dev = fdb->dst->dev;

						while( !netdev_path_is_root(dev) )  // find root device
							dev = netdev_path_next_dev(dev);
						if (!(dev->priv_flags & IFF_EXT_SWITCH))    // clear if root device is not on ext_switch
							ethswClearArlFun((void*)addr);
					}
				}
#endif /* CONFIG_BCM_KF_NETFILTER */

#if defined(CONFIG_BCM_KF_BRIDGE_MAC_FDB_LIMIT) && defined(CONFIG_BCM_BRIDGE_MAC_FDB_LIMIT)
				fdb_limit_update(br, source, 1);
#endif
#if defined(CONFIG_BCM_KF_RUNNER)
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#if defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE)
				br_fp_hook(BR_FP_FDB_ADD, fdb, NULL);
#endif /* CONFIG_BCM_RDPA_BRIDGE || CONFIG_BCM_RDPA_BRIDGE_MODULE */
#endif /* CONFIG_BCM_RUNNER */
#endif /* CONFIG_BCM_KF_RUNNER */
			}
		}
		/* else  we lose race and someone else inserts
		 * it first, don't bother updating
		 */
		spin_unlock(&br->hash_lock);
	}
}

static int fdb_to_nud(const struct net_bridge_fdb_entry *fdb)
{
	if (fdb->is_local)
		return NUD_PERMANENT;
	else if (fdb->is_static)
		return NUD_NOARP;
	else if (has_expired(fdb->dst->br, fdb))
		return NUD_STALE;
	else
		return NUD_REACHABLE;
}

static int fdb_fill_info(struct sk_buff *skb, const struct net_bridge *br,
			 const struct net_bridge_fdb_entry *fdb,
			 u32 portid, u32 seq, int type, unsigned int flags)
{
	unsigned long now = jiffies;
	struct nda_cacheinfo ci;
	struct nlmsghdr *nlh;
	struct ndmsg *ndm;

	nlh = nlmsg_put(skb, portid, seq, type, sizeof(*ndm), flags);
	if (nlh == NULL)
		return -EMSGSIZE;

	ndm = nlmsg_data(nlh);
	ndm->ndm_family	 = AF_BRIDGE;
	ndm->ndm_pad1    = 0;
	ndm->ndm_pad2    = 0;
	ndm->ndm_flags	 = fdb->added_by_external_learn ? NTF_EXT_LEARNED : 0;
	ndm->ndm_type	 = 0;
	ndm->ndm_ifindex = fdb->dst ? fdb->dst->dev->ifindex : br->dev->ifindex;
	ndm->ndm_state   = fdb_to_nud(fdb);

	if (nla_put(skb, NDA_LLADDR, ETH_ALEN, &fdb->addr))
		goto nla_put_failure;
	if (nla_put_u32(skb, NDA_MASTER, br->dev->ifindex))
		goto nla_put_failure;
	ci.ndm_used	 = jiffies_to_clock_t(now - fdb->used);
	ci.ndm_confirmed = 0;
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	blog_lock();
	if (fdb->fdb_key != BLOG_FDB_KEY_INVALID)
		blog_query(QUERY_BRIDGEFDB, (void*)fdb, fdb->fdb_key, 0, 0);
	blog_unlock();
#endif
	ci.ndm_updated	 = jiffies_to_clock_t(now - fdb->updated);
	ci.ndm_refcnt	 = 0;
	if (nla_put(skb, NDA_CACHEINFO, sizeof(ci), &ci))
		goto nla_put_failure;

	if (fdb->vlan_id && nla_put(skb, NDA_VLAN, sizeof(u16), &fdb->vlan_id))
		goto nla_put_failure;

	nlmsg_end(skb, nlh);
	return 0;

nla_put_failure:
	nlmsg_cancel(skb, nlh);
	return -EMSGSIZE;
}

static inline size_t fdb_nlmsg_size(void)
{
	return NLMSG_ALIGN(sizeof(struct ndmsg))
		+ nla_total_size(ETH_ALEN) /* NDA_LLADDR */
		+ nla_total_size(sizeof(u32)) /* NDA_MASTER */
		+ nla_total_size(sizeof(u16)) /* NDA_VLAN */
		+ nla_total_size(sizeof(struct nda_cacheinfo));
}

static void fdb_notify(struct net_bridge *br,
		       const struct net_bridge_fdb_entry *fdb, int type)
{
	struct net *net = dev_net(br->dev);
	struct sk_buff *skb;
	int err = -ENOBUFS;

	skb = nlmsg_new(fdb_nlmsg_size(), GFP_ATOMIC);
	if (skb == NULL)
		goto errout;

	err = fdb_fill_info(skb, br, fdb, 0, 0, type, 0);
	if (err < 0) {
		/* -EMSGSIZE implies BUG in fdb_nlmsg_size() */
		WARN_ON(err == -EMSGSIZE);
		kfree_skb(skb);
		goto errout;
	}
	rtnl_notify(skb, net, 0, RTNLGRP_NEIGH, NULL, GFP_ATOMIC);
	return;
errout:
	rtnl_set_sk_err(net, RTNLGRP_NEIGH, err);
}

/* Dump information about entries, in response to GETNEIGH */
int br_fdb_dump(struct sk_buff *skb,
		struct netlink_callback *cb,
		struct net_device *dev,
		struct net_device *filter_dev,
		int idx)
{
	struct net_bridge *br = netdev_priv(dev);
	int i;

	if (!(dev->priv_flags & IFF_EBRIDGE))
		goto out;

	if (!filter_dev)
		idx = ndo_dflt_fdb_dump(skb, cb, dev, NULL, idx);

	for (i = 0; i < BR_HASH_SIZE; i++) {
		struct net_bridge_fdb_entry *f;

		hlist_for_each_entry_rcu(f, &br->hash[i], hlist) {
			if (idx < cb->args[0])
				goto skip;

			if (filter_dev &&
			    (!f->dst || f->dst->dev != filter_dev)) {
				if (filter_dev != dev)
					goto skip;
				/* !f->dst is a special case for bridge
				 * It means the MAC belongs to the bridge
				 * Therefore need a little more filtering
				 * we only want to dump the !f->dst case
				 */
				if (f->dst)
					goto skip;
			}
			if (!filter_dev && f->dst)
				goto skip;

			if (fdb_fill_info(skb, br, f,
					  NETLINK_CB(cb->skb).portid,
					  cb->nlh->nlmsg_seq,
					  RTM_NEWNEIGH,
					  NLM_F_MULTI) < 0)
				break;
skip:
			++idx;
		}
	}

out:
	return idx;
}

/* Update (create or replace) forwarding database entry */
static int fdb_add_entry(struct net_bridge_port *source, const __u8 *addr,
			 __u16 state, __u16 flags, __u16 vid)
{
	struct net_bridge *br = source->br;
	struct hlist_head *head = &br->hash[br_mac_hash(addr, vid)];
	struct net_bridge_fdb_entry *fdb;
	bool modified = false;

	fdb = fdb_find(head, addr, vid);
	if (fdb == NULL) {
		if (!(flags & NLM_F_CREATE))
			return -ENOENT;

#if defined(CONFIG_BCM_KF_BRIDGE_MAC_FDB_LIMIT) && defined(CONFIG_BCM_BRIDGE_MAC_FDB_LIMIT)
		fdb = NULL; 
		if ((state & NUD_PERMANENT) || (fdb_limit_check(br, source) == 0))
#endif
#if defined(CONFIG_BCM_KF_NETFILTER)
		fdb = fdb_create(br, head, source, addr, vid);
#else
		fdb = fdb_create(head, source, addr, vid);
#endif
		if (!fdb)
			return -ENOMEM;

#if defined(CONFIG_BCM_KF_BRIDGE_MAC_FDB_LIMIT) && defined(CONFIG_BCM_BRIDGE_MAC_FDB_LIMIT)
		if (!(state & NUD_PERMANENT))
		{
			fdb_limit_update(br, source, 1);
		}
#endif
#if defined(CONFIG_BCM_KF_RUNNER)
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#if defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE)
		if (!(state & NUD_PERMANENT))
		{
			br_fp_hook(BR_FP_FDB_ADD, fdb, NULL);
		}
#endif /* CONFIG_BCM_RDPA_BRIDGE || CONFIG_BCM_RDPA_BRIDGE_MODULE */
#endif /* CONFIG_BCM_RUNNER */
#endif /* CONFIG_BCM_KF_RUNNER */
		modified = true;
	} else {
		if (flags & NLM_F_EXCL)
			return -EEXIST;

#if defined(CONFIG_BCM_KF_RUNNER)
#if defined(CONFIG_BCM_RDPA) || defined(CONFIG_BCM_RDPA_MODULE)
#if defined(CONFIG_BCM_RDPA_BRIDGE) || defined(CONFIG_BCM_RDPA_BRIDGE_MODULE)
#if defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)
			/*  Do not update FastPath if the the source still == dst and vid is same */
			if (fdb->dst != source || fdb->vlan_id != vid)
				br_fp_hook(BR_FP_FDB_MODIFY, fdb, NULL);
#else
			/*  Do not update FastPath if the the source still == dst */
			if (fdb->dst != source)
				br_fp_hook(BR_FP_FDB_MODIFY, fdb, NULL);
#endif /* CONFIG_BCM_KF_VLAN_AGGREGATION && CONFIG_BCM_VLAN_AGGREGATION */
#endif /* CONFIG_BCM_RDPA_BRIDGE || CONFIG_BCM_RDPA_BRIDGE_MODULE */
#endif /* CONFIG_BCM_RUNNER */
#endif /* CONFIG_BCM_KF_RUNNER */
		if (fdb->dst != source) {
#if defined(CONFIG_BCM_KF_BRIDGE_MAC_FDB_LIMIT) && defined(CONFIG_BCM_BRIDGE_MAC_FDB_LIMIT)
			if ( !fdb->is_local )
			{
				/* Check that mac can be learned on new port */
				if (fdb_limit_mac_move_check(br, fdb->dst, source) != 0)
				{
					return -EEXIST;
				}
				/* Modify both of old and new port counter */
				fdb_limit_update(br, fdb->dst, 0);
				fdb_limit_update(br, source, 1);
			}
#endif /* CONFIG_BCM_KF_BRIDGE_MAC_FDB_LIMIT && CONFIG_BCM_BRIDGE_MAC_FDB_LIMIT */
			fdb->dst = source;
			modified = true;
		}
	}
#if defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)
	fdb->vlan_id = vid;
#endif /* CONFIG_BCM_KF_VLAN_AGGREGATION && CONFIG_BCM_VLAN_AGGREGATION */
	if (fdb_to_nud(fdb) != state) {
		if (state & NUD_PERMANENT) {
			fdb->is_local = 1;
			if (!fdb->is_static) {
				fdb->is_static = 1;
				fdb_add_hw_addr(br, addr);
			}
		} else if (state & NUD_NOARP) {
			fdb->is_local = 0;
			if (!fdb->is_static) {
				fdb->is_static = 1;
				fdb_add_hw_addr(br, addr);
			}
		} else {
			fdb->is_local = 0;
			if (fdb->is_static) {
				fdb->is_static = 0;
				fdb_del_hw_addr(br, addr);
			}
		}

		modified = true;
	}
	fdb->added_by_user = 1;

	fdb->used = jiffies;
	if (modified) {
		fdb->updated = jiffies;
		fdb_notify(br, fdb, RTM_NEWNEIGH);
	}

	return 0;
}

static int __br_fdb_add(struct ndmsg *ndm, struct net_bridge_port *p,
	       const unsigned char *addr, u16 nlh_flags, u16 vid)
{
	int err = 0;

	if (ndm->ndm_flags & NTF_USE) {
		local_bh_disable();
		rcu_read_lock();
		br_fdb_update(p->br, p, addr, vid, true);
		rcu_read_unlock();
		local_bh_enable();
	} else {
		spin_lock_bh(&p->br->hash_lock);
		err = fdb_add_entry(p, addr, ndm->ndm_state,
				    nlh_flags, vid);
		spin_unlock_bh(&p->br->hash_lock);
	}

	return err;
}

/* Add new permanent fdb entry with RTM_NEWNEIGH */
int br_fdb_add(struct ndmsg *ndm, struct nlattr *tb[],
	       struct net_device *dev,
	       const unsigned char *addr, u16 vid, u16 nlh_flags)
{
	struct net_bridge_port *p;
	int err = 0;
	struct net_port_vlans *pv;

	if (!(ndm->ndm_state & (NUD_PERMANENT|NUD_NOARP|NUD_REACHABLE))) {
		pr_info("bridge: RTM_NEWNEIGH with invalid state %#x\n", ndm->ndm_state);
		return -EINVAL;
	}

	if (is_zero_ether_addr(addr)) {
		pr_info("bridge: RTM_NEWNEIGH with invalid ether address\n");
		return -EINVAL;
	}

	p = br_port_get_rtnl(dev);
	if (p == NULL) {
		pr_info("bridge: RTM_NEWNEIGH %s not a bridge port\n",
			dev->name);
		return -EINVAL;
	}

	pv = nbp_get_vlan_info(p);
	if (vid) {
		if (!pv || !test_bit(vid, pv->vlan_bitmap)) {
			pr_info("bridge: RTM_NEWNEIGH with unconfigured "
				"vlan %d on port %s\n", vid, dev->name);
			return -EINVAL;
		}

		/* VID was specified, so use it. */
		err = __br_fdb_add(ndm, p, addr, nlh_flags, vid);
	} else {
#if defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)
			err = __br_fdb_add(ndm, p, addr, nlh_flags, VLAN_N_VID);
#else
		err = __br_fdb_add(ndm, p, addr, nlh_flags, 0);
#endif
		if (err || !pv)
			goto out;

		/* We have vlans configured on this port and user didn't
		 * specify a VLAN.  To be nice, add/update entry for every
		 * vlan on this port.
		 */
		for_each_set_bit(vid, pv->vlan_bitmap, VLAN_N_VID) {
			err = __br_fdb_add(ndm, p, addr, nlh_flags, vid);
			if (err)
				goto out;
		}
	}

out:
	return err;
}

static int fdb_delete_by_addr(struct net_bridge *br, const u8 *addr, u16 vlan)
{
	struct hlist_head *head = &br->hash[br_mac_hash(addr, vlan)];
	struct net_bridge_fdb_entry *fdb;

	fdb = fdb_find(head, addr, vlan);
	if (!fdb)
		return -ENOENT;

	fdb_delete(br, fdb);
	return 0;
}

static int __br_fdb_delete(struct net_bridge_port *p,
			   const unsigned char *addr, u16 vid)
{
	int err;

	spin_lock_bh(&p->br->hash_lock);
	err = fdb_delete_by_addr(p->br, addr, vid);
	spin_unlock_bh(&p->br->hash_lock);

	return err;
}

/* Remove neighbor entry with RTM_DELNEIGH */
int br_fdb_delete(struct ndmsg *ndm, struct nlattr *tb[],
		  struct net_device *dev,
		  const unsigned char *addr, u16 vid)
{
	struct net_bridge_port *p;
	int err;
	struct net_port_vlans *pv;

	p = br_port_get_rtnl(dev);
	if (p == NULL) {
		pr_info("bridge: RTM_DELNEIGH %s not a bridge port\n",
			dev->name);
		return -EINVAL;
	}

	pv = nbp_get_vlan_info(p);
	if (vid) {
		if (!pv || !test_bit(vid, pv->vlan_bitmap)) {
			pr_info("bridge: RTM_DELNEIGH with unconfigured "
				"vlan %d on port %s\n", vid, dev->name);
			return -EINVAL;
		}

		err = __br_fdb_delete(p, addr, vid);
	} else {
		err = -ENOENT;
		err &= __br_fdb_delete(p, addr, 0);
		if (!pv)
			goto out;

		/* We have vlans configured on this port and user didn't
		 * specify a VLAN.  To be nice, add/update entry for every
		 * vlan on this port.
		 */
		for_each_set_bit(vid, pv->vlan_bitmap, VLAN_N_VID) {
			err &= __br_fdb_delete(p, addr, vid);
		}
	}
out:
	return err;
}

int br_fdb_sync_static(struct net_bridge *br, struct net_bridge_port *p)
{
	struct net_bridge_fdb_entry *fdb, *tmp;
	int i;
	int err;

	ASSERT_RTNL();

	for (i = 0; i < BR_HASH_SIZE; i++) {
		hlist_for_each_entry(fdb, &br->hash[i], hlist) {
			/* We only care for static entries */
			if (!fdb->is_static)
				continue;

			err = dev_uc_add(p->dev, fdb->addr.addr);
			if (err)
				goto rollback;
		}
	}
	return 0;

rollback:
	for (i = 0; i < BR_HASH_SIZE; i++) {
		hlist_for_each_entry(tmp, &br->hash[i], hlist) {
			/* If we reached the fdb that failed, we can stop */
			if (tmp == fdb)
				break;

			/* We only care for static entries */
			if (!tmp->is_static)
				continue;

			dev_uc_del(p->dev, tmp->addr.addr);
		}
	}
	return err;
}

void br_fdb_unsync_static(struct net_bridge *br, struct net_bridge_port *p)
{
	struct net_bridge_fdb_entry *fdb;
	int i;

	ASSERT_RTNL();

	for (i = 0; i < BR_HASH_SIZE; i++) {
		hlist_for_each_entry_rcu(fdb, &br->hash[i], hlist) {
			/* We only care for static entries */
			if (!fdb->is_static)
				continue;

			dev_uc_del(p->dev, fdb->addr.addr);
		}
	}
}

int br_fdb_external_learn_add(struct net_bridge *br, struct net_bridge_port *p,
			      const unsigned char *addr, u16 vid)
{
	struct hlist_head *head;
	struct net_bridge_fdb_entry *fdb;
	int err = 0;

	ASSERT_RTNL();
	spin_lock_bh(&br->hash_lock);

	head = &br->hash[br_mac_hash(addr, vid)];
	fdb = fdb_find(head, addr, vid);
	if (!fdb) {
#if defined(CONFIG_BCM_KF_NETFILTER)
		fdb = fdb_create(br, head, p, addr, vid);
#else
		fdb = fdb_create(head, p, addr, vid);
#endif
		if (!fdb) {
			err = -ENOMEM;
			goto err_unlock;
		}
		fdb->added_by_external_learn = 1;
		fdb_notify(br, fdb, RTM_NEWNEIGH);
	} else if (fdb->added_by_external_learn) {
		/* Refresh entry */
		fdb->updated = fdb->used = jiffies;
	} else if (!fdb->added_by_user) {
		/* Take over SW learned entry */
		fdb->added_by_external_learn = 1;
		fdb->updated = jiffies;
		fdb_notify(br, fdb, RTM_NEWNEIGH);
	}

err_unlock:
	spin_unlock_bh(&br->hash_lock);

	return err;
}

int br_fdb_external_learn_del(struct net_bridge *br, struct net_bridge_port *p,
			      const unsigned char *addr, u16 vid)
{
	struct hlist_head *head;
	struct net_bridge_fdb_entry *fdb;
	int err = 0;

	ASSERT_RTNL();
	spin_lock_bh(&br->hash_lock);

	head = &br->hash[br_mac_hash(addr, vid)];
	fdb = fdb_find(head, addr, vid);
	if (fdb && fdb->added_by_external_learn)
		fdb_delete(br, fdb);
	else
		err = -ENOENT;

	spin_unlock_bh(&br->hash_lock);

	return err;
}

#if defined(CONFIG_BCM_KF_WL)
EXPORT_SYMBOL(fdb_check_expired_wl_hook);
EXPORT_SYMBOL(fdb_check_expired_dhd_hook);
#endif

#if defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)
int br_fdb_get_vid(const unsigned char *addr)
{
	struct net_bridge *br = NULL;
	struct net_bridge_fdb_entry *fdb;
	struct net_device *br_dev;
	int addr_hash = br_mac_hash(addr, 0);
	int vid = -1;

	rcu_read_lock();

	for_each_netdev(&init_net, br_dev){
		if (br_dev->priv_flags & IFF_EBRIDGE) {
			br = netdev_priv(br_dev);
			hlist_for_each_entry_rcu(fdb, &br->hash[addr_hash], hlist) {
				if (ether_addr_equal(fdb->addr.addr, addr)) {
					if (unlikely(!has_expired(br, fdb)))
						vid = (int)fdb->vlan_id;
					break;
				}
			}
		}          
	}

	rcu_read_unlock();
	return vid;
}
EXPORT_SYMBOL(br_fdb_get_vid);
#endif //defined(CONFIG_BCM_KF_VLAN_AGGREGATION) && defined(CONFIG_BCM_VLAN_AGGREGATION)

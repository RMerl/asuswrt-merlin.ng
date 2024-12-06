/*
	<:copyright-BRCM:2023:DUAL/GPL:standard

	Copyright (c) 2023 Broadcom
	All Rights Reserved

	Unless you and Broadcom execute a separate written software license
	agreement governing use of this software, this software is licensed
	to you under the terms of the GNU General Public License version 2
	(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
	with the following added to such license:

	As a special exception, the copyright holders of this software give
	you permission to link this software with independent modules, and
	to copy and distribute the resulting executable under terms of your
	choice, provided that you also meet, for each linked independent
	module, the terms and conditions of the license of that module.
	An independent module is a module which is not derived from this
	software.  The special exception does not apply to any modifications
	of the software.

	Not withstanding the above, under no circumstances may you combine
	this software in any way with any other Broadcom software provided
	under a license other than the GPL, without Broadcom's express prior
	written consent.

	:>
 */

/* Interface to configure firewall rules in HW accelerator  */

#include <linux/types.h>
#include <linux/module.h>
#include <linux/bcm_log.h>
#include <linux/seq_file.h>
#include <linux/proc_fs.h>
#include <linux/skbuff.h>
#include <linux/kthread.h>

/* for ct debug  functions */
#define DEBUG

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_expect.h>

#include <linux/bcm_nf_conntrack.h>
#include <linux/bcm_nf_expect.h>

#include "hwf_ioctl.h"
#include "hwf.h"

extern int bcm_ct_limit_init(void);
extern void  bcm_ct_limit_exit(void);
extern int bcm_hwf_limit_ct_event_process(enum ip_conntrack_events events, 
		const struct nf_conn *ct, struct sk_buff *skb);
extern bool bcm_hwf_limit_pkt_rcv(union nf_inet_addr *l3addr, uint16_t l3proto);

extern int hwf_cdev_init(void);
extern void hwf_cdev_deinit(void);

extern void hwf_cdev_deinit(void);
extern struct bcm_hosts bcm_hosts_g;

#if IS_ENABLED(CONFIG_RNR_HW_FIREWALL)
#include "hwf_runner.h"
#else
static inline long  hwf_hw_node_add(bcm_hwf_obj_t *hwf_obj, bcm_hwf_ratelimiter_t *hwf_ratelimiter)
{ return HWF_INVALID_HW_IDX; };
static inline int hwf_hw_node_delete(bcm_hwf_obj_t *hwf_obj) {return 0; };
static int hwf_hw_config_enable(int enable) {return 0; };
static int __init hwf_hw_init(void) {return 0; };
static int __init hwf_hw_exit(void) {return 0; };
#endif


#define HWF_HASH_EXP 12
#define HWF_HASHTBL_SIZE (1<<HWF_HASH_EXP)


struct bcm_hwf {
	DECLARE_HASHTABLE(ct_hashtbl, HWF_HASH_EXP); /* 4096 hash buckets */
	spinlock_t ct_tbl_lock;

	spinlock_t cfg_lock; /*used for sync from userspace*/
	 struct task_struct     *event_thread;
	unsigned long event_thread_intvl; /* periodic refresh interval*/
	unsigned long stats_refresh_min_intvl; /* min interval between stats pull for HW*/
	unsigned long stats_next_refresh; /* time after which next refersh can be done*/

	bcm_hwf_ratelimiter_t ratelimiters[HWF_MAX_RATELIMITERS];
	bcm_hwf_vserver_t vservers[HWF_MAX_VSERVERS];
	uint64_t hw_ct_lookup_miss_cnt; /*total ct miss*/
	uint64_t hw_ct_miss_ratelimit_drop_cnt; /*drops by meters/policers on ddosq */
	uint64_t hw_ct_miss_tail_drop_cnt; /*ddsoq taildrops, when host is busy*/
	uint64_t hw_ct_miss_rx_cnt; /* total passed to host via ddosq */
	uint64_t bypass_pkts;
	uint16_t num_ratelimiters;
	uint16_t num_vservers;

	bool hwf_enable; /* hwf 5 tuple lookup */
	bool expect_lookup_enable; /* exp/vserver 3 tuple lookup */
	bool wan_miss_ratelimit_enable; /* ratelimit on DDOS queue */
	bool lan_ct_limit_enable; /* Conn ratelimit */
} bcm_hwf_g;

hwf_procfs_stat_t hwf_stat_g; 

#define BCM_HWF_CT_TABLE_LOCK()		spin_lock_bh(&bcm_hwf_g.ct_tbl_lock)
#define BCM_HWF_CT_TABLE_UNLOCK()		spin_unlock_bh(&bcm_hwf_g.ct_tbl_lock)

#define BCM_HWF_CONFIG_LOCK()		spin_lock_bh(&bcm_hwf_g.cfg_lock)
#define BCM_HWF_CONFIG_UNLOCK()		spin_unlock_bh(&bcm_hwf_g.cfg_lock)

static unsigned int hwf_hash_rnd __read_mostly;
static int bcm_hwf_debug_lvl_g __read_mostly;

static inline uint32_t hwf_tuple_to_hashid(const struct nf_conntrack_tuple *tuple)
{
	unsigned int n;
	/* Adapted from hash_conntrack_raw();
	 * The direction must be ignored, so we hash everything up to the
	 * destination ports (which is a multiple of 4) and treat the last
	 * three bytes manually.
	 */
	n = (sizeof(tuple->src) + sizeof(tuple->dst.u3)) / sizeof(u32);
	return jhash2((u32 *)tuple, n, hwf_hash_rnd ^
				(((__force __u16)tuple->dst.u.all << 16) |
				tuple->dst.protonum));
}

static inline bool hwf_tuple_is_equal(const struct nf_conntrack_tuple *tuple1,
		const struct nf_conntrack_tuple *tuple2)
{
	/*ignore net & zone */
	return nf_ct_tuple_equal(tuple1, tuple2);
}

static inline bcm_hwf_obj_t *hwf_obj_alloc(void)
{
	bcm_hwf_obj_t *hwf_obj;

	hwf_obj = kmalloc(sizeof(bcm_hwf_obj_t), GFP_ATOMIC);
	return hwf_obj;
}

static inline void hwf_obj_free(bcm_hwf_obj_t *hwf_obj)
{
	kfree(hwf_obj);
}

static inline int hwf_hash_add(bcm_hwf_obj_t *hwf_obj)
{
	uint32_t hashid = hwf_tuple_to_hashid(&hwf_obj->tuple);

	INIT_HLIST_NODE(&hwf_obj->hnode);
	/*add obj to hash */
	hash_add(bcm_hwf_g.ct_hashtbl, &hwf_obj->hnode, hashid);

	return 0;
}

/*caller should have a lock */
static inline bcm_hwf_obj_t *hwf_hash_lookup(const struct nf_conntrack_tuple *tuple)
{
	bcm_hwf_obj_t *hwf_obj;
	uint32_t hashid = hwf_tuple_to_hashid(tuple);

	hash_for_each_possible(bcm_hwf_g.ct_hashtbl, hwf_obj, hnode, hashid) {
		if (hwf_tuple_is_equal(&hwf_obj->tuple, tuple))
			return hwf_obj;
	}
	return NULL;
}

static inline int hwf_node_add(const struct nf_conntrack_tuple *key_tuple, bool is_exp,
		bcm_hwf_ratelimiter_t *ratelimiter)
{
	bcm_hwf_obj_t *hwf_obj = hwf_obj_alloc();

	if (!hwf_obj) {
		pr_warn("%s: hwf_obj allocation failure\n", __func__);
		return -1;
	}
	hwf_obj->refcnt = 1;
	hwf_obj->flags = 0;
	hwf_obj->is_exp = is_exp;
	memcpy(&hwf_obj->tuple, key_tuple, sizeof(struct nf_conntrack_tuple));
	if (ratelimiter)
		hwf_obj->is_ratelimiter = 1;

	hwf_hash_add(hwf_obj);

	hwf_obj->hwid = hwf_hw_node_add(hwf_obj, ratelimiter);

	if (bcm_hwf_debug_lvl_g >= HWF_DEBUG_INFO)
		printk("%s:tuple added hwid = %ld\n", __func__, hwf_obj->hwid);

	if (hwf_obj->hwid == HWF_INVALID_HW_IDX)
		return HWF_ERROR;
	else
		return HWF_SUCCESS;
}
static inline int hwf_node_get(bcm_hwf_obj_t *hwf_obj)
{
	hwf_obj->refcnt++;
	return 0;
}

static inline void hwf_node_delete(bcm_hwf_obj_t *hwf_obj)
{
	hash_del(&hwf_obj->hnode);

	if (hwf_obj->hwid != HWF_INVALID_HW_IDX)
		hwf_hw_node_delete(hwf_obj);

	if (bcm_hwf_debug_lvl_g >= HWF_DEBUG_INFO)
		printk("%s:tuple deleted hwid = %ld\n", __func__, hwf_obj->hwid);

	hwf_obj_free(hwf_obj);
}

static inline void hwf_node_put(bcm_hwf_obj_t *hwf_obj)
{
	hwf_obj->refcnt--;
	if (hwf_obj->refcnt == 0)
		hwf_node_delete(hwf_obj);
}

static inline int hwf_hash_del_bytuple(const struct nf_conntrack_tuple *tuple)
{
	bcm_hwf_obj_t *hwf_obj;
	struct hlist_node *tmpnode;
	uint32_t hashid = hwf_tuple_to_hashid(tuple);

	/* remove obj from hwf hash*/
	hash_for_each_possible_safe(bcm_hwf_g.ct_hashtbl, hwf_obj, tmpnode, hnode, hashid) {
		if (hwf_tuple_is_equal(&hwf_obj->tuple, tuple)) {
			hwf_node_put(hwf_obj);
			return 0;
		}
	}
	/* error if obj does not exist */
	pr_warn("%s:Object not found\n", __func__);
	return -1;
}

static inline void hwf_node_force_delete(bcm_hwf_obj_t *hwf_obj)
{
	hwf_obj->refcnt = 1; /*force refcnt to 1 */
	hwf_node_put(hwf_obj);
}

static inline void bcm_hwf_flush_all(bool delete_static)
{
	bcm_hwf_obj_t *hwf_obj;
	struct hlist_node *tmpnode;
	int bkt;

	BCM_HWF_CT_TABLE_LOCK();

	hash_for_each_safe(bcm_hwf_g.ct_hashtbl, bkt, tmpnode, hwf_obj, hnode) {
		if (delete_static || !hwf_obj->is_static)
		hwf_node_force_delete(hwf_obj);
	}

	BCM_HWF_CT_TABLE_UNLOCK();
}

static inline void bcm_hwf_flush_expect(bool delete_static)
{
	bcm_hwf_obj_t *hwf_obj;
	struct hlist_node *tmpnode;
	int bkt;

	BCM_HWF_CT_TABLE_LOCK();

	hash_for_each_safe(bcm_hwf_g.ct_hashtbl, bkt, tmpnode, hwf_obj, hnode) {
		if (hwf_obj->is_exp && (delete_static || !hwf_obj->is_static))
			hwf_node_force_delete(hwf_obj);
	}
	BCM_HWF_CT_TABLE_UNLOCK();
}

static inline int hwf_tbl_update(bool add, const struct nf_conntrack_tuple *key_tuple, bool is_exp)
{
	bcm_hwf_obj_t *hwf_obj;

	BCM_HWF_CT_TABLE_LOCK();
	hwf_obj = hwf_hash_lookup(key_tuple);

	if (add) {
		if (likely(!hwf_obj))
			hwf_node_add(key_tuple, is_exp, NULL);/* allocate node */
		else
			hwf_node_get(hwf_obj);
	} else {
		if (hwf_obj)
			hwf_node_put(hwf_obj);
		else {
			if (bcm_hwf_debug_lvl_g >= HWF_DEBUG_WARN) {
				printk("%s :tuple not found\n", __func__);
				nf_ct_dump_tuple(key_tuple);
			}
		}
	}
	BCM_HWF_CT_TABLE_UNLOCK();

	return 0;
}

static inline int hwf_tbl_update_vserver(bool add, const struct nf_conntrack_tuple *key_tuple,
		bcm_hwf_ratelimiter_t *ratelimiter)
{
	bcm_hwf_obj_t *hwf_obj;
	int ret = HWF_SUCCESS;

	BCM_HWF_CT_TABLE_LOCK();
	hwf_obj = hwf_hash_lookup(key_tuple);

	if (add) {
		if (likely(!hwf_obj))
			ret = hwf_node_add(key_tuple, true, ratelimiter);/* allocate node */
		else {	
			pr_err("%s :vserver hw obj already exists\n", __func__);
			nf_ct_dump_tuple(key_tuple);
			ret = HWF_ERROR; 
		}
	} else {
		if (hwf_obj)
			hwf_node_put(hwf_obj);
		else {
			pr_err("%s :vserver hw obj  not found\n", __func__);
			nf_ct_dump_tuple(key_tuple);
			ret = HWF_ERROR;
		}
	}
	BCM_HWF_CT_TABLE_UNLOCK();
	return ret; 
}

/*
 *IP_CT_NEW - new connection ,use reply tuple for WAN side flow
 *IP_CT_EXPECTED - use original tuple for WAN side
 *IP_CT_DESTROY  - delete connection, how to decide which tuple to use ?
 *	- ctstatus - IP_CT_EXPECTED - use orig tuple else use reply tuple
 *igonre net & zone, just use 5-tuple & maintain refcnt to handle conflict
 */


static int
bcm_hwf_ct_event(enum ip_conntrack_events events, const struct nf_conn *ct, struct sk_buff *skb)
{
	const struct nf_conntrack_tuple *key_tuple;
	const struct nf_conntrack_tuple *lan_key_tuple;
	bool add = false;

	if (unlikely(!bcm_hwf_g.hwf_enable) || !nf_ct_is_confirmed(ct))
		return 0;

	if (test_bit(IPS_EXPECTED_BIT, &ct->status)) {
		key_tuple = nf_ct_tuple(ct, IP_CT_DIR_ORIGINAL);
		lan_key_tuple = nf_ct_tuple(ct, IP_CT_DIR_REPLY);
	} else {
		key_tuple = nf_ct_tuple(ct, IP_CT_DIR_REPLY);
		lan_key_tuple = nf_ct_tuple(ct, IP_CT_DIR_ORIGINAL);
	}

	/*TODO add more filter to determine if tuple needs to be added */

	if (events & (1 << IPCT_DESTROY)) {
		add = false;
	} else if (events & ((1 << IPCT_NEW) | (1 << IPCT_RELATED))) {
		add = true;
#if 0
	} else if (events & (1 << IPCT_REPLY)) {
		/*TODO: remove entries if ct not for WAN */
#endif
	} else
		return 0;

	if (bcm_hwf_debug_lvl_g >= HWF_DEBUG_INFO) {
		printk("%s : add=%d  new=%d related=%d destroy=%d ", __func__,
				add, (events & (1 << IPCT_NEW)), (events & (1 << IPCT_RELATED)),
				(events & (1 << IPCT_DESTROY)));
		nf_ct_dump_tuple(key_tuple);
	}

	hwf_tbl_update(add, key_tuple, false);

	if (bcm_hwf_g.lan_ct_limit_enable) {
		hwf_tbl_update(add, lan_key_tuple, false);
		bcm_hwf_limit_ct_event_process(events, ct, skb);
	}
	return 0;
}

static int
bcm_hwf_expect_event(enum ip_conntrack_expect_events events, struct nf_conntrack_expect *exp)
{
	struct nf_conntrack_tuple key_tuple = {};
	bool add = false;

	if (unlikely(!bcm_hwf_g.hwf_enable || !bcm_hwf_g.expect_lookup_enable))
		return 0;

	/* only use DSTIP DSTPORT, L4 & L3 proto */
	memcpy(&key_tuple.dst.u3, &exp->tuple.dst.u3, sizeof(key_tuple.dst.u3));
	key_tuple.dst.u.all = exp->tuple.dst.u.all;
	key_tuple.dst.protonum = exp->tuple.dst.protonum;
	key_tuple.src.l3num = exp->tuple.src.l3num;

	if (events & (1 << IPEXP_DESTROY))
		add = false;
	else if (events & (1 << IPEXP_NEW))
		add = true;
	else
		return 0;

	return hwf_tbl_update(add, &key_tuple, true);
}

extern int (*blog_hwf_fn)(void *arg1, void *arg2, void *arg3);

int bcm_hwf_pkt_rcv(void *arg1, void *arg2, void *arg3)
{
	BlogFcArgs_t *fc_args = arg1;
	struct net_device *rx_dev = arg2;
	BlogTuple_t  *tupleV4;
	BlogTupleV6_t  *tupleV6;
	union nf_inet_addr *l3addr;
	struct nf_conntrack_tuple key_tuple;
	uint16_t l3proto;
	int ret = HWF_ALLOW;
	bool ct_miss = false;
	 

	if ((bcm_hwf_g.lan_ct_limit_enable) && !is_netdev_wan(rx_dev)) {

		if (!fc_args->is_hwf_data_valid) {
			bcm_hwf_g.bypass_pkts++;
			goto done;
			/* TODO better to reparse the packet by getting innerip & outerip offsets
			 * to handle ip options,frags etc..
			 */ 
		}

		/*TODO add logic to decide inner vs outer */

		/*use outer tuple */
		if (fc_args->is_rx_outer_ipv6) {
			tupleV6 = fc_args->rx_outer_tuple;
			l3addr = (union nf_inet_addr *)&tupleV6->saddr.p32[0];
			l3proto = AF_INET6;
		} else {
			tupleV4 = fc_args->rx_outer_tuple;
			l3addr = (union nf_inet_addr *)&tupleV4->saddr;
			l3proto = AF_INET;
		}

		/* do ct lookup if not done by hw */
		if (!fc_args->is_rx_ddos_q) {
			bcm_hwf_obj_t *hwf_obj;

			/*build key_tuple */
			memset(&key_tuple, 0, sizeof(key_tuple));
			key_tuple.src.l3num = l3proto;
			key_tuple.dst.protonum = fc_args->rx_outer_l4proto;
			if (l3proto == AF_INET) {
				key_tuple.src.u3.ip = tupleV4->saddr;
				key_tuple.dst.u3.ip = tupleV4->daddr;
				key_tuple.src.u.all = tupleV4->port.source;
				key_tuple.dst.u.all = tupleV4->port.dest;
			} else {
				memcpy(key_tuple.src.u3.ip6, tupleV6->saddr.p32, sizeof(key_tuple.src.u3.ip6));
				memcpy(key_tuple.dst.u3.ip6, tupleV6->daddr.p32, sizeof(key_tuple.dst.u3.ip6));
				key_tuple.src.u.all = tupleV6->port.source;
				key_tuple.dst.u.all = tupleV6->port.dest;
			}

			BCM_HWF_CT_TABLE_LOCK();
			hwf_obj = hwf_hash_lookup(&key_tuple);
			BCM_HWF_CT_TABLE_UNLOCK();

			if(!hwf_obj) {
				ct_miss = true;
				if ((bcm_hwf_debug_lvl_g >= HWF_DEBUG_INFO))
					nf_ct_dump_tuple(&key_tuple);
			}
		}

		if (ct_miss && !bcm_hwf_limit_pkt_rcv(l3addr, l3proto)) 
			ret = HWF_DROP;
	}
done:
	return ret;
}


int bcm_hwf_enable(bool enable)
{
	int ret = HWF_SUCCESS;

	if (enable == false) {
		ret = hwf_hw_config_enable(0);
		bcm_hwf_flush_all(false);

	} else
		ret = hwf_hw_config_enable(1);

	/*TODO meter & ct limit disable at run time, but maintain configured value, to restore */
	bcm_hwf_g.hwf_enable = enable;
	return ret;
}

int bcm_hwf_expect_lookup_enable(bool enable)
{
	int ret = 0; 

	bcm_hwf_flush_expect(false);

	hwf_hw_config_ct_expect_enable(enable);
	bcm_hwf_g.expect_lookup_enable = enable;
	return ret;
}

int bcm_hwf_wan_miss_ratelimit_enable(bool enable)
{
	/*TODO can we disable runner meter without deleting, otherwise this func is not needed */

	bcm_hwf_g.wan_miss_ratelimit_enable = enable;
	return HWF_SUCCESS;
}

int bcm_hwf_lan_ct_limit_enable(bool enable)
{
	bcm_hwf_g.lan_ct_limit_enable = enable;
	return HWF_SUCCESS;
}

int bcm_hwf_status(hwfctl_data_t *hwfctl)
{
	hwfctl->info.config.hwf_enable = bcm_hwf_g.hwf_enable;

	hwfctl->info.config.expect_lookup_enable = bcm_hwf_g.expect_lookup_enable;
	hwfctl->info.config.wan_miss_ratelimit = bcm_hwf_g.wan_miss_ratelimit_enable;
	hwfctl->info.config.lan_ct_limit = bcm_hwf_g.lan_ct_limit_enable;

	return HWF_SUCCESS;
}

static inline bcm_hwf_ratelimiter_t *hwf_ratelimiter_alloc(void)
{
	bcm_hwf_ratelimiter_t *ratelimiter;
	int i;

	for (i=0; i<HWF_MAX_RATELIMITERS; i++) {
		ratelimiter = &bcm_hwf_g.ratelimiters[i];

		if (!ratelimiter->valid) {
			ratelimiter->valid = 1;		
			bcm_hwf_g.num_ratelimiters++;
			return ratelimiter;
		}
	}
	return NULL;
}

static inline void hwf_ratelimiter_free(bcm_hwf_ratelimiter_t *ratelimiter)
{
	ratelimiter->valid = 0;		
	bcm_hwf_g.num_ratelimiters--;
}

bcm_hwf_ratelimiter_t *bcm_hwf_ratelimiter_get_byname(const char *name)
{
	int i;
	bcm_hwf_ratelimiter_t *ratelimiter = NULL;

	for (i=0; i < HWF_MAX_RATELIMITERS; i++) {
		ratelimiter = &bcm_hwf_g.ratelimiters[i];

		if ((ratelimiter->valid) && !strcmp(ratelimiter->name, name))
			return ratelimiter;
	}
	return NULL;
}

bcm_hwf_ratelimiter_t *bcm_hwf_ratelimiter_get_bytype(hwfctl_ratelimit_type_t type)
{
	int i;
	bcm_hwf_ratelimiter_t *ratelimiter = NULL;

	for (i=0; i < HWF_MAX_RATELIMITERS; i++) {
		ratelimiter = &bcm_hwf_g.ratelimiters[i];

		if ((ratelimiter->valid) && (ratelimiter->type == type))
			return ratelimiter;
	}
	return NULL;
}

static inline char* hwf_ratelimter_type_id_to_str(hwfctl_ratelimit_type_t type)
{
	if (type == hwfctl_ratelimit_vserver) 
		return "vserver";
	else if (type == hwfctl_ratelimit_wan_miss_all) 
		return "wan_miss_all";
	else 
		return "unknown";
}

int bcm_hwf_ratelimiter_add(hwfctl_ratelimiter_t *ctl_ratelimiter)
{
	bcm_hwf_ratelimiter_t *ratelimiter;
	int ret = -ENOMEM;

	BCM_HWF_CONFIG_LOCK();
	ratelimiter = bcm_hwf_ratelimiter_get_byname(ctl_ratelimiter->name);
	if (ratelimiter) {
		pr_err("%s: ratelimiter %s already exists \n", __func__, ctl_ratelimiter->name);
		goto done;
	}

	/* check for wan_miss_allm only one allowed */
	if (ctl_ratelimiter->type == hwfctl_ratelimit_wan_miss_all) {
		ratelimiter = bcm_hwf_ratelimiter_get_bytype(ctl_ratelimiter->type);
		if (ratelimiter) {
			pr_err("%s: ratelimiter type %s already exists \n", __func__,
					hwf_ratelimter_type_id_to_str(ctl_ratelimiter->type));
			goto done;
		}
	}

	ratelimiter = hwf_ratelimiter_alloc();

	if (ratelimiter){
		ratelimiter->pps_rate = ctl_ratelimiter->rate;
		ratelimiter->pps_burst_size = ctl_ratelimiter->burst;
		ratelimiter->type = ctl_ratelimiter->type;
		strcpy(ratelimiter->name, ctl_ratelimiter->name);
		ratelimiter->ref_count = 1;
		/*add ratelimiter in HW */
		ret = hwf_hw_ratelimiter_add(ratelimiter);
		if(ret) {
			pr_err("%s: HW ratelimiter add failed ret=%d\n", __func__, ret);
			hwf_ratelimiter_free(ratelimiter);
		}
	} else {
			pr_err("%s: Meter table full num_ratelimiters=%d\n", __func__, bcm_hwf_g.num_ratelimiters);
	}
done:
	BCM_HWF_CONFIG_UNLOCK();
	return ret;
}

int bcm_hwf_ratelimiter_update(hwfctl_ratelimiter_t *ctl_ratelimiter)
{
	bcm_hwf_ratelimiter_t *ratelimiter;
	int ret = -1;

	BCM_HWF_CONFIG_LOCK();

	ratelimiter = bcm_hwf_ratelimiter_get_byname(ctl_ratelimiter->name);
	if (!ratelimiter) {
		pr_err("%s: ratelimiter %s not found \n", __func__, ctl_ratelimiter->name);
		goto done;
	}

	if ((ctl_ratelimiter->type != 0) && (ctl_ratelimiter->type != ratelimiter->type)) {
		pr_err("%s: ratelimiter %s type cannot be updated\n", __func__, ctl_ratelimiter->name);
		goto done;
	}

	ratelimiter->pps_rate = ctl_ratelimiter->rate;
	ratelimiter->pps_burst_size = ctl_ratelimiter->burst;
	/*TODO reset any counters */
	ret = hwf_hw_ratelimiter_modify(ratelimiter);
done:
	BCM_HWF_CONFIG_UNLOCK();
	return ret;
}

int bcm_hwf_ratelimiter_delete(const char *name)
{
	bcm_hwf_ratelimiter_t *ratelimiter;
	int ret = -1;

	BCM_HWF_CONFIG_LOCK();

	ratelimiter = bcm_hwf_ratelimiter_get_byname(name);
	
	if (ratelimiter) {
		if (ratelimiter->ref_count != 1) {
			pr_err("%s: ratelimiter %s is in use refcnt=%d \n", __func__, name, ratelimiter->ref_count);
			goto done;
		}
		ret = hwf_hw_ratelimiter_delete(ratelimiter);
		/*TODO not sure if HW delete can fail, shoud we retry? */
		hwf_ratelimiter_free(ratelimiter);
	} else {
		pr_err("%s: ratelimiter %s not found \n", __func__, name);
	}
done:
	BCM_HWF_CONFIG_UNLOCK();
	return ret;
}


static inline bcm_hwf_vserver_t *hwf_vserver_alloc(void)
{
	bcm_hwf_vserver_t *vserver;
	int i;

	for (i=0; i < HWF_MAX_VSERVERS; i++) {
		vserver = &bcm_hwf_g.vservers[i];

		if (!vserver->valid) {
			vserver->valid = 1;
			bcm_hwf_g.num_vservers++;
			return vserver;
		}
	}
	return NULL;
}

static inline void hwf_vserver_free(bcm_hwf_vserver_t *vserver)
{
	vserver->valid = 0;
	bcm_hwf_g.num_vservers--;
}

static inline bool hwf_addr_is_equal(const union nf_inet_addr *a1,
					const union nf_inet_addr *a2, uint16_t l3proto)
{
	if(l3proto == AF_INET)
		return a1->all[0] == a2->all[0];
	else 
		return a1->all[0] == a2->all[0] &&
			a1->all[1] == a2->all[1] &&
			a1->all[2] == a2->all[2] &&
			a1->all[3] == a2->all[3];
}

static inline  bcm_hwf_vserver_t *hwf_vserver_lookup(struct nf_conntrack_tuple *key_tuple)
{
	int i;
	bcm_hwf_vserver_t *vserver = NULL;

	for (i=0; i < HWF_MAX_VSERVERS; i++) {

		vserver = &bcm_hwf_g.vservers[i];
		if ((vserver->valid) && (vserver->dst_port == key_tuple->dst.u.all)
		  && (vserver->l4_proto == key_tuple->dst.protonum)
		  && (vserver->l3num == key_tuple->src.l3num)
		  && hwf_addr_is_equal(&key_tuple->dst.u3, &vserver->daddr, vserver->l3num))
		{
			return vserver;
		}
	}
	return NULL;
}

static inline bcm_hwf_vserver_t *hwf_vserver_get_byname(const char *name)
{
	int i;
	bcm_hwf_vserver_t *vserver=NULL;

	for (i=0; i < HWF_MAX_VSERVERS; i++) {
		vserver = &bcm_hwf_g.vservers[i];

		if ((vserver->valid) && !strcmp(vserver->name, name))
			return vserver;
	}
	return NULL;
}

int bcm_hwf_vserver_add(hwfctl_vserver_t *ctl_vserver)
{
	bcm_hwf_vserver_t *vserver;
	int ret = HWF_SUCCESS;
	struct nf_conntrack_tuple key_tuple = {};
	bcm_hwf_ratelimiter_t *ratelimiter = NULL;

	/* build tuple */
	memcpy(&key_tuple.dst.u3, (union nf_inet_addr *)ctl_vserver->dst_ip, sizeof(key_tuple.dst.u3));
	/*convert to network order */
	hwf_swap_ipaddr(&key_tuple.dst.u3);
	key_tuple.dst.u.all = htons(ctl_vserver->dst_port);
	key_tuple.dst.protonum = ctl_vserver->l4_proto;
	key_tuple.src.l3num = ctl_vserver->is_ipv6? AF_INET6 : AF_INET;

	BCM_HWF_CONFIG_LOCK();
	/*lookup vserver by DSTIP+PORT, protocol */	
	vserver = hwf_vserver_lookup(&key_tuple);
	if (vserver) {
		pr_err("\n%s: vserver tuple already exists \n", __func__);
		ret = HWF_ERROR;
		goto done;
	}

	vserver = hwf_vserver_get_byname(ctl_vserver->name);
	if (vserver) {
		pr_err("\n%s: vserver %s already exists \n", __func__, ctl_vserver->name);
		ret = HWF_ERROR;
		goto done;
	}

	if (strcmp(ctl_vserver->ratelimiter_name,"")) {
		/* find ratelimiter by name */
		ratelimiter = bcm_hwf_ratelimiter_get_byname(ctl_vserver->ratelimiter_name);
		if (!ratelimiter) {
			pr_err("\n%s: ratelimiter %s does not exist\n", __func__, ctl_vserver->ratelimiter_name);
			ret = HWF_ERROR;
			goto done;
		}
	}

	vserver = hwf_vserver_alloc();

	if (vserver){

		/*addresses are stored in network order */
		memcpy(&vserver->daddr, &key_tuple.dst.u3, sizeof(vserver->daddr));
		vserver->l3num = key_tuple.src.l3num;
		vserver->dst_port = key_tuple.dst.u.all;
		vserver->l4_proto = key_tuple.dst.protonum;
		strcpy(vserver->name, ctl_vserver->name);

		if(ratelimiter) {
			/*increment refcnt of ratelimiter */
			ratelimiter->ref_count++;
			vserver->ratelimiter = ratelimiter;
		}

		 ret = hwf_tbl_update_vserver(true, &key_tuple, ratelimiter);

		if(ret) {
			pr_err("\n%s: HWF Vserver add failed ret=%d\n", __func__, ret);

			if(vserver->ratelimiter)
				vserver->ratelimiter->ref_count--;

			hwf_vserver_free(vserver);
		}
	} else {
			pr_err("\n%s: Vserver table full num_vservers=%d\n", __func__, bcm_hwf_g.num_vservers);
			ret = HWF_ERROR;
	}
done:
	BCM_HWF_CONFIG_UNLOCK();
	return ret;
}

int bcm_hwf_vserver_delete(const char *name)
{
	bcm_hwf_vserver_t *vserver;
	struct nf_conntrack_tuple key_tuple = {};
	int ret = HWF_SUCCESS;

	BCM_HWF_CONFIG_LOCK();

	vserver = hwf_vserver_get_byname(name);

	if (vserver) {
		/*addresses are stored in network order */
		memcpy(&key_tuple.dst.u3, &vserver->daddr, sizeof(vserver->daddr));
		key_tuple.src.l3num = vserver->l3num;
		key_tuple.dst.u.all = vserver->dst_port;
		key_tuple.dst.protonum = vserver->l4_proto;

		ret = hwf_tbl_update_vserver(false, &key_tuple, vserver->ratelimiter);

		if(ret)
			pr_err("\n%s: HWF Vserver delete failed ret=%d\n", __func__, ret);

		if(vserver->ratelimiter)
			vserver->ratelimiter->ref_count--;

		hwf_vserver_free(vserver);
	} else {
		pr_err("%s: vserver %s not found \n", __func__, name);
		ret = HWF_ERROR;
	}
	BCM_HWF_CONFIG_UNLOCK();
	return ret;
}

int bcm_hwf_vserver_update(hwfctl_vserver_t *ctl_vserver)
{
	int ret = HWF_ERROR;

	ret = bcm_hwf_vserver_delete(ctl_vserver->name);
	if (ret != HWF_SUCCESS) {
		pr_err("%s: vserver %s not found \n", __func__, ctl_vserver->name);
		goto done;
	}
	ret = bcm_hwf_vserver_add(ctl_vserver);
done:
	return ret;
}

void hwf_get_stats_from_hw(void)
{		
	uint64_t count = 0; 
	uint64_t rx_count = 0; 
	int ret, i;
	bcm_hwf_ratelimiter_t *ratelimiter = NULL;

	BCM_HWF_CONFIG_LOCK();

	if (!bcm_hwf_g.hwf_enable || time_after(bcm_hwf_g.stats_next_refresh, jiffies))
		goto skip;


	ret = hwf_hw_ct_miss_counter_get(&count);
	if(ret == 0)
		bcm_hwf_g.hw_ct_lookup_miss_cnt += count;

	/*retreive ddosq rx & tail drop */
	ret = hwf_hw_cpu_rxq_stat_get(&count, &rx_count);
	if(ret == 0) {
		bcm_hwf_g.hw_ct_miss_tail_drop_cnt = count;
		bcm_hwf_g.hw_ct_miss_rx_cnt = rx_count;
	}

	count = 0;
	/* get stats for hw ratelimiters */
	for (i=0; i < HWF_MAX_RATELIMITERS; i++) {
		ratelimiter = &bcm_hwf_g.ratelimiters[i];
		if ((ratelimiter->valid)) {
			hwf_hw_ratelimiter_get_stats(ratelimiter);
			count += ratelimiter->drop_cnt;
		}
	}
	bcm_hwf_g.hw_ct_miss_ratelimit_drop_cnt = count;

	bcm_hwf_g.stats_next_refresh = jiffies + bcm_hwf_g.stats_refresh_min_intvl;
skip:
	BCM_HWF_CONFIG_UNLOCK();
}


#ifdef CONFIG_NF_CONNTRACK_PROCFS
struct node_iter_state {
	struct hlist_head *hash;
	unsigned int htable_size;
	unsigned int bucket;
};

static struct hlist_node *node_get_first(struct seq_file *seq)
{
	struct node_iter_state *st = seq->private;
	struct hlist_node *n;

	for (st->bucket = 0;
		  st->bucket < st->htable_size;
		  st->bucket++) {
		n =	st->hash[st->bucket].first;
		if (n)
			return n;
	}
	return NULL;
}

static struct hlist_node *node_get_next(struct seq_file *seq,
						struct hlist_node *node)
{
	struct node_iter_state *st = seq->private;

	node = node->next;
	while (!(node)) {
		if (++st->bucket >= st->htable_size)
			return NULL;
		node =	st->hash[st->bucket].first;
	}
	return node;
}

static struct hlist_node *node_get_idx(struct seq_file *seq, loff_t pos)
{
	struct hlist_node *node = node_get_first(seq);

	if (node)
		while (pos && (node = node_get_next(seq, node)))
			pos--;

	return pos ? NULL : node;
}


static void *hwf_node_seq_start(struct seq_file *seq, loff_t *pos)
{
	struct node_iter_state *st = seq->private;

	BCM_HWF_CT_TABLE_LOCK();

	st->hash = bcm_hwf_g.ct_hashtbl;
	st->htable_size = HASH_SIZE(bcm_hwf_g.ct_hashtbl);
	return node_get_idx(seq, *pos);
}

static void *hwf_node_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	(*pos)++;
	return node_get_next(s, v);
}

static void hwf_node_seq_stop(struct seq_file *s, void *v)
{
	BCM_HWF_CT_TABLE_UNLOCK();
}

static const char *l3proto_name(u16 proto)
{
	switch (proto) {
	case AF_INET: return "ipv4";
	case AF_INET6: return "ipv6";
	}

	return "unknown";
}

static const char *l4proto_name(u16 proto)
{
	switch (proto) {
	case IPPROTO_ICMP: return "icmp";
	case IPPROTO_TCP: return "tcp";
	case IPPROTO_UDP: return "udp";
	case IPPROTO_DCCP: return "dccp";
	case IPPROTO_GRE: return "gre";
	case IPPROTO_SCTP: return "sctp";
	case IPPROTO_UDPLITE: return "udplite";
	case IPPROTO_ICMPV6: return "icmpv6";
	}
	return "unknown";
}

static int hwf_node_seq_show(struct seq_file *s, void *v)
{
	bcm_hwf_obj_t *hwf_obj = hlist_entry_safe(v, bcm_hwf_obj_t, hnode);
	const struct nf_conntrack_l4proto *l4proto;
	int ret = 0;

	WARN_ON(!hwf_obj);

	ret = -ENOSPC;

	l4proto = __nf_ct_l4proto_find(hwf_obj->tuple.src.l3num, hwf_obj->tuple.dst.protonum);
	WARN_ON(!l4proto);

	seq_printf(s, "%-8s %u %-8s %u ",
			l3proto_name(hwf_obj->tuple.src.l3num), hwf_obj->tuple.src.l3num,
			l4proto_name(l4proto->l4proto), hwf_obj->tuple.dst.protonum);


	print_tuple(s, &hwf_obj->tuple,
			 l4proto);

	if (seq_has_overflowed(s))
		goto release;


	seq_printf(s, "use=%u  hwid=%ld\n", hwf_obj->refcnt, hwf_obj->hwid);

	if (seq_has_overflowed(s))
		goto release;

	ret = 0;
release:
	return ret;
}

static int hwf_proc_stats_print(struct seq_file *s, void *v)
{
	hwf_get_stats_from_hw();
	seq_printf(s, "  HWF Lookup HW Miss : %llu  Drops : %llu\n",
			bcm_hwf_g.hw_ct_lookup_miss_cnt, bcm_hwf_g.hw_ct_miss_ratelimit_drop_cnt);
	seq_printf(s, "  DDOSQ Host RX : %llu  Tail Drops : %llu\n", bcm_hwf_g.hw_ct_miss_rx_cnt,
			bcm_hwf_g.hw_ct_miss_tail_drop_cnt);
	seq_printf(s, "  HWF Unsupported bypass : %llu \n", bcm_hwf_g.bypass_pkts);
	seq_printf(s, "  LAN Conn Limit :\n");
	seq_printf(s, "\t Total PKt drops : %llu  No Host Pkt drops : %llu \n",
			bcm_hosts_g.total_pkt_drops, bcm_hosts_g.nohost_pkt_drops);

	 seq_printf(s, "  HWF HW flow stats\n" );
	 seq_printf(s, "\tAdded : %u    Removed : %u\n", hwf_stat_g.cnt_success, 
			 hwf_stat_g.cnt_removed);
	 seq_printf(s, "\tOverflow : %u    Error : %u\n", hwf_stat_g.cnt_overflow, 
			 hwf_stat_g.cnt_error);
	 seq_printf(s, "\n");
	 return 0;
}

static struct proc_dir_entry *hwf_proc_top_dir;
static struct proc_dir_entry *hwf_proc_fwlist_file;
static struct proc_dir_entry *hwf_proc_vservers_file;
static struct proc_dir_entry *hwf_proc_ratelimiters_file;
static struct proc_dir_entry *hwf_proc_hosts_file;
static struct proc_dir_entry *hwf_proc_stats;
extern const struct seq_operations hwf_hosts_seq_ops;

static const struct seq_operations hwf_node_seq_ops = {
	.start = hwf_node_seq_start,
	.next  = hwf_node_seq_next,
	.stop  = hwf_node_seq_stop,
	.show  = hwf_node_seq_show
};

struct vserver_iter_state {
	unsigned int index;
};

static bcm_hwf_vserver_t *vserver_get_next(struct seq_file *seq, void *v)
{
	struct vserver_iter_state *st = seq->private;
	int i;
	bcm_hwf_vserver_t *vserver = NULL;

	for (i = st->index; i < HWF_MAX_VSERVERS; i++) {
		vserver = &bcm_hwf_g.vservers[i];
		if ((vserver->valid))
		{
			st->index = i+1;
			return vserver;
		}
	}
	st->index = i;
	return NULL;
}

static void *hwf_vserver_seq_start(struct seq_file *seq, loff_t *pos)
{
	struct vserver_iter_state *st = seq->private;
	loff_t p = *pos;

	BCM_HWF_CONFIG_LOCK();

	/*not sure why but, when cat is used, start,next,show stop qeuence is
	* repeating, to break this loop check if pos !=0 (second iteration) 
	* to break the loop 
    */
	if (p != 0)
		return NULL; /* needed to break loop for read() sycall */

	st->index = 0;
	return SEQ_START_TOKEN;
}

static void *hwf_vserver_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	(*pos)++; /* cuurently not used */
	return vserver_get_next(s, v);
}

static void hwf_vserver_seq_stop(struct seq_file *s, void *v)
{
	BCM_HWF_CONFIG_UNLOCK();
}

static inline void  hwf_vserver_seqprint(struct seq_file *s, bcm_hwf_vserver_t *vserver)
{
	char *ratelimiter_name = "None";
	int nw = HWF_MAX_KEY_NAME_LEN; /* name width */

	if (vserver->ratelimiter) {
		ratelimiter_name = vserver->ratelimiter->name;
	}

	if(vserver->l3num == AF_INET)
		seq_printf(s, "%-*s %pI4\t  %-8hu %-8u %s \n", nw, vserver->name, &vserver->daddr.ip, ntohs(vserver->dst_port),
			vserver->l4_proto, ratelimiter_name);
	 else
		seq_printf(s, "%-*s %pI6\t  %-8hu %-8u %s \n", nw, vserver->name, &vserver->daddr.all, ntohs(vserver->dst_port),
			vserver->l4_proto, ratelimiter_name);
}

static int hwf_vserver_seq_show(struct seq_file *s, void *v)
{
	int ret = 0;
	int nw = HWF_MAX_KEY_NAME_LEN; /* name width */

	if (v == SEQ_START_TOKEN) {
		seq_printf(s, "######## HWF Virtual Servers total=%d ########\n", bcm_hwf_g.num_vservers);
		seq_printf(s, "%-*s %-16s %-8s %-8s %s\n", nw, "Name", "DSTIP", "DSTPORT", "PROTO", "RATELIMITER");
	} else {
		hwf_vserver_seqprint(s, v);
	}

	if (seq_has_overflowed(s))
		ret = -ENOSPC;

	return ret;
}

static const struct seq_operations hwf_vserver_seq_ops = {
	.start = hwf_vserver_seq_start,
	.next  = hwf_vserver_seq_next,
	.stop  = hwf_vserver_seq_stop,
	.show  = hwf_vserver_seq_show
};

struct ratelimiters_iter_state {
	unsigned int index;
};

static bcm_hwf_ratelimiter_t *ratelimiters_get_next(struct seq_file *seq, void *v)
{
	struct ratelimiters_iter_state *st = seq->private;
	bcm_hwf_ratelimiter_t *ratelimiter = NULL;
	int i;

	for (i = st->index; i < HWF_MAX_VSERVERS; i++) {
		ratelimiter = &bcm_hwf_g.ratelimiters[i];
		if ((ratelimiter->valid))
		{
			st->index = i+1;
			return ratelimiter;
		}
	}
	st->index = i;
	return NULL;
}

static void *hwf_ratelimiters_seq_start(struct seq_file *seq, loff_t *pos)
{
	struct ratelimiters_iter_state *st = seq->private;
	loff_t p = *pos;

	BCM_HWF_CONFIG_LOCK();

	/*not sure why but, when cat is used, start,next,show stop qeuence is
	* repeating, to break this loop check if pos !=0 (second iteration) 
	* to break the loop 
    */
	if (p != 0)
		return NULL; /* needed to break loop for read() sycall */

	st->index = 0;
	return SEQ_START_TOKEN;
}

static void *hwf_ratelimiters_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	(*pos)++; /* cuurently not used */
	return ratelimiters_get_next(s, v);
}

static void hwf_ratelimiters_seq_stop(struct seq_file *s, void *v)
{
	BCM_HWF_CONFIG_UNLOCK();
}

static inline void  hwf_ratelimiters_seqprint(struct seq_file *s, bcm_hwf_ratelimiter_t *ratelimiter)
{
	seq_printf(s, "%-20s %-16s %-8u %-8u  %-10u %-10lld %-10llu %-10llu \n", ratelimiter->name,
			hwf_ratelimter_type_id_to_str(ratelimiter->type), ratelimiter->pps_rate,
			ratelimiter->pps_burst_size, ratelimiter->ref_count,
			ratelimiter->index, ratelimiter->drop_cnt, ratelimiter->hit_cnt);
}

static int hwf_ratelimiters_seq_show(struct seq_file *s, void *v)
{
	int ret = 0;

	if (v == SEQ_START_TOKEN) {
		seq_printf(s, "######## HWF Ratelimiters total=%d ########\n", bcm_hwf_g.num_ratelimiters);
		seq_printf(s, "%-20s %-16s %-8s %-8s %-10s %-10s %-10s %-10s\n", "Name",
			"Type", "Rate", "Burst", "Refcnt", "HW_Index", "PktDrops", "PktHits");
	} else {
		hwf_ratelimiters_seqprint(s, v);
	}

	if (seq_has_overflowed(s))
		ret = -ENOSPC;

	return ret;
}

static const struct seq_operations hwf_ratelimiters_seq_ops = {
	.start = hwf_ratelimiters_seq_start,
	.next  = hwf_ratelimiters_seq_next,
	.stop  = hwf_ratelimiters_seq_stop,
	.show  = hwf_ratelimiters_seq_show
};

static struct ctl_table_header *hwf_sysctl_header;
static struct ctl_table hwf_sysctl_table[] = {
	{
		.procname	= "debug",
		.data		= &bcm_hwf_debug_lvl_g,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{ }
};

#define HWF_PROCFS_TOP_DIR_PATH "hwf"

static int bcm_hwf_proc_exit(void)
{
	if (hwf_proc_fwlist_file)
		remove_proc_entry(HWF_PROCFS_TOP_DIR_PATH"/fwlist", NULL);

	if (hwf_proc_vservers_file)
		remove_proc_entry(HWF_PROCFS_TOP_DIR_PATH"/vservers", NULL);

	if (hwf_proc_ratelimiters_file)
		remove_proc_entry(HWF_PROCFS_TOP_DIR_PATH"/ratelimiters", NULL);

	if (hwf_proc_hosts_file)
		remove_proc_entry(HWF_PROCFS_TOP_DIR_PATH"/hosts", NULL);

	if (hwf_proc_stats)
		remove_proc_entry(HWF_PROCFS_TOP_DIR_PATH"/stats", NULL);

	if (hwf_sysctl_header)
		unregister_net_sysctl_table(hwf_sysctl_header);

	if (hwf_proc_top_dir)
		remove_proc_entry(HWF_PROCFS_TOP_DIR_PATH, NULL);

	return 0;
}

static int bcm_hwf_proc_init(void)
{

	hwf_proc_top_dir = proc_mkdir(HWF_PROCFS_TOP_DIR_PATH, NULL);

	if (!hwf_proc_top_dir) {
		pr_err("%s Unable to create %s proc directory ",
				__func__, HWF_PROCFS_TOP_DIR_PATH);
		goto error;
	}

	hwf_proc_fwlist_file = proc_create_seq_private("fwlist", 0440, hwf_proc_top_dir,
				&hwf_node_seq_ops, sizeof(struct node_iter_state), NULL);

	if (!hwf_proc_fwlist_file) {
		pr_err("%s Unable to create proc entry for fwlist", __func__);
		goto error;
	}

	hwf_proc_vservers_file = proc_create_seq_private("vservers", 0440, hwf_proc_top_dir,
				&hwf_vserver_seq_ops, sizeof(struct vserver_iter_state), NULL);

	if (!hwf_proc_vservers_file) {
		pr_err("%s Unable to create proc entry for vservers", __func__);
		goto error;
	}

	hwf_proc_ratelimiters_file = proc_create_seq_private("ratelimiters", 0440, hwf_proc_top_dir,
				&hwf_ratelimiters_seq_ops, sizeof(struct ratelimiters_iter_state), NULL);

	if (!hwf_proc_ratelimiters_file) {
		pr_err("%s Unable to create proc entry for ratelimiters", __func__);
		goto error;
	}

	hwf_proc_hosts_file = proc_create_seq_private("hosts", 0440, hwf_proc_top_dir,
				&hwf_hosts_seq_ops, sizeof(struct hosts_iter_state), NULL);

	if (!hwf_proc_hosts_file) {
		pr_err("%s Unable to create proc entry for hosts", __func__);
		goto error;
	}

	 hwf_proc_stats = proc_create_single(HWF_PROCFS_TOP_DIR_PATH"/stats",
		  S_IRUGO, NULL, hwf_proc_stats_print);

	if (!hwf_proc_stats) {
		pr_err("%s Unable to create proc entry for stats", __func__);
		goto error;
	}

	hwf_sysctl_header = register_sysctl(HWF_PROCFS_TOP_DIR_PATH, hwf_sysctl_table);
	if (!hwf_sysctl_header) {
		pr_err("%s Unable to create hwf_sysctl_table",	__func__);
		goto error;
	}

	return 0;

error:
	bcm_hwf_proc_exit();
	return -1;
}

#else
static int bcm_hwf_proc_init(void) {};
static int bcm_hwf_proc_exit(void) {};
#endif

static int hwf_event_thread_func(void *thread_data)
{
	 while (!kthread_should_stop()) {

		/* pull stats from hw */
		hwf_get_stats_from_hw();

		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(bcm_hwf_g.event_thread_intvl);
	}
	
	return 0;
}

int hwf_event_thread_create(void)
{
	 struct task_struct *tsk;

	bcm_hwf_g.event_thread_intvl = msecs_to_jiffies(1000); /*1sec */

	 tsk = kthread_create(hwf_event_thread_func, NULL, "hwf_event");
	 if (IS_ERR(tsk))
	 {
		pr_err("%s: failed\n", __func__);
		  return -1;
	 }

	bcm_hwf_g.event_thread = tsk;
	wake_up_process(bcm_hwf_g.event_thread);
	 printk("HWF Event Thread created successfully\n");
	 return 0;
}

void hwf_event_thread_destroy(void)
{
	 kthread_stop(bcm_hwf_g.event_thread);
}

static int __init bcm_hwf_init(void)
{
	int ret;

	get_random_once(&hwf_hash_rnd, sizeof(hwf_hash_rnd));
	bcm_hwf_g.hwf_enable = true;
	bcm_hwf_g.expect_lookup_enable = true;

	bcm_hwf_g.stats_refresh_min_intvl = msecs_to_jiffies(100); /*100 msec */
	bcm_hwf_g.stats_next_refresh = jiffies + bcm_hwf_g.stats_refresh_min_intvl;

	ret = hwf_event_thread_create();
	if (ret < 0)
		return ret;

	ret = hwf_cdev_init();
	if (ret < 0) {
		pr_err("%s: char dev initialization failed\n", __func__);
		return ret;
	}
	ret = bcm_ct_limit_init();
	if (ret < 0) {
		pr_err("%s: ct limit initialization failed\n", __func__);
		return ret;
	}

	ret = hwf_hw_init();
	if (ret < 0) {
		pr_err("%s: hwf hw initialization failed\n", __func__);
		return ret;
	}

	ret = bcm_hwf_proc_init();
	if (ret < 0) {
		pr_err("%s: proc initialization failed\n", __func__);
		hwf_hw_exit();
		return ret;
	}

	bcm_hwf_expect_lookup_enable(true);
	blog_hwf_fn = bcm_hwf_pkt_rcv;

	rcu_assign_pointer(bcm_hwf_ct_event_fn, bcm_hwf_ct_event);
	rcu_assign_pointer(bcm_hwf_expect_event_fn, bcm_hwf_expect_event);
	synchronize_rcu();

	return 0;
}



static void __exit bcm_hwf_exit(void)
{
	blog_hwf_fn = NULL;
	rcu_assign_pointer(bcm_hwf_ct_event_fn, NULL);
	rcu_assign_pointer(bcm_hwf_expect_event_fn, NULL);
	synchronize_rcu();

	/* flush sw table */
	bcm_hwf_flush_all(true);
	/*cleanup hw  */
	hwf_hw_exit();
	bcm_hwf_proc_exit();
	bcm_ct_limit_exit();
	hwf_cdev_deinit();
	hwf_event_thread_destroy();
}

module_init(bcm_hwf_init);
module_exit(bcm_hwf_exit);
MODULE_LICENSE("GPL");

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

/* for ct debug  functions */
#define DEBUG

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_expect.h>

#include <linux/bcm_nf_conntrack.h>
#include <linux/bcm_nf_expect.h>

#include "hwf.h"

#if IS_ENABLED(CONFIG_RNR_HW_FIREWALL)
#include "hwf_runner.h"
#else
static inline long  hwf_hw_node_add(bcm_hwf_obj_t *hwf_obj)
{ return HWF_INVALID_HW_IDX; };
static inline int hwf_hw_node_delete(bcm_hwf_obj_t *hwf_obj) {return 0; };
static int hwf_hw_config_enable(int enable) {return 0; };
static int __init hwf_hw_init(void) {return 0; };
static int __init hwf_hw_exit(void) {return 0; };
#endif


#define HWF_HASH_EXP 12
#define HWF_HASHTBL_SIZE (1<<HWF_HASH_EXP)

struct bcm_hwf {
	DECLARE_HASHTABLE(hashtbl, HWF_HASH_EXP); /* 4096 hash buckets */
	spinlock_t lock;
} bcm_hwf_g;

#define BCM_HWF_TABLE_LOCK()		spin_lock_bh(&bcm_hwf_g.lock)
#define BCM_HWF_TABLE_UNLOCK()		spin_unlock_bh(&bcm_hwf_g.lock)

static unsigned int hwf_hash_rnd __read_mostly;

static int bcm_hwf_enable_g __read_mostly = 1;
static int bcm_hwf_debug_g __read_mostly;


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
	hash_add(bcm_hwf_g.hashtbl, &hwf_obj->hnode, hashid);

	return 0;
}


/*caller should have a lock */
static inline bcm_hwf_obj_t *hwf_hash_lookup(const struct nf_conntrack_tuple *tuple)
{
	bcm_hwf_obj_t *hwf_obj;
	uint32_t hashid = hwf_tuple_to_hashid(tuple);

	hash_for_each_possible(bcm_hwf_g.hashtbl, hwf_obj, hnode, hashid) {
		if (hwf_tuple_is_equal(&hwf_obj->tuple, tuple))
			return hwf_obj;
	}
	return NULL;
}




static inline int hwf_node_add(const struct nf_conntrack_tuple *key_tuple, bool is_exp)
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

	hwf_hash_add(hwf_obj);

	hwf_obj->hwid = hwf_hw_node_add(hwf_obj);

	if (bcm_hwf_debug_g)
		printk("%s:tuple added hwid = %ld\n", __func__, hwf_obj->hwid);

	return 0;
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

	if (bcm_hwf_debug_g)
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
	hash_for_each_possible_safe(bcm_hwf_g.hashtbl, hwf_obj, tmpnode, hnode, hashid) {
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

static inline void bcm_hwf_flush_all(void)
{
	bcm_hwf_obj_t *hwf_obj;
	struct hlist_node *tmpnode;
	int bkt;

	BCM_HWF_TABLE_LOCK();

	hash_for_each_safe(bcm_hwf_g.hashtbl, bkt, tmpnode, hwf_obj, hnode) {
		hwf_node_force_delete(hwf_obj);
	}

	BCM_HWF_TABLE_UNLOCK();
}

static inline int hwf_tbl_update(bool add, const struct nf_conntrack_tuple *key_tuple, bool is_exp)
{
	bcm_hwf_obj_t *hwf_obj;

	BCM_HWF_TABLE_LOCK();
	hwf_obj = hwf_hash_lookup(key_tuple);

	if (add) {
		if (likely(!hwf_obj))
			hwf_node_add(key_tuple, is_exp);/* allocate node */
		else
			hwf_node_get(hwf_obj);
	} else {
		if (hwf_obj)
			hwf_node_put(hwf_obj);
		else {
			pr_warn("%s :tuple not found\n", __func__);
			nf_ct_dump_tuple(key_tuple);
		}
	}
	BCM_HWF_TABLE_UNLOCK();

	return 0;
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
	bool add = false;

	if (unlikely(!bcm_hwf_enable_g))
		return 0;

	if (test_bit(IPS_EXPECTED_BIT, &ct->status))
		key_tuple = nf_ct_tuple(ct, IP_CT_DIR_ORIGINAL);
	else
		key_tuple = nf_ct_tuple(ct, IP_CT_DIR_REPLY);

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

	if (bcm_hwf_debug_g) {
		printk("%s : add=%d  new=%d related=%d destroy=%d ", __func__,
				add, (events & (1 << IPCT_NEW)), (events & (1 << IPCT_RELATED)),
				(events & (1 << IPCT_DESTROY)));
		nf_ct_dump_tuple(key_tuple);
	}
	return hwf_tbl_update(add, key_tuple, false);
}

static int
bcm_hwf_expect_event(enum ip_conntrack_expect_events events, struct nf_conntrack_expect *exp)
{
	struct nf_conntrack_tuple key_tuple = {};
	bool add = false;

	if (unlikely(!bcm_hwf_enable_g))
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

	BCM_HWF_TABLE_LOCK();

	st->hash = bcm_hwf_g.hashtbl;
	st->htable_size = HASH_SIZE(bcm_hwf_g.hashtbl);
	return node_get_idx(seq, *pos);
}

static void *hwf_node_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	(*pos)++;
	return node_get_next(s, v);
}

static void hwf_node_seq_stop(struct seq_file *s, void *v)
{
	BCM_HWF_TABLE_UNLOCK();
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

static struct proc_dir_entry *hwf_proc_top_dir;
static struct proc_dir_entry *hwf_proc_fwlist_file;

static const struct seq_operations hwf_node_seq_ops = {
	.start = hwf_node_seq_start,
	.next  = hwf_node_seq_next,
	.stop  = hwf_node_seq_stop,
	.show  = hwf_node_seq_show
};


static int proc_hwf_enable(struct ctl_table *table, int write,
		void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int ret;
	//int old_val = bcm_hwf_enable_g;

	ret = proc_dointvec(table, write, buffer, lenp, ppos);
	if (!ret && write) {
		if (!bcm_hwf_enable_g) {
			hwf_hw_config_enable(0);
			bcm_hwf_flush_all();
		} else
			hwf_hw_config_enable(1);
	}
	return ret;
}

static struct ctl_table_header *hwf_sysctl_header;
static struct ctl_table hwf_sysctl_table[] = {
	{
		.procname	= "enable",
		.data		= &bcm_hwf_enable_g,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_hwf_enable,
	},
	{
		.procname	= "debug",
		.data		= &bcm_hwf_debug_g,
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

static int __init bcm_hwf_init(void)
{
	int ret;

	get_random_once(&hwf_hash_rnd, sizeof(hwf_hash_rnd));

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

	rcu_assign_pointer(bcm_hwf_ct_event_fn, bcm_hwf_ct_event);
	rcu_assign_pointer(bcm_hwf_expect_event_fn, bcm_hwf_expect_event);
	synchronize_rcu();

	return 0;
}

static void __exit bcm_hwf_exit(void)
{
	rcu_assign_pointer(bcm_hwf_ct_event_fn, NULL);
	rcu_assign_pointer(bcm_hwf_expect_event_fn, NULL);
	synchronize_rcu();

	/* flush sw table */
	bcm_hwf_flush_all();
	/*cleanup hw  */
	hwf_hw_exit();
	bcm_hwf_proc_exit();
}

module_init(bcm_hwf_init);
module_exit(bcm_hwf_exit);
MODULE_LICENSE("GPL");

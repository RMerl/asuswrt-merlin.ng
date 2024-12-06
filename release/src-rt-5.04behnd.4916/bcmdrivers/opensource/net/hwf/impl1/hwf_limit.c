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

/* Interface to configure ddos rate limit */

#include <linux/types.h>
#include <linux/module.h>
#include <linux/bcm_log.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/netfilter/nf_conntrack.h>
#include "hwf_ioctl.h"
#include "hwf.h"


#define BCM_HWF_HOSTS_MAX 512

#define BCM_HWF_HOST_DEFAULT_CT_MAX 1024
#define BCM_HWF_HOST_DEFAULT_CT_RATELIMIT 100
#define BCM_HWF_HOST_DEFAULT_CT_BURST 5
#define BCM_HWF_HOST_DEFAULT_NOHOST_RATE 5
#define BCM_HWF_HOST_DEFAULT_AFTER_MAX_CT_RATE 0

#define  BCM_HWF_HOST_DEFAULT_TIMEOUT  (10 * 60 * 1000) /*10 min */
#define  BCM_HWF_HOST_DEFAULT_PURGE_INTVL  (60 * 1000) /*1 min */
#define  BCM_HWF_HOST_REFRESH_INTVL (1000)  /* 1sec */

struct bcm_hosts bcm_hosts_g;

#define BCM_HWF_HOST_TABLE_LOCK()		spin_lock_bh(&bcm_hosts_g.lock)
#define BCM_HWF_HOST_TABLE_UNLOCK()		spin_unlock_bh(&bcm_hosts_g.lock)


static unsigned int host_hash_rnd __read_mostly;

static inline uint32_t host_ip_to_hashid(const union nf_inet_addr *l3addr, uint16_t l3proto)
{
	unsigned int n = 1;
	if(l3proto == AF_INET6)
		n = 4;

	return jhash2(l3addr->all, n, host_hash_rnd);
}

/* TODO may be add a local pool */
static inline bcm_host_obj_t *hwf_limit_host_obj_alloc(void)
{
	bcm_host_obj_t *host_obj;

	host_obj = kmalloc(sizeof(bcm_host_obj_t), GFP_ATOMIC);
	return host_obj;
}

static inline void hwf_limit_host_obj_free(bcm_host_obj_t *host_obj)
{
	kfree(host_obj);
}

static inline int hwf_limit_hash_add(bcm_host_obj_t *host_obj)
{
	uint32_t hashid = host_ip_to_hashid(&host_obj->l3addr,host_obj->l3proto);

	INIT_HLIST_NODE(&host_obj->hnode);
	/*add obj to hash */
	hash_add(bcm_hosts_g.hashtbl, &host_obj->hnode, hashid);
	return 0;
}

static inline void hwf_limit_hash_delete(bcm_host_obj_t *host_obj)
{
	hash_del(&host_obj->hnode);
}

static inline bool host_addr_is_equal(const union nf_inet_addr *a1,
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

static inline void host_addr_copy(union nf_inet_addr *dst,
				   const union nf_inet_addr *src, uint16_t l3proto)
{
	if(l3proto == AF_INET)
		dst->all[0] = src->all[0];
	else {
		dst->all[0] = src->all[0];
		dst->all[1] = src->all[1];
		dst->all[2] = src->all[2];
		dst->all[3] = src->all[3];
	}
}

static inline bcm_host_obj_t *hwf_limit_host_lookup(const union nf_inet_addr *l3addr, uint16_t l3proto)
{
	bcm_host_obj_t *host_obj;
	uint32_t hashid = host_ip_to_hashid(l3addr, l3proto);

	hash_for_each_possible(bcm_hosts_g.hashtbl, host_obj, hnode, hashid) {
		if ((l3proto == host_obj->l3proto) &&
				host_addr_is_equal(&host_obj->l3addr, l3addr, l3proto))
			return host_obj;
	}
	return NULL;
}

static inline void hwf_limit_host_obj_purge(bool force)
{
	bcm_host_obj_t *host_obj;
	struct hlist_node *tmpnode;
	int bkt;
	unsigned long curr_time = jiffies;

	hash_for_each_safe(bcm_hosts_g.hashtbl, bkt, tmpnode, host_obj, hnode) {
		if (force || 
				(!host_obj->is_static && (host_obj->total_count == 0) && time_after_eq(curr_time, host_obj->expires))) {
			hwf_limit_hash_delete(host_obj);
			hwf_limit_host_obj_free(host_obj);
			bcm_hosts_g.num_hosts--;
		}
	}
}


static inline bcm_host_obj_t *hwf_limit_host_obj_create(
		const union nf_inet_addr *l3addr, uint16_t l3proto)
{
	bcm_host_obj_t *host_obj;

	if(bcm_hosts_g.num_hosts >= bcm_hosts_g.max_hosts) {
		printk("%s: Failed, Table full num hosts=%u \n", __func__, bcm_hosts_g.num_hosts);
		return NULL;
	}
	/*TODO,may be add seperate limits for static/configured entries vs dynamic entries */
	host_obj = hwf_limit_host_obj_alloc();
	if (host_obj == NULL) { 
		printk("%s: Failed, host obj allocation  num hosts=%u \n", __func__, bcm_hosts_g.num_hosts);
		return NULL;
	}

    /* intialize host_obj */
	memset(host_obj, 0, sizeof(bcm_host_obj_t));

	host_obj->l3proto = l3proto;
	host_addr_copy(&host_obj->l3addr, l3addr, l3proto);

	host_obj->max_limit = bcm_hosts_g.default_ct_max;
	host_obj->idle_timeout = bcm_hosts_g.idle_timeout;
	host_obj->expires = jiffies + host_obj->idle_timeout;

	host_obj->rate.limit = bcm_hosts_g.default_ct_rate_limit;
	host_obj->rate.max_burst = bcm_hosts_g.default_ct_burst;
	host_obj->rate.after_max_limit = bcm_hosts_g.after_max_ct_rate;
	host_obj->rate.interval = bcm_hosts_g.ct_rate_interval;
	host_obj->rate.expires = jiffies + host_obj->rate.interval;

	bcm_hosts_g.num_hosts++;

	return host_obj;
}

int bcm_hwf_limit_host_obj_add(hwfctl_hosts_t *host)
{
	int ret = -EINVAL;
	bcm_host_obj_t *host_obj;
	uint16_t l3proto = host->is_ipv6 ? AF_INET6 : AF_INET;
	union nf_inet_addr *l3addr = (union nf_inet_addr *)host->ipaddr;

	/*convert to network order */
	hwf_swap_ipaddr(l3addr);

	BCM_HWF_HOST_TABLE_LOCK();

	host_obj = hwf_limit_host_lookup(l3addr, l3proto);

	if (host_obj) {
		/*TODO print address */
		if(host_obj->is_static) {
			pr_err("%s: host aready exist \n", __func__);
			goto done;
		} else 
			pr_info("%s: host is converted to staticn", __func__);

	} else {
		host_obj = hwf_limit_host_obj_create(l3addr, l3proto);
		if(host_obj == NULL)
			goto done;

		hwf_limit_hash_add(host_obj);
	}

	host_obj->is_static = true;

	/*update custom limits */
	if(host->ct_max)
		host_obj->max_limit = host->ct_max;
	if(host->ct_rate)
		host_obj->rate.limit = host->ct_rate;
	if(host->ct_burst)
		host_obj->rate.max_burst = host->ct_burst;

	ret = 0;
done:
	BCM_HWF_HOST_TABLE_UNLOCK();
	return ret;
}

int bcm_hwf_limit_host_obj_update(hwfctl_hosts_t *host)
{
	int ret = -EINVAL;
	bcm_host_obj_t *host_obj;
	uint16_t l3proto = host->is_ipv6 ? AF_INET6 : AF_INET;
	union nf_inet_addr *l3addr = (union nf_inet_addr *)host->ipaddr;

	/*convert to network order */
	hwf_swap_ipaddr(l3addr);

	BCM_HWF_HOST_TABLE_LOCK();

	host_obj = hwf_limit_host_lookup(l3addr, l3proto);

	if (!host_obj || !host_obj->is_static) {
		/*TODO print address */
			pr_err("%s: host does not exist \n", __func__);
			goto done;
	}

	/*update custom limits */
	if(host->ct_max)
		host_obj->max_limit = host->ct_max;
	if(host->ct_rate)
		host_obj->rate.limit = host->ct_rate;
	if(host->ct_burst)
		host_obj->rate.max_burst = host->ct_burst;

	ret = 0;
done:
	BCM_HWF_HOST_TABLE_UNLOCK();
	return ret;
}

int bcm_hwf_limit_host_obj_delete(hwfctl_hosts_t *host)
{
	int ret = -EINVAL;
	bcm_host_obj_t *host_obj;
	uint16_t l3proto = host->is_ipv6 ? AF_INET6 : AF_INET;
	union nf_inet_addr *l3addr = (union nf_inet_addr *)host->ipaddr;

	/*convert to network order */
	hwf_swap_ipaddr(l3addr);

	BCM_HWF_HOST_TABLE_LOCK();

	host_obj = hwf_limit_host_lookup(l3addr, l3proto);

	if (!host_obj) {
		if(l3proto == AF_INET)
			pr_err("%s: host %pI4 does not exist \n", __func__, &l3addr->ip);
		else
			pr_err("%s: host %pI6 does not exist \n", __func__, &l3addr->ip);

		goto done;
	}

	if (!host_obj->is_static) {
		if(l3proto == AF_INET)
			pr_err("%s: host %pI4 is not static, delete not allowed  \n", __func__, &l3addr->ip);
		else
			pr_err("%s: host %pI6 is not static, delete not allowed  \n", __func__, &l3addr->ip);

		goto done;
	}

	hwf_limit_hash_delete(host_obj);
	hwf_limit_host_obj_free(host_obj);
	bcm_hosts_g.num_hosts--;
	ret = 0;
done:
	BCM_HWF_HOST_TABLE_UNLOCK();
	return ret;
}

int bcm_hwf_limit_hosts_cfg_set(hwfctl_data_t *hwfctl)
{
	int ret = 0;

	if (hwfctl->info.hosts_default_ct_max)
		bcm_hosts_g.default_ct_max = hwfctl->info.hosts_default_ct_max;

	if (hwfctl->info.hosts_default_ct_rate)
		bcm_hosts_g.default_ct_rate_limit = hwfctl->info.hosts_default_ct_rate;

	if (hwfctl->info.hosts_default_ct_burst)
		bcm_hosts_g.default_ct_burst = hwfctl->info.hosts_default_ct_burst;

	
	if (hwfctl->info.valid.allow_dynamic_hosts)
		bcm_hosts_g.allow_dynamic_hosts = hwfctl->info.config.allow_dynamic_hosts;

	if (hwfctl->info.valid.nohost_pkt_rate)
		bcm_hosts_g.nohost_pkt_rate = hwfctl->info.nohost_pkt_rate;

	if (hwfctl->info.valid.after_max_ct_rate)
		bcm_hosts_g.after_max_ct_rate = hwfctl->info.after_max_ct_rate;

	return ret;
}

int bcm_hwf_limit_hosts_cfg_get(hwfctl_data_t *hwfctl)
{
	int ret = 0;

	hwfctl->info.hosts_default_ct_max =	bcm_hosts_g.default_ct_max;
	hwfctl->info.hosts_default_ct_rate = bcm_hosts_g.default_ct_rate_limit;
	hwfctl->info.hosts_default_ct_burst = bcm_hosts_g.default_ct_burst;
	hwfctl->info.config.allow_dynamic_hosts = bcm_hosts_g.allow_dynamic_hosts;
	hwfctl->info.nohost_pkt_rate = bcm_hosts_g.nohost_pkt_rate;
	hwfctl->info.after_max_ct_rate = bcm_hosts_g.after_max_ct_rate;

	return ret;
}

static struct hlist_node *hosts_get_first(struct seq_file *seq)
{
	struct hosts_iter_state *st = seq->private;
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

static struct hlist_node *hosts_get_next(struct seq_file *seq,
						struct hlist_node *node)
{
	struct hosts_iter_state *st = seq->private;

	node = node->next;
	while (!(node)) {
		if (++st->bucket >= st->htable_size)
			return NULL;
		node =	st->hash[st->bucket].first;
	}
	return node;
}

static void *hwf_hosts_seq_start(struct seq_file *seq, loff_t *pos)
{
	struct hosts_iter_state *st = seq->private;

	BCM_HWF_HOST_TABLE_LOCK();

	if (*pos != 0)
		return NULL; /* needed to break loop for read() sycall */

	st->hash = bcm_hosts_g.hashtbl;
	st->htable_size = HASH_SIZE(bcm_hosts_g.hashtbl);
	return SEQ_START_TOKEN;
	//return hosts_get_idx(seq, *pos);
}

static void *hwf_hosts_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	(*pos)++;
	if (v == SEQ_START_TOKEN) {
		return hosts_get_first(s);
	}
	return hosts_get_next(s, v);
}

static void hwf_hosts_seq_stop(struct seq_file *s, void *v)
{
	BCM_HWF_HOST_TABLE_UNLOCK();
}

void hwf_hosts_seq_printhdr(struct seq_file *s)
{
	seq_printf(s, "\t\t\t######## HWF Hosts ########\n");
	seq_printf(s, "Active LAN Hosts : %u Max Hosts : %u \n", bcm_hosts_g.num_hosts, bcm_hosts_g.max_hosts);
	//printk("Active total connections = %u \n", bcm_hosts_g.total_ct_count);
	seq_printf(s, "Total PKt drops : %llu  No Host Pkt drops : %llu \n",
			bcm_hosts_g.total_pkt_drops, bcm_hosts_g.nohost_pkt_drops);
	seq_printf(s, " Default Max connections per Host : %u\n", bcm_hosts_g.default_ct_max);
	seq_printf(s, " Default Connection Rate/Burst per Host : %u/%u \n",
			bcm_hosts_g.default_ct_rate_limit, bcm_hosts_g.default_ct_burst);
	seq_printf(s, " Connection rate after Max active connections : %u\n", bcm_hosts_g.after_max_ct_rate);
	seq_printf(s, " Dynamic Host creation %s \n",
			bcm_hosts_g.allow_dynamic_hosts ? "Enabled" : "Disabled");
	seq_printf(s, " No Host allowed Packet/sec : %u \n", bcm_hosts_g.nohost_pkt_rate);
	
	seq_printf(s, "IpAddr\t\t Static Max_Limit Ratelimit/Burst Total_Count/Max_Count Ratelimit_Drops Maxlimit_Drops\n");
}

static inline void hwf_limit_host_seqprint(struct seq_file *s, bcm_host_obj_t *host_obj)
{
	if(host_obj->l3proto == AF_INET)
		seq_printf(s, "%pI4\t %u\t %u\t\t %u/%u\t\t  %u/%u\t\t %u\t\t %u \n",&host_obj->l3addr.ip, host_obj->is_static, host_obj->max_limit,
		host_obj->rate.limit, host_obj->rate.max_burst, host_obj->total_count, host_obj->max_count,
		host_obj->rate_limit_drops, host_obj->max_limit_drops);
	else
		seq_printf(s, "%pI6\t %u\t %u\t\t %u/%u\t\t  %u/%u\t\t %u\t\t %u \n",&host_obj->l3addr.ip, host_obj->is_static, host_obj->max_limit,
		host_obj->rate.limit, host_obj->rate.max_burst, host_obj->total_count, host_obj->max_count,
		host_obj->rate_limit_drops, host_obj->max_limit_drops);
}

static int hwf_hosts_seq_show(struct seq_file *s, void *v)
{
	bcm_host_obj_t *host_obj;
	int ret = 0;

	if (v == SEQ_START_TOKEN) {
		hwf_hosts_seq_printhdr(s);
		return ret;
	}

	host_obj = hlist_entry_safe(v, bcm_host_obj_t, hnode);
	hwf_limit_host_seqprint(s, host_obj);

	if (seq_has_overflowed(s))
		ret = -ENOSPC;

	return ret;
}

const struct seq_operations hwf_hosts_seq_ops = {
	.start = hwf_hosts_seq_start,
	.next  = hwf_hosts_seq_next,
	.stop  = hwf_hosts_seq_stop,
	.show  = hwf_hosts_seq_show
};

static inline bool host_update_rate(bcm_host_obj_t *host_obj)
{
	unsigned long curr_time = jiffies;
	int diff;

	/* update rate */
	if (time_after_eq(curr_time, host_obj->rate.expires)) {
		/* interval expired */
		host_obj->rate.count = 0;
		host_obj->rate.after_max_count = 0;
		/* calculate burst, increase by 1 per second upto max 
		 * note:wrap around is ignored for simplicity 
		 */
		host_obj->rate.burst += ((curr_time - host_obj->rate.expires) / HZ);
		if(host_obj->rate.burst > host_obj->rate.max_burst)
			host_obj->rate.burst = host_obj->rate.max_burst;

		host_obj->rate.expires = curr_time + host_obj->rate.interval;
	}

	host_obj->rate.count++;

	diff = host_obj->rate.limit - host_obj->rate.count;
	if (diff  < 0 ) {
		if (host_obj->rate.burst == 0) {
			host_obj->rate_limit_drops++;
			return false;
		}
		host_obj->rate.burst--;
	}
	return true;
}

bool host_pkt_hwf_limit_check_limit(bcm_host_obj_t *host_obj)
{
	bool allow = host_update_rate(host_obj);

	if (allow && (host_obj->total_count > host_obj->max_limit)) {
		/* allow after_max_limit connections per sec after reaching max_limit */
		if (host_obj->rate.after_max_count < host_obj->rate.after_max_limit) {
			host_obj->rate.after_max_count++;
		} else {
			host_obj->max_limit_drops++;
			allow = false;
		}
		/*TODO trigger CT cleanup for specific host */
	}
	return allow;
}


static inline void host_obj_ct_update(bcm_host_obj_t *host_obj, bool add)
{
	unsigned long cur_time = jiffies;

	if (add) {
		host_obj->total_count++;
		bcm_hosts_g.total_ct_count++;
		if (host_obj->total_count > host_obj->max_count)
			host_obj->max_count = host_obj->total_count;
		
	} else {
		host_obj->total_count--;
		bcm_hosts_g.total_ct_count--;

		if (host_obj->total_count <= 0) {
			/* update idle timeout */
			host_obj->expires = cur_time + host_obj->idle_timeout;

			host_obj->total_count = 0; /*to handle intermediate /enable/disable */ 
		}
	}
}


int bcm_hwf_limit_ct_event_process(enum ip_conntrack_events events, 
		const struct nf_conn *ct, struct sk_buff *skb)
{
	const union nf_inet_addr *l3addr;
	uint16_t l3proto;
	bool is_rx_wan;
	const struct nf_conntrack_tuple *key_tuple;
	bcm_host_obj_t *host_obj;
	bool ct_add = false;
	int ret = 0;
	unsigned long curr_time = jiffies;

	if (skbuff_bcm_ext_indev_get(skb) &&
			is_netdev_wan(skbuff_bcm_ext_indev_get(skb)))
		is_rx_wan = true;

	if (test_bit(IPS_EXPECTED_BIT, &ct->status))
		key_tuple = nf_ct_tuple(ct, IP_CT_DIR_REPLY);
	else
		key_tuple = nf_ct_tuple(ct, IP_CT_DIR_ORIGINAL);

	if (events & (1 << IPCT_DESTROY)) 
		ct_add = false;
	else if (events & ((1 << IPCT_NEW) | (1 << IPCT_RELATED))) 
		ct_add = true;
	else
		return ret;

	/* check if host node is present otherwise add it */
	l3addr = &key_tuple->src.u3;
	l3proto = key_tuple->src.l3num;

	BCM_HWF_HOST_TABLE_LOCK();
	host_obj = hwf_limit_host_lookup(l3addr, l3proto);

	if ((ct_add) && (host_obj == NULL) && bcm_hosts_g.allow_dynamic_hosts) {
		/* create new host obj */
		host_obj = hwf_limit_host_obj_create(l3addr, l3proto);
		if (host_obj == NULL) {
			/* TODO handle failures, add overflow objects */
			ret = -1;
			goto unlock_done;
		}
		hwf_limit_hash_add(host_obj);
	}

	if (host_obj)
		host_obj_ct_update(host_obj, ct_add);

	/* event based periodic refresh every bcm_hosts_g->interval*/
	if (time_after_eq(curr_time, bcm_hosts_g.next_refresh))
		hwf_limit_host_obj_purge(false);

unlock_done:
	BCM_HWF_HOST_TABLE_UNLOCK();
	return ret;
}

bool bcm_hwf_limit_pkt_rcv(union nf_inet_addr *l3addr, uint16_t l3proto)
{
	bcm_host_obj_t *host_obj;

	/*on ct miss lookup host and check rate */
	host_obj = hwf_limit_host_lookup(l3addr, l3proto);
	/* TODO maintain l4proto stats */
	if(host_obj && host_pkt_hwf_limit_check_limit(host_obj))
		return true;

	 if (!host_obj) {
		 if (time_after_eq(jiffies, bcm_hosts_g.nohost_rate_refresh)) {
			 bcm_hosts_g.nohost_rate_refresh = jiffies + msecs_to_jiffies(BCM_HWF_HOST_REFRESH_INTVL);
			 bcm_hosts_g.nohost_pkt_count = 0;
		 }

		 if (bcm_hosts_g.nohost_pkt_count < bcm_hosts_g.nohost_pkt_rate) {
			 bcm_hosts_g.nohost_pkt_count++;
			 return true;
		 }
	 }

	bcm_hosts_g.total_pkt_drops++;
	return false;
}

int __init bcm_ct_limit_init(void)
{
	int ret = 0;

	get_random_once(&host_hash_rnd, sizeof(host_hash_rnd));
	spin_lock_init(&bcm_hosts_g.lock);
	
	bcm_hosts_g.max_hosts = BCM_HWF_HOSTS_MAX;
	bcm_hosts_g.default_ct_max = BCM_HWF_HOST_DEFAULT_CT_MAX;
	bcm_hosts_g.default_ct_rate_limit = BCM_HWF_HOST_DEFAULT_CT_RATELIMIT;
	bcm_hosts_g.default_ct_burst = BCM_HWF_HOST_DEFAULT_CT_BURST;
	bcm_hosts_g.nohost_pkt_rate = BCM_HWF_HOST_DEFAULT_NOHOST_RATE;
	bcm_hosts_g.after_max_ct_rate = BCM_HWF_HOST_DEFAULT_AFTER_MAX_CT_RATE;

	bcm_hosts_g.allow_dynamic_hosts = true; 
	bcm_hosts_g.ct_rate_interval = msecs_to_jiffies(BCM_HWF_HOST_REFRESH_INTVL);
	bcm_hosts_g.interval = msecs_to_jiffies(BCM_HWF_HOST_DEFAULT_PURGE_INTVL);
	bcm_hosts_g.idle_timeout = msecs_to_jiffies(BCM_HWF_HOST_DEFAULT_TIMEOUT);
	bcm_hosts_g.next_refresh = jiffies + bcm_hosts_g.interval;
	bcm_hosts_g.nohost_rate_refresh = jiffies + msecs_to_jiffies(BCM_HWF_HOST_REFRESH_INTVL);
	
	return ret;
}

void __exit bcm_ct_limit_exit(void)
{
	BCM_HWF_HOST_TABLE_LOCK();
	hwf_limit_host_obj_purge(true);
	BCM_HWF_HOST_TABLE_UNLOCK();
	return;
}
/* 
 * TODO
 * limit dynamic entires, and limit subnets upto 4 ipv4 and ipv6 
 * pass info from runner if ct lookup is done 
 * lookup for frags 
 */


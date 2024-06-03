/*
 * <:copyright-BRCM:2020:DUAL/GPL:standard 
 * 
 *    Copyright (c) 2020 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :>
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/hashtable.h>
#include <linux/etherdevice.h>
#include <net/ipv6.h>
#include <linux/ndi.h>
#include "ndi_local.h"
#include <linux/inetdevice.h>

#define DEV_TABLE_BITS		5
#define IP_TABLE_BITS		3
#define SA_TABLE_BITS		2

/* --- variables --- */
#define DEFINE_NDI_TABLE(name, bits) \
	struct name##_table { \
		DECLARE_HASHTABLE(table, bits); \
		struct kmem_cache *cache; \
		unsigned long size; \
	}; \
	static struct name##_table name
DEFINE_NDI_TABLE(devices,	DEV_TABLE_BITS);
DEFINE_NDI_TABLE(ips,		IP_TABLE_BITS);
DEFINE_NDI_TABLE(sas,		SA_TABLE_BITS);
static u32 dev_id;

/* --- functions --- */
static u32 dev_key(u8 *mac)
{
	return *((u32 *)&mac[0]) ^ *((u16 *)&mac[4]);
}

struct ndi_dev *dev_find_by_mac(u8 *mac)
{
	struct ndi_dev *dev;
	u32 key = dev_key(mac);

	hash_for_each_possible(devices.table, dev, node, key)
		if (ether_addr_equal(dev->mac, mac))
			break;

	return dev;
}

struct ndi_dev *dev_find_by_id(u32 id)
{
	struct ndi_dev *dev;
	int bkt;

	hash_for_each(devices.table, bkt, dev, node)
		if (dev->id == id)
			break;

	return dev;
}

static struct ndi_dev *__dev_alloc_init(u8 *mac)
{
	struct ndi_dev *dev;

	dev = kmem_cache_zalloc(devices.cache, GFP_ATOMIC);
	if (!dev) {
		pr_err("could not allocate device entry\n");
		goto out;
	}

	if (mac)
		memcpy(dev->mac, mac, sizeof(dev->mac));
	INIT_HLIST_NODE(&dev->node);
	dev->id = dev_id++;
	dev->state = DEV_OFFLINE;

out:
	return dev;
}

struct ndi_dev *__dev_find_or_new(u8 *mac, struct sk_buff *skb, int ignore)
{
	struct ndi_dev *dev;
	u32 key = dev_key(mac);

	if (is_zero_ether_addr(mac))
		return NULL;

	dev = dev_find_by_mac(mac);
	if (dev)
		goto out;

	dev = __dev_alloc_init(mac);
	if (!dev)
		goto out;

	if (ignore)
		set_bit(NDI_DEV_IGNORE_BIT, &dev->flags);
	dev_ip_update(dev, skb);

	hash_add(devices.table, &dev->node, key);
	pr_debug("%s new device\n", ndi_dev_name(dev));
	ndi_dev_nl_event(dev, NDINL_NEWDEVICE);

out:
	return dev;
}

struct ndi_dev *dev_find_or_new(u8 *mac, struct sk_buff *skb)
{
	return __dev_find_or_new(mac, skb, 0);
}

struct ndi_dev *dev_find_or_new_ignored(u8 *mac, struct sk_buff *skb)
{
	return __dev_find_or_new(mac, skb, 1);
}

struct ndi_dev
*dev_find_or_new_for_secpath(struct sec_path *sp, struct ndi_dev *ct_dev)
{
	struct ndi_dev *dev = NULL;
	struct ndi_sa *sa;

	sa = sa_find_first(sp);
	if (!sa)
		goto out;

	/* if the device in conntrack matches the SA map, we are done */
	dev = sa->dev;
	if (dev && (!ct_dev || dev == ct_dev))
		goto out;

	/* When we get a new conntrack, the first SA should create a new
	 * secpath ndi device and map to it, and the other direction flow
	 * should then already reference the same device through the conntrack.
	 * In the rare case that we have no conntrack and allocate two devices
	 * (one for each direction), handle deletion of one of the devices and
	 * remap the SA to the single device.
	 */
	dev_sa_put(sa->dev);

	dev = ct_dev;
	if (!dev) {
		dev = __dev_alloc_init(NULL);
		if (!dev)
			goto out;

		set_bit(NDI_DEV_IS_XFRM_BIT, &dev->flags);

		hash_add(devices.table, &dev->node, 0);
		pr_debug("%s new XFRM device\n", ndi_dev_name(dev));
		ndi_dev_nl_event(dev, NDINL_NEWDEVICE);
	}

	sa->dev = dev;
	dev_sa_get(dev);

out:
	return dev;
}

int dev_ip_update(struct ndi_dev *dev, struct sk_buff *skb)
{
	int updated = 0;

	if (!skb)
		return 0;

	if (skb->protocol == htons(ETH_P_IP)) {
		struct in_addr *addr	= ndi_ipv4_get_addr(skb);

		if (ipv4_ignore(addr->s_addr) ||
		    ipv4_addr_equal(addr, &dev->ip4))
			goto out;

		if (!ipv4_ignore(dev->ip4.s_addr))
			pr_debug("[%pM] IPv4 updated from %pI4 to %pI4\n",
				 dev->mac, &dev->ip4, addr);
		dev->ip4	= *addr;
		updated		= 1;
	} else if (skb->protocol == htons(ETH_P_IPV6)) {
		struct in6_addr *addr	= ndi_ipv6_get_addr(skb);

		if (ipv6_ignore(addr) || ipv6_addr_equal(addr, &dev->ip6))
			goto out;

		if (!ipv6_ignore(&dev->ip6))
			pr_debug("[%pM] IPv6 updated from %pI6c to %pI6c\n",
				 dev->mac, &dev->ip6, addr);
		dev->ip6	= *addr;
		updated		= 1;
	}

out:
	return updated;
}

int devs_bucket_count(void)
{
	return HASH_SIZE(devices.table);
}
struct hlist_head *devs_get_bucket(int i)
{
	return &devices.table[i];
}

static void dev_delete_helper(struct ndi_dev *dev)
{
	if (dev->netdev)
		dev_put(dev->netdev);
}

void dev_sa_get(struct ndi_dev *dev)
{
	if (!dev)
		return;

	if (atomic_add_return(1, &dev->sa_count) == 1) {
		clear_bit(NDI_DEV_STALE_BIT, &dev->flags);
		pr_debug("%s is online over ipsec\n", ndi_dev_name(dev));
		dev->state = DEV_ONLINE;
		ndi_dev_nl_event_locked(dev, NDINL_NEWDEVICE);
	}
}

void dev_sa_put(struct ndi_dev *dev)
{
	if (!dev)
		return;

	if (atomic_dec_and_test(&dev->sa_count)) {
		set_bit(NDI_DEV_STALE_BIT, &dev->flags);
		pr_debug("%s is offline\n", ndi_dev_name(dev));
		dev->state = DEV_OFFLINE;
		ndi_dev_nl_event_locked(dev, NDINL_NEWDEVICE);
	}
}

static inline u32 ip_hash(void *_ip, int l3proto)
{
	u32 val = 0;
	u32 *ip = _ip;

	if (l3proto == NFPROTO_IPV4)
		val = hash_min(*ip, IP_TABLE_BITS);
	if (l3proto == NFPROTO_IPV6) {
		val = hash_32(ip[0], 32) + hash_32(ip[1], 32) +
		      hash_32(ip[2], 32) + hash_32(ip[3], 32);
		val = hash_min(val, IP_TABLE_BITS);
	}

	return val;
}

struct ndi_ip *ip_find(void *ip, int l3proto)
{
	struct ndi_ip *entry;
	u32 key;

	key = ip_hash(ip, l3proto);
	hash_for_each_possible(ips.table, entry, node, key) {
		if (entry->l3proto != l3proto)
			continue;
		if (l3proto == NFPROTO_IPV4 &&
		    ipv4_addr_equal(&entry->ip4, ip))
			break;
		if (l3proto == NFPROTO_IPV6 &&
		    ipv6_addr_equal(&entry->ip6, ip))
			break;
	}

	return entry;
}

struct ndi_ip *ip_find_by_skb(struct sk_buff *skb)
{
	if (skb->protocol == htons(ETH_P_IP))
		return ip_find(ndi_ipv4_get_addr(skb), NFPROTO_IPV4);
	else if (skb->protocol == htons(ETH_P_IPV6))
		return ip_find(ndi_ipv6_get_addr(skb), NFPROTO_IPV6);

	return NULL;
}

void ip_free(struct ndi_ip *entry)
{
	hash_del(&entry->node);
	kmem_cache_free(ips.cache, entry);
	ips.size--;
}

struct ndi_ip *ip_find_or_new(void *ip, int l3proto, u8 *mac)
{
	struct hlist_head *h;
	struct ndi_ip *entry;
	struct ndi_ip *new_entry = NULL;
	struct hlist_node *n;
	int bkt;

	entry = ip_find(ip, l3proto);
	if (entry)
		goto check_duplicate_macs;

	if (is_zero_ether_addr(mac))
		return NULL;

	entry = kmem_cache_zalloc(ips.cache, GFP_ATOMIC);
	if (!entry) {
		pr_err("enable to allocate entry\n");
		goto out;
	}
	entry->l3proto = l3proto;
	if (l3proto == NFPROTO_IPV4) {
		memcpy(&entry->ip4, ip, sizeof(entry->ip4));
		pr_debug("[%pM] new ip %pI4\n", mac, ip);
	} else if (l3proto == NFPROTO_IPV6) {
		memcpy(&entry->ip6, ip, sizeof(entry->ip6));
		pr_debug("[%pM] new ip %pI6c\n", mac, ip);
	}

	/* add entry to hlist */
	INIT_HLIST_NODE(&entry->node);
	h = &ips.table[ip_hash((u32*)ip, l3proto)];
	hlist_add_head(&entry->node, h);
	ips.size++;

check_duplicate_macs:
	memcpy(entry->mac, mac, sizeof(entry->mac));
	new_entry = entry;

	/* Remove other entries that match the same mac as the new entry. This
	 * covers cases where a LAN client changes IP addresses. */
	hash_for_each_safe(ips.table, bkt, n, entry, node) {
		if (entry == new_entry)
			continue;
		if (ether_addr_equal(entry->mac, mac) &&
		    entry->l3proto == l3proto)
			ip_free(entry);
	}

out:
	return entry;
}

static void ip_delete_helper(struct ndi_ip *ip) { }

static u32 sa_key(struct xfrm_state *x)
{
	return (unsigned long)x & 0xFFFFFFFF;
}

struct ndi_sa *sa_find(struct xfrm_state *x)
{
	struct ndi_sa *sa;

	hash_for_each_possible(sas.table, sa, node, sa_key(x))
		if (sa->x == x)
			return sa;

	return NULL;
}

struct ndi_sa *sa_find_first(struct sec_path *sp)
{
	struct ndi_sa *sa = NULL;
	int i;

	for (i = 0; i < sp->len; i++) {
		sa = sa_find(sp->xvec[i]);
		if (!sa)
			continue;
	}

	return sa;
}

struct ndi_sa *sa_find_or_new(struct xfrm_state *x)
{
	struct ndi_sa *sa;

	if (!x)
		return NULL;

	sa = sa_find(x);
	if (sa)
		goto out;

	sa = kmem_cache_zalloc(sas.cache, GFP_ATOMIC);
	if (!sa) {
		pr_err("could not allocate SA entry\n");
		goto out;
	}

#if IS_ENABLED(CONFIG_XFRM)
	xfrm_state_hold(x);
#endif
	sa->x = x;

	INIT_HLIST_NODE(&sa->node);
	hash_add(sas.table, &sa->node, sa_key(x));

out:
	return sa;
}

static void sa_delete_helper(struct ndi_sa *sa)
{
	dev_sa_put(sa->dev);
	sa->dev = NULL;

#if IS_ENABLED(CONFIG_XFRM)
	xfrm_state_put(sa->x);
#endif
	sa->x = NULL;
}

void sa_delete(struct ndi_sa *sa)
{
	if (!sa)
		return;

	sa_delete_helper(sa);

	hash_del(&sa->node);
	kmem_cache_free(sas.cache, sa);
}

/* --- proc-related functions --- */
#define DEFINE_NDI_HLIST_SEQ(name, _table, entry_type, fmt, show_fun) \
	static void *ndi_##name##_seq_start(struct seq_file *s, loff_t *pos) \
	{ \
		loff_t *spos = s->private; \
		*spos = *pos - 1; \
		if (*pos == 0) \
			return SEQ_START_TOKEN; \
		if (*spos >= HASH_SIZE(_table.table)) \
			return NULL; \
		return spos; \
	} \
	static void *ndi_##name##_seq_next(struct seq_file *s, void *v, \
					    loff_t *pos) \
	{ \
		loff_t *spos = s->private; \
		(*pos)++; \
		(*spos)++; \
		if (*spos >= HASH_SIZE(_table.table)) \
			return NULL; \
		return spos; \
	} \
	static void ndi_##name##_seq_stop(struct seq_file *s, void *v) \
	{ \
	} \
	static int ndi_##name##_seq_show(struct seq_file *s, void *v) \
	{ \
		loff_t *spos = s->private; \
		struct hlist_head *h = &_table.table[*spos]; \
		entry_type *entry; \
		\
		if (v == SEQ_START_TOKEN) { \
			seq_printf(s, fmt "\n"); \
			return 0; \
		} \
		\
		/* print each entry in the bucket */ \
		hlist_for_each_entry(entry, h, node) \
			if (!show_fun(s, entry)) \
				seq_printf(s, "\n"); \
		return 0; \
	} \
	static const struct seq_operations ndi_##name##_seq_ops = { \
		.start	= ndi_##name##_seq_start, \
		.next	= ndi_##name##_seq_next, \
		.stop	= ndi_##name##_seq_stop, \
		.show	= ndi_##name##_seq_show \
	}; \
	static int ndi_##name##_open(struct inode *inode, struct file *file) \
	{ \
		return seq_open_private(file, &ndi_##name##_seq_ops, \
					sizeof(loff_t)); \
	} \
	static const struct file_operations ndi_##name##_fops = { \
		.open		= ndi_##name##_open, \
		.read		= seq_read, \
		.llseek		= seq_lseek, \
		.release	= seq_release_private, \
	}

static int __ndi_dev_entry_show(struct seq_file *s, struct ndi_dev *entry)
{
	seq_printf(s, "%u %pM %pI4 %pI6c %u %u %u %u %u %u %u %u \"%s\"",
		entry->id,
		entry->mac,
		&entry->ip4,
		&entry->ip6,
		entry->state,
#if IS_ENABLED(CONFIG_BCM_DPI)
		entry->dpi.vendor,
		entry->dpi.os,
		entry->dpi.os_class,
		entry->dpi.category,
		entry->dpi.family,
		entry->dpi.dev_id,
		entry->dpi.prio,
#else
		0, 0, 0, 0, 0, 0, 0,
#endif
		strlen(entry->hostname) ? entry->hostname : NULL);

	return 0;
}

static int ndi_dev_entry_show(struct seq_file *s, struct ndi_dev *entry)
{
	if (should_ignore_ndi_dev(entry))
		return 1;

	return __ndi_dev_entry_show(s, entry);
}
DEFINE_NDI_HLIST_SEQ(dev, devices, struct ndi_dev, "id mac ip4 ip6 state vendor os "
			"os_class category family dev_id prio \"hostname\"",
			ndi_dev_entry_show);

static int
ndi_ignored_dev_entry_show(struct seq_file *s, struct ndi_dev *entry)
{
	if (!test_bit(NDI_DEV_IGNORE_BIT, &entry->flags))
		return 1;
	if (test_bit(NDI_DEV_STALE_BIT, &entry->flags))
		return 1;

	return __ndi_dev_entry_show(s, entry);
}
DEFINE_NDI_HLIST_SEQ(ignored_dev, devices, struct ndi_dev,
		     "id mac ip4 ip6 state vendor os os_class category family dev_id prio \"hostname\"",
		     ndi_ignored_dev_entry_show);

static int ndi_ip_entry_show(struct seq_file *s, struct ndi_ip *entry)
{
	seq_printf(s, "%pI4 %pI6c %pM\n",
		   &entry->ip4,
		   &entry->ip6,
		   entry->mac);
	return 0;
}
DEFINE_NDI_HLIST_SEQ(ips, ips, struct ndi_ip, "ip mac", ndi_ip_entry_show);

static int ndi_sa_entry_show(struct seq_file *s, struct ndi_sa *entry)
{
	seq_printf(s, "%pX %d", entry->x, entry->dev ? entry->dev->id : -1);
	return 0;
}
DEFINE_NDI_HLIST_SEQ(sas, sas, struct ndi_sa, "xfrm ndi_dev_id",
		     ndi_sa_entry_show);

#define init_table(name, struct_name) \
	({ \
		int __ret = 0; \
		hash_init(name.table); \
		name.cache = KMEM_CACHE(struct_name, SLAB_HWCACHE_ALIGN); \
		if (!name.cache) { \
			pr_err("couldn't allocate "#name"\n"); \
			__ret = -ENOMEM; \
		} \
		__ret; \
	})

#define init_proc_entry(name, permissions, parent, fops) \
	({ \
		struct proc_dir_entry *__pde; \
		int __ret = 0; \
		__pde = proc_create(name, permissions, parent, &fops); \
		if (!__pde) { \
			pr_err("couldn't create proc entry '"#name"'\n"); \
			__ret = -1; \
		} \
		__ret; \
	})

int __init ndi_tables_init(void)
{
	int ret;

	ret = init_table(devices, ndi_dev);
	if (ret)
		goto err;
	ret = init_table(ips, ndi_ip);
	if (ret)
		goto err_free_devices_table;
	ret = init_table(sas, ndi_sa);
	if (ret)
		goto err_free_ips_table;

	/* create proc entries */
	if (init_proc_entry("devices", 0440, ndi_dir, ndi_dev_fops))
		goto err_free_sas_table;
	if (init_proc_entry("ignored_devices", 0440, ndi_dir,
			    ndi_ignored_dev_fops))
		goto err_free_devices;
	if (init_proc_entry("ips", 0440, ndi_dir, ndi_ips_fops))
		goto err_free_ignored_devices;
	if (init_proc_entry("sas", 0440, ndi_dir, ndi_sas_fops))
		goto err_free_ips;

	return 0;

err_free_ips:
	remove_proc_entry("ips", ndi_dir);
err_free_ignored_devices:
	remove_proc_entry("ignored_devices", ndi_dir);
err_free_devices:
	remove_proc_entry("devices", ndi_dir);
err_free_sas_table:
	kmem_cache_destroy(sas.cache);
err_free_ips_table:
	kmem_cache_destroy(ips.cache);
err_free_devices_table:
	kmem_cache_destroy(devices.cache);
err:
	return ret;
}

#define cleanup_table(name, type, cb) \
	do { \
		struct hlist_node *__tmp; \
		type *__item; \
		int __i; \
		\
		hash_for_each_safe(name.table, __i, __tmp, __item, node) { \
			cb(__item); \
			kmem_cache_free(name.cache, __item); \
		} \
		kmem_cache_destroy(name.cache); \
	} while (0)

void __exit ndi_tables_exit(void)
{
	remove_proc_entry("devices", ndi_dir);
	remove_proc_entry("ignored_devices", ndi_dir);
	remove_proc_entry("ips", ndi_dir);
	remove_proc_entry("sas", ndi_dir);

	spin_lock_bh(&lock);
	cleanup_table(sas, struct ndi_sa, sa_delete_helper);
	cleanup_table(ips, struct ndi_ip, ip_delete_helper);
	cleanup_table(devices, struct ndi_dev, dev_delete_helper);
	spin_unlock_bh(&lock);
}

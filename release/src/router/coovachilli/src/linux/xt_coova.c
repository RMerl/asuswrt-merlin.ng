/* -*- mode: c; c-basic-offset: 8; -*-
 *
 * Copyright (c) 2010-2012 David Bird (Coova Technologies)
 *
 * Inspired by the "recent" module which carried these notices:
 *
 * Copyright (c) 2006 Patrick McHardy <kaber@trash.net>
 * Copyright © CC Computer Consultants GmbH, 2007 - 2008
 *
 * Author: Stephen Frost <sfrost@snowman.net>
 * Copyright 2002-2003, Stephen Frost, 2.5.x port by laforge@netfilter.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/list.h>
#include <linux/random.h>
#include <linux/jhash.h>
#include <linux/bitops.h>
#include <linux/skbuff.h>
#include <linux/inet.h>
#include <net/net_namespace.h>

#include <linux/netfilter/x_tables.h>
#include "xt_coova.h"

MODULE_AUTHOR("David Bird <david@coova.com>");
MODULE_DESCRIPTION("Xtables: \"coova\" module for use with CoovaChilli");
MODULE_LICENSE("GPL");
MODULE_ALIAS("ipt_coova");
MODULE_ALIAS("ip6t_coova");

static unsigned int ip_list_tot = 100;
static unsigned int ip_pkt_list_tot = 20;
static unsigned int ip_list_hash_size = 0;
static unsigned int ip_list_perms = 0644;
static unsigned int ip_list_uid = 0;
static unsigned int ip_list_gid = 0;
module_param(ip_list_tot, uint, 0400);
module_param(ip_pkt_list_tot, uint, 0400);
module_param(ip_list_hash_size, uint, 0400);
module_param(ip_list_perms, uint, 0400);
module_param(ip_list_uid, uint, 0400);
module_param(ip_list_gid, uint, 0400);
MODULE_PARM_DESC(ip_list_tot, "number of IPs to remember per list");
MODULE_PARM_DESC(ip_pkt_list_tot, "number of packets per IP to remember (max. 255)");
MODULE_PARM_DESC(ip_list_hash_size, "size of hash table used to look up IPs");
MODULE_PARM_DESC(ip_list_perms, "permissions on /proc/net/coova/* files");
MODULE_PARM_DESC(ip_list_uid,"owner of /proc/net/coova/* files");
MODULE_PARM_DESC(ip_list_gid,"owning group of /proc/net/coova/* files");

struct coova_entry {
	struct list_head	list;
	struct list_head	lru_list;
	union nf_inet_addr	addr;
	unsigned char           hwaddr[ETH_ALEN];
	u_int16_t		family;
	u_int8_t		index;

	u_int8_t                state;
	u_int64_t		bytes_in;
	u_int64_t		bytes_out;
	u_int64_t		pkts_in;
	u_int64_t		pkts_out;
};

struct coova_table {
	struct list_head	list;
	char			name[XT_COOVA_NAME_LEN];
	unsigned int		refcnt;
	unsigned int		entries;
	struct list_head	lru_list;
	struct list_head	iphash[0];
};

static LIST_HEAD(tables);
static DEFINE_SPINLOCK(coova_lock);
static DEFINE_MUTEX(coova_mutex);

#ifdef CONFIG_PROC_FS
static struct proc_dir_entry *coova_proc_dir;
static const struct file_operations coova_old_fops, coova_mt_fops;
#endif

static u_int32_t hash_rnd;
static bool hash_rnd_initted;

static unsigned int coova_entry_hash4(const union nf_inet_addr *addr)
{
	if (!hash_rnd_initted) {
		get_random_bytes(&hash_rnd, sizeof(hash_rnd));
		hash_rnd_initted = true;
	}
	return jhash_1word((__force u32)addr->ip, hash_rnd) &
	       (ip_list_hash_size - 1);
}

static unsigned int coova_entry_hash6(const union nf_inet_addr *addr)
{
	if (!hash_rnd_initted) {
		get_random_bytes(&hash_rnd, sizeof(hash_rnd));
		hash_rnd_initted = true;
	}
	return jhash2((u32 *)addr->ip6, ARRAY_SIZE(addr->ip6), hash_rnd) &
	       (ip_list_hash_size - 1);
}

static struct coova_entry *
coova_entry_lookup(const struct coova_table *table,
		   const union nf_inet_addr *addrp, u_int16_t family)
{
	struct coova_entry *e;
	unsigned int h;

	if (family == AF_INET6)
		h = coova_entry_hash6(addrp);
	else
		h = coova_entry_hash4(addrp);

	list_for_each_entry(e, &table->iphash[h], list)
		if (e->family == family &&
		    memcmp(&e->addr, addrp, sizeof(e->addr)) == 0)
			return e;
	return NULL;
}

static void coova_entry_remove(struct coova_table *t, struct coova_entry *e)
{
	list_del(&e->list);
	list_del(&e->lru_list);
	kfree(e);
	t->entries--;
}

static void coova_entry_reset(struct coova_entry *e)
{
	e->state = 0;
	e->bytes_in = 0;
	e->bytes_out = 0;
	e->pkts_in = 0;
	e->pkts_out = 0;
}

static struct coova_entry *
coova_entry_init(struct coova_table *t, const union nf_inet_addr *addr,
		 u_int16_t family)
{
	struct coova_entry *e;

	if (t->entries >= ip_list_tot) {
		e = list_entry(t->lru_list.next, struct coova_entry, lru_list);
		coova_entry_remove(t, e);
	}

	e = kmalloc(sizeof(*e), GFP_ATOMIC);

	if (e == NULL)
		return NULL;

	memcpy(&e->addr, addr, sizeof(e->addr));
	e->index     = 1;
	e->family    = family;

	coova_entry_reset(e);

	if (family == AF_INET6)
		list_add_tail(&e->list, &t->iphash[coova_entry_hash6(addr)]);
	else
		list_add_tail(&e->list, &t->iphash[coova_entry_hash4(addr)]);

	list_add_tail(&e->lru_list, &t->lru_list);
	t->entries++;
	return e;
}

static void coova_entry_update(struct coova_table *t, struct coova_entry *e)
{
	list_move_tail(&e->lru_list, &t->lru_list);
}

static struct coova_table *coova_table_lookup(const char *name)
{
	struct coova_table *t;

	list_for_each_entry(t, &tables, list)
		if (!strcmp(t->name, name))
			return t;
	return NULL;
}

static void coova_table_flush(struct coova_table *t)
{
	struct coova_entry *e, *next;
	unsigned int i;

	for (i = 0; i < ip_list_hash_size; i++)
		list_for_each_entry_safe(e, next, &t->iphash[i], list)
			coova_entry_remove(t, e);
}

static bool
coova_mt(const struct sk_buff *skb, struct xt_action_param *par) 
{
	const struct xt_coova_mtinfo *info = par->matchinfo;
	struct coova_table *t;
	struct coova_entry *e;
	union nf_inet_addr addr = {};
	unsigned char *hwaddr = 0;
	bool ret = 0;

	uint16_t p_bytes = 0;

	if (par->match->family == AF_INET) {
		const struct iphdr *iph = ip_hdr(skb);

		if (info->side == XT_COOVA_DEST)
			addr.ip = iph->daddr;
		else
			addr.ip = iph->saddr;

		p_bytes = iph->tot_len;
	} else {
		const struct ipv6hdr *iph = ipv6_hdr(skb);

		if (info->side == XT_COOVA_DEST)
			memcpy(&addr.in6, &iph->daddr, sizeof(addr.in6));
		else
			memcpy(&addr.in6, &iph->saddr, sizeof(addr.in6));

		p_bytes = iph->payload_len;
	}

	if (info->side != XT_COOVA_DEST) {
		if (skb_mac_header(skb) >= skb->head &&
		    skb_mac_header(skb) + ETH_HLEN <= skb->data) {
			hwaddr = eth_hdr(skb)->h_source;
		} else {
			return ret;
		}
	}

	spin_lock_bh(&coova_lock);
	t = coova_table_lookup(info->name);
	e = coova_entry_lookup(t, &addr, par->match->family);

	if (e == NULL) {
		e = coova_entry_init(t, &addr, par->match->family);
		if (e == NULL)
			goto out;
	}

	if (hwaddr)
		memcpy(e->hwaddr, hwaddr, ETH_ALEN);

	if (e->state) {
		if (info->side == XT_COOVA_DEST) {
			e->bytes_out += (uint64_t) p_bytes;
			e->pkts_out ++;
		} else {
			e->bytes_in += (uint64_t) p_bytes;
			e->pkts_in ++;
		}
	}

	coova_entry_update(t, e);

	ret = e->state;
	
 out:
	spin_unlock_bh(&coova_lock);

	if (info->invert) 
		ret = !ret;

	return ret;
}

static int coova_mt_check(const struct xt_mtchk_param *par)
{
	const struct xt_coova_mtinfo *info = par->matchinfo;
	struct coova_table *t;
#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *pde;
#endif
	unsigned i;
	int ret = 0;

	if (info->name[0] == '\0' ||
	    strnlen(info->name, XT_COOVA_NAME_LEN) == XT_COOVA_NAME_LEN)
		return -EINVAL;

	printk(KERN_INFO "xt_coova: looking for %s\n", info->name);

	mutex_lock(&coova_mutex);
	t = coova_table_lookup(info->name);
	if (t != NULL) {
		t->refcnt++;
		printk(KERN_INFO "xt_coova: found %s refcnt=%d\n", 
		       info->name, t->refcnt);
		goto out;
	}

	t = kzalloc(sizeof(*t) + sizeof(t->iphash[0]) * ip_list_hash_size,
		    GFP_KERNEL);
	if (t == NULL) {
		ret = -ENOMEM;
		goto out;
	}
	t->refcnt = 1;
	strcpy(t->name, info->name);
	INIT_LIST_HEAD(&t->lru_list);
	for (i = 0; i < ip_list_hash_size; i++)
		INIT_LIST_HEAD(&t->iphash[i]);
#ifdef CONFIG_PROC_FS
	pde = proc_create_data(t->name, ip_list_perms, coova_proc_dir,
			       &coova_mt_fops, t);
	if (pde == NULL) {
		kfree(t);
		ret = -ENOMEM;
		goto out;
	}
	pde->uid = ip_list_uid;
	pde->gid = ip_list_gid;
#endif
	spin_lock_bh(&coova_lock);
	list_add_tail(&t->list, &tables);
	spin_unlock_bh(&coova_lock);
	printk(KERN_INFO "xt_coova: created %s refcnt=%d\n", 
	       t->name, t->refcnt);
out:
	mutex_unlock(&coova_mutex);
	printk(KERN_INFO "xt_coova: match ret=%d\n", ret); 
	return ret;
}

static void coova_mt_destroy(const struct xt_mtdtor_param *par)
{
	const struct xt_coova_mtinfo *info = par->matchinfo;
	struct coova_table *t;

	mutex_lock(&coova_mutex);
	t = coova_table_lookup(info->name);
	if (--t->refcnt == 0) {
		spin_lock_bh(&coova_lock);
		list_del(&t->list);
		spin_unlock_bh(&coova_lock);
#ifdef CONFIG_PROC_FS
		remove_proc_entry(t->name, coova_proc_dir);
#endif
		coova_table_flush(t);
		kfree(t);
	}
	mutex_unlock(&coova_mutex);
}

#ifdef CONFIG_PROC_FS
struct coova_iter_state {
	const struct coova_table *table;
	unsigned int bucket;
};

static void *coova_seq_start(struct seq_file *seq, loff_t *pos)
	__acquires(coova_lock)
{
	struct coova_iter_state *st = seq->private;
	const struct coova_table *t = st->table;
	struct coova_entry *e;
	loff_t p = *pos;

	spin_lock_bh(&coova_lock);

	for (st->bucket = 0; st->bucket < ip_list_hash_size; st->bucket++)
		list_for_each_entry(e, &t->iphash[st->bucket], list)
			if (p-- == 0)
				return e;
	return NULL;
}

static void *coova_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	struct coova_iter_state *st = seq->private;
	const struct coova_table *t = st->table;
	const struct coova_entry *e = v;
	const struct list_head *head = e->list.next;

	while (head == &t->iphash[st->bucket]) {
		if (++st->bucket >= ip_list_hash_size)
			return NULL;
		head = t->iphash[st->bucket].next;
	}
	(*pos)++;
	return list_entry(head, struct coova_entry, list);
}

static void coova_seq_stop(struct seq_file *s, void *v)
	__releases(coova_lock)
{
	spin_unlock_bh(&coova_lock);
}

static int coova_seq_show(struct seq_file *seq, void *v)
{
	const struct coova_entry *e = v;
	unsigned int i;

	i = (e->index - 1) % ip_pkt_list_tot;
	if (e->family == AF_INET6)
		seq_printf(seq, "mac=%.2X-%.2X-%.2X-%.2X-%.2X-%.2X src=%pI6",
			   e->hwaddr[0],e->hwaddr[1],e->hwaddr[2],
			   e->hwaddr[3],e->hwaddr[4],e->hwaddr[5],
			   &e->addr.in6);
	else
		seq_printf(seq, "mac=%.2X-%.2X-%.2X-%.2X-%.2X-%.2X src=%pI4",
			   e->hwaddr[0],e->hwaddr[1],e->hwaddr[2],
			   e->hwaddr[3],e->hwaddr[4],e->hwaddr[5],
			   &e->addr.ip);
	seq_printf(seq, " state=%u", e->state);
	seq_printf(seq, " bin=%llu bout=%llu", 
		   (unsigned long long)e->bytes_in, 
		   (unsigned long long)e->bytes_out);
	seq_printf(seq, " pin=%llu pout=%llu", 
		   (unsigned long long)e->pkts_in, 
		   (unsigned long long)e->pkts_out);
	seq_printf(seq, "\n");
	return 0;
}

static const struct seq_operations coova_seq_ops = {
	.start		= coova_seq_start,
	.next		= coova_seq_next,
	.stop		= coova_seq_stop,
	.show		= coova_seq_show,
};

static int coova_seq_open(struct inode *inode, struct file *file)
{
	struct proc_dir_entry *pde = PDE(inode);
	struct coova_iter_state *st;

	st = __seq_open_private(file, &coova_seq_ops, sizeof(*st));
	if (st == NULL)
		return -ENOMEM;

	st->table = pde->data;
	return 0;
}

static ssize_t
coova_mt_proc_write(struct file *file, const char __user *input,
		    size_t size, loff_t *loff)
{
	const struct proc_dir_entry *pde = PDE(file->f_path.dentry->d_inode);
	struct coova_table *t = pde->data;
	struct coova_entry *e;
	char buf[sizeof("+b335:1d35:1e55:dead:c0de:1715:5afe:c0de")];
	const char *c = buf;
	union nf_inet_addr addr = {};
	u_int16_t family;
	bool auth=false;
	bool deauth=false;
	bool release=false;
	bool succ;

	if (size == 0)
		return 0;
	if (size > sizeof(buf))
		size = sizeof(buf);
	if (copy_from_user(buf, input, size) != 0)
		return -EFAULT;

	/* Strict protocol! */
	if (*loff != 0)
		return -ESPIPE;
	switch (*c) {
	case '/': /* flush table */
		spin_lock_bh(&coova_lock);
		coova_table_flush(t);
		spin_unlock_bh(&coova_lock);
		return size;
	case '-': 
		deauth = true;
		break;
	case '+': 
		auth = true;
		break;
	case '*': 
		release = true;
		break;
	default:
		printk(KERN_INFO KBUILD_MODNAME ": Need +ip, -ip, or /\n");
		return -EINVAL;
	}

	++c;
	--size;
	if (strnchr(c, size, ':') != NULL) {
		family = AF_INET6;
		succ   = in6_pton(c, size, (void *)&addr, '\n', NULL);
	} else {
		family = AF_INET;
		succ   = in4_pton(c, size, (void *)&addr, '\n', NULL);
	}

	if (!succ) {
		printk(KERN_INFO KBUILD_MODNAME ": illegal address written "
		       "to procfs\n");
		return -EINVAL;
	}

	spin_lock_bh(&coova_lock);

	e = coova_entry_lookup(t, &addr, family);

	if (release) {

		if (e != NULL)
			coova_entry_remove(t, e);

	} else {

		if (e == NULL) {
			coova_entry_init(t, &addr, family);
		} 

		e = coova_entry_lookup(t, &addr, family);

		if (e != NULL) {
			coova_entry_reset(e);
			
			if (auth)
				e->state = 1;
			else if (deauth)
				e->state = 0;
			
			coova_entry_update(t, e);
		}

	}

	spin_unlock_bh(&coova_lock);

	/* Note we removed one above */
	*loff += size + 1;
	return size + 1;
}

static const struct file_operations coova_mt_fops = {
	.open    = coova_seq_open,
	.read    = seq_read,
	.write   = coova_mt_proc_write,
	.release = seq_release_private,
	.owner   = THIS_MODULE,
};
#endif /* CONFIG_PROC_FS */

static struct xt_match coova_mt_reg[] __read_mostly = {
        {
                .name           = "coova",
                .family         = AF_INET,
                .match          = coova_mt,
                .matchsize      = sizeof(struct xt_coova_mtinfo),
		.checkentry     = coova_mt_check,
		.destroy        = coova_mt_destroy,
                .hooks          = (1 << NF_INET_PRE_ROUTING) |
		                  (1 << NF_INET_POST_ROUTING) |
                                  (1 << NF_INET_LOCAL_IN) |
                                  (1 << NF_INET_LOCAL_OUT) |
                                  (1 << NF_INET_FORWARD),
                .me             = THIS_MODULE,
        },
        {
                .name           = "coova",
                .family         = AF_INET6,
                .match          = coova_mt,
                .matchsize      = sizeof(struct xt_coova_mtinfo),
		.checkentry     = coova_mt_check,
		.destroy        = coova_mt_destroy,
                .hooks          = (1 << NF_INET_PRE_ROUTING) |
		                  (1 << NF_INET_POST_ROUTING) |
                                  (1 << NF_INET_LOCAL_IN) |
                                  (1 << NF_INET_LOCAL_OUT) |
                                  (1 << NF_INET_FORWARD),
                .me             = THIS_MODULE,
        },
};

static int __init coova_mt_init(void)
{
	int err;

	if (!ip_list_tot || !ip_pkt_list_tot || ip_pkt_list_tot > 255)
		return -EINVAL;

	ip_list_hash_size = 1 << fls(ip_list_tot);

	err = xt_register_matches(coova_mt_reg, ARRAY_SIZE(coova_mt_reg));

#ifdef CONFIG_PROC_FS
	if (err < 0) {
		printk(KERN_ERR "xt_coova: could not register match %d\n",err);
		return err;
	}
	coova_proc_dir = proc_mkdir("coova", init_net.proc_net);
	if (coova_proc_dir == NULL) {
		xt_unregister_matches(coova_mt_reg, ARRAY_SIZE(coova_mt_reg));
		err = -ENOMEM;
	}
#endif

	printk(KERN_INFO "xt_coova: ready\n");

	return err;
}

static void __exit coova_mt_exit(void)
{
	printk(KERN_INFO "xt_coova: exit\n");

	/* BUG_ON(!list_empty(&tables)); */

	xt_unregister_matches(coova_mt_reg, ARRAY_SIZE(coova_mt_reg));

#ifdef CONFIG_PROC_FS
	remove_proc_entry("coova", init_net.proc_net);
#endif
}

module_init(coova_mt_init);
module_exit(coova_mt_exit);

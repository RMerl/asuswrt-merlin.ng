/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2004 Netfilter Core Team <coreteam@netfilter.org>
 * (C) 2005-2012 Patrick McHardy <kaber@trash.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/netfilter.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/percpu.h>
#include <linux/netdevice.h>
#include <linux/security.h>
#include <net/net_namespace.h>
#ifdef CONFIG_SYSCTL
#include <linux/sysctl.h>
#endif

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/netfilter/nf_conntrack_l3proto.h>
#include <net/netfilter/nf_conntrack_l4proto.h>
#include <net/netfilter/nf_conntrack_expect.h>
#include <net/netfilter/nf_conntrack_helper.h>
#include <net/netfilter/nf_conntrack_acct.h>
#include <net/netfilter/nf_conntrack_zones.h>
#include <net/netfilter/nf_conntrack_timestamp.h>
#include <linux/rculist_nulls.h>

#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BCM_DPI_MODULE)
#include <linux/dpi.h>
#endif

MODULE_LICENSE("GPL");

#ifdef CONFIG_NF_CONNTRACK_PROCFS
void
print_tuple(struct seq_file *s, const struct nf_conntrack_tuple *tuple,
            const struct nf_conntrack_l3proto *l3proto,
            const struct nf_conntrack_l4proto *l4proto)
{
	l3proto->print_tuple(s, tuple);
	l4proto->print_tuple(s, tuple);
}
EXPORT_SYMBOL_GPL(print_tuple);

struct ct_iter_state {
	struct seq_net_private p;
	unsigned int bucket;
	u_int64_t time_now;
};

static struct hlist_nulls_node *ct_get_first(struct seq_file *seq)
{
	struct net *net = seq_file_net(seq);
	struct ct_iter_state *st = seq->private;
	struct hlist_nulls_node *n;

	for (st->bucket = 0;
	     st->bucket < net->ct.htable_size;
	     st->bucket++) {
		n = rcu_dereference(hlist_nulls_first_rcu(&net->ct.hash[st->bucket]));
		if (!is_a_nulls(n))
			return n;
	}
	return NULL;
}

static struct hlist_nulls_node *ct_get_next(struct seq_file *seq,
				      struct hlist_nulls_node *head)
{
	struct net *net = seq_file_net(seq);
	struct ct_iter_state *st = seq->private;

	head = rcu_dereference(hlist_nulls_next_rcu(head));
	while (is_a_nulls(head)) {
		if (likely(get_nulls_value(head) == st->bucket)) {
			if (++st->bucket >= net->ct.htable_size)
				return NULL;
		}
		head = rcu_dereference(
				hlist_nulls_first_rcu(
					&net->ct.hash[st->bucket]));
	}
	return head;
}

static struct hlist_nulls_node *ct_get_idx(struct seq_file *seq, loff_t pos)
{
	struct hlist_nulls_node *head = ct_get_first(seq);

	if (head)
		while (pos && (head = ct_get_next(seq, head)))
			pos--;
	return pos ? NULL : head;
}

static void *ct_seq_start(struct seq_file *seq, loff_t *pos)
	__acquires(RCU)
{
	struct ct_iter_state *st = seq->private;

	st->time_now = ktime_get_real_ns();
	rcu_read_lock();
	return ct_get_idx(seq, *pos);
}

static void *ct_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	(*pos)++;
	return ct_get_next(s, v);
}

static void ct_seq_stop(struct seq_file *s, void *v)
	__releases(RCU)
{
	rcu_read_unlock();
}

#ifdef CONFIG_NF_CONNTRACK_SECMARK
static void ct_show_secctx(struct seq_file *s, const struct nf_conn *ct)
{
	int ret;
	u32 len;
	char *secctx;

	ret = security_secid_to_secctx(ct->secmark, &secctx, &len);
	if (ret)
		return;

	seq_printf(s, "secctx=%s ", secctx);

	security_release_secctx(secctx, len);
}
#else
static inline void ct_show_secctx(struct seq_file *s, const struct nf_conn *ct)
{
}
#endif

#ifdef CONFIG_NF_CONNTRACK_TIMESTAMP
static void ct_show_delta_time(struct seq_file *s, const struct nf_conn *ct)
{
	struct ct_iter_state *st = s->private;
	struct nf_conn_tstamp *tstamp;
	s64 delta_time;

	tstamp = nf_conn_tstamp_find(ct);
	if (tstamp) {
		delta_time = st->time_now - tstamp->start;
		if (delta_time > 0)
			delta_time = div_s64(delta_time, NSEC_PER_SEC);
		else
			delta_time = 0;

		seq_printf(s, "delta-time=%llu ",
			   (unsigned long long)delta_time);
	}
	return;
}
#else
static inline void
ct_show_delta_time(struct seq_file *s, const struct nf_conn *ct)
{
}
#endif


/* return 0 on success, 1 in case of error */
static int ct_seq_show(struct seq_file *s, void *v)
{
	struct nf_conntrack_tuple_hash *hash = v;
	struct nf_conn *ct = nf_ct_tuplehash_to_ctrack(hash);
	const struct nf_conntrack_l3proto *l3proto;
	const struct nf_conntrack_l4proto *l4proto;
	int ret = 0;
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BCM_KF_NETFILTER) && defined(CONFIG_BLOG)
	BlogCtTime_t ct_time;
	long ctExpiryVal = 0;
#endif

	NF_CT_ASSERT(ct);
	if (unlikely(!atomic_inc_not_zero(&ct->ct_general.use)))
		return 0;

	/* we only want to print DIR_ORIGINAL */
	if (NF_CT_DIRECTION(hash))
		goto release;

	l3proto = __nf_ct_l3proto_find(nf_ct_l3num(ct));
	NF_CT_ASSERT(l3proto);
	l4proto = __nf_ct_l4proto_find(nf_ct_l3num(ct), nf_ct_protonum(ct));
	NF_CT_ASSERT(l4proto);

	ret = -ENOSPC;
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BCM_KF_NETFILTER) && defined(CONFIG_BLOG)
	if (timer_pending(&ct->timeout))
	{
		memset(&ct_time, 0, sizeof(ct_time));
		blog_lock();
		if (ct->blog_key[BLOG_PARAM1_DIR_ORIG] != BLOG_KEY_FC_INVALID || 
				ct->blog_key[BLOG_PARAM1_DIR_REPLY] != BLOG_KEY_FC_INVALID) {
			blog_query(QUERY_FLOWTRACK, (void*)ct, 
					ct->blog_key[BLOG_PARAM1_DIR_ORIG],
					ct->blog_key[BLOG_PARAM1_DIR_REPLY], (unsigned long) &ct_time);

			ctExpiryVal = (long)(ct->extra_jiffies)/HZ - ct_time.idle;

		} else 
			ctExpiryVal = (long)(ct->timeout.expires - jiffies)/HZ;

		blog_unlock();
	}
	seq_printf(s, "%-8s %u %-8s %u %ld ",
			   l3proto->name, nf_ct_l3num(ct),
			   l4proto->name, nf_ct_protonum(ct),
			   ctExpiryVal);
#else
	seq_printf(s, "%-8s %u %-8s %u %ld ",
		   l3proto->name, nf_ct_l3num(ct),
		   l4proto->name, nf_ct_protonum(ct),
		   timer_pending(&ct->timeout)
		   ? (long)(ct->timeout.expires - jiffies)/HZ : 0);
#endif

	if (l4proto->print_conntrack)
		l4proto->print_conntrack(s, ct);

	print_tuple(s, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple,
		    l3proto, l4proto);

	if (seq_has_overflowed(s))
		goto release;

	if (seq_print_acct(s, ct, IP_CT_DIR_ORIGINAL))
		goto release;

	if (!(test_bit(IPS_SEEN_REPLY_BIT, &ct->status)))
		seq_printf(s, "[UNREPLIED] ");

	print_tuple(s, &ct->tuplehash[IP_CT_DIR_REPLY].tuple,
		    l3proto, l4proto);

	if (seq_print_acct(s, ct, IP_CT_DIR_REPLY))
		goto release;

	if (test_bit(IPS_ASSURED_BIT, &ct->status))
		seq_printf(s, "[ASSURED] ");

	if (seq_has_overflowed(s))
		goto release;

#if defined(CONFIG_NF_CONNTRACK_MARK)
	seq_printf(s, "mark=%u ", ct->mark);
#endif

	ct_show_secctx(s, ct);

#ifdef CONFIG_NF_CONNTRACK_ZONES
	seq_printf(s, "zone=%u ", nf_ct_zone(ct));
#endif

	ct_show_delta_time(s, ct);

#if defined(CONFIG_BCM_KF_NETFILTER)
 	if (seq_printf(s, "iqprio=%u ", ct->iq_prio))
 		goto release;

	if (seq_printf(s, "swaccel=%u ", ct->sw_accel_flows))
		goto release;

	if (seq_printf(s, "hwaccel=%u ", ct->hw_accel_flows))
		goto release;
#endif

#if defined(CONFIG_BCM_KF_XT_MATCH_LAYER7) && \
	(defined(CONFIG_NETFILTER_XT_MATCH_LAYER7) || defined(CONFIG_NETFILTER_XT_MATCH_LAYER7_MODULE))
	if(ct->layer7.app_proto &&
		seq_printf(s, "l7proto=%s ", ct->layer7.app_proto))
		return -ENOSPC;
#endif
	
	seq_printf(s, "use=%u\n", atomic_read(&ct->ct_general.use));

	if (seq_has_overflowed(s))
		goto release;

	ret = 0;
release:
	nf_ct_put(ct);
	return ret;
}

static const struct seq_operations ct_seq_ops = {
	.start = ct_seq_start,
	.next  = ct_seq_next,
	.stop  = ct_seq_stop,
	.show  = ct_seq_show
};

static int ct_open(struct inode *inode, struct file *file)
{
	return seq_open_net(inode, file, &ct_seq_ops,
			sizeof(struct ct_iter_state));
}

static const struct file_operations ct_file_ops = {
	.owner   = THIS_MODULE,
	.open    = ct_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release_net,
};

#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BCM_DPI_MODULE)
static void *ct_dpi_seq_start(struct seq_file *seq, loff_t *pos)
	__acquires(RCU)
{
	struct ct_iter_state *st = seq->private;

	st->time_now = ktime_to_ns(ktime_get_real());
	rcu_read_lock();
	if (*pos == 0)
		return SEQ_START_TOKEN;

	return ct_get_idx(seq, *pos);
}

static void *ct_dpi_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	if (v == SEQ_START_TOKEN)
		return ct_get_idx(s, *pos);

	(*pos)++;
	return ct_get_next(s, v);
}

static void ct_dpi_seq_stop(struct seq_file *s, void *v)
	__releases(RCU)
{
	rcu_read_unlock();
}

static int ct_dpi_seq_show(struct seq_file *s, void *v)
{
	struct nf_conntrack_tuple_hash *hash;
	struct nf_conn *ct;
	const struct nf_conntrack_l3proto *l3proto;
	const struct nf_conntrack_l4proto *l4proto;
	int ret = 0;

	if (v == SEQ_START_TOKEN) {
		seq_puts(s, "app_id mac dev_id us_pkt us_byte ds_pkt ds_byte status us_tuple ds_tuple\n");
		return 0;
	}

	hash = v;
	ct = nf_ct_tuplehash_to_ctrack(hash);

	NF_CT_ASSERT(ct);
	if (unlikely(!atomic_inc_not_zero(&ct->ct_general.use)))
		return 0;

	/* only print if app & dev exist, and only for DIR_ORIGINAL */
	if (!ct->dpi.app || !ct->dpi.dev || NF_CT_DIRECTION(hash))
		goto release;

	ret = -ENOSPC;

	l3proto = __nf_ct_l3proto_find(nf_ct_l3num(ct));
	NF_CT_ASSERT(l3proto);
	l4proto = __nf_ct_l4proto_find(nf_ct_l3num(ct), nf_ct_protonum(ct));
	NF_CT_ASSERT(l4proto);

	dpi_print_flow(s, ct);

	if (!dpi_ct_init_from_wan(ct)) {
		/* LAN-side */
		conntrack_dpi_seq_print_stats(s, ct, IP_CT_DIR_ORIGINAL);
		conntrack_dpi_seq_print_stats(s, ct, IP_CT_DIR_REPLY);
	} else {
		/* WAN-side */
		conntrack_dpi_seq_print_stats(s, ct, IP_CT_DIR_REPLY);
		conntrack_dpi_seq_print_stats(s, ct, IP_CT_DIR_ORIGINAL);
	}

	seq_printf(s, " %lX ", ct->dpi.flags);

	if (!dpi_ct_init_from_wan(ct)) {
		print_tuple(s, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple,
			    l3proto, l4proto);
		print_tuple(s, &ct->tuplehash[IP_CT_DIR_REPLY].tuple,
			    l3proto, l4proto);
	} else {
		print_tuple(s, &ct->tuplehash[IP_CT_DIR_REPLY].tuple,
			    l3proto, l4proto);
		print_tuple(s, &ct->tuplehash[IP_CT_DIR_ORIGINAL].tuple,
			    l3proto, l4proto);
	}

	seq_puts(s, "\n");

	if (seq_has_overflowed(s))
		goto release;

	ret = 0;
release:
	nf_ct_put(ct);
	return ret;
}

static const struct seq_operations ct_dpi_seq_ops = {
	.start = ct_dpi_seq_start,
	.next  = ct_dpi_seq_next,
	.stop  = ct_dpi_seq_stop,
	.show  = ct_dpi_seq_show
};

static int ct_dpi_open(struct inode *inode, struct file *file)
{
	return seq_open_net(inode, file, &ct_dpi_seq_ops,
			sizeof(struct ct_iter_state));
}

static const struct file_operations ct_dpi_file_ops = {
	.owner   = THIS_MODULE,
	.open    = ct_dpi_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release_net,
};
#endif /* defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BCM_DPI_MODULE) */

static void *ct_cpu_seq_start(struct seq_file *seq, loff_t *pos)
{
	struct net *net = seq_file_net(seq);
	int cpu;

	if (*pos == 0)
		return SEQ_START_TOKEN;

	for (cpu = *pos-1; cpu < nr_cpu_ids; ++cpu) {
		if (!cpu_possible(cpu))
			continue;
		*pos = cpu + 1;
		return per_cpu_ptr(net->ct.stat, cpu);
	}

	return NULL;
}

static void *ct_cpu_seq_next(struct seq_file *seq, void *v, loff_t *pos)
{
	struct net *net = seq_file_net(seq);
	int cpu;

	for (cpu = *pos; cpu < nr_cpu_ids; ++cpu) {
		if (!cpu_possible(cpu))
			continue;
		*pos = cpu + 1;
		return per_cpu_ptr(net->ct.stat, cpu);
	}

	return NULL;
}

static void ct_cpu_seq_stop(struct seq_file *seq, void *v)
{
}

static int ct_cpu_seq_show(struct seq_file *seq, void *v)
{
	struct net *net = seq_file_net(seq);
	unsigned int nr_conntracks = atomic_read(&net->ct.count);
	const struct ip_conntrack_stat *st = v;

	if (v == SEQ_START_TOKEN) {
		seq_printf(seq, "entries  searched found new invalid ignore delete delete_list insert insert_failed drop early_drop icmp_error  expect_new expect_create expect_delete search_restart\n");
		return 0;
	}

	seq_printf(seq, "%08x  %08x %08x %08x %08x %08x %08x %08x "
			"%08x %08x %08x %08x %08x  %08x %08x %08x %08x\n",
		   nr_conntracks,
		   st->searched,
		   st->found,
		   st->new,
		   st->invalid,
		   st->ignore,
		   st->delete,
		   st->delete_list,
		   st->insert,
		   st->insert_failed,
		   st->drop,
		   st->early_drop,
		   st->error,

		   st->expect_new,
		   st->expect_create,
		   st->expect_delete,
		   st->search_restart
		);
	return 0;
}

static const struct seq_operations ct_cpu_seq_ops = {
	.start	= ct_cpu_seq_start,
	.next	= ct_cpu_seq_next,
	.stop	= ct_cpu_seq_stop,
	.show	= ct_cpu_seq_show,
};

static int ct_cpu_seq_open(struct inode *inode, struct file *file)
{
	return seq_open_net(inode, file, &ct_cpu_seq_ops,
			    sizeof(struct seq_net_private));
}

static const struct file_operations ct_cpu_seq_fops = {
	.owner	 = THIS_MODULE,
	.open	 = ct_cpu_seq_open,
	.read	 = seq_read,
	.llseek	 = seq_lseek,
	.release = seq_release_net,
};

static int nf_conntrack_standalone_init_proc(struct net *net)
{
	struct proc_dir_entry *pde;

	pde = proc_create("nf_conntrack", 0440, net->proc_net, &ct_file_ops);
	if (!pde)
		goto out_nf_conntrack;

	pde = proc_create("nf_conntrack", S_IRUGO, net->proc_net_stat,
			  &ct_cpu_seq_fops);
	if (!pde)
		goto out_stat_nf_conntrack;
#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BCM_DPI_MODULE)
	pde = proc_create("nf_conntrack_dpi", 0440, net->proc_net,
			  &ct_dpi_file_ops);
	if (!pde)
		goto out_nf_conntrack_dpi;
#endif

	return 0;

#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BCM_DPI_MODULE)
out_nf_conntrack_dpi:
	remove_proc_entry("nf_conntrack", net->proc_net_stat);
#endif
out_stat_nf_conntrack:
	remove_proc_entry("nf_conntrack", net->proc_net);
out_nf_conntrack:
	return -ENOMEM;
}

static void nf_conntrack_standalone_fini_proc(struct net *net)
{
#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BCM_DPI_MODULE)
	remove_proc_entry("nf_conntrack_dpi", net->proc_net);
#endif
	remove_proc_entry("nf_conntrack", net->proc_net_stat);
	remove_proc_entry("nf_conntrack", net->proc_net);
}
#else
static int nf_conntrack_standalone_init_proc(struct net *net)
{
	return 0;
}

static void nf_conntrack_standalone_fini_proc(struct net *net)
{
}
#endif /* CONFIG_NF_CONNTRACK_PROCFS */

/* Sysctl support */

#ifdef CONFIG_SYSCTL
/* Log invalid packets of a given protocol */
static int log_invalid_proto_min = 0;
static int log_invalid_proto_max = 255;

static struct ctl_table_header *nf_ct_netfilter_header;

static struct ctl_table nf_ct_sysctl_table[] = {
	{
		.procname	= "nf_conntrack_max",
		.data		= &nf_conntrack_max,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "nf_conntrack_count",
		.data		= &init_net.ct.count,
		.maxlen		= sizeof(int),
		.mode		= 0444,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname       = "nf_conntrack_buckets",
		.data           = &init_net.ct.htable_size,
		.maxlen         = sizeof(unsigned int),
		.mode           = 0444,
		.proc_handler   = proc_dointvec,
	},
	{
		.procname	= "nf_conntrack_checksum",
		.data		= &init_net.ct.sysctl_checksum,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{
		.procname	= "nf_conntrack_log_invalid",
		.data		= &init_net.ct.sysctl_log_invalid,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec_minmax,
		.extra1		= &log_invalid_proto_min,
		.extra2		= &log_invalid_proto_max,
	},
	{
		.procname	= "nf_conntrack_expect_max",
		.data		= &nf_ct_expect_max,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{ }
};

#define NET_NF_CONNTRACK_MAX 2089

static struct ctl_table nf_ct_netfilter_table[] = {
	{
		.procname	= "nf_conntrack_max",
		.data		= &nf_conntrack_max,
		.maxlen		= sizeof(int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{ }
};

static int nf_conntrack_standalone_init_sysctl(struct net *net)
{
	struct ctl_table *table;

	table = kmemdup(nf_ct_sysctl_table, sizeof(nf_ct_sysctl_table),
			GFP_KERNEL);
	if (!table)
		goto out_kmemdup;

	table[1].data = &net->ct.count;
	table[2].data = &net->ct.htable_size;
	table[3].data = &net->ct.sysctl_checksum;
	table[4].data = &net->ct.sysctl_log_invalid;

	/* Don't export sysctls to unprivileged users */
	if (net->user_ns != &init_user_ns)
		table[0].procname = NULL;

	net->ct.sysctl_header = register_net_sysctl(net, "net/netfilter", table);
	if (!net->ct.sysctl_header)
		goto out_unregister_netfilter;

	return 0;

out_unregister_netfilter:
	kfree(table);
out_kmemdup:
	return -ENOMEM;
}

static void nf_conntrack_standalone_fini_sysctl(struct net *net)
{
	struct ctl_table *table;

	table = net->ct.sysctl_header->ctl_table_arg;
	unregister_net_sysctl_table(net->ct.sysctl_header);
	kfree(table);
}
#else
static int nf_conntrack_standalone_init_sysctl(struct net *net)
{
	return 0;
}

static void nf_conntrack_standalone_fini_sysctl(struct net *net)
{
}
#endif /* CONFIG_SYSCTL */

static int nf_conntrack_pernet_init(struct net *net)
{
	int ret;

	ret = nf_conntrack_init_net(net);
	if (ret < 0)
		goto out_init;

	ret = nf_conntrack_standalone_init_proc(net);
	if (ret < 0)
		goto out_proc;

	net->ct.sysctl_checksum = 1;
	net->ct.sysctl_log_invalid = 0;
	ret = nf_conntrack_standalone_init_sysctl(net);
	if (ret < 0)
		goto out_sysctl;

	return 0;

out_sysctl:
	nf_conntrack_standalone_fini_proc(net);
out_proc:
	nf_conntrack_cleanup_net(net);
out_init:
	return ret;
}

static void nf_conntrack_pernet_exit(struct list_head *net_exit_list)
{
	struct net *net;

	list_for_each_entry(net, net_exit_list, exit_list) {
		nf_conntrack_standalone_fini_sysctl(net);
		nf_conntrack_standalone_fini_proc(net);
	}
	nf_conntrack_cleanup_net_list(net_exit_list);
}

static struct pernet_operations nf_conntrack_net_ops = {
	.init		= nf_conntrack_pernet_init,
	.exit_batch	= nf_conntrack_pernet_exit,
};

static int __init nf_conntrack_standalone_init(void)
{
	int ret = nf_conntrack_init_start();
	if (ret < 0)
		goto out_start;

#ifdef CONFIG_SYSCTL
	nf_ct_netfilter_header =
		register_net_sysctl(&init_net, "net", nf_ct_netfilter_table);
	if (!nf_ct_netfilter_header) {
		pr_err("nf_conntrack: can't register to sysctl.\n");
		ret = -ENOMEM;
		goto out_sysctl;
	}
#endif

	ret = register_pernet_subsys(&nf_conntrack_net_ops);
	if (ret < 0)
		goto out_pernet;

	nf_conntrack_init_end();
	return 0;

out_pernet:
#ifdef CONFIG_SYSCTL
	unregister_net_sysctl_table(nf_ct_netfilter_header);
out_sysctl:
#endif
	nf_conntrack_cleanup_end();
out_start:
	return ret;
}

static void __exit nf_conntrack_standalone_fini(void)
{
	nf_conntrack_cleanup_start();
	unregister_pernet_subsys(&nf_conntrack_net_ops);
#ifdef CONFIG_SYSCTL
	unregister_net_sysctl_table(nf_ct_netfilter_header);
#endif
	nf_conntrack_cleanup_end();
}

module_init(nf_conntrack_standalone_init);
module_exit(nf_conntrack_standalone_fini);

/* Some modules need us, but don't depend directly on any symbol.
   They should call this. */
void need_conntrack(void)
{
}
EXPORT_SYMBOL_GPL(need_conntrack);

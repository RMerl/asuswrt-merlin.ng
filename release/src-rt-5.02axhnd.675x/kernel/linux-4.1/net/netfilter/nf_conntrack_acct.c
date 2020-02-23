/* Accouting handling for netfilter. */

/*
 * (C) 2008 Krzysztof Piotr Oledzki <ole@ans.pl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/netfilter.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/export.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_extend.h>
#include <net/netfilter/nf_conntrack_acct.h>

#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BCM_DPI_MODULE)
static bool nf_ct_acct __read_mostly = 1;
#else
static bool nf_ct_acct __read_mostly;
#endif

module_param_named(acct, nf_ct_acct, bool, 0644);
MODULE_PARM_DESC(acct, "Enable connection tracking flow accounting.");

#ifdef CONFIG_SYSCTL
static struct ctl_table acct_sysctl_table[] = {
	{
		.procname	= "nf_conntrack_acct",
		.data		= &init_net.ct.sysctl_acct,
		.maxlen		= sizeof(unsigned int),
		.mode		= 0644,
		.proc_handler	= proc_dointvec,
	},
	{}
};
#endif /* CONFIG_SYSCTL */

unsigned int
seq_print_acct(struct seq_file *s, const struct nf_conn *ct, int dir)
{
	struct nf_conn_acct *acct;
	struct nf_conn_counter *counter;

	acct = nf_conn_acct_find(ct);
	if (!acct)
		return 0;

	counter = acct->counter;
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	{
		unsigned long long pkts;
		unsigned long long bytes;
		BlogFcStats_t fast_stats;

		pkts = (unsigned long long)atomic64_read(&counter[dir].packets);
		bytes = (unsigned long long)atomic64_read(&counter[dir].bytes);

		fast_stats.rx_packets = 0;
		fast_stats.rx_bytes = 0;

		if(blog_ct_get_stats(ct, ct->blog_key[dir], dir, &fast_stats) == 0)
			counter[dir].ts = fast_stats.pollTS_ms;

		seq_printf(s, "packets=%llu bytes=%llu ",
			  pkts+counter[dir].cum_fast_pkts+fast_stats.rx_packets,
			  bytes+counter[dir].cum_fast_bytes+fast_stats.rx_bytes);
	}
#else
		seq_printf(s, "packets=%llu bytes=%llu ",
		   (unsigned long long)atomic64_read(&counter[dir].packets),
		   (unsigned long long)atomic64_read(&counter[dir].bytes));

#endif
	return 0;
};
EXPORT_SYMBOL_GPL(seq_print_acct);

#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BCM_DPI_MODULE)
static void conntrack_dpi_refresh_conn_stats(struct nf_conn *ct, int dir,
					     struct dpi_ct_stats *stats)
{
	struct nf_conn_acct *acct;
	struct nf_conn_counter *counter;
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	BlogFcStats_t fast_stats;
#endif

	if (!ct->dpi.appinst)
		return;

	acct = nf_conn_acct_find(ct);
	if (!acct)
		return;
	counter = acct->counter;

	/* refresh current connection stats */
	stats->pkts = atomic64_read(&counter[dir].packets);
	stats->bytes = atomic64_read(&counter[dir].bytes);

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	fast_stats.rx_packets = 0;
	fast_stats.rx_bytes = 0;

	if(blog_ct_get_stats(ct, ct->blog_key[dir], dir, &fast_stats) == 0)
		counter[dir].ts = fast_stats.pollTS_ms;

	/* update stats from accelerators */
	stats->pkts +=counter[dir].cum_fast_pkts + fast_stats.rx_packets;
	stats->bytes += counter[dir].cum_fast_bytes + fast_stats.rx_bytes;
#endif
}

void conntrack_dpi_evict_conn(struct nf_conn *ct, int dir)
{
	struct dpi_ct_stats *appinst_stats;
	struct dpi_ct_stats *dev_stats;
	struct nf_conn_acct *acct;
	struct nf_conn_counter *counter;
	uint64_t packets, bytes;

	acct = nf_conn_acct_find(ct);
	if (!acct)
		return;
	counter = acct->counter;

	packets = atomic64_read(&counter[dir].packets);
	bytes = atomic64_read(&counter[dir].bytes);

	pr_debug("app<%p> dev<%p> appinst<%p> dir<%s> counter<%llu, %llu>",
		 ct->dpi.app, ct->dpi.dev, ct->dpi.appinst,
		 dir == IP_CT_DIR_ORIGINAL ? "orig" : "reply",
		 packets, bytes);

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	pr_debug(" blog_counter<%lu, %llu>\n",
		 counter[dir].cum_fast_pkts,
		 counter[dir].cum_fast_bytes);
	packets += counter[dir].cum_fast_pkts;
	bytes += counter[dir].cum_fast_bytes;
#endif

	/* update cumulative appinst stats */
	if (ct->dpi.appinst) {
		appinst_stats = dpi_appinst_stats(ct, dir);

		appinst_stats->pkts += packets;
		appinst_stats->bytes += bytes;

		pr_debug(" appinst_stats<%llu, %llu>",
			 appinst_stats->pkts,
			 appinst_stats->bytes);
	}

	/* update cumulative device stats */
	if (ct->dpi.dev) {
		dev_stats = dpi_dev_stats(ct, dir);

		dev_stats->pkts += packets;
		dev_stats->bytes += bytes;

		pr_debug(" dev_stats<%llu, %llu>",
			 dev_stats->pkts,
			 dev_stats->bytes);
	}

	pr_debug("\n");
}

int conntrack_dpi_seq_print_stats(struct seq_file *s, struct nf_conn *ct,
				  int dir)
{
	struct dpi_ct_stats stats = {0};

	/* get updated stats for the connection */
	conntrack_dpi_refresh_conn_stats(ct, dir, &stats);

	return seq_printf(s, " %llu %llu", stats.pkts, stats.bytes);
}
#endif /* defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BCM_DPI_MODULE) */

static struct nf_ct_ext_type acct_extend __read_mostly = {
	.len	= sizeof(struct nf_conn_acct),
	.align	= __alignof__(struct nf_conn_acct),
	.id	= NF_CT_EXT_ACCT,
};

#ifdef CONFIG_SYSCTL
static int nf_conntrack_acct_init_sysctl(struct net *net)
{
	struct ctl_table *table;

	table = kmemdup(acct_sysctl_table, sizeof(acct_sysctl_table),
			GFP_KERNEL);
	if (!table)
		goto out;

	table[0].data = &net->ct.sysctl_acct;

	/* Don't export sysctls to unprivileged users */
	if (net->user_ns != &init_user_ns)
		table[0].procname = NULL;

	net->ct.acct_sysctl_header = register_net_sysctl(net, "net/netfilter",
							 table);
	if (!net->ct.acct_sysctl_header) {
		printk(KERN_ERR "nf_conntrack_acct: can't register to sysctl.\n");
		goto out_register;
	}
	return 0;

out_register:
	kfree(table);
out:
	return -ENOMEM;
}

static void nf_conntrack_acct_fini_sysctl(struct net *net)
{
	struct ctl_table *table;

	table = net->ct.acct_sysctl_header->ctl_table_arg;
	unregister_net_sysctl_table(net->ct.acct_sysctl_header);
	kfree(table);
}
#else
static int nf_conntrack_acct_init_sysctl(struct net *net)
{
	return 0;
}

static void nf_conntrack_acct_fini_sysctl(struct net *net)
{
}
#endif

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
/*
 *---------------------------------------------------------------------------
 * Function Name: flwStPushFunc
 *---------------------------------------------------------------------------
 */
void flwStPushFunc( void *ct, uint32_t dir, const BlogFcStats_t *flwSt_p )
{
	struct nf_conn_acct *acct;
	struct nf_conn_counter *counter;

	if (flwSt_p == NULL)
		return ;

	if (ct != NULL)
	{
		acct = nf_conn_acct_find((struct nf_conn *)ct);
		if (acct)
		{
			counter = acct->counter;
			counter[dir].cum_fast_pkts += flwSt_p->rx_packets;
			counter[dir].cum_fast_bytes += flwSt_p->rx_bytes;
			counter[dir].ts = flwSt_p->pollTS_ms;
		}
	}
}
#endif
int nf_conntrack_acct_pernet_init(struct net *net)
{
	net->ct.sysctl_acct = nf_ct_acct;
	return nf_conntrack_acct_init_sysctl(net);
}

void nf_conntrack_acct_pernet_fini(struct net *net)
{
	nf_conntrack_acct_fini_sysctl(net);
}

int nf_conntrack_acct_init(void)
{
	int ret = nf_ct_extend_register(&acct_extend);
	if (ret < 0)
		pr_err("nf_conntrack_acct: Unable to register extension\n");
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	blog_ct_put_stats_fn = flwStPushFunc;
#endif

	return ret;
}

void nf_conntrack_acct_fini(void)
{
	nf_ct_extend_unregister(&acct_extend);
}

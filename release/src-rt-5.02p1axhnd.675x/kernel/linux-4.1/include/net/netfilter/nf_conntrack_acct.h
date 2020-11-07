/*
 * (C) 2008 Krzysztof Piotr Oledzki <ole@ans.pl>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef _NF_CONNTRACK_ACCT_H
#define _NF_CONNTRACK_ACCT_H
#include <net/net_namespace.h>
#include <linux/netfilter/nf_conntrack_common.h>
#include <linux/netfilter/nf_conntrack_tuple_common.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_extend.h>
#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BCM_DPI_MODULE)
#include <linux/dpi.h>
#endif


struct nf_conn_counter {
	atomic64_t packets;
	atomic64_t bytes;
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	unsigned long cum_fast_pkts;
	unsigned long long cum_fast_bytes;
	unsigned long ts;
#endif    
};

struct nf_conn_acct {
	struct nf_conn_counter counter[IP_CT_DIR_MAX];
};

static inline
struct nf_conn_acct *nf_conn_acct_find(const struct nf_conn *ct)
{
	return nf_ct_ext_find(ct, NF_CT_EXT_ACCT);
}

static inline
struct nf_conn_acct *nf_ct_acct_ext_add(struct nf_conn *ct, gfp_t gfp)
{
	struct net *net = nf_ct_net(ct);
	struct nf_conn_acct *acct;

	if (!net->ct.sysctl_acct)
		return NULL;

	acct = nf_ct_ext_add(ct, NF_CT_EXT_ACCT, gfp);
	if (!acct)
		pr_debug("failed to add accounting extension area");


	return acct;
};

unsigned int seq_print_acct(struct seq_file *s, const struct nf_conn *ct,
			    int dir);
#if defined(CONFIG_BCM_KF_DPI) && defined(CONFIG_BCM_DPI_MODULE)
void conntrack_dpi_evict_conn(struct nf_conn *ct, int dir);
int conntrack_dpi_seq_print_stats(struct seq_file *s, struct nf_conn *ct,
				  int dir);
#endif

/* Check if connection tracking accounting is enabled */
static inline bool nf_ct_acct_enabled(struct net *net)
{
	return net->ct.sysctl_acct != 0;
}

/* Enable/disable connection tracking accounting */
static inline void nf_ct_set_acct(struct net *net, bool enable)
{
	net->ct.sysctl_acct = enable;
}

int nf_conntrack_acct_pernet_init(struct net *net);
void nf_conntrack_acct_pernet_fini(struct net *net);

int nf_conntrack_acct_init(void);
void nf_conntrack_acct_fini(void);
#endif /* _NF_CONNTRACK_ACCT_H */

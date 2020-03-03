/*
 * Copyright (c) 2008-2009 Patrick McHardy <kaber@trash.net>
 * Copyright (c) 2012 Pablo Neira Ayuso <pablo@netfilter.org>
 * Copyright (c) 2012 Intel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/netfilter/nf_tables.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_nat.h>
#include <net/netfilter/nf_nat_core.h>
#include <net/netfilter/nf_tables.h>
#include <net/netfilter/nf_tables_ipv4.h>
#include <net/netfilter/nf_nat_l3proto.h>
#include <net/ip.h>

static unsigned int nft_nat_do_chain(const struct nf_hook_ops *ops,
				      struct sk_buff *skb,
				      const struct nf_hook_state *state,
				      struct nf_conn *ct)
{
	struct nft_pktinfo pkt;

	nft_set_pktinfo_ipv4(&pkt, ops, skb, state);

	return nft_do_chain(&pkt, ops);
}

static unsigned int nft_nat_ipv4_fn(const struct nf_hook_ops *ops,
				    struct sk_buff *skb,
				    const struct nf_hook_state *state)
{
	return nf_nat_ipv4_fn(ops, skb, state, nft_nat_do_chain);
}

static unsigned int nft_nat_ipv4_in(const struct nf_hook_ops *ops,
				    struct sk_buff *skb,
				    const struct nf_hook_state *state)
{
	return nf_nat_ipv4_in(ops, skb, state, nft_nat_do_chain);
}

static unsigned int nft_nat_ipv4_out(const struct nf_hook_ops *ops,
				     struct sk_buff *skb,
				     const struct nf_hook_state *state)
{
	return nf_nat_ipv4_out(ops, skb, state, nft_nat_do_chain);
}

static unsigned int nft_nat_ipv4_local_fn(const struct nf_hook_ops *ops,
					  struct sk_buff *skb,
					  const struct nf_hook_state *state)
{
	return nf_nat_ipv4_local_fn(ops, skb, state, nft_nat_do_chain);
}

static const struct nf_chain_type nft_chain_nat_ipv4 = {
	.name		= "nat",
	.type		= NFT_CHAIN_T_NAT,
	.family		= NFPROTO_IPV4,
	.owner		= THIS_MODULE,
	.hook_mask	= (1 << NF_INET_PRE_ROUTING) |
			  (1 << NF_INET_POST_ROUTING) |
			  (1 << NF_INET_LOCAL_OUT) |
			  (1 << NF_INET_LOCAL_IN),
	.hooks		= {
		[NF_INET_PRE_ROUTING]	= nft_nat_ipv4_in,
		[NF_INET_POST_ROUTING]	= nft_nat_ipv4_out,
		[NF_INET_LOCAL_OUT]	= nft_nat_ipv4_local_fn,
		[NF_INET_LOCAL_IN]	= nft_nat_ipv4_fn,
	},
};

static int __init nft_chain_nat_init(void)
{
	int err;

	err = nft_register_chain_type(&nft_chain_nat_ipv4);
	if (err < 0)
		return err;

	return 0;
}

static void __exit nft_chain_nat_exit(void)
{
	nft_unregister_chain_type(&nft_chain_nat_ipv4);
}

module_init(nft_chain_nat_init);
module_exit(nft_chain_nat_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Patrick McHardy <kaber@trash.net>");
MODULE_ALIAS_NFT_CHAIN(AF_INET, "nat");

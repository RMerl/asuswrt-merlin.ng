/*
 * Copyright (c) 2008 Patrick McHardy <kaber@trash.net>
 * Copyright (c) 2012-2013 Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/ipv6.h>
#include <linux/netfilter_ipv6.h>
#include <net/netfilter/nf_tables.h>
#include <net/netfilter/nf_tables_ipv6.h>

static unsigned int nft_do_chain_ipv6(const struct nf_hook_ops *ops,
				      struct sk_buff *skb,
				      const struct nf_hook_state *state)
{
	struct nft_pktinfo pkt;

	/* malformed packet, drop it */
	if (nft_set_pktinfo_ipv6(&pkt, ops, skb, state) < 0)
		return NF_DROP;

	return nft_do_chain(&pkt, ops);
}

static unsigned int nft_ipv6_output(const struct nf_hook_ops *ops,
				    struct sk_buff *skb,
				    const struct nf_hook_state *state)
{
	if (unlikely(skb->len < sizeof(struct ipv6hdr))) {
		if (net_ratelimit())
			pr_info("nf_tables_ipv6: ignoring short SOCK_RAW "
				"packet\n");
		return NF_ACCEPT;
	}

	return nft_do_chain_ipv6(ops, skb, state);
}

struct nft_af_info nft_af_ipv6 __read_mostly = {
	.family		= NFPROTO_IPV6,
	.nhooks		= NF_INET_NUMHOOKS,
	.owner		= THIS_MODULE,
	.nops		= 1,
	.hooks		= {
		[NF_INET_LOCAL_IN]	= nft_do_chain_ipv6,
		[NF_INET_LOCAL_OUT]	= nft_ipv6_output,
		[NF_INET_FORWARD]	= nft_do_chain_ipv6,
		[NF_INET_PRE_ROUTING]	= nft_do_chain_ipv6,
		[NF_INET_POST_ROUTING]	= nft_do_chain_ipv6,
	},
};
EXPORT_SYMBOL_GPL(nft_af_ipv6);

static int nf_tables_ipv6_init_net(struct net *net)
{
	net->nft.ipv6 = kmalloc(sizeof(struct nft_af_info), GFP_KERNEL);
	if (net->nft.ipv6 == NULL)
		return -ENOMEM;

	memcpy(net->nft.ipv6, &nft_af_ipv6, sizeof(nft_af_ipv6));

	if (nft_register_afinfo(net, net->nft.ipv6) < 0)
		goto err;

	return 0;
err:
	kfree(net->nft.ipv6);
	return -ENOMEM;
}

static void nf_tables_ipv6_exit_net(struct net *net)
{
	nft_unregister_afinfo(net->nft.ipv6);
	kfree(net->nft.ipv6);
}

static struct pernet_operations nf_tables_ipv6_net_ops = {
	.init	= nf_tables_ipv6_init_net,
	.exit	= nf_tables_ipv6_exit_net,
};

static const struct nf_chain_type filter_ipv6 = {
	.name		= "filter",
	.type		= NFT_CHAIN_T_DEFAULT,
	.family		= NFPROTO_IPV6,
	.owner		= THIS_MODULE,
	.hook_mask	= (1 << NF_INET_LOCAL_IN) |
			  (1 << NF_INET_LOCAL_OUT) |
			  (1 << NF_INET_FORWARD) |
			  (1 << NF_INET_PRE_ROUTING) |
			  (1 << NF_INET_POST_ROUTING),
};

static int __init nf_tables_ipv6_init(void)
{
	int ret;

	nft_register_chain_type(&filter_ipv6);
	ret = register_pernet_subsys(&nf_tables_ipv6_net_ops);
	if (ret < 0)
		nft_unregister_chain_type(&filter_ipv6);

	return ret;
}

static void __exit nf_tables_ipv6_exit(void)
{
	unregister_pernet_subsys(&nf_tables_ipv6_net_ops);
	nft_unregister_chain_type(&filter_ipv6);
}

module_init(nf_tables_ipv6_init);
module_exit(nf_tables_ipv6_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Patrick McHardy <kaber@trash.net>");
MODULE_ALIAS_NFT_FAMILY(AF_INET6);

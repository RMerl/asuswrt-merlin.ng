/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2004 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/ip.h>
#include <linux/netfilter.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <net/route.h>
#include <net/ip.h>

#include <linux/netfilter_bridge.h>
#include <linux/netfilter_ipv4.h>
#include <net/netfilter/ipv4/nf_defrag_ipv4.h>
#if IS_ENABLED(CONFIG_NF_CONNTRACK)
#include <net/netfilter/nf_conntrack.h>
#endif
#include <net/netfilter/nf_conntrack_zones.h>

static int nf_ct_ipv4_gather_frags(struct sk_buff *skb, u_int32_t user)
{
	int err;

	skb_orphan(skb);

	local_bh_disable();
	err = ip_defrag(skb, user);
	local_bh_enable();

	if (!err) {
		ip_send_check(ip_hdr(skb));
		skb->ignore_df = 1;
	}

	return err;
}

static enum ip_defrag_users nf_ct_defrag_user(unsigned int hooknum,
					      struct sk_buff *skb)
{
	u16 zone = NF_CT_DEFAULT_ZONE;

#if IS_ENABLED(CONFIG_NF_CONNTRACK)
	if (skb->nfct)
		zone = nf_ct_zone((struct nf_conn *)skb->nfct);
#endif

#if IS_ENABLED(CONFIG_BRIDGE_NETFILTER)
	if (skb->nf_bridge &&
	    skb->nf_bridge->mask & BRNF_NF_BRIDGE_PREROUTING)
		return IP_DEFRAG_CONNTRACK_BRIDGE_IN + zone;
#endif
	if (hooknum == NF_INET_PRE_ROUTING)
		return IP_DEFRAG_CONNTRACK_IN + zone;
	else
		return IP_DEFRAG_CONNTRACK_OUT + zone;
}

static unsigned int ipv4_conntrack_defrag(const struct nf_hook_ops *ops,
					  struct sk_buff *skb,
					  const struct nf_hook_state *state)
{
	struct sock *sk = skb->sk;
	struct inet_sock *inet = inet_sk(skb->sk);

	if (sk && (sk->sk_family == PF_INET) &&
	    inet->nodefrag)
		return NF_ACCEPT;

#if IS_ENABLED(CONFIG_NF_CONNTRACK)
#if !IS_ENABLED(CONFIG_NF_NAT)
	/* Previously seen (loopback)?  Ignore.  Do this before
	   fragment check. */
	if (skb->nfct && !nf_ct_is_template((struct nf_conn *)skb->nfct))
		return NF_ACCEPT;
#endif
#endif
	/* Gather fragments. */
	if (ip_is_fragment(ip_hdr(skb))) {
		enum ip_defrag_users user =
			nf_ct_defrag_user(ops->hooknum, skb);

		if (nf_ct_ipv4_gather_frags(skb, user))
			return NF_STOLEN;
	}
	return NF_ACCEPT;
}

static struct nf_hook_ops ipv4_defrag_ops[] = {
	{
		.hook		= ipv4_conntrack_defrag,
		.owner		= THIS_MODULE,
		.pf		= NFPROTO_IPV4,
		.hooknum	= NF_INET_PRE_ROUTING,
		.priority	= NF_IP_PRI_CONNTRACK_DEFRAG,
	},
	{
		.hook           = ipv4_conntrack_defrag,
		.owner          = THIS_MODULE,
		.pf             = NFPROTO_IPV4,
		.hooknum        = NF_INET_LOCAL_OUT,
		.priority       = NF_IP_PRI_CONNTRACK_DEFRAG,
	},
};

static int __init nf_defrag_init(void)
{
	return nf_register_hooks(ipv4_defrag_ops, ARRAY_SIZE(ipv4_defrag_ops));
}

static void __exit nf_defrag_fini(void)
{
	nf_unregister_hooks(ipv4_defrag_ops, ARRAY_SIZE(ipv4_defrag_ops));
}

void nf_defrag_ipv4_enable(void)
{
}
EXPORT_SYMBOL_GPL(nf_defrag_ipv4_enable);

module_init(nf_defrag_init);
module_exit(nf_defrag_fini);

MODULE_LICENSE("GPL");

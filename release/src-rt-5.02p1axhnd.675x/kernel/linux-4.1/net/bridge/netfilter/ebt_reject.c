#if defined(CONFIG_BCM_KF_NETFILTER)
/*
*    Copyright (c) 2003-2019 Broadcom Corporation
*    All Rights Reserved
*
<:label-BRCM:2012:GPL/GPL:standard

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation (the "GPL").

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.


A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

:>
*/

/*
 *  ebt_reject
 *
 */
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_reject.h>
#include <linux/module.h>
#include <linux/ip.h>
#if defined(CONFIG_BCM_KF_IP) && defined(CONFIG_IPV6)
#include <linux/ipv6.h>
#include <net/netfilter/ipv6/nf_reject.h>
#include <net/ip6_checksum.h>
#endif
#include <linux/skbuff.h>
#include "../br_private.h"

#if defined(CONFIG_BCM_KF_IP) && defined(CONFIG_IPV6)
static struct ipv6hdr * ebt_reject_ip6hdr_put(struct sk_buff *nskb,
				     const struct sk_buff *oldskb,
				     __u8 protocol, int hoplimit)
{
	struct ipv6hdr *ip6h;
	const struct ipv6hdr *oip6h = ipv6_hdr(oldskb);
#define DEFAULT_TOS_VALUE	0x0U
	const __u8 tclass = DEFAULT_TOS_VALUE;

	skb_put(nskb, sizeof(struct ipv6hdr));
	skb_reset_network_header(nskb);
	ip6h = ipv6_hdr(nskb);
	ip6_flow_hdr(ip6h, tclass, 0);
	ip6h->hop_limit = hoplimit;
	ip6h->nexthdr = protocol;
	ip6h->saddr = oip6h->daddr;
	ip6h->daddr = oip6h->saddr;
	nskb->protocol = htons(ETH_P_IPV6);

	return ip6h;
}

static void ebt_reject_br_push_etherhdr(struct sk_buff *oldskb,
					struct sk_buff *nskb)
{
	struct ethhdr *eth;

	eth = (struct ethhdr *)skb_push(nskb, ETH_HLEN);
	skb_reset_mac_header(nskb);
	ether_addr_copy(eth->h_source, eth_hdr(oldskb)->h_dest);
	ether_addr_copy(eth->h_dest, eth_hdr(oldskb)->h_source);
	eth->h_proto = eth_hdr(oldskb)->h_proto;
	skb_pull(nskb, ETH_HLEN);
}

static void ebt_reject_br_send_v6_unreach(struct net *net,
					  struct sk_buff *oldskb,
					  const struct net_device *dev, u8 code)
{
	struct sk_buff *nskb;
	struct ipv6hdr *nip6h;
	struct icmp6hdr *icmp6h;
	unsigned int len;
	void *payload;

	/* Include "As much of invoking packet as possible without the ICMPv6
	 * packet exceeding the minimum IPv6 MTU" in the ICMP payload.
	 */
	len = min_t(unsigned int, 1220, oldskb->len);

	if (!pskb_may_pull(oldskb, len))
		return;

	nskb = alloc_skb(sizeof(struct ipv6hdr) + sizeof(struct icmp6hdr) +
			 LL_MAX_HEADER + len, GFP_ATOMIC);
	if (!nskb)
		return;

	skb_reserve(nskb, LL_MAX_HEADER);
	nip6h = ebt_reject_ip6hdr_put(nskb, oldskb, IPPROTO_ICMPV6,
				     net->ipv6.devconf_all->hop_limit);

	skb_reset_transport_header(nskb);
	icmp6h = (struct icmp6hdr *)skb_put(nskb, sizeof(struct icmp6hdr));
	memset(icmp6h, 0, sizeof(*icmp6h));
	icmp6h->icmp6_type = ICMPV6_DEST_UNREACH;
	icmp6h->icmp6_code = code;

	payload = skb_put(nskb, len);
	memcpy(payload, skb_network_header(oldskb), len);
	nip6h->payload_len = htons(nskb->len - sizeof(struct ipv6hdr));

	icmp6h->icmp6_cksum =
		csum_ipv6_magic(&nip6h->saddr, &nip6h->daddr,
				nskb->len - sizeof(struct ipv6hdr),
				IPPROTO_ICMPV6,
				csum_partial(icmp6h,
					     nskb->len - sizeof(struct ipv6hdr),
					     0));

	ebt_reject_br_push_etherhdr(oldskb, nskb);
	nskb->dev = oldskb->dev;

	br_dev_queue_push_xmit(NULL, nskb);
}
#endif

static unsigned int ebt_reject_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct ebt_reject_info *reject = par->targinfo;

	pr_debug("%s: medium point\n", __func__);
	switch (reject->with) {
		case EBT_ICMP6_POLICY_FAIL:
#if defined(CONFIG_BCM_KF_IP) && defined(CONFIG_IPV6)	
			ebt_reject_br_send_v6_unreach(dev_net(skb->dev), skb, skb->dev, ICMPV6_POLICY_FAIL);
		break;
#endif
		default:
			net_info_ratelimited("case %u not handled yet\n", reject->with);
		break;
	}

	return EBT_DROP;
}

static int ebt_reject_tg_check(const struct xt_tgchk_param *par)

{
#if defined(CONFIG_BCM_KF_IP) && defined(CONFIG_IPV6)
	const struct ebt_entry *e = par->entryinfo;
	
	if (e->ethproto != __constant_htons(ETH_P_IPV6) ||e->invflags & EBT_IPROTO)
		return -EINVAL;
				
	return 0;
#else
    return -EINVAL;
#endif
}

static struct xt_target ebt_reject_tg_reg = {
	.name       = "REJECT",
	.revision   = 0,
	.family     = NFPROTO_BRIDGE,
	.target     = ebt_reject_tg,
	.checkentry = ebt_reject_tg_check,
	.targetsize = XT_ALIGN(sizeof(struct ebt_reject_info)),
	.me         = THIS_MODULE,
};

static int __init ebt_reject_init(void)
{
	int ret;
	ret = xt_register_target(&ebt_reject_tg_reg);

	if(ret == 0)
		printk(KERN_INFO "ebt_qos_map registered\n");

	return ret;
}

static void __exit ebt_reject_fini(void)
{
	xt_unregister_target(&ebt_reject_tg_reg);
}

module_init(ebt_reject_init);
module_exit(ebt_reject_fini);
MODULE_LICENSE("GPL");
#endif

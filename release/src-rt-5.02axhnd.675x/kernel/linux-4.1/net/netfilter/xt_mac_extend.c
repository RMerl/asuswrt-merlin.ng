/* Kernel module to match MAC address parameters. */

/* (C) 1999-2001 Paul `Rusty' Russell
 * (C) 2002-2004 Netfilter Core Team <coreteam@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* extend from xt_mac.c for MAC address extend match operations,
 * i.e, MAC/mask. 
 * BRCM, Jan, 31. 2019.
 */


#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/etherdevice.h>

#include <linux/netfilter_ipv4.h>
#include <linux/netfilter_ipv6.h>
#include <linux/netfilter/xt_mac_extend.h>
#include <linux/netfilter/x_tables.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Netfilter Core Team <coreteam@netfilter.org>, Broadcom");
MODULE_DESCRIPTION("Xtables: MAC address extend match");
MODULE_ALIAS("ipt_mac_extend");
MODULE_ALIAS("ip6t_mac_extend");

static bool mac_mt_extend(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct xt_mac_info_extend *info = par->matchinfo;
	bool ret;
	unsigned char skb_mac[ETH_ALEN], xt_mac[ETH_ALEN];
	int i;

	if (skb->dev == NULL || skb->dev->type != ARPHRD_ETHER)
		return false;
	if (skb_mac_header(skb) < skb->head)
		return false;
	if (skb_mac_header(skb) + ETH_HLEN > skb->data)
		return false;

	for (i = 0; i < ETH_ALEN; i++)
	{
		skb_mac[i] = eth_hdr(skb)->h_source[i] & info->msk[i];
		xt_mac[i] = info->srcaddr[i] & info->msk[i];
	}
	ret  = ether_addr_equal(skb_mac, xt_mac);
	ret ^= info->invert;
	return ret;
}

static struct xt_match mac_mt_reg_extend __read_mostly = {
	.name      = "mac-extend",
	.revision  = 0,
	.family    = NFPROTO_UNSPEC,
	.match     = mac_mt_extend,
	.matchsize = sizeof(struct xt_mac_info_extend),
	.hooks     = (1 << NF_INET_PRE_ROUTING) | (1 << NF_INET_LOCAL_IN) |
	             (1 << NF_INET_FORWARD),
	.me        = THIS_MODULE,
};

static int __init mac_mt_init_extend(void)
{
	return xt_register_match(&mac_mt_reg_extend);
}

static void __exit mac_mt_exit_extend(void)
{
	xt_unregister_match(&mac_mt_reg_extend);
}

module_init(mac_mt_init_extend);
module_exit(mac_mt_exit_extend);

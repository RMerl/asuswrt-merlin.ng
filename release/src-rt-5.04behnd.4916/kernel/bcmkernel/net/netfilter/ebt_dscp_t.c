/*
*    Copyright (c) 2003-2023 Broadcom Corporation
*    All Rights Reserved
*
<:label-BRCM:2023:DUAL/GPL:standard

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
 *  ebt_dscp_t
 */

#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <net/checksum.h>
#include <linux/if_vlan.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_dscp_t.h>

#include <net/dsfield.h>

#define PPPOE_HLEN   6
#define PPP_TYPE_IPV4   0x0021  /* IPv4 in PPP */
#define PPP_TYPE_IPV6   0x0057  /* IPv6 in PPP */

static unsigned int ebt_dscp_tg(struct sk_buff *skb, const struct xt_action_param *par)   
{
	const  struct ebt_dscp_t_info *dscpinfo = par->targinfo;
	struct iphdr *iph = NULL;
	struct ipv6hdr *ipv6h = NULL;
	__u8 dscp;

	/* if VLAN frame, we need to point to correct network header */
	if (skb->protocol == __constant_htons(ETH_P_IP))
		iph = (struct iphdr *)skb_network_header(skb);
	else if ((skb)->protocol == __constant_htons(ETH_P_IPV6))
		ipv6h = (struct ipv6hdr *)skb_network_header(skb);
	else if (skb->protocol == __constant_htons(ETH_P_8021Q)) {
		if (*(unsigned short *)(skb_network_header(skb) + VLAN_HLEN - 2) == __constant_htons(ETH_P_IP))
			iph = (struct iphdr *)(skb_network_header(skb) + VLAN_HLEN);
		else if (*(unsigned short *)(skb_network_header(skb) + VLAN_HLEN - 2) == __constant_htons(ETH_P_IPV6))
			ipv6h = (struct ipv6hdr *)(skb_network_header(skb) + VLAN_HLEN);
	}
	else if (skb->protocol == __constant_htons(ETH_P_PPP_SES)) {
		if (*(unsigned short *)(skb_network_header(skb) + PPPOE_HLEN) == __constant_htons(PPP_TYPE_IPV4))
			iph = (struct iphdr *)(skb_network_header(skb) + PPPOE_HLEN + 2);
		else if (*(unsigned short *)(skb_network_header(skb) + PPPOE_HLEN) == __constant_htons(PPP_TYPE_IPV6))
			ipv6h = (struct ipv6hdr *)(skb_network_header(skb) + PPPOE_HLEN + 2);
	}
	/* if not IP header, do nothing. */
	if ((iph == NULL) && (ipv6h == NULL))
		return dscpinfo->target;

	/* DSCP value is the high 6 bits of the DSCP field */
	if ( iph != NULL ) //IPv4
	{
		dscp = ipv4_get_dsfield((struct iphdr *)(iph));
		if (dscp >> 2 != dscpinfo->dscp) {
			ipv4_change_dsfield((struct iphdr *)(iph), 0, (dscpinfo->dscp << 2) | (dscp & 0x3));
		} 
	}
	else //IPv6
	{
		dscp = ipv6_get_dsfield((struct ipv6hdr *)(ipv6h));
		if (dscp >> 2 != dscpinfo->dscp)
		{
			ipv6_change_dsfield((struct ipv6hdr *)(ipv6h), 0, (dscpinfo->dscp << 2) | (dscp & 0x3));
		} 
	}

	return dscpinfo->target;
}

static int ebt_dscp_tg_check(const struct xt_tgchk_param *par)
{
	const struct ebt_dscp_t_info *info = par->targinfo;

	if (BASE_CHAIN && info->target == EBT_RETURN)
		return -EINVAL;
	
	if (ebt_invalid_target(info->target))
		return -EINVAL;
	
	return 0;
}

static struct xt_target ebt_dscp_tg_reg = {
	.name       = EBT_DSCP_TARGET,
	.revision   = 0,
	.family     = NFPROTO_BRIDGE,
	.target     = ebt_dscp_tg,
	.checkentry = ebt_dscp_tg_check,
	.targetsize = XT_ALIGN(sizeof(struct ebt_dscp_t_info)),
	.me         = THIS_MODULE,
};

static int __init ebt_dscp_init(void)
{
	int ret;
	ret = xt_register_target(&ebt_dscp_tg_reg);
	if(ret == 0)
		printk(KERN_INFO "ebt_dscp registered\n");

	return ret;
}

static void __exit ebt_dscp_fini(void)
{
	xt_unregister_target(&ebt_dscp_tg_reg);
}

module_init(ebt_dscp_init);
module_exit(ebt_dscp_fini);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("EBT: DSCP value modification");

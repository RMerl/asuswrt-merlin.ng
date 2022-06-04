#if defined(CONFIG_BCM_KF_NETFILTER)
/*
*    Copyright (c) 2003-2018 Broadcom Corporation
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
 *  ebt_qos_map
 *
 */
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_qos_map.h>
#include <linux/module.h>
#include <linux/ip.h>
#if defined(CONFIG_BCM_KF_IP) && defined(CONFIG_IPV6)
#include <linux/ipv6.h>
#endif
#include <linux/skbuff.h>

static unsigned int ebt_qos_map_tg(struct sk_buff *skb, const struct xt_action_param *par)
{
	const struct ebt_qos_map_info *info = par->targinfo;

#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	blog_lock();
	if (skb->blog_p) {
		skb->blog_p->dscp2pbit = info->dscp2pbit ? 1:0;
		skb->blog_p->dscp2q = info->dscp2q ? 1:0;
	}
	blog_unlock();
#endif

    return EBT_CONTINUE;
}

static int ebt_qos_map_tg_check(const struct xt_tgchk_param *par)

{
	const struct ebt_entry *e = par->entryinfo;

#if defined(CONFIG_BCM_KF_IP) && defined(CONFIG_IPV6)
	if ((e->ethproto != __constant_htons(ETH_P_IPV6) && e->ethproto != __constant_htons(ETH_P_IP)) ||
#else   
	if (e->ethproto != __constant_htons(ETH_P_IP) ||
#endif      
	   e->invflags & EBT_IPROTO)
		return -EINVAL;
				
	return 0;
}

static struct xt_target ebt_qos_map_tg_reg = {
	.name       = "QOSMAP",
	.revision   = 0,
	.family     = NFPROTO_BRIDGE,
	.target     = ebt_qos_map_tg,
	.checkentry = ebt_qos_map_tg_check,
	.targetsize = XT_ALIGN(sizeof(struct ebt_qos_map_info)),
	.me         = THIS_MODULE,
};

static int __init ebt_qos_map_init(void)
{
	int ret;
	ret = xt_register_target(&ebt_qos_map_tg_reg);

	if(ret == 0)
		printk(KERN_INFO "ebt_qos_map registered\n");

	return ret;
}

static void __exit ebt_qos_map_fini(void)
{
	xt_unregister_target(&ebt_qos_map_tg_reg);
}

module_init(ebt_qos_map_init);
module_exit(ebt_qos_map_fini);
MODULE_LICENSE("GPL");
#endif

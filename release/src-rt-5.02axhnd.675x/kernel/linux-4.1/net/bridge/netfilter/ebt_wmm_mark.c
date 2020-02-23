#if defined(CONFIG_BCM_KF_NETFILTER)
/*
*    Copyright (c) 2003-2012 Broadcom Corporation
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
 *  ebt_wmm_mark
 *
 */
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_wmm_mark_t.h>
#include <linux/if_vlan.h>
#include <linux/module.h>
#include <linux/ip.h>
#if defined(CONFIG_BCM_KF_IP) && defined(CONFIG_IPV6)
#include <linux/ipv6.h>
#endif
#include <linux/skbuff.h>

static unsigned int ebt_wmm_mark_tg(struct sk_buff *skb, const struct xt_action_param *par)
   
{
	const struct ebt_wmm_mark_t_info *info = par->targinfo;

//	struct iphdr *iph;
//	struct vlan_hdr *frame;	
	unsigned char prio = 0;
//	unsigned short TCI;

	if (info->markset != WMM_MARK_VALUE_NONE) {
		/* use marset regardless of supported classification method */
		prio = (unsigned char)info->markset;
		
#if 0 /* TOS/DSCP priority update will be handled in wlan driver (bcmutils.c, pktsetprio()) */
      if (skb->protocol == __constant_htons(ETH_P_8021Q)) {

         unsigned short pbits = (unsigned short)(info->markset & 0x0000f000);

         if (pbits) {
            frame = (struct vlan_hdr *)(skb->network_header);
            TCI = ntohs(frame->h_vlan_TCI);
		      TCI = (TCI & 0x1fff) | (((pbits >> 12) - 1) << 13);
            frame->h_vlan_TCI = htons(TCI);
         }
      }
	} else if (info->mark & WMM_MARK_8021D) {
		if (skb->protocol == __constant_htons(ETH_P_8021Q)) {
			frame = (struct vlan_hdr *)(skb->network_header);
			TCI = ntohs(frame->h_vlan_TCI);
			prio = (unsigned char)((TCI >> 13) & 0x7);
        	} else
			return EBT_CONTINUE;        	
        					
	} else if (info->mark & WMM_MARK_DSCP) {
		
		/* if VLAN frame, we need to point to correct network header */
		if (skb->protocol == __constant_htons(ETH_P_8021Q))
        		iph = (struct iphdr *)(skb->network_header + VLAN_HLEN);
        	/* ip */
#if defined(CONFIG_BCM_KF_IP) && defined(CONFIG_IPV6)
        	else if (skb->protocol == __constant_htons(ETH_P_IP)||skb->protocol == __constant_htons(ETH_P_IPV6))
#else         
        	else if (skb->protocol == __constant_htons(ETH_P_IP))
#endif
			iph = (struct iphdr *)(skb->network_header);
		else
		/* pass for others */
			return EBT_CONTINUE;

#if defined(CONFIG_BCM_KF_IP) && defined(CONFIG_IPV6)
		if(skb->protocol == __constant_htons(ETH_P_IPV6)) 
			prio=((struct ipv6hdr *)iph)->priority>>1;			
		else
#endif

		prio = iph->tos>>WMM_DSCP_MASK_SHIFT ;
		
#endif /* if 0 */

	}
		
    //printk("markset 0x%08x, mark 0x%x, mark 0x%x \n", info->markset, info->mark, (*pskb)->mark);
	if(prio) {
		skb->mark &= ~(PRIO_LOC_NFMASK << info->markpos);		
		skb->mark |= (prio << info->markpos);
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// member below is removed
//		(*pskb)->nfcache |= NFC_ALTERED;
		//printk("mark 0x%x, mark 0x%x\n",( prio << info->markpos), (*pskb)->mark);			
	}
		
	return info->target;
}

static int ebt_wmm_mark_tg_check(const struct xt_tgchk_param *par)

{
	const struct ebt_wmm_mark_t_info *info = par->targinfo;
	const struct ebt_entry *e = par->entryinfo;

	//if (datalen != EBT_ALIGN(sizeof(struct ebt_wmm_mark_t_info)))
	//	return -EINVAL;
	
	//printk("e->ethproto=0x%x, e->invflags=0x%x\n",e->ethproto, e->invflags);

#if defined(CONFIG_BCM_KF_IP) && defined(CONFIG_IPV6)
	if ((e->ethproto != __constant_htons(ETH_P_IPV6) && e->ethproto != __constant_htons(ETH_P_IP) && e->ethproto != __constant_htons(ETH_P_8021Q)) ||
#else   
	if ((e->ethproto != __constant_htons(ETH_P_IP) && e->ethproto != __constant_htons(ETH_P_8021Q)) ||
#endif      
	   e->invflags & EBT_IPROTO)
		return -EINVAL;
				
	if (BASE_CHAIN && info->target == EBT_RETURN)
		return -EINVAL;
		
	//CLEAR_BASE_CHAIN_BIT;
	
	if (INVALID_TARGET)
		return -EINVAL;
	
	return 0;
	
}

static struct xt_target ebt_wmm_mark_tg_reg = {
	.name		= EBT_WMM_MARK_TARGET,
	.revision	= 0,
	.family		= NFPROTO_BRIDGE,
	.target		= ebt_wmm_mark_tg,
	.checkentry	= ebt_wmm_mark_tg_check,
	.targetsize	= XT_ALIGN(sizeof(struct ebt_wmm_mark_t_info)),
	.me		= THIS_MODULE,
};

static int __init ebt_wmm_mark_init(void)
{
	int ret;
	ret = xt_register_target(&ebt_wmm_mark_tg_reg);

	if(ret == 0)
		printk(KERN_INFO "ebt_wmm_mark registered\n");

	return ret;
}

static void __exit ebt_wmm_mark_fini(void)
{
	xt_unregister_target(&ebt_wmm_mark_tg_reg);
}

module_init(ebt_wmm_mark_init);
module_exit(ebt_wmm_mark_fini);
MODULE_LICENSE("GPL");
#endif

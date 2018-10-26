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
 *  ebt_skbvlan_m
 *
 */

/* This match moudle is 90% the same with ebt_vlan.c but compare to broadcom defined vlan_header fields */

#include <linux/if_ether.h>
#include <linux/if_vlan.h> 
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_skbvlan_m.h>

#include "bcm_vlan_defs.h"

#define MODULE_VERS "0.1"

#define GET_SKBVLAN_ID(VTAG) (VTAG & VLAN_VID_MASK)
#define GET_SKBVLAN_PRIO(VTAG) ((VTAG & VLAN_PRIO_MASK) >> VLAN_PRIO_SHIFT)

#define GET_BITMASK(_BIT_MASK_) info->bitmask & _BIT_MASK_
#define EXIT_ON_MISMATCH(_MATCH_,_MASK_) {if (!((info->_MATCH_ == _MATCH_)^!!(info->invflags & _MASK_))) return false; }

static bool
ebt_skbvlan_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
#if defined(CONFIG_BCM_KF_VLAN) && (defined(CONFIG_BCM_VLAN) || defined(CONFIG_BCM_VLAN_MODULE))
    const struct ebt_skbvlan_m_info *info = par->matchinfo;
    unsigned short id;    
    unsigned char  prio;  
    __be16         encap; 

    if (skb == NULL || skb->vlan_count == 0)
    {
        return false;
    }

    id = GET_SKBVLAN_ID(skb->vlan_header[0] >> 16);
    prio = GET_SKBVLAN_PRIO(skb->vlan_header[0] >> 16);
    encap = (skb->vlan_header[0]); 

	/* Checking VLAN Identifier (VID) */
	if (GET_BITMASK(EBT_SKBVLAN_ID))
		EXIT_ON_MISMATCH(id, EBT_SKBVLAN_ID);

	/* Checking user_priority */
	if (GET_BITMASK(EBT_SKBVLAN_PRIO))
		EXIT_ON_MISMATCH(prio, EBT_SKBVLAN_PRIO);

	/* Checking Encapsulated Proto (Length/Type) field */
	if (GET_BITMASK(EBT_SKBVLAN_ENCAP))
		EXIT_ON_MISMATCH(encap, EBT_SKBVLAN_ENCAP);

	return true;
#else
   return false;
#endif
}

static int ebt_skbvlan_mt_check(const struct xt_mtchk_param *par)
{
    const struct ebt_skbvlan_m_info *info = par->matchinfo;

	/* Check for bitmask range
	 * True if even one bit is out of mask */
	if (info->bitmask & ~EBT_SKBVLAN_MASK) {
		pr_debug("bitmask %2X is out of mask (%2X)\n",
			 info->bitmask, EBT_SKBVLAN_MASK);
		return -EINVAL;
	}

	/* Check for inversion flags range */
	if (info->invflags & ~EBT_SKBVLAN_MASK) {
		pr_debug("inversion flags %2X is out of mask (%2X)\n",
			 info->invflags, EBT_SKBVLAN_MASK);
		return -EINVAL;
	}

	if (GET_BITMASK(EBT_SKBVLAN_ID)) {
		if (info->id) { /* if id!=0 => check vid range */
			if (info->id > VLAN_N_VID) {
				pr_debug("id %d is out of range (1-4096)\n",
					 info->id);
				return -EINVAL;
			}
		}
	}

	if (GET_BITMASK(EBT_SKBVLAN_PRIO)) {
		if ((unsigned char) info->prio > 7) {
			pr_debug("prio %d is out of range (0-7)\n",
				 info->prio);
			return -EINVAL;
		}
	}
	/* Check for encapsulated proto range - it is possible to be
	 * any value for u_short range.
	 * if_ether.h:  ETH_ZLEN        60   -  Min. octets in frame sans FCS */
	if (GET_BITMASK(EBT_SKBVLAN_ENCAP)) {
		if ((unsigned short) ntohs(info->encap) < ETH_ZLEN) {
			pr_debug("encap frame length %d is less than "
				 "minimal\n", ntohs(info->encap));
			return -EINVAL;
		}
	}

	return 0;
}

static struct xt_match ebt_skbvlan_mt_reg __read_mostly = {
    .name       = "skbvlan",
    .revision   = 0,
    .family     = NFPROTO_BRIDGE,
    .match      = ebt_skbvlan_mt,
    .checkentry = ebt_skbvlan_mt_check,
    .matchsize  = sizeof(struct ebt_skbvlan_m_info),
    .me         = THIS_MODULE,
};

static int __init ebt_skbvlan_m_init(void)
{
	pr_debug("ebtables 802.1Q extension module for LANVLAN" MODULE_VERS "\n");
	return xt_register_match(&ebt_skbvlan_mt_reg);
}

static void __exit ebt_skbvlan_m_fini(void)
{
	xt_unregister_match(&ebt_skbvlan_mt_reg);
}

module_init(ebt_skbvlan_m_init);
module_exit(ebt_skbvlan_m_fini);
MODULE_DESCRIPTION("Ebtables: Packet skbvlan match");
MODULE_LICENSE("GPL");
#endif

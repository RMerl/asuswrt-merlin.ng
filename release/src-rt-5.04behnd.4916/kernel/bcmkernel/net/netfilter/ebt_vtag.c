/*
 *
 *  Copyright (c) 2020  Broadcom
 *  All Rights Reserved
 *
<:label-BRCM:2020:DUAL/GPL:standard

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
 *  ebt_vtag
 *
 */
#include <linux/module.h>
#include <linux/netfilter/x_tables.h>
#include <linux/netfilter_bridge/ebtables.h>
#include <linux/netfilter_bridge/ebt_vtag_t.h>
#include <linux/skbuff.h>

static unsigned int
ebt_vtag_tg(struct sk_buff *skb, const struct xt_action_param *par)  
{
	const struct ebt_vtag_t_info *info = par->targinfo;
	int vtag = info->vtag;

	/* if the 8021p priority field (bits 0-3) of vtag is not zero, we need
	 * to do p-bit marking.
	 */
	if (vtag & 0xf) {
		unsigned short TCI = 0;

		/* if this is a vlan frame, we want to re-mark its p-bit with the 8021p
		 * priority in vtag.
		 * if this is not a vlan frame, we want to add a 8021p tag to it, with
		 * vid=0 and p-bit=the 8021p priority in vtag.
		 */
		if (skb_vlan_tag_present(skb)) {
			TCI = skb->vlan_tci;

			/* Since the 8021p priority value in vtag had been incremented by 1,
			 * we need to minus 1 from it to get the exact value.
			 */
			TCI = (TCI & 0x1fff) | (((vtag & 0xf) - 1) << 13);

			skb->vlan_tci = TCI;
		}
		else {
			if ((skb_mac_header(skb) - skb->head) < VLAN_HLEN) {
				printk("ebt_mark_tg: No headroom for VLAN tag. Marking is not done.\n");
			}
			else {
				struct vlan_ethhdr *ethHeader;

				skb->protocol = __constant_htons(ETH_P_8021Q);
				skb->mac_header -= VLAN_HLEN;
				skb->network_header -= VLAN_HLEN;
				skb->data -= VLAN_HLEN;
				skb->len  += VLAN_HLEN;

				/* Move the mac addresses to the beginning of the new header. */
				memmove(skb_mac_header(skb), skb_mac_header(skb) + VLAN_HLEN, 2 * ETH_ALEN);

				ethHeader = (struct vlan_ethhdr *)(skb_mac_header(skb));

				ethHeader->h_vlan_proto = __constant_htons(ETH_P_8021Q);

				/* Since the 8021p priority value in vtag had been incremented by 1,
				 * we need to minus 1 from it to get the exact value.
				 */
				TCI = (TCI & 0x1fff) | (((vtag & 0xf) - 1) << 13);

				ethHeader->h_vlan_TCI = htons(TCI);
			}
		}
	}
	return info->target;
}

static int ebt_vtag_tg_check(const struct xt_tgchk_param *par)
{
	const struct ebt_vtag_t_info *info = par->targinfo;
	int tmp = info->target;

	if (BASE_CHAIN && tmp == EBT_RETURN)
		return -EINVAL;

	if (tmp < -NUM_STANDARD_TARGETS || tmp >= 0)
		return -EINVAL;

	if (ebt_invalid_target(info->target))
		return -EINVAL;

	return 0;
}

static struct xt_target ebt_vtag_tg_reg = {
	.name		= "vtag",
	.revision	= 0,
	.family		= NFPROTO_BRIDGE,
	.target		= ebt_vtag_tg,
	.checkentry	= ebt_vtag_tg_check,
	.targetsize	= XT_ALIGN(sizeof(struct ebt_vtag_t_info)),
	.me		= THIS_MODULE,
};

static int __init ebt_vtag_init(void)
{
	int ret;
	ret = xt_register_target(&ebt_vtag_tg_reg);

	if(ret == 0)
		printk(KERN_INFO "ebt_vtag registered\n");

	return ret;
}

static void __exit ebt_vtag_fini(void)
{
	xt_unregister_target(&ebt_vtag_tg_reg);
}

module_init(ebt_vtag_init);
module_exit(ebt_vtag_fini);
MODULE_LICENSE("GPL");

/***********************************************************************
 *
 *  Copyright(c) 2020 Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2020:DUAL/GPL:standard
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
:>
 *
 ************************************************************************/
#ifdef CONFIG_NETFILTER_FAMILY_BRIDGE

#include <linux/br_fp.h>
#include "bcm_br_hooks_local_switching.h"
#include "br_private.h"

unsigned int bcm_br_local_switching_should_deliver(struct sk_buff *skb, const struct nf_hook_state *state)
{
    struct net_device *br_dev = BR_INPUT_SKB_CB(skb)->brdev;

    if (bridge_local_switching_disable_get(br_dev) && !is_netdev_wan(state->in) && !is_netdev_wan(state->out))
        return NF_DROP;

    return NF_ACCEPT;
}
EXPORT_SYMBOL(bcm_br_local_switching_should_deliver);

#endif /* CONFIG_NETFILTER_FAMILY_BRIDGE */

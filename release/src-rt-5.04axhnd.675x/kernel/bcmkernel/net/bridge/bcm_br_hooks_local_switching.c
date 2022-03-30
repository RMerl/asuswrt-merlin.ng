/***********************************************************************
 *
 *  Copyright(c) 2020 Broadcom Corporation
 *  All Rights Reserved
 *
 * <:label-BRCM:2020:DUAL/GPL:standard
 *
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 *
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 *
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
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

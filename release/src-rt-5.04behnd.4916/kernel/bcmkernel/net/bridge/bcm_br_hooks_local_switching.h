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

#ifndef _BR_BCM_HOOKS_LOCAL_SWITCHING_H
#define _BR_BCM_HOOKS_LOCAL_SWITCHING_H

#ifdef CONFIG_NETFILTER_FAMILY_BRIDGE

#include <linux/netfilter.h>
#include <linux/skbuff.h>

unsigned int bcm_br_local_switching_should_deliver(struct sk_buff *skb, const struct nf_hook_state *state);

#endif /* CONFIG_NETFILTER_FAMILY_BRIDGE */

#endif /* _BR_BCM_HOOKS_LOCAL_SWITCHING_H */

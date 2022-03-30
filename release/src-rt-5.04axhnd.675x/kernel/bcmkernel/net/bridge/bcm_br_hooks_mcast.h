/*
*    Copyright (c) 2003-2019 Broadcom
*    All Rights Reserved
*
<:label-BRCM:2019:DUAL/GPL:standard

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

#ifndef _BR_BCM_HOOKS_MCAST_H
#define _BR_BCM_HOOKS_MCAST_H

#if (defined(CONFIG_BCM_MCAST) || defined(CONFIG_BCM_MCAST_MODULE))

#include <linux/netfilter.h>
#include <linux/skbuff.h>

unsigned int mcast_receive(struct sk_buff *skb);
unsigned int mcast_should_deliver(struct sk_buff *skb, const struct nf_hook_state *state);

#else /* !(CONFIG_BCM_MCAST || CONFIG_BCM_MCAST_MODULE) */

#define mcast_receive(_arg1) NF_ACCEPT
#define mcast_should_deliver(_arg1, _arg2) NF_ACCEPT

#endif /* CONFIG_BCM_MCAST || CONFIG_BCM_MCAST_MODULE */
#endif /* _BR_BCM_HOOKS_MCAST_H */

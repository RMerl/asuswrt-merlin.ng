/*
*    Copyright (c) 2003-2019 Broadcom
*    All Rights Reserved
*
<:label-BRCM:2019:DUAL/GPL:standard

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

#ifndef _BR_BCM_HOOKS_MCAST_H
#define _BR_BCM_HOOKS_MCAST_H

#if (defined(CONFIG_BCM_MCAST) || defined(CONFIG_BCM_MCAST_MODULE))

#include <linux/netfilter.h>
#include <linux/skbuff.h>
#include "br_private.h"

unsigned int mcast_receive(struct sk_buff *skb);
unsigned int mcast_should_deliver(const struct sk_buff *skb, const struct net_bridge_port *p);

#else /* !(CONFIG_BCM_MCAST || CONFIG_BCM_MCAST_MODULE) */

#define mcast_receive(_arg1) NF_ACCEPT
#define mcast_should_deliver(_arg1, _arg2) NF_ACCEPT

#endif /* CONFIG_BCM_MCAST || CONFIG_BCM_MCAST_MODULE */
#endif /* _BR_BCM_HOOKS_MCAST_H */

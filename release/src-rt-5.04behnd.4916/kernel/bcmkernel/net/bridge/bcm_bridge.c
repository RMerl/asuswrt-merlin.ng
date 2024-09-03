/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
   All Rights Reserved

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

#include <br_private.h>
#include "bcm_br_hooks_mcast.h"

#if (defined(CONFIG_BCM_WLAN) || defined(CONFIG_BCM_WLAN_MODULE))
//  ETHER_TYPE_BRCM 0x886c, ETHER_TYPE_802_1X 0x888e, ETHER_TYPE_802_1X_PREAUTH 0x88c7
#define WL_AUTH_PROTOCOL(proto) ((proto)==htons(0x886c)||(proto)==htons(0x888e)||(proto)==htons(0x88c7))
#else
#define WL_AUTH_PROTOCOL(proto) (0)
#endif

int bcm_br_hook_handle_frame_finish(struct sk_buff *skb, int state)
{
    return ((state != BR_STATE_FORWARDING) && WL_AUTH_PROTOCOL(skb->protocol));
}

int bcm_br_hook_should_deliver(const struct sk_buff *skb, const struct net_bridge_port *p)
{
    if (WL_AUTH_PROTOCOL(skb->protocol))
    {
        return 1;
    }

    if (p->state == BR_STATE_FORWARDING)
    {
        return mcast_should_deliver(skb, p);
    }
    return 0;
}

int bcm_br_hook_br_flood(struct sk_buff *skb, struct net_bridge *br)
{
#if defined(CONFIG_BCM_KF_BLOG) && defined(CONFIG_BLOG)
	Blog_t *blog_p = blog_ptr(skb);

	if (blog_p)
   	{
        if (BLOG_RX_UCAST(blog_p))
	   	{
#if defined(CONFIG_BCM_UNKNOWN_UCAST)
            if (blog_support_unknown_ucast_g)
		   	{
                blog_p->rx.unknown_ucast = 1;
                blog_p->br_dev_p = (void *)br;
            }
            else
#endif
                blog_skip(skb, blog_skip_reason_br_flood);
        }
    }
#endif

    skbuff_bcm_ext_br_flood_set(skb, 1);

    return 0;
}

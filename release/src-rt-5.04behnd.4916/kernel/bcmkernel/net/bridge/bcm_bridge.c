/*
<:copyright-BRCM:2022:DUAL/GPL:standard

   Copyright (c) 2022 Broadcom 
   All Rights Reserved

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
    return 0;
}

/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard

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

#include <linux/netdevice.h>
#include <linux/skbuff.h>

#include <wl_linux.h>
#include <wl_pktc.h>

int32 wl_rxchainhandler(wl_info_t * wl, struct sk_buff * skb)
{
	/* dummy */
	return BCME_ERROR; /* skip pkt chainning, send to upper layer */
}

wl_pktc_tbl_t *wl_pktc_attach(struct wl_info * wl, struct wl_if * wlif)
{
	/* dummy */
	return NULL;
}

void wl_pktc_detach(struct wl_info *wl)
{
	/* dummy */
}

int wl_pktc_init(wl_if_t * wlif, struct net_device *dev)
{
	/* dummy */
	return 0;
}

void wl_pktc_free(wl_if_t * wlif)
{
	/* dummy */
}

unsigned long
wl_pktc_req(int req_id, unsigned long param0, unsigned long param1, unsigned long param2)
{
	/* dummy */
	return 0;
}

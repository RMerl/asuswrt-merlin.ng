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

#ifndef _wl_wfd_h_
#define _wl_wfd_h_

#include <linux/netdevice.h>
#include <linux/skbuff.h>

#include <wfd_dev.h>
#include <wlan_shared_defs.h>

struct wl_info;
struct wl_if;

#if !defined(BCM_PKTFWD)
extern int wl_wfd_bind(struct net_device *net, unsigned int unit);
extern void wl_wfd_unbind(int wfd_idx);
#else  /* BCM_PKTFWD */
extern int wl_wfd_bind(struct wl_info * wl);
extern void wl_wfd_unbind(struct wl_info * wl);
#endif /* BCM_PKTFWD */

extern int wl_wfd_registerdevice(int wfd_idx, struct net_device *dev);
extern int wl_wfd_unregisterdevice(int wfd_idx, struct net_device *dev);

extern int (*send_nbuff_to_wfd)(pNBuff_t pNBuff, struct net_device * dev);
extern int (*send_packet_to_upper_layer)(struct sk_buff * skb);
extern int inject_to_fastpath;
extern void (*wfd_path_delay_calc)(struct sk_buff *head, int len, int type);

#endif /* _wl_wfd_h_ */

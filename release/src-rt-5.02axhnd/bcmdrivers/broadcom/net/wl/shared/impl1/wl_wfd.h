/*
    Copyright (c) 2017 Broadcom
    All Rights Reserved

    <:label-BRCM:2017:DUAL/GPL:standard
    
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

#ifndef _wl_wfd_h_
#define _wl_wfd_h_

#include <linux/netdevice.h>
#include <linux/skbuff.h>

#include <wfd_dev.h>
#include <wlan_shared_defs.h>

extern spinlock_t pktctbl_lock;
extern int wl_start_int(wl_info_t *wl, wl_if_t *wlif, struct sk_buff *skb);

extern int wl_wfd_bind(struct net_device *net, unsigned int unit);
extern void wl_wfd_unbind(int wfd_idx);

extern int wl_wfd_registerdevice(int wfd_idx, struct net_device *dev);
extern int wl_wfd_unregisterdevice(int wfd_idx, struct net_device *dev);

#endif /* _wl_wfd_h_ */

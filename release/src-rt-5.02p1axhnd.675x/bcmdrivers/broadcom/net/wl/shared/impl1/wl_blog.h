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

#ifndef _wl_blog_h_
#define _wl_blog_h_

#ifdef mips
#undef ABS
#endif

#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/nbuff.h>

struct wl_info;
struct wl_if;
struct wlc_event;

#define PRIO_LOC_NFMARK 16

extern struct sk_buff *wl_xlate_to_skb(struct wl_info *wl, struct sk_buff *s);
extern int wl_handle_blog_emit(struct wl_info *wl, struct wl_if *wlif, struct sk_buff *skb,
	struct net_device *dev);
extern int wl_handle_blog_sinit(struct wl_info *wl, struct sk_buff *skb);

extern void wl_handle_blog_event(struct wl_info *wl, struct wlc_event *e);
#endif /* _wl_blog_h_ */

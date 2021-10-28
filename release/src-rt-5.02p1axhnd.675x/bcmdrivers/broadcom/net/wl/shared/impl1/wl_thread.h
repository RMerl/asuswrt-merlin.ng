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

#ifndef _wl_thread_h_
#define _wl_thread_h_

#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>

#if (defined(BCM_WFD) || defined(CONFIG_BCM963381)) && defined(WL_ALL_PASSIVE) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0))
#define WL_USE_L34_THREAD
#endif

struct wl_info;
struct wl_task;

extern int wl_thread_attach(struct wl_info *wl);
extern void wl_thread_detach(struct wl_info *wl);
extern void wl_thread_schedule_work(struct wl_info *wl);

#endif /* _wl_thread_h_ */

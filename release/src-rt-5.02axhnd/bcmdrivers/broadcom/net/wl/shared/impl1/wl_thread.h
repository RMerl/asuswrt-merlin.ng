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

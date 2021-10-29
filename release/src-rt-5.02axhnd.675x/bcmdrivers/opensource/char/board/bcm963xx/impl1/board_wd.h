/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
* 
*    Copyright (c) 2016 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :> 
*/


#ifndef _BOARD_WD_H_
#define _BOARD_WD_H_

#include <linux/if.h>

/*watchdog timer callback*/
#if defined(CONFIG_BCM_WATCHDOG_TIMER)
typedef int (*cb_watchdog_t)(void *arg);
typedef struct _CB_WDOG__LIST
{
    struct list_head list;
    char name[IFNAMSIZ];
    cb_watchdog_t cb_wd_fn;
    void *context;
}CB_WDOG_LIST , *PCB_WDOG_LIST;

typedef struct watchdog_cfg {
    unsigned int enabled;       // enable watchdog
    unsigned int timer;         // unit is ns
    unsigned int suspend;       // watchdog function is suspended
    unsigned int userMode;      // enable user mode watchdog
    unsigned int userThreshold; // user mode watchdog threshold to reset cpe
    unsigned int userTimeout;   // user mode timeout
} watchdog_cfg;

extern volatile watchdog_cfg watchdog_data;

#endif

void start_watchdog(unsigned int timer, unsigned int reset);
void board_wd_init(void);

#if defined(CONFIG_BCM_WATCHDOG_TIMER)
int bcm_suspend_watchdog(void);
void bcm_resume_watchdog(void);
void bcm_set_watchdog(int enable, int timer, int mode, unsigned int threshold);
void bcm_reset_watchdog(void);
#endif

#if defined(CONFIG_BCM960333)
void disablePLCWatchdog(void);
#endif

#endif

/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
* 
*    Copyright (c) 2016 Broadcom 
*    All Rights Reserved
* 
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed
* to you under the terms of the GNU General Public License version 2
* (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
* with the following added to such license:
* 
*    As a special exception, the copyright holders of this software give
*    you permission to link this software with independent modules, and
*    to copy and distribute the resulting executable under terms of your
*    choice, provided that you also meet, for each linked independent
*    module, the terms and conditions of the license of that module.
*    An independent module is a module which is not derived from this
*    software.  The special exception does not apply to any modifications
*    of the software.
* 
* Not withstanding the above, under no circumstances may you combine
* this software in any way with any other Broadcom software provided
* under a license other than the GPL, without Broadcom's express prior
* written consent.
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

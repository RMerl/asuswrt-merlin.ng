/*
<:copyright-BRCM:2002:GPL/GPL:standard

   Copyright (c) 2002 Broadcom 
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
/***********************************************/
/*                                             */
/*   MODULE:  bcm_ext_timer.h                  */
/*   PURPOSE: Timer specific information.      */
/*                                             */
/***********************************************/
#ifndef _BCM_EXT_TIMER_H
#define _BCM_EXT_TIMER_H

#include <linux/types.h>
#include <bcm_map_part.h>

#ifdef __cplusplus
extern "C" {
#endif

/*Defines the numbers of the existing HW timres*/
typedef enum
{
    EXT_TIMER_INVALID = -1,
    EXT_TIMER_0       = 0,
    EXT_TIMER_1       = 1,
    EXT_TIMER_2       = 2,
#if !defined(CONFIG_BCM960333) && !defined(CONFIG_BCM963268) && !defined(CONFIG_BCM96838)
    EXT_TIMER_3       = 3,
#endif
    EXT_TIMER_NUM
}EXT_TIMER_NUMBER;

#if !defined(CONFIG_BCM960333) && !defined(CONFIG_BCM963268) && !defined(CONFIG_BCM96838)
#define EXT_TIMER_INT_MASK      0x0f
#define EXT_TIMER_COUNT         4
#else
#define EXT_TIMER_INT_MASK      0x07
#define EXT_TIMER_COUNT         3
#endif

#define EXT_TIMER_MODE_PERIODIC 0
#define EXT_TIMER_MODE_ONESHOT  1

#ifdef TIMER_64BIT
#define TIMER_FREQ_MHZ          200 /* MHz */
#define TIMER_CNT_MAX           ((0x1ULL << 62) - 1)
#else
#define TIMER_FREQ_MHZ          50 /* MHz */
#define TIMER_CNT_MAX           ((0x1 << 30) - 1)
#endif

/*****************************************************************************/
/*          Function Prototypes                                              */
/*****************************************************************************/
typedef void (*ExtTimerHandler)(unsigned long param);

int init_hw_timers(void);
int ext_timer_alloc(EXT_TIMER_NUMBER timer_num, unsigned long timer_period, ExtTimerHandler timer_callback, unsigned long param);
int ext_timer_alloc_only(EXT_TIMER_NUMBER timer_num, ExtTimerHandler timer_callback, unsigned long param);
int ext_timer_free(EXT_TIMER_NUMBER timer_num);
int ext_timer_stop(EXT_TIMER_NUMBER timer_num);
int ext_timer_start(EXT_TIMER_NUMBER timer_num);
int ext_timer_read_count(EXT_TIMER_NUMBER timer_num, uint64_t* count);
int ext_timer_set_count(EXT_TIMER_NUMBER timer_num, uint64_t count);
int ext_timer_set_period(EXT_TIMER_NUMBER timer_num, unsigned long timer_period);
int ext_timer_set_mode(EXT_TIMER_NUMBER timer_num, unsigned int mode);
int ext_timer_set_affinity(EXT_TIMER_NUMBER timer_num, unsigned int cpuId, int force);
#if defined(CONFIG_BCM_WATCHDOG_TIMER) && !defined(INTERRUPT_ID_WDTIMER)
int watchdog_callback_register(ExtTimerHandler callback);
#endif
#ifdef __cplusplus
}
#endif

#endif /* _BCM_EXT_TIMER_H */


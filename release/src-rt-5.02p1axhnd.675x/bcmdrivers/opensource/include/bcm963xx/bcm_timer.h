/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
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

/***************************************************************************/
/*                                                                         */
/* Broadcom Timer Driver                                                   */
/*                                                                         */
/***************************************************************************/

#ifndef __BCM_TIMER_H_INCLUDED__
#define __BCM_TIMER_H_INCLUDED__

#include <linux/brcm_dll.h>

#define BCM_TIMER_PERIOD_uSEC   50 // usec
#define BCM_TIMER_HZ            (1000000 / BCM_TIMER_PERIOD_uSEC)

#if defined(CONFIG_BCM963178)
#define BCM_TIMER_CPU_ID        2
#elif defined(CONFIG_BCM947622)
#define BCM_TIMER_CPU_ID        3
#else
#define BCM_TIMER_CPU_ID        0
#endif

typedef enum {
    BCM_TIMER_MODE_ONESHOT,
    BCM_TIMER_MODE_PERIODIC,
    BCM_TIMER_MODE_MAX
} bcm_timer_mode_t;

typedef void (* bcm_timer_handler_t)(void *arg_p);

typedef struct {
    Dll_t node;             /* used internally to maintain linked-list of timers */
    bcm_timer_mode_t mode;
    uint32_t interval;      /* periodic interval, in BCM Timer Jiffies */
    uint32_t expiration;    /* expiration time, in BCM Timer Jiffies */
    bcm_timer_handler_t handler;
    void *arg_p;
} bcm_timer_user_t;

int bcm_timer_construct(void);

int bcm_timer_init(bcm_timer_user_t *user_p, bcm_timer_mode_t mode, uint32_t interval,
                   bcm_timer_handler_t handler, void *arg_p);

int bcm_timer_add(bcm_timer_user_t *user_p, uint32_t expiration);

void bcm_timer_delete(bcm_timer_user_t *user_p);

uint32_t bcm_timer_jiffies(void);

#endif /* __BCM_TIMER_H_INCLUDED__ */

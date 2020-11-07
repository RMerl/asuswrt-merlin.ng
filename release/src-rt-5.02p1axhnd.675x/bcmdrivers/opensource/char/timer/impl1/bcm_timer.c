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

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/bcm_log.h>
#include "bcm_ext_timer.h"
#include "bcm_timer.h"

/*****************************************************************************
 * Private Functions
 *****************************************************************************/

//#define CC_BCM_TIMER_TEST

#define __print(fmt, arg...) printk(fmt, ##arg)

#define __error(fmt, arg...)                                            \
    __print("\n\tERROR [%s,%u]: " fmt "\n", __FUNCTION__, __LINE__, ##arg)

#define BCM_TIMER_JIFFIES_32(_jiffies64) ( (uint32_t)(_jiffies64) )

/* The following 4 macros are provided for comparing tick counts that correctly handle
   wraparound. The _unknown parameter is typically Jiffies,
   and the _known parameter is the value against which you want to compare */

/* if _unknown is after _known, true; otherwise false */
#define BCM_TIMER_IS_TIME_AFTER(_unknown, _known) ( (int32_t)(_known) - (int32_t)(_unknown) < 0 )

/* if _unknown is before _known, true; otherwise false */
#define BCM_TIMER_IS_TIME_BEFORE(_unknown, _known) BCM_TIMER_IS_TIME_AFTER(_known, _unknown)

/* if _unknown is after than or equal to _known, true; otherwise false */
#define BCM_TIMER_IS_TIME_AFTER_EQ(_unknown, _known) ( (int32_t)(_unknown) - (int32_t)(_known) >= 0 )

/* if _unknown is before than or equal to _known, true; otherwise false */
#define BCM_TIMER_IS_TIME_BEFORE_EQ(_unknown, _known) BCM_TIMER_IS_TIME_AFTER_EQ(_known, _unknown)

#define BCM_TIMER_USER_IN_LIST(_user_p)  ( dll_next_p((PDll_t)(_user_p)) )
#define BCM_TIMER_USER_RESET(_user_p)    ( dll_next_p((PDll_t)(_user_p)) = NULL )

static DEFINE_SPINLOCK(bcm_timer_lock_g);
#define BCM_TIMER_LOCK(flags) spin_lock_irqsave(&bcm_timer_lock_g, flags)
#define BCM_TIMER_UNLOCK(flags) spin_unlock_irqrestore(&bcm_timer_lock_g, flags)

typedef struct {
    volatile int64_t jiffies_64;
    EXT_TIMER_NUMBER ext_timer_number;
    Dll_t list;
} bcm_timer_t;

static bcm_timer_t bcm_timer_g;

#if defined(CC_BCM_TIMER_TEST)
static bcm_timer_user_t test_timer;

static void bcm_timer_test_handler(void *arg_p)
{
    static int count = 0;

    __print("\tTimer Test: %d\n", count++);

    if(count < 32)
    {
        bcm_timer_add(&test_timer, bcm_timer_jiffies() + BCM_TIMER_HZ);
        bcm_timer_add(&test_timer, bcm_timer_jiffies() + BCM_TIMER_HZ);
    }
    else
    {
        bcm_timer_delete(&test_timer);
    }
}

static void bcm_timer_test_run(void)
{
    bcm_timer_init(&test_timer, bcm_timer_test_handler, NULL);

    bcm_timer_add(&test_timer, bcm_timer_jiffies() + BCM_TIMER_HZ);
}
#endif

static inline void bcm_timer_list_check(void)
{
    if(dll_empty(&bcm_timer_g.list))
    {
        if(ext_timer_stop(bcm_timer_g.ext_timer_number))
        {
            __error("Could not ext_timer_stop");
        }
    }
}

static inline bcm_timer_user_t *bcm_timer_next_user(uint32_t jiffies_32)
{
    PDll_t timerNode, nextTimerNode = NULL;
    bcm_timer_user_t *user_p;
    unsigned long flags;

    BCM_TIMER_LOCK(flags);

    /* process timers list */
    for(timerNode = dll_head_p(&bcm_timer_g.list);
        !dll_end(&bcm_timer_g.list, timerNode);
        timerNode = nextTimerNode)
    {
        nextTimerNode = dll_next_p(timerNode);

        user_p = (bcm_timer_user_t *)timerNode;

        if(BCM_TIMER_IS_TIME_AFTER_EQ(jiffies_32, user_p->expiration))
        {
            if(BCM_TIMER_MODE_ONESHOT == user_p->mode)
            {
                /* remove timer from timers list */
                dll_delete(timerNode);

                BCM_TIMER_USER_RESET(user_p);
            }
            else
            {
                user_p->expiration = jiffies_32 + user_p->interval;
            }

            BCM_TIMER_UNLOCK(flags);

            return user_p;
        }
    }

    BCM_TIMER_UNLOCK(flags);

    return NULL;
}

static void ext_timer_handler(unsigned long param)
{
    bcm_timer_user_t *user_p;
    uint32_t jiffies_32;
    unsigned long flags;

    bcm_timer_g.jiffies_64++;

    jiffies_32 = BCM_TIMER_JIFFIES_32(bcm_timer_g.jiffies_64);

    while((user_p = bcm_timer_next_user(jiffies_32)))
    {
        /* Call timer handler */
        user_p->handler(user_p->arg_p);
    }

    BCM_TIMER_LOCK(flags);
    bcm_timer_list_check();
    BCM_TIMER_UNLOCK(flags);
}

/*****************************************************************************
 * User API
 *****************************************************************************/

int bcm_timer_init(bcm_timer_user_t *user_p, bcm_timer_mode_t mode, uint32_t interval,
                   bcm_timer_handler_t handler, void *arg_p)
{
    if(mode >= BCM_TIMER_MODE_MAX)
    {
        __error("Invalid Timer mode <%d>\n", mode);

        return -1;
    }

    if(BCM_TIMER_MODE_PERIODIC == mode && !interval)
    {
        __error("Unspecified Periodic Timer interval\n");

        return -1;
    }

    if(BCM_TIMER_MODE_ONESHOT == mode && interval)
    {
        __error("Oneshot Timer should have no interval\n");

        return -1;
    }

    if(handler == NULL)
    {
        __error("Must specify handler");

        return -1;
    }

    BCM_TIMER_USER_RESET(user_p);
    user_p->mode = mode;
    user_p->interval = interval;
    user_p->expiration = 0;
    user_p->handler = handler;
    user_p->arg_p = arg_p;

    return 0;
}

int bcm_timer_add(bcm_timer_user_t *user_p, uint32_t expiration)
{
    uint32_t jiffies_32;
    unsigned long flags;

    jiffies_32 = BCM_TIMER_JIFFIES_32(bcm_timer_g.jiffies_64);

    if(BCM_TIMER_IS_TIME_AFTER_EQ(jiffies_32, expiration))
    {
        __error("Timer expiration occurs in the past <%d> (jiffies_32 <%d>)\n",
                expiration, jiffies_32);

        return -1;
    }

    BCM_TIMER_LOCK(flags);

    if(!BCM_TIMER_USER_IN_LIST(user_p))
    {
        user_p->expiration = expiration;

        if(dll_empty(&bcm_timer_g.list))
        {
            if(ext_timer_start(bcm_timer_g.ext_timer_number))
            {
                __error("Could not ext_timer_start");
            }
        }

        dll_append(&bcm_timer_g.list, (PDll_t)user_p);
    }

    BCM_TIMER_UNLOCK(flags);

    return 0;
}

void bcm_timer_delete(bcm_timer_user_t *user_p)
{
    unsigned long flags;

    BCM_TIMER_LOCK(flags);

    if(BCM_TIMER_USER_IN_LIST(user_p))
    {
        dll_delete((PDll_t)user_p);

        BCM_TIMER_USER_RESET(user_p);

        bcm_timer_list_check();
    }

    BCM_TIMER_UNLOCK(flags);
}

uint32_t bcm_timer_jiffies(void)
{
    return BCM_TIMER_JIFFIES_32(bcm_timer_g.jiffies_64);
}

/*****************************************************************************
 * Module
 *****************************************************************************/

int bcm_timer_construct(void)
{
    int ret;

    memset(&bcm_timer_g, 0, sizeof(bcm_timer_t));

    dll_init(&bcm_timer_g.list);

    /* Allocate and configure the EXT timer */

    bcm_timer_g.ext_timer_number = ext_timer_alloc_only(-1, ext_timer_handler, 0);
    if(bcm_timer_g.ext_timer_number < 0)
    {
        __error("Could not ext_timer_alloc_only");

        return -1;
    }

    __print("Allocated EXT_TIMER number %u\n", bcm_timer_g.ext_timer_number);

    ret = ext_timer_set_period(bcm_timer_g.ext_timer_number, BCM_TIMER_PERIOD_uSEC);
    if(ret)
    {
        __error("Could not ext_timer_set_period: %u usec", BCM_TIMER_PERIOD_uSEC);

        return ret;
    }

    ret = ext_timer_set_mode(bcm_timer_g.ext_timer_number, EXT_TIMER_MODE_ONESHOT);
    if(ret)
    {
        __error("Could not ext_timer_set_mode: EXT_TIMER_MODE_ONESHOT");

        return ret;
    }

    ret = ext_timer_set_affinity(bcm_timer_g.ext_timer_number, BCM_TIMER_CPU_ID, false);
    if(ret)
    {
        __error("Could not ext_timer_set_affinity: cpuId %u", BCM_TIMER_CPU_ID);

        return ret;
    }

    __print("Broadcom Timer Initialized\n");

#if defined(CC_BCM_TIMER_TEST)
    bcm_timer_test_run();
#endif

    return 0;
}

EXPORT_SYMBOL(bcm_timer_init);
EXPORT_SYMBOL(bcm_timer_add);
EXPORT_SYMBOL(bcm_timer_delete);
EXPORT_SYMBOL(bcm_timer_jiffies);

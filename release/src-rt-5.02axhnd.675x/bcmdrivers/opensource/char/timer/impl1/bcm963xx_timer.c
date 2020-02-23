/*
* <:copyright-BRCM:2012:DUAL/GPL:standard
* 
*    Copyright (c) 2012 Broadcom 
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
/* Timer driver                                                            */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

#include <linux/module.h>
#include <linux/irqflags.h>
#include <linux/interrupt.h>
#include <linux/irqreturn.h>
#include <linux/spinlock.h>
#include <bcm_map_part.h>
#include <bcm_intr.h>
#include <bcm_ext_timer.h>
#if defined(CONFIG_BCM_TIMER)
#include <bcm_timer.h>
#endif

#define __error(fmt, arg...) printk("%s,%u: " fmt, __FUNCTION__, __LINE__, ##arg)

typedef struct
{
    int allocated; /* if non-zero this timer is in use */
#ifdef TIMER_64BIT
    volatile uint64_t *timer_ctrl_reg;
    volatile uint64_t *timer_cnt_reg;
#else
    volatile uint32_t *timer_ctrl_reg;
    volatile uint32_t *timer_cnt_reg;
#endif
    unsigned long callback_arg;
    ExtTimerHandler callback;
}TimerT ;

static TimerT   timers[EXT_TIMER_NUM];
static int      initialized = 0;

#if defined(CONFIG_BCM_WATCHDOG_TIMER) && !defined(INTERRUPT_ID_WDTIMER)
static ExtTimerHandler watchdog_callback = NULL;
#endif
static DEFINE_SPINLOCK(timer_spinlock);

irqreturn_t ext_timer_isr(int irq, void *param)
{
#if !defined(CONFIG_BCM960333) && !defined(CONFIG_BCM963268) && !defined(CONFIG_BCM96838)
    int index;
    
    index = irq - INTERRUPT_ID_TIMER0;
    if( index < EXT_TIMER_NUM ) {
        /* clear the interrupt */
        TIMER->TimerInts |= (TIMER0 << index);

        if (timers[index].allocated && timers[index].callback)
            timers[index].callback(timers[index].callback_arg);

#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
        BcmHalInterruptEnable(irq);
#endif
    }
#endif

#if defined(CONFIG_BCM96838)
    {
        uint32 int_status, i;

        /* retrieve timer int */
        int_status = TIMER->TimerInts;

        /* clear the interrupt */
        TIMER->TimerInts |= (int_status & EXT_TIMER_INT_MASK);
#if defined(CONFIG_BCM_WATCHDOG_TIMER) && !defined(INTERRUPT_ID_WDTIMER)
        TIMER->TimerInts |= (int_status & WATCHDOG);
#endif
        BcmHalInterruptEnable(INTERRUPT_ID_TIMER);

        /* get which timer produced the interrupt */
        for (i = 0; i < EXT_TIMER_NUM; i++) {
            if (int_status & (TIMER0EN << i)) {
                /* call the appropriate function */
                if (timers[i].allocated && timers[i].callback)
                    timers[i].callback(timers[i].callback_arg);
            }
        }

#if defined(CONFIG_BCM_WATCHDOG_TIMER) && !defined(INTERRUPT_ID_WDTIMER)
        /* check watchdog timer interrupt */
        if ((int_status & WATCHDOG) && watchdog_callback)
        {
            watchdog_callback(0);
        }
#endif
    }
#endif

    return IRQ_HANDLED;
}

static int map_hw_timer_interrupt(unsigned long interrupt_id)
{
    printk("%s,%u: interrupt_id %ld\n", __FUNCTION__, __LINE__, interrupt_id);

    if (BcmHalMapInterrupt((FN_HANDLER)ext_timer_isr, (void*)interrupt_id, interrupt_id))
    {
        __error("Could not BcmHalMapInterrupt: Interrupt ID %lu\n", interrupt_id);
        return -1;
    }
#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
    BcmHalInterruptEnable(interrupt_id);
#endif

    return 0;
}

/***************************************************************************/
/*     Initialize external timers mechanism                                */
/***************************************************************************/
int init_hw_timers(void)
{
    unsigned long flags;
    int ret;
    int i;

    if(initialized)
        return -1;

    spin_lock_irqsave(&timer_spinlock, flags); 

    for (i=0; i<EXT_TIMER_NUM; ++i)
    {
        timers[i].allocated = 0;
    }

    /* mask external timer interrupts */
    TIMER->TimerMask = 0;

    /* clear external timer interrupts */
    TIMER->TimerInts |= EXT_TIMER_INT_MASK;

    timers[0].timer_ctrl_reg = &(TIMER->TimerCtl0);
    timers[1].timer_ctrl_reg = &(TIMER->TimerCtl1);
    timers[2].timer_ctrl_reg = &(TIMER->TimerCtl2);
#if (EXT_TIMER_COUNT > 3)
    timers[3].timer_ctrl_reg = &(TIMER->TimerCtl3);
#endif

    timers[0].timer_cnt_reg = &(TIMER->TimerCnt0);
    timers[1].timer_cnt_reg = &(TIMER->TimerCnt1);
    timers[2].timer_cnt_reg = &(TIMER->TimerCnt2);
#if (EXT_TIMER_COUNT > 3)
    timers[3].timer_cnt_reg = &(TIMER->TimerCnt3);
#endif

#if defined(CONFIG_BCM96838)
    if((ret = map_hw_timer_interrupt(INTERRUPT_ID_TIMER)))
    {
        spin_unlock_irqrestore(&timer_spinlock, flags);
        return ret;
    }
#endif

#if !defined(CONFIG_BCM960333) && !defined(CONFIG_BCM963268) && !defined(CONFIG_BCM96838)
    if((ret = map_hw_timer_interrupt(INTERRUPT_ID_TIMER0)))
    {
        spin_unlock_irqrestore(&timer_spinlock, flags);
        return ret;
    }

    if((ret = map_hw_timer_interrupt(INTERRUPT_ID_TIMER1)))
    {
        spin_unlock_irqrestore(&timer_spinlock, flags);
        return ret;
    }

    if((ret = map_hw_timer_interrupt(INTERRUPT_ID_TIMER2)))
    {
        spin_unlock_irqrestore(&timer_spinlock, flags);
        return ret;
    }

    if((ret = map_hw_timer_interrupt(INTERRUPT_ID_TIMER3)))
    {
        spin_unlock_irqrestore(&timer_spinlock, flags);
        return ret;
    }
#endif

    initialized = 1;

    spin_unlock_irqrestore(&timer_spinlock, flags); 

#if defined(CONFIG_BCM_TIMER)
    bcm_timer_construct();
#endif

    return 0;
}
EXPORT_SYMBOL(init_hw_timers);


/*=========================================================================
                     HW TIMER INTERFACE
 ==========================================================================*/


/***************************************************************************/
/*    allocated and start new HW timer                                     */
/*    timer_period is in microseconds                                      */
/***************************************************************************/
int ext_timer_alloc(EXT_TIMER_NUMBER timer_num, unsigned long timer_period, ExtTimerHandler timer_callback, unsigned long param)
{
    unsigned long flags;
    uint64_t timer_count;

    if(timer_num >= EXT_TIMER_NUM)
    {
        return -1;
    }

    spin_lock_irqsave(&timer_spinlock, flags); 

    if(timer_num < 0)
    {
        for(timer_num = EXT_TIMER_NUM-1; timer_num >= 0 ; timer_num--)
        {
            if(!timers[timer_num].allocated)
              break;
        }
    }

    if( (timer_num < 0) || timers[timer_num].allocated)
    {
        spin_unlock_irqrestore(&timer_spinlock, flags); 
        return -1;
    }

    timers[timer_num].callback = timer_callback;
    timers[timer_num].callback_arg = param;

    timer_count = TIMER_FREQ_MHZ * timer_period - 1;
    if (timer_count > TIMER_CNT_MAX) {
        __error("timer count %lld(%ld) exceed max hw counter\n", timer_count, timer_period);
        return -1;
    }

#ifdef TIMER_64BIT
    *(timers[timer_num].timer_ctrl_reg) = (uint64_t)timer_count;
#else
    *(timers[timer_num].timer_ctrl_reg) = (uint32_t)timer_count;
#endif
    TIMER->TimerMask |= (TIMER0EN << timer_num);
    *(timers[timer_num].timer_ctrl_reg) |= TIMERENABLE ;
    timers[timer_num].allocated = 1;

    spin_unlock_irqrestore(&timer_spinlock, flags); 
    return timer_num;

}
EXPORT_SYMBOL(ext_timer_alloc);

/***************************************************************************/
/*    allocated but not start a new HW timer                               */
/***************************************************************************/
int ext_timer_alloc_only(EXT_TIMER_NUMBER timer_num, ExtTimerHandler timer_callback, unsigned long param)
{
    unsigned long flags;

    if (timer_num >= EXT_TIMER_NUM)
        return -1;

    spin_lock_irqsave(&timer_spinlock, flags); 

    if (timer_num < 0) {
        for (timer_num = EXT_TIMER_NUM-1; timer_num >= 0 ; timer_num--) {
            if (!timers[timer_num].allocated)
              break;
        }
    }

    if ((timer_num < 0) || timers[timer_num].allocated) {
        spin_unlock_irqrestore(&timer_spinlock, flags); 
        return -1;
    }

    timers[timer_num].callback = timer_callback;
    timers[timer_num].callback_arg = param;

    timers[timer_num].allocated = 1;

    spin_unlock_irqrestore(&timer_spinlock, flags); 
    return timer_num;

}
EXPORT_SYMBOL(ext_timer_alloc_only);


/***************************************************************************/
/*     Free previously allocated timer                                     */
/***************************************************************************/
int ext_timer_free(EXT_TIMER_NUMBER timer_num)
{
    unsigned long flags;

    if( (timer_num >= EXT_TIMER_NUM) || (timer_num < 0) )
        return -1;

    spin_lock_irqsave(&timer_spinlock, flags); 

    if ( !timers[timer_num].allocated )
    {
        spin_unlock_irqrestore(&timer_spinlock, flags); 
        return -1;
    }

    timers[timer_num].callback = NULL;
    /* disable the timer */
    *(timers[timer_num].timer_ctrl_reg) &= ~TIMERENABLE ;
    /* mask the interrupt */
    TIMER->TimerMask &= ~(TIMER0EN << timer_num);
    /* clear interrupt */
    TIMER->TimerInts |= (TIMER0 << timer_num);
    timers[timer_num].allocated = 0;

    spin_unlock_irqrestore(&timer_spinlock, flags); 

    return 0;
}
EXPORT_SYMBOL(ext_timer_free);

/***************************************************************************/
/*     Stop the timer                                                      */ 
/***************************************************************************/
int ext_timer_stop(EXT_TIMER_NUMBER timer_num)
{
    unsigned long flags;

    if( (timer_num >= EXT_TIMER_NUM) || (timer_num < 0) )
        return -1;

    spin_lock_irqsave(&timer_spinlock, flags); 

    if ( !timers[timer_num].allocated )
    {
        spin_unlock_irqrestore(&timer_spinlock, flags); 
        return -1;
    }

    /* disable the timer */
    *(timers[timer_num].timer_ctrl_reg) &= ~TIMERENABLE ;
    /* mask the interrupt */
    TIMER->TimerMask &= ~(TIMER0EN << timer_num);
    /* clear interrupt */
    TIMER->TimerInts |= (TIMER0 << timer_num);

    spin_unlock_irqrestore(&timer_spinlock, flags); 

    return 0;

}
EXPORT_SYMBOL(ext_timer_stop);


/***************************************************************************/
/*     Start timer                                                         */
/***************************************************************************/
int ext_timer_start(EXT_TIMER_NUMBER timer_num)
{
    unsigned long flags;

    if( (timer_num >= EXT_TIMER_NUM) || (timer_num < 0) )
        return -1;

    spin_lock_irqsave(&timer_spinlock, flags); 

    if ( !timers[timer_num].allocated )
    {
        spin_unlock_irqrestore(&timer_spinlock, flags); 
        return -1;
    }

    /* enable the timer */
    *(timers[timer_num].timer_ctrl_reg) |= TIMERENABLE;
    /* unmask the interrupt */
    if (timers[timer_num].callback != NULL)
        TIMER->TimerMask |= (TIMER0EN << timer_num);

    spin_unlock_irqrestore(&timer_spinlock, flags); 

    return 0;


}
EXPORT_SYMBOL(ext_timer_start);

/***************************************************************************/
/*     allows to access count register of the allocated timer              */
/***************************************************************************/
int  ext_timer_read_count(EXT_TIMER_NUMBER timer_num, uint64_t* count)
{
    if( (timer_num >= EXT_TIMER_NUM) || (timer_num < 0) )
        return -1;


    if (!timers[timer_num].allocated)
        return -1;

    *count = (uint64_t)(*(timers[timer_num].timer_cnt_reg) & TIMER_COUNT_MASK);

    return 0;
}
EXPORT_SYMBOL(ext_timer_read_count);

/***************************************************************************/
/*     set up the new timercount on the timer                              */ 
/***************************************************************************/
int ext_timer_set_count(EXT_TIMER_NUMBER timer_num, uint64_t count)
{
    unsigned long flags;

    if( (timer_num >= EXT_TIMER_NUM) || (timer_num < 0) )
        return -1;

    if (count > TIMER_CNT_MAX) {
        __error("timer count %lld exceed max hw counter\n", count);
        return -1;
    }

    spin_lock_irqsave(&timer_spinlock, flags); 

    if (!timers[timer_num].allocated) {
        spin_unlock_irqrestore(&timer_spinlock, flags); 
        return -1;
    }

    *(timers[timer_num].timer_ctrl_reg) &= ~(TIMER_COUNT_MASK);
#ifdef TIMER_64BIT
    *(timers[timer_num].timer_ctrl_reg) = (uint64_t)count;
#else
    *(timers[timer_num].timer_ctrl_reg) = (uint32_t)count;
#endif

    spin_unlock_irqrestore(&timer_spinlock, flags); 
    return 0;
}
EXPORT_SYMBOL(ext_timer_set_count);

/***************************************************************************/
/*     set up the new period on the timer                                  */ 
/***************************************************************************/
int ext_timer_set_period(EXT_TIMER_NUMBER timer_num, unsigned long timer_period)
{
    unsigned long flags;
    uint64_t timer_count;

    if( (timer_num >= EXT_TIMER_NUM) || (timer_num < 0) )
        return -1;

    timer_count = TIMER_FREQ_MHZ * timer_period - 1;
    if (timer_count > TIMER_CNT_MAX) {
        __error("timer count %lld(%ld) exceed max hw counter\n", timer_count, timer_period);
        return -1;
    }

    spin_lock_irqsave(&timer_spinlock, flags); 

    if (!timers[timer_num].allocated) {
        spin_unlock_irqrestore(&timer_spinlock, flags); 
        return -1;
    }

    *(timers[timer_num].timer_ctrl_reg) &= ~(TIMER_COUNT_MASK);
#ifdef TIMER_64BIT
    *(timers[timer_num].timer_ctrl_reg) = (uint64_t)timer_count;
#else
    *(timers[timer_num].timer_ctrl_reg) = (uint32_t)timer_count;
#endif

    spin_unlock_irqrestore(&timer_spinlock, flags); 
    return 0;
}
EXPORT_SYMBOL(ext_timer_set_period);

/***************************************************************************/
/*     set the timer mode to truely periodic or not                        */
/***************************************************************************/
int ext_timer_set_mode(EXT_TIMER_NUMBER timer_num, unsigned int mode)
{
    unsigned long flags;

    if( (timer_num >= EXT_TIMER_NUM) || (timer_num < 0) )
        return -1;

    spin_lock_irqsave(&timer_spinlock, flags); 

    if (!timers[timer_num].allocated) {
        spin_unlock_irqrestore(&timer_spinlock, flags); 
        return -1;
    }

    if (mode == EXT_TIMER_MODE_PERIODIC)
        *(timers[timer_num].timer_ctrl_reg) &= ~(RSTCNTCLR);
    else if (mode == EXT_TIMER_MODE_ONESHOT)
        *(timers[timer_num].timer_ctrl_reg) |= RSTCNTCLR;

    spin_unlock_irqrestore(&timer_spinlock, flags); 
    return 0;
}
EXPORT_SYMBOL(ext_timer_set_mode);

/***************************************************************************/
/*     set the timer interrupt CPU affinity                                */
/***************************************************************************/
int ext_timer_set_affinity(EXT_TIMER_NUMBER timer_num, unsigned int cpuId, int force)
{
#if !defined(CONFIG_BCM960333) && !defined(CONFIG_BCM963268) && !defined(CONFIG_BCM96838) && defined(CONFIG_SMP)
    unsigned long flags;
    const struct cpumask *mask;
    unsigned int irq;
    int ret;

    if( (timer_num >= EXT_TIMER_NUM) || (timer_num < 0) )
        return -1;

    spin_lock_irqsave(&timer_spinlock, flags); 

    if (!timers[timer_num].allocated) {
        spin_unlock_irqrestore(&timer_spinlock, flags);
        return -1;
    }

    if (cpuId < 0 || cpuId >= NR_CPUS) {
        spin_unlock_irqrestore(&timer_spinlock, flags);
        return -1;
    }

    irq = INTERRUPT_ID_TIMER0 + timer_num;

    mask = get_cpu_mask(cpuId);
    ret = irq_set_affinity_hint(irq, mask);
    /* force parameter allow the set irq affinity on the cpu that has not power up yet */
    if (!ret && force && !cpu_online(cpuId))
        ret = irq_force_affinity(irq, mask);

    spin_unlock_irqrestore(&timer_spinlock, flags); 

    return ret;
#else
    return 0;
#endif
}
EXPORT_SYMBOL(ext_timer_set_affinity);

#if defined(CONFIG_BCM_WATCHDOG_TIMER) && !defined(INTERRUPT_ID_WDTIMER)
/***************************************************************************/
/*     set watchdog interrupt handler                                      */
/***************************************************************************/
int watchdog_callback_register(ExtTimerHandler callback)
{
    unsigned long flags;

    spin_lock_irqsave(&timer_spinlock, flags); 
    watchdog_callback = callback;
    spin_unlock_irqrestore(&timer_spinlock, flags); 
    return 0;
}
EXPORT_SYMBOL(watchdog_callback_register);
#endif

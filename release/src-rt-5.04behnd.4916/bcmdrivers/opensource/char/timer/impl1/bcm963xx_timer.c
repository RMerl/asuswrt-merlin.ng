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
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/module.h>
#include <bcm_intr.h>
#include <bcm_ext_timer.h>
#if defined(CONFIG_BCM_TIMER)
#include <bcm_timer.h>
#endif

#if !defined(CONFIG_BCM963138) && !defined(CONFIG_BCM963148) && !defined(CONFIG_BCM94908)
#define TIMER_64BIT 1
#endif

#define __error(fmt, arg...) printk("%s,%u: " fmt, __FUNCTION__, __LINE__, ##arg)

void __iomem *timers_reg;

#ifdef TIMER_64BIT
typedef struct Timer {
    uint64_t        TimerCtl0;
    uint64_t        TimerCtl1;
    uint64_t        TimerCtl2;
    uint64_t        TimerCtl3;
#define TIMERENABLE     (1ULL << 63)
#define RSTCNTCLR       (1ULL << 62)
    uint64_t        TimerCnt0;
    uint64_t        TimerCnt1;
    uint64_t        TimerCnt2;
    uint64_t        TimerCnt3;
#define TIMER_COUNT_MASK   0x3FFFFFFFFFFFFFFFULL
    uint32_t        TimerMask;
#define TIMER0EN        0x01
#define TIMER1EN        0x02
#define TIMER2EN        0x04
#define TIMER3EN        0x08
    uint32_t        TimerInts;
#define TIMER0          0x01
#define TIMER1          0x02
#define TIMER2          0x04
#define TIMER3          0x08
} Timer;
#else
typedef struct Timer {
   uint32_t TimerCtl0; /* 0x00 */
   uint32_t TimerCtl1; /* 0x04 */
   uint32_t TimerCtl2; /* 0x08 */
   uint32_t TimerCtl3; /* 0x0c */
#define TIMERENABLE     (1 << 31)
#define RSTCNTCLR    (1 << 30)

   uint32_t TimerCnt0; /* 0x10 */
   uint32_t TimerCnt1; /* 0x14 */
   uint32_t TimerCnt2; /* 0x18 */
   uint32_t TimerCnt3; /* 0x1c */
#define TIMER_COUNT_MASK   0x3FFFFFFF

   uint32_t TimerMask; /* 0x20 */
#define TIMER0EN     (1 << 0)
#define TIMER1EN     (1 << 1)
#define TIMER2EN     (1 << 2)
#define TIMER3EN     (1 << 3)

   uint32_t TimerInts; /* 0x24 */
#define TIMER0       (1 << 0)
#define TIMER1       (1 << 1)
#define TIMER2       (1 << 2)
#define TIMER3       (1 << 3)
#define WATCHDOG     (1 << 4)
} Timer;
#endif

#define TIMER ((volatile Timer *) timers_reg)

#ifdef TIMER_64BIT
#define TIMER_FREQ_MHZ          200 /* MHz */
#define TIMER_CNT_MAX           ((0x1ULL << 62) - 1)
#else
#define TIMER_FREQ_MHZ          50 /* MHz */
#define TIMER_CNT_MAX           ((0x1 << 30) - 1)
#endif

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

int timer_base_irq, timer_max_irq;
static int initialized = 0;
static TimerT   timers[EXT_TIMER_NUM];
static DEFINE_SPINLOCK(timer_spinlock);

irqreturn_t ext_timer_isr(int irq, void *param)
{
    int index;
    
    index = irq - timer_base_irq;
    if( index < EXT_TIMER_NUM ) {
        /* clear the interrupt */
        TIMER->TimerInts |= (TIMER0 << index);

        if (timers[index].allocated && timers[index].callback)
            timers[index].callback(timers[index].callback_arg);
    }


    return IRQ_HANDLED;
}

static int map_hw_timer_interrupt(unsigned long interrupt_id)
{
    printk("%s,%u: interrupt_id %ld\n", __FUNCTION__, __LINE__, interrupt_id);

    if (BcmHalMapInterrupt((FN_HANDLER)ext_timer_isr, "brcm_timer", (void*)interrupt_id, interrupt_id))
    {
        __error("Could not BcmHalMapInterrupt: Interrupt ID %lu\n", interrupt_id);
        return -1;
    }

    return 0;
}

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
        spin_unlock_irqrestore(&timer_spinlock, flags); 
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
#if defined(CONFIG_SMP)
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

    if (cpuId >= NR_CPUS) {
        spin_unlock_irqrestore(&timer_spinlock, flags);
        return -1;
    }

    irq = timer_base_irq + timer_num;

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

#if defined(CONFIG_BCM96766)
int ext_timer_early_init(struct device_node *np)
{
    int ret;
    int i;
    void __iomem *reg_base;
    int res_irq;

    if(initialized)
        return -1;

    reg_base = of_iomap(np, 0);
    if (!reg_base) {
        pr_err("%s: Missing reg description in Device Tree\n",  __func__);
        return -ENXIO;
    }
    timers_reg = reg_base;

    /* mask external timer interrupts */
    TIMER->TimerMask = 0;

    /* clear external timer interrupts */
    TIMER->TimerInts |= EXT_TIMER_INT_MASK;

    timers[0].timer_ctrl_reg = &(TIMER->TimerCtl0);
    timers[1].timer_ctrl_reg = &(TIMER->TimerCtl1);
    timers[2].timer_ctrl_reg = &(TIMER->TimerCtl2);
    timers[3].timer_ctrl_reg = &(TIMER->TimerCtl3);

    timers[0].timer_cnt_reg = &(TIMER->TimerCnt0);
    timers[1].timer_cnt_reg = &(TIMER->TimerCnt1);
    timers[2].timer_cnt_reg = &(TIMER->TimerCnt2);
    timers[3].timer_cnt_reg = &(TIMER->TimerCnt3);

    for (i=0; i<EXT_TIMER_NUM; i++)
    {
        timers[i].allocated = 0;
        res_irq = irq_of_parse_and_map(np, i);
        if (!res_irq) {
            pr_err("%s: Missing interrupt description in Device Tree\n",  __func__);
            return -EINVAL;
        }

        if((ret = map_hw_timer_interrupt(res_irq)))
        {
            return ret;
        }
        if (i==0)
            timer_base_irq = res_irq;
    }

    timer_max_irq = res_irq;

#if defined(CONFIG_BCM_TIMER)
    bcm_timer_construct();
#endif
    initialized = 1;

    return 0;
}
#endif

static struct of_device_id const bcm_timer_match[] = {
    { .compatible = "brcm,bcm-timers" },
    {}
};

MODULE_DEVICE_TABLE(of, bcm_timer_match);

static int bcm_timer_probe(struct platform_device *pdev)
{
    int i, ret = 0;
    struct resource *reg_base, *res_irq;
    const struct of_device_id *match;

    if(initialized)
        return 0;

    match = of_match_device(bcm_timer_match, &pdev->dev);
    if (!match)
    {
        dev_err(&pdev->dev, "bcm_timers dev: Failed to match\n");
        return -ENODEV;
    }

    reg_base = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!reg_base)
    {
        dev_err(&pdev->dev, "Unable to get register resource.\n");
        return -ENODEV;
    }

    timers_reg = devm_ioremap_resource(&pdev->dev, reg_base);
    if (IS_ERR(timers_reg))
    {
        dev_err(&pdev->dev, "Failed to map the timers resource\n");
        ret = -ENXIO;
    }

    /* mask external timer interrupts */
    TIMER->TimerMask = 0;

    /* clear external timer interrupts */
    TIMER->TimerInts |= EXT_TIMER_INT_MASK;

    for (i=0; i<EXT_TIMER_NUM; i++)
    {
        timers[i].allocated = 0;

        res_irq = platform_get_resource(pdev, IORESOURCE_IRQ, i);
        if (!res_irq)
        {
            return -ENODEV;
        }
        if((ret = map_hw_timer_interrupt(res_irq->start)))
        {
            return ret;
        }
        if (i==0)
            timer_base_irq = res_irq->start;
    }

    timer_max_irq = res_irq->start;

    timers[0].timer_ctrl_reg = &(TIMER->TimerCtl0);
    timers[1].timer_ctrl_reg = &(TIMER->TimerCtl1);
    timers[2].timer_ctrl_reg = &(TIMER->TimerCtl2);
    timers[3].timer_ctrl_reg = &(TIMER->TimerCtl3);

    timers[0].timer_cnt_reg = &(TIMER->TimerCnt0);
    timers[1].timer_cnt_reg = &(TIMER->TimerCnt1);
    timers[2].timer_cnt_reg = &(TIMER->TimerCnt2);
    timers[3].timer_cnt_reg = &(TIMER->TimerCnt3);

#if defined(CONFIG_BCM_TIMER)
    bcm_timer_construct();
#endif

    initialized = 1;

    return ret;
}

static struct platform_driver bcm_timer_driver = {
    .driver = {
            .name = "brcm,bcm_timers",
            .of_match_table = bcm_timer_match,
    },
    .probe = bcm_timer_probe,
};

static int __init bcm_timers_init(void)
{
    return platform_driver_register(&bcm_timer_driver);
}

subsys_initcall(bcm_timers_init);


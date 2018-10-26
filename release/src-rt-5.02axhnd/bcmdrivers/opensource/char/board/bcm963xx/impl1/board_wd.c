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
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/reboot.h>
#include <linux/slab.h>

#include <bcmtypes.h>
#include <bcm_map_part.h>
#include <board.h>

#if defined(CONFIG_BCM_EXT_TIMER)
#include "bcm_ext_timer.h"
#endif

#include "board_wd.h"

#if defined(CONFIG_BCM_WATCHDOG_TIMER)
volatile watchdog_cfg watchdog_data = {0, 5000000, 0, 0, 8, 0};

static CB_WDOG_LIST *g_cb_wdog_list_head = NULL;
static DEFINE_SPINLOCK(watchdog_spinlock);
/* watchdog restart work */
static struct work_struct watchdogRestartWork;
static int watchdog_restart_in_progress = 0;

static void __init kerSysInitWatchdogCBList( void );
#if defined(INTERRUPT_ID_WDTIMER) || !defined(CONFIG_BCM_EXT_TIMER)
static irqreturn_t watchdog_isr(int irq, void *ignore);
#else
static void watchdog_isr(int param);
#endif

#endif

#if defined(CONFIG_BCM_WATCHDOG_TIMER)
int bcm_suspend_watchdog() 
{
    unsigned long flags;
    int needResume = 0;

    spin_lock_irqsave(&watchdog_spinlock, flags); 

    if (watchdog_data.enabled && !watchdog_data.suspend)
    {
#if defined (CONFIG_BCM96838)
        WDTIMER->WD0Ctl = 0xEE00;
        WDTIMER->WD0Ctl = 0x00EE;
#elif defined(CONFIG_BCM96858) || defined (CONFIG_BCM963158) ||  defined(CONFIG_BCM96846)
        WDTIMER0->WatchDogCtl = 0xEE00;
        WDTIMER0->WatchDogCtl = 0x00EE;
#else
        TIMER->WatchDogCtl = 0xEE00;
        TIMER->WatchDogCtl = 0x00EE;
#endif

        watchdog_data.suspend = 1;
        needResume = 1;
    } 

    spin_unlock_irqrestore(&watchdog_spinlock, flags); 
    return needResume;
}

void bcm_resume_watchdog() 
{
    unsigned long flags;

    start_watchdog(watchdog_data.timer, 0);

    spin_lock_irqsave(&watchdog_spinlock, flags); 
    watchdog_data.suspend = 0;
    spin_unlock_irqrestore(&watchdog_spinlock, flags); 
}

void bcm_set_watchdog(int enable, int timer, int mode, unsigned int threshold)
{
    unsigned long flags;

    spin_lock_irqsave(&watchdog_spinlock, flags); 
    
    watchdog_data.userMode = mode;
    watchdog_data.userThreshold = threshold * 2; // watchdog interrupt is half of timer
    watchdog_data.userTimeout = 0;             // reset userTimeout
    if (watchdog_data.enabled != enable) 
    { 
        watchdog_data.timer = timer;
        if (enable)
        {
#if defined(INTERRUPT_ID_WDTIMER)
            /*
             * On 63381, timer0-4 have dedicated interrupt IDs, INTERRUPT_ID_TIMER
             * does not be used for timers. It sets INTERRUPT_ID_TIMER as 
             * INTERRUPT_ID_WDTIMER, in case, 2nd parameters can not be 0.
             */
            BcmHalMapInterrupt((FN_HANDLER)watchdog_isr , (void*)INTERRUPT_ID_WDTIMER, INTERRUPT_ID_WDTIMER);
#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
            BcmHalInterruptEnable(INTERRUPT_ID_WDTIMER);
#endif /* !defined(CONFIG_ARM) */
#elif defined(CONFIG_BCM_EXT_TIMER)
            watchdog_callback_register(&watchdog_isr);
#else
            /* 
             *  The 2nd parameter must be unique to share same IRQ.
             *  We need to pass the same magic value when call free_irq().
             */
            BcmHalMapInterrupt((FN_HANDLER)watchdog_isr, 0xabcd1212, INTERRUPT_ID_TIMER);
            BcmHalInterruptEnable(INTERRUPT_ID_TIMER);
#endif
            watchdog_data.enabled = enable;
            watchdog_data.suspend = 0;
            spin_unlock_irqrestore(&watchdog_spinlock, flags); 
            start_watchdog(watchdog_data.timer, 0);
        }
        else
        {
            spin_unlock_irqrestore(&watchdog_spinlock, flags); 
            bcm_suspend_watchdog();
#if defined(INTERRUPT_ID_WDTIMER)
            free_irq(INTERRUPT_ID_WDTIMER, (void *)INTERRUPT_ID_WDTIMER);
#elif defined(CONFIG_BCM_EXT_TIMER)
            watchdog_callback_register(NULL);
#else
            free_irq(INTERRUPT_ID_TIMER, (void *)0xabcd1212);
#endif
            watchdog_data.enabled = enable;
        }
    }
    else if (watchdog_data.timer != timer)
    {
        watchdog_data.timer = timer;
        if (watchdog_data.enabled)
        {
            watchdog_data.suspend = 0;
            spin_unlock_irqrestore(&watchdog_spinlock, flags); 
            start_watchdog(watchdog_data.timer, 0);
        }    
    }
    else
        spin_unlock_irqrestore(&watchdog_spinlock, flags);

    return;
}

void bcm_reset_watchdog(void)
{
    unsigned long flags;

    spin_lock_irqsave(&watchdog_spinlock, flags); 
    if (watchdog_data.userMode)
        watchdog_data.userTimeout = 0;
    spin_unlock_irqrestore(&watchdog_spinlock, flags); 

    return;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
static void watchdog_restart_thread(struct work_struct *work)
#else
static void watchdog_restart_thread(void *arg)
#endif
{
    // kernel_restart is a high level, generic linux way of rebooting.
    // It calls a notifier list and lets sub-systems know that system is
    // rebooting, and then calls machine_restart, which eventually
    // calls kerSysMipsSoftReset.
    kernel_restart(NULL);

    return;
}

#if defined(INTERRUPT_ID_WDTIMER) || !defined(CONFIG_BCM_EXT_TIMER)
static irqreturn_t watchdog_isr(int irq, void *ignore)
#else
static void watchdog_isr(int param)
#endif
{
    unsigned long flags;
    struct list_head *pos;
    CB_WDOG_LIST *tmp = NULL;
    int reboot = 0;

#if !defined(INTERRUPT_ID_WDTIMER) && !defined(CONFIG_BCM_EXT_TIMER)
    /* 
     * if WD shares timer interrupt and EXT_TIMER is disabled, 
     * need to check if it is WD interrupt
     */ 
    if (!(TIMER->TimerInts & WATCHDOG))
        return IRQ_NONE;
#endif

    spin_lock_irqsave(&watchdog_spinlock, flags); 

#if defined(INTERRUPT_ID_WDTIMER) || !defined(CONFIG_BCM_EXT_TIMER)
    /* 
     * if WD shares timer interrupt and EXT_TIMER is enabled, 
     * interrupt be cleared in ext_timer_isr.
     */ 

    /* clear the interrupt */
    TIMER->TimerInts |= WATCHDOG;

#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
#if defined(INTERRUPT_ID_WDTIMER)
    BcmHalInterruptEnable(INTERRUPT_ID_WDTIMER);
#else
    BcmHalInterruptEnable(INTERRUPT_ID_TIMER);
#endif /* defined(INTERRUPT_ID_WDTIMER) */
#endif /* !defined(CONFIG_ARM) */
#endif /*  defined(INTERRUPT_ID_WDTIMER) || !defined(CONFIG_BCM_EXT_TIMER) */

#if defined (CONFIG_BCM96838)
    WDTIMER->WD0Ctl = 0xEE00;
    WDTIMER->WD0Ctl = 0x00EE;
    WDTIMER->WD0Ctl = 0xFF00;
    WDTIMER->WD0Ctl = 0x00FF;
#elif defined(CONFIG_BCM96858) || defined (CONFIG_BCM963158) || defined(CONFIG_BCM96846)
    WDTIMER0->WatchDogCtl = 0xEE00;
    WDTIMER0->WatchDogCtl = 0x00EE;
    WDTIMER0->WatchDogCtl = 0xFF00;
    WDTIMER0->WatchDogCtl = 0x00FF;
#else
    /* stop and reload timer counter then start WD */
    TIMER->WatchDogCtl = 0xEE00;
    TIMER->WatchDogCtl = 0x00EE;
    TIMER->WatchDogCtl = 0xFF00;
    TIMER->WatchDogCtl = 0x00FF;
#endif

    /* check watchdog callback function */
    list_for_each(pos, &g_cb_wdog_list_head->list) 
    {
        tmp = list_entry(pos, CB_WDOG_LIST, list);
        if ((tmp->cb_wd_fn)(tmp->context))
        {
            reboot = 1;
            printk("\nwatchdog cb of %s return 1, reset CPE!!!\n", tmp->name);
            break;
        }
    }

    if (!reboot && watchdog_data.userMode)
    {
        watchdog_data.userTimeout++;
        if (watchdog_data.userTimeout >= watchdog_data.userThreshold)
        {
            reboot = 1;
            printk("\nHit userMode watchdog threshold, reset CPE!!!\n");
        }
    }

    if (reboot)
    {
        spin_unlock_irqrestore(&watchdog_spinlock, flags); 
        bcm_suspend_watchdog();
        /* 
         *  If call kerSysMipsSoftReset() in interrupt,  
         *  kernel smp pops out warning.  
         */
        if( !watchdog_restart_in_progress )
        {
            INIT_WORK(&watchdogRestartWork, watchdog_restart_thread);
            schedule_work(&watchdogRestartWork);
            watchdog_restart_in_progress  = 1;
        }

#if defined(INTERRUPT_ID_WDTIMER) || !defined(CONFIG_BCM_EXT_TIMER)
        return IRQ_HANDLED;
#else
        return;
#endif
    }

    spin_unlock_irqrestore(&watchdog_spinlock, flags); 

#if defined(INTERRUPT_ID_WDTIMER) || !defined(CONFIG_BCM_EXT_TIMER)
    return IRQ_HANDLED;
#endif
}

static void __init kerSysInitWatchdogCBList( void )
{
    CB_WDOG_LIST *new_node;
    unsigned long flags;

    if( g_cb_wdog_list_head != NULL) 
    {
        printk("Error: kerSysInitWatchdogCBList: list head is not null\n");
        return;
    }
    new_node= (CB_WDOG_LIST *)kmalloc(sizeof(CB_WDOG_LIST), GFP_KERNEL);
    memset(new_node, 0x00, sizeof(CB_WDOG_LIST));
    INIT_LIST_HEAD(&new_node->list);
    spin_lock_irqsave(&watchdog_spinlock, flags); 
    g_cb_wdog_list_head = new_node;
    spin_unlock_irqrestore(&watchdog_spinlock, flags); 
} 

void kerSysRegisterWatchdogCB(char *devname, void *cbfn, void *context)
{
    CB_WDOG_LIST *new_node;
    unsigned long flags;

    // do all the stuff that can be done without the lock first
    if( devname == NULL || cbfn == NULL ) 
    {
        printk("Error: kerSysRegisterWatchdogCB: register info not enough (%s,%x,%x)\n", devname, (unsigned int)cbfn, (unsigned int)context);
        return;
    }

    if (strlen(devname) > (IFNAMSIZ - 1))
        printk("Warning: kerSysRegisterWatchdogCB: devname too long, will be truncated\n");

    new_node= (CB_WDOG_LIST *)kmalloc(sizeof(CB_WDOG_LIST), GFP_KERNEL);
    memset(new_node, 0x00, sizeof(CB_WDOG_LIST));
    INIT_LIST_HEAD(&new_node->list);
    strncpy(new_node->name, devname, IFNAMSIZ-1);
    new_node->cb_wd_fn = (cb_watchdog_t)cbfn;
    new_node->context = context;

    // OK, now acquire the lock and insert into list
    spin_lock_irqsave(&watchdog_spinlock, flags); 
    if( g_cb_wdog_list_head == NULL) 
    {
        printk("Error: kerSysRegisterWatchdogCB: list head is null\n");
        kfree(new_node);
    } 
    else 
    {
        list_add(&new_node->list, &g_cb_wdog_list_head->list);
        printk("watchdog: kerSysRegisterWatchdogCB: %s registered \n", devname);
    }
    spin_unlock_irqrestore(&watchdog_spinlock, flags); 

    return;
} 

void kerSysDeregisterWatchdogCB(char *devname)
{
    struct list_head *pos;
    CB_WDOG_LIST *tmp;
    int found=0;
    unsigned long flags;

    if(devname == NULL) {
        printk("Error: kerSysDeregisterWatchdogCB: devname is null\n");
        return;
    }

    printk("kerSysDeregisterWatchdogCB: %s is deregistering\n", devname);

    spin_lock_irqsave(&watchdog_spinlock, flags); 
    if(g_cb_wdog_list_head == NULL) 
    {
        printk("Error: kerSysDeregisterWatchdogCB: list head is null\n");
    } 
    else 
    {
        list_for_each(pos, &g_cb_wdog_list_head->list) 
        {
            tmp = list_entry(pos, CB_WDOG_LIST, list);
            if(!strcmp(tmp->name, devname)) 
            {
                list_del(pos);
                kfree(tmp);
                found = 1;
                printk("kerSysDeregisterWatchdogCB: %s is deregistered\n", devname);
                break;
            }
        }
        if (!found)
            printk("kerSysDeregisterWatchdogCB: %s not (de)registered\n", devname);
    }
    spin_unlock_irqrestore(&watchdog_spinlock, flags); 

    return;
} 

#endif

void start_watchdog(unsigned int timer, unsigned int reset) 
{
#if defined(CONFIG_BCM_WATCHDOG_TIMER)
    unsigned long flags;

    spin_lock_irqsave(&watchdog_spinlock, flags); 

    /* if watch dog is disabled and reset is 0, do nothing */
    if (!reset && !watchdog_data.enabled)
    {
        spin_unlock_irqrestore(&watchdog_spinlock, flags); 
        return;
    }
#endif /* defined(CONFIG_BCM_WATCHDOG_TIMER) */

#if defined(CONFIG_BCM947189)
#elif defined (CONFIG_BCM96838)
    WDTIMER->WD0Ctl = 0xEE00;
    WDTIMER->WD0Ctl = 0x00EE;
    WDTIMER->WD0DefCount = timer * (FPERIPH/1000000);
    WDTIMER->WD0Ctl = 0xFF00;
    WDTIMER->WD0Ctl = 0x00FF;
#elif defined(CONFIG_BCM96858) || defined (CONFIG_BCM963158) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856)
    WDTIMER0->WatchDogCtl = 0xEE00;
    WDTIMER0->WatchDogCtl = 0x00EE;
    WDTIMER0->WatchDogDefCount = timer * (FPERIPH/1000000);
    WDTIMER0->WatchDogCtl = 0xFF00;
    WDTIMER0->WatchDogCtl = 0x00FF;
#else
    TIMER->WatchDogCtl = 0xEE00;
    TIMER->WatchDogCtl = 0x00EE;
    TIMER->WatchDogDefCount = timer * (FPERIPH/1000000);
    TIMER->WatchDogCtl = 0xFF00;
    TIMER->WatchDogCtl = 0x00FF;
#endif

#if defined(CONFIG_BCM_WATCHDOG_TIMER)
    /* when reset is 1, disable interrupt */
    if (reset && watchdog_data.enabled)
    {
#if defined(INTERRUPT_ID_WDTIMER)
        BcmHalInterruptDisable(INTERRUPT_ID_WDTIMER);
#elif defined(CONFIG_BCM_EXT_TIMER)
        watchdog_callback_register(NULL);
#else
        BcmHalInterruptDisable(INTERRUPT_ID_TIMER);
#endif
    }
    else
    {
        /* reset userTimeout value */
        if (watchdog_data.userMode)
            watchdog_data.userTimeout = 0;
    }
    spin_unlock_irqrestore(&watchdog_spinlock, flags); 
#endif /* defined(CONFIG_BCM_WATCHDOG_TIMER)*/

}

#if defined(CONFIG_BCM960333)
void disablePLCWatchdog(void)
{
    unsigned int *pReg;
    pReg = (unsigned int *) ioremap_nocache(PLC_STATUS_ADDR, 4);
    if (pReg != NULL)
    {
       *pReg = PLC_STATUS_RUNNING_WDOG_DISABLED;
       iounmap(pReg);
    }
}
#endif /*CONFIG_BCM960333*/

void board_wd_init(void)
{
#if defined(CONFIG_BCM_WATCHDOG_TIMER)
    kerSysInitWatchdogCBList();
#endif
    return;
}


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

#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/reboot.h>
#include <linux/sched.h>
#include <linux/ctype.h>
#include <linux/netdevice.h>
#include <linux/if.h>
#include <bcmnetlink.h>
#include <net/sock.h>
#include <bcmnetlink.h>
#include <net/sock.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
#include <linux/sched/types.h>
#endif

#include <bcmtypes.h>
#include <bcm_map_part.h>
#include <board.h>
#include <boardparms.h>
#include <bcm_intr.h>
#include <shared_utils.h>
#include <bcm_pinmux.h>

#include <bcm_extirq.h>
#include "board_button.h"
#include "board_wl.h"

extern int restore_in_progress;

static void btnDoPress(BtnInfo *btn, unsigned long currentJiffies);
static void btnDoRelease(BtnInfo *btn, unsigned long currentJiffies);
static void btnDoHold(BtnInfo *btn, unsigned long currentJiffies);

//====================================================================================
// Default Button Callbacks: 
/* These are the commonly used callbacks.   It is possible to register other
   callbacks using the RegisterPushButtonPress/Hold/Release functions outside
   of boardparms */


/***************************************************************************/
// BP_BNT_CBACK_NONE
void btnHook_DoNothing(unsigned long timeInMs, void* param) {    
    // do nothing...
}


/***************************************************************************/
// BP_BTN_ACTION_RESTORE_DEFAULTS
void btnHook_RestoreToDefault(unsigned long timeInMs, void* param) {    
    if( !restore_in_progress )
    {
        char buf[256] = {};
        restore_in_progress  = 1;

        printk(" *** Restore to Factory Default Setting ***\n\n");
#if defined(WIRELESS) 
		board_util_wl_godefault();
#endif
        kerSysPersistentSet( buf, sizeof(buf), 0 );
#if defined(CONFIG_BCM_PLC_BOOT)
        kerSysFsFileSet("/data/plc/plc_pconfig_state", buf, 1);
#endif
        kernel_restart(NULL);        
    }    
}

/***************************************************************************/
// BP_BTN_ACTION_PRINT
void btnHook_Print(unsigned long timeInMs, void* param) {    
    printk("%s\n", (char *)param);
}


/***************************************************************************/
// BP_BTN_ACTION_SES  implementation in board_wl.c
/***************************************************************************/

/***************************************************************************/
// BP_BTN_ACTION_WLAN_DOWN
void btnHook_WlanDown(unsigned long timeInMs, void* param) {
#if defined(WIRELESS)
    struct net_device *wlan_dev;

    printk("Bringing down wireless interface wl0\n");
    wlan_dev = dev_get_by_name(&init_net, "wl0");
    if (wlan_dev)
    {
        netif_carrier_off(wlan_dev);
        rtnl_lock();
        dev_close(wlan_dev);
        rtnl_unlock();
        dev_put(wlan_dev);
    }
#endif
}

/***************************************************************************/
// BP_BTN_ACTION_PLC_UKE

static pushButtonNotifyHook_t gPlcUkeCallback = NULL;
void* gPlcUkeCallbackParam = NULL;
static pushButtonNotifyHook_t gPlcRandomizeCallback = NULL;
void* gPlcRandomizeCallbackParam = NULL;
static DEFINE_SPINLOCK(btn_spinlock);

void kerSysRegisterPlcUkeCallback( void (* callback)(unsigned long timeInMs, void* param), void* param ) {
    unsigned long flags;

    printk("Registering Plc Uke callback: (%pf)\n", callback);
    
    spin_lock_irqsave(&btn_spinlock, flags);
    gPlcUkeCallback = callback;
    gPlcUkeCallbackParam = param;
    spin_unlock_irqrestore(&btn_spinlock, flags);
}

void kerSysDeregisterPlcUkeCallback( void (* callback)(unsigned long timeInMs, void* param) ) {
    unsigned long flags;
    printk("Deregistering Plc Uke callback: (%pf)\n", callback);
    spin_lock_irqsave(&btn_spinlock, flags);
    if (gPlcUkeCallback == callback)
        gPlcUkeCallback = NULL;
    spin_unlock_irqrestore(&btn_spinlock, flags);
}

void kerSysRegisterPlcRandomizeCallback(void (* callback)(unsigned long timeInMs,
                                                          void* param),
                                        void* param) {
    unsigned long flags;

    printk("Registering PLC network key randomize callback: (%pf)\n", callback);

    spin_lock_irqsave(&btn_spinlock, flags);
    gPlcRandomizeCallback = callback;
    gPlcRandomizeCallbackParam = param;
    spin_unlock_irqrestore(&btn_spinlock, flags);
}

void kerSysDeregisterPlcRandomizeCallback(void (* callback)(unsigned long timeInMs,
                                                            void* param)) {
    unsigned long flags;
    printk("Deregistering PLC network key randomize callback: (%pf)\n", callback);
    spin_lock_irqsave(&btn_spinlock, flags);
    if (gPlcRandomizeCallback == callback)
        gPlcRandomizeCallback = NULL;
    spin_unlock_irqrestore(&btn_spinlock, flags);
}

void btnHook_PlcUke(unsigned long timeInMs, void* param) {
    unsigned long flags;
    pushButtonNotifyHook_t hook;
    void* hookParam;
    
    printk(" *** Doing PLC UKE (%pf) ***\n\n", gPlcUkeCallback);
    
    spin_lock_irqsave(&btn_spinlock, flags);
    hook=gPlcUkeCallback;
    hookParam=gPlcUkeCallbackParam;
    spin_unlock_irqrestore(&btn_spinlock, flags);
    if (hook)
        hook(timeInMs, hookParam);
}

/***************************************************************************/
// BP_BTN_ACTION_RANDOMIZE_PLC
void btnHook_RandomizePlc(unsigned long timeInMs, void* param) {
    unsigned long flags;
    pushButtonNotifyHook_t hook;
    void* hookParam;

    printk(" *** Randomizing PLC ***\n");

    spin_lock_irqsave(&btn_spinlock, flags);
    hook=gPlcRandomizeCallback;
    hookParam=gPlcRandomizeCallbackParam;
    spin_unlock_irqrestore(&btn_spinlock, flags);
    if (hook)
        hook(timeInMs, hookParam);
}


/***************************************************************************/
// BP_BTN_ACTION_RESET
void btnHook_Reset(unsigned long timeInMs, void* param) {
    printk(" *** Restarting System ***\n\n");
    kernel_restart(NULL);
}

pushButtonNotifyHook_t btnHooks[] = {
    [BP_BTN_ACTION_NONE >> BP_BTN_ACTION_SHIFT]             = (pushButtonNotifyHook_t)btnHook_DoNothing,
    [BP_BTN_ACTION_SES  >> BP_BTN_ACTION_SHIFT]             = (pushButtonNotifyHook_t)btnHook_Ses,  
    [BP_BTN_ACTION_RESTORE_DEFAULTS  >> BP_BTN_ACTION_SHIFT]= (pushButtonNotifyHook_t)btnHook_RestoreToDefault,  
    [BP_BTN_ACTION_RANDOMIZE_PLC>> BP_BTN_ACTION_SHIFT]     = (pushButtonNotifyHook_t)btnHook_RandomizePlc,
    [BP_BTN_ACTION_RESET>> BP_BTN_ACTION_SHIFT]             = (pushButtonNotifyHook_t)btnHook_Reset,
    [BP_BTN_ACTION_PRINT>> BP_BTN_ACTION_SHIFT]             = (pushButtonNotifyHook_t)btnHook_Print,
    [BP_BTN_ACTION_PLC_UKE>> BP_BTN_ACTION_SHIFT]           = (pushButtonNotifyHook_t)btnHook_PlcUke,
    [BP_BTN_ACTION_WLAN_DOWN>> BP_BTN_ACTION_SHIFT]         = (pushButtonNotifyHook_t)btnHook_WlanDown,
};


//====================================================================================
// Main Button Support: 

static BtnInfo btnInfo[PB_BUTTON_MAX] = {};

/***************************************************************************
 * Function Name: btnThread
 * Description  : This is the thread function that takes care of a button.
                  It is repsonsible for invoking any registered call backs,
                  and doing polling. 
                  Assume it will never exit...
 * Parameters   : arg: pointer to the button structure
 ***************************************************************************/
int btnThread(void * arg) {
    BtnInfo *btn = (BtnInfo*)arg;
    unsigned long flags;
    struct sched_param sp = { .sched_priority = 20 };

    // set to be realtime thread with somewhat high priority:
    sched_setscheduler(current, SCHED_FIFO, &sp);

    while(1) {     

        // at this point the button is not pressed -- wait for press:
        wait_event_interruptible(btn->waitq, (btn->events & BTN_EV_PRESSED) != 0);
        
        spin_lock_irqsave(&btn->lock, flags);        
        btn->events &= ~ BTN_EV_PRESSED;
        spin_unlock_irqrestore(&btn->lock, flags);
        btnDoPress(btn, btn->lastPressJiffies);

        // at this point the button is down -- wait for release or until next hold event:
        while(1) {
            // TBD: instead of waking up every 100ms, we can actually poll the pushButton
            // library to figure out when the next wakeup time should be.
            // Note: all times should be read by the ISR, as the thread can be delayed
            // in heavy traffic, giving inacurate results if read from here
            wait_event_interruptible_timeout(btn->waitq, btn->events != 0, msecs_to_jiffies(BTN_POLLFREQ));
            if (btn->events & BTN_EV_HOLD) {
                spin_lock_irqsave(&btn->lock, flags);        
                btn->events &= ~BTN_EV_HOLD;
                spin_unlock_irqrestore(&btn->lock, flags);
                btnDoHold(btn, btn->lastHoldJiffies);
            }
            if (btn->events & BTN_EV_RELEASED) {
                spin_lock_irqsave(&btn->lock, flags);        
                btn->events &= ~ (BTN_EV_RELEASED | BTN_EV_HOLD);
                spin_unlock_irqrestore(&btn->lock, flags);
                btnDoRelease(btn, btn->lastReleaseJiffies);
                break;
            }
        }
    }
}


/***************************************************************************
 * Function Name: btnPressIsr
 * Description  : This is the default btnPress interrupt handler.  It
                  assumes the button drives a gpio and is mapped to an
                  external interrupt.
                  This invokes the doPushButtonPress, and starts the
                  polling timer.
 * Parameters   : irq: the irq number of the button
                  info: a pointer to a BtnInfo structure
 * Returns      : IRQ_HANDLED.
 ***************************************************************************/
static irqreturn_t btnPressIsr(int irq, void *info) {
    BtnInfo *btn = (BtnInfo*)info;
    unsigned long currentJiffies = jiffies;
    int wasTimerActive;
    unsigned long flags;
    
    if (IsExtIntrShared(extIntrInfo[btn->extIrqIdx]) && !btn->isDown(btn)) {
        return IRQ_HANDLED;
    }
    
    spin_lock_irqsave(&btn->lock, flags);
    btn->active=1;
    btn->lastPressJiffies=currentJiffies;
    if ( btn->releaseIsr == NULL && btn->poll != NULL){
        wasTimerActive = mod_timer(&btn->timer, (currentJiffies + msecs_to_jiffies(BTN_POLLFREQ)));
    }
    btn->events |= BTN_EV_PRESSED;
    btn->disableIrqs(btn);
    wake_up(&btn->waitq);
    spin_unlock_irqrestore(&btn->lock, flags);
    return IRQ_HANDLED;
}

/***************************************************************************
 * Function Name: btnReleaseIsr
 * Description  : This is the default hw btn release interrupt handler.  It
                  assumes the button drives a gpio and is mapped to an
                  external interrupt.  It is only called if a button is edge
                  detected on both (up and down) edges.
                  This stops the polling timer, and invokes the doRelease
                  callback. 
 * Parameters   : irq: the irq number of the button
                  info: a pointer to a BtnInfo structure
 * Returns      : IRQ_HANDLED.
 ***************************************************************************/
static irqreturn_t btnReleaseIsr(int irq, void *info) {

    BtnInfo *btn = (BtnInfo*)info;
    unsigned long flags;
    
    if (IsExtIntrShared(extIntrInfo[btn->extIrqIdx]) && !btn->isDown(btn))
        return IRQ_HANDLED;
    
    spin_lock_irqsave(&btn->lock, flags);
    btn->active=0;
    btn->lastReleaseJiffies = jiffies;
    del_timer(&btn->timer);
    btn->events |= BTN_EV_RELEASED;
    wake_up(&btn->waitq);
    spin_unlock_irqrestore(&btn->lock, flags);
    return IRQ_HANDLED;
}


/***************************************************************************
 * Function Name: btnIsGpioBtnDown
 * Description  : This a the check to see if a gpio-based button is down
                  based on the gpio level
 * Parameters   : arg: a pointer to a BtnInfo structure
 * Returns      : 1 if the button is down
 ***************************************************************************/
static bool btnIsGpioBtnDown(BtnInfo *btn) {
    // check hardware to see if button is actually down:
    int value;
#if !defined(CONFIG_BCM960333)
    value = kerSysGetGpioValue(btn->gpio);  //TBD -- not supported for 60333 yet
#elif defined(CONFIG_BCM947189)
   val=-1;
#else
    value = GPIO->GPIOData & (1 << btn->gpio);
#endif
    if( !value == !btn->gpioActiveHigh ) {        
        return TRUE;    
    } else {
        return FALSE;
    }
}

static void __btnPoll(BtnInfo *btn)
{
    unsigned long currentJiffies = jiffies;
    int wasTimerActive;
    unsigned long flags;

    spin_lock_irqsave(&btn->lock, flags);
    if (btn->active) {
        if ( btn->isDown(btn) ) {
            btn->lastHoldJiffies = currentJiffies;
            wasTimerActive=mod_timer(&btn->timer, currentJiffies + msecs_to_jiffies(BTN_POLLFREQ)); 
            btn->events |= BTN_EV_HOLD;
            wake_up(&btn->waitq);
        }
        else if (btn->releaseIsr == NULL) {
            btn->lastReleaseJiffies = currentJiffies;
            btn->active = 0;
            del_timer(&btn->timer);
            btn->events |= BTN_EV_RELEASED;
            wake_up(&btn->waitq);
        } 
        else {
            // hit race condition.  releaseIsr is pending (and it will 
            // stop the timer, etc.   Do nothing here)
        }
    } 
    else {
        // we should not get here
    }
    
    spin_unlock_irqrestore(&btn->lock, flags);
}

/***************************************************************************
 * Function Name: btnPoll
 * Description  : This is the polling function.  It is started when a 
                  button press is detected, and stopped when a button 
                  release is detected.
 * Parameters   : arg: a pointer to a BtnInfo structure
 ***************************************************************************/

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
static void btnPoll(unsigned long arg) {
    BtnInfo *btn = (BtnInfo *)arg;

    __btnPoll(btn);
}
#else
static void btnPoll(struct timer_list * arg) {

    BtnInfo *btn = from_timer(btn, arg, timer);
    __btnPoll(btn);
}
#endif

/***************************************************************************
 * Function Name: btnDoPress
 * Description  : This is called when a press has been detected.
 * Parameters   : arg: a pointer to a BtnInfo structure
 ***************************************************************************/
static void btnDoPress (BtnInfo *btn, unsigned long currentJiffies) {
    // this is called from the kernel thread context.
    if (btn->releaseIsr) {
        // note: releaseIsr implies edge detect
            btn->enableIrqs(btn);
    } else {
        btn->disableIrqs(btn);
    }
    doPushButtonPress(btn->btnId, currentJiffies);
    return;
}

/***************************************************************************
 * Function Name: btnDoRelease
 * Description  : This is called when a release has been detected
 * Parameters   : arg: a pointer to a BtnInfo structure
 ***************************************************************************/
static void btnDoRelease (BtnInfo *btn, unsigned long currentJiffies) {
    // this is called from the kernel thread context.
    doPushButtonRelease(btn->btnId, currentJiffies);    
    if (btn->pressIsr) {
            btn->enableIrqs(btn);
    }
    return;
}


/***************************************************************************
 * Function Name: btnDoHold
 * Description  : This is called when a button hold is detected
 * Parameters   : arg: a pointer to a BtnInfo structure
 ***************************************************************************/
static void btnDoHold(BtnInfo *btn, unsigned long currentJiffies) {
    // this is called from the kernel thread context.
    doPushButtonHold(btn->btnId, currentJiffies);
}


/***************************************************************************
 * Function Name: btnEnableIrq
 * Description  : enable a buttons Irqs
 ***************************************************************************/
static void btnEnableIrq(BtnInfo *btn) {
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
    BcmHalExternalIrqUnmask(btn->extIrqMap);
#elif defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858) || \
      defined(CONFIG_BCM963158) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) || \
      defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) || defined(CONFIG_BCM96878)
    BcmHalExternalIrqClear(btn->extIrqMap);
    BcmHalExternalIrqUnmask(btn->extIrqMap);
#elif defined(CONFIG_BCM947189)
#else
    BcmHalInterruptEnable(btn->extIrqMap);
#endif
}

/***************************************************************************
 * Function Name: btnDisableIrq
 * Description  : enable a buttons Irqs
 ***************************************************************************/
static void btnDisableIrq(BtnInfo *btn) {
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
    BcmHalExternalIrqMask(btn->extIrqMap);
#elif defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858) || \
      defined(CONFIG_BCM963158) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) || \
      defined(CONFIG_BCM963178) || defined(CONFIG_BCM947622) || defined(CONFIG_BCM96878)
    BcmHalExternalIrqMask(btn->extIrqMap);
    BcmHalExternalIrqClear(btn->extIrqMap);
#elif defined(CONFIG_BCM947189)
#else
    BcmHalInterruptDisable(btn->extIrqMap);
#endif
}

/***************************************************************************
 * Function Name: registerBtns
 * Description  : This parses the board parms and sets up all buttons
                  accordingly.
 ***************************************************************************/

/* Parse board parms and read button Id's. */
/* For now, assume gpio button with ext interrupt.  This can be modified
   going forward however, by changing the callbacks, etc
*/
int registerBtns(void)
{
    void *          iter=NULL;                   // iterator
    int             ret;
    unsigned short  bpBtnIdx, bpGpio, bpExtIrq;  // values read from boardparms
    unsigned short  btnIdx;                      // actual button index
    unsigned short  gpioNum;                     // actual gpio number
    unsigned short  extIrqIdx;                      // actual ext irq
    unsigned short  isGpioActiveHigh;            // iff true gpio signal is high when button is pressed
    unsigned short  hasDownIsr;                  // This is true if isr fires on both up and down 
    BtnInfo *       btn;
    unsigned long   flags;
    unsigned short  bpNumHooks = MAX_BTN_HOOKS_PER_BTN;
    unsigned short  bpHooks[MAX_BTN_HOOKS_PER_BTN];
    void*           bpHookParms[MAX_BTN_HOOKS_PER_BTN];
    
    while(1) {
        bpNumHooks = MAX_BTN_HOOKS_PER_BTN;
        ret=BpGetButtonInfo(&iter, &bpBtnIdx, &bpGpio, &bpExtIrq, &bpNumHooks, bpHooks, bpHookParms);
        if (ret != BP_SUCCESS) {
            break;
        }
        if (bpBtnIdx >= PB_BUTTON_MAX) {
            printk("registerBtns: Button index %d out of range (max %d)\n", bpBtnIdx, PB_BUTTON_MAX);
            return -1;
        }
        
        if (bpGpio == BP_NOT_DEFINED) {
            printk("ERROR: registerBtns: GPIO not set for button %d (not handled yet)\n", bpBtnIdx);
            return -1;
        }
        
        if (bpExtIrq == BP_EXT_INTR_NONE) {
            //eventually this will be done via polling.
            printk("ERROR: registerBtns: ExtIrq not set for button %d (not handled yet)\n", bpBtnIdx);
            return -1;
        }

        btnIdx = bpBtnIdx;        
        hasDownIsr = 0;    // not supported as of yet
        gpioNum = bpGpio & BP_GPIO_NUM_MASK;
        isGpioActiveHigh = ((bpGpio & BP_ACTIVE_MASK) == BP_ACTIVE_HIGH);
        extIrqIdx = (bpExtIrq & ~BP_EXT_INTR_FLAGS_MASK) - BP_EXT_INTR_0;

        if (isGpioActiveHigh != IsExtIntrTypeActHigh(bpExtIrq)) {
            printk("registerBtns: Error -- mismatch on activehigh/low for button %d\n", bpBtnIdx);
            return -1;
        }

        if (IsExtIntrShared(extIntrInfo[extIrqIdx])) {
            printk("Error -- shared button (%d) interrupts not handled yet...\n", bpBtnIdx);
            return -1;
        }
        if (IsExtIntrConflict(extIntrInfo[extIrqIdx])) {
            printk("Error -- Btn conflicting interrupts not handled yet...\n");
            return -1;
        }


        btn = &btnInfo[btnIdx];
        
        printk("Registering button %d (%p) (bpGpio: %08x, bpExtIrq:%08x (%x))\n", 
            bpBtnIdx, btn, bpGpio, bpExtIrq, extIntrInfo[extIrqIdx]);

        printk("    extIrqIdx:%d, gpioNum:%d %s\n", 
                extIrqIdx, gpioNum, isGpioActiveHigh?"ACTIVE HIGH":"ACTIVE LOW");

        if (btn->isConfigured != 0) {
            printk("Warning -- button %d defined twice in boardparms \n", btn->btnId);
            // overriding old definition...
            memset(btn, 0, sizeof(*btn));
        }
        btn->isConfigured = 1;
        
        spin_lock_init(&btn->lock);
        
        btn->btnId = PB_BUTTON_0 + btnIdx;
        btn->gpio = gpioNum;
        btn->extIrqIdx = extIrqIdx;
        btn->active = FALSE;
        btn->gpioActiveHigh = isGpioActiveHigh;
        btn->events = 0;
        init_waitqueue_head(&btn->waitq);
        
        btn->thread = kthread_run(btnThread, (void *)btn, "btnhandler%d", btnIdx);
        if (!btn->thread) {
            printk("ERROR could not start kthread\n");   
            continue;  
        }
        
        spin_lock_irqsave(&btn->lock, flags);

        // set up data:

        /* The following is the default callbacks (assuming gpio to extIrq).  For any
           other type of button, simply replace the callbacks */
        btn->pressIsr = btnPressIsr; 
        btn->releaseIsr = hasDownIsr ? btnReleaseIsr : NULL;
        btn->poll = btnPoll;
        btn->isDown = btnIsGpioBtnDown;
        btn->enableIrqs = btnEnableIrq;
        btn->disableIrqs = btnDisableIrq;

        // set up timer:
        if (btn->releaseIsr == NULL) {
            // we're going to have to poll the button to see when it's released:
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 19, 0)
            init_timer(&btn->timer);
            btn->timer.function = btn->poll;
            btn->timer.data     = (unsigned long)btn;
#else
            timer_setup(&btn->timer, btn->poll,0);
#endif
            btn->timer.expires  = jiffies + msecs_to_jiffies(BTN_POLLFREQ);
        }

        // set up external interrupts / gpios:
        
        btn->extIrqMap = map_external_irq (bpExtIrq);


#if defined(CONFIG_BCM960333)
        if (IsExtIntrShared(extIntrInfo[btn->extIrqIdx]))
            printk("Error -- Cannot share ext irqs in 60333 (due to mux... %d)\n", btn->extIrqIdx);
        mapBcm960333GpioToIntr(btn->gpio, bpExtIrq);
#elif defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848)
        kerSysInitPinmuxInterface(BP_PINMUX_FNTYPE_IRQ | btn->extIrqIdx );
#endif



        // Register button hooks:
        {
            int idx;
            for (idx = 0; idx < bpNumHooks; idx++) {
                unsigned short bpHook = bpHooks[idx];                
                void* bpHookParm = bpHookParms[idx];
                int timeInMs = (bpHook & BP_BTN_TRIG_TIME_MASK) * BP_BTN_TRIG_TIME_UNIT_IN_MS;
                int bpType = bpHook & BP_BTN_TRIG_TYPE_MASK;
                pushButtonNotifyHook_t hook = btnHooks[(bpHook & BP_BTN_ACTION_MASK) >> BP_BTN_ACTION_SHIFT ];

                printk("  Button %d: Registering %s hook %pf after %d ms \n", 
                             btn->btnId, 
                             bpType==BP_BTN_TRIG_PRESS?"press":bpType==BP_BTN_TRIG_HOLD?"hold":"release",
                             hook, timeInMs);
                
                switch (bpType) {
                    case BP_BTN_TRIG_PRESS:
                        registerPushButtonPressNotifyHook(btn->btnId, hook, bpHookParm);
                        break;
                    case BP_BTN_TRIG_HOLD:
                        registerPushButtonHoldNotifyHook(btn->btnId, hook, timeInMs, bpHookParm);
                        break;
                    case BP_BTN_TRIG_RELEASE:
                        registerPushButtonReleaseNotifyHook(btn->btnId, hook, timeInMs, bpHookParm);
                        break;
                }
                
            }
        }
        
        spin_unlock_irqrestore(&btn->lock, flags);

#if !defined(CONFIG_BCM947189)
        if (btn->pressIsr)
            BcmHalMapInterrupt((FN_HANDLER)btn->pressIsr, (void*)btn, btn->extIrqMap);
#endif

#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
            if (btn->pressIsr || btn->releaseIsr)
                BcmHalInterruptEnable(btn->extIrqMap);
#endif
    
        
    }
    return 0;
}

#if defined(WIRELESS) 

/**************************************************************************
* Name       : btnGetSesBtnIdx
*
* Description: This function returns the Btn Idx assgined for the Wireless
*              Ses.
*
* Parameters : [OUT] btnIdx- the idx for wireless SES
*
* Returns    : BP_SUCCESS - Success, value is returned.
*              BP_VALUE_NOT_DEFINED - At least one return value is not defined
*                  for the board.
***************************************************************************/

int btnGetSesBtnIdx(unsigned short *btnIdx) 
{
        void *iter = NULL;
        unsigned short     pusIdx; 
        unsigned short     pusGpio;
        unsigned short     pusExtIrq;
        unsigned short     pusNumHooks;
        unsigned short  parrusHooks[MAX_BTN_HOOKS_PER_BTN];
        void*           bpHookParms[MAX_BTN_HOOKS_PER_BTN];
        int i,ret;

        do {
            pusNumHooks = MAX_BTN_HOOKS_PER_BTN;
            ret = BpGetButtonInfo(&iter, &pusIdx, &pusGpio, &pusExtIrq, &pusNumHooks, parrusHooks, bpHookParms);
            if (ret != BP_SUCCESS)  {
                  return BP_VALUE_NOT_DEFINED;
            }
            for (i=0; i<pusNumHooks; i++) {
                if( (parrusHooks[i] & BP_BTN_ACTION_MASK) == BP_BTN_ACTION_SES ) {
                         *btnIdx=pusIdx;    
                         return BP_SUCCESS;
                }
           }
        } while(1);

        return BP_VALUE_NOT_DEFINED;
}

#endif

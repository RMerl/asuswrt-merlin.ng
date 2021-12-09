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
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/pci.h>
#include <asm/uaccess.h>

#include <bcmtypes.h>
#include <bcm_map_part.h>
#include <board.h>
#include <boardparms.h>
#include <bcm_intr.h>
#include <shared_utils.h>
#include <bcm_pinmux.h>
#include <bcmpci.h>

#include "board_wl.h"
#include "board_button.h"
#include <bcm_extirq.h>
#include "board_image.h"
#if defined (WIRELESS)
#include <linux/kmod.h>
#endif
extern wait_queue_head_t g_board_wait_queue;

unsigned short sesBtn_irq = BP_NOT_DEFINED;
static unsigned short sesBtn_gpio = BP_NOT_DEFINED;
static unsigned short sesBtn_polling = 0;
static struct timer_list sesBtn_timer;
static atomic_t sesBtn_active;
static atomic_t sesBtn_forced;

static irqreturn_t  sesBtn_isr(int irq, void *dev_id);
static Bool         sesBtn_pressed(void);
static int  __init  sesBtn_mapIntr(int context);
static void sesBtn_defaultAction(unsigned long time, void* param);

#if defined (WIRELESS)
unsigned long gSesBtnEvOutstanding = 0;
unsigned long gLastSesBtnEvTime;

static unsigned short sesLed_gpio = BP_NOT_DEFINED;

static void __init sesLed_mapGpio(void);
static void __init kerSysScreenPciDevices(void);

/* This spinlock is used to avoid race conditions caused by the
 * non-atomic test-and-set of sesBtn_active in sesBtn_read */
static DEFINE_SPINLOCK(sesBtn_newapi_spinlock);
#endif

void __init ses_board_init()
{
    int ret;
    ret = sesBtn_mapIntr(0);
    if (ret) {
        registerPushButtonPressNotifyHook(PB_BUTTON_1, sesBtn_defaultAction, 0);
#if defined(SUPPORT_IEEE1905)
        //1905 is triggered by the plc uke button action.  Attach hook to button 1 if using
        //the old style of board parms
        registerPushButtonPressNotifyHook(PB_BUTTON_1, btnHook_PlcUke, 0);
#endif
    }
#if defined(WIRELESS)
    sesLed_mapGpio();
#endif
}

void __exit ses_board_deinit()
{
    if( sesBtn_polling == 0 && sesBtn_irq != BP_NOT_DEFINED )
    {
        if(sesBtn_irq) {
            del_timer(&sesBtn_timer);
            atomic_set(&sesBtn_active, 0);
            atomic_set(&sesBtn_forced, 0);
#if !defined(CONFIG_BCM947189)
            if (!IsExtIntrShared(extIntrInfo[sesBtn_irq - INTERRUPT_ID_EXTERNAL_0])) {
                BcmHalInterruptDisable(sesBtn_irq);
            }
#endif
        }
    }
}

unsigned short sesBtn_getIrq(void)
{
    return sesBtn_irq;
}

/***************************************************************************
* SES Button ISR/GPIO/LED functions.
***************************************************************************/
static Bool sesBtn_pressed(void)
{
    unsigned int intSts = 0, extIntr, value = 0;
    int gpioActHigh = 0, isrDectActHigh = 0;
    int intrActive = 0;
    Bool pressed = 1;

    if( sesBtn_polling == 0 )
    {
#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM94908) 
        if ((sesBtn_irq >= INTERRUPT_ID_EXTERNAL_0) && (sesBtn_irq <= INTERRUPT_ID_EXTERNAL_5)) {
#elif defined(CONFIG_BCM96836) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96858) || defined(CONFIG_BCM96856)
        if ((sesBtn_irq >= INTERRUPT_ID_EXTERNAL_0) && (sesBtn_irq <= INTERRUPT_ID_EXTERNAL_7)) {
#elif defined(CONFIG_BCM947189)
        if (sesBtn_irq == INTERRUPT_ID_EXTERNAL_0) {
#else
        if ((sesBtn_irq >= INTERRUPT_ID_EXTERNAL_0) && (sesBtn_irq <= INTERRUPT_ID_EXTERNAL_3)) {
#endif
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148)
            intSts = kerSysGetGpioValue(MAP_EXT_IRQ_TO_GPIO( sesBtn_irq - INTERRUPT_ID_EXTERNAL_0));
#elif defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848)
            intSts = PERF->ExtIrqSts & (1 << (sesBtn_irq - INTERRUPT_ID_EXTERNAL_0 + EI_STATUS_SHFT));
#elif defined(CONFIG_BCM96858) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM96836) || \
      defined(CONFIG_BCM963158) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856)
            intSts = PERF->ExtIrqStatus & (1 << (sesBtn_irq - INTERRUPT_ID_EXTERNAL_0 + EI_STATUS_SHFT));
#elif defined(CONFIG_BCM947189)
            intSts = kerSysGetGpioValue(sesBtn_gpio);
#else
            intSts = PERF->ExtIrqCfg & (1 << (sesBtn_irq - INTERRUPT_ID_EXTERNAL_0 + EI_STATUS_SHFT));
#endif
        }
        else
            return 0;

        extIntr = extIntrInfo[sesBtn_irq-INTERRUPT_ID_EXTERNAL_0];
#if defined(CONFIG_BCM94908) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM96858) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96836) || defined(CONFIG_BCM96856)
        /* 4908 simplifies the interrupt status reporting. ExtIrqStatus report the actual interrupt state,
         * not the interupt pin high/low value as in the old chip, so no need to check interrupt detection polarity */
        if( intSts ) {
            (void)isrDectActHigh;
            intrActive = 1;
            BcmHalExternalIrqClear(sesBtn_irq);
        }
#else
        isrDectActHigh = IsExtIntrTypeActHigh(extIntr);
        if( (isrDectActHigh && intSts) || (!isrDectActHigh && !intSts) )
            intrActive = 1;
#endif
        if( intrActive )
        {
            //check the gpio status here too if shared.
            if( IsExtIntrShared(extIntr) )
            {
                gpioActHigh = sesBtn_gpio&BP_ACTIVE_LOW ? 0 : 1;
                value = kerSysGetGpioValue(sesBtn_gpio);
                if( (value&&!gpioActHigh) || (!value&&gpioActHigh) )
                     pressed = 0;
            }
        }
        else {
            pressed = 0;
        }
    }
    else
    {
        pressed = 0;
        if( sesBtn_gpio != BP_NOT_DEFINED )
        {
            gpioActHigh = sesBtn_gpio&BP_ACTIVE_LOW ? 0 : 1;
            value = kerSysGetGpioValue(sesBtn_gpio);
            if( (value&&gpioActHigh) || (!value&&!gpioActHigh) )
                pressed = 1;
        }
    }

    return pressed;
}

static void sesBtn_timer_handler(unsigned long arg)
{
    unsigned long currentJiffies = jiffies;
    if ( sesBtn_pressed() ) {
        doPushButtonHold(PB_BUTTON_1, currentJiffies);
        mod_timer(&sesBtn_timer, currentJiffies + msecs_to_jiffies(100)); 
    }
    else {
        atomic_set(&sesBtn_active, 0);
        doPushButtonRelease(PB_BUTTON_1, currentJiffies);
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858) || \
    defined(CONFIG_BCM96836) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856)
        BcmHalExternalIrqUnmask(sesBtn_irq);
#elif defined(CONFIG_BCM947189)
        GPIO->gpiointmask |= (1 << sesBtn_gpio);
#else
        BcmHalInterruptEnable(sesBtn_irq);
#endif
    }
}


static void sesBtn_defaultAction(unsigned long time, void* param) {
    wake_up_interruptible(&g_board_wait_queue);
}

static irqreturn_t sesBtn_isr(int irq, void *dev_id)
{
    int ext_irq_idx = 0, value=0;
    irqreturn_t ret = IRQ_NONE;
    unsigned long currentJiffies = jiffies;

    ext_irq_idx = irq - INTERRUPT_ID_EXTERNAL_0;
    if (IsExtIntrShared(extIntrInfo[ext_irq_idx]))
    {
        value = kerSysGetGpioValue(*(int *)dev_id);
        if( (IsExtIntrTypeActHigh(extIntrInfo[ext_irq_idx]) && value) || (IsExtIntrTypeActLow(extIntrInfo[ext_irq_idx]) && !value) )
        {
#if defined(CONFIG_BCM947189)
            if (GPIO->gpiointmask & (1 << sesBtn_gpio))
#endif
            ret = IRQ_HANDLED;
        }
    }
    else
    {
        ret = IRQ_HANDLED;
    }

    if (IRQ_HANDLED == ret) {   
        int timerSet = mod_timer(&sesBtn_timer, (currentJiffies + msecs_to_jiffies(100)));    /* 100 msec */

#if defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858) || defined(CONFIG_BCM96836) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM96856)
        BcmHalExternalIrqClear(irq);
#endif
#if defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM94908) || defined(CONFIG_BCM96858) || \
    defined(CONFIG_BCM96836) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856)
        BcmHalExternalIrqMask(irq);
#endif
#if defined(CONFIG_BCM947189)
        GPIO->gpiointmask &= ~(1 << sesBtn_gpio);
#endif
        if ( 0 == timerSet ) { 
            atomic_set(&sesBtn_active, SES_BTN_LEGACY);
            doPushButtonPress(PB_BUTTON_1, currentJiffies);
        }
    }

#if !defined(CONFIG_BCM_6802_MoCA) && !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
    if (IsExtIntrShared(extIntrInfo[ext_irq_idx])) {
        BcmHalInterruptEnable(sesBtn_irq);
    }
#endif

    return ret;
}

// return 1 if interrupt was mapped.  Return 0 otherwise
static int __init sesBtn_mapIntr(int context)
{
    int ret = 0;
    int ext_irq_idx;

    if( BpGetWirelessSesExtIntr(&sesBtn_irq) == BP_SUCCESS )
    {
        BpGetWirelessSesExtIntrGpio(&sesBtn_gpio);
        if( sesBtn_irq != BP_EXT_INTR_NONE )
        {
#if defined(CONFIG_BCM960333)
            if( sesBtn_gpio != BP_NOT_DEFINED && sesBtn_irq != BP_EXT_INTR_NONE) 
                mapBcm960333GpioToIntr(sesBtn_gpio & BP_GPIO_NUM_MASK, sesBtn_irq);
#endif      
            printk("SES: Button Interrupt 0x%x is enabled\n", sesBtn_irq);
        }
        else
        {
            if( sesBtn_gpio != BP_NOT_DEFINED )
            {
                printk("SES: Button Polling is enabled on gpio %x\n", sesBtn_gpio);
                kerSysSetGpioDirInput(sesBtn_gpio);
                sesBtn_polling = 1;
            }
        }
    }
    else
        return 0;

    if( sesBtn_irq != BP_EXT_INTR_NONE )
    {
        ext_irq_idx = (sesBtn_irq&~BP_EXT_INTR_FLAGS_MASK)-BP_EXT_INTR_0;
#if defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848)
        kerSysInitPinmuxInterface(BP_PINMUX_FNTYPE_IRQ | ext_irq_idx);
#endif
        if (!IsExtIntrConflict(extIntrInfo[ext_irq_idx]))
        {
            static int dev = -1;
            int hookisr = 1;

            if (IsExtIntrShared(sesBtn_irq))
            {
                /* get the gpio and make it input dir */
                if( sesBtn_gpio != BP_NOT_DEFINED )
                {
                    sesBtn_gpio &= BP_GPIO_NUM_MASK;;
                    printk("SES: Button Interrupt gpio is %d\n", sesBtn_gpio);
                    kerSysSetGpioDirInput(sesBtn_gpio);
                    dev = sesBtn_gpio;
#if defined (CONFIG_BCM947189)
                    MISC->intmask |= 1; /* enable GPIO interrupts */
                    if ((sesBtn_irq & BP_ACTIVE_MASK) == BP_ACTIVE_LOW)
                        GPIO->gpiointpolarity |= (1 << sesBtn_gpio);
                    GPIO->gpiointmask |= (1 << sesBtn_gpio);
#endif
                }
                else
                {
                    printk("SES: Button Interrupt gpio definition not found \n");
                    hookisr = 0;
                }
            }

            if(hookisr)
            {
                init_timer(&sesBtn_timer);
                sesBtn_timer.function = sesBtn_timer_handler;
                sesBtn_timer.expires  = jiffies + msecs_to_jiffies(100);    /* 100 msec */
                sesBtn_timer.data     = 0;
                atomic_set(&sesBtn_active, 0);
                atomic_set(&sesBtn_forced, 0);
                sesBtn_irq = map_external_irq (sesBtn_irq);
                ret = 1;
                BcmHalMapInterrupt((FN_HANDLER)sesBtn_isr, (void*)&dev, sesBtn_irq);
#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
                BcmHalInterruptEnable(sesBtn_irq);
#endif
            }
        }
    }

    return ret;
}

/**
 *_get_wl_nandmanufacture check if system is nand system and manufacture bit is set
 *
 */
int _get_wl_nandmanufacture(void ) {

    int is_nand=0,is_m=0,has_size=0;
    unsigned int rootfs_ofs;
    NVRAM_DATA *pNvramData;
    
    is_nand =( kerSysBlParmsGetInt(NAND_RFS_OFS_NAME, (int *) &rootfs_ofs) != -1 )? WLAN_MFG_PARTITION_ISNAND:0;

    if (NULL != (pNvramData = readNvramData()))  {
        is_m = (((unsigned char)(pNvramData ->wlanParams[NVRAM_WLAN_PARAMS_LEN-1])) &
               WLAN_FEATURE_DHD_MFG_ENABLE)?WLAN_MFG_PARTITION_MFGSET:0;
        has_size= (pNvramData->part_info[WLAN_MFG_PARTITION_INDEX].size>0 &&
                   pNvramData->part_info[WLAN_MFG_PARTITION_INDEX].size<0xffff)?WLAN_MFG_PARTITION_HASSIZE:0;
        kfree(pNvramData);
    }
    return is_nand|is_m|has_size;
}

/**
 * _wlsrom_write_file - write char content into file 
 * @name        file name
 * @content     the content to save
 * @size        size of the conten
 *
 * Return 0 success or -1 totherwise
 */
int _wlsrom_write_file(char *name,char *content,int size) {
       struct file *fp=filp_open(name,O_WRONLY|O_CREAT,0);
       int err=0;
       if(!IS_ERR(fp)) {
          mm_segment_t old_fs=get_fs();
          loff_t pos=0L;
          set_fs(get_ds());
          vfs_write(fp,(void __user *)(content),size,&pos);
          set_fs(old_fs);
          err=vfs_fsync(fp,0);
          if(err) 
              printk("SYNC to disk error!!!\n");
          filp_close(fp,NULL);
          return 0;
    }
       return -1;
}


/***************************************************************************/
// BP_BTN_ACTION_SES
void btnHook_Ses(unsigned long timeInMs, void* param) {
#if defined(WIRELESS)
    unsigned long flags;

    printk(" *** Doing SES *** (%pf)\n\n", sesBtn_defaultAction);

    /* This one is a bit trickier, as the wireless does not accept sockets
       etc.  Thus, the wireless is going to poll to see whether the button
       was pressed.  What we do, is we record the time in jiffies.  When the
       wireless polls, we check if we were within 200ms of the last press */
    gLastSesBtnEvTime = jiffies;
    gSesBtnEvOutstanding = 1;

    /* Synchronization note: This code is protected with
     * sesBtn_newapi_spinlock to avoid a race condition with the
     * reading/writing of sesBtn_active in sesBtn_read
     */
    spin_lock_irqsave(&sesBtn_newapi_spinlock, flags);
    switch ((unsigned long)param) {
    case SES_BTN_PARAM_AP:
        atomic_set(&sesBtn_active, SES_BTN_AP);
        break;
    case SES_BTN_PARAM_STA:
        atomic_set(&sesBtn_active, SES_BTN_STA);
        break;
    default:
        atomic_set(&sesBtn_active, SES_BTN_AP);
        break;
    }
    spin_unlock_irqrestore(&sesBtn_newapi_spinlock, flags);

    sesBtn_defaultAction(timeInMs, param);
#endif
}

#if defined(WIRELESS)
unsigned int sesBtn_poll(struct file *file, struct poll_table_struct *wait)
{
    // this is called by the wireless driver to determine if the button is down.  If
    // we are using the new button method, we simply check if the button trigger 
    // occured within the last second.   Otherwise, we fall through to check the 
    // original checks.
    if (gSesBtnEvOutstanding) {
        if (time_after(gLastSesBtnEvTime+HZ, jiffies)) {            
            return POLLIN;
        } else {
            atomic_set(&sesBtn_active, 0);
            gSesBtnEvOutstanding = 0;
        }
         
    }
    
    if ( sesBtn_polling ) {
        if ( sesBtn_pressed() ) {
            return POLLIN;
        }
    }
    else if (atomic_read(&sesBtn_active)) {
        return POLLIN;
    }
    else if (atomic_read(&sesBtn_forced)) {
        atomic_set(&sesBtn_forced,0);
        return POLLIN;
    }
    return 0;
}

ssize_t sesBtn_read(struct file *file,  char __user *buffer, size_t count, loff_t *ppos)
{
    volatile unsigned int event=0;
    ssize_t ret=0;
    unsigned long flags;
    int exit = 0;

    /* Synchronization note: This code does a non-atomic test and set of
     * sesBtn_active that could cause a race-condition with btnHook_Ses,
     * so this must be protected with sesBtn_newapi_spinlock.
     */

    /* New button API: Return the type of SES button press (Short/Long) */
    spin_lock_irqsave(&sesBtn_newapi_spinlock, flags);
    if (atomic_read(&sesBtn_active) == SES_BTN_AP) {
        event = SES_EVENTS | SES_EVENT_BTN_AP;
        atomic_set(&sesBtn_active, 0);
    }
    else if (atomic_read(&sesBtn_active) == SES_BTN_STA) {
        event = SES_EVENTS | SES_EVENT_BTN_STA;
        atomic_set(&sesBtn_active, 0);
    }
    /* Legacy button API: Return a simple flag (SES_EVENTS) and let the
     * userspace code call read repeatedly to calculate the press time
     */
    else {
        if (sesBtn_polling) {
            if (0 == sesBtn_pressed()) {
                exit = 1;
            }
        }
        else if (0 == atomic_read(&sesBtn_active)) {
            exit = 1;
        }
        event = SES_EVENTS;
    }
    spin_unlock_irqrestore(&sesBtn_newapi_spinlock, flags);

    if (exit)
        return ret;

    gSesBtnEvOutstanding = 0;

    __copy_to_user((char*)buffer, (char*)&event, sizeof(event));
    count -= sizeof(event);
    buffer += sizeof(event);
    ret += sizeof(event);
    return ret;
}

/***********************************************************************
* Function Name: kerSysScreenPciDevices
* Description  : Screen Pci Devices before loading modules
***********************************************************************/
static void __init kerSysScreenPciDevices(void)
{
    unsigned short wlFlag;

    if((BpGetWirelessFlags(&wlFlag) == BP_SUCCESS) && (wlFlag & BP_WLAN_EXCLUDE_ONBOARD)) {
        /*
        * scan all available pci devices and delete on board BRCM wireless device
        * if external slot presents a BRCM wireless device
        */
        int foundPciAddOn = 0;
        struct pci_dev *pdevToExclude = NULL;
        struct pci_dev *dev = NULL;

        while((dev=pci_get_device(PCI_ANY_ID, PCI_ANY_ID, dev))!=NULL) {
            printk("kerSysScreenPciDevices: 0x%x:0x%x:(slot %d) detected\n", dev->vendor, dev->device, PCI_SLOT(dev->devfn));
            if((dev->vendor == BRCM_VENDOR_ID) &&
                (((dev->device & 0xff00) == BRCM_WLAN_DEVICE_IDS)|| 
                ((dev->device/1000) == BRCM_WLAN_DEVICE_IDS_DEC))) {
                    if(PCI_SLOT(dev->devfn) != WLAN_ONBOARD_SLOT) {
                        foundPciAddOn++;
                    } else {
                        pdevToExclude = dev;
                    }                
            }
        }

#ifdef CONFIG_PCI
        if(((wlFlag & BP_WLAN_EXCLUDE_ONBOARD_FORCE) || foundPciAddOn) && pdevToExclude) {
            printk("kerSysScreenPciDevices: 0x%x:0x%x:(onboard) deleted\n", pdevToExclude->vendor, pdevToExclude->device);
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 4, 0)
            pci_remove_bus_device(pdevToExclude);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0)
                ; //do nothing
#else
            __pci_remove_bus_device(pdevToExclude);
#endif
        }
#else
#error ATTEMPT TO COMPILE WIRELESS WITHOUT PCI
#endif
    }
}

/***********************************************************************
* Function Name: kerSetWirelessPD
* Description  : Control Power Down by Hardware if the board supports
***********************************************************************/
void kerSetWirelessPD(int state)
{
    unsigned short wlanPDGpio;
    if((BpGetWirelessPowerDownGpio(&wlanPDGpio)) == BP_SUCCESS) {
        if (wlanPDGpio != BP_NOT_DEFINED) {
            if(state == WLAN_OFF)
                kerSysSetGpioState(wlanPDGpio, kGpioActive);
            else
                kerSysSetGpioState(wlanPDGpio, kGpioInactive);
        }
    }
}

static void __init sesLed_mapGpio()
{
    if( BpGetWirelessSesLedGpio(&sesLed_gpio) == BP_SUCCESS )
    {
        printk("SES: LED GPIO 0x%x is enabled\n", sesLed_gpio);
    }
}

void sesLed_ctrl(int action)
{
    char blinktype = ((action >> 24) & 0xff); /* extract blink type for SES_LED_BLINK  */

    BOARD_LED_STATE led;

    if(sesLed_gpio == BP_NOT_DEFINED)
        return;

    action &= 0xff; /* extract led */

    switch (action) {
    case SES_LED_ON:
        led = kLedStateOn;
        break;
    case SES_LED_BLINK:
        if(blinktype)
            led = blinktype;
        else
            led = kLedStateSlowBlinkContinues;
        break;
    case SES_LED_OFF:
    default:
        led = kLedStateOff;
    }

    kerSysLedCtrl(kLedSes, led);
}

void kerSysSesEventTrigger( int forced )
{
   if (forced) {
      atomic_set (&sesBtn_forced, 1);
   }
   wake_up_interruptible(&g_board_wait_queue);
}


/**
 *kerSysGetWlanSromParamsLen - get wlan calibration data len
 *
 */

int kerSysGetWlanSromParamsLen(void ) 
{
    int nm=_get_wl_nandmanufacture();
    if(nm<WLAN_MFG_PARTITION_HASSIZE) {
        /*  there is no size when it is NAND or it is not NAND */
        return NVRAM_WLAN_PARAMS_LEN-1;
    } else {
        struct file *fp=filp_open(WL_SROM_CUSTOMER_FILE,O_RDONLY,0);
        if(!IS_ERR(fp)) {
            int cur_off=generic_file_llseek(fp,0,SEEK_CUR);
            int len=generic_file_llseek(fp,0,SEEK_END);
           generic_file_llseek(fp,cur_off,SEEK_SET);
           filp_close(fp,NULL);
           return len;
        } 
        return 0;
    }
}
EXPORT_SYMBOL(kerSysGetWlanSromParamsLen);

/*Read Wlan Params data from CFE */
int kerSysGetWlanSromParams( unsigned char *wlanParams, unsigned short len)
{
    int nm=_get_wl_nandmanufacture();
    if(nm<WLAN_MFG_PARTITION_HASSIZE) {
        NVRAM_DATA *pNvramData;
        if (NULL == (pNvramData = readNvramData()))
        {
            printk("kerSysGetWlanSromParams: could not read nvram data\n");
            return -1;
        }

        memcpy( wlanParams,
                (char *)pNvramData + ((size_t) &((NVRAM_DATA *)0)->wlanParams),
                len );
        kfree(pNvramData);
    } else {
        struct file *fp=filp_open(WL_SROM_CUSTOMER_FILE,O_RDONLY,0);
        if(!IS_ERR(fp)) {
           int rl=kernel_read(fp,0,wlanParams,len);
           filp_close(fp,NULL);
           if(rl<=0) 
               return -1;
        } else {
              return -1;
        }
    }
    return 0;
}
EXPORT_SYMBOL(kerSysGetWlanSromParams);

unsigned char kerSysGetWlanFeature(void)
{
    NVRAM_DATA *pNvramData;

    unsigned char wlfeature=0;
    if (NULL == (pNvramData = readNvramData()))
    {
        printk("kerSysGetWlanSromParams: could not read nvram data\n");
        return -1;
    }
    wlfeature= (unsigned char)(pNvramData ->wlanParams[NVRAM_WLAN_PARAMS_LEN-1]);
    kfree(pNvramData);
    return wlfeature;
    
}
EXPORT_SYMBOL(kerSysGetWlanFeature);

void board_util_wl_godefault(void) {
	char  *envp[]= {"HOME=/",NULL};
	char *argv[]= {"/bin/nvram","godefault",NULL};
	call_usermodehelper(argv[0],argv,envp,UMH_WAIT_EXEC);
}
#endif /* WIRELESS */


void __init board_wl_init(void)
{
#if defined (WIRELESS)
    kerSysScreenPciDevices();
    kerSetWirelessPD(WLAN_ON);
#endif
    ses_board_init();

    return;
}

void __exit board_wl_deinit(void)
{
#if !defined(CONFIG_BCM963138) && !defined(CONFIG_BCM963148)
    ses_board_deinit();
#endif
    return;
}

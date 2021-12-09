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
#include <linux/interrupt.h>

#include <bcmtypes.h>
#include <bcm_map_part.h>
#include <board.h>
#include <boardparms.h>
#include <bcm_intr.h>
#include <shared_utils.h>
#include <bcm_pinmux.h>

#include <bcm_extirq.h>
#if defined(CONFIG_BCM_6802_MoCA)
#include "./moca/board_moca.h"
#endif

#if defined(WIRELESS)
#include "board_wl.h"
#endif
int restore_in_progress = 0;
unsigned int extIntrInfo[NUM_EXT_INT];
unsigned short resetBtn_gpio = BP_NOT_DEFINED;

static unsigned short resetBtn2_gpio = BP_NOT_DEFINED;

static irqreturn_t reset_isr(int irq, void *dev_id);

/* restore default work structure */
static struct work_struct restoreDefaultWork;


static int set_ext_irq_info(unsigned short ext_irq)
{
    int irq_idx, rc = 0;

    irq_idx = (ext_irq&~BP_EXT_INTR_FLAGS_MASK)-BP_EXT_INTR_0;

    if (unlikely(irq_idx < 0 || irq_idx >= NUM_EXT_INT))
    {
        printk(KERN_ERR "%s ERROR: Invalid index entry %d max is %d   ext_irq %u BP_EXT_INTR_FLAGS_MASK 0x%x BP_EXT_INTR_0 0x%x\n",__func__, 
            irq_idx, NUM_EXT_INT, ext_irq, BP_EXT_INTR_FLAGS_MASK, BP_EXT_INTR_0);
        return -1;
    } 

    if( extIntrInfo[irq_idx] == (unsigned int)(-1) ) {
        extIntrInfo[irq_idx] = ext_irq;
    }
    else
    {
        /* make sure all the interrupt sharing this irq number has the trigger type and shared */
        if( ext_irq != (unsigned int)extIntrInfo[irq_idx] )
        {
            printk("Invalid ext intr type for BP_EXT_INTR_%d: 0x%x vs 0x%x\r\n", irq_idx, ext_irq, extIntrInfo[irq_idx]);
            extIntrInfo[irq_idx] |= BP_EXT_INTR_CONFLICT_MASK;
            rc = -1;
        }
    }

    return rc;
}

#ifdef CONFIG_BP_PHYS_INTF
static void init_optical_module_pres_ext_irq_info(int intf_type)
{
    int intf_num, i;
    unsigned short intr;

    if((intf_num = BpGetPhyIntfNumByType(intf_type))) {
        for( i = 0; i < intf_num; i++ ) {
            if( BpGetOpticalModulePresenceExtIntr(intf_type, i, &intr) == BP_SUCCESS ) 
                set_ext_irq_info(intr);
        }
    }

    return;
}
#endif

static void init_ext_irq_info(void)
{
    int i;
    unsigned short intr;
    void * iter = NULL;
    unsigned short bpBtnIdx, bpGpio, bpExtIrq;

    /* mark each entry invalid */
    for(i=0; i<NUM_EXT_INT; i++)
        extIntrInfo[i] = (unsigned int)(-1);

    /* collect all the external interrupt info from bp */
    if( BpGetResetToDefaultExtIntr(&intr) == BP_SUCCESS )
        set_ext_irq_info(intr);

    if( BpGetResetToDefault2ExtIntr(&intr) == BP_SUCCESS )
        set_ext_irq_info(intr);

    if( BpGetWirelessSesExtIntr(&intr) == BP_SUCCESS )
        set_ext_irq_info(intr);

    if( BpGetSDCardDetectExtIntr(&intr) == BP_SUCCESS )
        set_ext_irq_info(intr);

#if defined CONFIG_BCM963138 || defined CONFIG_BCM963148 
    if( BpGetNfcExtIntr(&intr) == BP_SUCCESS )
        set_ext_irq_info(intr);
#endif

#if defined(CONFIG_BCM_PON) || defined(CONFIG_BCM963158)
    if( BpGetPmdAlarmExtIntr(&intr) == BP_SUCCESS )
        set_ext_irq_info(intr);
#ifdef CONFIG_BCM_PON
   if( BpGetWanSignalDetectedExtIntr(&intr) == BP_SUCCESS )
        set_ext_irq_info(intr);
#endif
#endif

    if( BpGetWifiOnOffExtIntr(&intr) == BP_SUCCESS )
        set_ext_irq_info(intr);

#ifdef CONFIG_BP_PHYS_INTF
    /* both xPON and SGMII interface can use the this interrupt */
    init_optical_module_pres_ext_irq_info(BP_INTF_TYPE_xPON);
    init_optical_module_pres_ext_irq_info(BP_INTF_TYPE_SGMII);
#else
    if ( BpGetOpticalModulePresenceExtIntr(&intr) == BP_SUCCESS )
    {
        set_ext_irq_info(intr);
        BpGetOpticalModulePresenceExtIntrGpio(&bpGpio);
    }
#endif
    while(BpGetButtonInfo(&iter, &bpBtnIdx, &bpGpio, &bpExtIrq, NULL, NULL, NULL) == BP_SUCCESS) {
        set_ext_irq_info(bpExtIrq);
        }

    return;
}

#if defined(CONFIG_BCM_6802_MoCA)
void init_moca_ext_irq_info(void)
{
    int i,j;
    for( i = 0; i < boardGetMocaDevNum(); i++ )
    {
        PBP_MOCA_INFO pMocaInfo;

        pMocaInfo = boardGetMocaInfo(i);
        for( j = 0; j < BP_MOCA_MAX_INTR_NUM; j++ )
        {
            if( pMocaInfo->intr[j] != BP_NOT_DEFINED )
            {
#if defined(CONFIG_BCM96838)
                unsigned short irqIdx = map_external_irq(pMocaInfo->intr[j]) - INTERRUPT_ID_EXTERNAL_0;
                unsigned short gpio = pMocaInfo->intrGpio[j];
                gpio &= BP_GPIO_NUM_MASK;
                PERF->ext_irq_muxsel0 |= ( (gpio&EXT_IRQ_MASK_LOW) << (irqIdx*EXT_IRQ_OFF_LOW) );
                DBGPERF->Dbg_extirqmuxsel0_1 |= ( ((gpio&EXT_IRQ_MASK_HIGH)>>EXT_IRQ_OFF_LOW) << (irqIdx*EXT_IRQ_OFF_HIGH) );
#endif
                set_ext_irq_info(pMocaInfo->intr[j]);
            }
        }
    }
}
EXPORT_SYMBOL(init_moca_ext_irq_info);
#endif

void init_reset_irq(void)
{
    unsigned short rstToDflt_irq;
    unsigned short rstToDflt2_irq;
    int ext_irq_idx;
    int ext_irq2_idx;

    if( BpGetResetToDefaultExtIntr(&rstToDflt_irq) == BP_SUCCESS )
    {
        ext_irq_idx = (rstToDflt_irq&~BP_EXT_INTR_FLAGS_MASK)-BP_EXT_INTR_0;
#if defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848)
        kerSysInitPinmuxInterface(BP_PINMUX_FNTYPE_IRQ | ext_irq_idx);
#endif

#if defined(CONFIG_BCM960333)
        if (BpGetResetToDefaultExtIntrGpio(&resetBtn_gpio) == BP_SUCCESS) {
            resetBtn_gpio &= BP_GPIO_NUM_MASK;
            if ( ext_irq_idx  != BP_NOT_DEFINED && resetBtn_gpio != BP_NOT_DEFINED ) {
                mapBcm960333GpioToIntr( resetBtn_gpio, rstToDflt_irq );
            }
        }
#endif

        if (!IsExtIntrConflict(extIntrInfo[ext_irq_idx]))
        {
            static int dev = -1;
            int hookisr = 1;

            if (IsExtIntrShared(rstToDflt_irq))
            {
                /* get the gpio and make it input dir */
                if( (resetBtn_gpio != BP_NOT_DEFINED) || (BpGetResetToDefaultExtIntrGpio(&resetBtn_gpio) == BP_SUCCESS) )
                {
                    resetBtn_gpio &= BP_GPIO_NUM_MASK;
                    printk("brcm_board_init: Reset config Interrupt gpio is %d\n", resetBtn_gpio);
                    kerSysSetGpioDirInput(resetBtn_gpio); // Set gpio for input.
                    dev = resetBtn_gpio;
#if defined(CONFIG_BCM947189)
                    MISC->intmask |= 1; /* enable GPIO interrupts */
                    if ((rstToDflt_irq & BP_ACTIVE_MASK) == BP_ACTIVE_LOW)
                        GPIO->gpiointpolarity |= (1 << resetBtn_gpio);
                    GPIO->gpiointmask |= (1 << resetBtn_gpio);
#endif
                }
                else
                {
                    printk("brcm_board_init: Reset config gpio definition not found \n");
                    hookisr= 0;
                }
            }
                
            if(hookisr)
            {
                ext_irq_connect(rstToDflt_irq, (void*)&dev, (FN_HANDLER)reset_isr);
            }
        }
    }

    if( BpGetResetToDefault2ExtIntr(&rstToDflt2_irq) == BP_SUCCESS )
    {
        ext_irq2_idx = (rstToDflt2_irq&~BP_EXT_INTR_FLAGS_MASK)-BP_EXT_INTR_0;
#if defined(CONFIG_BCM960333)
        if (BpGetResetToDefault2ExtIntrGpio(&resetBtn2_gpio) == BP_SUCCESS) {
            resetBtn2_gpio &= BP_GPIO_NUM_MASK;
            if ( ext_irq2_idx  != BP_NOT_DEFINED && resetBtn2_gpio != BP_NOT_DEFINED ) {
                mapBcm960333GpioToIntr( resetBtn2_gpio, rstToDflt2_irq );
            }
        }
#endif

        if (!IsExtIntrConflict(extIntrInfo[ext_irq2_idx]))
        {
            static int dev = -1;
            int hookisr = 1;

            if (IsExtIntrShared(rstToDflt2_irq))
            {
                /* get the gpio and make it input dir */
                if( (resetBtn2_gpio != BP_NOT_DEFINED) || (BpGetResetToDefault2ExtIntrGpio(&resetBtn2_gpio) == BP_SUCCESS) )
                {
                    resetBtn2_gpio &= BP_GPIO_NUM_MASK;
                    kerSysSetGpioDirInput(resetBtn2_gpio); // Set gpio for input.
                    dev = resetBtn2_gpio;
                }
                else
                {
                        hookisr= 0;
                }
            }
                
            if(hookisr)
            {
                ext_irq_connect(rstToDflt2_irq, (void*)&dev, (FN_HANDLER)reset_isr);
            }
        }
        else 
        {
            printk("[%s.%d]: conflict exists %d (%08x)\n", __func__, __LINE__, ext_irq2_idx, extIntrInfo[ext_irq2_idx]);
        }
    }

    return;
}
EXPORT_SYMBOL(init_reset_irq);

/***************************************************************************
* legacy restore to default interrupt button handling
***************************************************************************/
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 20)
static void restore_to_default_thread(struct work_struct *work)
#else
static void restore_to_default_thread(void *arg)
#endif
{
    char buf[256];

    memset(buf, 0, sizeof(buf));

    // Do this in a kernel thread so we don't have any restriction
    printk("Restore to Factory Default Setting ***\n\n");
#if defined(WIRELESS)
	board_util_wl_godefault();
#endif
    kerSysPersistentSet( buf, sizeof(buf), 0 );
#if defined(CONFIG_BCM_PLC_BOOT)
    kerSysFsFileSet("/data/plc/plc_pconfig_state", buf, 1);
#endif

    // kernel_restart is a high level, generic linux way of rebooting.
    // It calls a notifier list and lets sub-systems know that system is
    // rebooting, and then calls machine_restart, which eventually
    // calls kerSysMipsSoftReset.
    kernel_restart(NULL);

    return;
}

static irqreturn_t reset_isr(int irq, void *dev_id)
{
    int isOurs = 1, ext_irq_idx = 0, value=0;

    //printk("reset_isr called irq %d, gpio %d 0x%lx\n", irq, *(int *)dev_id, PERF->IrqControl32[0].IrqMaskHi);

    ext_irq_idx = irq - INTERRUPT_ID_EXTERNAL_0;
    if (IsExtIntrShared(extIntrInfo[ext_irq_idx]))
    {
       value = kerSysGetGpioValue(*(int *)dev_id);
       if( (IsExtIntrTypeActHigh(extIntrInfo[ext_irq_idx]) && value) || (IsExtIntrTypeActLow(extIntrInfo[ext_irq_idx]) && !value) )
           isOurs = 1;
       else
           isOurs = 0;
    }

    if (isOurs)
    {
        if( !restore_in_progress )
        {
            restore_in_progress  = 1;
            printk("\n***reset button press detected***\n\n");
            INIT_WORK(&restoreDefaultWork, restore_to_default_thread);
            schedule_work(&restoreDefaultWork);
        }
#if defined(CONFIG_BCM94908)  || defined(CONFIG_BCM96858) | \
    defined(CONFIG_BCM963158) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM96878)
        BcmHalExternalIrqClear(irq);
#endif
        return IRQ_HANDLED;
    }

    return IRQ_NONE;
}

/*********************************************************************************
 *                            Exported Functions                                 *
 *********************************************************************************/                      

#if defined(CONFIG_BCM960333)
void mapBcm960333GpioToIntr( unsigned int gpio, unsigned int extIrq )
{
    unsigned int extIrqMap = map_external_irq(extIrq) - INTERRUPT_ID_EXTERNAL_0;
    volatile uint32 * pMuxReg = &(GPIO->GPIOMuxCtrl_0) + gpio/4;
    int gpioShift = (gpio % 4) * 8;
    uint32 gpioMask = 0x7f << gpioShift;

    BcmHalExternalIrqMask(extIrqMap);
    *pMuxReg = (*pMuxReg & (~gpioMask)) | (extIrqMap << gpioShift);
    GPIO->GPIOFuncMode |= 1 << gpio;
}
EXPORT_SYMBOL(mapBcm960333GpioToIntr);
#endif

int map_external_irq (int irq)
{
    int map_irq;
    irq &= ~BP_EXT_INTR_FLAGS_MASK;

    switch (irq) {
    case BP_EXT_INTR_0   :
        map_irq = INTERRUPT_ID_EXTERNAL_0;
        break ;
#if !defined(CONFIG_BCM947189)
    case BP_EXT_INTR_1   :
        map_irq = INTERRUPT_ID_EXTERNAL_1;
        break ;
    case BP_EXT_INTR_2   :
        map_irq = INTERRUPT_ID_EXTERNAL_2;
        break ;
    case BP_EXT_INTR_3   :
        map_irq = INTERRUPT_ID_EXTERNAL_3;
        break ;
#if defined(CONFIG_BCM96838) || defined(CONFIG_BCM963138) || defined(CONFIG_BCM963148) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM96858) || \
    defined(CONFIG_BCM94908) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM96878)
    case BP_EXT_INTR_4   :
        map_irq = INTERRUPT_ID_EXTERNAL_4;
        break ;
    case BP_EXT_INTR_5   :
        map_irq = INTERRUPT_ID_EXTERNAL_5;
        break ;
#endif
#if defined(CONFIG_BCM963381) || defined(CONFIG_BCM96848) || defined(CONFIG_BCM963158) || \
    defined(CONFIG_BCM96846) || defined(CONFIG_BCM96858)  || defined(CONFIG_BCM96856) || defined(CONFIG_BCM96878)
    case BP_EXT_INTR_6   :
        map_irq = INTERRUPT_ID_EXTERNAL_6;
        break ;
    case BP_EXT_INTR_7   :
        map_irq = INTERRUPT_ID_EXTERNAL_7;
        break ;
#endif
#endif
    default           :
        printk ("Invalid External Interrupt definition (%08x)\n", irq) ;
        map_irq = 0 ;
        break ;
    }

    return (map_irq) ;
}
EXPORT_SYMBOL(map_external_irq);

int ext_irq_connect(int irq, void* param, FN_HANDLER isr)
{
    int rc = 0;

    irq = map_external_irq(irq);
    rc = BcmHalMapInterrupt(isr, param, irq);
#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
    if (!rc)
    {
        BcmHalInterruptEnable(irq);
    }
#endif
    return rc;
}
EXPORT_SYMBOL(ext_irq_connect);

/* The ext_irq_disconnect API is experimental. Be careful. */
void ext_irq_disconnect(int irq, void* param)
{
#if !defined(CONFIG_BCM947189)
    ext_irq_enable(irq, 0);

    irq = map_external_irq(irq);

    /* TBD - We have an inherited potential memory leak of devname provided by request_irq
     * In some Linux versions, free_irq returns a pointer to devname. We get void.
     * We have to kfree(devname).
     * ( Please notice the internal structure: action->name = devname ).
     */
    free_irq(irq, param);
#endif
}
EXPORT_SYMBOL(ext_irq_disconnect);

void ext_irq_enable(int irq, int enable)
{
    irq = map_external_irq(irq);
#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
    if (enable)
        BcmHalInterruptEnable(irq);
    else
        BcmHalInterruptDisable(irq);
#else
#if defined(CONFIG_BCM96858) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM96878)
    if (enable)
    	BcmHalExternalIrqUnmask(irq);
    else
    	BcmHalExternalIrqMask(irq);
#endif
#endif
}
EXPORT_SYMBOL(ext_irq_enable);

/***************************************************************************
 * Function Name: kerSysGetExtIntInfo
 * Description  : return the external interrupt information which includes the
 *                trigger type, sharing enable.
 * Returns      : pointer to buf
 ***************************************************************************/
unsigned int kerSysGetExtIntInfo(unsigned int irq)
{
    return extIntrInfo[irq-INTERRUPT_ID_EXTERNAL_0];
}
#if !defined(CONFIG_BCM960333)
EXPORT_SYMBOL(kerSysGetExtIntInfo);
#endif

void bcm_extirq_init(void)
{
#if defined(CONFIG_BCM96858) || \
    defined(CONFIG_BCM963158) || defined(CONFIG_BCM96846) || defined(CONFIG_BCM96856) || defined(CONFIG_BCM96878)
    // clear all external interrupts
    PERF->ExtIrqCtrl |=  0xff;
    PERF->ExtIrqCtrl &=  ~0xff;
    PERF->ExtIrqClear = 0xff;
    PERF->ExtIrqClear = 0x0;
#endif
    init_ext_irq_info();
    return;
}
EXPORT_SYMBOL(bcm_extirq_init);



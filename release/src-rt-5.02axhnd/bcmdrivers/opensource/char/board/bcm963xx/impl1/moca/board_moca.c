/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
* 
*    Copyright (c) 2016 Broadcom 
*    All Rights Reserved
* 
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License, version 2, as published by
* the Free Software Foundation (the "GPL").
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* 
* A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
* writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
* Boston, MA 02111-1307, USA.
* 
* :> 
*/

#include <linux/kernel.h>

#include <bcmtypes.h>
#include <bcm_map_part.h>
#include <board.h>
#include <boardparms.h>
#include <bcm_intr.h>
#include <shared_utils.h>
#include <bcm_pinmux.h>

#include "board_moca.h"
#include <bcm_extirq.h>
#include "../board_wl.h"

extern unsigned short resetBtn_gpio;

static DEFINE_SPINLOCK(mocaint_spinlock);
static MOCA_INTR_ARG mocaIntrArg[BP_MOCA_MAX_NUM];
static BP_MOCA_INFO mocaInfo[BP_MOCA_MAX_NUM];
static int mocaChipNum = BP_MOCA_MAX_NUM;

static void kerSysMocaHostIntrDisableCount(int dev, int update_count);
static irqreturn_t mocaHost_isr(int irq, void *dev_id);

int board_mocaPreInit(void)
{
    if( BpGetMocaInfo(mocaInfo, &mocaChipNum) == BP_SUCCESS )
        return mocaChipNum;
    else
    {
        mocaChipNum = 0;
        return 0;
    }
}

int boardGetMocaDevNum(void)
{
    return mocaChipNum;
}

PBP_MOCA_INFO boardGetMocaInfo(int dev)
{
    if( dev >= mocaChipNum)
        return NULL;
    else
        return &mocaInfo[dev];
}

void kerSysMocaHostIntrReset(int dev)
{
    PMOCA_INTR_ARG pMocaInt;
    unsigned long flags;

    if( dev >=  mocaChipNum )
    {
        printk("kerSysMocaHostIntrReset: Error, invalid dev %d\n", dev);
        return;
    }

    spin_lock_irqsave(&mocaint_spinlock, flags);
    pMocaInt = &mocaIntrArg[dev];
    atomic_set(&pMocaInt->disableCount, 0);
    spin_unlock_irqrestore(&mocaint_spinlock, flags);
}

void kerSysRegisterMocaHostIntrCallback(MocaHostIntrCallback callback, void * userArg, int dev)
{
    int ext_irq_idx;
    unsigned short  mocaHost_irq;
    PBP_MOCA_INFO  pMocaInfo;
    PMOCA_INTR_ARG pMocaInt;
    unsigned long flags;

    if( dev >=  mocaChipNum )
    {
        printk("kerSysRegisterMocaHostIntrCallback: Error, invalid dev %d\n", dev);
        return;
    }

    pMocaInfo = &mocaInfo[dev];
    mocaHost_irq = pMocaInfo->intr[BP_MOCA_HOST_INTR_IDX];
    if( mocaHost_irq == BP_NOT_DEFINED )
    {
        printk("kerSysRegisterMocaHostIntrCallback: Error, no mocaHost_irq defined in boardparms\n");    
        return;
    }

    printk("kerSysRegisterMocaHostIntrCallback: mocaHost_irq = 0x%x, is_mocaHostIntr_shared=%d\n", mocaHost_irq, IsExtIntrShared(mocaHost_irq));

    ext_irq_idx = (mocaHost_irq&~BP_EXT_INTR_FLAGS_MASK)-BP_EXT_INTR_0;
    if (!IsExtIntrConflict(extIntrInfo[ext_irq_idx]))
    {
        pMocaInt = &mocaIntrArg[dev];
        pMocaInt->dev = dev;
        pMocaInt->intrGpio = -1;
        pMocaInt->userArg = userArg;
        pMocaInt->mocaCallback = callback;
        if (IsExtIntrShared(mocaHost_irq))
        {
            /* get the gpio and make it input dir */
            unsigned short gpio;
            if( (gpio = pMocaInfo->intrGpio[BP_MOCA_HOST_INTR_IDX]) != BP_NOT_DEFINED )
            {
                gpio &= BP_GPIO_NUM_MASK;
                printk("MoCA Interrupt gpio is %d\n", gpio);
                kerSysSetGpioDirInput(gpio);
                pMocaInt->intrGpio = gpio;
            }
            else
            {
                printk("MoCA interrupt gpio definition not found \n");
            }
        }

        spin_lock_irqsave(&mocaint_spinlock, flags);
        atomic_set(&pMocaInt->disableCount, 0);
        pMocaInt->irq = map_external_irq(mocaHost_irq);
        spin_unlock_irqrestore(&mocaint_spinlock, flags);

        BcmHalExternalIrqMask(pMocaInt->irq);
        BcmHalExternalIrqClear(pMocaInt->irq);
        BcmHalMapInterrupt((FN_HANDLER)mocaHost_isr, (void*)pMocaInt, pMocaInt->irq);

        BcmHalExternalIrqUnmask(pMocaInt->irq);
        kerSysMocaHostIntrDisableCount(dev, 0);
    }
}

void kerSysMocaHostIntrEnable(int dev)
{
    PMOCA_INTR_ARG  pMocaInt;
    unsigned long flags;

    spin_lock_irqsave(&mocaint_spinlock, flags);
    if( dev <  mocaChipNum )
    {
        pMocaInt = &mocaIntrArg[dev];

        if (atomic_dec_return(&pMocaInt->disableCount) <= 0)
        {
#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
           BcmHalInterruptEnable(pMocaInt->irq);
#else
           BcmHalExternalIrqUnmask(pMocaInt->irq);
#endif
        }
    }
    spin_unlock_irqrestore(&mocaint_spinlock, flags);
}

static void kerSysMocaHostIntrDisableCount(int dev, int update_count)
{
    PMOCA_INTR_ARG  pMocaInt;
    int i;
    unsigned long flags;

    spin_lock_irqsave(&mocaint_spinlock, flags);
    if( dev <  mocaChipNum )
    {
        pMocaInt = &mocaIntrArg[dev];

        if (update_count)
        atomic_inc(&pMocaInt->disableCount);

        for (i=0; i<BP_MOCA_MAX_NUM; i++)
        {
            if ((i != dev) &&
                (mocaIntrArg[i].irq == pMocaInt->irq) &&
                (atomic_read(&mocaIntrArg[i].disableCount) <= 0))
            {
                // Don't disable this interrupt. It's shared and
                // the other MoCA interface still needs it. 
                spin_unlock_irqrestore(&mocaint_spinlock, flags);
                return;
            }
        }

#if !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
        BcmHalInterruptDisable(pMocaInt->irq);
#else
        BcmHalExternalIrqMask(pMocaInt->irq);
#endif
    }
    spin_unlock_irqrestore(&mocaint_spinlock, flags);
}

void kerSysMocaHostIntrDisable(int dev)
{
   kerSysMocaHostIntrDisableCount(dev, 1);
}

static irqreturn_t mocaHost_isr(int irq, void *dev_id)
{
    PMOCA_INTR_ARG pMocaIntrArg = (PMOCA_INTR_ARG)dev_id;
    int isOurs = 1;
    PBP_MOCA_INFO pMocaInfo;
    int ext_irq_idx = 0, value=0, valueReset = 0, valueMocaW = 0;
    unsigned short gpio;

    ext_irq_idx = irq - INTERRUPT_ID_EXTERNAL_0;

    /* When MoCA and SES button share the interrupt, the MoCA handler must be called
       so that the interrupt is re-enabled */
#if defined (WIRELESS)
    if (IsExtIntrShared(extIntrInfo[ext_irq_idx]) && (irq != sesBtn_getIrq()))
#else
    if (IsExtIntrShared(extIntrInfo[ext_irq_idx]))
#endif
    {
        if( pMocaIntrArg->intrGpio != -1 )
        {
            value = kerSysGetGpioValue(pMocaIntrArg->intrGpio);
            if( (IsExtIntrTypeActHigh(extIntrInfo[ext_irq_idx]) && value) || (IsExtIntrTypeActLow(extIntrInfo[ext_irq_idx]) && !value) )
                isOurs = 1;
            else
                isOurs = 0;
        }
        else
        {
            /* for BHR board, the L_HOST_INTR does not have gpio pin. this really sucks! have to check all other interrupt sharing gpio status,
             * if they are not triggering, then it is L_HOST_INTR.  next rev of the board will add gpio for L_HOST_INTR. in the future, all the
             * shared interrupt will have a dedicated gpio pin.
             */
            if( resetBtn_gpio != BP_NOT_DEFINED )
                valueReset = kerSysGetGpioValue(resetBtn_gpio);

               pMocaInfo = &mocaInfo[BP_MOCA_TYPE_WAN];
            if( (gpio = pMocaInfo->intrGpio[BP_MOCA_HOST_INTR_IDX]) != BP_NOT_DEFINED )
                valueMocaW = kerSysGetGpioValue(gpio);

            if( IsExtIntrTypeActHigh(extIntrInfo[ext_irq_idx]) )
            {
                if( (value = (valueReset|valueMocaW)) )
                    isOurs = 0;
            }
            else
            {
                if( (value = (valueReset&valueMocaW)) == 0 )
                    isOurs = 0;
            }

            //printk("BHR board moca_l interrupt: reset %d:%d, ses %d:%d, moca_w %d:%d, isours %d\n", resetBtn_gpio, valueReset,
            //    sesBtn_gpio, valueSes, gpio&BP_GPIO_NUM_MASK, valueMocaW, isOurs);
        }
    }
    if (isOurs)
    {
       if (atomic_read(&pMocaIntrArg->disableCount) <= 0)
       {
          pMocaIntrArg->mocaCallback(irq, pMocaIntrArg->userArg);
#if defined(CONFIG_BCM94908) || defined(CONFIG_BCM963158) || defined(CONFIG_BCM96858)
          BcmHalExternalIrqClear(irq);
#endif
#if defined(CONFIG_BCM96858)          
          BcmHalExternalIrqUnmask(irq);
#endif          
          return IRQ_HANDLED;
       }
    }
#if defined(CONFIG_BCM96858)
    BcmHalExternalIrqUnmask(irq);
#endif

    return IRQ_NONE;
}

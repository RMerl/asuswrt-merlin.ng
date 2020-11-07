#ifndef __FAPIRQ_H_INCLUDED__
#define __FAPIRQ_H_INCLUDED__

/*
<:copyright-BRCM:2009:DUAL/GPL:standard

   Copyright (c) 2009 Broadcom 
   All Rights Reserved

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed
to you under the terms of the GNU General Public License version 2
(the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
with the following added to such license:

   As a special exception, the copyright holders of this software give
   you permission to link this software with independent modules, and
   to copy and distribute the resulting executable under terms of your
   choice, provided that you also meet, for each linked independent
   module, the terms and conditions of the license of that module.
   An independent module is a module which is not derived from this
   software.  The special exception does not apply to any modifications
   of the software.

Not withstanding the above, under no circumstances may you combine
this software in any way with any other Broadcom software provided
under a license other than the GPL, without Broadcom's express prior
written consent.

:>
*/

/*
 *******************************************************************************
 * File Name  : fap4ke_irq.h
 *
 * Description: This file contains ...
 *
 *******************************************************************************
 */

#define FAP4KE_IRQ_HANDLERS_MAX 8

#define fap4keIrq_enable(irq)    _fap4keIrq_enable(irq)

#define fap4keIrq_disable(irq)   _fap4keIrq_disable(irq)

#define fap4keIrq_register(_handler, _arg, irq) \
    __fap4keIrq_register(_handler, _arg, irq, #_handler)

#define fap4keIrq_unregister(_handler, _arg, irq) \
    __fap4keIrq_unregister(_handler, _arg, irq, #_handler)

typedef enum {                     /* Usage (see fap_hw.h for register definitions): */
    FAP4KE_IRQ_GROUP_FAP,          /* For interrupts in the irq_4ke_status register */
    FAP4KE_IRQ_GROUP_CHIP,         /* For interrupts in the chipIrqStatus register */
    FAP4KE_IRQ_GROUP_CHIP_EXTRA,   /* For interrupts in the extraChipIrqStatus register 
                                     (or fap1IrqMaskLo register for 963268) */
    FAP4KE_IRQ_GROUP_CHIP_EXTRA2,  /* For interrupts in the extra2ChipIrqStatus register */
    FAP4KE_IRQ_GROUP_MAX
} fap4keIrq_irqGroup_t;

typedef enum {
    FAP4KE_IRQ_ENET_RX_0 = 0,
    FAP4KE_IRQ_ENET_RX_1,
    FAP4KE_IRQ_ENET_RX_2,
    FAP4KE_IRQ_ENET_RX_3,
    FAP4KE_IRQ_ENET_RX_ALL,
    FAP4KE_IRQ_DQM,
    FAP4KE_IRQ_SAR_DMA_0,
    FAP4KE_IRQ_SAR_DMA_1,
    FAP4KE_IRQ_SAR_DMA_2,
    FAP4KE_IRQ_SAR_DMA_3,
    FAP4KE_IRQ_SAR_ALL,
    FAP4KE_IRQ_GENERAL_PURPOSE_INPUT,
    FAP4KE_IRQ_TIMER_0,
    FAP4KE_IRQ_TIMER_1,
#if defined (CONFIG_BCM_GMAC)
    FAP4KE_IRQ_GMAC_RX_0,
#endif
    FAP4KE_IRQ_MAILBOX,
    FAP4KE_IRQ_MAX,
} fap4keIrqs;


typedef fapRet(*fap4keIrq_handler_t)(uint32 intStatus, uint32 arg);

typedef struct
{
    fap4keIrq_handler_t handler;
    uint32 arg;
    fap4keIrq_irqGroup_t irqGroup;
    volatile uint32 *irqAddr;
    uint32 irqMask;
    const char *name;
    uint32 count;
} fap4keIrq_handlerInfo_t;


void fap4keIrq_init(void);

fapRet __fap4keIrq_register(    fap4keIrq_handler_t handler, 
                                uint32 arg, 
                                fap4keIrqs irq, 
                                const char *name);
fapRet __fap4keIrq_unregister(  fap4keIrq_handler_t handler, 
                                uint32 arg, 
                                fap4keIrqs irq, 
                                const char *name);


void _fap4keIrq_enable(fap4keIrqs irq);

void _fap4keIrq_disable(fap4keIrqs irq);

void fap4keIrq_mainHandler(void);

void printIrqStats(void);

#endif  /* defined(__FAPIRQ_H_INCLUDED__) */

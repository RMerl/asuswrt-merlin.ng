#ifndef __FAP_LOCAL_H_INCLUDED__
#define __FAP_LOCAL_H_INCLUDED__

/*
 <:copyright-BRCM:2007:DUAL/GPL:standard
 
    Copyright (c) 2007 Broadcom 
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
 * File Name  : fap_local.h
 *
 * Description: This file contains assorted definitions and prototypes for the
 *              Host MIPs side of the FAP interface for the 63268.
 *
 *******************************************************************************
 */

#include <linux/smp.h> /*To avoid warnings in r4kcache*/
#include <asm/io.h>
#include <asm/addrspace.h>
#include <asm/cpu-info.h>
#include <asm/r4kcache.h>
#include <linux/types.h>

#define FAP_IDX_FMT "[FAP%d] "

static inline void fap_cacheInvFlush(void *start, void *end, int flush)
{
    uint32_t a, e;
    uint32_t dcLsize = current_cpu_data.dcache.linesz;

    a = (uint32_t)start & ~(dcLsize - 1);
    e = (uint32_t)end & ~(dcLsize - 1);

    while (1)
    {
        if (flush)
            flush_dcache_line(a);
        else
            invalidate_dcache_line(a);

        if (a == e)
            break;

        a += dcLsize;
    }
}

/* Prototypes: fapHwInit.c */
int fap_init4ke(uint32 fapIdx);
void fap_enable4ke(uint32 fapIdx);

/* Prototypes: fap_interrupt.c */
int fapIrq_init(uint32 fapIdx, uint32 registerIrqs);

/******************************************************************************
 * fapIrq_enable: MUST BE CALLED WITH INTERRUPTS DISABLED!
 ******************************************************************************/
#define fapIrq_enable(fapIdx, _irqMask)                                 \
    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->irq_mips_mask,                \
                    (FAP_HOST_REG_RD(hostRegCntrl(fapIdx)->irq_mips_mask) | (_irqMask)))

/******************************************************************************
 * fapIrq_disable: MUST BE CALLED WITH INTERRUPTS DISABLED!
 ******************************************************************************/
#define fapIrq_disable(fapIdx, _irqMask)                                \
    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->irq_mips_mask,                \
                    (FAP_HOST_REG_RD(hostRegCntrl(fapIdx)->irq_mips_mask) & ~(_irqMask)))

/* Prototypes: fap_mailBox.c */
void fapMailBox_hostInit(uint32 fapIdx);

/* Prototyps: fapDqm.c */
void fapDqm_hostInit(void);

extern void fap_Updatejiffies(uint32 fapIdx);

#endif  /* defined(__FAP_LOCAL_H_INCLUDED__) */

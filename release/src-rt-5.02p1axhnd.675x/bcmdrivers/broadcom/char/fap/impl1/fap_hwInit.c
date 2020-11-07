/*
<:copyright-BRCM:2012:proprietary:standard

   Copyright (c) 2012 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>
*/
/*
 *******************************************************************************
 * File Name  : fap_hwInit.c
 *
 * Description: This file contains functions for the FAP to initialize itself
 * and the 4ke firmware
 *
 *******************************************************************************
 */

#include "fap_task.h"
#include <linux/version.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/capability.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/pagemap.h>
#include <asm/uaccess.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/list.h>
#include <linux/if.h>
#include <linux/pci.h>
#include <linux/ctype.h>
#include <linux/proc_fs.h>
#include <linux/smp.h>
#include <linux/delay.h>

#include "bcm_map.h"
#include <linux/bcm_log.h>
#include "fap_hw.h"
#include "fap_local.h"
#include "fap4ke_memory.h"

/*
    During the compile of the 4ke, a file called bcmfap4ke.h is
    generated which contains the 4ke code offsets of all the
    shared functions.  Once this file is generated, the elf file
    is stripped (so no further code offsets can be figured out).

    During the compile of the host, bcmfap4keBinaray.h is generated
    from the elf file.  This file contains all of the loadable
    segments.
*/

#include "bcmfap4ke.h"
#include "bcmfap4keBinary.h"

#define FAP_4KE_VECTOR_SPACE_SIZE   2048

/* The offset in bytes from the start of the 4ke code that the deret
   code is written.  Note: this must match the value in the linker
   script */
#define FAP4KE_DERET_OFFSET         0x480

/* The value written to FAP4KE_DERET_OFFSET */
#define FAP4KE_DERET_CODE           0x4200001f


#define CACHE_TO_NONCACHE(x)    KSEG1ADDR(x)
//#define NONCACHE_TO_CACHE(x)    KSEG0ADDR(x)


Fap_FapInfo gFap[NUM_FAPS];
Fap_FapSharedInfo gFaps;

extern uint32 * traps_fap0DbgVals;
extern uint32 * traps_fap1DbgVals;

static inline uint32 getMask32(uint32 num)
{
    if (num == 0)
        return 0;
    return ((1 << (32 - __builtin_clzl(num))) - 1);
}


/* setup4keMemory: This allocates all memory required by the 4ke.
   Included in this is memory for the data, and for the
   codespace.

   This function assumes that the memory will be placed within a
   single area in DDR, and that both FAP images will be identical
   when they start.  This area needs to be rewritten if either of
   these assumptions are broken.

   This function modifies gFapMem[fapIdx] with the loaded addresses.

   fapIdx:      The FAP core index (0-based)

 */
static __init fapRet setup4keMemory( int fapIdx )
{
    fapRet ret = FAP_SUCCESS;
    volatile uint32 *p4keMem;   /* The host address of the allocated memory that the 4ke
                                   actually runs from */
    uint32 codeSize = 0;        /* number of bytes required in the 4ke image */
    uint32 maxAddr;             /* temp varialbe used to calculate code size */
    int sectionIdx;
    uint32 roundedSize;         /* the size of the 4ke image rounded up to the next power of two */
    struct kmem_cache *pCache;  /* The kernel cache.  Because it is never freed, it can be stored
                                   as a local variable, and forgotten about when the function
                                   exits */

    /* ---------- 1. organize the target memory */

    /* ----------   1.1 get allocate size */

    for (sectionIdx = 0; sectionIdx < NUM_4KE_SECTIONS; sectionIdx++)
    {
        maxAddr = gSections4ke[sectionIdx].offset + gSections4ke[sectionIdx].size + 1;
        maxAddr &= 0x1FFFFFFF;
        if (codeSize < maxAddr)
        {
            codeSize = maxAddr;
        }
    }

    /* round up to nearest word: */
    codeSize = (codeSize + 4-1) & ~0x00000003;

    /* memory allocated must be aligned to a power-of-two boundary relative to its size.
       Thus, if the size is 0x00001234, then the memory must be aligned to a 0x00002000
       boundary...  This is a hardware restriction.
    */

    roundedSize = getMask32(codeSize) + 1;

    /* ----------   1.2 allocate memory */

    sprintf(gFap[fapIdx].cacheName, "fap%d_4ke_cache", fapIdx);

    BCM_LOG_DEBUG(BCM_LOG_ID_FAP, "About to create %s: size=0x%x, alignment=0x%x", gFap[fapIdx].cacheName, codeSize, roundedSize);

    pCache = kmem_cache_create(
        gFap[fapIdx].cacheName,
        codeSize,
        roundedSize,
        0,
        NULL );

    if (0 == pCache)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Error could not create cache for 4ke[%d] (size = 0x%x, alignment = 0x%x)",
                      fapIdx, codeSize, roundedSize);
        return FAP_ERROR;
    }

    p4keMem = kmem_cache_alloc(pCache, GFP_KERNEL);

    if (NULL == p4keMem)
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Error could not allocate memory for 4ke[%d] (roundedsize = 0x%x, codesize = 0x%x)",
                      fapIdx, roundedSize, codeSize);
        return FAP_ERROR;
    }

    if ((uint32)p4keMem & (uint32)(roundedSize - 1))
    {
        BCM_LOG_ERROR(BCM_LOG_ID_FAP, "Error 4ke[%d] memory not aligned (addr = 0x%p, roundedsize = 0x%x, codesize = 0x%x)",
                      fapIdx, p4keMem, roundedSize, codeSize);
        return FAP_ERROR;
    }


    /* ----------   1.3 flush the cache */
    fap_cacheInvFlush((void *)((uint32)(p4keMem)),
                      (void *)((uint32)(p4keMem) + codeSize - 1),
                      0);

    p4keMem = ( volatile uint32 *)CACHE_TO_NONCACHE(p4keMem);
    
    BCM_LOG_DEBUG(BCM_LOG_ID_FAP, "Allocated 4ke[%d] memory: p4keMem=0x%p, codeSize=0x%x, roundedSize=0x%x",
                  fapIdx, p4keMem, codeSize, roundedSize);

    /* ----------   1.4. set global variables */

    gFap[fapIdx].blockHostAddr[0] = (uint32)p4keMem;
    gFap[fapIdx].blockSize[0] = codeSize;
    gFap[fapIdx].blockMask[0] = roundedSize-1;
    gFap[fapIdx].blockFapAddr[0] = 0x00000000;

    gFap[fapIdx].blockHostAddr[1] = (uint32)p4keMem;
    gFap[fapIdx].blockSize[1] = codeSize;
    gFap[fapIdx].blockMask[1] = roundedSize-1;
    gFap[fapIdx].blockFapAddr[1] = (COMPILE_BASE_4KE & 0x1FFFFFFF);

    gFap[fapIdx].pBase = PHYS_FAP_BASES[fapIdx];
    gFap[fapIdx].mainOffset = 0;    // offset into memory block where main is
    gFap[fapIdx].mainBlock = 0;     // block in which main is
    gFap[fapIdx].pPrintBuffer = (char *)(gFap[fapIdx].blockHostAddr[0] + FAP4KE_MAILBOX_OFFSET);
    gFap[fapIdx].pSdram = (fap4keSdram_alloc_t *)(gFap[fapIdx].blockHostAddr[0] + FAP4KE_SDRAM_OFFSET);
    gFap[fapIdx].pPsm = (void *)(FAP_HOST_PSM_BASES[fapIdx]);
    gFap[fapIdx].pQsm = (void *)(FAP_HOST_QSM_BASES[fapIdx]);

    /* Save debug value pointers so they are printed on a kernel panic */
    if (fapIdx == 0)
        traps_fap0DbgVals = pHostFapSdram(fapIdx)->dbgVals;
    else
        traps_fap1DbgVals = pHostFapSdram(fapIdx)->dbgVals;
    
    BCM_LOG_DEBUG(BCM_LOG_ID_FAP, "Fap4ke[%d] Shared Mem: BaseAddr=0x%x, Sdram=%p, PrintBuffer=%p",
                  fapIdx, gFap[fapIdx].blockHostAddr[0], gFap[fapIdx].pSdram, gFap[fapIdx].pPrintBuffer);

    /* ---------- 2. copy to 4ke memory */
    for (sectionIdx = 0; sectionIdx < NUM_4KE_SECTIONS; sectionIdx++)
    {
        memcpy( ((char *)p4keMem + gSections4ke[sectionIdx].offset), gSections4ke[sectionIdx].pData, gSections4ke[sectionIdx].size );
    }

    BCM_LOG_DEBUG(BCM_LOG_ID_FAP, "Uploaded 4ke code to address %x, size 0x%x",
                  gFap[fapIdx].blockHostAddr[0], (unsigned int)codeSize);

    /* ---------- 3 Install a DERET at offset 0x480 for the debug exception vector */

    /* Install a DERET at offset 0x480 for the debug exception vector.
       The opcode for a DERET instruction is 0x4200001f */
    {
        uint32 deretOpcode = FAP4KE_DERET_CODE;
        ((uint32 *)gFap[fapIdx].blockHostAddr[0])[FAP4KE_DERET_OFFSET/sizeof(uint32)] = deretOpcode;
    }
    return ret;
}


void fap_softReset(uint32 fapIdx)
{
    uint32 val;
    int i;

    BCM_ASSERT(isValidFapIdx(fapIdx));

    val = FAP_HOST_REG_RD(PERF->softResetB);

    val &= ~SOFT_RST_FAPS[fapIdx];
    FAP_HOST_REG_WR(PERF->softResetB, val);

    for(i=0; i<20; ++i)
    {
        udelay(990);
    }

    val |= SOFT_RST_FAPS[fapIdx];
    FAP_HOST_REG_WR(PERF->softResetB, val);
}

int __init fap_init4ke(uint32 fapIdx)
{
    fapRet ret = FAP_SUCCESS;
    uint32 val;

    /* FAP_TBD: We will have two clocks to enable here for the 63268 */

#if defined(CONFIG_BCM963268)
    /* turn MISC_IDDQ_CTRL_FAP */
    val = FAP_HOST_REG_RD(MISC->miscIddqCtrl);
    val &= ~MISC_IDDQ_CTRL_FAP;
    FAP_HOST_REG_WR(MISC->miscIddqCtrl,val);

    val = FAP_HOST_REG_RD(TIMER->ClkRstCtl);
    val |= FAP1_PLL_CLKEN | FAP2_PLL_CLKEN;
    FAP_HOST_REG_WR(TIMER->ClkRstCtl,val);
#endif /* CONFIG_BCM963268 || CONFIG_BCM96828 || CONFIG_BCM96818 */

    /* Enable FAP clock */
    val = FAP_HOST_REG_RD(PERF->blkEnables);
    val |= FAP_CLK_ENS[fapIdx];
    FAP_HOST_REG_WR(PERF->blkEnables, val);



#ifdef USE_SMISBUS
    /* Enable SMISBUS */
    printk("Enabling SMISBUS PHYS_FAP_BASE[%u] is 0x%x\n", fapIdx, PHYS_FAP_BASES[fapIdx]);
    *mips_smisb_ctrl = FAP_SMISB_CTRL_VAL;
#endif
    /* Soft-reset the FAP */
    fap_softReset(fapIdx);

    printk("FAP Soft Reset Done\n");

    /* Put the 4ke into reset */
    val = FAP_HOST_REG_RD(hostRegCntrl(fapIdx)->soft_resets);
    val |= SOFT_RESET_4KE;
    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->soft_resets, val);

    printk("4ke Reset Done\n");

    /* Setup the 4ke memory */
    ret = setup4keMemory(fapIdx);
    if(ret == FAP_ERROR)
    {
        goto out;
    }

    /* Flush 4ke SDRAM allocations */
    fap_cacheInvFlush((void *)((uint32)(gFap[fapIdx].pSdram)),
                      (void *)((uint32)(gFap[fapIdx].pSdram) + sizeof(fap4keSdram_alloc_t)),
                      1);

    /* Pass the Host side fap4keSdram_alloc_t pointer to the FAPs */
    pHostPsmGbl(fapIdx)->pHostSdram_p = pHostFapSdram(fapIdx);

    /* Set the bus address re-direction registers. These registers are used
       by the bus logic to redirect blocks of mips addresses to other areas */
    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->addr1_mask, ~((uint32)(gFap[fapIdx].blockMask[0]) | 0xA0000000));
    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->addr1_base_in, gFap[fapIdx].blockFapAddr[0]);
    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->addr1_base_out, ((uint32)gFap[fapIdx].blockHostAddr[0]) & 0x1FFFFFFF);
    BCM_LOG_DEBUG(BCM_LOG_ID_FAP, "address translation 0: mask: 0x%08x, base_in: 0x%08x, base_out: 0x%08x",
            FAP_HOST_REG_RD(hostRegCntrl(fapIdx)->addr1_mask),
            FAP_HOST_REG_RD(hostRegCntrl(fapIdx)->addr1_base_in),
            FAP_HOST_REG_RD(hostRegCntrl(fapIdx)->addr1_base_out));

    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->addr2_mask, ~((uint32)(gFap[fapIdx].blockMask[1]) | 0xA0000000));
    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->addr2_base_in, gFap[fapIdx].blockFapAddr[1]);
    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->addr2_base_out, ((uint32)gFap[fapIdx].blockHostAddr[1]) & 0x1FFFFFFF);
    BCM_LOG_DEBUG(BCM_LOG_ID_FAP, "address translation 1: mask: 0x%08x, base_in: 0x%08x, base_out: 0x%08x",
            FAP_HOST_REG_RD(hostRegCntrl(fapIdx)->addr2_mask),
            FAP_HOST_REG_RD(hostRegCntrl(fapIdx)->addr2_base_in),
            FAP_HOST_REG_RD(hostRegCntrl(fapIdx)->addr2_base_out));

    /* Clear the host mailbox status */
    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->gp_status, IRQ_FAP_GP_MBOX_IN);

    /* Send a message to the 4ke telling it where to jump to it's main program */
    BCM_LOG_NOTICE(BCM_LOG_ID_FAP, "fap4ke_main : 0x%08X", (FAP4KE_ENTRY_OFFSET + COMPILE_BASE_4KE) | 0x80000000);
    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->host_mbox_out, (FAP4KE_ENTRY_OFFSET + COMPILE_BASE_4KE) | 0x80000000);
out:
    return ret;
}

void fap_enable4ke(uint32 fapIdx)
{
    uint32 val;

    BCM_ASSERT(isValidFapIdx(fapIdx));
   // Release 4KE reset
    val = FAP_HOST_REG_RD(hostRegCntrl(fapIdx)->soft_resets);
    val &= ~SOFT_RESET_4KE;
    FAP_HOST_REG_WR(hostRegCntrl(fapIdx)->soft_resets, val);

    BCM_LOG_NOTICE(BCM_LOG_ID_FAP, "Releasing FAP 4ke[%d] Reset", fapIdx);
}


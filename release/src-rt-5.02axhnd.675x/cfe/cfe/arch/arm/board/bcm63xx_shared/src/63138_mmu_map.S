/*  *********************************************************************
    *
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
    ********************************************************************* */
	
#include <arm.h>
#include <armmacros.h>
#include "cpu_config.h"
#include "bsp_config.h"
#include "mmu_map.h"
#include "bcm_map.h"

#define L1TABLE_PA (text_start-L1TABLE_SIZE)

FUNC(board_mmu_map_scn)
	mov	r9, lr

	/* clear mmu table first, total 16K mmu table */
	MMU_INIT_TABLE(L1TABLE_PA, L1TABLE_SIZE)

#if defined(CFG_RAMAPP)	

#if !defined(IKOS_NO_DDRINIT)
	/* read ddr memory size in MEMC */
	ldr	r2, =MEMC_BASE 
	ldr	r2, [r2, #MEMC_GLB_GCFG]
	and	r2, #MEMC_GLB_GCFG_DRAM_SIZE1_MASK
	mov	r5, #1
	lsl	r5, r5, r2
	mov	r10, r5

	_MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x0, r5, SECTION_ATTR_CACHED_MEM, 0x0)
	/* virtual address 0x4000-0000 to 0x8000-0000 -> 0x0000-0000 to 0x4000-0000 physcial uncached*/
	mov	r5, r10
	_MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x40000000, r5, SECTION_ATTR_NONCACHED_MEM, 0x0)
#else
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x0, 1024, SECTION_ATTR_CACHED_MEM, 0x0)
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x40000000, 1024, SECTION_ATTR_NONCACHED_MEM, 0x0)
#endif

#else
	/* cfe rom DDR must be mapped as non-executable device to prevent speculative data and instruction fetch */	
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x0, 1024, SECTION_ATTR_DEVICE, 0x0)

	/* entry for onchip memory at 0x807x-xxxx */
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x80700000, 1, SECTION_ATTR_CACHED_MEM, 0x80700000)
	
	/* entry for the direct mapped SPI flash */
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0xffd00000, 1, SECTION_ATTR_DEVICE_EXEC, 0xffd00000)

	/* entry for the direct mapped NAND flash */
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0xffe00000, 1, SECTION_ATTR_DEVICE_EXEC, 0xffe00000)

#endif

/*63148-63138*/
	/* entry for APM at 0x801x-xxxx */
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x80100000, 1, SECTION_ATTR_DEVICE, 0x80100000)
	
	/* entry for RDP at 0x802x-xxxx */
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x80200000, 1, SECTION_ATTR_DEVICE, 0x80200000)

	/* entry for PMC at 0x804x-xxxx */
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x80400000, 1, SECTION_ATTR_DEVICE, 0x80400000)

	/* entry for MEMC, ARM UART at 0x800x-xxxx */
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x80000000, 1, SECTION_ATTR_DEVICE, 0x80000000)

	/* entry for PERIPHERAL at 0xfffx-xxxx */
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0xfff00000, 1, SECTION_ATTR_DEVICE, 0xfff00000)

	MMU_SET_TTBR(L1TABLE_PA)

	mov	lr,r9
	mov	pc,lr
END(board_mmu_map_scn)

/* *********************************************************************
    *  mmu_update_scn(uint* addr)
    *  
    *  Update MMU table for entries that does not need XIP support.
    *  A board mmu callback
    *	
    *  Input parameters: 
    *  	  none 
    *  	   
    *  Return value:
    *  	   nothing
    *
    *  Registers used:
    *  	   r0 - r11
    ********************************************************************* */

FUNC(board_mmu_update_scn)
	
	mov	r11, lr
#if !defined(CFG_RAMAPP)
	/* SPI NOR direct window. Add NX to avoid any speculative fetch  */
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0xffd00000, 0x1, SECTION_ATTR_DEVICE, 0xffd00000)
	/* delete NAND entry. Nand driver is used after XIP */
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0xffe00000, 0x1, SECTION_ATTR_INVALID, 0xffe00000)

	bl	armv7_l1cache_inval_i
	bl	armv7_l1cache_flush_d
	bl	mmu_invalidate_tlb
#endif
	mov	lr,r11
	mov	pc,lr
END(board_mmu_update_scn)
/**********************************************************************
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
	
#include "mmu_map.h"
#include "bcm_map.h"

#define L1TABLE_PA (text_start-L1TABLE_SIZE)


/* *********************************************************************
    *  mmu_map_scn(uint* addr)
    *  
    *  MAP MMU table for the 47189 board. A board mmu callback 
    *  
    *  Input parameters: 
    *  	  none 
    *  	   
    *  Return value:
    *  	   nothing
    *
    *  Registers used:
    *  	   r0 - r5
    ********************************************************************* */

FUNC(board_mmu_map_scn)
	mov	r9,lr
		
	MMU_INIT_TABLE(L1TABLE_PA, L1TABLE_SIZE)

	/* get actual memsize in mega bytes */
	ldr	r0, =MEM_SIZE_REG
	ldr	r5, [r0]		/* memory size in Mbytes */
#if defined(CFG_RAMAPP)	
	cmp	r5, #0
	bne	1f
	mov	r5, #128		/* set default size 128MB for old version cferom */
	str	r5, [r0]
1:
	mov	r10, r5
	_MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x00000000, r5, SECTION_ATTR_CACHED_MEM, 0x00000000)
	mov	r5, r10
	_MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x40000000, r5, SECTION_ATTR_NONCACHED_MEM, 0x00000000)
#else
	/* Chip support maximum 512MB DDR. But CFE rom only map 256MB to avoid conflict with SPI and NAND window. DDR test is done
	   before MMU is enabled */
	cmp	r5, #256
	blt	1f
	mov	r5, #256
1:	
	_MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x00000000, r5, SECTION_ATTR_NONCACHED_MEM, 0x00000000)
	/* 47189 Serial Flash 0x1cxx-xxxx. Map as executable for xip. Update after xip is done */
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x1c000000, 0x10, SECTION_ATTR_DEVICE_EXEC, 0x1c000000)
	/* 47189 NAND Flash 0x30xx-xxxx. Map as executable for xip. Update after xip is done */
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x30000000, 0x10, SECTION_ATTR_DEVICE_EXEC, 0x30000000)
#endif	

	/* 47189 memory mapped registers 0x18xx-xxxx */
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x18000000, 0x10, SECTION_ATTR_DEVICE, 0x18000000)
	
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
	/* 47189 Serial Flash 0x1cxx-xxxx. NX to avoid any speculative fetch  */
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x1c000000, 0x10, SECTION_ATTR_DEVICE, 0x1c000000)
	/* delete NAND entry. Nand driver is used after XIP */
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x30000000, 0x10, SECTION_ATTR_INVALID, 0x30000000)

	bl	armv7_l1cache_inval_i
	bl	armv7_l1cache_flush_d
	bl	mmu_invalidate_tlb
#endif
	mov	lr,r11
	mov	pc,lr
END(board_mmu_update_scn)
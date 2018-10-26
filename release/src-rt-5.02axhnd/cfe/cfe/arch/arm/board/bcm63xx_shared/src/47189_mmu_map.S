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
	/* 0x1C0E, normal memory cached */
	/* setup maximum 1GB for cfe rom alias test.For cfe ram, set the mmu memory size based on mem detection result */
	/* DDR memory 0x0000-0000 to 0x4000-0000 */
#if defined(CFG_RAMAPP)

#if !defined(IKOS_NO_DDRINIT)
	/* For 47189: Hardcode the memory size for now (128MB) 0x0000-0000 to 0x0800-0000*/
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x00000000, 128, (SECTION_OUTER_WBWA_INNER_WBWA|SECTION_AP|DESC_DOMAIN(0)), 0x00000000)
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x40000000, 128, (SECTION_OUTER_NC_INNER_NC|SECTION_AP|DESC_DOMAIN(0)), 0x00000000)
#else
	/* 0x1C0E, map normal memory uncached */
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x0, 1024, (SECTION_OUTER_NC_INNER_NC|SECTION_AP|DESC_DOMAIN(0)), 0x0)
	/* For 47189, this is 0x4000-0000 to 0x4800-0000 -> 0x0000-0000 to 0x0800-0000 - IK: IKOS is posmorterm; setting size to 128 */
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x40000000, 128, (SECTION_OUTER_NC_INNER_NC|SECTION_AP|DESC_DOMAIN(0)), 0x00000000)
#endif

#else
	/* cfe rom maps DDR as uncached because we need to alias test when init the ddr */ 
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x00000000, 1024, (SECTION_OUTER_NC_INNER_NC|SECTION_AP|DESC_DOMAIN(0)), 0x00000000)
	/* 47189 Serial Flash 0x1cxx-xxxx */
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x1c000000, 0x10, (SECTION_OUTER_NC_INNER_NC|SECTION_AP|DESC_DOMAIN(0)), 0x1c000000)
#endif	

	/* 47189 memory mapped registers 0x18xx-xxxx */
	MMU_MAP_SCN(L1TABLE_PA, (L1TABLE_SIZE/4), 0x18000000, 0x10, (SECTION_OUTER_NSD_INNER_NSD|SECTION_AP|DESC_DOMAIN(0)), 0x18000000)
	
	MMU_SET_TTBR(L1TABLE_PA)
	mov	lr,r9
	mov	pc,lr
END(board_mmu_map_scn)

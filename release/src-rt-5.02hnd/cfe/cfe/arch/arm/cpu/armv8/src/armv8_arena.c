/*  *********************************************************************
    *
    <:copyright-BRCM:2015:proprietary:standard
    
       Copyright (c) 2015 Broadcom 
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

#include "lib_types.h"
#include "lib_string.h"
#include "lib_queue.h"
#include "lib_malloc.h"
#include "lib_printf.h"
#include "lib_arena.h"

#include "cfe_error.h"

#include "cfe.h"
#include "cfe_mem.h"

#include "initdata.h"

#define _NOPROTOS_
#include "cfe_boot.h"
#undef _NOPROTOS_


/*  *********************************************************************
    *  Macros
    ********************************************************************* */

#define ARENA_RANGE(bottom,top,type) arena_markrange(&cfe_arena,(uint64_t)(bottom), \
                                     (uint64_t)(top)-(uint64_t)bottom+1,(type),NULL)

#define MEG	(1024*1024)
#define KB      1024
#define PAGESIZE 4096
#define CFE_BOOTAREA_SIZE (256*KB)
#define CFE_BOOTAREA_ADDR 0x20000000

/*  *********************************************************************
    *  Globals
    ********************************************************************* */

extern arena_t cfe_arena;

void armv8_arena_init(void);
void armv8_pagetable_init(uint64_t *ptaddr,unsigned int physaddr);


/*  *********************************************************************
    *  armv8_arena_init()
    *  
    *  Create the initial map of physical memory
    *
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

void armv8_arena_init(void)
{
    int64_t memleft;

    arena_init(&cfe_arena,0x0,0x100000000LL);	/* 2^32 physical bytes */

    /*
     * Mark the ranges from the SB1250's memory map
     */

    ARENA_RANGE(0x0000000000,0x000FFFFFFF,MEMTYPE_DRAM_NOTINSTALLED);

    /*
     * Now, fix up the map with what is known about *this* system.
     *
     * Do each 256MB chunk.
     */

    memleft = ((int64_t)mem_totalsize) << 10;

    arena_markrange(&cfe_arena,0x00000000,memleft,MEMTYPE_DRAM_AVAILABLE,NULL);

    /*
     * Do the boot ROM
     */

    arena_markrange(&cfe_arena,0xFFF00000,128*1024,MEMTYPE_BOOTROM,NULL);

}


/*  *********************************************************************
    *  ARMV8_PAGETABLE_INIT(ptaddr,physaddr)
    *  
    *  This routine constructs the page table.  256KB is mapped
    *  starting at physical address 'physaddr' - the resulting
    *  table entries are placed at 'ptaddr'
    *  
    *  Input parameters: 
    *  	   ptaddr - base of page table
    *  	   physaddr - starting physical addr of area to map
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

void armv8_pagetable_init(uint64_t *ptaddr,unsigned int physaddr)
{
	/* We do it in cfe_l1cache_on() */
}

/*  *********************************************************************
    *  SB1250 Board Support Package
    *  
    *  L1C initialization			File: bcmcore_l1cache.S
    *  
    *  This module contains code to initialize the CPU's caches
    *  
    *  Note: all the routines in this module rely on registers only,
    *        since DRAM may not be active yet.
    *
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *  
    *********************************************************************  
    *
    *  XX Copyright 2000,2001
    *  Broadcom Corporation. All rights reserved.
    *
    *  BROADCOM PROPRIETARY AND CONFIDENTIAL
    *  
    *  This software is furnished under license and may be used and 
    *  copied only in accordance with the license.
    ********************************************************************* */

#include "sbmips.h"
#include "bsp_config.h"

		.text

		.set push	
		.set mips32

/*  *********************************************************************
    *  Macros
    ********************************************************************* */

#define CP0_CFG_ISMSK      (0x7 << 22)
#define CP0_CFG_ISSHF      22
#define CP0_CFG_ILMSK      (0x7 << 19)
#define CP0_CFG_ILSHF      19
#define CP0_CFG_IAMSK      (0x7 << 16)
#define CP0_CFG_IASHF      16
#define CP0_CFG_DSMSK      (0x7 << 13)
#define CP0_CFG_DSSHF      13
#define CP0_CFG_DLMSK      (0x7 << 10)
#define CP0_CFG_DLSHF      10
#define CP0_CFG_DAMSK      (0x7 << 7)
#define CP0_CFG_DASHF      7

#define cacheop(kva, size, linesize, op) \
        .set    noreorder;       \
        addu    t1, kva, size;   \
        subu    t2, linesize, 1; \
        not     t2;              \
        and     t0, kva, t2;     \
        addu    t1, -1;          \
        and     t1, t2;          \
10:     cache   op, 0(t0);       \
        bne     t0, t1, 10b;     \
        addu    t0, linesize;    \
11:                              \
        .set    reorder

#define size_icache(size, linesize) \
        mfc0    t7, C0_CONFIG, 1;       \
        and     t0, t7, CP0_CFG_ILMSK;  \
        srl     t0, t0, CP0_CFG_ILSHF;  \
        move    linesize, zero;         \
        beq     t0, zero,1f;            \
        add     t0, 1;                  \
        li      linesize, 1;            \
        sll     linesize, t0;           \
1:      and     t0, t7, CP0_CFG_ISMSK;  \
        srl     t0, t0, CP0_CFG_ISSHF;  \
        li      size, 64;               \
        sll     size, t0;               \
        and     t0, t7, CP0_CFG_IAMSK;  \
        srl     t0, t0, CP0_CFG_IASHF;  \
        add     t0, 1;                  \
        mult    size, t0;               \
        mflo    size;                   \
        mult    size, linesize;         \
        mflo    size
        
#define size_dcache(size, linesize) \
        mfc0    t7, C0_CONFIG, 1;       \
        and     t0, t7, CP0_CFG_DLMSK;  \
        srl     t0, t0, CP0_CFG_DLSHF;  \
        move    linesize, zero;         \
        beq     t0, zero,1f;            \
        add     t0, 1;                  \
        li      linesize, 1;            \
        sll     linesize, t0;           \
1:      and     t0, t7, CP0_CFG_DSMSK;  \
        srl     t0, t0, CP0_CFG_DSSHF;  \
        li      size, 64;               \
        sll     size, t0;               \
        and     t0, t7, CP0_CFG_DAMSK;  \
        srl     t0, t0, CP0_CFG_DASHF;  \
        add     t0, 1;                  \
        mult    size, t0;               \
        mflo    size;                   \
        mult    size, linesize;         \
        mflo    size
        

/*  *********************************************************************
    *  BCMCORE_L1CACHE_INIT()
    *  
    *  Initialize the L1 Cache
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    *  
    *  Registers used:
    *  	   t0,t1,t2
    ********************************************************************* */

LEAF(bcmcore_l1cache_init)

        /* Check if cache is enabled. Exit if not */
        mfc0    t0, C0_CONFIG
        and     t0, 0x7
        beq     t0, K_CFG_K0COH_UNCACHED, 2f

        mtc0    zero, C0_TAGLO  # Initialize TAGLO register
        mtc0    zero, C0_TAGLO,1 # Initialize DataLo register

        li      a0, K0BASE      # Initialise primary instruction cache.
        size_icache(a1, a2)
        cacheop(a0, a1, a2, Index_Store_Tag_I)

        li      a0, K0BASE      # Initialise primary data cache.
        size_dcache(a1, a2)
        cacheop(a0, a1, a2, Index_Store_Tag_D)
2:
		jr	ra

END(bcmcore_l1cache_init)

/*  *********************************************************************
    *  BCMCORE_L1CACHE_INVAL_I()
    *  
    *  Invalidate the entire ICache
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    *  
    *  Registers used:
    *  	   t0,t1,t2
    ********************************************************************* */
LEAF(bcmcore_l1cache_inval_i)
		
        /* Check if cache is enabled. Exit if not */
        mfc0    t0, C0_CONFIG
        and     t0, 0x7
        beq     t0, K_CFG_K0COH_UNCACHED, 1f

        li      a0, K0BASE
        size_icache(a1, a2)
        cacheop(a0, a1, a2, Index_Invalidate_I)
1:
		j	ra

END(bcmcore_l1cache_inval_i)

/*  *********************************************************************
    *  BCMCORE_L1CACHE_FLUSH_D()
    *  
    *  Flush the entire DCache
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    *  
    *  Registers used:
    *  	   t0,t1,t2
    ********************************************************************* */
LEAF(bcmcore_l1cache_flush_d)

        /* Check if cache is enabled. Exit if not */
        mfc0    t0, C0_CONFIG
        and     t0, 0x7
        beq     t0, K_CFG_K0COH_UNCACHED, 3f

        li      a0, K0BASE
        size_dcache(a1, a2)

# before flushing cache clear tags pointing to flash memory to avoid writes into flash
        addu    t1, a0, a1
        subu    t2, a2, 1
        not     t2
        and     t0, a0, t2
        addu    t1, -1
        and     t1, t2
1:
        cache   Index_Load_Tag_D, 0(t0)
        nop
        nop
        nop
        nop
        nop
        nop
        mfc0    t2, C0_TAGLO         # Read TAGLO register
        and     t2, 0x1f000000       # check address
        li      t3, 0x1f000000
        bne     t2, t3, 2f
        mtc0    zero, C0_TAGLO
        cache   Index_Store_Tag_D, 0(t0)    # Reset tag for flash memory locations
2:
        .set noreorder;
        bne     t0, t1, 1b
        addu    t0, a2
        .set reorder

        cacheop(a0, a1, a2, Index_Writeback_Inv_D)
3:
		j	ra

END(bcmcore_l1cache_flush_d)

		.set pop

/*  *********************************************************************
    *  End
    ********************************************************************* */


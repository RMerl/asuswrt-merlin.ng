#*****************************************************************************
#
#    Copyright 2004  Broadcom Corporation
#    All Rights Reserved
#    No portions of this material may be reproduced in any form without the
#    written permission of:
#             Broadcom Corporation
#             16215 Alton Parkway
#             Irvine, California  92618
#    All information contained in this document is Broadcom Corporation
#    company private, proprietary, and trade secret.
#
#
#*****************************************************************************
#    Filename:       Cache.s
#    Author:         David Pullen
#    Date:           Oct 20, 2004
#    Description:    This file contains the code to initialize the caches
#                    on the 3368.
#
#*****************************************************************************

#if (TRIM_SUP==1)  || defined(IKOS_FAST_CACHE_INIT)

#include "sbmips.h"
#include "bsp_config.h"
#include "mipsmacros.h"

		.text

		.set push
		.set mips32

# This code is small enough to place it before the exception vectors (but
# after the other startup code).
#.section .text._start,"ax"

#.globl  InvalidateAndEnableIcache
#.globl  InvalidateAndEnableDcache
#.globl  InvalidateAndEnableIDcache

# UNFINISHED - include 3368_map.h when it is available.  Until then, define
# things here.
.equ ICACHE_INDEX_INVALIDATE, 0
.equ ICACHE_INDEX_STORE_TAG,  8
.equ DCACHE_INDEX_INVALIDATE, 1
.equ DCACHE_INDEX_STORE_TAG,  9

#************************************************************************
#*     InvalidateAndEnableIcache_GGM
#*
#* Turns the ICache on (for the current TP), calculates the size of the
#* ICache, and then proceeds to invalidate every line of every set.  It
#* does this from a cached instruction address space, but will return
#* from the function in the cached/uncached space that the function was
#* called from.
#*
#* NOTE:  This function assumes that the TagLo and DataLo registers were
#*        previously set to 0 (in Init3368Hw.s.
#*
#* Parameters:  None.
#*
#* Modifies:  Registers t0-t2, and of course CP0 Reg 22, Sel 0 to enable
#*            the ICache.
#************************************************************************
#.ent    InvalidateAndEnableIcache
#InvalidateAndEnableIcache:
/*LEAF(InvalidateAndEnableIcache)
	.set	noreorder

    # We must be running in uncached space at startup (i.e. the ROM must be
    # linked in uncached space), so there is no need to jump to uncached
    # space.

    # Make sure the ICache is enabled.  This is bit 31 of CP0 Reg 22, Sel 0
    # (ICE); we need to set it to 1 to enable the ICache.  Note that this is
    # a shared CP0 register, so this bit controls the ICache for both TPs.
    mfc0    $9, $22, 0
    lui     $8, 0x8000
    or      $9, $9, $8
    mtc0    $9, $22, 0
    
    # Also, make sure the K0 space is allowed to be cached.  This is controlled
    # by the K0 field (bits 2:0) of CP0 Reg 16, Sel 0.  We can select write-
    # back mode by setting the bits to 011 (set all 3 bits using OR, then
    # clear the MSB using XOR).
    mfc0    $9, $16, 0
    ori     $9, $9, 0x7
    xori    $9, $9, 0x4
    mtc0    $9, $16, 0

             
    ## GGM:
    ## Hardcode stuff for now...

    ## linesize = 16-bbytes
    ## ways = 4
    ## numlines=1024
              
##    mfc0    $10, $16, 1
##    addi    $9, $0, 1024
##    srl     $10, $10, 22
##    andi    $10, $10, 7
##    sllv    $10, $9, $10

    # Use the top of the loop as the starting point, making sure it is in
    # cached space.  I do this by clearing bit 29 of the address.

    la      $8, ICacheInvalidateLoop
    lui     $9, 0x2000
    or      $8, $8, $9
    xor     $8, $8, $9
              ## $8  => start addr (cached)
    addi    $10, $0, 1024
              ## $10 => num loops

    li        $9, 0x8000
    add       $9, $8, $9

    nop
    nop
              
    # This is a rather clever trick I learned from the San Jose guys; we can
    # start off clearing the cache lines associated with the loop below, then
    # jump to the loop in cached space, clearing the rest of the cache lines
    # from there.  This is ultra fast since we'll be running from cache and
    # will have no bus accesses.
    cache   ICACHE_INDEX_STORE_TAG, 0x0000($8)   # way-0
    cache   ICACHE_INDEX_STORE_TAG, 0x4000($8)   # way-1
    cache   ICACHE_INDEX_STORE_TAG, 0x0000($9)   # way-2
    cache   ICACHE_INDEX_STORE_TAG, 0x4000($9)   # way-3

    cache   ICACHE_INDEX_STORE_TAG, 0x0010($8)   # way-0
    cache   ICACHE_INDEX_STORE_TAG, 0x4010($8)   # way-1
    cache   ICACHE_INDEX_STORE_TAG, 0x0010($9)   # way-2
    cache   ICACHE_INDEX_STORE_TAG, 0x4010($9)   # way-3

    cache   ICACHE_INDEX_STORE_TAG, 0x0020($8)   # way-0
    cache   ICACHE_INDEX_STORE_TAG, 0x4020($8)   # way-1
    cache   ICACHE_INDEX_STORE_TAG, 0x0020($9)   # way-2
    cache   ICACHE_INDEX_STORE_TAG, 0x4020($9)   # way-3

    cache   ICACHE_INDEX_STORE_TAG, 0x0030($8)   # way-0
    cache   ICACHE_INDEX_STORE_TAG, 0x4030($8)   # way-1
    cache   ICACHE_INDEX_STORE_TAG, 0x0030($9)   # way-2
    cache   ICACHE_INDEX_STORE_TAG, 0x4030($9)   # way-3

    addiu   $10, $10, -4
    addiu   $9, $9, 64
    jr      $8
    addiu   $8, $8, 64
    
    # Make sure this loop is aligned to a cache line boundary (4 instructions,
    # 16 bytes).  This ensures that we only need 2 full cache lines to store
    # the instructions in the loop.
    .align 4
    
    # The ICache invalidate loop.  We're invalidating each cache line of each
    # set by index.  I have unrolled the loop a bit, so that we do 2 lines for
    # each set.
ICacheInvalidateLoop:
    cache   ICACHE_INDEX_STORE_TAG, 0x0000($8)   # way-0
    cache   ICACHE_INDEX_STORE_TAG, 0x4000($8)   # way-1
    cache   ICACHE_INDEX_STORE_TAG, 0x0000($9)   # way-2
    cache   ICACHE_INDEX_STORE_TAG, 0x4000($9)   # way-3

    cache   ICACHE_INDEX_STORE_TAG, 0x0010($8)   # way-0
    cache   ICACHE_INDEX_STORE_TAG, 0x4010($8)   # way-1
    cache   ICACHE_INDEX_STORE_TAG, 0x0010($9)   # way-2
    cache   ICACHE_INDEX_STORE_TAG, 0x4010($9)   # way-3
              
    addiu   $10, $10, -2
    addiu   $9, $9, 32
    bne     $0, $10, ICacheInvalidateLoop
    addiu   $8, $8, 32

    # change the return address to cached space
    lui     $9, 0x2000
    or      $31, $31, $9
    xor     $31, $31, $9
    
    # Return from the function; note that this will take us back to cached
    # address space if we were in that space originally.
	j       $31
	nop
    
	.set	reorder
#.end InvalidateAndEnableIcache
END(InvalidateAndEnableIcache)*/



              
#************************************************************************
#*     InvalidateAndEnableDcache
#*
#* Turns the DCache on (shared by both TPs), calculates the size of the
#* DCache, and then proceeds to invalidate every line of every set.  We
#* assume that there is nothing interesting in the DCache; anything that
#* is in there will be destroyed.
#*
#* NOTE:  This function assumes that the TagLo and DataLo registers were
#*        previously set to 0 (in Init3368Hw.s.
#*
#* Parameters:  None.
#*
#* Modifies:  Registers t0-t2, and of course CP0 Reg 22, Sel 0 to enable
#*            the ICache.
#************************************************************************
#.ent    InvalidateAndEnableDcache
#InvalidateAndEnableDcache:
/*LEAF(InvalidateAndEnableDcache)
	.set	noreorder

    # Make sure the DCache is enabled.  This is bit 30 of CP0 Reg 22, Sel 0
    # (DCE); we need to set it to 1 to enable the DCache.  Note that this is
    # a shared CP0 register, so this bit controls the DCache for both TPs.
    mfc0    $9, $22, 0
    lui     $8, 0x4000
    or      $9, $9, $8
    mtc0    $9, $22, 0
    
    # Also, make sure the K0 space is allowed to be cached.  This is controlled
    # by the K0 field (bits 2:0) of CP0 Reg 16, Sel 0.  We can select write-
    # back mode by setting the bits to 011 (set all 3 bits using OR, then
    # clear the MSB using XOR).
    #
    # This was done by the InvalidateAndEnableIcache function, and does not need
    # to be done again.
    #mfc0    t1, $16, 0
    #ori     t1, t1, 0x7
    #xori    t1, t1, 0x4
    #mtc0    t1, $16, 0
    
    # The 3368 has a single DCache for both TPs; the cache has 2-way set
    # associativity, and has 16 byte line size.  The cache is 16k bytes.
    # Unlike the ICache (of which there are 2, and they are different sizes),
    # we can assume the size of the DCache rather than wasting code calculating
    # it.
    #
    # The goal of the code below is to put the start address for each way in
    # t0/t1, and to put the stop address in t2:
    #
    #   # Set the size (in bytes) of each way; this is half of the
    #   # total cache size.
    #   t2 = (16 * 1024) / 2
    #      = 8192
    #
    #   # Calculate the start address for each way.  We can pick any reasonal
    #   # address since we are invalidating by index.
    #   t0 = 0x9fc00000;
    #   t1 = t0 + t2;
    #
    #   # Calculate the address where we will stop.
    #   t2 = t1;
    #
    addi    $10, $0, 8192

    # Use the top of the loop as the starting point, making sure it is in
    # cached space.  I do this by clearing bit 29 of the address.		
    ##lui     $8, 0x9fc0
	## default boot address moved from 0xbfc00000 to 0xb8000000
    lui     $8, 0x9800
    
    # Calculate the start of the second set.
    add     $9, $8, $10
    
    # Use this as the stopping address.
    add     $10, $9, $0

   ## GGM: HACK...
              ## $10 => numlines = 1024
              ## 2 ways
              ## 16-bytes per line
              addiu $10, $0, 1024
              ## $8 => start address
              
    # The DCache invalidate loop.  We're invalidating each cache line of each
    # set by index.  I have unrolled the loop a bit, so that we do 2 lines for
    # each set.
DCacheInvalidateLoop:
    cache   DCACHE_INDEX_STORE_TAG, 0x0000($8)  ## way-0
    cache   DCACHE_INDEX_STORE_TAG, 0x4000($8)  ## way-1
              
    cache   DCACHE_INDEX_STORE_TAG, 0x0010($8)  ## way-0
    cache   DCACHE_INDEX_STORE_TAG, 0x4010($8)  ## way-1
              
    addiu   $10, $10, -2
    bne     $0, $10, DCacheInvalidateLoop
    addiu   $8, $8, 32              
    
    # Return from the function.
	j       $31
	nop
    
	.set	reorder
#.end InvalidateAndEnableDcache
END(InvalidateAndEnableDcache)*/

#************************************************************************
#*     InvalidateAndEnableIDcache
#*
#* call both I and D cache inits
#*            
#************************************************************************
#.ent    InvalidateAndEnableIDcache
#InvalidateAndEnableIDcache:
LEAF(InvalidateAndEnableIDcache)
	.set	noreorder

    # save r31
    //or      $26, $31
    move      $26, $31
    # Check A0 ($4) to see what was passed
    # IF BIT[0]=0, assume no cache_init
    #beq     $4, $0, _skip_cache
    #nop


    /****  InvalidateAndEnableIcache ***/
    # We must be running in uncached space at startup (i.e. the ROM must be
    # linked in uncached space), so there is no need to jump to uncached
    # space.

    # Make sure the ICache is enabled.  This is bit 31 of CP0 Reg 22, Sel 0
    # (ICE); we need to set it to 1 to enable the ICache.  Note that this is
    # a shared CP0 register, so this bit controls the ICache for both TPs.
    mfc0    $9, $22, 0
    lui     $8, 0x8000
    or      $9, $9, $8
    mtc0    $9, $22, 0

    # Also, make sure the K0 space is allowed to be cached.  This is controlled
    # by the K0 field (bits 2:0) of CP0 Reg 16, Sel 0.  We can select write-
    # back mode by setting the bits to 011 (set all 3 bits using OR, then
    # clear the MSB using XOR).
    mfc0    $9, $16, 0
    ori     $9, $9, 0x7
    xori    $9, $9, 0x4
    mtc0    $9, $16, 0


    ## GGM:
    ## Hardcode stuff for now...

    ## linesize = 16-bbytes
    ## ways = 4
    ## numlines=1024

##    mfc0    $10, $16, 1
##    addi    $9, $0, 1024
##    srl     $10, $10, 22
##    andi    $10, $10, 7
##    sllv    $10, $9, $10

    # Use the top of the loop as the starting point, making sure it is in
    # cached space.  I do this by clearing bit 29 of the address.

    la      $8, ICacheInvalidateLoop
    FIXUP  ($8);

    lui     $9, 0x2000
    or      $8, $8, $9
    xor     $8, $8, $9
              ## $8  => start addr (cached)
    addi    $10, $0, 1024
              ## $10 => num loops

    li        $9, 0x8000
    add       $9, $8, $9

    nop
    nop

    # This is a rather clever trick I learned from the San Jose guys; we can
    # start off clearing the cache lines associated with the loop below, then
    # jump to the loop in cached space, clearing the rest of the cache lines
    # from there.  This is ultra fast since we'll be running from cache and
    # will have no bus accesses.
    cache   ICACHE_INDEX_STORE_TAG, 0x0000($8)   # way-0
    cache   ICACHE_INDEX_STORE_TAG, 0x4000($8)   # way-1
    cache   ICACHE_INDEX_STORE_TAG, 0x0000($9)   # way-2
    cache   ICACHE_INDEX_STORE_TAG, 0x4000($9)   # way-3

    cache   ICACHE_INDEX_STORE_TAG, 0x0010($8)   # way-0
    cache   ICACHE_INDEX_STORE_TAG, 0x4010($8)   # way-1
    cache   ICACHE_INDEX_STORE_TAG, 0x0010($9)   # way-2
    cache   ICACHE_INDEX_STORE_TAG, 0x4010($9)   # way-3

    cache   ICACHE_INDEX_STORE_TAG, 0x0020($8)   # way-0
    cache   ICACHE_INDEX_STORE_TAG, 0x4020($8)   # way-1
    cache   ICACHE_INDEX_STORE_TAG, 0x0020($9)   # way-2
    cache   ICACHE_INDEX_STORE_TAG, 0x4020($9)   # way-3

    cache   ICACHE_INDEX_STORE_TAG, 0x0030($8)   # way-0
    cache   ICACHE_INDEX_STORE_TAG, 0x4030($8)   # way-1
    cache   ICACHE_INDEX_STORE_TAG, 0x0030($9)   # way-2
    cache   ICACHE_INDEX_STORE_TAG, 0x4030($9)   # way-3

    addiu   $10, $10, -4
    addiu   $9, $9, 64
    jr      $8
    addiu   $8, $8, 64

    # Make sure this loop is aligned to a cache line boundary (4 instructions,
    # 16 bytes).  This ensures that we only need 2 full cache lines to store
    # the instructions in the loop.
    .align 4

    # The ICache invalidate loop.  We're invalidating each cache line of each
    # set by index.  I have unrolled the loop a bit, so that we do 2 lines for
    # each set.
ICacheInvalidateLoop:
    cache   ICACHE_INDEX_STORE_TAG, 0x0000($8)   # way-0
    cache   ICACHE_INDEX_STORE_TAG, 0x4000($8)   # way-1
    cache   ICACHE_INDEX_STORE_TAG, 0x0000($9)   # way-2
    cache   ICACHE_INDEX_STORE_TAG, 0x4000($9)   # way-3

    cache   ICACHE_INDEX_STORE_TAG, 0x0010($8)   # way-0
    cache   ICACHE_INDEX_STORE_TAG, 0x4010($8)   # way-1
    cache   ICACHE_INDEX_STORE_TAG, 0x0010($9)   # way-2
    cache   ICACHE_INDEX_STORE_TAG, 0x4010($9)   # way-3

    addiu   $10, $10, -2
    addiu   $9, $9, 32
    bne     $0, $10, ICacheInvalidateLoop
    addiu   $8, $8, 32

    //jal     InvalidateAndEnableIcache
    //nop

   /***   InvalidateAndEnableDcache ***/
   # Make sure the DCache is enabled.  This is bit 30 of CP0 Reg 22, Sel 0
    # (DCE); we need to set it to 1 to enable the DCache.  Note that this is
    # a shared CP0 register, so this bit controls the DCache for both TPs.
    mfc0    $9, $22, 0
    lui     $8, 0x4000
    or      $9, $9, $8
    mtc0    $9, $22, 0

    # Also, make sure the K0 space is allowed to be cached.  This is controlled
    # by the K0 field (bits 2:0) of CP0 Reg 16, Sel 0.  We can select write-
    # back mode by setting the bits to 011 (set all 3 bits using OR, then
    # clear the MSB using XOR).
    #
    # This was done by the InvalidateAndEnableIcache function, and does not need
    # to be done again.
    #mfc0    t1, $16, 0
    #ori     t1, t1, 0x7
    #xori    t1, t1, 0x4
    #mtc0    t1, $16, 0

    # The 3368 has a single DCache for both TPs; the cache has 2-way set
    # associativity, and has 16 byte line size.  The cache is 16k bytes.
    # Unlike the ICache (of which there are 2, and they are different sizes),
    # we can assume the size of the DCache rather than wasting code calculating
    # it.
    #
    # The goal of the code below is to put the start address for each way in
    # t0/t1, and to put the stop address in t2:
    #
    #   # Set the size (in bytes) of each way; this is half of the
    #   # total cache size.
    #   t2 = (16 * 1024) / 2
    #      = 8192
    #
    #   # Calculate the start address for each way.  We can pick any reasonal
    #   # address since we are invalidating by index.
    #   t0 = 0x9fc00000;
    #   t1 = t0 + t2;
    #
    #   # Calculate the address where we will stop.
    #   t2 = t1;
    #
    addi    $10, $0, 8192

    # Use the top of the loop as the starting point, making sure it is in
    # cached space.  I do this by clearing bit 29 of the address.
    ##lui     $8, 0x9fc0
	## default boot address moved from 0xbfc00000 to 0xb8000000
    lui     $8, 0x9800

    # Calculate the start of the second set.
    add     $9, $8, $10

    # Use this as the stopping address.
    add     $10, $9, $0

   ## GGM: HACK...
              ## $10 => numlines = 1024
              ## 2 ways
              ## 16-bytes per line
              addiu $10, $0, 1024
              ## $8 => start address

    # The DCache invalidate loop.  We're invalidating each cache line of each
    # set by index.  I have unrolled the loop a bit, so that we do 2 lines for
    # each set.
DCacheInvalidateLoop:
    cache   DCACHE_INDEX_STORE_TAG, 0x0000($8)  ## way-0
    cache   DCACHE_INDEX_STORE_TAG, 0x4000($8)  ## way-1

    cache   DCACHE_INDEX_STORE_TAG, 0x0010($8)  ## way-0
    cache   DCACHE_INDEX_STORE_TAG, 0x4010($8)  ## way-1

    addiu   $10, $10, -2
    bne     $0, $10, DCacheInvalidateLoop
    addiu   $8, $8, 32


    //jal     InvalidateAndEnableDcache
    //nop

_skip_cache:
    # resetore r31
    //or      $31, $26, $26
    move   $31, $26

    mtc0 $0,$12                        # reset status register to old way

    mfc0    $4,$16
    or      $4,0x00000003              # enabled caches
    mtc0    $4,$16
    mfc0    $4, $22
    or      $4, 0xC0000000
    mtc0    $4, $22

    # Return from the function.
	j       $31
	nop
    
	.set	reorder
#.end InvalidateAndEnableIDcache
END(InvalidateAndEnableIDcache)

#endif


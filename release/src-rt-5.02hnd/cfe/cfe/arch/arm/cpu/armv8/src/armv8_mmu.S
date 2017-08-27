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


#include <armv8.h>
#include <armmacros.h>
#include "bcm_common.h"
#include "cpu_config.h"

/* Use more than 32 bit virtual address. Currenlty use 33 bit for simple
   uncached and cached DDR memory address mapping where bit 32 0 mean
   cached address and 1 mean uncached address */
#define MMU_EXTENDED_ADDR

#define MMU_GRANULE_SIZE        0x1000
#define MMU_GRANULE_SIZE_SHIFT  12

#ifdef MMU_EXTENDED_ADDR	
#define MMU_L1_TABLE_BASE       CPUCFG_MMU_TABLE_BASE+MMU_GRANULE_SIZE-64
#else
#define MMU_L1_TABLE_BASE       CPUCFG_MMU_TABLE_BASE+MMU_GRANULE_SIZE-32
#endif

#define add_new_table(shift, bits) \
	/* get the next available table address */			\
	ldr	x0, [x5];						\
	add	x0, x0, #MMU_GRANULE_SIZE;				\
	ldr	x11, =(CPUCFG_MMU_TABLE_SIZE+CPUCFG_MMU_TABLE_BASE);	\
	cmp	x0, x11;						\
	bgt	oot;							\
	str	x0, [x5];						\
	/* update the table */						\
	sub	x11, x0, #MMU_GRANULE_SIZE;				\
	orr	x11, x11, #TT_S1_ATTR_TABLE;				\
	str	x11, [x6, x7, lsl #3];					\
	/* now use the new table base and index*/;			\
	sub	x6, x0, #MMU_GRANULE_SIZE;				\
	ubfx	x7, x13, shift, bits;

#define update_table(shift, attr) \
	lsr	x1, x1, shift;						\
1:									\
	cbz	x9, success;						\
	ldr	x8, [x6, x7, lsl #3];					\
	cbz	x8, 2f;							\
	and	x8, x8, #TT_S1_ATTR_MASK;				\
	cmp	x8, attr;						\
	/* only output address and block attribute update is allowed */;\
	bne	invent;							\
2:									\
	mov	x8, xzr;						\
	orr	x8, x8, x1, lsl shift;					\
	orr	x8, x8, x3    /* block attributes */;			\
	orr	x8, x8, attr;						\
	str	x8, [x6, x7, lsl #3];					\
	add	x7, x7, #1;						\
	add	x1, x1, #1;						\
	sub	x9, x9, #1;						\
	b	1b;

/*****************************************************************************
    *  armv8_mmu_add_entry
    *
    *  add one mmu table entry
    *
    *  Input parameters:
    *      x0 - virtual address
    *      x1 - physical address
    *      x2 - total length.  multiple of 2MB if large than 2M or mutiple of 4KB if less than 2M
    *      x3 - mmu entry atrribute flags
    *
    *  Return value:
    *	   x0 - 0 success
    *         - 1 invalid argument
    *         - 2 invalid entry
    *         - 3 out of mmu table
    *
    *  Registers used:
    *   x0, x1, x4 - x13
    *
    *  Comments:
    *
    *   This function create 3 level page table entries starting from L1 as needed.
    *   L1 tables only need 4 entries to cover all 4GB space.  Each entry is the 
    *   table descriptor to L2 table entry. Each L2 entry covers 2MB space.  It can 
    *   be a block descriptor if all 2MB space map to the same attribute of the memory 
    *   or device.  If there are different types within 2MB space, the entry need be 
    *   a pointer to the L3 table descriptor. L3 table contains page descriptor for 
    *   each 4KB of memory or device area. Example below show how to build the mmu 
    *   table with different types of memory
    *
    *      L1                        L2                            L3
    *   table descriptor 0--> block descriptor   0
    *                         [PA 0x0000_0000 - 0x001f_ffff]
    *                         2MB WRWA cached DDR
    *                               ...
    *                         table descriptor 511     -->   page descriptor 0
    *                                                        [PA 0x7fc0_0000 - 0x7fc0_0fff]
    *                                                        4KB WRWA cached DDR
    *                                                              ...
    *                                                        page descriptor 495
    *                                                        [PA 0x7fff_0000 - 0x7fff_0fff]
    *                                                        4KB uncached SRAM
    *                                                               ...
    *                                                        page descriptor 511
    *                                                        [PA 0x7fff_f000 - 0x7fff_ffff]
    *                                                        4KB uncached SRAM
    *
    *   MMU table use the 64KB SRAM fromt the MEMC mapped at CPUCFG_MMU_TABLE_BASE. It can 
    *   hold up to 16 tables. L1 table only need 4 entry or 32 bytes but one 4K
    *   table is always reserved. L2 need 4 tables. Number of L3 tables depends on
    *   how many 4K page memory and device range is needed. The next_tbl_addr
    *   @ CPUCFG_MMU_TABLE_BASE contains the address of the next available table.  
    *   L1 MMU table starts at CPUCFG_MMU_TABLE_BASE+MMU_GRANULE_SIZE-32
    *
    **************************************************************************/

FUNC(armv8_mmu_add_entry)
	/* register usage:
	   x5 - nex_table_addr address
	   x6 - table base
	   x7 - table entry index
	   x8 - table entry value
	   x9 - number entry to update
	   x0, x4, x10, x11, x12 tempeorary
	   x13 - backup of x0
	*/
	mov	x13, x0    /* save input virtual in x13 */

	mov	x5, #CPUCFG_MMU_TABLE_BASE /* next_tbl_addr in x5 */
	ldr	x6, [x5]
	cbnz	x6, 1f

	/* first time this function is called, setup the next available table address */
	ldr	x6, =(CPUCFG_MMU_TABLE_BASE + MMU_GRANULE_SIZE)
	str	x6, [x5]
1:

	/* check first L1 table entry */
	ldr	x6, =MMU_L1_TABLE_BASE
#ifdef MMU_EXTENDED_ADDR
	ubfx	x7, x13, #30, #3
#else
	ubfx	x7, x13, #30, #2
#endif
	ldr	x8, [x6, x7, lsl #3]

	/* check how many and what type of entries is needed */
	lsr	x9, x2, #30
	cbnz	x9, l1entry
	lsr	x9, x2, #21
	cbz     x9, l3entry
	b	l2entry

l1entry:
	add	x10, x7, x9
#ifdef MMU_EXTENDED_ADDR
	cmp	x10, #12
#else	
	cmp	x10, #8
#endif
	bge	invarg

	update_table(#30, #TT_S1_ATTR_BLOCK)

l2entry:
	cbz	x9, invarg
	ubfx	x10, x13, #21, #9
	add	x10, x9, x10
	cmp	x10, #512
	/* no more than 512 entry or cross the 2MB boundary is allowed */
	bgt	invarg

	cbz	x8, l2newtable
	and	x10, x8, #TT_S1_ATTR_MASK
	cmp	x10, #TT_S1_ATTR_TABLE
	bne	invent
	/* now use the new table base and index*/
	and	x6, x8, #~TT_S1_ATTR_MASK
	ubfx	x7, x13, #21, #9
	b	l2update

l2newtable:
    add_new_table(#21, #9)
l2update:
	update_table(#21, #TT_S1_ATTR_BLOCK)

l3entry:
	lsr	x9, x2, #12
	ubfx	x10, x13, #12, #9
	add	x10, x9, x10
	cmp	x10, #512
	/* no more than 512 entry or cross the 2MB boundary is allowed */
	bgt	invarg
	cbz	x9, invarg

	mov	x12, #1
l3checktable:
	cbz	x8, l3newtable
	and	x10, x8, #TT_S1_ATTR_MASK
	cmp	x10, #TT_S1_ATTR_TABLE
	bne	invent
	/* now use the new table base and index*/
	and	x6, x8, #~TT_S1_ATTR_MASK
	cbz	x12, 1f
	ubfx	x7, x13, #21, #9
	b	l3checknext
1:
	ubfx	x7, x13, #12, #9
	b	l3checknext
l3newtable:
	cbz     x12, l3newtable2
	add_new_table(#21, #9)
	b	l3checknext
l3newtable2:
	add_new_table(#12, #9)
l3checknext:
	ldr	x8, [x6, x7, lsl #3]
	cbz	x12, l3conti
	sub	x12, x12, #1
	b	l3checktable

l3conti:
	update_table(#12, #TT_S1_ATTR_PAGE)

invarg:
	mov	x0, 1
	b	exit
invent:
	mov	x0, 2
	b	exit
oot:
	mov	x0, 3
	b	exit

success:
	mov	x0, 0
exit:
	ret

END(armv8_mmu_add_entry)

/* memory and device register table to be added to the mmu table
   32 bit virtual address
   32 bit physical address
   32 bit total length.  multiple of 2MB if large than 2M or mutiple of 4KB if less
	  than 2M. The addr range can not cross the boundry of 2M or 4KB respectively
   32 bit padding
   64 bit mmu entry atrribute flags
 */
#define mem_dev_table_entry(va, pa, length, attr)	\
	.word	va;					\
	.word	pa;					\
	.word	length;				\
	.word	0x0;					\
	.dword	attr;

#define ATTR_CACHED_MEM		((1 << TT_S1_ATTR_MATTR_LSB)|TT_S1_ATTR_AP_RW_EL3|TT_S1_ATTR_SH_INNER|TT_S1_ATTR_AF)
#define ATTR_CACHED_RO_MEM	((1 << TT_S1_ATTR_MATTR_LSB)|TT_S1_ATTR_AP_RO_EL3|TT_S1_ATTR_SH_INNER|TT_S1_ATTR_AF)
#define ATTR_NONCACHED_MEM	((0 << TT_S1_ATTR_MATTR_LSB)|TT_S1_ATTR_AP_RW_EL3|TT_S1_ATTR_SH_INNER|TT_S1_ATTR_AF)
#define ATTR_NONCACHED_RO_MEM	((0 << TT_S1_ATTR_MATTR_LSB)|TT_S1_ATTR_AP_RO_EL3|TT_S1_ATTR_SH_INNER|TT_S1_ATTR_AF)
#define ATTR_DEVICE		((2 << TT_S1_ATTR_MATTR_LSB)|TT_S1_ATTR_AP_RW_EL3|TT_S1_ATTR_XN_EL3|TT_S1_ATTR_AF)

	.align 3
mem_dev_table:



#if defined(_BCM94908_)
#if !defined(CFG_RAMAPP)  /* cfe rom and boot rom */
#if (INC_BTRM_BUILD==1)
	/* SRAM in MEMC for MMU table */
	mem_dev_table_entry(0x7fff0000, 0x7fff0000, 0x10000, ATTR_DEVICE)

	/* boot rom direct access window */
	mem_dev_table_entry(0xfff00000, 0xfff00000, 0x20000, ATTR_CACHED_RO_MEM)

	/* peripheral SRAM for transit code and security credential */ 
	mem_dev_table_entry(0xfff80000, 0xfff80000, 0x8000, ATTR_DEVICE)
#else
	/* SRAM in MEMC for MMU table */
	mem_dev_table_entry(0xa0000000, 0x7fff0000, 0x10000, ATTR_DEVICE)

	/* peripheral SRAM mmu disabler code */
	mem_dev_table_entry(0xfff80000, 0xfff80000, 0x8000, ATTR_NONCACHED_MEM)

	/* DDR memory including SRAM in  MEMC for MMU table. Enable the maximum
	   2GB DDR for alias test. Use Device Attr to avoid CPU speculative fetch */
	mem_dev_table_entry(0x00000000, 0x00000000, 0x80000000, ATTR_DEVICE)

	/* MEMC and DDR PHY control registers */
	mem_dev_table_entry(0x80018000, 0x80018000, 0x4000, ATTR_DEVICE)
#endif

	/* Runner SRAM for runtime memory */
	mem_dev_table_entry(0x90000000, 0x82210000, 0xc000, ATTR_CACHED_MEM)
	mem_dev_table_entry(0x9000c000, 0x82250000, 0xc000, ATTR_CACHED_MEM)
	mem_dev_table_entry(0x90018000, 0x82200000, 0x8000, ATTR_CACHED_MEM)
	mem_dev_table_entry(0x90020000, 0x822A0000, 0x20000, ATTR_CACHED_MEM)

	/* all the peripheral device register */
	mem_dev_table_entry(0xff800000, 0xff800000, 0x400000, ATTR_DEVICE)

	/* EMMC, SPI direct access window */
	mem_dev_table_entry(0xffc00000, 0xffc00000, 0x200000, ATTR_CACHED_RO_MEM)
	/* NAND direct access window */
	mem_dev_table_entry(0xffe00000, 0xffe00000, 0x100000, ATTR_CACHED_RO_MEM)
#else
	/* UNIMAC block registers */
	mem_dev_table_entry(0x80002000, 0x80002000, 0x1000, ATTR_DEVICE)

	/* MEMC and DDR PHY control registers */
	mem_dev_table_entry(0x80018000, 0x80018000, 0x4000, ATTR_DEVICE)

	/* switch block registers */
	mem_dev_table_entry(0x80080000, 0x80080000, 0x50000, ATTR_DEVICE)

	/* PMC */
	mem_dev_table_entry(0x80200000, 0x80200000, 0x5000, ATTR_DEVICE)

	/* Process monitor */
	mem_dev_table_entry(0x80280000, 0x80280000, 0x1000, ATTR_DEVICE)

	/* UBUS_RUBS block registers */
	mem_dev_table_entry(0x81060000, 0x81060000, 0x4000, ATTR_DEVICE)

	/* all the peripheral device register */
	mem_dev_table_entry(0xff800000, 0xff800000, 0x400000, ATTR_DEVICE)

	/* EMMC, SPI direct access window */
	mem_dev_table_entry(0xffc00000, 0xffc00000, 0x200000, ATTR_CACHED_RO_MEM)
	/* NAND direct access window */
	mem_dev_table_entry(0xffe00000, 0xffe00000, 0x100000, ATTR_CACHED_RO_MEM)
	/* LUT registers */
	mem_dev_table_entry(0xffff0000, 0xffff0000, 0x1000, ATTR_DEVICE)
#endif
#endif /* #if defined(_BCM94908_) */




#if defined(_BCM96858_)
#if !defined(CFG_RAMAPP)  /* cfe rom and boot rom */
#if (INC_BTRM_BUILD==1)
	/* SRAM in MEMC for MMU table */
	mem_dev_table_entry(0x7fff0000, 0x7fff0000, 0x10000, ATTR_DEVICE)

	/* boot rom direct access window */
	mem_dev_table_entry(0xfff00000, 0xfff00000, 0x20000, ATTR_CACHED_RO_MEM)

	/* peripheral SRAM for transit code and security credential */ 
	mem_dev_table_entry(0xfff80000, 0xfff80000, 0x8000, ATTR_DEVICE)

        /* peripheral SRAM for transit code and security credential */
        mem_dev_table_entry(0xfff80000, 0xfff80000, 0x8000, ATTR_DEVICE)

        /* SRAM for bootrom runtime  */
        mem_dev_table_entry(0x90000000, 0x82608000, 0xc000, ATTR_CACHED_MEM)
        mem_dev_table_entry(0x9000c000, 0x82614000, 0xc000, ATTR_CACHED_MEM)
        mem_dev_table_entry(0x90018000, 0x82600000, 0x8000, ATTR_CACHED_MEM)
        mem_dev_table_entry(0x90020000, 0x82620000, 0x20000, ATTR_CACHED_MEM)
#else
	/* SRAM in MEMC for MMU table */
	mem_dev_table_entry(0xa0000000, 0x7fff0000, 0x10000, ATTR_DEVICE)

	/* peripheral SRAM in case cfe rom need to use */
	mem_dev_table_entry(0xfff80000, 0xfff80000, 0x8000, ATTR_NONCACHED_MEM)

	/* DDR memory including SRAM in  MEMC for MMU table. Enable the maximum
	   2GB DDR for alias test. Use Device Attr to avoid CPU speculative fetch */
	mem_dev_table_entry(0x00000000, 0x00000000, 0x80000000, ATTR_DEVICE)

	/* MEMC and DDR PHY control registers */
	mem_dev_table_entry(0x80180000, 0x80180000, 0x24000, ATTR_DEVICE)

	/* PMC */
	mem_dev_table_entry(0x80200000, 0x80200000, 0x5000, ATTR_DEVICE)

	/* Process monitor */
	mem_dev_table_entry(0x80280000, 0x80280000, 0x1000, ATTR_DEVICE)

	/* ubus mapped registers */
	mem_dev_table_entry(0x83000000, 0x83000000, 0x1000, ATTR_DEVICE)
	mem_dev_table_entry(0x834d4000, 0x834d4000, 0x1000, ATTR_DEVICE)

	/* Runner SRAM for runtime memory */
	mem_dev_table_entry(0x82600000, 0x82600000, 0x80000, ATTR_CACHED_MEM)
#endif

	/* all the peripheral device register */
	mem_dev_table_entry(0xff800000, 0xff800000, 0x600000, ATTR_DEVICE)

	/* SPI direct access window */
	mem_dev_table_entry(0xffc00000, 0xffc00000, 0x200000, ATTR_CACHED_RO_MEM)

	/* NAND direct access window */
	mem_dev_table_entry(0xffe00000, 0xffe00000, 0x100000, ATTR_CACHED_RO_MEM)
#else
	/* MEMC and DDR PHY control registers */
	mem_dev_table_entry(0x80180000, 0x80180000, 0x24000, ATTR_DEVICE)

	/* XRDP block */
 	mem_dev_table_entry(0x82000000, 0x82000000, 0x1800000, ATTR_DEVICE)

 	/* LPORT block */
  	mem_dev_table_entry(0x80138000, 0x80138000 , 0x8000, ATTR_DEVICE)

	/* all the peripheral device register */
	mem_dev_table_entry(0xff800000, 0xff800000, 0x600000, ATTR_DEVICE)

	/* PMC */
	mem_dev_table_entry(0x80200000, 0x80200000, 0x5000, ATTR_DEVICE)

	/* Process monitor */
	mem_dev_table_entry(0x80280000, 0x80280000, 0x1000, ATTR_DEVICE)

	/* EMMC, SPI direct access window */
	mem_dev_table_entry(0xffc00000, 0xffc00000, 0x200000, ATTR_CACHED_RO_MEM)

	/* NAND direct access window */
	mem_dev_table_entry(0xffe00000, 0xffe00000, 0x100000, ATTR_CACHED_RO_MEM)

	/* ACE BIU registers */
	mem_dev_table_entry(0x81060120, 0x81060120, 0x1000, ATTR_DEVICE)

     /*CCI 400*/
    mem_dev_table_entry(0x81090000, 0x81090000 , 0xe000, ATTR_DEVICE)
#endif
#endif /* #if defined(_BCM96858_) */


#if defined(_BCM968360_)
#if !defined(CFG_RAMAPP)  /* cfe rom and boot rom */
#if (INC_BTRM_BUILD==1)

    /* btrm */

    /* SRAM in MEMC for MMU table */
    mem_dev_table_entry(0x7fff0000, 0x7fff0000, 0x10000, ATTR_DEVICE)

    /* boot rom direct access window */
    mem_dev_table_entry(0xfff00000, 0xfff00000, 0x20000, ATTR_CACHED_RO_MEM)

    /* peripheral SRAM for transit code and security credential */ 
    mem_dev_table_entry(0xfff80000, 0xfff80000, 0x8000, ATTR_DEVICE)

    /* SRAM for bootrom runtime  */
    mem_dev_table_entry(0x90000000, 0x82600000, 0x6000, ATTR_CACHED_MEM)
    mem_dev_table_entry(0x90006000, 0x82606000, 0x6000, ATTR_CACHED_MEM)
    mem_dev_table_entry(0x9000c000, 0x8260c000, 0x6000, ATTR_CACHED_MEM)
    mem_dev_table_entry(0x90012000, 0x82612000, 0x1e000, ATTR_CACHED_MEM)

#else

    /* cfe rom */

    /* SRAM in MEMC for MMU table */
    mem_dev_table_entry(0xa0000000, 0x7fff0000, 0x10000, ATTR_DEVICE)

    /* peripheral SRAM in case cfe rom need to use */
    mem_dev_table_entry(0xfff80000, 0xfff80000, 0x8000, ATTR_NONCACHED_MEM)

    /* DDR memory including SRAM in  MEMC for MMU table. Enable the maximum
       2GB DDR for alias test. Use Device Attr to avoid CPU speculative fetch */
    mem_dev_table_entry(0x00000000, 0x00000000, 0x80000000, ATTR_DEVICE)

    /* MEMC and DDR PHY control registers */
    mem_dev_table_entry(0xc0000000, 0xc0000000, 0x24000, ATTR_DEVICE)

    /* PMC */
    mem_dev_table_entry(0x80200000, 0x80200000, 0x5000, ATTR_DEVICE)

    /* Process monitor */
    mem_dev_table_entry(0x80280000, 0x80280000, 0x1000, ATTR_DEVICE)

    /* ubus mapped registers */
    mem_dev_table_entry(0x83000000, 0x83000000, 0x1000, ATTR_DEVICE)

    /* Runner SRAM for runtime memory */
    mem_dev_table_entry(0x82600000, 0x82600000, 0x80000, ATTR_CACHED_MEM)

    /* ACE BIU registers */
    mem_dev_table_entry(0x81060000, 0x81060000, 0x3000, ATTR_DEVICE)

#endif

    /* btrm and cfe rom */

    /* all the peripheral device registers */
    mem_dev_table_entry(0xff800000, 0xff800000, 0x60000, ATTR_DEVICE)

    /* EMMC and SPI direct access window */
    mem_dev_table_entry(0xffc00000, 0xffc00000, 0x200000, ATTR_CACHED_RO_MEM)

    /* NAND direct access window */
    mem_dev_table_entry(0xffe00000, 0xffe00000, 0x100000, ATTR_CACHED_RO_MEM)

#else

    /* cfe ram */

    /* MEMC and DDR PHY control registers */
    mem_dev_table_entry(0xc0000000, 0xc0000000, 0x24000, ATTR_DEVICE)

    /* all the peripheral device register */
    mem_dev_table_entry(0xff800000, 0xff800000, 0x60000, ATTR_DEVICE)

    /* PMC */
    mem_dev_table_entry(0x80200000, 0x80200000, 0x5000, ATTR_DEVICE)

    /* Process monitor */
    mem_dev_table_entry(0x80280000, 0x80280000, 0x1000, ATTR_DEVICE)

    /* EMMC, SPI direct access window */
    mem_dev_table_entry(0xffc00000, 0xffc00000, 0x200000, ATTR_CACHED_RO_MEM)

    /* NAND direct access window */
    mem_dev_table_entry(0xffe00000, 0xffe00000, 0x100000, ATTR_CACHED_RO_MEM)

    /* ACE BIU registers */
    mem_dev_table_entry(0x81060000, 0x81060000, 0x3000, ATTR_DEVICE)

#endif
#endif  /* #if defined(_BCM968360_) */


mem_dev_table_end:
	.word	0x0

FUNC(armv8_mmuinit)

	mov	x14, lr
#if !defined(CFG_RAMAPP)
	// enable 64KB sram in MEMC controller for MMU table
	ldr	x0, =MEMC_BASE
	ldr	w1, =(CPUCFG_MMU_TABLE_BASE|0x00000040) // map to 0x7fff-0000, size = 64KB
	str	w1, [x0, MEMC_SRAM_REMAP_CONTROL] ;

	orr	w1, w1, #2      /* enable the map */
	str	w1, [x0, MEMC_SRAM_REMAP_CONTROL] ;

	dmb	ish
#endif

	/* clear mmu tables */
	mov	x1, #CPUCFG_MMU_TABLE_BASE
	mov	x3, #(CPUCFG_MMU_TABLE_SIZE/8)
	mov	x0, x1
zerotbl:
	str	xzr, [x0], #8
	subs	x3, x3, #1
	bne	zerotbl

	ldr	x1, =MMU_L1_TABLE_BASE
	msr	TTBR0_EL3, x1

	/* Set up memory attributes
	   0 -> 0b01000100 = Normal, Inner/Outer Non-Cacheable
	   1 -> 0b11111111 = Normal, Inner/Outer WriteBack Read/Write Allocate
	   2 -> 0b00000000 = Device-nGnRnE
	*/
	mov	x1, #0x000000000000FF44
	msr	MAIR_EL3, x1


	/*  4K granularity, outer-shareable, Inner & Outer non-cachable memory
	    32 bits VA and PA space */
#ifdef MMU_EXTENDED_ADDR
	mov	x1, #0x201f
#else
	mov	x1, #0x2020
#endif
	msr	TCR_EL3, x1
	isb

	/* go through memory and device register table and add it to mmu table */
	LOADREL(x20, =mem_dev_table)
	LOADREL(x21, =mem_dev_table_end)
	mov	x0, xzr
	mov	x1, xzr
	mov	x2, xzr
	mov	x3, xzr
mmu_init_loop:
	ldr	w0, [x20], #4
	ldr	w1, [x20], #4
	ldr	w2, [x20], #8
	ldr	x3, [x20], #8
	CALLINIT(=armv8_mmu_add_entry)
	cbnz	x0, mmu_error
	cmp	x20, x21
	bne	mmu_init_loop

#if !defined(CFG_RAMAPP)
	/* in bootrom loaded case, cfe rom init code runs at virtual address
	 different than the physical address. We need to add one more entry
	 that pa=va for the cfe rom code itself so that the instrution right
	 after the mmu enable won't cause exception */
	
	/* this will fail if boot from SPI NOR XIP directly as it is already
	   added. but it is like a NOP and we expect XIP boot from rom too*/
	adr	x0, mmu_init_loop       /* get the current running physical address */
	and	x0, x0, #~0x1ffff
	mov	x1, x0                  /* pa = va */
	mov	x2, #0x20000            /* 128KB cfe rom size */
	ldr	x3, =ATTR_CACHED_MEM
	CALLINIT(=armv8_mmu_add_entry)
#else
	/*dected ddr size and setup mmu entry */
	ldr	x0, =MEMC_BASE
	mov	x1, xzr
#if defined(_BCM968360_)
    ldr w1, [x0, MEMC_GLB_FSBL_STATE]
    and w1, w1, #MEMC_GLB_FSBL_DRAM_SIZE_MASK
    lsr w1, w1, #MEMC_GLB_FSBL_DRAM_SIZE_SHIFT
#else
	ldr	w1, [x0, MEMC_GLB_GCFG]
	and	w1, w1, #MEMC_GLB_GCFG_DRAM_SIZE1_MASK
#endif
	add	w1, w1, #20
	mov	x2, #1
	lsl	x2, x2, x1
	mov	x0, xzr
	mov	x1, xzr
	ldr	x3, =ATTR_CACHED_MEM
	CALLINIT(=armv8_mmu_add_entry)
	cbnz	x0, mmu_error
#ifdef MMU_EXTENDED_ADDR
	mov	x0, #1
	lsl	x0, x0, #32
	mov	x1, xzr
	ldr	x3, =ATTR_NONCACHED_MEM
	CALLINIT(=armv8_mmu_add_entry)
	cbnz	x0, mmu_error
#endif
#endif
	/* invalidate all tlb entries */
	tlbi	ALLE3
	dsb	ish

#if !defined(CFG_RAMAPP)	
	/* print out how much table we used */
	mov	x1, #CPUCFG_MMU_TABLE_BASE
	ldr	x2, [x1]
	sub	x2, x2, x1
	lsr	x2, x2, #MMU_GRANULE_SIZE_SHIFT
	ldr	x1, =0x4d4d5530   /* MMU0 */
	add	x0, x1, x2
	CALLINIT(=board_setleds)
#endif

	mov	lr, x14
	ret

mmu_error:
	ldr	x1, =0x4d455230   /* MER0 */
	add	x0, x0, x1
	CALLINIT(=board_setleds)
1:
	b       1b

END(armv8_mmuinit)

/*  *********************************************************************
    *  armv8_enable_mmu
    *
    *  Enable MMU
    *
    *  Input parameters:
    *	   nothing
    *
    *  Return value:
    *	   nothing
    *
    *  Registers used:
    *	   x0
    ********************************************************************* */
FUNC(armv8_enable_mmu)

	mrs	x1, SCTLR_EL3
	orr	x1, x1, SCTLR_ELx_M
	msr	SCTLR_EL3, x1
	isb

	ret
END(armv8_enable_mmu)

/*  *********************************************************************
    *  armv8_disable_mmu
    *
    *  Disable MMU
    *
    *  Input parameters:
    *	   nothing
    *
    *  Return value:
    *	   nothing
    *
    *  Registers used:
    *	   x0
    ********************************************************************* */
FUNC(armv8_disable_mmu)

	mrs	x1, SCTLR_EL3
	bic	x1, x1, SCTLR_ELx_M
	msr	SCTLR_EL3, x1
	isb
#if !defined(CFG_RAMAPP)
	// disable 64KB sram in MEMC controller
	ldr	x0, =MEMC_BASE
	mov	w1, #0      /* disable the map */
	str	w1, [x0, MEMC_SRAM_REMAP_CONTROL] ;
	dmb	ish
#endif	
	ret

END(armv8_disable_mmu)

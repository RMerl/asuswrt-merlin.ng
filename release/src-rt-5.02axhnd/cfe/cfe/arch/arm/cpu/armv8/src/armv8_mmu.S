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
#include "bcm_map.h"
#include "cpu_config.h"
#include "bcm_hwdefs.h"	

#define MMU_GRANULE_SIZE	0x1000
#define MMU_GRANULE_SIZE_SHIFT	12

/* L1 table size. Determined by the number VA bits used. Each table entry
 * is 8 bytes
 */
#define MMU_L1_ADDR_BITS	(CPUCFG_NUM_VA_BITS-30)
#define MMU_L1_NUM_ENTRY	(0x1<<MMU_L1_ADDR_BITS)
#define MMU_L1_TABLE_SIZE	(MMU_L1_NUM_ENTRY*8)
#define MMU_L1_TABLE_BASE	CPUCFG_MMU_TABLE_BASE+MMU_GRANULE_SIZE-MMU_L1_TABLE_SIZE

#define MMU_TCR_T0SZ		(64-CPUCFG_NUM_VA_BITS)
#if (CPUCFG_NUM_PA_BITS==32)
#define MMU_PA_SIZE		TCR_SIZE_4G
#elif (CPUCFG_NUM_PA_BITS==36)
#define MMU_PA_SIZE		TCR_SIZE_64G
#else
#error "invalid physical address bits!"
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
    *   This function create 3 level page table entries as needed starting from L1.
    *   Each entry is either a 1GB block descriptor or table descriptor to L2 table
    *   entry. Each L2 entry covers 2MB space  which can be a block descriptor if
    *   all 2MB space map to the same attribute of memory or device. If there are
    *   different types within 2MB space, the entry need be a pointer to the L3 table
    *   descriptor. L3 table contains page descriptor for each 4KB of memory or device
    *   area. Example below show how to build the mmu table with different types of memory
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
    *   MMU table use memory defined by CPUCFG_MMU_TABLE_BASE and CPUCFG_MMU_TABLE_SIZE.
    * 	For CFE ROM and Boot ROM, it uses the 64KB SRAM from the MEMC controller which can support
    *   up to 16 descriptor tables. For CFE RAM, it use ddr and it has enough size to hold mmu table
    *
    *   The next_tbl_addr @ CPUCFG_MMU_TABLE_BASE contains the address of the next available
    *	table.  
    **************************************************************************/

FUNC(armv8_mmu_add_entry)
	/* register usage:
	   x5 - next_table_addr address
	   x6 - table base
	   x7 - table entry index
	   x8 - table entry value
	   x9 - number of entries to update
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
	ubfx	x7, x13, #30, #MMU_L1_ADDR_BITS
	ldr	x8, [x6, x7, lsl #3]

	/* check how many and what type of entries is needed */
	lsr	x9, x2, #30
	cbnz	x9, l1entry
	lsr	x9, x2, #21
	cbz     x9, l3entry
	b	l2entry

l1entry:
	add	x10, x7, x9
	cmp	x10, #MMU_L1_NUM_ENTRY
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
   up to 64 bit virtual address
   up to 64 bit physical address
   up to 64 bit total length.  multiple of 2MB if large than 2M or mutiple of 4KB if less
         than 2M. The addr range can not cross the boundry of 2M or 4KB respectively
   64 bit mmu entry atrribute flags
 */
#define mem_dev_table_entry(va, pa, length, attr)	\
	.dword	va;					\
	.dword	pa;					\
	.dword	length;					\
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
	mem_dev_table_entry(CPUCFG_MMU_TABLE_BASE, CPUCFG_MMU_TABLE_BASE, CPUCFG_MMU_TABLE_SIZE, ATTR_DEVICE)

	/* boot rom direct access window */
#if !defined (_BTRM_DEVEL_)
        /* BTRM_INT_SRAM_MEMC_64K_ADDR - text rom*/
	mem_dev_table_entry(BOOTROM_BASE, BOOTROM_BASE, BOOTROM_SIZE, ATTR_CACHED_RO_MEM)
#else
	mem_dev_table_entry(0x90400000, 0x00400000, 0x200000, ATTR_CACHED_MEM)
#endif
	/* peripheral SRAM for transit code and security credential */ 
	mem_dev_table_entry(PERF_SRAM_BASE, PERF_SRAM_BASE, PERF_SRAM_SIZE, ATTR_DEVICE)
#else
	/* SRAM in MEMC for MMU table */
	mem_dev_table_entry(0xa0000000, CPUCFG_MMU_TABLE_BASE, CPUCFG_MMU_TABLE_SIZE, ATTR_DEVICE)

	/* peripheral SRAM mmu disabler code */
	mem_dev_table_entry(PERF_SRAM_BASE, PERF_SRAM_BASE, PERF_SRAM_SIZE, ATTR_CACHED_MEM)

	/* DDR memory including SRAM in  MEMC for MMU table. Enable the maximum
	 2GB DDR for alias test. Use Device Attr to avoid CPU speculative fetch */
	mem_dev_table_entry(DRAM_BASE, DRAM_BASE, 0x80000000, ATTR_DEVICE)

	/* MEMC and DDR PHY control registers */
	mem_dev_table_entry(MEMC_BASE, MEMC_BASE, MEMC_SIZE, ATTR_DEVICE)
#endif
       /* Runner SRAM for runtime memory */
       /* BTRM_INT_SRAM_RDP_48K_0_ADDR  - data, rodata  0xc000*2 */
	mem_dev_table_entry(0x90000000, 0x82210000, 0xc000, ATTR_CACHED_MEM)
       /* BTRM_INT_SRAM_RDP_48K_0_ADDR -  data, rodata */
	mem_dev_table_entry(0x9000c000, 0x82250000, 0xc000, ATTR_CACHED_MEM)
        /* BTRM_INT_SRAM_RDP_32K_ADDR - bss*/ 
	mem_dev_table_entry(0x90018000, 0x82200000, 0x8000, ATTR_CACHED_MEM)
        /*BTRM_INT_MEM_SBI_LINK_ADDR_VIRT/PHYS*/
	mem_dev_table_entry(0x90020000, 0x822A0000, 0x20000, ATTR_CACHED_MEM)
	/* all the peripheral device register */
	mem_dev_table_entry(PERF_BASE, PERF_BASE, PERF_SIZE, ATTR_DEVICE)
	mem_dev_table_entry(PERF1_BASE, PERF1_BASE, PERF1_SIZE, ATTR_DEVICE)
	
	/* SPI direct access window */
	mem_dev_table_entry(SPIFLASH_BASE, SPIFLASH_BASE, SPIFLASH_SIZE, ATTR_DEVICE)
#else
	/* peripheral SRAM for security credential */ 
	mem_dev_table_entry(PERF_SRAM_BASE, PERF_SRAM_BASE, PERF_SRAM_SIZE, ATTR_DEVICE)

	/* DDR entries for cache and uncached virtual address, total size is a placehold
	   and will be filled in at run time  */
	mem_dev_table_entry(DRAM_BASE, DRAM_BASE, 0x00000000,  ATTR_CACHED_MEM)
	mem_dev_table_entry(DRAM_BASE|VA_FLAGS_NONCACHE_MEM, DRAM_BASE, 0x00000000,  ATTR_NONCACHED_MEM)
	
	/* UNIMAC block registers */
	mem_dev_table_entry(GMAC_BASE, GMAC_BASE, GMAC_SIZE, ATTR_DEVICE)

	/* MEMC and DDR PHY control registers */
	mem_dev_table_entry(MEMC_BASE, MEMC_BASE, MEMC_SIZE, ATTR_DEVICE)

	/* switch block registers */
	mem_dev_table_entry(SWITCH_BASE, SWITCH_BASE, SWITCH_SIZE, ATTR_DEVICE)

	/* PMC */
	mem_dev_table_entry(PMC_BASE, PMC_BASE, PMC_SIZE, ATTR_DEVICE)

	/* Process monitor */
	mem_dev_table_entry(PROC_MON_BASE, PROC_MON_BASE, PROC_MON_SIZE, ATTR_DEVICE)

	/* UBUS_RUBS block registers */
	mem_dev_table_entry(URB_BASE, URB_BASE, URB_SIZE, ATTR_DEVICE)

	/* all the peripheral device register */
	mem_dev_table_entry(PERF_BASE, PERF_BASE, PERF_SIZE, ATTR_DEVICE)
	mem_dev_table_entry(PERF1_BASE, PERF1_BASE, PERF1_SIZE, ATTR_DEVICE)

	/* LUT registers */
	mem_dev_table_entry(BOOTLUT_BASE, BOOTLUT_BASE, BOOTLUT_SIZE, ATTR_DEVICE)
#endif
#endif /* #if defined(_BCM94908_) */



#if defined(_BCM96858_)
#if !defined(CFG_RAMAPP)  /* cfe rom and boot rom */
#if (INC_BTRM_BUILD==1)
	/* SRAM in MEMC for MMU table */
	mem_dev_table_entry(CPUCFG_MMU_TABLE_BASE, CPUCFG_MMU_TABLE_BASE, CPUCFG_MMU_TABLE_SIZE, ATTR_DEVICE)

	/* boot rom direct access window */
	mem_dev_table_entry(BOOTROM_BASE, BOOTROM_BASE, BOOTROM_SIZE, ATTR_CACHED_RO_MEM)

	/* peripheral SRAM for transit code and security credential */ 
	mem_dev_table_entry(PERF_SRAM_BASE, PERF_SRAM_BASE, PERF_SRAM_SIZE, ATTR_DEVICE)

        /* SRAM for bootrom runtime  */
        mem_dev_table_entry(0x90000000, 0x82608000, 0xc000, ATTR_CACHED_MEM)
        mem_dev_table_entry(0x9000c000, 0x82614000, 0xc000, ATTR_CACHED_MEM)
        mem_dev_table_entry(0x90018000, 0x82600000, 0x8000, ATTR_CACHED_MEM)
        mem_dev_table_entry(0x90020000, 0x82620000, 0x20000, ATTR_CACHED_MEM)
#else
	/* SRAM in MEMC for MMU table */
	mem_dev_table_entry(0xa0000000, CPUCFG_MMU_TABLE_BASE, CPUCFG_MMU_TABLE_SIZE, ATTR_DEVICE)

	/* peripheral SRAM in case cfe rom need to use */
	mem_dev_table_entry(PERF_SRAM_BASE, PERF_SRAM_BASE, PERF_SRAM_SIZE, ATTR_CACHED_MEM)

	/* DDR memory including SRAM in  MEMC for MMU table. Enable the maximum
	   2GB DDR for alias test. Use Device Attr to avoid CPU speculative fetch */
	mem_dev_table_entry(DRAM_BASE, DRAM_BASE, 0x80000000, ATTR_DEVICE)

	/* MEMC and DDR PHY control registers */
	mem_dev_table_entry(MEMC_BASE, MEMC_BASE, MEMC_SIZE, ATTR_DEVICE)

	/* PMC */
	mem_dev_table_entry(PMC_BASE, PMC_BASE, PMC_SIZE, ATTR_DEVICE)

	/* Process monitor */
	mem_dev_table_entry(PROC_MON_BASE, PROC_MON_BASE, PROC_MON_SIZE, ATTR_DEVICE)

	/* ubus mapped registers */
	mem_dev_table_entry(UBUS_MAPPED_BASE, UBUS_MAPPED_BASE, UBUS_MAPPED_SIZE, ATTR_DEVICE)
	mem_dev_table_entry(UBUS_MAPPED_XRDP_BASE, UBUS_MAPPED_XRDP_BASE, UBUS_MAPPED_XRDP_SIZE, ATTR_DEVICE)

	/* Runner SRAM for runtime memory */
	mem_dev_table_entry(RDP_SRAM_BASE, RDP_SRAM_BASE, RDP_SRAM_SIZE, ATTR_CACHED_MEM)
#endif

	/* all the peripheral device register */
	mem_dev_table_entry(PERF_BASE, PERF_BASE, PERF_SIZE, ATTR_DEVICE)
	mem_dev_table_entry(PERF1_BASE, PERF1_BASE, PERF1_SIZE, ATTR_DEVICE)

	/* SPI direct access window */
	mem_dev_table_entry(SPIFLASH_BASE, SPIFLASH_BASE, SPIFLASH_SIZE, ATTR_DEVICE)
#else
	/* DDR entries for cache and uncached virtual address, total size is a placehold
	   and will be filled in at run time  */
	mem_dev_table_entry(DRAM_BASE, DRAM_BASE, 0x00000000,  ATTR_CACHED_MEM)
	mem_dev_table_entry(DRAM_BASE|VA_FLAGS_NONCACHE_MEM, DRAM_BASE, 0x00000000,  ATTR_NONCACHED_MEM)

	/* peripheral SRAM for security credential */ 
	mem_dev_table_entry(PERF_SRAM_BASE, PERF_SRAM_BASE, PERF_SRAM_SIZE, ATTR_DEVICE)

	/* MEMC and DDR PHY control registers */
	mem_dev_table_entry(MEMC_BASE, MEMC_BASE, MEMC_SIZE, ATTR_DEVICE)

	/* XRDP block */
 	mem_dev_table_entry(XRDP_BASE, XRDP_BASE, 0x1800000, ATTR_DEVICE)

 	/* LPORT block */
  	mem_dev_table_entry(LPORT_BASE, LPORT_BASE, LPORT_SIZE, ATTR_DEVICE)

	/* all the peripheral device register */
	mem_dev_table_entry(PERF_BASE, PERF_BASE, PERF_SIZE, ATTR_DEVICE)
	mem_dev_table_entry(PERF1_BASE, PERF1_BASE, PERF1_SIZE, ATTR_DEVICE)
	mem_dev_table_entry(I2C_2_BASE, I2C_2_BASE, I2C_2_SIZE, ATTR_DEVICE)

	/* PMC */
	mem_dev_table_entry(PMC_BASE, PMC_BASE, PMC_SIZE, ATTR_DEVICE)

	/* Process monitor */
	mem_dev_table_entry(PROC_MON_BASE, PROC_MON_BASE, PROC_MON_SIZE, ATTR_DEVICE)
	
	/* ACE BIU registers */
	mem_dev_table_entry(BIUCFG_BASE, BIUCFG_BASE, BIUCFG_SIZE, ATTR_DEVICE)

	/*CCI 400*/
	mem_dev_table_entry(CCI400_BASE, CCI400_BASE , CCI400_SIZE, ATTR_DEVICE)
#endif
#endif /* #if defined(_BCM96858_) */


#if defined(_BCM96836_) || defined(_BCM96856_)
#if !defined(CFG_RAMAPP)  /* cfe rom and boot rom */
#if (INC_BTRM_BUILD==1)

	/* btrm */

	/* SRAM in MEMC for MMU table */
	mem_dev_table_entry(CPUCFG_MMU_TABLE_BASE, CPUCFG_MMU_TABLE_BASE, CPUCFG_MMU_TABLE_SIZE, ATTR_DEVICE)

	/* boot rom direct access window */
	mem_dev_table_entry(BOOTROM_BASE, BOOTROM_BASE, BOOTROM_SIZE, ATTR_CACHED_RO_MEM)

	/* peripheral SRAM for transit code and security credential */ 
	mem_dev_table_entry(PERF_SRAM_BASE, PERF_SRAM_BASE, PERF_SRAM_SIZE, ATTR_DEVICE)

	/* SRAM for bootrom runtime  */
	mem_dev_table_entry(0x90000000, 0x82600000, 0x6000, ATTR_CACHED_MEM)
	mem_dev_table_entry(0x90006000, 0x82606000, 0x6000, ATTR_CACHED_MEM)
	mem_dev_table_entry(0x9000c000, 0x8260c000, 0x6000, ATTR_CACHED_MEM)
	mem_dev_table_entry(0x90012000, 0x82612000, 0x1e000, ATTR_CACHED_MEM)

#else

	/* cfe rom */

	/* SRAM in MEMC for MMU table */
	mem_dev_table_entry(0xa0000000, CPUCFG_MMU_TABLE_BASE, CPUCFG_MMU_TABLE_SIZE, ATTR_DEVICE)

	/* peripheral SRAM in case cfe rom need to use */
	mem_dev_table_entry(PERF_SRAM_BASE, PERF_SRAM_BASE, PERF_SRAM_SIZE, ATTR_CACHED_MEM)

	/* DDR memory including SRAM in  MEMC for MMU table. Enable the maximum
	   2GB DDR for alias test. Use Device Attr to avoid CPU speculative fetch */
	mem_dev_table_entry(DRAM_BASE, DRAM_BASE, 0x80000000, ATTR_DEVICE)

	/* MEMC and DDR PHY control registers */
	mem_dev_table_entry(MEMC_BASE, MEMC_BASE, MEMC_SIZE, ATTR_DEVICE)

	/* PMC */
	mem_dev_table_entry(PMC_BASE, PMC_BASE, PMC_SIZE, ATTR_DEVICE)

	/* Process monitor */
	mem_dev_table_entry(PROC_MON_BASE, PROC_MON_BASE, PROC_MON_SIZE, ATTR_DEVICE)

	/* ubus mapped registers */
	mem_dev_table_entry(UBUS_SYS_MODULE_BASE, UBUS_SYS_MODULE_BASE, UBUS_MAPPED_SIZE, ATTR_DEVICE)

	/* Runner SRAM for runtime memory */
	mem_dev_table_entry(RDP_SRAM_BASE, RDP_SRAM_BASE, RDP_SRAM_SIZE, ATTR_CACHED_MEM)

	/* ACE BIU registers */
	mem_dev_table_entry(BIUCFG_BASE, BIUCFG_BASE, BIUCFG_SIZE, ATTR_DEVICE)

#endif

	/* btrm and cfe rom */

	/* all the peripheral device register */
	mem_dev_table_entry(PERF_BASE, PERF_BASE, PERF_SIZE, ATTR_DEVICE)
	mem_dev_table_entry(PERF1_BASE, PERF1_BASE, PERF1_SIZE, ATTR_DEVICE)

	/* SPI direct access window */
	mem_dev_table_entry(SPIFLASH_BASE, SPIFLASH_BASE, SPIFLASH_SIZE, ATTR_DEVICE)
#else

	/* cfe ram */
	/* DDR entries for cache and uncached virtual address, total size is a placehold
	   and will be filled in at run time  */
	mem_dev_table_entry(DRAM_BASE, DRAM_BASE, 0x00000000,  ATTR_CACHED_MEM)
	mem_dev_table_entry(DRAM_BASE|VA_FLAGS_NONCACHE_MEM, DRAM_BASE, 0x00000000,  ATTR_NONCACHED_MEM)

	/* MEMC and DDR PHY control registers */
	mem_dev_table_entry(MEMC_BASE, MEMC_BASE, MEMC_SIZE, ATTR_DEVICE)

	/* XRDP block */
	mem_dev_table_entry(XRDP_BASE, XRDP_BASE, XRDP_SIZE, ATTR_DEVICE)

	/* peripheral SRAM for security credential */ 
	mem_dev_table_entry(PERF_SRAM_BASE, PERF_SRAM_BASE, PERF_SRAM_SIZE, ATTR_DEVICE)

	/* all the peripheral device register */
	mem_dev_table_entry(PERF_BASE, PERF_BASE, PERF_SIZE, ATTR_DEVICE)
	mem_dev_table_entry(PERF1_BASE, PERF1_BASE, PERF1_SIZE, ATTR_DEVICE)

	/* PMC */
	mem_dev_table_entry(PMC_BASE, PMC_BASE, PMC_SIZE, ATTR_DEVICE)

	/* Process monitor */
	mem_dev_table_entry(PROC_MON_BASE, PROC_MON_BASE, PROC_MON_SIZE, ATTR_DEVICE)
	
	/* ACE BIU registers */
	mem_dev_table_entry(BIUCFG_BASE, BIUCFG_BASE, BIUCFG_SIZE, ATTR_DEVICE)

	/*CCI 400*/
	mem_dev_table_entry(CCI400_BASE, CCI400_BASE , CCI400_SIZE, ATTR_DEVICE)
#endif
#endif  /* #if defined(_BCM96836_) */


#if defined(_BCM963158_)
#if !defined(CFG_RAMAPP)  /* cfe rom and boot rom */
#if (INC_BTRM_BUILD==1)
	/* btrm only */
	/* boot rom direct access window */
	mem_dev_table_entry(BOOTROM_BASE, BOOTROM_BASE, BOOTROM_SIZE, ATTR_CACHED_RO_MEM)

	/* peripheral SRAM for transit code and security credential */ 
	mem_dev_table_entry(PERF_SRAM_BASE, PERF_SRAM_BASE, PERF_SRAM_SIZE, ATTR_DEVICE)

	/* Crypto registers (SPU, PDC, SKP) */
	mem_dev_table_entry(CRYPTO_BASE, CRYPTO_BASE, CRYPTO_SIZE, ATTR_DEVICE)
#else
	/* DDR memory. Enable the maximum 2GB DDR for alias test. Use Device Attr
	  to avoid CPU speculative fetch */
	mem_dev_table_entry(DRAM_BASE, DRAM_BASE, 0x80000000, ATTR_DEVICE)
	mem_dev_table_entry(DRAM_BASE_2, DRAM_BASE_2, 0x80000000, ATTR_DEVICE)
	
	/* MEMC and DDR PHY control registers */
	mem_dev_table_entry(MEMC_BASE, MEMC_BASE, MEMC_SIZE, ATTR_DEVICE)
#endif
	/* common btrm and cfe rom definitions */
	/* SRAM in MEMC or LMEM reserved for MMU table */
	mem_dev_table_entry(CPUCFG_MMU_TABLE_BASE, CPUCFG_MMU_TABLE_BASE, CPUCFG_MMU_TABLE_SIZE, ATTR_DEVICE)
	/* LMEM for bootrom runtime  */
	mem_dev_table_entry(DSLLMEM_BASE, DSLLMEM_BASE, 0xa0000, ATTR_CACHED_MEM)
	mem_dev_table_entry(DSLLMEM_BASE|VA_FLAGS_NONCACHE_MEM, DSLLMEM_BASE, 0xa0000, ATTR_NONCACHED_MEM)

        /* For credentials */
	mem_dev_table_entry(PERF_SRAM_BASE, PERF_SRAM_BASE, PERF_SRAM_SIZE, ATTR_DEVICE)

	/* SPI direct access window */
	mem_dev_table_entry(SPIFLASH_BASE, SPIFLASH_BASE, SPIFLASH_SIZE, ATTR_DEVICE)
#else   /* cfe ram */
	/* DDR entries for cache and uncached virtual address, total size is a placehold
	   and will be filled in at run time  */
	mem_dev_table_entry(DRAM_BASE, DRAM_BASE, 0x00000000,  ATTR_CACHED_MEM)
	mem_dev_table_entry(DRAM_BASE|VA_FLAGS_NONCACHE_MEM, DRAM_BASE, 0x00000000,  ATTR_NONCACHED_MEM)
	mem_dev_table_entry(DRAM_BASE_2, DRAM_BASE_2, 0x00000000,  ATTR_CACHED_MEM)
	mem_dev_table_entry(DRAM_BASE_2|VA_FLAGS_NONCACHE_MEM, DRAM_BASE_2, 0x00000000,  ATTR_NONCACHED_MEM)

	/* peripheral SRAM for security credential */ 
	mem_dev_table_entry(PERF_SRAM_BASE, PERF_SRAM_BASE, PERF_SRAM_SIZE, ATTR_DEVICE)

	/* MEMC and DDR PHY control registers */
	mem_dev_table_entry(MEMC_BASE, MEMC_BASE, MEMC_SIZE, ATTR_DEVICE)

	/* PMC */
	mem_dev_table_entry(PMC_BASE, PMC_BASE, PMC_SIZE, ATTR_DEVICE)

	mem_dev_table_entry(XPORT_BASE, XPORT_BASE, XPORT_SIZE, ATTR_DEVICE)
	mem_dev_table_entry(WAN_BASE, WAN_BASE, WAN_SIZE, ATTR_DEVICE)

	/* Process monitor */
	mem_dev_table_entry(PROC_MON_BASE, PROC_MON_BASE, PROC_MON_SIZE, ATTR_DEVICE)

	/* ACE BIU registers */
	mem_dev_table_entry(BIUCFG_BASE, BIUCFG_BASE, BIUCFG_SIZE, ATTR_DEVICE)

	/* CCI500 registers */
	mem_dev_table_entry(CCI500_BASE, CCI500_BASE, CCI500_SIZE, ATTR_DEVICE)

	/* UBUS4 map and configuration register. Add the whole range for CFE */
	mem_dev_table_entry(UBUS_MAPPED_BASE, UBUS_MAPPED_BASE, 0x500000, ATTR_DEVICE)
	 
#endif
	/* common btrm,  cfe rom and ram definitions */
	/* switch block registers */
	mem_dev_table_entry(SWITCH_BASE, SWITCH_BASE, SWITCH_SIZE, ATTR_DEVICE)

	/* System port registers */ 
	mem_dev_table_entry(SYSPORT_BASE, SYSPORT_BASE, SYSPORT_SIZE, ATTR_DEVICE)

	/* all the peripheral device registers */
	mem_dev_table_entry(PERF_BASE, PERF_BASE, PERF_SIZE, ATTR_DEVICE)
	mem_dev_table_entry(PERF1_BASE, PERF1_BASE, PERF1_SIZE, ATTR_DEVICE)
	mem_dev_table_entry(AHB_BASE, AHB_BASE, AHB_SIZE, ATTR_DEVICE)
	mem_dev_table_entry(AHB_BASE_A0, AHB_BASE_A0, AHB_SIZE, ATTR_DEVICE)
	mem_dev_table_entry(ARM_UART_BASE, ARM_UART_BASE, ARM_UART_SIZE, ATTR_DEVICE)
#endif  /* #if defined(_BCM963158_) */


mem_dev_table_end:
	.word	0x0

FUNC(armv8_mmuinit)

	mov	x14, lr
#if !defined(CFG_RAMAPP)
#if defined (MEMC_SRAM)
	// enable 64KB sram in MEMC controller for MMU table
	ldr	x0, =MEMC_BASE
	ldr	w1, =(CPUCFG_MMU_TABLE_BASE|0x00000040) // map to 0x7fff-0000, size = 64KB
	str	w1, [x0, MEMC_SRAM_REMAP_CONTROL] ;

	orr	w1, w1, #2      /* enable the map */
	str	w1, [x0, MEMC_SRAM_REMAP_CONTROL] ;

	ldr	w1, [x0, MEMC_SRAM_REMAP_CONTROL]
	dmb	ish
#endif
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


	/*  4K granularity, outer-shareable, Inner & Outer non-cachable memory and
	    MMU_PA_SIZE PA, MMU_TCR_T0SZ for PA, VA size field */
	ldr	x1, =((MMU_PA_SIZE<<TCR_EL3_PS_SHIFT)|(TCR_GRANULE_4K<<TCR_ELx_TG0_SHIFT)|(TCR_SHARE_OUTER<<TCR_ELx_SH0_SHIFT)|(MMU_TCR_T0SZ<<TCR_ELx_T0SZ_SHIFT))
	msr	TCR_EL3, x1
	isb

#if defined(CFG_RAMAPP)
	/* save the ddr size first */
	CALLINIT(=board_get_ddr_size)
	mov	x22, x0
	/* if total size large than 2GB, the ddr is splitted into two regions */
	mov	x1, #1
	lsl	x1, x1, #31
	subs	x23, x0, x1
	ble	no_split
	mov	x22, x1       /* lower memory 2GB in x22, upper in x23 */
	b	size_done 
no_split:
	mov	x23, #0      /* clear x23 and total memory in x22 */
size_done:
#endif
	
	/* go through memory and device register table and add it to mmu table */
	LOADREL(x20, =mem_dev_table)
	LOADREL(x21, =mem_dev_table_end)
	mov	x0, xzr
	mov	x1, xzr
	mov	x2, xzr
	mov	x3, xzr
mmu_init_loop:
	ldr	x0, [x20], #8
	ldr	x1, [x20], #8
	ldr	x2, [x20], #8
	ldr	x3, [x20], #8
#if defined(CFG_RAMAPP)	
	/* check if it is ddr entry or not */
	ldr	x4, =DRAM_BASE
	cmp	x0, x4
	bne	check_nc
	mov	x2, x22
	str	x2, [x20, #-16]
	b	add_entry
check_nc:
	ldr	x4, =(DRAM_BASE|VA_FLAGS_NONCACHE_MEM)
	cmp	x0, x4
	bne	check_mem2
	mov	x2, x22
	str	x2, [x20, #-16]
	b	add_entry
check_mem2:
#if defined(DRAM_BASE_2)	
	ldr	x4, =DRAM_BASE_2
	cmp	x0, x4
	bne	check_nc2
	cmp	x23, #0
	beq	skip_entry
	mov	x2, x23
	str	x2, [x20, #-16]
	b	add_entry
check_nc2:
	ldr	x4, =(DRAM_BASE_2|VA_FLAGS_NONCACHE_MEM)
	cmp	x0, x4
	bne	add_entry
	cmp	x23, #0
	beq	skip_entry
	mov	x2, x23
	str	x2, [x20, #-16]
#endif
add_entry:
#endif	
	CALLINIT(=armv8_mmu_add_entry)
	cbnz	x0, mmu_error
skip_entry:
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
	cmp	x2, #10
	blt	noadj
	add	x2, x2, #7        /* skip the ascii between 9 and A */
noadj:
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


FUNC(armv8_el2_mmuinit)
	mov	x14, lr
	ldr	x1, =MMU_L1_TABLE_BASE
	msr	TTBR0_EL2, x1
	isb
        
	/* Set up memory attributes
	   0 -> 0b01000100 = Normal, Inner/Outer Non-Cacheable
	   1 -> 0b11111111 = Normal, Inner/Outer WriteBack Read/Write Allocate
	   2 -> 0b00000000 = Device-nGnRnE
	*/
	mov	x1, #0x000000000000FF44
	msr	MAIR_EL2, x1
	isb

	/*  4K granularity, outer-shareable, Inner & Outer non-cachable memory and
	    MMU_PA_SIZE PA, MMU_TCR_T0SZ for PA, VA size field */
	ldr	x1, =((MMU_PA_SIZE<<TCR_EL3_PS_SHIFT)|(TCR_GRANULE_4K<<TCR_ELx_TG0_SHIFT)|(TCR_SHARE_OUTER<<TCR_ELx_SH0_SHIFT)|(MMU_TCR_T0SZ<<TCR_ELx_T0SZ_SHIFT))
	msr	TCR_EL2, x1
	isb
        
	mov	lr, x14
	ret
END(armv8_el2_mmuinit)
        
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
	mrs	x0, CurrentEL
	lsr	x0, x0, #2
	and	x0, x0, #3
	cmp	x0, #3
	beq	el3_enable_mmu

	mrs	x1, SCTLR_EL2
	orr	x1, x1, SCTLR_ELx_M
	msr	SCTLR_EL2, x1
	isb
	ret

el3_enable_mmu:
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
	mrs	x0, CurrentEL
	lsr	x0, x0, #2
	and	x0, x0, #3
	cmp	x0, #3
	beq	el3_disable_mmu

	mrs	x1, SCTLR_EL2
	bic	x1, x1, SCTLR_ELx_M
	msr	SCTLR_EL2, x1
	isb
	b	disable_mmu_done

el3_disable_mmu:
	mrs	x1, SCTLR_EL3
	bic	x1, x1, SCTLR_ELx_M
	msr	SCTLR_EL3, x1
	isb
        
disable_mmu_done:
#if !defined(CFG_RAMAPP)
	// disable 64KB sram in MEMC controller
#if defined (MEMC_SRAM)
	ldr	x0, =MEMC_BASE
	mov	w1, #0      /* disable the map */
	str	w1, [x0, MEMC_SRAM_REMAP_CONTROL] ;
	dmb	ish
#endif
#endif	
	ret

END(armv8_disable_mmu)

/*  *********************************************************************
    *  armv8_mmu_va2pa
    *
    *  translate va to pa
    *
    *  Input parameters:
    *	   x0 virtual address
    *      x1 pointer to the size
    *	
    *  Return value:
    *	   x0 physical address. return -1 if va is invalid
    *      and save the maximum continous physical memory in [x1]
    *	
    *  Registers used:
    *	   x0 - x7, x9
    ********************************************************************* */
FUNC(armv8_mmu_va2pa)
	/* save the lr register */
	mov	x7, x30
	mov	x6, x0
	mov	x9, x1
	
	LOADREL(x4, =mem_dev_table)
	LOADREL(x5, =mem_dev_table_end)
	mov	x0, xzr
	mov	x1, xzr
	mov	x2, xzr

table_loop1:
	ldr	x0, [x4], #8
	ldr	x1, [x4], #8
	ldr	x2, [x4], #16

	cmp	x6, x0
	blt	next_entry1
	
	add	x0, x0, x2
	cmp	x6, x0
	bge	next_entry1
	
	sub	x0, x0, x2
	sub	x6, x6, x0
	add	x0, x6, x1
	cbz	x9, done1
	sub	x2, x2, x6
	str	x2, [x9]
	b	done1
	
next_entry1:
	cmp	x4, x5
	bne	table_loop1
	/* can't find the entry, invalid input */
invalid1:
	mov	x0, #-1
done1:
	mov	x30, x7
	ret

END(armv8_mmu_va2pa)

/*  *********************************************************************
    *  armv8_mmu_pa2va
    *
    *  translate pa to va
    *
    *  Input parameters:
    *	   x0 physical address
    *	   x1 flags, what kind of va to return
    *	   x2 flag mask, what flag bits to check
    *  Return value:
    *	   x0 virtual address. return -1 if va is invalid
    *
    *  Registers used:
    *	   x0 - x7, x9, x10
    ********************************************************************* */
FUNC(armv8_mmu_pa2va)

	/* save the lr register */
	mov	x10, x30
	
	mov	x6, x0
	mov	x7, x1
	mov	x3, x2
	
	LOADREL(x4, =mem_dev_table)
	LOADREL(x5, =mem_dev_table_end)
	mov	x0, xzr
	mov	x1, xzr
	mov	x2, xzr

table_loop2:
	ldr	x0, [x4], #8
	ldr	x1, [x4], #8
	ldr	x2, [x4], #16

	cmp	x6, x1
	blt	next_entry2
	
	add	x1, x1, x2
	cmp	x6, x1
	bge	next_entry2

	/* check flags to ensure it is the type we need such as uncached va */
	and	x9, x0, x3
	cmp	x9, x7
	bne	next_entry2

	sub	x1, x1, x2
	sub	x6, x6, x1
	add	x0, x6, x0
	b	done2
	
next_entry2:
	cmp	x4, x5
	bne	table_loop2
	/* can't find the entry, invalid input */
invalid2:
	mov	x0, #-1
done2:
	mov	x30, x10
	ret

END(armv8_mmu_pa2va)


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

#include "armv8.h"
#include "exception.h"
#include "cfe.h"
#include "bsp_config.h"
#include "cpu_config.h"
#include "bcm_map.h"
#ifdef _CFE_
#include "cfe_devfuncs.h"
#else
#define cfe_command_restart 0
#endif
/* BCM63XX specific change. */
#include "bcm_hwdefs.h"
#include "armmacros.h"

/* Do NOT use these register for any other purpose */
#define BOOTOFFSET	 x27
#define ROMOPTION	 x26

/*
 * This is the size of the stack, rounded to KByte boundaries.
 */

#ifndef CFG_STACK_SIZE
#error "CFG_STACK_SIZE not defined"
#else
#define STACK_SIZE	((CFG_STACK_SIZE+1023) & ~1023)
#endif


/* Entry point. Assume MMU, cache is off.cfe rom entry code is PIC.
   can run at any physical address and relocate itself to linked address */
	.section .text.startup

	.global startup
startup:
	mov	x0, xzr
	mov	x1, xzr
	mov	x2, xzr
	mov	x3, xzr
	mov	x4, xzr
	mov	x5, xzr
	mov	x6, xzr
	mov	x7, xzr
	mov	x8, xzr
	mov	x9, xzr
	mov	x10, xzr
	mov	x11, xzr
	mov	x12, xzr
	mov	x13, xzr
	mov	x14, xzr
	mov	x15, xzr
	mov	x16, xzr
	mov	x17, xzr
	mov	x18, xzr
	mov	x19, xzr
	mov	x20, xzr
	mov	x21, xzr
	mov	x22, xzr
	mov	x23, xzr
	mov	x24, xzr
	mov	x25, xzr
	mov	x26, xzr
	mov	x27, xzr
	mov	x28, xzr
	mov	x29, xzr
	mov	x30, xzr
	mov	sp, x0
	msr	elr_el3, x0

	b	tr_rst	/* 0 - reset */


	.align 3
#include "initdata.h"
#include "segtable.h"

#if defined(CONFIG_BRCM_IKOS)
/* need to skip this address range because the bench is writing to it */
	.org	0x120
	.word	1    
	.org	0x140
#endif

	.globl segment_table
segment_table:
	_LONG_	text_end			/* [  0] End of text (R_SEG_ETEXT) */
	_LONG_	data_start			/* [  1] Beginning of data (R_SEG_FDATA) */
	_LONG_	data_end			/* [  2] End of data (R_SEG_EDATA) */
	_LONG_	_end				/* [  3] End of BSS (R_SEG_END) */
	_LONG_	text_start			/* [  4] Beginning of text (R_SEG_FTEXT) */
	_LONG_	bss_start			/* [  5] Beginning of BSS (R_SEG_FBSS) */
	_LONG_	0				/* [  6] Global Pointer (R_SEG_GP) */
	_LONG_	0				/* [  7] Beginning of reloc entries */
	_LONG_	0				/* [  8] End of reloc entries */
#if !defined(CFG_RAMAPP)
	_LONG_	0				/* [  9] R_SEG_APIENTRY */
#else
	_LONG_	cpu_apientry			/* [  9] R_SEG_APIENTRY */
#endif

/*************************************************************************
 *
 * Startup Code (reset vector)
 *
 * do important init only if we don't start from memory!
 * setup Memory and board specific bits prior to relocation.
 * relocate armboot to ram
 * setup stack
 *
 *************************************************************************/

/*
 * the actual reset code
 */
tr_rst:
	/* check cpu exception level CFE always start at EL3 */
	mrs	x0, CurrentEL
	lsr	x0, x0, #2
	cmp	x0, #3
	beq	cont0
1:
	b	1b			/* stop the cpu */

cont0:

	adr	x0,startup		/* x0: source start address */
	ldr	x1,=startup		/* x1: target address */
	sub	BOOTOFFSET, x0, x1      /* save in bootoffst reg*/

	CALLINIT(=armv8_cpuinit)    /* enable and invalidate i cache, disable d cache and mmu */
#if defined(CONFIG_BRCM_IKOS)
	CALLINIT(=armv8_l1cache_enable_i)
#endif	
#if !defined(CFG_RAMAPP)
	/* common early init code between btrm/cferom/ram such as uart and etc*/
	CALLINIT(=board_earlyinit_common)
	/* chip specific early init for cfe rom */
	CALLINIT(=board_earlyinit)
#endif

	SETLEDS('H','E','L','O')
#if !defined(CFG_RAMAPP)
#if !defined(IKOS_NO_DDRINIT)
#if defined(IKOS_SMPL_DDRINIT)
	SETLEDS('D','D','R','I')
	CALLINIT(=ddr_init)
#endif
#else
	/* wait for ddr to be done */
	ldr	x0, =MEMC_BASE
wait_done:	
	ldr	w1, [x0, #MEMC_GLB_GCFG]
	and	w1, w1, #MEMC_GLB_GCFG_MEM_INIT_DONE
	cbz	w1, wait_done

#if defined(_BCM96836_) || defined(_BCM963158_) || defined(_BCM96856_)
	ldr	w2, [x0, #MEMC_GLB_FSBL_STATE]
	and	w2, w2, #~MEMC_GLB_FSBL_DRAM_SIZE_MASK
	mov	w1, #(8<<MEMC_GLB_FSBL_DRAM_SIZE_SHIFT)
	orr	w2, w2, w1
	str	w2, [x0, #MEMC_GLB_FSBL_STATE]
#else
	/* hard code 64MB ddr size */
	ldr	w2, [x0, #MEMC_GLB_GCFG]
	and	w2, w2, #~MEMC_GLB_GCFG_DRAM_SIZE1_MASK
	mov	w1, #5
	orr	w2, w2, w1
	str	w2, [x0, #MEMC_GLB_GCFG]
#endif
#endif
#endif

#if (BOOT_PRE_CFE==0)
#if !defined(CFG_RAMAPP)
	LOADREL(x0,=rel_version)
	CALLINIT(=board_puts)
#endif
	SETLEDS('C','P','U','0')
#endif

	/* Cycle Count */
	CALLINIT(=armv8_enable_tick) 

#if !defined(CFG_RAMAPP) && defined(IKOS_BD_CFERAM)
	/* invalid I cache to make the backdoor-loaded code is visible*/
	CALLINIT(=armv8_l1cache_inval_i)
	CALLINIT(=armv8_l1cache_disable_i)
	mov	x0,#0x1000000
	br	x0
#endif

#if !defined(CONFIG_BRCM_IKOS) /* ARMv8 invalid cache by hardware on reset */
    SETLEDS('L','1','C','D')
    /* invalid tlb, done in mmuinit. invalid d cache */
    CALLINIT(=armv8_l1cache_inval_d) 
    CALLINIT(=armv8_l1cache_inval_i) 
#endif

    /* enable MMU and then enable data cache */
    SETLEDS('M','M','U','I')
    CALLINIT(=armv8_mmuinit)
    CALLINIT(=armv8_enable_mmu)
    CALLINIT(=armv8_l1cache_enable_i)
    CALLINIT(=armv8_l1cache_enable_d)

    cmp	BOOTOFFSET, #0
    beq	clear_bss

	/* Copy code and data  to RAM */
	SETLEDS('C','O','D','E')
#if defined(_BCM96858_)
#if !defined(CFG_RAMAPP) && (BOOT_PRE_CFE==1)
	ldr x1, =0x80180020
	ldr x0, =0x60000042
	str x0, [x1]
#endif
#endif
copy2ram:
	LOADREL(x4,=segment_table)
	ldr	x1, [x4,#R_SEG_FTEXT]   /* x1: target address */
	add	x0, x1, BOOTOFFSET	/* x0: source start address */
#if (BOOT_PRE_CFE==1)
	add x2, x0, #65536
#else
	ldr	x2, [x4,#R_SEG_EDATA]
	add	x2, x2, BOOTOFFSET	/* x2: source end address */
#endif
copy_loop:
	ldp	w3, w4, [x0], #8	/* copy from source address [x0] */
	stp	w3, w4, [x1], #8	/* copy from source address [x1] */
	cmp	x0,x2			/* until source end address [x2] */
	blo	copy_loop

clear_bss:
	SETLEDS('Z','B','B','S')
	LOADREL(x4,=segment_table)
	/* Clear BSS */
	ldr	x0, [x4,#R_SEG_FBSS]	/* x0: bss start address */
	ldr	x1, [x4,#R_SEG_END]	/* x0: bss end address */
	mov	w2, #0x0
	mov	w3, #0x0
	/* empty bss section checking */
	cmp	x0, x1
	beq	call_c_main

clbss_l:
	stp	w2, w3, [x0], #8
	cmp	x0, x1
	blo	clbss_l

/* Set stackpointer in internal RAM to call c_main */
call_c_main:
	SETLEDS('M','A','I','N')

#if !defined(CFG_RAMAPP)
	/* cfe rom build for internal mem */
#if defined(_BCM94908_)
	mov	x0,#128			/* memory size in Kbytes */
#elif defined(_BCM963158_)
	mov	x0,#448	
#elif defined(_BCM96858_) 
#if (BOOT_PRE_CFE==1)
	mov	x0, #64
#else
	mov	x0, #384
#endif
#elif defined(_BCM96836_)
	mov	x0, #120
#elif defined(_BCM96856_)
    mov x0, #184
#endif
#else
	CALLINIT(=board_get_ddr_size)
	mov	x2, xzr
	add	w2, w2, #10                  /* memory size in Kbytes */
	lsr	x0, x0, x2
#endif

	ldr	x1,=mem_totalsize
	str	x0,[x1]

#if defined(CFE_ABORT_KEY)
#if !defined(CFG_RAMAPP)
	CALLINIT(=chek_abort_key)
	mov	ROMOPTION, x0
	ldr	x1,=rom_option
	str	ROMOPTION,[x1]
#endif
#endif

	/* not relocating, no offset */
	ldr	x0, =0
	ldr	x1, =mem_datareloc
	str	x0, [x1]
	ldr	x1, =mem_textreloc
	str	x0, [x1]

	LOADREL(x4,=segment_table)
	ldr	x0, [x4,#R_SEG_FTEXT]
	ldr	x1, =mem_bottomofmem
	str	x0, [x1]

	ldr	x2, [x4,#R_SEG_ETEXT]
	sub	x2, x2, x0
	ldr	x1, =mem_textsize
	str	x2, [x1]
	ldr	x1, =mem_textbase
	str	x0, [x1]

	ldr	x0, [x4,#R_SEG_END]	/* r0: bss end address */
	ldr     x1, =mem_stackbottom
	str     x0, [x1]
	add	x0, x0, #STACK_SIZE 
	ldr	x1, =mem_heapstart
	str	x0, [x1]
	ldr     x1, =mem_stacktop
	str     x0, [x1]
#if !defined(CFG_RAMAPP)
	ldr	x1, =(CFG_ROM_HEAP_SIZE*1024)
#else
	ldr	x1, =(CFG_HEAP_SIZE*1024)
#endif
	add	x0, x0, x1
	ldr	x1, =mem_topofmem
	str	x0, [x1]
	
	b	main

	.org	0x56c
	.globl	_cferom_size
_cferom_size:
	.word	_romsize

	.org	0x570
	.byte	'c','f','e','-','v',CFE_VER_MAJOR,CFE_VER_MINOR,CFE_VER_BUILD,BCM63XX_MAJOR,BCM63XX_MINOR  // CFE version info for applications
	.org	0x580			/*  move past exception vectors */
	/*
	 * BCM963XX NVRAM Data Storage. No internal bootrom is in play
	 */

	.globl nvram_data_storage
nvram_data_storage:
	.word	NVRAM_DATA_ID
	.space	0x400

main:
#if !defined(CFG_RAMAPP)
	/* invalid I cache and flush D cache to make the relocated code is visible*/
	CALLINIT(=armv8_l1cache_flush_d)
	CALLINIT(=armv8_l1cache_inval_i)

	ldr	x1, =gorelo
	br	x1           /* Now jump to an address code was compiled for */

gorelo:
	nop
	mov	BOOTOFFSET, #0      /* no longer running at offset */
#endif

	/* init exception support */
	CALLINIT(=armv8_exception_init)

#if defined(CFG_RAMAPP) || (BOOT_PRE_CFE==0)
	ldr	x1,=mem_stacktop
	ldr	x0,[x1]
	sub	x0,x0,#8
	mov	x1, #15
	bic	x0, x0, x1
	mov	sp, x0 /* 16-byte alignment for ABI compliance */
#else
	ldr x0, =0xfff86000
	mov sp, x0
#endif

	/* run in linked adress and SP is setup, no more call to CALLINIT, SETLED after this point */

	/* setup parameters for c_main */
	/* CFE: clear argument a & b in cfe_main */
	ldr	w0,=0x00000001
	ldr	w1,=0x00000002

	bl      cfe_main
die:
	b	die

/*  *********************************************************************
    *  CFE_FLUSHCACHE
    *
    *  Perform certain cache operations
    *
    *  Input parameters:
    *      w0 - flags (CFE_CACHE_xxx flags, or zero for a default flush d
    *      and invalidate i cache
    *      x1,x2 - start/end of range for "range invalidate" operations
    *      (not used otherwise)
    *
    *  Return value:
    *      nothing
    ********************************************************************* */

FUNC(_cfe_flushcache)

	str	lr, [sp, #-16]!  /* pre index and sp must be 16 bytes aligned*/
	bl	armv8_cacheops
	ldr	lr, [sp], #16

	ret

END(_cfe_flushcache)

/*  *********************************************************************
    *  get_SP()
    *  Needed to address stack safety check
    *  Returns current SP
    *  Input parameters:
    *  	   nothing
    *
    *  Return value:
    *      Current SP value
    **********************************************************************/
FUNC(get_SP)
	mov	x0, sp
	ret		/* back to my caller */
END(get_SP)

/*  *********************************************************************
    *  CFE_LAUNCH
    *
    *  Start the user program.  The program is passed a handle
    *  that must be passed back when calling the firmware.
    *
    *  Parameters passed to the called program are as follows:
    *
    *      x0 - CFE handle
    *      x1 - entry vector
    *      x2 - reserved, will be 0
    *      x3 - entrypoint signature.
    *
    *  Input parameters:
    *	   x0 - entry vector
    *
    *  Return value:
    *	   does not return
    ********************************************************************* */

FUNC(cfe_launch)

	/* entry save to x24, no need to worry about the abi register usage
	here as we will never return back to the caller */
	mov	x24, x0

	/* Mask all interrupts. */
	mrs	x4, DAIF
	orr	x4, x1, #(DAIF_I|DAIF_F)
	msr	DAIF, x4

#if (!defined(CFG_RAMAPP)) && defined(_BCM94908_) 

	/* cfe rom va != pa, need to copy the mmu disabler code to a flat mapped region and execute from there */
	adr	x0, mmu_dis_begin
	ldr	x1, =BTRM_INT_MEM_MMU_DIS_ADDR
	adr	x2, mmu_dis_end
1:
	ldp	w3, w4, [x0], #8
	stp	w3, w4, [x1], #8
	cmp	x0, x2
	blo	1b

        /* If mfg-secure or full-secure boot, copy shredder code to it's execution location */
        bl      otp_is_boot_secure 
        mov     w1,#1
        cmp     w0,w1
        bne     no_shrd
        ldr     x1, =BTRM_INT_MEM_SHREDDER_TRANSIT_PROG_ADDR /* x1: target address for shredder transict code relocation */
        ldr     x0, =__shredBegin                            /* x0: Shredder transit code start */
        ldr     x2, =__shredEnd                              /* x2: Shredder transit code end */
1:
        ldp     w3, w4, [x0], #8                             /* copy from source address [x0] */
        stp     w3, w4, [x1], #8                             /* copy to destinat address [x1] */
        cmp     x0, x2                                       /* until target end address [x2] */
        blo     1b

#endif

	/* Flush the D-Cache, Invalidate I-cache */
no_shrd:
	mov	w0, #0
	bl	_cfe_flushcache

#if defined(CFG_RAMAPP)

	/* Disable the D-Cache, MMU and I-Cache bit */
	bl	armv8_l1cache_disable_d
	bl	armv8_disable_mmu
	bl	armv8_l1cache_disable_i


	/* for linux boot only   ... */
	/* setup gic while still in secure mode */
	bl	armv8_gic_secure_init

	/* check if we need to drop to aarch 32 */
	ldr	x0, =g_switch2aarch32
	ldr	w1, [x0]
	cbz	w1, 1f
	
	/*  no traps or VM modifications from the Hypervisor, EL1 is AArch32 */
	msr	HCR_EL2, xzr
	
	/* drop to el1 nosecure aarch32 */
	mov     x4, x24
	ldr	x0, =start_drop_aarch32
	bl	armv8_drop_to_el1_aarch32_svc

start_drop_aarch32:
	.word 0xe3a00000 /* mov r0, #0			-> mov	r0, #0		*/   
	.word 0xe59f1010 /* ldr r1, =BCM63XX_MACH_ID	-> ldr	r1, [pc, #16]	*/	
	.word 0xe3a02c01 /* ldr r2, =ARM_ATAG_LOC	-> mov	r2, #256	*/	
	.word 0xe59f300c /* ldr r3, =CFE_EPTSEAL	-> ldr	r3, [pc, #12]	*/ 
	.word 0xe1a00000 /* nop				-> nop 			*/
	.word 0xe1a0f004 /* mov pc, r4			-> mov	pc, r4		*/       
	.word 0x00f00068 /* DUMMY			*/	
	.word 0x00002709 /* BCM63XX_MACH_ID		*/
	.word 0x43464531 /* CFE_EPTSEAL			*/
1:
	/* drop to el2 nosecure aarch64 */
	ldr	x0, =start_drop
	bl	armv8_drop_to_el2

start_drop:
	mov	x0, #CFG_DTB_ADDRESS
	mov	x1, xzr
	mov	x2, xzr
	mov	x3, xzr

	/* everything is cleared, jump to target entry pointed */
	br	x24
	

#else  /* CFE ROM */
#if defined (MEMC_SRAM)	
	/* save the memc base to register x4 */
	ldr	x4, =MEMC_BASE
#endif
#if defined(_BCM94908_)

	/* Will be jumping to the relocated mmu disable code shortly */
	/* Use ldr now for convenience before PIC comes into effect  */



        /* If mfg-secure or full-secure boot, the shredder start address will be in x23 */
	/* Otherwise, x23 will be zero */
	mov	x23, xzr
        bl      otp_is_boot_secure 
        mov     w1,#1
        cmp     w0,w1
        bne     not_secure_1
	ldr	x23, =BTRM_INT_MEM_SHREDDER_TRANSIT_PROG_ADDR

not_secure_1:
	/* jump to the relocated mmu disable code and do the cleanup
	   work in the flat mappped memory region */
	ldr	x0, =BTRM_INT_MEM_MMU_DIS_ADDR
	br	x0

#endif

#endif /* defined(CFG_RAMAPP) */


#if BOOT_PRE_CFE==0 || ( BOOT_PRE_CFE==1 && defined(_BCM94908_)) || defined(CFG_RAMAPP)

mmu_dis_begin:  /* beginning of mmu disable code that will be copied over */

	mrs	x1, SCTLR_EL3
	bic	x1, x1, #SCTLR_ELx_C
	bic	x1, x1, #SCTLR_ELx_M
	bic	x1, x1, #SCTLR_ELx_I
	msr	SCTLR_EL3, x1
	isb

	/* clear mmu tables */
	/* Use 16bit opcodes so the code remains relocatable */
	mov 	x6, #(CPUCFG_MMU_TABLE_BASE & 0x0000ffff)
	movk	x6, #(CPUCFG_MMU_TABLE_BASE  >> 16), LSL #16 
	mov	x7, #(CPUCFG_MMU_TABLE_SIZE/8 & 0x0000ffff)
	movk	x7, #(CPUCFG_MMU_TABLE_SIZE/8  >> 16), LSL #16 
	mov	x8, x6
zerotbL:
	str	xzr, [x8], #8
	subs	x7, x7, #1
	bne	zerotbL

#if defined (MEMC_SRAM)	
	/* disable 64KB sram in MEMC controller */
	mov	w1, #0      /* disable the map */
	str	w1, [x4, MEMC_SRAM_REMAP_CONTROL] ;
#endif
#if (!defined(CFG_RAMAPP)) && defined(_BCM94908_)
        /* If x23 is zero, it is an unsecure boot and just jump to cferam entry point */
	/* stored in x24. If x23 is non-zero, it is a mfg or field secure boot, and   */
        /* x23 contains the shredder entry point that needs to run first */
        cmp     xzr, x23
        beq     not_secure_2
	br	x23 /* mfg or fld secure boot. Jump to shredder code */
not_secure_2:
#endif
#endif
	br	x24 /* unsecure boot ... jump to target entry point  */



mmu_dis_end:     /* end of actual mmu disable code that will be copied over */

#if defined(_BCM94908_)
__shredBegin:
	
	/* shred 32k runner */
        mov     x0, #(BTRM_INT_SRAM_RDP_32K_ADDR & 0x0000ffff)
        movk    x0, #(BTRM_INT_SRAM_RDP_32K_ADDR >> 16 ), LSL #16
        mov     x1, #((BTRM_INT_SRAM_RDP_32K_ADDR + BTRM_INT_SRAM_RDP_32K_SIZE) & 0x0000ffff)
        movk    x1, #((BTRM_INT_SRAM_RDP_32K_ADDR + BTRM_INT_SRAM_RDP_32K_SIZE) >> 16 ), LSL #16
1:      str     xzr, [x0], #8
        cmp     x0, x1
        blo     1b

	/* shred 1st 48k runner */
        mov     x0, #(BTRM_INT_SRAM_RDP_48K_0_ADDR & 0x0000ffff)
        movk    x0, #(BTRM_INT_SRAM_RDP_48K_0_ADDR >> 16 ), LSL #16
        mov     x1, #((BTRM_INT_SRAM_RDP_48K_0_ADDR + BTRM_INT_SRAM_RDP_48K_SIZE) & 0x0000ffff)
        movk    x1, #((BTRM_INT_SRAM_RDP_48K_0_ADDR + BTRM_INT_SRAM_RDP_48K_SIZE) >> 16 ), LSL #16
1:      str     xzr, [x0], #8
        cmp     x0, x1
        blo     1b

	/* shred 2nd 48k runner */
        mov     x0, #(BTRM_INT_SRAM_RDP_48K_1_ADDR & 0x0000ffff)
        movk    x0, #(BTRM_INT_SRAM_RDP_48K_1_ADDR >> 16 ), LSL #16
        mov     x1, #((BTRM_INT_SRAM_RDP_48K_1_ADDR + BTRM_INT_SRAM_RDP_48K_SIZE) & 0x0000ffff)
        movk    x1, #((BTRM_INT_SRAM_RDP_48K_1_ADDR + BTRM_INT_SRAM_RDP_48K_SIZE) >> 16 ), LSL #16
1:      str     xzr, [x0], #8
        cmp     x0, x1
        blo     1b

	/* shred 128k runner */
        mov     x0, #(BTRM_INT_SRAM_RDP_128K_ADDR & 0x0000ffff)
        movk    x0, #(BTRM_INT_SRAM_RDP_128K_ADDR >> 16 ), LSL #16
        mov     x1, #((BTRM_INT_SRAM_RDP_128K_ADDR + BTRM_INT_SRAM_RDP_128K_SIZE) & 0x0000ffff)
        movk    x1, #((BTRM_INT_SRAM_RDP_128K_ADDR + BTRM_INT_SRAM_RDP_128K_SIZE) >> 16 ), LSL #16
1:      str     xzr, [x0], #8
        cmp     x0, x1
        blo     1b

        /* If authentication failed (x24 = 0), open jtag and loop forever */
        /* If authentication passed (x24!= 0), jump to cferam entry vector*/
        cmp     xzr, x24
        beq     auth_fail
        blr     x24 	/* Launch CFE RAM code. never to return */

auth_fail:
	/* shred credentials */
        mov     x0, #(BTRM_INT_MEM_CREDENTIALS_ADDR & 0x0000ffff)
        movk    x0, #(BTRM_INT_MEM_CREDENTIALS_ADDR >> 16 ), LSL #16
        mov     x1, #(BTRM_INT_MEM_CREDENTIALS_ADDR_END & 0x0000ffff)
        movk    x1, #(BTRM_INT_MEM_CREDENTIALS_ADDR_END >> 16 ), LSL #16
1:      str     xzr, [x0], #8
        cmp     x0, x1
        blo     1b

        /* open jtag, and then loop forever */
        mov     x0, #(BROM_GEN_BASE & 0x0000ffff)
        movk    x0, #(BROM_GEN_BASE >> 16 ), LSL #16
        mov     x1, #(BROM_GEN_SECBOOTCFG_INTF_UNLOCK)
        str     x1, [x0, #BROM_GEN_SECBOOTCFG]
loop_forever:
        b       loop_forever

__shredEnd:
#endif

END(cfe_launch)



FUNC(cpu_apientry)
	ret
END(cpu_apientry)

#if !defined(CFG_RAMAPP)
	.globl rel_version
rel_version:
	.ascii BRCM_VERSION,".",BRCM_RELEASE BRCM_EXTRAVERSION
	.ascii "-",CFE_VER_MAJ_STR,".",CFE_VER_MIN_STR,".",CFE_VER_ECO_STR
	.ascii "-",CFE_MAJOR_STR,".",CFE_MINOR_STR
	.ascii "\r"
	.ascii "\n"
	.byte  0x0
	.align 2
#endif

#if defined(CFG_RAMAPP)
    .align 3
save_stack:
    .dword 0

FUNC(cfe_save_context)
    /* stack all corruptible registers */
    stp x0,  x1,  [sp, #-16]!
    stp x2,  x3,  [sp, #-16]!
    stp x4,  x5,  [sp, #-16]!
    stp x6,  x7,  [sp, #-16]!
    stp x8,  x9,  [sp, #-16]!
    stp x10, x11, [sp, #-16]!
    stp x12, x13, [sp, #-16]!
    stp x14, x15, [sp, #-16]!
    stp x16, x17, [sp, #-16]!
    stp x18, x19, [sp, #-16]!
    stp x20, x21, [sp, #-16]!
    stp x22, x23, [sp, #-16]!
    stp x24, x25, [sp, #-16]!
    stp x26, x27, [sp, #-16]!
    stp x28, x29, [sp, #-16]!
    stp x29, x30, [sp, #-16]!
    /* Just remember the stack pointer */
    ldr x0,=save_stack
    mov x1, sp
    str x1, [x0]
    ret
END(cfe_save_context)

FUNC(cfe_restore_context)
    /* Retrieve the stack pointer */
    ldr x0,=save_stack
    ldr x1, [x0]
    mov sp, x1
    ldp x29, x0,  [sp], #16
    ldp x28, x29, [sp], #16
    ldp x26, x27, [sp], #16
    ldp x24, x25, [sp], #16
    ldp x22, x23, [sp], #16
    ldp x20, x21, [sp], #16
    ldp x18, x19, [sp], #16
    ldp x16, x17, [sp], #16
    ldp x14, x15, [sp], #16
    ldp x12, x13, [sp], #16
    ldp x10, x11, [sp], #16
    ldp x8,  x9,  [sp], #16
    ldp x6,  x7,  [sp], #16
    ldp x4,  x5,  [sp], #16
    ldp x2,  x3,  [sp], #16
    ldp x0,  x1,  [sp], #16
    ret
END(cfe_restore_context)

/*  *********************************************************************
    *  CFE_NONSECURE
    *
    *  Switches CFE into non-secure mode
    *
    *  Return value:
    *     void
    ********************************************************************* */
FUNC(cfe_nonsecure)
    /* Save the return address */
    stp x29, x30, [sp, #-16]!
    /* Save secure CFE context */
    bl cfe_save_context

    /* Mask all interrupts. */
    mrs x4, DAIF
    orr x4, x1, #(DAIF_I|DAIF_F)
    msr DAIF, x4

    mov w0, #0
    bl _cfe_flushcache

    /* Disable the D-Cache, MMU and I-Cache bit */
    bl armv8_l1cache_disable_d
    bl armv8_disable_mmu
    bl armv8_l1cache_disable_i
    ldr x0,=cfe_nonsecure_return
    bl armv8_drop_to_el2
cfe_nonsecure_return:
    /* Enable counters */
    mrs x0, PMCCFILTR_EL0
    orr x0, x0, #(1 <<27)
    msr PMCCFILTR_EL0, x0
    mrs x0, MDCR_EL2
    orr x0, x0, #(1 << 7)
    msr MDCR_EL2, x0
    /* Reconfigure MMU/Cache/etc..*/
    CALLINIT(=armv8_l1cache_inval_d)
    CALLINIT(=armv8_l1cache_inval_i)
    CALLINIT(=armv8_el2_mmuinit)
    CALLINIT(=armv8_enable_mmu)
    CALLINIT(=armv8_l1cache_enable_d)
    CALLINIT(=armv8_l1cache_enable_i)
    /* Use secure CFE context in non-secure world */
    bl cfe_restore_context
    /* restore the return address */
    ldp x29, x30,  [sp], #16
    ret
END(cfe_nonsecure)

/*  *********************************************************************
    *  CFE_SECUREOS
    *
    *  Launches secure OS and return back to caller in non-secure mode
    *
    *  Return value:
    *     void
    ********************************************************************* */

FUNC(cfe_secureos)
    /* Save the return address */
    stp x29, x30, [sp, #-16]!
    /* Save secure CFE context */
    bl cfe_save_context

    /* Mask all interrupts. */
    mrs x4, DAIF
    orr x4, x1, #(DAIF_I|DAIF_F)
    msr DAIF, x4

    mov w0, #0
    bl _cfe_flushcache

    /* Disable the D-Cache, MMU and I-Cache bit */
    bl armv8_l1cache_disable_d
    bl armv8_disable_mmu
    bl armv8_l1cache_disable_i

    /* Make sure system is in secure mode */
    mrs x0, scr_el3
    bic x0, x0, #1
    msr scr_el3, x0
    /* Make sure SMP bit is cleared */
    mrs x0, S3_1_C15_C2_1
    bic x0, x0, #(1 << 6)
    msr S3_1_C15_C2_1, x0
    /* Pass DTB and non-secure entry address to CFE */
    mov x0, #CFG_DTB_ADDRESS
    ldr x1, =cfe_secureos_return
    mov x2, xzr
    mov x3, xzr
    /* everything is cleared, jump to entry of ATF */
    ldr x24, =g_atf_addr
    ldr x24, [x24]
    lsl x24, x24, #32
    lsr x24, x24, #32
    /* Jump to ATF */
    br x24
    b .
cfe_secureos_return:
    /* Enable counters */
    mrs x0, PMCCFILTR_EL0
    orr x0, x0, #(1 <<27)
    msr PMCCFILTR_EL0, x0
    mrs x0, MDCR_EL2
    orr x0, x0, #(1 << 7)
    msr MDCR_EL2, x0
    /* Reconfigure MMU/Cache/etc..*/
    CALLINIT(=armv8_l1cache_inval_d)
    CALLINIT(=armv8_l1cache_inval_i)
    CALLINIT(=armv8_el2_mmuinit)
    CALLINIT(=armv8_enable_mmu)
    CALLINIT(=armv8_l1cache_enable_d)
    CALLINIT(=armv8_l1cache_enable_i)
    /* Use secure CFE context in non-secure world */
    bl cfe_restore_context
    /* restore the return address */
    ldp x29, x30,  [sp], #16
    ret
END(cfe_secureos)
#endif /* defined(CFG_RAMAPP) */

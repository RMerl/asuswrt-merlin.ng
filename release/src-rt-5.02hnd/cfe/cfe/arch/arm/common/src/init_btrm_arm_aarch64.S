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
#include "bcm_common.h"
#ifdef _CFE_
#include "cfe_devfuncs.h"
#else
#define cfe_command_restart 0
#endif
/* BCM63XX specific change. */
#include "bcm_hwdefs.h"
#include "armmacros.h"

/* Do NOT use these register for any other purpose */
#define ROMOPTION    x26

/* FIXME: Move to centralized location for V8 */
#undef CFG_STACK_SIZE
#undef CFG_ROM_HEAP_SIZE
#undef CFG_HEAP_SIZE
#define CFG_STACK_SIZE      32768
#define CFG_ROM_HEAP_SIZE   16
#define CFG_HEAP_SIZE       16

/*
 * This is the size of the stack, rounded to KByte boundaries.
 */

#ifndef CFG_STACK_SIZE
#error "CFG_STACK_SIZE not defined"
#else
#define STACK_SIZE  ((CFG_STACK_SIZE+1023) & ~1023)
#endif


/* Entry point. Assume MMU, cache is off. */
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

	fmov    d0,  xzr
	fmov    v0.d[1], xzr
	fmov    d1,  xzr
	fmov    v1.d[1], xzr
	fmov    d2,  xzr
	fmov    v2.d[1], xzr
	fmov    d3,  xzr
	fmov    v3.d[1], xzr
	fmov    d4,  xzr
	fmov    v4.d[1], xzr
	fmov    d5,  xzr
	fmov    v5.d[1], xzr
	fmov    d6,  xzr
	fmov    v6.d[1], xzr
	fmov    d7,  xzr
	fmov    v7.d[1], xzr
	fmov    d8,  xzr
	fmov    v8.d[1], xzr
	fmov    d9,  xzr
	fmov    v9.d[1], xzr
	fmov    d10, xzr
	fmov    v10.d[1], xzr
	fmov    d11, xzr
	fmov    v11.d[1], xzr
	fmov    d12, xzr
	fmov    v12.d[1], xzr
	fmov    d13, xzr
	fmov    v13.d[1], xzr
	fmov    d14, xzr
	fmov    v14.d[1], xzr
	fmov    d15, xzr
	fmov    v15.d[1], xzr
	fmov    d16, xzr
	fmov    v16.d[1], xzr
	fmov    d17, xzr
	fmov    v17.d[1], xzr
	fmov    d18, xzr
	fmov    v18.d[1], xzr
	fmov    d19, xzr
	fmov    v19.d[1], xzr
	fmov    d20, xzr
	fmov    v20.d[1], xzr
	fmov    d21, xzr
	fmov    v21.d[1], xzr
	fmov    d22, xzr
	fmov    v22.d[1], xzr
	fmov    d23, xzr
	fmov    v23.d[1], xzr
	fmov    d24, xzr
	fmov    v24.d[1], xzr
	fmov    d25, xzr
	fmov    v25.d[1], xzr
	fmov    d26, xzr
	fmov    v26.d[1], xzr
	fmov    d27, xzr
	fmov    v27.d[1], xzr
	fmov    d28, xzr
	fmov    v28.d[1], xzr
	fmov    d29, xzr
	fmov    v29.d[1], xzr
	fmov    d30, xzr
	fmov    v30.d[1], xzr
	fmov    d31, xzr
	fmov    v31.d[1], xzr

	mov	sp, x0

	msr	elr_el3, x0

	b	tr_rst  /* 0 - reset */

#ifdef CONFIG_BRCM_IKOS
/* Init configuration */
/* Shared data structure between Arm code and BCU bench */
CONFIG_INIT:
    .word  0
CONFIG_MEMC:
    .word  0
CONFIG_UNASSIGN2:
    .word  0
CONFIG_UNASSIGN3:
    .word  0
CONFIG_UNASSIGN4:
    .word  0
CONFIG_UNASSIGN5:
    .word  0
CONFIG_UNASSIGN6:
    .word  0
CONFIG_UNASSIGN7:
    .word  0
#endif

    .align 3

#include "segtable.h"

    .globl segment_table
segment_table:
	_LONG_  text_end            /* [  0] End of text (R_SEG_ETEXT) */
	_LONG_  data_start          /* [  1] Beginning of data (R_SEG_FDATA) */
	_LONG_  data_end            /* [  2] End of data (R_SEG_EDATA) */
	_LONG_  _end                /* [  3] End of BSS (R_SEG_END) */
	_LONG_  text_start          /* [  4] Beginning of text (R_SEG_FTEXT) */
	_LONG_  bss_start           /* [  5] Beginning of BSS (R_SEG_FBSS) */
	_LONG_  0               /* [  6] Global Pointer (R_SEG_GP) */
	_LONG_  0               /* [  7] Beginning of reloc entries */
	_LONG_  0               /* [  8] End of reloc entries */
	_LONG_  0               /* [  9] R_SEG_APIENTRY */
	_LONG_  rodata_end          /* [  10] End of ro data (R_SEG_ERODATA) */

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
	b	1b		/* stop the cpu */

cont0:

	CALLINIT(=armv8_cpuinit)    /* enable i cache, disable d cache and mmu */

	/* common early init code between btrm/cferom/ram such as uart and etc*/
	CALLINIT(=board_earlyinit_common)

	/* Setup vector table for bootrom XIP operation */
	ldr	x0, =vectors_base
	msr	VBAR_EL3, x0

	SETLEDS('-','-','-','-')
	SETLEDS('B','T','R','M')

#if defined(_BCM94908_)

	SETLEDS('V','1','.','6')
	SETLEDS('C','P','U','0')
	CALLINIT(=initialize_all_srams)

#elif defined(_BCM96858_)

	SETLEDS('V','1','.','1')
#if ! defined(_BTRM_EMULATION_)
	CALLINIT(=initialize_all_srams)
#else
	SETLEDS('4','E','M','U')
#endif

#endif

	/* Cycle Count */
	CALLINIT(=armv8_enable_tick)

#if defined(IKOS_BD_CFERAM)
	/* invalid I cache to make the backdoor-loaded code is visible*/
	CALLINIT(=armv8_l1cache_inval_i)
	CALLINIT(=armv8_l1cache_disable_i)

	ldr	x0,=0x1000000
	br	x0
#endif

	SETLEDS('L','1','C','D')
	/* invalid tlb, done in mmuinit. invalid d cache */
	CALLINIT(=armv8_l1cache_inval_d)

	/* enable MMU and then enable data cache */
	SETLEDS('M','M','U','I')

	CALLINIT(=armv8_mmuinit)
	CALLINIT(=armv8_enable_mmu)
	CALLINIT(=armv8_l1cache_enable_d)

	/* Copy data to RAM */
	SETLEDS('D','A','T','A')

copy2ram:
	LOADREL(x4,=segment_table)  /* This loads the segment table link address               
	                               it then adds to it the offset between linkaddress       
	                               and XIP code address on the storage media, giving us    
	                               the address of segment table on the XIP storage media */

	ldr	x1, [x4,#R_SEG_FDATA]   /* x1: target start address for DATA relocation only!*/
	ldr	x0, [x4,#R_SEG_ERODATA] /* x0: source start address for DATA relocation only!*/
	ldr	x2, [x4,#R_SEG_EDATA]   /* x2: target end address */
copy_loop:
	ldp	w3, w4, [x0], #8    /* copy from source address [x0] */
	stp	w3, w4, [x1], #8    /* copy to  target  address [x1] */
	cmp	x1, x2              /* until target end address [x2] */
	blo	copy_loop

clear_bss:
	SETLEDS('Z','B','B','S')
	LOADREL(x4,=segment_table)
	/* Clear BSS */
	ldr	x0, [x4,#R_SEG_FBSS]    /* x0: bss start address */
	ldr	x1, [x4,#R_SEG_END]     /* x0: bss end address */
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

	/* SRAM allocated for bootrom usage is 128KB */
	mov	x0,#128         /* memory size in Kbytes */
	ldr	x1,=mem_totalsize
	str	x0,[x1]

	/* not relocating, no offset */
	ldr	x0, =0
	ldr	x1, =mem_datareloc
	str	x0, [x1]
	ldr	x1, =mem_textreloc
	str	x0, [x1]

	/* Store memory related info */
	LOADREL(x4,=segment_table)
	ldr	x0, [x4,#R_SEG_FDATA] /* .data section is the first section in SRAM */
	ldr	x1, =mem_bottomofmem
	str	x0, [x1]

	ldr	x2, [x4,#R_SEG_ETEXT]
	sub	x2, x2, x0
	ldr	x1, =mem_textsize
	str	x2, [x1]
	ldr	x1, =mem_textbase
	str	x0, [x1]

	ldr	x0, [x4,#R_SEG_END] /* r0: bss end address */
	ldr	x1, =mem_heapstart
	str	x0, [x1]
	add	x0, x0, #(CFG_ROM_HEAP_SIZE*1024)
	add	x0, x0, #STACK_SIZE
	ldr	x1, =mem_topofmem
	str	x0, [x1]

	ldr	x1,=mem_topofmem
	ldr	x0,[x1]
	sub	x0,x0,#8
	mov	x1, #15
	bic	x0, x0, x1
	mov	sp, x0 /* 16-byte alignment for ABI compliance */

	/* run in linked adress and SP is setup, no more call to CALLINIT, SETLED after this point */

	/* setup parameters for c_main */
	/* CFE: clear argument a & b in cfe_main */
	ldr	w0,=0x00000001
	ldr	w1,=0x00000002
	
	bl	cfe_main

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
    *  CFE_LAUNCH
    *
    *  Start the user program.  The program is passed a handle
    *  that must be passed back when calling the firmware.
    *
    *  Parameters passed to the called program are as follows:
    *
    *      x0 - CFE handle (if 0, authentication failed and we will spin forever)
    *
    *  Input parameters:
    *      x0 - entry vector
    *
    *  Return value:
    *      does not return
    ********************************************************************* */
FUNC(cfe_launch)

	ldr	x1,=0xdeaddead
	cmp	x0,x1
	bne	setup_shred_args
	nop
	nop

	/* trap/exception took place ... clean up everything */
	/* SOTP block should be shutdown */
	ldr	x1,=SOTP_BASE
	mov	w2,#0
	str	w2,[x1,#SOTP_PERM]

	/* PKA block should be shutdown */
	ldr	x1,=PKA_BASE
	mov	w2,#0
	str	w2,[x1,#PKA_PERM]

	/* Clear all shredder arguments to force catastrophic auth failiure */
	mov	x24, xzr	/* set SBI vector to 0, in x24 */
	mov	w25, wzr	/* set sbi length to 0, in w25 */
	mov	w27, wzr	/* set jtag unlck to 0, in w27 */
	b	mask_all_ints

setup_shred_args:
	/* x24 - SBI launch vector*/
	mov	x24, x0 		/* save SBI vector in x24 */

	/* x25 - SBI image length */
	ldr	x0, =BTRM_INT_MEM_SBI_LINK_ADDR_VIRT
	sub	x0, x0, #4 		/* 32-bit word before SBI image is SBI length */
	ldr	w25, [x0] 		/* save sbi length in w25 */

	/* w27 - JTAG unlock flag */
	sub	x0, x0, #4 		/* 32-bit word 4 bytes before SBI image length is jtag unlock flag */
	ldr	w27, [x0] 		/* save jtag unlock flag in w27 */

	/* Check if we have a valid SBI length, if not, then force authentication failiure */
	cmp	wzr, w25
	bne	mask_all_ints
	/* Force authentication failure by setting SBI vector to zero */
	mov	x24, xzr

mask_all_ints:
	/* Mask all interrupts. */
	mrs	x1, DAIF
	orr	x1, x1,#(DAIF_I | DAIF_F)
	msr	DAIF, x1

relocate_shredder_transit:
	/* Copy Shredder transit code */
	ldr	x1, =BTRM_INT_MEM_SHREDDER_TRANSIT_PROG_ADDR /* x1: target address for shredder transict code relocation */
	ldr	x0, =__shredTransitBegin                     /* x0: Shredder transit code start */
	ldr	x2, =__shredTransitEnd                       /* x2: Shredder transit code end */
shredder_transit_copy_loop:
	ldp	w3, w4, [x0], #8    /* copy from source address [x0] */
	stp	w3, w4, [x1], #8    /* copy to destinat address [x1] */
	cmp	x0, x2              /* until target end address [x2] */
	blo	shredder_transit_copy_loop

relocate_vtable:
	/* Align vector table target address to 2K boundary */
	mov	x0, #0x7FF
	add	x1, x1, #0x800
	bic	x1, x1, x0
	mov	x28, x1				/* save vector table base address in x28 */
	/* Copy Vector table and exception handler to shredder space */
	ldr	x0, =vectors_base               /* x0: vector table start */
	ldr	x2, =btrm_excp_handler_end      /* x2: exception handler code end */
shredder_transit_vtable_copy_loop:
	ldp	w3, w4, [x0], #8    /* copy from source address [x0] */
	stp	w3, w4, [x1], #8    /* copy to destinat address [x1] */
	cmp	x0, x2              /* until target end address [x2] */
	blo	shredder_transit_vtable_copy_loop

__shredBegin:
	/***********************************************************************/
	/* Argument registers for  shredder code:                              */
	/* x24 = SBI entry vector, x25 = SBI length, x27 = jtag unlock/!lock   */
	/* x28 = vector table base address in shredder memory space            */
	/***********************************************************************/	
	
shred_btrm_data_bss_stack_heap:
	/* Clean Bootrom's data/stack/heap/.bss space + cferom hdrs + flags */
	ldr	x1, =BTRM_INT_MEM_BEGIN_ADDR
	ldr	x2, =BTRM_INT_MEM_SBI_LINK_ADDR_VIRT
	bl	shred_memory

auth_fail_chk_1:
	/* Check if authentication had failed */
	cmp	xzr, x24
	beq	auth_failed_shred

auth_passed_shred:
	/* shred memory from end of SBI image to end of BTRM SRAM memory space */
	ldr	x0, =BTRM_INT_MEM_SBI_LINK_ADDR_VIRT
	add	x1, x0, x25		/* Add saved SBI length from x25 */
	add	x1, x1,#0xf
	and	x1, x1,#0xfffffff0 /* make x5 land on a 16 byte boundary */
	ldr	x2, =BTRM_INT_MEM_END_ADDR
	bl	shred_memory
	b	disable_caches_mmu

auth_failed_shred:
	/* Clean SBI image */
	ldr	x1, =BTRM_INT_MEM_SBI_LINK_ADDR_VIRT
	ldr	x2, =BTRM_INT_MEM_END_ADDR
	bl	shred_memory

	/* Clean credentials */
	ldr	x1, =BTRM_INT_MEM_CREDENTIALS_ADDR
	ldr	x2, =BTRM_INT_MEM_CREDENTIALS_ADDR_END
	bl	shred_memory

disable_caches_mmu:
	/* Flush the D-Cache */
	mov	w0, #0
	bl	_cfe_flushcache

	/* Disable mmu */
	mrs	x1, SCTLR_EL3
	bic	x1, x1, SCTLR_ELx_M
	msr	SCTLR_EL3, x1
	isb
	ldr	x0, =MEMC_BASE
	mov	w1, #0      /* disable the map */
	str	w1, [x0, MEMC_SRAM_REMAP_CONTROL] ;

	/* Disable the D-Cache */
	mrs	x1, SCTLR_EL3
	bic	x1, x1, #SCTLR_ELx_C
	msr	SCTLR_EL3, x1
	isb
	
	/* Invalidate I-cache */
	ic	ialluis			
	isb	
	
	/* Disable I-cache - since all looping code is done */
	mrs	x0, SCTLR_EL3
	bic	x0, x0, #SCTLR_ELx_I
	msr	SCTLR_EL3, x0

	/* Jump to shredder transit code, never to return */
	ldr	x0,=BTRM_INT_MEM_SHREDDER_TRANSIT_PROG_ADDR
	br	x0
	
	/*************************************************/
	/*  shred_memory routine:                        */
	/*      x1,x2 - start/end of range for cleaning  */
	/*************************************************/
shred_memory:
	str	xzr, [x1], #8
	cmp	x1, x2
	blo	shred_memory
	ret

__shredEnd:

__shredTransitBegin:
	/***********************************************************************/
	/* Argument registers for  shredder transit code:                      */
	/* x24 = SBI entry vector, x25 = SBI length, x27 = jtag unlock/!lock   */
	/* x28 = vector table base address in shredder memory space            */
	/* Read arguments, check whether auth failed or not, open jtag  or not */
	/* All shredder transit code needs to be PIC, therefore no literal     */
	/* loads. All shredder transit code operates on physical memory space  */
	/* Calling code must have flushed all the caches                       */
	/***********************************************************************/
	
cfg_shred_vector_table:
	/* Setup shredder vector_table ( bootrom will be disabled ) */
	msr	VBAR_EL3, x28

disable_btrm:
	/* Disable bootrom access */
	mov	x1, #(BROM_SEC1_BASE & 0x0000ffff)
	movk	x1, #(BROM_SEC1_BASE >> 16 ), LSL #16
	mov	w2, #(BROM_SEC1_ACCESS_CNTRL_DISABLE_BTRM)
	str	w2, [x1,#BROM_SEC1_ACCESS_CNTRL]


        /* output ---- to uart */
	mov     w0, #0x2d
	mov	x1, #(UART_BASE & 0x0000ffff)
	movk	x1, #(UART_BASE >> 16 ), LSL #16
	mov	w2, #4
last_uart:
        ldrh    w3, [x1, #UART0INTSTAT]
        tst     w3, #TXFIFOEMT
        beq     last_uart

        strb    w0, [x1, #UART0DATA]
	sub	w2, w2, #1
        cmp     w2, #0
        bne     last_uart

        mov     w2, #'\r'
        strb    w2, [x1, #UART0DATA]
        mov     w2, #'\n'
        strb    w2, [x1, #UART0DATA]


jtag_unlock_check:
	/* open Jtag if unlock flag is set */
	cmp	wzr, w27
	beq	auth_fail_chk_2

open_jtag:
	/* Open JTAG + misc. debug interfaces */
	mov	x0, #(BROM_GEN_BASE & 0x0000ffff)
	movk	x0, #(BROM_GEN_BASE >> 16 ), LSL #16
	mov	x1, #(BROM_GEN_SECBOOTCFG_INTF_UNLOCK)
	str	x1, [x0, #BROM_GEN_SECBOOTCFG]


auth_fail_chk_2:
	/* Check if authentication had failed */
	cmp	xzr, x24
	beq	auth_fail_loop

launch_sbi:
	/* If auth passed, jump to SBI entry vector */
	blr	x24 /* Launch CFE ROM (SBI) code ... never to return ... */
	nop
	nop

auth_fail_loop:
	/* If auth failed, loop forever*/
	nop
	nop
	b	auth_fail_loop         /*  loop forever ....  */
	nop
	nop
	
__shredTransitEnd:

END(cfe_launch)

/*  *********************************************************************
    *  Bootrom Vector Table
    ********************************************************************* */

    /* Vector table base address needs to have zeroes in bits [10:0] */
	.align 11
vectors_base:

	/* current EL with SP0 */
sp0_vector_sync:
	b	btrm_excp_handler

	.align 7
sp0_vector_irq:
	b	btrm_excp_handler

	.align 7
sp0_vector_fiq:
	b	btrm_excp_handler

	.align 7
sp0_vector_serr:
	b	btrm_excp_handler

	/* current EL with SPx */
	.align 7
spx_vector_sync:
	b	btrm_excp_handler

	.align 7
spx_vector_irq:
	b	btrm_excp_handler

	.align 7
spx_vector_fiq:
	b	btrm_excp_handler

	.align 7
spx_vector_serr:
	b	btrm_excp_handler

	/* from lower EL using Aarch64 */
	.align 7
l64_vector_sync:
	b	btrm_excp_handler

	.align 7
l64_vector_irq:
	b	btrm_excp_handler

	.align 7
l64_vector_fiq:
	b	btrm_excp_handler

	.align 7
l64_vector_serr:
	b	btrm_excp_handler

	/* from lower EL using Aarch32 */
	.align 7
l32_vector_sync:
	b	btrm_excp_handler

	.align 7
l32_vector_irq:
	b	btrm_excp_handler

	.align 7
l32_vector_fiq:
	b	btrm_excp_handler

	.align 7
l32_vector_serr:
	b	btrm_excp_handler

/*  *********************************************************************
	*  btrm_excp_handler
	*
	*  -Bootrom exception handler
	*  -Calls cfe_launch with x0=0xdeaddead if exception occured while 
	*   XIP from bootrom space
	*  -Calls shredder exception handler is exception in shredder space
	*
	*  Input parameters:
	*    None
	*  Return value:
	*    Nothing
	********************************************************************* */	
btrm_excp_handler:
	movz	x0, #(BTRM_INT_MEM_SHREDDER_TRANSIT_PROG_ADDR >> 16 ), LSL #16
	mrs	x1, ELR_EL3
	and	x1, x1, #0xFFFF0000
	cmp	x0, x1
	beq	shredder_trans_excp_handler	/* If exception in shredder transit space */
	ldr	x0,=0xdeaddead
	bl	cfe_launch			/* If exception in XIP btrm space */

shredder_trans_excp_handler:
	/* Disable bootrom access */
	mov	x1, #(BROM_SEC1_BASE & 0x0000ffff)
	movk	x1, #(BROM_SEC1_BASE >> 16 ), LSL #16
	mov	w2, #(BROM_SEC1_ACCESS_CNTRL_DISABLE_BTRM)
	str	w2, [x1,#BROM_SEC1_ACCESS_CNTRL]

	/* Disable JTAG + misc. debug interfaces */
	mov	x0, #(BROM_GEN_BASE & 0x0000ffff)
	movk	x0, #(BROM_GEN_BASE >> 16 ), LSL #16
	mov	x1, xzr
	str	x1, [x0, #BROM_GEN_SECBOOTCFG]

shred_trans_exc_nop_loop:
	nop
	b	shred_trans_exc_nop_loop

btrm_excp_handler_end:


/*  *********************************************************************
    *  initialize_all_srams(x)
    *
    *  Does read-write memory test on all the SRAMS available
    *
    *  Input parameters:
    *      nothing
    *  Return value:
    *      nothing
    ********************************************************************* */
FUNC(initialize_all_srams)
	/* Save link register */
	mov	x27, x30

	ldr	x0, =BTRM_INT_SRAM_RDP_32K_ADDR
	ldr	x1, =BTRM_INT_SRAM_RDP_32K_SIZE
	CALLINIT(=zero_out_mem)

	ldr	x0, =BTRM_INT_SRAM_RDP_48K_0_ADDR
	ldr	x1, =BTRM_INT_SRAM_RDP_48K_SIZE
	CALLINIT(=zero_out_mem)

	ldr	x0, =BTRM_INT_SRAM_RDP_48K_1_ADDR
	ldr	x1, =BTRM_INT_SRAM_RDP_48K_SIZE
	CALLINIT(=zero_out_mem)

	ldr	x0, =BTRM_INT_SRAM_RDP_128K_ADDR
	ldr	x1, =BTRM_INT_SRAM_RDP_128K_SIZE
	CALLINIT(=zero_out_mem)

	/* Restore link register */
	mov	x30, x27
	ret
END(initialize_all_srams)

/*  *********************************************************************
    *  zero_out_mem(x)
    *
    *  Zeroes out specified address range
    *
    *  Input parameters:
    *      x0 - Address of memory to test 
    *      x1 - Size of memory
    *  Return value:
    *      nothing
    ********************************************************************* */
FUNC(zero_out_mem)

	/* x1 now contains end address */
	add     x1, x0, x1
zero_loop:	
	stp	wzr, wzr, [x0], #8    /* copy from source address [x0] */
	cmp     x0, x1
	blo     zero_loop
	ret

END(zero_out_mem)

.data

.align 3

#include "initdata.h"

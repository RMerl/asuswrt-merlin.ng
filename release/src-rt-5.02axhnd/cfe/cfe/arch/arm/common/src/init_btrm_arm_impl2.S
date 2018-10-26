/*
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
 */

#include "bcm_map.h"

#define c_main	cfe_main
#include <arm.h>
#include <armmacros.h>
#include <bcm_btrm_common.h>
#include <mmu_map.h>


/* Instruction macros that hide machine/arch details. */
.macro _LSL_ rd,rm,im
        lsl \rd,\rm,\im
.endm
.macro _LSR_ rd,rm,im
        lsr \rd,\rm,\im
.endm
.macro _ROR_ rd,rm,im
        ror \rd,\rm,\im
.endm
.macro _MUL_ rd,rm,rs
        mov \rd,\rm
        mul \rd,\rd,\rs
.endm
/* Entry point */
	.section .text.startup
	.globl startup
startup:
	b	tr_rst		/* 0 - reset */
	ldr	pc,_tr_und	/* 4 - undefined instruction */
	ldr	pc,_tr_swi	/* 8 - software interrupt */
	ldr	pc,_tr_iab	/* 0xc - prefetch abort */
	ldr	pc,_tr_dab	/* 0x10 - data abort */
	ldr	pc,_tr_bad	/* 0x14 - reserved */
	ldr	pc,_tr_irq	/* 0x18 - external interrupt */
	ldr	pc,_tr_fiq	/* 0x1c - fast interrupt */

/* Shared data structure between Arm code and BCU bench */
	.word 0x0
	.word 0x0
	.word 0x0
	.word 0x0
	.word 0x0
	.word 0x0
	.word 0x0
	.word 0x0

_tr_und:
	.word tr_und
_tr_swi:
	.word tr_swi
_tr_iab:
	.word tr_iab
_tr_dab:
	.word tr_dab
_tr_bad:
	.word tr_bad
_tr_irq:
	.word tr_irq
_tr_fiq:
	.word tr_fiq

#if CFG_EMBEDDED_PIC
#define RUNTIME_RELOC_START	__runtime_reloc_start
#define RUNTIME_RELOC_STOP	__runtime_reloc_stop
#else
#define RUNTIME_RELOC_START	0
#define RUNTIME_RELOC_STOP	0
#endif

#include "segtable.h"
	.globl segment_table
segment_table:
	.word	text_end			/* [  0] End of text (R_SEG_ETEXT) */
	.word	data_start			/* [  1] Beginning of data (R_SEG_FDATA) */
	.word	data_end			/* [  2] End of data (R_SEG_EDATA) */
	.word	_end				/* [  3] End of BSS (R_SEG_END) */
	.word	text_start			/* [  4] Beginning of text (R_SEG_FTEXT) */
	.word	bss_start			/* [  5] Beginning of BSS (R_SEG_FBSS) */
	.word	0				/* [  6] Global Pointer (R_SEG_GP) */
	.word	RUNTIME_RELOC_START		/* [  7] Beginning of reloc entries */
	.word	RUNTIME_RELOC_STOP		/* [  8] End of reloc entries */
	.word	0				/* [  9] R_SEG_APIENTRY */
	.word   rodata_end			/* [ 10] R_SEG_ERODATA */
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
#if (BTRM_TEST_TESTROM==1) 
/* Fake cfe rom build entry - needed as stub replacement for CFE ROM */
        ldr     r0, =0x444f4e45   /* DONE */
        ldr     r1, =UART_BASE
	mov	r7,#0
1:
        ldrh    r2, [r1, #UART0INTSTAT]
        tst     r2, #TXFIFOEMT
        beq     1b

        and     r2, r0, #0xff000000
        lsr     r2, r2, #24
        strb    r2, [r1, #UART0DATA]
        lsl     r0, r0, #8
        cmp     r0, #0
        bne     1b

        mov     r2, #'\r'
        strb    r2, [r1, #UART0DATA]
        mov     r2, #'\n'
        strb    r2, [r1, #UART0DATA]

	/* test status if 2 - 2 tests have been taken;exiting*/
	cmp	r7,#2
	beq	spin_forever

	bl	_krsa_mfg_pub_addr
	mov     r3,r11
	mov	r0,#'M'
	cmp	r7,#0
	beq	_test
	/*setting to fld key for the next test*/
	bl     _krot_fld_pub_addr
	mov     r3,r11
	mov	r0,#'F'
_test:	
	add	r7,r7,#1 /* test credentials started*/
	/*shifting first character*/	
	lsl	r0,r0,#8
	orr	r0,r0,#'K'
	lsl	r0,r0,#8
	orr	r0,r0,#'Y'
	lsl	r0,r0,#8
	orr	r0,r0,#'P'

	/* loop counter aka "while" */
	/*size of array in r8*/
	ldr	r8,[r3],#4 
	ldr	r4,=BTRM_INT_MEM_CREDENTIALS_ADDR
2:
	cmp	r8,#0
	beq	1b
	
	ldr	r5,[r3],#4
	ldr	r6,[r4],#4
	sub	r8,r8,#4
	cmp	r6,r5
	beq	2b

	bfc	r0,#0,#8
	orr	r0,r0,#'F'
	b	1b

spin_forever:
        b       spin_forever
        nop

#endif
	mrs	r0,cpsr
	mov	r1,#(PS_MM)
	bic	r0,r0,r1
	/* Switch to system mode and make sure interrupts are disabled */
	mov 	r1,#(PS_I | PS_F | PS_SYS)
	orr	r0,r0,r1
	msr	cpsr,r0
initcpu:

	mov	r0,#0
	mcr	p15,0,r0,c8,c7,0		/* Invalidate TLB */
	mcr	p15,0,r0,c7,c5,0		/* Invalidate icache */
	/* Initialize system control register */
	mrc	p15,0,r0,c1,c0,0
	bic	r0,r0,#0x7			/* Clear C, A, M bits */
	orr	r0,r0,#0x2000			/* Set V bit: high vectors (0xffff0000) */
	orr	r0,r0,#0x2			/* Set A bit: enable strict alignment */
	orr	r0,r0,#0x800			/* Set Z bit: enable program flow prediction */
	/* part of I+D $ init: one line below */
	orr	r0,r0,#0x1000			/* Set I bit: enable instruction cache */
	mcr	p15,0,r0,c1,c0,0
	/* part of I+D $ init: one line below */
	isb 

	/* Enable SMP before cache enabled, for Cortex-A7 series core 
		as per MPCore spec: "System Control ARM DDI 0464F :
		pg 6-3; 4-61 e.g for A-7 to enable L1 D-cache this this bit must be 1
		ACTRL.SMP == 1   
	*/
	mrc	p15,0,r1,c1,c0,1
	orr	r1,r1,#0x40
	mcr	p15,0,r1,c1,c0,1

	/* Identify current CPU */
	mrc	p15,0,r0,c0,c0,5
	and	r0,r0,#0xf			/* retrieve cpu core id */
	cmp	r0,#0				/* cpu core 0? */
        nop
	beq	check_startaddr

	/* parking core 1 */
        wfe

check_startaddr:

	CALLINIT(=board_earlyinit_common)

	SETLEDS2('-','-','-','-')

	SETLEDS2('B','T','R','M')

	SETLEDS2('V','1','.','1')

	CALLINIT(=armv7_enable_tick)
 
	/*board specific initialization*/
	CALLINIT(=board_earlyinit)

	SETLEDS2('C','A','C','H')
	/* enabled i cache */
	/*below are stack free function with no nesting calls*/
	bl	armv7_l1cache_inval_i 
	bl	armv7_l1cache_enable_i
        /* mmu page/section tables are defined 
	in the appropriate board implementation */
	bl	armv7_mmu_map

	/* enable d cache  - Cortex-A7 does D-Cache invalidation on Reset 
		- 63138 does require it
	*/
	bl	armv7_l1cache_inval_d

	bl	armv7_enable_mmu

	bl	armv7_l1cache_enable_d
	
	/* Memory is ready to use */
	/* Start moving code into internal LMEM */
        /* Copy to RAM */
		
copy2ram:
	/*using SETLEDS2 to avoid ruinning over sp*/
    	SETLEDS2('C','O','D','E')
	ldr r0, =segment_table
	/* get the end of RODATA which is == .data segment in abs. address */
	ldr r1,	[r0, #R_SEG_ERODATA]
	ldr r2, [r0, #R_SEG_END]
	ldr r3, [r0, #R_SEG_FDATA]
1:
	ldmia	r1!, {r4}		/* copy from source address [r0] */
	stmia	r3!, {r4}		/* copy to   target address [r1] */
	cmp	r3, r2
	bne	1b
	/* Clear BSS */

    	SETLEDS2('Z','B','S','S')
	ldr 	r0, =segment_table
	ldr	r3, [r0, #R_SEG_FBSS]
	ldr	r4, =bss_start
	ldr	r5, =bss_end
	/* a relocated bss_end address in r4 */
	sub	r4, r5, r4
	add	r4, r3, r4
	mov	r2, #0
1:
	stmia	r3!, {r2}
	cmp	r3, r4
	bne	1b

/* Set stackpointer in internal RAM to call c_main */
call_c_main:

    	SETLEDS2('M','A','I','N')
	
	/* 	
		r4 points to relocated bss_end 
	*/	
	ldr	r8,=BTRM_INT_MEM_UTIL_SIZE /* r8 contains 0x30000 = 192K (size of lmem) */

	/* Save memory size in bytes */
	mov	r0,r8
	ldr	r1,=_memsize		/* Load the addr of _memsize into r1 */
	str 	r0,[r1] 		/* store the content of r0 into the addr held by r1 */

	lsr 	r0,r0,#10		/* memory size in Kbytes */
	ldr	r1,=mem_totalsize
	str	r0,[r1]

        /* invalid I cache and flush D cache to make the relocated code is visible*/
        /*CALLINIT(=armv7_l1cache_inval_i) - has been called in cache_on; no code has been relocated*/

	/* Allocate auxilary bootrom segment table in place of TEMP stack
	*/
	/* max stack size is set to up to 16K*/
	mov	r0,r4			/* r0 should contain 0x8070_4000 */
	add 	r0,r0,#BTRM_STACK_RSRVD_SIZE
	sub	r0,r0,#8
	mov	sp,r0
	bic	sp,sp,#7		/* 8-byte alignment for ABI compliance */
	/* CFE: clear argument a & b in cfe_main */
	ldr	r0,=0x00000000
	ldr	r1,=0x00000000
	bl	c_main

/* In case c_main returns */
theend:
	wfe
	b	theend

/*  *********************************************************************
    *  traps
    *
    *  Interrupt handlers
    * Note: hndrte_arm.c for EXT_CBALL expects each trap handler to be 16 bytes 
    *
    *  Input parameters:
    *
    *  Return value:
    *  	   nothing
    ********************************************************************* */

LEAF(__traps)

/* Undefined instruction exception handler */
tr_und:

	ldr	r0,=0xdeaddead
	bl	cfe_launch

/* Software interrupt exception handler */
tr_swi:

	ldr	r0,=0xdeaddead
	bl	cfe_launch

/* Prefetch abort exception handler */
tr_iab:

	ldr	r0,=0xdeaddead
	bl	cfe_launch

/* Data abort exception handler */
tr_dab:

	ldr	r0,=0xdeaddead
	bl	cfe_launch

tr_bad: /* reserved */

	ldr	r0,=0xdeaddead
	bl	cfe_launch

/* Interrupt handler */
tr_irq:

	ldr	r0,=0xdeaddead
	bl	cfe_launch

/* Fast interrupt handler */
tr_fiq:

	ldr	r0,=0xdeaddead
	bl	cfe_launch

END(__traps)


/*  *********************************************************************
    *  raise
    *
    *  Dummy function just for link happy 
    *
    *  Input parameters:
    *
    *  Return value:
    *  	   nothing
    ********************************************************************* */

FUNC(raise)

	mov	r0, #0
	mov pc, lr

END(raise)


/*  *********************************************************************
    *  CFE_FLUSHCACHE
    *
    *  Perform certain cache operations
    *
    *  Input parameters:
    *  	   r0 - flags (CFE_CACHE_xxx flags, or zero for a default)
    *      r1,r2 - start/end of range for "range invalidate" operations
    *      (not used otherwise)
    *
    *  Return value:
    *  	   nothing
    ********************************************************************* */

FUNC(_cfe_flushcache)

        stmfd   sp!, {r0-r7, r9-r11, lr}
        bl      armv7_cacheops
        ldmfd   sp!, {r0-r7, r9-r11, lr}

	mov pc, lr

END(_cfe_flushcache)


/*  *********************************************************************
    *  CFE_LAUNCH - this call does not use stack
    *
    *  Start the user program.  The program is passed a handle
    *  that must be passed back when calling the firmware.
    *
    *  Parameters passed to the called program are as follows:
    *
    *      r0 - CFE handle (if 0, authentication failed and we will spin forever)
    *
    *  Input parameters:
    *  	   r0 - entry vector
    *
    *  Return value:
    *  	   does not return
    ********************************************************************* */

FUNC(cfe_launch)
	ldr	r1,=0xdeaddead
	cmp	r0,r1
	bne	1f
	nop
	nop

	/* trap/exception took place ... clean up everything */
	/* SOTP block should be shutdown */
        ldr     r1,=SOTP_BASE
        mov     r2,#0
	str	r2,[r1,#SOTP_PERM]

	/* For the excecption in particularly -  no gaurantee sp is pointing to valid memory. Furthermore, no gaurantee 
	   bootrom was still XIPing from bootrom address versus running from LMEM.
	   If XIPing from bootrom, the following bootrom demapping will freeze the
	   execution and JTAG will remain locked. If running from LMEM, the sp is 
	   valid and execution will continue
	   bootrom should be permanently demapped 
	   now proceed like if it was a normal authentication failure but avoiding stack
	*/
	mov	r0,#0
1:
	/* SBI entry vector in r8 temporarily*/
	mov	r8,r0
	/* Mask all interrupts. */
	mrs	r0,cpsr
	ldr	r1,=(PS_I | PS_F)
	orr	r0,r0,r1
	msr	cpsr,r0

	adrl    r0,__shredBegin                 		/* r0: source start address:      */
	ldr     r1,=BTRM_INT_MEM_SHREDDER_PROG_ADDR             /* r1: target address      :      */
        adrl    r2,__shredEnd

copyloop:
	ldmia	r0!,{r9-r10}		/* copy from source address [r0] */
	stmia	r1!,{r9-r10}		/* copy to   target address [r1] */
	cmp	r0,r2			/* until source end address [r2] */
	blo	copyloop


	/* invalidate icache */
	/* r8 is preserved for cacheops; stack isn't used 
	   used registers:  r0-r7,r9,r10,r12,lr 
        */
	//bl	armv7_l1cache_flush_d
	/* flush both L1 icache dcache*/
	mov     r0, #0
        bl      armv7_cacheops

	/* jtag unlock default setting - unlocked*/
	mov	r11,#0
	/* sbi vector or 0 if failure*/
	mov	r12,r8

        b       __shredEnd
        nop
        nop

__shredBegin:  /* beginning of actual shredder code that will be copied over */
        /* Load up registers r5 and r6 with the beginning of internal memory, and where
           the shredder program will be located, respectively, and have them point to the
           cached memory locations. We are about to clean up any text, data, bss pertaining 
           to the BTRM, and also any heap that was used. */
        mov     r5, #(BTRM_INT_MEM_BEGIN_ADDR & 0xff000000)
	orr	r5, r5, #(BTRM_INT_MEM_BEGIN_ADDR & 0x00ff0000)
	orr	r5, r5, #(BTRM_INT_MEM_BEGIN_ADDR & 0x0000ff00)
	orr	r5, r5, #(BTRM_INT_MEM_BEGIN_ADDR & 0x000000ff)

	add	r6 ,r5, #(BTRM_DATA_BSS_RSRVD_SIZE+BTRM_STACK_RSRVD_SIZE)
        /* Clean BSS,DATA and C-Stack */
	
        mov     r0,#0
1:      /* clean this swath of cached memory */
        str     r0,[r5]
        add     r5,r5,#4
        cmp     r5,r6
        blt     1b
        nop
        nop

	/* verify this for impl3 Gen3 implementation*/
        mov     r8,#(BTRM_INT_MEM_SBI_LINK_ADDR & 0xff000000) 
        orr     r8,#(BTRM_INT_MEM_SBI_LINK_ADDR & 0x00ff0000) 
        orr     r8,#(BTRM_INT_MEM_SBI_LINK_ADDR & 0x0000ff00) 
        orr     r8,#(BTRM_INT_MEM_SBI_LINK_ADDR & 0x000000ff)

        /* Load up registers r5 with where security credentials exist if authentication has failed, or
           load it up with the end addr of the SBI (CFE ROM) if authentication has passed. Also, 
           load up r6 with the end of utilized memory. Have both of these pointers point to the cached.
           memory. This will clean up the C stack space */
        /*ldr     r0,[sp] */
	cmp	r12,#0
	/* 0 jump to authentication failed*/
        beq     _cred_clean_loop
        nop
        nop

	/*Prepare JTAG Unlock status*/
        ldr     r11,[r8,#-8]					 	/* r11 contains JTAG open/close status*/
	eor	r11,r11,#1
        ldr	r1,[r8,#-4]						/* r1 contains the SBI length */
	add	r8,r8,r1						/* r8 points to the end of the SBI image */

        add     r8,r8,#0xf
        and	r8,r8,#0xfffffff0					/* make r5 land on a 16 byte boundary */

        b       _sbi_clean_loop
        nop
        nop
_cred_clean_loop:
        mov     r5,#(BTRM_INT_MEM_CREDENTIALS_ADDR & 0xff000000) /* auth has failed ... clean everything including SBI */
	orr	r5,r5,#(BTRM_INT_MEM_CREDENTIALS_ADDR & 0x00ff0000)
	orr	r5,r5,#(BTRM_INT_MEM_CREDENTIALS_ADDR & 0x0000ff00)
	orr	r5,r5,#(BTRM_INT_MEM_CREDENTIALS_ADDR & 0x000000ff)

	mov	r6,#(BTRM_INT_MEM_CREDENTIALS_ADDR_END & 0xff000000)
	orr	r6,r6,#(BTRM_INT_MEM_CREDENTIALS_ADDR_END & 0x00ff0000)
	orr	r6,r6,#(BTRM_INT_MEM_CREDENTIALS_ADDR_END & 0x0000ff00)
	orr	r6,r6,#(BTRM_INT_MEM_CREDENTIALS_ADDR_END & 0x000000ff)

	mov 	r1,#0
1:      /* clean this swath of cached memory */
        str     r1,[r5]
	add	r5,r5,#4
        cmp     r5,r6
        blt    	1b

_sbi_clean_loop:
/* cleaning SBI begin/end up to the end of memory*/
        mov     r6,#(BTRM_INT_MEM_END_ADDR & 0xff000000)
        orr     r6,r6,#(BTRM_INT_MEM_END_ADDR & 0x00ff0000)
        orr     r6,r6,#(BTRM_INT_MEM_END_ADDR & 0x0000ff00)
        orr     r6,r6,#(BTRM_INT_MEM_END_ADDR & 0x000000ff)

        mov     r1,#0

1:      /* if auth failed r8 is at BTRM_INT_MEM_SBI_LINK_ADDR+
	sbi_length if passed otherwise BTRM_INT_MEM_SBI_LINK_ADDR*/
        str     r1,[r8,#0]
        str     r1,[r8,#4]
        str     r1,[r8,#8]
        str     r1,[r8,#12]
        add     r8,r8,#16
        cmp     r8,r6
        blt     1b
        nop
        nop

        /* If authentication passed, need to reload the CFE ROM entry point back into
           where the sp is pointing (because we just zeroed out everything to the end of memory) */

	/* invalidate icache */
        mov   r0, #0
        mcr   p15, 0, r0, c7, c5, 0 
        isb


	/* flush data cache */
        mrc     p15, 1, r0, c0, c0, 1           @ read clidr
        ands    r3, r0, #0x7000000              @ extract loc from clidr
        mov     r3, r3, lsr #23                 @ left align loc bit field
        beq     lnch_fin                        @ if loc is 0, then no need to clean
        mov     r10, #0                         @ start clean at cache level 0
lnch_loop1:
        add     r2, r10, r10, lsr #1            @ work out 3x current cache level
        mov     r1, r0, lsr r2                  @ extract cache type bits from clidr
        and     r1, r1, #7                      @ mask of the bits for current cache only
        cmp     r1, #2                          @ see what cache we have at this level
        blt     lnch_skip                       @ skip if no cache, or just i-cache
        mcr     p15, 2, r10, c0, c0, 0          @ select current cache level in cssr
        isb
                                                @ but we compile with armv5
        mrc     p15, 1, r1, c0, c0, 0           @ read the new csidr
        and     r2, r1, #7                      @ extract the length of the cache lines
        add     r2, r2, #4                      @ add 4 (line length offset)
        mov     r4, #0xff			@ two lines that replace the ldr statement
        orr     r4, r4, #0x300
        ands    r4, r4, r1, lsr #3              @ find maximum number on the way size
        clz     r5, r4                          @ find bit position of way size increment
        mov     r7, #0xff			@ two lines that replace the ldr statement
        orr     r7, r7, #0x7f00
        ands    r7, r7, r1, lsr #13             @ extract max number of the index size
lnch_loop2:
        mov     r9, r4                          @ create working copy of max way size
lnch_loop3:
        orr     r6, r10, r9, lsl r5             @ factor way and cache number into r6
        orr     r6, r6, r7, lsl r2              @ factor index number into r6
        mcr     p15, 0, r6, c7, c14, 2          @ clean & invalidate by set/way
        subs    r9, r9, #1                      @ decrement the way
        bge     lnch_loop3
        subs    r7, r7, #1                      @ decrement the index
        bge     lnch_loop2
lnch_skip:
        add     r10, r10, #2                    @ increment cache number
        cmp     r3, r10
        bgt     lnch_loop1
lnch_fin:
        mov     r10, #0                         @ swith back to cache level 0
        mcr     p15, 2, r10, c0, c0, 0          @ select current cache level in cssr
        isb


	/* Disable the D-Cache */
        mrc     p15, 0, r0, c1, c0, 0 // Read Control Register configuration data
        bic     r0, r0, #CR_C         // Disable D Cache
        mcr     p15, 0, r0, c1, c0, 0 // Write Control Register configuration data
        isb


	/* Disable MMU */
        mrc     p15, 0, r0, c1, c0, 0 // Read Control Register configuration data
        bic     r0, r0, #CR_M 
        mcr     p15, 0, r0, c1, c0, 0 // Write Control Register configuration data
        isb


	/* Disable I-Cache */
        mrc     p15, 0, r0, c1, c0, 0 // Read Control Register configuration data
        bic     r0, r0, #CR_I         // Disable I Cache
        bic     r0, r0, #CR_Z         // Disable Prediction
        mcr     p15, 0, r0, c1, c0, 0 // Write Control Register configuration data
        isb

disable_btrm:
        /* Disable bootrom access */
        mov     r1, #(BROM_SEC1_BASE & 0xff000000)
        orr     r1, #(BROM_SEC1_BASE & 0x00ff0000)
        orr     r1, #(BROM_SEC1_BASE & 0x0000ff00)
        orr     r1, #(BROM_SEC1_BASE & 0x000000ff)
        mov     r2, #(BROM_SEC1_ACCESS_CNTRL_DISABLE_BTRM)
        str     r2, [r1, #BROM_SEC1_ACCESS_CNTRL]

 	/* print out last message ---- */
	ldr	r0,_shred_msg_
        mov     r1, #(UART_BASE & 0xff000000)
        orr     r1, #(UART_BASE & 0x00ff0000)
        orr     r1, #(UART_BASE & 0x0000ff00)
        orr     r1, #(UART_BASE & 0x000000ff)
1:
        ldrh    r2, [r1, #UART0INTSTAT]
        tst     r2, #TXFIFOEMT
        beq     1b

        and     r2, r0, #0xff000000
        lsr     r2, r2, #24
        strb    r2, [r1, #UART0DATA]
        lsl     r0, r0, #8
        cmp     r0, #0
        bne     1b

        mov     r2, #'\r'
        strb    r2, [r1, #UART0DATA]
        mov     r2, #'\n'
        strb    r2, [r1, #UART0DATA]

        /* Set the external interface enable bit high so that if interfaces such as JTAG are 
           suppose to be accessible, they become enabled. This allows a secure boot enabled board 
           to have its JTAG enabled during the manufacuring process, or during development */
	cmp 	r11, #0
	bne	__check_failed	
        mov     r1,#(BROM_GEN_BASE & 0xff000000) 
	orr	r1,r1,#(BROM_GEN_BASE & 0x00ff0000)
	orr	r1,r1,#(BROM_GEN_BASE & 0x0000ff00)
	orr	r1,r1,#(BROM_GEN_BASE & 0x000000ff)
	mov	r2,#BROM_GEN_SECBOOTCFG_JTAG_UNLOCK
        str	r2,[r1,#BROM_GEN_SECBOOTCFG]

__check_failed:
        cmp     r12, #0
        beq     __authFailed

__justLaunch:
	mov     pc,r12		/* Launch CFE ROM (SBI) code ... never to return ... */
        nop
        nop

__authFailed:
1:
        nop
        nop
        b       1b         /*  loop forever ....  */
        nop
        nop
_shred_data:
	DW(_shred_msg_, 0x2d2d2d2d)
__shredEnd:     /* end of actual shredder code that will be copied over */
        /* Run the LMEM shredder program at the cached physical address */
        ldr     r4,=BTRM_INT_MEM_SHREDDER_PROG_ADDR     /* cached addr in internal mem */           
	bx	r4					/* go for it */
END(cfe_launch)

/*
 * Common code/data start here...
 */
#ifdef CONFIG_XIP
	.data
#endif
	.data
	.align 2	
#include "initdata.h"	
	DW(_memsize, 0)

/* C trap handler */
	DW(trap_hook, 0)

#ifdef	CONFIG_XIP
	.text
#endif


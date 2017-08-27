/*
   <:copyright-BRCM:2013:DUAL/GPL:standard
   
      Copyright (c) 2013 Broadcom 
      All Rights Reserved
   
   Unless you and Broadcom execute a separate written software license
   agreement governing use of this software, this software is licensed
   to you under the terms of the GNU General Public License version 2
   (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
   with the following added to such license:
   
      As a special exception, the copyright holders of this software give
      you permission to link this software with independent modules, and
      to copy and distribute the resulting executable under terms of your
      choice, provided that you also meet, for each linked independent
      module, the terms and conditions of the license of that module.
      An independent module is a module which is not derived from this
      software.  The special exception does not apply to any modifications
      of the software.
   
   Not withstanding the above, under no circumstances may you combine
   this software in any way with any other Broadcom software provided
   under a license other than the GPL, without Broadcom's express prior
   written consent.
   
   :> 
*/

#include "arm.h"
#include "exception.h"

#include "cfe.h"	
#include "bsp_config.h"
#include "cpu_config.h"

#ifdef _CFE_
#include "cfe_devfuncs.h"
#else

#define cfe_command_restart 0
#endif

/* BCM63XX specific change. */
#include "bcm_hwdefs.h"

#include "armmacros.h"
#include "bcm_common.h"
#if 1 /* FIXME move to rdb */
#define ARM_CFGINIT_MEMC    (1<<1)
#endif

#if (INC_BTRM_BOOT==1)
#include "btrm_support.h"
#endif

/* use r11 as the offset flash address to linking address. r11 is only used by exception
   handler for c, so it is fine to use this it in PIC code before going to main func.
   make sure no other place corrupt this register.*/
#define BOOTOFFSET	 r11

/* use r12 as the rom option from abort_key function and pass it to romoption variable after
  cfe rom is loaded to memory. r12 is only used by exception handler for c, so it is fine to
  use this it in PIC code before going to main func. make sure no other place corrupt this
  register.*/
#define ROMOPTION	 r12
	
/*
 * This is the size of the stack, rounded to KByte boundaries.
 */

#ifndef CFG_STACK_SIZE
#error "CFG_STACK_SIZE not defined"
#else
#define STACK_SIZE	((CFG_STACK_SIZE+1023) & ~1023)
#endif

	
/* Entry point */
	.section .text.startup

	.globl startup
startup:
	b	tr_rst	/* 0 - reset */
	ldr	pc,_tr_und	/* 4 - undefined instruction */
	ldr	pc,_tr_swi	/* 8 - software interrupt */
	ldr	pc,_tr_iab	/* 0xc - prefetch abort */
	ldr	pc,_tr_dab	/* 0x10 - data abort */
	ldr	pc,_tr_bad	/* 0x14 - reserved */
	ldr	pc,_tr_irq	/* 0x18 - external interrupt */
	ldr	pc,_tr_fiq	/* 0x1c - fast interrupt */

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
_pad:
	.word 0x12345678 /* now 16*4=64 */

#if CFG_EMBEDDED_PIC
#define RUNTIME_RELOC_START	__runtime_reloc_start
#define RUNTIME_RELOC_STOP	__runtime_reloc_stop
#else
#define RUNTIME_RELOC_START	0
#define RUNTIME_RELOC_STOP	0
#endif

#include "initdata.h"
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
#if !defined(CFG_RAMAPP)
	.word	0				/* [  9] R_SEG_APIENTRY */
#else
	.word	cpu_apientry			/* [  9] R_SEG_APIENTRY */
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
	mrs	r0,cpsr
	mov	r1,#(PS_MM)
	bic	r0,r0,r1
	/* Switch to system mode and make sure interrupts are disabled */
	mov 	r1,#(PS_I | PS_F | PS_SVC)
	orr	r0,r0,r1
	msr	cpsr,r0

	/* R_INIT_CPUINIT: Disable MMU, Enable I-cache... */
	CALLINIT(=armv7_cpuinit)

	adrl	r0,startup		/* r0: source start address */
	ldr	r1,=startup		/* r1: target address */
	sub	BOOTOFFSET, r0, r1      /* r11 is bootoffst */

#if !defined(CFG_RAMAPP)
	/* common early init code between btrm/cferom/ram such as uart and etc*/
	CALLINIT(=board_earlyinit_common)

	/* chip specific early init for cfe rom */	
        CALLINIT(=board_earlyinit)
#endif

//#if CFG_CMT && !defined(JTAG_DOWNLOAD) && (INC_BTRM_BOOT==0)	
	/*  R_INIT_TP1_SWITCH, witch CPU to start, start cpu0 for now */
//#endif	
#if !defined(_BCM947189_)
	/* Identify current CPU */
	mrc	p15,0,r0,c0,c0,5
	and	r0,r0,#0xf			/* retrieve cpu core id */
	cmp	r0,#0				/* cpu core 0? */
	bne     core1_wait
#endif
	
	SETLEDS('H','E','L','O')

#if defined(_BCM947189_) && !defined(CFG_RAMAPP)
	/*
	 * ddr_init uses registers r0-r12 as scratch registers (not trestored)
	 */
	CALLINIT(=ddr_init)

	/*
	 * Save the boot offset in BOOTOFFSET (r11) again, because ddr_init
	 * destroyed the previous BOOTOFSET value.
	 */
	adrl	r0,startup		/* r0: source start address */
	ldr	r1,=startup		/* r1: target address */
	sub	BOOTOFFSET, r0, r1	/* r11 is bootoffst */
#endif
	SETLEDS('C','P','U','0')

#if defined(_BCM963148_) && defined(DISABLE_CPU1) && !defined(CFG_RAMAPP)
	ldr	r0, =CPU_RESET_CONFIG_REG
	ldr	r1, [r0]
	bic     r1, #0x2
	str	r1, [r0]
#endif
	
	/*  R_INIT_BOARDINIT??? */
#if !defined(CFG_RAMAPP)
	cmp	BOOTOFFSET, #0
	beq	skip_pmc

#if defined(_BCM963138_)
	ldr	r1,=MISC_BASE
	ldr	r0,[r1,#MISC_STRAP_BUS]
	tst	r0,#MISC_STRAP_BUS_PMCROM_BOOT
	beq	init_pmc	// skip margin update if pmc held in reset

	/* adjust margins to 75,50 for 63138B0 */
	SETLEDS('P','M','C','M')

	mov	r0,#34		/* DQM message word0 cmdId[7:0]=cmdCloseAVS=34 */
	mov	r1,#0
	mov	r2,#75
	mov	r3,#50
	CALLINIT(=pmc_send_cmd)
	/* ignore errors */
init_pmc:
#endif
#if !defined(_BCM947189_)
	CALLINIT(=pmc_ll_init)
	cmp     r0, #0
die_pmc:
	bne     die_pmc
#endif
skip_pmc:
#endif
	/* Cycle Count */
	CALLINIT(=armv7_enable_tick)

#if !defined(CFG_RAMAPP) && defined(IKOS_BD_CFERAM)
	/* invalid I cache to make the backdoor-loaded code is visible*/
	CALLINIT(=armv7_l1cache_inval_i)
	CALLINIT(=armv7_l1cache_disable_i)

	ldr	r0,=0xf00000
	mov     lr, r0
	mov	pc,lr
#endif



	cmp	BOOTOFFSET, #0
	beq	enable_mmu

	/* Copy code and data  to RAM */
	SETLEDS('C','O','D','E')
copy2ram:
	LOADREL(r4,=segment_table)
	ldr	r1,[r4,#R_SEG_FTEXT]   /* r1: target address */
	add	r0,r1, BOOTOFFSET	/* r0: source start address */
	ldr	r2,[r4,#R_SEG_EDATA]    
	add	r2,BOOTOFFSET 		/* r2: source end address */
copy_loop:
	ldmia	r0!,{r9-r10}		/* copy from source address [r0] */
	stmia	r1!,{r9-r10}		/* copy to   target address [r1] */
	cmp	r0,r2			/* until source end address [r2] */
	blo	copy_loop

enable_mmu:

	SETLEDS('L','1','C','D')
	/* invalid tlb, done in cpuinit. invalid d cache */
	CALLINIT(=armv7_l1cache_inval_d)
	/* enable MMU and then enable data cache */
	SETLEDS('M','M','U','I')

	//MMU page table right below the text code in the size of 16KB 
	LOADREL(r4,=segment_table)
	ldr	r0, [r4,#R_SEG_FTEXT]   /* r1: target address */
	sub	r0, r0, #16*1024
	CALLINIT(=armv7_mmuinit)

	CALLINIT(=armv7_enable_mmu)
	CALLINIT(=armv7_l1cache_enable_d)


	
clear_bss:
        SETLEDS('Z','B','B','S')
	LOADREL(r4,=segment_table)
	/* Clear BSS */
	ldr	r0,[r4,#R_SEG_FBSS]	/* r0: bss start address */
	ldr	r1,[r4,#R_SEG_END]	/* r0: bss end address */
	mov	r2,#0x00000000
	/* empty bss section checking */
	cmp	r0,r1
	beq	call_c_main

clbss_l:
	str	r2,[r0]
	add	r0,r0,#4
	cmp	r0,r1
	bne	clbss_l
	
/* Set stackpointer in internal RAM to call c_main */
call_c_main:

	SETLEDS('M','A','I','N')
#if !defined(CFG_RAMAPP)
	/* cfe rom build for internal mem now 512KB */
	mov	r0,#512			/* memory size in Kbytes */
#else
#if defined(IKOS_NO_DDRINIT)
	mov	r2, #26
	mov	r0, #1
	lsl	r0, r0, r2		/* hardcode 64 MB */
#else
#if !defined(_BCM947189_)
	ldr     r1, =MEMC_BASE 
	ldr     r2, [r1, #MEMC_GLB_GCFG]
	and     r2, #MEMC_GLB_GCFG_DRAM_SIZE1_MASK
	add     r2, #10                  /* memory size in Kbytes */
	mov     r0, #1
	lsl     r0, r0, r2
#else
	/* mem_size returns the memory sizes in bytes in r0 */
	CALLINIT(=mem_size)
	lsr	r0,r0,#10		/* memory size in Kbytes */
#endif
#endif
#endif
	ldr	r1,=mem_totalsize
	str	r0,[r1]

#if !defined(CFG_RAMAPP) && defined(CFE_ABORT_KEY)
	CALLINIT(=chek_abort_key)
  	mov	ROMOPTION, r0
	ldr	r1,=rom_option
	str	ROMOPTION,[r1]
#endif	

	/* not relocating, no offset */
	ldr	r0,=0
	ldr	r1,=mem_datareloc
	str	r0,[r1]
	ldr	r1,=mem_textreloc
	str	r0,[r1]
	
	LOADREL(r4,=segment_table)
	ldr	r0,[r4,#R_SEG_FTEXT]
	ldr	r1,=mem_bottomofmem
	str	r0,[r1]

	ldr	r2,[r4,#R_SEG_ETEXT]
	sub	r2,r2,r0
	ldr	r1,=mem_textsize
	str	r2,[r1]
	ldr	r1,=mem_textbase
	str	r0,[r1]
	
	ldr	r0,[r4,#R_SEG_END]	/* r0: bss end address */
	ldr	r1,=mem_stackbottom
	str     r0,[r1]
	add	r0,#STACK_SIZE
	ldr	r1,=mem_heapstart
	str	r0,[r1]
	ldr	r1,=mem_stacktop
	str     r0,[r1]
#if !defined(CFG_RAMAPP)
	add	r0,#(CFG_ROM_HEAP_SIZE*1024)  /* onchip memory is only 512KB, rom does not need big heap */
#else
	add	r0,#(CFG_HEAP_SIZE*1024)
#endif
	ldr	r1,=mem_topofmem
	str	r0,[r1]

	ldr	r0,=0x00000000
	ldr	r1,=cpu_prid
	str	r0,[r1]

	b	main  

theend:
	wfe
	b	theend

	.org    0x56c
        .globl  _cferom_size
_cferom_size:	
	.word   _romsize
	
	.org    0x570
        .byte   'c','f','e','-','v',CFE_VER_MAJOR,CFE_VER_MINOR,CFE_VER_BUILD,BCM63XX_MAJOR,BCM63XX_MINOR  // CFE version info for applications
	.org	0x580			/*  move past exception vectors */
	/*
	 * BCM963XX NVRAM Data Storage. No internal bootrom is in play
	 */

	.globl nvram_data_storage
nvram_data_storage:
	.word	NVRAM_DATA_ID
        .space  0x400

main:
#if !defined(CFG_RAMAPP)
	/* invalid I cache and flush D cache to make the relocated code is visible*/
	CALLINIT(=armv7_l1cache_flush_d)
	CALLINIT(=armv7_l1cache_inval_i)
	ldr	r1,=gorelo
 	bx      r1           /* Now jump to an address code was compiled for */

gorelo:	nop
	mov     BOOTOFFSET, #0      /* no longer running at offset */
    	LOADREL(r0,=rel_version)
    	CALLINIT(=board_puts)
#endif

	/* init exception support */
	CALLINIT(=armv7_exception_init)
	
	ldr	r1,=mem_stacktop
	ldr	r0,[r1]
	sub	r0,r0,#8
	mov	sp,r0
	bic	sp,sp,#7		/* 8-byte alignment for ABI compliance */

	/* run in linked adress and SP is setup, no more call to CALLINIT, SETLED after this point */

	/* setup parameters for c_main */
	/* CFE: clear argument a & b in cfe_main */
	ldr	r0,=0x00000000
	ldr	r1,=0x00000000

	bl      cfe_main
        /* c_main should never returns */        

core1_wait:	
        /* CPU1 waiting for kernel smp for now*/
	SETLEDS('C','P','U','1')
#if defined(_BCM963148_) && defined(DISABLE_CPU1) && !defined(CFG_RAMAPP)
	SETLEDS('L','O','O','P')
core1_loop:
	b core1_loop
#endif	
	SETLEDS('W','A','I','T')
        wfe
	
		
/*  *********************************************************************
    *  CFE_FLUSHCACHE
    *
    *  Perform certain cache operations
    *
    *  Input parameters:
    *  	   r0 - flags (CFE_CACHE_xxx flags, or zero for a default flush d
    *      and invalidate i cache 
    *      r1,r2 - start/end of range for "range invalidate" operations
    *      (not used otherwise)
    *
    *  Return value:
    *  	   nothing
    ********************************************************************* */

FUNC(_cfe_flushcache)

	stmfd	sp!, {r0-r7, r9-r11, lr}
	bl	armv7_cacheops
	ldmfd	sp!, {r0-r7, r9-r11, lr}
	mov pc, lr
	
END(_cfe_flushcache)


/*  *********************************************************************
    *  CFE_LAUNCH
    *
    *  Start the user program.  The program is passed a handle
    *  that must be passed back when calling the firmware.
    *
    *  Parameters passed to the called program are as follows:
    *
    *      a0 - CFE handle
    *      a1 - entry vector
    *      a2 - reserved, will be 0
    *      a3 - entrypoint signature.
    *
    *  Input parameters:
    *  	   a0 - entry vector
    *
    *  Return value:
    *  	   does not return
    ********************************************************************* */

FUNC(cfe_launch)

	sub  sp,sp,#8
	push {r0}

	/* Mask all interrupts */
	mrs	r0,cpsr
	ldr	r1,=(PS_I | PS_F)
	orr	r0,r0,r1
	msr	cpsr,r0


#if ((INC_BTRM_BOOT==1) && !defined(CFG_RAMAPP)) /* If cfe rom secure boot, we need to shred stack, heap, etc. */
	bl	otp_is_boot_secure
	mov	r1,#1
	cmp	r0,r1
	bne	noshrd

        adrl    r0,__shredBegin                     /* r0: source start address  */
        ldr     r1,=BTRM_INT_MEM_SHREDDER_PROG_ADDR /* r1: target address        */
        adrl    r2,__shredEnd
cpylp:  ldmia   r0!,{r9-r10}            /* copy from source address [r0] */
        stmia   r1!,{r9-r10}            /* copy to   target address [r1] */
        cmp     r0,r2                   /* until source end address [r2] */
        blo     cpylp
#endif /* INC_BTRM_BOOT==1 */


	/* Flush the D-Cache, Invalidate I-cache */
noshrd:	mov	r0, #0
	bl	_cfe_flushcache


#if ((INC_BTRM_BOOT==1) && !defined(CFG_RAMAPP))
	bl	otp_is_boot_secure
	mov	r1,#1
	cmp	r0,r1
	bne	__skipShred
	b	__shredEnd

__shredBegin:  /* beginning of actual shredder code that will be copied over */

	shredder()
 	unlockJtag()

        /* At this point, we are ready to launch the CFE RAM if authentication passed,
           or loop forever if authentication failed */
        ldr     r0,[sp]
        mov     r1,#0
        cmp     r0,r1
        beq     __authFailed
	mov     pc,r0           /* Launch CFE RAM code ... never to return ... */
__authFailed:
	b	__authFailed

__shredEnd:     /* end of actual shredder code that will be copied over */

        /* Run the shredder program */
        ldr     r4,=BTRM_INT_MEM_SHREDDER_PROG_ADDR
        bx      r4                               
#endif /* INC_BTRM_BOOT==1 */


__skipShred:
	/* Disable the D-Cache, MMU and I-Cache bit */
	bl	armv7_l1cache_disable_d
	bl	armv7_disable_mmu
	bl	armv7_l1cache_disable_i

	/* Launch the prog */
	b RunProgram

END(cfe_launch)

/*
 * This is a nice place to set a breakpoint.
 */
FUNC(RunProgram)
#if !defined(CFG_RAMAPP)
	pop {r4}		/* entry point */
	mov pc,r4		/* go for it. */
#else
	LOADREL(r2,=segment_table)
	ldr r2,[r2,#R_SEG_APIENTRY]	/* A2 = code entry */

#if CFG_EMBEDDED_PIC
	ldr r1,mem_textreloc	/* relocate table entry */
	add r2,r2,r1
#endif

	mov r0,#0		/* A0 = handle */
	ldr r1, =BCM63XX_MACH_ID /* A1 = MACH_ID */
	ldr r2, =ARM_ATAG_LOC     /* A2 = atag ptr */
	ldr r3,=CFE_EPTSEAL	/* A3 = entrypoint signature */
	pop {r4}		/* entry point */
	mov pc,r4		/* go for it. */
#endif
END(RunProgram)

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
#	mov	ip, lr		/* persevere link reg across call */
	mov     r0, sp
#	mov	lr, ip		/* restore link */
	mov 	pc, lr		/* back to my caller */
END(get_SP)

/*  *********************************************************************
    *  _GETSEGTBL()
    *
    *  Return the address of the segment table.  We use this
    *  to display the startup messages.
    *
    *  You can't just address the table from C because it lives
    *  in the text segment.
    *
    *  Input parameters:
    *  	   nothing
    *
    *  Return value:
    *      address of table
    ********************************************************************* */

FUNC(_getsegtbl)

	mov	ip, lr		/* persevere link reg across call */
	LOADREL(r0,=segment_table)
	mov	lr, ip		/* restore link */
	mov pc, lr		/* back to my caller */

END(_getsegtbl)

/*  *********************************************************************
    *  _SETCONTEXT()
    *
    *  Set the CONTEXT register.
    *
    *  Input parameters:
    *  	   a0 - context
    *
    *  Return value:
    *  	   nothing
    ********************************************************************* */

FUNC(_setcontext)

	mov	pc,lr

END(_setcontext)

#if defined(CFG_RAMAPP)
FUNC(cpu_apientry)
	//FIXME implement the API entry
        mov	pc,lr
END(cpu_apientry)
#endif

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

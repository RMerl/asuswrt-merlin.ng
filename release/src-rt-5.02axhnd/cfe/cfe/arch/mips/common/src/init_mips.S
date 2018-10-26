/*  *********************************************************************
    *  Broadcom Common Firmware Environment (CFE)
    *  
    *  CPU init module				File: init_mips.S
    *
    *  This module contains the vectors and lowest-level CPU startup
    *  functions for CFE.
    *
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *  
    *********************************************************************  
    *
    *  Copyright 2000,2001,2002,2003
    *  Broadcom Corporation. All rights reserved.
    *  
    *  This software is furnished under license and may be used and 
    *  copied only in accordance with the following terms and 
    *  conditions.  Subject to these conditions, you may download, 
    *  copy, install, use, modify and distribute modified or unmodified 
    *  copies of this software in source and/or binary form.  No title 
    *  or ownership is transferred hereby.
    *  
    *  1) Any source code used, modified or distributed must reproduce 
    *     and retain this copyright notice and list of conditions 
    *     as they appear in the source file.
    *  
    *  2) No right is granted to use any trade name, trademark, or 
    *     logo of Broadcom Corporation.  The "Broadcom Corporation" 
    *     name may not be used to endorse or promote products derived 
    *     from this software without the prior written permission of 
    *     Broadcom Corporation.
    *  
    *  3) THIS SOFTWARE IS PROVIDED "AS-IS" AND ANY EXPRESS OR
    *     IMPLIED WARRANTIES, INCLUDING BUT NOT LIMITED TO, ANY IMPLIED
    *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
    *     PURPOSE, OR NON-INFRINGEMENT ARE DISCLAIMED. IN NO EVENT 
    *     SHALL BROADCOM BE LIABLE FOR ANY DAMAGES WHATSOEVER, AND IN 
    *     PARTICULAR, BROADCOM SHALL NOT BE LIABLE FOR DIRECT, INDIRECT,
    *     INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
    *     (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
    *     GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
    *     BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY 
    *     OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR 
    *     TORT (INCLUDING NEGLIGENCE OR OTHERWISE), EVEN IF ADVISED OF 
    *     THE POSSIBILITY OF SUCH DAMAGE.
    ********************************************************************* */


#include "sbmips.h"
#include "exception.h"

#include "bsp_config.h"
#include "cpu_config.h"

#ifdef _CFE_
#include "cfe_devfuncs.h"
#else

#define cfe_command_restart 0
#endif
#include "cfe_iocb.h"

/* BCM63XX specific change. */
#include "bcm_hwdefs.h"
#include "bcm63xx_rom.h"

/*  *********************************************************************
    *  Macros
    ********************************************************************* */

#include "mipsmacros.h"
#include "bcm_map.h"

#include "btrm_support.h"

#ifndef INC_BTRM_BOOT
#define INC_BTRM_BOOT         0
#endif

	
/*  *********************************************************************
    *  SETLEDS(a,b,c,d)
    *  SETLEDS1(a,b,c,d)
    *
    *  Sets the on-board LED display (if present).  Two variants
    *  of this routine are provided.  If you're running KSEG1,
    *  call the SETLEDS1 variant, else call SETLEDS.
    *
    *  Input parameters:
    *     a,b,c,d - four ASCII characters (literal constants)
    *
    *  Return value:
    *     a0,k1,ra trashed
    ********************************************************************* */

#define SETLEDS(a,b,c,d)					\
		li	a0,(((a)<<24)|((b)<<16)|((c)<<8)|(d)) ;\
		CALLINIT_KSEG0(init_table,R_INIT_SETLEDS)

#define SETLEDS1(a,b,c,d)					\
		li	a0,(((a)<<24)|((b)<<16)|((c)<<8)|(d)) ;\
		CALLINIT_KSEG1(init_table,R_INIT_SETLEDS)

#define PRINT_STR(s)			\
		b	2f ;		\
		nop ;			\
1:		.string s ;		\
		.align   2 ;		\
2:		LOADREL(a0,1b) ;	\
		CALLINIT_KSEG1(init_table,R_INIT_PUTS)

/*  *********************************************************************
    *  Other constants
    ********************************************************************* */

/*
 * This is the size of the stack, rounded to KByte boundaries.
 */

#ifndef CFG_STACK_SIZE
#error "CFG_STACK_SIZE not defined"
#else
#define STACK_SIZE	((CFG_STACK_SIZE+1023) & ~1023)
#endif

#ifdef __MIPSEB
#define TEXTSECTION	0x2e746578		# ".tex", big-endian
#else
#define TEXTSECTION	0x7865742e		# ".tex", little-endian
#endif


/*
 * To make life easier reading this code, define "KSEGBASE"
 * to either K0BASE or K1BASE depending on whether we're running
 * uncached.
 */

#if CFG_RUNFROMKSEG0
#define KSEGBASE	K0BASE
#else
#define KSEGBASE	K1BASE
#endif


/*  *********************************************************************
    *  Names of registers used in this module
    ********************************************************************* */

#define RELOCOFFSET	s8			/* $30 (fp) */
#define TEXTOFFSET	t9			/* $25 (t9) */
#define MEMTOP		t8			/* $24 (t8) */
#define TEXTBASE	s7			/* $23 (s7) */
#undef BOOT_OFFSET
#define BOOT_OFFSET	s6			/* $22 (s6) */
#define FLASHBASE	s5
#if (CFG_COPY_PSRAM==1)
#define CODE_SRC_START	s1
#define DATA_SRC_END	s2
#endif

		.sdata

#include "initdata.h"		/* declare variables we use here */

#if CFG_MULTI_CPUS
		.globl	cfe_spinlock
cfe_spinlock:	.word	0
#endif

		.extern	_fdata
		.extern	_edata
		.extern	_etext

/*  *********************************************************************
    *  uninitialized data
    ********************************************************************* */

		.bss

		.comm	__junk,4

/*  *********************************************************************
    *  Exception Vectors
    ********************************************************************* */

		.text

		.set noreorder

/*
 * Declare the actual vectors.  This expands to code that
 * must be at the very beginning of the text segment.
 */

DECLARE_VECTOR(0x0000,vec_reset,cpu_reset)

#ifdef CONFIG_BRCM_IKOS
/* config word for ikos bench code to store the test parameters */
starup_cfg:
		_LONG_	0
sdc_cfg0:
		_LONG_	0
sdc_cfg1:
		_LONG_	0
		_LONG_	0
		_LONG_	0
		_LONG_	0
		_LONG_	0
#endif

		.set reorder

/*  *********************************************************************
    *  Some offsets depend on our current configuration
    ********************************************************************* */

#define RUNTIME_RELOC_START	0
#define RUNTIME_RELOC_STOP	0

/*  *********************************************************************
    *  Segment Table.
    *
    *  Addresses of data segments and of certain routines we're going
    *  to call from KSEG1.  These are here mostly for the embedded
    *  PIC case, since we can't count on the 'la' instruction to
    *  do the expected thing (the assembler expands it into a macro
    *  for doing GP-relative stuff, and the code is NOT GP-relative.
    *  So, we (relocatably) get the offset of this table and then
    *  index within it.
    *
    *  Pointer values in this segment will be relative to KSEG0 for
    *  cached versions of CFE, so we need to OR in K1BASE in the
    *  case of calling to a uncached address.
    *
    *  The LOADREL macro handles most of the nastiness here.
    ********************************************************************* */

#include "segtable.h"
#include "cfe.h"

#if (INC_BTRM_BOOT==1)

		.org	0x56c
		.globl	_cferom_size
_cferom_size:
		.word	_romsize

		.org	0x570
		.byte	'c','f','e','-','v',CFE_VER_MAJOR,CFE_VER_MINOR,CFE_VER_BUILD,BCM63XX_MAJOR,BCM63XX_MINOR # CFE version info for applications
		.org	0x580			/* move past exception vectors */

		.globl	nvram_data_storage
nvram_data_storage:
		.word	NVRAM_DATA_ID
#if defined(_BCM963268_)
		.space	0xc00			/* NOTE: NVRAM is 3k in size in this case !!! ... not 1k !! */
#else
		.space	0x400			/* 1k */
#endif

#endif /* (INC_BTRM_BOOT==1) */


		.globl segment_table
segment_table:
		_LONG_	_etext			# [  0] End of text (R_SEG_ETEXT)
		_LONG_	_fdata			# [  1] Beginning of data (R_SEG_FDATA)
		_LONG_	_edata			# [  2] End of data (R_SEG_EDATA)
		_LONG_	_end			# [  3] End of BSS (R_SEG_END)
		_LONG_	_ftext			# [  4] Beginning of text (R_SEG_FTEXT)
		_LONG_	_fbss			# [  5] Beginning of BSS (R_SEG_FBSS)
		_LONG_	_gp			# [  6] Global Pointer (R_SEG_GP)
		_LONG_	0			# [  7] Beginning of reloc entries
		_LONG_	0			# [  8] End of reloc entries
		_LONG_	0			# [  9] R_SEG_APIENTRY

/*  *********************************************************************
    *  Init Table.
    *
    *  This is like segment_table except it contains pointers to
    *  routines used during initialization.  It serves both as a
    *  table for doing PIC stuff and also to separate out
    *  machine-specific init routines.
    *
    *  The CALLINIT_xxx macros are used to call routines in this table.
    ********************************************************************* */


		.globl	init_table
init_table:
		_LONG_	board_earlyinit		# [  0] R_INIT_EARLYINIT
		_LONG_	board_setleds		# [  1] R_INIT_SETLEDS
		_LONG_	board_draminfo		# [  2] R_INIT_DRAMINFO
		_LONG_	CPUCFG_CPUINIT		# [  3] R_INIT_CPUINIT
		_LONG_	CPUCFG_ALTCPU_START1	# [  4] R_INIT_ALTCPU_START1
		_LONG_	CPUCFG_ALTCPU_START2	# [  5] R_INIT_ALTCPU_START2
		_LONG_	CPUCFG_ALTCPU_RESET	# [  6] R_INIT_ALTCPU_RESET
		_LONG_	CPUCFG_CPURESTART	# [  7] R_INIT_CPURESTART
		_LONG_	CPUCFG_DRAMINIT		# [  8] R_INIT_DRAMINIT
		_LONG_	CPUCFG_CACHEOPS		# [  9] R_INIT_CACHEOPS
		_LONG_	CPUCFG_TLBHANDLER	# [ 10] R_INIT_TLBHANDLER
		_LONG_	cfe_main		# [ 11] R_INIT_CMDSTART
		_LONG_	cfe_command_restart	# [ 12] R_INIT_CMDRESTART
		_LONG_	cfe_doxreq		# [ 13] R_INIT_DOXREQ
		_LONG_	CPUCFG_TP1_SWITCH	# [ 14] R_INIT_TP1_SWITCH
		_LONG_	cfe_size_ram		# [ 15] R_INIT_SIZERAM
		_LONG_	board_init		# [ 16] R_INIT_BOARDINIT
		_LONG_	pll_init		# [ 17] R_INIT_PLL
		_LONG_	board_puts		# [ 18] R_INIT_PUTS
#if defined(JTAG_DELAY)
		_LONG_	gpio_loop		# [ 19] R_INIT_GPIOLOOP
#else
		_LONG_	bcmcore_null		# [ 19] R_INIT_GPIOLOOP
#endif
		_LONG_	bcmcore_null		# [ 20] R_INIT_NANDLDR
#if defined(_BCM963381_) || defined(_BCM96848_)
		_LONG_	pmc_ll_init		# [ 21] R_INIT_PMC
#else
		_LONG_	bcmcore_null		# [ 21] R_INIT_PMC
#endif
#if defined(CFE_ABORT_KEY)
		_LONG_	board_getc		# [ 22] R_INIT_GETC
#else
		_LONG_	bcmcore_null		# [ 22] R_INIT_GETC
#endif
#if defined(_BCM963268_) || defined(_BCM96838_) || defined(_BCM963381_) || defined(_BCM96848_)
		_LONG_	otp_is_btrm_enabled	# [ 23] R_INIT_BTRM_ENABLED
		_LONG_	otp_is_boot_secure	# [ 24] R_INIT_BOOT_SECURE
#else
		_LONG_	bcmcore_null		# [ 23] R_INIT_BTRM_ENABLED
		_LONG_	bcmcore_null		# [ 24] R_INIT_BOOT_SECURE
#endif
#if defined(_BCM96848_)
		_LONG_	bcmcore_null		# [ 25] R_INIT_WAIT_AVS
		_LONG_	otp_is_tp1_enabled	# [ 26] R_INIT_TP1_ENABLE
		_LONG_	post_ddr_board_init	# [ 27] R_INIT_POST_DDR
#else
		_LONG_	bcmcore_null		# [ 25] R_INIT_WAIT_AVS
		_LONG_	bcmcore_null		# [ 26] R_INIT_TP1_ENABLE
		_LONG_	bcmcore_null		# [ 27] R_INIT_POST_DDR
#endif

/*  *********************************************************************
	*  CPU Startup Code
	********************************************************************* */

cpu_reset:

#if defined(_BCM96838_) || defined(_BCM96848_)
		/* Check to see whether the internal bootrom previously ran */
#if defined(_BCM96848_)
		/* Bootrom ran also when straps configures to boot from spi-nand */
		li	t0,MISC_BASE
		lw	t1,MISC_STRAP_BUS(t0)
		and	t2,t1,MISC_STRAP_BUS_BOOT_SEL_MASK
		beq	t2,MISC_STRAP_BUS_BOOT_SPI_NAND,1f
		nop
#endif
		/* CALLINIT_KSEG1(init_table,R_INIT_BTRM_ENABLED) */
		bal	otp_is_btrm_enabled
		nop
		beq	v0,0,2f
		nop

1:
		/* Bootrom peviously ran. Cfe rom is already relocated into */
		/* internal memory. No XIP from flash is occurring. Cfe rom */
		/* is running at a virtual address, but linked to run at the*/
		/* equivalent unmapped, cached address. Jump to that address*/
		la	t1, __cont
		j	t1
2:		nop
__cont:	nop
#endif

		/*
		 * Start with GP as zero.  Nobody should touch
		 * this or set it to any other value until we're ready
		 * to use it.  This is used to tell when we should start
		 * using relocated references in the init table,
		 * so beware!  (see CALLINIT_RELOC in mipsmacros.h)
		 */

		move	gp,zero			# start with no GP.

		.set noreorder

		bal	1f
		nop
1:		nop
		.set reorder
		li	FLASHBASE, 0x1fff0000
		and	FLASHBASE, ra
		GET_BOOT_OFFSET(BOOT_OFFSET)

#if defined(JTAG_DELAY) || defined(JTAG_DOWNLOAD) || (CFG_BOOT_PSRAM==1)
		/* sp register can contain any garbage value upon power on.
		when debugging this piece of early code, jtag reads the stack and
		might hit invalid address. this cause bus error and jtag break in failure.
		so setup the sp to point to the rom just to make sure debugger software can read
		the "stack" content successfully. you might still have to close the call frame
		window since there is no valid the call frames in the rom */
		move   sp, FLASHBASE
		or     sp, K1BASE ;
		add    sp, 0x400
#endif


#------------------------------------------------------------------------------

		/*
		 * Do low-level board initialization.  This is our first
		 * chance to customize the startup sequence.
		 */
		move	a0, FLASHBASE

		CALLINIT_KSEG1(init_table,R_INIT_EARLYINIT)

#if (CFG_COPY_PSRAM==1)
		bal	copy_to_psram
		nop
#endif

#if (CFG_COPY_PSRAM==1) || (CFG_BOOT_PSRAM==1)
psram_cp_done:
		LOADREL(a0,segment_table)
		LR	gp,R_SEG_GP(a0)

		la	sp,PSRAM_BASE		# destination address
		add	sp,PSRAM_SIZE
#if defined(_BCM96838_)
		sub	sp,0x4000
#elif defined(_BCM96848_)
		sub	sp,0x5000
#endif

#endif


#if defined(_BCM963268_) && (INC_BTRM_BOOT==1)
		/* If the flash image is built for secure boot, but the */
		/* SoC is not configured for secure boot, configure it  */
		mid_otp_fuse()
#endif
		SETLEDS1('H','E','L','O')

		CALLINIT_KSEG1(init_table,R_INIT_CPUINIT)

#if (INC_PMC_DRIVER==1) && defined(_BCM96838_)
		/* initialize PLL only in TP0 */
		mfc0	t1, C0_BCM_CONFIG, 3
		li	t2, CP0_CMT_TPID
		and	t1, t2
		bnez	t1, 2f
		nop

		CALLINIT_KSEG1(init_table,R_INIT_PLL)
		/* pll_init returns straps, save it in a2 in order to send
			it later to dram_init */
		bnez v0, 1f
		nop
		li v0, 0x10
		SETLEDS1('S','0','0','0')
1:
		move a2, v0
2:
#endif

#if defined(JTAG_DELAY)
		SETLEDS1('G','P','I','O')
		CALLINIT_KSEG1(init_table, R_INIT_GPIOLOOP)
#endif

#------------------------------------------------------------------------------

		/*
		 * Now, switch from KSEG1 to KSEG0
		 */

#if CFG_RUNFROMKSEG0
		bal	cpu_kseg0_switch
		GET_BOOT_OFFSET(BOOT_OFFSET)
#endif

#------------------------------------------------------------------------------
		/*
		 * Now running on cpu0 in K0SEG.
		 */

		/* perform any additional board initialization */
		CALLINIT_KSEG0(init_table, R_INIT_BOARDINIT)

		/*
		 * Check if the thread switch is required. If secure boot is included
		 * in the build, TP1 was already started and parked by the btrm
		 */
#if CFG_CMT && !defined(JTAG_DOWNLOAD) && (CFG_BOOT_PSRAM==0)
		/*
		 * About to run TP1 start and potential thread switch. If we are already 
		 * running on TP1, this function will park the thread that will not 
		 * run Linux. If we are running on TP0, this function will take TP1
		 * out of reset and put TP0 in a wait loop waiting for a signal from TP1/
		 */
#if defined(_BCM96838_) || defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM96848_)
		/* If the bootrom ran before this, TP1 was already started and parked */
		/* and therefore skip TP1 start                                       */
		CALLINIT_KSEG0(init_table,R_INIT_BTRM_ENABLED)
		bne	v0,0,__skip_tp1
		nop
#if defined(_BCM963381_)
		/* Check to see if we are strapped for SPI NAND flash. If it is, TP1 was started by the bootrom */
		li	t0,MISC_BASE
		lw	t1,MISC_STRAP_BUS(t0)
		and	t1,MISC_STRAP_BUS_SPI_NAND_DISABLE
		beq	t1,0,__skip_tp1
		nop
#endif
#endif

#if defined(_BCM96848_)
		CALLINIT_KSEG0(init_table,R_INIT_TP1_ENABLE)
		beq	v0,0,__skip_tp1
		nop

		/* TP1 started in bootrom when board is strapped to spi-nand */
		li	t0,MISC_BASE
		lw	t1,MISC_STRAP_BUS(t0)
		and	t2,t1,MISC_STRAP_BUS_BOOT_SEL_MASK
		beq	t2,MISC_STRAP_BUS_BOOT_SPI_NAND,__skip_tp1
		nop

#endif

		CALLINIT_KSEG0(init_table,R_INIT_TP1_SWITCH)
__skip_tp1:	nop
#endif

		LOADREL(a0,rel_version)
		CALLINIT_KSEG0(init_table,R_INIT_PUTS)
		li  ROMOPTION,0
#if defined(CFE_ABORT_KEY)
		bal	chek_abort_key
		nop
#endif

#if defined(_BCM963381_) || defined(_BCM96848_)
		CALLINIT_KSEG0(init_table,R_INIT_PMC)
#endif

#if CFG_INIT_DRAM && !defined(JTAG_DOWNLOAD) && !defined(IKOS_NO_DDRINIT)
		SETLEDS('D','R','A','M')

		CALLINIT_KSEG0(init_table,R_INIT_DRAMINFO)

		move	a0,v0			# pass these params
		CALLINIT_KSEG0(init_table,R_INIT_DRAMINIT)
		CALLINIT_KSEG0(init_table,R_INIT_SIZERAM)
		move	k0,v0			# Save in k0 for now
#else
		/* Only 64MB initialized by JTAG */
		li	k0, 64
#endif

#if defined(_BCM96848_)
		CALLINIT_KSEG0(init_table, R_INIT_POST_DDR)
		SETLEDS('P','D','D','R')
#endif

#------------------------------------------------------------------------------
#if CFG_BOOTRAM
		b	have_ram			# No RAM is ok if using emulator RAM
#endif

		bne	k0,zero,have_ram

		SETLEDS('R','A','M','X')	# die here if no ram

die1:		b	die1

have_ram:

		 /*
		  * If this is the 64-bit version, turn on the KX bit
		  * to allow 64-bit accesses.
		  */

#ifdef __long64
		mfc0	t0,C0_SR
		or	t0,t0,M_SR_KX
		mtc0	t0,C0_SR
#endif

#------------------------------------------------------------------------------
		/*
		 * K0 contains the RAM size (and therefore the top of RAM
		 * offset).  Start there, and subtract the amount of memory
		 * we expect to use.  If we have more than 256MB of
		 * physical memory, work backwards from the 256MB
		 * boundary.
		 */

__CalcMemTop:   li	MEMTOP,256		# 256MB boundary
		bgt	k0,MEMTOP,1f		# use 256MB if k0 is greater
		move	MEMTOP,k0		# otherwise keep top
1:		sll	MEMTOP,20		# make into byte amount

		li	RELOCOFFSET,0		# not relocating, no offset
		li	TEXTOFFSET,0

		/*
		 * DRAM is now running, and we're alive in cacheable memory
		 * on cpu0 in K0SEG.  Set up GP.
		 */

#if (CFG_COPY_PSRAM==0) && (CFG_BOOT_PSRAM==0)
		LOADREL(a0,segment_table)
		LR	gp,R_SEG_GP(a0)
		add	gp,RELOCOFFSET
#endif

#------------------------------------------------------------------------------
		/*
		 * Zero BSS
		 */

		SETLEDS('Z','B','S','S')

		LOADREL(a0,segment_table)
__ZeroBss:

		LR	v0,R_SEG_FBSS(a0)
		LR	v1,R_SEG_END(a0)
		ADD	v0,RELOCOFFSET		# Relocate to actual data segment
		ADD	v1,RELOCOFFSET

1:		SR	zero,0(v0)		# Zero one cacheline at a time
		SR	zero,(REGSIZE*1)(v0)
		SR	zero,(REGSIZE*2)(v0)
		SR	zero,(REGSIZE*3)(v0)
		add	v0,REGSIZE*4
		blt	v0,v1,1b


#------------------------------------------------------------------------------
#if (CFG_COPY_PSRAM==0) && (CFG_BOOT_PSRAM==0)

		SETLEDS('C','O','D','E')

		LOADREL(a0,segment_table)
__CopyCode:

		LR	t1,R_SEG_FTEXT(a0)		# destination address
		move	TEXTBASE,t1

		LR	t2,R_SEG_FTEXT(a0)		# Source address
		FIXUP (t2);
		LR	t3,R_SEG_ETEXT(a0)
		FIXUP (t3);

1:		LR	t4,0(t2)	# read one cache line
		LR	t5,(REGSIZE*1)(t2)
		LR	t6,(REGSIZE*2)(t2)
		LR	t7,(REGSIZE*3)(t2)
		SR	t4,0(t1)	# write one cache line
		SR	t5,(REGSIZE*1)(t1)
		SR	t6,(REGSIZE*2)(t1)
		SR	t7,(REGSIZE*3)(t1)
		add	t1,REGSIZE*4
		add	t2,REGSIZE*4
		bltu	t2,t3,1b

#------------------------------------------------------------------------------
		/*
		 * Copy initialized data
		 */

#if (CFG_BOOTRAM == 0)

		SETLEDS('D','A','T','A')

		LOADREL(a0,segment_table)

__CopyData:
		LR	t1,R_SEG_FDATA(a0)
		FIXUP (t1);
		li	t0,15
		add	t1,t0
		not	t0
		and	t1,t0		# t1 = _etext rounded up to 16-byte boundary

		LR	t2,R_SEG_FDATA(a0)
		LR	t3,R_SEG_EDATA(a0)
		ADD	t2,RELOCOFFSET	# Relocate to actual data segment
		ADD	t3,RELOCOFFSET

1:		LR	t4,0(t1)	# read one cache line
		LR	t5,(REGSIZE*1)(t1)
		LR	t6,(REGSIZE*2)(t1)
		LR	t7,(REGSIZE*3)(t1)
		SR	t4,0(t2)	# write one cache line
		SR	t5,(REGSIZE*1)(t2)
		SR	t6,(REGSIZE*2)(t2)
		SR	t7,(REGSIZE*3)(t2)
		add	t1,(REGSIZE*4)
		add	t2,(REGSIZE*4)
		bltu	t2,t3,1b

#endif

#endif /* CFG_COPY_PSRAM==0 && CFG_COPY_PSRAM==0*/

		/*
		 * Flush the cache, then switch to relocated code
		 * We need to flush the cache since we just moved the code and
		 * it may still live in our L1 DCache.  We also need to
		 * flush L2, since there are some rare times we run
		 * uncached from DRAM, like when we start/stop a CPU.
		 *
		 * In the case of running completely uncached, don't flush the
		 * cache.  It should not have any dirty lines in it, but you
		 * never know...
		 */
#if CFG_RUNFROMKSEG0
		SETLEDS('L','1','2','F')
#if defined(_BCM96838_) || defined(_BCM96848_)
		li	a0,CFE_CACHE_FLUSH_D
#else
		li	a0,CFE_CACHE_FLUSH_D | CFE_CACHE_FLUSH_L2
#endif
		CALLINIT_KSEG0(init_table,R_INIT_CACHEOPS)

		li	a0,CFE_CACHE_INVAL_I
		CALLINIT_KSEG0(init_table,R_INIT_CACHEOPS)
#endif /* CFG_RUNFROMKSEG0 */


#if (CFG_COPY_PSRAM==0) && (CFG_BOOT_PSRAM==0)

		la	t0,gorelo	# Now jump to an address code was compiled for
		j	t0		# and go there
gorelo:		nop
		li	BOOT_OFFSET, 0  # no longer running at offset

#endif /* (CFG_COPY_PSRAM==0) && (CFG_BOOT_PSRAM==0) */


		/*
		* Remember total amount of memory.  This is *still* in k0
		 * after all this time.  Hopefully.
		 */
__MemVars:

#if defined(_BCM96838_) || defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM96848_)
#if defined(_BCM96848_)
		/* Bootrom ran also when straps configures to boot from spi-nand */
		li	t0,MISC_BASE
		lw	t1,MISC_STRAP_BUS(t0)
		and	t2,t1,MISC_STRAP_BUS_BOOT_SEL_MASK
		beq	t2,MISC_STRAP_BUS_BOOT_SPI_NAND,2f
		nop
#endif
		CALLINIT_KSEG0(init_table,R_INIT_BTRM_ENABLED)
		beq v0,0,1f
		nop
#if defined(_BCM963268_) || defined(_BCM963381_)
		li	k0,BTRM_INT_MEM_UTIL_SIZE
#elif defined(_BCM96838_) || defined(_BCM96848_)
2:
		li	k0,PSRAM_SIZE
#endif
1:		nop
#endif

		SR	k0,mem_totalsize
		SR	RELOCOFFSET,mem_datareloc

		move	v0,zero

		b	after_nvram

#if (INC_BTRM_BOOT==0)

		.org	0x56c
		.globl	_cferom_size
_cferom_size:
		.word	_romsize

		.org	0x570
		.byte	'c','f','e','-','v',CFE_VER_MAJOR,CFE_VER_MINOR,CFE_VER_BUILD,BCM63XX_MAJOR,BCM63XX_MINOR # CFE version info for applications
		.org	0x580			/* move past exception vectors */

		.globl	nvram_data_storage
nvram_data_storage:
		.word	NVRAM_DATA_ID
		.space	0x400

#endif


after_nvram:
		SR	ROMOPTION, rom_option	

		LOADREL(a0,segment_table)	# trashed by l2 cache flush
		LR	v0,R_SEG_FDATA(a0)
		ADD	v0,RELOCOFFSET
		LR	v1,R_SEG_END(a0)
		ADD	v1,RELOCOFFSET

		SR	v0,mem_bottomofmem
#if defined(CONFIG_BRCM_IKOS) && (defined(IKOS_BD_LINUX) || defined(IKOS_BD_LINUX_ROM))
		/* linux image is backdoor loaded to 0x80010000. Change cfe rom heap/stack to 4M offset to
		avoid cfe rom corrupt linux image*/
		li	v1,0x80400000
#endif
#if defined(_BCM96848_)
		/* In 6848, the cfe size for nor flash is larger than sram size, so heap should be set in ddr */
		li	v1, DRAM_BASE
#endif
		SR      v1,mem_stackbottom
		add     v1,STACK_SIZE
		SR      v1,mem_stacktop
		SR	v1,mem_heapstart	


#if defined(_BCM96838_) || defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM96848_)
#if defined(_BCM96848_)
		/* Bootrom ran also when straps configures to boot from spi-nand */
		li	t0,MISC_BASE
		lw	t1,MISC_STRAP_BUS(t0)
		and	t2,t1,MISC_STRAP_BUS_BOOT_SEL_MASK
		beq	t2,MISC_STRAP_BUS_BOOT_SPI_NAND,2f
		nop
#endif
		CALLINIT_KSEG0(init_table,R_INIT_BTRM_ENABLED)
		beq	v0,0,1f
		nop
#if defined(_BCM963268_) || defined(_BCM963381_)
		li	v1,BTRM_INT_MEM_END_ADDR
#elif defined(_BCM96838_) || defined(_BCM96848_)
2:
		li	v1,PSRAM_BASE_KSEG0
		add	v1,PSRAM_SIZE
#endif
		b	__set_topofmem
1:		nop
#endif
		add	v1,(CFG_ROM_HEAP_SIZE*1024)	# Otherwise
__set_topofmem:	SR	v1,mem_topofmem
		SR	TEXTOFFSET,mem_textreloc



		/* At this point it's safe to use the CALLINIT_RELOC macro */
#if defined(_BCM96848_) || defined(_BCM963381_)
		LOADREL(a0,segment_table)
#endif
		LR	t1,R_SEG_FTEXT(a0)
		FIXUP (t1);
		LR	t0,R_SEG_ETEXT(a0)
		FIXUP (t0);
		sub	t0,t0,t1
		SR	t0,mem_textsize
		add	t1,TEXTOFFSET
		SR	t1,mem_textbase

#------------------------------------------------------------------------------

		/*
		 * Stash away some config register stuff
		 */

		mfc0	v0,C0_PRID
		SR	v0,cpu_prid


#------------------------------------------------------------------------------

		/*
		 * Set up the "C" stack and jump to the main routine.
		 */

		SETLEDS('M','A','I','N')

#if (CFG_COPY_PSRAM==0) && (CFG_BOOT_PSRAM==0)

#if defined(_BCM96838_) || defined(_BCM963268_) || defined(_BCM963381_)
		CALLINIT_KSEG0(init_table,R_INIT_BTRM_ENABLED)
		beq	v0,0,1f
		nop
#if defined(_BCM963268_)
		li	sp,BTRM_INT_MEM_END_ADDR
#elif defined(_BCM96838_)
		li	sp,PSRAM_BASE_KSEG0
		add	sp,PSRAM_SIZE
#elif defined(_BCM963381_)
		li	sp,BTRM_INT_MEM_STACK_PTR_ADDR
#endif
		b	__sp_set
1:		nop
#endif
		LR	sp,mem_stacktop
                sub     sp,8
__sp_set:	nop

#endif


		li	a0,0			# call as "cfe_main(0,0)"
		li	a1,0

		CALLINIT_RELOC(init_table,R_INIT_CMDSTART)  # should not return





#if (CFG_COPY_PSRAM==1)

LEAF(copy_to_psram)

		move t5, ra

		/* Copy code to PSRAM only in TP0 */
		mfc0	t1, C0_BCM_CONFIG, 3
		li	t2, CP0_CMT_TPID
		and	t1, t2
		bnez	t1, __jump_to_psram			# Already running on TP1

/* ------------------------------------------------------- */
		LOADREL(a0,segment_table)
		LR	CODE_SRC_START,R_SEG_FTEXT(a0)		# Source address
		FIXUP (CODE_SRC_START)
		LR	DATA_SRC_END,R_SEG_EDATA(a0)
		FIXUP (DATA_SRC_END)

		/* The copy tp PSRAM is done in two steps:
			1. From FLASH: Copy tiny code that will copy all text and data
			2. From PSRAM: Run tiny code, it will copy all text and data from FLASH to PSRAM
		*/

__CopyFromFlash:

		/* Copy to the last 16KB of PSRAM */
		la	t1,PSRAM_BASE		# destination address
		add	t1,PSRAM_SIZE
		sub t1,0x4000
		move    TEXTBASE,t1

		la	t2,__goCopy		# Source address
		FIXUP (t2);
		la	t3,__EndOfCopy
		FIXUP (t3);

1:		LR	t4,0(t2)	# read 32bit word
		SR	t4,0(t1)	# write 32bit word
		add	t1,REGSIZE
		add	t2,REGSIZE
		bltu	t2,t3,1b

		move t0, TEXTBASE           # Now jump to an address in PSRAM
		j	t0			# and go there
__goCopy:
		GET_BOOT_OFFSET(BOOT_OFFSET)

		/*
		 *Copy Code
		 */

/*		LOADREL(a0,segment_table) */
__CopyFromPsram:

		la	t1,PSRAM_BASE		# destination address - PSRAM basee address
		move	TEXTBASE,t1

		move	t2,CODE_SRC_START		# Source address
		move	t3,DATA_SRC_END

		/* flash has only 64KB direct access window size, only copy up to that limit */
		sub	t4, t3, t2
		blt	t4, 0x10000, 1f
		add	t3, t2, 0x10000

1:		LR	t4,0(t2)	# read one cache line
		SR	t4,0(t1)	# write one cache line
		add	t1,REGSIZE
		add	t2,REGSIZE
		bltu	t2,t3,1b

__jump_to_psram:

		/* now jump to PSRAM where the code was compiled to */
		nop
		la	t0, __goPsram
		j	t0			# and go there

/* --------------------------------------------------------------- */

__goPsram:
		GET_BOOT_OFFSET(BOOT_OFFSET)
		/* continue to the mips init where we break from */
		la     t0, psram_cp_done
		j      t0
__EndOfCopy:

END(copy_to_psram)
#endif

#if defined(CFE_ABORT_KEY)
LEAF(chek_abort_key)

		move	t5, ra

		li	v0, 0
		li	t0, 'a'		 /* 'a' for abort */
		CALLINIT_KSEG1(init_table,R_INIT_GETC)
		bne	v0, t0, end_loop
		nop

		LOADREL(a0,print_options)
		CALLINIT_KSEG1(init_table,R_INIT_PUTS)

start_loop:
		li	v0, 0
		CALLINIT_KSEG1(init_table,R_INIT_GETC)

		li	t0, 'c'		 /* 'c' for continue */
		beq	v0, t0, end_loop
		nop

		li	t0, 'i'		 /* 'i' for alternate image selction*/
		beq	v0, t0, 1f
		nop

		b	start_loop
		nop

1:		CALLINIT_KSEG1(abort_table,R_ABORT_SETROMPARM)
		nop

end_loop:
		move	ra,t5
		//	FIXUP(ra);
		jr	ra

END(chek_abort_key)
#endif   /* CFE_ABORT_KEY */

/*  *********************************************************************
    *  _GETTICKS()
    *
    *  Read the COUNT register into v0
    *
    *  Input parameters:
    *     nothing
    *
    *  Return value:
    *     v0 - count register
    ********************************************************************* */

LEAF(_getticks)

		mfc0	v0,C0_COUNT
		j	ra
END(_getticks)

/*  *********************************************************************
    *  get_SP()
    *  
    *  Return SP value 
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   v0 - SP register
    **********************************************************************/

LEAF(get_SP)
		move	v0,sp
		j	ra
END(get_SP)

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
    *      a0 - entry vector
    *
    *  Return value:
    *      does not return
    ********************************************************************* */

LEAF(cfe_launch)

		sub	sp,8
		SR	a0,0(sp)

		/*
		 * This function runs in RAM so BOOT_OFFSET is 0. It is called from
		 * C which could have modified the BOOT_OFFSET register, s6.
		 */
		li	BOOT_OFFSET, 0

		/*
		 * Mask all interrupts.
		 */
		mfc0	v0,C0_SR		# Get current interrupt flag
		li	v1,M_SR_IE		# master interrupt control
		not	v1			# disable interrupts
		and	v0,v1			# SR now has IE=0
		mtc0	v0,C0_SR		# put back into CP0


#if defined(_BCM96838_) || (defined(_BCM963268_) && (INC_BTRM_BOOT==1)) || (defined(_BCM963381_) && (INC_BTRM_BOOT==1))

		CALLINIT_RELOC(init_table,R_INIT_BOOT_SECURE)
		nop
		beq	v0,0,__skip_shred
		nop

		.set noreorder

		# before we launch the next program, shred any existence of the ROM program 
		# by creating a little shredder program in uncached physical internal memory 
		li	t1, BTRM_INT_MEM_SHREDDER_PROG_ADDR  # cached addr in internal mem
		or	t1, (1<<29)	# make it uncached address

		# Load t2 and t3 with the beginning and end cached addresses of where the 
		# shredder program resides within the BTRM code
		la	t2, __shredBegin
		la	t3, __shredEnd

1:		# Load shredder code to specified uncached memory area

		lw	t4, 0(t2)
		sw	t4, 0(t1)
		add	t2, 4
		add	t1, 4
		bne	t2, t3, 1b
		nop
		nop

		b	__shredEnd
		nop
		nop

__shredBegin:   # beginning of actual shredder code that will be copied over

		# Save the CFE RAM entry point off of the stack because the stack mem location will be scrubbed
		LR	t9,0(sp)

		# clean 1st swath of cached memory
		li	t5,BTRM_INT_MEM_BEGIN_ADDR
#if defined(_BCM963268_) || defined(_BCM963381_)
		li	t6,BTRM_INT_MEM_SHREDDER_PROG_ADDR
#elif defined(_BCM96838_)
		li	t6,BTRM_INT_MEM_SBI_LINK_ADDR
#endif
1:		sw	zero,0(t5)
		sw	zero,4(t5)
		sw	zero,8(t5)
		sw	zero,12(t5)
		add	t5,16
		blt	t5,t6,1b
		nop
		nop

		# clean 2nd swath of cached memory
#if defined(_BCM963268_) || defined(_BCM963381_)
		li	t5,BTRM_INT_MEM_CREDENTIALS_ADDR
#if defined(_BCM963268_)
		li	t6,BTRM_INT_MEM_END_ADDR
#else
		li	t6,BTRM_INT_MEM_32K_BLOCK_END_ADDR
#endif
		beq	t9,0,1f /* launch addr was 0, clean credentials */
		nop
		nop
		add	t5,0x1000 /* launch addr was not 0, leave credentials */
#elif defined(_BCM96838_)
		li	t5,BTRM_INT_MEM_128K_BLOCK
		li	t6,BTRM_INT_MEM_128K_BLOCK_END_ADDR
#endif
1:		sw	zero,0(t5)
		sw	zero,4(t5)
		sw	zero,8(t5)
		sw	zero,12(t5)
		add	t5,16
		blt	t5,t6,1b
		nop
		nop

		# clean 3rd swath of cached memory (if necessary)
#if defined(_BCM96838_)
		# If CFE RAM failed authentication, clean up the 32k unmapped block in 
		# which the security credentials reside (except the part that holds shredder)
		bne	t9,0,2f
		nop
		li	t5,BTRM_INT_MEM_CREDENTIALS_ADDR
		li	t6,BTRM_INT_MEM_32K_BLOCK_END_ADDR
1:		sw	zero,0(t5)
		sw	zero,4(t5)
		sw	zero,8(t5)
		sw	zero,12(t5)
		add	t5,16
		blt	t5,t6,1b
		nop
		nop
2:
#endif

		# shred encryption credentials (even if auth passed)
		li	t5,BTRM_INT_MEM_CREDENTIALS_ADDR
		add	t5,512
		li	t6,BTRM_INT_MEM_CREDENTIALS_ADDR
		add	t6,512
		add	t6,64
1:		sw	zero,0(t5)
		sw	zero,4(t5)
		sw	zero,8(t5)
		sw	zero,12(t5)
		add	t5,16
		blt	t5,t6,1b
		nop
		nop

		# Flush the D-Cache, and invalidate the I-cache
		# so that physical uncached memory is cleaned and no
		# traces of the CFE ROM code exists anywhere
		flush_inval_caches()
		unlockJtag()

		#At this point, we are ready to launch CFE RAM if auth passed, or loop forever if auth failed
		beq	t9, 0, __authFailed
		nop
		nop

		move	a0,t9
		j	t9  # Launch CFE ROM code ... never to return .....
		nop
		nop

__authFailed:
		nop
		nop
		b	__authFailed			# loop forever
		nop
		nop

__shredEnd:	 # end of actual shredder code that will be copied over

		.set reorder

		# Run the LMEM shredder program at the uncached physical address
		li	t1, BTRM_INT_MEM_SHREDDER_PROG_ADDR	  # addr in internal mem			
		or	t1, (1<<29) # make it uncached address
		j	t1

#endif /* defined(_BCM963268_) || defined(_BCM96838_) || defined(_BCM963381_)*/

		/*
		 * Flush the D-Cache, since the program we loaded is "data".
		 * Invalidate the I-Cache, so that addresses in the program
		 * region will miss and need to be filled from the data we
		 * just flushed above.
		 */
__skip_shred:	li	a0,CFE_CACHE_FLUSH_D|CFE_CACHE_INVAL_I
		CALLINIT_RELOC(init_table,R_INIT_CACHEOPS)

		/*
		 * Set things up for launching the program.  Pass the
		 * handle in A0 - apps need to remember that and pass it
		 * back.
		 */
		j	RunProgram

END(cfe_launch)


		/*
		 * This is a nice place to set a breakpoint.
		 */
LEAF(RunProgram)
		LR	t0,0(sp)	# entry point

		j	t0		# go for it.
END(RunProgram)

/*  *********************************************************************
    *  CPU_KSEG0_SWITCH
    *
    *  Hack the return address so we will come back in KSEG0
    *
    *  Input parameters:
    *     nothing
    *
    *  Return value:
    *     nothing
    ********************************************************************* */

LEAF(cpu_kseg0_switch)

		and	ra,(K0SIZE-1)
		or	ra,K0BASE
		jr	ra

END(cpu_kseg0_switch)

/**********************************************************************/

		.globl  rel_version
rel_version:
		.ascii BRCM_VERSION,".",BRCM_RELEASE BRCM_EXTRAVERSION
		.ascii "-",CFE_VER_MAJ_STR,".",CFE_VER_MIN_STR,".",CFE_VER_ECO_STR
		.ascii "-",CFE_MAJOR_STR,".",CFE_MINOR_STR
		.ascii "\r"
		.asciiz "\n"
		.align 2

#if defined(CFE_ABORT_KEY)
		.global print_options
print_options:
		.ascii "\r\n"
		.ascii "CFE-ROM WAS ABORTED\r\n"
		.ascii "Options:\r\n"
		.ascii " c  - continue\r\n"
		.ascii " i  - alternate image selection\r\n"
		.asciiz "\n"
		.align 2

		/* This function table consists of pointers to function used
		 * when stop cfe-rom with 'a' key
		 */
	.globl  abort_table
abort_table:
		_LONG_	set_romparm        # [  1] R_ABORT_SETROMPARM
#endif

/*  *********************************************************************
    *  End
    ********************************************************************* */

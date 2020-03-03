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

/*  *********************************************************************
    *  Macros
    ********************************************************************* */

#include "mipsmacros.h"
#include "utils_init.h"

/*  *********************************************************************
    *  SETLEDS(a,b,c,d)
    *  SETLEDS1(a,b,c,d)
    *  
    *  Sets the on-board LED display (if present).  Two variants
    *  of this routine are provided.  If you're running KSEG1,
    *  call the SETLEDS1 variant, else call SETLEDS.
    *  
    *  Input parameters: 
    *  	   a,b,c,d - four ASCII characters (literal constants)
    *  	   
    *  Return value:
    *  	   a0,k1,ra trashed
    ********************************************************************* */

#define SETLEDS(a,b,c,d)                     \
       li     a0,(((a)<<24)|((b)<<16)|((c)<<8)|(d)) ;    \
       CALLINIT_KSEG0(init_table,R_INIT_SETLEDS)

#define SETLEDS1(a,b,c,d)                     \
       li     a0,(((a)<<24)|((b)<<16)|((c)<<8)|(d)) ;    \
       CALLINIT_KSEG1(init_table,R_INIT_SETLEDS)


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
#define BOOT_SEL	s5			/* 0 boot from NAND flash, 1 boot from serial or parallel flash */
#define FLASHBASE       s4	

		.extern	_fdata
		.extern	_edata
		.extern	_etext


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


		.globl segment_table
segment_table:
		_LONG_	_etext			# [  0] End of text (R_SEG_ETEXT)
		_LONG_	_fdata			# [  1] Beginning of data (R_SEG_FDATA)
		_LONG_	_edata			# [  2] End of data (R_SEG_EDATA)
		_LONG_	_end			# [  3] End of BSS (R_SEG_END)
		_LONG_	_ftext			# [  4] Beginning of text (R_SEG_FTEXT)
		_LONG_	_fbss			# [  5] Beginning of BSS (R_SEG_FBSS)
		_LONG_	_gp 			# [  6] Global Pointer (R_SEG_GP)
		_LONG_  0   			# [  7] Beginning of reloc entries
		_LONG_  0   			# [  8] End of reloc entries
		_LONG_	0   			# [  9] R_SEG_APIENTRY
		_LONG_  _cferam			# [ 10] R_SEG_CFERAM
		_LONG_  _nandend		# [ 11] R_SEG_NAND_END

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


		.globl  init_table
init_table:
		_LONG_  board_earlyinit         # [  0] R_INIT_EARLYINIT
		_LONG_  board_setleds           # [  1] R_INIT_SETLEDS
		_LONG_  board_draminfo          # [  2] R_INIT_DRAMINFO
		_LONG_	CPUCFG_CPUINIT          # [  3] R_INIT_CPUINIT
		_LONG_  CPUCFG_ALTCPU_START1    # [  4] R_INIT_ALTCPU_START1
		_LONG_  CPUCFG_ALTCPU_START2    # [  5] R_INIT_ALTCPU_START2
		_LONG_  CPUCFG_ALTCPU_RESET     # [  6] R_INIT_ALTCPU_RESET
		_LONG_  CPUCFG_CPURESTART       # [  7] R_INIT_CPURESTART
		_LONG_  bcmcore_null            # [  8] R_INIT_DRAMINIT
		_LONG_  CPUCFG_CACHEOPS         # [  9] R_INIT_CACHEOPS
		_LONG_  bcmcore_null            # [ 10] R_INIT_TLBHANDLER
		_LONG_	cfe_main                # [ 11] R_INIT_CMDSTART
		_LONG_	bcmcore_null            # [ 12] R_INIT_CMDRESTART
		_LONG_  bcmcore_null            # [ 13] R_INIT_DOXREQ
		_LONG_  bcmcore_null            # [ 14] R_INIT_TP1_SWITCH
		_LONG_  cfe_size_ram            # [ 15] R_INIT_SIZERAM
		_LONG_  board_init              # [ 16] R_INIT_BOARDINIT
		_LONG_  bcmcore_null            # [ 17] R_INIT_PLL
		_LONG_  board_puts              # [ 18] R_INIT_PUTS
		_LONG_  bcmcore_null            # [ 19]	R_INIT_GPIOLOOP
		_LONG_  nand_ldr                # [ 20]	R_INIT_NANDLDR
#if defined(_BCM963381_)
		_LONG_  pmc_ll_init             # [ 21]	R_INIT_PMC
#else
		_LONG_  bcmcore_null            # [ 21] R_INIT_PMC
#endif
#if defined(CFE_ABORT_KEY)
		_LONG_  board_getc              # [ 22] R_INIT_GETC
#endif

		/* define the nand chip structure and other variable in the first 16KB */
		.globl  g_chip
g_chip:
		/* match the size of CFE_NAND_CHIP in nandflash.c */
		.word   0
		.space  48

		.global g_fname
g_fname:
		.byte   'c','f','e','r','a','m','.','0','0','0', 0, 0

		.global g_fname_util
g_fname_util:
		.byte   'c','f','e','a','u','t','l', 0

		.global g_fname_actual_len
g_fname_actual_len:
		/*g_fname_actual_len = sizeof(g_fname) - 1; */
		.word 10

		.global g_fname_cmp_len
g_fname_cmp_len:
		/*g_fname_cmp_len = sizeof(g_fname) - 4;*/
		.word 7

		.global g_fname_util_len
g_fname_util_len:
		.word 7


/*  *********************************************************************
    *  CPU Startup Code
    ********************************************************************* */

cpu_reset:

	/*
	 * Start with GP as zero.  Nobody should touch
	 * this or set it to any other value until we're ready
	 * to use it.  This is used to tell when we should start
	 * using relocated references in the init table,
	 * so beware!  (see CALLINIT_RELOC in mipsmacros.h)
	 */
		move	gp,zero			# start with no GP.
		
		.set noreorder
		bal 1f
		nop
1:		nop
		.set reorder
		li  FLASHBASE, 0x1fff0000
		and FLASHBASE, ra

		GET_BOOT_OFFSET(BOOT_OFFSET)

		/* sp register can contain any garbage value upon power on.
		when debugging this piece of early code, jtag reads the stack and
		might hit invalid address. this cause bus error and jtag break in failure.
		so setup the sp to point to the rom just to make sure debugger software can read
		the "stack" content successfully. you might still have to close the call frame
		window since there is no valid the call frames in the rom */
		move   sp, FLASHBASE
		or     sp, K1BASE ;
		add    sp, 0x400

#------------------------------------------------------------------------------

	/*
	 * Do low-level board initialization.  This is our first
	 * chance to customize the startup sequence.
	 */
		/* a0 must preserved for R_INIT_EARLYINIT call */
		move   a0, FLASHBASE

		CALLINIT_KSEG1(init_table,R_INIT_EARLYINIT)

		SETLEDS1('H','E','L','O')

		CALLINIT_KSEG1(init_table,R_INIT_CPUINIT)

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

		LOADREL(a0,rel_version)
		CALLINIT_KSEG0(init_table,R_INIT_PUTS)
	
#if defined(CFE_ABORT_KEY)
		bal   chek_abort_key
		nop
#endif

		SETLEDS('F','A','P','M')
#if defined(_BCM963268_)
        li      t1, TIMR_BASE
        lw      t0, TIMER_CLKRSTCTL(t1)
		or	    t0, (TIMER_CLKRSTCTL_FAP1_PLL_CLKEN | TIMER_CLKRSTCTL_FAP2_PLL_CLKEN)
        sw      t0, TIMER_CLKRSTCTL(t1)
#endif

#if defined(_BCM963268_)
        li      t1, MISC_BASE
        lw      t0, MISC_IDDQ_CONTROL(t1)
		and 	t0, ~MISC_IDDQ_CONTROL_IDDQ_CTRL_FAP
        sw      t0, MISC_IDDQ_CONTROL(t1)

        li      t1, PERF_BASE
        lw      t0, PERF_CLKCONTROL(t1)
		or	    t0, (PERF_CLKCONTROL_FAP0_CLKEN | PERF_CLKCONTROL_FAP1_CLKEN)
        sw      t0, PERF_CLKCONTROL(t1)

        li      t1, PERF_BASE
        lw      t0, PERF_SOFTRESETB(t1)
		or	    t0, (PERF_SOFTRESETB_SOFT_RST_FAP0_N | PERF_SOFTRESETB_SOFT_RST_FAP1_N)
        sw      t0, PERF_SOFTRESETB(t1)
#endif



#if defined(_BCM963381_)
	CALLINIT_KSEG0(init_table, R_INIT_PMC)
#endif	

   		li      BOOT_SEL, 0
#if (defined (_BCM963268_))
   		li      t1, MISC_BASE
        lw      t0, MISC_STRAP_BUS(t1)
		and     t0, MISC_STRAP_BUS_BOOT_SEL_MASK
		srl     t0, MISC_STRAP_BUS_BOOT_SEL_SHIFT
		beq     t0, MISC_STRAP_BUS_BOOT_NAND, 1f
		add     BOOT_SEL, 1
1:
#endif




#ifdef  _BCM963381_
		li      t1, MISC_BASE
		lw      t0, MISC_STRAP_BUS(t1)
		and     t0, MISC_STRAP_BUS_BOOT_SPI_NOR
		bne     t0, MISC_STRAP_BUS_BOOT_SPI_NOR, 1f
		add     BOOT_SEL, 1
1:
#endif	
		SETLEDS('A','2','4','6')

        li      RELOCOFFSET,0           # not relocating, no offset
        li      TEXTOFFSET,0

		LOADREL(a0,segment_table)
		LR	gp,R_SEG_GP(a0)
		add	gp,RELOCOFFSET

#------------------------------------------------------------------------------
       /* NAND boot only need to load first 16K text */
       bne BOOT_SEL, 1, __CopyCode
__ZeroBss:
	/*
	 * Zero BSS
         */
		SETLEDS('Z','B','S','S')

		LOADREL(a0,segment_table)

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
	/*
	 * Copy code
	 */
__CopyCode:
		SETLEDS('C','O','D','E')

		LOADREL(a0,segment_table)

		LR	t1,R_SEG_FTEXT(a0)		# destination address
		move    TEXTBASE,t1
			
		LR	t2,R_SEG_FTEXT(a0)		# Source address
		FIXUP (t2);

		bne BOOT_SEL, 1, nand_end_addr
		LR	t3,R_SEG_ETEXT(a0)
		b   1f
nand_end_addr:
		LR	t3,R_SEG_NAND_END(a0)
1:
 		# LR	t3,R_SEG_FDATA(a0) # FIXME -- keep going until data starts
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

		bne BOOT_SEL, 1, __GoRelo
 		b   __CopyData

		.org    0x570
		.byte   'c','f','e','-','v',CFE_VER_MAJOR,CFE_VER_MINOR,CFE_VER_BUILD,BCM63XX_MAJOR,BCM63XX_MINOR # CFE version info for applications
		.org	0x580			# move past exception vectors

                .globl nvram_data_storage
		.word NVRAM_DATA_ID
                .space 0x400


#------------------------------------------------------------------------------
	/*
	 * Copy initialized data
         */
__CopyData:
#if (CFG_BOOTRAM == 0)
		SETLEDS('D','A','T','A')

		LOADREL(a0,segment_table)

		LR	t1,R_SEG_FDATA(a0)
		FIXUP (t1);
		li	t0,15
		add	t1,t0
		not	t0
		and	t1,t0		# t1 = _etext rounded up to 16-byte boundary
			
		LR	t2,R_SEG_FDATA(a0)
		/* we only need to copy till reach cfe ram image data because the whole
		data size will execeed 64KB local memory */
		LR	t3,R_SEG_CFERAM(a0)
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

#------------------------------------------------------------------------------

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

__GoRelo:

#if CFG_RUNFROMKSEG0
		SETLEDS('L','1','2','F')

		li	a0,CFE_CACHE_FLUSH_D | CFE_CACHE_FLUSH_L2
		CALLINIT_KSEG0(init_table,R_INIT_CACHEOPS)
		li	a0,CFE_CACHE_INVAL_I
		CALLINIT_KSEG0(init_table,R_INIT_CACHEOPS)
#endif /* CFG_RUNFROMKSEG0 */

		la t0,gorelo            # Now jump to an address code was compiled for
		j	t0			# and go there
gorelo:		nop
		li  BOOT_OFFSET, 0      # no longer running at offset

#------------------------------------------------------------------------------
	/*
	 * Set up the "C" stack and jump to the main routine.
         */
        /*
         * For bootrom builds, "C" stack is in LMEM or FAP0
         * We have total 64K code/data, stack and heap. heap is starts
         * at the cfe_ram data arrage. There is no really heap function
         * in the cfe rom utily. Just used it as free memory. Currently
         * only the decompression and NAND driver use heap(about 16K and
         * maximum 8K respectively).  The code size is about 30K so this
         * leave us 18K for stack in worse case. Should be good enough.
         */
		li  sp, UTIL_STACK_TOP
		bne BOOT_SEL, 1, nand_loader

		sub sp,8
		li	a0,0			# call as "cfe_main(0,0)"
		li	a1,0

		SETLEDS('M','A','I','N')

		CALLINIT_RELOC(init_table,R_INIT_CMDSTART)  # should not return

nand_loader:
		SETLEDS('N','D','L','R')
		sub sp,4
		li	a0,1			# call as "nand_ldr(util_load=1)"
		CALLINIT_RELOC(init_table,R_INIT_NANDLDR)  # should not return

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


	/*
	 * Flush the D-Cache, since the program we loaded is "data".
	 * Invalidate the I-Cache, so that addresses in the program
	 * region will miss and need to be filled from the data we 
	 * just flushed above.
	 */

		li	a0,CFE_CACHE_FLUSH_D|CFE_CACHE_INVAL_I
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
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

LEAF(cpu_kseg0_switch)

		and	ra,(K0SIZE-1)
		or	ra,K0BASE
		jr	ra

END(cpu_kseg0_switch)

	
/*  *********************************************************************
    *  _GETTICKS()
    *  
    *  Read the COUNT register into v0
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   v0 - count register
    ********************************************************************* */

LEAF(_getticks)

		mfc0	v0,C0_COUNT
		j	ra
END(_getticks)

#if defined(CFE_ABORT_KEY)
LEAF(chek_abort_key)

    move t5, ra

    li  v0, 0
    li  t0, 'a'      /* 'a' for abort */
    CALLINIT_KSEG1(init_table,R_INIT_GETC)
    bne v0, t0, end_loop
    nop
       
    LOADREL(a0,print_options)
    CALLINIT_KSEG1(init_table,R_INIT_PUTS)

start_loop:
    li  v0, 0
    CALLINIT_KSEG1(init_table,R_INIT_GETC)

    li  t0, 'c'      /* 'c' for continue */
    beq v0, t0, end_loop
    nop

    li  t0, 'p'      /* 'p' for template function 1 */
    beq v0, t0, 2f
    nop

    li  t0, 's'      /* 's' for template function 2 */
    beq v0, t0, 3f
    nop

    b   start_loop
    nop

2:  CALLINIT_KSEG1(abort_table,R_ABORT_TEMP1)
    b   start_loop
    nop

3:  CALLINIT_KSEG1(abort_table,R_ABORT_TEMP2)
    b   start_loop
    nop
        
end_loop:
    move    ra,t5
//    FIXUP(ra);
    jr      ra

END(chek_abort_key)
#endif   /* CFE_ABORT_KEY */

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
    .ascii " p  - template function 1\r\n"
    .ascii " s  - template function 2\r\n"
    .asciiz "\n"
    .align 2

/* This function table consists of pointers to function used 
   when stop cfe-rom with 'a' key   */
	.globl  abort_table
abort_table:
        _LONG_  template_func1        # [  0] R_ABORT_TEMP1
        _LONG_  template_func2        # [  1] R_ABORT_TEMP2
#endif	
/*  *********************************************************************
    *  End
    ********************************************************************* */



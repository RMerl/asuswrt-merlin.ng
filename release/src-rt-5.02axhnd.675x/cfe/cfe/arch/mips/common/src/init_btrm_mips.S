/**********************************************************************
 *  
 *  init_btrm_mips.S       
 *
 *  Author:  Brian Nay (brian.nay@broadcom.com)
 *  
 *********************************************************************  
 *
 *  Copyright 2011
 *  Broadcom Corporation. All rights reserved.
 *  
 ********************************************************************* 
 */

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
#include "bcm_map.h"


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
        addu    t1, kva, size;   \
        subu    t2, linesize, 1; \
        not     t2;              \
        and     t0, kva, t2;     \
        addu    t1, -1;          \
        and     t1, t2;          \
10:     cache   op, 0(t0);       \
        bne     t0, t1, 10b;     \
        addu    t0, linesize;    \
11:                              

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
#define FLASHBASE       s5

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
DECLARE_VECTOR(0x0008,vec_soft_reset,cpu_soft_reset)

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

		.org    0x570
		.byte   'c','f','e','-','v',CFE_VER_MAJOR,CFE_VER_MINOR,CFE_VER_BUILD,BCM63XX_MAJOR,BCM63XX_MINOR # CFE version info for applications
		.org	0x580			# move past exception vectors

	/*
	 * BCM963XX NVRAM Data Storage
         * there is no NVRAM allocation within the secure bootrom. 
	 */

		.globl nvram_data_storage
nvram_data_storage:
		.word	NVRAM_DATA_ID

		.globl segment_table
segment_table:
		_LONG_	_etext			# [  0] End of text (R_SEG_ETEXT)
		_LONG_	_fdata			# [  1] Beginning of data (R_SEG_FDATA)
		_LONG_	_edata			# [  2] End of data (R_SEG_EDATA)
		_LONG_	_end			# [  3] End of BSS (R_SEG_END)
		_LONG_	_ftext			# [  4] Beginning of text (R_SEG_FTEXT)
		_LONG_	_fbss			# [  5] Beginning of BSS (R_SEG_FBSS)
		_LONG_	_gp			# [  6] Global Pointer (R_SEG_GP)
		_LONG_  0			# [  7] Beginning of reloc entries
		_LONG_  0			# [  8] End of reloc entries
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


		.globl  init_table
init_table:
		_LONG_  board_earlyinit         # [  0] R_INIT_EARLYINIT
		_LONG_  board_setleds           # [  1] R_INIT_SETLEDS
		_LONG_  board_draminfo		# [  2] R_INIT_DRAMINFO
		_LONG_	CPUCFG_CPUINIT		# [  3] R_INIT_CPUINIT
		_LONG_  CPUCFG_ALTCPU_START1	# [  4] R_INIT_ALTCPU_START1
		_LONG_  CPUCFG_ALTCPU_START2	# [  5] R_INIT_ALTCPU_START2
		_LONG_  CPUCFG_ALTCPU_RESET     # [  6] R_INIT_ALTCPU_RESET
		_LONG_  CPUCFG_CPURESTART	# [  7] R_INIT_CPURESTART
		_LONG_  CPUCFG_DRAMINIT		# [  8] R_INIT_DRAMINIT
		_LONG_  CPUCFG_CACHEOPS		# [  9] R_INIT_CACHEOPS
		_LONG_  CPUCFG_TLBHANDLER       # [ 10] R_INIT_TLBHANDLER
		_LONG_	cfe_main		# [ 11] R_INIT_CMDSTART
		_LONG_	cfe_command_restart	# [ 12] R_INIT_CMDRESTART
		_LONG_  cfe_doxreq		# [ 13] R_INIT_DOXREQ
		_LONG_  CPUCFG_TP1_SWITCH	# [ 14] R_INIT_TP1_SWITCH
		_LONG_  bcmcore_null		# [ 15] R_INIT_SIZERAM
		_LONG_  board_init	        # [ 16] R_INIT_BOARDINIT
		_LONG_  bcmcore_null            # [ 17] R_INIT_PLL
		_LONG_  bcmcore_null            # [ 18] R_INIT_PUTS
		_LONG_  bcmcore_null            # [ 19]	R_INIT_GPIOLOOP
		_LONG_  bcmcore_null            # [ 20]	R_INIT_NANDLDR
#if defined(_BCM963381_)
		_LONG_  pmc_ll_init             # [ 21]	R_INIT_PMC
#else
		_LONG_  bcmcore_null            # [ 21]	R_INIT_PMC
#endif
	
/*  *********************************************************************
    *  CPU Startup Code
    ********************************************************************* */

cpu_reset:
cpu_soft_reset:

#if defined(_BCM963268_)
		/* Just in case this is a soft reset, put the PHY MIPs into reset */
        	li      t1, MISC_BASE
		lw      t0, 0(t1)
		and	t0, ~VDSL_MIPS_RESET_BIT
        	sw      t0, 0(t1)
#endif

		move	gp,zero			# start with no GP.
		
		.set noreorder
		bal 1f
		nop
1:		nop
		.set reorder
		li  FLASHBASE, 0x1fff0000
		and FLASHBASE, ra

                GET_BOOT_OFFSET(BOOT_OFFSET)	

	 	/* Do low-level board initialization. */
		move   a0, FLASHBASE
		CALLINIT_KSEG1(init_table,R_INIT_EARLYINIT)

         	/* If running on TP0, print btrm, version; if running on TP1, skip */
                mfc0    t1, $22, 3
                li      t2, (0x1 << 31)
                and     t1, t2
                bnez    t1, __skipBtrm
                SETLEDS1('-','-','-','-')
                SETLEDS1('B','T','R','M')
	  	SETLEDS1('V','1','.','3')

__skipBtrm:	nop
                nop


		CALLINIT_KSEG1(init_table,R_INIT_CPUINIT)


	 	/* Now, switch from KSEG1 to KSEG0 */
#if CFG_RUNFROMKSEG0
		bal	cpu_kseg0_switch
		GET_BOOT_OFFSET(BOOT_OFFSET)	
#endif
	 	/* Now running on cpu0 in K0SEG. */



         	/* If running on TP0, set up LMEM, TLB; if running on TP1, skip it */
                mfc0    t1, $22, 3
                li      t2, (0x1 << 31)
                and     t1, t2
                bnez    t1, __skipTp1
                nop

		/* perform any additional board initialization */
        	CALLINIT_KSEG0(init_table, R_INIT_BOARDINIT)
	
#if defined(_BCM963381_)
		/* init pmc to turn on vdsl model */
        	CALLINIT_KSEG0(init_table, R_INIT_PMC)

#elif defined(_BCM963268_)
                SETLEDS('L','M','E','M')

		/* turn on vdsl memory */
                li      t1, MISC_BASE
		lw      t0, MISC_IDDQ_CONTROL(t1)
		and	t0, ~MISC_IDDQ_CTRL_VDSL_PHY_BIT
                sw      t0, MISC_IDDQ_CONTROL(t1)

		lw      t0, 0(t1)
		or	t0, VDSL_PHY_RESET_BIT
		or	t0, VDSL_MIPS_POR_RESET_BIT
                sw      t0, 0(t1)
#endif 
         	/* After L1C init, TP1 will branch to here */
__skipTp1:	nop
                nop

         	/* Check if the thread switch is required. */
		CALLINIT_KSEG0(init_table,R_INIT_TP1_SWITCH)



                li      RELOCOFFSET,0           # not relocating, no offset
                li      TEXTOFFSET,0

		LOADREL(a0,segment_table)
		LR	gp,R_SEG_GP(a0)
		add	gp,RELOCOFFSET


	 	/* Zero BSS */
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


	 	/* Copy code */
		SETLEDS('C','O','D','E')
		LOADREL(a0,segment_table)
__CopyCode:
		LR	t1,R_SEG_FTEXT(a0)		# destination address
		move    TEXTBASE,t1
			
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


	 	/* Copy initialized data */
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


	 	/* Flush cache, switch to relocated code */
__GoRelo:
#if CFG_RUNFROMKSEG0
		SETLEDS('L','1','2','F')

#if defined(_BCM96838_) || defined(_BCM96848_)
                li      a0,CFE_CACHE_FLUSH_D
#else
		li	a0,CFE_CACHE_FLUSH_D | CFE_CACHE_FLUSH_L2
#endif
		CALLINIT_KSEG0(init_table,R_INIT_CACHEOPS)
		li	a0,CFE_CACHE_INVAL_I
		CALLINIT_KSEG0(init_table,R_INIT_CACHEOPS)
#endif /* CFG_RUNFROMKSEG0 */


	 	la t0,gorelo            # Now jump to an address code was compiled for
		j	t0			# and go there
gorelo:		nop
		li  BOOT_OFFSET, 0      # no longer running at offset


__MemVars:
                li      v0,BTRM_INT_MEM_UTIL_SIZE
		SR	v0,mem_totalsize
		SR	RELOCOFFSET,mem_datareloc

		move	v0,zero

		LOADREL(a0,segment_table)	# trashed by l2 cache flush
		LR	v0,R_SEG_FDATA(a0)
		ADD	v0,RELOCOFFSET
		LR	v1,R_SEG_END(a0)
		ADD	v1,RELOCOFFSET

		SR	v0,mem_bottomofmem
		SR	v1,mem_heapstart


         	/* for bootrom builds, stacks are in internal MEM (LMEM or FAP0) */
#if defined(_BCM963268_)
		li      v1,BTRM_INT_MEM_END_ADDR
#elif defined(_BCM96838_) || defined(_BCM96848_) || defined(_BCM963381_)
		li      v1,BTRM_INT_MEM_STACK_PTR_ADDR
#endif
		SR	v1,mem_topofmem


		SR	TEXTOFFSET,mem_textreloc

		/* At this point it's safe to use the CALLINIT_RELOC macro */

		LR	t1,R_SEG_FTEXT(a0)
		FIXUP (t1);
		LR	t0,R_SEG_ETEXT(a0)
		FIXUP (t0);
		sub	t0,t0,t1
		SR	t0,mem_textsize
		add	t1,TEXTOFFSET
		SR	t1,mem_textbase


	 	/* Stash away some config register stuff */
		mfc0	v0,C0_PRID
		SR	v0,cpu_prid		


	 	/* Set up the "C" stack and jump to the main routine. */
		SETLEDS('M','A','I','N')

         	/* For bootrom builds, "C" stack is in LMEM or FAP0 */
#if defined(_BCM963268_)
                li      sp,BTRM_INT_MEM_END_ADDR
#elif defined(_BCM96838_) || defined(_BCM96848_) || defined(_BCM963381_)
		li      sp,BTRM_INT_MEM_STACK_PTR_ADDR
#endif
		li	a0,0			# call as "cfe_main(0,0)"
		li	a1,0

		CALLINIT_RELOC(init_table,R_INIT_CMDSTART)  # should not return

/*  *********************************************************************
    *  CFE_LAUNCH
    *  
    *  Start the user program.  The program is passed a handle
    *  that must be passed back when calling the firmware.
    *
    *  Parameters passed to the called program are as follows:
    *
    *      a0 - CFE handle (if 0, authentication failed and we will spin forever)
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

		# Push content of a0 onto the stack    
		sub	sp,8
		SR	a0,0(sp)

	 	# This function runs in internal memory at the address it was 
		# compiled for, so BOOT_OFFSET is 0. But it is called from
	 	# C which could have modified the BOOT_OFFSET register, s6.
		li	BOOT_OFFSET, 0

	 	# Mask all interrupts.
		mfc0	v0,C0_SR		# Get current interrupt flag
		li	v1,M_SR_IE		# master interrupt control
		not	v1			# disable interrupts
		and	v0,v1			# SR now has IE=0
		mtc0	v0,C0_SR		# put back into CP0


#if defined(_BCM963268_) || defined(_BCM96838_) || defined(_BCM96848_) || defined(_BCM963381_)

        	.set noreorder

		# before we launch the next program, shred any existence of the BTRM program 
		# by creating a little shredder program in uncached physical internal memory 
        	li      t1, BTRM_INT_MEM_SHREDDER_PROG_ADDR      # cached addr in internal mem            
		add	t1, 0x20000000				 # make it uncached address

		# Load t2 and t3 with the beginning and end cached addresses of where the 
		# shredder program resides within the BTRM code
        	la      t2, __shredBegin
        	la      t3, __shredEnd

    	1: 	# Load shredder code to specified uncached memory area

        	lw      t4, 0(t2)
        	sw      t4, 0(t1)
        	add     t2, 4
        	add     t1, 4
        	bne     t2, t3, 1b
        	nop
        	nop

        	b       __shredEnd
        	nop
        	nop

__shredBegin:  	# beginning of actual shredder code that will be copied over 

	        # pop the entry vector off of the stack and into t0 register 
		# because the sp points to an area that will get shredded 
		LR	t0,0(sp)
	
		# Load up registers t5 and t6 with the beginning of internal memory, and where
		# the shredder program will be located, respectively, and have them point to the
		# cached memory locations. We are about to clean up any text, data, bss pertaining 
		# to the BTRM, and also any heap that was used. 
		li	t5,BTRM_INT_MEM_BEGIN_ADDR
#if defined(_BCM963268_)
		li	t6,BTRM_INT_MEM_SHREDDER_PROG_ADDR
#elif defined(_BCM96838_) || defined(_BCM96848_) || defined(_BCM963381_)
		li	t6,BTRM_INT_MEM_SBI_LINK_ADDR
                # Don't whack the image size, or whether this is an unsecure or secure boot 
		sub	t6, 16		
#endif

	1:      # clean this swath of cached memory
        	sw      zero,0(t5) 
        	sw      zero,4(t5)
        	sw      zero,8(t5)
        	sw      zero,12(t5)
        	add     t5,16
        	blt     t5,t6,1b
        	nop
        	nop

		# Load up registers t5 with where security credentials exist if authentication has failed, or
		# load it up the end of the padded 48k(nand)/128k(spi) CFE ROM if authentication has passed. Also, 
		# load up t6 with the end of utilized memory. Have both of these pointers point to the cached.
		# memory. 
                beq     t0,0,1f
		nop
		nop
#if defined(_BCM963268_)
		li	t5,BTRM_INT_MEM_CFE_ROM_END_ADDR
#elif defined(_BCM96838_) || defined(_BCM96848_) || defined(_BCM963381_)
		li	t5,BTRM_INT_MEM_SBI_LINK_ADDR
		sub	t5,4
		LR	t6,0(t5)	/* t6 now contains the length of the cfe rom or sbi */
		li	t5,BTRM_INT_MEM_SBI_LINK_ADDR
		add	t5,t5,t6

                add     t5,16
                and     t5,t5,0xfffffff0
#endif
		b	2f
		nop
		nop
	1:	
#if defined(_BCM963268_)
		li	t5,BTRM_INT_MEM_CREDENTIALS_ADDR
#elif defined(_BCM96838_) || defined(_BCM96848_) || defined(_BCM963381_)
		li	t5,BTRM_INT_MEM_SBI_LINK_ADDR
#endif
	2:	

		li	t6,BTRM_INT_MEM_END_ADDR

	1:      # clean this swath of cached memory
        	sw      zero,0(t5) 
        	sw      zero,4(t5)
        	sw      zero,8(t5)
        	sw      zero,12(t5)
        	add     t5,16
        	blt     t5,t6,1b
        	nop
        	nop
	
#if defined(_BCM96838_) || defined(_BCM96848_) || defined(_BCM963381_)
		# If auth failed, need to clean up 32k unmapped internal mem in which credentials reside 
                bne     t0,0,2f
        	nop
        	nop
		li	t5,BTRM_INT_MEM_CREDENTIALS_ADDR
		li	t6,BTRM_INT_MEM_32K_BLOCK_END_ADDR

	1:      # clean this swath of cached memory
        	sw      zero,0(t5) 
        	sw      zero,4(t5)
        	sw      zero,8(t5)
        	sw      zero,12(t5)
        	add     t5,16
        	blt     t5,t6,1b
        2:	nop
        	nop
#endif
		# If authentication passed, need to reload the CFE ROM entry point back into
		# where the sp is pointing
		SR	t0,0(sp)

	 	#    Flush the D-Cache, and invalidate the I-cache 
                #    so that physical uncached memory is cleaned and no
                #    traces of the bootrom code exists anywhere

                # Flush the data cache
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
        	bne     t0, t1, 1b
        	addu    t0, a2

        	cacheop(a0, a1, a2, Index_Writeback_Inv_D)

		# Invalidate the I-cache
		li      a0, K0BASE
        	size_icache(a1, a2)
        	cacheop(a0, a1, a2, Index_Invalidate_I)

                #    At this point, we are ready to launch the CFE ROM in cached memory if authentication passed,
                #    or unlock the JTAG interface (if it is suppose to be accessible) and loop forever because
                #    authentication failed. It all depends on what a0 was when cfe_launch was called ...
		LR	t0,0(sp)
                beq     t0, 0, __authFailed
		nop
		nop
	
#if defined(_BCM96838_) || defined(_BCM96848_) || defined(_BCM963381_)
		# Launching to a valid boot vector ... unlock JTAG if we are doing unsecure boot 
		li	t5,BTRM_INT_MEM_SBI_LINK_ADDR
		sub	t5,8
		LR	t6,0(t5)	/* t6 now contains whether this boot is unsecure or secure */
                beq     t6, 0, __bypassJtagUnlock
		nop
		nop
#if defined(_BCM96838_)
                li      t8, OTP_BASE
		lw      t9, OTP_OTP_SECURE_BOOT_CFG(t8)
                and     t9,~OTP_OTP_SECURE_BOOT_CFG_LOCK_JTAG
		sw      t9, OTP_OTP_SECURE_BOOT_CFG(t8)
#elif defined(_BCM963381_) || defined(_BCM96848_)
                li      t8, BROM_SEC_BASE
		lw      t9, BROM_SEC_SECBOOTCFG(t8)
                or      t9, BROM_SEC_SECBOOTCFG_JTAG_UNLOCK
		sw      t9, BROM_SEC_SECBOOTCFG(t8)
#endif
__bypassJtagUnlock:
		nop
		nop
#endif	

		move	a0,t0
		j	t0			# Launch CFE ROM code ... never to return .....
		nop
		nop

__authFailed:

		#    Set the external interface enable bit high so that if interfaces such as JTAG are 
		#    suppose to be accessible, they become enabled. This allows a board that has been bricked
		#    by bad flash software to be recovered.
#if defined(_BCM96838_) || defined(_BCM963268_)
                li      t8, OTP_BASE
		lw      t9, OTP_OTP_SECURE_BOOT_CFG(t8)
#if defined(_BCM963268_)
		or	t9, OTP_OTP_SECURE_BOOT_CFG_UNLOCK_JTAG
#elif defined(_BCM96838_)
                and     t9,~OTP_OTP_SECURE_BOOT_CFG_LOCK_JTAG
#endif
		sw      t9, OTP_OTP_SECURE_BOOT_CFG(t8)
#elif defined(_BCM963381_) || defined(_BCM96848_)
                li      t8, BROM_SEC_BASE
		lw      t9, BROM_SEC_SECBOOTCFG(t8)
                or      t9, BROM_SEC_SECBOOTCFG_JTAG_UNLOCK
		sw      t9, BROM_SEC_SECBOOTCFG(t8)
#endif	
__toast:
        	nop
        	nop
        	b       __toast                 # loop forever .... 
        	nop
        	nop

__shredEnd: 	# end of actual shredder code that will be copied over


		# Run the LMEM shredder program at the cached physical address
        	li      t1, BTRM_INT_MEM_SHREDDER_PROG_ADDR      # addr in internal mem            
		add	t1, 0x20000000

        	j       t1
                nop
	
		.set reorder

#endif


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


#if defined(_BCM96838_) || defined(_BCM963381_) || defined(_BCM96848_)
/*  *********************************************************************
    *  _GETTICKS()
    *  
    *  Read the COUNT register into v0
    *  
    *  Input parameters: 
    *      nothing
    *      
    *  Return value:
    *      v0 - count register
    ********************************************************************* */
LEAF(_getticks)

                mfc0    v0,C0_COUNT
                j       ra
END(_getticks)

#endif

/*  *********************************************************************
    *  End
    ********************************************************************* */



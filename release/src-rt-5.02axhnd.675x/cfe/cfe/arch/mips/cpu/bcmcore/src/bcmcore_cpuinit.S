/*  *********************************************************************
    *  SB1250 Board Support Package
    *  
    *  CPU initialization			File: bcmcore_cpuinit.S
    *  
    *  This module contains code to initialize the CPU cores.
    *  
    *  Note: all the routines in this module rely on registers only,
    *        since DRAM may not be active yet.
    *
    *  Author:  Mitch Lichtenberg (mpl@broadcom.com)
    *  
    *********************************************************************  
    *
    *  XX Copyright 2000,2001
    *  Broadcom Corporation. All rights reserved.
    *
    *  BROADCOM PROPRIETARY AND CONFIDENTIAL
    *  
    *  This software is furnished under license and may be used and 
    *  copied only in accordance with the license.
    ********************************************************************* */

#include "sbmips.h"
#include "exception.h"
#include "bsp_config.h"
#include "mipsmacros.h"
#include "cpu_config.h"			/* for ERET and HAZARD */
#include "bcm_map.h"
#include "cfe_iocb.h"
	
#ifndef INC_BTRM_BUILD
#define INC_BTRM_BUILD         0
#endif

		.text


/*  *********************************************************************
    *  Macros
    ********************************************************************* */

#define LINESIZE 16

#define CACHEOP(cachename,op) ((cachename) | ((op) << 2))

#define CACHE_OP_IDXINVAL     0
#define CACHE_OP_IDXLOADTAG   1
#define CACHE_OP_IDXSTORETAG  2
#define CACHE_OP_IMPLRSVD     3
#define CACHE_OP_HITINVAL     4
#define CACHE_OP_FILL         5
#define CACHE_OP_HITWRITEBACK_INVAL  5
#define CACHE_OP_HITWRITEBACK 6
#define CACHE_OP_FETCHLOCK    7

#define L2C		    3
#define L1C_I		    0
#define L1C_D		    1

#define BCMCORE_NTLBENTRIES	32



#define SENDCHAR(c) \
	li t0,0xBF800000 ; \
	li t1,c ; \
	sb t1,0(t0)


#define R_CPU_CP0INIT _TBLIDX(0)
#define R_CPU_L1CINIT _TBLIDX(1)
#define R_CPU_SETLEDS _TBLIDX(2)
#define R_CPU_L1CFLASHD _TBLIDX(3)
#define R_CPU_L1CINVALI _TBLIDX(4)

#define SETLEDS1(a,b,c,d)                     \
       li     a0,(((a)<<24)|((b)<<16)|((c)<<8)|(d)) ;    \
       CALLINIT_KSEG1(cpuinit_table,R_CPU_SETLEDS)
#define SETLEDS(a,b,c,d)                     \
       li     a0,(((a)<<24)|((b)<<16)|((c)<<8)|(d)) ;    \
       CALLINIT_KSEG0(cpuinit_table,R_CPU_SETLEDS)

cpuinit_table:
		_LONG_	bcmcore_cp0_init                    # [ 0 ] R_CPU_CP0INIT
#if (TRIM_SUP==1) || defined(IKOS_FAST_CACHE_INIT)
                _LONG_  InvalidateAndEnableIDcache          # [ 1 ] R_CPU_L1CINIT
#else
		_LONG_	bcmcore_l1cache_init                # [ 1 ] R_CPU_L1CINIT
#endif
		_LONG_  board_setleds                       # [ 2 ] R_CPU_SETLEDS
		_LONG_	bcmcore_l1cache_flush_d             # [ 3 ] R_CPU_L1CFLASHD
		_LONG_	bcmcore_l1cache_inval_i             # [ 4 ] R_CPU_L1CINVALI


/*  *********************************************************************
    *  BCMCORE_CP0_INIT()
    *  
    *  Initialize an BCMCORE CPU's CP0 registers
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    *  
    *  Registers used:
    *  	   all
    ********************************************************************* */

	
LEAF(bcmcore_cp0_init)

		.set	noreorder

		mtc0	zero,C0_WATCHLO		# Watch registers.
		mtc0	zero,C0_WATCHHI
		mtc0	zero,C0_CAUSE		# must clear before writing SR

		mfc0	v0,C0_SR		# Get status register
		and	v0,M_SR_SR		# preserve soft reset
#ifdef DEBUG_ENV_ICE
		and	v0,~M_SR_BEV
#else
		or	v0,M_SR_BEV		# exceptions to boot vector
#endif

#if (INC_BTRM_BUILD==0)
		mtc0	v0,C0_SR		# set up the status register
#endif

		mfc0	v0,C0_CONFIG		# get current CONFIG register
		srl	v0,v0,3			# strip out K0 bits
		sll	v0,v0,3			# k0 bits now zero
		or	v0,v0,K_CFG_K0COH_CACHEABLE # K0 is cacheable.
		mtc0	v0,C0_CONFIG
		nop

		mtc0	zero,C0_WATCHLO		# Watch registers.
		mtc0	zero,C0_WATCHHI

		mtc0	zero,C0_TLBHI		# TLB entry (high half)
		nop


	#
	# This is probably not the right init value for C0_COMPARE,
	# but it seems to be necessary for the sim model right now.
	#

		li	v0,-1
		mtc0	v0,C0_COMPARE
		nop

	#
	# Initialize all the TLB entries to some invalid value
	#

		mtc0	zero,C0_TLBLO0		/* tlblo0 = invalid */
		nop
		mtc0	zero,C0_TLBLO1		/* tlblo1 = invalid */
		nop
		mtc0	zero,C0_PGMASK		/* 4K pages */
		nop

		li	t0,K1BASE		/* tlbhi  = impossible vpn */
		li	t1,(BCMCORE_NTLBENTRIES-1)	/* index */
	

		nop
1:		mtc0	t0,C0_TLBHI
		nop
		mtc0	t1,C0_INX
		nop
		addu	t0,0x2000		/* inc vpn */
		tlbwi
		bnez	t1,1b
		subu	t1,1			# BDSLOT
		.set reorder


/*
 * XXX What other CP0 initialization do I need?  
 */

		jr	ra


END(bcmcore_cp0_init)


/*  *********************************************************************
    *  BCMCORE_CPUINIT
    *  
    *  Do initialization of the Broadcom core
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */


LEAF(bcmcore_cpuinit)

		move	fp,ra

		SETLEDS1('C','P','U','I')
		CALLINIT_KSEG1(cpuinit_table,R_CPU_CP0INIT)

		SETLEDS1('L','1','C','I')
		CALLINIT_KSEG1(cpuinit_table,R_CPU_L1CINIT)

		move	ra,fp
		j	ra

END(bcmcore_cpuinit)


/*  *********************************************************************
    *  BCMCORE_KSEG0_SWITCH
    *  
    *  Return to the address of the routine that called us, except
    *  in K0seg instead of K1seg
    *  
    *  Input parameters: 
    *  	   nothing - ra is return address
    *  	   
    *  Return value:
    *  	   ra = same return address in K0
    ********************************************************************* */

LEAF(bcmcore_kseg0_switch)

		and	ra,(K0SIZE-1)
		or	ra,K0BASE
		jr	ra

END(bcmcore_kseg0_switch)

/*  *********************************************************************
    *  BCMCORE_NULL
    *  
    *  Dummy handler for routines we don't need to implement, like
    *  the multiprocessor stuff
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    *  	   
    *  Registers used:
    *  	   none
    ********************************************************************* */

LEAF(bcmcore_null)

		j	ra

END(bcmcore_null)


/*  *********************************************************************
    *  BCMCORE_CPURESTART
    *  
    *  This routine is called when someone soft-exits to CFE.  We
    *  reinitialize any CP0 stuff here.
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */

LEAF(bcmcore_cpurestart)

		j	ra

END(bcmcore_cpurestart)


/*  *********************************************************************
    *  BCMCORE_CACHEOPS
    *  
    *  Perform various cache operations on a BCM Core
    *  
    *  Input parameters: 
    *  	   a0 - flag bits (CFE_CACHE_xxx)
    *  	   
    *  Return value:
    *  	   nothing
    *  	   
    *  Registers used:
    *  	   t0,t1,t2,t3,v1,s0
    ********************************************************************* */

LEAF(bcmcore_cacheops)

		move	s0,ra

		move	v1,a0

	/*
	 * With no flags, we flush L1D and invalid L1I
	 */

		bne	v1,zero,1f
		li	v1,CFE_CACHE_FLUSH_D | CFE_CACHE_INVAL_I
1:

	/*
	 * Flush the D-Cache, since the program we loaded is "data".
	 */

		and	a0,v1,CFE_CACHE_FLUSH_D
		beq	a0,zero,1f
		CALLINIT_KSEG0(cpuinit_table,R_CPU_L1CFLASHD)
1:

	/*
	 * Invalidate the I-Cache, so that addresses in the program
	 * region will miss and need to be filled from the data we 
	 * just flushed above.
	 */

		and	a0,v1,CFE_CACHE_INVAL_I
		beq	a0,zero,1f
		CALLINIT_KSEG0(cpuinit_table,R_CPU_L1CINVALI)
1:


		.set push
		.set mips32

	/*
	 * Invalidate cache range
	 */

		and	a0,v1,CFE_CACHE_INVAL_RANGE
		beq	a0,zero,2f

		move	t0,a1
1:		cache	CACHEOP(L1C_D,CACHE_OP_HITINVAL),0(t0)
		add	t0,LINESIZE
		blt	t0,a2,1b

	/*
	 * Flush cache range
	 */
		

2:
		and	a0,v1,CFE_CACHE_FLUSH_RANGE
		beq	a0,zero,2f

		move	t0,a1
1:		cache	CACHEOP(L1C_D,CACHE_OP_HITWRITEBACK_INVAL),0(t0)
		add	t0,LINESIZE
		blt	t0,a2,1b

2:

		.set pop


		move	ra,s0
		j	ra

END(bcmcore_cacheops)



/*  *********************************************************************
    *  BCMCORE_TLBHANDLER
    *  
    *  This is the TLB exception handler for the bcmcore
    *  
    *  Note: only K0 and K1 are available to us at this time.
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   nothing
    ********************************************************************* */


LEAF(bcmcore_tlbhandler)
                .set    noreorder
                .set    noat

/*
 * XXX XXX XXX XXX XXX
 *
 * This won't work as-is on the BCMCORE.  The CONTEXT register is not
 * wide enough!  In fact, it's broken on all pre-mips4 CPUs.
 *
 * XXX XXX XXX XXX XXX
 */

/*
 * This requires a bit of explanation:  We only support 256KB
 * of mapped space for the boot program.  This space will be 
 * mapped from 0x2000_0000 to 0x2004_0000 to some physical 
 * memory allocated by the firmware.  This is 64 pages
 * of 4KB each.
 *
 * We know our BadVPN2 will be in the range
 * 0x100000 to 0x1001F0, since the memory is mapped from
 * 0x2000_0000 to 0x2004_0000.  BadVPN2 plus the four bits
 * of zeroes at the end are bits 31..9
 * 
 * We also want to place the PTEbase on something other than
 * a 16MB boundary.  Each entry is 16 bytes, and there
 * are 64 entries, so we need only 10 bits to address
 * the entire table (it can therefore be aligned on a
 * 1KB boundary).
 *
 * To make this work, we'll shift PTEbase to the right, leaving
 * the bottom ten bits for the page number, as:
 *
 *    Bits 31..10: PTEbase
 *    Bits 9..4:   BadVPN
 *    Bits 3..0:   16 bytes for table entry
 *
 * Therefore:
 *    PTEbase gets shifted right 13 bits.
 *    BadVPN  gets masked at 6 bits (mask is 0x3F0)
 *    The bottom 4 bits are zero.
 *
 * To range check the address, we can shift the Bad VPN
 * right by 9 bits, and check for values of 0x1000 and
 * 0x1001.
 */


	/*
	 * This part range checks the VPN2 field in the 
	 * context register.  We only handle
	 * VPN2s in the range 0x100000 to 0x1001F0
	 */
		mfc0	k0,C0_TLBHI

		mfc0	k0,C0_CTEXT		# Get context
		sra	k0,8			# keep hi part
		and	k0,0x1FFF		# of VPN2
		li	k1,0x1000		# 0x1000 is ok
		beq	k0,k1,1f		#
		nop				# BDSLOT
		li	k1,0x1001		# 0x1001 is ok
		beq	k0,k1,1f		#
		nop				# BDSLOT

		li	k0,XTYPE_TLBFILL	# all other bits are not
#		j	_exc_entry
		nop				# BDSLOT

1:		mfc0	k0,C0_CTEXT		# Get context
		sra	k0,13			# Shift PTEbase 
		li	k1,0x3FF		# Generate mask to kill 
		not	k1			# BadVPN2 bits
		and	k0,k1			# keep only PTEBase part.

		mfc0	k1,C0_CTEXT		# Get Context
		and	k1,0x3F0		# Keep only BadVPN2 bits
		or	k1,k0			# Replace PTEBase

		ld	k0,0(k1)		# Load entrylo0
		ld	k1,8(k1)		# Load entrylo1
		mtc0	k0,C0_TLBLO0		# and write to CP0
		mtc0	k1,C0_TLBLO1
		tlbwr				# put it in the TLB
		ERET
		nop

		.set	reorder
		.set	at

END(bcmcore_tlbhandler)

/*   ********************************************************************* */
LEAF(i_d_cache_init)
#if (CFG_COPY_PSRAM==1)	
	.set	noreorder

	move s7, ra
	
	/**----- Enable I Cache -----------------------------------------**/
		mfc0	t1, C0_BCM_CONFIG
		or	t1, (CP0_BCM_CFG_ICSHEN | CP0_BCM_CFG_DCSHEN)
		mtc0	t1, C0_BCM_CONFIG		# Enable I Cache

		mfc0	v0,C0_CONFIG		# get current CONFIG register
		srl	v0,v0,3			# strip out K0 bits
		sll	v0,v0,3			# k0 bits now zero
		or	v0,v0,K_CFG_K0COH_CACHEABLE # K0 is cacheable.
		mtc0	v0,C0_CONFIG
		nop

	/**----- Enable NBK and CLF -----------------------------------------**/
		mfc0	t1, C0_BCM_CONFIG
		or	t1, CP0_BCM_CFG_NBK
		or	t1, CP0_BCM_CFG_CLF
		mtc0	t1, C0_BCM_CONFIG

		.set reorder

		move ra, s7
#endif
		j	ra

END(i_d_cache_init)

/*  *********************************************************************
    *  End
    ********************************************************************* */


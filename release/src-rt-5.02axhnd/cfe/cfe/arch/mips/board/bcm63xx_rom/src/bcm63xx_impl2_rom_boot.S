#include "sbmips.h"
#include "bsp_config.h"

#include "bcm_cpu.h"
#include "bcm_map.h"
#include "bcm_hwdefs.h"
#include "boardparms.h"
#include "mipsmacros.h"
#include "bcm63xx_rom.h"

#include "segtable.h"

#if defined(CFE_ABORT_KEY)
#define PRINT_STR(s)			\
		b	2f ;		\
		nop ;			\
1:		.string	s ;		\
		.align	2 ;		\
2:		LOADREL(a0,1b) ;	\
		bal	board_puts ;
#endif

/*  *********************************************************************
	*  BOARD_EARLYINIT()
	*
	*  Initialize board registers.  This is the earliest
	*  time the BSP gets control.  This routine cannot assume that
	*  memory is operational, and therefore all code in this routine
	*  must run from registers only.  The $ra register must not
	*  be modified, as it contains the return address.
	*
	*  This routine will be called from uncached space, before
	*  the caches are initialized.  If you want to make
	*  subroutine calls from here, you must use the CALLKSEG1 macro.
	*
	*  Among other things, this is where the GPIO registers get
	*  programmed to make on-board LEDs function, or other startup
	*  that has to be done before anything will work.
	*
	*  Input parameters:
	*  		a0 - Flash base address (address of MIPS reset)
	*
	*  Return value:
	*  		nothing
	********************************************************************* */
#ifndef CFG_BOOTUTILS
LEAF(board_earlyinit)

		.set	noreorder
		mfc0	t1, C0_BCM_CONFIG, 3
		li	t2, CP0_CMT_TPID
		and	t1, t2
		bnez	t1, 2f					# if we are running on thread 1, skip init
		nop

#if (CFG_BOOT_PSRAM==1) || (CFG_COPY_PSRAM==1)
		li	t1, 0
		mtc0	t1, C0_CAUSE
		mtc0	t1, C0_STATUS
#endif
	
#if 0
		/* wait for a while to allow catch by jtag debugger */
		li	t8, -(200000000*3)		/* we will count up to 0 to delay a couple of seconds */
							/* and give the emulator a chance to catch us */
		mtc0	t8, C0_COUNT
catchloop:
		bltz	t8, catchloop
		mfc0	t8, C0_COUNT
#endif

#if defined(_BCM96838_) 
        	/* Check to see whether the internal bootrom previously ran */
                move    t8, ra
                bal     otp_is_btrm_enabled
                nop
                move    ra, t8
                beq     v0,0,1f
                nop
		/* Bootrom previously ran. 6838 use HW IRQ1 and IRQ2 for external hw interrupt instead of the regular IRQ0 
                * and IRQ1 in other MIPS chip. 6838 bootrom accidentally set the irq steering same as the regular MIPS chip:
		* 6838 bootrom accidentally set the irq steering same as the regular MIPS chip: 
                * IRQ0 -> TP0, IRQ1 -> TP1. Fix the steering to IRQ1 -> TP0, IRQ2 -> TP1 to make the interrupt work in linux.
		*/
		mfc0	t1, C0_BCM_CONFIG, 1
		and	t1, ~CP0_CMT_XIR_1
		or	t1, CP0_CMT_XIR_2
		mtc0	t1, C0_BCM_CONFIG, 1
1:              nop
#endif	
		/**--------------------------------------------------------------**/
		/** platform specific code										**/
		/**--------------------------------------------------------------**/
		/**----- Enable I Cache -----------------------------------------**/
		mfc0	t1, C0_BCM_CONFIG
		or	t1, (CP0_BCM_CFG_ICSHEN | CP0_BCM_CFG_DCSHEN)
		mtc0	t1, C0_BCM_CONFIG		# Enable I Cache

		// In the begining MIPS core registers are mapped to 0xbfax_xxxx
		li	t1, 0x1FA0000C			# Set up CBR to 1FAx_xxxx
		mtc0	t1, C0_BCM_CONFIG, 6

		li	t1, MIPS_BASE_BOOT
		lw	t2, MIPS_LMB_CR(t1)
		or	t2, 0xC0000000			# enable ffxx_xxxx space
		sw	t2, MIPS_LMB_CR(t1)
		li	t2, 0xFFF80001			# SBR FFF8_xxxx and enable
		sw	t2, MIPS_SBR(t1)

		#hermanl added a bunch of nops to make sure that the CBR does not change while the last write
		# to the control register is going on
		li	t3,0xDEADDEAD 	
		li	t0,0xCAFEF00D 	
		or	t0,v1,0xBAD0F00D
	
		// Now map MIPS core registers to 0xFF4x_xxxx space
		li	t1, 0xFF40000C			# CBR FF4x_xxxx (and reserved bits 0xc).
		mtc0	t1, C0_BCM_CONFIG, 6

	
		/**----- Initialize Serial --------------------------------------**/
		li		t3, ((FPERIPH / 115200) / 16)
		/*
		#  Baudword = (FPeriph)/Baud/32-1.  We have to perform rounding
		#  and subtraction.  Above we divided by 16 (instead of 32).  If
		#  bit0 is set, we round up.  However, we then subtract 1, so final
		#  result should be t3/2.  If bit0 is 0, then we truncate and subtract
		#  1, t3=t3/2-1.
		*/
		andi	t0, t3, 0x1
		bne	t0,zero,1f			# do shift only (in delay slot)
							# and jump to apply
		srl	t3,1				# do divide by 2
		addiu	t3, -1				# subtract 1
1:

		# t3 contains the UART BAUDWORD
		li	t0, UART_BASE
		sw	t3, UART0BAUD(t0)		# Store BaudRate
		li	t1, BITS8SYM|ONESTOP
		sb	t1, UART0CONFIG(t0)		# 8 Bits/1 Stop
		li	t1, TXEN|RXEN|BRGEN
		sb	t1, UART0CONTROL(t0)		# Enable, No Parity
		move	t1, zero
		sh	t1, UART0INTMASK(t0)

		.set	reorder
2:
		j		ra
END(board_earlyinit)

/*  *********************************************************************
*  BOARD_DRAMINFO
*
*  Return the address of the DRAM information table
*
*  Input parameters:
*  		nothing
*
*  Return value:
*  		v0 - DRAM info table, return 0 to use default table
********************************************************************* */
LEAF(board_draminfo)
		j	ra
END(board_draminfo)
#endif

#ifndef CFG_BOOTUTILS
/*  *********************************************************************
*  BOARD_SETLEDS(x)
*
*  Set LEDs for boot-time progress indication.  Not used if
*  the board does not have progress LEDs.  This routine
*  must not call any other routines, since it may be invoked
*  either from KSEG0 or KSEG1 and it may be invoked
*  whether or not the icache is operational.
*
*  Input parameters:
*  		a0 - LED value (8 bits per character, 4 characters)
*
*  Return value:
*  		nothing
*
*  Registers used:
*  		t0,t1,t2,t3
********************************************************************* */
LEAF(board_setleds)
		li	t0, UART_BASE
		li	t2, TXFIFOEMT

1:		lh	t1, UART0INTSTAT(t0)
		and	t1, t2
		bne	t1, t2, 1b

		srl	t3, a0, 24
		sb	t3, UART0DATA(t0)
		srl	t3, a0, 16
		sb	t3, UART0DATA(t0)
		srl	t3, a0, 8
		sb	t3, UART0DATA(t0)
		sb	a0, UART0DATA(t0)
		li	a0, '\r'
		sb	a0, UART0DATA(t0)
		li	a0, '\n'
		sb	a0, UART0DATA(t0)
		j	ra
END(board_setleds)

LEAF(board_puts)
1:		li	t0, UART_BASE /* wait for room in uart */
		li	t1, TXFIFOEMT
		lh	t2, UART0INTSTAT(t0)
		and	t2, t1
		bne	t2, t1, 1b
		nop
		lbu	t1, 0(a0)
		beqz	t1, 2f /* null ... done */
		nop
		sb	t1, UART0DATA(t0)
		addi	a0, 1
		b	1b /* go to next char */
		nop
2:
		li	v0, 0	
		j	ra
END(board_puts)

#if defined(CFE_ABORT_KEY)
/*  *********************************************************************
*  BOARD_GETC(x)
*
*  Get one character from uart0
*
*  Input parameters:
*  		None
*
*  Return value:
*  		v0 - Input chracter value (8 bit).
*
*  Registers used:
*  		t7, t8, t9
********************************************************************* */
LEAF(board_getc)
1:		li	t7, UART_BASE /* check if rx fifo is not empty */
		li	t8, RXFIFONE
		lh	t9, UART0INTSTAT(t7)
		and	t9, t8
		bne	t9, t8, 2f
		nop
		lb	v0, UART0DATA(t7)
2:		j	ra
END(board_getc)
#endif

/*  *********************************************************************
*  BCMCORE_TP1_SWITCH()
*
*  Check if the thread switch is required. If we are already
*  running on thread 1 this function will do nothing and just return
*  If we are running on thread 0 this function will take thread 1
*  out of reset and put thread 0 to sleep waiting for singnal from
*  thread 1.
*
*  Input parameters:
*  		nothing
*
*  Return value:
*  		nothing
********************************************************************* */
LEAF(bcmcore_tp1_switch)

#if (CFG_BOOT_PSRAM==0)
		mfc0	t1, C0_BCM_CONFIG, 3
		li	t2, CP0_CMT_TPID
		and	t1, t2
		bnez	t1, tp1			# Already running on thread 1
		nop

#if  defined(_BCM96838_)
		/* read OTP bit to check if TP1 is enabled by HW */
		li	t1, OTP_BASE
		lw	t2, OTP_SHADOW_BRCM_BITS_0_31(t1)
		and	 t2, OTP_BRCM_TP1_ENABLE_MASK
		srl	t2, OTP_BRCM_TP1_ENABLE_SHIFT
		bnez	t2, tp0_only
		nop
#endif

# Start TP1
# Set boot address for TP1
		li	t1, PERF_BASE
#if (CFG_COPY_PSRAM==1)
		li	t2, FLASH_BASE | ENABLE_ALT_BV
#else
		li	t2, IMAGE_BASE | ENABLE_ALT_BV
#endif
		sw	t2, PERF_PG_ALT_BV(t1)

# Set a flag so we can wait for TP1 to catch up
		mfc0	t1, C0_BCM_CONFIG
		move	t2, t1				  # Save C0_BCM_CONFIG register
		and	t1, ~CP0_BCM_CFG_TLBPD  # Reset TLBPD bit
		mtc0	t1, C0_BCM_CONFIG      


# Take TP1 out of reset
		mfc0	t1, C0_BCM_CONFIG, 2
		or	t1, CP0_CMT_RSTSE
		mtc0	t1, C0_BCM_CONFIG, 2

		/* wait until second thread catches up with the first */
waittp1:
		mfc0	t0, C0_BCM_CONFIG
		and	t0, CP0_BCM_CFG_TLBPD
		beqz	t0, waittp1             # branch while TLBPD is reset
		nop
		mtc0	t2, C0_BCM_CONFIG       # Restore C0_BCM_CONFIG register
		/* nvram copied to psram here already */
		li	t1, PSRAM_BASE		# destination address
		li	t0, THREAD_NUM_ADDRESS_OFFSET
		add	t1, t0
		lw	t0, 0(t1)
		li	t1, 1
		bne	t0, t1, return	# Linux will run on TP0, continue running bootloader

# Voice will run on TP0. Set it up and put it to sleep

		# enable interrupts and enable SW IRQ 0
		li	t0, M_SR_IE | M_SR_IBIT1
		mtc0	t0, C0_SR

		# Set up to use alternate exception vector 0x80000200
		li	t0, M_CAUSE_IV
		mtc0	t0, C0_CAUSE

		mfc0	t1, C0_BCM_CONFIG, 1
		# set all ints except IRQ1 to TP1 and cross over SW IRQ 0
#if  defined(_BCM96838_)
		or	t1, (CP0_CMT_XIR_4 | CP0_CMT_XIR_3 | CP0_CMT_XIR_1 | CP0_CMT_XIR_0 | CP0_CMT_SIR_0 | CP0_CMT_NMIR_TP1)
#else
		or	t1, (CP0_CMT_XIR_4 | CP0_CMT_XIR_3 | CP0_CMT_XIR_2 | CP0_CMT_XIR_0 | CP0_CMT_SIR_0 | CP0_CMT_NMIR_TP1)
#endif
		mtc0	t1, C0_BCM_CONFIG, 1

		mfc0	t1, C0_BCM_CONFIG, 2
		# Set debug on TP1 and set TLB exception serialization to ignore SCNT value in CP0 reg22 sel 4
		and	t1, ~CP0_CMT_TPS_MASK;
		or	t1, (CP0_CMT_DSU_TP1 | (1 << CP0_CMT_TPS_SHFT))
		mtc0	t1, C0_BCM_CONFIG, 2

2:
		b	wait_for_wake

tp1:
# Running on TP1....
# First signal to TP0 that TP1 is up
	
	 	mfc0	t1, C0_BCM_CONFIG
		or	t1, CP0_BCM_CFG_TLBPD   # Set TLBPD bit
		mtc0	t1, C0_BCM_CONFIG
		/* nvram copied to psram here already */
		li	t1, PSRAM_BASE		# destination address
		li	t0, THREAD_NUM_ADDRESS_OFFSET
		add	t1, t0
		lw	t0, 0(t1)
		li	t1, 1
		beq	t0, t1, return			# Linux will run on TP1, continue running bootloader

# Voice will run on TP1. Set it up and put it to sleep

		# enable interrupts and enable SW IRQ 0
		li	t0, M_SR_IE | M_SR_IBIT1
		mtc0	t0, C0_SR

		# Set up to use alternate exception vector 0x80000200
		li	t0, M_CAUSE_IV
		mtc0	t0, C0_CAUSE

		mfc0	t1, C0_BCM_CONFIG, 1
		# set IRQ1 to TP1 and cross over SW IRQ 0
#if  defined(_BCM96838_)
		or	t1, (CP0_CMT_XIR_2 | CP0_CMT_SIR_0 | CP0_CMT_NMIR_TP0)
#else
		or	t1, (CP0_CMT_XIR_1 | CP0_CMT_SIR_0 | CP0_CMT_NMIR_TP0)
#endif
		mtc0	t1, C0_BCM_CONFIG, 1

		mfc0	t1, C0_BCM_CONFIG, 2
		# Set debug on TP0 and set TLB exception serialization to ignore SCNT value in CP0 reg22 sel 4
		and	t1, ~CP0_CMT_TPS_MASK;
		or	t1, (1 << CP0_CMT_TPS_SHFT)
		mtc0	t1, C0_BCM_CONFIG, 2

		b	2b
		nop
		b	return
		nop

tp0_only:
		# set IRQ1 to TP1 and cross over SW IRQ 0
		mfc0	t1, C0_BCM_CONFIG, 1
		or	t1, (CP0_CMT_XIR_2 | CP0_CMT_SIR_0 | CP0_CMT_NMIR_TP0) 
		mtc0	t1, C0_BCM_CONFIG, 1

		# Set debug on TP0 and set TLB exception serialization to ignore SCNT value in CP0 reg22 sel 4
		mfc0	t1, C0_BCM_CONFIG, 2
		and	t1, ~CP0_CMT_TPS_MASK;
		or	t1, (1 << CP0_CMT_TPS_SHFT)
		mtc0	t1, C0_BCM_CONFIG, 2

return:
#endif
		j	ra

END(bcmcore_tp1_switch)


LEAF(board_init)
		### full speed mips clock and high performance bus mode
		mfc0	t0,C0_BCM_CONFIG , 0                      # Enable high performance mode
		li	t1, 0x00010000
		or	t0, t0, t1
		mtc0	t0, C0_BCM_CONFIG, 0

		mfc0	t0, C0_BCM_CONFIG, 5                      # clear Mips async mode
		li	t1, 0x8fffffff
		and	t0, t0, t1
		mtc0	t0, C0_BCM_CONFIG, 5
/**----- Enable RAC and LMB -------------------------------------**/
		li	t1, MIPS_BASE
		lw	t2, MIPS_LMB_CR(t1)
		or	t2, LMB_EN				# Enable LMB
		sw	t2, MIPS_LMB_CR(t1)

		li	t2, 0xFFF << RAC_UPB_SHFT		# Enable prefetch for RAM address range up to 256MB
		sw	t2, MIPS_RAC_ARR(t1)

		lw	t2, MIPS_RAC_CR0(t1)
		or	t2, (RAC_C_INV | RAC_I | RAC_PF_I)
		sw	t2, MIPS_RAC_CR0(t1)

		lw	t2, MIPS_RAC_CR1(t1)
		or	t2, (RAC_C_INV | RAC_I | RAC_PF_I)
		sw	t2, MIPS_RAC_CR1(t1)


		/**----- Enable branch prediction and non-blocking data cache ---**/
		mfc0	t1, C0_BCM_CONFIG
		and	t1, ~CP0_BCM_CFG_BTHD
		or	t1, CP0_BCM_CFG_NBK
		or	t1, CP0_BCM_CFG_CLF
		mtc0	t1, C0_BCM_CONFIG

#if  defined(_BCM96838_)
		/* For the C0_TLBLO0 and C0_TLBLO1 registers:				*/
		/* Bits 31 thru 26: must contain zeros					*/
		/* Bits 25 thru  6: Contains the PFN (physical frame number) which are	*/
		/*                  physical addr bits 31 thru 12 (4k page sizes)	*/
		/* Bits  5 thru  3: cache attribute of the cache lines in the 4k page	*/
		/*       000: Cacheable, Write-through, not allocated on write misses	*/
		/*       001: Cacheable, Write-through, allocated on write misses.	*/
		/*       010: Non-cacheable.						*/
		/*       011: Cacheable, Write-back.					*/
		/* Bit 2          : when set, allows writes to occur to this page	*/
		/* Bit 1          : when set, indicates this TLB entry is valid		*/
		/* Bit 0          : when set, do not do ASID comparison when looking	*/
		/*                  for a TLB entry match				*/

		li	t0, 0x20000							 /* VA: 0x0002_0000	*/
		li	t1,( 1 | 2 | 4 | (3 << 3)) | ((0x130c0000 >> 12) << 6)
		li	t2,( 1 | 2 | 4 | (3 << 3)) | ((0x130d0000 >> 12) << 6)
		li	t3, 0x1e000							/* 64k page size	*/
		li	t4, 1								/* TLB index		*/

		mtc0	t0, C0_TLBHI
		nop
		mtc0	t1, C0_TLBLO0
		nop
		mtc0	t2, C0_TLBLO1
		nop
		mtc0	t3, C0_PGMASK
		nop
		mtc0	t4, C0_INX
		nop
		nop

		tlbwi
		nop
		nop

		/* If full secure boot (ie not pre-secure), a contiguous swath of       */
		/* internal memory is required to relocate the signed, encrypted,       */
		/* compressed cfe ram into and perform authentication and decryption.   */
		/* If everything passes, the CFE RAM is decompressed out into DDR       */
		
		/* check to see if we are full secure boot                              */
		move	t7,ra
		bal 	otp_is_boot_secure
		nop
		move	ra,t7
                beq     v0,0,__not_secbt
                nop

        	li      t0, 0           /* VA: 0x0000_0000      */
        	li      t1,( 1 | 2 | 4 | (3 << 3)) | ((0x13010000 >> 12) << 6)
        	li      t2,( 1 | 2 | 4 | (3 << 3)) | ((0x13014000 >> 12) << 6)
        	li      t3, 0x6000      /* 16k page size        */
        	li      t4, 2           /* TLB index            */

        	mtc0    t0, C0_TLBHI
        	nop
        	mtc0    t1, C0_TLBLO0
        	nop
        	mtc0    t2, C0_TLBLO1
        	nop
        	mtc0    t3, C0_PGMASK
        	nop
        	mtc0    t4, C0_INX
        	nop
        	nop

        	tlbwi
        	nop
        	nop

        	li      t0, 0x8000      /* VA: 0x0000_8000      */
        	li      t1,( 1 | 2 | 4 | (3 << 3)) | ((0x13018000 >> 12) << 6)
        	li      t2,( 1 | 2 | 4 | (3 << 3)) | ((0x13050000 >> 12) << 6)
        	li      t3, 0x6000      /* 16k page size        */
        	li      t4, 3           /* TLB index            */

        	mtc0    t0, C0_TLBHI
        	nop
        	mtc0    t1, C0_TLBLO0
        	nop
        	mtc0    t2, C0_TLBLO1
        	nop
        	mtc0    t3, C0_PGMASK
        	nop
        	mtc0    t4, C0_INX
        	nop
        	nop

        	tlbwi
        	nop
        	nop

        	li      t0, 0x10000     /* VA: 0x0001_0000      */
        	li      t1,( 1 | 2 | 4 | (3 << 3)) | ((0x13054000 >> 12) << 6)
        	li      t2,( 1 | 2 | 4 | (3 << 3)) | ((0x13058000 >> 12) << 6)
        	li      t3, 0x6000      /* 16k page size        */
        	li      t4, 4           /* TLB index            */

        	mtc0    t0, C0_TLBHI
        	nop
        	mtc0    t1, C0_TLBLO0
        	nop
        	mtc0    t2, C0_TLBLO1
        	nop
        	mtc0    t3, C0_PGMASK
        	nop
        	mtc0    t4, C0_INX
        	nop
        	nop
	
        	tlbwi
        	nop
        	nop

        	li      t0, 0x18000     /* VA: 0x0001_8000      */
        	li      t1,( 1 | 2 | 4 | (3 << 3)) | ((0x13000000 >> 12) << 6)
        	li      t2,( 1 | 2 | 4 | (3 << 3)) | ((0x13004000 >> 12) << 6)
        	li      t3, 0x6000      /* 16k page size        */
        	li      t4, 5           /* TLB index            */

        	mtc0    t0, C0_TLBHI
        	nop
        	mtc0    t1, C0_TLBLO0
        	nop
        	mtc0    t2, C0_TLBLO1
        	nop
        	mtc0    t3, C0_PGMASK
        	nop
        	mtc0    t4, C0_INX
        	nop
        	nop

        	tlbwi
__not_secbt:    nop
        	nop

#endif

		j	ra
END(board_init)


# align this code to cache line. NAND flash is not memory mapped after system boots
# so when we are signaling to the second TP to wake we need
# jal instruction to be in cache
		.align  4
LEAF(wait_for_wake)
		sync
		wait			# wait for interrupt
		jal	t8	# jump to entry point
END(wait_for_wake)
#endif

#if defined(CFE_ABORT_KEY)
LEAF(set_romparm)
	move t0, ra
1:
	li   v0, 0
	CALLINIT_KSEG1(init_table,R_INIT_GETC)
	li   t1, 0x63     /* 'c' for continue */
	beq  v0, t1, 2f
	nop

	srl  v0, v0, 0x4
	li   t1, 0x3
	bne  v0, t1, 1b
	nop

	li   t1, 0x1
	sll  t1, t1, 29
	or ROMOPTION, t1
2:
	move	ra, t0
	j	ra
END(set_romparm)
#endif

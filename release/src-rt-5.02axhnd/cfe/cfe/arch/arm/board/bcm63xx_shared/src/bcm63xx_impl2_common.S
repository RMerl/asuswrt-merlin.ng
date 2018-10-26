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
#include "bcm_map.h"
#include "bcm_hwdefs.h"
#include "armmacros.h"
#include "bsp_config.h"

/*  *********************************************************************
    *  board_earlyinit_common()
    *
    *  Common early init code for btrm/cferom/cferam: uart init, uart rx/tx
    *  Called first before the board_earlyinit function
    *
    *  Input parameters:
    *       nothing
    *
    *  Return value:
    *       nothing
    ********************************************************************* */
FUNC(board_earlyinit_common)
	/**----- Initialize Serial --------------------------------------**/
#if defined (_BCM963158_)
	/* check chip rev id to see if it use brcm uart or arm uart */
	ldr	x0, =PERF_BASE
	ldr	w4, [x0, #PERF_CHIPID]
	and	w4, w4, #PERF_CHIPID_REV_ID_MASK
	lsr	w4, w4, #PERF_CHIPID_REV_ID_SHIFT

	cmp	w4, #0xa0
	beq	brcm_uart1
	/* 
	# (FPERIPH/4) #50Mhz clock for MAINUART CLK
	# Calc ibrd (integer part of the baudrate for 115200
	# Pls. follow Tech. Ref. manual for ARM uart
	# 50Mhz/(115200*16)=27.127
	# IBRD=27 FBRD=8 {integer((0.127*64)+0.5)=>8.628)=>8}
	# Generated divider will be: 27+8/64=27.125; Error estimate: 0.006%
	# IBRD = (FPERIPH/4)/(BAUD*16) = FPERIPH/BAUD/64 = (FPERIPH/BAUD)>>6
	# FBRD = (FPERIPH/BAUD) & 0x3f
	*/
	ldr	x0, =ARM_UART_BASE
#ifdef IKOS_FAST_UART	
	ldr	w1, =(FPERIPH/1562500)
#else
	ldr	w1, =(FPERIPH/115200)
#endif	
	lsr	w3, w1, #6
	str	w3, [x0, #UART0IBRD]
	and	w3, w1, #0x3f
	str	w3, [x0, #UART0FBRD]
         
	mov	w1, #LCR_8SYM_NOPAR_ONE_STOP_FIFOEN_CFG
	str	w1, [x0, #UART0LCR_H]     // 8 Bits/1 Stop

	mov	w3, #0
	str	w3, [x0, #UART0IMSC]

#if (INC_BTRM_BUILD==1)
	mov	w1, #CR_TXE
#else
	mov	w1, #(CR_RXE|CR_TXE)
#endif
	orr	w1, w1, #CR_EN
	str	w1, [x0, #UART0CR]    // Enable, No Parity

	b	uartinit_done
#endif
brcm_uart1:	
#ifdef IKOS_FAST_UART
	/* set to the maximum possible rate for simualtion: 50M/32=1562500 */
	ldr     w3, =0x0
#else
	ldr	w3, =((FPERIPH / 115200) / 16)
#endif
	/*
	#  Baudword = (FPeriph)/Baud/32-1.  We have to perform rounding
	#  and subtraction.  Above we divided by 16 (instead of 32).  If
	#  bit0 is set, we round up.  However, we then subtract 1, so final
	#  result should be t3/2.  If bit0 is 0, then we truncate and subtract
	#  1, t3=t3/2-1.
	*/
	and	w0, w3, #1
	lsr	w3, w3, #1
	cmp	w0, #0
	bne	1f
	sub	w3, w3, #1
1:
	// r3 contains the UART BAUDWORD
	ldr	x0, =UART_BASE
	str	w3, [x0, #UART0BAUD]
	mov	w1, #BITS8SYM|ONESTOP
	strb	w1, [x0, #UART0CONFIG]     // 8 Bits/1 Stop
#if (INC_BTRM_BUILD==1)
	mov	w1, #TXEN|BRGEN
#else
	mov	w1, #TXEN|RXEN|BRGEN
#endif
	strb	w1, [x0, #UART0CONTROL]    // Enable, No Parity
	mov	w1, #0
	strh	w1, [x0, #UART0INTMASK]

uartinit_done:
#if defined(_BCM96858_) || defined(_BCM96836_) || defined(_BCM963158_) || defined(_BCM96856_)
	/* set the system clock (enable timestamp0 counter) */
	ldr	x0, =BIUCFG_BASE
	mov	w1, #0x1
	str	w1, [x0, #TS0_CTRL_CNTCR]
	ldr	w1, [x0, #TS0_CTRL_CNTCR]
#endif	

#if !defined( _BOOTROM_) && !defined(CONFIG_BRCM_IKOS)
#if defined(_BCM963158_)
        /* we need to add some delay, so the early abort works */
	mov	x7,#0
7:
	add	x7, x7, #0x1
	tst	x7, #0xffff
	beq	5f
	b	7b
5:
#endif
#endif
	ret
END(board_earlyinit_common)

/*  *********************************************************************
    *  BOARD_SETLEDS(x)
    *
    *  Set LEDs for boot-time progress indication.  Not used if
    *  the board does not have progress LEDs.  This routine
    *  must not call any other routines, it may be invoked
    *  whether or not the icache is operational.
    *
    *  Input parameters:
    *	   w0 - LED value (8 bits per character, lower 4 characters)
    *      w1, w2, w3 used
    *  Return value:
    *	   nothing
    ********************************************************************* */
LEAF(board_setleds)
#if defined (_BCM963158_)
	/* check chip rev id to see if it use brcm uart or arm uart */
	ldr	x1, =PERF_BASE
	ldr	w3, [x1, #PERF_CHIPID]
	and	w3, w3, #PERF_CHIPID_REV_ID_MASK
	lsr	w3, w3, #PERF_CHIPID_REV_ID_SHIFT

	cmp	w3, #0xa0
	beq	brcm_uart2

	ldr	x1, =ARM_UART_BASE
#if (INC_BTRM_BUILD==1)
	mov	w2, #0
	ldr	w2, [x1, #UART0CR]
	and	w2, w2, #CR_TXE
	cmp	w2, #CR_TXE
	bne	uart_done1  /* branch if uart transmitter is turned off */
#endif
	and	x0, x0, #0xffffffffffff
	mov	x2, #0x0d0a
	lsl	x0, x0, #16
	orr	x0, x0, x2
1:
	ldr	w2, [x1, #UART0FR]
	and	w2, w2, #FR_TXFF
	cbnz	w2, 1b

	and	x2, x0, #0xff0000000000
	lsr	x2, x2, #40
	strb	w2, [x1, #UART0DR]
	lsl	x0, x0, #8
	and	x0, x0, #0xffffffffffff
	cmp	x0, #0
	bne	1b
	b	uart_done1
#endif

brcm_uart2:
	ldr	x1, =UART_BASE

#if (INC_BTRM_BUILD==1)
	mov	w2, #0
	ldrb	w2, [x1, #UART0CONTROL]
	and	w2, w2, #TXEN
	cmp	w2, #TXEN
	bne	uart_done1  /* branch if uart transmitter is turned off */
#endif
1:
	ldrh	w2, [x1, #UART0INTSTAT]
	tst	w2, #TXFIFOEMT
	beq	1b

	and	w2, w0, #0xff000000
	lsr	w2, w2, #24
	strb	w2, [x1, #UART0DATA]
	lsl	w0, w0, #8
	cmp	w0, #0
	bne	1b

	mov	w2, #'\r'
	strb	w2, [x1, #UART0DATA]
	mov	w2, #'\n'
	strb	w2, [x1, #UART0DATA]
uart_done1:
	ret
END(board_setleds)

/*  *********************************************************************
    * board_putc(char c)
    *
    *  send a char to periph UART.
    *
    *  Input parameters:
    *       w0 - character
    *       x1, x2, w3 used
    *  Return value:
    *       none
    ********************************************************************* */
FUNC(board_putc)
#if defined (_BCM963158_)
	/* check chip rev id to see if it use brcm uart or arm uart */
	ldr	x1, =PERF_BASE
	ldr	w3, [x1, #PERF_CHIPID]
	and	w3, w3, #PERF_CHIPID_REV_ID_MASK
	lsr	w3, w3, #PERF_CHIPID_REV_ID_SHIFT

	cmp	w3, #0xa0
	beq	brcm_uart3

	ldr	x1, =ARM_UART_BASE
1:
	ldr	w2, [x1, #UART0FR]
	and	w2, w2, #FR_TXFF
	cbnz	w2, 1b

	strb	w0, [x1, #UART0DR]
	ret
#endif
brcm_uart3:

	ldr	x1, =UART_BASE
1:
	ldrh	w2, [x1, #UART0INTSTAT]
	tst	w2, #TXFIFOEMT
	beq	1b

	strb	w0, [x1, #UART0DATA]

	ret
END(board_putc)

/*  *********************************************************************
    * board_puts(char* str)
    *
    *  send a null terminated string to periph UART.
    *
    *  Input parameters:
    *       x0 - pointer to the string
    *       w1, w2, w3, x4 used
    *  Return value:
    *       x0 - always zero
    ********************************************************************* */
FUNC(board_puts)
#if defined (_BCM963158_)
	/* check chip rev id to see if it use brcm uart or arm uart */
	ldr	x1, =PERF_BASE
	ldr	w3, [x1, #PERF_CHIPID]
	and	w3, w3, #PERF_CHIPID_REV_ID_MASK
	lsr	w3, w3, #PERF_CHIPID_REV_ID_SHIFT

	ldr	x1, =UART_BASE
	add	x4, x1, #UART0DATA
	cmp	w3, #0xa0
	beq	puts_start

	ldr	x1, =ARM_UART_BASE
	add	x4, x1, #UART0DR
#else
	ldr	x1, =UART_BASE
	add	x4, x1, #UART0DATA
	mov	w3, #0xa0
#endif

puts_start:
#if defined (_BCM963158_)
	cmp	w3, #0xa0
	beq	brcm_uart4
1:
	ldr	w2, [x1, #UART0FR]
	and	w2, w2, #FR_TXFF
	cbnz	w2, 1b
	b	txchar
brcm_uart4:
#endif
1:
	ldrh	w2, [x1, #UART0INTSTAT]
	tst	w2, #TXFIFOEMT
	beq	1b

txchar:
	ldrb	w2, [x0]
	cmp	w2, #0
	beq	3f
	cmp	w2, #10              /* If we aren't '\n' just print */
	bne	2f
	strb	w2, [x4]             /* Otherwise, write the '\n' */ 
	mov	w2, #13              /* Then write '\r' */
2:
	strb	w2, [x4]
	add	x0, x0, #1
	b	puts_start
3:
	mov	x0, #0
	ret
END(board_puts)

/*  *********************************************************************
    * int board_getc(void)
    *
    *  get a character from UART
    *
    *  Input parameters:
    *       none
    *  Return value:
    *       w0 zero if there is no character or character value
    ********************************************************************* */
FUNC(board_getc)
	mov     w0, #0
#if defined (_BCM963158_)
	/* check chip rev id to see if it use brcm uart or arm uart */
	ldr	x1, =PERF_BASE
	ldr	w3, [x1, #PERF_CHIPID]
	and	w3, w3, #PERF_CHIPID_REV_ID_MASK
	lsr	w3, w3, #PERF_CHIPID_REV_ID_SHIFT

	cmp	w3, #0xa0
	beq	brcm_uart5

	ldr	x1, =ARM_UART_BASE
	ldr	w2, [x1, #UART0FR]
	and	w2, w2, #FR_RXFE
	cbnz	w2, 1f

	ldrb	w0, [x1, #UART0DR]
1:
	ret
#endif
brcm_uart5:
	ldr	x1, =UART_BASE /* check if rx fifo is not empty */
	ldrh	w2, [x1, #UART0INTSTAT]
	tst	w2, #RXFIFONE
	beq	1f

	ldrb	w0, [x1, #UART0DATA]
1:
	ret
END(board_getc)

/*  *********************************************************************
    * int board_stsc(void)
    *
    *  check for rx uart fifo
    *
    *  Input parameters:
    *       none
    *  Return value:
    *       w0 zero if there is no character or one if there is character
    ********************************************************************* */
FUNC(board_stsc)
	mov	w0, #0
#if defined (_BCM963158_)
	/* check chip rev id to see if it use brcm uart or arm uart */
	ldr	x1, =PERF_BASE
	ldr	w3, [x1, #PERF_CHIPID]
	and	w3, w3, #PERF_CHIPID_REV_ID_MASK
	lsr	w3, w3, #PERF_CHIPID_REV_ID_SHIFT

	cmp	w3, #0xa0
	beq	brcm_uart6

	ldr	x1, =ARM_UART_BASE
	ldr	w2, [x1, #UART0FR]
	and	w2, w2, #FR_RXFE
	cbnz	w2, 1f

	mov	w0, #1
1:
	ret
#endif
brcm_uart6:
	ldr	x1, =UART_BASE /* check if rx fifo is not empty */
	ldrh	w2, [x1, #UART0INTSTAT]
	tst	w2, #RXFIFONE
	beq	1f

	mov	w0, #1
1:
	ret
END(board_stsc)

/*  *********************************************************************
    *  board_startup_secondary
    *
    *  power on secondary cores
    *
    *  Input parameters:
    *       x0 - entry point
    *
    *  Return value:
    *       nothing
    ********************************************************************* */
FUNC(board_startup_secondary)

#if defined(_BCM94908_)
	/* update the LUT table to set the reset vector to relocated entry ptr in x0.
	   Only lower 32 bit is needed for physical address of reset vector */
	mov	x1, x0
	ldr	x0, =BOOTLUT_BASE
	ldr	w2, [x0, #BOOT_LUT_RST]
	str	w1, [x0, #BOOT_LUT_RST]

	/* set default cpu bitmap in w1 */
	mov	w1, #0xe

	/* check chip id to see if it is dual core or quad core */
	ldr	x0, =PERF_BASE
	ldr	w3, [x0, #PERF_CHIPID]
	and	w3, w3, #PERF_CHIPID_CHIP_ID_MASK
	lsr	w3, w3, #PERF_CHIPID_CHIP_ID_SHIFT
	mov	w0, #0x4906
	cmp	w3, w0
	bne	pwr_on
	mov	w1, #0x2

pwr_on:
	/* power up the secondary cores */
	ldr	x0, =BIUCTRL_BASE
	ldr	w2, [x0,#CPU_POWER_CONFIG]
	lsl	w3, w1, #4
	orr	w2, w2, w3
	str	w2, [x0,#CPU_POWER_CONFIG]

	/* bring them out of reset */
	ldr	w2, [x0,#CPU_RESET_CONFIG]
	mvn	w3, w1
	and	w2, w2, w3
	str	w2, [x0,#CPU_RESET_CONFIG]
#endif
	ret
END(board_startup_secondary)

/*  *********************************************************************
    *  BOARD_GET_DDR_SIZE()
    *
    *  Get DDR size from memc. Must be called after ddr is init'ed
    *  Input parameters:
    *	   nothing
    *  Return value:
    *      x0 - ddr size in bytes
    ********************************************************************* */
LEAF(board_get_ddr_size)
	ldr	x0, =MEMC_BASE
	mov	x1, xzr
#if defined(_BCM96836_) || defined(_BCM963158_) || defined(_BCM96856_)
	ldr	w1, [x0, MEMC_GLB_FSBL_STATE]
	and	w1, w1, #MEMC_GLB_FSBL_DRAM_SIZE_MASK
	lsr	w1, w1, #MEMC_GLB_FSBL_DRAM_SIZE_SHIFT
#else
	ldr	w1, [x0, MEMC_GLB_GCFG]
	and	w1, w1, #MEMC_GLB_GCFG_DRAM_SIZE1_MASK
#endif
	add	w1, w1, #20
	mov	x2, #1
	lsl	x0, x2, x1

	ret
END(board_get_ddr_size)

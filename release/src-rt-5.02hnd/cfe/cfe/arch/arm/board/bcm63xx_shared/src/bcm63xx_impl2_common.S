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
#include "bcm_common.h"
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
#ifdef IKOS_FAST_UART
	/* set to the maximum possible rate for simualtion: 50M/32=1562500 */
	ldr     w3, =0x0
#else
	ldr	w3, =((FPERIPH / 115200) / 16)
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
#endif
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
    *      w1, w2 used
    *  Return value:
    *	   nothing
    ********************************************************************* */
LEAF(board_setleds)
	ldr	x1, =UART_BASE

#if (INC_BTRM_BUILD==1)
        mov     w2, #0
	ldrb	w2, [x1, #UART0CONTROL]
	and	w2, w2, #TXEN
	cmp	w2, #TXEN
        bne	uart_done  /* branch if uart transmitter is turned off */
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
uart_done:
	ret
END(board_setleds)


/*  *********************************************************************
    * board_putc(char c)
    *
    *  send a char to periph UART.
    *
    *  Input parameters:
    *       w0 - character
    *       x1, x2 used
    *  Return value:
    *       none
    ********************************************************************* */
FUNC(board_putc)
	ldr  x1, =UART_BASE
1:
	ldrh w2, [x1, #UART0INTSTAT]
	tst  w2, #TXFIFOEMT
	beq  1b

	strb w0, [x1, #UART0DATA]

	ret
END(board_putc)

/*  *********************************************************************
    * board_puts(char* str)
    *
    *  send a null terminated string to periph UART.
    *
    *  Input parameters:
    *       x0 - pointer to the string
    *       w1, w2 used
    *  Return value:
    *       x0 - always zero
    ********************************************************************* */
FUNC(board_puts)
1:
	ldr  x1, =UART_BASE
	ldrh w2, [x1, #UART0INTSTAT]
	tst  w2, #TXFIFOEMT
	beq  1b

	ldrb w2, [x0]
	cmp  w2, #0
	beq  3f
	cmp  w2, #10              /* If we aren't '\n' just print */
	bne  2f
	strb w2, [x1, #UART0DATA]  /* Otherwise, write the '\n' */ 
	mov  w2, #13               /* Then write '\r' */
2:
	strb w2, [x1, #UART0DATA]
	add  x0, x0, #1
	b    1b
3:
	mov  x0, #0
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
	ldr     x1, =UART_BASE /* check if rx fifo is not empty */
	ldrh    w2, [x1, #UART0INTSTAT]
	tst     w2, #RXFIFONE
	beq     1f

	ldrb    w0, [x1, #UART0DATA]
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
	mov     w0, #0
	ldr     x1, =UART_BASE /* check if rx fifo is not empty */
	ldrh    w2, [x1, #UART0INTSTAT]
	tst     w2, #RXFIFONE
	beq     1f

	mov     w0, #1
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
	ldr	x0, =PERF_CHIPID
	ldr	w3, [x0]
	and	w3, w3, #PERF_CHIPID_CHIP_ID_MASK
	lsr	w3, w3, #PERF_CHIPID_CHIP_ID_SHIFT
	mov	w0, #0x4906
	cmp	w3, w0
	bne	pwr_on
	mov	w1, #0x2
	
pwr_on:	
	/* power up the secondary cores */
	ldr	x0, =CPUBIUCTRL_BASE
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

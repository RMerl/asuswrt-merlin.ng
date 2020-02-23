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

#include <arm.h>
#include <armmacros.h>
#include "bcm_map.h"
#include "bcm_hwdefs.h"
#include <bcm_common.h>
#include "bsp_config.h"

FUNC(board_earlyinit_common)
	mov	r9,lr
	bl	uart_init

#if !defined( _BOOTROM_) && !defined(CONFIG_BRCM_IKOS)
	/* we need to add some delay, so the early abort works */
#if defined(_BCM96878_)
    ldr r8, =0x10000
#else
	ldr	r8, =0x1000000
#endif
	mov	r7,#0
7:
	add	r7, r7, #0x1
	cmp	r7, r8
	beq	5f
	b	7b
5:
#endif
	mov	lr,r9
	mov	pc,lr
END(board_earlyinit_common)

LEAF(uart_init)
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
	ldr	r0, =ARM_UART_BASE
	
	ldr	r1, [r0, #UART0CR]   // disable uart first
	bic	r1, r1, #CR_EN
	str	r1, [r0, #UART0CR]

	ldr	r1, =(FPERIPH/CFG_SERIAL_BAUD_RATE)
	lsr	r3, r1, #6
	str	r3, [r0, #UART0IBRD]
	and	r3, r1, #0x3f
	str	r3, [r0, #UART0FBRD]
	
	mov	r1, #LCR_8SYM_NOPAR_ONE_STOP_FIFOEN_CFG
	str	r1, [r0, #UART0LCR_H]	// 8 Bits/1 Stop

	mov	r3, #0
	str	r3, [r0, #UART0IMSC]

	mov	r3, #0xffff
	str	r3, [r0, #UART0ICR]
	
#if (INC_BTRM_BUILD==1)
	mov	r1, #CR_TXE
#else
	mov	r1, #(CR_RXE|CR_TXE)
#endif
	orr	r1, r1, #CR_EN
	str	r1, [r0, #UART0CR]    // Enable, No Parity

	mov	pc, lr
END(uart_init)

LEAF(board_setleds)
	ldr	r1, =ARM_UART_BASE
	mov	r2, #0
	ldr	r2, [r1, #UART0CR]
	ldr	r3, =(CR_TXE|CR_EN)
	and	r2, r2, r3
	cmp	r2, r3
	# exit if uart transmitter is turned off 
	bne	uart_done
	# packing CR and NL
	mov	r3, #'\r'
	lsl	r3, #8
	orr	r3, r3,#'\n'
	lsl	r3, r3,#16
1:
	ldr	r2, [r1, #UART0FR]
	and	r2, r2, #FR_TXFF
	cmp	r2,#0
	bne	1b

	lsr	r2, r0, #24
	strb    r2, [r1, #UART0DR]
	lsl	r0, r0, #8
	mov	r2, #24
	orr	r0, r0, r3, lsr r2
	lsl	r3, r3, #8
	cmp	r0, #0
	bne	1b
uart_done:
	mov  pc, lr
END(board_setleds)

FUNC(board_putc)
	ldr	r1, =ARM_UART_BASE
	mov	r2, #0
	ldr	r2, [r1, #UART0CR]
	ldr	r3, =(CR_TXE|CR_EN)
	and	r2, r2, r3
	cmp	r2, r3
	# exit if uart transmitter is turned off 
	bne	2f
1:
	ldr	r2, [r1, #UART0FR]
	and	r2, r2, #FR_TXFF
	cmp	r2,#0
	bne	1b
	strb    r0, [r1, #UART0DR]
2:
	mov  pc, lr
END(board_putc)


/*  *********************************************************************
    * board_puts(char* str)
    *
    *  send a null terminated string to periph UART.
    *
    *  Input parameters:
    *       r0 - pointer to the string
    *       r1, r2, r3
    *  Return value:
    *       r0 - always zero
    ********************************************************************* */
FUNC(board_puts)
	ldr	r1, =ARM_UART_BASE
	mov	r2, #0
	ldr	r2, [r1, #UART0CR]
	ldr	r3, =(CR_TXE|CR_EN)
	and	r2, r2, r3
	cmp	r2, r3
	bne	3f
	# exit if uart transmitter is turned off 	
puts_start:	
1:
	ldr	r2, [r1, #UART0FR]
	and	r2, r2, #FR_TXFF
	cmp	r2, #0
	bne	1b

	ldrb	r2, [r0]
	cmp	r2, #0
	beq	3f
	cmp	r2, #10		    /* If we aren't '\n' just print */
	bne	2f
	strb	r2, [r1, #UART0DR]   /* Otherwise, write the '\n' */
1:
	ldr	r2, [r1, #UART0FR]
	and	r2, r2, #FR_TXFF
	cmp	r2, #0
	bne	1b

	mov	r2, #13		    /* Then write '\r' */
2:
	strb	r2, [r1, #UART0DR]
	add	r0, r0, #1
	b	puts_start
3:
	mov	r0, #0
	mov	pc, lr
END(board_puts)

FUNC(board_gets)
	mov  pc, lr
END(board_gets)

/*  *********************************************************************
    * int board_getc(void)
    *
    *  get a character from UART
    *
    *  Input parameters:
    *       none
    *  Return value:
    *       r0 zero if there is no character or character value
    ********************************************************************* */
FUNC(board_getc)
	mov	r0, #0
	ldr	r1, =ARM_UART_BASE
	ldr	r2, [r1, #UART0FR]
	and	r2, r2, #FR_RXFE
	cmp	r2, #0
	bne	1f
	ldrb	r0, [r1, #UART0DR]
1:
	mov	pc, lr
END(board_getc)

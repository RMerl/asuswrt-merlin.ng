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

#include "arm.h"
#include "bsp_config.h"

#include "bcm_cpu.h"
#include "bcm_map.h"
	
#include "bcm_hwdefs.h"
#include "boardparms.h"
#include "armmacros.h"

#ifndef INC_BTRM_BOOT
#define INC_BTRM_BOOT         0
#endif

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
#ifdef CFG_ARM_UART
	/* init UART, set to 3.125Mbps for simulation */
	ldr	r0,=UART2_BASE
	mov	r1,#1
	str	r1,[r0,#UART2INTBAUD]
	
	mov	r1,#0
	str	r1,[r0,#UART2FRACBAUD]

	mov	r1,#0x70
	str	r1,[r0,#UART2LINECTRL]

	ldr	r1,[r0,#UART2CTRL]
	orr	r1,r1, #0x300
	str	r1,[r0,#UART2CTRL]

	ldr	r1,[r0,#UART2CTRL]
	orr	r1,r1, #0x1
	str	r1,[r0,#UART2CTRL]
#else
        /**----- Initialize Serial --------------------------------------**/
#ifdef IKOS_FAST_UART
	/* set to the maximum possible rate for simualtion: 50M/32=1562500 */
	ldr     r3, =0x0
#else	
        ldr     r3, =((FPERIPH / 115200) / 16)
        /*
        #  Baudword = (FPeriph)/Baud/32-1.  We have to perform rounding
        #  and subtraction.  Above we divided by 16 (instead of 32).  If
        #  bit0 is set, we round up.  However, we then subtract 1, so final
        #  result should be t3/2.  If bit0 is 0, then we truncate and subtract
        #  1, t3=t3/2-1.
        */
        and     r0, r3, #1
	mov     r3, r3, lsr #1
	cmp     r0, #0
        bne     1f             
        sub     r3, r3, #1         
1:
#endif
        // r3 contains the UART BAUDWORD
        ldr     r0, =UART_BASE
        str     r3, [r0, #UART0BAUD]      
        mov     r1, #BITS8SYM|ONESTOP
        strb    r1, [r0, #UART0CONFIG]     // 8 Bits/1 Stop
        mov     r1, #TXEN|RXEN|BRGEN
        strb    r1, [r0, #UART0CONTROL]    // Enable, No Parity
        mov     r1, #0
        strh    r1, [r0, #UART0INTMASK]
#endif
	mov	pc,lr
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
    *  	   r0 - LED value (8 bits per character, 4 characters)
    *      r1, r2 used
    *  Return value:
    *  	   nothing
    ********************************************************************* */
LEAF(board_setleds)
#ifdef CFG_ARM_UART	
	ldr r1, =UART2_BASE
1:
	ldr r2, [r1, #4]
	tst r2, #0x20
	bne 1b

	and  r2, r0, #0xff000000
	lsr  r2, #24
	str  r2, [r1, #0]
	lsl  r0, #8
	cmp  r0, #0
	bne  1b

	mov  r2, #'\r'
	str  r2, [r1, #0]
	mov  r2, #'\n'
	str  r2, [r1, #0]
#else
	ldr  r1, =UART_BASE
#if 0
#if (INC_BTRM_BUILD==1)
        mov     r2, #0
        ldrb    r2, [r1, #UART0CONTROL]
        and     r2, r2, #TXEN
        cmp     r2, #TXEN
        beq     1f  /* branch if uart transmitter is turned off */
	mov	pc, lr
#endif
#endif

1:	
	ldrh r2, [r1, #UART0INTSTAT]
	tst  r2, #TXFIFOEMT
	beq  1b

	and  r2, r0, #0xff000000
	lsr  r2, #24
	strb r2, [r1, #UART0DATA]
	lsl  r0, #8
	cmp  r0, #0
	bne  1b

	mov  r2, #'\r'
	strb r2, [r1, #UART0DATA]
	mov  r2, #'\n'
	strb r2, [r1, #UART0DATA]
#endif
	mov	pc, lr			@ back to my caller
END(board_setleds)

/*  *********************************************************************
    * board_puts(char* str)
    *
    *  send a null terminated string to periph UART.
    *
    *  Input parameters:
    *       r0 - pointer to the string
    *       r1, r2 used
    *  Return value:
    *       r0 - always zero
    ********************************************************************* */
FUNC(board_puts)
	ldr  r1, =UART_BASE
1:	
	ldrh r2, [r1, #UART0INTSTAT]
	tst  r2, #TXFIFOEMT
	beq  1b

        ldrb r2, [r0]
	cmp  r2, #0
	beq  3f
	cmp  r2, #10              /* If we aren't '\n' just print */
	bne  2f     
	strb r2, [r1, #UART0DATA]  /* Otherwise, write the '\n' */ 
	mov  r2, #13               /* Then write '\r' */
2:
	strb r2, [r1, #UART0DATA]
	add  r0, #1
	b    1b
3:
	mov  r0, #0
	mov  pc, lr
END(board_puts)

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
	mov     r0, #0
	ldr     r1, =UART_BASE /* check if rx fifo is not empty */
	ldrh    r2, [r1, #UART0INTSTAT]
	tst     r2, #RXFIFONE
	beq     1f

        ldrb    r0, [r1, #UART0DATA]
1:	
	mov     pc, lr
END(board_getc)	

/*  *********************************************************************
    * int board_stsc(void)
    *
    *  check for rx uart fifo
    *
    *  Input parameters:
    *       none
    *  Return value:
    *       r0 zero if there is no character or one if there is character
    ********************************************************************* */
FUNC(board_stsc)
	mov     r0, #0
	ldr     r1, =UART_BASE /* check if rx fifo is not empty */
	ldrh    r2, [r1, #UART0INTSTAT]
	tst     r2, #RXFIFONE
	beq     1f

        mov     r0, #1
1:	
	mov     pc, lr
END(board_stsc)		

/*  *********************************************************************
    * board_putc(char c)
    *
    *  send a char to periph UART.
    *
    *  Input parameters:
    *       r0 - character
    *       r1, r2 used
    *  Return value:
    *       none
    ********************************************************************* */
FUNC(board_putc)
    ldr  r1, =UART_BASE
1:
    ldrh r2, [r1, #UART0INTSTAT]
    tst  r2, #TXFIFOEMT
    beq  1b

    strb r0, [r1, #UART0DATA]

    mov pc, lr
END(board_putc)

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
	/* Turn off UART clock before switching clock source */
	ldr	r0,=CC_CORECONTROL
	ldrb	r1, [r0]
	bic	r1, r1, #CC_CTRL_UART_CLK_EN
	strb	r1, [r0]
	/* Set the override bit so we don't divide it */
	ldrb	r1, [r0]
	orr	r1, r1, #CC_CTRL_UART_CLK_OVR
	strb	r1, [r0]

	ldrb	r1, [r0]
	orr	r1, r1, #CC_CTRL_UART_CLK_EN
	strb	r1, [r0]

	/* UART base address */
	ldr	r0,=UART_BASE
	/* LCR <- DLAB */
	mov	r1,#LCR_DLAB
	strb	r1, [r0, #UART_LCR]
	/* Baudrate configuration (115200 bps)*/
	mov	r1,#0x16
	strb	r1, [r0]
	mov	r1,#0
	strb	r1, [r0, #UART_IMR]
	/* LCR <- 8 bits per byte */
	mov	r1,#3
	strb	r1, [r0, #UART_LCR]

	/* MCR <- DTR | RTS | IENABLE */
	mov	r1,#MCR_DTR|MCR_RTS|MCR_IENABLE
	strb	r1, [r0, #UART_MCR]
	/* IER <- 0 */
	mov	r1,#0
	strb	r1, [r0, #UART_IMR]
	/* FIFO <- ENABLE */
	mov	r1,#FIFO_ENABLE
	strb	r1, [r0, #UART_FCR]

	/* Delay */
	mov	r3,#100
1:	subs	r3,r3,#1
	bne	1b

	/* FIFO <- ENABLE | RCV_RST | XMT_RST | TRIGGER_1 */
	mov	r1,#FIFO_ENABLE|FIFO_RCV_RST|FIFO_XMT_RST|FIFO_TRIGGER_1
	strb	r1, [r0, #UART_FCR]

	mov pc, lr
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
FUNC(board_setleds)
	ldr	r1, =UART_BASE

1:	ldrb	r2, [r1, #UART_LSR]
	tst	r2, #LSR_TXRDY
	beq	1b

	and	r2, r0, #0xff000000
	lsr	r2, #24
	strb	r2, [r1]
	lsl	r0, #8
	cmp	r0, #0
	bne	1b

	mov	r2, #'\r'
	strb	r2, [r1]

	mov	r2, #'\n'
	strb	r2, [r1]

	mov pc, lr
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

1:	ldrb r2, [r1, #UART_LSR]
	tst  r2, #LSR_TXRDY
	beq  1b

        ldrb r2, [r0]
	cmp  r2, #0
	beq  3f
	cmp  r2, #10              /* If we aren't '\n' just print */
	bne  2f     
	strb r2, [r1]             /* Otherwise, write the '\n' */ 
	mov  r2, #13              /* Then write '\r' */
2:
	strb r2, [r1]
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
END(board_stsc)		

/*  *********************************************************************
    * mem_size(void)
    *
    *  Calculates the size of the on-board SDRAM memory using a simple
    *  aliasing check up to a maximum of 256 MB.
    *
    *  Input parameters:
    *       none
    *  Return value:
    *       r0: memory size in bytes
    *  Clobbers: r1-r5
    ********************************************************************* */
FUNC(mem_size)
	mov	ip, lr		/* preserve link reg */

	mov	r3, #0x00000000
	ldr	r2, =0xaa55beef
	mov	r1, #0
	str	r1, [r3, #0x70]
	mov	r0, #0x100000	/* Initial check offset */
	mov	r5, #0x10000000	/* Loop up to this offset (256 MB) */
1:
	mov	r4, #0x00000000
	add	r4, r4, r0
	str	r2, [r4, #0x70]
	dsb
	ldr	r1, [r3, #0x70]
	cmp	r1, r2
	beq	2f

	lsl	r0, r0, #1
	cmp	r0, r5
	bne	1b

	/* Didn't find an alias, must be 128 MB */
2:
	mov	lr, ip		/* restore link reg */
	mov	pc, lr
END(mem_size)
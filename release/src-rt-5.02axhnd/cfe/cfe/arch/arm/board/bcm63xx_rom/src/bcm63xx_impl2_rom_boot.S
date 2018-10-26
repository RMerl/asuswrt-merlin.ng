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

#ifndef INC_BTRM_BOOT
#define INC_BTRM_BOOT         0
#endif

/*  *********************************************************************
    *  BOARD_EARLYINIT()
    *
    *  Initialize board registers.  This is the earliest
    *  time the BSP gets control.  This routine cannot assume that
    *  memory is operational, and therefore all code in this routine
    *  must run from registers only.  The lr register must not
    *  be modified, as it contains the return address.
    *
    *  This routine will be called from uncached space, before
    *  the caches are initialized
    *
    *  Among other things, this is where the GPIO registers get
    *  programmed to make on-board LEDs function, or other startup
    *  that has to be done before anything will work.
    *
    *  Input parameters:
    *       nothing
    *       x0, x1, x2 used
    *
    *  Return value:
    *       nothing
    ********************************************************************* */

FUNC(board_earlyinit)
#if defined(_BCM94908_) && defined(CONFIG_BRCM_IKOS)
	/* set up the faster clock for ikos */
	ldr	x0, =BIUCTRL_BASE
	ldr	w1, [x0, #CPU_CLOCK_CONFIG]     // program SysIF clock to divide by 1
	and	w1, w1, #0xffffffe0
	str	w1, [x0, #CPU_CLOCK_CONFIG]
	ldr     w1, [x0, #CPU_CLUSTER0_CLOCK_CONTROL]	// program SEL_CLK_PATTERN to divide by 1
	str     wzr, [x0, #CPU_CLUSTER0_CLOCK_CONTROL]	// program SEL_CLK_PATTERN to divide by 1
#endif

#if defined(_BCM96836_)
	/* on BCM68360, mark the AUX register block accessible in non secure mode */
	/* clock_pattern register is accessed in Linux, as a guest OS, running in EL1 for CPUIDLE clock divider feature  */
	ldr x1, =BIUCFG_AUXBASE
	ldr w0, =0xFF
	str w0, [x1]
#endif

	ret
END(board_earlyinit)

/*  *********************************************************************
    *  BOARD_DRAMINFO
    *
    *  Return the address of the DRAM information table
    *
    *  Input parameters:
    *       nothing
    *
    *  Return value:
    *       v0 - DRAM info table, return 0 to use default table
    ********************************************************************* */
FUNC(board_draminfo)
	ret
END(board_draminfo)

LEAF(board_init)
	ret
END(board_init)

#if defined (CONFIG_CFE_DBG_JTAG_OPEN)
/* Open JTAG + misc. debug interfaces */
#define DBG_JTAG_OPEN mov	x0, #(BROM_GEN_BASE & 0x0000ffff);\
	movk	x0, #(BROM_GEN_BASE >> 16 ), LSL #16;\
	mov	x1, #(BROM_GEN_SECBOOTCFG_INTF_UNLOCK);\
	str	x1, [x0, #BROM_GEN_SECBOOTCFG];
#else
#define DBG_JTAG_OPEN
#endif

#if defined(CFE_ABORT_KEY)
/*  *********************************************************************
    *  int chek_abort_key(void)
    *
    *  Check for abort key and any addition task key
    *
    *  Input parameters:
    *	   none
    *
    *  Return value:
    *	   x0 containg the flags for requested tasks
    *
    *  Registers used:
    *	   x0, x4
    ********************************************************************* */

FUNC(chek_abort_key)

	mov	x14, lr          /* preserve link reg across call */

	mov	x4, #0
	mov	x1, #0

	CALLINIT(=board_getc)
	cmp	w0, #'a'        /* 'a' for abort */
	bne	end_loop
print_msg:
	LOADREL(x0,=print_options)
	CALLINIT(=board_puts)
    mov x4, #0
	
	DBG_JTAG_OPEN
start_loop:

	CALLINIT(=board_getc)
	cmp	w0, #'c'        /* 'c' for continue */
	beq	end_loop
	
	cmp	w0, #'0'        /* '0' to enter hex for MCB select */
	beq	starthex

	cmp	w0, #'i'        /* 'i' for image selection */
	beq	imgsel

	cmp	w0, #'s'        /* 's' for safe mode */
	bne	checkx
safemode:
	LOADREL(x0,=print_safemode)
	CALLINIT(=board_puts)
    mov x4, #0
	mov	x2, #0x1
	orr	w4, w4, w2       /* TODO: define more options in a header file */
	b	end_loop
	/* processing hex input*/
starthex:
	CALLINIT(=board_getc)
    mov x1, #0

	cmp	w0, #0xa       /* CR for continue */
	beq	endhex

	cmp	w0, #0xd       /* NR for continue */
	beq	endhex
	
	lsr	w1, w0, #4      /* removing lower 4 bits to decide on hex char */
	movn	x2, #0xf        /* clearing 58 most significant bits*/
	bic	x0, x0, x2

	cmp	w1, #0x6        /* upper case A-F*/
	beq	hexlet

	cmp	w1, #0x4        /* lower case a-f*/
	beq	hexlet

	cmp	w1, #0x3        /* number */
	beq	hexnum
	
	b	starthex

hexlet:
	cmp	w0, #6
	bgt	starthex
	
	add	x0, x0, #0x9
	b	hexpack
hexnum:
	cmp	w0, #9
	bgt	starthex
hexpack:
	orr	x4, x0, x4, lsl #4
	b	starthex
endhex:
	cmp	x4, #0
	beq	safemode
	mov	x2, #0x1
	orr	x4, x4, x2, lsl #31
	b	end_loop

imgsel:
	mov	x2, #1
	orr	x4, x4, x2, lsl #29 /* shift in selector bit */
	b	end_loop

checkx:
	/* check for any other task key here */
	b	start_loop

end_loop:
	mov	x0, x4
	mov	lr, x14 /* restore link */ 
	ret

END(chek_abort_key)

	.globl print_options
print_options:
	.ascii "\n"
	.ascii "CFE-ROM WAS ABORTED\n"
	.ascii "Options:\n"
	.ascii " c - continue\n"
	.ascii " s - safe mode\n"
	.ascii " i - boot from other image\n"
	.ascii " 0 - followed by hex numbers to select MCB; CR/NL to continue\n"
	.ascii "\n"
	.byte  0x0
	.align 2

	.globl print_safemode
print_safemode:
	.ascii "Booting in safe mode\n"
	.byte  0x0 
	.align 2
#endif

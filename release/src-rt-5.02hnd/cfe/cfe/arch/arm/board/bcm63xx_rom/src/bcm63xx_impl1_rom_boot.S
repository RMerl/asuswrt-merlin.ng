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
#include "bcm_common.h"

#include "bcm_hwdefs.h"
#include "boardparms.h"
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
    *  the caches are initialized.
    *
    *  Among other things, this is where the GPIO registers get
    *  programmed to make on-board LEDs function, or other startup
    *  that has to be done before anything will work.
    *
    *  Input parameters:
    *       nothing
    *       r0, r1 used
    *
    *  Return value:
    *       nothing
    ********************************************************************* */

FUNC(board_earlyinit)
#if defined(_BCM963148_)
#define CPU_RESET_CONFIG_REG		0x80022178
#define CPU_CREDIT_REG			0x80022184
#define CPU_RAC_CONFIG0_REG		0x80022078
#define CPU_RAC_CONFIG1_REG		0x8002207c
#define CPU_UBUS_CONFIG			0x800221dc

	/* set mcp0 read/write credits to 8 */
	ldr	r0, =CPU_CREDIT_REG
	ldr	r1, [r0]
	/*Workaround for HW63148-100: Disabling write pairing by clearing bits 28-31.*/
	ldr	r2, =0x0fffff00
	and	r1, r1, r2
	orr	r1, r1, #0x00000088
	str	r1, [r0]

	/* set RAC_config0 to 0x0 to disable it */
	ldr	r0, =CPU_RAC_CONFIG0_REG
	ldr	r1, =0x0
	str	r1, [r0]
#endif

#if 0 /* for test only. Redirect AIP traffic for memc to exercise ubus<->memc transaction */
	ldr	r0, =0x8001dc04  /* L2C_FILT_END */
	ldr	r1, =0x80000000
	str	r1, [r0]

	ldr	r0, =0x8001dc00  /* L2C_FILT_START */
	ldr	r1, =0x7ff00001  /* This will cause AXI1 (ddr axi port) to only have accesses >= 0x7ff00000  and < 0x80000000 so majority of the ddr memory access goes through AX0-AIP-UBUS-MEMC */
	str	r1, [r0]
#endif

#if defined(_BCM963138_)
	/* enable AIP UBUS fast ack to improve performance */
	ldr	r0, =0x80018000  /* AIP_CTRL_CONFIG */
	ldr	r1, [r0]
	orr	r1, #0x2
	str	r1, [r0]
#endif

#if defined(_BCM963148_)
	/* enable UBUS multiple read/write transaction to improve performance */
	ldr	r0, =CPU_UBUS_CONFIG
	ldr	r1, [r0]
	orr	r1, r1, #0x70
	str	r1, [r0]
#endif

	mov	pc,lr
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
	mov	pc,lr
END(board_draminfo)

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
    *       nothing
    *
    *  Return value:
    *       nothing
    ********************************************************************* */
FUNC(bcmcore_tp1_switch)
	mov	pc, lr
END(bcmcore_tp1_switch)


LEAF(board_init)
	mov	pc, lr
END(board_init)

#if defined(CFE_ABORT_KEY)
/*  *********************************************************************
    *  int chek_abort_key(void)
    *
    *  Check for abort key and any addition task key
    *
    *  Input parameters:
    *      none
    *
    *  Return value:
    *      r0 containg the flags for requested tasks
    *
    *  Registers used:
    *      r0, r4
    ********************************************************************* */
FUNC(chek_abort_key)

	mov	ip, lr           /* persevere link reg across call */

	mov	r4, #0           /* save flags in r3, assuming all functions called don't touch it */

	CALLINIT(=board_getc)
	cmp	r0, #0x61        /* 'a' for abort */
	bne	end_loop
print_msg:
	LOADREL(r0,=print_options)
	CALLINIT(=board_puts)

start_loop:

	CALLINIT(=board_getc)
	cmp	r0, #0x63       /* 'c' for continue */
	beq	end_loop
	
	cmp	r0, #0x30       /* '0' to enter hex for MCB select */
	beq	starthex

	cmp	r0, #0x69       /* 'i' for image selection */
	beq	imgsel

	cmp	r0, #0x73       /* 's' for ddr safe mode */
	bne	checkx
safemode:
	LOADREL(r0,=print_ddr_safemode)
	CALLINIT(=board_puts)
	orr	r4, #0x1       /* TODO: define more options in a header file */
	b	end_loop
	/* processing hex input*/
starthex:
	CALLINIT(=board_getc)
	cmp	r0, #0xa       /* CR for continue */
	beq	endhex

	cmp	r0, #0xd       /* NR for continue */
	beq	endhex
	
	lsr	r1, r0, #4      /* removing lower 4 bits to decide on hex char */
	bfc	r0, #4, #28

	cmp	r1, #0x6        /* upper case A-F*/
	beq	hexlet

	cmp	r1, #0x4        /* lower case a-f*/
	beq	hexlet

	cmp	r1, #0x3        /* number */
	beq	hexnum
	
	b	starthex

hexlet:
	cmp	r0, #6
	bgt	starthex 
	
	add	r0, r0, #0x9
	b	hexpack 
hexnum:
	cmp	r0, #9
	bgt	starthex
hexpack:
	orr	r4, r0, r4, lsl #4
	b	starthex
endhex:
	cmp	r4, #0
	beq	safemode 
	mov	r2, #0x1
	orr	r4, r4, r2, lsl #31
	b	end_loop

imgsel:
	CALLINIT(=board_getc)
	cmp	r0, #0x63           /* 'c' for continue */
	beq	end_loop

	lsr	r1, r0, #4
	cmp	r1, #3              /*   number */
	bne	imgsel

	bfc	r0, #4, #28         /* clear 28 ms bits*/
	mov	r2, #1
	orr	r4, r0, r2, lsl #29 /* shift in selector bit */
	b	imgsel 

checkx:
	/* check for any other task key here */
	b	start_loop

end_loop:
	mov	r0, r4
	mov	lr, ip             /* restore link */
	mov	pc, lr

END(chek_abort_key)

	.globl print_options
print_options:
	.ascii "\r\n"
	.ascii "CFE-ROM WAS ABORTED\r\n"
	.ascii "Please choose one of the following options:\r\n"
	.ascii " c - continue\r\n"
	.ascii " s - DDR safe mode\r\n"
	.ascii " i - followed by 0 to boot to a previous fs image\r\n"
	.ascii " 0 - followed by hex numbers to select MCB; CR/NL to continue\r\n"
	.ascii "\n"
	.byte  0x0
	.align 2

	.globl print_ddr_safemode
print_ddr_safemode:
	.ascii "using DDR safe mode\r\n"
	.byte  0x0
#endif   /* CFE_ABORT_KEY */

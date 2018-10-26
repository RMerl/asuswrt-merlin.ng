/*
    <:copyright-BRCM:2012:proprietary:standard
    
       Copyright (c) 2012 Broadcom 
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
 */


#include "arm.h"
#include "armmacros.h"
#include "segtable.h"
#include "bcm_map.h"

.text 

#define R_OTP_IS_BTRM_ENABLED _TBLIDX(0)
#define R_OTP_IS_BOOT_SECURE  _TBLIDX(1)

otpinit_table:
                _LONG_  otp_is_btrm_enabled                  /* [ 0 ] R_OTP_IS_BTRM_ENABLED */
                _LONG_  otp_is_boot_secure                   /* [ 1 ] R_OTP_IS_BOOT_SECURE */

/*  *********************************************************************
    *  otp_is_btrm_enabled
    *  
    *  checks to see whether the btrm ran at the beginning
    *  
    *  Input parameters: 
    *  	   none
    *  	   
    *  Return value:
    *  	   r0 is nonzero if btrm previously ran; zero otherwise
    ********************************************************************* */
LEAF(otp_is_btrm_enabled)

		# If both the brcm and any of the three customer OTP bootrom enable OTP
		# OTP bits are set, then the bootrom ran at the beginning

        	ldr     r1, =JTAG_OTP_BASE

        	ldr     r2, [r1, #JTAG_OTP_GENERAL_CTRL_1]
        	orr     r2, r2, #JTAG_OTP_GENERAL_CTRL_1_CPU_MODE
        	str     r2, [r1, #JTAG_OTP_GENERAL_CTRL_1]

        	ldr     r2, =OTP_BRCM_BTRM_BOOT_ENABLE_ROW
        	str     r2, [r1, #JTAG_OTP_GENERAL_CTRL_3]

        	ldr     r2, =JTAG_OTP_GENERAL_CTRL_0_START
		orr	r2, r2, #JTAG_OTP_GENERAL_CTRL_0_PROG_EN
		orr	r2, r2, #JTAG_OTP_GENERAL_CTRL_0_ACCESS_MODE
        	str     r2, [r1, #JTAG_OTP_GENERAL_CTRL_0]

        	mov     r2, #8000
otpwait1:	sub     r2, r2, #1
		cmp	r2, #0
        	bne     otpwait1
        	nop

        	mov     r0, #0x2000000
otpwait2:	sub	r0, r0, #1
		cmp	r0, #0
        	beq     return1  /* failed to read row */
        	nop
        	ldr     r2, [r1, #JTAG_OTP_GENERAL_STATUS_1]
        	and     r2, r2, #JTAG_OTP_GENERAL_STATUS_1_CMD_DONE
		cmp	r2, #0
        	beq     otpwait2
        	nop

        	ldr     r2, [r1, #JTAG_OTP_GENERAL_STATUS_0]
        	and     r2, r2, #OTP_BRCM_BTRM_BOOT_ENABLE_MASK
		mov     r2, r2, lsr #OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT
	   	/* zero out  the ctrl_0 reg */       
		mov	r0, #0
        	str     r0, [r1, #JTAG_OTP_GENERAL_CTRL_0]
		cmp	r2, r0
        	beq     return1 	/* bootrom did not run */
        	nop

		ldr	r2, =OTP_CUST_BTRM_BOOT_ENABLE_ROW
        	str     r2, [r1, #JTAG_OTP_GENERAL_CTRL_3]

        	ldr     r2, =JTAG_OTP_GENERAL_CTRL_0_START
		orr	r2, r2, #JTAG_OTP_GENERAL_CTRL_0_PROG_EN
		orr	r2, r2, #JTAG_OTP_GENERAL_CTRL_0_ACCESS_MODE
        	str     r2, [r1, #JTAG_OTP_GENERAL_CTRL_0]

        	mov     r2, #8000
otpwait3:	sub     r2, r2, #1
		cmp	r2, #0
        	bne     otpwait3
        	nop

        	mov     r0, #0x2000000
otpwait4:	sub	r0, r0, #1
		cmp	r0, #0
        	beq     return1  /* failed to read row */
        	nop
        	ldr     r2, [r1, #JTAG_OTP_GENERAL_STATUS_1]
        	and     r2, r2, #JTAG_OTP_GENERAL_STATUS_1_CMD_DONE
		cmp	r2, #0
        	beq     otpwait4
        	nop

        	ldr     r2, [r1, #JTAG_OTP_GENERAL_STATUS_0]
        	and     r2, r2, #OTP_CUST_BTRM_BOOT_ENABLE_MASK
		mov     r2, r2, lsr #OTP_CUST_BTRM_BOOT_ENABLE_SHIFT
	   	/* zero out  the ctrl_0 reg */       
		mov	r0, #0
        	str     r0, [r1, #JTAG_OTP_GENERAL_CTRL_0]
		cmp	r2, r0
        	beq     return1 	/* bootrom did not run */
        	nop
     
		mov  	r0, #1    /* bootrom did run */

return1:       	ldr     r2, [r1, #JTAG_OTP_GENERAL_CTRL_1]
		and     r2, r2, #(~JTAG_OTP_GENERAL_CTRL_1_CPU_MODE)
        	str     r2, [r1, #JTAG_OTP_GENERAL_CTRL_1]

		mov 	pc, lr

END(otp_is_btrm_enabled)

/*  *********************************************************************
    *  otp_is_boot_secure
    *  
    *  checks to see whether the bootup is secure or not
    *  
    *  Input parameters: 
    *  	   nothing
    *  	   
    *  Return value:
    *  	   r0 is nonzero if boot is secure; zero otherwise
    ********************************************************************* */
LEAF(otp_is_boot_secure)

		mov 	r3, lr 		/* Store away the ra register because we are calling another routine */
		bl 	otp_is_btrm_enabled
		mov	lr, r3		/* restoring the ra register */
		cmp	r0, #0
		beq 	__unsecure_bt 	/* bootrom didn't even run ... not secure boot*/
		nop

        	ldr     r1, =JTAG_OTP_BASE
        	ldr     r2, [r1, #JTAG_OTP_GENERAL_CTRL_1]
        	orr     r2, r2, #JTAG_OTP_GENERAL_CTRL_1_CPU_MODE
        	str     r2, [r1, #JTAG_OTP_GENERAL_CTRL_1]

        	ldr     r2, =OTP_CUST_MFG_MRKTID_ROW
        	str     r2, [r1, #JTAG_OTP_GENERAL_CTRL_3]

        	ldr     r2, =JTAG_OTP_GENERAL_CTRL_0_START
		orr	r2, r2, #JTAG_OTP_GENERAL_CTRL_0_PROG_EN
		orr	r2, r2, #JTAG_OTP_GENERAL_CTRL_0_ACCESS_MODE
        	str     r2, [r1, #JTAG_OTP_GENERAL_CTRL_0]

        	mov     r2, #8000
otpwait5:	sub     r2, r2, #1
		cmp	r2, #0
        	bne     otpwait5
        	nop

        	mov     r0, #0x2000000
otpwait6:	sub	r0, r0, #1
		cmp	r0, #0
        	beq     return2  	/* failed to read row */
        	nop
        	ldr     r2, [r1, #JTAG_OTP_GENERAL_STATUS_1]
        	and     r2, r2, #JTAG_OTP_GENERAL_STATUS_1_CMD_DONE
		cmp	r2, #0
        	beq     otpwait6
        	nop

        	ldr     r2, [r1, #JTAG_OTP_GENERAL_STATUS_0]
		ldr	r3, =OTP_CUST_MFG_MRKTID_MASK
        	and     r2, r2, r3
		mov     r2, r2, lsr #OTP_CUST_MFG_MRKTID_SHIFT
	   	/* zero out  the ctrl_0 reg */       
		mov	r0, #0
        	str     r0, [r1, #JTAG_OTP_GENERAL_CTRL_0]
		cmp	r2, r0
        	beq     return2 	/* MID is zero ... unsecure boot */
        	nop
     
		mov  	r0, #1    /* bootrom did run and it performed a secure boot */

return2:       	ldr     r2, [r1, #JTAG_OTP_GENERAL_CTRL_1]
	       	and     r2, r2, #(~JTAG_OTP_GENERAL_CTRL_1_CPU_MODE)
        	str     r2, [r1, #JTAG_OTP_GENERAL_CTRL_1]

__unsecure_bt:	nop
		mov 	pc, lr

END(otp_is_boot_secure)

/*
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
 */


#include "armv8.h"
#include "armmacros.h"
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
    *      none
    *      
    *  Return value:
    *      w0 is nonzero if btrm previously ran; zero otherwise
    ********************************************************************* */
LEAF(otp_is_btrm_enabled)

                /* Save link register */
                mov     x14, x30

                /* bootrom did run */
                mov     w0, #1   

                /* Restore link register */
                mov     x30, x14
                ret

END(otp_is_btrm_enabled)


/*  *********************************************************************
    *  otp_is_boot_secure
    *  
    *  checks to see whether the SoC is in mfg or field secure mode or not
    *  
    *  Input parameters: 
    *      nothing
    *      
    *  Return value:
    *      w0 is 1 if boot is mfg or field secure; zero otherwise
    ********************************************************************* */
LEAF(otp_is_boot_secure)

                /* Save link register */
                mov     x14, x30

                ldr     x1, =JTAG_OTP_BASE
                ldr     w2, [x1, #JTAG_OTP_GENERAL_CTRL_1]
                orr     w2, w2, #JTAG_OTP_GENERAL_CTRL_1_CPU_MODE
                str     w2, [x1, #JTAG_OTP_GENERAL_CTRL_1]

                ldr     w2, =OTP_CUST_BTRM_BOOT_ENABLE_ROW
                str     w2, [x1, #JTAG_OTP_GENERAL_CTRL_3]

                ldr     w2, =JTAG_OTP_GENERAL_CTRL_0_START
                orr     w2, w2, #JTAG_OTP_GENERAL_CTRL_0_PROG_EN
                orr     w2, w2, #JTAG_OTP_GENERAL_CTRL_0_ACCESS_MODE
                str     w2, [x1, #JTAG_OTP_GENERAL_CTRL_0]

                mov     w2, #8000
otpwait1:       sub     w2, w2, #1
                cmp     w2, #0
                bne     otpwait1
                nop

                mov     w0, #0x2000000
otpwait2:       sub     w0, w0, #1
                cmp     w0, #0
                beq     done         /* failed to read row */
                nop
                ldr     w2, [x1, #JTAG_OTP_GENERAL_STATUS_1]
                and     w2, w2, #JTAG_OTP_GENERAL_STATUS_1_CMD_DONE
                cmp     w2, #0
                beq     otpwait2
                nop

                ldr     w2, [x1, #JTAG_OTP_GENERAL_STATUS_0]
                ldr     w3, =OTP_CUST_BTRM_BOOT_ENABLE_MASK
                and     w2, w2, w3
                /* zero out  the ctrl_0 reg */
                mov     w0, #0
                str     w0, [x1, #JTAG_OTP_GENERAL_CTRL_0]
                cmp     w2, w0
                beq     done   /* none of the three customer bootrom-enable otp bits are set ... unsecure boot */
                nop

                mov     w0, #1    /* bootrom did run and it performed a secure boot */

done:           ldr     w2, [x1, #JTAG_OTP_GENERAL_CTRL_1]
                and     w2, w2, #(~JTAG_OTP_GENERAL_CTRL_1_CPU_MODE)
                str     w2, [x1, #JTAG_OTP_GENERAL_CTRL_1]

                /* Restore link register */
                mov     x30, x14
                ret

END(otp_is_boot_secure)

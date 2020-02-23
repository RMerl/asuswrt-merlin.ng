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

#if defined(_BCM96838_) || defined(_BCM963268_) || defined(_BCM963381_) || defined(_BCM96848_)

#include "sbmips.h"
#include "mipsmacros.h"
#include "segtable.h"
#include "bcm_map.h"

.text 

#define R_OTP_IS_BTRM_ENABLED	_TBLIDX(0)
#define R_OTP_IS_BOOT_SECURE	_TBLIDX(1)
#if defined(_BCM96848_)
#define R_OTP_IS_TP1_ENABLED	_TBLIDX(2)
#define R_OTP_GET_MAX_DDR_FREQ	_TBLIDX(3)
#define R_OTP_GET_MAX_CLK_SEL	_TBLIDX(4)
#define R_OTP_GET_REVID			_TBLIDX(5)
#endif

otpinit_table:
	_LONG_  otp_is_btrm_enabled		# [ 0 ] R_OTP_IS_BTRM_ENABLED
	_LONG_  otp_is_boot_secure		# [ 1 ] R_OTP_IS_BOOT_SECURE
#if defined(_BCM96848_)
	_LONG_  otp_is_tp1_enabled		# [ 2 ] R_OTP_IS_TP1_ENABLED
	_LONG_  otp_get_max_ddr_freq	# [ 3 ] R_OTP_GET_MAX_DDR_FREQ
	_LONG_  otp_get_max_clk_sel		# [ 4 ] R_OTP_GET_MAX_CLK_SEL
	_LONG_  otp_get_revid			# [ 5 ] R_OTP_GET_REVID
#endif

#if defined(_BCM963381_) || defined(_BCM96848_)
/*  *********************************************************************
	*  otp_get_row
	*
	*  Read 32 bit row from OTP.
	*
	*  Registers used - t1, t2, t3, t4
	*
	*  Input parameters:
	*      a0 - Row number.
	*
	*  Return value:
	*      v0 is row value.
	********************************************************************* */
LEAF(otp_get_row)

	.set noreorder

	li      v0, 0    /* Assume at first, otp is not set */

	li      t1, JTAG_OTP_BASE
	lw      t2, JTAG_OTP_GENERAL_CTRL_1(t1)
	or      t2, JTAG_OTP_GENERAL_CTRL_1_CPU_MODE
	sw      t2, JTAG_OTP_GENERAL_CTRL_1(t1)
	/*      t2 contains the ctr1 register value, don't override, need it at the end*/

	sw      a0, JTAG_OTP_GENERAL_CTRL_3(t1)       /* set the row */
	li      t3, JTAG_OTP_GENERAL_CTRL_0_START | JTAG_OTP_GENERAL_CTRL_0_PROG_EN | JTAG_OTP_GENERAL_CTRL_0_ACCESS_MODE
	sw      t3, JTAG_OTP_GENERAL_CTRL_0(t1)
	li      t3, 800
otpwait1:
	sub     t3, 1
	bne     t3, zero, otpwait1
	nop
	li      t3, 0x200000
otpwait2:
	sub t3, 1
	beq     t3, zero, return1  /* failed to read row */
	nop
	lw      t4, JTAG_OTP_GENERAL_STATUS_1(t1)
	and     t4, JTAG_OTP_GENERAL_STATUS_1_CMD_DONE
	beq     t4, zero, otpwait2
	nop

	lw      v0, JTAG_OTP_GENERAL_STATUS_0(t1)
	/* zero out  the ctrl_0 reg */
	sw      zero, JTAG_OTP_GENERAL_CTRL_0(t1)

return1:
	and     t2, ~JTAG_OTP_GENERAL_CTRL_1_CPU_MODE
	sw      t2, JTAG_OTP_GENERAL_CTRL_1(t1)

	jr  ra
	nop
	.set reorder

END(otp_get_row)
#endif

/*  *********************************************************************
	*  otp_is_btrm_enabled
	*  
	*  checks to see whether the btrm ran at the beginning
	*
	*  Registers used:
	*      t0, t1, t2, t3, t4, t5, a0
	*  
	*  Input parameters: 
	*  	   none
	*  	   
	*  Return value:
	*  	   v0 is nonzero if btrm previously ran; zero otherwise
	********************************************************************* */
LEAF(otp_is_btrm_enabled)

	.set noreorder
#if defined(_BCM96838_)

	# If either or the brcm or customer OTP bootrom enable OTP bits 
	# are set, then the bootrom ran at the beginning
	li  	t0, OTP_BASE
	lw  	t1, OTP_SHADOW_BRCM_BITS_96_127(t0)
	and 	t1, OTP_BRCM_BTRM_BOOT_ENABLE_MASK
	lw  	t2, OTP_SHADOW_CUST_BITS_0_31(t0)
	and 	t2, OTP_CUST_BTRM_BOOT_ENABLE_MASK
	or  	v0, t1, t2

#elif defined(_BCM963268_) 

#if (INC_BTRM_BOOT==1)
	li  	v0, 1
#else
	li  	v0, 0
#endif

#elif defined(_BCM963381_) || defined(_BCM96848_)

#if defined(CONFIG_BRCM_IKOS)
	/* IKOS does not support OTP model, so have to hard code here. change the return
		value as need here */
	li  	v0, 0
#else	
	# If both the brcm and any of the three customer OTP bootrom enable OTP
	# OTP bits are set, then the bootrom ran at the beginning

	move t5, ra     /* save return address in t5 */

	li  a0, OTP_BRCM_BTRM_BOOT_ENABLE_ROW
	bal otp_get_row
	nop

	and v0, OTP_BRCM_BTRM_BOOT_ENABLE_MASK
	srl v0, OTP_BRCM_BTRM_BOOT_ENABLE_SHIFT

	beq v0, zero, return 		/* bootrom did not run */
	nop

#if !defined(_BCM96848_)
	li  a0, OTP_CUST_BTRM_BOOT_ENABLE_ROW
	bal otp_get_row
	nop

	and v0, OTP_CUST_BTRM_BOOT_ENABLE_MASK
	srl v0, OTP_CUST_BTRM_BOOT_ENABLE_SHIFT

	beq v0, zero, return 		/* bootrom did not run */
	nop
#endif

	li  	v0, 1    /* bootrom did run */

return: 
	move ra, t5
#endif

#endif
	jr	ra
	nop
	.set reorder

END(otp_is_btrm_enabled)

/*  *********************************************************************
	*  otp_is_boot_secure
	*  
	*  checks to see whether the bootup is secure or not
	*
	*  Registers used:
	*      t0, t1, t2, t3, t4, t5, t6, a0
	*  
	*  Input parameters: 
	*  	   nothing
	*  	   
	*  Return value:
	*  	   v0 is nonzero if boot is secure; zero otherwise
	********************************************************************* */
LEAF(otp_is_boot_secure)

	.set noreorder
#if defined(_BCM96838_)

	# If either or the brcm or customer OTP bootrom enable OTP bits 
	# are set, then the bootrom ran at the beginning
	li  	t0, OTP_BASE
	lw  	t1, OTP_BRCM_BTRM_BOOT_ENABLE_ROW(t0)
	and 	t1, OTP_BRCM_BTRM_BOOT_ENABLE_MASK
	lw  	t2, OTP_CUST_BTRM_BOOT_ENABLE_ROW(t0)
	and 	t2, OTP_CUST_BTRM_BOOT_ENABLE_MASK
	or  	v0, t1, t2
	beq 	v0, 0, __unsecure_bt
	nop

	lw  	v0, OTP_CUST_MFG_MRKTID_ROW(t0)
	and 	v0, OTP_CUST_MFG_MRKTID_MASK

#elif defined(_BCM963268_)

#if (INC_BTRM_BOOT==1)
	li  	v0, 1
#else
	li  	v0, 0
#endif

#elif defined(_BCM963381_) || defined(_BCM96848_)
	
#if defined(CONFIG_BRCM_IKOS)
	/* IKOS does not support OTP model, so have to hard code here. change the return
		value as need here */
	li  	v0, 0	
#else	
	move 	t6, ra 		/* Store away the ra register because we are calling another routine */
	bal 	otp_is_btrm_enabled
	nop
	move 	ra, t6		/* restoring the ra register */
	beq 	v0, 0, __unsecure_bt 	/* bootrom didn't even run ... not secure boot*/
	nop

	move t6, ra
	li  a0, OTP_CUST_MFG_MRKTID_ROW
	bal otp_get_row
	nop
	move ra, t6

	and v0, OTP_CUST_MFG_MRKTID_MASK
	srl v0, OTP_CUST_MFG_MRKTID_SHIFT

	beq v0, zero, return2	 	/* MID is zero ... unsecure boot */
	nop

	li  	v0, 1    /* bootrom did run and it performed a secure boot */

return2:

#endif

#endif

__unsecure_bt:	
	nop
	jr  ra
	nop
	.set reorder

END(otp_is_boot_secure)

#if defined(_BCM96848_)
/*  *********************************************************************
	*  otp_is_tp1_enabled
	*
	*  checks to see whether tp1 is enabled
	*
	*  Input parameters:
	*      none
	*
	*  Return value:
	*      v0 is nonzero if tp1 is enabled; zero otherwise
	********************************************************************* */
LEAF(otp_is_tp1_enabled)
	.set noreorder

	move t5, ra

	li  a0, OTP_BRCM_FEATURE_DISABLE_ROW
	bal otp_get_row
	nop
	move t4, v0
	and  t4, OTP_BRCM_TP1_DISABLE_MASK
	srl  t4, OTP_BRCM_TP1_DISABLE_SHIFT

	li  v0, 0
	bne t4, zero, return3 
	nop

	li      v0, 1 

return3:     
	move ra, t5
	jr  ra
	nop
	.set reorder

END(otp_is_tp1_enabled)

/*  *********************************************************************
	*  otp_get_max_ddr_freq
	*
	*  read otp bits which determines max ddr frequency.
	*
	*  Input parameters:
	*      none
	*
	*  Return value:
	*      v0 otp bits
	********************************************************************* */
LEAF(otp_get_max_ddr_freq)

	.set noreorder

	move t5, ra

	li  a0, OTP_BRCM_FEATURE_DISABLE_ROW
	bal otp_get_row
	nop
	and  v0, OTP_BRCM_MAX_DDR_FREQ_MASK
	srl  v0, OTP_BRCM_MAX_DDR_FREQ_SHIFT

	move ra, t5
	jr  ra
	nop
	.set reorder

END(otp_get_max_ddr_freq)

/*  *********************************************************************
	*  otp_get_max_clk_sel
	*
	*  read otp bits which determines max clock frequency.
	*
	*  Input parameters:
	*      none
	*
	*  Return value:
	*      v0 otp bits
	********************************************************************* */
LEAF(otp_get_max_clk_sel)

	.set noreorder

	move t5, ra

	li  a0, OTP_BRCM_FEATURE_DISABLE_ROW
	bal otp_get_row
	nop
	and  v0, OTP_BRCM_MAX_CLOCK_FREQ_MASK
	srl  v0, OTP_BRCM_MAX_CLOCK_FREQ_SHIFT

	move ra, t5
	jr  ra
	nop
	.set reorder

END(otp_get_max_clk_sel)


LEAF(otp_get_revid)

	.set noreorder

	move t5, ra

	li  a0, OTP_BRCM_CHIPID_ROW
	bal otp_get_row
	nop
	and  v0, OTP_BRCM_CHIP_REVID_MASK
	srl  v0, OTP_BRCM_CHIP_REVID_SHIFT

	move ra, t5
	jr  ra
	nop
	.set reorder

END(otp_get_revid)

#endif     /* if defined(_BCM96848_) */

#endif /* defined(_BCM96838_) || defined(_BCM963268_) || defined(_BCM963381_) */

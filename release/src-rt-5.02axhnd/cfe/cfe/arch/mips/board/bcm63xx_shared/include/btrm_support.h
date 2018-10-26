/**********************************************************************
 *  
 *
 *     <:copyright-BRCM:2012:proprietary:standard
 *     
 *        Copyright (c) 2012 Broadcom 
 *        All Rights Reserved
 *     
 *      This program is the proprietary software of Broadcom and/or its
 *      licensors, and may only be used, duplicated, modified or distributed pursuant
 *      to the terms and conditions of a separate, written license agreement executed
 *      between you and Broadcom (an "Authorized License").  Except as set forth in
 *      an Authorized License, Broadcom grants no license (express or implied), right
 *      to use, or waiver of any kind with respect to the Software, and Broadcom
 *      expressly reserves all rights in and to the Software and all intellectual
 *      property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 *      NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 *      BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *     
 *      Except as expressly set forth in the Authorized License,
 *     
 *      1. This program, including its structure, sequence and organization,
 *         constitutes the valuable trade secrets of Broadcom, and you shall use
 *         all reasonable efforts to protect the confidentiality thereof, and to
 *         use this information only in connection with your use of Broadcom
 *         integrated circuit products.
 *     
 *      2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *         AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *         WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *         RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
 *         ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
 *         FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
 *         COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
 *         TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
 *         PERFORMANCE OF THE SOFTWARE.
 *     
 *      3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 *         ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *         INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
 *         WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 *         IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
 *         OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
 *         SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
 *         SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
 *         LIMITED REMEDY.
 *     :>
 *
 ********************************************************************* 
 */

/**********************************************************************
 *  
 *  btrm_support.h       
 *
 *  Author:  Brian Nay (brian.nay@broadcom.com)
 *  
 *********************************************************************  
 *
 *  Copyright 2014
 *  Broadcom Corporation. All rights reserved.
 *
 ********************************************************************* 
 *             */
#ifndef BTRM_SUPPORT_H_
#define BTRM_SUPPORT_H_

#define CP0_CFG_ISMSK      (0x7 << 22)
#define CP0_CFG_ISSHF      22
#define CP0_CFG_ILMSK      (0x7 << 19)
#define CP0_CFG_ILSHF      19
#define CP0_CFG_IAMSK      (0x7 << 16)
#define CP0_CFG_IASHF      16
#define CP0_CFG_DSMSK      (0x7 << 13)
#define CP0_CFG_DSSHF      13
#define CP0_CFG_DLMSK      (0x7 << 10)
#define CP0_CFG_DLSHF      10
#define CP0_CFG_DAMSK      (0x7 << 7)
#define CP0_CFG_DASHF      7

#define cacheop(kva, size, linesize, op) \
        addu    t1, kva, size;   \
        subu    t2, linesize, 1; \
        not     t2;              \
        and     t0, kva, t2;     \
        addu    t1, -1;          \
        and     t1, t2;          \
10:     cache   op, 0(t0);       \
        bne     t0, t1, 10b;     \
        addu    t0, linesize;    \
11:

#define size_icache(size, linesize) \
        mfc0    t7, C0_CONFIG, 1;       \
        and     t0, t7, CP0_CFG_ILMSK;  \
        srl     t0, t0, CP0_CFG_ILSHF;  \
        move    linesize, zero;         \
        beq     t0, zero,1f;            \
        add     t0, 1;                  \
        li      linesize, 1;            \
        sll     linesize, t0;           \
1:      and     t0, t7, CP0_CFG_ISMSK;  \
        srl     t0, t0, CP0_CFG_ISSHF;  \
        li      size, 64;               \
        sll     size, t0;               \
        and     t0, t7, CP0_CFG_IAMSK;  \
        srl     t0, t0, CP0_CFG_IASHF;  \
        add     t0, 1;                  \
        mult    size, t0;               \
        mflo    size;                   \
        mult    size, linesize;         \
        mflo    size

#define size_dcache(size, linesize) \
        mfc0    t7, C0_CONFIG, 1;       \
        and     t0, t7, CP0_CFG_DLMSK;  \
        srl     t0, t0, CP0_CFG_DLSHF;  \
        move    linesize, zero;         \
        beq     t0, zero,1f;            \
        add     t0, 1;                  \
        li      linesize, 1;            \
        sll     linesize, t0;           \
1:      and     t0, t7, CP0_CFG_DSMSK;  \
        srl     t0, t0, CP0_CFG_DSSHF;  \
        li      size, 64;               \
        sll     size, t0;               \
        and     t0, t7, CP0_CFG_DAMSK;  \
        srl     t0, t0, CP0_CFG_DASHF;  \
        add     t0, 1;                  \
        mult    size, t0;               \
        mflo    size;                   \
        mult    size, linesize;         \
        mflo    size


#define flush_inval_caches() \
        li      a0, K0BASE;		\
        size_dcache(a1, a2);		\
        addu    t1, a0, a1;		\
        subu    t2, a2, 1;		\
        not     t2;			\
        and     t0, a0, t2;		\
        addu    t1, -1;			\
        and     t1, t2;			\
1:      cache   Index_Load_Tag_D, 0(t0);\
        nop;				\
        nop;				\
        nop;				\
        nop;				\
        nop;				\
        nop;				\
        mfc0    t2, C0_TAGLO;		\
        and     t2, 0x1f000000;		\
        li      t3, 0x1f000000;		\
        bne     t2, t3, 2f;		\
        mtc0    zero, C0_TAGLO;		\
        cache   Index_Store_Tag_D, 0(t0);\
2:      bne     t0, t1, 1b;		\
        addu    t0, a2;			\
        cacheop(a0, a1, a2, Index_Writeback_Inv_D);\
        li      a0, K0BASE;		\
        size_icache(a1, a2);		\
        cacheop(a0, a1, a2, Index_Invalidate_I)

#if defined(_BCM963268_) && (INC_BTRM_BOOT==1)

/* Check whether the customer OTP bit "secure boot enable" is fused. */
/* If not, assume first time 63268 power-on such that the flash      */
/* contains an image that supports secure boot, but the SoC is not   */
/* running the bootrom yet. Therefore, fuse a 16 bit MID, and the    */
/* customer OTP bit "secure boot enable". Then perform a soft reset  */
#define mid_otp_fuse() \
        li      t1, OTP_BASE;					\
        lw      t0, OTP_OTP_CUST_CNTRL_0(t1);			\
        li      t1, OTP_OTP_CUST_CNTRL_0_SEC_BT_ENABLE;		\
        and     t2, t0, t1;					\
        bne     t2,0,__custOtpSet;				\
        SETLEDS1('O','T','P','F');				\
        SETLEDS1('M','I','D','F');				\
        li      t1, OTP_BASE;					\
        li      t2, OTP_SEC_BT_MID_OTP_ADDR;			\
        sw      t2, OTP_OTP_ADDR(t1);				\
        li      t2, OTP_SEC_BT_FACTORY_MFG_MRKT_ID_VAL << 16;	\
        sw      t2, OTP_OTP_WRITE(t1);				\
        li      t2, 0x2;					\
        sw      t2, OTP_OTP_CONFIG(t1);				\
        li      t2, 0xa00005;					\
        sw      t2, OTP_OTP_CONTROL(t1);			\
__midWrWait:    nop;						\
        nop;							\
        lw      t0, OTP_OTP_STATUS(t1);				\
        bne     t0, 0x83,__midWrWait;				\
        li      t2, 0x0;					\
        sw      t2, OTP_OTP_CONTROL(t1);			\
        SETLEDS1('S','B','E','F');				\
        li      t1, OTP_BASE;					\
        li      t2, OTP_SEC_BT_ENABLE_OTP_ADDR;			\
        sw      t2, OTP_OTP_ADDR(t1);				\
        li      t2, OTP_OTP_CUST_CNTRL_0_SEC_BT_ENABLE;		\
        sw      t2, OTP_OTP_WRITE(t1);				\
        li      t2, 0x2;					\
        sw      t2, OTP_OTP_CONFIG(t1);				\
        li      t2, 0xa00005;					\
        sw      t2, OTP_OTP_CONTROL(t1);			\
__secBtEnWait:  nop;						\
        nop;							\
        lw      t0, OTP_OTP_STATUS(t1);				\
        bne     t0, 0x83,__secBtEnWait;				\
        li      t2, 0x0;					\
        sw      t2, OTP_OTP_CONTROL(t1);			\
        SETLEDS1('S','R','S','T');				\
        li      t1, PERF_BASE;					\
        li      t0, PERF_TIMER_CONTROL_SOFTRST;			\
        sw      t0, PERF_TIMER_CONTROL(t1);			\
__pwrCycWait:   nop;						\
        nop;							\
        b       __pwrCycWait;					\
__custOtpSet:   nop;						\
        nop

#endif /* defined(_BCM963268_) && (INC_BTRM_BOOT==1) */

#define unlockJtag()						\
#if defined(_BCM96838_) || defined(_BCM963268_)			\
	li      t8, OTP_BASE;					\
	lw      t9, OTP_OTP_SECURE_BOOT_CFG(t8);		\
#if defined(_BCM963268_)					\
	or      t9, OTP_OTP_SECURE_BOOT_CFG_UNLOCK_JTAG;	\
#else								\
	and     t9,~OTP_OTP_SECURE_BOOT_CFG_LOCK_JTAG;		\
#endif								\
        sw      t9, OTP_OTP_SECURE_BOOT_CFG(t8);		\
#elif defined(_BCM963381_)					\
	li      t8, BROM_SEC_BASE;				\
	lw      t9, BROM_SEC_SECBOOTCFG(t8);			\
	or      t9, BROM_SEC_SECBOOTCFG_JTAG_UNLOCK;		\
	sw      t9, BROM_SEC_SECBOOTCFG(t8);			\
#endif								\
	nop

#endif /* BTRM_SUPPORT_H_ */

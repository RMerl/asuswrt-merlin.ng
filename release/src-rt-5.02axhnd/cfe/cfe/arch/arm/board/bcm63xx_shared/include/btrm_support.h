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

#define shredder() 							\
	/* shred from beginning of internal mem  up to the shredder */  \
	mov     r5,#(BTRM_INT_MEM_BEGIN_ADDR & 0xff000000);		\
	orr     r5,r5,#(BTRM_INT_MEM_BEGIN_ADDR & 0x00ff0000);		\
	add     r5,r5,#0x4000;						\
	mov     r6,#(BTRM_INT_MEM_SHREDDER_PROG_ADDR & 0xff000000);	\
	orr     r6,r6,#(BTRM_INT_MEM_SHREDDER_PROG_ADDR & 0x00ff0000);	\
        orr     r6,r6,#(BTRM_INT_MEM_SHREDDER_PROG_ADDR & 0x0000ff00);	\
        mov     r0,#0;							\
1:	str     r0,[r5,#0];						\
	str     r0,[r5,#4];						\
	str     r0,[r5,#8];						\
	str     r0,[r5,#12];						\
	add     r5,r5,#16;						\
	cmp     r5,r6;							\
	blt     1b;							\
	/* if auth failed, shred from credentials to end of mem */	\
	/* if auth passed, shred from end of credentials to end mem */	\
        ldr     r0,[sp];						\
        mov     r1,#0;							\
        cmp     r0,r1;							\
        beq     1f;							\
        mov     r5,#(BTRM_INT_MEM_CREDENTIALS_END_ADDR & 0xff000000); 	\
        orr     r5,r5,#(BTRM_INT_MEM_CREDENTIALS_END_ADDR & 0x00ff0000);\
        orr     r5,r5,#(BTRM_INT_MEM_CREDENTIALS_END_ADDR & 0x0000ff00);\
        orr     r5,r5,#(BTRM_INT_MEM_CREDENTIALS_END_ADDR & 0x000000ff);\
        b       2f;							\
1:      mov     r5,#(BTRM_INT_MEM_CREDENTIALS_ADDR & 0xff000000); 	\
        orr     r5,r5,#(BTRM_INT_MEM_CREDENTIALS_ADDR & 0x00ff0000);	\
        orr     r5,r5,#(BTRM_INT_MEM_CREDENTIALS_ADDR & 0x0000ff00);	\
2:      mov     r6,#(BTRM_INT_MEM_END_ADDR & 0xff000000);		\
        orr     r6,r6,#(BTRM_INT_MEM_END_ADDR & 0x00ff0000);		\
        orr     r6,r6,#(BTRM_INT_MEM_END_ADDR & 0x0000ff00);		\
        mov     r1,#0;							\
1:      str     r1,[r5,#0];						\
        str     r1,[r5,#4];						\
        str     r1,[r5,#8];						\
        str     r1,[r5,#12];						\
        add     r5,r5,#16;						\
        cmp     r5,r6;							\
        blt     1b;							\
	/* shred encryption credentials even if auth passed */		\
        mov     r1,#0;							\
        cmp     r0,r1;							\
        beq     1f;							\
        mov     r5,#(BTRM_INT_MEM_CREDENTIALS_ADDR & 0xff000000); 	\
        orr     r5,r5,#(BTRM_INT_MEM_CREDENTIALS_ADDR & 0x00ff0000);	\
        orr     r5,r5,#(BTRM_INT_MEM_CREDENTIALS_ADDR & 0x0000ff00);	\
        orr     r5,r5,#(BTRM_INT_MEM_CREDENTIALS_ADDR & 0x000000ff);	\
	add	r5,r5,#512; 						\
        mov     r6,#(BTRM_INT_MEM_CREDENTIALS_ADDR & 0xff000000); 	\
        orr     r6,r6,#(BTRM_INT_MEM_CREDENTIALS_ADDR & 0x00ff0000);	\
        orr     r6,r6,#(BTRM_INT_MEM_CREDENTIALS_ADDR & 0x0000ff00);	\
        orr     r6,r6,#(BTRM_INT_MEM_CREDENTIALS_ADDR & 0x000000ff);	\
	add	r6,r6,#512;						\
	add	r6,r6,#64;						\
        mov     r1,#0;							\
1:      str     r1,[r5,#0];						\
        str     r1,[r5,#4];						\
        str     r1,[r5,#8];						\
        str     r1,[r5,#12];						\
        add     r5,r5,#16;						\
        cmp     r5,r6;							\
        blt     1b;							\
	/* store arg passed into cfe_launch() back onto the stack */	\
        str     r0,[sp];						\
	/* invalidate i-cache, flush d-cache */				\
        mov   r0, #0;							\
        mcr   p15, 0, r0, c7, c5, 0; 					\
        isb	;							\
        mrc     p15, 1, r0, c0, c0, 1;       				\
        ands    r3, r0, #0x7000000;           				\
        mov     r3, r3, lsr #23;                			\
        beq     lnch_fin;                     				\
        mov     r10, #0;                        			\
lnch_loop1:								\
        add     r2, r10, r10, lsr #1;   				\
        mov     r1, r0, lsr r2;          				\
        and     r1, r1, #7;                				\
        cmp     r1, #2;                    				\
        blt     lnch_skip;                  				\
        mcr     p15, 2, r10, c0, c0, 0;      				\
        isb	;							\
        mrc     p15, 1, r1, c0, c0, 0;          			\
        and     r2, r1, #7;                   				\
        add     r2, r2, #4;                   				\
        mov     r4, #0xff;                     				\
        orr     r4, r4, #0x300;						\
        ands    r4, r4, r1, lsr #3;           				\
        clz     r5, r4;                       				\
        mov     r7, #0xff;                   				\
        orr     r7, r7, #0x7f00;					\
        ands    r7, r7, r1, lsr #13;         				\
lnch_loop2:								\
        mov     r9, r4;                   				\
lnch_loop3:								\
        orr     r6, r10, r9, lsl r5;        				\
        orr     r6, r6, r7, lsl r2;          				\
        mcr     p15, 0, r6, c7, c14, 2;      				\
        subs    r9, r9, #1;                    				\
        bge     lnch_loop3;						\
        subs    r7, r7, #1;               				\
        bge     lnch_loop2;						\
lnch_skip:								\
        add     r10, r10, #2;          					\
        cmp     r3, r10;						\
        bgt     lnch_loop1;						\
lnch_fin:								\
        mov     r10, #0;                   				\
        mcr     p15, 2, r10, c0, c0, 0;     				\
        isb	;							\
        mrc     p15, 0, r0, c1, c0, 0; 					\
        bic     r0, r0, #CR_C;         					\
        mcr     p15, 0, r0, c1, c0, 0; 					\
        isb	;							\
        mrc     p15, 0, r0, c1, c0, 0; 					\
        bic     r0, r0, #CR_M; 						\
        mcr     p15, 0, r0, c1, c0, 0; 					\
        isb	;							\
        mrc     p15, 0, r0, c1, c0, 0; 					\
        bic     r0, r0, #CR_I;        					\
        bic     r0, r0, #CR_Z;        					\
        mcr     p15, 0, r0, c1, c0, 0; 					\
        isb	;


#define unlockJtag() 							\
	mov     r0,#(BROM_SEC_BASE & 0xff000000); 			\
	orr     r0,r0,#(BROM_SEC_BASE & 0x00ff0000);			\
	orr     r0,r0,#(BROM_SEC_BASE & 0x0000ff00);			\
	mov     r1,#BROM_SEC_SECBOOTCFG_JTAG_UNLOCK;			\
	str     r1,[r0,#BROM_SEC_SECBOOTCFG];

#endif /* BTRM_SUPPORT_H_ */

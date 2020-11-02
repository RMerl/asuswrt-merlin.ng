/*
 * HND mips trap handling.
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: hnd_mipstrap.h 708017 2017-06-29 14:11:45Z $
 */

#ifndef	_HND_MIPSTRAP_H
#define	_HND_MIPSTRAP_H

/* MIPS trap handling */

/* Trap locations in lo memory */
#define FIRST_TRAP	0
#define LAST_TRAP	0x480
#define	TRAP_STRIDE	0x80

/* Trap "type" is the trap location */

#define	TRAP_TYPE_SH	7
#define	MAX_TRAP_TYPE	((LAST_TRAP >> TRAP_TYPE_SH) + 1)

/* The trap structure is defined here as offsets for assembly */
#define	TR_TYPE		0x00
#define	TR_STATUS	0x04
#define	TR_CAUSE	0x08
#define	TR_EPC		0x0c
#define	TR_HI		0x10
#define	TR_LO		0x14
#define	TR_BVA		0x18
#define	TR_ERRPC	0x1c
#define	TR_REGS		0x20

#define	TRAP_T_SIZE	160

#ifndef	_LANGUAGE_ASSEMBLY

#include <typedefs.h>

typedef struct _trap_struct {
	uint32		type;
	uint32		status;
	uint32		cause;
	uint32		epc;
	uint32		hi;
	uint32		lo;
	uint32		badvaddr;
	uint32		errorpc;
	uint32		r0;
	uint32		r1;
	uint32		r2;
	uint32		r3;
	uint32		r4;
	uint32		r5;
	uint32		r6;
	uint32		r7;
	uint32		r8;
	uint32		r9;
	uint32		r10;
	uint32		r11;
	uint32		r12;
	uint32		r13;
	uint32		r14;
	uint32		r15;
	uint32		r16;
	uint32		r17;
	uint32		r18;
	uint32		r19;
	uint32		r20;
	uint32		r21;
	uint32		r22;
	uint32		r23;
	uint32		r24;
	uint32		r25;
	uint32		r26;
	uint32		r27;
	uint32		r28;
	uint32		r29;
	uint32		r30;
	uint32		r31;
} trap_t;

#endif	/* !_LANGUAGE_ASSEMBLY */
#endif	/* _HND_TRAP_H */

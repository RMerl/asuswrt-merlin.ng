/*
 * HND Run Time Environment MIPS specific.
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
 * $Id: rte_mips.h 474540 2014-05-01 18:40:15Z $
 */

#ifndef _rte_mips_h_
#define _rte_mips_h_

#include <mips33_core.h>
#include <mips74k_core.h>
#include <mipsinc.h>

/* register access macros */
#ifdef BCMSIM
extern uint32 rreg32(volatile uint32 *r);
extern uint16 rreg16(volatile uint16 *r);
extern uint8 rreg8(volatile uint8 *r);
extern void wreg32(volatile uint32 *r, uint32 v);
extern void wreg16(volatile uint16 *r, uint16 v);
extern void wreg8(volatile uint8 *r, uint8 v);
#else
#define wreg32(r, v)	(*(volatile uint32*)(r) = (uint32)(v))
#define rreg32(r)	(*(volatile uint32*)(r))
#ifdef IL_BIGENDIAN
#define wreg16(r, v)	(*(volatile uint16*)((ulong)(r)^2) = (uint16)(v))
#define rreg16(r)	(*(volatile uint16*)((ulong)(r)^2))
#define wreg8(r, v)	(*(volatile uint8*)((ulong)(r)^3) = (uint8)(v))
#define rreg8(r)	(*(volatile uint8*)((ulong)(r)^3))
#else
#define wreg16(r, v)	(*(volatile uint16*)(r) = (uint16)(v))
#define rreg16(r)	(*(volatile uint16*)(r))
#define wreg8(r, v)	(*(volatile uint8*)(r) = (uint8)(v))
#define rreg8(r)	(*(volatile uint8*)(r))
#endif // endif
#endif	/* BCMSIM */

/* uncached/cached virtual address */
#define	hnd_uncached(va)	((void *)KSEG1ADDR((ulong)(va)))
#define	hnd_cached(va)		((void *)KSEG0ADDR((ulong)(va)))

/* get processor cycle count */
#define osl_getcycles(x)	(2 * get_c0_count())

/* map/unmap physical to virtual I/O */
#ifdef BCMSIM
#define	hnd_reg_map(pa, size)	ioremap((ulong)(pa), (ulong)(size))
extern void *ioremap(ulong offset, ulong size);
#else
#define	hnd_reg_map(pa, size)	({BCM_REFERENCE(size); (void *)KSEG1ADDR((ulong)(pa));})
#endif	/* BCMSIM */
#define	hnd_reg_unmap(va)	BCM_REFERENCE(va)

/* map/unmap shared (dma-able) memory */
#define	hnd_dma_map(va, size) ({ \
	flush_dcache((uint32)va, size); \
	PHYSADDR((ulong)(va)); \
})
#define	hnd_dma_unmap(pa, size)	({BCM_REFERENCE(pa); BCM_REFERENCE(size);})

/* Cache support */
extern void caches_on(void);
extern void blast_dcache(void);
extern void blast_icache(void);
extern void flush_dcache(uint32 base, uint size);
extern void flush_icache(uint32 base, uint size);

#endif	/* _rte_mips_h_ */

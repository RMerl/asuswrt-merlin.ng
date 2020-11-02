/*
 * HND Run Time Environment ARM7TDMIs specific.
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
 * $Id: rte_arm.h 514393 2014-11-11 02:02:54Z $
 */

#ifndef _rte_arm_h_
#define _rte_arm_h_

#include <typedefs.h>
#include <sbhndarm.h>
#include <hndarm.h>

/* register access macros */
#define wreg32(r, v)		(*(volatile uint32 *)(r) = (uint32)(v))
#define rreg32(r)		(*(volatile uint32 *)(r))
#ifdef IL_BIGENDIAN
#define wreg16(r, v)		(*(volatile uint16 *)((uintptr)(r) ^ 2) = (uint16)(v))
#define rreg16(r)		(*(volatile uint16 *)((uintptr)(r) ^ 2))
#define wreg8(r, v)		(*(volatile uint8 *)((uintptr)(r) ^ 3) = (uint8)(v))
#define rreg8(r)		(*(volatile uint8 *)((uintptr)(r) ^ 3))
#else
#define wreg16(r, v)		(*(volatile uint16 *)(r) = (uint16)(v))
#define rreg16(r)		(*(volatile uint16 *)(r))
#define wreg8(r, v)		(*(volatile uint8 *)(r) = (uint8)(v))
#define rreg8(r)		(*(volatile uint8 *)(r))
#endif // endif

/* uncached/cached virtual address */
#define	hnd_uncached(va)	((void *)(va))
#define	hnd_cached(va)		((void *)(va))

/* host/bus architecture-specific address byte swap */
#define BUS_SWAP32(v)		(v)

/* get cycle counter */
#define	osl_getcycles		get_arm_cyclecount

/* map/unmap physical to virtual I/O */
#define	hnd_reg_map(pa, size)	({BCM_REFERENCE(size); (void *)(pa);})
#define	hnd_reg_unmap(va)	BCM_REFERENCE(va)

/* map/unmap shared (dma-able) memory */
#ifdef CONFIG_XIP
#define MEMORY_REMAP	(SI_ARM_SRAM2)
/*
 * arm bootloader memory is remapped but backplane addressing is 0-based
 *
 * Background: since the mask rom bootloader code executes in a
 * read-only memory space apart from SoC RAM, data addresses
 * specified by bootloader code must be decoded differently from
 * text addresses. Consequently, the processor must run in a
 * "memory remap" mode whereby data addresses containing the
 * MEMORY_REMAP bit are decoded as residing in SoC RAM. However,
 * backplane agents, e.g., the dma engines, always use 0-based
 * addresses for SoC RAM, regardless of processor mode.
 * Consequently it is necessary to strip the MEMORY_REMAP bit
 * from addresses programmed into backplane agents.
 */
#define	hnd_dma_map(va, size)	({BCM_REFERENCE(size); ((uint32)va & ~MEMORY_REMAP);})
#else
#define	hnd_dma_map(va, size)	({BCM_REFERENCE(size); va;})
#endif /* CONFIG_XIP */
#define	hnd_dma_unmap(pa, size)	({BCM_REFERENCE(pa); BCM_REFERENCE(size);})

/* Cache support (or lack thereof) */
#if defined(__ARM_ARCH_7R__) && defined(RTE_CACHED)
extern void caches_on(void);
#else
static inline void caches_on(void) { return; }
#endif /* defined(__ARM_ARCH_7R__) && defined(RTE_CACHED) */

static inline void blast_dcache(void) { return; }
static inline void blast_icache(void) { return; }
static inline void flush_dcache(uint32 base, uint size) { return; }
static inline void flush_icache(uint32 base, uint size) { return; }

#endif	/* _rte_arm_h_ */

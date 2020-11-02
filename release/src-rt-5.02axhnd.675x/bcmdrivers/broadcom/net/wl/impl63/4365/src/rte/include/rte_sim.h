/*
 * HND Run Time Environment Simulation specific.
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
 * $Id: rte_sim.h 474540 2014-05-01 18:40:15Z $
 */

#ifndef _rte_sim_h_
#define _rte_sim_h_

#include <stddef.h>

/* uncached/cached virtual address */
#define	hnd_uncached(va)	((void *)(va))
#define	hnd_cached(va)		((void *)(va))

/* map/unmap physical to virtual I/O */
#define	hnd_reg_map(pa, size)	({BCM_REFERENCE(size); (void *)(pa);})
#define	hnd_reg_unmap(va)	BCM_REFERENCE(va)

/* map/unmap shared (dma-able) memory */
#define	hnd_dma_map(va, size)	({BCM_REFERENCE(size); virt_to_phys(va);})
#define	hnd_dma_unmap(pa, size)	({BCM_REFERENCE(pa); BCM_REFERENCE(size);})

extern uint32 rreg32(volatile uint32 *r);
extern uint16 rreg16(volatile uint16 *r);
extern uint8 rreg8(volatile uint8 *r);
extern void wreg32(volatile uint32 *r, uint32 v);
extern void wreg16(volatile uint16 *r, uint16 v);
extern void wreg8(volatile uint8 *r, uint8 v);

#ifdef ARM7TIMESTAMPS

#define TIMER_BASE			0x0a800000

#define CURRENT_TIME()		(*((volatile uint32*)(TIMER_BASE + 0x04)) & 0xFFFF)
#define ENABLE_TIMER()		do { (*((volatile uint32*)(TIMER_BASE + 0x08))) = 0x80;} while (0)
#define DISABLE_TIMER()		do { (*((volatile uint32*)(TIMER_BASE + 0x08))) = 0x00;}.while (0)

#define PROF_START()		do { __gcycles = 0; __gtimer = CURRENT_TIME();} while (0)
#define PROF_SUSPEND()		(__gcycles += ((__gtimer - CURRENT_TIME()) & 0xFFFF) - __gcaladj)
#define PROF_RESUME()		(__gtimer = CURRENT_TIME())
#define PROF_INTERIM(tag)					\
		do {						\
			PROF_SUSPEND(); 			\
			printf("Interim profile %s, %lu cycles\n", \
				tag, __gcycles);		\
			PROF_RESUME();				\
		} while (0)
#define PROF_FINISH(tag)					\
		do {						\
			PROF_SUSPEND();				\
			printf("Final profile %s, %lu cycles\n", \
				tag, __gcycles);		\
		} while (0)

#endif /* ARM7TIMESTAMPS */

#ifdef CORTEXM3TIMESTAMPS

#define TIMER_BASE			0xe000e010

#define CURRENT_TIME()		curr_time()

extern uint32 curr_time();

#define ENABLE_TIMER()						\
		do {						\
			unsigned int volatile			\
			*p = (unsigned int volatile *)0xe000e010; \
			p[0] = 0x00000000;			\
			p[1] = 0x00FFFFFF;			\
			p[2] = 0x00000000;			\
			p[0] = 0x00000005;			\
		} while (0)

#define DISABLE_TIMER()						\
		do {						\
			unsigned int volatile			\
			*p = (unsigned int volatile *)0xe000e010; \
			unsigned int a = p[0];			\
			if (a & 0x00010000)			\
				printf("ERROR: CortexM3 time-stamp overflowed\n"); \
			else					\
				p[0] = 0x00000000;		\
		} while (0)

/* Enable the timer everytime you start profiling because its only
 * 24bits counter and not 32 bits, will get over in < 1sec for 80MHz clock
 */
#define PROF_START()						\
		do {						\
			ENABLE_TIMER();				\
			__gcycles = 0;				\
			__gtimer = CURRENT_TIME();		\
		} while (0)
#define PROF_SUSPEND()						\
		(__gcycles +=					\
			((__gtimer - CURRENT_TIME()) &		\
			0xFFFFFF) - __gcaladj)
#define PROF_RESUME()			(__gtimer = CURRENT_TIME())
#define PROF_INTERIM(tag)					\
		do {						\
			PROF_SUSPEND();				\
			printf("Interim profile %s, %lu cycles\n", \
				tag, __gcycles);		\
			PROF_RESUME();				\
		} while (0)
#define PROF_FINISH(tag)					\
		do {						\
			PROF_SUSPEND();				\
			printf("Final profile %s, %lu cycles\n", \
				tag, __gcycles);		\
		} while (0)

#endif /* CORTEXM3TIMESTAMPS */

extern uint32 __gcycles, __gtimer, __gcaladj;

/* Cache support */
static inline void caches_on(void) { return; };
static inline void blast_dcache(void) { return; };
static inline void blast_icache(void) { return; };
static inline void flush_dcache(uint32 base, uint size) { return; };
static inline void flush_icache(uint32 base, uint size) { return; };

#endif	/* _rte_sim_h_ */

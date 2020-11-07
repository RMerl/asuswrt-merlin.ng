/*
 * HND SiliconBackplane MIPS/ARM cores software interface.
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: hndcpu.h 669161 2016-11-08 08:04:36Z $
 */

#ifndef _hndcpu_h_
#define _hndcpu_h_

#if defined(mips)
#include <hndmips.h>
#elif defined(__arm__) || defined(__thumb__) || defined(__thumb2__)
#include <hndarm.h>
#endif // endif

extern uint si_irq(si_t *sih);
extern uint si_irq_alt(si_t *sih);
extern uint32 si_cpu_clock(si_t *sih);
extern uint32 si_mem_clock(si_t *sih);
extern void hnd_cpu_wait(si_t *sih);
extern void hnd_cpu_jumpto(void *addr);
extern void hnd_cpu_reset(si_t *sih);
#if defined(BCMDBG_LOADAVG) && defined(__ARM_ARCH_7R__)
extern void hnd_cpu_loadavg_timer(si_t *sih, uint32 val);
#else
extern void hnd_cpu_deadman_timer(si_t *sih, uint32 val);
#endif // endif
#ifdef __ARM_ARCH_7A__
extern void hnd_hw_coherent_enable(si_t *sih);
extern void hnd_cpu_enable_gtimer(si_t *sih);
extern uint32 hnd_cpu_gtimer_clock(si_t *sih);
#endif	/* __ARM_ARCH_7A__ */
extern void si_dmc_phyctl(si_t *sih, uint32 phyctl_val);
extern uint32 si_arm_sflags(si_t *sih);

#endif /* _hndcpu_h_ */

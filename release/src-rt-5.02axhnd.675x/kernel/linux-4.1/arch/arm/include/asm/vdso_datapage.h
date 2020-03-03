/*
 * Adapted from arm64 version.
 *
 * Copyright (C) 2012 ARM Limited
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __ASM_VDSO_DATAPAGE_H
#define __ASM_VDSO_DATAPAGE_H

#ifdef __KERNEL__

#ifndef __ASSEMBLY__

#include <asm/page.h>

/* Try to be cache-friendly on systems that don't implement the
 * generic timer: fit the unconditionally updated fields in the first
 * 32 bytes.
 */
struct vdso_data {
	u32 seq_count;		/* sequence count - odd during updates */
	u16 tk_is_cntvct;	/* fall back to syscall if false */
	u16 cs_shift;		/* clocksource shift */
	u32 xtime_coarse_sec;	/* coarse time */
	u32 xtime_coarse_nsec;

	u32 wtm_clock_sec;	/* wall to monotonic offset */
	u32 wtm_clock_nsec;
	u32 xtime_clock_sec;	/* CLOCK_REALTIME - seconds */
	u32 cs_mult;		/* clocksource multiplier */

	u64 cs_cycle_last;	/* last cycle value */
	u64 cs_mask;		/* clocksource mask */

	u64 xtime_clock_snsec;	/* CLOCK_REALTIME sub-ns base */
	u32 tz_minuteswest;	/* timezone info for gettimeofday(2) */
	u32 tz_dsttime;
};

union vdso_data_store {
	struct vdso_data data;
	u8 page[PAGE_SIZE];
};

#endif /* !__ASSEMBLY__ */

#endif /* __KERNEL__ */

#endif /* __ASM_VDSO_DATAPAGE_H */

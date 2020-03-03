/*
 * Copyright (C) 2012 ARM Ltd.
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
#ifndef __CLKSOURCE_ARM_ARCH_TIMER_H
#define __CLKSOURCE_ARM_ARCH_TIMER_H

#include <linux/timecounter.h>
#include <linux/types.h>

#define ARCH_TIMER_CTRL_ENABLE		(1 << 0)
#define ARCH_TIMER_CTRL_IT_MASK		(1 << 1)
#define ARCH_TIMER_CTRL_IT_STAT		(1 << 2)

enum arch_timer_reg {
	ARCH_TIMER_REG_CTRL,
	ARCH_TIMER_REG_TVAL,
};

#define ARCH_TIMER_PHYS_ACCESS		0
#define ARCH_TIMER_VIRT_ACCESS		1
#define ARCH_TIMER_MEM_PHYS_ACCESS	2
#define ARCH_TIMER_MEM_VIRT_ACCESS	3

#define ARCH_TIMER_USR_PCT_ACCESS_EN	(1 << 0) /* physical counter */
#define ARCH_TIMER_USR_VCT_ACCESS_EN	(1 << 1) /* virtual counter */
#define ARCH_TIMER_VIRT_EVT_EN		(1 << 2)
#define ARCH_TIMER_EVT_TRIGGER_SHIFT	(4)
#define ARCH_TIMER_EVT_TRIGGER_MASK	(0xF << ARCH_TIMER_EVT_TRIGGER_SHIFT)
#define ARCH_TIMER_USR_VT_ACCESS_EN	(1 << 8) /* virtual timer registers */
#define ARCH_TIMER_USR_PT_ACCESS_EN	(1 << 9) /* physical timer registers */

#define ARCH_TIMER_EVT_STREAM_FREQ	10000	/* 100us */

#ifdef CONFIG_ARM_ARCH_TIMER

extern u32 arch_timer_get_rate(void);
extern u64 (*arch_timer_read_counter)(void);
extern struct timecounter *arch_timer_get_timecounter(void);

#else

static inline u32 arch_timer_get_rate(void)
{
	return 0;
}

static inline u64 arch_timer_read_counter(void)
{
	return 0;
}

static inline struct timecounter *arch_timer_get_timecounter(void)
{
	return NULL;
}

#endif

#endif

/*
 * Copyright (C) 2010 Google, Inc.
 * Copyright (c) 2010-2012 NVIDIA Corporation. All rights reserved.
 *
 * Author:
 *	Colin Cross <ccross@google.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _MACH_TEGRA_PM_H_
#define _MACH_TEGRA_PM_H_

struct tegra_lp1_iram {
	void	*start_addr;
	void	*end_addr;
};

extern struct tegra_lp1_iram tegra_lp1_iram;
extern void (*tegra_sleep_core_finish)(unsigned long v2p);

void tegra20_lp1_iram_hook(void);
void tegra20_sleep_core_init(void);
void tegra30_lp1_iram_hook(void);
void tegra30_sleep_core_init(void);

void tegra_clear_cpu_in_lp2(void);
bool tegra_set_cpu_in_lp2(void);

void tegra_idle_lp2_last(void);
extern void (*tegra_tear_down_cpu)(void);

#ifdef CONFIG_PM_SLEEP
void tegra_init_suspend(void);
#else
static inline void tegra_init_suspend(void) {}
#endif

#endif /* _MACH_TEGRA_PM_H_ */

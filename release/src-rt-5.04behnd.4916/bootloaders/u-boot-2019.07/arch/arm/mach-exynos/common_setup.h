/*
 * Common APIs for EXYNOS based board
 *
 * Copyright (C) 2013 Samsung Electronics
 * Rajeshwari Shinde <rajeshwari.s@samsung.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <asm/arch/system.h>

#define DMC_OFFSET	0x10000

/*
 * Memory initialization
 *
 * @param reset     Reset PHY during initialization.
 */
void mem_ctrl_init(int reset);

 /* System Clock initialization */
void system_clock_init(void);

/*
 * Init subsystems according to the reset status
 *
 * @return 0 for a normal boot, non-zero for a resume
 */
int do_lowlevel_init(void);

void sdelay(unsigned long);

enum l2_cache_params {
	CACHE_DATA_RAM_LATENCY_2_CYCLES = (2 << 0),
	CACHE_DATA_RAM_LATENCY_3_CYCLES = (3 << 0),
	CACHE_DISABLE_CLEAN_EVICT = (1 << 3),
	CACHE_DATA_RAM_SETUP = (1 << 5),
	CACHE_TAG_RAM_LATENCY_2_CYCLES = (2 << 6),
	CACHE_TAG_RAM_LATENCY_3_CYCLES = (3 << 6),
	CACHE_ENABLE_HAZARD_DETECT = (1 << 7),
	CACHE_TAG_RAM_SETUP = (1 << 9),
	CACHE_ECC_AND_PARITY = (1 << 21),
	CACHE_ENABLE_FORCE_L2_LOGIC = (1 << 27)
};


#if !defined(CONFIG_SYS_L2CACHE_OFF) && defined(CONFIG_EXYNOS5420)
/*
 * Configure L2CTLR to get timings that keep us from hanging/crashing.
 *
 * Must be inline here since low_power_start() is called without a
 * stack (!).
 */
static inline void configure_l2_ctlr(void)
{
	uint32_t val;

	mrc_l2_ctlr(val);

	val |= CACHE_TAG_RAM_SETUP |
		CACHE_DATA_RAM_SETUP |
		CACHE_TAG_RAM_LATENCY_2_CYCLES |
		CACHE_DATA_RAM_LATENCY_2_CYCLES;

	if (proid_is_exynos542x()) {
		val |= CACHE_ECC_AND_PARITY |
			CACHE_TAG_RAM_LATENCY_3_CYCLES |
			CACHE_DATA_RAM_LATENCY_3_CYCLES;
	}

	mcr_l2_ctlr(val);
}

/*
 * Configure L2ACTLR.
 *
 * Must be inline here since low_power_start() is called without a
 * stack (!).
 */
static inline void configure_l2_actlr(void)
{
	uint32_t val;

	if (proid_is_exynos542x()) {
		mrc_l2_aux_ctlr(val);
		val |= CACHE_ENABLE_FORCE_L2_LOGIC |
			CACHE_DISABLE_CLEAN_EVICT;
		mcr_l2_aux_ctlr(val);
	}
}
#endif

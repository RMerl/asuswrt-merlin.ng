// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2004 Freescale Semiconductor.
 * Jeff Brown
 * Srikanth Srinivasan (srikanth.srinivasan@freescale.com)
 *
 * (C) Copyright 2000-2002
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#include <common.h>
#include <mpc86xx.h>
#include <asm/processor.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/* used in some defintiions of CONFIG_SYS_CLK_FREQ */
extern unsigned long get_board_sys_clk(unsigned long dummy);

void get_sys_info(sys_info_t *sys_info)
{
	volatile immap_t *immap = (immap_t *) CONFIG_SYS_IMMR;
	volatile ccsr_gur_t *gur = &immap->im_gur;
	uint plat_ratio, e600_ratio;

	plat_ratio = (gur->porpllsr) & 0x0000003e;
	plat_ratio >>= 1;

	switch (plat_ratio) {
	case 0x0:
		sys_info->freq_systembus = 16 * CONFIG_SYS_CLK_FREQ;
		break;
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x05:
	case 0x06:
	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0c:
	case 0x10:
		sys_info->freq_systembus = plat_ratio * CONFIG_SYS_CLK_FREQ;
		break;
	default:
		sys_info->freq_systembus = 0;
		break;
	}

	e600_ratio = (gur->porpllsr) & 0x003f0000;
	e600_ratio >>= 16;

	switch (e600_ratio) {
	case 0x10:
		sys_info->freq_processor = 2 * sys_info->freq_systembus;
		break;
	case 0x19:
		sys_info->freq_processor = 5 * sys_info->freq_systembus / 2;
		break;
	case 0x20:
		sys_info->freq_processor = 3 * sys_info->freq_systembus;
		break;
	case 0x39:
		sys_info->freq_processor = 7 * sys_info->freq_systembus / 2;
		break;
	case 0x28:
		sys_info->freq_processor = 4 * sys_info->freq_systembus;
		break;
	case 0x1d:
		sys_info->freq_processor = 9 * sys_info->freq_systembus / 2;
		break;
	default:
		sys_info->freq_processor = e600_ratio +
						sys_info->freq_systembus;
		break;
	}

	sys_info->freq_localbus = sys_info->freq_systembus;
}


/*
 * Measure CPU clock speed (core clock GCLK1, GCLK2)
 * (Approx. GCLK frequency in Hz)
 */

int get_clocks(void)
{
	sys_info_t sys_info;

	get_sys_info(&sys_info);
	gd->cpu_clk = sys_info.freq_processor;
	gd->bus_clk = sys_info.freq_systembus;
	gd->arch.lbc_clk = sys_info.freq_localbus;

	/*
	 * The base clock for I2C depends on the actual SOC.  Unfortunately,
	 * there is no pattern that can be used to determine the frequency, so
	 * the only choice is to look up the actual SOC number and use the value
	 * for that SOC. This information is taken from application note
	 * AN2919.
	 */
#ifdef CONFIG_ARCH_MPC8610
	gd->arch.i2c1_clk = sys_info.freq_systembus;
#else
	gd->arch.i2c1_clk = sys_info.freq_systembus / 2;
#endif
	gd->arch.i2c2_clk = gd->arch.i2c1_clk;

	if (gd->cpu_clk != 0)
		return 0;
	else
		return 1;
}


/*
 * get_bus_freq
 *	Return system bus freq in Hz
 */

ulong get_bus_freq(ulong dummy)
{
	ulong val;
	sys_info_t sys_info;

	get_sys_info(&sys_info);
	val = sys_info.freq_systembus;

	return val;
}

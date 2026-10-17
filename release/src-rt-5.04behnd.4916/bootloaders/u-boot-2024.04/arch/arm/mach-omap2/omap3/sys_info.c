// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Texas Instruments, <www.ti.com>
 *
 * Author :
 *      Manikandan Pillai <mani.pillai@ti.com>
 *
 * Derived from Beagle Board and 3430 SDP code by
 *      Richard Woodruff <r-woodruff2@ti.com>
 *      Syed Mohammed Khasim <khasim@ti.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/mem.h>	/* get mem tables */
#include <asm/arch/sys_proto.h>
#include <asm/bootm.h>
#include <asm/omap_common.h>

#include <i2c.h>
#include <linux/compiler.h>

extern omap3_sysinfo sysinfo;
static struct ctrl *ctrl_base = (struct ctrl *)OMAP34XX_CTRL_BASE;

#ifdef CONFIG_DISPLAY_CPUINFO
static char *rev_s[CPU_3XX_MAX_REV] = {
				"1.0",
				"2.0",
				"2.1",
				"3.0",
				"3.1",
				"UNKNOWN",
				"UNKNOWN",
				"3.1.2"};

/* this is the revision table for 37xx CPUs */
static char *rev_s_37xx[CPU_37XX_MAX_REV] = {
				"1.0",
				"1.1",
				"1.2"};
#endif /* CONFIG_DISPLAY_CPUINFO */

void omap_die_id(unsigned int *die_id)
{
	struct ctrl_id *id_base = (struct ctrl_id *)OMAP34XX_ID_L4_IO_BASE;

	die_id[0] = readl(&id_base->die_id_0);
	die_id[1] = readl(&id_base->die_id_1);
	die_id[2] = readl(&id_base->die_id_2);
	die_id[3] = readl(&id_base->die_id_3);
}

/******************************************
 * get_cpu_type(void) - extract cpu info
 ******************************************/
static u32 get_cpu_type(void)
{
	return readl(&ctrl_base->ctrl_omap_stat);
}

/******************************************
 * get_cpu_id(void) - extract cpu id
 * returns 0 for ES1.0, cpuid otherwise
 ******************************************/
static u32 get_cpu_id(void)
{
	struct ctrl_id *id_base;
	u32 cpuid = 0;

	/*
	 * On ES1.0 the IDCODE register is not exposed on L4
	 * so using CPU ID to differentiate between ES1.0 and > ES1.0.
	 */
	__asm__ __volatile__("mrc p15, 0, %0, c0, c0, 0":"=r"(cpuid));
	if ((cpuid & 0xf) == 0x0) {
		return 0;
	} else {
		/* Decode the IDs on > ES1.0 */
		id_base = (struct ctrl_id *) OMAP34XX_ID_L4_IO_BASE;

		cpuid = readl(&id_base->idcode);
	}

	return cpuid;
}

/******************************************
 * get_cpu_family(void) - extract cpu info
 ******************************************/
__used u32 get_cpu_family(void)
{
	u16 hawkeye;
	u32 cpu_family;
	u32 cpuid = get_cpu_id();

	if (cpuid == 0)
		return CPU_OMAP34XX;

	hawkeye = (cpuid >> HAWKEYE_SHIFT) & 0xffff;
	switch (hawkeye) {
	case HAWKEYE_OMAP34XX:
		cpu_family = CPU_OMAP34XX;
		break;
	case HAWKEYE_AM35XX:
		cpu_family = CPU_AM35XX;
		break;
	case HAWKEYE_OMAP36XX:
		cpu_family = CPU_OMAP36XX;
		break;
	default:
		cpu_family = CPU_OMAP34XX;
	}

	return cpu_family;
}

/******************************************
 * get_cpu_rev(void) - extract version info
 ******************************************/
__used u32 get_cpu_rev(void)
{
	u32 cpuid = get_cpu_id();

	if (cpuid == 0)
		return CPU_3XX_ES10;
	else
		return (cpuid >> CPU_3XX_ID_SHIFT) & 0xf;
}

/*****************************************************************
 * get_sku_id(void) - read sku_id to get info on max clock rate
 *****************************************************************/
static u32 get_sku_id(void)
{
	struct ctrl_id *id_base = (struct ctrl_id *)OMAP34XX_ID_L4_IO_BASE;
	return readl(&id_base->sku_id) & SKUID_CLK_MASK;
}

/*************************************************************************
 * get_board_rev() - setup to pass kernel board revision information
 * returns:(bit[0-3] sub version, higher bit[7-4] is higher version)
 *************************************************************************/
#ifdef CONFIG_REVISION_TAG
u32 __weak get_board_rev(void)
{
	return 0x20;
}
#endif

/********************************************************
 *  get_base(); get upper addr of current execution
 *******************************************************/
static u32 get_base(void)
{
	u32 val;

	__asm__ __volatile__("mov %0, pc \n":"=r"(val)::"memory");
	val &= 0xF0000000;
	val >>= 28;
	return val;
}

/********************************************************
 *  is_running_in_flash() - tell if currently running in
 *  FLASH.
 *******************************************************/
u32 is_running_in_flash(void)
{
	if (get_base() < 4)
		return 1;	/* in FLASH */

	return 0;		/* running in SRAM or SDRAM */
}

/********************************************************
 *  is_running_in_sram() - tell if currently running in
 *  SRAM.
 *******************************************************/
u32 is_running_in_sram(void)
{
	if (get_base() == 4)
		return 1;	/* in SRAM */

	return 0;		/* running in FLASH or SDRAM */
}

/********************************************************
 *  is_running_in_sdram() - tell if currently running in
 *  SDRAM.
 *******************************************************/
u32 is_running_in_sdram(void)
{
	if (get_base() > 4)
		return 1;	/* in SDRAM */

	return 0;		/* running in SRAM or FLASH */
}

/***************************************************************
 *  get_boot_type() - Is this an XIP type device or a stream one
 *  bits 4-0 specify type. Bit 5 says mem/perif
 ***************************************************************/
u32 get_boot_type(void)
{
	return (readl(&ctrl_base->status) & SYSBOOT_MASK);
}

#ifdef CONFIG_DISPLAY_CPUINFO
/**
 * Print CPU information
 */
int print_cpuinfo (void)
{
	char *cpu_family_s, *cpu_s, *sec_s, *max_clk;

	switch (get_cpu_family()) {
	case CPU_OMAP34XX:
		cpu_family_s = "OMAP";
		switch (get_cpu_type()) {
		case OMAP3503:
			cpu_s = "3503";
			break;
		case OMAP3515:
			cpu_s = "3515";
			break;
		case OMAP3525:
			cpu_s = "3525";
			break;
		case OMAP3530:
			cpu_s = "3530";
			break;
		default:
			cpu_s = "35XX";
			break;
		}
		if ((get_cpu_rev() >= CPU_3XX_ES31) &&
		    (get_sku_id() == SKUID_CLK_720MHZ))
			max_clk = "720 MHz";
		else
			max_clk = "600 MHz";

		break;
	case CPU_AM35XX:
		cpu_family_s = "AM";
		switch (get_cpu_type()) {
		case AM3505:
			cpu_s = "3505";
			break;
		case AM3517:
			cpu_s = "3517";
			break;
		default:
			cpu_s = "35XX";
			break;
		}
		max_clk = "600 MHz";
		break;
	case CPU_OMAP36XX:
		switch (get_cpu_type()) {
		case AM3703:
			cpu_family_s = "AM";
			cpu_s = "3703";
			max_clk = "800 MHz";
			break;
		case AM3703_1GHZ:
			cpu_family_s = "AM";
			cpu_s = "3703";
			max_clk = "1 GHz";
			break;
		case AM3715:
			cpu_family_s = "AM";
			cpu_s = "3715";
			max_clk = "800 MHz";
			break;
		case AM3715_1GHZ:
			cpu_family_s = "AM";
			cpu_s = "3715";
			max_clk = "1 GHz";
			break;
		case OMAP3725:
			cpu_family_s = "OMAP";
			cpu_s = "3625/3725";
			max_clk = "800 MHz";
			break;
		case OMAP3725_1GHZ:
			cpu_family_s = "OMAP";
			cpu_s = "3625/3725";
			max_clk = "1 GHz";
			break;
		case OMAP3730:
			cpu_family_s = "OMAP";
			cpu_s = "3630/3730";
			max_clk = "800 MHz";
			break;
		case OMAP3730_1GHZ:
			cpu_family_s = "OMAP";
			cpu_s = "3630/3730";
			max_clk = "1 GHz";
			break;
		default:
			cpu_family_s = "OMAP/AM";
			cpu_s = "36XX/37XX";
			max_clk = "1 GHz";
			break;
		}

		break;
	default:
		cpu_family_s = "OMAP";
		cpu_s = "35XX";
		max_clk = "600 MHz";
	}

	switch (get_device_type()) {
	case TST_DEVICE:
		sec_s = "TST";
		break;
	case EMU_DEVICE:
		sec_s = "EMU";
		break;
	case HS_DEVICE:
		sec_s = "HS";
		break;
	case GP_DEVICE:
		sec_s = "GP";
		break;
	default:
		sec_s = "?";
	}

	if (CPU_OMAP36XX == get_cpu_family())
		printf("%s%s-%s ES%s, CPU-OPP2, L3-200MHz, Max CPU Clock %s\n",
		       cpu_family_s, cpu_s, sec_s,
		       rev_s_37xx[get_cpu_rev()], max_clk);
	else
		printf("%s%s-%s ES%s, CPU-OPP2, L3-165MHz, Max CPU Clock %s\n",
			cpu_family_s, cpu_s, sec_s,
			rev_s[get_cpu_rev()], max_clk);

	return 0;
}
#endif	/* CONFIG_DISPLAY_CPUINFO */

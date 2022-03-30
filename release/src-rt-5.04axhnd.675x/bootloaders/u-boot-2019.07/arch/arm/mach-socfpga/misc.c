// SPDX-License-Identifier: GPL-2.0+
/*
 *  Copyright (C) 2012-2017 Altera Corporation <www.altera.com>
 */

#include <common.h>
#include <asm/io.h>
#include <errno.h>
#include <fdtdec.h>
#include <linux/libfdt.h>
#include <altera.h>
#include <miiphy.h>
#include <netdev.h>
#include <watchdog.h>
#include <asm/arch/misc.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/scan_manager.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/nic301.h>
#include <asm/arch/scu.h>
#include <asm/pl310.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SYS_L2_PL310
static const struct pl310_regs *const pl310 =
	(struct pl310_regs *)CONFIG_SYS_PL310_BASE;
#endif

struct bsel bsel_str[] = {
	{ "rsvd", "Reserved", },
	{ "fpga", "FPGA (HPS2FPGA Bridge)", },
	{ "nand", "NAND Flash (1.8V)", },
	{ "nand", "NAND Flash (3.0V)", },
	{ "sd", "SD/MMC External Transceiver (1.8V)", },
	{ "sd", "SD/MMC Internal Transceiver (3.0V)", },
	{ "qspi", "QSPI Flash (1.8V)", },
	{ "qspi", "QSPI Flash (3.0V)", },
};

int dram_init(void)
{
	if (fdtdec_setup_mem_size_base() != 0)
		return -EINVAL;

	return 0;
}

void enable_caches(void)
{
#if !CONFIG_IS_ENABLED(SYS_ICACHE_OFF)
	icache_enable();
#endif
#if !CONFIG_IS_ENABLED(SYS_DCACHE_OFF)
	dcache_enable();
#endif
}

#ifdef CONFIG_SYS_L2_PL310
void v7_outer_cache_enable(void)
{
	struct udevice *dev;

	if (uclass_get_device(UCLASS_CACHE, 0, &dev))
		pr_err("cache controller driver NOT found!\n");
}

void v7_outer_cache_disable(void)
{
	/* Disable the L2 cache */
	clrbits_le32(&pl310->pl310_ctrl, L2X0_CTRL_EN);
}

void socfpga_pl310_clear(void)
{
	u32 mask = 0xff, ena = 0;

	icache_enable();

	/* Disable the L2 cache */
	clrbits_le32(&pl310->pl310_ctrl, L2X0_CTRL_EN);

	writel(0x0, &pl310->pl310_tag_latency_ctrl);
	writel(0x10, &pl310->pl310_data_latency_ctrl);

	/* enable BRESP, instruction and data prefetch, full line of zeroes */
	setbits_le32(&pl310->pl310_aux_ctrl,
		     L310_AUX_CTRL_DATA_PREFETCH_MASK |
		     L310_AUX_CTRL_INST_PREFETCH_MASK |
		     L310_SHARED_ATT_OVERRIDE_ENABLE);

	/* Enable the L2 cache */
	ena = readl(&pl310->pl310_ctrl);
	ena |= L2X0_CTRL_EN;

	/*
	 * Invalidate the PL310 L2 cache. Keep the invalidation code
	 * entirely in L1 I-cache to avoid any bus traffic through
	 * the L2.
	 */
	asm volatile(
		".align	5			\n"
		"	b	3f		\n"
		"1:	str	%1,	[%4]	\n"
		"	dsb			\n"
		"	isb			\n"
		"	str	%0,	[%2]	\n"
		"	dsb			\n"
		"	isb			\n"
		"2:	ldr	%0,	[%2]	\n"
		"	cmp	%0,	#0	\n"
		"	bne	2b		\n"
		"	str	%0,	[%3]	\n"
		"	dsb			\n"
		"	isb			\n"
		"	b	4f		\n"
		"3:	b	1b		\n"
		"4:	nop			\n"
	: "+r"(mask), "+r"(ena)
	: "r"(&pl310->pl310_inv_way),
	  "r"(&pl310->pl310_cache_sync), "r"(&pl310->pl310_ctrl)
	: "memory", "cc");

	/* Disable the L2 cache */
	clrbits_le32(&pl310->pl310_ctrl, L2X0_CTRL_EN);
}
#endif

#if defined(CONFIG_SYS_CONSOLE_IS_IN_ENV) && \
defined(CONFIG_SYS_CONSOLE_OVERWRITE_ROUTINE)
int overwrite_console(void)
{
	return 0;
}
#endif

#ifdef CONFIG_FPGA
/* add device descriptor to FPGA device table */
void socfpga_fpga_add(void *fpga_desc)
{
	fpga_init();
	fpga_add(fpga_altera, fpga_desc);
}
#endif

int arch_cpu_init(void)
{
#ifdef CONFIG_HW_WATCHDOG
	/*
	 * In case the watchdog is enabled, make sure to (re-)configure it
	 * so that the defined timeout is valid. Otherwise the SPL (Perloader)
	 * timeout value is still active which might too short for Linux
	 * booting.
	 */
	hw_watchdog_init();
#else
	/*
	 * If the HW watchdog is NOT enabled, make sure it is not running,
	 * for example because it was enabled in the preloader. This might
	 * trigger a watchdog-triggered reboot of Linux kernel later.
	 * Toggle watchdog reset, so watchdog in not running state.
	 */
	socfpga_per_reset(SOCFPGA_RESET(L4WD0), 1);
	socfpga_per_reset(SOCFPGA_RESET(L4WD0), 0);
#endif

	return 0;
}

#ifndef CONFIG_SPL_BUILD
static int do_bridge(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned int mask = ~0;

	if (argc < 2 || argc > 3)
		return CMD_RET_USAGE;

	argv++;

	if (argc == 3)
		mask = simple_strtoul(argv[1], NULL, 16);

	switch (*argv[0]) {
	case 'e':	/* Enable */
		do_bridge_reset(1, mask);
		break;
	case 'd':	/* Disable */
		do_bridge_reset(0, mask);
		break;
	default:
		return CMD_RET_USAGE;
	}

	return 0;
}

U_BOOT_CMD(bridge, 3, 1, do_bridge,
	   "SoCFPGA HPS FPGA bridge control",
	   "enable [mask] - Enable HPS-to-FPGA, FPGA-to-HPS, LWHPS-to-FPGA bridges\n"
	   "bridge disable [mask] - Enable HPS-to-FPGA, FPGA-to-HPS, LWHPS-to-FPGA bridges\n"
	   ""
);

#endif

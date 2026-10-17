// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Intel Corporation <www.intel.com>
 *
 */

#include <init.h>
#include <log.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/u-boot.h>
#include <asm/utils.h>
#include <common.h>
#include <hang.h>
#include <image.h>
#include <spl.h>
#include <asm/arch/clock_manager.h>
#include <asm/arch/firewall.h>
#include <asm/arch/mailbox_s10.h>
#include <asm/arch/misc.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>
#include <watchdog.h>
#include <dm/uclass.h>

DECLARE_GLOBAL_DATA_PTR;

void board_init_f(ulong dummy)
{
	int ret;
	struct udevice *dev;

	ret = spl_early_init();
	if (ret)
		hang();

	socfpga_get_managers_addr();

	/* Ensure watchdog is paused when debugging is happening */
	writel(SYSMGR_WDDBG_PAUSE_ALL_CPU,
	       socfpga_get_sysmgr_addr() + SYSMGR_SOC64_WDDBG);

#ifdef CONFIG_HW_WATCHDOG
	/* Enable watchdog before initializing the HW */
	socfpga_per_reset(SOCFPGA_RESET(L4WD0), 1);
	socfpga_per_reset(SOCFPGA_RESET(L4WD0), 0);
	hw_watchdog_init();
#endif

	/* ensure all processors are not released prior Linux boot */
	writeq(0, CPU_RELEASE_ADDR);

	timer_init();

	sysmgr_pinmux_init();

	ret = uclass_get_device(UCLASS_CLK, 0, &dev);
	if (ret) {
		debug("Clock init failed: %d\n", ret);
		hang();
	}

	preloader_console_init();
	print_reset_info();
	cm_print_clock_quick_summary();

	firewall_setup();
	ret = uclass_get_device(UCLASS_CACHE, 0, &dev);
	if (ret) {
		debug("CCU init failed: %d\n", ret);
		hang();
	}

#if CONFIG_IS_ENABLED(ALTERA_SDRAM)
	ret = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (ret) {
		debug("DRAM init failed: %d\n", ret);
		hang();
	}
#endif

	mbox_init();

#ifdef CONFIG_CADENCE_QSPI
	mbox_qspi_open();
#endif
}

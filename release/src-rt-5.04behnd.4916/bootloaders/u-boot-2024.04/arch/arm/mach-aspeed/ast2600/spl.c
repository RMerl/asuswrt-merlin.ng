// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) Aspeed Technology Inc.
 */
#include <common.h>
#include <debug_uart.h>
#include <dm.h>
#include <spl.h>
#include <init.h>
#include <linux/err.h>
#include <asm/io.h>
#include <asm/arch/scu_ast2600.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

void board_init_f(ulong dummy)
{
	spl_early_init();
	preloader_console_init();
	timer_init();
	dram_init();
}

/*
 * Try to detect the boot mode. Fallback to the default,
 * memory mapped SPI XIP booting if detection failed.
 */
u32 spl_boot_device(void)
{
	int rc;
	struct udevice *scu_dev;
	struct ast2600_scu *scu;

	rc = uclass_get_device_by_driver(UCLASS_CLK,
					 DM_DRIVER_GET(aspeed_ast2600_scu), &scu_dev);
	if (rc) {
		debug("%s: failed to get SCU driver\n", __func__);
		goto out;
	}

	scu = devfdt_get_addr_ptr(scu_dev);
	if (IS_ERR_OR_NULL(scu)) {
		debug("%s: failed to get SCU base\n", __func__);
		goto out;
	}

	/* boot from UART has higher priority */
	if (scu->hwstrap2 & SCU_HWSTRAP2_BOOT_UART)
		return BOOT_DEVICE_UART;

	if (scu->hwstrap1 & SCU_HWSTRAP1_BOOT_EMMC)
		return BOOT_DEVICE_MMC1;

out:
	return BOOT_DEVICE_RAM;
}

struct legacy_img_hdr *spl_get_load_buffer(ssize_t offset, size_t size)
{
	return (struct legacy_img_hdr *)(CONFIG_SYS_LOAD_ADDR);
}

#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	/* boot linux */
	return 0;
}
#endif

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);
	return 0;
}
#endif

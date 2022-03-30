// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2013 Samsung Electronics
 */

#include <common.h>
#include <usb.h>
#include <asm/gpio.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/dwmmc.h>
#include <asm/arch/power.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_USB_EHCI_EXYNOS
int board_usb_init(int index, enum usb_init_type init)
{
	/* Configure gpios for usb 3503 hub:
	 * disconnect, toggle reset and connect
	 */
	gpio_request(EXYNOS5_GPIO_D17, "usb_connect");
	gpio_request(EXYNOS5_GPIO_X35, "usb_reset");
	gpio_direction_output(EXYNOS5_GPIO_D17, 0);
	gpio_direction_output(EXYNOS5_GPIO_X35, 0);

	gpio_direction_output(EXYNOS5_GPIO_X35, 1);
	gpio_direction_output(EXYNOS5_GPIO_D17, 1);

	return 0;
}
#endif

int board_init(void)
{
	gd->bd->bi_boot_params = (PHYS_SDRAM_1 + 0x100UL);
	return 0;
}

int dram_init(void)
{
	int i;
	u32 addr;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		addr = CONFIG_SYS_SDRAM_BASE + (i * SDRAM_BANK_SIZE);
		gd->ram_size += get_ram_size((long *)addr, SDRAM_BANK_SIZE);
	}
	return 0;
}

int power_init_board(void)
{
	set_ps_hold_ctrl();
	return 0;
}

int dram_init_banksize(void)
{
	int i;
	u32 addr, size;

	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		addr = CONFIG_SYS_SDRAM_BASE + (i * SDRAM_BANK_SIZE);
		size = get_ram_size((long *)addr, SDRAM_BANK_SIZE);

		gd->bd->bi_dram[i].start = addr;
		gd->bd->bi_dram[i].size = size;
	}

	return 0;
}

#ifdef CONFIG_MMC
int board_mmc_init(bd_t *bis)
{
	int ret;
	/* dwmmc initializattion for available channels */
	ret = exynos_dwmmc_init(gd->fdt_blob);
	if (ret)
		debug("dwmmc init failed\n");

	return ret;
}
#endif

static int board_uart_init(void)
{
	int err = 0, uart_id;

	for (uart_id = PERIPH_ID_UART0; uart_id <= PERIPH_ID_UART3; uart_id++) {
		err = exynos_pinmux_config(uart_id, PINMUX_FLAG_NONE);
		if (err) {
			debug("UART%d not configured\n",
			      (uart_id - PERIPH_ID_UART0));
			return err;
		}
	}
	return err;
}

#ifdef CONFIG_BOARD_EARLY_INIT_F
int board_early_init_f(void)
{
	int err;

	err = board_uart_init();
	if (err) {
		debug("UART init failed\n");
		return err;
	}
	return err;
}
#endif

#ifdef CONFIG_DISPLAY_BOARDINFO
int checkboard(void)
{
	printf("\nBoard: Arndale\n");

	return 0;
}
#endif

#ifdef CONFIG_S5P_PA_SYSRAM
void smp_set_core_boot_addr(unsigned long addr, int corenr)
{
	writel(addr, CONFIG_S5P_PA_SYSRAM);

	/* make sure this write is really executed */
	__asm__ volatile ("dsb\n");
}
#endif

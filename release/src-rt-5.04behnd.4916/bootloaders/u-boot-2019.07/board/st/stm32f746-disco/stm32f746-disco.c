// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016, STMicroelectronics - All Rights Reserved
 * Author(s): Vikas Manocha, <vikas.manocha@st.com> for STMicroelectronics.
 */

#include <common.h>
#include <dm.h>
#include <lcd.h>
#include <miiphy.h>
#include <phy_interface.h>
#include <ram.h>
#include <spl.h>
#include <splash.h>
#include <st_logo_data.h>
#include <video.h>
#include <asm/io.h>
#include <asm/armv7m.h>
#include <asm/arch/stm32.h>
#include <asm/arch/gpio.h>
#include <asm/arch/syscfg.h>
#include <asm/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
#ifndef CONFIG_SUPPORT_SPL
	int rv;
	struct udevice *dev;
	rv = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (rv) {
		debug("DRAM init failed: %d\n", rv);
		return rv;
	}

#endif
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

int board_early_init_f(void)
{
	return 0;
}

#ifdef CONFIG_SPL_BUILD
#ifdef CONFIG_SPL_OS_BOOT
int spl_start_uboot(void)
{
	debug("SPL: booting kernel\n");
	/* break into full u-boot on 'c' */
	return serial_tstc() && serial_getc() == 'c';
}
#endif

int spl_dram_init(void)
{
	struct udevice *dev;
	int rv;
	rv = uclass_get_device(UCLASS_RAM, 0, &dev);
	if (rv)
		debug("DRAM init failed: %d\n", rv);
	return rv;
}
void spl_board_init(void)
{
	spl_dram_init();
	preloader_console_init();
	arch_cpu_init(); /* to configure mpu for sdram rw permissions */
}
u32 spl_boot_device(void)
{
	return BOOT_DEVICE_XIP;
}

#endif
u32 get_board_rev(void)
{
	return 0;
}

int board_late_init(void)
{
	struct gpio_desc gpio = {};
	int node;

	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0, "st,led1");
	if (node < 0)
		return -1;

	gpio_request_by_name_nodev(offset_to_ofnode(node), "led-gpio", 0, &gpio,
				   GPIOD_IS_OUT);

	if (dm_gpio_is_valid(&gpio)) {
		dm_gpio_set_value(&gpio, 0);
		mdelay(10);
		dm_gpio_set_value(&gpio, 1);
	}

	/* read button 1*/
	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0, "st,button1");
	if (node < 0)
		return -1;

	gpio_request_by_name_nodev(offset_to_ofnode(node), "button-gpio", 0,
				   &gpio, GPIOD_IS_IN);

	if (dm_gpio_is_valid(&gpio)) {
		if (dm_gpio_get_value(&gpio))
			puts("usr button is at HIGH LEVEL\n");
		else
			puts("usr button is at LOW LEVEL\n");
	}

	return 0;
}

int board_init(void)
{
	gd->bd->bi_boot_params = gd->bd->bi_dram[0].start + 0x100;

#ifdef CONFIG_ETH_DESIGNWARE
	const char *phy_mode;
	int node;

	node = fdt_node_offset_by_compatible(gd->fdt_blob, 0, "st,stm32-dwmac");
	if (node < 0)
		return -1;

	phy_mode = fdt_getprop(gd->fdt_blob, node, "phy-mode", NULL);

	switch (phy_get_interface_by_name(phy_mode)) {
	case PHY_INTERFACE_MODE_RMII:
		STM32_SYSCFG->pmc |= SYSCFG_PMC_MII_RMII_SEL;
		break;
	case PHY_INTERFACE_MODE_MII:
		STM32_SYSCFG->pmc &= ~SYSCFG_PMC_MII_RMII_SEL;
		break;
	default:
		printf("PHY interface %s not supported !\n", phy_mode);
	}
#endif

#if defined(CONFIG_CMD_BMP)
	bmp_display((ulong)stmicroelectronics_uboot_logo_8bit_rle,
		    BMP_ALIGN_CENTER, BMP_ALIGN_CENTER);
#endif /* CONFIG_CMD_BMP */

	return 0;
}

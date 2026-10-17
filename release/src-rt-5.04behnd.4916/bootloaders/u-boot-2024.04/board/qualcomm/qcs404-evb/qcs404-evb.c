// SPDX-License-Identifier: GPL-2.0+
/*
 * Board init file for QCS404-EVB
 *
 * (C) Copyright 2022 Sumit Garg <sumit.garg@linaro.org>
 */

#include <common.h>
#include <cpu_func.h>
#include <dm.h>
#include <env.h>
#include <init.h>
#include <asm/cache.h>
#include <asm/gpio.h>
#include <asm/global_data.h>
#include <fdt_support.h>
#include <asm/arch/dram.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int board_init(void)
{
	struct udevice *pmic_gpio;
	struct gpio_desc usb_vbus_boost_pin;
	int ret, node;

	ret = uclass_get_device_by_name(UCLASS_GPIO,
					"pms405_gpios@c000",
					&pmic_gpio);
	if (ret < 0) {
		printf("Failed to find pms405_gpios@c000 node.\n");
		return ret;
	}

	node = fdt_subnode_offset(gd->fdt_blob, dev_of_offset(pmic_gpio),
				  "usb_vbus_boost_pin");
	if (node < 0) {
		printf("Failed to find usb_hub_reset_pm dt node.\n");
		return node;
	}
	ret = gpio_request_by_name_nodev(offset_to_ofnode(node), "gpios", 0,
					 &usb_vbus_boost_pin, 0);
	if (ret < 0) {
		printf("Failed to request usb_hub_reset_pm gpio.\n");
		return ret;
	}

	dm_gpio_set_dir_flags(&usb_vbus_boost_pin,
			      GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);

	return 0;
}

void reset_cpu(void)
{
	psci_system_reset();
}

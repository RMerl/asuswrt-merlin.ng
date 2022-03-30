// SPDX-License-Identifier: GPL-2.0+
/*
 * Amazon Kindle Fire (first generation) codename kc1 config
 *
 * Copyright (C) 2016 Paul Kocialkowski <contact@paulk.fr>
 */

#include <config.h>
#include <common.h>
#include <linux/ctype.h>
#include <linux/usb/musb.h>
#include <asm/omap_musb.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/gpio.h>
#include <asm/emif.h>
#include <twl6030.h>
#include "kc1.h"
#include <asm/mach-types.h>

DECLARE_GLOBAL_DATA_PTR;

const struct omap_sysinfo sysinfo = {
	.board_string = "kc1"
};

static struct musb_hdrc_config musb_config = {
	.multipoint = 1,
	.dyn_fifo = 1,
	.num_eps = 16,
	.ram_bits = 12
};

static struct omap_musb_board_data musb_board_data = {
	.interface_type	= MUSB_INTERFACE_UTMI,
};

static struct musb_hdrc_platform_data musb_platform_data = {
	.mode = MUSB_PERIPHERAL,
	.config = &musb_config,
	.power = 100,
	.platform_ops = &omap2430_ops,
	.board_data = &musb_board_data,
};


void set_muxconf_regs(void)
{
	do_set_mux((*ctrl)->control_padconf_core_base, core_padconf_array,
		sizeof(core_padconf_array) / sizeof(struct pad_conf_entry));
}

struct lpddr2_device_details *emif_get_device_details(u32 emif_nr, u8 cs,
	struct lpddr2_device_details *lpddr2_dev_details)
{
	if (cs == CS1)
		return NULL;

	*lpddr2_dev_details = elpida_2G_S4_details;

	return lpddr2_dev_details;
}

void emif_get_device_timings(u32 emif_nr,
	const struct lpddr2_device_timings **cs0_device_timings,
	const struct lpddr2_device_timings **cs1_device_timings)
{
	*cs0_device_timings = &elpida_2G_S4_timings;
	*cs1_device_timings = NULL;
}

int board_init(void)
{
	/* GPMC init */
	gpmc_init();

	/* MACH number */
	gd->bd->bi_arch_number = MACH_TYPE_OMAP_4430SDP;

	/* ATAGs location */
	gd->bd->bi_boot_params = OMAP44XX_DRAM_ADDR_SPACE_START + 0x100;

	return 0;
}

int misc_init_r(void)
{
	char reboot_mode[2] = { 0 };
	u32 data = 0;
	u32 value;
	int rc;

	/* Reboot mode */

	rc = omap_reboot_mode(reboot_mode, sizeof(reboot_mode));

	/* USB ID pin pull-up indicates factory (fastboot) cable detection. */
	gpio_request(KC1_GPIO_USB_ID, "USB_ID");
	gpio_direction_input(KC1_GPIO_USB_ID);
	value = gpio_get_value(KC1_GPIO_USB_ID);

	if (value)
		reboot_mode[0] = 'b';

	if (rc < 0 || reboot_mode[0] == 'o') {
		/*
		 * When not rebooting, valid power on reasons are either the
		 * power button, charger plug or USB plug.
		 */

		data |= twl6030_input_power_button();
		data |= twl6030_input_charger();
		data |= twl6030_input_usb();

		if (!data)
			twl6030_power_off();
	}

	if (reboot_mode[0] > 0 && isascii(reboot_mode[0])) {
		if (!env_get("reboot-mode"))
			env_set("reboot-mode", (char *)reboot_mode);
	}

	omap_reboot_mode_clear();

	/* Serial number */

	omap_die_id_serial();

	/* MUSB */

	musb_register(&musb_platform_data, &musb_board_data, (void *)MUSB_BASE);

	return 0;
}

u32 get_board_rev(void)
{
	u32 value = 0;

	gpio_request(KC1_GPIO_MBID0, "MBID0");
	gpio_request(KC1_GPIO_MBID1, "MBID1");
	gpio_request(KC1_GPIO_MBID2, "MBID2");
	gpio_request(KC1_GPIO_MBID3, "MBID3");

	gpio_direction_input(KC1_GPIO_MBID0);
	gpio_direction_input(KC1_GPIO_MBID1);
	gpio_direction_input(KC1_GPIO_MBID2);
	gpio_direction_input(KC1_GPIO_MBID3);

	value |= (gpio_get_value(KC1_GPIO_MBID0) << 0);
	value |= (gpio_get_value(KC1_GPIO_MBID1) << 1);
	value |= (gpio_get_value(KC1_GPIO_MBID2) << 2);
	value |= (gpio_get_value(KC1_GPIO_MBID3) << 3);

	return value;
}

void get_board_serial(struct tag_serialnr *serialnr)
{
	omap_die_id_get_board_serial(serialnr);
}

int fastboot_set_reboot_flag(void)
{
	return omap_reboot_mode_store("b");
}

int board_mmc_init(bd_t *bis)
{
	return omap_mmc_init(1, 0, 0, -1, -1);
}

void board_mmc_power_init(void)
{
	twl6030_power_mmc_init(1);
}

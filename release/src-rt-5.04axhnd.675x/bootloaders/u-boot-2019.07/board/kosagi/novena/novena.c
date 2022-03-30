// SPDX-License-Identifier: GPL-2.0+
/*
 * Novena board support
 *
 * Copyright (C) 2014 Marek Vasut <marex@denx.de>
 */

#include <common.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <ahci.h>
#include <linux/errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mxc_hdmi.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/mach-imx/sata.h>
#include <asm/mach-imx/video.h>
#include <dwc_ahsata.h>
#include <environment.h>
#include <fsl_esdhc.h>
#include <i2c.h>
#include <input.h>
#include <ipu_pixfmt.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <malloc.h>
#include <micrel.h>
#include <miiphy.h>
#include <mmc.h>
#include <netdev.h>
#include <power/pmic.h>
#include <power/pfuze100_pmic.h>
#include <stdio_dev.h>
#include <video_console.h>

#include "novena.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * GPIO button
 */
#ifdef CONFIG_KEYBOARD
static struct input_config button_input;

static int novena_gpio_button_read_keys(struct input_config *input)
{
	int key = KEY_ENTER;
	if (gpio_get_value(NOVENA_BUTTON_GPIO))
		return 0;
	input_send_keycodes(&button_input, &key, 1);
	return 1;
}

static int novena_gpio_button_getc(struct stdio_dev *dev)
{
	return input_getc(&button_input);
}

static int novena_gpio_button_tstc(struct stdio_dev *dev)
{
	return input_tstc(&button_input);
}

static int novena_gpio_button_init(struct stdio_dev *dev)
{
	gpio_direction_input(NOVENA_BUTTON_GPIO);
	input_set_delays(&button_input, 250, 250);
	return 0;
}

int drv_keyboard_init(void)
{
	int error;
	struct stdio_dev dev = {
		.name	= "button",
		.flags	= DEV_FLAGS_INPUT,
		.start	= novena_gpio_button_init,
		.getc	= novena_gpio_button_getc,
		.tstc	= novena_gpio_button_tstc,
	};

	gpio_request(NOVENA_BUTTON_GPIO, "button");

	error = input_init(&button_input, 0);
	if (error) {
		debug("%s: Cannot set up input\n", __func__);
		return -1;
	}
	input_add_tables(&button_input, false);
	button_input.read_keys = novena_gpio_button_read_keys;

	error = input_stdio_register(&dev);
	if (error)
		return error;

	return 0;
}
#endif

int board_early_init_f(void)
{
#if defined(CONFIG_VIDEO_IPUV3)
	setup_display_clock();
#endif

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

int board_late_init(void)
{
#if defined(CONFIG_VIDEO_IPUV3)
	struct udevice *con;
	char buf[DISPLAY_OPTIONS_BANNER_LENGTH];
	int ret;

	setup_display_lvds();

	ret = uclass_get_device(UCLASS_VIDEO_CONSOLE, 0, &con);
	if (ret)
		return ret;

	display_options_get_banner(false, buf, sizeof(buf));
	vidconsole_position_cursor(con, 0, 0);
	vidconsole_put_string(con, buf);
#endif
	return 0;
}

int checkboard(void)
{
	puts("Board: Novena 4x\n");
	return 0;
}

int dram_init(void)
{
	gd->ram_size = imx_ddr_size();
	return 0;
}

/* setup board specific PMIC */
int power_init_board(void)
{
	struct pmic *p;
	u32 reg;
	int ret;

	power_pfuze100_init(1);
	p = pmic_get("PFUZE100");
	if (!p)
		return -EINVAL;

	ret = pmic_probe(p);
	if (ret)
		return ret;

	pmic_reg_read(p, PFUZE100_DEVICEID, &reg);
	printf("PMIC:  PFUZE100 ID=0x%02x\n", reg);

	/* Set SWBST to 5.0V and enable (for USB) */
	pmic_reg_read(p, PFUZE100_SWBSTCON1, &reg);
	reg &= ~(SWBST_MODE_MASK | SWBST_VOL_MASK);
	reg |= (SWBST_5_00V | (SWBST_MODE_AUTO << SWBST_MODE_SHIFT));
	pmic_reg_write(p, PFUZE100_SWBSTCON1, reg);

	return 0;
}

/* EEPROM configuration data */
struct novena_eeprom_data {
	uint8_t		signature[6];
	uint8_t		version;
	uint8_t		reserved;
	uint32_t	serial;
	uint8_t		mac[6];
	uint16_t	features;
};

int misc_init_r(void)
{
	struct novena_eeprom_data data;
	uchar *datap = (uchar *)&data;
	const char *signature = "Novena";
	int ret;

	/* If 'ethaddr' is already set, do nothing. */
	if (env_get("ethaddr"))
		return 0;

	/* EEPROM is at bus 2. */
	ret = i2c_set_bus_num(2);
	if (ret) {
		puts("Cannot select EEPROM I2C bus.\n");
		return 0;
	}

	/* EEPROM is at address 0x56. */
	ret = eeprom_read(0x56, 0, datap, sizeof(data));
	if (ret) {
		puts("Cannot read I2C EEPROM.\n");
		return 0;
	}

	/* Check EEPROM signature. */
	if (memcmp(data.signature, signature, 6)) {
		puts("Invalid I2C EEPROM signature.\n");
		return 0;
	}

	/* Set ethernet address from EEPROM. */
	eth_env_set_enetaddr("ethaddr", data.mac);

	return ret;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * LG Optimus Black codename sniper board
 *
 * Copyright (C) 2015 Paul Kocialkowski <contact@paulk.fr>
 */

#include <config.h>
#include <common.h>
#include <dm.h>
#include <linux/ctype.h>
#include <linux/usb/musb.h>
#include <asm/omap_musb.h>
#include <asm/arch/mmc_host_def.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/mem.h>
#include <asm/io.h>
#include <ns16550.h>
#include <twl4030.h>
#include "sniper.h"

DECLARE_GLOBAL_DATA_PTR;

const omap3_sysinfo sysinfo = {
	.mtype = DDR_STACKED,
	.board_string = "sniper",
	.nand_string = "MMC"
};

static const struct ns16550_platdata serial_omap_platdata = {
	.base = OMAP34XX_UART3,
	.reg_shift = 2,
	.clock = V_NS16550_CLK,
	.fcr = UART_FCR_DEFVAL,
};

U_BOOT_DEVICE(sniper_serial) = {
	.name = "ns16550_serial",
	.platdata = &serial_omap_platdata
};

static struct musb_hdrc_config musb_config = {
	.multipoint = 1,
	.dyn_fifo = 1,
	.num_eps = 16,
	.ram_bits = 12
};

static struct omap_musb_board_data musb_board_data = {
	.interface_type	= MUSB_INTERFACE_ULPI,
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
	MUX_SNIPER();
}

#ifdef CONFIG_SPL_BUILD
void get_board_mem_timings(struct board_sdrc_timings *timings)
{
	timings->mcfg = HYNIX_V_MCFG_200(256 << 20);
	timings->ctrla = HYNIX_V_ACTIMA_200;
	timings->ctrlb = HYNIX_V_ACTIMB_200;
	timings->rfr_ctrl = SDP_3430_SDRC_RFR_CTRL_200MHz;
	timings->mr = MICRON_V_MR_165;
}
#endif

int board_init(void)
{
	/* GPMC init */
	gpmc_init();

	/* MACH number */
	gd->bd->bi_arch_number = 3000;

	/* ATAGs location */
	gd->bd->bi_boot_params = OMAP34XX_SDRC_CS0 + 0x100;

	return 0;
}

int misc_init_r(void)
{
	unsigned char keypad_matrix[64] = { 0 };
	char reboot_mode[2] = { 0 };
	unsigned char keys[3];
	unsigned char data = 0;
	int rc;

	/* Power button reset init */

	twl4030_power_reset_init();

	/* Keypad */

	twl4030_keypad_scan((unsigned char *)&keypad_matrix);

	keys[0] = twl4030_keypad_key((unsigned char *)&keypad_matrix, 0, 0);
	keys[1] = twl4030_keypad_key((unsigned char *)&keypad_matrix, 0, 1);
	keys[2] = twl4030_keypad_key((unsigned char *)&keypad_matrix, 0, 2);

	/* Reboot mode */

	rc = omap_reboot_mode(reboot_mode, sizeof(reboot_mode));

	if (keys[0])
		reboot_mode[0] = 'r';
	else if (keys[1])
		reboot_mode[0] = 'b';

	if (rc < 0 || reboot_mode[0] == 'o') {
		/*
		 * When not rebooting, valid power on reasons are either the
		 * power button, charger plug or USB plug.
		 */

		data |= twl4030_input_power_button();
		data |= twl4030_input_charger();
		data |= twl4030_input_usb();

		if (!data)
			twl4030_power_off();
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
	/* Sold devices are expected to be at least revision F. */
	return 6;
}

void get_board_serial(struct tag_serialnr *serialnr)
{
	omap_die_id_get_board_serial(serialnr);
}

void reset_misc(void)
{
	char reboot_mode[2] = { 0 };

	/*
	 * Valid resets must contain the reboot mode magic, but we must not
	 * override it when set previously (e.g. reboot to bootloader).
	 */

	omap_reboot_mode(reboot_mode, sizeof(reboot_mode));
	omap_reboot_mode_store(reboot_mode);
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
	twl4030_power_mmc_init(1);
}

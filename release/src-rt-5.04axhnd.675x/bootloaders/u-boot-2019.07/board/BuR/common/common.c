// SPDX-License-Identifier: GPL-2.0+
/*
 * common.c
 *
 * common board functions for B&R boards
 *
 * Copyright (C) 2013 Hannes Schmelzer <oe5hpm@oevsv.at>
 * Bernecker & Rainer Industrieelektronik GmbH - http://www.br-automation.com
 *
 */
#include <version.h>
#include <common.h>
#include <fdtdec.h>
#include <i2c.h>
#include <lcd.h>
#include "bur_common.h"

DECLARE_GLOBAL_DATA_PTR;

/* --------------------------------------------------------------------------*/
#if defined(CONFIG_LCD) && defined(CONFIG_AM335X_LCD) && \
	!defined(CONFIG_SPL_BUILD)
#include <asm/arch/hardware.h>
#include <asm/arch/cpu.h>
#include <asm/gpio.h>
#include <power/tps65217.h>
#include "../../../drivers/video/am335x-fb.h"

void lcdbacklight(int on)
{
	unsigned int driver = env_get_ulong("ds1_bright_drv", 16, 0UL);
	unsigned int bright = env_get_ulong("ds1_bright_def", 10, 50);
	unsigned int pwmfrq = env_get_ulong("ds1_pwmfreq", 10, ~0UL);
	unsigned int tmp;
	struct gptimer *timerhw;

	if (on)
		bright = bright != ~0UL ? bright : 50;
	else
		bright = 0;

	switch (driver) {
	case 2:
		timerhw = (struct gptimer *)DM_TIMER5_BASE;
		break;
	default:
		timerhw = (struct gptimer *)DM_TIMER6_BASE;
	}

	switch (driver) {
	case 0:	/* PMIC LED-Driver */
		/* brightness level */
		tps65217_reg_write(TPS65217_PROT_LEVEL_NONE,
				   TPS65217_WLEDCTRL2, bright, 0xFF);
		/* current sink */
		tps65217_reg_write(TPS65217_PROT_LEVEL_NONE,
				   TPS65217_WLEDCTRL1,
				   bright != 0 ? 0x0A : 0x02,
				   0xFF);
		break;
	case 1:
	case 2: /* PWM using timer */
		if (pwmfrq != ~0UL) {
			timerhw->tiocp_cfg = TCFG_RESET;
			udelay(10);
			while (timerhw->tiocp_cfg & TCFG_RESET)
				;
			tmp = ~0UL-(V_OSCK/pwmfrq);	/* bottom value */
			timerhw->tldr = tmp;
			timerhw->tcrr = tmp;
			tmp = tmp + ((V_OSCK/pwmfrq)/100) * bright;
			timerhw->tmar = tmp;
			timerhw->tclr = (TCLR_PT | (2 << TCLR_TRG_SHIFT) |
					TCLR_CE | TCLR_AR | TCLR_ST);
		} else {
			puts("invalid pwmfrq in env/dtb! skip PWM-setup.\n");
		}
		break;
	default:
		puts("no suitable backlightdriver in env/dtb!\n");
		break;
	}
}

int load_lcdtiming(struct am335x_lcdpanel *panel)
{
	struct am335x_lcdpanel pnltmp;

	pnltmp.hactive = env_get_ulong("ds1_hactive", 10, ~0UL);
	pnltmp.vactive = env_get_ulong("ds1_vactive", 10, ~0UL);
	pnltmp.bpp = env_get_ulong("ds1_bpp", 10, ~0UL);
	pnltmp.hfp = env_get_ulong("ds1_hfp", 10, ~0UL);
	pnltmp.hbp = env_get_ulong("ds1_hbp", 10, ~0UL);
	pnltmp.hsw = env_get_ulong("ds1_hsw", 10, ~0UL);
	pnltmp.vfp = env_get_ulong("ds1_vfp", 10, ~0UL);
	pnltmp.vbp = env_get_ulong("ds1_vbp", 10, ~0UL);
	pnltmp.vsw = env_get_ulong("ds1_vsw", 10, ~0UL);
	pnltmp.pxl_clk = env_get_ulong("ds1_pxlclk", 10, ~0UL);
	pnltmp.pol = env_get_ulong("ds1_pol", 16, ~0UL);
	pnltmp.pup_delay = env_get_ulong("ds1_pupdelay", 10, ~0UL);
	pnltmp.pon_delay = env_get_ulong("ds1_tondelay", 10, ~0UL);
	panel_info.vl_rot = env_get_ulong("ds1_rotation", 10, 0);

	if (
	   ~0UL == (pnltmp.hactive) ||
	   ~0UL == (pnltmp.vactive) ||
	   ~0UL == (pnltmp.bpp) ||
	   ~0UL == (pnltmp.hfp) ||
	   ~0UL == (pnltmp.hbp) ||
	   ~0UL == (pnltmp.hsw) ||
	   ~0UL == (pnltmp.vfp) ||
	   ~0UL == (pnltmp.vbp) ||
	   ~0UL == (pnltmp.vsw) ||
	   ~0UL == (pnltmp.pxl_clk) ||
	   ~0UL == (pnltmp.pol) ||
	   ~0UL == (pnltmp.pup_delay) ||
	   ~0UL == (pnltmp.pon_delay)
	   ) {
		puts("lcd-settings in env/dtb incomplete!\n");
		printf("display-timings:\n"
			"================\n"
			"hactive: %d\n"
			"vactive: %d\n"
			"bpp    : %d\n"
			"hfp    : %d\n"
			"hbp    : %d\n"
			"hsw    : %d\n"
			"vfp    : %d\n"
			"vbp    : %d\n"
			"vsw    : %d\n"
			"pxlclk : %d\n"
			"pol    : 0x%08x\n"
			"pondly : %d\n",
			pnltmp.hactive, pnltmp.vactive, pnltmp.bpp,
			pnltmp.hfp, pnltmp.hbp, pnltmp.hsw,
			pnltmp.vfp, pnltmp.vbp, pnltmp.vsw,
			pnltmp.pxl_clk, pnltmp.pol, pnltmp.pon_delay);

		return -1;
	}
	debug("lcd-settings in env complete, taking over.\n");
	memcpy((void *)panel,
	       (void *)&pnltmp,
	       sizeof(struct am335x_lcdpanel));

	return 0;
}

static void br_summaryscreen_printenv(char *prefix,
				       char *name, char *altname,
				       char *suffix)
{
	char *envval = env_get(name);
	if (0 != envval) {
		lcd_printf("%s %s %s", prefix, envval, suffix);
	} else if (0 != altname) {
		envval = env_get(altname);
		if (0 != envval)
			lcd_printf("%s %s %s", prefix, envval, suffix);
	} else {
		lcd_printf("\n");
	}
}

void br_summaryscreen(void)
{
	br_summaryscreen_printenv(" - B&R -", "br_orderno", 0, "-\n");
	br_summaryscreen_printenv(" Serial/Rev :", "br_serial", 0, "\n");
	br_summaryscreen_printenv(" MAC1       :", "br_mac1", "ethaddr", "\n");
	br_summaryscreen_printenv(" MAC2       :", "br_mac2", 0, "\n");
	lcd_puts(" Bootloader : " PLAIN_VERSION "\n");
	lcd_puts("\n");
}

void lcdpower(int on)
{
	u32 pin, swval, i;
	char buf[16] = { 0 };

	pin = env_get_ulong("ds1_pwr", 16, ~0UL);

	if (pin == ~0UL) {
		puts("no pwrpin in dtb/env, cannot powerup display!\n");
		return;
	}

	for (i = 0; i < 3; i++) {
		if (pin != 0) {
			snprintf(buf, sizeof(buf), "ds1_pwr#%d", i);
			if (gpio_request(pin & 0x7F, buf) != 0) {
				printf("%s: not able to request gpio %s",
				       __func__, buf);
				continue;
			}
			swval = pin & 0x80 ? 0 : 1;
			if (on)
				gpio_direction_output(pin & 0x7F, swval);
			else
				gpio_direction_output(pin & 0x7F, !swval);

			debug("switched pin %d to %d\n", pin & 0x7F, swval);
		}
		pin >>= 8;
	}
}

vidinfo_t	panel_info = {
		.vl_col = 1366,	/*
				 * give full resolution for allocating enough
				 * memory
				 */
		.vl_row = 768,
		.vl_bpix = 5,
		.priv = 0
};

void lcd_ctrl_init(void *lcdbase)
{
	struct am335x_lcdpanel lcd_panel;

	memset(&lcd_panel, 0, sizeof(struct am335x_lcdpanel));
	if (load_lcdtiming(&lcd_panel) != 0)
		return;

	lcd_panel.panel_power_ctrl = &lcdpower;

	if (0 != am335xfb_init(&lcd_panel))
		printf("ERROR: failed to initialize video!");
	/*
	 * modifiy panel info to 'real' resolution, to operate correct with
	 * lcd-framework.
	 */
	panel_info.vl_col = lcd_panel.hactive;
	panel_info.vl_row = lcd_panel.vactive;

	lcd_set_flush_dcache(1);
}

void lcd_enable(void)
{
	br_summaryscreen();
	lcdbacklight(1);
}
#endif /* CONFIG_LCD */

int ft_board_setup(void *blob, bd_t *bd)
{
	int nodeoffset;

	nodeoffset = fdt_path_offset(blob, "/factory-settings");
	if (nodeoffset < 0) {
		printf("%s: cannot find /factory-settings, trying /fset\n",
		       __func__);
		nodeoffset = fdt_path_offset(blob, "/fset");
		if (nodeoffset < 0) {
			printf("%s: cannot find /fset.\n", __func__);
			return 0;
		}
	}

	if (fdt_setprop(blob, nodeoffset, "bl-version",
			PLAIN_VERSION, strlen(PLAIN_VERSION)) != 0) {
		printf("%s: no 'bl-version' prop in fdt!\n", __func__);
		return 0;
	}
	return 0;
}

int brdefaultip_setup(int bus, int chip)
{
	int rc;
	struct udevice *i2cdev;
	u8 u8buf = 0;
	char defip[256] = { 0 };

	rc = i2c_get_chip_for_busnum(bus, chip, 2, &i2cdev);
	if (rc != 0) {
		printf("WARN: cannot probe baseboard EEPROM!\n");
		return -1;
	}

	rc = dm_i2c_read(i2cdev, 0, &u8buf, 1);
	if (rc != 0) {
		printf("WARN: cannot read baseboard EEPROM!\n");
		return -1;
	}

	if (u8buf != 0xFF)
		snprintf(defip, sizeof(defip),
			 "if test -r ${ipaddr}; then; else setenv ipaddr 192.168.60.%d; setenv serverip 192.168.60.254; setenv gatewayip 192.168.60.254; setenv netmask 255.255.255.0; fi;",
			 u8buf);
	else
		strncpy(defip,
			"if test -r ${ipaddr}; then; else setenv ipaddr 192.168.60.1; setenv serverip 192.168.60.254; setenv gatewayip 192.168.60.254; setenv netmask 255.255.255.0; fi;",
			sizeof(defip));

	env_set("brdefaultip", defip);
	env_set_hex("board_id", u8buf);

	return 0;
}

int overwrite_console(void)
{
	return 1;
}

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_AM33XX)
#include <asm/arch/hardware.h>
#include <asm/arch/omap.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <power/tps65217.h>

static struct ctrl_dev *cdev = (struct ctrl_dev *)CTRL_DEVICE_BASE;

void pmicsetup(u32 mpupll, unsigned int bus)
{
	int mpu_vdd;
	int usb_cur_lim;

	if (power_tps65217_init(bus)) {
		printf("WARN: cannot setup PMIC 0x24 @ bus #%d, not found!.\n",
		       bus);
		return;
	}

	/* Get the frequency which is defined by device fuses */
	dpll_mpu_opp100.m = am335x_get_efuse_mpu_max_freq(cdev);
	printf("detected max. frequency: %d - ", dpll_mpu_opp100.m);

	if (0 != mpupll) {
		dpll_mpu_opp100.m = mpupll;
		printf("retuning MPU-PLL to: %d MHz.\n", dpll_mpu_opp100.m);
	} else {
		puts("ok.\n");
	}
	/*
	 * Increase USB current limit to 1300mA or 1800mA and set
	 * the MPU voltage controller as needed.
	 */
	if (dpll_mpu_opp100.m == MPUPLL_M_1000) {
		usb_cur_lim = TPS65217_USB_INPUT_CUR_LIMIT_1800MA;
		mpu_vdd = TPS65217_DCDC_VOLT_SEL_1325MV;
	} else {
		usb_cur_lim = TPS65217_USB_INPUT_CUR_LIMIT_1300MA;
		mpu_vdd = TPS65217_DCDC_VOLT_SEL_1275MV;
	}

	if (tps65217_reg_write(TPS65217_PROT_LEVEL_NONE, TPS65217_POWER_PATH,
			       usb_cur_lim, TPS65217_USB_INPUT_CUR_LIMIT_MASK))
		puts("tps65217_reg_write failure\n");

	/* Set DCDC3 (CORE) voltage to 1.125V */
	if (tps65217_voltage_update(TPS65217_DEFDCDC3,
				    TPS65217_DCDC_VOLT_SEL_1125MV)) {
		puts("tps65217_voltage_update failure\n");
		return;
	}

	/* Set CORE Frequencies to OPP100 */
	do_setup_dpll(&dpll_core_regs, &dpll_core_opp100);

	/* Set DCDC2 (MPU) voltage */
	if (tps65217_voltage_update(TPS65217_DEFDCDC2, mpu_vdd)) {
		puts("tps65217_voltage_update failure\n");
		return;
	}

	/* Set LDO3 to 1.8V */
	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,
			       TPS65217_DEFLS1,
			       TPS65217_LDO_VOLTAGE_OUT_1_8,
			       TPS65217_LDO_MASK))
		puts("tps65217_reg_write failure\n");
	/* Set LDO4 to 3.3V */
	if (tps65217_reg_write(TPS65217_PROT_LEVEL_2,
			       TPS65217_DEFLS2,
			       TPS65217_LDO_VOLTAGE_OUT_3_3,
			       TPS65217_LDO_MASK))
		puts("tps65217_reg_write failure\n");

	/* Set MPU Frequency to what we detected now that voltages are set */
	do_setup_dpll(&dpll_mpu_regs, &dpll_mpu_opp100);
	/* Set PWR_EN bit in Status Register */
	tps65217_reg_write(TPS65217_PROT_LEVEL_NONE,
			   TPS65217_STATUS, TPS65217_PWR_OFF, TPS65217_PWR_OFF);
}

void set_uart_mux_conf(void)
{
	enable_uart0_pin_mux();
}

void set_mux_conf_regs(void)
{
	enable_board_pin_mux();
}

#endif /* CONFIG_SPL_BUILD && CONFIG_AM33XX */

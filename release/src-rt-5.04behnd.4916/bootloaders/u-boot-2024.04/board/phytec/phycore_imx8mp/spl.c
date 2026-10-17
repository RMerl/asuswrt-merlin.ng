// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2020 PHYTEC Messtechnik GmbH
 * Author: Teresa Remmet <t.remmet@phytec.de>
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/mach-imx/iomux-v3.h>
#include <hang.h>
#include <init.h>
#include <log.h>
#include <power/pmic.h>
#include <power/pca9450.h>
#include <spl.h>

#include "../common/imx8m_som_detection.h"

DECLARE_GLOBAL_DATA_PTR;

#define EEPROM_ADDR             0x51
#define EEPROM_ADDR_FALLBACK    0x59

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	return BOOT_DEVICE_BOOTROM;
}

void spl_dram_init(void)
{
	int ret;

	ret = phytec_eeprom_data_setup_fallback(NULL, 0, EEPROM_ADDR,
						EEPROM_ADDR_FALLBACK);
	if (ret)
		goto out;

	ret = phytec_imx8m_detect(NULL);
	if (!ret)
		phytec_print_som_info(NULL);

	u8 rev = phytec_get_rev(NULL);
	u8 somtype = phytec_get_som_type(NULL);

	if (rev != PHYTEC_EEPROM_INVAL && (rev >= 3 || (somtype == SOM_TYPE_PCL && rev >= 1))) {
		dram_timing.ddrc_cfg[3].val = 0x1323;
		dram_timing.ddrc_cfg[4].val = 0x1e84800;
		dram_timing.ddrc_cfg[5].val = 0x7a0118;
		dram_timing.ddrc_cfg[8].val = 0xc00307a3;
		dram_timing.ddrc_cfg[9].val = 0xc50000;
		dram_timing.ddrc_cfg[10].val = 0xf4003f;
		dram_timing.ddrc_cfg[11].val = 0xf30000;
		dram_timing.ddrc_cfg[14].val = 0x2028222a;
		dram_timing.ddrc_cfg[15].val = 0x8083f;
		dram_timing.ddrc_cfg[16].val = 0xe0e000;
		dram_timing.ddrc_cfg[17].val = 0x12040a12;
		dram_timing.ddrc_cfg[18].val = 0x2050f0f;
		dram_timing.ddrc_cfg[19].val = 0x1010009;
		dram_timing.ddrc_cfg[20].val = 0x502;
		dram_timing.ddrc_cfg[21].val = 0x20800;
		dram_timing.ddrc_cfg[22].val = 0xe100002;
		dram_timing.ddrc_cfg[23].val = 0x120;
		dram_timing.ddrc_cfg[24].val = 0xc80064;
		dram_timing.ddrc_cfg[25].val = 0x3e8001e;
		dram_timing.ddrc_cfg[26].val = 0x3207a12;
		dram_timing.ddrc_cfg[28].val = 0x4a3820e;
		dram_timing.ddrc_cfg[30].val = 0x230e;
		dram_timing.ddrc_cfg[37].val = 0x799;
		dram_timing.ddrc_cfg[38].val = 0x9141d1c;
		dram_timing.ddrc_cfg[74].val = 0x302;
		dram_timing.ddrc_cfg[83].val = 0x599;
		dram_timing.ddrc_cfg[99].val = 0x302;
		dram_timing.ddrc_cfg[108].val = 0x599;
		dram_timing.ddrphy_cfg[66].val = 0x18;
		dram_timing.ddrphy_cfg[75].val = 0x1e3;
		dram_timing.ddrphy_cfg[77].val = 0x1e3;
		dram_timing.ddrphy_cfg[79].val = 0x1e3;
		dram_timing.ddrphy_cfg[145].val = 0x3e8;
		dram_timing.fsp_msg[0].drate = 4000;
		dram_timing.fsp_msg[0].fsp_cfg[1].val = 0xfa0;
		dram_timing.fsp_msg[0].fsp_cfg[10].val = 0x3ff4;
		dram_timing.fsp_msg[0].fsp_cfg[11].val = 0xf3;
		dram_timing.fsp_msg[0].fsp_cfg[15].val = 0x3ff4;
		dram_timing.fsp_msg[0].fsp_cfg[16].val = 0xf3;
		dram_timing.fsp_msg[0].fsp_cfg[22].val = 0xf400;
		dram_timing.fsp_msg[0].fsp_cfg[23].val = 0xf33f;
		dram_timing.fsp_msg[0].fsp_cfg[28].val = 0xf400;
		dram_timing.fsp_msg[0].fsp_cfg[29].val = 0xf33f;
		dram_timing.fsp_msg[3].drate = 4000;
		dram_timing.fsp_msg[3].fsp_cfg[1].val = 0xfa0;
		dram_timing.fsp_msg[3].fsp_cfg[11].val = 0x3ff4;
		dram_timing.fsp_msg[3].fsp_cfg[12].val = 0xf3;
		dram_timing.fsp_msg[3].fsp_cfg[16].val = 0x3ff4;
		dram_timing.fsp_msg[3].fsp_cfg[17].val = 0xf3;
		dram_timing.fsp_msg[3].fsp_cfg[23].val = 0xf400;
		dram_timing.fsp_msg[3].fsp_cfg[24].val = 0xf33f;
		dram_timing.fsp_msg[3].fsp_cfg[29].val = 0xf400;
		dram_timing.fsp_msg[3].fsp_cfg[30].val = 0xf33f;
		dram_timing.ddrphy_pie[480].val = 0x465;
		dram_timing.ddrphy_pie[481].val = 0xfa;
		dram_timing.ddrphy_pie[482].val = 0x9c4;
		dram_timing.fsp_table[0] = 4000;
	}

out:
	ddr_init(&dram_timing);
}

#define I2C_PAD_CTRL (PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_PE)
#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)
struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = MX8MP_PAD_I2C1_SCL__I2C1_SCL | PC,
		.gpio_mode = MX8MP_PAD_I2C1_SCL__GPIO5_IO14 | PC,
		.gp = IMX_GPIO_NR(5, 14),
	},
	.sda = {
		.i2c_mode = MX8MP_PAD_I2C1_SDA__I2C1_SDA | PC,
		.gpio_mode = MX8MP_PAD_I2C1_SDA__GPIO5_IO15 | PC,
		.gp = IMX_GPIO_NR(5, 15),
	},
};

int power_init_board(void)
{
	struct pmic *p;
	int ret;

	ret = power_pca9450_init(0, 0x25);
	if (ret)
		printf("power init failed");
	p = pmic_get("PCA9450");
	pmic_probe(p);

	/* BUCKxOUT_DVS0/1 control BUCK123 output */
	pmic_reg_write(p, PCA9450_BUCK123_DVS, 0x29);

	/* Increase VDD_SOC and VDD_ARM to OD voltage 0.95V */
	pmic_reg_write(p, PCA9450_BUCK1OUT_DVS0, 0x1C);
	pmic_reg_write(p, PCA9450_BUCK2OUT_DVS0, 0x1C);

	/* Set BUCK1 DVS1 to suspend controlled through PMIC_STBY_REQ */
	pmic_reg_write(p, PCA9450_BUCK1OUT_DVS1, 0x14);
	pmic_reg_write(p, PCA9450_BUCK1CTRL, 0x59);

	/* Set WDOG_B_CFG to cold reset */
	pmic_reg_write(p, PCA9450_RESET_CTRL, 0xA1);

	return 0;
}

void spl_board_init(void)
{
	/* Set GIC clock to 500Mhz for OD VDD_SOC. */
	clock_enable(CCGR_GIC, 0);
	clock_set_target_val(GIC_CLK_ROOT, CLK_ROOT_ON | CLK_ROOT_SOURCE_SEL(5));
	clock_enable(CCGR_GIC, 1);
}

int board_fit_config_name_match(const char *name)
{
	return 0;
}

void board_init_f(ulong dummy)
{
	int ret;

	arch_cpu_init();

	init_uart_clk(0);

	ret = spl_early_init();
	if (ret) {
		debug("spl_early_init() failed: %d\n", ret);
		hang();
	}

	preloader_console_init();

	enable_tzc380();

	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);

	power_init_board();

	/* DDR initialization */
	spl_dram_init();
}

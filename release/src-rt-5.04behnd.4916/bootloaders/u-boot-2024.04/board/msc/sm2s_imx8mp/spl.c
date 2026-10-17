// SPDX-License-Identifier: GPL-2.0
/*
 * Based on vendor support provided by AVNET Embedded
 *
 * Copyright (C) 2021 AVNET Embedded, MSC Technologies GmbH
 * Copyright 2021 General Electric Company
 * Copyright 2021 Collabora Ltd.
 */

#include <common.h>
#include <cpu_func.h>
#include <fsl_esdhc_imx.h>
#include <hang.h>
#include <i2c.h>
#include <image.h>
#include <init.h>
#include <log.h>
#include <mmc.h>
#include <spl.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/imx8mp_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <dm/uclass.h>
#include <dm/device.h>
#include <linux/delay.h>
#include <power/pmic.h>
#include <power/rn5t567_pmic.h>

DECLARE_GLOBAL_DATA_PTR;

int spl_board_boot_device(enum boot_device boot_dev_spl)
{
	return BOOT_DEVICE_BOOTROM;
}

void spl_dram_init(void)
{
	ddr_init(&dram_timing);
}

void spl_board_init(void)
{
	/*
	 * Set GIC clock to 500Mhz for OD VDD_SOC. Kernel driver does
	 * not allow to change it. Should set the clock after PMIC
	 * setting done. Default is 400Mhz (system_pll1_800m with div = 2)
	 * set by ROM for ND VDD_SOC
	 */
	clock_enable(CCGR_GIC, 0);
	clock_set_target_val(GIC_CLK_ROOT, CLK_ROOT_ON | CLK_ROOT_SOURCE_SEL(5));
	clock_enable(CCGR_GIC, 1);

	puts("Normal Boot\n");
}

#define USDHC_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE \
	| PAD_CTL_PE | PAD_CTL_FSEL2)
#define USDHC_GPIO_PAD_CTRL (PAD_CTL_HYS | PAD_CTL_DSE1)
#define USDHC_CD_PAD_CTRL (PAD_CTL_PE | PAD_CTL_PUE | PAD_CTL_HYS \
	| PAD_CTL_DSE4)

static const iomux_v3_cfg_t usdhc2_pads[] = {
	MX8MP_PAD_SD2_CLK__USDHC2_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_SD2_CMD__USDHC2_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_SD2_DATA0__USDHC2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_SD2_DATA1__USDHC2_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_SD2_DATA2__USDHC2_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_SD2_DATA3__USDHC2_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_SD2_RESET_B__GPIO2_IO19 | MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
	MX8MP_PAD_SD2_WP__GPIO2_IO20 | MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
	MX8MP_PAD_SD2_CD_B__GPIO2_IO12 | MUX_PAD_CTRL(USDHC_CD_PAD_CTRL),
};

#define USDHC2_CD_GPIO	IMX_GPIO_NR(2, 12)
#define USDHC2_RESET_GPIO IMX_GPIO_NR(2, 19)

static const iomux_v3_cfg_t usdhc3_pads[] = {
	MX8MP_PAD_NAND_WE_B__USDHC3_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_WP_B__USDHC3_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_DATA04__USDHC3_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_DATA05__USDHC3_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_DATA06__USDHC3_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_DATA07__USDHC3_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_RE_B__USDHC3_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_CE2_B__USDHC3_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_CE3_B__USDHC3_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_CLE__USDHC3_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_READY_B__USDHC3_RESET_B | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	MX8MP_PAD_NAND_CE1_B__USDHC3_STROBE | MUX_PAD_CTRL(USDHC_PAD_CTRL),

};

static struct fsl_esdhc_cfg usdhc_cfg[] = {
	{ USDHC2_BASE_ADDR, 0, 4 },
	{ USDHC3_BASE_ADDR, 0, 8 },
};

int board_mmc_init(struct bd_info *bis)
{
	int i, ret;
	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-Boot device node)    (Physical Port)
	 * mmc0 (sd)               USDHC2
	 * mmc1 (emmc)             USDHC3
	 */
	for (i = 0; i < CFG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			init_clk_usdhc(1);
			usdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);
			imx_iomux_v3_setup_multiple_pads(usdhc2_pads,
							 ARRAY_SIZE(usdhc2_pads));
			gpio_request(USDHC2_RESET_GPIO, "usdhc2_reset");
			gpio_direction_output(USDHC2_RESET_GPIO, 0);
			udelay(500);
			gpio_direction_output(USDHC2_RESET_GPIO, 1);
			gpio_request(USDHC2_CD_GPIO, "usdhc2 cd");
			gpio_direction_input(USDHC2_CD_GPIO);
			break;
		case 1:
			init_clk_usdhc(2);
			usdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
			imx_iomux_v3_setup_multiple_pads(usdhc3_pads,
							 ARRAY_SIZE(usdhc3_pads));
			break;
		default:
			printf("Warning: you configured more USDHC controllers (%d) than supported by the board\n",
			       i + 1);
			return -EINVAL;
		}

		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		if (ret)
			return ret;
	}

	return 0;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC2_BASE_ADDR:
		ret = !gpio_get_value(USDHC2_CD_GPIO);
		break;
	case USDHC3_BASE_ADDR:
		ret = 1;
		break;
	}

	return ret;
}

#define WDOG_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_ODE | PAD_CTL_PUE | PAD_CTL_PE)

static const iomux_v3_cfg_t wdog_pads[] = {
	MX8MP_PAD_GPIO1_IO02__WDOG1_WDOG_B  | MUX_PAD_CTRL(WDOG_PAD_CTRL),
};

int board_early_init_f(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	imx_iomux_v3_setup_multiple_pads(wdog_pads, ARRAY_SIZE(wdog_pads));
	set_wdog_reset(wdog);

	return 0;
}

static const iomux_v3_cfg_t reset_out_pad[] = {
	MX8MP_PAD_SAI2_MCLK__GPIO4_IO27 | MUX_PAD_CTRL(0x19)
};

#define RESET_OUT_GPIO IMX_GPIO_NR(4, 27)

static void pulse_reset_out(void)
{
	imx_iomux_v3_setup_multiple_pads(reset_out_pad, ARRAY_SIZE(reset_out_pad));

	gpio_request(RESET_OUT_GPIO, "reset_out_gpio");
	gpio_direction_output(RESET_OUT_GPIO, 0);
	udelay(10);
	gpio_direction_output(RESET_OUT_GPIO, 1);
}

#define I2C_PAD_CTRL (PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE | PAD_CTL_PE)
#define PC MUX_PAD_CTRL(I2C_PAD_CTRL)
struct i2c_pads_info i2c_dev_pads = {
	.scl = {
		.i2c_mode = MX8MP_PAD_SAI5_RXFS__I2C6_SCL | PC,
		.gpio_mode = MX8MP_PAD_SAI5_RXFS__GPIO3_IO19 | PC,
		.gp = IMX_GPIO_NR(3, 19),
	},
	.sda = {
		.i2c_mode = MX8MP_PAD_SAI5_RXC__I2C6_SDA | PC,
		.gpio_mode = MX8MP_PAD_SAI5_RXC__GPIO3_IO20 | PC,
		.gp = IMX_GPIO_NR(3, 20),
	},
};

int power_init_board(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_PMIC, 0, &dev);
	if (ret) {
		printf("Error: Failed to get PMIC\n");
		return ret;
	}

	/* set VCC_DRAM (buck2) to 1.1V */
	pmic_reg_write(dev, RN5T567_DC2DAC, 0x28);

	/* set VCC_ARM (buck2) to 0.95V */
	pmic_reg_write(dev, RN5T567_DC3DAC, 0x1C);

	return 0;
}

int board_fit_config_name_match(const char *name)
{
	return 0;
}

void board_init_f(ulong dummy)
{
	int ret;

	arch_cpu_init();

	init_uart_clk(1);

	board_early_init_f();

	pulse_reset_out();

	timer_init();

	ret = spl_early_init();
	if (ret) {
		printf("Error: failed to initialize SPL!\n");
		hang();
	}

	preloader_console_init();

	enable_tzc380();

	power_init_board();

	spl_dram_init();
}

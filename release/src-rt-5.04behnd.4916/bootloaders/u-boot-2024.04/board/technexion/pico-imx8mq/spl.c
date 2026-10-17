// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018 NXP
 */

#include <common.h>
#include <hang.h>
#include <init.h>
#include <log.h>
#include <asm/arch/clock.h>
#include <asm/arch/ddr.h>
#include <asm/arch/imx8mq_pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mach-imx/gpio.h>
#include <asm/mach-imx/iomux-v3.h>
#include <asm/mach-imx/mxc_i2c.h>
#include <asm/sections.h>
#include <linux/delay.h>
#include <errno.h>
#include <fsl_esdhc_imx.h>
#include <mmc.h>
#include <spl.h>

#include "lpddr4_timing.h"

DECLARE_GLOBAL_DATA_PTR;

#define DDR_DET_1		IMX_GPIO_NR(3, 11)
#define DDR_DET_2		IMX_GPIO_NR(3, 12)
#define DDR_DET_3		IMX_GPIO_NR(3, 13)

static iomux_v3_cfg_t const verdet_pads[] = {
	IMX8MQ_PAD_NAND_DATA01__GPIO3_IO7 | MUX_PAD_CTRL(NO_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA02__GPIO3_IO8 | MUX_PAD_CTRL(NO_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA03__GPIO3_IO9 | MUX_PAD_CTRL(NO_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA04__GPIO3_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA05__GPIO3_IO11 | MUX_PAD_CTRL(NO_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA06__GPIO3_IO12 | MUX_PAD_CTRL(NO_PAD_CTRL),
	IMX8MQ_PAD_NAND_DATA07__GPIO3_IO13 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

/*
 * DDR_DET_1    DDR_DET_2   DDR_DET_3
 *    0            0            1       4G LPDDR4
 *    1            1            1       3G LPDDR4
 *    1            1            0       2G LPDDR4
 *    1            0            1       1G LPDDR4
 */
static void spl_dram_init(void)
{
	struct dram_timing_info *dram_timing;
	u8 ddr = 0, size;

	imx_iomux_v3_setup_multiple_pads(verdet_pads, ARRAY_SIZE(verdet_pads));

	gpio_request(DDR_DET_1, "ddr_det_1");
	gpio_direction_input(DDR_DET_1);
	gpio_request(DDR_DET_2, "ddr_det_2");
	gpio_direction_input(DDR_DET_2);
	gpio_request(DDR_DET_3, "ddr_det_3");
	gpio_direction_input(DDR_DET_3);

	ddr |= !!gpio_get_value(DDR_DET_3) << 0;
	ddr |= !!gpio_get_value(DDR_DET_2) << 1;
	ddr |= !!gpio_get_value(DDR_DET_1) << 2;

	switch (ddr) {
	case 0x1:
		size = 4;
		dram_timing = &dram_timing_4gb;
		break;
	case 0x7:
		size = 3;
		dram_timing = &dram_timing_3gb;
		break;
	case 0x6:
		size = 2;
		dram_timing = &dram_timing_2gb;
		break;
	case 0x5:
		size = 1;
		dram_timing = &dram_timing_1gb;
		break;
	default:
		puts("Unknown DDR type!!!\n");
		return;
	}

	printf("%s: LPDDR4 %d GiB\n", __func__, size);
	ddr_init(dram_timing);
	writel(size, MCU_BOOTROM_BASE_ADDR);
}

#define USDHC2_CD_GPIO	IMX_GPIO_NR(2, 12)
#define USDHC1_PWR_GPIO IMX_GPIO_NR(2, 10)
#define USDHC2_PWR_GPIO IMX_GPIO_NR(2, 19)

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
		ret = 1;
		break;
	case USDHC2_BASE_ADDR:
		ret = !gpio_get_value(USDHC2_CD_GPIO);
		return ret;
	}

	return 1;
}

#define USDHC_PAD_CTRL	(PAD_CTL_DSE6 | PAD_CTL_HYS | PAD_CTL_PUE | \
			 PAD_CTL_FSEL2)
#define USDHC_GPIO_PAD_CTRL (PAD_CTL_PUE | PAD_CTL_DSE1)

static iomux_v3_cfg_t const usdhc1_pads[] = {
	IMX8MQ_PAD_SD1_CLK__USDHC1_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_CMD__USDHC1_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA0__USDHC1_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA1__USDHC1_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA2__USDHC1_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA3__USDHC1_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA4__USDHC1_DATA4 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA5__USDHC1_DATA5 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA6__USDHC1_DATA6 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_DATA7__USDHC1_DATA7 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD1_RESET_B__GPIO2_IO10 | MUX_PAD_CTRL(NO_PAD_CTRL),
};

static iomux_v3_cfg_t const usdhc2_pads[] = {
	IMX8MQ_PAD_SD2_CLK__USDHC2_CLK | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD2_CMD__USDHC2_CMD | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD2_DATA0__USDHC2_DATA0 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD2_DATA1__USDHC2_DATA1 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD2_DATA2__USDHC2_DATA2 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD2_DATA3__USDHC2_DATA3 | MUX_PAD_CTRL(USDHC_PAD_CTRL),
	IMX8MQ_PAD_SD2_CD_B__GPIO2_IO12 | MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
	IMX8MQ_PAD_SD2_RESET_B__GPIO2_IO19 | MUX_PAD_CTRL(USDHC_GPIO_PAD_CTRL),
};

static struct fsl_esdhc_cfg usdhc_cfg[2] = {
	{USDHC1_BASE_ADDR, 0, 8},
	{USDHC2_BASE_ADDR, 0, 4},
};

int board_mmc_init(struct bd_info *bis)
{
	int ret;
	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-Boot device node)    (Physical Port)
	 * mmc0                    USDHC1
	 * mmc1                    USDHC2
	 */
	init_clk_usdhc(0);
	usdhc_cfg[0].sdhc_clk = mxc_get_clock(USDHC1_CLK_ROOT);
	imx_iomux_v3_setup_multiple_pads(usdhc1_pads, ARRAY_SIZE(usdhc1_pads));
	gpio_request(USDHC1_PWR_GPIO, "usdhc1_reset");
	gpio_direction_output(USDHC1_PWR_GPIO, 0);
	udelay(500);
	gpio_direction_output(USDHC1_PWR_GPIO, 1);
	ret = fsl_esdhc_initialize(bis, &usdhc_cfg[0]);
	if (ret)
		return ret;

	init_clk_usdhc(1);
	usdhc_cfg[1].sdhc_clk = mxc_get_clock(USDHC2_CLK_ROOT);
	imx_iomux_v3_setup_multiple_pads(usdhc2_pads, ARRAY_SIZE(usdhc2_pads));
	gpio_request(USDHC2_PWR_GPIO, "usdhc2_reset");
	gpio_direction_output(USDHC2_PWR_GPIO, 0);
	udelay(500);
	gpio_direction_output(USDHC2_PWR_GPIO, 1);
	return fsl_esdhc_initialize(bis, &usdhc_cfg[1]);
}

void spl_board_init(void)
{
	puts("Normal Boot\n");
}

#ifdef CONFIG_SPL_LOAD_FIT
int board_fit_config_name_match(const char *name)
{
	/* Just empty function now - can't decide what to choose */
	debug("%s: %s\n", __func__, name);

	return 0;
}
#endif

void board_init_f(ulong dummy)
{
	int ret;

	/* Clear global data */
	memset((void *)gd, 0, sizeof(gd_t));

	arch_cpu_init();

	init_uart_clk(0);

	board_early_init_f();

	timer_init();

	preloader_console_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	ret = spl_init();
	if (ret) {
		debug("spl_init() failed: %d\n", ret);
		hang();
	}

	enable_tzc380();

	/* DDR initialization */
	spl_dram_init();

	board_init_r(NULL, 0);
}

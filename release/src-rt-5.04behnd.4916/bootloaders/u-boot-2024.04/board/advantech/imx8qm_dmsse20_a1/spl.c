// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2017-2018 NXP
 * Copyright 2019-2023 Kococonnector GmbH
 */
#include <common.h>
#include <dm.h>
#include <spl.h>
#include <init.h>

#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/clock.h>
#include <firmware/imx/sci/sci.h>
#include <asm/arch/imx8-pins.h>
#include <asm/arch/iomux.h>
#include <asm/sections.h>
#include <fsl_esdhc_imx.h>

DECLARE_GLOBAL_DATA_PTR;

#define ESDHC_PAD_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | \
		(SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
		(SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
		(SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define ESDHC_CLK_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | \
		(SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
		(SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
		(SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define ENET_INPUT_PAD_CTRL	((SC_PAD_CONFIG_OD_IN << PADRING_CONFIG_SHIFT) | \
		(SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
		(SC_PAD_28FDSOI_DSE_18V_10MA << PADRING_DSE_SHIFT) | \
		(SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define ENET_NORMAL_PAD_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | \
		(SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
		(SC_PAD_28FDSOI_DSE_18V_10MA << PADRING_DSE_SHIFT) | \
		(SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define FSPI_PAD_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | \
		(SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
		(SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
		(SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define GPIO_PAD_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | \
		(SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
		(SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
		(SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define I2C_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | \
		(SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
		(SC_PAD_28FDSOI_DSE_DV_LOW << PADRING_DSE_SHIFT) | \
		(SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define UART_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | \
		(SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
		(SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
		(SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))
#ifdef CONFIG_FSL_ESDHC_IMX

#define USDHC1_CD_GPIO	IMX_GPIO_NR(5, 22)
#define USDHC2_CD_GPIO	IMX_GPIO_NR(4, 12)

static struct fsl_esdhc_cfg usdhc_cfg[CFG_SYS_FSL_USDHC_NUM] = {
	{USDHC1_BASE_ADDR, 0, 8},
	{USDHC3_BASE_ADDR, 0, 4},
};

static iomux_cfg_t emmc0[] = {
	SC_P_EMMC0_CLK | MUX_PAD_CTRL(ESDHC_CLK_PAD_CTRL),
	SC_P_EMMC0_CMD | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA0 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA1 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA2 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA3 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA4 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA5 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA6 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_DATA7 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_RESET_B | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_EMMC0_STROBE | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
};

static iomux_cfg_t usdhc2_sd[] = {
	SC_P_USDHC2_CLK | MUX_PAD_CTRL(ESDHC_CLK_PAD_CTRL),
	SC_P_USDHC2_CMD | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC2_DATA0 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC2_DATA1 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC2_DATA2 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC2_DATA3 | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC2_RESET_B | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC2_WP   | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
	SC_P_USDHC2_CD_B | MUX_MODE_ALT(3) | MUX_PAD_CTRL(ESDHC_PAD_CTRL),
};

void init_clk_usdhc(u32 index);

int board_mmc_init(struct bd_info *bis)
{
	int i, ret;

	/*
	 * According to the board_mmc_init() the following map is done:
	 * (U-Boot device node)    (Physical Port)
	 * mmc0                    USDHC1
	 * mmc1                    USDHC2
	 * mmc2                    USDHC3
	 */
	for (i = 0; i < CFG_SYS_FSL_USDHC_NUM; i++) {
		switch (i) {
		case 0:
			ret = sc_pm_set_resource_power_mode(-1, SC_R_SDHC_0, SC_PM_PW_MODE_ON);
			if (ret)
				return ret;

			imx8_iomux_setup_multiple_pads(emmc0, ARRAY_SIZE(emmc0));
			init_clk_usdhc(0);
			usdhc_cfg[i].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
			break;
		case 1:
			ret = sc_pm_set_resource_power_mode(-1, SC_R_SDHC_2, SC_PM_PW_MODE_ON);
			if (ret)
				return ret;
			ret = sc_pm_set_resource_power_mode(-1, SC_R_GPIO_4, SC_PM_PW_MODE_ON);
			if (ret)
				return ret;

			imx8_iomux_setup_multiple_pads(usdhc2_sd, ARRAY_SIZE(usdhc2_sd));
			init_clk_usdhc(2);
			usdhc_cfg[i].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);
			gpio_request(USDHC2_CD_GPIO, "sd2_cd");
			gpio_direction_input(USDHC2_CD_GPIO);
			break;
		default:
			printf("Warning: you configured more USDHC controllers"
				"(%d) than supported by the board\n", i + 1);
			return 0;
		}
		ret = fsl_esdhc_initialize(bis, &usdhc_cfg[i]);
		if (ret) {
			printf("Warning: failed to initialize mmc dev %d\n", i);
			return ret;
		}
	}

	return 0;
}

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret = 0;

	switch (cfg->esdhc_base) {
	case USDHC1_BASE_ADDR:
		ret = 1;
		break;
	case USDHC2_BASE_ADDR:
		ret = !gpio_get_value(USDHC1_CD_GPIO);
		break;
	case USDHC3_BASE_ADDR:
		ret = !gpio_get_value(USDHC2_CD_GPIO);
		break;
	}

	return ret;
}

#endif /* CONFIG_FSL_ESDHC_IMX */

void spl_board_init(void)
{
#if defined(CONFIG_SPL_SPI_SUPPORT)
	if (sc_rm_is_resource_owned(-1, SC_R_FSPI_0)) {
		if (sc_pm_set_resource_power_mode(-1, SC_R_FSPI_0, SC_PM_PW_MODE_ON)) {
			puts("Warning: failed to initialize FSPI0\n");
		}
	}
#endif

	puts("Normal Boot\n");
}

void spl_board_prepare_for_boot(void)
{
#if defined(CONFIG_SPL_SPI_SUPPORT)
	if (sc_rm_is_resource_owned(-1, SC_R_FSPI_0)) {
		if (sc_pm_set_resource_power_mode(-1, SC_R_FSPI_0, SC_PM_PW_MODE_OFF)) {
			puts("Warning: failed to turn off FSPI0\n");
		}
	}
#endif
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
	/* Clear global data */
	memset((void *)gd, 0, sizeof(gd_t));

	arch_cpu_init();

	board_early_init_f();

	timer_init();

	preloader_console_init();

	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	board_init_r(NULL, 0);
}

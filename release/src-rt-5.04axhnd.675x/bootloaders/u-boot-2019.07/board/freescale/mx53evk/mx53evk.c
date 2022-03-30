// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux-mx53.h>
#include <linux/errno.h>
#include <asm/mach-imx/boot_mode.h>
#include <netdev.h>
#include <i2c.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <power/pmic.h>
#include <fsl_pmic.h>
#include <asm/gpio.h>
#include <mc13892.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				PHYS_SDRAM_1_SIZE);
	return 0;
}

#define UART_PAD_CTRL	(PAD_CTL_HYS | PAD_CTL_DSE_HIGH | \
			 PAD_CTL_PUS_100K_UP | PAD_CTL_ODE)

static void setup_iomux_uart(void)
{
	static const iomux_v3_cfg_t uart_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_CSI0_DAT11__UART1_RXD_MUX, UART_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_CSI0_DAT10__UART1_TXD_MUX, UART_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));
}

#define I2C_PAD_CTRL	(PAD_CTL_SRE_FAST | PAD_CTL_DSE_HIGH | \
			 PAD_CTL_HYS | PAD_CTL_ODE)

static void setup_i2c(unsigned int port_number)
{
	static const iomux_v3_cfg_t i2c1_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_CSI0_DAT8__I2C1_SDA, I2C_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_CSI0_DAT9__I2C1_SCL, I2C_PAD_CTRL),
	};

	static const iomux_v3_cfg_t i2c2_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_KEY_ROW3__I2C2_SDA, I2C_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_KEY_COL3__I2C2_SCL, I2C_PAD_CTRL),
	};

	switch (port_number) {
	case 0:
		imx_iomux_v3_setup_multiple_pads(i2c1_pads,
							ARRAY_SIZE(i2c1_pads));
		break;
	case 1:
		imx_iomux_v3_setup_multiple_pads(i2c2_pads,
							ARRAY_SIZE(i2c2_pads));
		break;
	default:
		printf("Warning: Wrong I2C port number\n");
		break;
	}
}

void power_init(void)
{
	unsigned int val;
	struct pmic *p;
	int ret;

	ret = pmic_init(I2C_0);
	if (ret)
		return;

	p = pmic_get("FSL_PMIC");
	if (!p)
		return;

	/* Set VDDA to 1.25V */
	pmic_reg_read(p, REG_SW_2, &val);
	val &= ~SWX_OUT_MASK;
	val |= SWX_OUT_1_25;
	pmic_reg_write(p, REG_SW_2, val);

	/*
	 * Need increase VCC and VDDA to 1.3V
	 * according to MX53 IC TO2 datasheet.
	 */
	if (is_soc_rev(CHIP_REV_2_0) == 0) {
		/* Set VCC to 1.3V for TO2 */
		pmic_reg_read(p, REG_SW_1, &val);
		val &= ~SWX_OUT_MASK;
		val |= SWX_OUT_1_30;
		pmic_reg_write(p, REG_SW_1, val);

		/* Set VDDA to 1.3V for TO2 */
		pmic_reg_read(p, REG_SW_2, &val);
		val &= ~SWX_OUT_MASK;
		val |= SWX_OUT_1_30;
		pmic_reg_write(p, REG_SW_2, val);
	}
}

static void setup_iomux_fec(void)
{
	static const iomux_v3_cfg_t fec_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_FEC_MDIO__FEC_MDIO, PAD_CTL_HYS |
			PAD_CTL_DSE_HIGH | PAD_CTL_PUS_22K_UP | PAD_CTL_ODE),
		NEW_PAD_CTRL(MX53_PAD_FEC_MDC__FEC_MDC, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_FEC_RXD1__FEC_RDATA_1,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_RXD0__FEC_RDATA_0,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_TXD1__FEC_TDATA_1, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_FEC_TXD0__FEC_TDATA_0, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_FEC_TX_EN__FEC_TX_EN, PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_FEC_REF_CLK__FEC_TX_CLK,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_RX_ER__FEC_RX_ER,
				PAD_CTL_HYS | PAD_CTL_PKE),
		NEW_PAD_CTRL(MX53_PAD_FEC_CRS_DV__FEC_RX_DV,
				PAD_CTL_HYS | PAD_CTL_PKE),
	};

	imx_iomux_v3_setup_multiple_pads(fec_pads, ARRAY_SIZE(fec_pads));
}

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[2] = {
	{MMC_SDHC1_BASE_ADDR},
	{MMC_SDHC3_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret;

	imx_iomux_v3_setup_pad(MX53_PAD_EIM_DA11__GPIO3_11);
	gpio_direction_input(IMX_GPIO_NR(3, 11));
	imx_iomux_v3_setup_pad(MX53_PAD_EIM_DA13__GPIO3_13);
	gpio_direction_input(IMX_GPIO_NR(3, 13));

	if (cfg->esdhc_base == MMC_SDHC1_BASE_ADDR)
		ret = !gpio_get_value(IMX_GPIO_NR(3, 13));
	else
		ret = !gpio_get_value(IMX_GPIO_NR(3, 11));

	return ret;
}

#define SD_CMD_PAD_CTRL		(PAD_CTL_HYS | PAD_CTL_DSE_HIGH | \
				 PAD_CTL_PUS_100K_UP)
#define SD_PAD_CTRL		(PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | \
				 PAD_CTL_DSE_HIGH)

int board_mmc_init(bd_t *bis)
{
	static const iomux_v3_cfg_t sd1_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_SD1_CMD__ESDHC1_CMD, SD_CMD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_CLK__ESDHC1_CLK, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_DATA0__ESDHC1_DAT0, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_DATA1__ESDHC1_DAT1, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_DATA2__ESDHC1_DAT2, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_DATA3__ESDHC1_DAT3, SD_PAD_CTRL),
		MX53_PAD_EIM_DA13__GPIO3_13,
	};

	static const iomux_v3_cfg_t sd2_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_PATA_RESET_B__ESDHC3_CMD,
				SD_CMD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_IORDY__ESDHC3_CLK, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA8__ESDHC3_DAT0, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA9__ESDHC3_DAT1, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA10__ESDHC3_DAT2, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA11__ESDHC3_DAT3, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA0__ESDHC3_DAT4, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA1__ESDHC3_DAT5, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA2__ESDHC3_DAT6, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA3__ESDHC3_DAT7, SD_PAD_CTRL),
		MX53_PAD_EIM_DA11__GPIO3_11,
	};

	u32 index;
	int ret;

	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	esdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC3_CLK);

	for (index = 0; index < CONFIG_SYS_FSL_ESDHC_NUM; index++) {
		switch (index) {
		case 0:
			imx_iomux_v3_setup_multiple_pads(sd1_pads,
							 ARRAY_SIZE(sd1_pads));
			break;
		case 1:
			imx_iomux_v3_setup_multiple_pads(sd2_pads,
							 ARRAY_SIZE(sd2_pads));
			break;
		default:
			printf("Warning: you configured more ESDHC controller"
				"(%d) as supported by the board(2)\n",
				CONFIG_SYS_FSL_ESDHC_NUM);
			return -EINVAL;
		}
		ret = fsl_esdhc_initialize(bis, &esdhc_cfg[index]);
		if (ret)
			return ret;
	}

	return 0;
}
#endif

int board_early_init_f(void)
{
	setup_iomux_uart();
	setup_iomux_fec();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

#ifdef CONFIG_CMD_BMODE
static const struct boot_mode board_boot_modes[] = {
	/* 4 bit bus width */
	{"mmc0",	MAKE_CFGVAL(0x40, 0x20, 0x00, 0x12)},
	{"mmc1",	MAKE_CFGVAL(0x40, 0x20, 0x08, 0x12)},
	{NULL,		0},
};
#endif

int board_late_init(void)
{
	setup_i2c(1);
	power_init();

#ifdef CONFIG_CMD_BMODE
	add_board_boot_modes(board_boot_modes);
#endif
	return 0;
}

int checkboard(void)
{
	puts("Board: MX53EVK\n");

	return 0;
}

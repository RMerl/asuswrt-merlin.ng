// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011 Freescale Semiconductor, Inc.
 *
 * Author: Fabio Estevam <fabio.estevam@freescale.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-mx25.h>
#include <asm/arch/clock.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <i2c.h>
#include <power/pmic.h>
#include <fsl_pmic.h>
#include <mc34704.h>

#define FEC_RESET_B		IMX_GPIO_NR(4, 8)
#define FEC_ENABLE_B		IMX_GPIO_NR(2, 3)
#define CARD_DETECT		IMX_GPIO_NR(2, 1)

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[1] = {
	{IMX_MMC_SDHC1_BASE},
};
#endif

/*
 * FIXME: need to revisit this
 * The original code enabled PUE and 100-k pull-down without PKE, so the right
 * value here is likely:
 *	0 for no pull
 * or:
 *	PAD_CTL_PUS_100K_DOWN for 100-k pull-down
 */
#define FEC_OUT_PAD_CTRL	0

#define I2C_PAD_CTRL		(PAD_CTL_HYS | PAD_CTL_PUS_100K_UP | \
				 PAD_CTL_ODE)

static void mx25pdk_fec_init(void)
{
	static const iomux_v3_cfg_t fec_pads[] = {
		MX25_PAD_FEC_TX_CLK__FEC_TX_CLK,
		MX25_PAD_FEC_RX_DV__FEC_RX_DV,
		MX25_PAD_FEC_RDATA0__FEC_RDATA0,
		NEW_PAD_CTRL(MX25_PAD_FEC_TDATA0__FEC_TDATA0, FEC_OUT_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_FEC_TX_EN__FEC_TX_EN, FEC_OUT_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_FEC_MDC__FEC_MDC, FEC_OUT_PAD_CTRL),
		MX25_PAD_FEC_MDIO__FEC_MDIO,
		MX25_PAD_FEC_RDATA1__FEC_RDATA1,
		NEW_PAD_CTRL(MX25_PAD_FEC_TDATA1__FEC_TDATA1, FEC_OUT_PAD_CTRL),

		NEW_PAD_CTRL(MX25_PAD_D12__GPIO_4_8, 0), /* FEC_RESET_B */
		NEW_PAD_CTRL(MX25_PAD_A17__GPIO_2_3, 0), /* FEC_ENABLE_B */
	};

	static const iomux_v3_cfg_t i2c_pads[] = {
		NEW_PAD_CTRL(MX25_PAD_I2C1_CLK__I2C1_CLK, I2C_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_I2C1_DAT__I2C1_DAT, I2C_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(fec_pads, ARRAY_SIZE(fec_pads));

	/* Assert RESET and ENABLE low */
	gpio_direction_output(FEC_RESET_B, 0);
	gpio_direction_output(FEC_ENABLE_B, 0);

	udelay(10);

	/* Deassert RESET and ENABLE */
	gpio_set_value(FEC_RESET_B, 1);
	gpio_set_value(FEC_ENABLE_B, 1);

	/* Setup I2C pins so that PMIC can turn on PHY supply */
	imx_iomux_v3_setup_multiple_pads(i2c_pads, ARRAY_SIZE(i2c_pads));
}

int dram_init(void)
{
	/* dram_init must store complete ramsize in gd->ram_size */
	gd->ram_size = get_ram_size((void *)CONFIG_SYS_SDRAM_BASE,
				PHYS_SDRAM_1_SIZE);
	return 0;
}

/*
 * Set up input pins with hysteresis and 100-k pull-ups
 */
#define UART1_IN_PAD_CTRL	(PAD_CTL_HYS | PAD_CTL_PUS_100K_UP)
/*
 * FIXME: need to revisit this
 * The original code enabled PUE and 100-k pull-down without PKE, so the right
 * value here is likely:
 *	0 for no pull
 * or:
 *	PAD_CTL_PUS_100K_DOWN for 100-k pull-down
 */
#define UART1_OUT_PAD_CTRL	0

static void mx25pdk_uart1_init(void)
{
	static const iomux_v3_cfg_t uart1_pads[] = {
		NEW_PAD_CTRL(MX25_PAD_UART1_RXD__UART1_RXD, UART1_IN_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_UART1_TXD__UART1_TXD, UART1_OUT_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_UART1_RTS__UART1_RTS, UART1_OUT_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_UART1_CTS__UART1_CTS, UART1_IN_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

int board_early_init_f(void)
{
	mx25pdk_uart1_init();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

int board_late_init(void)
{
	struct pmic *p;
	int ret;

	mx25pdk_fec_init();

	ret = pmic_init(I2C_0);
	if (ret)
		return ret;

	p = pmic_get("FSL_PMIC");
	if (!p)
		return -ENODEV;

	/* Turn on Ethernet PHY and LCD supplies */
	pmic_reg_write(p, MC34704_GENERAL2_REG, ONOFFE | ONOFFA);

	return 0;
}

#ifdef CONFIG_FSL_ESDHC
int board_mmc_getcd(struct mmc *mmc)
{
	/* Set up the Card Detect pin. */
	imx_iomux_v3_setup_pad(NEW_PAD_CTRL(MX25_PAD_A15__GPIO_2_1, 0));

	gpio_direction_input(CARD_DETECT);
	return !gpio_get_value(CARD_DETECT);
}

int board_mmc_init(bd_t *bis)
{
	static const iomux_v3_cfg_t sdhc1_pads[] = {
		NEW_PAD_CTRL(MX25_PAD_SD1_CMD__SD1_CMD, NO_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_SD1_CLK__SD1_CLK, NO_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_SD1_DATA0__SD1_DATA0, NO_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_SD1_DATA1__SD1_DATA1, NO_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_SD1_DATA2__SD1_DATA2, NO_PAD_CTRL),
		NEW_PAD_CTRL(MX25_PAD_SD1_DATA3__SD1_DATA3, NO_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(sdhc1_pads, ARRAY_SIZE(sdhc1_pads));

	/*
	 * Set the eSDHC1 PER clock to the maximum frequency lower than or equal
	 * to 50 MHz that can be obtained, which requires to use UPLL as the
	 * clock source. This actually gives 48 MHz.
	 */
	imx_set_perclk(MXC_ESDHC1_CLK, true, 50000000);
	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC1_CLK);
	return fsl_esdhc_initialize(bis, &esdhc_cfg[0]);
}
#endif

int checkboard(void)
{
	puts("Board: MX25PDK\n");

	return 0;
}

/* Lowlevel init isn't used on mx25pdk, so just provide a dummy one here */
void lowlevel_init(void) {}

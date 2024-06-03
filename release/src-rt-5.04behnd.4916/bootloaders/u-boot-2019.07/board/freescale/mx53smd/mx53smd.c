// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/iomux-mx53.h>
#include <linux/errno.h>
#include <netdev.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <asm/gpio.h>

DECLARE_GLOBAL_DATA_PTR;

int dram_init(void)
{
	u32 size1, size2;

	size1 = get_ram_size((void *)PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE);
	size2 = get_ram_size((void *)PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE);

	gd->ram_size = size1 + size2;

	return 0;
}
int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = PHYS_SDRAM_1;
	gd->bd->bi_dram[0].size = PHYS_SDRAM_1_SIZE;

	gd->bd->bi_dram[1].start = PHYS_SDRAM_2;
	gd->bd->bi_dram[1].size = PHYS_SDRAM_2_SIZE;

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
struct fsl_esdhc_cfg esdhc_cfg[1] = {
	{MMC_SDHC1_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	imx_iomux_v3_setup_pad(MX53_PAD_EIM_DA13__GPIO3_13);
	gpio_direction_input(IMX_GPIO_NR(3, 13));
	return !gpio_get_value(IMX_GPIO_NR(3, 13));
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

	u32 index;
	int ret;

	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);

	for (index = 0; index < CONFIG_SYS_FSL_ESDHC_NUM; index++) {
		switch (index) {
		case 0:
			imx_iomux_v3_setup_multiple_pads(sd1_pads,
							 ARRAY_SIZE(sd1_pads));
			break;

		default:
			printf("Warning: you configured more ESDHC controller"
				"(%d) as supported by the board(1)\n",
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

int checkboard(void)
{
	puts("Board: MX53SMD\n");

	return 0;
}

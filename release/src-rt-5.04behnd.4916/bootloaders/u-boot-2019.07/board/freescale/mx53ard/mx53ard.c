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

#define ETHERNET_INT		IMX_GPIO_NR(2, 31)

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

#ifdef CONFIG_NAND_MXC
static void setup_iomux_nand(void)
{
	static const iomux_v3_cfg_t nand_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_NANDF_CS0__EMI_NANDF_CS_0,
				PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_NANDF_CS1__EMI_NANDF_CS_1,
				PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_NANDF_RB0__EMI_NANDF_RB_0,
				PAD_CTL_PUS_100K_UP),
		NEW_PAD_CTRL(MX53_PAD_NANDF_CLE__EMI_NANDF_CLE,
				PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_NANDF_ALE__EMI_NANDF_ALE,
				PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_NANDF_WP_B__EMI_NANDF_WP_B,
				PAD_CTL_PUS_100K_UP),
		NEW_PAD_CTRL(MX53_PAD_NANDF_RE_B__EMI_NANDF_RE_B,
				PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_NANDF_WE_B__EMI_NANDF_WE_B,
				PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_DA0__EMI_NAND_WEIM_DA_0,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_DA1__EMI_NAND_WEIM_DA_1,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_DA2__EMI_NAND_WEIM_DA_2,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_DA3__EMI_NAND_WEIM_DA_3,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_DA4__EMI_NAND_WEIM_DA_4,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_DA5__EMI_NAND_WEIM_DA_5,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_DA6__EMI_NAND_WEIM_DA_6,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_DA7__EMI_NAND_WEIM_DA_7,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
	};

	u32 i, reg;

	reg = __raw_readl(M4IF_BASE_ADDR + 0xc);
	reg &= ~M4IF_GENP_WEIM_MM_MASK;
	__raw_writel(reg, M4IF_BASE_ADDR + 0xc);
	for (i = 0x4; i < 0x94; i += 0x18) {
		reg = __raw_readl(WEIM_BASE_ADDR + i);
		reg &= ~WEIM_GCR2_MUX16_BYP_GRANT_MASK;
		__raw_writel(reg, WEIM_BASE_ADDR + i);
	}

	imx_iomux_v3_setup_multiple_pads(nand_pads, ARRAY_SIZE(nand_pads));
}
#else
static void setup_iomux_nand(void)
{
}
#endif

#define UART_PAD_CTRL	(PAD_CTL_HYS | PAD_CTL_DSE_HIGH | \
			 PAD_CTL_PUS_100K_UP | PAD_CTL_ODE)

static void setup_iomux_uart(void)
{
	static const iomux_v3_cfg_t uart_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_PATA_DMACK__UART1_RXD_MUX, UART_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DIOW__UART1_TXD_MUX, UART_PAD_CTRL),
	};

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));
}

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[2] = {
	{MMC_SDHC1_BASE_ADDR},
	{MMC_SDHC2_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	struct fsl_esdhc_cfg *cfg = (struct fsl_esdhc_cfg *)mmc->priv;
	int ret;

	imx_iomux_v3_setup_pad(MX53_PAD_GPIO_1__GPIO1_1);
	gpio_direction_input(IMX_GPIO_NR(1, 1));
	imx_iomux_v3_setup_pad(MX53_PAD_GPIO_4__GPIO1_4);
	gpio_direction_input(IMX_GPIO_NR(1, 4));

	if (cfg->esdhc_base == MMC_SDHC1_BASE_ADDR)
		ret = !gpio_get_value(IMX_GPIO_NR(1, 1));
	else
		ret = !gpio_get_value(IMX_GPIO_NR(1, 4));

	return ret;
}

#define SD_CMD_PAD_CTRL		(PAD_CTL_HYS | PAD_CTL_DSE_HIGH | \
				 PAD_CTL_PUS_100K_UP)
#define SD_CLK_PAD_CTRL		(PAD_CTL_PUS_47K_UP | PAD_CTL_DSE_HIGH)
#define SD_PAD_CTRL		(PAD_CTL_HYS | PAD_CTL_PUS_47K_UP | \
				 PAD_CTL_DSE_HIGH)

int board_mmc_init(bd_t *bis)
{
	static const iomux_v3_cfg_t sd1_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_SD1_CMD__ESDHC1_CMD, SD_CMD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_CLK__ESDHC1_CLK, SD_CLK_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_DATA0__ESDHC1_DAT0, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_DATA1__ESDHC1_DAT1, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_DATA2__ESDHC1_DAT2, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD1_DATA3__ESDHC1_DAT3, SD_PAD_CTRL),
	};

	static const iomux_v3_cfg_t sd2_pads[] = {
		NEW_PAD_CTRL(MX53_PAD_SD2_CMD__ESDHC2_CMD, SD_CMD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD2_CLK__ESDHC2_CLK, SD_CLK_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD2_DATA0__ESDHC2_DAT0, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD2_DATA1__ESDHC2_DAT1, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD2_DATA2__ESDHC2_DAT2, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_SD2_DATA3__ESDHC2_DAT3, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA12__ESDHC2_DAT4, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA13__ESDHC2_DAT5, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA14__ESDHC2_DAT6, SD_PAD_CTRL),
		NEW_PAD_CTRL(MX53_PAD_PATA_DATA15__ESDHC2_DAT7, SD_PAD_CTRL),
	};

	u32 index;
	int ret;

	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
	esdhc_cfg[1].sdhc_clk = mxc_get_clock(MXC_ESDHC2_CLK);

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

static void weim_smc911x_iomux(void)
{
	static const iomux_v3_cfg_t weim_smc911x_pads[] = {
		/* Data bus */
		NEW_PAD_CTRL(MX53_PAD_EIM_D16__EMI_WEIM_D_16,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_D17__EMI_WEIM_D_17,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_D18__EMI_WEIM_D_18,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_D19__EMI_WEIM_D_19,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_D20__EMI_WEIM_D_20,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_D21__EMI_WEIM_D_21,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_D22__EMI_WEIM_D_22,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_D23__EMI_WEIM_D_23,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_D24__EMI_WEIM_D_24,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_D25__EMI_WEIM_D_25,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_D26__EMI_WEIM_D_26,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_D27__EMI_WEIM_D_27,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_D28__EMI_WEIM_D_28,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_D29__EMI_WEIM_D_29,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_D30__EMI_WEIM_D_30,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_D31__EMI_WEIM_D_31,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),

		/* Address lines */
		NEW_PAD_CTRL(MX53_PAD_EIM_DA0__EMI_NAND_WEIM_DA_0,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_DA1__EMI_NAND_WEIM_DA_1,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_DA2__EMI_NAND_WEIM_DA_2,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_DA3__EMI_NAND_WEIM_DA_3,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_DA4__EMI_NAND_WEIM_DA_4,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_DA5__EMI_NAND_WEIM_DA_5,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),
		NEW_PAD_CTRL(MX53_PAD_EIM_DA6__EMI_NAND_WEIM_DA_6,
				PAD_CTL_PKE | PAD_CTL_DSE_HIGH),

		/* other EIM signals for ethernet */
		MX53_PAD_EIM_OE__EMI_WEIM_OE,
		MX53_PAD_EIM_RW__EMI_WEIM_RW,
		MX53_PAD_EIM_CS1__EMI_WEIM_CS_1,
	};

	/* ETHERNET_INT as GPIO2_31 */
	imx_iomux_v3_setup_pad(MX53_PAD_EIM_EB3__GPIO2_31);
	gpio_direction_input(ETHERNET_INT);

	/* WEIM bus */
	imx_iomux_v3_setup_multiple_pads(weim_smc911x_pads,
						ARRAY_SIZE(weim_smc911x_pads));
}

static void weim_cs1_settings(void)
{
	struct weim *weim_regs = (struct weim *)WEIM_BASE_ADDR;

	writel(MX53ARD_CS1GCR1, &weim_regs->cs1gcr1);
	writel(0x0, &weim_regs->cs1gcr2);
	writel(MX53ARD_CS1RCR1, &weim_regs->cs1rcr1);
	writel(MX53ARD_CS1RCR2, &weim_regs->cs1rcr2);
	writel(MX53ARD_CS1WCR1, &weim_regs->cs1wcr1);
	writel(0x0, &weim_regs->cs1wcr2);
	writel(0x0, &weim_regs->wcr);

	set_chipselect_size(CS0_64M_CS1_64M);
}

int board_early_init_f(void)
{
	setup_iomux_nand();
	setup_iomux_uart();
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM_1 + 0x100;

	return 0;
}

int board_eth_init(bd_t *bis)
{
	int rc = -ENODEV;

	weim_smc911x_iomux();
	weim_cs1_settings();

#ifdef CONFIG_SMC911X
	rc = smc911x_initialize(0, CONFIG_SMC911X_BASE);
#endif
	return rc;
}

int checkboard(void)
{
	puts("Board: MX53ARD\n");

	return 0;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Linaro.
 * Peter Griffin <peter.griffin@linaro.org>
 */

#include <common.h>
#include <fdtdec.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <asm/arch/pinmux.h>

struct hi6220_pinmux0_regs *pmx0 =
	(struct hi6220_pinmux0_regs *)HI6220_PINMUX0_BASE;

struct hi6220_pinmux1_regs *pmx1 =
	(struct hi6220_pinmux1_regs *)HI6220_PINMUX1_BASE;

static void hi6220_uart_config(int peripheral)
{
	switch (peripheral) {
	case PERIPH_ID_UART0:
		writel(MUX_M0, &pmx0->iomg[48]); /* UART0_RXD */
		writel(MUX_M0, &pmx0->iomg[49]); /* UART0_TXD */

		writel(DRIVE1_02MA | PULL_UP, &pmx1->iocfg[49]); /* UART0_RXD */
		writel(DRIVE1_02MA | PULL_UP, &pmx1->iocfg[50]); /* UART0_TXD */
		break;

	case PERIPH_ID_UART1:
		writel(MUX_M0, &pmx0->iomg[50]); /* UART1_CTS_N */
		writel(MUX_M0, &pmx0->iomg[51]); /* UART1_RTS_N */
		writel(MUX_M0, &pmx0->iomg[52]); /* UART1_RXD */
		writel(MUX_M0, &pmx0->iomg[53]); /* UART1_TXD */

		writel(DRIVE1_02MA | PULL_UP, &pmx1->iocfg[51]); /*UART1_CTS_N*/
		writel(DRIVE1_02MA | PULL_UP, &pmx1->iocfg[53]); /* UART1_RXD */
		writel(DRIVE1_02MA, &pmx1->iocfg[52]); /* UART1_RTS_N */
		writel(DRIVE1_02MA, &pmx1->iocfg[54]); /* UART1_TXD */
		break;

	case PERIPH_ID_UART2:
		writel(MUX_M0, &pmx0->iomg[54]); /* UART2_CTS_N */
		writel(MUX_M0, &pmx0->iomg[55]); /* UART2_RTS_N */
		writel(MUX_M0, &pmx0->iomg[56]); /* UART2_RXD */
		writel(MUX_M0, &pmx0->iomg[57]); /* UART2_TXD */

		writel(DRIVE1_02MA, &pmx1->iocfg[55]); /* UART2_CTS_N */
		writel(DRIVE1_02MA, &pmx1->iocfg[56]); /* UART2_RTS_N */
		writel(DRIVE1_02MA, &pmx1->iocfg[57]); /* UART2_RXD */
		writel(DRIVE1_02MA, &pmx1->iocfg[58]); /* UART2_TXD */
		break;

	case PERIPH_ID_UART3:
		writel(MUX_M1, &pmx0->iomg[96]); /* UART3_CTS_N */
		writel(MUX_M1, &pmx0->iomg[97]); /* UART3_RTS_N */
		writel(MUX_M1, &pmx0->iomg[98]); /* UART3_RXD */
		writel(MUX_M1, &pmx0->iomg[99]); /* UART3_TXD */

		/* UART3_TXD */
		writel(DRIVE1_02MA | PULL_DOWN, &pmx1->iocfg[100]);
		/* UART3_RTS_N */
		writel(DRIVE1_02MA | PULL_DOWN, &pmx1->iocfg[101]);
		/* UART3_RXD */
		writel(DRIVE1_02MA | PULL_DOWN, &pmx1->iocfg[102]);
		/* UART3_TXD */
		writel(DRIVE1_02MA | PULL_DOWN, &pmx1->iocfg[103]);
		break;

	case PERIPH_ID_UART4:
		writel(MUX_M1, &pmx0->iomg[116]); /* UART4_CTS_N */
		writel(MUX_M1, &pmx0->iomg[117]); /* UART4_RTS_N */
		writel(MUX_M1, &pmx0->iomg[118]); /* UART4_RXD */
		writel(MUX_M1, &pmx0->iomg[119]); /* UART4_TXD */

		/* UART4_CTS_N */
		writel(DRIVE1_02MA | PULL_DOWN, &pmx1->iocfg[120]);
		/* UART4_RTS_N */
		writel(DRIVE1_02MA | PULL_DOWN, &pmx1->iocfg[121]);
		/* UART4_RXD */
		writel(DRIVE1_02MA | PULL_DOWN, &pmx1->iocfg[122]);
		/* UART4_TXD */
		writel(DRIVE1_02MA | PULL_DOWN, &pmx1->iocfg[123]);
		break;
	case PERIPH_ID_UART5:
		writel(MUX_M1, &pmx0->iomg[114]); /* UART5_RXD */
		writel(MUX_M1, &pmx0->iomg[115]); /* UART5_TXD */

		/* UART5_RXD */
		writel(DRIVE1_02MA | PULL_DOWN, &pmx1->iocfg[118]);
		/* UART5_TXD */
		writel(DRIVE1_02MA | PULL_DOWN, &pmx1->iocfg[119]);

		break;

	default:
		debug("%s: invalid peripheral %d", __func__, peripheral);
		return;
	}
}

static int hi6220_mmc_config(int peripheral)
{
	u32 tmp;

	switch (peripheral) {
	case PERIPH_ID_SDMMC0:

		/* eMMC pinmux config */
		writel(MUX_M0, &pmx0->iomg[64]); /* EMMC_CLK */
		writel(MUX_M0, &pmx0->iomg[65]); /* EMMC_CMD */
		writel(MUX_M0, &pmx0->iomg[66]); /* EMMC_DATA0 */
		writel(MUX_M0, &pmx0->iomg[67]); /* EMMC_DATA1 */
		writel(MUX_M0, &pmx0->iomg[68]); /* EMMC_DATA2 */
		writel(MUX_M0, &pmx0->iomg[69]); /* EMMC_DATA3 */
		writel(MUX_M0, &pmx0->iomg[70]); /* EMMC_DATA4 */
		writel(MUX_M0, &pmx0->iomg[71]); /* EMMC_DATA5 */
		writel(MUX_M0, &pmx0->iomg[72]); /* EMMC_DATA6 */
		writel(MUX_M0, &pmx0->iomg[73]); /* EMMC_DATA7 */

		/*eMMC configure up/down/drive */
		writel(DRIVE1_08MA, &pmx1->iocfg[65]); /* EMMC_CLK */

		tmp = DRIVE1_04MA | PULL_UP;
		writel(tmp, &pmx1->iocfg[65]); /* EMMC_CMD */
		writel(tmp, &pmx1->iocfg[66]); /* EMMC_DATA0 */
		writel(tmp, &pmx1->iocfg[67]); /* EMMC_DATA1 */
		writel(tmp, &pmx1->iocfg[68]); /* EMMC_DATA2 */
		writel(tmp, &pmx1->iocfg[69]); /* EMMC_DATA3 */
		writel(tmp, &pmx1->iocfg[70]); /* EMMC_DATA4 */
		writel(tmp, &pmx1->iocfg[71]); /* EMMC_DATA5 */
		writel(tmp, &pmx1->iocfg[72]); /* EMMC_DATA6 */
		writel(tmp, &pmx1->iocfg[73]); /* EMMC_DATA7 */

		writel(DRIVE1_04MA, &pmx1->iocfg[73]); /* EMMC_RST_N */
		break;

	case PERIPH_ID_SDMMC1:

		writel(MUX_M0, &pmx0->iomg[3]); /* SD_CLK */
		writel(MUX_M0, &pmx0->iomg[4]); /* SD_CMD */
		writel(MUX_M0, &pmx0->iomg[5]); /* SD_DATA0 */
		writel(MUX_M0, &pmx0->iomg[6]); /* SD_DATA1 */
		writel(MUX_M0, &pmx0->iomg[7]); /* SD_DATA2 */
		writel(MUX_M0, &pmx0->iomg[8]); /* SD_DATA3 */

		writel(DRIVE1_10MA | BIT(2), &pmx1->iocfg[3]); /*SD_CLK*/
		writel(DRIVE1_08MA | BIT(2), &pmx1->iocfg[4]); /*SD_CMD*/
		writel(DRIVE1_08MA | BIT(2), &pmx1->iocfg[5]); /*SD_DATA0*/
		writel(DRIVE1_08MA | BIT(2), &pmx1->iocfg[6]); /*SD_DATA1*/
		writel(DRIVE1_08MA | BIT(2), &pmx1->iocfg[7]); /*SD_DATA2*/
		writel(DRIVE1_08MA | BIT(2), &pmx1->iocfg[8]); /*SD_DATA3*/
		break;

	default:
		debug("%s: invalid peripheral %d", __func__, peripheral);
		return -1;
	}

	return 0;
}

int hi6220_pinmux_config(int peripheral)
{
	switch (peripheral) {
	case PERIPH_ID_UART0:
	case PERIPH_ID_UART1:
	case PERIPH_ID_UART2:
	case PERIPH_ID_UART3:
		hi6220_uart_config(peripheral);
		break;
	case PERIPH_ID_SDMMC0:
	case PERIPH_ID_SDMMC1:
		return hi6220_mmc_config(peripheral);
	default:
		debug("%s: invalid peripheral %d", __func__, peripheral);
		return -1;
	}

	return 0;
}



// SPDX-License-Identifier: MIT
/*
 *  Copyright(c) 2015 - 2020 Xilinx, Inc.
 *
 *  Jorge Ramirez-Ortiz <jorge@foundries.io>
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/arch/hardware.h>
#include <asm/arch/ecc_spl_init.h>
#include <asm/io.h>
#include <linux/delay.h>

#define ZDMA_TRANSFER_MAX_LEN		(0x3FFFFFFFU - 7U)
#define ZDMA_CH_STATUS			((ADMA_CH0_BASEADDR) + 0x0000011CU)
#define ZDMA_CH_STATUS_STATE_MASK	0x00000003U
#define ZDMA_CH_STATUS_STATE_DONE	0x00000000U
#define ZDMA_CH_STATUS_STATE_ERR	0x00000003U
#define ZDMA_CH_CTRL0			((ADMA_CH0_BASEADDR) + 0x00000110U)
#define ZDMA_CH_CTRL0_POINT_TYPE_MASK	(u32)0x00000040U
#define ZDMA_CH_CTRL0_POINT_TYPE_NORMAL	(u32)0x00000000U
#define ZDMA_CH_CTRL0_MODE_MASK		(u32)0x00000030U
#define ZDMA_CH_CTRL0_MODE_WR_ONLY	(u32)0x00000010U
#define ZDMA_CH_CTRL0_TOTAL_BYTE_COUNT	((ADMA_CH0_BASEADDR) + 0x00000188U)
#define ZDMA_CH_WR_ONLY_WORD0		((ADMA_CH0_BASEADDR) + 0x00000148U)
#define ZDMA_CH_WR_ONLY_WORD1		((ADMA_CH0_BASEADDR) + 0x0000014CU)
#define ZDMA_CH_WR_ONLY_WORD2		((ADMA_CH0_BASEADDR) + 0x00000150U)
#define ZDMA_CH_WR_ONLY_WORD3		((ADMA_CH0_BASEADDR) + 0x00000154U)
#define ZDMA_CH_DST_DSCR_WORD0		((ADMA_CH0_BASEADDR) + 0x00000138U)
#define ZDMA_CH_DST_DSCR_WORD0_LSB_MASK	0xFFFFFFFFU
#define ZDMA_CH_DST_DSCR_WORD1		((ADMA_CH0_BASEADDR) + 0x0000013CU)
#define ZDMA_CH_DST_DSCR_WORD1_MSB_MASK	0x0001FFFFU
#define ZDMA_CH_SRC_DSCR_WORD2		((ADMA_CH0_BASEADDR) + 0x00000130U)
#define ZDMA_CH_DST_DSCR_WORD2		((ADMA_CH0_BASEADDR) + 0x00000140U)
#define ZDMA_CH_CTRL2			((ADMA_CH0_BASEADDR) + 0x00000200U)
#define ZDMA_CH_CTRL2_EN_MASK		0x00000001U
#define ZDMA_CH_ISR			((ADMA_CH0_BASEADDR) + 0x00000100U)
#define ZDMA_CH_ISR_DMA_DONE_MASK	0x00000400U
#define ECC_INIT_VAL_WORD		0xDEADBEEFU

#define ZDMA_IDLE_TIMEOUT_USEC		1000000
#define ZDMA_DONE_TIMEOUT_USEC		5000000

static void ecc_zdma_restore(void)
{
	/* Restore reset values for the DMA registers used */
	writel(ZDMA_CH_CTRL0, 0x00000080U);
	writel(ZDMA_CH_WR_ONLY_WORD0, 0x00000000U);
	writel(ZDMA_CH_WR_ONLY_WORD1, 0x00000000U);
	writel(ZDMA_CH_WR_ONLY_WORD2, 0x00000000U);
	writel(ZDMA_CH_WR_ONLY_WORD3, 0x00000000U);
	writel(ZDMA_CH_DST_DSCR_WORD0, 0x00000000U);
	writel(ZDMA_CH_DST_DSCR_WORD1, 0x00000000U);
	writel(ZDMA_CH_SRC_DSCR_WORD2, 0x00000000U);
	writel(ZDMA_CH_DST_DSCR_WORD2, 0x00000000U);
	writel(ZDMA_CH_CTRL0_TOTAL_BYTE_COUNT, 0x00000000U);
}

static void ecc_dram_bank_init(u64 addr, u64 len)
{
	bool retry = true;
	u32 timeout;
	u64 bytes;
	u32 size;
	u64 src;
	u32 reg;

	if (!len)
		return;
retry:
	bytes = len;
	src = addr;
	ecc_zdma_restore();
	while (bytes > 0) {
		size = bytes > ZDMA_TRANSFER_MAX_LEN ?
			ZDMA_TRANSFER_MAX_LEN : (u32)bytes;

		/* Wait until the DMA is in idle state */
		timeout = ZDMA_IDLE_TIMEOUT_USEC;
		do {
			udelay(1);
			reg = readl(ZDMA_CH_STATUS);
			reg &= ZDMA_CH_STATUS_STATE_MASK;
			if (!timeout--) {
				puts("error, ECC DMA failed to idle\n");
				goto done;
			}

		} while ((reg != ZDMA_CH_STATUS_STATE_DONE) &&
			(reg != ZDMA_CH_STATUS_STATE_ERR));

		/* Enable Simple (Write Only) Mode */
		reg = readl(ZDMA_CH_CTRL0);
		reg &= (ZDMA_CH_CTRL0_POINT_TYPE_MASK |
			ZDMA_CH_CTRL0_MODE_MASK);
		reg |= (ZDMA_CH_CTRL0_POINT_TYPE_NORMAL |
			ZDMA_CH_CTRL0_MODE_WR_ONLY);
		writel(reg, ZDMA_CH_CTRL0);

		/* Fill in the data to be written */
		writel(ECC_INIT_VAL_WORD, ZDMA_CH_WR_ONLY_WORD0);
		writel(ECC_INIT_VAL_WORD, ZDMA_CH_WR_ONLY_WORD1);
		writel(ECC_INIT_VAL_WORD, ZDMA_CH_WR_ONLY_WORD2);
		writel(ECC_INIT_VAL_WORD, ZDMA_CH_WR_ONLY_WORD3);

		/* Write Destination Address */
		writel((u32)(src & ZDMA_CH_DST_DSCR_WORD0_LSB_MASK),
		       ZDMA_CH_DST_DSCR_WORD0);
		writel((u32)((src >> 32) & ZDMA_CH_DST_DSCR_WORD1_MSB_MASK),
		       ZDMA_CH_DST_DSCR_WORD1);

		/* Size to be Transferred. Recommended to set both src and dest sizes */
		writel(size, ZDMA_CH_SRC_DSCR_WORD2);
		writel(size, ZDMA_CH_DST_DSCR_WORD2);

		/* DMA Enable */
		reg = readl(ZDMA_CH_CTRL2);
		reg |= ZDMA_CH_CTRL2_EN_MASK;
		writel(reg, ZDMA_CH_CTRL2);

		/* Check the status of the transfer by polling on DMA Done */
		timeout = ZDMA_DONE_TIMEOUT_USEC;
		do {
			udelay(1);
			reg = readl(ZDMA_CH_ISR);
			reg &= ZDMA_CH_ISR_DMA_DONE_MASK;
			if (!timeout--) {
				puts("error, ECC DMA timeout\n");
				goto done;
			}
		} while (reg != ZDMA_CH_ISR_DMA_DONE_MASK);

		/* Clear DMA status */
		reg = readl(ZDMA_CH_ISR);
		reg |= ZDMA_CH_ISR_DMA_DONE_MASK;
		writel(ZDMA_CH_ISR_DMA_DONE_MASK, ZDMA_CH_ISR);

		/* Read the channel status for errors */
		reg = readl(ZDMA_CH_STATUS);
		if (reg == ZDMA_CH_STATUS_STATE_ERR) {
			if (retry) {
				retry = false;
				goto retry;
			}
			puts("error, ECC DMA error\n");
			break;
		}

		bytes -= size;
		src += size;
	}
done:
	ecc_zdma_restore();
}

void zynqmp_ecc_init(void)
{
	ecc_dram_bank_init(CONFIG_SPL_ZYNQMP_DRAM_BANK1_BASE,
			   CONFIG_SPL_ZYNQMP_DRAM_BANK1_LEN);
	ecc_dram_bank_init(CONFIG_SPL_ZYNQMP_DRAM_BANK2_BASE,
			   CONFIG_SPL_ZYNQMP_DRAM_BANK2_LEN);
}

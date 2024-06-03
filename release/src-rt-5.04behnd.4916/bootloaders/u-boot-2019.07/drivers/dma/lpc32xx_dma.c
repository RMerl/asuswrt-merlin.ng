// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2008 by NXP Semiconductors
 * @Author: Kevin Wells
 * @Descr: LPC3250 DMA controller interface support functions
 *
 * Copyright (c) 2015 Tyco Fire Protection Products.
 */

#include <common.h>
#include <errno.h>
#include <asm/arch/dma.h>
#include <asm/arch/cpu.h>
#include <asm/arch/clk.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>

/* DMA controller channel register structure */
struct dmac_chan_reg {
	u32 src_addr;
	u32 dest_addr;
	u32 lli;
	u32 control;
	u32 config_ch;
	u32 reserved[3];
};

/* DMA controller register structures */
struct dma_reg {
	u32 int_stat;
	u32 int_tc_stat;
	u32 int_tc_clear;
	u32 int_err_stat;
	u32 int_err_clear;
	u32 raw_tc_stat;
	u32 raw_err_stat;
	u32 chan_enable;
	u32 sw_burst_req;
	u32 sw_single_req;
	u32 sw_last_burst_req;
	u32 sw_last_single_req;
	u32 config;
	u32 sync;
	u32 reserved[50];
	struct dmac_chan_reg dma_chan[8];
};

#define DMA_NO_OF_CHANNELS	8

/* config register definitions */
#define DMAC_CTRL_ENABLE	(1 << 0) /* For enabling the DMA controller */

static u32 alloc_ch;

static struct dma_reg *dma = (struct dma_reg *)DMA_BASE;

int lpc32xx_dma_get_channel(void)
{
	int i;

	if (!alloc_ch) { /* First time caller */
		/*
		 * DMA clock are enable by "lpc32xx_dma_init()" and should
		 * be call by board "board_early_init_f()" function.
		 */

		/*
		 * Make sure DMA controller and all channels are disabled.
		 * Controller is in little-endian mode. Disable sync signals.
		 */
		writel(0, &dma->config);
		writel(0, &dma->sync);

		/* Clear interrupt and error statuses */
		writel(0xFF, &dma->int_tc_clear);
		writel(0xFF, &dma->raw_tc_stat);
		writel(0xFF, &dma->int_err_clear);
		writel(0xFF, &dma->raw_err_stat);

		/* Enable DMA controller */
		writel(DMAC_CTRL_ENABLE, &dma->config);
	}

	i = ffz(alloc_ch);

	/* Check if all the available channels are busy */
	if (unlikely(i == DMA_NO_OF_CHANNELS))
		return -1;
	alloc_ch |= BIT_MASK(i);
	return i;
}

int lpc32xx_dma_start_xfer(unsigned int channel,
			   const struct lpc32xx_dmac_ll *desc, u32 config)
{
	if (unlikely(((BIT_MASK(channel) & alloc_ch) == 0) ||
		     (channel >= DMA_NO_OF_CHANNELS))) {
		pr_err("Request for xfer on unallocated channel %d", channel);
		return -1;
	}
	writel(BIT_MASK(channel), &dma->int_tc_clear);
	writel(BIT_MASK(channel), &dma->int_err_clear);
	writel(desc->dma_src, &dma->dma_chan[channel].src_addr);
	writel(desc->dma_dest, &dma->dma_chan[channel].dest_addr);
	writel(desc->next_lli, &dma->dma_chan[channel].lli);
	writel(desc->next_ctrl, &dma->dma_chan[channel].control);
	writel(config, &dma->dma_chan[channel].config_ch);

	return 0;
}

int lpc32xx_dma_wait_status(unsigned int channel)
{
	unsigned long start;
	u32 reg;

	/* Check if given channel is valid */
	if (unlikely(channel >= DMA_NO_OF_CHANNELS)) {
		pr_err("Request for status on unallocated channel %d", channel);
		return -1;
	}

	start = get_timer(0);
	while (1) {
		reg = readl(&dma->raw_tc_stat);
		reg |= readl(dma->raw_err_stat);
		if (reg & BIT_MASK(channel))
			break;

		if (get_timer(start) > CONFIG_SYS_HZ) {
			pr_err("DMA status timeout channel %d\n", channel);
			return -ETIMEDOUT;
		}
		udelay(1);
	}

	if (unlikely(readl(&dma->raw_err_stat) & BIT_MASK(channel))) {
		setbits_le32(&dma->int_err_clear, BIT_MASK(channel));
		setbits_le32(&dma->raw_err_stat, BIT_MASK(channel));
		pr_err("DMA error on channel %d\n", channel);
		return -1;
	}
	setbits_le32(&dma->int_tc_clear, BIT_MASK(channel));
	setbits_le32(&dma->raw_tc_stat, BIT_MASK(channel));
	return 0;
}

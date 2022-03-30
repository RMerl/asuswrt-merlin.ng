// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <common.h>
#include <clk.h>
#include <fdtdec.h>
#include <mmc.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <linux/compat.h>
#include <linux/dma-direction.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include <power/regulator.h>
#include <asm/unaligned.h>

#include "tmio-common.h"

DECLARE_GLOBAL_DATA_PTR;

static u64 tmio_sd_readq(struct tmio_sd_priv *priv, unsigned int reg)
{
	return readq(priv->regbase + (reg << 1));
}

static void tmio_sd_writeq(struct tmio_sd_priv *priv,
			       u64 val, unsigned int reg)
{
	writeq(val, priv->regbase + (reg << 1));
}

static u16 tmio_sd_readw(struct tmio_sd_priv *priv, unsigned int reg)
{
	return readw(priv->regbase + (reg >> 1));
}

static void tmio_sd_writew(struct tmio_sd_priv *priv,
			       u16 val, unsigned int reg)
{
	writew(val, priv->regbase + (reg >> 1));
}

u32 tmio_sd_readl(struct tmio_sd_priv *priv, unsigned int reg)
{
	u32 val;

	if (priv->caps & TMIO_SD_CAP_64BIT)
		return readl(priv->regbase + (reg << 1));
	else if (priv->caps & TMIO_SD_CAP_16BIT) {
		val = readw(priv->regbase + (reg >> 1)) & 0xffff;
		if ((reg == TMIO_SD_RSP10) || (reg == TMIO_SD_RSP32) ||
		    (reg == TMIO_SD_RSP54) || (reg == TMIO_SD_RSP76)) {
			val |= readw(priv->regbase + (reg >> 1) + 2) << 16;
		}
		return val;
	} else
		return readl(priv->regbase + reg);
}

void tmio_sd_writel(struct tmio_sd_priv *priv,
			       u32 val, unsigned int reg)
{
	if (priv->caps & TMIO_SD_CAP_64BIT)
		writel(val, priv->regbase + (reg << 1));
	else if (priv->caps & TMIO_SD_CAP_16BIT) {
		writew(val & 0xffff, priv->regbase + (reg >> 1));
		if (reg == TMIO_SD_INFO1 || reg == TMIO_SD_INFO1_MASK ||
		    reg == TMIO_SD_INFO2 || reg == TMIO_SD_INFO2_MASK ||
		    reg == TMIO_SD_ARG)
			writew(val >> 16, priv->regbase + (reg >> 1) + 2);
	} else
		writel(val, priv->regbase + reg);
}

static dma_addr_t __dma_map_single(void *ptr, size_t size,
				   enum dma_data_direction dir)
{
	unsigned long addr = (unsigned long)ptr;

	if (dir == DMA_FROM_DEVICE)
		invalidate_dcache_range(addr, addr + size);
	else
		flush_dcache_range(addr, addr + size);

	return addr;
}

static void __dma_unmap_single(dma_addr_t addr, size_t size,
			       enum dma_data_direction dir)
{
	if (dir != DMA_TO_DEVICE)
		invalidate_dcache_range(addr, addr + size);
}

static int tmio_sd_check_error(struct udevice *dev, struct mmc_cmd *cmd)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);
	u32 info2 = tmio_sd_readl(priv, TMIO_SD_INFO2);

	if (info2 & TMIO_SD_INFO2_ERR_RTO) {
		/*
		 * TIMEOUT must be returned for unsupported command.  Do not
		 * display error log since this might be a part of sequence to
		 * distinguish between SD and MMC.
		 */
		return -ETIMEDOUT;
	}

	if (info2 & TMIO_SD_INFO2_ERR_TO) {
		dev_err(dev, "timeout error\n");
		return -ETIMEDOUT;
	}

	if (info2 & (TMIO_SD_INFO2_ERR_END | TMIO_SD_INFO2_ERR_CRC |
		     TMIO_SD_INFO2_ERR_IDX)) {
		if ((cmd->cmdidx != MMC_CMD_SEND_TUNING_BLOCK) &&
		    (cmd->cmdidx != MMC_CMD_SEND_TUNING_BLOCK_HS200))
			dev_err(dev, "communication out of sync\n");
		return -EILSEQ;
	}

	if (info2 & (TMIO_SD_INFO2_ERR_ILA | TMIO_SD_INFO2_ERR_ILR |
		     TMIO_SD_INFO2_ERR_ILW)) {
		dev_err(dev, "illegal access\n");
		return -EIO;
	}

	return 0;
}

static int tmio_sd_wait_for_irq(struct udevice *dev, struct mmc_cmd *cmd,
				unsigned int reg, u32 flag)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);
	long wait = 1000000;
	int ret;

	while (!(tmio_sd_readl(priv, reg) & flag)) {
		if (wait-- < 0) {
			dev_err(dev, "timeout\n");
			return -ETIMEDOUT;
		}

		ret = tmio_sd_check_error(dev, cmd);
		if (ret)
			return ret;

		udelay(1);
	}

	return 0;
}

#define tmio_pio_read_fifo(__width, __suffix)				\
static void tmio_pio_read_fifo_##__width(struct tmio_sd_priv *priv,	\
					  char *pbuf, uint blksz)	\
{									\
	u##__width *buf = (u##__width *)pbuf;				\
	int i;								\
									\
	if (likely(IS_ALIGNED((uintptr_t)buf, ((__width) / 8)))) {	\
		for (i = 0; i < blksz / ((__width) / 8); i++) {		\
			*buf++ = tmio_sd_read##__suffix(priv,		\
							 TMIO_SD_BUF);	\
		}							\
	} else {							\
		for (i = 0; i < blksz / ((__width) / 8); i++) {		\
			u##__width data;				\
			data = tmio_sd_read##__suffix(priv,		\
						       TMIO_SD_BUF);	\
			put_unaligned(data, buf++);			\
		}							\
	}								\
}

tmio_pio_read_fifo(64, q)
tmio_pio_read_fifo(32, l)
tmio_pio_read_fifo(16, w)

static int tmio_sd_pio_read_one_block(struct udevice *dev, struct mmc_cmd *cmd,
				      char *pbuf, uint blocksize)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);
	int ret;

	/* wait until the buffer is filled with data */
	ret = tmio_sd_wait_for_irq(dev, cmd, TMIO_SD_INFO2,
				   TMIO_SD_INFO2_BRE);
	if (ret)
		return ret;

	/*
	 * Clear the status flag _before_ read the buffer out because
	 * TMIO_SD_INFO2_BRE is edge-triggered, not level-triggered.
	 */
	tmio_sd_writel(priv, 0, TMIO_SD_INFO2);

	if (priv->caps & TMIO_SD_CAP_64BIT)
		tmio_pio_read_fifo_64(priv, pbuf, blocksize);
	else if (priv->caps & TMIO_SD_CAP_16BIT)
		tmio_pio_read_fifo_16(priv, pbuf, blocksize);
	else
		tmio_pio_read_fifo_32(priv, pbuf, blocksize);

	return 0;
}

#define tmio_pio_write_fifo(__width, __suffix)				\
static void tmio_pio_write_fifo_##__width(struct tmio_sd_priv *priv,	\
					   const char *pbuf, uint blksz)\
{									\
	const u##__width *buf = (const u##__width *)pbuf;		\
	int i;								\
									\
	if (likely(IS_ALIGNED((uintptr_t)buf, ((__width) / 8)))) {	\
		for (i = 0; i < blksz / ((__width) / 8); i++) {		\
			tmio_sd_write##__suffix(priv, *buf++,		\
						 TMIO_SD_BUF);		\
		}							\
	} else {							\
		for (i = 0; i < blksz / ((__width) / 8); i++) {		\
			u##__width data = get_unaligned(buf++);		\
			tmio_sd_write##__suffix(priv, data,		\
						 TMIO_SD_BUF);		\
		}							\
	}								\
}

tmio_pio_write_fifo(64, q)
tmio_pio_write_fifo(32, l)
tmio_pio_write_fifo(16, w)

static int tmio_sd_pio_write_one_block(struct udevice *dev, struct mmc_cmd *cmd,
					   const char *pbuf, uint blocksize)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);
	int ret;

	/* wait until the buffer becomes empty */
	ret = tmio_sd_wait_for_irq(dev, cmd, TMIO_SD_INFO2,
				   TMIO_SD_INFO2_BWE);
	if (ret)
		return ret;

	tmio_sd_writel(priv, 0, TMIO_SD_INFO2);

	if (priv->caps & TMIO_SD_CAP_64BIT)
		tmio_pio_write_fifo_64(priv, pbuf, blocksize);
	else if (priv->caps & TMIO_SD_CAP_16BIT)
		tmio_pio_write_fifo_16(priv, pbuf, blocksize);
	else
		tmio_pio_write_fifo_32(priv, pbuf, blocksize);

	return 0;
}

static int tmio_sd_pio_xfer(struct udevice *dev, struct mmc_cmd *cmd,
			    struct mmc_data *data)
{
	const char *src = data->src;
	char *dest = data->dest;
	int i, ret;

	for (i = 0; i < data->blocks; i++) {
		if (data->flags & MMC_DATA_READ)
			ret = tmio_sd_pio_read_one_block(dev, cmd, dest,
							     data->blocksize);
		else
			ret = tmio_sd_pio_write_one_block(dev, cmd, src,
							      data->blocksize);
		if (ret)
			return ret;

		if (data->flags & MMC_DATA_READ)
			dest += data->blocksize;
		else
			src += data->blocksize;
	}

	return 0;
}

static void tmio_sd_dma_start(struct tmio_sd_priv *priv,
				  dma_addr_t dma_addr)
{
	u32 tmp;

	tmio_sd_writel(priv, 0, TMIO_SD_DMA_INFO1);
	tmio_sd_writel(priv, 0, TMIO_SD_DMA_INFO2);

	/* enable DMA */
	tmp = tmio_sd_readl(priv, TMIO_SD_EXTMODE);
	tmp |= TMIO_SD_EXTMODE_DMA_EN;
	tmio_sd_writel(priv, tmp, TMIO_SD_EXTMODE);

	tmio_sd_writel(priv, dma_addr & U32_MAX, TMIO_SD_DMA_ADDR_L);

	/* suppress the warning "right shift count >= width of type" */
	dma_addr >>= min_t(int, 32, 8 * sizeof(dma_addr));

	tmio_sd_writel(priv, dma_addr & U32_MAX, TMIO_SD_DMA_ADDR_H);

	tmio_sd_writel(priv, TMIO_SD_DMA_CTL_START, TMIO_SD_DMA_CTL);
}

static int tmio_sd_dma_wait_for_irq(struct udevice *dev, u32 flag,
					unsigned int blocks)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);
	long wait = 1000000 + 10 * blocks;

	while (!(tmio_sd_readl(priv, TMIO_SD_DMA_INFO1) & flag)) {
		if (wait-- < 0) {
			dev_err(dev, "timeout during DMA\n");
			return -ETIMEDOUT;
		}

		udelay(10);
	}

	if (tmio_sd_readl(priv, TMIO_SD_DMA_INFO2)) {
		dev_err(dev, "error during DMA\n");
		return -EIO;
	}

	return 0;
}

static int tmio_sd_dma_xfer(struct udevice *dev, struct mmc_data *data)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);
	size_t len = data->blocks * data->blocksize;
	void *buf;
	enum dma_data_direction dir;
	dma_addr_t dma_addr;
	u32 poll_flag, tmp;
	int ret;

	tmp = tmio_sd_readl(priv, TMIO_SD_DMA_MODE);

	if (data->flags & MMC_DATA_READ) {
		buf = data->dest;
		dir = DMA_FROM_DEVICE;
		/*
		 * The DMA READ completion flag position differs on Socionext
		 * and Renesas SoCs. It is bit 20 on Socionext SoCs and using
		 * bit 17 is a hardware bug and forbidden. It is either bit 17
		 * or bit 20 on Renesas SoCs, depending on SoC.
		 */
		poll_flag = priv->read_poll_flag;
		tmp |= TMIO_SD_DMA_MODE_DIR_RD;
	} else {
		buf = (void *)data->src;
		dir = DMA_TO_DEVICE;
		poll_flag = TMIO_SD_DMA_INFO1_END_WR;
		tmp &= ~TMIO_SD_DMA_MODE_DIR_RD;
	}

	tmio_sd_writel(priv, tmp, TMIO_SD_DMA_MODE);

	dma_addr = __dma_map_single(buf, len, dir);

	tmio_sd_dma_start(priv, dma_addr);

	ret = tmio_sd_dma_wait_for_irq(dev, poll_flag, data->blocks);

	if (poll_flag == TMIO_SD_DMA_INFO1_END_RD)
		udelay(1);

	__dma_unmap_single(dma_addr, len, dir);

	return ret;
}

/* check if the address is DMA'able */
static bool tmio_sd_addr_is_dmaable(const char *src)
{
	uintptr_t addr = (uintptr_t)src;

	if (!IS_ALIGNED(addr, TMIO_SD_DMA_MINALIGN))
		return false;

#if defined(CONFIG_RCAR_GEN3)
	/* Gen3 DMA has 32bit limit */
	if (addr >> 32)
		return false;
#endif

#if defined(CONFIG_ARCH_UNIPHIER) && !defined(CONFIG_ARM64) && \
	defined(CONFIG_SPL_BUILD)
	/*
	 * For UniPhier ARMv7 SoCs, the stack is allocated in the locked ways
	 * of L2, which is unreachable from the DMA engine.
	 */
	if (addr < CONFIG_SPL_STACK)
		return false;
#endif

	return true;
}

int tmio_sd_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
		      struct mmc_data *data)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);
	int ret;
	u32 tmp;

	if (tmio_sd_readl(priv, TMIO_SD_INFO2) & TMIO_SD_INFO2_CBSY) {
		dev_err(dev, "command busy\n");
		return -EBUSY;
	}

	/* clear all status flags */
	tmio_sd_writel(priv, 0, TMIO_SD_INFO1);
	tmio_sd_writel(priv, 0, TMIO_SD_INFO2);

	/* disable DMA once */
	tmp = tmio_sd_readl(priv, TMIO_SD_EXTMODE);
	tmp &= ~TMIO_SD_EXTMODE_DMA_EN;
	tmio_sd_writel(priv, tmp, TMIO_SD_EXTMODE);

	tmio_sd_writel(priv, cmd->cmdarg, TMIO_SD_ARG);

	tmp = cmd->cmdidx;

	if (data) {
		tmio_sd_writel(priv, data->blocksize, TMIO_SD_SIZE);
		tmio_sd_writel(priv, data->blocks, TMIO_SD_SECCNT);

		/* Do not send CMD12 automatically */
		tmp |= TMIO_SD_CMD_NOSTOP | TMIO_SD_CMD_DATA;

		if (data->blocks > 1)
			tmp |= TMIO_SD_CMD_MULTI;

		if (data->flags & MMC_DATA_READ)
			tmp |= TMIO_SD_CMD_RD;
	}

	/*
	 * Do not use the response type auto-detection on this hardware.
	 * CMD8, for example, has different response types on SD and eMMC,
	 * while this controller always assumes the response type for SD.
	 * Set the response type manually.
	 */
	switch (cmd->resp_type) {
	case MMC_RSP_NONE:
		tmp |= TMIO_SD_CMD_RSP_NONE;
		break;
	case MMC_RSP_R1:
		tmp |= TMIO_SD_CMD_RSP_R1;
		break;
	case MMC_RSP_R1b:
		tmp |= TMIO_SD_CMD_RSP_R1B;
		break;
	case MMC_RSP_R2:
		tmp |= TMIO_SD_CMD_RSP_R2;
		break;
	case MMC_RSP_R3:
		tmp |= TMIO_SD_CMD_RSP_R3;
		break;
	default:
		dev_err(dev, "unknown response type\n");
		return -EINVAL;
	}

	dev_dbg(dev, "sending CMD%d (SD_CMD=%08x, SD_ARG=%08x)\n",
		cmd->cmdidx, tmp, cmd->cmdarg);
	tmio_sd_writel(priv, tmp, TMIO_SD_CMD);

	ret = tmio_sd_wait_for_irq(dev, cmd, TMIO_SD_INFO1,
				   TMIO_SD_INFO1_RSP);
	if (ret)
		return ret;

	if (cmd->resp_type & MMC_RSP_136) {
		u32 rsp_127_104 = tmio_sd_readl(priv, TMIO_SD_RSP76);
		u32 rsp_103_72 = tmio_sd_readl(priv, TMIO_SD_RSP54);
		u32 rsp_71_40 = tmio_sd_readl(priv, TMIO_SD_RSP32);
		u32 rsp_39_8 = tmio_sd_readl(priv, TMIO_SD_RSP10);

		cmd->response[0] = ((rsp_127_104 & 0x00ffffff) << 8) |
				   ((rsp_103_72  & 0xff000000) >> 24);
		cmd->response[1] = ((rsp_103_72  & 0x00ffffff) << 8) |
				   ((rsp_71_40   & 0xff000000) >> 24);
		cmd->response[2] = ((rsp_71_40   & 0x00ffffff) << 8) |
				   ((rsp_39_8    & 0xff000000) >> 24);
		cmd->response[3] = (rsp_39_8     & 0xffffff)   << 8;
	} else {
		/* bit 39-8 */
		cmd->response[0] = tmio_sd_readl(priv, TMIO_SD_RSP10);
	}

	if (data) {
		/* use DMA if the HW supports it and the buffer is aligned */
		if (priv->caps & TMIO_SD_CAP_DMA_INTERNAL &&
		    tmio_sd_addr_is_dmaable(data->src))
			ret = tmio_sd_dma_xfer(dev, data);
		else
			ret = tmio_sd_pio_xfer(dev, cmd, data);
		if (ret)
			return ret;

		ret = tmio_sd_wait_for_irq(dev, cmd, TMIO_SD_INFO1,
					   TMIO_SD_INFO1_CMP);
		if (ret)
			return ret;
	}

	return tmio_sd_wait_for_irq(dev, cmd, TMIO_SD_INFO2,
				    TMIO_SD_INFO2_SCLKDIVEN);
}

static int tmio_sd_set_bus_width(struct tmio_sd_priv *priv,
				     struct mmc *mmc)
{
	u32 val, tmp;

	switch (mmc->bus_width) {
	case 0:
	case 1:
		val = TMIO_SD_OPTION_WIDTH_1;
		break;
	case 4:
		val = TMIO_SD_OPTION_WIDTH_4;
		break;
	case 8:
		val = TMIO_SD_OPTION_WIDTH_8;
		break;
	default:
		return -EINVAL;
	}

	tmp = tmio_sd_readl(priv, TMIO_SD_OPTION);
	tmp &= ~TMIO_SD_OPTION_WIDTH_MASK;
	tmp |= val;
	tmio_sd_writel(priv, tmp, TMIO_SD_OPTION);

	return 0;
}

static void tmio_sd_set_ddr_mode(struct tmio_sd_priv *priv,
				     struct mmc *mmc)
{
	u32 tmp;

	tmp = tmio_sd_readl(priv, TMIO_SD_IF_MODE);
	if (mmc->ddr_mode)
		tmp |= TMIO_SD_IF_MODE_DDR;
	else
		tmp &= ~TMIO_SD_IF_MODE_DDR;
	tmio_sd_writel(priv, tmp, TMIO_SD_IF_MODE);
}

static ulong tmio_sd_clk_get_rate(struct tmio_sd_priv *priv)
{
	return priv->clk_get_rate(priv);
}

static void tmio_sd_set_clk_rate(struct tmio_sd_priv *priv, struct mmc *mmc)
{
	unsigned int divisor;
	u32 tmp, val = 0;
	ulong mclk;

	if (mmc->clock) {
		mclk = tmio_sd_clk_get_rate(priv);

		divisor = DIV_ROUND_UP(mclk, mmc->clock);

		/* Do not set divider to 0xff in DDR mode */
		if (mmc->ddr_mode && (divisor == 1))
			divisor = 2;

		if (divisor <= 1)
			val = (priv->caps & TMIO_SD_CAP_RCAR) ?
			      TMIO_SD_CLKCTL_RCAR_DIV1 : TMIO_SD_CLKCTL_DIV1;
		else if (divisor <= 2)
			val = TMIO_SD_CLKCTL_DIV2;
		else if (divisor <= 4)
			val = TMIO_SD_CLKCTL_DIV4;
		else if (divisor <= 8)
			val = TMIO_SD_CLKCTL_DIV8;
		else if (divisor <= 16)
			val = TMIO_SD_CLKCTL_DIV16;
		else if (divisor <= 32)
			val = TMIO_SD_CLKCTL_DIV32;
		else if (divisor <= 64)
			val = TMIO_SD_CLKCTL_DIV64;
		else if (divisor <= 128)
			val = TMIO_SD_CLKCTL_DIV128;
		else if (divisor <= 256)
			val = TMIO_SD_CLKCTL_DIV256;
		else if (divisor <= 512 || !(priv->caps & TMIO_SD_CAP_DIV1024))
			val = TMIO_SD_CLKCTL_DIV512;
		else
			val = TMIO_SD_CLKCTL_DIV1024;
	}

	tmp = tmio_sd_readl(priv, TMIO_SD_CLKCTL);
	if (mmc->clock &&
	    !((tmp & TMIO_SD_CLKCTL_SCLKEN) &&
	      ((tmp & TMIO_SD_CLKCTL_DIV_MASK) == val))) {
		/*
		 * Stop the clock before changing its rate
		 * to avoid a glitch signal
		 */
		tmp &= ~TMIO_SD_CLKCTL_SCLKEN;
		tmio_sd_writel(priv, tmp, TMIO_SD_CLKCTL);

		/* Change the clock rate. */
		tmp &= ~TMIO_SD_CLKCTL_DIV_MASK;
		tmp |= val;
	}

	/* Enable or Disable the clock */
	if (mmc->clk_disable) {
		tmp |= TMIO_SD_CLKCTL_OFFEN;
		tmp &= ~TMIO_SD_CLKCTL_SCLKEN;
	} else {
		tmp &= ~TMIO_SD_CLKCTL_OFFEN;
		tmp |= TMIO_SD_CLKCTL_SCLKEN;
	}

	tmio_sd_writel(priv, tmp, TMIO_SD_CLKCTL);

	udelay(1000);
}

static void tmio_sd_set_pins(struct udevice *dev)
{
	__maybe_unused struct mmc *mmc = mmc_get_mmc_dev(dev);

#ifdef CONFIG_DM_REGULATOR
	struct tmio_sd_priv *priv = dev_get_priv(dev);

	if (priv->vqmmc_dev) {
		if (mmc->signal_voltage == MMC_SIGNAL_VOLTAGE_180)
			regulator_set_value(priv->vqmmc_dev, 1800000);
		else
			regulator_set_value(priv->vqmmc_dev, 3300000);
		regulator_set_enable(priv->vqmmc_dev, true);
	}
#endif

#ifdef CONFIG_PINCTRL
	if (mmc->signal_voltage == MMC_SIGNAL_VOLTAGE_180)
		pinctrl_select_state(dev, "state_uhs");
	else
		pinctrl_select_state(dev, "default");
#endif
}

int tmio_sd_set_ios(struct udevice *dev)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);
	struct mmc *mmc = mmc_get_mmc_dev(dev);
	int ret;

	dev_dbg(dev, "clock %uHz, DDRmode %d, width %u\n",
		mmc->clock, mmc->ddr_mode, mmc->bus_width);

	tmio_sd_set_clk_rate(priv, mmc);
	ret = tmio_sd_set_bus_width(priv, mmc);
	if (ret)
		return ret;
	tmio_sd_set_ddr_mode(priv, mmc);
	tmio_sd_set_pins(dev);

	return 0;
}

int tmio_sd_get_cd(struct udevice *dev)
{
	struct tmio_sd_priv *priv = dev_get_priv(dev);

	if (priv->caps & TMIO_SD_CAP_NONREMOVABLE)
		return 1;

	return !!(tmio_sd_readl(priv, TMIO_SD_INFO1) &
		  TMIO_SD_INFO1_CD);
}

static void tmio_sd_host_init(struct tmio_sd_priv *priv)
{
	u32 tmp;

	/* soft reset of the host */
	tmp = tmio_sd_readl(priv, TMIO_SD_SOFT_RST);
	tmp &= ~TMIO_SD_SOFT_RST_RSTX;
	tmio_sd_writel(priv, tmp, TMIO_SD_SOFT_RST);
	tmp |= TMIO_SD_SOFT_RST_RSTX;
	tmio_sd_writel(priv, tmp, TMIO_SD_SOFT_RST);

	/* FIXME: implement eMMC hw_reset */

	tmio_sd_writel(priv, TMIO_SD_STOP_SEC, TMIO_SD_STOP);

	/*
	 * Connected to 32bit AXI.
	 * This register dropped backward compatibility at version 0x10.
	 * Write an appropriate value depending on the IP version.
	 */
	if (priv->version >= 0x10) {
		if (priv->caps & TMIO_SD_CAP_64BIT)
			tmio_sd_writel(priv, 0x000, TMIO_SD_HOST_MODE);
		else
			tmio_sd_writel(priv, 0x101, TMIO_SD_HOST_MODE);
	} else {
		tmio_sd_writel(priv, 0x0, TMIO_SD_HOST_MODE);
	}

	if (priv->caps & TMIO_SD_CAP_DMA_INTERNAL) {
		tmp = tmio_sd_readl(priv, TMIO_SD_DMA_MODE);
		tmp |= TMIO_SD_DMA_MODE_ADDR_INC;
		tmio_sd_writel(priv, tmp, TMIO_SD_DMA_MODE);
	}
}

int tmio_sd_bind(struct udevice *dev)
{
	struct tmio_sd_plat *plat = dev_get_platdata(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

int tmio_sd_probe(struct udevice *dev, u32 quirks)
{
	struct tmio_sd_plat *plat = dev_get_platdata(dev);
	struct tmio_sd_priv *priv = dev_get_priv(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	fdt_addr_t base;
	ulong mclk;
	int ret;

	base = devfdt_get_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->regbase = devm_ioremap(dev, base, SZ_2K);
	if (!priv->regbase)
		return -ENOMEM;

#ifdef CONFIG_DM_REGULATOR
	device_get_supply_regulator(dev, "vqmmc-supply", &priv->vqmmc_dev);
	if (priv->vqmmc_dev)
		regulator_set_value(priv->vqmmc_dev, 3300000);
#endif

	ret = mmc_of_parse(dev, &plat->cfg);
	if (ret < 0) {
		dev_err(dev, "failed to parse host caps\n");
		return ret;
	}

	plat->cfg.name = dev->name;
	plat->cfg.host_caps |= MMC_MODE_HS_52MHz | MMC_MODE_HS;

	if (quirks)
		priv->caps = quirks;

	priv->version = tmio_sd_readl(priv, TMIO_SD_VERSION) &
						TMIO_SD_VERSION_IP;
	dev_dbg(dev, "version %x\n", priv->version);
	if (priv->version >= 0x10) {
		priv->caps |= TMIO_SD_CAP_DMA_INTERNAL;
		priv->caps |= TMIO_SD_CAP_DIV1024;
	}

	if (fdt_get_property(gd->fdt_blob, dev_of_offset(dev), "non-removable",
			     NULL))
		priv->caps |= TMIO_SD_CAP_NONREMOVABLE;

	tmio_sd_host_init(priv);

	mclk = tmio_sd_clk_get_rate(priv);

	plat->cfg.voltages = MMC_VDD_165_195 | MMC_VDD_32_33 | MMC_VDD_33_34;
	plat->cfg.f_min = mclk /
			(priv->caps & TMIO_SD_CAP_DIV1024 ? 1024 : 512);
	plat->cfg.f_max = mclk;
	if (quirks & TMIO_SD_CAP_16BIT)
		plat->cfg.b_max = U16_MAX; /* max value of TMIO_SD_SECCNT */
	else
		plat->cfg.b_max = U32_MAX; /* max value of TMIO_SD_SECCNT */

	upriv->mmc = &plat->mmc;

	return 0;
}

/*
 *	Copyright (C) 2002 Motorola GSG-China
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version 2
 *	of the License, or (at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 * Author:
 *	Darius Augulis, Teltonika Inc.
 *
 * Desc.:
 *	Implementation of I2C Adapter/Algorithm Driver
 *	for I2C Bus integrated in Freescale i.MX/MXC processors
 *
 *	Derived from Motorola GSG China I2C example driver
 *
 *	Copyright (C) 2005 Torsten Koschorrek <koschorrek at synertronixx.de
 *	Copyright (C) 2005 Matthias Blaschke <blaschke at synertronixx.de
 *	Copyright (C) 2007 RightHand Technologies, Inc.
 *	Copyright (C) 2008 Darius Augulis <darius.augulis at teltonika.lt>
 *
 *	Copyright 2013 Freescale Semiconductor, Inc.
 *
 */

/** Includes *******************************************************************
*******************************************************************************/

#include <linux/clk.h>
#include <linux/completion.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/dmapool.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_dma.h>
#include <linux/platform_data/i2c-imx.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>

/** Defines ********************************************************************
*******************************************************************************/

/* This will be the driver name the kernel reports */
#define DRIVER_NAME "imx-i2c"

/* Default value */
#define IMX_I2C_BIT_RATE	100000	/* 100kHz */

/*
 * Enable DMA if transfer byte size is bigger than this threshold.
 * As the hardware request, it must bigger than 4 bytes.\
 * I have set '16' here, maybe it's not the best but I think it's
 * the appropriate.
 */
#define DMA_THRESHOLD	16
#define DMA_TIMEOUT	1000

/* IMX I2C registers:
 * the I2C register offset is different between SoCs,
 * to provid support for all these chips, split the
 * register offset into a fixed base address and a
 * variable shift value, then the full register offset
 * will be calculated by
 * reg_off = ( reg_base_addr << reg_shift)
 */
#define IMX_I2C_IADR	0x00	/* i2c slave address */
#define IMX_I2C_IFDR	0x01	/* i2c frequency divider */
#define IMX_I2C_I2CR	0x02	/* i2c control */
#define IMX_I2C_I2SR	0x03	/* i2c status */
#define IMX_I2C_I2DR	0x04	/* i2c transfer data */

#define IMX_I2C_REGSHIFT	2
#define VF610_I2C_REGSHIFT	0

/* Bits of IMX I2C registers */
#define I2SR_RXAK	0x01
#define I2SR_IIF	0x02
#define I2SR_SRW	0x04
#define I2SR_IAL	0x10
#define I2SR_IBB	0x20
#define I2SR_IAAS	0x40
#define I2SR_ICF	0x80
#define I2CR_DMAEN	0x02
#define I2CR_RSTA	0x04
#define I2CR_TXAK	0x08
#define I2CR_MTX	0x10
#define I2CR_MSTA	0x20
#define I2CR_IIEN	0x40
#define I2CR_IEN	0x80

/* register bits different operating codes definition:
 * 1) I2SR: Interrupt flags clear operation differ between SoCs:
 * - write zero to clear(w0c) INT flag on i.MX,
 * - but write one to clear(w1c) INT flag on Vybrid.
 * 2) I2CR: I2C module enable operation also differ between SoCs:
 * - set I2CR_IEN bit enable the module on i.MX,
 * - but clear I2CR_IEN bit enable the module on Vybrid.
 */
#define I2SR_CLR_OPCODE_W0C	0x0
#define I2SR_CLR_OPCODE_W1C	(I2SR_IAL | I2SR_IIF)
#define I2CR_IEN_OPCODE_0	0x0
#define I2CR_IEN_OPCODE_1	I2CR_IEN

/** Variables ******************************************************************
*******************************************************************************/

/*
 * sorted list of clock divider, register value pairs
 * taken from table 26-5, p.26-9, Freescale i.MX
 * Integrated Portable System Processor Reference Manual
 * Document Number: MC9328MXLRM, Rev. 5.1, 06/2007
 *
 * Duplicated divider values removed from list
 */
struct imx_i2c_clk_pair {
	u16	div;
	u16	val;
};

static struct imx_i2c_clk_pair imx_i2c_clk_div[] = {
	{ 22,	0x20 }, { 24,	0x21 }, { 26,	0x22 }, { 28,	0x23 },
	{ 30,	0x00 },	{ 32,	0x24 }, { 36,	0x25 }, { 40,	0x26 },
	{ 42,	0x03 }, { 44,	0x27 },	{ 48,	0x28 }, { 52,	0x05 },
	{ 56,	0x29 }, { 60,	0x06 }, { 64,	0x2A },	{ 72,	0x2B },
	{ 80,	0x2C }, { 88,	0x09 }, { 96,	0x2D }, { 104,	0x0A },
	{ 112,	0x2E }, { 128,	0x2F }, { 144,	0x0C }, { 160,	0x30 },
	{ 192,	0x31 },	{ 224,	0x32 }, { 240,	0x0F }, { 256,	0x33 },
	{ 288,	0x10 }, { 320,	0x34 },	{ 384,	0x35 }, { 448,	0x36 },
	{ 480,	0x13 }, { 512,	0x37 }, { 576,	0x14 },	{ 640,	0x38 },
	{ 768,	0x39 }, { 896,	0x3A }, { 960,	0x17 }, { 1024,	0x3B },
	{ 1152,	0x18 }, { 1280,	0x3C }, { 1536,	0x3D }, { 1792,	0x3E },
	{ 1920,	0x1B },	{ 2048,	0x3F }, { 2304,	0x1C }, { 2560,	0x1D },
	{ 3072,	0x1E }, { 3840,	0x1F }
};

/* Vybrid VF610 clock divider, register value pairs */
static struct imx_i2c_clk_pair vf610_i2c_clk_div[] = {
	{ 20,   0x00 }, { 22,   0x01 }, { 24,   0x02 }, { 26,   0x03 },
	{ 28,   0x04 }, { 30,   0x05 }, { 32,   0x09 }, { 34,   0x06 },
	{ 36,   0x0A }, { 40,   0x07 }, { 44,   0x0C }, { 48,   0x0D },
	{ 52,   0x43 }, { 56,   0x0E }, { 60,   0x45 }, { 64,   0x12 },
	{ 68,   0x0F }, { 72,   0x13 }, { 80,   0x14 }, { 88,   0x15 },
	{ 96,   0x19 }, { 104,  0x16 }, { 112,  0x1A }, { 128,  0x17 },
	{ 136,  0x4F }, { 144,  0x1C }, { 160,  0x1D }, { 176,  0x55 },
	{ 192,  0x1E }, { 208,  0x56 }, { 224,  0x22 }, { 228,  0x24 },
	{ 240,  0x1F }, { 256,  0x23 }, { 288,  0x5C }, { 320,  0x25 },
	{ 384,  0x26 }, { 448,  0x2A }, { 480,  0x27 }, { 512,  0x2B },
	{ 576,  0x2C }, { 640,  0x2D }, { 768,  0x31 }, { 896,  0x32 },
	{ 960,  0x2F }, { 1024, 0x33 }, { 1152, 0x34 }, { 1280, 0x35 },
	{ 1536, 0x36 }, { 1792, 0x3A }, { 1920, 0x37 }, { 2048, 0x3B },
	{ 2304, 0x3C }, { 2560, 0x3D }, { 3072, 0x3E }, { 3584, 0x7A },
	{ 3840, 0x3F }, { 4096, 0x7B }, { 5120, 0x7D }, { 6144, 0x7E },
};

enum imx_i2c_type {
	IMX1_I2C,
	IMX21_I2C,
	VF610_I2C,
};

struct imx_i2c_hwdata {
	enum imx_i2c_type	devtype;
	unsigned		regshift;
	struct imx_i2c_clk_pair	*clk_div;
	unsigned		ndivs;
	unsigned		i2sr_clr_opcode;
	unsigned		i2cr_ien_opcode;
};

struct imx_i2c_dma {
	struct dma_chan		*chan_tx;
	struct dma_chan		*chan_rx;
	struct dma_chan		*chan_using;
	struct completion	cmd_complete;
	dma_addr_t		dma_buf;
	unsigned int		dma_len;
	enum dma_transfer_direction dma_transfer_dir;
	enum dma_data_direction dma_data_dir;
};

struct imx_i2c_struct {
	struct i2c_adapter	adapter;
	struct clk		*clk;
	void __iomem		*base;
	wait_queue_head_t	queue;
	unsigned long		i2csr;
	unsigned int		disable_delay;
	int			stopped;
	unsigned int		ifdr; /* IMX_I2C_IFDR */
	unsigned int		cur_clk;
	unsigned int		bitrate;
	const struct imx_i2c_hwdata	*hwdata;

	struct imx_i2c_dma	*dma;
};

static const struct imx_i2c_hwdata imx1_i2c_hwdata  = {
	.devtype		= IMX1_I2C,
	.regshift		= IMX_I2C_REGSHIFT,
	.clk_div		= imx_i2c_clk_div,
	.ndivs			= ARRAY_SIZE(imx_i2c_clk_div),
	.i2sr_clr_opcode	= I2SR_CLR_OPCODE_W0C,
	.i2cr_ien_opcode	= I2CR_IEN_OPCODE_1,

};

static const struct imx_i2c_hwdata imx21_i2c_hwdata  = {
	.devtype		= IMX21_I2C,
	.regshift		= IMX_I2C_REGSHIFT,
	.clk_div		= imx_i2c_clk_div,
	.ndivs			= ARRAY_SIZE(imx_i2c_clk_div),
	.i2sr_clr_opcode	= I2SR_CLR_OPCODE_W0C,
	.i2cr_ien_opcode	= I2CR_IEN_OPCODE_1,

};

static struct imx_i2c_hwdata vf610_i2c_hwdata = {
	.devtype		= VF610_I2C,
	.regshift		= VF610_I2C_REGSHIFT,
	.clk_div		= vf610_i2c_clk_div,
	.ndivs			= ARRAY_SIZE(vf610_i2c_clk_div),
	.i2sr_clr_opcode	= I2SR_CLR_OPCODE_W1C,
	.i2cr_ien_opcode	= I2CR_IEN_OPCODE_0,

};

static struct platform_device_id imx_i2c_devtype[] = {
	{
		.name = "imx1-i2c",
		.driver_data = (kernel_ulong_t)&imx1_i2c_hwdata,
	}, {
		.name = "imx21-i2c",
		.driver_data = (kernel_ulong_t)&imx21_i2c_hwdata,
	}, {
		/* sentinel */
	}
};
MODULE_DEVICE_TABLE(platform, imx_i2c_devtype);

static const struct of_device_id i2c_imx_dt_ids[] = {
	{ .compatible = "fsl,imx1-i2c", .data = &imx1_i2c_hwdata, },
	{ .compatible = "fsl,imx21-i2c", .data = &imx21_i2c_hwdata, },
	{ .compatible = "fsl,vf610-i2c", .data = &vf610_i2c_hwdata, },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, i2c_imx_dt_ids);

static inline int is_imx1_i2c(struct imx_i2c_struct *i2c_imx)
{
	return i2c_imx->hwdata->devtype == IMX1_I2C;
}

static inline void imx_i2c_write_reg(unsigned int val,
		struct imx_i2c_struct *i2c_imx, unsigned int reg)
{
	writeb(val, i2c_imx->base + (reg << i2c_imx->hwdata->regshift));
}

static inline unsigned char imx_i2c_read_reg(struct imx_i2c_struct *i2c_imx,
		unsigned int reg)
{
	return readb(i2c_imx->base + (reg << i2c_imx->hwdata->regshift));
}

/* Functions for DMA support */
static void i2c_imx_dma_request(struct imx_i2c_struct *i2c_imx,
						dma_addr_t phy_addr)
{
	struct imx_i2c_dma *dma;
	struct dma_slave_config dma_sconfig;
	struct device *dev = &i2c_imx->adapter.dev;
	int ret;

	dma = devm_kzalloc(dev, sizeof(*dma), GFP_KERNEL);
	if (!dma)
		return;

	dma->chan_tx = dma_request_slave_channel(dev, "tx");
	if (!dma->chan_tx) {
		dev_dbg(dev, "can't request DMA tx channel\n");
		goto fail_al;
	}

	dma_sconfig.dst_addr = phy_addr +
				(IMX_I2C_I2DR << i2c_imx->hwdata->regshift);
	dma_sconfig.dst_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
	dma_sconfig.dst_maxburst = 1;
	dma_sconfig.direction = DMA_MEM_TO_DEV;
	ret = dmaengine_slave_config(dma->chan_tx, &dma_sconfig);
	if (ret < 0) {
		dev_dbg(dev, "can't configure tx channel\n");
		goto fail_tx;
	}

	dma->chan_rx = dma_request_slave_channel(dev, "rx");
	if (!dma->chan_rx) {
		dev_dbg(dev, "can't request DMA rx channel\n");
		goto fail_tx;
	}

	dma_sconfig.src_addr = phy_addr +
				(IMX_I2C_I2DR << i2c_imx->hwdata->regshift);
	dma_sconfig.src_addr_width = DMA_SLAVE_BUSWIDTH_1_BYTE;
	dma_sconfig.src_maxburst = 1;
	dma_sconfig.direction = DMA_DEV_TO_MEM;
	ret = dmaengine_slave_config(dma->chan_rx, &dma_sconfig);
	if (ret < 0) {
		dev_dbg(dev, "can't configure rx channel\n");
		goto fail_rx;
	}

	i2c_imx->dma = dma;
	init_completion(&dma->cmd_complete);
	dev_info(dev, "using %s (tx) and %s (rx) for DMA transfers\n",
		dma_chan_name(dma->chan_tx), dma_chan_name(dma->chan_rx));

	return;

fail_rx:
	dma_release_channel(dma->chan_rx);
fail_tx:
	dma_release_channel(dma->chan_tx);
fail_al:
	devm_kfree(dev, dma);
	dev_info(dev, "can't use DMA\n");
}

static void i2c_imx_dma_callback(void *arg)
{
	struct imx_i2c_struct *i2c_imx = (struct imx_i2c_struct *)arg;
	struct imx_i2c_dma *dma = i2c_imx->dma;

	dma_unmap_single(dma->chan_using->device->dev, dma->dma_buf,
			dma->dma_len, dma->dma_data_dir);
	complete(&dma->cmd_complete);
}

static int i2c_imx_dma_xfer(struct imx_i2c_struct *i2c_imx,
					struct i2c_msg *msgs)
{
	struct imx_i2c_dma *dma = i2c_imx->dma;
	struct dma_async_tx_descriptor *txdesc;
	struct device *dev = &i2c_imx->adapter.dev;
	struct device *chan_dev = dma->chan_using->device->dev;

	dma->dma_buf = dma_map_single(chan_dev, msgs->buf,
					dma->dma_len, dma->dma_data_dir);
	if (dma_mapping_error(chan_dev, dma->dma_buf)) {
		dev_err(dev, "DMA mapping failed\n");
		goto err_map;
	}

	txdesc = dmaengine_prep_slave_single(dma->chan_using, dma->dma_buf,
					dma->dma_len, dma->dma_transfer_dir,
					DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!txdesc) {
		dev_err(dev, "Not able to get desc for DMA xfer\n");
		goto err_desc;
	}

	txdesc->callback = i2c_imx_dma_callback;
	txdesc->callback_param = i2c_imx;
	if (dma_submit_error(dmaengine_submit(txdesc))) {
		dev_err(dev, "DMA submit failed\n");
		goto err_submit;
	}

	dma_async_issue_pending(dma->chan_using);
	return 0;

err_submit:
err_desc:
	dma_unmap_single(chan_dev, dma->dma_buf,
			dma->dma_len, dma->dma_data_dir);
err_map:
	return -EINVAL;
}

static void i2c_imx_dma_free(struct imx_i2c_struct *i2c_imx)
{
	struct imx_i2c_dma *dma = i2c_imx->dma;

	dma->dma_buf = 0;
	dma->dma_len = 0;

	dma_release_channel(dma->chan_tx);
	dma->chan_tx = NULL;

	dma_release_channel(dma->chan_rx);
	dma->chan_rx = NULL;

	dma->chan_using = NULL;
}

/** Functions for IMX I2C adapter driver ***************************************
*******************************************************************************/

static int i2c_imx_bus_busy(struct imx_i2c_struct *i2c_imx, int for_busy)
{
	unsigned long orig_jiffies = jiffies;
	unsigned int temp;

	dev_dbg(&i2c_imx->adapter.dev, "<%s>\n", __func__);

	while (1) {
		temp = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2SR);

		/* check for arbitration lost */
		if (temp & I2SR_IAL) {
			temp &= ~I2SR_IAL;
			imx_i2c_write_reg(temp, i2c_imx, IMX_I2C_I2SR);
			return -EAGAIN;
		}

		if (for_busy && (temp & I2SR_IBB))
			break;
		if (!for_busy && !(temp & I2SR_IBB))
			break;
		if (time_after(jiffies, orig_jiffies + msecs_to_jiffies(500))) {
			dev_dbg(&i2c_imx->adapter.dev,
				"<%s> I2C bus is busy\n", __func__);
			return -ETIMEDOUT;
		}
		schedule();
	}

	return 0;
}

static int i2c_imx_trx_complete(struct imx_i2c_struct *i2c_imx)
{
	wait_event_timeout(i2c_imx->queue, i2c_imx->i2csr & I2SR_IIF, HZ / 10);

	if (unlikely(!(i2c_imx->i2csr & I2SR_IIF))) {
		dev_dbg(&i2c_imx->adapter.dev, "<%s> Timeout\n", __func__);
		return -ETIMEDOUT;
	}
	dev_dbg(&i2c_imx->adapter.dev, "<%s> TRX complete\n", __func__);
	i2c_imx->i2csr = 0;
	return 0;
}

static int i2c_imx_acked(struct imx_i2c_struct *i2c_imx)
{
	if (imx_i2c_read_reg(i2c_imx, IMX_I2C_I2SR) & I2SR_RXAK) {
		dev_dbg(&i2c_imx->adapter.dev, "<%s> No ACK\n", __func__);
		return -EIO;  /* No ACK */
	}

	dev_dbg(&i2c_imx->adapter.dev, "<%s> ACK received\n", __func__);
	return 0;
}

static void i2c_imx_set_clk(struct imx_i2c_struct *i2c_imx)
{
	struct imx_i2c_clk_pair *i2c_clk_div = i2c_imx->hwdata->clk_div;
	unsigned int i2c_clk_rate;
	unsigned int div;
	int i;

	/* Divider value calculation */
	i2c_clk_rate = clk_get_rate(i2c_imx->clk);
	if (i2c_imx->cur_clk == i2c_clk_rate)
		return;

	i2c_imx->cur_clk = i2c_clk_rate;

	div = (i2c_clk_rate + i2c_imx->bitrate - 1) / i2c_imx->bitrate;
	if (div < i2c_clk_div[0].div)
		i = 0;
	else if (div > i2c_clk_div[i2c_imx->hwdata->ndivs - 1].div)
		i = i2c_imx->hwdata->ndivs - 1;
	else
		for (i = 0; i2c_clk_div[i].div < div; i++)
			;

	/* Store divider value */
	i2c_imx->ifdr = i2c_clk_div[i].val;

	/*
	 * There dummy delay is calculated.
	 * It should be about one I2C clock period long.
	 * This delay is used in I2C bus disable function
	 * to fix chip hardware bug.
	 */
	i2c_imx->disable_delay = (500000U * i2c_clk_div[i].div
		+ (i2c_clk_rate / 2) - 1) / (i2c_clk_rate / 2);

#ifdef CONFIG_I2C_DEBUG_BUS
	dev_dbg(&i2c_imx->adapter.dev, "I2C_CLK=%d, REQ DIV=%d\n",
		i2c_clk_rate, div);
	dev_dbg(&i2c_imx->adapter.dev, "IFDR[IC]=0x%x, REAL DIV=%d\n",
		i2c_clk_div[i].val, i2c_clk_div[i].div);
#endif
}

static int i2c_imx_start(struct imx_i2c_struct *i2c_imx)
{
	unsigned int temp = 0;
	int result;

	dev_dbg(&i2c_imx->adapter.dev, "<%s>\n", __func__);

	i2c_imx_set_clk(i2c_imx);

	result = clk_prepare_enable(i2c_imx->clk);
	if (result)
		return result;
	imx_i2c_write_reg(i2c_imx->ifdr, i2c_imx, IMX_I2C_IFDR);
	/* Enable I2C controller */
	imx_i2c_write_reg(i2c_imx->hwdata->i2sr_clr_opcode, i2c_imx, IMX_I2C_I2SR);
	imx_i2c_write_reg(i2c_imx->hwdata->i2cr_ien_opcode, i2c_imx, IMX_I2C_I2CR);

	/* Wait controller to be stable */
	udelay(50);

	/* Start I2C transaction */
	temp = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2CR);
	temp |= I2CR_MSTA;
	imx_i2c_write_reg(temp, i2c_imx, IMX_I2C_I2CR);
	result = i2c_imx_bus_busy(i2c_imx, 1);
	if (result)
		return result;
	i2c_imx->stopped = 0;

	temp |= I2CR_IIEN | I2CR_MTX | I2CR_TXAK;
	temp &= ~I2CR_DMAEN;
	imx_i2c_write_reg(temp, i2c_imx, IMX_I2C_I2CR);
	return result;
}

static void i2c_imx_stop(struct imx_i2c_struct *i2c_imx)
{
	unsigned int temp = 0;

	if (!i2c_imx->stopped) {
		/* Stop I2C transaction */
		dev_dbg(&i2c_imx->adapter.dev, "<%s>\n", __func__);
		temp = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2CR);
		temp &= ~(I2CR_MSTA | I2CR_MTX);
		if (i2c_imx->dma)
			temp &= ~I2CR_DMAEN;
		imx_i2c_write_reg(temp, i2c_imx, IMX_I2C_I2CR);
	}
	if (is_imx1_i2c(i2c_imx)) {
		/*
		 * This delay caused by an i.MXL hardware bug.
		 * If no (or too short) delay, no "STOP" bit will be generated.
		 */
		udelay(i2c_imx->disable_delay);
	}

	if (!i2c_imx->stopped) {
		i2c_imx_bus_busy(i2c_imx, 0);
		i2c_imx->stopped = 1;
	}

	/* Disable I2C controller */
	temp = i2c_imx->hwdata->i2cr_ien_opcode ^ I2CR_IEN,
	imx_i2c_write_reg(temp, i2c_imx, IMX_I2C_I2CR);
	clk_disable_unprepare(i2c_imx->clk);
}

static irqreturn_t i2c_imx_isr(int irq, void *dev_id)
{
	struct imx_i2c_struct *i2c_imx = dev_id;
	unsigned int temp;

	temp = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2SR);
	if (temp & I2SR_IIF) {
		/* save status register */
		i2c_imx->i2csr = temp;
		temp &= ~I2SR_IIF;
		temp |= (i2c_imx->hwdata->i2sr_clr_opcode & I2SR_IIF);
		imx_i2c_write_reg(temp, i2c_imx, IMX_I2C_I2SR);
		wake_up(&i2c_imx->queue);
		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

static int i2c_imx_dma_write(struct imx_i2c_struct *i2c_imx,
					struct i2c_msg *msgs)
{
	int result;
	unsigned long time_left;
	unsigned int temp = 0;
	unsigned long orig_jiffies = jiffies;
	struct imx_i2c_dma *dma = i2c_imx->dma;
	struct device *dev = &i2c_imx->adapter.dev;

	dma->chan_using = dma->chan_tx;
	dma->dma_transfer_dir = DMA_MEM_TO_DEV;
	dma->dma_data_dir = DMA_TO_DEVICE;
	dma->dma_len = msgs->len - 1;
	result = i2c_imx_dma_xfer(i2c_imx, msgs);
	if (result)
		return result;

	temp = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2CR);
	temp |= I2CR_DMAEN;
	imx_i2c_write_reg(temp, i2c_imx, IMX_I2C_I2CR);

	/*
	 * Write slave address.
	 * The first byte must be transmitted by the CPU.
	 */
	imx_i2c_write_reg(msgs->addr << 1, i2c_imx, IMX_I2C_I2DR);
	reinit_completion(&i2c_imx->dma->cmd_complete);
	time_left = wait_for_completion_timeout(
				&i2c_imx->dma->cmd_complete,
				msecs_to_jiffies(DMA_TIMEOUT));
	if (time_left == 0) {
		dmaengine_terminate_all(dma->chan_using);
		return -ETIMEDOUT;
	}

	/* Waiting for transfer complete. */
	while (1) {
		temp = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2SR);
		if (temp & I2SR_ICF)
			break;
		if (time_after(jiffies, orig_jiffies +
				msecs_to_jiffies(DMA_TIMEOUT))) {
			dev_dbg(dev, "<%s> Timeout\n", __func__);
			return -ETIMEDOUT;
		}
		schedule();
	}

	temp = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2CR);
	temp &= ~I2CR_DMAEN;
	imx_i2c_write_reg(temp, i2c_imx, IMX_I2C_I2CR);

	/* The last data byte must be transferred by the CPU. */
	imx_i2c_write_reg(msgs->buf[msgs->len-1],
				i2c_imx, IMX_I2C_I2DR);
	result = i2c_imx_trx_complete(i2c_imx);
	if (result)
		return result;

	return i2c_imx_acked(i2c_imx);
}

static int i2c_imx_dma_read(struct imx_i2c_struct *i2c_imx,
			struct i2c_msg *msgs, bool is_lastmsg)
{
	int result;
	unsigned long time_left;
	unsigned int temp;
	unsigned long orig_jiffies = jiffies;
	struct imx_i2c_dma *dma = i2c_imx->dma;
	struct device *dev = &i2c_imx->adapter.dev;

	temp = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2CR);
	temp |= I2CR_DMAEN;
	imx_i2c_write_reg(temp, i2c_imx, IMX_I2C_I2CR);

	dma->chan_using = dma->chan_rx;
	dma->dma_transfer_dir = DMA_DEV_TO_MEM;
	dma->dma_data_dir = DMA_FROM_DEVICE;
	/* The last two data bytes must be transferred by the CPU. */
	dma->dma_len = msgs->len - 2;
	result = i2c_imx_dma_xfer(i2c_imx, msgs);
	if (result)
		return result;

	reinit_completion(&i2c_imx->dma->cmd_complete);
	time_left = wait_for_completion_timeout(
				&i2c_imx->dma->cmd_complete,
				msecs_to_jiffies(DMA_TIMEOUT));
	if (time_left == 0) {
		dmaengine_terminate_all(dma->chan_using);
		return -ETIMEDOUT;
	}

	/* waiting for transfer complete. */
	while (1) {
		temp = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2SR);
		if (temp & I2SR_ICF)
			break;
		if (time_after(jiffies, orig_jiffies +
				msecs_to_jiffies(DMA_TIMEOUT))) {
			dev_dbg(dev, "<%s> Timeout\n", __func__);
			return -ETIMEDOUT;
		}
		schedule();
	}

	temp = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2CR);
	temp &= ~I2CR_DMAEN;
	imx_i2c_write_reg(temp, i2c_imx, IMX_I2C_I2CR);

	/* read n-1 byte data */
	temp = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2CR);
	temp |= I2CR_TXAK;
	imx_i2c_write_reg(temp, i2c_imx, IMX_I2C_I2CR);

	msgs->buf[msgs->len-2] = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2DR);
	/* read n byte data */
	result = i2c_imx_trx_complete(i2c_imx);
	if (result)
		return result;

	if (is_lastmsg) {
		/*
		 * It must generate STOP before read I2DR to prevent
		 * controller from generating another clock cycle
		 */
		dev_dbg(dev, "<%s> clear MSTA\n", __func__);
		temp = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2CR);
		temp &= ~(I2CR_MSTA | I2CR_MTX);
		imx_i2c_write_reg(temp, i2c_imx, IMX_I2C_I2CR);
		i2c_imx_bus_busy(i2c_imx, 0);
		i2c_imx->stopped = 1;
	} else {
		/*
		 * For i2c master receiver repeat restart operation like:
		 * read -> repeat MSTA -> read/write
		 * The controller must set MTX before read the last byte in
		 * the first read operation, otherwise the first read cost
		 * one extra clock cycle.
		 */
		temp = readb(i2c_imx->base + IMX_I2C_I2CR);
		temp |= I2CR_MTX;
		writeb(temp, i2c_imx->base + IMX_I2C_I2CR);
	}
	msgs->buf[msgs->len-1] = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2DR);

	return 0;
}

static int i2c_imx_write(struct imx_i2c_struct *i2c_imx, struct i2c_msg *msgs)
{
	int i, result;

	dev_dbg(&i2c_imx->adapter.dev, "<%s> write slave address: addr=0x%x\n",
		__func__, msgs->addr << 1);

	/* write slave address */
	imx_i2c_write_reg(msgs->addr << 1, i2c_imx, IMX_I2C_I2DR);
	result = i2c_imx_trx_complete(i2c_imx);
	if (result)
		return result;
	result = i2c_imx_acked(i2c_imx);
	if (result)
		return result;
	dev_dbg(&i2c_imx->adapter.dev, "<%s> write data\n", __func__);

	/* write data */
	for (i = 0; i < msgs->len; i++) {
		dev_dbg(&i2c_imx->adapter.dev,
			"<%s> write byte: B%d=0x%X\n",
			__func__, i, msgs->buf[i]);
		imx_i2c_write_reg(msgs->buf[i], i2c_imx, IMX_I2C_I2DR);
		result = i2c_imx_trx_complete(i2c_imx);
		if (result)
			return result;
		result = i2c_imx_acked(i2c_imx);
		if (result)
			return result;
	}
	return 0;
}

static int i2c_imx_read(struct imx_i2c_struct *i2c_imx, struct i2c_msg *msgs, bool is_lastmsg)
{
	int i, result;
	unsigned int temp;
	int block_data = msgs->flags & I2C_M_RECV_LEN;

	dev_dbg(&i2c_imx->adapter.dev,
		"<%s> write slave address: addr=0x%x\n",
		__func__, (msgs->addr << 1) | 0x01);

	/* write slave address */
	imx_i2c_write_reg((msgs->addr << 1) | 0x01, i2c_imx, IMX_I2C_I2DR);
	result = i2c_imx_trx_complete(i2c_imx);
	if (result)
		return result;
	result = i2c_imx_acked(i2c_imx);
	if (result)
		return result;

	dev_dbg(&i2c_imx->adapter.dev, "<%s> setup bus\n", __func__);

	/* setup bus to read data */
	temp = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2CR);
	temp &= ~I2CR_MTX;

	/*
	 * Reset the I2CR_TXAK flag initially for SMBus block read since the
	 * length is unknown
	 */
	if ((msgs->len - 1) || block_data)
		temp &= ~I2CR_TXAK;
	imx_i2c_write_reg(temp, i2c_imx, IMX_I2C_I2CR);
	imx_i2c_read_reg(i2c_imx, IMX_I2C_I2DR); /* dummy read */

	dev_dbg(&i2c_imx->adapter.dev, "<%s> read data\n", __func__);

	if (i2c_imx->dma && msgs->len >= DMA_THRESHOLD && !block_data)
		return i2c_imx_dma_read(i2c_imx, msgs, is_lastmsg);

	/* read data */
	for (i = 0; i < msgs->len; i++) {
		u8 len = 0;

		result = i2c_imx_trx_complete(i2c_imx);
		if (result)
			return result;
		/*
		 * First byte is the length of remaining packet
		 * in the SMBus block data read. Add it to
		 * msgs->len.
		 */
		if ((!i) && block_data) {
			len = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2DR);
			if ((len == 0) || (len > I2C_SMBUS_BLOCK_MAX))
				return -EPROTO;
			dev_dbg(&i2c_imx->adapter.dev,
				"<%s> read length: 0x%X\n",
				__func__, len);
			msgs->len += len;
		}
		if (i == (msgs->len - 1)) {
			if (is_lastmsg) {
				/*
				 * It must generate STOP before read I2DR to prevent
				 * controller from generating another clock cycle
				 */
				dev_dbg(&i2c_imx->adapter.dev,
					"<%s> clear MSTA\n", __func__);
				temp = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2CR);
				temp &= ~(I2CR_MSTA | I2CR_MTX);
				imx_i2c_write_reg(temp, i2c_imx, IMX_I2C_I2CR);
				i2c_imx_bus_busy(i2c_imx, 0);
				i2c_imx->stopped = 1;
			} else {
				/*
				 * For i2c master receiver repeat restart operation like:
				 * read -> repeat MSTA -> read/write
				 * The controller must set MTX before read the last byte in
				 * the first read operation, otherwise the first read cost
				 * one extra clock cycle.
				 */
				temp = readb(i2c_imx->base + IMX_I2C_I2CR);
				temp |= I2CR_MTX;
				writeb(temp, i2c_imx->base + IMX_I2C_I2CR);
			}
		} else if (i == (msgs->len - 2)) {
			dev_dbg(&i2c_imx->adapter.dev,
				"<%s> set TXAK\n", __func__);
			temp = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2CR);
			temp |= I2CR_TXAK;
			imx_i2c_write_reg(temp, i2c_imx, IMX_I2C_I2CR);
		}
		if ((!i) && block_data)
			msgs->buf[0] = len;
		else
			msgs->buf[i] =  imx_i2c_read_reg(i2c_imx, IMX_I2C_I2DR);
		dev_dbg(&i2c_imx->adapter.dev,
			"<%s> read byte: B%d=0x%X\n",
			__func__, i, msgs->buf[i]);
	}
	return 0;
}

static int i2c_imx_xfer(struct i2c_adapter *adapter,
						struct i2c_msg *msgs, int num)
{
	unsigned int i, temp;
	int result;
	bool is_lastmsg = false;
	struct imx_i2c_struct *i2c_imx = i2c_get_adapdata(adapter);

	dev_dbg(&i2c_imx->adapter.dev, "<%s>\n", __func__);

	/* Start I2C transfer */
	result = i2c_imx_start(i2c_imx);
	if (result)
		goto fail0;

	/* read/write data */
	for (i = 0; i < num; i++) {
		if (i == num - 1)
			is_lastmsg = true;

		if (i) {
			dev_dbg(&i2c_imx->adapter.dev,
				"<%s> repeated start\n", __func__);
			temp = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2CR);
			temp |= I2CR_RSTA;
			imx_i2c_write_reg(temp, i2c_imx, IMX_I2C_I2CR);
			result =  i2c_imx_bus_busy(i2c_imx, 1);
			if (result)
				goto fail0;
		}
		dev_dbg(&i2c_imx->adapter.dev,
			"<%s> transfer message: %d\n", __func__, i);
		/* write/read data */
#ifdef CONFIG_I2C_DEBUG_BUS
		temp = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2CR);
		dev_dbg(&i2c_imx->adapter.dev,
			"<%s> CONTROL: IEN=%d, IIEN=%d, MSTA=%d, MTX=%d, TXAK=%d, RSTA=%d\n",
			__func__,
			(temp & I2CR_IEN ? 1 : 0), (temp & I2CR_IIEN ? 1 : 0),
			(temp & I2CR_MSTA ? 1 : 0), (temp & I2CR_MTX ? 1 : 0),
			(temp & I2CR_TXAK ? 1 : 0), (temp & I2CR_RSTA ? 1 : 0));
		temp = imx_i2c_read_reg(i2c_imx, IMX_I2C_I2SR);
		dev_dbg(&i2c_imx->adapter.dev,
			"<%s> STATUS: ICF=%d, IAAS=%d, IBB=%d, IAL=%d, SRW=%d, IIF=%d, RXAK=%d\n",
			__func__,
			(temp & I2SR_ICF ? 1 : 0), (temp & I2SR_IAAS ? 1 : 0),
			(temp & I2SR_IBB ? 1 : 0), (temp & I2SR_IAL ? 1 : 0),
			(temp & I2SR_SRW ? 1 : 0), (temp & I2SR_IIF ? 1 : 0),
			(temp & I2SR_RXAK ? 1 : 0));
#endif
		if (msgs[i].flags & I2C_M_RD)
			result = i2c_imx_read(i2c_imx, &msgs[i], is_lastmsg);
		else {
			if (i2c_imx->dma && msgs[i].len >= DMA_THRESHOLD)
				result = i2c_imx_dma_write(i2c_imx, &msgs[i]);
			else
				result = i2c_imx_write(i2c_imx, &msgs[i]);
		}
		if (result)
			goto fail0;
	}

fail0:
	/* Stop I2C transfer */
	i2c_imx_stop(i2c_imx);

	dev_dbg(&i2c_imx->adapter.dev, "<%s> exit with: %s: %d\n", __func__,
		(result < 0) ? "error" : "success msg",
			(result < 0) ? result : num);
	return (result < 0) ? result : num;
}

static u32 i2c_imx_func(struct i2c_adapter *adapter)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL
		| I2C_FUNC_SMBUS_READ_BLOCK_DATA;
}

static struct i2c_algorithm i2c_imx_algo = {
	.master_xfer	= i2c_imx_xfer,
	.functionality	= i2c_imx_func,
};

static int i2c_imx_probe(struct platform_device *pdev)
{
	const struct of_device_id *of_id = of_match_device(i2c_imx_dt_ids,
							   &pdev->dev);
	struct imx_i2c_struct *i2c_imx;
	struct resource *res;
	struct imxi2c_platform_data *pdata = dev_get_platdata(&pdev->dev);
	void __iomem *base;
	int irq, ret;
	dma_addr_t phy_addr;

	dev_dbg(&pdev->dev, "<%s>\n", __func__);

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "can't get irq number\n");
		return irq;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(base))
		return PTR_ERR(base);

	phy_addr = (dma_addr_t)res->start;
	i2c_imx = devm_kzalloc(&pdev->dev, sizeof(*i2c_imx), GFP_KERNEL);
	if (!i2c_imx)
		return -ENOMEM;

	if (of_id)
		i2c_imx->hwdata = of_id->data;
	else
		i2c_imx->hwdata = (struct imx_i2c_hwdata *)
				platform_get_device_id(pdev)->driver_data;

	/* Setup i2c_imx driver structure */
	strlcpy(i2c_imx->adapter.name, pdev->name, sizeof(i2c_imx->adapter.name));
	i2c_imx->adapter.owner		= THIS_MODULE;
	i2c_imx->adapter.algo		= &i2c_imx_algo;
	i2c_imx->adapter.dev.parent	= &pdev->dev;
	i2c_imx->adapter.nr		= pdev->id;
	i2c_imx->adapter.dev.of_node	= pdev->dev.of_node;
	i2c_imx->base			= base;

	/* Get I2C clock */
	i2c_imx->clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(i2c_imx->clk)) {
		dev_err(&pdev->dev, "can't get I2C clock\n");
		return PTR_ERR(i2c_imx->clk);
	}

	ret = clk_prepare_enable(i2c_imx->clk);
	if (ret) {
		dev_err(&pdev->dev, "can't enable I2C clock\n");
		return ret;
	}
	/* Request IRQ */
	ret = devm_request_irq(&pdev->dev, irq, i2c_imx_isr, 0,
				pdev->name, i2c_imx);
	if (ret) {
		dev_err(&pdev->dev, "can't claim irq %d\n", irq);
		goto clk_disable;
	}

	/* Init queue */
	init_waitqueue_head(&i2c_imx->queue);

	/* Set up adapter data */
	i2c_set_adapdata(&i2c_imx->adapter, i2c_imx);

	/* Set up clock divider */
	i2c_imx->bitrate = IMX_I2C_BIT_RATE;
	ret = of_property_read_u32(pdev->dev.of_node,
				   "clock-frequency", &i2c_imx->bitrate);
	if (ret < 0 && pdata && pdata->bitrate)
		i2c_imx->bitrate = pdata->bitrate;

	/* Set up chip registers to defaults */
	imx_i2c_write_reg(i2c_imx->hwdata->i2cr_ien_opcode ^ I2CR_IEN,
			i2c_imx, IMX_I2C_I2CR);
	imx_i2c_write_reg(i2c_imx->hwdata->i2sr_clr_opcode, i2c_imx, IMX_I2C_I2SR);

	/* Add I2C adapter */
	ret = i2c_add_numbered_adapter(&i2c_imx->adapter);
	if (ret < 0) {
		dev_err(&pdev->dev, "registration failed\n");
		goto clk_disable;
	}

	/* Set up platform driver data */
	platform_set_drvdata(pdev, i2c_imx);
	clk_disable_unprepare(i2c_imx->clk);

	dev_dbg(&i2c_imx->adapter.dev, "claimed irq %d\n", irq);
	dev_dbg(&i2c_imx->adapter.dev, "device resources: %pR\n", res);
	dev_dbg(&i2c_imx->adapter.dev, "adapter name: \"%s\"\n",
		i2c_imx->adapter.name);
	dev_info(&i2c_imx->adapter.dev, "IMX I2C adapter registered\n");

	/* Init DMA config if supported */
	i2c_imx_dma_request(i2c_imx, phy_addr);

	return 0;   /* Return OK */

clk_disable:
	clk_disable_unprepare(i2c_imx->clk);
	return ret;
}

static int i2c_imx_remove(struct platform_device *pdev)
{
	struct imx_i2c_struct *i2c_imx = platform_get_drvdata(pdev);

	/* remove adapter */
	dev_dbg(&i2c_imx->adapter.dev, "adapter removed\n");
	i2c_del_adapter(&i2c_imx->adapter);

	if (i2c_imx->dma)
		i2c_imx_dma_free(i2c_imx);

	/* setup chip registers to defaults */
	imx_i2c_write_reg(0, i2c_imx, IMX_I2C_IADR);
	imx_i2c_write_reg(0, i2c_imx, IMX_I2C_IFDR);
	imx_i2c_write_reg(0, i2c_imx, IMX_I2C_I2CR);
	imx_i2c_write_reg(0, i2c_imx, IMX_I2C_I2SR);

	return 0;
}

static struct platform_driver i2c_imx_driver = {
	.probe = i2c_imx_probe,
	.remove = i2c_imx_remove,
	.driver	= {
		.name	= DRIVER_NAME,
		.of_match_table = i2c_imx_dt_ids,
	},
	.id_table	= imx_i2c_devtype,
};

static int __init i2c_adap_imx_init(void)
{
	return platform_driver_register(&i2c_imx_driver);
}
subsys_initcall(i2c_adap_imx_init);

static void __exit i2c_adap_imx_exit(void)
{
	platform_driver_unregister(&i2c_imx_driver);
}
module_exit(i2c_adap_imx_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Darius Augulis");
MODULE_DESCRIPTION("I2C adapter driver for IMX I2C bus");
MODULE_ALIAS("platform:" DRIVER_NAME);

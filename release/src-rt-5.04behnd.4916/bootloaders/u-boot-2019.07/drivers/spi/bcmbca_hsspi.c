// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * Derived from linux/drivers/spi/spi-bcm63xx-hsspi.c:
 *	Copyright (C) 2000-2010 Broadcom Corporation
 *	Copyright (C) 2012-2013 Jonas Gorski <jogo@openwrt.org>
 *	Copyright (C) 2021 Broadcom Ltd
 */

#include <common.h>
#include <asm/io.h>
#include <clk.h>
#include <spi.h>
#include <reset.h>
#include <wait_bit.h>
#include <dm.h>

#define HSSPI_PP			0

#define SPI_MAX_SYNC_CLOCK		30000000

/* SPI Control register */
#define SPI_CTL_REG			0x000
#define SPI_CTL_CS_POL_SHIFT		0
#define SPI_CTL_CS_POL_MASK		(0xff << SPI_CTL_CS_POL_SHIFT)
#define SPI_CTL_CLK_GATE_SHIFT		16
#define SPI_CTL_CLK_GATE_MASK		(1 << SPI_CTL_CLK_GATE_SHIFT)
#define SPI_CTL_CLK_POL_SHIFT		17
#define SPI_CTL_CLK_POL_MASK		(1 << SPI_CTL_CLK_POL_SHIFT)

/* SPI Interrupts registers */
#define SPI_IR_STAT_REG			0x008
#define SPI_IR_ST_MASK_REG		0x00c
#define SPI_IR_MASK_REG			0x010

#define SPI_IR_CLEAR_ALL		0xff001f1f

/* SPI Ping-Pong Command registers */
#define SPI_CMD_REG			(0x080 + (0x40 * (HSSPI_PP)) + 0x00)
#define SPI_CMD_OP_SHIFT		0
#define SPI_CMD_OP_START		(0x1 << SPI_CMD_OP_SHIFT)
#define SPI_CMD_PFL_SHIFT		8
#define SPI_CMD_PFL_MASK		(0x7 << SPI_CMD_PFL_SHIFT)
#define SPI_CMD_SLAVE_SHIFT		12
#define SPI_CMD_SLAVE_MASK		(0x7 << SPI_CMD_SLAVE_SHIFT)

/* SPI Ping-Pong Status registers */
#define SPI_STAT_REG			(0x080 + (0x40 * (HSSPI_PP)) + 0x04)
#define SPI_STAT_SRCBUSY_SHIFT		1
#define SPI_STAT_SRCBUSY_MASK		(1 << SPI_STAT_SRCBUSY_SHIFT)

/* SPI Profile Clock registers */
#define SPI_PFL_CLK_REG(x)		(0x100 + (0x20 * (x)) + 0x00)
#define SPI_PFL_CLK_FREQ_SHIFT		0
#define SPI_PFL_CLK_FREQ_MASK		(0x3fff << SPI_PFL_CLK_FREQ_SHIFT)
#define SPI_PFL_CLK_RSTLOOP_SHIFT	15
#define SPI_PFL_CLK_RSTLOOP_MASK	(1 << SPI_PFL_CLK_RSTLOOP_SHIFT)

/* SPI Profile Signal registers */
#define SPI_PFL_SIG_REG(x)		(0x100 + (0x20 * (x)) + 0x04)
#define SPI_PFL_SIG_LATCHRIS_SHIFT	12
#define SPI_PFL_SIG_LATCHRIS_MASK	(1 << SPI_PFL_SIG_LATCHRIS_SHIFT)
#define SPI_PFL_SIG_LAUNCHRIS_SHIFT	13
#define SPI_PFL_SIG_LAUNCHRIS_MASK	(1 << SPI_PFL_SIG_LAUNCHRIS_SHIFT)
#define SPI_PFL_SIG_ASYNCIN_SHIFT	16
#define SPI_PFL_SIG_ASYNCIN_MASK	(1 << SPI_PFL_SIG_ASYNCIN_SHIFT)

/* SPI Profile Mode registers */
#define SPI_PFL_MODE_REG(x)		(0x100 + (0x20 * (x)) + 0x08)
#define SPI_PFL_MODE_FILL_SHIFT		0
#define SPI_PFL_MODE_FILL_MASK		(0xff << SPI_PFL_MODE_FILL_SHIFT)
#define SPI_PFL_MODE_MDRDSZ_SHIFT	16
#define SPI_PFL_MODE_MDRDSZ_MASK	(1 << SPI_PFL_MODE_MDRDSZ_SHIFT)
#define SPI_PFL_MODE_MDWRSZ_SHIFT	18
#define SPI_PFL_MODE_MDWRSZ_MASK	(1 << SPI_PFL_MODE_MDWRSZ_SHIFT)
#define SPI_PFL_MODE_3WIRE_SHIFT	20
#define SPI_PFL_MODE_3WIRE_MASK		(1 << SPI_PFL_MODE_3WIRE_SHIFT)
#define SPI_PFL_MODE_PREPEND_CNT_SHIFT	24

/* SPI Ping-Pong FIFO registers */
#define HSSPI_FIFO_SIZE			0x200
#define HSSPI_FIFO_BASE			(0x200 + \
					 (HSSPI_FIFO_SIZE * HSSPI_PP))

/* SPI Ping-Pong FIFO OP register */
#define HSSPI_FIFO_OP_SIZE		0x2
#define HSSPI_FIFO_OP_REG		(HSSPI_FIFO_BASE + 0x00)
#define HSSPI_FIFO_OP_BYTES_SHIFT	0
#define HSSPI_FIFO_OP_BYTES_MASK	(0x3ff << HSSPI_FIFO_OP_BYTES_SHIFT)
#define HSSPI_FIFO_OP_MBIT_SHIFT	11
#define HSSPI_FIFO_OP_MBIT_MASK		(1 << HSSPI_FIFO_OP_MBIT_SHIFT)
#define HSSPI_FIFO_OP_CODE_SHIFT	13
#define HSSPI_FIFO_OP_READ_WRITE	(1 << HSSPI_FIFO_OP_CODE_SHIFT)
#define HSSPI_FIFO_OP_CODE_W		(2 << HSSPI_FIFO_OP_CODE_SHIFT)
#define HSSPI_FIFO_OP_CODE_R		(3 << HSSPI_FIFO_OP_CODE_SHIFT)

#define HSSPI_MAX_DATA_SIZE		(HSSPI_FIFO_SIZE - HSSPI_FIFO_OP_SIZE)


#define SPIM_CTRL_CS_OVERRIDE_SEL_SHIFT		0
#define SPIM_CTRL_CS_OVERRIDE_SEL_MASK		0xff
#define SPIM_CTRL_CS_OVERRIDE_VAL_SHIFT		8
#define SPIM_CTRL_CS_OVERRIDE_VAL_MASK		0xff

struct bcmbca_hsspi_priv {
	void __iomem *regs;
	void __iomem *spim_ctrl;
	ulong clk_rate;
	uint8_t num_cs;
	uint8_t cs_pols;
	uint speed;
	uint max_speed;
};

static int bcmbca_hsspi_cs_info(struct udevice *bus, uint cs,
			   struct spi_cs_info *info)
{
	struct bcmbca_hsspi_priv *priv = dev_get_priv(bus);

	if (cs >= priv->num_cs) {
		dev_err(bus, "no cs %u\n", cs);
		return -ENODEV;
	}

	return 0;
}

static int bcmbca_hsspi_set_mode(struct udevice *bus, uint mode)
{
	struct bcmbca_hsspi_priv *priv = dev_get_priv(bus);

	/* clock polarity */
	if (mode & SPI_CPOL)
		setbits_32(priv->regs + SPI_CTL_REG, SPI_CTL_CLK_POL_MASK);
	else
		clrbits_32(priv->regs + SPI_CTL_REG, SPI_CTL_CLK_POL_MASK);

	return 0;
}

static int bcmbca_hsspi_set_speed(struct udevice *bus, uint speed)
{
	struct bcmbca_hsspi_priv *priv = dev_get_priv(bus);

	if (priv->max_speed && speed > priv->max_speed) {
		priv->speed = priv->max_speed;
	} else
		priv->speed = speed;

	return 0;
}

static void bcmbca_hsspi_setup_clock(struct bcmbca_hsspi_priv *priv,
				   struct dm_spi_slave_platdata *plat)
{
	uint32_t clr, set;

	/* profile clock */
	set = DIV_ROUND_UP(priv->clk_rate, priv->speed);
	set = DIV_ROUND_UP(2048, set);
	set &= SPI_PFL_CLK_FREQ_MASK;
	set |= SPI_PFL_CLK_RSTLOOP_MASK;
	writel(set, priv->regs + SPI_PFL_CLK_REG(plat->cs));

	/* profile signal */
	set = 0;
	clr = SPI_PFL_SIG_LAUNCHRIS_MASK |
	      SPI_PFL_SIG_LATCHRIS_MASK |
	      SPI_PFL_SIG_ASYNCIN_MASK;

	/* latch/launch config */
	if (plat->mode & SPI_CPHA)
		set |= SPI_PFL_SIG_LAUNCHRIS_MASK;
	else
		set |= SPI_PFL_SIG_LATCHRIS_MASK;

	/* async clk */
	if (priv->speed > SPI_MAX_SYNC_CLOCK)
		set |= SPI_PFL_SIG_ASYNCIN_MASK;

	clrsetbits_32(priv->regs + SPI_PFL_SIG_REG(plat->cs), clr, set);

	/* global control */
	set = 0;
	clr = 0;

 	if (priv->cs_pols & BIT(plat->cs))
		set |= BIT(plat->cs);
	else
		clr |= BIT(plat->cs);

	clrsetbits_32(priv->regs + SPI_CTL_REG, clr, set);
}

static void bcmbca_hsspi_activate_cs(struct bcmbca_hsspi_priv *priv,
				   struct dm_spi_slave_platdata *plat)										
{
	uint32_t val;

	/* set the override bit and cs active value */
	val = readl(priv->spim_ctrl);
	val |= BIT(plat->cs+SPIM_CTRL_CS_OVERRIDE_SEL_SHIFT);
	val &= ~BIT(plat->cs+SPIM_CTRL_CS_OVERRIDE_VAL_SHIFT);
	if (priv->cs_pols & BIT(plat->cs))
		val |= BIT(plat->cs+SPIM_CTRL_CS_OVERRIDE_VAL_SHIFT);
	writel(val, priv->spim_ctrl);
}

static void bcmbca_hsspi_deactivate_cs(struct bcmbca_hsspi_priv *priv,
				   struct dm_spi_slave_platdata *plat)										
{
	uint32_t val;

	/* clear the cs override bit */	  
	val = readl(priv->spim_ctrl);
	val &= ~BIT(plat->cs+SPIM_CTRL_CS_OVERRIDE_SEL_SHIFT);
	writel(val, priv->spim_ctrl);
}

static int bcmbca_hsspi_xfer(struct udevice *dev, unsigned int bitlen,
		const void *dout, void *din, unsigned long flags)
{
	struct bcmbca_hsspi_priv *priv = dev_get_priv(dev->parent);
	struct dm_spi_slave_platdata *plat = dev_get_parent_platdata(dev);
	size_t data_bytes = bitlen / 8;
	size_t step_size = HSSPI_FIFO_SIZE;
	uint16_t opcode = 0;
	uint32_t val = SPI_PFL_MODE_FILL_MASK;
	const uint8_t *tx = dout;
	uint8_t *rx = din;
	uint32_t cs_act = 0;
 

	if (flags & SPI_XFER_BEGIN)
		bcmbca_hsspi_setup_clock(priv, plat);

	/* fifo operation */
	if (tx && rx)
		opcode = HSSPI_FIFO_OP_READ_WRITE;
	else if (rx)
		opcode = HSSPI_FIFO_OP_CODE_R;
	else if (tx)
		opcode = HSSPI_FIFO_OP_CODE_W;

	if (opcode != HSSPI_FIFO_OP_CODE_R)
		step_size -= HSSPI_FIFO_OP_SIZE;

	/* dual mode */
	if ( (flags & SPI_XFER_DUAL) && (
	    (opcode == HSSPI_FIFO_OP_CODE_R && (plat->mode & SPI_RX_DUAL)) ||
	    (opcode == HSSPI_FIFO_OP_CODE_W && (plat->mode & SPI_TX_DUAL))) ) {
		opcode |= HSSPI_FIFO_OP_MBIT_MASK;

		/* profile mode */
		if (plat->mode & SPI_RX_DUAL)
			val |= SPI_PFL_MODE_MDRDSZ_MASK;
		if (plat->mode & SPI_TX_DUAL)
			val |= SPI_PFL_MODE_MDWRSZ_MASK;
	}

	if (plat->mode & SPI_3WIRE)
		val |= SPI_PFL_MODE_3WIRE_MASK;
	writel(val, priv->regs + SPI_PFL_MODE_REG(plat->cs));

	/* transfer loop */
	while (data_bytes > 0) {
		size_t curr_step = min(step_size, data_bytes);
		int ret;

		/* copy tx data */
		if (tx) {
			memcpy_toio(priv->regs + HSSPI_FIFO_BASE +
				    HSSPI_FIFO_OP_SIZE, tx, curr_step);
			tx += curr_step;
		}

		/* set fifo operation */
		writew(cpu_to_be16(opcode | (curr_step & HSSPI_FIFO_OP_BYTES_MASK)),
			  priv->regs + HSSPI_FIFO_OP_REG);

		/* make sure we keep cs active until spi transfer is done */
		if (!cs_act) {
			bcmbca_hsspi_activate_cs(priv, plat);
			cs_act = 1;
		}
	
		/* issue the transfer */
		val = SPI_CMD_OP_START;
		val |= (plat->cs << SPI_CMD_PFL_SHIFT) &
			  SPI_CMD_PFL_MASK;
		val |= (plat->cs << SPI_CMD_SLAVE_SHIFT) &
			  SPI_CMD_SLAVE_MASK;
		writel(val, priv->regs + SPI_CMD_REG);

		/* wait for completion */
		ret = wait_for_bit_32(priv->regs + SPI_STAT_REG,
					SPI_STAT_SRCBUSY_MASK, false,
					1000, false);
		if (ret) {
			bcmbca_hsspi_deactivate_cs(priv, plat);
			dev_err(dev, "interrupt timeout\n");
			return ret;
		}

		data_bytes -= curr_step;
		if ((flags & SPI_XFER_END) && !data_bytes)
			bcmbca_hsspi_deactivate_cs(priv, plat);
		
		/* copy rx data */
		if (rx) {
			memcpy_fromio(rx, priv->regs + HSSPI_FIFO_BASE,
				      curr_step);
			rx += curr_step;
		}
	}

	return 0;
}

static const struct dm_spi_ops bcmbca_hsspi_ops = {
	.cs_info = bcmbca_hsspi_cs_info,
	.set_mode = bcmbca_hsspi_set_mode,
	.set_speed = bcmbca_hsspi_set_speed,
	.xfer = bcmbca_hsspi_xfer,
};

static const struct udevice_id bcmbca_hsspi_ids[] = {
	{ .compatible = "brcm,bcmbca-hsspi", },	
	{ /* sentinel */ }
};

static int bcmbca_hsspi_child_pre_probe(struct udevice *dev)
{
	struct bcmbca_hsspi_priv *priv = dev_get_priv(dev->parent);
	struct dm_spi_slave_platdata *plat = dev_get_parent_platdata(dev);

	/* check cs */
	if (plat->cs >= priv->num_cs) {
		dev_err(dev, "no cs %u\n", plat->cs);
		return -ENODEV;
	}

	/* cs polarity */
	if (plat->mode & SPI_CS_HIGH)
		priv->cs_pols |= BIT(plat->cs);
	else
		priv->cs_pols &= ~BIT(plat->cs);

	return 0;
}

static int bcmbca_hsspi_probe(struct udevice *dev)
{
	struct bcmbca_hsspi_priv *priv = dev_get_priv(dev);
	struct reset_ctl rst_ctl;
	struct clk clk;
	int ret;
	u32 freq = 0;

	priv->regs = dev_remap_addr_name(dev, "hsspi");
	if (!priv->regs)
		return -EINVAL;

	priv->spim_ctrl = dev_remap_addr_name(dev, "spim-ctrl");
	if (!priv->spim_ctrl) {
		dev_err(dev, "misc spim ctrl register not defined in dts!\n");
		return -EINVAL;
	}

	priv->num_cs = dev_read_u32_default(dev, "num-cs", 8);

	if (dev_read_u32(dev, "max-uboot-freq", &freq) == 0) {
		priv->max_speed = freq;
		if (priv->max_speed > SPI_MAX_SYNC_CLOCK)
			priv->max_speed = SPI_MAX_SYNC_CLOCK;
		dev_warn(dev,"limit spi bus speed to %dHz\n", priv->max_speed);
	}
	else
		priv->max_speed = 0;

	/* enable clock */
	ret = clk_get_by_name(dev, "hsspi", &clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&clk);
	if (ret < 0 && ret != -ENOSYS)
		return ret;

	ret = clk_free(&clk);
	if (ret < 0 && ret != -ENOSYS)
		return ret;

	/* get clock rate */
	ret = clk_get_by_name(dev, "pll", &clk);
	if (ret < 0 && ret != -ENOSYS)
		return ret;

	priv->clk_rate = clk_get_rate(&clk);

	ret = clk_free(&clk);
	if (ret < 0 && ret != -ENOSYS)
		return ret;

	/* perform reset */
	ret = reset_get_by_index(dev, 0, &rst_ctl);
	if (ret >= 0) {
		ret = reset_deassert(&rst_ctl);
		if (ret < 0)
			return ret;
	}

	ret = reset_free(&rst_ctl);
	if (ret < 0)
		return ret;

	/* initialize hardware */
	writel(0, priv->regs + SPI_IR_MASK_REG);

	/* clear pending interrupts */
	writel(SPI_IR_CLEAR_ALL, priv->regs + SPI_IR_STAT_REG);

	/* enable clk gate */
	setbits_32(priv->regs + SPI_CTL_REG, SPI_CTL_CLK_GATE_MASK);

	/* read default cs polarities */
	priv->cs_pols = readl(priv->regs + SPI_CTL_REG) &
			SPI_CTL_CS_POL_MASK;

	printf("Broadcom BCMBCA HS SPI bus driver\n");
	return 0;
}

U_BOOT_DRIVER(bcmbca_hsspi) = {
	.name = "bcmbca_hsspi",
	.id = UCLASS_SPI,
	.of_match = bcmbca_hsspi_ids,
	.ops = &bcmbca_hsspi_ops,
	.priv_auto_alloc_size = sizeof(struct bcmbca_hsspi_priv),
	.child_pre_probe = bcmbca_hsspi_child_pre_probe,
	.probe = bcmbca_hsspi_probe,
};

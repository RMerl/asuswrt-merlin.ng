// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017 Álvaro Fernández Rojas <noltari@gmail.com>
 *
 * Derived from linux/drivers/spi/spi-bcm63xx-hsspi.c:
 *	Copyright (C) 2000-2010 Broadcom Corporation
 *	Copyright (C) 2012-2013 Jonas Gorski <jogo@openwrt.org>
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <spi.h>
#include <spi-mem.h>
#include <reset.h>
#include <wait_bit.h>
#include <asm/io.h>
#include <linux/bitops.h>

#define HSSPI_PP			0

/*
 * The maximum frequency for SPI synchronous mode is 30MHz for some chips and
 * 25MHz for some others. This depends on the chip layout and SPI signals
 * distance to the pad. We use the lower of these values to cover all relevant
 * chips.
 */
#define SPI_MAX_SYNC_CLOCK		25000000

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
#define SPI_PFL_MODE_MDRDST_SHIFT	8
#define SPI_PFL_MODE_MDWRST_SHIFT	12
#define SPI_PFL_MODE_MDRDSZ_SHIFT	16
#define SPI_PFL_MODE_MDRDSZ_MASK	(1 << SPI_PFL_MODE_MDRDSZ_SHIFT)
#define SPI_PFL_MODE_MDWRSZ_SHIFT	18
#define SPI_PFL_MODE_MDWRSZ_MASK	(1 << SPI_PFL_MODE_MDWRSZ_SHIFT)
#define SPI_PFL_MODE_3WIRE_SHIFT	20
#define SPI_PFL_MODE_3WIRE_MASK		(1 << SPI_PFL_MODE_3WIRE_SHIFT)
#define SPI_PFL_MODE_PREPCNT_SHIFT	24
#define SPI_PFL_MODE_PREPCNT_MASK	(4 << SPI_PFL_MODE_PREPCNT_SHIFT)

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

#define HSSPI_MAX_DATA_SIZE			(HSSPI_FIFO_SIZE - HSSPI_FIFO_OP_SIZE)
#define HSSPI_MAX_PREPEND_SIZE		15

#define HSSPI_XFER_MODE_PREPEND		0
#define HSSPI_XFER_MODE_DUMMYCS		1

struct bcm63xx_hsspi_priv {
	void __iomem *regs;
	ulong clk_rate;
	uint8_t num_cs;
	uint8_t cs_pols;
	uint speed;
	uint xfer_mode;
	uint32_t prepend_cnt;
	uint8_t prepend_buf[HSSPI_MAX_PREPEND_SIZE];
};

static int bcm63xx_hsspi_cs_info(struct udevice *bus, uint cs,
			   struct spi_cs_info *info)
{
	struct bcm63xx_hsspi_priv *priv = dev_get_priv(bus);

	if (cs >= priv->num_cs) {
		printf("no cs %u\n", cs);
		return -EINVAL;
	}

	return 0;
}

static int bcm63xx_hsspi_set_mode(struct udevice *bus, uint mode)
{
	struct bcm63xx_hsspi_priv *priv = dev_get_priv(bus);

	/* clock polarity */
	if (mode & SPI_CPOL)
		setbits_32(priv->regs + SPI_CTL_REG, SPI_CTL_CLK_POL_MASK);
	else
		clrbits_32(priv->regs + SPI_CTL_REG, SPI_CTL_CLK_POL_MASK);

	return 0;
}

static int bcm63xx_hsspi_set_speed(struct udevice *bus, uint speed)
{
	struct bcm63xx_hsspi_priv *priv = dev_get_priv(bus);

	priv->speed = speed;

	return 0;
}

static void bcm63xx_hsspi_activate_cs(struct bcm63xx_hsspi_priv *priv,
				   struct dm_spi_slave_plat *plat)
{
	uint32_t clr, set;
	uint speed = priv->speed;

	if (priv->xfer_mode == HSSPI_XFER_MODE_DUMMYCS &&
	    speed > SPI_MAX_SYNC_CLOCK) {
		speed = SPI_MAX_SYNC_CLOCK;
		debug("Force to dummy cs mode. Reduce the speed to %dHz\n", speed);
	}

	/* profile clock */
	set = DIV_ROUND_UP(priv->clk_rate, speed);
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
	if (speed > SPI_MAX_SYNC_CLOCK)
		set |= SPI_PFL_SIG_ASYNCIN_MASK;

	clrsetbits_32(priv->regs + SPI_PFL_SIG_REG(plat->cs), clr, set);

	/* global control */
	set = 0;
	clr = 0;

	if (priv->xfer_mode == HSSPI_XFER_MODE_PREPEND) {
		if (priv->cs_pols & BIT(plat->cs))
			set |= BIT(plat->cs);
		else
			clr |= BIT(plat->cs);
	} else {
		/* invert cs polarity */
		if (priv->cs_pols & BIT(plat->cs))
			clr |= BIT(plat->cs);
		else
			set |= BIT(plat->cs);

		/* invert dummy cs polarity */
		if (priv->cs_pols & BIT(!plat->cs))
			clr |= BIT(!plat->cs);
		else
			set |= BIT(!plat->cs);
	}

	clrsetbits_32(priv->regs + SPI_CTL_REG, clr, set);
}

static void bcm63xx_hsspi_deactivate_cs(struct bcm63xx_hsspi_priv *priv)
{
	/* restore cs polarities */
	clrsetbits_32(priv->regs + SPI_CTL_REG, SPI_CTL_CS_POL_MASK,
			priv->cs_pols);
}

/*
 * BCM63xx HSSPI driver doesn't allow keeping CS active between transfers
 * because they are controlled by HW.
 * However, it provides a mechanism to prepend write transfers prior to read
 * transfers (with a maximum prepend of 15 bytes), which is usually enough for
 * SPI-connected flashes since reading requires prepending a write transfer of
 * 5 bytes. On the other hand it also provides a way to invert each CS
 * polarity, not only between transfers like the older BCM63xx SPI driver, but
 * also the rest of the time.
 *
 * Instead of using the prepend mechanism, this implementation inverts the
 * polarity of both the desired CS and another dummy CS when the bus is
 * claimed. This way, the dummy CS is restored to its inactive value when
 * transfers are issued and the desired CS is preserved in its active value
 * all the time. This hack is also used in the upstream linux driver and
 * allows keeping CS active between transfers even if the HW doesn't give
 * this possibility.
 *
 * This workaround only works when the dummy CS (usually CS1 when the actual
 * CS is 0) pinmuxed to SPI chip select function if SPI clock is faster than
 * SPI_MAX_SYNC_CLOCK. In old broadcom chip, CS1 pin is default to chip select
 * function. But this is not the case for new chips. To make this function
 * always work, it should be called with maximum clock of SPI_MAX_SYNC_CLOCK.
 */
static int bcm63xx_hsspi_xfer_dummy_cs(struct udevice *dev, unsigned int data_bytes,
				       const void *dout, void *din, unsigned long flags)
{
	struct bcm63xx_hsspi_priv *priv = dev_get_priv(dev->parent);
	struct dm_spi_slave_plat *plat = dev_get_parent_plat(dev);
	size_t step_size = HSSPI_FIFO_SIZE;
	uint16_t opcode = 0;
	uint32_t val = SPI_PFL_MODE_FILL_MASK;
	const uint8_t *tx = dout;
	uint8_t *rx = din;

	if (flags & SPI_XFER_BEGIN)
		bcm63xx_hsspi_activate_cs(priv, plat);

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
	    (opcode == HSSPI_FIFO_OP_CODE_W && (plat->mode & SPI_TX_DUAL)))) {
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
		size_t curr_step = min(step_size, (size_t)data_bytes);
		int ret;

		/* copy tx data */
		if (tx) {
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
			memcpy(priv->regs + HSSPI_FIFO_BASE +
#else
			memcpy_toio(priv->regs + HSSPI_FIFO_BASE +
#endif
				    HSSPI_FIFO_OP_SIZE, tx, curr_step);
			tx += curr_step;
		}

		/* set fifo operation */
		writew(cpu_to_be16(opcode | (curr_step & HSSPI_FIFO_OP_BYTES_MASK)),
			  priv->regs + HSSPI_FIFO_OP_REG);

		/* issue the transfer */
		val = SPI_CMD_OP_START;
		val |= (plat->cs << SPI_CMD_PFL_SHIFT) &
		       SPI_CMD_PFL_MASK;
		val |= (!plat->cs << SPI_CMD_SLAVE_SHIFT) &
		       SPI_CMD_SLAVE_MASK;
		writel(val, priv->regs + SPI_CMD_REG);

		/* wait for completion */
		ret = wait_for_bit_32(priv->regs + SPI_STAT_REG,
					SPI_STAT_SRCBUSY_MASK, false,
					1000, false);
		if (ret) {
			printf("interrupt timeout\n");
			return ret;
		}

		/* copy rx data */
		if (rx) {
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
			memcpy(rx, priv->regs + HSSPI_FIFO_BASE,
#else
			memcpy_fromio(rx, priv->regs + HSSPI_FIFO_BASE,
#endif
				      curr_step);
			rx += curr_step;
		}

		data_bytes -= curr_step;
	}

	if (flags & SPI_XFER_END)
		bcm63xx_hsspi_deactivate_cs(priv);

	return 0;
}

static int bcm63xx_prepare_prepend_transfer(struct bcm63xx_hsspi_priv *priv,
					    unsigned int data_bytes, const void *dout, void *din,
					    unsigned long flags)
{
	/*
	 * only support multiple half duplex write transfer + optional
	 * full duplex read/write at the end.
	 */
	if (flags & SPI_XFER_BEGIN) {
		/* clear prepends */
		priv->prepend_cnt = 0;
	}

	if (din) {
		/* buffering reads not possible for prepend mode */
		if (!(flags & SPI_XFER_END)) {
			debug("unable to buffer reads\n");
			return HSSPI_XFER_MODE_DUMMYCS;
		}

		/* check rx size */
		if (data_bytes > HSSPI_MAX_DATA_SIZE) {
			debug("max rx bytes exceeded\n");
			return HSSPI_XFER_MODE_DUMMYCS;
		}
	}

	if (dout) {
		/* check tx size */
		if (flags & SPI_XFER_END) {
			if (priv->prepend_cnt + data_bytes > HSSPI_MAX_DATA_SIZE) {
				debug("max tx bytes exceeded\n");
				return HSSPI_XFER_MODE_DUMMYCS;
			}
		} else {
			if (priv->prepend_cnt + data_bytes > HSSPI_MAX_PREPEND_SIZE) {
				debug("max prepend bytes exceeded\n");
				return HSSPI_XFER_MODE_DUMMYCS;
			}

			/*
			 * buffer transfer data in the prepend buf in case we have to fall
			 * back to dummy cs mode.
			 */
			memcpy(&priv->prepend_buf[priv->prepend_cnt], dout, data_bytes);
			priv->prepend_cnt += data_bytes;
		}
	}

	return	HSSPI_XFER_MODE_PREPEND;
}

static int bcm63xx_hsspi_xfer_prepend(struct udevice *dev, unsigned int data_bytes,
				      const void *dout, void *din, unsigned long flags)
{
	struct bcm63xx_hsspi_priv *priv = dev_get_priv(dev->parent);
	struct dm_spi_slave_plat *plat = dev_get_parent_plat(dev);
	uint16_t opcode = 0;
	uint32_t val, offset;
	int ret;

	if (flags & SPI_XFER_END) {
		offset = HSSPI_FIFO_BASE + HSSPI_FIFO_OP_SIZE;
		if (priv->prepend_cnt) {
			/* copy prepend data */
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
			memcpy(priv->regs + offset,
#else
			memcpy_toio(priv->regs + offset,
#endif
				    priv->prepend_buf, priv->prepend_cnt);
		}

		if (dout && data_bytes) {
			/* copy tx data */
			offset += priv->prepend_cnt;
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
			memcpy(priv->regs + offset, dout, data_bytes);
#else
			memcpy_toio(priv->regs + offset, dout, data_bytes);
#endif
		}

		bcm63xx_hsspi_activate_cs(priv, plat);
		if (dout && !din) {
			/* all half-duplex write. merge to single write */
			data_bytes += priv->prepend_cnt;
			opcode = HSSPI_FIFO_OP_CODE_W;
			priv->prepend_cnt = 0;
		} else if (!dout && din) {
			/* half-duplex read with prepend write */
			opcode = HSSPI_FIFO_OP_CODE_R;
		} else {
			/* full duplex read/write */
			opcode = HSSPI_FIFO_OP_READ_WRITE;
		}

		/* profile mode */
		val = SPI_PFL_MODE_FILL_MASK;
		if (plat->mode & SPI_3WIRE)
			val |= SPI_PFL_MODE_3WIRE_MASK;

		/* dual mode */
		if ( (flags & SPI_XFER_DUAL) && (
                    (opcode == HSSPI_FIFO_OP_CODE_R && (plat->mode & SPI_RX_DUAL)) ||
		    (opcode == HSSPI_FIFO_OP_CODE_W && (plat->mode & SPI_TX_DUAL)) ) ) {
			opcode |= HSSPI_FIFO_OP_MBIT_MASK;

			if (plat->mode & SPI_RX_DUAL) {
				val |= SPI_PFL_MODE_MDRDSZ_MASK;
				val |= priv->prepend_cnt << SPI_PFL_MODE_MDRDST_SHIFT;
			}
			if (plat->mode & SPI_TX_DUAL) {
				val |= SPI_PFL_MODE_MDWRSZ_MASK;
				val |= priv->prepend_cnt << SPI_PFL_MODE_MDWRST_SHIFT;
			}
		}
		val |= (priv->prepend_cnt << SPI_PFL_MODE_PREPCNT_SHIFT);
		writel(val, priv->regs + SPI_PFL_MODE_REG(plat->cs));

		/* set fifo operation */
		val = opcode | (data_bytes & HSSPI_FIFO_OP_BYTES_MASK);
		writew(cpu_to_be16(val),
		       priv->regs + HSSPI_FIFO_OP_REG);

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
			bcm63xx_hsspi_deactivate_cs(priv);
			printf("spi polling timeout\n");
			return ret;
		}

		/* copy rx data */
		if (din)
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
			memcpy(din, priv->regs + HSSPI_FIFO_BASE,
#else
			memcpy_fromio(din, priv->regs + HSSPI_FIFO_BASE,
#endif
				      data_bytes);
		bcm63xx_hsspi_deactivate_cs(priv);
	}

	return 0;
}

static int bcm63xx_hsspi_xfer(struct udevice *dev, unsigned int bitlen,
			      const void *dout, void *din, unsigned long flags)
{
	struct bcm63xx_hsspi_priv *priv = dev_get_priv(dev->parent);
	int ret;
	u32 data_bytes = bitlen >> 3;

	if (priv->xfer_mode == HSSPI_XFER_MODE_PREPEND) {
		priv->xfer_mode =
			bcm63xx_prepare_prepend_transfer(priv, data_bytes, dout, din, flags);
	}

	/* if not prependable, fall back to dummy cs mode with safe clock */
	if (priv->xfer_mode == HSSPI_XFER_MODE_DUMMYCS) {
		/* For pending prepend data from previous transfers, send it first */
		if (priv->prepend_cnt) {
			bcm63xx_hsspi_xfer_dummy_cs(dev, priv->prepend_cnt,
						    priv->prepend_buf, NULL,
						    (flags & ~SPI_XFER_END) | SPI_XFER_BEGIN);
			priv->prepend_cnt = 0;
		}
		ret = bcm63xx_hsspi_xfer_dummy_cs(dev, data_bytes, dout, din, flags);
	} else {
		ret = bcm63xx_hsspi_xfer_prepend(dev, data_bytes, dout, din, flags);
	}

	if (flags & SPI_XFER_END)
		priv->xfer_mode = HSSPI_XFER_MODE_PREPEND;

	return ret;
}

static bool bcm63xx_hsspi_mem_supports_op(struct spi_slave *slave,
			    const struct spi_mem_op *op)
{
	if (!spi_mem_default_supports_op(slave, op))
		return false;

	// this controller does not support spi mem dual/quad io read cmd in prepend mode
	if ((op->cmd.opcode == 0xbb) || (op->cmd.opcode == 0xeb))
		return false;

	return true;
}

static const struct spi_controller_mem_ops bcm63xx_hsspi_mem_ops = {
	.supports_op = bcm63xx_hsspi_mem_supports_op,
};
static const struct dm_spi_ops bcm63xx_hsspi_ops = {
	.cs_info = bcm63xx_hsspi_cs_info,
	.set_mode = bcm63xx_hsspi_set_mode,
	.set_speed = bcm63xx_hsspi_set_speed,
	.xfer = bcm63xx_hsspi_xfer,
	.mem_ops = &bcm63xx_hsspi_mem_ops,
};

static const struct udevice_id bcm63xx_hsspi_ids[] = {
	{ .compatible = "brcm,bcm6328-hsspi", },
	{ .compatible = "brcm,bcmbca-hsspi-v1.0", },
	{ /* sentinel */ }
};

static int bcm63xx_hsspi_child_pre_probe(struct udevice *dev)
{
	struct bcm63xx_hsspi_priv *priv = dev_get_priv(dev->parent);
	struct dm_spi_slave_plat *plat = dev_get_parent_plat(dev);
	struct spi_slave *slave = dev_get_parent_priv(dev);

	/* check cs */
	if (plat->cs >= priv->num_cs) {
		printf("no cs %u\n", plat->cs);
		return -ENODEV;
	}

	/* cs polarity */
	if (plat->mode & SPI_CS_HIGH)
		priv->cs_pols |= BIT(plat->cs);
	else
		priv->cs_pols &= ~BIT(plat->cs);

	/*
	 * set the max read/write size to make sure each xfer are within the
	 * prepend limit
	 */
	slave->max_read_size = HSSPI_MAX_DATA_SIZE;
	slave->max_write_size = HSSPI_MAX_DATA_SIZE;

	return 0;
}

static int bcm63xx_hsspi_probe(struct udevice *dev)
{
	struct bcm63xx_hsspi_priv *priv = dev_get_priv(dev);
	struct reset_ctl rst_ctl;
	struct clk clk;
	int ret;

	priv->regs = dev_remap_addr(dev);
	if (!priv->regs)
		return -EINVAL;

	priv->num_cs = dev_read_u32_default(dev, "num-cs", 8);

	/* enable clock */
	ret = clk_get_by_name(dev, "hsspi", &clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&clk);
	if (ret < 0 && ret != -ENOSYS)
		return ret;

	/* get clock rate */
	ret = clk_get_by_name(dev, "pll", &clk);
	if (ret < 0 && ret != -ENOSYS)
		return ret;

	priv->clk_rate = clk_get_rate(&clk);

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

	/* default in prepend mode */
	priv->xfer_mode = HSSPI_XFER_MODE_PREPEND;

	return 0;
}

U_BOOT_DRIVER(bcm63xx_hsspi) = {
	.name = "bcm63xx_hsspi",
	.id = UCLASS_SPI,
	.of_match = bcm63xx_hsspi_ids,
	.ops = &bcm63xx_hsspi_ops,
	.priv_auto	= sizeof(struct bcm63xx_hsspi_priv),
	.child_pre_probe = bcm63xx_hsspi_child_pre_probe,
	.probe = bcm63xx_hsspi_probe,
};

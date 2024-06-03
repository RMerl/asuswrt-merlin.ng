// SPDX-License-Identifier: GPL-2.0+
/*
 * Xilinx AXI I2C driver
 *
 * Copyright (C) 2018 Marek Vasut <marex@denx.de>
 *
 * Based on Linux 4.14.y i2c-xiic.c
 * Copyright (c) 2002-2007 Xilinx Inc.
 * Copyright (c) 2009-2010 Intel Corporation
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <i2c.h>
#include <wait_bit.h>
#include <asm/io.h>

struct xilinx_xiic_priv {
	void __iomem		*base;
	struct clk		clk;
};

#define XIIC_MSB_OFFSET 0
#define XIIC_REG_OFFSET (0x100+XIIC_MSB_OFFSET)

/*
 * Register offsets in bytes from RegisterBase. Three is added to the
 * base offset to access LSB (IBM style) of the word
 */
#define XIIC_CR_REG_OFFSET   (0x00+XIIC_REG_OFFSET)	/* Control Register   */
#define XIIC_SR_REG_OFFSET   (0x04+XIIC_REG_OFFSET)	/* Status Register    */
#define XIIC_DTR_REG_OFFSET  (0x08+XIIC_REG_OFFSET)	/* Data Tx Register   */
#define XIIC_DRR_REG_OFFSET  (0x0C+XIIC_REG_OFFSET)	/* Data Rx Register   */
#define XIIC_ADR_REG_OFFSET  (0x10+XIIC_REG_OFFSET)	/* Address Register   */
#define XIIC_TFO_REG_OFFSET  (0x14+XIIC_REG_OFFSET)	/* Tx FIFO Occupancy  */
#define XIIC_RFO_REG_OFFSET  (0x18+XIIC_REG_OFFSET)	/* Rx FIFO Occupancy  */
#define XIIC_TBA_REG_OFFSET  (0x1C+XIIC_REG_OFFSET)	/* 10 Bit Address reg */
#define XIIC_RFD_REG_OFFSET  (0x20+XIIC_REG_OFFSET)	/* Rx FIFO Depth reg  */
#define XIIC_GPO_REG_OFFSET  (0x24+XIIC_REG_OFFSET)	/* Output Register    */

/* Control Register masks */
#define XIIC_CR_ENABLE_DEVICE_MASK        0x01	/* Device enable = 1      */
#define XIIC_CR_TX_FIFO_RESET_MASK        0x02	/* Transmit FIFO reset=1  */
#define XIIC_CR_MSMS_MASK                 0x04	/* Master starts Txing=1  */
#define XIIC_CR_DIR_IS_TX_MASK            0x08	/* Dir of tx. Txing=1     */
#define XIIC_CR_NO_ACK_MASK               0x10	/* Tx Ack. NO ack = 1     */
#define XIIC_CR_REPEATED_START_MASK       0x20	/* Repeated start = 1     */
#define XIIC_CR_GENERAL_CALL_MASK         0x40	/* Gen Call enabled = 1   */

/* Status Register masks */
#define XIIC_SR_GEN_CALL_MASK             0x01	/* 1=a mstr issued a GC   */
#define XIIC_SR_ADDR_AS_SLAVE_MASK        0x02	/* 1=when addr as slave   */
#define XIIC_SR_BUS_BUSY_MASK             0x04	/* 1 = bus is busy        */
#define XIIC_SR_MSTR_RDING_SLAVE_MASK     0x08	/* 1=Dir: mstr <-- slave  */
#define XIIC_SR_TX_FIFO_FULL_MASK         0x10	/* 1 = Tx FIFO full       */
#define XIIC_SR_RX_FIFO_FULL_MASK         0x20	/* 1 = Rx FIFO full       */
#define XIIC_SR_RX_FIFO_EMPTY_MASK        0x40	/* 1 = Rx FIFO empty      */
#define XIIC_SR_TX_FIFO_EMPTY_MASK        0x80	/* 1 = Tx FIFO empty      */

/* Interrupt Status Register masks    Interrupt occurs when...       */
#define XIIC_INTR_ARB_LOST_MASK           0x01	/* 1 = arbitration lost   */
#define XIIC_INTR_TX_ERROR_MASK           0x02	/* 1=Tx error/msg complete */
#define XIIC_INTR_TX_EMPTY_MASK           0x04	/* 1 = Tx FIFO/reg empty  */
#define XIIC_INTR_RX_FULL_MASK            0x08	/* 1=Rx FIFO/reg=OCY level */
#define XIIC_INTR_BNB_MASK                0x10	/* 1 = Bus not busy       */
#define XIIC_INTR_AAS_MASK                0x20	/* 1 = when addr as slave */
#define XIIC_INTR_NAAS_MASK               0x40	/* 1 = not addr as slave  */
#define XIIC_INTR_TX_HALF_MASK            0x80	/* 1 = TX FIFO half empty */

/* The following constants specify the depth of the FIFOs */
#define IIC_RX_FIFO_DEPTH         16	/* Rx fifo capacity               */
#define IIC_TX_FIFO_DEPTH         16	/* Tx fifo capacity               */

/*
 * Tx Fifo upper bit masks.
 */
#define XIIC_TX_DYN_START_MASK            0x0100 /* 1 = Set dynamic start */
#define XIIC_TX_DYN_STOP_MASK             0x0200 /* 1 = Set dynamic stop */

/*
 * The following constants define the register offsets for the Interrupt
 * registers. There are some holes in the memory map for reserved addresses
 * to allow other registers to be added and still match the memory map of the
 * interrupt controller registers
 */
#define XIIC_DGIER_OFFSET    0x1C /* Device Global Interrupt Enable Register */
#define XIIC_IISR_OFFSET     0x20 /* Interrupt Status Register */
#define XIIC_IIER_OFFSET     0x28 /* Interrupt Enable Register */
#define XIIC_RESETR_OFFSET   0x40 /* Reset Register */

#define XIIC_RESET_MASK             0xAUL

static u8 i2c_8bit_addr_from_flags(uint addr, u16 flags)
{
	return (addr << 1) | (flags & I2C_M_RD ? 1 : 0);
}

static void xiic_irq_clr(struct xilinx_xiic_priv *priv, u32 mask)
{
	u32 isr = readl(priv->base + XIIC_IISR_OFFSET);

	writel(isr & mask, priv->base + XIIC_IISR_OFFSET);
}

static int xiic_read_rx(struct xilinx_xiic_priv *priv,
			struct i2c_msg *msg, int nmsgs)
{
	u8 bytes_in_fifo;
	u32 pos = 0;
	int i, ret;

	while (pos < msg->len) {
		ret = wait_for_bit_8(priv->base + XIIC_SR_REG_OFFSET,
				     XIIC_SR_RX_FIFO_EMPTY_MASK, false,
				     1000, true);
		if (ret)
			return ret;

		bytes_in_fifo = readb(priv->base + XIIC_RFO_REG_OFFSET) + 1;

		if (bytes_in_fifo > msg->len)
			bytes_in_fifo = msg->len;

		for (i = 0; i < bytes_in_fifo; i++) {
			msg->buf[pos++] = readb(priv->base +
						XIIC_DRR_REG_OFFSET);
		}
	}

	return 0;
}

static int xiic_tx_fifo_space(struct xilinx_xiic_priv *priv)
{
	/* return the actual space left in the FIFO */
	return IIC_TX_FIFO_DEPTH - readb(priv->base + XIIC_TFO_REG_OFFSET) - 1;
}

static void xiic_fill_tx_fifo(struct xilinx_xiic_priv *priv,
			      struct i2c_msg *msg, int nmsgs)
{
	u8 fifo_space = xiic_tx_fifo_space(priv);
	int len = msg->len;
	u32 pos = 0;

	len = (len > fifo_space) ? fifo_space : len;

	while (len--) {
		u16 data = msg->buf[pos++];

		if (pos == len && nmsgs == 1) {
			/* last message in transfer -> STOP */
			data |= XIIC_TX_DYN_STOP_MASK;
		}
		writew(data, priv->base + XIIC_DTR_REG_OFFSET);
	}
}

static void xilinx_xiic_set_addr(struct udevice *dev, u8 addr,
				 u16 flags, u32 len, u32 nmsgs)
{
	struct xilinx_xiic_priv *priv = dev_get_priv(dev);

	xiic_irq_clr(priv, XIIC_INTR_TX_ERROR_MASK);

	if (!(flags & I2C_M_NOSTART)) {
		/* write the address */
		u16 data = i2c_8bit_addr_from_flags(addr, flags) |
			XIIC_TX_DYN_START_MASK;
		if (nmsgs == 1 && len == 0)
			/* no data and last message -> add STOP */
			data |= XIIC_TX_DYN_STOP_MASK;

		writew(data, priv->base + XIIC_DTR_REG_OFFSET);
	}
}

static int xilinx_xiic_read_common(struct udevice *dev, struct i2c_msg *msg,
				   u32 nmsgs)
{
	struct xilinx_xiic_priv *priv = dev_get_priv(dev);
	u8 rx_watermark;

	/* Clear and enable Rx full interrupt. */
	xiic_irq_clr(priv, XIIC_INTR_RX_FULL_MASK | XIIC_INTR_TX_ERROR_MASK);

	/* we want to get all but last byte, because the TX_ERROR IRQ is used
	 * to inidicate error ACK on the address, and negative ack on the last
	 * received byte, so to not mix them receive all but last.
	 * In the case where there is only one byte to receive
	 * we can check if ERROR and RX full is set at the same time
	 */
	rx_watermark = msg->len;
	if (rx_watermark > IIC_RX_FIFO_DEPTH)
		rx_watermark = IIC_RX_FIFO_DEPTH;

	writeb(rx_watermark - 1, priv->base + XIIC_RFD_REG_OFFSET);

	xilinx_xiic_set_addr(dev, msg->addr, msg->flags, msg->len, nmsgs);

	xiic_irq_clr(priv, XIIC_INTR_BNB_MASK);

	writew((msg->len & 0xff) | ((nmsgs == 1) ? XIIC_TX_DYN_STOP_MASK : 0),
	       priv->base + XIIC_DTR_REG_OFFSET);

	if (nmsgs == 1)
		/* very last, enable bus not busy as well */
		xiic_irq_clr(priv, XIIC_INTR_BNB_MASK);

	return xiic_read_rx(priv, msg, nmsgs);
}

static int xilinx_xiic_write_common(struct udevice *dev, struct i2c_msg *msg,
				    int nmsgs)
{
	struct xilinx_xiic_priv *priv = dev_get_priv(dev);
	int ret;

	xilinx_xiic_set_addr(dev, msg->addr, msg->flags, msg->len, nmsgs);
	xiic_fill_tx_fifo(priv, msg, nmsgs);

	ret = wait_for_bit_8(priv->base + XIIC_SR_REG_OFFSET,
			     XIIC_SR_TX_FIFO_EMPTY_MASK, false, 1000, true);
	if (ret)
		return ret;

	/* Clear any pending Tx empty, Tx Error and then enable them. */
	xiic_irq_clr(priv, XIIC_INTR_TX_EMPTY_MASK | XIIC_INTR_TX_ERROR_MASK |
			   XIIC_INTR_BNB_MASK);

	return 0;
}

static void xiic_clear_rx_fifo(struct xilinx_xiic_priv *priv)
{
	u8 sr;

	for (sr = readb(priv->base + XIIC_SR_REG_OFFSET);
		!(sr & XIIC_SR_RX_FIFO_EMPTY_MASK);
		sr = readb(priv->base + XIIC_SR_REG_OFFSET))
		readb(priv->base + XIIC_DRR_REG_OFFSET);
}

static void xiic_reinit(struct xilinx_xiic_priv *priv)
{
	writel(XIIC_RESET_MASK, priv->base + XIIC_RESETR_OFFSET);

	/* Set receive Fifo depth to maximum (zero based). */
	writeb(IIC_RX_FIFO_DEPTH - 1, priv->base + XIIC_RFD_REG_OFFSET);

	/* Reset Tx Fifo. */
	writeb(XIIC_CR_TX_FIFO_RESET_MASK, priv->base + XIIC_CR_REG_OFFSET);

	/* Enable IIC Device, remove Tx Fifo reset & disable general call. */
	writeb(XIIC_CR_ENABLE_DEVICE_MASK, priv->base + XIIC_CR_REG_OFFSET);

	/* make sure RX fifo is empty */
	xiic_clear_rx_fifo(priv);

	/* Disable interrupts */
	writel(0, priv->base + XIIC_DGIER_OFFSET);

	xiic_irq_clr(priv, XIIC_INTR_ARB_LOST_MASK);
}

static int xilinx_xiic_xfer(struct udevice *dev, struct i2c_msg *msg, int nmsgs)
{
	int ret = 0;

	for (; nmsgs > 0; nmsgs--, msg++) {
		if (msg->flags & I2C_M_RD)
			ret = xilinx_xiic_read_common(dev, msg, nmsgs);
		else
			ret = xilinx_xiic_write_common(dev, msg, nmsgs);

		if (ret)
			return -EREMOTEIO;
	}

	return ret;
}

static int xilinx_xiic_probe_chip(struct udevice *dev, uint addr, uint flags)
{
	struct xilinx_xiic_priv *priv = dev_get_priv(dev);
	u32 reg;
	int ret;

	xiic_reinit(priv);

	xilinx_xiic_set_addr(dev, addr, 0, 0, 1);
	ret = wait_for_bit_8(priv->base + XIIC_SR_REG_OFFSET,
			     XIIC_SR_BUS_BUSY_MASK, false, 1000, true);
	if (ret)
		return ret;

	reg = readl(priv->base + XIIC_IISR_OFFSET);
	if (reg & XIIC_INTR_TX_ERROR_MASK)
		return -ENODEV;

	return 0;
}

static int xilinx_xiic_set_speed(struct udevice *dev, uint speed)
{
	return 0;
}

static int xilinx_xiic_probe(struct udevice *dev)
{
	struct xilinx_xiic_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr_ptr(dev);

	writel(XIIC_CR_TX_FIFO_RESET_MASK, priv->base + XIIC_CR_REG_OFFSET);
	xiic_reinit(priv);

	return 0;
}

static const struct dm_i2c_ops xilinx_xiic_ops = {
	.xfer		= xilinx_xiic_xfer,
	.probe_chip	= xilinx_xiic_probe_chip,
	.set_bus_speed	= xilinx_xiic_set_speed,
};

static const struct udevice_id xilinx_xiic_ids[] = {
	{ .compatible = "xlnx,xps-iic-2.00.a" },
	{ }
};

U_BOOT_DRIVER(xilinx_xiic) = {
	.name		= "xilinx_axi_i2c",
	.id		= UCLASS_I2C,
	.of_match	= xilinx_xiic_ids,
	.probe		= xilinx_xiic_probe,
	.priv_auto_alloc_size = sizeof(struct xilinx_xiic_priv),
	.ops		= &xilinx_xiic_ops,
};

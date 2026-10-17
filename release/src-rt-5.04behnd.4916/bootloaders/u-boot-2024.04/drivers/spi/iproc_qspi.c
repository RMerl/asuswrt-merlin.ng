// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020-2021 Broadcom
 */

#include <common.h>
#include <dm.h>
#include <spi.h>
#include <spi-mem.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/iopoll.h>
#include <linux/log2.h>

/* Delay required to change the mode of operation */
#define BUSY_DELAY_US				1
#define BUSY_TIMEOUT_US				200000
#define DWORD_ALIGNED(a)			(!(((ulong)(a)) & 3))

/* Chip attributes */
#define QSPI_AXI_CLK				175000000
#define SPBR_MIN				8U
#define SPBR_MAX				255U
#define NUM_CDRAM				16U

#define CDRAM_PCS0				2
#define CDRAM_CONT				BIT(7)
#define CDRAM_BITS_EN				BIT(6)
#define CDRAM_QUAD_MODE				BIT(8)
#define CDRAM_RBIT_INPUT			BIT(10)
#define MSPI_SPE				BIT(6)
#define MSPI_CONT_AFTER_CMD			BIT(7)
#define MSPI_MSTR				BIT(7)

/* Register fields */
#define MSPI_SPCR0_MSB_BITS_8			0x00000020
#define BSPI_RAF_CONTROL_START_MASK		0x00000001
#define BSPI_RAF_STATUS_SESSION_BUSY_MASK	0x00000001
#define BSPI_RAF_STATUS_FIFO_EMPTY_MASK		0x00000002
#define BSPI_STRAP_OVERRIDE_DATA_QUAD_SHIFT	3
#define BSPI_STRAP_OVERRIDE_4BYTE_SHIFT	2
#define BSPI_STRAP_OVERRIDE_DATA_DUAL_SHIFT	1
#define BSPI_STRAP_OVERRIDE_SHIFT		0
#define BSPI_BPC_DATA_SHIFT			0
#define BSPI_BPC_MODE_SHIFT			8
#define BSPI_BPC_ADDR_SHIFT			16
#define BSPI_BPC_CMD_SHIFT			24
#define BSPI_BPP_ADDR_SHIFT			16

/* MSPI registers */
#define MSPI_SPCR0_LSB_REG			0x000
#define MSPI_SPCR0_MSB_REG			0x004
#define MSPI_SPCR1_LSB_REG			0x008
#define MSPI_SPCR1_MSB_REG			0x00c
#define MSPI_NEWQP_REG				0x010
#define MSPI_ENDQP_REG				0x014
#define MSPI_SPCR2_REG				0x018
#define MSPI_STATUS_REG				0x020
#define MSPI_CPTQP_REG				0x024
#define MSPI_TX_REG				0x040
#define MSPI_RX_REG				0x0c0
#define MSPI_CDRAM_REG				0x140
#define MSPI_WRITE_LOCK_REG			0x180
#define MSPI_DISABLE_FLUSH_GEN_REG		0x184

/* BSPI registers */
#define BSPI_REVISION_ID_REG			0x000
#define BSPI_SCRATCH_REG			0x004
#define BSPI_MAST_N_BOOT_CTRL_REG		0x008
#define BSPI_BUSY_STATUS_REG			0x00c
#define BSPI_INTR_STATUS_REG			0x010
#define BSPI_B0_STATUS_REG			0x014
#define BSPI_B0_CTRL_REG			0x018
#define BSPI_B1_STATUS_REG			0x01c
#define BSPI_B1_CTRL_REG			0x020
#define BSPI_STRAP_OVERRIDE_CTRL_REG		0x024
#define BSPI_FLEX_MODE_ENABLE_REG		0x028
#define BSPI_BITS_PER_CYCLE_REG			0x02C
#define BSPI_BITS_PER_PHASE_REG			0x030
#define BSPI_CMD_AND_MODE_BYTE_REG		0x034
#define BSPI_FLASH_UPPER_ADDR_BYTE_REG		0x038
#define BSPI_XOR_VALUE_REG			0x03C
#define BSPI_XOR_ENABLE_REG			0x040
#define BSPI_PIO_MODE_ENABLE_REG		0x044
#define BSPI_PIO_IODIR_REG			0x048
#define BSPI_PIO_DATA_REG			0x04C

/* RAF registers */
#define BSPI_RAF_START_ADDRESS_REG		0x00
#define BSPI_RAF_NUM_WORDS_REG			0x04
#define BSPI_RAF_CTRL_REG			0x08
#define BSPI_RAF_FULLNESS_REG			0x0C
#define BSPI_RAF_WATERMARK_REG			0x10
#define BSPI_RAF_STATUS_REG			0x14
#define BSPI_RAF_READ_DATA_REG			0x18
#define BSPI_RAF_WORD_CNT_REG			0x1C
#define BSPI_RAF_CURR_ADDR_REG			0x20

#define XFER_DUAL				BIT(30)
#define XFER_QUAD				BIT(31)

#define FLUSH_BIT				BIT(0)
#define MAST_N_BOOT_BIT				BIT(0)
#define WRITE_LOCK_BIT				BIT(0)

#define CEIL(m, n)				(((m) + (n) - 1) / (n))
#define UPPER_BYTE_MASK				0xFF000000
#define SIZE_16MB				0x001000000

/*
 * struct bcmspi_priv - qspi private structure
 *
 * @bspi_addr: bspi read address
 * @bspi_4byte_addr: bspi 4 byte address mode
 * @mspi: mspi registers block address
 * @bspi: bspi registers block address
 * @bspi_raf: bspi raf registers block address
 */
struct bcmspi_priv {
	u32 bspi_addr;
	bool bspi_4byte_addr;
	fdt_addr_t mspi;
	fdt_addr_t bspi;
	fdt_addr_t bspi_raf;
};

/* BSPI mode */

static void bspi_flush_prefetch_buffers(struct bcmspi_priv *priv)
{
	writel(0, priv->bspi + BSPI_B0_CTRL_REG);
	writel(0, priv->bspi + BSPI_B1_CTRL_REG);
	writel(FLUSH_BIT, priv->bspi + BSPI_B0_CTRL_REG);
	writel(FLUSH_BIT, priv->bspi + BSPI_B1_CTRL_REG);
}

static int bspi_enable(struct bcmspi_priv *priv)
{
	/* Disable write lock */
	writel(0, priv->mspi + MSPI_WRITE_LOCK_REG);
	/* Flush prefetch buffers */
	bspi_flush_prefetch_buffers(priv);
	/* Switch to BSPI */
	writel(0, priv->bspi + BSPI_MAST_N_BOOT_CTRL_REG);

	return 0;
}

static int bspi_disable(struct bcmspi_priv *priv)
{
	int ret;
	uint val;

	if ((readl(priv->bspi + BSPI_MAST_N_BOOT_CTRL_REG) & 1) == 0) {
		ret = readl_poll_timeout(priv->bspi + BSPI_BUSY_STATUS_REG, val, !(val & 1),
					 BUSY_TIMEOUT_US);
		if (ret) {
			printf("%s: Failed to disable bspi, device busy\n", __func__);
			return ret;
		}

		/* Switch to MSPI */
		writel(MAST_N_BOOT_BIT, priv->bspi + BSPI_MAST_N_BOOT_CTRL_REG);
		udelay(BUSY_DELAY_US);

		val = readl(priv->bspi + BSPI_MAST_N_BOOT_CTRL_REG);
		if (!(val & 1)) {
			printf("%s: Failed to enable mspi\n", __func__);
			return -EBUSY;
		}
	}

	/* Enable write lock */
	writel(WRITE_LOCK_BIT, priv->mspi + MSPI_WRITE_LOCK_REG);

	return 0;
}

static int bspi_read_via_raf(struct bcmspi_priv *priv, u8 *rx, uint bytes)
{
	u32 status;
	uint words;
	int aligned;
	int ret;

	/*
	 * Flush data from the previous session (unlikely)
	 * Read outstanding bits in the poll condition to empty FIFO
	 */
	ret = readl_poll_timeout(priv->bspi_raf + BSPI_RAF_STATUS_REG,
				 status,
				 (!readl(priv->bspi_raf + BSPI_RAF_READ_DATA_REG) &&
				  status & BSPI_RAF_STATUS_FIFO_EMPTY_MASK) &&
				  !(status & BSPI_RAF_STATUS_SESSION_BUSY_MASK),
				  BUSY_TIMEOUT_US);
	if (ret) {
		printf("%s: Failed to flush fifo\n", __func__);
		return ret;
	}

	/* Transfer is in words */
	words = CEIL(bytes, 4);

	/* Setup hardware */
	if (priv->bspi_4byte_addr) {
		u32 val = priv->bspi_addr & UPPER_BYTE_MASK;

		if (val != readl(priv->bspi + BSPI_FLASH_UPPER_ADDR_BYTE_REG)) {
			writel(val, priv->bspi + BSPI_FLASH_UPPER_ADDR_BYTE_REG);
			bspi_flush_prefetch_buffers(priv);
		}
	}

	writel(priv->bspi_addr & ~UPPER_BYTE_MASK, priv->bspi_raf + BSPI_RAF_START_ADDRESS_REG);
	writel(words, priv->bspi_raf + BSPI_RAF_NUM_WORDS_REG);
	writel(0, priv->bspi_raf + BSPI_RAF_WATERMARK_REG);

	/* Start reading */
	writel(BSPI_RAF_CONTROL_START_MASK, priv->bspi_raf + BSPI_RAF_CTRL_REG);
	aligned = DWORD_ALIGNED(rx);
	while (bytes) {
		status = readl(priv->bspi_raf + BSPI_RAF_STATUS_REG);
		if (!(status & BSPI_RAF_STATUS_FIFO_EMPTY_MASK)) {
			/* RAF is LE only, convert data to host endianness */
			u32 data = le32_to_cpu(readl(priv->bspi_raf + BSPI_RAF_READ_DATA_REG));

			/* Check if we can use the whole word */
			if (aligned && bytes >= 4) {
				*(u32 *)rx = data;
				rx += 4;
				bytes -= 4;
			} else {
				uint chunk = min(bytes, 4U);

				/* Read out bytes one by one */
				while (chunk) {
					*rx++ = (u8)data;
					data >>= 8;
					chunk--;
					bytes--;
				}
			}

			continue;
		}
		if (!(status & BSPI_RAF_STATUS_SESSION_BUSY_MASK)) {
			/* FIFO is empty and the session is done */
			break;
		}
	}

	return 0;
}

static int bspi_read(struct bcmspi_priv *priv, u8 *rx, uint bytes)
{
	int ret;

	/* Transfer data */
	while (bytes > 0) {
		/* Special handing since RAF cannot go across 16MB boundary */
		uint trans = bytes;
		/* Divide into multiple transfers if it goes across the 16MB boundary */
		if (priv->bspi_4byte_addr && (priv->bspi_addr >> 24) !=
		    ((priv->bspi_addr + bytes) >> 24))
			trans = SIZE_16MB - (priv->bspi_addr & ~UPPER_BYTE_MASK);

		ret = bspi_read_via_raf(priv, rx, trans);
		if (ret)
			return ret;

		priv->bspi_addr += trans;
		rx += trans;
		bytes -= trans;
	}

	bspi_flush_prefetch_buffers(priv);
	return 0;
}

static void bspi_set_flex_mode(struct bcmspi_priv *priv, const struct spi_mem_op *op)
{
	int bpp = (op->dummy.nbytes * 8) / op->dummy.buswidth;
	int cmd = op->cmd.opcode;
	int bpc = ilog2(op->data.buswidth) << BSPI_BPC_DATA_SHIFT |
			  ilog2(op->addr.buswidth) << BSPI_BPC_ADDR_SHIFT |
			  ilog2(op->cmd.buswidth) << BSPI_BPC_CMD_SHIFT;
	int so =  BIT(BSPI_STRAP_OVERRIDE_SHIFT) |
			  (op->data.buswidth > 1) << BSPI_STRAP_OVERRIDE_DATA_DUAL_SHIFT |
			  (op->addr.nbytes > 3) << BSPI_STRAP_OVERRIDE_4BYTE_SHIFT |
			  (op->data.buswidth > 3) << BSPI_STRAP_OVERRIDE_DATA_QUAD_SHIFT;

	/* Disable flex mode first */
	writel(0, priv->bspi + BSPI_FLEX_MODE_ENABLE_REG);

	/* Configure single, dual or quad mode */
	writel(bpc, priv->bspi + BSPI_BITS_PER_CYCLE_REG);

	/* Opcode */
	writel(cmd, priv->bspi + BSPI_CMD_AND_MODE_BYTE_REG);

	/* Count of dummy cycles */
	writel(bpp, priv->bspi + BSPI_BITS_PER_PHASE_REG);

	/* Enable 4-byte address */
	if (priv->bspi_4byte_addr) {
		setbits_le32(priv->bspi + BSPI_BITS_PER_PHASE_REG, BIT(BSPI_BPP_ADDR_SHIFT));
	} else {
		clrbits_le32(priv->bspi + BSPI_BITS_PER_PHASE_REG, BIT(BSPI_BPP_ADDR_SHIFT));
		writel(0, priv->bspi + BSPI_FLASH_UPPER_ADDR_BYTE_REG);
	}

	/* Enable flex mode to take effect */
	writel(1, priv->bspi + BSPI_FLEX_MODE_ENABLE_REG);

	/* Flush prefetch buffers since 32MB window BSPI could be used */
	bspi_flush_prefetch_buffers(priv);

	/* Override the strap settings */
	writel(so, priv->bspi + BSPI_STRAP_OVERRIDE_CTRL_REG);
}

static int bspi_exec_op(struct spi_slave *slave, const struct spi_mem_op *op)
{
	struct udevice *bus = dev_get_parent(slave->dev);
	struct bcmspi_priv *priv = dev_get_priv(bus);
	int ret = -ENOTSUPP;

	/* BSPI read */
	if (op->data.dir == SPI_MEM_DATA_IN &&
	    op->data.nbytes && op->addr.nbytes) {
		priv->bspi_4byte_addr = (op->addr.nbytes > 3);
		priv->bspi_addr = op->addr.val;
		bspi_set_flex_mode(priv, op);
		ret = bspi_read(priv, op->data.buf.in, op->data.nbytes);
	}

	return ret;
}

static const struct spi_controller_mem_ops bspi_mem_ops = {
	.exec_op = bspi_exec_op,
};

/* MSPI mode */

static int mspi_exec(struct bcmspi_priv *priv, uint bytes, const u8 *tx, u8 *rx, ulong flags)
{
	u32 cdr = CDRAM_PCS0 | CDRAM_CONT;
	bool use_16bits = !(bytes & 1);

	if (flags & XFER_QUAD) {
		cdr |= CDRAM_QUAD_MODE;

		if (!tx)
			cdr |= CDRAM_RBIT_INPUT;
	}

	while (bytes) {
		uint chunk;
		uint queues;
		uint i;
		uint val;
		int ret;

		if (use_16bits) {
			chunk = min(bytes, NUM_CDRAM * 2);
			queues = (chunk + 1) / 2;
			bytes -= chunk;

			/* Fill CDRAMs */
			for (i = 0; i < queues; i++)
				writel(cdr | CDRAM_BITS_EN, priv->mspi + MSPI_CDRAM_REG + 4 * i);

			/* Fill TXRAMs */
			for (i = 0; i < chunk; i++)
				writel(tx ? tx[i] : 0xff, priv->mspi + MSPI_TX_REG + 4 * i);
		} else {
			/* Determine how many bytes to process this time */
			chunk = min(bytes, NUM_CDRAM);
			queues = chunk;
			bytes -= chunk;

			/* Fill CDRAMs and TXRAMS */
			for (i = 0; i < chunk; i++) {
				writel(cdr, priv->mspi + MSPI_CDRAM_REG + 4 * i);
				writel(tx ? tx[i] : 0xff, priv->mspi + MSPI_TX_REG + 8 * i);
			}
		}

		/* Setup queue pointers */
		writel(0, priv->mspi + MSPI_NEWQP_REG);
		writel(queues - 1, priv->mspi + MSPI_ENDQP_REG);

		/* Deassert CS if requested and it's the last transfer */
		if (bytes == 0 && (flags & SPI_XFER_END))
			clrbits_le32(priv->mspi + MSPI_CDRAM_REG + ((queues - 1) << 2), CDRAM_CONT);

		/* Kick off */
		writel(0, priv->mspi + MSPI_STATUS_REG);
		if (bytes == 0 && (flags & SPI_XFER_END))
			writel(MSPI_SPE, priv->mspi + MSPI_SPCR2_REG);
		else
			writel(MSPI_SPE | MSPI_CONT_AFTER_CMD,
			       priv->mspi + MSPI_SPCR2_REG);

		ret = readl_poll_timeout(priv->mspi + MSPI_STATUS_REG, val, (val & 1),
					 BUSY_TIMEOUT_US);
		if (ret) {
			printf("%s: Failed to disable bspi, device busy\n", __func__);
			return ret;
		}

		/* Read data out */
		if (rx) {
			if (use_16bits) {
				for (i = 0; i < chunk; i++)
					rx[i] = readl(priv->mspi + MSPI_RX_REG + 4 * i) & 0xff;
			} else {
				for (i = 0; i < chunk; i++)
					rx[i] = readl(priv->mspi + MSPI_RX_REG + 8 * i + 4) & 0xff;
			}
		}

		/* Advance pointers */
		if (tx)
			tx += chunk;
		if (rx)
			rx += chunk;
	}

	return 0;
}

static int mspi_xfer(struct udevice *dev, uint bitlen, const void *dout, void *din, ulong flags)
{
	struct udevice *bus = dev_get_parent(dev);
	struct bcmspi_priv *priv = dev_get_priv(bus);
	uint bytes;
	int ret = 0;

	/* we can only transfer multiples of 8 bits */
	if (bitlen % 8)
		return -EPROTONOSUPPORT;

	bytes = bitlen / 8;

	if (flags & SPI_XFER_BEGIN) {
		/* Switch to MSPI */
		ret = bspi_disable(priv);
		if (ret)
			return ret;
	}

	/* MSPI: Transfer */
	if (bytes)
		ret = mspi_exec(priv, bytes, dout, din, flags);

	if (flags & SPI_XFER_END) {
		/* Switch back to BSPI */
		ret = bspi_enable(priv);
		if (ret)
			return ret;
	}

	return ret;
}

/* iProc interface */

static int iproc_qspi_set_speed(struct udevice *bus, uint speed)
{
	struct bcmspi_priv *priv = dev_get_priv(bus);
	uint spbr;

	/* MSPI: SCK configuration */
	spbr = (QSPI_AXI_CLK - 1) / (2 * speed) + 1;
	writel(max(min(spbr, SPBR_MAX), SPBR_MIN), priv->mspi + MSPI_SPCR0_LSB_REG);

	return 0;
}

static int iproc_qspi_set_mode(struct udevice *bus, uint mode)
{
	struct bcmspi_priv *priv = dev_get_priv(bus);

	/* MSPI: set master bit and mode */
	writel(MSPI_MSTR /* Master */ | (mode & 3), priv->mspi + MSPI_SPCR0_MSB_REG);

	return 0;
}

static int iproc_qspi_claim_bus(struct udevice *dev)
{
	/* Nothing to do */
	return 0;
}

static int iproc_qspi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct bcmspi_priv *priv = dev_get_priv(bus);

	/* Make sure no operation is in progress */
	writel(0, priv->mspi + MSPI_SPCR2_REG);
	udelay(BUSY_DELAY_US);

	return 0;
}

static int iproc_qspi_of_to_plat(struct udevice *bus)
{
	struct bcmspi_priv *priv = dev_get_priv(bus);

	priv->bspi = dev_read_addr_name(bus, "bspi");
	if (IS_ERR((void *)priv->bspi)) {
		printf("%s: Failed to get bspi base address\n", __func__);
		return PTR_ERR((void *)priv->bspi);
	}

	priv->bspi_raf = dev_read_addr_name(bus, "bspi_raf");
	if (IS_ERR((void *)priv->bspi_raf)) {
		printf("%s: Failed to get bspi_raf base address\n", __func__);
		return PTR_ERR((void *)priv->bspi_raf);
	}

	priv->mspi = dev_read_addr_name(bus, "mspi");
	if (IS_ERR((void *)priv->mspi)) {
		printf("%s: Failed to get mspi base address\n", __func__);
		return PTR_ERR((void *)priv->mspi);
	}

	return 0;
}

static int iproc_qspi_probe(struct udevice *bus)
{
	struct bcmspi_priv *priv = dev_get_priv(bus);

	/* configure mspi */
	writel(0, priv->mspi + MSPI_SPCR1_LSB_REG);
	writel(0, priv->mspi + MSPI_SPCR1_MSB_REG);
	writel(0, priv->mspi + MSPI_NEWQP_REG);
	writel(0, priv->mspi + MSPI_ENDQP_REG);
	writel(0, priv->mspi + MSPI_SPCR2_REG);

	/* configure bspi */
	bspi_enable(priv);

	return 0;
}

static const struct dm_spi_ops iproc_qspi_ops = {
	.claim_bus	= iproc_qspi_claim_bus,
	.release_bus	= iproc_qspi_release_bus,
	.xfer		= mspi_xfer,
	.set_speed	= iproc_qspi_set_speed,
	.set_mode	= iproc_qspi_set_mode,
	.mem_ops	= &bspi_mem_ops,
};

static const struct udevice_id iproc_qspi_ids[] = {
	{ .compatible = "brcm,iproc-qspi" },
	{ }
};

U_BOOT_DRIVER(iproc_qspi) = {
	.name	= "iproc_qspi",
	.id	= UCLASS_SPI,
	.of_match = iproc_qspi_ids,
	.ops	= &iproc_qspi_ops,
	.of_to_plat = iproc_qspi_of_to_plat,
	.priv_auto = sizeof(struct bcmspi_priv),
	.probe	= iproc_qspi_probe,
};

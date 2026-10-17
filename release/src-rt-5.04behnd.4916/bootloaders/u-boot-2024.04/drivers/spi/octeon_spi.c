// SPDX-License-Identifier:    GPL-2.0
/*
 * Copyright (C) 2018 Marvell International Ltd.
 */

#include <clk.h>
#include <dm.h>
#include <malloc.h>
#include <spi.h>
#include <spi-mem.h>
#include <watchdog.h>
#include <asm/io.h>
#include <asm/unaligned.h>
#include <linux/bitfield.h>
#include <linux/compat.h>
#include <linux/delay.h>

#define OCTEON_SPI_MAX_BYTES	9
#define OCTEON_SPI_MAX_CLOCK_HZ	50000000

#define OCTEON_SPI_NUM_CS	4

#define OCTEON_SPI_CS_VALID(cs)	((cs) < OCTEON_SPI_NUM_CS)

#define MPI_CFG			0x0000
#define MPI_STS			0x0008
#define MPI_TX			0x0010
#define MPI_XMIT		0x0018
#define MPI_WIDE_DAT		0x0040
#define MPI_IO_CTL		0x0048
#define MPI_DAT(X)		(0x0080 + ((X) << 3))
#define MPI_WIDE_BUF(X)		(0x0800 + ((X) << 3))
#define MPI_CYA_CFG		0x1000
#define MPI_CLKEN		0x1080

#define MPI_CFG_ENABLE		BIT_ULL(0)
#define MPI_CFG_IDLELO		BIT_ULL(1)
#define MPI_CFG_CLK_CONT	BIT_ULL(2)
#define MPI_CFG_WIREOR		BIT_ULL(3)
#define MPI_CFG_LSBFIRST	BIT_ULL(4)
#define MPI_CFG_CS_STICKY	BIT_ULL(5)
#define MPI_CFG_CSHI		BIT_ULL(7)
#define MPI_CFG_IDLECLKS	GENMASK_ULL(9, 8)
#define MPI_CFG_TRITX		BIT_ULL(10)
#define MPI_CFG_CSLATE		BIT_ULL(11)
#define MPI_CFG_CSENA0		BIT_ULL(12)
#define MPI_CFG_CSENA1		BIT_ULL(13)
#define MPI_CFG_CSENA2		BIT_ULL(14)
#define MPI_CFG_CSENA3		BIT_ULL(15)
#define MPI_CFG_CLKDIV		GENMASK_ULL(28, 16)
#define MPI_CFG_LEGACY_DIS	BIT_ULL(31)
#define MPI_CFG_IOMODE		GENMASK_ULL(35, 34)
#define MPI_CFG_TB100_EN	BIT_ULL(49)

#define MPI_DAT_DATA		GENMASK_ULL(7, 0)

#define MPI_STS_BUSY		BIT_ULL(0)
#define MPI_STS_MPI_INTR	BIT_ULL(1)
#define MPI_STS_RXNUM		GENMASK_ULL(12, 8)

#define MPI_TX_TOTNUM		GENMASK_ULL(4, 0)
#define MPI_TX_TXNUM		GENMASK_ULL(12, 8)
#define MPI_TX_LEAVECS		BIT_ULL(16)
#define MPI_TX_CSID		GENMASK_ULL(21, 20)

#define MPI_XMIT_TOTNUM		GENMASK_ULL(10, 0)
#define MPI_XMIT_TXNUM		GENMASK_ULL(30, 20)
#define MPI_XMIT_BUF_SEL	BIT_ULL(59)
#define MPI_XMIT_LEAVECS	BIT_ULL(60)
#define MPI_XMIT_CSID		GENMASK_ULL(62, 61)

/* Used on Octeon TX2 */
void board_acquire_flash_arb(bool acquire);

/* Local driver data structure */
struct octeon_spi {
	void __iomem *base;	/* Register base address */
	struct clk clk;
	u32 clkdiv;		/* Clock divisor for device speed */
};

static u64 octeon_spi_set_mpicfg(struct udevice *dev)
{
	struct dm_spi_slave_plat *slave = dev_get_parent_plat(dev);
	struct udevice *bus = dev_get_parent(dev);
	struct octeon_spi *priv = dev_get_priv(bus);
	u64 mpi_cfg;
	uint max_speed = slave->max_hz;
	bool cpha, cpol;

	if (!max_speed)
		max_speed = 12500000;
	if (max_speed > OCTEON_SPI_MAX_CLOCK_HZ)
		max_speed = OCTEON_SPI_MAX_CLOCK_HZ;

	debug("\n slave params %d %d %d\n", slave->cs,
	      slave->max_hz, slave->mode);
	cpha = !!(slave->mode & SPI_CPHA);
	cpol = !!(slave->mode & SPI_CPOL);

	mpi_cfg = FIELD_PREP(MPI_CFG_CLKDIV, priv->clkdiv & 0x1fff) |
		FIELD_PREP(MPI_CFG_CSHI, !!(slave->mode & SPI_CS_HIGH)) |
		FIELD_PREP(MPI_CFG_LSBFIRST, !!(slave->mode & SPI_LSB_FIRST)) |
		FIELD_PREP(MPI_CFG_WIREOR, !!(slave->mode & SPI_3WIRE)) |
		FIELD_PREP(MPI_CFG_IDLELO, cpha != cpol) |
		FIELD_PREP(MPI_CFG_CSLATE, cpha) |
		MPI_CFG_CSENA0 | MPI_CFG_CSENA1 |
		MPI_CFG_CSENA2 | MPI_CFG_CSENA1 |
		MPI_CFG_ENABLE;

	debug("\n mpi_cfg %llx\n", mpi_cfg);
	return mpi_cfg;
}

/**
 * Wait until the SPI bus is ready
 *
 * @param	dev	SPI device to wait for
 */
static void octeon_spi_wait_ready(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct octeon_spi *priv = dev_get_priv(bus);
	void *base = priv->base;
	u64 mpi_sts;

	do {
		mpi_sts = readq(base + MPI_STS);
		schedule();
	} while (mpi_sts & MPI_STS_BUSY);

	debug("%s(%s)\n", __func__, dev->name);
}

/**
 * Claim the bus for a slave device
 *
 * @param	dev	SPI bus
 *
 * Return:	0 for success, -EINVAL if chip select is invalid
 */
static int octeon_spi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct octeon_spi *priv = dev_get_priv(bus);
	void *base = priv->base;
	u64 mpi_cfg;

	debug("\n\n%s(%s)\n", __func__, dev->name);
	if (!OCTEON_SPI_CS_VALID(spi_chip_select(dev)))
		return -EINVAL;

	if (IS_ENABLED(CONFIG_ARCH_OCTEONTX2))
		board_acquire_flash_arb(true);

	mpi_cfg = readq(base + MPI_CFG);
	mpi_cfg &= ~MPI_CFG_TRITX;
	mpi_cfg |= MPI_CFG_ENABLE;
	writeq(mpi_cfg, base + MPI_CFG);
	mpi_cfg = readq(base + MPI_CFG);
	udelay(5);	/** Wait for bus to settle */

	return 0;
}

/**
 * Release the bus to a slave device
 *
 * @param	dev	SPI bus
 *
 * Return:	0 for success, -EINVAL if chip select is invalid
 */
static int octeon_spi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev_get_parent(dev);
	struct octeon_spi *priv = dev_get_priv(bus);
	void *base = priv->base;
	u64 mpi_cfg;

	debug("%s(%s)\n\n", __func__, dev->name);
	if (!OCTEON_SPI_CS_VALID(spi_chip_select(dev)))
		return -EINVAL;

	if (IS_ENABLED(CONFIG_ARCH_OCTEONTX2))
		board_acquire_flash_arb(false);

	mpi_cfg = readq(base + MPI_CFG);
	mpi_cfg &= ~MPI_CFG_ENABLE;
	writeq(mpi_cfg, base + MPI_CFG);
	mpi_cfg = readq(base + MPI_CFG);
	udelay(1);

	return 0;
}

static int octeon_spi_xfer(struct udevice *dev, unsigned int bitlen,
			   const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev_get_parent(dev);
	struct octeon_spi *priv = dev_get_priv(bus);
	void *base = priv->base;
	u64 mpi_tx;
	u64 mpi_cfg;
	u64 wide_dat = 0;
	int len = bitlen / 8;
	int i;
	const u8 *tx_data = dout;
	u8 *rx_data = din;
	int cs = spi_chip_select(dev);

	if (!OCTEON_SPI_CS_VALID(cs))
		return -EINVAL;

	debug("\n %s(%s, %u, %p, %p, 0x%lx), cs: %d\n",
	      __func__, dev->name, bitlen, dout, din, flags, cs);

	mpi_cfg = octeon_spi_set_mpicfg(dev);
	if (mpi_cfg != readq(base + MPI_CFG)) {
		writeq(mpi_cfg, base + MPI_CFG);
		mpi_cfg = readq(base + MPI_CFG);
		udelay(10);
	}

	debug("\n mpi_cfg upd %llx\n", mpi_cfg);

	/*
	 * Start by writing and reading 8 bytes at a time. While we can support
	 * up to 10, it's easier to just use 8 with the MPI_WIDE_DAT register.
	 */
	while (len > 8) {
		if (tx_data) {
			wide_dat = get_unaligned((u64 *)tx_data);
			debug("  tx: %016llx \t", (unsigned long long)wide_dat);
			tx_data += 8;
			writeq(wide_dat, base + MPI_WIDE_DAT);
		}

		mpi_tx = FIELD_PREP(MPI_TX_CSID, cs) |
			FIELD_PREP(MPI_TX_LEAVECS, 1) |
			FIELD_PREP(MPI_TX_TXNUM, tx_data ? 8 : 0) |
			FIELD_PREP(MPI_TX_TOTNUM, 8);
		writeq(mpi_tx, base + MPI_TX);

		octeon_spi_wait_ready(dev);

		debug("\n ");

		if (rx_data) {
			wide_dat = readq(base + MPI_WIDE_DAT);
			debug("  rx: %016llx\t", (unsigned long long)wide_dat);
			*(u64 *)rx_data = wide_dat;
			rx_data += 8;
		}
		len -= 8;
	}

	debug("\n ");

	/* Write and read the rest of the data */
	if (tx_data) {
		for (i = 0; i < len; i++) {
			debug("  tx: %02x\n", *tx_data);
			writeq(*tx_data++, base + MPI_DAT(i));
		}
	}

	mpi_tx = FIELD_PREP(MPI_TX_CSID, cs) |
		FIELD_PREP(MPI_TX_LEAVECS, !(flags & SPI_XFER_END)) |
		FIELD_PREP(MPI_TX_TXNUM, tx_data ? len : 0) |
		FIELD_PREP(MPI_TX_TOTNUM, len);
	writeq(mpi_tx, base + MPI_TX);

	octeon_spi_wait_ready(dev);

	debug("\n ");

	if (rx_data) {
		for (i = 0; i < len; i++) {
			*rx_data = readq(base + MPI_DAT(i)) & 0xff;
			debug("  rx: %02x\n", *rx_data);
			rx_data++;
		}
	}

	return 0;
}

static int octeontx2_spi_xfer(struct udevice *dev, unsigned int bitlen,
			      const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev_get_parent(dev);
	struct octeon_spi *priv = dev_get_priv(bus);
	void *base = priv->base;
	u64 mpi_xmit;
	u64 mpi_cfg;
	u64 wide_dat = 0;
	int len = bitlen / 8;
	int rem;
	int i;
	const u8 *tx_data = dout;
	u8 *rx_data = din;
	int cs = spi_chip_select(dev);

	if (!OCTEON_SPI_CS_VALID(cs))
		return -EINVAL;

	debug("\n %s(%s, %u, %p, %p, 0x%lx), cs: %d\n",
	      __func__, dev->name, bitlen, dout, din, flags, cs);

	mpi_cfg = octeon_spi_set_mpicfg(dev);

	mpi_cfg |= MPI_CFG_TRITX | MPI_CFG_LEGACY_DIS | MPI_CFG_CS_STICKY |
		MPI_CFG_TB100_EN;

	mpi_cfg &= ~MPI_CFG_IOMODE;
	if (flags & (SPI_TX_DUAL | SPI_RX_DUAL))
		mpi_cfg |= FIELD_PREP(MPI_CFG_IOMODE, 2);
	if (flags & (SPI_TX_QUAD | SPI_RX_QUAD))
		mpi_cfg |= FIELD_PREP(MPI_CFG_IOMODE, 3);

	if (mpi_cfg != readq(base + MPI_CFG)) {
		writeq(mpi_cfg, base + MPI_CFG);
		mpi_cfg = readq(base + MPI_CFG);
		udelay(10);
	}

	debug("\n mpi_cfg upd %llx\n\n", mpi_cfg);

	/* Start by writing or reading 1024 bytes at a time. */
	while (len > 1024) {
		if (tx_data) {
			/* 8 bytes per iteration */
			for (i = 0; i < 128; i++) {
				wide_dat = get_unaligned((u64 *)tx_data);
				debug("  tx: %016llx \t",
				      (unsigned long long)wide_dat);
				if ((i % 4) == 3)
					debug("\n");
				tx_data += 8;
				writeq(wide_dat, base + MPI_WIDE_BUF(i));
			}
		}

		mpi_xmit = FIELD_PREP(MPI_XMIT_CSID, cs) | MPI_XMIT_LEAVECS |
			FIELD_PREP(MPI_XMIT_TXNUM, tx_data ? 1024 : 0) |
			FIELD_PREP(MPI_XMIT_TOTNUM, 1024);
		writeq(mpi_xmit, base + MPI_XMIT);

		octeon_spi_wait_ready(dev);

		debug("\n ");

		if (rx_data) {
			/* 8 bytes per iteration */
			for (i = 0; i < 128; i++) {
				wide_dat = readq(base + MPI_WIDE_BUF(i));
				debug("  rx: %016llx\t",
				      (unsigned long long)wide_dat);
				if ((i % 4) == 3)
					debug("\n");
				*(u64 *)rx_data = wide_dat;
				rx_data += 8;
			}
		}
		len -= 1024;
	}

	if (tx_data) {
		rem = len % 8;
		/* 8 bytes per iteration */
		for (i = 0; i < len / 8; i++) {
			wide_dat = get_unaligned((u64 *)tx_data);
			debug("  tx: %016llx \t",
			      (unsigned long long)wide_dat);
			if ((i % 4) == 3)
				debug("\n");
			tx_data += 8;
			writeq(wide_dat, base + MPI_WIDE_BUF(i));
		}
		if (rem) {
			memcpy(&wide_dat, tx_data, rem);
			debug("  rtx: %016llx\t", wide_dat);
			writeq(wide_dat, base + MPI_WIDE_BUF(i));
		}
	}

	mpi_xmit = FIELD_PREP(MPI_XMIT_CSID, cs) |
		FIELD_PREP(MPI_XMIT_LEAVECS, !(flags & SPI_XFER_END)) |
		FIELD_PREP(MPI_XMIT_TXNUM, tx_data ? len : 0) |
		FIELD_PREP(MPI_XMIT_TOTNUM, len);
	writeq(mpi_xmit, base + MPI_XMIT);

	octeon_spi_wait_ready(dev);

	debug("\n ");

	if (rx_data) {
		rem = len % 8;
		/* 8 bytes per iteration */
		for (i = 0; i < len / 8; i++) {
			wide_dat = readq(base + MPI_WIDE_BUF(i));
			debug("  rx: %016llx\t",
			      (unsigned long long)wide_dat);
			if ((i % 4) == 3)
				debug("\n");
			*(u64 *)rx_data = wide_dat;
			rx_data += 8;
		}
		if (rem) {
			wide_dat = readq(base + MPI_WIDE_BUF(i));
			debug("  rrx: %016llx\t",
			      (unsigned long long)wide_dat);
			memcpy(rx_data, &wide_dat, rem);
			rx_data += rem;
		}
	}

	return 0;
}

static bool octeon_spi_supports_op(struct spi_slave *slave,
				   const struct spi_mem_op *op)
{
	/* For now, support only below combinations
	 * 1-1-1
	 * 1-1-2 1-2-2
	 * 1-1-4 1-4-4
	 */
	if (op->cmd.buswidth != 1)
		return false;
	return true;
}

static int octeon_spi_exec_op(struct spi_slave *slave,
			      const struct spi_mem_op *op)
{
	unsigned long flags = SPI_XFER_BEGIN;
	const void *tx;
	void *rx;
	u8 opcode, *buf;
	u8 *addr;
	int i, temp, ret;

	if (op->cmd.buswidth != 1)
		return -ENOTSUPP;

	/* Send CMD */
	i = 0;
	opcode = op->cmd.opcode;

	if (!op->data.nbytes && !op->addr.nbytes && !op->dummy.nbytes)
		flags |= SPI_XFER_END;

	ret = octeontx2_spi_xfer(slave->dev, 8, (void *)&opcode, NULL, flags);
	if (ret < 0)
		return ret;

	/* Send Address and dummy */
	if (op->addr.nbytes) {
		/* Alloc buffer for address+dummy */
		buf = (u8 *)calloc(1, op->addr.nbytes + op->dummy.nbytes);
		if (!buf) {
			printf("%s Out of memory\n", __func__);
			return -ENOMEM;
		}
		addr = (u8 *)&op->addr.val;
		for (temp = 0; temp < op->addr.nbytes; temp++)
			buf[i++] = *(u8 *)(addr + op->addr.nbytes - 1 - temp);
		for (temp = 0; temp < op->dummy.nbytes; temp++)
			buf[i++] = 0xff;
		if (op->addr.buswidth == 2)
			flags |= SPI_RX_DUAL;
		if (op->addr.buswidth == 4)
			flags |= SPI_RX_QUAD;

		if (!op->data.nbytes)
			flags |= SPI_XFER_END;
		ret = octeontx2_spi_xfer(slave->dev, i * 8, (void *)buf, NULL,
					 flags);
		free(buf);
		if (ret < 0)
			return ret;
	}
	if (!op->data.nbytes)
		return 0;

	/* Send/Receive Data */
	flags |= SPI_XFER_END;
	if (op->data.buswidth == 2)
		flags |= SPI_RX_DUAL;
	if (op->data.buswidth == 4)
		flags |= SPI_RX_QUAD;

	rx = (op->data.dir == SPI_MEM_DATA_IN) ? op->data.buf.in : NULL;
	tx = (op->data.dir == SPI_MEM_DATA_OUT) ? op->data.buf.out : NULL;

	ret = octeontx2_spi_xfer(slave->dev, (op->data.nbytes * 8), tx, rx,
				 flags);
	return ret;
}

static const struct spi_controller_mem_ops octeontx2_spi_mem_ops = {
	.supports_op = octeon_spi_supports_op,
	.exec_op = octeon_spi_exec_op,
};

/**
 * Set the speed of the SPI bus
 *
 * @param	bus	bus to set
 * @param	max_hz	maximum speed supported
 */
static int octeon_spi_set_speed(struct udevice *bus, uint max_hz)
{
	struct octeon_spi *priv = dev_get_priv(bus);
	ulong clk_rate;
	u32 calc_hz;

	if (max_hz > OCTEON_SPI_MAX_CLOCK_HZ)
		max_hz = OCTEON_SPI_MAX_CLOCK_HZ;

	if (device_is_compatible(bus, "cavium,thunderx-spi"))
		clk_rate = 100000000;
	else
		clk_rate = clk_get_rate(&priv->clk);
	if (IS_ERR_VALUE(clk_rate))
		return -EINVAL;

	debug("%s(%s, %u, %lu)\n", __func__, bus->name, max_hz, clk_rate);

	priv->clkdiv = clk_rate / (2 * max_hz);
	while (1) {
		calc_hz = clk_rate / (2 * priv->clkdiv);
		if (calc_hz <= max_hz)
			break;
		priv->clkdiv += 1;
	}

	if (priv->clkdiv > 8191)
		return -EINVAL;

	debug("%s: clkdiv=%d\n", __func__, priv->clkdiv);

	return 0;
}

static int octeon_spi_set_mode(struct udevice *bus, uint mode)
{
	/* We don't set it here */
	return 0;
}

static struct dm_spi_ops octeon_spi_ops = {
	.claim_bus	= octeon_spi_claim_bus,
	.release_bus	= octeon_spi_release_bus,
	.set_speed	= octeon_spi_set_speed,
	.set_mode	= octeon_spi_set_mode,
	.xfer		= octeon_spi_xfer,
};

static int octeon_spi_probe(struct udevice *dev)
{
	struct octeon_spi *priv = dev_get_priv(dev);
	int ret;

	/* Octeon TX & TX2 use PCI based probing */
	if (device_is_compatible(dev, "cavium,thunder-8190-spi")) {
		pci_dev_t bdf = dm_pci_get_bdf(dev);

		debug("SPI PCI device: %x\n", bdf);
		priv->base = dm_pci_map_bar(dev, PCI_BASE_ADDRESS_0, 0, 0, PCI_REGION_TYPE,
					    PCI_REGION_MEM);
		/* Add base offset */
		priv->base += 0x1000;

		/*
		 * Octeon TX2 needs a different xfer function and supports
		 * mem_ops
		 */
		if (device_is_compatible(dev, "cavium,thunderx-spi")) {
			octeon_spi_ops.xfer = octeontx2_spi_xfer;
			octeon_spi_ops.mem_ops = &octeontx2_spi_mem_ops;
		}
	} else {
		priv->base = dev_remap_addr(dev);
	}

	ret = clk_get_by_index(dev, 0, &priv->clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&priv->clk);
	if (ret)
		return ret;

	debug("SPI bus %s %d at %p\n", dev->name, dev_seq(dev), priv->base);

	return 0;
}

static const struct udevice_id octeon_spi_ids[] = {
	/* MIPS Octeon */
	{ .compatible = "cavium,octeon-3010-spi" },
	/* ARM Octeon TX / TX2 */
	{ .compatible = "cavium,thunder-8190-spi" },
	{ }
};

U_BOOT_DRIVER(octeon_spi) = {
	.name			= "spi_octeon",
	.id			= UCLASS_SPI,
	.of_match		= octeon_spi_ids,
	.probe			= octeon_spi_probe,
	.priv_auto	= sizeof(struct octeon_spi),
	.ops			= &octeon_spi_ops,
};

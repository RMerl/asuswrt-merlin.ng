// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Nuvoton Technology Corp.
 * NPCM Flash Interface Unit(FIU) SPI master controller driver.
 */

#include <clk.h>
#include <dm.h>
#include <spi.h>
#include <spi-mem.h>
#include <linux/bitfield.h>
#include <linux/log2.h>
#include <linux/iopoll.h>
#include <power/regulator.h>

#define DW_SIZE			4
#define CHUNK_SIZE		16
#define XFER_TIMEOUT		1000000

/* FIU UMA Configuration Register (UMA_CFG) */
#define UMA_CFG_RDATSIZ_MASK	GENMASK(28, 24)
#define UMA_CFG_DBSIZ_MASK	GENMASK(23, 21)
#define UMA_CFG_WDATSIZ_MASK	GENMASK(20, 16)
#define UMA_CFG_ADDSIZ_MASK	GENMASK(13, 11)
#define UMA_CFG_RDBPCK_MASK	GENMASK(9, 8)
#define UMA_CFG_DBPCK_MASK	GENMASK(7, 6)
#define UMA_CFG_WDBPCK_MASK	GENMASK(5, 4)
#define UMA_CFG_ADBPCK_MASK	GENMASK(3, 2)
#define UMA_CFG_CMBPCK_MASK	GENMASK(1, 0)
#define UMA_CFG_CMDSIZ_SHIFT	10

/* FIU UMA Control and Status Register (UMA_CTS) */
#define UMA_CTS_SW_CS		BIT(16)
#define UMA_CTS_EXEC_DONE	BIT(0)
#define UMA_CTS_RDYST		BIT(24)
#define UMA_CTS_DEV_NUM_MASK	GENMASK(9, 8)

/* Direct Write Configuration Register */
#define DWR_CFG_WBURST_MASK	GENMASK(25, 24)
#define DWR_CFG_ADDSIZ_MASK	GENMASK(17, 16)
#define DWR_CFG_ABPCK_MASK	GENMASK(11, 10)
#define DRW_CFG_DBPCK_MASK	GENMASK(9, 8)
#define DRW_CFG_WRCMD		2
enum {
	DWR_WBURST_1_BYTE,
	DWR_WBURST_16_BYTE = 3,
};

enum {
	DWR_ADDSIZ_24_BIT,
	DWR_ADDSIZ_32_BIT,
};

enum {
	DWR_ABPCK_BIT_PER_CLK,
	DWR_ABPCK_2_BIT_PER_CLK,
	DWR_ABPCK_4_BIT_PER_CLK,
};

enum {
	DWR_DBPCK_BIT_PER_CLK,
	DWR_DBPCK_2_BIT_PER_CLK,
	DWR_DBPCK_4_BIT_PER_CLK,
};

struct npcm_fiu_regs {
	unsigned int    drd_cfg;
	unsigned int    dwr_cfg;
	unsigned int    uma_cfg;
	unsigned int    uma_cts;
	unsigned int    uma_cmd;
	unsigned int    uma_addr;
	unsigned int    prt_cfg;
	unsigned char	res1[4];
	unsigned int    uma_dw0;
	unsigned int    uma_dw1;
	unsigned int    uma_dw2;
	unsigned int    uma_dw3;
	unsigned int    uma_dr0;
	unsigned int    uma_dr1;
	unsigned int    uma_dr2;
	unsigned int    uma_dr3;
	unsigned int    prt_cmd0;
	unsigned int    prt_cmd1;
	unsigned int    prt_cmd2;
	unsigned int    prt_cmd3;
	unsigned int    prt_cmd4;
	unsigned int    prt_cmd5;
	unsigned int    prt_cmd6;
	unsigned int    prt_cmd7;
	unsigned int    prt_cmd8;
	unsigned int    prt_cmd9;
	unsigned int    stuff[4];
	unsigned int    fiu_cfg;
};

struct npcm_fiu_priv {
	struct npcm_fiu_regs *regs;
};

static int npcm_fiu_spi_set_speed(struct udevice *bus, uint speed)
{
	return 0;
}

static int npcm_fiu_spi_set_mode(struct udevice *bus, uint mode)
{
	return 0;
}

static inline void activate_cs(struct npcm_fiu_regs *regs, int cs)
{
	writel(FIELD_PREP(UMA_CTS_DEV_NUM_MASK, cs), &regs->uma_cts);
}

static inline void deactivate_cs(struct npcm_fiu_regs *regs, int cs)
{
	writel(FIELD_PREP(UMA_CTS_DEV_NUM_MASK, cs) | UMA_CTS_SW_CS, &regs->uma_cts);
}

static int fiu_uma_read(struct udevice *bus, u8 *buf, u32 size)
{
	struct npcm_fiu_priv *priv = dev_get_priv(bus);
	struct npcm_fiu_regs *regs = priv->regs;
	u32 data_reg[4];
	u32 val;
	int ret;

	/* Set data size */
	writel(FIELD_PREP(UMA_CFG_RDATSIZ_MASK, size), &regs->uma_cfg);

	/* Initiate the read */
	writel(readl(&regs->uma_cts) | UMA_CTS_EXEC_DONE, &regs->uma_cts);

	/* Wait for completion */
	ret = readl_poll_timeout(&regs->uma_cts, val,
				 !(val & UMA_CTS_EXEC_DONE), XFER_TIMEOUT);
	if (ret) {
		printf("npcm_fiu: read timeout\n");
		return ret;
	}

	/* Copy data from data registers */
	if (size)
		data_reg[0] = readl(&regs->uma_dr0);
	if (size > DW_SIZE)
		data_reg[1] = readl(&regs->uma_dr1);
	if (size > DW_SIZE * 2)
		data_reg[2] = readl(&regs->uma_dr2);
	if (size > DW_SIZE * 3)
		data_reg[3] = readl(&regs->uma_dr3);
	memcpy(buf, data_reg, size);

	return 0;
}

static int fiu_uma_write(struct udevice *bus, const u8 *buf, u32 size)
{
	struct npcm_fiu_priv *priv = dev_get_priv(bus);
	struct npcm_fiu_regs *regs = priv->regs;
	u32 data_reg[4];
	u32 val;
	int ret;

	/* Set data size */
	writel(FIELD_PREP(UMA_CFG_WDATSIZ_MASK, size), &regs->uma_cfg);

	/* Write data to data registers */
	memcpy(data_reg, buf, size);
	if (size)
		writel(data_reg[0], &regs->uma_dw0);
	if (size > DW_SIZE)
		writel(data_reg[1], &regs->uma_dw1);
	if (size > DW_SIZE * 2)
		writel(data_reg[2], &regs->uma_dw2);
	if (size > DW_SIZE * 3)
		writel(data_reg[3], &regs->uma_dw3);

	/* Initiate the transaction */
	writel(readl(&regs->uma_cts) | UMA_CTS_EXEC_DONE, &regs->uma_cts);

	/* Wait for completion */
	ret = readl_poll_timeout(&regs->uma_cts, val,
				 !(val & UMA_CTS_EXEC_DONE), XFER_TIMEOUT);
	if (ret)
		printf("npcm_fiu: write timeout\n");

	return ret;
}

static int npcm_fiu_spi_xfer(struct udevice *dev, unsigned int bitlen,
			     const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct npcm_fiu_priv *priv = dev_get_priv(bus);
	struct npcm_fiu_regs *regs = priv->regs;
	struct dm_spi_slave_plat *slave_plat =
			dev_get_parent_plat(dev);
	const u8 *tx = dout;
	u8 *rx = din;
	int bytes = bitlen / 8;
	int ret = 0;
	int len;

	if (flags & SPI_XFER_BEGIN)
		activate_cs(regs, slave_plat->cs);

	while (bytes) {
		len = (bytes > CHUNK_SIZE) ? CHUNK_SIZE : bytes;
		if (tx) {
			ret = fiu_uma_write(bus, tx, len);
			if (ret)
				break;
			tx += len;
		} else {
			ret = fiu_uma_read(bus, rx, len);
			if (ret)
				break;
			rx += len;
		}
		bytes -= len;
	}

	if (flags & SPI_XFER_END)
		deactivate_cs(regs, slave_plat->cs);

	return ret;
}

static int npcm_fiu_uma_operation(struct npcm_fiu_priv *priv, const struct spi_mem_op *op,
				  u32 addr, const u8 *tx, u8 *rx, u32 nbytes, bool started)
{
	struct npcm_fiu_regs *regs = priv->regs;
	u32 uma_cfg = 0, val;
	u32 data_reg[4];
	int ret;

	debug("fiu_uma: opcode 0x%x, dir %d, addr 0x%x, %d bytes\n",
	      op->cmd.opcode, op->data.dir, addr, nbytes);
	debug("         buswidth cmd:%d, addr:%d, dummy:%d, data:%d\n",
	      op->cmd.buswidth, op->addr.buswidth, op->dummy.buswidth,
	      op->data.buswidth);
	debug("         size cmd:%d, addr:%d, dummy:%d, data:%d\n",
	      1, op->addr.nbytes, op->dummy.nbytes, op->data.nbytes);
	debug("         tx %p, rx %p\n", tx, rx);

	if (!started) {
		/* Send cmd/addr in the begin of an transaction */
		writel(op->cmd.opcode, &regs->uma_cmd);

		uma_cfg |= FIELD_PREP(UMA_CFG_CMBPCK_MASK, ilog2(op->cmd.buswidth)) |
			   (1 << UMA_CFG_CMDSIZ_SHIFT);
		/* Configure addr bytes */
		if (op->addr.nbytes) {
			uma_cfg |= FIELD_PREP(UMA_CFG_ADBPCK_MASK, ilog2(op->addr.buswidth)) |
				   FIELD_PREP(UMA_CFG_ADDSIZ_MASK, op->addr.nbytes);
			writel(addr, &regs->uma_addr);
		}
		/* Configure dummy bytes */
		if (op->dummy.nbytes)
			uma_cfg |= FIELD_PREP(UMA_CFG_DBPCK_MASK, ilog2(op->dummy.buswidth)) |
				   FIELD_PREP(UMA_CFG_DBSIZ_MASK, op->dummy.nbytes);
	}
	/* Set data bus width and data size */
	if (op->data.dir == SPI_MEM_DATA_IN && nbytes)
		uma_cfg |= FIELD_PREP(UMA_CFG_RDBPCK_MASK, ilog2(op->data.buswidth)) |
			   FIELD_PREP(UMA_CFG_RDATSIZ_MASK, nbytes);
	else if (op->data.dir == SPI_MEM_DATA_OUT && nbytes)
		uma_cfg |= FIELD_PREP(UMA_CFG_WDBPCK_MASK, ilog2(op->data.buswidth)) |
			   FIELD_PREP(UMA_CFG_WDATSIZ_MASK, nbytes);
	writel(uma_cfg, &regs->uma_cfg);

	if (op->data.dir == SPI_MEM_DATA_OUT && nbytes) {
		memcpy(data_reg, tx, nbytes);

		if (nbytes)
			writel(data_reg[0], &regs->uma_dw0);
		if (nbytes > DW_SIZE)
			writel(data_reg[1], &regs->uma_dw1);
		if (nbytes > DW_SIZE * 2)
			writel(data_reg[2], &regs->uma_dw2);
		if (nbytes > DW_SIZE * 3)
			writel(data_reg[3], &regs->uma_dw3);
	}
	/* Initiate the transaction */
	writel(readl(&regs->uma_cts) | UMA_CTS_EXEC_DONE, &regs->uma_cts);

	/* Wait for completion */
	ret = readl_poll_timeout(&regs->uma_cts, val,
				 !(val & UMA_CTS_EXEC_DONE), XFER_TIMEOUT);
	if (ret) {
		printf("npcm_fiu: UMA op timeout\n");
		return ret;
	}

	if (op->data.dir == SPI_MEM_DATA_IN && nbytes) {
		if (nbytes)
			data_reg[0] = readl(&regs->uma_dr0);
		if (nbytes > DW_SIZE)
			data_reg[1] = readl(&regs->uma_dr1);
		if (nbytes > DW_SIZE * 2)
			data_reg[2] = readl(&regs->uma_dr2);
		if (nbytes > DW_SIZE * 3)
			data_reg[3] = readl(&regs->uma_dr3);

		memcpy(rx, data_reg, nbytes);
	}

	return 0;
}

static int npcm_fiu_exec_op(struct spi_slave *slave,
			    const struct spi_mem_op *op)
{
	struct udevice *bus = slave->dev->parent;
	struct npcm_fiu_priv *priv = dev_get_priv(bus);
	struct npcm_fiu_regs *regs = priv->regs;
	struct dm_spi_slave_plat *slave_plat = dev_get_parent_plat(slave->dev);
	u32 bytes, len, addr;
	const u8 *tx;
	u8 *rx;
	bool started = false;
	int ret;

	bytes = op->data.nbytes;
	addr = (u32)op->addr.val;
	if (!bytes) {
		activate_cs(regs, slave_plat->cs);
		ret = npcm_fiu_uma_operation(priv, op, addr, NULL, NULL, 0, false);
		deactivate_cs(regs, slave_plat->cs);
		return ret;
	}

	tx = op->data.buf.out;
	rx = op->data.buf.in;
	/*
	 * Use SW-control CS for write to extend the transaction and
	 *     keep the Write Enable state.
	 * Use HW-control CS for read to avoid clock and timing issues.
	 */
	if (op->data.dir == SPI_MEM_DATA_OUT)
		activate_cs(regs, slave_plat->cs);
	else
		writel(FIELD_PREP(UMA_CTS_DEV_NUM_MASK, slave_plat->cs) | UMA_CTS_SW_CS,
		       &regs->uma_cts);
	while (bytes) {
		len = (bytes > CHUNK_SIZE) ? CHUNK_SIZE : bytes;
		ret = npcm_fiu_uma_operation(priv, op, addr, tx, rx, len, started);
		if (ret)
			return ret;

		/* CS is kept low for uma write, extend the transaction */
		if (op->data.dir == SPI_MEM_DATA_OUT)
			started = true;

		bytes -= len;
		addr += len;
		if (tx)
			tx += len;
		if (rx)
			rx += len;
	}
	if (op->data.dir == SPI_MEM_DATA_OUT)
		deactivate_cs(regs, slave_plat->cs);

	return 0;
}

static int npcm_fiu_spi_probe(struct udevice *bus)
{
	struct npcm_fiu_priv *priv = dev_get_priv(bus);
	struct udevice *vqspi_supply;
	int vqspi_uv;

	priv->regs = (struct npcm_fiu_regs *)dev_read_addr_ptr(bus);

	if (IS_ENABLED(CONFIG_DM_REGULATOR)) {
		device_get_supply_regulator(bus, "vqspi-supply", &vqspi_supply);
		vqspi_uv = dev_read_u32_default(bus, "vqspi-microvolt", 0);
		/* Set IO voltage */
		if (vqspi_supply && vqspi_uv)
			regulator_set_value(vqspi_supply, vqspi_uv);
	}

	return 0;
}

static int npcm_fiu_spi_bind(struct udevice *bus)
{
	struct npcm_fiu_regs *regs;

	if (dev_read_bool(bus, "nuvoton,spix-mode")) {
		regs = dev_read_addr_ptr(bus);
		if (!regs)
			return -EINVAL;

		/* Setup direct write cfg for SPIX */
		writel(FIELD_PREP(DWR_CFG_WBURST_MASK, DWR_WBURST_16_BYTE) |
		       FIELD_PREP(DWR_CFG_ADDSIZ_MASK, DWR_ADDSIZ_24_BIT) |
		       FIELD_PREP(DWR_CFG_ABPCK_MASK, DWR_ABPCK_4_BIT_PER_CLK) |
		       FIELD_PREP(DRW_CFG_DBPCK_MASK, DWR_DBPCK_4_BIT_PER_CLK) |
		       DRW_CFG_WRCMD, &regs->dwr_cfg);
	}

	return 0;
}

static const struct spi_controller_mem_ops npcm_fiu_mem_ops = {
	.exec_op = npcm_fiu_exec_op,
};

static const struct dm_spi_ops npcm_fiu_spi_ops = {
	.xfer           = npcm_fiu_spi_xfer,
	.set_speed      = npcm_fiu_spi_set_speed,
	.set_mode       = npcm_fiu_spi_set_mode,
	.mem_ops        = &npcm_fiu_mem_ops,
};

static const struct udevice_id npcm_fiu_spi_ids[] = {
	{ .compatible = "nuvoton,npcm845-fiu" },
	{ .compatible = "nuvoton,npcm750-fiu" },
	{ }
};

U_BOOT_DRIVER(npcm_fiu_spi) = {
	.name   = "npcm_fiu_spi",
	.id     = UCLASS_SPI,
	.of_match = npcm_fiu_spi_ids,
	.ops    = &npcm_fiu_spi_ops,
	.priv_auto = sizeof(struct npcm_fiu_priv),
	.probe  = npcm_fiu_spi_probe,
	.bind = npcm_fiu_spi_bind,
};

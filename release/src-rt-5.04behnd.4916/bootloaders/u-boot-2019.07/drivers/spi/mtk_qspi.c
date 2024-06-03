// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018  MediaTek, Inc.
 * Author : Guochun.Mao@mediatek.com
 */

#include <common.h>
#include <dm.h>
#include <malloc.h>
#include <spi.h>
#include <asm/io.h>
#include <linux/iopoll.h>
#include <linux/ioport.h>

/* Register Offset */
struct mtk_qspi_regs {
	u32 cmd;
	u32 cnt;
	u32 rdsr;
	u32 rdata;
	u32 radr[3];
	u32 wdata;
	u32 prgdata[6];
	u32 shreg[10];
	u32 cfg[2];
	u32 shreg10;
	u32 mode_mon;
	u32 status[4];
	u32 flash_time;
	u32 flash_cfg;
	u32 reserved_0[3];
	u32 sf_time;
	u32 pp_dw_data;
	u32 reserved_1;
	u32 delsel_0[2];
	u32 intrstus;
	u32 intren;
	u32 reserved_2;
	u32 cfg3;
	u32 reserved_3;
	u32 chksum;
	u32 aaicmd;
	u32 wrprot;
	u32 radr3;
	u32 dual;
	u32 delsel_1[3];
};

struct mtk_qspi_platdata {
	fdt_addr_t reg_base;
	fdt_addr_t mem_base;
};

struct mtk_qspi_priv {
	struct mtk_qspi_regs *regs;
	unsigned long *mem_base;
	u8 op;
	u8 tx[3]; /* only record max 3 bytes paras, when it's address. */
	u32 txlen; /* dout buffer length  - op code length */
	u8 *rx;
	u32 rxlen;
};

#define MTK_QSPI_CMD_POLLINGREG_US 500000
#define MTK_QSPI_WRBUF_SIZE        256
#define MTK_QSPI_COMMAND_ENABLE    0x30

/* NOR flash controller commands */
#define MTK_QSPI_RD_TRIGGER        BIT(0)
#define MTK_QSPI_READSTATUS        BIT(1)
#define MTK_QSPI_PRG_CMD           BIT(2)
#define MTK_QSPI_WR_TRIGGER        BIT(4)
#define MTK_QSPI_WRITESTATUS       BIT(5)
#define MTK_QSPI_AUTOINC           BIT(7)

#define MTK_QSPI_MAX_RX_TX_SHIFT   0x6
#define MTK_QSPI_MAX_SHIFT         0x8

#define MTK_QSPI_WR_BUF_ENABLE     0x1
#define MTK_QSPI_WR_BUF_DISABLE    0x0

static int mtk_qspi_execute_cmd(struct mtk_qspi_priv *priv, u8 cmd)
{
	u8 tmp;
	u8 val = cmd & ~MTK_QSPI_AUTOINC;

	writeb(cmd, &priv->regs->cmd);

	return readb_poll_timeout(&priv->regs->cmd, tmp, !(val & tmp),
				  MTK_QSPI_CMD_POLLINGREG_US);
}

static int mtk_qspi_tx_rx(struct mtk_qspi_priv *priv)
{
	int len = 1 + priv->txlen + priv->rxlen;
	int i, ret, idx;

	if (len > MTK_QSPI_MAX_SHIFT)
		return -ERR_INVAL;

	writeb(len * 8, &priv->regs->cnt);

	/* start at PRGDATA5, go down to PRGDATA0 */
	idx = MTK_QSPI_MAX_RX_TX_SHIFT - 1;

	/* opcode */
	writeb(priv->op, &priv->regs->prgdata[idx]);
	idx--;

	/* program TX data */
	for (i = 0; i < priv->txlen; i++, idx--)
		writeb(priv->tx[i], &priv->regs->prgdata[idx]);

	/* clear out rest of TX registers */
	while (idx >= 0) {
		writeb(0, &priv->regs->prgdata[idx]);
		idx--;
	}

	ret = mtk_qspi_execute_cmd(priv, MTK_QSPI_PRG_CMD);
	if (ret)
		return ret;

	/* restart at first RX byte */
	idx = priv->rxlen - 1;

	/* read out RX data */
	for (i = 0; i < priv->rxlen; i++, idx--)
		priv->rx[i] = readb(&priv->regs->shreg[idx]);

	return 0;
}

static int mtk_qspi_read(struct mtk_qspi_priv *priv,
			 u32 addr, u8 *buf, u32 len)
{
	memcpy(buf, (u8 *)priv->mem_base + addr, len);
	return 0;
}

static void mtk_qspi_set_addr(struct mtk_qspi_priv *priv, u32 addr)
{
	int i;

	for (i = 0; i < 3; i++) {
		writeb(addr & 0xff, &priv->regs->radr[i]);
		addr >>= 8;
	}
}

static int mtk_qspi_write_single_byte(struct mtk_qspi_priv *priv,
				      u32 addr, u32 length, const u8 *data)
{
	int i, ret;

	mtk_qspi_set_addr(priv, addr);

	for (i = 0; i < length; i++) {
		writeb(*data++, &priv->regs->wdata);
		ret = mtk_qspi_execute_cmd(priv, MTK_QSPI_WR_TRIGGER);
		if (ret < 0)
			return ret;
	}
	return 0;
}

static int mtk_qspi_write_buffer(struct mtk_qspi_priv *priv, u32 addr,
				 const u8 *buf)
{
	int i, data;

	mtk_qspi_set_addr(priv, addr);

	for (i = 0; i < MTK_QSPI_WRBUF_SIZE; i += 4) {
		data = buf[i + 3] << 24 | buf[i + 2] << 16 |
		       buf[i + 1] << 8 | buf[i];
		writel(data, &priv->regs->pp_dw_data);
	}

	return mtk_qspi_execute_cmd(priv, MTK_QSPI_WR_TRIGGER);
}

static int mtk_qspi_write(struct mtk_qspi_priv *priv,
			  u32 addr, const u8 *buf, u32 len)
{
	int ret;

	/* setting pre-fetch buffer for page program */
	writel(MTK_QSPI_WR_BUF_ENABLE, &priv->regs->cfg[1]);
	while (len >= MTK_QSPI_WRBUF_SIZE) {
		ret = mtk_qspi_write_buffer(priv, addr, buf);
		if (ret < 0)
			return ret;

		len -= MTK_QSPI_WRBUF_SIZE;
		addr += MTK_QSPI_WRBUF_SIZE;
		buf += MTK_QSPI_WRBUF_SIZE;
	}
	/* disable pre-fetch buffer for page program */
	writel(MTK_QSPI_WR_BUF_DISABLE, &priv->regs->cfg[1]);

	if (len)
		return mtk_qspi_write_single_byte(priv, addr, len, buf);

	return 0;
}

static int mtk_qspi_claim_bus(struct udevice *dev)
{
	/* nothing to do */
	return 0;
}

static int mtk_qspi_release_bus(struct udevice *dev)
{
	/* nothing to do */
	return 0;
}

static int mtk_qspi_transfer(struct mtk_qspi_priv *priv, unsigned int bitlen,
			     const void *dout, void *din, unsigned long flags)
{
	u32 bytes = DIV_ROUND_UP(bitlen, 8);
	u32 addr;

	if (!bytes)
		return -ERR_INVAL;

	if (dout) {
		if (flags & SPI_XFER_BEGIN) {
			/* parse op code and potential paras first */
			priv->op = *(u8 *)dout;
			if (bytes > 1)
				memcpy(priv->tx, (u8 *)dout + 1,
				       bytes <= 4 ? bytes - 1 : 3);
			priv->txlen = bytes - 1;
		}

		if (flags == SPI_XFER_ONCE) {
			/* operations without receiving or sending data.
			 * for example: erase, write flash register or write
			 * enable...
			 */
			priv->rx = NULL;
			priv->rxlen = 0;
			return mtk_qspi_tx_rx(priv);
		}

		if (flags & SPI_XFER_END) {
			/* here, dout should be data to be written.
			 * and priv->tx should be filled 3Bytes address.
			 */
			addr = priv->tx[0] << 16 | priv->tx[1] << 8 |
			       priv->tx[2];
			return mtk_qspi_write(priv, addr, (u8 *)dout, bytes);
		}
	}

	if (din) {
		if (priv->txlen >= 3) {
			/* if run to here, priv->tx[] should be the address
			 * where read data from,
			 * and, din is the buf to receive data.
			 */
			addr = priv->tx[0] << 16 | priv->tx[1] << 8 |
			       priv->tx[2];
			return mtk_qspi_read(priv, addr, (u8 *)din, bytes);
		}

		/* should be reading flash's register */
		priv->rx = (u8 *)din;
		priv->rxlen = bytes;
		return mtk_qspi_tx_rx(priv);
	}

	return 0;
}

static int mtk_qspi_xfer(struct udevice *dev, unsigned int bitlen,
			 const void *dout, void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct mtk_qspi_priv *priv = dev_get_priv(bus);

	return  mtk_qspi_transfer(priv, bitlen, dout, din, flags);
}

static int mtk_qspi_set_speed(struct udevice *bus, uint speed)
{
	/* nothing to do */
	return 0;
}

static int mtk_qspi_set_mode(struct udevice *bus, uint mode)
{
	/* nothing to do */
	return 0;
}

static int mtk_qspi_ofdata_to_platdata(struct udevice *bus)
{
	struct resource res_reg, res_mem;
	struct mtk_qspi_platdata *plat = bus->platdata;
	int ret;

	ret = dev_read_resource_byname(bus, "reg_base", &res_reg);
	if (ret) {
		debug("can't get reg_base resource(ret = %d)\n", ret);
		return -ENOMEM;
	}

	ret = dev_read_resource_byname(bus, "mem_base", &res_mem);
	if (ret) {
		debug("can't get map_base resource(ret = %d)\n", ret);
		return -ENOMEM;
	}

	plat->mem_base = res_mem.start;
	plat->reg_base = res_reg.start;

	return 0;
}

static int mtk_qspi_probe(struct udevice *bus)
{
	struct mtk_qspi_platdata *plat = dev_get_platdata(bus);
	struct mtk_qspi_priv *priv = dev_get_priv(bus);

	priv->regs = (struct mtk_qspi_regs *)plat->reg_base;
	priv->mem_base = (unsigned long *)plat->mem_base;

	writel(MTK_QSPI_COMMAND_ENABLE, &priv->regs->wrprot);

	return 0;
}

static const struct dm_spi_ops mtk_qspi_ops = {
	.claim_bus      = mtk_qspi_claim_bus,
	.release_bus    = mtk_qspi_release_bus,
	.xfer           = mtk_qspi_xfer,
	.set_speed      = mtk_qspi_set_speed,
	.set_mode       = mtk_qspi_set_mode,
};

static const struct udevice_id mtk_qspi_ids[] = {
	{ .compatible = "mediatek,mt7629-qspi" },
	{ }
};

U_BOOT_DRIVER(mtk_qspi) = {
	.name     = "mtk_qspi",
	.id       = UCLASS_SPI,
	.of_match = mtk_qspi_ids,
	.ops      = &mtk_qspi_ops,
	.ofdata_to_platdata       = mtk_qspi_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct mtk_qspi_platdata),
	.priv_auto_alloc_size     = sizeof(struct mtk_qspi_priv),
	.probe    = mtk_qspi_probe,
};

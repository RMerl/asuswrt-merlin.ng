// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016
 *
 * Michael Kurz, <michi.kurz@gmail.com>
 *
 * STM32 QSPI driver
 */

#include <common.h>
#include <clk.h>
#include <reset.h>
#include <spi-mem.h>
#include <linux/iopoll.h>
#include <linux/ioport.h>
#include <linux/sizes.h>

struct stm32_qspi_regs {
	u32 cr;		/* 0x00 */
	u32 dcr;	/* 0x04 */
	u32 sr;		/* 0x08 */
	u32 fcr;	/* 0x0C */
	u32 dlr;	/* 0x10 */
	u32 ccr;	/* 0x14 */
	u32 ar;		/* 0x18 */
	u32 abr;	/* 0x1C */
	u32 dr;		/* 0x20 */
	u32 psmkr;	/* 0x24 */
	u32 psmar;	/* 0x28 */
	u32 pir;	/* 0x2C */
	u32 lptr;	/* 0x30 */
};

/*
 * QUADSPI control register
 */
#define STM32_QSPI_CR_EN		BIT(0)
#define STM32_QSPI_CR_ABORT		BIT(1)
#define STM32_QSPI_CR_DMAEN		BIT(2)
#define STM32_QSPI_CR_TCEN		BIT(3)
#define STM32_QSPI_CR_SSHIFT		BIT(4)
#define STM32_QSPI_CR_DFM		BIT(6)
#define STM32_QSPI_CR_FSEL		BIT(7)
#define STM32_QSPI_CR_FTHRES_SHIFT	8
#define STM32_QSPI_CR_TEIE		BIT(16)
#define STM32_QSPI_CR_TCIE		BIT(17)
#define STM32_QSPI_CR_FTIE		BIT(18)
#define STM32_QSPI_CR_SMIE		BIT(19)
#define STM32_QSPI_CR_TOIE		BIT(20)
#define STM32_QSPI_CR_APMS		BIT(22)
#define STM32_QSPI_CR_PMM		BIT(23)
#define STM32_QSPI_CR_PRESCALER_MASK	GENMASK(7, 0)
#define STM32_QSPI_CR_PRESCALER_SHIFT	24

/*
 * QUADSPI device configuration register
 */
#define STM32_QSPI_DCR_CKMODE		BIT(0)
#define STM32_QSPI_DCR_CSHT_MASK	GENMASK(2, 0)
#define STM32_QSPI_DCR_CSHT_SHIFT	8
#define STM32_QSPI_DCR_FSIZE_MASK	GENMASK(4, 0)
#define STM32_QSPI_DCR_FSIZE_SHIFT	16

/*
 * QUADSPI status register
 */
#define STM32_QSPI_SR_TEF		BIT(0)
#define STM32_QSPI_SR_TCF		BIT(1)
#define STM32_QSPI_SR_FTF		BIT(2)
#define STM32_QSPI_SR_SMF		BIT(3)
#define STM32_QSPI_SR_TOF		BIT(4)
#define STM32_QSPI_SR_BUSY		BIT(5)

/*
 * QUADSPI flag clear register
 */
#define STM32_QSPI_FCR_CTEF		BIT(0)
#define STM32_QSPI_FCR_CTCF		BIT(1)
#define STM32_QSPI_FCR_CSMF		BIT(3)
#define STM32_QSPI_FCR_CTOF		BIT(4)

/*
 * QUADSPI communication configuration register
 */
#define STM32_QSPI_CCR_DDRM		BIT(31)
#define STM32_QSPI_CCR_DHHC		BIT(30)
#define STM32_QSPI_CCR_SIOO		BIT(28)
#define STM32_QSPI_CCR_FMODE_SHIFT	26
#define STM32_QSPI_CCR_DMODE_SHIFT	24
#define STM32_QSPI_CCR_DCYC_SHIFT	18
#define STM32_QSPI_CCR_ABSIZE_SHIFT	16
#define STM32_QSPI_CCR_ABMODE_SHIFT	14
#define STM32_QSPI_CCR_ADSIZE_SHIFT	12
#define STM32_QSPI_CCR_ADMODE_SHIFT	10
#define STM32_QSPI_CCR_IMODE_SHIFT	8

#define STM32_QSPI_CCR_IND_WRITE	0
#define STM32_QSPI_CCR_IND_READ		1
#define STM32_QSPI_CCR_MEM_MAP		3

#define STM32_QSPI_MAX_MMAP_SZ		SZ_256M
#define STM32_QSPI_MAX_CHIP		2

#define STM32_QSPI_FIFO_TIMEOUT_US	30000
#define STM32_QSPI_CMD_TIMEOUT_US	1000000
#define STM32_BUSY_TIMEOUT_US		100000
#define STM32_ABT_TIMEOUT_US		100000

struct stm32_qspi_flash {
	u32 cr;
	u32 dcr;
	bool initialized;
};

struct stm32_qspi_priv {
	struct stm32_qspi_regs *regs;
	struct stm32_qspi_flash flash[STM32_QSPI_MAX_CHIP];
	void __iomem *mm_base;
	resource_size_t mm_size;
	ulong clock_rate;
	int cs_used;
};

static int _stm32_qspi_wait_for_not_busy(struct stm32_qspi_priv *priv)
{
	u32 sr;
	int ret;

	ret = readl_poll_timeout(&priv->regs->sr, sr,
				 !(sr & STM32_QSPI_SR_BUSY),
				 STM32_BUSY_TIMEOUT_US);
	if (ret)
		pr_err("busy timeout (stat:%#x)\n", sr);

	return ret;
}

static int _stm32_qspi_wait_cmd(struct stm32_qspi_priv *priv,
				const struct spi_mem_op *op)
{
	u32 sr;
	int ret;

	if (!op->data.nbytes)
		return _stm32_qspi_wait_for_not_busy(priv);

	ret = readl_poll_timeout(&priv->regs->sr, sr,
				 sr & STM32_QSPI_SR_TCF,
				 STM32_QSPI_CMD_TIMEOUT_US);
	if (ret) {
		pr_err("cmd timeout (stat:%#x)\n", sr);
	} else if (readl(&priv->regs->sr) & STM32_QSPI_SR_TEF) {
		pr_err("transfer error (stat:%#x)\n", sr);
		ret = -EIO;
	}

	/* clear flags */
	writel(STM32_QSPI_FCR_CTCF | STM32_QSPI_FCR_CTEF, &priv->regs->fcr);

	return ret;
}

static void _stm32_qspi_read_fifo(u8 *val, void __iomem *addr)
{
	*val = readb(addr);
}

static void _stm32_qspi_write_fifo(u8 *val, void __iomem *addr)
{
	writeb(*val, addr);
}

static int _stm32_qspi_poll(struct stm32_qspi_priv *priv,
			    const struct spi_mem_op *op)
{
	void (*fifo)(u8 *val, void __iomem *addr);
	u32 len = op->data.nbytes, sr;
	u8 *buf;
	int ret;

	if (op->data.dir == SPI_MEM_DATA_IN) {
		fifo = _stm32_qspi_read_fifo;
		buf = op->data.buf.in;

	} else {
		fifo = _stm32_qspi_write_fifo;
		buf = (u8 *)op->data.buf.out;
	}

	while (len--) {
		ret = readl_poll_timeout(&priv->regs->sr, sr,
					 sr & STM32_QSPI_SR_FTF,
					 STM32_QSPI_FIFO_TIMEOUT_US);
		if (ret) {
			pr_err("fifo timeout (len:%d stat:%#x)\n", len, sr);
			return ret;
		}

		fifo(buf++, &priv->regs->dr);
	}

	return 0;
}

static int stm32_qspi_mm(struct stm32_qspi_priv *priv,
			 const struct spi_mem_op *op)
{
	memcpy_fromio(op->data.buf.in, priv->mm_base + op->addr.val,
		      op->data.nbytes);

	return 0;
}

static int _stm32_qspi_tx(struct stm32_qspi_priv *priv,
			  const struct spi_mem_op *op,
			  u8 mode)
{
	if (!op->data.nbytes)
		return 0;

	if (mode == STM32_QSPI_CCR_MEM_MAP)
		return stm32_qspi_mm(priv, op);

	return _stm32_qspi_poll(priv, op);
}

static int _stm32_qspi_get_mode(u8 buswidth)
{
	if (buswidth == 4)
		return 3;

	return buswidth;
}

static int stm32_qspi_exec_op(struct spi_slave *slave,
			      const struct spi_mem_op *op)
{
	struct stm32_qspi_priv *priv = dev_get_priv(slave->dev->parent);
	u32 cr, ccr, addr_max;
	u8 mode = STM32_QSPI_CCR_IND_WRITE;
	int timeout, ret;

	debug("%s: cmd:%#x mode:%d.%d.%d.%d addr:%#llx len:%#x\n",
	      __func__, op->cmd.opcode, op->cmd.buswidth, op->addr.buswidth,
	      op->dummy.buswidth, op->data.buswidth,
	      op->addr.val, op->data.nbytes);

	ret = _stm32_qspi_wait_for_not_busy(priv);
	if (ret)
		return ret;

	addr_max = op->addr.val + op->data.nbytes + 1;

	if (op->data.dir == SPI_MEM_DATA_IN && op->data.nbytes) {
		if (addr_max < priv->mm_size && op->addr.buswidth)
			mode = STM32_QSPI_CCR_MEM_MAP;
		else
			mode = STM32_QSPI_CCR_IND_READ;
	}

	if (op->data.nbytes)
		writel(op->data.nbytes - 1, &priv->regs->dlr);

	ccr = (mode << STM32_QSPI_CCR_FMODE_SHIFT);
	ccr |= op->cmd.opcode;
	ccr |= (_stm32_qspi_get_mode(op->cmd.buswidth)
		<< STM32_QSPI_CCR_IMODE_SHIFT);

	if (op->addr.nbytes) {
		ccr |= ((op->addr.nbytes - 1) << STM32_QSPI_CCR_ADSIZE_SHIFT);
		ccr |= (_stm32_qspi_get_mode(op->addr.buswidth)
			<< STM32_QSPI_CCR_ADMODE_SHIFT);
	}

	if (op->dummy.buswidth && op->dummy.nbytes)
		ccr |= (op->dummy.nbytes * 8 / op->dummy.buswidth
			<< STM32_QSPI_CCR_DCYC_SHIFT);

	if (op->data.nbytes)
		ccr |= (_stm32_qspi_get_mode(op->data.buswidth)
			<< STM32_QSPI_CCR_DMODE_SHIFT);

	writel(ccr, &priv->regs->ccr);

	if (op->addr.nbytes && mode != STM32_QSPI_CCR_MEM_MAP)
		writel(op->addr.val, &priv->regs->ar);

	ret = _stm32_qspi_tx(priv, op, mode);
	/*
	 * Abort in:
	 * -error case
	 * -read memory map: prefetching must be stopped if we read the last
	 *  byte of device (device size - fifo size). like device size is not
	 *  knows, the prefetching is always stop.
	 */
	if (ret || mode == STM32_QSPI_CCR_MEM_MAP)
		goto abort;

	/* Wait end of tx in indirect mode */
	ret = _stm32_qspi_wait_cmd(priv, op);
	if (ret)
		goto abort;

	return 0;

abort:
	setbits_le32(&priv->regs->cr, STM32_QSPI_CR_ABORT);

	/* Wait clear of abort bit by hw */
	timeout = readl_poll_timeout(&priv->regs->cr, cr,
				     !(cr & STM32_QSPI_CR_ABORT),
				     STM32_ABT_TIMEOUT_US);

	writel(STM32_QSPI_FCR_CTCF, &priv->regs->fcr);

	if (ret || timeout)
		pr_err("%s ret:%d abort timeout:%d\n", __func__, ret, timeout);

	return ret;
}

static int stm32_qspi_probe(struct udevice *bus)
{
	struct stm32_qspi_priv *priv = dev_get_priv(bus);
	struct resource res;
	struct clk clk;
	struct reset_ctl reset_ctl;
	int ret;

	ret = dev_read_resource_byname(bus, "qspi", &res);
	if (ret) {
		dev_err(bus, "can't get regs base addresses(ret = %d)!\n", ret);
		return ret;
	}

	priv->regs = (struct stm32_qspi_regs *)res.start;

	ret = dev_read_resource_byname(bus, "qspi_mm", &res);
	if (ret) {
		dev_err(bus, "can't get mmap base address(ret = %d)!\n", ret);
		return ret;
	}

	priv->mm_base = (void __iomem *)res.start;

	priv->mm_size = resource_size(&res);
	if (priv->mm_size > STM32_QSPI_MAX_MMAP_SZ)
		return -EINVAL;

	debug("%s: regs=<0x%p> mapped=<0x%p> mapped_size=<0x%lx>\n",
	      __func__, priv->regs, priv->mm_base, priv->mm_size);

	ret = clk_get_by_index(bus, 0, &clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&clk);
	if (ret) {
		dev_err(bus, "failed to enable clock\n");
		return ret;
	}

	priv->clock_rate = clk_get_rate(&clk);
	if (priv->clock_rate < 0) {
		clk_disable(&clk);
		return priv->clock_rate;
	}

	ret = reset_get_by_index(bus, 0, &reset_ctl);
	if (ret) {
		if (ret != -ENOENT) {
			dev_err(bus, "failed to get reset\n");
			clk_disable(&clk);
			return ret;
		}
	} else {
		/* Reset QSPI controller */
		reset_assert(&reset_ctl);
		udelay(2);
		reset_deassert(&reset_ctl);
	}

	priv->cs_used = -1;

	setbits_le32(&priv->regs->cr, STM32_QSPI_CR_SSHIFT);

	/* Set dcr fsize to max address */
	setbits_le32(&priv->regs->dcr,
		     STM32_QSPI_DCR_FSIZE_MASK << STM32_QSPI_DCR_FSIZE_SHIFT);

	return 0;
}

static int stm32_qspi_claim_bus(struct udevice *dev)
{
	struct stm32_qspi_priv *priv = dev_get_priv(dev->parent);
	struct dm_spi_slave_platdata *slave_plat = dev_get_parent_platdata(dev);

	if (slave_plat->cs >= STM32_QSPI_MAX_CHIP)
		return -ENODEV;

	if (priv->cs_used != slave_plat->cs) {
		struct stm32_qspi_flash *flash = &priv->flash[slave_plat->cs];

		priv->cs_used = slave_plat->cs;

		if (flash->initialized) {
			/* Set the configuration: speed + cs */
			writel(flash->cr, &priv->regs->cr);
			writel(flash->dcr, &priv->regs->dcr);
		} else {
			/* Set chip select */
			clrsetbits_le32(&priv->regs->cr, STM32_QSPI_CR_FSEL,
					priv->cs_used ? STM32_QSPI_CR_FSEL : 0);

			/* Save the configuration: speed + cs */
			flash->cr = readl(&priv->regs->cr);
			flash->dcr = readl(&priv->regs->dcr);

			flash->initialized = true;
		}
	}

	setbits_le32(&priv->regs->cr, STM32_QSPI_CR_EN);

	return 0;
}

static int stm32_qspi_release_bus(struct udevice *dev)
{
	struct stm32_qspi_priv *priv = dev_get_priv(dev->parent);

	clrbits_le32(&priv->regs->cr, STM32_QSPI_CR_EN);

	return 0;
}

static int stm32_qspi_set_speed(struct udevice *bus, uint speed)
{
	struct stm32_qspi_priv *priv = dev_get_priv(bus);
	u32 qspi_clk = priv->clock_rate;
	u32 prescaler = 255;
	u32 csht;
	int ret;

	if (speed > 0) {
		prescaler = DIV_ROUND_UP(qspi_clk, speed) - 1;
		if (prescaler > 255)
			prescaler = 255;
		else if (prescaler < 0)
			prescaler = 0;
	}

	csht = DIV_ROUND_UP((5 * qspi_clk) / (prescaler + 1), 100000000);
	csht = (csht - 1) & STM32_QSPI_DCR_CSHT_MASK;

	ret = _stm32_qspi_wait_for_not_busy(priv);
	if (ret)
		return ret;

	clrsetbits_le32(&priv->regs->cr,
			STM32_QSPI_CR_PRESCALER_MASK <<
			STM32_QSPI_CR_PRESCALER_SHIFT,
			prescaler << STM32_QSPI_CR_PRESCALER_SHIFT);

	clrsetbits_le32(&priv->regs->dcr,
			STM32_QSPI_DCR_CSHT_MASK << STM32_QSPI_DCR_CSHT_SHIFT,
			csht << STM32_QSPI_DCR_CSHT_SHIFT);

	debug("%s: regs=%p, speed=%d\n", __func__, priv->regs,
	      (qspi_clk / (prescaler + 1)));

	return 0;
}

static int stm32_qspi_set_mode(struct udevice *bus, uint mode)
{
	struct stm32_qspi_priv *priv = dev_get_priv(bus);
	int ret;

	ret = _stm32_qspi_wait_for_not_busy(priv);
	if (ret)
		return ret;

	if ((mode & SPI_CPHA) && (mode & SPI_CPOL))
		setbits_le32(&priv->regs->dcr, STM32_QSPI_DCR_CKMODE);
	else if (!(mode & SPI_CPHA) && !(mode & SPI_CPOL))
		clrbits_le32(&priv->regs->dcr, STM32_QSPI_DCR_CKMODE);
	else
		return -ENODEV;

	if (mode & SPI_CS_HIGH)
		return -ENODEV;

	debug("%s: regs=%p, mode=%d rx: ", __func__, priv->regs, mode);

	if (mode & SPI_RX_QUAD)
		debug("quad, tx: ");
	else if (mode & SPI_RX_DUAL)
		debug("dual, tx: ");
	else
		debug("single, tx: ");

	if (mode & SPI_TX_QUAD)
		debug("quad\n");
	else if (mode & SPI_TX_DUAL)
		debug("dual\n");
	else
		debug("single\n");

	return 0;
}

static const struct spi_controller_mem_ops stm32_qspi_mem_ops = {
	.exec_op = stm32_qspi_exec_op,
};

static const struct dm_spi_ops stm32_qspi_ops = {
	.claim_bus	= stm32_qspi_claim_bus,
	.release_bus	= stm32_qspi_release_bus,
	.set_speed	= stm32_qspi_set_speed,
	.set_mode	= stm32_qspi_set_mode,
	.mem_ops	= &stm32_qspi_mem_ops,
};

static const struct udevice_id stm32_qspi_ids[] = {
	{ .compatible = "st,stm32-qspi" },
	{ .compatible = "st,stm32f469-qspi" },
	{ }
};

U_BOOT_DRIVER(stm32_qspi) = {
	.name = "stm32_qspi",
	.id = UCLASS_SPI,
	.of_match = stm32_qspi_ids,
	.ops = &stm32_qspi_ops,
	.priv_auto_alloc_size = sizeof(struct stm32_qspi_priv),
	.probe = stm32_qspi_probe,
};

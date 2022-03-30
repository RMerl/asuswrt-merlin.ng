// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018 Xilinx
 *
 * Xilinx ZynqMP Generic Quad-SPI(QSPI) controller driver(master mode only)
 */

#include <common.h>
#include <asm/arch/clk.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <malloc.h>
#include <memalign.h>
#include <spi.h>
#include <ubi_uboot.h>
#include <wait_bit.h>

#define GQSPI_GFIFO_STRT_MODE_MASK	BIT(29)
#define GQSPI_CONFIG_MODE_EN_MASK	(3 << 30)
#define GQSPI_CONFIG_DMA_MODE		(2 << 30)
#define GQSPI_CONFIG_CPHA_MASK		BIT(2)
#define GQSPI_CONFIG_CPOL_MASK		BIT(1)

/*
 * QSPI Interrupt Registers bit Masks
 *
 * All the four interrupt registers (Status/Mask/Enable/Disable) have the same
 * bit definitions.
 */
#define GQSPI_IXR_TXNFULL_MASK		0x00000004 /* QSPI TX FIFO Overflow */
#define GQSPI_IXR_TXFULL_MASK		0x00000008 /* QSPI TX FIFO is full */
#define GQSPI_IXR_RXNEMTY_MASK		0x00000010 /* QSPI RX FIFO Not Empty */
#define GQSPI_IXR_GFEMTY_MASK		0x00000080 /* QSPI Generic FIFO Empty */
#define GQSPI_IXR_ALL_MASK		(GQSPI_IXR_TXNFULL_MASK | \
					 GQSPI_IXR_RXNEMTY_MASK)

/*
 * QSPI Enable Register bit Masks
 *
 * This register is used to enable or disable the QSPI controller
 */
#define GQSPI_ENABLE_ENABLE_MASK	0x00000001 /* QSPI Enable Bit Mask */

#define GQSPI_GFIFO_LOW_BUS		BIT(14)
#define GQSPI_GFIFO_CS_LOWER		BIT(12)
#define GQSPI_GFIFO_UP_BUS		BIT(15)
#define GQSPI_GFIFO_CS_UPPER		BIT(13)
#define GQSPI_SPI_MODE_QSPI		(3 << 10)
#define GQSPI_SPI_MODE_SPI		BIT(10)
#define GQSPI_SPI_MODE_DUAL_SPI		(2 << 10)
#define GQSPI_IMD_DATA_CS_ASSERT	5
#define GQSPI_IMD_DATA_CS_DEASSERT	5
#define GQSPI_GFIFO_TX			BIT(16)
#define GQSPI_GFIFO_RX			BIT(17)
#define GQSPI_GFIFO_STRIPE_MASK		BIT(18)
#define GQSPI_GFIFO_IMD_MASK		0xFF
#define GQSPI_GFIFO_EXP_MASK		BIT(9)
#define GQSPI_GFIFO_DATA_XFR_MASK	BIT(8)
#define GQSPI_STRT_GEN_FIFO		BIT(28)
#define GQSPI_GEN_FIFO_STRT_MOD		BIT(29)
#define GQSPI_GFIFO_WP_HOLD		BIT(19)
#define GQSPI_BAUD_DIV_MASK		(7 << 3)
#define GQSPI_DFLT_BAUD_RATE_DIV	BIT(3)
#define GQSPI_GFIFO_ALL_INT_MASK	0xFBE
#define GQSPI_DMA_DST_I_STS_DONE	BIT(1)
#define GQSPI_DMA_DST_I_STS_MASK	0xFE
#define MODEBITS			0x6

#define GQSPI_GFIFO_SELECT		BIT(0)
#define GQSPI_FIFO_THRESHOLD		1

#define SPI_XFER_ON_BOTH		0
#define SPI_XFER_ON_LOWER		1
#define SPI_XFER_ON_UPPER		2

#define GQSPI_DMA_ALIGN			0x4
#define GQSPI_MAX_BAUD_RATE_VAL		7
#define GQSPI_DFLT_BAUD_RATE_VAL	2

#define GQSPI_TIMEOUT			100000000

#define GQSPI_BAUD_DIV_SHIFT		2
#define GQSPI_LPBK_DLY_ADJ_LPBK_SHIFT	5
#define GQSPI_LPBK_DLY_ADJ_DLY_1	0x2
#define GQSPI_LPBK_DLY_ADJ_DLY_1_SHIFT	3
#define GQSPI_LPBK_DLY_ADJ_DLY_0	0x3
#define GQSPI_USE_DATA_DLY		0x1
#define GQSPI_USE_DATA_DLY_SHIFT	31
#define GQSPI_DATA_DLY_ADJ_VALUE	0x2
#define GQSPI_DATA_DLY_ADJ_SHIFT	28
#define TAP_DLY_BYPASS_LQSPI_RX_VALUE	0x1
#define TAP_DLY_BYPASS_LQSPI_RX_SHIFT	2
#define GQSPI_DATA_DLY_ADJ_OFST		0x000001F8
#define IOU_TAPDLY_BYPASS_OFST		0xFF180390
#define GQSPI_LPBK_DLY_ADJ_LPBK_MASK	0x00000020
#define GQSPI_FREQ_40MHZ		40000000
#define GQSPI_FREQ_100MHZ		100000000
#define GQSPI_FREQ_150MHZ		150000000
#define IOU_TAPDLY_BYPASS_MASK		0x7

#define GQSPI_REG_OFFSET		0x100
#define GQSPI_DMA_REG_OFFSET		0x800

/* QSPI register offsets */
struct zynqmp_qspi_regs {
	u32 confr;	/* 0x00 */
	u32 isr;	/* 0x04 */
	u32 ier;	/* 0x08 */
	u32 idisr;	/* 0x0C */
	u32 imaskr;	/* 0x10 */
	u32 enbr;	/* 0x14 */
	u32 dr;		/* 0x18 */
	u32 txd0r;	/* 0x1C */
	u32 drxr;	/* 0x20 */
	u32 sicr;	/* 0x24 */
	u32 txftr;	/* 0x28 */
	u32 rxftr;	/* 0x2C */
	u32 gpior;	/* 0x30 */
	u32 reserved0;	/* 0x34 */
	u32 lpbkdly;	/* 0x38 */
	u32 reserved1;	/* 0x3C */
	u32 genfifo;	/* 0x40 */
	u32 gqspisel;	/* 0x44 */
	u32 reserved2;	/* 0x48 */
	u32 gqfifoctrl;	/* 0x4C */
	u32 gqfthr;	/* 0x50 */
	u32 gqpollcfg;	/* 0x54 */
	u32 gqpollto;	/* 0x58 */
	u32 gqxfersts;	/* 0x5C */
	u32 gqfifosnap;	/* 0x60 */
	u32 gqrxcpy;	/* 0x64 */
	u32 reserved3[36];	/* 0x68 */
	u32 gqspidlyadj;	/* 0xF8 */
};

struct zynqmp_qspi_dma_regs {
	u32 dmadst;	/* 0x00 */
	u32 dmasize;	/* 0x04 */
	u32 dmasts;	/* 0x08 */
	u32 dmactrl;	/* 0x0C */
	u32 reserved0;	/* 0x10 */
	u32 dmaisr;	/* 0x14 */
	u32 dmaier;	/* 0x18 */
	u32 dmaidr;	/* 0x1C */
	u32 dmaimr;	/* 0x20 */
	u32 dmactrl2;	/* 0x24 */
	u32 dmadstmsb;	/* 0x28 */
};

DECLARE_GLOBAL_DATA_PTR;

struct zynqmp_qspi_platdata {
	struct zynqmp_qspi_regs *regs;
	struct zynqmp_qspi_dma_regs *dma_regs;
	u32 frequency;
	u32 speed_hz;
};

struct zynqmp_qspi_priv {
	struct zynqmp_qspi_regs *regs;
	struct zynqmp_qspi_dma_regs *dma_regs;
	const void *tx_buf;
	void *rx_buf;
	unsigned int len;
	int bytes_to_transfer;
	int bytes_to_receive;
	unsigned int is_inst;
	unsigned int cs_change:1;
};

static int zynqmp_qspi_ofdata_to_platdata(struct udevice *bus)
{
	struct zynqmp_qspi_platdata *plat = bus->platdata;

	debug("%s\n", __func__);

	plat->regs = (struct zynqmp_qspi_regs *)(devfdt_get_addr(bus) +
						 GQSPI_REG_OFFSET);
	plat->dma_regs = (struct zynqmp_qspi_dma_regs *)
			  (devfdt_get_addr(bus) + GQSPI_DMA_REG_OFFSET);

	return 0;
}

static void zynqmp_qspi_init_hw(struct zynqmp_qspi_priv *priv)
{
	u32 config_reg;
	struct zynqmp_qspi_regs *regs = priv->regs;

	writel(GQSPI_GFIFO_SELECT, &regs->gqspisel);
	writel(GQSPI_GFIFO_ALL_INT_MASK, &regs->idisr);
	writel(GQSPI_FIFO_THRESHOLD, &regs->txftr);
	writel(GQSPI_FIFO_THRESHOLD, &regs->rxftr);
	writel(GQSPI_GFIFO_ALL_INT_MASK, &regs->isr);

	config_reg = readl(&regs->confr);
	config_reg &= ~(GQSPI_GFIFO_STRT_MODE_MASK |
			GQSPI_CONFIG_MODE_EN_MASK);
	config_reg |= GQSPI_CONFIG_DMA_MODE |
		      GQSPI_GFIFO_WP_HOLD |
		      GQSPI_DFLT_BAUD_RATE_DIV;
	writel(config_reg, &regs->confr);

	writel(GQSPI_ENABLE_ENABLE_MASK, &regs->enbr);
}

static u32 zynqmp_qspi_bus_select(struct zynqmp_qspi_priv *priv)
{
	u32 gqspi_fifo_reg = 0;

	gqspi_fifo_reg = GQSPI_GFIFO_LOW_BUS |
			 GQSPI_GFIFO_CS_LOWER;

	return gqspi_fifo_reg;
}

static void zynqmp_qspi_fill_gen_fifo(struct zynqmp_qspi_priv *priv,
				      u32 gqspi_fifo_reg)
{
	struct zynqmp_qspi_regs *regs = priv->regs;
	int ret = 0;

	ret = wait_for_bit_le32(&regs->isr, GQSPI_IXR_GFEMTY_MASK, 1,
				GQSPI_TIMEOUT, 1);
	if (ret)
		printf("%s Timeout\n", __func__);

	writel(gqspi_fifo_reg, &regs->genfifo);
}

static void zynqmp_qspi_chipselect(struct zynqmp_qspi_priv *priv, int is_on)
{
	u32 gqspi_fifo_reg = 0;

	if (is_on) {
		gqspi_fifo_reg = zynqmp_qspi_bus_select(priv);
		gqspi_fifo_reg |= GQSPI_SPI_MODE_SPI |
				  GQSPI_IMD_DATA_CS_ASSERT;
	} else {
		gqspi_fifo_reg = GQSPI_GFIFO_LOW_BUS;
		gqspi_fifo_reg |= GQSPI_IMD_DATA_CS_DEASSERT;
	}

	debug("GFIFO_CMD_CS: 0x%x\n", gqspi_fifo_reg);

	zynqmp_qspi_fill_gen_fifo(priv, gqspi_fifo_reg);
}

void zynqmp_qspi_set_tapdelay(struct udevice *bus, u32 baudrateval)
{
	struct zynqmp_qspi_platdata *plat = bus->platdata;
	struct zynqmp_qspi_priv *priv = dev_get_priv(bus);
	struct zynqmp_qspi_regs *regs = priv->regs;
	u32 tapdlybypass = 0, lpbkdlyadj = 0, datadlyadj = 0, clk_rate;
	u32 reqhz = 0;

	clk_rate = plat->frequency;
	reqhz = (clk_rate / (GQSPI_BAUD_DIV_SHIFT << baudrateval));

	debug("%s, req_hz:%d, clk_rate:%d, baudrateval:%d\n",
	      __func__, reqhz, clk_rate, baudrateval);

	if (reqhz < GQSPI_FREQ_40MHZ) {
		zynqmp_mmio_read(IOU_TAPDLY_BYPASS_OFST, &tapdlybypass);
		tapdlybypass |= (TAP_DLY_BYPASS_LQSPI_RX_VALUE <<
				TAP_DLY_BYPASS_LQSPI_RX_SHIFT);
	} else if (reqhz <= GQSPI_FREQ_100MHZ) {
		zynqmp_mmio_read(IOU_TAPDLY_BYPASS_OFST, &tapdlybypass);
		tapdlybypass |= (TAP_DLY_BYPASS_LQSPI_RX_VALUE <<
				TAP_DLY_BYPASS_LQSPI_RX_SHIFT);
		lpbkdlyadj = readl(&regs->lpbkdly);
		lpbkdlyadj |= (GQSPI_LPBK_DLY_ADJ_LPBK_MASK);
		datadlyadj = readl(&regs->gqspidlyadj);
		datadlyadj |= ((GQSPI_USE_DATA_DLY << GQSPI_USE_DATA_DLY_SHIFT)
				| (GQSPI_DATA_DLY_ADJ_VALUE <<
					GQSPI_DATA_DLY_ADJ_SHIFT));
	} else if (reqhz <= GQSPI_FREQ_150MHZ) {
		lpbkdlyadj = readl(&regs->lpbkdly);
		lpbkdlyadj |= ((GQSPI_LPBK_DLY_ADJ_LPBK_MASK) |
				GQSPI_LPBK_DLY_ADJ_DLY_0);
	}

	zynqmp_mmio_write(IOU_TAPDLY_BYPASS_OFST, IOU_TAPDLY_BYPASS_MASK,
			  tapdlybypass);
	writel(lpbkdlyadj, &regs->lpbkdly);
	writel(datadlyadj, &regs->gqspidlyadj);
}

static int zynqmp_qspi_set_speed(struct udevice *bus, uint speed)
{
	struct zynqmp_qspi_platdata *plat = bus->platdata;
	struct zynqmp_qspi_priv *priv = dev_get_priv(bus);
	struct zynqmp_qspi_regs *regs = priv->regs;
	u32 confr;
	u8 baud_rate_val = 0;

	debug("%s\n", __func__);
	if (speed > plat->frequency)
		speed = plat->frequency;

	/* Set the clock frequency */
	confr = readl(&regs->confr);
	if (speed == 0) {
		/* Set baudrate x8, if the freq is 0 */
		baud_rate_val = GQSPI_DFLT_BAUD_RATE_VAL;
	} else if (plat->speed_hz != speed) {
		while ((baud_rate_val < 8) &&
		       ((plat->frequency /
		       (2 << baud_rate_val)) > speed))
			baud_rate_val++;

		if (baud_rate_val > GQSPI_MAX_BAUD_RATE_VAL)
			baud_rate_val = GQSPI_DFLT_BAUD_RATE_VAL;

		plat->speed_hz = plat->frequency / (2 << baud_rate_val);
	}
	confr &= ~GQSPI_BAUD_DIV_MASK;
	confr |= (baud_rate_val << 3);
	writel(confr, &regs->confr);

	zynqmp_qspi_set_tapdelay(bus, baud_rate_val);
	debug("regs=%p, speed=%d\n", priv->regs, plat->speed_hz);

	return 0;
}

static int zynqmp_qspi_probe(struct udevice *bus)
{
	struct zynqmp_qspi_platdata *plat = dev_get_platdata(bus);
	struct zynqmp_qspi_priv *priv = dev_get_priv(bus);
	struct clk clk;
	unsigned long clock;
	int ret;

	debug("%s: bus:%p, priv:%p\n", __func__, bus, priv);

	priv->regs = plat->regs;
	priv->dma_regs = plat->dma_regs;

	ret = clk_get_by_index(bus, 0, &clk);
	if (ret < 0) {
		dev_err(dev, "failed to get clock\n");
		return ret;
	}

	clock = clk_get_rate(&clk);
	if (IS_ERR_VALUE(clock)) {
		dev_err(dev, "failed to get rate\n");
		return clock;
	}
	debug("%s: CLK %ld\n", __func__, clock);

	ret = clk_enable(&clk);
	if (ret && ret != -ENOSYS) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}
	plat->frequency = clock;
	plat->speed_hz = plat->frequency / 2;

	/* init the zynq spi hw */
	zynqmp_qspi_init_hw(priv);

	return 0;
}

static int zynqmp_qspi_set_mode(struct udevice *bus, uint mode)
{
	struct zynqmp_qspi_priv *priv = dev_get_priv(bus);
	struct zynqmp_qspi_regs *regs = priv->regs;
	u32 confr;

	debug("%s\n", __func__);
	/* Set the SPI Clock phase and polarities */
	confr = readl(&regs->confr);
	confr &= ~(GQSPI_CONFIG_CPHA_MASK |
		   GQSPI_CONFIG_CPOL_MASK);

	if (mode & SPI_CPHA)
		confr |= GQSPI_CONFIG_CPHA_MASK;
	if (mode & SPI_CPOL)
		confr |= GQSPI_CONFIG_CPOL_MASK;

	writel(confr, &regs->confr);

	return 0;
}

static int zynqmp_qspi_fill_tx_fifo(struct zynqmp_qspi_priv *priv, u32 size)
{
	u32 data;
	int ret = 0;
	struct zynqmp_qspi_regs *regs = priv->regs;
	u32 *buf = (u32 *)priv->tx_buf;
	u32 len = size;

	debug("TxFIFO: 0x%x, size: 0x%x\n", readl(&regs->isr),
	      size);

	while (size) {
		ret = wait_for_bit_le32(&regs->isr, GQSPI_IXR_TXNFULL_MASK, 1,
					GQSPI_TIMEOUT, 1);
		if (ret) {
			printf("%s: Timeout\n", __func__);
			return ret;
		}

		if (size >= 4) {
			writel(*buf, &regs->txd0r);
			buf++;
			size -= 4;
		} else {
			switch (size) {
			case 1:
				data = *((u8 *)buf);
				buf += 1;
				data |= GENMASK(31, 8);
				break;
			case 2:
				data = *((u16 *)buf);
				buf += 2;
				data |= GENMASK(31, 16);
				break;
			case 3:
				data = *((u16 *)buf);
				buf += 2;
				data |= (*((u8 *)buf) << 16);
				buf += 1;
				data |= GENMASK(31, 24);
				break;
			}
			writel(data, &regs->txd0r);
			size = 0;
		}
	}

	priv->tx_buf += len;
	return 0;
}

static void zynqmp_qspi_genfifo_cmd(struct zynqmp_qspi_priv *priv)
{
	u32 gen_fifo_cmd;
	u32 bytecount = 0;

	while (priv->len) {
		gen_fifo_cmd = zynqmp_qspi_bus_select(priv);
		gen_fifo_cmd |= GQSPI_GFIFO_TX | GQSPI_SPI_MODE_SPI;
		gen_fifo_cmd |= *(u8 *)priv->tx_buf;
		bytecount++;
		priv->len--;
		priv->tx_buf = (u8 *)priv->tx_buf + 1;

		debug("GFIFO_CMD_Cmd = 0x%x\n", gen_fifo_cmd);

		zynqmp_qspi_fill_gen_fifo(priv, gen_fifo_cmd);
	}
}

static u32 zynqmp_qspi_calc_exp(struct zynqmp_qspi_priv *priv,
				u32 *gen_fifo_cmd)
{
	u32 expval = 8;
	u32 len;

	while (1) {
		if (priv->len > 255) {
			if (priv->len & (1 << expval)) {
				*gen_fifo_cmd &= ~GQSPI_GFIFO_IMD_MASK;
				*gen_fifo_cmd |= GQSPI_GFIFO_EXP_MASK;
				*gen_fifo_cmd |= expval;
				priv->len -= (1 << expval);
				return expval;
			}
			expval++;
		} else {
			*gen_fifo_cmd &= ~(GQSPI_GFIFO_IMD_MASK |
					  GQSPI_GFIFO_EXP_MASK);
			*gen_fifo_cmd |= (u8)priv->len;
			len = (u8)priv->len;
			priv->len  = 0;
			return len;
		}
	}
}

static int zynqmp_qspi_genfifo_fill_tx(struct zynqmp_qspi_priv *priv)
{
	u32 gen_fifo_cmd;
	u32 len;
	int ret = 0;

	gen_fifo_cmd = zynqmp_qspi_bus_select(priv);
	gen_fifo_cmd |= GQSPI_GFIFO_TX |
			GQSPI_GFIFO_DATA_XFR_MASK;

	gen_fifo_cmd |= GQSPI_SPI_MODE_SPI;

	while (priv->len) {
		len = zynqmp_qspi_calc_exp(priv, &gen_fifo_cmd);
		zynqmp_qspi_fill_gen_fifo(priv, gen_fifo_cmd);

		debug("GFIFO_CMD_TX:0x%x\n", gen_fifo_cmd);

		if (gen_fifo_cmd & GQSPI_GFIFO_EXP_MASK)
			ret = zynqmp_qspi_fill_tx_fifo(priv,
						       1 << len);
		else
			ret = zynqmp_qspi_fill_tx_fifo(priv,
						       len);

		if (ret)
			return ret;
	}
	return ret;
}

static int zynqmp_qspi_start_dma(struct zynqmp_qspi_priv *priv,
				 u32 gen_fifo_cmd, u32 *buf)
{
	u32 addr;
	u32 size, len;
	u32 actuallen = priv->len;
	int ret = 0;
	struct zynqmp_qspi_dma_regs *dma_regs = priv->dma_regs;

	writel((unsigned long)buf, &dma_regs->dmadst);
	writel(roundup(priv->len, ARCH_DMA_MINALIGN), &dma_regs->dmasize);
	writel(GQSPI_DMA_DST_I_STS_MASK, &dma_regs->dmaier);
	addr = (unsigned long)buf;
	size = roundup(priv->len, ARCH_DMA_MINALIGN);
	flush_dcache_range(addr, addr + size);

	while (priv->len) {
		len = zynqmp_qspi_calc_exp(priv, &gen_fifo_cmd);
		if (!(gen_fifo_cmd & GQSPI_GFIFO_EXP_MASK) &&
		    (len % ARCH_DMA_MINALIGN)) {
			gen_fifo_cmd &= ~GENMASK(7, 0);
			gen_fifo_cmd |= roundup(len, ARCH_DMA_MINALIGN);
		}
		zynqmp_qspi_fill_gen_fifo(priv, gen_fifo_cmd);

		debug("GFIFO_CMD_RX:0x%x\n", gen_fifo_cmd);
	}

	ret = wait_for_bit_le32(&dma_regs->dmaisr, GQSPI_DMA_DST_I_STS_DONE,
				1, GQSPI_TIMEOUT, 1);
	if (ret) {
		printf("DMA Timeout:0x%x\n", readl(&dma_regs->dmaisr));
		return -ETIMEDOUT;
	}

	writel(GQSPI_DMA_DST_I_STS_DONE, &dma_regs->dmaisr);

	debug("buf:0x%lx, rxbuf:0x%lx, *buf:0x%x len: 0x%x\n",
	      (unsigned long)buf, (unsigned long)priv->rx_buf, *buf,
	      actuallen);

	if (buf != priv->rx_buf)
		memcpy(priv->rx_buf, buf, actuallen);

	return 0;
}

static int zynqmp_qspi_genfifo_fill_rx(struct zynqmp_qspi_priv *priv)
{
	u32 gen_fifo_cmd;
	u32 *buf;
	u32 actuallen = priv->len;

	gen_fifo_cmd = zynqmp_qspi_bus_select(priv);
	gen_fifo_cmd |= GQSPI_GFIFO_RX |
			GQSPI_GFIFO_DATA_XFR_MASK;

	gen_fifo_cmd |= GQSPI_SPI_MODE_SPI;

	/*
	 * Check if receive buffer is aligned to 4 byte and length
	 * is multiples of four byte as we are using dma to receive.
	 */
	if (!((unsigned long)priv->rx_buf & (GQSPI_DMA_ALIGN - 1)) &&
	    !(actuallen % GQSPI_DMA_ALIGN)) {
		buf = (u32 *)priv->rx_buf;
		return zynqmp_qspi_start_dma(priv, gen_fifo_cmd, buf);
	}

	ALLOC_CACHE_ALIGN_BUFFER(u8, tmp, roundup(priv->len,
						  GQSPI_DMA_ALIGN));
	buf = (u32 *)tmp;
	return zynqmp_qspi_start_dma(priv, gen_fifo_cmd, buf);
}

static int zynqmp_qspi_start_transfer(struct zynqmp_qspi_priv *priv)
{
	int ret = 0;

	if (priv->is_inst) {
		if (priv->tx_buf)
			zynqmp_qspi_genfifo_cmd(priv);
		else
			return -EINVAL;
	} else {
		if (priv->tx_buf)
			ret = zynqmp_qspi_genfifo_fill_tx(priv);
		else if (priv->rx_buf)
			ret = zynqmp_qspi_genfifo_fill_rx(priv);
		else
			return -EINVAL;
	}
	return ret;
}

static int zynqmp_qspi_transfer(struct zynqmp_qspi_priv *priv)
{
	static unsigned int cs_change = 1;
	int status = 0;

	debug("%s\n", __func__);

	while (1) {
		/* Select the chip if required */
		if (cs_change)
			zynqmp_qspi_chipselect(priv, 1);

		cs_change = priv->cs_change;

		if (!priv->tx_buf && !priv->rx_buf && priv->len) {
			status = -EINVAL;
			break;
		}

		/* Request the transfer */
		if (priv->len) {
			status = zynqmp_qspi_start_transfer(priv);
			priv->is_inst = 0;
			if (status < 0)
				break;
		}

		if (cs_change)
			/* Deselect the chip */
			zynqmp_qspi_chipselect(priv, 0);
		break;
	}

	return status;
}

static int zynqmp_qspi_claim_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct zynqmp_qspi_priv *priv = dev_get_priv(bus);
	struct zynqmp_qspi_regs *regs = priv->regs;

	writel(GQSPI_ENABLE_ENABLE_MASK, &regs->enbr);

	return 0;
}

static int zynqmp_qspi_release_bus(struct udevice *dev)
{
	struct udevice *bus = dev->parent;
	struct zynqmp_qspi_priv *priv = dev_get_priv(bus);
	struct zynqmp_qspi_regs *regs = priv->regs;

	writel(~GQSPI_ENABLE_ENABLE_MASK, &regs->enbr);

	return 0;
}

int zynqmp_qspi_xfer(struct udevice *dev, unsigned int bitlen, const void *dout,
		     void *din, unsigned long flags)
{
	struct udevice *bus = dev->parent;
	struct zynqmp_qspi_priv *priv = dev_get_priv(bus);

	debug("%s: priv: 0x%08lx bitlen: %d dout: 0x%08lx ", __func__,
	      (unsigned long)priv, bitlen, (unsigned long)dout);
	debug("din: 0x%08lx flags: 0x%lx\n", (unsigned long)din, flags);

	priv->tx_buf = dout;
	priv->rx_buf = din;
	priv->len = bitlen / 8;

	/*
	 * Assume that the beginning of a transfer with bits to
	 * transmit must contain a device command.
	 */
	if (dout && flags & SPI_XFER_BEGIN)
		priv->is_inst = 1;
	else
		priv->is_inst = 0;

	if (flags & SPI_XFER_END)
		priv->cs_change = 1;
	else
		priv->cs_change = 0;

	zynqmp_qspi_transfer(priv);

	return 0;
}

static const struct dm_spi_ops zynqmp_qspi_ops = {
	.claim_bus      = zynqmp_qspi_claim_bus,
	.release_bus    = zynqmp_qspi_release_bus,
	.xfer           = zynqmp_qspi_xfer,
	.set_speed      = zynqmp_qspi_set_speed,
	.set_mode       = zynqmp_qspi_set_mode,
};

static const struct udevice_id zynqmp_qspi_ids[] = {
	{ .compatible = "xlnx,zynqmp-qspi-1.0" },
	{ .compatible = "xlnx,versal-qspi-1.0" },
	{ }
};

U_BOOT_DRIVER(zynqmp_qspi) = {
	.name   = "zynqmp_qspi",
	.id     = UCLASS_SPI,
	.of_match = zynqmp_qspi_ids,
	.ops    = &zynqmp_qspi_ops,
	.ofdata_to_platdata = zynqmp_qspi_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct zynqmp_qspi_platdata),
	.priv_auto_alloc_size = sizeof(struct zynqmp_qspi_priv),
	.probe  = zynqmp_qspi_probe,
};

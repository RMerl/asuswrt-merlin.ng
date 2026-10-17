// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018 Xilinx
 *
 * Xilinx ZynqMP Generic Quad-SPI(QSPI) controller driver(master mode only)
 */

#define LOG_CATEGORY UCLASS_SPI

#include <common.h>
#include <cpu_func.h>
#include <log.h>
#include <asm/arch/sys_proto.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <clk.h>
#include <dm.h>
#include <malloc.h>
#include <memalign.h>
#include <spi.h>
#include <spi-mem.h>
#include <ubi_uboot.h>
#include <wait_bit.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/err.h>
#include <linux/sizes.h>
#include <zynqmp_firmware.h>

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
#define GQSPI_IXR_TXFIFOEMPTY_MASK	0x00000100 /* QSPI TX FIFO is Empty */
#define GQSPI_IXR_RXNEMTY_MASK		0x00000010 /* QSPI RX FIFO Not Empty */
#define GQSPI_IXR_GFEMTY_MASK		0x00000080 /* QSPI Generic FIFO Empty */
#define GQSPI_IXR_GFNFULL_MASK		0x00000200 /* QSPI GENFIFO not full */
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
#define GQSPI_GENFIFO_THRESHOLD		31

#define SPI_XFER_ON_BOTH		0
#define SPI_XFER_ON_LOWER		1
#define SPI_XFER_ON_UPPER		2

#define GQSPI_DMA_ALIGN			0x4
#define GQSPI_MAX_BAUD_RATE_VAL		7
#define GQSPI_DFLT_BAUD_RATE_VAL	2

#define GQSPI_TIMEOUT			100000000

#define GQSPI_BAUD_DIV_SHIFT		2
#define GQSPI_LPBK_DLY_ADJ_LPBK_SHIFT	5
#define GQSPI_LPBK_DLY_ADJ_DLY_1	0x1
#define GQSPI_LPBK_DLY_ADJ_DLY_1_SHIFT	3
#define GQSPI_LPBK_DLY_ADJ_DLY_0	0x3
#define GQSPI_USE_DATA_DLY		0x1
#define GQSPI_USE_DATA_DLY_SHIFT	31
#define GQSPI_DATA_DLY_ADJ_VALUE	0x2
#define GQSPI_DATA_DLY_ADJ_SHIFT	28
#define TAP_DLY_BYPASS_LQSPI_RX_VALUE	0x1
#define TAP_DLY_BYPASS_LQSPI_RX_SHIFT	2
#define GQSPI_DATA_DLY_ADJ_OFST		0x000001F8
#define IOU_TAPDLY_BYPASS_OFST !(IS_ENABLED(CONFIG_ARCH_VERSAL) || \
				 IS_ENABLED(CONFIG_ARCH_VERSAL_NET)) ? \
				0xFF180390 : 0xF103003C
#define GQSPI_LPBK_DLY_ADJ_LPBK_MASK	0x00000020
#define GQSPI_FREQ_37_5MHZ		37500000
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

struct zynqmp_qspi_plat {
	struct zynqmp_qspi_regs *regs;
	struct zynqmp_qspi_dma_regs *dma_regs;
	u32 frequency;
	u32 speed_hz;
	unsigned int io_mode;
};

struct zynqmp_qspi_priv {
	struct zynqmp_qspi_regs *regs;
	struct zynqmp_qspi_dma_regs *dma_regs;
	const void *tx_buf;
	void *rx_buf;
	unsigned int len;
	unsigned int io_mode;
	int bytes_to_transfer;
	int bytes_to_receive;
	const struct spi_mem_op *op;
};

__weak int zynqmp_mmio_write(const u32 address, const u32 mask, const u32 value)
{
	return 0;
}

static int zynqmp_qspi_of_to_plat(struct udevice *bus)
{
	struct zynqmp_qspi_plat *plat = dev_get_plat(bus);

	plat->regs = (struct zynqmp_qspi_regs *)(dev_read_addr(bus) +
						 GQSPI_REG_OFFSET);
	plat->dma_regs = (struct zynqmp_qspi_dma_regs *)
			  (dev_read_addr(bus) + GQSPI_DMA_REG_OFFSET);

	plat->io_mode = dev_read_bool(bus, "has-io-mode");

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
	writel(GQSPI_GENFIFO_THRESHOLD, &regs->gqfthr);
	writel(GQSPI_GFIFO_ALL_INT_MASK, &regs->isr);
	writel(~GQSPI_ENABLE_ENABLE_MASK, &regs->enbr);

	config_reg = readl(&regs->confr);
	config_reg &= ~(GQSPI_GFIFO_STRT_MODE_MASK |
			GQSPI_CONFIG_MODE_EN_MASK);
	config_reg |= GQSPI_GFIFO_WP_HOLD | GQSPI_DFLT_BAUD_RATE_DIV;
	config_reg |= GQSPI_GFIFO_STRT_MODE_MASK;
	if (!priv->io_mode)
		config_reg |= GQSPI_CONFIG_DMA_MODE;

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

static u32 zynqmp_qspi_genfifo_mode(u8 buswidth)
{
	switch (buswidth) {
	case 1:
		return GQSPI_SPI_MODE_SPI;
	case 2:
		return GQSPI_SPI_MODE_DUAL_SPI;
	case 4:
		return GQSPI_SPI_MODE_QSPI;
	default:
		log_warning("Unsupported bus width %u\n", buswidth);
		return GQSPI_SPI_MODE_SPI;
	}
}

static void zynqmp_qspi_fill_gen_fifo(struct zynqmp_qspi_priv *priv,
				      u32 gqspi_fifo_reg)
{
	struct zynqmp_qspi_regs *regs = priv->regs;
	u32 config_reg, ier;
	int ret = 0;

	log_content("%s, GFIFO_CMD: 0x%X\n", __func__, gqspi_fifo_reg);

	writel(gqspi_fifo_reg, &regs->genfifo);

	config_reg = readl(&regs->confr);
	/* Manual start if needed */
	config_reg |= GQSPI_STRT_GEN_FIFO;
	writel(config_reg, &regs->confr);

	/* Enable interrupts */
	ier = readl(&regs->ier);
	ier |= GQSPI_IXR_GFEMTY_MASK;
	writel(ier, &regs->ier);

	/* Wait until the gen fifo is empty to write the new command */
	ret = wait_for_bit_le32(&regs->isr, GQSPI_IXR_GFEMTY_MASK, 1,
				GQSPI_TIMEOUT, 1);
	if (ret)
		log_warning("%s, Timeout\n", __func__);

}

static void zynqmp_qspi_chipselect(struct zynqmp_qspi_priv *priv, int is_on)
{
	u32 gqspi_fifo_reg = 0;

	log_debug("%s, assert: %d\r\n", __func__, is_on);

	if (is_on) {
		gqspi_fifo_reg = zynqmp_qspi_bus_select(priv);
		gqspi_fifo_reg |= GQSPI_SPI_MODE_SPI |
				  GQSPI_IMD_DATA_CS_ASSERT;
	} else {
		gqspi_fifo_reg = GQSPI_GFIFO_LOW_BUS;
		gqspi_fifo_reg |= GQSPI_IMD_DATA_CS_DEASSERT;
	}

	zynqmp_qspi_fill_gen_fifo(priv, gqspi_fifo_reg);
}

static void zynqmp_qspi_set_tapdelay(struct udevice *bus, u32 baudrateval)
{
	struct zynqmp_qspi_plat *plat = dev_get_plat(bus);
	struct zynqmp_qspi_priv *priv = dev_get_priv(bus);
	struct zynqmp_qspi_regs *regs = priv->regs;
	u32 tapdlybypass = 0, lpbkdlyadj = 0, datadlyadj = 0, clk_rate;
	u32 reqhz = 0;

	clk_rate = plat->frequency;
	reqhz = (clk_rate / (GQSPI_BAUD_DIV_SHIFT << baudrateval));

	log_debug("%s, clk_rate:%d, baudrateval:%d, bus_clk: %d\n",
		  __func__, clk_rate, baudrateval, reqhz);

	if (!(IS_ENABLED(CONFIG_ARCH_VERSAL) ||
	      IS_ENABLED(CONFIG_ARCH_VERSAL_NET))) {
		if (reqhz <= GQSPI_FREQ_40MHZ) {
			tapdlybypass = TAP_DLY_BYPASS_LQSPI_RX_VALUE <<
					TAP_DLY_BYPASS_LQSPI_RX_SHIFT;
		} else if (reqhz <= GQSPI_FREQ_100MHZ) {
			tapdlybypass = TAP_DLY_BYPASS_LQSPI_RX_VALUE <<
					TAP_DLY_BYPASS_LQSPI_RX_SHIFT;
			lpbkdlyadj = GQSPI_LPBK_DLY_ADJ_LPBK_MASK;
			datadlyadj = (GQSPI_USE_DATA_DLY <<
				      GQSPI_USE_DATA_DLY_SHIFT) |
				       (GQSPI_DATA_DLY_ADJ_VALUE <<
					GQSPI_DATA_DLY_ADJ_SHIFT);
		} else if (reqhz <= GQSPI_FREQ_150MHZ) {
			lpbkdlyadj = GQSPI_LPBK_DLY_ADJ_LPBK_MASK |
				      GQSPI_LPBK_DLY_ADJ_DLY_0;
		}
		zynqmp_mmio_write(IOU_TAPDLY_BYPASS_OFST,
				  IOU_TAPDLY_BYPASS_MASK, tapdlybypass);
	} else {
		if (reqhz <= GQSPI_FREQ_37_5MHZ) {
			tapdlybypass = TAP_DLY_BYPASS_LQSPI_RX_VALUE <<
					TAP_DLY_BYPASS_LQSPI_RX_SHIFT;
		} else if (reqhz <= GQSPI_FREQ_100MHZ) {
			tapdlybypass = TAP_DLY_BYPASS_LQSPI_RX_VALUE <<
					TAP_DLY_BYPASS_LQSPI_RX_SHIFT;
			lpbkdlyadj = GQSPI_LPBK_DLY_ADJ_LPBK_MASK;
			datadlyadj = GQSPI_USE_DATA_DLY <<
				      GQSPI_USE_DATA_DLY_SHIFT;
		} else if (reqhz <= GQSPI_FREQ_150MHZ) {
			lpbkdlyadj = GQSPI_LPBK_DLY_ADJ_LPBK_MASK |
				      (GQSPI_LPBK_DLY_ADJ_DLY_1 <<
				       GQSPI_LPBK_DLY_ADJ_DLY_1_SHIFT);
		}
		writel(tapdlybypass, IOU_TAPDLY_BYPASS_OFST);
	}
	writel(lpbkdlyadj, &regs->lpbkdly);
	writel(datadlyadj, &regs->gqspidlyadj);
}

static int zynqmp_qspi_set_speed(struct udevice *bus, uint speed)
{
	struct zynqmp_qspi_plat *plat = dev_get_plat(bus);
	struct zynqmp_qspi_priv *priv = dev_get_priv(bus);
	struct zynqmp_qspi_regs *regs = priv->regs;
	u32 confr;
	u8 baud_rate_val = 0;

	log_debug("%s, Speed: %d, Max: %d\n", __func__, speed, plat->frequency);

	if (speed > plat->frequency)
		speed = plat->frequency;

	if (plat->speed_hz != speed) {
		/* Set the clock frequency */
		/* If speed == 0, default to lowest speed */
		while ((baud_rate_val < 8) &&
		       ((plat->frequency /
		       (2 << baud_rate_val)) > speed))
			baud_rate_val++;

		if (baud_rate_val > GQSPI_MAX_BAUD_RATE_VAL)
			baud_rate_val = GQSPI_DFLT_BAUD_RATE_VAL;

		plat->speed_hz = plat->frequency / (2 << baud_rate_val);

		confr = readl(&regs->confr);
		confr &= ~GQSPI_BAUD_DIV_MASK;
		confr |= (baud_rate_val << 3);
		writel(confr, &regs->confr);

		zynqmp_qspi_set_tapdelay(bus, baud_rate_val);
	}

	return 0;
}

static int zynqmp_qspi_probe(struct udevice *bus)
{
	struct zynqmp_qspi_plat *plat = dev_get_plat(bus);
	struct zynqmp_qspi_priv *priv = dev_get_priv(bus);
	struct clk clk;
	unsigned long clock;
	int ret;

	priv->regs = plat->regs;
	priv->dma_regs = plat->dma_regs;
	priv->io_mode = plat->io_mode;

	ret = clk_get_by_index(bus, 0, &clk);
	if (ret < 0) {
		dev_err(bus, "failed to get clock\n");
		return ret;
	}

	clock = clk_get_rate(&clk);
	if (IS_ERR_VALUE(clock)) {
		dev_err(bus, "failed to get rate\n");
		return clock;
	}

	ret = clk_enable(&clk);
	if (ret) {
		dev_err(bus, "failed to enable clock\n");
		return ret;
	}
	plat->frequency = clock;
	plat->speed_hz = plat->frequency / 2;

	/* init the zynq spi hw */
	zynqmp_qspi_init_hw(priv);

	log_debug("%s, Rerence clock frequency: %ld\n", __func__, clock);

	return 0;
}

static int zynqmp_qspi_set_mode(struct udevice *bus, uint mode)
{
	struct zynqmp_qspi_priv *priv = dev_get_priv(bus);
	struct zynqmp_qspi_regs *regs = priv->regs;
	u32 confr;

	log_debug("%s, 0x%X\n", __func__, mode);

	/* Set the SPI Clock phase and polarities */
	confr = readl(&regs->confr);
	confr &= ~(GQSPI_CONFIG_CPHA_MASK | GQSPI_CONFIG_CPOL_MASK);

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

	while (size) {
		ret = wait_for_bit_le32(&regs->isr, GQSPI_IXR_TXNFULL_MASK, 1,
					GQSPI_TIMEOUT, 1);
		if (ret)
			return log_msg_ret("Timeout\n", ret);

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
				data = *buf;
				buf += 3;
				data |= GENMASK(31, 24);
				break;
			}
			writel(data, &regs->txd0r);
			size = 0;
		}
	}

	ret = wait_for_bit_le32(&regs->isr, GQSPI_IXR_TXFIFOEMPTY_MASK, 1,
				GQSPI_TIMEOUT, 1);
	if (ret)
		return log_msg_ret("Timeout\n", ret);

	priv->tx_buf += len;
	return 0;
}

static void zynqmp_qspi_genfifo_cmd(struct zynqmp_qspi_priv *priv)
{
	const struct spi_mem_op *op = priv->op;
	u32 gen_fifo_cmd;
	u8 i, dummy_cycles, addr;

	log_debug("%s, opcode: 0x%0X, addr.nbytes: %d, dummy.mbytes: %d\r\n",
		  __func__, op->cmd.opcode, op->addr.nbytes, op->dummy.nbytes);

	/* Send opcode */
	gen_fifo_cmd = zynqmp_qspi_bus_select(priv);
	gen_fifo_cmd |= zynqmp_qspi_genfifo_mode(op->cmd.buswidth);
	gen_fifo_cmd |= GQSPI_GFIFO_TX;
	gen_fifo_cmd |= op->cmd.opcode;
	zynqmp_qspi_fill_gen_fifo(priv, gen_fifo_cmd);

	/* Send address */
	for (i = 0; i < op->addr.nbytes; i++) {
		addr = op->addr.val >> (8 * (op->addr.nbytes - i - 1));

		gen_fifo_cmd = zynqmp_qspi_bus_select(priv);
		gen_fifo_cmd |= zynqmp_qspi_genfifo_mode(op->addr.buswidth);
		gen_fifo_cmd |= GQSPI_GFIFO_TX;
		gen_fifo_cmd |= addr;

		zynqmp_qspi_fill_gen_fifo(priv, gen_fifo_cmd);
	}

	/* Send dummy */
	if (op->dummy.nbytes) {
		dummy_cycles = op->dummy.nbytes * 8 / op->dummy.buswidth;

		gen_fifo_cmd = zynqmp_qspi_bus_select(priv);
		gen_fifo_cmd |= zynqmp_qspi_genfifo_mode(op->dummy.buswidth);
		gen_fifo_cmd &= ~(GQSPI_GFIFO_TX | GQSPI_GFIFO_RX);
		gen_fifo_cmd |= GQSPI_GFIFO_DATA_XFR_MASK;
		gen_fifo_cmd |= dummy_cycles;
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

	log_debug("%s, length: %d\r\n", __func__, priv->len);

	gen_fifo_cmd = zynqmp_qspi_bus_select(priv);
	gen_fifo_cmd |= zynqmp_qspi_genfifo_mode(priv->op->data.buswidth);
	gen_fifo_cmd |= GQSPI_GFIFO_TX | GQSPI_GFIFO_DATA_XFR_MASK;

	while (priv->len) {
		len = zynqmp_qspi_calc_exp(priv, &gen_fifo_cmd);
		zynqmp_qspi_fill_gen_fifo(priv, gen_fifo_cmd);

		if (gen_fifo_cmd & GQSPI_GFIFO_EXP_MASK)
			ret = zynqmp_qspi_fill_tx_fifo(priv, 1 << len);
		else
			ret = zynqmp_qspi_fill_tx_fifo(priv, len);

		if (ret)
			return ret;
	}
	return ret;
}

static int zynqmp_qspi_start_io(struct zynqmp_qspi_priv *priv,
				u32 gen_fifo_cmd, u32 *buf)
{
	u32 len;
	u32 config_reg, ier, isr;
	u32 timeout = GQSPI_TIMEOUT;
	struct zynqmp_qspi_regs *regs = priv->regs;
	u32 last_bits;
	u32 *traverse = buf;

	while (priv->len) {
		len = zynqmp_qspi_calc_exp(priv, &gen_fifo_cmd);
		/* If exponent bit is set, reset immediate to be 2^len */
		if (gen_fifo_cmd & GQSPI_GFIFO_EXP_MASK)
			priv->bytes_to_receive = (1 << len);
		else
			priv->bytes_to_receive = len;
		zynqmp_qspi_fill_gen_fifo(priv, gen_fifo_cmd);

		/* Manual start */
		config_reg = readl(&regs->confr);
		config_reg |= GQSPI_STRT_GEN_FIFO;
		writel(config_reg, &regs->confr);
		/* Enable RX interrupts for IO mode */
		ier = readl(&regs->ier);
		ier |= GQSPI_IXR_ALL_MASK;
		writel(ier, &regs->ier);
		while (priv->bytes_to_receive && timeout) {
			isr = readl(&regs->isr);
			if (isr & GQSPI_IXR_RXNEMTY_MASK) {
				if (priv->bytes_to_receive >= 4) {
					*traverse = readl(&regs->drxr);
					traverse++;
					priv->bytes_to_receive -= 4;
				} else {
					last_bits = readl(&regs->drxr);
					memcpy(traverse, &last_bits,
					       priv->bytes_to_receive);
					priv->bytes_to_receive = 0;
				}
				timeout = GQSPI_TIMEOUT;
			} else {
				udelay(1);
				timeout--;
			}
		}

		if (!timeout)
			return log_msg_retz("Timeout\n", timeout);
	}

	return 0;
}

static int zynqmp_qspi_start_dma(struct zynqmp_qspi_priv *priv,
				 u32 gen_fifo_cmd, u32 *buf)
{
	unsigned long addr;
	u32 size;
	u32 actuallen = priv->len;
	u32 totallen = priv->len;
	int ret = 0;
	struct zynqmp_qspi_dma_regs *dma_regs = priv->dma_regs;

	while (totallen) {
		if (totallen >= SZ_512M)
			priv->len = SZ_256M;
		else
			priv->len = totallen;

		totallen -= priv->len; /* Save remaining bytes length to read */
		actuallen = priv->len; /* Actual number of bytes reading */

		writel(lower_32_bits((unsigned long)buf), &dma_regs->dmadst);
		writel(upper_32_bits((unsigned long)buf) & GENMASK(11, 0),
							&dma_regs->dmadstmsb);
		writel(roundup(priv->len, GQSPI_DMA_ALIGN), &dma_regs->dmasize);
		writel(GQSPI_DMA_DST_I_STS_MASK, &dma_regs->dmaier);
		addr = (unsigned long)buf;
		size = roundup(priv->len, GQSPI_DMA_ALIGN);
		invalidate_dcache_range(addr, addr + size);

		while (priv->len) {
			zynqmp_qspi_calc_exp(priv, &gen_fifo_cmd);
			zynqmp_qspi_fill_gen_fifo(priv, gen_fifo_cmd);
		}

		ret = wait_for_bit_le32(&dma_regs->dmaisr,
					GQSPI_DMA_DST_I_STS_DONE, 1,
					GQSPI_TIMEOUT, 1);
		if (ret)
			return log_msg_ret("Timeout:\n", ret);

		invalidate_dcache_range(addr, addr + size);

		writel(GQSPI_DMA_DST_I_STS_DONE, &dma_regs->dmaisr);

		if (buf != priv->rx_buf)
			memcpy(priv->rx_buf, buf, actuallen);

		buf = (u32 *)((u8 *)buf + actuallen);
		priv->rx_buf = (u8 *)priv->rx_buf + actuallen;
	}

	return 0;
}

static int zynqmp_qspi_genfifo_fill_rx(struct zynqmp_qspi_priv *priv)
{
	u32 gen_fifo_cmd;
	u32 *buf;
	u32 actuallen = priv->len;

	log_debug("%s, length: %d\r\n", __func__, priv->len);

	gen_fifo_cmd = zynqmp_qspi_bus_select(priv);
	gen_fifo_cmd |= zynqmp_qspi_genfifo_mode(priv->op->data.buswidth);
	gen_fifo_cmd |= GQSPI_GFIFO_RX | GQSPI_GFIFO_DATA_XFR_MASK;

	/*
	 * Check if receive buffer is aligned to 4 byte and length
	 * is multiples of four byte as we are using dma to receive.
	 */
	if ((!((unsigned long)priv->rx_buf & (GQSPI_DMA_ALIGN - 1)) &&
	     !(actuallen % GQSPI_DMA_ALIGN)) || priv->io_mode) {
		buf = (u32 *)priv->rx_buf;
		if (priv->io_mode)
			return zynqmp_qspi_start_io(priv, gen_fifo_cmd, buf);
		else
			return zynqmp_qspi_start_dma(priv, gen_fifo_cmd, buf);
	}

	ALLOC_CACHE_ALIGN_BUFFER(u8, tmp, roundup(priv->len, GQSPI_DMA_ALIGN));
	buf = (u32 *)tmp;
	return zynqmp_qspi_start_dma(priv, gen_fifo_cmd, buf);
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

static int zynqmp_qspi_exec_op(struct spi_slave *slave,
			       const struct spi_mem_op *op)
{
	struct zynqmp_qspi_priv *priv = dev_get_priv(slave->dev->parent);
	int ret = 0;

	priv->op = op;
	priv->tx_buf = op->data.buf.out;
	priv->rx_buf = op->data.buf.in;
	priv->len = op->data.nbytes;

	zynqmp_qspi_chipselect(priv, 1);

	/* Send opcode, addr, dummy */
	zynqmp_qspi_genfifo_cmd(priv);

	/* Request the transfer */
	if (op->data.dir == SPI_MEM_DATA_IN)
		ret = zynqmp_qspi_genfifo_fill_rx(priv);
	else if (op->data.dir == SPI_MEM_DATA_OUT)
		ret = zynqmp_qspi_genfifo_fill_tx(priv);

	zynqmp_qspi_chipselect(priv, 0);

	return ret;
}

static const struct spi_controller_mem_ops zynqmp_qspi_mem_ops = {
	.exec_op = zynqmp_qspi_exec_op,
};

static const struct dm_spi_ops zynqmp_qspi_ops = {
	.claim_bus      = zynqmp_qspi_claim_bus,
	.release_bus    = zynqmp_qspi_release_bus,
	.set_speed      = zynqmp_qspi_set_speed,
	.set_mode       = zynqmp_qspi_set_mode,
	.mem_ops        = &zynqmp_qspi_mem_ops,
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
	.of_to_plat = zynqmp_qspi_of_to_plat,
	.plat_auto	= sizeof(struct zynqmp_qspi_plat),
	.priv_auto	= sizeof(struct zynqmp_qspi_priv),
	.probe  = zynqmp_qspi_probe,
};

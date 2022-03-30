// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <fsl_lpuart.h>
#include <watchdog.h>
#include <asm/io.h>
#include <serial.h>
#include <linux/compiler.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>

#define US1_TDRE	(1 << 7)
#define US1_RDRF	(1 << 5)
#define US1_OR		(1 << 3)
#define UC2_TE		(1 << 3)
#define UC2_RE		(1 << 2)
#define CFIFO_TXFLUSH	(1 << 7)
#define CFIFO_RXFLUSH	(1 << 6)
#define SFIFO_RXOF	(1 << 2)
#define SFIFO_RXUF	(1 << 0)

#define STAT_LBKDIF	(1 << 31)
#define STAT_RXEDGIF	(1 << 30)
#define STAT_TDRE	(1 << 23)
#define STAT_RDRF	(1 << 21)
#define STAT_IDLE	(1 << 20)
#define STAT_OR		(1 << 19)
#define STAT_NF		(1 << 18)
#define STAT_FE		(1 << 17)
#define STAT_PF		(1 << 16)
#define STAT_MA1F	(1 << 15)
#define STAT_MA2F	(1 << 14)
#define STAT_FLAGS	(STAT_LBKDIF | STAT_RXEDGIF | STAT_IDLE | STAT_OR | \
			 STAT_NF | STAT_FE | STAT_PF | STAT_MA1F | STAT_MA2F)

#define CTRL_TE		(1 << 19)
#define CTRL_RE		(1 << 18)

#define FIFO_RXFLUSH		BIT(14)
#define FIFO_TXFLUSH		BIT(15)
#define FIFO_TXSIZE_MASK	0x70
#define FIFO_TXSIZE_OFF	4
#define FIFO_RXSIZE_MASK	0x7
#define FIFO_RXSIZE_OFF	0
#define FIFO_TXFE		0x80
#ifdef CONFIG_ARCH_IMX8
#define FIFO_RXFE		0x08
#else
#define FIFO_RXFE		0x40
#endif

#define WATER_TXWATER_OFF	0
#define WATER_RXWATER_OFF	16

DECLARE_GLOBAL_DATA_PTR;

#define LPUART_FLAG_REGMAP_32BIT_REG	BIT(0)
#define LPUART_FLAG_REGMAP_ENDIAN_BIG	BIT(1)

enum lpuart_devtype {
	DEV_VF610 = 1,
	DEV_LS1021A,
	DEV_MX7ULP,
	DEV_IMX8
};

struct lpuart_serial_platdata {
	void *reg;
	enum lpuart_devtype devtype;
	ulong flags;
};

static void lpuart_read32(u32 flags, u32 *addr, u32 *val)
{
	if (flags & LPUART_FLAG_REGMAP_32BIT_REG) {
		if (flags & LPUART_FLAG_REGMAP_ENDIAN_BIG)
			*(u32 *)val = in_be32(addr);
		else
			*(u32 *)val = in_le32(addr);
	}
}

static void lpuart_write32(u32 flags, u32 *addr, u32 val)
{
	if (flags & LPUART_FLAG_REGMAP_32BIT_REG) {
		if (flags & LPUART_FLAG_REGMAP_ENDIAN_BIG)
			out_be32(addr, val);
		else
			out_le32(addr, val);
	}
}


#ifndef CONFIG_SYS_CLK_FREQ
#define CONFIG_SYS_CLK_FREQ	0
#endif

u32 __weak get_lpuart_clk(void)
{
	return CONFIG_SYS_CLK_FREQ;
}

#if IS_ENABLED(CONFIG_CLK)
static int get_lpuart_clk_rate(struct udevice *dev, u32 *clk)
{
	struct clk per_clk;
	ulong rate;
	int ret;

	ret = clk_get_by_name(dev, "per", &per_clk);
	if (ret) {
		dev_err(dev, "Failed to get per clk: %d\n", ret);
		return ret;
	}

	rate = clk_get_rate(&per_clk);
	if ((long)rate <= 0) {
		dev_err(dev, "Failed to get per clk rate: %ld\n", (long)rate);
		return ret;
	}
	*clk = rate;
	return 0;
}
#else
static inline int get_lpuart_clk_rate(struct udevice *dev, u32 *clk)
{ return -ENOSYS; }
#endif

static bool is_lpuart32(struct udevice *dev)
{
	struct lpuart_serial_platdata *plat = dev->platdata;

	return plat->flags & LPUART_FLAG_REGMAP_32BIT_REG;
}

static void _lpuart_serial_setbrg(struct udevice *dev,
				  int baudrate)
{
	struct lpuart_serial_platdata *plat = dev_get_platdata(dev);
	struct lpuart_fsl *base = plat->reg;
	u32 clk;
	u16 sbr;
	int ret;

	if (IS_ENABLED(CONFIG_CLK)) {
		ret = get_lpuart_clk_rate(dev, &clk);
		if (ret)
			return;
	} else {
		clk = get_lpuart_clk();
	}

	sbr = (u16)(clk / (16 * baudrate));

	/* place adjustment later - n/32 BRFA */
	__raw_writeb(sbr >> 8, &base->ubdh);
	__raw_writeb(sbr & 0xff, &base->ubdl);
}

static int _lpuart_serial_getc(struct lpuart_serial_platdata *plat)
{
	struct lpuart_fsl *base = plat->reg;
	while (!(__raw_readb(&base->us1) & (US1_RDRF | US1_OR)))
		WATCHDOG_RESET();

	barrier();

	return __raw_readb(&base->ud);
}

static void _lpuart_serial_putc(struct lpuart_serial_platdata *plat,
				const char c)
{
	struct lpuart_fsl *base = plat->reg;

	while (!(__raw_readb(&base->us1) & US1_TDRE))
		WATCHDOG_RESET();

	__raw_writeb(c, &base->ud);
}

/* Test whether a character is in the RX buffer */
static int _lpuart_serial_tstc(struct lpuart_serial_platdata *plat)
{
	struct lpuart_fsl *base = plat->reg;

	if (__raw_readb(&base->urcfifo) == 0)
		return 0;

	return 1;
}

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 */
static int _lpuart_serial_init(struct udevice *dev)
{
	struct lpuart_serial_platdata *plat = dev_get_platdata(dev);
	struct lpuart_fsl *base = (struct lpuart_fsl *)plat->reg;
	u8 ctrl;

	ctrl = __raw_readb(&base->uc2);
	ctrl &= ~UC2_RE;
	ctrl &= ~UC2_TE;
	__raw_writeb(ctrl, &base->uc2);

	__raw_writeb(0, &base->umodem);
	__raw_writeb(0, &base->uc1);

	/* Disable FIFO and flush buffer */
	__raw_writeb(0x0, &base->upfifo);
	__raw_writeb(0x0, &base->utwfifo);
	__raw_writeb(0x1, &base->urwfifo);
	__raw_writeb(CFIFO_TXFLUSH | CFIFO_RXFLUSH, &base->ucfifo);

	/* provide data bits, parity, stop bit, etc */
	_lpuart_serial_setbrg(dev, gd->baudrate);

	__raw_writeb(UC2_RE | UC2_TE, &base->uc2);

	return 0;
}

static void _lpuart32_serial_setbrg_7ulp(struct udevice *dev,
					 int baudrate)
{
	struct lpuart_serial_platdata *plat = dev_get_platdata(dev);
	struct lpuart_fsl_reg32 *base = plat->reg;
	u32 sbr, osr, baud_diff, tmp_osr, tmp_sbr, tmp_diff, tmp;
	u32 clk;
	int ret;

	if (IS_ENABLED(CONFIG_CLK)) {
		ret = get_lpuart_clk_rate(dev, &clk);
		if (ret)
			return;
	} else {
		clk = get_lpuart_clk();
	}

	baud_diff = baudrate;
	osr = 0;
	sbr = 0;

	for (tmp_osr = 4; tmp_osr <= 32; tmp_osr++) {
		tmp_sbr = (clk / (baudrate * tmp_osr));

		if (tmp_sbr == 0)
			tmp_sbr = 1;

		/*calculate difference in actual buad w/ current values */
		tmp_diff = (clk / (tmp_osr * tmp_sbr));
		tmp_diff = tmp_diff - baudrate;

		/* select best values between sbr and sbr+1 */
		if (tmp_diff > (baudrate - (clk / (tmp_osr * (tmp_sbr + 1))))) {
			tmp_diff = baudrate - (clk / (tmp_osr * (tmp_sbr + 1)));
			tmp_sbr++;
		}

		if (tmp_diff <= baud_diff) {
			baud_diff = tmp_diff;
			osr = tmp_osr;
			sbr = tmp_sbr;
		}
	}

	/*
	 * TODO: handle buadrate outside acceptable rate
	 * if (baudDiff > ((config->baudRate_Bps / 100) * 3))
	 * {
	 *   Unacceptable baud rate difference of more than 3%
	 *   return kStatus_LPUART_BaudrateNotSupport;
	 * }
	 */
	tmp = in_le32(&base->baud);

	if ((osr > 3) && (osr < 8))
		tmp |= LPUART_BAUD_BOTHEDGE_MASK;

	tmp &= ~LPUART_BAUD_OSR_MASK;
	tmp |= LPUART_BAUD_OSR(osr-1);

	tmp &= ~LPUART_BAUD_SBR_MASK;
	tmp |= LPUART_BAUD_SBR(sbr);

	/* explicitly disable 10 bit mode & set 1 stop bit */
	tmp &= ~(LPUART_BAUD_M10_MASK | LPUART_BAUD_SBNS_MASK);

	out_le32(&base->baud, tmp);
}

static void _lpuart32_serial_setbrg(struct udevice *dev,
				    int baudrate)
{
	struct lpuart_serial_platdata *plat = dev_get_platdata(dev);
	struct lpuart_fsl_reg32 *base = plat->reg;
	u32 clk;
	u32 sbr;
	int ret;

	if (IS_ENABLED(CONFIG_CLK)) {
		ret = get_lpuart_clk_rate(dev, &clk);
		if (ret)
			return;
	} else {
		clk = get_lpuart_clk();
	}

	sbr = (clk / (16 * baudrate));

	/* place adjustment later - n/32 BRFA */
	lpuart_write32(plat->flags, &base->baud, sbr);
}

static int _lpuart32_serial_getc(struct lpuart_serial_platdata *plat)
{
	struct lpuart_fsl_reg32 *base = plat->reg;
	u32 stat, val;

	lpuart_read32(plat->flags, &base->stat, &stat);
	while ((stat & STAT_RDRF) == 0) {
		lpuart_write32(plat->flags, &base->stat, STAT_FLAGS);
		WATCHDOG_RESET();
		lpuart_read32(plat->flags, &base->stat, &stat);
	}

	lpuart_read32(plat->flags, &base->data, &val);

	lpuart_read32(plat->flags, &base->stat, &stat);
	if (stat & STAT_OR)
		lpuart_write32(plat->flags, &base->stat, STAT_OR);

	return val & 0x3ff;
}

static void _lpuart32_serial_putc(struct lpuart_serial_platdata *plat,
				  const char c)
{
	struct lpuart_fsl_reg32 *base = plat->reg;
	u32 stat;

	if (c == '\n')
		serial_putc('\r');

	while (true) {
		lpuart_read32(plat->flags, &base->stat, &stat);

		if ((stat & STAT_TDRE))
			break;

		WATCHDOG_RESET();
	}

	lpuart_write32(plat->flags, &base->data, c);
}

/* Test whether a character is in the RX buffer */
static int _lpuart32_serial_tstc(struct lpuart_serial_platdata *plat)
{
	struct lpuart_fsl_reg32 *base = plat->reg;
	u32 water;

	lpuart_read32(plat->flags, &base->water, &water);

	if ((water >> 24) == 0)
		return 0;

	return 1;
}

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 */
static int _lpuart32_serial_init(struct udevice *dev)
{
	struct lpuart_serial_platdata *plat = dev_get_platdata(dev);
	struct lpuart_fsl_reg32 *base = (struct lpuart_fsl_reg32 *)plat->reg;
	u32 val, tx_fifo_size;

	lpuart_read32(plat->flags, &base->ctrl, &val);
	val &= ~CTRL_RE;
	val &= ~CTRL_TE;
	lpuart_write32(plat->flags, &base->ctrl, val);

	lpuart_write32(plat->flags, &base->modir, 0);

	lpuart_read32(plat->flags, &base->fifo, &val);
	tx_fifo_size = (val & FIFO_TXSIZE_MASK) >> FIFO_TXSIZE_OFF;
	/* Set the TX water to half of FIFO size */
	if (tx_fifo_size > 1)
		tx_fifo_size = tx_fifo_size >> 1;

	/* Set RX water to 0, to be triggered by any receive data */
	lpuart_write32(plat->flags, &base->water,
		       (tx_fifo_size << WATER_TXWATER_OFF));

	/* Enable TX and RX FIFO */
	val |= (FIFO_TXFE | FIFO_RXFE | FIFO_TXFLUSH | FIFO_RXFLUSH);
	lpuart_write32(plat->flags, &base->fifo, val);

	lpuart_write32(plat->flags, &base->match, 0);

	if (plat->devtype == DEV_MX7ULP || plat->devtype == DEV_IMX8) {
		_lpuart32_serial_setbrg_7ulp(dev, gd->baudrate);
	} else {
		/* provide data bits, parity, stop bit, etc */
		_lpuart32_serial_setbrg(dev, gd->baudrate);
	}

	lpuart_write32(plat->flags, &base->ctrl, CTRL_RE | CTRL_TE);

	return 0;
}

static int lpuart_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct lpuart_serial_platdata *plat = dev_get_platdata(dev);

	if (is_lpuart32(dev)) {
		if (plat->devtype == DEV_MX7ULP || plat->devtype == DEV_IMX8)
			_lpuart32_serial_setbrg_7ulp(dev, baudrate);
		else
			_lpuart32_serial_setbrg(dev, baudrate);
	} else {
		_lpuart_serial_setbrg(dev, baudrate);
	}

	return 0;
}

static int lpuart_serial_getc(struct udevice *dev)
{
	struct lpuart_serial_platdata *plat = dev->platdata;

	if (is_lpuart32(dev))
		return _lpuart32_serial_getc(plat);

	return _lpuart_serial_getc(plat);
}

static int lpuart_serial_putc(struct udevice *dev, const char c)
{
	struct lpuart_serial_platdata *plat = dev->platdata;

	if (is_lpuart32(dev))
		_lpuart32_serial_putc(plat, c);
	else
		_lpuart_serial_putc(plat, c);

	return 0;
}

static int lpuart_serial_pending(struct udevice *dev, bool input)
{
	struct lpuart_serial_platdata *plat = dev->platdata;
	struct lpuart_fsl *reg = plat->reg;
	struct lpuart_fsl_reg32 *reg32 = plat->reg;
	u32 stat;

	if (is_lpuart32(dev)) {
		if (input) {
			return _lpuart32_serial_tstc(plat);
		} else {
			lpuart_read32(plat->flags, &reg32->stat, &stat);
			return stat & STAT_TDRE ? 0 : 1;
		}
	}

	if (input)
		return _lpuart_serial_tstc(plat);
	else
		return __raw_readb(&reg->us1) & US1_TDRE ? 0 : 1;
}

static int lpuart_serial_probe(struct udevice *dev)
{
	if (is_lpuart32(dev))
		return _lpuart32_serial_init(dev);
	else
		return _lpuart_serial_init(dev);
}

static int lpuart_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct lpuart_serial_platdata *plat = dev->platdata;
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev);
	fdt_addr_t addr;

	addr = devfdt_get_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->reg = (void *)addr;
	plat->flags = dev_get_driver_data(dev);

	if (!fdt_node_check_compatible(blob, node, "fsl,ls1021a-lpuart"))
		plat->devtype = DEV_LS1021A;
	else if (!fdt_node_check_compatible(blob, node, "fsl,imx7ulp-lpuart"))
		plat->devtype = DEV_MX7ULP;
	else if (!fdt_node_check_compatible(blob, node, "fsl,vf610-lpuart"))
		plat->devtype = DEV_VF610;
	else if (!fdt_node_check_compatible(blob, node, "fsl,imx8qm-lpuart"))
		plat->devtype = DEV_IMX8;

	return 0;
}

static const struct dm_serial_ops lpuart_serial_ops = {
	.putc = lpuart_serial_putc,
	.pending = lpuart_serial_pending,
	.getc = lpuart_serial_getc,
	.setbrg = lpuart_serial_setbrg,
};

static const struct udevice_id lpuart_serial_ids[] = {
	{ .compatible = "fsl,ls1021a-lpuart", .data =
		LPUART_FLAG_REGMAP_32BIT_REG | LPUART_FLAG_REGMAP_ENDIAN_BIG },
	{ .compatible = "fsl,imx7ulp-lpuart",
		.data = LPUART_FLAG_REGMAP_32BIT_REG },
	{ .compatible = "fsl,vf610-lpuart"},
	{ .compatible = "fsl,imx8qm-lpuart",
		.data = LPUART_FLAG_REGMAP_32BIT_REG },
	{ }
};

U_BOOT_DRIVER(serial_lpuart) = {
	.name	= "serial_lpuart",
	.id	= UCLASS_SERIAL,
	.of_match = lpuart_serial_ids,
	.ofdata_to_platdata = lpuart_serial_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct lpuart_serial_platdata),
	.probe = lpuart_serial_probe,
	.ops	= &lpuart_serial_ops,
};

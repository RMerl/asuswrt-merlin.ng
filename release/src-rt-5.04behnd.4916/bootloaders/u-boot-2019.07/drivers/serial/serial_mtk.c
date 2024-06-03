// SPDX-License-Identifier: GPL-2.0
/*
 * MediaTek High-speed UART driver
 *
 * Copyright (C) 2018 MediaTek Inc.
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <clk.h>
#include <common.h>
#include <div64.h>
#include <dm.h>
#include <errno.h>
#include <serial.h>
#include <watchdog.h>
#include <asm/io.h>
#include <asm/types.h>

struct mtk_serial_regs {
	u32 rbr;
	u32 ier;
	u32 fcr;
	u32 lcr;
	u32 mcr;
	u32 lsr;
	u32 msr;
	u32 spr;
	u32 mdr1;
	u32 highspeed;
	u32 sample_count;
	u32 sample_point;
	u32 fracdiv_l;
	u32 fracdiv_m;
	u32 escape_en;
	u32 guard;
	u32 rx_sel;
};

#define thr rbr
#define iir fcr
#define dll rbr
#define dlm ier

#define UART_LCR_WLS_8	0x03		/* 8 bit character length */
#define UART_LCR_DLAB	0x80		/* Divisor latch access bit */

#define UART_LSR_DR	0x01		/* Data ready */
#define UART_LSR_THRE	0x20		/* Xmit holding register empty */

/* the data is correct if the real baud is within 3%. */
#define BAUD_ALLOW_MAX(baud)	((baud) + (baud) * 3 / 100)
#define BAUD_ALLOW_MIX(baud)	((baud) - (baud) * 3 / 100)

struct mtk_serial_priv {
	struct mtk_serial_regs __iomem *regs;
	u32 clock;
};

static void _mtk_serial_setbrg(struct mtk_serial_priv *priv, int baud)
{
	bool support_clk12m_baud115200;
	u32 quot, samplecount, realbaud;

	if ((baud <= 115200) && (priv->clock == 12000000))
		support_clk12m_baud115200 = true;
	else
		support_clk12m_baud115200 = false;

	if (baud <= 115200) {
		writel(0, &priv->regs->highspeed);
		quot = DIV_ROUND_CLOSEST(priv->clock, 16 * baud);

		if (support_clk12m_baud115200) {
			writel(3, &priv->regs->highspeed);
			quot = DIV_ROUND_CLOSEST(priv->clock, 256 * baud);
			if (quot == 0)
				quot = 1;

			samplecount = DIV_ROUND_CLOSEST(priv->clock,
							quot * baud);
			if (samplecount != 0) {
				realbaud = priv->clock / samplecount / quot;
				if ((realbaud > BAUD_ALLOW_MAX(baud)) ||
				    (realbaud < BAUD_ALLOW_MIX(baud))) {
					pr_info("baud %d can't be handled\n",
						baud);
				}
			} else {
				pr_info("samplecount is 0\n");
			}
		}
	} else if (baud <= 576000) {
		writel(2, &priv->regs->highspeed);

		/* Set to next lower baudrate supported */
		if ((baud == 500000) || (baud == 576000))
			baud = 460800;
		quot = DIV_ROUND_UP(priv->clock, 4 * baud);
	} else {
		writel(3, &priv->regs->highspeed);
		quot = DIV_ROUND_UP(priv->clock, 256 * baud);
	}

	/* set divisor */
	writel(UART_LCR_WLS_8 | UART_LCR_DLAB, &priv->regs->lcr);
	writel(quot & 0xff, &priv->regs->dll);
	writel((quot >> 8) & 0xff, &priv->regs->dlm);
	writel(UART_LCR_WLS_8, &priv->regs->lcr);

	if (baud > 460800) {
		u32 tmp;

		tmp = DIV_ROUND_CLOSEST(priv->clock, quot * baud);
		writel(tmp - 1, &priv->regs->sample_count);
		writel((tmp - 2) >> 1, &priv->regs->sample_point);
	} else {
		writel(0, &priv->regs->sample_count);
		writel(0xff, &priv->regs->sample_point);
	}

	if (support_clk12m_baud115200) {
		writel(samplecount - 1, &priv->regs->sample_count);
		writel((samplecount - 2) >> 1, &priv->regs->sample_point);
	}
}

static int mtk_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct mtk_serial_priv *priv = dev_get_priv(dev);

	_mtk_serial_setbrg(priv, baudrate);

	return 0;
}

static int mtk_serial_putc(struct udevice *dev, const char ch)
{
	struct mtk_serial_priv *priv = dev_get_priv(dev);

	if (!(readl(&priv->regs->lsr) & UART_LSR_THRE))
		return -EAGAIN;

	writel(ch, &priv->regs->thr);

	if (ch == '\n')
		WATCHDOG_RESET();

	return 0;
}

static int mtk_serial_getc(struct udevice *dev)
{
	struct mtk_serial_priv *priv = dev_get_priv(dev);

	if (!(readl(&priv->regs->lsr) & UART_LSR_DR))
		return -EAGAIN;

	return readl(&priv->regs->rbr);
}

static int mtk_serial_pending(struct udevice *dev, bool input)
{
	struct mtk_serial_priv *priv = dev_get_priv(dev);

	if (input)
		return (readl(&priv->regs->lsr) & UART_LSR_DR) ? 1 : 0;
	else
		return (readl(&priv->regs->lsr) & UART_LSR_THRE) ? 0 : 1;
}

static int mtk_serial_probe(struct udevice *dev)
{
	struct mtk_serial_priv *priv = dev_get_priv(dev);

	/* Disable interrupt */
	writel(0, &priv->regs->ier);

	return 0;
}

static int mtk_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct mtk_serial_priv *priv = dev_get_priv(dev);
	fdt_addr_t addr;
	struct clk clk;
	int err;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->regs = map_physmem(addr, 0, MAP_NOCACHE);

	err = clk_get_by_index(dev, 0, &clk);
	if (!err) {
		err = clk_get_rate(&clk);
		if (!IS_ERR_VALUE(err))
			priv->clock = err;
	} else if (err != -ENOENT && err != -ENODEV && err != -ENOSYS) {
		debug("mtk_serial: failed to get clock\n");
		return err;
	}

	if (!priv->clock)
		priv->clock = dev_read_u32_default(dev, "clock-frequency", 0);

	if (!priv->clock) {
		debug("mtk_serial: clock not defined\n");
		return -EINVAL;
	}

	return 0;
}

static const struct dm_serial_ops mtk_serial_ops = {
	.putc = mtk_serial_putc,
	.pending = mtk_serial_pending,
	.getc = mtk_serial_getc,
	.setbrg = mtk_serial_setbrg,
};

static const struct udevice_id mtk_serial_ids[] = {
	{ .compatible = "mediatek,hsuart" },
	{ .compatible = "mediatek,mt6577-uart" },
	{ }
};

U_BOOT_DRIVER(serial_mtk) = {
	.name = "serial_mtk",
	.id = UCLASS_SERIAL,
	.of_match = mtk_serial_ids,
	.ofdata_to_platdata = mtk_serial_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct mtk_serial_priv),
	.probe = mtk_serial_probe,
	.ops = &mtk_serial_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

#ifdef CONFIG_DEBUG_UART_MTK

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
	struct mtk_serial_priv priv;

	priv.regs = (void *) CONFIG_DEBUG_UART_BASE;
	priv.clock = CONFIG_DEBUG_UART_CLOCK;

	writel(0, &priv.regs->ier);

	_mtk_serial_setbrg(&priv, CONFIG_BAUDRATE);
}

static inline void _debug_uart_putc(int ch)
{
	struct mtk_serial_regs __iomem *regs =
		(void *) CONFIG_DEBUG_UART_BASE;

	while (!(readl(&regs->lsr) & UART_LSR_THRE))
		;

	writel(ch, &regs->thr);
}

DEBUG_UART_FUNCS

#endif
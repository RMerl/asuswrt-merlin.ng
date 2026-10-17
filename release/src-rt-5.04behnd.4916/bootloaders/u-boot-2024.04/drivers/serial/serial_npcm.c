// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2021 Nuvoton Technology Corp.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <serial.h>

struct npcm_uart {
	union {
		u32	rbr;	/* Receive Buffer Register */
		u32	thr;	/* Transmit Holding Register */
		u32	dll;	/* Divisor Latch (Low Byte) Register */
	};
	union {
		u32	ier;	/* Interrupt Enable Register */
		u32	dlm;	/* Divisor Latch (Low Byte) Register */
	};
	union {
		u32	iir;	/* Interrupt Identification Register */
		u32	fcr;	/* FIFO Control Register */
	};
	u32	lcr;		/* Line Control Register */
	u32	mcr;		/* Modem Control Register */
	u32	lsr;		/* Line Status Control Register */
	u32	msr;		/* Modem Status Register */
	u32	tor;		/* Timeout Register */
};

#define	LCR_WLS_8BITS	3	/* 8-bit word length select */
#define	FCR_TFR		BIT(2)	/* TxFIFO reset */
#define	FCR_RFR		BIT(1)	/* RxFIFO reset */
#define	FCR_FME		BIT(0)	/* FIFO mode enable */
#define	LSR_THRE	BIT(5)	/* Status of TxFIFO empty */
#define	LSR_RFDR	BIT(0)	/* Status of RxFIFO data ready */
#define	LCR_DLAB	BIT(7)	/* Divisor latch access bit */

struct npcm_serial_plat {
	struct npcm_uart *reg;
	u32 uart_clk;		/* frequency of uart clock source */
};

static int npcm_serial_pending(struct udevice *dev, bool input)
{
	struct npcm_serial_plat *plat = dev_get_plat(dev);
	struct npcm_uart *uart = plat->reg;

	if (input)
		return readb(&uart->lsr) & LSR_RFDR ? 1 : 0;
	else
		return readb(&uart->lsr) & LSR_THRE ? 0 : 1;
}

static int npcm_serial_putc(struct udevice *dev, const char ch)
{
	struct npcm_serial_plat *plat = dev_get_plat(dev);
	struct npcm_uart *uart = plat->reg;

	if (!(readb(&uart->lsr) & LSR_THRE))
		return -EAGAIN;

	writeb(ch, &uart->thr);

	return 0;
}

static int npcm_serial_getc(struct udevice *dev)
{
	struct npcm_serial_plat *plat = dev_get_plat(dev);
	struct npcm_uart *uart = plat->reg;

	if (!(readb(&uart->lsr) & LSR_RFDR))
		return -EAGAIN;

	return readb(&uart->rbr);
}

static int npcm_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct npcm_serial_plat *plat = dev_get_plat(dev);
	struct npcm_uart *uart = plat->reg;
	u16 divisor;

	if (IS_ENABLED(CONFIG_SYS_SKIP_UART_INIT))
		return 0;

	/* BaudOut = UART Clock / (16 * [Divisor + 2]) */
	divisor = DIV_ROUND_CLOSEST(plat->uart_clk, 16 * baudrate) - 2;

	setbits_8(&uart->lcr, LCR_DLAB);
	writeb(divisor & 0xff, &uart->dll);
	writeb(divisor >> 8, &uart->dlm);
	clrbits_8(&uart->lcr, LCR_DLAB);

	return 0;
}

static int npcm_serial_probe(struct udevice *dev)
{
	struct npcm_serial_plat *plat = dev_get_plat(dev);
	struct npcm_uart *uart;
	struct clk clk, parent;
	u32 freq;
	int ret;

	plat->reg = dev_read_addr_ptr(dev);
	uart = plat->reg;

	if (!IS_ENABLED(CONFIG_SYS_SKIP_UART_INIT)) {
		freq = dev_read_u32_default(dev, "clock-frequency", 24000000);

		ret = clk_get_by_index(dev, 0, &clk);
		if (ret < 0)
			return ret;

		ret = clk_get_by_index(dev, 1, &parent);
		if (!ret) {
			ret = clk_set_parent(&clk, &parent);
			if (ret)
				return ret;
		}

		if (freq) {
			ret = clk_set_rate(&clk, freq);
			if (ret < 0)
				return ret;
		}
		plat->uart_clk = clk_get_rate(&clk);
	}

	/* Disable all interrupt */
	writeb(0, &uart->ier);

	/* Set 8 bit, 1 stop, no parity */
	writeb(LCR_WLS_8BITS, &uart->lcr);

	/* Reset RX/TX FIFO */
	writeb(FCR_FME | FCR_RFR | FCR_TFR, &uart->fcr);

	return 0;
}

static const struct dm_serial_ops npcm_serial_ops = {
	.getc = npcm_serial_getc,
	.setbrg = npcm_serial_setbrg,
	.putc = npcm_serial_putc,
	.pending = npcm_serial_pending,
};

static const struct udevice_id npcm_serial_ids[] = {
	{ .compatible = "nuvoton,npcm750-uart" },
	{ .compatible = "nuvoton,npcm845-uart" },
	{ }
};

U_BOOT_DRIVER(serial_npcm) = {
	.name	= "serial_npcm",
	.id	= UCLASS_SERIAL,
	.of_match = npcm_serial_ids,
	.plat_auto  = sizeof(struct npcm_serial_plat),
	.probe = npcm_serial_probe,
	.ops	= &npcm_serial_ops,
	.flags = DM_FLAG_PRE_RELOC,
};

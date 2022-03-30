// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 */

#include <common.h>
#include <errno.h>
#include <asm/io.h>

#define PCI_DEV_CONFIG(segbus, dev, fn) ( \
		(((segbus) & 0xfff) << 20) | \
		(((dev) & 0x1f) << 15) | \
		(((fn)  & 0x07) << 12))

/* Platform Controller Unit */
#define LPC_DEV			0x1f
#define LPC_FUNC		0

/* Enable UART */
#define UART_CONT		0x80

/* SCORE Pad definitions */
#define UART_RXD_PAD			82
#define UART_TXD_PAD			83

/* Pad base: PAD_CONF0[n]= PAD_BASE + 16 * n */
#define GPSCORE_PAD_BASE	(IO_BASE_ADDRESS + IO_BASE_OFFSET_GPSCORE)

/* IO Memory */
#define IO_BASE_ADDRESS			0xfed0c000
#define  IO_BASE_OFFSET_GPSCORE		0x0000
#define  IO_BASE_OFFSET_GPNCORE		0x1000
#define  IO_BASE_OFFSET_GPSSUS		0x2000
#define IO_BASE_SIZE			0x4000

static inline unsigned int score_pconf0(int pad_num)
{
	return GPSCORE_PAD_BASE + pad_num * 16;
}

static void score_select_func(int pad, int func)
{
	uint32_t reg;
	uint32_t pconf0_addr = score_pconf0(pad);

	reg = readl(pconf0_addr);
	reg &= ~0x7;
	reg |= func & 0x7;
	writel(reg, pconf0_addr);
}

static void x86_pci_write_config32(int dev, unsigned int where, u32 value)
{
	unsigned long addr;

	addr = CONFIG_PCIE_ECAM_BASE | dev | (where & ~3);
	writel(value, addr);
}

/* This can be called after memory-mapped PCI is working */
int setup_internal_uart(int enable)
{
	/* Enable or disable the legacy UART hardware */
	x86_pci_write_config32(PCI_DEV_CONFIG(0, LPC_DEV, LPC_FUNC), UART_CONT,
			       enable);

	/* All done for the disable part, so just return */
	if (!enable)
		return 0;

	/*
	 * Set up the pads to the UART function. This allows the signals to
	 * leave the chip
	 */
	score_select_func(UART_RXD_PAD, 1);
	score_select_func(UART_TXD_PAD, 1);

	/* TODO(sjg@chromium.org): Call debug_uart_init() */

	return 0;
}

void board_debug_uart_init(void)
{
	setup_internal_uart(1);
}

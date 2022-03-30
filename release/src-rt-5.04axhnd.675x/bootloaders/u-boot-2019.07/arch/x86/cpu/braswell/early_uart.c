// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2017, Bin Meng <bmeng.cn@gmail.com>
 */

#include <common.h>
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

/* UART PAD definitions */
#define UART_RXD_COMMUITY	1
#define UART_TXD_COMMUITY	1
#define UART_RXD_FAMILY		4
#define UART_TXD_FAMILY		4
#define UART_RXD_PAD		2
#define UART_TXD_PAD		7
#define UART_RXD_FUNC		3
#define UART_TXD_FUNC		3

/* IO Memory */
#define IO_BASE_ADDRESS		0xfed80000

static inline uint32_t gpio_pconf0(int community, int family, int pad)
{
	return IO_BASE_ADDRESS + community * 0x8000 + 0x4400 +
		family * 0x400 + pad * 8;
}

static void gpio_select_func(int community, int family, int pad, int func)
{
	uint32_t pconf0_addr = gpio_pconf0(community, family, pad);

	clrsetbits_le32(pconf0_addr, 0xf << 16, func << 16);
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
	gpio_select_func(UART_RXD_COMMUITY, UART_RXD_FAMILY,
			 UART_RXD_PAD, UART_RXD_FUNC);
	gpio_select_func(UART_TXD_COMMUITY, UART_TXD_FAMILY,
			 UART_TXD_PAD, UART_TXD_FUNC);

	return 0;
}

void board_debug_uart_init(void)
{
	setup_internal_uart(1);
}

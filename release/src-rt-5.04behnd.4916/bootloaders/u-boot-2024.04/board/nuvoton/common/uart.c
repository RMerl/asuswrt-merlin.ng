// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Nuvoton Technology Corp.
 */

#include <clk.h>
#include <dm.h>
#include <env.h>
#include <serial.h>
#include <linux/delay.h>

#define UART_DLL	0x0
#define UART_DLM	0x4
#define UART_LCR	0xc
#define LCR_DLAB	BIT(7)

void board_set_console(void)
{
	const unsigned long baudrate_table[] = CFG_SYS_BAUDRATE_TABLE;
	struct udevice *dev = gd->cur_serial_dev;
	unsigned int baudrate, max_delta;
	void __iomem *uart_reg;
	struct clk clk;
	char string[32];
	u32 uart_clk;
	u8 dll, dlm;
	u16 divisor;
	int ret, i;

	if (!dev)
		return;

	uart_reg = dev_read_addr_ptr(dev);
	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return;

	uart_clk = clk_get_rate(&clk);
	setbits_8(uart_reg + UART_LCR, LCR_DLAB);
	dll = readb(uart_reg + UART_DLL);
	dlm = readb(uart_reg + UART_DLM);
	clrbits_8(uart_reg + UART_LCR, LCR_DLAB);
	divisor = dll | (dlm << 8);
	baudrate =  uart_clk / ((16 * (divisor + 2)));
	for (i = 0; i < ARRAY_SIZE(baudrate_table); ++i) {
		max_delta = baudrate_table[i] / 20;
		if (abs(baudrate - baudrate_table[i]) < max_delta) {
			/* The baudrate is supported */
			gd->baudrate = baudrate_table[i];
			break;
		}
	}

	if (i == ARRAY_SIZE(baudrate_table)) {
		/* current baudrate is not suitable, set to default */
		divisor = DIV_ROUND_CLOSEST(uart_clk, 16 * gd->baudrate) - 2;
		setbits_8(uart_reg + UART_LCR, LCR_DLAB);
		writeb(divisor & 0xff, uart_reg + UART_DLL);
		writeb(divisor >> 8, uart_reg + UART_DLM);
		clrbits_8(uart_reg + UART_LCR, LCR_DLAB);
		udelay(100);
		printf("\r\nUART(source %u): change baudrate from %u to %u\n",
		       uart_clk, baudrate, uart_clk / ((16 * (divisor + 2))));
	}

	debug("Set env baudrate=%u\n", gd->baudrate);
	snprintf(string, sizeof(string), "ttyS0,%un8", gd->baudrate);
	env_set("console", string);

}

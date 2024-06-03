// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 * modified by Wolfgang Wegner <w.wegner@astro-kom.de> for ASTRO 5373l
 */

#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <asm/m5329.h>
#include <asm/immap_5329.h>
#include <asm/io.h>

/* needed for astro bus: */
#include <asm/uart.h>
#include "astro.h"

DECLARE_GLOBAL_DATA_PTR;
extern void uart_port_conf(void);

int checkboard(void)
{
	puts("Board: ");
	puts("ASTRO MCF5373L (Urmel) Board\n");
	return 0;
}

int dram_init(void)
{
#if !defined(CONFIG_MONITOR_IS_IN_RAM)
	sdram_t *sdp = (sdram_t *)(MMAP_SDRAM);

	/*
	 * GPIO configuration for bus should be set correctly from reset,
	 * so we do not care! First, set up address space: at this point,
	 * we should be running from internal SRAM;
	 * so use CONFIG_SYS_SDRAM_BASE as the base address for SDRAM,
	 * and do not care where it is
	 */
	__raw_writel((CONFIG_SYS_SDRAM_BASE & 0xFFF00000) | 0x00000018,
			&sdp->cs0);
	__raw_writel((CONFIG_SYS_SDRAM_BASE & 0xFFF00000) | 0x00000000,
			&sdp->cs1);
	/*
	 * I am not sure from the data sheet, but it seems burst length
	 * has to be 8 for the 16 bit data bus we use;
	 * so these values are for BL = 8
	 */
	__raw_writel(0x33211530, &sdp->cfg1);
	__raw_writel(0x56570000, &sdp->cfg2);
	/* send PrechargeALL, REF and IREF remain cleared! */
	__raw_writel(0xE1462C02, &sdp->ctrl);
	udelay(1);
	/* refresh SDRAM twice */
	__raw_writel(0xE1462C04, &sdp->ctrl);
	udelay(1);
	__raw_writel(0xE1462C04, &sdp->ctrl);
	/* init MR  */
	__raw_writel(0x008D0000, &sdp->mode);
	/* initialize EMR */
	__raw_writel(0x80010000, &sdp->mode);
	/* wait until DLL is locked */
	udelay(1);
	/*
	 * enable automatic refresh, lock mode register,
	 * clear iref and ipall
	 */
	__raw_writel(0x71462C00, &sdp->ctrl);
	/* Dummy write to start SDRAM */
	writel(0, CONFIG_SYS_SDRAM_BASE);
#endif

	/*
	 * for get_ram_size() to work, both CS areas have to be
	 * configured, i.e. CS1 has to be explicitely disabled, else
	 * probing for memory will cause the SDRAM bus to hang!
	 * (Do not rely on the SDCS register(s) being set to 0x00000000
	 * during reset as stated in the data sheet.)
	 */
	gd->ram_size = get_ram_size((long *)CONFIG_SYS_SDRAM_BASE,
				0x80000000 - CONFIG_SYS_SDRAM_BASE);

	return 0;
}

#define UART_BASE MMAP_UART0
int rs_serial_init(int port, int baud)
{
	uart_t *uart;
	u32 counter;

	switch (port) {
	case 0:
		uart = (uart_t *)(MMAP_UART0);
		break;
	case 1:
		uart = (uart_t *)(MMAP_UART1);
		break;
	case 2:
		uart = (uart_t *)(MMAP_UART2);
		break;
	default:
		uart = (uart_t *)(MMAP_UART0);
	}

	uart_port_conf();

	/* write to SICR: SIM2 = uart mode,dcd does not affect rx */
	writeb(UART_UCR_RESET_RX, &uart->ucr);
	writeb(UART_UCR_RESET_TX, &uart->ucr);
	writeb(UART_UCR_RESET_ERROR, &uart->ucr);
	writeb(UART_UCR_RESET_MR, &uart->ucr);
	__asm__ ("nop");

	writeb(0, &uart->uimr);

	/* write to CSR: RX/TX baud rate from timers */
	writeb(UART_UCSR_RCS_SYS_CLK | UART_UCSR_TCS_SYS_CLK, &uart->ucsr);

	writeb(UART_UMR_BC_8 | UART_UMR_PM_NONE, &uart->umr);
	writeb(UART_UMR_SB_STOP_BITS_1, &uart->umr);

	/* Setting up BaudRate */
	counter = (u32) (gd->bus_clk / (baud));
	counter >>= 5;

	/* write to CTUR: divide counter upper byte */
	writeb((u8) ((counter & 0xff00) >> 8), &uart->ubg1);
	/* write to CTLR: divide counter lower byte */
	writeb((u8) (counter & 0x00ff), &uart->ubg2);

	writeb(UART_UCR_RX_ENABLED | UART_UCR_TX_ENABLED, &uart->ucr);

	return 0;
}

void astro_put_char(char ch)
{
	uart_t *uart;
	unsigned long timer;

	uart = (uart_t *)(MMAP_UART0);
	/*
	 * Wait for last character to go. Timeout of 6ms should
	 * be enough for our lowest baud rate of 2400.
	 */
	timer = get_timer(0);
	while (get_timer(timer) < 6) {
		if (readb(&uart->usr) & UART_USR_TXRDY)
			break;
	}
	writeb(ch, &uart->utb);

	return;
}

int astro_is_char(void)
{
	uart_t *uart;

	uart = (uart_t *)(MMAP_UART0);
	return readb(&uart->usr) & UART_USR_RXRDY;
}

int astro_get_char(void)
{
	uart_t *uart;

	uart = (uart_t *)(MMAP_UART0);
	while (!(readb(&uart->usr) & UART_USR_RXRDY)) ;
	return readb(&uart->urb);
}

int misc_init_r(void)
{
	int retval = 0;

	puts("Configure Xilinx FPGA...");
	retval = astro5373l_xilinx_load();
	if (!retval) {
		puts("failed!\n");
		return retval;
	}
	puts("done\n");

	puts("Configure Altera FPGA...");
	retval = astro5373l_altera_load();
	if (!retval) {
		puts("failed!\n");
		return retval;
	}
	puts("done\n");

	return retval;
}

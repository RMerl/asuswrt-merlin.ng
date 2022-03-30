/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015 Rockchip Electronics Co., Ltd
 */

#ifndef __ASM_ARCH_UART_H
#define __ASM_ARCH_UART_H
struct rk_uart {
	unsigned int rbr; /* Receive buffer register. */
	unsigned int ier; /* Interrupt enable register. */
	unsigned int fcr; /* FIFO control register. */
	unsigned int lcr; /* Line control register. */
	unsigned int mcr; /* Modem control register. */
	unsigned int lsr; /* Line status register. */
	unsigned int msr; /* Modem status register. */
	unsigned int scr;
	unsigned int reserved1[(0x30 - 0x20) / 4];
	unsigned int srbr[(0x70 - 0x30) / 4];
	unsigned int far;
	unsigned int tfr;
	unsigned int rfw;
	unsigned int usr;
	unsigned int tfl;
	unsigned int rfl;
	unsigned int srr;
	unsigned int srts;
	unsigned int sbcr;
	unsigned int sdmam;
	unsigned int sfe;
	unsigned int srt;
	unsigned int stet;
	unsigned int htx;
	unsigned int dmasa;
	unsigned int reserver2[(0xf4 - 0xac) / 4];
	unsigned int cpr;
	unsigned int ucv;
	unsigned int ctr;
};

void rk_uart_init(void *base);
void print_hex(unsigned int n);
void print(char *s);
#endif

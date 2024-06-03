/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012 Samsung Electronics
 */

#ifndef _S3C24X0_I2C_H
#define _S3C24X0_I2C_H

struct s3c24x0_i2c {
	u32	iiccon;
	u32	iicstat;
	u32	iicadd;
	u32	iicds;
	u32	iiclc;
};

struct exynos5_hsi2c {
	u32	usi_ctl;
	u32	usi_fifo_ctl;
	u32	usi_trailing_ctl;
	u32	usi_clk_ctl;
	u32	usi_clk_slot;
	u32	spi_ctl;
	u32	uart_ctl;
	u32	res1;
	u32	usi_int_en;
	u32	usi_int_stat;
	u32	usi_modem_stat;
	u32	usi_error_stat;
	u32	usi_fifo_stat;
	u32	usi_txdata;
	u32	usi_rxdata;
	u32	res2;
	u32	usi_conf;
	u32	usi_auto_conf;
	u32	usi_timeout;
	u32	usi_manual_cmd;
	u32	usi_trans_status;
	u32	usi_timing_hs1;
	u32	usi_timing_hs2;
	u32	usi_timing_hs3;
	u32	usi_timing_fs1;
	u32	usi_timing_fs2;
	u32	usi_timing_fs3;
	u32	usi_timing_sla;
	u32	i2c_addr;
};

struct s3c24x0_i2c_bus {
	bool active;	/* port is active and available */
	int node;	/* device tree node */
	int bus_num;	/* i2c bus number */
	struct s3c24x0_i2c *regs;
	struct exynos5_hsi2c *hsregs;
	int is_highspeed;	/* High speed type, rather than I2C */
	unsigned clock_frequency;
	int id;
	unsigned clk_cycle;
	unsigned clk_div;
};

#define	I2C_WRITE	0
#define I2C_READ	1

#define I2C_OK		0
#define I2C_NOK		1
#define I2C_NACK	2
#define I2C_NOK_LA	3	/* Lost arbitration */
#define I2C_NOK_TOUT	4	/* time out */

/* S3C I2C Controller bits */
#define I2CSTAT_BSY	0x20	/* Busy bit */
#define I2CSTAT_NACK	0x01	/* Nack bit */
#define I2CCON_ACKGEN	0x80	/* Acknowledge generation */
#define I2CCON_IRPND	0x10	/* Interrupt pending bit */
#define I2C_MODE_MT	0xC0	/* Master Transmit Mode */
#define I2C_MODE_MR	0x80	/* Master Receive Mode */
#define I2C_START_STOP	0x20	/* START / STOP */
#define I2C_TXRX_ENA	0x10	/* I2C Tx/Rx enable */

#define I2C_TIMEOUT_MS 10		/* 10 ms */

#endif /* _S3C24X0_I2C_H */

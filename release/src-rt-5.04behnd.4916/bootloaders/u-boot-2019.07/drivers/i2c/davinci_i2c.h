/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2004-2014
 * Texas Instruments, <www.ti.com>
 *
 * Some changes copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 */
#ifndef _DAVINCI_I2C_H_
#define _DAVINCI_I2C_H_

#define I2C_WRITE		0
#define I2C_READ		1

struct i2c_regs {
	u32	i2c_oa;
	u32	i2c_ie;
	u32	i2c_stat;
	u32	i2c_scll;
	u32	i2c_sclh;
	u32	i2c_cnt;
	u32	i2c_drr;
	u32	i2c_sa;
	u32	i2c_dxr;
	u32	i2c_con;
	u32	i2c_iv;
	u32	res_2c;
	u32	i2c_psc;
};

/* I2C masks */

/* I2C Interrupt Enable Register (I2C_IE): */
#define I2C_IE_SCD_IE	(1 << 5)  /* Stop condition detect interrupt enable */
#define I2C_IE_XRDY_IE	(1 << 4)  /* Transmit data ready interrupt enable */
#define I2C_IE_RRDY_IE	(1 << 3)  /* Receive data ready interrupt enable */
#define I2C_IE_ARDY_IE	(1 << 2)  /* Register access ready interrupt enable */
#define I2C_IE_NACK_IE	(1 << 1)  /* No acknowledgment interrupt enable */
#define I2C_IE_AL_IE	(1 << 0)  /* Arbitration lost interrupt enable */

/* I2C Status Register (I2C_STAT): */

#define I2C_STAT_BB	(1 << 12) /* Bus busy */
#define I2C_STAT_ROVR	(1 << 11) /* Receive overrun */
#define I2C_STAT_XUDF	(1 << 10) /* Transmit underflow */
#define I2C_STAT_AAS	(1 << 9)  /* Address as slave */
#define I2C_STAT_SCD	(1 << 5)  /* Stop condition detect */
#define I2C_STAT_XRDY	(1 << 4)  /* Transmit data ready */
#define I2C_STAT_RRDY	(1 << 3)  /* Receive data ready */
#define I2C_STAT_ARDY	(1 << 2)  /* Register access ready */
#define I2C_STAT_NACK	(1 << 1)  /* No acknowledgment interrupt enable */
#define I2C_STAT_AL	(1 << 0)  /* Arbitration lost interrupt enable */

/* I2C Interrupt Code Register (I2C_INTCODE): */

#define I2C_INTCODE_MASK	7
#define I2C_INTCODE_NONE	0
#define I2C_INTCODE_AL		1 /* Arbitration lost */
#define I2C_INTCODE_NAK		2 /* No acknowledgement/general call */
#define I2C_INTCODE_ARDY	3 /* Register access ready */
#define I2C_INTCODE_RRDY	4 /* Rcv data ready */
#define I2C_INTCODE_XRDY	5 /* Xmit data ready */
#define I2C_INTCODE_SCD		6 /* Stop condition detect */

/* I2C Configuration Register (I2C_CON): */

#define I2C_CON_EN	(1 << 5)   /* I2C module enable */
#define I2C_CON_STB	(1 << 4)   /* Start byte mode (master mode only) */
#define I2C_CON_MST	(1 << 10)  /* Master/slave mode */
#define I2C_CON_TRX	(1 << 9)   /* Tx/Rx mode (master mode only) */
#define I2C_CON_XA	(1 << 8)   /* Expand address */
#define I2C_CON_STP	(1 << 11)  /* Stop condition (master mode only) */
#define I2C_CON_STT	(1 << 13)  /* Start condition (master mode only) */
#define I2C_CON_FREE	(1 << 14)  /* Free run on emulation */

#define I2C_TIMEOUT	0xffff0000 /* Timeout mask for poll_i2c_irq() */

#endif

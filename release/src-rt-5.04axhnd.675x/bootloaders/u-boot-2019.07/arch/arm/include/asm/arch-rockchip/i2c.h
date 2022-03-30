/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2012 SAMSUNG Electronics
 * Jaehoon Chung <jh80.chung@samsung.com>
 */

#ifndef __ASM_ARCH_I2C_H
#define __ASM_ARCH_I2C_H

struct i2c_regs {
	u32 con;
	u32 clkdiv;
	u32 mrxaddr;
	u32 mrxraddr;
	u32 mtxcnt;
	u32 mrxcnt;
	u32 ien;
	u32 ipd;
	u32 fcnt;
	u32 reserved0[0x37];
	u32 txdata[8];
	u32 reserved1[0x38];
	u32 rxdata[8];
};

/* Control register */
#define I2C_CON_EN		(1 << 0)
#define I2C_CON_MOD(mod)	((mod) << 1)
#define I2C_MODE_TX		0x00
#define I2C_MODE_TRX		0x01
#define I2C_MODE_RX		0x02
#define I2C_MODE_RRX		0x03
#define I2C_CON_MASK		(3 << 1)

#define I2C_CON_START		(1 << 3)
#define I2C_CON_STOP		(1 << 4)
#define I2C_CON_LASTACK		(1 << 5)
#define I2C_CON_ACTACK		(1 << 6)

/* Clock dividor register */
#define I2C_CLKDIV_VAL(divl, divh) \
	(((divl) & 0xffff) | (((divh) << 16) & 0xffff0000))

/* the slave address accessed  for master rx mode */
#define I2C_MRXADDR_SET(vld, addr)	(((vld) << 24) | (addr))

/* the slave register address accessed  for master rx mode */
#define I2C_MRXRADDR_SET(vld, raddr)	(((vld) << 24) | (raddr))

/* interrupt enable register */
#define I2C_BTFIEN		(1 << 0)
#define I2C_BRFIEN		(1 << 1)
#define I2C_MBTFIEN		(1 << 2)
#define I2C_MBRFIEN		(1 << 3)
#define I2C_STARTIEN		(1 << 4)
#define I2C_STOPIEN		(1 << 5)
#define I2C_NAKRCVIEN		(1 << 6)

/* interrupt pending register */
#define I2C_BTFIPD              (1 << 0)
#define I2C_BRFIPD              (1 << 1)
#define I2C_MBTFIPD             (1 << 2)
#define I2C_MBRFIPD             (1 << 3)
#define I2C_STARTIPD            (1 << 4)
#define I2C_STOPIPD             (1 << 5)
#define I2C_NAKRCVIPD           (1 << 6)
#define I2C_IPD_ALL_CLEAN       0x7f

#endif

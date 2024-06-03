/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2012-2020  ASPEED Technology Inc.
 * Copyright 2016 IBM Corporation
 * Copyright 2017 Google, Inc.
 */
#ifndef __AST_I2C_H_
#define __AST_I2C_H_

struct ast_i2c_regs {
	u32 fcr;
	u32 cactcr1;
	u32 cactcr2;
	u32 icr;
	u32 isr;
	u32 csr;
	u32 sdar;
	u32 pbcr;
	u32 trbbr;
#ifdef CONFIG_ASPEED_AST2500
	u32 dma_mbar;
	u32 dma_tlr;
#endif
};

/* Device Register Definition */
/* 0x00 : I2CD Function Control Register  */
#define I2CD_BUFF_SEL_MASK				(0x7 << 20)
#define I2CD_BUFF_SEL(x)				(x << 20)
#define I2CD_M_SDA_LOCK_EN			(0x1 << 16)
#define I2CD_MULTI_MASTER_DIS			(0x1 << 15)
#define I2CD_M_SCL_DRIVE_EN		(0x1 << 14)
#define I2CD_MSB_STS					(0x1 << 9)
#define I2CD_SDA_DRIVE_1T_EN			(0x1 << 8)
#define I2CD_M_SDA_DRIVE_1T_EN		(0x1 << 7)
#define I2CD_M_HIGH_SPEED_EN		(0x1 << 6)
#define I2CD_DEF_ADDR_EN				(0x1 << 5)
#define I2CD_DEF_ALERT_EN				(0x1 << 4)
#define I2CD_DEF_ARP_EN					(0x1 << 3)
#define I2CD_DEF_GCALL_EN				(0x1 << 2)
#define I2CD_SLAVE_EN					(0x1 << 1)
#define I2CD_MASTER_EN					(0x1)

/* 0x04 : I2CD Clock and AC Timing Control Register #1 */
/* Base register value. These bits are always set by the driver. */
#define I2CD_CACTC_BASE			0xfff00300
#define I2CD_TCKHIGH_SHIFT			16
#define I2CD_TCKLOW_SHIFT			12
#define I2CD_THDDAT_SHIFT			10
#define I2CD_TO_DIV_SHIFT			8
#define I2CD_BASE_DIV_SHIFT			0

/* 0x08 : I2CD Clock and AC Timing Control Register #2 */
#define I2CD_tTIMEOUT					1
#define I2CD_NO_TIMEOUT_CTRL			0

/* 0x0c : I2CD Interrupt Control Register &
 * 0x10 : I2CD Interrupt Status Register
 *
 * These share bit definitions, so use the same values for the enable &
 * status bits.
 */
#define I2CD_INTR_SDA_DL_TIMEOUT			(0x1 << 14)
#define I2CD_INTR_BUS_RECOVER_DONE			(0x1 << 13)
#define I2CD_INTR_SMBUS_ALERT			(0x1 << 12)
#define I2CD_INTR_SMBUS_ARP_ADDR			(0x1 << 11)
#define I2CD_INTR_SMBUS_DEV_ALERT_ADDR		(0x1 << 10)
#define I2CD_INTR_SMBUS_DEF_ADDR			(0x1 << 9)
#define I2CD_INTR_GCALL_ADDR			(0x1 << 8)
#define I2CD_INTR_SLAVE_MATCH			(0x1 << 7)
#define I2CD_INTR_SCL_TIMEOUT			(0x1 << 6)
#define I2CD_INTR_ABNORMAL				(0x1 << 5)
#define I2CD_INTR_NORMAL_STOP			(0x1 << 4)
#define I2CD_INTR_ARBIT_LOSS			(0x1 << 3)
#define I2CD_INTR_RX_DONE				(0x1 << 2)
#define I2CD_INTR_TX_NAK				(0x1 << 1)
#define I2CD_INTR_TX_ACK				(0x1 << 0)

/* 0x14 : I2CD Command/Status Register   */
#define I2CD_SDA_OE					(0x1 << 28)
#define I2CD_SDA_O					(0x1 << 27)
#define I2CD_SCL_OE					(0x1 << 26)
#define I2CD_SCL_O					(0x1 << 25)
#define I2CD_TX_TIMING				(0x1 << 24)
#define I2CD_TX_STATUS				(0x1 << 23)

/* Tx State Machine */
#define I2CD_IDLE					0x0
#define I2CD_MACTIVE				0x8
#define I2CD_MSTART					0x9
#define I2CD_MSTARTR				0xa
#define I2CD_MSTOP					0xb
#define I2CD_MTXD					0xc
#define I2CD_MRXACK					0xd
#define I2CD_MRXD					0xe
#define I2CD_MTXACK				0xf
#define I2CD_SWAIT					0x1
#define I2CD_SRXD					0x4
#define I2CD_STXACK				0x5
#define I2CD_STXD					0x6
#define I2CD_SRXACK				0x7
#define I2CD_RECOVER				0x3

#define I2CD_SCL_LINE_STS				(0x1 << 18)
#define I2CD_SDA_LINE_STS				(0x1 << 17)
#define I2CD_BUS_BUSY_STS				(0x1 << 16)
#define I2CD_SDA_OE_OUT_DIR				(0x1 << 15)
#define I2CD_SDA_O_OUT_DIR				(0x1 << 14)
#define I2CD_SCL_OE_OUT_DIR				(0x1 << 13)
#define I2CD_SCL_O_OUT_DIR				(0x1 << 12)
#define I2CD_BUS_RECOVER_CMD			(0x1 << 11)
#define I2CD_S_ALT_EN				(0x1 << 10)
#define I2CD_RX_DMA_ENABLE				(0x1 << 9)
#define I2CD_TX_DMA_ENABLE				(0x1 << 8)

/* Command Bit */
#define I2CD_RX_BUFF_ENABLE				(0x1 << 7)
#define I2CD_TX_BUFF_ENABLE				(0x1 << 6)
#define I2CD_M_STOP_CMD					(0x1 << 5)
#define I2CD_M_S_RX_CMD_LAST			(0x1 << 4)
#define I2CD_M_RX_CMD					(0x1 << 3)
#define I2CD_S_TX_CMD					(0x1 << 2)
#define I2CD_M_TX_CMD					(0x1 << 1)
#define I2CD_M_START_CMD				0x1

#define I2CD_RX_DATA_SHIFT			8
#define I2CD_RX_DATA_MASK			(0xff << I2CD_RX_DATA_SHIFT)

#define I2C_HIGHSPEED_RATE    400000

#endif				/* __AST_I2C_H_ */

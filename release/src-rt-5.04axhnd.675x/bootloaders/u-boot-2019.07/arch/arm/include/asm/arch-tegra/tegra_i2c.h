/* SPDX-License-Identifier: GPL-2.0 */
/*
 * NVIDIA Tegra I2C controller
 *
 * Copyright 2010-2011 NVIDIA Corporation
 */

#ifndef _TEGRA_I2C_H_
#define _TEGRA_I2C_H_

#include <asm/types.h>

enum {
	I2C_TIMEOUT_USEC = 10000,	/* Wait time for completion */
	I2C_FIFO_DEPTH = 8,		/* I2C fifo depth */
};

enum i2c_transaction_flags {
	I2C_IS_WRITE = 0x1,		/* for I2C write operation */
	I2C_IS_10_BIT_ADDRESS = 0x2,	/* for 10-bit I2C slave address */
	I2C_USE_REPEATED_START = 0x4,	/* for repeat start */
	I2C_NO_ACK = 0x8,		/* for slave that won't generate ACK */
	I2C_SOFTWARE_CONTROLLER	= 0x10,	/* for I2C transfer using GPIO */
	I2C_NO_STOP = 0x20,
};

/* Contians the I2C transaction details */
struct i2c_trans_info {
	/* flags to indicate the transaction details */
	enum i2c_transaction_flags flags;
	u32 address;	/* I2C slave device address */
	u32 num_bytes;	/* number of bytes to be transferred */
	/*
	 * Send/receive buffer. For the I2C send operation this buffer should
	 * be filled with the data to be sent to the slave device. For the I2C
	 * receive operation this buffer is filled with the data received from
	 * the slave device.
	 */
	u8 *buf;
	int is_10bit_address;
};

struct i2c_control {
	u32 tx_fifo;
	u32 rx_fifo;
	u32 packet_status;
	u32 fifo_control;
	u32 fifo_status;
	u32 int_mask;
	u32 int_status;
};

struct dvc_ctlr {
	u32 ctrl1;			/* 00: DVC_CTRL_REG1 */
	u32 ctrl2;			/* 04: DVC_CTRL_REG2 */
	u32 ctrl3;			/* 08: DVC_CTRL_REG3 */
	u32 status;			/* 0C: DVC_STATUS_REG */
	u32 ctrl;			/* 10: DVC_I2C_CTRL_REG */
	u32 addr_data;			/* 14: DVC_I2C_ADDR_DATA_REG */
	u32 reserved_0[2];		/* 18: */
	u32 req;			/* 20: DVC_REQ_REGISTER */
	u32 addr_data3;			/* 24: DVC_I2C_ADDR_DATA_REG_3 */
	u32 reserved_1[6];		/* 28: */
	u32 cnfg;			/* 40: DVC_I2C_CNFG */
	u32 cmd_addr0;			/* 44: DVC_I2C_CMD_ADDR0 */
	u32 cmd_addr1;			/* 48: DVC_I2C_CMD_ADDR1 */
	u32 cmd_data1;			/* 4C: DVC_I2C_CMD_DATA1 */
	u32 cmd_data2;			/* 50: DVC_I2C_CMD_DATA2 */
	u32 reserved_2[2];		/* 54: */
	u32 i2c_status;			/* 5C: DVC_I2C_STATUS */
	struct i2c_control control;	/* 60 ~ 78 */
};

struct i2c_ctlr {
	u32 cnfg;			/* 00: I2C_I2C_CNFG */
	u32 cmd_addr0;			/* 04: I2C_I2C_CMD_ADDR0 */
	u32 cmd_addr1;			/* 08: I2C_I2C_CMD_DATA1 */
	u32 cmd_data1;			/* 0C: I2C_I2C_CMD_DATA2 */
	u32 cmd_data2;			/* 10: DVC_I2C_CMD_DATA2 */
	u32 reserved_0[2];		/* 14: */
	u32 status;			/* 1C: I2C_I2C_STATUS */
	u32 sl_cnfg;			/* 20: I2C_I2C_SL_CNFG */
	u32 sl_rcvd;			/* 24: I2C_I2C_SL_RCVD */
	u32 sl_status;			/* 28: I2C_I2C_SL_STATUS */
	u32 sl_addr1;			/* 2C: I2C_I2C_SL_ADDR1 */
	u32 sl_addr2;			/* 30: I2C_I2C_SL_ADDR2 */
	u32 reserved_1[2];		/* 34: */
	u32 sl_delay_count;		/* 3C: I2C_I2C_SL_DELAY_COUNT */
	u32 reserved_2[4];		/* 40: */
	struct i2c_control control;	/* 50 ~ 68 */
	u32 clk_div;			/* 6C: I2C_I2C_CLOCK_DIVISOR */
};

/* bit fields definitions for IO Packet Header 1 format */
#define PKT_HDR1_PROTOCOL_SHIFT		4
#define PKT_HDR1_PROTOCOL_MASK		(0xf << PKT_HDR1_PROTOCOL_SHIFT)
#define PKT_HDR1_CTLR_ID_SHIFT		12
#define PKT_HDR1_CTLR_ID_MASK		(0xf << PKT_HDR1_CTLR_ID_SHIFT)
#define PKT_HDR1_PKT_ID_SHIFT		16
#define PKT_HDR1_PKT_ID_MASK		(0xff << PKT_HDR1_PKT_ID_SHIFT)
#define PROTOCOL_TYPE_I2C		1

/* bit fields definitions for IO Packet Header 2 format */
#define PKT_HDR2_PAYLOAD_SIZE_SHIFT	0
#define PKT_HDR2_PAYLOAD_SIZE_MASK	(0xfff << PKT_HDR2_PAYLOAD_SIZE_SHIFT)

/* bit fields definitions for IO Packet Header 3 format */
#define PKT_HDR3_READ_MODE_SHIFT	19
#define PKT_HDR3_READ_MODE_MASK		(1 << PKT_HDR3_READ_MODE_SHIFT)
#define PKT_HDR3_REPEAT_START_SHIFT	16
#define PKT_HDR3_REPEAT_START_MASK	(1 << PKT_HDR3_REPEAT_START_SHIFT)
#define PKT_HDR3_SLAVE_ADDR_SHIFT	0
#define PKT_HDR3_SLAVE_ADDR_MASK	(0x3ff << PKT_HDR3_SLAVE_ADDR_SHIFT)

#define DVC_CTRL_REG3_I2C_HW_SW_PROG_SHIFT	26
#define DVC_CTRL_REG3_I2C_HW_SW_PROG_MASK	\
				(1 << DVC_CTRL_REG3_I2C_HW_SW_PROG_SHIFT)

/* I2C_CNFG */
#define I2C_CNFG_NEW_MASTER_FSM_SHIFT	11
#define I2C_CNFG_NEW_MASTER_FSM_MASK	(1 << I2C_CNFG_NEW_MASTER_FSM_SHIFT)
#define I2C_CNFG_PACKET_MODE_SHIFT	10
#define I2C_CNFG_PACKET_MODE_MASK	(1 << I2C_CNFG_PACKET_MODE_SHIFT)

/* I2C_SL_CNFG */
#define I2C_SL_CNFG_NEWSL_SHIFT		2
#define I2C_SL_CNFG_NEWSL_MASK		(1 << I2C_SL_CNFG_NEWSL_SHIFT)

/* I2C_FIFO_STATUS */
#define TX_FIFO_FULL_CNT_SHIFT		0
#define TX_FIFO_FULL_CNT_MASK		(0xf << TX_FIFO_FULL_CNT_SHIFT)
#define TX_FIFO_EMPTY_CNT_SHIFT		4
#define TX_FIFO_EMPTY_CNT_MASK		(0xf << TX_FIFO_EMPTY_CNT_SHIFT)

/* I2C_INTERRUPT_STATUS */
#define I2C_INT_XFER_COMPLETE_SHIFT	7
#define I2C_INT_XFER_COMPLETE_MASK	(1 << I2C_INT_XFER_COMPLETE_SHIFT)
#define I2C_INT_NO_ACK_SHIFT		3
#define I2C_INT_NO_ACK_MASK		(1 << I2C_INT_NO_ACK_SHIFT)
#define I2C_INT_ARBITRATION_LOST_SHIFT	2
#define I2C_INT_ARBITRATION_LOST_MASK	(1 << I2C_INT_ARBITRATION_LOST_SHIFT)

/* I2C_CLK_DIVISOR_REGISTER */
#define CLK_DIV_STD_FAST_MODE		0x19
#define CLK_DIV_HS_MODE			1
#define CLK_MULT_STD_FAST_MODE		8

/**
 * Returns the bus number of the DVC controller
 *
 * @return number of bus, or -1 if there is no DVC active
 */
int tegra_i2c_get_dvc_bus(struct udevice **busp);

#endif	/* _TEGRA_I2C_H_ */

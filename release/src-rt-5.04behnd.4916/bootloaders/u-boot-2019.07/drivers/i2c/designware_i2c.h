/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Vipin Kumar, ST Micoelectronics, vipin.kumar@st.com.
 */

#ifndef __DW_I2C_H_
#define __DW_I2C_H_

struct i2c_regs {
	u32 ic_con;		/* 0x00 */
	u32 ic_tar;		/* 0x04 */
	u32 ic_sar;		/* 0x08 */
	u32 ic_hs_maddr;	/* 0x0c */
	u32 ic_cmd_data;	/* 0x10 */
	u32 ic_ss_scl_hcnt;	/* 0x14 */
	u32 ic_ss_scl_lcnt;	/* 0x18 */
	u32 ic_fs_scl_hcnt;	/* 0x1c */
	u32 ic_fs_scl_lcnt;	/* 0x20 */
	u32 ic_hs_scl_hcnt;	/* 0x24 */
	u32 ic_hs_scl_lcnt;	/* 0x28 */
	u32 ic_intr_stat;	/* 0x2c */
	u32 ic_intr_mask;	/* 0x30 */
	u32 ic_raw_intr_stat;	/* 0x34 */
	u32 ic_rx_tl;		/* 0x38 */
	u32 ic_tx_tl;		/* 0x3c */
	u32 ic_clr_intr;	/* 0x40 */
	u32 ic_clr_rx_under;	/* 0x44 */
	u32 ic_clr_rx_over;	/* 0x48 */
	u32 ic_clr_tx_over;	/* 0x4c */
	u32 ic_clr_rd_req;	/* 0x50 */
	u32 ic_clr_tx_abrt;	/* 0x54 */
	u32 ic_clr_rx_done;	/* 0x58 */
	u32 ic_clr_activity;	/* 0x5c */
	u32 ic_clr_stop_det;	/* 0x60 */
	u32 ic_clr_start_det;	/* 0x64 */
	u32 ic_clr_gen_call;	/* 0x68 */
	u32 ic_enable;		/* 0x6c */
	u32 ic_status;		/* 0x70 */
	u32 ic_txflr;		/* 0x74 */
	u32 ic_rxflr;		/* 0x78 */
	u32 ic_sda_hold;	/* 0x7c */
	u32 ic_tx_abrt_source;	/* 0x80 */
	u8 res1[0x18];		/* 0x84 */
	u32 ic_enable_status;	/* 0x9c */
};

#if !defined(IC_CLK)
#define IC_CLK			166
#endif
#define NANO_TO_MICRO		1000

/* High and low times in different speed modes (in ns) */
#define MIN_SS_SCL_HIGHTIME	4000
#define MIN_SS_SCL_LOWTIME	4700
#define MIN_FS_SCL_HIGHTIME	600
#define MIN_FS_SCL_LOWTIME	1300
#define MIN_HS_SCL_HIGHTIME	60
#define MIN_HS_SCL_LOWTIME	160

/* Worst case timeout for 1 byte is kept as 2ms */
#define I2C_BYTE_TO		(CONFIG_SYS_HZ/500)
#define I2C_STOPDET_TO		(CONFIG_SYS_HZ/500)
#define I2C_BYTE_TO_BB		(I2C_BYTE_TO * 16)

/* i2c control register definitions */
#define IC_CON_SD		0x0040
#define IC_CON_RE		0x0020
#define IC_CON_10BITADDRMASTER	0x0010
#define IC_CON_10BITADDR_SLAVE	0x0008
#define IC_CON_SPD_MSK		0x0006
#define IC_CON_SPD_SS		0x0002
#define IC_CON_SPD_FS		0x0004
#define IC_CON_SPD_HS		0x0006
#define IC_CON_MM		0x0001

/* i2c target address register definitions */
#define TAR_ADDR		0x0050

/* i2c slave address register definitions */
#define IC_SLAVE_ADDR		0x0002

/* i2c data buffer and command register definitions */
#define IC_CMD			0x0100
#define IC_STOP			0x0200

/* i2c interrupt status register definitions */
#define IC_GEN_CALL		0x0800
#define IC_START_DET		0x0400
#define IC_STOP_DET		0x0200
#define IC_ACTIVITY		0x0100
#define IC_RX_DONE		0x0080
#define IC_TX_ABRT		0x0040
#define IC_RD_REQ		0x0020
#define IC_TX_EMPTY		0x0010
#define IC_TX_OVER		0x0008
#define IC_RX_FULL		0x0004
#define IC_RX_OVER 		0x0002
#define IC_RX_UNDER		0x0001

/* fifo threshold register definitions */
#define IC_TL0			0x00
#define IC_TL1			0x01
#define IC_TL2			0x02
#define IC_TL3			0x03
#define IC_TL4			0x04
#define IC_TL5			0x05
#define IC_TL6			0x06
#define IC_TL7			0x07
#define IC_RX_TL		IC_TL0
#define IC_TX_TL		IC_TL0

/* i2c enable register definitions */
#define IC_ENABLE_0B		0x0001

/* i2c status register  definitions */
#define IC_STATUS_SA		0x0040
#define IC_STATUS_MA		0x0020
#define IC_STATUS_RFF		0x0010
#define IC_STATUS_RFNE		0x0008
#define IC_STATUS_TFE		0x0004
#define IC_STATUS_TFNF		0x0002
#define IC_STATUS_ACT		0x0001

/* Speed Selection */
#define IC_SPEED_MODE_STANDARD	1
#define IC_SPEED_MODE_FAST	2
#define IC_SPEED_MODE_MAX	3

#define I2C_MAX_SPEED		3400000
#define I2C_FAST_SPEED		400000
#define I2C_STANDARD_SPEED	100000

#endif /* __DW_I2C_H_ */

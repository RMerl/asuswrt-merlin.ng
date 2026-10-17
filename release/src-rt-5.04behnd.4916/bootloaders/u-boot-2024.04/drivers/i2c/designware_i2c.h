/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009
 * Vipin Kumar, STMicroelectronics, vipin.kumar@st.com.
 */

#ifndef __DW_I2C_H_
#define __DW_I2C_H_

#include <clk.h>
#include <i2c.h>
#include <reset.h>
#include <linux/bitops.h>

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
	u32 slv_data_nak_only;
	u32 dma_cr;
	u32 dma_tdlr;
	u32 dma_rdlr;
	u32 sda_setup;
	u32 ack_general_call;
	u32 ic_enable_status;	/* 0x9c */
	u32 fs_spklen;
	u32 hs_spklen;
	u32 clr_restart_det;
	u8 reserved[0xf4 - 0xac];
	u32 comp_param1;	/* 0xf4 */
	u32 comp_version;
	u32 comp_type;
};

#define IC_CLK			166666666
#define NANO_TO_KILO		1000000

/* High and low times in different speed modes (in ns) */
#define MIN_SS_SCL_HIGHTIME	4000
#define MIN_SS_SCL_LOWTIME	4700
#define MIN_FS_SCL_HIGHTIME	600
#define MIN_FS_SCL_LOWTIME	1300
#define MIN_FP_SCL_HIGHTIME	260
#define MIN_FP_SCL_LOWTIME	500
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
#define IC_RX_OVER		0x0002
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

#define DW_IC_COMP_PARAM_1_SPEED_MODE_HIGH      (BIT(2) | BIT(3))
#define DW_IC_COMP_PARAM_1_SPEED_MODE_MASK      (BIT(2) | BIT(3))

/**
 * struct dw_scl_sda_cfg - I2C timing configuration
 *
 * @ss_hcnt: Standard speed high time in ns
 * @fs_hcnt: Fast speed high time in ns
 * @hs_hcnt: High speed high time in ns
 * @ss_lcnt: Standard speed low time in ns
 * @fs_lcnt: Fast speed low time in ns
 * @hs_lcnt: High speed low time in ns
 * @sda_hold: SDA hold time
 */
struct dw_scl_sda_cfg {
	u32 ss_hcnt;
	u32 fs_hcnt;
	u32 hs_hcnt;
	u32 ss_lcnt;
	u32 fs_lcnt;
	u32 hs_lcnt;
	u32 sda_hold;
};

/**
 * struct dw_i2c_speed_config - timings to use for a particular speed
 *
 * This holds calculated values to be written to the I2C controller. Each value
 * is represented as a number of IC clock cycles.
 *
 * @scl_lcnt: Low count value for SCL
 * @scl_hcnt: High count value for SCL
 * @sda_hold: Data hold count
 * @speed_mode: Speed mode being used
 */
struct dw_i2c_speed_config {
	/* SCL high and low period count */
	u16 scl_lcnt;
	u16 scl_hcnt;
	u32 sda_hold;
	enum i2c_speed_mode speed_mode;
};

/**
 * struct dw_i2c - private information for the bus
 *
 * @regs: Registers pointer
 * @scl_sda_cfg: Deprecated information for x86 (should move to device tree)
 * @resets: Resets for the I2C controller
 * @scl_rise_time_ns: Configured SCL rise time in nanoseconds
 * @scl_fall_time_ns: Configured SCL fall time in nanoseconds
 * @sda_hold_time_ns: Configured SDA hold time in nanoseconds
 * @has_spk_cnt: true if the spike-count register is present
 * @clk: Clock input to the I2C controller
 */
struct dw_i2c {
	struct i2c_regs *regs;
	struct dw_scl_sda_cfg *scl_sda_cfg;
	struct reset_ctl_bulk resets;
	u32 scl_rise_time_ns;
	u32 scl_fall_time_ns;
	u32 sda_hold_time_ns;
	bool has_spk_cnt;
#if CONFIG_IS_ENABLED(CLK)
	struct clk clk;
#endif
	struct dw_i2c_speed_config config;
};

extern const struct dm_i2c_ops designware_i2c_ops;

int designware_i2c_probe(struct udevice *bus);
int designware_i2c_remove(struct udevice *dev);
int designware_i2c_of_to_plat(struct udevice *bus);

/**
 * dw_i2c_gen_speed_config() - Calculate config info from requested speed
 *
 * Calculate the speed config from the given @speed_hz and return it so that
 * it can be incorporated in ACPI tables
 *
 * @dev: I2C bus to check
 * @speed_hz: Requested speed in Hz
 * @config: Returns config to use for that speed
 * Return: 0 if OK, -ve on error
 */
int dw_i2c_gen_speed_config(const struct udevice *dev, int speed_hz,
			    struct dw_i2c_speed_config *config);

#endif /* __DW_I2C_H_ */

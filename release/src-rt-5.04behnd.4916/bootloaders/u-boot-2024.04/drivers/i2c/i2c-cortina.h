/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2019
 * Cortina Access, <www.cortina-access.com>
 */

#ifndef __CA_I2C_H_
#define __CA_I2C_H_

#include <linux/bitops.h>
#include <linux/delay.h>

#if !defined(__ASSEMBLER__) && !defined(__ASSEMBLY__)
struct i2c_regs {
	u32 i2c_cfg;
	u32 i2c_ctrl;
	u32 i2c_txr;
	u32 i2c_rxr;
	u32 i2c_ack;
	u32 i2c_ie0;
	u32 i2c_int0;
	u32 i2c_ie1;
	u32 i2c_int1;
	u32 i2c_stat;
};

union ca_biw_cfg {
	struct biw_cfg {
		u32 core_en		: 1;
		u32 biw_soft_reset	: 1;
		u32 busywait_en		: 1;
		u32 stretch_en		: 1;
		u32 arb_en		: 1;
		u32 clksync_en		: 1;
		u32 rsrvd1		: 2;
		u32 spike_cnt		: 4;
		u32 rsrvd2		: 4;
		u32 prer		: 16;
	} bf;
	unsigned int wrd;
};

union ca_biw_ctrl {
	struct biw_ctrl {
		u32 biwdone	: 1;
		u32 rsrvd1	: 2;
		u32 ack_in	: 1;
		u32 write	: 1;
		u32 read	: 1;
		u32 stop	: 1;
		u32 start	: 1;
		u32 rsrvd2	: 24;
	} bf;
	unsigned int wrd;
};

union ca_biw_ack {
	struct biw_ack {
		u32 al		:1;
		u32 biw_busy	:1;
		u32 ack_out	:1;
		u32 rsrvd1	:29;
	} bf;
	unsigned int wrd;
};
#endif /* !__ASSEMBLER__*/

struct ca_i2c {
	struct i2c_regs *regs;
	unsigned int speed;
};

#define I2C_CMD_WT			0
#define I2C_CMD_RD			1

#define BIW_CTRL_DONE		BIT(0)
#define BIW_CTRL_ACK_IN		BIT(3)
#define BIW_CTRL_WRITE		BIT(4)
#define BIW_CTRL_READ		BIT(5)
#define BIW_CTRL_STOP		BIT(6)
#define BIW_CTRL_START		BIT(7)

#define I2C_BYTE_TO		(CONFIG_SYS_HZ / 500)
#define I2C_STOPDET_TO		(CONFIG_SYS_HZ / 500)
#define I2C_BYTE_TO_BB		(10)

#endif							/* __CA_I2C_H_ */

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 */

#ifndef __GDSYS_FPGA_H
#define __GDSYS_FPGA_H

#ifdef CONFIG_GDSYS_LEGACY_DRIVERS
int init_func_fpga(void);

enum {
	FPGA_STATE_DONE_FAILED = 1 << 0,
	FPGA_STATE_REFLECTION_FAILED = 1 << 1,
	FPGA_STATE_PLATFORM = 1 << 2,
};

int get_fpga_state(unsigned dev);

int fpga_set_reg(u32 fpga, u16 *reg, off_t regoff, u16 data);
int fpga_get_reg(u32 fpga, u16 *reg, off_t regoff, u16 *data);

extern struct ihs_fpga *fpga_ptr[];

#define FPGA_SET_REG(ix, fld, val) \
	fpga_set_reg((ix), \
		     &fpga_ptr[ix]->fld, \
		     offsetof(struct ihs_fpga, fld), \
		     val)

#define FPGA_GET_REG(ix, fld, val) \
	fpga_get_reg((ix), \
		     &fpga_ptr[ix]->fld, \
		     offsetof(struct ihs_fpga, fld), \
		     val)
#endif

struct ihs_gpio {
	u16 read;
	u16 clear;
	u16 set;
};

struct ihs_i2c {
	u16 interrupt_status;
	u16 interrupt_enable;
	u16 write_mailbox_ext;
	u16 write_mailbox;
	u16 read_mailbox_ext;
	u16 read_mailbox;
};

struct ihs_osd {
	u16 version;
	u16 features;
	u16 control;
	u16 xy_size;
	u16 xy_scale;
	u16 x_pos;
	u16 y_pos;
};

struct ihs_mdio {
	u16 control;
	u16 address_data;
	u16 rx_data;
};

struct ihs_io_ep {
	u16 transmit_data;
	u16 rx_tx_control;
	u16 receive_data;
	u16 rx_tx_status;
	u16 reserved;
	u16 device_address;
	u16 target_address;
};

#ifdef CONFIG_NEO
struct ihs_fpga {
	u16 reflection_low;	/* 0x0000 */
	u16 versions;		/* 0x0002 */
	u16 fpga_features;	/* 0x0004 */
	u16 fpga_version;	/* 0x0006 */
	u16 reserved_0[8187];	/* 0x0008 */
	u16 reflection_high;	/* 0x3ffe */
};
#endif

#if defined(CONFIG_TARGET_HRCON) || defined(CONFIG_STRIDER_CON_DP)
struct ihs_fpga {
	u16 reflection_low;	/* 0x0000 */
	u16 versions;		/* 0x0002 */
	u16 fpga_version;	/* 0x0004 */
	u16 fpga_features;	/* 0x0006 */
	u16 reserved_0[1];	/* 0x0008 */
	u16 top_interrupt;	/* 0x000a */
	u16 reserved_1[2];	/* 0x000c */
	u16 control;		/* 0x0010 */
	u16 extended_control;	/* 0x0012 */
	struct ihs_gpio gpio;	/* 0x0014 */
	u16 mpc3w_control;	/* 0x001a */
	u16 reserved_2[2];	/* 0x001c */
	struct ihs_io_ep ep;	/* 0x0020 */
	u16 reserved_3[9];	/* 0x002e */
	struct ihs_i2c i2c0;	/* 0x0040 */
	u16 reserved_4[10];	/* 0x004c */
	u16 mc_int;		/* 0x0060 */
	u16 mc_int_en;		/* 0x0062 */
	u16 mc_status;		/* 0x0064 */
	u16 mc_control;		/* 0x0066 */
	u16 mc_tx_data;		/* 0x0068 */
	u16 mc_tx_address;	/* 0x006a */
	u16 mc_tx_cmd;		/* 0x006c */
	u16 mc_res;		/* 0x006e */
	u16 mc_rx_cmd_status;	/* 0x0070 */
	u16 mc_rx_data;		/* 0x0072 */
	u16 reserved_5[69];	/* 0x0074 */
	u16 reflection_high;	/* 0x00fe */
	struct ihs_osd osd0;	/* 0x0100 */
#ifdef CONFIG_SYS_OSD_DH
	u16 reserved_6[57];	/* 0x010e */
	struct ihs_osd osd1;	/* 0x0180 */
	u16 reserved_7[9];	/* 0x018e */
	struct ihs_i2c i2c1;	/* 0x01a0 */
	u16 reserved_8[1834];	/* 0x01ac */
	u16 videomem0[2048];	/* 0x1000 */
	u16 videomem1[2048];	/* 0x2000 */
#else
	u16 reserved_6[889];	/* 0x010e */
	u16 videomem0[2048];	/* 0x0800 */
#endif
};
#endif

#ifdef CONFIG_STRIDER_CPU
struct ihs_fpga {
	u16 reflection_low;	/* 0x0000 */
	u16 versions;		/* 0x0002 */
	u16 fpga_version;	/* 0x0004 */
	u16 fpga_features;	/* 0x0006 */
	u16 reserved_0[1];	/* 0x0008 */
	u16 top_interrupt;	/* 0x000a */
	u16 reserved_1[3];	/* 0x000c */
	u16 extended_control;	/* 0x0012 */
	struct ihs_gpio gpio;	/* 0x0014 */
	u16 mpc3w_control;	/* 0x001a */
	u16 reserved_2[2];	/* 0x001c */
	struct ihs_io_ep ep;	/* 0x0020 */
	u16 reserved_3[9];	/* 0x002e */
	u16 mc_int;		/* 0x0040 */
	u16 mc_int_en;		/* 0x0042 */
	u16 mc_status;		/* 0x0044 */
	u16 mc_control;		/* 0x0046 */
	u16 mc_tx_data;		/* 0x0048 */
	u16 mc_tx_address;	/* 0x004a */
	u16 mc_tx_cmd;		/* 0x004c */
	u16 mc_res;		/* 0x004e */
	u16 mc_rx_cmd_status;	/* 0x0050 */
	u16 mc_rx_data;		/* 0x0052 */
	u16 reserved_4[62];	/* 0x0054 */
	struct ihs_i2c i2c0;	/* 0x00d0 */
};
#endif

#ifdef CONFIG_STRIDER_CON
struct ihs_fpga {
	u16 reflection_low;	/* 0x0000 */
	u16 versions;		/* 0x0002 */
	u16 fpga_version;	/* 0x0004 */
	u16 fpga_features;	/* 0x0006 */
	u16 reserved_0[1];	/* 0x0008 */
	u16 top_interrupt;	/* 0x000a */
	u16 reserved_1[4];	/* 0x000c */
	struct ihs_gpio gpio;	/* 0x0014 */
	u16 mpc3w_control;	/* 0x001a */
	u16 reserved_2[2];	/* 0x001c */
	struct ihs_io_ep ep;	/* 0x0020 */
	u16 reserved_3[9];	/* 0x002e */
	struct ihs_i2c i2c0;	/* 0x0040 */
	u16 reserved_4[10];	/* 0x004c */
	u16 mc_int;		/* 0x0060 */
	u16 mc_int_en;		/* 0x0062 */
	u16 mc_status;		/* 0x0064 */
	u16 mc_control;		/* 0x0066 */
	u16 mc_tx_data;		/* 0x0068 */
	u16 mc_tx_address;	/* 0x006a */
	u16 mc_tx_cmd;		/* 0x006c */
	u16 mc_res;		/* 0x006e */
	u16 mc_rx_cmd_status;	/* 0x0070 */
	u16 mc_rx_data;		/* 0x0072 */
	u16 reserved_5[70];	/* 0x0074 */
	struct ihs_osd osd0;	/* 0x0100 */
	u16 reserved_6[889];	/* 0x010e */
	u16 videomem0[2048];	/* 0x0800 */
};
#endif

#endif

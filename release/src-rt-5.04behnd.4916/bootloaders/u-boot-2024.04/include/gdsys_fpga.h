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

#endif

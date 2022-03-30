/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2018
 * Mario Six, Guntermann & Drunck GmbH, mario.six@gdsys.cc
 */

/**
 * struct ihs_fpga_regs - IHS FPGA register map structure
 * @reflection_low:	  Lower reflection register
 * @versions:		  PCB versions register
 * @fpga_version:	  FPGA versions register
 * @features:		  FPGA features register
 * @extended_features:	  FPGA extended features register
 * @top_interrupt:	  Top interrupt register
 * @top_interrupt_enable: Top interrupt enable register
 * @status:		  FPGA status register
 * @control:		  FPGA control register
 * @extended_control:	  FPGA extended control register
 */
struct ihs_fpga_regs {
	u16 reflection_low;
	u16 versions;
	u16 fpga_version;
	u16 features;
	u16 extended_features;
	u16 top_interrupt;
	u16 top_interrupt_enable;
	u16 status;
	u16 control;
	u16 extended_control;
};

/**
 * ihs_fpga_set() - Convenience macro to set values in FPGA register map
 * @map:    Register map to set a value in
 * @member: Name of member (described by ihs_fpga_regs) to set
 * @val:    Value to set the member to
 */
#define ihs_fpga_set(map, member, val) \
	regmap_set(map, struct ihs_fpga_regs, member, val)

/**
 * ihs_fpga_get() - Convenience macro to get values from FPGA register map
 * @map:    Register map to read value from
 * @member: Name of member (described by ihs_fpga_regs) to get
 * @valp:   Pointe to variable to receive the value read
 */
#define ihs_fpga_get(map, member, valp) \
	regmap_get(map, struct ihs_fpga_regs, member, valp)

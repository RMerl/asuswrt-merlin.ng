/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * LPC32xx GPIO interface
 *
 * (C) Copyright 2014  DENX Software Engineering GmbH
 * Written-by: Albert ARIBAUD <albert.aribaud@3adev.fr>
 */

/**
 * GPIO Register map for LPC32xx
 */

struct gpio_regs {
	u32 p3_inp_state;
	u32 p3_outp_set;
	u32 p3_outp_clr;
	u32 p3_outp_state;
	/* Watch out! the following are shared between p2 and p3 */
	u32 p2_p3_dir_set;
	u32 p2_p3_dir_clr;
	u32 p2_p3_dir_state;
	/* Now back to 'one register for one port' */
	u32 p2_inp_state;
	u32 p2_outp_set;
	u32 p2_outp_clr;
	u32 reserved1[6];
	u32 p0_inp_state;
	u32 p0_outp_set;
	u32 p0_outp_clr;
	u32 p0_outp_state;
	u32 p0_dir_set;
	u32 p0_dir_clr;
	u32 p0_dir_state;
	u32 reserved2;
	u32 p1_inp_state;
	u32 p1_outp_set;
	u32 p1_outp_clr;
	u32 p1_outp_state;
	u32 p1_dir_set;
	u32 p1_dir_clr;
	u32 p1_dir_state;
};

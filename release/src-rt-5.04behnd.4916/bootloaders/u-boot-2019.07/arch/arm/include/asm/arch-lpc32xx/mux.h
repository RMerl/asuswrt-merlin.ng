/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * LPC32xx MUX interface
 *
 * (C) Copyright 2015  DENX Software Engineering GmbH
 * Written-by: Albert ARIBAUD <albert.aribaud@3adev.fr>
 */

/**
 * MUX register map for LPC32xx
 */

struct mux_regs {
	u32 reserved1[10];
	u32 p2_mux_set;
	u32 p2_mux_clr;
	u32 p2_mux_state;
	u32 reserved2[51];
	u32 p_mux_set;
	u32 p_mux_clr;
	u32 p_mux_state;
	u32 reserved3;
	u32 p3_mux_set;
	u32 p3_mux_clr;
	u32 p3_mux_state;
	u32 reserved4;
	u32 p0_mux_set;
	u32 p0_mux_clr;
	u32 p0_mux_state;
	u32 reserved5;
	u32 p1_mux_set;
	u32 p1_mux_clr;
	u32 p1_mux_state;
};

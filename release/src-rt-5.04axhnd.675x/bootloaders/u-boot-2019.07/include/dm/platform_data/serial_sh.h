/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2014  Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
 * Copyright (c) 2014  Renesas Electronics Corporation
 */

#ifndef __serial_sh_h
#define __serial_sh_h

enum sh_clk_mode {
	INT_CLK,
	EXT_CLK,
};

enum sh_serial_type {
	PORT_SCI,
	PORT_SCIF,
	PORT_SCIFA,
	PORT_SCIFB,
};

/*
 * Information about SCIF port
 *
 * @base:	Register base address
 * @clk:	Input clock rate, used for calculating the baud rate divisor
 * @clk_mode:	Clock mode, set internal (INT) or external (EXT)
 * @type:	Type of SCIF
 */
struct sh_serial_platdata {
	unsigned long base;
	unsigned int clk;
	enum sh_clk_mode clk_mode;
	enum sh_serial_type type;
};
#endif /* __serial_sh_h */

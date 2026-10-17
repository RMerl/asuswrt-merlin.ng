/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef __CLOCK_LEGACY_H
#define __CLOCK_LEGACY_H

int get_clocks(void);
unsigned long get_bus_freq(unsigned long dummy);
int get_serial_clock(void);

/*
 * If we have CONFIG_DYNAMIC_DDR_CLK_FREQ then there will be an
 * implentation of get_board_ddr_clk() somewhere.  Otherwise we have
 * a static value to use now.
 */
#ifdef CONFIG_DYNAMIC_DDR_CLK_FREQ
unsigned long get_board_ddr_clk(void);
#else
#define get_board_ddr_clk()		CONFIG_DDR_CLK_FREQ
#endif

/*
 * If we have CONFIG_DYNAMIC_SYS_CLK_FREQ then there will be an
 * implentation of get_board_sys_clk() somewhere.  Otherwise we have
 * a static value to use now.
 */
#ifdef CONFIG_DYNAMIC_SYS_CLK_FREQ
unsigned long get_board_sys_clk(void);
#else
#define get_board_sys_clk()		CONFIG_SYS_CLK_FREQ
#endif

#endif

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2010-2011 Freescale Semiconductor, Inc.
 */
#ifndef __ICS_CLK_H_
#define __ICS_CLK_H_	1

#ifndef __ASSEMBLY__

extern unsigned long get_board_sys_clk(void);
extern unsigned long get_board_ddr_clk(void);
extern unsigned long ics307_sysclk_calculator(unsigned long out_freq);
#endif

#endif	/* __ICS_CLK_H_ */

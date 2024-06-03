/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002-2013
 * Eric Jarrige <eric.jarrige@armadeus.org>
 *
 * based on the files by
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com
 * and
 * Keith Outwater, keith_outwater@mvis.com
 */
extern void APF27_init_fpga(void);

extern int fpga_pre_fn(int cookie);
extern int fpga_pgm_fn(int assert_pgm, int flush, int cookie);
extern int fpga_cs_fn(int assert_cs, int flush, int cookie);
extern int fpga_init_fn(int cookie);
extern int fpga_done_fn(int cookie);
extern int fpga_clk_fn(int assert_clk, int flush, int cookie);
extern int fpga_wr_fn(int assert_write, int flush, int cookie);
extern int fpga_rdata_fn(unsigned char *data, int cookie);
extern int fpga_wdata_fn(unsigned char data, int flush, int cookie);
extern int fpga_abort_fn(int cookie);
extern int fpga_post_fn(int cookie);
extern int fpga_busy_fn(int cookie);

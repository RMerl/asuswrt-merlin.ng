/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 */

#ifndef _SPARTAN2_H_
#define _SPARTAN2_H_

#include <xilinx.h>

/* Slave Parallel Implementation function table */
typedef struct {
	xilinx_pre_fn	pre;
	xilinx_pgm_fn	pgm;
	xilinx_init_fn	init;
	xilinx_err_fn	err;
	xilinx_done_fn	done;
	xilinx_clk_fn	clk;
	xilinx_cs_fn	cs;
	xilinx_wr_fn	wr;
	xilinx_rdata_fn	rdata;
	xilinx_wdata_fn	wdata;
	xilinx_busy_fn	busy;
	xilinx_abort_fn	abort;
	xilinx_post_fn	post;
} xilinx_spartan2_slave_parallel_fns;

/* Slave Serial Implementation function table */
typedef struct {
	xilinx_pre_fn	pre;
	xilinx_pgm_fn	pgm;
	xilinx_clk_fn	clk;
	xilinx_init_fn	init;
	xilinx_done_fn	done;
	xilinx_wr_fn	wr;
	xilinx_post_fn	post;
} xilinx_spartan2_slave_serial_fns;

#if defined(CONFIG_FPGA_SPARTAN2)
extern struct xilinx_fpga_op spartan2_op;
# define FPGA_SPARTAN2_OPS	&spartan2_op
#else
# define FPGA_SPARTAN2_OPS	NULL
#endif

/* Device Image Sizes
 *********************************************************************/
/* Spartan-II (2.5V) */
#define XILINX_XC2S15_SIZE	197728/8
#define XILINX_XC2S30_SIZE	336800/8
#define XILINX_XC2S50_SIZE	559232/8
#define XILINX_XC2S100_SIZE	781248/8
#define XILINX_XC2S150_SIZE	1040128/8
#define XILINX_XC2S200_SIZE	1335872/8

/* Spartan-IIE (1.8V) */
#define XILINX_XC2S50E_SIZE     630048/8
#define XILINX_XC2S100E_SIZE    863840/8
#define XILINX_XC2S150E_SIZE    1134496/8
#define XILINX_XC2S200E_SIZE    1442016/8
#define XILINX_XC2S300E_SIZE    1875648/8

/* Descriptor Macros
 *********************************************************************/
/* Spartan-II devices */
#define XILINX_XC2S15_DESC(iface, fn_table, cookie) \
{ xilinx_spartan2, iface, XILINX_XC2S15_SIZE, fn_table, cookie, \
	FPGA_SPARTAN2_OPS }

#define XILINX_XC2S30_DESC(iface, fn_table, cookie) \
{ xilinx_spartan2, iface, XILINX_XC2S30_SIZE, fn_table, cookie, \
	FPGA_SPARTAN2_OPS }

#define XILINX_XC2S50_DESC(iface, fn_table, cookie) \
{ xilinx_spartan2, iface, XILINX_XC2S50_SIZE, fn_table, cookie, \
	FPGA_SPARTAN2_OPS }

#define XILINX_XC2S100_DESC(iface, fn_table, cookie) \
{ xilinx_spartan2, iface, XILINX_XC2S100_SIZE, fn_table, cookie, \
	FPGA_SPARTAN2_OPS }

#define XILINX_XC2S150_DESC(iface, fn_table, cookie) \
{ xilinx_spartan2, iface, XILINX_XC2S150_SIZE, fn_table, cookie, \
	FPGA_SPARTAN2_OPS }

#define XILINX_XC2S200_DESC(iface, fn_table, cookie) \
{ xilinx_spartan2, iface, XILINX_XC2S200_SIZE, fn_table, cookie, \
	FPGA_SPARTAN2_OPS }

#define XILINX_XC2S50E_DESC(iface, fn_table, cookie) \
{ xilinx_spartan2, iface, XILINX_XC2S50E_SIZE, fn_table, cookie, \
	FPGA_SPARTAN2_OPS }

#define XILINX_XC2S100E_DESC(iface, fn_table, cookie) \
{ xilinx_spartan2, iface, XILINX_XC2S100E_SIZE, fn_table, cookie, \
	FPGA_SPARTAN2_OPS }

#define XILINX_XC2S150E_DESC(iface, fn_table, cookie) \
{ xilinx_spartan2, iface, XILINX_XC2S150E_SIZE, fn_table, cookie, \
	FPGA_SPARTAN2_OPS }

#define XILINX_XC2S200E_DESC(iface, fn_table, cookie) \
{ xilinx_spartan2, iface, XILINX_XC2S200E_SIZE, fn_table, cookie, \
	FPGA_SPARTAN2_OPS }

#define XILINX_XC2S300E_DESC(iface, fn_table, cookie) \
{ xilinx_spartan2, iface, XILINX_XC2S300E_SIZE, fn_table, cookie, \
	FPGA_SPARTAN2_OPS }

#endif /* _SPARTAN2_H_ */

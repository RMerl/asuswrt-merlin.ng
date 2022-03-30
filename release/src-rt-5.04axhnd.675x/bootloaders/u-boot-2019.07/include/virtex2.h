/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 * Keith Outwater, keith_outwater@mvis.com
 */

#ifndef _VIRTEX2_H_
#define _VIRTEX2_H_

#include <xilinx.h>

/*
 * Slave SelectMap Implementation function table.
 */
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
} xilinx_virtex2_slave_selectmap_fns;

/* Slave Serial Implementation function table */
typedef struct {
	xilinx_pgm_fn	pgm;
	xilinx_clk_fn	clk;
	xilinx_rdata_fn	rdata;
	xilinx_wdata_fn	wdata;
} xilinx_virtex2_slave_serial_fns;

#if defined(CONFIG_FPGA_VIRTEX2)
extern struct xilinx_fpga_op virtex2_op;
# define FPGA_VIRTEX2_OPS	&virtex2_op
#else
# define FPGA_VIRTEX2_OPS	NULL
#endif

/* Device Image Sizes (in bytes)
 *********************************************************************/
#define XILINX_XC2V40_SIZE	(338208 / 8)
#define XILINX_XC2V80_SIZE	(597408 / 8)
#define XILINX_XC2V250_SIZE	(1591584 / 8)
#define XILINX_XC2V500_SIZE	(2557857 / 8)
#define XILINX_XC2V1000_SIZE	(3749408 / 8)
#define XILINX_XC2V1500_SIZE	(5166240 / 8)
#define XILINX_XC2V2000_SIZE	(6808352 / 8)
#define XILINX_XC2V3000_SIZE	(9589408 / 8)
#define XILINX_XC2V4000_SIZE	(14220192 / 8)
#define XILINX_XC2V6000_SIZE	(19752096 / 8)
#define XILINX_XC2V8000_SIZE	(26185120 / 8)
#define XILINX_XC2V10000_SIZE	(33519264 / 8)

/* Descriptor Macros
 *********************************************************************/
#define XILINX_XC2V40_DESC(iface, fn_table, cookie)	\
{ xilinx_virtex2, iface, XILINX_XC2V40_SIZE, fn_table, cookie, \
	FPGA_VIRTEX2_OPS }

#define XILINX_XC2V80_DESC(iface, fn_table, cookie) \
{ xilinx_virtex2, iface, XILINX_XC2V80_SIZE, fn_table, cookie, \
	FPGA_VIRTEX2_OPS }

#define XILINX_XC2V250_DESC(iface, fn_table, cookie) \
{ xilinx_virtex2, iface, XILINX_XC2V250_SIZE, fn_table, cookie, \
	FPGA_VIRTEX2_OPS }

#define XILINX_XC2V500_DESC(iface, fn_table, cookie) \
{ xilinx_virtex2, iface, XILINX_XC2V500_SIZE, fn_table, cookie, \
	FPGA_VIRTEX2_OPS }

#define XILINX_XC2V1000_DESC(iface, fn_table, cookie) \
{ xilinx_virtex2, iface, XILINX_XC2V1000_SIZE, fn_table, cookie, \
	FPGA_VIRTEX2_OPS }

#define XILINX_XC2V1500_DESC(iface, fn_table, cookie) \
{ xilinx_virtex2, iface, XILINX_XC2V1500_SIZE, fn_table, cookie, \
	FPGA_VIRTEX2_OPS }

#define XILINX_XC2V2000_DESC(iface, fn_table, cookie) \
{ xilinx_virtex2, iface, XILINX_XC2V2000_SIZE, fn_table, cookie, \
	FPGA_VIRTEX2_OPS }

#define XILINX_XC2V3000_DESC(iface, fn_table, cookie) \
{ xilinx_virtex2, iface, XILINX_XC2V3000_SIZE, fn_table, cookie, \
	FPGA_VIRTEX2_OPS }

#define XILINX_XC2V4000_DESC(iface, fn_table, cookie) \
{ xilinx_virtex2, iface, XILINX_XC2V4000_SIZE, fn_table, cookie, \
	FPGA_VIRTEX2_OPS }

#define XILINX_XC2V6000_DESC(iface, fn_table, cookie) \
{ xilinx_virtex2, iface, XILINX_XC2V6000_SIZE, fn_table, cookie, \
	FPGA_VIRTEX2_OPS }

#define XILINX_XC2V8000_DESC(iface, fn_table, cookie) \
{ xilinx_virtex2, iface, XILINX_XC2V8000_SIZE, fn_table, cookie, \
	FPGA_VIRTEX2_OPS }

#define XILINX_XC2V10000_DESC(iface, fn_table, cookie) \
{ xilinx_virtex2, iface, XILINX_XC2V10000_SIZE, fn_table, cookie, \
	FPGA_VIRTEX2_OPS }

#endif /* _VIRTEX2_H_ */

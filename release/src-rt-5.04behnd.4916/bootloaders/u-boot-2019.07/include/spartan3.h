/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 */

#ifndef _SPARTAN3_H_
#define _SPARTAN3_H_

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
} xilinx_spartan3_slave_parallel_fns;

/* Slave Serial Implementation function table */
typedef struct {
	xilinx_pre_fn	pre;
	xilinx_pgm_fn	pgm;
	xilinx_clk_fn	clk;
	xilinx_init_fn	init;
	xilinx_done_fn	done;
	xilinx_wr_fn	wr;
	xilinx_post_fn	post;
	xilinx_bwr_fn	bwr; /* block write function */
	xilinx_abort_fn abort;
} xilinx_spartan3_slave_serial_fns;

#if defined(CONFIG_FPGA_SPARTAN3)
extern struct xilinx_fpga_op spartan3_op;
# define FPGA_SPARTAN3_OPS	&spartan3_op
#else
# define FPGA_SPARTAN3_OPS	NULL
#endif

/* Device Image Sizes
 *********************************************************************/
/* Spartan-III (1.2V) */
#define XILINX_XC3S50_SIZE	439264/8
#define XILINX_XC3S200_SIZE	1047616/8
#define XILINX_XC3S400_SIZE	1699136/8
#define XILINX_XC3S1000_SIZE	3223488/8
#define XILINX_XC3S1500_SIZE	5214784/8
#define XILINX_XC3S2000_SIZE	7673024/8
#define XILINX_XC3S4000_SIZE	11316864/8
#define XILINX_XC3S5000_SIZE	13271936/8

/* Spartan-3E (v3.4) */
#define	XILINX_XC3S100E_SIZE	581344/8
#define	XILINX_XC3S250E_SIZE	1353728/8
#define	XILINX_XC3S500E_SIZE	2270208/8
#define	XILINX_XC3S1200E_SIZE	3841184/8
#define	XILINX_XC3S1600E_SIZE	5969696/8

/*
 * Spartan-6 : the Spartan-6 family can be programmed
 * exactly as the Spartan-3
 */
#define XILINK_XC6SLX4_SIZE	(3713568/8)

/* Descriptor Macros
 *********************************************************************/
/* Spartan-III devices */
#define XILINX_XC3S50_DESC(iface, fn_table, cookie) \
{ xilinx_spartan3, iface, XILINX_XC3S50_SIZE, fn_table, cookie, \
	FPGA_SPARTAN3_OPS }

#define XILINX_XC3S200_DESC(iface, fn_table, cookie) \
{ xilinx_spartan3, iface, XILINX_XC3S200_SIZE, fn_table, cookie, \
	FPGA_SPARTAN3_OPS }

#define XILINX_XC3S400_DESC(iface, fn_table, cookie) \
{ xilinx_spartan3, iface, XILINX_XC3S400_SIZE, fn_table, cookie, \
	FPGA_SPARTAN3_OPS }

#define XILINX_XC3S1000_DESC(iface, fn_table, cookie) \
{ xilinx_spartan3, iface, XILINX_XC3S1000_SIZE, fn_table, cookie, \
	FPGA_SPARTAN3_OPS }

#define XILINX_XC3S1500_DESC(iface, fn_table, cookie) \
{ xilinx_spartan3, iface, XILINX_XC3S1500_SIZE, fn_table, cookie, \
	FPGA_SPARTAN3_OPS }

#define XILINX_XC3S2000_DESC(iface, fn_table, cookie) \
{ xilinx_spartan3, iface, XILINX_XC3S2000_SIZE, fn_table, cookie, \
	FPGA_SPARTAN3_OPS }

#define XILINX_XC3S4000_DESC(iface, fn_table, cookie) \
{ xilinx_spartan3, iface, XILINX_XC3S4000_SIZE, fn_table, cookie, \
	FPGA_SPARTAN3_OPS }

#define XILINX_XC3S5000_DESC(iface, fn_table, cookie) \
{ xilinx_spartan3, iface, XILINX_XC3S5000_SIZE, fn_table, cookie, \
	FPGA_SPARTAN3_OPS }

/* Spartan-3E devices */
#define XILINX_XC3S100E_DESC(iface, fn_table, cookie) \
{ xilinx_spartan3, iface, XILINX_XC3S100E_SIZE, fn_table, cookie, \
	FPGA_SPARTAN3_OPS }

#define XILINX_XC3S250E_DESC(iface, fn_table, cookie) \
{ xilinx_spartan3, iface, XILINX_XC3S250E_SIZE, fn_table, cookie, \
	FPGA_SPARTAN3_OPS }

#define XILINX_XC3S500E_DESC(iface, fn_table, cookie) \
{ xilinx_spartan3, iface, XILINX_XC3S500E_SIZE, fn_table, cookie, \
	FPGA_SPARTAN3_OPS }

#define XILINX_XC3S1200E_DESC(iface, fn_table, cookie) \
{ xilinx_spartan3, iface, XILINX_XC3S1200E_SIZE, fn_table, cookie, \
	FPGA_SPARTAN3_OPS }

#define XILINX_XC3S1600E_DESC(iface, fn_table, cookie) \
{ xilinx_spartan3, iface, XILINX_XC3S1600E_SIZE, fn_table, cookie, \
	FPGA_SPARTAN3_OPS }

#define XILINX_XC6SLX4_DESC(iface, fn_table, cookie) \
{ xilinx_spartan3, iface, XILINK_XC6SLX4_SIZE, fn_table, cookie, \
	FPGA_SPARTAN3_OPS }

#endif /* _SPARTAN3_H_ */

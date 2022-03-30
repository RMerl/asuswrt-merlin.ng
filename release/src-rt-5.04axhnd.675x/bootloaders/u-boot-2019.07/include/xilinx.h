/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 */

#include <fpga.h>

#ifndef _XILINX_H_
#define _XILINX_H_

/* Xilinx types
 *********************************************************************/
typedef enum {			/* typedef xilinx_iface */
	min_xilinx_iface_type,	/* low range check value */
	slave_serial,		/* serial data and external clock */
	master_serial,		/* serial data w/ internal clock (not used) */
	slave_parallel,		/* parallel data w/ external latch */
	jtag_mode,		/* jtag/tap serial (not used ) */
	master_selectmap,	/* master SelectMap (virtex2)           */
	slave_selectmap,	/* slave SelectMap (virtex2)            */
	devcfg,			/* devcfg interface (zynq) */
	csu_dma,		/* csu_dma interface (zynqmp) */
	max_xilinx_iface_type	/* insert all new types before this */
} xilinx_iface;			/* end, typedef xilinx_iface */

typedef enum {			/* typedef xilinx_family */
	min_xilinx_type,	/* low range check value */
	xilinx_spartan2,	/* Spartan-II Family */
	xilinx_virtexE,		/* Virtex-E Family */
	xilinx_virtex2,		/* Virtex2 Family */
	xilinx_spartan3,	/* Spartan-III Family */
	xilinx_zynq,		/* Zynq Family */
	xilinx_zynqmp,		/* ZynqMP Family */
	max_xilinx_type		/* insert all new types before this */
} xilinx_family;		/* end, typedef xilinx_family */

typedef struct {		/* typedef xilinx_desc */
	xilinx_family family;	/* part type */
	xilinx_iface iface;	/* interface type */
	size_t size;		/* bytes of data part can accept */
	void *iface_fns;	/* interface function table */
	int cookie;		/* implementation specific cookie */
	struct xilinx_fpga_op *operations; /* operations */
	char *name;		/* device name in bitstream */
} xilinx_desc;			/* end, typedef xilinx_desc */

struct xilinx_fpga_op {
	int (*load)(xilinx_desc *, const void *, size_t, bitstream_type);
	int (*loadfs)(xilinx_desc *, const void *, size_t, fpga_fs_info *);
	int (*loads)(xilinx_desc *desc, const void *buf, size_t bsize,
		     struct fpga_secure_info *fpga_sec_info);
	int (*dump)(xilinx_desc *, const void *, size_t);
	int (*info)(xilinx_desc *);
};

/* Generic Xilinx Functions
 *********************************************************************/
int xilinx_load(xilinx_desc *desc, const void *image, size_t size,
		bitstream_type bstype);
int xilinx_dump(xilinx_desc *desc, const void *buf, size_t bsize);
int xilinx_info(xilinx_desc *desc);
int xilinx_loadfs(xilinx_desc *desc, const void *buf, size_t bsize,
		  fpga_fs_info *fpga_fsinfo);
int xilinx_loads(xilinx_desc *desc, const void *buf, size_t bsize,
		 struct fpga_secure_info *fpga_sec_info);

/* Board specific implementation specific function types
 *********************************************************************/
typedef int (*xilinx_pgm_fn)(int assert_pgm, int flush, int cookie);
typedef int (*xilinx_init_fn)(int cookie);
typedef int (*xilinx_err_fn)(int cookie);
typedef int (*xilinx_done_fn)(int cookie);
typedef int (*xilinx_clk_fn)(int assert_clk, int flush, int cookie);
typedef int (*xilinx_cs_fn)(int assert_cs, int flush, int cookie);
typedef int (*xilinx_wr_fn)(int assert_write, int flush, int cookie);
typedef int (*xilinx_rdata_fn)(unsigned char *data, int cookie);
typedef int (*xilinx_wdata_fn)(unsigned char data, int flush, int cookie);
typedef int (*xilinx_busy_fn)(int cookie);
typedef int (*xilinx_abort_fn)(int cookie);
typedef int (*xilinx_pre_fn)(int cookie);
typedef int (*xilinx_post_fn)(int cookie);
typedef int (*xilinx_bwr_fn)(void *buf, size_t len, int flush, int cookie);

#endif  /* _XILINX_H_ */

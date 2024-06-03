/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  Copyright (C) 2013 Altera Corporation <www.altera.com>
 */

#ifndef	_SCAN_MANAGER_H_
#define	_SCAN_MANAGER_H_

struct socfpga_scan_manager {
	u32	stat;
	u32	en;
	u32	padding[2];
	u32	fifo_single_byte;
	u32	fifo_double_byte;
	u32	fifo_triple_byte;
	u32	fifo_quad_byte;
};

int scan_mgr_configure_iocsr(void);
u32 scan_mgr_get_fpga_id(void);
int iocsr_get_config_table(const unsigned int chain_id,
			   const unsigned long **table,
			   unsigned int *table_len);

#endif /* _SCAN_MANAGER_H_ */

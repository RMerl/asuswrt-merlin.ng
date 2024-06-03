/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _SCU_H_
#define _SCU_H_

/* ARM Snoop Control Unit (SCU) registers */
struct scu_ctlr {
	uint scu_ctrl;		/* SCU Control Register, offset 00 */
	uint scu_cfg;		/* SCU Config Register, offset 04 */
	uint scu_cpu_pwr_stat;	/* SCU CPU Power Status Register, offset 08 */
	uint scu_inv_all;	/* SCU Invalidate All Register, offset 0C */
	uint scu_reserved0[12];	/* reserved, offset 10-3C */
	uint scu_filt_start;	/* SCU Filtering Start Address Reg, offset 40 */
	uint scu_filt_end;	/* SCU Filtering End Address Reg, offset 44 */
	uint scu_reserved1[2];	/* reserved, offset 48-4C */
	uint scu_acc_ctl;	/* SCU Access Control Register, offset 50 */
	uint scu_ns_acc_ctl;	/* SCU Non-secure Access Cntrl Reg, offset 54 */
};

#define SCU_CTRL_ENABLE		(1 << 0)

#endif	/* SCU_H */

/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright (C) 2012-2017 Altera Corporation <www.altera.com>
 * All rights reserved.
 */

#ifndef	_FPGA_MANAGER_H_
#define	_FPGA_MANAGER_H_

#include <altera.h>

#if defined(CONFIG_TARGET_SOCFPGA_GEN5)
#include <asm/arch/fpga_manager_gen5.h>
#elif defined(CONFIG_TARGET_SOCFPGA_ARRIA10)
#include <asm/arch/fpga_manager_arria10.h>
#endif

/* FPGA CD Ratio Value */
#define CDRATIO_x1				0x0
#define CDRATIO_x2				0x1
#define CDRATIO_x4				0x2
#define CDRATIO_x8				0x3

#ifndef __ASSEMBLY__

/* Common prototypes */
int fpgamgr_get_mode(void);
int fpgamgr_poll_fpga_ready(void);
void fpgamgr_program_write(const void *rbf_data, size_t rbf_size);
int fpgamgr_test_fpga_ready(void);
int fpgamgr_dclkcnt_set(unsigned long cnt);

#endif /* __ASSEMBLY__ */
#endif /* _FPGA_MANAGER_H_ */

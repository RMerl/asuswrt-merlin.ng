/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2015 Xilinx, Inc,
 * Michal Simek <michal.simek@amd.com>
 */

#ifndef _ZYNQMPPL_H_
#define _ZYNQMPPL_H_

#include <xilinx.h>
#include <linux/bitops.h>

#define ZYNQMP_FPGA_OP_INIT			(1 << 0)
#define ZYNQMP_FPGA_OP_LOAD			(1 << 1)
#define ZYNQMP_FPGA_OP_DONE			(1 << 2)

#define ZYNQMP_FPGA_FLAG_AUTHENTICATED		BIT(2)
#define ZYNQMP_FPGA_FLAG_ENCRYPTED		BIT(3)

#define ZYNQMP_CSU_IDCODE_DEVICE_CODE_SHIFT	15
#define ZYNQMP_CSU_IDCODE_DEVICE_CODE_MASK	(0xf << \
					ZYNQMP_CSU_IDCODE_DEVICE_CODE_SHIFT)
#define ZYNQMP_CSU_IDCODE_SVD_SHIFT	12
#define ZYNQMP_CSU_IDCODE_SVD_MASK	(0x7 << ZYNQMP_CSU_IDCODE_SVD_SHIFT)

extern struct xilinx_fpga_op zynqmp_op;

#if CONFIG_IS_ENABLED(FPGA_LOAD_SECURE)
#define ZYNQMP_FPGA_FLAGS	(FPGA_LEGACY | \
				 FPGA_XILINX_ZYNQMP_DDRAUTH | \
				 FPGA_XILINX_ZYNQMP_ENC)
#else
#define ZYNQMP_FPGA_FLAGS	(FPGA_LEGACY)
#endif

#endif /* _ZYNQMPPL_H_ */

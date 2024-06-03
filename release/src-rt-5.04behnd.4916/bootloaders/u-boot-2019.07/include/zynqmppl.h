/* SPDX-License-Identifier: GPL-2.0 */
/*
 * (C) Copyright 2015 Xilinx, Inc,
 * Michal Simek <michal.simek@xilinx.com>
 */

#ifndef _ZYNQMPPL_H_
#define _ZYNQMPPL_H_

#include <xilinx.h>

#define ZYNQMP_SIP_SVC_CSU_DMA_CHIPID		0xC2000018
#define ZYNQMP_SIP_SVC_PM_FPGA_LOAD		0xC2000016
#define ZYNQMP_SIP_SVC_PM_FPGA_STATUS		0xC2000017
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

#define XILINX_ZYNQMP_DESC \
{ xilinx_zynqmp, csu_dma, 1, &zynqmp_op, 0, &zynqmp_op }

#endif /* _ZYNQMPPL_H_ */

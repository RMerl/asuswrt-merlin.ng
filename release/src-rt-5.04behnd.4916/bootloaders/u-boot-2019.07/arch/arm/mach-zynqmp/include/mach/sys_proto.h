/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 */

#ifndef _ASM_ARCH_SYS_PROTO_H
#define _ASM_ARCH_SYS_PROTO_H

#define PAYLOAD_ARG_CNT		5

#define ZYNQMP_CSU_SILICON_VER_MASK	0xF
#define ZYNQMP_SIP_SVC_PM_SECURE_IMG_LOAD	0xC200002D
#define KEY_PTR_LEN	32

#define ZYNQMP_FPGA_BIT_AUTH_DDR	1
#define ZYNQMP_FPGA_BIT_AUTH_OCM	2
#define ZYNQMP_FPGA_BIT_ENC_USR_KEY	3
#define ZYNQMP_FPGA_BIT_ENC_DEV_KEY	4
#define ZYNQMP_FPGA_BIT_NS		5

#define ZYNQMP_FPGA_AUTH_DDR	1

#define ZYNQMP_SIP_SVC_GET_API_VERSION		0xC2000001

#define ZYNQMP_PM_VERSION_MAJOR		1
#define ZYNQMP_PM_VERSION_MINOR		0
#define ZYNQMP_PM_VERSION_MAJOR_SHIFT	16
#define ZYNQMP_PM_VERSION_MINOR_MASK	0xFFFF

#define ZYNQMP_PM_VERSION	\
	((ZYNQMP_PM_VERSION_MAJOR << ZYNQMP_PM_VERSION_MAJOR_SHIFT) | \
				 ZYNQMP_PM_VERSION_MINOR)

#define ZYNQMP_PM_VERSION_INVALID	~0

#define PMUFW_V1_0	((1 << ZYNQMP_PM_VERSION_MAJOR_SHIFT) | 0)

enum {
	IDCODE,
	VERSION,
	IDCODE2,
};

enum {
	ZYNQMP_SILICON_V1,
	ZYNQMP_SILICON_V2,
	ZYNQMP_SILICON_V3,
	ZYNQMP_SILICON_V4,
};

enum {
	TCM_LOCK,
	TCM_SPLIT,
};

int zynq_board_read_rom_ethaddr(unsigned char *ethaddr);
unsigned int zynqmp_get_silicon_version(void);

void handoff_setup(void);

unsigned int zynqmp_pmufw_version(void);
int zynqmp_mmio_write(const u32 address, const u32 mask, const u32 value);
int zynqmp_mmio_read(const u32 address, u32 *value);
int invoke_smc(u32 pm_api_id, u32 arg0, u32 arg1, u32 arg2, u32 arg3,
	       u32 *ret_payload);

void initialize_tcm(bool mode);
void mem_map_fill(void);
int chip_id(unsigned char id);
#if defined(CONFIG_SYS_MEM_RSVD_FOR_MMU) || defined(CONFIG_DEFINE_TCM_OCM_MMAP)
void tcm_init(u8 mode);
#endif

#endif /* _ASM_ARCH_SYS_PROTO_H */

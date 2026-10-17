/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@amd.com>
 */

#ifndef _ASM_ARCH_SYS_PROTO_H
#define _ASM_ARCH_SYS_PROTO_H

#define ZYNQMP_CSU_SILICON_VER_MASK	0xF
#define KEY_PTR_LEN	32
#define IV_SIZE		12
#define RSA_KEY_SIZE	512
#define MODULUS_LEN	512
#define PRIV_EXPO_LEN	512
#define PUB_EXPO_LEN	4

#define ZYNQMP_SHA3_INIT	1
#define ZYNQMP_SHA3_UPDATE	2
#define ZYNQMP_SHA3_FINAL	4
#define ZYNQMP_SHA3_SIZE	48

#define ZYNQMP_FPGA_BIT_AUTH_DDR	1
#define ZYNQMP_FPGA_BIT_AUTH_OCM	2
#define ZYNQMP_FPGA_BIT_ENC_USR_KEY	3
#define ZYNQMP_FPGA_BIT_ENC_DEV_KEY	4
#define ZYNQMP_FPGA_BIT_NS		5

#define ZYNQMP_FPGA_AUTH_DDR	1

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

unsigned int zynqmp_get_silicon_version(void);

void initialize_tcm(bool mode);
void mem_map_fill(void);
#if defined(CONFIG_SYS_MEM_RSVD_FOR_MMU) || defined(CONFIG_DEFINE_TCM_OCM_MMAP)
void tcm_init(u8 mode);
#endif

#endif /* _ASM_ARCH_SYS_PROTO_H */

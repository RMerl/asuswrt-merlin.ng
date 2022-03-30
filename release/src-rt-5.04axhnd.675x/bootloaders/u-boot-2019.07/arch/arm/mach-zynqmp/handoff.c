// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 - 2017 Xilinx, Inc.
 *
 * Michal Simek <michal.simek@xilinx.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>

/*
 * atfhandoffparams
 * Parameter	bitfield	encoding
 * -----------------------------------------------------------------------------
 * Exec State	0	0 -> Aarch64, 1-> Aarch32
 * endianness	1	0 -> LE, 1 -> BE
 * secure (TZ)	2	0 -> Non secure, 1 -> secure
 * EL		3:4	00 -> EL0, 01 -> EL1, 10 -> EL2, 11 -> EL3
 * CPU#		5:6	00 -> A53_0, 01 -> A53_1, 10 -> A53_2, 11 -> A53_3
 */

#define FSBL_FLAGS_ESTATE_SHIFT		0
#define FSBL_FLAGS_ESTATE_MASK		(1 << FSBL_FLAGS_ESTATE_SHIFT)
#define FSBL_FLAGS_ESTATE_A64		0
#define FSBL_FLAGS_ESTATE_A32		1

#define FSBL_FLAGS_ENDIAN_SHIFT		1
#define FSBL_FLAGS_ENDIAN_MASK		(1 << FSBL_FLAGS_ENDIAN_SHIFT)
#define FSBL_FLAGS_ENDIAN_LE		0
#define FSBL_FLAGS_ENDIAN_BE		1

#define FSBL_FLAGS_TZ_SHIFT		2
#define FSBL_FLAGS_TZ_MASK		(1 << FSBL_FLAGS_TZ_SHIFT)
#define FSBL_FLAGS_NON_SECURE		0
#define FSBL_FLAGS_SECURE		1

#define FSBL_FLAGS_EL_SHIFT		3
#define FSBL_FLAGS_EL_MASK		(3 << FSBL_FLAGS_EL_SHIFT)
#define FSBL_FLAGS_EL0			0
#define FSBL_FLAGS_EL1			1
#define FSBL_FLAGS_EL2			2
#define FSBL_FLAGS_EL3			3

#define FSBL_FLAGS_CPU_SHIFT		5
#define FSBL_FLAGS_CPU_MASK		(3 << FSBL_FLAGS_CPU_SHIFT)
#define FSBL_FLAGS_A53_0		0
#define FSBL_FLAGS_A53_1		1
#define FSBL_FLAGS_A53_2		2
#define FSBL_FLAGS_A53_3		3

#define FSBL_MAX_PARTITIONS		8

/* Structure corresponding to each partition entry */
struct xfsbl_partition {
	uint64_t entry_point;
	uint64_t flags;
};

/* Structure for handoff parameters to ARM Trusted Firmware (ATF) */
struct xfsbl_atf_handoff_params {
	uint8_t magic[4];
	uint32_t num_entries;
	struct xfsbl_partition partition[FSBL_MAX_PARTITIONS];
};

#ifdef CONFIG_SPL_OS_BOOT
void handoff_setup(void)
{
	struct xfsbl_atf_handoff_params *atfhandoffparams;

	atfhandoffparams = (void *)CONFIG_SPL_TEXT_BASE;
	atfhandoffparams->magic[0] = 'X';
	atfhandoffparams->magic[1] = 'L';
	atfhandoffparams->magic[2] = 'N';
	atfhandoffparams->magic[3] = 'X';

	atfhandoffparams->num_entries = 1;
	atfhandoffparams->partition[0].entry_point = CONFIG_SYS_TEXT_BASE;
	atfhandoffparams->partition[0].flags = FSBL_FLAGS_EL2 <<
					       FSBL_FLAGS_EL_SHIFT;

	writel(CONFIG_SPL_TEXT_BASE, &pmu_base->gen_storage6);
}
#endif

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2020 Broadcom
 */

#ifndef DT_BINDINGS_BCM_NS3_MC_H
#define DT_BINDINGS_BCM_NS3_MC_H

/*
 * +--------+----------+ 0x8b000000
 * | NITRO CRASH DUMP  |  32MB
 * +--------+----------+ 0x8d000000
 * | SHMEM (NS)         | 16 MB
 * +-------------------+ 0x8e000000
 * |        | TEE_RAM(S)| 4MB
 * + TZDRAM +----------+ 0x8e400000
 * |        | TA_RAM(S) | 12MB
 * +--------+----------+ 0x8f000000
 * | BL31 + TMON + LPM  |
 * | memory             | 1MB
 * +-------------------+ 0x8f100000
 */

#define BCM_NS3_MEM_NITRO_CRASH_START  0x8ae00000
#define BCM_NS3_MEM_NITRO_CRASH_LEN    0x21fffff
#define BCM_NS3_MEM_NITRO_CRASH_SIZE   0x2200000

#define BCM_NS3_MEM_SHARE_START    0x8d000000
#define BCM_NS3_MEM_SHARE_LEN      0x020fffff

/* ATF/U-Boot/Linux error logs */
#define BCM_NS3_MEM_ELOG_START     0x8f113000
#define BCM_NS3_MEM_ELOG_LEN       0x00100000

/* CRMU Page table memroy */
#define BCM_NS3_MEM_CRMU_PT_START  0x880000000
#define BCM_NS3_MEM_CRMU_PT_LEN    0x200000

/* default memory starting address and length */
#define BCM_NS3_MEM_START          0x80000000UL
#define BCM_NS3_MEM_LEN            0x80000000UL
#define BCM_NS3_MEM_END            (BCM_NS3_MEM_START + BCM_NS3_MEM_LEN)

/* memory starting address and length for BANK_1 */
#define BCM_NS3_BANK_1_MEM_START   0x880000000UL
#define BCM_NS3_BANK_1_MEM_LEN     0x180000000UL

/* memory layout information */
#define BCM_NS3_DDR_INFO_BASE      0x8f220000
#define BCM_NS3_DDR_INFO_RSVD_LEN  0x1000
#define BCM_NS3_DDR_INFO_LEN       73
#define BCM_NS3_DDR_INFO_SIG       0x42434d44
#define BCM_NS3_MAX_NR_BANKS       4

#define BCM_NS3_GIC_LPI_BASE      0x8ad70000
#define BCM_NS3_MEM_RSVE_START    BCM_NS3_GIC_LPI_BASE
#define BCM_NS3_MEM_RSVE_END      ((BCM_NS3_MEM_ELOG_START + \
				   BCM_NS3_MEM_ELOG_LEN) - \
				   BCM_NS3_MEM_RSVE_START)

#define BCM_NS3_CRMU_PGT_START    0x880000000UL
#define BCM_NS3_CRMU_PGT_SIZE     0x100000
#endif

/*
 * Copyright (C) 2013 - ARM Ltd
 * Author: Marc Zyngier <marc.zyngier@arm.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __ARM_PSCI_H__
#define __ARM_PSCI_H__

#define ARM_PSCI_VER_1_0		(0x00010000)
#define ARM_PSCI_VER_0_2		(0x00000002)

/* PSCI 0.1 interface */
#define ARM_PSCI_FN_BASE		0x95c1ba5e
#define ARM_PSCI_FN(n)			(ARM_PSCI_FN_BASE + (n))

#define ARM_PSCI_FN_CPU_SUSPEND		ARM_PSCI_FN(0)
#define ARM_PSCI_FN_CPU_OFF		ARM_PSCI_FN(1)
#define ARM_PSCI_FN_CPU_ON		ARM_PSCI_FN(2)
#define ARM_PSCI_FN_MIGRATE		ARM_PSCI_FN(3)

#define ARM_PSCI_RET_SUCCESS		0
#define ARM_PSCI_RET_NI			(-1)
#define ARM_PSCI_RET_INVAL		(-2)
#define ARM_PSCI_RET_DENIED		(-3)
#define ARM_PSCI_RET_ALREADY_ON		(-4)
#define ARM_PSCI_RET_ON_PENDING		(-5)
#define ARM_PSCI_RET_INTERNAL_FAILURE	(-6)
#define ARM_PSCI_RET_NOT_PRESENT	(-7)
#define ARM_PSCI_RET_DISABLED		(-8)
#define ARM_PSCI_RET_INVALID_ADDRESS	(-9)

/* PSCI 0.2 interface */
#define ARM_PSCI_0_2_FN_BASE			0x84000000
#define ARM_PSCI_0_2_FN(n)			(ARM_PSCI_0_2_FN_BASE + (n))

#define ARM_PSCI_0_2_FN64_BASE			0xC4000000
#define ARM_PSCI_0_2_FN64(n)			(ARM_PSCI_0_2_FN64_BASE + (n))

#define ARM_PSCI_0_2_FN_PSCI_VERSION		ARM_PSCI_0_2_FN(0)
#define ARM_PSCI_0_2_FN_CPU_SUSPEND		ARM_PSCI_0_2_FN(1)
#define ARM_PSCI_0_2_FN_CPU_OFF			ARM_PSCI_0_2_FN(2)
#define ARM_PSCI_0_2_FN_CPU_ON			ARM_PSCI_0_2_FN(3)
#define ARM_PSCI_0_2_FN_AFFINITY_INFO		ARM_PSCI_0_2_FN(4)
#define ARM_PSCI_0_2_FN_MIGRATE			ARM_PSCI_0_2_FN(5)
#define ARM_PSCI_0_2_FN_MIGRATE_INFO_TYPE	ARM_PSCI_0_2_FN(6)
#define ARM_PSCI_0_2_FN_MIGRATE_INFO_UP_CPU	ARM_PSCI_0_2_FN(7)
#define ARM_PSCI_0_2_FN_SYSTEM_OFF		ARM_PSCI_0_2_FN(8)
#define ARM_PSCI_0_2_FN_SYSTEM_RESET		ARM_PSCI_0_2_FN(9)

#define ARM_PSCI_0_2_FN64_CPU_SUSPEND		ARM_PSCI_0_2_FN64(1)
#define ARM_PSCI_0_2_FN64_CPU_ON		ARM_PSCI_0_2_FN64(3)
#define ARM_PSCI_0_2_FN64_AFFINITY_INFO		ARM_PSCI_0_2_FN64(4)
#define ARM_PSCI_0_2_FN64_MIGRATE		ARM_PSCI_0_2_FN64(5)
#define ARM_PSCI_0_2_FN64_MIGRATE_INFO_UP_CPU	ARM_PSCI_0_2_FN64(7)

/* PSCI 1.0 interface */
#define ARM_PSCI_1_0_FN_PSCI_FEATURES		ARM_PSCI_0_2_FN(10)
#define ARM_PSCI_1_0_FN_CPU_FREEZE		ARM_PSCI_0_2_FN(11)
#define ARM_PSCI_1_0_FN_CPU_DEFAULT_SUSPEND	ARM_PSCI_0_2_FN(12)
#define ARM_PSCI_1_0_FN_NODE_HW_STATE		ARM_PSCI_0_2_FN(13)
#define ARM_PSCI_1_0_FN_SYSTEM_SUSPEND		ARM_PSCI_0_2_FN(14)
#define ARM_PSCI_1_0_FN_SET_SUSPEND_MODE	ARM_PSCI_0_2_FN(15)
#define ARM_PSCI_1_0_FN_STAT_RESIDENCY		ARM_PSCI_0_2_FN(16)
#define ARM_PSCI_1_0_FN_STAT_COUNT		ARM_PSCI_0_2_FN(17)

#define ARM_PSCI_1_0_FN64_CPU_DEFAULT_SUSPEND	ARM_PSCI_0_2_FN64(12)
#define ARM_PSCI_1_0_FN64_NODE_HW_STATE		ARM_PSCI_0_2_FN64(13)
#define ARM_PSCI_1_0_FN64_SYSTEM_SUSPEND	ARM_PSCI_0_2_FN64(14)
#define ARM_PSCI_1_0_FN64_STAT_RESIDENCY	ARM_PSCI_0_2_FN64(16)
#define ARM_PSCI_1_0_FN64_STAT_COUNT		ARM_PSCI_0_2_FN64(17)

/* 1KB stack per core */
#define ARM_PSCI_STACK_SHIFT	10
#define ARM_PSCI_STACK_SIZE	(1 << ARM_PSCI_STACK_SHIFT)

/* PSCI affinity level state returned by AFFINITY_INFO */
#define PSCI_AFFINITY_LEVEL_ON		0
#define PSCI_AFFINITY_LEVEL_OFF		1
#define PSCI_AFFINITY_LEVEL_ON_PENDING	2

#ifndef __ASSEMBLY__
#include <asm/types.h>

/* These 3 helper functions assume cpu < CONFIG_ARMV7_PSCI_NR_CPUS */
u32 psci_get_target_pc(int cpu);
u32 psci_get_context_id(int cpu);
void psci_save(int cpu, u32 pc, u32 context_id);

void psci_cpu_entry(void);
u32 psci_get_cpu_id(void);
void psci_cpu_off_common(void);

int psci_update_dt(void *fdt);
void psci_board_init(void);
int fdt_psci(void *fdt);

void psci_v7_flush_dcache_all(void);
#endif /* ! __ASSEMBLY__ */

#endif /* __ARM_PSCI_H__ */

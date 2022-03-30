// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <config.h>
#include <common.h>
#include <asm/armv7.h>
#include <asm/gic.h>
#include <asm/io.h>
#include <asm/psci.h>
#include <asm/secure.h>

#define BOOT_API_A7_CORE0_MAGIC_NUMBER	0xCA7FACE0
#define BOOT_API_A7_CORE1_MAGIC_NUMBER	0xCA7FACE1

#define MPIDR_AFF0			GENMASK(7, 0)

#define RCC_MP_GRSTCSETR		(STM32_RCC_BASE + 0x0404)
#define RCC_MP_GRSTCSETR_MPUP1RST	BIT(5)
#define RCC_MP_GRSTCSETR_MPUP0RST	BIT(4)
#define RCC_MP_GRSTCSETR_MPSYSRST	BIT(0)

#define STM32MP1_PSCI_NR_CPUS		2
#if STM32MP1_PSCI_NR_CPUS > CONFIG_ARMV7_PSCI_NR_CPUS
#error "invalid value for CONFIG_ARMV7_PSCI_NR_CPUS"
#endif

u8 psci_state[STM32MP1_PSCI_NR_CPUS] __secure_data = {
	 PSCI_AFFINITY_LEVEL_ON,
	 PSCI_AFFINITY_LEVEL_OFF};

void __secure psci_set_state(int cpu, u8 state)
{
	psci_state[cpu] = state;
	dsb();
	isb();
}

static u32 __secure stm32mp_get_gicd_base_address(void)
{
	u32 periphbase;

	/* get the GIC base address from the CBAR register */
	asm("mrc p15, 4, %0, c15, c0, 0\n" : "=r" (periphbase));

	return (periphbase & CBAR_MASK) + GIC_DIST_OFFSET;
}

static void __secure stm32mp_raise_sgi0(int cpu)
{
	u32 gic_dist_addr;

	gic_dist_addr = stm32mp_get_gicd_base_address();

	/* ask cpu with SGI0 */
	writel((BIT(cpu) << 16), gic_dist_addr + GICD_SGIR);
}

void __secure psci_arch_cpu_entry(void)
{
	u32 cpu = psci_get_cpu_id();

	psci_set_state(cpu, PSCI_AFFINITY_LEVEL_ON);

	/* reset magic in TAMP register */
	writel(0xFFFFFFFF, TAMP_BACKUP_MAGIC_NUMBER);
}

int __secure psci_features(u32 function_id, u32 psci_fid)
{
	switch (psci_fid) {
	case ARM_PSCI_0_2_FN_PSCI_VERSION:
	case ARM_PSCI_0_2_FN_CPU_OFF:
	case ARM_PSCI_0_2_FN_CPU_ON:
	case ARM_PSCI_0_2_FN_AFFINITY_INFO:
	case ARM_PSCI_0_2_FN_MIGRATE_INFO_TYPE:
	case ARM_PSCI_0_2_FN_SYSTEM_OFF:
	case ARM_PSCI_0_2_FN_SYSTEM_RESET:
		return 0x0;
	}
	return ARM_PSCI_RET_NI;
}

unsigned int __secure psci_version(u32 function_id)
{
	return ARM_PSCI_VER_1_0;
}

int __secure psci_affinity_info(u32 function_id, u32 target_affinity,
				u32  lowest_affinity_level)
{
	u32 cpu = target_affinity & MPIDR_AFF0;

	if (lowest_affinity_level > 0)
		return ARM_PSCI_RET_INVAL;

	if (target_affinity & ~MPIDR_AFF0)
		return ARM_PSCI_RET_INVAL;

	if (cpu >= STM32MP1_PSCI_NR_CPUS)
		return ARM_PSCI_RET_INVAL;

	return psci_state[cpu];
}

int __secure psci_migrate_info_type(u32 function_id)
{
	/*
	 * in Power_State_Coordination_Interface_PDD_v1_1_DEN0022D.pdf
	 * return 2 = Trusted OS is either not present or does not require
	 * migration, system of this type does not require the caller
	 * to use the MIGRATE function.
	 * MIGRATE function calls return NOT_SUPPORTED.
	 */
	return 2;
}

int __secure psci_cpu_on(u32 function_id, u32 target_cpu, u32 pc,
			 u32 context_id)
{
	u32 cpu = target_cpu & MPIDR_AFF0;

	if (target_cpu & ~MPIDR_AFF0)
		return ARM_PSCI_RET_INVAL;

	if (cpu >= STM32MP1_PSCI_NR_CPUS)
		return ARM_PSCI_RET_INVAL;

	if (psci_state[cpu] == PSCI_AFFINITY_LEVEL_ON)
		return ARM_PSCI_RET_ALREADY_ON;

	/* reset magic in TAMP register */
	if (readl(TAMP_BACKUP_MAGIC_NUMBER))
		writel(0xFFFFFFFF, TAMP_BACKUP_MAGIC_NUMBER);
	/*
	 * ROM code need a first SGI0 after core reset
	 * core is ready when magic is set to 0 in ROM code
	 */
	while (readl(TAMP_BACKUP_MAGIC_NUMBER))
		stm32mp_raise_sgi0(cpu);

	/* store target PC and context id*/
	psci_save(cpu, pc, context_id);

	/* write entrypoint in backup RAM register */
	writel((u32)&psci_cpu_entry, TAMP_BACKUP_BRANCH_ADDRESS);
	psci_set_state(cpu, PSCI_AFFINITY_LEVEL_ON_PENDING);

	/* write magic number in backup register */
	if (cpu == 0x01)
		writel(BOOT_API_A7_CORE1_MAGIC_NUMBER,
		       TAMP_BACKUP_MAGIC_NUMBER);
	else
		writel(BOOT_API_A7_CORE0_MAGIC_NUMBER,
		       TAMP_BACKUP_MAGIC_NUMBER);

	/* Generate an IT to start the core */
	stm32mp_raise_sgi0(cpu);

	return ARM_PSCI_RET_SUCCESS;
}

int __secure psci_cpu_off(u32 function_id)
{
	u32 cpu;

	cpu = psci_get_cpu_id();

	psci_cpu_off_common();
	psci_set_state(cpu, PSCI_AFFINITY_LEVEL_OFF);

	/* reset core: wfi is managed by BootRom */
	if (cpu == 0x01)
		writel(RCC_MP_GRSTCSETR_MPUP1RST, RCC_MP_GRSTCSETR);
	else
		writel(RCC_MP_GRSTCSETR_MPUP0RST, RCC_MP_GRSTCSETR);

	/* just waiting reset */
	while (1)
		wfi();
}

void __secure psci_system_reset(u32 function_id)
{
	/* System reset */
	writel(RCC_MP_GRSTCSETR_MPSYSRST, RCC_MP_GRSTCSETR);
	/* just waiting reset */
	while (1)
		wfi();
}

void __secure psci_system_off(u32 function_id)
{
	/* System Off is not managed, waiting user power off
	 * TODO: handle I2C write in PMIC Main Control register bit 0 = SWOFF
	 */
	while (1)
		wfi();
}

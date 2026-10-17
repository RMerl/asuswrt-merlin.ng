// SPDX-License-Identifier: GPL-2.0
/*
 * This file implements basic PSCI support for i.MX8M
 *
 * Copyright (C) 2022 Marek Vasut <marex@denx.de>
 */
#include <asm/arch/imx-regs.h>
#include <asm/cache.h>
#include <asm/gic.h>
#include <asm/io.h>
#include <asm/psci.h>
#include <asm/secure.h>
#include <common.h>
#include <cpu_func.h>
#include <debug_uart.h>
#include <fsl_wdog.h>
#include <linux/bitops.h>

#define SNVS_LPCR			0x38
#define SNVS_LPCR_TOP			BIT(6)
#define SNVS_LPCR_DP_EN			BIT(5)
#define SNVS_LPCR_SRTC_ENV		BIT(0)

#define MPIDR_AFF0			GENMASK(7, 0)

#define GPC_LPCR_A53_AD			0x4
#define EN_Cn_WFI_PDN(cpu)		BIT(((((cpu) & 1) * 2) + (((cpu) & 2) * 8)))
#define GPC_PGC_nCTRL(cpu)		(0x800 + ((cpu) * 0x40))
#define PGC_PCR				BIT(0)
#define GPC_CPU_PGC_SW_PUP_REQ		(IS_ENABLED(CONFIG_IMX8MP) ? 0xd0 : 0xf0)
#define COREn_A53_SW_PUP_REQ(cpu)	BIT(cpu)

#define SRC_A53RCR1			0x8
#define A53_COREn_ENABLE(n)		BIT(n)
#define SRC_GPR(n)			(0x74 + ((n) * 4))

/*
 * Helper code
 */
static u8 psci_state[CONFIG_ARMV8_PSCI_NR_CPUS] __secure_data = {
	PSCI_AFFINITY_LEVEL_ON,
	PSCI_AFFINITY_LEVEL_OFF,
	PSCI_AFFINITY_LEVEL_OFF,
	PSCI_AFFINITY_LEVEL_OFF
};

int psci_update_dt(void *fdt)
{
	return 0;
}

__secure static void psci_set_state(int cpu, u8 state)
{
	psci_state[cpu] = state;
	dsb();
	isb();
}

__secure static s32 psci_cpu_on_validate_mpidr(u64 mpidr, u32 *cpu)
{
	*cpu = mpidr & MPIDR_AFF0;

	if (mpidr & ~MPIDR_AFF0)
		return ARM_PSCI_RET_INVAL;

	if (*cpu >= CONFIG_ARMV8_PSCI_NR_CPUS)
		return ARM_PSCI_RET_INVAL;

	if (psci_state[*cpu] == PSCI_AFFINITY_LEVEL_ON)
		return ARM_PSCI_RET_ALREADY_ON;

	if (psci_state[*cpu] == PSCI_AFFINITY_LEVEL_ON_PENDING)
		return ARM_PSCI_RET_ON_PENDING;

	return ARM_PSCI_RET_SUCCESS;
}

__secure static void psci_cpu_on_write_entry_point(const u32 cpu, u64 entry_point)
{
	const u64 ep = CONFIG_SPL_TEXT_BASE;

	/* Trampoline target */
	writeq(entry_point, CPU_RELEASE_ADDR);
	/* RVBAR address HI */
	writel((u32)(ep >> 24) & 0xffff,
	       (void *)SRC_BASE_ADDR + SRC_GPR(cpu * 2));
	/* RVBAR address LO */
	writel((u32)(ep >> 2) & 0x3fffff,
	       (void *)SRC_BASE_ADDR + SRC_GPR(cpu * 2 + 1));
}

__secure static void psci_cpu_on_power_on(const u32 cpu)
{
	int i;

	clrbits_le32((void *)GPC_BASE_ADDR + GPC_LPCR_A53_AD, EN_Cn_WFI_PDN(cpu));
	clrbits_le32((void *)SRC_BASE_ADDR + SRC_A53RCR1, A53_COREn_ENABLE(cpu));
	setbits_le32((void *)GPC_BASE_ADDR + GPC_PGC_nCTRL(cpu), PGC_PCR);
	setbits_le32((void *)GPC_BASE_ADDR + GPC_CPU_PGC_SW_PUP_REQ, COREn_A53_SW_PUP_REQ(cpu));

	/* If we fail here, the core gets power cycled, hang is OK */
	while (readl(GPC_BASE_ADDR + GPC_CPU_PGC_SW_PUP_REQ) & COREn_A53_SW_PUP_REQ(cpu))
			;

	clrbits_le32((void *)GPC_BASE_ADDR + GPC_PGC_nCTRL(cpu), PGC_PCR);
	setbits_le32((void *)SRC_BASE_ADDR + SRC_A53RCR1, A53_COREn_ENABLE(cpu));

	/* Give the core a bit of time to boot and start executing code */
	for (i = 0; i < 100000; i++)
		asm volatile("nop");
}

__secure static void psci_cpu_on_power_off(const u32 cpu)
{
	setbits_le32((void *)GPC_BASE_ADDR + GPC_LPCR_A53_AD, EN_Cn_WFI_PDN(cpu));
	setbits_le32((void *)GPC_BASE_ADDR + GPC_PGC_nCTRL(cpu), PGC_PCR);
}

/*
 * Common PSCI code
 */
/* Return supported PSCI version */
__secure u32 psci_version(void)
{
	return ARM_PSCI_VER_1_0;
}

/*
 * 64bit PSCI code
 */
__secure s32 psci_cpu_on_64(u32 __always_unused function_id, u64 mpidr,
			    u64 entry_point_address, u64 context_id)
{
	u32 cpu = 0;
	int ret;

	ret = psci_cpu_on_validate_mpidr(mpidr, &cpu);
	if (ret != ARM_PSCI_RET_SUCCESS)
		return ret;

	psci_cpu_on_write_entry_point(cpu, entry_point_address);

	psci_set_state(cpu, PSCI_AFFINITY_LEVEL_ON);

	psci_cpu_on_power_on(cpu);

	smp_kick_all_cpus();

	return ARM_PSCI_RET_SUCCESS;
}

__secure s32 psci_affinity_info_64(u32 __always_unused function_id,
				   u64 target_affinity, u32 lowest_affinity_level)
{
	u32 cpu = target_affinity & MPIDR_AFF0;

	if (lowest_affinity_level > 0)
		return ARM_PSCI_RET_INVAL;

	if (target_affinity & ~MPIDR_AFF0)
		return ARM_PSCI_RET_INVAL;

	if (cpu >= CONFIG_ARMV8_PSCI_NR_CPUS)
		return ARM_PSCI_RET_INVAL;

	return psci_state[cpu];
}

__secure s32 psci_system_reset2_64(u32 __always_unused function_id,
				   u32 reset_type, u64 cookie)
{
	psci_system_reset();
	return 0;	/* Not reached */
}

/*
 * 32bit PSCI code
 */
__secure s32 psci_affinity_info(u32 __always_unused function_id,
				u32 target_affinity, u32 lowest_affinity_level)
{
	return psci_affinity_info_64(function_id, target_affinity, lowest_affinity_level);
}

__secure s32 psci_cpu_on(u32 __always_unused function_id, u32 mpidr,
			 u32 entry_point_address, u32 context_id)
{
	return psci_cpu_on_64(function_id, mpidr, entry_point_address, context_id);
}

__secure s32 psci_cpu_off(void)
{
	u32 cpu = psci_get_cpu_id();

	psci_cpu_on_power_off(cpu);
	psci_set_state(cpu, PSCI_AFFINITY_LEVEL_OFF);

	while (1)
		wfi();
}

__secure u32 psci_migrate_info_type(void)
{
	/* Trusted OS is either not present or does not require migration */
	return 2;
}

__secure void psci_system_reset(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;
	bool ext_reset = true;

	u16 wcr = WCR_WDE;

	if (ext_reset)
		wcr |= WCR_SRS; /* do not assert internal reset */
	else
		wcr |= WCR_WDA; /* do not assert external reset */

	/* Write 3 times to ensure it works, due to IMX6Q errata ERR004346 */
	writew(wcr, &wdog->wcr);
	writew(wcr, &wdog->wcr);
	writew(wcr, &wdog->wcr);

	while (1)
		wfi();
}

__secure void psci_system_off(void)
{
	writel(SNVS_LPCR_TOP | SNVS_LPCR_DP_EN | SNVS_LPCR_SRTC_ENV,
	       SNVS_BASE_ADDR + SNVS_LPCR);

	while (1)
		wfi();
}

/*
 * PSCI jump table
 */
__secure s32 psci_features(u32 __always_unused function_id, u32 psci_fid)
{
	switch (psci_fid) {
	case ARM_PSCI_0_2_FN_PSCI_VERSION:
	case ARM_PSCI_0_2_FN_CPU_OFF:
	case ARM_PSCI_0_2_FN_CPU_ON:
	case ARM_PSCI_0_2_FN_AFFINITY_INFO:
	case ARM_PSCI_0_2_FN_MIGRATE_INFO_TYPE:
	case ARM_PSCI_0_2_FN_SYSTEM_OFF:
	case ARM_PSCI_0_2_FN_SYSTEM_RESET:
	case ARM_PSCI_0_2_FN64_CPU_ON:
	case ARM_PSCI_0_2_FN64_AFFINITY_INFO:

	/* PSCI 1.0 interface */
	case ARM_PSCI_1_0_FN_PSCI_FEATURES:

	/* PSCI 1.1 interface */
	case ARM_PSCI_1_1_FN64_SYSTEM_RESET2:
		return 0x0;

	/*
	 * Not implemented:
	 * ARM_PSCI_0_2_FN_CPU_SUSPEND
	 * ARM_PSCI_1_0_FN_CPU_FREEZE
	 * ARM_PSCI_1_0_FN_CPU_DEFAULT_SUSPEND
	 * ARM_PSCI_1_0_FN_NODE_HW_STATE
	 * ARM_PSCI_1_0_FN_SYSTEM_SUSPEND
	 * ARM_PSCI_1_0_FN_SET_SUSPEND_MODE
	 * ARM_PSCI_1_0_FN_STAT_RESIDENCY
	 * ARM_PSCI_1_0_FN_STAT_COUNT
	 * ARM_PSCI_0_2_FN64_CPU_SUSPEND
	 * ARM_PSCI_1_0_FN64_CPU_DEFAULT_SUSPEND
	 * ARM_PSCI_1_0_FN64_NODE_HW_STATE
	 * ARM_PSCI_1_0_FN64_SYSTEM_SUSPEND
	 * ARM_PSCI_1_0_FN64_STAT_RESIDENCY
	 * ARM_PSCI_1_0_FN64_STAT_COUNT
	 */

	/* Not required, ARM_PSCI_0_2_FN_MIGRATE_INFO_TYPE returns 2 */
	case ARM_PSCI_0_2_FN_MIGRATE:
	case ARM_PSCI_0_2_FN64_MIGRATE:
	/* Not required */
	case ARM_PSCI_0_2_FN_MIGRATE_INFO_UP_CPU:
	case ARM_PSCI_0_2_FN64_MIGRATE_INFO_UP_CPU:
	default:
		return ARM_PSCI_RET_NI;
	}
}

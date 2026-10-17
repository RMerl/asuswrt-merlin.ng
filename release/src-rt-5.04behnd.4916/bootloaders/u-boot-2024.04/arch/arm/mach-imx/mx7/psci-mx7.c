// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 */

#include <cpu_func.h>
#include <asm/cache.h>
#include <asm/io.h>
#include <asm/psci.h>
#include <asm/secure.h>
#include <asm/arch/imx-regs.h>
#include <asm/armv7.h>
#include <asm/gic.h>
#include <linux/bitops.h>
#include <common.h>
#include <fsl_wdog.h>

#define GPC_LPCR_A7_BSC	0x0
#define GPC_LPCR_A7_AD		0x4
#define GPC_SLPCR		0x14
#define GPC_PGC_ACK_SEL_A7	0x24
#define GPC_IMR1_CORE0		0x30
#define GPC_SLOT0_CFG		0xb0
#define GPC_CPU_PGC_SW_PUP_REQ	0xf0
#define GPC_CPU_PGC_SW_PDN_REQ	0xfc
#define GPC_PGC_C0		0x800
#define GPC_PGC_C0		0x800
#define GPC_PGC_C1		0x840
#define GPC_PGC_SCU		0x880

#define BM_LPCR_A7_BSC_CPU_CLK_ON_LPM		0x4000
#define BM_LPCR_A7_BSC_LPM1			0xc
#define BM_LPCR_A7_BSC_LPM0			0x3
#define BP_LPCR_A7_BSC_LPM0			0
#define BM_SLPCR_EN_DSM				0x80000000
#define BM_SLPCR_RBC_EN				0x40000000
#define BM_SLPCR_REG_BYPASS_COUNT		0x3f000000
#define BM_SLPCR_VSTBY				0x4
#define BM_SLPCR_SBYOS				0x2
#define BM_SLPCR_BYPASS_PMIC_READY		0x1
#define BM_LPCR_A7_AD_L2PGE			0x10000
#define BM_LPCR_A7_AD_EN_C1_PUP			0x800
#define BM_LPCR_A7_AD_EN_C0_PUP			0x200
#define BM_LPCR_A7_AD_EN_PLAT_PDN		0x10
#define BM_LPCR_A7_AD_EN_C1_PDN			0x8
#define BM_LPCR_A7_AD_EN_C0_PDN			0x2

#define BM_CPU_PGC_SW_PDN_PUP_REQ_CORE0_A7	0x1
#define BM_CPU_PGC_SW_PDN_PUP_REQ_CORE1_A7	0x2

#define BM_GPC_PGC_ACK_SEL_A7_PD_DUMMY_ACK	0x8000
#define BM_GPC_PGC_ACK_SEL_A7_PU_DUMMY_ACK	0x80000000

#define MAX_SLOT_NUMBER				10
#define A7_LPM_WAIT				0x5
#define A7_LPM_STOP				0xa

#define BM_SYS_COUNTER_CNTCR_FCR1 0x200
#define BM_SYS_COUNTER_CNTCR_FCR0 0x100

#define REG_SET		0x4
#define REG_CLR		0x8

#define ANADIG_ARM_PLL		0x60
#define ANADIG_DDR_PLL		0x70
#define ANADIG_SYS_PLL		0xb0
#define ANADIG_ENET_PLL		0xe0
#define ANADIG_AUDIO_PLL	0xf0
#define ANADIG_VIDEO_PLL	0x130
#define BM_ANATOP_ARM_PLL_OVERRIDE	BIT(20)
#define BM_ANATOP_DDR_PLL_OVERRIDE	BIT(19)
#define BM_ANATOP_SYS_PLL_OVERRIDE	(0x1ff << 17)
#define BM_ANATOP_ENET_PLL_OVERRIDE	BIT(13)
#define BM_ANATOP_AUDIO_PLL_OVERRIDE	BIT(24)
#define BM_ANATOP_VIDEO_PLL_OVERRIDE	BIT(24)

#define DDRC_STAT	0x4
#define DDRC_PWRCTL	0x30
#define DDRC_PSTAT	0x3fc

#define SRC_GPR1_MX7D		0x074
#define SRC_GPR2_MX7D		0x078
#define SRC_A7RCR0		0x004
#define SRC_A7RCR1		0x008

#define BP_SRC_A7RCR0_A7_CORE_RESET0	0
#define BP_SRC_A7RCR1_A7_CORE1_ENABLE	1

#define SNVS_LPCR		0x38
#define BP_SNVS_LPCR_DP_EN	0x20
#define BP_SNVS_LPCR_TOP	0x40

#define CCM_CCGR_SNVS		0x4250

#define CCM_ROOT_WDOG		0xbb80
#define CCM_CCGR_WDOG1		0x49c0

#define MPIDR_AFF0		GENMASK(7, 0)

#define IMX7D_PSCI_NR_CPUS	2
#if IMX7D_PSCI_NR_CPUS > CONFIG_ARMV7_PSCI_NR_CPUS
#error "invalid value for CONFIG_ARMV7_PSCI_NR_CPUS"
#endif

#define imx_cpu_gpr_entry_offset(cpu) \
	(SRC_BASE_ADDR + SRC_GPR1_MX7D + cpu * 8)
#define imx_cpu_gpr_para_offset(cpu) \
	(imx_cpu_gpr_entry_offset(cpu) + 4)

#define IMX_CPU_SYNC_OFF	~0
#define IMX_CPU_SYNC_ON		0

u8 psci_state[IMX7D_PSCI_NR_CPUS] __secure_data = {
	 PSCI_AFFINITY_LEVEL_ON,
	 PSCI_AFFINITY_LEVEL_OFF};

enum imx_gpc_slot {
	CORE0_A7,
	CORE1_A7,
	SCU_A7,
	FAST_MEGA_MIX,
	MIPI_PHY,
	PCIE_PHY,
	USB_OTG1_PHY,
	USB_OTG2_PHY,
	USB_HSIC_PHY,
	CORE0_M4,
};

enum mxc_cpu_pwr_mode {
	RUN,
	WAIT,
	STOP,
};

extern void psci_system_resume(void);

static inline void psci_set_state(int cpu, u8 state)
{
	psci_state[cpu] = state;
	dsb();
	isb();
}

static inline void imx_gpcv2_set_m_core_pgc(bool enable, u32 offset)
{
	writel(enable, GPC_IPS_BASE_ADDR + offset);
}

__secure void imx_gpcv2_set_core_power(int cpu, bool pdn)
{
	u32 reg = pdn ? GPC_CPU_PGC_SW_PUP_REQ : GPC_CPU_PGC_SW_PDN_REQ;
	u32 pgc = cpu ? GPC_PGC_C1 : GPC_PGC_C0;
	u32 pdn_pup_req = cpu ? BM_CPU_PGC_SW_PDN_PUP_REQ_CORE1_A7 :
				BM_CPU_PGC_SW_PDN_PUP_REQ_CORE0_A7;
	u32 val;

	imx_gpcv2_set_m_core_pgc(true, pgc);

	val = readl(GPC_IPS_BASE_ADDR + reg);
	val |= pdn_pup_req;
	writel(val, GPC_IPS_BASE_ADDR + reg);

	while ((readl(GPC_IPS_BASE_ADDR + reg) & pdn_pup_req) != 0)
		;

	imx_gpcv2_set_m_core_pgc(false, pgc);
}

__secure void imx_enable_cpu_ca7(int cpu, bool enable)
{
	u32 mask, val;

	mask = 1 << (BP_SRC_A7RCR1_A7_CORE1_ENABLE + cpu - 1);
	val = readl(SRC_BASE_ADDR + SRC_A7RCR1);
	val = enable ? val | mask : val & ~mask;
	writel(val, SRC_BASE_ADDR + SRC_A7RCR1);
}

__secure void psci_arch_cpu_entry(void)
{
	u32 cpu = psci_get_cpu_id();

	psci_set_state(cpu, PSCI_AFFINITY_LEVEL_ON);
}

__secure s32 psci_cpu_on(u32 __always_unused function_id, u32 mpidr, u32 ep,
			 u32 context_id)
{
	u32 cpu = mpidr & MPIDR_AFF0;

	if (mpidr & ~MPIDR_AFF0)
		return ARM_PSCI_RET_INVAL;

	if (cpu >= IMX7D_PSCI_NR_CPUS)
		return ARM_PSCI_RET_INVAL;

	if (psci_state[cpu] == PSCI_AFFINITY_LEVEL_ON)
		return ARM_PSCI_RET_ALREADY_ON;

	if (psci_state[cpu] == PSCI_AFFINITY_LEVEL_ON_PENDING)
		return ARM_PSCI_RET_ON_PENDING;

	psci_save(cpu, ep, context_id);

	writel((u32)psci_cpu_entry, imx_cpu_gpr_entry_offset(cpu));

	psci_set_state(cpu, PSCI_AFFINITY_LEVEL_ON_PENDING);

	imx_gpcv2_set_core_power(cpu, true);
	imx_enable_cpu_ca7(cpu, true);

	return ARM_PSCI_RET_SUCCESS;
}

__secure s32 psci_cpu_off(void)
{
	int cpu;

	cpu = psci_get_cpu_id();

	psci_cpu_off_common();
	psci_set_state(cpu, PSCI_AFFINITY_LEVEL_OFF);

	imx_enable_cpu_ca7(cpu, false);
	imx_gpcv2_set_core_power(cpu, false);
	/*
	 * We use the cpu jumping argument register to sync with
	 * psci_affinity_info() which is running on cpu0 to kill the cpu.
	 */
	writel(IMX_CPU_SYNC_OFF, imx_cpu_gpr_para_offset(cpu));

	while (1)
		wfi();
}

__secure void psci_system_reset(void)
{
	struct wdog_regs *wdog = (struct wdog_regs *)WDOG1_BASE_ADDR;

	/* make sure WDOG1 clock is enabled */
	writel(0x1 << 28, CCM_BASE_ADDR + CCM_ROOT_WDOG);
	writel(0x3, CCM_BASE_ADDR + CCM_CCGR_WDOG1);
	writew(WCR_WDE, &wdog->wcr);

	while (1)
		wfi();
}

__secure void psci_system_off(void)
{
	u32 val;

	/* make sure SNVS clock is enabled */
	writel(0x3, CCM_BASE_ADDR + CCM_CCGR_SNVS);

	val = readl(SNVS_BASE_ADDR + SNVS_LPCR);
	val |= BP_SNVS_LPCR_DP_EN | BP_SNVS_LPCR_TOP;
	writel(val, SNVS_BASE_ADDR + SNVS_LPCR);

	while (1)
		wfi();
}

__secure u32 psci_version(void)
{
	return ARM_PSCI_VER_1_0;
}

__secure s32 psci_cpu_suspend(u32 __always_unused function_id, u32 power_state,
			      u32 entry_point_address,
			      u32 context_id)
{
	return ARM_PSCI_RET_INVAL;
}

__secure s32 psci_affinity_info(u32 __always_unused function_id,
				u32 target_affinity,
				u32 lowest_affinity_level)
{
	u32 cpu = target_affinity & MPIDR_AFF0;

	if (lowest_affinity_level > 0)
		return ARM_PSCI_RET_INVAL;

	if (target_affinity & ~MPIDR_AFF0)
		return ARM_PSCI_RET_INVAL;

	if (cpu >= IMX7D_PSCI_NR_CPUS)
		return ARM_PSCI_RET_INVAL;

	/* CPU is waiting for killed */
	if (readl(imx_cpu_gpr_para_offset(cpu)) == IMX_CPU_SYNC_OFF) {
		imx_enable_cpu_ca7(cpu, false);
		imx_gpcv2_set_core_power(cpu, false);
		writel(IMX_CPU_SYNC_ON, imx_cpu_gpr_para_offset(cpu));
	}

	return psci_state[cpu];
}

__secure u32 psci_migrate_info_type(void)
{
	/* Trusted OS is either not present or does not require migration */
	return 2;
}

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
	case ARM_PSCI_1_0_FN_PSCI_FEATURES:
	case ARM_PSCI_1_0_FN_SYSTEM_SUSPEND:
		return 0x0;
	}
	return ARM_PSCI_RET_NI;
}

static __secure void imx_gpcv2_set_lpm_mode(enum mxc_cpu_pwr_mode mode)
{
	u32 val1, val2, val3;

	val1 = readl(GPC_IPS_BASE_ADDR + GPC_LPCR_A7_BSC);
	val2 = readl(GPC_IPS_BASE_ADDR + GPC_SLPCR);

	/* all cores' LPM settings must be same */
	val1 &= ~(BM_LPCR_A7_BSC_LPM0 | BM_LPCR_A7_BSC_LPM1);
	val1 |= BM_LPCR_A7_BSC_CPU_CLK_ON_LPM;

	val2 &= ~(BM_SLPCR_EN_DSM | BM_SLPCR_VSTBY | BM_SLPCR_RBC_EN |
		BM_SLPCR_SBYOS | BM_SLPCR_BYPASS_PMIC_READY);
	/*
	 * GPC: When improper low-power sequence is used,
	 * the SoC enters low power mode before the ARM core executes WFI.
	 *
	 * Software workaround:
	 * 1) Software should trigger IRQ #32 (IOMUX) to be always pending
	 *    by setting IOMUX_GPR1_IRQ.
	 * 2) Software should then unmask IRQ #32 in GPC before setting GPC
	 *    Low-Power mode.
	 * 3) Software should mask IRQ #32 right after GPC Low-Power mode
	 *    is set.
	 */
	switch (mode) {
	case RUN:
		val3 = readl(GPC_IPS_BASE_ADDR + GPC_IMR1_CORE0);
		val3 &= ~0x1;
		writel(val3, GPC_IPS_BASE_ADDR + GPC_IMR1_CORE0);
		break;
	case WAIT:
		val1 |= A7_LPM_WAIT << BP_LPCR_A7_BSC_LPM0;
		val1 &= ~BM_LPCR_A7_BSC_CPU_CLK_ON_LPM;
		val3 = readl(GPC_IPS_BASE_ADDR + GPC_IMR1_CORE0);
		val3 &= ~0x1;
		writel(val3, GPC_IPS_BASE_ADDR + GPC_IMR1_CORE0);
		break;
	case STOP:
		val1 |= A7_LPM_STOP << BP_LPCR_A7_BSC_LPM0;
		val1 &= ~BM_LPCR_A7_BSC_CPU_CLK_ON_LPM;
		val2 |= BM_SLPCR_EN_DSM;
		val2 |= BM_SLPCR_SBYOS;
		val2 |= BM_SLPCR_VSTBY;
		val2 |= BM_SLPCR_BYPASS_PMIC_READY;
		val3 = readl(GPC_IPS_BASE_ADDR + GPC_IMR1_CORE0);
		val3 |= 0x1;
		writel(val3, GPC_IPS_BASE_ADDR + GPC_IMR1_CORE0);
		break;
	default:
		return;
	}
	writel(val1, GPC_IPS_BASE_ADDR + GPC_LPCR_A7_BSC);
	writel(val2, GPC_IPS_BASE_ADDR + GPC_SLPCR);
}

static __secure void imx_gpcv2_set_plat_power_gate_by_lpm(bool pdn)
{
	u32 val = readl(GPC_IPS_BASE_ADDR + GPC_LPCR_A7_AD);

	val &= ~(BM_LPCR_A7_AD_EN_PLAT_PDN | BM_LPCR_A7_AD_L2PGE);
	if (pdn)
		val |= BM_LPCR_A7_AD_EN_PLAT_PDN | BM_LPCR_A7_AD_L2PGE;

	writel(val, GPC_IPS_BASE_ADDR + GPC_LPCR_A7_AD);
}

static __secure void imx_gpcv2_set_cpu_power_gate_by_lpm(u32 cpu, bool pdn)
{
	u32 val;

	val = readl(GPC_IPS_BASE_ADDR + GPC_LPCR_A7_AD);
	if (cpu == 0) {
		if (pdn)
			val |= BM_LPCR_A7_AD_EN_C0_PDN |
				BM_LPCR_A7_AD_EN_C0_PUP;
		else
			val &= ~(BM_LPCR_A7_AD_EN_C0_PDN |
				BM_LPCR_A7_AD_EN_C0_PUP);
	}
	if (cpu == 1) {
		if (pdn)
			val |= BM_LPCR_A7_AD_EN_C1_PDN |
				BM_LPCR_A7_AD_EN_C1_PUP;
		else
			val &= ~(BM_LPCR_A7_AD_EN_C1_PDN |
				BM_LPCR_A7_AD_EN_C1_PUP);
	}
	writel(val, GPC_IPS_BASE_ADDR + GPC_LPCR_A7_AD);
}

static __secure void imx_gpcv2_set_slot_ack(u32 index, enum imx_gpc_slot m_core,
					    bool mode, bool ack)
{
	u32 val;

	if (index >= MAX_SLOT_NUMBER)
		return;

	/* set slot */
	writel(readl(GPC_IPS_BASE_ADDR + GPC_SLOT0_CFG + index * 4) |
		((mode + 1) << (m_core * 2)),
		GPC_IPS_BASE_ADDR + GPC_SLOT0_CFG + index * 4);

	if (ack) {
		/* set ack */
		val = readl(GPC_IPS_BASE_ADDR + GPC_PGC_ACK_SEL_A7);
		/* clear dummy ack */
		val &= ~(mode ? BM_GPC_PGC_ACK_SEL_A7_PU_DUMMY_ACK :
			BM_GPC_PGC_ACK_SEL_A7_PD_DUMMY_ACK);
		val |= 1 << (m_core + (mode ? 16 : 0));
		writel(val, GPC_IPS_BASE_ADDR + GPC_PGC_ACK_SEL_A7);
	}
}

static __secure void imx_system_counter_resume(void)
{
	u32 val;

	val = readl(SYSCNT_CTRL_IPS_BASE_ADDR);
	val &= ~BM_SYS_COUNTER_CNTCR_FCR1;
	val |= BM_SYS_COUNTER_CNTCR_FCR0;
	writel(val, SYSCNT_CTRL_IPS_BASE_ADDR);
}

static __secure void imx_system_counter_suspend(void)
{
	u32 val;

	val = readl(SYSCNT_CTRL_IPS_BASE_ADDR);
	val &= ~BM_SYS_COUNTER_CNTCR_FCR0;
	val |= BM_SYS_COUNTER_CNTCR_FCR1;
	writel(val, SYSCNT_CTRL_IPS_BASE_ADDR);
}

static __secure void gic_resume(void)
{
	u32 itlinesnr, i;
	u32 gic_dist_addr = GIC400_ARB_BASE_ADDR + GIC_DIST_OFFSET;

	/* enable the GIC distributor */
	writel(readl(gic_dist_addr + GICD_CTLR) | 0x03,
	       gic_dist_addr + GICD_CTLR);

	/* TYPER[4:0] contains an encoded number of available interrupts */
	itlinesnr = readl(gic_dist_addr + GICD_TYPER) & 0x1f;

	/* set all bits in the GIC group registers to one to allow access
	 * from non-secure state. The first 32 interrupts are private per
	 * CPU and will be set later when enabling the GIC for each core
	 */
	for (i = 1; i <= itlinesnr; i++)
		writel((u32)-1, gic_dist_addr + GICD_IGROUPRn + 4 * i);
}

static inline void imx_pll_suspend(void)
{
	writel(BM_ANATOP_ARM_PLL_OVERRIDE,
	       ANATOP_BASE_ADDR + ANADIG_ARM_PLL + REG_SET);
	writel(BM_ANATOP_DDR_PLL_OVERRIDE,
	       ANATOP_BASE_ADDR + ANADIG_DDR_PLL + REG_SET);
	writel(BM_ANATOP_SYS_PLL_OVERRIDE,
	       ANATOP_BASE_ADDR + ANADIG_SYS_PLL + REG_SET);
	writel(BM_ANATOP_ENET_PLL_OVERRIDE,
	       ANATOP_BASE_ADDR + ANADIG_ENET_PLL + REG_SET);
	writel(BM_ANATOP_AUDIO_PLL_OVERRIDE,
	       ANATOP_BASE_ADDR + ANADIG_AUDIO_PLL + REG_SET);
	writel(BM_ANATOP_VIDEO_PLL_OVERRIDE,
	       ANATOP_BASE_ADDR + ANADIG_VIDEO_PLL + REG_SET);
}

static inline void imx_pll_resume(void)
{
	writel(BM_ANATOP_ARM_PLL_OVERRIDE,
	       ANATOP_BASE_ADDR + ANADIG_ARM_PLL + REG_CLR);
	writel(BM_ANATOP_DDR_PLL_OVERRIDE,
	       ANATOP_BASE_ADDR + ANADIG_DDR_PLL + REG_CLR);
	writel(BM_ANATOP_SYS_PLL_OVERRIDE,
	       ANATOP_BASE_ADDR + ANADIG_SYS_PLL + REG_CLR);
	writel(BM_ANATOP_ENET_PLL_OVERRIDE,
	       ANATOP_BASE_ADDR + ANADIG_ENET_PLL + REG_CLR);
	writel(BM_ANATOP_AUDIO_PLL_OVERRIDE,
	       ANATOP_BASE_ADDR + ANADIG_AUDIO_PLL + REG_CLR);
	writel(BM_ANATOP_VIDEO_PLL_OVERRIDE,
	       ANATOP_BASE_ADDR + ANADIG_VIDEO_PLL + REG_CLR);
}

static inline void imx_udelay(u32 usec)
{
	u32 freq;
	u64 start, end;

	asm volatile("mrc p15, 0, %0, c14, c0, 0" : "=r" (freq));
	asm volatile("mrrc p15, 0, %Q0, %R0, c14" : "=r" (start));
	do {
		asm volatile("mrrc p15, 0, %Q0, %R0, c14" : "=r" (end));
		if ((end - start) > usec * (freq / 1000000))
			break;
	} while (1);
}

static inline void imx_ddrc_enter_self_refresh(void)
{
	writel(0, DDRC_IPS_BASE_ADDR + DDRC_PWRCTL);
	while (readl(DDRC_IPS_BASE_ADDR + DDRC_PSTAT) & 0x10001)
		;

	writel(0x20, DDRC_IPS_BASE_ADDR + DDRC_PWRCTL);
	while ((readl(DDRC_IPS_BASE_ADDR + DDRC_STAT) & 0x23) != 0x23)
		;
	writel(readl(DDRC_IPS_BASE_ADDR + DDRC_PWRCTL) | 0x8,
	       DDRC_IPS_BASE_ADDR + DDRC_PWRCTL);
}

static inline void imx_ddrc_exit_self_refresh(void)
{
	writel(0, DDRC_IPS_BASE_ADDR + DDRC_PWRCTL);
	while ((readl(DDRC_IPS_BASE_ADDR + DDRC_STAT) & 0x3) == 0x3)
		;
	writel(readl(DDRC_IPS_BASE_ADDR + DDRC_PWRCTL) | 0x1,
	       DDRC_IPS_BASE_ADDR + DDRC_PWRCTL);
}

__secure void imx_system_resume(void)
{
	unsigned int i, val, imr[4], entry;

	entry = psci_get_target_pc(0);
	imx_ddrc_exit_self_refresh();
	imx_system_counter_resume();
	imx_gpcv2_set_lpm_mode(RUN);
	imx_gpcv2_set_cpu_power_gate_by_lpm(0, false);
	imx_gpcv2_set_plat_power_gate_by_lpm(false);
	imx_gpcv2_set_m_core_pgc(false, GPC_PGC_C0);
	imx_gpcv2_set_m_core_pgc(false, GPC_PGC_SCU);

	/*
	 * need to mask all interrupts in GPC before
	 * operating RBC configurations
	 */
	for (i = 0; i < 4; i++) {
		imr[i] = readl(GPC_IPS_BASE_ADDR + GPC_IMR1_CORE0 + i * 4);
		writel(~0, GPC_IPS_BASE_ADDR + GPC_IMR1_CORE0 + i * 4);
	}

	/* configure RBC enable bit */
	val = readl(GPC_IPS_BASE_ADDR + GPC_SLPCR);
	val &= ~BM_SLPCR_RBC_EN;
	writel(val, GPC_IPS_BASE_ADDR + GPC_SLPCR);

	/* configure RBC count */
	val = readl(GPC_IPS_BASE_ADDR + GPC_SLPCR);
	val &= ~BM_SLPCR_REG_BYPASS_COUNT;
	writel(val, GPC_IPS_BASE_ADDR + GPC_SLPCR);

	/*
	 * need to delay at least 2 cycles of CKIL(32K)
	 * due to hardware design requirement, which is
	 * ~61us, here we use 65us for safe
	 */
	imx_udelay(65);

	/* restore GPC interrupt mask settings */
	for (i = 0; i < 4; i++)
		writel(imr[i], GPC_IPS_BASE_ADDR + GPC_IMR1_CORE0 + i * 4);

	/* initialize gic distributor */
	gic_resume();
	_nonsec_init();

	/* save cpu0 entry */
	psci_save(0, entry, 0);
	psci_cpu_entry();
}

__secure void psci_system_suspend(u32 __always_unused function_id,
				  u32 ep, u32 context_id)
{
	u32 gpc_mask[4];
	u32 i, val;

	psci_save(0, ep, context_id);
	/* overwrite PLL to be controlled by low power mode */
	imx_pll_suspend();
	imx_system_counter_suspend();
	/* set CA7 platform to enter STOP mode */
	imx_gpcv2_set_lpm_mode(STOP);
	/* enable core0/scu power down/up with low power mode */
	imx_gpcv2_set_cpu_power_gate_by_lpm(0, true);
	imx_gpcv2_set_plat_power_gate_by_lpm(true);
	/* time slot settings for core0 and scu */
	imx_gpcv2_set_slot_ack(0, CORE0_A7, false, false);
	imx_gpcv2_set_slot_ack(1, SCU_A7, false, true);
	imx_gpcv2_set_slot_ack(5, SCU_A7, true, false);
	imx_gpcv2_set_slot_ack(6, CORE0_A7, true, true);
	imx_gpcv2_set_m_core_pgc(true, GPC_PGC_C0);
	imx_gpcv2_set_m_core_pgc(true, GPC_PGC_SCU);
	psci_v7_flush_dcache_all();

	imx_ddrc_enter_self_refresh();

	/*
	 * e10133: ARM: Boot failure after A7 enters into
	 * low-power idle mode
	 *
	 * Workaround:
	 * If both CPU0/CPU1 are IDLE, the last IDLE CPU should
	 * disable GIC first, then REG_BYPASS_COUNTER is used
	 * to mask wakeup INT, and then execute “wfi” is used to
	 * bring the system into power down processing safely.
	 * The counter must be enabled as close to the “wfi” state
	 * as possible. The following equation can be used to
	 * determine the RBC counter value:
	 * RBC_COUNT * (1/32K RTC frequency) >=
	 * (46 + PDNSCR_SW + PDNSCR_SW2ISO ) ( 1/IPG_CLK frequency ).
	 */

	/* disable GIC distributor */
	writel(0, GIC400_ARB_BASE_ADDR + GIC_DIST_OFFSET);

	for (i = 0; i < 4; i++) {
		gpc_mask[i] = readl(GPC_IPS_BASE_ADDR + GPC_IMR1_CORE0 + i * 4);
		writel(~0, GPC_IPS_BASE_ADDR + GPC_IMR1_CORE0 + i * 4);
	}

	/*
	 * enable the RBC bypass counter here
	 * to hold off the interrupts. RBC counter
	 * = 8 (240us). With this setting, the latency
	 * from wakeup interrupt to ARM power up
	 * is ~250uS.
	 */
	val = readl(GPC_IPS_BASE_ADDR + GPC_SLPCR);
	val &= ~(0x3f << 24);
	val |= (0x8 << 24);
	writel(val, GPC_IPS_BASE_ADDR + GPC_SLPCR);

	/* enable the counter. */
	val = readl(GPC_IPS_BASE_ADDR + GPC_SLPCR);
	val |= (1 << 30);
	writel(val, GPC_IPS_BASE_ADDR + GPC_SLPCR);

	/* unmask all the GPC interrupts. */
	for (i = 0; i < 4; i++)
		writel(gpc_mask[i], GPC_IPS_BASE_ADDR + GPC_IMR1_CORE0 + i * 4);

	/*
	 * now delay for a short while (~3usec)
	 * ARM is at 1GHz at this point
	 * so a short loop should be enough.
	 * this delay is required to ensure that
	 * the RBC counter can start counting in
	 * case an interrupt is already pending
	 * or in case an interrupt arrives just
	 * as ARM is about to assert DSM_request.
	 */
	for (i = 0; i < 2000; i++)
		asm volatile("");

	/* save resume entry and sp in CPU0 GPR registers */
	asm volatile("mov %0, sp" : "=r" (val));
	writel((u32)psci_system_resume, SRC_BASE_ADDR + SRC_GPR1_MX7D);
	writel(val, SRC_BASE_ADDR + SRC_GPR2_MX7D);

	/* sleep */
	while (1)
		wfi();
}

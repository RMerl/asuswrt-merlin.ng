// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <config.h>
#include <common.h>
#include <asm/armv7.h>
#include <asm/cache.h>
#include <asm/gic.h>
#include <asm/io.h>
#include <asm/psci.h>
#include <asm/secure.h>
#include <hang.h>
#include <linux/bitops.h>

/* PWR */
#define PWR_CR3					0x0c
#define PWR_MPUCR				0x10

#define PWR_CR3_DDRSREN				BIT(10)
#define PWR_CR3_DDRRETEN			BIT(12)

#define PWR_MPUCR_PDDS				BIT(0)
#define PWR_MPUCR_CSTDBYDIS			BIT(3)
#define PWR_MPUCR_CSSF				BIT(9)

/* RCC */
#define RCC_MSSCKSELR				0x48
#define RCC_DDRITFCR				0xd8

#define RCC_DDRITFCR_DDRC1EN			BIT(0)
#define RCC_DDRITFCR_DDRC1LPEN			BIT(1)
#define RCC_DDRITFCR_DDRC2EN			BIT(2)
#define RCC_DDRITFCR_DDRC2LPEN			BIT(3)
#define RCC_DDRITFCR_DDRPHYCEN			BIT(4)
#define RCC_DDRITFCR_DDRPHYCLPEN		BIT(5)
#define RCC_DDRITFCR_DDRCAPBEN			BIT(6)
#define RCC_DDRITFCR_DDRCAPBLPEN		BIT(7)
#define RCC_DDRITFCR_AXIDCGEN			BIT(8)
#define RCC_DDRITFCR_DDRPHYCAPBEN		BIT(9)
#define RCC_DDRITFCR_DDRPHYCAPBLPEN		BIT(10)
#define RCC_DDRITFCR_DDRCKMOD_MASK		GENMASK(22, 20)
#define RCC_DDRITFCR_GSKPCTRL			BIT(24)

#define RCC_MP_SREQSETR				0x104
#define RCC_MP_SREQCLRR				0x108

#define RCC_MP_CIER				0x414
#define RCC_MP_CIFR				0x418
#define RCC_MP_CIFR_WKUPF			BIT(20)

#define RCC_MCUDIVR				0x830
#define RCC_PLL3CR				0x880
#define RCC_PLL4CR				0x894

/* SYSCFG */
#define SYSCFG_CMPCR				0x20
#define SYSCFG_CMPCR_SW_CTRL			BIT(2)
#define SYSCFG_CMPENSETR			0x24
#define SYSCFG_CMPENCLRR			0x28
#define SYSCFG_CMPENR_MPUEN			BIT(0)

/* DDR Controller registers offsets */
#define DDRCTRL_STAT				0x004
#define DDRCTRL_PWRCTL				0x030
#define DDRCTRL_PWRTMG				0x034
#define DDRCTRL_HWLPCTL				0x038
#define DDRCTRL_DFIMISC				0x1b0
#define DDRCTRL_SWCTL				0x320
#define DDRCTRL_SWSTAT				0x324
#define DDRCTRL_PSTAT				0x3fc
#define DDRCTRL_PCTRL_0				0x490
#define DDRCTRL_PCTRL_1				0x540

/* DDR Controller Register fields */
#define DDRCTRL_STAT_OPERATING_MODE_MASK	GENMASK(2, 0)
#define DDRCTRL_STAT_OPERATING_MODE_NORMAL	0x1
#define DDRCTRL_STAT_OPERATING_MODE_SR		0x3
#define DDRCTRL_STAT_SELFREF_TYPE_MASK		GENMASK(5, 4)
#define DDRCTRL_STAT_SELFREF_TYPE_ASR		(0x3 << 4)
#define DDRCTRL_STAT_SELFREF_TYPE_SR		(0x2 << 4)

#define DDRCTRL_PWRCTL_SELFREF_EN		BIT(0)
#define DDRCTRL_PWRCTL_EN_DFI_DRAM_CLK_DISABLE	BIT(3)
#define DDRCTRL_PWRCTL_SELFREF_SW		BIT(5)

#define DDRCTRL_PWRTMG_SELFREF_TO_X32_MASK	GENMASK(23, 16)
#define DDRCTRL_PWRTMG_SELFREF_TO_X32_0		BIT(16)

#define DDRCTRL_HWLPCTL_HW_LP_EN		BIT(0)

#define DDRCTRL_DFIMISC_DFI_INIT_COMPLETE_EN	BIT(0)

#define DDRCTRL_SWCTL_SW_DONE			BIT(0)

#define DDRCTRL_SWSTAT_SW_DONE_ACK		BIT(0)

#define DDRCTRL_PSTAT_RD_PORT_BUSY_0		BIT(0)
#define DDRCTRL_PSTAT_RD_PORT_BUSY_1		BIT(1)
#define DDRCTRL_PSTAT_WR_PORT_BUSY_0		BIT(16)
#define DDRCTRL_PSTAT_WR_PORT_BUSY_1		BIT(17)

#define DDRCTRL_PCTRL_N_PORT_EN			BIT(0)

/* DDR PHY registers offsets */
#define DDRPHYC_PIR				0x004
#define DDRPHYC_PGSR				0x00c
#define DDRPHYC_ACDLLCR				0x014
#define DDRPHYC_ACIOCR				0x024
#define DDRPHYC_DXCCR				0x028
#define DDRPHYC_DSGCR				0x02c
#define DDRPHYC_ZQ0CR0				0x180
#define DDRPHYC_DX0DLLCR			0x1cc
#define DDRPHYC_DX1DLLCR			0x20c
#define DDRPHYC_DX2DLLCR			0x24c
#define DDRPHYC_DX3DLLCR			0x28c

/* DDR PHY Register fields */
#define DDRPHYC_PIR_INIT			BIT(0)
#define DDRPHYC_PIR_DLLSRST			BIT(1)
#define DDRPHYC_PIR_DLLLOCK			BIT(2)
#define DDRPHYC_PIR_ITMSRST			BIT(4)

#define DDRPHYC_PGSR_IDONE			BIT(0)

#define DDRPHYC_ACDLLCR_DLLSRST			BIT(30)
#define DDRPHYC_ACDLLCR_DLLDIS			BIT(31)

#define DDRPHYC_ACIOCR_ACOE			BIT(1)
#define DDRPHYC_ACIOCR_ACPDD			BIT(3)
#define DDRPHYC_ACIOCR_ACPDR			BIT(4)
#define DDRPHYC_ACIOCR_CKPDD_MASK		GENMASK(10, 8)
#define DDRPHYC_ACIOCR_CKPDD_0			BIT(8)
#define DDRPHYC_ACIOCR_CKPDR_MASK		GENMASK(13, 11)
#define DDRPHYC_ACIOCR_CKPDR_0			BIT(11)
#define DDRPHYC_ACIOCR_CSPDD_MASK		GENMASK(20, 18)
#define DDRPHYC_ACIOCR_CSPDD_0			BIT(18)

#define DDRPHYC_DXCCR_DXPDD			BIT(2)
#define DDRPHYC_DXCCR_DXPDR			BIT(3)

#define DDRPHYC_DSGCR_CKEPDD_MASK		GENMASK(19, 16)
#define DDRPHYC_DSGCR_CKEPDD_0			BIT(16)
#define DDRPHYC_DSGCR_ODTPDD_MASK		GENMASK(23, 20)
#define DDRPHYC_DSGCR_ODTPDD_0			BIT(20)
#define DDRPHYC_DSGCR_NL2PD			BIT(24)
#define DDRPHYC_DSGCR_CKOE			BIT(28)

#define DDRPHYC_ZQ0CRN_ZQPD			BIT(31)

#define DDRPHYC_DXNDLLCR_DLLDIS			BIT(31)

#define BOOT_API_A7_CORE0_MAGIC_NUMBER		0xca7face0
#define BOOT_API_A7_CORE1_MAGIC_NUMBER		0xca7face1

#define MPIDR_AFF0				GENMASK(7, 0)

#define RCC_MP_GRSTCSETR			(STM32_RCC_BASE + 0x0404)
#define RCC_MP_GRSTCSETR_MPSYSRST		BIT(0)
#define RCC_MP_GRSTCSETR_MPUP0RST		BIT(4)
#define RCC_MP_GRSTCSETR_MPUP1RST		BIT(5)

/* IWDG */
#define IWDG_KR					0x00
#define IWDG_KR_RELOAD_KEY			0xaaaa
#define IWDG_EWCR				0x14
#define IWDG_EWCR_EWIC				BIT(14)

#define STM32MP1_PSCI_NR_CPUS			2
#if STM32MP1_PSCI_NR_CPUS > CONFIG_ARMV7_PSCI_NR_CPUS
#error "invalid value for CONFIG_ARMV7_PSCI_NR_CPUS"
#endif

u8 psci_state[STM32MP1_PSCI_NR_CPUS] __secure_data = {
	 PSCI_AFFINITY_LEVEL_ON,
	 PSCI_AFFINITY_LEVEL_OFF};

static u32 __secure_data cntfrq;

static u32 __secure cp15_read_cntfrq(void)
{
	u32 frq;

	asm volatile("mrc p15, 0, %0, c14, c0, 0" : "=r" (frq));

	return frq;
}

static void __secure cp15_write_cntfrq(u32 frq)
{
	asm volatile ("mcr p15, 0, %0, c14, c0, 0" : : "r" (frq));
}

static inline void psci_set_state(int cpu, u8 state)
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

	/* write the saved cntfrq */
	cp15_write_cntfrq(cntfrq);

	/* reset magic in TAMP register */
	writel(0xFFFFFFFF, TAMP_BACKUP_MAGIC_NUMBER);
}

s32 __secure psci_features(u32 function_id, u32 psci_fid)
{
	switch (psci_fid) {
	case ARM_PSCI_0_2_FN_PSCI_VERSION:
	case ARM_PSCI_0_2_FN_CPU_OFF:
	case ARM_PSCI_0_2_FN_CPU_ON:
	case ARM_PSCI_0_2_FN_AFFINITY_INFO:
	case ARM_PSCI_0_2_FN_MIGRATE_INFO_TYPE:
	case ARM_PSCI_0_2_FN_SYSTEM_OFF:
	case ARM_PSCI_0_2_FN_SYSTEM_RESET:
	case ARM_PSCI_1_0_FN_SYSTEM_SUSPEND:
		return 0x0;
	}
	return ARM_PSCI_RET_NI;
}

u32 __secure psci_version(void)
{
	return ARM_PSCI_VER_1_0;
}

s32 __secure psci_affinity_info(u32 function_id, u32 target_affinity,
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

u32 __secure psci_migrate_info_type(void)
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

s32 __secure psci_cpu_on(u32 function_id, u32 target_cpu, u32 pc,
			 u32 context_id)
{
	u32 cpu = target_cpu & MPIDR_AFF0;

	if (target_cpu & ~MPIDR_AFF0)
		return ARM_PSCI_RET_INVAL;

	if (cpu >= STM32MP1_PSCI_NR_CPUS)
		return ARM_PSCI_RET_INVAL;

	if (psci_state[cpu] == PSCI_AFFINITY_LEVEL_ON)
		return ARM_PSCI_RET_ALREADY_ON;

	/* read and save cntfrq of current cpu to write on target cpu  */
	cntfrq = cp15_read_cntfrq();

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

s32 __secure psci_cpu_off(void)
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

void __secure psci_system_reset(void)
{
	/* System reset */
	writel(RCC_MP_GRSTCSETR_MPSYSRST, RCC_MP_GRSTCSETR);
	/* just waiting reset */
	while (1)
		wfi();
}

void __secure psci_system_off(void)
{
	/* System Off is not managed, waiting user power off
	 * TODO: handle I2C write in PMIC Main Control register bit 0 = SWOFF
	 */
	while (1)
		wfi();
}

static void __secure secure_udelay(unsigned int delay)
{
	u32 freq = cp15_read_cntfrq() / 1000000;
	u64 start, end;

	delay *= freq;

	asm volatile("mrrc p15, 0, %Q0, %R0, c14" : "=r" (start));
	for (;;) {
		asm volatile("mrrc p15, 0, %Q0, %R0, c14" : "=r" (end));
		if ((end - start) > delay)
			break;
	}
}

static int __secure secure_waitbits(u32 reg, u32 mask, u32 val)
{
	u32 freq = cp15_read_cntfrq() / 1000000;
	u32 delay = 500 * freq;	/* 500 us */
	u64 start, end;
	u32 tmp;

	asm volatile("mrrc p15, 0, %Q0, %R0, c14" : "=r" (start));
	for (;;) {
		tmp = readl(reg);
		tmp &= mask;
		if ((tmp & val) == val)
			return 0;
		asm volatile("mrrc p15, 0, %Q0, %R0, c14" : "=r" (end));
		if ((end - start) > delay)
			return -ETIMEDOUT;
	}
}

static void __secure ddr_sr_mode_ssr(u32 *saved_pwrctl)
{
	setbits_le32(STM32_RCC_BASE + RCC_DDRITFCR,
		     RCC_DDRITFCR_DDRC1LPEN | RCC_DDRITFCR_DDRC1EN |
		     RCC_DDRITFCR_DDRC2LPEN | RCC_DDRITFCR_DDRC2EN |
		     RCC_DDRITFCR_DDRCAPBLPEN | RCC_DDRITFCR_DDRPHYCAPBLPEN |
		     RCC_DDRITFCR_DDRCAPBEN | RCC_DDRITFCR_DDRPHYCAPBEN |
		     RCC_DDRITFCR_DDRPHYCEN);

	clrbits_le32(STM32_RCC_BASE + RCC_DDRITFCR,
		     RCC_DDRITFCR_AXIDCGEN | RCC_DDRITFCR_DDRCKMOD_MASK);

	/* Disable HW LP interface of uMCTL2 */
	clrbits_le32(STM32_DDRCTRL_BASE + DDRCTRL_HWLPCTL,
		     DDRCTRL_HWLPCTL_HW_LP_EN);

	/* Configure Automatic LP modes of uMCTL2 */
	clrsetbits_le32(STM32_DDRCTRL_BASE + DDRCTRL_PWRTMG,
			DDRCTRL_PWRTMG_SELFREF_TO_X32_MASK,
			DDRCTRL_PWRTMG_SELFREF_TO_X32_0);

	/* Save PWRCTL register to restart ASR after suspend (if applicable) */
	*saved_pwrctl = readl(STM32_DDRCTRL_BASE + DDRCTRL_PWRCTL);

	/*
	 * Disable Clock disable with LP modes
	 * (used in RUN mode for LPDDR2 with specific timing).
	 */
	clrbits_le32(STM32_DDRCTRL_BASE + DDRCTRL_PWRCTL,
		     DDRCTRL_PWRCTL_EN_DFI_DRAM_CLK_DISABLE);

	/* Disable automatic Self-Refresh mode */
	clrbits_le32(STM32_DDRCTRL_BASE + DDRCTRL_PWRCTL,
		     DDRCTRL_PWRCTL_SELFREF_EN);
}

static void __secure ddr_sr_mode_restore(u32 saved_pwrctl)
{
	saved_pwrctl &= DDRCTRL_PWRCTL_EN_DFI_DRAM_CLK_DISABLE |
			DDRCTRL_PWRCTL_SELFREF_EN;

	/* Restore ASR mode in case it was enabled before suspend. */
	setbits_le32(STM32_DDRCTRL_BASE + DDRCTRL_PWRCTL, saved_pwrctl);
}

static int __secure ddr_sw_self_refresh_in(void)
{
	int ret;

	clrbits_le32(STM32_RCC_BASE + RCC_DDRITFCR, RCC_DDRITFCR_AXIDCGEN);

	/* Blocks AXI ports from taking anymore transactions */
	clrbits_le32(STM32_DDRCTRL_BASE + DDRCTRL_PCTRL_0,
		     DDRCTRL_PCTRL_N_PORT_EN);
	clrbits_le32(STM32_DDRCTRL_BASE + DDRCTRL_PCTRL_1,
		     DDRCTRL_PCTRL_N_PORT_EN);

	/*
	 * Waits unit all AXI ports are idle
	 * Poll PSTAT.rd_port_busy_n = 0
	 * Poll PSTAT.wr_port_busy_n = 0
	 */
	ret = secure_waitbits(STM32_DDRCTRL_BASE + DDRCTRL_PSTAT,
			      DDRCTRL_PSTAT_RD_PORT_BUSY_0 |
			      DDRCTRL_PSTAT_RD_PORT_BUSY_1 |
			      DDRCTRL_PSTAT_WR_PORT_BUSY_0 |
			      DDRCTRL_PSTAT_WR_PORT_BUSY_1, 0);
	if (ret)
		goto pstat_failed;

	/* SW Self-Refresh entry */
	setbits_le32(STM32_DDRCTRL_BASE + DDRCTRL_PWRCTL, DDRCTRL_PWRCTL_SELFREF_SW);

	/*
	 * Wait operating mode change in self-refresh mode
	 * with STAT.operating_mode[1:0]==11.
	 * Ensure transition to self-refresh was due to software
	 * by checking also that STAT.selfref_type[1:0]=2.
	 */
	ret = secure_waitbits(STM32_DDRCTRL_BASE + DDRCTRL_STAT,
			      DDRCTRL_STAT_OPERATING_MODE_MASK |
			      DDRCTRL_STAT_SELFREF_TYPE_MASK,
			      DDRCTRL_STAT_OPERATING_MODE_SR |
			      DDRCTRL_STAT_SELFREF_TYPE_SR);
	if (ret)
		goto selfref_sw_failed;

	/* IOs powering down (PUBL registers) */
	setbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_ACIOCR, DDRPHYC_ACIOCR_ACPDD);
	setbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_ACIOCR, DDRPHYC_ACIOCR_ACPDR);

	clrsetbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_ACIOCR,
			DDRPHYC_ACIOCR_CKPDD_MASK,
			DDRPHYC_ACIOCR_CKPDD_0);

	clrsetbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_ACIOCR,
			DDRPHYC_ACIOCR_CKPDR_MASK,
			DDRPHYC_ACIOCR_CKPDR_0);

	clrsetbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_ACIOCR,
			DDRPHYC_ACIOCR_CSPDD_MASK,
			DDRPHYC_ACIOCR_CSPDD_0);

	/* Disable command/address output driver */
	clrbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_ACIOCR, DDRPHYC_ACIOCR_ACOE);

	setbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_DXCCR, DDRPHYC_DXCCR_DXPDD);

	setbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_DXCCR, DDRPHYC_DXCCR_DXPDR);

	clrsetbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_DSGCR,
			DDRPHYC_DSGCR_ODTPDD_MASK,
			DDRPHYC_DSGCR_ODTPDD_0);

	setbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_DSGCR, DDRPHYC_DSGCR_NL2PD);

	clrsetbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_DSGCR,
			DDRPHYC_DSGCR_CKEPDD_MASK,
			DDRPHYC_DSGCR_CKEPDD_0);

	/* Disable PZQ cell (PUBL register) */
	setbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_ZQ0CR0, DDRPHYC_ZQ0CRN_ZQPD);

	/* Set latch */
	clrbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_DSGCR, DDRPHYC_DSGCR_CKOE);

	/* Additional delay to avoid early latch */
	secure_udelay(10);

	/* Activate sw retention in PWRCTRL */
	setbits_le32(STM32_PWR_BASE + PWR_CR3, PWR_CR3_DDRRETEN);

	/* Switch controller clocks (uMCTL2/PUBL) to DLL ref clock */
	setbits_le32(STM32_RCC_BASE + RCC_DDRITFCR, RCC_DDRITFCR_GSKPCTRL);

	/* Disable all DLLs: GLITCH window */
	setbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_ACDLLCR, DDRPHYC_ACDLLCR_DLLDIS);

	setbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_DX0DLLCR, DDRPHYC_DXNDLLCR_DLLDIS);

	setbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_DX1DLLCR, DDRPHYC_DXNDLLCR_DLLDIS);

	setbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_DX2DLLCR, DDRPHYC_DXNDLLCR_DLLDIS);

	setbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_DX3DLLCR, DDRPHYC_DXNDLLCR_DLLDIS);

	/* Switch controller clocks (uMCTL2/PUBL) to DLL output clock */
	clrbits_le32(STM32_RCC_BASE + RCC_DDRITFCR, RCC_DDRITFCR_GSKPCTRL);

	/* Deactivate all DDR clocks */
	clrbits_le32(STM32_RCC_BASE + RCC_DDRITFCR,
		     RCC_DDRITFCR_DDRC1EN | RCC_DDRITFCR_DDRC2EN |
		     RCC_DDRITFCR_DDRCAPBEN | RCC_DDRITFCR_DDRPHYCAPBEN);

	return 0;

selfref_sw_failed:
	/* This bit should be cleared to restore DDR in its previous state */
	clrbits_le32(STM32_DDRCTRL_BASE + DDRCTRL_PWRCTL,
		     DDRCTRL_PWRCTL_SELFREF_SW);

pstat_failed:
	setbits_le32(STM32_DDRCTRL_BASE + DDRCTRL_PCTRL_0,
		     DDRCTRL_PCTRL_N_PORT_EN);
	setbits_le32(STM32_DDRCTRL_BASE + DDRCTRL_PCTRL_1,
		     DDRCTRL_PCTRL_N_PORT_EN);

	return -EINVAL;
};

static void __secure ddr_sw_self_refresh_exit(void)
{
	int ret;

	/* Enable all clocks */
	setbits_le32(STM32_RCC_BASE + RCC_DDRITFCR,
		     RCC_DDRITFCR_DDRC1EN | RCC_DDRITFCR_DDRC2EN |
		     RCC_DDRITFCR_DDRPHYCEN | RCC_DDRITFCR_DDRPHYCAPBEN |
		     RCC_DDRITFCR_DDRCAPBEN);

	/* Handshake */
	clrbits_le32(STM32_DDRCTRL_BASE + DDRCTRL_SWCTL, DDRCTRL_SWCTL_SW_DONE);

	/* Mask dfi_init_complete_en */
	clrbits_le32(STM32_DDRCTRL_BASE + DDRCTRL_DFIMISC,
		     DDRCTRL_DFIMISC_DFI_INIT_COMPLETE_EN);

	/* Ack */
	setbits_le32(STM32_DDRCTRL_BASE + DDRCTRL_SWCTL, DDRCTRL_SWCTL_SW_DONE);
	ret = secure_waitbits(STM32_DDRCTRL_BASE + DDRCTRL_SWSTAT,
			      DDRCTRL_SWSTAT_SW_DONE_ACK,
			      DDRCTRL_SWSTAT_SW_DONE_ACK);
	if (ret)
		hang();

	/* Switch controller clocks (uMCTL2/PUBL) to DLL ref clock */
	setbits_le32(STM32_RCC_BASE + RCC_DDRITFCR, RCC_DDRITFCR_GSKPCTRL);

	/* Enable all DLLs: GLITCH window */
	clrbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_ACDLLCR,
		     DDRPHYC_ACDLLCR_DLLDIS);

	clrbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_DX0DLLCR, DDRPHYC_DXNDLLCR_DLLDIS);

	clrbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_DX1DLLCR, DDRPHYC_DXNDLLCR_DLLDIS);

	clrbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_DX2DLLCR, DDRPHYC_DXNDLLCR_DLLDIS);

	clrbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_DX3DLLCR, DDRPHYC_DXNDLLCR_DLLDIS);

	/* Additional delay to avoid early DLL clock switch */
	secure_udelay(50);

	/* Switch controller clocks (uMCTL2/PUBL) to DLL ref clock */
	clrbits_le32(STM32_RCC_BASE + RCC_DDRITFCR, RCC_DDRITFCR_GSKPCTRL);

	clrbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_ACDLLCR, DDRPHYC_ACDLLCR_DLLSRST);

	secure_udelay(10);

	setbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_ACDLLCR, DDRPHYC_ACDLLCR_DLLSRST);

	/* PHY partial init: (DLL lock and ITM reset) */
	writel(DDRPHYC_PIR_DLLSRST | DDRPHYC_PIR_DLLLOCK |
	       DDRPHYC_PIR_ITMSRST | DDRPHYC_PIR_INIT,
	       STM32_DDRPHYC_BASE + DDRPHYC_PIR);

	/* Need to wait at least 10 clock cycles before accessing PGSR */
	secure_udelay(1);

	/* Pool end of init */
	ret = secure_waitbits(STM32_DDRPHYC_BASE + DDRPHYC_PGSR,
			      DDRPHYC_PGSR_IDONE, DDRPHYC_PGSR_IDONE);
	if (ret)
		hang();

	/* Handshake */
	clrbits_le32(STM32_DDRCTRL_BASE + DDRCTRL_SWCTL, DDRCTRL_SWCTL_SW_DONE);

	/* Unmask dfi_init_complete_en to uMCTL2 */
	setbits_le32(STM32_DDRCTRL_BASE + DDRCTRL_DFIMISC, DDRCTRL_DFIMISC_DFI_INIT_COMPLETE_EN);

	/* Ack */
	setbits_le32(STM32_DDRCTRL_BASE + DDRCTRL_SWCTL, DDRCTRL_SWCTL_SW_DONE);
	ret = secure_waitbits(STM32_DDRCTRL_BASE + DDRCTRL_SWSTAT,
			      DDRCTRL_SWSTAT_SW_DONE_ACK,
			      DDRCTRL_SWSTAT_SW_DONE_ACK);
	if (ret)
		hang();

	/* Deactivate sw retention in PWR */
	clrbits_le32(STM32_PWR_BASE + PWR_CR3, PWR_CR3_DDRRETEN);

	/* Enable PZQ cell (PUBL register) */
	clrbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_ZQ0CR0, DDRPHYC_ZQ0CRN_ZQPD);

	/* Enable pad drivers */
	clrbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_ACIOCR, DDRPHYC_ACIOCR_ACPDD);

	/* Enable command/address output driver */
	setbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_ACIOCR, DDRPHYC_ACIOCR_ACOE);

	clrbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_ACIOCR, DDRPHYC_ACIOCR_CKPDD_MASK);

	clrbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_ACIOCR, DDRPHYC_ACIOCR_CSPDD_MASK);

	clrbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_DXCCR, DDRPHYC_DXCCR_DXPDD);

	clrbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_DXCCR, DDRPHYC_DXCCR_DXPDR);

	/* Release latch */
	setbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_DSGCR, DDRPHYC_DSGCR_CKOE);

	clrbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_DSGCR, DDRPHYC_DSGCR_ODTPDD_MASK);

	clrbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_DSGCR, DDRPHYC_DSGCR_NL2PD);

	clrbits_le32(STM32_DDRPHYC_BASE + DDRPHYC_DSGCR, DDRPHYC_DSGCR_CKEPDD_MASK);

	/* Remove selfrefresh */
	clrbits_le32(STM32_DDRCTRL_BASE + DDRCTRL_PWRCTL, DDRCTRL_PWRCTL_SELFREF_SW);

	/* Wait operating_mode == normal */
	ret = secure_waitbits(STM32_DDRCTRL_BASE + DDRCTRL_STAT,
			      DDRCTRL_STAT_OPERATING_MODE_MASK,
			      DDRCTRL_STAT_OPERATING_MODE_NORMAL);
	if (ret)
		hang();

	/* AXI ports are no longer blocked from taking transactions */
	setbits_le32(STM32_DDRCTRL_BASE + DDRCTRL_PCTRL_0, DDRCTRL_PCTRL_N_PORT_EN);
	setbits_le32(STM32_DDRCTRL_BASE + DDRCTRL_PCTRL_1, DDRCTRL_PCTRL_N_PORT_EN);

	setbits_le32(STM32_RCC_BASE + RCC_DDRITFCR, RCC_DDRITFCR_AXIDCGEN);
}

void __secure psci_system_suspend(u32 __always_unused function_id,
				  u32 ep, u32 context_id)
{
	u32 saved_mcudivr, saved_pll3cr, saved_pll4cr, saved_mssckselr;
	u32 gicd_addr = stm32mp_get_gicd_base_address();
	bool iwdg1_wake = false;
	bool iwdg2_wake = false;
	bool other_wake = false;
	u32 saved_pwrctl, reg;
	u32 gic_enabled[8];
	u32 irqs;
	int i;

	/* Cache enable mask of all 256 SPI */
	for (i = 0; i < ARRAY_SIZE(gic_enabled); i++)
		gic_enabled[i] = readl(gicd_addr + GICD_ISENABLERn + 0x4 + 4 * i);

	/* Disable IO compensation */

	/* Place current APSRC/ANSRC into RAPSRC/RANSRC */
	reg = readl(STM32_SYSCFG_BASE + SYSCFG_CMPCR);
	reg >>= 8;
	reg &= 0xff << 16;
	reg |= SYSCFG_CMPCR_SW_CTRL;
	writel(reg, STM32_SYSCFG_BASE + SYSCFG_CMPCR);
	writel(SYSCFG_CMPENR_MPUEN, STM32_SYSCFG_BASE + SYSCFG_CMPENCLRR);

	writel(RCC_MP_CIFR_WKUPF, STM32_RCC_BASE + RCC_MP_CIFR);
	setbits_le32(STM32_RCC_BASE + RCC_MP_CIER, RCC_MP_CIFR_WKUPF);

	setbits_le32(STM32_PWR_BASE + PWR_MPUCR,
		     PWR_MPUCR_CSSF | PWR_MPUCR_CSTDBYDIS);

	saved_mcudivr = readl(STM32_RCC_BASE + RCC_MCUDIVR);
	saved_pll3cr = readl(STM32_RCC_BASE + RCC_PLL3CR);
	saved_pll4cr = readl(STM32_RCC_BASE + RCC_PLL4CR);
	saved_mssckselr = readl(STM32_RCC_BASE + RCC_MSSCKSELR);

	psci_v7_flush_dcache_all();
	ddr_sr_mode_ssr(&saved_pwrctl);
	ddr_sw_self_refresh_in();
	setbits_le32(STM32_PWR_BASE + PWR_CR3, PWR_CR3_DDRSREN);
	writel(0x3, STM32_RCC_BASE + RCC_MP_SREQSETR);

	/* Ping the IWDG before entering suspend */
	iwdg1_wake = !!(gic_enabled[4] & BIT(22));	/* SPI 150 */
	iwdg2_wake = !!(gic_enabled[4] & BIT(23));	/* SPI 151 */

	for (;;) {
		/* Ping IWDG1 and ACK pretimer IRQ */
		if (iwdg1_wake) {
			writel(IWDG_KR_RELOAD_KEY, STM32_IWDG1_BASE + IWDG_KR);
			writel(IWDG_EWCR_EWIC, STM32_IWDG1_BASE + IWDG_EWCR);
		}

		/* Ping IWDG2 and ACK pretimer IRQ */
		if (iwdg2_wake) {
			writel(IWDG_KR_RELOAD_KEY, STM32_IWDG2_BASE + IWDG_KR);
			writel(IWDG_EWCR_EWIC, STM32_IWDG2_BASE + IWDG_EWCR);
		}

		iwdg1_wake = false;
		iwdg2_wake = false;

		/* Zzz, enter stop mode */
		asm volatile(
			"isb\n"
			"dsb\n"
			"wfi\n");

		/* Determine the wake up source */
		for (i = 0; i < ARRAY_SIZE(gic_enabled); i++) {
			irqs = readl(gicd_addr + GICR_IGROUPMODRn + 0x4 + 4 * i);
			irqs &= gic_enabled[i];
			if (!irqs)
				continue;

			/* Test whether IWDG pretimeout triggered the wake up. */
			if (i == 4) {	/* SPI Num 128..159 */
				iwdg1_wake = !!(irqs & BIT(22));	/* SPI 150 */
				iwdg2_wake = !!(irqs & BIT(23));	/* SPI 151 */
				irqs &= ~(BIT(22) | BIT(23));
			}

			/* Test whether there is any other wake up trigger. */
			if (irqs) {
				other_wake = true;
				break;
			}
		}

		/* Other wake up triggers pending, let OS deal with all of it. */
		if (other_wake)
			break;
	}

	writel(0x3, STM32_RCC_BASE + RCC_MP_SREQCLRR);
	ddr_sw_self_refresh_exit();
	ddr_sr_mode_restore(saved_pwrctl);

	writel(saved_mcudivr, STM32_RCC_BASE + RCC_MCUDIVR);
	writel(saved_pll3cr, STM32_RCC_BASE + RCC_PLL3CR);
	writel(saved_pll4cr, STM32_RCC_BASE + RCC_PLL4CR);
	writel(saved_mssckselr, STM32_RCC_BASE + RCC_MSSCKSELR);

	writel(SYSCFG_CMPENR_MPUEN, STM32_SYSCFG_BASE + SYSCFG_CMPENSETR);
	clrbits_le32(STM32_SYSCFG_BASE + SYSCFG_CMPCR, SYSCFG_CMPCR_SW_CTRL);
}

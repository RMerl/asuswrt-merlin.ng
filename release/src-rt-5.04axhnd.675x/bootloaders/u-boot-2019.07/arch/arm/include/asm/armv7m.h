/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010,2011
 * Vladimir Khusainov, Emcraft Systems, vlad@emcraft.com
 *
 * (C) Copyright 2015
 * Kamil Lulko, <kamil.lulko@gmail.com>
 */

#ifndef ARMV7M_H
#define ARMV7M_H

/* armv7m fixed base addresses */
#define V7M_SCS_BASE		0xE000E000
#define V7M_NVIC_BASE		(V7M_SCS_BASE + 0x0100)
#define V7M_SCB_BASE		(V7M_SCS_BASE + 0x0D00)
#define V7M_PROC_FTR_BASE	(V7M_SCS_BASE + 0x0D78)
#define V7M_MPU_BASE		(V7M_SCS_BASE + 0x0D90)
#define V7M_FPU_BASE		(V7M_SCS_BASE + 0x0F30)
#define V7M_CACHE_MAINT_BASE	(V7M_SCS_BASE + 0x0F50)
#define V7M_ACCESS_CNTL_BASE	(V7M_SCS_BASE + 0x0F90)

#define V7M_SCB_VTOR		0x08

#if !defined(__ASSEMBLY__)
struct v7m_scb {
	uint32_t cpuid;		/* CPUID Base Register */
	uint32_t icsr;		/* Interrupt Control and State Register */
	uint32_t vtor;		/* Vector Table Offset Register */
	uint32_t aircr;		/* App Interrupt and Reset Control Register */
	uint32_t scr;		/* offset 0x10: System Control Register */
	uint32_t ccr;		/* offset 0x14: Config and Control Register */
	uint32_t shpr1;		/* offset 0x18: System Handler Priority Reg 1 */
	uint32_t shpr2;		/* offset 0x1c: System Handler Priority Reg 2 */
	uint32_t shpr3;		/* offset 0x20: System Handler Priority Reg 3 */
	uint32_t shcrs;		/* offset 0x24: System Handler Control State */
	uint32_t cfsr;		/* offset 0x28: Configurable Fault Status Reg */
	uint32_t hfsr;		/* offset 0x2C: HardFault Status Register */
	uint32_t res;		/* offset 0x30: reserved */
	uint32_t mmar;		/* offset 0x34: MemManage Fault Address Reg */
	uint32_t bfar;		/* offset 0x38: BusFault Address Reg */
	uint32_t afsr;		/* offset 0x3C: Auxiliary Fault Status Reg */
};
#define V7M_SCB				((struct v7m_scb *)V7M_SCB_BASE)

#define V7M_AIRCR_VECTKEY		0x5fa
#define V7M_AIRCR_VECTKEY_SHIFT		16
#define V7M_AIRCR_ENDIAN		(1 << 15)
#define V7M_AIRCR_PRIGROUP_SHIFT	8
#define V7M_AIRCR_PRIGROUP_MSK		(0x7 << V7M_AIRCR_PRIGROUP_SHIFT)
#define V7M_AIRCR_SYSRESET		(1 << 2)

#define V7M_ICSR_VECTACT_MSK		0xFF

#define V7M_CCR_DCACHE			16
#define V7M_CCR_ICACHE			17

struct v7m_mpu {
	uint32_t type;		/* Type Register */
	uint32_t ctrl;		/* Control Register */
	uint32_t rnr;		/* Region Number Register */
	uint32_t rbar;		/* Region Base Address Register */
	uint32_t rasr;		/* Region Attribute and Size Register */
};
#define V7M_MPU				((struct v7m_mpu *)V7M_MPU_BASE)

#endif /* !defined(__ASSEMBLY__) */
#endif /* ARMV7M_H */

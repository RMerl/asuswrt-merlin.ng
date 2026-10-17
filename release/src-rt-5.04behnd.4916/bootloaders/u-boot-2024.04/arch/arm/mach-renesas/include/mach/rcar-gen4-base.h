/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * ./arch/arm/mach-renesas/include/mach/rcar-gen4-base.h
 *
 * Copyright (C) 2021 Renesas Electronics Corp.
 */

#ifndef __ASM_ARCH_RCAR_GEN4_BASE_H
#define __ASM_ARCH_RCAR_GEN4_BASE_H

/*
 * R-Car (R8A779F0) I/O Addresses
 */
#define RWDT_BASE		0xE6020000
#define SWDT_BASE		0xE6030000
#define TMU_BASE		0xE61E0000

/* SCIF */
#define SCIF0_BASE		0xE6E60000
#define SCIF1_BASE		0xE6E68000
#define SCIF2_BASE		0xE6E88000
#define SCIF3_BASE		0xE6C50000
#define SCIF4_BASE		0xE6C40000
#define SCIF5_BASE		0xE6F30000

/* CPG */
#define CPGWPR			0xE6150000
#define CPGWPCR			0xE6150004

/* Reset */
#define RST_BASE		0xE6160000 /* Domain0 */
#define RST_SRESCR0		(RST_BASE + 0x18)
#define RST_SPRES		0x5AA58000

/* Arm Generic Timer */
#define CNTCR_BASE		0xE6080000
#define CNTFID0			(CNTCR_BASE + 0x020)
#define CNTCR_EN		BIT(0)

/* GICv3 */
/* Distributor Registers */
#define GICD_BASE		0xF1000000
#define GICR_BASE		(GICR_LPI_BASE)

/* ReDistributor Registers for Control and Physical LPIs */
#define GICR_LPI_BASE		0xF1060000
#define GICR_WAKER		0x0014
#define GICR_PWRR		0x0024
#define GICR_LPI_WAKER		(GICR_LPI_BASE + GICR_WAKER)
#define GICR_LPI_PWRR		(GICR_LPI_BASE + GICR_PWRR)

/* ReDistributor Registers for SGIs and PPIs */
#define GICR_SGI_BASE		0xF1070000
#define GICR_IGROUPR0		0x0080

#ifndef __ASSEMBLY__
#include <asm/types.h>
#include <linux/bitops.h>

/* RWDT */
struct rcar_rwdt {
	u32 rwtcnt;
	u32 rwtcsra;
	u32 rwtcsrb;
};

/* SWDT */
struct rcar_swdt {
	u32 swtcnt;
	u32 swtcsra;
	u32 swtcsrb;
};
#endif

#endif /* __ASM_ARCH_RCAR_GEN4_BASE_H */

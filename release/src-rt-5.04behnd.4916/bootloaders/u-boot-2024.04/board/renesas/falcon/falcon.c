// SPDX-License-Identifier: GPL-2.0+
/*
 * board/renesas/falcon/falcon.c
 *     This file is Falcon board support.
 *
 * Copyright (C) 2020 Renesas Electronics Corp.
 */

#include <asm/arch/renesas.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mach-types.h>
#include <asm/processor.h>
#include <linux/errno.h>
#include <asm/system.h>
#include <asm/u-boot.h>

DECLARE_GLOBAL_DATA_PTR;

#define CPGWPR		0xE6150000
#define CPGWPCR		0xE6150004

#define EXTAL_CLK	16666600u
#define CNTCR_BASE	0xE6080000
#define CNTFID0		(CNTCR_BASE + 0x020)
#define CNTCR_EN	BIT(0)

static void init_generic_timer(void)
{
	u32 freq;

	/* Set frequency data in CNTFID0 */
	freq = EXTAL_CLK;

	/* Update memory mapped and register based freqency */
	asm volatile ("msr cntfrq_el0, %0" :: "r" (freq));
	writel(freq, CNTFID0);

	/* Enable counter */
	setbits_le32(CNTCR_BASE, CNTCR_EN);
}

/* Distributor Registers */
#define GICD_BASE	0xF1000000

/* ReDistributor Registers for Control and Physical LPIs */
#define GICR_LPI_BASE	0xF1060000
#define GICR_WAKER	0x0014
#define GICR_PWRR	0x0024
#define GICR_LPI_WAKER	(GICR_LPI_BASE + GICR_WAKER)
#define GICR_LPI_PWRR	(GICR_LPI_BASE + GICR_PWRR)

/* ReDistributor Registers for SGIs and PPIs */
#define GICR_SGI_BASE	0xF1070000
#define GICR_IGROUPR0	0x0080

static void init_gic_v3(void)
{
	 /* GIC v3 power on */
	writel(0x00000002, (GICR_LPI_PWRR));

	/* Wait till the WAKER_CA_BIT changes to 0 */
	writel(readl(GICR_LPI_WAKER) & ~0x00000002, (GICR_LPI_WAKER));
	while (readl(GICR_LPI_WAKER) & 0x00000004)
		;

	writel(0xffffffff, GICR_SGI_BASE + GICR_IGROUPR0);
}

void s_init(void)
{
	if (current_el() == 3)
		init_generic_timer();
}

int board_early_init_f(void)
{
	/* Unlock CPG access */
	writel(0x5A5AFFFF, CPGWPR);
	writel(0xA5A50000, CPGWPCR);

	return 0;
}

#define RST_BASE	0xE6160000 /* Domain0 */
#define RST_WDTRSTCR	(RST_BASE + 0x10)
#define RST_RWDT	0xA55A8002

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_TEXT_BASE + 0x50000;

	if (current_el() == 3) {
		init_gic_v3();

		/* Enable RWDT reset */
		writel(RST_RWDT, RST_WDTRSTCR);
	}

	return 0;
}

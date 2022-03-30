// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011-2015 Panasonic Corporation
 * Copyright (C) 2015-2017 Socionext Inc.
 *   Author: Masahiro Yamada <yamada.masahiro@socionext.com>
 */

#include <linux/io.h>

#include "../init.h"
#include "sbc-regs.h"

#define SBCTRL0_ADMULTIPLX_PERI_VALUE	0x33120000
#define SBCTRL1_ADMULTIPLX_PERI_VALUE	0x03005500
#define SBCTRL2_ADMULTIPLX_PERI_VALUE	0x14000020

#define SBCTRL0_ADMULTIPLX_MEM_VALUE	0x33120000
#define SBCTRL1_ADMULTIPLX_MEM_VALUE	0x03005500
#define SBCTRL2_ADMULTIPLX_MEM_VALUE	0x14000010

/* slower but LED works */
#define SBCTRL0_SAVEPIN_PERI_VALUE	0x55450000
#define SBCTRL1_SAVEPIN_PERI_VALUE	0x07168d00
#define SBCTRL2_SAVEPIN_PERI_VALUE	0x34000009
#define SBCTRL4_SAVEPIN_PERI_VALUE	0x02110110

/* faster but LED does not work */
#define SBCTRL0_SAVEPIN_MEM_VALUE	0x55450000
#define SBCTRL1_SAVEPIN_MEM_VALUE	0x06057700
/* NOR flash needs more wait counts than SRAM */
#define SBCTRL2_SAVEPIN_MEM_VALUE	0x34000009
#define SBCTRL4_SAVEPIN_MEM_VALUE	0x02110210

static void __uniphier_sbc_init(int savepin)
{
	/*
	 * Only CS1 is connected to support card.
	 * BKSZ[1:0] should be set to "01".
	 */
	if (savepin) {
		writel(SBCTRL0_SAVEPIN_PERI_VALUE, SBCTRL10);
		writel(SBCTRL1_SAVEPIN_PERI_VALUE, SBCTRL11);
		writel(SBCTRL2_SAVEPIN_PERI_VALUE, SBCTRL12);
		writel(SBCTRL4_SAVEPIN_PERI_VALUE, SBCTRL14);
	} else {
		writel(SBCTRL0_ADMULTIPLX_MEM_VALUE, SBCTRL10);
		writel(SBCTRL1_ADMULTIPLX_MEM_VALUE, SBCTRL11);
		writel(SBCTRL2_ADMULTIPLX_MEM_VALUE, SBCTRL12);
	}

	if (boot_is_swapped()) {
		/*
		 * Boot Swap On: boot from external NOR/SRAM
		 * 0x42000000-0x43ffffff is a mirror of 0x40000000-0x41ffffff.
		 *
		 * 0x40000000-0x41efffff, 0x42000000-0x43efffff: memory bank
		 * 0x41f00000-0x41ffffff, 0x43f00000-0x43ffffff: peripherals
		 */
		writel(0x0000bc01, SBBASE0);
	} else {
		/*
		 * Boot Swap Off: boot from mask ROM
		 * 0x40000000-0x41ffffff: mask ROM
		 * 0x42000000-0x43efffff: memory bank (31MB)
		 * 0x43f00000-0x43ffffff: peripherals (1MB)
		 */
		writel(0x0000be01, SBBASE0); /* dummy */
		writel(0x0200be01, SBBASE1);
	}
}

void uniphier_sbc_init_admulti(void)
{
	__uniphier_sbc_init(0);
}

void uniphier_sbc_init_savepin(void)
{
	__uniphier_sbc_init(1);
}

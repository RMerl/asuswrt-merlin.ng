// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 - 2013 Michal Simek <monstr@monstr.eu>
 * Copyright (C) 2012 - 2017 Xilinx, Inc. All rights reserved.
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>

#ifndef CONFIG_ZYNQ_DDRC_INIT
void zynq_ddrc_init(void) {}
#else
/* Control regsiter bitfield definitions */
#define ZYNQ_DDRC_CTRLREG_BUSWIDTH_MASK		0xC
#define ZYNQ_DDRC_CTRLREG_BUSWIDTH_SHIFT	2
#define ZYNQ_DDRC_CTRLREG_BUSWIDTH_16BIT	1

/* ECC scrub regsiter definitions */
#define ZYNQ_DDRC_ECC_SCRUBREG_ECC_MODE_MASK	0x7
#define ZYNQ_DDRC_ECC_SCRUBREG_ECCMODE_SECDED	0x4

void zynq_ddrc_init(void)
{
	u32 width, ecctype;

	width = readl(&ddrc_base->ddrc_ctrl);
	width = (width & ZYNQ_DDRC_CTRLREG_BUSWIDTH_MASK) >>
					ZYNQ_DDRC_CTRLREG_BUSWIDTH_SHIFT;
	ecctype = (readl(&ddrc_base->ecc_scrub) &
		ZYNQ_DDRC_ECC_SCRUBREG_ECC_MODE_MASK);

	/* ECC is enabled when memory is in 16bit mode and it is enabled */
	if ((ecctype == ZYNQ_DDRC_ECC_SCRUBREG_ECCMODE_SECDED) &&
	    (width == ZYNQ_DDRC_CTRLREG_BUSWIDTH_16BIT)) {
		puts("ECC enabled ");
		/*
		 * Clear the first 1MB because it is not initialized from
		 * first stage bootloader. To get ECC to work all memory has
		 * been initialized by writing any value.
		 */
		/* cppcheck-suppress nullPointer */
		memset((void *)0, 0, 1 * 1024 * 1024);
	} else {
		puts("ECC disabled ");
	}
}
#endif

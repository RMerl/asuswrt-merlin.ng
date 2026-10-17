/*
 * (C) Copyright 2022 Arm Limited
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SUNXI_CPU_SUNXI_NCAT2_H
#define _SUNXI_CPU_SUNXI_NCAT2_H

#define SUNXI_CCM_BASE			0x02001000
#define SUNXI_TIMER_BASE		0x02050000

#define SUNXI_TWI0_BASE			0x02502000
#define SUNXI_TWI1_BASE			0x02502400
#define SUNXI_TWI2_BASE			0x02502800
#define SUNXI_TWI3_BASE			0x02502C00

#define SUNXI_SRAMC_BASE		0x03000000
/* SID address space starts at 0x03006000, but e-fuse is at offset 0x200 */
#define SUNXI_SIDC_BASE			0x03006000
#define SUNXI_SID_BASE			0x03006200
#define SUNXI_GIC400_BASE		0x03020000

#define SUNXI_MMC0_BASE			0x04020000
#define SUNXI_MMC1_BASE			0x04021000
#define SUNXI_MMC2_BASE			0x04022000

#define SUNXI_R_CPUCFG_BASE		0x07000400
#define SUNXI_PRCM_BASE			0x07010000

#define SUNXI_CPUCFG_BASE		0x09010000

#ifndef __ASSEMBLY__
void sunxi_board_init(void);
void sunxi_reset(void);
int sunxi_get_sid(unsigned int *sid);
#endif

#endif /* _SUNXI_CPU_SUNXI_NCAT2_H */

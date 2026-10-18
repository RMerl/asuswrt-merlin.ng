/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2019 Broadcom */
#ifndef _ASM_BRCMSTB_RESET_API_H
#define _ASM_BRCMSTB_RESET_API_H

#define BRST_SW_OFFSET		0x5000

#define BRST_SW_CPU0		(BRST_SW_OFFSET + 0x0)
#define BRST_SW_V3D		(BRST_SW_OFFSET + 0x1)
#define BRST_SW_SYSIF		(BRST_SW_OFFSET + 0x2)
#define BRST_SW_SCB		(BRST_SW_OFFSET + 0x3)
#define BRST_SW_HVD0		(BRST_SW_OFFSET + 0x4)
#define BRST_SW_RAAGA0		(BRST_SW_OFFSET + 0x5)
#define BRST_SW_VICE0		(BRST_SW_OFFSET + 0x6)
#define BRST_SW_VICE0_PSS	(BRST_SW_OFFSET + 0x7)
#define BRST_SW_VICE1		(BRST_SW_OFFSET + 0x8)
#define BRST_SW_VICE1_PSS	(BRST_SW_OFFSET + 0x9)
#define BRST_SW_XPT		(BRST_SW_OFFSET + 0xa)
#define BRST_SW_M2MC0		(BRST_SW_OFFSET + 0xb)
#define BRST_SW_M2MC1		(BRST_SW_OFFSET + 0xc)
#define BRST_SW_MIPMAP0		(BRST_SW_OFFSET + 0xd)
#define BRST_SW_TSX0		(BRST_SW_OFFSET + 0xe)
#define BRST_SW_SMARTCARD0	(BRST_SW_OFFSET + 0xf)
#define BRST_SW_SMARTCARD1	(BRST_SW_OFFSET + 0x10)
#define BRST_SW_VPU0		(BRST_SW_OFFSET + 0x11)
#define BRST_SW_BNE0		(BRST_SW_OFFSET + 0x12)
#define BRST_SW_ASP0		(BRST_SW_OFFSET + 0x13)
#define BRST_SW_HVD_CABAC0	(BRST_SW_OFFSET + 0x14)
#define BRST_SW_AXI0		(BRST_SW_OFFSET + 0x15)
#define BRST_SW_BSTM0		(BRST_SW_OFFSET + 0x16)
#define BRST_SW_CPU1		(BRST_SW_OFFSET + 0x17)
#define BRST_SW_CPU2		(BRST_SW_OFFSET + 0x18)
#define BRST_SW_CPU3		(BRST_SW_OFFSET + 0x19)
#define BRST_SW_DVPHT_CORE	(BRST_SW_OFFSET + 0x1a)
#define BRST_SW_HVS0		(BRST_SW_OFFSET + 0x1b)
/*
 * IF YOU ADD A RESET CORE, PLEASE UPDATE BELOW.
 * ALSO, KEEP THIS IN SYNC WITH THE clk_api.h CORE LIST.
 */
#define BRST_SW_NUM_CORES	(BRST_SW_HVS0 - BCLK_SW_OFFSET + 1)



#if IS_ENABLED(CONFIG_BRCMSTB_NEXUS_RESET_API)
int brcm_reset_assert(unsigned int rst_id);
int brcm_reset_deassert(unsigned int rst_id);
int brcm_overtemp_reset(unsigned int temp);
int brcm_overtemp_reset_threshold(unsigned int temp,
				  unsigned int park_high,
				  unsigned int park_low);

#else
static inline int brcm_reset_assert(unsigned int rst_id)
{
	return -EOPNOTSUPP;
}
static inline int brcm_reset_deassert(unsigned int rst_id)
{
	return -EOPNOTSUPP;
}
static inline int brcm_overtemp_reset(unsigned int temp)
{
	return -EOPNOTSUPP;
}
static inline int brcm_overtemp_reset_threshold(unsigned int temp,
						unsigned int park_high,
						unsigned int park_low)
{
	return -EOPNOTSUPP;
}
#endif
#endif

/* SPDX-License-Identifier: GPL-2.0+  */
/*
 * Rockchip Electronics Co., Ltd.
 */

#ifndef __ASM_ARCH_CPU_RK3288_H
#define __ASM_ARCH_CPU_RK3288_H

#include <asm/io.h>

#define ROCKCHIP_CPU_MASK       0xffff0000
#define ROCKCHIP_CPU_RK3288     0x32880000

#define ROCKCHIP_SOC_MASK	(ROCKCHIP_CPU_MASK | 0xff)
#define ROCKCHIP_SOC_RK3288     (ROCKCHIP_CPU_RK3288 | 0x00)
#define ROCKCHIP_SOC_RK3288W    (ROCKCHIP_CPU_RK3288 | 0x01)

#define RK3288_HDMI_PHYS	0xff980000
#define HDMI_CONFIG0_ID		0x4
#define RK3288W_HDMI_REVID	0x1a

static inline int rockchip_soc_id(void)
{
	u8 reg;

#if defined(CONFIG_ROCKCHIP_RK3288)
	reg = readb(RK3288_HDMI_PHYS + HDMI_CONFIG0_ID);
	if (reg == RK3288W_HDMI_REVID)
		return ROCKCHIP_SOC_RK3288W;
	else
		return ROCKCHIP_SOC_RK3288;
#else
	return 0;
#endif
}

#define ROCKCHIP_SOC(id, ID) \
static inline bool soc_is_##id(void) \
{ \
	int soc_id = rockchip_soc_id(); \
	if (soc_id) \
		return ((soc_id & ROCKCHIP_SOC_MASK) == ROCKCHIP_SOC_ ##ID); \
	return false; \
}

ROCKCHIP_SOC(rk3288, RK3288)
ROCKCHIP_SOC(rk3288w, RK3288W)

#endif /* __ASM_ARCH_CPU_RK3288_H */

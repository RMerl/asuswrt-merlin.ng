// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2018, 2020 NXP
 */
#include <common.h>
#include <phy.h>
#include <fsl-mc/ldpaa_wriop.h>
#include <asm/io.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>
#include <linux/mii.h>

u32 dpmac_to_devdisr[] = {
	[WRIOP1_DPMAC1] = FSL_CHASSIS3_DEVDISR2_DPMAC1,
	[WRIOP1_DPMAC2] = FSL_CHASSIS3_DEVDISR2_DPMAC2,
	[WRIOP1_DPMAC3] = FSL_CHASSIS3_DEVDISR2_DPMAC3,
	[WRIOP1_DPMAC4] = FSL_CHASSIS3_DEVDISR2_DPMAC4,
	[WRIOP1_DPMAC5] = FSL_CHASSIS3_DEVDISR2_DPMAC5,
	[WRIOP1_DPMAC6] = FSL_CHASSIS3_DEVDISR2_DPMAC6,
	[WRIOP1_DPMAC7] = FSL_CHASSIS3_DEVDISR2_DPMAC7,
	[WRIOP1_DPMAC8] = FSL_CHASSIS3_DEVDISR2_DPMAC8,
	[WRIOP1_DPMAC9] = FSL_CHASSIS3_DEVDISR2_DPMAC9,
	[WRIOP1_DPMAC10] = FSL_CHASSIS3_DEVDISR2_DPMAC10,
	[WRIOP1_DPMAC11] = FSL_CHASSIS3_DEVDISR2_DPMAC11,
	[WRIOP1_DPMAC12] = FSL_CHASSIS3_DEVDISR2_DPMAC12,
	[WRIOP1_DPMAC13] = FSL_CHASSIS3_DEVDISR2_DPMAC13,
	[WRIOP1_DPMAC14] = FSL_CHASSIS3_DEVDISR2_DPMAC14,
	[WRIOP1_DPMAC15] = FSL_CHASSIS3_DEVDISR2_DPMAC15,
	[WRIOP1_DPMAC16] = FSL_CHASSIS3_DEVDISR2_DPMAC16,
	[WRIOP1_DPMAC17] = FSL_CHASSIS3_DEVDISR2_DPMAC17,
	[WRIOP1_DPMAC18] = FSL_CHASSIS3_DEVDISR2_DPMAC18,
};

static int is_device_disabled(int dpmac_id)
{
	struct ccsr_gur __iomem *gur = (void *)CFG_SYS_FSL_GUTS_ADDR;
	u32 devdisr2 = in_le32(&gur->devdisr2);

	return dpmac_to_devdisr[dpmac_id] & devdisr2;
}

void wriop_dpmac_disable(int dpmac_id)
{
	struct ccsr_gur __iomem *gur = (void *)CFG_SYS_FSL_GUTS_ADDR;

	setbits_le32(&gur->devdisr2, dpmac_to_devdisr[dpmac_id]);
}

void wriop_dpmac_enable(int dpmac_id)
{
	struct ccsr_gur __iomem *gur = (void *)CFG_SYS_FSL_GUTS_ADDR;

	clrbits_le32(&gur->devdisr2, dpmac_to_devdisr[dpmac_id]);
}

phy_interface_t wriop_dpmac_enet_if(int dpmac_id, int lane_prtcl)
{
	enum srds_prtcl;

	if (is_device_disabled(dpmac_id))
		return PHY_INTERFACE_MODE_NA;

	if (lane_prtcl >= SGMII1 && lane_prtcl <= SGMII18)
		return PHY_INTERFACE_MODE_SGMII;

	if (lane_prtcl >= XFI1 && lane_prtcl <= XFI14)
		return PHY_INTERFACE_MODE_XGMII;

	if (lane_prtcl >= _25GE1 && lane_prtcl <= _25GE10)
		return PHY_INTERFACE_MODE_25G_AUI;

	if (lane_prtcl >= _40GE1 && lane_prtcl <= _40GE2)
		return PHY_INTERFACE_MODE_XLAUI;

	if (lane_prtcl >= _50GE1 && lane_prtcl <= _50GE2)
		return PHY_INTERFACE_MODE_CAUI2;

	if (lane_prtcl >= _100GE1 && lane_prtcl <= _100GE2)
		return PHY_INTERFACE_MODE_CAUI4;

	return PHY_INTERFACE_MODE_NA;
}

#ifdef CONFIG_SYS_FSL_HAS_RGMII
void fsl_rgmii_init(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CFG_SYS_FSL_GUTS_ADDR);
	u32 ec;

#ifdef CONFIG_SYS_FSL_EC1
	ec = gur_in32(&gur->rcwsr[FSL_CHASSIS3_EC1_REGSR - 1])
		& FSL_CHASSIS3_EC1_REGSR_PRTCL_MASK;
	ec >>= FSL_CHASSIS3_EC1_REGSR_PRTCL_SHIFT;

	if (!ec)
		wriop_init_dpmac_enet_if(17, PHY_INTERFACE_MODE_RGMII_ID);
#endif

#ifdef CONFIG_SYS_FSL_EC2
	ec = gur_in32(&gur->rcwsr[FSL_CHASSIS3_EC2_REGSR - 1])
		& FSL_CHASSIS3_EC2_REGSR_PRTCL_MASK;
	ec >>= FSL_CHASSIS3_EC2_REGSR_PRTCL_SHIFT;

	if (!ec)
		wriop_init_dpmac_enet_if(18, PHY_INTERFACE_MODE_RGMII_ID);
#endif
}
#endif

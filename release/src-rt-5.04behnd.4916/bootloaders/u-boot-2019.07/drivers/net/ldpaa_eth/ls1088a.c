// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 NXP
 */
#include <common.h>
#include <phy.h>
#include <fsl-mc/ldpaa_wriop.h>
#include <asm/io.h>
#include <asm/arch/fsl_serdes.h>
#include <asm/arch/soc.h>

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
};

static int is_device_disabled(int dpmac_id)
{
	struct ccsr_gur __iomem *gur = (void *)CONFIG_SYS_FSL_GUTS_ADDR;
	u32 devdisr2 = in_le32(&gur->devdisr2);

	return dpmac_to_devdisr[dpmac_id] & devdisr2;
}

void wriop_dpmac_disable(int dpmac_id)
{
	struct ccsr_gur __iomem *gur = (void *)CONFIG_SYS_FSL_GUTS_ADDR;

	setbits_le32(&gur->devdisr2, dpmac_to_devdisr[dpmac_id]);
}

void wriop_dpmac_enable(int dpmac_id)
{
	struct ccsr_gur __iomem *gur = (void *)CONFIG_SYS_FSL_GUTS_ADDR;

	clrbits_le32(&gur->devdisr2, dpmac_to_devdisr[dpmac_id]);
}

phy_interface_t wriop_dpmac_enet_if(int dpmac_id, int lane_prtcl)
{
	enum srds_prtcl;

	if (is_device_disabled(dpmac_id + 1))
		return PHY_INTERFACE_MODE_NONE;

	switch (lane_prtcl) {
	case SGMII1:
	case SGMII2:
	case SGMII3:
	case SGMII7:
		return PHY_INTERFACE_MODE_SGMII;
	}

	if (lane_prtcl >= XFI1 && lane_prtcl <= XFI2)
		return PHY_INTERFACE_MODE_XGMII;

	if (lane_prtcl >= QSGMII_A && lane_prtcl <= QSGMII_B)
		return PHY_INTERFACE_MODE_QSGMII;

	return PHY_INTERFACE_MODE_NONE;
}

void wriop_init_dpmac_qsgmii(int sd, int lane_prtcl)
{
	switch (lane_prtcl) {
	case QSGMII_A:
		wriop_init_dpmac(sd, 3, (int)lane_prtcl);
		wriop_init_dpmac(sd, 4, (int)lane_prtcl);
		wriop_init_dpmac(sd, 5, (int)lane_prtcl);
		wriop_init_dpmac(sd, 6, (int)lane_prtcl);
		break;
	case QSGMII_B:
		wriop_init_dpmac(sd, 7, (int)lane_prtcl);
		wriop_init_dpmac(sd, 8, (int)lane_prtcl);
		wriop_init_dpmac(sd, 9, (int)lane_prtcl);
		wriop_init_dpmac(sd, 10, (int)lane_prtcl);
		break;
	}
}

#ifdef CONFIG_SYS_FSL_HAS_RGMII
void fsl_rgmii_init(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	u32 ec;

#ifdef CONFIG_SYS_FSL_EC1
	ec = gur_in32(&gur->rcwsr[FSL_CHASSIS3_EC1_REGSR])
		& FSL_CHASSIS3_RCWSR25_EC1_PRTCL_MASK;
	ec >>= FSL_CHASSIS3_RCWSR25_EC1_PRTCL_SHIFT;

	if (!ec)
		wriop_init_dpmac_enet_if(4, PHY_INTERFACE_MODE_RGMII_ID);
#endif

#ifdef CONFIG_SYS_FSL_EC2
	ec = gur_in32(&gur->rcwsr[FSL_CHASSIS3_EC2_REGSR])
		& FSL_CHASSIS3_RCWSR25_EC2_PRTCL_MASK;
	ec >>= FSL_CHASSIS3_RCWSR25_EC2_PRTCL_SHIFT;

	if (!ec)
		wriop_init_dpmac_enet_if(5, PHY_INTERFACE_MODE_RGMII_ID);
#endif
}
#endif

// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2014 Freescale Semiconductor, Inc.
 *
 * Shengzhou Liu <Shengzhou.Liu@freescale.com>
 */

#include <common.h>
#include <phy.h>
#include <fm_eth.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_serdes.h>

u32 port_to_devdisr[] = {
	[FM1_DTSEC1] = FSL_CORENET_DEVDISR2_DTSEC1_1,
	[FM1_DTSEC2] = FSL_CORENET_DEVDISR2_DTSEC1_2,
	[FM1_DTSEC3] = FSL_CORENET_DEVDISR2_DTSEC1_3,
	[FM1_DTSEC4] = FSL_CORENET_DEVDISR2_DTSEC1_4,
	[FM1_10GEC1] = FSL_CORENET_DEVDISR2_10GEC1_1, /* MAC1 */
};

static int is_device_disabled(enum fm_port port)
{
	ccsr_gur_t *gur = (void __iomem *)(CFG_SYS_MPC85xx_GUTS_ADDR);
	u32 devdisr2 = in_be32(&gur->devdisr2);

	return port_to_devdisr[port] & devdisr2;
}

void fman_disable_port(enum fm_port port)
{
	ccsr_gur_t *gur = (void __iomem *)(CFG_SYS_MPC85xx_GUTS_ADDR);

	setbits_be32(&gur->devdisr2, port_to_devdisr[port]);
}

phy_interface_t fman_port_enet_if(enum fm_port port)
{
	ccsr_gur_t *gur = (void __iomem *)(CFG_SYS_MPC85xx_GUTS_ADDR);
	u32 rcwsr13 = in_be32(&gur->rcwsr[13]);

	if (is_device_disabled(port))
		return PHY_INTERFACE_MODE_NA;

	if ((port == FM1_10GEC1) && (is_serdes_configured(XFI_FM1_MAC1)))
		return PHY_INTERFACE_MODE_XGMII;

	if ((port == FM1_DTSEC3) && ((rcwsr13 & FSL_CORENET_RCWSR13_EC2) ==
		FSL_CORENET_RCWSR13_EC2_RGMII) &&
					(!is_serdes_configured(QSGMII_FM1_A)))
		return PHY_INTERFACE_MODE_RGMII;

	if ((port == FM1_DTSEC4) && ((rcwsr13 & FSL_CORENET_RCWSR13_EC1) ==
		FSL_CORENET_RCWSR13_EC1_RGMII) &&
					(!is_serdes_configured(QSGMII_FM1_A)))
		return PHY_INTERFACE_MODE_RGMII;

	/* handle SGMII */
	switch (port) {
	case FM1_DTSEC1:
	case FM1_DTSEC2:
	case FM1_DTSEC3:
		if (is_serdes_configured(SGMII_FM1_DTSEC1 + port - FM1_DTSEC1))
			return PHY_INTERFACE_MODE_SGMII;
		else if (is_serdes_configured(SGMII_2500_FM1_DTSEC1
			 + port - FM1_DTSEC1))
			return PHY_INTERFACE_MODE_2500BASEX;
		break;
	default:
		break;
	}

	/* handle QSGMII */
	switch (port) {
	case FM1_DTSEC1:
	case FM1_DTSEC2:
	case FM1_DTSEC3:
	case FM1_DTSEC4:
		/* check lane A on SerDes1 */
		if (is_serdes_configured(QSGMII_FM1_A))
			return PHY_INTERFACE_MODE_QSGMII;
		break;
	default:
		break;
	}

	return PHY_INTERFACE_MODE_NA;
}

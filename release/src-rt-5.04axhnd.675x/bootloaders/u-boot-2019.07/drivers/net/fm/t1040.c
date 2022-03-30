// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 */
#include <common.h>
#include <phy.h>
#include <fm_eth.h>
#include <asm/io.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_serdes.h>

phy_interface_t fman_port_enet_if(enum fm_port port)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 rcwsr13 = in_be32(&gur->rcwsr[13]);

	/* handle RGMII first */
	if ((port == FM1_DTSEC2) &&
	    ((rcwsr13 & FSL_CORENET_RCWSR13_MAC2_GMII_SEL) ==
			FSL_CORENET_RCWSR13_MAC2_GMII_SEL_ENET_PORT)) {
		if ((rcwsr13 & FSL_CORENET_RCWSR13_EC1) ==
				FSL_CORENET_RCWSR13_EC1_FM1_DTSEC4_RGMII)
			return PHY_INTERFACE_MODE_RGMII;
		else if ((rcwsr13 & FSL_CORENET_RCWSR13_EC1) ==
				FSL_CORENET_RCWSR13_EC1_FM1_DTSEC4_MII)
			return PHY_INTERFACE_MODE_MII;
	}

	if ((port == FM1_DTSEC4) &&
	    ((rcwsr13 & FSL_CORENET_RCWSR13_MAC2_GMII_SEL) ==
			FSL_CORENET_RCWSR13_MAC2_GMII_SEL_L2_SWITCH)) {
		if ((rcwsr13 & FSL_CORENET_RCWSR13_EC1) ==
				FSL_CORENET_RCWSR13_EC1_FM1_DTSEC4_RGMII)
			return PHY_INTERFACE_MODE_RGMII;
		else if ((rcwsr13 & FSL_CORENET_RCWSR13_EC1) ==
				FSL_CORENET_RCWSR13_EC1_FM1_DTSEC4_MII)
			return PHY_INTERFACE_MODE_MII;
	}

	if (port == FM1_DTSEC5) {
		if ((rcwsr13 & FSL_CORENET_RCWSR13_EC2) ==
				FSL_CORENET_RCWSR13_EC2_FM1_DTSEC5_RGMII)
			return PHY_INTERFACE_MODE_RGMII;
	}

	switch (port) {
	case FM1_DTSEC1:
	case FM1_DTSEC2:
		if (is_serdes_configured(QSGMII_SW1_A + port - FM1_DTSEC1) ||
		    is_serdes_configured(SGMII_SW1_MAC1  + port - FM1_DTSEC1))
			return PHY_INTERFACE_MODE_QSGMII;
	case FM1_DTSEC3:
	case FM1_DTSEC4:
	case FM1_DTSEC5:
		if (is_serdes_configured(SGMII_FM1_DTSEC1 + port - FM1_DTSEC1))
			return PHY_INTERFACE_MODE_SGMII;
		break;
	default:
		return PHY_INTERFACE_MODE_NONE;
	}

	return PHY_INTERFACE_MODE_NONE;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2011 Freescale Semiconductor, Inc.
 */
#include <common.h>
#include <phy.h>
#include <fm_eth.h>
#include <asm/io.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_serdes.h>

static u32 port_to_devdisr[] = {
	[FM1_DTSEC1] = FSL_CORENET_DEVDISR2_DTSEC1_1,
	[FM1_DTSEC2] = FSL_CORENET_DEVDISR2_DTSEC1_2,
	[FM1_DTSEC3] = FSL_CORENET_DEVDISR2_DTSEC1_3,
	[FM1_DTSEC4] = FSL_CORENET_DEVDISR2_DTSEC1_4,
	[FM1_DTSEC5] = FSL_CORENET_DEVDISR2_DTSEC1_5,
	[FM1_10GEC1] = FSL_CORENET_DEVDISR2_10GEC1,
};

static int is_device_disabled(enum fm_port port)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 devdisr2 = in_be32(&gur->devdisr2);

	return port_to_devdisr[port] & devdisr2;
}

void fman_disable_port(enum fm_port port)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	/* don't allow disabling of DTSEC1 as its needed for MDIO */
	if (port == FM1_DTSEC1)
		return;

	setbits_be32(&gur->devdisr2, port_to_devdisr[port]);
}

void fman_enable_port(enum fm_port port)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	clrbits_be32(&gur->devdisr2, port_to_devdisr[port]);
}

phy_interface_t fman_port_enet_if(enum fm_port port)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
	u32 rcwsr11 = in_be32(&gur->rcwsr[11]);

	if (is_device_disabled(port))
		return PHY_INTERFACE_MODE_NONE;

	if ((port == FM1_10GEC1) && (is_serdes_configured(XAUI_FM1)))
		return PHY_INTERFACE_MODE_XGMII;

	/* handle RGMII first */
	if ((port == FM1_DTSEC4) && ((rcwsr11 & FSL_CORENET_RCWSR11_EC1) ==
		FSL_CORENET_RCWSR11_EC1_FM1_DTSEC4_RGMII))
		return PHY_INTERFACE_MODE_RGMII;

	if ((port == FM1_DTSEC4) && ((rcwsr11 & FSL_CORENET_RCWSR11_EC1) ==
		FSL_CORENET_RCWSR11_EC1_FM1_DTSEC4_MII))
		return PHY_INTERFACE_MODE_MII;

	if ((port == FM1_DTSEC5) && ((rcwsr11 & FSL_CORENET_RCWSR11_EC2) ==
		FSL_CORENET_RCWSR11_EC2_FM1_DTSEC5_RGMII))
		return PHY_INTERFACE_MODE_RGMII;

	if ((port == FM1_DTSEC5) && ((rcwsr11 & FSL_CORENET_RCWSR11_EC2) ==
		FSL_CORENET_RCWSR11_EC2_FM1_DTSEC5_MII))
		return PHY_INTERFACE_MODE_MII;

	switch (port) {
	case FM1_DTSEC1:
	case FM1_DTSEC2:
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

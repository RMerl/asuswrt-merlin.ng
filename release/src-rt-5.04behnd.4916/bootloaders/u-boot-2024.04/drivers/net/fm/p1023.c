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
	[FM1_DTSEC1] = MPC85xx_DEVDISR_TSEC1,
	[FM1_DTSEC2] = MPC85xx_DEVDISR_TSEC2,
};

static int is_device_disabled(enum fm_port port)
{
	ccsr_gur_t *gur = (void *)(CFG_SYS_MPC85xx_GUTS_ADDR);
	u32 devdisr = in_be32(&gur->devdisr);

	return port_to_devdisr[port] & devdisr;
}

void fman_disable_port(enum fm_port port)
{
	ccsr_gur_t *gur = (void *)(CFG_SYS_MPC85xx_GUTS_ADDR);

	/* don't allow disabling of DTSEC1 as its needed for MDIO */
	if (port == FM1_DTSEC1)
		return;

	setbits_be32(&gur->devdisr, port_to_devdisr[port]);
}

void fman_enable_port(enum fm_port port)
{
	ccsr_gur_t *gur = (void *)(CFG_SYS_MPC85xx_GUTS_ADDR);

	clrbits_be32(&gur->devdisr, port_to_devdisr[port]);
}

phy_interface_t fman_port_enet_if(enum fm_port port)
{
	ccsr_gur_t *gur = (void *)(CFG_SYS_MPC85xx_GUTS_ADDR);
	u32 pordevsr = in_be32(&gur->pordevsr);

	if (is_device_disabled(port))
		return PHY_INTERFACE_MODE_NA;

	/* DTSEC1 can be SGMII, RGMII or RMII */
	if (port == FM1_DTSEC1) {
		if (is_serdes_configured(SGMII_FM1_DTSEC1))
			return PHY_INTERFACE_MODE_SGMII;
		if (pordevsr & MPC85xx_PORDEVSR_SGMII1_DIS) {
			if (pordevsr & MPC85xx_PORDEVSR_TSEC1_PRTC)
				return PHY_INTERFACE_MODE_RGMII;
			else
				return PHY_INTERFACE_MODE_RMII;
		}
	}

	/* DTSEC2 only supports SGMII or RGMII */
	if (port == FM1_DTSEC2) {
		if (is_serdes_configured(SGMII_FM1_DTSEC2))
			return PHY_INTERFACE_MODE_SGMII;
		if (pordevsr & MPC85xx_PORDEVSR_SGMII2_DIS)
			return PHY_INTERFACE_MODE_RGMII;
	}

	return PHY_INTERFACE_MODE_NA;
}

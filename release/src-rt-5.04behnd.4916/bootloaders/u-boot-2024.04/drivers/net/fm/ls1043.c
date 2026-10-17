// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */
#include <common.h>
#include <phy.h>
#include <fm_eth.h>
#include <asm/io.h>
#include <asm/arch/fsl_serdes.h>

#define FSL_CHASSIS2_RCWSR13_EC1		0xe0000000 /* bits 416..418 */
#define FSL_CHASSIS2_RCWSR13_EC1_DTSEC3_RGMII	0x00000000
#define FSL_CHASSIS2_RCWSR13_EC1_GPIO		0x20000000
#define FSL_CHASSIS2_RCWSR13_EC1_FTM		0xa0000000
#define FSL_CHASSIS2_RCWSR13_EC2		0x1c000000 /* bits 419..421 */
#define FSL_CHASSIS2_RCWSR13_EC2_DTSEC4_RGMII	0x00000000
#define FSL_CHASSIS2_RCWSR13_EC2_GPIO		0x04000000
#define FSL_CHASSIS2_RCWSR13_EC2_1588		0x08000000
#define FSL_CHASSIS2_RCWSR13_EC2_FTM		0x14000000

u32 port_to_devdisr[] = {
	[FM1_DTSEC1] = FSL_CHASSIS2_DEVDISR2_DTSEC1_1,
	[FM1_DTSEC2] = FSL_CHASSIS2_DEVDISR2_DTSEC1_2,
	[FM1_DTSEC3] = FSL_CHASSIS2_DEVDISR2_DTSEC1_3,
	[FM1_DTSEC4] = FSL_CHASSIS2_DEVDISR2_DTSEC1_4,
	[FM1_DTSEC5] = FSL_CHASSIS2_DEVDISR2_DTSEC1_5,
	[FM1_DTSEC6] = FSL_CHASSIS2_DEVDISR2_DTSEC1_6,
	[FM1_DTSEC9] = FSL_CHASSIS2_DEVDISR2_DTSEC1_9,
	[FM1_DTSEC10] = FSL_CHASSIS2_DEVDISR2_DTSEC1_10,
	[FM1_10GEC1] = FSL_CHASSIS2_DEVDISR2_10GEC1_1,
	[FM1_10GEC2] = FSL_CHASSIS2_DEVDISR2_10GEC1_2,
	[FM1_10GEC3] = FSL_CHASSIS2_DEVDISR2_10GEC1_3,
	[FM1_10GEC4] = FSL_CHASSIS2_DEVDISR2_10GEC1_4,
};

static int is_device_disabled(enum fm_port port)
{
	struct ccsr_gur *gur = (void *)(CFG_SYS_FSL_GUTS_ADDR);
	u32 devdisr2 = in_be32(&gur->devdisr2);

	return port_to_devdisr[port] & devdisr2;
}

void fman_disable_port(enum fm_port port)
{
	struct ccsr_gur *gur = (void *)(CFG_SYS_FSL_GUTS_ADDR);

	setbits_be32(&gur->devdisr2, port_to_devdisr[port]);
}

phy_interface_t fman_port_enet_if(enum fm_port port)
{
	struct ccsr_gur *gur = (void *)(CFG_SYS_FSL_GUTS_ADDR);
	u32 rcwsr13 = in_be32(&gur->rcwsr[13]);

	if (is_device_disabled(port))
		return PHY_INTERFACE_MODE_NA;

	if ((port == FM1_10GEC1) && (is_serdes_configured(XFI_FM1_MAC9)))
		return PHY_INTERFACE_MODE_XGMII;

	if ((port == FM1_DTSEC9) && (is_serdes_configured(XFI_FM1_MAC9)))
		return PHY_INTERFACE_MODE_NA;

	if (port == FM1_DTSEC3)
		if ((rcwsr13 & FSL_CHASSIS2_RCWSR13_EC1) ==
				FSL_CHASSIS2_RCWSR13_EC1_DTSEC3_RGMII) {
			return PHY_INTERFACE_MODE_RGMII_ID;
		}
	if (port == FM1_DTSEC4)
		if ((rcwsr13 & FSL_CHASSIS2_RCWSR13_EC2) ==
				FSL_CHASSIS2_RCWSR13_EC2_DTSEC4_RGMII) {
			return PHY_INTERFACE_MODE_RGMII_ID;
		}

	/* handle SGMII */
	switch (port) {
	case FM1_DTSEC1:
	case FM1_DTSEC2:
		if ((port == FM1_DTSEC2) &&
		    is_serdes_configured(SGMII_2500_FM1_DTSEC2))
			return PHY_INTERFACE_MODE_2500BASEX;
	case FM1_DTSEC5:
	case FM1_DTSEC6:
	case FM1_DTSEC9:
		if (is_serdes_configured(SGMII_FM1_DTSEC1 + port - FM1_DTSEC1))
			return PHY_INTERFACE_MODE_SGMII;
		else if ((port == FM1_DTSEC9) &&
			 is_serdes_configured(SGMII_2500_FM1_DTSEC9))
			return PHY_INTERFACE_MODE_2500BASEX;
		break;
	default:
		break;
	}

	/* handle QSGMII */
	switch (port) {
	case FM1_DTSEC1:
	case FM1_DTSEC2:
	case FM1_DTSEC5:
	case FM1_DTSEC6:
		/* only MAC 1,2,5,6 available for QSGMII */
		if (is_serdes_configured(QSGMII_FM1_A))
			return PHY_INTERFACE_MODE_QSGMII;
		break;
	default:
		break;
	}

	return PHY_INTERFACE_MODE_NA;
}

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2012 Freescale Semiconductor, Inc.
 *	Roy Zang <tie-fei.zang@freescale.com>
 */
#include <common.h>
#include <env.h>
#include <phy.h>
#include <fm_eth.h>
#include <asm/io.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_serdes.h>
#include <hwconfig.h>

u32 port_to_devdisr[] = {
	[FM1_DTSEC1] = FSL_CORENET_DEVDISR2_DTSEC1_1,
	[FM1_DTSEC2] = FSL_CORENET_DEVDISR2_DTSEC1_2,
	[FM1_DTSEC3] = FSL_CORENET_DEVDISR2_DTSEC1_3,
	[FM1_DTSEC4] = FSL_CORENET_DEVDISR2_DTSEC1_4,
	[FM1_DTSEC5] = FSL_CORENET_DEVDISR2_DTSEC1_5,
	[FM1_DTSEC6] = FSL_CORENET_DEVDISR2_DTSEC1_6,
	[FM1_10GEC1] = FSL_CORENET_DEVDISR2_10GEC1_1,
	[FM1_10GEC2] = FSL_CORENET_DEVDISR2_10GEC1_2,
};

static int is_device_disabled(enum fm_port port)
{
	ccsr_gur_t *gur = (void *)(CFG_SYS_MPC85xx_GUTS_ADDR);
	u32 devdisr2 = in_be32(&gur->devdisr2);

	return port_to_devdisr[port] & devdisr2;
}

void fman_disable_port(enum fm_port port)
{
	ccsr_gur_t *gur = (void *)(CFG_SYS_MPC85xx_GUTS_ADDR);

	setbits_be32(&gur->devdisr2, port_to_devdisr[port]);
}

void fman_enable_port(enum fm_port port)
{
	ccsr_gur_t *gur = (void *)(CFG_SYS_MPC85xx_GUTS_ADDR);

	clrbits_be32(&gur->devdisr2, port_to_devdisr[port]);
}

phy_interface_t fman_port_enet_if(enum fm_port port)
{
#if defined(CONFIG_TARGET_B4860QDS) || defined(CONFIG_TARGET_B4420QDS)
	u32 serdes2_prtcl;
	char buffer[HWCONFIG_BUFFER_SIZE];
	char *buf = NULL;
	ccsr_gur_t *gur = (void *)(CFG_SYS_MPC85xx_GUTS_ADDR);
#endif

	if (is_device_disabled(port))
		return PHY_INTERFACE_MODE_NA;

	/*B4860 has two 10Gig Mac*/
	if ((port == FM1_10GEC1 || port == FM1_10GEC2)	&&
	    ((is_serdes_configured(XAUI_FM1_MAC9))	||
	     #if (!defined(CONFIG_TARGET_B4860QDS) && \
		  !defined(CONFIG_TARGET_B4R420QDS))
	     (is_serdes_configured(XFI_FM1_MAC9))	||
	     (is_serdes_configured(XFI_FM1_MAC10))	||
	     #endif
	     (is_serdes_configured(XAUI_FM1_MAC10))
	     ))
		return PHY_INTERFACE_MODE_XGMII;

#if defined(CONFIG_TARGET_B4860QDS) || defined(CONFIG_TARGET_B4420QDS)
	serdes2_prtcl = in_be32(&gur->rcwsr[4]) &
			FSL_CORENET2_RCWSR4_SRDS2_PRTCL;

	if (serdes2_prtcl) {
		serdes2_prtcl >>= FSL_CORENET2_RCWSR4_SRDS2_PRTCL_SHIFT;
		switch (serdes2_prtcl) {
		case 0x80:
		case 0x81:
		case 0x82:
		case 0x83:
		case 0x84:
		case 0x85:
		case 0x86:
		case 0x87:
		case 0x88:
		case 0x89:
		case 0x8a:
		case 0x8b:
		case 0x8c:
		case 0x8d:
		case 0x8e:
		case 0xb1:
		case 0xb2:
			/*
			 * Extract hwconfig from environment since environment
			 * is not setup yet
			 */
			env_get_f("hwconfig", buffer, sizeof(buffer));
			buf = buffer;

			/* check if 10GBase-R interface enable in hwconfig for 10g */
			if (hwconfig_subarg_cmp_f("fsl_b4860_serdes2",
						  "sfp_amc", "sfp", buf)) {
				if ((port == FM1_10GEC1 ||
				     port == FM1_10GEC2) &&
				    ((is_serdes_configured(XFI_FM1_MAC9)) ||
				    (is_serdes_configured(XFI_FM1_MAC10))))
					return PHY_INTERFACE_MODE_XGMII;
				else if ((port == FM1_DTSEC1) ||
					 (port == FM1_DTSEC2) ||
					 (port == FM1_DTSEC3) ||
					 (port == FM1_DTSEC4))
					return PHY_INTERFACE_MODE_NA;
			}
		}
	}
#endif

	/* Fix me need to handle RGMII here first */

	switch (port) {
	case FM1_DTSEC1:
	case FM1_DTSEC2:
	case FM1_DTSEC3:
	case FM1_DTSEC4:
	case FM1_DTSEC5:
	case FM1_DTSEC6:
		if (is_serdes_configured(SGMII_FM1_DTSEC1 + port - FM1_DTSEC1))
			return PHY_INTERFACE_MODE_SGMII;
		break;
	default:
		return PHY_INTERFACE_MODE_NA;
	}

	return PHY_INTERFACE_MODE_NA;
}

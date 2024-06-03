/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2010 Freescale Semiconductor, Inc.
 */

#ifndef __FSL_SERDES_H
#define __FSL_SERDES_H

#include <config.h>

enum srds_prtcl {
	/*
	 * Nobody will check whether the device 'NONE' has been configured,
	 * So use it to indicate if the serdes_prtcl_map has been initialized.
	 */
	NONE = 0,
	PCIE1,
	PCIE2,
	PCIE3,
	PCIE4,
	SATA1,
	SATA2,
	SRIO1,
	SRIO2,
	SGMII_FM1_DTSEC1,
	SGMII_FM1_DTSEC2,
	SGMII_FM1_DTSEC3,
	SGMII_FM1_DTSEC4,
	SGMII_FM1_DTSEC5,
	SGMII_FM1_DTSEC6,
	SGMII_FM1_DTSEC9,
	SGMII_FM1_DTSEC10,
	SGMII_FM2_DTSEC1,
	SGMII_FM2_DTSEC2,
	SGMII_FM2_DTSEC3,
	SGMII_FM2_DTSEC4,
	SGMII_FM2_DTSEC5,
	SGMII_FM2_DTSEC6,
	SGMII_FM2_DTSEC9,
	SGMII_FM2_DTSEC10,
	SGMII_TSEC1,
	SGMII_TSEC2,
	SGMII_TSEC3,
	SGMII_TSEC4,
	XAUI_FM1,
	XAUI_FM2,
	AURORA,
	CPRI1,
	CPRI2,
	CPRI3,
	CPRI4,
	CPRI5,
	CPRI6,
	CPRI7,
	CPRI8,
	XAUI_FM1_MAC9,
	XAUI_FM1_MAC10,
	XAUI_FM2_MAC9,
	XAUI_FM2_MAC10,
	HIGIG_FM1_MAC9,
	HIGIG_FM1_MAC10,
	HIGIG_FM2_MAC9,
	HIGIG_FM2_MAC10,
	QSGMII_FM1_A,		/* A indicates MACs 1-4 */
	QSGMII_FM1_B,		/* B indicates MACs 5,6,9,10 */
	QSGMII_FM2_A,
	QSGMII_FM2_B,
	XFI_FM1_MAC1,
	XFI_FM1_MAC2,
	XFI_FM1_MAC9,
	XFI_FM1_MAC10,
	XFI_FM2_MAC9,
	XFI_FM2_MAC10,
	INTERLAKEN,
	QSGMII_SW1_A,		/* Indicates ports on L2 Switch */
	QSGMII_SW1_B,
	SGMII_2500_FM1_DTSEC1,
	SGMII_2500_FM1_DTSEC2,
	SGMII_2500_FM1_DTSEC3,
	SGMII_2500_FM1_DTSEC4,
	SGMII_2500_FM1_DTSEC5,
	SGMII_2500_FM1_DTSEC6,
	SGMII_2500_FM1_DTSEC9,
	SGMII_2500_FM1_DTSEC10,
	SGMII_2500_FM2_DTSEC1,
	SGMII_2500_FM2_DTSEC2,
	SGMII_2500_FM2_DTSEC3,
	SGMII_2500_FM2_DTSEC4,
	SGMII_2500_FM2_DTSEC5,
	SGMII_2500_FM2_DTSEC6,
	SGMII_2500_FM2_DTSEC9,
	SGMII_2500_FM2_DTSEC10,
	SGMII_SW1_MAC1,
	SGMII_SW1_MAC2,
	SGMII_SW1_MAC3,
	SGMII_SW1_MAC4,
	SGMII_SW1_MAC5,
	SGMII_SW1_MAC6,
	SERDES_PRCTL_COUNT	/* Keep this item the last one */
};

enum srds {
	FSL_SRDS_1  = 0,
	FSL_SRDS_2  = 1,
	FSL_SRDS_3  = 2,
	FSL_SRDS_4  = 3,
};

int is_serdes_configured(enum srds_prtcl device);
void fsl_serdes_init(void);
const char *serdes_clock_to_string(u32 clock);

#ifdef CONFIG_FSL_CORENET
#ifdef CONFIG_SYS_FSL_QORIQ_CHASSIS2
int serdes_get_first_lane(u32 sd, enum srds_prtcl device);
enum srds_prtcl serdes_get_prtcl(int serdes, int cfg, int lane);
#else
int serdes_get_first_lane(enum srds_prtcl device);
#endif
#ifdef CONFIG_SYS_P4080_ERRATUM_SERDES9
void serdes_reset_rx(enum srds_prtcl device);
#endif
#endif

#endif /* __FSL_SERDES_H */

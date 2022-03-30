/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018 NXP
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#ifndef __FSL_SERDES_H__
#define __FSL_SERDES_H__

#include <config.h>

#ifdef CONFIG_FSL_LSCH3
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
	PCIE5,
	PCIE6,
	SATA1,
	SATA2,
	SATA3,
	SATA4,
	XAUI1,
	XAUI2,
	XFI1,
	XFI2,
	XFI3,
	XFI4,
	XFI5,
	XFI6,
	XFI7,
	XFI8,
	XFI9,
	XFI10,
	XFI11,
	XFI12,
	XFI13,
	XFI14,
	SGMII1,
	SGMII2,
	SGMII3,
	SGMII4,
	SGMII5,
	SGMII6,
	SGMII7,
	SGMII8,
	SGMII9,
	SGMII10,
	SGMII11,
	SGMII12,
	SGMII13,
	SGMII14,
	SGMII15,
	SGMII16,
	SGMII17,
	SGMII18,
	QSGMII_A,
	QSGMII_B,
	QSGMII_C,
	QSGMII_D,
	SGMII_T1,
	SGMII_T2,
	SGMII_T3,
	SGMII_T4,
	SXGMII1,
	SXGMII2,
	SXGMII3,
	SXGMII4,
	QXGMII1,
	QXGMII2,
	QXGMII3,
	QXGMII4,
	_25GE1,
	_25GE2,
	_25GE3,
	_25GE4,
	_25GE5,
	_25GE6,
	_25GE7,
	_25GE8,
	_25GE9,
	_25GE10,
	_40GE1,
	_40GE2,
	_50GE1,
	_50GE2,
	_100GE1,
	_100GE2,
	SERDES_PRCTL_COUNT
};

enum srds {
	FSL_SRDS_1  = 0,
	FSL_SRDS_2  = 1,
	NXP_SRDS_3  = 2,
};
#elif defined(CONFIG_FSL_LSCH2)
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
	QSGMII_FM1_A,		/* A indicates MACs 1,2,5,6 */
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
	TX_CLK,
	SERDES_PRCTL_COUNT
};

enum srds {
	FSL_SRDS_1  = 0,
	FSL_SRDS_2  = 1,
};

#endif

int is_serdes_configured(enum srds_prtcl device);
void fsl_serdes_init(void);
int serdes_get_first_lane(u32 sd, enum srds_prtcl device);
enum srds_prtcl serdes_get_prtcl(int serdes, int cfg, int lane);
int is_serdes_prtcl_valid(int serdes, u32 prtcl);
int serdes_get_number(int serdes, int cfg);
void fsl_rgmii_init(void);

#ifdef CONFIG_FSL_LSCH2
const char *serdes_clock_to_string(u32 clock);
int get_serdes_protocol(void);
#endif
#ifdef CONFIG_SYS_HAS_SERDES
/* Get the volt of SVDD in unit mV */
int get_serdes_volt(void);
/* Set the volt of SVDD in unit mV */
int set_serdes_volt(int svdd);
/* The target volt of SVDD in unit mV */
int setup_serdes_volt(u32 svdd);
#endif

#endif /* __FSL_SERDES_H__ */

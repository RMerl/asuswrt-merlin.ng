/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * USB 3.0 DRD Controller
 *
 * (C) Copyright 2012-2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#define USB3_PHY_REF_SSP_EN		BIT(29)
#define USB3_PHY_OTG_VBUSVLDECTSEL	BIT(16)

/* KEYSTONE2 XHCI PHY register structure */
struct keystone_xhci_phy {
	unsigned int phy_utmi;		/* ctl0 */
	unsigned int phy_pipe;		/* ctl1 */
	unsigned int phy_param_ctrl_1;	/* ctl2 */
	unsigned int phy_param_ctrl_2;	/* ctl3 */
	unsigned int phy_clock;		/* ctl4 */
	unsigned int phy_pll;		/* ctl5 */
};

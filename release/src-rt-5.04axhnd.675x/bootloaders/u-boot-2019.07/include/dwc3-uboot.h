/* SPDX-License-Identifier: GPL-2.0 */
/* include/dwc3-uboot.h
 *
 * Copyright (c) 2015 Texas Instruments Incorporated - http://www.ti.com
 *
 * Designware SuperSpeed USB uboot init
 */

#ifndef __DWC3_UBOOT_H_
#define __DWC3_UBOOT_H_

#include <linux/usb/otg.h>

struct dwc3_device {
	unsigned long base;
	enum usb_dr_mode dr_mode;
	u32 maximum_speed;
	unsigned tx_fifo_resize:1;
	unsigned has_lpm_erratum;
	u8 lpm_nyet_threshold;
	unsigned is_utmi_l1_suspend;
	u8 hird_threshold;
	unsigned disable_scramble_quirk;
	unsigned u2exit_lfps_quirk;
	unsigned u2ss_inp3_quirk;
	unsigned req_p1p2p3_quirk;
	unsigned del_p1p2p3_quirk;
	unsigned del_phy_power_chg_quirk;
	unsigned lfps_filter_quirk;
	unsigned rx_detect_poll_quirk;
	unsigned dis_u3_susphy_quirk;
	unsigned dis_u2_susphy_quirk;
	unsigned tx_de_emphasis_quirk;
	unsigned tx_de_emphasis;
	int index;
};

int dwc3_uboot_init(struct dwc3_device *dev);
void dwc3_uboot_exit(int index);
void dwc3_uboot_handle_interrupt(int index);

struct phy;
#if CONFIG_IS_ENABLED(PHY) && CONFIG_IS_ENABLED(DM_USB)
int dwc3_setup_phy(struct udevice *dev, struct phy **array, int *num_phys);
int dwc3_shutdown_phy(struct udevice *dev, struct phy *usb_phys, int num_phys);
#else
static inline int dwc3_setup_phy(struct udevice *dev, struct phy **array,
				 int *num_phys)
{
	return -ENOTSUPP;
}

static inline int dwc3_shutdown_phy(struct udevice *dev, struct phy *usb_phys,
				    int num_phys)
{
	return -ENOTSUPP;
}
#endif

#endif /* __DWC3_UBOOT_H_ */

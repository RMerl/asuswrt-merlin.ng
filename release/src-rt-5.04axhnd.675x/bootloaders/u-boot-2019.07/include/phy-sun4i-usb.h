/*
 * Copyright (C) 2017 Jagan Teki <jagan@amarulasolutions.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __GENERIC_PHY_SUN4I_USB_H
#define __GENERIC_PHY_SUN4I_USB_H

/**
 * sun4i_usb_phy_id_detect - detect ID pin of USB PHY
 *
 * @phy:	USB PHY port to detect ID pin
 * @return 0 if OK, or a negative error code
 */
int sun4i_usb_phy_id_detect(struct phy *phy);

/**
 * sun4i_usb_phy_vbus_detect - detect VBUS pin of USB PHY
 *
 * @phy:	USB PHY port to detect VBUS pin
 * @return 0 if OK, or a negative error code
 */
int sun4i_usb_phy_vbus_detect(struct phy *phy);

/**
 * sun4i_usb_phy_set_squelch_detect() - Enable/disable squelch detect
 *
 * @phy: reference to a sun4i usb phy
 * @enabled: wether to enable or disable squelch detect
 */
void sun4i_usb_phy_set_squelch_detect(struct phy *phy, bool enabled);

#endif /*__GENERIC_PHY_SUN4I_USB_H */

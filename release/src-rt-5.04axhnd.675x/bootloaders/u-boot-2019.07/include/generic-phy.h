/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2017 Texas Instruments Incorporated - http://www.ti.com/
 * Written by Jean-Jacques Hiblot  <jjhiblot@ti.com>
 */

#ifndef __GENERIC_PHY_H
#define __GENERIC_PHY_H

struct ofnode_phandle_args;

/**
 * struct phy - A handle to (allowing control of) a single phy port.
 *
 * Clients provide storage for phy handles. The content of the structure is
 * managed solely by the PHY API and PHY drivers. A phy struct is
 * initialized by "get"ing the phy struct. The phy struct is passed to all
 * other phy APIs to identify which PHY port to operate upon.
 *
 * @dev: The device which implements the PHY port.
 * @id: The PHY ID within the provider.
 *
 */
struct phy {
	struct udevice *dev;
	unsigned long id;
};

/*
 * struct udevice_ops - set of function pointers for phy operations
 * @init: operation to be performed for initializing phy (optional)
 * @exit: operation to be performed while exiting (optional)
 * @reset: reset the phy (optional).
 * @power_on: powering on the phy (optional)
 * @power_off: powering off the phy (optional)
 */
struct phy_ops {
	/**
	 * of_xlate - Translate a client's device-tree (OF) phy specifier.
	 *
	 * The PHY core calls this function as the first step in implementing
	 * a client's generic_phy_get_by_*() call.
	 *
	 * If this function pointer is set to NULL, the PHY core will use a
	 * default implementation, which assumes #phy-cells = <0> or
	 * #phy-cells = <1>, and in the later case that the DT cell
	 * contains a simple integer PHY port ID.
	 *
	 * @phy:	The phy struct to hold the translation result.
	 * @args:	The phy specifier values from device tree.
	 * @return 0 if OK, or a negative error code.
	 */
	int	(*of_xlate)(struct phy *phy, struct ofnode_phandle_args *args);

	/**
	 * init - initialize the hardware.
	 *
	 * Hardware intialization should not be done in during probe() but
	 * should be implemented in this init() function. It could be starting
	 * PLL, taking a controller out of reset, routing, etc. This function
	 * is typically called only once per PHY port.
	 * If power_on() is not implemented, it must power up the phy.
	 *
	 * @phy:	the PHY port to initialize
	 * @return 0 if OK, or a negative error code.
	 */
	int	(*init)(struct phy *phy);

	/**
	* exit - de-initialize the PHY device
	*
	* Hardware de-intialization should be done here. Every step done in
	* init() should be undone here.
	* This could be used to suspend the phy to reduce power consumption or
	* to put the phy in a known condition before booting the OS (though it
	* is NOT called automatically before booting the OS)
	* If power_off() is not implemented, it must power down the phy.
	*
	* @phy:	PHY port to be de-initialized
	* @return 0 if OK, or a negative error code
	*/
	int	(*exit)(struct phy *phy);

	/**
	* reset - resets a PHY device without shutting down
	*
	* @phy:	PHY port to be reset
	*
	* During runtime, the PHY may need to be reset in order to
	* re-establish connection etc without being shut down or exit.
	*
	* @return 0 if OK, or a negative error code
	*/
	int	(*reset)(struct phy *phy);

	/**
	* power_on - power on a PHY device
	*
	* @phy:	PHY port to be powered on
	*
	* During runtime, the PHY may need to be powered on or off several
	* times. This function is used to power on the PHY. It relies on the
	* setup done in init(). If init() is not implemented, it must take care
	* of setting up the context (PLLs, ...)
	*
	* @return 0 if OK, or a negative error code
	*/
	int	(*power_on)(struct phy *phy);

	/**
	* power_off - power off a PHY device
	*
	* @phy:	PHY port to be powered off
	*
	* During runtime, the PHY may need to be powered on or off several
	* times. This function is used to power off the PHY. Except if
	* init()/deinit() are not implemented, it must not de-initialize
	* everything.
	*
	* @return 0 if OK, or a negative error code
	*/
	int	(*power_off)(struct phy *phy);
};

#ifdef CONFIG_PHY

/**
 * generic_phy_init() - initialize the PHY port
 *
 * @phy:	the PHY port to initialize
 * @return 0 if OK, or a negative error code
 */
int generic_phy_init(struct phy *phy);

/**
 * generic_phy_init() - de-initialize the PHY device
 *
 * @phy:	PHY port to be de-initialized
 * @return 0 if OK, or a negative error code
 */
int generic_phy_exit(struct phy *phy);

/**
 * generic_phy_reset() - resets a PHY device without shutting down
 *
 * @phy:	PHY port to be reset
 *@return 0 if OK, or a negative error code
 */
int generic_phy_reset(struct phy *phy);

/**
 * generic_phy_power_on() - power on a PHY device
 *
 * @phy:	PHY port to be powered on
 * @return 0 if OK, or a negative error code
 */
int generic_phy_power_on(struct phy *phy);

/**
 * generic_phy_power_off() - power off a PHY device
 *
 * @phy:	PHY port to be powered off
 * @return 0 if OK, or a negative error code
 */
int generic_phy_power_off(struct phy *phy);


/**
 * generic_phy_get_by_index() - Get a PHY device by integer index.
 *
 * @user:	the client device
 * @index:	The index in the list of available PHYs
 * @phy:	A pointer to the PHY port
 *
 * This looks up a PHY device for a client device based on its position in the
 * list of the possible PHYs.
 *
 * example:
 * usb1: usb_otg_ss@xxx {
 *       compatible = "xxx";
 *       reg = <xxx>;
 *   .
 *   .
 *   phys = <&usb2_phy>, <&usb3_phy>;
 *   .
 *   .
 * };
 * the USB2 phy can be accessed by passing index '0' and the USB3 phy can
 * be accessed by passing index '1'
 *
 * @return 0 if OK, or a negative error code
 */
int generic_phy_get_by_index(struct udevice *user, int index,
			     struct phy *phy);

/**
 * generic_phy_get_by_name() - Get a PHY device by its name.
 *
 * @user:	the client device
 * @phy_name:	The name of the PHY in the list of possible PHYs
 * @phy:	A pointer to the PHY port
 *
 * This looks up a PHY device for a client device in the
 * list of the possible PHYs based on its name.
 *
 * example:
 * usb1: usb_otg_ss@xxx {
 *       compatible = "xxx";
 *       reg = <xxx>;
 *   .
 *   .
 *   phys = <&usb2_phy>, <&usb3_phy>;
 *   phy-names = "usb2phy", "usb3phy";
 *   .
 *   .
 * };
 * the USB3 phy can be accessed using "usb3phy", and USB2 by using "usb2phy"
 *
 * @return 0 if OK, or a negative error code
 */
int generic_phy_get_by_name(struct udevice *user, const char *phy_name,
			    struct phy *phy);

#else /* CONFIG_PHY */

static inline int generic_phy_init(struct phy *phy)
{
	return 0;
}

static inline int generic_phy_exit(struct phy *phy)
{
	return 0;
}

static inline int generic_phy_reset(struct phy *phy)
{
	return 0;
}

static inline int generic_phy_power_on(struct phy *phy)
{
	return 0;
}

static inline int generic_phy_power_off(struct phy *phy)
{
	return 0;
}

static inline int generic_phy_get_by_index(struct udevice *user, int index,
			     struct phy *phy)
{
	return 0;
}

static inline int generic_phy_get_by_name(struct udevice *user, const char *phy_name,
			    struct phy *phy)
{
	return 0;
}

#endif /* CONFIG_PHY */

/**
 * generic_phy_valid() - check if PHY port is valid
 *
 * @phy:	the PHY port to check
 * @return TRUE if valid, or FALSE
 */
static inline bool generic_phy_valid(struct phy *phy)
{
	return phy->dev != NULL;
}

#endif /*__GENERIC_PHY_H */

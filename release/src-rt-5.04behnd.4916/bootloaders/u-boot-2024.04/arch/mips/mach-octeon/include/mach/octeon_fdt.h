/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 */

#ifndef __OCTEON_FDT_H__
#define __OCTEON_FDT_H__

struct phy_device;

/** Type of GPIO pin */
enum octeon_gpio_type {
	GPIO_TYPE_OCTEON,  /** Native Octeon */
	GPIO_TYPE_PCA953X, /** PCA953X i2c GPIO expander */
	GPIO_TYPE_PCA9554, /** PCA9554 i2c GPIO expander */
	GPIO_TYPE_PCA9555, /** PCA9555 i2c GPIO expander */
	GPIO_TYPE_PCA9698, /** PCA9698 i2c GPIO expander */
#ifdef CONFIG_PHY_VITESSE
	GPIO_TYPE_VSC8488, /** Vitesse VSC8488 or related PHY GPIO */
#endif
	GPIO_TYPE_UNKNOWN /** Unknown GPIO type */
};

/**
 * Trims nodes from the flat device tree.
 *
 * @param fdt - pointer to working FDT, usually in gd->fdt_blob
 * @param fdt_key - key to preserve.  All non-matching keys are removed
 * @param trim_name - name of property to look for.  If NULL use
 *		      'cavium,qlm-trim'
 * @param rename - set to TRUE to rename interfaces.
 * @param callback - function to call on matched nodes.
 * @param cbarg - passed to callback.
 *
 * The key should look something like device #, type where device # is a
 * number from 0-9 and type is a string describing the type.  For QLM
 * operations this would typically contain the QLM number followed by
 * the type in the device tree, like "0,xaui", "0,sgmii", etc.  This function
 * will trim all items in the device tree which match the device number but
 * have a type which does not match.  For example, if a QLM has a xaui module
 * installed on QLM 0 and "0,xaui" is passed as a key, then all FDT nodes that
 * have "0,xaui" will be preserved but all others, i.e. "0,sgmii" will be
 * removed.
 *
 * Note that the trim_name must also match.  If trim_name is NULL then it
 * looks for the property "cavium,qlm-trim".
 *
 * Also, when the trim_name is "cavium,qlm-trim" or NULL that the interfaces
 * will also be renamed based on their register values.
 *
 * For example, if a PIP interface is named "interface@W" and has the property
 * reg = <0> then the interface will be renamed after this function to
 * interface@0.
 *
 * Return: 0 for success.
 */
int octeon_fdt_patch_rename(void *fdt, const char *fdt_key, const char *trim_name, bool rename,
			    void (*callback)(void *fdt, int offset, void *arg), void *cbarg);

/**
 * Trims nodes from the flat device tree.
 *
 * @param fdt - pointer to working FDT, usually in gd->fdt_blob
 * @param fdt_key - key to preserve.  All non-matching keys are removed
 * @param trim_name - name of property to look for.  If NULL use
 *		      'cavium,qlm-trim'
 *
 * The key should look something like device #, type where device # is a
 * number from 0-9 and type is a string describing the type.  For QLM
 * operations this would typically contain the QLM number followed by
 * the type in the device tree, like "0,xaui", "0,sgmii", etc.  This function
 * will trim all items in the device tree which match the device number but
 * have a type which does not match.  For example, if a QLM has a xaui module
 * installed on QLM 0 and "0,xaui" is passed as a key, then all FDT nodes that
 * have "0,xaui" will be preserved but all others, i.e. "0,sgmii" will be
 * removed.
 *
 * Note that the trim_name must also match.  If trim_name is NULL then it
 * looks for the property "cavium,qlm-trim".
 *
 * Also, when the trim_name is "cavium,qlm-trim" or NULL that the interfaces
 * will also be renamed based on their register values.
 *
 * For example, if a PIP interface is named "interface@W" and has the property
 * reg = <0> then the interface will be renamed after this function to
 * interface@0.
 *
 * Return: 0 for success.
 */
int octeon_fdt_patch(void *fdt, const char *fdt_key, const char *trim_name);

/**
 * Fix up the MAC address in the flat device tree based on the MAC address
 * stored in ethaddr or in the board descriptor.
 *
 * NOTE: This function is weak and an alias for __octeon_fixup_fdt_mac_addr.
 */
void octeon_fixup_fdt_mac_addr(void);

/**
 * This function fixes the clock-frequency in the flat device tree for the UART.
 *
 * NOTE: This function is weak and an alias for __octeon_fixup_fdt_uart.
 */
void octeon_fixup_fdt_uart(void);

/**
 * This function fills in the /memory portion of the flat device tree.
 *
 * NOTE: This function is weak and aliased to __octeon_fixup_fdt_memory.
 */
void octeon_fixup_fdt_memory(void);

int board_fixup_fdt(void);

void octeon_fixup_fdt(void);

/**
 * This is a helper function to find the offset of a PHY device given
 * an Ethernet device.
 *
 * @param[in] eth - Ethernet device to search for PHY offset
 *
 * @returns offset of phy info in device tree or -1 if not found
 */
int octeon_fdt_find_phy(const struct udevice *eth);

/**
 * This helper function returns if a node contains the specified vendor name.
 *
 * @param[in]	fdt		pointer to device tree blob
 * @param	nodeoffset	offset of the tree node
 * @param[in]	vendor		name of vendor to check
 *
 * returns:
 *	0, if the node has a compatible vendor string property
 *	1, if the node does not contain the vendor string property
 *	-FDT_ERR_NOTFOUND, if the given node has no 'compatible' property
 *	-FDT_ERR_BADOFFSET, if nodeoffset does not refer to a BEGIN_NODE tag
 *	-FDT_ERR_BADMAGIC,
 *	-FDT_ERR_BADVERSION,
 *	-FDT_BADSTATE,
 *	-FDT_ERR_BADSTRUCTURE, standard meanings
 */
int octeon_fdt_compat_vendor(const void *fdt, int nodeoffset, const char *vendor);

/**
 * Given a node in the device tree get the OCTEON OCX node number
 *
 * @param fdt		pointer to flat device tree
 * @param nodeoffset	node offset to get OCX node for
 *
 * Return: the Octeon OCX node number
 */
int octeon_fdt_get_soc_node(const void *fdt, int nodeoffset);

/**
 * Given a FDT node, check if it is compatible with a list of devices
 *
 * @param[in]	fdt		Flat device tree pointer
 * @param	node_offset	Node offset in device tree
 * @param[in]	strlist		Array of FDT devices to check, end must be NULL
 *
 * Return:	0 if at least one device is compatible, 1 if not compatible.
 */
int octeon_fdt_node_check_compatible(const void *fdt, int node_offset, const char *const *strlist);
/**
 * Given a node offset, find the i2c bus number for that node
 *
 * @param[in]	fdt	Pointer to flat device tree
 * @param	node_offset	Node offset in device tree
 *
 * Return:	i2c bus number or -1 if error
 */
int octeon_fdt_i2c_get_bus(const void *fdt, int node_offset);

/**
 * Given an offset into the fdt, output the i2c bus and address of the device
 *
 * @param[in]	fdt	fdt blob pointer
 * @param	node	offset in FDT of device
 * @param[out]	bus	i2c bus number of device
 * @param[out]	addr	address of device on i2c bus
 *
 * Return:	0 for success, -1 on error
 */
int octeon_fdt_get_i2c_bus_addr(const void *fdt, int node, int *bus, int *addr);

/**
 * Reads a GPIO pin given the node of the GPIO device in the device tree and
 * the pin number.
 *
 * @param[in]	fdt	fdt blob pointer
 * @param	phandle	phandle of GPIO node
 * @param	pin	pin number to read
 *
 * Return:	0 = pin is low, 1 = pin is high, -1 = error
 */
int octeon_fdt_read_gpio(const void *fdt, int phandle, int pin);

/**
 * Reads a GPIO pin given the node of the GPIO device in the device tree and
 * the pin number.
 *
 * @param[in]	fdt	fdt blob pointer
 * @param	phandle	phandle of GPIO node
 * @param	pin	pin number to read
 * @param	val	value to write (1 = high, 0 = low)
 *
 * Return:	0 = success, -1 = error
 */
int octeon_fdt_set_gpio(const void *fdt, int phandle, int pin, int val);

/**
 * Given the node to a MAC entry in the device tree, output the i2c bus, address
 * and if the module is absent.
 *
 * @param[in]	fdt		flat device tree pointer
 * @param	mac_node	node of Ethernet port in the FDT
 * @param[out]	bus		i2c bus address of SFP EEPROM
 * @param[out]	addr		i2c address of SFP EEPROM
 * @param[out]	mod_abs		Set true if module is absent, false if present
 *
 * Return:	0 for success, -1 if there are problems with the device tree
 */
int octeon_fdt_get_sfp_eeprom(const void *fdt, int mac_node, int *bus, int *addr, bool *mod_abs);

/**
 * Given a node to a MAC entry in the device tree, output the i2c bus, address
 * and if the module is absent
 *
 * @param[in]	fdt		flat device tree pointer
 * @param	mac_node	node of QSFP Ethernet port in FDT
 * @param[out]	bus		i2c bus address of SFP EEPROM
 * @param[out]	addr		i2c address of SFP eeprom
 * @param[out]	mod_abs		Set true if module is absent, false if present
 *
 * Return:	0 for success, -1 if there are problems with the device tree
 */
int octeon_fdt_get_qsfp_eeprom(const void *fdt, int mac_node, int *bus, int *addr, bool *mod_abs);

/**
 * Given the node of a GPIO entry output the GPIO type, i2c bus and i2c
 * address.
 *
 * @param	fdt_node	node of GPIO in device tree, generally
 *				derived from a phandle.
 * @param[out]	type		Type of GPIO detected
 * @param[out]	i2c_bus		For i2c GPIO expanders, the i2c bus number
 * @param[out]	i2c_addr	For i2c GPIO expanders, the i2c address
 *
 * Return:	0 for success, -1 for errors
 *
 * NOTE: It is up to the caller to determine the pin number.
 */
int octeon_fdt_get_gpio_info(int fdt_node, enum octeon_gpio_type *type, int *i2c_bus,
			     int *i2c_addr);

/**
 * Get the PHY data structure for the specified FDT node and output the type
 *
 * @param	fdt_node	FDT node of phy
 * @param[out]	type		Type of GPIO
 *
 * Return:	pointer to phy device or NULL if no match found.
 */
struct phy_device *octeon_fdt_get_phy_gpio_info(int fdt_node, enum octeon_gpio_type *type);
#endif /* __OCTEON_FDT_H__ */

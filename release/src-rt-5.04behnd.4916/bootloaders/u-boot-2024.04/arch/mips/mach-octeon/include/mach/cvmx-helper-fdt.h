/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * FDT Helper functions similar to those provided to U-Boot.
 * If compiled for U-Boot, just provide wrappers to the equivalent U-Boot
 * functions.
 */

#ifndef __CVMX_HELPER_FDT_H__
#define __CVMX_HELPER_FDT_H__

#include <fdt_support.h>
#include <fdtdec.h>
#include <time.h>
#include <asm/global_data.h>
#include <asm-generic/gpio.h>
#include <dm/device.h>
#include <linux/libfdt.h>

#include <mach/cvmx-helper-sfp.h>

/* todo: this is deprecated and some of it can be removed at some time */
enum cvmx_i2c_bus_type {
	CVMX_I2C_BUS_OCTEON,
	CVMX_I2C_MUX_PCA9540,
	CVMX_I2C_MUX_PCA9542,
	CVMX_I2C_MUX_PCA9543,
	CVMX_I2C_MUX_PCA9544,
	CVMX_I2C_MUX_PCA9545,
	CVMX_I2C_MUX_PCA9546,
	CVMX_I2C_MUX_PCA9547,
	CVMX_I2C_MUX_PCA9548,
	CVMX_I2C_MUX_OTHER
};

struct cvmx_sfp_mod_info; /** Defined in cvmx-helper-sfp.h */
struct cvmx_phy_info;	  /** Defined in cvmx-helper-board.h */

/**
 * This data structure holds information about various I2C muxes and switches
 * that may be between a device and the Octeon chip.
 */
struct cvmx_fdt_i2c_bus_info {
	/** Parent I2C bus, NULL if root */
	struct cvmx_fdt_i2c_bus_info *parent;
	/** Child I2C bus or NULL if last entry in the chain */
	struct cvmx_fdt_i2c_bus_info *child;
	/** Offset in device tree */
	int of_offset;
	/** Type of i2c bus or mux */
	enum cvmx_i2c_bus_type type;
	/** I2C address of mux */
	u8 i2c_addr;
	/** Mux channel number */
	u8 channel;
	/** For muxes, the bit(s) to set to enable them */
	u8 enable_bit;
	/** True if mux, false if switch */
	bool is_mux;

	struct udevice *i2c_bus;
};

/**
 * Data structure containing information about SFP/QSFP slots
 */
struct cvmx_fdt_sfp_info {
	/** Used for a linked list of slots */
	struct cvmx_fdt_sfp_info *next, *prev;
	/** Used when multiple SFP ports share the same IPD port */
	struct cvmx_fdt_sfp_info *next_iface_sfp;
	/** Name from device tree of slot */
	const char *name;
	/** I2C bus for slot EEPROM */
	struct cvmx_fdt_i2c_bus_info *i2c_bus;
	/** Data from SFP or QSFP EEPROM */
	struct cvmx_sfp_mod_info sfp_info;
	/** Data structure with PHY information */
	struct cvmx_phy_info *phy_info;
	/** IPD port(s) slot is connected to */
	int ipd_port[4];
	/** Offset in device tree of slot */
	int of_offset;
	/** EEPROM address of SFP module (usually 0x50) */
	u8 i2c_eeprom_addr;
	/** Diagnostic address of SFP module (usually 0x51) */
	u8 i2c_diag_addr;
	/** True if QSFP module */
	bool is_qsfp;
	/** True if EEPROM data is valid */
	bool valid;

	/** SFP tx_disable GPIO descriptor */
	struct gpio_desc tx_disable;
	/** SFP mod_abs/QSFP mod_prs GPIO descriptor */
	struct gpio_desc mod_abs;
	/** SFP tx_error GPIO descriptor */
	struct gpio_desc tx_error;
	/** SFP rx_los GPIO discriptor */
	struct gpio_desc rx_los;
	/** QSFP select GPIO descriptor */
	struct gpio_desc select;
	/** QSFP reset GPIO descriptor */
	struct gpio_desc reset;
	/** QSFP interrupt GPIO descriptor */
	struct gpio_desc interrupt;
	/** QSFP lp_mode GPIO descriptor */
	struct gpio_desc lp_mode;

	/** Last mod_abs value */
	int last_mod_abs;
	/** Last rx_los value */
	int last_rx_los;
	/** Function to call to check mod_abs */
	int (*check_mod_abs)(struct cvmx_fdt_sfp_info *sfp_info, void *data);
	/** User-defined data to pass to check_mod_abs */
	void *mod_abs_data;
	/** Function to call when mod_abs changes */
	int (*mod_abs_changed)(struct cvmx_fdt_sfp_info *sfp_info, int val, void *data);
	/** User-defined data to pass to mod_abs_changed */
	void *mod_abs_changed_data;
	/** Function to call when rx_los changes */
	int (*rx_los_changed)(struct cvmx_fdt_sfp_info *sfp_info, int val, void *data);
	/** User-defined data to pass to rx_los_changed */
	void *rx_los_changed_data;
	/** True if we're connected to a Microsemi VSC7224 reclocking chip */
	bool is_vsc7224;
	/** Data structure for first vsc7224 channel we're attached to */
	struct cvmx_vsc7224_chan *vsc7224_chan;
	/** True if we're connected to a Avago AVSP5410 phy */
	bool is_avsp5410;
	/** Data structure for avsp5410 phy we're attached to */
	struct cvmx_avsp5410 *avsp5410;
	/** xinterface we're on */
	int xiface;
	/** port index */
	int index;
};

/**
 * Look up a phandle and follow it to its node then return the offset of that
 * node.
 *
 * @param[in]	fdt_addr	pointer to FDT blob
 * @param	node		node to read phandle from
 * @param[in]	prop_name	name of property to find
 * @param[in,out] lenp		Number of phandles, input max number
 * @param[out]	nodes		Array of phandle nodes
 *
 * Return:	-ve error code on error or 0 for success
 */
int cvmx_fdt_lookup_phandles(const void *fdt_addr, int node, const char *prop_name, int *lenp,
			     int *nodes);

int cvmx_ofnode_lookup_phandles(ofnode node, const char *prop_name,
				int *lenp, ofnode *nodes);

/**
 * Helper to return the address property
 *
 * @param[in] fdt_addr	pointer to FDT blob
 * @param node		node to read address from
 * @param prop_name	property name to read
 *
 * Return: address of property or FDT_ADDR_T_NONE if not found
 */
static inline fdt_addr_t cvmx_fdt_get_addr(const void *fdt_addr, int node, const char *prop_name)
{
	return fdtdec_get_addr(fdt_addr, node, prop_name);
}

/**
 * Helper function to return an integer property
 *
 * @param[in] fdt_addr	pointer to FDT blob
 * @param node		node to read integer from
 * @param[in] prop_name	property name to read
 * @param default_val	default value to return if property doesn't exist
 *
 * Return:	integer value of property or default_val if it doesn't exist.
 */
static inline int cvmx_fdt_get_int(const void *fdt_addr, int node, const char *prop_name,
				   int default_val)
{
	return fdtdec_get_int(fdt_addr, node, prop_name, default_val);
}

static inline bool cvmx_fdt_get_bool(const void *fdt_addr, int node, const char *prop_name)
{
	return fdtdec_get_bool(fdt_addr, node, prop_name);
}

static inline u64 cvmx_fdt_get_uint64(const void *fdt_addr, int node, const char *prop_name,
				      u64 default_val)
{
	return fdtdec_get_uint64(fdt_addr, node, prop_name, default_val);
}

/**
 * Look up a phandle and follow it to its node then return the offset of that
 * node.
 *
 * @param[in] fdt_addr	pointer to FDT blob
 * @param node		node to read phandle from
 * @param[in] prop_name	name of property to find
 *
 * Return:	node offset if found, -ve error code on error
 */
static inline int cvmx_fdt_lookup_phandle(const void *fdt_addr, int node, const char *prop_name)
{
	return fdtdec_lookup_phandle(fdt_addr, node, prop_name);
}

/**
 * Translate an address from the device tree into a CPU physical address by
 * walking up the device tree and applying bus mappings along the way.
 *
 * This uses #size-cells and #address-cells.
 *
 * @param[in]	fdt_addr	Address of flat device tree
 * @param	node		node to start translating from
 * @param[in]	in_addr		Address to translate
 *				NOTE: in_addr must be in the native ENDIAN
 *				format.
 *
 * Return:	Translated address or FDT_ADDR_T_NONE if address cannot be
 *		translated.
 */
static inline u64 cvmx_fdt_translate_address(const void *fdt_addr, int node, const u32 *in_addr)
{
	return fdt_translate_address((void *)fdt_addr, node, in_addr);
}

/**
 * Compare compatibile strings in the flat device tree.
 *
 * @param[in] s1	First string to compare
 * @param[in] sw	Second string to compare
 *
 * Return:	0 if no match
 *		1 if only the part number matches and not the manufacturer
 *		2 if both the part number and manufacturer match
 */
int cvmx_fdt_compat_match(const char *s1, const char *s2);

/**
 * Returns whether a list of strings contains the specified string
 *
 * @param[in]	slist	String list
 * @param	llen	string list total length
 * @param[in]	str	string to search for
 *
 * Return:	1 if string list contains string, 0 if it does not.
 */
int cvmx_fdt_compat_list_contains(const char *slist, int llen, const char *str);

/**
 * Check if a node is compatible with the specified compat string
 *
 * @param[in]	fdt_addr	FDT address
 * @param	node		node offset to check
 * @param[in]	compat		compatible string to check
 *
 * Return:	0 if compatible, 1 if not compatible, error if negative
 */
int cvmx_fdt_node_check_compatible(const void *fdt_addr, int node, const char *compat);

/**
 * @INTERNAL
 * Compares a string to a compatible field.
 *
 * @param[in]	compat		compatible string
 * @param[in]	str		string to check
 *
 * Return:	0 if not compatible, 1 if manufacturer compatible, 2 if
 *		part is compatible, 3 if both part and manufacturer are
 *		compatible.
 */
int __cvmx_fdt_compat_match(const char *compat, const char *str);

/**
 * Given a phandle to a GPIO device return the type of GPIO device it is.
 *
 * @param[in]	fdt_addr	Address of flat device tree
 * @param	phandle		phandle to GPIO
 * @param[out]	size		Number of pins (optional, may be NULL)
 *
 * Return:	Type of GPIO device or PIN_ERROR if error
 */
enum cvmx_gpio_type cvmx_fdt_get_gpio_type(const void *fdt_addr, int phandle, int *size);

/**
 * Given a phandle to a GPIO node output the i2c bus and address
 *
 * @param[in]	fdt_addr	Address of FDT
 * @param	phandle		phandle of GPIO device
 * @param[out]	bus		TWSI bus number with node in bits 1-3, can be
 *				NULL for none.
 * @param[out]	addr		TWSI address number, can be NULL for none
 *
 * Return:	0 for success, error otherwise
 */
int cvmx_fdt_get_twsi_gpio_bus_addr(const void *fdt_addr, int phandle, int *bus, int *addr);

/**
 * Given a FDT node return the CPU node number
 *
 * @param[in]	fdt_addr	Address of FDT
 * @param	node		FDT node number
 *
 * Return:	CPU node number or error if negative
 */
int cvmx_fdt_get_cpu_node(const void *fdt_addr, int node);

/**
 * Get the total size of the flat device tree
 *
 * @param[in]	fdt_addr	Address of FDT
 *
 * Return:	Size of flat device tree in bytes or -1 if error.
 */
int cvmx_fdt_get_fdt_size(const void *fdt_addr);

/**
 * Returns if a node is compatible with one of the items in the string list
 *
 * @param[in]	fdt_addr	Pointer to flat device tree
 * @param	node		Node offset to check
 * @param[in]	strlist		Array of FDT device compatibility strings,
 *				must end with NULL or empty string.
 *
 * Return:	0 if at least one item matches, 1 if no matches
 */
int cvmx_fdt_node_check_compatible_list(const void *fdt_addr, int node, const char *const *strlist);

/**
 * Given a FDT node, return the next compatible node.
 *
 * @param[in]	fdt_addr	Pointer to flat device tree
 * @param	start_offset	Starting node offset or -1 to find the first
 * @param	strlist		Array of FDT device compatibility strings, must
 *				end with NULL or empty string.
 *
 * Return:	next matching node or -1 if no more matches.
 */
int cvmx_fdt_node_offset_by_compatible_list(const void *fdt_addr, int startoffset,
					    const char *const *strlist);

/**
 * Given the parent offset of an i2c device build up a list describing the bus
 * which can contain i2c muxes and switches.
 *
 * @param[in]	node		ofnode of the parent node of a GPIO device in
 *				the device tree.
 *
 * Return:	pointer to list of i2c devices starting from the root which
 *		can include i2c muxes and switches or NULL if error.  Note that
 *		all entries are allocated on the heap.
 *
 * @see cvmx_fdt_free_i2c_bus()
 */
struct cvmx_fdt_i2c_bus_info *cvmx_ofnode_get_i2c_bus(ofnode node);

/**
 * Return the Octeon bus number for a bus descriptor
 *
 * @param[in]	bus	bus descriptor
 *
 * Return:	Octeon twsi bus number or -1 on error
 */
int cvmx_fdt_i2c_get_root_bus(const struct cvmx_fdt_i2c_bus_info *bus);

/**
 * Frees all entries for an i2c bus descriptor
 *
 * @param	bus	bus to free
 *
 * Return:	0
 */
int cvmx_fdt_free_i2c_bus(struct cvmx_fdt_i2c_bus_info *bus);

/**
 * Given the bus to a device, enable it.
 *
 * @param[in]	bus	i2c bus descriptor to enable or disable
 * @param	enable	set to true to enable, false to disable
 *
 * Return:	0 for success or -1 for invalid bus
 *
 * This enables the entire bus including muxes and switches in the path.
 */
int cvmx_fdt_enable_i2c_bus(const struct cvmx_fdt_i2c_bus_info *bus, bool enable);

/**
 * Return a GPIO handle given a GPIO phandle of the form <&gpio pin flags>
 *
 * @param[in]	fdt_addr	Address of flat device tree
 * @param	of_offset	node offset for property
 * @param	prop_name	name of property
 *
 * Return:	pointer to GPIO handle or NULL if error
 */
struct cvmx_fdt_gpio_info *cvmx_fdt_gpio_get_info_phandle(const void *fdt_addr, int of_offset,
							  const char *prop_name);

/**
 * Sets a GPIO pin given the GPIO descriptor
 *
 * @param	pin	GPIO pin descriptor
 * @param	value	value to set it to, 0 or 1
 *
 * Return:	0 on success, -1 on error.
 *
 * NOTE: If the CVMX_GPIO_ACTIVE_LOW flag is set then the output value will be
 * inverted.
 */
int cvmx_fdt_gpio_set(struct cvmx_fdt_gpio_info *pin, int value);

/**
 * Given a GPIO pin descriptor, input the value of that pin
 *
 * @param	pin	GPIO pin descriptor
 *
 * Return:	0 if low, 1 if high, -1 on error.  Note that the input will be
 *		inverted if the CVMX_GPIO_ACTIVE_LOW flag bit is set.
 */
int cvmx_fdt_gpio_get(struct cvmx_fdt_gpio_info *pin);

/**
 * Assigns an IPD port to a SFP slot
 *
 * @param	sfp		Handle to SFP data structure
 * @param	ipd_port	Port to assign it to
 *
 * Return:	0 for success, -1 on error
 */
int cvmx_sfp_set_ipd_port(struct cvmx_fdt_sfp_info *sfp, int ipd_port);

/**
 * Get the IPD port of a SFP slot
 *
 * @param[in]	sfp		Handle to SFP data structure
 *
 * Return:	IPD port number for SFP slot
 */
static inline int cvmx_sfp_get_ipd_port(const struct cvmx_fdt_sfp_info *sfp)
{
	return sfp->ipd_port[0];
}

/**
 * Get the IPD ports for a QSFP port
 *
 * @param[in]	sfp		Handle to SFP data structure
 * @param[out]	ipd_ports	IPD ports for each lane, if running as 40G then
 *				only ipd_ports[0] is valid and the others will
 *				be set to -1.
 */
static inline void cvmx_qsfp_get_ipd_ports(const struct cvmx_fdt_sfp_info *sfp, int ipd_ports[4])
{
	int i;

	for (i = 0; i < 4; i++)
		ipd_ports[i] = sfp->ipd_port[i];
}

/**
 * Attaches a PHY to a SFP or QSFP.
 *
 * @param	sfp		sfp to attach PHY to
 * @param	phy_info	phy descriptor to attach or NULL to detach
 */
void cvmx_sfp_attach_phy(struct cvmx_fdt_sfp_info *sfp, struct cvmx_phy_info *phy_info);

/**
 * Returns a phy descriptor for a SFP slot
 *
 * @param[in]	sfp	SFP to get phy info from
 *
 * Return:	phy descriptor or NULL if none.
 */
static inline struct cvmx_phy_info *cvmx_sfp_get_phy_info(const struct cvmx_fdt_sfp_info *sfp)
{
	return sfp->phy_info;
}

/**
 * @INTERNAL
 * Parses all instances of the Vitesse VSC7224 reclocking chip
 *
 * @param[in]	fdt_addr	Address of flat device tree
 *
 * Return:	0 for success, error otherwise
 */
int __cvmx_fdt_parse_vsc7224(const void *fdt_addr);

/**
 * @INTERNAL
 * Parses all instances of the Avago AVSP5410 gearbox phy
 *
 * @param[in]   fdt_addr        Address of flat device tree
 *
 * Return:      0 for success, error otherwise
 */
int __cvmx_fdt_parse_avsp5410(const void *fdt_addr);

/**
 * @INTERNAL
 * Parses either a CS4343 phy or a slice of the phy from the device tree
 * @param[in]	fdt_addr	Address of FDT
 * @param	of_offset	offset of slice or phy in device tree
 * @param	phy_info	phy_info data structure to fill in
 *
 * Return:	0 for success, -1 on error
 */
int cvmx_fdt_parse_cs4343(const void *fdt_addr, int of_offset, struct cvmx_phy_info *phy_info);

/**
 * Given an i2c bus and device address, write an 8 bit value
 *
 * @param bus	i2c bus number
 * @param addr	i2c device address (7 bits)
 * @param val	8-bit value to write
 *
 * This is just an abstraction to ease support in both U-Boot and SE.
 */
void cvmx_fdt_i2c_reg_write(int bus, int addr, u8 val);

/**
 * Read an 8-bit value from an i2c bus and device address
 *
 * @param bus	i2c bus number
 * @param addr	i2c device address (7 bits)
 *
 * Return: 8-bit value or error if negative
 */
int cvmx_fdt_i2c_reg_read(int bus, int addr);

/**
 * Write an 8-bit value to a register indexed i2c device
 *
 * @param bus	i2c bus number to write to
 * @param addr	i2c device address (7 bits)
 * @param reg	i2c 8-bit register address
 * @param val	8-bit value to write
 *
 * Return: 0 for success, otherwise error
 */
int cvmx_fdt_i2c_write8(int bus, int addr, int reg, u8 val);

/**
 * Read an 8-bit value from a register indexed i2c device
 *
 * @param bus	i2c bus number to write to
 * @param addr	i2c device address (7 bits)
 * @param reg	i2c 8-bit register address
 *
 * Return: value or error if negative
 */
int cvmx_fdt_i2c_read8(int bus, int addr, int reg);

int cvmx_sfp_vsc7224_mod_abs_changed(struct cvmx_fdt_sfp_info *sfp_info,
				     int val, void *data);
int cvmx_sfp_avsp5410_mod_abs_changed(struct cvmx_fdt_sfp_info *sfp_info,
				      int val, void *data);

#endif /* CVMX_HELPER_FDT_H__ */

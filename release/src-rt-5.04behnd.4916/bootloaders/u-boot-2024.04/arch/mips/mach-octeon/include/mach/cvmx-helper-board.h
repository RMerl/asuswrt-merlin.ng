/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Helper functions to abstract board specific data about
 * network ports from the rest of the cvmx-helper files.
 */

#ifndef __CVMX_HELPER_BOARD_H__
#define __CVMX_HELPER_BOARD_H__

#include <asm-generic/gpio.h>

#define CVMX_VSC7224_NAME_LEN 16

typedef enum {
	USB_CLOCK_TYPE_REF_12,
	USB_CLOCK_TYPE_REF_24,
	USB_CLOCK_TYPE_REF_48,
	USB_CLOCK_TYPE_CRYSTAL_12,
} cvmx_helper_board_usb_clock_types_t;

typedef enum cvmx_phy_type {
	BROADCOM_GENERIC_PHY,
	MARVELL_GENERIC_PHY,
	CORTINA_PHY, /** Now Inphi */
	AQUANTIA_PHY,
	GENERIC_8023_C22_PHY,
	GENERIC_8023_C45_PHY,
	INBAND_PHY,
	QUALCOMM_S17,	     /** Qualcomm QCA833X switch */
	VITESSE_VSC8490_PHY, /** Vitesse VSC8490 is non-standard for SGMII */
	FAKE_PHY,	     /** Unsupported or no PHY, use GPIOs for LEDs */
} cvmx_phy_type_t;

/** Used to record the host mode used by the Cortina CS4321 PHY */
typedef enum {
	CVMX_PHY_HOST_MODE_UNKNOWN,
	CVMX_PHY_HOST_MODE_SGMII,
	CVMX_PHY_HOST_MODE_QSGMII,
	CVMX_PHY_HOST_MODE_XAUI,
	CVMX_PHY_HOST_MODE_RXAUI,
} cvmx_phy_host_mode_t;

typedef enum {
	set_phy_link_flags_autoneg = 0x1,
	set_phy_link_flags_flow_control_dont_touch = 0x0 << 1,
	set_phy_link_flags_flow_control_enable = 0x1 << 1,
	set_phy_link_flags_flow_control_disable = 0x2 << 1,
	set_phy_link_flags_flow_control_mask = 0x3 << 1,
} cvmx_helper_board_set_phy_link_flags_types_t;

/**
 * The EBB6600 board uses a MDIO mux device to select between the two QLM
 * modules since both QLM modules share the same PHY addresses.  The
 * MDIO mux is controlled via GPIO by a GPIO device that is also on
 * the TWSI bus rather than native OCTEON GPIO libes.
 *
 * To further complicate matters, the TWSI GPIO device sits behind
 * a TWSI mux device as well, making accessing the MDIO devices on
 * this board a very complex operation involving writing to the TWSI mux,
 * followed by the MDIO mux device.
 */
/** Maximum number of GPIO devices used to control the MDIO mux */
#define CVMX_PHY_MUX_MAX_GPIO 2

/** Type of MDIO mux device, currently OTHER isn't supported */
typedef enum {
	SN74CBTLV3253, /** SN74CBTLV3253 I2C device */
	OTHER	       /** Unknown/other */
} cvmx_phy_mux_type_t;

/** Type of GPIO line controlling MDIO mux */
typedef enum {
	GPIO_OCTEON, /** Native OCTEON */
	GPIO_PCA8574 /** TWSI mux device */
} cvmx_phy_gpio_type_t;

/* Forward declarations */
struct cvmx_fdt_sfp_info; /** Defined in cvmx-helper-fdt.h */
struct cvmx_vsc7224;
struct cvmx_fdt_gpio_info;    /** Defined in cvmx-helper-fdt.h */
struct cvmx_fdt_i2c_bus_info; /** Defined in cvmx-helper-fdt.h */
struct cvmx_phy_info;
struct cvmx_fdt_i2c_bus_info;
struct cvmx_fdt_gpio_info;
struct cvmx_fdt_gpio_led;

/**
 * @INTERNAL
 * This data structure is used when the port LEDs are wired up to Octeon's GPIO
 * lines instead of to a traditional PHY.
 */
struct cvmx_phy_gpio_leds {
	struct cvmx_phy_gpio_leds *next; /** For when ports are grouped together */
	u64 last_rx_count;		 /** Counters used to check for activity */
	u64 last_tx_count;		 /** Counters used to check for activity */
	u64 last_activity_poll_time;	 /** Last time activity was polled */
	u64 last_link_poll_time;	 /** Last time link was polled */
	int of_offset;
	int link_poll_interval_ms;     /** Link polling interval in ms */
	int activity_poll_interval_ms; /** Activity polling interval in ms */
	struct cvmx_fdt_gpio_led *link_status;
	struct cvmx_fdt_gpio_led *error;
	struct cvmx_fdt_gpio_led *rx_activity;
	struct cvmx_fdt_gpio_led *tx_activity;
	struct cvmx_fdt_gpio_led *identify;

	struct cvmx_fdt_gpio_info *link_status_gpio;
	struct cvmx_fdt_gpio_info *error_gpio;
	/** Type of GPIO for error LED */
	/** If GPIO expander, describe the bus to the expander */
	struct cvmx_fdt_gpio_info *rx_activity_gpio;
	struct cvmx_fdt_gpio_info *tx_activity_gpio;

	u16 rx_activity_hz; /** RX activity blink time in hz */
	u16 tx_activity_hz; /** TX activity blink time in hz */
	/**
	 * Set if activity and/or link is using an Inphi/Cortina CS4343 or
	 * compatible phy that requires software assistance.  NULL if not used.
	 */
	bool link_status_active_low;  /** True if active link is active low */
	bool error_status_active_low; /** True if error LED is active low */
	bool error_active_low;	      /** True if error is active low */
	bool rx_activity_active_low;  /** True if rx activity is active low */
	bool tx_activity_active_low;  /** True if tx activity is active low */
	/** Set true if LEDs are shared on an interface by all ports */
	bool interface_leds;
	int8_t rx_gpio_timer; /** GPIO clock generator timer [0-3] */
	int8_t tx_gpio_timer; /** GPIO clock generator timer [0-3] */

	/** True if LOS signal activates error LED */
	bool los_generate_error;
	/** True if the error LED is hooked up to a GPIO expander */
	bool error_gpio_expander;
	/** True if the link and RX activity LEDs are shared */
	bool link_and_rx_activity_shared;
	/** True if the link and TX activity LEDs are shared */
	bool link_and_tx_activity_shared;
	/** True if the RX activity and TX activity LEDs are shared */
	bool rx_and_tx_activity_shared;
	/** True if link is driven directly by the hardware */
	bool link_led_hw_link;
	bool error_lit;	    /** True if ERROR LED is lit */
	bool quad_sfp_mode; /** True if using four SFPs for XLAUI */
	/** User-defined function to update the link LED */
	void (*update_link_led)(int xiface, int index, cvmx_helper_link_info_t result);
	/** User-defined function to update the rx activity LED */
	void (*update_rx_activity_led)(struct cvmx_phy_gpio_leds *led, int xiface, int index,
				       bool check_time);
};

/** This structure contains the tap values to use for various cable lengths */
struct cvmx_vsc7224_tap {
	u16 len;      /** Starting cable length for tap values */
	u16 main_tap; /** Main tap value to use */
	u16 pre_tap;  /** Pre-tap value to use */
	u16 post_tap; /** Post-tap value to use */
};

/** Data structure for Microsemi VSC7224 channel */
struct cvmx_vsc7224_chan {
	struct cvmx_vsc7224_chan *next, *prev; /** Used for linking */
	int ipd_port;			       /** IPD port this channel belongs to */
	int xiface;			       /** xinterface of SFP */
	int index;			       /** Port index of SFP */
	int lane;			       /** Lane on port */
	int of_offset;			       /** Offset of channel info in dt */
	bool is_tx;			       /** True if is transmit channel */
	bool maintap_disable;		       /** True to disable the main tap */
	bool pretap_disable;		       /** True to disable pre-tap */
	bool posttap_disable;		       /** True to disable post-tap */
	int num_taps;			       /** Number of tap values */
	/** (Q)SFP attached to this channel */
	struct cvmx_fdt_sfp_info *sfp_info;
	struct cvmx_vsc7224 *vsc7224; /** Pointer to parent */
	/** Tap values for various lengths, must be at the end */
	struct cvmx_vsc7224_tap taps[0];
};

/** Data structure for Microsemi VSC7224 reclocking chip */
struct cvmx_vsc7224 {
	const char *name; /** Name */
	/** Pointer to cannel data */
	struct cvmx_vsc7224_chan *channel[4];
	/** I2C bus device is connected to */
	struct cvmx_fdt_i2c_bus_info *i2c_bus;
	/** Address of VSC7224 on i2c bus */
	int i2c_addr;
	struct gpio_desc los_gpio;		/** LoS GPIO pin */
	struct gpio_desc reset_gpio;		/** Reset GPIO pin */
	int of_offset;			       /** Offset in device tree */
};

/** Data structure for Avago AVSP5410 gearbox phy */
struct cvmx_avsp5410 {
	const char *name; /** Name */
	/** I2C bus device is connected to */
	struct cvmx_fdt_i2c_bus_info *i2c_bus;
	/** Address of AVSP5410 on i2c bus */
	int i2c_addr;
	int of_offset;	    /** Offset in device tree */
	int ipd_port;	    /** IPD port this phy belongs to */
	int xiface;	    /** xinterface of SFP */
	int index;	    /** Port index of SFP */
	u64 prev_temp;	    /** Previous temparature recorded on Phy Core */
	u64 prev_temp_mins; /** Mininutes when the prev temp check is done */
	/** (Q)SFP attached to this phy */
	struct cvmx_fdt_sfp_info *sfp_info;
};

struct cvmx_cs4343_info;

/**
 * @INTERNAL
 *
 * Data structure containing Inphi CS4343 slice information
 */
struct cvmx_cs4343_slice_info {
	const char *name;	       /** Name of this slice in device tree */
	struct cvmx_cs4343_info *mphy; /** Pointer to mphy cs4343 */
	struct cvmx_phy_info *phy_info;
	int of_offset;		      /** Offset in device tree */
	int slice_no;		      /** Slice number */
	int reg_offset;		      /** Offset for this slice */
	u16 sr_stx_cmode_res;	      /** See Rainier device tree */
	u16 sr_stx_drv_lower_cm;      /** See Rainier device tree */
	u16 sr_stx_level;	      /** See Rainier device tree */
	u16 sr_stx_pre_peak;	      /** See Rainier device tree */
	u16 sr_stx_muxsubrate_sel;    /** See Rainier device tree */
	u16 sr_stx_post_peak;	      /** See Rainier device tree */
	u16 cx_stx_cmode_res;	      /** See Rainier device tree */
	u16 cx_stx_drv_lower_cm;      /** See Rainier device tree */
	u16 cx_stx_level;	      /** See Rainier device tree */
	u16 cx_stx_pre_peak;	      /** See Rainier device tree */
	u16 cx_stx_muxsubrate_sel;    /** See Rainier device tree */
	u16 cx_stx_post_peak;	      /** See Rainier device tree */
	u16 basex_stx_cmode_res;      /** See Rainier device tree */
	u16 basex_stx_drv_lower_cm;   /** See Rainier device tree */
	u16 basex_stx_level;	      /** See Rainier device tree */
	u16 basex_stx_pre_peak;	      /** See Rainier device tree */
	u16 basex_stx_muxsubrate_sel; /** See Rainier device tree */
	u16 basex_stx_post_peak;      /** See Rainier device tree */
	int link_gpio;		      /** Link LED gpio pin number, -1 if none */
	int error_gpio;		      /** Error LED GPIO pin or -1 if none */
	int los_gpio;		      /** LoS input GPIO or -1 if none */
	bool los_inverted;	      /** True if LoS input is inverted */
	bool link_inverted;	      /** True if link output is inverted */
	bool error_inverted;	      /** True if error output is inverted */
};

/**
 * @INTERNAL
 *
 * Data structure for Cortina/Inphi CS4343 device
 */
struct cvmx_cs4343_info {
	const char *name; /** Name of Inphi/Cortina CS4343 in DT */
	struct cvmx_phy_info *phy_info;
	struct cvmx_cs4343_slice_info slice[4]; /** Slice information */
	int of_offset;
};

/**
 * @INTERNAL
 * This data structure is used to hold PHY information and is subject to change.
 * Please do  not use this data structure directly.
 *
 * NOTE: The U-Boot OCTEON Ethernet drivers depends on this data structure for
 * the mux support.
 */
typedef struct cvmx_phy_info {
	int phy_addr;	  /** MDIO address of PHY */
	int phy_sub_addr; /** Sub-address (i.e. slice), used by Cortina */
	int ipd_port;	  /** IPD port number for the PHY */
	/** MDIO bus PHY connected to (even if behind mux) */
	int mdio_unit;
	int direct_connect;		 /** 1 if PHY is directly connected */
	int gpio[CVMX_PHY_MUX_MAX_GPIO]; /** GPIOs used to control mux, -1 if not used */

	/** Type of GPIO.  It can be a local OCTEON GPIO or a TWSI GPIO */
	cvmx_phy_gpio_type_t gpio_type[CVMX_PHY_MUX_MAX_GPIO];

	/** Address of TWSI GPIO */
	int cvmx_gpio_twsi[CVMX_PHY_MUX_MAX_GPIO];

	/** Value to put into the GPIO lines to select MDIO bus */
	int gpio_value;
	int gpio_parent_mux_twsi;	/** -1 if not used, parent TWSI mux for ebb6600 */
	int gpio_parent_mux_select;	/** selector to use on parent TWSI mux */
	cvmx_phy_type_t phy_type;	/** Type of PHY */
	cvmx_phy_mux_type_t mux_type;	/** Type of MDIO mux */
	int mux_twsi_addr;		/** Address of the MDIO mux */
	cvmx_phy_host_mode_t host_mode; /** Used by Cortina PHY */
	void *phydev;			/** Pointer to parent phy device */
	int fdt_offset;			/** Node in flat device tree */
	int phy_i2c_bus;		/** I2C bus for reclocking chips */
	int phy_i2c_addr;		/** I2C address of reclocking chip */
	int num_vsc7224;		/** Number of Microsemi VSC7224 devices */
	struct cvmx_vsc7224 *vsc7224;	/** Info for VSC7224 devices */
	/** SFP/QSFP descriptor */
	struct cvmx_fdt_sfp_info *sfp_info;
	/** CS4343 slice information for SGMII/XFI.  This is NULL in XLAUI mode */
	struct cvmx_cs4343_slice_info *cs4343_slice_info;
	/** CS4343 mphy information for XLAUI */
	struct cvmx_cs4343_info *cs4343_info;
	/** Pointer to function to return link information */
	cvmx_helper_link_info_t (*link_function)(struct cvmx_phy_info *phy_info);
	/**
	 * If there are LEDs driven by GPIO lines instead of by a PHY device
	 * then they are described here, otherwise gpio_leds should be NULL.
	 */
	struct cvmx_phy_gpio_leds *gpio_leds;
} cvmx_phy_info_t;

/* Fake IPD port, the RGMII/MII interface may use different PHY, use this
   macro to return appropriate MIX address to read the PHY. */
#define CVMX_HELPER_BOARD_MGMT_IPD_PORT -10

/**
 * Return the MII PHY address associated with the given IPD
 * port. A result of -1 means there isn't a MII capable PHY
 * connected to this port. On chips supporting multiple MII
 * busses the bus number is encoded in bits <15:8>.
 *
 * This function must be modified for every new Octeon board.
 * Internally it uses switch statements based on the cvmx_sysinfo
 * data to determine board types and revisions. It relies on the
 * fact that every Octeon board receives a unique board type
 * enumeration from the bootloader.
 *
 * @param ipd_port Octeon IPD port to get the MII address for.
 *
 * Return: MII PHY address and bus number or -1.
 */
int cvmx_helper_board_get_mii_address(int ipd_port);

/**
 * This function as a board specific method of changing the PHY
 * speed, duplex, and autonegotiation. This programs the PHY and
 * not Octeon. This can be used to force Octeon's links to
 * specific settings.
 *
 * @param phy_addr  The address of the PHY to program
 * @param link_flags
 *                  Flags to control autonegotiation.  Bit 0 is autonegotiation
 *                  enable/disable to maintain backward compatibility.
 * @param link_info Link speed to program. If the speed is zero and autonegotiation
 *                  is enabled, all possible negotiation speeds are advertised.
 *
 * Return: Zero on success, negative on failure
 */
int cvmx_helper_board_link_set_phy(int phy_addr,
				   cvmx_helper_board_set_phy_link_flags_types_t link_flags,
				   cvmx_helper_link_info_t link_info);

/**
 * @INTERNAL
 * This function is the board specific method of determining an
 * ethernet ports link speed. Most Octeon boards have Marvell PHYs
 * and are handled by the fall through case. This function must be
 * updated for boards that don't have the normal Marvell PHYs.
 *
 * This function must be modified for every new Octeon board.
 * Internally it uses switch statements based on the cvmx_sysinfo
 * data to determine board types and revisions. It relies on the
 * fact that every Octeon board receives a unique board type
 * enumeration from the bootloader.
 *
 * @param ipd_port IPD input port associated with the port we want to get link
 *                 status for.
 *
 * Return: The ports link status. If the link isn't fully resolved, this must
 *         return zero.
 */
cvmx_helper_link_info_t __cvmx_helper_board_link_get(int ipd_port);

/**
 * @INTERNAL
 * This function is called by cvmx_helper_interface_probe() after it
 * determines the number of ports Octeon can support on a specific
 * interface. This function is the per board location to override
 * this value. It is called with the number of ports Octeon might
 * support and should return the number of actual ports on the
 * board.
 *
 * This function must be modified for every new Octeon board.
 * Internally it uses switch statements based on the cvmx_sysinfo
 * data to determine board types and revisions. It relies on the
 * fact that every Octeon board receives a unique board type
 * enumeration from the bootloader.
 *
 * @param interface Interface to probe
 * @param supported_ports
 *                  Number of ports Octeon supports.
 *
 * Return: Number of ports the actual board supports. Many times this will
 *         simple be "support_ports".
 */
int __cvmx_helper_board_interface_probe(int interface, int supported_ports);

/**
 * @INTERNAL
 * Enable packet input/output from the hardware. This function is
 * called after by cvmx_helper_packet_hardware_enable() to
 * perform board specific initialization. For most boards
 * nothing is needed.
 *
 * @param interface Interface to enable
 *
 * Return: Zero on success, negative on failure
 */
int __cvmx_helper_board_hardware_enable(int interface);

/**
 * @INTERNAL
 * Gets the clock type used for the USB block based on board type.
 * Used by the USB code for auto configuration of clock type.
 *
 * Return: USB clock type enumeration
 */
cvmx_helper_board_usb_clock_types_t __cvmx_helper_board_usb_get_clock_type(void);

/**
 * @INTERNAL
 * Adjusts the number of available USB ports on Octeon based on board
 * specifics.
 *
 * @param supported_ports expected number of ports based on chip type;
 *
 *
 * Return: number of available usb ports, based on board specifics.
 *         Return value is supported_ports if function does not
 *         override.
 */
int __cvmx_helper_board_usb_get_num_ports(int supported_ports);

/**
 * @INTERNAL
 * Returns if a port is present on an interface
 *
 * @param fdt_addr - address fo flat device tree
 * @param ipd_port - IPD port number
 *
 * Return: 1 if port is present, 0 if not present, -1 if error
 */
int __cvmx_helper_board_get_port_from_dt(void *fdt_addr, int ipd_port);

/**
 * Return the host mode for the PHY.  Currently only the Cortina CS4321 PHY
 * needs this.
 *
 * @param ipd_port - ipd port number to get the host mode for
 *
 * Return: host mode for phy
 */
cvmx_phy_host_mode_t cvmx_helper_board_get_phy_host_mode(int ipd_port);

/**
 * @INTERNAL
 * This function outputs the cvmx_phy_info_t data structure for the specified
 * port.
 *
 * @param[out] - phy_info - phy info data structure
 * @param ipd_port - port to get phy info for
 *
 * Return: 0 for success, -1 if info not available
 *
 * NOTE: The phy_info data structure is subject to change.
 */
int cvmx_helper_board_get_phy_info(cvmx_phy_info_t *phy_info, int ipd_port);

/**
 * @INTERNAL
 * Parse the device tree and set whether a port is valid or not.
 *
 * @param fdt_addr	Pointer to device tree
 *
 * Return: 0 for success, -1 on error.
 */
int __cvmx_helper_parse_bgx_dt(const void *fdt_addr);

/**
 * @INTERNAL
 * Parse the device tree and set whether a port is valid or not.
 *
 * @param fdt_addr	Pointer to device tree
 *
 * Return: 0 for success, -1 on error.
 */
int __cvmx_helper_parse_bgx_rgmii_dt(const void *fdt_addr);

/**
 * @INTERNAL
 * Updates any GPIO link LEDs if present
 *
 * @param xiface	Interface number
 * @param index		Port index
 * @param result	Link status result
 */
void cvmx_helper_update_link_led(int xiface, int index, cvmx_helper_link_info_t result);
/**
 * Update the RX activity LED for the specified interface and port index
 *
 * @param xiface	Interface number
 * @param index		Port index
 * @parma check_time	True if we should bail out before the polling interval
 */
void cvmx_update_rx_activity_led(int xiface, int index, bool check_time);

/**
 * @INTERNAL
 * Figure out which mod_abs changed function to use based on the phy type
 *
 * @param	xiface	xinterface number
 * @param	index	port index on interface
 *
 * Return:	0 for success, -1 on error
 *
 * This function figures out the proper mod_abs_changed function to use and
 * registers the appropriate function.  This should be called after the device
 * tree has been fully parsed for the given port as well as after all SFP
 * slots and any Microsemi VSC7224 devices have been parsed in the device tree.
 */
int cvmx_helper_phy_register_mod_abs_changed(int xiface, int index);

/**
 * @INTERNAL
 * Return loss of signal
 *
 * @param	xiface	xinterface number
 * @param	index	port index on interface
 *
 * Return:	0 if signal present, 1 if loss of signal.
 *
 * @NOTE:	A result of 0 is possible in some cases where the signal is
 *		not present.
 *
 * This is for use with __cvmx_qlm_rx_equilization
 */
int __cvmx_helper_get_los(int xiface, int index);

/**
 * Given the address of the MDIO registers, output the CPU node and MDIO bus
 *
 * @param	addr	64-bit address of MDIO registers (from device tree)
 * @param[out]	node	CPU node number (78xx)
 * @param[out]	bus	MDIO bus number
 */
void __cvmx_mdio_addr_to_node_bus(u64 addr, int *node, int *bus);

/**
 * Turn on the error LED
 *
 * @param	leds	LEDs belonging to port
 * @param	error	true to turn on LED, false to turn off
 */
void cvmx_helper_leds_show_error(struct cvmx_phy_gpio_leds *leds, bool error);

#endif /* __CVMX_HELPER_BOARD_H__ */

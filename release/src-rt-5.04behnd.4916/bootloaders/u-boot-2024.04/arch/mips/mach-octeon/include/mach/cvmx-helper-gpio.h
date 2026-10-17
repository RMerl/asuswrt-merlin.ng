/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Marvell International Ltd.
 *
 * Defines some GPIO information used in multiple places
 */

#ifndef __CVMX_HELPER_GPIO_H__
#define __CVMX_HELPER_GPIO_H__

#define CVMX_GPIO_NAME_LEN 32 /** Length of name */

enum cvmx_gpio_type {
	CVMX_GPIO_PIN_OCTEON,  /** GPIO pin is directly connected to OCTEON */
	CVMX_GPIO_PIN_PCA953X, /** GPIO pin is NXP PCA953X compat chip */
	CVMX_GPIO_PIN_PCA957X,
	CVMX_GPIO_PIN_PCF857X, /** GPIO pin is NXP PCF857X compat chip */
	CVMX_GPIO_PIN_PCA9698, /** GPIO pin is NXP PCA9698 compat chip */
	CVMX_GPIO_PIN_CS4343,  /** Inphi/Cortina CS4343 GPIO pins */
	CVMX_GPIO_PIN_OTHER,   /** GPIO pin is something else */
};

enum cvmx_gpio_operation {
	CVMX_GPIO_OP_CONFIG,	  /** Initial configuration of the GPIO pin */
	CVMX_GPIO_OP_SET,	  /** Set pin */
	CVMX_GPIO_OP_CLEAR,	  /** Clear pin */
	CVMX_GPIO_OP_READ,	  /** Read pin */
	CVMX_GPIO_OP_TOGGLE,	  /** Toggle pin */
	CVMX_GPIO_OP_BLINK_START, /** Put in blink mode (if supported) */
	CVMX_GPIO_OP_BLINK_STOP,  /** Takes the pin out of blink mode */
	CVMX_GPIO_OP_SET_LINK,	  /** Put in link monitoring mode */
	CVMX_GPIO_OP_SET_ACT,	  /** Put in RX activity mode */
};

/**
 * Inphi CS4343 output source select values for the GPIO_GPIOX output_src_sel.
 */
enum cvmx_inphi_cs4343_gpio_gpio_output_src_sel {
	GPIO_SEL_DRIVE = 0,	/** Value of GPIOX_DRIVE */
	GPIO_SEL_DELAY = 1,	/** Drive delayed */
	GPIO_SEL_TOGGLE = 2,	/** Used for blinking */
	GPIO_SEL_EXT = 3,	/** External function */
	GPIO_SEL_EXT_DELAY = 4, /** External function delayed */
};

/** Inphi GPIO_GPIOX configuration register */
union cvmx_inphi_cs4343_gpio_cfg_reg {
	u16 u;
	struct {
u16: 4;
		/** Data source for the GPIO output */
		u16 output_src_sel : 3;
		/** 1 = GPIO output is inverted before being output */
		u16 invert_output : 1;
		/** 1 = GPIO input is inverted before being processed */
		u16 invert_input : 1;
		/** 0 = 2.5v/1.8v signalling, 1 = 1.2v signalling */
		u16 iovddsel_1v2 : 1;
		/**
		 * 0 = output selected by outen bit
		 * 1 = output controlled by selected GPIO output source
		 */
		u16 outen_ovr : 1;
		/** 0 = GPIO is input only, 1 = GPIO output driver enabled */
		u16 outen : 1;
u16: 2;
		u16 pullup_1k;	/** 1 = enable 1K pad pullup */
		u16 pullup_10k; /** 1 = enable 10K pad pullup */
	} s;
};

#define CVMX_INPHI_CS4343_GPIO_CFG_OFFSET 0x0

/**
 * This selects which port the GPIO gets its signals from when configured
 * as an output.
 */
enum cvmx_inphi_cs4343_gpio_output_cfg_port {
	PORT_0_HOST_RX = 0, /** Port pair 0 host RX */
	PORT_0_LINE_RX = 1, /** Port pair 0 line RX */
	PORT_1_HOST_RX = 2, /** Port pair 1 host RX */
	PORT_1_LINE_RX = 3, /** Port pair 1 line RX */
	PORT_3_HOST_RX = 4, /** Port pair 3 host RX */
	PORT_3_LINE_RX = 5, /** Port pair 3 line RX */
	PORT_2_HOST_RX = 6, /** Port pair 2 host RX */
	PORT_2_LINE_RX = 7, /** Port pair 2 line RX */
	COMMON = 8,	    /** Common */
};

enum cvmx_inphi_cs4343_gpio_output_cfg_function {
	RX_LOS = 0,	   /** Port - 1 = Receive LOS (from DSP) */
	RX_LOL = 1,	   /** Port - 1 = Receive LOL (inverted from MSEQ) */
	EDC_CONVERGED = 2, /** Port - 1 = EDC converged (from DSP) */
	/** Port - 1 = PRBS checker in sync (inverted from SDS) */
	RX_PRBS_SYNC = 3,
	COMMON_LOGIC_0 = 0,	 /** Common - Logic 0 */
	COMMON_GPIO1_INPUT = 1,	 /** Common - GPIO 1 input */
	COMMON_GPIO2_INPUT = 2,	 /** Common - GPIO 2 input */
	COMMON_GPIO3_INPUT = 3,	 /** Common - GPIO 3 input */
	COMMON_GPIO4_INPUT = 4,	 /** Common - GPIO 4 input */
	COMMON_INTERR_INPUT = 5, /** Common - INTERR input */
	/** Common - Interrupt output from GLOBAL_INT register */
	COMMON_GLOBAL_INT = 6,
	/** Common - Interrupt output from GPIO_INT register */
	COMMON_GPIO_INT = 7,
	/** Common - Temp/voltage monitor interrupt */
	COMMON_MONITOR_INT = 8,
	/** Common - Selected clock output of global clock monitor */
	COMMON_GBL_CLKMON_CLK = 9,
};

union cvmx_inphi_cs4343_gpio_output_cfg {
	u16 u;
	struct {
u16: 8;
		u16 port : 4;	  /** port */
		u16 function : 4; /** function */
	} s;
};

#define CVMX_INPHI_CS4343_GPIO_OUTPUT_CFG_OFFSET 0x1

union cvmx_inphi_cs4343_gpio_drive {
	u16 u;
	struct {
u16: 15;
		u16 value : 1; /** output value */
	} s;
};

#define CVMX_INPHI_CS4343_GPIO_DRIVE_OFFSET 0x2

union cvmx_inphi_cs4343_gpio_value {
	u16 u;
	struct {
u16: 15;
		u16 value : 1; /** input value (read-only) */
	} s;
};

#define CVMX_INPHI_CS4343_GPIO_VALUE_OFFSET 0x3

union cvmx_inphi_cs4343_gpio_toggle {
	u16 u;
	struct {
		/** Toggle rate in ms, multiply by 2 to get period in ms */
		u16 rate : 16;
	} s;
};

#define CVMX_INPHI_CS4343_GPIO_TOGGLE_OFFSET 0x4

union cvmx_inphi_cs4343_gpio_delay {
	u16 u;
	struct {
		/** On delay for GPIO output in ms when enabled */
		u16 on_delay : 16;
	} s;
};

#define CVMX_INPHI_CS4343_GPIO_DELAY_OFFSET 0x5

/**
 * GPIO flags associated with a GPIO pin (can be combined)
 */
enum cvmx_gpio_flags {
	CVMX_GPIO_ACTIVE_HIGH = 0,    /** Active high (default) */
	CVMX_GPIO_ACTIVE_LOW = 1,     /** Active low (inverted) */
	CVMX_GPIO_OPEN_COLLECTOR = 2, /** Output is open-collector */
};

/** Default timer number to use for outputting a frequency [0..3] */
#define CVMX_GPIO_DEFAULT_TIMER 3

/** Configuration data for native Octeon GPIO pins */
struct cvmx_octeon_gpio_data {
	int cpu_node; /** CPU node for GPIO pin */
	int timer;    /** Timer number used when in toggle mode, 0-3 */
};

struct cvmx_pcf857x_gpio_data {
	unsigned int latch_out;
};

#define CVMX_INPHI_CS4343_EFUSE_PDF_SKU_REG 0x19f
#define CVMX_INPHI_CS4343_SKU_CS4223	    0x10
#define CVMX_INPHI_CS4343_SKU_CS4224	    0x11
#define CVMX_INPHI_CS4343_SKU_CS4343	    0x12
#define CVMX_INPHI_CS4343_SKU_CS4221	    0x13
#define CVMX_INPHI_CS4343_SKU_CS4227	    0x14
#define CVMX_INPHI_CS4343_SKU_CS4341	    0x16

struct cvmx_cs4343_gpio_data {
	int reg_offset; /** Base register address for GPIO */
	enum cvmx_gpio_operation last_op;
	u8 link_port; /** Link port number for link status */
	u16 sku;      /** Value from CS4224_EFUSE_PDF_SKU register */
	u8 out_src_sel;
	u8 field_func;
	bool out_en;
	bool is_cs4343; /** True if dual package */
	struct phy_device *phydev;
};

struct cvmx_fdt_gpio_info;

/** Function called for GPIO operations */
typedef int (*cvmx_fdt_gpio_op_func_t)(struct cvmx_fdt_gpio_info *, enum cvmx_gpio_operation);

/**
 * GPIO descriptor
 */
struct cvmx_fdt_gpio_info {
	struct cvmx_fdt_gpio_info *next; /** For list of GPIOs */
	char name[CVMX_GPIO_NAME_LEN];	 /** Name of GPIO */
	int pin;			 /** GPIO pin number */
	enum cvmx_gpio_type gpio_type;	 /** Type of GPIO controller */
	int of_offset;			 /** Offset in device tree */
	int phandle;
	struct cvmx_fdt_i2c_bus_info *i2c_bus; /** I2C bus descriptor */
	int i2c_addr;			       /** Address on i2c bus */
	enum cvmx_gpio_flags flags;	       /** Flags associated with pin */
	int num_pins;			       /** Total number of pins */
	unsigned int latch_out;		       /** Latched output for 857x */
	/** Rate in ms between toggle states */
	int toggle_rate;
	/** Pointer to user data for user-defined functions */
	void *data;
	/** Function to set, clear, toggle, etc. */
	cvmx_fdt_gpio_op_func_t op_func;
	/* Two values are used to detect the initial case where nothing has
	 * been configured.  Initially, all of the following will be false
	 * which will force the initial state to be properly set.
	 */
	/** True if the GPIO pin is currently set, useful for toggle */
	bool is_set;
	/** Set if configured to invert */
	bool invert_set;
	/** Set if input is to be inverted */
	bool invert_input;
	/** Set if direction is configured as output */
	bool dir_out;
	/** Set if direction is configured as input */
	bool dir_in;
	/** Pin is set to toggle periodically */
	bool toggle;
	/** True if LED is used to indicate link status */
	bool link_led;
	/** True if LED is used to indicate rx activity */
	bool rx_act_led;
	/** True if LED is used to indicate tx activity */
	bool tx_act_led;
	/** True if LED is used to indicate networking errors */
	bool error_led;
	/** True if LED can automatically show link */
	bool hw_link;
};

/** LED datastructure */
struct cvmx_fdt_gpio_led {
	struct cvmx_fdt_gpio_led *next, *prev; /** List of LEDs */
	char name[CVMX_GPIO_NAME_LEN];	       /** Name */
	struct cvmx_fdt_gpio_info *gpio;       /** GPIO for LED */
	int of_offset;			       /** Device tree node */
	/** True if active low, note that GPIO contains this info */
	bool active_low;
};

/**
 * Returns the operation function for the GPIO phandle
 *
 * @param[in]	fdt_addr	Pointer to FDT
 * @param	phandle		phandle of GPIO entry
 *
 * Return:	Pointer to op function or NULL if not found.
 */
cvmx_fdt_gpio_op_func_t cvmx_fdt_gpio_get_op_func(const void *fdt_addr, int phandle);

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
 * Return a GPIO handle given a GPIO phandle of the form <&gpio pin flags>
 *
 * @param[in]	fdt_addr	Address of flat device tree
 * @param	of_offset	node offset of GPIO device
 * @param	prop_name	name of property
 *
 * Return:	pointer to GPIO handle or NULL if error
 */
struct cvmx_fdt_gpio_info *cvmx_fdt_gpio_get_info(const void *fdt_addr, int of_offset,
						  const char *prop_name);

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
 * Parses a GPIO entry and fills in the gpio info data structure
 *
 * @param[in]	fdt_addr	Address of FDT
 * @param	phandle		phandle for GPIO
 * @param	pin		pin number
 * @param	flags		flags set (1 = invert)
 * @param[out]	gpio		GPIO info data structure
 *
 * Return:	0 for success, -1 on error
 */
int cvmx_fdt_parse_gpio(const void *fdt_addr, int phandle, int pin, u32 flags,
			struct cvmx_fdt_gpio_info *gpio);

/**
 * @param	gpio	GPIO descriptor to assign timer to
 * @param	timer	Octeon hardware timer number [0..3]
 */
void cvmx_fdt_gpio_set_timer(struct cvmx_fdt_gpio_info *gpio, int timer);

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
 * Sets a GPIO pin given the GPIO descriptor
 *
 * @param	gpio	GPIO pin descriptor
 * @param	value	value to set it to, 0 or 1
 *
 * Return:	0 on success, -1 on error.
 *
 * NOTE: If the CVMX_GPIO_ACTIVE_LOW flag is set then the output value will be
 * inverted.
 */
int cvmx_fdt_gpio_set(struct cvmx_fdt_gpio_info *gpio, int value);

/**
 * Sets the blink frequency for a GPIO pin
 *
 * @param gpio	GPIO handle
 * @param freq	Frequency in hz [0..500]
 */
void cvmx_fdt_gpio_set_freq(struct cvmx_fdt_gpio_info *gpio, int freq);

/**
 * Enables or disables blinking a GPIO pin
 *
 * @param	gpio	GPIO handle
 * @param	blink	True to start blinking, false to stop
 *
 * Return:	0 for success, -1 on error
 * NOTE: Not all GPIO types support blinking.
 */
int cvmx_fdt_gpio_set_blink(struct cvmx_fdt_gpio_info *gpio, bool blink);

/**
 * Alternates between link and blink mode
 *
 * @param	gpio	GPIO handle
 * @param	blink	True to start blinking, false to use link status
 *
 * Return:	0 for success, -1 on error
 * NOTE: Not all GPIO types support this.
 */
int cvmx_fdt_gpio_set_link_blink(struct cvmx_fdt_gpio_info *gpio, bool blink);

static inline bool cvmx_fdt_gpio_hw_link_supported(const struct cvmx_fdt_gpio_info *gpio)
{
	return gpio->hw_link;
}

/**
 * Configures a GPIO pin as input or output
 *
 * @param	gpio	GPIO pin to configure
 * @param	output	Set to true to make output, false for input
 */
void cvmx_fdt_gpio_set_output(struct cvmx_fdt_gpio_info *gpio, bool output);

/**
 * Allocates an LED data structure
 * @param[in]	name		name to assign LED
 * @param	of_offset	Device tree offset
 * @param	gpio		GPIO assigned to LED (can be NULL)
 * @param	last		Previous LED to build a list
 *
 * Return:	pointer to LED data structure or NULL if out of memory
 */
struct cvmx_fdt_gpio_led *cvmx_alloc_led(const char *name, int of_offset,
					 struct cvmx_fdt_gpio_info *gpio,
					 struct cvmx_fdt_gpio_led *last);

/**
 * Parses an LED in the device tree
 *
 * @param[in]	fdt_addr		Pointer to flat device tree
 * @param	led_of_offset		Device tree offset of LED
 * @param	gpio			GPIO data structure to use (can be NULL)
 * @param	last			Previous LED if this is a group of LEDs
 *
 * Return:	Pointer to LED data structure or NULL if error
 */
struct cvmx_fdt_gpio_led *cvmx_fdt_parse_led(const void *fdt_addr, int led_of_offset,
					     struct cvmx_fdt_gpio_info *gpio,
					     struct cvmx_fdt_gpio_led *last);

#endif /* __CVMX_HELPER_GPIO_H__ */

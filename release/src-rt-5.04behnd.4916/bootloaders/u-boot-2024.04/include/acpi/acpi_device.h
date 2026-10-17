/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Generation of tables for particular device types
 *
 * Copyright 2019 Google LLC
 * Mostly taken from coreboot file of the same name
 */

#ifndef __ACPI_DEVICE_H
#define __ACPI_DEVICE_H

#include <i2c.h>
#include <irq.h>
#include <spi.h>
#include <asm-generic/gpio.h>
#include <linux/bitops.h>

struct acpi_ctx;
struct gpio_desc;
struct irq;
struct udevice;

/* ACPI descriptor values for common descriptors: SERIAL_BUS means I2C */
#define ACPI_DESCRIPTOR_LARGE		BIT(7)
#define ACPI_DESCRIPTOR_REGISTER	(ACPI_DESCRIPTOR_LARGE | 2)
#define ACPI_DESCRIPTOR_INTERRUPT	(ACPI_DESCRIPTOR_LARGE | 9)
#define ACPI_DESCRIPTOR_GPIO		(ACPI_DESCRIPTOR_LARGE | 12)
#define ACPI_DESCRIPTOR_SERIAL_BUS	(ACPI_DESCRIPTOR_LARGE | 14)

/* Length of a full path to an ACPI device */
#define ACPI_PATH_MAX		30

/* UUID for an I2C _DSM method */
#define ACPI_DSM_I2C_HID_UUID	"3cdff6f7-4267-4555-ad05-b30a3d8938de"

/* Values that can be returned for ACPI device _STA method */
enum acpi_dev_status {
	ACPI_DSTATUS_PRESENT		= BIT(0),
	ACPI_DSTATUS_ENABLED		= BIT(1),
	ACPI_DSTATUS_SHOW_IN_UI		= BIT(2),
	ACPI_DSTATUS_OK			= BIT(3),
	ACPI_DSTATUS_HAS_BATTERY	= BIT(4),

	ACPI_DSTATUS_ALL_OFF	= 0,
	ACPI_DSTATUS_HIDDEN_ON	= ACPI_DSTATUS_PRESENT | ACPI_DSTATUS_ENABLED |
		ACPI_DSTATUS_OK,
	ACPI_DSTATUS_ALL_ON	= ACPI_DSTATUS_HIDDEN_ON |
		ACPI_DSTATUS_SHOW_IN_UI,
};

/** enum acpi_irq_mode - edge/level trigger mode */
enum acpi_irq_mode {
	ACPI_IRQ_EDGE_TRIGGERED,
	ACPI_IRQ_LEVEL_TRIGGERED,
};

/**
 * enum acpi_irq_polarity - polarity of interrupt
 *
 * @ACPI_IRQ_ACTIVE_LOW - for ACPI_IRQ_EDGE_TRIGGERED this means falling edge
 * @ACPI_IRQ_ACTIVE_HIGH - for ACPI_IRQ_EDGE_TRIGGERED this means rising edge
 * @ACPI_IRQ_ACTIVE_BOTH - not meaningful for ACPI_IRQ_EDGE_TRIGGERED
 */
enum acpi_irq_polarity {
	ACPI_IRQ_ACTIVE_LOW,
	ACPI_IRQ_ACTIVE_HIGH,
	ACPI_IRQ_ACTIVE_BOTH,
};

/**
 * enum acpi_irq_shared - whether interrupt is shared or not
 *
 * @ACPI_IRQ_EXCLUSIVE: only this device uses the interrupt
 * @ACPI_IRQ_SHARED: other devices may use this interrupt
 */
enum acpi_irq_shared {
	ACPI_IRQ_EXCLUSIVE,
	ACPI_IRQ_SHARED,
};

/** enum acpi_irq_wake - indicates whether this interrupt can wake the device */
enum acpi_irq_wake {
	ACPI_IRQ_NO_WAKE,
	ACPI_IRQ_WAKE,
};

/**
 * struct acpi_irq - representation of an ACPI interrupt
 *
 * @pin: ACPI pin that is monitored for the interrupt
 * @mode: Edge/level triggering
 * @polarity: Interrupt polarity
 * @shared: Whether interrupt is shared or not
 * @wake: Whether interrupt can wake the device from sleep
 */
struct acpi_irq {
	unsigned int pin;
	enum acpi_irq_mode mode;
	enum acpi_irq_polarity polarity;
	enum acpi_irq_shared shared;
	enum acpi_irq_wake wake;
};

/**
 * enum acpi_gpio_type - type of the descriptor
 *
 * @ACPI_GPIO_TYPE_INTERRUPT: GpioInterrupt
 * @ACPI_GPIO_TYPE_IO: GpioIo
 */
enum acpi_gpio_type {
	ACPI_GPIO_TYPE_INTERRUPT,
	ACPI_GPIO_TYPE_IO,
};

/**
 * enum acpi_gpio_pull - pull direction
 *
 * @ACPI_GPIO_PULL_DEFAULT: Use default value for pin
 * @ACPI_GPIO_PULL_UP: Pull up
 * @ACPI_GPIO_PULL_DOWN: Pull down
 * @ACPI_GPIO_PULL_NONE: No pullup/pulldown
 */
enum acpi_gpio_pull {
	ACPI_GPIO_PULL_DEFAULT,
	ACPI_GPIO_PULL_UP,
	ACPI_GPIO_PULL_DOWN,
	ACPI_GPIO_PULL_NONE,
};

/**
 * enum acpi_gpio_io_restrict - controls input/output of pin
 *
 * @ACPI_GPIO_IO_RESTRICT_NONE: no restrictions
 * @ACPI_GPIO_IO_RESTRICT_INPUT: input only (no output)
 * @ACPI_GPIO_IO_RESTRICT_OUTPUT: output only (no input)
 * @ACPI_GPIO_IO_RESTRICT_PRESERVE: preserve settings when driver not active
 */
enum acpi_gpio_io_restrict {
	ACPI_GPIO_IO_RESTRICT_NONE,
	ACPI_GPIO_IO_RESTRICT_INPUT,
	ACPI_GPIO_IO_RESTRICT_OUTPUT,
	ACPI_GPIO_IO_RESTRICT_PRESERVE,
};

/** enum acpi_gpio_polarity - controls the GPIO polarity */
enum acpi_gpio_polarity {
	ACPI_GPIO_ACTIVE_HIGH = 0,
	ACPI_GPIO_ACTIVE_LOW = 1,
};

#define ACPI_GPIO_REVISION_ID		1
#define ACPI_GPIO_MAX_PINS		2

/**
 * struct acpi_gpio - representation of an ACPI GPIO
 *
 * @pin_count: Number of pins represented
 * @pins: List of pins
 * @pin0_addr: Address in memory of the control registers for pin 0. This is
 *   used when generating ACPI tables
 * @type: GPIO type
 * @pull: Pullup/pulldown setting
 * @resource: Resource name for this GPIO controller
 * For GpioInt:
 * @interrupt_debounce_timeout: Debounce timeout in units of 10us
 * @irq: Interrupt
 *
 * For GpioIo:
 * @output_drive_strength: Drive strength in units of 10uA
 * @io_shared; true if GPIO is shared
 * @io_restrict: I/O restriction setting
 * @polarity: GPIO polarity
 *
 * Note that GpioIo() doesn't have any means of Active Low / High setting, so a
 * _DSD must be provided to mitigate this. This parameter does not make sense
 * for GpioInt() since it has its own means to define it.
 *
 * GpioIo() doesn't properly communicate the initial state of the output pin,
 * thus Linux assumes the simple rule:
 *
 * Pull Bias       Polarity      Requested...
 *
 * Implicit        x             AS IS (assumed firmware configured for us)
 * Explicit        x (no _DSD)   as Pull Bias (Up == High, Down == Low),
 *                               assuming non-active (Polarity = !Pull Bias)
 *
 * Down            Low           as low, assuming active
 * Down            High          as low, assuming non-active
 * Up              Low           as high, assuming non-active
 * Up              High          as high, assuming active
 *
 * GpioIo() can be used as interrupt and in this case the IoRestriction mustn't
 * be OutputOnly. It also requires active_low flag from _DSD in cases where it's
 * needed (better to always provide than rely on above assumption made on OS
 * level).
 */
struct acpi_gpio {
	int pin_count;
	u16 pins[ACPI_GPIO_MAX_PINS];
	ulong pin0_addr;

	enum acpi_gpio_type type;
	enum acpi_gpio_pull pull;
	char resource[ACPI_PATH_MAX];

	/* GpioInt */
	u16 interrupt_debounce_timeout;
	struct acpi_irq irq;

	/* GpioIo */
	u16 output_drive_strength;
	bool io_shared;
	enum acpi_gpio_io_restrict io_restrict;
	enum acpi_gpio_polarity polarity;
};

/* ACPI Descriptors for Serial Bus interfaces */
#define ACPI_SERIAL_BUS_TYPE_I2C		1
#define ACPI_SERIAL_BUS_TYPE_SPI		2
#define ACPI_I2C_SERIAL_BUS_REVISION_ID		1 /* TODO: upgrade to 2 */
#define ACPI_I2C_TYPE_SPECIFIC_REVISION_ID	1
#define ACPI_SPI_SERIAL_BUS_REVISION_ID		1
#define ACPI_SPI_TYPE_SPECIFIC_REVISION_ID	1

/**
 * struct acpi_i2c - representation of an ACPI I2C device
 *
 * @address: 7-bit or 10-bit I2C address
 * @mode_10bit: Which address size is used
 * @speed: Bus speed in Hz
 * @resource: Resource name for the I2C controller
 */
struct acpi_i2c {
	u16 address;
	enum i2c_address_mode mode_10bit;
	enum i2c_speed_rate speed;
	const char *resource;
};

/**
 * struct acpi_spi - representation of an ACPI SPI device
 *
 * @device_select: Chip select used by this device (typically 0)
 * @device_select_polarity: Polarity for the device
 * @wire_mode: Number of wires used for SPI
 * @speed: Bus speed in Hz
 * @data_bit_length: Word length for SPI (typically 8)
 * @clock_phase: Clock phase to capture data
 * @clock_polarity: Bus polarity
 * @resource: Resource name for the SPI controller
 */
struct acpi_spi {
	u16 device_select;
	enum spi_polarity device_select_polarity;
	enum spi_wire_mode wire_mode;
	unsigned int speed;
	u8 data_bit_length;
	enum spi_clock_phase clock_phase;
	enum spi_polarity clock_polarity;
	const char *resource;
};

/**
 * struct acpi_i2c_priv - Information read from device tree
 *
 * This is used by devices which want to specify various pieces of ACPI
 * information, including power control. It allows a generic function to
 * generate the information for ACPI, based on device-tree properties.
 *
 * @disable_gpio_export_in_crs: Don't export GPIOs in the CRS
 * @reset_gpio: GPIO used to assert reset to the device
 * @enable_gpio: GPIO used to enable the device
 * @stop_gpio: GPIO used to stop the device
 * @irq_gpio: GPIO used for interrupt (if @irq is not used)
 * @irq: IRQ used for interrupt (if @irq_gpio is not used)
 * @hid: _HID value for device (required)
 * @uid: _UID value for device
 * @desc: _DDN value for device
 * @wake: Wake event, e.g. GPE0_DW1_15; 0 if none
 * @property_count: Number of other DSD properties (currently always 0)
 * @probed: true set set 'linux,probed' property
 * @compat_string: Device tree compatible string to report through ACPI
 * @has_power_resource: true if this device has a power resource
 * @reset_delay_ms: Delay after de-asserting reset, in ms
 * @reset_off_delay_ms: Delay after asserting reset (during power off)
 * @enable_delay_ms: Delay after asserting enable
 * @enable_off_delay_ms: Delay after de-asserting enable (during power off)
 * @stop_delay_ms: Delay after de-aserting stop
 * @stop_off_delay_ms: Delay after asserting stop (during power off)
 * @hid_desc_reg_offset: HID register offset (for Human Interface Devices)
 */
struct acpi_i2c_priv {
	bool disable_gpio_export_in_crs;
	struct gpio_desc reset_gpio;
	struct gpio_desc enable_gpio;
	struct gpio_desc irq_gpio;
	struct gpio_desc stop_gpio;
	struct irq irq;
	const char *hid;
	u32 uid;
	const char *desc;
	u32 wake;
	u32 property_count;
	bool probed;
	const char *compat_string;
	bool has_power_resource;
	u32 reset_delay_ms;
	u32 reset_off_delay_ms;
	u32 enable_delay_ms;
	u32 enable_off_delay_ms;
	u32 stop_delay_ms;
	u32 stop_off_delay_ms;
	u32 hid_desc_reg_offset;
};

/**
 * acpi_device_path() - Get the full path to an ACPI device
 *
 * This gets the full path in the form XXXX.YYYY.ZZZZ where XXXX is the root
 * and ZZZZ is the device. All parent devices are added to the path.
 *
 * @dev: Device to check
 * @buf: Buffer to place the path in (should be ACPI_PATH_MAX long)
 * @maxlen: Size of buffer (typically ACPI_PATH_MAX)
 * Return: 0 if OK, -ve on error
 */
int acpi_device_path(const struct udevice *dev, char *buf, int maxlen);

/**
 * acpi_device_scope() - Get the scope of an ACPI device
 *
 * This gets the scope which is the full path of the parent device, as per
 * acpi_device_path().
 *
 * @dev: Device to check
 * @buf: Buffer to place the path in (should be ACPI_PATH_MAX long)
 * @maxlen: Size of buffer (typically ACPI_PATH_MAX)
 * Return: 0 if OK, -EINVAL if the device has no parent, other -ve on other
 *	error
 */
int acpi_device_scope(const struct udevice *dev, char *scope, int maxlen);

/**
 * acpi_device_status() - Get the status of a device
 *
 * This currently just returns ACPI_DSTATUS_ALL_ON. It does not support
 * inactive or hidden devices.
 *
 * @dev: Device to check
 * Return: device status, as ACPI_DSTATUS_...
 */
enum acpi_dev_status acpi_device_status(const struct udevice *dev);

/**
 * acpi_device_write_interrupt_irq() - Write an interrupt descriptor
 *
 * This writes an ACPI interrupt descriptor for the given interrupt, converting
 * fields as needed.
 *
 * @ctx: ACPI context pointer
 * @req_irq: Interrupt to output
 * Return: IRQ pin number if OK, -ve on error
 */
int acpi_device_write_interrupt_irq(struct acpi_ctx *ctx,
				    const struct irq *req_irq);

/**
 * acpi_device_write_gpio() - Write GpioIo() or GpioInt() descriptor
 *
 * @gpio: GPIO information to write
 * Return: GPIO pin number of first GPIO if OK, -ve on error
 */
int acpi_device_write_gpio(struct acpi_ctx *ctx, const struct acpi_gpio *gpio);

/**
 * acpi_device_write_gpio_desc() - Write a GPIO to ACPI
 *
 * This creates a GPIO descriptor for a GPIO, including information ACPI needs
 * to use it.
 *
 * @ctx: ACPI context pointer
 * @desc: GPIO to write
 * Return: 0 if OK, -ve on error
 */
int acpi_device_write_gpio_desc(struct acpi_ctx *ctx,
				const struct gpio_desc *desc);

/**
 * acpi_device_write_interrupt_or_gpio() - Write interrupt or GPIO to ACPI
 *
 * This reads an interrupt from the device tree "interrupts-extended" property,
 * if available. If not it reads the first GPIO with the name @prop.
 *
 * If an interrupt is found, an ACPI interrupt descriptor is written to the ACPI
 * output. If not, but if a GPIO is found, a GPIO descriptor is written.
 *
 * Return: irq or GPIO pin number if OK, -ve if neither an interrupt nor a GPIO
 *	could be found, or some other error occurred
 */
int acpi_device_write_interrupt_or_gpio(struct acpi_ctx *ctx,
					struct udevice *dev, const char *prop);

/**
 * acpi_device_write_dsm_i2c_hid() - Write a device-specific method for HID
 *
 * This writes a DSM for an I2C Human-Interface Device based on the config
 * provided
 *
 * @hid_desc_reg_offset: HID register offset
 */
int acpi_device_write_dsm_i2c_hid(struct acpi_ctx *ctx,
				  int hid_desc_reg_offset);

/**
 * acpi_device_write_i2c_dev() - Write an I2C device to ACPI
 *
 * This creates a I2cSerialBusV2 descriptor for an I2C device, including
 * information ACPI needs to use it.
 *
 * @ctx: ACPI context pointer
 * @dev: I2C device to write
 * Return: I2C address of device if OK, -ve on error
 */
int acpi_device_write_i2c_dev(struct acpi_ctx *ctx, const struct udevice *dev);

/**
 * acpi_device_write_spi_dev() - Write a SPI device to ACPI
 *
 * This writes a serial bus descriptor for the SPI device so that ACPI can use
 * it
 *
 * @ctx: ACPI context pointer
 * @dev: SPI device to write
 * Return: 0 if OK, -ve on error
 */
int acpi_device_write_spi_dev(struct acpi_ctx *ctx, const struct udevice *dev);

/**
 * acpi_device_add_power_res() - Add a basic PowerResource block for a device
 *
 * This includes GPIOs to control enable, reset and stop operation of the
 * device. Each GPIO is optional, but at least one must be provided.
 * This can be applied to any device that has power control, so is fairly
 * generic.
 *
 * Reset - Put the device into / take the device out of reset.
 * Enable - Enable / disable power to device.
 * Stop - Stop / start operation of device.
 *
 * @ctx: ACPI context pointer
 * @tx_state_val: Mask to use to toggle the TX state on the GPIO pin, e,g.
 *	PAD_CFG0_TX_STATE
 * @dw0_read: Name to use to read dw0, e.g. "\\_SB.GPC0"
 * @dw0_write: Name to use to read dw0, e.g. "\\_SB.SPC0"
 * @reset_gpio: GPIO used to take device out of reset or to put it into reset
 * @reset_delay_ms: Delay to be inserted after device is taken out of reset
 *	(_ON method delay)
 * @reset_off_delay_ms: Delay to be inserted after device is put into reset
 *	(_OFF method delay)
 * @enable_gpio: GPIO used to enable device
 * @enable_delay_ms: Delay to be inserted after device is enabled
 * @enable_off_delay_ms: Delay to be inserted after device is disabled
 *	(_OFF method delay)
 * @stop_gpio: GPIO used to stop operation of device
 * @stop_delay_ms: Delay to be inserted after disabling stop (_ON method delay)
 * @stop_off_delay_ms: Delay to be inserted after enabling stop.
 *	(_OFF method delay)
 *
 * Return: 0 if OK, -ve if at least one GPIO is not provided
 */
int acpi_device_add_power_res(struct acpi_ctx *ctx, u32 tx_state_val,
			      const char *dw0_read, const char *dw0_write,
			      const struct gpio_desc *reset_gpio,
			      uint reset_delay_ms, uint reset_off_delay_ms,
			      const struct gpio_desc *enable_gpio,
			      uint enable_delay_ms, uint enable_off_delay_ms,
			      const struct gpio_desc *stop_gpio,
			      uint stop_delay_ms, uint stop_off_delay_ms);

/**
 * acpi_device_infer_name() - Infer the name from its uclass or parent
 *
 * Many ACPI devices have a standard name that can be inferred from the uclass
 * they are in, or the uclass of their parent. These rules are implemented in
 * this function. It attempts to produce a name for a device based on these
 * rules.
 *
 * NOTE: This currently supports only x86 devices. Feel free to enhance it for
 * other architectures as needed.
 *
 * @dev: Device to check
 * @out_name: Place to put the name (must hold ACPI_NAME_MAX bytes)
 * Return: 0 if a name was found, -ENOENT if not found, -ENXIO if the device
 *	sequence number could not be determined
 */
int acpi_device_infer_name(const struct udevice *dev, char *out_name);

#endif

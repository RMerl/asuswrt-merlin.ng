// SPDX-License-Identifier: GPL-2.0
/*
 * Generation of tables for particular device types
 *
 * Copyright 2019 Google LLC
 * Mostly taken from coreboot file of the same name
 */

#include <dm.h>
#include <irq.h>
#include <log.h>
#include <usb.h>
#include <acpi/acpigen.h>
#include <acpi/acpi_device.h>
#include <acpi/acpigen.h>
#include <asm-generic/gpio.h>
#include <dm/acpi.h>

/**
 * acpi_device_path_fill() - Find the root device and build a path from there
 *
 * This recursively reaches back to the root device and progressively adds path
 * elements until the device is reached.
 *
 * @dev: Device to return path of
 * @buf: Buffer to hold the path
 * @buf_len: Length of buffer
 * @cur: Current position in the buffer
 * Return: new position in buffer after adding @dev, or -ve on error
 */
static int acpi_device_path_fill(const struct udevice *dev, char *buf,
				 size_t buf_len, int cur)
{
	char name[ACPI_NAME_MAX];
	int next = 0;
	int ret;

	ret = acpi_get_name(dev, name);
	if (ret)
		return ret;

	/*
	 * Make sure this name segment will fit, including the path segment
	 * separator and possible NULL terminator, if this is the last segment.
	 */
	if (cur + strlen(name) + 2 > buf_len)
		return -ENOSPC;

	/* Walk up the tree to the root device */
	if (dev_get_parent(dev)) {
		next = acpi_device_path_fill(dev_get_parent(dev), buf, buf_len,
					     cur);
		if (next < 0)
			return next;
	}

	/* Fill in the path from the root device */
	next += snprintf(buf + next, buf_len - next, "%s%s",
			 dev_get_parent(dev) && *name ? "." : "", name);

	return next;
}

int acpi_device_path(const struct udevice *dev, char *buf, int maxlen)
{
	int ret;

	ret = acpi_device_path_fill(dev, buf, maxlen, 0);
	if (ret < 0)
		return ret;

	return 0;
}

int acpi_device_scope(const struct udevice *dev, char *scope, int maxlen)
{
	int ret;

	if (!dev_get_parent(dev))
		return log_msg_ret("noparent", -EINVAL);

	ret = acpi_device_path_fill(dev_get_parent(dev), scope, maxlen, 0);
	if (ret < 0)
		return log_msg_ret("fill", ret);

	return 0;
}

enum acpi_dev_status acpi_device_status(const struct udevice *dev)
{
	return ACPI_DSTATUS_ALL_ON;
}

/**
 * largeres_write_len_f() - Write a placeholder word value
 *
 * Write a forward length for a large resource (2 bytes)
 *
 * Return: pointer to the zero word (for fixing up later)
 */
static void *largeres_write_len_f(struct acpi_ctx *ctx)
{
	u8 *p = acpigen_get_current(ctx);

	acpigen_emit_word(ctx, 0);

	return p;
}

/**
 * largeres_fill_from_len() - Fill in a length value
 *
 * This calculated the number of bytes since the provided @start and writes it
 * to @ptr, which was previous returned by largeres_write_len_f().
 *
 * @ptr: Word to update
 * @start: Start address to count from to calculated the length
 */
static void largeres_fill_from_len(struct acpi_ctx *ctx, char *ptr, u8 *start)
{
	u16 len = acpigen_get_current(ctx) - start;

	ptr[0] = len & 0xff;
	ptr[1] = (len >> 8) & 0xff;
}

/**
 * largeres_fill_len() - Fill in a length value, excluding the length itself
 *
 * Fill in the length field with the value calculated from after the 16bit
 * field to acpigen current. This is useful since the length value does not
 * include the length field itself.
 *
 * This calls acpi_device_largeres_fill_len() passing @ptr + 2 as @start
 *
 * @ptr: Word to update.
 */
static void largeres_fill_len(struct acpi_ctx *ctx, void *ptr)
{
	largeres_fill_from_len(ctx, ptr, ptr + sizeof(u16));
}

/* ACPI 6.3 section 6.4.3.6: Extended Interrupt Descriptor */
static int acpi_device_write_interrupt(struct acpi_ctx *ctx,
				       const struct acpi_irq *irq)
{
	void *desc_length;
	u8 flags;

	if (!irq->pin)
		return -ENOENT;

	/* This is supported by GpioInt() but not Interrupt() */
	if (irq->polarity == ACPI_IRQ_ACTIVE_BOTH)
		return -EINVAL;

	/* Byte 0: Descriptor Type */
	acpigen_emit_byte(ctx, ACPI_DESCRIPTOR_INTERRUPT);

	/* Byte 1-2: Length (filled in later) */
	desc_length = largeres_write_len_f(ctx);

	/*
	 * Byte 3: Flags
	 *  [7:5]: Reserved
	 *    [4]: Wake     (0=NO_WAKE   1=WAKE)
	 *    [3]: Sharing  (0=EXCLUSIVE 1=SHARED)
	 *    [2]: Polarity (0=HIGH      1=LOW)
	 *    [1]: Mode     (0=LEVEL     1=EDGE)
	 *    [0]: Resource (0=PRODUCER  1=CONSUMER)
	 */
	flags = BIT(0); /* ResourceConsumer */
	if (irq->mode == ACPI_IRQ_EDGE_TRIGGERED)
		flags |= BIT(1);
	if (irq->polarity == ACPI_IRQ_ACTIVE_LOW)
		flags |= BIT(2);
	if (irq->shared == ACPI_IRQ_SHARED)
		flags |= BIT(3);
	if (irq->wake == ACPI_IRQ_WAKE)
		flags |= BIT(4);
	acpigen_emit_byte(ctx, flags);

	/* Byte 4: Interrupt Table Entry Count */
	acpigen_emit_byte(ctx, 1);

	/* Byte 5-8: Interrupt Number */
	acpigen_emit_dword(ctx, irq->pin);

	/* Fill in Descriptor Length (account for len word) */
	largeres_fill_len(ctx, desc_length);

	return 0;
}

int acpi_device_write_interrupt_irq(struct acpi_ctx *ctx,
				    const struct irq *req_irq)
{
	struct acpi_irq irq;
	int ret;

	ret = irq_get_acpi(req_irq, &irq);
	if (ret)
		return log_msg_ret("get", ret);
	ret = acpi_device_write_interrupt(ctx, &irq);
	if (ret)
		return log_msg_ret("write", ret);

	return irq.pin;
}

/* ACPI 6.3 section 6.4.3.8.1 - GPIO Interrupt or I/O */
int acpi_device_write_gpio(struct acpi_ctx *ctx, const struct acpi_gpio *gpio)
{
	void *start, *desc_length;
	void *pin_table_offset, *vendor_data_offset, *resource_offset;
	u16 flags = 0;
	int pin;

	if (gpio->type > ACPI_GPIO_TYPE_IO)
		return -EINVAL;

	start = acpigen_get_current(ctx);

	/* Byte 0: Descriptor Type */
	acpigen_emit_byte(ctx, ACPI_DESCRIPTOR_GPIO);

	/* Byte 1-2: Length (fill in later) */
	desc_length = largeres_write_len_f(ctx);

	/* Byte 3: Revision ID */
	acpigen_emit_byte(ctx, ACPI_GPIO_REVISION_ID);

	/* Byte 4: GpioIo or GpioInt */
	acpigen_emit_byte(ctx, gpio->type);

	/*
	 * Byte 5-6: General Flags
	 *   [15:1]: 0 => Reserved
	 *      [0]: 1 => ResourceConsumer
	 */
	acpigen_emit_word(ctx, 1 << 0);

	switch (gpio->type) {
	case ACPI_GPIO_TYPE_INTERRUPT:
		/*
		 * Byte 7-8: GPIO Interrupt Flags
		 *   [15:5]: 0 => Reserved
		 *      [4]: Wake     (0=NO_WAKE   1=WAKE)
		 *      [3]: Sharing  (0=EXCLUSIVE 1=SHARED)
		 *    [2:1]: Polarity (0=HIGH      1=LOW     2=BOTH)
		 *      [0]: Mode     (0=LEVEL     1=EDGE)
		 */
		if (gpio->irq.mode == ACPI_IRQ_EDGE_TRIGGERED)
			flags |= 1 << 0;
		if (gpio->irq.shared == ACPI_IRQ_SHARED)
			flags |= 1 << 3;
		if (gpio->irq.wake == ACPI_IRQ_WAKE)
			flags |= 1 << 4;

		switch (gpio->irq.polarity) {
		case ACPI_IRQ_ACTIVE_HIGH:
			flags |= 0 << 1;
			break;
		case ACPI_IRQ_ACTIVE_LOW:
			flags |= 1 << 1;
			break;
		case ACPI_IRQ_ACTIVE_BOTH:
			flags |= 2 << 1;
			break;
		}
		break;

	case ACPI_GPIO_TYPE_IO:
		/*
		 * Byte 7-8: GPIO IO Flags
		 *   [15:4]: 0 => Reserved
		 *      [3]: Sharing  (0=EXCLUSIVE 1=SHARED)
		 *      [2]: 0 => Reserved
		 *    [1:0]: IO Restriction
		 *           0 => IoRestrictionNone
		 *           1 => IoRestrictionInputOnly
		 *           2 => IoRestrictionOutputOnly
		 *           3 => IoRestrictionNoneAndPreserve
		 */
		flags |= gpio->io_restrict & 3;
		if (gpio->io_shared)
			flags |= 1 << 3;
		break;
	}
	acpigen_emit_word(ctx, flags);

	/*
	 * Byte 9: Pin Configuration
	 *  0x01 => Default (no configuration applied)
	 *  0x02 => Pull-up
	 *  0x03 => Pull-down
	 *  0x04-0x7F => Reserved
	 *  0x80-0xff => Vendor defined
	 */
	acpigen_emit_byte(ctx, gpio->pull);

	/* Byte 10-11: Output Drive Strength in 1/100 mA */
	acpigen_emit_word(ctx, gpio->output_drive_strength);

	/* Byte 12-13: Debounce Timeout in 1/100 ms */
	acpigen_emit_word(ctx, gpio->interrupt_debounce_timeout);

	/* Byte 14-15: Pin Table Offset, relative to start */
	pin_table_offset = largeres_write_len_f(ctx);

	/* Byte 16: Reserved */
	acpigen_emit_byte(ctx, 0);

	/* Byte 17-18: Resource Source Name Offset, relative to start */
	resource_offset = largeres_write_len_f(ctx);

	/* Byte 19-20: Vendor Data Offset, relative to start */
	vendor_data_offset = largeres_write_len_f(ctx);

	/* Byte 21-22: Vendor Data Length */
	acpigen_emit_word(ctx, 0);

	/* Fill in Pin Table Offset */
	largeres_fill_from_len(ctx, pin_table_offset, start);

	/* Pin Table, one word for each pin */
	for (pin = 0; pin < gpio->pin_count; pin++)
		acpigen_emit_word(ctx, gpio->pins[pin]);

	/* Fill in Resource Source Name Offset */
	largeres_fill_from_len(ctx, resource_offset, start);

	/* Resource Source Name String */
	acpigen_emit_string(ctx, gpio->resource);

	/* Fill in Vendor Data Offset */
	largeres_fill_from_len(ctx, vendor_data_offset, start);

	/* Fill in GPIO Descriptor Length (account for len word) */
	largeres_fill_len(ctx, desc_length);

	return gpio->pins[0];
}

int acpi_device_write_gpio_desc(struct acpi_ctx *ctx,
				const struct gpio_desc *desc)
{
	struct acpi_gpio gpio;
	int ret;

	ret = gpio_get_acpi(desc, &gpio);
	if (ret)
		return log_msg_ret("desc", ret);
	ret = acpi_device_write_gpio(ctx, &gpio);
	if (ret < 0)
		return log_msg_ret("gpio", ret);

	return ret;
}

int acpi_device_write_interrupt_or_gpio(struct acpi_ctx *ctx,
					struct udevice *dev, const char *prop)
{
	struct irq req_irq;
	int pin;
	int ret;

	ret = irq_get_by_index(dev, 0, &req_irq);
	if (!ret) {
		ret = acpi_device_write_interrupt_irq(ctx, &req_irq);
		if (ret < 0)
			return log_msg_ret("irq", ret);
		pin = ret;
	} else {
		struct gpio_desc req_gpio;

		ret = gpio_request_by_name(dev, prop, 0, &req_gpio,
					   GPIOD_IS_IN);
		if (ret)
			return log_msg_ret("no gpio", ret);
		ret = acpi_device_write_gpio_desc(ctx, &req_gpio);
		if (ret < 0)
			return log_msg_ret("gpio", ret);
		pin = ret;
	}

	return pin;
}

/* PowerResource() with Enable and/or Reset control */
int acpi_device_add_power_res(struct acpi_ctx *ctx, u32 tx_state_val,
			      const char *dw0_read, const char *dw0_write,
			      const struct gpio_desc *reset_gpio,
			      uint reset_delay_ms, uint reset_off_delay_ms,
			      const struct gpio_desc *enable_gpio,
			      uint enable_delay_ms, uint enable_off_delay_ms,
			      const struct gpio_desc *stop_gpio,
			      uint stop_delay_ms, uint stop_off_delay_ms)
{
	static const char *const power_res_dev_states[] = { "_PR0", "_PR3" };
	struct acpi_gpio reset, enable, stop;
	bool has_reset, has_enable, has_stop;
	int ret;

	gpio_get_acpi(reset_gpio, &reset);
	gpio_get_acpi(enable_gpio, &enable);
	gpio_get_acpi(stop_gpio, &stop);
	has_reset = reset.pins[0];
	has_enable = enable.pins[0];
	has_stop = stop.pins[0];

	if (!has_reset && !has_enable && !has_stop)
		return -EINVAL;

	/* PowerResource (PRIC, 0, 0) */
	acpigen_write_power_res(ctx, "PRIC", 0, 0, power_res_dev_states,
				ARRAY_SIZE(power_res_dev_states));

	/* Method (_STA, 0, NotSerialized) { Return (0x1) } */
	acpigen_write_sta(ctx, 0x1);

	/* Method (_ON, 0, Serialized) */
	acpigen_write_method_serialized(ctx, "_ON", 0);
	if (has_reset) {
		ret = acpigen_set_enable_tx_gpio(ctx, tx_state_val, dw0_read,
						 dw0_write, &reset, true);
		if (ret)
			return log_msg_ret("reset1", ret);
	}
	if (has_enable) {
		ret = acpigen_set_enable_tx_gpio(ctx, tx_state_val, dw0_read,
						 dw0_write, &enable, true);
		if (ret)
			return log_msg_ret("enable1", ret);
		if (enable_delay_ms)
			acpigen_write_sleep(ctx, enable_delay_ms);
	}
	if (has_reset) {
		ret = acpigen_set_enable_tx_gpio(ctx, tx_state_val, dw0_read,
						 dw0_write, &reset, false);
		if (ret)
			return log_msg_ret("reset2", ret);
		if (reset_delay_ms)
			acpigen_write_sleep(ctx, reset_delay_ms);
	}
	if (has_stop) {
		ret = acpigen_set_enable_tx_gpio(ctx, tx_state_val, dw0_read,
						 dw0_write, &stop, false);
		if (ret)
			return log_msg_ret("stop1", ret);
		if (stop_delay_ms)
			acpigen_write_sleep(ctx, stop_delay_ms);
	}
	acpigen_pop_len(ctx);		/* _ON method */

	/* Method (_OFF, 0, Serialized) */
	acpigen_write_method_serialized(ctx, "_OFF", 0);
	if (has_stop) {
		ret = acpigen_set_enable_tx_gpio(ctx, tx_state_val, dw0_read,
						 dw0_write, &stop, true);
		if (ret)
			return log_msg_ret("stop2", ret);
		if (stop_off_delay_ms)
			acpigen_write_sleep(ctx, stop_off_delay_ms);
	}
	if (has_reset) {
		ret = acpigen_set_enable_tx_gpio(ctx, tx_state_val, dw0_read,
						 dw0_write, &reset, true);
		if (ret)
			return log_msg_ret("reset3", ret);
		if (reset_off_delay_ms)
			acpigen_write_sleep(ctx, reset_off_delay_ms);
	}
	if (has_enable) {
		ret = acpigen_set_enable_tx_gpio(ctx, tx_state_val, dw0_read,
						 dw0_write, &enable, false);
		if (ret)
			return log_msg_ret("enable2", ret);
		if (enable_off_delay_ms)
			acpigen_write_sleep(ctx, enable_off_delay_ms);
	}
	acpigen_pop_len(ctx);		/* _OFF method */

	acpigen_pop_len(ctx);		/* PowerResource PRIC */

	return 0;
}

int acpi_device_write_dsm_i2c_hid(struct acpi_ctx *ctx,
				  int hid_desc_reg_offset)
{
	int ret;

	acpigen_write_dsm_start(ctx);
	ret = acpigen_write_dsm_uuid_start(ctx, ACPI_DSM_I2C_HID_UUID);
	if (ret)
		return log_ret(ret);

	acpigen_write_dsm_uuid_start_cond(ctx, 0);
	/* ToInteger (Arg1, Local2) */
	acpigen_write_to_integer(ctx, ARG1_OP, LOCAL2_OP);
	/* If (LEqual (Local2, 0x0)) */
	acpigen_write_if_lequal_op_int(ctx, LOCAL2_OP, 0x0);
	/*   Return (Buffer (One) { 0x1f }) */
	acpigen_write_return_singleton_buffer(ctx, 0x1f);
	acpigen_pop_len(ctx);	/* Pop : If */
	/* Else */
	acpigen_write_else(ctx);
	/*   If (LEqual (Local2, 0x1)) */
	acpigen_write_if_lequal_op_int(ctx, LOCAL2_OP, 0x1);
	/*     Return (Buffer (One) { 0x3f }) */
	acpigen_write_return_singleton_buffer(ctx, 0x3f);
	acpigen_pop_len(ctx);	/* Pop : If */
	/*   Else */
	acpigen_write_else(ctx);
	/*     Return (Buffer (One) { 0x0 }) */
	acpigen_write_return_singleton_buffer(ctx, 0x0);
	acpigen_pop_len(ctx);	/* Pop : Else */
	acpigen_pop_len(ctx);	/* Pop : Else */
	acpigen_write_dsm_uuid_end_cond(ctx);

	acpigen_write_dsm_uuid_start_cond(ctx, 1);
	acpigen_write_return_byte(ctx, hid_desc_reg_offset);
	acpigen_write_dsm_uuid_end_cond(ctx);

	acpigen_write_dsm_uuid_end(ctx);
	acpigen_write_dsm_end(ctx);

	return 0;
}

/* ACPI 6.3 section 6.4.3.8.2.1 - I2cSerialBusV2() */
static void acpi_device_write_i2c(struct acpi_ctx *ctx,
				  const struct acpi_i2c *i2c)
{
	void *desc_length, *type_length;

	/* Byte 0: Descriptor Type */
	acpigen_emit_byte(ctx, ACPI_DESCRIPTOR_SERIAL_BUS);

	/* Byte 1+2: Length (filled in later) */
	desc_length = largeres_write_len_f(ctx);

	/* Byte 3: Revision ID */
	acpigen_emit_byte(ctx, ACPI_I2C_SERIAL_BUS_REVISION_ID);

	/* Byte 4: Resource Source Index is Reserved */
	acpigen_emit_byte(ctx, 0);

	/* Byte 5: Serial Bus Type is I2C */
	acpigen_emit_byte(ctx, ACPI_SERIAL_BUS_TYPE_I2C);

	/*
	 * Byte 6: Flags
	 *  [7:2]: 0 => Reserved
	 *    [1]: 1 => ResourceConsumer
	 *    [0]: 0 => ControllerInitiated
	 */
	acpigen_emit_byte(ctx, 1 << 1);

	/*
	 * Byte 7-8: Type Specific Flags
	 *   [15:1]: 0 => Reserved
	 *      [0]: 0 => 7bit, 1 => 10bit
	 */
	acpigen_emit_word(ctx, i2c->mode_10bit);

	/* Byte 9: Type Specific Revision ID */
	acpigen_emit_byte(ctx, ACPI_I2C_TYPE_SPECIFIC_REVISION_ID);

	/* Byte 10-11: I2C Type Data Length */
	type_length = largeres_write_len_f(ctx);

	/* Byte 12-15: I2C Bus Speed */
	acpigen_emit_dword(ctx, i2c->speed);

	/* Byte 16-17: I2C Slave Address */
	acpigen_emit_word(ctx, i2c->address);

	/* Fill in Type Data Length */
	largeres_fill_len(ctx, type_length);

	/* Byte 18+: ResourceSource */
	acpigen_emit_string(ctx, i2c->resource);

	/* Fill in I2C Descriptor Length */
	largeres_fill_len(ctx, desc_length);
}

/**
 * acpi_device_set_i2c() - Set up an ACPI I2C struct from a device
 *
 * The value of @scope is not copied, but only referenced. This implies the
 * caller has to ensure it stays valid for the lifetime of @i2c.
 *
 * @dev: I2C device to convert
 * @i2c: Place to put the new structure
 * @scope: Scope of the I2C device (this is the controller path)
 * Return: chip address of device
 */
static int acpi_device_set_i2c(const struct udevice *dev, struct acpi_i2c *i2c,
			       const char *scope)
{
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);
	struct udevice *bus = dev_get_parent(dev);

	memset(i2c, '\0', sizeof(*i2c));
	i2c->address = chip->chip_addr;
	i2c->mode_10bit = 0;

	/*
	 * i2c_bus->speed_hz is set if this device is probed, but if not we
	 * must use the device tree
	 */
	i2c->speed = dev_read_u32_default(bus, "clock-frequency",
					  I2C_SPEED_STANDARD_RATE);
	i2c->resource = scope;

	return i2c->address;
}

int acpi_device_write_i2c_dev(struct acpi_ctx *ctx, const struct udevice *dev)
{
	char scope[ACPI_PATH_MAX];
	struct acpi_i2c i2c;
	int ret;

	ret = acpi_device_scope(dev, scope, sizeof(scope));
	if (ret)
		return log_msg_ret("scope", ret);
	ret = acpi_device_set_i2c(dev, &i2c, scope);
	if (ret < 0)
		return log_msg_ret("set", ret);
	acpi_device_write_i2c(ctx, &i2c);

	return ret;
}

#ifdef CONFIG_SPI
/* ACPI 6.1 section 6.4.3.8.2.2 - SpiSerialBus() */
static void acpi_device_write_spi(struct acpi_ctx *ctx, const struct acpi_spi *spi)
{
	void *desc_length, *type_length;
	u16 flags = 0;

	/* Byte 0: Descriptor Type */
	acpigen_emit_byte(ctx, ACPI_DESCRIPTOR_SERIAL_BUS);

	/* Byte 1+2: Length (filled in later) */
	desc_length = largeres_write_len_f(ctx);

	/* Byte 3: Revision ID */
	acpigen_emit_byte(ctx, ACPI_SPI_SERIAL_BUS_REVISION_ID);

	/* Byte 4: Resource Source Index is Reserved */
	acpigen_emit_byte(ctx, 0);

	/* Byte 5: Serial Bus Type is SPI */
	acpigen_emit_byte(ctx, ACPI_SERIAL_BUS_TYPE_SPI);

	/*
	 * Byte 6: Flags
	 *  [7:2]: 0 => Reserved
	 *    [1]: 1 => ResourceConsumer
	 *    [0]: 0 => ControllerInitiated
	 */
	acpigen_emit_byte(ctx, BIT(1));

	/*
	 * Byte 7-8: Type Specific Flags
	 *   [15:2]: 0 => Reserveda
	 *      [1]: 0 => ActiveLow, 1 => ActiveHigh
	 *      [0]: 0 => FourWire, 1 => ThreeWire
	 */
	if (spi->wire_mode == SPI_3_WIRE_MODE)
		flags |= BIT(0);
	if (spi->device_select_polarity == SPI_POLARITY_HIGH)
		flags |= BIT(1);
	acpigen_emit_word(ctx, flags);

	/* Byte 9: Type Specific Revision ID */
	acpigen_emit_byte(ctx, ACPI_SPI_TYPE_SPECIFIC_REVISION_ID);

	/* Byte 10-11: SPI Type Data Length */
	type_length = largeres_write_len_f(ctx);

	/* Byte 12-15: Connection Speed */
	acpigen_emit_dword(ctx, spi->speed);

	/* Byte 16: Data Bit Length */
	acpigen_emit_byte(ctx, spi->data_bit_length);

	/* Byte 17: Clock Phase */
	acpigen_emit_byte(ctx, spi->clock_phase);

	/* Byte 18: Clock Polarity */
	acpigen_emit_byte(ctx, spi->clock_polarity);

	/* Byte 19-20: Device Selection */
	acpigen_emit_word(ctx, spi->device_select);

	/* Fill in Type Data Length */
	largeres_fill_len(ctx, type_length);

	/* Byte 21+: ResourceSource String */
	acpigen_emit_string(ctx, spi->resource);

	/* Fill in SPI Descriptor Length */
	largeres_fill_len(ctx, desc_length);
}

/**
 * acpi_device_set_spi() - Set up an ACPI SPI struct from a device
 *
 * The value of @scope is not copied, but only referenced. This implies the
 * caller has to ensure it stays valid for the lifetime of @spi.
 *
 * @dev: SPI device to convert
 * @spi: Place to put the new structure
 * @scope: Scope of the SPI device (this is the controller path)
 * Return: 0 (always)
 */
static int acpi_device_set_spi(const struct udevice *dev, struct acpi_spi *spi,
			       const char *scope)
{
	struct dm_spi_slave_plat *plat;
	struct spi_slave *slave = dev_get_parent_priv(dev);

	plat = dev_get_parent_plat(slave->dev);
	memset(spi, '\0', sizeof(*spi));
	spi->device_select = plat->cs;
	spi->device_select_polarity = SPI_POLARITY_LOW;
	spi->wire_mode = SPI_4_WIRE_MODE;
	spi->speed = plat->max_hz;
	spi->data_bit_length = slave->wordlen;
	spi->clock_phase = plat->mode & SPI_CPHA ?
		 SPI_CLOCK_PHASE_SECOND : SPI_CLOCK_PHASE_FIRST;
	spi->clock_polarity = plat->mode & SPI_CPOL ?
		 SPI_POLARITY_HIGH : SPI_POLARITY_LOW;
	spi->resource = scope;

	return 0;
}

int acpi_device_write_spi_dev(struct acpi_ctx *ctx, const struct udevice *dev)
{
	char scope[ACPI_PATH_MAX];
	struct acpi_spi spi;
	int ret;

	ret = acpi_device_scope(dev, scope, sizeof(scope));
	if (ret)
		return log_msg_ret("scope", ret);
	ret = acpi_device_set_spi(dev, &spi, scope);
	if (ret)
		return log_msg_ret("set", ret);
	acpi_device_write_spi(ctx, &spi);

	return 0;
}
#endif /* CONFIG_SPI */

static const char *acpi_name_from_id(enum uclass_id id)
{
	switch (id) {
	case UCLASS_USB_HUB:
		/* Root Hub */
		return "RHUB";
	/* DSDT: acpi/northbridge.asl */
	case UCLASS_NORTHBRIDGE:
		return "MCHC";
	/* DSDT: acpi/lpc.asl */
	case UCLASS_LPC:
		return "LPCB";
	/* DSDT: acpi/xhci.asl */
	case UCLASS_USB:
		/* This only supports USB3.0 controllers at present */
		return "XHCI";
	case UCLASS_PWM:
		return "PWM";
	default:
		return NULL;
	}
}

/* If you change this function, add test cases to dm_test_acpi_get_name() */
int acpi_device_infer_name(const struct udevice *dev, char *out_name)
{
	enum uclass_id parent_id = UCLASS_INVALID;
	enum uclass_id id;
	const char *name = NULL;

	id = device_get_uclass_id(dev);
	if (dev_get_parent(dev))
		parent_id = device_get_uclass_id(dev_get_parent(dev));

	if (id == UCLASS_SOUND)
		name = "HDAS";
	else if (id == UCLASS_PCI)
		name = "PCI0";
	else if (device_is_on_pci_bus(dev))
		name = acpi_name_from_id(id);
	if (!name) {
		switch (parent_id) {
		case UCLASS_USB: {
			struct usb_device *udev = dev_get_parent_priv(dev);

			sprintf(out_name, udev->speed >= USB_SPEED_SUPER ?
				"HS%02d" : "FS%02d", udev->portnr);
			name = out_name;
			break;
		}
		default:
			break;
		}
	}
	if (!name) {
		switch (id) {
		/* DSDT: acpi/lpss.asl */
		case UCLASS_SERIAL:
			sprintf(out_name, "URT%d", dev_seq(dev));
			name = out_name;
			break;
		case UCLASS_I2C:
			sprintf(out_name, "I2C%d", dev_seq(dev));
			name = out_name;
			break;
		case UCLASS_SPI:
			sprintf(out_name, "SPI%d", dev_seq(dev));
			name = out_name;
			break;
		default:
			break;
		}
	}
	if (!name) {
		log_warning("No name for device '%s'\n", dev->name);
		return -ENOENT;
	}
	if (name != out_name)
		acpi_copy_name(out_name, name);

	return 0;
}

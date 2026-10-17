// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2014 Google, Inc
 */

#define LOG_CATEGORY UCLASS_I2C

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <log.h>
#include <malloc.h>
#include <acpi/acpi_device.h>
#include <dm/acpi.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <dm/pinctrl.h>
#if CONFIG_IS_ENABLED(DM_GPIO)
#include <asm/gpio.h>
#endif
#include <linux/delay.h>
#include "acpi_i2c.h"

#define I2C_MAX_OFFSET_LEN	4

enum {
	PIN_SDA = 0,
	PIN_SCL,
	PIN_COUNT,
};

/* Useful debugging function */
void i2c_dump_msgs(struct i2c_msg *msg, int nmsgs)
{
	int i;

	for (i = 0; i < nmsgs; i++) {
		struct i2c_msg *m = &msg[i];

		printf("   %s %x len=%x", m->flags & I2C_M_RD ? "R" : "W",
		       msg->addr, msg->len);
		if (!(m->flags & I2C_M_RD))
			printf(": %x", m->buf[0]);
		printf("\n");
	}
}

/**
 * i2c_setup_offset() - Set up a new message with a chip offset
 *
 * @chip:	Chip to use
 * @offset:	Byte offset within chip
 * @offset_buf:	Place to put byte offset
 * @msg:	Message buffer
 * Return: 0 if OK, -EADDRNOTAVAIL if the offset length is 0. In that case the
 * message is still set up but will not contain an offset.
 */
static int i2c_setup_offset(struct dm_i2c_chip *chip, uint offset,
			    uint8_t offset_buf[], struct i2c_msg *msg)
{
	int offset_len = chip->offset_len;

	msg->addr = chip->chip_addr;
	if (chip->chip_addr_offset_mask)
		msg->addr |= (offset >> (8 * offset_len)) &
			chip->chip_addr_offset_mask;
	msg->flags = chip->flags & DM_I2C_CHIP_10BIT ? I2C_M_TEN : 0;
	msg->len = chip->offset_len;
	msg->buf = offset_buf;
	if (!offset_len)
		return -EADDRNOTAVAIL;
	assert(offset_len <= I2C_MAX_OFFSET_LEN);

	while (offset_len--)
		*offset_buf++ = offset >> (8 * offset_len);

	return 0;
}

static int i2c_read_bytewise(struct udevice *dev, uint offset,
			     uint8_t *buffer, int len)
{
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);
	struct udevice *bus = dev_get_parent(dev);
	struct dm_i2c_ops *ops = i2c_get_ops(bus);
	struct i2c_msg msg[2], *ptr;
	uint8_t offset_buf[I2C_MAX_OFFSET_LEN];
	int ret;
	int i;

	for (i = 0; i < len; i++) {
		if (i2c_setup_offset(chip, offset + i, offset_buf, msg))
			return -EINVAL;
		ptr = msg + 1;
		ptr->addr = msg->addr;
		ptr->flags = msg->flags | I2C_M_RD;
		ptr->len = 1;
		ptr->buf = &buffer[i];
		ptr++;

		ret = ops->xfer(bus, msg, ptr - msg);
		if (ret)
			return ret;
	}

	return 0;
}

static int i2c_write_bytewise(struct udevice *dev, uint offset,
			     const uint8_t *buffer, int len)
{
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);
	struct udevice *bus = dev_get_parent(dev);
	struct dm_i2c_ops *ops = i2c_get_ops(bus);
	struct i2c_msg msg[1];
	uint8_t buf[I2C_MAX_OFFSET_LEN + 1];
	int ret;
	int i;

	for (i = 0; i < len; i++) {
		if (i2c_setup_offset(chip, offset + i, buf, msg))
			return -EINVAL;
		buf[msg->len++] = buffer[i];

		ret = ops->xfer(bus, msg, 1);
		if (ret)
			return ret;
	}

	return 0;
}

int dm_i2c_read(struct udevice *dev, uint offset, uint8_t *buffer, int len)
{
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);
	struct udevice *bus = dev_get_parent(dev);
	struct dm_i2c_ops *ops = i2c_get_ops(bus);
	struct i2c_msg msg[2], *ptr;
	uint8_t offset_buf[I2C_MAX_OFFSET_LEN];
	int msg_count;

	if (!ops->xfer)
		return -ENOSYS;
	if (chip->flags & DM_I2C_CHIP_RD_ADDRESS)
		return i2c_read_bytewise(dev, offset, buffer, len);
	ptr = msg;
	if (!i2c_setup_offset(chip, offset, offset_buf, ptr))
		ptr++;

	if (len) {
		ptr->addr = msg->addr;
		ptr->flags = chip->flags & DM_I2C_CHIP_10BIT ? I2C_M_TEN : 0;
		ptr->flags |= I2C_M_RD;
		ptr->len = len;
		ptr->buf = buffer;
		ptr++;
	}
	msg_count = ptr - msg;

	return ops->xfer(bus, msg, msg_count);
}

int dm_i2c_write(struct udevice *dev, uint offset, const uint8_t *buffer,
		 int len)
{
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);
	struct udevice *bus = dev_get_parent(dev);
	struct dm_i2c_ops *ops = i2c_get_ops(bus);
	struct i2c_msg msg[1];
	uint8_t _buf[I2C_MAX_OFFSET_LEN + 64];
	uint8_t *buf = _buf;
	int ret;

	if (!ops->xfer)
		return -ENOSYS;

	if (chip->flags & DM_I2C_CHIP_WR_ADDRESS)
		return i2c_write_bytewise(dev, offset, buffer, len);
	/*
	 * The simple approach would be to send two messages here: one to
	 * set the offset and one to write the bytes. However some drivers
	 * will not be expecting this, and some chips won't like how the
	 * driver presents this on the I2C bus.
	 *
	 * The API does not support separate offset and data. We could extend
	 * it with a flag indicating that there is data in the next message
	 * that needs to be processed in the same transaction. We could
	 * instead add an additional buffer to each message. For now, handle
	 * this in the uclass since it isn't clear what the impact on drivers
	 * would be with this extra complication. Unfortunately this means
	 * copying the message.
	 *
	 * Use the stack for small messages, malloc() for larger ones. We
	 * need to allow space for the offset (up to 4 bytes) and the message
	 * itself.
	 */
	if (len > sizeof(_buf) - I2C_MAX_OFFSET_LEN) {
		buf = malloc(I2C_MAX_OFFSET_LEN + len);
		if (!buf)
			return -ENOMEM;
	}

	i2c_setup_offset(chip, offset, buf, msg);
	msg->len += len;
	memcpy(buf + chip->offset_len, buffer, len);

	ret = ops->xfer(bus, msg, 1);
	if (buf != _buf)
		free(buf);
	return ret;
}

int dm_i2c_xfer(struct udevice *dev, struct i2c_msg *msg, int nmsgs)
{
	struct udevice *bus = dev_get_parent(dev);
	struct dm_i2c_ops *ops = i2c_get_ops(bus);

	if (!ops->xfer)
		return -ENOSYS;

	return ops->xfer(bus, msg, nmsgs);
}

int dm_i2c_reg_read(struct udevice *dev, uint offset)
{
	uint8_t val;
	int ret;

	ret = dm_i2c_read(dev, offset, &val, 1);
	if (ret < 0)
		return ret;

	return val;
}

int dm_i2c_reg_write(struct udevice *dev, uint offset, uint value)
{
	uint8_t val = value;

	return dm_i2c_write(dev, offset, &val, 1);
}

int dm_i2c_reg_clrset(struct udevice *dev, uint offset, u32 clr, u32 set)
{
	uint8_t val;
	int ret;

	ret = dm_i2c_read(dev, offset, &val, 1);
	if (ret < 0)
		return ret;

	val &= ~clr;
	val |= set;

	return dm_i2c_write(dev, offset, &val, 1);
}

/**
 * i2c_probe_chip() - probe for a chip on a bus
 *
 * @bus:	Bus to probe
 * @chip_addr:	Chip address to probe
 * @flags:	Flags for the chip
 * Return: 0 if found, -ENOSYS if the driver is invalid, -EREMOTEIO if the chip
 * does not respond to probe
 */
static int i2c_probe_chip(struct udevice *bus, uint chip_addr,
			  enum dm_i2c_chip_flags chip_flags)
{
	struct dm_i2c_ops *ops = i2c_get_ops(bus);
	struct i2c_msg msg[1];
	int ret;

	if (ops->probe_chip) {
		ret = ops->probe_chip(bus, chip_addr, chip_flags);
		if (ret != -ENOSYS)
			return ret;
	}

	if (!ops->xfer)
		return -ENOSYS;

	/* Probe with a zero-length message */
	msg->addr = chip_addr;
	msg->flags = chip_flags & DM_I2C_CHIP_10BIT ? I2C_M_TEN : 0;
	msg->len = 0;
	msg->buf = NULL;

	return ops->xfer(bus, msg, 1);
}

static int i2c_bind_driver(struct udevice *bus, uint chip_addr, uint offset_len,
			   struct udevice **devp)
{
	struct dm_i2c_chip *chip;
	char name[30], *str;
	struct udevice *dev;
	int ret;

	snprintf(name, sizeof(name), "generic_%x", chip_addr);
	str = strdup(name);
	if (!str)
		return -ENOMEM;
	ret = device_bind_driver(bus, "i2c_generic_chip_drv", str, &dev);
	debug("%s:  device_bind_driver: ret=%d\n", __func__, ret);
	if (ret)
		goto err_bind;

	/* Tell the device what we know about it */
	chip = dev_get_parent_plat(dev);
	chip->chip_addr = chip_addr;
	chip->offset_len = offset_len;
	ret = device_probe(dev);
	debug("%s:  device_probe: ret=%d\n", __func__, ret);
	if (ret)
		goto err_probe;

	*devp = dev;
	return 0;

err_probe:
	/*
	 * If the device failed to probe, unbind it. There is nothing there
	 * on the bus so we don't want to leave it lying around
	 */
	device_unbind(dev);
err_bind:
	free(str);
	return ret;
}

int i2c_get_chip(struct udevice *bus, uint chip_addr, uint offset_len,
		 struct udevice **devp)
{
	struct udevice *dev;

	debug("%s: Searching bus '%s' for address %02x: ", __func__,
	      bus->name, chip_addr);
	for (device_find_first_child(bus, &dev); dev;
			device_find_next_child(&dev)) {
		struct dm_i2c_chip *chip = dev_get_parent_plat(dev);
		int ret;

		if (chip->chip_addr == (chip_addr &
					~chip->chip_addr_offset_mask)) {
			ret = device_probe(dev);
			debug("found, ret=%d\n", ret);
			if (ret)
				return ret;
			*devp = dev;
			return 0;
		}
	}
	debug("not found\n");
	return i2c_bind_driver(bus, chip_addr, offset_len, devp);
}

int i2c_get_chip_for_busnum(int busnum, int chip_addr, uint offset_len,
			    struct udevice **devp)
{
	struct udevice *bus;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_I2C, busnum, &bus);
	if (ret) {
		debug("Cannot find I2C bus %d\n", busnum);
		return ret;
	}

	/* detect the presence of the chip on the bus */
	ret = i2c_probe_chip(bus, chip_addr, 0);
	debug("%s: bus='%s', address %02x, ret=%d\n", __func__, bus->name,
	      chip_addr, ret);
	if (ret) {
		debug("Cannot detect I2C chip %02x on bus %d\n", chip_addr,
		      busnum);
		return ret;
	}

	ret = i2c_get_chip(bus, chip_addr, offset_len, devp);
	if (ret) {
		debug("Cannot find I2C chip %02x on bus %d\n", chip_addr,
		      busnum);
		return ret;
	}

	return 0;
}

/* Find and probe I2C bus based on a chip attached to it */
static int i2c_get_parent_bus(ofnode chip, struct udevice **devp)
{
	ofnode node;
	struct udevice *dev;
	int ret;

	node = ofnode_get_parent(chip);
	if (!ofnode_valid(node))
		return -ENODEV;

	ret = uclass_get_device_by_ofnode(UCLASS_I2C, node, &dev);
	if (ret) {
		*devp = NULL;
		return ret;
	}

	*devp = dev;
	return 0;
}

int i2c_get_chip_by_phandle(const struct udevice *parent, const char *prop_name,
			    struct udevice **devp)
{
	ofnode node;
	uint phandle;
	struct udevice *bus, *chip;
	char *dev_name;
	int ret;

	debug("%s: Searching I2C chip for phandle \"%s\"\n",
	      __func__, prop_name);

	dev_name = strdup(prop_name);
	if (!dev_name) {
		ret = -ENOMEM;
		goto err_exit;
	}

	ret = dev_read_u32(parent, prop_name, &phandle);
	if (ret)
		goto err_exit;

	node = ofnode_get_by_phandle(phandle);
	if (!ofnode_valid(node)) {
		ret = -ENODEV;
		goto err_exit;
	}

	ret = i2c_get_parent_bus(node, &bus);
	if (ret)
		goto err_exit;

	ret = device_bind_driver_to_node(bus, "i2c_generic_chip_drv",
					 dev_name, node, &chip);
	if (ret)
		goto err_exit;

	ret = device_probe(chip);
	if (ret) {
		device_unbind(chip);
		goto err_exit;
	}

	debug("%s succeeded\n", __func__);
	*devp = chip;
	return 0;

err_exit:
	free(dev_name);
	debug("%s failed, ret = %d\n", __func__, ret);
	*devp = NULL;
	return ret;
}

int dm_i2c_probe(struct udevice *bus, uint chip_addr, uint chip_flags,
		 struct udevice **devp)
{
	int ret;

	*devp = NULL;

	/* First probe that chip */
	ret = i2c_probe_chip(bus, chip_addr, chip_flags);
	debug("%s: bus='%s', address %02x, ret=%d\n", __func__, bus->name,
	      chip_addr, ret);
	if (ret)
		return ret;

	/* The chip was found, see if we have a driver, and probe it */
	ret = i2c_get_chip(bus, chip_addr, 1, devp);
	debug("%s:  i2c_get_chip: ret=%d\n", __func__, ret);

	return ret;
}

int dm_i2c_set_bus_speed(struct udevice *bus, unsigned int speed)
{
	struct dm_i2c_ops *ops = i2c_get_ops(bus);
	struct dm_i2c_bus *i2c = dev_get_uclass_priv(bus);
	int ret;

	/*
	 * If we have a method, call it. If not then the driver probably wants
	 * to deal with speed changes on the next transfer. It can easily read
	 * the current speed from this uclass
	 */
	if (ops->set_bus_speed) {
		ret = ops->set_bus_speed(bus, speed);
		if (ret)
			return ret;
	}
	i2c->speed_hz = speed;

	return 0;
}

int dm_i2c_get_bus_speed(struct udevice *bus)
{
	struct dm_i2c_ops *ops = i2c_get_ops(bus);
	struct dm_i2c_bus *i2c = dev_get_uclass_priv(bus);

	if (!ops->get_bus_speed)
		return i2c->speed_hz;

	return ops->get_bus_speed(bus);
}

int i2c_set_chip_flags(struct udevice *dev, uint flags)
{
	struct udevice *bus = dev->parent;
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);
	struct dm_i2c_ops *ops = i2c_get_ops(bus);
	int ret;

	if (ops->set_flags) {
		ret = ops->set_flags(dev, flags);
		if (ret)
			return ret;
	}
	chip->flags = flags;

	return 0;
}

int i2c_get_chip_flags(struct udevice *dev, uint *flagsp)
{
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);

	*flagsp = chip->flags;

	return 0;
}

int i2c_set_chip_offset_len(struct udevice *dev, uint offset_len)
{
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);

	if (offset_len > I2C_MAX_OFFSET_LEN)
		return log_ret(-EINVAL);
	chip->offset_len = offset_len;

	return 0;
}

int i2c_get_chip_offset_len(struct udevice *dev)
{
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);

	return chip->offset_len;
}

int i2c_set_chip_addr_offset_mask(struct udevice *dev, uint mask)
{
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);

	chip->chip_addr_offset_mask = mask;

	return 0;
}

uint i2c_get_chip_addr_offset_mask(struct udevice *dev)
{
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);

	return chip->chip_addr_offset_mask;
}

#if CONFIG_IS_ENABLED(DM_GPIO)
static void i2c_gpio_set_pin(struct gpio_desc *pin, int bit)
{
	if (bit)
		dm_gpio_set_dir_flags(pin, GPIOD_IS_IN);
	else
		dm_gpio_set_dir_flags(pin, GPIOD_IS_OUT |
					   GPIOD_IS_OUT_ACTIVE);
}

static int i2c_gpio_get_pin(struct gpio_desc *pin)
{
	/* DTS need config GPIO_ACTIVE_LOW */
	return !dm_gpio_get_value(pin);
}

int i2c_deblock_gpio_loop(struct gpio_desc *sda_pin,
			  struct gpio_desc *scl_pin,
			  unsigned int scl_count,
			  unsigned int start_count,
			  unsigned int delay)
{
	int i, ret = -EREMOTEIO;

	i2c_gpio_set_pin(sda_pin, 1);
	i2c_gpio_set_pin(scl_pin, 1);
	udelay(delay);

	/*  Toggle SCL until slave release SDA */
	for (; scl_count; --scl_count) {
		i2c_gpio_set_pin(scl_pin, 1);
		udelay(delay);
		i2c_gpio_set_pin(scl_pin, 0);
		udelay(delay);
		if (i2c_gpio_get_pin(sda_pin)) {
			ret = 0;
			break;
		}
	}

	if (!ret && start_count) {
		for (i = 0; i < start_count; i++) {
			/* Send start condition */
			udelay(delay);
			i2c_gpio_set_pin(sda_pin, 1);
			udelay(delay);
			i2c_gpio_set_pin(scl_pin, 1);
			udelay(delay);
			i2c_gpio_set_pin(sda_pin, 0);
			udelay(delay);
			i2c_gpio_set_pin(scl_pin, 0);
		}
	}

	/* Then, send I2C stop */
	i2c_gpio_set_pin(sda_pin, 0);
	udelay(delay);

	i2c_gpio_set_pin(scl_pin, 1);
	udelay(delay);

	i2c_gpio_set_pin(sda_pin, 1);
	udelay(delay);

	if (!i2c_gpio_get_pin(sda_pin) || !i2c_gpio_get_pin(scl_pin))
		ret = -EREMOTEIO;

	return ret;
}

static int i2c_deblock_gpio(struct udevice *bus)
{
	struct gpio_desc gpios[PIN_COUNT];
	int ret, ret0;

	ret = gpio_request_list_by_name(bus, "gpios", gpios,
					ARRAY_SIZE(gpios), GPIOD_IS_IN);
	if (ret != ARRAY_SIZE(gpios)) {
		debug("%s: I2C Node '%s' has no 'gpios' property %s\n",
		      __func__, dev_read_name(bus), bus->name);
		if (ret >= 0) {
			gpio_free_list(bus, gpios, ret);
			ret = -ENOENT;
		}
		goto out;
	}

	ret = pinctrl_select_state(bus, "gpio");
	if (ret) {
		debug("%s: I2C Node '%s' has no 'gpio' pinctrl state. %s\n",
		      __func__, dev_read_name(bus), bus->name);
		goto out_no_pinctrl;
	}

	ret0 = i2c_deblock_gpio_loop(&gpios[PIN_SDA], &gpios[PIN_SCL], 9, 0, 5);

	ret = pinctrl_select_state(bus, "default");
	if (ret) {
		debug("%s: I2C Node '%s' has no 'default' pinctrl state. %s\n",
		      __func__, dev_read_name(bus), bus->name);
	}

	ret = !ret ? ret0 : ret;

out_no_pinctrl:
	gpio_free_list(bus, gpios, ARRAY_SIZE(gpios));
out:
	return ret;
}
#else
static int i2c_deblock_gpio(struct udevice *bus)
{
	return -ENOSYS;
}
#endif /* DM_GPIO */

int i2c_deblock(struct udevice *bus)
{
	struct dm_i2c_ops *ops = i2c_get_ops(bus);

	if (!ops->deblock)
		return i2c_deblock_gpio(bus);

	return ops->deblock(bus);
}

#if CONFIG_IS_ENABLED(OF_REAL)
int i2c_chip_of_to_plat(struct udevice *dev, struct dm_i2c_chip *chip)
{
	int addr;

	chip->offset_len = dev_read_u32_default(dev, "u-boot,i2c-offset-len",
						1);
	chip->flags = 0;
	addr = dev_read_u32_default(dev, "reg", -1);
	if (addr == -1) {
		debug("%s: I2C Node '%s' has no 'reg' property %s\n", __func__,
		      dev_read_name(dev), dev->name);
		return log_ret(-EINVAL);
	}
	chip->chip_addr = addr;

	return 0;
}
#endif

static int i2c_pre_probe(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(OF_REAL)
	struct dm_i2c_bus *i2c = dev_get_uclass_priv(dev);
	unsigned int max = 0;
	ofnode node;
	int ret;

	i2c->max_transaction_bytes = 0;
	dev_for_each_subnode(node, dev) {
		ret = ofnode_read_u32(node,
				      "u-boot,i2c-transaction-bytes",
				      &max);
		if (!ret && max > i2c->max_transaction_bytes)
			i2c->max_transaction_bytes = max;
	}

	debug("%s: I2C bus: %s max transaction bytes: %d\n", __func__,
	      dev->name, i2c->max_transaction_bytes);
#endif
	return 0;
}

static int i2c_post_probe(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(OF_REAL)
	struct dm_i2c_bus *i2c = dev_get_uclass_priv(dev);

	i2c->speed_hz = dev_read_u32_default(dev, "clock-frequency",
					     I2C_SPEED_STANDARD_RATE);

	return dm_i2c_set_bus_speed(dev, i2c->speed_hz);
#else
	return 0;
#endif
}

static int i2c_child_post_bind(struct udevice *dev)
{
#if CONFIG_IS_ENABLED(OF_REAL)
	struct dm_i2c_chip *plat = dev_get_parent_plat(dev);

	if (!dev_has_ofnode(dev))
		return 0;
	return i2c_chip_of_to_plat(dev, plat);
#else
	return 0;
#endif
}

static int i2c_post_bind(struct udevice *dev)
{
	int ret = 0;

	debug("%s: %s, seq=%d\n", __func__, dev->name, dev_seq(dev));

#if CONFIG_IS_ENABLED(OF_REAL)
	ret = dm_scan_fdt_dev(dev);
#endif
	return ret;
}

UCLASS_DRIVER(i2c) = {
	.id		= UCLASS_I2C,
	.name		= "i2c",
	.flags		= DM_UC_FLAG_SEQ_ALIAS,
	.post_bind	= i2c_post_bind,
	.pre_probe      = i2c_pre_probe,
	.post_probe	= i2c_post_probe,
	.per_device_auto	= sizeof(struct dm_i2c_bus),
	.per_child_plat_auto	= sizeof(struct dm_i2c_chip),
	.child_post_bind = i2c_child_post_bind,
};

UCLASS_DRIVER(i2c_generic) = {
	.id		= UCLASS_I2C_GENERIC,
	.name		= "i2c_generic",
};

static const struct udevice_id generic_chip_i2c_ids[] = {
	{ .compatible = "i2c-chip", .data = I2C_DEVICE_GENERIC },
#if CONFIG_IS_ENABLED(ACPIGEN)
	{ .compatible = "hid-over-i2c", .data = I2C_DEVICE_HID_OVER_I2C },
#endif
	{ }
};

U_BOOT_DRIVER(i2c_generic_chip_drv) = {
	.name		= "i2c_generic_chip_drv",
	.id		= UCLASS_I2C_GENERIC,
	.of_match	= generic_chip_i2c_ids,
#if CONFIG_IS_ENABLED(ACPIGEN)
	.of_to_plat	= acpi_i2c_of_to_plat,
	.priv_auto	= sizeof(struct acpi_i2c_priv),
#endif
	ACPI_OPS_PTR(&acpi_i2c_ops)
};

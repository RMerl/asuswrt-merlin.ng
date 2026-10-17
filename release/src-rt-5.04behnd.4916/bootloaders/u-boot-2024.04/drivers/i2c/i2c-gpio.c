/*
 * (C) Copyright 2015, Samsung Electronics
 * Przemyslaw Marczak <p.marczak@samsung.com>
 *
 * This file is based on: drivers/i2c/soft-i2c.c,
 * with added driver-model support and code cleanup.
 */
#include <common.h>
#include <errno.h>
#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <asm/gpio.h>
#include <linux/delay.h>
#include <linux/printk.h>

#define DEFAULT_UDELAY	5
#define RETRIES		0
#define I2C_ACK		0
#define I2C_NOACK	1

enum {
	PIN_SDA = 0,
	PIN_SCL,
	PIN_COUNT,
};

struct i2c_gpio_bus {
	/**
	  * udelay - delay [us] between GPIO toggle operations,
	  * which is 1/4 of I2C speed clock period.
	 */
	int udelay;
	 /* sda, scl */
	struct gpio_desc gpios[PIN_COUNT];

	int (*get_sda)(struct i2c_gpio_bus *bus);
	void (*set_sda)(struct i2c_gpio_bus *bus, int bit);
	void (*set_scl)(struct i2c_gpio_bus *bus, int bit);
};

static int i2c_gpio_sda_get(struct i2c_gpio_bus *bus)
{
	struct gpio_desc *sda = &bus->gpios[PIN_SDA];

	return dm_gpio_get_value(sda);
}

static void i2c_gpio_sda_set(struct i2c_gpio_bus *bus, int bit)
{
	struct gpio_desc *sda = &bus->gpios[PIN_SDA];
	ulong flags;

	if (bit)
		flags = GPIOD_IS_IN;
	else
		flags = GPIOD_IS_OUT;
	dm_gpio_clrset_flags(sda, GPIOD_MASK_DIR, flags);
}

static void i2c_gpio_scl_set(struct i2c_gpio_bus *bus, int bit)
{
	struct gpio_desc *scl = &bus->gpios[PIN_SCL];
	int count = 0;

	if (bit) {
		dm_gpio_clrset_flags(scl, GPIOD_MASK_DIR, GPIOD_IS_IN);
		while (!dm_gpio_get_value(scl) && count++ < 100000)
			udelay(1);

		if (!dm_gpio_get_value(scl))
			pr_err("timeout waiting on slave to release scl\n");
	} else {
		dm_gpio_clrset_flags(scl, GPIOD_MASK_DIR, GPIOD_IS_OUT);
	}
}

/* variant for output only gpios which cannot support clock stretching */
static void i2c_gpio_scl_set_output_only(struct i2c_gpio_bus *bus, int bit)
{
	struct gpio_desc *scl = &bus->gpios[PIN_SCL];
	ulong flags = GPIOD_IS_OUT;

	if (bit)
		flags |= GPIOD_IS_OUT_ACTIVE;
	dm_gpio_clrset_flags(scl, GPIOD_MASK_DIR, flags);
}

static void i2c_gpio_write_bit(struct i2c_gpio_bus *bus, int delay, uchar bit)
{
	bus->set_scl(bus, 0);
	udelay(delay);
	bus->set_sda(bus, bit);
	udelay(delay);
	bus->set_scl(bus, 1);
	udelay(2 * delay);
}

static int i2c_gpio_read_bit(struct i2c_gpio_bus *bus, int delay)
{
	int value;

	bus->set_scl(bus, 1);
	udelay(delay);
	value = bus->get_sda(bus);
	udelay(delay);
	bus->set_scl(bus, 0);
	udelay(2 * delay);

	return value;
}

/* START: High -> Low on SDA while SCL is High */
static void i2c_gpio_send_start(struct i2c_gpio_bus *bus, int delay)
{
	udelay(delay);
	bus->set_sda(bus, 1);
	udelay(delay);
	bus->set_scl(bus, 1);
	udelay(delay);
	bus->set_sda(bus, 0);
	udelay(delay);
}

/* STOP: Low -> High on SDA while SCL is High */
static void i2c_gpio_send_stop(struct i2c_gpio_bus *bus, int delay)
{
	bus->set_scl(bus, 0);
	udelay(delay);
	bus->set_sda(bus, 0);
	udelay(delay);
	bus->set_scl(bus, 1);
	udelay(delay);
	bus->set_sda(bus, 1);
	udelay(delay);
}

/* ack should be I2C_ACK or I2C_NOACK */
static void i2c_gpio_send_ack(struct i2c_gpio_bus *bus, int delay, int ack)
{
	i2c_gpio_write_bit(bus, delay, ack);
	bus->set_scl(bus, 0);
	udelay(delay);
}

/**
 * Send a reset sequence consisting of 9 clocks with the data signal high
 * to clock any confused device back into an idle state.  Also send a
 * <stop> at the end of the sequence for belts & suspenders.
 */
static void i2c_gpio_send_reset(struct i2c_gpio_bus *bus, int delay)
{
	int j;

	for (j = 0; j < 9; j++)
		i2c_gpio_write_bit(bus, delay, 1);

	i2c_gpio_send_stop(bus, delay);
}

/* Set sda high with low clock, before reading slave data */
static void i2c_gpio_sda_high(struct i2c_gpio_bus *bus, int delay)
{
	bus->set_scl(bus, 0);
	udelay(delay);
	bus->set_sda(bus, 1);
	udelay(delay);
}

/* Send 8 bits and look for an acknowledgement */
static int i2c_gpio_write_byte(struct i2c_gpio_bus *bus, int delay, uchar data)
{
	int j;
	int nack;

	for (j = 0; j < 8; j++) {
		i2c_gpio_write_bit(bus, delay, data & 0x80);
		data <<= 1;
	}

	udelay(delay);

	/* Look for an <ACK>(negative logic) and return it */
	i2c_gpio_sda_high(bus, delay);
	nack = i2c_gpio_read_bit(bus, delay);

	return nack;	/* not a nack is an ack */
}

/**
 * if ack == I2C_ACK, ACK the byte so can continue reading, else
 * send I2C_NOACK to end the read.
 */
static uchar i2c_gpio_read_byte(struct i2c_gpio_bus *bus, int delay, int ack)
{
	int  data;
	int  j;

	i2c_gpio_sda_high(bus, delay);
	data = 0;
	for (j = 0; j < 8; j++) {
		data <<= 1;
		data |= i2c_gpio_read_bit(bus, delay);
	}
	i2c_gpio_send_ack(bus, delay, ack);

	return data;
}

/* send start and the slave chip address */
int i2c_send_slave_addr(struct i2c_gpio_bus *bus, int delay, uchar chip)
{
	i2c_gpio_send_start(bus, delay);

	if (i2c_gpio_write_byte(bus, delay, chip)) {
		i2c_gpio_send_stop(bus, delay);
		return -EIO;
	}

	return 0;
}

static int i2c_gpio_write_data(struct i2c_gpio_bus *bus, uchar chip,
			       uchar *buffer, int len,
			       bool end_with_repeated_start)
{
	unsigned int delay = bus->udelay;
	int failures = 0;

	debug("%s: chip %x buffer %p len %d\n", __func__, chip, buffer, len);

	if (i2c_send_slave_addr(bus, delay, chip << 1)) {
		debug("i2c_write, no chip responded %02X\n", chip);
		return -EIO;
	}

	while (len-- > 0) {
		if (i2c_gpio_write_byte(bus, delay, *buffer++))
			failures++;
	}

	if (!end_with_repeated_start) {
		i2c_gpio_send_stop(bus, delay);
		return failures;
	}

	if (i2c_send_slave_addr(bus, delay, (chip << 1) | 0x1)) {
		debug("i2c_write, no chip responded %02X\n", chip);
		return -EIO;
	}

	return failures;
}

static int i2c_gpio_read_data(struct i2c_gpio_bus *bus, uchar chip,
			      uchar *buffer, int len)
{
	unsigned int delay = bus->udelay;

	debug("%s: chip %x buffer: %p len %d\n", __func__, chip, buffer, len);

	while (len-- > 0)
		*buffer++ = i2c_gpio_read_byte(bus, delay, len == 0);

	i2c_gpio_send_stop(bus, delay);

	return 0;
}

static int i2c_gpio_xfer(struct udevice *dev, struct i2c_msg *msg, int nmsgs)
{
	struct i2c_gpio_bus *bus = dev_get_priv(dev);
	int ret;

	for (; nmsgs > 0; nmsgs--, msg++) {
		bool next_is_read = nmsgs > 1 && (msg[1].flags & I2C_M_RD);

		if (msg->flags & I2C_M_RD) {
			ret = i2c_gpio_read_data(bus, msg->addr, msg->buf,
						 msg->len);
		} else {
			ret = i2c_gpio_write_data(bus, msg->addr, msg->buf,
						  msg->len, next_is_read);
		}

		if (ret)
			return -EREMOTEIO;
	}

	return 0;
}

static int i2c_gpio_probe(struct udevice *dev, uint chip, uint chip_flags)
{
	struct i2c_gpio_bus *bus = dev_get_priv(dev);
	unsigned int delay = bus->udelay;
	int ret;

	i2c_gpio_send_start(bus, delay);
	ret = i2c_gpio_write_byte(bus, delay, (chip << 1) | 0);
	i2c_gpio_send_stop(bus, delay);

	debug("%s: bus: %d (%s) chip: %x flags: %x ret: %d\n",
	      __func__, dev_seq(dev), dev->name, chip, chip_flags, ret);

	return ret;
}

static int i2c_gpio_set_bus_speed(struct udevice *dev, unsigned int speed_hz)
{
	struct i2c_gpio_bus *bus = dev_get_priv(dev);

	bus->udelay = 1000000 / (speed_hz << 2);

	i2c_gpio_send_reset(bus, bus->udelay);

	return 0;
}

static int i2c_gpio_drv_probe(struct udevice *dev)
{
	if (dev_read_bool(dev, "i2c-gpio,deblock")) {
		/* @200kHz 9 clocks = 44us, 62us is ok */
		const unsigned int DELAY_ABORT_SEQ = 62;
		struct i2c_gpio_bus *bus = dev_get_priv(dev);

		return i2c_deblock_gpio_loop(&bus->gpios[PIN_SDA],
					     &bus->gpios[PIN_SCL],
					     16, 5, DELAY_ABORT_SEQ);
	}

	return 0;
}

static int i2c_gpio_of_to_plat(struct udevice *dev)
{
	struct i2c_gpio_bus *bus = dev_get_priv(dev);
	int ret;

	/* "gpios" is deprecated and replaced by "sda-gpios" + "scl-gpios". */
	ret = gpio_request_list_by_name(dev, "gpios", bus->gpios,
					ARRAY_SIZE(bus->gpios), 0);
	if (ret == 0) {
		ret = gpio_request_by_name(dev, "sda-gpios", 0,
					   &bus->gpios[PIN_SDA], 0);
		if (ret < 0)
			goto error;
		ret = gpio_request_by_name(dev, "scl-gpios", 0,
					   &bus->gpios[PIN_SCL], 0);
	}
	if (ret < 0)
		goto error;

	bus->udelay = dev_read_u32_default(dev, "i2c-gpio,delay-us",
					   DEFAULT_UDELAY);

	bus->get_sda = i2c_gpio_sda_get;
	bus->set_sda = i2c_gpio_sda_set;
	if (dev_read_bool(dev, "i2c-gpio,scl-output-only"))
		bus->set_scl = i2c_gpio_scl_set_output_only;
	else
		bus->set_scl = i2c_gpio_scl_set;

	return 0;
error:
	pr_err("Can't get %s gpios! Error: %d\n", dev->name, ret);
	return ret;
}

static const struct dm_i2c_ops i2c_gpio_ops = {
	.xfer		= i2c_gpio_xfer,
	.probe_chip	= i2c_gpio_probe,
	.set_bus_speed	= i2c_gpio_set_bus_speed,
};

static const struct udevice_id i2c_gpio_ids[] = {
	{ .compatible = "i2c-gpio" },
	{ }
};

U_BOOT_DRIVER(i2c_gpio) = {
	.name	= "i2c-gpio",
	.id	= UCLASS_I2C,
	.of_match = i2c_gpio_ids,
	.probe	= i2c_gpio_drv_probe,
	.of_to_plat = i2c_gpio_of_to_plat,
	.priv_auto	= sizeof(struct i2c_gpio_bus),
	.ops	= &i2c_gpio_ops,
};

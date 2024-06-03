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
#include <asm/gpio.h>

#define DEFAULT_UDELAY	5
#define RETRIES		0
#define I2C_ACK		0
#define I2C_NOACK	1

DECLARE_GLOBAL_DATA_PTR;

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
};

static int i2c_gpio_sda_get(struct gpio_desc *sda)
{
	return dm_gpio_get_value(sda);
}

static void i2c_gpio_sda_set(struct gpio_desc *sda, int bit)
{
	if (bit)
		dm_gpio_set_dir_flags(sda, GPIOD_IS_IN);
	else
		dm_gpio_set_dir_flags(sda, GPIOD_IS_OUT);
}

static void i2c_gpio_scl_set(struct gpio_desc *scl, int bit)
{
	ulong flags = GPIOD_IS_OUT;

	if (bit)
		flags |= GPIOD_IS_OUT_ACTIVE;
	dm_gpio_set_dir_flags(scl, flags);
}

static void i2c_gpio_write_bit(struct gpio_desc *scl, struct gpio_desc *sda,
			       int delay, uchar bit)
{
	i2c_gpio_scl_set(scl, 0);
	udelay(delay);
	i2c_gpio_sda_set(sda, bit);
	udelay(delay);
	i2c_gpio_scl_set(scl, 1);
	udelay(2 * delay);
}

static int i2c_gpio_read_bit(struct gpio_desc *scl, struct gpio_desc *sda,
			     int delay)
{
	int value;

	i2c_gpio_scl_set(scl, 1);
	udelay(delay);
	value = i2c_gpio_sda_get(sda);
	udelay(delay);
	i2c_gpio_scl_set(scl, 0);
	udelay(2 * delay);

	return value;
}

/* START: High -> Low on SDA while SCL is High */
static void i2c_gpio_send_start(struct gpio_desc *scl, struct gpio_desc *sda,
				int delay)
{
	udelay(delay);
	i2c_gpio_sda_set(sda, 1);
	udelay(delay);
	i2c_gpio_scl_set(scl, 1);
	udelay(delay);
	i2c_gpio_sda_set(sda, 0);
	udelay(delay);
}

/* STOP: Low -> High on SDA while SCL is High */
static void i2c_gpio_send_stop(struct gpio_desc *scl, struct gpio_desc *sda,
			       int delay)
{
	i2c_gpio_scl_set(scl, 0);
	udelay(delay);
	i2c_gpio_sda_set(sda, 0);
	udelay(delay);
	i2c_gpio_scl_set(scl, 1);
	udelay(delay);
	i2c_gpio_sda_set(sda, 1);
	udelay(delay);
}

/* ack should be I2C_ACK or I2C_NOACK */
static void i2c_gpio_send_ack(struct gpio_desc *scl, struct gpio_desc *sda,
			      int delay, int ack)
{
	i2c_gpio_write_bit(scl, sda, delay, ack);
	i2c_gpio_scl_set(scl, 0);
	udelay(delay);
}

/**
 * Send a reset sequence consisting of 9 clocks with the data signal high
 * to clock any confused device back into an idle state.  Also send a
 * <stop> at the end of the sequence for belts & suspenders.
 */
static void i2c_gpio_send_reset(struct gpio_desc *scl, struct gpio_desc *sda,
				int delay)
{
	int j;

	for (j = 0; j < 9; j++)
		i2c_gpio_write_bit(scl, sda, delay, 1);

	i2c_gpio_send_stop(scl, sda, delay);
}

/* Set sda high with low clock, before reading slave data */
static void i2c_gpio_sda_high(struct gpio_desc *scl, struct gpio_desc *sda,
			      int delay)
{
	i2c_gpio_scl_set(scl, 0);
	udelay(delay);
	i2c_gpio_sda_set(sda, 1);
	udelay(delay);
}

/* Send 8 bits and look for an acknowledgement */
static int i2c_gpio_write_byte(struct gpio_desc *scl, struct gpio_desc *sda,
			       int delay, uchar data)
{
	int j;
	int nack;

	for (j = 0; j < 8; j++) {
		i2c_gpio_write_bit(scl, sda, delay, data & 0x80);
		data <<= 1;
	}

	udelay(delay);

	/* Look for an <ACK>(negative logic) and return it */
	i2c_gpio_sda_high(scl, sda, delay);
	nack = i2c_gpio_read_bit(scl, sda, delay);

	return nack;	/* not a nack is an ack */
}

/**
 * if ack == I2C_ACK, ACK the byte so can continue reading, else
 * send I2C_NOACK to end the read.
 */
static uchar i2c_gpio_read_byte(struct gpio_desc *scl, struct gpio_desc *sda,
				int delay, int ack)
{
	int  data;
	int  j;

	i2c_gpio_sda_high(scl, sda, delay);
	data = 0;
	for (j = 0; j < 8; j++) {
		data <<= 1;
		data |= i2c_gpio_read_bit(scl, sda, delay);
	}
	i2c_gpio_send_ack(scl, sda, delay, ack);

	return data;
}

/* send start and the slave chip address */
int i2c_send_slave_addr(struct gpio_desc *scl, struct gpio_desc *sda, int delay,
			uchar chip)
{
	i2c_gpio_send_start(scl, sda, delay);

	if (i2c_gpio_write_byte(scl, sda, delay, chip)) {
		i2c_gpio_send_stop(scl, sda, delay);
		return -EIO;
	}

	return 0;
}

static int i2c_gpio_write_data(struct i2c_gpio_bus *bus, uchar chip,
			       uchar *buffer, int len,
			       bool end_with_repeated_start)
{
	struct gpio_desc *scl = &bus->gpios[PIN_SCL];
	struct gpio_desc *sda = &bus->gpios[PIN_SDA];
	unsigned int delay = bus->udelay;
	int failures = 0;

	debug("%s: chip %x buffer %p len %d\n", __func__, chip, buffer, len);

	if (i2c_send_slave_addr(scl, sda, delay, chip << 1)) {
		debug("i2c_write, no chip responded %02X\n", chip);
		return -EIO;
	}

	while (len-- > 0) {
		if (i2c_gpio_write_byte(scl, sda, delay, *buffer++))
			failures++;
	}

	if (!end_with_repeated_start) {
		i2c_gpio_send_stop(scl, sda, delay);
		return failures;
	}

	if (i2c_send_slave_addr(scl, sda, delay, (chip << 1) | 0x1)) {
		debug("i2c_write, no chip responded %02X\n", chip);
		return -EIO;
	}

	return failures;
}

static int i2c_gpio_read_data(struct i2c_gpio_bus *bus, uchar chip,
			      uchar *buffer, int len)
{
	struct gpio_desc *scl = &bus->gpios[PIN_SCL];
	struct gpio_desc *sda = &bus->gpios[PIN_SDA];
	unsigned int delay = bus->udelay;

	debug("%s: chip %x buffer: %p len %d\n", __func__, chip, buffer, len);

	while (len-- > 0)
		*buffer++ = i2c_gpio_read_byte(scl, sda, delay, len == 0);

	i2c_gpio_send_stop(scl, sda, delay);

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
	struct gpio_desc *scl = &bus->gpios[PIN_SCL];
	struct gpio_desc *sda = &bus->gpios[PIN_SDA];
	unsigned int delay = bus->udelay;
	int ret;

	i2c_gpio_send_start(scl, sda, delay);
	ret = i2c_gpio_write_byte(scl, sda, delay, (chip << 1) | 0);
	i2c_gpio_send_stop(scl, sda, delay);

	debug("%s: bus: %d (%s) chip: %x flags: %x ret: %d\n",
	      __func__, dev->seq, dev->name, chip, chip_flags, ret);

	return ret;
}

static int i2c_gpio_set_bus_speed(struct udevice *dev, unsigned int speed_hz)
{
	struct i2c_gpio_bus *bus = dev_get_priv(dev);
	struct gpio_desc *scl = &bus->gpios[PIN_SCL];
	struct gpio_desc *sda = &bus->gpios[PIN_SDA];

	bus->udelay = 1000000 / (speed_hz << 2);

	i2c_gpio_send_reset(scl, sda, bus->udelay);

	return 0;
}

static int i2c_gpio_ofdata_to_platdata(struct udevice *dev)
{
	struct i2c_gpio_bus *bus = dev_get_priv(dev);
	const void *blob = gd->fdt_blob;
	int node = dev_of_offset(dev);
	int ret;

	ret = gpio_request_list_by_name(dev, "gpios", bus->gpios,
					ARRAY_SIZE(bus->gpios), 0);
	if (ret < 0)
		goto error;

	bus->udelay = fdtdec_get_int(blob, node, "i2c-gpio,delay-us",
				     DEFAULT_UDELAY);

	return 0;
error:
	pr_err("Can't get %s gpios! Error: %d", dev->name, ret);
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
	.ofdata_to_platdata = i2c_gpio_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct i2c_gpio_bus),
	.ops	= &i2c_gpio_ops,
};

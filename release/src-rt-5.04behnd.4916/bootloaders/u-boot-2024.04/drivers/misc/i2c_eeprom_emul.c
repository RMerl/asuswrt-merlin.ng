// SPDX-License-Identifier: GPL-2.0+
/*
 * Simulate an I2C eeprom
 *
 * Copyright (c) 2014 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <log.h>
#include <malloc.h>
#include <asm/test.h>

#ifdef DEBUG
#define debug_buffer print_buffer
#else
#define debug_buffer(x, ...)
#endif

struct sandbox_i2c_flash_plat_data {
	enum sandbox_i2c_eeprom_test_mode test_mode;
	const char *filename;
	int offset_len;		/* Length of an offset in bytes */
	int size;		/* Size of data buffer */
	uint chip_addr_offset_mask; /* mask of addr bits used for offset */
};

struct sandbox_i2c_flash {
	uint8_t *data;
	uint prev_addr;		/* slave address of previous access */
	uint prev_offset;	/* offset of previous access */
};

void sandbox_i2c_eeprom_set_test_mode(struct udevice *dev,
				      enum sandbox_i2c_eeprom_test_mode mode)
{
	struct sandbox_i2c_flash_plat_data *plat = dev_get_plat(dev);

	plat->test_mode = mode;
}

void sandbox_i2c_eeprom_set_offset_len(struct udevice *dev, int offset_len)
{
	struct sandbox_i2c_flash_plat_data *plat = dev_get_plat(dev);

	plat->offset_len = offset_len;
}

void sandbox_i2c_eeprom_set_chip_addr_offset_mask(struct udevice *dev,
						  uint mask)
{
	struct sandbox_i2c_flash_plat_data *plat = dev_get_plat(dev);

	plat->chip_addr_offset_mask = mask;
}

uint sanbox_i2c_eeprom_get_prev_addr(struct udevice *dev)
{
	struct sandbox_i2c_flash *priv = dev_get_priv(dev);

	return priv->prev_addr;
}

uint sanbox_i2c_eeprom_get_prev_offset(struct udevice *dev)
{
	struct sandbox_i2c_flash *priv = dev_get_priv(dev);

	return priv->prev_offset;
}

static int sandbox_i2c_eeprom_xfer(struct udevice *emul, struct i2c_msg *msg,
				  int nmsgs)
{
	struct sandbox_i2c_flash *priv = dev_get_priv(emul);
	struct sandbox_i2c_flash_plat_data *plat = dev_get_plat(emul);
	uint offset = msg->addr & plat->chip_addr_offset_mask;

	debug("\n%s\n", __func__);
	debug_buffer(0, priv->data, 1, 16, 0);

	/* store addr for testing visibity */
	priv->prev_addr = msg->addr;

	for (; nmsgs > 0; nmsgs--, msg++) {
		int len;
		u8 *ptr;

		if (!plat->size)
			return -ENODEV;
		len = msg->len;
		debug("   %s: msg->addr=%x msg->len=%d",
		      msg->flags & I2C_M_RD ? "read" : "write",
		      msg->addr, msg->len);
		if (msg->flags & I2C_M_RD) {
			if (plat->test_mode == SIE_TEST_MODE_SINGLE_BYTE)
				len = 1;
			debug(", offset %x, len %x: ", offset, len);
			if (offset + len > plat->size) {
				int overflow = offset + len - plat->size;
				int initial = len - overflow;

				memcpy(msg->buf, priv->data + offset, initial);
				memcpy(msg->buf + initial, priv->data,
				       overflow);
			} else {
				memcpy(msg->buf, priv->data + offset, len);
			}
			memset(msg->buf + len, '\xff', msg->len - len);
			debug_buffer(0, msg->buf, 1, msg->len, 0);
		} else if (len >= plat->offset_len) {
			int i;

			ptr = msg->buf;
			for (i = 0; i < plat->offset_len; i++, len--)
				offset = (offset << 8) | *ptr++;
			debug(", set offset %x: ", offset);
			debug_buffer(0, msg->buf, 1, msg->len, 0);
			if (plat->test_mode == SIE_TEST_MODE_SINGLE_BYTE)
				len = min(len, 1);

			/* store offset for testing visibility */
			priv->prev_offset = offset;

			/* For testing, map offsets into our limited buffer.
			 * offset wraps every 256 bytes
			 */
			offset &= 0xff;
			debug("mapped offset to %x\n", offset);

			if (offset + len > plat->size) {
				int overflow = offset + len - plat->size;
				int initial = len - overflow;

				memcpy(priv->data + offset, ptr, initial);
				memcpy(priv->data, ptr + initial, overflow);
			} else {
				memcpy(priv->data + offset, ptr, len);
			}
		}
	}
	debug_buffer(0, priv->data, 1, 16, 0);

	return 0;
}

struct dm_i2c_ops sandbox_i2c_emul_ops = {
	.xfer = sandbox_i2c_eeprom_xfer,
};

static int sandbox_i2c_eeprom_of_to_plat(struct udevice *dev)
{
	struct sandbox_i2c_flash_plat_data *plat = dev_get_plat(dev);

	plat->size = dev_read_u32_default(dev, "sandbox,size", 32);
	plat->filename = dev_read_string(dev, "sandbox,filename");
	if (!plat->filename) {
		debug("%s: No filename for device '%s'\n", __func__,
		      dev->name);
		return -EINVAL;
	}
	plat->test_mode = SIE_TEST_MODE_NONE;
	plat->offset_len = 1;
	plat->chip_addr_offset_mask = 0;

	return 0;
}

static int sandbox_i2c_eeprom_probe(struct udevice *dev)
{
	struct sandbox_i2c_flash_plat_data *plat = dev_get_plat(dev);
	struct sandbox_i2c_flash *priv = dev_get_priv(dev);
	/* For eth3 */
	const u8 mac[] = { 0x02, 0x00, 0x11, 0x22, 0x33, 0x45 };

	priv->data = calloc(1, plat->size);
	if (!priv->data)
		return -ENOMEM;

	memcpy(&priv->data[24], mac, sizeof(mac));

	return 0;
}

static int sandbox_i2c_eeprom_remove(struct udevice *dev)
{
	struct sandbox_i2c_flash *priv = dev_get_priv(dev);

	free(priv->data);

	return 0;
}

static const struct udevice_id sandbox_i2c_ids[] = {
	{ .compatible = "sandbox,i2c-eeprom" },
	{ }
};

U_BOOT_DRIVER(sandbox_i2c_emul) = {
	.name		= "sandbox_i2c_eeprom_emul",
	.id		= UCLASS_I2C_EMUL,
	.of_match	= sandbox_i2c_ids,
	.of_to_plat = sandbox_i2c_eeprom_of_to_plat,
	.probe		= sandbox_i2c_eeprom_probe,
	.remove		= sandbox_i2c_eeprom_remove,
	.priv_auto	= sizeof(struct sandbox_i2c_flash),
	.plat_auto	= sizeof(struct sandbox_i2c_flash_plat_data),
	.ops		= &sandbox_i2c_emul_ops,
};

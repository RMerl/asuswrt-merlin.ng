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
};

struct sandbox_i2c_flash {
	uint8_t *data;
};

void sandbox_i2c_eeprom_set_test_mode(struct udevice *dev,
				      enum sandbox_i2c_eeprom_test_mode mode)
{
	struct sandbox_i2c_flash_plat_data *plat = dev_get_platdata(dev);

	plat->test_mode = mode;
}

void sandbox_i2c_eeprom_set_offset_len(struct udevice *dev, int offset_len)
{
	struct sandbox_i2c_flash_plat_data *plat = dev_get_platdata(dev);

	plat->offset_len = offset_len;
}

static int sandbox_i2c_eeprom_xfer(struct udevice *emul, struct i2c_msg *msg,
				  int nmsgs)
{
	struct sandbox_i2c_flash *priv = dev_get_priv(emul);
	uint offset = 0;

	debug("\n%s\n", __func__);
	debug_buffer(0, priv->data, 1, 16, 0);
	for (; nmsgs > 0; nmsgs--, msg++) {
		struct sandbox_i2c_flash_plat_data *plat =
				dev_get_platdata(emul);
		int len;
		u8 *ptr;

		if (!plat->size)
			return -ENODEV;
		if (msg->addr + msg->len > plat->size) {
			debug("%s: Address %x, len %x is outside range 0..%x\n",
			      __func__, msg->addr, msg->len, plat->size);
			return -EINVAL;
		}
		len = msg->len;
		debug("   %s: msg->len=%d",
		      msg->flags & I2C_M_RD ? "read" : "write",
		      msg->len);
		if (msg->flags & I2C_M_RD) {
			if (plat->test_mode == SIE_TEST_MODE_SINGLE_BYTE)
				len = 1;
			debug(", offset %x, len %x: ", offset, len);
			memcpy(msg->buf, priv->data + offset, len);
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

			/* For testing, map offsets into our limited buffer */
			for (i = 24; i > 0; i -= 8) {
				if (offset > (1 << i)) {
					offset = (offset >> i) |
						(offset & ((1 << i) - 1));
					offset += i;
				}
			}
			memcpy(priv->data + offset, ptr, len);
		}
	}
	debug_buffer(0, priv->data, 1, 16, 0);

	return 0;
}

struct dm_i2c_ops sandbox_i2c_emul_ops = {
	.xfer = sandbox_i2c_eeprom_xfer,
};

static int sandbox_i2c_eeprom_ofdata_to_platdata(struct udevice *dev)
{
	struct sandbox_i2c_flash_plat_data *plat = dev_get_platdata(dev);

	plat->size = dev_read_u32_default(dev, "sandbox,size", 32);
	plat->filename = dev_read_string(dev, "sandbox,filename");
	if (!plat->filename) {
		debug("%s: No filename for device '%s'\n", __func__,
		      dev->name);
		return -EINVAL;
	}
	plat->test_mode = SIE_TEST_MODE_NONE;
	plat->offset_len = 1;

	return 0;
}

static int sandbox_i2c_eeprom_probe(struct udevice *dev)
{
	struct sandbox_i2c_flash_plat_data *plat = dev_get_platdata(dev);
	struct sandbox_i2c_flash *priv = dev_get_priv(dev);

	priv->data = calloc(1, plat->size);
	if (!priv->data)
		return -ENOMEM;

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
	.ofdata_to_platdata = sandbox_i2c_eeprom_ofdata_to_platdata,
	.probe		= sandbox_i2c_eeprom_probe,
	.remove		= sandbox_i2c_eeprom_remove,
	.priv_auto_alloc_size = sizeof(struct sandbox_i2c_flash),
	.platdata_auto_alloc_size = sizeof(struct sandbox_i2c_flash_plat_data),
	.ops		= &sandbox_i2c_emul_ops,
};

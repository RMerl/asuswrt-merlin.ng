/* SPDX-License-Identifier:	GPL-2.0+
 *
 * Copyright (c) 2018 Microchip Technology, Inc.
 *
 */

#include <common.h>
#include <linux/err.h>
#include <dm.h>
#include <w1-eeprom.h>
#include <w1.h>

#define W1_F2D_READ_EEPROM      0xf0

#define EEP_SANDBOX_SAMPLE_MEM "this is a sample EEPROM memory string."

static int eep_sandbox_read_buf(struct udevice *dev, unsigned int offset,
				u8 *buf, unsigned int count)
{
	/* do not allow to copy more than our maximum sample string */
	if (offset + count < strlen(EEP_SANDBOX_SAMPLE_MEM)) {
		offset = 0;
		count = strlen(EEP_SANDBOX_SAMPLE_MEM);
	}
	strncpy((char *)buf, EEP_SANDBOX_SAMPLE_MEM, count);

	/*
	 * in case the w1 subsystem uses some different kind of sandbox testing,
	 * like randomized gpio values , we take the buffer from there
	 */

	w1_reset_select(dev);

	w1_write_byte(dev, W1_F2D_READ_EEPROM);
	w1_write_byte(dev, offset & 0xff);
	w1_write_byte(dev, offset >> 8);

	w1_read_buf(dev, buf, count);

	/*
	 * even if read buf from w1 fails, return success as we hardcoded
	 * the buffer.
	 */
	return 0;
}

static const struct w1_eeprom_ops eep_sandbox_ops = {
	.read_buf	= eep_sandbox_read_buf,
};

static const struct udevice_id eep_sandbox_id[] = {
	{ .compatible = "sandbox,w1-eeprom", .data = W1_FAMILY_EEP_SANDBOX },
	{ },
};

U_BOOT_DRIVER(eep_sandbox) = {
	.name		= "eep_sandbox",
	.id		= UCLASS_W1_EEPROM,
	.of_match	= eep_sandbox_id,
	.ops		= &eep_sandbox_ops,
};

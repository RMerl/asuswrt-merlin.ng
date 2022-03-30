// SPDX-License-Identifier: GPL-2.0+
/*
 * ULCB board CPLD access support
 *
 * Copyright (C) 2017 Renesas Electronics Corporation
 * Copyright (C) 2017 Cogent Embedded, Inc.
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm.h>
#include <errno.h>
#include <linux/err.h>
#include <sysreset.h>

#define CPLD_ADDR_MODE		0x00 /* RW */
#define CPLD_ADDR_MUX		0x02 /* RW */
#define CPLD_ADDR_DIPSW6	0x08 /* R */
#define CPLD_ADDR_RESET		0x80 /* RW */
#define CPLD_ADDR_VERSION	0xFF /* R */

struct renesas_ulcb_sysreset_priv {
	struct gpio_desc	miso;
	struct gpio_desc	mosi;
	struct gpio_desc	sck;
	struct gpio_desc	sstbz;
};

static u32 cpld_read(struct udevice *dev, u8 addr)
{
	struct renesas_ulcb_sysreset_priv *priv = dev_get_priv(dev);
	u32 data = 0;
	int i;

	for (i = 0; i < 8; i++) {
		dm_gpio_set_value(&priv->mosi, !!(addr & 0x80)); /* MSB first */
		dm_gpio_set_value(&priv->sck, 1);
		addr <<= 1;
		dm_gpio_set_value(&priv->sck, 0);
	}

	dm_gpio_set_value(&priv->mosi, 0); /* READ */
	dm_gpio_set_value(&priv->sstbz, 0);
	dm_gpio_set_value(&priv->sck, 1);
	dm_gpio_set_value(&priv->sck, 0);
	dm_gpio_set_value(&priv->sstbz, 1);

	for (i = 0; i < 32; i++) {
		dm_gpio_set_value(&priv->sck, 1);
		data <<= 1;
		data |= dm_gpio_get_value(&priv->miso); /* MSB first */
		dm_gpio_set_value(&priv->sck, 0);
	}

	return data;
}

static void cpld_write(struct udevice *dev, u8 addr, u32 data)
{
	struct renesas_ulcb_sysreset_priv *priv = dev_get_priv(dev);
	int i;

	for (i = 0; i < 32; i++) {
		dm_gpio_set_value(&priv->mosi, data & (1 << 31)); /* MSB first */
		dm_gpio_set_value(&priv->sck, 1);
		data <<= 1;
		dm_gpio_set_value(&priv->sck, 0);
	}

	for (i = 0; i < 8; i++) {
		dm_gpio_set_value(&priv->mosi, addr & 0x80); /* MSB first */
		dm_gpio_set_value(&priv->sck, 1);
		addr <<= 1;
		dm_gpio_set_value(&priv->sck, 0);
	}

	dm_gpio_set_value(&priv->mosi, 1); /* WRITE */
	dm_gpio_set_value(&priv->sstbz, 0);
	dm_gpio_set_value(&priv->sck, 1);
	dm_gpio_set_value(&priv->sck, 0);
	dm_gpio_set_value(&priv->sstbz, 1);
}

static int do_cpld(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	u32 addr, val;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_SYSRESET,
					  DM_GET_DRIVER(sysreset_renesas_ulcb),
					  &dev);
	if (ret)
		return ret;

	if (argc == 2 && strcmp(argv[1], "info") == 0) {
		printf("CPLD version:\t\t\t0x%08x\n",
		       cpld_read(dev, CPLD_ADDR_VERSION));
		printf("H3 Mode setting (MD0..28):\t0x%08x\n",
		       cpld_read(dev, CPLD_ADDR_MODE));
		printf("Multiplexer settings:\t\t0x%08x\n",
		       cpld_read(dev, CPLD_ADDR_MUX));
		printf("DIPSW (SW6):\t\t\t0x%08x\n",
		       cpld_read(dev, CPLD_ADDR_DIPSW6));
		return 0;
	}

	if (argc < 3)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[2], NULL, 16);
	if (!(addr == CPLD_ADDR_VERSION || addr == CPLD_ADDR_MODE ||
	      addr == CPLD_ADDR_MUX || addr == CPLD_ADDR_DIPSW6 ||
	      addr == CPLD_ADDR_RESET)) {
		printf("Invalid CPLD register address\n");
		return CMD_RET_USAGE;
	}

	if (argc == 3 && strcmp(argv[1], "read") == 0) {
		printf("0x%x\n", cpld_read(dev, addr));
	} else if (argc == 4 && strcmp(argv[1], "write") == 0) {
		val = simple_strtoul(argv[3], NULL, 16);
		cpld_write(dev, addr, val);
	}

	return 0;
}

U_BOOT_CMD(
	cpld, 4, 1, do_cpld,
	"CPLD access",
	"info\n"
	"cpld read addr\n"
	"cpld write addr val\n"
);

static int renesas_ulcb_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	cpld_write(dev, CPLD_ADDR_RESET, 1);

	return -EINPROGRESS;
}

static int renesas_ulcb_sysreset_probe(struct udevice *dev)
{
	struct renesas_ulcb_sysreset_priv *priv = dev_get_priv(dev);

	if (gpio_request_by_name(dev, "gpio-miso", 0, &priv->miso,
				 GPIOD_IS_IN))
		return -EINVAL;

	if (gpio_request_by_name(dev, "gpio-sck", 0, &priv->sck,
				 GPIOD_IS_OUT))
		return -EINVAL;

	if (gpio_request_by_name(dev, "gpio-sstbz", 0, &priv->sstbz,
				 GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE))
		return -EINVAL;

	if (gpio_request_by_name(dev, "gpio-mosi", 0, &priv->mosi,
				 GPIOD_IS_OUT))
		return -EINVAL;

	/* PULL-UP on MISO line */
	setbits_le32(PFC_PUEN5, PUEN_SSI_SDATA4);

	/* Dummy read */
	cpld_read(dev, CPLD_ADDR_VERSION);

	return 0;
}

static struct sysreset_ops renesas_ulcb_sysreset = {
	.request	= renesas_ulcb_sysreset_request,
};

static const struct udevice_id renesas_ulcb_sysreset_ids[] = {
	{ .compatible = "renesas,ulcb-cpld" },
	{ }
};

U_BOOT_DRIVER(sysreset_renesas_ulcb) = {
	.name		= "renesas_ulcb_sysreset",
	.id		= UCLASS_SYSRESET,
	.ops		= &renesas_ulcb_sysreset,
	.probe		= renesas_ulcb_sysreset_probe,
	.of_match	= renesas_ulcb_sysreset_ids,
	.priv_auto_alloc_size = sizeof(struct renesas_ulcb_sysreset_priv),
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * V3MSK board CPLD access support
 *
 * Copyright (C) 2019 Renesas Electronics Corporation
 * Copyright (C) 2019 Cogent Embedded, Inc.
 *
 */

#include <asm/gpio.h>
#include <asm/io.h>
#include <dm.h>
#include <errno.h>
#include <linux/err.h>
#include <sysreset.h>
#include <linux/delay.h>
#include <command.h>

#define CPLD_ADDR_PRODUCT_L		0x000 /* R */
#define CPLD_ADDR_PRODUCT_H		0x001 /* R */
#define CPLD_ADDR_CPLD_VERSION_D	0x002 /* R */
#define CPLD_ADDR_CPLD_VERSION_Y	0x003 /* R */
#define CPLD_ADDR_MODE_SET_L		0x004 /* R/W */
#define CPLD_ADDR_MODE_SET_H		0x005 /* R/W */
#define CPLD_ADDR_MODE_APPLIED_L	0x006 /* R */
#define CPLD_ADDR_MODE_APPLIED_H	0x007 /* R */
#define CPLD_ADDR_DIPSW			0x008 /* R */
#define CPLD_ADDR_RESET			0x00A /* R/W */
#define CPLD_ADDR_POWER_CFG		0x00B /* R/W */
#define CPLD_ADDR_PERI_CFG1		0x00C /* R/W */
#define CPLD_ADDR_PERI_CFG2		0x00D /* R/W */
#define CPLD_ADDR_LEDS			0x00E /* R/W */
#define CPLD_ADDR_PCB_VERSION		0x300 /* R */
#define CPLD_ADDR_SOC_VERSION		0x301 /* R */
#define CPLD_ADDR_PCB_SN_L		0x302 /* R */
#define CPLD_ADDR_PCB_SN_H		0x303 /* R */

#define MDIO_DELAY			10 /* microseconds */

#define CPLD_MAX_GPIOS			2

struct renesas_v3msk_sysreset_priv {
	struct gpio_desc	miso;
	struct gpio_desc	mosi;
	struct gpio_desc	mdc;
	struct gpio_desc	enablez;
	/*
	 * V3MSK Videobox Mini board has CANFD PHY connected
	 * we must shutdown this chip to use bb pins
	 */
	struct gpio_desc	gpios[CPLD_MAX_GPIOS];
};

static void mdio_bb_active_mdio(struct renesas_v3msk_sysreset_priv *priv)
{
	dm_gpio_set_dir_flags(&priv->mosi, GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
}

static void mdio_bb_tristate_mdio(struct renesas_v3msk_sysreset_priv *priv)
{
	dm_gpio_set_dir_flags(&priv->mosi, GPIOD_IS_IN);
}

static void mdio_bb_set_mdio(struct renesas_v3msk_sysreset_priv *priv, int val)
{
	dm_gpio_set_value(&priv->mosi, val);
}

static int mdio_bb_get_mdio(struct renesas_v3msk_sysreset_priv *priv)
{
	return dm_gpio_get_value(&priv->miso);
}

static void mdio_bb_set_mdc(struct renesas_v3msk_sysreset_priv *priv, int val)
{
	dm_gpio_set_value(&priv->mdc, val);
}

static void mdio_bb_delay(void)
{
	udelay(MDIO_DELAY);
}

/* Send the preamble, address, and register (common to read and write) */
static void mdio_bb_pre(struct renesas_v3msk_sysreset_priv *priv,
			u8 op, u8 addr, u8 reg)
{
	int i;

	/* 32-bit preamble */
	mdio_bb_active_mdio(priv);
	mdio_bb_set_mdio(priv, 1);
	for (i = 0; i < 32; i++) {
		mdio_bb_set_mdc(priv, 0);
		mdio_bb_delay();
		mdio_bb_set_mdc(priv, 1);
		mdio_bb_delay();
	}
	/* send the ST (2-bits of '01') */
	mdio_bb_set_mdio(priv, 0);
	mdio_bb_set_mdc(priv, 0);
	mdio_bb_delay();
	mdio_bb_set_mdc(priv, 1);
	mdio_bb_delay();
	mdio_bb_set_mdio(priv, 1);
	mdio_bb_set_mdc(priv, 0);
	mdio_bb_delay();
	mdio_bb_set_mdc(priv, 1);
	mdio_bb_delay();
	/* send the OP (2-bits of Opcode: '10'-read, '01'-write) */
	mdio_bb_set_mdio(priv, op >> 1);
	mdio_bb_set_mdc(priv, 0);
	mdio_bb_delay();
	mdio_bb_set_mdc(priv, 1);
	mdio_bb_delay();
	mdio_bb_set_mdio(priv, op & 1);
	mdio_bb_set_mdc(priv, 0);
	mdio_bb_delay();
	mdio_bb_set_mdc(priv, 1);
	mdio_bb_delay();
	/* send the PA5 (5-bits of PHY address) */
	for (i = 0; i < 5; i++) {
		mdio_bb_set_mdio(priv, addr & 0x10); /* MSB first */
		mdio_bb_set_mdc(priv, 0);
		mdio_bb_delay();
		mdio_bb_set_mdc(priv, 1);
		mdio_bb_delay();
		addr <<= 1;
	}
	/* send the RA5 (5-bits of register address) */
	for (i = 0; i < 5; i++) {
		mdio_bb_set_mdio(priv, reg & 0x10); /* MSB first */
		mdio_bb_set_mdc(priv, 0);
		mdio_bb_delay();
		mdio_bb_set_mdc(priv, 1);
		mdio_bb_delay();
		reg <<= 1;
	}
}

static int mdio_bb_read(struct renesas_v3msk_sysreset_priv *priv,
			u8 addr, u8 reg)
{
	int i;
	u16 data = 0;

	mdio_bb_pre(priv, 2, addr, reg);
	/* tri-state MDIO */
	mdio_bb_tristate_mdio(priv);
	/* read TA (2-bits of turn-around, last bit must be '0') */
	mdio_bb_set_mdc(priv, 0);
	mdio_bb_delay();
	mdio_bb_set_mdc(priv, 1);
	mdio_bb_delay();
	mdio_bb_set_mdc(priv, 0);
	mdio_bb_delay();
	mdio_bb_set_mdc(priv, 1);
	mdio_bb_delay();
	/* check the turnaround bit: the PHY should drive line to zero */
	if (mdio_bb_get_mdio(priv) != 0) {
		printf("PHY didn't drive TA low\n");
		for (i = 0; i < 32; i++) {
			mdio_bb_set_mdc(priv, 0);
			mdio_bb_delay();
			mdio_bb_set_mdc(priv, 1);
			mdio_bb_delay();
		}
		/* There is no PHY, set value to 0xFFFF */
		return 0xFFFF;
	}
	mdio_bb_set_mdc(priv, 0);
	mdio_bb_delay();
	/* read 16-bits of data */
	for (i = 0; i < 16; i++) {
		mdio_bb_set_mdc(priv, 1);
		mdio_bb_delay();
		data <<= 1;
		data |= mdio_bb_get_mdio(priv);
		mdio_bb_set_mdc(priv, 0);
		mdio_bb_delay();
	}

	mdio_bb_set_mdc(priv, 1);
	mdio_bb_delay();
	mdio_bb_set_mdc(priv, 0);
	mdio_bb_delay();
	mdio_bb_set_mdc(priv, 1);
	mdio_bb_delay();

	debug("cpld_read(0x%x) @ 0x%x = 0x%04x\n", reg, addr, data);

	return data;
}

static void mdio_bb_write(struct renesas_v3msk_sysreset_priv *priv,
			  u8 addr, u8 reg, u16 val)
{
	int i;

	mdio_bb_pre(priv, 1, addr, reg);
	/* send the TA (2-bits of turn-around '10') */
	mdio_bb_set_mdio(priv, 1);
	mdio_bb_set_mdc(priv, 0);
	mdio_bb_delay();
	mdio_bb_set_mdc(priv, 1);
	mdio_bb_delay();
	mdio_bb_set_mdio(priv, 0);
	mdio_bb_set_mdc(priv, 0);
	mdio_bb_delay();
	mdio_bb_set_mdc(priv, 1);
	mdio_bb_delay();
	/* write 16-bits of data */
	for (i = 0; i < 16; i++) {
		mdio_bb_set_mdio(priv, val & 0x8000); /* MSB first */
		mdio_bb_set_mdc(priv, 0);
		mdio_bb_delay();
		mdio_bb_set_mdc(priv, 1);
		mdio_bb_delay();
		val <<= 1;
	}
	/* tri-state MDIO */
	mdio_bb_tristate_mdio(priv);
	mdio_bb_set_mdc(priv, 0);
	mdio_bb_delay();
	mdio_bb_set_mdc(priv, 1);
	mdio_bb_delay();
}

static u16 cpld_read(struct udevice *dev, u16 addr)
{
	struct renesas_v3msk_sysreset_priv *priv = dev_get_priv(dev);

	/* random flash reads require 2 reads: first read is unreliable */
	if (addr >= CPLD_ADDR_PCB_VERSION)
		mdio_bb_read(priv, addr >> 5, addr & 0x1f);

	return mdio_bb_read(priv, addr >> 5, addr & 0x1f);
}

static void cpld_write(struct udevice *dev, u16 addr, u16 data)
{
	struct renesas_v3msk_sysreset_priv *priv = dev_get_priv(dev);

	mdio_bb_write(priv, addr >> 5, addr & 0x1f, data);
}

static int do_cpld(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	struct udevice *dev;
	u16 addr, val;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_SYSRESET,
					  DM_DRIVER_GET(sysreset_renesas_v3msk),
					  &dev);
	if (ret)
		return ret;

	if (argc == 2 && strcmp(argv[1], "info") == 0) {
		printf("Product:                0x%08x\n",
		       (cpld_read(dev, CPLD_ADDR_PRODUCT_H) << 16) |
			cpld_read(dev, CPLD_ADDR_PRODUCT_L));
		printf("CPLD version:           0x%08x\n",
		       (cpld_read(dev, CPLD_ADDR_CPLD_VERSION_Y) << 16) |
			cpld_read(dev, CPLD_ADDR_CPLD_VERSION_D));
		printf("Mode setting (MD0..26): 0x%08x\n",
		       (cpld_read(dev, CPLD_ADDR_MODE_APPLIED_H) << 16) |
			cpld_read(dev, CPLD_ADDR_MODE_APPLIED_L));
		printf("DIPSW (SW4, SW5):       0x%02x, 0x%x\n",
		       (cpld_read(dev, CPLD_ADDR_DIPSW) & 0xff) ^ 0xff,
		       (cpld_read(dev, CPLD_ADDR_DIPSW) >> 8) ^ 0xf);
		printf("Power config:           0x%08x\n",
		       cpld_read(dev, CPLD_ADDR_POWER_CFG));
		printf("Periferals config:      0x%08x\n",
		       (cpld_read(dev, CPLD_ADDR_PERI_CFG2) << 16) |
			cpld_read(dev, CPLD_ADDR_PERI_CFG1));
		printf("PCB version:            %d.%d\n",
		       cpld_read(dev, CPLD_ADDR_PCB_VERSION) >> 8,
		       cpld_read(dev, CPLD_ADDR_PCB_VERSION) & 0xff);
		printf("SOC version:            %d.%d\n",
		       cpld_read(dev, CPLD_ADDR_SOC_VERSION) >> 8,
		       cpld_read(dev, CPLD_ADDR_SOC_VERSION) & 0xff);
		printf("PCB S/N:                %d\n",
		       (cpld_read(dev, CPLD_ADDR_PCB_SN_H) << 16) |
			cpld_read(dev, CPLD_ADDR_PCB_SN_L));
		return 0;
	}

	if (argc < 3)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[2], NULL, 16);
	if (!(addr >= CPLD_ADDR_PRODUCT_L && addr <= CPLD_ADDR_LEDS)) {
		printf("cpld invalid addr\n");
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

U_BOOT_CMD(cpld, 4, 1, do_cpld,
	   "CPLD access",
	   "info\n"
	   "cpld read addr\n"
	   "cpld write addr val\n"
);

static int renesas_v3msk_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	cpld_write(dev, CPLD_ADDR_RESET, 1);

	return -EINPROGRESS;
}

static int renesas_v3msk_sysreset_probe(struct udevice *dev)
{
	struct renesas_v3msk_sysreset_priv *priv = dev_get_priv(dev);

	if (gpio_request_by_name(dev, "gpio-miso", 0, &priv->miso,
				 GPIOD_IS_IN))
		return -EINVAL;

	if (gpio_request_by_name(dev, "gpio-mosi", 0, &priv->mosi,
				 GPIOD_IS_OUT))
		return -EINVAL;

	if (gpio_request_by_name(dev, "gpio-mdc", 0, &priv->mdc,
				 GPIOD_IS_OUT))
		return -EINVAL;

	if (gpio_request_by_name(dev, "gpio-enablez", 0, &priv->enablez,
				 GPIOD_IS_OUT))
		return -EINVAL;

	/* V3MSK Videobox Mini board has CANFD PHY connected
	 * we must shutdown this chip to use bb pins
	 */
	gpio_request_list_by_name(dev, "gpios", priv->gpios, CPLD_MAX_GPIOS,
				  GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);

	return 0;
}

static struct sysreset_ops renesas_v3msk_sysreset = {
	.request	= renesas_v3msk_sysreset_request,
};

static const struct udevice_id renesas_v3msk_sysreset_ids[] = {
	{ .compatible = "renesas,v3msk-cpld" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sysreset_renesas_v3msk) = {
	.name		= "renesas_v3msk_sysreset",
	.id		= UCLASS_SYSRESET,
	.ops		= &renesas_v3msk_sysreset,
	.probe		= renesas_v3msk_sysreset_probe,
	.of_match	= renesas_v3msk_sysreset_ids,
	.priv_auto	= sizeof(struct renesas_v3msk_sysreset_priv),
};

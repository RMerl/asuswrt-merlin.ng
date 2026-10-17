// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2021 Marek Vasut <marek.vasut+renesas@gmail.com>
 */

#include <common.h>
#include <dm.h>
#include <i2c_eeprom.h>
#include <log.h>
#include <sysinfo.h>

#define BOARD_CODE_MASK		0xF8
#define BOARD_REV_MASK		0x07
#define BOARD_CODE_SHIFT	0x03

#define BOARD_SALVATOR_X	0x0
#define BOARD_KRIEK		0x1
#define BOARD_STARTER_KIT	0x2
#define BOARD_EAGLE		0x3
#define BOARD_SALVATOR_XS	0x4
#define BOARD_CONDOR		0x6
#define BOARD_DRAAK		0x7
#define BOARD_EBISU		0x8
#define BOARD_STARTER_KIT_PRE	0xB
#define BOARD_EBISU_4D		0xD
#define BOARD_CONDOR_I		0x10

/**
 * struct sysinfo_rcar_priv - sysinfo private data
 * @boardname: board model and revision
 * @val: board ID value from eeprom
 */
struct sysinfo_rcar_priv {
	char	boardmodel[64];
	u8	val;
};

static int sysinfo_rcar_detect(struct udevice *dev)
{
	struct sysinfo_rcar_priv *priv = dev_get_priv(dev);

	return priv->val == 0xff;
}

static int sysinfo_rcar_get_str(struct udevice *dev, int id, size_t size, char *val)
{
	struct sysinfo_rcar_priv *priv = dev_get_priv(dev);

	switch (id) {
	case SYSINFO_ID_BOARD_MODEL:
		strncpy(val, priv->boardmodel, size);
		val[size - 1] = '\0';
		return 0;
	default:
		return -EINVAL;
	};
}

static const struct sysinfo_ops sysinfo_rcar_ops = {
	.detect = sysinfo_rcar_detect,
	.get_str = sysinfo_rcar_get_str,
};

static void sysinfo_rcar_parse(struct sysinfo_rcar_priv *priv)
{
	const u8 board_id = (priv->val & BOARD_CODE_MASK) >> BOARD_CODE_SHIFT;
	const u8 board_rev = priv->val & BOARD_REV_MASK;
	bool salvator_xs = false;
	bool ebisu_4d = false;
	bool condor_i = false;
	char rev_major = '?';
	char rev_minor = '?';

	switch (board_id) {
	case BOARD_SALVATOR_XS:
		salvator_xs = true;
		fallthrough;
	case BOARD_SALVATOR_X:
		if (!(board_rev & ~1)) { /* Only rev 0 and 1 is valid */
			rev_major = '1';
			rev_minor = '0' + (board_rev & BIT(0));
		}
		snprintf(priv->boardmodel, sizeof(priv->boardmodel),
			 "Renesas Salvator-X%s board rev %c.%c",
			 salvator_xs ? "S" : "", rev_major, rev_minor);
		return;
	case BOARD_STARTER_KIT:
		if (!(board_rev & ~1)) { /* Only rev 0 and 1 is valid */
			rev_major = (board_rev & BIT(0)) ? '3' : '1';
			rev_minor = '0';
		}
		snprintf(priv->boardmodel, sizeof(priv->boardmodel),
			 "Renesas Starter Kit board rev %c.%c",
			 rev_major, rev_minor);
		return;
	case BOARD_STARTER_KIT_PRE:
		if (!(board_rev & ~3)) { /* Only rev 0..3 is valid */
			rev_major = (board_rev & BIT(1)) ? '2' : '1';
			rev_minor = (board_rev == 3) ? '1' : '0';
		}
		snprintf(priv->boardmodel, sizeof(priv->boardmodel),
			 "Renesas Starter Kit Premier board rev %c.%c",
			 rev_major, rev_minor);
		return;
	case BOARD_EAGLE:
		if (!board_rev) { /* Only rev 0 is valid */
			rev_major = '1';
			rev_minor = '0';
		}
		snprintf(priv->boardmodel, sizeof(priv->boardmodel),
			 "Renesas Eagle board rev %c.%c",
			 rev_major, rev_minor);
		return;
	case BOARD_EBISU_4D:
		ebisu_4d = true;
		fallthrough;
	case BOARD_EBISU:
		if (!board_rev) { /* Only rev 0 is valid */
			rev_major = '1';
			rev_minor = '0';
		}
		snprintf(priv->boardmodel, sizeof(priv->boardmodel),
			 "Renesas Ebisu%s board rev %c.%c",
			 ebisu_4d ? "-4D" : "", rev_major, rev_minor);
		return;
	case BOARD_DRAAK:
		if (!board_rev) { /* Only rev 0 is valid */
			rev_major = '1';
			rev_minor = '0';
		}
		snprintf(priv->boardmodel, sizeof(priv->boardmodel),
			 "Renesas Draak board rev %c.%c",
			 rev_major, rev_minor);
		return;
	case BOARD_KRIEK:
		if (!board_rev) { /* Only rev 0 is valid */
			rev_major = '1';
			rev_minor = '0';
		}
		snprintf(priv->boardmodel, sizeof(priv->boardmodel),
			 "Renesas Kriek board rev %c.%c",
			 rev_major, rev_minor);
		return;
	case BOARD_CONDOR_I:
		condor_i = true;
		fallthrough;
	case BOARD_CONDOR:
		if (!board_rev) { /* Only rev 0 is valid */
			rev_major = '1';
			rev_minor = '0';
		}
		snprintf(priv->boardmodel, sizeof(priv->boardmodel),
			"Renesas Condor%s board rev %c.%c",
			condor_i ? "-I" : "", rev_major, rev_minor);
		return;
	default:
		snprintf(priv->boardmodel, sizeof(priv->boardmodel),
			 "Renesas -Unknown- board rev ?.?");
		priv->val = 0xff;
		return;
	}
}

static int sysinfo_rcar_probe(struct udevice *dev)
{
	struct sysinfo_rcar_priv *priv = dev_get_priv(dev);
	struct ofnode_phandle_args phandle_args;
	struct udevice *i2c_eeprom;
	u32 offset;
	int ret;

	offset = dev_read_u32_default(dev, "offset", 0x70);

	ret = dev_read_phandle_with_args(dev, "i2c-eeprom", NULL,
					 0, 0, &phandle_args);
	if (ret) {
		debug("%s: i2c-eeprom backing device not specified\n",
		      dev->name);
		return ret;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_I2C_EEPROM, phandle_args.node,
					  &i2c_eeprom);
	if (ret) {
		debug("%s: could not get backing device\n", dev->name);
		return ret;
	}

	ret = i2c_eeprom_read(i2c_eeprom, offset, &priv->val, 1);
	if (ret < 0) {
		debug("%s: read failed\n", __func__);
		return -EIO;
	}

	sysinfo_rcar_parse(priv);

	return 0;
}

static const struct udevice_id sysinfo_rcar_ids[] = {
	{ .compatible = "renesas,rcar-sysinfo" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sysinfo_rcar) = {
	.name           = "sysinfo_rcar",
	.id             = UCLASS_SYSINFO,
	.of_match       = sysinfo_rcar_ids,
	.ops		= &sysinfo_rcar_ops,
	.priv_auto	= sizeof(struct sysinfo_rcar_priv),
	.probe          = sysinfo_rcar_probe,
};

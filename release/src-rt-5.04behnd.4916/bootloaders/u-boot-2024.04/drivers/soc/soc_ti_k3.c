// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 Texas Instruments Incorporated - https://www.ti.com/
 *	Dave Gerlach <d-gerlach@ti.com>
 */

#include <common.h>
#include <dm.h>
#include <soc.h>

#include <asm/arch/hardware.h>
#include <asm/io.h>

struct soc_ti_k3_plat {
	const char *family;
	const char *revision;
};

static const char *get_family_string(u32 idreg)
{
	const char *family;
	u32 soc;

	soc = (idreg & JTAG_ID_PARTNO_MASK) >> JTAG_ID_PARTNO_SHIFT;

	switch (soc) {
	case JTAG_ID_PARTNO_AM65X:
		family = "AM65X";
		break;
	case JTAG_ID_PARTNO_J721E:
		family = "J721E";
		break;
	case JTAG_ID_PARTNO_J7200:
		family = "J7200";
		break;
	case JTAG_ID_PARTNO_AM64X:
		family = "AM64X";
		break;
	case JTAG_ID_PARTNO_J721S2:
		family = "J721S2";
		break;
	case JTAG_ID_PARTNO_AM62X:
		family = "AM62X";
		break;
	case JTAG_ID_PARTNO_AM62AX:
		family = "AM62AX";
		break;
	default:
		family = "Unknown Silicon";
	};

	return family;
}

static char *j721e_rev_string_map[] = {
	"1.0", "1.1",
};

static char *typical_rev_string_map[] = {
	"1.0", "2.0", "3.0",
};

static const char *get_rev_string(u32 idreg)
{
	u32 rev;
	u32 soc;

	rev = (idreg & JTAG_ID_VARIANT_MASK) >> JTAG_ID_VARIANT_SHIFT;
	soc = (idreg & JTAG_ID_PARTNO_MASK) >> JTAG_ID_PARTNO_SHIFT;

	switch (soc) {
	case JTAG_ID_PARTNO_J721E:
		if (rev >= ARRAY_SIZE(j721e_rev_string_map))
			goto bail;
		return j721e_rev_string_map[rev];

	default:
		if (rev >= ARRAY_SIZE(typical_rev_string_map))
			goto bail;
		return typical_rev_string_map[rev];
	};

bail:
	return "Unknown Revision";
}

static int soc_ti_k3_get_family(struct udevice *dev, char *buf, int size)
{
	struct soc_ti_k3_plat *plat = dev_get_plat(dev);

	snprintf(buf, size, "%s", plat->family);

	return 0;
}

static int soc_ti_k3_get_revision(struct udevice *dev, char *buf, int size)
{
	struct soc_ti_k3_plat *plat = dev_get_plat(dev);

	snprintf(buf, size, "SR%s", plat->revision);

	return 0;
}

static const struct soc_ops soc_ti_k3_ops = {
	.get_family = soc_ti_k3_get_family,
	.get_revision = soc_ti_k3_get_revision,
};

int soc_ti_k3_probe(struct udevice *dev)
{
	struct soc_ti_k3_plat *plat = dev_get_plat(dev);
	u32 idreg;
	void *idreg_addr;

	idreg_addr = dev_read_addr_ptr(dev);
	if (!idreg_addr)
		return -EINVAL;

	idreg = readl(idreg_addr);

	plat->family = get_family_string(idreg);
	plat->revision = get_rev_string(idreg);

	return 0;
}

static const struct udevice_id soc_ti_k3_ids[] = {
	{ .compatible = "ti,am654-chipid" },
	{ }
};

U_BOOT_DRIVER(soc_ti_k3) = {
	.name           = "soc_ti_k3",
	.id             = UCLASS_SOC,
	.ops		= &soc_ti_k3_ops,
	.of_match       = soc_ti_k3_ids,
	.probe          = soc_ti_k3_probe,
	.plat_auto	= sizeof(struct soc_ti_k3_plat),
};

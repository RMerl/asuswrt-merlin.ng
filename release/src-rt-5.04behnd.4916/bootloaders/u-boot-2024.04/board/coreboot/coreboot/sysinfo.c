// SPDX-License-Identifier: GPL-2.0+
/*
 * coreboot sysinfo driver
 *
 * Copyright 2023 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <dm.h>
#include <smbios.h>
#include <sysinfo.h>
#include <asm/cb_sysinfo.h>

struct cb_sysinfo_priv {
	const struct smbios_header *bios;
	const struct smbios_header *system;
	const struct smbios_type0 *t0;
	const struct smbios_type1 *t1;
};

static int cb_get_str(struct udevice *dev, int id, size_t size, char *val)
{
	struct cb_sysinfo_priv *priv = dev_get_priv(dev);
	const char *str = NULL;

	switch (id) {
	case SYSINFO_ID_BOARD_MODEL:
		if (priv->t1)
			str = smbios_string(priv->system,
					    priv->t1->product_name);
		break;
	case SYSINFO_ID_BOARD_MANUFACTURER:
		if (priv->t1)
			str = smbios_string(priv->system,
					    priv->t1->manufacturer);
		break;
	case SYSINFO_ID_PRIOR_STAGE_VERSION:
		if (priv->t0)
			str = smbios_string(priv->bios, priv->t0->bios_ver);
		break;
	case SYSINFO_ID_PRIOR_STAGE_DATE:
		if (priv->t0)
			str = smbios_string(priv->bios,
					    priv->t0->bios_release_date);
		break;
	}
	if (!str)
		return -ENOTSUPP;

	strlcpy(val, str, size);

	return  0;
}

static int cb_detect(struct udevice *dev)
{
	struct cb_sysinfo_priv *priv = dev_get_priv(dev);
	const struct smbios_entry *smbios;

	smbios = smbios_entry(lib_sysinfo.smbios_start,
			      lib_sysinfo.smbios_size);
	if (!smbios)
		return 0;

	priv->bios = smbios_header(smbios, SMBIOS_BIOS_INFORMATION);
	priv->system = smbios_header(smbios, SMBIOS_SYSTEM_INFORMATION);
	priv->t0 = (struct smbios_type0 *)priv->bios;
	priv->t1 = (struct smbios_type1 *)priv->system;

	return 0;
}

static const struct udevice_id sysinfo_coreboot_ids[] = {
	{ .compatible = "coreboot,sysinfo" },
	{ /* sentinel */ }
};

static const struct sysinfo_ops sysinfo_coreboot_ops = {
	.detect		= cb_detect,
	.get_str	= cb_get_str,
};

U_BOOT_DRIVER(sysinfo_coreboot) = {
	.name           = "sysinfo_coreboot",
	.id             = UCLASS_SYSINFO,
	.of_match       = sysinfo_coreboot_ids,
	.ops		= &sysinfo_coreboot_ops,
	.priv_auto	= sizeof(struct cb_sysinfo_priv),
};

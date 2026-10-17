// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx Versal SOC driver
 *
 * Copyright (C) 2021 Xilinx, Inc.
 */

#include <common.h>
#include <dm.h>
#include <soc.h>
#include <zynqmp_firmware.h>
#include <asm/io.h>
#include <asm/arch/hardware.h>

/*
 * v1 -> 0x10 - ES1
 * v2 -> 0x20 - Production
 */
static const char versal_family[] = "Versal";

struct soc_xilinx_versal_priv {
	const char *family;
	char revision;
};

static int soc_xilinx_versal_get_family(struct udevice *dev, char *buf, int size)
{
	struct soc_xilinx_versal_priv *priv = dev_get_priv(dev);

	return snprintf(buf, size, "%s", priv->family);
}

static int soc_xilinx_versal_get_revision(struct udevice *dev, char *buf, int size)
{
	struct soc_xilinx_versal_priv *priv = dev_get_priv(dev);

	return snprintf(buf, size, "v%d", priv->revision);
}

static const struct soc_ops soc_xilinx_versal_ops = {
	.get_family = soc_xilinx_versal_get_family,
	.get_revision = soc_xilinx_versal_get_revision,
};

static int soc_xilinx_versal_probe(struct udevice *dev)
{
	struct soc_xilinx_versal_priv *priv = dev_get_priv(dev);
	u32 ret_payload[PAYLOAD_ARG_CNT];
	int ret;

	priv->family = versal_family;

	if (IS_ENABLED(CONFIG_ZYNQMP_FIRMWARE)) {
		ret = xilinx_pm_request(PM_GET_CHIPID, 0, 0, 0, 0,
					ret_payload);
		if (ret)
			return ret;
	} else {
		ret_payload[2] = readl(VERSAL_PS_PMC_VERSION);
		if (!ret_payload[2])
			return -EINVAL;
	}

	priv->revision = ret_payload[2] >> VERSAL_PS_VER_SHIFT;

	return 0;
}

U_BOOT_DRIVER(soc_xilinx_versal) = {
	.name		= "soc_xilinx_versal",
	.id		= UCLASS_SOC,
	.ops		= &soc_xilinx_versal_ops,
	.probe		= soc_xilinx_versal_probe,
	.priv_auto	= sizeof(struct soc_xilinx_versal_priv),
	.flags		= DM_FLAG_PRE_RELOC,
};

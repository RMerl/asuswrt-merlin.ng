// SPDX-License-Identifier: GPL-2.0
/*
 * Xilinx ZynqMP SOC driver
 *
 * Copyright (C) 2021 Xilinx, Inc.
 * Michal Simek <michal.simek@amd.com>
 *
 * Copyright (C) 2022 Weidmüller Interface GmbH & Co. KG
 * Stefan Herbrechtsmeier <stefan.herbrechtsmeier@weidmueller.com>
 */

#include <common.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <asm/cache.h>
#include <soc.h>
#include <zynqmp_firmware.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/hardware.h>

/*
 * Zynqmp has 4 silicon revisions
 * v0 -> 0(XCZU9EG-ES1)
 * v1 -> 1(XCZU3EG-ES1, XCZU15EG-ES1)
 * v2 -> 2(XCZU7EV-ES1, XCZU9EG-ES2, XCZU19EG-ES1)
 * v3 -> 3(Production Level)
 */
static const char zynqmp_family[] = "ZynqMP";

#define EFUSE_VCU_DIS_SHIFT	8
#define EFUSE_VCU_DIS_MASK	BIT(EFUSE_VCU_DIS_SHIFT)
#define EFUSE_GPU_DIS_SHIFT	5
#define EFUSE_GPU_DIS_MASK	BIT(EFUSE_GPU_DIS_SHIFT)
#define IDCODE_DEV_TYPE_MASK	GENMASK(27, 0)
#define IDCODE2_PL_INIT_SHIFT	9
#define IDCODE2_PL_INIT_MASK	BIT(IDCODE2_PL_INIT_SHIFT)

#define ZYNQMP_VERSION_SIZE	10

enum {
	ZYNQMP_VARIANT_EG = BIT(0),
	ZYNQMP_VARIANT_EV = BIT(1),
	ZYNQMP_VARIANT_CG = BIT(2),
	ZYNQMP_VARIANT_DR = BIT(3),
	ZYNQMP_VARIANT_DR_SE = BIT(4),
	ZYNQMP_VARIANT_EG_SE = BIT(5),
};

struct zynqmp_device {
	u32 id;
	u8 device;
	u8 variants;
};

struct soc_xilinx_zynqmp_priv {
	const char *family;
	char machine[ZYNQMP_VERSION_SIZE];
	char revision;
};

static const struct zynqmp_device zynqmp_devices[] = {
	{
		.id = 0x04688093,
		.device = 1,
		.variants = ZYNQMP_VARIANT_EG,
	},
	{
		.id = 0x04711093,
		.device = 2,
		.variants = ZYNQMP_VARIANT_EG | ZYNQMP_VARIANT_CG,
	},
	{
		.id = 0x04710093,
		.device = 3,
		.variants = ZYNQMP_VARIANT_EG | ZYNQMP_VARIANT_CG,
	},
	{
		.id = 0x04721093,
		.device = 4,
		.variants = ZYNQMP_VARIANT_EG | ZYNQMP_VARIANT_CG |
			ZYNQMP_VARIANT_EV,
	},
	{
		.id = 0x04720093,
		.device = 5,
		.variants = ZYNQMP_VARIANT_EG | ZYNQMP_VARIANT_CG |
			ZYNQMP_VARIANT_EV,
	},
	{
		.id = 0x04739093,
		.device = 6,
		.variants = ZYNQMP_VARIANT_EG | ZYNQMP_VARIANT_CG,
	},
	{
		.id = 0x04730093,
		.device = 7,
		.variants = ZYNQMP_VARIANT_EG | ZYNQMP_VARIANT_CG |
			ZYNQMP_VARIANT_EV,
	},
	{
		.id = 0x04738093,
		.device = 9,
		.variants = ZYNQMP_VARIANT_EG | ZYNQMP_VARIANT_CG,
	},
	{
		.id = 0x04740093,
		.device = 11,
		.variants = ZYNQMP_VARIANT_EG,
	},
	{
		.id = 0x04741093,
		.device = 11,
		.variants = ZYNQMP_VARIANT_EG_SE,
	},
	{
		.id = 0x04750093,
		.device = 15,
		.variants = ZYNQMP_VARIANT_EG,
	},
	{
		.id = 0x04759093,
		.device = 17,
		.variants = ZYNQMP_VARIANT_EG,
	},
	{
		.id = 0x04758093,
		.device = 19,
		.variants = ZYNQMP_VARIANT_EG,
	},
	{
		.id = 0x0475C093,
		.device = 19,
		.variants = ZYNQMP_VARIANT_EG_SE,
	},
	{
		.id = 0x047E1093,
		.device = 21,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047E3093,
		.device = 23,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047E5093,
		.device = 25,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047E4093,
		.device = 27,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047E0093,
		.device = 28,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047E2093,
		.device = 29,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047E6093,
		.device = 39,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047FD093,
		.device = 43,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047F8093,
		.device = 46,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047FF093,
		.device = 47,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047FA093,
		.device = 47,
		.variants = ZYNQMP_VARIANT_DR_SE,
	},
	{
		.id = 0x047FB093,
		.device = 48,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x047FE093,
		.device = 49,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x046d0093,
		.device = 67,
		.variants = ZYNQMP_VARIANT_DR,
	},
	{
		.id = 0x046d7093,
		.device = 67,
		.variants = ZYNQMP_VARIANT_DR_SE,
	},
	{
		.id = 0x04712093,
		.device = 24,
		.variants = 0,
	},
	{
		.id = 0x04724093,
		.device = 26,
		.variants = 0,
	},
};

static const struct zynqmp_device *zynqmp_get_device(u32 idcode)
{
	idcode &= IDCODE_DEV_TYPE_MASK;

	for (int i = 0; i < ARRAY_SIZE(zynqmp_devices); i++) {
		if (zynqmp_devices[i].id == idcode)
			return &zynqmp_devices[i];
	}

	return NULL;
}

static int soc_xilinx_zynqmp_detect_machine(struct udevice *dev, u32 idcode,
					    u32 idcode2)
{
	struct soc_xilinx_zynqmp_priv *priv = dev_get_priv(dev);
	const struct zynqmp_device *device;
	int ret;

	device = zynqmp_get_device(idcode);
	if (!device)
		return 0;

	/* Add device prefix to the name */
	ret = snprintf(priv->machine, sizeof(priv->machine), "%s%d",
		       device->variants ? "zu" : "xck", device->device);
	if (ret < 0)
		return ret;

	if (device->variants & ZYNQMP_VARIANT_EV) {
		/* Devices with EV variant might be EG/CG/EV family */
		if (idcode2 & IDCODE2_PL_INIT_MASK) {
			u32 family = ((idcode2 & EFUSE_VCU_DIS_MASK) >>
				      EFUSE_VCU_DIS_SHIFT) << 1 |
				     ((idcode2 & EFUSE_GPU_DIS_MASK) >>
				      EFUSE_GPU_DIS_SHIFT);

			/*
			 * Get family name based on extended idcode values as
			 * determined on UG1087, EXTENDED_IDCODE register
			 * description
			 */
			switch (family) {
			case 0x00:
				strlcat(priv->machine, "ev",
					sizeof(priv->machine));
				break;
			case 0x10:
				strlcat(priv->machine, "eg",
					sizeof(priv->machine));
				break;
			case 0x11:
				strlcat(priv->machine, "cg",
					sizeof(priv->machine));
				break;
			default:
				/* Do not append family name*/
				break;
			}
		} else {
			/*
			 * When PL powered down the VCU Disable efuse cannot be
			 * read. So, ignore the bit and just findout if it is CG
			 * or EG/EV variant.
			 */
			strlcat(priv->machine, (idcode2 & EFUSE_GPU_DIS_MASK) ?
				"cg" : "e", sizeof(priv->machine));
		}
	} else if (device->variants & ZYNQMP_VARIANT_CG) {
		/* Devices with CG variant might be EG or CG family */
		strlcat(priv->machine, (idcode2 & EFUSE_GPU_DIS_MASK) ?
			"cg" : "eg", sizeof(priv->machine));
	} else if (device->variants & ZYNQMP_VARIANT_EG) {
		strlcat(priv->machine, "eg", sizeof(priv->machine));
	} else if (device->variants & ZYNQMP_VARIANT_EG_SE) {
		strlcat(priv->machine, "eg_SE", sizeof(priv->machine));
	} else if (device->variants & ZYNQMP_VARIANT_DR) {
		strlcat(priv->machine, "dr", sizeof(priv->machine));
	} else if (device->variants & ZYNQMP_VARIANT_DR_SE) {
		strlcat(priv->machine, "dr_SE", sizeof(priv->machine));
	}

	return 0;
}

static int soc_xilinx_zynqmp_get_family(struct udevice *dev, char *buf, int size)
{
	struct soc_xilinx_zynqmp_priv *priv = dev_get_priv(dev);

	return snprintf(buf, size, "%s", priv->family);
}

static int soc_xilinx_zynqmp_get_machine(struct udevice *dev, char *buf, int size)
{
	struct soc_xilinx_zynqmp_priv *priv = dev_get_priv(dev);
	const char *machine = priv->machine;

	if (!machine[0])
		machine = "unknown";

	return snprintf(buf, size, "%s", machine);
}

static int soc_xilinx_zynqmp_get_revision(struct udevice *dev, char *buf, int size)
{
	struct soc_xilinx_zynqmp_priv *priv = dev_get_priv(dev);

	return snprintf(buf, size, "v%d", priv->revision);
}

static const struct soc_ops soc_xilinx_zynqmp_ops = {
	.get_family = soc_xilinx_zynqmp_get_family,
	.get_revision = soc_xilinx_zynqmp_get_revision,
	.get_machine = soc_xilinx_zynqmp_get_machine,
};

static int soc_xilinx_zynqmp_probe(struct udevice *dev)
{
	struct soc_xilinx_zynqmp_priv *priv = dev_get_priv(dev);
	u32 ret_payload[PAYLOAD_ARG_CNT];
	int ret;

	priv->family = zynqmp_family;

	if (!IS_ENABLED(CONFIG_ZYNQMP_FIRMWARE))
		ret = zynqmp_mmio_read(ZYNQMP_PS_VERSION, &ret_payload[2]);
	else
		ret = xilinx_pm_request(PM_GET_CHIPID, 0, 0, 0, 0,
					ret_payload);
	if (ret < 0)
		return ret;

	priv->revision = ret_payload[2] & ZYNQMP_PS_VER_MASK;

	if (IS_ENABLED(CONFIG_ZYNQMP_FIRMWARE)) {
		/*
		 * Firmware returns:
		 * payload[0][31:0] = status of the operation
		 * payload[1] = IDCODE
		 * payload[2][19:0] = Version
		 * payload[2][28:20] = EXTENDED_IDCODE
		 * payload[2][29] = PL_INIT
		 */
		u32 idcode = ret_payload[1];
		u32 idcode2 = ret_payload[2] >>
				   ZYNQMP_CSU_VERSION_EMPTY_SHIFT;
		dev_dbg(dev, "IDCODE: 0x%0x, IDCODE2: 0x%0x\n", idcode,
			idcode2);

		ret = soc_xilinx_zynqmp_detect_machine(dev, idcode, idcode2);
		if (ret)
			return ret;
	}

	return 0;
}

U_BOOT_DRIVER(soc_xilinx_zynqmp) = {
	.name		= "soc_xilinx_zynqmp",
	.id		= UCLASS_SOC,
	.ops		= &soc_xilinx_zynqmp_ops,
	.probe		= soc_xilinx_zynqmp_probe,
	.priv_auto	= sizeof(struct soc_xilinx_zynqmp_priv),
	.flags		= DM_FLAG_PRE_RELOC,
};

// SPDX-License-Identifier: GPL-2.0+
/*
 * Host interface (LPC or eSPI) configuration on Nuvoton BMC
 * Copyright (c) 2022 Nuvoton Technology Corp.
 */

#include <common.h>
#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>

#define SMC_CTL_REG_ADDR	0xc0001001
#define SMC_CTL_HOSTWAIT	0x80

/* GCR Register Offsets */
#define HIFCR			0x50
#define MFSEL1			0x260
#define MFSEL4			0x26c

/* ESPI Register offsets */
#define ESPICFG			0x4
#define ESPIHINDP		0x80

/* MFSEL bit fileds */
#define MFSEL1_LPCSEL		BIT(26)
#define MFSEL4_ESPISEL		BIT(8)

/* ESPICFG bit fileds */
#define CHSUPP_MASK		GENMASK(27, 24)
#define IOMODE_MASK		GENMASK(9, 8)
#define IOMODE_SDQ		FIELD_PREP(IOMODE_MASK, 3)
#define MAXFREQ_MASK		GENMASK(12, 10)
#define MAXFREQ_33MHZ		FIELD_PREP(MAXFREQ_MASK, 2)

/* ESPIHINDP bit fileds */
#define AUTO_SBLD		BIT(4)
#define AUTO_HS1		BIT(8)
#define AUTO_HS2		BIT(12)
#define AUTO_HS3		BIT(16)

static int npcm_host_intf_bind(struct udevice *dev)
{
	struct regmap *syscon;
	void __iomem *base;
	u32 ch_supp, val;
	u32 ioaddr;
	const char *type;
	int ret;

	syscon = syscon_regmap_lookup_by_phandle(dev, "syscon");
	if (IS_ERR(syscon)) {
		dev_err(dev, "%s: unable to get syscon, dev %s\n", __func__, dev->name);
		return PTR_ERR(syscon);
	}

	ioaddr  = dev_read_u32_default(dev, "ioaddr", 0);
	if (ioaddr)
		regmap_write(syscon, HIFCR, ioaddr);

	type = dev_read_string(dev, "type");
	if (!type)
		return -EINVAL;

	if (!strcmp(type, "espi")) {
		base = dev_read_addr_ptr(dev);
		if (!base)
			return -EINVAL;

		ret = dev_read_u32(dev, "channel-support", &ch_supp);
		if (ret)
			return ret;

		/* Select eSPI pins function */
		regmap_update_bits(syscon, MFSEL1, MFSEL1_LPCSEL, 0);
		regmap_update_bits(syscon, MFSEL4, MFSEL4_ESPISEL, MFSEL4_ESPISEL);

		val = AUTO_SBLD | AUTO_HS1 | AUTO_HS2 | AUTO_HS3 | ch_supp;
		writel(val, base + ESPIHINDP);

		val = readl(base + ESPICFG);
		val &= ~(CHSUPP_MASK | IOMODE_MASK | MAXFREQ_MASK);
		val |= IOMODE_SDQ | MAXFREQ_33MHZ | FIELD_PREP(CHSUPP_MASK, ch_supp);
		writel(val, base + ESPICFG);
	} else if (!strcmp(type, "lpc")) {
		/* Select LPC pin function */
		regmap_update_bits(syscon, MFSEL4, MFSEL4_ESPISEL, 0);
		regmap_update_bits(syscon, MFSEL1, MFSEL1_LPCSEL, MFSEL1_LPCSEL);
	}

	/* Release host wait */
	setbits_8(SMC_CTL_REG_ADDR, SMC_CTL_HOSTWAIT);

	return 0;
}

static const struct udevice_id npcm_hostintf_ids[] = {
	{ .compatible = "nuvoton,npcm750-host-intf" },
	{ .compatible = "nuvoton,npcm845-host-intf" },
	{ }
};

U_BOOT_DRIVER(npcm_host_intf) = {
	.name	= "npcm_host_intf",
	.id	= UCLASS_MISC,
	.of_match = npcm_hostintf_ids,
	.bind = npcm_host_intf_bind,
};

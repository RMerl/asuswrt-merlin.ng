// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2017 NXP
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <malloc.h>
#include <power-domain-uclass.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/mach-imx/sys_proto.h>
#include <dm/device-internal.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <imx_sip.h>
#include <linux/bitmap.h>
#include <wait_bit.h>

#include <dt-bindings/power/imx8mm-power.h>
#include <dt-bindings/power/imx8mn-power.h>
#include <dt-bindings/power/imx8mp-power.h>
#include <dt-bindings/power/imx8mq-power.h>

DECLARE_GLOBAL_DATA_PTR;

#define GPC_PGC_CPU_MAPPING			0x0ec
#define IMX8MP_GPC_PGC_CPU_MAPPING		0x1cc

#define IMX8M_PCIE2_A53_DOMAIN			BIT(15)
#define IMX8M_OTG2_A53_DOMAIN			BIT(5)
#define IMX8M_OTG1_A53_DOMAIN			BIT(4)
#define IMX8M_PCIE1_A53_DOMAIN			BIT(3)

#define IMX8MM_OTG2_A53_DOMAIN			BIT(5)
#define IMX8MM_OTG1_A53_DOMAIN			BIT(4)
#define IMX8MM_PCIE_A53_DOMAIN			BIT(3)

#define IMX8MN_OTG1_A53_DOMAIN			BIT(4)
#define IMX8MN_MIPI_A53_DOMAIN			BIT(2)

#define IMX8MP_HSIOMIX_A53_DOMAIN		BIT(19)
#define IMX8MP_USB2_PHY_A53_DOMAIN		BIT(5)
#define IMX8MP_USB1_PHY_A53_DOMAIN		BIT(4)
#define IMX8MP_PCIE_PHY_A53_DOMAIN		BIT(3)

#define IMX8MP_GPC_PU_PGC_SW_PUP_REQ		0x0d8
#define IMX8MP_GPC_PU_PGC_SW_PDN_REQ		0x0e4

#define GPC_PU_PGC_SW_PUP_REQ			0x0f8
#define GPC_PU_PGC_SW_PDN_REQ			0x104

#define IMX8M_PCIE2_SW_Pxx_REQ			BIT(13)
#define IMX8M_OTG2_SW_Pxx_REQ			BIT(3)
#define IMX8M_OTG1_SW_Pxx_REQ			BIT(2)
#define IMX8M_PCIE1_SW_Pxx_REQ			BIT(1)

#define IMX8MM_OTG2_SW_Pxx_REQ			BIT(3)
#define IMX8MM_OTG1_SW_Pxx_REQ			BIT(2)
#define IMX8MM_PCIE_SW_Pxx_REQ			BIT(1)

#define IMX8MN_OTG1_SW_Pxx_REQ			BIT(2)
#define IMX8MN_MIPI_SW_Pxx_REQ			BIT(0)

#define IMX8MP_HSIOMIX_Pxx_REQ			BIT(17)
#define IMX8MP_USB2_PHY_Pxx_REQ			BIT(3)
#define IMX8MP_USB1_PHY_Pxx_REQ			BIT(2)
#define IMX8MP_PCIE_PHY_SW_Pxx_REQ		BIT(1)

#define GPC_M4_PU_PDN_FLG			0x1bc

#define IMX8MP_GPC_PU_PWRHSK			0x190
#define GPC_PU_PWRHSK				0x1fc

#define IMX8MM_HSIO_HSK_PWRDNACKN		(BIT(23) | BIT(24))
#define IMX8MM_HSIO_HSK_PWRDNREQN		(BIT(5) | BIT(6))

#define IMX8MN_HSIO_HSK_PWRDNACKN		BIT(23)
#define IMX8MN_HSIO_HSK_PWRDNREQN		BIT(5)

#define IMX8MP_HSIOMIX_PWRDNACKN		BIT(28)
#define IMX8MP_HSIOMIX_PWRDNREQN		BIT(12)

/*
 * The PGC offset values in Reference Manual
 * (Rev. 1, 01/2018 and the older ones) GPC chapter's
 * GPC_PGC memory map are incorrect, below offset
 * values are from design RTL.
 */
#define IMX8M_PGC_PCIE1			17
#define IMX8M_PGC_OTG1			18
#define IMX8M_PGC_OTG2			19
#define IMX8M_PGC_PCIE2			29

#define IMX8MM_PGC_PCIE			17
#define IMX8MM_PGC_OTG1			18
#define IMX8MM_PGC_OTG2			19

#define IMX8MN_PGC_OTG1			18

#define IMX8MP_PGC_PCIE			13
#define IMX8MP_PGC_USB1			14
#define IMX8MP_PGC_USB2			15
#define IMX8MP_PGC_HSIOMIX		29

#define GPC_PGC_CTRL(n)			(0x800 + (n) * 0x40)
#define GPC_PGC_SR(n)			(GPC_PGC_CTRL(n) + 0xc)

#define GPC_PGC_CTRL_PCR		BIT(0)

struct imx_pgc_regs {
	u16 map;
	u16 pup;
	u16 pdn;
	u16 hsk;
};

struct imx_pgc_domain {
	unsigned long pgc;

	const struct {
		u32 pxx;
		u32 map;
		u32 hskreq;
		u32 hskack;
	} bits;

	const bool keep_clocks;
};

struct imx_pgc_domain_data {
	const struct imx_pgc_domain *domains;
	size_t domains_num;
	const struct imx_pgc_regs *pgc_regs;
};

struct imx8m_power_domain_plat {
	struct power_domain pd;
	const struct imx_pgc_domain *domain;
	const struct imx_pgc_regs *regs;
	struct clk_bulk clk;
	void __iomem *base;
	int resource_id;
	int has_pd;
};

#if defined(CONFIG_IMX8MM) || defined(CONFIG_IMX8MN) || defined(CONFIG_IMX8MQ)
static const struct imx_pgc_regs imx7_pgc_regs = {
	.map = GPC_PGC_CPU_MAPPING,
	.pup = GPC_PU_PGC_SW_PUP_REQ,
	.pdn = GPC_PU_PGC_SW_PDN_REQ,
	.hsk = GPC_PU_PWRHSK,
};
#endif

#ifdef CONFIG_IMX8MQ
static const struct imx_pgc_domain imx8m_pgc_domains[] = {
	[IMX8M_POWER_DOMAIN_PCIE1] = {
		.bits  = {
			.pxx = IMX8M_PCIE1_SW_Pxx_REQ,
			.map = IMX8M_PCIE1_A53_DOMAIN,
		},
		.pgc   = BIT(IMX8M_PGC_PCIE1),
	},

	[IMX8M_POWER_DOMAIN_USB_OTG1] = {
		.bits  = {
			.pxx = IMX8M_OTG1_SW_Pxx_REQ,
			.map = IMX8M_OTG1_A53_DOMAIN,
		},
		.pgc   = BIT(IMX8M_PGC_OTG1),
	},

	[IMX8M_POWER_DOMAIN_USB_OTG2] = {
		.bits  = {
			.pxx = IMX8M_OTG2_SW_Pxx_REQ,
			.map = IMX8M_OTG2_A53_DOMAIN,
		},
		.pgc   = BIT(IMX8M_PGC_OTG2),
	},

	[IMX8M_POWER_DOMAIN_PCIE2] = {
		.bits  = {
			.pxx = IMX8M_PCIE2_SW_Pxx_REQ,
			.map = IMX8M_PCIE2_A53_DOMAIN,
		},
		.pgc   = BIT(IMX8M_PGC_PCIE2),
	},
};

static const struct imx_pgc_domain_data imx8m_pgc_domain_data = {
	.domains = imx8m_pgc_domains,
	.domains_num = ARRAY_SIZE(imx8m_pgc_domains),
	.pgc_regs = &imx7_pgc_regs,
};
#endif

#ifdef CONFIG_IMX8MM
static const struct imx_pgc_domain imx8mm_pgc_domains[] = {
	[IMX8MM_POWER_DOMAIN_HSIOMIX] = {
		.bits  = {
			.pxx = 0, /* no power sequence control */
			.map = 0, /* no power sequence control */
			.hskreq = IMX8MM_HSIO_HSK_PWRDNREQN,
			.hskack = IMX8MM_HSIO_HSK_PWRDNACKN,
		},
		.keep_clocks = true,
	},

	[IMX8MM_POWER_DOMAIN_PCIE] = {
		.bits  = {
			.pxx = IMX8MM_PCIE_SW_Pxx_REQ,
			.map = IMX8MM_PCIE_A53_DOMAIN,
		},
		.pgc   = BIT(IMX8MM_PGC_PCIE),
	},

	[IMX8MM_POWER_DOMAIN_OTG1] = {
		.bits  = {
			.pxx = IMX8MM_OTG1_SW_Pxx_REQ,
			.map = IMX8MM_OTG1_A53_DOMAIN,
		},
		.pgc   = BIT(IMX8MM_PGC_OTG1),
	},

	[IMX8MM_POWER_DOMAIN_OTG2] = {
		.bits  = {
			.pxx = IMX8MM_OTG2_SW_Pxx_REQ,
			.map = IMX8MM_OTG2_A53_DOMAIN,
		},
		.pgc   = BIT(IMX8MM_PGC_OTG2),
	},
};

static const struct imx_pgc_domain_data imx8mm_pgc_domain_data = {
	.domains = imx8mm_pgc_domains,
	.domains_num = ARRAY_SIZE(imx8mm_pgc_domains),
	.pgc_regs = &imx7_pgc_regs,
};
#endif

#ifdef CONFIG_IMX8MN
static const struct imx_pgc_domain imx8mn_pgc_domains[] = {
	[IMX8MN_POWER_DOMAIN_HSIOMIX] = {
		.bits  = {
			.pxx = 0, /* no power sequence control */
			.map = 0, /* no power sequence control */
			.hskreq = IMX8MN_HSIO_HSK_PWRDNREQN,
			.hskack = IMX8MN_HSIO_HSK_PWRDNACKN,
		},
		.keep_clocks = true,
	},

	[IMX8MN_POWER_DOMAIN_OTG1] = {
		.bits  = {
			.pxx = IMX8MN_OTG1_SW_Pxx_REQ,
			.map = IMX8MN_OTG1_A53_DOMAIN,
		},
		.pgc   = BIT(IMX8MN_PGC_OTG1),
	},
};

static const struct imx_pgc_domain_data imx8mn_pgc_domain_data = {
	.domains = imx8mn_pgc_domains,
	.domains_num = ARRAY_SIZE(imx8mn_pgc_domains),
	.pgc_regs = &imx7_pgc_regs,
};
#endif

#ifdef CONFIG_IMX8MP
static const struct imx_pgc_domain imx8mp_pgc_domains[] = {
	[IMX8MP_POWER_DOMAIN_PCIE_PHY] = {
		.bits = {
			.pxx = IMX8MP_PCIE_PHY_SW_Pxx_REQ,
			.map = IMX8MP_PCIE_PHY_A53_DOMAIN,
		},
		.pgc = BIT(IMX8MP_PGC_PCIE),
	},

	[IMX8MP_POWER_DOMAIN_USB1_PHY] = {
		.bits = {
			.pxx = IMX8MP_USB1_PHY_Pxx_REQ,
			.map = IMX8MP_USB1_PHY_A53_DOMAIN,
		},
		.pgc = BIT(IMX8MP_PGC_USB1),
	},

	[IMX8MP_POWER_DOMAIN_USB2_PHY] = {
		.bits = {
			.pxx = IMX8MP_USB2_PHY_Pxx_REQ,
			.map = IMX8MP_USB2_PHY_A53_DOMAIN,
		},
		.pgc = BIT(IMX8MP_PGC_USB2),
	},

	[IMX8MP_POWER_DOMAIN_HSIOMIX] = {
		.bits = {
			.pxx = IMX8MP_HSIOMIX_Pxx_REQ,
			.map = IMX8MP_HSIOMIX_A53_DOMAIN,
			.hskreq = IMX8MP_HSIOMIX_PWRDNREQN,
			.hskack = IMX8MP_HSIOMIX_PWRDNACKN,
		},
		.pgc = BIT(IMX8MP_PGC_HSIOMIX),
		.keep_clocks = true,
	},
};

static const struct imx_pgc_regs imx8mp_pgc_regs = {
	.map = IMX8MP_GPC_PGC_CPU_MAPPING,
	.pup = IMX8MP_GPC_PU_PGC_SW_PUP_REQ,
	.pdn = IMX8MP_GPC_PU_PGC_SW_PDN_REQ,
	.hsk = IMX8MP_GPC_PU_PWRHSK,
};

static const struct imx_pgc_domain_data imx8mp_pgc_domain_data = {
	.domains = imx8mp_pgc_domains,
	.domains_num = ARRAY_SIZE(imx8mp_pgc_domains),
	.pgc_regs = &imx8mp_pgc_regs,
};
#endif

static int imx8m_power_domain_on(struct power_domain *power_domain)
{
	struct udevice *dev = power_domain->dev;
	struct imx8m_power_domain_plat *pdata = dev_get_plat(dev);
	const struct imx_pgc_domain *domain = pdata->domain;
	const struct imx_pgc_regs *regs = pdata->regs;
	void __iomem *base = pdata->base;
	u32 pgc;
	int ret;

	if (pdata->clk.count) {
		ret = clk_enable_bulk(&pdata->clk);
		if (ret) {
			dev_err(dev, "failed to enable reset clocks\n");
			return ret;
		}
	}

	/* delay for reset to propagate */
	udelay(5);

	if (domain->bits.pxx) {
		/* request the domain to power up */
		setbits_le32(base + regs->pup, domain->bits.pxx);

		/*
		 * As per "5.5.9.4 Example Code 4" in IMX7DRM.pdf wait
		 * for PUP_REQ/PDN_REQ bit to be cleared
		 */
		ret = wait_for_bit_le32(base + regs->pup, domain->bits.pxx,
					false, 1000, false);
		if (ret) {
			dev_err(dev, "failed to command PGC\n");
			goto out_clk_disable;
		}

		/* disable power control */
		for_each_set_bit(pgc, &domain->pgc, 32) {
			clrbits_le32(base + GPC_PGC_CTRL(pgc),
				     GPC_PGC_CTRL_PCR);
		}
	}

	/* delay for reset to propagate */
	udelay(5);

	/* request the ADB400 to power up */
	if (domain->bits.hskreq)
		setbits_le32(base + regs->hsk, domain->bits.hskreq);

	/* Disable reset clocks for all devices in the domain */
	if (!domain->keep_clocks && pdata->clk.count)
		clk_disable_bulk(&pdata->clk);

	return 0;

out_clk_disable:
	if (pdata->clk.count)
		clk_disable_bulk(&pdata->clk);
	return ret;
}

static int imx8m_power_domain_off(struct power_domain *power_domain)
{
	struct udevice *dev = power_domain->dev;
	struct imx8m_power_domain_plat *pdata = dev_get_plat(dev);
	const struct imx_pgc_domain *domain = pdata->domain;
	const struct imx_pgc_regs *regs = pdata->regs;
	void __iomem *base = pdata->base;
	u32 pgc;
	int ret;

	/* Enable reset clocks for all devices in the domain */
	if (!domain->keep_clocks && pdata->clk.count) {
		ret = clk_enable_bulk(&pdata->clk);
		if (ret)
			return ret;
	}

	/* request the ADB400 to power down */
	if (domain->bits.hskreq) {
		clrbits_le32(base + regs->hsk, domain->bits.hskreq);

		ret = wait_for_bit_le32(base + regs->hsk, domain->bits.hskack,
					false, 1000, false);
		if (ret) {
			dev_err(dev, "failed to power down ADB400\n");
			goto out_clk_disable;
		}
	}

	if (domain->bits.pxx) {
		/* enable power control */
		for_each_set_bit(pgc, &domain->pgc, 32) {
			setbits_le32(base + GPC_PGC_CTRL(pgc),
				     GPC_PGC_CTRL_PCR);
		}

		/* request the domain to power down */
		setbits_le32(base + regs->pdn, domain->bits.pxx);

		/*
		 * As per "5.5.9.4 Example Code 4" in IMX7DRM.pdf wait
		 * for PUP_REQ/PDN_REQ bit to be cleared
		 */
		ret = wait_for_bit_le32(base + regs->pdn, domain->bits.pxx,
					false, 1000, false);
		if (ret) {
			dev_err(dev, "failed to command PGC\n");
			goto out_clk_disable;
		}
	}

	/* Disable reset clocks for all devices in the domain */
	if (pdata->clk.count)
		clk_disable_bulk(&pdata->clk);

	if (pdata->has_pd)
		power_domain_off(&pdata->pd);

	return 0;

out_clk_disable:
	if (!domain->keep_clocks && pdata->clk.count)
		clk_disable_bulk(&pdata->clk);

	return ret;
}

static int imx8m_power_domain_of_xlate(struct power_domain *power_domain,
				      struct ofnode_phandle_args *args)
{
	return 0;
}

static int imx8m_power_domain_bind(struct udevice *dev)
{
	int offset;
	const char *name;
	int ret = 0;

	offset = dev_of_offset(dev);
	for (offset = fdt_first_subnode(gd->fdt_blob, offset); offset > 0;
	     offset = fdt_next_subnode(gd->fdt_blob, offset)) {
		/* Bind the subnode to this driver */
		name = fdt_get_name(gd->fdt_blob, offset, NULL);

		/* Descend into 'pgc' subnode */
		if (!strstr(name, "power-domain")) {
			offset = fdt_first_subnode(gd->fdt_blob, offset);
			name = fdt_get_name(gd->fdt_blob, offset, NULL);
		}

		ret = device_bind_with_driver_data(dev, dev->driver, name,
						   dev->driver_data,
						   offset_to_ofnode(offset),
						   NULL);

		if (ret == -ENODEV)
			printf("Driver '%s' refuses to bind\n",
			       dev->driver->name);

		if (ret)
			printf("Error binding driver '%s': %d\n",
			       dev->driver->name, ret);
	}

	return 0;
}

static int imx8m_power_domain_probe(struct udevice *dev)
{
	struct imx8m_power_domain_plat *pdata = dev_get_plat(dev);
	int ret;

	/* Nothing to do for non-"power-domain" driver instances. */
	if (!strstr(dev->name, "power-domain"))
		return 0;

	/* Grab optional power domain clock. */
	ret = clk_get_bulk(dev, &pdata->clk);
	if (ret && ret != -ENOENT) {
		dev_err(dev, "Failed to get domain clock (%d)\n", ret);
		return ret;
	}

	return 0;
}

static int imx8m_power_domain_of_to_plat(struct udevice *dev)
{
	struct imx8m_power_domain_plat *pdata = dev_get_plat(dev);
	struct imx_pgc_domain_data *domain_data =
		(struct imx_pgc_domain_data *)dev_get_driver_data(dev);

	pdata->resource_id = fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev),
					    "reg", -1);
	pdata->domain = &domain_data->domains[pdata->resource_id];
	pdata->regs = domain_data->pgc_regs;
	pdata->base = dev_read_addr_ptr(dev->parent);

	if (!power_domain_get(dev, &pdata->pd))
		pdata->has_pd = 1;

	return 0;
}

static const struct udevice_id imx8m_power_domain_ids[] = {
#ifdef CONFIG_IMX8MQ
	{ .compatible = "fsl,imx8mq-gpc", .data = (long)&imx8m_pgc_domain_data },
#endif
#ifdef CONFIG_IMX8MM
	{ .compatible = "fsl,imx8mm-gpc", .data = (long)&imx8mm_pgc_domain_data },
#endif
#ifdef CONFIG_IMX8MN
	{ .compatible = "fsl,imx8mn-gpc", .data = (long)&imx8mn_pgc_domain_data },
#endif
#ifdef CONFIG_IMX8MP
	{ .compatible = "fsl,imx8mp-gpc", .data = (long)&imx8mp_pgc_domain_data },
#endif
	{ }
};

struct power_domain_ops imx8m_power_domain_ops = {
	.on = imx8m_power_domain_on,
	.off = imx8m_power_domain_off,
	.of_xlate = imx8m_power_domain_of_xlate,
};

U_BOOT_DRIVER(imx8m_power_domain) = {
	.name = "imx8m_power_domain",
	.id = UCLASS_POWER_DOMAIN,
	.of_match = imx8m_power_domain_ids,
	.bind = imx8m_power_domain_bind,
	.probe = imx8m_power_domain_probe,
	.of_to_plat = imx8m_power_domain_of_to_plat,
	.plat_auto	= sizeof(struct imx8m_power_domain_plat),
	.ops = &imx8m_power_domain_ops,
};

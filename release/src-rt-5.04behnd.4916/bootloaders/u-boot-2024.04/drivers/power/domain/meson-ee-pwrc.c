// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#include <common.h>
#include <dm.h>
#include <log.h>
#include <malloc.h>
#include <power-domain-uclass.h>
#include <regmap.h>
#include <syscon.h>
#include <reset.h>
#include <clk.h>
#include <dt-bindings/power/meson-axg-power.h>
#include <dt-bindings/power/meson-g12a-power.h>
#include <dt-bindings/power/meson-gxbb-power.h>
#include <dt-bindings/power/meson-sm1-power.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/err.h>

/* AO Offsets */

#define AO_RTI_GEN_PWR_SLEEP0		(0x3a << 2)
#define AO_RTI_GEN_PWR_ISO0		(0x3b << 2)

/* HHI Offsets */

#define HHI_MEM_PD_REG0			(0x40 << 2)
#define HHI_VPU_MEM_PD_REG0		(0x41 << 2)
#define HHI_VPU_MEM_PD_REG1		(0x42 << 2)
#define HHI_VPU_MEM_PD_REG3		(0x43 << 2)
#define HHI_VPU_MEM_PD_REG4		(0x44 << 2)
#define HHI_AUDIO_MEM_PD_REG0		(0x45 << 2)
#define HHI_NANOQ_MEM_PD_REG0		(0x46 << 2)
#define HHI_NANOQ_MEM_PD_REG1		(0x47 << 2)
#define HHI_VPU_MEM_PD_REG2		(0x4d << 2)

struct meson_ee_pwrc;
struct meson_ee_pwrc_domain;

struct meson_ee_pwrc_mem_domain {
	unsigned int reg;
	unsigned int mask;
};

struct meson_ee_pwrc_top_domain {
	unsigned int sleep_reg;
	unsigned int sleep_mask;
	unsigned int iso_reg;
	unsigned int iso_mask;
};

struct meson_ee_pwrc_domain_desc {
	char *name;
	unsigned int reset_names_count;
	unsigned int clk_names_count;
	struct meson_ee_pwrc_top_domain *top_pd;
	unsigned int mem_pd_count;
	struct meson_ee_pwrc_mem_domain *mem_pd;
	bool (*get_power)(struct power_domain *power_domain);
};

struct meson_ee_pwrc_domain_data {
	unsigned int count;
	struct meson_ee_pwrc_domain_desc *domains;
};

/* TOP Power Domains */

static struct meson_ee_pwrc_top_domain gx_pwrc_vpu = {
	.sleep_reg = AO_RTI_GEN_PWR_SLEEP0,
	.sleep_mask = BIT(8),
	.iso_reg = AO_RTI_GEN_PWR_SLEEP0,
	.iso_mask = BIT(9),
};

#define SM1_EE_PD(__bit)					\
	{							\
		.sleep_reg = AO_RTI_GEN_PWR_SLEEP0,		\
		.sleep_mask = BIT(__bit),			\
		.iso_reg = AO_RTI_GEN_PWR_ISO0,			\
		.iso_mask = BIT(__bit),				\
	}

static struct meson_ee_pwrc_top_domain sm1_pwrc_vpu = SM1_EE_PD(8);
static struct meson_ee_pwrc_top_domain sm1_pwrc_nna = SM1_EE_PD(16);
static struct meson_ee_pwrc_top_domain sm1_pwrc_usb = SM1_EE_PD(17);
static struct meson_ee_pwrc_top_domain sm1_pwrc_pci = SM1_EE_PD(18);
static struct meson_ee_pwrc_top_domain sm1_pwrc_ge2d = SM1_EE_PD(19);

/* Memory PD Domains */

#define VPU_MEMPD(__reg)					\
	{ __reg, GENMASK(1, 0) },				\
	{ __reg, GENMASK(3, 2) },				\
	{ __reg, GENMASK(5, 4) },				\
	{ __reg, GENMASK(7, 6) },				\
	{ __reg, GENMASK(9, 8) },				\
	{ __reg, GENMASK(11, 10) },				\
	{ __reg, GENMASK(13, 12) },				\
	{ __reg, GENMASK(15, 14) },				\
	{ __reg, GENMASK(17, 16) },				\
	{ __reg, GENMASK(19, 18) },				\
	{ __reg, GENMASK(21, 20) },				\
	{ __reg, GENMASK(23, 22) },				\
	{ __reg, GENMASK(25, 24) },				\
	{ __reg, GENMASK(27, 26) },				\
	{ __reg, GENMASK(29, 28) },				\
	{ __reg, GENMASK(31, 30) }

#define VPU_HHI_MEMPD(__reg)					\
	{ __reg, BIT(8) },					\
	{ __reg, BIT(9) },					\
	{ __reg, BIT(10) },					\
	{ __reg, BIT(11) },					\
	{ __reg, BIT(12) },					\
	{ __reg, BIT(13) },					\
	{ __reg, BIT(14) },					\
	{ __reg, BIT(15) }

static struct meson_ee_pwrc_mem_domain g12a_pwrc_mem_vpu[] = {
	VPU_MEMPD(HHI_VPU_MEM_PD_REG0),
	VPU_MEMPD(HHI_VPU_MEM_PD_REG1),
	VPU_MEMPD(HHI_VPU_MEM_PD_REG2),
	VPU_HHI_MEMPD(HHI_MEM_PD_REG0),
};

static struct meson_ee_pwrc_mem_domain axg_pwrc_mem_vpu[] = {
	VPU_MEMPD(HHI_VPU_MEM_PD_REG0),
	VPU_HHI_MEMPD(HHI_MEM_PD_REG0),
};

static struct meson_ee_pwrc_mem_domain gxbb_pwrc_mem_vpu[] = {
	VPU_MEMPD(HHI_VPU_MEM_PD_REG0),
	VPU_MEMPD(HHI_VPU_MEM_PD_REG1),
	VPU_HHI_MEMPD(HHI_MEM_PD_REG0),
};

static struct meson_ee_pwrc_mem_domain g12a_pwrc_mem_eth[] = {
	{ HHI_MEM_PD_REG0, GENMASK(3, 2) },
};

static struct meson_ee_pwrc_mem_domain sm1_pwrc_mem_vpu[] = {
	VPU_MEMPD(HHI_VPU_MEM_PD_REG0),
	VPU_MEMPD(HHI_VPU_MEM_PD_REG1),
	VPU_MEMPD(HHI_VPU_MEM_PD_REG2),
	VPU_MEMPD(HHI_VPU_MEM_PD_REG3),
	{ HHI_VPU_MEM_PD_REG4, GENMASK(1, 0) },
	{ HHI_VPU_MEM_PD_REG4, GENMASK(3, 2) },
	{ HHI_VPU_MEM_PD_REG4, GENMASK(5, 4) },
	{ HHI_VPU_MEM_PD_REG4, GENMASK(7, 6) },
	VPU_HHI_MEMPD(HHI_MEM_PD_REG0),
};

static struct meson_ee_pwrc_mem_domain sm1_pwrc_mem_nna[] = {
	{ HHI_NANOQ_MEM_PD_REG0, 0xff },
	{ HHI_NANOQ_MEM_PD_REG1, 0xff },
};

static struct meson_ee_pwrc_mem_domain sm1_pwrc_mem_usb[] = {
	{ HHI_MEM_PD_REG0, GENMASK(31, 30) },
};

static struct meson_ee_pwrc_mem_domain sm1_pwrc_mem_pcie[] = {
	{ HHI_MEM_PD_REG0, GENMASK(29, 26) },
};

static struct meson_ee_pwrc_mem_domain sm1_pwrc_mem_ge2d[] = {
	{ HHI_MEM_PD_REG0, GENMASK(25, 18) },
};

static struct meson_ee_pwrc_mem_domain axg_pwrc_mem_audio[] = {
	{ HHI_MEM_PD_REG0, GENMASK(5, 4) },
};

static struct meson_ee_pwrc_mem_domain sm1_pwrc_mem_audio[] = {
	{ HHI_MEM_PD_REG0, GENMASK(5, 4) },
	{ HHI_AUDIO_MEM_PD_REG0, GENMASK(1, 0) },
	{ HHI_AUDIO_MEM_PD_REG0, GENMASK(3, 2) },
	{ HHI_AUDIO_MEM_PD_REG0, GENMASK(5, 4) },
	{ HHI_AUDIO_MEM_PD_REG0, GENMASK(7, 6) },
	{ HHI_AUDIO_MEM_PD_REG0, GENMASK(13, 12) },
	{ HHI_AUDIO_MEM_PD_REG0, GENMASK(15, 14) },
	{ HHI_AUDIO_MEM_PD_REG0, GENMASK(17, 16) },
	{ HHI_AUDIO_MEM_PD_REG0, GENMASK(19, 18) },
	{ HHI_AUDIO_MEM_PD_REG0, GENMASK(21, 20) },
	{ HHI_AUDIO_MEM_PD_REG0, GENMASK(23, 22) },
	{ HHI_AUDIO_MEM_PD_REG0, GENMASK(25, 24) },
	{ HHI_AUDIO_MEM_PD_REG0, GENMASK(27, 26) },
};

#define VPU_PD(__name, __top_pd, __mem, __get_power, __resets, __clks)	\
	{								\
		.name = __name,						\
		.reset_names_count = __resets,				\
		.clk_names_count = __clks,				\
		.top_pd = __top_pd,					\
		.mem_pd_count = ARRAY_SIZE(__mem),			\
		.mem_pd = __mem,					\
		.get_power = __get_power,				\
	}

#define TOP_PD(__name, __top_pd, __mem, __get_power)			\
	{								\
		.name = __name,						\
		.top_pd = __top_pd,					\
		.mem_pd_count = ARRAY_SIZE(__mem),			\
		.mem_pd = __mem,					\
		.get_power = __get_power,				\
	}

#define MEM_PD(__name, __mem)						\
	TOP_PD(__name, NULL, __mem, NULL)

static bool pwrc_ee_get_power(struct power_domain *power_domain);

static struct meson_ee_pwrc_domain_desc g12a_pwrc_domains[] = {
	[PWRC_G12A_VPU_ID]  = VPU_PD("VPU", &gx_pwrc_vpu, g12a_pwrc_mem_vpu,
				     pwrc_ee_get_power, 11, 2),
	[PWRC_G12A_ETH_ID] = MEM_PD("ETH", g12a_pwrc_mem_eth),
};

static struct meson_ee_pwrc_domain_desc axg_pwrc_domains[] = {
	[PWRC_AXG_VPU_ID]  = VPU_PD("VPU", &gx_pwrc_vpu, axg_pwrc_mem_vpu,
			pwrc_ee_get_power, 5, 2),
	[PWRC_AXG_ETHERNET_MEM_ID] = MEM_PD("ETH", g12a_pwrc_mem_eth),
	[PWRC_AXG_AUDIO_ID] = MEM_PD("AUDIO", axg_pwrc_mem_audio),
};

static struct meson_ee_pwrc_domain_desc gxbb_pwrc_domains[] = {
	[PWRC_GXBB_VPU_ID]  = VPU_PD("VPU", &gx_pwrc_vpu, gxbb_pwrc_mem_vpu,
			pwrc_ee_get_power, 12, 2),
	[PWRC_GXBB_ETHERNET_MEM_ID] = MEM_PD("ETH", g12a_pwrc_mem_eth),
};

static struct meson_ee_pwrc_domain_desc sm1_pwrc_domains[] = {
	[PWRC_SM1_VPU_ID]  = VPU_PD("VPU", &sm1_pwrc_vpu, sm1_pwrc_mem_vpu,
				    pwrc_ee_get_power, 11, 2),
	[PWRC_SM1_NNA_ID]  = TOP_PD("NNA", &sm1_pwrc_nna, sm1_pwrc_mem_nna,
				    pwrc_ee_get_power),
	[PWRC_SM1_USB_ID]  = TOP_PD("USB", &sm1_pwrc_usb, sm1_pwrc_mem_usb,
				    pwrc_ee_get_power),
	[PWRC_SM1_PCIE_ID] = TOP_PD("PCI", &sm1_pwrc_pci, sm1_pwrc_mem_pcie,
				    pwrc_ee_get_power),
	[PWRC_SM1_GE2D_ID] = TOP_PD("GE2D", &sm1_pwrc_ge2d, sm1_pwrc_mem_ge2d,
				    pwrc_ee_get_power),
	[PWRC_SM1_AUDIO_ID] = MEM_PD("AUDIO", sm1_pwrc_mem_audio),
	[PWRC_SM1_ETH_ID] = MEM_PD("ETH", g12a_pwrc_mem_eth),
};

struct meson_ee_pwrc_priv {
	struct regmap *regmap_ao;
	struct regmap *regmap_hhi;
	struct reset_ctl_bulk resets;
	struct clk_bulk clks;
	const struct meson_ee_pwrc_domain_data *data;
};

static bool pwrc_ee_get_power(struct power_domain *power_domain)
{
	struct meson_ee_pwrc_priv *priv = dev_get_priv(power_domain->dev);
	struct meson_ee_pwrc_domain_desc *pwrc_domain;
	u32 reg;

	pwrc_domain = &priv->data->domains[power_domain->id];

	regmap_read(priv->regmap_ao,
		    pwrc_domain->top_pd->sleep_reg, &reg);

	return (reg & pwrc_domain->top_pd->sleep_mask);
}

static int meson_ee_pwrc_off(struct power_domain *power_domain)
{
	struct meson_ee_pwrc_priv *priv = dev_get_priv(power_domain->dev);
	struct meson_ee_pwrc_domain_desc *pwrc_domain;
	int i;

	pwrc_domain = &priv->data->domains[power_domain->id];

	if (pwrc_domain->top_pd)
		regmap_update_bits(priv->regmap_ao,
				   pwrc_domain->top_pd->sleep_reg,
				   pwrc_domain->top_pd->sleep_mask,
				   pwrc_domain->top_pd->sleep_mask);
	udelay(20);

	for (i = 0 ; i < pwrc_domain->mem_pd_count ; ++i)
		regmap_update_bits(priv->regmap_hhi,
				   pwrc_domain->mem_pd[i].reg,
				   pwrc_domain->mem_pd[i].mask,
				   pwrc_domain->mem_pd[i].mask);

	udelay(20);

	if (pwrc_domain->top_pd)
		regmap_update_bits(priv->regmap_ao,
				   pwrc_domain->top_pd->iso_reg,
				   pwrc_domain->top_pd->iso_mask,
				   pwrc_domain->top_pd->iso_mask);

	if (pwrc_domain->clk_names_count) {
		mdelay(20);
		clk_disable_bulk(&priv->clks);
	}

	return 0;
}

static int meson_ee_pwrc_on(struct power_domain *power_domain)
{
	struct meson_ee_pwrc_priv *priv = dev_get_priv(power_domain->dev);
	struct meson_ee_pwrc_domain_desc *pwrc_domain;
	int i, ret;

	pwrc_domain = &priv->data->domains[power_domain->id];

	if (pwrc_domain->top_pd)
		regmap_update_bits(priv->regmap_ao,
				   pwrc_domain->top_pd->sleep_reg,
				   pwrc_domain->top_pd->sleep_mask, 0);
	udelay(20);

	for (i = 0 ; i < pwrc_domain->mem_pd_count ; ++i)
		regmap_update_bits(priv->regmap_hhi,
				   pwrc_domain->mem_pd[i].reg,
				   pwrc_domain->mem_pd[i].mask, 0);

	udelay(20);

	if (pwrc_domain->reset_names_count) {
		ret = reset_assert_bulk(&priv->resets);
		if (ret)
			return ret;
	}

	if (pwrc_domain->top_pd)
		regmap_update_bits(priv->regmap_ao,
				   pwrc_domain->top_pd->iso_reg,
				   pwrc_domain->top_pd->iso_mask, 0);

	if (pwrc_domain->reset_names_count) {
		ret = reset_deassert_bulk(&priv->resets);
		if (ret)
			return ret;
	}

	if (pwrc_domain->clk_names_count)
		return clk_enable_bulk(&priv->clks);

	return 0;
}

static int meson_ee_pwrc_of_xlate(struct power_domain *power_domain,
				  struct ofnode_phandle_args *args)
{
	struct meson_ee_pwrc_priv *priv = dev_get_priv(power_domain->dev);

	/* #power-domain-cells is 1 */

	if (args->args_count < 1) {
		debug("Invalid args_count: %d\n", args->args_count);
		return -EINVAL;
	}

	power_domain->id = args->args[0];

	if (power_domain->id >= priv->data->count) {
		debug("Invalid domain ID: %lu\n", power_domain->id);
		return -EINVAL;
	}

	return 0;
}

struct power_domain_ops meson_ee_pwrc_ops = {
	.off = meson_ee_pwrc_off,
	.on = meson_ee_pwrc_on,
	.of_xlate = meson_ee_pwrc_of_xlate,
};

static struct meson_ee_pwrc_domain_data meson_ee_g12a_pwrc_data = {
	.count = ARRAY_SIZE(g12a_pwrc_domains),
	.domains = g12a_pwrc_domains,
};

static struct meson_ee_pwrc_domain_data meson_ee_axg_pwrc_data = {
	.count = ARRAY_SIZE(axg_pwrc_domains),
	.domains = axg_pwrc_domains,
};

static struct meson_ee_pwrc_domain_data meson_ee_gxbb_pwrc_data = {
	.count = ARRAY_SIZE(gxbb_pwrc_domains),
	.domains = gxbb_pwrc_domains,
};

static struct meson_ee_pwrc_domain_data meson_ee_sm1_pwrc_data = {
	.count = ARRAY_SIZE(sm1_pwrc_domains),
	.domains = sm1_pwrc_domains,
};

static const struct udevice_id meson_ee_pwrc_ids[] = {
	{
		.compatible = "amlogic,meson-g12a-pwrc",
		.data = (unsigned long)&meson_ee_g12a_pwrc_data,
	},
	{
		.compatible = "amlogic,meson-gxbb-pwrc",
		.data = (unsigned long)&meson_ee_gxbb_pwrc_data,
	},
	{
		.compatible = "amlogic,meson-axg-pwrc",
		.data = (unsigned long)&meson_ee_axg_pwrc_data,
	},
	{
		.compatible = "amlogic,meson-sm1-pwrc",
		.data = (unsigned long)&meson_ee_sm1_pwrc_data,
	},
	{ }
};

static int meson_ee_pwrc_probe(struct udevice *dev)
{
	struct meson_ee_pwrc_priv *priv = dev_get_priv(dev);
	u32 ao_phandle;
	ofnode ao_node;
	int ret;

	priv->data = (void *)dev_get_driver_data(dev);
	if (!priv->data)
		return -EINVAL;

	priv->regmap_hhi = syscon_node_to_regmap(dev_ofnode(dev_get_parent(dev)));
	if (IS_ERR(priv->regmap_hhi))
		return PTR_ERR(priv->regmap_hhi);

	ret = ofnode_read_u32(dev_ofnode(dev), "amlogic,ao-sysctrl",
			      &ao_phandle);
	if (ret)
		return ret;

	ao_node = ofnode_get_by_phandle(ao_phandle);
	if (!ofnode_valid(ao_node))
		return -EINVAL;

	priv->regmap_ao = syscon_node_to_regmap(ao_node);
	if (IS_ERR(priv->regmap_ao))
		return PTR_ERR(priv->regmap_ao);

	ret = reset_get_bulk(dev, &priv->resets);
	if (ret)
		return ret;

	ret = clk_get_bulk(dev, &priv->clks);
	if (ret)
		return ret;

	return 0;
}

U_BOOT_DRIVER(meson_ee_pwrc) = {
	.name = "meson_ee_pwrc",
	.id = UCLASS_POWER_DOMAIN,
	.of_match = meson_ee_pwrc_ids,
	.probe = meson_ee_pwrc_probe,
	.ops = &meson_ee_pwrc_ops,
	.priv_auto	= sizeof(struct meson_ee_pwrc_priv),
};

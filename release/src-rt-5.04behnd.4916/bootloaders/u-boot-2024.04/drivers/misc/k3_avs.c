// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments' K3 Clas 0 Adaptive Voltage Scaling driver
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com/
 *      Tero Kristo <t-kristo@ti.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <i2c.h>
#include <k3-avs.h>
#include <dm/device_compat.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <power/regulator.h>

#define AM6_VTM_DEVINFO(i)	(priv->base + 0x100 + 0x20 * (i))
#define AM6_VTM_OPPVID_VD(i)	(priv->base + 0x104 + 0x20 * (i))

#define AM6_VTM_AVS0_SUPPORTED	BIT(12)

#define AM6_VTM_OPP_SHIFT(opp)	(8 * (opp))
#define AM6_VTM_OPP_MASK	0xff

#define K3_VTM_DEVINFO_PWR0_OFFSET		0x4
#define K3_VTM_DEVINFO_PWR0_TEMPSENS_CT_MASK	0xf0
#define K3_VTM_TMPSENS0_CTRL_OFFSET		0x300
#define K3_VTM_TMPSENS_STAT_OFFSET		0x8
#define K3_VTM_ANYMAXT_OUTRG_ALERT_EN		0x1
#define K3_VTM_LOW_TEMP_OFFSET			0x10
#define K3_VTM_MISC_CTRL2_OFFSET		0x10
#define K3_VTM_MISC_CTRL1_OFFSET		0xc
#define K3_VTM_TMPSENS_CTRL1_SOC		BIT(5)
#define K3_VTM_TMPSENS_CTRL_CLRZ		BIT(6)
#define K3_VTM_TMPSENS_CTRL_MAXT_OUTRG_EN	BIT(11)
#define K3_VTM_ADC_COUNT_FOR_123C		0x2f8
#define K3_VTM_ADC_COUNT_FOR_105C		0x288
#define K3_VTM_ADC_WA_VALUE			0x2c
#define K3_VTM_FUSE_MASK			0xc0000000

#define VD_FLAG_INIT_DONE	BIT(0)

struct k3_avs_privdata {
	void *base;
	struct vd_config *vd_config;
	struct udevice *dev;
};

struct opp {
	u32 freq;
	u32 volt;
};

struct vd_data {
	int id;
	u8 opp;
	u8 flags;
	int dev_id;
	int clk_id;
	struct opp opps[NUM_OPPS];
	struct udevice *supply;
};

struct vd_config {
	struct vd_data *vds;
	u32 (*efuse_xlate)(struct k3_avs_privdata *priv, int idx, int opp);
};

static struct k3_avs_privdata *k3_avs_priv;

/**
 * am6_efuse_voltage: read efuse voltage from VTM
 * @priv: driver private data
 * @idx: VD to read efuse for
 * @opp: opp id to read
 *
 * Reads efuse value for the specified OPP, and converts the register
 * value to a voltage. Returns the voltage in uV, or 0 if nominal voltage
 * should be used.
 *
 * Efuse val to volt conversion logic:
 *
 * val > 171 volt increments in 20mV steps with base 171 => 1.66V
 * val between 115 to 11 increments in 10mV steps with base 115 => 1.1V
 * val between 15 to 115 increments in 5mV steps with base 15 => .6V
 * val between 1 to 15 increments in 20mv steps with base 0 => .3V
 * val 0 is invalid
 */
static u32 am6_efuse_xlate(struct k3_avs_privdata *priv, int idx, int opp)
{
	u32 val = readl(AM6_VTM_OPPVID_VD(idx));

	val >>= AM6_VTM_OPP_SHIFT(opp);
	val &= AM6_VTM_OPP_MASK;

	if (!val)
		return 0;

	if (val > 171)
		return 1660000 + 20000 * (val - 171);

	if (val > 115)
		return 1100000 + 10000 * (val - 115);

	if (val > 15)
		return 600000 + 5000 * (val - 15);

	return 300000 + 20000 * val;
}

static int k3_avs_program_voltage(struct k3_avs_privdata *priv,
				  struct vd_data *vd,
				  int opp_id)
{
	u32 volt = vd->opps[opp_id].volt;
	struct vd_data *vd2;

	if (!vd->supply)
		return -ENODEV;

	vd->opp = opp_id;
	vd->flags |= VD_FLAG_INIT_DONE;

	/* Take care of ganged rails and pick the Max amongst them*/
	for (vd2 = priv->vd_config->vds; vd2->id >= 0; vd2++) {
		if (vd == vd2)
			continue;

		if (vd2->supply != vd->supply)
			continue;

		if (vd2->opps[vd2->opp].volt > volt)
			volt = vd2->opps[vd2->opp].volt;

		vd2->flags |= VD_FLAG_INIT_DONE;
	}

	return regulator_set_value(vd->supply, volt);
}

static struct vd_data *get_vd(struct k3_avs_privdata *priv, int idx)
{
	struct vd_data *vd;

	for (vd = priv->vd_config->vds; vd->id >= 0 && vd->id != idx; vd++)
		;

	if (vd->id < 0)
		return NULL;

	return vd;
}

/**
 * k3_avs_set_opp: Sets the voltage for an arbitrary VD rail
 * @dev: AVS device
 * @vdd_id: voltage domain ID
 * @opp_id: OPP ID
 *
 * Programs the desired OPP value for the defined voltage rail. This
 * should be called from board files if reconfiguration is desired.
 * Returns 0 on success, negative error value on failure.
 */
int k3_avs_set_opp(struct udevice *dev, int vdd_id, int opp_id)
{
	struct k3_avs_privdata *priv = dev_get_priv(dev);
	struct vd_data *vd;

	vd = get_vd(priv, vdd_id);
	if (!vd)
		return -EINVAL;

	return k3_avs_program_voltage(priv, vd, opp_id);
}

static int match_opp(struct vd_data *vd, u32 freq)
{
	struct opp *opp;
	int opp_id;

	for (opp_id = 0; opp_id < NUM_OPPS; opp_id++) {
		opp = &vd->opps[opp_id];
		if (opp->freq == freq)
			return opp_id;
	}

	printf("No matching OPP found for freq %d.\n", freq);

	return -EINVAL;
}

/**
 * k3_avs_notify_freq: Notify clock rate change towards AVS subsystem
 * @dev_id: Device ID for the clock to be changed
 * @clk_id: Clock ID for the clock to be changed
 * @freq: New frequency for clock
 *
 * Checks if the provided clock is the MPU clock or not, if not, return
 * immediately. If MPU clock is provided, maps the provided MPU frequency
 * towards an MPU OPP, and programs the voltage to the regulator. Return 0
 * on success, negative error value on failure.
 */
int k3_avs_notify_freq(int dev_id, int clk_id, u32 freq)
{
	int opp_id;
	struct k3_avs_privdata *priv = k3_avs_priv;
	struct vd_data *vd;

	/* Driver may not be probed yet */
	if (!priv)
		return -EINVAL;

	for (vd = priv->vd_config->vds; vd->id >= 0; vd++) {
		if (vd->dev_id != dev_id || vd->clk_id != clk_id)
			continue;

		opp_id = match_opp(vd, freq);
		if (opp_id < 0)
			return opp_id;

		vd->opp = opp_id;
		return k3_avs_program_voltage(priv, vd, opp_id);
	}

	return -EINVAL;
}

static int k3_avs_configure(struct udevice *dev, struct k3_avs_privdata *priv)
{
	struct vd_config *conf;
	int ret;
	char pname[20];
	struct vd_data *vd;

	conf = (void *)dev_get_driver_data(dev);

	priv->vd_config = conf;

	for (vd = conf->vds; vd->id >= 0; vd++) {
		sprintf(pname, "vdd-supply-%d", vd->id);
		ret = device_get_supply_regulator(dev, pname, &vd->supply);
		if (ret)
			dev_warn(dev, "supply not found for VD%d.\n", vd->id);

		sprintf(pname, "ti,default-opp-%d", vd->id);
		ret = dev_read_u32_default(dev, pname, -1);
		if (ret != -1)
			vd->opp = ret;
	}

	return 0;
}

/* k3_avs_program_tshut : Program thermal shutdown value for SOC
 * set the values corresponding to thresholds to ~123C and 105C
 * This is optional feature, Few times OS driver takes care of
 * tshut programing.
 */

static void k3_avs_program_tshut(struct k3_avs_privdata *priv)
{
	int cnt, id, val;
	int workaround_needed = 0;
	u32 ctrl_offset;
	void __iomem *cfg2_base;
	void __iomem *fuse_base;

	cfg2_base = (void __iomem *)devfdt_get_addr_index(priv->dev, 1);
	if (IS_ERR(cfg2_base)) {
		dev_err(priv->dev, "cfg base is not defined\n");
		return;
	}

	/*
	 * Some of TI's J721E SoCs require a software trimming procedure
	 * for the temperature monitors to function properly. To determine
	 * if this particular SoC is NOT affected, both bits in the
	 * WKUP_SPARE_FUSE0[31:30] will be set (0xC0000000) indicating
	 * when software trimming should NOT be applied.
	 *
	 * https://www.ti.com/lit/er/sprz455c/sprz455c.pdf
	 * This routine checks if workaround_needed to be applied or not
	 * based upon workaround_needed, adjust fixed value of tshut high and low
	 */

	if (device_is_compatible(priv->dev, "ti,j721e-vtm")) {
		fuse_base = (void __iomem *)devfdt_get_addr_index(priv->dev, 2);
		if (IS_ERR(fuse_base)) {
			dev_err(priv->dev, "fuse-base is not defined for J721E Soc\n");
			return;
		}

		if (!((readl(fuse_base) & K3_VTM_FUSE_MASK) == K3_VTM_FUSE_MASK))
			workaround_needed = 1;
	}

	dev_dbg(priv->dev, "Work around %sneeded\n", workaround_needed ? "" : "not ");

	/* Get the sensor count in the VTM */
	val = readl(priv->base + K3_VTM_DEVINFO_PWR0_OFFSET);
	cnt = val & K3_VTM_DEVINFO_PWR0_TEMPSENS_CT_MASK;
	cnt >>= __ffs(K3_VTM_DEVINFO_PWR0_TEMPSENS_CT_MASK);

	/* Program the thermal sensors */
	for (id = 0; id < cnt; id++) {
		ctrl_offset = K3_VTM_TMPSENS0_CTRL_OFFSET + id * 0x20;

		val = readl(cfg2_base + ctrl_offset);
		val |= (K3_VTM_TMPSENS_CTRL_MAXT_OUTRG_EN |
			K3_VTM_TMPSENS_CTRL1_SOC |
			K3_VTM_TMPSENS_CTRL_CLRZ | BIT(4));
		writel(val, cfg2_base + ctrl_offset);
	}

	/*
	 * Program TSHUT thresholds
	 * Step 1: set the thresholds to ~123C and 105C WKUP_VTM_MISC_CTRL2
	 * Step 2: WKUP_VTM_TMPSENS_CTRL_j set the MAXT_OUTRG_EN  bit
	 *         This is already taken care as per of init
	 * Step 3: WKUP_VTM_MISC_CTRL set the ANYMAXT_OUTRG_ALERT_EN  bit
	 */

	/* Low thresholds for tshut*/
	val = (K3_VTM_ADC_COUNT_FOR_105C - workaround_needed * K3_VTM_ADC_WA_VALUE)
		<< K3_VTM_LOW_TEMP_OFFSET;
	/* high thresholds */
	val |= K3_VTM_ADC_COUNT_FOR_123C - workaround_needed * K3_VTM_ADC_WA_VALUE;

	writel(val, cfg2_base + K3_VTM_MISC_CTRL2_OFFSET);
	/* ramp-up delay from Linux code */
	mdelay(100);
	val = readl(cfg2_base + K3_VTM_MISC_CTRL1_OFFSET) | K3_VTM_ANYMAXT_OUTRG_ALERT_EN;
	writel(val, cfg2_base + K3_VTM_MISC_CTRL1_OFFSET);
}

/**
 * k3_avs_probe: parses VD info from VTM, and re-configures the OPP data
 *
 * Parses all VDs on a device calculating the AVS class-0 voltages for them,
 * and updates the vd_data based on this. The vd_data itself shall be used
 * to program the required OPPs later on. Returns 0 on success, negative
 * error value on failure.
 */
static int k3_avs_probe(struct udevice *dev)
{
	int opp_id;
	u32 volt;
	struct opp *opp;
	struct k3_avs_privdata *priv;
	struct vd_data *vd;
	int ret;

	priv = dev_get_priv(dev);
	priv->dev = dev;

	k3_avs_priv = priv;

	ret = k3_avs_configure(dev, priv);
	if (ret)
		return ret;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -ENODEV;

	for (vd = priv->vd_config->vds; vd->id >= 0; vd++) {
		if (!(readl(AM6_VTM_DEVINFO(vd->id)) &
		      AM6_VTM_AVS0_SUPPORTED)) {
			dev_warn(dev, "AVS-class 0 not supported for VD%d\n",
				 vd->id);
			continue;
		}

		for (opp_id = 0; opp_id < NUM_OPPS; opp_id++) {
			opp = &vd->opps[opp_id];

			if (!opp->freq)
				continue;

			volt = priv->vd_config->efuse_xlate(priv, vd->id,
							    opp_id);
			if (volt)
				opp->volt = volt;
		}
	}

	for (vd = priv->vd_config->vds; vd->id >= 0; vd++) {
		if (vd->flags & VD_FLAG_INIT_DONE)
			continue;

		k3_avs_program_voltage(priv, vd, vd->opp);
	}

	if (!device_is_compatible(priv->dev, "ti,am654-avs"))
		k3_avs_program_tshut(priv);

	return 0;
}

static struct vd_data am654_vd_data[] = {
	{
		.id = AM6_VDD_CORE,
		.dev_id = 82, /* AM6_DEV_CBASS0 */
		.clk_id = 0, /* main sysclk0 */
		.opp = AM6_OPP_NOM,
		.opps = {
			[AM6_OPP_NOM] = {
				.volt = 1000000,
				.freq = 250000000, /* CBASS0 */
			},
		},
	},
	{
		.id = AM6_VDD_MPU0,
		.dev_id = 202, /* AM6_DEV_COMPUTE_CLUSTER_A53_0 */
		.clk_id = 0, /* ARM clock */
		.opp = AM6_OPP_NOM,
		.opps = {
			[AM6_OPP_NOM] = {
				.volt = 1100000,
				.freq = 800000000,
			},
			[AM6_OPP_OD] = {
				.volt = 1200000,
				.freq = 1000000000,
			},
			[AM6_OPP_TURBO] = {
				.volt = 1240000,
				.freq = 1100000000,
			},
		},
	},
	{
		.id = AM6_VDD_MPU1,
		.opp = AM6_OPP_NOM,
		.dev_id = 204, /* AM6_DEV_COMPUTE_CLUSTER_A53_2 */
		.clk_id = 0, /* ARM clock */
		.opps = {
			[AM6_OPP_NOM] = {
				.volt = 1100000,
				.freq = 800000000,
			},
			[AM6_OPP_OD] = {
				.volt = 1200000,
				.freq = 1000000000,
			},
			[AM6_OPP_TURBO] = {
				.volt = 1240000,
				.freq = 1100000000,
			},
		},
	},
	{ .id = -1 },
};

static struct vd_data j721e_vd_data[] = {
	{
		.id = J721E_VDD_MPU,
		.opp = AM6_OPP_NOM,
		.dev_id = 202, /* J721E_DEV_A72SS0_CORE0 */
		.clk_id = 2, /* ARM clock */
		.opps = {
			[AM6_OPP_NOM] = {
				.volt = 880000, /* TBD in DM */
				.freq = 2000000000,
			},
		},
	},
	{ .id = -1 },
};

static struct vd_config j721e_vd_config = {
	.efuse_xlate = am6_efuse_xlate,
	.vds = j721e_vd_data,
};

static struct vd_config am654_vd_config = {
	.efuse_xlate = am6_efuse_xlate,
	.vds = am654_vd_data,
};

static const struct udevice_id k3_avs_ids[] = {
	{ .compatible = "ti,am654-avs", .data = (ulong)&am654_vd_config },
	{ .compatible = "ti,j721e-avs", .data = (ulong)&j721e_vd_config },
	{ .compatible = "ti,j721e-vtm", .data = (ulong)&j721e_vd_config },
	{ .compatible = "ti,j7200-vtm", .data = (ulong)&j721e_vd_config },
	{}
};

U_BOOT_DRIVER(k3_avs) = {
	.name = "k3_avs",
	.of_match = k3_avs_ids,
	.id = UCLASS_MISC,
	.probe = k3_avs_probe,
	.priv_auto	= sizeof(struct k3_avs_privdata),
};

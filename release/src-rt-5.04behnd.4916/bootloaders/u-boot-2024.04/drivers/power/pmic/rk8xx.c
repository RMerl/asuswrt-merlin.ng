// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2015 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <dm.h>
#include <dm/lists.h>
#include <errno.h>
#include <log.h>
#include <power/rk8xx_pmic.h>
#include <power/pmic.h>
#include <sysreset.h>

static int rk8xx_sysreset_request(struct udevice *dev, enum sysreset_t type)
{
	struct rk8xx_priv *priv = dev_get_priv(dev->parent);

	if (type != SYSRESET_POWER_OFF)
		return -EPROTONOSUPPORT;

	switch (priv->variant) {
	case RK805_ID:
	case RK808_ID:
	case RK816_ID:
	case RK818_ID:
		pmic_clrsetbits(dev->parent, REG_DEVCTRL, 0, BIT(0));
		break;
	case RK809_ID:
	case RK817_ID:
		pmic_clrsetbits(dev->parent, RK817_REG_SYS_CFG3, 0,
				BIT(0));
		break;
	default:
		printf("Unknown PMIC RK%x: Cannot shutdown\n",
		       priv->variant);
		return -EPROTONOSUPPORT;
	};

	return -EINPROGRESS;
}

static struct sysreset_ops rk8xx_sysreset_ops = {
	.request	= rk8xx_sysreset_request,
};

U_BOOT_DRIVER(rk8xx_sysreset) = {
	.name		= "rk8xx_sysreset",
	.id		= UCLASS_SYSRESET,
	.ops		= &rk8xx_sysreset_ops,
};

/* In the event of a plug-in and the appropriate option has been
 * selected, we simply shutdown instead of continue the normal boot
 * process. Please note the rk808 is not supported as it doesn't
 * have the appropriate register.
 */
void rk8xx_off_for_plugin(struct udevice *dev)
{
	struct rk8xx_priv *priv = dev_get_priv(dev);

	switch (priv->variant) {
	case RK805_ID:
	case RK816_ID:
	case RK818_ID:
		if (pmic_reg_read(dev, RK8XX_ON_SOURCE) & RK8XX_ON_PLUG_IN) {
			printf("Power Off due to plug-in event\n");
			pmic_clrsetbits(dev, REG_DEVCTRL, 0, BIT(0));
		}
		break;
	case RK809_ID:
	case RK817_ID:
		if (pmic_reg_read(dev, RK817_ON_SOURCE) & RK8XX_ON_PLUG_IN) {
			printf("Power Off due to plug-in event\n");
			pmic_clrsetbits(dev, RK817_REG_SYS_CFG3, 0,
					BIT(0));
		}
		break;
	default:
		printf("PMIC RK%x: Cannot read boot reason.\n",
		       priv->variant);
	}
}

static struct reg_data rk817_init_reg[] = {
/* enable the under-voltage protection,
 * the under-voltage protection will shutdown the LDO3 and reset the PMIC
 */
	{ RK817_BUCK4_CMIN, 0x60, 0x60},
};

static const struct pmic_child_info pmic_children_info[] = {
	{ .prefix = "DCDC_REG", .driver = "rk8xx_buck"},
	{ .prefix = "LDO_REG", .driver = "rk8xx_ldo"},
	{ .prefix = "SWITCH_REG", .driver = "rk8xx_switch"},
	{ },
};

static int rk8xx_reg_count(struct udevice *dev)
{
	return RK808_NUM_OF_REGS;
}

static int rk8xx_write(struct udevice *dev, uint reg, const uint8_t *buff,
			  int len)
{
	int ret;

	ret = dm_i2c_write(dev, reg, buff, len);
	if (ret) {
		debug("write error to device: %p register: %#x!\n", dev, reg);
		return ret;
	}

	return 0;
}

static int rk8xx_read(struct udevice *dev, uint reg, uint8_t *buff, int len)
{
	int ret;

	ret = dm_i2c_read(dev, reg, buff, len);
	if (ret) {
		debug("read error from device: %p register: %#x!\n", dev, reg);
		return ret;
	}

	return 0;
}

#if CONFIG_IS_ENABLED(PMIC_CHILDREN)
static int rk8xx_bind(struct udevice *dev)
{
	ofnode regulators_node;
	int children, ret;

	regulators_node = dev_read_subnode(dev, "regulators");
	if (!ofnode_valid(regulators_node)) {
		debug("%s: %s regulators subnode not found!\n", __func__,
		      dev->name);
		return -ENXIO;
	}

	debug("%s: '%s' - found regulators subnode\n", __func__, dev->name);

	if (CONFIG_IS_ENABLED(SYSRESET)) {
		ret = device_bind_driver_to_node(dev, "rk8xx_sysreset",
						 "rk8xx_sysreset",
						 dev_ofnode(dev), NULL);
		if (ret)
			return ret;
	}

	children = pmic_bind_children(dev, regulators_node, pmic_children_info);
	if (!children)
		debug("%s: %s - no child found\n", __func__, dev->name);

	if (IS_ENABLED(CONFIG_SPL_BUILD) &&
	    IS_ENABLED(CONFIG_ROCKCHIP_RK8XX_DISABLE_BOOT_ON_POWERON))
		dev_or_flags(dev, DM_FLAG_PROBE_AFTER_BIND);

	/* Always return success for this device */
	return 0;
}
#endif

static int rk8xx_probe(struct udevice *dev)
{
	struct rk8xx_priv *priv = dev_get_priv(dev);
	struct reg_data *init_data = NULL;
	int init_data_num = 0;
	int ret = 0, i, show_variant;
	u8 msb, lsb, id_msb, id_lsb;
	u8 on_source = 0, off_source = 0;
	u8 power_en0, power_en1, power_en2, power_en3;
	u8 value;

	/* read Chip variant */
	if (device_is_compatible(dev, "rockchip,rk817") ||
	    device_is_compatible(dev, "rockchip,rk809")) {
		id_msb = RK817_ID_MSB;
		id_lsb = RK817_ID_LSB;
	} else {
		id_msb = ID_MSB;
		id_lsb = ID_LSB;
	}

	ret = rk8xx_read(dev, id_msb, &msb, 1);
	if (ret)
		return ret;
	ret = rk8xx_read(dev, id_lsb, &lsb, 1);
	if (ret)
		return ret;

	priv->variant = ((msb << 8) | lsb) & RK8XX_ID_MSK;
	show_variant = priv->variant;
	switch (priv->variant) {
	case RK808_ID:
		show_variant = 0x808;	/* RK808 hardware ID is 0 */
		break;
	case RK805_ID:
	case RK816_ID:
	case RK818_ID:
		on_source = RK8XX_ON_SOURCE;
		off_source = RK8XX_OFF_SOURCE;
		break;
	case RK809_ID:
	case RK817_ID:
		on_source = RK817_ON_SOURCE;
		off_source = RK817_OFF_SOURCE;
		init_data = rk817_init_reg;
		init_data_num = ARRAY_SIZE(rk817_init_reg);
		power_en0 = pmic_reg_read(dev, RK817_POWER_EN0);
		power_en1 = pmic_reg_read(dev, RK817_POWER_EN1);
		power_en2 = pmic_reg_read(dev, RK817_POWER_EN2);
		power_en3 = pmic_reg_read(dev, RK817_POWER_EN3);

		value = (power_en0 & 0x0f) | ((power_en1 & 0x0f) << 4);
		pmic_reg_write(dev, RK817_POWER_EN_SAVE0, value);
		value = (power_en2 & 0x0f) | ((power_en3 & 0x0f) << 4);
		pmic_reg_write(dev, RK817_POWER_EN_SAVE1, value);
		break;
	default:
		printf("Unknown PMIC: RK%x!!\n", priv->variant);
		return -EINVAL;
	}

	for (i = 0; i < init_data_num; i++) {
		ret = pmic_clrsetbits(dev,
				      init_data[i].reg,
				      init_data[i].mask,
				      init_data[i].val);
		if (ret < 0) {
			printf("%s: i2c set reg 0x%x failed, ret=%d\n",
			       __func__, init_data[i].reg, ret);
		}

		debug("%s: reg[0x%x] = 0x%x\n", __func__, init_data[i].reg,
		      pmic_reg_read(dev, init_data[i].reg));
	}

	if (!IS_ENABLED(CONFIG_SPL_BUILD)) {
		printf("PMIC:  RK%x ", show_variant);
		if (on_source && off_source)
			printf("(on=0x%02x, off=0x%02x)",
			       pmic_reg_read(dev, on_source),
			       pmic_reg_read(dev, off_source));
		printf("\n");
	}

	if (IS_ENABLED(CONFIG_ROCKCHIP_RK8XX_DISABLE_BOOT_ON_POWERON))
		rk8xx_off_for_plugin(dev);

	return 0;
}

static struct dm_pmic_ops rk8xx_ops = {
	.reg_count = rk8xx_reg_count,
	.read = rk8xx_read,
	.write = rk8xx_write,
};

static const struct udevice_id rk8xx_ids[] = {
	{ .compatible = "rockchip,rk805" },
	{ .compatible = "rockchip,rk808" },
	{ .compatible = "rockchip,rk809" },
	{ .compatible = "rockchip,rk816" },
	{ .compatible = "rockchip,rk817" },
	{ .compatible = "rockchip,rk818" },
	{ }
};

U_BOOT_DRIVER(rockchip_rk805) = {
	.name = "rockchip_rk805",
	.id = UCLASS_PMIC,
	.of_match = rk8xx_ids,
#if CONFIG_IS_ENABLED(PMIC_CHILDREN)
	.bind = rk8xx_bind,
#endif
	.priv_auto	  = sizeof(struct rk8xx_priv),
	.probe = rk8xx_probe,
	.ops = &rk8xx_ops,
};

DM_DRIVER_ALIAS(rockchip_rk805, rockchip_rk808)

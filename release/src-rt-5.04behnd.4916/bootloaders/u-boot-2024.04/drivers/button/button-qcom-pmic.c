// SPDX-License-Identifier: GPL-2.0+
/*
 * Qualcomm generic pmic gpio driver
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 * (C) Copyright 2023 Linaro Ltd.
 */

#include <button.h>
#include <dt-bindings/input/linux-event-codes.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/lists.h>
#include <log.h>
#include <power/pmic.h>
#include <spmi/spmi.h>
#include <linux/bitops.h>

#define REG_TYPE		0x4
#define REG_SUBTYPE		0x5

struct qcom_pmic_btn_priv {
	u32 base;
	u32 status_bit;
	int code;
	struct udevice *pmic;
};

#define PON_INT_RT_STS                        0x10
#define KPDPWR_ON_INT_BIT                     0
#define RESIN_ON_INT_BIT                      1

#define NODE_IS_PWRKEY(node) (!strncmp(ofnode_get_name(node), "pwrkey", strlen("pwrkey")))
#define NODE_IS_RESIN(node) (!strncmp(ofnode_get_name(node), "resin", strlen("resin")))

static enum button_state_t qcom_pwrkey_get_state(struct udevice *dev)
{
	struct qcom_pmic_btn_priv *priv = dev_get_priv(dev);

	int reg = pmic_reg_read(priv->pmic, priv->base + PON_INT_RT_STS);

	if (reg < 0)
		return 0;

	return (reg & BIT(priv->status_bit)) != 0;
}

static int qcom_pwrkey_get_code(struct udevice *dev)
{
	struct qcom_pmic_btn_priv *priv = dev_get_priv(dev);

	return priv->code;
}

static int qcom_pwrkey_probe(struct udevice *dev)
{
	struct button_uc_plat *uc_plat = dev_get_uclass_plat(dev);
	struct qcom_pmic_btn_priv *priv = dev_get_priv(dev);
	ofnode node = dev_ofnode(dev);
	int ret;
	u64 base;

	/* Ignore the top-level pon node */
	if (!uc_plat->label)
		return 0;

	/* the pwrkey and resin nodes are children of the "pon" node, get the
	 * PMIC device to use in pmic_reg_* calls.
	 */
	priv->pmic = dev->parent->parent;

	/* Get the address of the parent pon node */
	base = dev_read_addr(dev->parent);
	if (base == FDT_ADDR_T_NONE) {
		printf("%s: Can't find address\n", dev->name);
		return -EINVAL;
	}

	priv->base = base;

	/* Do a sanity check */
	ret = pmic_reg_read(priv->pmic, priv->base + REG_TYPE);
	if (ret != 0x1 && ret != 0xb) {
		printf("%s: unexpected PMIC function type %d\n", dev->name, ret);
		return -ENXIO;
	}

	ret = pmic_reg_read(priv->pmic, priv->base + REG_SUBTYPE);
	if (ret < 0 || (ret & 0x7) == 0) {
		printf("%s: unexpected PMCI function subtype %d\n", dev->name, ret);
		return -ENXIO;
	}

	if (NODE_IS_PWRKEY(node)) {
		priv->status_bit = 0;
		priv->code = KEY_ENTER;
	} else if (NODE_IS_RESIN(node)) {
		priv->status_bit = 1;
		priv->code = KEY_DOWN;
	} else {
		/* Should not get here! */
		printf("Invalid pon node '%s' should be 'pwrkey' or 'resin'\n",
		       ofnode_get_name(node));
		return -EINVAL;
	}

	return 0;
}

static int button_qcom_pmic_bind(struct udevice *parent)
{
	struct udevice *dev;
	ofnode node;
	int ret;

	dev_for_each_subnode(node, parent) {
		struct button_uc_plat *uc_plat;
		const char *label;

		if (!ofnode_is_enabled(node))
			continue;

		ret = device_bind_driver_to_node(parent, "qcom_pwrkey",
						 ofnode_get_name(node),
						 node, &dev);
		if (ret) {
			printf("Failed to bind %s! %d\n", label, ret);
			return ret;
		}
		uc_plat = dev_get_uclass_plat(dev);
		if (NODE_IS_PWRKEY(node)) {
			uc_plat->label = "pwrkey";
		} else if (NODE_IS_RESIN(node)) {
			uc_plat->label = "vol_down";
		} else {
			debug("Unknown button node '%s' should be 'pwrkey' or 'resin'\n",
			       ofnode_get_name(node));
			device_unbind(dev);
		}
	}

	return 0;
}

static const struct button_ops button_qcom_pmic_ops = {
	.get_state	= qcom_pwrkey_get_state,
	.get_code	= qcom_pwrkey_get_code,
};

static const struct udevice_id qcom_pwrkey_ids[] = {
	{ .compatible = "qcom,pm8916-pon" },
	{ .compatible = "qcom,pm8941-pon" },
	{ .compatible = "qcom,pm8998-pon" },
	{ }
};

U_BOOT_DRIVER(qcom_pwrkey) = {
	.name = "qcom_pwrkey",
	.id = UCLASS_BUTTON,
	.of_match = qcom_pwrkey_ids,
	.bind = button_qcom_pmic_bind,
	.probe = qcom_pwrkey_probe,
	.ops = &button_qcom_pmic_ops,
	.priv_auto = sizeof(struct qcom_pmic_btn_priv),
};

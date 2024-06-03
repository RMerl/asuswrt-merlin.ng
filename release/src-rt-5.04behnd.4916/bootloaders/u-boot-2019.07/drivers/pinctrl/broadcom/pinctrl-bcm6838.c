// SPDX-License-Identifier: GPL-2.0

#include <common.h>
#include <dm.h>
#include <regmap.h>
#include <syscon.h>
#include <dm/pinctrl.h>

#define BCM6838_CMD_LOAD_MUX            0x21

#define BCM6838_FUNC_OFFS               12
#define BCM6838_FUNC_MASK               (0x37 << BCM6838_FUNC_OFFS)
#define BCM6838_PIN_OFFS                 0
#define BCM6838_PIN_MASK                (0xfff << BCM6838_PIN_OFFS)

#define BCM6838_MAX_PIN_NAME_LEN         8
static char bcm6838_pin_name[BCM6838_MAX_PIN_NAME_LEN];

#define BCM6838_MAX_FUNC_NAME_LEN        8
static char bcm6838_func_name[BCM6838_MAX_FUNC_NAME_LEN];

struct bcm6838_test_port_hw {
	unsigned long port_blk_data1;
	unsigned long port_blk_data2;
	unsigned long port_command;
};

static const struct bcm6838_test_port_hw bcm6838_hw = {
	.port_blk_data1 = 0x10,
	.port_blk_data2 = 0x14,
	.port_command   = 0x18
};

struct bcm6838_pinctrl_priv {
	const struct bcm6838_test_port_hw *hw;
	struct regmap *regmap;
	u32 pins_count;
	u32 functions_count;
};

int bcm6838_pinctrl_get_pins_count(struct udevice *dev)
{
	struct bcm6838_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->pins_count;
}

const char *bcm6838_pinctrl_get_pin_name(struct udevice *dev,
					 unsigned int selector)
{
	snprintf(bcm6838_pin_name, BCM6838_MAX_PIN_NAME_LEN, "%u", selector);
	return bcm6838_pin_name;
}

int bcm6838_pinctrl_get_functions_count(struct udevice *dev)
{
	struct bcm6838_pinctrl_priv *priv = dev_get_priv(dev);

	return priv->functions_count;
}

const char *bcm6838_pinctrl_get_function_name(struct udevice *dev,
					      unsigned int selector)
{
	snprintf(bcm6838_func_name, BCM6838_MAX_FUNC_NAME_LEN, "%u", selector);
	return bcm6838_func_name;
}

int bcm6838_pinctrl_pinmux_set(struct udevice *dev,
			       unsigned int pin_selector,
			       unsigned int func_selector)
{
	struct bcm6838_pinctrl_priv *priv = dev_get_priv(dev);
	const struct bcm6838_test_port_hw *hw = priv->hw;
	unsigned int data;

	regmap_write(priv->regmap, hw->port_blk_data1, 0);
	data = (func_selector << BCM6838_FUNC_OFFS) & BCM6838_FUNC_MASK;
	data |= (pin_selector << BCM6838_PIN_OFFS) & BCM6838_PIN_MASK;
	regmap_write(priv->regmap, hw->port_blk_data2, data);
	regmap_write(priv->regmap, hw->port_command, BCM6838_CMD_LOAD_MUX);

	return 0;
}

int bcm6838_pinctrl_probe(struct udevice *dev)
{
	struct bcm6838_pinctrl_priv *priv = dev_get_priv(dev);
	const struct bcm6838_test_port_hw *hw =
		(const struct bcm6838_test_port_hw *)dev_get_driver_data(dev);
	int err;
	u32 phandle;
	ofnode node;

	err = ofnode_read_u32(dev_ofnode(dev), "regmap", &phandle);
	if (err) {
		dev_err(dev, "%s: unable to read regmap\n", __func__);
		goto out;
	}

	node = ofnode_get_by_phandle(phandle);
	if (!ofnode_valid(node)) {
		dev_err(dev, "%s: unable to find node\n", __func__);
		err = -EINVAL;
		goto out;
	}

	priv->regmap = syscon_node_to_regmap(node);
	if (!priv->regmap) {
		dev_err(dev, "%s: unable to find regmap\n", __func__);
		err = -ENODEV;
		goto out;
	}

	err = ofnode_read_u32(dev_ofnode(dev), "brcm,pins-count",
			      &priv->pins_count);
	if (err) {
		dev_err(dev, "%s: unable to read brcm,pins-count\n",
			__func__);
		goto out;
	}

	err = ofnode_read_u32(dev_ofnode(dev), "brcm,functions-count",
			      &priv->functions_count);
	if (err) {
		dev_err(dev, "%s: unable to read brcm,functions-count\n",
			__func__);
		goto out;
	}

	priv->hw = hw;

 out:
	return err;
}

const struct pinctrl_ops bcm6838_pinctrl_ops = {
	.set_state = pinctrl_generic_set_state,
	.get_pins_count = bcm6838_pinctrl_get_pins_count,
	.get_pin_name = bcm6838_pinctrl_get_pin_name,
	.get_functions_count = bcm6838_pinctrl_get_functions_count,
	.get_function_name = bcm6838_pinctrl_get_function_name,
	.pinmux_set = bcm6838_pinctrl_pinmux_set,
};

static const struct udevice_id bcm6838_pinctrl_match[] = {
	{
		.compatible = "brcm,bcm6838-pinctrl",
		.data = (ulong)&bcm6838_hw,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(bcm6838_pinctrl) = {
	.name = "bcm6838_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = bcm6838_pinctrl_match,
	.ops = &bcm6838_pinctrl_ops,
	.priv_auto_alloc_size = sizeof(struct bcm6838_pinctrl_priv),
	.probe = bcm6838_pinctrl_probe,
};

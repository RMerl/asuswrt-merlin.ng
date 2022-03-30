// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 Google, Inc
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/arch/pinctrl.h>
#include <asm/arch/scu_ast2500.h>
#include <dm/pinctrl.h>

/*
 * This driver works with very simple configuration that has the same name
 * for group and function. This way it is compatible with the Linux Kernel
 * driver.
 */

struct ast2500_pinctrl_priv {
	struct ast2500_scu *scu;
};

static int ast2500_pinctrl_probe(struct udevice *dev)
{
	struct ast2500_pinctrl_priv *priv = dev_get_priv(dev);

	priv->scu = ast_get_scu();

	return 0;
}

struct ast2500_group_config {
	char *group_name;
	/* Control register number (1-10) */
	unsigned reg_num;
	/* The mask of control bits in the register */
	u32 ctrl_bit_mask;
};

static const struct ast2500_group_config ast2500_groups[] = {
	{ "I2C1", 8, (1 << 13) | (1 << 12) },
	{ "I2C2", 8, (1 << 15) | (1 << 14) },
	{ "I2C3", 8, (1 << 16) },
	{ "I2C4", 5, (1 << 17) },
	{ "I2C4", 5, (1 << 17) },
	{ "I2C5", 5, (1 << 18) },
	{ "I2C6", 5, (1 << 19) },
	{ "I2C7", 5, (1 << 20) },
	{ "I2C8", 5, (1 << 21) },
	{ "I2C9", 5, (1 << 22) },
	{ "I2C10", 5, (1 << 23) },
	{ "I2C11", 5, (1 << 24) },
	{ "I2C12", 5, (1 << 25) },
	{ "I2C13", 5, (1 << 26) },
	{ "I2C14", 5, (1 << 27) },
	{ "MAC1LINK", 1, (1 << 0) },
	{ "MDIO1", 3, (1 << 31) | (1 << 30) },
	{ "MAC2LINK", 1, (1 << 1) },
	{ "MDIO2", 5, (1 << 2) },
};

static int ast2500_pinctrl_get_groups_count(struct udevice *dev)
{
	debug("PINCTRL: get_(functions/groups)_count\n");

	return ARRAY_SIZE(ast2500_groups);
}

static const char *ast2500_pinctrl_get_group_name(struct udevice *dev,
						  unsigned selector)
{
	debug("PINCTRL: get_(function/group)_name %u\n", selector);

	return ast2500_groups[selector].group_name;
}

static int ast2500_pinctrl_group_set(struct udevice *dev, unsigned selector,
				     unsigned func_selector)
{
	struct ast2500_pinctrl_priv *priv = dev_get_priv(dev);
	const struct ast2500_group_config *config;
	u32 *ctrl_reg;

	debug("PINCTRL: group_set <%u, %u>\n", selector, func_selector);
	if (selector >= ARRAY_SIZE(ast2500_groups))
		return -EINVAL;

	config = &ast2500_groups[selector];
	if (config->reg_num > 6)
		ctrl_reg = &priv->scu->pinmux_ctrl1[config->reg_num - 7];
	else
		ctrl_reg = &priv->scu->pinmux_ctrl[config->reg_num - 1];

	ast_scu_unlock(priv->scu);
	setbits_le32(ctrl_reg, config->ctrl_bit_mask);
	ast_scu_lock(priv->scu);

	return 0;
}

static struct pinctrl_ops ast2500_pinctrl_ops = {
	.set_state = pinctrl_generic_set_state,
	.get_groups_count = ast2500_pinctrl_get_groups_count,
	.get_group_name = ast2500_pinctrl_get_group_name,
	.get_functions_count = ast2500_pinctrl_get_groups_count,
	.get_function_name = ast2500_pinctrl_get_group_name,
	.pinmux_group_set = ast2500_pinctrl_group_set,
};

static const struct udevice_id ast2500_pinctrl_ids[] = {
	{ .compatible = "aspeed,ast2500-pinctrl" },
	{ .compatible = "aspeed,g5-pinctrl" },
	{ }
};

U_BOOT_DRIVER(pinctrl_ast2500) = {
	.name = "aspeed_ast2500_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = ast2500_pinctrl_ids,
	.priv_auto_alloc_size = sizeof(struct ast2500_pinctrl_priv),
	.ops = &ast2500_pinctrl_ops,
	.probe = ast2500_pinctrl_probe,
};

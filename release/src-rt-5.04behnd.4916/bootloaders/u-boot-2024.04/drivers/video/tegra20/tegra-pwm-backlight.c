// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022 Svyatoslav Ryhel <clamor95@gmail.com>
 */

#define LOG_CATEGORY UCLASS_PANEL_BACKLIGHT

#include <backlight.h>
#include <common.h>
#include <dm.h>
#include <i2c.h>
#include <log.h>
#include <linux/delay.h>
#include <linux/err.h>

#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/display.h>

#define TEGRA_DISPLAY_A_BASE		0x54200000
#define TEGRA_DISPLAY_B_BASE		0x54240000

#define TEGRA_PWM_BL_MIN_BRIGHTNESS	0x10
#define TEGRA_PWM_BL_MAX_BRIGHTNESS	0xFF

#define TEGRA_PWM_BL_PERIOD		0xFF
#define TEGRA_PWM_BL_CLK_DIV		0x14
#define TEGRA_PWM_BL_CLK_SELECT		0x00

#define PM_PERIOD_SHIFT                 18
#define PM_CLK_DIVIDER_SHIFT		4

#define TEGRA_PWM_PM0			0
#define TEGRA_PWM_PM1			1

struct tegra_pwm_backlight_priv {
	struct dc_ctlr *dc;		/* Display controller regmap */

	u32 pwm_source;
	u32 period;
	u32 clk_div;
	u32 clk_select;
	u32 dft_brightness;
};

static int tegra_pwm_backlight_set_brightness(struct udevice *dev, int percent)
{
	struct tegra_pwm_backlight_priv *priv = dev_get_priv(dev);
	struct dc_cmd_reg *cmd = &priv->dc->cmd;
	struct dc_com_reg *com = &priv->dc->com;
	unsigned int ctrl;
	unsigned long out_sel;
	unsigned long cmd_state;

	if (percent == BACKLIGHT_DEFAULT)
		percent = priv->dft_brightness;

	if (percent < TEGRA_PWM_BL_MIN_BRIGHTNESS)
		percent = TEGRA_PWM_BL_MIN_BRIGHTNESS;

	if (percent > TEGRA_PWM_BL_MAX_BRIGHTNESS)
		percent = TEGRA_PWM_BL_MAX_BRIGHTNESS;

	ctrl = ((priv->period << PM_PERIOD_SHIFT) |
		(priv->clk_div << PM_CLK_DIVIDER_SHIFT) |
		 priv->clk_select);

	/* The new value should be effected immediately */
	cmd_state = readl(&cmd->state_access);
	writel((cmd_state | (1 << 2)), &cmd->state_access);

	switch (priv->pwm_source) {
	case TEGRA_PWM_PM0:
		/* Select the LM0 on PM0 */
		out_sel = readl(&com->pin_output_sel[5]);
		out_sel &= ~(7 << 0);
		out_sel |= (3 << 0);
		writel(out_sel, &com->pin_output_sel[5]);
		writel(ctrl, &com->pm0_ctrl);
		writel(percent, &com->pm0_duty_cycle);
		break;
	case TEGRA_PWM_PM1:
		/* Select the LM1 on PM1 */
		out_sel = readl(&com->pin_output_sel[5]);
		out_sel &= ~(7 << 4);
		out_sel |= (3 << 4);
		writel(out_sel, &com->pin_output_sel[5]);
		writel(ctrl, &com->pm1_ctrl);
		writel(percent, &com->pm1_duty_cycle);
		break;
	default:
		break;
	}

	writel(cmd_state, &cmd->state_access);
	return 0;
}

static int tegra_pwm_backlight_enable(struct udevice *dev)
{
	struct tegra_pwm_backlight_priv *priv = dev_get_priv(dev);

	return tegra_pwm_backlight_set_brightness(dev, priv->dft_brightness);
}

static int tegra_pwm_backlight_probe(struct udevice *dev)
{
	struct tegra_pwm_backlight_priv *priv = dev_get_priv(dev);

	if (dev_read_bool(dev, "nvidia,display-b-base"))
		priv->dc = (struct dc_ctlr *)TEGRA_DISPLAY_B_BASE;
	else
		priv->dc = (struct dc_ctlr *)TEGRA_DISPLAY_A_BASE;

	if (!priv->dc) {
		log_err("no display controller address\n");
		return -EINVAL;
	}

	priv->pwm_source =
		dev_read_u32_default(dev, "nvidia,pwm-source",
				     TEGRA_PWM_PM0);
	priv->period =
		dev_read_u32_default(dev, "nvidia,period",
				     TEGRA_PWM_BL_PERIOD);
	priv->clk_div =
		dev_read_u32_default(dev, "nvidia,clock-div",
				     TEGRA_PWM_BL_CLK_DIV);
	priv->clk_select =
		dev_read_u32_default(dev, "nvidia,clock-select",
				     TEGRA_PWM_BL_CLK_SELECT);
	priv->dft_brightness =
		dev_read_u32_default(dev, "nvidia,default-brightness",
				     TEGRA_PWM_BL_MAX_BRIGHTNESS);

	return 0;
}

static const struct backlight_ops tegra_pwm_backlight_ops = {
	.enable = tegra_pwm_backlight_enable,
	.set_brightness = tegra_pwm_backlight_set_brightness,
};

static const struct udevice_id tegra_pwm_backlight_ids[] = {
	{ .compatible = "nvidia,tegra-pwm-backlight" },
	{ }
};

U_BOOT_DRIVER(tegra_pwm_backlight) = {
	.name		= "tegra_pwm_backlight",
	.id		= UCLASS_PANEL_BACKLIGHT,
	.of_match	= tegra_pwm_backlight_ids,
	.probe		= tegra_pwm_backlight_probe,
	.ops		= &tegra_pwm_backlight_ops,
	.priv_auto	= sizeof(struct tegra_pwm_backlight_priv),
};

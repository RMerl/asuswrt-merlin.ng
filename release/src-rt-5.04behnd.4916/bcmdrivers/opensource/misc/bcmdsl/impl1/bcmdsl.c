/*
   Copyright (c) 2021 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2021:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.
    :> 
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/gpio/consumer.h>
#include "bcmdsl.h"

enum {
	AFE_CTL_MODE,
	AFE_CTL_PWR,
	AFE_CTL_DATA,
	AFE_CTL_CLK,
	AFE_CTL_MAX,
};

static const char *afe_ctl_names[] = {
    "afe-mode-ctl",
    "afe-pwr-ctl",
    "afe-data-ctl",
    "afe-clk-ctl",
};

enum {
	GPIO_PWRBOOST,
	GPIO_RELAY,
	GPIO_RESET,
	GPIO_VR5P3PWR,
	GPIO_TDDENABLE,
	GPIO_MAX,
};

static const char *gpio_names[] = {
    "pwrboost",
    "relay",
    "reset",
    "vr5p3pwr",
    "tddenable",
};

struct bcmdsl_line {
	struct device *dev;
	uint32_t afeid;
	uint32_t sec_afeid;
	uint32_t afe_ctl[AFE_CTL_MAX];
	struct gpio_desc *gpiod[GPIO_MAX];
	uint32_t gpio[GPIO_MAX];
};

static struct bcmdsl_line dsl_lines[BCMDSL_MAX_LINES];

struct bcmdsl_ctrl_priv {
	struct device *dev;
	void __iomem  *phy_base;
	void __iomem  *lmem_base;
	void __iomem  *xmem_base;
	int irq;
};

static struct bcmdsl_ctrl_priv ctrl_priv;

static int bcm_dsl_probe(struct platform_device *pdev)
{
	struct device_node *dn = pdev->dev.of_node;
	struct device *dev = &pdev->dev;
	const __be32 *reg;
	struct bcmdsl_line *dsl;
	int line = 0, i;

	if (!dn || !dev)
		return -ENODEV;

	reg = of_get_property(dn, "reg", NULL);
	if (!reg) {
		dev_err(dev, "missing reg property\n");
		return -EINVAL;
	}

	line = of_read_number(reg, of_n_addr_cells(dn));
	if (line > BCMDSL_MAX_LINES) {
		dev_err(dev, "invalid dsl line number %d\n", line);
		return -EINVAL;
	}

	dsl = &dsl_lines[line];
	memset(dsl, BCMDSL_INVALID_VALUE, sizeof(struct bcmdsl_line));
	dsl->dev = dev;

	if (of_property_read_u32(dn, "afeid", &dsl->afeid))
		dsl->afeid = BCMDSL_DEFAULT_VALUE;
	if (of_property_read_u32(dn, "secondary-afeid", &dsl->sec_afeid))
		dsl->sec_afeid = BCMDSL_DEFAULT_VALUE;

	for (i = 0; i < AFE_CTL_MAX; i++) {
		of_property_read_u32(dn, afe_ctl_names[i], &dsl->afe_ctl[i]);
		dev_info(dev, "line %d afe_ctl %s %d\n",
			line, afe_ctl_names[i], dsl->afe_ctl[i]);
	}

	for (i = 0; i < GPIO_MAX; i++ ) {
		dsl->gpiod[i] = devm_gpiod_get_optional(dev, gpio_names[i], GPIOD_IN);
		if (!IS_ERR_OR_NULL(dsl->gpiod[i])) {
			dsl->gpio[i] = desc_to_gpio(dsl->gpiod[i]);
			/* specail case for pwrboost. the same gpio can be defined in */
			/* both lines for bonding so put it back and request again when needed */
			if (i == GPIO_PWRBOOST)
				devm_gpiod_put(dev, dsl->gpiod[i]);
		} else if (dsl->gpiod[i] == ERR_PTR(-EPROBE_DEFER))
			return -EPROBE_DEFER;

		dev_info(dev, "line %d gpio %s %d\n",
			line, gpio_names[i], dsl->gpio[i]);
	}
	
	dev_info(dev, "line %d afe 0x%x sec afe 0x%x\n",
			line, dsl->afeid, dsl->sec_afeid);
		
	return 0;
}

static int bcmdsl_get_afe_ctl(int line, int ctl_id, uint16_t *afe_ctl)
{
  	struct bcmdsl_line *dsl;

	if (line >= BCMDSL_MAX_LINES || ctl_id >= AFE_CTL_MAX)
		return BCMDSL_INVALID;

	dsl = &dsl_lines[line];
	*afe_ctl = (uint16_t)dsl->afe_ctl[ctl_id];
	if (dsl->afe_ctl[ctl_id] == BCMDSL_INVALID_VALUE)
		return BCMDSL_NOT_DEFINED;
	else
		return BCMDSL_SUCCESS;
}


static int bcmdsl_set_gpio(int line, int gpio_id, int active)
{
	struct bcmdsl_line *dsl;
	int ret;

	if (line >= BCMDSL_MAX_LINES || gpio_id >= GPIO_MAX)
		return BCMDSL_INVALID;

	dsl = &dsl_lines[line];
	/* pwrboot gpio can be shared between lines.. need to request/put back every time */
	if (gpio_id == GPIO_PWRBOOST) {
		dsl->gpiod[gpio_id] = devm_gpiod_get_optional(dsl->dev, gpio_names[gpio_id], 0);
		if (IS_ERR_OR_NULL(dsl->gpiod[gpio_id]))
			return BCMDSL_NOT_DEFINED;
	}

	ret = gpiod_direction_output(dsl->gpiod[gpio_id], active);
	if (gpio_id == GPIO_PWRBOOST)
		devm_gpiod_put(dsl->dev, dsl->gpiod[gpio_id]);
	if (ret) {
		dev_err(dsl->dev, "bcmdsl_set_gpio %s %d for line %d failed ret %d\n",
			gpio_names[gpio_id], dsl->gpio[gpio_id], line, ret);
		return BCMDSL_SET_GPIO_FAIL;
	}
	else
		return BCMDSL_SUCCESS;
}

static int bcmdsl_get_gpio_num(int line, int gpio_id, uint16_t* gpio)
{
	struct bcmdsl_line *dsl;

	if (line >= BCMDSL_MAX_LINES || gpio_id >= GPIO_MAX)
		return BCMDSL_INVALID;

	dsl = &dsl_lines[line];
	*gpio = (uint16_t)dsl->gpio[gpio_id];
	if (dsl->gpio[gpio_id] == BCMDSL_INVALID_VALUE)
		return BCMDSL_NOT_DEFINED;
	else
		return BCMDSL_SUCCESS; 
}
  
int bcmdsl_get_afe_ids(uint32_t *afe_ids)
{
	int i;
	struct bcmdsl_line *dsl;

	for (i = 0; i < BCMDSL_MAX_LINES; i++) {
		dsl = &dsl_lines[i];
		afe_ids[i] = dsl->afeid;
	}

	return BCMDSL_SUCCESS;
}
EXPORT_SYMBOL(bcmdsl_get_afe_ids);

int bcmdsl_get_primary_afe_id(int line, uint32_t *afe_id)
{
	if (line >= BCMDSL_MAX_LINES)
		return BCMDSL_INVALID;

	*afe_id = dsl_lines[line].afeid;

	return BCMDSL_SUCCESS;
}
EXPORT_SYMBOL(bcmdsl_get_primary_afe_id);

int bcmdsl_get_secondary_afe_id(int line, uint32_t *afe_id)
{
	if (line >= BCMDSL_MAX_LINES)
		return BCMDSL_INVALID;

	*afe_id = dsl_lines[line].sec_afeid;

	return BCMDSL_SUCCESS;
}
EXPORT_SYMBOL(bcmdsl_get_secondary_afe_id);

int bcmdsl_get_afe_pwr_ctl(int line, uint16_t *pwr_ctl)
{
	return bcmdsl_get_afe_ctl(line, AFE_CTL_PWR, pwr_ctl);
}
EXPORT_SYMBOL(bcmdsl_get_afe_pwr_ctl);

int bcmdsl_get_afe_data_ctl(int line, uint16_t *data_ctl)
{
	return bcmdsl_get_afe_ctl(line, AFE_CTL_DATA, data_ctl);
}
EXPORT_SYMBOL(bcmdsl_get_afe_data_ctl);

int bcmdsl_get_afe_clk_ctl(int line, uint16_t *clk_ctl)
{
	return bcmdsl_get_afe_ctl(line, AFE_CTL_CLK, clk_ctl);
}
EXPORT_SYMBOL(bcmdsl_get_afe_clk_ctl);

int bcmdsl_get_afe_mode_ctl(int line, uint16_t *mode_ctl)
{
	return bcmdsl_get_afe_ctl(line, AFE_CTL_MODE, mode_ctl); 
}
EXPORT_SYMBOL(bcmdsl_get_afe_mode_ctl);

int bcmdsl_set_relay_gpio(int line, int active)
{
	return bcmdsl_set_gpio(line, GPIO_RELAY, active);
}
EXPORT_SYMBOL(bcmdsl_set_relay_gpio);

int bcmdsl_set_reset_gpio(int line, int active)
{
	return bcmdsl_set_gpio(line, GPIO_RESET, active);
}
EXPORT_SYMBOL(bcmdsl_set_reset_gpio);

int bcmdsl_set_vr5p3pwr_gpio(int line, int active)
{
	return bcmdsl_set_gpio(line, GPIO_VR5P3PWR, active);
}
EXPORT_SYMBOL(bcmdsl_set_vr5p3pwr_gpio);

int bcmdsl_set_pwrboost_gpio(int line, int active)
{
	return bcmdsl_set_gpio(line, GPIO_PWRBOOST, active);
}
EXPORT_SYMBOL(bcmdsl_set_pwrboost_gpio);

int bcmdsl_set_tddenable_gpio(int line, int active)
{
	return bcmdsl_set_gpio(line, GPIO_TDDENABLE, active);
}
EXPORT_SYMBOL(bcmdsl_set_tddenable_gpio);

int bcmdsl_get_pwrboost_gpio_num(int line, uint16_t* gpio)
{
	return bcmdsl_get_gpio_num(line, GPIO_PWRBOOST, gpio);
}
EXPORT_SYMBOL(bcmdsl_get_pwrboost_gpio_num);

int bcmdsl_get_relay_gpio_num(int line, uint16_t* gpio)
{
	return bcmdsl_get_gpio_num(line, GPIO_RELAY, gpio);
}
EXPORT_SYMBOL(bcmdsl_get_relay_gpio_num);

int bcmdsl_get_reset_gpio_num(int line, uint16_t* gpio)
{
	return bcmdsl_get_gpio_num(line, GPIO_RESET, gpio);
}
EXPORT_SYMBOL(bcmdsl_get_reset_gpio_num);

int bcmdsl_get_vr5p3pwr_gpio_num(int line, uint16_t* gpio)
{
	return bcmdsl_get_gpio_num(line, GPIO_VR5P3PWR, gpio);
}
EXPORT_SYMBOL(bcmdsl_get_vr5p3pwr_gpio_num);

int bcmdsl_get_tddenable_gpio_num(int line, uint16_t* gpio)
{
	return bcmdsl_get_gpio_num(line, GPIO_TDDENABLE, gpio);
}
EXPORT_SYMBOL(bcmdsl_get_tddenable_gpio_num);

static const struct of_device_id bcm_dsl_of_match[] = {
	{ .compatible = "brcm,dsl", .data = NULL, },
	{},
};
MODULE_DEVICE_TABLE(of, bcm_dsl_of_match);


static struct platform_driver bcm_dsl_dt_driver = {
	.probe = bcm_dsl_probe,
	.driver = {
		.name = "bcm-dsl-dt",
		.of_match_table = bcm_dsl_of_match,
	},
};

static int __init bcm_dsl_drv_reg(void)
{
	return platform_driver_register(&bcm_dsl_dt_driver);
}

subsys_initcall_sync(bcm_dsl_drv_reg);

static void bcm_dsl_ctrl_free(struct bcmdsl_ctrl_priv* priv)
{
	if (priv->phy_base)
		iounmap(priv->phy_base);

	if (priv->lmem_base)
		iounmap(priv->lmem_base);

	if (priv->xmem_base)
		iounmap(priv->xmem_base);
}

static int bcm_dsl_ctrl_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct resource *res;
	u32 ret = 0;

	ctrl_priv.dev = dev;

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dsl-phy");
	if (!res) {
		dev_err(dev, "Failed to find dsl-phy resource\n");
		ret = -EINVAL;
		goto error;
	}
	ctrl_priv.phy_base = devm_ioremap_resource(dev, res);
	if (IS_ERR_OR_NULL(ctrl_priv.phy_base)) {
		dev_err(dev, "Failed to map the dsl-phy resource\n");
		ret = -ENXIO;
		ctrl_priv.phy_base = NULL;
		goto error;
	}
	dev_info(dev, "dsl-phy  %pr\n", res);
	dev_info(dev, "virt addr 0x%px\n", ctrl_priv.phy_base);
	
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dsl-lmem");
	if (!res) {
		dev_err(dev, "Failed to find dsl-lmem resource\n");
		ret = -EINVAL;
		goto error;
	}
	ctrl_priv.lmem_base = devm_ioremap_resource(dev, res);
	if (IS_ERR_OR_NULL(ctrl_priv.lmem_base)) {
		dev_err(dev, "Failed to map the dsl-lmem resource\n");
		ret = -ENXIO;
		ctrl_priv.lmem_base = NULL;
		goto error;
	}
	dev_info(dev, "dsl-lmem %pr\n", res);
	dev_info(dev, "virt addr 0x%px\n", ctrl_priv.lmem_base);

	 /* optional resource for dsl xmem register */
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "dsl-xmem");
	if (res) {
		ctrl_priv.xmem_base = devm_ioremap_resource(dev, res);
		if (IS_ERR_OR_NULL(ctrl_priv.xmem_base)) {
			dev_err(dev, "Failed to map the dsl-xmem resource\n");
			ret = -ENXIO;
			ctrl_priv.xmem_base = NULL;
			goto error;
		}
		dev_info(dev, "dsl-xmem %pr\n", res);
		dev_info(dev, "virt addr 0x%px\n", ctrl_priv.xmem_base);		
	} else
		ctrl_priv.xmem_base = NULL;

	ctrl_priv.irq = platform_get_irq(pdev, 0);
	if (ctrl_priv.irq < 0) {
		dev_err(dev, "Failed to find dsl interrupt defintion\n");
		ret = -EINVAL;
		goto error;
	}

	dev_info(dev, "dsl interrupt %d\n", ctrl_priv.irq);
	return ret;

error:
	bcm_dsl_ctrl_free(&ctrl_priv);
	return ret;
}

static const struct of_device_id bcm_dsl_ctrl_of_match[] = {
	{ .compatible = "brcm,dsl-ctrl", .data = NULL, },
	{},
};
MODULE_DEVICE_TABLE(of, bcm_dsl_ctrl_of_match);

static struct platform_driver bcm_dsl_ctrl_driver = {
	.probe = bcm_dsl_ctrl_probe,
	.driver = {
		.name = "bcm-dsl-ctrl",
		.of_match_table = bcm_dsl_ctrl_of_match,
	},
};

static int __init bcm_dsl_ctrl_reg(void)
{
	return platform_driver_register(&bcm_dsl_ctrl_driver);
}

subsys_initcall(bcm_dsl_ctrl_reg);

uintptr_t bcmdsl_get_phy_base(void)
{
	return (uintptr_t)ctrl_priv.phy_base;
}
EXPORT_SYMBOL(bcmdsl_get_phy_base);

uintptr_t bcmdsl_get_lmem_base(void)
{
	return (uintptr_t)ctrl_priv.lmem_base;
}
EXPORT_SYMBOL(bcmdsl_get_lmem_base);

uintptr_t bcmdsl_get_xmem_base(void)
{
	return (uintptr_t)ctrl_priv.xmem_base;
}
EXPORT_SYMBOL(bcmdsl_get_xmem_base);

int bcmdsl_get_irq(void)
{
	return ctrl_priv.irq;
}
EXPORT_SYMBOL(bcmdsl_get_irq);

MODULE_DESCRIPTION("Broadcom BCA DSL Driver Open Portion");
MODULE_LICENSE("GPL v2");

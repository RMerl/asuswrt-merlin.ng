/*
 * Copyright (c) 2014 Linaro Ltd.
 * Copyright (c) 2014 Hisilicon Limited.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/regmap.h>
#include <media/rc-core.h>

#define IR_ENABLE		0x00
#define IR_CONFIG		0x04
#define CNT_LEADS		0x08
#define CNT_LEADE		0x0c
#define CNT_SLEADE		0x10
#define CNT0_B			0x14
#define CNT1_B			0x18
#define IR_BUSY			0x1c
#define IR_DATAH		0x20
#define IR_DATAL		0x24
#define IR_INTM			0x28
#define IR_INTS			0x2c
#define IR_INTC			0x30
#define IR_START		0x34

/* interrupt mask */
#define INTMS_SYMBRCV		(BIT(24) | BIT(8))
#define INTMS_TIMEOUT		(BIT(25) | BIT(9))
#define INTMS_OVERFLOW		(BIT(26) | BIT(10))
#define INT_CLR_OVERFLOW	BIT(18)
#define INT_CLR_TIMEOUT		BIT(17)
#define INT_CLR_RCV		BIT(16)
#define INT_CLR_RCVTIMEOUT	(BIT(16) | BIT(17))

#define IR_CLK			0x48
#define IR_CLK_ENABLE		BIT(4)
#define IR_CLK_RESET		BIT(5)

#define IR_CFG_WIDTH_MASK	0xffff
#define IR_CFG_WIDTH_SHIFT	16
#define IR_CFG_FORMAT_MASK	0x3
#define IR_CFG_FORMAT_SHIFT	14
#define IR_CFG_INT_LEVEL_MASK	0x3f
#define IR_CFG_INT_LEVEL_SHIFT	8
/* only support raw mode */
#define IR_CFG_MODE_RAW		BIT(7)
#define IR_CFG_FREQ_MASK	0x7f
#define IR_CFG_FREQ_SHIFT	0
#define IR_CFG_INT_THRESHOLD	1
/* symbol start from low to high, symbol stream end at high*/
#define IR_CFG_SYMBOL_FMT	0
#define IR_CFG_SYMBOL_MAXWIDTH	0x3e80

#define IR_HIX5HD2_NAME		"hix5hd2-ir"

struct hix5hd2_ir_priv {
	int			irq;
	void volatile __iomem	*base;
	struct device		*dev;
	struct rc_dev		*rdev;
	struct regmap		*regmap;
	struct clk		*clock;
	unsigned long		rate;
};

static void hix5hd2_ir_enable(struct hix5hd2_ir_priv *dev, bool on)
{
	u32 val;

	regmap_read(dev->regmap, IR_CLK, &val);
	if (on) {
		val &= ~IR_CLK_RESET;
		val |= IR_CLK_ENABLE;
	} else {
		val &= ~IR_CLK_ENABLE;
		val |= IR_CLK_RESET;
	}
	regmap_write(dev->regmap, IR_CLK, val);
}

static int hix5hd2_ir_config(struct hix5hd2_ir_priv *priv)
{
	int timeout = 10000;
	u32 val, rate;

	writel_relaxed(0x01, priv->base + IR_ENABLE);
	while (readl_relaxed(priv->base + IR_BUSY)) {
		if (timeout--) {
			udelay(1);
		} else {
			dev_err(priv->dev, "IR_BUSY timeout\n");
			return -ETIMEDOUT;
		}
	}

	/* Now only support raw mode, with symbol start from low to high */
	rate = DIV_ROUND_CLOSEST(priv->rate, 1000000);
	val = IR_CFG_SYMBOL_MAXWIDTH & IR_CFG_WIDTH_MASK << IR_CFG_WIDTH_SHIFT;
	val |= IR_CFG_SYMBOL_FMT & IR_CFG_FORMAT_MASK << IR_CFG_FORMAT_SHIFT;
	val |= (IR_CFG_INT_THRESHOLD - 1) & IR_CFG_INT_LEVEL_MASK
	       << IR_CFG_INT_LEVEL_SHIFT;
	val |= IR_CFG_MODE_RAW;
	val |= (rate - 1) & IR_CFG_FREQ_MASK << IR_CFG_FREQ_SHIFT;
	writel_relaxed(val, priv->base + IR_CONFIG);

	writel_relaxed(0x00, priv->base + IR_INTM);
	/* write arbitrary value to start  */
	writel_relaxed(0x01, priv->base + IR_START);
	return 0;
}

static int hix5hd2_ir_open(struct rc_dev *rdev)
{
	struct hix5hd2_ir_priv *priv = rdev->priv;

	hix5hd2_ir_enable(priv, true);
	return hix5hd2_ir_config(priv);
}

static void hix5hd2_ir_close(struct rc_dev *rdev)
{
	struct hix5hd2_ir_priv *priv = rdev->priv;

	hix5hd2_ir_enable(priv, false);
}

static irqreturn_t hix5hd2_ir_rx_interrupt(int irq, void *data)
{
	u32 symb_num, symb_val, symb_time;
	u32 data_l, data_h;
	u32 irq_sr, i;
	struct hix5hd2_ir_priv *priv = data;

	irq_sr = readl_relaxed(priv->base + IR_INTS);
	if (irq_sr & INTMS_OVERFLOW) {
		/*
		 * we must read IR_DATAL first, then we can clean up
		 * IR_INTS availably since logic would not clear
		 * fifo when overflow, drv do the job
		 */
		ir_raw_event_reset(priv->rdev);
		symb_num = readl_relaxed(priv->base + IR_DATAH);
		for (i = 0; i < symb_num; i++)
			readl_relaxed(priv->base + IR_DATAL);

		writel_relaxed(INT_CLR_OVERFLOW, priv->base + IR_INTC);
		dev_info(priv->dev, "overflow, level=%d\n",
			 IR_CFG_INT_THRESHOLD);
	}

	if ((irq_sr & INTMS_SYMBRCV) || (irq_sr & INTMS_TIMEOUT)) {
		DEFINE_IR_RAW_EVENT(ev);

		symb_num = readl_relaxed(priv->base + IR_DATAH);
		for (i = 0; i < symb_num; i++) {
			symb_val = readl_relaxed(priv->base + IR_DATAL);
			data_l = ((symb_val & 0xffff) * 10);
			data_h =  ((symb_val >> 16) & 0xffff) * 10;
			symb_time = (data_l + data_h) / 10;

			ev.duration = US_TO_NS(data_l);
			ev.pulse = true;
			ir_raw_event_store(priv->rdev, &ev);

			if (symb_time < IR_CFG_SYMBOL_MAXWIDTH) {
				ev.duration = US_TO_NS(data_h);
				ev.pulse = false;
				ir_raw_event_store(priv->rdev, &ev);
			} else {
				ir_raw_event_set_idle(priv->rdev, true);
			}
		}

		if (irq_sr & INTMS_SYMBRCV)
			writel_relaxed(INT_CLR_RCV, priv->base + IR_INTC);
		if (irq_sr & INTMS_TIMEOUT)
			writel_relaxed(INT_CLR_TIMEOUT, priv->base + IR_INTC);
	}

	/* Empty software fifo */
	ir_raw_event_handle(priv->rdev);
	return IRQ_HANDLED;
}

static int hix5hd2_ir_probe(struct platform_device *pdev)
{
	struct rc_dev *rdev;
	struct device *dev = &pdev->dev;
	struct resource *res;
	struct hix5hd2_ir_priv *priv;
	struct device_node *node = pdev->dev.of_node;
	const char *map_name;
	int ret;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->regmap = syscon_regmap_lookup_by_phandle(node,
						       "hisilicon,power-syscon");
	if (IS_ERR(priv->regmap)) {
		dev_err(dev, "no power-reg\n");
		return -EINVAL;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	priv->base = devm_ioremap_resource(dev, res);
	if (IS_ERR((__force void *)priv->base))
		return PTR_ERR((__force void *)priv->base);

	priv->irq = platform_get_irq(pdev, 0);
	if (priv->irq < 0) {
		dev_err(dev, "irq can not get\n");
		return priv->irq;
	}

	rdev = rc_allocate_device();
	if (!rdev)
		return -ENOMEM;

	priv->clock = devm_clk_get(dev, NULL);
	if (IS_ERR(priv->clock)) {
		dev_err(dev, "clock not found\n");
		ret = PTR_ERR(priv->clock);
		goto err;
	}
	clk_prepare_enable(priv->clock);
	priv->rate = clk_get_rate(priv->clock);

	rdev->driver_type = RC_DRIVER_IR_RAW;
	rdev->allowed_protocols = RC_BIT_ALL;
	rdev->priv = priv;
	rdev->open = hix5hd2_ir_open;
	rdev->close = hix5hd2_ir_close;
	rdev->driver_name = IR_HIX5HD2_NAME;
	map_name = of_get_property(node, "linux,rc-map-name", NULL);
	rdev->map_name = map_name ?: RC_MAP_EMPTY;
	rdev->input_name = IR_HIX5HD2_NAME;
	rdev->input_phys = IR_HIX5HD2_NAME "/input0";
	rdev->input_id.bustype = BUS_HOST;
	rdev->input_id.vendor = 0x0001;
	rdev->input_id.product = 0x0001;
	rdev->input_id.version = 0x0100;
	rdev->rx_resolution = US_TO_NS(10);
	rdev->timeout = US_TO_NS(IR_CFG_SYMBOL_MAXWIDTH * 10);

	ret = rc_register_device(rdev);
	if (ret < 0)
		goto clkerr;

	if (devm_request_irq(dev, priv->irq, hix5hd2_ir_rx_interrupt,
			     IRQF_NO_SUSPEND, pdev->name, priv) < 0) {
		dev_err(dev, "IRQ %d register failed\n", priv->irq);
		ret = -EINVAL;
		goto regerr;
	}

	priv->rdev = rdev;
	priv->dev = dev;
	platform_set_drvdata(pdev, priv);

	return ret;

regerr:
	rc_unregister_device(rdev);
	rdev = NULL;
clkerr:
	clk_disable_unprepare(priv->clock);
err:
	rc_free_device(rdev);
	dev_err(dev, "Unable to register device (%d)\n", ret);
	return ret;
}

static int hix5hd2_ir_remove(struct platform_device *pdev)
{
	struct hix5hd2_ir_priv *priv = platform_get_drvdata(pdev);

	clk_disable_unprepare(priv->clock);
	rc_unregister_device(priv->rdev);
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int hix5hd2_ir_suspend(struct device *dev)
{
	struct hix5hd2_ir_priv *priv = dev_get_drvdata(dev);

	clk_disable_unprepare(priv->clock);
	hix5hd2_ir_enable(priv, false);

	return 0;
}

static int hix5hd2_ir_resume(struct device *dev)
{
	struct hix5hd2_ir_priv *priv = dev_get_drvdata(dev);

	hix5hd2_ir_enable(priv, true);
	clk_prepare_enable(priv->clock);

	writel_relaxed(0x01, priv->base + IR_ENABLE);
	writel_relaxed(0x00, priv->base + IR_INTM);
	writel_relaxed(0xff, priv->base + IR_INTC);
	writel_relaxed(0x01, priv->base + IR_START);

	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(hix5hd2_ir_pm_ops, hix5hd2_ir_suspend,
			 hix5hd2_ir_resume);

static struct of_device_id hix5hd2_ir_table[] = {
	{ .compatible = "hisilicon,hix5hd2-ir", },
	{},
};
MODULE_DEVICE_TABLE(of, hix5hd2_ir_table);

static struct platform_driver hix5hd2_ir_driver = {
	.driver = {
		.name = IR_HIX5HD2_NAME,
		.of_match_table = hix5hd2_ir_table,
		.pm     = &hix5hd2_ir_pm_ops,
	},
	.probe = hix5hd2_ir_probe,
	.remove = hix5hd2_ir_remove,
};

module_platform_driver(hix5hd2_ir_driver);

MODULE_DESCRIPTION("IR controller driver for hix5hd2 platforms");
MODULE_AUTHOR("Guoxiong Yan <yanguoxiong@huawei.com>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:hix5hd2-ir");

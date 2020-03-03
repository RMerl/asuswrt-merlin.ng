/*
 * SuperH Timer Support - TMU
 *
 *  Copyright (C) 2009 Magnus Damm
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/clk.h>
#include <linux/clockchips.h>
#include <linux/clocksource.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_domain.h>
#include <linux/pm_runtime.h>
#include <linux/sh_timer.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

enum sh_tmu_model {
	SH_TMU,
	SH_TMU_SH3,
};

struct sh_tmu_device;

struct sh_tmu_channel {
	struct sh_tmu_device *tmu;
	unsigned int index;

	void __iomem *base;
	int irq;

	unsigned long rate;
	unsigned long periodic;
	struct clock_event_device ced;
	struct clocksource cs;
	bool cs_enabled;
	unsigned int enable_count;
};

struct sh_tmu_device {
	struct platform_device *pdev;

	void __iomem *mapbase;
	struct clk *clk;

	enum sh_tmu_model model;

	raw_spinlock_t lock; /* Protect the shared start/stop register */

	struct sh_tmu_channel *channels;
	unsigned int num_channels;

	bool has_clockevent;
	bool has_clocksource;
};

#define TSTR -1 /* shared register */
#define TCOR  0 /* channel register */
#define TCNT 1 /* channel register */
#define TCR 2 /* channel register */

#define TCR_UNF			(1 << 8)
#define TCR_UNIE		(1 << 5)
#define TCR_TPSC_CLK4		(0 << 0)
#define TCR_TPSC_CLK16		(1 << 0)
#define TCR_TPSC_CLK64		(2 << 0)
#define TCR_TPSC_CLK256		(3 << 0)
#define TCR_TPSC_CLK1024	(4 << 0)
#define TCR_TPSC_MASK		(7 << 0)

static inline unsigned long sh_tmu_read(struct sh_tmu_channel *ch, int reg_nr)
{
	unsigned long offs;

	if (reg_nr == TSTR) {
		switch (ch->tmu->model) {
		case SH_TMU_SH3:
			return ioread8(ch->tmu->mapbase + 2);
		case SH_TMU:
			return ioread8(ch->tmu->mapbase + 4);
		}
	}

	offs = reg_nr << 2;

	if (reg_nr == TCR)
		return ioread16(ch->base + offs);
	else
		return ioread32(ch->base + offs);
}

static inline void sh_tmu_write(struct sh_tmu_channel *ch, int reg_nr,
				unsigned long value)
{
	unsigned long offs;

	if (reg_nr == TSTR) {
		switch (ch->tmu->model) {
		case SH_TMU_SH3:
			return iowrite8(value, ch->tmu->mapbase + 2);
		case SH_TMU:
			return iowrite8(value, ch->tmu->mapbase + 4);
		}
	}

	offs = reg_nr << 2;

	if (reg_nr == TCR)
		iowrite16(value, ch->base + offs);
	else
		iowrite32(value, ch->base + offs);
}

static void sh_tmu_start_stop_ch(struct sh_tmu_channel *ch, int start)
{
	unsigned long flags, value;

	/* start stop register shared by multiple timer channels */
	raw_spin_lock_irqsave(&ch->tmu->lock, flags);
	value = sh_tmu_read(ch, TSTR);

	if (start)
		value |= 1 << ch->index;
	else
		value &= ~(1 << ch->index);

	sh_tmu_write(ch, TSTR, value);
	raw_spin_unlock_irqrestore(&ch->tmu->lock, flags);
}

static int __sh_tmu_enable(struct sh_tmu_channel *ch)
{
	int ret;

	/* enable clock */
	ret = clk_enable(ch->tmu->clk);
	if (ret) {
		dev_err(&ch->tmu->pdev->dev, "ch%u: cannot enable clock\n",
			ch->index);
		return ret;
	}

	/* make sure channel is disabled */
	sh_tmu_start_stop_ch(ch, 0);

	/* maximum timeout */
	sh_tmu_write(ch, TCOR, 0xffffffff);
	sh_tmu_write(ch, TCNT, 0xffffffff);

	/* configure channel to parent clock / 4, irq off */
	ch->rate = clk_get_rate(ch->tmu->clk) / 4;
	sh_tmu_write(ch, TCR, TCR_TPSC_CLK4);

	/* enable channel */
	sh_tmu_start_stop_ch(ch, 1);

	return 0;
}

static int sh_tmu_enable(struct sh_tmu_channel *ch)
{
	if (ch->enable_count++ > 0)
		return 0;

	pm_runtime_get_sync(&ch->tmu->pdev->dev);
	dev_pm_syscore_device(&ch->tmu->pdev->dev, true);

	return __sh_tmu_enable(ch);
}

static void __sh_tmu_disable(struct sh_tmu_channel *ch)
{
	/* disable channel */
	sh_tmu_start_stop_ch(ch, 0);

	/* disable interrupts in TMU block */
	sh_tmu_write(ch, TCR, TCR_TPSC_CLK4);

	/* stop clock */
	clk_disable(ch->tmu->clk);
}

static void sh_tmu_disable(struct sh_tmu_channel *ch)
{
	if (WARN_ON(ch->enable_count == 0))
		return;

	if (--ch->enable_count > 0)
		return;

	__sh_tmu_disable(ch);

	dev_pm_syscore_device(&ch->tmu->pdev->dev, false);
	pm_runtime_put(&ch->tmu->pdev->dev);
}

static void sh_tmu_set_next(struct sh_tmu_channel *ch, unsigned long delta,
			    int periodic)
{
	/* stop timer */
	sh_tmu_start_stop_ch(ch, 0);

	/* acknowledge interrupt */
	sh_tmu_read(ch, TCR);

	/* enable interrupt */
	sh_tmu_write(ch, TCR, TCR_UNIE | TCR_TPSC_CLK4);

	/* reload delta value in case of periodic timer */
	if (periodic)
		sh_tmu_write(ch, TCOR, delta);
	else
		sh_tmu_write(ch, TCOR, 0xffffffff);

	sh_tmu_write(ch, TCNT, delta);

	/* start timer */
	sh_tmu_start_stop_ch(ch, 1);
}

static irqreturn_t sh_tmu_interrupt(int irq, void *dev_id)
{
	struct sh_tmu_channel *ch = dev_id;

	/* disable or acknowledge interrupt */
	if (ch->ced.mode == CLOCK_EVT_MODE_ONESHOT)
		sh_tmu_write(ch, TCR, TCR_TPSC_CLK4);
	else
		sh_tmu_write(ch, TCR, TCR_UNIE | TCR_TPSC_CLK4);

	/* notify clockevent layer */
	ch->ced.event_handler(&ch->ced);
	return IRQ_HANDLED;
}

static struct sh_tmu_channel *cs_to_sh_tmu(struct clocksource *cs)
{
	return container_of(cs, struct sh_tmu_channel, cs);
}

static cycle_t sh_tmu_clocksource_read(struct clocksource *cs)
{
	struct sh_tmu_channel *ch = cs_to_sh_tmu(cs);

	return sh_tmu_read(ch, TCNT) ^ 0xffffffff;
}

static int sh_tmu_clocksource_enable(struct clocksource *cs)
{
	struct sh_tmu_channel *ch = cs_to_sh_tmu(cs);
	int ret;

	if (WARN_ON(ch->cs_enabled))
		return 0;

	ret = sh_tmu_enable(ch);
	if (!ret) {
		__clocksource_update_freq_hz(cs, ch->rate);
		ch->cs_enabled = true;
	}

	return ret;
}

static void sh_tmu_clocksource_disable(struct clocksource *cs)
{
	struct sh_tmu_channel *ch = cs_to_sh_tmu(cs);

	if (WARN_ON(!ch->cs_enabled))
		return;

	sh_tmu_disable(ch);
	ch->cs_enabled = false;
}

static void sh_tmu_clocksource_suspend(struct clocksource *cs)
{
	struct sh_tmu_channel *ch = cs_to_sh_tmu(cs);

	if (!ch->cs_enabled)
		return;

	if (--ch->enable_count == 0) {
		__sh_tmu_disable(ch);
		pm_genpd_syscore_poweroff(&ch->tmu->pdev->dev);
	}
}

static void sh_tmu_clocksource_resume(struct clocksource *cs)
{
	struct sh_tmu_channel *ch = cs_to_sh_tmu(cs);

	if (!ch->cs_enabled)
		return;

	if (ch->enable_count++ == 0) {
		pm_genpd_syscore_poweron(&ch->tmu->pdev->dev);
		__sh_tmu_enable(ch);
	}
}

static int sh_tmu_register_clocksource(struct sh_tmu_channel *ch,
				       const char *name)
{
	struct clocksource *cs = &ch->cs;

	cs->name = name;
	cs->rating = 200;
	cs->read = sh_tmu_clocksource_read;
	cs->enable = sh_tmu_clocksource_enable;
	cs->disable = sh_tmu_clocksource_disable;
	cs->suspend = sh_tmu_clocksource_suspend;
	cs->resume = sh_tmu_clocksource_resume;
	cs->mask = CLOCKSOURCE_MASK(32);
	cs->flags = CLOCK_SOURCE_IS_CONTINUOUS;

	dev_info(&ch->tmu->pdev->dev, "ch%u: used as clock source\n",
		 ch->index);

	/* Register with dummy 1 Hz value, gets updated in ->enable() */
	clocksource_register_hz(cs, 1);
	return 0;
}

static struct sh_tmu_channel *ced_to_sh_tmu(struct clock_event_device *ced)
{
	return container_of(ced, struct sh_tmu_channel, ced);
}

static void sh_tmu_clock_event_start(struct sh_tmu_channel *ch, int periodic)
{
	struct clock_event_device *ced = &ch->ced;

	sh_tmu_enable(ch);

	clockevents_config(ced, ch->rate);

	if (periodic) {
		ch->periodic = (ch->rate + HZ/2) / HZ;
		sh_tmu_set_next(ch, ch->periodic, 1);
	}
}

static void sh_tmu_clock_event_mode(enum clock_event_mode mode,
				    struct clock_event_device *ced)
{
	struct sh_tmu_channel *ch = ced_to_sh_tmu(ced);
	int disabled = 0;

	/* deal with old setting first */
	switch (ced->mode) {
	case CLOCK_EVT_MODE_PERIODIC:
	case CLOCK_EVT_MODE_ONESHOT:
		sh_tmu_disable(ch);
		disabled = 1;
		break;
	default:
		break;
	}

	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		dev_info(&ch->tmu->pdev->dev,
			 "ch%u: used for periodic clock events\n", ch->index);
		sh_tmu_clock_event_start(ch, 1);
		break;
	case CLOCK_EVT_MODE_ONESHOT:
		dev_info(&ch->tmu->pdev->dev,
			 "ch%u: used for oneshot clock events\n", ch->index);
		sh_tmu_clock_event_start(ch, 0);
		break;
	case CLOCK_EVT_MODE_UNUSED:
		if (!disabled)
			sh_tmu_disable(ch);
		break;
	case CLOCK_EVT_MODE_SHUTDOWN:
	default:
		break;
	}
}

static int sh_tmu_clock_event_next(unsigned long delta,
				   struct clock_event_device *ced)
{
	struct sh_tmu_channel *ch = ced_to_sh_tmu(ced);

	BUG_ON(ced->mode != CLOCK_EVT_MODE_ONESHOT);

	/* program new delta value */
	sh_tmu_set_next(ch, delta, 0);
	return 0;
}

static void sh_tmu_clock_event_suspend(struct clock_event_device *ced)
{
	pm_genpd_syscore_poweroff(&ced_to_sh_tmu(ced)->tmu->pdev->dev);
}

static void sh_tmu_clock_event_resume(struct clock_event_device *ced)
{
	pm_genpd_syscore_poweron(&ced_to_sh_tmu(ced)->tmu->pdev->dev);
}

static void sh_tmu_register_clockevent(struct sh_tmu_channel *ch,
				       const char *name)
{
	struct clock_event_device *ced = &ch->ced;
	int ret;

	ced->name = name;
	ced->features = CLOCK_EVT_FEAT_PERIODIC;
	ced->features |= CLOCK_EVT_FEAT_ONESHOT;
	ced->rating = 200;
	ced->cpumask = cpu_possible_mask;
	ced->set_next_event = sh_tmu_clock_event_next;
	ced->set_mode = sh_tmu_clock_event_mode;
	ced->suspend = sh_tmu_clock_event_suspend;
	ced->resume = sh_tmu_clock_event_resume;

	dev_info(&ch->tmu->pdev->dev, "ch%u: used for clock events\n",
		 ch->index);

	clockevents_config_and_register(ced, 1, 0x300, 0xffffffff);

	ret = request_irq(ch->irq, sh_tmu_interrupt,
			  IRQF_TIMER | IRQF_IRQPOLL | IRQF_NOBALANCING,
			  dev_name(&ch->tmu->pdev->dev), ch);
	if (ret) {
		dev_err(&ch->tmu->pdev->dev, "ch%u: failed to request irq %d\n",
			ch->index, ch->irq);
		return;
	}
}

static int sh_tmu_register(struct sh_tmu_channel *ch, const char *name,
			   bool clockevent, bool clocksource)
{
	if (clockevent) {
		ch->tmu->has_clockevent = true;
		sh_tmu_register_clockevent(ch, name);
	} else if (clocksource) {
		ch->tmu->has_clocksource = true;
		sh_tmu_register_clocksource(ch, name);
	}

	return 0;
}

static int sh_tmu_channel_setup(struct sh_tmu_channel *ch, unsigned int index,
				bool clockevent, bool clocksource,
				struct sh_tmu_device *tmu)
{
	/* Skip unused channels. */
	if (!clockevent && !clocksource)
		return 0;

	ch->tmu = tmu;
	ch->index = index;

	if (tmu->model == SH_TMU_SH3)
		ch->base = tmu->mapbase + 4 + ch->index * 12;
	else
		ch->base = tmu->mapbase + 8 + ch->index * 12;

	ch->irq = platform_get_irq(tmu->pdev, index);
	if (ch->irq < 0) {
		dev_err(&tmu->pdev->dev, "ch%u: failed to get irq\n",
			ch->index);
		return ch->irq;
	}

	ch->cs_enabled = false;
	ch->enable_count = 0;

	return sh_tmu_register(ch, dev_name(&tmu->pdev->dev),
			       clockevent, clocksource);
}

static int sh_tmu_map_memory(struct sh_tmu_device *tmu)
{
	struct resource *res;

	res = platform_get_resource(tmu->pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&tmu->pdev->dev, "failed to get I/O memory\n");
		return -ENXIO;
	}

	tmu->mapbase = ioremap_nocache(res->start, resource_size(res));
	if (tmu->mapbase == NULL)
		return -ENXIO;

	return 0;
}

static int sh_tmu_parse_dt(struct sh_tmu_device *tmu)
{
	struct device_node *np = tmu->pdev->dev.of_node;

	tmu->model = SH_TMU;
	tmu->num_channels = 3;

	of_property_read_u32(np, "#renesas,channels", &tmu->num_channels);

	if (tmu->num_channels != 2 && tmu->num_channels != 3) {
		dev_err(&tmu->pdev->dev, "invalid number of channels %u\n",
			tmu->num_channels);
		return -EINVAL;
	}

	return 0;
}

static int sh_tmu_setup(struct sh_tmu_device *tmu, struct platform_device *pdev)
{
	unsigned int i;
	int ret;

	tmu->pdev = pdev;

	raw_spin_lock_init(&tmu->lock);

	if (IS_ENABLED(CONFIG_OF) && pdev->dev.of_node) {
		ret = sh_tmu_parse_dt(tmu);
		if (ret < 0)
			return ret;
	} else if (pdev->dev.platform_data) {
		const struct platform_device_id *id = pdev->id_entry;
		struct sh_timer_config *cfg = pdev->dev.platform_data;

		tmu->model = id->driver_data;
		tmu->num_channels = hweight8(cfg->channels_mask);
	} else {
		dev_err(&tmu->pdev->dev, "missing platform data\n");
		return -ENXIO;
	}

	/* Get hold of clock. */
	tmu->clk = clk_get(&tmu->pdev->dev, "fck");
	if (IS_ERR(tmu->clk)) {
		dev_err(&tmu->pdev->dev, "cannot get clock\n");
		return PTR_ERR(tmu->clk);
	}

	ret = clk_prepare(tmu->clk);
	if (ret < 0)
		goto err_clk_put;

	/* Map the memory resource. */
	ret = sh_tmu_map_memory(tmu);
	if (ret < 0) {
		dev_err(&tmu->pdev->dev, "failed to remap I/O memory\n");
		goto err_clk_unprepare;
	}

	/* Allocate and setup the channels. */
	tmu->channels = kzalloc(sizeof(*tmu->channels) * tmu->num_channels,
				GFP_KERNEL);
	if (tmu->channels == NULL) {
		ret = -ENOMEM;
		goto err_unmap;
	}

	/*
	 * Use the first channel as a clock event device and the second channel
	 * as a clock source.
	 */
	for (i = 0; i < tmu->num_channels; ++i) {
		ret = sh_tmu_channel_setup(&tmu->channels[i], i,
					   i == 0, i == 1, tmu);
		if (ret < 0)
			goto err_unmap;
	}

	platform_set_drvdata(pdev, tmu);

	return 0;

err_unmap:
	kfree(tmu->channels);
	iounmap(tmu->mapbase);
err_clk_unprepare:
	clk_unprepare(tmu->clk);
err_clk_put:
	clk_put(tmu->clk);
	return ret;
}

static int sh_tmu_probe(struct platform_device *pdev)
{
	struct sh_tmu_device *tmu = platform_get_drvdata(pdev);
	int ret;

	if (!is_early_platform_device(pdev)) {
		pm_runtime_set_active(&pdev->dev);
		pm_runtime_enable(&pdev->dev);
	}

	if (tmu) {
		dev_info(&pdev->dev, "kept as earlytimer\n");
		goto out;
	}

	tmu = kzalloc(sizeof(*tmu), GFP_KERNEL);
	if (tmu == NULL)
		return -ENOMEM;

	ret = sh_tmu_setup(tmu, pdev);
	if (ret) {
		kfree(tmu);
		pm_runtime_idle(&pdev->dev);
		return ret;
	}
	if (is_early_platform_device(pdev))
		return 0;

 out:
	if (tmu->has_clockevent || tmu->has_clocksource)
		pm_runtime_irq_safe(&pdev->dev);
	else
		pm_runtime_idle(&pdev->dev);

	return 0;
}

static int sh_tmu_remove(struct platform_device *pdev)
{
	return -EBUSY; /* cannot unregister clockevent and clocksource */
}

static const struct platform_device_id sh_tmu_id_table[] = {
	{ "sh-tmu", SH_TMU },
	{ "sh-tmu-sh3", SH_TMU_SH3 },
	{ }
};
MODULE_DEVICE_TABLE(platform, sh_tmu_id_table);

static const struct of_device_id sh_tmu_of_table[] __maybe_unused = {
	{ .compatible = "renesas,tmu" },
	{ }
};
MODULE_DEVICE_TABLE(of, sh_tmu_of_table);

static struct platform_driver sh_tmu_device_driver = {
	.probe		= sh_tmu_probe,
	.remove		= sh_tmu_remove,
	.driver		= {
		.name	= "sh_tmu",
		.of_match_table = of_match_ptr(sh_tmu_of_table),
	},
	.id_table	= sh_tmu_id_table,
};

static int __init sh_tmu_init(void)
{
	return platform_driver_register(&sh_tmu_device_driver);
}

static void __exit sh_tmu_exit(void)
{
	platform_driver_unregister(&sh_tmu_device_driver);
}

early_platform_init("earlytimer", &sh_tmu_device_driver);
subsys_initcall(sh_tmu_init);
module_exit(sh_tmu_exit);

MODULE_AUTHOR("Magnus Damm");
MODULE_DESCRIPTION("SuperH TMU Timer Driver");
MODULE_LICENSE("GPL v2");

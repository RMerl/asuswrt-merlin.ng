// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 * Author(s): Patrice Chotard, <patrice.chotard@st.com> for STMicroelectronics.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <fdtdec.h>
#include <timer.h>

#include <asm/io.h>

/* Timer control1 register  */
#define CR1_CEN			BIT(0)
#define CR1_ARPE		BIT(7)

/* Event Generation Register register  */
#define EGR_UG			BIT(0)

/* Auto reload register for free running config */
#define GPT_FREE_RUNNING	0xFFFFFFFF

struct stm32_timer_regs {
	u32 cr1;
	u32 cr2;
	u32 smcr;
	u32 dier;
	u32 sr;
	u32 egr;
	u32 ccmr1;
	u32 ccmr2;
	u32 ccer;
	u32 cnt;
	u32 psc;
	u32 arr;
	u32 reserved;
	u32 ccr1;
	u32 ccr2;
	u32 ccr3;
	u32 ccr4;
	u32 reserved1;
	u32 dcr;
	u32 dmar;
	u32 tim2_5_or;
};

struct stm32_timer_priv {
	struct stm32_timer_regs *base;
};

static int stm32_timer_get_count(struct udevice *dev, u64 *count)
{
	struct stm32_timer_priv *priv = dev_get_priv(dev);
	struct stm32_timer_regs *regs = priv->base;

	*count = readl(&regs->cnt);

	return 0;
}

static int stm32_timer_probe(struct udevice *dev)
{
	struct timer_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct stm32_timer_priv *priv = dev_get_priv(dev);
	struct stm32_timer_regs *regs;
	struct clk clk;
	fdt_addr_t addr;
	int ret;
	u32 rate, psc;

	addr = dev_read_addr(dev);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base = (struct stm32_timer_regs *)addr;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret < 0)
		return ret;

	ret = clk_enable(&clk);
	if (ret) {
		dev_err(dev, "failed to enable clock\n");
		return ret;
	}

	regs = priv->base;

	/* Stop the timer */
	clrbits_le32(&regs->cr1, CR1_CEN);

	/* get timer clock */
	rate = clk_get_rate(&clk);

	/* we set timer prescaler to obtain a 1MHz timer counter frequency */
	psc = (rate / CONFIG_SYS_HZ_CLOCK) - 1;
	writel(psc, &regs->psc);

	/* Set timer frequency to 1MHz */
	uc_priv->clock_rate = CONFIG_SYS_HZ_CLOCK;

	/* Configure timer for auto-reload */
	setbits_le32(&regs->cr1, CR1_ARPE);

	/* load value for auto reload */
	writel(GPT_FREE_RUNNING, &regs->arr);

	/* start timer */
	setbits_le32(&regs->cr1, CR1_CEN);

	/* Update generation */
	setbits_le32(&regs->egr, EGR_UG);

	return 0;
}

static const struct timer_ops stm32_timer_ops = {
	.get_count = stm32_timer_get_count,
};

static const struct udevice_id stm32_timer_ids[] = {
	{ .compatible = "st,stm32-timer" },
	{}
};

U_BOOT_DRIVER(stm32_timer) = {
	.name = "stm32_timer",
	.id = UCLASS_TIMER,
	.of_match = stm32_timer_ids,
	.priv_auto_alloc_size = sizeof(struct stm32_timer_priv),
	.probe = stm32_timer_probe,
	.ops = &stm32_timer_ops,
};


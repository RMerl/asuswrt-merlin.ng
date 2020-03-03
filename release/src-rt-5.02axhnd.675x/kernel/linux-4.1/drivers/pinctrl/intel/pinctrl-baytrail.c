/*
 * Pinctrl GPIO driver for Intel Baytrail
 * Copyright (c) 2012-2013, Intel Corporation.
 *
 * Author: Mathias Nyman <mathias.nyman@linux.intel.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/bitops.h>
#include <linux/interrupt.h>
#include <linux/gpio.h>
#include <linux/acpi.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>
#include <linux/io.h>
#include <linux/pm_runtime.h>
#include <linux/pinctrl/pinctrl.h>

/* memory mapped register offsets */
#define BYT_CONF0_REG		0x000
#define BYT_CONF1_REG		0x004
#define BYT_VAL_REG		0x008
#define BYT_DFT_REG		0x00c
#define BYT_INT_STAT_REG	0x800

/* BYT_CONF0_REG register bits */
#define BYT_IODEN		BIT(31)
#define BYT_DIRECT_IRQ_EN	BIT(27)
#define BYT_TRIG_NEG		BIT(26)
#define BYT_TRIG_POS		BIT(25)
#define BYT_TRIG_LVL		BIT(24)
#define BYT_PULL_STR_SHIFT	9
#define BYT_PULL_STR_MASK	(3 << BYT_PULL_STR_SHIFT)
#define BYT_PULL_STR_2K		(0 << BYT_PULL_STR_SHIFT)
#define BYT_PULL_STR_10K	(1 << BYT_PULL_STR_SHIFT)
#define BYT_PULL_STR_20K	(2 << BYT_PULL_STR_SHIFT)
#define BYT_PULL_STR_40K	(3 << BYT_PULL_STR_SHIFT)
#define BYT_PULL_ASSIGN_SHIFT	7
#define BYT_PULL_ASSIGN_MASK	(3 << BYT_PULL_ASSIGN_SHIFT)
#define BYT_PULL_ASSIGN_UP	(1 << BYT_PULL_ASSIGN_SHIFT)
#define BYT_PULL_ASSIGN_DOWN	(2 << BYT_PULL_ASSIGN_SHIFT)
#define BYT_PIN_MUX		0x07

/* BYT_VAL_REG register bits */
#define BYT_INPUT_EN		BIT(2)  /* 0: input enabled (active low)*/
#define BYT_OUTPUT_EN		BIT(1)  /* 0: output enabled (active low)*/
#define BYT_LEVEL		BIT(0)

#define BYT_DIR_MASK		(BIT(1) | BIT(2))
#define BYT_TRIG_MASK		(BIT(26) | BIT(25) | BIT(24))

#define BYT_CONF0_RESTORE_MASK	(BYT_DIRECT_IRQ_EN | BYT_TRIG_MASK | \
				 BYT_PIN_MUX)
#define BYT_VAL_RESTORE_MASK	(BYT_DIR_MASK | BYT_LEVEL)

#define BYT_NGPIO_SCORE		102
#define BYT_NGPIO_NCORE		28
#define BYT_NGPIO_SUS		44

#define BYT_SCORE_ACPI_UID	"1"
#define BYT_NCORE_ACPI_UID	"2"
#define BYT_SUS_ACPI_UID	"3"

/*
 * Baytrail gpio controller consist of three separate sub-controllers called
 * SCORE, NCORE and SUS. The sub-controllers are identified by their acpi UID.
 *
 * GPIO numbering is _not_ ordered meaning that gpio # 0 in ACPI namespace does
 * _not_ correspond to the first gpio register at controller's gpio base.
 * There is no logic or pattern in mapping gpio numbers to registers (pads) so
 * each sub-controller needs to have its own mapping table
 */

/* score_pins[gpio_nr] = pad_nr */

static unsigned const score_pins[BYT_NGPIO_SCORE] = {
	85, 89, 93, 96, 99, 102, 98, 101, 34, 37,
	36, 38, 39, 35, 40, 84, 62, 61, 64, 59,
	54, 56, 60, 55, 63, 57, 51, 50, 53, 47,
	52, 49, 48, 43, 46, 41, 45, 42, 58, 44,
	95, 105, 70, 68, 67, 66, 69, 71, 65, 72,
	86, 90, 88, 92, 103, 77, 79, 83, 78, 81,
	80, 82, 13, 12, 15, 14, 17, 18, 19, 16,
	2, 1, 0, 4, 6, 7, 9, 8, 33, 32,
	31, 30, 29, 27, 25, 28, 26, 23, 21, 20,
	24, 22, 5, 3, 10, 11, 106, 87, 91, 104,
	97, 100,
};

static unsigned const ncore_pins[BYT_NGPIO_NCORE] = {
	19, 18, 17, 20, 21, 22, 24, 25, 23, 16,
	14, 15, 12, 26, 27, 1, 4, 8, 11, 0,
	3, 6, 10, 13, 2, 5, 9, 7,
};

static unsigned const sus_pins[BYT_NGPIO_SUS] = {
	29, 33, 30, 31, 32, 34, 36, 35, 38, 37,
	18, 7, 11, 20, 17, 1, 8, 10, 19, 12,
	0, 2, 23, 39, 28, 27, 22, 21, 24, 25,
	26, 51, 56, 54, 49, 55, 48, 57, 50, 58,
	52, 53, 59, 40,
};

static struct pinctrl_gpio_range byt_ranges[] = {
	{
		.name = BYT_SCORE_ACPI_UID, /* match with acpi _UID in probe */
		.npins = BYT_NGPIO_SCORE,
		.pins = score_pins,
	},
	{
		.name = BYT_NCORE_ACPI_UID,
		.npins = BYT_NGPIO_NCORE,
		.pins = ncore_pins,
	},
	{
		.name = BYT_SUS_ACPI_UID,
		.npins = BYT_NGPIO_SUS,
		.pins = sus_pins,
	},
	{
	},
};

struct byt_gpio_pin_context {
	u32 conf0;
	u32 val;
};

struct byt_gpio {
	struct gpio_chip		chip;
	struct platform_device		*pdev;
	raw_spinlock_t			lock;
	void __iomem			*reg_base;
	struct pinctrl_gpio_range	*range;
	struct byt_gpio_pin_context	*saved_context;
};

#define to_byt_gpio(c)	container_of(c, struct byt_gpio, chip)

static void __iomem *byt_gpio_reg(struct gpio_chip *chip, unsigned offset,
				 int reg)
{
	struct byt_gpio *vg = to_byt_gpio(chip);
	u32 reg_offset;

	if (reg == BYT_INT_STAT_REG)
		reg_offset = (offset / 32) * 4;
	else
		reg_offset = vg->range->pins[offset] * 16;

	return vg->reg_base + reg_offset + reg;
}

static void byt_gpio_clear_triggering(struct byt_gpio *vg, unsigned offset)
{
	void __iomem *reg = byt_gpio_reg(&vg->chip, offset, BYT_CONF0_REG);
	unsigned long flags;
	u32 value;

	raw_spin_lock_irqsave(&vg->lock, flags);
	value = readl(reg);
	value &= ~(BYT_TRIG_POS | BYT_TRIG_NEG | BYT_TRIG_LVL);
	writel(value, reg);
	raw_spin_unlock_irqrestore(&vg->lock, flags);
}

static u32 byt_get_gpio_mux(struct byt_gpio *vg, unsigned offset)
{
	/* SCORE pin 92-93 */
	if (!strcmp(vg->range->name, BYT_SCORE_ACPI_UID) &&
		offset >= 92 && offset <= 93)
		return 1;

	/* SUS pin 11-21 */
	if (!strcmp(vg->range->name, BYT_SUS_ACPI_UID) &&
		offset >= 11 && offset <= 21)
		return 1;

	return 0;
}

static int byt_gpio_request(struct gpio_chip *chip, unsigned offset)
{
	struct byt_gpio *vg = to_byt_gpio(chip);
	void __iomem *reg = byt_gpio_reg(chip, offset, BYT_CONF0_REG);
	u32 value, gpio_mux;
	unsigned long flags;

	raw_spin_lock_irqsave(&vg->lock, flags);

	/*
	 * In most cases, func pin mux 000 means GPIO function.
	 * But, some pins may have func pin mux 001 represents
	 * GPIO function.
	 *
	 * Because there are devices out there where some pins were not
	 * configured correctly we allow changing the mux value from
	 * request (but print out warning about that).
	 */
	value = readl(reg) & BYT_PIN_MUX;
	gpio_mux = byt_get_gpio_mux(vg, offset);
	if (WARN_ON(gpio_mux != value)) {
		value = readl(reg) & ~BYT_PIN_MUX;
		value |= gpio_mux;
		writel(value, reg);

		dev_warn(&vg->pdev->dev,
			 "pin %u forcibly re-configured as GPIO\n", offset);
	}

	raw_spin_unlock_irqrestore(&vg->lock, flags);

	pm_runtime_get(&vg->pdev->dev);

	return 0;
}

static void byt_gpio_free(struct gpio_chip *chip, unsigned offset)
{
	struct byt_gpio *vg = to_byt_gpio(chip);

	byt_gpio_clear_triggering(vg, offset);
	pm_runtime_put(&vg->pdev->dev);
}

static int byt_irq_type(struct irq_data *d, unsigned type)
{
	struct byt_gpio *vg = to_byt_gpio(irq_data_get_irq_chip_data(d));
	u32 offset = irqd_to_hwirq(d);
	u32 value;
	unsigned long flags;
	void __iomem *reg = byt_gpio_reg(&vg->chip, offset, BYT_CONF0_REG);

	if (offset >= vg->chip.ngpio)
		return -EINVAL;

	raw_spin_lock_irqsave(&vg->lock, flags);
	value = readl(reg);

	WARN(value & BYT_DIRECT_IRQ_EN,
		"Bad pad config for io mode, force direct_irq_en bit clearing");

	/* For level trigges the BYT_TRIG_POS and BYT_TRIG_NEG bits
	 * are used to indicate high and low level triggering
	 */
	value &= ~(BYT_DIRECT_IRQ_EN | BYT_TRIG_POS | BYT_TRIG_NEG |
		   BYT_TRIG_LVL);

	writel(value, reg);

	if (type & IRQ_TYPE_EDGE_BOTH)
		__irq_set_handler_locked(d->irq, handle_edge_irq);
	else if (type & IRQ_TYPE_LEVEL_MASK)
		__irq_set_handler_locked(d->irq, handle_level_irq);

	raw_spin_unlock_irqrestore(&vg->lock, flags);

	return 0;
}

static int byt_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	void __iomem *reg = byt_gpio_reg(chip, offset, BYT_VAL_REG);
	struct byt_gpio *vg = to_byt_gpio(chip);
	unsigned long flags;
	u32 val;

	raw_spin_lock_irqsave(&vg->lock, flags);
	val = readl(reg);
	raw_spin_unlock_irqrestore(&vg->lock, flags);

	return val & BYT_LEVEL;
}

static void byt_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	struct byt_gpio *vg = to_byt_gpio(chip);
	void __iomem *reg = byt_gpio_reg(chip, offset, BYT_VAL_REG);
	unsigned long flags;
	u32 old_val;

	raw_spin_lock_irqsave(&vg->lock, flags);

	old_val = readl(reg);

	if (value)
		writel(old_val | BYT_LEVEL, reg);
	else
		writel(old_val & ~BYT_LEVEL, reg);

	raw_spin_unlock_irqrestore(&vg->lock, flags);
}

static int byt_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	struct byt_gpio *vg = to_byt_gpio(chip);
	void __iomem *reg = byt_gpio_reg(chip, offset, BYT_VAL_REG);
	unsigned long flags;
	u32 value;

	raw_spin_lock_irqsave(&vg->lock, flags);

	value = readl(reg) | BYT_DIR_MASK;
	value &= ~BYT_INPUT_EN;		/* active low */
	writel(value, reg);

	raw_spin_unlock_irqrestore(&vg->lock, flags);

	return 0;
}

static int byt_gpio_direction_output(struct gpio_chip *chip,
				     unsigned gpio, int value)
{
	struct byt_gpio *vg = to_byt_gpio(chip);
	void __iomem *conf_reg = byt_gpio_reg(chip, gpio, BYT_CONF0_REG);
	void __iomem *reg = byt_gpio_reg(chip, gpio, BYT_VAL_REG);
	unsigned long flags;
	u32 reg_val;

	raw_spin_lock_irqsave(&vg->lock, flags);

	/*
	 * Before making any direction modifications, do a check if gpio
	 * is set for direct IRQ.  On baytrail, setting GPIO to output does
	 * not make sense, so let's at least warn the caller before they shoot
	 * themselves in the foot.
	 */
	WARN(readl(conf_reg) & BYT_DIRECT_IRQ_EN,
		"Potential Error: Setting GPIO with direct_irq_en to output");

	reg_val = readl(reg) | BYT_DIR_MASK;
	reg_val &= ~(BYT_OUTPUT_EN | BYT_INPUT_EN);

	if (value)
		writel(reg_val | BYT_LEVEL, reg);
	else
		writel(reg_val & ~BYT_LEVEL, reg);

	raw_spin_unlock_irqrestore(&vg->lock, flags);

	return 0;
}

static void byt_gpio_dbg_show(struct seq_file *s, struct gpio_chip *chip)
{
	struct byt_gpio *vg = to_byt_gpio(chip);
	int i;
	u32 conf0, val, offs;

	for (i = 0; i < vg->chip.ngpio; i++) {
		const char *pull_str = NULL;
		const char *pull = NULL;
		unsigned long flags;
		const char *label;
		offs = vg->range->pins[i] * 16;

		raw_spin_lock_irqsave(&vg->lock, flags);
		conf0 = readl(vg->reg_base + offs + BYT_CONF0_REG);
		val = readl(vg->reg_base + offs + BYT_VAL_REG);
		raw_spin_unlock_irqrestore(&vg->lock, flags);

		label = gpiochip_is_requested(chip, i);
		if (!label)
			label = "Unrequested";

		switch (conf0 & BYT_PULL_ASSIGN_MASK) {
		case BYT_PULL_ASSIGN_UP:
			pull = "up";
			break;
		case BYT_PULL_ASSIGN_DOWN:
			pull = "down";
			break;
		}

		switch (conf0 & BYT_PULL_STR_MASK) {
		case BYT_PULL_STR_2K:
			pull_str = "2k";
			break;
		case BYT_PULL_STR_10K:
			pull_str = "10k";
			break;
		case BYT_PULL_STR_20K:
			pull_str = "20k";
			break;
		case BYT_PULL_STR_40K:
			pull_str = "40k";
			break;
		}

		seq_printf(s,
			   " gpio-%-3d (%-20.20s) %s %s %s pad-%-3d offset:0x%03x mux:%d %s%s%s",
			   i,
			   label,
			   val & BYT_INPUT_EN ? "  " : "in",
			   val & BYT_OUTPUT_EN ? "   " : "out",
			   val & BYT_LEVEL ? "hi" : "lo",
			   vg->range->pins[i], offs,
			   conf0 & 0x7,
			   conf0 & BYT_TRIG_NEG ? " fall" : "     ",
			   conf0 & BYT_TRIG_POS ? " rise" : "     ",
			   conf0 & BYT_TRIG_LVL ? " level" : "      ");

		if (pull && pull_str)
			seq_printf(s, " %-4s %-3s", pull, pull_str);
		else
			seq_puts(s, "          ");

		if (conf0 & BYT_IODEN)
			seq_puts(s, " open-drain");

		seq_puts(s, "\n");
	}
}

static void byt_gpio_irq_handler(unsigned irq, struct irq_desc *desc)
{
	struct irq_data *data = irq_desc_get_irq_data(desc);
	struct byt_gpio *vg = to_byt_gpio(irq_desc_get_handler_data(desc));
	struct irq_chip *chip = irq_data_get_irq_chip(data);
	u32 base, pin;
	void __iomem *reg;
	unsigned long pending;
	unsigned virq;

	/* check from GPIO controller which pin triggered the interrupt */
	for (base = 0; base < vg->chip.ngpio; base += 32) {
		reg = byt_gpio_reg(&vg->chip, base, BYT_INT_STAT_REG);
		pending = readl(reg);
		for_each_set_bit(pin, &pending, 32) {
			virq = irq_find_mapping(vg->chip.irqdomain, base + pin);
			generic_handle_irq(virq);
		}
	}
	chip->irq_eoi(data);
}

static void byt_irq_ack(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct byt_gpio *vg = to_byt_gpio(gc);
	unsigned offset = irqd_to_hwirq(d);
	void __iomem *reg;

	raw_spin_lock(&vg->lock);
	reg = byt_gpio_reg(&vg->chip, offset, BYT_INT_STAT_REG);
	writel(BIT(offset % 32), reg);
	raw_spin_unlock(&vg->lock);
}

static void byt_irq_unmask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct byt_gpio *vg = to_byt_gpio(gc);
	unsigned offset = irqd_to_hwirq(d);
	unsigned long flags;
	void __iomem *reg;
	u32 value;

	reg = byt_gpio_reg(&vg->chip, offset, BYT_CONF0_REG);

	raw_spin_lock_irqsave(&vg->lock, flags);
	value = readl(reg);

	switch (irqd_get_trigger_type(d)) {
	case IRQ_TYPE_LEVEL_HIGH:
		value |= BYT_TRIG_LVL;
	case IRQ_TYPE_EDGE_RISING:
		value |= BYT_TRIG_POS;
		break;
	case IRQ_TYPE_LEVEL_LOW:
		value |= BYT_TRIG_LVL;
	case IRQ_TYPE_EDGE_FALLING:
		value |= BYT_TRIG_NEG;
		break;
	case IRQ_TYPE_EDGE_BOTH:
		value |= (BYT_TRIG_NEG | BYT_TRIG_POS);
		break;
	}

	writel(value, reg);

	raw_spin_unlock_irqrestore(&vg->lock, flags);
}

static void byt_irq_mask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct byt_gpio *vg = to_byt_gpio(gc);

	byt_gpio_clear_triggering(vg, irqd_to_hwirq(d));
}

static struct irq_chip byt_irqchip = {
	.name = "BYT-GPIO",
	.irq_ack = byt_irq_ack,
	.irq_mask = byt_irq_mask,
	.irq_unmask = byt_irq_unmask,
	.irq_set_type = byt_irq_type,
	.flags = IRQCHIP_SKIP_SET_WAKE,
};

static void byt_gpio_irq_init_hw(struct byt_gpio *vg)
{
	void __iomem *reg;
	u32 base, value;
	int i;

	/*
	 * Clear interrupt triggers for all pins that are GPIOs and
	 * do not use direct IRQ mode. This will prevent spurious
	 * interrupts from misconfigured pins.
	 */
	for (i = 0; i < vg->chip.ngpio; i++) {
		value = readl(byt_gpio_reg(&vg->chip, i, BYT_CONF0_REG));
		if ((value & BYT_PIN_MUX) == byt_get_gpio_mux(vg, i) &&
		    !(value & BYT_DIRECT_IRQ_EN)) {
			byt_gpio_clear_triggering(vg, i);
			dev_dbg(&vg->pdev->dev, "disabling GPIO %d\n", i);
		}
	}

	/* clear interrupt status trigger registers */
	for (base = 0; base < vg->chip.ngpio; base += 32) {
		reg = byt_gpio_reg(&vg->chip, base, BYT_INT_STAT_REG);
		writel(0xffffffff, reg);
		/* make sure trigger bits are cleared, if not then a pin
		   might be misconfigured in bios */
		value = readl(reg);
		if (value)
			dev_err(&vg->pdev->dev,
				"GPIO interrupt error, pins misconfigured\n");
	}
}

static int byt_gpio_probe(struct platform_device *pdev)
{
	struct byt_gpio *vg;
	struct gpio_chip *gc;
	struct resource *mem_rc, *irq_rc;
	struct device *dev = &pdev->dev;
	struct acpi_device *acpi_dev;
	struct pinctrl_gpio_range *range;
	acpi_handle handle = ACPI_HANDLE(dev);
	int ret;

	if (acpi_bus_get_device(handle, &acpi_dev))
		return -ENODEV;

	vg = devm_kzalloc(dev, sizeof(struct byt_gpio), GFP_KERNEL);
	if (!vg) {
		dev_err(&pdev->dev, "can't allocate byt_gpio chip data\n");
		return -ENOMEM;
	}

	for (range = byt_ranges; range->name; range++) {
		if (!strcmp(acpi_dev->pnp.unique_id, range->name)) {
			vg->chip.ngpio = range->npins;
			vg->range = range;
			break;
		}
	}

	if (!vg->chip.ngpio || !vg->range)
		return -ENODEV;

	vg->pdev = pdev;
	platform_set_drvdata(pdev, vg);

	mem_rc = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	vg->reg_base = devm_ioremap_resource(dev, mem_rc);
	if (IS_ERR(vg->reg_base))
		return PTR_ERR(vg->reg_base);

	raw_spin_lock_init(&vg->lock);

	gc = &vg->chip;
	gc->label = dev_name(&pdev->dev);
	gc->owner = THIS_MODULE;
	gc->request = byt_gpio_request;
	gc->free = byt_gpio_free;
	gc->direction_input = byt_gpio_direction_input;
	gc->direction_output = byt_gpio_direction_output;
	gc->get = byt_gpio_get;
	gc->set = byt_gpio_set;
	gc->dbg_show = byt_gpio_dbg_show;
	gc->base = -1;
	gc->can_sleep = false;
	gc->dev = dev;

#ifdef CONFIG_PM_SLEEP
	vg->saved_context = devm_kcalloc(&pdev->dev, gc->ngpio,
				       sizeof(*vg->saved_context), GFP_KERNEL);
#endif

	ret = gpiochip_add(gc);
	if (ret) {
		dev_err(&pdev->dev, "failed adding byt-gpio chip\n");
		return ret;
	}

	/* set up interrupts  */
	irq_rc = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (irq_rc && irq_rc->start) {
		byt_gpio_irq_init_hw(vg);
		ret = gpiochip_irqchip_add(gc, &byt_irqchip, 0,
					   handle_simple_irq, IRQ_TYPE_NONE);
		if (ret) {
			dev_err(dev, "failed to add irqchip\n");
			gpiochip_remove(gc);
			return ret;
		}

		gpiochip_set_chained_irqchip(gc, &byt_irqchip,
					     (unsigned)irq_rc->start,
					     byt_gpio_irq_handler);
	}

	pm_runtime_enable(dev);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int byt_gpio_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct byt_gpio *vg = platform_get_drvdata(pdev);
	int i;

	for (i = 0; i < vg->chip.ngpio; i++) {
		void __iomem *reg;
		u32 value;

		reg = byt_gpio_reg(&vg->chip, i, BYT_CONF0_REG);
		value = readl(reg) & BYT_CONF0_RESTORE_MASK;
		vg->saved_context[i].conf0 = value;

		reg = byt_gpio_reg(&vg->chip, i, BYT_VAL_REG);
		value = readl(reg) & BYT_VAL_RESTORE_MASK;
		vg->saved_context[i].val = value;
	}

	return 0;
}

static int byt_gpio_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct byt_gpio *vg = platform_get_drvdata(pdev);
	int i;

	for (i = 0; i < vg->chip.ngpio; i++) {
		void __iomem *reg;
		u32 value;

		reg = byt_gpio_reg(&vg->chip, i, BYT_CONF0_REG);
		value = readl(reg);
		if ((value & BYT_CONF0_RESTORE_MASK) !=
		     vg->saved_context[i].conf0) {
			value &= ~BYT_CONF0_RESTORE_MASK;
			value |= vg->saved_context[i].conf0;
			writel(value, reg);
			dev_info(dev, "restored pin %d conf0 %#08x", i, value);
		}

		reg = byt_gpio_reg(&vg->chip, i, BYT_VAL_REG);
		value = readl(reg);
		if ((value & BYT_VAL_RESTORE_MASK) !=
		     vg->saved_context[i].val) {
			u32 v;

			v = value & ~BYT_VAL_RESTORE_MASK;
			v |= vg->saved_context[i].val;
			if (v != value) {
				writel(v, reg);
				dev_dbg(dev, "restored pin %d val %#08x\n",
					i, v);
			}
		}
	}

	return 0;
}
#endif

static int byt_gpio_runtime_suspend(struct device *dev)
{
	return 0;
}

static int byt_gpio_runtime_resume(struct device *dev)
{
	return 0;
}

static const struct dev_pm_ops byt_gpio_pm_ops = {
	SET_LATE_SYSTEM_SLEEP_PM_OPS(byt_gpio_suspend, byt_gpio_resume)
	SET_RUNTIME_PM_OPS(byt_gpio_runtime_suspend, byt_gpio_runtime_resume,
			   NULL)
};

static const struct acpi_device_id byt_gpio_acpi_match[] = {
	{ "INT33B2", 0 },
	{ "INT33FC", 0 },
	{ }
};
MODULE_DEVICE_TABLE(acpi, byt_gpio_acpi_match);

static int byt_gpio_remove(struct platform_device *pdev)
{
	struct byt_gpio *vg = platform_get_drvdata(pdev);

	pm_runtime_disable(&pdev->dev);
	gpiochip_remove(&vg->chip);

	return 0;
}

static struct platform_driver byt_gpio_driver = {
	.probe          = byt_gpio_probe,
	.remove         = byt_gpio_remove,
	.driver         = {
		.name   = "byt_gpio",
		.pm	= &byt_gpio_pm_ops,
		.acpi_match_table = ACPI_PTR(byt_gpio_acpi_match),
	},
};

static int __init byt_gpio_init(void)
{
	return platform_driver_register(&byt_gpio_driver);
}
subsys_initcall(byt_gpio_init);

static void __exit byt_gpio_exit(void)
{
	platform_driver_unregister(&byt_gpio_driver);
}
module_exit(byt_gpio_exit);

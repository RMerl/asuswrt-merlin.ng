/*
 * Nexus GPIO(s) resolution API
 *
 * Copyright (C) 2015-2016, Broadcom Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * A copy of the GPL is available at
 * http://www.broadcom.com/licenses/GPLv2.php or from the Free Software
 * Foundation at https://www.gnu.org/licenses/ .
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/gpio.h>
#include <linux/bitmap.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/gpio/driver.h>
#include <linux/gpio/consumer.h>
#include <linux/slab.h>
#include <linux/module.h>

#include <linux/brcmstb/brcmstb.h>
#include <linux/brcmstb/gpio_api.h>

#include "gpio.h"
#define GIO_BANK_SIZE	0x20
#define GPIO_PER_BANK	32
#define GIO_DATA_OFFSET 4
#define GIO_DIR_OFFSET	8

/* The largest register bus aperture is 64MB so limit offsets to 26 bits */
#define BCHP_BUS_MASK	0x3FFFFFF

/* BCM2835 GPIO register offsets */
#define GPFSEL0		0x0	/* Function Select */
#define GPSET0		0x1c	/* Pin Output Set */
#define GPCLR0		0x28	/* Pin Output Clear */
#define GPLEV0		0x34	/* Pin Level */
#define GPEDS0		0x40	/* Pin Event Detect Status */
#define GPREN0		0x4c	/* Pin Rising Edge Detect Enable */
#define GPFEN0		0x58	/* Pin Falling Edge Detect Enable */
#define GPHEN0		0x64	/* Pin High Detect Enable */
#define GPLEN0		0x70	/* Pin Low Detect Enable */
#define GPAREN0		0x7c	/* Pin Async Rising Edge Detect */
#define GPAFEN0		0x88	/* Pin Async Falling Edge Detect */
#define GPPUD		0x94	/* Pin Pull-up/down Enable */
#define GPPUDCLK0	0x98	/* Pin Pull-up/down Enable Clock */
#define GPLAST		0xa0	/* Last offset conflicting with Linux driver */

struct bcm2835_pinctrl {
	struct device *dev;
	void __iomem *base;
};

struct brcmstb_gpio_ctl_list_ent {
	struct device_node *dn;
	struct list_head next;
};

static LIST_HEAD(brcmstb_gpio_ctl_list);
static const char *brcmstb_gpio_compats[] = {
	"brcm,brcmstb-gpio",
	"brcm,bcm2835-gpio",
};

static int brcmstb_gpio_chip_find(struct gpio_chip *chip, void *data)
{
	struct device_node *dn = data;

	if (chip->of_node == dn)
		return 1;

	return 0;
}

/* Keep an additional bitmap of which GPIOs are requested by Nexus to
 * maintain Linux/Nexus GPIO exclusive ownership managed via gpio_request().
 * We do want multiple consecutive calls to brcmstb_gpio_irq() not to fail
 * because the first one claimed ownernship already.
 */
static DECLARE_BITMAP(brcmstb_gpio_requested, ARCH_NR_GPIOS);

static int brcmstb_gpio_request(unsigned int gpio)
{
	int ret = 0;

	/* Request the GPIO to flag that Nexus now owns it, but
	 * also keep it requested in our local bitmap to avoid
	 * subsequent gpio_request() calls to the same GPIO
	 */
	if (!test_bit(gpio, brcmstb_gpio_requested)) {
		ret = gpio_request(gpio, "nexus");
		if (ret) {
			pr_err("%s: GPIO request failed\n", __func__);
			return ret;
		}

		/* We could get ownership, so flag it now */
		set_bit(gpio, brcmstb_gpio_requested);
	}

	return ret;
}

static struct gpio_chip *brcmstb_gpio_find_chip_by_addr(uint32_t addr,
							struct resource *res)
{
	struct brcmstb_gpio_ctl_list_ent *ent;
	struct device_node *dn;
	struct gpio_chip *gc;
	int ret;

	if (!res)
		return NULL;

	list_for_each_entry(ent, &brcmstb_gpio_ctl_list, next) {
		dn = ent->dn;
		ret = of_address_to_resource(dn, 0, res);
		if (ret) {
			pr_err("%s: unable to translate resource\n", __func__);
			continue;
		}

		if (res->flags != IORESOURCE_MEM) {
			pr_err("%s: invalid resource type\n", __func__);
			continue;
		}

		/* Verify address is in the resource range, if not, go to the
		 * other GPIO controllers
		 *
		 * NB: of_address_to_resource already performs the physical
		 * address transformation based on the "reg" and parent node's
		 * "ranges" property. The bus base address must be masked off
		 * for comparisons
		 */
		if (addr < (res->start & BCHP_BUS_MASK) ||
			addr >= (res->end & BCHP_BUS_MASK))
			continue;

		gc = gpiochip_find(dn, brcmstb_gpio_chip_find);
		if (!gc) {
			pr_err("%s: unable to find gpio chip\n", __func__);
			continue;
		}

		return gc;
	}

	return NULL;
}

static int brcmstb_gpio_find_base_by_addr(uint32_t addr, uint32_t mask,
					  uint32_t *start)
{
	struct gpio_chip *gc = NULL;
	struct resource res;
	uint32_t gc_start;
	int ret, bit, gpio, gpio_base, bank_offset, field_width = 1;

	if (IS_ENABLED(CONFIG_GPIO_BRCMSTB) && !gc) {
		gc = brcmstb_gpio_find_chip_by_addr(addr, &res);
		if (gc) {
			gpio_base = gc->base;
			gc_start = (uint32_t)(res.start & BCHP_BUS_MASK);
			bank_offset = (addr - gc_start) / GIO_BANK_SIZE;

			pr_debug("%s: xlate base=%d, offset=%d, gpio=%d\n",
				__func__, gpio_base, bank_offset,
				(bank_offset * GPIO_PER_BANK));

			gpio_base += bank_offset * GPIO_PER_BANK;
			gc_start += bank_offset * GIO_BANK_SIZE;
		}
	}

	if (IS_ENABLED(CONFIG_PINCTRL_BCM2835) && !gc) {
		gc = brcmstb_gpio_find_chip_by_addr(addr, &res);
		if (gc) {
			gpio_base = gc->base;
			gc_start = (uint32_t)(res.start & BCHP_BUS_MASK);

			bank_offset = addr - gc_start;
			if (bank_offset > GPLAST) {
				pr_err("%s: register offset 0x%x is not supported\n",
				       __func__, bank_offset);
				return -EPERM;
			} else if (bank_offset >= GPPUDCLK0) {
				bank_offset -= GPPUDCLK0;
				bank_offset >>= 2;
				gpio_base += bank_offset * 32;
			} else if (bank_offset >= GPPUD) {
				pr_err("%s: register offset 0x%x is not supported\n",
				       __func__, bank_offset);
				return -EPERM;
			} else if (bank_offset >= GPSET0) {
				/* These registers control 8 GPIO per byte */
				bank_offset -= GPSET0;
				bank_offset %= (GPCLR0 - GPSET0);
				gpio_base += bank_offset * 8;
			} else {
				/* These registers control 10 GPIO per word */
				field_width = 3;
				bank_offset >>= 2;
				gpio_base += bank_offset * 10;
			}

			pr_debug("%s: xlate base=%d\n",
				__func__, gpio_base);
		}
	}

	if (gc) {
		if (start)
			*start = gc_start;

		for (bit = 0, gpio = gpio_base;
		     bit + (field_width - 1) < 32;
		     bit += field_width, gpio++) {
			/* Ignore bits which are not in mask */
			if (!((((1 << field_width) - 1) << bit) & mask))
				continue;

			ret = brcmstb_gpio_request(gpio);
			if (ret < 0) {
				pr_err("%s: unable to request gpio %d\n",
					__func__, gpio);
				return ret;
			}
		}

		/* We got full access to the entire mask */

		return gpio_base;
	}

	pr_err("%s: addr is not in GPIO range\n", __func__);
	return -ENODEV;

}

static int bcm2835_gpio_update32(struct gpio_chip *gc, uint32_t offset,
				 uint32_t mask, uint32_t value)
{
	struct bcm2835_pinctrl *pc = gpiochip_get_data(gc);
	uint32_t ivalue;

	if (offset >= GPSET0 && offset < GPLEV0) {
		/* We don't want to read from these registers */
		writel(value & mask, pc->base + offset);
	} else {
		ivalue = readl(pc->base + offset);
		ivalue &= ~(mask);
		ivalue |= (value & mask);
		writel(ivalue, pc->base + offset);
	}

	return 0;
}

int brcmstb_gpio_update32(uint32_t addr, uint32_t mask, uint32_t value)
{
	struct gpio_chip *gc;
	int gpio_base, offset;
	unsigned long __maybe_unused flags;
	uint32_t start, __maybe_unused ivalue;
	void __iomem *__maybe_unused reg;

	/* Silently strip any higher order bits from the addr value passed
	 * to this function, so that regardless of whether or not it is a
	 * physical address it will be a Register Bus offset.
	 */
	addr &= BCHP_BUS_MASK;

	gpio_base = brcmstb_gpio_find_base_by_addr(addr, mask, &start);
	if (gpio_base < 0)
		return gpio_base;

	offset = addr - start;

	pr_debug("%s: offset=0x%08x mask=0x%08x, value=0x%08x\n",
		__func__, addr, mask, value);

	gc = gpiod_to_chip(gpio_to_desc(gpio_base));
	if (gc == NULL) {
		pr_err("%s: unable to resolve gpio chip\n", __func__);
		return -EPERM;
	}

#if IS_ENABLED(CONFIG_GPIO_BRCMSTB)
	reg = gc->reg_dat;
	if (reg) {
		spin_lock_irqsave(&gc->bgpio_lock, flags);
		ivalue = gc->read_reg(reg + offset - GIO_DATA_OFFSET);
		ivalue &= ~(mask);
		ivalue |= (value & mask);

		/* update shadows */
		switch (offset) {
		case GIO_DATA_OFFSET:
			gc->bgpio_data = ivalue;
			break;
		case GIO_DIR_OFFSET:
			gc->bgpio_dir = ~ivalue;
			break;
		default:
			break;
		}

		gc->write_reg(reg + offset - GIO_DATA_OFFSET, ivalue);
		spin_unlock_irqrestore(&gc->bgpio_lock, flags);

		return 0;
	}
#endif /* CONFIG_GPIO_BRCMSTB */

	if (IS_ENABLED(CONFIG_PINCTRL_BCM2835)) {
		return bcm2835_gpio_update32(gc, offset, mask, value);
	}

	pr_err("%s: unable to resolve GIO mapped address\n", __func__);
	return -EPERM;
}
EXPORT_SYMBOL(brcmstb_gpio_update32);

int brcmstb_gpio_irq(uint32_t addr, unsigned int shift)
{
	int gpio, ret;

	/* Silently strip any higher order bits from the addr value passed
	 * to this function, so that regardless of whether or not it is a
	 * physical address it will be a Register Bus offset.
	 */
	addr &= BCHP_BUS_MASK;

	gpio = brcmstb_gpio_find_base_by_addr(addr, (1 << shift), NULL);
	if (gpio < 0)
		return gpio;

	gpio += shift;

	ret = gpio_to_irq(gpio);
	if (ret < 0) {
		gpio_free(gpio);
		pr_err("%s: unable to map GPIO%d to irq, ret=%d\n",
		       __func__, gpio, ret);
	}

	return ret;
}
EXPORT_SYMBOL(brcmstb_gpio_irq);

static void brcmstb_gpio_free(unsigned int gpio)
{
    if (test_bit(gpio, brcmstb_gpio_requested)) {
        gpio_free(gpio);
        clear_bit(gpio, brcmstb_gpio_requested);
    }
}

void brcmstb_gpio_remove(void)
{
    unsigned i;

    for (i = 0; i < ARCH_NR_GPIOS; i++)
    {
        brcmstb_gpio_free(i);
    }
}
EXPORT_SYMBOL(brcmstb_gpio_remove);

static int brcmstb_gpio_cache_entry(struct device_node *dn)
{
	struct brcmstb_gpio_ctl_list_ent *ent;

	ent = kzalloc(sizeof(*ent), GFP_KERNEL);
	if (!ent)
		return -ENOMEM;

	INIT_LIST_HEAD(&ent->next);
	ent->dn = dn;

	pr_debug("%s: added GPIO %s\n", __func__, dn->full_name);

	list_add_tail(&ent->next, &brcmstb_gpio_ctl_list);

	return 0;
}

static int brcmstb_gpio_cache_init(void)
{
	struct device_node *dn;
	unsigned int i;
	int ret;

	for (i = 0; i < ARRAY_SIZE(brcmstb_gpio_compats); i++) {
		for_each_compatible_node(dn, NULL, brcmstb_gpio_compats[i]) {
			if (!of_device_is_available(dn))
				continue;

			ret = brcmstb_gpio_cache_entry(dn);
			if (ret)
				return ret;
		}
	}

	return 0;
}
module_init(brcmstb_gpio_cache_init);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Broadcom");
MODULE_DESCRIPTION("Broadcom STB Nexus GPIO APIs");

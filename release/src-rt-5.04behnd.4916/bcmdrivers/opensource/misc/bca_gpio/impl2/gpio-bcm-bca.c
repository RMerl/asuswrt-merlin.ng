/*
 * <:copyright-BRCM:2019:DUAL/GPL:standard
 * 
 *    Copyright (c) 2019 Broadcom 
 *    All Rights Reserved
 * 
 * Unless you and Broadcom execute a separate written software license
 * agreement governing use of this software, this software is licensed
 * to you under the terms of the GNU General Public License version 2
 * (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
 * with the following added to such license:
 * 
 *    As a special exception, the copyright holders of this software give
 *    you permission to link this software with independent modules, and
 *    to copy and distribute the resulting executable under terms of your
 *    choice, provided that you also meet, for each linked independent
 *    module, the terms and conditions of the license of that module.
 *    An independent module is a module which is not derived from this
 *    software.  The special exception does not apply to any modifications
 *    of the software.
 * 
 * Not withstanding the above, under no circumstances may you combine
 * this software in any way with any other Broadcom software provided
 * under a license other than the GPL, without Broadcom's express prior
 * written consent.
 * 
 * :> 
 */

#include <linux/bitops.h>
#include <linux/gpio/driver.h>
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/irq.h>   
#include <linux/of_address.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/irqdomain.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/pinctrl/consumer.h>
#include <linux/gpio/driver.h>
#include <linux/gpio.h>

/**
 * enum gio_reg_index - GIO block GPIO register index.
 * @GIO_REG_ODEN: Open drain enable register.
 * @GIO_REG_DATA: Data register.
 * @GIO_REG_IODIR: Input/output direction register.
 * @GIO_REG_EC: Edge insensitive register.
 * @GIO_REG_MASK: Interrupt mask register.
 * @GIO_REG_LEVEL: Interrupt level type register.
 * @GIO_REG_STAT: Interrupt status register.
 * @GIO_REG_COUNT: Total GIO GPIO register count.
 */
enum gio_reg_index {
	GIO_REG_ODEN = 0,
	GIO_REG_DATA,
	GIO_REG_IODIR,
	GIO_REG_EC,
	GIO_REG_EI,
	GIO_REG_MASK,
	GIO_REG_LEVEL,
	GIO_REG_STAT,
	GIO_REG_COUNT
};

#define MODULE_NAME                 "bcm-bca-gpio"
#define COMPATIBLE_GPIO             "brcm,bca-gpio"
#define COMPATIBLE_PROPERTY         "compatible"
#define GPIO_BANK_WIDTH_PROPERTY    "brcm,gpio-bank-widths"

/**
 * struct bcm_bca_gpio_priv: wrapper for holding driver data
 *		together.
 * @bank_list: Global bank list node.
 * @reg_base: Register base address of the controller.
 * @pdev: device instance representing the controller.
 * @irq_domain: Represent IRQ translation domains.
 * @parent_irq: Platform irq information.
 * @gpio_base: Representing base GPIO number.
 * @can_wake: Flag to wake events.
 * @wake_mask: Mask to vakeup events.
 * @parent_wake_irq: Representing irq wake information. 
 */
struct bcm_bca_gpio_priv {
	struct list_head bank_list;
	void __iomem *reg_base;
	void __iomem *access_reg;
	struct platform_device *pdev;
	struct irq_domain *irq_domain;
	int l2_gpio_irq;
	int gpio_base;
	int num_gpios;
};

/**
 * struct bcm_bca_gpio_bank: represent a controller pin-bank.
 * @node: Bank list head to track all bank information.
 * @id: Bank id.
 * @gc: GPIO chip of the bank.
 * @width: Bank width(Number of GPIO per bank).
 * @icg: Generic irq chip data structure.
 * @wake_active: Active wake flag to store irq mask.
 * @regs: Bank register storage.
 * @data: Generic data pointer to store any information.
 */
struct bcm_bca_gpio_bank {
	struct list_head node;
	int id;
	struct gpio_chip gc;
	struct bcm_bca_gpio_priv *parent_priv;
	u32 width;
	spinlock_t lock;
	struct irq_chip_generic *icg;
	void *data;
};

#define GIO_BANK_SIZE           (GIO_REG_COUNT * sizeof(u32))
#define GIO_BANK_OFF(bank, off)	(((bank) * GIO_BANK_SIZE) + (off * sizeof(u32)))
#define GIO_ODEN(bank)          GIO_BANK_OFF(bank, GIO_REG_ODEN)
#define GIO_DATA(bank)          GIO_BANK_OFF(bank, GIO_REG_DATA)
#define GIO_IODIR(bank)         GIO_BANK_OFF(bank, GIO_REG_IODIR)
#define GIO_EC(bank)            GIO_BANK_OFF(bank, GIO_REG_EC)
#define GIO_EI(bank)            GIO_BANK_OFF(bank, GIO_REG_EI)
#define GIO_MASK(bank)          GIO_BANK_OFF(bank, GIO_REG_MASK)
#define GIO_LEVEL(bank)         GIO_BANK_OFF(bank, GIO_REG_LEVEL)
#define GIO_STAT(bank)          GIO_BANK_OFF(bank, GIO_REG_STAT)

#define MAX_GPIO_PER_BANK		32
#define GPIO_BANK(gpio)			((gpio) >> 5)
#define BITS_PER_GPO			0x1


static inline struct bcm_bca_gpio_priv *
bcm_bca_gpio_gc_to_priv(struct gpio_chip *gc)
{
	struct bcm_bca_gpio_bank *bank = gpiochip_get_data(gc);

	return bank->parent_priv;
}

static unsigned long
__bcm_bca_gpio_get_active_irqs(struct bcm_bca_gpio_bank *bank)
{
	void __iomem *reg_base = bank->parent_priv->reg_base;

	return readl(reg_base + GIO_STAT(bank->id)) &
	       readl(reg_base + GIO_MASK(bank->id));
}

static unsigned long
bcm_bca_gpio_get_active_irqs(struct bcm_bca_gpio_bank *bank)
{
	unsigned long status;
	unsigned long flags;

	irq_gc_lock_irqsave(bank->icg, flags);
	status = __bcm_bca_gpio_get_active_irqs(bank);
	irq_gc_unlock_irqrestore(bank->icg, flags);

	return status;
}

static int bcm_bca_gpio_to_irq(struct gpio_chip *gc,
			unsigned int gc_offset)
{
	struct bcm_bca_gpio_priv *priv = bcm_bca_gpio_gc_to_priv(gc);
	/* gc_offset is relative to this gpio_chip; want real offset */
	int offset = gc_offset + (gc->base - priv->gpio_base);

	if (offset >= priv->num_gpios)
		return -ENXIO;
	return irq_create_mapping(priv->irq_domain, offset);
}

/* -------------------- IRQ chip functions -------------------- */

static int bcm_bca_gpio_hwirq_to_offset(irq_hw_number_t hwirq,
					struct bcm_bca_gpio_bank *bank)
{
	return hwirq - (bank->gc.base - bank->parent_priv->gpio_base);
}

static int bcm_bca_gpio_irq_set_type(struct irq_data *d, unsigned int type)
{
	struct irq_chip_generic *icg = irq_data_get_irq_chip_data(d);
	struct bcm_bca_gpio_bank *bank = icg->private;
	struct bcm_bca_gpio_priv *priv = bank->parent_priv;
	u32 mask = BIT(bcm_bca_gpio_hwirq_to_offset(d->hwirq, bank));
	u32 edge_insensitive, iedge_insensitive;
	u32 edge_config, iedge_config;
	u32 level, ilevel;
	unsigned long flags;

	switch (type) {
	case IRQ_TYPE_LEVEL_LOW:
		level = mask;
		edge_config = 0;
		edge_insensitive = 0;
		break;
	case IRQ_TYPE_LEVEL_HIGH:
		level = mask;
		edge_config = mask;
		edge_insensitive = 0;
		break;
	case IRQ_TYPE_EDGE_FALLING:
		level = 0;
		edge_config = 0;
		edge_insensitive = 0;
		break;
	case IRQ_TYPE_EDGE_RISING:
		level = 0;
		edge_config = mask;
		edge_insensitive = 0;
		break;
	case IRQ_TYPE_EDGE_BOTH:
		level = 0;
		edge_config = 0;  /* don't care, but want known value */
		edge_insensitive = mask;
		break;
	default:
		return -EINVAL;
	}

	spin_lock_irqsave(&bank->lock, flags);

	iedge_config = readl(priv->reg_base +
			GIO_EC(bank->id)) & ~mask;
	iedge_insensitive = readl(priv->reg_base +
			GIO_EI(bank->id)) & ~mask;
	ilevel = readl(priv->reg_base +
			GIO_LEVEL(bank->id)) & ~mask;

	writel(iedge_config | edge_config, priv->reg_base + GIO_EC(bank->id));
	writel(iedge_insensitive | edge_insensitive, priv->reg_base + GIO_EI(bank->id));
	writel(ilevel | level, priv->reg_base + GIO_LEVEL(bank->id));

	spin_unlock_irqrestore(&bank->lock, flags);
	return 0;
}

static void bcm_bca_gpio_irq_bank_handler(struct bcm_bca_gpio_bank *bank)
{
	struct bcm_bca_gpio_priv *priv = bank->parent_priv;
	struct irq_domain *domain = priv->irq_domain;
	int hwbase = bank->gc.base - priv->gpio_base;
	unsigned long status;
	unsigned int irq;

	while ((status = bcm_bca_gpio_get_active_irqs(bank))) {
		for_each_set_bit(irq, &status, 32) {
			if (irq >= bank->width)
				dev_warn(&priv->pdev->dev,
					 "IRQ for invalid GPIO (bank=%d, offset=%d)\n",
					 bank->id, irq);
			irq = irq_linear_revmap(domain, irq + hwbase);
			generic_handle_irq(irq);
		}
	}
}

/* Each UPG GIO block has one IRQ for all banks */
static void bcm_bca_gpio_irq_handler(struct irq_desc *desc)
{
	struct bcm_bca_gpio_priv *priv = irq_desc_get_handler_data(desc);
	struct irq_chip *chip = irq_desc_get_chip(desc);
	struct bcm_bca_gpio_bank *bank;

	/* Interrupts weren't properly cleared during probe */
	BUG_ON(!priv || !chip);

	chained_irq_enter(chip, desc);
	list_for_each_entry(bank, &priv->bank_list, node)
		bcm_bca_gpio_irq_bank_handler(bank);
	chained_irq_exit(chip, desc);
}

static int bcm_bca_gpio_request(struct gpio_chip *gc, unsigned offset)
{
	unsigned int pin;
	struct bcm_bca_gpio_priv *priv = bcm_bca_gpio_gc_to_priv(gc);

	if (offset >= gc->ngpio)
		return -EINVAL;

	pin = offset + (gc->base - priv->gpio_base);

    return pinctrl_gpio_request(pin);
}

static void bcm_bca_gpio_free(struct gpio_chip *gc, unsigned offset)
{
	unsigned int pin;
	struct bcm_bca_gpio_priv *priv = bcm_bca_gpio_gc_to_priv(gc);

	if (offset >= gc->ngpio)
		return;

	pin = offset + (gc->base - priv->gpio_base);

	pinctrl_gpio_free(pin);
}

static int bcm_bca_gpio_get_direction(struct gpio_chip *gc, unsigned gpio)
{
	struct bcm_bca_gpio_bank *bank = gpiochip_get_data(gc);
	struct bcm_bca_gpio_priv *priv = bcm_bca_gpio_gc_to_priv(gc);
	unsigned int val;
	unsigned long flags;

	spin_lock_irqsave(&bank->lock, flags);
	
	val = readl(priv->reg_base + GIO_IODIR(bank->id));
	val &= (1 << gpio);
	val >>= gpio;
	
	spin_unlock_irqrestore(&bank->lock, flags);

	return val ? GPIO_LINE_DIRECTION_IN : GPIO_LINE_DIRECTION_OUT;
}

static int bcm_bca_gpio_direction_input(struct gpio_chip *gc, unsigned gpio)
{
	struct bcm_bca_gpio_bank *bank = gpiochip_get_data(gc);
	struct bcm_bca_gpio_priv *priv = bcm_bca_gpio_gc_to_priv(gc);
	unsigned long flags;
	unsigned int val; 

	if (gpio >= gc->ngpio)
		return -EINVAL;

	spin_lock_irqsave(&bank->lock, flags);

	val = readl(priv->reg_base + GIO_IODIR(bank->id));
	val |= (1 << gpio);

	writel(val, priv->reg_base + GIO_IODIR(bank->id));

	spin_unlock_irqrestore(&bank->lock, flags);

    return 0;
}

static int bcm_bca_gpio_direction_output(struct gpio_chip *gc, unsigned gpio, int value)
{
	struct bcm_bca_gpio_bank *bank = gpiochip_get_data(gc);
	struct bcm_bca_gpio_priv *priv = bcm_bca_gpio_gc_to_priv(gc);
	unsigned long flags;
	unsigned int val; 

	if (gpio >= gc->ngpio)
		return -EINVAL;

	spin_lock_irqsave(&bank->lock, flags);

	val = readl(priv->reg_base + GIO_IODIR(bank->id));
	val &= ~(1 << gpio);
	writel(val, priv->reg_base + GIO_IODIR(bank->id));

	val = readl(priv->reg_base + GIO_DATA(bank->id));
	if (value){
		val |= (1 << gpio);
	}
    else {
		val &= ~(1 << gpio);
	}
	writel(val, priv->reg_base + GIO_DATA(bank->id));

	spin_unlock_irqrestore(&bank->lock, flags);

    return 0;
}

static int bcm_bca_gpio_get(struct gpio_chip *gc, unsigned gpio)
{
	struct bcm_bca_gpio_bank *bank = gpiochip_get_data(gc);
	struct bcm_bca_gpio_priv *priv = bcm_bca_gpio_gc_to_priv(gc);
	unsigned long flags;
	unsigned int val; 

	if (gpio >= gc->ngpio)
		return -EINVAL;

	spin_lock_irqsave(&bank->lock, flags);

	val = readl(priv->reg_base + GIO_DATA(bank->id));
	val &= (1 << gpio);
	val >>= gpio;

	spin_unlock_irqrestore(&bank->lock, flags);

	return val;
}

static void bcm_bca_gpio_set(struct gpio_chip *gc, unsigned gpio, int value)
{
	struct bcm_bca_gpio_bank *bank = gpiochip_get_data(gc);
	struct bcm_bca_gpio_priv *priv = bcm_bca_gpio_gc_to_priv(gc);
	unsigned long flags;
	unsigned int val = 0; 

	if (gpio >= gc->ngpio)
		return;

	spin_lock_irqsave(&bank->lock, flags);

	val = readl(priv->reg_base + GIO_DATA(bank->id));
	if (value) {
		val |= (1 << gpio);
	}
    else {
		val &= ~(1 << gpio);
	}
	writel(val, priv->reg_base + GIO_DATA(bank->id));

	spin_unlock_irqrestore(&bank->lock, flags);
}

static int bcm_bca_gpio_of_xlate(struct gpio_chip *gc,
		const struct of_phandle_args *gpiospec, u32 *flags)
{
	struct bcm_bca_gpio_priv *priv = bcm_bca_gpio_gc_to_priv(gc);
	struct bcm_bca_gpio_bank *bank = gpiochip_get_data(gc);
	int offset;

	if (gc->of_gpio_n_cells != 2) {
		WARN_ON(1);
		return -EINVAL;
	}

	if (WARN_ON(gpiospec->args_count < gc->of_gpio_n_cells))
		return -EINVAL;

	offset = gpiospec->args[0] - (gc->base - priv->gpio_base);
	if (offset >= gc->ngpio || offset < 0)
		return -EINVAL;

	if (unlikely(offset >= bank->width)) {
		dev_warn_ratelimited(&priv->pdev->dev,
			"Received request for invalid GPIO offset %d\n",
			gpiospec->args[0]);
	}

	if (flags)
		*flags = gpiospec->args[1];

	return offset;
}

static int bcm_bca_gpio_irq_setup(struct platform_device *pdev,
		struct bcm_bca_gpio_priv *priv)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct bcm_bca_gpio_bank *bank;
	struct irq_chip_generic *gc;
	struct irq_chip_type *ct;
	unsigned int clr = IRQ_NOREQUEST | IRQ_NOPROBE | IRQ_NOAUTOEN;
	unsigned int flags;
	int ret, hw_int;

	priv->irq_domain = irq_domain_add_linear(np, priv->num_gpios,
						&irq_generic_chip_ops,
						priv);
	if (!priv->irq_domain) {
		dev_err(dev, "Couldn't allocate IRQ domain\n");
		return -ENXIO;
	}

	flags = IRQ_GC_INIT_MASK_CACHE;
	ret = irq_alloc_domain_generic_chips(priv->irq_domain,
					     MAX_GPIO_PER_BANK, 1,
					     dev_name(dev),
					     handle_level_irq, clr, 0, flags);
	if (ret) {
		dev_err(&pdev->dev, "failed to allocate generic irq chip\n");
		goto out_free_domain;
	}

	list_for_each_entry(bank, &priv->bank_list, node) {
		hw_int = bank->id * MAX_GPIO_PER_BANK;
		gc = irq_get_domain_generic_chip(priv->irq_domain, hw_int);
		gc->reg_base = priv->reg_base + bank->id * GIO_BANK_SIZE;
		if (bank->width < sizeof(gc->unused) * 8) {
			gc->unused = -((typeof(gc->unused))1 << bank->width);
		}
		gc->private = bank;
		ct = gc->chip_types;
		ct->regs.mask = GIO_MASK(0);
		ct->regs.ack = GIO_STAT(0);
		ct->chip.irq_ack = irq_gc_ack_set_bit;
		ct->chip.irq_mask = irq_gc_mask_clr_bit;
		ct->chip.irq_unmask = irq_gc_mask_set_bit;
		ct->chip.irq_set_type = bcm_bca_gpio_irq_set_type;
		ct->chip.irq_disable = irq_gc_mask_clr_bit;
		/* Ensure that interrupts are masked when changing type */
		ct->chip.flags = IRQCHIP_SET_TYPE_MASKED;
		bank->icg = gc;
	}

	irq_set_chained_handler_and_data(priv->l2_gpio_irq,
					 bcm_bca_gpio_irq_handler, priv);

	return 0;

out_free_domain:
	irq_domain_remove(priv->irq_domain);

	return ret;
}

static int bcm_bca_gpio_add_data(struct device *dev,
			struct bcm_bca_gpio_bank *bank, int gpio_base)
{
	struct bcm_bca_gpio_priv *priv = bank->parent_priv;
	struct device_node *np = dev->of_node;
	void __iomem *reg_base = priv->reg_base;
	struct gpio_chip *gc = &bank->gc;
	int ret;

	gc->request = bcm_bca_gpio_request,
	gc->free = bcm_bca_gpio_free,
	gc->get_direction = bcm_bca_gpio_get_direction,
	gc->direction_input = bcm_bca_gpio_direction_input,
	gc->direction_output = bcm_bca_gpio_direction_output,
	gc->get = bcm_bca_gpio_get,
	gc->set = bcm_bca_gpio_set,
	gc->set_config = gpiochip_generic_config,
	gc->of_node = np;
	gc->owner = THIS_MODULE;
	gc->label = np->full_name;
	gc->base = gpio_base;
	gc->of_gpio_n_cells = 2;
	gc->of_xlate = bcm_bca_gpio_of_xlate;
	if (priv->l2_gpio_irq >= 0)
		gc->to_irq = bcm_bca_gpio_to_irq;

	spin_lock_init(&bank->lock);

	/*
	 * Mask all interrupts by default
	 */
	writel(0, reg_base + GIO_MASK(bank->id));

	ret = gpiochip_add_data(gc, bank);
	if (ret) {
		dev_err(dev, "Could not add gpiochip for bank %d\n",
				bank->id);
		return ret;
	}
	return 0;
}

static int bcm_bca_gpio_register(struct platform_device *pdev,
		struct bcm_bca_gpio_priv *priv, struct resource *res)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct property *prop;
	const __be32 *p;
	u32 bank_width;
	int num_banks = 0;
	int ret;
	static int gpio_base;
	struct bcm_bca_gpio_bank *bank;
	struct gpio_chip *gc;
	void __iomem* access_reg = priv->access_reg;
 
	priv->gpio_base = gpio_base;

	of_property_for_each_u32(np, GPIO_BANK_WIDTH_PROPERTY, prop, p,
			bank_width) {
		u32 access_mask = 0xffffffff;

		access_mask >>= (MAX_GPIO_PER_BANK - bank_width);

		//enable access to each GPIO in current bank
		writel(access_mask, access_reg); 

		bank = devm_kzalloc(dev, sizeof(*bank), GFP_KERNEL);
		if (!bank) {
			dev_err(dev, "bank memory allocate failure\n");
			ret = -ENOMEM;
			goto out;
		}
		bank->parent_priv = priv;
		bank->id = num_banks;
		if (bank_width <= 0 || bank_width > MAX_GPIO_PER_BANK) {
			dev_err(dev, "Invalid bank width %d\n", bank_width);
			ret = -EINVAL;
			goto out;
		} else
			bank->width = bank_width;

		gc = &bank->gc;
		gc->ngpio = bank->width;
		ret = bcm_bca_gpio_add_data(dev, bank, gpio_base);
		if (ret) {
			dev_err(dev, "GPIO chip add data failed\n");
			goto out;
		}
		gpio_base += gc->ngpio;
		dev_dbg(dev, "bank=%d, base=%d, ngpio=%d, width=%d\n",
			bank->id, gc->base, gc->ngpio, bank->width);

		/* Everything looks good, so add bank to list */
		list_add(&bank->node, &priv->bank_list);
		num_banks++;
		access_reg += 4;
	}
	priv->num_gpios = gpio_base - priv->gpio_base;

	if (priv->l2_gpio_irq >= 0) {
		ret = bcm_bca_gpio_irq_setup(pdev, priv);
		if (ret)
			goto out;
	}

	dev_info(dev, "Registered %d banks (GPIO(s): %d-%d)\n",
			num_banks, priv->gpio_base, gpio_base - 1);

out:
	return ret;
}

static int bcm_bca_gpio_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	void __iomem *reg_base, *access_reg;
	struct bcm_bca_gpio_priv *priv;
	struct resource *res;
	int ret;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	platform_set_drvdata(pdev, priv);
	INIT_LIST_HEAD(&priv->bank_list);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res){
		return -EINVAL;
	}

	reg_base = devm_ioremap_resource(dev, res);
	if (IS_ERR(reg_base))
		return PTR_ERR(reg_base);

	priv->reg_base = reg_base;
	priv->pdev = pdev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!res){
		return -EINVAL;
	}

	access_reg = devm_ioremap_resource(dev, res);
	if (IS_ERR(access_reg))
		return PTR_ERR(access_reg);

	priv->access_reg = access_reg;

	priv->l2_gpio_irq = platform_get_irq(pdev, 0);
	if (priv->l2_gpio_irq <= 0) {
		dev_err(dev, "Couldn't get IRQ");
		return -ENOENT;
	}

	ret = bcm_bca_gpio_register(pdev, priv, res);
	if (ret)
		return ret;

	return 0;
}

static const struct of_device_id bcm_bca_gpio_of_match[] = {
	{ .compatible = "brcm,bca-gpio-smc" },
	{},
};

MODULE_DEVICE_TABLE(of, bcm_bca_gpio_of_match);

static struct platform_driver bcm_bca_gpio_driver = {
	.driver = {
		.name = MODULE_NAME,
		.of_match_table = bcm_bca_gpio_of_match,
	},
	.probe = bcm_bca_gpio_probe,
};

static int __init bcm_bca_gpio_init(void)
{
	int rc = 0;

	rc = platform_driver_register(&bcm_bca_gpio_driver);
	if (rc) {
		pr_err("%s: Failed to register platform driver.\n",
			   MODULE_NAME);
		return rc;
	}
	return 0;
}
postcore_initcall(bcm_bca_gpio_init);

static void __exit bcm_bca_gpio_exit(void)
{
	platform_driver_unregister(&bcm_bca_gpio_driver);
}
module_exit(bcm_bca_gpio_exit);

MODULE_LICENSE("GPL v2");

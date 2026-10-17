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

#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/module.h>
#include <linux/version.h>

#if defined(PERIPH_INTR)
extern int bcm_bca_periph_intr_init(struct platform_device *pdev);
#endif

#define GPIO_TO_IDX(gpio)   ((gpio) >> 5)
#define GPIO_TO_MASK(gpio)  (1<<((gpio) & 0x1f))
#define GPIO_TO_SHIFT(gpio) ((gpio) & 0x1f)

#define EXT_INT_NOT_MAPPED   0xFFFF

#define EI_CLEAR_SHFT                   0
#define EI_SENSE_SHFT                   8
#define EI_INSENS_SHFT                  16
#define EI_LEVEL_SHFT                   24
#define EXT_IRQ_CTRL_REG                0x1c

#define EI_STATUS_MASK                  0xff
#define EXT_IRQ_STATUS_REG              0x20

#define EXT_IRQ_SET_REG                 0x24
#define EXT_IRQ_CLEAR_REG               0x28
#define EXT_IRQ_MASK_STATUS_REG         0x2c
#define EXT_IRQ_MASK_SET_REG            0x30
#define EXT_IRQ_MASK_CLEAR_REG          0x34

#define EXT_IRQ_SLOT_SIZE               16
#define EXT_IRQ_MUX_SEL0_SHIFT          4
#define EXT_IRQ_MUX_SEL0_MASK           0xf
#define EXT_IRQ_MUX_SEL0_REG            0x40

#define EXT_IRQ_MUX_SEL1_SHIFT          4
#define EXT_IRQ_MUX_SEL1_MASK           0xf
#define EXT_IRQ_MUX_SEL1_REG            0x44

struct bcm_bca_gpio {
    void __iomem *gpio_dir_base;
    void __iomem *gpio_data_base;
    void __iomem *gpio_intr_base;
    u32 num_intr;
    bool clear_workarround;
    spinlock_t lock;
    struct gpio_chip gpio_chip;
    struct irq_domain *irq_domain;
    struct bca_irq_map *irq_map;
    struct platform_device *pdev;
};

struct bca_irq_map {
    /* external interrupt index. Up to bca_gpio->num_intr-1 */
    int irq_idx;
    /*
     * GPIO that is mapped to the external irq. The gpio interrupt
     * controller's irq_domain hw_irq is same as gpio number
     */
    unsigned int gpio;
    /* linux interrupt number */
    unsigned int virq;
    /* parent linux interrupt number */
    int parent_virq;
    irq_hw_number_t parent_hwirq;
    /* Used in the interrupt handler */
    struct bcm_bca_gpio *bca_gpio;
};

static struct bcm_bca_gpio *bcm_bca_gpio = NULL;

static inline struct bcm_bca_gpio *to_bca_gpio(struct gpio_chip *chip)
{
    return container_of(chip, struct bcm_bca_gpio, gpio_chip);
}

static int bcm_bca_gpio_direction_input(struct gpio_chip *chip, unsigned gpio)
{
    struct bcm_bca_gpio *bca_gpio;
    volatile uint32_t *gpio_dir;
    unsigned long flags;

    bca_gpio = to_bca_gpio(chip);
    gpio_dir = (volatile uint32_t *)bca_gpio->gpio_dir_base;
    spin_lock_irqsave(&bca_gpio->lock, flags);

    gpio_dir[GPIO_TO_IDX(gpio)] &= ~GPIO_TO_MASK(gpio);

    spin_unlock_irqrestore(&bca_gpio->lock, flags);

    return 0;
}

static int bcm_bca_gpio_direction_output(struct gpio_chip *chip, unsigned gpio, int value)
{
    struct bcm_bca_gpio *bca_gpio;
    volatile uint32_t *gpio_dir;
    volatile uint32_t *gpio_data;
    unsigned long flags;

    bca_gpio = to_bca_gpio(chip);
    gpio_dir = (volatile uint32_t *)bca_gpio->gpio_dir_base;
    gpio_data = (volatile uint32_t *)bca_gpio->gpio_data_base;
    spin_lock_irqsave(&bca_gpio->lock, flags);

    gpio_dir[GPIO_TO_IDX(gpio)] |= GPIO_TO_MASK(gpio);

    if (value)
        gpio_data[GPIO_TO_IDX(gpio)] |= GPIO_TO_MASK(gpio);
    else
        gpio_data[GPIO_TO_IDX(gpio)] &= ~GPIO_TO_MASK(gpio);

    spin_unlock_irqrestore(&bca_gpio->lock, flags);

    return 0;
}

static int bcm_bca_gpio_get_direction(struct gpio_chip *chip, unsigned gpio)
{
    struct bcm_bca_gpio *bca_gpio;
    volatile uint32_t *gpio_dir;
    u32 val;

    bca_gpio = to_bca_gpio(chip);
    gpio_dir = (volatile uint32_t *)bca_gpio->gpio_dir_base;

    val = (gpio_dir[GPIO_TO_IDX(gpio)] & GPIO_TO_MASK(gpio)) >> GPIO_TO_SHIFT(gpio);

    return val ? GPIOF_DIR_OUT : GPIOF_DIR_IN;
}

static int bcm_bca_gpio_get(struct gpio_chip *chip, unsigned gpio)
{
    struct bcm_bca_gpio *bca_gpio;
    volatile uint32_t *gpio_data;
    u32 val;

    bca_gpio = to_bca_gpio(chip);
    gpio_data = (volatile uint32_t *)bca_gpio->gpio_data_base;

    val = (gpio_data[GPIO_TO_IDX(gpio)] & GPIO_TO_MASK(gpio)) >> GPIO_TO_SHIFT(gpio);

    return (int)val;
}

static void bcm_bca_gpio_set(struct gpio_chip *chip, unsigned gpio, int value)
{
    struct bcm_bca_gpio *bca_gpio;
    volatile uint32_t *gpio_data;
    unsigned long flags;

    bca_gpio = to_bca_gpio(chip);
    gpio_data = (volatile uint32_t *)bca_gpio->gpio_data_base;
    spin_lock_irqsave(&bca_gpio->lock, flags);


    if (value)
        gpio_data[GPIO_TO_IDX(gpio)] |= GPIO_TO_MASK(gpio);
    else
        gpio_data[GPIO_TO_IDX(gpio)] &= ~GPIO_TO_MASK(gpio);

    spin_unlock_irqrestore(&bca_gpio->lock, flags);
}

static int bcm_bca_gpio_request(struct gpio_chip *chip, unsigned offset)
{
    struct bcm_bca_gpio *bca_gpio;
    bca_gpio = to_bca_gpio(chip);
    dev_dbg(&bca_gpio->pdev->dev, "bcm_bca_gpio_request called for GPIO %d\n", offset);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0)
    return pinctrl_gpio_request(offset);
#else
    return pinctrl_request_gpio(offset);
#endif
}

static void bcm_bca_gpio_free(struct gpio_chip *chip, unsigned offset)
{
    struct bcm_bca_gpio *bca_gpio;
    bca_gpio = to_bca_gpio(chip);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0)
    pinctrl_gpio_free(offset);
#else
    pinctrl_free_gpio(offset);
#endif
}

static inline int gpio_to_extirq(struct bcm_bca_gpio *bca_gpio, unsigned gpio)
{
    int i = 0;

    while (bca_gpio->irq_map[i].gpio != gpio && i < bca_gpio->num_intr)
        i++;
    if (i == bca_gpio->num_intr) {
       i = -1;
        dev_err(&bca_gpio->pdev->dev,
            "gpio %d not mapped to any ext irq\n", gpio);
    }
    return i;
}

static inline int virq_to_extirq(struct bcm_bca_gpio *bca_gpio, unsigned int virq)
{
    int i = 0;

    while (bca_gpio->irq_map[i].virq != virq && i < bca_gpio->num_intr)
        i++;
    if (i == bca_gpio->num_intr) {
        i = -1;
        dev_err(&bca_gpio->pdev->dev,
            "virq %d not mapped to any ext irq\n", virq);
    }
    return i;
}


int bcm_bca_map_extintr(struct bcm_bca_gpio *bca_gpio, unsigned int virq,
               irq_hw_number_t hwirq)
{
    void __iomem *reg =  bca_gpio->gpio_intr_base;
    int sel0, sel1;
    u32 mask, value, data;
    unsigned long flags;
    u32 gpio = (u32)hwirq;
    int ext = 0;

    spin_lock_irqsave(&bca_gpio->lock, flags);
    while ((ext < bca_gpio->num_intr) &&
        (bca_gpio->irq_map[ext].gpio != EXT_INT_NOT_MAPPED))
            ext++;

    if (ext == bca_gpio->num_intr) {
        spin_unlock_irqrestore(&bca_gpio->lock, flags);
        dev_err(&bca_gpio->pdev->dev,
            "No free External Interrupt found for request virq %d gpio %ld\n",
            virq, hwirq);
        return -ENXIO;
    }

    bca_gpio->irq_map[ext].gpio = gpio;
    bca_gpio->irq_map[ext].virq = virq;

    sel0 = gpio % EXT_IRQ_SLOT_SIZE; // select one gpio pin in the slot
    sel1 = gpio / EXT_IRQ_SLOT_SIZE; // select the slot

    mask = ~(EXT_IRQ_MUX_SEL0_MASK<<(EXT_IRQ_MUX_SEL0_SHIFT * ext));
    value = sel0<<(EXT_IRQ_MUX_SEL0_SHIFT * ext);
    data = readl(reg + EXT_IRQ_MUX_SEL0_REG);
    data &= mask;
    data |= value;
    writel(data, reg + EXT_IRQ_MUX_SEL0_REG);

    mask = ~(EXT_IRQ_MUX_SEL1_MASK<<(EXT_IRQ_MUX_SEL1_SHIFT * ext));
    value = sel1<<(EXT_IRQ_MUX_SEL1_SHIFT * ext);
    data = readl(reg + EXT_IRQ_MUX_SEL1_REG);
    data &= mask;
    data |= value;
    writel(data, reg + EXT_IRQ_MUX_SEL1_REG);

    spin_unlock_irqrestore(&bca_gpio->lock, flags);
    dev_info(&bca_gpio->pdev->dev, "GPIO %d is mapped to ExtIntr_%d [virq %d]\n",
        gpio, ext, virq);

    return 0;
}

void bcm_bca_unmap_extintr(struct bcm_bca_gpio *bca_gpio, unsigned int virq)
{
    unsigned long flags;
    u32 mask, value, data;
    void __iomem *reg;
    int ext;

    reg = bca_gpio->gpio_intr_base;
    ext = virq_to_extirq(bca_gpio, virq);

    spin_lock_irqsave(&bca_gpio->lock, flags);
    bca_gpio->irq_map[ext].gpio = EXT_INT_NOT_MAPPED;

    mask = ~(EXT_IRQ_MUX_SEL0_MASK<<(EXT_IRQ_MUX_SEL0_SHIFT * ext));
    value = 0xf << (EXT_IRQ_MUX_SEL0_SHIFT * ext);
    data = readl(reg + EXT_IRQ_MUX_SEL0_REG);
    data &= mask;
    data |= value;
    writel(data, reg + EXT_IRQ_MUX_SEL0_REG);

    mask = ~(EXT_IRQ_MUX_SEL1_MASK<<(EXT_IRQ_MUX_SEL1_SHIFT * ext));
    value = 0xf << (EXT_IRQ_MUX_SEL1_SHIFT * ext);
    data = readl(reg + EXT_IRQ_MUX_SEL1_REG);
    data &= mask;
    data |= value;
    writel(data, reg + EXT_IRQ_MUX_SEL1_REG);
    spin_unlock_irqrestore(&bca_gpio->lock, flags);
}

static void bcm_bca_gpio_irq_reset(struct bcm_bca_gpio *bca_gpio)
{
    void __iomem *reg;
    reg = bca_gpio->gpio_intr_base;

    /* set the mux register to invalid selection to avoid default of gpio 0 */
    writel(0xffffffff, reg + EXT_IRQ_MUX_SEL0_REG);
    writel(0xffffffff, reg + EXT_IRQ_MUX_SEL1_REG);

    writel(0xffffffff, reg + EXT_IRQ_CLEAR_REG);
}

static void bcm_bca_gpio_irq_ack(struct irq_data *d)
{
    struct bcm_bca_gpio *bca_gpio;
    unsigned long flags;
    void __iomem *reg;
    int ext;
    u32 data;

    bca_gpio = irq_data_get_irq_chip_data(d);
    reg = bca_gpio->gpio_intr_base;
    ext = gpio_to_extirq(bca_gpio, d->hwirq);

    spin_lock_irqsave(&bca_gpio->lock, flags);

    if(bca_gpio->clear_workarround) {
        do {
            data = readl(reg + EXT_IRQ_CTRL_REG);
            writel(data | (0xff << EI_CLEAR_SHFT), reg + EXT_IRQ_CTRL_REG);
            writel(1 << ext, reg + EXT_IRQ_CLEAR_REG);
        } while (((readl(reg + EXT_IRQ_STATUS_REG) & (1 << ext)) & EI_STATUS_MASK));
    } else {
        writel(1 << ext, reg + EXT_IRQ_CLEAR_REG);
        writel(~(1 << ext), reg + EXT_IRQ_CLEAR_REG);
    }
    spin_unlock_irqrestore(&bca_gpio->lock, flags);
}

static void bcm_bca_gpio_irq_mask(struct irq_data *d)
{
    struct bcm_bca_gpio *bca_gpio;
    unsigned long flags;
    void __iomem *reg;
    int ext;

    bca_gpio = irq_data_get_irq_chip_data(d);
    reg = bca_gpio->gpio_intr_base;
    ext = gpio_to_extirq(bca_gpio, d->hwirq);

    spin_lock_irqsave(&bca_gpio->lock, flags);
    writel(1 << ext, reg + EXT_IRQ_MASK_CLEAR_REG);
    spin_unlock_irqrestore(&bca_gpio->lock, flags);
}

static void bcm_bca_gpio_irq_unmask(struct irq_data *d)
{
    struct bcm_bca_gpio *bca_gpio;
    unsigned long flags;
    void __iomem *reg;
    int ext;

    bca_gpio = irq_data_get_irq_chip_data(d);
    reg = bca_gpio->gpio_intr_base;
    ext = gpio_to_extirq(bca_gpio, d->hwirq);

    spin_lock_irqsave(&bca_gpio->lock, flags);
    writel(1 << ext, reg + EXT_IRQ_MASK_SET_REG);
    spin_unlock_irqrestore(&bca_gpio->lock, flags);
}

static int bcm_bca_gpio_irq_set_type(struct irq_data *d, unsigned int type)
{
    int is_level = 0, det_sense = 0, both_edge = 0;
    struct bcm_bca_gpio *bca_gpio;
    u32 mask, value, data;
    unsigned long flags;
    void __iomem *reg;
    int ext;

    bca_gpio = irq_data_get_irq_chip_data(d);
    reg = bca_gpio->gpio_intr_base;
    ext = gpio_to_extirq(bca_gpio, d->hwirq);

    switch (type & IRQ_TYPE_SENSE_MASK) {
    case IRQ_TYPE_EDGE_RISING:
        is_level = 0;
        det_sense = 1;
        both_edge = 0;
        break;

    case IRQ_TYPE_EDGE_FALLING:
        is_level = 0;
        det_sense = 0;
        both_edge = 0;
        break;

    case IRQ_TYPE_EDGE_BOTH:
        is_level = 0;
        det_sense = 1;
        both_edge = 1;
        break;

    case IRQ_TYPE_LEVEL_HIGH:
        is_level = 1;
        det_sense = 1;
        both_edge = 0;
        break;

    case IRQ_TYPE_LEVEL_LOW:
        is_level = 1;
        det_sense = 0;
        both_edge = 0;
        break;

    default:
        dev_err(bca_gpio->gpio_chip.parent,
            "Invalid BCM GPIO irq type 0x%x\n", type);
        return -EINVAL;
    }

    if (type & (IRQ_TYPE_LEVEL_LOW | IRQ_TYPE_LEVEL_HIGH))
        irq_set_handler_locked(d, handle_level_irq);
    else if (type & (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING))
        irq_set_handler_locked(d, handle_edge_irq);

    mask = ~((0x1 << (EI_LEVEL_SHFT + ext)) |
            (0x1 << (EI_SENSE_SHFT + ext)) |
            (0x1 << (EI_INSENS_SHFT + ext)));
    value = (is_level << (EI_LEVEL_SHFT + ext)) |
            (det_sense << (EI_SENSE_SHFT + ext)) |
            (both_edge << (EI_INSENS_SHFT + ext));

    if (bca_gpio->clear_workarround)
        value |= (1 << (EI_CLEAR_SHFT + ext));

    spin_lock_irqsave(&bca_gpio->lock, flags);
    data = readl(reg + EXT_IRQ_CTRL_REG);
    data &= mask;
    data |= value;
    writel(data, reg + EXT_IRQ_CTRL_REG);
    spin_unlock_irqrestore(&bca_gpio->lock, flags);

    return 0;
}

static void bcm_bca_gpio_irq_handler(struct irq_desc *desc)
{
    struct bca_irq_map *irq_map = irq_desc_get_handler_data(desc);
    struct bcm_bca_gpio *bca_gpio = irq_map->bca_gpio;
    struct irq_chip *chip = irq_desc_get_chip(desc);

    chained_irq_enter(chip, desc);

    generic_handle_domain_irq(bca_gpio->irq_domain, irq_map->gpio);

    chained_irq_exit(chip, desc);
}

static int bcm_bca_gpio_to_irq(struct gpio_chip *chip, unsigned gpio)
{
    struct bcm_bca_gpio *bca_gpio;
    unsigned int virq;

    bca_gpio = gpiochip_get_data(chip);
    if (gpio >= bca_gpio->gpio_chip.ngpio)
        return -ENXIO;
    if (!bca_gpio->irq_domain)
        return -ENXIO;  
    virq = irq_create_mapping(bca_gpio->irq_domain, gpio);

    return virq;
}

static struct irq_chip bca_gpio_irq_chip = {
    .name = "bcm-bca-gpio",
    .irq_ack = bcm_bca_gpio_irq_ack,
    .irq_mask = bcm_bca_gpio_irq_mask,
    .irq_unmask = bcm_bca_gpio_irq_unmask,
    .irq_set_type = bcm_bca_gpio_irq_set_type,
};

static int bcm_bca_gpio_irq_map(struct irq_domain *d, unsigned int virq,
                 irq_hw_number_t hwirq)
{
    int ret;

    ret = bcm_bca_map_extintr((struct bcm_bca_gpio *)d->host_data, virq, hwirq);
    if (ret < 0)
        return ret;

    ret = irq_set_chip_data(virq, d->host_data);
    if (ret < 0)
        return ret;

    irq_set_chip_and_handler(virq, &bca_gpio_irq_chip, handle_simple_irq);
    irq_set_noprobe(virq);

    return 0;
}

static void bcm_bca_gpio_irq_unmap(struct irq_domain *d, unsigned int virq)
{
    irq_set_chip_and_handler(virq, NULL, NULL);
    irq_set_chip_data(virq, NULL);

    bcm_bca_unmap_extintr((struct bcm_bca_gpio *)d->host_data, virq);
}

static const struct irq_domain_ops bca_irq_ops = {
    .map = bcm_bca_gpio_irq_map,
    .unmap = bcm_bca_gpio_irq_unmap,
    .xlate = irq_domain_xlate_twocell,
};

static struct gpio_chip template_chip = {
    .label = "bcm-bca-gpio",
    .owner = THIS_MODULE,
    .request = bcm_bca_gpio_request,
    .free = bcm_bca_gpio_free,
    .get_direction = bcm_bca_gpio_get_direction,
    .direction_input = bcm_bca_gpio_direction_input,
    .direction_output = bcm_bca_gpio_direction_output,
    .get = bcm_bca_gpio_get,
    .set = bcm_bca_gpio_set,
    .set_config = gpiochip_generic_config,
    .to_irq = bcm_bca_gpio_to_irq,
    .base = 0,
};

static struct of_device_id const bcm_bca_gpio_of_match[] = {
    { .compatible = "brcm,bca-gpio" },
    {}
};

MODULE_DEVICE_TABLE(of, bcm_bca_gpio_of_match);

static int bcm_bca_gpio_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    const struct of_device_id *match;
    struct irq_data *irq_data;
    struct resource *res_dir;
    struct resource *res_data;
    struct resource *res_intr;
    struct gpio_chip *chip;
    struct bcm_bca_gpio *bca_gpio = NULL;
    struct bca_irq_map *irq_map = NULL;
    int ret, i;
    unsigned int ngpio = 0, num_intr = 0;
    bool use_irq = false;

    match = of_match_device(bcm_bca_gpio_of_match, dev);
    if (!match)
    {
        dev_err(dev, "Failed to find gpio controller\n");
        return -ENODEV;
    }

    bca_gpio = devm_kzalloc(dev, sizeof(*bca_gpio), GFP_KERNEL);
    if (!bca_gpio)
    {
        ret = -ENOMEM;
        goto error;
    }

    bca_gpio->gpio_chip = template_chip;
    chip = &bca_gpio->gpio_chip;

    if (of_property_read_u32(pdev->dev.of_node, "ngpios", &ngpio))
    {
        dev_err(&pdev->dev, "Missing ngpios OF property\n");
        ret = -EINVAL;
        goto error;
    }

    chip->ngpio = ngpio;

    bca_gpio->pdev = pdev;
    platform_set_drvdata(pdev, bca_gpio);
    chip->of_node = dev->of_node;

    res_dir = platform_get_resource_byname(pdev, IORESOURCE_MEM, "gpio-dir");
    if (!res_dir)
    {
        dev_err(dev, "Failed to find gpio-dir resource\n");
        ret = -EINVAL;
        goto error;
    }

    bca_gpio->gpio_dir_base = devm_ioremap_resource(dev, res_dir);
    if (IS_ERR(bca_gpio->gpio_dir_base))
    {
        dev_err(dev, "Failed to map the gpio-dir resource\n");
        ret = -ENXIO;
        goto error;
    }

    res_data = platform_get_resource_byname(pdev, IORESOURCE_MEM, "gpio-data");
    if (!res_data)
    {
        dev_err(dev, "Failed to find gpio-data resource\n");
        ret = -EINVAL;
        goto error;
    }

    bca_gpio->gpio_data_base = devm_ioremap_resource(dev, res_data);
    if (IS_ERR(bca_gpio->gpio_data_base))
    {
        dev_err(dev, "Failed to map the gpio-data resource\n");
        ret = -ENXIO;
        goto error;
    }

    res_intr = platform_get_resource_byname(pdev, IORESOURCE_MEM, "gpio-intr");
    if (res_intr) {
        bca_gpio->gpio_intr_base = devm_ioremap_resource(dev, res_intr);
        if (IS_ERR(bca_gpio->gpio_intr_base)) {
            dev_err(dev, "Failed to map the gpio-data resource\n");
            ret = -ENXIO;
            goto error;
        }

        bca_gpio->irq_domain = irq_domain_add_linear(dev->of_node,
            chip->ngpio, &bca_irq_ops, bca_gpio);
        if (!bca_gpio->irq_domain) {
            dev_err(dev, "Couldn't allocate IRQ domain\n");
            ret = -ENXIO;
            goto error;
        }

        if (of_property_read_u32(dev->of_node, "num_ext_intr", &num_intr)) {
            dev_err(&pdev->dev, "Missing num_ext_intr OF property\n");
            ret = -EINVAL;
            goto error;
        }

        bca_gpio->clear_workarround =
            of_property_read_bool(dev->of_node, "clear_workarround");

        bca_gpio->num_intr = num_intr;
        bca_gpio->irq_map =
            devm_kzalloc(dev, sizeof(struct bca_irq_map)*num_intr, GFP_KERNEL);
        if (!bca_gpio->irq_map) {
            ret = -ENOMEM;
            goto error;
        }

        for (i = 0; i < bca_gpio->num_intr; i++) {
            irq_map = &bca_gpio->irq_map[i];
            irq_map->irq_idx = i;
            irq_map->gpio = EXT_INT_NOT_MAPPED;
            irq_map->parent_virq = platform_get_irq(pdev, i);
            if (irq_map->parent_virq < 0) {
                dev_err(dev, "Couldn't get GPIO IRQ for idx %d\n", i);
                ret = -ENOENT;
                goto error;
            }
            irq_data = irq_get_irq_data(irq_map->parent_virq);
            if (irq_data) {
                irq_map->parent_hwirq = irqd_to_hwirq(irq_data);
                dev_info(&pdev->dev, "Ext_Int_%d HWIrq %ld virq %d\n",
                    i, irq_map->parent_hwirq, irq_map->parent_virq);
            } else {
                dev_err(dev, "Couldn't get irqdata for parent virq %d\n", 
                    irq_map->parent_virq);
                ret = -ENOENT;
                goto error;
            }
            irq_map->bca_gpio = bca_gpio;
        }

        bcm_bca_gpio_irq_reset(bca_gpio);
        use_irq = true;
    }

    dev_info(&pdev->dev, "Setting up BCA GPIO controller %s\n",
          use_irq ? "with interrupt support" : "");
    ret = devm_gpiochip_add_data(dev, chip, bca_gpio);
    if (ret < 0) {
        dev_err(dev, "Couldn't add GPIO chip -- %d\n", ret);
        goto error;
    }

    for (i = 0; i < bca_gpio->num_intr; i++) {
        irq_map = &bca_gpio->irq_map[i];
        irq_set_chained_handler_and_data(irq_map->parent_virq,
              bcm_bca_gpio_irq_handler, irq_map);
    }

#if defined(PERIPH_INTR)
    bcm_bca_periph_intr_init(pdev);
#endif

    spin_lock_init(&bca_gpio->lock);
    bcm_bca_gpio = bca_gpio;

    return 0;

error:
    if (bca_gpio)
    {
        if (bca_gpio->irq_domain)
            irq_domain_remove(bca_gpio->irq_domain);

        if (!IS_ERR(bca_gpio->gpio_data_base))
            devm_iounmap(dev, bca_gpio->gpio_data_base);

        if (!IS_ERR(bca_gpio->gpio_dir_base))
            devm_iounmap(dev, bca_gpio->gpio_dir_base);

        if (!IS_ERR(bca_gpio->gpio_intr_base))
            devm_iounmap(dev, bca_gpio->gpio_intr_base);

        platform_set_drvdata(pdev, NULL);
        if (bca_gpio->irq_map)
            devm_kfree(dev, bca_gpio->irq_map);
        devm_kfree(dev, bca_gpio);

    }
    return ret;
}

static struct platform_driver bcm_bca_gpio_driver = {
    .driver = {
        .name = "bcm-bca-gpio",
        .of_match_table = bcm_bca_gpio_of_match,
    },
    .probe = bcm_bca_gpio_probe,
};

static int __init bcmbca_gpio_drv_reg(void)
{
    return platform_driver_register(&bcm_bca_gpio_driver);
}
postcore_initcall(bcmbca_gpio_drv_reg);

#if defined(PERIPH_INTR)
/* 
 * return the raw GIC SPI IRQ number. This is 32 smaller than linux
 * hwirq number.  First 32 hwirqs are for PPI and SGI in linux.
 */
int bcm_bca_extintr_get_hwirq(int virq)
{
    int i = 0;
    while (bcm_bca_gpio->irq_map[i].virq != virq && i <bcm_bca_gpio->num_intr)
        i++;
    return i < bcm_bca_gpio->num_intr ? 
        (int)(bcm_bca_gpio->irq_map[i].parent_hwirq - 32) : -1;
}
EXPORT_SYMBOL(bcm_bca_extintr_get_hwirq);

int bcm_bca_extintr_get_virq_by_gpio(int gpio)
{
    int i;

    for (i = 0; i < bcm_bca_gpio->num_intr; i++) {
        if (bcm_bca_gpio->irq_map[i].gpio == gpio)
            return bcm_bca_gpio->irq_map[i].virq;
    }

    return -1;
}
EXPORT_SYMBOL(bcm_bca_extintr_get_virq_by_gpio);
#endif

MODULE_AUTHOR("Samyon Furman (samyon.furman@broadcom.com)");
MODULE_DESCRIPTION("Broadcom BCA GPIO Driver");
MODULE_LICENSE("GPL v2");

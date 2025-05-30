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
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/cpu.h>
#include <linux/gpio.h>
#include <dt-bindings/interrupt-controller/arm-gic.h>
#include <dt-bindings/gpio/gpio.h>
#include <bcm_bca_extintr.h>

#define INTR_NAME_MAX_LENGTH 16
#define EXT_INT_NOT_MAPPED   0xFFFF
#define EXT_INT_RESERVED     0xFEFE
#define MAX_INT_SETS         5

struct map_t {
    unsigned int gpio;
    unsigned int irq;
    unsigned int hw_irq;
    char *intr_name;
};

struct bcm_bca_extintr {
	void __iomem *reg_base;
	void __iomem *intset_base[MAX_INT_SETS];
	spinlock_t lock;
	struct platform_device *pdev;
    unsigned int num_ext_intr;
    bool clear_workarround;
    struct map_t *map;
};

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

#define INT_SET_IRQ_SENSE_OFFSET       0x0
#define INT_SET_IRQ_MASK0_OFFSET       0x10
#define INT_SET_IRQ_STATUS0_OFFSET     0x50
#define INT_SET_IRQ_MASKX_OFFSET(x)    (INT_SET_IRQ_MASK0_OFFSET + (4*(sizeof(uint32_t))*(x)))
#define INT_SET_IRQ_STATUSX_OFFSET(x)  (INT_SET_IRQ_STATUS0_OFFSET + (4*(sizeof(uint32_t))*(x)))
#define INT_SET_IRQ_INTERNAL_OFFSET(x) (x * sizeof(uint32_t))

static  struct bcm_bca_extintr *bca_ext = NULL;

static inline unsigned int extirq_to_irq(unsigned int extirq)
{
    return bca_ext->map[extirq].irq;
}

static inline int irq_to_extirq(unsigned int irq)
{
    int i = 0;
    while (bca_ext->map[i].irq != irq && i <bca_ext->num_ext_intr)
        i++;
    return i;
}

static inline int extirq_to_gpio(unsigned int irq)
{
    int i = 0;
    while (bca_ext->map[i].irq != irq && i <bca_ext->num_ext_intr)
        i++;
    if(i == bca_ext->num_ext_intr)
        return EXT_INT_NOT_MAPPED;
    return bca_ext->map[i].gpio;
}

static int find_and_bind_gpio_to_extintr(unsigned int gpio, unsigned int interrupt_type, unsigned int reserved_ext_id)
{
    int ext = 0;
    void __iomem *reg =  bca_ext->reg_base;
    int sel0, sel1;
    u32 mask, value, data;
    unsigned long flags;
    int levelOrEdge = 0, detectSense = 0, bothEdge = 0;

    spin_lock_irqsave(&bca_ext->lock, flags);
    if (reserved_ext_id != EXT_INT_NOT_MAPPED)
    {
        if (reserved_ext_id >= bca_ext->num_ext_intr)
        {
            spin_unlock_irqrestore(&bca_ext->lock, flags);
            dev_err(&bca_ext->pdev->dev, "Provided reserved External Interrupt id %d is out of range\n", reserved_ext_id);
            return -EINVAL;
        }

        if ((bca_ext->map[reserved_ext_id].gpio != EXT_INT_RESERVED) &&
            (bca_ext->map[reserved_ext_id].gpio != EXT_INT_NOT_MAPPED))
        {
            spin_unlock_irqrestore(&bca_ext->lock, flags);
            dev_err(&bca_ext->pdev->dev, "Duplicate use of reserved External Interrupt %d\n", reserved_ext_id);
            return -EINVAL;
        }

        ext = reserved_ext_id;
    }
    else
    {
        while ((ext < bca_ext->num_ext_intr) && (bca_ext->map[ext].gpio != EXT_INT_NOT_MAPPED))
            ext++;
    }

    if (ext == bca_ext->num_ext_intr)
    {
        spin_unlock_irqrestore(&bca_ext->lock, flags);
        dev_err(&bca_ext->pdev->dev, "No free External Interrupt found\n");
        return -EINVAL;
    }

    bca_ext->map[ext].gpio = gpio;

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

    if (IsExtIntrActHigh(interrupt_type))
        detectSense = 1;

    if (IsExtIntrSenseLevel(interrupt_type))
        levelOrEdge = 1;

    if(IsExtIntrBothEdge(interrupt_type))
        bothEdge = 1;

    value = (levelOrEdge << (EI_LEVEL_SHFT + ext)) |
            (detectSense << (EI_SENSE_SHFT + ext)) |
            (bothEdge << (EI_INSENS_SHFT + ext)) ;

    if (bca_ext->clear_workarround)
        value |= (1 << (EI_CLEAR_SHFT + ext));

    data = readl(reg + EXT_IRQ_CTRL_REG);
    writel(data | value, reg + EXT_IRQ_CTRL_REG);

    writel(1 << ext, reg + EXT_IRQ_MASK_SET_REG);

    spin_unlock_irqrestore(&bca_ext->lock, flags);
    dev_info(&bca_ext->pdev->dev, "GPIO %d is mapped to ExtIntr_%d [virq %d]\n",
        gpio, ext, extirq_to_irq(ext));

    return ext;
}

static struct of_device_id const bcm_bca_extintr_of_match[] = {
	{ .compatible = "brcm,bca-extintr" },
	{}
};

MODULE_DEVICE_TABLE(of, bcm_bca_extintr_of_match);

static int bcm_bca_extintr_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	const struct of_device_id *match;
	struct resource res;
	int ret;
    unsigned int num_intr = 0;
    int i;
    struct of_phandle_args oirq;
    uint32_t *val_array;
    int num_reserved_ext_intr = 0;

	match = of_match_device(bcm_bca_extintr_of_match, dev);
	if (!match)
    {
		dev_err(dev, "Failed to find external interrupt controller\n");
		return -ENODEV;
	}

	bca_ext = devm_kzalloc(dev, sizeof(*bca_ext), GFP_KERNEL);
	if (!bca_ext)
    {
        ret = -ENOMEM;
        goto error;
    }

    if (of_property_read_u32(pdev->dev.of_node, "num_ext_intr", &num_intr))
    {
        dev_err(&pdev->dev, "Missing num_ext_intr OF property\n");
        ret = -EINVAL;
        goto error;
    }

    bca_ext->num_ext_intr = num_intr;

    bca_ext->clear_workarround = of_property_read_bool(pdev->dev.of_node, "clear_workarround");

    bca_ext->map = devm_kmalloc(dev, (sizeof(struct map_t) * num_intr), GFP_KERNEL);

    if (!bca_ext->map)
    {
        ret = -ENOMEM;
        goto error;
    }

    val_array = devm_kmalloc(dev, (sizeof(uint32_t) * num_intr), GFP_KERNEL);
    if (!val_array)
    {
        ret = -ENOMEM;
        goto error;
    }

    ret = of_property_read_u32_array(pdev->dev.of_node, "external_interrupts", val_array, num_intr);
    if (ret)
    {
        dev_err(&pdev->dev, "external_interrupts property not found %d\n", ret);
        ret = -EINVAL;
        goto error;
    }

    oirq.np = of_irq_find_parent(pdev->dev.of_node);
    oirq.args_count = 3;
    oirq.args[0] = GIC_SPI;
    oirq.args[2] = IRQ_TYPE_LEVEL_HIGH;
    for (i = 0; i < num_intr; i++)
    {
        oirq.args[1] = val_array[i];
        bca_ext->map[i].hw_irq = val_array[i];
        bca_ext->map[i].irq = irq_create_of_mapping(&oirq);
        bca_ext->map[i].gpio = EXT_INT_NOT_MAPPED;
        bca_ext->map[i].intr_name = NULL;
        dev_info(&pdev->dev, "Ext_Int_%d HWIrq %d virq %d\n", i, val_array[i]+32, bca_ext->map[i].irq);
    }

    /* Try to reserve some ids for hardcodded functionality */

    memset(val_array, 0, (sizeof(uint32_t) * num_intr));
    num_reserved_ext_intr = of_property_read_variable_u32_array(pdev->dev.of_node, "reserved_ext_ids",
        val_array, 0, num_intr);

    if(num_reserved_ext_intr > 0)
    {
        for (i = 0; i<num_reserved_ext_intr; i++)
        {
            if (val_array[i] > num_intr)
            {
                dev_err(&pdev->dev, "The provided reserved EXT_INT id %d is out of range \n", val_array[i]);
                BUG();
            }

            bca_ext->map[val_array[i]].gpio = EXT_INT_RESERVED;
        }
    }

    devm_kfree(dev, val_array);

	bca_ext->pdev = pdev;
	platform_set_drvdata(pdev, bca_ext);

    ret = of_address_to_resource(pdev->dev.of_node, 0, &res);
    if (ret)
    {
        goto error;
    }

#if defined CONFIG_PHYS_ADDR_T_64BIT
    dev_info(&pdev->dev, "Readed resouce %s start 0x%llx end 0x%llx\n", res.name, res.start, res.end);
#else
    dev_info(&pdev->dev, "Readed resouce %s start 0x%x end 0x%x\n", res.name, res.start, res.end);
#endif

	bca_ext->reg_base = devm_ioremap_resource(dev, &res);
	if (IS_ERR(bca_ext->reg_base)) {
		ret = -ENXIO;
		goto error;
	}

	spin_lock_init(&bca_ext->lock);

	/* set the mux register to invalid selection to avoid default of gpio 0 */
	writel(0xffffffff, bca_ext->reg_base + EXT_IRQ_MUX_SEL0_REG);
	writel(0xffffffff, bca_ext->reg_base + EXT_IRQ_MUX_SEL1_REG);

    /* Map optional IntSetX registers */

    for (i = 0; i < MAX_INT_SETS; i++)
    {
        ret = of_address_to_resource(pdev->dev.of_node, (i+1), &res);
        if (ret)
            break;

        bca_ext->intset_base[i] = devm_ioremap_resource(dev, &res);
    	if (IS_ERR(bca_ext->intset_base[i]))
        {
            ret = -ENXIO;
            goto error;
        }
    }

    return 0;
error:

    if (bca_ext)
    {
        if (bca_ext->map)
            devm_kfree(dev, bca_ext->map);

        devm_kfree(dev, bca_ext);
        bca_ext = NULL;
    }
    return ret;
}

static struct platform_driver bcm_bca_extintr_driver = {
	.driver = {
			.name = "bca_extintr",
			.of_match_table = bcm_bca_extintr_of_match,
	},
	.probe = bcm_bca_extintr_probe,
};

int bcm_bca_extintr_free(void *_dev, int irq, void *param)
{
    int extirq;
    unsigned long flags;
    struct device *dev = (struct device *)_dev;

    if (!bca_ext)
        return -EPROBE_DEFER;

    extirq = irq_to_extirq(irq);

    free_irq(irq, param);
    gpio_free(bca_ext->map[extirq].gpio);

    spin_lock_irqsave(&bca_ext->lock, flags);
    bca_ext->map[extirq].gpio = EXT_INT_NOT_MAPPED;
    spin_unlock_irqrestore(&bca_ext->lock, flags);
    devm_kfree(dev, bca_ext->map[extirq].intr_name);
    return 0;
}

int bcm_bca_extintr_request(void *_dev, struct device_node *np, const char *consumer_name, irq_handler_t pfunc, void *param,
    const char *interrupt_name, irq_handler_t thread_fn)
{
    return bcm_bca_extintr_request_ex(_dev, np, consumer_name, pfunc, param, interrupt_name, thread_fn,
        EXT_INT_NOT_MAPPED);
}

int bcm_bca_extintr_request_ex(void *_dev, struct device_node *np, const char *consumer_name, irq_handler_t pfunc, void *param,
    const char *interrupt_name, irq_handler_t thread_fn, unsigned int reserved_ext_id)
{
    struct device *dev = (struct device *)_dev;
    int ret;
    struct of_phandle_args params;
    uint32_t gpio;
    uint32_t interrupt_type;
    int irq;
    int extirq;
    char *intr_name;
    unsigned long irqflags = 0x00;
    unsigned long gflags = GPIOF_DIR_IN;
    struct device_node *node = np;

    if (!bca_ext)
        return -EPROBE_DEFER;

    if (!node)
        node = dev->of_node;

    ret = of_parse_phandle_with_fixed_args(node, consumer_name, 3, 0, &params);
    if (ret)
    {
        dev_err(dev, "%s property not found\n", consumer_name);
        return ret;
    }
    gpio = params.args[0];
    interrupt_type = params.args[2];

    /* Check if the GPIO is already registered */
        irq = bcm_bca_extintr_get_virq_by_gpio(gpio);
    if ((irq > 0) && (interrupt_type & BCA_EXTINTR_SHARE_GPIO_MASK))
    {
        goto exit;
    }

    if (params.args[1] & GPIO_ACTIVE_LOW)
        gflags |= GPIOF_ACTIVE_LOW;

    ret = gpio_request_one(gpio, gflags, consumer_name);
    if (ret)
    {
        dev_err(dev, "Failed to request GPIO %d(ret = %d)\n", gpio, ret);
        goto exit;
    }

        extirq = find_and_bind_gpio_to_extintr(gpio, interrupt_type, reserved_ext_id);
    if (extirq < 0)
    {
        dev_err(dev, "No free external interrupts left\n");
        ret = extirq;
        goto exit;
    }

    irq = extirq_to_irq(extirq);
    bcm_bca_extintr_clear(irq);

    intr_name = devm_kzalloc(dev, INTR_NAME_MAX_LENGTH, GFP_KERNEL);
    if (!intr_name)
    {
        dev_err(dev, "kzalloc(%d, GFP_KERNEL) failed for intr name\n", INTR_NAME_MAX_LENGTH);
        ret = -ENOMEM;
        goto exit;
    }
    snprintf(intr_name, INTR_NAME_MAX_LENGTH, "%s", interrupt_name);

    if (interrupt_type & BCA_EXTINTR_SHARED)
        irqflags |= IRQF_SHARED;

    ret = request_threaded_irq(irq, pfunc, thread_fn, irqflags, intr_name, param);
    if (ret)
    {
        dev_err(dev, "request IRQ falied for irq=%d ret=%d\n", irq, ret);
        devm_kfree(dev, intr_name);
        goto exit;
    }

    bca_ext->map[extirq].intr_name = intr_name;

exit:
    of_node_put(params.np);

    return ret ? ret : irq;
}

void bcm_bca_extintr_clear(unsigned int irq)
{
    unsigned long flags;
    void __iomem *reg;
    int ext;
    u32 data;

    if (!bca_ext)
        return;
    reg = bca_ext->reg_base;

    ext = irq_to_extirq(irq);

    spin_lock_irqsave(&bca_ext->lock, flags);

    if(bca_ext->clear_workarround)
    {
        do {
            data = readl(reg + EXT_IRQ_CTRL_REG);
            writel(data | (0xff << EI_CLEAR_SHFT), reg + EXT_IRQ_CTRL_REG);
            writel(1 << ext, reg + EXT_IRQ_CLEAR_REG);
        } while ((readl(reg + EXT_IRQ_STATUS_REG) & (1 << ext)) && EI_STATUS_MASK);
    }
    else
    {
        writel(1 << ext, reg + EXT_IRQ_CLEAR_REG);
        writel(~(1 << ext), reg + EXT_IRQ_CLEAR_REG);
    }

    spin_unlock_irqrestore(&bca_ext->lock, flags);
}

void bcm_bca_extintr_mask(unsigned int irq)
{
    unsigned long flags;
    void __iomem *reg;
    int ext;

    if (!bca_ext)
        return;
    reg =  bca_ext->reg_base;

    ext = irq_to_extirq(irq);

    spin_lock_irqsave(&bca_ext->lock, flags);
    writel(1 << ext, reg + EXT_IRQ_MASK_CLEAR_REG);
    spin_unlock_irqrestore(&bca_ext->lock, flags);
}

void bcm_bca_extintr_unmask(unsigned int irq)
{
    unsigned long flags;
    void __iomem *reg;
    int ext;

    if (!bca_ext)
        return;
    reg = bca_ext->reg_base;

    ext = irq_to_extirq(irq);

    spin_lock_irqsave(&bca_ext->lock, flags);
    writel(1 << ext, reg + EXT_IRQ_MASK_SET_REG);
    spin_unlock_irqrestore(&bca_ext->lock, flags);
}

void* bcm_bca_extintr_get_gpiod(unsigned int irq)
{
    int gpio;

    if (!bca_ext)
        return ERR_PTR(-EPROBE_DEFER);

    gpio = extirq_to_gpio(irq);

    if(gpio == EXT_INT_NOT_MAPPED)
        return NULL;
    return gpio_to_desc(gpio);
}

int bcm_bca_extintr_get_hwirq(int virq)
{
    int i = 0;
    while (bca_ext->map[i].irq != virq && i <bca_ext->num_ext_intr)
        i++;
    return i < bca_ext->num_ext_intr ? bca_ext->map[i].hw_irq : -1;
}

int bcm_bca_extintr_get_virq_by_gpio(int gpio)
{
    int i;

    for (i = 0; i < bca_ext->num_ext_intr; i++)
    {
        if (bca_ext->map[i].gpio == gpio) // Found searched GPIO
            return bca_ext->map[i].irq;
    }

    return -1;
}

int periph_intr_sense_read(unsigned int hw_irq)
{
    int set_num = hw_irq / 128;
    int int_offset = hw_irq % 128;
    int int_set = int_offset / 32;
    int int_shift = int_offset % 32;
    void __iomem *reg;
    uint32_t data;

    if (!bca_ext)
        return -ENODEV;
    if (set_num >= MAX_INT_SETS)
        return -EINVAL;

    reg = bca_ext->intset_base[set_num];
    if (!reg)
        return -EINVAL;

    data = readl(reg + INT_SET_IRQ_SENSE_OFFSET + INT_SET_IRQ_INTERNAL_OFFSET(int_set));

    if (data & (1 << int_shift))
        return 1;
    else
        return 0;
}

int periph_intr_sense_set(unsigned int hw_irq, bool is_active_high)
{
    int set_num = hw_irq / 128;
    int int_offset = hw_irq % 128;
    int int_set = int_offset / 32;
    int int_shift = int_offset % 32;
    void __iomem *reg;
    uint32_t data;

    if (!bca_ext)
        return -ENODEV;
    if (set_num >= MAX_INT_SETS)
        return -EINVAL;

    reg = bca_ext->intset_base[set_num];
    if (!reg)
        return -EINVAL;

    data = readl(reg + INT_SET_IRQ_SENSE_OFFSET + INT_SET_IRQ_INTERNAL_OFFSET(int_set));

    data |= (1 << int_shift);
    if (is_active_high)
        data &= ~(1 << int_shift);

    writel(data, reg + INT_SET_IRQ_SENSE_OFFSET + INT_SET_IRQ_INTERNAL_OFFSET(int_set));
    return 0;
}

int periph_intr_mask_read(unsigned int mask_num, unsigned int hw_irq)
{
    int set_num = hw_irq / 128;
    int int_offset = hw_irq % 128;
    int int_set = int_offset / 32;
    int int_shift = int_offset % 32;
    void __iomem *reg;
    uint32_t data;

    if (!bca_ext)
        return -ENODEV;
    if (set_num >= MAX_INT_SETS)
        return -EINVAL;

    reg = bca_ext->intset_base[set_num];
    if (!reg)
        return -EINVAL;

    data = readl(reg + INT_SET_IRQ_MASKX_OFFSET(mask_num) + INT_SET_IRQ_INTERNAL_OFFSET(int_set));
    if (data & (1 << int_shift))
        return 1;
    else
        return 0;
}

int periph_intr_mask_set(unsigned int mask_num, unsigned int hw_irq, bool do_set)
{
    int set_num = hw_irq / 128;
    int int_offset = hw_irq % 128;
    int int_set = int_offset / 32;
    int int_shift = int_offset % 32;
    void __iomem *reg;
    uint32_t data;

    if (!bca_ext)
        return -ENODEV;
    if (set_num >= MAX_INT_SETS)
        return -EINVAL;

    reg = bca_ext->intset_base[set_num];
    if (!reg)
        return -EINVAL;

    data = readl(reg + INT_SET_IRQ_MASKX_OFFSET(mask_num) + INT_SET_IRQ_INTERNAL_OFFSET(int_set));

    data &= ~(1 << int_shift);
    if (do_set)
        data |= (1 << int_shift);

    writel(data, reg + INT_SET_IRQ_MASKX_OFFSET(mask_num) + INT_SET_IRQ_INTERNAL_OFFSET(int_set));
    return 0;
}

int periph_intr_status_read(unsigned int mask_num, unsigned int hw_irq)
{
    int set_num = hw_irq / 128;
    int int_offset = hw_irq % 128;
    int int_set = int_offset / 32;
    int int_shift = int_offset % 32;
    void __iomem *reg;
    uint32_t data;

    if (!bca_ext)
        return -ENODEV;
    if (set_num >= MAX_INT_SETS)
        return -EINVAL;

    reg = bca_ext->intset_base[set_num];
    if (!reg)
        return -EINVAL;

    data = readl(reg + INT_SET_IRQ_STATUSX_OFFSET(mask_num) + INT_SET_IRQ_INTERNAL_OFFSET(int_set));

    if (data & (1 << int_shift))
        return 1;
    else
        return 0;
}

int periph_intr_status_set(unsigned int mask_num, unsigned int hw_irq, bool do_set)
{
    int set_num = hw_irq / 128;
    int int_offset = hw_irq % 128;
    int int_set = int_offset / 32;
    int int_shift = int_offset % 32;
    void __iomem *reg;
    uint32_t data;

    if (!bca_ext)
        return -ENODEV;
    if (set_num >= MAX_INT_SETS)
        return -EINVAL;

    reg = bca_ext->intset_base[set_num];
    if (!reg)
        return -EINVAL;

    data = readl(reg + INT_SET_IRQ_STATUSX_OFFSET(mask_num) + INT_SET_IRQ_INTERNAL_OFFSET(int_set));

    data &= ~(1 << int_shift);
    if (do_set)
        data |= (1 << int_shift);

    writel(data, reg + INT_SET_IRQ_STATUSX_OFFSET(mask_num) + INT_SET_IRQ_INTERNAL_OFFSET(int_set));
    return 0;
}

EXPORT_SYMBOL(bcm_bca_extintr_request);
EXPORT_SYMBOL(bcm_bca_extintr_request_ex);
EXPORT_SYMBOL(bcm_bca_extintr_free);
EXPORT_SYMBOL(bcm_bca_extintr_clear);
EXPORT_SYMBOL(bcm_bca_extintr_mask);
EXPORT_SYMBOL(bcm_bca_extintr_unmask);
EXPORT_SYMBOL(bcm_bca_extintr_get_gpiod);
EXPORT_SYMBOL(bcm_bca_extintr_get_hwirq);
EXPORT_SYMBOL(bcm_bca_extintr_get_virq_by_gpio);
EXPORT_SYMBOL(periph_intr_sense_read);
EXPORT_SYMBOL(periph_intr_sense_set);
EXPORT_SYMBOL(periph_intr_mask_read);
EXPORT_SYMBOL(periph_intr_mask_set);
EXPORT_SYMBOL(periph_intr_status_read);
EXPORT_SYMBOL(periph_intr_status_set);

static int __init bcmbca_extintr_drv_reg(void)
{
	return platform_driver_register(&bcm_bca_extintr_driver);
}

arch_initcall(bcmbca_extintr_drv_reg);

MODULE_AUTHOR("Samyon Furman (samyon.furman@broadcom.com)");
MODULE_DESCRIPTION("Broadcom BCA External Interrupt Driver");
MODULE_LICENSE("GPL v2");

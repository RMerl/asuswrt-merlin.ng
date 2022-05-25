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

#define GPIO_TO_IDX(gpio)   ((gpio) >> 5)
#define GPIO_TO_MASK(gpio)  (1<<((gpio) & 0x1f))
#define GPIO_TO_SHIFT(gpio) ((gpio) & 0x1f)

struct bcm_bca_gpio {
    void __iomem *gpio_dir_base;
    void __iomem *gpio_data_base;
    spinlock_t lock;
    struct gpio_chip gpio_chip;
    struct platform_device *pdev;
};

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
    struct resource *res_dir;
    struct resource *res_data;
    struct gpio_chip *chip;
    struct bcm_bca_gpio *bca_gpio = NULL;
    int ret;
    unsigned int ngpio = 0;

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

    dev_info(&pdev->dev, "Setting up BCA GPIO\n");

    ret = gpiochip_add(chip);
    if (ret < 0) 
    {
        dev_err(dev, "Couldn't add GPIO chip -- %d\n", ret);
        goto error;
    }

    spin_lock_init(&bca_gpio->lock);

    return 0;

error:
    if (bca_gpio)
    {
        if (!IS_ERR(bca_gpio->gpio_data_base))
            devm_iounmap(dev, bca_gpio->gpio_data_base);

        if (!IS_ERR(bca_gpio->gpio_dir_base))
            devm_iounmap(dev, bca_gpio->gpio_dir_base);

        platform_set_drvdata(pdev, NULL);
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

MODULE_AUTHOR("Samyon Furman (samyon.furman@broadcom.com)");
MODULE_DESCRIPTION("Broadcom BCA GPIO Driver");
MODULE_LICENSE("GPL v2");

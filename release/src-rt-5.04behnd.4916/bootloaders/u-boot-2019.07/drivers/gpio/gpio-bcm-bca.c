// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 Broadcom
 */
/*
 *  
 */
#include <common.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/io.h>

#define GPIO_TO_IDX(gpio)   ((gpio) >> 5)
#define GPIO_TO_MASK(gpio)  (1<<((gpio) & 0x1f))
#define GPIO_TO_SHIFT(gpio) ((gpio) & 0x1f)

struct bcmbca_gpio {
    void __iomem *gpio_dir_base;
    void __iomem *gpio_data_base;
};

static inline struct bcmbca_gpio *to_bca_gpio(struct udevice *dev)
{
    return dev_get_priv(dev);
}

static int bcmbca_gpio_direction_input(struct udevice *dev, unsigned gpio)
{
    struct bcmbca_gpio *bca_gpio;
    volatile uint32_t *gpio_dir;

    bca_gpio = to_bca_gpio(dev);
    gpio_dir = (volatile uint32_t *)bca_gpio->gpio_dir_base;

    gpio_dir[GPIO_TO_IDX(gpio)] &= ~GPIO_TO_MASK(gpio);


    return 0;
}

static int bcmbca_gpio_direction_output(struct udevice *dev, unsigned gpio, int value)
{
    struct bcmbca_gpio *bca_gpio;
    volatile uint32_t *gpio_dir;
    volatile uint32_t *gpio_data;

    bca_gpio = to_bca_gpio(dev);
    gpio_dir = (volatile uint32_t *)bca_gpio->gpio_dir_base;
    gpio_data = (volatile uint32_t *)bca_gpio->gpio_data_base;

    gpio_dir[GPIO_TO_IDX(gpio)] |= GPIO_TO_MASK(gpio);

    if (value)
        gpio_data[GPIO_TO_IDX(gpio)] |= GPIO_TO_MASK(gpio);
    else
        gpio_data[GPIO_TO_IDX(gpio)] &= ~GPIO_TO_MASK(gpio);


    return 0;
}

static int bcmbca_gpio_get_direction(struct udevice *dev, unsigned gpio)
{
    struct bcmbca_gpio *bca_gpio;
    volatile uint32_t *gpio_dir;
    u32 val;

    bca_gpio = to_bca_gpio(dev);
    gpio_dir = (volatile uint32_t *)bca_gpio->gpio_dir_base;

    val = (gpio_dir[GPIO_TO_IDX(gpio)] & GPIO_TO_MASK(gpio)) >> GPIO_TO_SHIFT(gpio);

    return val ? GPIOF_OUTPUT : GPIOF_INPUT;
}

static int bcmbca_gpio_get(struct udevice *dev, unsigned gpio)
{
    struct bcmbca_gpio *bca_gpio;
    volatile uint32_t *gpio_data;
    u32 val;

    bca_gpio = to_bca_gpio(dev);
    gpio_data = (volatile uint32_t *)bca_gpio->gpio_data_base;

    val = (gpio_data[GPIO_TO_IDX(gpio)] & GPIO_TO_MASK(gpio)) >> GPIO_TO_SHIFT(gpio);

    return (int)val;
}

static int bcmbca_gpio_set(struct udevice *dev, unsigned gpio, int value)
{
    struct bcmbca_gpio *bca_gpio;
    volatile uint32_t *gpio_data;

    bca_gpio = to_bca_gpio(dev);
    gpio_data = (volatile uint32_t *)bca_gpio->gpio_data_base;

    if (value)
        gpio_data[GPIO_TO_IDX(gpio)] |= GPIO_TO_MASK(gpio);
    else
        gpio_data[GPIO_TO_IDX(gpio)] &= ~GPIO_TO_MASK(gpio);

    return 0;
}

static int bcmbca_gpio_request(struct udevice *dev, unsigned offset, const char *label)
{
    return pinctrl_gpio_request(dev, offset);
}

static const struct dm_gpio_ops bcmbca_gpio_ops = {
    .request = bcmbca_gpio_request,
    .get_function = bcmbca_gpio_get_direction,
    .direction_input = bcmbca_gpio_direction_input,
    .direction_output = bcmbca_gpio_direction_output,
    .get_value = bcmbca_gpio_get,
    .set_value = bcmbca_gpio_set,
};

static struct udevice_id const bcmbca_gpio_of_match[] = {
    { .compatible = "brcm,bca-gpio" },
    {}
};

static int bcmbca_gpio_probe(struct udevice *dev)
{
    struct gpio_dev_priv *chip = dev_get_uclass_priv(dev);
    struct bcmbca_gpio *bca_gpio = dev_get_priv(dev); 
    int ret;
    unsigned int ngpio = 0;

    if (dev_read_u32u(dev, "ngpios", &ngpio)) 
    {
        printf("Missing ngpios OF property\n");
        ret = -EINVAL;
        goto error;
    }

    bca_gpio->gpio_dir_base = dev_remap_addr_name(dev, "gpio-dir");
    if (!bca_gpio->gpio_dir_base) 
    {
        printf("Failed to find/map the gpio-dir resource\n");
        ret = -ENXIO;
        goto error;
    }

    bca_gpio->gpio_data_base = dev_remap_addr_name(dev, "gpio-data");
    if (!bca_gpio->gpio_data_base) 
    {
        printf("Failed to find/map the gpio-data resource\n");
        ret = -ENXIO;
        goto error;
    }

    chip->gpio_count = ngpio;
    chip->bank_name = dev->name;
    chip->gpio_base = 0;

    return 0;

error:
    bca_gpio->gpio_data_base = NULL;
    bca_gpio->gpio_dir_base = NULL;

    return ret;
}

U_BOOT_DRIVER(bcmbca_gpio) = {
    .name = "bcm-bca-gpio",
    .id = UCLASS_GPIO,
    .of_match = bcmbca_gpio_of_match,
    .ops = &bcmbca_gpio_ops,
    .priv_auto_alloc_size = sizeof(struct bcmbca_gpio),
    .probe = bcmbca_gpio_probe,
};


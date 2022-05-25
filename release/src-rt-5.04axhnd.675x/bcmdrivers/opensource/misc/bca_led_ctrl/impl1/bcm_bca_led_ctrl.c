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
#include <linux/leds.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/module.h>
#include <linux/version.h>

enum LP_LED_REGS {
    LED_CTRL = 0,
    LED_HW_EN,
    LED_SERIAL_SHIFT_SEL,
    LED_FLASH_RATE,
    LED_BRIGHTNESS,
    LED_POWER_LED_CFG,
    LED_POWER_LUT,
    LED_HW_POLARITY,
    LED_SW_DATA,
    LED_SW_POLARITY,
    LED_PARALLEL_POLARITY,
    LED_SERIAL_POLARITY,
    LED_LAST_MANDATORY,
    LED_MASK = LED_LAST_MANDATORY,
    LED_MAX_REG,
};

struct bcm_bca_led_ctrl {
    void __iomem *led_regs[LED_MAX_REG];
    spinlock_t lock;
    uint8_t max_supported_leds;
    struct platform_device *pdev;
    uint8_t serial_shifters_num;
    uint32_t serial_led_map;
    uint8_t active_serial_led_count;
};

static struct bcm_bca_led_ctrl *bca_led = NULL;

static const struct of_device_id bca_led_ctrl_of_match[] = {
	{ .compatible = "brcm,bca-leds-ctrl", },
	{ },
};

MODULE_DEVICE_TABLE(of, bca_led_ctrl_of_match);

static int bca_led_ctrl_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    const struct of_device_id *match;
	struct resource *res;
    unsigned int val = 0;
	int ret;
    int i;
    bool serial_msb_first = 0;
    bool serial_data_polarity_low = 0;
    uint32_t reg_val;
    volatile uint32_t *led_bright;
    struct device_node *serial_pinctrl;

    char *reg_names[LED_MAX_REG] = {"glbl_ctrl", "hw_en", "ser_shift", "flash_rate", "brightness",
                                    "power_led_cfg", "power_lut", "hw_polarity", "sw_data", "sw_polarity",
                                    "par_polarity", "ser_polarity", "mask"}; 

	match = of_match_device(bca_led_ctrl_of_match, dev);
	if (!match)
    {
		dev_err(dev, "Failed to find LED controller\n");
		return -ENODEV;
	}

	bca_led = devm_kzalloc(dev, sizeof(*bca_led), GFP_KERNEL);
	if (!bca_led)
    {
		dev_err(dev, "Failed to allocate memory for LED controller\n");
        return -ENOMEM;
    }

    if(of_property_read_u32(dev->of_node, "nleds", &val))
    {
        dev_err(dev, "nleds property not present\n");
        ret = -EINVAL;
        goto error;
    }

    bca_led->max_supported_leds = (uint8_t)val;
    dev_info(dev, "max supported leds %d[%d]\n", bca_led->max_supported_leds, val);

    serial_pinctrl = of_parse_phandle(dev->of_node, "pinctrl-0", 0);

    if (serial_pinctrl)
    {

        if (of_property_read_u32(dev->of_node, "serial-shifters-installed", &val)) 
        {
            dev_err(dev, "The serial-shifters-installed property not present while Serial LED controller interface is configured\n");
            ret = -EINVAL;
            goto error;
        }

        bca_led->serial_shifters_num = (uint8_t)val;

        serial_msb_first = of_property_read_bool(dev->of_node, "serial-order-msb-first");
        serial_data_polarity_low =  of_property_read_bool(dev->of_node, "serial-data-polarity-low");
        dev_info(dev, "Serial LED interface found num shifters %d [%d] serial data polarity low %d\n",
            bca_led->serial_shifters_num, val, serial_data_polarity_low);
    }
    else
    {
        dev_info(dev, " Parallel LED interface found\n");
    }

    bca_led->pdev = pdev;
    platform_set_drvdata(pdev, bca_led);
    spin_lock_init(&bca_led->lock);

    for (i = 0; i < LED_MAX_REG; i++)
    {
        res = platform_get_resource_byname(pdev, IORESOURCE_MEM, reg_names[i]);
        if (!res)
        {
            if (i>= LED_LAST_MANDATORY)
            {
                bca_led->led_regs[i] = NULL;
                continue;
            }
            else
            {
                dev_err(dev, "Failed to find %s resource\n", reg_names[i]);
                ret = -EINVAL;
                goto error;
            }
        }

        bca_led->led_regs[i] = devm_ioremap_resource(dev, res);
        if (IS_ERR(bca_led->led_regs[i])) 
        {
            dev_err(dev, "Failed to map the %s resource\n", reg_names[i]);
            ret = -ENXIO;
            goto error;
        }
    }

    if (!of_property_read_u32(dev->of_node, "hw-polarity-quirk", &val)) 
    {
        *(volatile uint32_t *)(bca_led->led_regs[LED_HW_POLARITY]) = (uint32_t)val;
    }

    if (!of_property_read_u32(dev->of_node, "sw-polarity-quirk", &val)) 
    {
        *(volatile uint32_t *)(bca_led->led_regs[LED_SW_POLARITY]) = (uint32_t)val;
    }

    if (serial_data_polarity_low)
        reg_val = 0x8;
    else
        reg_val = 0xa;

    if (serial_msb_first)
        reg_val |= 0x10;

    *(volatile uint32_t *)(bca_led->led_regs[LED_CTRL]) = reg_val;

    led_bright = (volatile uint32_t *)(bca_led->led_regs[LED_BRIGHTNESS]);
    for (i = 0; i < bca_led->max_supported_leds/8; i++)
            led_bright[i] = 0x88888888;

    *(volatile uint32_t *)(bca_led->led_regs[LED_HW_EN]) = 0;
    *(volatile uint32_t *)(bca_led->led_regs[LED_SERIAL_POLARITY]) = 0;
    *(volatile uint32_t *)(bca_led->led_regs[LED_PARALLEL_POLARITY]) = 0;

    dev_info(dev, "BCA LED Controller initialized\n");
    
    of_platform_populate(dev->of_node, NULL, NULL, dev);

    return 0;

error:
    if (bca_led)
    {
       devm_kfree(dev, bca_led);
    }
    return ret;
}

static struct platform_driver bcm_bca_led_ctrl_driver = {
	.probe = bca_led_ctrl_probe,
	.driver = {
		.name = "bcm-bca-led-ctrl",
		.of_match_table = bca_led_ctrl_of_match,
	},
};

int bca_led_setup_serial(unsigned int led_num, unsigned int polarity, unsigned int is_hw)
{
    unsigned long flags;
    uint32_t led_mask = 1 << led_num;
    volatile uint32_t *polarity_reg;
    uint32_t led_map = 0;
    uint8_t missed_pins;
    int i;

    if(!bca_led)
        return -ENODEV;

    if (led_num > bca_led->max_supported_leds)
    {
        dev_err(&bca_led->pdev->dev,"requested LED %d is out of supported range(%d)\n", led_num,
            bca_led->max_supported_leds);
        return -EINVAL;
    }

    if (bca_led->serial_shifters_num == 0)
    {
        dev_err(&bca_led->pdev->dev,"Serial LED%d is requested, but no serial LED interface defined\n", led_num);
        return -EINVAL;
    }

    polarity_reg = (volatile uint32_t *)bca_led->led_regs[LED_SERIAL_POLARITY];

    spin_lock_irqsave(&bca_led->lock, flags);
    bca_led->active_serial_led_count++;
    if (bca_led->active_serial_led_count > (bca_led->serial_shifters_num * 8))
    {
        bca_led->active_serial_led_count--;
        spin_unlock_irqrestore(&bca_led->lock, flags);
        dev_err(&bca_led->pdev->dev,"The number of registered serial LEDs is bigger than supported by this configuration\n" );
        return -EINVAL;
    }

    if (is_hw)
        *(volatile uint32_t *)(bca_led->led_regs[LED_HW_EN]) |= led_mask;

    if (polarity)
        *polarity_reg |= led_mask;
    else
        *polarity_reg &= ~(led_mask);

    bca_led->serial_led_map |= led_mask;

    missed_pins = (bca_led->serial_shifters_num * 8) - bca_led->active_serial_led_count;

    led_map = bca_led->serial_led_map;

    for (i = 0; i < 32 && missed_pins; i++)
    {
        if (bca_led->serial_led_map & (1 << i))
            continue;
        led_map |= (1 << i);
        missed_pins--;
    }

    *(volatile uint32_t *)(bca_led->led_regs[LED_SERIAL_SHIFT_SEL]) = led_map; 

    spin_unlock_irqrestore(&bca_led->lock, flags);
    return 0;
}

int bca_led_setup_parallel(unsigned int led_num, int polarity, int is_hw)
{
    unsigned long flags;
    uint32_t led_mask = 1 << led_num;
    volatile uint32_t *polarity_reg;
   
    if(!bca_led)
        return -ENODEV;

    if (led_num > bca_led->max_supported_leds)
    {
        dev_err(&bca_led->pdev->dev,"requested LED %d is out of supported range(%d)\n", led_num,
            bca_led->max_supported_leds);
        return -EINVAL;
    }

    polarity_reg = (volatile uint32_t *)bca_led->led_regs[LED_PARALLEL_POLARITY];

    spin_lock_irqsave(&bca_led->lock, flags);

    if (is_hw)
        *(volatile uint32_t *)(bca_led->led_regs[LED_HW_EN]) |= led_mask;

    if (polarity)
        *polarity_reg |= led_mask;
    else
        *polarity_reg &= ~(led_mask);

    spin_unlock_irqrestore(&bca_led->lock, flags);
    return 0;
}

int bca_led_set_value(unsigned int led_num, unsigned int value)
{
    unsigned long flags;
    uint32_t led_mask = 1 << led_num;
    
    if(!bca_led)
        return -ENODEV;

    if (led_num > bca_led->max_supported_leds)
    {
        dev_err(&bca_led->pdev->dev,"requested LED %d is out of supported range(%d)\n", led_num,
            bca_led->max_supported_leds);
        return -EINVAL;
    }
    spin_lock_irqsave(&bca_led->lock, flags);
    if (value)
        *(volatile uint32_t *)(bca_led->led_regs[LED_SW_DATA]) |= led_mask;
    else
        *(volatile uint32_t *)(bca_led->led_regs[LED_SW_DATA]) &= ~led_mask;
    
    spin_unlock_irqrestore(&bca_led->lock, flags);
    
    return 0;
}

int bca_led_set_brightness(unsigned int led_num, unsigned int value)
{
    unsigned long flags;
    uint8_t reg_idx = (led_num >> 3);
    uint32_t led_mask = 0xf << ((led_num & 0x7) << 2);
    uint32_t mapped_val = value << ((led_num & 0x7) << 2);
    volatile uint32_t *led_bright;
    
    if(!bca_led)
        return -ENODEV;

    if (led_num > bca_led->max_supported_leds)
    {
        dev_err(&bca_led->pdev->dev,"requested LED %d is out of supported range(%d)\n", led_num,
            bca_led->max_supported_leds);
        return -EINVAL;
    }

    led_bright = (volatile uint32_t *)(bca_led->led_regs[LED_BRIGHTNESS]);

    spin_lock_irqsave(&bca_led->lock, flags);
    led_bright[reg_idx] &= ~led_mask;
    led_bright[reg_idx] |= mapped_val;
    spin_unlock_irqrestore(&bca_led->lock, flags);
    return 0;
}

int bca_led_set_flash_rate(unsigned int led_num, unsigned int value)
{
    unsigned long flags;
    uint8_t reg_idx = (led_num >> 3);
    uint32_t led_mask = 0xf << ((led_num & 0x7) << 2);
    uint32_t mapped_val = value << ((led_num & 0x7) << 2);
    volatile uint32_t *led_flash;

    if(!bca_led)
        return -ENODEV;

    if (led_num > bca_led->max_supported_leds)
    {
        dev_err(&bca_led->pdev->dev,"requested LED %d is out of supported range(%d)\n", led_num,
            bca_led->max_supported_leds);
        return -EINVAL;
    }

    led_flash = (volatile uint32_t *)(bca_led->led_regs[LED_FLASH_RATE]);

    spin_lock_irqsave(&bca_led->lock, flags);
    led_flash[reg_idx] &= ~led_mask;
    led_flash[reg_idx] |= mapped_val;
    spin_unlock_irqrestore(&bca_led->lock, flags);
    return 0;
}


EXPORT_SYMBOL(bca_led_setup_serial);
EXPORT_SYMBOL(bca_led_setup_parallel);
EXPORT_SYMBOL(bca_led_set_value);
EXPORT_SYMBOL(bca_led_set_brightness);
EXPORT_SYMBOL(bca_led_set_flash_rate);
    
static int __init bcmbca_led_ctrl_drv_reg(void)
{
	return platform_driver_register(&bcm_bca_led_ctrl_driver);
}

postcore_initcall(bcmbca_led_ctrl_drv_reg);

MODULE_AUTHOR("Samyon Furman (samyon.furman@broadcom.com)");
MODULE_DESCRIPTION("Broadcom BCA LED Controller Driver");
MODULE_LICENSE("GPL v2");

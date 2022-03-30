// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2019 Broadcom
 */
/*
 *  
 */
#include <common.h>
#include <dm.h>
#include <dm/ofnode.h>
#include <errno.h>
#include <led.h>
#include <asm/io.h>

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
    uint8_t max_supported_leds;
    uint8_t serial_shifters_num;
    uint32_t serial_led_map;
    uint8_t active_serial_led_count;
};

static struct bcm_bca_led_ctrl *bca_led = NULL;

static const struct udevice_id bca_led_ctrl_of_match[] = {
	{ .compatible = "brcm,bca-leds-ctrl", },
	{ },
};

static const ofnode of_parse_phandle(const ofnode handle, const char *phandle_name, int index)
{
    ofnode node = ofnode_null();
    struct ofnode_phandle_args out_args;

    if (ofnode_parse_phandle_with_args(handle, phandle_name, NULL, 0, index, &out_args) == 0)
    {
        node = out_args.node;
    }
    return node;
}

static int bca_led_ctrl_probe(struct udevice *dev)
{
    unsigned int val = 0;
	int ret;
    int i;
    bool serial_msb_first = 0;
    bool serial_data_polarity_low = 0;
    uint32_t reg_val;
    volatile uint32_t *led_bright;
    ofnode serial_pinctrl;

    char *reg_names[LED_MAX_REG] = {"glbl_ctrl", "hw_en", "ser_shift", "flash_rate", "brightness",
                                    "power_led_cfg", "power_lut", "hw_polarity", "sw_data", "sw_polarity",
                                    "par_polarity", "ser_polarity", "mask"}; 

	bca_led = dev_get_priv(dev);

    if(dev_read_u32u(dev, "nleds", &val))
    {
        printf("nleds property not present\n");
        ret = -EINVAL;
        goto error;
    }

    bca_led->max_supported_leds = (uint8_t)val;
    printf("max supported leds %d[%d]\n", bca_led->max_supported_leds, val);

    serial_pinctrl = of_parse_phandle(dev_ofnode(dev), "pinctrl-0", 0);

    if (serial_pinctrl.of_offset != -1)
    {

        if (dev_read_u32u(dev, "serial-shifters-installed", &val)) 
        {
            printf("The serial-shifters-installed property not present while Serial LED controller interface is configured\n");
            ret = -EINVAL;
            goto error;
        }

        bca_led->serial_shifters_num = (uint8_t)val;

        serial_msb_first = dev_read_bool(dev, "serial-order-msb-first");
        serial_data_polarity_low =  dev_read_bool(dev, "serial-data-polarity-low");
        printf("Serial LED interface found num shifters %d [%d] serial data polarity low %d\n",
            bca_led->serial_shifters_num, val, serial_data_polarity_low);
    }
    else
    {
        printf(" Parallel LED interface found\n");
    }

    for (i = 0; i < LED_MAX_REG; i++)
    {
        bca_led->led_regs[i] = dev_remap_addr_name(dev, reg_names[i]);
        if ((bca_led->led_regs[i] == NULL) && (i < LED_LAST_MANDATORY))
        {
            printf("Failed to find %s resource\n", reg_names[i]);
            ret = -EINVAL;
            goto error;
        }
    }

    if (!dev_read_u32u(dev, "hw-polarity-quirk", &val)) 
    {
        *(volatile uint32_t *)(bca_led->led_regs[LED_HW_POLARITY]) = (uint32_t)val;
    }

    if (!dev_read_u32(dev, "sw-polarity-quirk", &val)) 
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

    printf("BCA LED Controller initialized\n");

    return 0;

error:
    return ret;
}

static int bca_led_ctrl_bind(struct udevice *parent)
{
	ofnode node;

	dev_for_each_subnode(node, parent) {
		struct led_uc_plat *uc_plat;
		struct udevice *dev;
		const char *label;
		int ret;

		ret = device_bind_driver_to_node(parent, "bcm-bca-leds", ofnode_get_name(node),
            node, &dev);
		if (ret)
        {
            if (ret != -ENODEV)
                printf("failed to bind node %s ret %d\n", ofnode_get_name(node), ret);
            continue;
        }

		label = ofnode_read_string(node, "label");
        if (label)
        {
            uc_plat = dev_get_uclass_platdata(dev);
            uc_plat->label = label;
        }
	}

	return 0;
}

U_BOOT_DRIVER(bcm_bca_led_ctrl_driver) = {
    .name = "bcm-bca-led-ctrl",
	.id = UCLASS_LED,
	.probe = bca_led_ctrl_probe,
    .bind = bca_led_ctrl_bind,
    .of_match = bca_led_ctrl_of_match,
	.priv_auto_alloc_size = sizeof(struct bcm_bca_led_ctrl),
};

int bca_led_setup_serial(unsigned int led_num, unsigned int polarity, unsigned int is_hw)
{
    uint32_t led_mask = 1 << led_num;
    volatile uint32_t *polarity_reg;
    uint32_t led_map = 0;
    uint8_t missed_pins;
    int i;

    if(!bca_led)
        return -ENODEV;

    if (led_num > bca_led->max_supported_leds)
    {
        printf("requested LED %d is out of supported range(%d)\n", led_num,
            bca_led->max_supported_leds);
        return -EINVAL;
    }

    if (bca_led->serial_shifters_num == 0)
    {
        printf("Serial LED%d is requested, but no serial LED interface defined\n", led_num);
        return -EINVAL;
    }

    polarity_reg = (volatile uint32_t *)bca_led->led_regs[LED_SERIAL_POLARITY];

    bca_led->active_serial_led_count++;
    if (bca_led->active_serial_led_count > (bca_led->serial_shifters_num * 8))
    {
        bca_led->active_serial_led_count--;
        printf("The number of registered serial LEDs is bigger than supported by this configuration\n" );
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

    return 0;
}

int bca_led_setup_parallel(unsigned int led_num, int polarity, int is_hw)
{
    uint32_t led_mask = 1 << led_num;
    volatile uint32_t *polarity_reg;
   
    if(!bca_led)
        return -ENODEV;

    if (led_num > bca_led->max_supported_leds)
    {
        printf("requested LED %d is out of supported range(%d)\n", led_num, bca_led->max_supported_leds);
        return -EINVAL;
    }

    polarity_reg = (volatile uint32_t *)bca_led->led_regs[LED_PARALLEL_POLARITY];

    if (is_hw)
        *(volatile uint32_t *)(bca_led->led_regs[LED_HW_EN]) |= led_mask;

    if (polarity)
        *polarity_reg |= led_mask;
    else
        *polarity_reg &= ~(led_mask);

    return 0;
}

int bca_led_get_value(unsigned int led_num)
{
    uint32_t led_mask = 1 << led_num;
    
    if(!bca_led)
        return -ENODEV;

    if (led_num > bca_led->max_supported_leds)
    {
        printf("requested LED %d is out of supported range(%d)\n", led_num, bca_led->max_supported_leds);
        return -EINVAL;
    }

    return *(volatile uint32_t *)(bca_led->led_regs[LED_SW_DATA]) & led_mask;
}

int bca_led_set_value(unsigned int led_num, unsigned int value)
{
    uint32_t led_mask = 1 << led_num;
    
    if(!bca_led)
        return -ENODEV;

    if (led_num > bca_led->max_supported_leds)
    {
        printf("requested LED %d is out of supported range(%d)\n", led_num, bca_led->max_supported_leds);
        return -EINVAL;
    }
    if (value)
        *(volatile uint32_t *)(bca_led->led_regs[LED_SW_DATA]) |= led_mask;
    else
        *(volatile uint32_t *)(bca_led->led_regs[LED_SW_DATA]) &= ~led_mask;
    
    return 0;
}

int bca_led_set_brightness(unsigned int led_num, unsigned int value)
{
    uint8_t reg_idx = (led_num >> 3);
    uint32_t led_mask = 0xf << ((led_num & 0x7) << 2);
    uint32_t mapped_val = value << ((led_num & 0x7) << 2);
    volatile uint32_t *led_bright;
    
    if(!bca_led)
        return -ENODEV;

    if (led_num > bca_led->max_supported_leds)
    {
        printf("requested LED %d is out of supported range(%d)\n", led_num, bca_led->max_supported_leds);
        return -EINVAL;
    }

    led_bright = (volatile uint32_t *)(bca_led->led_regs[LED_BRIGHTNESS]);

    led_bright[reg_idx] &= ~led_mask;
    led_bright[reg_idx] |= mapped_val;
    return 0;
}

int bca_led_set_flash_rate(unsigned int led_num, unsigned int value)
{
    uint8_t reg_idx = (led_num >> 3);
    uint32_t led_mask = 0xf << ((led_num & 0x7) << 2);
    uint32_t mapped_val = value << ((led_num & 0x7) << 2);
    volatile uint32_t *led_flash;

    if(!bca_led)
        return -ENODEV;

    if (led_num > bca_led->max_supported_leds)
    {
        printf("requested LED %d is out of supported range(%d)\n", led_num, bca_led->max_supported_leds);
        return -EINVAL;
    }

    led_flash = (volatile uint32_t *)(bca_led->led_regs[LED_FLASH_RATE]);

    led_flash[reg_idx] &= ~led_mask;
    led_flash[reg_idx] |= mapped_val;
    return 0;
}

void bca_led_probe(void)
{
    struct udevice *dev;

    for (uclass_first_device_check(UCLASS_LED, &dev); dev; uclass_next_device_check(&dev));
}

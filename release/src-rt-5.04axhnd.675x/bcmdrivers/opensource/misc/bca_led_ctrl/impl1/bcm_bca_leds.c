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
#include <linux/gpio.h>
#include <linux/module.h>
#include <linux/version.h>
#include "phy_drv.h"
#include "phy_drv_crossbar.h"
#include "bcm_bca_leds.h"

struct gpio_led_timer{
        struct timer_list timer;
        struct bca_led *bca_led;
};

struct bca_led {
    struct led_classdev cdev;
    uint32_t pin;
    struct {
        spinlock_t lock;
        unsigned int stop;
        unsigned int cur_state;
        unsigned long interval_on;
        unsigned long interval_off;
        struct gpio_led_timer timer;
    }gpio_led;
};

extern int bca_led_set_value(unsigned int led_num, unsigned int value);
extern int bca_led_set_brightness(unsigned int led_num, unsigned int value);
extern int bca_led_set_flash_rate(unsigned int led_num, unsigned int value);
extern int bca_led_setup_serial(unsigned int led_num, unsigned int polarity, unsigned int is_hw);
extern int bca_led_setup_parallel(unsigned int led_num, unsigned int polarity, unsigned int is_hw);
/*CLED controller APIs*/
extern int bca_cled_set_value(unsigned int led_num, unsigned int value);
extern int bca_cled_set_brightness(unsigned int led_num, unsigned int value);
extern int bca_cled_set_flash_rate(unsigned int led_num, unsigned int value);
extern int bca_cled_setup_serial(unsigned int led_num, unsigned int polarity, unsigned int is_hw,
    const struct device_node *np, bool is_crossbar);
extern int bca_cled_setup_parallel(unsigned int led_num, unsigned int polarity, unsigned int is_hw,
    const struct device_node *np, bool is_crossbar);

extern struct list_head leds_list; 

static int bca_leds_init_done = 0;

static struct bca_led* find_led_device_by_pin(uint32_t pin)
{
    struct led_classdev *p;

    if (!bca_leds_init_done)
        return ERR_PTR(-EPROBE_DEFER);

    list_for_each_entry(p, &leds_list, node)
    {
        struct bca_led *led = container_of(p, struct bca_led, cdev);
        if (pin == led->pin)
            return led;
    }
    return NULL;
}

static void gpio_led_toggle(struct bca_led *led)
{
	struct gpio_desc *desc;
    unsigned long flags;

	desc = gpio_to_desc(led->pin);

    spin_lock_irqsave(&led->gpio_led.lock, flags);        // LEDs can be changed from ISR
    led->gpio_led.cur_state = (led->gpio_led.cur_state == 1) ? 0 : 1;
    spin_unlock_irqrestore(&led->gpio_led.lock, flags);
    
    gpiod_set_value(desc,led->gpio_led.cur_state);
}

static void gpio_led_timer_exp(struct timer_list *tl)
{
    struct bca_led *led = ((struct gpio_led_timer *)tl)->bca_led;
    unsigned long flags;

    spin_lock_irqsave(&led->gpio_led.lock, flags);        // LEDs can be changed from ISR
    if(led->gpio_led.stop)
    {
        spin_unlock_irqrestore(&led->gpio_led.lock, flags);
        return;
    }

    spin_unlock_irqrestore(&led->gpio_led.lock, flags);
    gpio_led_toggle(led);
    spin_lock_irqsave(&led->gpio_led.lock, flags);        // LEDs can be changed from ISR

    if (led->gpio_led.cur_state)
        led->gpio_led.timer.timer.expires = jiffies + msecs_to_jiffies(led->gpio_led.interval_on);
    else
        led->gpio_led.timer.timer.expires = jiffies + msecs_to_jiffies(led->gpio_led.interval_off);
    spin_unlock_irqrestore(&led->gpio_led.lock, flags);
    add_timer(&led->gpio_led.timer.timer);
}

static void bca_gpio_led_set(struct led_classdev *led_cdev, enum led_brightness value)
{
    struct bca_led *led = container_of(led_cdev, struct bca_led, cdev);
	struct gpio_desc *desc;
    unsigned long flags;
	desc = gpio_to_desc(led->pin);
    spin_lock_irqsave(&led->gpio_led.lock, flags);        // LEDs can be changed from ISR
    led->gpio_led.cur_state = (value == LED_OFF) ? 0 : 1;
    if (value == LED_OFF)
        led->gpio_led.stop = 1;
    spin_unlock_irqrestore(&led->gpio_led.lock, flags);
    gpiod_set_value(desc, led->gpio_led.cur_state);
}

static int bca_gpio_led_blink(struct led_classdev *led_cdev, unsigned long *delay_on, unsigned long *delay_off)
{
    struct bca_led *led = container_of(led_cdev, struct bca_led, cdev);
    unsigned long flags;
    if (*delay_on == 0)
        *delay_on = 320;
    if (*delay_off == 0)
        *delay_off = 320;

    spin_lock_irqsave(&led->gpio_led.lock, flags);        // LEDs can be changed from ISR
    led->gpio_led.stop = 0;
    led->gpio_led.interval_on = *delay_on;
    led->gpio_led.interval_off = *delay_off;
    if (led->gpio_led.cur_state)
        led->gpio_led.timer.timer.expires = jiffies + msecs_to_jiffies(*delay_on);
    else
        led->gpio_led.timer.timer.expires = jiffies + msecs_to_jiffies(*delay_off);
    spin_unlock_irqrestore(&led->gpio_led.lock, flags);
    add_timer(&led->gpio_led.timer.timer);
    return 0;
}

static int gpio_led_parse(struct device *dev, struct bca_led *led)
{
    uint32_t gpio;
    int ret;
    unsigned long gflags = GPIOF_DIR_OUT;

    if (!of_property_read_bool(dev->of_node, "software_led"))
    {
        dev_err(dev, "Hardware LEDs over GPIO inerface is not supported\n");
        return -EINVAL;
    }

    if (of_property_read_u32(dev->of_node, "pin", &gpio))
    {
        dev_err(dev, "GPIO pin is not specified\n");
        return -EINVAL;
    }

    if (of_property_read_bool(dev->of_node, "active_low"))
        gflags |= GPIOF_ACTIVE_LOW;

    if (of_property_read_bool(dev->of_node, "init_low"))
        gflags |= GPIOF_OUT_INIT_LOW;
    else
        gflags |= GPIOF_OUT_INIT_HIGH;

    led->cdev.name = of_get_property(dev->of_node, "label", NULL) ? : dev->of_node->name;

    ret = gpio_request_one(gpio, gflags, led->cdev.name); 
    if (ret)
    {
        dev_err(dev, "Failed to request GPIO %d\n", gpio);
        return ret;
    }

    spin_lock_init(&led->gpio_led.lock);
    timer_setup(&led->gpio_led.timer.timer, gpio_led_timer_exp, 0);
    led->gpio_led.timer.bca_led = led;
    led->gpio_led.stop = 1;

    led->pin = gpio;
    led->cdev.brightness_set = bca_gpio_led_set;
    led->cdev.blink_set = bca_gpio_led_blink;

    return 0;
}

static void common_bca_led_set(struct led_classdev *led_cdev, enum led_brightness value, int is_cled)
{
    int (* led_set_value)(unsigned int led_num, unsigned int value) = NULL;
    int (* led_set_brightness)(unsigned int led_num, unsigned int value) = NULL;
    int (* led_set_flash_rate)(unsigned int led_num, unsigned int value) = NULL;
    int full_bright, real_bright;

    struct bca_led *led = container_of(led_cdev, struct bca_led, cdev);
    if (is_cled)
    {
        led_set_value = bca_cled_set_value;
        led_set_brightness = bca_cled_set_brightness;
        led_set_flash_rate = bca_cled_set_flash_rate;
        full_bright = 128;
    }
    else
    {
        led_set_value = bca_led_set_value;
        led_set_brightness = bca_led_set_brightness;
        led_set_flash_rate = bca_led_set_flash_rate;
        full_bright = 8;
    }

    if (value == LED_OFF)
    {
        led_set_value(led->pin, 0);
        led_set_flash_rate(led->pin, 0);
    }
    else if (value == LED_ON)
    {
        led_set_value(led->pin, 1);
    }
    else if ((value <= LED_FULL) && (value > LED_ON))
    {
        real_bright = (full_bright * value)/LED_FULL;
        led_set_brightness(led->pin, value);
        led_set_value(led->pin, 1);
    }
    else
    {
        dev_err(led_cdev->dev,"\nUnsupported bridhtness value(%d)\n", value);
    }
}

static void bca_cled_set(struct led_classdev *led_cdev, enum led_brightness value)
{
    common_bca_led_set(led_cdev, value, 1);
}

static void bca_led_set(struct led_classdev *led_cdev, enum led_brightness value)
{
    common_bca_led_set(led_cdev, value, 0);
}

static unsigned int convert_ms_to_hz(unsigned int ms)
{
    if (ms <= 60)
        return 1;
    if (ms <= 120)
        return 2;
    if (ms <= 240)
        return 3;
    if (ms <= 480)
        return 4;
    if (ms <= 960)
        return 5;
    if (ms <= 1920)
        return 6;
    return 7;
}

static int common_bca_blink_set(struct led_classdev *led_cdev, unsigned long *delay_on, 
    unsigned long *delay_off, int is_cled)
{
    struct bca_led *led = container_of(led_cdev, struct bca_led, cdev);
    unsigned int delay;

    if (!*delay_on)
        *delay_on = 320;
    if (!*delay_off)
        *delay_off = 320;

    delay = convert_ms_to_hz(*delay_on);
    if (delay != convert_ms_to_hz(*delay_off))
    {
        dev_dbg(led_cdev->dev,
            "fallback to soft blinking (delay_on != delay_off)\n");
        return -EINVAL;
    }

    if (is_cled)
        bca_cled_set_flash_rate(led->pin, delay);
    else
        bca_led_set_flash_rate(led->pin, delay);

    return 0;
}

static int bca_blink_set(struct led_classdev *led_cdev, unsigned long *delay_on, unsigned long *delay_off)
{
    return common_bca_blink_set(led_cdev, delay_on, delay_off, 0);
}

static int bca_c_blink_set(struct led_classdev *led_cdev, unsigned long *delay_on, unsigned long *delay_off)
{
    return common_bca_blink_set(led_cdev, delay_on, delay_off, 1);
}

static int serial_led_parse(struct device *dev, struct bca_led *led)
{
    bool active_high;
    bool is_hw;
    uint32_t led_num;
    uint32_t init_brightness = 255;
    uint32_t init_flash_rate = 0;
    uint32_t default_state;
    int ret;

    if (of_property_read_u32(dev->of_node, "bit", &led_num))
    {
        dev_err(dev, "bit property is not specified\n");
        return -EINVAL;
    }

    is_hw = of_property_read_bool(dev->of_node, "hardware_led");
    active_high = of_property_read_bool(dev->of_node, "active_high");

    ret = bca_led_setup_serial(led_num, active_high, is_hw);
    if (ret)
    {
        //if (ret != -ENODEV)
            dev_err(dev, "Failed to setup serial led %d\n", led_num);
        return ret;
    }

    ret = of_property_read_u32(dev->of_node, "brightness", &init_brightness);
    ret = of_property_read_u32(dev->of_node, "flash_rate", &init_flash_rate);

    bca_led_set_brightness(led_num, init_brightness == 0 ? 0 : init_brightness < 31 ?
        1 : init_brightness/31);
    bca_led_set_flash_rate(led_num, init_flash_rate & 0x7);

    ret = of_property_read_u32(dev->of_node, "default_state", &default_state);
    if (!ret)
    {
        bca_led_set_value(led_num, default_state);
    }

    led->pin = led_num;
    if (is_hw)
    {
        led->cdev.brightness_set = NULL;
        led->cdev.blink_set = NULL;
    }
    else
    {
        led->cdev.brightness_set = bca_led_set;
        led->cdev.blink_set = bca_blink_set;
    }
    led->cdev.name = of_get_property(dev->of_node, "label", NULL) ? : dev->of_node->name;

    return 0;
}

static int serial_cled_parse(struct device *dev, struct bca_led *led)
{
    bool active_high;
    bool is_hw;
    uint32_t led_num;
    uint32_t init_brightness = 128;
    uint32_t init_flash_rate = 0;
    uint32_t default_state;
    int ret;
    bool is_crossbar;

    if (of_property_read_u32(dev->of_node, "bit", &led_num))
    {
        dev_err(dev, "bit property is not specified\n");
        return -EINVAL;
    }

    is_hw = of_property_read_bool(dev->of_node, "hardware_led");
    active_high = of_property_read_bool(dev->of_node, "active_high");
    is_crossbar = of_property_read_bool(dev->of_node, "crossbar");

    ret = bca_cled_setup_serial(led_num, active_high, is_hw, dev->of_node, is_crossbar);
    if (ret < 0)
    {
        dev_err(dev, "Failed to setup serial led %d\n", led_num);
        return ret;
    }

    led_num = (uint32_t)ret; /* In some Led controllers the operational bit is changed to new one */

    ret = of_property_read_u32(dev->of_node, "brightness", &init_brightness);
    ret = of_property_read_u32(dev->of_node, "flash_rate", &init_flash_rate);

    bca_cled_set_brightness(led_num, init_brightness);
    bca_cled_set_flash_rate(led_num, init_flash_rate & 0x7);

    ret = of_property_read_u32(dev->of_node, "default_state", &default_state);
    if (!ret)
    {
        bca_cled_set_value(led_num, default_state);
    }

    led->pin = led_num;
    if (is_hw)
    {
        led->cdev.brightness_set = NULL;
        led->cdev.blink_set = NULL;
    }
    else
    {
        led->cdev.brightness_set = bca_cled_set;
        led->cdev.blink_set = bca_c_blink_set;
    }
    led->cdev.name = of_get_property(dev->of_node, "label", NULL) ? : dev->of_node->name;

    return 0;
}

static int parallel_led_parse(struct device *dev, struct bca_led *led)
{
    bool active_high;
    bool is_hw;
    uint32_t led_num;
    uint32_t init_brightness = 255;
    uint32_t default_state;
    uint32_t init_flash_rate = 0;
    int ret;

    if (of_property_read_u32(dev->of_node, "bit", &led_num))
    {
        dev_err(dev, "bit property is not specified\n");
        return -EINVAL;
    }

    is_hw = of_property_read_bool(dev->of_node, "hardware_led");
    active_high = of_property_read_bool(dev->of_node, "active_high");

    ret = bca_led_setup_parallel(led_num, active_high, is_hw);
    if (ret)
    {
        if (ret != -ENODEV)
            dev_err(dev, "Failed to setup parallel led %d\n", led_num);
        return ret;
    }

    ret = of_property_read_u32(dev->of_node, "brightness", &init_brightness);
    ret = of_property_read_u32(dev->of_node, "flash_rate", &init_flash_rate);

    bca_led_set_brightness(led_num, init_brightness == 0 ? 0 : init_brightness < 31 ?
        1 : init_brightness/31);
    bca_led_set_flash_rate(led_num, init_flash_rate & 0x7);

    ret = of_property_read_u32(dev->of_node, "default_state", &default_state);
    if (!ret)
    {
        bca_led_set_value(led_num, default_state);
    }

    led->pin = led_num;
    if (is_hw)
    {
        led->cdev.brightness_set = NULL;
        led->cdev.blink_set = NULL;
    }
    else
    {
        led->cdev.brightness_set = bca_led_set;
        led->cdev.blink_set = bca_blink_set;
    }
    led->cdev.name = of_get_property(dev->of_node, "label", NULL) ? : dev->of_node->name;

    return 0;
}

static int parallel_cled_parse(struct device *dev, struct bca_led *led)
{
    bool active_high;
    bool is_hw;
    uint32_t led_num;
    uint32_t init_brightness = 128;
    uint32_t default_state;
    uint32_t init_flash_rate = 0;
    int ret;
    bool is_crossbar;

    if (of_property_read_u32(dev->of_node, "bit", &led_num))
    {
        dev_err(dev, "bit property is not specified\n");
        return -EINVAL;
    }

    is_hw = of_property_read_bool(dev->of_node, "hardware_led");
    active_high = of_property_read_bool(dev->of_node, "active_high");
    is_crossbar = of_property_read_bool(dev->of_node, "crossbar");

    ret = bca_cled_setup_parallel(led_num, active_high, is_hw, dev->of_node, is_crossbar);
    if (ret < 0)
    {
        dev_err(dev, "Failed to setup parallel led %d\n", led_num);
        return ret;
    }

    led_num = (uint32_t)ret; /* In some Led controllers the operational bit is changed to new one */

    ret = of_property_read_u32(dev->of_node, "brightness", &init_brightness);
    ret = of_property_read_u32(dev->of_node, "flash_rate", &init_flash_rate);

    bca_cled_set_brightness(led_num, init_brightness);
    bca_cled_set_flash_rate(led_num, init_flash_rate & 0x7);

    ret = of_property_read_u32(dev->of_node, "default_state", &default_state);
    if (!ret)
    {
        bca_cled_set_value(led_num, default_state);
    }

    led->pin = led_num;
    if (is_hw)
    {
        led->cdev.brightness_set = NULL;
        led->cdev.blink_set = NULL;
    }
    else
    {
        led->cdev.brightness_set = bca_cled_set;
        led->cdev.blink_set = bca_c_blink_set;
    }
    led->cdev.name = of_get_property(dev->of_node, "label", NULL) ? : dev->of_node->name;

    return 0;
}

static const struct of_device_id bca_leds_of_match[] = {
    { .compatible = "brcm,serial-led", .data = (void *)serial_led_parse, },
    { .compatible = "brcm,serial-cled", .data = (void *)serial_cled_parse, },
	{ .compatible = "brcm,gpio-led", .data = (void *)gpio_led_parse, },
    { .compatible = "brcm,parallel-led", .data = (void *)parallel_led_parse, },
    { .compatible = "brcm,parallel-cled", .data = (void *)parallel_cled_parse, },
	{ },
};

MODULE_DEVICE_TABLE(of, bca_leds_of_match);

static int bca_leds_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    const struct of_device_id *match;
    struct bca_led *led;
    int ret = 0;
    int(*parse_func)(struct device *dev, struct bca_led *led);

    match = of_match_device(bca_leds_of_match, dev);
    if (!match)
    {
        dev_err(dev, "Failed to find correct LED interface\n");
        return -ENODEV;
    }

    led = devm_kzalloc(dev, sizeof(*led), GFP_KERNEL);
    if (!led)
    {
        dev_err(dev, "Failed to allocate memory for LED\n");
        return -ENOMEM;
    }

    parse_func = (int(*)(struct device *, struct bca_led *))match->data;
    ret = parse_func(dev, led);
    if (ret)
        goto error; 
    
    if (led->cdev.brightness_set) /* Software leds will be registered in Linux framework */
    {
        ret = led_classdev_register(dev, &led->cdev);
        if (ret)
        {
            dev_err(dev, "Failed to register SW LED %d error %d \n", led->pin, ret);
            goto error;
        }
        dev_info(dev, "SW led %d registered\n", led->pin);
        
        bca_leds_init_done = 1;
    }
    else
    {
        dev_info(dev, "HW led %d registered\n", led->pin);
        devm_kfree(dev, led); /* Free not needed led object */
    }

    return 0;
error:
    devm_kfree(dev, led);
    return ret;
}

static struct platform_driver bcm_bca_leds_driver = {
	.probe = bca_leds_probe,
	.driver = {
		.name = "bcm-bca-leds",
		.of_match_table = bca_leds_of_match,
	},
};

#define led_err(fmt, ...) \
    printk(KERN_ERR "%s:L%d "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define led_info(fmt, ...) \
    printk(KERN_INFO "%s:L%d "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

struct led_classdev *bca_led_request_sw_led(struct device_node *dn, const char *consumer_led_name)
{
    struct device_node *led_node;
    struct bca_led *bca_led;
    uint32_t led;

    led_node = of_parse_phandle(dn, consumer_led_name, 0);
    if (!led_node)
    {
        led_info("%s led not found\n", consumer_led_name);
        return NULL;
    }

    if (of_property_read_u32(led_node, "bit", &led))
    {
        if (of_property_read_u32(led_node, "pin", &led))
        {
            led_err("LED structure referenced by %s is in wrong format\n", consumer_led_name);
            return NULL;
        }
    }

    bca_led = find_led_device_by_pin(led); 
    if (bca_led == ERR_PTR(-EPROBE_DEFER))
        return ERR_PTR(-EPROBE_DEFER);

    if (!bca_led)
        return NULL; 

    return &bca_led->cdev;
}

void bca_led_request_network_leds(struct device_node *dn, bca_leds_info_t *led_info)
{
    int i, cb_ep_num;
    struct device_node *led_node;
    uint32_t port, led, sw_port;
    uint32_t link;
    uint32_t activity;
    uint32_t skip;
    char prop_name[16];
    int ret;

    ret = of_property_read_u32(dn, "reg", &sw_port);
    if (ret) {
        led_err("Failed to read the switch port number for port %s!\n", dn->name);
        return;
    }

    /* 
     * This simplified implementation assumes:
     * 1. led_info[0] is shared by network-led property for regular port and network-led-0
     *    for crossbar port external end point 0.
     * 2. network-led and network-led-n property does not co-exist in the same port node. 
     *    This is true for the current devices. 
     * 3. If such needs arrives in the future devices, rework is needed for this driver 
     *    and crossbar phy driver. 
     */
    for (cb_ep_num = -1; cb_ep_num < MAX_PHYS_PER_CROSSBAR_GROUP; cb_ep_num++)
    {
        led_info->port_id = 0xff;
        led_info->skip_in_aggregate = 0;
        for(i = 0; i < 4; i++)
        {
            led_info->link[i] = 0;
            led_info->activity[i] = 0;
        }

        if (cb_ep_num != -1)
            snprintf(&prop_name[0], 15, "network-leds-%d", cb_ep_num);
        else
            snprintf(&prop_name[0], 15, "network-leds");

        for(i = 0; i < 4; i++)
        {
            led_node = of_parse_phandle(dn, prop_name, i);
            if (!led_node)
               break;

            if (!of_property_read_bool(led_node, "hardware_led"))
            {
                led_err("Only Hardware LEDs could be used as Network Leds node %s\n", led_node->name);
                continue;
            }

            ret = of_property_read_u32(led_node, "port", &port);
            if (ret)
            {
                led_err("LED structure referenced by %s is in wrong format\n", led_node->name);
                continue;
            }

            /*
             * Switch port should match led port number. In case of crossbar switch port, 
             * the external endpoint number should match led port number.
             */
            if ((cb_ep_num != -1 && cb_ep_num != port) ||
                (cb_ep_num == -1 && sw_port != port)) {
                led_err("Invalid HW LED assigned to sw port %d cb ext ep %d referenced from led port %d node %s\n",
                    sw_port, cb_ep_num, port, led_node->name);
                continue;
            }

            ret = of_property_read_u32(led_node, "led", &led);
            if (ret)
            {
                led_err("LED structure referenced by %s is in wrong format\n", led_node->name);
                continue;
            }

            link = 0;
            activity = 0;
            skip = 0;
            of_property_read_u32(led_node, "link", &link);
            of_property_read_u32(led_node, "activity", &activity);
            of_property_read_u32(led_node, "no_aggregate", &skip);
            if (link == 0 && activity == 0)
            {
                led_err("At least one of the activity/link LED functionality must be specified for port %d led %d\n",
                    port, led);
                continue;
            }

            led_info("network led %d for %s %d link 0x%x activity 0x%x\n",
                led, cb_ep_num != -1 ? "crossbar port" : "port", port, link, activity);

            led_info->port_id = sw_port;
            led_info->skip_in_aggregate = led_info->skip_in_aggregate ? led_info->skip_in_aggregate : skip;
            led_info->link[led] = link;
            led_info->activity[led] = activity;
        }

        /* network-leds and network-leds-0 should share the same placeholder*/
        if (cb_ep_num != -1)
            led_info++;
        else if(led_info->port_id != 0xff) /* && cb_ep_num == -1 */
        {
            /* only one of the following formats are supported:                   */
            /*   networks-leds for ports with only one possible led configuration */
            /*   networks-leds-X for ports with multiple leds configurations.     */
            return;
        }
    }
}

EXPORT_SYMBOL(bca_led_request_sw_led);
EXPORT_SYMBOL(bca_led_request_network_leds);

static int __init bcmbca_leds_drv_reg(void)
{
	return platform_driver_register(&bcm_bca_leds_driver);
}

subsys_initcall(bcmbca_leds_drv_reg);

MODULE_AUTHOR("Samyon Furman (samyon.furman@broadcom.com)");
MODULE_DESCRIPTION("Broadcom BCA LEDs  Driver");
MODULE_LICENSE("GPL v2");

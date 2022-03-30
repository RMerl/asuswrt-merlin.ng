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
#include <led.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/io.h>

#include "bcm_bca_leds.h"
struct bca_led {
    struct led_ops ops;
    uint32_t pin;
    struct udevice *dev;
	struct gpio_desc desc;
    struct list_head node;
};

extern int bca_led_set_value(unsigned int led_num, unsigned int value);
extern int bca_led_get_value(unsigned int led_num);
extern int bca_led_set_brightness(unsigned int led_num, unsigned int value);
extern int bca_led_set_flash_rate(unsigned int led_num, unsigned int value);
extern int bca_led_setup_serial(unsigned int led_num, unsigned int polarity, unsigned int is_hw);
extern int bca_led_setup_parallel(unsigned int led_num, unsigned int polarity, unsigned int is_hw);
/*CLED controller APIs*/
extern int bca_cled_set_value(unsigned int led_num, unsigned int value);
extern int bca_cled_get_value(unsigned int led_num);
extern int bca_cled_set_brightness(unsigned int led_num, unsigned int value);
extern int bca_cled_set_flash_rate(unsigned int led_num, unsigned int value);
extern int bca_cled_setup_serial(unsigned int led_num, unsigned int polarity, unsigned int is_hw,
    struct udevice *dev, bool is_crossbar);
extern int bca_cled_setup_parallel(unsigned int led_num, unsigned int polarity, unsigned int is_hw,
    struct udevice *dev, bool is_crossbar);

LIST_HEAD(leds_list);

struct udevice* find_led_device_by_pin(uint32_t pin)
{
    struct  bca_led *led;
    list_for_each_entry(led, &leds_list, node)
    {
        if (pin == led->pin)
            return led->dev;
    }
    return NULL;
}

static int bca_gpio_led_set(struct udevice *dev, enum led_state_t value)
{
    struct bca_led *led = dev_get_priv(dev);

    if (value == LEDST_OFF || value == LEDST_ON)
        dm_gpio_set_value(&led->desc, value == LEDST_OFF ? 0 : 1);
    else
    {
        int state;
        state  = dm_gpio_get_value(&led->desc);
        dm_gpio_set_value(&led->desc, state == 0 ? 1 : 0);
    }
    return 0;
}

static enum led_state_t bca_gpio_led_get(struct udevice *dev)
{
    struct bca_led *led = dev_get_priv(dev);

    int state;
    state  = dm_gpio_get_value(&led->desc);
    return state == 0 ? LEDST_OFF : LEDST_ON;
}

static int gpio_led_parse(struct udevice *dev, struct bca_led *led)
{
    uint32_t gpio;
    int ret;
    unsigned long gflags = GPIOD_IS_OUT;
    struct led_uc_plat *uc_plat;
    char name[5];

    if (!dev_read_bool(dev, "software_led"))
    {
        printf("Hardware LEDs over GPIO inerface is not supported\n");
        return -EINVAL;
    }

    if (dev_read_u32u(dev, "pin", &gpio))
    {
        printf("GPIO pin is not specified\n");
        return -EINVAL;
    }

    if (dev_read_bool(dev, "active_low"))
        gflags |= GPIOD_ACTIVE_LOW;
    uc_plat = dev_get_uclass_platdata(dev);
    uc_plat->label = dev_read_prop(dev, "label", NULL) ? : dev_read_name(dev);

    snprintf(name, 5, "%d", gpio);
    ret = dm_gpio_lookup_name(name, &led->desc);
    if (ret)
    {
        printf("Failed to find GPIO %d(%s)\n", gpio, name);
        return ret;
    }

    ret = dm_gpio_request(&led->desc, uc_plat->label); 
    if (ret)
    {
        printf("Failed to request GPIO %d\n", gpio);
        return ret;
    }

    led->pin = gpio;

    dm_gpio_set_dir_flags(&led->desc, gflags);

    led->ops.set_state = bca_gpio_led_set;
    led->ops.get_state = bca_gpio_led_get;
#ifdef CONFIG_LED_BLINK    
    led->ops.set_period = NULL;
#endif
    return 0;
}

static enum led_state_t common_bca_led_get(struct udevice *dev, int is_cled)
{
    struct bca_led *led = dev_get_priv(dev);
    int curr_value; 
    if (is_cled)
        curr_value = bca_cled_get_value(led->pin);
    else
        curr_value = bca_led_get_value(led->pin);
    return curr_value == 0 ? LEDST_OFF : LEDST_ON; 
}

static enum led_state_t bca_cled_get(struct udevice *dev)
{
    return common_bca_led_get(dev, 1);
}

static enum led_state_t bca_led_get(struct udevice *dev)
{
    return common_bca_led_get(dev, 0);
}

static int common_bca_led_set(struct udevice *dev, enum led_state_t value, int is_cled)
{
    int (* led_set_value)(unsigned int led_num, unsigned int value) = NULL;
    int (* led_set_flash_rate)(unsigned int led_num, unsigned int value) = NULL;
    int (* led_get_value)(unsigned int led_num) = NULL;

    struct bca_led *led = dev_get_priv(dev);
    int curr_value; 

    if (is_cled)
    {
        led_set_value = bca_cled_set_value;
        led_get_value = bca_cled_get_value;
        led_set_flash_rate = bca_cled_set_flash_rate;
    }
    else
    {
        led_set_value = bca_led_set_value;
        led_get_value = bca_led_get_value;
        led_set_flash_rate = bca_led_set_flash_rate;
    }

    switch (value)
    {
    case LEDST_OFF:
        led_set_value(led->pin, 0);
        led_set_flash_rate(led->pin, 0);
        break;
#ifdef CONFIG_LED_BLINK
    case LEDST_BLINK:
#endif
    case LEDST_ON:
        led_set_value(led->pin, 1);
        break;
    case LEDST_TOGGLE:
        curr_value = led_get_value(led->pin);
        if (curr_value > 0)
            led_set_value(led->pin, 0);
        else if (curr_value == 0)
            led_set_value(led->pin, 1);

    default:
        break;
    }
    return 0;
}

static int bca_cled_set(struct udevice *dev, enum led_state_t value)
{
    return common_bca_led_set(dev, value, 1);
}

static int bca_led_set(struct udevice *dev, enum led_state_t value)
{
    return common_bca_led_set(dev, value, 0);
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

#ifdef CONFIG_LED_BLINK
static int common_bca_blink_set(struct udevice *dev, int delay_on, int is_cled)
{
    struct bca_led *led = dev_get_priv(dev);
    unsigned int delay;

    delay = convert_ms_to_hz(delay_on);

    if (is_cled)
        bca_cled_set_flash_rate(led->pin, delay);
    else
        bca_led_set_flash_rate(led->pin, delay);

    return 0;
}

static int bca_blink_set(struct udevice *dev, int delay_on)
{
    return common_bca_blink_set(dev, delay_on, 0);
}

static int bca_c_blink_set(struct udevice *dev, int delay_on)
{
    return common_bca_blink_set(dev, delay_on, 1);
}
#endif

static int serial_led_parse(struct udevice *dev, struct bca_led *led)
{
    bool active_high;
    bool is_hw;
    uint32_t led_num;
    uint32_t init_brightness = 255;
    uint32_t init_flash_rate = 0;
    uint32_t default_state;
    struct led_uc_plat *uc_plat;
    int ret;

    if (dev_read_u32u(dev, "bit", &led_num))
    {
        printf("bit property is not specified\n");
        return -EINVAL;
    }

    is_hw = dev_read_bool(dev, "hardware_led");
    active_high = dev_read_bool(dev, "active_high");

    ret = bca_led_setup_serial(led_num, active_high, is_hw);
    if (ret)
    {
        if (ret != -ENODEV)
            printf("Failed to setup serial led %d\n", led_num);
        return ret;
    }

    ret = dev_read_u32u(dev, "brightness", &init_brightness);
    ret = dev_read_u32u(dev, "flash_rate", &init_flash_rate);

    bca_led_set_brightness(led_num, init_brightness == 0 ? 0 : init_brightness < 31 ?
        1 : init_brightness/31);
    bca_led_set_flash_rate(led_num, init_flash_rate & 0x7);

    ret = dev_read_u32u(dev, "default_state", &default_state);
    if (!ret)
    {
        bca_led_set_value(led_num, default_state);
    }

    led->pin = led_num;
    if (is_hw)
    {
        led->ops.set_state = NULL;
        led->ops.get_state = NULL;
#ifdef CONFIG_LED_BLINK
        led->ops.set_period = NULL;
#endif
    }
    else
    {
        led->ops.set_state = bca_led_set;
        led->ops.get_state = bca_led_get;
#ifdef CONFIG_LED_BLINK
        led->ops.set_period = bca_blink_set;
#endif
    }
    uc_plat = dev_get_uclass_platdata(dev);
    uc_plat->label = dev_read_prop(dev, "label", NULL) ? : dev_read_name(dev);

    return 0;
}

static int serial_cled_parse(struct udevice *dev, struct bca_led *led)
{
    bool active_high;
    bool is_hw;
    uint32_t led_num;
    uint32_t init_brightness = 255;
    uint32_t init_flash_rate = 0;
    uint32_t default_state;
    struct led_uc_plat *uc_plat;
    int ret;
    bool is_crossbar;

    if (dev_read_u32u(dev, "bit", &led_num))
    {
        printf("bit property is not specified\n");
        return -EINVAL;
    }

    is_hw = dev_read_bool(dev, "hardware_led");
    active_high = dev_read_bool(dev, "active_high");
    is_crossbar = dev_read_bool(dev, "crossbar");

    ret = bca_cled_setup_serial(led_num, active_high, is_hw, dev, is_crossbar);

    if (ret < 0)
    {
        printf("Failed to setup serial led %d\n", led_num);
        return ret;
    }

    led_num = (uint32_t)ret; /* In some Led controllers the operational bit is changed to new one */

    ret = dev_read_u32u(dev, "brightness", &init_brightness);
    ret = dev_read_u32u(dev, "flash_rate", &init_flash_rate);

    bca_cled_set_brightness(led_num, init_brightness);
    bca_cled_set_flash_rate(led_num, init_flash_rate & 0x7);

    ret = dev_read_u32u(dev, "default_state", &default_state);
    if (!ret)
    {
        bca_cled_set_value(led_num, default_state);
    }

    led->pin = led_num;
    if (is_hw)
    {
        led->ops.set_state = NULL;
        led->ops.get_state = NULL;
#ifdef CONFIG_LED_BLINK
        led->ops.set_period = NULL;
#endif
    }
    else
    {
        led->ops.set_state = bca_cled_set;
        led->ops.get_state = bca_cled_get;
#ifdef CONFIG_LED_BLINK
        led->ops.set_period = bca_c_blink_set;
#endif
    }
    uc_plat = dev_get_uclass_platdata(dev);
    uc_plat->label = dev_read_prop(dev, "label", NULL) ? : dev_read_name(dev);

    return 0;
}

static int parallel_led_parse(struct udevice *dev, struct bca_led *led)
{
    bool active_high;
    bool is_hw;
    uint32_t led_num;
    uint32_t init_brightness = 255;
    uint32_t init_flash_rate = 0;
    uint32_t default_state;
    struct led_uc_plat *uc_plat;
    int ret;

    if (dev_read_u32u(dev, "bit", &led_num))
    {
        printf("bit property is not specified\n");
        return -EINVAL;
    }

    is_hw = dev_read_bool(dev, "hardware_led");
    active_high = dev_read_bool(dev, "active_high");

    ret = bca_led_setup_parallel(led_num, active_high, is_hw);

    if (ret)
    {
        if (ret != -ENODEV)
            printf("Failed to setup parallel led %d\n", led_num);
        return ret;
    }

    ret = dev_read_u32u(dev, "brightness", &init_brightness);
    ret = dev_read_u32u(dev, "flash_rate", &init_flash_rate);

    bca_led_set_brightness(led_num, init_brightness == 0 ? 0 : init_brightness < 31 ?
        1 : init_brightness/31);
    bca_led_set_flash_rate(led_num, init_flash_rate & 0x7);

    ret = dev_read_u32u(dev, "default_state", &default_state);
    if (!ret)
    {
        bca_led_set_value(led_num, default_state);
    }

    led->pin = led_num;
    if (is_hw)
    {
        led->ops.set_state = NULL;
        led->ops.get_state = NULL;
#ifdef CONFIG_LED_BLINK
        led->ops.set_period = NULL;
#endif
    }
    else
    {
        led->ops.set_state = bca_led_set;
        led->ops.get_state = bca_led_get;
#ifdef CONFIG_LED_BLINK
        led->ops.set_period = bca_blink_set;
#endif
    }
    uc_plat = dev_get_uclass_platdata(dev);
    uc_plat->label = dev_read_prop(dev, "label", NULL) ? : dev_read_name(dev);

    return 0;
}

static int parallel_cled_parse(struct udevice *dev, struct bca_led *led)
{
    bool active_high;
    bool is_hw;
    uint32_t led_num;
    uint32_t init_brightness = 255;
    uint32_t init_flash_rate = 0;
    uint32_t default_state;
    struct led_uc_plat *uc_plat;
    int ret;
    bool is_crossbar;

    if (dev_read_u32u(dev, "bit", &led_num))
    {
        printf("bit property is not specified\n");
        return -EINVAL;
    }

    is_hw = dev_read_bool(dev, "hardware_led");
    active_high = dev_read_bool(dev, "active_high");
    is_crossbar = dev_read_bool(dev, "crossbar");

    ret = bca_cled_setup_parallel(led_num, active_high, is_hw, dev, is_crossbar);

    if (ret < 0)
    {
        printf("Failed to setup parallel led %d\n", led_num);
        return ret;
    }
    
    led_num = (uint32_t)ret; /* In some Led controllers the operational bit is changed to new one */

    ret = dev_read_u32u(dev, "brightness", &init_brightness);
    ret = dev_read_u32u(dev, "flash_rate", &init_flash_rate);

    bca_cled_set_brightness(led_num, init_brightness);
    bca_cled_set_flash_rate(led_num, init_flash_rate & 0x7);

    ret = dev_read_u32u(dev, "default_state", &default_state);
    if (!ret)
    {
        bca_cled_set_value(led_num, default_state);
    }

    led->pin = led_num;
    if (is_hw)
    {
        led->ops.set_state = NULL;
        led->ops.get_state = NULL;
#ifdef CONFIG_LED_BLINK
        led->ops.set_period = NULL;
#endif
    }
    else
    {
        led->ops.set_state = bca_cled_set;
        led->ops.get_state = bca_cled_get;
#ifdef CONFIG_LED_BLINK
        led->ops.set_period = bca_c_blink_set;
#endif
    }
    uc_plat = dev_get_uclass_platdata(dev);
    uc_plat->label = dev_read_prop(dev, "label", NULL) ? : dev_read_name(dev);

    return 0;
}

static enum led_state_t bcm_led_get_state(struct udevice *dev)
{
    struct bca_led *led;
    led = dev_get_priv(dev);

    if (led->ops.get_state)
        return led->ops.get_state(dev);
    else
        return LEDST_OFF;
}

static int bcm_led_set_state(struct udevice *dev, enum led_state_t state)
{
    struct bca_led *led;
    led = dev_get_priv(dev);

    if (led->ops.set_state)
        return led->ops.set_state(dev, state);
    else
        return -EINVAL;
};

#ifdef CONFIG_LED_BLINK
static int bcm_led_set_period(struct udevice *dev, int period_ms)
{
    struct bca_led *led;
    led = dev_get_priv(dev);
    if (led->ops.set_period)
        return led->ops.set_period(dev, period_ms);
    else
        return -EINVAL;
}
#endif

static const struct led_ops bcm_bca_led_ops = {
	.get_state = bcm_led_get_state,
	.set_state = bcm_led_set_state,
#ifdef CONFIG_LED_BLINK
	.set_period = bcm_led_set_period,
#endif
};

static const struct udevice_id bca_leds_of_match[] = {
    { .compatible = "brcm,serial-led", .data = (long unsigned int )serial_led_parse, },
    { .compatible = "brcm,serial-cled", .data = (long unsigned int )serial_cled_parse, },
	{ .compatible = "brcm,gpio-led", .data = (long unsigned int)gpio_led_parse, },
    { .compatible = "brcm,parallel-led", .data = (long unsigned int)parallel_led_parse, },
    { .compatible = "brcm,parallel-cled", .data = (long unsigned int)parallel_cled_parse, },
	{ },
};

static int driver_check_compatible(const struct udevice_id *of_match,
				   const struct udevice_id **of_idp,
				   const char *compat)
{
	if (!of_match)
		return -ENOENT;

	while (of_match->compatible) {
		if (!strcmp(of_match->compatible, compat)) {
			*of_idp = of_match;
			return 0;
		}
		of_match++;
	}

	return -ENOENT;
}

static int bca_leds_bind(struct udevice *dev)
{
    const char *compat, *status;
    const struct udevice_id *id;
    int ret;

    compat = dev_read_prop(dev, "compatible", NULL);
    ret = driver_check_compatible(bca_leds_of_match, &id, compat);
    if (!ret)
    {
        status = dev_read_prop(dev, "status", NULL);
        if (!strcmp("ok", status) || !strcmp("okay", status))
            dev->driver_data = id->data;
        else
            ret = -ENODEV;
    }

    return ret;
}

static int bca_leds_probe(struct udevice *dev)
{
    struct bca_led *led;
    int ret = 0;
    int(*parse_func)(struct udevice *dev, struct bca_led *led);
    ulong data = 0;

    data = dev_get_driver_data(dev);
    if (!data)
        return -ENODEV;

    led = dev_get_priv(dev);

    parse_func = (int(*)(struct udevice *, struct bca_led *))data;
    ret = parse_func(dev, led);
    if (ret)
        goto error; 
    
    if (led->ops.set_state) /* Software leds will be registered in Linux framework */
    {
        list_add_tail(&led->node, &leds_list);
        printf("SW led %d registered\n", led->pin);
    }
    else
    {
        printf("HW led %d registered\n", led->pin);
    }

    return 0;
error:
    return ret;
}

U_BOOT_DRIVER(bcm_bca_leds_driver) = {
    .name = "bcm-bca-leds",
	.id = UCLASS_LED,
    .of_match = bca_leds_of_match,
	.probe = bca_leds_probe,
    .bind = bca_leds_bind,
	.priv_auto_alloc_size = sizeof(struct bca_led),
    .ops = &bcm_bca_led_ops,
};

#define led_err(fmt, ...) \
    printf("%s:L%d "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define led_info(fmt, ...) \
    printf("%s:L%d "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

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

struct udevice * bca_led_request_sw_led(ofnode dn, const char *consumer_led_name)
{
    ofnode led_node;
    uint32_t led;

    led_node = of_parse_phandle(dn, consumer_led_name, 0);
    if (led_node.of_offset == -1)
    {
        led_info("%s led not found\n", consumer_led_name);
        return NULL;
    }

    if (ofnode_read_u32(led_node, "bit", &led))
    {
        if (ofnode_read_u32(led_node, "pin", &led))
        {
            led_err("LED structure referenced by %s is in wrong format\n", consumer_led_name);
            return NULL;
        }
    }

    return find_led_device_by_pin(led); 
}

void bca_led_request_network_leds(ofnode dn, bca_leds_info_t *led_info)
{
    int i;
    ofnode led_node;
    uint32_t port, led;
    uint32_t link;
    uint32_t activity;
    int ret;

    led_info->port_id = 0xff;
    for(i = 0; i < 4; i++)
    {
        led_info->link[i] = 0;
        led_info->activity[i] = 0;
    }

    for(i = 0; i < 4; i++)
    {
        led_node = of_parse_phandle(dn, "network-leds", i);
        if (led_node.of_offset == -1)
            return;

        if (!ofnode_read_bool(led_node, "hardware_led"))
        {
            led_err("Only Hardware LEDs could be used as Network Leds node\n");
            continue;
        }

        ret = ofnode_read_u32(led_node, "port", &port);
        if (ret)
        {
            led_err("LED structure referenced by is in wrong format\n");
            continue;
        }
        
        if (led_info->port_id != 0xff && led_info->port_id != port)
        {
            led_err("HW LED assigned to port %d referenced from port %d\n", port, led_info->port_id);
            continue;
        }

        ret = ofnode_read_u32(led_node, "led", &led);
        if (ret)
        {
            led_err("LED structure referenced is in wrong format\n");
            continue;
        }

        link = 0;
        activity = 0;
        ofnode_read_u32(led_node, "link", &link);
        ofnode_read_u32(led_node, "activity", &activity);

        led_info("network led %d for port %d link 0x%x activity 0x%x\n", led, port, link, activity);

        if (link == 0 && activity == 0)
        {
            led_err("At least one of the activity/link LED functionality must be specified for port %d led %d\n",
                port, led);
            continue;
        }
        led_info->port_id = port;
        led_info->link[led] = link;
        led_info->activity[led] = activity;
    }
}


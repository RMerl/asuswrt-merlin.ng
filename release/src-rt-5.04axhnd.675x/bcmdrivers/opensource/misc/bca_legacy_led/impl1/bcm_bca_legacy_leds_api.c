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
#include <linux/slab.h>
#include <linux/version.h>

#include "bcm_bca_legacy_led_api.h"
#include "bcm_bca_leds.h"

struct bca_legacy_led {
    struct led_classdev *cdev;
    BOARD_LED_NAME led_name;
    BOARD_LED_TYPE led_type;
    const char *consumer_name;
    struct list_head node;
};

LIST_HEAD(bca_legacy_leds_list);

struct bca_legacy_led legacy_leds[] = {
    {.consumer_name = "adsl-led", .led_name = kLedAdsl, .led_type = kLedOK},
    {.consumer_name = "adsl-fail-led", .led_name = kLedAdsl, .led_type = kLedFail},
    {.consumer_name = "sec-adsl-led", .led_name = kLedSecAdsl, .led_type = kLedOK},
    {.consumer_name = "sec-adsl-fail-led", .led_name = kLedSecAdsl, .led_type = kLedFail},
    {.consumer_name = "wan-data-led", .led_name = kLedWanData, .led_type = kLedOK},
    {.consumer_name = "wan-data-fail-led", .led_name = kLedWanData, .led_type = kLedFail},
    {.consumer_name = "wl-sess-led", .led_name = kLedSes, .led_type = kLedOK},
    {.consumer_name = "voip-led", .led_name = kLedVoip, .led_type = kLedOK},
    {.consumer_name = "voip1-ok-led", .led_name = kLedVoip1, .led_type = kLedOK},
    {.consumer_name = "voip1-fail-led", .led_name = kLedVoip1, .led_type = kLedFail},
    {.consumer_name = "voip2-ok-led", .led_name = kLedVoip2, .led_type = kLedOK},
    {.consumer_name = "voip2-fail-led", .led_name = kLedVoip2, .led_type = kLedFail},
    {.consumer_name = "pots-led", .led_name = kLedPots, .led_type = kLedOK},
    {.consumer_name = "dect-led", .led_name = kLedDect, .led_type = kLedOK},
    {.consumer_name = "wl0-led", .led_name = kLedWL0, .led_type = kLedOK},
    {.consumer_name = "wl1-led", .led_name = kLedWL1, .led_type = kLedOK},
    {.consumer_name = "usb-led", .led_name = kLedUSB, .led_type = kLedOK},
    {.consumer_name = "sim-led", .led_name = kLedSim, .led_type = kLedOK},
    {.consumer_name = "sim-itms-led", .led_name = kLedSimITMS, .led_type = kLedOK},
    {.led_name = kLedEnd}, 
};

static struct led_classdev * find_led_device_by_led_name(BOARD_LED_NAME led_name, BOARD_LED_TYPE led_type)
{
    struct bca_legacy_led *led;

    if (led_name == kLedEpon) /* Use the same LED */
        led_name = kLedGpon;

    list_for_each_entry(led, &bca_legacy_leds_list, node)
    {
        if ((led_name == led->led_name) && (led_type == led->led_type))
            return led->cdev;
    }
    return NULL;
}

static const struct of_device_id bca_legacy_leds_of_match[] = {
    { .compatible = "brcm,legacy-led", .data = NULL, },
    {},
};

MODULE_DEVICE_TABLE(of, bca_legacy_leds_of_match);

static int bca_legacy_leds_probe(struct platform_device *pdev)
{
    int i = 0;
    struct device *dev = &pdev->dev;
    struct led_classdev *cled = NULL;
    struct bca_legacy_led *leg_led = &legacy_leds[i++];

    while (leg_led->led_name != kLedEnd)
    {
        cled = bca_led_request_sw_led(dev->of_node, leg_led->consumer_name);
        if (cled == ERR_PTR(-EPROBE_DEFER))
            return -EPROBE_DEFER;

        if (cled)
        {
           leg_led->cdev = cled;
           list_add_tail(&leg_led->node, &bca_legacy_leds_list);
           dev_info(dev, "Legacy Led %s registered\n", leg_led->consumer_name);
        }

        leg_led = &legacy_leds[i++];
    }
    
    return 0;
}

static struct platform_driver bcm_bca_legacy_leds_driver = {
	.probe = bca_legacy_leds_probe,
	.driver = {
		.name = "bcm-bca-legacy-leds",
		.of_match_table = bca_legacy_leds_of_match,
	},
};

#define led_err(fmt, ...) \
    printk(KERN_ERR "%s:L%d "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define led_info(fmt, ...) \
    printk(KERN_INFO "%s:L%d "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__)

static int bca_led_ctrl(BOARD_LED_NAME led_name, BOARD_LED_STATE led_state)
{
    struct led_classdev *led;
    struct led_classdev *led_fail;
    unsigned long delay_ms_on;
    unsigned long delay_ms_off;

    led = find_led_device_by_led_name(led_name, kLedOK);
    if (!led)
        return -ENODEV;

    led_fail = find_led_device_by_led_name(led_name, kLedFail);

    led->brightness_set(led, LED_OFF); /*This will turn off blinking if set */
    if(led_fail)
        led_fail->brightness_set(led, LED_OFF); /*This will turn off blinking if set */

    switch (led_state)
    {
    case kLedStateOff:
        led->brightness_set(led, LED_OFF);
        break;
    case kLedStateOn:
        led->brightness_set(led, LED_ON);
        break;
    case kLedStateFail:
        if(led_fail)
        {
            led_fail->brightness_set(led, LED_ON);
            break;
        } /* If fail led does not defined use slow blink */
        __attribute__((fallthrough));
    case kLedStateSlowBlinkContinues:
        delay_ms_off = 500;
        delay_ms_on = 500;
        led->blink_set(led, &delay_ms_on, &delay_ms_off);
        led->brightness_set(led, LED_ON);
        break;
    case kLedStateFastBlinkContinues:
        delay_ms_off = 250;
        delay_ms_on = 250;
        led->blink_set(led, &delay_ms_on, &delay_ms_off);
        led->brightness_set(led, LED_ON);
        break;
    case kLedStateUserWpsInProgress:
        delay_ms_off = 200;
        delay_ms_on = 200;
        led->blink_set(led, &delay_ms_on, &delay_ms_off);
        led->brightness_set(led, LED_ON);
        break;
    case kLedStateUserWpsError:
        delay_ms_off = 100;
        delay_ms_on = 100;
        led->blink_set(led, &delay_ms_on, &delay_ms_off);
        led->brightness_set(led, LED_ON);
        break;
    case kLedStateUserWpsSessionOverLap:
        delay_ms_off = 500;
        delay_ms_on = 500;
        led->blink_set(led, &delay_ms_on, &delay_ms_off);
        led->brightness_set(led, LED_ON);
        break;
    }

    return 0;
}

void kerSysLedCtrl(BOARD_LED_NAME ledName, BOARD_LED_STATE ledState)
{
    bca_led_ctrl(ledName, ledState);
}

unsigned int kerSysGetWifiLed(unsigned char core)
{
    struct led_classdev *led;
	BOARD_LED_NAME led_name = 0;

	switch (core) {
	    case 0:
            led_name =  kLedWL0;
	        break;
	    case 1:
            led_name =  kLedWL1;
	        break;
        default:
            return 0;
	}

    led = find_led_device_by_led_name(led_name, kLedOK);
    if (!led)
    {
        led_name = 0;
    }
	return led_name;
}

void kerSysWifiLed(unsigned int led, unsigned int on)
{
	kerSysLedCtrl(led, (on) ? kLedStateOn : kLedStateOff);
}

int bca_legacy_led_request_sw_led(struct device_node *dn, const char *consumer_led_name, 
    BOARD_LED_NAME led_name, BOARD_LED_TYPE led_type)
{
    struct bca_legacy_led *leg_led;
    struct led_classdev *cled;

    cled = bca_led_request_sw_led(dn, consumer_led_name);
    if (!cled)
        return -ENOENT;

    /* try to find this led in DB, if found overwrite it with new value. */
    leg_led = (struct bca_legacy_led *)find_led_device_by_led_name(led_name, led_type);
    if (leg_led == NULL)
    {
        leg_led = kzalloc(sizeof(*leg_led), GFP_KERNEL);
        if (!leg_led)
        {
            led_err("Unable to allocate memory for new LED\n");
            return -ENOMEM;
        }
        leg_led->led_name = led_name;
        leg_led->led_type = led_type;

        list_add_tail(&leg_led->node, &bca_legacy_leds_list);
        led_info("Legacy LED %s registered\n", consumer_led_name);
    }
    else
    {
        /* No need to add to the list already there */
        led_info("Legacy LED %s replaced with %s\n", leg_led->consumer_name, consumer_led_name);
    }

    leg_led->cdev = cled;

    return 0;
}


EXPORT_SYMBOL(bca_legacy_led_request_sw_led);
EXPORT_SYMBOL(kerSysLedCtrl);
EXPORT_SYMBOL(kerSysGetWifiLed);
EXPORT_SYMBOL(kerSysWifiLed);

static int __init bcmbca_legacy_leds_drv_reg(void)
{
	return platform_driver_register(&bcm_bca_legacy_leds_driver);
}

subsys_initcall_sync(bcmbca_legacy_leds_drv_reg);

MODULE_AUTHOR("Samyon Furman (samyon.furman@broadcom.com)");
MODULE_DESCRIPTION("Broadcom BCA Legacy LEDs  Driver");
MODULE_LICENSE("GPL v2");

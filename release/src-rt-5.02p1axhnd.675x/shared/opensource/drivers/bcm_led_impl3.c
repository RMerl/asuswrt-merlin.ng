/*
 * <:copyright-BRCM:2017:DUAL/GPL:standard
 * 
 *    Copyright (c) 2017 Broadcom 
 *    All Rights Reserved
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation (the "GPL").
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * 
 * A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
 * writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 * 
 * :>
 */

#include "boardparms.h"
#include "bcm_led.h"
#include "bcm_gpio.h"

#ifdef _CFE_
#include "lib_types.h"
#include "lib_printf.h"
#include "lib_string.h"
#include "bcm_map.h"
#define printk  printf
#else // Linux
#include <linux/kernel.h>
#include <linux/module.h>
#include <bcm_map_part.h>
#include <linux/string.h>
#endif


// #define BCM_LED_DEBUG 1

/*
  These are low level functions that can be called from CFE or from the Linux board driver
  The Linux board driver handles any necessary locking so these functions should not be called
  directly from elsewhere.
*/


/* These are defined the same as in
 * cfe/cfe/board/bcm63xx_ram/include/bcm63xx_util.h
 */
enum {
    LED_OFF,
    LED_ON
};

/**********************************************************************
 * Name        : bcm_led_driver_set
 *
 * Description : Given a GPIO number and a state, configures the GPIO
 *               for OUTPUT and sets it to the value in state [1,0]
 *
 * Parameters  : [IN] num        - GPIO number (physical GPIO)
 *               [IN] state      - state (1: LED_ON, 0: LED_OFF)
 *
 * Notes       : Uses bcm_gpio functions to configure GPIO registers.
 *               The caller function must handle the GPIO spinlock
 *               when calling this function.
 *
 *               Assumes GPIO_num is in functional mode 0 (not muxed).
 *********************************************************************/
void bcm_led_driver_set(unsigned short num, unsigned short state)
{
#ifdef BCM_LED_DEBUG
    printk("LED %x set state %d\n",num,state);
#endif

    /* Regular GPIO-driven LED */
    bcm_gpio_set_dir(num, 1);
    bcm_gpio_set_data(num, state);
}

/**********************************************************************
 * Name        : bcm_led_driver_toggle
 *
 * Description : Given a GPIO number toggles its value
 *
 * Parameters  : [IN] pin_num      - GPIO number (physical GPIO)
 *
 * Notes       : Uses bcm_gpio functions to configure GPIO registers.
 *               The caller function must handle the GPIO spinlock
 *               when calling this function.
 *
 *               Assumes GPIO_num is in functional mode 0 (not muxed).
 *********************************************************************/
void bcm_led_driver_toggle(unsigned short num)
{
    bcm_gpio_set_data(num, 1 ^ bcm_gpio_get_data(num));
}

/**********************************************************************
 * Name        : bcm_common_led_setAllSoftLedsOff
 *
 * Description : Turns off the PowerOn LED
 *********************************************************************/
void bcm_common_led_setAllSoftLedsOff(void)
{
    unsigned short gpio;
    int i = 0, rc;
    void* token = NULL;

    for(;;)
    {
        rc = BpGetLedGpio(i, &token, &gpio);
        if( rc == BP_MAX_ITEM_EXCEEDED )
            break;
        else if( rc == BP_SUCCESS && gpio != BP_GPIO_NONE )
        {
            if (gpio & BP_ACTIVE_LOW)
                bcm_led_driver_set(gpio, 1);
            else
                bcm_led_driver_set(gpio, 0);
        }
        else
        {
            token = 0;
            i++;
        }
    }

    return;
}

/**********************************************************************
 * Name        : bcm_common_led_init
 *
 *********************************************************************/
void bcm_common_led_init(void) 
{
}

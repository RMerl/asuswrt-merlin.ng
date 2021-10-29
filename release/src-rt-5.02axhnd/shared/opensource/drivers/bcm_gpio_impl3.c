/*
 * <:copyright-BRCM:2016:DUAL/GPL:standard
 * 
 *    Copyright (c) 2016 Broadcom 
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

/*
  These are low level functions that can be called from CFE or from the Linux board driver
  The Linux board driver handles any necessary locking so these functions should not be called
  directly from elsewhere.
*/

/**********************************************************************
 * Name        : bcm_gpio_get_dir
 *
 * Description : Returns the configured direction of a GPIO
 *
 * Parameters  : [IN] gpio_num      - GPIO number (physical GPIO)
 *
 * Returns     : Direction of the GPIO (0: GPIO_IN, 1: GPIO_OUT)
 *********************************************************************/
unsigned int bcm_gpio_get_dir(unsigned int gpio_num)
{
    gpio_num &= BP_GPIO_NUM_MASK;	
    return ((GPIO_WATCHDOG->gpioouten & (1 << gpio_num)) >> gpio_num);
}

/**********************************************************************
 * Name        : bcm_gpio_set_dir
 *
 * Description : Sets GPIO direction
 *
 * Parameters  : [IN] gpio_num    - GPIO number (physical GPIO)
 *               [IN] dir         - Direction (0: GPIO_IN, 1: GPIO_OUT)
 *********************************************************************/
void bcm_gpio_set_dir(unsigned int gpio_num, unsigned int dir)
{
    gpio_num &= BP_GPIO_NUM_MASK;	
    if(dir == GPIO_OUT)
        GPIO_WATCHDOG->gpioouten |= (1 << gpio_num);
    else
        GPIO_WATCHDOG->gpioouten &= ~(1 << gpio_num);
}

/**********************************************************************
 * Name        : bcm_gpio_get_data
 *
 * Description : Gets current GPIO value
 *
 * Parameters  : [IN] gpio_num    - GPIO number (physical GPIO)
 *
 * Returns     : GPIO value
 *********************************************************************/
unsigned int bcm_gpio_get_data(unsigned int gpio_num)
{
    gpio_num &= BP_GPIO_NUM_MASK;	
    return ((GPIO_WATCHDOG->gpioin & (1 << gpio_num)) >> gpio_num);
}

/**********************************************************************
 * Name        : bcm_gpio_set_data
 *
 * Description : Sets GPIO value
 *
 * Parameters  : [IN] gpio_num    - GPIO number (physical GPIO)
 *               [IN] data        - Value
 *********************************************************************/
void bcm_gpio_set_data(unsigned int gpio_num, unsigned int data)
{
    gpio_num &= BP_GPIO_NUM_MASK;	
    if (data)
        GPIO_WATCHDOG->gpioout |= (1 << gpio_num);
    else
        GPIO_WATCHDOG->gpioout &= ~(1 << gpio_num);
}


/**********************************************************************
 * Name        : bcm_multi_gpio_set_dir
 *
 * Description : Sets GPIO direction
 *
 * Parameters  : [IN] gpio_num    - GPIO number (physical GPIO)
 *               [IN] dir         - Direction (0: GPIO_IN, 1: GPIO_OUT)
 *********************************************************************/
void bcm_multi_gpio_set_dir(unsigned int gpio_mask, unsigned int dir)
{
    unsigned int data = GPIO_WATCHDOG->gpioouten;

    data = (data & (~gpio_mask)) | dir;
    GPIO_WATCHDOG->gpioouten = data;
}


/**********************************************************************
 * Name        : bcm_multi_gpio_set_data
 *
 * Description : Sets GPIO value
 *
 * Parameters  : [IN] gpio_num    - GPIO number (physical GPIO)
 *               [IN] data        - Value
 *********************************************************************/
void bcm_multi_gpio_set_data(unsigned int gpio_mask, unsigned int value)
{
    unsigned int data = GPIO_WATCHDOG->gpioout;

    data = (data & (~gpio_mask)) | value;
    GPIO_WATCHDOG->gpioout = data;
}

#ifndef _CFE_
EXPORT_SYMBOL(bcm_gpio_set_dir);
EXPORT_SYMBOL(bcm_gpio_set_data);
EXPORT_SYMBOL(bcm_gpio_get_data);
#endif


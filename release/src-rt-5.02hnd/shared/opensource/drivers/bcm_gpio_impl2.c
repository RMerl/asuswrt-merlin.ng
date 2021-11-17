/*
 * <:copyright-BRCM:2014:DUAL/GPL:standard
 * 
 *    Copyright (c) 2014 Broadcom 
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

// BCMFORMAT: notabs reindent:uncrustify:bcm_minimal_i4.cfg

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
    return ((GPIO->GPIODir & (1 << gpio_num)) >> gpio_num);
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
    if(dir == GPIO_OUT)
        GPIO->GPIODir |= (1 << gpio_num);
    else
        GPIO->GPIODir &= ~(1 << gpio_num);
}
#ifndef _CFE_
EXPORT_SYMBOL(bcm_gpio_set_dir);
#endif

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
    return ((GPIO->GPIOData & (1 << gpio_num)) >> gpio_num);
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
    if (data)
        GPIO->GPIOData |= (1 << gpio_num);
    else
        GPIO->GPIOData &= ~(1 << gpio_num);
}
#ifndef _CFE_
EXPORT_SYMBOL(bcm_gpio_set_data);
#endif

/**********************************************************************
 * Name        : bcm_gpio_get_funcmode
 *
 * Description : Gets current GPIO functional mode
 *
 * Parameters  : [IN] gpio_num    - GPIO number (physical GPIO)
 *
 * Returns     : Functional mode (0: Direct data-driven, 1: Muxed)
 *********************************************************************/
unsigned int bcm_gpio_get_funcmode(unsigned int gpio_num)
{
    return ((GPIO->GPIOFuncMode & (1 << gpio_num)) >> gpio_num);
}

/**********************************************************************
 * Name        : bcm_gpio_set_funcmode
 *
 * Description : Sets current GPIO functional mode
 *
 * Parameters  : [IN] gpio_num    - GPIO number (physical GPIO)
 *               [IN] mode        - Func. mode
 *                                  (0: Data driven, 1: Muxed)
 *********************************************************************/
void bcm_gpio_set_funcmode(unsigned int gpio_num, unsigned int mode)
{
    if (mode == GPIO_MUXED_MODE)
        GPIO->GPIOFuncMode |= (1 << gpio_num);
    else
        GPIO->GPIOFuncMode &= ~(1 << gpio_num);
}

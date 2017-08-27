/*
 * <:copyright-BRCM:2014:DUAL/GPL:standard
 * 
 *    Copyright (c) 2014 Broadcom 
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

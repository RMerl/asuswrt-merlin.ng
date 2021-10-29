/*
 * <:copyright-BRCM:2013:DUAL/GPL:standard
 * 
 *    Copyright (c) 2013 Broadcom 
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

unsigned int bcm_gpio_get_dir(unsigned int gpio_num)
{
    return (GPIO->GPIODir[GPIO_NUM_TO_ARRAY_IDX(gpio_num)] & (GPIO_NUM_TO_MASK(gpio_num))) >> GPIO_NUM_TO_ARRAY_SHIFT(gpio_num);
}
#ifndef _CFE_
EXPORT_SYMBOL(bcm_gpio_get_dir);
#endif

void bcm_gpio_set_dir(unsigned int gpio_num, unsigned int dir)
{
    if(dir)	
        GPIO->GPIODir[GPIO_NUM_TO_ARRAY_IDX(gpio_num)] |= GPIO_NUM_TO_MASK(gpio_num);	
    else
        GPIO->GPIODir[GPIO_NUM_TO_ARRAY_IDX(gpio_num)] &= ~GPIO_NUM_TO_MASK(gpio_num);	
}
#ifndef _CFE_
EXPORT_SYMBOL(bcm_gpio_set_dir);
#endif

unsigned int bcm_gpio_get_data(unsigned int gpio_num)
{
    return (GPIO->GPIOio[GPIO_NUM_TO_ARRAY_IDX(gpio_num)] & (GPIO_NUM_TO_MASK(gpio_num))) >> GPIO_NUM_TO_ARRAY_SHIFT(gpio_num);
}
#ifndef _CFE_
EXPORT_SYMBOL(bcm_gpio_get_data);
#endif

void bcm_gpio_set_data(unsigned int gpio_num, unsigned int data)
{
    if (data)	
        GPIO->GPIOio[GPIO_NUM_TO_ARRAY_IDX(gpio_num)] |= GPIO_NUM_TO_MASK(gpio_num);	
    else
        GPIO->GPIOio[GPIO_NUM_TO_ARRAY_IDX(gpio_num)] &= ~GPIO_NUM_TO_MASK(gpio_num);
}
#ifndef _CFE_
EXPORT_SYMBOL(bcm_gpio_set_data);
#endif

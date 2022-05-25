/*
* <:copyright-BRCM:2016:DUAL/GPL:standard
* 
*    Copyright (c) 2016 Broadcom 
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


/***************************************************************************
* File Name  : bcm63xx_gpio.c
*
* Description: This file contains functions related to GPIO access.
*     See GPIO register defs in shared/broadcom/include/bcm963xx/xxx_common.h
*
*
***************************************************************************/

/*
 * Access to _SHARED_ GPIO registers should go through common functions
 * defined in board.h.  These common functions will use a spinlock with
 * irq's disabled to prevent concurrent access.
 * Functions which don't want to call the common gpio access functions must
 * acquire the bcm_gpio_spinlock with irq's disabled before accessing the
 * shared GPIO registers.
 * The GPIO registers that must be protected are:
 * GPIO->GPIODir
 * GPIO->GPIOio
 * GPIO->GPIOMode
 *
 * Note that many GPIO registers are dedicated to some driver or sub-system.
 * In those cases, the driver/sub-system can use its own locking scheme to
 * ensure serial access to its GPIO registers.
 *
 * DEFINE_SPINLOCK is the new, recommended way to declaring a spinlock.
 * See spinlock_types.h
 *
 * I am using a spin_lock_irqsave/spin_lock_irqrestore to lock and unlock
 * so that GPIO's can be accessed in interrupt context.
 */
#include <linux/version.h>
#include <linux/errno.h>
#include <linux/smp.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,0)
#include <linux/module.h>
#include <linux/cache.h>
#endif

#include "bcm_map_part.h"
#include "bcm_assert_locks.h"
#include "board.h"
#include "shared_utils.h"
#include "bcm_gpio.h"
#include "boardparms.h"


DEFINE_SPINLOCK(bcm_gpio_spinlock);
EXPORT_SYMBOL(bcm_gpio_spinlock);


void kerSysSetGpioStateLocked(unsigned short bpGpio, GPIO_STATE_t state)
{
    BCM_ASSERT_V(bpGpio != BP_NOT_DEFINED);
    BCM_ASSERT_V((bpGpio & BP_GPIO_NUM_MASK) < GPIO_NUM_MAX);
    BCM_ASSERT_HAS_SPINLOCK_V(&bcm_gpio_spinlock);

    kerSysSetGpioDirLocked(bpGpio);


    if((state == kGpioActive && !(bpGpio & BP_ACTIVE_LOW)) ||
        (state == kGpioInactive && (bpGpio & BP_ACTIVE_LOW))) {
        bcm_gpio_set_data((bpGpio & BP_GPIO_NUM_MASK), 1);
    }
    else {
        bcm_gpio_set_data((bpGpio & BP_GPIO_NUM_MASK), 0);
    }
}

void kerSysSetGpioState(unsigned short bpGpio, GPIO_STATE_t state)
{
#ifndef CONFIG_BRCM_QEMU
    unsigned long flags;

    spin_lock_irqsave(&bcm_gpio_spinlock, flags);
    kerSysSetGpioStateLocked(bpGpio, state);
    spin_unlock_irqrestore(&bcm_gpio_spinlock, flags);
#endif    
}



void kerSysSetGpioDirLocked(unsigned short bpGpio)
{
    BCM_ASSERT_V(bpGpio != BP_NOT_DEFINED);
    BCM_ASSERT_V((bpGpio & BP_GPIO_NUM_MASK) < GPIO_NUM_MAX);
    BCM_ASSERT_HAS_SPINLOCK_V(&bcm_gpio_spinlock);

    bcm_gpio_set_dir(bpGpio, 1);
}

void kerSysSetGpioDir(unsigned short bpGpio)
{
#ifndef CONFIG_BRCM_QEMU
    unsigned long flags;

    spin_lock_irqsave(&bcm_gpio_spinlock, flags);
    kerSysSetGpioDirLocked(bpGpio);
    spin_unlock_irqrestore(&bcm_gpio_spinlock, flags);
#endif    
}


/* Set gpio direction to input. Parameter gpio is in boardparms.h format. */
int kerSysSetGpioDirInput(unsigned short bpGpio)
{
    unsigned long flags;

    spin_lock_irqsave(&bcm_gpio_spinlock, flags);
  //  GPIO_TAKE_CONTROL(bpGpio & BP_GPIO_NUM_MASK);


    bcm_gpio_set_dir(bpGpio, 0);
    spin_unlock_irqrestore(&bcm_gpio_spinlock, flags);

    return(0);
}

/* Return a gpio's value, 0 or 1. Parameter gpio is in boardparms.h format. */
int kerSysGetGpioValue(unsigned short bpGpio)
{
    return bcm_gpio_get_data(bpGpio);
}

#if defined(CONFIG_I2C_GPIO) && !defined(CONFIG_GPIOLIB)
/* GPIO bit functions to support drivers/i2c/busses/i2c-gpio.c */

#define GPIO_TAKE_CONTROL(G) 

/* noop */
int gpio_request(unsigned bpGpio, const char *label)
{
    return(0); /* success */
}

/* noop */
void gpio_free(unsigned bpGpio)
{
}

/* Assign a gpio's value. Parameter gpio is in boardparms.h format. */
void gpio_set_value(unsigned bpGpio, int value)
{
    unsigned long flags;

    /* value should be either 0 or 1 */
    spin_lock_irqsave(&bcm_gpio_spinlock, flags);

    bcm_gpio_set_data(bpGpio, value);

    spin_unlock_irqrestore(&bcm_gpio_spinlock, flags);
}

/* Set gpio direction to input. Parameter gpio is in boardparms.h format. */
int gpio_direction_input(unsigned bpGpio)
{
    unsigned long flags;

    spin_lock_irqsave(&bcm_gpio_spinlock, flags);

    bcm_gpio_set_dir(bpGpio, 0);

    spin_unlock_irqrestore(&bcm_gpio_spinlock, flags);
    return(0);
}

/* Set gpio direction to output. Parameter gpio is in boardparms.h format. */
int gpio_direction_output(unsigned bpGpio, int value)
{
    unsigned long flags;

    spin_lock_irqsave(&bcm_gpio_spinlock, flags);

    bcm_gpio_set_dir(bpGpio, 1);

    spin_unlock_irqrestore(&bcm_gpio_spinlock, flags);

    gpio_set_value(bpGpio, value);

    return(0);
}

/* Return a gpio's value, 0 or 1. Parameter gpio is in boardparms.h format. */
int gpio_get_value(unsigned bpGpio)
{
#if defined(CONFIG_BCM947189)
    unsigned long flags;

    spin_lock_irqsave(&bcm_gpio_spinlock, flags);
    GPIO_TAKE_CONTROL(bpGpio & BP_GPIO_NUM_MASK);
    spin_unlock_irqrestore(&bcm_gpio_spinlock, flags);
#endif

    return(bcm_gpio_get_data(bpGpio));

}
#endif

EXPORT_SYMBOL(kerSysSetGpioState);
EXPORT_SYMBOL(kerSysSetGpioStateLocked);
EXPORT_SYMBOL(kerSysSetGpioDir);
EXPORT_SYMBOL(kerSysSetGpioDirLocked);
EXPORT_SYMBOL(kerSysSetGpioDirInput);
EXPORT_SYMBOL(kerSysGetGpioValue);


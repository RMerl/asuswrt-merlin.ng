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

/*
 *******************************************************************************
 * File Name  : i2c_bcmgpio.c
 *
 * Description: This file contains Broadcom i2c gpio device driver using Linux
 *              i2c Bit-Bang driver.
 *
 *******************************************************************************
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/i2c-gpio.h>
#include <linux/gpio.h>

#include <bcm_gpio.h>
#include <board.h>
#include <linux/platform_device.h>

/****************************************************************************
 * If CONFIG_I2C_GPIO is defined, the kernel I2C gpio driver,
 * kernel/linux/drivers/i2c/busses/i2c-gpio.c, is compiled into the image.
 * Register a device with it.
 *
 ****************************************************************************/

static int bcm_gpio_direction_input(struct gpio_chip *chip, unsigned gpio)
{
	kerSysSetGpioDirInput(gpio);
	return 0;
}

static int bcm_gpio_direction_output(struct gpio_chip *chip, unsigned gpio, int level)
{
	kerSysSetGpioDir(gpio);
	kerSysSetGpioState(gpio, level);
	return 0;
}

static int bcm_gpio_get_value(struct gpio_chip *chip, unsigned gpio)
{
	return kerSysGetGpioValue(gpio);
}

static void bcm_gpio_set_value(struct gpio_chip *chip, unsigned gpio, int value)
{
	kerSysSetGpioState(gpio, value);
}

static struct gpio_chip bcm_gpio_chip = {
	.label            = "BCM_GPIO_CHIP",
	.direction_input  = bcm_gpio_direction_input,
	.direction_output = bcm_gpio_direction_output,
	.get              = bcm_gpio_get_value,
	.set              = bcm_gpio_set_value,
	.base             = 0,
	.ngpio            = 68,
};


static struct i2c_gpio_platform_data bcm_i2c_gpio_data = {
	.sda_pin = 0,
	.scl_pin = 0,
};

struct platform_device bcm_device_i2c_gpio = {
    .name   = "i2c-gpio",
    .id     = 0,
    .dev    = {
        .platform_data = &bcm_i2c_gpio_data,
    },
};

static struct platform_device *bcm_i2c_devices[] __initdata = {
    &bcm_device_i2c_gpio,
};


static int __init i2c_bcm_gpio_init(void)
{
    unsigned short bpGpio_scl, bpGpio_sda;
    int ret;

    BpGetBitbangSclGpio(&bpGpio_scl);
    BpGetBitbangSdaGpio(&bpGpio_sda);

    if (bpGpio_scl != BP_NOT_DEFINED &&
        bpGpio_sda != BP_NOT_DEFINED ) {

        /* Note : The scl/sda pins for BCM49408REF board are reversed
           because the temperature sensor is wired backwards. This needs
           to be addressed in the boardparams for this board */
        bcm_i2c_gpio_data.scl_pin = bpGpio_scl & BP_GPIO_NUM_MASK;
        bcm_i2c_gpio_data.sda_pin = bpGpio_sda & BP_GPIO_NUM_MASK;

        printk("i2c_bcm_gpio_init: SDA = %d and SCL =%d GPIOs\n",
                bcm_i2c_gpio_data.sda_pin, bcm_i2c_gpio_data.scl_pin);

        ret = gpiochip_add(&bcm_gpio_chip);
        if (ret)
            printk("%s: gpiochip_add failed ret=%d\n", __FUNCTION__, ret);
        ret = platform_add_devices(bcm_i2c_devices, ARRAY_SIZE(bcm_i2c_devices));
        if (ret)
            printk("%s: platform_add_device failed ret=%d\n", __FUNCTION__, ret);
    }
    else
    {
        printk("i2c_bcm_gpio_init: GPIO pins undefined\n");
    }

	return 0; /* successful */
}
arch_initcall(i2c_bcm_gpio_init);

static void __exit i2c_bcm_gpio_exit(void)
{
}
module_exit(i2c_bcm_gpio_exit);

MODULE_DESCRIPTION("Broadcom I2C GPIO driver");
MODULE_LICENSE("GPL");

/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __JZ4780_GPIO_H__
#define __JZ4780_GPIO_H__

#define JZ_GPIO(bank, pin) ((32 * (bank)) + (pin))

int jz47xx_gpio_get_value(unsigned int gpio);
void jz47xx_gpio_direction_input(unsigned int gpio);
void jz47xx_gpio_direction_output(unsigned int gpio, int value);

#endif

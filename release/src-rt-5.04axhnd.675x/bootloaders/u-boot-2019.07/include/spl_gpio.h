/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Simple GPIO access from SPL. This only supports a single GPIO space,
 * typically the SoC GPIO banks.
 *
 * Copyright 2018 Google LLC
 */

#ifndef __SPL_GPIO_H
#define __SPL_GPIO_H

#include <asm/gpio.h>

/*
 * The functions listed here should be implemented in the SoC GPIO driver.
 * They correspond to the normal GPIO API (asm-generic/gpio.h). The GPIO
 * number is encoded in an unsigned int by an SoC-specific means. Pull
 * values are also SoC-specific.
 *
 * This API should only be used in TPL/SPL where GPIO access is needed but
 * driver model is not available (yet) or adds too much overhead.
 *
 * The caller must supply the GPIO register base since this information is
 * often specific to a particular SoC generation. This allows the GPIO
 * code to be fairly generic.
 *
 * Only a single implementation of each of these functions can be provided.
 *
 * The 'gpio' value can include both a bank and a GPIO number, if desired. The
 * encoding is SoC-specific.
 */

/**
 * spl_gpio_set_pull() - Set the pull up/down state of a GPIO
 *
 * @regs: Pointer to GPIO registers
 * @gpio: GPIO to adjust (SoC-specific)
 * @pull: Pull value (SoC-specific)
 * @return return 0 if OK, -ve on error
 */
int spl_gpio_set_pull(void *regs, uint gpio, int pull);

/**
 * spl_gpio_output() - Set a GPIO as an output
 *
 * @regs: Pointer to GPIO registers
 * @gpio: GPIO to adjust (SoC-specific)
 * @value: 0 to set the output low, 1 to set it high
 * @return return 0 if OK, -ve on error
 */
int spl_gpio_output(void *regs, uint gpio, int value);

/**
 * spl_gpio_input() - Set a GPIO as an input
 *
 * @regs: Pointer to GPIO registers
 * @gpio: GPIO to adjust (SoC-specific)
 * @return return 0 if OK, -ve on error
 */
int spl_gpio_input(void *regs, uint gpio);

#endif /* __SPL_GPIO_H */

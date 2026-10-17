/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2020 Sean Anderson <seanga2@gmail.com>
 */

#ifndef SANDBOX_PINMUX_H
#define SANDBOX_PINMUX_H

#define SANDBOX_PINMUX_UART 0
#define SANDBOX_PINMUX_I2C  1
#define SANDBOX_PINMUX_SPI  2
#define SANDBOX_PINMUX_I2S  3
#define SANDBOX_PINMUX_GPIO 4
#define SANDBOX_PINMUX_CS   5
#define SANDBOX_PINMUX_PWM  6

#define SANDBOX_PINMUX(pin, func) ((func) << 16 | (pin))

#endif /* SANDBOX_PINMUX_H */

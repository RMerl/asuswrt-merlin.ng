/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * LPC32xx GPIO interface macro for pin mapping.
 *
 * (C) Copyright 2015  DENX Software Engineering GmbH
 * Written-by: Sylvain Lemieux <slemieux@@tycoint.com>
 */

#ifndef _LPC32XX_GPIO_GRP_H
#define _LPC32XX_GPIO_GRP_H

/*
 * Macro to map the pin for the lpc32xx_gpio driver.
 * Note: - GPIOS are considered here as homogeneous and linear from 0 to 159;
 *         mapping is done per register, as group of 32.
 *         (see drivers/gpio/lpc32xx_gpio.c for details).
 *       - macros can be use with the following pins:
 *         P0.0 - P0.7
 *         P1.0 - P1.23
 *         P2.0 - P2.12
 *         P3 GPI_0 - GPI_9 / GPI_15 - GPI_23 / GPI_25 / GPI_27 - GPI_28
 *         P3 GPO_0 - GPO_23
 *         P3 GPIO_0 - GPIO_5 (output register only)
 */
#define LPC32XX_GPIO_P0_GRP 0
#define LPC32XX_GPIO_P1_GRP 32
#define LPC32XX_GPIO_P2_GRP 64
#define LPC32XX_GPO_P3_GRP  96
#define LPC32XX_GPIO_P3_GRP (LPC32XX_GPO_P3_GRP + 25)
#define LPC32XX_GPI_P3_GRP  128

/*
 * A specific GPIO can be selected with this macro
 * ie, GPIO P0.1 can be selected with LPC32XX_GPIO(LPC32XX_GPIO_P0_GRP, 1)
 * See the LPC32x0 User's guide for GPIO group numbers
 */
#define LPC32XX_GPIO(x, y) ((x) + (y))

#endif /* _LPC32XX_GPIO_GRP_H */

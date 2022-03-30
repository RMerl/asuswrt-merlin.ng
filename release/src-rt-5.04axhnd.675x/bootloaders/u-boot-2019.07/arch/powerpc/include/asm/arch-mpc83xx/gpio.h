/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _MPC83XX_GPIO_H_
#define _MPC83XX_GPIO_H_

/*
 * The MCP83xx's 1-2 GPIO controllers each with 32 bits.
 */
#if defined(CONFIG_ARCH_MPC8313) || defined(CONFIG_ARCH_MPC8308) || \
	defined(CONFIG_ARCH_MPC8315)
#define MPC83XX_GPIO_CTRLRS 1
#elif defined(CONFIG_ARCH_MPC834X) || defined(CONFIG_ARCH_MPC837X)
#define MPC83XX_GPIO_CTRLRS 2
#else
#define MPC83XX_GPIO_CTRLRS 0
#endif

#define MAX_NUM_GPIOS (32 * MPC83XX_GPIO_CTRLRS)

struct mpc8xxx_gpio_plat {
       ulong addr;
       unsigned long size;
       uint ngpios;
};

#ifndef DM_GPIO
void mpc83xx_gpio_init_f(void);
void mpc83xx_gpio_init_r(void);
#endif	/* DM_GPIO */

#endif	/* MPC83XX_GPIO_H_ */

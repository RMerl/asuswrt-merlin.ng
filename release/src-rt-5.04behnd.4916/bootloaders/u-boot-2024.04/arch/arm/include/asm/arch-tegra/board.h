/* SPDX-License-Identifier: GPL-2.0+ */
/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 */

#ifndef _TEGRA_BOARD_H_
#define _TEGRA_BOARD_H_

/* Set up pinmux to make UART usable */
void gpio_early_init_uart(void);

/* Set up early UART output */
void board_init_uart_f(void);

/* Set up any early GPIOs the board might need for proper operation */
void gpio_early_init(void);  /* overrideable GPIO config        */

/*
 * Hooks to allow boards to set up the pinmux for a specific function.
 * Has to be implemented in the board files as we don't yet support pinmux
 * setup from FDT. If a board file does not implement one of those functions
 * an empty stub function will be called.
 */

void pinmux_init(void);      /* overridable general pinmux setup */
void pin_mux_usb(void);      /* overridable USB pinmux setup     */
void pin_mux_spi(void);      /* overridable SPI pinmux setup     */
void pin_mux_nand(void);     /* overridable NAND pinmux setup    */
void pin_mux_mmc(void);      /* overridable mmc pinmux setup     */
void pin_mux_display(void);  /* overridable DISPLAY pinmux setup */

/*
 * Helpers for various standard DT update mechanisms.
 */

#if defined(CONFIG_ARM64)
void ft_mac_address_setup(void *fdt);
void ft_carveout_setup(void *fdt, const char *const *nodes,
		       unsigned int count);
#endif

#endif

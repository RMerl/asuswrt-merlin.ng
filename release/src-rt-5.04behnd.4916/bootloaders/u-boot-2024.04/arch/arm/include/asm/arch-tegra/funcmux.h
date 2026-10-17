/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2010-2012, NVIDIA CORPORATION.  All rights reserved.
 */

/* Tegra high-level function multiplexing */

#ifndef _TEGRA_FUNCMUX_H_
#define _TEGRA_FUNCMUX_H_

/**
 * Select a config for a particular peripheral.
 *
 * Each peripheral can operate through a number of configurations,
 * which are sets of pins that it uses to bring out its signals.
 * The basic config is 0, and higher numbers indicate different
 * pinmux settings to bring the peripheral out on other pins,
 *
 * This function also disables tristate for the function's pins,
 * so that they operate in normal mode.
 *
 * @param id		Peripheral id
 * @param config	Configuration to use (FUNCMUX_...), 0 for default
 * Return: 0 if ok, -1 on error (e.g. incorrect id or config)
 */
int funcmux_select(enum periph_id id, int config);

#endif	/* _TEGRA_FUNCMUX_H_ */

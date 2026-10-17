// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013
 * NVIDIA Corporation <www.nvidia.com>
 */

/* Tegra124 high-level function multiplexing */

#include <common.h>
#include <log.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>

int funcmux_select(enum periph_id id, int config)
{
	int bad_config = config != FUNCMUX_DEFAULT;

	switch (id) {
	case PERIPH_ID_UART4:
		switch (config) {
		case FUNCMUX_UART4_GPIO: /* TXD,RXD,CTS,RTS */
			pinmux_set_func(PMUX_PINGRP_PJ7, PMUX_FUNC_UARTD);
			pinmux_set_func(PMUX_PINGRP_PB0, PMUX_FUNC_UARTD);
			pinmux_set_func(PMUX_PINGRP_PB1, PMUX_FUNC_UARTD);
			pinmux_set_func(PMUX_PINGRP_PK7, PMUX_FUNC_UARTD);

			pinmux_set_io(PMUX_PINGRP_PJ7, PMUX_PIN_OUTPUT);
			pinmux_set_io(PMUX_PINGRP_PB0, PMUX_PIN_INPUT);
			pinmux_set_io(PMUX_PINGRP_PB1, PMUX_PIN_INPUT);
			pinmux_set_io(PMUX_PINGRP_PK7, PMUX_PIN_OUTPUT);

			pinmux_tristate_disable(PMUX_PINGRP_PJ7);
			pinmux_tristate_disable(PMUX_PINGRP_PB0);
			pinmux_tristate_disable(PMUX_PINGRP_PB1);
			pinmux_tristate_disable(PMUX_PINGRP_PK7);
			break;
		}
		break;

	case PERIPH_ID_UART1:
		switch (config) {
		case FUNCMUX_UART1_KBC:
			pinmux_set_func(PMUX_PINGRP_KB_ROW9_PS1,
					PMUX_FUNC_UARTA);
			pinmux_set_func(PMUX_PINGRP_KB_ROW10_PS2,
					PMUX_FUNC_UARTA);

			pinmux_set_io(PMUX_PINGRP_KB_ROW9_PS1, PMUX_PIN_OUTPUT);
			pinmux_set_io(PMUX_PINGRP_KB_ROW10_PS2, PMUX_PIN_INPUT);

			pinmux_tristate_disable(PMUX_PINGRP_KB_ROW9_PS1);
			pinmux_tristate_disable(PMUX_PINGRP_KB_ROW10_PS2);
			break;
		}
		break;

	/* Add other periph IDs here as needed */

	default:
		debug("%s: invalid periph_id %d", __func__, id);
		return -1;
	}

	if (bad_config) {
		debug("%s: invalid config %d for periph_id %d", __func__,
		      config, id);
		return -1;
	}
	return 0;
}

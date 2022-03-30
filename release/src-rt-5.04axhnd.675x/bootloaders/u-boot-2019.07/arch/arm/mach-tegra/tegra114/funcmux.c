// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2010-2013, NVIDIA CORPORATION.  All rights reserved.
 */

/* Tegra114 high-level function multiplexing */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/funcmux.h>
#include <asm/arch/pinmux.h>

int funcmux_select(enum periph_id id, int config)
{
	int bad_config = config != FUNCMUX_DEFAULT;

	switch (id) {
	case PERIPH_ID_UART4:
		switch (config) {
		case FUNCMUX_UART4_GMI:
			pinmux_set_func(PMUX_PINGRP_GMI_A16_PJ7,
					PMUX_FUNC_UARTD);
			pinmux_set_func(PMUX_PINGRP_GMI_A17_PB0,
					PMUX_FUNC_UARTD);
			pinmux_set_func(PMUX_PINGRP_GMI_A18_PB1,
					PMUX_FUNC_UARTD);
			pinmux_set_func(PMUX_PINGRP_GMI_A19_PK7,
					PMUX_FUNC_UARTD);

			pinmux_set_io(PMUX_PINGRP_GMI_A16_PJ7, PMUX_PIN_OUTPUT);
			pinmux_set_io(PMUX_PINGRP_GMI_A17_PB0, PMUX_PIN_INPUT);
			pinmux_set_io(PMUX_PINGRP_GMI_A18_PB1, PMUX_PIN_INPUT);
			pinmux_set_io(PMUX_PINGRP_GMI_A19_PK7, PMUX_PIN_OUTPUT);

			pinmux_tristate_disable(PMUX_PINGRP_GMI_A16_PJ7);
			pinmux_tristate_disable(PMUX_PINGRP_GMI_A17_PB0);
			pinmux_tristate_disable(PMUX_PINGRP_GMI_A18_PB1);
			pinmux_tristate_disable(PMUX_PINGRP_GMI_A19_PK7);
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

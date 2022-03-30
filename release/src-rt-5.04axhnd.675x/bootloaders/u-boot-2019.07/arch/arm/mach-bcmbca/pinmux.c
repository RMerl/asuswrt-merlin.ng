/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019 Broadcom Ltd.
 */

#include <common.h>
#include "pinctrl.h"

/* this function is intended to be shared in SPL / TPL / UBOOT */
int bcmbca_pinmux_set (volatile struct pinctrl_reg *regp, int pin, int func)
{
	uint32_t data = 0;

	data |= pin;
	data |= (func << PINMUX_DATA_SHIFT);

	debug("pinmux %d func %d\n", pin, func);

	regp->TestPortBlockDataMSB = 0;
	regp->TestPortBlockDataLSB = data;
	regp->TestPortCmd = LOAD_MUX_REG_CMD;

	return 0;
}


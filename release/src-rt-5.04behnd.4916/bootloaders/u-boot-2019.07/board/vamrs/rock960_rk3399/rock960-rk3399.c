// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Manivannan Sadhasivam <manivannan.sadhasivam@linaro.org>
 */

#include <common.h>
#include <dm.h>
#include <power/regulator.h>

int board_init(void)
{
	int ret;

	ret = regulators_enable_boot_on(false);
	if (ret)
		debug("%s: Cannot enable boot on regulator\n", __func__);

	return 0;
}

// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include "mv_ddr_common.h"
#include "ddr_ml_wrapper.h"

void mv_ddr_ver_print(void)
{
	printf("%s %s\n", mv_ddr_version_string, mv_ddr_build_message);
}

/* ceiling division for positive integers */
unsigned int ceil_div(unsigned int x, unsigned int y)
{
	return (x % y) ? (x / y + 1) : (x / y);
}

/*
 * time to number of clocks calculation based on the rounding algorithm
 * using 97.4% inverse factor per JEDEC Standard No. 21-C, 4.1.2.L-4:
 * Serial Presence Detect (SPD) for DDR4 SDRAM Modules
 */
unsigned int time_to_nclk(unsigned int t, unsigned int tclk)
{
	/* t & tclk parameters are in ps */
	return ((unsigned long)t * 1000 / tclk + 974) / 1000;
}

/* round division of two positive integers to the nearest whole number */
int round_div(unsigned int dividend, unsigned int divisor, unsigned int *quotient)
{
	if (quotient == NULL) {
		printf("%s: error: NULL quotient pointer found\n", __func__);
		return MV_FAIL;
	}

	if (divisor == 0) {
		printf("%s: error: division by zero\n", __func__);
		return MV_FAIL;
	} else {
		*quotient = (dividend + divisor / 2) / divisor;
	}

	return MV_OK;
}

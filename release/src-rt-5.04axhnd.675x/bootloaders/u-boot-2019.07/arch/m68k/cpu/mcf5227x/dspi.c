// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019
 * Angelo Dureghello <angleo@sysam.it>
 *
 * CPU specific dspi routines
 */

#include <common.h>
#include <asm/immap.h>
#include <asm/io.h>

#ifdef CONFIG_CF_DSPI
void dspi_chip_select(int cs)
{
	struct gpio *gpio = (struct gpio *)MMAP_GPIO;

	switch (cs) {
	case 0:
		clrbits_8(&gpio->par_dspi, GPIO_PAR_DSPI_PCS0_UNMASK);
		setbits_8(&gpio->par_dspi, GPIO_PAR_DSPI_PCS0_PCS0);
		break;
	case 2:
		clrbits_8(&gpio->par_timer, ~GPIO_PAR_TIMER_T2IN_UNMASK);
		setbits_8(&gpio->par_timer, GPIO_PAR_TIMER_T2IN_DSPIPCS2);
		break;
	}
}

void dspi_chip_unselect(int cs)
{
	struct gpio *gpio = (struct gpio *)MMAP_GPIO;

	switch (cs) {
	case 0:
		clrbits_8(&gpio->par_dspi, GPIO_PAR_DSPI_PCS0_PCS0);
		break;
	case 2:
		clrbits_8(&gpio->par_timer, ~GPIO_PAR_TIMER_T2IN_UNMASK);
		break;
	}
}
#endif /* CONFIG_CF_DSPI */

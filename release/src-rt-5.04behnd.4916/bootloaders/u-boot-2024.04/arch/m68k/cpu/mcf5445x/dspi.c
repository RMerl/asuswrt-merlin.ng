// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2019
 * Angelo Dureghello <angleo@sysam.it>
 *
 * CPU specific dspi routines
 */

#include <asm/immap.h>
#include <asm/io.h>

#ifdef CONFIG_CF_DSPI
void dspi_chip_select(int cs)
{
	struct gpio *gpio = (struct gpio *)MMAP_GPIO;

#ifdef CONFIG_MCF5441x
	switch (cs) {
	case 0:
		clrbits_8(&gpio->par_dspi0,
			  ~GPIO_PAR_DSPI0_PCS0_MASK);
		setbits_8(&gpio->par_dspi0,
			  GPIO_PAR_DSPI0_PCS0_DSPI0PCS0);
		break;
	case 1:
		clrbits_8(&gpio->par_dspiow,
			  GPIO_PAR_DSPIOW_DSPI0PSC1);
		setbits_8(&gpio->par_dspiow,
			  GPIO_PAR_DSPIOW_DSPI0PSC1);
		break;
	}
#endif
}

void dspi_chip_unselect(int cs)
{
	struct gpio *gpio = (struct gpio *)MMAP_GPIO;

#ifdef CONFIG_MCF5441x
	if (cs == 1)
		clrbits_8(&gpio->par_dspiow, GPIO_PAR_DSPIOW_DSPI0PSC1);
#endif
}
#endif /* CONFIG_CF_DSPI */

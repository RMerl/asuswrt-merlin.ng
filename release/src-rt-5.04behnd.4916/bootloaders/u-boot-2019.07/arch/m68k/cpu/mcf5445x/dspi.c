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

#ifdef CONFIG_MCF5445x
	switch (cs) {
	case 0:
		clrbits_8(&gpio->par_dspi, GPIO_PAR_DSPI_PCS0_PCS0);
		setbits_8(&gpio->par_dspi, GPIO_PAR_DSPI_PCS0_PCS0);
		break;
	case 1:
		clrbits_8(&gpio->par_dspi, GPIO_PAR_DSPI_PCS1_PCS1);
		setbits_8(&gpio->par_dspi, GPIO_PAR_DSPI_PCS1_PCS1);
		break;
	case 2:
		clrbits_8(&gpio->par_dspi, GPIO_PAR_DSPI_PCS2_PCS2);
		setbits_8(&gpio->par_dspi, GPIO_PAR_DSPI_PCS2_PCS2);
		break;
	case 3:
		clrbits_8(&gpio->par_dma, ~GPIO_PAR_DMA_DACK0_UNMASK);
		setbits_8(&gpio->par_dma, GPIO_PAR_DMA_DACK0_PCS3);
		break;
	case 5:
		clrbits_8(&gpio->par_dspi, GPIO_PAR_DSPI_PCS5_PCS5);
		setbits_8(&gpio->par_dspi, GPIO_PAR_DSPI_PCS5_PCS5);
		break;
	}
#endif
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

#ifdef CONFIG_MCF5445x
	switch (cs) {
	case 0:
		clrbits_8(&gpio->par_dspi, GPIO_PAR_DSPI_PCS0_PCS0);
		break;
	case 1:
		clrbits_8(&gpio->par_dspi, GPIO_PAR_DSPI_PCS1_PCS1);
		break;
	case 2:
		clrbits_8(&gpio->par_dspi, GPIO_PAR_DSPI_PCS2_PCS2);
		break;
	case 3:
		clrbits_8(&gpio->par_dma, ~GPIO_PAR_DMA_DACK0_UNMASK);
		break;
	case 5:
		clrbits_8(&gpio->par_dspi, GPIO_PAR_DSPI_PCS5_PCS5);
		break;
	}
#endif
#ifdef CONFIG_MCF5441x
	if (cs == 1)
		clrbits_8(&gpio->par_dspiow, GPIO_PAR_DSPIOW_DSPI0PSC1);
#endif
}
#endif /* CONFIG_CF_DSPI */

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008 Extreme Engineering Solutions, Inc.
 */

#include <common.h>
#include <asm/mmu.h>
#ifdef CONFIG_PCA953X
#include <pca953x.h>

/*
 * Determine if a board's flashes are write protected
 */
int board_flash_wp_on(void)
{
	if (pca953x_get_val(CONFIG_SYS_I2C_PCA953X_ADDR0) &
			CONFIG_SYS_PCA953X_NVM_WP)
		return 1;

	return 0;
}
#endif

/*
 * Return a board's derivative model number.  For example:
 * return 2 for the XPedite5372 and return 1 for the XPedite5201.
 */
uint get_board_derivative(void)
{
#if defined(CONFIG_MPC85xx)
       volatile ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
#elif defined(CONFIG_MPC86xx)
       volatile immap_t *immap = (immap_t *)CONFIG_SYS_CCSRBAR;
       volatile ccsr_gur_t *gur = &immap->im_gur;
#endif

       /*
	* The top 4 lines of the local bus address are pulled low/high and
	* can be read to determine the least significant digit of a board's
	* model number.
	*/
       return gur->gpporcr >> 28;
}

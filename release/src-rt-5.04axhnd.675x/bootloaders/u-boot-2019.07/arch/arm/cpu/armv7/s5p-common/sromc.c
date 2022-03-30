// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2010 Samsung Electronics
 * Naveen Krishna Ch <ch.naveen@samsung.com>
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/sromc.h>

/*
 * s5p_config_sromc() - select the proper SROMC Bank and configure the
 * band width control and bank control registers
 * srom_bank	- SROM
 * srom_bw_conf  - SMC Band witdh reg configuration value
 * srom_bc_conf  - SMC Bank Control reg configuration value
 */
void s5p_config_sromc(u32 srom_bank, u32 srom_bw_conf, u32 srom_bc_conf)
{
	u32 tmp;
	struct s5p_sromc *srom =
		(struct s5p_sromc *)samsung_get_base_sromc();

	/* Configure SMC_BW register to handle proper SROMC bank */
	tmp = srom->bw;
	tmp &= ~(0xF << (srom_bank * 4));
	tmp |= srom_bw_conf;
	srom->bw = tmp;

	/* Configure SMC_BC register */
	srom->bc[srom_bank] = srom_bc_conf;
}

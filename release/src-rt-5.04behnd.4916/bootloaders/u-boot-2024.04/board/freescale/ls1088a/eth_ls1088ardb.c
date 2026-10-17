// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017 NXP
 */

#include <fsl-mc/fsl_mc.h>

#if defined(CONFIG_RESET_PHY_R)
void reset_phy(void)
{
	mc_env_boot();
}
#endif /* CONFIG_RESET_PHY_R */

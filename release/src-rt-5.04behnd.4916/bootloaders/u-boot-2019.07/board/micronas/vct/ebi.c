// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
 *
 * Copyright (C) 2006 Micronas GmbH
 */

#include <common.h>
#include <asm/io.h>
#include "vct.h"

int ebi_initialize(void)
{
#if defined(CONFIG_VCT_NOR)
	if (ebi_init_nor_flash())
		return -1;
#endif

#if defined(CONFIG_VCT_ONENAND)
	if (ebi_init_onenand())
		return -1;
#endif

#if defined(CONFIG_DRIVER_SMC911X)
	if (ebi_init_smc911x())
		return -1;
#endif

	reg_write(EBI_CTRL_SIG_ACTLV(EBI_BASE), 0x00004100);

	ebi_wait();

	return 0;
}

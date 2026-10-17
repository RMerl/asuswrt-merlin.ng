/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright Altera Corporation (C) 2014-2015
 */
#ifndef	_SDRAM_H_
#define	_SDRAM_H_

#ifndef __ASSEMBLY__

#if defined(CONFIG_TARGET_SOCFPGA_GEN5)
#include <asm/arch/sdram_gen5.h>
#elif defined(CONFIG_TARGET_SOCFPGA_ARRIA10)
#include <asm/arch/sdram_arria10.h>
#endif

#endif
#endif /* _SDRAM_H_ */

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2020 Jernej Skrabec <jernej.skrabec@siol.net>
 *
 * Sunxi platform prcm register definition.
 */

#ifndef _SUNXI_PRCM_H
#define _SUNXI_PRCM_H

/* prcm regs definition */
#if defined(CONFIG_SUN50I_GEN_H6) || defined(CONFIG_SUNXI_GEN_NCAT2)
#include <asm/arch/prcm_sun50i.h>
#else
#include <asm/arch/prcm_sun6i.h>
#endif

#endif /* _SUNXI_PRCM_H */

// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2023 Andes Technology Corporation
 */

#ifndef _ASM_ANDES_CSR_H
#define _ASM_ANDES_CSR_H

#include <asm/asm.h>
#include <linux/bitops.h>
#include <linux/const.h>

#define CSR_MCACHE_CTL 0x7ca
#define CSR_MMISC_CTL 0x7d0
#define CSR_MCCTLCOMMAND 0x7cc

/* mcache_ctl register */

#define MCACHE_CTL_IC_EN		BIT(0)
#define MCACHE_CTL_DC_EN		BIT(1)
#define MCACHE_CTL_IC_ECCEN		BIT(3)
#define MCACHE_CTL_DC_ECCEN		BIT(5)
#define MCACHE_CTL_CCTL_SUEN		BIT(8)
#define MCACHE_CTL_IC_PREFETCH_EN	BIT(9)
#define MCACHE_CTL_DC_PREFETCH_EN	BIT(10)
#define MCACHE_CTL_DC_WAROUND_EN	BIT(13)
#define MCACHE_CTL_L2C_WAROUND_EN	BIT(15)
#define MCACHE_CTL_TLB_ECCEN		BIT(18)
#define MCACHE_CTL_DC_COHEN		BIT(19)
#define MCACHE_CTL_DC_COHSTA		BIT(20)

/* mmisc_ctl register */
#define MMISC_CTL_NON_BLOCKING_EN	BIT(8)

#define CCTL_L1D_WBINVAL_ALL 6

#endif /* _ASM_ANDES_CSR_H */

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2013 - 2015  Freescale Semiconductor, Inc.
 */

#ifndef _FSL_ERRATA_H
#define _FSL_ERRATA_H

#include <common.h>
#if defined(CONFIG_PPC)
#include <asm/processor.h>
#elif defined(CONFIG_ARCH_LS1021A)
#include <asm/arch-ls102xa/immap_ls102xa.h>
#elif defined(CONFIG_FSL_LAYERSCAPE)
#include <asm/arch/soc.h>
#endif


#ifdef CONFIG_SYS_FSL_ERRATUM_A006379
static inline bool has_erratum_a006379(void)
{
	u32 svr = get_svr();
	if (((SVR_SOC_VER(svr) == SVR_T4240) && SVR_MAJ(svr) <= 1) ||
	    ((SVR_SOC_VER(svr) == SVR_T4160) && SVR_MAJ(svr) <= 1) ||
	    ((SVR_SOC_VER(svr) == SVR_T4080) && SVR_MAJ(svr) <= 1) ||
	    ((SVR_SOC_VER(svr) == SVR_B4860) && SVR_MAJ(svr) <= 2) ||
	    ((SVR_SOC_VER(svr) == SVR_B4420) && SVR_MAJ(svr) <= 2) ||
	    ((SVR_SOC_VER(svr) == SVR_T2080) && SVR_MAJ(svr) <= 1) ||
	    ((SVR_SOC_VER(svr) == SVR_T2081) && SVR_MAJ(svr) <= 1))
		return true;

	return false;
}
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_A007186
static inline bool has_erratum_a007186(void)
{
	u32 svr = get_svr();
	u32 soc = SVR_SOC_VER(svr);

	switch (soc) {
	case SVR_T4240:
		return IS_SVR_REV(svr, 2, 0);
	case SVR_T4160:
		return IS_SVR_REV(svr, 2, 0);
	case SVR_B4860:
		return IS_SVR_REV(svr, 2, 0);
	case SVR_B4420:
		return IS_SVR_REV(svr, 2, 0);
	case SVR_T2081:
	case SVR_T2080:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 1, 1);
	}

	return false;
}
#endif

#ifdef CONFIG_SYS_FSL_ERRATUM_A008378
static inline bool has_erratum_a008378(void)
{
	u32 svr = get_svr();
	u32 soc = SVR_SOC_VER(svr);


	switch (soc) {
#ifdef CONFIG_ARCH_LS1021A
	case SOC_VER_LS1020:
	case SOC_VER_LS1021:
	case SOC_VER_LS1022:
	case SOC_VER_SLS1020:
		return IS_SVR_REV(svr, 1, 0);
#endif
#ifdef CONFIG_PPC
	case SVR_T1023:
	case SVR_T1024:
		return IS_SVR_REV(svr, 1, 0);
	case SVR_T1020:
	case SVR_T1022:
	case SVR_T1040:
	case SVR_T1042:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 1, 1);
#endif
	default:
		return false;
	}
}
#endif

#endif /*  _FSL_ERRATA_H */

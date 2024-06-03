// SPDX-License-Identifier: GPL-2.0+
/*
 * Freescale USB Controller
 *
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <hwconfig.h>
#include <fsl_errata.h>
#include<fsl_usb.h>
#if defined(CONFIG_FSL_LSCH2) || defined(CONFIG_FSL_LSCH3) || \
	defined(CONFIG_ARM)
#include <asm/arch/clock.h>
#endif

/* USB Erratum Checking code */
#if defined(CONFIG_PPC) || defined(CONFIG_ARM)
bool has_dual_phy(void)
{
	u32 svr = get_svr();
	u32 soc = SVR_SOC_VER(svr);

	switch (soc) {
#ifdef CONFIG_PPC
	case SVR_T1023:
	case SVR_T1024:
	case SVR_T1013:
	case SVR_T1014:
		return IS_SVR_REV(svr, 1, 0);
	case SVR_T1040:
	case SVR_T1042:
	case SVR_T1020:
	case SVR_T1022:
	case SVR_T2080:
	case SVR_T2081:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 1, 1);
	case SVR_T4240:
	case SVR_T4160:
	case SVR_T4080:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0);
#endif
	}

	return false;
}

bool has_erratum_a005275(void)
{
	u32 svr = get_svr();
	u32 soc = SVR_SOC_VER(svr);

	if (hwconfig("no_erratum_a005275"))
		return false;

	switch (soc) {
#ifdef CONFIG_PPC
	case SVR_P3041:
	case SVR_P2041:
	case SVR_P2040:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 1, 1);
	case SVR_P5010:
	case SVR_P5020:
	case SVR_P5021:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0);
	case SVR_P5040:
	case SVR_P1010:
		return IS_SVR_REV(svr, 1, 0);
#endif
	}

	return false;
}

bool has_erratum_a006261(void)
{
	u32 svr = get_svr();
	u32 soc = SVR_SOC_VER(svr);

	switch (soc) {
#ifdef CONFIG_PPC
	case SVR_P1010:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0);
	case SVR_P2041:
	case SVR_P2040:
		return IS_SVR_REV(svr, 1, 0) ||
			IS_SVR_REV(svr, 1, 1) ||
			IS_SVR_REV(svr, 2, 0) || IS_SVR_REV(svr, 2, 1);
	case SVR_P3041:
		return IS_SVR_REV(svr, 1, 0) ||
			IS_SVR_REV(svr, 1, 1) ||
			IS_SVR_REV(svr, 2, 0) || IS_SVR_REV(svr, 2, 1);
	case SVR_P5010:
	case SVR_P5020:
	case SVR_P5021:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0);
	case SVR_T4240:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0);
	case SVR_P5040:
		return IS_SVR_REV(svr, 1, 0) ||
			IS_SVR_REV(svr, 2, 0) || IS_SVR_REV(svr, 2, 1);
#endif
	}

	return false;
}

bool has_erratum_a007075(void)
{
	u32 svr = get_svr();
	u32 soc = SVR_SOC_VER(svr);

	switch (soc) {
#ifdef CONFIG_PPC
	case SVR_B4860:
	case SVR_B4420:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0);
	case SVR_P1010:
		return IS_SVR_REV(svr, 1, 0);
	case SVR_P4080:
		return IS_SVR_REV(svr, 2, 0) || IS_SVR_REV(svr, 3, 0);
#endif
	}
	return false;
}

bool has_erratum_a007798(void)
{
#ifdef CONFIG_PPC
	return SVR_SOC_VER(get_svr()) == SVR_T4240 &&
		IS_SVR_REV(get_svr(), 2, 0);
#endif
	return false;
}

bool has_erratum_a007792(void)
{
	u32 svr = get_svr();
	u32 soc = SVR_SOC_VER(svr);

	switch (soc) {
#ifdef CONFIG_PPC
	case SVR_T4240:
	case SVR_T4160:
	case SVR_T4080:
		return IS_SVR_REV(svr, 2, 0);
	case SVR_T1024:
	case SVR_T1023:
		return IS_SVR_REV(svr, 1, 0);
	case SVR_T1040:
	case SVR_T1042:
	case SVR_T1020:
	case SVR_T1022:
	case SVR_T2080:
	case SVR_T2081:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 1, 1);
#endif
	}
	return false;
}

bool has_erratum_a005697(void)
{
	u32 svr = get_svr();
	u32 soc = SVR_SOC_VER(svr);

	switch (soc) {
#ifdef CONFIG_PPC
	case SVR_9131:
	case SVR_9132:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 1, 1);
#endif
#ifdef ONFIG_ARM64
	case SVR_LS1012A:
		return IS_SVR_REV(svr, 1, 0);
#endif
	}
	return false;
}

bool has_erratum_a004477(void)
{
	u32 svr = get_svr();
	u32 soc = SVR_SOC_VER(svr);

	switch (soc) {
#ifdef CONFIG_PPC
	case SVR_P1010:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0);
	case SVR_P1022:
	case SVR_9131:
	case SVR_9132:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 1, 1);
	case SVR_P2020:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0) ||
			IS_SVR_REV(svr, 2, 1);
	case SVR_B4860:
	case SVR_B4420:
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 2, 0);
	case SVR_P4080:
		return IS_SVR_REV(svr, 2, 0) || IS_SVR_REV(svr, 3, 0);
#endif
	}

	return false;
}

bool has_erratum_a008751(void)
{
	u32 svr = get_svr();
	u32 soc = SVR_SOC_VER(svr);

	switch (soc) {
#ifdef CONFIG_ARM64
	case SVR_LS2080A:
	case SVR_LS2085A:
		return IS_SVR_REV(svr, 1, 0);
#endif
	}
	return false;
}

bool has_erratum_a010151(void)
{
	u32 svr = get_svr();
	u32 soc = SVR_SOC_VER(svr);

#ifdef CONFIG_ARM64
	if (IS_SVR_DEV(svr, SVR_DEV(SVR_LS1043A)))
		return IS_SVR_REV(svr, 1, 0) || IS_SVR_REV(svr, 1, 1);
#endif

	switch (soc) {
#ifdef CONFIG_ARM64
	case SVR_LS2080A:
	case SVR_LS2085A:
			/* fallthrough */
	case SVR_LS2088A:
			/* fallthrough */
	case SVR_LS2081A:
	case SVR_LS1046A:
	case SVR_LS1012A:
		return IS_SVR_REV(svr, 1, 0);
#endif
#ifdef CONFIG_ARCH_LS1021A
	case SOC_VER_LS1020:
	case SOC_VER_LS1021:
	case SOC_VER_LS1022:
	case SOC_VER_SLS1020:
		return IS_SVR_REV(svr, 2, 0);
#endif
	}
	return false;
}

#endif

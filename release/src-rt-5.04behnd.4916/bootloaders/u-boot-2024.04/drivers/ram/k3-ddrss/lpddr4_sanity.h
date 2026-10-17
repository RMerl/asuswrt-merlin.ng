/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef LPDDR4_SANITY_H
#define LPDDR4_SANITY_H

#include <errno.h>
#include <linux/types.h>
#include "lpddr4_if.h"

static inline u32 lpddr4_configsf(const lpddr4_config *obj);
static inline u32 lpddr4_privatedatasf(const lpddr4_privatedata *obj);

static inline u32 lpddr4_sanityfunction1(const lpddr4_config *config, const u16 *configsize);
static inline u32 lpddr4_sanityfunction2(const lpddr4_privatedata *pd, const lpddr4_config *cfg);
static inline u32 lpddr4_sanityfunction3(const lpddr4_privatedata *pd);
static inline u32 lpddr4_sanityfunction4(const lpddr4_privatedata *pd, const lpddr4_regblock cpp, const u32 *regvalue);
static inline u32 lpddr4_sanityfunction5(const lpddr4_privatedata *pd, const lpddr4_regblock cpp);
static inline u32 lpddr4_sanityfunction6(const lpddr4_privatedata *pd, const u64 *mmrvalue, const u8 *mmrstatus);
static inline u32 lpddr4_sanityfunction7(const lpddr4_privatedata *pd, const u8 *mrwstatus);
static inline u32 lpddr4_sanityfunction14(const lpddr4_privatedata *pd, const u64 *mask);
static inline u32 lpddr4_sanityfunction15(const lpddr4_privatedata *pd, const u64 *mask);
static inline u32 lpddr4_sanityfunction16(const lpddr4_privatedata *pd, const u32 *mask);
static inline u32 lpddr4_sanityfunction18(const lpddr4_privatedata *pd, const lpddr4_debuginfo *debuginfo);
static inline u32 lpddr4_sanityfunction19(const lpddr4_privatedata *pd, const lpddr4_lpiwakeupparam *lpiwakeupparam, const lpddr4_ctlfspnum *fspnum, const u32 *cycles);
static inline u32 lpddr4_sanityfunction21(const lpddr4_privatedata *pd, const lpddr4_eccenable *eccparam);
static inline u32 lpddr4_sanityfunction22(const lpddr4_privatedata *pd, const lpddr4_eccenable *eccparam);
static inline u32 lpddr4_sanityfunction23(const lpddr4_privatedata *pd, const lpddr4_reducmode *mode);
static inline u32 lpddr4_sanityfunction24(const lpddr4_privatedata *pd, const lpddr4_reducmode *mode);
static inline u32 lpddr4_sanityfunction25(const lpddr4_privatedata *pd, const bool *on_off);
static inline u32 lpddr4_sanityfunction27(const lpddr4_privatedata *pd, const lpddr4_dbimode *mode);
static inline u32 lpddr4_sanityfunction28(const lpddr4_privatedata *pd, const lpddr4_ctlfspnum *fspnum, const u32 *tref_val, const u32 *tras_max_val);
static inline u32 lpddr4_sanityfunction29(const lpddr4_privatedata *pd, const lpddr4_ctlfspnum *fspnum, const u32 *tref, const u32 *tras_max);

#define lpddr4_probesf lpddr4_sanityfunction1
#define lpddr4_initsf lpddr4_sanityfunction2
#define lpddr4_startsf lpddr4_sanityfunction3
#define lpddr4_readregsf lpddr4_sanityfunction4
#define lpddr4_writeregsf lpddr4_sanityfunction5
#define lpddr4_getmmrregistersf lpddr4_sanityfunction6
#define lpddr4_setmmrregistersf lpddr4_sanityfunction7
#define lpddr4_writectlconfigsf lpddr4_sanityfunction3
#define lpddr4_writephyconfigsf lpddr4_sanityfunction3
#define lpddr4_writephyindepconfigsf lpddr4_sanityfunction3
#define lpddr4_readctlconfigsf lpddr4_sanityfunction3
#define lpddr4_readphyconfigsf lpddr4_sanityfunction3
#define lpddr4_readphyindepconfigsf lpddr4_sanityfunction3
#define lpddr4_getctlinterruptmasksf lpddr4_sanityfunction14
#define lpddr4_setctlinterruptmasksf lpddr4_sanityfunction15
#define lpddr4_getphyindepinterruptmsf lpddr4_sanityfunction16
#define lpddr4_setphyindepinterruptmsf lpddr4_sanityfunction16
#define lpddr4_getdebuginitinfosf lpddr4_sanityfunction18
#define lpddr4_getlpiwakeuptimesf lpddr4_sanityfunction19
#define lpddr4_setlpiwakeuptimesf lpddr4_sanityfunction19
#define lpddr4_geteccenablesf lpddr4_sanityfunction21
#define lpddr4_seteccenablesf lpddr4_sanityfunction22
#define lpddr4_getreducmodesf lpddr4_sanityfunction23
#define lpddr4_setreducmodesf lpddr4_sanityfunction24
#define lpddr4_getdbireadmodesf lpddr4_sanityfunction25
#define lpddr4_getdbiwritemodesf lpddr4_sanityfunction25
#define lpddr4_setdbimodesf lpddr4_sanityfunction27
#define lpddr4_getrefreshratesf lpddr4_sanityfunction28
#define lpddr4_setrefreshratesf lpddr4_sanityfunction29
#define lpddr4_refreshperchipselectsf lpddr4_sanityfunction3
#define lpddr4_deferredregverifysf lpddr4_sanityfunction5

static inline u32 lpddr4_configsf(const lpddr4_config *obj)
{
	u32 ret = 0;

	if (obj == NULL)
		ret = EINVAL;

	return ret;
}

static inline u32 lpddr4_privatedatasf(const lpddr4_privatedata *obj)
{
	u32 ret = 0;

	if (obj == NULL)
		ret = EINVAL;

	return ret;
}

static inline u32 lpddr4_sanityfunction1(const lpddr4_config *config, const u16 *configsize)
{
	u32 ret = 0;

	if (configsize == NULL) {
		ret = EINVAL;
	} else if (lpddr4_configsf(config) == EINVAL) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_sanityfunction2(const lpddr4_privatedata *pd, const lpddr4_config *cfg)
{
	u32 ret = 0;

	if (lpddr4_privatedatasf(pd) == EINVAL) {
		ret = EINVAL;
	} else if (lpddr4_configsf(cfg) == EINVAL) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_sanityfunction3(const lpddr4_privatedata *pd)
{
	u32 ret = 0;

	if (lpddr4_privatedatasf(pd) == EINVAL)
		ret = EINVAL;

	return ret;
}

static inline u32 lpddr4_sanityfunction4(const lpddr4_privatedata *pd, const lpddr4_regblock cpp, const u32 *regvalue)
{
	u32 ret = 0;

	if (regvalue == NULL) {
		ret = EINVAL;
	} else if (lpddr4_privatedatasf(pd) == EINVAL) {
		ret = EINVAL;
	} else if (
		(cpp != LPDDR4_CTL_REGS) &&
		(cpp != LPDDR4_PHY_REGS) &&
		(cpp != LPDDR4_PHY_INDEP_REGS)
		) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_sanityfunction5(const lpddr4_privatedata *pd, const lpddr4_regblock cpp)
{
	u32 ret = 0;

	if (lpddr4_privatedatasf(pd) == EINVAL) {
		ret = EINVAL;
	} else if (
		(cpp != LPDDR4_CTL_REGS) &&
		(cpp != LPDDR4_PHY_REGS) &&
		(cpp != LPDDR4_PHY_INDEP_REGS)
		) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_sanityfunction6(const lpddr4_privatedata *pd, const u64 *mmrvalue, const u8 *mmrstatus)
{
	u32 ret = 0;

	if (mmrvalue == NULL) {
		ret = EINVAL;
	} else if (mmrstatus == NULL) {
		ret = EINVAL;
	} else if (lpddr4_privatedatasf(pd) == EINVAL) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_sanityfunction7(const lpddr4_privatedata *pd, const u8 *mrwstatus)
{
	u32 ret = 0;

	if (mrwstatus == NULL) {
		ret = EINVAL;
	} else if (lpddr4_privatedatasf(pd) == EINVAL) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_sanityfunction14(const lpddr4_privatedata *pd, const u64 *mask)
{
	u32 ret = 0;

	if (mask == NULL) {
		ret = EINVAL;
	} else if (lpddr4_privatedatasf(pd) == EINVAL) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_sanityfunction15(const lpddr4_privatedata *pd, const u64 *mask)
{
	u32 ret = 0;

	if (mask == NULL) {
		ret = EINVAL;
	} else if (lpddr4_privatedatasf(pd) == EINVAL) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_sanityfunction16(const lpddr4_privatedata *pd, const u32 *mask)
{
	u32 ret = 0;

	if (mask == NULL) {
		ret = EINVAL;
	} else if (lpddr4_privatedatasf(pd) == EINVAL) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_sanityfunction18(const lpddr4_privatedata *pd, const lpddr4_debuginfo *debuginfo)
{
	u32 ret = 0;

	if (debuginfo == NULL) {
		ret = EINVAL;
	} else if (lpddr4_privatedatasf(pd) == EINVAL) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_sanityfunction19(const lpddr4_privatedata *pd, const lpddr4_lpiwakeupparam *lpiwakeupparam, const lpddr4_ctlfspnum *fspnum, const u32 *cycles)
{
	u32 ret = 0;

	if (lpiwakeupparam == NULL) {
		ret = EINVAL;
	} else if (fspnum == NULL) {
		ret = EINVAL;
	} else if (cycles == NULL) {
		ret = EINVAL;
	} else if (lpddr4_privatedatasf(pd) == EINVAL) {
		ret = EINVAL;
	} else if (
		(*lpiwakeupparam != LPDDR4_LPI_PD_WAKEUP_FN) &&
		(*lpiwakeupparam != LPDDR4_LPI_SR_SHORT_WAKEUP_FN) &&
		(*lpiwakeupparam != LPDDR4_LPI_SR_LONG_WAKEUP_FN) &&
		(*lpiwakeupparam != LPDDR4_LPI_SR_LONG_MCCLK_GATE_WAKEUP_FN) &&
		(*lpiwakeupparam != LPDDR4_LPI_SRPD_SHORT_WAKEUP_FN) &&
		(*lpiwakeupparam != LPDDR4_LPI_SRPD_LONG_WAKEUP_FN) &&
		(*lpiwakeupparam != LPDDR4_LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_FN)
		) {
		ret = EINVAL;
	} else if (
		(*fspnum != LPDDR4_FSP_0) &&
		(*fspnum != LPDDR4_FSP_1) &&
		(*fspnum != LPDDR4_FSP_2)
		) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_sanityfunction21(const lpddr4_privatedata *pd, const lpddr4_eccenable *eccparam)
{
	u32 ret = 0;

	if (eccparam == NULL) {
		ret = EINVAL;
	} else if (lpddr4_privatedatasf(pd) == EINVAL) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_sanityfunction22(const lpddr4_privatedata *pd, const lpddr4_eccenable *eccparam)
{
	u32 ret = 0;

	if (eccparam == NULL) {
		ret = EINVAL;
	} else if (lpddr4_privatedatasf(pd) == EINVAL) {
		ret = EINVAL;
	} else if (
		(*eccparam != LPDDR4_ECC_DISABLED) &&
		(*eccparam != LPDDR4_ECC_ENABLED) &&
		(*eccparam != LPDDR4_ECC_ERR_DETECT) &&
		(*eccparam != LPDDR4_ECC_ERR_DETECT_CORRECT)
		) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_sanityfunction23(const lpddr4_privatedata *pd, const lpddr4_reducmode *mode)
{
	u32 ret = 0;

	if (mode == NULL) {
		ret = EINVAL;
	} else if (lpddr4_privatedatasf(pd) == EINVAL) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_sanityfunction24(const lpddr4_privatedata *pd, const lpddr4_reducmode *mode)
{
	u32 ret = 0;

	if (mode == NULL) {
		ret = EINVAL;
	} else if (lpddr4_privatedatasf(pd) == EINVAL) {
		ret = EINVAL;
	} else if (
		(*mode != LPDDR4_REDUC_ON) &&
		(*mode != LPDDR4_REDUC_OFF)
		) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_sanityfunction25(const lpddr4_privatedata *pd, const bool *on_off)
{
	u32 ret = 0;

	if (on_off == NULL) {
		ret = EINVAL;
	} else if (lpddr4_privatedatasf(pd) == EINVAL) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_sanityfunction27(const lpddr4_privatedata *pd, const lpddr4_dbimode *mode)
{
	u32 ret = 0;

	if (mode == NULL) {
		ret = EINVAL;
	} else if (lpddr4_privatedatasf(pd) == EINVAL) {
		ret = EINVAL;
	} else if (
		(*mode != LPDDR4_DBI_RD_ON) &&
		(*mode != LPDDR4_DBI_RD_OFF) &&
		(*mode != LPDDR4_DBI_WR_ON) &&
		(*mode != LPDDR4_DBI_WR_OFF)
		) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_sanityfunction28(const lpddr4_privatedata *pd, const lpddr4_ctlfspnum *fspnum, const u32 *tref_val, const u32 *tras_max_val)
{
	u32 ret = 0;

	if (fspnum == NULL) {
		ret = EINVAL;
	} else if (tref_val == NULL) {
		ret = EINVAL;
	} else if (tras_max_val == NULL) {
		ret = EINVAL;
	} else if (lpddr4_privatedatasf(pd) == EINVAL) {
		ret = EINVAL;
	} else if (
		(*fspnum != LPDDR4_FSP_0) &&
		(*fspnum != LPDDR4_FSP_1) &&
		(*fspnum != LPDDR4_FSP_2)
		) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

static inline u32 lpddr4_sanityfunction29(const lpddr4_privatedata *pd, const lpddr4_ctlfspnum *fspnum, const u32 *tref, const u32 *tras_max)
{
	u32 ret = 0;

	if (fspnum == NULL) {
		ret = EINVAL;
	} else if (tref == NULL) {
		ret = EINVAL;
	} else if (tras_max == NULL) {
		ret = EINVAL;
	} else if (lpddr4_privatedatasf(pd) == EINVAL) {
		ret = EINVAL;
	} else if (
		(*fspnum != LPDDR4_FSP_0) &&
		(*fspnum != LPDDR4_FSP_1) &&
		(*fspnum != LPDDR4_FSP_2)
		) {
		ret = EINVAL;
	} else {
	}

	return ret;
}

#endif  /* LPDDR4_SANITY_H */

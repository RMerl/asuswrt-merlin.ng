/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef LPDDR4_IF_H
#define LPDDR4_IF_H

#include <linux/types.h>
#ifdef CONFIG_K3_AM64_DDRSS
#include <lpddr4_am64_if.h>
#elif CONFIG_K3_AM62A_DDRSS
#include <lpddr4_am62a_if.h>
#else
#include <lpddr4_j721e_if.h>
#endif

typedef struct lpddr4_config_s lpddr4_config;
typedef struct lpddr4_privatedata_s lpddr4_privatedata;
typedef struct lpddr4_debuginfo_s lpddr4_debuginfo;
typedef struct lpddr4_fspmoderegs_s lpddr4_fspmoderegs;

typedef enum {
	LPDDR4_CTL_REGS		= 0U,
	LPDDR4_PHY_REGS		= 1U,
	LPDDR4_PHY_INDEP_REGS	= 2U
} lpddr4_regblock;

typedef enum {
	LPDDR4_DRV_NONE			= 0U,
	LPDDR4_DRV_SOC_PLL_UPDATE	= 1U
} lpddr4_infotype;

typedef enum {
	LPDDR4_LPI_PD_WAKEUP_FN				= 0U,
	LPDDR4_LPI_SR_SHORT_WAKEUP_FN			= 1U,
	LPDDR4_LPI_SR_LONG_WAKEUP_FN			= 2U,
	LPDDR4_LPI_SR_LONG_MCCLK_GATE_WAKEUP_FN		= 3U,
	LPDDR4_LPI_SRPD_SHORT_WAKEUP_FN			= 4U,
	LPDDR4_LPI_SRPD_LONG_WAKEUP_FN			= 5U,
	LPDDR4_LPI_SRPD_LONG_MCCLK_GATE_WAKEUP_FN	= 6U
} lpddr4_lpiwakeupparam;

typedef enum {
	LPDDR4_REDUC_ON		= 0U,
	LPDDR4_REDUC_OFF	= 1U
} lpddr4_reducmode;

typedef enum {
	LPDDR4_ECC_DISABLED		= 0U,
	LPDDR4_ECC_ENABLED		= 1U,
	LPDDR4_ECC_ERR_DETECT		= 2U,
	LPDDR4_ECC_ERR_DETECT_CORRECT	= 3U
} lpddr4_eccenable;

typedef enum {
	LPDDR4_DBI_RD_ON	= 0U,
	LPDDR4_DBI_RD_OFF	= 1U,
	LPDDR4_DBI_WR_ON	= 2U,
	LPDDR4_DBI_WR_OFF	= 3U
} lpddr4_dbimode;

typedef enum {
	LPDDR4_FSP_0	= 0U,
	LPDDR4_FSP_1	= 1U,
	LPDDR4_FSP_2	= 2U
} lpddr4_ctlfspnum;

typedef void (*lpddr4_infocallback)(const lpddr4_privatedata *pd, lpddr4_infotype infotype);

typedef void (*lpddr4_ctlcallback)(const lpddr4_privatedata *pd, lpddr4_intr_ctlinterrupt ctlinterrupt, u8 chipselect);

typedef void (*lpddr4_phyindepcallback)(const lpddr4_privatedata *pd, lpddr4_intr_phyindepinterrupt phyindepinterrupt, u8 chipselect);

u32 lpddr4_probe(const lpddr4_config *config, u16 *configsize);

u32 lpddr4_init(lpddr4_privatedata *pd, const lpddr4_config *cfg);

u32 lpddr4_start(const lpddr4_privatedata *pd);

u32 lpddr4_readreg(const lpddr4_privatedata *pd, lpddr4_regblock cpp, u32 regoffset, u32 *regvalue);

u32 lpddr4_writereg(const lpddr4_privatedata *pd, lpddr4_regblock cpp, u32 regoffset, u32 regvalue);

u32 lpddr4_getmmrregister(const lpddr4_privatedata *pd, u32 readmoderegval, u64 *mmrvalue, u8 *mmrstatus);

u32 lpddr4_setmmrregister(const lpddr4_privatedata *pd, u32 writemoderegval, u8 *mrwstatus);

u32 lpddr4_writectlconfig(const lpddr4_privatedata *pd, u32 regvalues[], u16 regnum[], u16 regcount);

u32 lpddr4_writephyconfig(const lpddr4_privatedata *pd, u32 regvalues[], u16 regnum[], u16 regcount);

u32 lpddr4_writephyindepconfig(const lpddr4_privatedata *pd, u32 regvalues[], u16 regnum[], u16 regcount);

u32 lpddr4_readctlconfig(const lpddr4_privatedata *pd, u32 regvalues[], u16 regnum[], u16 regcount);

u32 lpddr4_readphyconfig(const lpddr4_privatedata *pd, u32 regvalues[], u16 regnum[], u16 regcount);

u32 lpddr4_readphyindepconfig(const lpddr4_privatedata *pd, u32 regvalues[], u16 regnum[], u16 regcount);

u32 lpddr4_getctlinterruptmask(const lpddr4_privatedata *pd, u64 *mask);

u32 lpddr4_setctlinterruptmask(const lpddr4_privatedata *pd, const u64 *mask);

u32 lpddr4_checkctlinterrupt(const lpddr4_privatedata *pd, lpddr4_intr_ctlinterrupt intr, bool *irqstatus);

u32 lpddr4_ackctlinterrupt(const lpddr4_privatedata *pd, lpddr4_intr_ctlinterrupt intr);

u32 lpddr4_getphyindepinterruptmask(const lpddr4_privatedata *pd, u32 *mask);

u32 lpddr4_setphyindepinterruptmask(const lpddr4_privatedata *pd, const u32 *mask);

u32 lpddr4_checkphyindepinterrupt(const lpddr4_privatedata *pd, lpddr4_intr_phyindepinterrupt intr, bool *irqstatus);

u32 lpddr4_ackphyindepinterrupt(const lpddr4_privatedata *pd, lpddr4_intr_phyindepinterrupt intr);

u32 lpddr4_getdebuginitinfo(const lpddr4_privatedata *pd, lpddr4_debuginfo *debuginfo);

u32 lpddr4_getlpiwakeuptime(const lpddr4_privatedata *pd, const lpddr4_lpiwakeupparam *lpiwakeupparam, const lpddr4_ctlfspnum *fspnum, u32 *cycles);

u32 lpddr4_setlpiwakeuptime(const lpddr4_privatedata *pd, const lpddr4_lpiwakeupparam *lpiwakeupparam, const lpddr4_ctlfspnum *fspnum, const u32 *cycles);

u32 lpddr4_geteccenable(const lpddr4_privatedata *pd, lpddr4_eccenable *eccparam);

u32 lpddr4_seteccenable(const lpddr4_privatedata *pd, const lpddr4_eccenable *eccparam);

u32 lpddr4_getreducmode(const lpddr4_privatedata *pd, lpddr4_reducmode *mode);

u32 lpddr4_setreducmode(const lpddr4_privatedata *pd, const lpddr4_reducmode *mode);

u32 lpddr4_getdbireadmode(const lpddr4_privatedata *pd, bool *on_off);

u32 lpddr4_getdbiwritemode(const lpddr4_privatedata *pd, bool *on_off);

u32 lpddr4_setdbimode(const lpddr4_privatedata *pd, const lpddr4_dbimode *mode);

u32 lpddr4_getrefreshrate(const lpddr4_privatedata *pd, const lpddr4_ctlfspnum *fspnum, u32 *tref, u32 *tras_max);

u32 lpddr4_setrefreshrate(const lpddr4_privatedata *pd, const lpddr4_ctlfspnum *fspnum, const u32 *tref, const u32 *tras_max);

u32 lpddr4_refreshperchipselect(const lpddr4_privatedata *pd, const u32 trefinterval);

u32 lpddr4_deferredregverify(const lpddr4_privatedata *pd, lpddr4_regblock cpp, u32 regvalues[], u16 regnum[], u16 regcount);

#endif  /* LPDDR4_IF_H */

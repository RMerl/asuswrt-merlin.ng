/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#ifndef lpddr4_obj_if_h
#define lpddr4_obj_if_h

#include "lpddr4_if.h"

typedef struct lpddr4_obj_s {
	u32 (*probe)(const lpddr4_config *config, u16 *configsize);

	u32 (*init)(lpddr4_privatedata *pd, const lpddr4_config *cfg);

	u32 (*start)(const lpddr4_privatedata *pd);

	u32 (*readreg)(const lpddr4_privatedata *pd, lpddr4_regblock cpp, u32 regoffset, u32 *regvalue);

	u32 (*writereg)(const lpddr4_privatedata *pd, lpddr4_regblock cpp, u32 regoffset, u32 regvalue);

	u32 (*getmmrregister)(const lpddr4_privatedata *pd, u32 readmoderegval, u64 *mmrvalue, u8 *mmrstatus);

	u32 (*setmmrregister)(const lpddr4_privatedata *pd, u32 writemoderegval, u8 *mrwstatus);

	u32 (*writectlconfig)(const lpddr4_privatedata *pd, u32 regvalues[], u16 regnum[], u16 regcount);

	u32 (*writephyconfig)(const lpddr4_privatedata *pd, u32 regvalues[], u16 regnum[], u16 regcount);

	u32 (*writephyindepconfig)(const lpddr4_privatedata *pd, u32 regvalues[], u16 regnum[], u16 regcount);

	u32 (*readctlconfig)(const lpddr4_privatedata *pd, u32 regvalues[], u16 regnum[], u16 regcount);

	u32 (*readphyconfig)(const lpddr4_privatedata *pd, u32 regvalues[], u16 regnum[], u16 regcount);

	u32 (*readphyindepconfig)(const lpddr4_privatedata *pd, u32 regvalues[], u16 regnum[], u16 regcount);

	u32 (*getctlinterruptmask)(const lpddr4_privatedata *pd, u64 *mask);

	u32 (*setctlinterruptmask)(const lpddr4_privatedata *pd, const u64 *mask);

	u32 (*checkctlinterrupt)(const lpddr4_privatedata *pd, lpddr4_intr_ctlinterrupt intr, bool *irqstatus);

	u32 (*ackctlinterrupt)(const lpddr4_privatedata *pd, lpddr4_intr_ctlinterrupt intr);

	u32 (*getphyindepinterruptmask)(const lpddr4_privatedata *pd, u32 *mask);

	u32 (*setphyindepinterruptmask)(const lpddr4_privatedata *pd, const u32 *mask);

	u32 (*checkphyindepinterrupt)(const lpddr4_privatedata *pd, lpddr4_intr_phyindepinterrupt intr, bool *irqstatus);

	u32 (*ackphyindepinterrupt)(const lpddr4_privatedata *pd, lpddr4_intr_phyindepinterrupt intr);

	u32 (*getdebuginitinfo)(const lpddr4_privatedata *pd, lpddr4_debuginfo *debuginfo);

	u32 (*getlpiwakeuptime)(const lpddr4_privatedata *pd, const lpddr4_lpiwakeupparam *lpiwakeupparam, const lpddr4_ctlfspnum *fspnum, u32 *cycles);

	u32 (*setlpiwakeuptime)(const lpddr4_privatedata *pd, const lpddr4_lpiwakeupparam *lpiwakeupparam, const lpddr4_ctlfspnum *fspnum, const u32 *cycles);

	u32 (*geteccenable)(const lpddr4_privatedata *pd, lpddr4_eccenable *eccparam);

	u32 (*seteccenable)(const lpddr4_privatedata *pd, const lpddr4_eccenable *eccparam);

	u32 (*getreducmode)(const lpddr4_privatedata *pd, lpddr4_reducmode *mode);

	u32 (*setreducmode)(const lpddr4_privatedata *pd, const lpddr4_reducmode *mode);

	u32 (*getdbireadmode)(const lpddr4_privatedata *pd, bool *on_off);

	u32 (*getdbiwritemode)(const lpddr4_privatedata *pd, bool *on_off);

	u32 (*setdbimode)(const lpddr4_privatedata *pd, const lpddr4_dbimode *mode);

	u32 (*getrefreshrate)(const lpddr4_privatedata *pd, const lpddr4_ctlfspnum *fspnum, u32 *tref, u32 *tras_max);

	u32 (*setrefreshrate)(const lpddr4_privatedata *pd, const lpddr4_ctlfspnum *fspnum, const u32 *tref, const u32 *tras_max);

	u32 (*refreshperchipselect)(const lpddr4_privatedata *pd, const u32 trefinterval);

	u32 (*deferredregverify)(const lpddr4_privatedata *pd, lpddr4_regblock cpp, u32 regvalues[], u16 regnum[], u16 regcount);
} lpddr4_obj;

extern lpddr4_obj *lpddr4_getinstance(void);

#endif  /* lpddr4_obj_if_h */

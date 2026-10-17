// SPDX-License-Identifier: BSD-3-Clause
/*
 * Cadence DDR Driver
 *
 * Copyright (C) 2012-2022 Cadence Design Systems, Inc.
 * Copyright (C) 2018-2022 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <errno.h>

#include "cps_drv_lpddr4.h"
#include "lpddr4_ctl_regs.h"
#include "lpddr4_if.h"
#include "lpddr4.h"
#include "lpddr4_structs_if.h"

static void lpddr4_setrxoffseterror(lpddr4_ctlregs *ctlregbase, lpddr4_debuginfo *debuginfo, bool *errorfound);

u32 lpddr4_enablepiinitiator(const lpddr4_privatedata *pd)
{
	u32 result = 0U;
	u32 regval = 0U;

	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

	regval = CPS_FLD_SET(LPDDR4__PI_INIT_LVL_EN__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__PI_INIT_LVL_EN__REG)));
	regval = CPS_FLD_SET(LPDDR4__PI_NORMAL_LVL_SEQ__FLD, regval);
	CPS_REG_WRITE((&(ctlregbase->LPDDR4__PI_INIT_LVL_EN__REG)), regval);
	return result;
}

u32 lpddr4_getctlinterruptmask(const lpddr4_privatedata *pd, u64 *mask)
{
	u32 result = 0U;
	u32 lowermask = 0U;

	result = lpddr4_getctlinterruptmasksf(pd, mask);
	if (result == (u32)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
		lowermask = (u32)(CPS_FLD_READ(LPDDR4__INT_MASK_0__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__INT_MASK_0__REG))));
		*mask = (u64)(CPS_FLD_READ(LPDDR4__INT_MASK_1__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__INT_MASK_1__REG))));
		*mask = (u64)((*mask << WORD_SHIFT) | lowermask);
	}
	return result;
}

u32 lpddr4_setctlinterruptmask(const lpddr4_privatedata *pd, const u64 *mask)
{
	u32 result;
	u32 regval = 0;
	const u64 ui64one = 1ULL;
	const u32 ui32irqcount = (u32)LPDDR4_INTR_LOR_BITS + 1U;

	result = lpddr4_setctlinterruptmasksf(pd, mask);
	if ((result == (u32)0) && (ui32irqcount < 64U)) {
		if (*mask >= (ui64one << ui32irqcount))
			result = (u32)EINVAL;
	}

	if (result == (u32)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

		regval = (u32)(*mask & WORD_MASK);
		regval = CPS_FLD_WRITE(LPDDR4__INT_MASK_0__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__INT_MASK_0__REG)), regval);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_MASK_0__REG), regval);

		regval = (u32)((*mask >> WORD_SHIFT) & WORD_MASK);
		regval = CPS_FLD_WRITE(LPDDR4__INT_MASK_1__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__INT_MASK_1__REG)), regval);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_MASK_1__REG), regval);
	}
	return result;
}

u32 lpddr4_checkctlinterrupt(const lpddr4_privatedata *pd, lpddr4_intr_ctlinterrupt intr, bool *irqstatus)
{
	u32 result;
	u32 ctlirqstatus = 0;
	u32 fieldshift = 0;

	result = LPDDR4_INTR_CheckCtlIntSF(pd, intr, irqstatus);
	if (result == (u32)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

		if ((u32)intr >= (u32)WORD_SHIFT) {
			ctlirqstatus = CPS_REG_READ(&(ctlregbase->LPDDR4__INT_STATUS_1__REG));
			fieldshift = (u32)intr - ((u32)WORD_SHIFT);
		} else {
			ctlirqstatus = CPS_REG_READ(&(ctlregbase->LPDDR4__INT_STATUS_0__REG));
			fieldshift = (u32)intr;
		}

		if (fieldshift < WORD_SHIFT) {
			if (((ctlirqstatus >> fieldshift) & LPDDR4_BIT_MASK) > 0U)
				*irqstatus = true;
			else
				*irqstatus = false;
		}
	}
	return result;
}

u32 lpddr4_ackctlinterrupt(const lpddr4_privatedata *pd, lpddr4_intr_ctlinterrupt intr)
{
	u32 result = 0;
	u32 regval = 0;
	u32 localinterrupt = (u32)intr;

	result = LPDDR4_INTR_AckCtlIntSF(pd, intr);
	if (result == (u32)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

		if (localinterrupt > WORD_SHIFT) {
			localinterrupt = (localinterrupt - (u32)WORD_SHIFT);
			regval = ((u32)LPDDR4_BIT_MASK << localinterrupt);
			CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_ACK_1__REG), regval);
		} else {
			regval = ((u32)LPDDR4_BIT_MASK << localinterrupt);
			CPS_REG_WRITE(&(ctlregbase->LPDDR4__INT_ACK_0__REG), regval);
		}
	}

	return result;
}

void lpddr4_checkwrlvlerror(lpddr4_ctlregs *ctlregbase, lpddr4_debuginfo *debuginfo, bool *errfoundptr)
{
	u32 regval;
	u32 errbitmask = 0U;
	u32 snum;
	volatile u32 *regaddress;

	regaddress = (volatile u32 *)(&(ctlregbase->LPDDR4__PHY_WRLVL_ERROR_OBS_0__REG));
	errbitmask = (LPDDR4_BIT_MASK << 1) | (LPDDR4_BIT_MASK);
	for (snum = 0U; snum < DSLICE_NUM; snum++) {
		regval = CPS_REG_READ(regaddress);
		if ((regval & errbitmask) != 0U) {
			debuginfo->wrlvlerror = CDN_TRUE;
			*errfoundptr = true;
		}
		regaddress = lpddr4_addoffset(regaddress, (u32)SLICE_WIDTH);
	}
}

static void lpddr4_setrxoffseterror(lpddr4_ctlregs *ctlregbase, lpddr4_debuginfo *debuginfo, bool *errorfound)
{
	volatile u32 *regaddress;
	u32 snum = 0U;
	u32 errbitmask = 0U;
	u32 regval = 0U;

	if (*errorfound == (bool)false) {
		regaddress = (volatile u32 *)(&(ctlregbase->LPDDR4__PHY_RX_CAL_LOCK_OBS_0__REG));
		errbitmask = (RX_CAL_DONE) | (NIBBLE_MASK);
		for (snum = (u32)0U; snum < DSLICE_NUM; snum++) {
			regval = CPS_FLD_READ(LPDDR4__PHY_RX_CAL_LOCK_OBS_0__FLD, CPS_REG_READ(regaddress));
			if ((regval & errbitmask) != RX_CAL_DONE) {
				debuginfo->rxoffseterror = (u8)true;
				*errorfound = true;
			}
			regaddress = lpddr4_addoffset(regaddress, (u32)SLICE_WIDTH);
		}
	}
}

u32 lpddr4_getdebuginitinfo(const lpddr4_privatedata *pd, lpddr4_debuginfo *debuginfo)
{
	u32 result = 0U;
	bool errorfound = false;

	result = lpddr4_getdebuginitinfosf(pd, debuginfo);
	if (result == (u32)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
		lpddr4_seterrors(ctlregbase, debuginfo, (u8 *)&errorfound);
		lpddr4_setsettings(ctlregbase, errorfound);
		lpddr4_setrxoffseterror(ctlregbase, debuginfo, &errorfound);
		errorfound = (bool)lpddr4_checklvlerrors(pd, debuginfo, errorfound);
	}

	if (errorfound == (bool)true)
		result = (u32)EPROTO;

	return result;
}

u32 lpddr4_geteccenable(const lpddr4_privatedata *pd, lpddr4_eccenable *eccparam)
{
	u32 result = 0U;
	u32 fldval = 0U;

	result = lpddr4_geteccenablesf(pd, eccparam);
	if (result == (u32)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

		fldval = CPS_FLD_READ(LPDDR4__ECC_ENABLE__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__ECC_ENABLE__REG)));
		switch (fldval) {
		case 3:
			*eccparam = LPDDR4_ECC_ERR_DETECT_CORRECT;
			break;
		case 2:
			*eccparam = LPDDR4_ECC_ERR_DETECT;
			break;
		case 1:
			*eccparam = LPDDR4_ECC_ENABLED;
			break;
		default:
			*eccparam = LPDDR4_ECC_DISABLED;
			break;
		}
	}
	return result;
}

u32 lpddr4_seteccenable(const lpddr4_privatedata *pd, const lpddr4_eccenable *eccparam)
{
	u32 result = 0U;
	u32 regval = 0U;

	result = lpddr4_seteccenablesf(pd, eccparam);
	if (result == (u32)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;

		regval = CPS_FLD_WRITE(LPDDR4__ECC_ENABLE__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__ECC_ENABLE__REG)), *eccparam);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__ECC_ENABLE__REG), regval);
	}
	return result;
}

u32 lpddr4_getreducmode(const lpddr4_privatedata *pd, lpddr4_reducmode *mode)
{
	u32 result = 0U;

	result = lpddr4_getreducmodesf(pd, mode);
	if (result == (u32)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
		if (CPS_FLD_READ(LPDDR4__REDUC__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__REDUC__REG))) == 0U)
			*mode = LPDDR4_REDUC_ON;
		else
			*mode = LPDDR4_REDUC_OFF;
	}
	return result;
}
u32 lpddr4_setreducmode(const lpddr4_privatedata *pd, const lpddr4_reducmode *mode)
{
	u32 result = 0U;
	u32 regval = 0U;

	result = lpddr4_setreducmodesf(pd, mode);
	if (result == (u32)0) {
		lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
		regval = (u32)CPS_FLD_WRITE(LPDDR4__REDUC__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__REDUC__REG)), *mode);
		CPS_REG_WRITE(&(ctlregbase->LPDDR4__REDUC__REG), regval);
	}
	return result;
}

u32 lpddr4_checkmmrreaderror(const lpddr4_privatedata *pd, u64 *mmrvalue, u8 *mrrstatus)
{
	u32 lowerdata;
	lpddr4_ctlregs *ctlregbase = (lpddr4_ctlregs *)pd->ctlbase;
	u32 result = (u32)0;

	if (lpddr4_pollctlirq(pd, LPDDR4_INTR_MRR_ERROR, 100) == 0U) {
		*mrrstatus = (u8)CPS_FLD_READ(LPDDR4__MRR_ERROR_STATUS__FLD, CPS_REG_READ(&(ctlregbase->LPDDR4__MRR_ERROR_STATUS__REG)));
		*mmrvalue = (u64)0;
		result = (u32)EIO;
	} else {
		*mrrstatus = (u8)0;
		lowerdata = CPS_REG_READ(&(ctlregbase->LPDDR4__PERIPHERAL_MRR_DATA_0__REG));
		*mmrvalue = CPS_REG_READ(&(ctlregbase->LPDDR4__PERIPHERAL_MRR_DATA_1__REG));
		*mmrvalue = (u64)((*mmrvalue << WORD_SHIFT) | lowerdata);
		result = lpddr4_ackctlinterrupt(pd, LPDDR4_INTR_MR_READ_DONE);
	}
	return result;
}

u32 lpddr4_getdslicemask(u32 dslicenum, u32 arrayoffset)
{
	u32 rwmask = 0U;

	switch (dslicenum) {
	case 0:
		if (arrayoffset < DSLICE0_REG_COUNT)
			rwmask = g_lpddr4_data_slice_0_rw_mask[arrayoffset];
		break;
	case 1:
		if (arrayoffset < DSLICE1_REG_COUNT)
			rwmask = g_lpddr4_data_slice_1_rw_mask[arrayoffset];
		break;
	case 2:
		if (arrayoffset < DSLICE2_REG_COUNT)
			rwmask = g_lpddr4_data_slice_2_rw_mask[arrayoffset];
		break;
	default:
		if (arrayoffset < DSLICE3_REG_COUNT)
			rwmask = g_lpddr4_data_slice_3_rw_mask[arrayoffset];
		break;
	}
	return rwmask;
}

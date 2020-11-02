/*
 * Functions for supporting HW save-restore functionality
 *
 * Copyright 2020 Broadcom
 *
 * This program is the proprietary software of Broadcom and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1. This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use
 * all reasonable efforts to protect the confidentiality thereof, and to
 * use this information only in connection with your use of Broadcom
 * integrated circuit products.
 *
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES,
 * REPRESENTATIONS OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR
 * OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY
 * DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
 * NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES,
 * ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING
 * OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL
 * BROADCOM OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL,
 * SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR
 * IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
 * IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii)
 * ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
 * OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY
 * NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *
 * $Id: saverestore.c 708017 2017-06-29 14:11:45Z $
 */

#include <typedefs.h>
#include <osl.h>
#include <sbchipc.h>
#include <siutils.h>
#include <bcmdevs.h>
#include <bcmutils.h>
#include <hndsoc.h>
#include <sbsocram.h>
#include <hndpmu.h>

#include "siutils_priv.h"
#include <saverestore.h>

static uint32 sr_control(si_t *sih, uint32 n, uint32 mask, uint32 val);

#ifdef SR_DEBUG
static uint32 nvram_min_sr_mask;
static bool min_sr_mask_valid = FALSE;
#endif /* SR_DEBUG */

/* minimal SR functionality to support srvsdb */
#ifndef SR_ESSENTIALS_DISABLED
bool _sr_essentials = TRUE;
#else
bool _sr_essentials = FALSE;
#endif // endif

/* Power save related save/restore support */
#ifndef SAVERESTORE_DISABLED
bool _sr = TRUE;
#else
bool _sr = FALSE;
#endif // endif

/* Access the save-restore portion of the PMU chipcontrol reg */
uint32
sr_chipcontrol(si_t *sih, uint32 mask, uint32 val)
{
	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID:
	case BCM4335_CHIP_ID:
	case BCM4345_CHIP_ID:
#ifdef UNRELEASEDCHIP
	case BCM43430_CHIP_ID:
#endif /* UNRELEASEDCHIP */
	case BCM43909_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43242_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
		/* All 32 bits of pmu chipctl3 are for save-restore.
		 * Hence not needed of masking/shifting the input arguments
		 */
		val = si_pmu_chipcontrol(sih, 3, mask, val);
		break;
	case BCM43602_CHIP_ID: /* uses a newer access mechanism */
	case BCM4349_CHIP_GRPID: /* uses a newer access mechanism */
	default:
		val = BCME_NOTFOUND;
		break;
	}
	return val;
}

/* Access the chipcontrol reg 4 */
uint32
sr_chipcontrol4(si_t *sih, uint32 mask, uint32 val)
{
	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID:
	case BCM4324_CHIP_ID:
	case BCM4335_CHIP_ID:
	case BCM4345_CHIP_ID:
#ifdef UNRELEASEDCHIP
	case BCM43430_CHIP_ID:
#endif /* UNRELEASEDCHIP */
	case BCM43909_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
		/* In 43239, all 32 bits are for save-restore chipcontrol.
		 * Hence not needed of masking/shifting the input arguments
		 */
		val = si_pmu_chipcontrol(sih, 4, mask, val);
		break;
	case BCM43242_CHIP_ID: /* fall through. pmu chip control #4 reg contains no S/R bits */
	case BCM43602_CHIP_ID: /* uses a newer access mechanism */
	case BCM4349_CHIP_GRPID: /* uses a newer access mechanism */
	default:
		val = BCME_NOTFOUND;
		break;
	}
	return val;
}

/* Access the chipcontrol reg 2 */
uint32
sr_chipcontrol2(si_t *sih, uint32 mask, uint32 val)
{
	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID:
	case BCM4324_CHIP_ID:
	case BCM4335_CHIP_ID:
	case BCM4345_CHIP_ID:
#ifdef UNRELEASEDCHIP
	case BCM43430_CHIP_ID:
#endif /* UNRELEASEDCHIP */
	case BCM43909_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
		/* In 43239, all 32 bits are for save-restore chipcontrol.
		 * Hence not needed of masking/shifting the input arguments
		 */
		val = si_pmu_chipcontrol(sih, 2, mask, val);
		break;
	case BCM43602_CHIP_ID: /* uses a newer access mechanism */
	case BCM4349_CHIP_GRPID: /* uses a newer access mechanism */
	case BCM43242_CHIP_ID: /* fall through. pmu chip control #2 reg contains no S/R bits */
	default:
		val = BCME_NOTFOUND;
		break;
	}
	return val;
}

/* Access the chipcontrol reg 5 */
uint32
sr_chipcontrol5(si_t *sih, uint32 mask, uint32 val)
{
	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID:
	case BCM4324_CHIP_ID:
	case BCM4335_CHIP_ID:
	case BCM4345_CHIP_ID:
#ifdef UNRELEASEDCHIP
	case BCM43430_CHIP_ID:
#endif /* UNRELEASEDCHIP */
	case BCM43909_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
		/* In 43239, all 32 bits are for save-restore chipcontrol.
		 * Hence not needed of masking/shifting the input arguments
		 */
		val = si_pmu_chipcontrol(sih, 5, mask, val);
		break;
	case BCM43602_CHIP_ID: /* uses a newer access mechanism */
	case BCM43462_CHIP_ID: /* uses a newer access mechanism */
	case BCM4349_CHIP_GRPID: /* uses a newer access mechanism */
	case BCM43242_CHIP_ID: /* fall through. pmu chip control #5 reg contains no S/R bits */
	default:
		val = BCME_NOTFOUND;
		break;
	}
	return val;
}

/* Access the chipcontrol reg 6 */
uint32
sr_chipcontrol6(si_t *sih, uint32 mask, uint32 val)
{
	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID:
	case BCM4324_CHIP_ID:
	case BCM4335_CHIP_ID:
	case BCM4345_CHIP_ID:
#ifdef UNRELEASEDCHIP
	case BCM43430_CHIP_ID:
#endif /* UNRELEASEDCHIP */
	case BCM43909_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
		/* In 43239, all 32 bits are for save-restore chipcontrol.
		 * Hence not needed of masking/shifting the input arguments
		 */
		val = si_pmu_chipcontrol(sih, 6, mask, val);
		break;
	case BCM43602_CHIP_ID: /* uses a newer access mechanism */
	case BCM43462_CHIP_ID: /* uses a newer access mechanism */
	case BCM4349_CHIP_GRPID: /* uses a newer access mechanism */
	case BCM43242_CHIP_ID: /* fall through. pmu chip control #6 reg contains no S/R bits */
	default:
		val = BCME_NOTFOUND;
		break;
	}
	return val;
}

/** Access regulator control register 4 */
uint32
sr_regcontrol4(si_t *sih, uint32 mask, uint32 val)
{
	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID:
	case BCM4324_CHIP_ID:
	case BCM4335_CHIP_ID:
	case BCM4345_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
	case BCM43909_CHIP_ID:
#ifdef UNRELEASEDCHIP
	case BCM43430_CHIP_ID:
#endif /* UNRELEASEDCHIP */

		/* In 43239, all 32 bits are for save-restore chipcontrol.
		 * Hence not needed of masking/shifting the input arguments
		 */
		val = si_pmu_regcontrol(sih, 4, mask, val);
		break;
	case BCM43242_CHIP_ID: /* fall through. pmu reg control #4 is not used in PHOENIX2 branch */
	case BCM43602_CHIP_ID: /* uses a newer access mechanism */
	case BCM4349_CHIP_GRPID: /* uses a newer access mechanism */
	default:
		val = BCME_NOTFOUND;
		break;
	}
	return val;
}

/** register access introduced with 43602a0 */
static uint32
sr_control(si_t *sih, uint32 n, uint32 mask, uint32 val)
{
	uint32 retVal;
	chipcregs_t *cc;
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	uint origidx = si_coreidx(sih);
	uint intr_val = 0;
	volatile unsigned int *p;

	/* Block ints and save current core */
	INTR_OFF(sii, intr_val);
	cc = si_setcoreidx(sih, SI_CC_IDX);

	if (n == 0)
		p = &cc->sr_control0;
	else
		p = &cc->sr_control1;

	retVal = R_REG(sii->osh, p);

	if (mask != 0) {
		AND_REG(sii->osh, p, ~mask);
		OR_REG(sii->osh, p, val);
	}

	si_setcoreidx(sih, origidx);
	INTR_RESTORE(sii, intr_val);

	return retVal;
}

/* Function to change the sr memory control to sr engine or driver access */
static int
sr_enable_sr_mem_for(si_t *sih, int opt)
{
	int retVal = 0;
	si_info_t *sii;
	uint origidx;
	sbsocramregs_t *socramregs;

	sii = SI_INFO(sih);
	origidx = si_coreidx(sih);

	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID:
		/* Switch to SOCRAM core */
		socramregs = si_setcore(sih, SOCRAM_CORE_ID, 0);
		if (!socramregs)
			goto done;

		switch (opt) {
		case SR_HOST:
			W_REG(sii->osh, &socramregs->sr_control,
				(R_REG(sii->osh, &socramregs->sr_control) | 0x800));
			break;
		case SR_ENGINE:
			W_REG(sii->osh, &socramregs->sr_control,
				(R_REG(sii->osh, &socramregs->sr_control) & ~(0x800)));
			break;
		default:
			retVal = -1;
		}
		break;
	case BCM4324_CHIP_ID:
	case BCM43242_CHIP_ID:
		/* Disable all the controls */
		/* Switch to SOCRAM core */
		socramregs = si_setcore(sih, SOCRAM_CORE_ID, 0);
		if (!socramregs)
			goto done;
		W_REG(sii->osh, &socramregs->sr_control,
			(R_REG(sii->osh, &socramregs->sr_control) & ~(0x800)));
		/* Below two operations shall not be combined. */
		sr_chipcontrol(sih, 0x1, 0x0); /* Disable sr mem_clk_en */
		sr_chipcontrol(sih, 0x2, 0x0); /* Disable sr mem_clk_sel */

		switch (opt) {
		case SR_HOST:
			W_REG(sii->osh, &socramregs->sr_control,
				(R_REG(sii->osh, &socramregs->sr_control) | 0x800));
			break;
		case SR_ENGINE:
			/* Below two operations shall not be combined. */
			sr_chipcontrol(sih, 0x2, 0x2); /* Enable sr mem_clk_sel */
			sr_chipcontrol(sih, 0x1, 0x1); /* Enable sr mem_clk_en */
			break;
		default:
			retVal = -1;
		}
		break;
	case BCM4335_CHIP_ID:
	case BCM4345_CHIP_ID:
		/* setcore d11 */
		socramregs = si_setcore(sih, D11_CORE_ID, 0);
		if (!socramregs)
			goto done;

		/* Below two operation shall not be combined. */
		/* chipctlreg 0x3 [hexpr ([chipctlreg 0x3] & ~(0x1))] */
		sr_chipcontrol(sih, 0x1, 0x0);
		/* chipctlreg 0x3 [hexpr ([chipctlreg 0x3] & ~(0x4))] */
		sr_chipcontrol(sih, 0x4, 0x0);

		switch (opt) {
		case SR_HOST:
			break;
		case SR_ENGINE:
			/* Below two operation shall not be combined. */
			/* chipctlreg 0x3 [hexpr ([chipctlreg 0x3] | (0x1))] */
			sr_chipcontrol(sih, 0x1, 0x1);
			break;
		default:
			retVal = -1;
		}
		break;

	default:
		retVal = -1;
	}

	/* Return to previous core */
	si_setcoreidx(sih, origidx);

done:

	return retVal;
}

#if defined(SAVERESTORE)
/** Interface function used to read/write value from/to sr memory */
uint32
sr_mem_access(si_t *sih, int op, uint32 addr, uint32 data)
{
	si_info_t *sii;
	uint origidx;
	sbsocramregs_t *socramregs;
	uint32 sr_ctrl;

	BCM_REFERENCE(sr_ctrl);
	sii = SI_INFO(sih);
	origidx = si_coreidx(sih);

	sr_enable_sr_mem_for(sih, SR_HOST);

	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID:
	case BCM4324_CHIP_ID:
	case BCM4335_CHIP_ID:
	case BCM43242_CHIP_ID:
	case BCM4345_CHIP_ID:
		/* Switch to SOCRAM core */
		socramregs = si_setcore(sih, SOCRAM_CORE_ID, 0);
		if (!socramregs)
			goto done;
		sr_ctrl = R_REG(sii->osh, &socramregs->sr_control);
		ASSERT(addr < (SRCTL_BANK_NUM(sr_ctrl) * SRCTL_BANK_SIZE(sr_ctrl)));
		BCM_REFERENCE(sr_ctrl);
		W_REG(sii->osh, &socramregs->sr_address, addr);
		if (op == IOV_SET) {
			W_REG(sii->osh, &socramregs->sr_data, data);
		}
		else {
			data = R_REG(sii->osh, &socramregs->sr_data);
		}
		break;
	default:
		data = 0xFFFFFFFF;
	}

	/* Return to previous core */
	si_setcoreidx(sih, origidx);

done:
	sr_enable_sr_mem_for(sih, SR_ENGINE);

	return data;
}
#endif /* SAVERESTORE */

/** This address is offset to the start of the txfifo bank containing the SR assembly code */
static uint32
BCMATTACHFN(sr_asm_addr)(si_t *sih)
{
	switch (CHIPID(sih->chip)) {
	case BCM4335_CHIP_ID:
		return CC4_4335_SR_ASM_ADDR;
	case BCM4345_CHIP_ID:
	case BCM43909_CHIP_ID:
		return CC4_4345_SR_ASM_ADDR;
		break;

	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
		return CC4_4350_SR_ASM_ADDR;
	case BCM43602_CHIP_ID:
		return CC_SR1_43602_SR_ASM_ADDR;
	case BCM4349_CHIP_GRPID:
		return CC_SR1_4349_SR_ASM_ADDR;
	case BCM43242_CHIP_ID: /* fall through: SR mem in this chip is not contained in tx fifo */
	default:
		ASSERT(0);
		break;
	}

	return 0;
}

/** Initialize the save-restore engine during chip attach */
void
BCMATTACHFN(sr_save_restore_init)(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	chipcregs_t *cc;
	uint origidx;
	uint intr_val = 0;

	/* 43242 supports SR-VSDB but not SR-PS. Still it needs initialization. */
	if (sr_cap(sih) == FALSE && !(CHIPID(sih->chip) == BCM43242_CHIP_ID))
		return;

	/* Block ints and save current core */
	INTR_OFF(sii, intr_val);
	origidx = si_coreidx(sih);

	cc = si_setcoreidx(sih, SI_CC_IDX);
	UNUSED_PARAMETER(cc);
	ASSERT(cc != NULL);
	BCM_REFERENCE(cc);

	switch (CHIPID(sih->chip)) {
	case BCM43340_CHIP_ID:
	case BCM43341_CHIP_ID:
#ifdef UNRELEASEDCHIP
	case BCM43342_CHIP_ID:
#endif /* UNRELEASEDCHIP */
	case BCM4334_CHIP_ID: {
		uint8 clkdiv = 2;
		uint8 chainlen = 0;
		uint8 macphy_disable = 0;
		uint8 logic_disable = 0;
		uint8 retmode_enable = 1;
		uint8 dft_enable = 0;
		uint16 grp0chainlen = 0;
		uint16 grp1chainlen = 0;
#ifdef SR_DEBUG
		const char *val;
#endif // endif
		if ((CHIPID(sih->chip) == BCM4334_CHIP_ID) && (CHIPREV(sih->chiprev) < 3))
			goto done;

		OR_REG(sii->osh, &cc->pmucontrol, PCTL_LQ_REQ_EN);

		si_pmu_pllcontrol(sih, PMU15_PLL_PLLCTL1,
			PMU15_PLL_PC1_OPENLP_EN_MASK, (1 << PMU15_PLL_PC1_OPENLP_EN_SHIFT));
		/* Toggle it to update */
		OR_REG(sii->osh, &cc->pmucontrol, PCTL_PLL_PLLCTL_UPD);

		logic_disable = 0;
		macphy_disable = 0;
		chainlen = 0;
		clkdiv = 2;
		if ((CHIPID(sih->chip) == BCM43340_CHIP_ID) ||
			(CHIPID(sih->chip) == BCM43341_CHIP_ID)) {
			grp0chainlen = PMU43341_RCTLGRP_CHAIN_LEN_GRP0;
			grp1chainlen = PMU43341_RCTLGRP_CHAIN_LEN_GRP1;
		}
		else
		{
			grp0chainlen = PMU4334_RCTLGRP_CHAIN_LEN_GRP0;
			grp1chainlen = PMU4334_RCTLGRP_CHAIN_LEN_GRP1;
		}

		/* Retention Control */
		W_REG(sii->osh, &cc->retention_ctl,
			(logic_disable << PMU_RCTL_LOGIC_DISABLE_SHIFT) |
			(macphy_disable << PMU_RCTL_MACPHY_DISABLE_SHIFT) |
			(chainlen << PMU_RCTL_CHAIN_LEN_SHIFT) | clkdiv);

		/* Retention Group 0 */
		W_REG(sii->osh, &cc->retention_grpidx, 0);
		W_REG(sii->osh, &cc->retention_grpctl,
			(retmode_enable << PMU_RCTLGRP_RMODE_ENABLE_SHIFT) |
			(dft_enable << PMU_RCTLGRP_DFT_ENABLE_SHIFT) |
			grp0chainlen);

		/* Retention Group 1 */
		W_REG(sii->osh, &cc->retention_grpidx, 1);
		W_REG(sii->osh, &cc->retention_grpctl,
			(retmode_enable << PMU_RCTLGRP_RMODE_ENABLE_SHIFT) |
			(dft_enable << PMU_RCTLGRP_DFT_ENABLE_SHIFT) |
			grp1chainlen);

		if (CST4334_CHIPMODE_HSIC(sih->chipst)) {
			/* HSIC specific */
			si_pmu_chipcontrol(sih, 2,
				CCTRL4334_PMU_WAKEUP_HSIC, CCTRL4334_PMU_WAKEUP_HSIC);
			si_pmu_regcontrol(sih, 4, 1 << 27, 0);

		} else {
			/* SDIO specific */
			si_pmu_chipcontrol(sih, 2,
				CCTRL4334_PMU_WAKEUP_AOS, CCTRL4334_PMU_WAKEUP_AOS);
		}

		if (((CHIPID(sih->chip) == BCM4334_CHIP_ID) && (CHIPREV(sih->chiprev) >= 3)) ||
#ifdef UNRELEASEDCHIP
			(CHIPID(sih->chip) == BCM43342_CHIP_ID) ||
#endif /* UNRELEASEDCHIP */
			0) {
			/* Enable the saverestore fix for 4334b2 */
			si_pmu_chipcontrol(sih, 3,
				CCTRL4334_BLOCK_EXTRNL_WAKE | CCTRL4334_SAVERESTORE_FIX,
				CCTRL4334_BLOCK_EXTRNL_WAKE | CCTRL4334_SAVERESTORE_FIX);
		} else if ((CHIPID(sih->chip) == BCM43340_CHIP_ID) ||
			(CHIPID(sih->chip) == BCM43341_CHIP_ID)) {
			uint32 val;

			val = CCTRL43341_BLOCK_EXTRNL_WAKE | CCTRL43341_SAVERESTORE_FIX;
			if (sr_get_cur_minresmask(sih) & PMURES_BIT(RES4334_WL_PWRSW_PU))
				val &= ~CCTRL43341_BLOCK_EXTRNL_WAKE;

			/* Enable the saverestore fix for 4334b2 */
			si_pmu_chipcontrol(sih, 3,
				CCTRL43341_BLOCK_EXTRNL_WAKE | CCTRL43341_SAVERESTORE_FIX,
				val);
		}

#ifdef SR_OOB_WAR
		if ((CHIPID(sih->chip) == BCM4334_CHIP_ID) && (CHIPREV(sih->chiprev) == 2)) {
			si_gpioouten(sih, (1 << 0), (1 << 0), GPIO_HI_PRIORITY);
			si_gpioout(sih, (1 << 0), (1 << 0), GPIO_HI_PRIORITY);
		}
#endif // endif
#ifdef SR_DEBUG
		if ((val = getvar(NULL, "rmin")) != NULL) {
			nvram_min_sr_mask = (uint32)bcm_strtoul(val, NULL, 0);
			min_sr_mask_valid = TRUE;
		}
#endif /* SR_DEBUG */
	}
	break;
	case BCM43239_CHIP_ID: {
		sbsocramregs_t *regs;

		/* Switch to SOCRAM core */
		if (!(regs = si_setcore(sih, SOCRAM_CORE_ID, 0)))
			goto done;

		/* Download the save-restore firmware */
		sr_download_firmware(sih);

		/* Initialize save-trigger controls like polarity, etc */
		W_REG(sii->osh, &regs->sr_control,
		    (R_REG(sii->osh, &regs->sr_control)| (0x0A00 << 16)));

		/* Enable the save-restore engine */
		sr_engine_enable(sih, IOV_SET, TRUE);
		break;
	}
	case BCM43242_CHIP_ID: {
		/* Switch to SOCRAM core */
		if (!si_setcore(sih, SOCRAM_CORE_ID, 0))
			goto done;

		/* Disable the save-restore engine */
		sr_engine_enable(sih, IOV_SET, FALSE);

		/* Download the save-restore firmware */
		sr_download_firmware(sih);

		/* Set min_div */
		sr_chipcontrol(sih, 0xF0, (0x30));

		/* Enable the save-restore engine */
		sr_engine_enable(sih, IOV_SET, TRUE);
		break;
	}

	case BCM4335_CHIP_ID: {
		/* Download the save-restore firmware */
		/* Initialize save-trigger controls like polarity, etc */

		/* For 4335, SR binary is stored in TX-FIFO */
		/* Hence it is downloaded in wlc.c */
		sr_chipcontrol4(sih, CC4_SR_INIT_ADDR_MASK,
			sr_asm_addr(sih) << CC4_SR_INIT_ADDR_SHIFT);
		sr_enable_sr_mem_for(sih, SR_HOST);
		sr_enable_sr_mem_for(sih, SR_ENGINE);
		sr_engine_enable(sih, IOV_SET, TRUE);
		break;
	}
	case BCM4345_CHIP_ID:
	case BCM43909_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
	{
		/* Add ASM Init address */
		sr_chipcontrol4(sih, CC4_SR_INIT_ADDR_MASK,
			sr_asm_addr(sih) << CC4_SR_INIT_ADDR_SHIFT);

		/* Turn on MEMLPDO before MEMLPLDO power switch .(bit 159) */
		sr_regcontrol4(sih, VREG4_4350_MEMLPDO_PU_MASK,
			(1 << VREG4_4350_MEMLPDO_PU_SHIFT));

		/* Switch to ALP/HT clk as SR clk is too long */
		sr_chipcontrol4(sih, CC4_4350_EN_SR_CLK_ALP_MASK | CC4_4350_EN_SR_CLK_HT_MASK,
			(1 << CC4_4350_EN_SR_CLK_ALP_SHIFT) | (1 << CC4_4350_EN_SR_CLK_HT_SHIFT));

		/* MEMLPLDO power switch enable */
		sr_chipcontrol2(sih, CC2_4350_MEMLPLDO_PWRSW_EN_MASK,
			(1 << CC2_4350_MEMLPLDO_PWRSW_EN_SHIFT));

		/* VDDM power switch enable */
		sr_chipcontrol2(sih, CC2_4350_VDDM_PWRSW_EN_MASK,
			(1 << CC2_4350_VDDM_PWRSW_EN_SHIFT));
#ifdef SRFAST
		sr_chipcontrol2(sih, CC2_4350_PHY_PWRSW_UPTIME_MASK,
			0x4 << CC2_4350_PHY_PWRSW_UPTIME_SHIFT);
		sr_chipcontrol2(sih, CC2_4350_VDDM_PWRSW_UPDELAY_MASK,
			0x4 << CC2_4350_VDDM_PWRSW_UPDELAY_SHIFT);
#endif // endif

		if ((BCM4350_CHIP(sih->chip) &&
			CST4350_CHIPMODE_SDIOD(sih->chipst)) ||
			((CHIPID(sih->chip) == BCM4345_CHIP_ID) &&
			CST4345_CHIPMODE_SDIOD(sih->chipst))) {
			sr_chipcontrol2(sih, CC2_4350_SDIO_AOS_WAKEUP_MASK,
				(1 << CC2_4350_SDIO_AOS_WAKEUP_SHIFT));
		} else if ((BCM4350_CHIP(sih->chip) &&
			CST4350_CHIPMODE_PCIE(sih->chipst)) ||
			((CHIPID(sih->chip) == BCM4345_CHIP_ID) &&
			CST4345_CHIPMODE_PCIE(sih->chipst))) {
			si_pmu_chipcontrol(sih, 6,
				(CC6_4350_PCIE_CLKREQ_WAKEUP_MASK |
				CC6_4350_PMU_WAKEUP_ALPAVAIL_MASK),
				((1 << CC6_4350_PCIE_CLKREQ_WAKEUP_SHIFT) |
				(1 << CC6_4350_PMU_WAKEUP_ALPAVAIL_SHIFT)));
		}

		/* Magic number for chip control 3: 0xF012C33 */
		sr_chipcontrol(sih, ~0, (0x3 << CC3_SR_CLK_SR_MEM_SHIFT |
			0x3 << CC3_SR_MINDIV_FAST_CLK_SHIFT |
			1 << CC3_SR_R23_SR_RISE_EDGE_TRIG_SHIFT |
			1 << CC3_SR_R23_SR_FALL_EDGE_TRIG_SHIFT |
			2 << CC3_SR_NUM_CLK_HIGH_SHIFT |
			1 << CC3_SR_PHY_FUNC_PIC_SHIFT |
			0x7 << CC3_SR_ALLOW_SBC_FUNC_PIC_SHIFT |
			1 << CC3_SR_ALLOW_SBC_STBY_SHIFT));
	} break;

	case BCM43602_CHIP_ID:
		/*
		 * In contrast to 4345/4350/4335 chips, there is no need to touch PMU chip control
		 * 3 and 4 for SR setup.
		 */
		sr_control(sih, 0, 0xFFFFFFFF, (1 << CC_SR_CTL0_EN_SR_ENG_CLK_SHIFT)     |
			(0xc << CC_SR_CTL0_RSRC_TRIGGER_SHIFT)    |
			(3 << CC_SR_CTL0_MIN_DIV_SHIFT)           |
			(1 << CC_SR_CTL0_EN_SBC_STBY_SHIFT)       |
			(1 << CC_SR_CTL0_EN_SR_HT_CLK_SHIFT)      |
			(3 << CC_SR_CTL0_ALLOW_PIC_SHIFT)         |
			(0x10 << CC_SR_CTL0_MAX_SR_LQ_CLK_CNT_SHIFT) |
			(1 << CC_SR_CTL0_EN_MEM_DISABLE_FOR_SLEEP));

		W_REG(sii->osh, &cc->sr_control1, sr_asm_addr(sih)); /* TxFIFO offset */
		break;

	case BCM4349_CHIP_GRPID:
		/*
		 * In contrast to 4345/4350/4335 chips, there is no need to touch PMU chip control
		 * 3 and 4 for SR setup.
		 */
		sr_control(sih, 0, 0xFFFFFFFF, 0x403900F2);

		W_REG(sii->osh, &cc->sr_control1, sr_asm_addr(sih)); /* TxFIFO offset */
		si_pmu_chipcontrol(sih, 2, CC2_4349_VDDM_PWRSW_EN_MASK,
			(1 << CC2_4349_VDDM_PWRSW_EN_SHIFT));

		if (CST4349_CHIPMODE_SDIOD(sih->chipst)) {
			si_pmu_chipcontrol(sih, 2, CC2_4349_SDIO_AOS_WAKEUP_MASK,
				(1 << CC2_4349_SDIO_AOS_WAKEUP_SHIFT));
		} else if (CST4349_CHIPMODE_PCIE(sih->chipst)) {
			si_pmu_chipcontrol(sih, 6,
				(CC6_4349_PCIE_CLKREQ_WAKEUP_MASK |
				CC6_4349_PMU_WAKEUP_ALPAVAIL_MASK),
				((1 << CC6_4349_PCIE_CLKREQ_WAKEUP_SHIFT) |
				(1 << CC6_4349_PMU_WAKEUP_ALPAVAIL_SHIFT)));
		}
		break;

	default:
		break;
	}

	/* Return to previous core */
	si_setcoreidx(sih, origidx);

done:
	INTR_RESTORE(sii, intr_val);
}

#ifdef SR_ESSENTIALS
/**
 * Load the save-restore firmware into the memory for some chips, other chips use a different
 * function.
 */
void
BCMATTACHFN(sr_download_firmware)(si_t *sih)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	uint origidx;
	uint intr_val = 0;
	uint16 i;

	/* Block ints and save current core */
	INTR_OFF(sii, intr_val);
	origidx = si_coreidx(sih);

	switch (CHIPID(sih->chip)) {
	case BCM43242_CHIP_ID:
	case BCM43239_CHIP_ID: {
		sbsocramregs_t *regs;
		uint32 sr_ctrl;
		uint16 count;
		uint32 *sr_source_code = NULL, sr_source_codesz;

		sr_get_source_code_array(sih, &sr_source_code, &sr_source_codesz);

		/* Check if the source code array is properly aligned */
		ASSERT(ISALIGNED(sr_source_codesz, sizeof(uint32)));
		count = (sr_source_codesz/sizeof(uint32));

		/* At least one instruction should be present */
		ASSERT(count > 1);

		/* Switch to SOCRAM core */
		if (!(regs = si_setcore(sih, SOCRAM_CORE_ID, 0)))
			goto done;

		/* Save the current value of sr_control */
		sr_ctrl = R_REG(sii->osh, &regs->sr_control);

		/* Enable access through SR address and data registers */
		W_REG(sii->osh, &regs->sr_control, (sr_ctrl | 0x800));

		/* Write the firmware using auto-increment from location 0 */
		W_REG(sii->osh, &regs->sr_address, 0);
		for (i = 0; i < count; i++) {
			W_REG(sii->osh, &regs->sr_data, sr_source_code[i]);
		}

		/* Bank size expressed as 32-bit words. i.e. ((x << 10) >> 2) */
		count = ((SRCTL43239_BANK_SIZE(sr_ctrl) + 1) << 8);

		/* Total size expreseed as number of 32-bit words */
		/* Add 1 to take care of invisible bank */
		count = (SRCTL43239_BANK_NUM(sr_ctrl) + 1) * count;

		/* Zero out all the other locations. Resume from previous i */
		for (; i < count; i++) {
			W_REG(sii->osh, &regs->sr_data, 0);
		}

		/* Disable access to SR data. Grant access to SR engine */
		W_REG(sii->osh, &regs->sr_control, (sr_ctrl & ~(0x800)));
		break;
	}
	case BCM4335_CHIP_ID:
	case BCM4345_CHIP_ID:
	case BCM4349_CHIP_GRPID:
#ifdef UNRELEASEDCHIP
	case BCM43430_CHIP_ID:
#endif /* UNRELEASEDCHIP */
	case BCM43909_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43602_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
	{
		/* Firmware is downloaded in tx-fifo from wlc.c */
	}
	default:
		break;
	}

	/* Return to previous core */
	si_setcoreidx(sih, origidx);

done:
	INTR_RESTORE(sii, intr_val);
}

/**
 * Either query or enable/disable the save restore engine. Returns a negative value on error.
 * For a query, returns 0 when engine is disabled, positive value when engine is enabled.
 */
int
sr_engine_enable(si_t *sih, bool oper, bool enable)
{
	si_info_t *sii = SI_INFO(sih);
	si_cores_info_t *cores_info = (si_cores_info_t *)sii->cores_info;
	uint origidx;
	uint intr_val = 0;
	int ret_val = 0;

	/* Block ints and save current core */
	INTR_OFF(sii, intr_val);
	origidx = si_coreidx(sih);

	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID: {
		sbsocramregs_t *regs;
		uint32 sr_ctrl;

		/* Switch to SOCRAM core */
		if (!(regs = si_setcore(sih, SOCRAM_CORE_ID, 0)))
			goto done;

		/* Save current value of SR Control */
		sr_ctrl = R_REG(sii->osh, &regs->sr_control);

		/* In 43239A0, save-restore is controlled through sr_control
		 * and certain bits of chipcontrol(phy power down)
		 */
		if (oper == IOV_GET) {
			ret_val = mboolisset(sr_ctrl, (1 << 20));
			if (ret_val != mboolisset(sr_chipcontrol(sih, 0, 0), (0x7 << 24))) {
				SI_ERROR(("%s: Error unknown saverestore state\n", __FUNCTION__));
				ret_val = BCME_ERROR;
				ASSERT(0);
				break;
			}
			ret_val &= mboolisset(sr_chipcontrol(sih, 0, 0), (0x7 << 24));
		} else if (enable) {
			W_REG(sii->osh, &regs->sr_control, sr_ctrl | (1 << 20));
			sr_chipcontrol(sih, (0x7 << 24), (0x7 << 24));
			ret_val = enable;
		} else {
			W_REG(sii->osh, &regs->sr_control, sr_ctrl & ~(1 << 20));
			sr_chipcontrol(sih, (0x7 << 24), 0);
			ret_val = enable;
		}
		break;
	}
	case BCM43340_CHIP_ID:
	case BCM43341_CHIP_ID:
#ifdef UNRELEASEDCHIP
	case BCM43342_CHIP_ID:
	case BCM43430_CHIP_ID:
#endif /* UNRELEASEDCHIP */
	case BCM43909_CHIP_ID:
	case BCM4334_CHIP_ID: {
		chipcregs_t *cc;
		uint32 new_min;

		cc = si_setcoreidx(sih, SI_CC_IDX);
		ASSERT(cc != NULL);

		if (enable == TRUE)
			new_min = sr_get_cur_minresmask(sih);
		else {
			W_REG(sii->osh, &cc->res_table_sel, RES4334_WL_CORE_READY);
			new_min = R_REG(sii->osh, &cc->res_dep_mask);
			new_min |= PMURES_BIT(RES4334_WL_CORE_READY);
		}

		W_REG(sii->osh, &cc->min_res_mask, new_min);
		break;
	}
	case BCM4345_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
	{
		uint8 on;

		/* Use sr_isenab() for IOV_GET */
		ASSERT(oper == IOV_SET);

		on = enable ? 1 : 0;
		sr_chipcontrol(sih, CC3_SR_ENGINE_ENABLE_MASK, (on << CC3_SR_ENGINE_ENABLE_SHIFT));
	} break;
	case BCM4335_CHIP_ID:{
		if (oper == IOV_GET) {
			ret_val = sr_chipcontrol(sih, 0, 0) & 0x7;
			if (ret_val != 0x7) ret_val = 0;
		}
		else if (enable) {
		  /* regctlreg 4 [hexpr [regctlreg 4] | (1<<(159-32*4))] */
		  uint32 tmp;
		  tmp = (1<<(159-32*4));
		  sr_regcontrol4(sih, tmp, tmp);
		  sr_chipcontrol4(sih, 0xc0000000, (0xc0000000));
		  /* bit 21 = MEMLPLDO pwrsw enable */
		  /* chipctlreg 2 [hexpr ( [chipctlreg 2] | (1 << 21))] */
		  sr_chipcontrol2(sih, 0x200000, 0x200000);
		  ret_val = enable;
		} else {
			/* chipctlreg 3 [hexpr ( [chipctlreg 3] & ~0x4)] */
			sr_chipcontrol(sih, 0x4, 0x0);
			ret_val = enable;
		}
		break;
	}
	case BCM43242_CHIP_ID: {
		if (oper == IOV_GET) {
			/* get clk_en, clk_sel and sr_en bits */
			ret_val = sr_chipcontrol(sih, 0, 0) & 0x7;
			if (ret_val != 0x7)
				ret_val = 0;
		} else if (enable) {
			/* set SR-VSDB bits */
			sr_chipcontrol(sih, 0xFFFFF0F, (0x2030007));
			ret_val = enable;
		} else {
			/* reset SR-VSDB bits */
			sr_chipcontrol(sih, 0xFFFFF0F, 0);
			ret_val = enable;
		}
		break;
	}

	case BCM43602_CHIP_ID:
	case BCM4349_CHIP_GRPID:
		if (oper == IOV_GET) {
			ret_val = (sr_control(sih, 0, 0, 0) & CC_SR_CTL0_ENABLE_MASK);
			ret_val >>= CC_SR_CTL0_ENABLE_SHIFT;
		} else {
			si_force_islanding(sih, enable);
			sr_control(sih, 0, CC_SR_CTL0_ENABLE_MASK,
				(enable ? 1: 0) << CC_SR_CTL0_ENABLE_SHIFT);
			ret_val = enable;
		}
		break;

	default:
		ret_val = BCME_UNSUPPORTED;
		break;
	}

	si_setcoreidx(sih, origidx);

done:
	INTR_RESTORE(sii, intr_val);
	return ret_val;
} /* sr_engine_enable */
#endif /* SR_ESSENTIALS */

#if defined(SAVERESTORE)
CONST uint32*
BCMPREATTACHFN(sr_get_sr_params)(si_t *sih, uint32 *arrsz, uint32 *offset)
{
	uint32 addr;
	uint32 *sr_source_code = NULL;

	if (sih->ccrev >= 48) {
		addr = sr_control(sih, 1, 0, 0);
		addr = (addr & CC_SR_CTL1_SR_INIT_MASK) >> CC_SR_CTL1_SR_INIT_SHIFT;
	} else {
		addr = sr_chipcontrol4(sih, 0, 0);
		addr = (addr & CC4_SR_INIT_ADDR_MASK) >> CC4_SR_INIT_ADDR_SHIFT;
	}

	/* convert to txfifo byte offset */
	*offset = addr << 8;

	sr_get_source_code_array(sih, &sr_source_code, arrsz);
	return sr_source_code;
}
#endif /* SAVERESTORE */

#if defined(SAVERESTORE)
void
sr_engine_enable_post_dnld(si_t *sih, bool enable)
{
	switch (CHIPID(sih->chip)) {
	case BCM4345_CHIP_ID:
#ifdef UNRELEASEDCHIP
	case BCM43430_CHIP_ID:
#endif /* UNRELEASEDCHIP */
	case BCM43909_CHIP_ID:
	case BCM43602_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
	case BCM4349_CHIP_GRPID:
	{
		sr_engine_enable(sih, IOV_SET, enable);
	} break;
	case BCM4335_CHIP_ID: {
		si_force_islanding(sih, enable);

		if (enable) {
			/* enable SR */
			/* Don't allow stby from SR (sbc_memStby_allow= bit 27 = 0) */
			/* From RTL sims chipctlreg 3 should have value 0x0F012C37
			 * but it's being changed in driver
			 */
			si_ccreg(sih, CHIPCTRLADDR, ~0, CHIPCTRLREG3);
			si_ccreg(sih, CHIPCTRLDATA, 0x07012C33, 0x07012C33);
			si_ccreg(sih, CHIPCTRLADDR, ~0, CHIPCTRLREG3);
			si_ccreg(sih, CHIPCTRLDATA, 0x4, 0x4);
			/* Increase resource up/dwn time for SR */
			si_ccreg(sih, RSRCTABLEADDR, ~0, 23);
			si_ccreg(sih, RSRCUPDWNTIME, ~0, 0x00200020);
		} else {
			/* SR is disabled */
			/* disable SR */
			si_ccreg(sih, CHIPCTRLADDR, ~0, CHIPCTRLREG3);
			si_ccreg(sih, CHIPCTRLDATA, 0x4, 0x0);
		}
	} break;
	default:
		break;
	}
}
#endif /* SAVERESTORE */

uint32
sr_get_cur_minresmask(si_t *sih)
{
	uint32 minmask = 0;

	switch (CHIPID(sih->chip)) {
	case BCM43340_CHIP_ID:
	case BCM43341_CHIP_ID:
		minmask = PMURES_BIT(RES4334_LPLDO_PU) |
			PMURES_BIT(RES4334_RESET_PULLDN_DIS);

		if (CHIPREV(sih->chiprev) == 0)
			minmask |= PMURES_BIT(RES4334_WL_PWRSW_PU);
		break;
	case BCM4334_CHIP_ID:
		/* retain power switch during deep sleep */
		minmask = (PMURES_BIT(RES4334_WL_PWRSW_PU) |
			PMURES_BIT(RES4334_LPLDO_PU) |
			PMURES_BIT(RES4334_RESET_PULLDN_DIS));
		break;
	case BCM43342_CHIP_ID:
		minmask = (PMURES_BIT(RES4334_LPLDO_PU) |
			PMURES_BIT(RES4334_RESET_PULLDN_DIS));

		/* retain power switch during deep sleep only for 43342a0 */
		if (CHIPREV(sih->chiprev) == 0)
			minmask |= PMURES_BIT(RES4334_WL_PWRSW_PU);
		break;
	default:
		ASSERT(0);
		break;
	}

	#ifdef SR_DEBUG
		/* Override with nvram */
		if (min_sr_mask_valid)
			minmask = nvram_min_sr_mask;
	#endif /* SR_DEBUG */

	return minmask;
}

#ifdef SAVERESTORE
/** return TRUE if the power save variant of save/restore is enabled */
bool
sr_isenab(si_t *sih)
{
	bool enab = FALSE;

	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID:
	case BCM4335_CHIP_ID:
	case BCM4345_CHIP_ID:
#ifdef UNRELEASEDCHIP
	case BCM43430_CHIP_ID:
#endif /* UNRELEASEDCHIP */
	case BCM43909_CHIP_ID:
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
		enab = (sr_chipcontrol(sih, 0, 0) & CC3_SR_ENGINE_ENABLE_MASK) ? TRUE : FALSE;
		break;

	case BCM43602_CHIP_ID: /* add chips here when ccrev >= 48 */
	case BCM4349_CHIP_GRPID:
	enab = ((sr_control(sih, 0, 0, 0) & CC_SR_CTL0_ENABLE_MASK) >>
			CC_SR_CTL0_ENABLE_SHIFT);
		enab = (enab ? TRUE: FALSE);
		break;

	case BCM43242_CHIP_ID: /* fall through, does not support SR-PS */
	default:
		ASSERT(0);
		break;
	}

	return enab;
}
#endif /* SAVERESTORE */

#ifdef SR_ESSENTIALS
/**
 * return TRUE for chips that support the power save variant of save/restore
 */
bool
sr_cap(si_t *sih)
{
	bool cap = FALSE;

	switch (CHIPID(sih->chip)) {
	case BCM43239_CHIP_ID:
	case BCM4345_CHIP_ID:
#ifdef UNRELEASEDCHIP
	case BCM43430_CHIP_ID:
#endif /* UNRELEASEDCHIP */
	case BCM43909_CHIP_ID:
	case BCM4349_CHIP_GRPID:
		cap = TRUE;
		break;
	case BCM4350_CHIP_ID:
	case BCM4354_CHIP_ID:
	case BCM4356_CHIP_ID:
	case BCM43556_CHIP_ID:
	case BCM43558_CHIP_ID:
	case BCM43566_CHIP_ID:
	case BCM43568_CHIP_ID:
	case BCM43569_CHIP_ID:
	case BCM43570_CHIP_ID:
	case BCM4358_CHIP_ID:
		/* 4350A0 is broken */
		if (CHIPREV(sih->chiprev) > 0)
			cap = TRUE;
		break;
	case BCM43242_CHIP_ID: /* does not support SR-PS, return FALSE */
		break;
	case BCM4335_CHIP_ID:
		if (BUSTYPE(sih->bustype) != PCI_BUS)
			cap = TRUE;
		break;
	default:
		break;
	}

	return cap;
}
#endif /* SR_ESSENTIALS */

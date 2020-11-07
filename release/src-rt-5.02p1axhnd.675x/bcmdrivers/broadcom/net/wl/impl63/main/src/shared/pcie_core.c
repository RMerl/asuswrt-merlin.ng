/** @file pcie_core.c
 *
 * Contains PCIe related functions that are shared between different driver models (e.g. firmware
 * builds, DHD builds, BMAC builds), in order to avoid code duplication.
 *
 * Copyright (C) 2020, Broadcom. All Rights Reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *
 * <<Broadcom-WL-IPTag/Open:>>
 *
 * $Id: pcie_core.c 778660 2019-09-06 12:21:21Z $
 */

#include <bcm_cfg.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <bcmdefs.h>
#include <osl.h>
#include <siutils.h>
#include <hndsoc.h>
#include <sbchipc.h>

#include "pcie_core.h"

/* local prototypes */

/* local variables */

/* function definitions */

#ifdef BCMDRIVER

/**
 * XXX: WAR for CRWLPCIEGEN2-163, needed for all the chips at this point.
 * The PCIe core contains a 'snoop bus', that allows the logic in the PCIe core to read and write
 * to the PCIe configuration registers. When chip backplane reset hits, e.g. on driver unload, the
 * pcie snoop out will reset to default values and may get out of sync with pcie config registers.
 * This is causing failures because the LTR enable bit on the snoop bus gets out of sync. Also on
 * the snoop bus are the device power state, MSI info, L1subenable which may potentially cause
 * problems.
 */
void pcie_watchdog_reset(osl_t *osh, si_t *sih, sbpcieregs_t *sbpcieregs)
{
	uint32 val, i, lsc;
	uint32 cfg_offset[] = {PCIECFGREG_STATUS_CMD, PCIECFGREG_PM_CSR,
		PCIECFGREG_MSI_CAP, PCIECFGREG_MSI_ADDR_L,
		PCIECFGREG_MSI_ADDR_H, PCIECFGREG_MSI_DATA,
		PCIECFGREG_LINK_STATUS_CTRL2, PCIECFGREG_RBAR_CTRL,
		PCIECFGREG_PML1_SUB_CTRL1, PCIECFGREG_REG_BAR2_CONFIG,
		PCIECFGREG_REG_BAR3_CONFIG};
	sbpcieregs_t *pcie = NULL;
	uint32 origidx = si_coreidx(sih);

	/* Switch to PCIE2 core */
	pcie = (sbpcieregs_t *)si_setcore(sih, PCIE2_CORE_ID, 0);
	BCM_REFERENCE(pcie);
	ASSERT(pcie != NULL);

	/* Disable/restore ASPM Control to protect the watchdog reset */
	W_REG(osh, &sbpcieregs->configaddr, PCIECFGREG_LINK_STATUS_CTRL);
	lsc = R_REG(osh, &sbpcieregs->configdata);
	val = lsc & (~PCIE_ASPM_ENAB);
	W_REG(osh, &sbpcieregs->configdata, val);

	si_corereg(sih, SI_CC_IDX, OFFSETOF(chipcregs_t, watchdog), ~0, 4);
#ifdef BCMQT
	OSL_DELAY(1000000);
#else
	OSL_DELAY(100000);
#endif /* BCMQT */

	W_REG(osh, &sbpcieregs->configaddr, PCIECFGREG_LINK_STATUS_CTRL);
	W_REG(osh, &sbpcieregs->configdata, lsc);

	if (sih->buscorerev <= 13) {
		/* Write configuration registers back to the shadow registers
		 * cause shadow registers are cleared out after watchdog reset.
		 */
		for (i = 0; i < ARRAYSIZE(cfg_offset); i++) {
			W_REG(osh, &sbpcieregs->configaddr, cfg_offset[i]);
			val = R_REG(osh, &sbpcieregs->configdata);
			W_REG(osh, &sbpcieregs->configdata, val);
		}
	}
	si_setcoreidx(sih, origidx);
}

/* CRWLPCIEGEN2-117 pcie_pipe_Iddq should be controlled
 * by the L12 state from MAC to save power by putting the
 * SerDes analog in IDDQ mode
 */
void  pcie_serdes_iddqdisable(osl_t *osh, si_t *sih, sbpcieregs_t *sbpcieregs)
{
	sbpcieregs_t *pcie = NULL;
	uint crwlpciegen2_117_disable = 0;
	uint32 origidx = si_coreidx(sih);

	crwlpciegen2_117_disable = PCIE_PipeIddqDisable0 | PCIE_PipeIddqDisable1;
	/* Switch to PCIE2 core */
	pcie = (sbpcieregs_t *)si_setcore(sih, PCIE2_CORE_ID, 0);
	BCM_REFERENCE(pcie);
	ASSERT(pcie != NULL);

	OR_REG(osh, &sbpcieregs->control,
		crwlpciegen2_117_disable);

	si_setcoreidx(sih, origidx);
}

void pcie_coherent_accenable(osl_t *osh, si_t *sih)
{
	sbpcieregs_t *pcie = NULL;
	uint32 val;
	uint32 origidx = si_coreidx(sih);

	if ((pcie = si_setcore(sih, PCIE2_CORE_ID, 0)) != NULL) {
		/* PCIe BAR1 and BAR2 coherent access enabled */
		W_REG(osh, &pcie->configaddr, PCIECFGREG_SPROM_CTRL);
		val = R_REG(osh, &pcie->configdata);
		val |= (PCIE_BAR1COHERENTACCEN | PCIE_BAR2COHERENTACCEN);
		W_REG(osh, &pcie->configdata, val);
	}

	si_setcoreidx(sih, origidx);
}

void pcie_getdevctl(osl_t *osh, si_t *sih, uint32 *devctl)
{
	sbpcieregs_t *pcie = NULL;
	uint32 origidx = si_coreidx(sih);

	*devctl = 0;
	if ((pcie = si_setcore(sih, PCIE2_CORE_ID, 0)) != NULL) {
		W_REG(osh, &pcie->configaddr, PCIECFGREG_DEVCONTROL);
		*devctl = R_REG(osh, &pcie->configdata);
	}

	si_setcoreidx(sih, origidx);
}
#endif /* BCMDRIVER */

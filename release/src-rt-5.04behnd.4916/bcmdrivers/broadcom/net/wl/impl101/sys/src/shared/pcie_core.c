/** @file pcie_core.c
 *
 * Contains PCIe related functions that are shared between different driver models (e.g. firmware
 * builds, DHD builds, BMAC builds), in order to avoid code duplication.
 *
 * Copyright (C) 2023, Broadcom. All Rights Reserved.
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
 * $Id: pcie_core.c 827399 2023-07-12 04:12:57Z $
 */

#include <bcm_cfg.h>
#include <typedefs.h>
#include <bcmutils.h>
#include <bcmdefs.h>
#include <osl.h>
#include <siutils.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <bcmdevs.h>
#include <hndpmu.h>

#include "pcie_core.h"

/* local prototypes */

/* local variables */

/* function definitions */

#ifdef BCMDRIVER

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
	uint32 origidx;
#ifdef BCM_ROUTER
	uint32 pmuresetctl = 0;
	/* To retain NIC mode PCIE integrity through a "rmmod wl.ko",
	 * ensure PMU-reset is excluded from the chip reset below.
	 */
	switch (CHIPID(sih->chip)) {
	CASE_BCM6710_CHIP:
	CASE_BCM43684_CHIP:
		pmuresetctl = si_pmu_get_resetcontrol(sih);
		si_pmu_set_resetcontrol(sih, PCTL_RESETCTL_BP_ONLY);
		break;
	default:
		/* Do nothing */
		break;
	}
#endif /* BCM_ROUTER */

	origidx = si_coreidx(sih);
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

#ifdef BCM_ROUTER
	/* Restore PMU reset-control to prior value */
	switch (CHIPID(sih->chip)) {
	CASE_BCM6710_CHIP:
	CASE_BCM43684_CHIP:
		si_pmu_set_resetcontrol(sih, pmuresetctl);
		break;
	default:
		/* Do nothing */
		break;
	}
#endif /* BCM_ROUTER */
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

uint32 pcie_dmatxblen(osl_t *osh, si_t *sih)
{
	uint32 devctl;
	uint32 mrrs, burstlen;

	pcie_getdevctl(osh, sih, &devctl);
	mrrs = (devctl & PCIE_CAP_DEVCTRL_MRRS_MASK) >> PCIE_CAP_DEVCTRL_MRRS_SHIFT;
	switch (mrrs)
	{
		case PCIE_CAP_DEVCTRL_MRRS_128B:
			burstlen = DMA_BL_128;
			break;
		case PCIE_CAP_DEVCTRL_MRRS_256B:
			burstlen = DMA_BL_256;
			break;
		case PCIE_CAP_DEVCTRL_MRRS_512B:
			burstlen = DMA_BL_512;
			break;
		case PCIE_CAP_DEVCTRL_MRRS_1024B:
			burstlen = DMA_BL_1024;
			break;
		default:
			burstlen = DMA_BL_128;
			break;
	}

	if (ISSIM_ENAB(sih) && burstlen > DMA_BL_256) {
		// printf("=== CAUTION: Tx DMA BurstLen in pcie_dmatxblen is downsized "
		//	"from %d to %d per PCIE MRRS ===\n", burstlen, DMA_BL_256);
		burstlen = DMA_BL_256;
	}

	return burstlen;
}

uint32 pcie_dmarxblen(osl_t *osh, si_t *sih)
{
	uint32 devctl;
	uint32 mps, burstlen;

	pcie_getdevctl(osh, sih, &devctl);
	mps = (devctl & PCIE_CAP_DEVCTRL_MPS_MASK) >> PCIE_CAP_DEVCTRL_MPS_SHIFT;
	switch (mps)
	{
		case PCIE_CAP_DEVCTRL_MPS_128B:
			burstlen = DMA_BL_128;
			break;
		case PCIE_CAP_DEVCTRL_MPS_256B:
			burstlen = DMA_BL_256;
			break;
		case PCIE_CAP_DEVCTRL_MPS_512B:
			burstlen = DMA_BL_512;
			break;
		case PCIE_CAP_DEVCTRL_MPS_1024B:
			burstlen = DMA_BL_1024;
			break;
		default:
			burstlen = DMA_BL_128;
			break;
	}

	if (ISSIM_ENAB(sih) && burstlen > DMA_BL_256) {
		// printf("=== CAUTION: Rx DMA BurstLen in pcie_dmarxblen is downsized "
		//	"from %d to %d per PCIE MPS ===\n", burstlen, DMA_BL_256);
		burstlen = DMA_BL_256;
	}

	return burstlen;
}

void pcie_tl_control0(osl_t *osh, si_t *sih)
{
	uint32 val;
	sbpcieregs_t *pcie = NULL;
	uint32 origidx = si_coreidx(sih);

	if ((pcie = si_setcore(sih, PCIE2_CORE_ID, 0)) != NULL) {

		/* Setup MemRd & MemWr burst sizes */
		W_REG(osh, &pcie->configaddr, PCIECFGREG_TL_CTRL0);
		val = R_REG(osh, &pcie->configdata);

		/* Mask away MEMRD/WR_DW_CHK */
		val = val & 0xfffffc01;

		/* BAR0: max 1DW */
		val |= (0x1 << 3) | (0x1 << 1);
		/* BAR1 & BAR2: do not restrict 32DW/128B */
		val &= ~((0x1 << 9) | (0x1 << 8));

		W_REG(osh, &pcie->configdata, val);
	}

	si_setcoreidx(sih, origidx);
}

#endif /* BCMDRIVER */

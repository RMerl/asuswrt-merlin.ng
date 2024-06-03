/*
 * +--------------------------------------------------------------------------+
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
 * <<Broadcom-WL-IPTag/Open:>>
 * PTM (Precision Time Management) Protocol Support
 * $Id:$
 * +--------------------------------------------------------------------------+
 */
#if !defined(WL_MLO)
#error "PTM protocol is used only when MLO is defined."
#endif
#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <hndsoc.h>
#include <siutils.h>
#include <wlc_pub.h>
#include <wlc.h>
#include <wlc_types.h>
#include <wlc_hw.h>
#include <pcie_core.h>
#include <wlc_hw_priv.h>
#include <wlc_ptm.h>
#include <wlc_bmac.h>
#include <pcie_core.h>

static void wlc_ptm_write_cfgreg(wlc_hw_info_t *wlc_hw, uint32 reg, uint32 val32);
static uint32 wlc_ptm_read_cfgreg(wlc_hw_info_t *wlc_hw, uint32 reg);
static int wlc_ptm_validate_ptm(wlc_hw_info_t *wlc_hw);
#if !defined(DONGLEBUILD)
/* wl-nic: integrated radio */
static int wlc_ptm_enable_internal_radio(wlc_hw_info_t *wlc_hw);
#endif /* !DONGLEBUILD */

static uint32
wlc_ptm_read_cfgreg(wlc_hw_info_t *wlc_hw, uint32 reg)
{
	si_t *sih = wlc_hw->sih;
	uint32 val32 = 0;
	sbpcieregs_t *regs = NULL;
	uint origidx;

	origidx = si_coreidx(sih);
	regs = (sbpcieregs_t *)si_setcore(sih, PCIE2_CORE_ID, 0);

	W_REG(wlc_hw->osh, &regs->configaddr, reg);
	val32 = R_REG(wlc_hw->osh, &regs->configdata);

	si_setcoreidx(sih, origidx);

	return val32;
}

static void
wlc_ptm_write_cfgreg(wlc_hw_info_t *wlc_hw, uint32 reg, uint32 val32)
{
	si_t *sih = wlc_hw->sih;
	sbpcieregs_t *regs = NULL;
	uint origidx;

	origidx = si_coreidx(sih);
	regs = (sbpcieregs_t *)si_setcore(sih, PCIE2_CORE_ID, 0);

	W_REG(wlc_hw->osh, &regs->configaddr, reg);
	W_REG(wlc_hw->osh, &regs->configdata, val32);
	val32 = R_REG(wlc_hw->osh, &regs->configdata);

	si_setcoreidx(sih, origidx);

	return;
}

static int
wlc_ptm_validate_ptm(wlc_hw_info_t *wlc_hw)
{
	wlc_pub_t	*wlc_pub = wlc_hw->wlc->pub;
	int status = BCME_OK;
	uint origidx, intr_val;
	int16 v16;

	/* validate PTM counter is running */
	si_switch_core(wlc_hw->sih, D11_CORE_ID, &origidx, &intr_val);

	v16 = wlc_bmac_read_ihr(wlc_hw, (volatile int16 *) D11_TIME1_SNAP(wlc_hw));
	/* confirm D11 PTM valid status */
	if (v16 & D11_TIME1_SNAP_PTM_VALID) {
		status = BCME_OK;
		wlc_pub->ptm_flags |= WL_PTM_FLAG_PTM_ENABLED;
	} else {
		wlc_pub->ptm_flags |= WL_PTM_FLAG_PTM_EANBLE_FAILED;
		status = BCME_ERROR;
	}
	si_restore_core(wlc_hw->sih, origidx, intr_val);
	return status;
}

#if !defined(DONGLEBUILD)
static int
wlc_ptm_enable_internal_radio(wlc_hw_info_t *wlc_hw)
{
	int status = BCME_OK;
#if defined(CONFIG_BCM96764) || defined(CONFIG_BCM96765) || defined(CONFIG_BCM96766)
	ulong reg_pa;
	uint32 *preg;
	int wlcore = osl_pci_slot(wlc_hw->osh)-1;

	/* note : this a write to a host register from end point driver */
	reg_pa = (ulong) CONFIG_WIFI_MLO_CONTROL_ADDR + wlcore*CONFIG_WIFI_MLO_BLOCK_SIZE;
	preg = (uint32 *)ioremap(reg_pa, sizeof(uint32));
	*preg = PTM_VALID;
	iounmap(preg);

	/* delay 1 msec for PTM to populate */
	OSL_DELAY(1000);
#else
	 ASSERT(!"wlc_ptm_enable_internal_radio");
#endif  /* CONFIG_BCM6764 || CONFIG_BCM96765 || CONFIG_BCM96766 */
	return status;
}
#endif /* !DONGLEBUILD */

int
wlc_ptm_enable(wlc_hw_info_t *wlc_hw)
{
	int status = BCME_OK;
	wlc_pub_t *wlc_pub = wlc_hw->wlc->pub;
	pcie_ptm_regs_t *regs;
	uint origidx, intr_val;
#if defined(BCMQT)
	uint32 ptm_interval = 0x6; /* 2^6 = 64usec */
#else
	uint32 ptm_interval = 0xA;  /* 2^10 = 1024usec = 1msec */
#endif
	uint32 threshold = 0xfff;
	uint32	ptm_control = ((ptm_interval << PTM_INTERVAL_SHIFT)
				| (0x2 << PTM_REQ_RETRY_CNT_SHIFT)
				| (0x1 << PTM_MASTER_UPDATE_EN_SHIFT)
				| (0x1 << PTM_ENABLE_SHIFT)
				| (0x1 << PTM_DIALOG_REQ_SHIFT));

	if (wlc_pub->ptm_flags & WL_PTM_FLAG_PTM_ENABLED) {
		return BCME_OK;
	}

	/* ptm is not supported or enable attempt failed */
	if (!(wlc_pub->ptm_flags & WL_PTM_FLAG_PTM_SUPPORTED) ||
		(wlc_pub->ptm_flags & WL_PTM_FLAG_PTM_EANBLE_FAILED)) {
		return BCME_ERROR;
	}

#if !defined(DONGLEBUILD)
	if (BUSTYPE(wlc_hw->sih->bustype) != PCI_BUS) {
		/* wl-nic: integrated radio */
		wlc_ptm_enable_internal_radio(wlc_hw);
		goto validate_ptm;
	}
#endif /* DONGLEBUILD */

	regs = si_switch_core(wlc_hw->sih, PCIE2_CORE_ID, &origidx, &intr_val);
	/* enable PTM timer */
	W_REG(wlc_hw->osh, &regs->ptm_invalidate_thresh, threshold);
	W_REG(wlc_hw->osh, &regs->ptm_control, ptm_control);
	si_restore_core(wlc_hw->sih, origidx, intr_val);
	OSL_DELAY(10000); /* delay 10 msec for PTM handshake */

#if !defined(DONGLEBUILD)
validate_ptm:
#endif /* DONGLEBUILD */
	status = wlc_ptm_validate_ptm(wlc_hw);
	WL_ERROR(("wl%d: Precision Time Managment: %s\n", wlc_pub->unit,
			status == BCME_OK ? "Enabled" : "not Enabled"));
	return status;
}

void
wlc_ptm_init(wlc_hw_info_t *wlc_hw)
{
	wlc_pub_t	*wlc_pub = wlc_hw->wlc->pub;
	uint32	ptm_ctrl;
	uint32	ptm_cap;

	/* d11 rev older than rev for 6726b0 */
	if (D11REV_LE(wlc_hw->corerev, 133))  {
		/* ptm support not available on non-11be radio cores */
		ASSERT(wlc_hw->corerev <= 133);
		wlc_pub->ptm_flags = 0;
		goto done;
	}
#if !defined(DONGLEBUILD)
	if (BUSTYPE(wlc_hw->sih->bustype) != PCI_BUS) {
		/* wl-nic: integrated radios: there is no ptm handshake required */
		wlc_pub->ptm_flags |= WL_PTM_FLAG_PTM_SUPPORTED;
		goto done;
	}
#endif /* !DONGLEBUILD */

	ptm_cap = wlc_ptm_read_cfgreg(wlc_hw, PCIE_CFG_PTM_PTM_CAP_REG);
	if (!ptm_cap & PCIE_CFG_PTM_REQUESTER_CAPABLE)
	{
		WL_ERROR(("%s: wl%d: PCIE_CFG_PTM_CAP_REG PTM_CAPABLE: %x\n",
			__FUNCTION__, wlc_pub->unit, ptm_cap));
		ASSERT(ptm_cap&PCIE_CFG_PTM_REQUESTER_CAPABLE);
		goto done;
	}
	ptm_ctrl = wlc_ptm_read_cfgreg(wlc_hw, PCIE_CFG_PTM_PTM_CTRL_REG);
	ptm_ctrl |= PCIE_CFG_PTM_ENABLE;
	wlc_ptm_write_cfgreg(wlc_hw, PCIE_CFG_PTM_PTM_CTRL_REG, ptm_ctrl);

	ptm_ctrl = wlc_ptm_read_cfgreg(wlc_hw, PCIE_CFG_PTM_PTM_CTRL_REG);

	wlc_pub->ptm_flags |= WL_PTM_FLAG_PTM_SUPPORTED;
done:
	WL_ERROR(("wl%d: Precision Time Managment: %s\n", wlc_pub->unit,
			wlc_pub->ptm_flags ? "Supported" : "not Supported"));
}

#if defined(BCMDBG)
int
wlc_ptm_dump(void *ctx, struct bcmstrbuf *b)
{
	wlc_info_t *wlc = ctx;
	wlc_hw_info_t *wlc_hw = wlc->hw;
	uint origidx, intr_val;
	uint16 v16[8];
	int i;

	bcm_bprintf(b, "PTM flags:0x%02x\n", wlc->pub->ptm_flags);
#if !(defined(CONFIG_BCM96765) || defined(CONFIG_BCM96766))
	pcie_ptm_regs_t *regs;
	uint32 ptm_lo, ptm_hi;

	/* validate PTM counter is incrementing */
	regs = si_switch_core(wlc_hw->sih, PCIE2_CORE_ID, &origidx, &intr_val);
	W_REG(wlc_hw->osh, &regs->ptm_master_time_cmd, 0x1);
	ptm_lo = R_REG(wlc_hw->osh, &regs->ptm_master_time_lo);
	ptm_hi = R_REG(wlc_hw->osh, &regs->ptm_master_time_hi);

	bcm_bprintf(b, "PTM Master Time hi:lo 0x%08x:0x%08x\n", ptm_hi, ptm_lo);
	si_restore_core(wlc_hw->sih, origidx, intr_val);

#endif /* !(CONFIG_BCM96765 || CONFIG_BCM96766) */

	si_switch_core(wlc_hw->sih, D11_CORE_ID, &origidx, &intr_val);
	/* latch tfs, ptm values into registers */
	wlc_bmac_write_ihr(wlc->hw, (volatile int16 *)D11_TIME1_SNAP(wlc_hw),
		(uint16)D11_TIME1_SNAP_SNAPSHOT);
	for (i = 0; i < 8; i++) {
		v16[i] = wlc_bmac_read_ihr(wlc->hw,
			(volatile int16 *)(D11_TIME1_SNAP(wlc_hw) + i * 2));
	}
	bcm_bprintf(b, "D11_TIME1_SNAP TSF: 0x%04x:0x%04x:0x%04x:0x%04x\n",
			v16[3], v16[2], v16[1], v16[0]);
	bcm_bprintf(b, "D11_TIME1_SNAP PTM: 0x%04x:0x%04x:0x%04x:0x%04x\n",
			v16[7], v16[6], v16[5], v16[4]);
	si_restore_core(wlc_hw->sih, origidx, intr_val);
	return 0;
}
#endif

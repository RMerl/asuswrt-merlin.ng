/*
 * ACPHY Miscellaneous modules implementation
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
 * $Id$
 */

#include <phy_cfg.h>
#include <typedefs.h>
#include <bcmdefs.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_misc.h"
#include <phy_ac.h>
#include <phy_ac_misc.h>

/* ************************ */
/* Modules used by this module */
/* ************************ */
#include <wlc_phyreg_ac.h>

#include <phy_utils_reg.h>
#include <phy_ac_info.h>

/* module private states */
struct phy_ac_misc_info {
	phy_info_t *pi;
	phy_ac_info_t *aci;
	phy_misc_info_t *cmn_info;
};

/* local functions */
/*
 * Return vasip version, -1 if not present.
 */
uint8 phy_ac_misc_get_vasip_ver(phy_type_misc_ctx_t *ctx);
/*
 * reset/activate vasip.
 */
void phy_ac_misc_vasip_proc_reset(phy_type_misc_ctx_t *ctx, int reset);

void phy_ac_misc_vasip_clk_set(phy_type_misc_ctx_t *ctx, bool val);
void phy_ac_misc_vasip_bin_write(phy_type_misc_ctx_t *ctx, const uint32 vasip_code[],
	const uint nbytes);
#ifdef VASIP_SPECTRUM_ANALYSIS
void phy_ac_misc_vasip_spectrum_tbl_write(phy_type_misc_ctx_t *ctx,
	const uint32 vasip_spectrum_tbl[], const uint nbytes_tbl);
#endif // endif

void phy_ac_misc_vasip_svmp_write(phy_type_misc_ctx_t *ctx, uint32 offset, uint16 val);
uint16 phy_ac_misc_vasip_svmp_read(phy_type_misc_ctx_t *ctx, uint32 offset);
void phy_ac_misc_vasip_svmp_write_blk(phy_type_misc_ctx_t *ctx, uint32 offset, uint16 len,
	uint16 *val);
void phy_ac_misc_vasip_svmp_read_blk(phy_type_misc_ctx_t *ctx, uint32 offset, uint16 len,
	uint16 *val);

/* register phy type specific implementation */
phy_ac_misc_info_t *
BCMATTACHFN(phy_ac_misc_register_impl)(phy_info_t *pi, phy_ac_info_t *aci,
	phy_misc_info_t *cmn_info)
{
	phy_ac_misc_info_t *ac_info;
	phy_type_misc_fns_t fns;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate all storage together */
	if ((ac_info = phy_malloc(pi, sizeof(phy_ac_misc_info_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}
	ac_info->pi = pi;
	ac_info->aci = aci;
	ac_info->cmn_info = cmn_info;

	/* register PHY type specific implementation */
	bzero(&fns, sizeof(fns));
	fns.ctx = ac_info;
	fns.phy_type_vasip_get_ver = phy_ac_misc_get_vasip_ver;
	fns.phy_type_vasip_proc_reset = phy_ac_misc_vasip_proc_reset;
	fns.phy_type_vasip_clk_set = phy_ac_misc_vasip_clk_set;
	fns.phy_type_vasip_bin_write = phy_ac_misc_vasip_bin_write;
#ifdef VASIP_SPECTRUM_ANALYSIS
	fns.phy_type_vasip_spectrum_tbl_write = phy_ac_misc_vasip_spectrum_tbl_write;
#endif // endif
	fns.phy_type_vasip_svmp_read = phy_ac_misc_vasip_svmp_read;
	fns.phy_type_vasip_svmp_write = phy_ac_misc_vasip_svmp_write;
	fns.phy_type_vasip_svmp_write_blk = phy_ac_misc_vasip_svmp_write_blk;
	fns.phy_type_vasip_svmp_read_blk = phy_ac_misc_vasip_svmp_read_blk;

	if (phy_misc_register_impl(cmn_info, &fns) != BCME_OK) {
		PHY_ERROR(("%s: phy_misc_register_impl failed\n", __FUNCTION__));
		goto fail;
	}

	return ac_info;

	/* error handling */
fail:
	if (ac_info != NULL)
		phy_mfree(pi, ac_info, sizeof(phy_ac_misc_info_t));
	return NULL;
}

void
BCMATTACHFN(phy_ac_misc_unregister_impl)(phy_ac_misc_info_t *ac_info)
{
	phy_info_t *pi;
	phy_misc_info_t *cmn_info;

	ASSERT(ac_info);
	pi = ac_info->pi;
	cmn_info = ac_info->cmn_info;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* unregister from common */
	phy_misc_unregister_impl(cmn_info);

	phy_mfree(pi, ac_info, sizeof(phy_ac_misc_info_t));
}

/* ********************************************* */
/*				External Definitions					*/
/* ********************************************* */
/* enable/disable receiving of LDPC frame */
void
wlc_phy_update_rxldpc_acphy(phy_info_t *pi, bool ldpc)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	if (ldpc != pi_ac->ac_rxldpc_override) {
		pi_ac->ac_rxldpc_override = ldpc;

		MOD_PHYREG(pi, HTSigTones, support_ldpc, (ldpc) ? 1 : 0);
	}
}

void
wlc_phy_force_rfseq_acphy(phy_info_t *pi, uint8 cmd)
{
	uint16 trigger_mask, status_mask;
	uint16 orig_RfseqCoreActv;
	uint8 stall_val, fifo_rst_val = 0;

	switch (cmd) {
	case ACPHY_RFSEQ_RX2TX:
		trigger_mask = ACPHY_RfseqTrigger_rx2tx_MASK(pi->pubpi.phy_rev);
		status_mask = ACPHY_RfseqStatus0_rx2tx_MASK(pi->pubpi.phy_rev);
		break;
	case ACPHY_RFSEQ_TX2RX:
		trigger_mask = ACPHY_RfseqTrigger_tx2rx_MASK(pi->pubpi.phy_rev);
		status_mask = ACPHY_RfseqStatus0_tx2rx_MASK(pi->pubpi.phy_rev);
		break;
	case ACPHY_RFSEQ_RESET2RX:
		trigger_mask = ACPHY_RfseqTrigger_reset2rx_MASK(pi->pubpi.phy_rev);
		status_mask = ACPHY_RfseqStatus0_reset2rx_MASK(pi->pubpi.phy_rev);
		break;
	case ACPHY_RFSEQ_UPDATEGAINH:
		trigger_mask = ACPHY_RfseqTrigger_updategainh_MASK(pi->pubpi.phy_rev);
		status_mask = ACPHY_RfseqStatus0_updategainh_MASK(pi->pubpi.phy_rev);
		break;
	case ACPHY_RFSEQ_UPDATEGAINL:
		trigger_mask = ACPHY_RfseqTrigger_updategainl_MASK(pi->pubpi.phy_rev);
		status_mask = ACPHY_RfseqStatus0_updategainl_MASK(pi->pubpi.phy_rev);
		break;
	case ACPHY_RFSEQ_UPDATEGAINU:
		trigger_mask = ACPHY_RfseqTrigger_updategainu_MASK(pi->pubpi.phy_rev);
		status_mask = ACPHY_RfseqStatus0_updategainu_MASK(pi->pubpi.phy_rev);
		break;
	default:
		PHY_ERROR(("wl%d: %s: Unknown cmd %d\n", pi->sh->unit, __FUNCTION__, cmd));
		return;
	}

	/* Save */
	orig_RfseqCoreActv = READ_PHYREG(pi, RfseqMode);
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	fifo_rst_val = READ_PHYREGFLD(pi, RxFeCtrl1, soft_sdfeFifoReset);
	/* Force gated clocks on */
	wlapi_bmac_phyclk_fgc(pi->sh->physhim, ON);
	if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		W_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param, 0x6); /* set reg(PHY_CTL) 0x6 */

		/* Disable Stalls */
		if (stall_val == 0)
			ACPHY_DISABLE_STALL(pi);
	}

	if (fifo_rst_val == 0)
		MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, 1);
	OSL_DELAY(1);

	/* Trigger */
	phy_utils_or_phyreg(pi, ACPHY_RfseqMode(pi->pubpi.phy_rev),
		(ACPHY_RfseqMode_CoreActv_override_MASK(pi->pubpi.phy_rev) |
		ACPHY_RfseqMode_Trigger_override_MASK(pi->pubpi.phy_rev)));
	phy_utils_or_phyreg(pi, ACPHY_RfseqTrigger(pi->pubpi.phy_rev), trigger_mask);
	SPINWAIT((READ_PHYREG(pi, RfseqStatus0) & status_mask), ACPHY_SPINWAIT_RFSEQ_FORCE);
	ASSERT((READ_PHYREG(pi, RfseqStatus0) & status_mask) == 0);

	/* Restore */
	MOD_PHYREG(pi, RxFeCtrl1, soft_sdfeFifoReset, fifo_rst_val);
	if (ACMAJORREV_32(pi->pubpi.phy_rev) || ACMAJORREV_33(pi->pubpi.phy_rev)) {
		/* Undo Stalls */
		ACPHY_ENABLE_STALL(pi, stall_val);
		OSL_DELAY(1);

		/* Force gated clocks off */
		W_REG(pi->sh->osh, &pi->regs->psm_phy_hdr_param, 0x2); /* set reg(PHY_CTL) 0x2 */
	}
	wlapi_bmac_phyclk_fgc(pi->sh->physhim, OFF);

	WRITE_PHYREG(pi, RfseqMode, orig_RfseqCoreActv);

	if (ACMAJORREV_1(pi->pubpi.phy_rev))
		return;

	ASSERT((READ_PHYREG(pi, RfseqStatus0) & status_mask) == 0);
}

uint16
wlc_phy_classifier_acphy(phy_info_t *pi, uint16 mask, uint16 val)
{
	uint16 curr_ctl, new_ctl;

	PHY_TRACE(("wl%d: %s\n", pi->sh->unit, __FUNCTION__));

	/* Turn on/off classification (bphy, ofdm, and wait_ed), mask and
	 * val are bit fields, bit 0: bphy, bit 1: ofdm, bit 2: wait_ed;
	 * for types corresponding to bits set in mask, apply on/off state
	 * from bits set in val; if no bits set in mask, simply returns
	 * current on/off state.
	 */
	curr_ctl = READ_PHYREG(pi, ClassifierCtrl);

	new_ctl = (curr_ctl & (~mask)) | (val & mask);

	WRITE_PHYREG(pi, ClassifierCtrl, new_ctl);

	return new_ctl;
}

void
wlc_phy_deaf_acphy(phy_info_t *pi, bool mode)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	if (mode) {
	  if (pi_ac->deaf_count == 0)
			wlc_phy_stay_in_carriersearch_acphy(pi, TRUE);
		else
			PHY_ERROR(("%s: Deafness already set\n", __FUNCTION__));
	}
	else {
		if (pi_ac->deaf_count > 0)
			wlc_phy_stay_in_carriersearch_acphy(pi, FALSE);
		else
			PHY_ERROR(("%s: Deafness already cleared\n", __FUNCTION__));
	}
	wlapi_enable_mac(pi->sh->physhim);
}

bool
wlc_phy_get_deaf_acphy(phy_info_t *pi)
{
	uint8 core;
	uint16 curr_classifctl, val;
	bool isDeaf = TRUE;
	/* Get current classifier and clip_detect settings */
	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	curr_classifctl = READ_PHYREG(pi, ClassifierCtrl) & ACPHY_ClassifierCtrl_classifierSel_MASK;
	/* XXX
	 * For deafness to be set, ofdm and cck classifiers must be disabled,
	 * AND adc_clip thresholds must be set to max (0xffff)
	 */
	if (curr_classifctl != 4) {
		isDeaf = FALSE;
	} else {
		if (ACREV_IS(pi->pubpi.phy_rev, 0)) {
			FOREACH_CORE(pi, core) {
				val = READ_PHYREGC(pi, Clip1Threshold, core);
				if (val != 0xffff) {
					isDeaf = FALSE;
					break;
				}
			}
		}
		else {
			FOREACH_CORE(pi, core) {
				val = READ_PHYREGFLDC(pi, computeGainInfo, core,
				                      disableClip1detect);
				if (val != 1) {
					isDeaf = FALSE;
					break;
				}
			}
	        }
	}

	wlapi_enable_mac(pi->sh->physhim);
	return isDeaf;
}

void
wlc_phy_gpiosel_acphy(phy_info_t *pi, uint16 sel, uint8 word_swap)
{
	uint16 save_gpioHiOutEn;

	save_gpioHiOutEn = READ_PHYREG(pi, gpioHiOutEn);
	save_gpioHiOutEn |= 0x8000;

	/* set up acphy GPIO sel */
	WRITE_PHYREG(pi, gpioSel, (word_swap<<8) | sel);
	WRITE_PHYREG(pi, gpioHiOutEn, save_gpioHiOutEn);
}

#if defined(BCMDBG) || defined(WLTEST)
int
wlc_phy_freq_accuracy_acphy(phy_info_t *pi, int channel)
{
	phy_info_acphy_t *pi_ac = (phy_info_acphy_t *)pi->u.pi_acphy;
	int bcmerror = BCME_OK;

	if (channel == 0) {
		wlc_phy_stopplayback_acphy(pi);
		/* restore the old BBconfig, to restore resampler setting */
		WRITE_PHYREG(pi, BBConfig, pi_ac->saved_bbconf);
		WRITE_PHYREG(pi, AfePuCtrl, pi_ac->AfePuCtrl);
		wlc_phy_resetcca_acphy(pi);
	} else {
		/* Disable the re-sampler (in case we are in spur avoidance mode) */
		pi_ac->saved_bbconf = READ_PHYREG(pi, BBConfig);
		pi_ac->AfePuCtrl = READ_PHYREG(pi, AfePuCtrl);

		MOD_PHYREG(pi, AfePuCtrl, tssiSleepEn, 0);
		MOD_PHYREG(pi, bphyTest, dccomp, 0);
		MOD_PHYREG(pi, BBConfig, resample_clk160, 0);
		/* use 151 since that should correspond to nominal tx output power */
		bcmerror = wlc_phy_tx_tone_acphy(pi, 0, 151, 0, 0, TRUE);

	}
	return bcmerror;
}
#endif // endif

#if defined(WLTEST)
void
wlc_phy_test_scraminit_acphy(phy_info_t *pi, int8 init)
{
	/* PR 38226: routine to allow special testmodes where the scrambler is
	 * forced to a fixed init value, hence, the same bit sequence into
	 * the MAC produces the same constellation point sequence every
	 * time
	 */

	if (init < 0) {
		/* auto: clear Mode bit so that scrambler LFSR will be free
		 * running.  ok to leave scramindexctlEn and initState in
		 * whatever current condition, since their contents are unused
		 * when free running.
		 */
		MOD_PHYREG(pi, ScramSigCtrl, scramCtrlMode, 0);
	} else {
		/* fixed init: set Mode bit, clear scramindexctlEn, and write
		 * init to initState, so that scrambler LFSR will be
		 * initialized with specified value for each transmission.
		 */
		MOD_PHYREG(pi, ScramSigCtrl, initStateValue, init);
		MOD_PHYREG(pi, ScramSigCtrl, scramindexctlEn, 0);
		MOD_PHYREG(pi, ScramSigCtrl, scramCtrlMode, 1);
	}
}
#endif // endif

void wlc_acphy_set_scramb_dyn_bw_en(wlc_phy_t *pih, bool enable)
{
	phy_info_t *pi = (phy_info_t *)pih;

	phy_utils_phyreg_enter(pi);
	MOD_PHYREG(pi, NsyncscramInit1, scramb_dyn_bw_en, (enable) ? 1 : 0);
	phy_utils_phyreg_exit(pi);
}

/*
 * Return vasip version, -1 if not present.
 */
uint8
phy_ac_misc_get_vasip_ver(phy_type_misc_ctx_t *ctx)
{
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_acphy_t *pi_ac;

	pi_ac = info->pi->u.pi_acphy;

	return pi_ac->vasipver;
}

/*
 * reset/activate vasip.
 */
void
phy_ac_misc_vasip_proc_reset(phy_type_misc_ctx_t *ctx, int reset)
{
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint32 reset_val = 1;

	if (reset) {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_VASIPREGISTERS, 1, 0xe4, 32, &reset_val);
	} else {
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_VASIPREGISTERS, 1, 0xe0, 32, &reset_val);
	}
}

void
phy_ac_misc_vasip_clk_set(phy_type_misc_ctx_t *ctx, bool val)
{
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;

	MOD_PHYREG(pi, dacClkCtrl, vasipClkEn, val);
}

void
phy_ac_misc_vasip_bin_write(phy_type_misc_ctx_t *ctx, const uint32 vasip_code[], const uint nbytes)
{
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint32 stall_val, mem_id;
	uint32	count;
	uint32 svmp_addr = 0x0;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	mem_id = 0;
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, 0x8000, 32, &mem_id);

	count = (nbytes/sizeof(uint32));
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, count, svmp_addr, 32, &vasip_code[0]);

	/* restore stall value */
	ACPHY_ENABLE_STALL(pi, stall_val);
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}

#ifdef VASIP_SPECTRUM_ANALYSIS
void
phy_ac_misc_vasip_spectrum_tbl_write(phy_type_misc_ctx_t *ctx,
        const uint32 vasip_tbl_code[], const uint nbytes_tbl)
{
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint8  stall_val;
	uint32 mem_id_tbl;
	uint32 count_tbl;
	uint32 offset;
	uint32 count_tbl_total;
	uint32 svmp_tbl_addr = 0x3400; // (0x26800-0x8000*4)>>1

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	mem_id_tbl = 4;
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, 0x8000, 32, &mem_id_tbl);

	count_tbl_total = (nbytes_tbl/sizeof(uint32));

	/* If table cross M4/M5 boundary, have to perform two table writes */
	if ((svmp_tbl_addr + count_tbl_total) > 0x4000) {
		count_tbl = 0x4000 - svmp_tbl_addr;
	} else {
		count_tbl = count_tbl_total;
	}

	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, count_tbl, svmp_tbl_addr, 32,
		&vasip_tbl_code[0]);

	/* remainder */
	offset = count_tbl;
	count_tbl = count_tbl_total - count_tbl;

	if (count_tbl > 0) {
		mem_id_tbl = 5;
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, 0x8000, 32, &mem_id_tbl);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, count_tbl, 0, 32,
			&vasip_tbl_code[offset]);
	}
	/* restore stall value */
	ACPHY_ENABLE_STALL(pi, stall_val);
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}
#endif /* VASIP_SPECTRUM_ANALYSIS */

uint16
phy_ac_misc_vasip_svmp_read(phy_type_misc_ctx_t *ctx, uint32 offset)
{
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint32 tbl_val;
	uint32 stall_val, mem_id, odd_even;

	mem_id = offset/0x8000;
	offset = offset%0x8000;

	odd_even = offset%2;
	offset = offset >> 1;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, 0x8000, 32, &mem_id);
	wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);

	/* restore stall value */
	ACPHY_ENABLE_STALL(pi, stall_val);
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);

	tbl_val = odd_even ? ((tbl_val>> NBITS(uint16)) & 0xffff): (tbl_val & 0xffff);

	return  (uint16) tbl_val;
}

void
phy_ac_misc_vasip_svmp_write(phy_type_misc_ctx_t *ctx, uint32 offset, uint16 val)
{
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint32 tbl_val;
	uint32 stall_val, mem_id, odd_even;

	mem_id = offset/0x8000;
	offset = offset%0x8000;

	odd_even = offset%2;
	offset = offset >> 1;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, 0x8000, 32, &mem_id);
	wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);
	if (odd_even) {
		tbl_val = tbl_val & 0xffff;
		tbl_val = tbl_val | (uint32) (val << NBITS(uint16));
	} else {
		tbl_val = tbl_val & (0xffff << NBITS(uint16));
		tbl_val = tbl_val | (uint32) (val);
	}
	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);

	/* restore stall value */
	ACPHY_ENABLE_STALL(pi, stall_val);
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}

void
phy_ac_misc_vasip_svmp_read_blk(phy_type_misc_ctx_t *ctx, uint32 offset, uint16 len, uint16 *val)
{
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint32 tbl_val;
	uint8 stall_val;
	uint32 mem_id;
	uint16 n, odd_start, odd_end;

	mem_id = offset / 0x8000;
	offset = offset % 0x8000;

	odd_start = offset % 2;
	odd_end = (offset + len) % 2;

	offset = offset >> 1;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, 0x8000, 32, &mem_id);

	if (odd_start == 1) {
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);
		val[0] = ((tbl_val >> NBITS(uint16)) & 0xffff);
		offset += 1;
	}
	for (n = odd_start; n < (len - odd_start - odd_end); n += 2) {
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);
		val[n]   = (tbl_val & 0xffff);
		val[n+1] = ((tbl_val >> NBITS(uint16)) & 0xffff);
		offset += 1;
	}
	if (odd_end == 1) {
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);
		val[len-1] = (tbl_val & 0xffff);
	}

	/* restore stall value */
	ACPHY_ENABLE_STALL(pi, stall_val);
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}

void
phy_ac_misc_vasip_svmp_write_blk(phy_type_misc_ctx_t *ctx, uint32 offset, uint16 len, uint16 *val)
{
	phy_ac_misc_info_t *info = (phy_ac_misc_info_t *)ctx;
	phy_info_t *pi = info->pi;
	uint32 tbl_val;
	uint8  stall_val;
	uint32 mem_id;
	uint16 n, odd_start, odd_end;

	mem_id = offset / 0x8000;
	offset = offset % 0x8000;

	odd_start = offset % 2;
	odd_end = (offset + len) % 2;

	offset = offset >> 1;

	wlapi_suspend_mac_and_wait(pi->sh->physhim);
	phy_utils_phyreg_enter(pi);
	stall_val = READ_PHYREGFLD(pi, RxFeCtrl1, disable_stalls);
	ACPHY_DISABLE_STALL(pi);

	wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, 0x8000, 32, &mem_id);

	if (odd_start == 1) {
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);
		tbl_val &= 0xffff;
		tbl_val |= ((uint32)val[0] << NBITS(uint16));
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);
		offset += 1;
	}
	for (n = odd_start; n < (len - odd_start - odd_end); n += 2) {
		tbl_val  = ((uint32)val[n+1] << NBITS(uint16));
		tbl_val |= ((uint32)val[n]);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);
		offset += 1;
	}
	if (odd_end == 1) {
		wlc_phy_table_read_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);
		tbl_val &= (0xffff << NBITS(uint16));
		tbl_val |= ((uint32)val[len-1]);
		wlc_phy_table_write_acphy(pi, ACPHY_TBL_ID_SVMPMEMS, 1, offset, 32, &tbl_val);
	}

	/* restore stall value */
	ACPHY_ENABLE_STALL(pi, stall_val);
	phy_utils_phyreg_exit(pi);
	wlapi_enable_mac(pi->sh->physhim);
}

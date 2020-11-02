/*
 * Sample Collect module implementation.
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
#include <bcmutils.h>
#include <bcmendian.h>
#include <phy_dbg.h>
#include <phy_mem.h>
#include "phy_type_samp.h"
#include <phy_rstr.h>
#include <phy_samp.h>

/* forward declaration */
typedef struct phy_samp_mem phy_samp_mem_t;

/* module private states */
struct phy_samp_info {
	phy_info_t 		*pi;	/* PHY info ptr */
	phy_type_samp_fns_t	*fns;	/* PHY specific function ptrs */
	phy_samp_mem_t		*mem;	/* Memory layout ptr */
};

/* module private states memory layout */
struct phy_samp_mem {
	phy_samp_info_t		cmn_info;
	phy_type_samp_fns_t	fns;
/* add other variable size variables here at the end */
};

/* local function declaration */

/* attach/detach */
phy_samp_info_t *
BCMATTACHFN(phy_samp_attach)(phy_info_t *pi)
{
	phy_samp_mem_t	*mem = NULL;
	phy_samp_info_t	*cmn_info = NULL;

	PHY_TRACE(("%s\n", __FUNCTION__));

	/* allocate attach info storage */
	if ((mem = phy_malloc(pi, sizeof(phy_samp_mem_t))) == NULL) {
		PHY_ERROR(("%s: phy_malloc failed\n", __FUNCTION__));
		goto fail;
	}

	/* Initialize infra params */
	cmn_info = &(mem->cmn_info);
	cmn_info->pi = pi;
	cmn_info->fns = &(mem->fns);
	cmn_info->mem = mem;

	/* Initialize samp params */

	/* Register callbacks */

	return cmn_info;

	/* error */
fail:
	phy_samp_detach(cmn_info);
	return NULL;
}

void
BCMATTACHFN(phy_samp_detach)(phy_samp_info_t *cmn_info)
{
	phy_samp_mem_t *mem;

	PHY_CAL(("%s\n", __FUNCTION__));

	/* Clean up module related states */

	/* Clean up infra related states */
	if (!cmn_info)
		return;

	mem = cmn_info->mem;

	if (mem == NULL) {
		PHY_INFORM(("%s: null samp module\n", __FUNCTION__));
		return;
	}

	phy_mfree(cmn_info->pi, mem, sizeof(phy_samp_mem_t));
}

/* register phy type specific implementations */
int
BCMATTACHFN(phy_samp_register_impl)(phy_samp_info_t *cmn_info, phy_type_samp_fns_t *fns)
{
	PHY_TRACE(("%s\n", __FUNCTION__));

	*cmn_info->fns = *fns;

	return BCME_OK;
}

void
BCMATTACHFN(phy_samp_unregister_impl)(phy_samp_info_t *cmn_info)
{
	PHY_TRACE(("%s\n", __FUNCTION__));
}

/* driver down processing */
int
phy_samp_down(phy_samp_info_t *cmn_info)
{
	int callbacks = 0;

	PHY_TRACE(("%s\n", __FUNCTION__));

	return callbacks;
}

#ifdef SAMPLE_COLLECT

/* ******************************************** */
/*		Internal Definitions		*/
/* ******************************************** */

/*
 * A function to do sample collect
 * Parameters for NPHY (with NREV < 7) are ----
 * coll_us: collection time (in us).
 * cores: 0, 1 or -1 for using core 0, 1 or both in 40MHz (ignored in 20MHz).
 * Parameters for NPHY (with NREV >= 7) are ----
 * mode: sample collect type.
 * trigger: sample collect trigger bitmap.
 * post_dur: post-trigger collection time (in us).
 * For NPHY with (NREV < 7), we have the following
 * 	If BW=20MHz, both cores are sampled simultaneously and the returned buffer
 * 	has the structure [(uint16)num_bytes, rxpower(core0), rxpower(core1),
 * 	I0(core0), Q0(core0), I0(core1), Q0(core1),...].
 * 	If BW=40MHz, cores are sampled sequentially and the returned buffer has the
 * 	structure [(uint16)num_bytes(core0), rxpower(core0), rxpower(core1),
 * 	I0(core0), Q0(core0),...,(uint16)num_bytes(core1), rxpower(core0), rxpower(core1),
 * 	I0(core1), Q0(core1),...].
 * 	In 20MHz the sample frequency is 20MHz and in 40MHz the sample frequency is 40MHz.
 * For HTPHY, ???
 */
static int
phy_sample_collect(phy_samp_info_t *sampi, wl_samplecollect_args_t *collect, void *b)
{
	int status;
	phy_info_t *pi = sampi->pi;
	phy_type_samp_fns_t *fns = sampi->fns;

	/* driver must be "out" (not up, but chip is alive)
	 * if (pi->sh->up)
	 * 	return BCME_NOTDOWN;
	 * if (!pi->sh->clk)
	 * 	return BCME_NOCLK;
	 */

	if (ISACPHY(pi))
	{
		ASSERT(fns->samp_collect != NULL);
		status = ((fns->samp_collect)(fns->ctx, collect, (uint32 *)b));
	}
	else if (ISNPHY(pi))
		status = phy_n_sample_collect(pi, collect, (uint32 *)b);
	else if (ISLCN40PHY(pi))
		status = phy_lcn40_sample_collect(pi, collect, (uint32 *)b);
	else if (ISHTPHY(pi))
		status = phy_ht_sample_collect(pi, collect, (uint32 *)b);
	else
		status = BCME_UNSUPPORTED;

	return status;
}

static int
phy_mac_triggered_sample_collect(phy_samp_info_t *sampi, wl_samplecollect_args_t *collect, void *b)
{
	phy_info_t *pi = sampi->pi;
	if (ISNPHY(pi) && (NREV_GT(pi->pubpi.phy_rev, 7)))
		return phy_n_mac_triggered_sample_collect(pi, collect, (uint32 *)b);
	else
		return BCME_UNSUPPORTED;
}

static int
phy_sample_data(phy_samp_info_t *sampi, wl_sampledata_t *sample_data, void *b)
{
	int status = BCME_UNSUPPORTED;
	phy_info_t *pi = sampi->pi;
	phy_type_samp_fns_t *fns = sampi->fns;

	/* driver must be "out" (not up but chip is alive)
	 *	if (pi->sh->up)
	 *       	return BCME_NOTDOWN;
	 *      if (!pi->sh->clk)
	 *       	return BCME_NOCLK;
	 */

	if (ISACPHY(pi))
	{
		ASSERT(fns->samp_data != NULL);
		status = ((fns->samp_data)(fns->ctx, sample_data, (uint32 *) b));
	}
	else if (ISHTPHY(pi))
		status = phy_ht_sample_data(pi, sample_data, b);
	else if (ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 7))
		status = phy_n_sample_data(pi, sample_data, b);
	else
		status = BCME_UNSUPPORTED;

	return status;
}

static int
phy_mac_triggered_sample_data(phy_samp_info_t *sampi, wl_sampledata_t *sample_data, void *b)
{
	phy_info_t *pi = sampi->pi;

	if (ISNPHY(pi) && NREV_GE(pi->pubpi.phy_rev, 7))
		return phy_n_mac_triggered_sample_data(pi, sample_data, b);
	else
		return BCME_UNSUPPORTED;
}

/* ******************************************** */
/*		External Definitions		*/
/* ******************************************** */
int
phy_iovars_sample_collect(phy_info_t *pi, uint32 actionid, uint16 type, void *p, uint plen,
	void *a, int alen, int vsize)
{
	int32 int_val = 0;
	int err = BCME_OK;
	int32 *ret_int_ptr = (int32 *)a;

	if (plen >= (uint)sizeof(int_val))
		bcopy(p, &int_val, sizeof(int_val));

	BCM_REFERENCE(*ret_int_ptr);

	switch (actionid) {
	case IOV_SVAL(IOV_PHY_SAMPLE_COLLECT_GAIN_ADJUST):
		if (ISNPHY(pi))
			phy_n_sample_collect_gainadj(pi, (int8)int_val, TRUE);
		else if (ISLCN40PHY(pi))
			phy_lcn40_sample_collect_gainadj(pi, (int8)int_val, TRUE);
		else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_GVAL(IOV_PHY_SAMPLE_COLLECT_GAIN_ADJUST):
		if (ISNPHY(pi))
			*ret_int_ptr = (int32)phy_n_sample_collect_gainadj(pi,
				(int8)int_val, FALSE);
		else if (ISLCN40PHY(pi))
			*ret_int_ptr = (int32)phy_lcn40_sample_collect_gainadj(pi,
				(int8)int_val, FALSE);
		else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_SVAL(IOV_PHY_SAMPLE_COLLECT_GAIN_INDEX):
		if (ISLCN40PHY(pi))
			phy_lcn40_sample_collect_gainidx(pi, (uint8)int_val, TRUE);
		else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_GVAL(IOV_PHY_SAMPLE_COLLECT_GAIN_INDEX):
		if (ISLCN40PHY(pi))
			*ret_int_ptr = (int32)phy_lcn40_sample_collect_gainidx(pi,
				(uint8)int_val, FALSE);
		else
			err = BCME_UNSUPPORTED;
		break;

	case IOV_GVAL(IOV_PHY_SAMPLE_COLLECT):
	{
		wl_samplecollect_args_t samplecollect_args;
		if (plen < (int)sizeof(wl_samplecollect_args_t)) {
			PHY_ERROR(("plen (%d) < sizeof(wl_samplecollect_args_t) (%d)\n",
				plen, (int)sizeof(wl_samplecollect_args_t)));
			err = BCME_BUFTOOSHORT;
			break;
		}
		bcopy((char*)p, (char*)&samplecollect_args, sizeof(wl_samplecollect_args_t));
		if (samplecollect_args.version != WL_SAMPLECOLLECT_T_VERSION) {
			PHY_ERROR(("Incompatible version; use %d expected version %d\n",
				samplecollect_args.version, WL_SAMPLECOLLECT_T_VERSION));
			err = BCME_BADARG;
			break;
		}
		if (!ISLCN40PHY(pi)) {
			if (ltoh16(samplecollect_args.length) > (uint16)alen) {
				PHY_ERROR(("Bad length, length requested > buf len (%d > %d)\n",
					samplecollect_args.length, alen));
				err = BCME_BADLEN;
				break;
			}
		} else {
			if (samplecollect_args.nsamps > ((uint16)alen >> 2)) {
				PHY_ERROR(("Bad length, length requested > buf len (%d > %d)\n",
					samplecollect_args.nsamps, alen));
				err = BCME_BADLEN;
				break;
			}
		}
		err = phy_sample_collect(pi->sampi, &samplecollect_args, a);
		break;
	}
	case IOV_GVAL(IOV_PHY_MAC_TRIGGERED_SAMPLE_COLLECT):
	{
		wl_samplecollect_args_t samplecollect_args;
		if (plen < (int)sizeof(wl_samplecollect_args_t)) {
			PHY_ERROR(("plen (%d) < sizeof(wl_samplecollect_args_t) (%d)\n",
				plen, (int)sizeof(wl_samplecollect_args_t)));
			err = BCME_BUFTOOSHORT;
			break;
		}
		bcopy((char*)p, (char*)&samplecollect_args, sizeof(wl_samplecollect_args_t));
		if (samplecollect_args.version != WL_SAMPLECOLLECT_T_VERSION) {
			PHY_ERROR(("Incompatible version; use %d expected version %d\n",
				samplecollect_args.version, WL_SAMPLECOLLECT_T_VERSION));
			err = BCME_BADARG;
			break;
		}
		if (!ISLCN40PHY(pi)) {
			if (ltoh16(samplecollect_args.length) > (uint16)alen) {
				PHY_ERROR(("Bad length, length requested > buf len (%d > %d)\n",
					samplecollect_args.length, alen));
				err = BCME_BADLEN;
				break;
			}
		} else {
			if (samplecollect_args.nsamps > ((uint16)alen >> 2)) {
				PHY_ERROR(("Bad length, length requested > buf len (%d > %d)\n",
					samplecollect_args.nsamps, alen));
				err = BCME_BADLEN;
				break;
			}
		}
		err = phy_mac_triggered_sample_collect(pi->sampi, &samplecollect_args, a);
		break;
	}
	case IOV_GVAL(IOV_PHY_SAMPLE_DATA):
	{
		wl_sampledata_t sampledata_args;
		if (plen < (int)sizeof(wl_sampledata_t)) {
			PHY_ERROR(("plen (%d) < sizeof(wl_samplecollect_args_t) (%d)\n",
				plen, (int)sizeof(wl_sampledata_t)));
			err = BCME_BUFTOOSHORT;
			break;
		}
		bcopy((char*)p, (char*)&sampledata_args, sizeof(wl_sampledata_t));

		if ((ltoh16(sampledata_args.version) != WL_SAMPLEDATA_T_VERSION) &&
		     (ltoh16(sampledata_args.version) != WL_SAMPLEDATA_T_VERSION_SPEC_AN)) {
			PHY_ERROR(("Incompatible version; use %d expected version %d\n",
				sampledata_args.version, WL_SAMPLEDATA_T_VERSION));
			err = BCME_BADARG;
			break;
		}

		if (ltoh16(sampledata_args.length) > (uint16)alen) {
			PHY_TRACE(("length requested > buf len (%d > %d) limiting to buf len\n",
				sampledata_args.length, alen));
			/* limit the user request to alen */
			sampledata_args.length = htol16((uint16)alen);
		}
		err = phy_sample_data(pi->sampi, &sampledata_args, a);
		break;
	}

	case IOV_GVAL(IOV_IQ_IMBALANCE_METRIC_DATA):
	{
		/* driver must be "out" (not up but chip is alive) */
		if (pi->sh->up) {
			err = BCME_NOTDOWN;
			break;
		}
		if (!pi->sh->clk) {
			err = BCME_NOCLK;
			break;
		}

		if (ISLCN40PHY(pi))
			err = phy_lcn40_iqimb_check(pi, 2048, (uint32 *)a, NULL, NULL);
		else
			err = BCME_UNSUPPORTED;
		break;
	}

	case IOV_GVAL(IOV_IQ_IMBALANCE_METRIC):
	{
		/* driver must be "out" (not up but chip is alive) */
		if (pi->sh->up) {
			err = BCME_NOTDOWN;
			break;
		}
		if (!pi->sh->clk) {
			err = BCME_NOCLK;
			break;
		}

		if (ISLCN40PHY(pi))
			err = phy_lcn40_iqimb_check(pi, 8000, NULL, ret_int_ptr, NULL);
		else
			err = BCME_UNSUPPORTED;
		break;
	}

	case IOV_GVAL(IOV_IQ_IMBALANCE_METRIC_PASS):
	{
		/* driver must be "out" (not up but chip is alive) */
		if (pi->sh->up) {
			err = BCME_NOTDOWN;
			break;
		}
		if (!pi->sh->clk) {
			err = BCME_NOCLK;
			break;
		}

		if (ISLCN40PHY(pi))
			err = phy_lcn40_iqimb_check(pi, 8000, NULL, NULL, ret_int_ptr);
		else
			err = BCME_UNSUPPORTED;
		break;
	}

	case IOV_GVAL(IOV_PHY_MAC_TRIGGERED_SAMPLE_DATA):
	{
		wl_sampledata_t sampledata_args;

		if (plen < (int)sizeof(wl_sampledata_t)) {
			PHY_ERROR(("plen (%d) < sizeof(wl_samplecollect_args_t) (%d)\n",
				plen, (int)sizeof(wl_sampledata_t)));
			err = BCME_BUFTOOSHORT;
			break;
		}

		bcopy((char*)p, (char*)&sampledata_args, sizeof(wl_sampledata_t));

		if ((ltoh16(sampledata_args.version) != WL_SAMPLEDATA_T_VERSION) &&
		     (ltoh16(sampledata_args.version) != WL_SAMPLEDATA_T_VERSION_SPEC_AN)) {
			PHY_ERROR(("Incompatible version; use %d expected version %d\n",
				sampledata_args.version, WL_SAMPLEDATA_T_VERSION));
			err = BCME_BADARG;
			break;
		}

		if (ltoh16(sampledata_args.length) > (uint16)alen) {
			PHY_TRACE(("length requested > buf len (%d > %d) limiting to buf len\n",
				sampledata_args.length, alen));
			/* limit the user request to alen */
			sampledata_args.length = htol16((uint16)alen);
		}
		err = phy_mac_triggered_sample_data(pi->sampi, &sampledata_args, a);
		break;
	}
	default:
		err = BCME_UNSUPPORTED;
		break;
	}
	return err;
}
#endif /* SAMPLE_COLLECT */

/*
 * Sample Collect module interface (to other PHY modules).
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

#ifndef _phy_samp_h_
#define _phy_samp_h_

#include <phy_api.h>

/* forward declaration */
typedef struct phy_samp_info phy_samp_info_t;

/* attach/detach */
phy_samp_info_t *phy_samp_attach(phy_info_t *pi);
void phy_samp_detach(phy_samp_info_t *cmn_info);

/* up/down */
int phy_samp_init(phy_samp_info_t *cmn_info);
int phy_samp_down(phy_samp_info_t *cmn_info);

/* MAC based sample play regs */
#define PHYREF_SampleCollectCurPtr	u.d11acregs.SampleCollectCurPtr
#define PHYREF_SaveRestoreStartPtr	u.d11acregs.SaveRestoreStartPtr
#define PHYREF_SampleCollectStopPtr	u.d11acregs.SampleCollectStopPtr
#define PHYREF_SampleCollectStartPtr	u.d11acregs.SampleCollectStartPtr
#define PHYREF_SampleCollectPlayCtrl	u.d11acregs.SampleCollectPlayCtrl
#define PHYREF_SampleCollectCurPtrHigh	u.d11acregs.SampleCollectCurPtrHigh
#define PHYREF_SampleCollectPlayPtrHigh	u.d11acregs.SampleCollectPlayPtrHigh

/* bitfields in PhyCtrl (IHR Address 0x049) */
#define PHYCTRL_SAMPLEPLAYSTART_SHIFT 11
#define PHYCTRL_MACPHYFORCEGATEDCLKSON_SHIFT 1

/* bitfields in SampleCollectPlayCtrl | Applicable to (d11rev >= 53) and (d11rev == 50) */
#define SAMPLE_COLLECT_PLAY_CTRL_PLAY_START_SHIFT 9

/* ****************************************** */
/* CMN Layer sample collect Public API's      */
/* ****************************************** */
#ifdef SAMPLE_COLLECT
extern int phy_iovars_sample_collect(phy_info_t *pi, uint32 actionid, uint16 type, void *p,
	uint plen, void *a, int alen, int vsize);

/* ************************* */
/* phytype export prototypes */
/* ************************* */

/* HTPHY */
extern int phy_ht_sample_data(phy_info_t *pi, wl_sampledata_t *p, void *b);
extern int phy_ht_sample_collect(phy_info_t *pi, wl_samplecollect_args_t *p, uint32 *b);

/* NPHY */
extern int8 phy_n_sample_collect_gainadj(phy_info_t *pi, int8 gainadj, bool set);
extern int phy_n_sample_data(phy_info_t *pi, wl_sampledata_t *p, void *b);
extern int phy_n_sample_collect(phy_info_t *pi,	wl_samplecollect_args_t *p, uint32 *b);
extern int phy_n_mac_triggered_sample_data(phy_info_t *pi, wl_sampledata_t *p, void *b);
extern int phy_n_mac_triggered_sample_collect(phy_info_t *pi, wl_samplecollect_args_t *p,
	uint32 *b);

/* LCN40PHY */
extern int phy_lcn40_sample_collect(phy_info_t *pi, wl_samplecollect_args_t *collect,
	uint32 *buf);
extern int8 phy_lcn40_sample_collect_gainadj(phy_info_t *pi, int8 gainadj, bool set);
extern uint8 phy_lcn40_sample_collect_gainidx(phy_info_t *pi, uint8 gainidx, bool set);
extern int phy_lcn40_iqimb_check(phy_info_t *pi, uint32 nsamps, uint32 *buf, int32 *metric,
	int32 *result);
#endif /* SAMPLE_COLLECT */

#endif /* _phy_samp_h_ */

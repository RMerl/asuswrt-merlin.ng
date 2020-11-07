/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

    Unless you and Broadcom execute a separate written software license
    agreement governing use of this software, this software is licensed
    to you under the terms of the GNU General Public License version 2
    (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php,
    with the following added to such license:

       As a special exception, the copyright holders of this software give
       you permission to link this software with independent modules, and
       to copy and distribute the resulting executable under terms of your
       choice, provided that you also meet, for each linked independent
       module, the terms and conditions of the license of that module.
       An independent module is a module which is not derived from this
       software.  The special exception does not apply to any modifications
       of the software.

    Not withstanding the above, under no circumstances may you combine
    this software in any way with any other Broadcom software provided
    under a license other than the GPL, without Broadcom's express prior
    written consent.

:>
*/

#ifndef _DRV_EPON_EPN_AG_H_
#define _DRV_EPON_EPN_AG_H_

#include "access_macros.h"
#if !defined(_CFE_)
#include "bdmf_interface.h"
#else
#include "bdmf_data_types.h"
#include "bdmf_errno.h"
#endif
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
typedef struct
{
    bdmf_boolean cfgen1588ts;
    bdmf_boolean cfgreplaceupfcs;
    bdmf_boolean cfgappendupfcs;
    bdmf_boolean cfgdropscb;
    bdmf_boolean moduncappedreportlimit;
    bdmf_boolean modmpquesetfirst;
    bdmf_boolean prvlocalmpcppropagation;
    bdmf_boolean prvtekmodeprefetch;
    bdmf_boolean prvincnonzeroaccum;
    bdmf_boolean prvnounmapppedfcs;
    bdmf_boolean prvsupressdiscen;
    bdmf_boolean cfgvlanmax;
    bdmf_boolean fcserronlydatafr;
    bdmf_boolean prvdropunmapppedllid;
    bdmf_boolean prvsuppressllidmodebit;
    bdmf_boolean moddiscoverydafilteren;
    uint8_t rptselect;
    bdmf_boolean prvdisablesvaquestatusbp;
    bdmf_boolean utxloopback;
    bdmf_boolean utxen;
    bdmf_boolean utxrst_pre_n;
    bdmf_boolean cfgdisabledns;
    bdmf_boolean drxloopback;
    bdmf_boolean drxen;
    bdmf_boolean drxrst_pre_n;
} epn_control_0;

typedef struct
{
    bdmf_boolean cfgdisablempcpcorrectiondithering;
    bdmf_boolean prvoverlappedgntenable;
    bdmf_boolean rstmisalignthr;
    bdmf_boolean cfgstalegntchk;
    bdmf_boolean fecrpten;
    bdmf_boolean cfgl1l2truestrict;
    uint8_t cfgctcrpt;
    bdmf_boolean cfgtscorrdis;
    bdmf_boolean cfgnodiscrpt;
    bdmf_boolean disablediscscale;
    bdmf_boolean clronrd;
} epn_control_1;

typedef struct
{
    bdmf_boolean nullrptpri15;
    bdmf_boolean nullrptpri14;
    bdmf_boolean nullrptpri13;
    bdmf_boolean nullrptpri12;
    bdmf_boolean nullrptpri11;
    bdmf_boolean nullrptpri10;
    bdmf_boolean nullrptpri9;
    bdmf_boolean nullrptpri8;
    bdmf_boolean nullrptpri7;
    bdmf_boolean nullrptpri6;
    bdmf_boolean nullrptpri5;
    bdmf_boolean nullrptpri4;
    bdmf_boolean nullrptpri3;
    bdmf_boolean nullrptpri2;
    bdmf_boolean nullrptpri1;
    bdmf_boolean nullrptpri0;
} epn_reset_rpt_pri;

typedef struct
{
    bdmf_boolean intbbhupfrabort;
    bdmf_boolean intcol2sburstcapoverflowpres;
    bdmf_boolean intcoemptyrpt;
    bdmf_boolean intcodrxerrabortpres;
    bdmf_boolean intl2sfifooverrun;
    bdmf_boolean intco1588tsint;
    bdmf_boolean intcorptpres;
    bdmf_boolean intcogntpres;
    bdmf_boolean intcodelstalegnt;
    bdmf_boolean intcogntnonpoll;
    bdmf_boolean intcogntmisalign;
    bdmf_boolean intcognttoofar;
    bdmf_boolean intcogntinterval;
    bdmf_boolean intcogntdiscovery;
    bdmf_boolean intcogntmissabort;
    bdmf_boolean intcogntfullabort;
    bdmf_boolean intbadupfrlen;
    bdmf_boolean intuptardypacket;
    bdmf_boolean intuprptfrxmt;
    bdmf_boolean intbififooverrun;
    bdmf_boolean intburstgnttoobig;
    bdmf_boolean intwrgnttoobig;
    bdmf_boolean intrcvgnttoobig;
    bdmf_boolean intdnstatsoverrun;
    bdmf_boolean intupstatsoverrun;
    bdmf_boolean intdnoutoforder;
    bdmf_boolean inttruantbbhhalt;
    bdmf_boolean intupinvldgntlen;
    bdmf_boolean intcobbhupsfault;
    bdmf_boolean intdntimeinsync;
    bdmf_boolean intdntimenotinsync;
    bdmf_boolean intdportrdy;
} epn_main_int_status;

typedef struct
{
    bdmf_boolean bbhupfrabortmask;
    bdmf_boolean intl2sburstcapoverflowmask;
    bdmf_boolean intcoemptyrptmask;
    bdmf_boolean intdrxerrabortmask;
    bdmf_boolean intl2sfifooverrunmask;
    bdmf_boolean intco1588tsmask;
    bdmf_boolean intcorptpresmask;
    bdmf_boolean intcogntpresmask;
    bdmf_boolean intcodelstalegntmask;
    bdmf_boolean intcogntnonpollmask;
    bdmf_boolean intcogntmisalignmask;
    bdmf_boolean intcognttoofarmask;
    bdmf_boolean intcogntintervalmask;
    bdmf_boolean intcogntdiscoverymask;
    bdmf_boolean intcogntmissabortmask;
    bdmf_boolean intcogntfullabortmask;
    bdmf_boolean badupfrlenmask;
    bdmf_boolean uptardypacketmask;
    bdmf_boolean uprptfrxmtmask;
    bdmf_boolean intbififooverrunmask;
    bdmf_boolean burstgnttoobigmask;
    bdmf_boolean wrgnttoobigmask;
    bdmf_boolean rcvgnttoobigmask;
    bdmf_boolean dnstatsoverrunmask;
    bdmf_boolean intupstatsoverrunmask;
    bdmf_boolean dnoutofordermask;
    bdmf_boolean truantbbhhaltmask;
    bdmf_boolean upinvldgntlenmask;
    bdmf_boolean intcobbhupsfaultmask;
    bdmf_boolean dntimeinsyncmask;
    bdmf_boolean dntimenotinsyncmask;
    bdmf_boolean dportrdymask;
} epn_main_int_mask;

typedef struct
{
    bdmf_boolean cfgctcschdeficiten;
    uint8_t prvzeroburstcapoverridemode;
    bdmf_boolean cfgsharedl2;
    bdmf_boolean cfgsharedburstcap;
    bdmf_boolean cfgrptgntsoutst0;
    bdmf_boolean cfgrpthiprifirst0;
    bdmf_boolean cfgrptswapqs0;
    bdmf_boolean cfgrptmultipri0;
} epn_multi_pri_cfg_0;

bdmf_error_t ag_drv_epn_control_0_set(const epn_control_0 *control_0);
bdmf_error_t ag_drv_epn_control_0_get(epn_control_0 *control_0);
bdmf_error_t ag_drv_epn_control_1_set(const epn_control_1 *control_1);
bdmf_error_t ag_drv_epn_control_1_get(epn_control_1 *control_1);
bdmf_error_t ag_drv_epn_enable_grants_set(uint8_t link_idx, bdmf_boolean grant_en);
bdmf_error_t ag_drv_epn_enable_grants_get(uint8_t link_idx, bdmf_boolean *grant_en);
bdmf_error_t ag_drv_epn_drop_disc_gates_set(uint8_t link_idx, bdmf_boolean linkDiscGates_en);
bdmf_error_t ag_drv_epn_drop_disc_gates_get(uint8_t link_idx, bdmf_boolean *linkDiscGates_en);
bdmf_error_t ag_drv_epn_dis_fcs_chk_set(uint8_t link_idx, bdmf_boolean disableFcsChk);
bdmf_error_t ag_drv_epn_dis_fcs_chk_get(uint8_t link_idx, bdmf_boolean *disableFcsChk);
bdmf_error_t ag_drv_epn_pass_gates_set(uint8_t link_idx, bdmf_boolean passGates);
bdmf_error_t ag_drv_epn_pass_gates_get(uint8_t link_idx, bdmf_boolean *passGates);
bdmf_error_t ag_drv_epn_cfg_misalgn_fb_set(uint8_t link_idx, bdmf_boolean cfgMisalignFeedback);
bdmf_error_t ag_drv_epn_cfg_misalgn_fb_get(uint8_t link_idx, bdmf_boolean *cfgMisalignFeedback);
bdmf_error_t ag_drv_epn_discovery_filter_set(uint16_t prvdiscinfomask, uint16_t prvdiscinfovalue);
bdmf_error_t ag_drv_epn_discovery_filter_get(uint16_t *prvdiscinfomask, uint16_t *prvdiscinfovalue);
bdmf_error_t ag_drv_epn_minimum_grant_setup_set(uint16_t cfgmingrantsetup);
bdmf_error_t ag_drv_epn_minimum_grant_setup_get(uint16_t *cfgmingrantsetup);
bdmf_error_t ag_drv_epn_reset_gnt_fifo_set(uint8_t link_idx, bdmf_boolean rstGntFifo);
bdmf_error_t ag_drv_epn_reset_gnt_fifo_get(uint8_t link_idx, bdmf_boolean *rstGntFifo);
bdmf_error_t ag_drv_epn_reset_l1_accumulator_set(uint32_t cfgl1sclracum);
bdmf_error_t ag_drv_epn_reset_l1_accumulator_get(uint32_t *cfgl1sclracum);
bdmf_error_t ag_drv_epn_l1_accumulator_sel_set(uint8_t cfgl1suvasizesel, uint8_t cfgl1ssvasizesel);
bdmf_error_t ag_drv_epn_l1_accumulator_sel_get(uint8_t *cfgl1suvasizesel, uint8_t *cfgl1ssvasizesel);
bdmf_error_t ag_drv_epn_l1_sva_bytes_get(uint32_t *l1ssvasize);
bdmf_error_t ag_drv_epn_l1_uva_bytes_get(uint32_t *l1suvasize);
bdmf_error_t ag_drv_epn_l1_sva_overflow_get(uint32_t *l1ssvaoverflow);
bdmf_error_t ag_drv_epn_l1_uva_overflow_get(uint32_t *l1suvaoverflow);
bdmf_error_t ag_drv_epn_reset_rpt_pri_set(const epn_reset_rpt_pri *reset_rpt_pri);
bdmf_error_t ag_drv_epn_reset_rpt_pri_get(epn_reset_rpt_pri *reset_rpt_pri);
bdmf_error_t ag_drv_epn_reset_l2_rpt_fifo_set(uint8_t link_idx, bdmf_boolean cfgL2SClrQue);
bdmf_error_t ag_drv_epn_reset_l2_rpt_fifo_get(uint8_t link_idx, bdmf_boolean *cfgL2SClrQue);
bdmf_error_t ag_drv_epn_enable_upstream_set(uint8_t link_idx, bdmf_boolean cfgEnableUpstream);
bdmf_error_t ag_drv_epn_enable_upstream_get(uint8_t link_idx, bdmf_boolean *cfgEnableUpstream);
bdmf_error_t ag_drv_epn_enable_upstream_fb_get(uint8_t link_idx, bdmf_boolean *cfgEnableUpstreamFeedBack);
bdmf_error_t ag_drv_epn_enable_upstream_fec_set(uint8_t link_idx, bdmf_boolean upstreamFecEn);
bdmf_error_t ag_drv_epn_enable_upstream_fec_get(uint8_t link_idx, bdmf_boolean *upstreamFecEn);
bdmf_error_t ag_drv_epn_report_byte_length_set(uint8_t prvrptbytelen);
bdmf_error_t ag_drv_epn_report_byte_length_get(uint8_t *prvrptbytelen);
bdmf_error_t ag_drv_epn_main_int_status_set(const epn_main_int_status *main_int_status);
bdmf_error_t ag_drv_epn_main_int_status_get(epn_main_int_status *main_int_status);
bdmf_error_t ag_drv_epn_gnt_full_int_status_set(uint8_t link_idx, bdmf_boolean intDnGntFullAbort);
bdmf_error_t ag_drv_epn_gnt_full_int_status_get(uint8_t link_idx, bdmf_boolean *intDnGntFullAbort);
bdmf_error_t ag_drv_epn_gnt_full_int_mask_set(uint8_t link_idx, bdmf_boolean maskIntDnGntFullAbort);
bdmf_error_t ag_drv_epn_gnt_full_int_mask_get(uint8_t link_idx, bdmf_boolean *maskIntDnGntFullAbort);
bdmf_error_t ag_drv_epn_gnt_miss_int_status_set(uint8_t link_idx, bdmf_boolean intDnGntMissAbort);
bdmf_error_t ag_drv_epn_gnt_miss_int_status_get(uint8_t link_idx, bdmf_boolean *intDnGntMissAbort);
bdmf_error_t ag_drv_epn_gnt_miss_int_mask_set(uint8_t link_idx, bdmf_boolean maskIntDnGntMissAbort);
bdmf_error_t ag_drv_epn_gnt_miss_int_mask_get(uint8_t link_idx, bdmf_boolean *maskIntDnGntMissAbort);
bdmf_error_t ag_drv_epn_disc_rx_int_status_set(uint8_t link_idx, bdmf_boolean intDnGntDiscovery);
bdmf_error_t ag_drv_epn_disc_rx_int_status_get(uint8_t link_idx, bdmf_boolean *intDnGntDiscovery);
bdmf_error_t ag_drv_epn_disc_rx_int_mask_set(uint8_t link_idx, bdmf_boolean maskIntDnGntDiscovery);
bdmf_error_t ag_drv_epn_disc_rx_int_mask_get(uint8_t link_idx, bdmf_boolean *maskIntDnGntDiscovery);
bdmf_error_t ag_drv_epn_gnt_intv_int_status_set(uint8_t link_idx, bdmf_boolean intDnGntInterval);
bdmf_error_t ag_drv_epn_gnt_intv_int_status_get(uint8_t link_idx, bdmf_boolean *intDnGntInterval);
bdmf_error_t ag_drv_epn_gnt_intv_int_mask_set(uint8_t link_idx, bdmf_boolean maskIntDnGntInterval);
bdmf_error_t ag_drv_epn_gnt_intv_int_mask_get(uint8_t link_idx, bdmf_boolean *maskIntDnGntInterval);
bdmf_error_t ag_drv_epn_gnt_far_int_status_set(uint8_t link_idx, bdmf_boolean intDnGntTooFar);
bdmf_error_t ag_drv_epn_gnt_far_int_status_get(uint8_t link_idx, bdmf_boolean *intDnGntTooFar);
bdmf_error_t ag_drv_epn_gnt_far_int_mask_set(uint8_t link_idx, bdmf_boolean maskDnGntTooFar);
bdmf_error_t ag_drv_epn_gnt_far_int_mask_get(uint8_t link_idx, bdmf_boolean *maskDnGntTooFar);
bdmf_error_t ag_drv_epn_gnt_misalgn_int_status_set(uint8_t link_idx, bdmf_boolean intDnGntMisalign);
bdmf_error_t ag_drv_epn_gnt_misalgn_int_status_get(uint8_t link_idx, bdmf_boolean *intDnGntMisalign);
bdmf_error_t ag_drv_epn_gnt_misalgn_int_mask_set(uint8_t link_idx, bdmf_boolean maskIntDnGntMisalign);
bdmf_error_t ag_drv_epn_gnt_misalgn_int_mask_get(uint8_t link_idx, bdmf_boolean *maskIntDnGntMisalign);
bdmf_error_t ag_drv_epn_np_gnt_int_status_set(uint8_t link_idx, bdmf_boolean intDnGntNonPoll);
bdmf_error_t ag_drv_epn_np_gnt_int_status_get(uint8_t link_idx, bdmf_boolean *intDnGntNonPoll);
bdmf_error_t ag_drv_epn_np_gnt_int_mask_set(uint8_t link_idx, bdmf_boolean maskDnGntNonPoll);
bdmf_error_t ag_drv_epn_np_gnt_int_mask_get(uint8_t link_idx, bdmf_boolean *maskDnGntNonPoll);
bdmf_error_t ag_drv_epn_del_stale_int_status_set(uint8_t link_idx, bdmf_boolean intDelStaleGnt);
bdmf_error_t ag_drv_epn_del_stale_int_status_get(uint8_t link_idx, bdmf_boolean *intDelStaleGnt);
bdmf_error_t ag_drv_epn_del_stale_int_mask_set(uint8_t link_idx, bdmf_boolean maskIntDelStaleGnt);
bdmf_error_t ag_drv_epn_del_stale_int_mask_get(uint8_t link_idx, bdmf_boolean *maskIntDelStaleGnt);
bdmf_error_t ag_drv_epn_gnt_pres_int_status_set(uint8_t link_idx, bdmf_boolean intDnGntRdy);
bdmf_error_t ag_drv_epn_gnt_pres_int_status_get(uint8_t link_idx, bdmf_boolean *intDnGntRdy);
bdmf_error_t ag_drv_epn_gnt_pres_int_mask_set(uint8_t link_idx, bdmf_boolean maskDnGntRdy);
bdmf_error_t ag_drv_epn_gnt_pres_int_mask_get(uint8_t link_idx, bdmf_boolean *maskDnGntRdy);
bdmf_error_t ag_drv_epn_rpt_pres_int_status_set(uint8_t link_idx, bdmf_boolean intUpRptFifo);
bdmf_error_t ag_drv_epn_rpt_pres_int_status_get(uint8_t link_idx, bdmf_boolean *intUpRptFifo);
bdmf_error_t ag_drv_epn_rpt_pres_int_mask_set(uint8_t link_idx, bdmf_boolean maskIntUpRptFifo);
bdmf_error_t ag_drv_epn_rpt_pres_int_mask_get(uint8_t link_idx, bdmf_boolean *maskIntUpRptFifo);
bdmf_error_t ag_drv_epn_drx_abort_int_status_set(uint8_t link_idx, bdmf_boolean intDrxErrAbort);
bdmf_error_t ag_drv_epn_drx_abort_int_status_get(uint8_t link_idx, bdmf_boolean *intDrxErrAbort);
bdmf_error_t ag_drv_epn_drx_abort_int_mask_set(uint8_t link_idx, bdmf_boolean maskIntDrxErrAbort);
bdmf_error_t ag_drv_epn_drx_abort_int_mask_get(uint8_t link_idx, bdmf_boolean *maskIntDrxErrAbort);
bdmf_error_t ag_drv_epn_empty_rpt_int_status_set(uint8_t link_idx, bdmf_boolean intEmptyRpt);
bdmf_error_t ag_drv_epn_empty_rpt_int_status_get(uint8_t link_idx, bdmf_boolean *intEmptyRpt);
bdmf_error_t ag_drv_epn_empty_rpt_int_mask_set(uint8_t link_idx, bdmf_boolean  maskIntEmptyRpt);
bdmf_error_t ag_drv_epn_empty_rpt_int_mask_get(uint8_t link_idx, bdmf_boolean * maskIntEmptyRpt);
bdmf_error_t ag_drv_epn_bcap_overflow_int_status_set(uint8_t l2_acc_idx, bdmf_boolean intl2sBurstCapOverFlow);
bdmf_error_t ag_drv_epn_bcap_overflow_int_status_get(uint8_t l2_acc_idx, bdmf_boolean *intl2sBurstCapOverFlow);
bdmf_error_t ag_drv_epn_bcap_overflow_int_mask_set(uint8_t link_idx, bdmf_boolean maskIntl2sBurstCapOverFlow);
bdmf_error_t ag_drv_epn_bcap_overflow_int_mask_get(uint8_t link_idx, bdmf_boolean *maskIntl2sBurstCapOverFlow);
bdmf_error_t ag_drv_epn_bbh_dns_fault_int_status_set(bdmf_boolean intbbhdnsoverflow);
bdmf_error_t ag_drv_epn_bbh_dns_fault_int_status_get(bdmf_boolean *intbbhdnsoverflow);
bdmf_error_t ag_drv_epn_bbh_dns_fault_int_mask_set(bdmf_boolean maskintbbhdnsoverflow);
bdmf_error_t ag_drv_epn_bbh_dns_fault_int_mask_get(bdmf_boolean *maskintbbhdnsoverflow);
bdmf_error_t ag_drv_epn_bbh_ups_fault_int_status_set(uint8_t link_idx, bdmf_boolean intBbhUpsFault);
bdmf_error_t ag_drv_epn_bbh_ups_fault_int_status_get(uint8_t link_idx, bdmf_boolean *intBbhUpsFault);
bdmf_error_t ag_drv_epn_bbh_ups_fault_int_mask_set(uint8_t link_idx, bdmf_boolean maskIntBbhUpsFault);
bdmf_error_t ag_drv_epn_bbh_ups_fault_int_mask_get(uint8_t link_idx, bdmf_boolean *maskIntBbhUpsFault);
bdmf_error_t ag_drv_epn_bbh_ups_abort_int_status_set(bdmf_boolean tardybbhabort);
bdmf_error_t ag_drv_epn_bbh_ups_abort_int_status_get(bdmf_boolean *tardybbhabort);
bdmf_error_t ag_drv_epn_bbh_ups_abort_int_mask_set(bdmf_boolean masktardybbhabort);
bdmf_error_t ag_drv_epn_bbh_ups_abort_int_mask_get(bdmf_boolean *masktardybbhabort);
bdmf_error_t ag_drv_epn_main_int_mask_set(const epn_main_int_mask *main_int_mask);
bdmf_error_t ag_drv_epn_main_int_mask_get(epn_main_int_mask *main_int_mask);
bdmf_error_t ag_drv_epn_max_gnt_size_set(uint16_t maxgntsize);
bdmf_error_t ag_drv_epn_max_gnt_size_get(uint16_t *maxgntsize);
bdmf_error_t ag_drv_epn_max_frame_size_set(uint16_t cfgmaxframesize);
bdmf_error_t ag_drv_epn_max_frame_size_get(uint16_t *cfgmaxframesize);
bdmf_error_t ag_drv_epn_grant_ovr_hd_set(uint16_t gntovrhdfec, uint16_t gntovrhd);
bdmf_error_t ag_drv_epn_grant_ovr_hd_get(uint16_t *gntovrhdfec, uint16_t *gntovrhd);
bdmf_error_t ag_drv_epn_poll_size_set(uint16_t pollsizefec, uint16_t pollsize);
bdmf_error_t ag_drv_epn_poll_size_get(uint16_t *pollsizefec, uint16_t *pollsize);
bdmf_error_t ag_drv_epn_dn_rd_gnt_margin_set(uint16_t rdgntstartmargin);
bdmf_error_t ag_drv_epn_dn_rd_gnt_margin_get(uint16_t *rdgntstartmargin);
bdmf_error_t ag_drv_epn_gnt_time_start_delta_set(uint16_t gntstarttimedelta);
bdmf_error_t ag_drv_epn_gnt_time_start_delta_get(uint16_t *gntstarttimedelta);
bdmf_error_t ag_drv_epn_time_stamp_diff_set(uint16_t timestampdiffdelta);
bdmf_error_t ag_drv_epn_time_stamp_diff_get(uint16_t *timestampdiffdelta);
bdmf_error_t ag_drv_epn_up_time_stamp_off_set(uint16_t timestampoffsetfec, uint16_t timestampoffset);
bdmf_error_t ag_drv_epn_up_time_stamp_off_get(uint16_t *timestampoffsetfec, uint16_t *timestampoffset);
bdmf_error_t ag_drv_epn_gnt_interval_set(uint16_t gntinterval);
bdmf_error_t ag_drv_epn_gnt_interval_get(uint16_t *gntinterval);
bdmf_error_t ag_drv_epn_dn_gnt_misalign_thr_set(uint16_t prvunusedgntthreshold, uint16_t gntmisalignthresh);
bdmf_error_t ag_drv_epn_dn_gnt_misalign_thr_get(uint16_t *prvunusedgntthreshold, uint16_t *gntmisalignthresh);
bdmf_error_t ag_drv_epn_dn_gnt_misalign_pause_set(uint16_t gntmisalignpause);
bdmf_error_t ag_drv_epn_dn_gnt_misalign_pause_get(uint16_t *gntmisalignpause);
bdmf_error_t ag_drv_epn_non_poll_intv_set(uint16_t nonpollgntintv);
bdmf_error_t ag_drv_epn_non_poll_intv_get(uint16_t *nonpollgntintv);
bdmf_error_t ag_drv_epn_force_fcs_err_set(uint8_t link_idx, bdmf_boolean forceFcsErr);
bdmf_error_t ag_drv_epn_force_fcs_err_get(uint8_t link_idx, bdmf_boolean *forceFcsErr);
bdmf_error_t ag_drv_epn_grant_overlap_limit_set(uint16_t prvgrantoverlaplimit);
bdmf_error_t ag_drv_epn_grant_overlap_limit_get(uint16_t *prvgrantoverlaplimit);
bdmf_error_t ag_drv_epn_aes_configuration_0_set(uint8_t link_idx, uint8_t prvUpstreamAesMode_0);
bdmf_error_t ag_drv_epn_aes_configuration_0_get(uint8_t link_idx, uint8_t *prvUpstreamAesMode_0);
bdmf_error_t ag_drv_epn_aes_configuration_1_set(uint8_t link_idx, uint8_t prvUpstreamAesMode_1);
bdmf_error_t ag_drv_epn_aes_configuration_1_get(uint8_t link_idx, uint8_t *prvUpstreamAesMode_1);
bdmf_error_t ag_drv_epn_disc_grant_ovr_hd_set(uint16_t discgntovrhd);
bdmf_error_t ag_drv_epn_disc_grant_ovr_hd_get(uint16_t *discgntovrhd);
bdmf_error_t ag_drv_epn_dn_discovery_seed_set(uint16_t cfgdiscseed);
bdmf_error_t ag_drv_epn_dn_discovery_seed_get(uint16_t *cfgdiscseed);
bdmf_error_t ag_drv_epn_dn_discovery_inc_set(uint16_t cfgdiscinc);
bdmf_error_t ag_drv_epn_dn_discovery_inc_get(uint16_t *cfgdiscinc);
bdmf_error_t ag_drv_epn_dn_discovery_size_set(uint16_t cfgdiscsize);
bdmf_error_t ag_drv_epn_dn_discovery_size_get(uint16_t *cfgdiscsize);
bdmf_error_t ag_drv_epn_fec_ipg_length_set(uint8_t modipgpreamblebytes, uint8_t cfgrptlen, uint8_t cfgfecrptlength, uint8_t cfgfecipglength);
bdmf_error_t ag_drv_epn_fec_ipg_length_get(uint8_t *modipgpreamblebytes, uint8_t *cfgrptlen, uint8_t *cfgfecrptlength, uint8_t *cfgfecipglength);
bdmf_error_t ag_drv_epn_fake_report_value_en_set(uint32_t fakereportvalueen);
bdmf_error_t ag_drv_epn_fake_report_value_en_get(uint32_t *fakereportvalueen);
bdmf_error_t ag_drv_epn_fake_report_value_set(uint32_t fakereportvalue);
bdmf_error_t ag_drv_epn_fake_report_value_get(uint32_t *fakereportvalue);
bdmf_error_t ag_drv_epn_burst_cap_set(uint8_t link_idx, uint32_t burstcap);
bdmf_error_t ag_drv_epn_burst_cap_get(uint8_t link_idx, uint32_t *burstcap);
bdmf_error_t ag_drv_epn_queue_llid_map_set(uint8_t que_idx, uint8_t quellidmap);
bdmf_error_t ag_drv_epn_queue_llid_map_get(uint8_t que_idx, uint8_t *quellidmap);
bdmf_error_t ag_drv_epn_valid_opcode_map_set(uint16_t prvvalidmpcpopcodes);
bdmf_error_t ag_drv_epn_valid_opcode_map_get(uint16_t *prvvalidmpcpopcodes);
bdmf_error_t ag_drv_epn_up_packet_tx_margin_set(uint16_t uppackettxmargin);
bdmf_error_t ag_drv_epn_up_packet_tx_margin_get(uint16_t *uppackettxmargin);
bdmf_error_t ag_drv_epn_multi_pri_cfg_0_set(const epn_multi_pri_cfg_0 *multi_pri_cfg_0);
bdmf_error_t ag_drv_epn_multi_pri_cfg_0_get(epn_multi_pri_cfg_0 *multi_pri_cfg_0);
bdmf_error_t ag_drv_epn_shared_bcap_ovrflow_get(uint16_t *sharedburstcapoverflow);
bdmf_error_t ag_drv_epn_forced_report_en_set(uint8_t link_idx, bdmf_boolean cfgForceReportEn);
bdmf_error_t ag_drv_epn_forced_report_en_get(uint8_t link_idx, bdmf_boolean *cfgForceReportEn);
bdmf_error_t ag_drv_epn_forced_report_max_interval_set(uint8_t cfgmaxreportinterval);
bdmf_error_t ag_drv_epn_forced_report_max_interval_get(uint8_t *cfgmaxreportinterval);
bdmf_error_t ag_drv_epn_l2s_flush_config_set(bdmf_boolean cfgflushl2sen, bdmf_boolean flushl2sdone, uint8_t cfgflushl2ssel);
bdmf_error_t ag_drv_epn_l2s_flush_config_get(bdmf_boolean *cfgflushl2sen, bdmf_boolean *flushl2sdone, uint8_t *cfgflushl2ssel);
bdmf_error_t ag_drv_epn_data_port_command_set(bdmf_boolean dportbusy, uint8_t dportselect, bdmf_boolean dportcontrol);
bdmf_error_t ag_drv_epn_data_port_command_get(bdmf_boolean *dportbusy, uint8_t *dportselect, bdmf_boolean *dportcontrol);
bdmf_error_t ag_drv_epn_data_port_address_set(uint16_t dportaddr);
bdmf_error_t ag_drv_epn_data_port_address_get(uint16_t *dportaddr);
bdmf_error_t ag_drv_epn_data_port_data_0_set(uint32_t dportdata0);
bdmf_error_t ag_drv_epn_data_port_data_0_get(uint32_t *dportdata0);
bdmf_error_t ag_drv_epn_unmap_big_cnt_set(uint32_t unmapbigerrcnt);
bdmf_error_t ag_drv_epn_unmap_big_cnt_get(uint32_t *unmapbigerrcnt);
bdmf_error_t ag_drv_epn_unmap_frame_cnt_set(uint32_t unmapfrcnt);
bdmf_error_t ag_drv_epn_unmap_frame_cnt_get(uint32_t *unmapfrcnt);
bdmf_error_t ag_drv_epn_unmap_fcs_cnt_set(uint32_t unmapfcserrcnt);
bdmf_error_t ag_drv_epn_unmap_fcs_cnt_get(uint32_t *unmapfcserrcnt);
bdmf_error_t ag_drv_epn_unmap_gate_cnt_set(uint32_t unmapgatecnt);
bdmf_error_t ag_drv_epn_unmap_gate_cnt_get(uint32_t *unmapgatecnt);
bdmf_error_t ag_drv_epn_unmap_oam_cnt_set(uint32_t unmapoamcnt);
bdmf_error_t ag_drv_epn_unmap_oam_cnt_get(uint32_t *unmapoamcnt);
bdmf_error_t ag_drv_epn_unmap_small_cnt_set(uint32_t unmapsmallerrcnt);
bdmf_error_t ag_drv_epn_unmap_small_cnt_get(uint32_t *unmapsmallerrcnt);
bdmf_error_t ag_drv_epn_fif_dequeue_event_cnt_set(uint32_t fifdequeueeventcnt);
bdmf_error_t ag_drv_epn_fif_dequeue_event_cnt_get(uint32_t *fifdequeueeventcnt);
bdmf_error_t ag_drv_epn_unused_tq_cnt_set(uint8_t link_idx, uint32_t unusedtqcnt);
bdmf_error_t ag_drv_epn_unused_tq_cnt_get(uint8_t link_idx, uint32_t *unusedtqcnt);
bdmf_error_t ag_drv_epn_bbh_up_fault_halt_en_set(uint8_t link_idx, bdmf_boolean bbhUpsFaultHaltEn);
bdmf_error_t ag_drv_epn_bbh_up_fault_halt_en_get(uint8_t link_idx, bdmf_boolean *bbhUpsFaultHaltEn);
bdmf_error_t ag_drv_epn_bbh_up_tardy_halt_en_set(bdmf_boolean fataltardybbhaborten);
bdmf_error_t ag_drv_epn_bbh_up_tardy_halt_en_get(bdmf_boolean *fataltardybbhaborten);
bdmf_error_t ag_drv_epn_debug_status_0_get(uint8_t *l2squefulldebug, bdmf_boolean *dndlufull, bdmf_boolean *dnsecfull, bdmf_boolean *epnliffifofull);
bdmf_error_t ag_drv_epn_debug_status_1_get(uint8_t link_idx, bdmf_boolean *gntRdy);
bdmf_error_t ag_drv_epn_debug_l2s_ptr_sel_set(uint8_t cfgl2sdebugptrsel, uint16_t l2sdebugptrstate);
bdmf_error_t ag_drv_epn_debug_l2s_ptr_sel_get(uint8_t *cfgl2sdebugptrsel, uint16_t *l2sdebugptrstate);
bdmf_error_t ag_drv_epn_olt_mac_addr_lo_set(uint32_t oltaddrlo);
bdmf_error_t ag_drv_epn_olt_mac_addr_lo_get(uint32_t *oltaddrlo);
bdmf_error_t ag_drv_epn_olt_mac_addr_hi_set(uint16_t oltaddrhi);
bdmf_error_t ag_drv_epn_olt_mac_addr_hi_get(uint16_t *oltaddrhi);
bdmf_error_t ag_drv_epn_tx_l1s_shp_dqu_empty_get(uint8_t l1_acc_idx, bdmf_boolean *l1sDquQueEmpty);
bdmf_error_t ag_drv_epn_tx_l1s_unshaped_empty_get(uint8_t l1_acc_idx, bdmf_boolean *l1sUnshapedQueEmpty);
bdmf_error_t ag_drv_epn_tx_l1s_shp_que_mask_set(uint8_t shaper_idx, uint32_t cfgshpmask);
bdmf_error_t ag_drv_epn_tx_l1s_shp_que_mask_get(uint8_t shaper_idx, uint32_t *cfgshpmask);
bdmf_error_t ag_drv_epn_tx_l2s_queue_config_set(uint8_t que_idx, uint16_t cfgl2squeend, uint16_t cfgl2squestart);
bdmf_error_t ag_drv_epn_tx_l2s_queue_config_get(uint8_t que_idx, uint16_t *cfgl2squeend, uint16_t *cfgl2squestart);
bdmf_error_t ag_drv_epn_tx_l2s_que_empty_get(uint8_t l2_queue_idx, bdmf_boolean *l2sQueEmpty);
bdmf_error_t ag_drv_epn_tx_l2s_que_full_get(uint8_t l2_queue_idx, bdmf_boolean *l2sQueFull);
bdmf_error_t ag_drv_epn_tx_l2s_que_stopped_get(uint8_t l2_queue_idx, bdmf_boolean *l2sStoppedQueues);
bdmf_error_t ag_drv_epn_tx_ctc_burst_limit_set(uint8_t l2_queue_idx, uint32_t prvburstlimit);
bdmf_error_t ag_drv_epn_tx_ctc_burst_limit_get(uint8_t l2_queue_idx, uint32_t *prvburstlimit);
bdmf_error_t ag_drv_epn_bbh_max_outstanding_tardy_packets_set(uint16_t cfgmaxoutstandingtardypackets);
bdmf_error_t ag_drv_epn_bbh_max_outstanding_tardy_packets_get(uint16_t *cfgmaxoutstandingtardypackets);
bdmf_error_t ag_drv_epn_min_report_value_difference_set(uint16_t prvminreportdiff);
bdmf_error_t ag_drv_epn_min_report_value_difference_get(uint16_t *prvminreportdiff);
bdmf_error_t ag_drv_epn_bbh_status_fifo_overflow_get(uint8_t bbh_queue_idx, bdmf_boolean *utxBbhStatusFifoOverflow);
bdmf_error_t ag_drv_epn_spare_ctl_set(uint32_t cfgepnspare, bdmf_boolean ecoutxsnfenable, bdmf_boolean ecojira758enable);
bdmf_error_t ag_drv_epn_spare_ctl_get(uint32_t *cfgepnspare, bdmf_boolean *ecoutxsnfenable, bdmf_boolean *ecojira758enable);
bdmf_error_t ag_drv_epn_ts_sync_offset_set(uint8_t cfgtssyncoffset);
bdmf_error_t ag_drv_epn_ts_sync_offset_get(uint8_t *cfgtssyncoffset);
bdmf_error_t ag_drv_epn_dn_ts_offset_set(uint32_t cfgdntsoffset);
bdmf_error_t ag_drv_epn_dn_ts_offset_get(uint32_t *cfgdntsoffset);
bdmf_error_t ag_drv_epn_up_ts_offset_lo_set(uint32_t cfguptsoffset_lo);
bdmf_error_t ag_drv_epn_up_ts_offset_lo_get(uint32_t *cfguptsoffset_lo);
bdmf_error_t ag_drv_epn_up_ts_offset_hi_set(uint16_t cfguptsoffset_hi);
bdmf_error_t ag_drv_epn_up_ts_offset_hi_get(uint16_t *cfguptsoffset_hi);
bdmf_error_t ag_drv_epn_two_step_ts_ctl_set(bdmf_boolean twostepffrd, uint8_t twostepffentries);
bdmf_error_t ag_drv_epn_two_step_ts_ctl_get(bdmf_boolean *twostepffrd, uint8_t *twostepffentries);
bdmf_error_t ag_drv_epn_two_step_ts_value_lo_get(uint32_t *twosteptimestamp_lo);
bdmf_error_t ag_drv_epn_two_step_ts_value_hi_get(uint16_t *twosteptimestamp_hi);
bdmf_error_t ag_drv_epn_1588_timestamp_int_status_set(bdmf_boolean int1588pktabort, bdmf_boolean int1588twostepffint);
bdmf_error_t ag_drv_epn_1588_timestamp_int_status_get(bdmf_boolean *int1588pktabort, bdmf_boolean *int1588twostepffint);
bdmf_error_t ag_drv_epn_1588_timestamp_int_mask_set(bdmf_boolean ts1588pktabort_mask, bdmf_boolean ts1588twostepff_mask);
bdmf_error_t ag_drv_epn_1588_timestamp_int_mask_get(bdmf_boolean *ts1588pktabort_mask, bdmf_boolean *ts1588twostepff_mask);
bdmf_error_t ag_drv_epn_up_packet_fetch_margin_set(uint16_t uppacketfetchmargin);
bdmf_error_t ag_drv_epn_up_packet_fetch_margin_get(uint16_t *uppacketfetchmargin);
bdmf_error_t ag_drv_epn_dn_1588_timestamp_get(uint32_t *dn_1588_ts);
bdmf_error_t ag_drv_epn_persistent_report_cfg_set(uint16_t cfgpersrptduration, uint16_t cfgpersrptticksize);
bdmf_error_t ag_drv_epn_persistent_report_cfg_get(uint16_t *cfgpersrptduration, uint16_t *cfgpersrptticksize);
bdmf_error_t ag_drv_epn_persistent_report_enables_set(uint8_t link_idx, bdmf_boolean cfgPersRptEnable);
bdmf_error_t ag_drv_epn_persistent_report_enables_get(uint8_t link_idx, bdmf_boolean *cfgPersRptEnable);
bdmf_error_t ag_drv_epn_persistent_report_request_size_set(uint16_t cfgpersrptreqtq);
bdmf_error_t ag_drv_epn_persistent_report_request_size_get(uint16_t *cfgpersrptreqtq);
bdmf_error_t ag_drv_epn_10g_abc_size_set(uint8_t size_idx, uint16_t cfgaddburstcap2, uint16_t cfgaddburstcap1);
bdmf_error_t ag_drv_epn_10g_abc_size_get(uint8_t size_idx, uint16_t *cfgaddburstcap2, uint16_t *cfgaddburstcap1);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_epn_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif


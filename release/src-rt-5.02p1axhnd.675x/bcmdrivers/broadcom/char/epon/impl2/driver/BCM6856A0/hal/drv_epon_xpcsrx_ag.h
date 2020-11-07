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

#ifndef _DRV_EPON_XPCSRX_AG_H_
#define _DRV_EPON_XPCSRX_AG_H_

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
    bdmf_boolean intrxidledajit;
    bdmf_boolean intrxfrmrmisbrst;
    bdmf_boolean intrxidlesopeopgapbig;
    bdmf_boolean intrxidlefrcins;
    bdmf_boolean intrx64b66bminipgerr;
    bdmf_boolean intrxfecnquecntneq;
    bdmf_boolean intrxidlefifoundrun;
    bdmf_boolean intrxidlefifoovrrun;
    bdmf_boolean intrxfechighcor;
    bdmf_boolean intrxfecdecstoponerr;
    bdmf_boolean intrxfecdecpass;
    bdmf_boolean intrxstatfrmrhighber;
    bdmf_boolean intrxfrmrexitbysp;
    bdmf_boolean intrxfrmrbadshmax;
    bdmf_boolean intrxdscramburstseqout;
    bdmf_boolean intrxtestpsudolock;
    bdmf_boolean intrxtestpsudotype;
    bdmf_boolean intrxtestpsudoerr;
    bdmf_boolean intrxtestprbslock;
    bdmf_boolean intrxtestprbserr;
    bdmf_boolean intrxfecpsistdecfail;
    bdmf_boolean intrxframerbadsh;
    bdmf_boolean intrxframercwloss;
    bdmf_boolean intrxframercwlock;
    bdmf_boolean intrxfecdecfail;
    bdmf_boolean intrx64b66bdecerr;
    bdmf_boolean intrxfrmrnolocklos;
    bdmf_boolean intrxfrmrrogue;
    bdmf_boolean int_regs_err;
} xpcsrx_rx_int_stat;

typedef struct
{
    bdmf_boolean mskrxidledajit;
    bdmf_boolean mskrxfrmrmisbrst;
    bdmf_boolean mskrxidlesopeopgapbig;
    bdmf_boolean mskrxidlefrcins;
    bdmf_boolean mskrx64b66bminipgerr;
    bdmf_boolean mskrxfecnquecntneq;
    bdmf_boolean mskrxidlefifoundrun;
    bdmf_boolean mskrxidlefifoovrrun;
    bdmf_boolean mskrxfechighcor;
    bdmf_boolean mskrxfecdecstoponerr;
    bdmf_boolean mskrxfecdecpass;
    bdmf_boolean mskrxstatfrmrhighber;
    bdmf_boolean mskrxfrmrexitbysp;
    bdmf_boolean mskrxfrmrbadshmax;
    bdmf_boolean mskrxdscramburstseqout;
    bdmf_boolean mskrxtestpsudolock;
    bdmf_boolean mskrxtestpsudotype;
    bdmf_boolean mskrxtestpsudoerr;
    bdmf_boolean mskrxtestprbslock;
    bdmf_boolean mskrxtestprbserr;
    bdmf_boolean mskrxfecpsistdecfail;
    bdmf_boolean mskrxframerbadsh;
    bdmf_boolean mskrxframercwloss;
    bdmf_boolean mskrxframercwlock;
    bdmf_boolean mskrxfecdecfail;
    bdmf_boolean mskrx64b66bdecerr;
    bdmf_boolean mskrxfrmrnolocklos;
    bdmf_boolean mskrxfrmrrogue;
    bdmf_boolean msk_int_regs_err;
} xpcsrx_rx_int_msk;

typedef struct
{
    bdmf_boolean cfgxpcsrxfrmrfrcearlyalign;
    bdmf_boolean cfgxpcsrxfrmrmodea;
    bdmf_boolean cfgxpcsrxfrmroverlapbdebdzero;
    bdmf_boolean cfgxpcsrxfrmroverlapgnten;
    bdmf_boolean cfgxpcsrxframeburstoldalign;
    bdmf_boolean cfgxpcsrxfrmrmisbrsttype;
    bdmf_boolean cfgxpcsrxfrmrebdvlden;
    bdmf_boolean cfgxpcsrxfrmrbdcnten;
    bdmf_boolean cfgxpcsrxfrmrburstbadshen;
    bdmf_boolean cfgxpcsrxfrmrspulken;
    bdmf_boolean cfgxpcsrxframeburst;
    bdmf_boolean cfgxpcsrxfrmren;
    bdmf_boolean cfgxpcsrxfrmrblkfecfail;
    bdmf_boolean cfgxpcsrxframefec;
} xpcsrx_rx_framer_ctl;

typedef struct
{
    bdmf_boolean cfgxpcsrxfecstoponerr;
    bdmf_boolean cfgxpcsrxfeccntnquecw;
    bdmf_boolean cfgxpcsrxfecnquerst;
    bdmf_boolean cfgxpcsrxfeconezeromode;
    bdmf_boolean cfgxpcsrxfecblkcorrect;
    bdmf_boolean cfgxpcsrxfecnquetestpat;
    bdmf_boolean cfgxpcsrxfecfailblksh0;
    bdmf_boolean cfgxpcsrxfecstrip;
    bdmf_boolean cfgxpcsrxfecbypas;
    bdmf_boolean cfgxpcsrxfecidleins;
    bdmf_boolean cfgxpcsrxfecen;
} xpcsrx_rx_fec_ctl;

typedef struct
{
    uint8_t cfgxpcsrx64b66btmask1;
    uint8_t cfgxpcsrx64b66btmask0;
    uint8_t cfgxpcsrx64b66bsmask1;
    uint8_t cfgxpcsrx64b66bsmask0;
    uint8_t cfgxpcsrx64b66btdlay;
    bdmf_boolean cfgxpcsrx64b66bdecbypas;
} xpcsrx_rx_64b66b_ctl;

typedef struct
{
    bdmf_boolean statrxidledajit;
    bdmf_boolean statrxfrmrmisbrst;
    bdmf_boolean statrxidlesopeopgapbig;
    bdmf_boolean statrxidlefrcins;
    bdmf_boolean statrx64b66bminipgerr;
    bdmf_boolean statrxfecnquecntneq;
    bdmf_boolean statrxidlefifoundrun;
    bdmf_boolean statrxidlefifoovrrun;
    bdmf_boolean statrxfechighcor;
    bdmf_boolean statrxfecdecpass;
    bdmf_boolean statrxstatfrmrhighber;
    bdmf_boolean statrxfrmrexitbysp;
    bdmf_boolean statrxfrmrbadshmax;
    bdmf_boolean statrxdscramburstseqout;
    bdmf_boolean statrxtestpsudolock;
    bdmf_boolean statrxtestpsudotype;
    bdmf_boolean statrxtestpsudoerr;
    bdmf_boolean statrxtestprbslock;
    bdmf_boolean statrxtestprbserr;
    bdmf_boolean statrxfecpsistdecfail;
    bdmf_boolean statrxframerbadsh;
    bdmf_boolean statrxframercwloss;
    bdmf_boolean statrxframercwlock;
    bdmf_boolean statrxfecdecfail;
    bdmf_boolean statrx64b66bdecerr;
    bdmf_boolean statrxfrmrnolocklos;
    bdmf_boolean statrxfrmrrogue;
} xpcsrx_rx_status;

typedef struct
{
    bdmf_boolean xpcsrxdpbusy;
    bdmf_boolean xpcsrxdperr;
    uint8_t cfgxpcsrxdpctl;
    uint8_t cfgxpcsrxdpramsel;
    uint16_t cfgxpcsrxdpaddr;
} xpcsrx_rx_dport_ctl;

bdmf_error_t ag_drv_xpcsrx_rx_rst_set(bdmf_boolean cfgxpcsrxclk161rstn);
bdmf_error_t ag_drv_xpcsrx_rx_rst_get(bdmf_boolean *cfgxpcsrxclk161rstn);
bdmf_error_t ag_drv_xpcsrx_rx_int_stat_set(const xpcsrx_rx_int_stat *rx_int_stat);
bdmf_error_t ag_drv_xpcsrx_rx_int_stat_get(xpcsrx_rx_int_stat *rx_int_stat);
bdmf_error_t ag_drv_xpcsrx_rx_int_msk_set(const xpcsrx_rx_int_msk *rx_int_msk);
bdmf_error_t ag_drv_xpcsrx_rx_int_msk_get(xpcsrx_rx_int_msk *rx_int_msk);
bdmf_error_t ag_drv_xpcsrx_rx_framer_ctl_set(const xpcsrx_rx_framer_ctl *rx_framer_ctl);
bdmf_error_t ag_drv_xpcsrx_rx_framer_ctl_get(xpcsrx_rx_framer_ctl *rx_framer_ctl);
bdmf_error_t ag_drv_xpcsrx_rx_fec_ctl_set(const xpcsrx_rx_fec_ctl *rx_fec_ctl);
bdmf_error_t ag_drv_xpcsrx_rx_fec_ctl_get(xpcsrx_rx_fec_ctl *rx_fec_ctl);
bdmf_error_t ag_drv_xpcsrx_rx_dscram_ctl_set(bdmf_boolean cfgxpcsrxdscrambypas);
bdmf_error_t ag_drv_xpcsrx_rx_dscram_ctl_get(bdmf_boolean *cfgxpcsrxdscrambypas);
bdmf_error_t ag_drv_xpcsrx_rx_64b66b_ctl_set(const xpcsrx_rx_64b66b_ctl *rx_64b66b_ctl);
bdmf_error_t ag_drv_xpcsrx_rx_64b66b_ctl_get(xpcsrx_rx_64b66b_ctl *rx_64b66b_ctl);
bdmf_error_t ag_drv_xpcsrx_rx_test_ctl_set(bdmf_boolean cfgxpcsrxtestprbsdeten, bdmf_boolean cfgxpcsrxtestpsudodeten);
bdmf_error_t ag_drv_xpcsrx_rx_test_ctl_get(bdmf_boolean *cfgxpcsrxtestprbsdeten, bdmf_boolean *cfgxpcsrxtestpsudodeten);
bdmf_error_t ag_drv_xpcsrx_rx_idle_rd_timer_dly_set(uint8_t cfgxpcsrxidlerddelaytimermax);
bdmf_error_t ag_drv_xpcsrx_rx_idle_rd_timer_dly_get(uint8_t *cfgxpcsrxidlerddelaytimermax);
bdmf_error_t ag_drv_xpcsrx_rx_idle_gap_siz_max_set(uint16_t cfgxpcsrxidleovrsizmax, uint16_t cfgxpcsrxidlesopeopgap);
bdmf_error_t ag_drv_xpcsrx_rx_idle_gap_siz_max_get(uint16_t *cfgxpcsrxidleovrsizmax, uint16_t *cfgxpcsrxidlesopeopgap);
bdmf_error_t ag_drv_xpcsrx_rx_framer_lk_max_set(uint16_t cfgxpcsrxfrmrcwlktimermax);
bdmf_error_t ag_drv_xpcsrx_rx_framer_lk_max_get(uint16_t *cfgxpcsrxfrmrcwlktimermax);
bdmf_error_t ag_drv_xpcsrx_rx_framer_unlk_max_set(uint16_t cfgxpcsrxfrmrcwunlktimermax);
bdmf_error_t ag_drv_xpcsrx_rx_framer_unlk_max_get(uint16_t *cfgxpcsrxfrmrcwunlktimermax);
bdmf_error_t ag_drv_xpcsrx_rx_framer_bd_sh_set(uint8_t cfgxpcsrxoltbdsh);
bdmf_error_t ag_drv_xpcsrx_rx_framer_bd_sh_get(uint8_t *cfgxpcsrxoltbdsh);
bdmf_error_t ag_drv_xpcsrx_rx_framer_bd_lo_set(uint32_t cfgxpcsrxoltbdlo);
bdmf_error_t ag_drv_xpcsrx_rx_framer_bd_lo_get(uint32_t *cfgxpcsrxoltbdlo);
bdmf_error_t ag_drv_xpcsrx_rx_framer_bd_hi_set(uint32_t cfgxpcsrxoltbdhi);
bdmf_error_t ag_drv_xpcsrx_rx_framer_bd_hi_get(uint32_t *cfgxpcsrxoltbdhi);
bdmf_error_t ag_drv_xpcsrx_rx_framer_ebd_sh_set(uint8_t cfgxpcsrxoltebdsh);
bdmf_error_t ag_drv_xpcsrx_rx_framer_ebd_sh_get(uint8_t *cfgxpcsrxoltebdsh);
bdmf_error_t ag_drv_xpcsrx_rx_framer_ebd_lo_set(uint32_t cfgxpcsrxoltebdlo);
bdmf_error_t ag_drv_xpcsrx_rx_framer_ebd_lo_get(uint32_t *cfgxpcsrxoltebdlo);
bdmf_error_t ag_drv_xpcsrx_rx_framer_ebd_hi_set(uint32_t cfgxpcsrxoltebdhi);
bdmf_error_t ag_drv_xpcsrx_rx_framer_ebd_hi_get(uint32_t *cfgxpcsrxoltebdhi);
bdmf_error_t ag_drv_xpcsrx_rx_status_get(xpcsrx_rx_status *rx_status);
bdmf_error_t ag_drv_xpcsrx_rx_framer_lk_ulk_max_set(uint16_t cfgxpcsrxfrmrsplkmax, uint16_t cfgxpcsrxfrmrspulkmax);
bdmf_error_t ag_drv_xpcsrx_rx_framer_lk_ulk_max_get(uint16_t *cfgxpcsrxfrmrsplkmax, uint16_t *cfgxpcsrxfrmrspulkmax);
bdmf_error_t ag_drv_xpcsrx_rx_framer_sp_sh_set(uint8_t cfgxpcsrxoltspsh);
bdmf_error_t ag_drv_xpcsrx_rx_framer_sp_sh_get(uint8_t *cfgxpcsrxoltspsh);
bdmf_error_t ag_drv_xpcsrx_rx_framer_sp_lo_set(uint32_t cfgxpcsrxoltsplo);
bdmf_error_t ag_drv_xpcsrx_rx_framer_sp_lo_get(uint32_t *cfgxpcsrxoltsplo);
bdmf_error_t ag_drv_xpcsrx_rx_framer_sp_hi_set(uint32_t cfgxpcsrxoltsphi);
bdmf_error_t ag_drv_xpcsrx_rx_framer_sp_hi_get(uint32_t *cfgxpcsrxoltsphi);
bdmf_error_t ag_drv_xpcsrx_rx_framer_state_get(uint8_t *xpcsrxfrmrstate);
bdmf_error_t ag_drv_xpcsrx_rx_framer_bd_ebd_ham_set(uint8_t cfgxpcsrxfrmrspham, uint8_t cfgxpcsrxfrmrebdham, uint8_t cfgxpcsrxfrmrbdham);
bdmf_error_t ag_drv_xpcsrx_rx_framer_bd_ebd_ham_get(uint8_t *cfgxpcsrxfrmrspham, uint8_t *cfgxpcsrxfrmrebdham, uint8_t *cfgxpcsrxfrmrbdham);
bdmf_error_t ag_drv_xpcsrx_rx_framer_misbrst_cnt_set(uint32_t rxfrmrmisbrstcnt);
bdmf_error_t ag_drv_xpcsrx_rx_framer_misbrst_cnt_get(uint32_t *rxfrmrmisbrstcnt);
bdmf_error_t ag_drv_xpcsrx_rx_framer_bd_err_get(uint8_t *xpcsrxstatfrmrbderr);
bdmf_error_t ag_drv_xpcsrx_rx_framer_rogue_ctl_set(bdmf_boolean cfgxpcsrxfrmrrogueen, uint16_t cfgxpcsrxfrmrroguesptresh);
bdmf_error_t ag_drv_xpcsrx_rx_framer_rogue_ctl_get(bdmf_boolean *cfgxpcsrxfrmrrogueen, uint16_t *cfgxpcsrxfrmrroguesptresh);
bdmf_error_t ag_drv_xpcsrx_rx_framer_nolock_ctl_set(bdmf_boolean cfgxpcsrxfrmrnolocklosen, uint32_t cfgxpcsrxfrmrnolocklosintval);
bdmf_error_t ag_drv_xpcsrx_rx_framer_nolock_ctl_get(bdmf_boolean *cfgxpcsrxfrmrnolocklosen, uint32_t *cfgxpcsrxfrmrnolocklosintval);
bdmf_error_t ag_drv_xpcsrx_rx_64b66b_ipg_det_cnt_set(uint32_t rx64b66bipgdetcnt);
bdmf_error_t ag_drv_xpcsrx_rx_64b66b_ipg_det_cnt_get(uint32_t *rx64b66bipgdetcnt);
bdmf_error_t ag_drv_xpcsrx_rx_fec_nque_in_cnt_set(uint32_t rxfecnqueincnt);
bdmf_error_t ag_drv_xpcsrx_rx_fec_nque_in_cnt_get(uint32_t *rxfecnqueincnt);
bdmf_error_t ag_drv_xpcsrx_rx_fec_nque_out_cnt_set(uint32_t rxfecnqueoutcnt);
bdmf_error_t ag_drv_xpcsrx_rx_fec_nque_out_cnt_get(uint32_t *rxfecnqueoutcnt);
bdmf_error_t ag_drv_xpcsrx_rx_idle_start_cnt_set(uint32_t rxidlestartcnt);
bdmf_error_t ag_drv_xpcsrx_rx_idle_start_cnt_get(uint32_t *rxidlestartcnt);
bdmf_error_t ag_drv_xpcsrx_rx_idle_stop_cnt_set(uint32_t rxidlestopcnt);
bdmf_error_t ag_drv_xpcsrx_rx_idle_stop_cnt_get(uint32_t *rxidlestopcnt);
bdmf_error_t ag_drv_xpcsrx_rx_fec_cor_intval_set(uint32_t cfgxpcsrxfeccorintval);
bdmf_error_t ag_drv_xpcsrx_rx_fec_cor_intval_get(uint32_t *cfgxpcsrxfeccorintval);
bdmf_error_t ag_drv_xpcsrx_rx_fec_cor_tresh_set(uint32_t cfgxpcsrxfeccortresh);
bdmf_error_t ag_drv_xpcsrx_rx_fec_cor_tresh_get(uint32_t *cfgxpcsrxfeccortresh);
bdmf_error_t ag_drv_xpcsrx_rx_fec_cw_fail_cnt_set(uint32_t rxfecdeccwfailcnt);
bdmf_error_t ag_drv_xpcsrx_rx_fec_cw_fail_cnt_get(uint32_t *rxfecdeccwfailcnt);
bdmf_error_t ag_drv_xpcsrx_rx_fec_cw_tot_cnt_set(uint32_t rxfecdeccwtotcnt);
bdmf_error_t ag_drv_xpcsrx_rx_fec_cw_tot_cnt_get(uint32_t *rxfecdeccwtotcnt);
bdmf_error_t ag_drv_xpcsrx_rx_fec_correct_cnt_lower_set(uint32_t rxfecdecerrcorcntlower);
bdmf_error_t ag_drv_xpcsrx_rx_fec_correct_cnt_lower_get(uint32_t *rxfecdecerrcorcntlower);
bdmf_error_t ag_drv_xpcsrx_rx_fec_correct_cnt_upper_get(uint8_t *rxfecdecerrcorcntupper);
bdmf_error_t ag_drv_xpcsrx_rx_fec_correct_cnt_shadow_set(uint8_t rxfecdecerrcorcntshadow);
bdmf_error_t ag_drv_xpcsrx_rx_fec_correct_cnt_shadow_get(uint8_t *rxfecdecerrcorcntshadow);
bdmf_error_t ag_drv_xpcsrx_rx_fec_ones_cor_cnt_lower_set(uint32_t rxfecdeconescorcntlower);
bdmf_error_t ag_drv_xpcsrx_rx_fec_ones_cor_cnt_lower_get(uint32_t *rxfecdeconescorcntlower);
bdmf_error_t ag_drv_xpcsrx_rx_fec_ones_cor_cnt_upper_get(uint8_t *rxfecdeconescorcntupper);
bdmf_error_t ag_drv_xpcsrx_rx_fec_ones_cor_cnt_shadow_set(uint8_t rxfecdeconescorcntshadow);
bdmf_error_t ag_drv_xpcsrx_rx_fec_ones_cor_cnt_shadow_get(uint8_t *rxfecdeconescorcntshadow);
bdmf_error_t ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_lower_set(uint32_t rxfecdeczeroscorcntlower);
bdmf_error_t ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_lower_get(uint32_t *rxfecdeczeroscorcntlower);
bdmf_error_t ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_upper_get(uint8_t *rxfecdeczeroscorcntupper);
bdmf_error_t ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_shadow_set(uint8_t rxfecdeczeroscorcntshadow);
bdmf_error_t ag_drv_xpcsrx_rx_fec_zeros_cor_cnt_shadow_get(uint8_t *rxfecdeczeroscorcntshadow);
bdmf_error_t ag_drv_xpcsrx_rx_fec_stop_on_err_read_pointer_get(uint8_t *rxfecstoponerrrdptr, uint8_t *rxfecstoponerrwrptr);
bdmf_error_t ag_drv_xpcsrx_rx_fec_stop_on_err_burst_location_get(uint32_t *rxfecstoponerrbrstloc);
bdmf_error_t ag_drv_xpcsrx_rx_64b66b_fail_cnt_set(uint32_t rx64b66bdecerrcnt);
bdmf_error_t ag_drv_xpcsrx_rx_64b66b_fail_cnt_get(uint32_t *rx64b66bdecerrcnt);
bdmf_error_t ag_drv_xpcsrx_rx_frmr_bad_sh_cnt_set(uint32_t rxfrmrbadshcnt);
bdmf_error_t ag_drv_xpcsrx_rx_frmr_bad_sh_cnt_get(uint32_t *rxfrmrbadshcnt);
bdmf_error_t ag_drv_xpcsrx_rx_psudo_cnt_set(uint32_t rxtestpsudoerrcnt);
bdmf_error_t ag_drv_xpcsrx_rx_psudo_cnt_get(uint32_t *rxtestpsudoerrcnt);
bdmf_error_t ag_drv_xpcsrx_rx_prbs_cnt_set(uint32_t rxtestprbserrcnt);
bdmf_error_t ag_drv_xpcsrx_rx_prbs_cnt_get(uint32_t *rxtestprbserrcnt);
bdmf_error_t ag_drv_xpcsrx_rx_ber_intval_set(uint32_t cfgxpcsrxfrmrberintval);
bdmf_error_t ag_drv_xpcsrx_rx_ber_intval_get(uint32_t *cfgxpcsrxfrmrberintval);
bdmf_error_t ag_drv_xpcsrx_rx_ber_tresh_set(uint16_t cfgxpcsrxfrmrbertresh);
bdmf_error_t ag_drv_xpcsrx_rx_ber_tresh_get(uint16_t *cfgxpcsrxfrmrbertresh);
bdmf_error_t ag_drv_xpcsrx_rx_64b66b_start_cnt_set(uint32_t rx64b66bdecstartcnt);
bdmf_error_t ag_drv_xpcsrx_rx_64b66b_start_cnt_get(uint32_t *rx64b66bdecstartcnt);
bdmf_error_t ag_drv_xpcsrx_rx_idle_good_pkt_cnt_set(uint32_t rxidlegoodpktcnt);
bdmf_error_t ag_drv_xpcsrx_rx_idle_good_pkt_cnt_get(uint32_t *rxidlegoodpktcnt);
bdmf_error_t ag_drv_xpcsrx_rx_idle_err_pkt_cnt_set(uint32_t rxidleerrpktcnt);
bdmf_error_t ag_drv_xpcsrx_rx_idle_err_pkt_cnt_get(uint32_t *rxidleerrpktcnt);
bdmf_error_t ag_drv_xpcsrx_rx_64b66b_stop_cnt_set(uint32_t rx64b66bdecstopcnt);
bdmf_error_t ag_drv_xpcsrx_rx_64b66b_stop_cnt_get(uint32_t *rx64b66bdecstopcnt);
bdmf_error_t ag_drv_xpcsrx_rx_burst_out_odr_cnt_set(uint32_t rxburstseqoutofordercnt);
bdmf_error_t ag_drv_xpcsrx_rx_burst_out_odr_cnt_get(uint32_t *rxburstseqoutofordercnt);
bdmf_error_t ag_drv_xpcsrx_rx_idle_da_jit_dly_get(uint16_t *rxidlelastdacnt, uint16_t *rxidledacnt);
bdmf_error_t ag_drv_xpcsrx_rx_dport_ctl_set(const xpcsrx_rx_dport_ctl *rx_dport_ctl);
bdmf_error_t ag_drv_xpcsrx_rx_dport_ctl_get(xpcsrx_rx_dport_ctl *rx_dport_ctl);
bdmf_error_t ag_drv_xpcsrx_rx_dport_data0_set(uint32_t xpcsrxdpdata0);
bdmf_error_t ag_drv_xpcsrx_rx_dport_data0_get(uint32_t *xpcsrxdpdata0);
bdmf_error_t ag_drv_xpcsrx_rx_dport_data1_set(uint32_t xpcsrxdpdata1);
bdmf_error_t ag_drv_xpcsrx_rx_dport_data1_get(uint32_t *xpcsrxdpdata1);
bdmf_error_t ag_drv_xpcsrx_rx_dport_data2_set(uint32_t xpcsrxdpdata2);
bdmf_error_t ag_drv_xpcsrx_rx_dport_data2_get(uint32_t *xpcsrxdpdata2);
bdmf_error_t ag_drv_xpcsrx_rx_dport_acc_set(bdmf_boolean cfgxpcsrxidleramdpsel, bdmf_boolean cfgxpcsrxfecdecramdpsel, bdmf_boolean cfgxpcsrxfecnqueramdpsel);
bdmf_error_t ag_drv_xpcsrx_rx_dport_acc_get(bdmf_boolean *cfgxpcsrxidleramdpsel, bdmf_boolean *cfgxpcsrxfecdecramdpsel, bdmf_boolean *cfgxpcsrxfecnqueramdpsel);
bdmf_error_t ag_drv_xpcsrx_rx_ram_ecc_int_stat_set(bdmf_boolean intrxidleraminitdone, bdmf_boolean intrxfecnqueraminitdone, bdmf_boolean intrxfecdecraminitdone);
bdmf_error_t ag_drv_xpcsrx_rx_ram_ecc_int_stat_get(bdmf_boolean *intrxidleraminitdone, bdmf_boolean *intrxfecnqueraminitdone, bdmf_boolean *intrxfecdecraminitdone);
bdmf_error_t ag_drv_xpcsrx_rx_ram_ecc_int_msk_set(bdmf_boolean mskrxidleraminitdone, bdmf_boolean mskrxfecnqueraminitdone, bdmf_boolean mskrxfecdecraminitdone);
bdmf_error_t ag_drv_xpcsrx_rx_ram_ecc_int_msk_get(bdmf_boolean *mskrxidleraminitdone, bdmf_boolean *mskrxfecnqueraminitdone, bdmf_boolean *mskrxfecdecraminitdone);
bdmf_error_t ag_drv_xpcsrx_rx_dft_testmode_set(uint16_t tm_pd);
bdmf_error_t ag_drv_xpcsrx_rx_dft_testmode_get(uint16_t *tm_pd);
bdmf_error_t ag_drv_xpcsrx_rx_ram_power_pda_ctl0_set(bdmf_boolean cfgxpcsrxidlerampda, bdmf_boolean cfgxpcsrxfecdecrampda);
bdmf_error_t ag_drv_xpcsrx_rx_ram_power_pda_ctl0_get(bdmf_boolean *cfgxpcsrxidlerampda, bdmf_boolean *cfgxpcsrxfecdecrampda);
bdmf_error_t ag_drv_xpcsrx_rx_int_stat1_set(bdmf_boolean intrx64b66btrailstart, bdmf_boolean intrx64b66btwostop, bdmf_boolean intrx64b66btwostart, bdmf_boolean intrx64b66bleadstop);
bdmf_error_t ag_drv_xpcsrx_rx_int_stat1_get(bdmf_boolean *intrx64b66btrailstart, bdmf_boolean *intrx64b66btwostop, bdmf_boolean *intrx64b66btwostart, bdmf_boolean *intrx64b66bleadstop);
bdmf_error_t ag_drv_xpcsrx_rx_int_msk1_set(bdmf_boolean mskrx64b66btrailstart, bdmf_boolean mskrx64b66btwostop, bdmf_boolean mskrx64b66btwostart, bdmf_boolean mskrx64b66bleadstop);
bdmf_error_t ag_drv_xpcsrx_rx_int_msk1_get(bdmf_boolean *mskrx64b66btrailstart, bdmf_boolean *mskrx64b66btwostop, bdmf_boolean *mskrx64b66btwostart, bdmf_boolean *mskrx64b66bleadstop);
bdmf_error_t ag_drv_xpcsrx_rx_spare_ctl_set(uint32_t cfgxpcsrxspare);
bdmf_error_t ag_drv_xpcsrx_rx_spare_ctl_get(uint32_t *cfgxpcsrxspare);

#ifdef USE_BDMF_SHELL
bdmfmon_handle_t ag_drv_xpcsrx_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif


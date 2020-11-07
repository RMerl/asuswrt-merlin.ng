/*
   Copyright (c) 2015 Broadcom
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

#ifndef _XRDP_DRV_UNIMAC_MIB_AG_H_
#define _XRDP_DRV_UNIMAC_MIB_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"

bdmf_error_t ag_drv_unimac_mib_mib_cntrl_set(uint8_t umac_mib_id, bdmf_boolean tx_cnt_rst, bdmf_boolean runt_cnt_rst, bdmf_boolean rx_cnt_st);
bdmf_error_t ag_drv_unimac_mib_mib_cntrl_get(uint8_t umac_mib_id, bdmf_boolean *tx_cnt_rst, bdmf_boolean *runt_cnt_rst, bdmf_boolean *rx_cnt_st);
bdmf_error_t ag_drv_unimac_mib_gr64_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gr64_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gr127_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gr127_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gr255_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gr255_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gr511_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gr511_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gr1023_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gr1023_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gr1518_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gr1518_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_grmgv_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_grmgv_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gr2047_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gr2047_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gr4095_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gr4095_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gr9216_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gr9216_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_grpkt_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_grpkt_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_grbyt_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_grbyt_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_grmca_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_grmca_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_grbca_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_grbca_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_grfcs_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_grfcs_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_grxcf_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_grxcf_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_grxpf_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_grxpf_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_grxuo_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_grxuo_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_graln_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_graln_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_grflr_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_grflr_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_grcde_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_grcde_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_grfcr_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_grfcr_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_grovr_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_grovr_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_grjbr_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_grjbr_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_grmtue_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_grmtue_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_grpok_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_grpok_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gruc_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gruc_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_grppp_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_grppp_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_grcrc_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_grcrc_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_rrpkt_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_rrpkt_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_rrund_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_rrund_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_rrfrg_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_rrfrg_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_rrbyt_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_rrbyt_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_tr64_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_tr64_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_tr127_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_tr127_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_tr255_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_tr255_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_tr511_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_tr511_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_tr1023_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_tr1023_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_tr1518_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_tr1518_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_trmgv_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_trmgv_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_tr2047_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_tr2047_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_tr4095_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_tr4095_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_tr9216_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_tr9216_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gtpkt_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gtpkt_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gtmca_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gtmca_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gtbca_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gtbca_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gtxpf_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gtxpf_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gtxcf_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gtxcf_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gtfcs_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gtfcs_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gtovr_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gtovr_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gtdrf_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gtdrf_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gtedf_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gtedf_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gtscl_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gtscl_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gtmcl_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gtmcl_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gtlcl_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gtlcl_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gtxcl_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gtxcl_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gtfrg_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gtfrg_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gtncl_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gtncl_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gtjbr_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gtjbr_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gtbyt_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gtbyt_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gtpok_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gtpok_get(uint8_t umac_mib_id, uint32_t *gr);
bdmf_error_t ag_drv_unimac_mib_gtuc_set(uint8_t umac_mib_id, uint32_t gr);
bdmf_error_t ag_drv_unimac_mib_gtuc_get(uint8_t umac_mib_id, uint32_t *gr);

#ifdef USE_BDMF_SHELL
enum
{
    cli_unimac_mib_mib_cntrl,
    cli_unimac_mib_gr64,
    cli_unimac_mib_gr127,
    cli_unimac_mib_gr255,
    cli_unimac_mib_gr511,
    cli_unimac_mib_gr1023,
    cli_unimac_mib_gr1518,
    cli_unimac_mib_grmgv,
    cli_unimac_mib_gr2047,
    cli_unimac_mib_gr4095,
    cli_unimac_mib_gr9216,
    cli_unimac_mib_grpkt,
    cli_unimac_mib_grbyt,
    cli_unimac_mib_grmca,
    cli_unimac_mib_grbca,
    cli_unimac_mib_grfcs,
    cli_unimac_mib_grxcf,
    cli_unimac_mib_grxpf,
    cli_unimac_mib_grxuo,
    cli_unimac_mib_graln,
    cli_unimac_mib_grflr,
    cli_unimac_mib_grcde,
    cli_unimac_mib_grfcr,
    cli_unimac_mib_grovr,
    cli_unimac_mib_grjbr,
    cli_unimac_mib_grmtue,
    cli_unimac_mib_grpok,
    cli_unimac_mib_gruc,
    cli_unimac_mib_grppp,
    cli_unimac_mib_grcrc,
    cli_unimac_mib_rrpkt,
    cli_unimac_mib_rrund,
    cli_unimac_mib_rrfrg,
    cli_unimac_mib_rrbyt,
    cli_unimac_mib_tr64,
    cli_unimac_mib_tr127,
    cli_unimac_mib_tr255,
    cli_unimac_mib_tr511,
    cli_unimac_mib_tr1023,
    cli_unimac_mib_tr1518,
    cli_unimac_mib_trmgv,
    cli_unimac_mib_tr2047,
    cli_unimac_mib_tr4095,
    cli_unimac_mib_tr9216,
    cli_unimac_mib_gtpkt,
    cli_unimac_mib_gtmca,
    cli_unimac_mib_gtbca,
    cli_unimac_mib_gtxpf,
    cli_unimac_mib_gtxcf,
    cli_unimac_mib_gtfcs,
    cli_unimac_mib_gtovr,
    cli_unimac_mib_gtdrf,
    cli_unimac_mib_gtedf,
    cli_unimac_mib_gtscl,
    cli_unimac_mib_gtmcl,
    cli_unimac_mib_gtlcl,
    cli_unimac_mib_gtxcl,
    cli_unimac_mib_gtfrg,
    cli_unimac_mib_gtncl,
    cli_unimac_mib_gtjbr,
    cli_unimac_mib_gtbyt,
    cli_unimac_mib_gtpok,
    cli_unimac_mib_gtuc,
};

int bcm_unimac_mib_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_unimac_mib_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif


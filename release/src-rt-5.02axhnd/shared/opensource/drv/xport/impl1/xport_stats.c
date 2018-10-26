/*
   Copyright (c) 2017 Broadcom Corporation
   All Rights Reserved

   <:label-BRCM:2017:DUAL/GPL:standard

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
/*
 * xport_stats.c
 */

#include "xport_defs.h"
#include "xport_stats.h"
#include "xport_ag.h"

int xport_stats_get_rx(uint32_t portid, xport_rx_stats_s *rx_stats)
{
    int rc;
    rc = ag_drv_xport_mib_core_grx64_get(portid, &rx_stats->GRX64);
    rc = rc ? rc :ag_drv_xport_mib_core_grx127_get(portid, &rx_stats->GRX127);
    rc = rc ? rc :ag_drv_xport_mib_core_grx255_get(portid, &rx_stats->GRX255);
    rc = rc ? rc :ag_drv_xport_mib_core_grx511_get(portid,&rx_stats->GRX511);
    rc = rc ? rc :ag_drv_xport_mib_core_grx1023_get(portid,&rx_stats->GRX1023);
    rc = rc ? rc :ag_drv_xport_mib_core_grx1518_get(portid,&rx_stats->GRX1518);
    rc = rc ? rc :ag_drv_xport_mib_core_grx1522_get(portid,&rx_stats->GRX1522);
    rc = rc ? rc :ag_drv_xport_mib_core_grx2047_get(portid,&rx_stats->GRX2047);
    rc = rc ? rc :ag_drv_xport_mib_core_grx4095_get(portid,&rx_stats->GRX4095);
    rc = rc ? rc :ag_drv_xport_mib_core_grx9216_get(portid,&rx_stats->GRX9216);
    rc = rc ? rc :ag_drv_xport_mib_core_grx16383_get(portid,&rx_stats->GRX16383);
    rc = rc ? rc :ag_drv_xport_mib_core_grxpkt_get(portid,&rx_stats->GRXPKT);
    rc = rc ? rc :ag_drv_xport_mib_core_grxuca_get(portid,&rx_stats->GRXUCA);
    rc = rc ? rc :ag_drv_xport_mib_core_grxmca_get(portid,&rx_stats->GRXMCA);
    rc = rc ? rc :ag_drv_xport_mib_core_grxbca_get(portid,&rx_stats->GRXBCA);
    rc = rc ? rc :ag_drv_xport_mib_core_grxfcs_get(portid,&rx_stats->GRXFCS);
    rc = rc ? rc :ag_drv_xport_mib_core_grxcf_get(portid,&rx_stats->GRXCF);
    rc = rc ? rc :ag_drv_xport_mib_core_grxpp_get(portid,&rx_stats->GRXPP);
    rc = rc ? rc :ag_drv_xport_mib_core_grxpf_get(portid,&rx_stats->GRXPF);
    rc = rc ? rc :ag_drv_xport_mib_core_grxuo_get(portid,&rx_stats->GRXUDA);
    rc = rc ? rc :ag_drv_xport_mib_core_grxwsa_get(portid,&rx_stats->GRXWSA);
    rc = rc ? rc :ag_drv_xport_mib_core_grxaln_get(portid,&rx_stats->GRXALN);
    rc = rc ? rc :ag_drv_xport_mib_core_grxflr_get(portid,&rx_stats->GRXFLR);
    rc = rc ? rc :ag_drv_xport_mib_core_grxfrerr_get(portid,&rx_stats->GRXFRERR);
    rc = rc ? rc :ag_drv_xport_mib_core_grxfcr_get(portid,&rx_stats->GRXFCR);
    rc = rc ? rc :ag_drv_xport_mib_core_grxovr_get(portid,&rx_stats->GRXOVR);
    rc = rc ? rc :ag_drv_xport_mib_core_grxjbr_get(portid,&rx_stats->GRXJBR);
    rc = rc ? rc :ag_drv_xport_mib_core_grxmtue_get(portid,&rx_stats->GRXMTUE);
    rc = rc ? rc :ag_drv_xport_mib_core_grxmcrc_get(portid,&rx_stats->GRXMCRC);
    rc = rc ? rc :ag_drv_xport_mib_core_grxprm_get(portid,&rx_stats->GRXPRM);
    rc = rc ? rc :ag_drv_xport_mib_core_grxvln_get(portid,&rx_stats->GRXVLN);
    rc = rc ? rc :ag_drv_xport_mib_core_grxdvln_get(portid,&rx_stats->GRXDVLN);
    rc = rc ? rc :ag_drv_xport_mib_core_grxtrfu_get(portid,&rx_stats->GRXTRFU);
    rc = rc ? rc :ag_drv_xport_mib_core_grxpok_get(portid,&rx_stats->GRXPOK);
    rc = rc ? rc :ag_drv_xport_mib_core_grxschcrc_get(portid, &rx_stats->GRXSCHCRC);
    rc = rc ? rc :ag_drv_xport_mib_core_grxbyt_get(portid,&rx_stats->GRXBYT);
    rc = rc ? rc :ag_drv_xport_mib_core_grxrpkt_get(portid,&rx_stats->GRXRPKT);
    rc = rc ? rc :ag_drv_xport_mib_core_grxund_get(portid,&rx_stats->GRXUND);
    rc = rc ? rc :ag_drv_xport_mib_core_grxfrg_get(portid,&rx_stats->GRXFRG);
    rc = rc ? rc :ag_drv_xport_mib_core_grxrbyt_get(portid,&rx_stats->GRXRBYT);
    rc = rc ? rc :ag_drv_xport_mib_core_grxlpi_get(portid,&rx_stats->GRXLPI);
    rc = rc ? rc :ag_drv_xport_mib_core_grxdlpi_get(portid,&rx_stats->GRXDLPI);
    rc = rc ? rc :ag_drv_xport_mib_core_grxptllfc_get(portid,&rx_stats->GRXPTLLFC);
    rc = rc ? rc :ag_drv_xport_mib_core_grxltllfc_get(portid,&rx_stats->GRXLTLLFC);
    rc = rc ? rc :ag_drv_xport_mib_core_grxllfcfcs_get(portid,&rx_stats->GRXLLFCFCS);
    rc = rc ? rc :ag_drv_xport_mib_core_grxuo_get(portid, &rx_stats->GRXUO);
    return rc;
}

int xport_stats_get_tx(uint32_t portid, xport_tx_stats_s *tx_stats)
{
    int rc;
    rc = ag_drv_xport_mib_core_gtx64_get(portid, &tx_stats->GTX64);
    rc = rc ? rc :ag_drv_xport_mib_core_gtx127_get(portid, &tx_stats->GTX127);
    rc = rc ? rc :ag_drv_xport_mib_core_gtx255_get(portid, &tx_stats->GTX255);
    rc = rc ? rc :ag_drv_xport_mib_core_gtx511_get(portid, &tx_stats->GTX511);
    rc = rc ? rc :ag_drv_xport_mib_core_gtx1023_get(portid, &tx_stats->GTX1023);
    rc = rc ? rc :ag_drv_xport_mib_core_gtx1518_get(portid, &tx_stats->GTX1518);
    rc = rc ? rc :ag_drv_xport_mib_core_gtx1522_get(portid, &tx_stats->GTX1522);
    rc = rc ? rc :ag_drv_xport_mib_core_gtx2047_get(portid, &tx_stats->GTX2047);
    rc = rc ? rc :ag_drv_xport_mib_core_gtx4095_get(portid, &tx_stats->GTX4095);
    rc = rc ? rc :ag_drv_xport_mib_core_gtx9216_get(portid, &tx_stats->GTX9216);
    rc = rc ? rc :ag_drv_xport_mib_core_gtx16383_get(portid, &tx_stats->GTX16383);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxpok_get(portid, &tx_stats->GTXPOK);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxpkt_get(portid, &tx_stats->GTXPKT);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxuca_get(portid, &tx_stats->GTXUCA);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxmca_get(portid, &tx_stats->GTXMCA);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxbca_get(portid, &tx_stats->GTXBCA);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxpf_get(portid, &tx_stats->GTXPF);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxpfc_get(portid, &tx_stats->GTXPFC);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxjbr_get(portid, &tx_stats->GTXJBR);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxfcs_get(portid, &tx_stats->GTXFCS);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxcf_get(portid, &tx_stats->GTXCF);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxovr_get(portid, &tx_stats->GTXOVR);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxdfr_get(portid, &tx_stats->GTXDFR);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxedf_get(portid, &tx_stats->GTXEDF);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxscl_get(portid, &tx_stats->GTXSCL);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxmcl_get(portid, &tx_stats->GTXMCL);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxlcl_get(portid, &tx_stats->GTXLCL);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxxcl_get(portid, &tx_stats->GTXXCL);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxfrg_get(portid, &tx_stats->GTXFRG);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxerr_get(portid, &tx_stats->GTXERR);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxvln_get(portid, &tx_stats->GTXVLN);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxdvln_get(portid, &tx_stats->GTXDVLN);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxrpkt_get(portid, &tx_stats->GTXRPKT);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxufl_get(portid, &tx_stats->GTXUFL);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxncl_get(portid, &tx_stats->GTXNCL);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxbyt_get(portid, &tx_stats->GTXBYT);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxlpi_get(portid, &tx_stats->GTXLPI);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxdlpi_get(portid, &tx_stats->GTXDLPI);
    rc = rc ? rc :ag_drv_xport_mib_core_gtxltllfc_get(portid, &tx_stats->GTXLTLLFC);
    return rc;
}

int xport_stats_rst_rx_single(uint32_t portid,  XPORT_STATS_RX_TYPE stat_type)
{
    switch(stat_type)
    {
    case (STAT_GRX64):
        return ag_drv_xport_mib_core_gtx64_set(portid, 0);
    case (STAT_GRX127):
        return ag_drv_xport_mib_core_grx127_set(portid, 0);
    case (STAT_GRX255):
        return ag_drv_xport_mib_core_grx255_set(portid, 0);
    case (STAT_GRX511):
        return ag_drv_xport_mib_core_grx511_set(portid, 0);
    case (STAT_GRX1023):
        return ag_drv_xport_mib_core_grx1023_set(portid, 0);
    case (STAT_GRX1518):
        return ag_drv_xport_mib_core_grx1518_set(portid, 0);
    case (STAT_GRX1522):
        return ag_drv_xport_mib_core_grx1522_set(portid, 0);
    case (STAT_GRX2047):
        return ag_drv_xport_mib_core_grx2047_set(portid, 0);
    case (STAT_GRX4095):
        return ag_drv_xport_mib_core_grx4095_set(portid, 0);
    case (STAT_GRX9216):
        return ag_drv_xport_mib_core_grx9216_set(portid, 0);
    case (STAT_GRX16383):
        return ag_drv_xport_mib_core_grx16383_set(portid, 0);
    case (STAT_GRXPKT):
        return ag_drv_xport_mib_core_grxpkt_set(portid, 0);
    case (STAT_GRXUCA):
        return ag_drv_xport_mib_core_grxuca_set(portid, 0);
    case (STAT_GRXMCA):
        return ag_drv_xport_mib_core_grxmca_set(portid, 0);
    case (STAT_GRXBCA):
        return ag_drv_xport_mib_core_grxbca_set(portid, 0);
    case (STAT_GRXFCS):
        return ag_drv_xport_mib_core_grxfcs_set(portid, 0);
    case (STAT_GRXCF):
        return ag_drv_xport_mib_core_grxcf_set(portid, 0);
    case (STAT_GRXPF):
        return ag_drv_xport_mib_core_grxpp_set(portid, 0);
    case (STAT_GRXPP):
        return ag_drv_xport_mib_core_grxuo_set(portid, 0);
    case (STAT_GRXUO):
        return ag_drv_xport_mib_core_grxuda_set(portid, 0);
    case (STAT_GRXUDA):
        return ag_drv_xport_mib_core_grxwsa_set(portid, 0);
    case (STAT_GRXWSA):
        return ag_drv_xport_mib_core_grxaln_set(portid, 0);
    case (STAT_GRXALN):
        return ag_drv_xport_mib_core_grxflr_set(portid, 0);
    case (STAT_GRXFLR):
        return ag_drv_xport_mib_core_grxflr_set(portid, 0);
    case (STAT_GRXFRERR):
        return ag_drv_xport_mib_core_grxfrerr_set(portid, 0);
    case (STAT_GRXFCR):
        return ag_drv_xport_mib_core_grxfcr_set(portid, 0);
    case (STAT_GRXOVR):
        return ag_drv_xport_mib_core_grxovr_set(portid, 0);
    case (STAT_GRXJBR):
        return ag_drv_xport_mib_core_grxjbr_set(portid, 0);
    case (STAT_GRXMTUE):
        return ag_drv_xport_mib_core_grxmtue_set(portid, 0);
    case (STAT_GRXMCRC):
        return ag_drv_xport_mib_core_grxmcrc_set(portid, 0);
    case (STAT_GRXPRM):
        return ag_drv_xport_mib_core_grxprm_set(portid, 0);
    case (STAT_GRXVLN):
        return ag_drv_xport_mib_core_grxvln_set(portid, 0);
    case (STAT_GRXDVLN):
        return ag_drv_xport_mib_core_grxdvln_set(portid, 0);
    case (STAT_GRXTRFU):
        return ag_drv_xport_mib_core_grxtrfu_set(portid, 0);
    case (STAT_GRXPOK):
        return ag_drv_xport_mib_core_grxpok_set(portid, 0);
    case (STAT_GRXSCHCRC):
        return ag_drv_xport_mib_core_grxschcrc_set(portid, 0);
    case (STAT_GRXBYT):
        return ag_drv_xport_mib_core_grxbyt_set(portid, 0);
    case (STAT_GRXRPKT):
        return ag_drv_xport_mib_core_grxrpkt_set(portid, 0);
    case (STAT_GRXUND):
        return ag_drv_xport_mib_core_grxund_set(portid, 0);
    case (STAT_GRXFRG):
        return ag_drv_xport_mib_core_grxfrg_set(portid, 0);
    case (STAT_GRXRBYT):
        return ag_drv_xport_mib_core_grxrbyt_set(portid, 0);
    case (STAT_GRXLPI):
        return ag_drv_xport_mib_core_grxlpi_set(portid, 0);
    case (STAT_GRXDLPI):
        return ag_drv_xport_mib_core_grxdlpi_set(portid, 0);
    case (STAT_GRXPTLLFC):
        return ag_drv_xport_mib_core_grxptllfc_set(portid, 0);
    case (STAT_GRXLTLLFC):
        return ag_drv_xport_mib_core_grxltllfc_set(portid, 0);
    case (STAT_GRXLLFCFCS):
        return ag_drv_xport_mib_core_grxllfcfcs_set(portid, 0);
    default:
        return XPORT_ERR_PARAM;
    }

    return XPORT_ERR_OK;
}

int xport_stats_rst_tx_single(uint32_t portid,  XPORT_STATS_TX_TYPE stat_type)
{
    switch(stat_type)
    {
    case(STAT_GTX64):
        return ag_drv_xport_mib_core_gtx64_set(portid, 0);
    case(STAT_GTX127):
        return ag_drv_xport_mib_core_gtx127_set(portid, 0);
    case(STAT_GTX255):
        return ag_drv_xport_mib_core_gtx255_set(portid, 0);
    case(STAT_GTX511):
        return ag_drv_xport_mib_core_gtx511_set(portid, 0);
    case(STAT_GTX1023):
        return ag_drv_xport_mib_core_gtx1023_set(portid, 0);
    case(STAT_GTX1518):
        return ag_drv_xport_mib_core_gtx1518_set(portid, 0);
    case(STAT_GTX1522):
        return ag_drv_xport_mib_core_gtx1522_set(portid, 0);
    case(STAT_GTX2047):
        return ag_drv_xport_mib_core_gtx2047_set(portid, 0);
    case(STAT_GTX4095):
        return ag_drv_xport_mib_core_gtx4095_set(portid, 0);
    case(STAT_GTX9216):
        return ag_drv_xport_mib_core_gtx9216_set(portid, 0);
    case(STAT_GTX16383):
        return ag_drv_xport_mib_core_gtx16383_set(portid, 0);
    case(STAT_GTXPOK):
        return ag_drv_xport_mib_core_gtxpok_set(portid, 0);
    case(STAT_GTXPKT):
        return ag_drv_xport_mib_core_gtxpkt_set(portid, 0);
    case(STAT_GTXUCA):
        return ag_drv_xport_mib_core_gtxuca_set(portid, 0);
    case(STAT_GTXMCA):
        return ag_drv_xport_mib_core_gtxmca_set(portid, 0);
    case(STAT_GTXBCA):
        return ag_drv_xport_mib_core_gtxbca_set(portid, 0);
    case(STAT_GTXPF):
        return ag_drv_xport_mib_core_gtxpf_set(portid, 0);
    case(STAT_GTXPFC):
        return ag_drv_xport_mib_core_gtxpfc_set(portid, 0);
    case(STAT_GTXJBR):
        return ag_drv_xport_mib_core_gtxjbr_set(portid, 0);
    case(STAT_GTXFCS):
        return ag_drv_xport_mib_core_gtxfcs_set(portid, 0);
    case(STAT_GTXCF):
        return ag_drv_xport_mib_core_gtxcf_set(portid, 0);
    case(STAT_GTXOVR):
        return ag_drv_xport_mib_core_gtxovr_set(portid, 0);
    case(STAT_GTXDFR):
        return ag_drv_xport_mib_core_gtxdfr_set(portid, 0);
    case(STAT_GTXEDF):
        return ag_drv_xport_mib_core_gtxedf_set(portid, 0);
    case(STAT_GTXSCL):
        return ag_drv_xport_mib_core_gtxscl_set(portid, 0);
    case(STAT_GTXMCL):
        return ag_drv_xport_mib_core_gtxmcl_set(portid, 0);
    case(STAT_GTXLCL):
        return ag_drv_xport_mib_core_gtxlcl_set(portid, 0);
    case(STAT_GTXXCL):
        return ag_drv_xport_mib_core_gtxxcl_set(portid, 0);
    case(STAT_GTXFRG):
        return ag_drv_xport_mib_core_gtxfrg_set(portid, 0);
    case(STAT_GTXERR):
        return ag_drv_xport_mib_core_gtxerr_set(portid, 0);
    case(STAT_GTXVLN):
        return ag_drv_xport_mib_core_gtxvln_set(portid, 0);
    case(STAT_GTXDVLN):
        return ag_drv_xport_mib_core_gtxdvln_set(portid, 0);
    case(STAT_GTXRPKT):
        return ag_drv_xport_mib_core_gtxrpkt_set(portid, 0);
    case(STAT_GTXUFL):
        return ag_drv_xport_mib_core_gtxufl_set(portid, 0);
    case(STAT_GTXNCL):
        return ag_drv_xport_mib_core_gtxncl_set(portid, 0);
    case(STAT_GTXBYT):
        return ag_drv_xport_mib_core_gtxbyt_set(portid, 0);
    case(STAT_GTXLPI):
        return ag_drv_xport_mib_core_gtxlpi_set(portid, 0);
    case(STAT_GTXDLPI):
        return ag_drv_xport_mib_core_gtxdlpi_set(portid, 0);
    case(STAT_GTXLTLLFC):
        return ag_drv_xport_mib_core_gtxltllfc_set(portid, 0);
    default:
        return XPORT_ERR_PARAM;
    }
    return XPORT_ERR_OK;
}

int xport_stats_cfg_set(uint32_t portid, xport_stats_cfg_s *stats_cfg)
{
    uint8_t instance = portid % 4;
    uint8_t eee_cnt_mode;
    uint8_t saturate_en;
    uint8_t cor_en;
    uint8_t cnt_rst;

    ag_drv_xport_mib_reg_cntrl_get(&eee_cnt_mode, &saturate_en,
        &cor_en, &cnt_rst);

    if(stats_cfg->saturate_en)
        saturate_en |= (1 << instance);
    else
        saturate_en &= ~(1 << instance);

    if(stats_cfg->cor_en)
        cor_en |= (1 << instance);
    else
        cor_en &= ~(1 << instance);

    return ag_drv_xport_mib_reg_cntrl_set(eee_cnt_mode, saturate_en,
        cor_en, cnt_rst);
}

int xport_stats_cfg_get(uint32_t portid, xport_stats_cfg_s *stats_cfg)
{
    uint8_t instance = portid % 4;
    uint8_t eee_sym_mode;
    uint8_t saturate_en;
    uint8_t cor_en;
    uint8_t cnt_rst;

    ag_drv_xport_mib_reg_cntrl_get(&eee_sym_mode, &saturate_en,
        &cor_en, &cnt_rst);

    stats_cfg->saturate_en = (saturate_en & (1 << instance)) != 0;
    stats_cfg->eee_sym_mode = (eee_sym_mode & (1 << instance)) != 0;
    stats_cfg->cor_en =( cor_en & (1 << instance)) != 0;

    return 0;
}

int xport_stats_reset(uint32_t portid)
{
    uint8_t instance = portid % 4;
    uint8_t eee_cnt_mode;
    uint8_t saturate_en;
    uint8_t cor_en;
    uint8_t cnt_rst;

    ag_drv_xport_mib_reg_cntrl_get(&eee_cnt_mode, &saturate_en,
        &cor_en, &cnt_rst);

    cnt_rst |= (1 << instance);

    ag_drv_xport_mib_reg_cntrl_set(eee_cnt_mode, saturate_en,
        cor_en, cnt_rst );

    cnt_rst &= ~(1 << instance);

    ag_drv_xport_mib_reg_cntrl_set(eee_cnt_mode, saturate_en,
        cor_en, cnt_rst );

    return 0;
}

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
/*
 * lport_stats.c
 *
 *  Created on: 16 בספט׳ 2015
 *      Author: yonatani
 */
#include "lport_defs.h"
#include "lport_stats.h"
#include "bcm6858_lport_mib_ag.h"
#include "bcm6858_mib_conf_ag.h"


int lport_stats_get_rx(uint32_t portid, lport_rx_stats_s *rx_stats)
{
    int rc;
    rc = ag_drv_lport_mib_grx64_get(portid, &rx_stats->GRX64);
    rc = rc ? rc :ag_drv_lport_mib_grx127_get(portid, &rx_stats->GRX127);
    rc = rc ? rc :ag_drv_lport_mib_grx255_get(portid, &rx_stats->GRX255);
    rc = rc ? rc :ag_drv_lport_mib_grx511_get(portid,&rx_stats->GRX511);
    rc = rc ? rc :ag_drv_lport_mib_grx1023_get(portid,&rx_stats->GRX1023);
    rc = rc ? rc :ag_drv_lport_mib_grx1518_get(portid,&rx_stats->GRX1518);
    rc = rc ? rc :ag_drv_lport_mib_grx1522_get(portid,&rx_stats->GRX1522);
    rc = rc ? rc :ag_drv_lport_mib_grx2047_get(portid,&rx_stats->GRX2047);
    rc = rc ? rc :ag_drv_lport_mib_grx4095_get(portid,&rx_stats->GRX4095);
    rc = rc ? rc :ag_drv_lport_mib_grx9216_get(portid,&rx_stats->GRX9216);
    rc = rc ? rc :ag_drv_lport_mib_grx16383_get(portid,&rx_stats->GRX16383);
    rc = rc ? rc :ag_drv_lport_mib_grxpkt_get(portid,&rx_stats->GRXPKT);
    rc = rc ? rc :ag_drv_lport_mib_grxuca_get(portid,&rx_stats->GRXUCA);
    rc = rc ? rc :ag_drv_lport_mib_grxmca_get(portid,&rx_stats->GRXMCA);
    rc = rc ? rc :ag_drv_lport_mib_grxbca_get(portid,&rx_stats->GRXBCA);
    rc = rc ? rc :ag_drv_lport_mib_grxfcs_get(portid,&rx_stats->GRXFCS);
    rc = rc ? rc :ag_drv_lport_mib_grxcf_get(portid,&rx_stats->GRXCF);
    rc = rc ? rc :ag_drv_lport_mib_grxpp_get(portid,&rx_stats->GRXPP);
    rc = rc ? rc :ag_drv_lport_mib_grxpf_get(portid,&rx_stats->GRXPF);
    rc = rc ? rc :ag_drv_lport_mib_grxuo_get(portid,&rx_stats->GRXUDA);
    rc = rc ? rc :ag_drv_lport_mib_grxwsa_get(portid,&rx_stats->GRXWSA);
    rc = rc ? rc :ag_drv_lport_mib_grxaln_get(portid,&rx_stats->GRXALN);
    rc = rc ? rc :ag_drv_lport_mib_grxflr_get(portid,&rx_stats->GRXFLR);
    rc = rc ? rc :ag_drv_lport_mib_grxfrerr_get(portid,&rx_stats->GRXFRERR);
    rc = rc ? rc :ag_drv_lport_mib_grxfcr_get(portid,&rx_stats->GRXFCR);
    rc = rc ? rc :ag_drv_lport_mib_grxovr_get(portid,&rx_stats->GRXOVR);
    rc = rc ? rc :ag_drv_lport_mib_grxjbr_get(portid,&rx_stats->GRXJBR);
    rc = rc ? rc :ag_drv_lport_mib_grxmtue_get(portid,&rx_stats->GRXMTUE);
    rc = rc ? rc :ag_drv_lport_mib_grxmcrc_get(portid,&rx_stats->GRXMCRC);
    rc = rc ? rc :ag_drv_lport_mib_grxprm_get(portid,&rx_stats->GRXPRM);
    rc = rc ? rc :ag_drv_lport_mib_grxvln_get(portid,&rx_stats->GRXVLN);
    rc = rc ? rc :ag_drv_lport_mib_grxdvln_get(portid,&rx_stats->GRXDVLN);
    rc = rc ? rc :ag_drv_lport_mib_grxtrfu_get(portid,&rx_stats->GRXTRFU);
    rc = rc ? rc :ag_drv_lport_mib_grxpok_get(portid,&rx_stats->GRXPOK);
    rc = rc ? rc :ag_drv_lport_mib_grxschcrc_get(portid, &rx_stats->GRXSCHCRC);
    rc = rc ? rc :ag_drv_lport_mib_grxbyt_get(portid,&rx_stats->GRXBYT);
    rc = rc ? rc :ag_drv_lport_mib_grxrpkt_get(portid,&rx_stats->GRXRPKT);
    rc = rc ? rc :ag_drv_lport_mib_grxund_get(portid,&rx_stats->GRXUND);
    rc = rc ? rc :ag_drv_lport_mib_grxfrg_get(portid,&rx_stats->GRXFRG);
    rc = rc ? rc :ag_drv_lport_mib_grxrbyt_get(portid,&rx_stats->GRXRBYT);
    rc = rc ? rc :ag_drv_lport_mib_grxlpi_get(portid,&rx_stats->GRXLPI);
    rc = rc ? rc :ag_drv_lport_mib_grxdlpi_get(portid,&rx_stats->GRXDLPI);
    rc = rc ? rc :ag_drv_lport_mib_grxptllfc_get(portid,&rx_stats->GRXPTLLFC);
    rc = rc ? rc :ag_drv_lport_mib_grxltllfc_get(portid,&rx_stats->GRXLTLLFC);
    rc = rc ? rc :ag_drv_lport_mib_grxllfcfcs_get(portid,&rx_stats->GRXLLFCFCS);
    rc = rc ? rc :ag_drv_lport_mib_grxuo_get(portid, &rx_stats->GRXUO);
    return rc;
}

int lport_stats_get_tx(uint32_t portid, lport_tx_stats_s *tx_stats)
{
    int rc;
    rc = ag_drv_lport_mib_gtx64_get(portid, &tx_stats->GTX64);
    rc = rc ? rc :ag_drv_lport_mib_gtx127_get(portid, &tx_stats->GTX127);
    rc = rc ? rc :ag_drv_lport_mib_gtx255_get(portid, &tx_stats->GTX255);
    rc = rc ? rc :ag_drv_lport_mib_gtx511_get(portid, &tx_stats->GTX511);
    rc = rc ? rc :ag_drv_lport_mib_gtx1023_get(portid, &tx_stats->GTX1023);
    rc = rc ? rc :ag_drv_lport_mib_gtx1518_get(portid, &tx_stats->GTX1518);
    rc = rc ? rc :ag_drv_lport_mib_gtx1522_get(portid, &tx_stats->GTX1522);
    rc = rc ? rc :ag_drv_lport_mib_gtx2047_get(portid, &tx_stats->GTX2047);
    rc = rc ? rc :ag_drv_lport_mib_gtx4095_get(portid, &tx_stats->GTX4095);
    rc = rc ? rc :ag_drv_lport_mib_gtx9216_get(portid, &tx_stats->GTX9216);
    rc = rc ? rc :ag_drv_lport_mib_gtx16383_get(portid, &tx_stats->GTX16383);
    rc = rc ? rc :ag_drv_lport_mib_gtxpok_get(portid, &tx_stats->GTXPOK);
    rc = rc ? rc :ag_drv_lport_mib_gtxpkt_get(portid, &tx_stats->GTXPKT);
    rc = rc ? rc :ag_drv_lport_mib_gtxuca_get(portid, &tx_stats->GTXUCA);
    rc = rc ? rc :ag_drv_lport_mib_gtxmca_get(portid, &tx_stats->GTXMCA);
    rc = rc ? rc :ag_drv_lport_mib_gtxbca_get(portid, &tx_stats->GTXBCA);
    rc = rc ? rc :ag_drv_lport_mib_gtxpf_get(portid, &tx_stats->GTXPF);
    rc = rc ? rc :ag_drv_lport_mib_gtxpfc_get(portid, &tx_stats->GTXPFC);
    rc = rc ? rc :ag_drv_lport_mib_gtxjbr_get(portid, &tx_stats->GTXJBR);
    rc = rc ? rc :ag_drv_lport_mib_gtxfcs_get(portid, &tx_stats->GTXFCS);
    rc = rc ? rc :ag_drv_lport_mib_gtxcf_get(portid, &tx_stats->GTXCF);
    rc = rc ? rc :ag_drv_lport_mib_gtxovr_get(portid, &tx_stats->GTXOVR);
    rc = rc ? rc :ag_drv_lport_mib_gtxdfr_get(portid, &tx_stats->GTXDFR);
    rc = rc ? rc :ag_drv_lport_mib_gtxedf_get(portid, &tx_stats->GTXEDF);
    rc = rc ? rc :ag_drv_lport_mib_gtxscl_get(portid, &tx_stats->GTXSCL);
    rc = rc ? rc :ag_drv_lport_mib_gtxmcl_get(portid, &tx_stats->GTXMCL);
    rc = rc ? rc :ag_drv_lport_mib_gtxlcl_get(portid, &tx_stats->GTXLCL);
    rc = rc ? rc :ag_drv_lport_mib_gtxxcl_get(portid, &tx_stats->GTXXCL);
    rc = rc ? rc :ag_drv_lport_mib_gtxfrg_get(portid, &tx_stats->GTXFRG);
    rc = rc ? rc :ag_drv_lport_mib_gtxerr_get(portid, &tx_stats->GTXERR);
    rc = rc ? rc :ag_drv_lport_mib_gtxvln_get(portid, &tx_stats->GTXVLN);
    rc = rc ? rc :ag_drv_lport_mib_gtxdvln_get(portid, &tx_stats->GTXDVLN);
    rc = rc ? rc :ag_drv_lport_mib_gtxrpkt_get(portid, &tx_stats->GTXRPKT);
    rc = rc ? rc :ag_drv_lport_mib_gtxufl_get(portid, &tx_stats->GTXUFL);
    rc = rc ? rc :ag_drv_lport_mib_gtxncl_get(portid, &tx_stats->GTXNCL);
    rc = rc ? rc :ag_drv_lport_mib_gtxbyt_get(portid, &tx_stats->GTXBYT);
    rc = rc ? rc :ag_drv_lport_mib_gtxlpi_get(portid, &tx_stats->GTXLPI);
    rc = rc ? rc :ag_drv_lport_mib_gtxdlpi_get(portid, &tx_stats->GTXDLPI);
    rc = rc ? rc :ag_drv_lport_mib_gtxltllfc_get(portid, &tx_stats->GTXLTLLFC);
    return rc;
}

int lport_stats_rst_rx_single(uint32_t portid,  LPORT_STATS_RX_TYPE stat_type)
{
    switch(stat_type)
    {
    case (STAT_GRX64):
        return ag_drv_lport_mib_gtx64_set(portid, 0);
    case (STAT_GRX127):
        return ag_drv_lport_mib_grx127_set(portid, 0);
    case (STAT_GRX255):
        return ag_drv_lport_mib_grx255_set(portid, 0);
    case (STAT_GRX511):
        return ag_drv_lport_mib_grx511_set(portid, 0);
    case (STAT_GRX1023):
        return ag_drv_lport_mib_grx1023_set(portid, 0);
    case (STAT_GRX1518):
        return ag_drv_lport_mib_grx1518_set(portid, 0);
    case (STAT_GRX1522):
        return ag_drv_lport_mib_grx1522_set(portid, 0);
    case (STAT_GRX2047):
        return ag_drv_lport_mib_grx2047_set(portid, 0);
    case (STAT_GRX4095):
        return ag_drv_lport_mib_grx4095_set(portid, 0);
    case (STAT_GRX9216):
        return ag_drv_lport_mib_grx9216_set(portid, 0);
    case (STAT_GRX16383):
        return ag_drv_lport_mib_grx16383_set(portid, 0);
    case (STAT_GRXPKT):
        return ag_drv_lport_mib_grxpkt_set(portid, 0);
    case (STAT_GRXUCA):
        return ag_drv_lport_mib_grxuca_set(portid, 0);
    case (STAT_GRXMCA):
        return ag_drv_lport_mib_grxmca_set(portid, 0);
    case (STAT_GRXBCA):
        return ag_drv_lport_mib_grxbca_set(portid, 0);
    case (STAT_GRXFCS):
        return ag_drv_lport_mib_grxfcs_set(portid, 0);
    case (STAT_GRXCF):
        return ag_drv_lport_mib_grxcf_set(portid, 0);
    case (STAT_GRXPF):
        return ag_drv_lport_mib_grxpp_set(portid, 0);
    case (STAT_GRXPP):
        return ag_drv_lport_mib_grxuo_set(portid, 0);
    case (STAT_GRXUO):
        return ag_drv_lport_mib_grxuda_set(portid, 0);
    case (STAT_GRXUDA):
        return ag_drv_lport_mib_grxwsa_set(portid, 0);
    case (STAT_GRXWSA):
        return ag_drv_lport_mib_grxaln_set(portid, 0);
    case (STAT_GRXALN):
        return ag_drv_lport_mib_grxflr_set(portid, 0);
    case (STAT_GRXFLR):
        return ag_drv_lport_mib_grxflr_set(portid, 0);
    case (STAT_GRXFRERR):
        return ag_drv_lport_mib_grxfrerr_set(portid, 0);
    case (STAT_GRXFCR):
        return ag_drv_lport_mib_grxfcr_set(portid, 0);
    case (STAT_GRXOVR):
        return ag_drv_lport_mib_grxovr_set(portid, 0);
    case (STAT_GRXJBR):
        return ag_drv_lport_mib_grxjbr_set(portid, 0);
    case (STAT_GRXMTUE):
        return ag_drv_lport_mib_grxmtue_set(portid, 0);
    case (STAT_GRXMCRC):
        return ag_drv_lport_mib_grxmcrc_set(portid, 0);
    case (STAT_GRXPRM):
        return ag_drv_lport_mib_grxprm_set(portid, 0);
    case (STAT_GRXVLN):
        return ag_drv_lport_mib_grxvln_set(portid, 0);
    case (STAT_GRXDVLN):
        return ag_drv_lport_mib_grxdvln_set(portid, 0);
    case (STAT_GRXTRFU):
        return ag_drv_lport_mib_grxtrfu_set(portid, 0);
    case (STAT_GRXPOK):
        return ag_drv_lport_mib_grxpok_set(portid, 0);
    case (STAT_GRXSCHCRC):
        return ag_drv_lport_mib_grxschcrc_set(portid, 0);
    case (STAT_GRXBYT):
        return ag_drv_lport_mib_grxbyt_set(portid, 0);
    case (STAT_GRXRPKT):
        return ag_drv_lport_mib_grxrpkt_set(portid, 0);
    case (STAT_GRXUND):
        return ag_drv_lport_mib_grxund_set(portid, 0);
    case (STAT_GRXFRG):
        return ag_drv_lport_mib_grxfrg_set(portid, 0);
    case (STAT_GRXRBYT):
        return ag_drv_lport_mib_grxrbyt_set(portid, 0);
    case (STAT_GRXLPI):
        return ag_drv_lport_mib_grxlpi_set(portid, 0);
    case (STAT_GRXDLPI):
        return ag_drv_lport_mib_grxdlpi_set(portid, 0);
    case (STAT_GRXPTLLFC):
        return ag_drv_lport_mib_grxptllfc_set(portid, 0);
    case (STAT_GRXLTLLFC):
        return ag_drv_lport_mib_grxltllfc_set(portid, 0);
    case (STAT_GRXLLFCFCS):
        return ag_drv_lport_mib_grxllfcfcs_set(portid, 0);
    default:
        return LPORT_ERR_PARAM;
    }

    return LPORT_ERR_OK;
}

int lport_stats_rst_tx_single(uint32_t portid,  LPORT_STATS_TX_TYPE stat_type)
{
    switch(stat_type)
    {
    case(STAT_GTX64):
        return ag_drv_lport_mib_gtx64_set(portid, 0);
    case(STAT_GTX127):
        return ag_drv_lport_mib_gtx127_set(portid, 0);
    case(STAT_GTX255):
        return ag_drv_lport_mib_gtx255_set(portid, 0);
    case(STAT_GTX511):
        return ag_drv_lport_mib_gtx511_set(portid, 0);
    case(STAT_GTX1023):
        return ag_drv_lport_mib_gtx1023_set(portid, 0);
    case(STAT_GTX1518):
        return ag_drv_lport_mib_gtx1518_set(portid, 0);
    case(STAT_GTX1522):
        return ag_drv_lport_mib_gtx1522_set(portid, 0);
    case(STAT_GTX2047):
        return ag_drv_lport_mib_gtx2047_set(portid, 0);
    case(STAT_GTX4095):
        return ag_drv_lport_mib_gtx4095_set(portid, 0);
    case(STAT_GTX9216):
        return ag_drv_lport_mib_gtx9216_set(portid, 0);
    case(STAT_GTX16383):
        return ag_drv_lport_mib_gtx16383_set(portid, 0);
    case(STAT_GTXPOK):
        return ag_drv_lport_mib_gtxpok_set(portid, 0);
    case(STAT_GTXPKT):
        return ag_drv_lport_mib_gtxpkt_set(portid, 0);
    case(STAT_GTXUCA):
        return ag_drv_lport_mib_gtxuca_set(portid, 0);
    case(STAT_GTXMCA):
        return ag_drv_lport_mib_gtxmca_set(portid, 0);
    case(STAT_GTXBCA):
        return ag_drv_lport_mib_gtxbca_set(portid, 0);
    case(STAT_GTXPF):
        return ag_drv_lport_mib_gtxpf_set(portid, 0);
    case(STAT_GTXPFC):
        return ag_drv_lport_mib_gtxpfc_set(portid, 0);
    case(STAT_GTXJBR):
        return ag_drv_lport_mib_gtxjbr_set(portid, 0);
    case(STAT_GTXFCS):
        return ag_drv_lport_mib_gtxfcs_set(portid, 0);
    case(STAT_GTXCF):
        return ag_drv_lport_mib_gtxcf_set(portid, 0);
    case(STAT_GTXOVR):
        return ag_drv_lport_mib_gtxovr_set(portid, 0);
    case(STAT_GTXDFR):
        return ag_drv_lport_mib_gtxdfr_set(portid, 0);
    case(STAT_GTXEDF):
        return ag_drv_lport_mib_gtxedf_set(portid, 0);
    case(STAT_GTXSCL):
        return ag_drv_lport_mib_gtxscl_set(portid, 0);
    case(STAT_GTXMCL):
        return ag_drv_lport_mib_gtxmcl_set(portid, 0);
    case(STAT_GTXLCL):
        return ag_drv_lport_mib_gtxlcl_set(portid, 0);
    case(STAT_GTXXCL):
        return ag_drv_lport_mib_gtxxcl_set(portid, 0);
    case(STAT_GTXFRG):
        return ag_drv_lport_mib_gtxfrg_set(portid, 0);
    case(STAT_GTXERR):
        return ag_drv_lport_mib_gtxerr_set(portid, 0);
    case(STAT_GTXVLN):
        return ag_drv_lport_mib_gtxvln_set(portid, 0);
    case(STAT_GTXDVLN):
        return ag_drv_lport_mib_gtxdvln_set(portid, 0);
    case(STAT_GTXRPKT):
        return ag_drv_lport_mib_gtxrpkt_set(portid, 0);
    case(STAT_GTXUFL):
        return ag_drv_lport_mib_gtxufl_set(portid, 0);
    case(STAT_GTXNCL):
        return ag_drv_lport_mib_gtxncl_set(portid, 0);
    case(STAT_GTXBYT):
        return ag_drv_lport_mib_gtxbyt_set(portid, 0);
    case(STAT_GTXLPI):
        return ag_drv_lport_mib_gtxlpi_set(portid, 0);
    case(STAT_GTXDLPI):
        return ag_drv_lport_mib_gtxdlpi_set(portid, 0);
    case(STAT_GTXLTLLFC):
        return ag_drv_lport_mib_gtxltllfc_set(portid, 0);
    default:
        return LPORT_ERR_PARAM;
    }
    return LPORT_ERR_OK;
}

int lport_stats_cfg_set(uint32_t portid, lport_stats_cfg_s *stats_cfg)
{
    uint8_t xlmacid = portid >> 2;
    uint8_t instance = portid % 4;
    uint8_t eee_cnt_mode;
    uint8_t saturate_en;
    uint8_t cor_en;
    uint8_t cnt_rst;

    ag_drv_mib_conf_control_get(xlmacid, &eee_cnt_mode, &saturate_en,
        &cor_en, &cnt_rst);

    if(stats_cfg->saturate_en)
        saturate_en |= (1 << instance);
    else
        saturate_en &= ~(1 << instance);

    if(stats_cfg->cor_en)
        cor_en |= (1 << instance);
    else
        cor_en &= ~(1 << instance);

    return ag_drv_mib_conf_control_set(xlmacid, eee_cnt_mode, saturate_en,
        cor_en, cnt_rst);
}

int lport_stats_cfg_get(uint32_t portid, lport_stats_cfg_s *stats_cfg)
{
    uint8_t xlmacid = portid >> 2;
    uint8_t instance = portid % 4;
    uint8_t eee_sym_mode;
    uint8_t saturate_en;
    uint8_t cor_en;
    uint8_t cnt_rst;

    ag_drv_mib_conf_control_get(xlmacid, &eee_sym_mode, &saturate_en,
        &cor_en, &cnt_rst);

    stats_cfg->saturate_en = (saturate_en & (1 << instance)) != 0;
    stats_cfg->eee_sym_mode = (eee_sym_mode & (1 << instance)) != 0;
    stats_cfg->cor_en =( cor_en & (1 << instance)) != 0;

    return 0;
}

int lport_stats_reset(uint32_t portid)
{
    uint8_t xlmacid = portid >> 2;
    uint8_t instance = portid % 4;
    uint8_t eee_cnt_mode;
    uint8_t saturate_en;
    uint8_t cor_en;
    uint8_t cnt_rst;

    ag_drv_mib_conf_control_get(xlmacid, &eee_cnt_mode, &saturate_en,
        &cor_en, &cnt_rst);

    cnt_rst |= (1 << instance);

    ag_drv_mib_conf_control_set(xlmacid, eee_cnt_mode, saturate_en,
        cor_en, cnt_rst );

    cnt_rst &= ~(1 << instance);

    ag_drv_mib_conf_control_set(xlmacid, eee_cnt_mode, saturate_en,
        cor_en, cnt_rst );

    return 0;
}

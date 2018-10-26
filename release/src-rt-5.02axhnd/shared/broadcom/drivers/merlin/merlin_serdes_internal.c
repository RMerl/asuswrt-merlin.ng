/***********************************************************************************
 ***********************************************************************************
 *  File Name     :  merlin_serdes_internal.c                                      *
 *  Created On    :  30/11/2015                                                    *
 *  Created By    :  Lei Cai                                                       *
 *  Description   :  BCM APIs for Merlin  PHY                                      *
 *  Revision      :  $Id: merlin_serdes_internal.c   1    2015-11-30 $             *
 *                                                                                 *
 *  $Copyright: (c) 2015 Broadcom Corporation                                      *
 *  All Rights Reserved$                                                           *
 *  No portions of this material may be reproduced in any form without             *
 *  the written permission of:                                                     *
 *      Broadcom Corporation                                                       *
 *      5300 California Avenue                                                     *
 *      Irvine, CA  92617                                                          *
 *                                                                                 *
 *  All information contained in this document is Broadcom Corporation             *
 *  company private proprietary, and trade secret.                                 *
 *                                                                                 *
 ***********************************************************************************
 ***********************************************************************************/
 /*
<:copyright-BRCM:2015:proprietary:standard

   Copyright (c) 2015 Broadcom 
   All Rights Reserved

 This program is the proprietary software of Broadcom and/or its
 licensors, and may only be used, duplicated, modified or distributed pursuant
 to the terms and conditions of a separate, written license agreement executed
 between you and Broadcom (an "Authorized License").  Except as set forth in
 an Authorized License, Broadcom grants no license (express or implied), right
 to use, or waiver of any kind with respect to the Software, and Broadcom
 expressly reserves all rights in and to the Software and all intellectual
 property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
 NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
 BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization,
    constitutes the valuable trade secrets of Broadcom, and you shall use
    all reasonable efforts to protect the confidentiality thereof, and to
    use this information only in connection with your use of Broadcom
    integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
    ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
    FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
    TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
    PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
    ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
    INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
    WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
    IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
    SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
    SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
    LIMITED REMEDY.
:>

*/
/***********************************************************************/
/*                                                                     */
/*   MODULE:  merlin_serdes_internal.c                                 */
/*   DATE:    30/11/2015                                               */
/*   PURPOSE: Internal functions used by Merlin APIs                   */
/*                                                                     */
/***********************************************************************/ 
#include "merlin_mptwo_functions.h"
#include "merlin_mptwo_interface.h"

#include "merlin_serdes.h"
#include "merlin_serdes_internal.h"

uint8_t merlin_core_prtad[MERLIN_MAX_CORE_NUMBER] = {6, 7};

err_code_t merlin_apply_txfir_cfg(merlin_access_t *ma, int8_t pre, int8_t main, int8_t post1, int8_t post2) 
{
    err_code_t failcode = merlin_mptwo_validate_txfir_cfg(ma, pre, main, post1, post2);

    if (!failcode) {
        EFUN(wr_ams_tx_post2to1(post2==0));     /* Convert post2 slices to post1, if post2 is 0 */
        EFUN(wr_ams_tx_en_pre(pre>0));          /* Enable the TXFIR blocks */
        EFUN(wr_ams_tx_en_post1(post1>0));
        EFUN(wr_ams_tx_en_post2(post2>0));
        EFUN(wr_txfir_pre_override((uint8_t)pre));
        EFUN(wr_txfir_main_override((uint8_t)main));
        EFUN(wr_txfir_post_override((uint8_t)post1));
        EFUN(wr_txfir_post2((uint8_t)post2));

        if ((pre+main+post1+post2) > 45) {
            EFUN(wr_ams_tx_refcalshunt(5));
        } else {
            EFUN(wr_ams_tx_refcalshunt(4));
        }
    }

    return (_error(failcode));
}

uint16_t merlin_dvt_tx_apply_elec_spec (merlin_access_t *ma, merlin_elec_spec_enum_t elec_spec, uint16_t attribute)
{
    uint16_t err_code = 0;

    switch (elec_spec) {
    case SFI_COPPER_OSx1 :
    case SFI_OPTICS_OSx1 :
    case SFI_OPTICS_DFE_OSx1 :
    case nPPI_OPTICS_OSx1 :
    case nPPI_OPTICS_DFE_OSx1 :
        { 
            // (0:8) dB in 1dB steps 
            int postc[9] = {0, 9, 16, 23, 30, 35, 40, 44, 48};
            if (attribute>8) {
                //USR_PRINTF("SFI and nPPI support channel loss from 0 to 8dB (0 to 8) only\n");
                err_code = 1;
            } else {
                merlin_apply_txfir_cfg(ma, 0,36 - postc[attribute],postc[attribute],0);    /* total = 112 */
            }
        }
        break;

    default :
        //USR_PRINTF ("merlin_dvt_tx_apply_elec_spec: Invalid TX elec spec.\n");
        err_code = 1;
        break;
    }

    return err_code;
}

uint16_t merlin_rx_apply_elec_spec (merlin_access_t *ma, merlin_elec_spec_enum_t elec_spec, struct merlin_mptwo_uc_lane_config_st *lane_config) 
{
    uint16_t err_code = 0;
    uint8_t osr_mode = 0;
    uint8_t media_type = MEDIA_TYPE_PCB_TRACE_BACKPLANE;
    uint8_t dfe_on = 0;
    uint8_t scrambling_dis = 0;

    //
    // The switch-case table below lists the Electrical Spec modes in the
    // same order as shown in the Eagle Elec Spec DVT Mode spreadsheet.
    // The following default values are assumed:
    //
    media_type = MEDIA_TYPE_PCB_TRACE_BACKPLANE;
    dfe_on = 0;
    scrambling_dis = 0;

    switch (elec_spec) {
    case SFI_COPPER_OSx1 :
        osr_mode = MERLIN_MPTWO_OSX1;
        media_type = MEDIA_TYPE_COPPER_CABLE;
        dfe_on = 1;
        break;
    case SFI_OPTICS_OSx1 :
        osr_mode = MERLIN_MPTWO_OSX1;
        media_type = MEDIA_TYPE_OPTICS;
        break;
    case SFI_OPTICS_DFE_OSx1 :
        osr_mode = MERLIN_MPTWO_OSX1;
        media_type = MEDIA_TYPE_OPTICS;
        dfe_on = 1;
        break;
    case XFI_OSx1 :
        osr_mode = MERLIN_MPTWO_OSX1;
        break;
    case XFI_DFE_OSx1 :
        osr_mode = MERLIN_MPTWO_OSX1;
        dfe_on = 1;
        break;
    case nPPI_OPTICS_OSx1 :
        osr_mode = MERLIN_MPTWO_OSX1;
        media_type = MEDIA_TYPE_OPTICS;
        break;
    case nPPI_OPTICS_DFE_OSx1 :
        osr_mode = MERLIN_MPTWO_OSX1;
        media_type = MEDIA_TYPE_OPTICS;
        dfe_on = 1;
        break;
    case CUSTOM_ELEC_SPEC :
        break;
    default :
        //USR_PRINTF ("merlin_rx_apply_elec_spec: Invalid RX elec spec.\n");
        err_code = 1;
        break;
    }

    if ((err_code == 0) && (elec_spec != CUSTOM_ELEC_SPEC)) {
        wr_osr_mode_frc_val (osr_mode);
        wr_osr_mode_frc (1);
        /*wr_cl72_ieee_training_enable(cl72_on);*/
        lane_config->field.media_type = media_type;
        lane_config->field.dfe_on = dfe_on;
        lane_config->field.scrambling_dis = scrambling_dis;
    }

    return err_code;
}

err_code_t merlin_link_optical_configure(uint16_t lane_index, merlin_elec_spec_enum_t spec, uint16_t attribute)
{
    struct merlin_mptwo_uc_lane_config_st lane_config = {};
    merlin_access_t ma_in, *ma=&ma_in;

    ma->index = lane_index;    
    merlin_dvt_tx_apply_elec_spec(ma, spec, attribute);  /*attr 0->sr, 1->lr*/
    merlin_rx_apply_elec_spec(ma, spec, &lane_config);  
    merlin_mptwo_set_uc_lane_cfg(ma, lane_config);

    return ERR_CODE_NONE;
}

err_code_t merlin_link_type_configure(uint16_t lane_index, merlin_lane_type_t intf)
{
    merlin_access_t ma_in, *ma=&ma_in;
    
    ma->index = lane_index;

    switch(intf)
    {
        case MERLIN_INTERFACE_XFI:
            /*XFI, force 10G*/
            wr_pcs_dig_credit_sw_en(0x0);
            wr_pcs_dig_sw_actual_speed_force_en(0x1);
            wr_pcs_dig_sw_actual_speed(0xf);
            /*wr_pcs_tx_fec_enable(0x1);*/              /*fec optional*/
            wr_pcs_rx_block_sync_mode(0x4);             /*fec optional*/
            
            wr_pcs_rx_rstb_lane(0x1);
            wr_pcs_dig_mac_creditenable(0x1);
            wr_pcs_tx_rstb_tx_lane(0x1);
            wr_pcs_tx_enable_tx_lane(0x1);
            break;
        case MERLIN_INTERFACE_SGMII:
            /*SGMII slave, AN 1G/100M/10M, default is 1G*/
            wr_pcs_dig_use_ieee_reg_ctrl_sel(0x2);      /*User space AN*/
            /*
                    wr_pcs_an_sgmii_full_duplex(0x1);
                    wr_pcs_an_sgmii_speed(0x2);
                    wr_pcs_an_sgmii_master_mode(0x1);
                    */
            wr_pcs_an_sgmii_master_mode(0);
            
            wr_pcs_an_cl37_enable(0x1);
            wr_pcs_an_cl37_sgmii_enable(0x1);
            wr_pcs_dig_credit_sw_en(0x0);
            wr_pcs_dig_sw_actual_speed_force_en(0);

            wr_pcs_rx_rstb_lane(0x1);
            wr_pcs_dig_mac_creditenable(0x1);
            wr_pcs_tx_rstb_tx_lane(0x1);
            wr_pcs_tx_enable_tx_lane(0x1);
            break;
        case MERLIN_INTERFACE_HSGMII:
            /*2.5G, no AN, force speed*/
            wr_pcs_dig_credit_sw_en(0x0);
            wr_pcs_dig_sw_actual_speed_force_en(0x1);
            wr_pcs_dig_sw_actual_speed(0x3);

            wr_pcs_rx_rstb_lane(0x1);
            wr_pcs_dig_mac_creditenable(0x1);
            wr_pcs_tx_rstb_tx_lane(0x1);
            wr_pcs_tx_enable_tx_lane(0x1);
            break;
        default:
            return ERR_CODE_BAD_LANE;
            break;
    }
    
    return ERR_CODE_NONE;
}

err_code_t merlin_lane_assert_reset(merlin_access_t *ma, uint8_t enable)
{
    if(enable)
    {
        wr_ln_dp_s_rstb(0);
    }
    else
    {
        wr_ln_dp_s_rstb(1);        
    }
    
    return ERR_CODE_NONE;
}

void merlin_lane_type_get(LPORT_PORT_MUX_SELECT lport_sel, merlin_lane_type_t * lane_type)
{
    switch(lport_sel)
        {
        case PORT_XFI:
            *lane_type = MERLIN_INTERFACE_XFI;
            break;
        case PORT_HSGMII:
            *lane_type = MERLIN_INTERFACE_HSGMII;
            break;
        case PORT_SGMII_SLAVE:
        case PORT_SGMII_1000BASE_X:
            *lane_type = MERLIN_INTERFACE_SGMII;
            break;
        default:
            *lane_type = MERLIN_INTERFACE_UNUSED;
            break;
        }
}

err_code_t merlin_lane_status_get(merlin_access_t *ma, merlin_status_t * status)
{
    ESTM(status->tx_LOCAL_FAULT = rd_pcs_tx_local_fault());
    ESTM(status->tx_REMOTE_FAULT = rd_pcs_tx_remote_fault());
    ESTM(status->PMD_LOCK = rd_pcs_rx_pmd_lock());
    ESTM(status->signal_ok = rd_pcs_rx_signal_ok());
    ESTM(status->rx_LOCAL_FAULT  = rd_pcs_rx_local_fault());
    ESTM(status->rx_REMOTE_FAULT = rd_pcs_rx_remote_fault());
    ESTM(status->rx_LINK_STATUS  = rd_pcs_rx_link_status());
    ESTM(status->rx_SYNC_STATUS  = rd_pcs_rx_sync_status());
    ESTM(status->pll_lock = rdc_pll_lock());
    ESTM(status->cl36_syncacq_his_state_per_ln = rd_pcs_rx_cl36_syncacq_his_state_per_ln());
    ESTM(status->cl36_syncacq_state_coded_per_ln = rd_pcs_rx_cl36_syncacq_state_coded_per_ln());

    return ERR_CODE_NONE;
}

err_code_t merlin_lane_stats_get(merlin_access_t *ma, merlin_stats_t * stats)
{
    ESTM(stats->kcode66ErrCount = rd_pcs_rx_kcode66err_count());  /*64/66 encode*/
    ESTM(stats->sync66ErrCount = rd_pcs_rx_sync66err_count());
    ESTM(stats->cl49ieee_errored_blocks = rd_pcs_rx_cl49ieee_errored_blocks());   /*64/66 encode*/
    ESTM(stats->BER_count_per_ln = rd_pcs_rx_ber_count_per_ln());
    ESTM(stats->cl49_invalid_sh_cnt = rdc_pcs_rx_cl49_valid_sh_cnt());
    ESTM(stats->cl49_valid_sh_cnt = rdc_pcs_rx_cl49_invalid_sh_cnt());        
    return ERR_CODE_NONE;
}

err_code_t merlin_lane_prbs_stats_get(merlin_access_t *ma, merlin_prbs_stats_t * stats)
{
    
    ESTM(stats->prbs_error = rd_pcs_pktgen_prbs_error_lo());
    ESTM(stats->prbs_error |= (uint32_t)rd_pcs_pktgen_prbs_error_hi()<<16);

    ESTM(stats->prbs_tx_pkt = rd_pcs_pktgen_txpktcnt_l());
    ESTM(stats->prbs_tx_pkt |= (uint32_t)rd_pcs_pktgen_txpktcnt_u()<<16);

    ESTM(stats->prbs_rx_pkt = rd_pcs_pktgen_rxpktcnt_l());
    ESTM(stats->prbs_rx_pkt |= (uint32_t)rd_pcs_pktgen_rxpktcnt_u()<<16);

    ESTM(stats->crcerrcnt = rdc_pcs_pktgen_crcerrcnt());

    return ERR_CODE_NONE;
}

err_code_t merlin_core_id_get(merlin_access_t *ma, merlin_serdes_id_t * id)
{
    ESTM(id->rev_letter = rdc_pcs_main_rev_letter());
    ESTM(id->rev_number = rdc_pcs_main_rev_number());
    ESTM(id->bonding    = rdc_pcs_main_bonding());
    ESTM(id->tech_proc  = rdc_pcs_main_tech_proc());
    ESTM(id->model_number  = rdc_pcs_main_model_number());

    return ERR_CODE_NONE;
}


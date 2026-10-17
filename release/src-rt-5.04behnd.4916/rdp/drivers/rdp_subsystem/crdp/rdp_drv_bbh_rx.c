/*
    <:copyright-BRCM:2015:DUAL/GPL:standard
    
       Copyright (c) 2015 Broadcom 
       All Rights Reserved
    
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

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/

#include "rdp_subsystem_common.h"
#include "rdp_drv_bbh_rx.h"
#include "rdp_common.h"
#include "xrdp_autogens.h"

/******************************************************************************/
/*                                                                            */
/* Types and values definitions                                               */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/*                                                                            */
/* Macros definitions                                                         */
/*                                                                            */
/******************************************************************************/

int drv_bbh_rx_pkt_size_set(uint8_t bbh_id, uint8_t min_pkt_size, uint16_t max_pkt_size)
{
    ag_drv_bbh_rx_general_configuration_minpkt_set(bbh_id, min_pkt_size, MIN_OMCI_PACKET_SIZE_BYTES);
    return ag_drv_bbh_rx_general_configuration_maxpkt_set(bbh_id, max_pkt_size);
}

int drv_bbh_rx_pkt_size_get(uint8_t bbh_id, uint8_t *min_pkt_size, uint16_t *max_pkt_size)
{
    uint8_t min_pkt_size_omci;
    ag_drv_bbh_rx_general_configuration_minpkt_get(bbh_id, min_pkt_size, &min_pkt_size_omci);
    return ag_drv_bbh_rx_general_configuration_maxpkt_get(bbh_id, max_pkt_size);
}

bdmf_error_t drv_bbh_rx_channel_enable(uint8_t bbh_id, int channel_num, int enable)
{
    uint8_t pkten;
    bdmf_boolean sbpmen;
    bdmf_boolean geminit;
    bdmf_boolean grace_dis;
    ag_drv_bbh_rx_general_configuration_enable_get(bbh_id, &pkten, &sbpmen, &geminit, &grace_dis);
    
    if (enable)
    {
        pkten |= (1 << channel_num);
        sbpmen = 1;
    }
    else
    {
        pkten &= ~(1 << channel_num);
    }
    return ag_drv_bbh_rx_general_configuration_enable_set(bbh_id, pkten, sbpmen, geminit, grace_dis);
}

bdmf_error_t drv_bbh_rx_init(uint8_t bbh_id, int num_of_channels_in_this_bbhrx, bbh_rx_config *config)
{
    int channel_num, psram_index;
    bdmf_error_t rc;

    for (channel_num = 0; channel_num < num_of_channels_in_this_bbhrx; channel_num++) 
    {
        rc = ag_drv_bbh_rx_general_configuration_sopoffset_set(bbh_id, channel_num, SBPM_PACKET_HEADER_OFFSET);
        BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: drv_bbh_rx_init. bbh_id: %d ;channel_num: %d\n", bbh_id, channel_num);
    }

    rc = ag_drv_bbh_rx_general_configuration_bbcfg_set(bbh_id, config->disp_bb_id, config->sbpm_bb_id);
    BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: drv_bbh_rx_init\n");
    
    rc = ag_drv_bbh_rx_general_configuration_ingresschannel_set(bbh_id, bbh_id);
    BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: drv_bbh_rx_init. bbh_id: %d\n", bbh_id);

    psram_index =  bbh_id;
    rc = ag_drv_bbh_rx_general_configuration_psrambase_set(bbh_id, RU_BLK(PSRAM_MEM).addr[psram_index]>>20);
    BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: drv_bbh_rx_init. bbh_id: %d\n", bbh_id);

    rc = ag_drv_bbh_rx_general_configuration_enable_set(bbh_id, 0, 0, 1, 0);
    BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: drv_bbh_rx_init. bbh_id: %d\n", bbh_id);

#ifndef RDP_SIM /* SystemC simulator doesn't set the gemdone bit */
{
    int num_wait_attempts;
    bdmf_boolean gemdone;

    for (num_wait_attempts = 0; num_wait_attempts < BBHRX_GEMDONE_MAX_WAIT_ATTEMPTS; num_wait_attempts++)
    {
        rc = ag_drv_bbh_rx_general_configuration_gemdone_get(bbh_id, &gemdone);
        BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: drv_bbh_rx_init. bbh_id: %d\n", bbh_id);
        if (gemdone)
            break;
        bdmf_usleep(1);
    }
    
    BDMF_TRACE_RET_IF_COND(
        num_wait_attempts == BBHRX_GEMDONE_MAX_WAIT_ATTEMPTS,
        BDMF_ERR_INTERNAL,
        "ERR: drv_bbh_rx_init: gemdone never asserted. bbh_id: %d\n",
        bbh_id);
}
#endif

/* to add for WAN init*/
#if 0
    for (channel_num = 0; channel_num < NUM_OF_BBH_RX_CHANNELS; channel_num++)
    {
        rc = ag_drv_bbh_rx_general_configuration_mactype_set(bbh_id, channel_num, config->mac_type);           
        BDMF_TRACE_RET_IF_COND(rc , rc, "ERR: drv_bbh_rx_init. mactype\n");
    }
#endif

    return BDMF_ERR_OK;
}

bdmf_error_t drv_bbh_rx_set_viq(uint8_t bbh_id, uint8_t channel_num, uint8_t viq_idx, uint8_t exclusive_viq_idx)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint16_t portviqmask, excviqmask; 
    uint16_t viqfcenmask;
    bdmf_boolean excviqen;

    portviqmask = (1 << viq_idx);
    viqfcenmask = (1 << viq_idx);
    rc = rc ? rc : ag_drv_bbh_rx_general_configuration_portviqmask_set(bbh_id, channel_num, portviqmask);

    if (exclusive_viq_idx != EXCLUSIVE_VIQ_INVALID)
    {
        excviqmask = (1 << exclusive_viq_idx); /* map each of the BBH RX channels to the exclusive VIQ as well */
        excviqen = 1;
        rc = rc ? rc : ag_drv_bbh_rx_general_configuration_excviqmask_set(bbh_id, channel_num, excviqmask, excviqen, 0); 
        viqfcenmask |= (1 << exclusive_viq_idx);
    }

    /* configure dispatcher to bbh rx flow control for this port */
    rc = rc ? rc : ag_drv_bbh_rx_general_configuration_viqfcen_set(bbh_id, channel_num, viqfcenmask);
    return rc;
}


#ifdef USE_BDMF_SHELL
static bdmfmon_handle_t bbh_dir;

void drv_bbh_rx_cli_init(bdmfmon_handle_t driver_dir)
{

}

void drv_bbh_rx_cli_exit(bdmfmon_handle_t driver_dir)
{
    if (bbh_dir)
    {
        bdmfmon_token_destroy(bbh_dir);
        bbh_dir = NULL;
    }
}
#endif /* USE_BDMF_SHELL */




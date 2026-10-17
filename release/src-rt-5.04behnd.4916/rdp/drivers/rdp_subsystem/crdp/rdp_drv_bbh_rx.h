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

#ifndef DRV_BBH_RX_H_INCLUDED
#define DRV_BBH_RX_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include <ru.h>
#include <bdmf_interface.h>
#include <rdp_common.h>

#ifdef USE_BDMF_SHELL
#include <bdmf_shell.h>
#endif


typedef struct
{
    uint16_t disp_bb_id;
    uint16_t sbpm_bb_id;
} bbh_rx_config;

/* BBH_RX */
#define SBPM_PACKET_HEADER_OFFSET       24
#define PFB_NNI_RX_WIDTH  2    /* 256   from PMA */
#define PFB_GPON_RX_WIDTH 1    /* 128  from GPON */
#define MIN_OMCI_PACKET_SIZE_BYTES (14)
#define BBHRX_GEMDONE_MAX_WAIT_ATTEMPTS (10)

#define NUM_OF_BBH_RX_CHANNELS         8    /* the number of supported ports, per BBHRX */
#define NUM_OF_BBH_RX_VIQS             9 
#define NUM_OF_BBH_RX                  (NUM_OF_INGRESS_LAYOUT_BLOCKS * NUM_OF_CHANNELS_PER_INGRESS)
#define EXCLUSIVE_VIQ_INVALID          -1


int drv_bbh_rx_pkt_size_set(uint8_t bbh_id, uint8_t min_pkt_size, uint16_t max_pkt_size);
int drv_bbh_rx_pkt_size_get(uint8_t bbh_id, uint8_t *min_pkt_size, uint16_t *max_pkt_size);
bdmf_error_t drv_bbh_rx_init(uint8_t bbh_id, int num_of_channels_in_this_bbhrx, bbh_rx_config *config);


#ifdef USE_BDMF_SHELL
void drv_bbh_rx_cli_init(bdmfmon_handle_t driver_dir);
void drv_bbh_rx_cli_exit(bdmfmon_handle_t driver_dir);
#endif

#ifdef __cplusplus
}
#endif

#endif

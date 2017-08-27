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

#ifndef _XRDP_DRV_DMA_AG_H_
#define _XRDP_DRV_DMA_AG_H_

#include "access_macros.h"
#include "bdmf_interface.h"
#ifdef USE_BDMF_SHELL
#include "bdmf_shell.h"
#endif
#include "rdp_common.h"


/**************************************************************************************************/
/* eth0rxne: Ethernet0_RX_not_empty_indications - indication of the queue state                   */
/* eth1rxne: Ethernet1_RX_not_empty_indications - indication of the queue state                   */
/* eth2rxne: Ethernet2_RX_not_empty_indications - indication of the queue state                   */
/* eth3rxne: Ethernet3_RX_not_empty_indications - indication of the queue state                   */
/* eth4rxne: Ethernet4_RX_not_empty_indications - indication of the queue state                   */
/* gponrxne: GPON_RX_not_empty_indications - indication of the queue state                        */
/* eth0txne: Ethernet0_TX_not_empty_indications - indication of the queue state                   */
/* eth1txne: Ethernet1_TX_not_empty_indications - indication of the queue state                   */
/* eth2txne: Ethernet2_TX_not_empty_indications - indication of the queue state                   */
/* eth3txne: Ethernet3_TX_not_empty_indications - indication of the queue state                   */
/* eth4txne: Ethernet4_TX_not_empty_indications - indication of the queue state                   */
/* gpontxne: GPON_TX_not_empty_indications - indication of the queue state                        */
/* eth0rxu: Ethernet0_RX_urgent_indication - indication whether the queue is in urgent state or n */
/*          ot                                                                                    */
/* eth1rxu: Ethernet1_RX_urgent_indication - indication whether the queue is in urgent state or n */
/*          ot                                                                                    */
/* eth2rxu: Ethernet2_RX_urgent_indication - indication whether the queue is in urgent state or n */
/*          ot                                                                                    */
/* eth3rxu: Ethernet3_RX_urgent_indication - indication whether the queue is in urgent state or n */
/*          ot                                                                                    */
/* eth4rxu: Ethernet4_RX_urgent_indication - indication whether the queue is in urgent state or n */
/*          ot                                                                                    */
/* gponrxu: GPON_RX_urgent_indication - indication whether the queue is in urgent state or not    */
/* eth0txu: Ethernet0_TX_urgent_indication - indication whether the queue is in urgent state or n */
/*          ot                                                                                    */
/* eth1txu: Ethernet1_TX_urgent_indication - indication whether the queue is in urgent state or n */
/*          ot                                                                                    */
/* eth2txu: Ethernet2_TX_urgent_indication - indication whether the queue is in urgent state or n */
/*          ot                                                                                    */
/* eth3txu: Ethernet3_TX_urgent_indication - indication whether the queue is in urgent state or n */
/*          ot                                                                                    */
/* eth4txu: Ethernet4_TX_urgent_indication - indication whether the queue is in urgent state or n */
/*          ot                                                                                    */
/* gpontxu: GPON_TX_urgent_indication - indication whether the queue is in urgent state or not    */
/* sel_src: selected_source - the next peripheral to be served by the dma                         */
/* address: address - address within the ram                                                      */
/* datacs: data_ram_cs - chip select for write data ram                                           */
/* cdcs: cd_ram_cs - chip select for chunk descriptors ram                                        */
/* rrcs: rr_ram_cd - chip select for read requests ram                                            */
/* valid: valid - indirect read request is valid                                                  */
/* ready: ready - read data ready                                                                 */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean eth0rxne;
    bdmf_boolean eth1rxne;
    bdmf_boolean eth2rxne;
    bdmf_boolean eth3rxne;
    bdmf_boolean eth4rxne;
    bdmf_boolean gponrxne;
    bdmf_boolean eth0txne;
    bdmf_boolean eth1txne;
    bdmf_boolean eth2txne;
    bdmf_boolean eth3txne;
    bdmf_boolean eth4txne;
    bdmf_boolean gpontxne;
    bdmf_boolean eth0rxu;
    bdmf_boolean eth1rxu;
    bdmf_boolean eth2rxu;
    bdmf_boolean eth3rxu;
    bdmf_boolean eth4rxu;
    bdmf_boolean gponrxu;
    bdmf_boolean eth0txu;
    bdmf_boolean eth1txu;
    bdmf_boolean eth2txu;
    bdmf_boolean eth3txu;
    bdmf_boolean eth4txu;
    bdmf_boolean gpontxu;
    uint8_t sel_src;
    uint16_t address;
    bdmf_boolean datacs;
    bdmf_boolean cdcs;
    bdmf_boolean rrcs;
    bdmf_boolean valid;
    bdmf_boolean ready;
} dma_debug_info;


/**************************************************************************************************/
/* eth0rx: ethernet_0_rx_reset - resets the pointers of ethernet 0 rx                             */
/* eth0tx: ethernet_0_tx_reset - resets the pointers of ethernet 0 tx                             */
/* eth1rx: ethernet_1_rx_reset - resets the pointers of ethernet 1 rx                             */
/* eth1tx: ethernet_1_tx_reset - resets the pointers of ethernet 1 tx                             */
/* eth2rx: ethernet_2_rx_reset - resets the pointers of ethernet 2 rx                             */
/* eth2tx: ethernet_2_tx_reset - resets the pointers of ethernet 2 tx                             */
/* eth3rx: ethernet_3_rx_reset - resets the pointers of ethernet 3 rx                             */
/* eth3tx: ethernet_3_tx_reset - resets the pointers of ethernet 3 tx                             */
/* eth4rx: ethernet_4_rx_reset - resets the pointers of ethernet 4 rx                             */
/* eth4tx: ethernet_4_tx_reset - resets the pointers of ethernet 4 tx                             */
/* gponrx: gpon_rx_reset - resets the pointers of gpon rx                                         */
/* gpontx: gpon_tx_reset - resets the pointers of gpon tx                                         */
/**************************************************************************************************/
typedef struct
{
    bdmf_boolean eth0rx;
    bdmf_boolean eth0tx;
    bdmf_boolean eth1rx;
    bdmf_boolean eth1tx;
    bdmf_boolean eth2rx;
    bdmf_boolean eth2tx;
    bdmf_boolean eth3rx;
    bdmf_boolean eth3tx;
    bdmf_boolean eth4rx;
    bdmf_boolean eth4tx;
    bdmf_boolean gponrx;
    bdmf_boolean gpontx;
} dma_config_ptrrst;

bdmf_error_t ag_drv_dma_debug_info_set(uint8_t dma_id, const dma_debug_info *debug_info);
bdmf_error_t ag_drv_dma_debug_info_get(uint8_t dma_id, dma_debug_info *debug_info);
bdmf_error_t ag_drv_dma_config_bbrouteovrd_set(uint8_t dma_id, uint8_t dest, uint16_t route, bdmf_boolean ovrd);
bdmf_error_t ag_drv_dma_config_bbrouteovrd_get(uint8_t dma_id, uint8_t *dest, uint16_t *route, bdmf_boolean *ovrd);
bdmf_error_t ag_drv_dma_config_num_of_writes_set(uint8_t dma_id, uint8_t emac_index, uint8_t numofbuff);
bdmf_error_t ag_drv_dma_config_num_of_writes_get(uint8_t dma_id, uint8_t emac_index, uint8_t *numofbuff);
bdmf_error_t ag_drv_dma_config_num_of_reads_set(uint8_t dma_id, uint8_t emac_index, uint8_t rr_num);
bdmf_error_t ag_drv_dma_config_num_of_reads_get(uint8_t dma_id, uint8_t emac_index, uint8_t *rr_num);
bdmf_error_t ag_drv_dma_config_u_thresh_set(uint8_t dma_id, uint8_t emac_index, uint8_t into_u, uint8_t out_of_u);
bdmf_error_t ag_drv_dma_config_u_thresh_get(uint8_t dma_id, uint8_t emac_index, uint8_t *into_u, uint8_t *out_of_u);
bdmf_error_t ag_drv_dma_config_pri_set(uint8_t dma_id, uint8_t emac_index, uint8_t rxpri, uint8_t txpri);
bdmf_error_t ag_drv_dma_config_pri_get(uint8_t dma_id, uint8_t emac_index, uint8_t *rxpri, uint8_t *txpri);
bdmf_error_t ag_drv_dma_config_weight_set(uint8_t dma_id, uint8_t emac_index, uint8_t rxweight, uint8_t txweight);
bdmf_error_t ag_drv_dma_config_weight_get(uint8_t dma_id, uint8_t emac_index, uint8_t *rxweight, uint8_t *txweight);
bdmf_error_t ag_drv_dma_config_periph_source_set(uint8_t dma_id, uint8_t emac_index, uint8_t rxsource, uint8_t txsource);
bdmf_error_t ag_drv_dma_config_periph_source_get(uint8_t dma_id, uint8_t emac_index, uint8_t *rxsource, uint8_t *txsource);
bdmf_error_t ag_drv_dma_config_ptrrst_set(uint8_t dma_id, const dma_config_ptrrst *config_ptrrst);
bdmf_error_t ag_drv_dma_config_ptrrst_get(uint8_t dma_id, dma_config_ptrrst *config_ptrrst);
bdmf_error_t ag_drv_dma_config_max_otf_set(uint8_t dma_id, uint8_t max);
bdmf_error_t ag_drv_dma_config_max_otf_get(uint8_t dma_id, uint8_t *max);
bdmf_error_t ag_drv_dma_debug_req_cnt_rx_get(uint8_t dma_id, uint8_t emac_index, uint8_t *req_cnt);
bdmf_error_t ag_drv_dma_debug_req_cnt_tx_get(uint8_t dma_id, uint8_t emac_index, uint8_t *req_cnt);
bdmf_error_t ag_drv_dma_debug_req_cnt_rx_acc_get(uint8_t dma_id, uint8_t emac_index, uint32_t *req_cnt);
bdmf_error_t ag_drv_dma_debug_req_cnt_tx_acc_get(uint8_t dma_id, uint8_t emac_index, uint32_t *req_cnt);
bdmf_error_t ag_drv_dma_debug_rddata_get(uint8_t dma_id, uint8_t word_index, uint32_t *data);

#ifdef USE_BDMF_SHELL
enum
{
    cli_dma_debug_info,
    cli_dma_config_bbrouteovrd,
    cli_dma_config_num_of_writes,
    cli_dma_config_num_of_reads,
    cli_dma_config_u_thresh,
    cli_dma_config_pri,
    cli_dma_config_weight,
    cli_dma_config_periph_source,
    cli_dma_config_ptrrst,
    cli_dma_config_max_otf,
    cli_dma_debug_req_cnt_rx,
    cli_dma_debug_req_cnt_tx,
    cli_dma_debug_req_cnt_rx_acc,
    cli_dma_debug_req_cnt_tx_acc,
    cli_dma_debug_rddata,
};

int bcm_dma_cli_get(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms);
bdmfmon_handle_t ag_drv_dma_cli_init(bdmfmon_handle_t driver_dir);
#endif


#endif


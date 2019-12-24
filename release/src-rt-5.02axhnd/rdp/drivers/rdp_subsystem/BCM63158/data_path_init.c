/*
* <:copyright-BRCM:2014:proprietary:standard
* 
*    Copyright (c) 2014 Broadcom 
*    All Rights Reserved
* 
*  This program is the proprietary software of Broadcom and/or its
*  licensors, and may only be used, duplicated, modified or distributed pursuant
*  to the terms and conditions of a separate, written license agreement executed
*  between you and Broadcom (an "Authorized License").  Except as set forth in
*  an Authorized License, Broadcom grants no license (express or implied), right
*  to use, or waiver of any kind with respect to the Software, and Broadcom
*  expressly reserves all rights in and to the Software and all intellectual
*  property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE
*  NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY NOTIFY
*  BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
* 
*  Except as expressly set forth in the Authorized License,
* 
*  1. This program, including its structure, sequence and organization,
*     constitutes the valuable trade secrets of Broadcom, and you shall use
*     all reasonable efforts to protect the confidentiality thereof, and to
*     use this information only in connection with your use of Broadcom
*     integrated circuit products.
* 
*  2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
*     AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
*     WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
*     RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND
*     ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT,
*     FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
*     COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE
*     TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR
*     PERFORMANCE OF THE SOFTWARE.
* 
*  3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
*     ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
*     INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY
*     WAY RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN
*     IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
*     OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE
*     SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS
*     SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF ANY
*     LIMITED REMEDY.
* :>
*/


#define xrdp_memset memset
#define xrdp_memcpy memcpy
#define xrdp_usleep(_a) bdmf_usleep(_a)
#define xrdp_alloc(_a) bdmf_alloc(_a)

#include "rdd.h"
#include "rdd_defs.h"
#include "data_path_init.h"
#include "rdd_init.h"
#include "rdd_cpu_rx.h"
#include "rdd_tcam_ic.h"
#include "rdp_platform.h"
#include "rdd_basic_scheduler.h"
#include "rdp_drv_sbpm.h"
#include "rdp_drv_cnpl.h"
#include "rdp_drv_cntr.h"
#include "xrdp_drv_psram_ag.h"
#include "xrdp_drv_fpm_ag.h"
#include "rdp_common.h"
#include "rdp_drv_dma.h"
#include "rdp_drv_rnr.h"
#include "rdp_drv_dqm.h"
#include "rdp_drv_dis_reor.h"
#include "rdp_drv_natc.h"
#include "rdp_drv_tcam.h"
#include "rdp_drv_fpm.h"
#include "rdp_drv_hash.h"
#include "rdp_drv_bkpt.h"
#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_ubus_mstr_ag.h"
#include "xrdp_drv_ubus_slv_ag.h"
#include "xrdp_drv_bac_if_ag.h"
#include "rdp_drv_qm.h"
#include "bdmf_data_types.h"
#include "rdd_scheduling.h"
#include "rdd_ghost_reporting.h"
#include "xrdp_drv_unimac_rdp_ag.h"
#include "xrdp_drv_unimac_misc_ag.h"
#include "rdd_ag_ds_tm.h"
#include "rdd_ag_us_tm.h"
#include "xrdp_drv_xlif_rx_if_ag.h"
#include "xrdp_drv_xlif_tx_if_ag.h"

#ifdef XRDP_EMULATION
#define NATC_DDR_RES_DS_BASE 0x00100000
#define NATC_DDR_KEY_DS_BASE 0x00180000
#define NATC_DDR_RES_US_BASE 0x00200000
#define NATC_DDR_KEY_US_BASE 0x00280000
#endif

extern rdd_bb_id rdpa_emac_to_bb_id_tx[rdpa_emac__num_of];
extern rdd_bb_id rdpa_emac_to_bb_id_rx[rdpa_emac__num_of];

rdd_mac_type rdpa_emac_to_bbh_mac_type_tx[rdpa_emac__num_of] = {
    MAC_TYPE_EMAC,
    MAC_TYPE_EMAC,
    MAC_TYPE_EMAC,
    MAC_TYPE_GPON,
    MAC_TYPE_AE10G,
    MAC_TYPE_AE2P5,
    MAC_TYPE_DSL
};

rdd_mac_type bbh_tx_id_to_mac_type[BBH_TX_ID_NUM] = {
    MAC_TYPE_EMAC,
    MAC_TYPE_GPON,
    MAC_TYPE_EMAC, // MAC_TYPE_AE10G, apparently this configuration doesn't matter in BBHTX block
    MAC_TYPE_EMAC, // MAC_TYPE_AE2P5, apparently this configuration doesn't matter in BBHTX block
    MAC_TYPE_GPON  // MAC_TYPE_DSL, apparently this configuration doesn't matter in BBHTX block
};

dpi_params_t *p_dpi_cfg;
extern uint32_t total_length_bytes[];

RDD_FPM_GLOBAL_CFG_DTS g_fpm_hw_cfg;
void *natc_mem_end_addr;
int32_t natc_mem_available_size;
bbh_rx_sdma_profile_t *g_bbh_rx_sdma_profile;
bbh_tx_dma_profile_t *g_bbh_tx_dma_profile;
bbh_tx_sdma_profile_t *g_bbh_tx_sdma_profile;
bbh_tx_ddr_profile_t *g_bbh_tx_ddr_profile;

pd_wkup_threshold_t g_lan_pd_wkup_threshold[2] = { {0, 0}, {0, 0} };
pd_fifo_size_t g_lan_pd_fifo_size[LAN_QUEUE_PAIRS] = {
          {BBH_TX_DS_PD_FIFO_SIZE_0, BBH_TX_DS_PD_FIFO_SIZE_1}, {BBH_TX_DS_PD_FIFO_SIZE_0, BBH_TX_DS_PD_FIFO_SIZE_1},
          {BBH_TX_DS_PD_FIFO_SIZE_0, BBH_TX_DS_PD_FIFO_SIZE_1}, {BBH_TX_DS_PD_FIFO_SIZE_0, BBH_TX_DS_PD_FIFO_SIZE_1} };
queue_to_rnr_t g_lan_queue_to_rnr[BBH_TX_NUM_OF_LAN_QUEUES_PAIRS] = { {RNR0,RNR0}, {RNR0,RNR0}, {RNR0,RNR0}, {RNR0,RNR0} };
pd_bytes_threshold_t g_lan_pd_bytes_threshold[BBH_TX_NUM_OF_LAN_QUEUES_PAIRS] = { {0,0}, {0,0}, {0,0}, {0,0} };

pd_fifo_base_t g_lan_fe_fifo_base[BBH_TX_NUM_OF_LAN_QUEUES_PAIRS] = {
          {BBH_TX_LAN_FE_FIFO_BASE_0, BBH_TX_LAN_FE_FIFO_BASE_1}, {BBH_TX_LAN_FE_FIFO_BASE_2, BBH_TX_LAN_FE_FIFO_BASE_3},
          {BBH_TX_LAN_FE_FIFO_BASE_4, BBH_TX_LAN_FE_FIFO_BASE_5}, {BBH_TX_LAN_FE_FIFO_BASE_6, BBH_TX_LAN_FE_FIFO_BASE_7} };

pd_fifo_size_t g_lan_fe_size_base[BBH_TX_NUM_OF_LAN_QUEUES_PAIRS] = {
          {BBH_TX_LAN_FE_FIFO_SIZE, BBH_TX_LAN_FE_FIFO_SIZE}, {BBH_TX_LAN_FE_FIFO_SIZE, BBH_TX_LAN_FE_FIFO_SIZE},
          {BBH_TX_LAN_FE_FIFO_SIZE, BBH_TX_LAN_FE_FIFO_SIZE}, {BBH_TX_LAN_FE_FIFO_SIZE, BBH_TX_LAN_FE_FIFO_SIZE} };

pd_fifo_base_t g_lan_fe_pd_fifo_base[BBH_TX_NUM_OF_LAN_QUEUES_PAIRS] = {
          {BBH_TX_LAN_FE_PD_FIFO_BASE_0, BBH_TX_LAN_FE_PD_FIFO_BASE_1}, {BBH_TX_LAN_FE_PD_FIFO_BASE_2, BBH_TX_LAN_FE_PD_FIFO_BASE_3},
          {BBH_TX_LAN_FE_PD_FIFO_BASE_4, BBH_TX_LAN_FE_PD_FIFO_BASE_5}, {BBH_TX_LAN_FE_PD_FIFO_BASE_6, BBH_TX_LAN_FE_PD_FIFO_BASE_7} };

pd_fifo_size_t g_lan_fe_pd_size_base[BBH_TX_NUM_OF_LAN_QUEUES_PAIRS] = {
          {BBH_TX_LAN_FE_PD_FIFO_SIZE, BBH_TX_LAN_FE_PD_FIFO_SIZE}, {BBH_TX_LAN_FE_PD_FIFO_SIZE, BBH_TX_LAN_FE_PD_FIFO_SIZE},
          {BBH_TX_LAN_FE_PD_FIFO_SIZE, BBH_TX_LAN_FE_PD_FIFO_SIZE}, {BBH_TX_LAN_FE_PD_FIFO_SIZE, BBH_TX_LAN_FE_PD_FIFO_SIZE} };

pd_fifo_size_t g_lan_pd_size_base[BBH_TX_NUM_OF_LAN_QUEUES_PAIRS] = {
          {BBH_TX_DS_PD_FIFO_SIZE_0, BBH_TX_DS_PD_FIFO_SIZE_0}, {BBH_TX_DS_PD_FIFO_SIZE_0, BBH_TX_DS_PD_FIFO_SIZE_0},
          {BBH_TX_DS_PD_FIFO_SIZE_0, BBH_TX_DS_PD_FIFO_SIZE_0}, {BBH_TX_DS_PD_FIFO_SIZE_0, BBH_TX_DS_PD_FIFO_SIZE_0} };

queue_to_rnr_t g_wan_queue_to_rnr[TX_QEUEU_PAIRS] = { 
          {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0},
          {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0} };

pd_fifo_size_t g_gpon_pd_fifo_size[TX_QEUEU_PAIRS] = {
          {BBH_TX_GPON_PD_FIFO_SIZE_0_7, BBH_TX_GPON_PD_FIFO_SIZE_0_7}, {BBH_TX_GPON_PD_FIFO_SIZE_0_7, BBH_TX_GPON_PD_FIFO_SIZE_0_7},
          {BBH_TX_GPON_PD_FIFO_SIZE_0_7, BBH_TX_GPON_PD_FIFO_SIZE_0_7}, {BBH_TX_GPON_PD_FIFO_SIZE_0_7, BBH_TX_GPON_PD_FIFO_SIZE_0_7},
          {BBH_TX_GPON_PD_FIFO_SIZE_8_15, BBH_TX_GPON_PD_FIFO_SIZE_8_15}, {BBH_TX_GPON_PD_FIFO_SIZE_8_15, BBH_TX_GPON_PD_FIFO_SIZE_8_15},
          {BBH_TX_GPON_PD_FIFO_SIZE_8_15, BBH_TX_GPON_PD_FIFO_SIZE_8_15}, {BBH_TX_GPON_PD_FIFO_SIZE_8_15, BBH_TX_GPON_PD_FIFO_SIZE_8_15},
          {BBH_TX_GPON_PD_FIFO_SIZE_16_23, BBH_TX_GPON_PD_FIFO_SIZE_16_23}, {BBH_TX_GPON_PD_FIFO_SIZE_16_23, BBH_TX_GPON_PD_FIFO_SIZE_16_23},
          {BBH_TX_GPON_PD_FIFO_SIZE_16_23, BBH_TX_GPON_PD_FIFO_SIZE_16_23}, {BBH_TX_GPON_PD_FIFO_SIZE_16_23, BBH_TX_GPON_PD_FIFO_SIZE_16_23},
          {BBH_TX_GPON_PD_FIFO_SIZE_24_31, BBH_TX_GPON_PD_FIFO_SIZE_24_31}, {BBH_TX_GPON_PD_FIFO_SIZE_24_31, BBH_TX_GPON_PD_FIFO_SIZE_24_31}, 
          {BBH_TX_GPON_PD_FIFO_SIZE_24_31, BBH_TX_GPON_PD_FIFO_SIZE_24_31}, {BBH_TX_GPON_PD_FIFO_SIZE_24_31, BBH_TX_GPON_PD_FIFO_SIZE_24_31},
          {BBH_TX_GPON_PD_FIFO_SIZE_32_39, BBH_TX_GPON_PD_FIFO_SIZE_32_39}, {BBH_TX_GPON_PD_FIFO_SIZE_32_39, BBH_TX_GPON_PD_FIFO_SIZE_32_39}, 
          {BBH_TX_GPON_PD_FIFO_SIZE_32_39, BBH_TX_GPON_PD_FIFO_SIZE_32_39}, {BBH_TX_GPON_PD_FIFO_SIZE_32_39, BBH_TX_GPON_PD_FIFO_SIZE_32_39} };

pd_wkup_threshold_t g_gpon_pd_wkup_threshold[TX_QEUEU_PAIRS] = {
          {BBH_TX_GPON_PD_FIFO_SIZE_0_7 - 1, BBH_TX_GPON_PD_FIFO_SIZE_0_7 - 1}, {BBH_TX_GPON_PD_FIFO_SIZE_0_7 - 1, BBH_TX_GPON_PD_FIFO_SIZE_0_7 - 1},
          {BBH_TX_GPON_PD_FIFO_SIZE_0_7 - 1, BBH_TX_GPON_PD_FIFO_SIZE_0_7 - 1}, {BBH_TX_GPON_PD_FIFO_SIZE_0_7 - 1, BBH_TX_GPON_PD_FIFO_SIZE_0_7 - 1},
          {BBH_TX_GPON_PD_FIFO_SIZE_8_15 - 1, BBH_TX_GPON_PD_FIFO_SIZE_8_15 - 1}, {BBH_TX_GPON_PD_FIFO_SIZE_8_15 - 1, BBH_TX_GPON_PD_FIFO_SIZE_8_15 - 1},
          {BBH_TX_GPON_PD_FIFO_SIZE_8_15 - 1, BBH_TX_GPON_PD_FIFO_SIZE_8_15 - 1}, {BBH_TX_GPON_PD_FIFO_SIZE_8_15 - 1, BBH_TX_GPON_PD_FIFO_SIZE_8_15 - 1},
          {BBH_TX_GPON_PD_FIFO_SIZE_16_23 - 1, BBH_TX_GPON_PD_FIFO_SIZE_16_23 - 1}, {BBH_TX_GPON_PD_FIFO_SIZE_16_23 - 1, BBH_TX_GPON_PD_FIFO_SIZE_16_23 - 1},
          {BBH_TX_GPON_PD_FIFO_SIZE_16_23 - 1, BBH_TX_GPON_PD_FIFO_SIZE_16_23 - 1}, {BBH_TX_GPON_PD_FIFO_SIZE_16_23 - 1, BBH_TX_GPON_PD_FIFO_SIZE_16_23 - 1},
          {BBH_TX_GPON_PD_FIFO_SIZE_24_31 - 1, BBH_TX_GPON_PD_FIFO_SIZE_24_31 - 1}, {BBH_TX_GPON_PD_FIFO_SIZE_24_31 - 1, BBH_TX_GPON_PD_FIFO_SIZE_24_31 - 1}, 
          {BBH_TX_GPON_PD_FIFO_SIZE_24_31 - 1, BBH_TX_GPON_PD_FIFO_SIZE_24_31 - 1}, {BBH_TX_GPON_PD_FIFO_SIZE_24_31 - 1, BBH_TX_GPON_PD_FIFO_SIZE_24_31 - 1}, 
          {BBH_TX_GPON_PD_FIFO_SIZE_32_39 - 1, BBH_TX_GPON_PD_FIFO_SIZE_32_39 - 1}, {BBH_TX_GPON_PD_FIFO_SIZE_32_39 - 1, BBH_TX_GPON_PD_FIFO_SIZE_32_39 - 1}, 
          {BBH_TX_GPON_PD_FIFO_SIZE_32_39 - 1, BBH_TX_GPON_PD_FIFO_SIZE_32_39 - 1}, {BBH_TX_GPON_PD_FIFO_SIZE_32_39 - 1, BBH_TX_GPON_PD_FIFO_SIZE_32_39 - 1} };

pd_fifo_size_t g_dsl_pd_fifo_size[TX_QEUEU_PAIRS] = {
          {BBH_TX_DSL_PD_FIFO_SIZE_0_7, BBH_TX_DSL_PD_FIFO_SIZE_0_7}, {BBH_TX_DSL_PD_FIFO_SIZE_0_7, BBH_TX_DSL_PD_FIFO_SIZE_0_7},
          {BBH_TX_DSL_PD_FIFO_SIZE_0_7, BBH_TX_DSL_PD_FIFO_SIZE_0_7}, {BBH_TX_DSL_PD_FIFO_SIZE_0_7, BBH_TX_DSL_PD_FIFO_SIZE_0_7},
          {BBH_TX_DSL_PD_FIFO_SIZE_8_15, BBH_TX_DSL_PD_FIFO_SIZE_8_15}, {BBH_TX_DSL_PD_FIFO_SIZE_8_15, BBH_TX_DSL_PD_FIFO_SIZE_8_15},
          {BBH_TX_DSL_PD_FIFO_SIZE_8_15, BBH_TX_DSL_PD_FIFO_SIZE_8_15}, {BBH_TX_DSL_PD_FIFO_SIZE_8_15, BBH_TX_DSL_PD_FIFO_SIZE_8_15},
          {BBH_TX_DSL_PD_FIFO_SIZE_16_23, BBH_TX_DSL_PD_FIFO_SIZE_16_23}, {BBH_TX_DSL_PD_FIFO_SIZE_16_23, BBH_TX_DSL_PD_FIFO_SIZE_16_23},
          {BBH_TX_DSL_PD_FIFO_SIZE_16_23, BBH_TX_DSL_PD_FIFO_SIZE_16_23}, {BBH_TX_DSL_PD_FIFO_SIZE_16_23, BBH_TX_DSL_PD_FIFO_SIZE_16_23},
          {BBH_TX_DSL_PD_FIFO_SIZE_24_31, BBH_TX_DSL_PD_FIFO_SIZE_24_31}, {BBH_TX_DSL_PD_FIFO_SIZE_24_31, BBH_TX_DSL_PD_FIFO_SIZE_24_31}, 
          {BBH_TX_DSL_PD_FIFO_SIZE_24_31, BBH_TX_DSL_PD_FIFO_SIZE_24_31}, {BBH_TX_DSL_PD_FIFO_SIZE_24_31, BBH_TX_DSL_PD_FIFO_SIZE_24_31},
          {BBH_TX_DSL_PD_FIFO_SIZE_32_39, BBH_TX_DSL_PD_FIFO_SIZE_32_39}, {BBH_TX_DSL_PD_FIFO_SIZE_32_39, BBH_TX_DSL_PD_FIFO_SIZE_32_39}, 
          {BBH_TX_DSL_PD_FIFO_SIZE_32_39, BBH_TX_DSL_PD_FIFO_SIZE_32_39}, {BBH_TX_DSL_PD_FIFO_SIZE_32_39, BBH_TX_DSL_PD_FIFO_SIZE_32_39} };

pd_wkup_threshold_t g_dsl_pd_wkup_threshold[TX_QEUEU_PAIRS] = {
          {BBH_TX_DSL_PD_FIFO_SIZE_0_7 - 1, BBH_TX_DSL_PD_FIFO_SIZE_0_7 - 1}, {BBH_TX_DSL_PD_FIFO_SIZE_0_7 - 1, BBH_TX_DSL_PD_FIFO_SIZE_0_7 - 1},
          {BBH_TX_DSL_PD_FIFO_SIZE_0_7 - 1, BBH_TX_DSL_PD_FIFO_SIZE_0_7 - 1}, {BBH_TX_DSL_PD_FIFO_SIZE_0_7 - 1, BBH_TX_DSL_PD_FIFO_SIZE_0_7 - 1},
          {BBH_TX_DSL_PD_FIFO_SIZE_8_15 - 1, BBH_TX_DSL_PD_FIFO_SIZE_8_15 - 1}, {BBH_TX_DSL_PD_FIFO_SIZE_8_15 - 1, BBH_TX_DSL_PD_FIFO_SIZE_8_15 - 1},
          {BBH_TX_DSL_PD_FIFO_SIZE_8_15 - 1, BBH_TX_DSL_PD_FIFO_SIZE_8_15 - 1}, {BBH_TX_DSL_PD_FIFO_SIZE_8_15 - 1, BBH_TX_DSL_PD_FIFO_SIZE_8_15 - 1},
          {BBH_TX_DSL_PD_FIFO_SIZE_16_23, BBH_TX_DSL_PD_FIFO_SIZE_16_23}, {BBH_TX_DSL_PD_FIFO_SIZE_16_23, BBH_TX_DSL_PD_FIFO_SIZE_16_23},
          {BBH_TX_DSL_PD_FIFO_SIZE_16_23, BBH_TX_DSL_PD_FIFO_SIZE_16_23}, {BBH_TX_DSL_PD_FIFO_SIZE_16_23, BBH_TX_DSL_PD_FIFO_SIZE_16_23},
          {BBH_TX_DSL_PD_FIFO_SIZE_24_31, BBH_TX_DSL_PD_FIFO_SIZE_24_31}, {BBH_TX_DSL_PD_FIFO_SIZE_24_31, BBH_TX_DSL_PD_FIFO_SIZE_24_31}, 
          {BBH_TX_DSL_PD_FIFO_SIZE_24_31, BBH_TX_DSL_PD_FIFO_SIZE_24_31}, {BBH_TX_DSL_PD_FIFO_SIZE_24_31, BBH_TX_DSL_PD_FIFO_SIZE_24_31}, 
          {BBH_TX_DSL_PD_FIFO_SIZE_32_39, BBH_TX_DSL_PD_FIFO_SIZE_32_39}, {BBH_TX_DSL_PD_FIFO_SIZE_32_39, BBH_TX_DSL_PD_FIFO_SIZE_32_39}, 
          {BBH_TX_DSL_PD_FIFO_SIZE_32_39, BBH_TX_DSL_PD_FIFO_SIZE_32_39}, {BBH_TX_DSL_PD_FIFO_SIZE_32_39, BBH_TX_DSL_PD_FIFO_SIZE_32_39} };


bbh_to_dma_x_t  g_bbh_rx_to_dma_x[BBH_ID_NUM] = { 
                                             {BBH_ID_0, DMA0_ID},
                                             {BBH_ID_1, DMA0_ID},
                                             {BBH_ID_2, DMA0_ID},
                                             {BBH_ID_PON, DMA0_ID},
                                             {BBH_ID_AE10, DMA0_ID},
                                             {BBH_ID_AE2P5, DMA0_ID},
                                             {BBH_ID_DSL, DMA0_ID}};

bbh_to_dma_x_t  g_bbh_rx_to_sdma_x[BBH_ID_NUM] = { 
                                             {BBH_ID_0, SDMA0_ID},
                                             {BBH_ID_1, SDMA0_ID},
                                             {BBH_ID_2, SDMA0_ID},
                                             {BBH_ID_PON, SDMA0_ID},
                                             {BBH_ID_AE10, SDMA1_ID},
                                             {BBH_ID_AE2P5, SDMA1_ID},
                                             {BBH_ID_DSL, SDMA1_ID}};

bbh_id_e g_dma_to_bbh_rx_x[NUM_OF_DMA][NUM_OF_PERIPHERALS_PER_DMA] = {
  {BBH_ID_0, BBH_ID_1, BBH_ID_2, BBH_ID_PON, BBH_ID_AE10, BBH_ID_AE2P5, BBH_ID_DSL}};

bbh_id_e g_sdma_to_bbh_rx_x[NUM_OF_SDMA][NUM_OF_PERIPHERALS_PER_DMA] = {
  {BBH_ID_0, BBH_ID_1, BBH_ID_2, BBH_ID_PON, BBH_ID_NULL, BBH_ID_NULL, BBH_ID_NULL},
  {BBH_ID_AE10, BBH_ID_AE2P5, BBH_ID_DSL, BBH_ID_NULL, BBH_ID_NULL, BBH_ID_NULL, BBH_ID_NULL}};

bbh_to_dma_x_t  g_bbh_tx_to_dma_x[BBH_ID_NUM] = { 
                                             {BBH_TX_ID_0, DMA0_ID},
                                             {BBH_TX_ID_PON, DMA0_ID},
                                             {BBH_TX_ID_AE10, DMA0_ID},
                                             {BBH_TX_ID_AE2P5, DMA0_ID},
                                             {BBH_TX_ID_DSL, DMA0_ID}};

bbh_to_dma_x_t  g_bbh_tx_to_sdma_x[BBH_ID_NUM] = { 
                                             {BBH_TX_ID_0, SDMA0_ID},
                                             {BBH_TX_ID_PON, SDMA0_ID},
                                             {BBH_TX_ID_AE10, SDMA1_ID},
                                             {BBH_TX_ID_AE2P5, SDMA1_ID},
                                             {BBH_TX_ID_DSL, SDMA1_ID}};

bbh_id_e g_dma_to_bbh_tx_x[NUM_OF_DMA][NUM_OF_PERIPHERALS_PER_DMA] = {
  {BBH_TX_ID_0, BBH_TX_ID_PON, BBH_TX_ID_AE10, BBH_TX_ID_AE2P5, BBH_TX_ID_DSL, BBH_TX_ID_NULL, BBH_TX_ID_NULL}};

bbh_id_e g_sdma_to_bbh_tx_x[NUM_OF_SDMA][NUM_OF_PERIPHERALS_PER_DMA] = {
  {BBH_TX_ID_0, BBH_TX_ID_PON, BBH_TX_ID_NULL, BBH_TX_ID_NULL, BBH_TX_ID_NULL, BBH_TX_ID_NULL, BBH_TX_ID_NULL},
  {BBH_TX_ID_AE10, BBH_TX_ID_AE2P5, BBH_TX_ID_DSL, BBH_TX_ID_NULL, BBH_TX_ID_NULL, BBH_TX_ID_NULL, BBH_TX_ID_NULL} };

/* array desription:
      - this array describe the buffers weight according to g_dma_to_bbhX array.
      - total 48 for rx, 64 for tx for each DMA */
uint8_t g_bbh_sdma_rx_buff_num[NUM_OF_SDMA][NUM_OF_PERIPHERALS_PER_DMA] = {{8, 8, 8, 24, 0, 0, 0}, {24, 12, 12, 0, 0, 0, 0}};

uint8_t g_bbh_dma_tx_buff_num[NUM_OF_DMA][NUM_OF_PERIPHERALS_PER_DMA] = {{12, 12, 12, 12, 12, 0, 0}};
uint8_t g_bbh_sdma_tx_buff_num[NUM_OF_SDMA][NUM_OF_PERIPHERALS_PER_DMA] = {{32, 32, 0, 0, 0, 0, 0}, {32, 16, 16, 0, 0, 0, 0}};

bb_source_t g_dma_bb_source[DMA_NUM][BBH_ID_NUM] = {  /* DMA, SDMA0, SDMA1.   each entry is {rx,  tx} */
          {{0, BB_ID_TX_LAN},
           {0, BB_ID_TX_PON},
           {0, BB_ID_TX_10G},
           {0, BB_ID_TX_2P5},
           {0, BB_ID_TX_DSL},
           {0, 0},
           {0, 0}},

          {{BB_ID_RX_BBH_0, BB_ID_TX_LAN},
           {BB_ID_RX_BBH_1, BB_ID_TX_PON},
           {BB_ID_RX_BBH_2, 0},
           {BB_ID_RX_PON, 0},
           {0, 0},
           {0, 0},
           {0, 0}},

          {{BB_ID_RX_10G, BB_ID_TX_10G},
           {BB_ID_RX_2P5, BB_ID_TX_2P5},
           {BB_ID_RX_DSL, BB_ID_TX_DSL},
           {0, 0},
           {0, 0},
           {0, 0},
           {0, 0}}};

urgent_threhold_t g_dma_urgent_threshold[DMA_NUM][BBH_ID_NUM] = {
          {{DMA_U_THRESH_IN_BBH_LAN_VALUE, DMA_U_THRESH_OUT_BBH_LAN_VALUE},
           {DMA_U_THRESH_IN_BBH_LAN_VALUE, DMA_U_THRESH_OUT_BBH_LAN_VALUE},
           {DMA_U_THRESH_IN_BBH_LAN_VALUE, DMA_U_THRESH_OUT_BBH_LAN_VALUE}, 
           {DMA_U_THRESH_IN_BBH_PON_VALUE, DMA_U_THRESH_OUT_BBH_PON_VALUE},
           {DMA_U_THRESH_IN_BBH_XPORT10G_VALUE, DMA_U_THRESH_OUT_BBH_XPORT10G_VALUE},
           {DMA_U_THRESH_IN_BBH_XPORT2P5G_VALUE, DMA_U_THRESH_OUT_BBH_XPORT2P5G_VALUE},
           {DMA_U_THRESH_IN_BBH_DSL_VALUE, DMA_U_THRESH_OUT_BBH_DSL_VALUE}},

          {{SDMA_U_THRESH_IN_BBH_LAN_VALUE, SDMA_U_THRESH_OUT_BBH_LAN_VALUE},
           {SDMA_U_THRESH_IN_BBH_LAN_VALUE, SDMA_U_THRESH_OUT_BBH_LAN_VALUE},
           {SDMA_U_THRESH_IN_BBH_LAN_VALUE, SDMA_U_THRESH_OUT_BBH_LAN_VALUE},
           {SDMA_U_THRESH_IN_BBH_PON_VALUE, SDMA_U_THRESH_OUT_BBH_PON_VALUE},
           {0, 0},
           {0, 0},
           {0, 0}},

          {{SDMA_U_THRESH_IN_BBH_XPORT10G_VALUE, SDMA_U_THRESH_OUT_BBH_XPORT10G_VALUE},
           {SDMA_U_THRESH_IN_BBH_XPORT2P5G_VALUE, SDMA_U_THRESH_OUT_BBH_XPORT2P5G_VALUE},
           {SDMA_U_THRESH_IN_BBH_DSL_VALUE, SDMA_U_THRESH_OUT_BBH_DSL_VALUE},
           {0, 0},
           {0, 0},
           {0, 0},
           {0, 0}}};

strict_priority_t g_dma_strict_priority[DMA_NUM][BBH_ID_NUM] = {
          {{DMA_STRICT_PRI_RX_BBH_LAN_VALUE, DMA_STRICT_PRI_TX_BBH_LAN_VALUE},
           {DMA_STRICT_PRI_RX_BBH_LAN_VALUE, DMA_STRICT_PRI_TX_BBH_LAN_VALUE},
           {DMA_STRICT_PRI_RX_BBH_LAN_VALUE, DMA_STRICT_PRI_TX_BBH_LAN_VALUE}, 
           {DMA_STRICT_PRI_RX_BBH_PON_VALUE, DMA_STRICT_PRI_TX_BBH_PON_VALUE},
           {DMA_STRICT_PRI_RX_BBH_XPORT10G_VALUE, DMA_STRICT_PRI_TX_BBH_XPORT10G_VALUE},
           {DMA_STRICT_PRI_RX_BBH_XPORT2P5G_VALUE, DMA_STRICT_PRI_TX_BBH_XPORT2P5G_VALUE},
           {DMA_STRICT_PRI_RX_BBH_DSL_VALUE, DMA_STRICT_PRI_TX_BBH_DSL_VALUE}},

          {{SDMA_STRICT_PRI_RX_BBH_LAN_VALUE, SDMA_STRICT_PRI_TX_BBH_LAN_VALUE},
           {SDMA_STRICT_PRI_RX_BBH_LAN_VALUE, SDMA_STRICT_PRI_TX_BBH_LAN_VALUE},
           {SDMA_STRICT_PRI_RX_BBH_LAN_VALUE, SDMA_STRICT_PRI_TX_BBH_LAN_VALUE},
           {SDMA_STRICT_PRI_RX_BBH_PON_VALUE, SDMA_STRICT_PRI_TX_BBH_PON_VALUE},
           {SDMA_STRICT_PRI_RX_BBH_XPORT10G_VALUE, SDMA_STRICT_PRI_TX_BBH_XPORT10G_VALUE},
           {SDMA_STRICT_PRI_RX_BBH_XPORT2P5G_VALUE, SDMA_STRICT_PRI_TX_BBH_XPORT2P5G_VALUE},
           {SDMA_STRICT_PRI_RX_BBH_DSL_VALUE, SDMA_STRICT_PRI_TX_BBH_DSL_VALUE}},

          {{SDMA_STRICT_PRI_RX_BBH_LAN_VALUE, SDMA_STRICT_PRI_TX_BBH_LAN_VALUE},
           {SDMA_STRICT_PRI_RX_BBH_LAN_VALUE, SDMA_STRICT_PRI_TX_BBH_LAN_VALUE},
           {SDMA_STRICT_PRI_RX_BBH_LAN_VALUE, SDMA_STRICT_PRI_TX_BBH_LAN_VALUE},
           {SDMA_STRICT_PRI_RX_BBH_PON_VALUE, SDMA_STRICT_PRI_TX_BBH_PON_VALUE},
           {SDMA_STRICT_PRI_RX_BBH_XPORT10G_VALUE, SDMA_STRICT_PRI_TX_BBH_XPORT10G_VALUE},
           {SDMA_STRICT_PRI_RX_BBH_XPORT2P5G_VALUE, SDMA_STRICT_PRI_TX_BBH_XPORT2P5G_VALUE},
           {SDMA_STRICT_PRI_RX_BBH_DSL_VALUE, SDMA_STRICT_PRI_TX_BBH_DSL_VALUE}}};

rr_weight_t g_dma_rr_weight[DMA_NUM][BBH_ID_NUM] = {
          {{DMA_RR_WEIGHT_RX_BBH_LAN_VALUE, DMA_RR_WEIGHT_TX_BBH_LAN_VALUE},
           {DMA_RR_WEIGHT_RX_BBH_LAN_VALUE, DMA_RR_WEIGHT_TX_BBH_LAN_VALUE},
           {DMA_RR_WEIGHT_RX_BBH_LAN_VALUE, DMA_RR_WEIGHT_TX_BBH_LAN_VALUE}, 
           {DMA_RR_WEIGHT_RX_BBH_PON_VALUE, DMA_RR_WEIGHT_TX_BBH_PON_VALUE},
           {DMA_RR_WEIGHT_RX_BBH_XPORT10G_VALUE, DMA_RR_WEIGHT_TX_BBH_XPORT10G_VALUE},
           {DMA_RR_WEIGHT_RX_BBH_XPORT2P5G_VALUE, DMA_RR_WEIGHT_TX_BBH_XPORT2P5G_VALUE},
           {DMA_RR_WEIGHT_RX_BBH_DSL_VALUE, DMA_RR_WEIGHT_TX_BBH_DSL_VALUE}},

          {{SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE, SDMA_RR_WEIGHT_TX_BBH_LAN_VALUE},
           {SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE, SDMA_RR_WEIGHT_TX_BBH_LAN_VALUE},
           {SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE, SDMA_RR_WEIGHT_TX_BBH_LAN_VALUE},
           {SDMA_RR_WEIGHT_RX_BBH_PON_VALUE, SDMA_RR_WEIGHT_TX_BBH_PON_VALUE},
           {SDMA_RR_WEIGHT_RX_BBH_XPORT10G_VALUE, SDMA_RR_WEIGHT_TX_BBH_XPORT10G_VALUE},
           {SDMA_RR_WEIGHT_RX_BBH_XPORT2P5G_VALUE, SDMA_RR_WEIGHT_TX_BBH_XPORT2P5G_VALUE},
           {SDMA_RR_WEIGHT_RX_BBH_DSL_VALUE, SDMA_RR_WEIGHT_TX_BBH_DSL_VALUE}},

          {{SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE, SDMA_RR_WEIGHT_TX_BBH_LAN_VALUE},
           {SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE, SDMA_RR_WEIGHT_TX_BBH_LAN_VALUE},
           {SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE, SDMA_RR_WEIGHT_TX_BBH_LAN_VALUE},
           {SDMA_RR_WEIGHT_RX_BBH_PON_VALUE, SDMA_RR_WEIGHT_TX_BBH_PON_VALUE},
           {SDMA_RR_WEIGHT_RX_BBH_XPORT10G_VALUE, SDMA_RR_WEIGHT_TX_BBH_XPORT10G_VALUE},
           {SDMA_RR_WEIGHT_RX_BBH_XPORT2P5G_VALUE, SDMA_RR_WEIGHT_TX_BBH_XPORT2P5G_VALUE},
           {SDMA_RR_WEIGHT_RX_BBH_DSL_VALUE, SDMA_RR_WEIGHT_TX_BBH_DSL_VALUE}}};

uint8_t g_max_otf_reads[BBH_ID_NUM] = {MAX_OTF_READ_REQUEST_DEFAULT_DMA0,
                                       MAX_OTF_READ_REQUEST_DEFAULT_DMA0,
                                       MAX_OTF_READ_REQUEST_DEFAULT_DMA0,
                                       MAX_OTF_READ_REQUEST_DEFAULT_DMA0,
                                       MAX_OTF_READ_REQUEST_DEFAULT_DMA0,
                                       MAX_OTF_READ_REQUEST_DEFAULT_DMA0,
                                       MAX_OTF_READ_REQUEST_DEFAULT_DMA0};


static uint8_t calculate_buffers_offset(uint8_t dma_num, uint8_t periphreal_num, uint8_t bbh_buff_num[][NUM_OF_PERIPHERALS_PER_DMA])
{
    uint8_t j, offset = 0;

    for (j = 0; j < periphreal_num; j++)
    {
       offset = offset + bbh_buff_num[dma_num][j];
    }
    return offset;
}

static void bbh_rx_profiles_init(void)
{
    uint8_t i, j, bbh_id;
    bbh_rx_sdma_chunks_cfg_t *bbh_rx_sdma_cfg;

    g_bbh_rx_sdma_profile = (bbh_rx_sdma_profile_t *)xrdp_alloc(sizeof(bbh_rx_sdma_profile_t));

    for (bbh_id = 0; bbh_id < BBH_ID_NUM; bbh_id++)
    {
        /* serach BBH_ID in DMA/SDMA array*/
        for (i = 0; i < NUM_OF_SDMA; i++)
        {
            for (j = 0; j < NUM_OF_PERIPHERALS_PER_DMA; j++)
            {
                if (g_bbh_rx_to_sdma_x[bbh_id].bbh_id == g_sdma_to_bbh_rx_x[i][j]) 
				{
                    goto bbh_rx_profiles_init_exit;
                }
            }
        }
		
		continue;
		
bbh_rx_profiles_init_exit:

        bbh_rx_sdma_cfg = &(g_bbh_rx_sdma_profile->bbh_rx_sdma_chunks_config[g_bbh_rx_to_sdma_x[bbh_id].bbh_id]);
        bbh_rx_sdma_cfg->sdma_bb_id = (g_bbh_rx_to_sdma_x[bbh_id].dma_id == SDMA0_ID)? BB_ID_SDMA0 : BB_ID_SDMA1;
        bbh_rx_sdma_cfg->first_chunk_idx = calculate_buffers_offset(i, j, g_bbh_sdma_rx_buff_num);
        bbh_rx_sdma_cfg->sdma_chunks = g_bbh_sdma_rx_buff_num[i][j];
    }
}

static void bbh_tx_profiles_init(void)
{
    uint8_t i, j, bbh_id;
    bbh_tx_bbh_dma_cfg *bbh_tx_dma_cfg;
    bbh_tx_bbh_sdma_cfg *bbh_tx_sdma_cfg;
    bbh_tx_bbh_ddr_cfg *bbh_tx_ddr_cfg;

    g_bbh_tx_dma_profile = (bbh_tx_dma_profile_t *)xrdp_alloc(sizeof(bbh_tx_dma_profile_t));
    g_bbh_tx_sdma_profile = (bbh_tx_sdma_profile_t *)xrdp_alloc(sizeof(bbh_tx_sdma_profile_t));
    g_bbh_tx_ddr_profile = (bbh_tx_ddr_profile_t *)xrdp_alloc(sizeof(bbh_tx_ddr_profile_t));

    for (bbh_id = BBH_TX_ID_FIRST; bbh_id < BBH_TX_ID_NUM; bbh_id++)
    {
        /* serach BBH_ID in DMA array */
        for (i = 0; i < NUM_OF_DMA; i++)
        {
            for (j = 0; j < NUM_OF_PERIPHERALS_PER_DMA; j++)
            {
                if (g_bbh_tx_to_dma_x[bbh_id].bbh_id == g_dma_to_bbh_tx_x[i][j])
                {
                    goto bbh_dma_tx_profiles_init_exit;
                }
            }
        }
        continue;
         
bbh_dma_tx_profiles_init_exit:

        bbh_tx_dma_cfg = &(g_bbh_tx_dma_profile->bbh_tx_dma_cfg[g_bbh_tx_to_dma_x[bbh_id].bbh_id]);
        bbh_tx_dma_cfg->dmasrc =  BB_ID_DMA0;
        bbh_tx_dma_cfg->descbase = calculate_buffers_offset(i, j, g_bbh_dma_tx_buff_num);
        bbh_tx_dma_cfg->descsize = g_bbh_dma_tx_buff_num[i][j];

        bbh_tx_ddr_cfg = &(g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[g_bbh_tx_to_dma_x[bbh_id].bbh_id]);

        lookup_bbh_tx_bufsz_by_fpm_bufsz(&p_dpi_cfg->fpm_buf_size, &bbh_tx_ddr_cfg->bufsize);

        bbh_tx_ddr_cfg->byteresul = RES_1B;
        bbh_tx_ddr_cfg->ddrtxoffset = 0;
        /* MIN size = 0x4 - MAX size = 0x40  where to check values?*/
        bbh_tx_ddr_cfg->hnsize0 = 0x20;
        bbh_tx_ddr_cfg->hnsize1 = 0x20;
    }

    for (bbh_id = BBH_TX_ID_FIRST; bbh_id < BBH_TX_ID_NUM; bbh_id++)
    {
        /* serach BBH_ID in SDMA array */
        for (i = 0; i < NUM_OF_SDMA; i++)
        {
            for (j = 0; j < NUM_OF_PERIPHERALS_PER_DMA; j++)
            {
                if (g_bbh_tx_to_sdma_x[bbh_id].bbh_id == g_sdma_to_bbh_tx_x[i][j])
                {
                    goto bbh_sdma_tx_profiles_init_exit;
                }
            }
        }
        continue;
         
bbh_sdma_tx_profiles_init_exit:

        bbh_tx_sdma_cfg = &(g_bbh_tx_sdma_profile->bbh_tx_sdma_cfg[g_bbh_tx_to_sdma_x[bbh_id].bbh_id]);
        bbh_tx_sdma_cfg->sdmasrc = (g_bbh_tx_to_sdma_x[bbh_id].dma_id == SDMA0_ID)? BB_ID_SDMA0 :BB_ID_SDMA1 ;
        bbh_tx_sdma_cfg->descbase = calculate_buffers_offset(i, j, g_bbh_sdma_tx_buff_num);
        bbh_tx_sdma_cfg->descsize = g_bbh_sdma_tx_buff_num[i][j];
    }
    return;
}

static int bbh_rx_cfg(bbh_id_e bbh_id)
{
    uint8_t i;
    bdmf_error_t rc = BDMF_ERR_OK;
    bbh_rx_config cfg;

    xrdp_memset(&cfg, 0, sizeof(bbh_rx_config));

    cfg.sdma_chunks_cfg = &(g_bbh_rx_sdma_profile->bbh_rx_sdma_chunks_config[bbh_id]);
    cfg.disp_bb_id = BB_ID_DISPATCHER_REORDER;
    cfg.sbpm_bb_id = BB_ID_SBPM;

    /* TODO: should match other blocks configurations (runner) */
    /* TODO: should it be a profile as well ? */
    cfg.normal_viq = bbh_id;
    cfg.excl_viq = (bbh_id == BBH_ID_PON) ? (BBH_ID_NUM + bbh_id) : bbh_id;
    cfg.excl_cfg.exc_en = (bbh_id == BBH_ID_PON);

    for (i = 0; i < 4; i++)
    {
        if (IS_WAN_RX_PORT(bbh_id))
            cfg.min_pkt_size[i] = MIN_WAN_PKT_SIZE;
        else
            cfg.min_pkt_size[i] = MIN_ETH_PKT_SIZE;
        cfg.max_pkt_size[i] = p_dpi_cfg->mtu_size;
    }

    if (IS_WAN_RX_PORT(bbh_id))
        cfg.min_pkt_size[1] = MIN_OMCI_PKT_SIZE;

    cfg.crc_omit_dis = 0;
    cfg.sop_offset = SOP_OFFSET;
    cfg.per_flow_th = BBH_RX_FLOWS_32_255_GROUP_DIVIDER;

    cfg.max_otf_sbpm = DRV_BBH_RX_MAXIMUM_OTF_SBPM_REQUESTS_DEFAULT_VAL;

    /*config.flow_ctrl_cfg.drops = ?*/
    /*config.flow_ctrl_cfg.timer = ?*/

    rc = rc ? rc : drv_bbh_rx_configuration_set(bbh_id, &cfg);
    rc = rc ? rc : ag_drv_bbh_rx_mac_mode_set(bbh_id, cfg.mac_mode_cfg.is_epon, cfg.mac_mode_cfg.is_xgpon, cfg.mac_mode_cfg.is_vdsl);

#if !defined(RDP_SIM)
//|| defined(XRDP_EMULATION)
    /* initialize all flows (including the 2 groups) */
    for (i = 0; i < DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_0 / 2; i++)
    {
        rc = rc ? rc : ag_drv_bbh_rx_min_pkt_sel_flows_0_15_set(bbh_id, i, 0);
        rc = rc ? rc : ag_drv_bbh_rx_max_pkt_sel_flows_0_15_set(bbh_id, i, 0);
        rc = rc ? rc : ag_drv_bbh_rx_min_pkt_sel_flows_16_31_set(bbh_id, i, 0);
        rc = rc ? rc : ag_drv_bbh_rx_max_pkt_sel_flows_16_31_set(bbh_id, i, 0);
    }

    rc = rc ? rc : ag_drv_bbh_rx_pkt_sel_group_0_set(bbh_id, 0, 0);
    rc = rc ? rc : ag_drv_bbh_rx_pkt_sel_group_1_set(bbh_id, 0, 0);
#endif

    return rc;
}

static int bbh_rx_pon_init(int wan_bbh)
{
    bdmf_error_t rc = BDMF_ERR_OK;

    /* BBH-RX mactype init */
    rc = ag_drv_bbh_rx_mac_mode_set(BBH_ID_PON, (wan_bbh == MAC_TYPE_EPON || wan_bbh == MAC_TYPE_XEPON),
        (wan_bbh == MAC_TYPE_XGPON || wan_bbh == MAC_TYPE_XEPON), 0);
    
    if (wan_bbh == MAC_TYPE_GPON || wan_bbh == MAC_TYPE_XGPON)
    {
        rc = rc ? rc : ag_drv_bbh_rx_ploam_en_set(BBH_ID_PON, 1);
        rc = rc ? rc : ag_drv_bbh_rx_user_priority3_en_set(BBH_ID_PON, 1);
    }

    rc = rc ? rc : bbh_rx_cfg(BBH_ID_PON);

    if (rc)
    {
        BDMF_TRACE_ERR("Error whlie configuring wan bbh_rx %d\n", BBH_ID_PON);
        return rc;
    }
    rc = rc ? rc : ag_drv_bbh_rx_general_configuration_enable_set(BBH_ID_PON, 1, 1);

    return rc;
}

static int bbh_rx_dsl_init(int wan_bbh)
{
    bdmf_error_t rc = BDMF_ERR_OK;

    rc = rc ? rc : bbh_rx_cfg(BBH_ID_DSL);

    if (rc)
    {
        BDMF_TRACE_ERR("Error whlie configuring wan bbh_rx %d\n", BBH_ID_DSL);
        return rc;
    }

    /* BBH-RX mactype init */
    rc = ag_drv_bbh_rx_mac_mode_set(BBH_ID_DSL, 0, 0, 1) ;
    
    /* Disable CRC omit functionality as it is controlled at DSL SAR based on
    ** DSL modes.
    **/
    rc = rc ? rc : ag_drv_bbh_rx_general_configuration_crcomitdis_set(BBH_ID_DSL, 1);

    rc = rc ? rc : ag_drv_bbh_rx_general_configuration_enable_set(BBH_ID_DSL, 1, 1);

    return rc;
}


static int bbh_rx_init(void)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    bbh_id_e bbh_id;

    for (bbh_id = BBH_ID_FIRST; bbh_id < BBH_ID_NUM; bbh_id++)
    {
        if (bbh_id == BBH_ID_PON) continue;  /* Skip PON. It has its own */
        if (bbh_id == BBH_ID_DSL) continue;  /* Skip DSL. It has its own */

        rc = rc ? rc : bbh_rx_cfg(bbh_id);

        if (rc)
        {
            BDMF_TRACE_ERR("Error whlie configuring bbh_rx %d\n", bbh_id);
            return rc;
        }
    }
    return rc;
}

static void ubus_bridge_init(void)
{
    ag_drv_ubus_slv_vpb_base_set(UBUS_SLV_VPB_BASE_ADDR);
    ag_drv_ubus_slv_vpb_mask_set(UBUS_SLV_MASK);
    ag_drv_ubus_slv_apb_base_set(UBUS_SLV_APB_BASE_ADDR);
    ag_drv_ubus_slv_apb_mask_set(UBUS_SLV_MASK);

    ag_drv_ubus_mstr_hyst_ctrl_set(UBUS_MSTR_ID_0, UBUS_MSTR_CMD_SPCAE, UBUS_MSTR_DATA_SPCAE);
    // TBD verify ag_drv_ubus_mstr_hyst_ctrl_set(0, 2, 2);  ubus_mstr_id, cmd_space, data_space
}

uint32_t fpm_get_dqm_extra_fpm_tokens(void)
{
    return 0;
}

static int fpm_init(void)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    bdmf_boolean reset_req = 1;
    fpm_pool2_intr_sts interrupt_status = FPM_INTERRUPT_STATUS;
    fpm_pool2_intr_msk interrupt_mask = FPM_INTERRUPT_MASK;
    uint16_t timeout = 0;
    fpm_pool_cfg pool_cfg = {};

     /* pool configurations */ 
    pool_cfg.fpm_buf_size = FPM_BUF_SIZE_DEFAULT == FPM_BUF_SIZE_0 ? 0 : 1;
    pool_cfg.pool_base_address = FPM_POOL_BASE_ADDRESS;
    pool_cfg.pool_base_address_pool2 = 0;
    
    drv_fpm_init(p_dpi_cfg->rdp_ddr_pkt_base_virt, p_dpi_cfg->fpm_buf_size);

    rc = rc ? rc : ag_drv_fpm_init_mem_set(1);

    /* polling until reset is finished */
    while (!rc && reset_req && timeout <= FPM_INIT_TIMEOUT)
    {
        rc = rc ? rc : ag_drv_fpm_init_mem_get(&reset_req);
        xrdp_usleep(FPM_POLL_SLEEP_TIME);
        timeout++;
    }
    if (timeout == FPM_INIT_TIMEOUT)
        rc = BDMF_ERR_INTERNAL;

    /* enable the fpm pool */
    rc = rc ? rc : ag_drv_fpm_pool1_en_set(1);

    /* fpm configurations */
    rc = rc ? rc : ag_drv_fpm_pool_cfg_set(&pool_cfg);

    /* fpm interrupts */
    rc = rc ? rc : ag_drv_fpm_pool1_intr_sts_set(&interrupt_status);
    rc = rc ? rc : ag_drv_fpm_pool1_intr_msk_set(&interrupt_mask);

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize fpm driver\n");

    return rc;
}

static int cnpl_init(bdmf_boolean is_gateway, bdmf_boolean vlan_stats_enable)
{
    int rc;

    /* reset counters-policers memory */
    rc = drv_cnpl_memory_data_init();
    /* init counter groups according to project */
    rc = rc ? rc : drv_cntr_group_init(is_gateway, vlan_stats_enable);
    return rc;
}

static int parser_init(void)
{
    int rc = BDMF_ERR_OK;
    uint8_t i;

    bdmf_trace("%s  %d\n", __FUNCTION__, __LINE__);
    /* configure runner quads */
    for (i = 0; !rc && i < NUM_OF_RNR_QUAD; i++)
    {
    bdmf_trace("%s  %d\n", __FUNCTION__, __LINE__);
        /* parser configuration */
        rc = rc ? rc : ag_drv_rnr_quad_tcp_flags_set(i, PARSER_TCP_CTL_FLAGS);
        rc = rc ? rc : ag_drv_rnr_quad_profile_us_set(i, PARSER_PROFILE_US);
        rc = rc ? rc : ag_drv_rnr_quad_exception_bits_set(i, PARSER_EXCP_STATUS_BITS);
        rc = rc ? rc : ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_set(i,
            PARSER_PPP_PROTOCOL_CODE_0_IPV4, PARSER_PPP_PROTOCOL_CODE_1_IPV6);
        rc = rc ? rc : ag_drv_rnr_quad_parser_core_configuration_eng_set(i, PARSER_AH_DETECTION);
        rc = rc ? rc : ag_drv_rnr_quad_parser_ip_protocol3_set(i, PARSER_IP_PROTOCOL_IPIP);
    }

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize parser driver\n");

    return rc;
}

static int hash_init(void)
{
    hash_config_t hash_cfg = {};

    hash_cfg.tbl_num = HASH_TABLE_LAST;

    /* IPTV table configuration */
    hash_cfg.tbl_cfg[GET_SW_TABLE_ID(HASH_TABLE_IPTV)].tbl_size = HASH_TABLE_IPTV_SIZE_PER_ENGINE;
    hash_cfg.tbl_cfg[GET_SW_TABLE_ID(HASH_TABLE_IPTV)].cam_en = 1;
    hash_cfg.tbl_init[GET_SW_TABLE_ID(HASH_TABLE_IPTV)].int_ctx_size = 3;
    hash_cfg.tbl_init[GET_SW_TABLE_ID(HASH_TABLE_IPTV)].ext_ctx_size = 0;
    hash_cfg.tbl_init[GET_SW_TABLE_ID(HASH_TABLE_IPTV)].search_depth_per_engine = 4;
    /* 32 bit key */
    hash_cfg.mask[GET_SW_TABLE_ID(HASH_TABLE_IPTV)].maskl = 0xFFFFFFFF;
    hash_cfg.mask[GET_SW_TABLE_ID(HASH_TABLE_IPTV)].maskh = 0x000FFFF;

    /* reset hash memory */
    return drv_hash_init(&hash_cfg);
}


static int sbpm_init(void)
{
    bdmf_error_t rc;
    uint16_t base_addr;
    uint16_t init_offset;
    sbpm_thr_ug thr_ug = {};

    /* base address and offset */
    base_addr = SBPM_BASE_ADDRESS;
    init_offset = SBPM_INIT_OFFSET;

    rc = ag_drv_sbpm_regs_init_free_list_set(base_addr, init_offset, 0, 0);

    rc = rc ? rc : drv_sbpm_thr_ug0_get(&thr_ug);
    thr_ug.bn_thr = SBPM_UG0_BN_THRESHOLD;
    /* TODO : temporary : for primitive Ingress Qos implementation in FW */
    thr_ug.excl_low_thr = SBPM_UG0_EXCL_LOW_THRESHOLD;
    thr_ug.excl_low_hyst = SBPM_UG0_EXCL_LOW_HIST;
    rc = rc ? rc : drv_sbpm_thr_ug0_set(&thr_ug);
        
    rc = rc ? rc : drv_sbpm_thr_ug1_get(&thr_ug);
    thr_ug.bn_thr = SBPM_UG1_BN_THRESHOLD;
    thr_ug.excl_low_thr = SBPM_UG1_EXCL_LOW_THRESHOLD;
    thr_ug.excl_low_hyst = SBPM_UG1_EXCL_LOW_HIST;
    rc = rc ? rc : drv_sbpm_thr_ug1_set(&thr_ug);

#ifdef RDP_SIM
    drv_sbpm_default_val_init();
#endif
    /* the following is for BBH_TX_XTM workaround, where US_TM core will
     * allocate from UG#1 */
    rc = rc ? rc : drv_sbpm_runner_sp_set(get_runner_idx(us_tm_runner_image), 1);

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize sbpm driver\n");

    return rc;
}


static int bbh_tx_lan_init(void)
{
    bbh_tx_config config;
    uint8_t i, max_on_the_fly_reads;
    bdmf_phys_addr_t fpm_base_phys_addr;
    bdmf_error_t rc = BDMF_ERR_OK;
    pd_fifo_base_t lan_pd_fifo_base[LAN_QUEUE_PAIRS] = {};

    /* init fifo base arrays */
    lan_pd_fifo_base[0].base0 = 0; 
    lan_pd_fifo_base[0].base1 = lan_pd_fifo_base[0].base0 + g_lan_pd_fifo_size[0].size0 + 1;

    for (i = 1; i < LAN_QUEUE_PAIRS; ++i)
    {
        lan_pd_fifo_base[i].base0 = lan_pd_fifo_base[i-1].base1 + g_lan_pd_fifo_size[i-1].size1 + 1;
        lan_pd_fifo_base[i].base1 = lan_pd_fifo_base[i].base0 + g_lan_pd_fifo_size[i].size0 + 1;
    }

    xrdp_memset(&config, 0, sizeof(bbh_tx_config));

    /* cores which sending PDs */
    config.mac_type = MAC_TYPE_GPON; /* The unified BBH TX is based on GPON BBH TX */
    config.rnr_cfg[0].skb_addr = 0;
    config.rnr_cfg[0].rnr_src_id = get_runner_idx(ds_tm_runner_image);
    config.rnr_cfg[0].tcont_addr = IMAGE_0_DS_TM_BBH_TX_QUEUE_ID_TABLE_ADDRESS >> 3;
    config.rnr_cfg[0].task_number = IMAGE_0_DS_TM_TX_TASK_THREAD_NUMBER;
    config.rnr_cfg[0].ptr_addr = IMAGE_0_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS >> 3;

    /* enable per queue task number */
    config.per_q_task.task0 = IMAGE_0_DS_TM_TX_TASK_THREAD_NUMBER;
    config.per_q_task.task1 = IMAGE_0_DS_TM_TX_TASK_THREAD_NUMBER;
    config.per_q_task.task2 = IMAGE_0_DS_TM_TX_TASK_THREAD_NUMBER;
    config.per_q_task.task3 = IMAGE_0_DS_TM_TX_TASK_THREAD_NUMBER;
    config.per_q_task.task4 = IMAGE_0_DS_TM_TX_TASK_THREAD_NUMBER;
    config.per_q_task.task5 = IMAGE_0_DS_TM_TX_TASK_THREAD_NUMBER;
    config.per_q_task.task6 = IMAGE_0_DS_TM_TX_TASK_THREAD_NUMBER;
    config.per_q_task.task7 = IMAGE_0_DS_TM_TX_TASK_THREAD_NUMBER;

    /* CHECK : how many queues to set (q2rnr)?*/
    config.lan_queue_cfg.queue_to_rnr = g_lan_queue_to_rnr;
    /* For Ethernet port working in MDU mode, PD FIFO size should be configured to 4 (and not 8). */
    config.lan_queue_cfg.pd_fifo_base = lan_pd_fifo_base;
    config.lan_queue_cfg.pd_fifo_size = g_lan_pd_size_base;
    config.lan_queue_cfg.fe_fifo_base = g_lan_fe_fifo_base;
    config.lan_queue_cfg.fe_fifo_size = g_lan_fe_size_base;
    config.lan_queue_cfg.fe_pd_fifo_base = g_lan_fe_pd_fifo_base;
    config.lan_queue_cfg.fe_pd_fifo_size = g_lan_fe_pd_size_base;
    /* why it says in the regge it is used for epon */
    config.lan_queue_cfg.pd_wkup_threshold = g_lan_pd_wkup_threshold;
    config.lan_queue_cfg.pd_bytes_threshold = g_lan_pd_bytes_threshold;
    /* pd_prefetch_byte_threshold feature is irrelevant in EMAC (since there is only one FIFO) */
    config.lan_queue_cfg.pd_bytes_threshold_en = 0; 
    config.lan_queue_cfg.pd_empty_threshold = 1;

    config.src_id.fpmsrc = BB_ID_FPM;
    config.src_id.sbpmsrc = BB_ID_SBPM;

    config.dma_cfg = &(g_bbh_tx_dma_profile->bbh_tx_dma_cfg[BBH_TX_ID_FIRST]);
    config.sdma_cfg = &(g_bbh_tx_sdma_profile->bbh_tx_sdma_cfg[BBH_TX_ID_FIRST]);
    config.ddr_cfg = &(g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[BBH_TX_ID_FIRST]);
    
    fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
    GET_ADDR_HIGH_LOW(config.base_addr_high.addr[0], config.base_addr_low.addr[0], fpm_base_phys_addr);

    config.base_addr_low.addr[1] = 0;
    config.base_addr_high.addr[1] = 0;

    rc = rc ? rc : drv_bbh_tx_configuration_set(BBH_TX_ID_FIRST, &config);
    max_on_the_fly_reads = (g_max_otf_reads[BBH_TX_ID_FIRST] > config.dma_cfg->descsize) ? config.dma_cfg->descsize : g_max_otf_reads[BBH_TX_ID_FIRST];
    rc = rc ? rc : ag_drv_bbh_tx_dma_max_otf_read_request_set(BBH_TX_ID_FIRST, max_on_the_fly_reads);
    max_on_the_fly_reads = (g_max_otf_reads[BBH_TX_ID_FIRST] > config.sdma_cfg->descsize) ? config.sdma_cfg->descsize : g_max_otf_reads[BBH_TX_ID_FIRST];
    rc = rc ? rc : ag_drv_bbh_tx_sdma_max_otf_read_request_set(BBH_TX_ID_FIRST, max_on_the_fly_reads);
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_dfifoctrl_set(BBH_TX_ID_FIRST, BBH_TX_DATA_FIFO_SRAM_SIZE, BBH_TX_DATA_FIFO_DDR_SIZE, BBH_TX_DATA_FIFO_SRAM_BASE);

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize bbh_tx driver");

    return rc;
}

static int bbh_tx_wan_init(void)
{
    bbh_tx_config config;
    uint8_t i, max_on_the_fly_reads;
    bdmf_phys_addr_t fpm_base_phys_addr;
    bdmf_error_t rc = BDMF_ERR_OK;
    pd_fifo_base_t ethwan_pd_fifo_base[TX_QEUEU_PAIRS] = {};
    pd_fifo_base_t dsl_pd_fifo_base[TX_QEUEU_PAIRS] = {}; 
    bbh_id_e bbh_id;
    uint16_t thread_number, tcont_addr, ptr_addr;

    /* TBD. Using same array for GPON / DSL and Ethernet LAN (bbh_tx_lan_init) / EThernet WAN
     * Is this correct?
     */

    /* init fifo base arrays */
    ethwan_pd_fifo_base[0].base0 = 0; 
    ethwan_pd_fifo_base[0].base1 = ethwan_pd_fifo_base[0].base0 + g_lan_pd_fifo_size[0].size0 + 1;

    for (i = 1; i < LAN_QUEUE_PAIRS; ++i)
    {
        ethwan_pd_fifo_base[i].base0 = ethwan_pd_fifo_base[i-1].base1 + g_lan_pd_fifo_size[i-1].size1 + 1;
        ethwan_pd_fifo_base[i].base1 = ethwan_pd_fifo_base[i].base0 + g_lan_pd_fifo_size[i].size0 + 1;
    }

    dsl_pd_fifo_base[0].base0 = 0;
    dsl_pd_fifo_base[0].base1 = dsl_pd_fifo_base[0].base0 + g_dsl_pd_fifo_size[0].size0 + 1;

    for (i = 1; i < TX_QEUEU_PAIRS; i++)
    {
        dsl_pd_fifo_base[i].base0 = dsl_pd_fifo_base[i-1].base1 + g_dsl_pd_fifo_size[i-1].size1 + 1;
        dsl_pd_fifo_base[i].base1 = dsl_pd_fifo_base[i].base0 + g_dsl_pd_fifo_size[i].size0 + 1;   
    }

    xrdp_memset(&config, 0, sizeof(bbh_tx_config));

    for (bbh_id = BBH_TX_ID_AE10; bbh_id < BBH_TX_ID_NUM; bbh_id++)
    {
        if (bbh_id == BBH_TX_ID_PON) continue;  /* Skip PON. It has its own */

        /* the following mac type doesn't seem to have any effect */
        config.mac_type = bbh_tx_id_to_mac_type[bbh_id];

        if (bbh_id == BBH_TX_ID_AE2P5)
        {
            thread_number = IMAGE_3_US_TM_WAN_AE2P5_THREAD_NUMBER;
            tcont_addr = IMAGE_3_US_TM_AE2P5_BBH_TX_QUEUE_ID_TABLE_ADDRESS;
            ptr_addr = IMAGE_3_US_TM_AE2P5_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS;
        }
        else if (bbh_id == BBH_TX_ID_AE10)
        {
            thread_number = IMAGE_3_US_TM_WAN_AE10_THREAD_NUMBER;
            tcont_addr = IMAGE_3_US_TM_AE10_BBH_TX_QUEUE_ID_TABLE_ADDRESS;
            ptr_addr = IMAGE_3_US_TM_AE10_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS;
            ag_drv_bbh_tx_lan_configurations_txthresh_set(bbh_id, BBH_TX_AE10_LAN_DDR_THRESHOLD, BBH_TX_AE10_LAN_SRAM_THRESHOLD);
        }
        else /* if (bbh_id == BBH_TX_ID_DSL) */
        {
            thread_number = IMAGE_3_US_TM_WAN_DSL_THREAD_NUMBER;
            tcont_addr = IMAGE_3_US_TM_DSL_BBH_TX_QUEUE_ID_TABLE_ADDRESS;
            ptr_addr = IMAGE_3_US_TM_DSL_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS;
        }
        /* cores which sending PDs */
        config.rnr_cfg[0].rnr_src_id  = get_runner_idx(us_tm_runner_image);
        config.rnr_cfg[0].tcont_addr  = tcont_addr >> 3;
        config.rnr_cfg[0].skb_addr    = 0;
        config.rnr_cfg[0].task_number = thread_number;
        config.rnr_cfg[0].ptr_addr    = ptr_addr >> 3;

        config.src_id.stsrnrsrc = get_runner_idx(us_tm_runner_image);

        /* priority for transmitting Q */
        rc = rc ? rc : ag_drv_bbh_tx_common_configurations_arb_cfg_set(bbh_id, 1);

        if (bbh_id == BBH_TX_ID_DSL)
        {
            config.wan_queue_cfg.pd_fifo_base = dsl_pd_fifo_base;
            config.wan_queue_cfg.pd_fifo_size = g_dsl_pd_fifo_size;
            config.wan_queue_cfg.pd_wkup_threshold = g_dsl_pd_wkup_threshold;

            config.wan_queue_cfg.queue_to_rnr = g_wan_queue_to_rnr;
            config.wan_queue_cfg.pd_bytes_threshold_en = 0; 
            config.wan_queue_cfg.pd_empty_threshold = 1;
        }
        else
        {
            config.lan_queue_cfg.pd_fifo_base = ethwan_pd_fifo_base;
            config.lan_queue_cfg.pd_fifo_size = g_lan_pd_fifo_size;
            config.lan_queue_cfg.fe_fifo_base = g_lan_fe_fifo_base;
            config.lan_queue_cfg.fe_fifo_size = g_lan_fe_size_base;
            config.lan_queue_cfg.fe_pd_fifo_base = g_lan_fe_pd_fifo_base;
            config.lan_queue_cfg.fe_pd_fifo_size = g_lan_fe_pd_size_base;
            config.lan_queue_cfg.pd_wkup_threshold = g_lan_pd_wkup_threshold;
            config.lan_queue_cfg.pd_bytes_threshold = g_lan_pd_bytes_threshold;

            config.lan_queue_cfg.queue_to_rnr = g_lan_queue_to_rnr;
            config.lan_queue_cfg.pd_bytes_threshold_en = 0; 
            config.lan_queue_cfg.pd_empty_threshold = 1;
        }

        /* enable per queue task number */
        config.per_q_task.task0 = thread_number;
        config.per_q_task.task1 = thread_number;
        config.per_q_task.task2 = thread_number;
        config.per_q_task.task3 = thread_number;
        config.per_q_task.task4 = thread_number;
        config.per_q_task.task5 = thread_number;
        config.per_q_task.task6 = thread_number;
        config.per_q_task.task7 = thread_number;

        /* configurations for reporting core (DBR\GHOST) */
#if 0   /* TODO! check if the following is needed for ETH/DSL BBH TX, or else they are using GPON's resource */
        config.msg_rnr_cfg[0].tcont_addr = IMAGE_0_REPORT_BBH_TX_QUEUE_ID_TABLE_ADDRESS >> 3;
        config.msg_rnr_cfg[0].ptr_addr = IMAGE_0_BBH_TX_EGRESS_REPORT_COUNTER_TABLE_ADDRESS >> 3;
        config.msg_rnr_cfg[0].task_number = IMAGE_0_US_TM_REPORTING_THREAD_NUMBER;
#endif
        config.src_id.fpmsrc = BB_ID_FPM;
        config.src_id.sbpmsrc = BB_ID_SBPM;
        config.src_id.msgrnrsrc = get_runner_idx(reporting_runner_image);

        config.dma_cfg = &(g_bbh_tx_dma_profile->bbh_tx_dma_cfg[bbh_id]);
        config.sdma_cfg = &(g_bbh_tx_sdma_profile->bbh_tx_sdma_cfg[bbh_id]);
        config.ddr_cfg = &(g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[bbh_id]);

        fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
        GET_ADDR_HIGH_LOW(config.base_addr_high.addr[0], config.base_addr_low.addr[0], fpm_base_phys_addr);

        rc = rc ? rc : drv_bbh_tx_configuration_set(bbh_id, &config);
        max_on_the_fly_reads = (g_max_otf_reads[bbh_id] > config.dma_cfg->descsize) ? config.dma_cfg->descsize : g_max_otf_reads[bbh_id];
        rc = rc ? rc : ag_drv_bbh_tx_dma_max_otf_read_request_set(bbh_id, max_on_the_fly_reads);
        max_on_the_fly_reads = (g_max_otf_reads[bbh_id] > config.sdma_cfg->descsize) ? config.sdma_cfg->descsize : g_max_otf_reads[bbh_id];
        rc = rc ? rc : ag_drv_bbh_tx_sdma_max_otf_read_request_set(bbh_id, max_on_the_fly_reads);
        rc = rc ? rc : ag_drv_bbh_tx_common_configurations_dfifoctrl_set(bbh_id, BBH_TX_DATA_FIFO_SRAM_SIZE, BBH_TX_DATA_FIFO_DDR_SIZE, BBH_TX_DATA_FIFO_SRAM_BASE);

        if (rc)
            BDMF_TRACE_ERR("Failed to initialize wantype in bbh_tx driver");
    }

    return rc;
}


#if defined(XRDP_EMULATION)
#define BBH_TX_BLOCK_COUNT BBH_TX_ID_NUM
#define BBH_TX_WAN_ID      BBH_ID_PON
#endif


static int bbh_tx_pon_init(int wantype)
{
    bbh_tx_config config;
    uint8_t i, max_on_the_fly_reads;
    bdmf_phys_addr_t fpm_base_phys_addr;
    bdmf_error_t rc = BDMF_ERR_OK;
    pd_fifo_base_t gpon_pd_fifo_base[TX_QEUEU_PAIRS] = {}; 

    gpon_pd_fifo_base[0].base0 = 0;
    gpon_pd_fifo_base[0].base1 = gpon_pd_fifo_base[0].base0 + g_gpon_pd_fifo_size[0].size0 + 1;

    for (i = 1; i < TX_QEUEU_PAIRS; i++)
    {
        gpon_pd_fifo_base[i].base0 = gpon_pd_fifo_base[i-1].base1 + g_gpon_pd_fifo_size[i-1].size1 + 1;
        gpon_pd_fifo_base[i].base1 = gpon_pd_fifo_base[i].base0 + g_gpon_pd_fifo_size[i].size0 + 1;   
    }

    xrdp_memset(&config, 0, sizeof(bbh_tx_config));

    config.mac_type = wantype;

    /* cores which sending PDs */
    config.rnr_cfg[0].rnr_src_id = get_runner_idx(us_tm_runner_image);
    config.rnr_cfg[0].tcont_addr = IMAGE_3_US_TM_PON_BBH_TX_QUEUE_ID_TABLE_ADDRESS >> 3;
    config.rnr_cfg[0].skb_addr = 0;
    config.rnr_cfg[0].task_number = IMAGE_3_US_TM_WAN_PON_THREAD_NUMBER;
    config.rnr_cfg[0].ptr_addr = IMAGE_3_US_TM_PON_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS >> 3;

    config.src_id.stsrnrsrc = get_runner_idx(us_tm_runner_image);

    /* priority for transmitting Q */
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_arb_cfg_set(BBH_TX_ID_PON, 1);

    config.wan_queue_cfg.pd_fifo_base = gpon_pd_fifo_base;
    config.wan_queue_cfg.pd_fifo_size = g_gpon_pd_fifo_size;
    config.wan_queue_cfg.pd_wkup_threshold = g_gpon_pd_wkup_threshold;

    config.wan_queue_cfg.queue_to_rnr = g_wan_queue_to_rnr;
    config.wan_queue_cfg.pd_bytes_threshold_en = 0; 
    config.wan_queue_cfg.pd_empty_threshold = 1;

    /* enable per queue task number */
    config.per_q_task.task0 = IMAGE_3_US_TM_WAN_PON_THREAD_NUMBER;
    config.per_q_task.task1 = IMAGE_3_US_TM_WAN_PON_THREAD_NUMBER;
    config.per_q_task.task2 = IMAGE_3_US_TM_WAN_PON_THREAD_NUMBER;
    config.per_q_task.task3 = IMAGE_3_US_TM_WAN_PON_THREAD_NUMBER;
    config.per_q_task.task4 = IMAGE_3_US_TM_WAN_PON_THREAD_NUMBER;
    config.per_q_task.task5 = IMAGE_3_US_TM_WAN_PON_THREAD_NUMBER;
    config.per_q_task.task6 = IMAGE_3_US_TM_WAN_PON_THREAD_NUMBER;
    config.per_q_task.task7 = IMAGE_3_US_TM_WAN_PON_THREAD_NUMBER;

    /* configurations for reporting core (DBR\GHOST) */
    config.msg_rnr_cfg[0].tcont_addr = IMAGE_0_REPORT_BBH_TX_QUEUE_ID_TABLE_ADDRESS >> 3;
    config.msg_rnr_cfg[0].ptr_addr = IMAGE_0_BBH_TX_EGRESS_REPORT_COUNTER_TABLE_ADDRESS >> 3;
    config.msg_rnr_cfg[0].task_number = IMAGE_0_DS_TM_REPORTING_THREAD_NUMBER;
    config.src_id.fpmsrc = BB_ID_FPM;
    config.src_id.sbpmsrc = BB_ID_SBPM;
    config.src_id.msgrnrsrc = get_runner_idx(reporting_runner_image);

    config.dma_cfg = &(g_bbh_tx_dma_profile->bbh_tx_dma_cfg[BBH_TX_ID_PON]);
    config.sdma_cfg = &(g_bbh_tx_sdma_profile->bbh_tx_sdma_cfg[BBH_TX_ID_PON]);
    config.ddr_cfg = &(g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[BBH_TX_ID_PON]);

    fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
    GET_ADDR_HIGH_LOW(config.base_addr_high.addr[0], config.base_addr_low.addr[0], fpm_base_phys_addr);

    rc = rc ? rc : drv_bbh_tx_configuration_set(BBH_TX_ID_PON, &config);
    max_on_the_fly_reads = (g_max_otf_reads[BBH_TX_ID_PON] > config.dma_cfg->descsize) ? config.dma_cfg->descsize : g_max_otf_reads[BBH_TX_ID_PON];
    rc = rc ? rc : ag_drv_bbh_tx_dma_max_otf_read_request_set(BBH_TX_ID_PON, max_on_the_fly_reads);
    max_on_the_fly_reads = (g_max_otf_reads[BBH_TX_ID_PON] > config.sdma_cfg->descsize) ? config.sdma_cfg->descsize : g_max_otf_reads[BBH_TX_ID_PON];
    rc = rc ? rc : ag_drv_bbh_tx_sdma_max_otf_read_request_set(BBH_TX_ID_PON, max_on_the_fly_reads);
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_dfifoctrl_set(BBH_TX_ID_PON, BBH_TX_DATA_FIFO_SRAM_SIZE, BBH_TX_DATA_FIFO_DDR_SIZE, BBH_TX_DATA_FIFO_SRAM_BASE);

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize wantype in bbh_tx driver");

    return rc;
}

static int dma_sdma_init(void)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint32_t dma_id, bbh_id, max_read_on_the_fly;
    uint32_t sdma_to_bbh_rx, dma_to_bbh_tx, sdma_to_bbh_tx;
    
    for (dma_id = DMA_ID_FIRST; dma_id < DMA_NUM; dma_id++)
    {
        max_read_on_the_fly = IS_SDMA(dma_id) ? SDMA_MAX_READ_ON_THE_FLY : DMA_MAX_READ_ON_THE_FLY;
        rc = rc ? rc : ag_drv_dma_config_max_otf_set(dma_id, max_read_on_the_fly);

        for (bbh_id = BBH_ID_FIRST; bbh_id < BBH_ID_NUM; bbh_id++)
        {
            if (IS_SDMA(dma_id))
            {
                sdma_to_bbh_rx = g_sdma_to_bbh_rx_x[dma_id - SDMA0_ID][bbh_id];
                if (sdma_to_bbh_rx <= BBH_ID_LAST)
                {
                    rc = rc ? rc : ag_drv_dma_config_num_of_writes_set(dma_id, bbh_id, g_bbh_rx_sdma_profile->bbh_rx_sdma_chunks_config[sdma_to_bbh_rx].sdma_chunks); 
                }
            }

            if (bbh_id < BBH_TX_BLOCK_COUNT) 
            {
                if (IS_DMA(dma_id))
                {
                    dma_to_bbh_tx = g_dma_to_bbh_tx_x[dma_id][bbh_id];
                    if (dma_to_bbh_tx <= BBH_TX_ID_LAST)
                    {
                        rc = rc ? rc : ag_drv_dma_config_num_of_reads_set(dma_id, bbh_id, g_bbh_tx_dma_profile->bbh_tx_dma_cfg[dma_to_bbh_tx].descsize);
                    }
                }
                else
                {
                    sdma_to_bbh_tx = g_sdma_to_bbh_tx_x[dma_id - SDMA0_ID][bbh_id];
                    if (sdma_to_bbh_tx <= BBH_TX_ID_LAST)
                    {
                        rc = rc ? rc : ag_drv_dma_config_num_of_reads_set(dma_id, bbh_id, g_bbh_tx_sdma_profile->bbh_tx_sdma_cfg[sdma_to_bbh_tx].descsize); 
                    }
                }
            }

            rc = rc ? rc : ag_drv_dma_config_u_thresh_set(dma_id, bbh_id, g_dma_urgent_threshold[dma_id][bbh_id].into_urgent_threshold, 
                g_dma_urgent_threshold[dma_id][bbh_id].out_of_urgent_threshold);
            rc = rc ? rc : ag_drv_dma_config_pri_set(dma_id, bbh_id, g_dma_strict_priority[dma_id][bbh_id].rx_side, 
                g_dma_strict_priority[dma_id][bbh_id].tx_side);
            rc = rc ? rc : ag_drv_dma_config_weight_set(dma_id, bbh_id, g_dma_rr_weight[dma_id][bbh_id].rx_side, g_dma_rr_weight[dma_id][bbh_id].tx_side);
            rc = rc ? rc : ag_drv_dma_config_periph_source_set(dma_id, bbh_id, g_dma_bb_source[dma_id][bbh_id].rx_side, g_dma_bb_source[dma_id][bbh_id].tx_side);

            if (rc)
                break;
        }
    }

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize dma_sdma driver\n");

    return rc;
}

static int rnr_frequency_set(uint16_t freq)
{
    uint8_t rnr_idx;
    int rc = BDMF_ERR_OK;

    for (rnr_idx = 0; rnr_idx <= RNR_LAST; rnr_idx++)
    {
        rc = rc ? rc : ag_drv_rnr_regs_rnr_freq_set(rnr_idx, (freq-1));
    }
    return rc;
}

extern uintptr_t xrdp_virt2phys(const ru_block_rec *ru_block, uint8_t addr_idx);

static int runner_init(int is_basic)
{
    rdd_init_params_t rdd_init_params = {0};
    rnr_dma_regs_cfg_t rnr_dma_cfg;
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t ubus_slave_idx, quad_idx, core_index;
    uint32_t addr_hi, addr_lo;
    uint16_t ddr_sop_offset0, ddr_sop_offset1;
    bdmf_phys_addr_t fpm_base_phys_addr;
#if !defined(RDP_SIM)
    bdmf_phys_addr_t psram_dma_base_phys_addr;
#endif
    RDD_HW_IPTV_CONFIGURATION_DTS iptv_hw_config = {};

    drv_rnr_cores_addr_init();
    drv_rnr_mem_init();
	drv_qm_update_queue_tables();
    drv_rnr_load_microcode();
    drv_rnr_load_prediction();

    /* Disable dma old flow control 
    DMA will not check read FIFO occupancy when issuing READ requests, 
    relying instead on DMA backpressure mechanism vs read dispatcher block */
    for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
         ag_drv_rnr_regs_cfg_gen_cfg_set(core_index, 1, 0);

    /* scheduler configuration */
    drv_rnr_set_sch_cfg();
#if 0
    /* image 0, core 0 */
    ag_drv_rnr_regs_cfg_sch_cfg_set(get_runner_idx(image_0_runner_image), DRV_RNR_4SP_12RR);
    /* image 1, core 3 */
    ag_drv_rnr_regs_cfg_sch_cfg_set(get_runner_idx(image_1_runner_image), DRV_RNR_8SP_8RR);
    /* image 2, core 2 */
    ag_drv_rnr_regs_cfg_sch_cfg_set(get_runner_idx(image_2_runner_image), DRV_RNR_2SP_14RR);
    /* image 3, core 1 */
    ag_drv_rnr_regs_cfg_sch_cfg_set(get_runner_idx(image_3_runner_image), DRV_RNR_4SP_12RR);    
    /* image 4, core 4 */
    ag_drv_rnr_regs_cfg_sch_cfg_set(get_runner_idx(image_4_runner_image), DRV_RNR_16RR);    
    /* image 5, core 5 */
    ag_drv_rnr_regs_cfg_sch_cfg_set(get_runner_idx(image_5_runner_image), DRV_RNR_8SP_8RR);
#endif
    /* image 0, core 0 */
    ag_drv_rnr_regs_cfg_sch_cfg_set(get_runner_idx(image_0_runner_image), DRV_RNR_4SP_12RR);
    /* image 1, core 3 */
    ag_drv_rnr_regs_cfg_sch_cfg_set(get_runner_idx(image_1_runner_image), DRV_RNR_16RR);
    /* image 2, core 2 */
    ag_drv_rnr_regs_cfg_sch_cfg_set(get_runner_idx(image_2_runner_image), DRV_RNR_16RR);
    /* image 3, core 1 */
    ag_drv_rnr_regs_cfg_sch_cfg_set(get_runner_idx(image_3_runner_image), DRV_RNR_4SP_12RR);    
    /* image 4, core 4 */
    ag_drv_rnr_regs_cfg_sch_cfg_set(get_runner_idx(image_4_runner_image), DRV_RNR_8SP_8RR);    
    /* image 5, core 5 */
    ag_drv_rnr_regs_cfg_sch_cfg_set(get_runner_idx(image_5_runner_image), DRV_RNR_16RR);

    fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
    GET_ADDR_HIGH_LOW(addr_hi, addr_lo, fpm_base_phys_addr);

    g_fpm_hw_cfg.fpm_base_low = addr_lo;
    g_fpm_hw_cfg.fpm_base_high = addr_hi;

    rnr_dma_cfg.ddr.dma_base = (addr_hi << 12) | (addr_lo >> 20);

    lookup_dma_bufsz_by_fpm_bufsz(&p_dpi_cfg->fpm_buf_size, &rnr_dma_cfg.ddr.dma_buf_size);

    rnr_dma_cfg.ddr.dma_static_offset = 0;
#if defined(RDP_SIM)
    rnr_dma_cfg.psram.dma_base = ((RU_BLK(PSRAM).addr[0] + RU_REG_OFFSET(PSRAM, MEMORY_DATA)) >> 20);
#else
    psram_dma_base_phys_addr = xrdp_virt2phys(&RU_BLK(PSRAM), 0) + RU_REG_OFFSET(PSRAM, MEMORY_DATA);
    rnr_dma_cfg.psram.dma_base = psram_dma_base_phys_addr >> 20;
#endif
    rnr_dma_cfg.psram.dma_buf_size = DMA_BUFSIZE_128;
    rnr_dma_cfg.psram.dma_static_offset = 0;

    rc = drv_rnr_dma_cfg(&rnr_dma_cfg);
    
    /* ToDo: fill rdd_init_params */

    rdd_init_params.is_basic = is_basic;
    if (!rdd_init_params.is_basic) 
    {
        rc =  ag_drv_qm_ddr_sop_offset_get(&ddr_sop_offset0, &ddr_sop_offset1);
        if (rc)
        {
            BDMF_TRACE_ERR("ddr_sop_offset not set\n");
            return rc;
        }
        else
        {
            iptv_hw_config.ddr_sop_offset0 = ddr_sop_offset0;
            iptv_hw_config.ddr_sop_offset1 = ddr_sop_offset1;
        }
        iptv_hw_config.fpm_base_token_size = p_dpi_cfg->fpm_buf_size;

        /* Assuming that hn size is the same for every BBH TX */
        iptv_hw_config.hn_size0 = g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[0].hnsize0;
        iptv_hw_config.hn_size1 = g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[1].hnsize1;
    }
    g_fpm_hw_cfg.fpm_token_size_asr_8 = p_dpi_cfg->fpm_buf_size >> 8;

#ifdef CONFIG_DHD_RUNNER
    /* offset should be int number of words */
    rdd_init_params.dhd_hw_config.ddr_sop_offset = (ddr_sop_offset0 + DHD_DATA_OFFSET + 7) & ~7;
#endif

    rc = rc ? rc : rdd_data_structures_init(&rdd_init_params, &iptv_hw_config);
    rc = rc ? rc : rnr_frequency_set(p_dpi_cfg->runner_freq);
    rc = rc ? rc : parser_init();

    if (rc)
        return rc;

    for (quad_idx = 0; quad_idx < NUM_OF_RNR_QUAD; quad_idx++) 
    {
       drv_rnr_quad_profiling_quad_init(quad_idx);

        /* change dynamic clock threshold in each quad */
        rc = rc ? rc : ag_drv_rnr_quad_general_config_dma_arb_cfg_set(quad_idx, 0, 0, 0, RNR_QUAD_DMA_ARB_CONJEST_THRESHOLD);

        /* extend PSRAM slave tokens to 8 */
        for (ubus_slave_idx = 16; ubus_slave_idx < 20; ubus_slave_idx++) 
        {
            rc = rc ? rc : ag_drv_rnr_quad_ext_flowctrl_config_token_val_set(quad_idx, ubus_slave_idx, 8);
        }

        if (rc)
            return rc;

        /* ToDo: should be moved to rdpa_filter file */
        // _rdpa_filter_ip_filter_en(quad_idx);

        //Todo:  remove when rdpa_iptv_ex is check in */
        // _rdpa_iptv_mac_filter_en(quad_idx);
    }
    rc = rc ? rc : rdp_drv_bkpt_init();

    return rc;
}

static inline void dispatcher_reorder_viq_init(dsptchr_config *cfg, dsptchr_cngs_params *congs_init, uint8_t bb_id, uint32_t target_address,
    bdmf_boolean dest, bdmf_boolean delayed, uint8_t viq_num, uint8_t guaranteed_limit, uint16_t common_max_limit, bdmf_boolean is_bbh_queue)
{
    cfg->dsptchr_viq_list[viq_num].wakeup_thrs = DSPTCHR_WAKEUP_THRS;
    cfg->dsptchr_viq_list[viq_num].bb_id = bb_id;
    cfg->dsptchr_viq_list[viq_num].bbh_target_address = target_address; /* for BBH - 2 is normal, 3 is exclusive */
    cfg->dsptchr_viq_list[viq_num].queue_dest = dest; /* 0-disp, 1-reor */
    cfg->dsptchr_viq_list[viq_num].delayed_queue = delayed; /* 0-non delayed, 1-delayed */
    cfg->dsptchr_viq_list[viq_num].common_max_limit = common_max_limit;
    cfg->dsptchr_viq_list[viq_num].guaranteed_max_limit = guaranteed_limit;
    cfg->dsptchr_viq_list[viq_num].coherency_en = is_bbh_queue;
    cfg->dsptchr_viq_list[viq_num].coherency_ctr = 0;
    xrdp_memcpy(&(cfg->dsptchr_viq_list[viq_num].ingress_cngs), congs_init, sizeof(dsptchr_cngs_params));
    xrdp_memcpy(&(cfg->dsptchr_viq_list[viq_num].egress_cngs), congs_init, sizeof(dsptchr_cngs_params));
    cfg->viq_num++;
}

static inline void dispatcher_reorder_rnr_group_init(dsptchr_config *cfg, uint8_t grp_idx, uint8_t core_index, uint32_t task_mask, uint32_t base_addr, uint32_t offset)
{
    cfg->dsptchr_rnr_group_list[grp_idx].tasks_mask.task_mask[core_index/2] |= (task_mask << ((core_index & 1) << 4));
    cfg->rg_available_tasks[grp_idx] += asserted_bits_count_get(task_mask);
    cfg->dsptchr_rnr_cfg[core_index].rnr_num = core_index;
    cfg->dsptchr_rnr_cfg[core_index].rnr_addr_cfg.base_add = base_addr;
    cfg->dsptchr_rnr_cfg[core_index].rnr_addr_cfg.offset_add = offset;
    cfg->rnr_disp_en_vector |= (1 << core_index);
}

static int dispatcher_reorder_init(void)
{
    dsptchr_config cfg;
    dsptchr_cngs_params congs_init;
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t bbh_id, core_index, i;
    uint16_t common_max_limit = DIS_REOR_LINKED_LIST_BUFFER_NUM 
       - (DSPTCHR_NORMAL_GUARANTEED_BUFFERS * (DISP_REOR_VIQ_LAST + 1 - 2)) /* TODO: will change when increasing number of G9991 VIQs */
       + DSPTCHR_NORMAL_GUARANTEED_BUFFERS * (BBH_ID_NUM - 1) /*Guaranteed for PON exclusive VIQ*/
       + (DSPTCHR_NORMAL_GUARANTEED_BUFFERS - DSPTCHR_EXCLUSIVE_WAN_GUARANTEED_BUFFERS)
       - DISP_REOR_VIQ_LAST + 2; /* Guaranteed for dummy buffers*/

    xrdp_memset(&cfg, 0, sizeof(dsptchr_config));
    xrdp_memset(&congs_init, 0, sizeof(dsptchr_cngs_params));
    congs_init.frst_lvl = DIS_REOR_LINKED_LIST_BUFFER_NUM - 1;
    congs_init.scnd_lvl = DIS_REOR_LINKED_LIST_BUFFER_NUM - 1;
    congs_init.hyst_thrs = (DIS_REOR_LINKED_LIST_BUFFER_NUM / 4) - 1;

    /* reset dispatcher credit for all queues */
    for (i = 0; i < DSPTCHR_VIRTUAL_QUEUE_NUM; ++i)
    {
        rc = rc ? rc : ag_drv_dsptchr_credit_cnt_set(i, 0);
    }

    /* configure all viq for bbh-rx LAN */
    for (bbh_id = BBH_ID_FIRST; bbh_id <= BBH_ID_NUM_LAN; bbh_id++)
    {
        /* normal viq for bbh-rx */
        dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_BBH_0 + (2 * bbh_id), dsptchr_viq_bbh_target_addr_normal, dsptchr_viq_dest_disp,
            dsptchr_viq_delayed, bbh_id, DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 1);

        /* exclusive viq for bbh-rx */
        dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_BBH_0 + (2 * bbh_id), dsptchr_viq_bbh_target_addr_excl, dsptchr_viq_dest_disp,
            dsptchr_viq_delayed, bbh_id + BBH_ID_NUM, DSPTCHR_EXCLUSIVE_WAN_GUARANTEED_BUFFERS * (bbh_id == BBH_ID_PON), common_max_limit, 1);
    }

    /* configure all viq for bbh-rx WANs */
    /* normal viq for bbh-rx 10G */
    dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_10G, dsptchr_viq_bbh_target_addr_normal, dsptchr_viq_dest_disp,
        dsptchr_viq_delayed, BBH_ID_AE10, DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 1);
    /* exclusive viq for bbh-rx 10G */
    dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_10G, dsptchr_viq_bbh_target_addr_excl, dsptchr_viq_dest_disp,
        dsptchr_viq_delayed, BBH_ID_AE10 + BBH_ID_NUM, DSPTCHR_EXCLUSIVE_WAN_GUARANTEED_BUFFERS * (bbh_id == BBH_ID_PON), common_max_limit, 1);
    
    /* normal viq for bbh-rx 2P5 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_2P5, dsptchr_viq_bbh_target_addr_normal, dsptchr_viq_dest_disp,
        dsptchr_viq_delayed, BBH_ID_AE2P5, DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 1);
    /* exclusive viq for bbh-rx 2P5 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_2P5, dsptchr_viq_bbh_target_addr_excl, dsptchr_viq_dest_disp,
        dsptchr_viq_delayed, BBH_ID_AE2P5 + BBH_ID_NUM, DSPTCHR_EXCLUSIVE_WAN_GUARANTEED_BUFFERS * (bbh_id == BBH_ID_PON), common_max_limit, 1);
    
    /* normal viq for bbh-rx DSL */
    dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_DSL, dsptchr_viq_bbh_target_addr_normal, dsptchr_viq_dest_disp,
        dsptchr_viq_delayed, BBH_ID_DSL, DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 1);
    /* exclusive viq for bbh-rx DSL */
    dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_DSL, dsptchr_viq_bbh_target_addr_excl, dsptchr_viq_dest_disp,
        dsptchr_viq_delayed, BBH_ID_DSL + BBH_ID_NUM, DSPTCHR_EXCLUSIVE_WAN_GUARANTEED_BUFFERS * (bbh_id == BBH_ID_PON), common_max_limit, 1);
    
    /* normal viq for bbh-rx GPON */
    dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_PON, dsptchr_viq_bbh_target_addr_normal, dsptchr_viq_dest_disp,
        dsptchr_viq_delayed, BBH_ID_PON, DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 1);
    /* exclusive viq for bbh-rx GPON */
    dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_PON, dsptchr_viq_bbh_target_addr_excl, dsptchr_viq_dest_disp,
        dsptchr_viq_delayed, BBH_ID_PON + BBH_ID_NUM, DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 1);

    /* VIQ for CPU_TX egress - 14 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(cpu_tx_runner_image), (IMAGE_2_CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 |
        IMAGE_2_IMAGE_2_CPU_TX_EGRESS_THREAD_NUMBER << 12), dsptchr_viq_dest_reor, dsptchr_viq_delayed, DISP_REOR_VIQ_CPU_TX_EGRESS,
        DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);

    /* VIQ for CPU_TX forward - 15 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(cpu_tx_runner_image), (IMAGE_2_CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 |
        IMAGE_2_IMAGE_2_CPU_TX_INGRESS_THREAD_NUMBER << 12), dsptchr_viq_dest_disp, dsptchr_viq_delayed, DISP_REOR_VIQ_CPU_TX_FORWARD,
        DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);

    /* VIQ for flush US - 16 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(us_tm_runner_image), (IMAGE_3_US_TM_FLUSH_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3) |
        (IMAGE_3_US_TM_FLUSH_THREAD_NUMBER << 12), dsptchr_viq_dest_reor, dsptchr_viq_delayed, DISP_REOR_VIQ_US_TM_FLUSH,
        DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);

    /* VIQ for flush DS - 17 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(ds_tm_runner_image), (IMAGE_0_DS_TM_FLUSH_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3) |
        (IMAGE_0_DS_TM_FLUSH_THREAD_NUMBER << 12), dsptchr_viq_dest_reor, dsptchr_viq_delayed, DISP_REOR_VIQ_DS_TM_FLUSH,
        DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);

    /* VIQ for TX Mirroring - Initialized for US - 18 */
    /* FIXME!! see if we want to do TX mirror for other WAN.. right now set it to 2P5 only */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(us_tm_runner_image), 
        ((IMAGE_3_TX_MIRRORING_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3) | (IMAGE_3_US_TM_WAN_AE2P5_THREAD_NUMBER << 12)),
         dsptchr_viq_dest_reor, dsptchr_viq_delayed, DISP_REOR_VIQ_TX_MIRRORING,
         DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);

    /* VIQ for CPU_RX_copy - 19 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(cpu_rx_runner_image),
        (IMAGE_1_CPU_RX_COPY_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3) | (IMAGE_1_IMAGE_1_CPU_RX_COPY_THREAD_NUMBER << 12),
        dsptchr_viq_dest_reor, dsptchr_viq_delayed, DISP_REOR_VIQ_CPU_RX_COPY,
        DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);

#ifdef CONFIG_DHD_RUNNER
    /* VIQ for DHD_RX_COMPLETE_0 - 20 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(dhd_complete_runner_image), (IMAGE_3_DHD_RX_COMPLETE_0_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 |
        IMAGE_3_US_TM_DHD_RX_COMPLETE_0_THREAD_NUMBER << 12), dsptchr_viq_dest_disp, dsptchr_viq_delayed, DISP_REOR_VIQ_DHD_RX_COMPLETE_0,
        DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);

    /* VIQ for DHD_RX_COMPLETE_1 - 21 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(dhd_complete_runner_image), (IMAGE_3_DHD_RX_COMPLETE_1_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 |
        IMAGE_3_US_TM_DHD_RX_COMPLETE_1_THREAD_NUMBER << 12), dsptchr_viq_dest_disp, dsptchr_viq_delayed, DISP_REOR_VIQ_DHD_RX_COMPLETE_1,
        DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);

    /* VIQ for DHD_RX_COMPLETE_2 - 22 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(dhd_complete_runner_image), (IMAGE_3_DHD_RX_COMPLETE_2_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 |
        IMAGE_3_US_TM_DHD_RX_COMPLETE_2_THREAD_NUMBER << 12), dsptchr_viq_dest_disp, dsptchr_viq_delayed, DISP_REOR_VIQ_DHD_RX_COMPLETE_2,
        DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);

    /* VIQ for DHD Mcast - 23 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(dhd_tx_post_runner_image), (IMAGE_4_DHD_MCAST_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 |
        IMAGE_4_IMAGE_4_DHD_MCAST_THREAD_NUMBER << 12), dsptchr_viq_dest_reor, dsptchr_viq_delayed, DISP_REOR_VIQ_DHD_MCAST,
        DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);
    
    /* VIQ for DHD_TX_COMPLETE_0 - 24 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(dhd_complete_runner_image), (IMAGE_3_DHD_TX_COMPLETE_0_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 |
        IMAGE_3_US_TM_DHD_TX_COMPLETE_0_THREAD_NUMBER << 12), dsptchr_viq_dest_reor, dsptchr_viq_delayed, DISP_REOR_VIQ_DHD_TX_COMPLETE_0,
        DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);

    /* VIQ for DHD_TX_COMPLETE_1 - 25 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(dhd_complete_runner_image), (IMAGE_3_DHD_TX_COMPLETE_1_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 |
        IMAGE_3_US_TM_DHD_TX_COMPLETE_1_THREAD_NUMBER << 12), dsptchr_viq_dest_reor, dsptchr_viq_delayed, DISP_REOR_VIQ_DHD_TX_COMPLETE_1,
        DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);

    /* VIQ for DHD_TX_COMPLETE_2 - 26 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(dhd_complete_runner_image), (IMAGE_3_DHD_TX_COMPLETE_2_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 |
        IMAGE_3_US_TM_DHD_TX_COMPLETE_2_THREAD_NUMBER << 12), dsptchr_viq_dest_reor, dsptchr_viq_delayed, DISP_REOR_VIQ_DHD_TX_COMPLETE_2,
        DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);
#endif
    
    /* VIQ for WAN TX DDR_READ DDR prefetch - 27 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(us_tm_runner_image), 
        ((IMAGE_3_WAN_TX_DDR_READ_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3) | (IMAGE_3_US_TM_WAN_TX_DDR_READ_THREAD_NUMBER << 12)),
         dsptchr_viq_dest_reor, dsptchr_viq_delayed, DISP_REOR_VIQ_WAN_TX_DDR_READ,
         DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);

    /* VIQ for WAN TX DDR_READ DDR prefetch - 28 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(us_tm_runner_image), 
        ((IMAGE_3_WAN_TX_PSRAM_WRITE_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3) | (IMAGE_3_US_TM_WAN_TX_PSRAM_WRITE_THREAD_NUMBER << 12)),
         dsptchr_viq_dest_reor, dsptchr_viq_delayed, DISP_REOR_VIQ_WAN_TX_PSRAM_WRITE,
         DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);

    /* configure all rnr groups */
    for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
    {
        /* setting group 0 - processing */
        if (IS_PROCESSING_RUNNER_IMAGE(core_index))
        {
            /* base address should stay 0 - packet buffer placed manually to 8*sizeof(RDD_PACKET_BUFFER_DTS) */
            dispatcher_reorder_rnr_group_init(&cfg, 0, core_index, 0xFF00, offsetof(RDD_PACKET_BUFFER_DTS, pd) >> 3,
                sizeof(RDD_PACKET_BUFFER_DTS) >> 3);
        }

        /* setting group 1 - wan direct */
        if (rdp_core_to_image_map[core_index] == wan_direct_runner_image)
        {
            dispatcher_reorder_rnr_group_init(&cfg, 1, core_index, (1<< IMAGE_1_IMAGE_1_WAN_DIRECT_THREAD_NUMBER), offsetof(RDD_PACKET_BUFFER_DTS, pd) >> 3, (sizeof(RDD_PACKET_BUFFER_DTS) >> 3));
        }
    }

    /* mapping viq to rnr group */
    cfg.rnr_grp_num = 2;
    cfg.dsptchr_rnr_group_list[0].queues_mask = 0x007F | /* all bbh viq except wan-bbh exclusive viq */
        (1 << DISP_REOR_VIQ_CPU_TX_FORWARD);
#ifdef CONFIG_DHD_RUNNER
    cfg.dsptchr_rnr_group_list[0].queues_mask |=  (1 << DISP_REOR_VIQ_DHD_RX_COMPLETE_0) | (1 << DISP_REOR_VIQ_DHD_RX_COMPLETE_1) | (1 << DISP_REOR_VIQ_DHD_RX_COMPLETE_2);
#endif        
    cfg.dsptchr_rnr_group_list[1].queues_mask = (0x1 << (BBH_ID_PON + BBH_ID_NUM)); /* wan-bbh exclusive viq */

    /* set up pools limits */
    cfg.pools_limits.grnted_pool_lmt = cfg.viq_num
        +  cfg.viq_num * DSPTCHR_NORMAL_GUARANTEED_BUFFERS  /* should be equal to sum of grntd credits in all active queues */
        - (DSPTCHR_NORMAL_GUARANTEED_BUFFERS * (BBH_ID_NUM - 1)  /*No Guaranteed for exclusive VIQs besides PON*/
        + (DSPTCHR_NORMAL_GUARANTEED_BUFFERS - DSPTCHR_EXCLUSIVE_WAN_GUARANTEED_BUFFERS)); /*Guaranteed for PON exclusive VIQ*/
    cfg.pools_limits.grnted_pool_size = cfg.pools_limits.grnted_pool_lmt;
    cfg.pools_limits.mcast_pool_lmt = 0;
    cfg.pools_limits.mcast_pool_size = cfg.pools_limits.mcast_pool_lmt;
    cfg.pools_limits.cmn_pool_lmt = DIS_REOR_LINKED_LIST_BUFFER_NUM
        - (cfg.pools_limits.grnted_pool_lmt + cfg.pools_limits.mcast_pool_lmt);
    cfg.pools_limits.cmn_pool_size = cfg.pools_limits.cmn_pool_lmt;
    cfg.pools_limits.rnr_pool_lmt = 0; /* runners multicast prefetch limit */
    cfg.pools_limits.rnr_pool_size = 0; /* current in processing for multicast */
    cfg.pools_limits.processing_pool_size = 0; /* current in processing */

    /* enable all viq except wan-bbh */
    cfg.queue_en_vec = DISP_REOR_XRDP_VIQ_EN(cfg.viq_num);

    rc = drv_dis_reor_queues_init();
    rc = rc ? rc : drv_dis_reor_tasks_to_rg_init();
    rc = rc ? rc : drv_dis_reor_free_linked_list_init();
    rc = rc ? rc : drv_dis_reor_cfg(&cfg);

    return rc;
}

static int dispatcher_reorder_wantype_init(void)
{
    uint32_t viq_en;
    bdmf_error_t rc;

    /* enable wan bbh viq */
    rc = ag_drv_dsptchr_reorder_cfg_vq_en_get(&viq_en);
    viq_en = DISP_REOR_WAN_BBH_VIQ_EN(viq_en);
    rc = rc ? rc : ag_drv_dsptchr_reorder_cfg_vq_en_set(viq_en);

    return rc;
}

static int natc_init(void)
{
    int rc = BDMF_ERR_OK;
    natc_config_t cfg;
    natc_ddr_cfg_natc_ddr_size ddr_size = {ddr_size_32k, ddr_size_32k, ddr_size_32k, ddr_size_32k, ddr_size_32k, ddr_size_32k, ddr_size_32k, ddr_size_32k};
    natc_ddr_cfg_total_len total_len = {total_len_144B, total_len_144B, total_len_144B, total_len_144B, total_len_144B, total_len_144B, total_len_144B, total_len_144B};

    xrdp_memset(&cfg, 0, sizeof(natc_config_t));
    cfg.ctrl_status.nat_hash_mode = hash_mode_crc32;
    cfg.ctrl_status.ddr_hash_mode = hash_mode_crc32_high;
    cfg.ctrl_status.multi_hash_limit = multi_hash_limit_def;

    cfg.ctrl_status.age_timer_tick = timer_tick_packet;
    cfg.ctrl_status.age_timer = age_timer_256_ticks;
    cfg.ctrl_status.cache_update_on_reg_ddr_lookup = 1;
    cfg.ctrl_status.ddr_enable = 1;
    cfg.ctrl_status.natc_enable = 1;

    cfg.tbl_num = 2;

    /* DS table */
    cfg.tbl_cntrl[tuple_lkp_ds_tbl].key_len = key_len_16B;
#if defined(USE_NATC_VAR_CONTEXT_LEN)
    cfg.tbl_cntrl[tuple_lkp_ds_tbl].var_context_len_en = 1;
#else
    cfg.tbl_cntrl[tuple_lkp_ds_tbl].var_context_len_en = 0;
#endif
    cfg.tbl_cfg[tuple_lkp_ds_tbl].mask = NATC_16BYTE_KEY_MASK;
    cfg.tbl_cfg[tuple_lkp_ds_tbl].key_len = NATC_TABLE_KEY_16B + (NATC_TABLE_KEY_16B * cfg.tbl_cntrl[tuple_lkp_ds_tbl].key_len);
    cfg.tbl_cfg[tuple_lkp_ds_tbl].key_tbl_size = ((((NATC_TABLE_BASE_SIZE_SIZE << ddr_size.ddr_size_tbl0) + ddr_8_bins_per_bucket) * cfg.tbl_cfg[tuple_lkp_ds_tbl].key_len) + 0x1f) & ~0x1f;
    cfg.tbl_cfg[tuple_lkp_ds_tbl].res_len = total_length_bytes[total_len_144B] - cfg.tbl_cfg[tuple_lkp_ds_tbl].key_len;
    cfg.tbl_cfg[tuple_lkp_ds_tbl].res_tbl_size = ((((NATC_TABLE_BASE_SIZE_SIZE << ddr_size.ddr_size_tbl0) + ddr_8_bins_per_bucket) * cfg.tbl_cfg[tuple_lkp_ds_tbl].res_len) + 0x1f) & ~0x1f;
    cfg.tbl_cfg[tuple_lkp_ds_tbl].tbl_entry_num = NATC_TABLE_BASE_SIZE_SIZE << ddr_size.ddr_size_tbl0;

#ifndef XRDP_EMULATION
    cfg.tbl_cfg[tuple_lkp_ds_tbl].vir_addr.key = p_dpi_cfg->rdp_ddr_rnr_tables_base_virt;
    cfg.tbl_cfg[tuple_lkp_ds_tbl].vir_addr.res = (void *)((uint8_t *)cfg.tbl_cfg[tuple_lkp_ds_tbl].vir_addr.key +
        cfg.tbl_cfg[tuple_lkp_ds_tbl].key_tbl_size);

    cfg.tbl_cfg[tuple_lkp_ds_tbl].phy_addr.key = RDD_RSV_VIRT_TO_PHYS(cfg.tbl_cfg[tuple_lkp_ds_tbl].vir_addr.key);
    cfg.tbl_cfg[tuple_lkp_ds_tbl].phy_addr.res = RDD_RSV_VIRT_TO_PHYS(cfg.tbl_cfg[tuple_lkp_ds_tbl].vir_addr.res);
#else
    cfg.tbl_cfg[tuple_lkp_ds_tbl].vir_addr.key = NATC_DDR_KEY_DS_BASE;
    cfg.tbl_cfg[tuple_lkp_ds_tbl].vir_addr.res = NATC_DDR_RES_DS_BASE;
    cfg.tbl_cfg[tuple_lkp_ds_tbl].phy_addr.key = NATC_DDR_KEY_DS_BASE;
    cfg.tbl_cfg[tuple_lkp_ds_tbl].phy_addr.res = NATC_DDR_RES_DS_BASE;
#endif

    /* US table */
    cfg.tbl_cntrl[tuple_lkp_us_tbl].key_len = key_len_16B;
#if defined(USE_NATC_VAR_CONTEXT_LEN)
    cfg.tbl_cntrl[tuple_lkp_us_tbl].var_context_len_en = 1;
#else
    cfg.tbl_cntrl[tuple_lkp_us_tbl].var_context_len_en = 0;
#endif
    cfg.tbl_cfg[tuple_lkp_us_tbl].mask = NATC_16BYTE_KEY_MASK;
    cfg.tbl_cfg[tuple_lkp_us_tbl].key_len = NATC_TABLE_KEY_16B + (NATC_TABLE_KEY_16B * cfg.tbl_cntrl[tuple_lkp_us_tbl].key_len);

    /* To compute the actual size of the table, add DDR_BINS_PER_BUCKET field to the table size selection;
       For instance, if DDR_BINS_PER_BUCKET is 7 (8 bins per bucket) and DDR_size is 2 (32k entries), the actual size of the table in DDR is
      (32*1024+7) multiply by total length (TOTAL_LEN) of key and context in bytes
       Extra 7 entries are used to store collided entries of the last entry  
       Align to 32b key and res sizes to optimal performance */

    cfg.tbl_cfg[tuple_lkp_us_tbl].key_tbl_size = ((((NATC_TABLE_BASE_SIZE_SIZE << ddr_size.ddr_size_tbl1) + ddr_8_bins_per_bucket) * cfg.tbl_cfg[tuple_lkp_us_tbl].key_len) + 0x1f) & ~0x1f;
    cfg.tbl_cfg[tuple_lkp_us_tbl].res_len = total_length_bytes[total_len_144B] - cfg.tbl_cfg[tuple_lkp_us_tbl].key_len;
    cfg.tbl_cfg[tuple_lkp_us_tbl].res_tbl_size = ((((NATC_TABLE_BASE_SIZE_SIZE << ddr_size.ddr_size_tbl1) + ddr_8_bins_per_bucket) * cfg.tbl_cfg[tuple_lkp_us_tbl].res_len) + 0x1f) & ~0x1f;
    cfg.tbl_cfg[tuple_lkp_us_tbl].tbl_entry_num = NATC_TABLE_BASE_SIZE_SIZE << ddr_size.ddr_size_tbl1;

#ifndef XRDP_EMULATION
    cfg.tbl_cfg[tuple_lkp_us_tbl].vir_addr.key = (void *)((uint8_t *)cfg.tbl_cfg[tuple_lkp_ds_tbl].vir_addr.res + 
                                                          cfg.tbl_cfg[tuple_lkp_ds_tbl].res_tbl_size);
    cfg.tbl_cfg[tuple_lkp_us_tbl].vir_addr.res = (void *)((uint8_t *)cfg.tbl_cfg[tuple_lkp_us_tbl].vir_addr.key +
        cfg.tbl_cfg[tuple_lkp_us_tbl].key_tbl_size);

    cfg.tbl_cfg[tuple_lkp_us_tbl].phy_addr.key = RDD_RSV_VIRT_TO_PHYS(cfg.tbl_cfg[tuple_lkp_us_tbl].vir_addr.key);
    cfg.tbl_cfg[tuple_lkp_us_tbl].phy_addr.res = RDD_RSV_VIRT_TO_PHYS(cfg.tbl_cfg[tuple_lkp_us_tbl].vir_addr.res);
#else
    cfg.tbl_cfg[tuple_lkp_us_tbl].vir_addr.key = NATC_DDR_KEY_US_BASE;
    cfg.tbl_cfg[tuple_lkp_us_tbl].vir_addr.res = NATC_DDR_RES_US_BASE;
    cfg.tbl_cfg[tuple_lkp_us_tbl].phy_addr.key = NATC_DDR_KEY_US_BASE;
    cfg.tbl_cfg[tuple_lkp_us_tbl].phy_addr.res = NATC_DDR_RES_US_BASE;
#endif
    
    /* marks end of NATC memory */
    natc_mem_end_addr = (void *)((uint8_t *)cfg.tbl_cfg[tuple_lkp_us_tbl].vir_addr.res +
                                                          cfg.tbl_cfg[tuple_lkp_us_tbl].res_tbl_size);
    natc_mem_available_size = p_dpi_cfg->rnr_tables_buf_size - (int)(natc_mem_end_addr - p_dpi_cfg->rdp_ddr_rnr_tables_base_virt);
    rc = rc ? rc : ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_0_set(ddr_8_bins_per_bucket, ddr_8_bins_per_bucket, ddr_8_bins_per_bucket, ddr_8_bins_per_bucket);
    rc = rc ? rc : ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_1_set(ddr_8_bins_per_bucket, ddr_8_bins_per_bucket, ddr_8_bins_per_bucket, ddr_8_bins_per_bucket);
    rc = rc ? rc : ag_drv_natc_ddr_cfg_natc_ddr_size_set(&ddr_size);
    rc = rc ? rc : ag_drv_natc_ddr_cfg_total_len_set(&total_len);
    rc = rc ? rc : ag_drv_natc_key_mask_tbl_key_mask_set(tuple_lkp_ds_tbl, NATC_16BYTE_KEY_MASK);
    rc = rc ? rc : ag_drv_natc_key_mask_tbl_key_mask_set(tuple_lkp_us_tbl, NATC_16BYTE_KEY_MASK);

    return (rc ? rc : drv_natc_init(&cfg));
}

static int qm_wantype_init(int wantype)
{
    bdmf_error_t rc = BDMF_ERR_OK;
#if 1  /* TODO! check if we need the following? */
    qm_epon_overhead_ctrl epon_counter_cfg = {};

    /* 10G EPON counters init */
    epon_counter_cfg.fec_ipg_length = QM_EPON_IPG_DEFAULT_LENGTH;
    if (wantype == MAC_TYPE_XEPON)
        epon_counter_cfg.epon_line_rate = 1;
    rc = rc ? rc : ag_drv_qm_epon_overhead_ctrl_set(&epon_counter_cfg);
#endif

    return rc;
}

static int dqm_init(void)
{
    bdmf_error_t rc;
    bdmf_phys_addr_t fpm_base_phys_addr;
    uint32_t addr_hi, addr_lo, buf_size;

    fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
    GET_ADDR_HIGH_LOW(addr_hi, addr_lo, fpm_base_phys_addr);
    buf_size = g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[BBH_TX_ID_FIRST].bufsize;

    rc = ag_drv_dqm_fpm_addr_set(xrdp_virt2phys(&RU_BLK(FPM), 0) + RU_REG_OFFSET(FPM, POOL1_ALLOC_DEALLOC));
    rc = rc ? rc : ag_drv_dqm_buf_base_set((addr_hi << 24) | (addr_lo >> 8));
    rc = rc ? rc : ag_drv_dqm_buf_size_set(buf_size);

    return rc;
}

static int qm_init(void)
{
    uint32_t addr_hi, addr_lo;
    bdmf_phys_addr_t fpm_base_phys_addr;
    bdmf_error_t rc = BDMF_ERR_OK;
    qm_rnr_group_cfg rnr_group_cfg = {};
    qm_q_context queue_cfg = {};
    uint32_t idx = 0;
    qm_drop_counters_ctrl drop_ctrl = {};

    qm_init_cfg init_cfg =
    {
        .is_close_agg_disable = 1,     /* TRUE=close aggregation timers are disable */
        .is_counters_read_clear = 1,   /* TRUE=read-clear counter access mode */
        .is_drop_counters_enable = 1,  /* TRUE=drop counters enable. FALSE=max occupancy holder */
        .ddr_sop_offset = {0, 18}      /* DDR SoP offsets profile0 - everything except multicast , profile1 - for multicast, */
    };

    fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
    GET_ADDR_HIGH_LOW(addr_hi, addr_lo, fpm_base_phys_addr);

    init_cfg.fpm_base = (addr_hi << 24) | (addr_lo >> 8); /* in 256B resolution */
    init_cfg.fpm_buf_size = g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[BBH_TX_ID_FIRST].bufsize;

    rc = rc ? rc : drv_qm_init(&init_cfg);
	rc = rc ? rc : drv_qm_system_init(p_dpi_cfg);

    rc = rc ? rc : ag_drv_qm_qm_pd_cong_ctrl_set(DEF_QM_PD_CONGESTION_CONTROL_THRESHOLD);

    ag_drv_qm_copy_decision_profile_set(QM_COPY_DEC_PROFILE_LOW, QM_COPY_DEC_MAX_QUEUE_OCCUPANCY , 0);
    ag_drv_qm_copy_decision_profile_set(QM_COPY_DEC_PROFILE_HIGH, QM_COPY_DEC_MAX_QUEUE_OCCUPANCY , 1);

    /* Initializing DQM before any of QM's queues */
    rc = rc ? rc : dqm_init();

    /* setup 1 queue */
    queue_cfg.wred_profile = 1;                 /* WRED profile */
    queue_cfg.copy_dec_profile = QM_COPY_DEC_PROFILE_LOW; /* Copy decision profile */
    queue_cfg.fpm_ug = FPM_US_UG;               /* FPM UG */
    queue_cfg.ddr_copy_disable = 0;             /* TRUE=never copy to DDR */
    queue_cfg.copy_to_ddr = US_DDR_COPY_ENABLE; /* TRUE=Force copy to DDR */
    /* FIXME!! temporarily disable aggregation */
    queue_cfg.aggregation_disable = 1;          /* TRUE=Disable aggregation */
    queue_cfg.exclusive_priority = 0;           /* TRUE=exclusive priority */
    queue_cfg.q_802_1ae = 0;                    /* TRUE=802.1AE */
    queue_cfg.sci = 0;                          /* TRUE=SCI */
    queue_cfg.fec_enable = 0;                   /* TRUE=enable FEC */
    queue_cfg.res_profile = 0;                  /* Profile=0 : no minimum buffer reservation */

    /* Initialize US queues */
    for (idx = drv_qm_get_us_start(); idx <= drv_qm_get_us_end(); idx++)
    {
        rc = rc ? rc : drv_qm_queue_config(idx, &queue_cfg);
        rc = rc ? rc : drv_qm_queue_enable(idx);
    }
    
    /* Initialize DS queues */
    queue_cfg.fpm_ug = FPM_DS_UG;                       /* FPM UG */
    queue_cfg.copy_to_ddr = DS_DDR_COPY_ENABLE;         /* TRUE=Force copy to DDR */
    queue_cfg.aggregation_disable = DS_AGG_DISABLE;     /* TRUE=Disable aggregation */
    for (idx = drv_qm_get_ds_start(); idx <= drv_qm_get_ds_end(); idx++)
    {
        rc = rc ? rc : drv_qm_queue_config(idx, &queue_cfg);
        rc = rc ? rc : drv_qm_queue_enable(idx);
    }
    /* Init DS drop queue */
    queue_cfg.wred_profile = QM_WRED_PROFILE_DROP_ALL;
    queue_cfg.aggregation_disable = 1;   /* TRUE=Disable aggregation */
    rc = rc ? rc : drv_qm_queue_config(QM_QUEUE_DROP, &queue_cfg);
    rc = rc ? rc : drv_qm_queue_enable(QM_QUEUE_DROP);

    queue_cfg.fpm_ug = FPM_ALL_PASS_UG;       /* FPM UG */
    queue_cfg.wred_profile = 0;
    queue_cfg.copy_to_ddr = 0;           /* TRUE=Force copy to DDR */
    queue_cfg.aggregation_disable = 1;   /* TRUE=Disable aggregation */
    for (idx = QM_QUEUE_MAX_DYNAMIC_QUANTITY; idx < QM_NUM_QUEUES; idx++)
    {
        if ((idx == QM_QUEUE_DROP) || (idx == QM_QUEUE_CPU_RX)  || (idx == QM_QUEUE_CPU_RX_COPY_NORMAL) || (idx == QM_QUEUE_CPU_RX_COPY_EXCLUSIVE))
            continue;
#ifdef CONFIG_DHD_RUNNER
       if ((idx == QM_QUEUE_DHD_TX_POST_0)  || (idx == QM_QUEUE_DHD_TX_POST_1)  || (idx == QM_QUEUE_DHD_TX_POST_2))
	        continue;
#endif
        rc = rc ? rc : drv_qm_queue_config(idx, &queue_cfg);
        rc = rc ? rc : drv_qm_queue_enable(idx);
    }

#ifdef CONFIG_DHD_RUNNER
    /* configure params for DHD_TX_POST queue - DHD CPU queues are standard and configured above*/
    queue_cfg.copy_to_ddr = 1;
    queue_cfg.fpm_ug = FPM_WLAN_UG;
    queue_cfg.exclusive_priority = 1;

    for (idx = 0; idx < 3; idx++)
    {
        rc = rc ? rc : drv_qm_queue_config(QM_QUEUE_DHD_TX_POST_0 + idx*2, &queue_cfg);
        rc = rc ? rc : drv_qm_queue_enable(QM_QUEUE_DHD_TX_POST_0 + idx*2);
    }

    /* configure params for DHD_MCAST queue */
    queue_cfg.copy_to_ddr = 0;
    queue_cfg.exclusive_priority = 0;
    queue_cfg.ddr_copy_disable = 1;
    rc = rc ? rc : drv_qm_queue_config(QM_QUEUE_DHD_MCAST, &queue_cfg);
    rc = rc ? rc : drv_qm_queue_enable(QM_QUEUE_DHD_MCAST);

    queue_cfg.fpm_ug = FPM_ALL_PASS_UG;       /* FPM UG */
    queue_cfg.copy_to_ddr = 0;           /* TRUE=Force copy to DDR */
    queue_cfg.ddr_copy_disable = 0;
    /* -------------------------------- */
#endif

    /* configure WRED profile for CPU_RX queue */
    queue_cfg.wred_profile = QM_WRED_PROFILE_CPU_RX;

    rc = rc ? rc : drv_qm_queue_config(QM_QUEUE_CPU_RX, &queue_cfg);
    rc = rc ? rc : drv_qm_queue_enable(QM_QUEUE_CPU_RX);

    queue_cfg.wred_profile = QM_WRED_PROFILE_CPU_RX;
    queue_cfg.copy_dec_profile = QM_COPY_DEC_PROFILE_HIGH;
    queue_cfg.ddr_copy_disable = 1;
    
    rc = rc ? rc : drv_qm_queue_config(QM_QUEUE_CPU_RX_COPY_NORMAL, &queue_cfg);
    rc = rc ? rc : drv_qm_queue_enable(QM_QUEUE_CPU_RX_COPY_NORMAL);
    
    queue_cfg.wred_profile = QM_WRED_PROFILE_CPU_RX;
    queue_cfg.ddr_copy_disable = 0;
    
    rc = rc ? rc : drv_qm_queue_config(QM_QUEUE_CPU_RX_COPY_EXCLUSIVE, &queue_cfg);
    rc = rc ? rc : drv_qm_queue_enable(QM_QUEUE_CPU_RX_COPY_EXCLUSIVE);

    /* ds group 0 */
    rnr_group_cfg.start_queue = drv_qm_get_ds_start();
    rnr_group_cfg.end_queue = drv_qm_get_ds_end();
    rnr_group_cfg.pd_fifo_base = (IMAGE_0_DS_TM_PD_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
    rnr_group_cfg.upd_fifo_base = (IMAGE_0_UPDATE_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
    rnr_group_cfg.rnr_bb_id = get_runner_idx(ds_tm_runner_image);
    rnr_group_cfg.rnr_task = IMAGE_0_DS_TM_UPDATE_FIFO_THREAD_NUMBER;
    rnr_group_cfg.rnr_enable = 1;
    rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_0, &rnr_group_cfg);

    /* us - group 1 (update fifo task) */
    rnr_group_cfg.start_queue = drv_qm_get_us_start();
    rnr_group_cfg.end_queue = drv_qm_get_us_end();
    rnr_group_cfg.pd_fifo_base = (IMAGE_3_US_TM_PD_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.rnr_task = IMAGE_3_US_TM_UPDATE_FIFO_THREAD_NUMBER;
    rnr_group_cfg.upd_fifo_base = (IMAGE_3_UPDATE_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
    rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
    rnr_group_cfg.rnr_bb_id = get_runner_idx(us_tm_runner_image);
    rnr_group_cfg.rnr_enable = 1;
    rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_1, &rnr_group_cfg);

    /* cpu - group 3 (cpu_rx fifo task) */
    rnr_group_cfg.start_queue = QM_QUEUE_CPU_RX; /* single queue for CPU RX */
    rnr_group_cfg.end_queue = QM_QUEUE_CPU_RX; 
    rnr_group_cfg.pd_fifo_base = (IMAGE_1_PD_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.rnr_task = IMAGE_1_IMAGE_1_CPU_RX_THREAD_NUMBER;
    rnr_group_cfg.upd_fifo_base = (IMAGE_1_UPDATE_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
    rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
    rnr_group_cfg.rnr_bb_id = get_runner_idx(cpu_rx_runner_image);
    rnr_group_cfg.rnr_enable = 1;
    rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_3, &rnr_group_cfg);

    /* cpu - group 4 (cpu_tx egress fifo task) */
    rnr_group_cfg.start_queue = QM_QUEUE_CPU_TX_EGRESS; /* single queue for CPU TX egress*/
    rnr_group_cfg.end_queue = QM_QUEUE_CPU_TX_EGRESS; 
    rnr_group_cfg.pd_fifo_base = (IMAGE_2_CPU_TX_EGRESS_PD_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.rnr_task = IMAGE_2_IMAGE_2_CPU_TX_EGRESS_THREAD_NUMBER;
    rnr_group_cfg.upd_fifo_base = (IMAGE_2_CPU_TX_EGRESS_UPDATE_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
    rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
    rnr_group_cfg.rnr_bb_id = get_runner_idx(cpu_tx_runner_image);
    rnr_group_cfg.rnr_enable = 1;
    rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_4, &rnr_group_cfg);

    /* cpu - group 5 (cpu_tx forward fifo task) */
    rnr_group_cfg.start_queue = QM_QUEUE_CPU_TX_INGRESS; /* single queue for CPU TX ingress */
    rnr_group_cfg.end_queue = QM_QUEUE_CPU_TX_INGRESS; 
    rnr_group_cfg.pd_fifo_base = (IMAGE_2_CPU_TX_INGRESS_PD_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.rnr_task = IMAGE_2_IMAGE_2_CPU_TX_INGRESS_THREAD_NUMBER;
    rnr_group_cfg.upd_fifo_base = (IMAGE_2_CPU_TX_INGRESS_UPDATE_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
    rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
    rnr_group_cfg.rnr_bb_id = get_runner_idx(cpu_tx_runner_image);
    rnr_group_cfg.rnr_enable = 1;
    rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_5, &rnr_group_cfg);

    /* cpu - group 6 (cpu_rx_copy fifo task) */
    rnr_group_cfg.start_queue = QM_QUEUE_CPU_RX_COPY_EXCLUSIVE; /* 2 queues for CPU RX COPY*/
    rnr_group_cfg.end_queue = QM_QUEUE_CPU_RX_COPY_NORMAL; 
    rnr_group_cfg.pd_fifo_base = (IMAGE_1_CPU_RX_COPY_PD_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.rnr_task = IMAGE_1_IMAGE_1_CPU_RX_COPY_THREAD_NUMBER;
    rnr_group_cfg.upd_fifo_base = (IMAGE_1_CPU_RX_COPY_UPDATE_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
    rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
    rnr_group_cfg.rnr_bb_id = get_runner_idx(cpu_rx_runner_image);
    rnr_group_cfg.rnr_enable = 1;
    rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_6, &rnr_group_cfg);


#ifdef CONFIG_DHD_RUNNER
    /* dhd tx post - group 7 (dhd tx post and cpu tx post fifo task) */
    rnr_group_cfg.start_queue = QM_QUEUE_DHD_CPU_TX_POST_0; /* DHD CPU and TX group */
    rnr_group_cfg.end_queue = QM_QUEUE_DHD_TX_POST_2;
    rnr_group_cfg.pd_fifo_base = (IMAGE_4_DHD_TX_POST_PD_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.rnr_task = IMAGE_4_IMAGE_4_DHD_TX_POST_UPDATE_FIFO_THREAD_NUMBER;
    rnr_group_cfg.upd_fifo_base = (IMAGE_4_DHD_TX_POST_UPDATE_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
    rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
    rnr_group_cfg.rnr_bb_id = get_runner_idx(dhd_tx_post_runner_image);
    rnr_group_cfg.rnr_enable = 1;
    rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_7, &rnr_group_cfg);

#ifdef UNDEF
    /* enable DHD mcast on B0, as not enough runner groups */
    rnr_group_cfg.start_queue = QM_QUEUE_DHD_MCAST; /* single queue for DHD Mcast for all 3 radios */
    rnr_group_cfg.end_queue = QM_QUEUE_DHD_MCAST;
    rnr_group_cfg.pd_fifo_base = (IMAGE_3_DHD_MCAST_PD_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.rnr_task = IMAGE_3_US_TM_DHD_MCAST_UPDATE_FIFO_THREAD_NUMBER;
    rnr_group_cfg.upd_fifo_base = (IMAGE_3_DHD_MCAST_UPDATE_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
    rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;    
    rnr_group_cfg.rnr_bb_id = get_runner_idx(dhd_tx_post_runner_image);
    rnr_group_cfg.rnr_enable = 1;
    rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_8, &rnr_group_cfg);        
#endif
        
#endif

    /* Cancel drop qm counters accumulation in HW */
    /* counters accumulation is done in SW */    
    rc = rc ? rc : ag_drv_qm_drop_counters_ctrl_get(&drop_ctrl);
    drop_ctrl.read_clear_bytes = 1;
    drop_ctrl.read_clear_pkts = 1;
    rc = rc ? rc : ag_drv_qm_drop_counters_ctrl_set(&drop_ctrl);


    return rc;
}



static int unimac_init(void)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t idx;
    uint16_t max_pkt_size;
    uint16_t rxfifo_congestion_threshold;

    for (idx = 0; idx < NUM_OF_UNIMAC; idx++) 
    {
        rc = rc ? rc : ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_get(idx, &max_pkt_size, &rxfifo_congestion_threshold);
        rc = rc ? rc : ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_set(idx, p_dpi_cfg->mtu_size, rxfifo_congestion_threshold);
        rc = rc ? rc : ag_drv_unimac_rdp_rx_max_pkt_size_set(idx, p_dpi_cfg->mtu_size);
        rc = rc ? rc : ag_drv_unimac_rdp_frm_len_set(idx, p_dpi_cfg->mtu_size);
    }
    
    return rc;
}


#ifdef XRDP_EMULATION
static int clock_gate_en() {
  
  bdmf_error_t rc = BDMF_ERR_OK;
  uint8_t i;
  bac_if_bacif_block_bacif_configurations_clk_gate_cntrl bacif_clk_gate_cntrl;
  bbh_rx_general_configuration_clk_gate_cntrl bbh_rx_clk_gate_cntrl;
  bbh_tx_common_configurations_clk_gate_cntrl bbh_tx_clk_gate_cntrl;
  dsptchr_reorder_cfg_clk_gate_cntrl dsp_clk_gate_cntrl;
  dma_config_clk_gate_cntrl dma_clk_gate_cntrl;
  psram_configurations_clk_gate_cntrl psram_clk_gate_cntrl;
  sbpm_regs_sbpm_clk_gate_cntrl sbpm_clk_gate_cntrl;
  qm_clk_gate_clk_gate_cntrl qm_clk_gate_cntrl;

  for (i = 0; i < BACIF_ID_NUM; i++) {
	// NATC doesn't support clock gating
	if (i != BACIF_NATC_ID) {
	  rc = rc ? rc : ag_drv_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl_get(i, &bacif_clk_gate_cntrl);
	  bacif_clk_gate_cntrl.bypass_clk_gate = 0;
	  bacif_clk_gate_cntrl.timer_val = 200;
	  rc = rc ? rc : ag_drv_bac_if_bacif_block_bacif_configurations_clk_gate_cntrl_set(i, &bacif_clk_gate_cntrl);
	}
  }

  for (i = 0; i < BBH_ID_NUM; i++) {
    rc = rc ? rc :  ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_get(i, &bbh_rx_clk_gate_cntrl);
	bbh_rx_clk_gate_cntrl.bypass_clk_gate = 0;
	bbh_rx_clk_gate_cntrl.timer_val = 200;
    rc = rc ? rc :  ag_drv_bbh_rx_general_configuration_clk_gate_cntrl_set(i, &bbh_rx_clk_gate_cntrl);
  }

  for (i = 0; i < BBH_TX_ID_NUM; i++) {
    rc = rc ? rc :  ag_drv_bbh_tx_common_configurations_clk_gate_cntrl_get(i, &bbh_tx_clk_gate_cntrl);
	bbh_tx_clk_gate_cntrl.bypass_clk_gate = 0;
	bbh_tx_clk_gate_cntrl.timer_val = 200;
	rc = rc ? rc :  ag_drv_bbh_tx_common_configurations_clk_gate_cntrl_set(i, &bbh_tx_clk_gate_cntrl);
  }

  rc = rc ? rc :  ag_drv_dsptchr_reorder_cfg_clk_gate_cntrl_get(&dsp_clk_gate_cntrl);
  dsp_clk_gate_cntrl.bypass_clk_gate = 0;
  dsp_clk_gate_cntrl.timer_val = 200;
  rc = rc ? rc :  ag_drv_dsptchr_reorder_cfg_clk_gate_cntrl_set(&dsp_clk_gate_cntrl);

  for (i = 0; i < DMA_NUM; i++) {
	rc = rc ? rc :  ag_drv_dma_config_clk_gate_cntrl_get(i, &dma_clk_gate_cntrl);
	dma_clk_gate_cntrl.bypass_clk_gate = 0;
	dma_clk_gate_cntrl.timer_val = 200;
	rc = rc ? rc :  ag_drv_dma_config_clk_gate_cntrl_set(i, &dma_clk_gate_cntrl);
  }

  rc = rc ? rc :  ag_drv_psram_configurations_clk_gate_cntrl_get(&psram_clk_gate_cntrl);
  psram_clk_gate_cntrl.bypass_clk_gate = 0;
  psram_clk_gate_cntrl.timer_val = 200;
  rc = rc ? rc :  ag_drv_psram_configurations_clk_gate_cntrl_set(&psram_clk_gate_cntrl);

  rc = rc ? rc :  ag_drv_sbpm_regs_sbpm_clk_gate_cntrl_get(&sbpm_clk_gate_cntrl);
  sbpm_clk_gate_cntrl.bypass_clk_gate = 0;
  sbpm_clk_gate_cntrl.timer_val = 200;
  rc = rc ? rc :  ag_drv_sbpm_regs_sbpm_clk_gate_cntrl_set(&sbpm_clk_gate_cntrl);

  rc = rc ? rc :  ag_drv_qm_clk_gate_clk_gate_cntrl_get(&qm_clk_gate_cntrl);
  qm_clk_gate_cntrl.bypass_clk_gate = 0;
  qm_clk_gate_cntrl.timer_val = 200;
  rc = rc ? rc :  ag_drv_qm_clk_gate_clk_gate_cntrl_set(&qm_clk_gate_cntrl);

  return rc;
}
#endif


static int xlif_init(rdpa_emac wan_emac, int enable)
{
    // Channel 0 is AE10
    // Channel 1 is AE2p5
    int disable = !enable;

    if (rdpa_emac_to_bb_id_rx[wan_emac] == BB_ID_RX_10G)
    {
        ag_drv_xlif_tx_if_if_enable_set(0, disable,  disable);
        ag_drv_xlif_tx_if_urun_port_enable_set(0, 0);
        ag_drv_xlif_tx_if_tx_threshold_set(0, 0x0C);
        ag_drv_xlif_rx_if_if_dis_set(0, disable);   // Channel 0 disabled?
    } 
    else if (rdpa_emac_to_bb_id_rx[wan_emac] == BB_ID_RX_2P5)
    {
        ag_drv_xlif_tx_if_if_enable_set(1, disable, disable);
        ag_drv_xlif_tx_if_urun_port_enable_set(1, 0);
        ag_drv_xlif_tx_if_tx_threshold_set(1, 0x0C);
        ag_drv_xlif_rx_if_if_dis_set(1, disable);  // Channel 1 !disabled
    }
    
    // Channel 2 & 3 are not connected in 63158
    ag_drv_xlif_rx_if_if_dis_set(2, 1);  // Channel 2 disabled
    ag_drv_xlif_rx_if_if_dis_set(3, 1);  // Channel 3 disabled

    return BDMF_ERR_OK;
}


static void rdp_block_enable(void)
{
    unimac_rdp_cmd cmd;
    uint8_t idx;
    qm_enable_ctrl qm_enable = {
        .fpm_prefetch_enable = 1,
        .reorder_credit_enable = 1,
        .dqm_pop_enable = 1,
        .rmt_fixed_arb_enable = 1,
        .dqm_push_fixed_arb_enable = 1
    };

    dsptchr_reorder_cfg_dsptchr_reordr_cfg reorder = {1, 0, 1, 0, 1};
    
    bdmf_error_t rc = BDMF_ERR_OK;
#ifndef RDP_SIM
    bdmf_boolean rdy = 0;
    bdmf_boolean bsy = 0;
    uint16_t init_offset = 0;
    uint16_t base_addr = 0;

    /* enable BBH_RX */
    while ((rc == BDMF_ERR_OK) && (rdy == 0))
    {
        rc = ag_drv_sbpm_regs_init_free_list_get(&base_addr, &init_offset, &bsy, &rdy);
        xrdp_usleep(FPM_POLL_SLEEP_TIME);
    }
#endif

    for (idx = BBH_ID_FIRST; idx < BBH_ID_NUM; idx++)
    {
        /* skip BBH_ID_PON, as it needs configure ploams before enabling BBH_RX */
        if ((idx == BBH_ID_PON) || (idx == BBH_ID_DSL))
            continue;

        rc = rc ? rc : ag_drv_bbh_rx_general_configuration_enable_set(idx, 1, 1);
    }

    for (idx = 0; idx < UNIMAC_ID_NUM; idx++)
    {
        rc = rc ? rc : ag_drv_unimac_rdp_cmd_get(idx, &cmd);
        cmd.rx_ena = 1;
        cmd.tx_ena = 1;
        cmd.cntl_frm_ena = 1;
        cmd.runt_filter_dis = 1;
        rc = rc ? rc : ag_drv_unimac_rdp_cmd_set(idx, &cmd);
    }

    /* enable DISP_REOR */
    rc = rc ? rc : ag_drv_dsptchr_reorder_cfg_dsptchr_reordr_cfg_set(&reorder);

    /* enable QM */
    rc = rc ? rc : ag_drv_qm_enable_ctrl_set(&qm_enable);
    rc = rc ? rc : ag_drv_ubus_mstr_en_set(0, 1);

    /* enable TCAM */
    rc = rc ? rc : ag_drv_tcam_op_set(TCAM_CMD_INVALIDATE);

    /* enable RNR */
    for (idx = 0; idx <= RNR_LAST; idx++)
        rc = rc ? rc : ag_drv_rnr_regs_rnr_enable_set(idx, 1);


    if (rc)
        BDMF_TRACE_ERR("Failed to enable rdp blocks\n");
}

/* Note that wan_bbh is actually rdd_mac_type */
uint32_t data_path_init_fiber(int wan_bbh)
{
    int rc;

    /* RUNNER mactype init, 63158 only support GPON as fiber type */
    BUG_ON(wan_bbh == MAC_TYPE_EMAC);
    BUG_ON(wan_bbh == MAC_TYPE_EPON);
    BUG_ON(wan_bbh == MAC_TYPE_XEPON);
    BUG_ON(wan_bbh == MAC_TYPE_XGPON);

    /* used for direct mode - XGPON ignore ploam CRC */
    rdd_mac_type_cfg(wan_bbh);

    /* BBH-RX mactype init */
    rc = bbh_rx_pon_init(wan_bbh);

    /* BBH-TX mactype init */
    rc = rc ? rc : bbh_tx_pon_init(wan_bbh);

    /* DISPATCHER mactype init */
    rc = rc ? rc : dispatcher_reorder_wantype_init();

    /* QM mactype init */
    rc = rc ? rc : qm_wantype_init(wan_bbh);

#if 0 /* FIXME!! we don't need the following */
    rc = rc ? rc : rdd_ag_us_tm_bb_destination_table_set(BB_ID_TX_PON);
#endif

    /* configure bbh-tx fifo size */
    rc = rc ? rc : rdd_ag_us_tm_pon_bbh_tx_fifo_size_set(BBH_TX_GPON_PD_FIFO_SIZE_0_7);

    /* Enable ghost\dbr reporting */
    rdd_ghost_reporting_mac_type_init(wan_bbh);
    rc = rc ? rc : rdd_ghost_reporting_timer_set();

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize drivers for wantype %d", wan_bbh);
    return rc;
}

uint32_t data_path_init_gbe(rdpa_emac wan_emac)
{
    int rc = 0;

#if 0 /* FIXME!! we don't need the following */
    /* RUNNER mactype init */
    rc = rdd_ag_us_tm_bb_destination_table_set(rdpa_emac_to_bb_id_tx[wan_emac]);
#endif

    /* configure bbh-tx fifo size */
    rc = rc ? rc : rdd_ag_us_tm_eth_bbh_tx_fifo_size_set(BBH_TX_DS_PD_FIFO_SIZE_0);

    xlif_init(wan_emac, 1);
    
    if (rc)
        BDMF_TRACE_ERR("Failed to initialize drivers for GBE, wan_emac %d", wan_emac);

    return rc;
}

uint32_t data_path_init_dsl(int wan_bbh)
{
    int rc;

    /* BBH-RX mactype init */
    rc = bbh_rx_dsl_init(wan_bbh);

#if 0 /* FIXME!! we don't need the following */
    /* RUNNER mactype init */
    rc = rc ? rc : rdd_ag_us_tm_bb_destination_table_set(BB_ID_TX_DSL);
#endif

    /* FIXME!! TODO!! if PD_FIFO#0-7 and PD_FIFO#8-15 have different sizes,
     * it will cause issue!!! */
    /* configure bbh-tx fifo size */
    rc = rc ? rc : rdd_ag_us_tm_dsl_bbh_tx_fifo_size_set(BBH_TX_DSL_PD_FIFO_SIZE_0_7);

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize drivers for wantype %d", wan_bbh);

    return rc;
}

static int _data_path_init(dpi_params_t *dpi_params, int is_basic)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    p_dpi_cfg = dpi_params;

    bdmf_trace("INFO: %s#%d: Start.\n", __FUNCTION__, __LINE__);
#if (defined(RDP_SIM) && (!defined(XRDP_EMULATION)))
    if (rdd_sim_alloc_segments())
        return BDMF_ERR_INTERNAL;
#endif
    ubus_bridge_init();

    bbh_rx_profiles_init();
    bbh_tx_profiles_init();

    bdmf_trace("INFO: %s#%d: Driver init.\n", __FUNCTION__, __LINE__);

    rc = rc ? rc : fpm_init();
    bdmf_trace("%s  %d  %d\n", __FUNCTION__, __LINE__, rc);

    rc = rc ? rc : qm_init();
    bdmf_trace("%s  %d  %d\n", __FUNCTION__, __LINE__, rc);

    rc = rc ? rc : runner_init(is_basic);
    bdmf_trace("%s  %d  %d\n", __FUNCTION__, __LINE__, rc);

    rc = rc ? rc : sbpm_init();
    bdmf_trace("%s  %d  %d\n", __FUNCTION__, __LINE__, rc);

    rc = rc ? rc : bbh_rx_init();
    bdmf_trace("%s  %d  %d\n", __FUNCTION__, __LINE__, rc);

    rc = rc ? rc : bbh_tx_lan_init();
    bdmf_trace("%s  %d  %d\n", __FUNCTION__, __LINE__, rc);

    rc = rc ? rc : bbh_tx_wan_init();
    bdmf_trace("%s  %d  %d\n", __FUNCTION__, __LINE__, rc);

    rc = rc ? rc : cnpl_init(dpi_params->is_gateway, dpi_params->vlan_stats_enable);
    bdmf_trace("%s  %d  %d\n", __FUNCTION__, __LINE__, rc);

    rc = rc ? rc : dma_sdma_init();
    bdmf_trace("%s  %d  %d\n", __FUNCTION__, __LINE__, rc);

    rc = rc ? rc : dispatcher_reorder_init();
    bdmf_trace("%s  %d  %d\n", __FUNCTION__, __LINE__, rc);

    rc = rc ? rc : natc_init();
    bdmf_trace("%s  %d  %d\n", __FUNCTION__, __LINE__, rc);

    rc = rc ? rc : hash_init();
    bdmf_trace("%s  %d  %d\n", __FUNCTION__, __LINE__, rc);

    rc = rc ? rc : unimac_init();
    bdmf_trace("%s  %d  %d\n", __FUNCTION__, __LINE__, rc);

    xlif_init(rdpa_emac4, 0);
    xlif_init(rdpa_emac5, 0);

#ifdef XRDP_EMULATION
    // RAL - We should probably enable this for power savings when the blocks are idle.
    // However, we should have a stable system before enabling, and then carefuly check that
    // a system with clock gating enabled behaves as expected after enabled.
    rc = rc ? rc : clock_gate_en();
#endif    

    bdmf_trace("%s  %d  %d\n", __FUNCTION__, __LINE__, rc);

    if (rc)
    {
        bdmf_trace("Error: %s#%d: rc = %d, End.\n", __FUNCTION__, __LINE__, rc);
        return rc;
    }

    bdmf_trace("INFO: %s#%d: Enable Accelerators.\n", __FUNCTION__, __LINE__);
    rdp_block_enable();
    bdmf_trace("INFO: %s#%d: End.\n", __FUNCTION__, __LINE__);

    return rc;
}

int data_path_init(dpi_params_t *dpi_params)
{
    return _data_path_init(dpi_params, 0);
}

int data_path_init_basic(dpi_params_t *dpi_params)
{
    int rc;

    /* Basic data path */
    rc = _data_path_init(dpi_params, 1);

    if (rc)
        bdmf_trace("%s failed, error %d\n", __FUNCTION__, rc);

    return rc;
}


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

#include "lib_types.h"
#include "lib_malloc.h"
#include "lib_printf.h"
#include "lib_string.h"
#define xrdp_memset lib_memset
#define xrdp_memcpy lib_memcpy
extern void cfe_usleep(int);
#define xrdp_alloc(_a) KMALLOC(_a, 32)
#define xrdp_usleep(_a) cfe_usleep(_a)
#define BDMF_TRACE_ERR xprintf
#define bdmf_ioremap(_a, _b) _a

#include "rdd.h"
#include "rdd_defs.h"
#include "data_path_init.h"
#include "rdd_init.h"
#include "rdd_cpu_rx.h"
#include "rdp_platform.h"

#include "rdp_drv_sbpm.h"
#include "xrdp_drv_psram_ag.h"
#include "xrdp_drv_fpm_ag.h"
#include "rdp_common.h"
#include "rdp_drv_dma.h"
#include "rdp_drv_rnr.h"

#include "rdp_drv_dis_reor.h"
#include "rdp_drv_bbh_tx.h"
#include "rdp_drv_fpm.h"
#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_ubus_mstr_ag.h"
#include "xrdp_drv_ubus_slv_ag.h"
#include "rdp_drv_qm.h"
#include "bdmf_data_types.h"


#ifndef RDP_SIM
#include "bcm_misc_hw_init.h"
#include "bcm_map_part.h"
#endif

dpi_params_t *p_dpi_cfg;

extern uint32_t total_length_bytes[];

RDD_FPM_GLOBAL_CFG_DTS g_fpm_hw_cfg;
bbh_rx_sdma_profile_t *g_bbh_rx_sdma_profile;
bbh_tx_dma_profile_t *g_bbh_tx_dma_profile;
bbh_tx_sdma_profile_t *g_bbh_tx_sdma_profile;
bbh_tx_ddr_profile_t *g_bbh_tx_ddr_profile;
pd_wkup_threshold_t g_lan_pd_wkup_threshold[2] = { {0, 0}, {0, 0} };
pd_fifo_size_t g_lan_pd_fifo_size[LAN_QUEUE_PAIRS] = {
          {BBH_TX_DS_PD_FIFO_SIZE_0, BBH_TX_DS_PD_FIFO_SIZE_1}, {BBH_TX_DS_PD_FIFO_SIZE_0, BBH_TX_DS_PD_FIFO_SIZE_1},
          {BBH_TX_DS_PD_FIFO_SIZE_0, BBH_TX_DS_PD_FIFO_SIZE_1}, {BBH_TX_DS_PD_FIFO_SIZE_0, BBH_TX_DS_PD_FIFO_SIZE_1} };

queue_to_rnr_t g_bbhtx_lan_queue_profile[BBH_TX_NUM_OF_LAN_QUEUES_PAIRS] = { {0,0}, {0,0}, {0,0}, {0,0} };
queue_to_rnr_t g_bbhtx_qm_queue_profile[BBH_TX_NUM_OF_LAN_QUEUES_PAIRS] = { {1,0}, {0,0}, {0,0}, {0,0} }; //1 is for indicating that this bbh lan queue is for QM not runner
queue_to_rnr_t g_lan_queue_to_rnr[BBH_TX_NUM_OF_LAN_QUEUES_PAIRS] = { {RNR0,RNR0}, {RNR0,RNR0}, {RNR0,RNR0}, {RNR0,RNR0} };
queue_to_rnr_t g_bbhtx_lan_qm_q[LAN_QUEUE_PAIRS] = { {0,0}, {0,0}, {0,0}, {0,0} };
queue_to_rnr_t g_qm_lan_qm_q[LAN_QUEUE_PAIRS] = { {1,0}, {0,0}, {0,0}, {0,0} };            //1 is for indicating that this bbh lan queue is for QM not runner

uint8_t g_bbh_rx_buffers_num[BBH_ID_NUM] = {4, 4, 4, 4, 4, 12, 32};
uint8_t g_bbh_rx_buffers_num_index[DMA_NUM][PERIPHERALS_PER_DMA] =
    {{0, 1, 2, 3, 4, 5, 0, 0}, {6, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}};


urgent_threhold_t g_dma_urgent_threshold[DMA_NUM][PERIPHERALS_PER_DMA] =
    {{{SDMA_U_THRESH_IN_BBH_LAN_VALUE, SDMA_U_THRESH_OUT_BBH_LAN_VALUE},
      {SDMA_U_THRESH_IN_BBH_LAN_VALUE, SDMA_U_THRESH_OUT_BBH_LAN_VALUE},
      {SDMA_U_THRESH_IN_BBH_LAN_VALUE, SDMA_U_THRESH_OUT_BBH_LAN_VALUE},
      {SDMA_U_THRESH_IN_BBH_LAN_VALUE, SDMA_U_THRESH_OUT_BBH_LAN_VALUE},
      {SDMA_U_THRESH_IN_BBH_LAN_VALUE, SDMA_U_THRESH_OUT_BBH_LAN_VALUE},
      {SDMA_U_THRESH_IN_BBH_LAN_VALUE, SDMA_U_THRESH_OUT_BBH_LAN_VALUE},
      {0, 0},
      {0, 0}},

     {{SDMA_U_THRESH_IN_BBH_10G_VALUE, SDMA_U_THRESH_OUT_BBH_10G_VALUE},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0}},

     {{0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0}}};

bb_source_t g_dma_bb_source[DMA_NUM][PERIPHERALS_PER_DMA] =  /* DMA0, DMA1, DMA_COPY   each entry is {rx,  tx} */
    {{{BB_ID_RX_BBH_0, BB_ID_TX_LAN},
      {BB_ID_RX_BBH_1, BB_ID_TX_LAN},
      {BB_ID_RX_BBH_2, 0},
      {BB_ID_RX_BBH_3, 0},
      {BB_ID_RX_BBH_4, 0},
      {BB_ID_RX_BBH_5, 0},
      {0, 0},
      {0, 0}},

     {{BB_ID_RX_PON, BB_ID_TX_PON_ETH_PD},
      {0, BB_ID_TX_PON_ETH_PD},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0}},

     {{0, BB_ID_TX_COPY},
      {0, BB_ID_TX_COPY},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0}}};



strict_priority_t g_dma_strict_priority[DMA_NUM][PERIPHERALS_PER_DMA] =
    {{{SDMA_STRICT_PRI_RX_BBH_LAN_VALUE, DMA_STRICT_PRI_TX_BBH_LAN_VALUE},
      {SDMA_STRICT_PRI_RX_BBH_LAN_VALUE, SDMA_STRICT_PRI_TX_BBH_LAN_VALUE},
      {SDMA_STRICT_PRI_RX_BBH_LAN_VALUE, 0},
      {SDMA_STRICT_PRI_RX_BBH_LAN_VALUE, 0},
      {SDMA_STRICT_PRI_RX_BBH_LAN_VALUE, 0},
      {SDMA_STRICT_PRI_RX_BBH_LAN_VALUE, 0},
      {0, 0},
      {0, 0}},

     {{SDMA_STRICT_PRI_RX_BBH_10G_VALUE, DMA_STRICT_PRI_TX_BBH_LAN_1_VALUE},
      {0, SDMA_STRICT_PRI_TX_BBH_LAN_1_VALUE},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0}},

     {{0, DMA_STRICT_PRI_TX_BBH_COPY_VALUE},
      {0, SDMA_STRICT_PRI_TX_BBH_COPY_VALUE},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0}}};

rr_weight_t g_dma_rr_weight[DMA_NUM][PERIPHERALS_PER_DMA] =
    {{{SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE, DMA_RR_WEIGHT_TX_BBH_LAN_VALUE},
      {SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE, SDMA_RR_WEIGHT_TX_BBH_LAN_VALUE},
      {SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE, 0},
      {SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE, 0},
      {SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE, 0},
      {SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE, 0},
      {0, 0},
      {0, 0}},

     {{SDMA_RR_WEIGHT_RX_BBH_10G_VALUE, DMA_RR_WEIGHT_TX_BBH_LAN_1_VALUE},
      {0, SDMA_RR_WEIGHT_TX_BBH_LAN_1_VALUE},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0}},

     {{0, DMA_RR_WEIGHT_TX_BBH_COPY_VALUE},
      {0, SDMA_RR_WEIGHT_TX_BBH_COPY_VALUE},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0},
      {0, 0}}};

// DMA
uint8_t g_rx_buffers_offset[DMA_TARGET_NUM][BBH_TX_ID_NUM] = {{0, 0, 32}, {16, 16, 48}};
uint8_t g_bbh_tx_buffers_num[DMA_TARGET_NUM][BBH_TX_ID_NUM] = {{16, 16, 16}, {16, 16, 16}};


uint8_t g_bbhrx_to_dma[DMA_NUM][PERIPHERALS_PER_DMA] =
    {{1, 1, 1, 1, 1, 1, 0, 0}, {1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}};
uint8_t g_bbhtx_to_dma[DMA_NUM][PERIPHERALS_PER_DMA] =
    {{1, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0, 0, 0}};

pd_bytes_threshold_t g_lan_pd_bytes_threshold[BBH_TX_NUM_OF_LAN_QUEUES_PAIRS] = { {BBH_TX_FIFO_BYTES_THRESHOLD_LAN,BBH_TX_FIFO_BYTES_THRESHOLD_LAN}, {BBH_TX_FIFO_BYTES_THRESHOLD_LAN,BBH_TX_FIFO_BYTES_THRESHOLD_LAN}, {BBH_TX_FIFO_BYTES_THRESHOLD_LAN,BBH_TX_FIFO_BYTES_THRESHOLD_LAN}, {BBH_TX_FIFO_BYTES_THRESHOLD_LAN,BBH_TX_FIFO_BYTES_THRESHOLD_LAN} };
pd_bytes_threshold_t g_wan_pd_bytes_threshold_1g[1] = { {BBH_TX_FIFO_BYTES_THRESHOLD_WAN_1G, BBH_TX_FIFO_BYTES_THRESHOLD_WAN_1G} };
pd_bytes_threshold_t g_wan_pd_bytes_threshold_10g[1] = { {BBH_TX_FIFO_BYTES_THRESHOLD_WAN_10G, BBH_TX_FIFO_BYTES_THRESHOLD_WAN_10G} };
queue_to_rnr_t g_lan_queue_profile[BBH_TX_NUM_OF_LAN_QUEUES_PAIRS] = { {0,0}, {0,0}, {0,1}, {0,0} };
queue_to_rnr_t g_lan_qm_q[BBH_TX_NUM_OF_LAN_QUEUES_PAIRS] = { {0,0}, {0,0}, {0,1}, {0,0} }; //1 is for indicating that this bbh lan queue is for QM not runner
queue_to_rnr_t g_wan_queue_profile[TX_QEUEU_PAIRS] = {
          {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
          {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}};
queue_to_rnr_t g_wan_qm_q[TX_QEUEU_PAIRS] = {
          {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0},
          {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}};
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

pd_fifo_size_t g_lan_pd_size_base[1] = {{BBH_TX_DS_PD_FIFO_SIZE_0, BBH_TX_DS_PD_FIFO_SIZE_1}};

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

pd_fifo_size_t g_epon_pd_fifo_size[TX_QEUEU_PAIRS] = {
          {BBH_TX_EPON_PD_FIFO_SIZE_0_7, BBH_TX_EPON_PD_FIFO_SIZE_0_7}, {BBH_TX_EPON_PD_FIFO_SIZE_0_7, BBH_TX_EPON_PD_FIFO_SIZE_0_7},
          {BBH_TX_EPON_PD_FIFO_SIZE_0_7, BBH_TX_EPON_PD_FIFO_SIZE_0_7}, {BBH_TX_EPON_PD_FIFO_SIZE_0_7, BBH_TX_EPON_PD_FIFO_SIZE_0_7},
          {BBH_TX_EPON_PD_FIFO_SIZE_8_15, BBH_TX_EPON_PD_FIFO_SIZE_8_15}, {BBH_TX_EPON_PD_FIFO_SIZE_8_15, BBH_TX_EPON_PD_FIFO_SIZE_8_15},
          {BBH_TX_EPON_PD_FIFO_SIZE_8_15, BBH_TX_EPON_PD_FIFO_SIZE_8_15}, {BBH_TX_EPON_PD_FIFO_SIZE_8_15, BBH_TX_EPON_PD_FIFO_SIZE_8_15},
          {BBH_TX_EPON_PD_FIFO_SIZE_16_23, BBH_TX_EPON_PD_FIFO_SIZE_16_23}, {BBH_TX_EPON_PD_FIFO_SIZE_16_23, BBH_TX_EPON_PD_FIFO_SIZE_16_23},
          {BBH_TX_EPON_PD_FIFO_SIZE_16_23, BBH_TX_EPON_PD_FIFO_SIZE_16_23}, {BBH_TX_EPON_PD_FIFO_SIZE_16_23, BBH_TX_EPON_PD_FIFO_SIZE_16_23},
          {BBH_TX_EPON_PD_FIFO_SIZE_24_31, BBH_TX_EPON_PD_FIFO_SIZE_24_31}, {BBH_TX_EPON_PD_FIFO_SIZE_24_31, BBH_TX_EPON_PD_FIFO_SIZE_24_31}, 
          {BBH_TX_EPON_PD_FIFO_SIZE_24_31, BBH_TX_EPON_PD_FIFO_SIZE_24_31}, {BBH_TX_EPON_PD_FIFO_SIZE_24_31, BBH_TX_EPON_PD_FIFO_SIZE_24_31},
          {BBH_TX_EPON_PD_FIFO_SIZE_32_39, BBH_TX_EPON_PD_FIFO_SIZE_32_39}, {BBH_TX_EPON_PD_FIFO_SIZE_32_39, BBH_TX_EPON_PD_FIFO_SIZE_32_39}, 
          {BBH_TX_EPON_PD_FIFO_SIZE_32_39, BBH_TX_EPON_PD_FIFO_SIZE_32_39}, {BBH_TX_EPON_PD_FIFO_SIZE_32_39, BBH_TX_EPON_PD_FIFO_SIZE_32_39} };

pd_wkup_threshold_t g_epon_pd_wkup_threshold[TX_QEUEU_PAIRS] = {
          {BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1, BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1}, {BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1, BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1},
          {BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1, BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1}, {BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1, BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1},
          {BBH_TX_EPON_PD_FIFO_SIZE_8_15 - 1, BBH_TX_EPON_PD_FIFO_SIZE_8_15 - 1}, {BBH_TX_EPON_PD_FIFO_SIZE_8_15 - 1, BBH_TX_EPON_PD_FIFO_SIZE_8_15 - 1},
          {BBH_TX_EPON_PD_FIFO_SIZE_8_15 - 1, BBH_TX_EPON_PD_FIFO_SIZE_8_15 - 1}, {BBH_TX_EPON_PD_FIFO_SIZE_8_15 - 1, BBH_TX_EPON_PD_FIFO_SIZE_8_15 - 1},
          {BBH_TX_EPON_PD_FIFO_SIZE_16_23 - 1, BBH_TX_EPON_PD_FIFO_SIZE_16_23 - 1}, {BBH_TX_EPON_PD_FIFO_SIZE_16_23 - 1, BBH_TX_EPON_PD_FIFO_SIZE_16_23 - 1},
          {BBH_TX_EPON_PD_FIFO_SIZE_16_23 - 1, BBH_TX_EPON_PD_FIFO_SIZE_16_23 - 1}, {BBH_TX_EPON_PD_FIFO_SIZE_16_23 - 1, BBH_TX_EPON_PD_FIFO_SIZE_16_23 - 1},
          {BBH_TX_EPON_PD_FIFO_SIZE_24_31 - 1, BBH_TX_EPON_PD_FIFO_SIZE_24_31 - 1}, {BBH_TX_EPON_PD_FIFO_SIZE_24_31 - 1, BBH_TX_EPON_PD_FIFO_SIZE_24_31 - 1}, 
          {BBH_TX_EPON_PD_FIFO_SIZE_24_31 - 1, BBH_TX_EPON_PD_FIFO_SIZE_24_31 - 1}, {BBH_TX_EPON_PD_FIFO_SIZE_24_31 - 1, BBH_TX_EPON_PD_FIFO_SIZE_24_31 - 1}, 
          {BBH_TX_EPON_PD_FIFO_SIZE_32_39 - 1, BBH_TX_EPON_PD_FIFO_SIZE_32_39 - 1}, {BBH_TX_EPON_PD_FIFO_SIZE_32_39 - 1, BBH_TX_EPON_PD_FIFO_SIZE_32_39 - 1}, 
          {BBH_TX_EPON_PD_FIFO_SIZE_32_39 - 1, BBH_TX_EPON_PD_FIFO_SIZE_32_39 - 1}, {BBH_TX_EPON_PD_FIFO_SIZE_32_39 - 1, BBH_TX_EPON_PD_FIFO_SIZE_32_39 - 1} };

uint8_t g_max_otf_reads[BBH_ID_NUM] = 
         {MAX_OTF_READ_REQUEST_DEFAULT_DMA0, MAX_OTF_READ_REQUEST_DEFAULT_DMA0,
          MAX_OTF_READ_REQUEST_DEFAULT_DMA0, MAX_OTF_READ_REQUEST_DEFAULT_DMA0,
          MAX_OTF_READ_REQUEST_DEFAULT_DMA0, MAX_OTF_READ_REQUEST_DEFAULT_DMA0};

// BBH Tx
uint8_t g_bbh_tx_sdma_bb_id_per_bbhtx[DMA_TARGET_NUM][BBH_TX_ID_NUM] =
    {{BB_ID_SDMA0, BB_ID_SDMA_COPY, BB_ID_SDMA1},
     {BB_ID_SDMA0, BB_ID_SDMA_COPY, BB_ID_SDMA1}};

uint8_t g_bbh_rx_sdma_bb_id_per_bbhrx[BBH_ID_NUM] =
    {BB_ID_SDMA0, BB_ID_SDMA0, BB_ID_SDMA0, BB_ID_SDMA0, BB_ID_SDMA0, BB_ID_SDMA0, BB_ID_SDMA1};

uint8_t g_bbh_rx_buff_num_new[NUM_OF_PERIPHERALS_PER_DMA] = {4,4,4,4,4,12,32};
uint8_t calculate_buffers_offset_rx_new[NUM_OF_PERIPHERALS_PER_DMA] = {0,4,8,12,16,20,0};
uint8_t g_bbh_tx_buff_num_new[NUM_OF_PERIPHERALS_PER_DMA] = {16,16,16,16,16,16,0};  // in tx, each DMA has 1 bbh that approach it
uint8_t calculate_buffers_offset_tx_new[NUM_OF_PERIPHERALS_PER_DMA] = {0,16,0,16,0,16,0};
uint8_t g_dma_into_urgent_threshold_new[NUM_OF_PERIPHERALS_PER_DMA] = {DMA_U_THRESH_IN_BBH_LAN_VALUE,DMA_U_THRESH_IN_BBH_LAN_VALUE,DMA_U_THRESH_IN_BBH_LAN_VALUE,DMA_U_THRESH_IN_BBH_LAN_VALUE,DMA_U_THRESH_IN_BBH_LAN_VALUE,DMA_U_THRESH_IN_BBH_PON_VALUE,DMA_U_THRESH_IN_BBH_PON_VALUE};
uint8_t g_dma_out_of_urgent_threshold_new[NUM_OF_PERIPHERALS_PER_DMA] = {DMA_U_THRESH_OUT_BBH_LAN_VALUE,DMA_U_THRESH_OUT_BBH_LAN_VALUE,DMA_U_THRESH_OUT_BBH_LAN_VALUE,DMA_U_THRESH_OUT_BBH_LAN_VALUE,DMA_U_THRESH_OUT_BBH_LAN_VALUE,DMA_U_THRESH_OUT_BBH_PON_VALUE,DMA_U_THRESH_OUT_BBH_PON_VALUE};
uint8_t g_dma_strict_priority_rx_new[NUM_OF_PERIPHERALS_PER_DMA] = {DMA_STRICT_PRI_RX_BBH_LAN_VALUE,DMA_STRICT_PRI_RX_BBH_LAN_VALUE,DMA_STRICT_PRI_RX_BBH_LAN_VALUE,DMA_STRICT_PRI_RX_BBH_LAN_VALUE,DMA_STRICT_PRI_RX_BBH_LAN_VALUE,DMA_STRICT_PRI_RX_BBH_PON_VALUE,DMA_STRICT_PRI_RX_BBH_PON_VALUE};
uint8_t g_dma_strict_priority_tx_new[NUM_OF_PERIPHERALS_PER_DMA] = {8,4,8,4,8,4,8};

uint32_t g_extra_dqm_tokens = 0;

static void bbh_rx_profiles_init(void)
{
    uint8_t bbh_id;
    bbh_rx_sdma_chunks_cfg_t *bbh_rx_sdma_cfg;

    g_bbh_rx_sdma_profile = (bbh_rx_sdma_profile_t *)xrdp_alloc(sizeof(bbh_rx_sdma_profile_t));

    for (bbh_id = 0; bbh_id < BBH_ID_NUM; bbh_id++)
    {
        bbh_rx_sdma_cfg = &(g_bbh_rx_sdma_profile->bbh_rx_sdma_chunks_config[bbh_id]);
        bbh_rx_sdma_cfg->sdma_bb_id = g_bbh_rx_sdma_bb_id_per_bbhrx[bbh_id];
        bbh_rx_sdma_cfg->first_chunk_idx = calculate_buffers_offset_rx_new[bbh_id];
        bbh_rx_sdma_cfg->sdma_chunks = g_bbh_rx_buff_num_new[bbh_id];
    }
}

static void bbh_tx_profiles_init(void)
{
    uint8_t tx_bbh_id;
    bbh_tx_bbh_dma_cfg *bbh_tx_dma_cfg;
    bbh_tx_bbh_sdma_cfg *bbh_tx_sdma_cfg;
    bbh_tx_bbh_ddr_cfg *bbh_tx_ddr_cfg;

    g_bbh_tx_dma_profile = (bbh_tx_dma_profile_t *)xrdp_alloc(sizeof(bbh_tx_dma_profile_t));
    g_bbh_tx_sdma_profile = (bbh_tx_sdma_profile_t *)xrdp_alloc(sizeof(bbh_tx_sdma_profile_t));
    g_bbh_tx_ddr_profile = (bbh_tx_ddr_profile_t *)xrdp_alloc(sizeof(bbh_tx_ddr_profile_t));

    for (tx_bbh_id = BBH_TX_ID_FIRST; tx_bbh_id < BBH_TX_ID_NUM; tx_bbh_id++)
    {
        bbh_tx_dma_cfg = &(g_bbh_tx_dma_profile->bbh_tx_dma_cfg[tx_bbh_id]);
        bbh_tx_dma_cfg->dmasrc =  g_bbh_tx_sdma_bb_id_per_bbhtx[DMA_TARGET_DMA][tx_bbh_id];;
        bbh_tx_dma_cfg->descbase = calculate_buffers_offset_tx_new[tx_bbh_id*2];
        bbh_tx_dma_cfg->descsize = g_bbh_tx_buff_num_new[tx_bbh_id*2];

        bbh_tx_sdma_cfg = &(g_bbh_tx_sdma_profile->bbh_tx_sdma_cfg[tx_bbh_id]);
        bbh_tx_sdma_cfg->sdmasrc = g_bbh_tx_sdma_bb_id_per_bbhtx[DMA_TARGET_SDMA][tx_bbh_id];;
        bbh_tx_sdma_cfg->descbase = calculate_buffers_offset_tx_new[tx_bbh_id*2+1];
        bbh_tx_sdma_cfg->descsize = g_bbh_tx_buff_num_new[tx_bbh_id*2+1];

        bbh_tx_ddr_cfg = &(g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[tx_bbh_id]);
        lookup_bbh_tx_bufsz_by_fpm_bufsz(&p_dpi_cfg->fpm_buf_size, &bbh_tx_ddr_cfg->bufsize);

        bbh_tx_ddr_cfg->byteresul = RES_1B;
        bbh_tx_ddr_cfg->ddrtxoffset = 0;
        /* MIN size = 0x4 - MAX size = 0x40  where to check values?*/
        bbh_tx_ddr_cfg->hnsize0 = 0x20;
        bbh_tx_ddr_cfg->hnsize1 = 0x20;
    }
    return;
}


static int bbh_rx_cfg(bbh_id_e bbh_id)
{
    uint8_t i;
    bdmf_error_t rc;
    bbh_rx_config cfg;

    xrdp_memset(&cfg, 0, sizeof(bbh_rx_config));

    cfg.sdma_chunks_cfg = &(g_bbh_rx_sdma_profile->bbh_rx_sdma_chunks_config[bbh_id]);
    cfg.disp_bb_id = BB_ID_DISPATCHER_REORDER;
    cfg.sbpm_bb_id = BB_ID_SBPM;

    /* TODO: should match other blocks configurations (runner) */
    /* TODO: should it be a profile as well ? */
    cfg.normal_viq = bbh_id;
    /* There's no exclusive viqs for non PON BBHs, therefore, excl viq configuration of BBH RX is the same as normal VIQ*/
    /*cfg.excl_viq = (bbh_id == BBH_ID_PON) ? DISP_REOR_VIQ_BBH_WAN_EXCL : bbh_id;*/
    cfg.excl_viq = bbh_id;

    cfg.excl_cfg.exc_en = (bbh_id == BBH_ID_PON);

    if (IS_WAN_RX_PORT(bbh_id))
    {
        cfg.min_pkt_size[0] = MIN_WAN_PKT_SIZE;

        cfg.min_pkt_size[1] = MIN_OMCI_PKT_SIZE;
        cfg.max_pkt_size[1] = MAX_OMCI_PKT_SIZE;
    }
    else
    {
        cfg.min_pkt_size[0] = MIN_ETH_PKT_SIZE;
    }

    cfg.max_pkt_size[0] = p_dpi_cfg->mtu_size;

    cfg.crc_omit_dis = 0;
    cfg.sop_offset = SOP_OFFSET;
    cfg.per_flow_th = BBH_RX_FLOWS_32_255_GROUP_DIVIDER;

    cfg.max_otf_sbpm = DRV_BBH_RX_MAXIMUM_OTF_SBPM_REQUESTS_DEFAULT_VAL;

    /* TODO: taken from 6838 : By default, the triggers for FC will be disabled and the triggers for drop enabled.
       If the user configures flow control for the port, the triggers for drop will be
       disabled and triggers for FC (including Runner request) will be enabled */
    /*config.flow_ctrl_cfg.drops = ?*/
    /*config.flow_ctrl_cfg.timer = ?*/

    rc = drv_bbh_rx_configuration_set(bbh_id, &cfg);
#ifndef RDP_SIM
    /* initialize all flows (including the 2 groups) */
    for (i = 0; !rc && i < DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_0 / 2; i++)
    {
        rc = ag_drv_bbh_rx_min_pkt_sel_flows_0_15_set(bbh_id, i, 0);
        rc = rc ? rc : ag_drv_bbh_rx_max_pkt_sel_flows_0_15_set(bbh_id, i, 0);
        rc = rc ? rc : ag_drv_bbh_rx_min_pkt_sel_flows_16_31_set(bbh_id, i, 0);
        rc = rc ? rc : ag_drv_bbh_rx_max_pkt_sel_flows_16_31_set(bbh_id, i, 0);
    }

    rc = rc ? rc : ag_drv_bbh_rx_pkt_sel_group_0_set(bbh_id, 0, 0);
    rc = rc ? rc : ag_drv_bbh_rx_pkt_sel_group_1_set(bbh_id, 0, 0);
#endif

    return rc;
}


static int bbh_rx_init(void)
{
    bdmf_error_t rc;
    bbh_id_e bbh_id;

    for (bbh_id = BBH_ID_FIRST; bbh_id < BBH_ID_PON; bbh_id++)
    {
        rc = bbh_rx_cfg(bbh_id);
        if (rc)
        {
            BDMF_TRACE_ERR("Error whlie configuring bbh_rx %d\n", bbh_id);
            return rc;
        }
    }
    return BDMF_ERR_OK;
}

static void ubus_bridge_init(void)
{

    ag_drv_ubus_slv_device_0_start_set(UBUS_SLV_DEVICE_0_START_ADDR);
    ag_drv_ubus_slv_device_0_end_set(UBUS_SLV_DEVICE_0_END_ADDR);
    ag_drv_ubus_slv_device_1_start_set(UBUS_SLV_DEVICE_1_START_ADDR);
    ag_drv_ubus_slv_device_1_end_set(UBUS_SLV_DEVICE_1_END_ADDR);
    ag_drv_ubus_slv_device_2_start_set(UBUS_SLV_DEVICE_2_START_ADDR);
    ag_drv_ubus_slv_device_2_end_set(UBUS_SLV_DEVICE_2_END_ADDR);

    ag_drv_ubus_slv_vpb_start_set(UBUS_SLV_VPB_START_ADDR);
    ag_drv_ubus_slv_vpb_end_set(UBUS_SLV_VPB_END_ADDRESS);
    ag_drv_ubus_slv_apb_start_set(UBUS_SLV_APB_START_ADDR);
    ag_drv_ubus_slv_apb_end_set(UBUS_SLV_APB_END_ADDRESS);
    ag_drv_ubus_mstr_hyst_ctrl_set(0, 2, 2); // ubus_mstr_id, cmd_space, data_space
    ag_drv_ubus_mstr_hyst_ctrl_set(1, 2, 2); // ubus_mstr_id, cmd_space, data_space
    ag_drv_ubus_mstr_hyst_ctrl_set(2, 2, 2); // ubus_mstr_id, cmd_space, data_space

}

uint32_t fpm_get_dqm_extra_fpm_tokens(void)
{
    return g_extra_dqm_tokens;
}

static int fpm_init(void)
{
    bdmf_error_t rc;
    bdmf_boolean reset_req = 1;
    fpm_pool2_intr_sts interrupt_status = FPM_INTERRUPT_STATUS;
    fpm_pool2_intr_msk interrupt_mask = FPM_INTERRUPT_MASK;
    uint16_t timeout = 0;

    drv_fpm_init(p_dpi_cfg->rdp_ddr_pkt_base_virt, p_dpi_cfg->fpm_buf_size);

    bdmf_trace("fpm_init buf size is =%d\n", p_dpi_cfg->fpm_buf_size);
    switch(p_dpi_cfg->fpm_buf_size)
    {
        case FPM_BUF_SIZE_256:
        case FPM_BUF_SIZE_512:
            g_extra_dqm_tokens = 0;
            break;
        case FPM_BUF_SIZE_1K:
        case FPM_BUF_SIZE_2K:
            g_extra_dqm_tokens = 0;
            break;
    }

    bdmf_trace("before ag_drv_fpm_pool1_xon_xoff_cfg_set p1=%d p2=%d\n", FPM_XON_THRESHOLD + g_extra_dqm_tokens, FPM_XOFF_THRESHOLD + g_extra_dqm_tokens);
    rc = ag_drv_fpm_pool1_xon_xoff_cfg_set(FPM_XON_THRESHOLD + g_extra_dqm_tokens, FPM_XOFF_THRESHOLD + g_extra_dqm_tokens);
    bdmf_trace("after ag_drv_fpm_pool1_xon_xoff_cfg_set p1=%d p2=%d\n", FPM_XON_THRESHOLD + g_extra_dqm_tokens, FPM_XOFF_THRESHOLD + g_extra_dqm_tokens);

    rc = rc ? rc : ag_drv_fpm_init_mem_set(1);

    /* polling until reset is finished */
    rc = rc ? rc : ag_drv_fpm_init_mem_get(&reset_req);
    while (!rc && reset_req && timeout <= FPM_INIT_TIMEOUT)
    {
        xrdp_usleep(FPM_POLL_SLEEP_TIME/2);
        rc = ag_drv_fpm_init_mem_get(&reset_req);
        timeout++;
    }
    if (timeout == FPM_INIT_TIMEOUT)
        rc = BDMF_ERR_INTERNAL;

    /* enable the fpm pool */
    rc = rc ? rc : ag_drv_fpm_pool1_en_set(1);

    /* fpm configurations */
#ifndef XRDP_EMULATION
    /* fpm interrupts */
    rc = rc ? rc : ag_drv_fpm_pool1_intr_sts_set(&interrupt_status);
    rc = rc ? rc : ag_drv_fpm_pool1_intr_msk_set(&interrupt_mask);
#endif

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize fpm driver\n");

    return rc;
}




static int sbpm_init(void)
{
    bdmf_error_t rc;
    uint16_t base_addr;
    uint16_t init_offset;

    /* base address and offset */
    base_addr = SBPM_BASE_ADDRESS;
    init_offset = SBPM_INIT_OFFSET;

    rc = ag_drv_sbpm_regs_init_free_list_set(base_addr, init_offset, 0, 0);



#ifdef RDP_SIM
    drv_sbpm_default_val_init();
#endif
    if (rc)
        BDMF_TRACE_ERR("Failed to initialize sbpm driver\n");

    return rc;
}


static int bbh_tx_lan_init(bbh_id_e bbh_id)
{
    bbh_tx_config config;
    uint8_t i, max_on_the_fly_reads;
    bdmf_phys_addr_t fpm_base_phys_addr;
    bdmf_error_t rc;
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

    config.rnr_cfg[0].rnr_src_id = get_runner_idx(cfe_core_runner_image);
    config.rnr_cfg[0].tcont_addr = 0;
    config.rnr_cfg[0].task_number = IMAGE_0_CFE_CORE_CPU_TX_THREAD_NUMBER;
    config.rnr_cfg[0].ptr_addr = IMAGE_0_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS >> 3;


    /* CHECK : how many queues to set (q2rnr)?*/
    config.lan_queue_cfg.queue_to_rnr = g_lan_queue_to_rnr;
    config.lan_queue_cfg.queue_profile = (bbh_id == BBH_ID_QM_COPY) ? g_bbhtx_qm_queue_profile : g_bbhtx_lan_queue_profile;
        config.lan_queue_cfg.qm_q = (bbh_id == BBH_ID_QM_COPY) ? g_qm_lan_qm_q : g_bbhtx_lan_qm_q;


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

    /* pd_prefetch_byte_threshold feature not enabled */
    config.lan_queue_cfg.pd_bytes_threshold_en = 0;
    config.lan_queue_cfg.pd_empty_threshold = 1;

    config.src_id.fpmsrc = BB_ID_FPM;
    config.src_id.sbpmsrc = BB_ID_SBPM;

    config.dma_cfg = &(g_bbh_tx_dma_profile->bbh_tx_dma_cfg[bbh_id]);
    config.sdma_cfg = &(g_bbh_tx_sdma_profile->bbh_tx_sdma_cfg[bbh_id]);
    config.ddr_cfg = &(g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[bbh_id]);
    
    fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
    GET_ADDR_HIGH_LOW(config.base_addr_high.addr[0], config.base_addr_low.addr[0], fpm_base_phys_addr);

    config.base_addr_low.addr[1] = 0;
    config.base_addr_high.addr[1] = 0;

    rc = drv_bbh_tx_configuration_set(bbh_id, &config);
    rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_eee_set(bbh_id, 0x3f);

    max_on_the_fly_reads = (g_max_otf_reads[bbh_id] > config.dma_cfg->descsize) ? config.dma_cfg->descsize : g_max_otf_reads[bbh_id];
    rc = rc ? rc : ag_drv_bbh_tx_dma_max_otf_read_request_set(bbh_id, max_on_the_fly_reads);
    max_on_the_fly_reads = (g_max_otf_reads[bbh_id] > config.sdma_cfg->descsize) ? config.sdma_cfg->descsize : g_max_otf_reads[bbh_id];
    rc = rc ? rc : ag_drv_bbh_tx_sdma_max_otf_read_request_set(bbh_id, max_on_the_fly_reads);
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_dfifoctrl_set(bbh_id, BBH_TX_DATA_FIFO_SRAM_SIZE, BBH_TX_DATA_FIFO_DDR_SIZE, BBH_TX_DATA_FIFO_SRAM_BASE, 1);

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize bbh_tx driver");

    return rc;
}

static int dma_sdma_init(void)
{
    int dma_id, peripheral_id;
      bdmf_error_t rc = BDMF_ERR_OK;

      for (dma_id = 0; dma_id < DMA_NUM; dma_id++)
      {
          for (peripheral_id = 0; peripheral_id < PERIPHERALS_PER_DMA; peripheral_id++)
          {
              if (g_bbhrx_to_dma[dma_id][peripheral_id])
              {
                  rc = rc ? rc : ag_drv_dma_config_num_of_writes_set(dma_id, peripheral_id,
                      g_bbh_rx_buffers_num[g_bbh_rx_buffers_num_index[dma_id][peripheral_id]]);
              }

              if (g_bbhtx_to_dma[dma_id][peripheral_id])
              {
                  //  each DMA "sees" each BBH TX twice. one as PSRAM BBH and one as DDR BBH
                  rc = rc ? rc :ag_drv_dma_config_target_mem_set(dma_id, peripheral_id * 2, 1, 0);
                  rc = rc ? rc :ag_drv_dma_config_target_mem_set(dma_id, peripheral_id * 2 + 1, 1, 1);

                  rc = rc ? rc : ag_drv_dma_config_num_of_reads_set(dma_id, peripheral_id * 2, g_bbh_tx_buffers_num[DMA_TARGET_DMA][peripheral_id]);
                  rc = rc ? rc : ag_drv_dma_config_num_of_reads_set(dma_id, peripheral_id * 2 + 1, g_bbh_tx_buffers_num[DMA_TARGET_SDMA][peripheral_id]);
              }

              rc = rc ? rc : ag_drv_dma_config_u_thresh_set(dma_id, peripheral_id, g_dma_urgent_threshold[dma_id][peripheral_id].into_urgent_threshold,
                  g_dma_urgent_threshold[dma_id][peripheral_id].out_of_urgent_threshold);

              rc = rc ? rc : ag_drv_dma_config_pri_set(dma_id, peripheral_id, g_dma_strict_priority[dma_id][peripheral_id].rx_side,
                  g_dma_strict_priority[dma_id][peripheral_id].tx_side);
              rc = rc ? rc : ag_drv_dma_config_weight_set(dma_id, peripheral_id, g_dma_rr_weight[dma_id][peripheral_id].rx_side, g_dma_rr_weight[dma_id][peripheral_id].tx_side);
              rc = rc ? rc : ag_drv_dma_config_periph_source_set(dma_id, peripheral_id, g_dma_bb_source[dma_id][peripheral_id].rx_side, g_dma_bb_source[dma_id][peripheral_id].tx_side);
          }

          if (dma_id == DMA_COPY_ID)
          {
              rc = rc ? rc :ag_drv_dma_config_ubus_credits_set(dma_id, DMA_DDR_CREDITS_1, DMA_PSRAM_CREDITS_1, 1, 1);
              rc = rc ? rc :ag_drv_dma_config_ubus_credits_set(dma_id, DMA_DDR_CREDITS_1, DMA_PSRAM_CREDITS_1, 0, 0);
          }
          else
          {
              rc = rc ? rc :ag_drv_dma_config_ubus_credits_set(dma_id, DMA_DDR_CREDITS_0, DMA_PSRAM_CREDITS_0, 1, 1);
              rc = rc ? rc :ag_drv_dma_config_ubus_credits_set(dma_id, DMA_DDR_CREDITS_0, DMA_PSRAM_CREDITS_0, 0, 0);
          }

          if (dma_id == DMA1_ID || dma_id == DMA0_ID)
          {
              ag_drv_dma_config_psram_base_set(dma_id, PSRAM1_BASE_OFFSET);
              ag_drv_dma_config_ubus_dpids_set(dma_id, 2, 23); // 2=DDR;  23=PSRAM1 + non default base addr 0x822000
          }
          else
          {
        	  ag_drv_dma_config_psram_base_set(dma_id, PSRAM0_BASE_OFFSET);
              ag_drv_dma_config_ubus_dpids_set(dma_id, 2, 22); // 2=DDR;  22=PSRAM
          }

          rc = rc ? rc :ag_drv_dma_config_max_otf_set(dma_id, DMA_DDR_BYTES_OTF, DMA_PSRAM_BYTES_OTF);
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

static int runner_init(void)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    rnr_dma_regs_cfg_t rnr_dma_cfg;
    uint8_t ubus_slave_idx, quad_idx;
    uint32_t addr_hi, addr_lo;
    bdmf_phys_addr_t fpm_base_phys_addr;
    rnr_regs_cfg_gen_cfg cfg_gen_cfg;
#if !defined(RDP_SIM)
    int done;
#endif
#if !defined(_CFE_) && !defined(RDP_SIM)
    bdmf_phys_addr_t psram_dma_base_phys_addr;
#endif
    rdd_init_params_t rdd_init_params = {0};
    

    drv_rnr_cores_addr_init();

#if !defined(RDP_SIM)
        ag_drv_rnr_regs_cfg_gen_cfg_get(0, &cfg_gen_cfg);
        cfg_gen_cfg.zero_data_mem = 1;
        cfg_gen_cfg.zero_context_mem = 1;
        ag_drv_rnr_regs_cfg_gen_cfg_set(0, &cfg_gen_cfg);
    do{
        done = 1;
            ag_drv_rnr_regs_cfg_gen_cfg_get(0, &cfg_gen_cfg);
            if ((cfg_gen_cfg.zero_data_mem_done == 0) || (cfg_gen_cfg.zero_context_mem_done == 0))
            {
                done = 0;
            }
    }while (!done);
#else
    drv_rnr_mem_init();
#endif
    drv_rnr_load_microcode();
#ifndef CONFIG_BRCM_IKOS
    drv_rnr_load_prediction();
#endif

         ag_drv_rnr_regs_cfg_gen_cfg_get(0, &cfg_gen_cfg);
         cfg_gen_cfg.disable_dma_old_flow_control = 1;
         cfg_gen_cfg.test_fit_fail = 0;
         cfg_gen_cfg.chicken_enable_dma_old_mode = 1;
        ag_drv_rnr_regs_cfg_gen_cfg_set(0, &cfg_gen_cfg);
    /* scheduler configuration */
    ag_drv_rnr_regs_cfg_sch_cfg_set(get_runner_idx(0), DRV_RNR_16SP);
    fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
    GET_ADDR_HIGH_LOW(addr_hi, addr_lo, fpm_base_phys_addr);

    g_fpm_hw_cfg.fpm_base_low = addr_lo;
    g_fpm_hw_cfg.fpm_base_high = addr_hi;

    rnr_dma_cfg.ddr.dma_base = (addr_hi << 12) | (addr_lo >> 20);

    lookup_dma_bufsz_by_fpm_bufsz(&p_dpi_cfg->fpm_buf_size, &rnr_dma_cfg.ddr.dma_buf_size); 

    rnr_dma_cfg.ddr.dma_static_offset = 0;
    rnr_dma_cfg.psram.dma_base = ((RU_BLK(PSRAM_MEM).addr[0] + RU_REG_OFFSET(PSRAM_MEM, MEMORY_DATA)) >> 20);
    rnr_dma_cfg.psram.dma_buf_size = DMA_BUFSIZE_128;
    rnr_dma_cfg.psram.dma_static_offset = 0;

    rc = drv_rnr_dma_cfg(&rnr_dma_cfg);
    if (rc)
    {
        BDMF_TRACE_ERR("drv_rnr_dma_cfg failed, rc %d\n", rc);
        return rc;
    }
    
    /* ToDo: fill rdd_init_params */

    g_fpm_hw_cfg.fpm_token_size_asr_8 = p_dpi_cfg->fpm_buf_size >> 8;

    rc = rc ? rc : rdd_data_structures_init(&rdd_init_params);
    rc = rc ? rc : rnr_frequency_set(RNR_FREQ_IN_MHZ / 2);
    if (rc)
        return rc;

    for (quad_idx = 0; quad_idx < NUM_OF_RNR_QUAD; quad_idx++) 
    {

        rnr_quad_general_config_dma_arb_cfg general_config_dma_arb_cfg;
        general_config_dma_arb_cfg.use_fifo_for_ddr_only   = 0;
        general_config_dma_arb_cfg.token_arbiter_is_rr     = 0;
        general_config_dma_arb_cfg.chicken_no_flowctrl     = 0;
        general_config_dma_arb_cfg.ddr_congest_threshold   = RNR_QUAD_DMA_ARB_CONJEST_THRESHOLD;
        general_config_dma_arb_cfg.psram_congest_threshold = RNR_QUAD_DMA_ARB_CONJEST_THRESHOLD;
        general_config_dma_arb_cfg.flow_ctrl_clear_token   = 0;
        general_config_dma_arb_cfg.enable_reply_threshold  = 0;

        /* change dynamic clock threshold in each quad */
        rc = rc ? rc : ag_drv_rnr_quad_general_config_dma_arb_cfg_set(quad_idx, &general_config_dma_arb_cfg);

        /* extend PSRAM slave tokens to 8 */
        for (ubus_slave_idx = 16; ubus_slave_idx < 20; ubus_slave_idx++) 
            rc = rc ? rc : ag_drv_rnr_quad_ext_flowctrl_config_token_val_set(quad_idx, ubus_slave_idx, 8);

        if (rc)
            return rc;
    }

    return rc;
}

static inline void dispatcher_reorder_viq_init(dsptchr_config *cfg, dsptchr_cngs_params *ingress_congs_init, dsptchr_cngs_params *egress_congs_init,
	uint8_t bb_id, uint32_t target_address, bdmf_boolean dest, bdmf_boolean delayed, uint8_t viq_num, uint8_t guaranteed_limit,
	uint16_t common_max_limit, bdmf_boolean is_bbh_queue)
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
    xrdp_memcpy(&(cfg->dsptchr_viq_list[viq_num].ingress_cngs), ingress_congs_init, sizeof(dsptchr_cngs_params));
    xrdp_memcpy(&(cfg->dsptchr_viq_list[viq_num].egress_cngs), egress_congs_init, sizeof(dsptchr_cngs_params));
    cfg->total_viq_guaranteed_buf += guaranteed_limit;
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
    int bbh_id;
    dsptchr_config cfg = {};
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t core_index, i;
    dsptchr_cngs_params ingress_congs_init = {
            .frst_lvl = DSPTCHR_CONG_PARAM_INGRESS_NORMAL,
            .scnd_lvl = DSPTCHR_CONG_PARAM_INGRESS_NORMAL,
            .hyst_thrs = DSPTCHR_CONG_PARAM_HYST};
    dsptchr_cngs_params egress_congs_init = {
            .frst_lvl = DSPTCHR_CONG_PARAM_EGRESS_NORMAL,
            .scnd_lvl = DSPTCHR_CONG_PARAM_EGRESS_NORMAL,
            .hyst_thrs = DSPTCHR_CONG_PARAM_HYST};


    /* reset dispatcher credit for all queues */
    for (i = 0; i < DSPTCHR_VIRTUAL_QUEUE_NUM; ++i)
        rc = rc ? rc : ag_drv_dsptchr_credit_cnt_set(i, 0);

    /* configure all viq for bbh-rx LAN */
    for (bbh_id = BBH_ID_FIRST; bbh_id <= BBH_ID_LAST; bbh_id++)
    {
        if (bbh_id == BBH_ID_PON)
            continue;
    /* normal VIQs for bbh-rx - 0*/
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, BB_ID_RX_BBH_0 + (2 * bbh_id), dsptchr_viq_bbh_target_addr_normal,
        dsptchr_viq_dest_disp, dsptchr_viq_delayed, bbh_id , DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ,  DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 1);

        /* exclusive viq for bbh-rx */
        /* TODO: actually all queues besides PON exclusive can be ommited */
   /* normal VIQs for bbh-rx - 0*/
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, BB_ID_RX_BBH_0 + (2 * bbh_id), dsptchr_viq_bbh_target_addr_normal,
        dsptchr_viq_dest_disp, dsptchr_viq_delayed, bbh_id + BBH_ID_NUM, DSPTCHR_EXCLUSIVE_WAN_GUARANTEED_BUFFERS * (bbh_id == BBH_ID_PON),
        DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 1);

    }

   



    /* VIQ for CPU_TX egress - 12 */
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, get_runner_idx(cfe_core_runner_image), (IMAGE_0_CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 |
        IMAGE_0_CFE_CORE_CPU_TX_THREAD_NUMBER << 12), dsptchr_viq_dest_reor, dsptchr_viq_delayed, DISP_REOR_VIQ_CPU_TX_EGRESS,
        DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ, DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 0);

 
    /* configure all rnr groups */
    for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
    {
        /* setting group 1 - wan direct */
        if (rdp_core_to_image_map[core_index] == cfe_core_runner_image)
        {
            dispatcher_reorder_rnr_group_init(&cfg, 1, core_index, (1<< IMAGE_0_CFE_CORE_CPU_RX_THREAD_NUMBER), (IMAGE_0_DIRECT_PROCESSING_PD_TABLE_ADDRESS >> 3), 0);
        }
    }

    /* mapping viq to rnr group */
    /* send all traffic via "direct flow" */
    cfg.rnr_grp_num = 2;
    cfg.dsptchr_rnr_group_list[0].queues_mask = 0x0;
    cfg.dsptchr_rnr_group_list[1].queues_mask = 0x3F;

    /* set up pools limits */
    cfg.pools_limits.grnted_pool_lmt = cfg.total_viq_guaranteed_buf;
    cfg.pools_limits.grnted_pool_size = cfg.total_viq_guaranteed_buf;
    cfg.pools_limits.cmn_pool_lmt = DSPTCHR_CONG_PARAM_GLOBAL - cfg.total_viq_guaranteed_buf;
    cfg.pools_limits.cmn_pool_size = DSPTCHR_CONG_PARAM_GLOBAL - cfg.total_viq_guaranteed_buf;
    cfg.pools_limits.mcast_pool_lmt = 0;
    cfg.pools_limits.mcast_pool_size = 0;
    cfg.pools_limits.rnr_pool_lmt = 0;         /* runners multicast prefetch limit */
    cfg.pools_limits.rnr_pool_size = 0;        /* current in processing for multicast */
    cfg.pools_limits.processing_pool_size = 0; /* current in processing */

    /* enable all viq except wan-bbh */
    cfg.queue_en_vec = DISP_REOR_XRDP_VIQ_EN((DISP_REOR_VIQ_LAST + 1));

    rc = drv_dis_reor_queues_init();
    rc = rc ? rc : drv_dis_reor_tasks_to_rg_init();
    rc = rc ? rc : drv_dis_reor_free_linked_list_init();
    rc = rc ? rc : drv_dis_reor_cfg(&cfg);
    rc = rc ? rc : ag_drv_dsptchr_load_balancing_lb_cfg_set(1, 2); /*set strict task priority mode in dispatcher*/
    return rc;
}




static void rdp_block_enable(void)
{
#if defined(XRDP_EMULATION) || defined(CONFIG_BRCM_QEMU)
    unimac_rdp_command_config cmd;
    unimac_rdp_mac_pfc_ctrl   ppp_cntrl;
#endif
    uint8_t idx;
    qm_enable_ctrl qm_enable = {
        .fpm_prefetch_enable = 1,
        .reorder_credit_enable = 1,
        .dqm_pop_enable = 1,
        .rmt_fixed_arb_enable = 1,
        .dqm_push_fixed_arb_enable = 1
    };

    bdmf_error_t rc = BDMF_ERR_OK;
    dsptchr_reorder_cfg_dsptchr_reordr_cfg reorder_cfg_dsptchr_reordr_cfg = {1, 1, 0, 0, 0};
#ifndef RDP_SIM
    bdmf_boolean rdy = 0;
    bdmf_boolean bsy = 0;
    uint16_t init_offset = 0;
    uint16_t base_addr = 0;

    /* enable BBH_RX */
    rc = ag_drv_sbpm_regs_init_free_list_get(&base_addr, &init_offset, &bsy, &rdy);
    while ((rc == BDMF_ERR_OK) && (rdy == 0))
    {
        xrdp_usleep(FPM_POLL_SLEEP_TIME);
        rc = ag_drv_sbpm_regs_init_free_list_get(&base_addr, &init_offset, &bsy, &rdy);
    }
#endif
    for (idx = BBH_ID_FIRST; idx < BBH_ID_PON; idx++)
    {
        rc = rc ? rc : ag_drv_bbh_rx_general_configuration_enable_set(idx, 1, 1);
    }

    /* enable DISP_REOR */
    rc = rc ? rc : ag_drv_dsptchr_reorder_cfg_dsptchr_reordr_cfg_set(&reorder_cfg_dsptchr_reordr_cfg);

    /* enable QM */
    rc = rc ? rc : ag_drv_qm_enable_ctrl_set(&qm_enable);
    rc = rc ? rc : ag_drv_ubus_mstr_en_set(0, 1);
    rc = rc ? rc : ag_drv_ubus_mstr_en_set(1, 1);
    rc = rc ? rc : ag_drv_ubus_mstr_en_set(2, 1);


    /* enable RNR 0*/
    rc = rc ? rc : ag_drv_rnr_regs_rnr_enable_set(0, 1);

#if defined(XRDP_EMULATION) || defined(CONFIG_BRCM_QEMU)
    /* enable unimac */
    for (idx = 0; idx < NUM_OF_UNIMAC; idx++)
    {
        rc = rc ? rc : ag_drv_unimac_rdp_command_config_get(idx, &cmd);
        cmd.rx_ena = 1;
        cmd.tx_ena = 1;
        cmd.cntl_frm_ena  = 1;
        rc = rc ? rc : ag_drv_unimac_rdp_command_config_set(idx, &cmd);

        rc = rc ? rc : ag_drv_unimac_rdp_mac_pfc_ctrl_get(idx, &ppp_cntrl);
        ppp_cntrl.pfc_stats_en = 1;
        ppp_cntrl.rx_pass_pfc_frm = 1;
        ppp_cntrl.pfc_rx_enbl = 1;
        rc = rc ? rc : ag_drv_unimac_rdp_mac_pfc_ctrl_set(idx, &ppp_cntrl);

        /* interrupt enable*/
        rc = rc ? rc : ag_drv_unimac_misc_unimac_top_unimac_ints_ier_set(idx, 1);
    }
#endif

    if (rc)
        BDMF_TRACE_ERR("Failed to enable rdp blocks\n");
}


static int _data_path_init(dpi_params_t *dpi_params)
{
    bdmf_error_t rc;
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

    rc = fpm_init();
    rc = rc ? rc : runner_init();
    rc = rc ? rc : bbh_rx_init();
    rc = rc ? rc : bbh_tx_lan_init(BBH_TX_ID_LAN);
    rc = rc ? rc : bbh_tx_lan_init(BBH_ID_QM_COPY);
    rc = rc ? rc : sbpm_init();
    rc = rc ? rc : dma_sdma_init();
    rc = rc ? rc : dispatcher_reorder_init();
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
#if defined(CONFIG_BCM_UBUS_DECODE_REMAP)
    int rc; 
    rc = drv_rnr_quad_ubus_decode_wnd_cfg(0, 0, g_board_size_power_of_2, UBUS_PORT_ID_BIU, IS_DDR_COHERENT);
    if (rc < 0)
    {
        printk("Error %s line[%d] size[%d]: \n",__FILE__, __LINE__, g_board_size_power_of_2);
        return rc;
    }
#endif
    return _data_path_init(dpi_params);
}

int data_path_init_basic(dpi_params_t *dpi_params)
{
    int rc;

    /* Basic data path */
    rc = _data_path_init(dpi_params);

    if (rc)
        bdmf_trace("%s failed, error %d\n", __FUNCTION__, rc);
    return rc;
}


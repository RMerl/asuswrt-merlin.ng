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


#ifdef _CFE_
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
#else
#define xrdp_memset memset
#define xrdp_memcpy memcpy
#define xrdp_usleep(_a) bdmf_usleep(_a)
#define xrdp_alloc(_a) bdmf_alloc(_a)
#endif

#include "rdd.h"
#include "rdd_defs.h"
#include "data_path_init.h"
#include "rdd_init.h"
#include "rdd_cpu_rx.h"
#ifndef _CFE_
#include "rdd_tcam_ic.h"
#endif
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
#ifndef _CFE_ 
#include "rdp_drv_natc.h"
#endif
#include "rdp_drv_tcam.h"
#include "rdp_drv_fpm.h"
#ifndef _CFE_
#include "rdp_drv_hash.h"
#endif
#include "rdp_drv_bkpt.h"
#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_ubus_mstr_ag.h"
#include "xrdp_drv_ubus_slv_ag.h"
#include "rdp_drv_qm.h"
#include "bdmf_data_types.h"
#include "rdd_scheduling.h"
#include "rdd_ghost_reporting.h"
#include "xrdp_drv_unimac_rdp_ag.h"
#include "xrdp_drv_unimac_misc_ag.h"
#include "rdd_ag_service_queues.h"
#include "rdd_ag_ds_tm.h"
#include "rdd_ag_us_tm.h"


#ifndef RDP_SIM
#include "bcm_misc_hw_init.h"
#include "bcm_map_part.h"
#endif

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
pd_bytes_threshold_t g_lan_pd_bytes_threshold[BBH_TX_NUM_OF_LAN_QUEUES_PAIRS] = { {BBH_TX_FIFO_BYTES_THRESHOLD_LAN,BBH_TX_FIFO_BYTES_THRESHOLD_LAN}, {BBH_TX_FIFO_BYTES_THRESHOLD_LAN,BBH_TX_FIFO_BYTES_THRESHOLD_LAN}, {BBH_TX_FIFO_BYTES_THRESHOLD_LAN,BBH_TX_FIFO_BYTES_THRESHOLD_LAN}, {BBH_TX_FIFO_BYTES_THRESHOLD_LAN,BBH_TX_FIFO_BYTES_THRESHOLD_LAN} };
pd_bytes_threshold_t g_wan_pd_bytes_threshold_1g[1] = { {BBH_TX_FIFO_BYTES_THRESHOLD_WAN_1G, BBH_TX_FIFO_BYTES_THRESHOLD_WAN_1G} };
pd_bytes_threshold_t g_wan_pd_bytes_threshold_10g[1] = { {BBH_TX_FIFO_BYTES_THRESHOLD_WAN_10G, BBH_TX_FIFO_BYTES_THRESHOLD_WAN_10G} };
dma_id_e g_dma_source[] = {BB_ID_DMA0, BB_ID_SDMA0};

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

bbh_to_dma_x_t  g_bbh_to_dma_x[BBH_ID_NUM] = { 
          {BBH_ID_0, DMA0_ID},
          {BBH_ID_1, DMA0_ID},
          {BBH_ID_2, DMA0_ID},
          {BBH_ID_3, DMA0_ID},
          {BBH_ID_4, DMA0_ID},
          {BBH_ID_PON, DMA0_ID} };

bbh_id_e g_dma_to_bbh_x[NUM_OF_DMA][NUM_OF_PERIPHERALS_PER_DMA] = {
          {BBH_ID_0, BBH_ID_1, BBH_ID_2, BBH_ID_3, BBH_ID_4, BBH_ID_PON} };
          
/* array desription:
      - this array describe the buffers weight accroding to g_dma_to_bbh_X array.
      - total weight is 32 for each DMA (four BBH IDs) */
uint8_t g_bbh_buff_num[NUM_OF_DMA][NUM_OF_PERIPHERALS_PER_DMA] = {{7, 7, 7, 7, 7, 13 }};

bb_source_t g_dma_bb_source[BBH_ID_NUM] = {
         {BB_ID_RX_BBH_0, BB_ID_TX_LAN},
         {BB_ID_RX_BBH_1, BB_ID_TX_PON_ETH_PD},
         {BB_ID_RX_BBH_2, BB_ID_TX_LAN},
         {BB_ID_RX_BBH_3, BB_ID_TX_LAN}, 
         {BB_ID_RX_BBH_4, BB_ID_TX_LAN},
         {BB_ID_RX_PON, BB_ID_TX_LAN} };

uint8_t g_max_otf_reads[BBH_ID_NUM] = {MAX_OTF_READ_REQUEST_DEFAULT_DMA0,
          MAX_OTF_READ_REQUEST_DEFAULT_DMA0,
          MAX_OTF_READ_REQUEST_DEFAULT_DMA0, 
	  MAX_OTF_READ_REQUEST_DEFAULT_DMA0,
          MAX_OTF_READ_REQUEST_DEFAULT_DMA0, 
	  MAX_OTF_READ_REQUEST_DEFAULT_DMA0};

urgent_threhold_t g_dma_urgent_threshold[BBH_ID_NUM][2] = {
         { {DMA_U_THRESH_IN_BBH_LAN_VALUE, DMA_U_THRESH_OUT_BBH_LAN_VALUE},
           {SDMA_U_THRESH_IN_BBH_LAN_VALUE, SDMA_U_THRESH_OUT_BBH_LAN_VALUE} },
         { {DMA_U_THRESH_IN_BBH_LAN_VALUE, DMA_U_THRESH_OUT_BBH_LAN_VALUE},
           {SDMA_U_THRESH_IN_BBH_LAN_VALUE, SDMA_U_THRESH_OUT_BBH_LAN_VALUE} },
         { {DMA_U_THRESH_IN_BBH_LAN_VALUE, DMA_U_THRESH_OUT_BBH_LAN_VALUE}, 
           {SDMA_U_THRESH_IN_BBH_LAN_VALUE, SDMA_U_THRESH_OUT_BBH_LAN_VALUE} },
         { {DMA_U_THRESH_IN_BBH_LAN_VALUE, DMA_U_THRESH_OUT_BBH_LAN_VALUE},
           {SDMA_U_THRESH_IN_BBH_LAN_VALUE, SDMA_U_THRESH_OUT_BBH_LAN_VALUE} },
         { {DMA_U_THRESH_IN_BBH_LAN_VALUE, DMA_U_THRESH_OUT_BBH_LAN_VALUE},
           {SDMA_U_THRESH_IN_BBH_LAN_VALUE, SDMA_U_THRESH_OUT_BBH_LAN_VALUE} },
         { {DMA_U_THRESH_IN_BBH_PON_VALUE, DMA_U_THRESH_OUT_BBH_PON_VALUE},
           {SDMA_U_THRESH_IN_BBH_PON_VALUE, SDMA_U_THRESH_OUT_BBH_PON_VALUE} } };

strict_priority_t g_dma_strict_priority[BBH_ID_NUM][2] = {
         { {DMA_STRICT_PRI_RX_BBH_LAN_VALUE, DMA_STRICT_PRI_TX_BBH_LAN_VALUE},
           {SDMA_STRICT_PRI_RX_BBH_LAN_VALUE, SDMA_STRICT_PRI_TX_BBH_LAN_VALUE} },
         { {DMA_STRICT_PRI_RX_BBH_LAN_VALUE, DMA_STRICT_PRI_TX_BBH_LAN_VALUE},
           {SDMA_STRICT_PRI_RX_BBH_LAN_VALUE, SDMA_STRICT_PRI_TX_BBH_LAN_VALUE} },
         { {DMA_STRICT_PRI_RX_BBH_LAN_VALUE, DMA_STRICT_PRI_TX_BBH_LAN_VALUE}, 
           {SDMA_STRICT_PRI_RX_BBH_LAN_VALUE, SDMA_STRICT_PRI_TX_BBH_LAN_VALUE} },
         { {DMA_STRICT_PRI_RX_BBH_LAN_VALUE, DMA_STRICT_PRI_TX_BBH_LAN_VALUE},
           {SDMA_STRICT_PRI_RX_BBH_LAN_VALUE, SDMA_STRICT_PRI_TX_BBH_LAN_VALUE} },
         { {DMA_STRICT_PRI_RX_BBH_LAN_VALUE, DMA_STRICT_PRI_TX_BBH_LAN_VALUE},
           {SDMA_STRICT_PRI_RX_BBH_LAN_VALUE, SDMA_STRICT_PRI_TX_BBH_LAN_VALUE} },
         { {DMA_STRICT_PRI_RX_BBH_PON_VALUE, DMA_STRICT_PRI_TX_BBH_PON_VALUE},
           {SDMA_STRICT_PRI_RX_BBH_PON_VALUE, SDMA_STRICT_PRI_TX_BBH_PON_VALUE} } };

rr_weight_t g_dma_rr_weight[BBH_ID_NUM][2] = {
         { {DMA_RR_WEIGHT_RX_BBH_LAN_VALUE, DMA_RR_WEIGHT_TX_BBH_LAN_VALUE},
           {SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE, SDMA_RR_WEIGHT_TX_BBH_LAN_VALUE} },
         { {DMA_RR_WEIGHT_RX_BBH_LAN_VALUE, DMA_RR_WEIGHT_TX_BBH_LAN_VALUE},
           {SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE, SDMA_RR_WEIGHT_TX_BBH_LAN_VALUE} },
         { {DMA_RR_WEIGHT_RX_BBH_LAN_VALUE, DMA_RR_WEIGHT_TX_BBH_LAN_VALUE}, 
           {SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE, SDMA_RR_WEIGHT_TX_BBH_LAN_VALUE} },
         { {DMA_RR_WEIGHT_RX_BBH_LAN_VALUE, DMA_RR_WEIGHT_TX_BBH_LAN_VALUE},
           {SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE, SDMA_RR_WEIGHT_TX_BBH_LAN_VALUE} },
         { {DMA_RR_WEIGHT_RX_BBH_LAN_VALUE, DMA_RR_WEIGHT_TX_BBH_LAN_VALUE},
           {SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE, SDMA_RR_WEIGHT_TX_BBH_LAN_VALUE} },
         { {DMA_RR_WEIGHT_RX_BBH_PON_VALUE, DMA_RR_WEIGHT_TX_BBH_PON_VALUE},
           {SDMA_RR_WEIGHT_RX_BBH_PON_VALUE, SDMA_RR_WEIGHT_TX_BBH_PON_VALUE} } };

uint32_t g_extra_dqm_tokens = 0;

static uint8_t calculate_buffers_offset(uint8_t dma_num, uint8_t periphreal_num)
{
    uint8_t j, offset = 0;

    for (j = 0; j < periphreal_num; j++)
    {
       offset = offset + g_bbh_buff_num[dma_num][j];
    }
    return offset;
}

static int bbh_profiles_init(void)
{
    uint8_t i, j, bbh_id;
    bbh_rx_sdma_chunks_cfg_t *bbh_rx_sdma_cfg;
    bbh_tx_bbh_dma_cfg *bbh_tx_dma_cfg;
    bbh_tx_bbh_sdma_cfg *bbh_tx_sdma_cfg;
    bbh_tx_bbh_ddr_cfg *bbh_tx_ddr_cfg;

    g_bbh_rx_sdma_profile = (bbh_rx_sdma_profile_t *)xrdp_alloc(sizeof(bbh_rx_sdma_profile_t));
    g_bbh_tx_dma_profile = (bbh_tx_dma_profile_t *)xrdp_alloc(sizeof(bbh_tx_dma_profile_t));
    g_bbh_tx_sdma_profile = (bbh_tx_sdma_profile_t *)xrdp_alloc(sizeof(bbh_tx_sdma_profile_t));
    g_bbh_tx_ddr_profile = (bbh_tx_ddr_profile_t *)xrdp_alloc(sizeof(bbh_tx_ddr_profile_t));

    for (bbh_id = 0; bbh_id < BBH_ID_NUM; bbh_id++)
    {
        int found = 0;

        /* serach BBH_ID in DMA/SDMA array */
        for (i = 0; i < NUM_OF_DMA; i++)
        {
            for (j = 0; j < NUM_OF_PERIPHERALS_PER_DMA; j++)
            {
                if (g_bbh_to_dma_x[bbh_id].bbh_id == g_dma_to_bbh_x[i][j])
                {
                    found = 1;
                    break;
                }
            }
            if (found)
                break;
        }
        if (!found)
            return BDMF_ERR_INTERNAL;

        bbh_rx_sdma_cfg = &(g_bbh_rx_sdma_profile->bbh_rx_sdma_chunks_config[bbh_id]);
        bbh_rx_sdma_cfg->sdma_bb_id = BB_ID_SDMA0;
        bbh_rx_sdma_cfg->first_chunk_idx = calculate_buffers_offset(i, j) * BBH_RX_TOTAL_BUFFER_NUM / TOTAL_BUFFERS_WEIGHT;
        bbh_rx_sdma_cfg->sdma_chunks = g_bbh_buff_num[i][j] * BBH_RX_TOTAL_BUFFER_NUM / TOTAL_BUFFERS_WEIGHT;

        bbh_tx_dma_cfg = &(g_bbh_tx_dma_profile->bbh_tx_dma_cfg[bbh_id]);
        bbh_tx_dma_cfg->dmasrc = BB_ID_DMA0;
        bbh_tx_dma_cfg->descbase = calculate_buffers_offset(i, j) * BBH_TX_TOTAL_BUFFER_NUM / TOTAL_BUFFERS_WEIGHT;
        bbh_tx_dma_cfg->descsize = g_bbh_buff_num[i][j] * BBH_TX_TOTAL_BUFFER_NUM / TOTAL_BUFFERS_WEIGHT;

        bbh_tx_sdma_cfg = &(g_bbh_tx_sdma_profile->bbh_tx_sdma_cfg[bbh_id]);
        bbh_tx_sdma_cfg->sdmasrc = BB_ID_SDMA0;
        bbh_tx_sdma_cfg->descbase = calculate_buffers_offset(i, j) * BBH_TX_TOTAL_BUFFER_NUM / TOTAL_BUFFERS_WEIGHT;
        bbh_tx_sdma_cfg->descsize = g_bbh_buff_num[i][j] * BBH_TX_TOTAL_BUFFER_NUM / TOTAL_BUFFERS_WEIGHT;

        bbh_tx_ddr_cfg = &(g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[bbh_id]);

        lookup_bbh_tx_bufsz_by_fpm_bufsz(&p_dpi_cfg->fpm_buf_size, &bbh_tx_ddr_cfg->bufsize);

        bbh_tx_ddr_cfg->byteresul = RES_1B;
        bbh_tx_ddr_cfg->ddrtxoffset = 0;
        /* MIN size = 0x4 - MAX size = 0x40  where to check values?*/
        bbh_tx_ddr_cfg->hnsize0 = 0x20;
        bbh_tx_ddr_cfg->hnsize1 = 0x20;
    }
    return 0;
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
    cfg.excl_viq = (bbh_id == BBH_ID_PON) ? DISP_REOR_VIQ_BBH_WAN_EXCL : bbh_id;
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

static int bbh_rx_pon_init(int wan_bbh)
{
    bdmf_error_t rc;
    bbh_rx_pattern_recog pattern_recog = {};

    /* BBH-RX mactype init */
    rc = ag_drv_bbh_rx_mac_mode_set(BBH_ID_PON, (wan_bbh == MAC_TYPE_EPON || wan_bbh == MAC_TYPE_XEPON),
        (wan_bbh == MAC_TYPE_XGPON || wan_bbh == MAC_TYPE_XEPON));
    
    if (wan_bbh == MAC_TYPE_GPON || wan_bbh == MAC_TYPE_XGPON)
    {
        rc = rc ? rc : ag_drv_bbh_rx_ploam_en_set(BBH_ID_PON, 1);
        rc = rc ? rc : ag_drv_bbh_rx_user_priority3_en_set(BBH_ID_PON, 1);
    }

    rc = rc ? rc : bbh_rx_cfg(BBH_ID_PON);

    if (wan_bbh == MAC_TYPE_EPON || wan_bbh == MAC_TYPE_XEPON)
    {
        pattern_recog.patterndatamsb = EPON_CONTROL_PATTERN_MSB;
        pattern_recog.patterndatalsb = EPON_CONTROL_PATTERN_LSB;
        pattern_recog.patternmaskmsb = EPON_CONTROL_PATTERN_MASK_MSB;
        pattern_recog.patternmasklsb = EPON_CONTROL_PATTERN_MASK_LSB;
        pattern_recog.pattenoffset = 1;
        rc = rc ? rc : ag_drv_bbh_rx_pattern_recog_set(BBH_ID_PON, &pattern_recog);
        rc = rc ? rc : ag_drv_bbh_rx_pattern_en_set(BBH_ID_PON, 1);
    }
    if (rc)
    {
        BDMF_TRACE_ERR("Error whlie configuring wan bbh_rx %d\n", BBH_ID_PON);
        return rc;
    }
    return ag_drv_bbh_rx_general_configuration_enable_set(BBH_ID_PON, 1, 1);
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
    ag_drv_ubus_slv_vpb_base_set(UBUS_SLV_VPB_BASE_ADDR);
    ag_drv_ubus_slv_vpb_mask_set(UBUS_SLV_MASK);
    ag_drv_ubus_slv_apb_base_set(UBUS_SLV_APB_BASE_ADDR);
    ag_drv_ubus_slv_apb_mask_set(UBUS_SLV_MASK);

    ag_drv_ubus_mstr_hyst_ctrl_set(UBUS_MSTR_ID_0, UBUS_MSTR_CMD_SPCAE, UBUS_MSTR_DATA_SPCAE);
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

    g_extra_dqm_tokens = FPM_EXTRA_TOKENS_FOR_DQM;

    rc = ag_drv_fpm_pool1_xon_xoff_cfg_set(FPM_XON_THRESHOLD + g_extra_dqm_tokens, FPM_XOFF_THRESHOLD + g_extra_dqm_tokens);
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

    /* configure runner quads */
    for (i = 0; !rc && i < NUM_OF_RNR_QUAD; i++)
    {
        /* parser configuration */
        rc = ag_drv_rnr_quad_tcp_flags_set(i, PARSER_TCP_CTL_FLAGS);
        rc = rc ? rc : ag_drv_rnr_quad_profile_us_set(i, PARSER_PROFILE_US);
        rc = rc ? rc : ag_drv_rnr_quad_exception_bits_set(i, PARSER_EXCP_STATUS_BITS);
        rc = rc ? rc : ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_set(i,
            PARSER_PPP_PROTOCOL_CODE_0_IPV4, PARSER_PPP_PROTOCOL_CODE_1_IPV6);
        rc = rc ? rc : ag_drv_rnr_quad_parser_core_configuration_eng_set(i, PARSER_AH_DETECTION);
    }

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize parser driver\n");

    return rc;
}

#ifndef _CFE_
static int hash_init(void)
{
    hash_config_t hash_cfg = {};

    hash_cfg.tbl_num = 3;

    /* IPTV table configuration */
    hash_cfg.tbl_cfg[GET_SW_TABLE_ID(HASH_TABLE_IPTV)].tbl_size = p_dpi_cfg->iptv_table_size;
    hash_cfg.tbl_cfg[GET_SW_TABLE_ID(HASH_TABLE_IPTV)].cam_en = 1;
    hash_cfg.tbl_init[GET_SW_TABLE_ID(HASH_TABLE_IPTV)].int_ctx_size = 3;
    hash_cfg.tbl_init[GET_SW_TABLE_ID(HASH_TABLE_IPTV)].ext_ctx_size = 0;
    hash_cfg.tbl_init[GET_SW_TABLE_ID(HASH_TABLE_IPTV)].search_depth_per_engine = 16;

    /* 32 bit key */
    hash_cfg.mask[GET_SW_TABLE_ID(HASH_TABLE_IPTV)].maskl = HASH_TABLE_IPTV_KEY_MASK_LO;
    hash_cfg.mask[GET_SW_TABLE_ID(HASH_TABLE_IPTV)].maskh = HASH_TABLE_IPTV_KEY_MASK_HI;


    /* MAC (ARL) table configuration */
    hash_cfg.tbl_cfg[GET_SW_TABLE_ID(HASH_TABLE_ARL)].tbl_size = p_dpi_cfg->arl_table_size;
    hash_cfg.tbl_cfg[GET_SW_TABLE_ID(HASH_TABLE_ARL)].cam_en = 1;
    hash_cfg.tbl_init[GET_SW_TABLE_ID(HASH_TABLE_ARL)].int_ctx_size = 0;
    hash_cfg.tbl_init[GET_SW_TABLE_ID(HASH_TABLE_ARL)].ext_ctx_size = 3;
    hash_cfg.tbl_init[GET_SW_TABLE_ID(HASH_TABLE_ARL)].search_depth_per_engine = 12;
    hash_cfg.tbl_init[GET_SW_TABLE_ID(HASH_TABLE_ARL)].cam_max_threshold = DEF_MAX_CAM_THRESHOLD;

    /* 60 bit key */
    hash_cfg.mask[GET_SW_TABLE_ID(HASH_TABLE_ARL)].maskl = HASH_TABLE_ARL_KEY_MASK_LO;
    hash_cfg.mask[GET_SW_TABLE_ID(HASH_TABLE_ARL)].maskh = HASH_TABLE_ARL_KEY_MASK_HI;


    /* VLAN lookup table configuration */
    hash_cfg.tbl_cfg[GET_SW_TABLE_ID(HASH_TABLE_BRIDGE_AND_VLAN_LKP)].tbl_size =
        HASH_TABLE_BRIDGE_AND_VLAN_LKP_SIZE_PER_ENGINE;
    hash_cfg.tbl_cfg[GET_SW_TABLE_ID(HASH_TABLE_BRIDGE_AND_VLAN_LKP)].cam_en = 1;
    hash_cfg.tbl_init[GET_SW_TABLE_ID(HASH_TABLE_BRIDGE_AND_VLAN_LKP)].int_ctx_size = 3;
    hash_cfg.tbl_init[GET_SW_TABLE_ID(HASH_TABLE_BRIDGE_AND_VLAN_LKP)].ext_ctx_size = 6;
    hash_cfg.tbl_init[GET_SW_TABLE_ID(HASH_TABLE_BRIDGE_AND_VLAN_LKP)].search_depth_per_engine = 12;

    /* 20 bit key */
    hash_cfg.mask[GET_SW_TABLE_ID(HASH_TABLE_BRIDGE_AND_VLAN_LKP)].maskl = HASH_TABLE_BRIDGE_AND_VLAN_LKP_MASK_LO;
    hash_cfg.mask[GET_SW_TABLE_ID(HASH_TABLE_BRIDGE_AND_VLAN_LKP)].maskh = HASH_TABLE_BRIDGE_AND_VLAN_LKP_MASK_HI;

    /* reset hash memory */
    return drv_hash_init(&hash_cfg);
}
#endif /*_CFE_*/

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
        
#ifndef _CFE_
    rc = rc ? rc : drv_sbpm_thr_ug1_get(&thr_ug);
    thr_ug.bn_thr = SBPM_UG1_BN_THRESHOLD;
    thr_ug.excl_low_thr = SBPM_UG1_EXCL_LOW_THRESHOLD;
    thr_ug.excl_low_hyst = SBPM_UG1_EXCL_LOW_HIST;
    rc = rc ? rc : drv_sbpm_thr_ug1_set(&thr_ug);
#endif

#ifdef RDP_SIM
    drv_sbpm_default_val_init();
#endif
    if (rc)
        BDMF_TRACE_ERR("Failed to initialize sbpm driver\n");

    return rc;
}

static int bbh_tx_pon_init(int wantype)
{
    bbh_tx_config config;
    uint8_t i, max_on_the_fly_reads;
    bdmf_phys_addr_t fpm_base_phys_addr;
    bdmf_error_t rc;
    pd_fifo_base_t gpon_pd_fifo_base[TX_QEUEU_PAIRS] = {}; 
    pd_fifo_base_t epon_pd_fifo_base[TX_QEUEU_PAIRS] = {};
    pd_bytes_threshold_t *pd_bytes_threshold;

    gpon_pd_fifo_base[0].base0 = 0;
    gpon_pd_fifo_base[0].base1 = gpon_pd_fifo_base[0].base0 + g_gpon_pd_fifo_size[0].size0 + 1;

    epon_pd_fifo_base[0].base0 = 0;
    epon_pd_fifo_base[0].base1 = epon_pd_fifo_base[0].base0 + g_epon_pd_fifo_size[0].size0 + 1;

    for (i = 1; i < TX_QEUEU_PAIRS; i++)
    {
        gpon_pd_fifo_base[i].base0 = gpon_pd_fifo_base[i-1].base1 + g_gpon_pd_fifo_size[i-1].size1 + 1;
        gpon_pd_fifo_base[i].base1 = gpon_pd_fifo_base[i].base0 + g_gpon_pd_fifo_size[i].size0 + 1;   
        
        epon_pd_fifo_base[i].base0 = epon_pd_fifo_base[i-1].base1 + g_epon_pd_fifo_size[i-1].size1 + 1;
        epon_pd_fifo_base[i].base1 = epon_pd_fifo_base[i].base0 + g_epon_pd_fifo_size[i].size0 + 1;     
    }

    xrdp_memset(&config, 0, sizeof(bbh_tx_config));

    config.mac_type = wantype;

    /* cores which sending PDs */
    config.rnr_cfg[0].rnr_src_id = get_runner_idx(us_tm_runner_image);
    config.src_id.stsrnrsrc = get_runner_idx(us_tm_runner_image);

    if (config.mac_type == MAC_TYPE_EPON)
    {
        rc = ag_drv_bbh_tx_dma_epon_urgent_set(BBH_TX_WAN_ID, EPON_URGENT_REQUEST_ENABLE);
        rc = rc ? rc : ag_drv_bbh_tx_sdma_epon_urgent_set(BBH_TX_WAN_ID, EPON_URGENT_REQUEST_DISABLE);

        config.wan_queue_cfg.pd_fifo_base = epon_pd_fifo_base;
        config.wan_queue_cfg.pd_fifo_size = g_epon_pd_fifo_size;
        config.wan_queue_cfg.pd_wkup_threshold = g_epon_pd_wkup_threshold;
        pd_bytes_threshold = g_wan_pd_bytes_threshold_1g;

        config.rnr_cfg[0].tcont_addr = IMAGE_0_BBH_TX_EPON_WAKE_UP_DATA_TABLE_ADDRESS >> 3;
        config.rnr_cfg[0].skb_addr = 0;
        config.rnr_cfg[0].task_number = IMAGE_0_TM_WAN_EPON_THREAD_NUMBER;
        config.rnr_cfg[0].ptr_addr = IMAGE_0_BBH_TX_EPON_EGRESS_COUNTER_TABLE_ADDRESS >> 3;
      
        /* only one core runner for EPON sts */
        config.sts_rnr_cfg[0].tcont_addr = IMAGE_0_US_TM_BBH_TX_WAKE_UP_DATA_TABLE_ADDRESS >> 3;
        config.sts_rnr_cfg[0].task_number = IMAGE_0_TM_WAN_THREAD_NUMBER;
        config.sts_rnr_cfg[0].ptr_addr = IMAGE_0_US_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS >> 3;

        /* enable per queue task number */
        config.per_q_task.task0 = IMAGE_0_TM_WAN_EPON_THREAD_NUMBER;
        config.per_q_task.task1 = IMAGE_0_TM_WAN_EPON_THREAD_NUMBER;
        config.per_q_task.task2 = IMAGE_0_TM_WAN_EPON_THREAD_NUMBER;
        config.per_q_task.task3 = IMAGE_0_TM_WAN_EPON_THREAD_NUMBER;
        config.per_q_task.task4 = IMAGE_0_TM_WAN_EPON_THREAD_NUMBER;
        config.per_q_task.task5 = IMAGE_0_TM_WAN_EPON_THREAD_NUMBER;
        config.per_q_task.task6 = IMAGE_0_TM_WAN_EPON_THREAD_NUMBER;
        config.per_q_task.task7 = IMAGE_0_TM_WAN_EPON_THREAD_NUMBER;
    }
    else
    {
        /* priority for transmitting Q */
        rc = ag_drv_bbh_tx_common_configurations_arb_cfg_set(BBH_TX_WAN_ID, 1);

        config.wan_queue_cfg.pd_fifo_base = gpon_pd_fifo_base;
        config.wan_queue_cfg.pd_fifo_size = g_gpon_pd_fifo_size;
        config.wan_queue_cfg.pd_wkup_threshold = g_gpon_pd_wkup_threshold;
        pd_bytes_threshold = g_wan_pd_bytes_threshold_10g;

        config.rnr_cfg[0].tcont_addr = IMAGE_0_US_TM_BBH_TX_WAKE_UP_DATA_TABLE_ADDRESS >> 3;
        config.rnr_cfg[0].skb_addr = 0;
        config.rnr_cfg[0].task_number = IMAGE_0_TM_WAN_THREAD_NUMBER;
        config.rnr_cfg[0].ptr_addr = IMAGE_0_US_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS >> 3;

        /* enable per queue task number */
        config.per_q_task.task0 = IMAGE_0_TM_WAN_THREAD_NUMBER;
        config.per_q_task.task1 = IMAGE_0_TM_WAN_THREAD_NUMBER;
        config.per_q_task.task2 = IMAGE_0_TM_WAN_THREAD_NUMBER;
        config.per_q_task.task3 = IMAGE_0_TM_WAN_THREAD_NUMBER;
        config.per_q_task.task4 = IMAGE_0_TM_WAN_THREAD_NUMBER;
        config.per_q_task.task5 = IMAGE_0_TM_WAN_THREAD_NUMBER;
        config.per_q_task.task6 = IMAGE_0_TM_WAN_THREAD_NUMBER;
        config.per_q_task.task7 = IMAGE_0_TM_WAN_THREAD_NUMBER;
    }

    /* configurations for reporting core (DBR\GHOST) */
    config.msg_rnr_cfg[0].tcont_addr = IMAGE_0_REPORT_BBH_TX_QUEUE_ID_TABLE_ADDRESS >> 3;
    config.msg_rnr_cfg[0].ptr_addr = IMAGE_0_BBH_TX_EGRESS_REPORT_COUNTER_TABLE_ADDRESS >> 3;
    config.msg_rnr_cfg[0].task_number = IMAGE_0_TM_REPORTING_THREAD_NUMBER;
    config.wan_queue_cfg.queue_to_rnr = g_wan_queue_to_rnr;
    config.wan_queue_cfg.pd_bytes_threshold_en = 1;
    config.wan_queue_cfg.pd_bytes_threshold = pd_bytes_threshold;
    config.wan_queue_cfg.pd_empty_threshold = 1;
    config.src_id.fpmsrc = BB_ID_FPM;
    config.src_id.sbpmsrc = BB_ID_SBPM;
    config.src_id.msgrnrsrc = get_runner_idx(reporting_runner_image);

    config.dma_cfg = &(g_bbh_tx_dma_profile->bbh_tx_dma_cfg[BBH_TX_WAN_ID]);
    config.sdma_cfg = &(g_bbh_tx_sdma_profile->bbh_tx_sdma_cfg[BBH_TX_WAN_ID]);
    config.ddr_cfg = &(g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[BBH_TX_WAN_ID]);

    fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
    GET_ADDR_HIGH_LOW(config.base_addr_high.addr[0], config.base_addr_low.addr[0], fpm_base_phys_addr);

    config.base_addr_low.addr[1] = 0;
    config.base_addr_high.addr[1] = 0;

    rc = rc ? rc : drv_bbh_tx_configuration_set(BBH_TX_WAN_ID, &config);

    max_on_the_fly_reads = (g_max_otf_reads[BBH_TX_WAN_ID] > config.dma_cfg->descsize) ? config.dma_cfg->descsize : g_max_otf_reads[BBH_TX_WAN_ID];
    rc = rc ? rc : ag_drv_bbh_tx_dma_max_otf_read_request_set(BBH_TX_WAN_ID, max_on_the_fly_reads);
    max_on_the_fly_reads = (g_max_otf_reads[BBH_TX_WAN_ID] > config.sdma_cfg->descsize) ? config.sdma_cfg->descsize : g_max_otf_reads[BBH_TX_WAN_ID];
    rc = rc ? rc : ag_drv_bbh_tx_sdma_max_otf_read_request_set(BBH_TX_WAN_ID, max_on_the_fly_reads);
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_dfifoctrl_set(BBH_TX_WAN_ID, BBH_TX_DATA_FIFO_SRAM_SIZE, BBH_TX_DATA_FIFO_DDR_SIZE, BBH_TX_DATA_FIFO_SRAM_BASE);

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize wantype in bbh_tx driver");

    return rc;
}

static int bbh_tx_init(void)
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
    config.rnr_cfg[0].skb_addr = 0;

    config.rnr_cfg[0].rnr_src_id = get_runner_idx(ds_tm_runner_image);
    config.rnr_cfg[0].tcont_addr = IMAGE_0_DS_TM_BBH_TX_WAKE_UP_DATA_TABLE_ADDRESS >> 3;
    config.rnr_cfg[0].task_number = IMAGE_0_TM_TX_TASK_DS_THREAD_NUMBER;
    config.rnr_cfg[0].ptr_addr = IMAGE_0_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS >> 3;

    /* enable per queue task number */
    config.per_q_task.task0 = IMAGE_0_TM_TX_TASK_DS_THREAD_NUMBER;
    config.per_q_task.task1 = IMAGE_0_TM_TX_TASK_DS_THREAD_NUMBER;
    config.per_q_task.task2 = IMAGE_0_TM_TX_TASK_DS_THREAD_NUMBER;
    config.per_q_task.task3 = IMAGE_0_TM_TX_TASK_DS_THREAD_NUMBER;
    config.per_q_task.task4 = IMAGE_0_TM_TX_TASK_DS_THREAD_NUMBER;
    config.per_q_task.task5 = IMAGE_0_TM_TX_TASK_DS_THREAD_NUMBER;
    config.per_q_task.task6 = IMAGE_0_TM_TX_TASK_DS_THREAD_NUMBER;
    config.per_q_task.task7 = IMAGE_0_TM_TX_TASK_DS_THREAD_NUMBER;

    /* CHECK : how many queues to set (q2rnr)?*/
    config.lan_queue_cfg.queue_to_rnr = g_lan_queue_to_rnr;

    if (p_dpi_cfg->bbh_id_gbe_wan != BBH_ID_NULL)
    {
        config.rnr_cfg[1].rnr_src_id = get_runner_idx(us_tm_runner_image);
        config.rnr_cfg[1].tcont_addr = IMAGE_0_US_TM_BBH_TX_WAKE_UP_DATA_TABLE_ADDRESS >> 3;
        config.rnr_cfg[1].ptr_addr = IMAGE_0_US_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS >> 3;
        config.rnr_cfg[1].task_number = IMAGE_0_TM_WAN_THREAD_NUMBER;
        if (p_dpi_cfg->bbh_id_gbe_wan & 0x1)
            config.lan_queue_cfg.queue_to_rnr[(p_dpi_cfg->bbh_id_gbe_wan >> 1)].q1 = 1;
        else
            config.lan_queue_cfg.queue_to_rnr[(p_dpi_cfg->bbh_id_gbe_wan >> 1)].q0 = 1;
        *(((uint8_t *)&config.per_q_task) + p_dpi_cfg->bbh_id_gbe_wan) = IMAGE_0_TM_WAN_THREAD_NUMBER;
    }

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

    config.dma_cfg = &(g_bbh_tx_dma_profile->bbh_tx_dma_cfg[BBH_TX_DS_ID]);
    config.sdma_cfg = &(g_bbh_tx_sdma_profile->bbh_tx_sdma_cfg[BBH_TX_DS_ID]);
    config.ddr_cfg = &(g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[BBH_TX_DS_ID]);
    
    fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
    GET_ADDR_HIGH_LOW(config.base_addr_high.addr[0], config.base_addr_low.addr[0], fpm_base_phys_addr);

    config.base_addr_low.addr[1] = 0;
    config.base_addr_high.addr[1] = 0;

    rc = drv_bbh_tx_configuration_set(BBH_TX_DS_ID, &config);
    rc = rc ? rc : ag_drv_bbh_tx_unified_configurations_eee_set(BBH_TX_DS_ID, 0x3f);

    max_on_the_fly_reads = (g_max_otf_reads[BBH_TX_DS_ID] > config.dma_cfg->descsize) ? config.dma_cfg->descsize : g_max_otf_reads[BBH_TX_DS_ID];
    rc = rc ? rc : ag_drv_bbh_tx_dma_max_otf_read_request_set(BBH_TX_DS_ID, max_on_the_fly_reads);
    max_on_the_fly_reads = (g_max_otf_reads[BBH_TX_DS_ID] > config.sdma_cfg->descsize) ? config.sdma_cfg->descsize : g_max_otf_reads[BBH_TX_DS_ID];
    rc = rc ? rc : ag_drv_bbh_tx_sdma_max_otf_read_request_set(BBH_TX_DS_ID, max_on_the_fly_reads);
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_dfifoctrl_set(BBH_TX_DS_ID, BBH_TX_DATA_FIFO_SRAM_SIZE, BBH_TX_DATA_FIFO_DDR_SIZE, BBH_TX_DATA_FIFO_SRAM_BASE);

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize bbh_tx driver");

    return rc;
}

static int dma_sdma_init(void)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t dma_id, bbh_id, max_read_on_the_fly;
    
    for (dma_id = DMA_ID_FIRST; dma_id < DMA_NUM; dma_id++)
    {
        max_read_on_the_fly = IS_SDMA(dma_id) ? SDMA_MAX_READ_ON_THE_FLY : DMA_MAX_READ_ON_THE_FLY;
        rc = rc ? rc : ag_drv_dma_config_max_otf_set(dma_id, max_read_on_the_fly);
        for (bbh_id = BBH_ID_FIRST; bbh_id < BBH_ID_NUM; bbh_id++)
        {
            if (IS_SDMA(dma_id))
                rc = rc ? rc : ag_drv_dma_config_num_of_writes_set(dma_id, bbh_id, g_bbh_rx_sdma_profile->bbh_rx_sdma_chunks_config[bbh_id].sdma_chunks); 

            if (bbh_id < BBH_TX_BLOCK_COUNT) 
            {
                if (IS_DMA(dma_id))
                {
                    rc = rc ? rc : ag_drv_dma_config_num_of_reads_set(dma_id, bbh_id, g_bbh_tx_dma_profile->bbh_tx_dma_cfg[bbh_id].descsize);
                }
                else
                {
                    rc = rc ? rc : ag_drv_dma_config_num_of_reads_set(dma_id, bbh_id, g_bbh_tx_sdma_profile->bbh_tx_sdma_cfg[bbh_id].descsize); 
                }
            }
            rc = rc ? rc : ag_drv_dma_config_u_thresh_set(dma_id, bbh_id, g_dma_urgent_threshold[bbh_id][dma_id%2].into_urgent_threshold, 
                g_dma_urgent_threshold[bbh_id][dma_id%2].out_of_urgent_threshold);
            rc = rc ? rc : ag_drv_dma_config_pri_set(dma_id, bbh_id, g_dma_strict_priority[bbh_id][dma_id%2].rx_side, 
                g_dma_strict_priority[bbh_id][dma_id%2].tx_side);
            rc = rc ? rc : ag_drv_dma_config_weight_set(dma_id, bbh_id, g_dma_rr_weight[bbh_id][dma_id%2].rx_side, g_dma_rr_weight[bbh_id][dma_id%2].tx_side);
            rc = rc ? rc : ag_drv_dma_config_periph_source_set(dma_id, bbh_id, g_dma_bb_source[bbh_id].rx_side, g_dma_bb_source[bbh_id].tx_side);

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
    bdmf_error_t rc;
    uint8_t ubus_slave_idx, quad_idx, core_index;
    uint32_t addr_hi, addr_lo;
    uint16_t ddr_sop_offset0 = 0, ddr_sop_offset1 = 0;
    bdmf_phys_addr_t fpm_base_phys_addr;
#if !defined(_CFE_) && !defined(RDP_SIM)
    bdmf_phys_addr_t psram_dma_base_phys_addr;
#endif
    RDD_HW_IPTV_CONFIGURATION_DTS iptv_hw_config = {};

    drv_rnr_cores_addr_init();
#ifndef CONFIG_BRCM_IKOS
    drv_rnr_mem_init();
#endif
    drv_qm_update_queue_tables();
    drv_rnr_load_microcode();
    drv_rnr_load_prediction();

    /* Disable dma old flow control 
       DMA will not check read FIFO occupancy when issuing READ requests, 
       relying instead on DMA backpressure mechanism vs read dispatcher block */
    for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
         ag_drv_rnr_regs_cfg_gen_cfg_set(core_index, 1, 0);

    /* scheduler configuration */
    ag_drv_rnr_regs_cfg_sch_cfg_set(get_runner_idx(us_tm_runner_image), DRV_RNR_16SP);
    ag_drv_rnr_regs_cfg_sch_cfg_set(get_runner_idx(cpu_tx_runner_image), DRV_RNR_8SP_8RR);
    ag_drv_rnr_regs_cfg_sch_cfg_set(get_runner_idx(cpu_rx_runner_image), DRV_RNR_8SP_8RR);

    fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
    GET_ADDR_HIGH_LOW(addr_hi, addr_lo, fpm_base_phys_addr);

    g_fpm_hw_cfg.fpm_base_low = addr_lo;
    g_fpm_hw_cfg.fpm_base_high = addr_hi;

    rnr_dma_cfg.ddr.dma_base = (addr_hi << 12) | (addr_lo >> 20);

    lookup_dma_bufsz_by_fpm_bufsz(&p_dpi_cfg->fpm_buf_size, &rnr_dma_cfg.ddr.dma_buf_size); 

    rnr_dma_cfg.ddr.dma_static_offset = 0;
#if defined(_CFE_) || defined(RDP_SIM)
    rnr_dma_cfg.psram.dma_base = ((RU_BLK(PSRAM_MEM).addr[0] + RU_REG_OFFSET(PSRAM_MEM, MEMORY_DATA)) >> 20);
#else
    psram_dma_base_phys_addr = xrdp_virt2phys(&RU_BLK(PSRAM_MEM), 0) + RU_REG_OFFSET(PSRAM_MEM, MEMORY_DATA);
    rnr_dma_cfg.psram.dma_base = psram_dma_base_phys_addr >> 20;
#endif
    rnr_dma_cfg.psram.dma_buf_size = DMA_BUFSIZE_128;
    rnr_dma_cfg.psram.dma_static_offset = 0;

    rc = drv_rnr_dma_cfg(&rnr_dma_cfg);
    if (rc)
    {
        BDMF_TRACE_ERR("drv_rnr_dma_cfg failed, rc %d\n", rc);
        return rc;
    }

    /* ToDo: fill rdd_init_params */

    rdd_init_params.is_basic = is_basic;
    if (!rdd_init_params.is_basic) 
    {
        rc = ag_drv_qm_ddr_sop_offset_get(&ddr_sop_offset0, &ddr_sop_offset1);
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
    /* offset should be int muber of words */
    rdd_init_params.dhd_hw_config.ddr_sop_offset = ((ddr_sop_offset0 + DHD_DATA_OFFSET + 7) >> 3) << 3;
#endif

    rc = rdd_data_structures_init(&rdd_init_params, &iptv_hw_config);

    rc = rc ? rc : rnr_frequency_set(RNR_FREQ_IN_MHZ);
    rc = rc ? rc : parser_init();
    if (rc)
        return rc;

    for (quad_idx = 0; quad_idx < NUM_OF_RNR_QUAD; quad_idx++) 
    {
        rnr_quad_general_config_dma_arb_cfg general_config_dma_arb_cfg = {0, 0, 0, RNR_QUAD_DMA_ARB_CONJEST_THRESHOLD, RNR_QUAD_DMA_ARB_CONJEST_THRESHOLD};
        
        drv_rnr_quad_profiling_quad_init(quad_idx);

        /* change dynamic clock threshold in each quad */
        rc = ag_drv_rnr_quad_general_config_dma_arb_cfg_set(quad_idx, &general_config_dma_arb_cfg);

        /* extend PSRAM slave tokens to 8 */
        for (ubus_slave_idx = 16; !rc && ubus_slave_idx < 20; ubus_slave_idx++) 
            rc = ag_drv_rnr_quad_ext_flowctrl_config_token_val_set(quad_idx, ubus_slave_idx, 8);

        if (rc)
            return rc;
    }
    return rdp_drv_bkpt_init();
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


    /* normal VIQs for bbh-rx - 0*/
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, BB_ID_RX_BBH_0, dsptchr_viq_bbh_target_addr_normal,
        dsptchr_viq_dest_disp, dsptchr_viq_delayed, DISP_REOR_VIQ_BBH_RX0_NORMAL, DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ,
        DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 1);
    
    /* normal VIQs for bbh-rx - 1*/
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, BB_ID_RX_BBH_1, dsptchr_viq_bbh_target_addr_normal,
        dsptchr_viq_dest_disp, dsptchr_viq_delayed, DISP_REOR_VIQ_BBH_RX1_NORMAL, DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ, 
        DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 1);
        
    /* normal VIQs for bbh-rx - 2*/
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, BB_ID_RX_BBH_2, dsptchr_viq_bbh_target_addr_normal,
        dsptchr_viq_dest_disp, dsptchr_viq_delayed, DISP_REOR_VIQ_BBH_RX2_NORMAL, DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ, 
        DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 1);
        
    /* normal VIQs for bbh-rx - 3*/
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, BB_ID_RX_BBH_3, dsptchr_viq_bbh_target_addr_normal,
        dsptchr_viq_dest_disp, dsptchr_viq_delayed, DISP_REOR_VIQ_BBH_RX3_NORMAL, DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ, 
        DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 1);
        
    /* normal VIQs for bbh-rx - 4*/
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, BB_ID_RX_BBH_4, dsptchr_viq_bbh_target_addr_normal,
        dsptchr_viq_dest_disp, dsptchr_viq_delayed, DISP_REOR_VIQ_BBH_RX4_NORMAL, DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ, 
        DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 1);
    
    /* normal BB_ID_RX_PON for bbh-rx WAN - 5*/
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, BB_ID_RX_PON, dsptchr_viq_bbh_target_addr_normal, 
        dsptchr_viq_dest_disp, dsptchr_viq_delayed, DISP_REOR_VIQ_BBH_RX5_NORMAL, DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ, 
        DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 1);
        
    /* exclusive BB_ID_RX_PON for bbh-rx WAN - 6*/
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, BB_ID_RX_PON, dsptchr_viq_bbh_target_addr_excl,
        dsptchr_viq_dest_disp, dsptchr_viq_delayed, DISP_REOR_VIQ_BBH_WAN_EXCL,  DSPTCHR_GUARANTEED_MAX_LIMIT_PER_EXCLUS_VIQ, 
        DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 1);

    /* VIQ for epon tm - 7 */
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, get_runner_idx(us_tm_runner_image), 
        ((IMAGE_0_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3) | (IMAGE_0_TM_WAN_THREAD_NUMBER << 12)), dsptchr_viq_dest_reor, dsptchr_viq_non_delayed, 
        DISP_REOR_VIQ_EPON_TM, DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ, DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 0);

    /* VIQ for CPU_TX egress - 8 */
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, get_runner_idx(cpu_tx_runner_image),
        (IMAGE_2_CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 | IMAGE_2_IMAGE_2_CPU_TX_EGRESS_THREAD_NUMBER << 12), dsptchr_viq_dest_reor, 
        dsptchr_viq_delayed, DISP_REOR_VIQ_CPU_TX_EGRESS, DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ, DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 0);

    /* VIQ for CPU_TX forward - 9 */
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, get_runner_idx(cpu_tx_runner_image), 
        (IMAGE_2_CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 | IMAGE_2_IMAGE_2_CPU_TX_INGRESS_THREAD_NUMBER << 12), dsptchr_viq_dest_disp,
        dsptchr_viq_delayed, DISP_REOR_VIQ_CPU_TX_FORWARD, DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ, DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 0);

    /* VIQ for flush US - 10 */
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, get_runner_idx(us_tm_runner_image) /* us/ds - same runner */, 
        (IMAGE_0_FLUSH_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3) | (IMAGE_0_TM_FLUSH_THREAD_NUMBER << 12), dsptchr_viq_dest_reor, dsptchr_viq_delayed,
        DISP_REOR_VIQ_TM_FLUSH, DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ, DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 0);

    /* VIQ for TX Mirroring - Initialized for US - 11 */
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, get_runner_idx(us_tm_runner_image), 
        ((IMAGE_0_TX_MIRRORING_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3) | (IMAGE_0_TM_WAN_THREAD_NUMBER << 12)), dsptchr_viq_dest_reor, 
        dsptchr_viq_delayed, DISP_REOR_VIQ_TX_MIRRORING, DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ, DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 0);
    
    /* VIQ for CPU_RX_copy - 12 */
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, get_runner_idx(cpu_rx_runner_image),
        (IMAGE_1_CPU_RX_COPY_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 | IMAGE_1_PROCESSING_CPU_RX_COPY_THREAD_NUMBER << 12), dsptchr_viq_dest_reor,
        dsptchr_viq_delayed, DISP_REOR_VIQ_CPU_RX_COPY, DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ, DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 0);
    
    /* Another VIQ for WAN loopback - 13 */
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, get_runner_idx(cpu_rx_runner_image),
        (IMAGE_1_WAN_LOOPBACK_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 | IMAGE_1_PROCESSING_CPU_RX_COPY_THREAD_NUMBER << 12), dsptchr_viq_dest_disp,
        dsptchr_viq_delayed, DISP_REOR_VIQ_WAN_LOOPBACK, DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ, DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 0);

#ifdef CONFIG_DHD_RUNNER
    /* VIQ for DHD_RX_COMPLETE_0 - 14 */
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, get_runner_idx(dhd_complete_runner_image),
        (IMAGE_1_DHD_RX_COMPLETE_0_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 | IMAGE_1_PROCESSING_DHD_RX_COMPLETE_0_THREAD_NUMBER << 12), 
        dsptchr_viq_dest_disp, dsptchr_viq_delayed, DISP_REOR_VIQ_DHD_RX_COMPLETE_0, DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ,
        DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 0);

    /* VIQ for DHD Mcast - 15 */
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, get_runner_idx(dhd_tx_post_runner_image),
        (IMAGE_2_DHD_MCAST_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 | IMAGE_2_IMAGE_2_DHD_MCAST_THREAD_NUMBER << 12), dsptchr_viq_dest_reor, 
        dsptchr_viq_delayed, DISP_REOR_VIQ_DHD_MCAST, DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ, DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 0);
	
    /* VIQ for DHD_TX_COMPLETE_0 - 16 */
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, get_runner_idx(dhd_complete_runner_image), 
        (IMAGE_1_DHD_TX_COMPLETE_0_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 | IMAGE_1_PROCESSING_DHD_TX_COMPLETE_0_THREAD_NUMBER << 12), 
        dsptchr_viq_dest_reor, dsptchr_viq_delayed, DISP_REOR_VIQ_DHD_TX_COMPLETE_0, DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ, 
        DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 0);

#if defined(CONFIG_BCM_SPDSVC_SUPPORT)
    /* VIQ for SPDSVC - 17 */
    dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, get_runner_idx(spdsvc_gen_runner_image),
	    (IMAGE_1_SPDSVC_GEN_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 | IMAGE_1_PROCESSING_SPDSVC_GEN_THREAD_NUMBER << 12),
		dsptchr_viq_dest_reor, dsptchr_viq_delayed, DISP_REOR_VIQ_SPDSVC, DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ,
		DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 0);
#endif

    /* VIQ for SERVICE QUEUES - 18 */
    if  (p_dpi_cfg->number_of_service_queues)
    {
        dispatcher_reorder_viq_init(&cfg, &ingress_congs_init, &egress_congs_init, get_runner_idx(service_queues_runner_image), 
        (IMAGE_0_SERVICE_QUEUES_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 | IMAGE_0_TM_SERVICE_QUEUES_TX_THREAD_NUMBER << 12), dsptchr_viq_dest_reor, 
        dsptchr_viq_delayed, DISP_REOR_VIQ_SERVICE_QUEUES, DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ, DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ, 0);
    }

#endif   
    
    /* configure all rnr groups */
    for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
    {
        /* setting group 0 - processing */
        if (IS_PROCESSING_RUNNER_IMAGE(core_index))
        {
            dispatcher_reorder_rnr_group_init(&cfg, 0, core_index, 0x3F00, offsetof(RDD_PACKET_BUFFER_DTS, pd) >> 3,
                sizeof(RDD_PACKET_BUFFER_DTS) >> 3);
        }

        /* setting group 1 - wan direct */
        if (rdp_core_to_image_map[core_index] == wan_direct_runner_image)
        {
            dispatcher_reorder_rnr_group_init(&cfg, 1, core_index, (1<< IMAGE_1_PROCESSING_WAN_DIRECT_THREAD_NUMBER), offsetof(RDD_PACKET_BUFFER_DTS, pd) >> 3, 
                (sizeof(RDD_PACKET_BUFFER_DTS) >> 3));
        }
    }

    /* mapping viq to rnr group */
#ifndef _CFE_
    cfg.rnr_grp_num = 2;
    cfg.dsptchr_rnr_group_list[0].queues_mask = ((1 << BBH_ID_NUM) - 1) | /* all bbh viq except wan-bbh exclusive viq */
        (1 << DISP_REOR_VIQ_CPU_TX_FORWARD) | (1 << DISP_REOR_VIQ_WAN_LOOPBACK);
#ifdef CONFIG_DHD_RUNNER
    cfg.dsptchr_rnr_group_list[0].queues_mask |=  (1 << DISP_REOR_VIQ_DHD_RX_COMPLETE_0);
#endif        
    cfg.dsptchr_rnr_group_list[1].queues_mask = (1 << DISP_REOR_VIQ_BBH_WAN_EXCL); /* wan-bbh exclusive viq 11 */
#else
    /* send all traffic via "direct flow" */
    cfg.rnr_grp_num = 2;
    cfg.dsptchr_rnr_group_list[0].queues_mask = 0x0;
    cfg.dsptchr_rnr_group_list[1].queues_mask = ((1 << BBH_ID_NUM) - 1) | (1 << DISP_REOR_VIQ_BBH_WAN_EXCL);
#endif

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

    cfg.glbl_congs_init.frst_lvl = cfg.pools_limits.cmn_pool_size * 40 /100;
    cfg.glbl_congs_init.scnd_lvl = cfg.pools_limits.cmn_pool_size - DSPTCHR_RESERVED_PRIORITY_BUFF_NUM;
    cfg.glbl_congs_init.hyst_thrs = DSPTCHR_CONG_PARAM_HYST;
    cfg.glbl_egress_congs_init.frst_lvl = DSPTCHR_CONG_PARAM_EGRESS_GLOBAL - NUM_OF_PROCESSING_TASKS;
    cfg.glbl_egress_congs_init.scnd_lvl = DSPTCHR_CONG_PARAM_EGRESS_GLOBAL;
    cfg.glbl_egress_congs_init.hyst_thrs = DSPTCHR_CONG_PARAM_HYST;

    /* enable all viq except wan-bbh */
    cfg.queue_en_vec = DISP_REOR_XRDP_VIQ_EN((DISP_REOR_VIQ_LAST + 1));

    rc = drv_dis_reor_queues_init();
    rc = rc ? rc : drv_dis_reor_tasks_to_rg_init();
    rc = rc ? rc : drv_dis_reor_free_linked_list_init();
    rc = rc ? rc : drv_dis_reor_cfg(&cfg);
    rc = rc ? rc : ag_drv_dsptchr_load_balancing_lb_cfg_set(1, 2); /*set strict task priority mode in dispatcher*/

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

#ifndef _CFE_
static int natc_init(void)
{
    int rc;
    int i;
    natc_ddr_cfg_natc_ddr_size ddr_size = {ddr_size_32k, ddr_size_32k, ddr_size_8k, ddr_size_32k, ddr_size_32k, ddr_size_32k, ddr_size_32k, ddr_size_32k};
    natc_ddr_cfg_total_len total_len = {total_len_80B, total_len_80B, total_len_80B, total_len_80B, total_len_80B, total_len_80B, total_len_80B, total_len_80B};
    natc_config_t cfg;

    xrdp_memset(&cfg, 0, sizeof(natc_config_t));
    cfg.ctrl_status.nat_hash_mode = hash_mode_crc32;
    cfg.ctrl_status.ddr_hash_mode = hash_mode_crc32_high;
    cfg.ctrl_status.multi_hash_limit = multi_hash_limit_def;

    cfg.ctrl_status.age_timer_tick = timer_tick_packet;
    cfg.ctrl_status.age_timer = age_timer_256_ticks;

    cfg.ctrl_status.cache_update_on_reg_ddr_lookup = 1;
    cfg.ctrl_status.ddr_enable = 1;
    cfg.ctrl_status.natc_enable = 1;
    cfg.ctrl_status.cache_update_on_ddr_miss = 1;

    cfg.tbl_num = natc_tbls_num;

    drv_natc_cfg_build_tables(&cfg, &ddr_size, &total_len, NATC_TABLE_BASE_SIZE_SIZE, p_dpi_cfg->rdp_ddr_rnr_tables_base_virt);
    
    /* marks end of NATC memory */
    natc_mem_end_addr = (void *)((uint8_t *)cfg.tbl_cfg[natc_tbls_num - 1].vir_addr.res +
                                                          cfg.tbl_cfg[natc_tbls_num - 1].res_tbl_size);
    natc_mem_available_size = p_dpi_cfg->rnr_tables_buf_size - (int)(natc_mem_end_addr - p_dpi_cfg->rdp_ddr_rnr_tables_base_virt);
    rc = ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_0_set(ddr_8_bins_per_bucket, ddr_8_bins_per_bucket,
        ddr_8_bins_per_bucket, ddr_8_bins_per_bucket);
    rc = rc ? rc : ag_drv_natc_ddr_cfg_ddr_bins_per_bucket_1_set(ddr_8_bins_per_bucket, ddr_8_bins_per_bucket,
        ddr_8_bins_per_bucket, ddr_8_bins_per_bucket);
    rc = rc ? rc : ag_drv_natc_ddr_cfg_natc_ddr_size_set(&ddr_size);
    rc = rc ? rc : ag_drv_natc_ddr_cfg_total_len_set(&total_len);

    for (i = 0; i< natc_tbls_num; i++)
    {
        rc = rc ? rc : ag_drv_natc_key_mask_tbl_key_mask_set(i, NATC_16BYTE_KEY_MASK);
    }

    return (rc ? rc : drv_natc_init(&cfg));
}
#endif /* _CFE_ */

static int qm_wantype_init(int wantype)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    qm_q_context queue_cfg = {};
    qm_rnr_group_cfg rnr_group_cfg = {};
    qm_epon_overhead_ctrl epon_counter_cfg = {};
    uint32_t idx = 0;

    /* No aggregation in EPON */
    if (wantype == MAC_TYPE_EPON)
    {
        if (drv_qm_get_us_epon_start() == QM_ILLEGAL_QUEUE)
        {
            //there are no empty queues for EPON
            BDMF_TRACE_ERR("there are not enough queues for EPON, please leave at least %d queues empty\n", QM_QUEUE_DYNAMIC_EPON);
            return BDMF_ERR_NORES;
        }
        /* setup 1 queue */
        queue_cfg.wred_profile = 0;                 /* WRED profile */
        queue_cfg.copy_dec_profile = QM_COPY_DEC_PROFILE_LOW; /* Copy decision profile */
        queue_cfg.fpm_ug = FPM_US_UG;               /* FPM UG */
        queue_cfg.ddr_copy_disable = 0;             /* TRUE=never copy to DDR */
        queue_cfg.copy_to_ddr = US_DDR_COPY_ENABLE; /* TRUE=Force copy to DDR */
        queue_cfg.aggregation_disable = 1;          /* TRUE=Disable aggregation */
        queue_cfg.exclusive_priority = 0;           /* TRUE=exclusive priority */
        queue_cfg.q_802_1ae = 0;                    /* TRUE=802.1AE */
        queue_cfg.sci = 0;                          /* TRUE=SCI */
        queue_cfg.fec_enable = 0;                   /* TRUE=enable FEC */
        queue_cfg.res_profile = 0;                  /* Profile=0 : no minimum buffer reservation */

        for (idx = drv_qm_get_us_start(); !rc && idx <= drv_qm_get_us_end(); idx++)
        {
            rc = drv_qm_queue_config(idx, &queue_cfg);
        }

        queue_cfg.wred_profile = 0; /* WRED profile irrelevant in case queue has exclusive_priority */
        queue_cfg.exclusive_priority = 1;           /* TRUE=exclusive priority for 2nd stage epon queues */
        for (idx = drv_qm_get_us_epon_start(); !rc && idx <= drv_qm_get_us_epon_end(); idx++)
        {
            rc = drv_qm_queue_config(idx, &queue_cfg);
        }

        /* us - group 2 (update fifo task) */
        rnr_group_cfg.start_queue = drv_qm_get_us_start(); /* should be from dpi params */
        rnr_group_cfg.end_queue = drv_qm_get_us_end(); /* should be from dpi params */
        rnr_group_cfg.pd_fifo_base = (IMAGE_0_US_TM_PD_FIFO_TABLE_ADDRESS >> 3);
        rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
        rnr_group_cfg.upd_fifo_base = (IMAGE_0_US_TM_UPDATE_FIFO_TABLE_ADDRESS >> 3);
        rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
        rnr_group_cfg.rnr_bb_id = get_runner_idx(us_tm_runner_image);
        rnr_group_cfg.rnr_task = IMAGE_0_TM_UPDATE_FIFO_US_THREAD_NUMBER;
        rnr_group_cfg.rnr_enable = 1;
        rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_2, &rnr_group_cfg);
        /* us - group 1 (epon update fifo task) */
        rnr_group_cfg.start_queue = drv_qm_get_us_epon_start();
        rnr_group_cfg.end_queue = drv_qm_get_us_epon_end();
        rnr_group_cfg.pd_fifo_base = (IMAGE_0_US_TM_PD_FIFO_TABLE_ADDRESS >> 3) + drv_qm_get_us_epon_start() * 2 * 2;
        rnr_group_cfg.rnr_task = IMAGE_0_TM_UPDATE_FIFO_US_EPON_THREAD_NUMBER;
        rnr_group_cfg.upd_fifo_base = (IMAGE_0_EPON_UPDATE_FIFO_TABLE_ADDRESS >> 3);
        rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
        rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
        rnr_group_cfg.rnr_bb_id = get_runner_idx(us_tm_runner_image);
        rnr_group_cfg.rnr_enable = 1;
        rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_1, &rnr_group_cfg);
    }
    else
    {
        /* us - group 1 (update fifo task) */
        rnr_group_cfg.start_queue = drv_qm_get_us_start(); 
        rnr_group_cfg.end_queue = drv_qm_get_us_end(); 
        rnr_group_cfg.pd_fifo_base = (IMAGE_0_US_TM_PD_FIFO_TABLE_ADDRESS >> 3);
        rnr_group_cfg.rnr_task = IMAGE_0_TM_UPDATE_FIFO_US_THREAD_NUMBER;
        rnr_group_cfg.upd_fifo_base = (IMAGE_0_US_TM_UPDATE_FIFO_TABLE_ADDRESS >> 3);
        rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
        rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
        rnr_group_cfg.rnr_bb_id = get_runner_idx(us_tm_runner_image);
        rnr_group_cfg.rnr_enable = 1;
        rc = ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_1, &rnr_group_cfg);
    }

    /* 10G EPON counters init */
    epon_counter_cfg.fec_ipg_length = QM_EPON_IPG_DEFAULT_LENGTH;
    if (wantype == MAC_TYPE_XEPON)
        epon_counter_cfg.epon_line_rate = 1;
    rc = rc ? rc : ag_drv_qm_epon_overhead_ctrl_set(&epon_counter_cfg);

    return rc;
}

static int dqm_init(void)
{
    bdmf_error_t rc;
    bdmf_phys_addr_t fpm_base_phys_addr;
    uint32_t addr_hi, addr_lo, buf_size;

    fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
    GET_ADDR_HIGH_LOW(addr_hi, addr_lo, fpm_base_phys_addr);
    buf_size = g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[BBH_ID_FIRST].bufsize;

    rc = ag_drv_dqm_fpm_addr_set(xrdp_virt2phys(&RU_BLK(FPM), 0) + RU_REG_OFFSET(FPM, POOL1_ALLOC_DEALLOC));
    rc = rc ? rc : ag_drv_dqm_buf_base_set((addr_hi << 24) | (addr_lo >> 8));
    rc = rc ? rc : ag_drv_dqm_buf_size_set(buf_size);

    return rc;
}

static int qm_init(void)
{
    uint32_t addr_hi, addr_lo;
    bdmf_phys_addr_t fpm_base_phys_addr;
    bdmf_error_t rc;
    qm_rnr_group_cfg rnr_group_cfg = {};
    qm_q_context queue_cfg = {};
    uint32_t idx = 0;
    qm_drop_counters_ctrl drop_ctrl = {};

    qm_init_cfg init_cfg =
    {
        .is_close_agg_disable = 1,     /* TRUE=close aggregation timers are disable */
        .is_counters_read_clear = 0,   /* TRUE=read-clear counter access mode */
        .is_drop_counters_enable = 1,  /* TRUE=drop counters enable. FALSE=max occupancy holder */
        .ddr_sop_offset = {0, 18}     /* DDR SoP offsets profile0 - everything except multicast , profile1 - for multicast, */
    };
    fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
    GET_ADDR_HIGH_LOW(addr_hi, addr_lo, fpm_base_phys_addr);

    init_cfg.fpm_base = (addr_hi << 24) | (addr_lo >> 8); /* in 256B resolution */
    init_cfg.fpm_buf_size = g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[BBH_ID_FIRST].bufsize;
    
    rc = drv_qm_init(&init_cfg);
    rc = rc ? rc : drv_qm_system_init(p_dpi_cfg);
	
    /* Aggregation timer (HW aggregation):
     * ------------------
     * prescaler_granularity    :  0 -> 10 bits (1024)
     * aggregation_timeout_value:  7
     * clock                    : 466Mhz (1 tick every 2.146ns)
     * timer:  prescaler_granularity * time_per_tick * aggregation_timeout_value =
     *          1024 * 2.146 * 7 = 15.382us
     */
    /*rc = rc ? rc : ag_drv_qm_global_cfg_qm_aggregation_timer_ctrl_set(0, 7);*/
    
    rc = rc ? rc : ag_drv_qm_qm_pd_cong_ctrl_set(DEF_QM_PD_CONGESTION_CONTROL_THRESHOLD);

    ag_drv_qm_copy_decision_profile_set(QM_COPY_DEC_PROFILE_LOW, QM_COPY_DEC_MAX_QUEUE_OCCUPANCY , 0);
    ag_drv_qm_copy_decision_profile_set(QM_COPY_DEC_PROFILE_HIGH, QM_COPY_DEC_MAX_QUEUE_OCCUPANCY , 1);
     
    /* Initializing DQM before any of QM's queues */
    rc = rc ? rc : dqm_init();

    /* setup 1 queue */
    queue_cfg.wred_profile = 0;                 /* WRED profile */
    queue_cfg.copy_dec_profile = QM_COPY_DEC_PROFILE_LOW; /* Copy decision profile */
    queue_cfg.fpm_ug = FPM_US_UG;               /* FPM UG */
    queue_cfg.ddr_copy_disable = 0;             /* TRUE=never copy to DDR */
    queue_cfg.copy_to_ddr = US_DDR_COPY_ENABLE; /* TRUE=Force copy to DDR */
#ifndef _CFE_
    queue_cfg.aggregation_disable = 0;          /* TRUE=Disable aggregation */
#else
    queue_cfg.aggregation_disable = 1;
#endif
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

    /* Initialize EPON 2'nd stage queues */
    if (drv_qm_get_us_epon_start() != QM_ILLEGAL_QUEUE)
    {
        queue_cfg.exclusive_priority = 1;           /* TRUE=exclusive priority for 2nd stage epon queues */
        for (idx = drv_qm_get_us_epon_start(); idx <= drv_qm_get_us_epon_end(); idx++)
        {
            rc = rc ? rc : drv_qm_queue_config(idx, &queue_cfg);
            rc = rc ? rc : drv_qm_queue_enable(idx);
        }
        queue_cfg.exclusive_priority = 0;           /* reset */
    }
    
    queue_cfg.fpm_ug = FPM_DS_UG;                       /* FPM UG */
    queue_cfg.copy_to_ddr = DS_DDR_COPY_ENABLE;         /* TRUE=Force copy to DDR */
    queue_cfg.aggregation_disable = DS_AGG_DISABLE;     /* TRUE=Disable aggregation */

    for (idx = drv_qm_get_ds_start(); idx <= drv_qm_get_ds_end(); idx++)
    {
        rc = rc ? rc : drv_qm_queue_config(idx, &queue_cfg);
        rc = rc ? rc : drv_qm_queue_enable(idx);
    }

    queue_cfg.aggregation_disable = 1;

    if  (p_dpi_cfg->number_of_service_queues)
    {
        for (idx = drv_qm_get_sq_start(); idx <= drv_qm_get_sq_end(); idx++)
        {
            rc = rc ? rc : drv_qm_queue_config(idx, &queue_cfg);
            rc = rc ? rc : drv_qm_queue_enable(idx);
        }
    }

    queue_cfg.fpm_ug = FPM_ALL_PASS_UG;       /* FPM UG */
    queue_cfg.wred_profile = 0;
    queue_cfg.copy_to_ddr = 0;           /* TRUE=Force copy to DDR */
    queue_cfg.aggregation_disable = 1;   /* TRUE=Disable aggregation */
    for (idx =  QM_QUEUE_MAX_DYNAMIC_QUANTITY; idx < QM_NUM_QUEUES; idx++)
    {
        if ((idx == QM_QUEUE_CPU_RX)  || (idx == QM_QUEUE_CPU_RX_COPY_NORMAL) || (idx == QM_QUEUE_CPU_RX_COPY_EXCLUSIVE))
            continue;
#ifdef CONFIG_DHD_RUNNER
       if (idx == QM_QUEUE_DHD_TX_POST_0)
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


    rc = rc ? rc : drv_qm_queue_config(QM_QUEUE_DHD_TX_POST_0, &queue_cfg);
    rc = rc ? rc : drv_qm_queue_enable(QM_QUEUE_DHD_TX_POST_0);


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
    rnr_group_cfg.start_queue = drv_qm_get_ds_start(); /* should be from dpi params */
    rnr_group_cfg.end_queue = drv_qm_get_ds_end(); /* should be from dpi params */
    rnr_group_cfg.pd_fifo_base = (IMAGE_0_DS_TM_PD_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
    rnr_group_cfg.upd_fifo_base = (IMAGE_0_DS_TM_UPDATE_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
    rnr_group_cfg.rnr_bb_id = get_runner_idx(ds_tm_runner_image);
    rnr_group_cfg.rnr_task = IMAGE_0_TM_UPDATE_FIFO_DS_THREAD_NUMBER;
    rnr_group_cfg.rnr_enable = 1;
    rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_0, &rnr_group_cfg);

    /* cpu - group 3 (cpu_rx fifo task) */
    rnr_group_cfg.start_queue = QM_QUEUE_CPU_RX; /* single queue for CPU RX */
    rnr_group_cfg.end_queue = QM_QUEUE_CPU_RX; 
    rnr_group_cfg.pd_fifo_base = (IMAGE_1_PD_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.rnr_task = IMAGE_1_PROCESSING_CPU_RX_THREAD_NUMBER;
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
    rnr_group_cfg.rnr_task = IMAGE_1_PROCESSING_CPU_RX_COPY_THREAD_NUMBER;
    rnr_group_cfg.upd_fifo_base = (IMAGE_1_CPU_RX_COPY_UPDATE_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
    rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
    rnr_group_cfg.rnr_bb_id = get_runner_idx(cpu_rx_runner_image);
    rnr_group_cfg.rnr_enable = 1;
    rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_6, &rnr_group_cfg);


#ifdef CONFIG_DHD_RUNNER

    /* dhd tx post - group 7 (dhd tx post and cpu tx post fifo task) */
    rnr_group_cfg.start_queue = QM_QUEUE_DHD_CPU_TX_POST_0; /* DHD CPU and TX group */
    rnr_group_cfg.end_queue = QM_QUEUE_DHD_TX_POST_0;
    rnr_group_cfg.pd_fifo_base = (IMAGE_2_DHD_TX_POST_PD_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.rnr_task = IMAGE_2_IMAGE_2_DHD_TX_POST_UPDATE_FIFO_THREAD_NUMBER;
    rnr_group_cfg.upd_fifo_base = (IMAGE_2_DHD_TX_POST_UPDATE_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
    rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
    rnr_group_cfg.rnr_bb_id = get_runner_idx(dhd_tx_post_runner_image);
    rnr_group_cfg.rnr_enable = 1;
    rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_7, &rnr_group_cfg);

    /* dhd mcast fifo task - group 8 */
    rnr_group_cfg.start_queue = QM_QUEUE_DHD_MCAST; /* single queue for DHD Mcast for all 3 radios */
    rnr_group_cfg.end_queue = QM_QUEUE_DHD_MCAST;
    rnr_group_cfg.pd_fifo_base = (IMAGE_2_DHD_MCAST_PD_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.upd_fifo_base = (IMAGE_2_DHD_MCAST_UPDATE_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
    rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;

    rnr_group_cfg.rnr_task = IMAGE_2_IMAGE_2_DHD_MCAST_UPDATE_FIFO_THREAD_NUMBER;

    rnr_group_cfg.rnr_bb_id = get_runner_idx(dhd_tx_post_runner_image);
    rnr_group_cfg.rnr_enable = 1;
    rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_8, &rnr_group_cfg);
      
#endif

    /* service queues group 9 */
    if (p_dpi_cfg->number_of_service_queues)
    {
        rnr_group_cfg.start_queue = drv_qm_get_sq_start(); 
        rnr_group_cfg.end_queue = drv_qm_get_sq_end(); 
        rnr_group_cfg.pd_fifo_base = (IMAGE_0_SERVICE_QUEUES_PD_FIFO_TABLE_ADDRESS >> 3);
        rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
        rnr_group_cfg.upd_fifo_base = (IMAGE_0_SERVICE_QUEUES_UPDATE_FIFO_TABLE_ADDRESS >> 3);
        rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
        rnr_group_cfg.rnr_bb_id = get_runner_idx(service_queues_runner_image);
        rnr_group_cfg.rnr_task = IMAGE_0_TM_SERVICE_QUEUES_UPDATE_FIFO_THREAD_NUMBER;
        rnr_group_cfg.rnr_enable = 1;
        rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_9, &rnr_group_cfg);
    }

    /* Cancel drop qm counters accumulation in HW */
    /* counters accumulation is done in SW */    
    rc = rc ? rc : ag_drv_qm_drop_counters_ctrl_get(&drop_ctrl);
    drop_ctrl.read_clear_bytes = 1;
    drop_ctrl.read_clear_pkts = 1;
    rc = rc ? rc : ag_drv_qm_drop_counters_ctrl_set(&drop_ctrl);

    return rc;
}


#if defined(XRDP_EMULATION)
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
        rc = rc ? rc : ag_drv_unimac_rdp_frm_length_set(idx, p_dpi_cfg->mtu_size);
        /* ignore CRC error */
        ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_rsv_mask_set(idx, 0);
    }
    return rc;
}
#endif

static void rdp_block_enable(void)
{
#ifdef XRDP_EMULATION
    unimac_rdp_cmd cmd;
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

    /* enable TCAM */
    rc = rc ? rc : ag_drv_tcam_op_set(TCAM_CMD_INVALIDATE);

    /* enable RNR */
    for (idx = 0; idx <= RNR_LAST; idx++)
        rc = rc ? rc : ag_drv_rnr_regs_rnr_enable_set(idx, 1);

#ifdef XRDP_EMULATION
    /* enable unimac */
    for (idx = 0; idx < NUM_OF_UNIMAC; idx++)
    {
        rc = rc ? rc : ag_drv_unimac_rdp_cmd_get(idx, &cmd);
        cmd.rx_ena = 1;
        cmd.tx_ena = 1;
        rc = rc ? rc : ag_drv_unimac_rdp_cmd_set(idx, &cmd);
    }
#endif

    if (rc)
        BDMF_TRACE_ERR("Failed to enable rdp blocks\n");
}

uint32_t data_path_init_fiber(int wan_bbh)
{
    int i, rc;

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

    /* RUNNER mactype init */
    BUG_ON(wan_bbh == MAC_TYPE_EMAC);

    if (wan_bbh == MAC_TYPE_EPON)
    {
        rc = rc ? rc : rdd_tm_epon_cfg();
        rc = rc ? rc : rdd_ag_us_tm_bb_destination_table_set(BB_ID_TX_PON_ETH_STAT);
    }
    else
        rc = rc ? rc : rdd_ag_us_tm_bb_destination_table_set(BB_ID_TX_PON_ETH_PD);

    /* configure bbh-tx fifo size */
    if (wan_bbh == MAC_TYPE_EPON || wan_bbh == MAC_TYPE_XEPON)
        rc = rc ? rc : rdd_ag_us_tm_bbh_tx_fifo_size_set(BBH_TX_EPON_PD_FIFO_SIZE_0_7);
    else
    {
        rc = rc ? rc : rdd_ag_us_tm_bbh_tx_fifo_size_set(BBH_TX_GPON_PD_FIFO_SIZE_0_7);
        for (i = drv_qm_get_us_start(); i <= drv_qm_get_us_end(); ++i)
            rdd_ghost_reporting_mapping_queue_to_wan_channel(i, 0);
    }
    if (rc)
        BDMF_TRACE_ERR("Failed to initialize drivers for wantype %d", wan_bbh);

    /* Enable ghost\dbr reporting */
#ifndef _CFE_
    rdd_ghost_reporting_mac_type_init(wan_bbh);
    rc = rc ? rc : rdd_ghost_reporting_timer_set();
#endif

#if defined(CONFIG_DHD_RUNNER)
    if (wan_bbh == MAC_TYPE_XEPON)
    {
        /* 10G/10G EPON case - Re-calculate FPM budget */
        rc = rc ? rc : set_fpm_budget(FPM_RES_XEPON, 1, 0);
    }
    else
    {
        /* Otherwise - return to default */
        rc = rc ? rc : set_fpm_budget(FPM_RES_XEPON, 0, 0);
    }
#endif

    if (wan_bbh == MAC_TYPE_EPON || wan_bbh == MAC_TYPE_GPON)
        rc = rc ? rc : rdd_ag_us_tm_bbh_tx_us_fifo_bytes_threshold_set(BBH_TX_FIFO_BYTES_WAN_1G_LIMIT);
    else
        rc = rc ? rc : rdd_ag_us_tm_bbh_tx_us_fifo_bytes_threshold_set(BBH_TX_FIFO_BYTES_WAN_10G_LIMIT);

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize drivers for wantype %d", wan_bbh);

    return rc;
}

uint32_t data_path_init_gbe(rdpa_emac wan_emac)
{
    int rc;

    /* QM mactype init */
    rc = qm_wantype_init(MAC_TYPE_EMAC);

    /* RUNNER mactype init */
    rc = rc ? rc : rdd_ag_us_tm_bb_destination_table_set(BB_ID_TX_LAN | (wan_emac << 6));

    /* configure bbh-tx fifo size */
    rc = rc ? rc : rdd_ag_us_tm_bbh_tx_fifo_size_set(BBH_TX_DS_PD_FIFO_SIZE_0);
    rc = rc ? rc : rdd_ag_us_tm_bbh_tx_us_fifo_bytes_threshold_set(BBH_TX_FIFO_BYTES_LAN_LIMIT);
    if (rc)
        BDMF_TRACE_ERR("Failed to initialize drivers for GBE, wan_emac %d", wan_emac);
    
#if defined(CONFIG_DHD_RUNNER)
    /* Set UG thresholds to default */
    rc = rc ? rc : set_fpm_budget(FPM_RES_XEPON, 0, 0);
    if (rc)
        BDMF_TRACE_ERR("Failed to Set UG thresholds for GBE, wan_emac %d", wan_emac);
#endif

    return rc;
}

static int _data_path_init(dpi_params_t *dpi_params, int is_basic)
{
    bdmf_error_t rc;
    p_dpi_cfg = dpi_params;

    bdmf_trace("INFO: %s#%d: Start.\n", __FUNCTION__, __LINE__);
#if (defined(RDP_SIM) && (!defined(XRDP_EMULATION)))
    if (rdd_sim_alloc_segments())
        return BDMF_ERR_INTERNAL;
#endif
    ubus_bridge_init();

    rc = bbh_profiles_init();
    if (rc)
    {
        bdmf_trace("Error: %s#%d: Failed to init BBH profiles.\n", __FUNCTION__, __LINE__);
        return rc;
    }

    bdmf_trace("INFO: %s#%d: Driver init.\n", __FUNCTION__, __LINE__);

    rc = fpm_init();
    rc = rc ? rc : qm_init();
    rc = rc ? rc : runner_init(is_basic);
    rc = rc ? rc : bbh_rx_init();
    rc = rc ? rc : bbh_tx_init();
    rc = rc ? rc : sbpm_init();
    rc = rc ? rc : cnpl_init(dpi_params->is_gateway, dpi_params->vlan_stats_enable);
    rc = rc ? rc : dma_sdma_init();
    rc = rc ? rc : dispatcher_reorder_init();
#ifndef _CFE_
    rc = rc ? rc : natc_init();
    rc = rc ? rc : hash_init();
#endif

#if defined(XRDP_EMULATION)
    rc = rc ? rc : unimac_init();
#endif
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

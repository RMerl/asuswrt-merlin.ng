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
#ifdef RDP_SIM
#include "rdd_ip_class.h"
#include "rdp_cpu_ring_sim.h"
#endif
#include "rdd_init.h"
#include "rdd_cpu_rx.h"
#ifndef _CFE_
#include "rdd_tcam_ic.h"
#endif
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
#include "rdd_tuple_lkp.h"
#include "bdmf_data_types.h"
#include "rdd_scheduling.h"
#include "rdd_ghost_reporting.h"
#ifdef XRDP_EMULATION
#include "xrdp_drv_unimac_rdp_ag.h"
#include "xrdp_drv_unimac_misc_ag.h"
#endif

dpi_params_t *p_dpi_cfg;

extern uint32_t total_length_bytes[];

bbh_rx_sdma_profile_t *g_bbh_rx_sdma_profile;
bbh_tx_dma_profile_t *g_bbh_tx_dma_profile;
bbh_tx_sdma_profile_t *g_bbh_tx_sdma_profile;
bbh_tx_ddr_profile_t *g_bbh_tx_ddr_profile;
pd_wkup_threshold_t g_lan_pd_wkup_threshold[2] = { {0, 0}, {0, 0} };
pd_fifo_size_t g_lan_pd_fifo_size[1] = { {BBH_TX_DS_PD_FIFO_SIZE_0, BBH_TX_DS_PD_FIFO_SIZE_1} };
queue_to_rnr_t g_lan_queue_to_rnr[BBH_TX_NUM_OF_LAN_QUEUES_PAIRS] = { {RNR0,RNR0}, {RNR0,RNR0}, {RNR0,RNR0}, {RNR0,RNR0} };
pd_bytes_threshold_t g_lan_pd_bytes_threshold[BBH_TX_NUM_OF_LAN_QUEUES_PAIRS] = { {0,0}, {0,0}, {0,0}, {0,0} };
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
          {BBH_TX_GPON_PD_FIFO_SIZE_0_7/2, BBH_TX_GPON_PD_FIFO_SIZE_0_7/2}, {BBH_TX_GPON_PD_FIFO_SIZE_0_7/2, BBH_TX_GPON_PD_FIFO_SIZE_0_7/2},
          {BBH_TX_GPON_PD_FIFO_SIZE_0_7/2, BBH_TX_GPON_PD_FIFO_SIZE_0_7/2}, {BBH_TX_GPON_PD_FIFO_SIZE_0_7/2, BBH_TX_GPON_PD_FIFO_SIZE_0_7/2},
          {BBH_TX_GPON_PD_FIFO_SIZE_8_15/2, BBH_TX_GPON_PD_FIFO_SIZE_8_15/2}, {BBH_TX_GPON_PD_FIFO_SIZE_8_15/2, BBH_TX_GPON_PD_FIFO_SIZE_8_15/2},
          {BBH_TX_GPON_PD_FIFO_SIZE_8_15/2, BBH_TX_GPON_PD_FIFO_SIZE_8_15/2}, {BBH_TX_GPON_PD_FIFO_SIZE_8_15/2, BBH_TX_GPON_PD_FIFO_SIZE_8_15/2},
          {BBH_TX_GPON_PD_FIFO_SIZE_16_23/2, BBH_TX_GPON_PD_FIFO_SIZE_16_23/2}, {BBH_TX_GPON_PD_FIFO_SIZE_16_23/2, BBH_TX_GPON_PD_FIFO_SIZE_16_23/2},
          {BBH_TX_GPON_PD_FIFO_SIZE_16_23/2, BBH_TX_GPON_PD_FIFO_SIZE_16_23/2}, {BBH_TX_GPON_PD_FIFO_SIZE_16_23/2, BBH_TX_GPON_PD_FIFO_SIZE_16_23/2},
          {BBH_TX_GPON_PD_FIFO_SIZE_24_31/2, BBH_TX_GPON_PD_FIFO_SIZE_24_31/2}, {BBH_TX_GPON_PD_FIFO_SIZE_24_31/2, BBH_TX_GPON_PD_FIFO_SIZE_24_31/2}, 
          {BBH_TX_GPON_PD_FIFO_SIZE_24_31/2, BBH_TX_GPON_PD_FIFO_SIZE_24_31/2}, {BBH_TX_GPON_PD_FIFO_SIZE_24_31/2, BBH_TX_GPON_PD_FIFO_SIZE_24_31/2},
          {BBH_TX_GPON_PD_FIFO_SIZE_32_39/2, BBH_TX_GPON_PD_FIFO_SIZE_32_39/2}, {BBH_TX_GPON_PD_FIFO_SIZE_32_39/2, BBH_TX_GPON_PD_FIFO_SIZE_32_39/2}, 
          {BBH_TX_GPON_PD_FIFO_SIZE_32_39/2, BBH_TX_GPON_PD_FIFO_SIZE_32_39/2}, {BBH_TX_GPON_PD_FIFO_SIZE_32_39/2, BBH_TX_GPON_PD_FIFO_SIZE_32_39/2} };

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
          {BBH_TX_EPON_PD_FIFO_SIZE_0_7/2, BBH_TX_EPON_PD_FIFO_SIZE_0_7/2}, {BBH_TX_EPON_PD_FIFO_SIZE_0_7/2, BBH_TX_EPON_PD_FIFO_SIZE_0_7/2},
          {BBH_TX_EPON_PD_FIFO_SIZE_0_7/2, BBH_TX_EPON_PD_FIFO_SIZE_0_7/2}, {BBH_TX_EPON_PD_FIFO_SIZE_0_7/2, BBH_TX_EPON_PD_FIFO_SIZE_0_7/2},
          {BBH_TX_EPON_PD_FIFO_SIZE_8_15/2, BBH_TX_EPON_PD_FIFO_SIZE_8_15/2}, {BBH_TX_EPON_PD_FIFO_SIZE_8_15/2, BBH_TX_EPON_PD_FIFO_SIZE_8_15/2},
          {BBH_TX_EPON_PD_FIFO_SIZE_8_15/2, BBH_TX_EPON_PD_FIFO_SIZE_8_15/2}, {BBH_TX_EPON_PD_FIFO_SIZE_8_15/2, BBH_TX_EPON_PD_FIFO_SIZE_8_15/2},
          {BBH_TX_EPON_PD_FIFO_SIZE_16_23/2, BBH_TX_EPON_PD_FIFO_SIZE_16_23/2}, {BBH_TX_EPON_PD_FIFO_SIZE_16_23/2, BBH_TX_EPON_PD_FIFO_SIZE_16_23/2},
          {BBH_TX_EPON_PD_FIFO_SIZE_16_23/2, BBH_TX_EPON_PD_FIFO_SIZE_16_23/2}, {BBH_TX_EPON_PD_FIFO_SIZE_16_23/2, BBH_TX_EPON_PD_FIFO_SIZE_16_23/2},
          {BBH_TX_EPON_PD_FIFO_SIZE_24_31/2, BBH_TX_EPON_PD_FIFO_SIZE_24_31/2}, {BBH_TX_EPON_PD_FIFO_SIZE_24_31/2, BBH_TX_EPON_PD_FIFO_SIZE_24_31/2}, 
          {BBH_TX_EPON_PD_FIFO_SIZE_24_31/2, BBH_TX_EPON_PD_FIFO_SIZE_24_31/2}, {BBH_TX_EPON_PD_FIFO_SIZE_24_31/2, BBH_TX_EPON_PD_FIFO_SIZE_24_31/2},
          {BBH_TX_EPON_PD_FIFO_SIZE_32_39/2, BBH_TX_EPON_PD_FIFO_SIZE_32_39/2}, {BBH_TX_EPON_PD_FIFO_SIZE_32_39/2, BBH_TX_EPON_PD_FIFO_SIZE_32_39/2}, 
          {BBH_TX_EPON_PD_FIFO_SIZE_32_39/2, BBH_TX_EPON_PD_FIFO_SIZE_32_39/2}, {BBH_TX_EPON_PD_FIFO_SIZE_32_39/2, BBH_TX_EPON_PD_FIFO_SIZE_32_39/2} };

bbh_to_dma_x_t  g_bbh_to_dma_x[BBH_ID_NUM] = { 
          {BBH_ID_0, DMA0_ID},
          {BBH_ID_1, DMA0_ID},
          {BBH_ID_2, DMA0_ID},
          {BBH_ID_3, DMA0_ID},
          {BBH_ID_4, DMA0_ID},
          {BBH_ID_5, DMA0_ID},
          {BBH_ID_PON, DMA0_ID} };

bbh_id_e g_dma_to_bbh_x[NUM_OF_DMA][NUM_OF_PERIPHERALS_PER_DMA] = {
          {BBH_ID_0, BBH_ID_1, BBH_ID_2, BBH_ID_3, BBH_ID_4, BBH_ID_5, BBH_ID_PON} };
          
/* array desription:
      - this array describe the buffers weight accroding to g_dma_to_bbh_X array.
      - total weight is 32 for each DMA (four BBH IDs) */
uint8_t g_bbh_buff_num[NUM_OF_DMA][NUM_OF_PERIPHERALS_PER_DMA] = {{6, 6, 6, 6, 6, 6, 12 }};

bb_source_t g_dma_bb_source[BBH_ID_NUM] = {
         {BB_ID_RX_BBH_0, BB_ID_TX_LAN},
         {BB_ID_RX_BBH_1, BB_ID_TX_PON_ETH_PD},
         {BB_ID_RX_BBH_2, BB_ID_TX_LAN},
         {BB_ID_RX_BBH_3, BB_ID_TX_LAN}, 
         {BB_ID_RX_BBH_4, BB_ID_TX_LAN},
         {BB_ID_RX_BBH_5, BB_ID_TX_LAN},
         {BB_ID_RX_PON, BB_ID_TX_LAN} };

uint8_t g_max_otf_reads[BBH_ID_NUM] = 
         {MAX_OTF_READ_REQUEST_DEFAULT_DMA0, MAX_OTF_READ_REQUEST_DEFAULT_DMA0,
          MAX_OTF_READ_REQUEST_DEFAULT_DMA0, MAX_OTF_READ_REQUEST_DEFAULT_DMA0,
          MAX_OTF_READ_REQUEST_DEFAULT_DMA0, MAX_OTF_READ_REQUEST_DEFAULT_DMA0,
          MAX_OTF_READ_REQUEST_DEFAULT_DMA0};

urgent_threhold_t g_dma_urgent_threshold[BBH_ID_NUM][2] = {
         { {DMA1_U_THRESH_IN_BBH_ID_XLMAC0_0_10G_VALUE, DMA1_U_THRESH_OUT_BBH_ID_XLMAC0_0_10G_VALUE},
           {SDMA1_U_THRESH_IN_BBH_ID_XLMAC0_0_10G_VALUE, SDMA1_U_THRESH_OUT_BBH_ID_XLMAC0_0_10G_VALUE} },
         { {DMA0_U_THRESH_IN_BBH_ID_XLMAC0_1_2p5G_VALUE, DMA0_U_THRESH_OUT_BBH_ID_XLMAC0_1_2p5G_VALUE},
           {SDMA0_U_THRESH_IN_BBH_ID_XLMAC0_1_2p5G_VALUE, SDMA0_U_THRESH_OUT_BBH_ID_XLMAC0_1_2p5G_VALUE} },
         { {DMA1_U_THRESH_IN_BBH_ID_XLMAC0_2_1G_VALUE, DMA1_U_THRESH_OUT_BBH_ID_XLMAC0_2_1G_VALUE},
           {SDMA1_U_THRESH_IN_BBH_ID_XLMAC0_2_1G_VALUE, SDMA1_U_THRESH_OUT_BBH_ID_XLMAC0_2_1G_VALUE} },
         { {DMA1_U_THRESH_IN_BBH_ID_XLMAC0_3_1G_VALUE, DMA1_U_THRESH_OUT_BBH_ID_XLMAC0_3_1G_VALUE}, 
           {SDMA1_U_THRESH_IN_BBH_ID_XLMAC0_3_1G_VALUE, SDMA1_U_THRESH_OUT_BBH_ID_XLMAC0_3_1G_VALUE} },
         { {DMA1_U_THRESH_IN_BBH_ID_XLMAC1_0_RGMII_VALUE, DMA1_U_THRESH_OUT_BBH_ID_XLMAC1_0_RGMII_VALUE},
           {SDMA1_U_THRESH_IN_BBH_ID_XLMAC1_0_RGMII_VALUE, SDMA1_U_THRESH_OUT_BBH_ID_XLMAC1_0_RGMII_VALUE} },
         { {DMA0_U_THRESH_IN_BBH_ID_XLMAC1_1_RGMII_VALUE, DMA0_U_THRESH_OUT_BBH_ID_XLMAC1_1_RGMII_VALUE},
           {SDMA0_U_THRESH_IN_BBH_ID_XLMAC1_1_RGMII_VALUE, SDMA0_U_THRESH_OUT_BBH_ID_XLMAC1_1_RGMII_VALUE} },
         { {DMA0_U_THRESH_IN_BBH_ID_PON_VALUE, DMA0_U_THRESH_OUT_BBH_ID_PON_VALUE},
           {SDMA0_U_THRESH_IN_BBH_ID_PON_VALUE, SDMA0_U_THRESH_OUT_BBH_ID_PON_VALUE} } };

strict_priority_t g_dma_strict_priority[BBH_ID_NUM][2] = {
         { {DMA1_STRICT_PRI_RX_BBH_ID_XLMAC0_0_10G_VALUE, DMA1_STRICT_PRI_TX_BBH_ID_XLMAC0_0_10G_VALUE},
           {SDMA1_STRICT_PRI_RX_BBH_ID_XLMAC0_0_10G_VALUE, SDMA1_STRICT_PRI_TX_BBH_ID_XLMAC0_0_10G_VALUE} },
         { {DMA0_STRICT_PRI_RX_BBH_ID_XLMAC0_1_2p5G_VALUE, DMA0_STRICT_PRI_TX_BBH_ID_XLMAC0_1_2p5G_VALUE},
           {SDMA0_STRICT_PRI_RX_BBH_ID_XLMAC0_1_2p5G_VALUE, SDMA0_STRICT_PRI_TX_BBH_ID_XLMAC0_1_2p5G_VALUE} },
         { {DMA1_STRICT_PRI_RX_BBH_ID_XLMAC0_2_1G_VALUE, DMA1_STRICT_PRI_TX_BBH_ID_XLMAC0_2_1G_VALUE},
           {SDMA1_STRICT_PRI_RX_BBH_ID_XLMAC0_2_1G_VALUE, SDMA1_STRICT_PRI_TX_BBH_ID_XLMAC0_2_1G_VALUE} },
         { {DMA1_STRICT_PRI_RX_BBH_ID_XLMAC0_3_1G_VALUE, DMA1_STRICT_PRI_TX_BBH_ID_XLMAC0_3_1G_VALUE}, 
           {SDMA1_STRICT_PRI_RX_BBH_ID_XLMAC0_3_1G_VALUE, SDMA1_STRICT_PRI_TX_BBH_ID_XLMAC0_3_1G_VALUE} },
         { {DMA1_STRICT_PRI_RX_BBH_ID_XLMAC1_0_RGMII_VALUE, DMA1_STRICT_PRI_TX_BBH_ID_XLMAC1_0_RGMII_VALUE},
           {SDMA1_STRICT_PRI_RX_BBH_ID_XLMAC1_0_RGMII_VALUE, SDMA1_STRICT_PRI_TX_BBH_ID_XLMAC1_0_RGMII_VALUE} },
         { {DMA0_STRICT_PRI_RX_BBH_ID_XLMAC1_1_RGMII_VALUE, DMA0_STRICT_PRI_TX_BBH_ID_XLMAC1_1_RGMII_VALUE},
           {SDMA0_STRICT_PRI_RX_BBH_ID_XLMAC1_1_RGMII_VALUE, SDMA0_STRICT_PRI_TX_BBH_ID_XLMAC1_1_RGMII_VALUE} },
         { {DMA0_STRICT_PRI_RX_BBH_ID_PON_VALUE, DMA0_STRICT_PRI_TX_BBH_ID_PON_VALUE},
           {SDMA0_STRICT_PRI_RX_BBH_ID_PON_VALUE, SDMA0_STRICT_PRI_TX_BBH_ID_PON_VALUE} } };
         
rr_weight_t g_dma_rr_weight[BBH_ID_NUM][2] = {
         { {DMA1_RR_WEIGHT_RX_BBH_ID_XLMAC0_0_10G_VALUE, DMA1_RR_WEIGHT_TX_BBH_ID_XLMAC0_0_10G_VALUE},
           {SDMA1_RR_WEIGHT_RX_BBH_ID_XLMAC0_0_10G_VALUE, SDMA1_RR_WEIGHT_TX_BBH_ID_XLMAC0_0_10G_VALUE} },
         { {DMA0_RR_WEIGHT_RX_BBH_ID_XLMAC0_1_2p5G_VALUE, DMA0_RR_WEIGHT_TX_BBH_ID_XLMAC0_1_2p5G_VALUE},
           {SDMA0_RR_WEIGHT_RX_BBH_ID_XLMAC0_1_2p5G_VALUE, SDMA0_RR_WEIGHT_TX_BBH_ID_XLMAC0_1_2p5G_VALUE} },
         { {DMA1_RR_WEIGHT_RX_BBH_ID_XLMAC0_2_1G_VALUE, DMA1_RR_WEIGHT_TX_BBH_ID_XLMAC0_2_1G_VALUE},
           {SDMA1_RR_WEIGHT_RX_BBH_ID_XLMAC0_2_1G_VALUE, SDMA1_RR_WEIGHT_TX_BBH_ID_XLMAC0_2_1G_VALUE} },
         { {DMA1_RR_WEIGHT_RX_BBH_ID_XLMAC0_3_1G_VALUE, DMA1_RR_WEIGHT_TX_BBH_ID_XLMAC0_3_1G_VALUE}, 
           {SDMA1_RR_WEIGHT_RX_BBH_ID_XLMAC0_3_1G_VALUE, SDMA1_RR_WEIGHT_TX_BBH_ID_XLMAC0_3_1G_VALUE} },
         { {DMA1_RR_WEIGHT_RX_BBH_ID_XLMAC1_0_RGMII_VALUE, DMA1_RR_WEIGHT_TX_BBH_ID_XLMAC1_0_RGMII_VALUE},
           {SDMA1_RR_WEIGHT_RX_BBH_ID_XLMAC1_0_RGMII_VALUE, SDMA1_RR_WEIGHT_TX_BBH_ID_XLMAC1_0_RGMII_VALUE} },
         { {DMA0_RR_WEIGHT_RX_BBH_ID_XLMAC1_1_RGMII_VALUE, DMA0_RR_WEIGHT_TX_BBH_ID_XLMAC1_1_RGMII_VALUE},
           {SDMA0_RR_WEIGHT_RX_BBH_ID_XLMAC1_1_RGMII_VALUE, SDMA0_RR_WEIGHT_TX_BBH_ID_XLMAC1_1_RGMII_VALUE} },
         { {DMA0_RR_WEIGHT_RX_BBH_ID_PON_VALUE, DMA0_RR_WEIGHT_TX_BBH_ID_PON_VALUE},
           {SDMA0_RR_WEIGHT_RX_BBH_ID_PON_VALUE, SDMA0_RR_WEIGHT_TX_BBH_ID_PON_VALUE} } };

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
        bbh_tx_ddr_cfg->bufsize = BUF_4K;
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
    bdmf_error_t rc = BDMF_ERR_OK;
    bbh_rx_config cfg;

    xrdp_memset(&cfg, 0, sizeof(bbh_rx_config));

    cfg.sdma_chunks_cfg = &(g_bbh_rx_sdma_profile->bbh_rx_sdma_chunks_config[bbh_id]);
    cfg.disp_bb_id = BB_ID_DISPATCHER_REORDER;
    cfg.sbpm_bb_id = BB_ID_SBPM;

    /* TODO: should match other blocks configurations (runner) */
    /* TODO: should it be a profile as well ? */
    cfg.normal_viq = bbh_id;
    cfg.excl_viq = BBH_ID_NUM + bbh_id;
    cfg.excl_cfg.exc_en = 1;

    for (i = 0; i < 4; i++)
    {
        if (IS_WAN_PORT(bbh_id))
            cfg.min_pkt_size[i] = MIN_WAN_PKT_SIZE;
        else
            cfg.min_pkt_size[i] = MIN_ETH_PKT_SIZE;
        cfg.max_pkt_size[i] = p_dpi_cfg->mtu_size;
    }

    if (IS_WAN_PORT(bbh_id))
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

    rc = rc ? rc : drv_bbh_rx_configuration_set(bbh_id, &cfg);
#ifndef RDP_SIM
    /* initialize all flows (including the 2 groups) */
    for (i = 0; i < DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_0; i++)
    {
        rc = rc ? rc : ag_drv_bbh_rx_min_pkt_sel_flows_0_31_set(bbh_id, i, 0);
        rc = rc ? rc : ag_drv_bbh_rx_max_pkt_sel_flows_0_31_set(bbh_id, i, 0);
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
        (wan_bbh == MAC_TYPE_XGPON || wan_bbh == MAC_TYPE_XEPON));
    
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

static int bbh_rx_init(void)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    bbh_id_e bbh_id;

    for (bbh_id = BBH_ID_FIRST; bbh_id < BBH_ID_PON; bbh_id++)
    {
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
}

static int fpm_init(void)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    bdmf_boolean reset_req = 1;
    fpm_pool2_intr_sts interrupt_status = FPM_INTERRUPT_STATUS;
    fpm_pool2_intr_msk interrupt_mask = FPM_INTERRUPT_MASK;
    uint16_t timeout = 0;

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
#ifndef XRDP_EMULATION
    /* fpm interrupts */
    rc = rc ? rc : ag_drv_fpm_pool1_intr_sts_set(&interrupt_status);
    rc = rc ? rc : ag_drv_fpm_pool1_intr_msk_set(&interrupt_mask);
#endif

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize fpm driver\n");

    return rc;
}

static int cnpl_init(void)
{
    int rc;

    /* reset counters-policers memory */
    rc = drv_cnpl_memory_data_init();
    /* init counter groups according to project */
    rc = rc ? rc : drv_cntr_group_init();
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
        rc = rc ? rc : ag_drv_rnr_quad_tcp_flags_set(i, PARSER_TCP_CTL_FLAGS);
        rc = rc ? rc : ag_drv_rnr_quad_exception_bits_set(i, PARSER_EXCP_STATUS_BITS);
        rc = rc ? rc : ag_drv_rnr_quad_parser_core_configuration_ppp_ip_prot_code_set(i,
            PARSER_PPP_PROTOCOL_CODE_0_IPV4, PARSER_PPP_PROTOCOL_CODE_1_IPV6);
        rc = rc ? rc : ag_drv_rnr_quad_parser_core_configuration_eng_set(i, PARSER_AH_DETECTION);
    }

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize parser driver\n");

    return rc;
}

#if 0
#ifndef _CFE_
static int hash_init(void)
{
    hash_config_t hash_cfg = {};

    hash_cfg.tbl_num = 1;

    /* IPTV table configuration */
    hash_cfg.tbl_cfg[0].tbl_size = hash_max_256_entries_per_engine;
    hash_cfg.tbl_cfg[0].cam_en = 0;
    hash_cfg.tbl_init[0].int_ctx_size = 3;
    hash_cfg.tbl_init[0].ext_ctx_size = 0;
    hash_cfg.tbl_init[0].search_depth_per_engine = 4;
    /* 32 bit key */
    hash_cfg.mask[0].maskl = 0xFFFFFFFF;
    /* Only 12 bits of internal context are used*/
    hash_cfg.mask[0].maskh = 0x000FFFF;

    /* reset hash memory */
    return drv_hash_init(&hash_cfg);
}
#endif /*_CFE_*/
#endif

static int sbpm_init(void)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint16_t base_addr;
    uint16_t init_offset;

    /* base address and offset */
    base_addr = SBPM_BASE_ADDRESS;
    init_offset = SBPM_INIT_OFFSET;

    rc = rc ? rc : ag_drv_sbpm_regs_init_free_list_set(base_addr, init_offset, 0, 0);

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
    bdmf_error_t rc = BDMF_ERR_OK;
    pd_fifo_base_t gpon_pd_fifo_base[TX_QEUEU_PAIRS] = {}; 
    pd_fifo_base_t epon_pd_fifo_base[TX_QEUEU_PAIRS] = {};

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
        rc = rc ? rc : ag_drv_bbh_tx_dma_epon_urgent_set(BBH_ID_PON, EPON_URGENT_REQUEST_ENABLE);
        rc = rc ? rc : ag_drv_bbh_tx_sdma_epon_urgent_set(BBH_ID_PON, EPON_URGENT_REQUEST_DISABLE);

        config.wan_queue_cfg.pd_fifo_base = epon_pd_fifo_base;
        config.wan_queue_cfg.pd_fifo_size = g_epon_pd_fifo_size;
        config.wan_queue_cfg.pd_wkup_threshold = g_epon_pd_wkup_threshold;

        config.rnr_cfg[0].tcont_addr = IMAGE_3_BBH_TX_EPON_QUEUE_ID_TABLE_ADDRESS >> 3;
        config.rnr_cfg[0].skb_addr = 0;
        config.rnr_cfg[0].task_number = IMAGE_3_US_TM_WAN_EPON_THREAD_NUMBER;
        config.rnr_cfg[0].ptr_addr = IMAGE_3_BBH_TX_EPON_EGRESS_COUNTER_TABLE_ADDRESS >> 3;
      
        /* only one core runner for EPON sts */
        config.sts_rnr_cfg[0].tcont_addr = IMAGE_3_BBH_TX_QUEUE_ID_TABLE_ADDRESS >> 3;
        config.sts_rnr_cfg[0].task_number = IMAGE_3_US_TM_WAN_THREAD_NUMBER;
        config.sts_rnr_cfg[0].ptr_addr = IMAGE_3_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS >> 3;
    }
    else
    {
        /* priority for transmitting Q */
        rc = rc ? rc : ag_drv_bbh_tx_common_configurations_arb_cfg_set(BBH_ID_PON, 1);

        config.wan_queue_cfg.pd_fifo_base = gpon_pd_fifo_base;
        config.wan_queue_cfg.pd_fifo_size = g_gpon_pd_fifo_size;
        config.wan_queue_cfg.pd_wkup_threshold = g_gpon_pd_wkup_threshold;

        config.rnr_cfg[0].tcont_addr = IMAGE_3_BBH_TX_QUEUE_ID_TABLE_ADDRESS >> 3;
        config.rnr_cfg[0].skb_addr = 0;
        config.rnr_cfg[0].task_number = IMAGE_3_US_TM_WAN_THREAD_NUMBER;
        config.rnr_cfg[0].ptr_addr = IMAGE_3_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS >> 3;
    }

    /* configurations for reporting core (DBR\GHOST) */
    config.msg_rnr_cfg[0].tcont_addr = IMAGE_2_BBH_TX_QUEUE_ID_TABLE_ADDRESS >> 3;
    config.msg_rnr_cfg[0].ptr_addr = IMAGE_2_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS >> 3;
    config.msg_rnr_cfg[0].task_number = IMAGE_2_CPU_IF_2_REPORTING_THREAD_NUMBER;
    config.wan_queue_cfg.queue_to_rnr = g_wan_queue_to_rnr;
    config.wan_queue_cfg.pd_bytes_threshold_en = 0; 
    config.wan_queue_cfg.pd_empty_threshold = 1;
    config.src_id.fpmsrc = BB_ID_FPM;
    config.src_id.sbpmsrc = BB_ID_SBPM;
    config.src_id.msgrnrsrc = get_runner_idx(reporting_runner_image);

    config.dma_cfg = &(g_bbh_tx_dma_profile->bbh_tx_dma_cfg[BBH_ID_PON]);
    config.sdma_cfg = &(g_bbh_tx_sdma_profile->bbh_tx_sdma_cfg[BBH_ID_PON]);
    config.ddr_cfg = &(g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[BBH_ID_PON]);

    fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
    GET_ADDR_HIGH_LOW(config.base_addr_high.addr[0], config.base_addr_low.addr[0], fpm_base_phys_addr);

    config.base_addr_low.addr[1] = 0;
    config.base_addr_high.addr[1] = 0;

    rc = rc ? rc : drv_bbh_tx_configuration_set(BBH_ID_PON, &config);

    max_on_the_fly_reads = (g_max_otf_reads[BBH_ID_PON] > config.dma_cfg->descsize) ? config.dma_cfg->descsize : g_max_otf_reads[BBH_ID_PON];
    rc = rc ? rc : ag_drv_bbh_tx_dma_max_otf_read_request_set(BBH_ID_PON, max_on_the_fly_reads);
    max_on_the_fly_reads = (g_max_otf_reads[BBH_ID_PON] > config.sdma_cfg->descsize) ? config.sdma_cfg->descsize : g_max_otf_reads[BBH_ID_PON];
    rc = rc ? rc : ag_drv_bbh_tx_sdma_max_otf_read_request_set(BBH_ID_PON, max_on_the_fly_reads);
    rc = rc ? rc : ag_drv_bbh_tx_common_configurations_dfifoctrl_set(BBH_ID_PON, BBH_TX_DATA_FIFO_SRAM_SIZE, BBH_TX_DATA_FIFO_DDR_SIZE, BBH_TX_DATA_FIFO_SRAM_BASE);

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize wantype in bbh_tx driver");

    return rc;
}

static int bbh_tx_init()
{
    bbh_id_e bbh_id;
    bbh_tx_config config;
    uint8_t max_on_the_fly_reads;
    bdmf_phys_addr_t fpm_base_phys_addr;
    bdmf_error_t rc = BDMF_ERR_OK;
    pd_fifo_base_t lan_pd_fifo_base[1] = {};

    /* init fifo base arrays */
    lan_pd_fifo_base[0].base0 = 0;
    lan_pd_fifo_base[0].base1 = lan_pd_fifo_base[0].base0 + g_lan_pd_fifo_size[0].size0 + 1;

    for (bbh_id = 0; bbh_id < BBH_ID_PON; bbh_id++)
    {
        xrdp_memset(&config, 0, sizeof(bbh_tx_config));

        /* cores which sending PDs */
        config.mac_type = MAC_TYPE_GPON; /* The unified BBH TX is based on GPON BBH TX */
        config.rnr_cfg[0].skb_addr = 0;
        config.rnr_cfg[0].rnr_src_id = get_runner_idx(ds_tm_runner_image);
        config.rnr_cfg[0].tcont_addr = IMAGE_0_BBH_TX_QUEUE_ID_TABLE_ADDRESS >> 3;
        config.rnr_cfg[0].task_number = IMAGE_0_DS_TM_TX_TASK_THREAD_NUMBER;
        config.rnr_cfg[0].ptr_addr = IMAGE_0_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS >> 3;

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

        config.dma_cfg = &(g_bbh_tx_dma_profile->bbh_tx_dma_cfg[bbh_id]);
        config.sdma_cfg = &(g_bbh_tx_sdma_profile->bbh_tx_sdma_cfg[bbh_id]);
        config.ddr_cfg = &(g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[bbh_id]);
        
        fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
        GET_ADDR_HIGH_LOW(config.base_addr_high.addr[0], config.base_addr_low.addr[0], fpm_base_phys_addr);

        config.base_addr_low.addr[1] = 0;
        config.base_addr_high.addr[1] = 0;

        rc = rc ? rc : drv_bbh_tx_configuration_set(bbh_id, &config);

        max_on_the_fly_reads = (g_max_otf_reads[bbh_id] > config.dma_cfg->descsize) ? config.dma_cfg->descsize : g_max_otf_reads[bbh_id];
        rc = rc ? rc : ag_drv_bbh_tx_dma_max_otf_read_request_set(bbh_id, max_on_the_fly_reads);
        max_on_the_fly_reads = (g_max_otf_reads[bbh_id] > config.sdma_cfg->descsize) ? config.sdma_cfg->descsize : g_max_otf_reads[bbh_id];
        rc = rc ? rc : ag_drv_bbh_tx_sdma_max_otf_read_request_set(bbh_id, max_on_the_fly_reads);
        rc = rc ? rc : ag_drv_bbh_tx_common_configurations_dfifoctrl_set(bbh_id, BBH_TX_DATA_FIFO_SRAM_SIZE, BBH_TX_DATA_FIFO_DDR_SIZE, BBH_TX_DATA_FIFO_SRAM_BASE);
        if (rc)
            break;
    }

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize bbh_tx driver");

    return rc;
}

static int dma_sdma_init(void)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t dma_id, peripheral_id, bbh_id, max_read_on_the_fly;
    
    for (dma_id = DMA_ID_FIRST; dma_id < DMA_NUM; dma_id++)
    {
        max_read_on_the_fly = IS_SDMA(dma_id) ? SDMA_MAX_READ_ON_THE_FLY : DMA_MAX_READ_ON_THE_FLY;
        rc = rc ? rc : ag_drv_dma_config_max_otf_set(dma_id, max_read_on_the_fly);
        peripheral_id = 0;
        for (bbh_id = BBH_ID_FIRST; bbh_id < BBH_ID_NUM; bbh_id++)
        {
            if (IS_SDMA(dma_id))
                rc = rc ? rc : ag_drv_dma_config_num_of_writes_set(dma_id, peripheral_id, g_bbh_rx_sdma_profile->bbh_rx_sdma_chunks_config[bbh_id].sdma_chunks); 
                         
            if (IS_DMA(dma_id))
            {
                rc = rc ? rc : ag_drv_dma_config_num_of_reads_set(dma_id, peripheral_id, g_bbh_tx_dma_profile->bbh_tx_dma_cfg[bbh_id].descsize);
            }
            else
            {
                rc = rc ? rc : ag_drv_dma_config_num_of_reads_set(dma_id, peripheral_id, g_bbh_tx_sdma_profile->bbh_tx_sdma_cfg[bbh_id].descsize); 
            }

            rc = rc ? rc : ag_drv_dma_config_u_thresh_set(dma_id, peripheral_id, g_dma_urgent_threshold[bbh_id][dma_id%2].into_urgent_threshold, 
                g_dma_urgent_threshold[bbh_id][dma_id%2].out_of_urgent_threshold);
            rc = rc ? rc : ag_drv_dma_config_pri_set(dma_id, peripheral_id, g_dma_strict_priority[bbh_id][dma_id%2].rx_side, 
                g_dma_strict_priority[bbh_id][dma_id%2].tx_side);
            rc = rc ? rc : ag_drv_dma_config_weight_set(dma_id, peripheral_id, g_dma_rr_weight[bbh_id][dma_id%2].rx_side, g_dma_rr_weight[bbh_id][dma_id%2].tx_side);
            rc = rc ? rc : ag_drv_dma_config_periph_source_set(dma_id, peripheral_id, g_dma_bb_source[bbh_id].rx_side, g_dma_bb_source[bbh_id].tx_side);

            peripheral_id++;

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

/*******************************************************************************/
/* Todo: */
/* remove when rdpa_iptv_ex is check in */

#define MAC_ADDR_LOW(addr)     ((uint64_t)(addr) & 0xFFFFFFFF)
#define MAC_ADDR_HIGH(addr)    ((uint64_t)(addr) >> 32)
#define IPV4_MCAST_MAC_ADDR_PREFIX  (0x01005E000000)
#define IPV4_MCAST_MAC_ADDR_MASK    (0xFFFFFF800000)
#define IPV6_MCAST_MAC_ADDR_PREFIX  (0x333300000000)
#define IPV6_MCAST_MAC_ADDR_MASK    (0xFFFF00000000)

/* Enable IPTV mac prefix filter in parser */
static int _rdpa_iptv_mac_filter_en(uint8_t quad_idx)
{
    rnr_quad_parser_da_filter ipv4_da_filter = {};
    rnr_quad_parser_da_filter ipv6_da_filter = {};
    bdmf_error_t rc = BDMF_ERR_OK;

    ipv4_da_filter.da_filt_lsb = MAC_ADDR_LOW(IPV4_MCAST_MAC_ADDR_PREFIX);
    ipv4_da_filter.da_filt_msb = MAC_ADDR_HIGH(IPV4_MCAST_MAC_ADDR_PREFIX);
    ipv4_da_filter.da_filt_mask_l = MAC_ADDR_LOW(IPV4_MCAST_MAC_ADDR_MASK);
    ipv4_da_filter.da_filt_mask_msb = MAC_ADDR_HIGH(IPV4_MCAST_MAC_ADDR_MASK);

    ipv6_da_filter.da_filt_lsb = MAC_ADDR_LOW(IPV6_MCAST_MAC_ADDR_PREFIX);
    ipv6_da_filter.da_filt_msb = MAC_ADDR_HIGH(IPV6_MCAST_MAC_ADDR_PREFIX);
    ipv6_da_filter.da_filt_mask_l = MAC_ADDR_LOW(IPV6_MCAST_MAC_ADDR_MASK);
    ipv6_da_filter.da_filt_mask_msb = MAC_ADDR_HIGH(IPV6_MCAST_MAC_ADDR_MASK);

    /*TODO: note that only profile 0 is set*/
    rc = rc ? rc : ag_drv_rnr_quad_parser_da_filter2_set(quad_idx, 0, 0);
    rc = rc ? rc : ag_drv_rnr_quad_parser_da_filter2_set(quad_idx, 1, 0);

    rc = rc ? rc : ag_drv_rnr_quad_parser_da_filter_set(quad_idx, &ipv4_da_filter);
    rc = rc ? rc : ag_drv_rnr_quad_parser_da_filter1_set(quad_idx, &ipv6_da_filter);

    rc = rc ? rc : ag_drv_rnr_quad_parser_da_filter2_set(quad_idx, 0, 1);
    rc = rc ? rc : ag_drv_rnr_quad_parser_da_filter2_set(quad_idx, 1, 1);

    return rc;
}

/* remove when rdpa_filter_ex is check in */
static int  _rdpa_filter_ip_filter_en(uint8_t quad_idx)
{
    int rc = BDMF_ERR_OK;

    /* Ipv4 multicast control range - 224.0.0.0 to 224.0.0.255, mask = 255.255.255.0.
     * Runner expects to receive in host-order */
    /* Ipv6 multicast control range - FF02:: to FF02:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF,
     * mask = FFFF::. By default, MS DW is used. */
    rnr_quad_parser_ip0 parser_ip0 = {.ip_address = 0xe0000000, .ip_address_mask = 0xFFFFFF00, .ip_filter0_dip_en = 1, .ip_filter0_valid = 1};
    rnr_quad_parser_ip0 parser_ip1 = {.ip_address = 0xff020000, .ip_address_mask = 0xffff0000, .ip_filter0_dip_en = 1, .ip_filter0_valid = 1};

    rc = rc ? rc : ag_drv_rnr_quad_parser_ip0_set(quad_idx, &parser_ip0);
    rc = rc ? rc : ag_drv_rnr_quad_parser_ip1_set(quad_idx, &parser_ip1);

    return rc;
}
/*******************************************************************************/

extern uintptr_t xrdp_virt2phys(const ru_block_rec *ru_block, uint8_t addr_idx);

static int runner_init(int is_basic)
{
    rdd_init_params_t rdd_init_params = {0};
    rnr_dma_regs_cfg_t rnr_dma_cfg;
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t ubus_slave_idx, quad_idx;
    uint32_t addr_hi, addr_lo;
    uint16_t ddr_sop_offset0, ddr_sop_offset1;
    bdmf_phys_addr_t fpm_base_phys_addr;
#if !defined(_CFE_) && !defined(RDP_SIM)
    bdmf_phys_addr_t psram_dma_base_phys_addr;
#endif
    RDD_HW_IPTV_CONFIGURATION_DTS iptv_hw_config;

    drv_rnr_cores_addr_init();
    drv_rnr_mem_init();
    drv_rnr_load_microcode();
    drv_rnr_load_prediction();
    drv_rnr_set_sch_cfg();

    fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
    GET_ADDR_HIGH_LOW(addr_hi, addr_lo, fpm_base_phys_addr);
    rnr_dma_cfg.ddr.dma_base = (addr_hi << 12) | (addr_lo >> 20);
    rnr_dma_cfg.ddr.dma_buf_size = DMA_BUFSIZE_512;
    rnr_dma_cfg.ddr.dma_static_offset = 0;
#if defined(_CFE_) || defined(RDP_SIM)
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
            rc = rc ? rc : ag_drv_rnr_quad_ext_flowctrl_config_token_val_set(quad_idx, ubus_slave_idx, 8);

        if (rc)
            return rc;
        /* ToDo: should be moved to rdpa_filter file */
        _rdpa_filter_ip_filter_en(quad_idx);

        //Todo:  remove when rdpa_iptv_ex is check in */
        _rdpa_iptv_mac_filter_en(quad_idx);
    }
    rc = rc ? rc : rdp_drv_bkpt_init();

    return rc;
}

static inline void dispatcher_reorder_viq_init(dsptchr_config *cfg, dsptchr_cngs_params *congs_init, uint8_t bb_id, uint32_t target_address,
    bdmf_boolean dest, bdmf_boolean delayed, uint8_t viq_num, bdmf_boolean is_bbh_queue)
{
    cfg->dsptchr_viq_list[viq_num].wakeup_thrs = DSPTCHR_WAKEUP_THRS;
    cfg->dsptchr_viq_list[viq_num].bb_id = bb_id;
    cfg->dsptchr_viq_list[viq_num].bbh_target_address = target_address; /* for BBH - 2 is normal, 3 is exclusive */
    cfg->dsptchr_viq_list[viq_num].queue_dest = dest; /* 0-disp, 1-reor */
    cfg->dsptchr_viq_list[viq_num].delayed_queue = delayed; /* 0-non delayed, 1-delayed */
    cfg->dsptchr_viq_list[viq_num].common_max_limit = (DIS_REOR_LINKED_LIST_BUFFER_NUM - (DSPTCHR_GUARANTEED_BUFFERS * (DISP_REOR_VIQ_LAST + 1)));
    cfg->dsptchr_viq_list[viq_num].guaranteed_max_limit = DSPTCHR_GUARANTEED_BUFFERS;
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
}

static int dispatcher_reorder_init(void)
{
    dsptchr_config cfg;
    dsptchr_cngs_params congs_init;
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t bbh_id, core_index, i;
    uint32_t system_port_mask = 0;
    uint16_t common_max_limit = DIS_REOR_LINKED_LIST_BUFFER_NUM 
        - (DSPTCHR_NORMAL_GUARANTEED_BUFFERS * (DISP_REOR_VIQ_LAST + 1))
        + DSPTCHR_NORMAL_GUARANTEED_BUFFERS * (BBH_ID_NUM - 1)  /*No Guaranteed for exclusive VIQs besides PON*/
        + (DSPTCHR_NORMAL_GUARANTEED_BUFFERS - DSPTCHR_EXCLUSIVE_WAN_GUARANTEED_BUFFERS) /*Guaranteed for PON exclusive VIQ*/
        + DISP_REOR_VIQ_LAST; /* Guaranteed for dummy buffers*/

    xrdp_memset(&cfg, 0, sizeof(dsptchr_config));
    xrdp_memset(&congs_init, 0, sizeof(dsptchr_cngs_params));
    congs_init.frst_lvl = DIS_REOR_LINKED_LIST_BUFFER_NUM - 1;
    congs_init.scnd_lvl = DIS_REOR_LINKED_LIST_BUFFER_NUM - 1;
    congs_init.hyst_thrs = (DIS_REOR_LINKED_LIST_BUFFER_NUM / 4) - 1;

    /* reset dispatcher credit for all queues */
    for (i = 0; i < DSPTCHR_VIRTUAL_QUEUE_NUM; ++i)
        rc = rc ? rc : ag_drv_dsptchr_credit_cnt_set(i, 0);

    /* configure all viq 0-15 are for bbh-rx */
    for (bbh_id = BBH_ID_FIRST; bbh_id < BBH_ID_NUM; bbh_id++)
    {
        /* normal viq for bbh-rx */
        dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_BBH_0 + (2 * bbh_id), dsptchr_viq_bbh_target_addr_normal, dsptchr_viq_dest_disp,
            dsptchr_viq_delayed, bbh_id, DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 1);

        /* exclusive viq for bbh-rx */
        dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_BBH_0 + (2 * bbh_id), dsptchr_viq_bbh_target_addr_excl, dsptchr_viq_dest_disp,
            dsptchr_viq_delayed, bbh_id + BBH_ID_NUM, DSPTCHR_EXCLUSIVE_WAN_GUARANTEED_BUFFERS * (bbh_id == BBH_ID_PON), common_max_limit, 1);
    }

    /* VIQ for epon tm - 16 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(us_tm_runner_image), ((IMAGE_3_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3) |
        (IMAGE_3_US_TM_WAN_THREAD_NUMBER << 12)), dsptchr_viq_dest_reor, dsptchr_viq_non_delayed, DISP_REOR_VIQ_EPON_TM, 0);
        DISP_REOR_VIQ_EPON_TM, DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);

    /* VIQ for CPU_TX egress - 17 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(cpu_tx_runner_image), (IMAGE_2_CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 |
        IMAGE_2_CPU_IF_2_CPU_TX_EGRESS_THREAD_NUMBER << 12), dsptchr_viq_dest_reor, dsptchr_viq_non_delayed, DISP_REOR_VIQ_CPU_TX_EGRESS, 0);
        DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);

    /* VIQ for CPU_TX forward - 18 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(cpu_tx_runner_image), (IMAGE_2_CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 |
        IMAGE_2_CPU_IF_2_CPU_TX_INGRESS_THREAD_NUMBER << 12), dsptchr_viq_dest_disp, dsptchr_viq_delayed, DISP_REOR_VIQ_CPU_TX_FORWARD, 0);
        DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);

    /* VIQ for flush US - 19 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(us_tm_runner_image), (IMAGE_3_FLUSH_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3) |
        (IMAGE_3_US_TM_FLUSH_THREAD_NUMBER << 12), dsptchr_viq_dest_reor, dsptchr_viq_delayed, DISP_REOR_VIQ_US_TM_FLUSH, 0);
        DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);

    /* VIQ for flush DS - 20 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(ds_tm_runner_image), (IMAGE_0_FLUSH_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3) |
        (IMAGE_0_DS_TM_FLUSH_THREAD_NUMBER << 12), dsptchr_viq_dest_reor, dsptchr_viq_delayed, DISP_REOR_VIQ_DS_TM_FLUSH, 0);
        DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);

    /* configure all rnr groups */
    for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
    {
        /* setting group 0 - processing */
        if (rdp_core_to_image_map[core_index] == processing_runner_image)
        {
            dispatcher_reorder_rnr_group_init(&cfg, 0, core_index, 0xFF, ((RDD_DS_PACKET_BUFFER_ADDRESS_ARR[core_index] + offsetof(RDD_PACKET_BUFFER_DTS, pd)) >> 3),
                sizeof(RDD_PACKET_BUFFER_DTS) >> 3);
        }

        /* setting group 1 - wan direct */
        if (rdp_core_to_image_map[core_index] == wan_direct_runner_image)
        {
            dispatcher_reorder_rnr_group_init(&cfg, 1, core_index, (1<< IMAGE_1_CPU_IF_1_WAN_DIRECT_THREAD_NUMBER), (IMAGE_1_DIRECT_PROCESSING_PD_TABLE_ADDRESS >> 3), 0);
        }
    }

    cfg.prcs_rnr_num = GROUPED_EN_SEGMENTS_NUM;

    /* mapping viq to rnr group */
#ifndef _CFE_
    cfg.rnr_grp_num = 2;
    cfg.dsptchr_rnr_group_list[0].queues_mask = 0x7FFF | (1 << DISP_REOR_VIQ_CPU_TX_FORWARD); /* all bbh viq except wan-bbh exclusive viq and cpu tx forwarding */
    cfg.dsptchr_rnr_group_list[1].queues_mask = 0x8000; /* wan-bbh exclusive viq */
#else
    /* send all traffic via "direct flow" */
    cfg.rnr_grp_num = 2;
    cfg.dsptchr_rnr_group_list[0].queues_mask = 0x0000;
    cfg.dsptchr_rnr_group_list[1].queues_mask = 0xFFFF;
#endif

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

#ifndef _CFE_
static int natc_init(void)
{
    natc_config_t cfg;

    xrdp_memset(&cfg, 0, sizeof(natc_config_t));
    cfg.ctrl_status.nat_hash_mode = hash_mode_crc32;
    cfg.ctrl_status.ddr_hash_mode = hash_mode_crc32;
    cfg.ctrl_status.multi_hash_limit = multi_hash_limit_def;

    cfg.ctrl_status.age_timer_tick = timer_tick_packet;
    cfg.ctrl_status.age_timer = age_timer_32_ticks;
    cfg.ctrl_status.ddr_bins_per_bucket = ddr_8_bins_per_bucket;
    cfg.ctrl_status.ddr_size = ddr_size_64k;
    cfg.ctrl_status.total_len = total_len_80B;
    cfg.ctrl_status.cache_update_on_reg_ddr_lookup = 1;
    cfg.ctrl_status.ddr_enable = 1;
    cfg.ctrl_status.natc_enable = 1;

    cfg.tbl_num = 1;
    cfg.tbl_cntrl[tuple_lkp_tbl].key_len = key_len_16B;
    cfg.tbl_cfg[tuple_lkp_tbl].mask = NATC_16BYTE_KEY_MASK;
    cfg.tbl_cfg[tuple_lkp_tbl].key_len = NATC_TABLE_KEY_16B +
        (NATC_TABLE_KEY_16B * cfg.tbl_cntrl[tuple_lkp_tbl].key_len);
    cfg.tbl_cfg[tuple_lkp_tbl].key_tbl_size = NATC_TABLE_SIZE * cfg.tbl_cfg[tuple_lkp_tbl].key_len;
    cfg.tbl_cfg[tuple_lkp_tbl].res_len = total_length_bytes[cfg.ctrl_status.total_len] - cfg.tbl_cfg[tuple_lkp_tbl].key_len;
    cfg.tbl_cfg[tuple_lkp_tbl].res_tbl_size = NATC_TABLE_SIZE * cfg.tbl_cfg[tuple_lkp_tbl].res_len;

    cfg.tbl_cfg[tuple_lkp_tbl].vir_addr.key = p_dpi_cfg->rdp_ddr_rnr_tables_base_virt;
    cfg.tbl_cfg[tuple_lkp_tbl].vir_addr.res = (void *)((uint8_t *)cfg.tbl_cfg[tuple_lkp_tbl].vir_addr.key +
	    cfg.tbl_cfg[tuple_lkp_tbl].key_tbl_size);

    cfg.tbl_cfg[tuple_lkp_tbl].phy_addr.key = RDD_RSV_VIRT_TO_PHYS(cfg.tbl_cfg[tuple_lkp_tbl].vir_addr.key);
    cfg.tbl_cfg[tuple_lkp_tbl].phy_addr.res = RDD_RSV_VIRT_TO_PHYS(cfg.tbl_cfg[tuple_lkp_tbl].vir_addr.res);

    return drv_natc_init(&cfg);
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
        /* setup 1 queue */
        queue_cfg.wred_profile = 0;                 /* WRED profile */
        queue_cfg.copy_dec_profile = 0;             /* Copy decision profile */
        queue_cfg.fpm_ug = FPM_US_UG;               /* FPM UG */
        queue_cfg.ddr_copy_disable = 0;             /* TRUE=never copy to DDR */
        queue_cfg.copy_to_ddr = US_DDR_COPY_ENABLE; /* TRUE=Force copy to DDR */
        queue_cfg.aggregation_disable = 1;          /* TRUE=Disable aggregation */
        queue_cfg.exclusive_priority = 0;           /* TRUE=exclusive priority */
        queue_cfg.q_802_1ae = 0;                    /* TRUE=802.1AE */
        queue_cfg.sci = 0;                          /* TRUE=SCI */
        queue_cfg.fec_enable = 0;                   /* TRUE=enable FEC */

        for (idx = 0; idx <= QM_QUEUE_US_END; idx++)
        {
            rc = rc ? rc : drv_qm_queue_config(idx, &queue_cfg);
        }

        /* us - group 2 (update fifo task) */
        rnr_group_cfg.start_queue = QM_QUEUE_US_START; /* should be from dpi params */
        rnr_group_cfg.end_queue = QM_QUEUE_US_EPON_START-1; /* should be from dpi params */
        rnr_group_cfg.pd_fifo_base = (IMAGE_3_PD_FIFO_TABLE_ADDRESS >> 3);
        rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
        rnr_group_cfg.upd_fifo_base = (IMAGE_3_UPDATE_FIFO_TABLE_ADDRESS >> 3);
        rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
        rnr_group_cfg.rnr_bb_id = get_runner_idx(us_tm_runner_image);
        rnr_group_cfg.rnr_task = IMAGE_3_US_TM_UPDATE_FIFO_THREAD_NUMBER;
        rnr_group_cfg.rnr_enable = 1;
        rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_2, &rnr_group_cfg);

        /* us - group 1 (epon update fifo task) */
        rnr_group_cfg.start_queue = QM_QUEUE_US_EPON_START; /* should be from dpi params */
        rnr_group_cfg.end_queue = QM_QUEUE_US_END; /* should be from dpi params */
        rnr_group_cfg.pd_fifo_base = (IMAGE_3_PD_FIFO_TABLE_ADDRESS >> 3) + 128 * 2 * 2;
        rnr_group_cfg.rnr_task = IMAGE_3_US_TM_UPDATE_FIFO_EPON_THREAD_NUMBER;
        rnr_group_cfg.upd_fifo_base = (IMAGE_3_EPON_UPDATE_FIFO_TABLE_ADDRESS >> 3);
        rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
        rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
        rnr_group_cfg.rnr_bb_id = get_runner_idx(us_tm_runner_image);
        rnr_group_cfg.rnr_enable = 1;
        rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_1, &rnr_group_cfg);
    }
    else
    {
        /* us - group 1 (update fifo task) */
        rnr_group_cfg.start_queue = QM_QUEUE_US_START; /* should be from dpi params */
        rnr_group_cfg.end_queue = QM_QUEUE_US_END; /* should be from dpi params */
        rnr_group_cfg.pd_fifo_base = (IMAGE_3_PD_FIFO_TABLE_ADDRESS >> 3);
        rnr_group_cfg.rnr_task = IMAGE_3_US_TM_UPDATE_FIFO_THREAD_NUMBER;
        rnr_group_cfg.upd_fifo_base = (IMAGE_3_UPDATE_FIFO_TABLE_ADDRESS >> 3);
        rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
        rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
        rnr_group_cfg.rnr_bb_id = get_runner_idx(us_tm_runner_image);
        rnr_group_cfg.rnr_enable = 1;
        rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_1, &rnr_group_cfg);
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
    uint32_t buf_size;

    fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
    buf_size = g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[BBH_ID_FIRST].bufsize;

    rc = ag_drv_dqm_fpm_addr_set(xrdp_virt2phys(&RU_BLK(FPM), 0) + RU_REG_OFFSET(FPM, POOL1_ALLOC_DEALLOC));
    rc = rc ? rc : ag_drv_dqm_buf_base_set(fpm_base_phys_addr);
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

    qm_init_cfg init_cfg =
    {
        .is_counters_read_clear = 1,   /* TRUE=read-clear counter access mode */
        .is_drop_counters_enable = 1,  /* TRUE=drop counters enable. FALSE=max occupancy holder */
        .ddr_sop_offset = {18, 192}     /* DDR SoP offsets */
    };

    fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
    GET_ADDR_HIGH_LOW(addr_hi, addr_lo, fpm_base_phys_addr);

    init_cfg.fpm_base = (addr_hi << 24) | (addr_lo >> 8); /* in 256B resolution */
    init_cfg.fpm_buf_size = g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[BBH_ID_FIRST].bufsize;

    init_cfg.num_fpm_ug = 4;          /* Number of valid elements in fpm_ug_thr array */

    init_cfg.fpm_ug_thr[0].higher_thr = 8*1024; /* DS */
    init_cfg.fpm_ug_thr[0].lower_thr = init_cfg.fpm_ug_thr[0].mid_thr = init_cfg.fpm_ug_thr[0].higher_thr;

    init_cfg.fpm_ug_thr[1].higher_thr = 24*1024; /* US */
    init_cfg.fpm_ug_thr[1].lower_thr = init_cfg.fpm_ug_thr[1].mid_thr = init_cfg.fpm_ug_thr[1].higher_thr;

    init_cfg.fpm_ug_thr[2].higher_thr = 8*1024; /* CPU */
    init_cfg.fpm_ug_thr[2].lower_thr = init_cfg.fpm_ug_thr[2].mid_thr = init_cfg.fpm_ug_thr[2].higher_thr;

    init_cfg.fpm_ug_thr[3].higher_thr = 24*1024; /* WLAN */
    init_cfg.fpm_ug_thr[3].lower_thr = init_cfg.fpm_ug_thr[3].mid_thr = init_cfg.fpm_ug_thr[3].higher_thr;


    rc = rc ? rc : drv_qm_init(&init_cfg);
    if (!rc)
        ag_drv_qm_qm_pd_cong_ctrl_set(DEF_QM_PD_CONGESTION_CONTROL_THRESHOLD);

    /* Initializing DQM before any of QM's queues */
    rc = rc ? rc : dqm_init();

    /* setup 1 queue */
    queue_cfg.wred_profile = 0;                 /* WRED profile */
    queue_cfg.copy_dec_profile = 0;             /* Copy decision profile */
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

    /* Init ALL queues */
    for (idx = QM_QUEUE_US_START; idx <= QM_QUEUE_US_END; idx++)
    {
        rc = rc ? rc : drv_qm_queue_config(idx, &queue_cfg);
        rc = rc ? rc : drv_qm_queue_enable(idx);
    }
    queue_cfg.fpm_ug = FPM_DS_UG;                       /* FPM UG */
    queue_cfg.copy_to_ddr = DS_DDR_COPY_ENABLE;         /* TRUE=Force copy to DDR */
    queue_cfg.aggregation_disable = DS_AGG_DISABLE;     /* TRUE=Disable aggregation */
    for (idx = QM_QUEUE_DS_START; idx <= QM_QUEUE_DS_END; idx++)
    {
        rc = rc ? rc : drv_qm_queue_config(idx, &queue_cfg);
        rc = rc ? rc : drv_qm_queue_enable(idx);
    }
    /* Init DS drop queue */
    queue_cfg.wred_profile = QM_WRED_PROFILE_DROP_ALL;
    queue_cfg.aggregation_disable = 1;   /* TRUE=Disable aggregation */
    rc = rc ? rc : drv_qm_queue_config(QM_QUEUE_DROP, &queue_cfg);
    rc = rc ? rc : drv_qm_queue_enable(idx);

    queue_cfg.fpm_ug = FPM_CPU_UG;       /* FPM UG */
    queue_cfg.wred_profile = 0;
    queue_cfg.copy_to_ddr = 0;           /* TRUE=Force copy to DDR */
    queue_cfg.aggregation_disable = 1;   /* TRUE=Disable aggregation */
    for (idx = QM_QUEUE_DS_END + 1; idx < QM_NUM_QUEUES; idx++)
    {
        if (idx == QM_QUEUE_DROP)
            continue;
        rc = rc ? rc : drv_qm_queue_config(idx, &queue_cfg);
        rc = rc ? rc : drv_qm_queue_enable(idx);
    }

    /* ds group 0 */
    rnr_group_cfg.start_queue = QM_QUEUE_DS_START; /* should be from dpi params */
    rnr_group_cfg.end_queue = QM_QUEUE_DS_END; /* should be from dpi params */
    rnr_group_cfg.pd_fifo_base = (IMAGE_0_PD_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
    rnr_group_cfg.upd_fifo_base = (IMAGE_0_UPDATE_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
    rnr_group_cfg.rnr_bb_id = get_runner_idx(ds_tm_runner_image);
    rnr_group_cfg.rnr_task = IMAGE_0_DS_TM_UPDATE_FIFO_THREAD_NUMBER;
    rnr_group_cfg.rnr_enable = 1;
    rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_0, &rnr_group_cfg);

    /* cpu - group 3 (cpu_rx fifo task) */
    rnr_group_cfg.start_queue = QM_QUEUE_CPU_RX; /* single queue for CPU RX */
    rnr_group_cfg.end_queue = QM_QUEUE_CPU_RX; 
    rnr_group_cfg.pd_fifo_base = (IMAGE_1_PD_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.rnr_task = IMAGE_1_CPU_IF_1_CPU_RX_UPDATE_FIFO_THREAD_NUMBER;
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
    rnr_group_cfg.rnr_task = IMAGE_2_CPU_IF_2_CPU_TX_EGRESS_THREAD_NUMBER;
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
    rnr_group_cfg.rnr_task = IMAGE_2_CPU_IF_2_CPU_TX_INGRESS_THREAD_NUMBER;
    rnr_group_cfg.upd_fifo_base = (IMAGE_2_CPU_TX_INGRESS_UPDATE_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
    rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
    rnr_group_cfg.rnr_bb_id = get_runner_idx(cpu_tx_runner_image);
    rnr_group_cfg.rnr_enable = 1;
    rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_5, &rnr_group_cfg);

    return rc;
}

#ifdef XRDP_EMULATION
static int unimac_init()
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t idx;
    uint16_t max_pkt_size;
    uint16_t rxfifo_congestion_threshold;

    for (idx = 0; idx < NUM_OF_UNIMAC; idx++) 
    {
        rc = rc ? rc : ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_get(idx, &max_pkt_size, &rxfifo_congestion_threshold);
        rc = rc ? rc : ag_drv_unimac_misc_unimac_top_unimac_misc_unimac_ext_cfg1_set(idx, p_dpi_cfg->mtu_size, rxfifo_congestion_threshold);
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
    for (idx = BBH_ID_FIRST; idx < BBH_ID_PON; idx++)
    {
        rc = rc ? rc : ag_drv_bbh_rx_general_configuration_enable_set(idx, 1, 1);
    }

    /* enable DISP_REOR */
    rc = rc ? rc : ag_drv_dsptchr_reorder_cfg_dsptchr_reordr_cfg_set(1, 1, 0, 0);

    /* enable QM */
    rc = rc ? rc : ag_drv_qm_enable_ctrl_set(&qm_enable);
    rc = rc ? rc : ag_drv_ubus_mstr_en_set(0, 1);
    rc = rc ? rc : ag_drv_ubus_mstr_en_set(1, 1);

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
    int rc;

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
    BUG_ON(wan_bbh == MAC_TYPE_XLMAC);
    rc = rc ? rc : rdd_scheduling_us_wan(wan_bbh, rdpa_emac__num_of);

    /* Enable ghost\dbr reporting */
#ifndef _CFE_
    rdd_ghost_reporting_mac_type_init(wan_bbh);
    rc = rc ? rc : rdd_ghost_reporting_timer_set();
#endif

    if (wan_bbh == MAC_TYPE_EPON)
        rc = rc ? rc : rdd_tm_epon_cfg();

    /* configure bbh-tx fifo size */
    if (wan_bbh == MAC_TYPE_EPON || wan_bbh == MAC_TYPE_XEPON)
        rdd_scheduling_bbh_tx_fifo_size_cfg(BBH_TX_EPON_PD_FIFO_SIZE_0_7);
    else
        rdd_scheduling_bbh_tx_fifo_size_cfg(BBH_TX_GPON_PD_FIFO_SIZE_0_7);

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize drivers for wantype %d", wan_bbh);

    return rc;
}

uint32_t data_path_init_gbe(rdpa_emac wan_emac)
{
    int rc;

    /* QM mactype init */
    rc = qm_wantype_init(0);

    /* RUNNER mactype init */
    rc = rc ? rc : rdd_scheduling_us_wan(0, wan_emac);

    /* configure bbh-tx fifo size */
    rdd_scheduling_bbh_tx_fifo_size_cfg(BBH_TX_DS_PD_FIFO_SIZE_0);

    if (rc)
        BDMF_TRACE_ERR("Failed to initialize drivers for GBE, wan_emac %d", wan_emac);

    return rc;
}

static int _data_path_init(dpi_params_t *dpi_params, int is_basic)
{
    bdmf_error_t rc;
    p_dpi_cfg = dpi_params;

    bdmf_trace("INFO: %s#%d: Start.\n", __FUNCTION__, __LINE__);
#ifdef RDP_SIM
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
    rc = rc ? rc : cnpl_init();
    rc = rc ? rc : dma_sdma_init();
    rc = rc ? rc : dispatcher_reorder_init();    
#ifndef _CFE_
    rc = rc ? rc : natc_init();
#if 0
    rc = rc ? rc : hash_init();
#endif
#endif
#ifdef XRDP_EMULATION
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


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
#include "rdd_basic_scheduler.h"
#include "rdp_drv_sbpm.h"
#include "rdp_drv_cnpl.h"
#include "xrdp_drv_psram_ag.h"
#include "xrdp_drv_fpm_ag.h"
#include "rdp_common.h"
#include "rdp_drv_dma.h"
#include "rdp_drv_rnr.h"
#include "rdp_drv_dqm.h"
#include "rdp_drv_dis_reor.h"
#include "rdp_drv_tcam.h"
#include "rdp_drv_fpm.h"
#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_xlif_rx_if_ag.h"
#include "xrdp_drv_xlif_tx_if_ag.h"
#include "xrdp_drv_ubus_mstr_ag.h"
#include "xrdp_drv_ubus_slv_ag.h"
#include "rdp_drv_qm.h"
#include "bdmf_data_types.h"
#include "rdd_scheduling.h"
#include "rdpa_config.h"

#undef RNR_LAST
#define RNR_LAST 2

#if defined(CONFIG_BCM_GLB_COHERENCY) && !defined(RDP_SIM)
#include <board.h>
#include "bcm_misc_hw_init.h"
#include "bcm_map_part.h"
#define DECODE_WIN0 0
#define DECODE_WIN1 1
#define CACHE_BIT_OFF 0
#define CACHE_BIT_ON 1
#endif

extern rdpa_speed_type rdpa_wan_speed_get(rdpa_if if_);
extern bdmf_boolean rdpa_is_epon_ae_mode(void);
dpi_params_t *p_dpi_cfg;

extern uint32_t total_length_bytes[];
extern rdpa_emac bbh_id_to_rdpa_emac[BBH_ID_NUM];
extern rdd_bb_id rdpa_emac_to_bb_id_tx[rdpa_emac__num_of];

RDD_FPM_GLOBAL_CFG_DTS g_fpm_hw_cfg;
bbh_rx_sdma_profile_t *g_bbh_rx_sdma_profile;
bbh_tx_dma_profile_t *g_bbh_tx_dma_profile;
bbh_tx_sdma_profile_t *g_bbh_tx_sdma_profile;
bbh_tx_ddr_profile_t *g_bbh_tx_ddr_profile;
queue_to_rnr_t g_lan_queue_to_rnr[1] = { {RNR0, RNR0} };
pd_fifo_size_t g_lan_pd_fifo_size[1] = { {BBH_TX_DS_PD_FIFO_SIZE_0, BBH_TX_DS_PD_FIFO_SIZE_1} };
pd_wkup_threshold_t g_lan_pd_wkup_threshold[2] = { {0, 0}, {0, 0} };
pd_bytes_threshold_t g_lan_pd_bytes_threshold[1] = { {0, 0} };
dma_id_e g_dma_source[] = {BB_ID_DMA0, BB_ID_DMA1, BB_ID_SDMA0, BB_ID_SDMA1};

#ifdef BCM6858
#define XFI_EMAC_PORT 0
#endif

queue_to_rnr_t g_wan_queue_to_rnr[TX_QEUEU_PAIRS] = {
          {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0},
          {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0} };

bbh_to_dma_x_t  g_bbh_to_dma_x[BBH_ID_NUM] = {
          {BBH_ID_XLMAC0_0_10G, DMA1_ID},
          {BBH_ID_XLMAC0_1_2p5G, DMA1_ID},
          {BBH_ID_XLMAC0_2_1G, DMA1_ID},
          {BBH_ID_XLMAC0_3_1G, DMA0_ID},
          {BBH_ID_XLMAC1_0_RGMII, DMA1_ID},
          {BBH_ID_XLMAC1_1_RGMII, DMA0_ID},
          {BBH_ID_XLMAC1_2_RGMII, DMA0_ID},
#ifdef BCM6858
          {BBH_ID_XLMAC1_3_RGMII, DMA0_ID},
#endif
          {BBH_ID_PON, DMA0_ID} };

bbh_id_e g_dma_to_bbh_x[profiles_num][NUM_OF_DMA][NUM_OF_PERIPHERALS_PER_DMA] = {
#ifdef BCM6858
         {{BBH_ID_XLMAC0_3_1G, BBH_ID_XLMAC1_1_RGMII, BBH_ID_XLMAC1_2_RGMII, BBH_ID_XLMAC1_3_RGMII, BBH_ID_PON, BBH_ID_NULL},
          {BBH_ID_XLMAC0_0_10G, BBH_ID_XLMAC0_1_2p5G, BBH_ID_XLMAC0_2_1G, BBH_ID_XLMAC1_0_RGMII, BBH_ID_NULL, BBH_ID_NULL}},
         {{BBH_ID_XLMAC0_3_1G, BBH_ID_XLMAC1_1_RGMII, BBH_ID_XLMAC1_2_RGMII, BBH_ID_XLMAC1_3_RGMII, BBH_ID_PON, BBH_ID_NULL},
          {BBH_ID_XLMAC0_0_10G, BBH_ID_XLMAC0_1_2p5G, BBH_ID_XLMAC0_2_1G, BBH_ID_XLMAC1_0_RGMII, BBH_ID_NULL, BBH_ID_NULL}},
         {{BBH_ID_XLMAC0_3_1G, BBH_ID_XLMAC1_1_RGMII, BBH_ID_XLMAC1_2_RGMII, BBH_ID_XLMAC1_3_RGMII, BBH_ID_PON, BBH_ID_NULL},
#else
         {{BBH_ID_XLMAC0_3_1G, BBH_ID_XLMAC1_1_RGMII, BBH_ID_XLMAC1_2_RGMII, BBH_ID_PON, BBH_ID_NULL, BBH_ID_NULL},
          {BBH_ID_XLMAC0_0_10G, BBH_ID_XLMAC0_1_2p5G, BBH_ID_XLMAC0_2_1G, BBH_ID_XLMAC1_0_RGMII, BBH_ID_NULL, BBH_ID_NULL}},
         {{BBH_ID_XLMAC0_3_1G, BBH_ID_XLMAC1_1_RGMII, BBH_ID_XLMAC1_2_RGMII, BBH_ID_PON, BBH_ID_NULL, BBH_ID_NULL},
          {BBH_ID_XLMAC0_0_10G, BBH_ID_XLMAC0_1_2p5G, BBH_ID_XLMAC0_2_1G, BBH_ID_XLMAC1_0_RGMII, BBH_ID_NULL, BBH_ID_NULL}},
         {{BBH_ID_XLMAC0_3_1G, BBH_ID_XLMAC1_1_RGMII, BBH_ID_XLMAC1_2_RGMII, BBH_ID_PON, BBH_ID_NULL, BBH_ID_NULL},
#endif
          {BBH_ID_XLMAC0_0_10G, BBH_ID_XLMAC0_1_2p5G, BBH_ID_XLMAC0_2_1G, BBH_ID_XLMAC1_0_RGMII, BBH_ID_NULL, BBH_ID_NULL}} };

/* array description:
      - this array describe the buffers weight according to g_dma_to_bbh_X array.
      - total weight is 32 for each DMA (four BBH IDs) */
uint8_t g_bbh_buff_num[profiles_num][NUM_OF_DMA][NUM_OF_PERIPHERALS_PER_DMA] = {
#ifdef BCM6858
         /* PON + No XFI port */ {{5, 5, 5, 5, 12}, {8, 8, 8, 8}},
         /* PON + XFI port0   */ {{5, 5, 5, 5, 12}, {6, 6, 6, 14}},
         /* PON + XFI port4   */ {{5, 5, 5, 5, 12}, {14, 6, 6, 6}} };
#else
         /* PON + No XFI port */ {{6, 6, 6, 14}, {8, 8, 8, 8}},
         /* PON + XFI port0   */ {{6, 6, 6, 14}, {14, 6, 6, 6}},
         /* PON + XFI port4   */ {{6, 6, 6, 14}, {6, 6, 6, 14}} };
#endif

bb_source_t g_dma_bb_source[BBH_ID_NUM] = {
         {BB_ID_RX_XLMAC0_0_10G, BB_ID_TX_XLMAC0_0_10G},
         {BB_ID_RX_XLMAC0_1_2P5G, BB_ID_TX_XLMAC0_1_2P5G},
         {BB_ID_RX_XLMAC0_2_1G, BB_ID_TX_XLMAC0_2_1G},
         {BB_ID_RX_XLMAC0_3_1G, BB_ID_TX_XLMAC0_3_1G},
         {BB_ID_RX_XLMAC1_0_RGMII, BB_ID_TX_XLMAC1_0_RGMII},
         {BB_ID_RX_XLMAC1_1_RGMII, BB_ID_TX_XLMAC1_1_RGMII},
         {BB_ID_RX_XLMAC1_2_RGMII, BB_ID_TX_XLMAC1_2_RGMII},
#ifdef BCM6858
         {BB_ID_RX_XLMAC1_3_RGMII, BB_ID_TX_XLMAC1_3_RGMII},
#endif
         {BB_ID_RX_PON_ETH, BB_ID_TX_PON_ETH_PD} };

uint8_t g_max_otf_reads[BBH_ID_NUM] =
         {MAX_OTF_READ_REQUEST_DEFAULT_DMA1, MAX_OTF_READ_REQUEST_DEFAULT_DMA1,
          MAX_OTF_READ_REQUEST_DEFAULT_DMA1, MAX_OTF_READ_REQUEST_DEFAULT_DMA1,
          MAX_OTF_READ_REQUEST_DEFAULT_DMA1, MAX_OTF_READ_REQUEST_DEFAULT_DMA0,
#ifdef BCM6858
          MAX_OTF_READ_REQUEST_DEFAULT_DMA0, MAX_OTF_READ_REQUEST_DEFAULT_DMA0,
          MAX_OTF_READ_REQUEST_DEFAULT_DMA0};
#else
          MAX_OTF_READ_REQUEST_DEFAULT_DMA0, MAX_OTF_READ_REQUEST_DEFAULT_DMA0};
#endif

urgent_threhold_t g_dma_urgent_threshold[BBH_ID_NUM][2] = {
         { {DMA1_U_THRESH_IN_BBH_ID_XLMAC0_0_10G_VALUE, DMA1_U_THRESH_OUT_BBH_ID_XLMAC0_0_10G_VALUE},
           {SDMA1_U_THRESH_IN_BBH_ID_XLMAC0_0_10G_VALUE, SDMA1_U_THRESH_OUT_BBH_ID_XLMAC0_0_10G_VALUE} },
         { {DMA1_U_THRESH_IN_BBH_ID_XLMAC0_1_2p5G_VALUE, DMA1_U_THRESH_OUT_BBH_ID_XLMAC0_1_2p5G_VALUE},
           {SDMA1_U_THRESH_IN_BBH_ID_XLMAC0_1_2p5G_VALUE, SDMA1_U_THRESH_OUT_BBH_ID_XLMAC0_1_2p5G_VALUE} },
         { {DMA1_U_THRESH_IN_BBH_ID_XLMAC0_2_1G_VALUE, DMA1_U_THRESH_OUT_BBH_ID_XLMAC0_2_1G_VALUE},
           {SDMA1_U_THRESH_IN_BBH_ID_XLMAC0_2_1G_VALUE, SDMA1_U_THRESH_OUT_BBH_ID_XLMAC0_2_1G_VALUE} },
         { {DMA0_U_THRESH_IN_BBH_ID_XLMAC0_3_1G_VALUE, DMA0_U_THRESH_OUT_BBH_ID_XLMAC0_3_1G_VALUE},
           {SDMA0_U_THRESH_IN_BBH_ID_XLMAC0_3_1G_VALUE, SDMA0_U_THRESH_OUT_BBH_ID_XLMAC0_3_1G_VALUE} },
         { {DMA1_U_THRESH_IN_BBH_ID_XLMAC1_0_RGMII_VALUE, DMA1_U_THRESH_OUT_BBH_ID_XLMAC1_0_RGMII_VALUE},
           {SDMA1_U_THRESH_IN_BBH_ID_XLMAC1_0_RGMII_VALUE, SDMA1_U_THRESH_OUT_BBH_ID_XLMAC1_0_RGMII_VALUE} },
         { {DMA0_U_THRESH_IN_BBH_ID_XLMAC1_1_RGMII_VALUE, DMA0_U_THRESH_OUT_BBH_ID_XLMAC1_1_RGMII_VALUE},
           {SDMA0_U_THRESH_IN_BBH_ID_XLMAC1_1_RGMII_VALUE, SDMA0_U_THRESH_OUT_BBH_ID_XLMAC1_1_RGMII_VALUE} },
         { {DMA0_U_THRESH_IN_BBH_ID_XLMAC1_2_RGMII_VALUE, DMA0_U_THRESH_OUT_BBH_ID_XLMAC1_2_RGMII_VALUE},
           {SDMA0_U_THRESH_IN_BBH_ID_XLMAC1_2_RGMII_VALUE, SDMA0_U_THRESH_OUT_BBH_ID_XLMAC1_2_RGMII_VALUE} },
#ifdef BCM6858
         { {DMA0_U_THRESH_IN_BBH_ID_XLMAC1_2_RGMII_VALUE, DMA0_U_THRESH_OUT_BBH_ID_XLMAC1_2_RGMII_VALUE},
           {SDMA0_U_THRESH_IN_BBH_ID_XLMAC1_2_RGMII_VALUE, SDMA0_U_THRESH_OUT_BBH_ID_XLMAC1_2_RGMII_VALUE} },
#endif
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
         { {DMA0_STRICT_PRI_RX_BBH_ID_XLMAC1_2_RGMII_VALUE, DMA0_STRICT_PRI_TX_BBH_ID_XLMAC1_2_RGMII_VALUE},
           {SDMA0_STRICT_PRI_RX_BBH_ID_XLMAC1_2_RGMII_VALUE, SDMA0_STRICT_PRI_TX_BBH_ID_XLMAC1_2_RGMII_VALUE} },
#ifdef BCM6858
         { {DMA0_STRICT_PRI_RX_BBH_ID_XLMAC1_2_RGMII_VALUE, DMA0_STRICT_PRI_TX_BBH_ID_XLMAC1_2_RGMII_VALUE},
           {SDMA0_STRICT_PRI_RX_BBH_ID_XLMAC1_2_RGMII_VALUE, SDMA0_STRICT_PRI_TX_BBH_ID_XLMAC1_2_RGMII_VALUE} },
#endif
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
         { {DMA0_RR_WEIGHT_RX_BBH_ID_XLMAC1_2_RGMII_VALUE, DMA0_RR_WEIGHT_TX_BBH_ID_XLMAC1_2_RGMII_VALUE},
           {SDMA0_RR_WEIGHT_RX_BBH_ID_XLMAC1_2_RGMII_VALUE, SDMA0_RR_WEIGHT_TX_BBH_ID_XLMAC1_2_RGMII_VALUE} },
#ifdef BCM6858
         { {DMA0_RR_WEIGHT_RX_BBH_ID_XLMAC1_2_RGMII_VALUE, DMA0_RR_WEIGHT_TX_BBH_ID_XLMAC1_2_RGMII_VALUE},
           {SDMA0_RR_WEIGHT_RX_BBH_ID_XLMAC1_2_RGMII_VALUE, SDMA0_RR_WEIGHT_TX_BBH_ID_XLMAC1_2_RGMII_VALUE} },
#endif
         { {DMA0_RR_WEIGHT_RX_BBH_ID_PON_VALUE, DMA0_RR_WEIGHT_TX_BBH_ID_PON_VALUE},
           {SDMA0_RR_WEIGHT_RX_BBH_ID_PON_VALUE, SDMA0_RR_WEIGHT_TX_BBH_ID_PON_VALUE} } };

static uint8_t calculate_buffers_offset(uint8_t profile_id, uint8_t dma_num, uint8_t periphreal_num)
{
    uint8_t j, offset = 0;

    for (j = 0; j < periphreal_num; j++)
    {
        offset = offset + g_bbh_buff_num[profile_id][dma_num][j];
    }
    return offset;
}

static int bbh_profiles_init(void)
{
    uint8_t i, j, bbh_id, profile_id;
    bbh_rx_sdma_chunks_cfg_t *bbh_rx_sdma_cfg;
    bbh_tx_bbh_dma_cfg *bbh_tx_dma_cfg;
    bbh_tx_bbh_sdma_cfg *bbh_tx_sdma_cfg;
    bbh_tx_bbh_ddr_cfg *bbh_tx_ddr_cfg;

    g_bbh_rx_sdma_profile = (bbh_rx_sdma_profile_t *)xrdp_alloc(sizeof(bbh_rx_sdma_profile_t));
    g_bbh_tx_dma_profile = (bbh_tx_dma_profile_t *)xrdp_alloc(sizeof(bbh_tx_dma_profile_t));
    g_bbh_tx_sdma_profile = (bbh_tx_sdma_profile_t *)xrdp_alloc(sizeof(bbh_tx_sdma_profile_t));
    g_bbh_tx_ddr_profile = (bbh_tx_ddr_profile_t *)xrdp_alloc(sizeof(bbh_tx_ddr_profile_t));

    /* TODO: check profile ID according to active_ports and board_configuration */
    switch (p_dpi_cfg->xfi_port)
    {
        case rdpa_emac0:
            profile_id = wan_pon_xfi0_profile_id;
            break;
        case rdpa_emac4:
            profile_id = wan_pon_xfi4_profile_id;
            break;
        case rdpa_emac_none:
            profile_id = wan_pon_no_xfi_profile_id;
            break;
        default:
            return BDMF_ERR_PARM;
    }

    for (bbh_id = 0; bbh_id < BBH_ID_NUM; bbh_id++)
    {
        int found = 0;

        /* serach BBH_ID in DMA/SDMA array */
        for (i = 0; i < NUM_OF_DMA; i++)
        {
            for (j = 0; j < NUM_OF_PERIPHERALS_PER_DMA; j++)
            {
                if (g_bbh_to_dma_x[bbh_id].bbh_id == g_dma_to_bbh_x[profile_id][i][j])
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
        bbh_rx_sdma_cfg->sdma_bb_id = i ? BB_ID_SDMA1 : BB_ID_SDMA0;
        bbh_rx_sdma_cfg->first_chunk_idx =
            calculate_buffers_offset(profile_id, i, j) * BBH_RX_TOTAL_BUFFER_NUM / TOTAL_BUFFERS_WEIGHT;
        bbh_rx_sdma_cfg->sdma_chunks =
            g_bbh_buff_num[profile_id][i][j] * BBH_RX_TOTAL_BUFFER_NUM / TOTAL_BUFFERS_WEIGHT;

        bbh_tx_dma_cfg = &(g_bbh_tx_dma_profile->bbh_tx_dma_cfg[bbh_id]);
        bbh_tx_dma_cfg->dmasrc = i ? BB_ID_DMA1 : BB_ID_DMA0;
        bbh_tx_dma_cfg->descbase =
            calculate_buffers_offset(profile_id, i, j) * BBH_TX_TOTAL_BUFFER_NUM / TOTAL_BUFFERS_WEIGHT;
        bbh_tx_dma_cfg->descsize = g_bbh_buff_num[profile_id][i][j] * BBH_TX_TOTAL_BUFFER_NUM / TOTAL_BUFFERS_WEIGHT;

        bbh_tx_sdma_cfg = &(g_bbh_tx_sdma_profile->bbh_tx_sdma_cfg[bbh_id]);
        bbh_tx_sdma_cfg->sdmasrc = i ? BB_ID_SDMA1 : BB_ID_SDMA0;
        bbh_tx_sdma_cfg->descbase =
            calculate_buffers_offset(profile_id, i, j) * BBH_TX_TOTAL_BUFFER_NUM / TOTAL_BUFFERS_WEIGHT;
        bbh_tx_sdma_cfg->descsize = g_bbh_buff_num[profile_id][i][j] * BBH_TX_TOTAL_BUFFER_NUM / TOTAL_BUFFERS_WEIGHT;

        bbh_tx_ddr_cfg = &(g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[bbh_id]);

        lookup_bbh_tx_bufsz_by_fpm_bufsz(&p_dpi_cfg->fpm_buf_size, &bbh_tx_ddr_cfg->bufsize);

        bbh_tx_ddr_cfg->byteresul = RES_1B;
        bbh_tx_ddr_cfg->ddrtxoffset = 0;
        /* MIN size = 0x4 - MAX size = 0x40  where to check values?*/
        /*hnsize0 must be equal to hnsize1 because the per packet header size selection is not implemented yet*/
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

    if ((bbh_id == 0) || (bbh_id == 4))
        cfg.normal_viq = 4 - bbh_id;
    else
        cfg.normal_viq = bbh_id;

    /* In non G9991, there's no exclusive viqs for non PON BBHs, therefore, excl viq configuration of BBH RX is the same as normal VIQ*/
    cfg.excl_viq = (bbh_id == BBH_ID_PON) ? (BBH_ID_NUM + bbh_id) : bbh_id;
    cfg.excl_cfg.exc_en = (bbh_id == BBH_ID_PON);
    for (i = 0; i < 4; i++)
    {
        cfg.min_pkt_size[i] = MIN_ETH_PKT_SIZE;
        cfg.max_pkt_size[i] = p_dpi_cfg->mtu_size;
    }
    cfg.crc_omit_dis = 0;
    cfg.sop_offset = SOP_OFFSET;
    cfg.per_flow_th = BBH_RX_FLOWS_32_255_GROUP_DIVIDER;
    cfg.max_otf_sbpm = DRV_BBH_RX_MAXIMUM_OTF_SBPM_REQUESTS_DEFAULT_VAL;

    rc = rc ? rc : drv_bbh_rx_configuration_set(bbh_id, &cfg);
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
    return rc;
}


static int bbh_rx_init(void)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    bbh_id_e bbh_id;

    for (bbh_id = BBH_ID_FIRST; bbh_id <= BBH_ID_LAST_XLMAC; bbh_id++)
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
    ag_drv_ubus_mstr_hyst_ctrl_set(UBUS_MSTR_ID_1, UBUS_MSTR_CMD_SPCAE, UBUS_MSTR_DATA_SPCAE);
}

static int fpm_init(void)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    bdmf_boolean reset_req = 1;
    fpm_pool2_intr_sts interrupt_status = FPM_INTERRUPT_STATUS;
    fpm_pool2_intr_msk interrupt_mask = FPM_INTERRUPT_MASK;
    uint16_t timeout = 0;

    drv_fpm_init(p_dpi_cfg->rdp_ddr_pkt_base_virt, p_dpi_cfg->fpm_buf_size);

    rc = rc ? rc : ag_drv_fpm_pool1_xon_xoff_cfg_set(FPM_XON_THRESHOLD, FPM_XOFF_THRESHOLD);
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

static int sbpm_init(void)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint16_t base_addr;
    uint16_t init_offset;
    sbpm_thr_ug thr_ug = {};

    /* base address and offset */
    base_addr = SBPM_BASE_ADDRESS;
    init_offset = SBPM_INIT_OFFSET;

    rc = rc ? rc : ag_drv_sbpm_regs_init_free_list_set(base_addr, init_offset, 0, 0);
	/* TODO : temporary : for primitive Ingress Qos implementation in FW */
    rc = rc ? rc : drv_sbpm_thr_ug0_get(&thr_ug);
    thr_ug.excl_low_thr = 900;
    rc = rc ? rc : drv_sbpm_thr_ug0_set(&thr_ug);

    ag_drv_sbpm_regs_sbpm_ug0_excl_low_trsh_set(SBPM_UG0_EXCL_LOW_THRESHOLD, SBPM_UG0_EXCL_LOW_HIST);

#ifdef RDP_SIM
    drv_sbpm_default_val_init();
#endif
    if (rc)
        BDMF_TRACE_ERR("Failed to initialize sbpm driver\n");

    return rc;
}


static int bbh_tx_init(void)
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

    for (bbh_id = BBH_ID_FIRST; bbh_id <= BBH_ID_LAST_XLMAC; bbh_id++)
    {
        xrdp_memset(&config, 0, sizeof(bbh_tx_config));

        /* cores which sending PDs */
        config.mac_type = MAC_TYPE_EMAC;
        config.rnr_cfg[0].skb_addr = 0;


        if (p_dpi_cfg->bbh_id_gbe_wan != bbh_id)
        {
            config.rnr_cfg[0].rnr_src_id = get_runner_idx(ds_tm_runner_image);
            config.rnr_cfg[0].tcont_addr = IMAGE_0_DS_TM_BBH_TX_QUEUE_ID_TABLE_ADDRESS >> 3;
            if ((bbh_id == 0) || (bbh_id == 4))
                config.rnr_cfg[0].ptr_addr = (IMAGE_0_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS +
                    ((4 - bbh_id) * sizeof(RDD_BBH_TX_EGRESS_COUNTER_ENTRY_DTS))) >> 3;
            else
                config.rnr_cfg[0].ptr_addr = (IMAGE_0_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS + (bbh_id * sizeof(RDD_BBH_TX_EGRESS_COUNTER_ENTRY_DTS))) >> 3;
            if ((bbh_id == 0) || (bbh_id == 4))
                config.rnr_cfg[0].task_number = (IMAGE_0_DS_TM_LAN0_THREAD_NUMBER + (4 - bbh_id));
            else
                config.rnr_cfg[0].task_number = (IMAGE_0_DS_TM_LAN0_THREAD_NUMBER + bbh_id);
        }

        /* CHECK : how many queues to set (q2rnr)?*/
        config.lan_queue_cfg.queue_to_rnr = g_lan_queue_to_rnr;
        /* For Ethernet port working in MDU mode, PD FIFO size should be configured to 4 (and not 8). */
        config.lan_queue_cfg.pd_fifo_base = lan_pd_fifo_base;
        config.lan_queue_cfg.pd_fifo_size = g_lan_pd_fifo_size;
        /* why it says in the regge it is used for epon */
        config.lan_queue_cfg.pd_wkup_threshold = g_lan_pd_wkup_threshold;
        config.lan_queue_cfg.pd_bytes_threshold = g_lan_pd_bytes_threshold;
        /* pd_prefetch_byte_threshold feature is irrelevant in EMAC (since there is only one FIFO) */
        config.lan_queue_cfg.pd_bytes_threshold_en = 0;
        config.lan_queue_cfg.pd_empty_threshold = 1;

        config.src_id.fpmsrc = BB_ID_FPM;
        config.src_id.sbpmsrc = BB_ID_SBPM;
        /* Missing: MSG_RNR_SRC_ID */

        config.dma_cfg = &(g_bbh_tx_dma_profile->bbh_tx_dma_cfg[bbh_id]);
        config.sdma_cfg = &(g_bbh_tx_sdma_profile->bbh_tx_sdma_cfg[bbh_id]);
        config.ddr_cfg = &(g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[bbh_id]);

        fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
        GET_ADDR_HIGH_LOW(config.base_addr_high.addr[0], config.base_addr_low.addr[0], fpm_base_phys_addr);

        config.base_addr_low.addr[1] = 0;
        config.base_addr_high.addr[1] = 0;

        rc = rc ? rc : drv_bbh_tx_configuration_set(bbh_id, &config);
        rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_eee_set(bbh_id, 1);

        max_on_the_fly_reads = (g_max_otf_reads[bbh_id] > config.dma_cfg->descsize) ? config.dma_cfg->descsize : g_max_otf_reads[bbh_id];
        rc = rc ? rc : ag_drv_bbh_tx_dma_max_otf_read_request_set(bbh_id, max_on_the_fly_reads);
        max_on_the_fly_reads = (g_max_otf_reads[bbh_id] > config.sdma_cfg->descsize) ? config.sdma_cfg->descsize : g_max_otf_reads[bbh_id];
        rc = rc ? rc : ag_drv_bbh_tx_sdma_max_otf_read_request_set(bbh_id, max_on_the_fly_reads);

        /* only bbh_id 0 has extra long fifos for xfi */
        if (bbh_id == XFI_EMAC_PORT)
        {
            rc = rc ? rc : ag_drv_bbh_tx_common_configurations_dfifoctrl_set(bbh_id, BBH_TX_DATA_XFI_FIFO_SRAM_SIZE, BBH_TX_DATA_XFI_FIFO_DDR_SIZE, BBH_TX_XFI_DATA_FIFO_SRAM_BASE);
            rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_txthresh_set(bbh_id, BBH_TX_XFI_LAN_DDR_THRESHOLD, BBH_TX_XFI_LAN_SRAM_THRESHOLD);
        }
        else
        {
            rc = rc ? rc : ag_drv_bbh_tx_common_configurations_dfifoctrl_set(bbh_id, BBH_TX_DATA_FIFO_SRAM_SIZE, BBH_TX_DATA_FIFO_DDR_SIZE, BBH_TX_DATA_FIFO_SRAM_BASE);
            rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_txthresh_set(bbh_id, BBH_TX_LAN_DDR_THRESHOLD, BBH_TX_LAN_SRAM_THRESHOLD);
        }

        rc = rc ? rc : ag_drv_bbh_tx_lan_configurations_ts_set(bbh_id, 1);
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
            /* check if peripherial correspond to the right DMA */
            if (g_bbh_to_dma_x[bbh_id].dma_id != dma_id%2)
                continue;

            if (IS_SDMA(dma_id))
                rc = rc ? rc : ag_drv_dma_config_num_of_writes_set(dma_id, peripheral_id, g_bbh_rx_sdma_profile->bbh_rx_sdma_chunks_config[bbh_id].sdma_chunks);

            if (IS_DMA(dma_id))
                rc = rc ? rc : ag_drv_dma_config_num_of_reads_set(dma_id, peripheral_id, g_bbh_tx_dma_profile->bbh_tx_dma_cfg[bbh_id].descsize);
            else
                rc = rc ? rc : ag_drv_dma_config_num_of_reads_set(dma_id, peripheral_id, g_bbh_tx_sdma_profile->bbh_tx_sdma_cfg[bbh_id].descsize);

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
        if (rc)
            break;
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
    bdmf_error_t rc = BDMF_ERR_OK;
    rnr_dma_regs_cfg_t rnr_dma_cfg;
    rnr_quad_general_config_powersave_config powersave_cfg;
    uint8_t quad_idx, ubus_slave_idx;
    uint32_t addr_hi, addr_lo;
    bdmf_phys_addr_t fpm_base_phys_addr;
#if !defined(_CFE_) && !defined(RDP_SIM)
    bdmf_phys_addr_t psram_dma_base_phys_addr;
#endif
    rdd_init_params_t rdd_init_params = {0};

    drv_rnr_cores_addr_init();
    drv_rnr_mem_init();
    drv_rnr_load_microcode();
    drv_rnr_load_prediction();

    /* scheduler configuration */
    ag_drv_rnr_regs_cfg_sch_cfg_set(get_runner_idx(ds_tm_runner_image), DRV_RNR_4SP_12RR);

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

    /* ToDo: fill rdd_init_params */

    rdd_init_params.is_basic = is_basic;
    g_fpm_hw_cfg.fpm_token_size_asr_8 = p_dpi_cfg->fpm_buf_size >> 8;

    rc = rc ? rc : rdd_data_structures_init(&rdd_init_params);
    rc = rc ? rc : rnr_frequency_set(p_dpi_cfg->runner_freq);
    rc = rc ? rc : parser_init();
    if (rc)
        return rc;
    powersave_cfg.time_counter = 0x20; /* cycles to wait before power save */
    powersave_cfg.enable_powersave_core_0 = 1;
    powersave_cfg.enable_powersave_core_1 = 1;
    powersave_cfg.enable_powersave_core_2 = 1;
    powersave_cfg.enable_powersave_core_3 = 1;

    for (quad_idx = 0; quad_idx < NUM_OF_RNR_QUAD; quad_idx++)
    {
        rc = rc ? rc : ag_drv_rnr_quad_general_config_powersave_config_set(quad_idx, &powersave_cfg);

        /* change dynamic clock threshold in each quad */
        rc = rc ? rc : ag_drv_rnr_quad_general_config_dma_arb_cfg_set(quad_idx, 0, 0, 0, RNR_QUAD_DMA_ARB_CONJEST_THRESHOLD);
        rc = rc ? rc : ag_drv_rnr_quad_ext_flowctrl_config_token_val_set(quad_idx, XRDP_UBUS_SLAVE_IDX, 1);

        /* extend PSRAM slave tokens to 8 */
	    for (ubus_slave_idx = 16; ubus_slave_idx < 20; ubus_slave_idx++)
	        rc = rc ? rc : ag_drv_rnr_quad_ext_flowctrl_config_token_val_set(quad_idx, ubus_slave_idx, 8);

        if (rc)
            return rc;
    }

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
    if (cfg->viq_num <= viq_num)
        cfg->viq_num = viq_num + 1;
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


static uint32_t disp_reor_enable_vec(const dsptchr_config *cfg)
{
    uint8_t viq;
    uint32_t enable_vec = 0;

    for (viq = 0; viq < cfg->viq_num; viq++)
    {
        if (cfg->dsptchr_viq_list[viq].wakeup_thrs)
            enable_vec |= (1 << viq);
    }
    enable_vec &= ~(1 << BBH_ID_PON);
    enable_vec &= ~(1 << (BBH_ID_PON + BBH_ID_NUM));
    return enable_vec;
}

static int dispatcher_reorder_init(void)
{
    dsptchr_config cfg;
    dsptchr_cngs_params congs_init;
    bdmf_error_t rc = BDMF_ERR_OK;
    uint8_t bbh_id, core_index, i;

    /* Dispatcher queues for reassembly are not present in this case*/
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
        rc = rc ? rc : ag_drv_dsptchr_credit_cnt_set(i, 0);

    /* configure all viq 0-15 are for bbh-rx */
    for (bbh_id = BBH_ID_FIRST; bbh_id < BBH_ID_NUM; bbh_id++)
    {
        if ((bbh_id == 0) || (bbh_id == 4))
            /* normal viq for bbh-rx */
            dispatcher_reorder_viq_init(&cfg, &congs_init, (BB_ID_RX_XLMAC1_0_RGMII - (2 * (4 - bbh_id))), dsptchr_viq_bbh_target_addr_normal, dsptchr_viq_dest_disp,
                dsptchr_viq_delayed, (4 - bbh_id), DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 1);
        else if (bbh_id == BBH_ID_PON)
            /* normal viq for bbh-rx */
            dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_PON_ETH, dsptchr_viq_bbh_target_addr_normal, dsptchr_viq_dest_disp,
                dsptchr_viq_delayed, bbh_id, DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 1);
        else if (bbh_id == (BBH_ID_PON - 1))
            /* normal viq for bbh-rx */
            dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_XLMAC1_3_RGMII, dsptchr_viq_bbh_target_addr_normal, dsptchr_viq_dest_disp,
                dsptchr_viq_delayed, bbh_id, DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 1);
        else
            /* normal viq for bbh-rx */
            dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_XLMAC0_0_10G + (2 * bbh_id), dsptchr_viq_bbh_target_addr_normal, dsptchr_viq_dest_disp,
                dsptchr_viq_delayed, bbh_id, DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 1);

        if (bbh_id == BBH_ID_PON)
            dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_PON_ETH, dsptchr_viq_bbh_target_addr_excl, dsptchr_viq_dest_disp,
                dsptchr_viq_delayed, bbh_id + BBH_ID_NUM, DSPTCHR_EXCLUSIVE_WAN_GUARANTEED_BUFFERS * (bbh_id == BBH_ID_PON), common_max_limit, 1);
        else if (bbh_id == (BBH_ID_PON - 1))
            dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_XLMAC1_3_RGMII, dsptchr_viq_bbh_target_addr_excl, dsptchr_viq_dest_disp,
                dsptchr_viq_delayed, bbh_id + BBH_ID_NUM, DSPTCHR_EXCLUSIVE_WAN_GUARANTEED_BUFFERS * (bbh_id == BBH_ID_PON), common_max_limit, 1);
        else
            dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_XLMAC0_0_10G + (2 * bbh_id), dsptchr_viq_bbh_target_addr_excl, dsptchr_viq_dest_disp,
                dsptchr_viq_delayed, bbh_id + BBH_ID_NUM, DSPTCHR_EXCLUSIVE_WAN_GUARANTEED_BUFFERS * (bbh_id == BBH_ID_PON), common_max_limit, 1);
    }

    /* VIQ for CPU_TX egress - 18 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(cpu_tx_runner_image), (IMAGE_2_CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 |
        IMAGE_2_CPU_IF_2_CPU_TX_EGRESS_THREAD_NUMBER << 12), dsptchr_viq_dest_reor, dsptchr_viq_delayed, DISP_REOR_VIQ_CPU_TX_EGRESS,
        DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);


    /* VIQ for CPU_RX_copy - 19 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(cpu_rx_runner_image), (IMAGE_1_CPU_RX_COPY_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 |
        IMAGE_1_CPU_IF_1_CPU_RX_COPY_THREAD_NUMBER << 12), dsptchr_viq_dest_reor, dsptchr_viq_delayed, DISP_REOR_VIQ_CPU_RX_COPY,
        DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);

    /* configure all rnr groups */
    for (core_index = 0; core_index < GROUPED_EN_SEGMENTS_NUM; core_index++)
    {
        /* setting group 1 - wan direct */
        if (rdp_core_to_image_map[core_index] == wan_direct_runner_image)
        {
            dispatcher_reorder_rnr_group_init(&cfg, 1, core_index, (1<< IMAGE_1_CPU_IF_1_WAN_DIRECT_THREAD_NUMBER), (IMAGE_1_DIRECT_PROCESSING_PD_TABLE_ADDRESS >> 3), 0);
        }
    }

    /* mapping viq to rnr group */
    /* send all traffic via "direct flow" */
    cfg.rnr_grp_num = 2;
    cfg.dsptchr_rnr_group_list[0].queues_mask = 0x0000;
    cfg.dsptchr_rnr_group_list[1].queues_mask = ((1 << BBH_ID_NUM) - 1) | (1 << (BBH_ID_NUM + BBH_ID_PON));;

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
    cfg.queue_en_vec = disp_reor_enable_vec(&cfg);

    rc = drv_dis_reor_queues_init();
    rc = rc ? rc : drv_dis_reor_tasks_to_rg_init();
    rc = rc ? rc : drv_dis_reor_free_linked_list_init();
    rc = rc ? rc : drv_dis_reor_cfg(&cfg);
    rc = rc ? rc : ag_drv_dsptchr_load_balancing_lb_cfg_set(1, 2); /*set strict task priority mode in dispatcher*/
    return rc;
}

static int dqm_init(void)
{
    bdmf_error_t rc;
    bdmf_phys_addr_t fpm_base_phys_addr;
    uint32_t buf_size;

    fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
    buf_size = g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[BBH_ID_FIRST].bufsize;
    fpm_base_phys_addr = (fpm_base_phys_addr >> 8);
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
    qm_wred_profile_cfg wred_cfg = {};
    uint32_t idx = 0;

    qm_init_cfg init_cfg =
    {
        .is_counters_read_clear = 0,   /* TRUE=read-clear counter access mode */
        .is_drop_counters_enable = 1,  /* TRUE=drop counters enable. FALSE=max occupancy holder */
        .is_close_agg_disable = 0,
        .ddr_sop_offset = {0, 18}     /* DDR SoP offsets profile0 - everythig except multicast , profile1 - for multicast, */
    };

    fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
    GET_ADDR_HIGH_LOW(addr_hi, addr_lo, fpm_base_phys_addr);

    init_cfg.fpm_base = (addr_hi << 24) | (addr_lo >> 8); /* in 256B resolution */
    init_cfg.fpm_buf_size = g_bbh_tx_ddr_profile->bbh_tx_ddr_cfg[BBH_ID_FIRST].bufsize;

    init_cfg.num_fpm_ug = NUM_OF_FPM_UG;          /* Number of valid elements in fpm_ug_thr array */

    /* Default configuration - no WLAN */

    init_cfg.fpm_ug_thr[0].higher_thr = DS_FPM_UG_DEFAULT;  /* DS */
    init_cfg.fpm_ug_thr[0].lower_thr = init_cfg.fpm_ug_thr[0].mid_thr = init_cfg.fpm_ug_thr[0].higher_thr;

    init_cfg.fpm_ug_thr[1].higher_thr = US_FPM_UG_DEFAULT;  /* US */
    init_cfg.fpm_ug_thr[1].lower_thr = init_cfg.fpm_ug_thr[1].mid_thr = init_cfg.fpm_ug_thr[1].higher_thr;

    /* WLAN group in future solution*/
    init_cfg.fpm_ug_thr[2].higher_thr = WLAN_FPM_UG_DEFAULT;  /* WLAN */
    init_cfg.fpm_ug_thr[2].lower_thr = init_cfg.fpm_ug_thr[2].mid_thr = init_cfg.fpm_ug_thr[2].higher_thr;

    /* All pass group - for CPU and WLAN in short term solution */
    init_cfg.fpm_ug_thr[3].higher_thr = TOTAL_FPM_TOKENS;
    init_cfg.fpm_ug_thr[3].lower_thr = init_cfg.fpm_ug_thr[3].mid_thr = init_cfg.fpm_ug_thr[3].higher_thr;

    rc = rc ? rc : drv_qm_init(&init_cfg);

	/* Init spedific WRED profiles*/

    /* CPU_RX WRED profile */
    wred_cfg.min_thr0 = 8000;
    wred_cfg.min_thr1 = 24000;
    wred_cfg.max_thr0 = 8000;
    wred_cfg.max_thr1 = 24000;
    rc = rc ? rc :ag_drv_qm_wred_profile_cfg_set(QM_WRED_PROFILE_CPU_RX, &wred_cfg);

    if (!rc)
        ag_drv_qm_qm_pd_cong_ctrl_set(DEF_QM_PD_CONGESTION_CONTROL_THRESHOLD);


    ag_drv_qm_copy_decision_profile_set(QM_COPY_DEC_PROFILE_LOW, QM_COPY_DEC_MAX_QUEUE_OCCUPANCY , 0);
    ag_drv_qm_copy_decision_profile_set(QM_COPY_DEC_PROFILE_HIGH, QM_COPY_DEC_MAX_QUEUE_OCCUPANCY , 1);


    /* Initializing DQM before any of QM's queues */
    rc = rc ? rc : dqm_init();

    /* setup 1 queue */
    queue_cfg.wred_profile = 0;                            /* WRED profile */
    queue_cfg.copy_dec_profile = QM_COPY_DEC_PROFILE_LOW;  /* Copy decision profile */
    queue_cfg.fpm_ug = FPM_US_UG;                          /* FPM UG */
    queue_cfg.ddr_copy_disable = 0;                        /* TRUE=never copy to DDR */
    queue_cfg.copy_to_ddr = US_DDR_COPY_ENABLE;            /* TRUE=Force copy to DDR */
    queue_cfg.aggregation_disable = 1;
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

    for (idx = QM_QUEUE_DS_START; idx <= QM_QUEUE_DS_END; idx++)
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

    for (idx = QM_QUEUE_DS_END + 1; idx < QM_NUM_QUEUES; idx++)
    {
        if ((idx == QM_QUEUE_DROP) || (idx == QM_QUEUE_CPU_RX) || (idx == QM_QUEUE_CPU_RX_COPY_NORMAL) || (idx == QM_QUEUE_CPU_RX_COPY_EXCLUSIVE))
            continue;

        rc = rc ? rc : drv_qm_queue_config(idx, &queue_cfg);
        rc = rc ? rc : drv_qm_queue_enable(idx);
    }

    /* configure WRED profile for CPU_RX queue */
    queue_cfg.wred_profile = QM_WRED_PROFILE_CPU_RX;

    rc = rc ? rc : drv_qm_queue_config(QM_QUEUE_CPU_RX, &queue_cfg);
    rc = rc ? rc : drv_qm_queue_enable(QM_QUEUE_CPU_RX);

    queue_cfg.copy_dec_profile = QM_COPY_DEC_PROFILE_HIGH;             /* Copy decision profile */
    queue_cfg.wred_profile = QM_WRED_PROFILE_CPU_RX;
    queue_cfg.ddr_copy_disable = QM_COPY_DEC_PROFILE_HIGH;

    rc = rc ? rc : drv_qm_queue_config(QM_QUEUE_CPU_RX_COPY_NORMAL, &queue_cfg);
    rc = rc ? rc : drv_qm_queue_enable(QM_QUEUE_CPU_RX_COPY_NORMAL);

    queue_cfg.wred_profile = QM_WRED_PROFILE_CPU_RX;
    queue_cfg.ddr_copy_disable = 0;

    rc = rc ? rc : drv_qm_queue_config(QM_QUEUE_CPU_RX_COPY_EXCLUSIVE, &queue_cfg);
    rc = rc ? rc : drv_qm_queue_enable(QM_QUEUE_CPU_RX_COPY_EXCLUSIVE);

    /* ds group 0 */
    rnr_group_cfg.start_queue = QM_QUEUE_DS_START; /* should be from dpi params */
    rnr_group_cfg.end_queue = QM_QUEUE_DS_END; /* should be from dpi params */
    rnr_group_cfg.pd_fifo_base = (IMAGE_0_DS_TM_PD_FIFO_TABLE_ADDRESS >> 3);
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
    rnr_group_cfg.rnr_task = IMAGE_1_CPU_IF_1_CPU_RX_THREAD_NUMBER;
    rnr_group_cfg.upd_fifo_base = (IMAGE_1_UPDATE_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
    rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
    rnr_group_cfg.rnr_bb_id = get_runner_idx(cpu_rx_runner_image);
    rnr_group_cfg.rnr_enable = 1;
    rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_3, &rnr_group_cfg);

    /* cpu - group 8 (cpu_rx_copy fifo task) */
    rnr_group_cfg.start_queue = QM_QUEUE_CPU_RX_COPY_EXCLUSIVE; /* 2 queues for CPU RX COPY*/
    rnr_group_cfg.end_queue = QM_QUEUE_CPU_RX_COPY_NORMAL;
    rnr_group_cfg.pd_fifo_base = (IMAGE_1_CPU_RX_COPY_PD_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.rnr_task = IMAGE_1_CPU_IF_1_CPU_RX_COPY_THREAD_NUMBER;
    rnr_group_cfg.upd_fifo_base = (IMAGE_1_CPU_RX_COPY_UPDATE_FIFO_TABLE_ADDRESS >> 3);
    rnr_group_cfg.pd_fifo_size = qm_pd_fifo_size_2;
    rnr_group_cfg.upd_fifo_size = qm_update_fifo_size_8;
    rnr_group_cfg.rnr_bb_id = get_runner_idx(cpu_rx_runner_image);
    rnr_group_cfg.rnr_enable = 1;
    rc = rc ? rc : ag_drv_qm_rnr_group_cfg_set(qm_rnr_group_8, &rnr_group_cfg);
    return rc;
}

static void rdp_block_enable(void)
{
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
    for (idx = BBH_ID_FIRST; idx <= BBH_ID_LAST_XLMAC; idx++)
    {
        rc = rc ? rc : ag_drv_xlif_rx_if_if_dis_set(idx, 0);
        rc = rc ? rc : ag_drv_xlif_tx_if_if_enable_set(idx, 0, 0);
        rc = rc ? rc : ag_drv_xlif_tx_if_urun_port_enable_set(idx, 0);
        rc = rc ? rc : ag_drv_xlif_tx_if_tx_threshold_set(idx, 12);
    }
    for (idx = BBH_ID_FIRST; idx < BBH_ID_PON; idx++)
    {
        rc = rc ? rc : ag_drv_bbh_rx_general_configuration_enable_set(idx, 1, 1);
    }

    /* enable DISP_REOR */
    rc = rc ? rc : ag_drv_dsptchr_reorder_cfg_dsptchr_reordr_cfg_set(1, 1);

    /* enable QM */
    rc = rc ? rc : ag_drv_qm_enable_ctrl_set(&qm_enable);
    rc = rc ? rc : ag_drv_ubus_mstr_en_set(0, 1);
    rc = rc ? rc : ag_drv_ubus_mstr_en_set(1, 1);

    /* enable RNR */
    for (idx = 0; idx <= RNR_LAST; idx++)
        rc = rc ? rc : ag_drv_rnr_regs_rnr_enable_set(idx, 1);

    if (rc)
        BDMF_TRACE_ERR("Failed to enable rdp blocks\n");
}

static int _data_path_init(dpi_params_t *dpi_params, int is_basic)
{
    bdmf_error_t rc;
    p_dpi_cfg = dpi_params;

    bdmf_trace("INFO: %s#%d: Start.\n", __FUNCTION__, __LINE__);
#if defined(RDP_SIM) && !defined(XRDP_EMULATION)
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

#ifdef CONFIG_BCM_UBUS_DECODE_REMAP
extern unsigned int g_board_size_power_of_2;
#endif

int data_path_init(dpi_params_t *dpi_params)
{
#ifdef CONFIG_BCM_UBUS_DECODE_REMAP
	void *fpm_pool_addr;
	uint32_t fpm_pool_size;
	int rc;

	if ((rc = drv_rnr_quad_ubus_decode_wnd_cfg(DECODE_WIN0, 0, g_board_size_power_of_2, UBUS_PORT_ID_BIU, IS_DDR_COHERENT)))
	{
		printk("Error %s line[%d] size[%d]: \n",__FILE__, __LINE__,
				g_board_size_power_of_2);
		return rc;
	}

#ifdef CONFIG_BCM_FPM_COHERENCY_EXCLUDE
	/* FPM pool memory must not be cache coherent since CCI-400 can't handle 10Gig bandwidth */
	if (BcmMemReserveGetByName(FPMPOOL_BASE_ADDR_STR, &fpm_pool_addr, &fpm_pool_size))
	{
		printk("misc_hw_init: failed to get fpm_pool base address\n");
		return -1;
	}
	if ((rc = drv_rnr_quad_ubus_decode_wnd_cfg(DECODE_WIN1, virt_to_phys(fpm_pool_addr), log2_32(fpm_pool_size), UBUS_PORT_ID_BIU, CACHE_BIT_OFF)))
	{
		printk("%s:%d: failed to configure ubus decode window: \n",__FILE__, __LINE__);
		return rc;
	}
#endif /* CONFIG_BCM_FPM_COHERENCY_EXCLUDE */
#endif /* CONFIG_BCM_GLB_COHERENCY */

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


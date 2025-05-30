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

#if defined(_CFE_)
#include "lib_types.h"
#include "lib_malloc.h"
#include "lib_printf.h"
#include "lib_string.h"
extern void cfe_usleep(int);
#define xrdp_memset lib_memset
#define xrdp_memcpy lib_memcpy
#define xrdp_alloc(_a) KMALLOC(_a, 32)
#define xrdp_usleep(_a) cfe_usleep(_a)
#define BDMF_TRACE_ERR xprintf
#define bdmf_ioremap(_a, _b) _a
#elif defined(__UBOOT__)
#include "linux/delay.h"
#include "malloc.h"
#include "string.h"
#include "stdio.h"
#define xrdp_memset memset
#define xrdp_memcpy memcpy
#define xrdp_alloc(_a) malloc(_a)
#define xrdp_usleep(_a) udelay(_a)
#define BDMF_TRACE_ERR printf
#endif
#include "rdd.h"
#include "rdd_defs.h"
#include "rdd_runner_defs_auto.h"
#include "data_path_init.h"
#include "rdd_init.h"
#include "rdp_platform.h"
#include "rdp_drv_sbpm.h"
#include "xrdp_drv_psram_ag.h"
#include "rdp_common.h"
#include "rdp_drv_dma.h"
#include "rdp_drv_rnr.h"
#include "rdp_drv_dis_reor.h"
#include "rdp_drv_bbh_tx.h"
#include "xrdp_drv_drivers_common_ag.h"
#include "xrdp_drv_xlif_rx_if_ag.h"
#include "xrdp_drv_xlif_tx_if_ag.h"
#include "xrdp_drv_ubus_mstr_ag.h"
#include "xrdp_drv_ubus_slv_ag.h"
#include "rdp_drv_qm.h"
#include "bdmf_data_types.h"
#include "bdmf_errno.h"

#if defined(CONFIG_BCM_GLB_COHERENCY)
#include <board.h>
#include "bcm_misc_hw_init.h"
#define DECODE_WIN0 0
#define DECODE_WIN1 1
#define CACHE_BIT_OFF 0
#define CACHE_BIT_ON 1
#endif

dpi_params_t *p_dpi_cfg;

extern uint32_t total_length_bytes[];
extern rdpa_emac bbh_id_to_rdpa_emac[BBH_ID_NUM];

bbh_rx_sdma_profile_t *g_bbh_rx_sdma_profile;
bbh_tx_dma_profile_t *g_bbh_tx_dma_profile;
bbh_tx_sdma_profile_t *g_bbh_tx_sdma_profile;
bbh_tx_ddr_profile_t *g_bbh_tx_ddr_profile;
queue_to_rnr_t g_lan_queue_to_rnr[1] = { {RNR0, RNR0} };
pd_fifo_size_t g_lan_pd_fifo_size[1] = { {BBH_TX_DS_PD_FIFO_SIZE_0, BBH_TX_DS_PD_FIFO_SIZE_1} };
pd_wkup_threshold_t g_lan_pd_wkup_threshold[2] = { {0, 0}, {0, 0} };
pd_bytes_threshold_t g_lan_pd_bytes_threshold[1] = { {0, 0} };
dma_id_e g_dma_source[] = {BB_ID_DMA0, BB_ID_DMA1, BB_ID_SDMA0, BB_ID_SDMA1};

#define XFI_EMAC_PORT 0

typedef enum
{
    wan_pon_no_xfi_profile_id,
    wan_pon_xfi0_profile_id,
    wan_pon_xfi4_profile_id,
    profiles_num,
} profile_id_t;

queue_to_rnr_t g_wan_queue_to_rnr[TX_QEUEU_PAIRS] = {
          {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0},
          {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0}, {RNR0, RNR0} };

bbh_to_dma_x_t  g_bbh_to_dma_x[BBH_ID_NUM] = {
          {BBH_ID_4, DMA1_ID},
          {BBH_ID_1, DMA1_ID},
          {BBH_ID_2, DMA1_ID},
          {BBH_ID_3, DMA0_ID},
          {BBH_ID_0, DMA1_ID},
          {BBH_ID_5, DMA0_ID},
          {BBH_ID_6, DMA0_ID},
          {BBH_ID_7, DMA0_ID},
          {BBH_ID_PON, DMA0_ID} };


bbh_id_e g_dma_to_bbh_x[profiles_num][NUM_OF_DMA][NUM_OF_PERIPHERALS_PER_DMA] = {
         {{BBH_ID_3, BBH_ID_5, BBH_ID_6, BBH_ID_7, BBH_ID_PON, BBH_ID_NULL},
          {BBH_ID_4, BBH_ID_1, BBH_ID_2, BBH_ID_0, BBH_ID_NULL, BBH_ID_NULL}},
         {{BBH_ID_3, BBH_ID_5, BBH_ID_6, BBH_ID_7, BBH_ID_PON, BBH_ID_NULL},
          {BBH_ID_4, BBH_ID_1, BBH_ID_2, BBH_ID_0, BBH_ID_NULL, BBH_ID_NULL}},
         {{BBH_ID_3, BBH_ID_5, BBH_ID_6, BBH_ID_7, BBH_ID_PON, BBH_ID_NULL},
      {BBH_ID_4, BBH_ID_1, BBH_ID_2, BBH_ID_0, BBH_ID_NULL, BBH_ID_NULL}}};

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
         {BB_ID_RX_BBH_0, BB_ID_TX_BBH_0},
         {BB_ID_RX_BBH_1, BB_ID_TX_BBH_1},
         {BB_ID_RX_BBH_2, BB_ID_TX_BBH_2},
         {BB_ID_RX_BBH_3, BB_ID_TX_BBH_3},
         {BB_ID_RX_BBH_4, BB_ID_TX_BBH_4},
         {BB_ID_RX_BBH_5, BB_ID_TX_BBH_5},
         {BB_ID_RX_BBH_6, BB_ID_TX_BBH_6},
         {BB_ID_RX_BBH_7, BB_ID_TX_BBH_7},
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
         { {DMA1_U_THRESH_IN_BBH_ID_4_VALUE, DMA1_U_THRESH_OUT_BBH_ID_4_VALUE},
           {SDMA1_U_THRESH_IN_BBH_ID_4_VALUE, SDMA1_U_THRESH_OUT_BBH_ID_4_VALUE} },
         { {DMA1_U_THRESH_IN_BBH_ID_1_VALUE, DMA1_U_THRESH_OUT_BBH_ID_1_VALUE},
           {SDMA1_U_THRESH_IN_BBH_ID_1_VALUE, SDMA1_U_THRESH_OUT_BBH_ID_1_VALUE} },
         { {DMA1_U_THRESH_IN_BBH_ID_2_VALUE, DMA1_U_THRESH_OUT_BBH_ID_2_VALUE},
           {SDMA1_U_THRESH_IN_BBH_ID_2_VALUE, SDMA1_U_THRESH_OUT_BBH_ID_2_VALUE} },
         { {DMA0_U_THRESH_IN_BBH_ID_3_VALUE, DMA0_U_THRESH_OUT_BBH_ID_3_VALUE},
           {SDMA0_U_THRESH_IN_BBH_ID_3_VALUE, SDMA0_U_THRESH_OUT_BBH_ID_3_VALUE} },
         { {DMA1_U_THRESH_IN_BBH_ID_0_VALUE, DMA1_U_THRESH_OUT_BBH_ID_0_VALUE},
           {SDMA1_U_THRESH_IN_BBH_ID_0_VALUE, SDMA1_U_THRESH_OUT_BBH_ID_0_VALUE} },
         { {DMA0_U_THRESH_IN_BBH_ID_5_VALUE, DMA0_U_THRESH_OUT_BBH_ID_5_VALUE},
           {SDMA0_U_THRESH_IN_BBH_ID_5_VALUE, SDMA0_U_THRESH_OUT_BBH_ID_5_VALUE} },
         { {DMA0_U_THRESH_IN_BBH_ID_6_VALUE, DMA0_U_THRESH_OUT_BBH_ID_6_VALUE},
           {SDMA0_U_THRESH_IN_BBH_ID_6_VALUE, SDMA0_U_THRESH_OUT_BBH_ID_6_VALUE} },
         { {DMA0_U_THRESH_IN_BBH_ID_6_VALUE, DMA0_U_THRESH_OUT_BBH_ID_6_VALUE},
           {SDMA0_U_THRESH_IN_BBH_ID_6_VALUE, SDMA0_U_THRESH_OUT_BBH_ID_6_VALUE} },
         { {DMA0_U_THRESH_IN_BBH_ID_PON_VALUE, DMA0_U_THRESH_OUT_BBH_ID_PON_VALUE},
           {SDMA0_U_THRESH_IN_BBH_ID_PON_VALUE, SDMA0_U_THRESH_OUT_BBH_ID_PON_VALUE} } };


strict_priority_t g_dma_strict_priority[BBH_ID_NUM][2] = {
         { {DMA1_STRICT_PRI_RX_BBH_ID_4_VALUE, DMA1_STRICT_PRI_TX_BBH_ID_4_VALUE},
           {SDMA1_STRICT_PRI_RX_BBH_ID_4_VALUE, SDMA1_STRICT_PRI_TX_BBH_ID_4_VALUE} },
         { {DMA0_STRICT_PRI_RX_BBH_ID_1_VALUE, DMA0_STRICT_PRI_TX_BBH_ID_1_VALUE},
           {SDMA0_STRICT_PRI_RX_BBH_ID_1_VALUE, SDMA0_STRICT_PRI_TX_BBH_ID_1_VALUE} },
         { {DMA1_STRICT_PRI_RX_BBH_ID_2_VALUE, DMA1_STRICT_PRI_TX_BBH_ID_2_VALUE},
           {SDMA1_STRICT_PRI_RX_BBH_ID_2_VALUE, SDMA1_STRICT_PRI_TX_BBH_ID_2_VALUE} },
         { {DMA1_STRICT_PRI_RX_BBH_ID_3_VALUE, DMA1_STRICT_PRI_TX_BBH_ID_3_VALUE},
           {SDMA1_STRICT_PRI_RX_BBH_ID_3_VALUE, SDMA1_STRICT_PRI_TX_BBH_ID_3_VALUE} },
         { {DMA1_STRICT_PRI_RX_BBH_ID_0_VALUE, DMA1_STRICT_PRI_TX_BBH_ID_0_VALUE},
           {SDMA1_STRICT_PRI_RX_BBH_ID_0_VALUE, SDMA1_STRICT_PRI_TX_BBH_ID_0_VALUE} },
         { {DMA0_STRICT_PRI_RX_BBH_ID_5_VALUE, DMA0_STRICT_PRI_TX_BBH_ID_5_VALUE},
           {SDMA0_STRICT_PRI_RX_BBH_ID_5_VALUE, SDMA0_STRICT_PRI_TX_BBH_ID_5_VALUE} },
         { {DMA0_STRICT_PRI_RX_BBH_ID_6_VALUE, DMA0_STRICT_PRI_TX_BBH_ID_6_VALUE},
           {SDMA0_STRICT_PRI_RX_BBH_ID_6_VALUE, SDMA0_STRICT_PRI_TX_BBH_ID_6_VALUE} },
         { {DMA0_STRICT_PRI_RX_BBH_ID_6_VALUE, DMA0_STRICT_PRI_TX_BBH_ID_6_VALUE},
           {SDMA0_STRICT_PRI_RX_BBH_ID_6_VALUE, SDMA0_STRICT_PRI_TX_BBH_ID_6_VALUE} },
         { {DMA0_STRICT_PRI_RX_BBH_ID_PON_VALUE, DMA0_STRICT_PRI_TX_BBH_ID_PON_VALUE},
           {SDMA0_STRICT_PRI_RX_BBH_ID_PON_VALUE, SDMA0_STRICT_PRI_TX_BBH_ID_PON_VALUE} } };


rr_weight_t g_dma_rr_weight[BBH_ID_NUM][2] = {
         { {DMA1_RR_WEIGHT_RX_BBH_ID_4_VALUE, DMA1_RR_WEIGHT_TX_BBH_ID_4_VALUE},
           {SDMA1_RR_WEIGHT_RX_BBH_ID_4_VALUE, SDMA1_RR_WEIGHT_TX_BBH_ID_4_VALUE} },
         { {DMA0_RR_WEIGHT_RX_BBH_ID_1_VALUE, DMA0_RR_WEIGHT_TX_BBH_ID_1_VALUE},
           {SDMA0_RR_WEIGHT_RX_BBH_ID_1_VALUE, SDMA0_RR_WEIGHT_TX_BBH_ID_1_VALUE} },
         { {DMA1_RR_WEIGHT_RX_BBH_ID_2_VALUE, DMA1_RR_WEIGHT_TX_BBH_ID_2_VALUE},
           {SDMA1_RR_WEIGHT_RX_BBH_ID_2_VALUE, SDMA1_RR_WEIGHT_TX_BBH_ID_2_VALUE} },
         { {DMA1_RR_WEIGHT_RX_BBH_ID_3_VALUE, DMA1_RR_WEIGHT_TX_BBH_ID_3_VALUE},
           {SDMA1_RR_WEIGHT_RX_BBH_ID_3_VALUE, SDMA1_RR_WEIGHT_TX_BBH_ID_3_VALUE} },
         { {DMA1_RR_WEIGHT_RX_BBH_ID_0_VALUE, DMA1_RR_WEIGHT_TX_BBH_ID_0_VALUE},
           {SDMA1_RR_WEIGHT_RX_BBH_ID_0_VALUE, SDMA1_RR_WEIGHT_TX_BBH_ID_0_VALUE} },
         { {DMA0_RR_WEIGHT_RX_BBH_ID_5_VALUE, DMA0_RR_WEIGHT_TX_BBH_ID_5_VALUE},
           {SDMA0_RR_WEIGHT_RX_BBH_ID_5_VALUE, SDMA0_RR_WEIGHT_TX_BBH_ID_5_VALUE} },
         { {DMA0_RR_WEIGHT_RX_BBH_ID_6_VALUE, DMA0_RR_WEIGHT_TX_BBH_ID_6_VALUE},
           {SDMA0_RR_WEIGHT_RX_BBH_ID_6_VALUE, SDMA0_RR_WEIGHT_TX_BBH_ID_6_VALUE} },
         { {DMA0_RR_WEIGHT_RX_BBH_ID_6_VALUE, DMA0_RR_WEIGHT_TX_BBH_ID_6_VALUE},
           {SDMA0_RR_WEIGHT_RX_BBH_ID_6_VALUE, SDMA0_RR_WEIGHT_TX_BBH_ID_6_VALUE} },
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
        cfg.max_pkt_size[i] = p_dpi_cfg->max_pkt_size;
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



static int sbpm_init(void)
{
    bdmf_error_t rc = BDMF_ERR_OK;
    uint16_t base_addr;
    uint16_t init_offset;

    /* base address and offset */
    base_addr = SBPM_BASE_ADDRESS;
    init_offset = SBPM_INIT_OFFSET;
    rc = rc ? rc : ag_drv_sbpm_regs_init_free_list_set(base_addr, init_offset, 0, 0);

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


        config.rnr_cfg[0].rnr_src_id = get_runner_idx(cfe_core_runner_image);
        config.rnr_cfg[0].tcont_addr = 0;
        config.rnr_cfg[0].task_number = IMAGE_0_CFE_CORE_CPU_TX_THREAD_NUMBER;
        config.rnr_cfg[0].ptr_addr = IMAGE_0_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS >> 3;
        if ((bbh_id == 0) || (bbh_id == 4))
                config.rnr_cfg[0].ptr_addr = (IMAGE_0_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS +
                    ((4 - bbh_id) * sizeof(BBH_TX_EGRESS_COUNTER_ENTRY_STRUCT))) >> 3;
            else
                config.rnr_cfg[0].ptr_addr = (IMAGE_0_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS + (bbh_id * sizeof(BBH_TX_EGRESS_COUNTER_ENTRY_STRUCT))) >> 3;

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
    rdd_init_params_t rdd_init_params = {0};

    drv_rnr_cores_addr_init();
    drv_rnr_mem_init();
    drv_rnr_load_microcode();

    /* scheduler configuration */
    ag_drv_rnr_regs_cfg_sch_cfg_set(get_runner_idx(0), DRV_RNR_16SP);

    fpm_base_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_pkt_base_virt);
    GET_ADDR_HIGH_LOW(addr_hi, addr_lo, fpm_base_phys_addr);


    rnr_dma_cfg.ddr.dma_base = (addr_hi << 12) | (addr_lo >> 20);


    rnr_dma_cfg.ddr.dma_static_offset = 0;
    rnr_dma_cfg.psram.dma_base = ((RU_BLK(PSRAM_MEM).addr[0] + RU_REG_OFFSET(PSRAM_MEM, MEMORY_DATA)) >> 20);
    rnr_dma_cfg.psram.dma_buf_size = DMA_BUFSIZE_128;
    rnr_dma_cfg.psram.dma_static_offset = 0;

    rc = drv_rnr_dma_cfg(&rnr_dma_cfg);

    /* ToDo: fill rdd_init_params */

    rdd_init_params.is_basic = is_basic;

    rc = rc ? rc : rdd_data_structures_init(&rdd_init_params);
    rc = rc ? rc : rnr_frequency_set(p_dpi_cfg->runner_freq);
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
            dispatcher_reorder_viq_init(&cfg, &congs_init, (BB_ID_RX_BBH_4 - (2 * (4 - bbh_id))), dsptchr_viq_bbh_target_addr_normal, dsptchr_viq_dest_disp,
                dsptchr_viq_delayed, (4 - bbh_id), DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 1);
        else if (bbh_id == BBH_ID_PON)
            /* normal viq for bbh-rx */
            dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_PON_ETH, dsptchr_viq_bbh_target_addr_normal, dsptchr_viq_dest_disp,
                dsptchr_viq_delayed, bbh_id, DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 1);
        else if (bbh_id == (BBH_ID_PON - 1))
            /* normal viq for bbh-rx */
            dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_BBH_7, dsptchr_viq_bbh_target_addr_normal, dsptchr_viq_dest_disp,
                dsptchr_viq_delayed, bbh_id, DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 1);
        else
            /* normal viq for bbh-rx */
            dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_BBH_0 + (2 * bbh_id), dsptchr_viq_bbh_target_addr_normal, dsptchr_viq_dest_disp,
                dsptchr_viq_delayed, bbh_id, DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 1);

        if (bbh_id == BBH_ID_PON)
            dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_PON_ETH, dsptchr_viq_bbh_target_addr_excl, dsptchr_viq_dest_disp,
                dsptchr_viq_delayed, bbh_id + BBH_ID_NUM, DSPTCHR_EXCLUSIVE_WAN_GUARANTEED_BUFFERS * (bbh_id == BBH_ID_PON), common_max_limit, 1);
        else if (bbh_id == (BBH_ID_PON - 1))
            dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_BBH_7, dsptchr_viq_bbh_target_addr_excl, dsptchr_viq_dest_disp,
                dsptchr_viq_delayed, bbh_id + BBH_ID_NUM, DSPTCHR_EXCLUSIVE_WAN_GUARANTEED_BUFFERS * (bbh_id == BBH_ID_PON), common_max_limit, 1);
        else
            dispatcher_reorder_viq_init(&cfg, &congs_init, BB_ID_RX_BBH_0 + (2 * bbh_id), dsptchr_viq_bbh_target_addr_excl, dsptchr_viq_dest_disp,
                dsptchr_viq_delayed, bbh_id + BBH_ID_NUM, DSPTCHR_EXCLUSIVE_WAN_GUARANTEED_BUFFERS * (bbh_id == BBH_ID_PON), common_max_limit, 1);
    }

    /* VIQ for CPU_TX egress - 18 */
    dispatcher_reorder_viq_init(&cfg, &congs_init, get_runner_idx(cfe_core_runner_image), (IMAGE_0_CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS >> 3 |
        IMAGE_0_CFE_CORE_CPU_TX_THREAD_NUMBER << 12), dsptchr_viq_dest_reor, dsptchr_viq_delayed, DISP_REOR_VIQ_CPU_TX_EGRESS,
        DSPTCHR_NORMAL_GUARANTEED_BUFFERS, common_max_limit, 0);



    /* configure all rnr groups */
    for (core_index = 0; core_index < NUM_OF_RUNNER_CORES; core_index++)
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
    cfg.dsptchr_rnr_group_list[1].queues_mask = (1 << BBH_ID_NUM) - 1;

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
    return rc;
}



static int qm_init(void)
{
    qm_ubus_slave ubus_slave =
    {
      .vpb_base = QM_UBUS_SLV_VPB_BASE,
      .vpb_mask = QM_UBUS_SLV_VPB_MASK,
      .apb_base = QM_UBUS_SLV_APB_BASE,
      .apb_mask = QM_UBUS_SLV_APB_MASK,
      .dqm_base = QM_UBUS_SLV_DQM_BASE,
      .dqm_mask = QM_UBUS_SLV_DQM_MASK,
    };
    return ag_drv_qm_ubus_slave_set(&ubus_slave);
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

    /* enable runner 0 */
    rc = rc ? rc : ag_drv_rnr_regs_rnr_enable_set(0, 1);

    if (rc)
        BDMF_TRACE_ERR("Failed to enable rdp blocks\n");
}

static int _data_path_init(dpi_params_t *dpi_params, int is_basic)
{
    bdmf_error_t rc;
    p_dpi_cfg = dpi_params;

    printf("INFO: %s#%d: Start.\n", __FUNCTION__, __LINE__);
    ubus_bridge_init();

    rc = bbh_profiles_init();
    if (rc)
    {
        printf("Error: %s#%d: Failed to init BBH profiles.\n", __FUNCTION__, __LINE__);
        return rc;
    }

    printf("INFO: %s#%d: Driver init.\n", __FUNCTION__, __LINE__);

    rc = qm_init();
    rc = rc ? rc : runner_init(is_basic);
    rc = rc ? rc : bbh_rx_init();
    rc = rc ? rc : bbh_tx_init();
    rc = rc ? rc : sbpm_init();
    rc = rc ? rc : dma_sdma_init();
    rc = rc ? rc : dispatcher_reorder_init();
    if (rc)
    {
        printf("Error: %s#%d: rc = %d, End.\n", __FUNCTION__, __LINE__, rc);
        return rc;
    }

    printf("INFO: %s#%d: Enable Accelerators.\n", __FUNCTION__, __LINE__);
    rdp_block_enable();
    printf("INFO: %s#%d: End.\n", __FUNCTION__, __LINE__);
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


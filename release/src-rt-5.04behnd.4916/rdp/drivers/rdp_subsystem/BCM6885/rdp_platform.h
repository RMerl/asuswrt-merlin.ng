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

#ifndef RDP_PLATFORM_H_INCLUDED
#define RDP_PLATFORM_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include "rdd_map_auto.h"
extern int drv_qm_get_tm_core_per_image(int image_index);
#define RNR_FREQ_IN_MHZ 1250
#define UBUS_SLV_FREQ_IN_MHZ 1000 


/* NATC */
#ifdef PATH_STAT_WA
#define NATC_TABLE_SIZE_128 128
#endif

/* CNPL */

#ifdef POLICER_SHIFT_N_WA
/* note i dont divide by 1000 as it should be because of policer bug in A0 */
#define CNPL_PERIODIC_UPDATE_US        (CNPL_PERIODIC_UPDATE_QUANTA_NS)
#else
/* equation is : something close to:
CNPL_PERIODIC_UPDATE_QUANTA_NS << n) >= _ns
_ns is CNPL_PERIODIC_UPDATE_US   * 1000
CNPL_PERIODIC_UPDATE_QUANTA_NS is 16,384 (16K) */

#define CNPL_PERIODIC_UPDATE_US    2097  /*n = 7*/
/*#define CNPL_PERIODIC_UPDATE_US  131 */ /* n = 3*/

#endif

/* TODO: implement */
#define IS_WAN_TX_PORT(bbh_id) (bbh_id == BBH_TX_WAN_ID)
#define IS_WAN_RX_PORT(bbh_id) (bbh_id == BBH_ID_PON)
#define IS_SDMA(dma_id)  (dma_id > DMA0_ID)
#define IS_DMA(dma_id)   (dma_id <= DMA0_ID)
#define IS_PROCESSING_RUNNER_IMAGE(i)  ((rdp_core_to_image_map[i] == processing0_runner_image) || \
                                        (rdp_core_to_image_map[i] == processing1_runner_image) || \
                                        (rdp_core_to_image_map[i] == processing2_runner_image) || \
                                        (rdp_core_to_image_map[i] == processing3_runner_image) || \
                                        (rdp_core_to_image_map[i] == processing4_runner_image) || \
                                        (rdp_core_to_image_map[i] == processing5_runner_image) || \
                                        (rdp_core_to_image_map[i] == processing6_runner_image))
#define IS_DS_TM_RUNNER_IMAGE(i)  ((rdp_core_to_image_map[i] == tm0_lan_runner_image) || (rdp_core_to_image_map[i] == tm2_reproc_runner_image))
#define IS_US_TM_RUNNER_IMAGE(i)  (rdp_core_to_image_map[i] == tm1_wan_runner_image)

#define MAX_NUM_OF_LLID 8
#define MAX_NUM_OF_QUEUES_IN_SCHED 32
#define MAX_TX_QUEUES__NUM_OF             448

#define DMA_MAX_READ_ON_THE_FLY ( 16 )
#define SDMA_MAX_READ_ON_THE_FLY ( 16 )

/* NATC */
#define NATC_CACHE_ENTRIES_NUM   512

/* HASH */
#define HASH_NUM_OF_ENGINES           4
#define HASH_NUM_OF_ENGINES_LOG2      2
#define HASH_NUM_OF_EFFECTIVE_ENGINES 4
#define HASH_NUM_OF_ENTRIES_IN_RAM    2048

/* VLAN */
#define RDPA_MAX_VLANS                  32     /**< Max number of VIDs */

/* TCAM table size (single entries) */
#define RDP_TCAM_TABLE_SIZE     2048

/* Size of TCAM entry in words.  128 bits = 4 words */
#define RDP_TCAM_ENTRY_SIZE_WORDS    4    


/* Max number of EPON queues supported by QM */
#define QM_NUM_EPON_QUEUES      32
/* Max number of reported queues supported by QM */
#define QM_NUM_REPORTED_QUEUES  32


/* BUF_MNG default thresholds */
#define WLAN_GROUP_RSV_TOKENS_PERCENTAGE    (15)
#define BE1_GROUP_RSV_TOKENS_PERCENTAGE     (0)
#define BE2_GROUP_RSV_TOKENS_PERCENTAGE     (0)
#define WIRED_GROUP_RSV_TOKENS_PERCENTAGE   (40)

#define WLAN_GROUP_MAX_TOKENS_PERCENTAGE    (100)
#define BE1_GROUP_MAX_TOKENS_PERCENTAGE     (100)
#define BE2_GROUP_MAX_TOKENS_PERCENTAGE     (100)
#define WIRED_GROUP_MAX_TOKENS_PERCENTAGE   (100)

#define WLAN_PORT_RSV_TOKENS                (2*1024)
#define WLAN_PORT_HIGH_PRIO_RSV_TOKENS      (1024)

#define FPM_PREFETCH_QM                     (32 * 32)
#define FPM_RESERVED_REQUIRE                (1024 * 15) 
#define TOTAL_RESERVED_FPM_TOKENS           (FPM_RESERVED_REQUIRE + FPM_PREFETCH_QM)

/* BBH_RX */
#define NUM_OF_INGRESS_LAYOUT_BLOCKS   1
#define NUM_OF_CHANNELS_PER_INGRESS    3

typedef enum dma_id_e
{
    DMA_COPY_ID = 0,
    DMA1_ID,
    DMA2_ID,
    DMA3_ID,
    DMA_NUM,
} dma_id_e;

typedef enum dma_target_e
{
    DMA_TARGET_DMA = 0,
    DMA_TARGET_SDMA,
    DMA_TARGET_NUM,
    DMA_TARGET_FIRST = DMA_TARGET_DMA,
} dma_target_e;


typedef enum bbh_id_e
{
    BBH_ID_0 = 0,
    BBH_ID_1,
    BBH_ID_2,
    BBH_ID_3,
    BBH_ID_4,
    BBH_ID_5,
    BBH_ID_PON,
    BBH_ID_NUM,
    BBH_ID_FIRST = BBH_ID_0,
    BBH_ID_LAST = BBH_ID_PON,
    BBH_ID_NULL = BBH_ID_NUM,
    BBH_ID_NUM_LAN = BBH_ID_5,
    BBH_ID_AE = BBH_ID_5,
    BBH_ID_QM_COPY = 0,
    BBH_TX_ID_LAN0,
    BBH_TX_ID_LAN1,
    BBH_TX_ID_LAN2,
    BBH_TX_ID_PON,
    BBH_TX_ID_NUM,
    BBH_TX_ID_FIRST = BBH_ID_QM_COPY,
    BBH_TX_ID_LAST = BBH_TX_ID_PON,
    BBH_TX_ID_NULL = BBH_ID_NULL
    
} bbh_id_e;

#define BB_ID_RX_BBH_AE BBH_ID_AE
#define TM_IDENTITY_HAS_MULTI_ETH_ENUMS 1

typedef enum tm_identifier_e
{
    TM_START = 0,
    TM_PON_DSL_AE = 0,
    TM_ETH_START = 1,
    TM_ETH_PORTS_0 = TM_ETH_START,
    TM_ETH_PORTS_1 = 2,
    TM_ETH_END = TM_ETH_PORTS_1,
    TM_ETH_SQ = 3,
    TM_MAX = 3,
    TM_MAX_GROUP = TM_ETH_END - TM_ETH_START + 1,
    TM_ERROR = 4,
    TM_STILL_NOT_DEFINED = 5,
    TM_NUM_OF_IDENTITY = 4
} tm_identifier_e;

static inline int tm_get_core_for_tm(tm_identifier_e tm_identity)
{
    switch (tm_identity)
    {
    case TM_PON_DSL_AE:
        return drv_qm_get_tm_core_per_image(tm1_wan_runner_image);

    case TM_ETH_PORTS_0:
        return drv_qm_get_tm_core_per_image(tm0_lan_runner_image);
    
    case TM_ETH_PORTS_1:
        return drv_qm_get_tm_core_per_image(tm2_reproc_runner_image);

    case TM_ETH_SQ:
        return drv_qm_get_tm_core_per_image(tm3_service_runner_image);
   
    default:
        return -1;
    }
}

typedef enum rnr_quad_id_e
{
    RNR_QUAD0_ID = 0,
    RNR_QUAD_ID_LAST = RNR_QUAD0_ID,
    NUM_OF_RNR_QUADS = RNR_QUAD_ID_LAST + 1,
} rnr_quad_id_e;

#define NUM_OF_CORES_IN_QUAD                        28
#define DRV_PARSER_MASKED_DA_FILTER_NUM             2
#define DRV_PARSER_DA_FILTER_NUM                    16
#define NUM_OF_RNR_WITH_PROFILING                   24

#define UBUS_PORT_ID_MEMC        1
#define UBUS_PORT_ID_BIU         2
#define UBUS_PORT_ID_PSRAM       16

typedef enum bac_if_id_e
{
    BACIF_0_HASH_ID,
    BACIF_0_CNPL_POLICER_ID,
    BACIF_1_CNPL_POLICER_ID,
    BACIF_0_CNPL_CNTRS_BB_ID,
    BACIF_1_CNPL_CNTRS_BB_ID,
    BACIF_0_NATC_ID,
    BACIF_1_NATC_ID,
    BACIF_2_NATC_ID,
    BACIF_3_NATC_ID,
    BACIF_0_TCAM_ID,
    BACIF_1_TCAM_ID,
    BACIF_ID_NUM,
    BACIF_ID_LAST = BACIF_1_TCAM_ID,
} bac_if_id_e;

#define SUPPORTED_NUM_OF_FPM_TOKENS  CONST_INT_256K
#define SERVICE_QUEUES_THREAD_NUMBER IMAGE_10_IMAGE_10_SERVICE_QUEUES_THREAD_NUMBER

#define RC_MAX_PMC_CORES 7

#define NUM_OF_PROCESSING_RUNNERS       24
#define NUM_OF_TM_RUNNERS               4
#define NUM_OF_CORES (NUM_OF_PROCESSING_RUNNERS + NUM_OF_TM_RUNNERS)
#define IS_PROCESSING_CORE_ID(index)    (index < NUM_OF_PROCESSING_RUNNERS)
#define NUM_OF_PROCESSING_RNR_CMN       3
#define NUM_OF_PP_RNR_PER_RNR_CMN       8


#define TM_CORE_RANGE(_first, _after_last) \
    do { \
          _first = NUM_OF_PROCESSING_RUNNERS; \
          _after_last = NUM_OF_CORES; \
    } while (0)

/* DISPATCHER */
/* DISP CONGESTION */
#define DIS_REOR_LINKED_LIST_BUFFER_NUM     2048
#define DSPTCHR_CONG_PARAM_INGRESS_NORMAL   2047
#define DSPTCHR_CONG_PARAM_EGRESS_NORMAL    2047
#define DSPTCHR_CONG_PARAM_HYST             255

#define DSPTCHR_VIRTUAL_QUEUE_NUM       9
#define DSPTCHR_CONG_PARAM_GLOBAL       (DIS_REOR_LINKED_LIST_BUFFER_NUM - DSPTCHR_VIRTUAL_QUEUE_NUM - 2)

#define DSPTCHR_FLL_1ST_LVL             1024
#define DSPTCHR_FLL_2ND_LVL             1500
#define DSPTCHR_FLL_HYST                16

#define DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ    (DIS_REOR_LINKED_LIST_BUFFER_NUM - 1)
#define DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ  8
#define DSPTCHR_GUARANTEED_MAX_LIMIT_PER_EXCLUS_VIQ  16
#define DSPTCHR_PRE_ALLOCATED_BUFFERS                9

#define NUM_OF_DISPATCHERS             3
#define NUM_OF_CMN_DISPATCHERS         1

typedef enum 
{
    DISPATCHER_CHAN_VIQ_IDX_0 = 0,
    DISPATCHER_CHAN_VIQ_IDX_FIRST = DISPATCHER_CHAN_VIQ_IDX_0,
    DISPATCHER_CHAN_VIQ_IDX_1 = 1,
    DISPATCHER_CHAN_VIQ_IDX_2 = 2, /* note: only 2 VIQ available for dispatcher channels that are part of 200G/400G NNI */
    DISPATCHER_CHAN_VIQ_IDX_3 = 3,
    DISPATCHER_CHAN_VIQ_IDX_4 = 4,
    DISPATCHER_CHAN_VIQ_IDX_5 = 5,
    DISPATCHER_CHAN_VIQ_IDX_6 = 6,
    DISPATCHER_CHAN_VIQ_IDX_7 = 7,
    DISPATCHER_CHAN_VIQ_IDX_8 = 8,
    DISPATCHER_CHAN_VIQ_IDX_LAST = DISPATCHER_CHAN_VIQ_IDX_8,
    DISPATCHER_CHAN_VIQ_NUM = DISPATCHER_CHAN_VIQ_IDX_LAST + 1,
    DISPATCHER_CHAN_VIQ_IDX_UNUSED = 255
} dispatcher_chan_viq_idx_e;

typedef enum
{
    RUNNER_TASK_GROUP_IDX_0 = 0,
    RUNNER_TASK_GROUP_IDX_1 = 1,
    RUNNER_TASK_GROUP_IDX_2 = 2,
    RUNNER_TASK_GROUP_IDX_3 = 3,
    RUNNER_TASK_GROUP_IDX_4 = 4,
    RUNNER_TASK_GROUP_IDX_5 = 5,
    RUNNER_TASK_GROUP_IDX_6 = 6,
    RUNNER_TASK_GROUP_IDX_7 = 7,
    RUNNER_TASK_GROUP_NUM  = RUNNER_TASK_GROUP_IDX_7 + 1,
    RUNNER_TASK_GROUP_NONE = 255
} runner_task_group_idx_e;

/* BBH_RX */
#define BB_ID_SUB_SYS_BASE 256
#define BB_ID_BBHRX_0                   BB_ID_BBHRX0_INGPIPE0_INGRESS0_HALF0_ID

/* DISPATCHER */
#define DISPATCHER_GROUP_PROCESSING RUNNER_TASK_GROUP_IDX_0
#define DISPATCHER_GROUP_DIRECT_FLOWS RUNNER_TASK_GROUP_IDX_5
#define DISPATCHER_GROUP_REPLICATION RUNNER_TASK_GROUP_IDX_7                               
#define BB_ID_BBHRX(idx)  (BB_ID_BBHRX_0  + (((idx) % 10) * 4) + ((idx)<10 ? 0 : BB_ID_SUB_SYS_BASE))

/* DISPATCHER */
#define BB_ID_DISPATCHER_0                  BB_ID_DISPREST0_INGPIPE0_INGRESS0_HALF0_ID
#define BB_ID_DISPATCHER(idx)               (BB_ID_DISPATCHER_0  + (((idx)%10) * 4) + ((idx)<10 ? 0 : BB_ID_SUB_SYS_BASE))
#define BB_ID_SBPM_ING_0                    BB_ID_ISBPM0_INGPIPE0_INGRESS0_HALF0_ID
#define BB_ID_SBPM_ING(idx)                 ((BB_ID_SBPM_ING_0  + (((idx) % 10) * 4)) + ((idx)<10 ? 0 : BB_ID_SUB_SYS_BASE))



/* SBPM */
#define NUM_OF_SBPM_INGRESS            (NUM_OF_INGRESS_LAYOUT_BLOCKS * NUM_OF_CHANNELS_PER_INGRESS)
#define NUM_OF_SBPM_EGRESS             1

#define SBPM_INIT_OFFSET_ING            0x7fe
#define SBPM_INIT_OFFSET_EGR            0x1ffe
#define SBPM_BASE_ADDRESS               0
#define ING_GL_TRSH (SBPM_INIT_OFFSET_ING - 100)
#define ING_GL_EXCL_HIGH_TRSH (SBPM_INIT_OFFSET_ING - 200)
#define ING_GL_EXCL_MED_TRSH (SBPM_INIT_OFFSET_ING - 400)
#define ING_GL_EXCL_LOW_TRSH (SBPM_INIT_OFFSET_ING - 600)
#define DS_CMN_GL_TRSH (SBPM_INIT_OFFSET_ING - 200)
#define DS_CMN_EXCL_HIGH_TRSH (SBPM_INIT_OFFSET_ING - 400)
#define DS_CMN_EXCL_MED_TRSH (SBPM_INIT_OFFSET_ING - 600)
#define DS_CMN_EXCL_LOW_TRSH (SBPM_INIT_OFFSET_ING - 1400)

typedef enum mapping_port_to_rx_numbers_e
{
    RX_PORT_LAN_0 = 0,
    RX_PORT_LAN_1,
    RX_PORT_LAN_2,
    RX_PORT_LAN_3,
    RX_PORT_LAN_4,
    RX_PORT_POND,
    RX_PORT_PONC0,
    RX_PORT_PONC1,
    RX_PORT_REP_0,
    RX_PORT_REP_1,
    RX_PORT_NUM,
} port_to_rx_numbers_;

#endif

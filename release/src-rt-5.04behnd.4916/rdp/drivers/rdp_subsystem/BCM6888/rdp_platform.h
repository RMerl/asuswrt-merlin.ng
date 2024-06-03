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

/* XLIF */
#define XLIF0_ACTIVE_CHANNELS 3
#define XLIF1_ACTIVE_CHANNELS 4
#define XLIF2_ACTIVE_CHANNELS 4


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
#define IS_PROCESSING_RUNNER_IMAGE(i)  ((rdp_core_to_image_map[i] == processing0_runner_image) || (rdp_core_to_image_map[i] == processing1_runner_image)|| (rdp_core_to_image_map[i] == processing2_runner_image) || (rdp_core_to_image_map[i] == processing3_runner_image) || (rdp_core_to_image_map[i] == processing4_runner_image) || (rdp_core_to_image_map[i] == processing5_runner_image) || (rdp_core_to_image_map[i] == processing6_runner_image) || (rdp_core_to_image_map[i] == processing7_runner_image) || (rdp_core_to_image_map[i] == processing8_runner_image) || (rdp_core_to_image_map[i] == processing9_runner_image))
#define IS_DS_TM_RUNNER_IMAGE(i)  ((rdp_core_to_image_map[i] == ds_tm_runner_image) || (rdp_core_to_image_map[i] == ds_tm_1_runner_image) || (rdp_core_to_image_map[i] == ds_tm_2_runner_image))
#define IS_US_TM_RUNNER_IMAGE(i)  (rdp_core_to_image_map[i] == us_tm_runner_image)

#define MAX_NUM_OF_LLID 8
#define MAX_NUM_OF_QUEUES_IN_SCHED 32
#define MAX_TX_QUEUES__NUM_OF             448

#define DMA_MAX_READ_ON_THE_FLY ( 16 )
#define SDMA_MAX_READ_ON_THE_FLY ( 16 )

/* NATC */
#define NATC_CACHE_ENTRIES_NUM   512
#define HASH_MODE hash_mode_crc32_low

/* DISP CONGESTION */
#define DIS_REOR_LINKED_LIST_BUFFER_NUM   1024
#define NUM_OF_PROCESSING_TASKS           92

#define DSPTCHR_CONG_PARAM_INGRESS_NORMAL 1023
#define DSPTCHR_CONG_PARAM_EGRESS_NORMAL  1023


#define DSPTCHR_CONG_PARAM_HYST           8

#define DSPTCHR_CONG_PARAM_GLOBAL         (DIS_REOR_LINKED_LIST_BUFFER_NUM - DSPTCHR_VIRTUAL_QUEUE_NUM - 2)
#define DSPTCHR_CONG_PARAM_EGRESS_GLOBAL  300
#define DSPTCHR_RESERVED_PRIORITY_BUFF_NUM 100

#define DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ             (DIS_REOR_LINKED_LIST_BUFFER_NUM - 1)
#define DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ  8
#define DSPTCHR_GUARANTEED_MAX_LIMIT_PER_EXCLUS_VIQ  16

/* SBPM */
#define SBPM_BASE_ADDRESS                   0
#define SBPM_MAX_BUFFER_NUMBER              0xFFF
#define SBPM_INIT_OFFSET                    SBPM_MAX_BUFFER_NUMBER
#define SBPM_UG0_BN_THRESHOLD               0x3FF
#define SBPM_UG0_EXCL_LOW_THRESHOLD         0x380
#define SBPM_UG0_EXCL_LOW_HIST              0xa
#define SBPM_UG0_EXCL_HIGH_THRESHOLD        0x3C0
#define SBPM_UG0_EXCL_HIGH_HIST             0xa
#define SBPM_UG0_HYSTERESIS                 0x00000014
#define SBPM_UG0_THRESHOLD                  0x00000400


#define SBPM_UG1_HYSTERESIS                 0x0000000a
#define SBPM_UG1_THRESHOLD                  0x00000c00
#define SBPM_UG1_BN_THRESHOLD               0xC00
#define SBPM_UG1_EXCL_LOW_THRESHOLD         0x980
#define SBPM_UG1_EXCL_HIGH_THRESHOLD        0xb80
#define SBPM_UG1_EXCL_LOW_HIST              0xa
#define SBPM_UG1_EXCL_HIGH_HIST             0xa

#define SBPM_GLOBAL_HIST                    0xA
#define SBPM_MAX_NUM_OF_BNS                 (SBPM_MAX_BUFFER_NUMBER + 1)

/* BBH_RX */
#define DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET 63
#define BBH_FREQUENCY (466770000) /* 466.77 MHz */

/* BBH_TX */
#define BBH_TX_DS_PD_FIFO_SIZE_0 7
#define BBH_TX_DS_PD_FIFO_SIZE_1 15
#define STSSIZE 3


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

/* UG max default thresholds */
#if defined(CONFIG_CPU_RX_FROM_XPM)
#define DS_FPM_UG_DEFAULT_PERCENTAGE        (29)
#define US_FPM_UG_DEFAULT_PERCENTAGE        (61)
#define WLAN_FPM_UG_DEFAULT_PERCENTAGE      (10)  /* For CPU only */
#define CPU_RX_PORT_RSV_TOKENS              (5*1024)
#define CPU_RX_PORT_HIGH_RSV_TOKENS         (1*1024)
#else
#define DS_FPM_UG_DEFAULT_PERCENTAGE        (34)
#define US_FPM_UG_DEFAULT_PERCENTAGE        (66)
#define WLAN_FPM_UG_DEFAULT_PERCENTAGE      (0)
#define CPU_RX_PORT_RSV_TOKENS              (0)
#define CPU_RX_PORT_HIGH_RSV_TOKENS         (0)
#endif

#define DS_FPM_UG_NO_XEPON_PERCENTAGE       (13)
#define US_FPM_UG_NO_XEPON_PERCENTAGE       (39)
#define WLAN_FPM_UG_NO_XEPON_PERCENTAGE     (48)

#define DS_FPM_UG_XEPON_PERCENTAGE          (13)
#define US_FPM_UG_XEPON_PERCENTAGE          (61)
#define WLAN_FPM_UG_XEPON_PERCENTAGE        (26)

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

/*GPON DEFS*/
/* size of each one of FIFOs 0-7 - value is size + 1*/
#define BBH_TX_GPON_PD_FIFO_SIZE_0_7 5
#define BBH_TX_GPON_PD_FIFO_SIZE_8_15 5
#define BBH_TX_GPON_PD_FIFO_SIZE_16_23 5
#define BBH_TX_GPON_PD_FIFO_SIZE_24_31 5
#define BBH_TX_GPON_PD_FIFO_SIZE_32_39 5

/*EPON DEFS*/
/* size of each one of FIFOs 8-15 */
/* size of each one of FIFOs 8-15 */
#define BBH_TX_EPON_PD_FIFO_SIZE_0_7 7
#define BBH_TX_EPON_PD_FIFO_SIZE_8_15 7
#define BBH_TX_EPON_PD_FIFO_SIZE_16_23 0
#define BBH_TX_EPON_PD_FIFO_SIZE_24_31 0
#define BBH_TX_EPON_PD_FIFO_SIZE_32_39 0

#define BBH_TX_GPON_PD_WAKE_UP_TH 3

#define SDMA_U_THRESH_IN_BBH_5G_VALUE           ( 0x03 )
#define SDMA_U_THRESH_OUT_BBH_5G_VALUE          ( 0x02 )

#define SDMA_U_THRESH_IN_BBH_2P5G_VALUE         ( 0x03 )
#define SDMA_U_THRESH_OUT_BBH_2P5G_VALUE        ( 0x02 )
#define SDMA_U_THRESH_IN_BBH_10G_VALUE          ( 0x06 )
#define SDMA_U_THRESH_OUT_BBH_10G_VALUE         ( 0x04 )
#define SDMA_RR_WEIGHT_TX_BBH_LAN_1_VALUE       ( 0x00 )
#define DMA_RR_WEIGHT_TX_BBH_LAN_1_VALUE        ( 0x00 )
#define SDMA_U_THRESH_IN_BBH_DSL_VALUE          ( 0x06 )
#define SDMA_U_THRESH_OUT_BBH_DSL_VALUE         ( 0x04 )
#define SDMA_STRICT_PRI_TX_BBH_LAN_1_VALUE      ( 0x04 )
#define DMA_STRICT_PRI_TX_BBH_LAN_1_VALUE       ( 0x08 )
#define SDMA_STRICT_PRI_RX_BBH_LAN_VALUE        ( 0x04 )
#define SDMA_STRICT_PRI_RX_BBH_5G_VALUE         ( 0x04 )
#define SDMA_STRICT_PRI_RX_BBH_10G_VALUE        ( 0x04 )
#define SDMA_STRICT_PRI_RX_BBH_2P5G_VALUE       ( 0x04 )
#define SDMA_STRICT_PRI_RX_BBH_DSL_VALUE        ( 0x04 )

#define SDMA_STRICT_PRI_TX_BBH_LAN_VALUE        ( 0x04 )
#define DMA_STRICT_PRI_TX_BBH_LAN_VALUE         ( 0x08 )
#define SDMA_STRICT_PRI_TX_BBH_COPY_VALUE       ( 0x04 )
#define DMA_STRICT_PRI_TX_BBH_COPY_VALUE        ( 0x08 )
#define SDMA_STRICT_PRI_TX_BBH_PON_VALUE        ( 0x04 )


#define SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE         ( 0x00 )
#define SDMA_RR_WEIGHT_RX_BBH_5G_VALUE          ( 0x00 )
#define SDMA_RR_WEIGHT_RX_BBH_2P5G_VALUE        ( 0x00 )
#define SDMA_RR_WEIGHT_RX_BBH_DSL_VALUE         ( 0x00 )
#define SDMA_RR_WEIGHT_RX_BBH_10G_VALUE         ( 0x00 )

#define SDMA_RR_WEIGHT_TX_BBH_LAN_VALUE         ( 0x00 )
#define DMA_RR_WEIGHT_TX_BBH_LAN_VALUE          ( 0x00 )
#define SDMA_RR_WEIGHT_TX_BBH_COPY_VALUE        ( 0x00 )
#define DMA_RR_WEIGHT_TX_BBH_COPY_VALUE         ( 0x00 )
#define SDMA_RR_WEIGHT_TX_BBH_DSL_VALUE         ( 0x00 )
#define DMA_RR_WEIGHT_TX_BBH_DSL_VALUE          ( 0x00 )

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

#define DMA_AVAIL_CHUNKS 48
#define DMA_CHUNKS_1GB 6
#define DMA_CHUNKS_2_5GB 7
#define DMA_CHUNKS_5GB 12
#define DMA_CHUNKS_10GB 15
#define DMA_MAX_SPEED_PER_DMA 225
#define DMA_MAX_SPEED 520

typedef enum bbh_id_e
{
    BBH_ID_0 = 0,
    BBH_ID_1,
    BBH_ID_2,
    BBH_ID_3,
    BBH_ID_4,
    BBH_ID_5,
    BBH_ID_6,
    BBH_ID_7,
    BBH_ID_8,
    BBH_ID_9,
    BBH_ID_10,
    BBH_ID_11,
    BBH_ID_12,
    BBH_ID_13,
    BBH_ID_14,
    BBH_ID_15,
    BBH_ID_PON,
    BBH_ID_NUM,
    BBH_ID_FIRST = BBH_ID_0,
    BBH_ID_LAST = BBH_ID_PON,
    BBH_ID_NULL = BBH_ID_NUM,
    BBH_ID_NUM_LAN = BBH_ID_15,
    BBH_ID_AE = BBH_ID_6,
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

typedef enum tm_identifier_e
{
    TM_START = 0,
    TM_PON_DSL = 0,
    TM_ETH_START = 1,
    TM_ETH_PORTS_0 = TM_ETH_START,
    TM_ETH_PORTS_1 = 2,
    TM_ETH_PORTS_2 = 3,
    TM_ETH_END = TM_ETH_PORTS_2,
    TM_ETH_SQ = 4,
    TM_MAX = 4,
    TM_ERROR = 5,
    TM_NUM_OF_IDENTITY = 5
} tm_identifier_e;

static inline int tm_get_core_for_tm(tm_identifier_e tm_identity)
{
    switch (tm_identity)
    {
    case TM_PON_DSL:
        return drv_qm_get_tm_core_per_image(us_tm_runner_image);

    case TM_ETH_PORTS_0:
        return drv_qm_get_tm_core_per_image(ds_tm_runner_image);
    
    case TM_ETH_PORTS_1:
        return drv_qm_get_tm_core_per_image(ds_tm_1_runner_image);

    case TM_ETH_PORTS_2:
        return drv_qm_get_tm_core_per_image(ds_tm_2_runner_image);

    case TM_ETH_SQ:
        return drv_qm_get_tm_core_per_image(service_queues_runner_image);
   
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

#define NUM_OF_CORES_IN_QUAD                        14
#define DRV_PARSER_MASKED_DA_FILTER_NUM             2
#define DRV_PARSER_DA_FILTER_NUM                    9
#define NUM_OF_RNR_WITH_PROFILING                   14


#define SDMA_U_THRESH_IN_BBH_LAN_VALUE          ( 0x02 )
#define SDMA_U_THRESH_OUT_BBH_LAN_VALUE         ( 0x01 )

#define DMA_U_THRESH_IN_BBH_LAN_VALUE     ( 0x2 )
#define DMA_U_THRESH_IN_BBH_PON_VALUE     ( 0x3 )

#define DMA_U_THRESH_OUT_BBH_LAN_VALUE    ( 0x1 )
#define DMA_U_THRESH_OUT_BBH_PON_VALUE    ( 0x2 )

#define DMA_STRICT_PRI_RX_BBH_LAN_VALUE   ( 0x4 )
#define DMA_STRICT_PRI_RX_BBH_PON_VALUE   ( 0x4 )

#define DMA_STRICT_PRI_TX_BBH_PON_VALUE   ( 0x8 )
#define DMA_RR_WEIGHT_RX_BBH_PON_VALUE    ( 0x0 )
#define DMA_RR_WEIGHT_TX_BBH_PON_VALUE    ( 0x0 )


#define DISP_REOR_VIQ_BBH_PON_RX_EXCL DISP_REOR_VIQ_BBH_RX10_EXCL
#define DISP_REOR_VIQ_BBH_RX_MAX_NUM DISP_REOR_VIQ_BBH_RX10_NORMAL

typedef enum bac_if_id_e
{
    BACIF_0_TCAM_ID = 0,
    BACIF_0_HASH_ID,
    BACIF_0_CNPL_ID,
    BACIF_0_NATC_ID,
    BACIF_1_TCAM_ID,
    BACIF_1_HASH_ID,
    BACIF_1_CNPL_ID,
    BACIF_1_NATC_ID,
    BACIF_ID_NUM,
    BACIF_ID_LAST = BACIF_1_NATC_ID,
} bac_if_id_e;

#define SUPPORTED_NUM_OF_FPM_TOKENS  CONST_INT_256K

#ifdef RDP_SIM
#define SBPM_SP_RNR_LOW_INIT_VAL                       0xffff
#define SBPM_SP_RNR_HIGH_INIT_VAL                      0
#define SBPM_UG_MAP_LOW_INIT_VAL                       0x1550000
#define SBPM_UG_MAP_HIGH_INIT_VAL                      0x80C05400
#endif

#define CSO_BBH_TX_OFFLOAD_CHANNEL (7)
#define SERVICE_QUEUES_THREAD_NUMBER IMAGE_4_IMAGE_4_SERVICE_QUEUES_THREAD_NUMBER

#define RC_MAX_PMC_CORES 7

#endif

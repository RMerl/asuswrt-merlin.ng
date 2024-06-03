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

#define RNR_FREQ_IN_MHZ 1250
#define UBUS_SLV_FREQ_IN_MHZ 500
#define MAX_TX_QUEUES__NUM_OF             128

/* IS_WAN_TX_PORT and IS_WAN_RX_PORT identify TX WAN ports for DSL only (NOT Ethernet) */
#define IS_WAN_TX_PORT(bbh_id) (bbh_id == BBH_TX_ID_DSL)
#define IS_WAN_RX_PORT(bbh_id) (bbh_id == BBH_ID_DSL)
/* IS_DS_WAN_PORT and IS_US_WAN_PORT identify ALL (DS/US respectively) WAN ports */
/* TBD63146. Identify Ethernet WAN */
#define IS_DS_WAN_PORT(bbh_id)  (bbh_id == BBH_ID_DSL)
#define IS_US_WAN_PORT(bbh_id) (bbh_id == BBH_TX_ID_DSL)
#define IS_PROCESSING_RUNNER_IMAGE(i)  (  (rdp_core_to_image_map[i] == processing1_runner_image) \
                                       || (rdp_core_to_image_map[i] == processing2_runner_image) \
                                       || (rdp_core_to_image_map[i] == processing3_runner_image) \
                                       || (rdp_core_to_image_map[i] == processing4_runner_image) )
#define IS_DS_TM_RUNNER_IMAGE(i)  (rdp_core_to_image_map[i] == ds_tm_runner_image)
#define IS_US_TM_RUNNER_IMAGE(i)  (rdp_core_to_image_map[i] == us_tm_runner_image)

/* CNPL */
/* update every 3730ns (N=255 ; 14628ns * 255 = ns ) */
#define CNPL_PERIODIC_UPDATE_US                 3730

/* NATC */
#define NATC_CACHE_ENTRIES_NUM   1024

/* DISP CONGESTION */
#define DIS_REOR_LINKED_LIST_BUFFER_NUM   512
#define NUM_OF_PROCESSING_TASKS           32

#define DSPTCHR_CONG_PARAM_INGRESS_NORMAL 511
#define DSPTCHR_CONG_PARAM_EGRESS_NORMAL  511

#define DSPTCHR_CONG_PARAM_HYST           8

#define DSPTCHR_CONG_PARAM_GLOBAL         (DIS_REOR_LINKED_LIST_BUFFER_NUM - DSPTCHR_VIRTUAL_QUEUE_NUM - 2)
#define DSPTCHR_CONG_PARAM_EGRESS_GLOBAL  150
#define DSPTCHR_RESERVED_PRIORITY_BUFF_NUM 50

#define DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ             225
#define DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ  8
#define DSPTCHR_GUARANTEED_MAX_LIMIT_PER_EXCLUS_VIQ  16

/* SBPM */
#define SBPM_BASE_ADDRESS                   0
#define SBPM_INIT_OFFSET                    0x5FF
#define SBPM_UG0_BN_THRESHOLD               0x2F0
#define SBPM_UG0_EXCL_LOW_THRESHOLD         0x294
#define SBPM_UG0_EXCL_LOW_HIST              0xF
#define SBPM_UG0_EXCL_HIGH_THRESHOLD        0x2C2
#define SBPM_UG0_EXCL_HIGH_HIST             0x7
#define SBPM_UG1_BN_THRESHOLD               0x300
#define SBPM_UG1_EXCL_HIGH_THRESHOLD        0x294
#define SBPM_UG1_EXCL_HIGH_HIST             0x7
#define SBPM_UG1_EXCL_LOW_THRESHOLD         0x114
#define SBPM_UG1_EXCL_LOW_HIST              0xF
#define SBPM_MAX_NUM_OF_BNS                 1536

/* BBH_RX */
#define DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET 127
#define BBH_FREQUENCY (UBUS_SLV_FREQ_IN_MHZ*1000000)

/* BBH_TX */
#define BBH_TX_DS_PD_FIFO_SIZE_0 7
#define BBH_TX_DS_PD_FIFO_SIZE_1 7

/* HASH */
#define HASH_NUM_OF_ENGINES           4
#define HASH_NUM_OF_ENGINES_LOG2      2

/* VLAN */
#define RDPA_MAX_VLANS                  256     /**< Max number of VIDs */

/* TCAM table size (single entries) */
#define RDP_TCAM_TABLE_SIZE     256

/* Size of TCAM entry in words.  256 bits = 8 words */
#define RDP_TCAM_ENTRY_SIZE_WORDS    8    

/* Max number of reported queues supported by QM */
#define QM_NUM_REPORTED_QUEUES  32 /* TBD63146. Need for BCM63146 */

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
#define WLAN_GROUP_MAX_TOKENS_PERCENTAGE    (100)

#define WLAN_PORT_RSV_TOKENS                (1*1024)
#define WLAN_PORT_HIGH_PRIO_RSV_TOKENS      (512)

/*DSL DEFS*/
#define BBH_TX_DSL_PD_FIFO_SIZE_0_7 15
#define BBH_TX_DSL_PD_FIFO_SIZE_8_15 15
#define BBH_TX_DSL_PD_FIFO_SIZE_16_23 0
#define BBH_TX_DSL_PD_FIFO_SIZE_24_31 0
#define BBH_TX_DSL_PD_FIFO_SIZE_32_39 0

#define DMA_MAX_READ_ON_THE_FLY ( 16 )
#define SDMA_MAX_READ_ON_THE_FLY ( 8 )

#define SDMA_U_THRESH_IN_BBH_LAN_VALUE          ( 0x02 )
#define SDMA_U_THRESH_OUT_BBH_LAN_VALUE         ( 0x01 )

#define SDMA_U_THRESH_IN_BBH_5G_VALUE           ( 0x03 )
#define SDMA_U_THRESH_OUT_BBH_5G_VALUE          ( 0x02 )

#define SDMA_U_THRESH_IN_BBH_2P5G_VALUE         ( 0x03 )
#define SDMA_U_THRESH_OUT_BBH_2P5G_VALUE        ( 0x02 )

#define SDMA_U_THRESH_IN_BBH_DSL_VALUE          ( 0x06 )
#define SDMA_U_THRESH_OUT_BBH_DSL_VALUE         ( 0x04 )

#define SDMA_STRICT_PRI_RX_BBH_LAN_VALUE        ( 0x04 )
#define SDMA_STRICT_PRI_RX_BBH_5G_VALUE         ( 0x04 )
#define SDMA_STRICT_PRI_RX_BBH_2P5G_VALUE       ( 0x04 )
#define SDMA_STRICT_PRI_RX_BBH_DSL_VALUE        ( 0x04 )

#define SDMA_STRICT_PRI_TX_BBH_LAN_VALUE        ( 0x04 )
#define DMA_STRICT_PRI_TX_BBH_LAN_VALUE         ( 0x08 )
#define SDMA_STRICT_PRI_TX_BBH_COPY_VALUE       ( 0x04 )
#define DMA_STRICT_PRI_TX_BBH_COPY_VALUE        ( 0x08 )
#define SDMA_STRICT_PRI_TX_BBH_DSL_VALUE        ( 0x04 )
#define DMA_STRICT_PRI_TX_BBH_DSL_VALUE         ( 0x08 )

#define SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE         ( 0x00 )
#define SDMA_RR_WEIGHT_RX_BBH_5G_VALUE          ( 0x00 )
#define SDMA_RR_WEIGHT_RX_BBH_2P5G_VALUE        ( 0x00 )
#define SDMA_RR_WEIGHT_RX_BBH_DSL_VALUE         ( 0x00 )

#define SDMA_RR_WEIGHT_TX_BBH_LAN_VALUE         ( 0x00 )
#define DMA_RR_WEIGHT_TX_BBH_LAN_VALUE          ( 0x00 )
#define SDMA_RR_WEIGHT_TX_BBH_COPY_VALUE        ( 0x00 )
#define DMA_RR_WEIGHT_TX_BBH_COPY_VALUE         ( 0x00 )
#define SDMA_RR_WEIGHT_TX_BBH_DSL_VALUE         ( 0x00 )
#define DMA_RR_WEIGHT_TX_BBH_DSL_VALUE          ( 0x00 )

typedef enum bbh_id_e
{
    /* RX */
    /* RX BBH_ID needs to be the same as DISP_REOR_VIQ defined in
     * project_data_structures.xml or else bbh_rx configuration will break */
    BBH_ID_0 = 0,
    BBH_ID_1,
    BBH_ID_2,
    BBH_ID_3,
    BBH_ID_4,
    BBH_ID_5,
    BBH_ID_6_2P5G,
    BBH_ID_7_5G,
    BBH_ID_DSL,
    BBH_ID_NUM,
    BBH_ID_FIRST = BBH_ID_0,
    BBH_ID_NUM_LAN = BBH_ID_7_5G,
    BBH_ID_LAST = BBH_ID_DSL,
    BBH_ID_NULL = BBH_ID_NUM,
    /* TX */
    BBH_TX_ID_LAN = 0,
    BBH_ID_QM_COPY,
    BBH_TX_ID_DSL,
    BBH_TX_ID_NUM,
    BBH_TX_ID_FIRST = BBH_TX_ID_LAN,
    BBH_TX_ID_LAST = BBH_TX_ID_DSL,
    BBH_TX_ID_NULL = BBH_ID_NULL,
} bbh_id_e;

typedef enum dma_id_e
{
    DMA0_ID = 0,
    DMA1_ID,
    DMA_NUM,
    DMA_ID_FIRST = DMA0_ID,
} dma_id_e;

typedef enum dma_target_e
{
    DMA_TARGET_DMA = 0,
    DMA_TARGET_SDMA,
    DMA_TARGET_NUM,
    DMA_TARGET_FIRST = DMA_TARGET_DMA,
} dma_target_e;

typedef enum rnr_quad_id_e
{
    RNR_QUAD0_ID = 0,
    RNR_QUAD_ID_LAST = RNR_QUAD0_ID,
    NUM_OF_RNR_QUADS = RNR_QUAD_ID_LAST + 1,
} rnr_quad_id_e;

#define NUM_OF_CORES_IN_QUAD                        5
#define DRV_PARSER_MASKED_DA_FILTER_NUM             2
#define DRV_PARSER_DA_FILTER_NUM                    9
#define NUM_OF_RNR_WITH_PROFILING                   5

typedef enum unimac_id_e
{
    UNIMAC0_ID = 0,
    UNIMAC1_ID,
    UNIMAC2_ID,
    UNIMAC3_ID,
    UNIMAC4_ID,
    UNIMAC5_ID,
    UNIMAC6_ID,
    UNIMAC7_ID,
    UNIMAC_ID_NUM,
    UNIMAC_ID_LAST = UNIMAC7_ID,
} unimac_id_e;

typedef enum xumac_id_e
{
    XUMAC0_ID = 6,
    XUMAC1_ID,
    XUMAC_ID_LAST = XUMAC1_ID,
} xumac_id_e;

typedef enum bac_if_id_e
{
    BACIF_TCAM_ID = 0,
    BACIF_HASH_ID,
    BACIF_CNPL_ID,
    BACIF_NATC_ID,
    BACIF_ID_NUM,
    BACIF_ID_LAST = BACIF_NATC_ID,
} bac_if_id_e;

/* TBD63146 - need to find RU/HAL generator syntax to create dsptchr_glbl_cngs_params */
#define dsptchr_glbl_cngs_params dsptchr_cngs_params

#define SUPPORTED_NUM_OF_FPM_TOKENS  CONST_INT_128K

#ifdef __cplusplus
}
#endif

#ifdef RDP_SIM
#define SBPM_SP_RNR_LOW_INIT_VAL                       0x1f
#define SBPM_SP_RNR_HIGH_INIT_VAL                      0
#define SBPM_UG_MAP_LOW_INIT_VAL                       0x60000000
#define SBPM_UG_MAP_HIGH_INIT_VAL                      0x80c00015
#endif

#endif


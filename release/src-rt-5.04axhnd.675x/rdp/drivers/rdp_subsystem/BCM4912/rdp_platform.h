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
#define UBUS_SLV_FREQ_IN_MHZ 750

/* IS_WAN_TX_PORT and IS_WAN_RX_PORT identify TX WAN ports for DSL only (NOT Ethernet) */
#define IS_WAN_TX_PORT(bbh_id) (0)
#define IS_WAN_RX_PORT(bbh_id) (0)
/* IS_DS_WAN_PORT and IS_US_WAN_PORT identify ALL (DS/US respectively) WAN ports */
/* TBD4912. Identify Ethernet WAN */
#define IS_DS_WAN_PORT(bbh_id)  (0)
#define IS_US_WAN_PORT(bbh_id) (0)
#define IS_PROCESSING_RUNNER_IMAGE(i)  (  (rdp_core_to_image_map[i] == processing0_runner_image) \
                                       || (rdp_core_to_image_map[i] == processing1_runner_image) \
                                       || (rdp_core_to_image_map[i] == processing2_runner_image) \
                                       || (rdp_core_to_image_map[i] == processing3_runner_image) \
                                       || (rdp_core_to_image_map[i] == processing4_runner_image) \
                                       || (rdp_core_to_image_map[i] == processing5_runner_image) \
                                       || (rdp_core_to_image_map[i] == processing6_runner_image) )
#define IS_DS_TM_RUNNER_IMAGE(i)  (rdp_core_to_image_map[i] == ds_tm_runner_image)
#define IS_US_TM_RUNNER_IMAGE(i)  (rdp_core_to_image_map[i] == us_tm_runner_image)

/* CNPL */
/* update every 3730ns (N=255 ; 14628ns * 255 = ns ) */
#define CNPL_PERIODIC_UPDATE_US                 3730

/* NATC */
#define NATC_CACHE_ENTRIES_NUM   1024

/* DISP CONGESTION */
#define DIS_REOR_LINKED_LIST_BUFFER_NUM   1024
#define NUM_OF_PROCESSING_TASKS           48

#define DSPTCHR_CONG_PARAM_INGRESS_NORMAL 1023
#define DSPTCHR_CONG_PARAM_EGRESS_NORMAL  1023

#define DSPTCHR_CONG_PARAM_HYST           16

#define DSPTCHR_CONG_PARAM_GLOBAL         (DIS_REOR_LINKED_LIST_BUFFER_NUM - DSPTCHR_VIRTUAL_QUEUE_NUM - 2)
#define DSPTCHR_CONG_PARAM_EGRESS_GLOBAL  300
#define DSPTCHR_RESERVED_PRIORITY_BUFF_NUM 100

#define DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ             (DIS_REOR_LINKED_LIST_BUFFER_NUM - 1)
#define DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ  8
#define DSPTCHR_GUARANTEED_MAX_LIMIT_PER_EXCLUS_VIQ  16

/* SBPM */
/* 4K buffers, recommended usage is 3K for egress (UG1) and 1K for ingress (UG0) */
#define SBPM_BASE_ADDRESS                   0
#define SBPM_MAX_BUFFER_NUMBER              0xFFF
#define SBPM_INIT_OFFSET                    SBPM_MAX_BUFFER_NUMBER
#define SBPM_UG0_BN_THRESHOLD               0x400
#define SBPM_UG0_EXCL_HIGH_THRESHOLD        0x3C0
#define SBPM_UG0_EXCL_LOW_THRESHOLD         0x300
#define SBPM_UG0_EXCL_LOW_HIST              0x14
#define SBPM_UG1_BN_THRESHOLD               0xC00
#define SBPM_UG1_EXCL_HIGH_THRESHOLD        0xB80
#define SBPM_UG1_EXCL_LOW_THRESHOLD         0x980
#define SBPM_UG1_EXCL_LOW_HIST              0x14
#define SBPM_MAX_NUM_OF_BNS                 (SBPM_MAX_BUFFER_NUMBER + 1)
#define TUNNEL_BN_FIRST                     (SBPM_MAX_BUFFER_NUMBER - 1)

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

/* TCAM table size (entries) */
#define RDP_TCAM_TABLE_SIZE     256
/* Number of TCAM engines */
#define RDP_TCAM_NUM_ENGINES    2

/* Max number of queues supported by QM */
#define QM_NUM_QUEUES           128
/* Max number of reported queues supported by QM */
#define QM_NUM_REPORTED_QUEUES  32 /* TBD4912. Need for BCM4912 */

/* UG max default thresholds */
#define DS_FPM_UG_DEFAULT_PERCENTAGE        (34)
#define US_FPM_UG_DEFAULT_PERCENTAGE        (66)
#define WLAN_FPM_UG_DEFAULT_PERCENTAGE      (0)

#define DS_FPM_UG_NO_XEPON_PERCENTAGE       (13)
#define US_FPM_UG_NO_XEPON_PERCENTAGE       (39)
#define WLAN_FPM_UG_NO_XEPON_PERCENTAGE     (48)

#define DS_FPM_UG_XEPON_PERCENTAGE          (13)
#define US_FPM_UG_XEPON_PERCENTAGE          (61)
#define WLAN_FPM_UG_XEPON_PERCENTAGE        (26)

/* BUF_MNG thresholds */
#define WLAN_MAX_TOKENS                     (64*1024)
#define WLAN_RADIO_RSV_TOKENS               (1*1024)
#define WLAN_RADIO_HIGH_PRIO_RSV_TOKENS     (512)
#define WLAN_RSV_TOKENS                     (20*1024)

#define DMA_MAX_READ_ON_THE_FLY ( 16 )
#define SDMA_MAX_READ_ON_THE_FLY ( 8 )

#define SDMA_U_THRESH_IN_BBH_LAN_VALUE          ( 0x02 )
#define SDMA_U_THRESH_OUT_BBH_LAN_VALUE         ( 0x01 )

#define SDMA_U_THRESH_IN_BBH_5G_VALUE           ( 0x03 )
#define SDMA_U_THRESH_OUT_BBH_5G_VALUE          ( 0x02 )

#define SDMA_U_THRESH_IN_BBH_10G_VALUE          ( 0x06 )
#define SDMA_U_THRESH_OUT_BBH_10G_VALUE         ( 0x04 )

#define SDMA_STRICT_PRI_RX_BBH_LAN_VALUE        ( 0x04 )
#define SDMA_STRICT_PRI_RX_BBH_5G_VALUE         ( 0x04 )
#define SDMA_STRICT_PRI_RX_BBH_10G_VALUE        ( 0x04 )

#define SDMA_STRICT_PRI_TX_BBH_LAN_VALUE        ( 0x04 )
#define DMA_STRICT_PRI_TX_BBH_LAN_VALUE         ( 0x08 )
#define SDMA_STRICT_PRI_TX_BBH_LAN_1_VALUE      ( 0x04 )
#define DMA_STRICT_PRI_TX_BBH_LAN_1_VALUE       ( 0x08 )
#define SDMA_STRICT_PRI_TX_BBH_COPY_VALUE       ( 0x04 )
#define DMA_STRICT_PRI_TX_BBH_COPY_VALUE        ( 0x08 )

#define SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE         ( 0x00 )
#define SDMA_RR_WEIGHT_RX_BBH_5G_VALUE          ( 0x00 )
#define SDMA_RR_WEIGHT_RX_BBH_10G_VALUE         ( 0x00 )

#define SDMA_RR_WEIGHT_TX_BBH_LAN_VALUE         ( 0x00 )
#define DMA_RR_WEIGHT_TX_BBH_LAN_VALUE          ( 0x00 )
#define SDMA_RR_WEIGHT_TX_BBH_LAN_1_VALUE       ( 0x00 )
#define DMA_RR_WEIGHT_TX_BBH_LAN_1_VALUE        ( 0x00 )
#define SDMA_RR_WEIGHT_TX_BBH_COPY_VALUE        ( 0x00 )
#define DMA_RR_WEIGHT_TX_BBH_COPY_VALUE         ( 0x00 )

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
    BBH_ID_5_10G,
    BBH_ID_6_10G,
    BBH_ID_7_10G_5G_2P5G,
    BBH_ID_8_5G_2P5G,
    BBH_ID_9_2P5G,
    BBH_ID_10_2P5G,
    BBH_ID_NUM,
    BBH_ID_FIRST = BBH_ID_0,
    BBH_ID_NUM_LAN = BBH_ID_7_10G_5G_2P5G,
    BBH_ID_LAST = BBH_ID_7_10G_5G_2P5G,
    BBH_ID_NULL = BBH_ID_NUM,

    /* TX */
    BBH_TX_ID_LAN = 0,
    /* BBH Queue 0-3: QGPHY
     * BBH Queue 4: RGMII
     * BBH Queue 5: XLMAC 0, port 0, 10Gbps */
    BBH_ID_QM_COPY,
    BBH_TX_ID_LAN_1,
    /* BBH Queue 0: XLMAC 0, port 2, 5Gbps
     * BBH Queue 1: XLMAC 1, port 0, 10Gbps */
    BBH_TX_ID_NUM,
    BBH_TX_ID_FIRST = BBH_TX_ID_LAN,
    BBH_TX_ID_LAST = BBH_TX_ID_LAN_1,
    BBH_TX_ID_NULL = BBH_ID_NULL,
} bbh_id_e;

typedef enum xlif_id_e
{
    XLIF_ID_CHANNEL_FIRST = 0,
    XLIF_ID_CHANNEL_0 = XLIF_ID_CHANNEL_FIRST,
    XLIF_ID_CHANNEL_1,
    XLIF_ID_CHANNEL_2,
    XLIF_ID_CHANNEL_3,
    XLIF_ID_CHANNEL_NUM
} xlif_id_e;

typedef enum dma_id_e
{
    DMA0_ID = 0,
    DMA1_ID,
    DMA_COPY_ID,
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

#define NUM_OF_CORES_IN_QUAD                        7
#define DRV_PARSER_MASKED_DA_FILTER_NUM             2
#define DRV_PARSER_DA_FILTER_NUM                    9
#define NUM_OF_RNR_WITH_PROFILING                   7

typedef enum unimac_id_e
{
    UNIMAC0_ID = 0,
    UNIMAC1_ID,
    UNIMAC2_ID,
    UNIMAC3_ID,
    UNIMAC4_ID,
    UNIMAC_ID_NUM,
    UNIMAC_ID_LAST = UNIMAC4_ID,
} unimac_id_e;

typedef enum bac_if_id_e
{
    BACIF_TCAM_ID = 0,
    BACIF_HASH_ID,
    BACIF_CNPL_ID,
    BACIF_NATC_ID,
    BACIF_ID_NUM,
    BACIF_ID_LAST = BACIF_NATC_ID,
} bac_if_id_e;

/* TBD4912 - need to find RU/HAL generator syntax to create dsptchr_glbl_cngs_params */
#define dsptchr_glbl_cngs_params dsptchr_cngs_params

#define SUPPORTED_NUM_OF_FPM_TOKENS  CONST_INT_256K

#ifdef __cplusplus
}
#endif

#endif


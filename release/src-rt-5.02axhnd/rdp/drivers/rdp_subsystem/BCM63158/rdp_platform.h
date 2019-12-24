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

#define RNR_FREQ_IN_MHZ 1400
#define UBUS_SLV_FREQ_IN_MHZ 560

/* IS_WAN_TX_PORT and IS_WAN_RX_PORT identify TX WAN ports for DSL and PON only (NOT Ethernet) */
#define IS_WAN_TX_PORT(bbh_id) ((bbh_id == BBH_TX_ID_PON) || (bbh_id == BBH_TX_ID_DSL))
#define IS_WAN_RX_PORT(bbh_id) ((bbh_id == BBH_ID_PON) || (bbh_id == BBH_ID_DSL))
/* IS_DS_WAN_PORT and IS_US_WAN_PORT identify ALL (DS/US respectively) WAN ports */
#define IS_DS_WAN_PORT(bbh_id)  (bbh_id >= BBH_ID_PON)
#define IS_US_WAN_PORT(bbh_id) (bbh_id >= BBH_TX_ID_PON)
#define IS_SDMA(dma_id)  (dma_id > DMA0_ID)
#define IS_DMA(dma_id)   (dma_id <= DMA0_ID)
#define IS_PROCESSING_RUNNER_IMAGE(i)  (  (rdp_core_to_image_map[i] == processing0_runner_image) \
                                       || (rdp_core_to_image_map[i] == processing1_runner_image) \
                                       || (rdp_core_to_image_map[i] == processing2_runner_image) \
                                       || (rdp_core_to_image_map[i] == processing3_runner_image) )
#define IS_XFI_PORT(bbh_id) (bbh_id == BBH_ID_AE10)
#define IS_DS_TM_RUNNER_IMAGE(i)  (rdp_core_to_image_map[i] == ds_tm_runner_image)
#define IS_US_TM_RUNNER_IMAGE(i)  (rdp_core_to_image_map[i] == us_tm_runner_image)

/* CNPL */
/* update every 3730ns (N=255 ; 14628ns * 255 = ns ) */
#define CNPL_PERIODIC_UPDATE_US                 3730

/* NATC */
#define NATC_CACHE_ENTRIES_NUM   1024

/* DISP CONGESTION */
#define DIS_REOR_CONGESTION_THRESHOLD    200
#define DIS_REOR_LINKED_LIST_BUFFER_NUM  512
#define NUM_OF_PROCESSING_TASKS          0 /* Ingress Qos not activated */

/* SBPM */
#define SBPM_BASE_ADDRESS                   0
#define SBPM_INIT_OFFSET                    0x00007ff
#define SBPM_UG0_BN_THRESHOLD               0x1FF
#define SBPM_UG0_EXCL_LOW_THRESHOLD         0x190
#define SBPM_UG0_EXCL_LOW_HIST              0x14
#define SBPM_UG1_BN_THRESHOLD               0x600
#define SBPM_UG1_EXCL_LOW_THRESHOLD         0x400
#define SBPM_UG1_EXCL_LOW_HIST              0x14

/* BBH_RX */
#define DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET 127
#define BBH_FREQUENCY (560*1000000) /* 560 MHz */

/* BBH_TX */
#define BBH_TX_DS_PD_FIFO_SIZE_0 7
#define BBH_TX_DS_PD_FIFO_SIZE_1 7

/* HASH */
#define HASH_NUM_OF_ENGINES           2
#define HASH_NUM_OF_ENGINES_LOG2      1
#define HASH_NUM_OF_EFFECTIVE_ENGINES 2
#define HASH_NUM_OF_ENTRIES_IN_RAM    2048

/* VLAN */
#define RDPA_MAX_VLANS                  256     /**< Max number of VIDs */

/* IPTV */
#define HASH_TABLE_IPTV_SIZE_PER_ENGINE  hash_max_256_entries_per_engine

/* TCAM table size (entries) */
#define RDP_TCAM_TABLE_SIZE     256
/* Number of TCAM engines */
#define RDP_TCAM_NUM_ENGINES    2

/* Max number of queues supported by QM */
#define QM_NUM_QUEUES           160
/* Max number of EPON queues supported by QM */
#define QM_NUM_EPON_QUEUES      96
/* Max number of reported queues supported by QM */
#define QM_NUM_REPORTED_QUEUES  128

/* UG max default thresholds */
#define DS_FPM_UG_DEFAULT_PERCENTAGE        (34)
#define US_FPM_UG_DEFAULT_PERCENTAGE        (66)
#define WLAN_FPM_UG_DEFAULT_PERCENTAGE      (0)


/*DSL DEFS*/
#define BBH_TX_DSL_PD_FIFO_SIZE_0_7 15
#define BBH_TX_DSL_PD_FIFO_SIZE_8_15 15
#define BBH_TX_DSL_PD_FIFO_SIZE_16_23 0
#define BBH_TX_DSL_PD_FIFO_SIZE_24_31 0
#define BBH_TX_DSL_PD_FIFO_SIZE_32_39 0

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

#define MAX_NUM_OF_LLID 32

#define DMA_U_THRESH_IN_BBH_LAN_VALUE           ( 0x00 )
#define DMA_U_THRESH_OUT_BBH_LAN_VALUE          ( 0x00 )
#define SDMA_U_THRESH_IN_BBH_LAN_VALUE          ( 0x06 )
#define SDMA_U_THRESH_OUT_BBH_LAN_VALUE         ( 0x05 )

#define DMA_U_THRESH_IN_BBH_PON_VALUE           ( 0x00 )
#define DMA_U_THRESH_OUT_BBH_PON_VALUE          ( 0x00 )
#define SDMA_U_THRESH_IN_BBH_PON_VALUE          ( 0x12 )
#define SDMA_U_THRESH_OUT_BBH_PON_VALUE         ( 0x0d )

#define DMA_U_THRESH_IN_BBH_XPORT10G_VALUE      ( 0x00 )
#define DMA_U_THRESH_OUT_BBH_XPORT10G_VALUE     ( 0x00 )
#define SDMA_U_THRESH_IN_BBH_XPORT10G_VALUE     ( 0x12 )
#define SDMA_U_THRESH_OUT_BBH_XPORT10G_VALUE    ( 0x0e )

#define DMA_U_THRESH_IN_BBH_XPORT2P5G_VALUE     ( 0x00 )
#define DMA_U_THRESH_OUT_BBH_XPORT2P5G_VALUE    ( 0x00 )
#define SDMA_U_THRESH_IN_BBH_XPORT2P5G_VALUE    ( 0x0a )
#define SDMA_U_THRESH_OUT_BBH_XPORT2P5G_VALUE   ( 0x09 )

#define DMA_U_THRESH_IN_BBH_DSL_VALUE           ( 0x00 )
#define DMA_U_THRESH_OUT_BBH_DSL_VALUE          ( 0x00 )
#define SDMA_U_THRESH_IN_BBH_DSL_VALUE          ( 0x0a )
#define SDMA_U_THRESH_OUT_BBH_DSL_VALUE         ( 0x09 )

#define DMA_STRICT_PRI_RX_BBH_LAN_VALUE         ( 0x04 )
#define DMA_STRICT_PRI_TX_BBH_LAN_VALUE         ( 0x08 )
#define SDMA_STRICT_PRI_RX_BBH_LAN_VALUE        ( 0x04 )
#define SDMA_STRICT_PRI_TX_BBH_LAN_VALUE        ( 0x08 )

#define DMA_STRICT_PRI_RX_BBH_PON_VALUE         ( 0x04 )
#define DMA_STRICT_PRI_TX_BBH_PON_VALUE         ( 0x08 )
#define SDMA_STRICT_PRI_RX_BBH_PON_VALUE        ( 0x04 )
#define SDMA_STRICT_PRI_TX_BBH_PON_VALUE        ( 0x08 )

#define DMA_STRICT_PRI_RX_BBH_XPORT10G_VALUE    ( 0x04 )
#define DMA_STRICT_PRI_TX_BBH_XPORT10G_VALUE    ( 0x08 )
#define SDMA_STRICT_PRI_RX_BBH_XPORT10G_VALUE   ( 0x04 )
#define SDMA_STRICT_PRI_TX_BBH_XPORT10G_VALUE   ( 0x08 )

#define DMA_STRICT_PRI_RX_BBH_XPORT2P5G_VALUE   ( 0x04 )
#define DMA_STRICT_PRI_TX_BBH_XPORT2P5G_VALUE   ( 0x08 )
#define SDMA_STRICT_PRI_RX_BBH_XPORT2P5G_VALUE  ( 0x04 )
#define SDMA_STRICT_PRI_TX_BBH_XPORT2P5G_VALUE  ( 0x08 )

#define DMA_STRICT_PRI_RX_BBH_DSL_VALUE         ( 0x04 )
#define DMA_STRICT_PRI_TX_BBH_DSL_VALUE         ( 0x08 )
#define SDMA_STRICT_PRI_RX_BBH_DSL_VALUE        ( 0x04 )
#define SDMA_STRICT_PRI_TX_BBH_DSL_VALUE        ( 0x08 )

#define DMA_RR_WEIGHT_RX_BBH_LAN_VALUE          ( 0x00 )
#define DMA_RR_WEIGHT_TX_BBH_LAN_VALUE          ( 0x00 )
#define SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE         ( 0x00 )
#define SDMA_RR_WEIGHT_TX_BBH_LAN_VALUE         ( 0x00 )

#define DMA_RR_WEIGHT_RX_BBH_PON_VALUE          ( 0x00 )
#define DMA_RR_WEIGHT_TX_BBH_PON_VALUE          ( 0x00 )
#define SDMA_RR_WEIGHT_RX_BBH_PON_VALUE         ( 0x00 )
#define SDMA_RR_WEIGHT_TX_BBH_PON_VALUE         ( 0x00 )

#define DMA_RR_WEIGHT_RX_BBH_XPORT10G_VALUE     ( 0x00 )
#define DMA_RR_WEIGHT_TX_BBH_XPORT10G_VALUE     ( 0x00 )
#define SDMA_RR_WEIGHT_RX_BBH_XPORT10G_VALUE    ( 0x00 )
#define SDMA_RR_WEIGHT_TX_BBH_XPORT10G_VALUE    ( 0x00 )

#define DMA_RR_WEIGHT_RX_BBH_XPORT2P5G_VALUE    ( 0x00 )
#define DMA_RR_WEIGHT_TX_BBH_XPORT2P5G_VALUE    ( 0x00 )
#define SDMA_RR_WEIGHT_RX_BBH_XPORT2P5G_VALUE   ( 0x00 )
#define SDMA_RR_WEIGHT_TX_BBH_XPORT2P5G_VALUE   ( 0x00 )

#define DMA_RR_WEIGHT_RX_BBH_DSL_VALUE          ( 0x00 )
#define DMA_RR_WEIGHT_TX_BBH_DSL_VALUE          ( 0x00 )
#define SDMA_RR_WEIGHT_RX_BBH_DSL_VALUE         ( 0x00 )
#define SDMA_RR_WEIGHT_TX_BBH_DSL_VALUE         ( 0x00 )

typedef enum bbh_id_e
{
    /* RX */
    BBH_ID_0 = 0,
    BBH_ID_1,
    BBH_ID_2,
    BBH_ID_PON,
    BBH_ID_AE10,
    BBH_ID_AE2P5,
    BBH_ID_DSL,
    BBH_ID_NUM,
    BBH_ID_FIRST = BBH_ID_0,
    BBH_ID_NUM_LAN = BBH_ID_2,
    BBH_ID_LAST = BBH_ID_DSL,
    BBH_ID_NULL = BBH_ID_NUM,
    /* TX */
    BBH_TX_ID_0 = 0,
    BBH_TX_LAN  = BBH_TX_ID_0,
    BBH_TX_ID_PON,
    BBH_TX_ID_AE10,
    BBH_TX_ID_AE2P5,
    BBH_TX_ID_DSL,
    BBH_TX_ID_NUM,
    BBH_TX_ID_FIRST = BBH_TX_ID_0,
    BBH_TX_ID_LAST = BBH_TX_ID_DSL,
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
    SDMA0_ID,
    SDMA1_ID,
    DMA_NUM,
    DMA_ID_FIRST = DMA0_ID,
    SDMA_ID_FIRST = SDMA0_ID,
} dma_id_e;

typedef enum rnr_quad_id_e
{
    RNR_QUAD0_ID = 0,
    RNR_QUAD_ID_LAST = RNR_QUAD0_ID,
    NUM_OF_RNR_QUADS = RNR_QUAD_ID_LAST + 1,
} rnr_quad_id_e;

#define NUM_OF_CORES_IN_QUAD                        6
#define DRV_PARSER_MASKED_DA_FILTER_NUM             2
#define DRV_PARSER_DA_FILTER_NUM                    9
#define NUM_OF_RNR_WITH_PROFILING                   6


typedef enum unimac_id_e
{
    UNIMAC0_ID = 0,
    UNIMAC1_ID,
    UNIMAC2_ID,
    UNIMAC_ID_NUM,
    UNIMAC_ID_LAST = UNIMAC2_ID,
} unimac_id_e;

typedef enum bac_if_id_e
{
    BACIF_TCAM_ID = 0,
    BACIF_HASH_ID,
    BACIF_CNPL_ID,
    BACIF_NATC_ID,
    BACIF_ACB_ID,
    BACIF_ID_NUM,
    BACIF_ID_LAST = BACIF_ACB_ID,
} bac_if_id_e;

#ifdef __cplusplus
}
#endif

#endif


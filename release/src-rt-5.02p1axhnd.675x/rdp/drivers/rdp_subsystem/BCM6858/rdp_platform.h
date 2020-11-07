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

#define RNR_FREQ_IN_MHZ 1000
#define UBUS_SLV_FREQ_IN_MHZ 500

#define MAX_NUM_OF_LLID 32
#define MAX_NUM_OF_QUEUES_IN_SCHED 32

/* CNPL */
/* update every 4177ns (N=255 ; 16384ns * 255 = ns ) */
#define CNPL_PERIODIC_UPDATE_US                 4177

/* NATC */
#define NATC_CACHE_ENTRIES_NUM   1024

/* DISP CONGESTION */
#define DIS_REOR_LINKED_LIST_BUFFER_NUM   1024
#define NUM_OF_PROCESSING_TASKS           96

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
#define SBPM_BASE_ADDRESS                   0
#define SBPM_MAX_BUFFER_NUMBER              0xFFF
#define SBPM_PRE_ALLOCTED_BUFFERS_NUMBER    2
#define SBPM_INIT_OFFSET                    (SBPM_MAX_BUFFER_NUMBER - SBPM_PRE_ALLOCTED_BUFFERS_NUMBER)  /* last 2 buffers used for tunneling */
#define SBPM_UG0_BN_THRESHOLD               0x3FD
#define SBPM_UG0_EXCL_LOW_THRESHOLD         0x320
#define SBPM_UG0_EXCL_LOW_HIST              0x14
#define SBPM_UG1_BN_THRESHOLD               0xC00
#define SBPM_UG1_EXCL_LOW_THRESHOLD         0x800
#define SBPM_UG1_EXCL_LOW_HIST              0x14
#define SBPM_MAX_NUM_OF_BNS                 ((SBPM_MAX_BUFFER_NUMBER + 1) - SBPM_PRE_ALLOCTED_BUFFERS_NUMBER)
#define TUNNEL_BN_FIRST                     (SBPM_MAX_BUFFER_NUMBER - 1)

/* BBH_RX */
#define DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET 127
#define BBH_FREQUENCY (500*1000000) /* 500 MHz */

/* BBH_TX */
#define BBH_TX_DS_PD_FIFO_SIZE_0 8
#define BBH_TX_DS_PD_FIFO_SIZE_1 8

/* HASH */
#define HASH_NUM_OF_ENGINES           4
#define HASH_NUM_OF_ENGINES_LOG2      2
#define HASH_NUM_OF_EFFECTIVE_ENGINES 4
#define HASH_NUM_OF_ENTRIES_IN_RAM    6144

/* VLAN */
#define RDPA_MAX_VLANS                  256     /**< Max number of VIDs */

/* TODO: implement */
#define IS_WAN_TX_PORT(bbh_id) (bbh_id == BBH_ID_PON)
#define IS_WAN_RX_PORT(bbh_id) (bbh_id == BBH_ID_PON)
#define IS_SDMA(dma_id)     (dma_id > DMA1_ID)
#define IS_DMA(dma_id)      (dma_id <= DMA1_ID)
#define IS_PROCESSING_RUNNER_IMAGE(i)  (rdp_core_to_image_map[i] == processing_runner_image)
#define IS_DS_TM_RUNNER_IMAGE(i)  (rdp_core_to_image_map[i] == ds_tm_runner_image)
#define IS_US_TM_RUNNER_IMAGE(i)  (rdp_core_to_image_map[i] == us_tm_runner_image)
#define DS_TM_CORE_BBH_0_1 0
#define DS_TM_CORE_BBH_2_3 4

/* TCAM table size (entries) */
#define RDP_TCAM_TABLE_SIZE     512
/* Number of TCAM engines */
#define RDP_TCAM_NUM_ENGINES    2

/* QM */
/* Max number of queues supported by QM */
#define QM_NUM_QUEUES           288
/* Max number of EPON queues supported by QM */
#define QM_NUM_EPON_QUEUES      128
/* Max number of reported queues supported by QM */
#define QM_NUM_REPORTED_QUEUES  160

/* UG max default thresholds */
#ifndef G9991
#define DS_FPM_UG_DEFAULT_PERCENTAGE        (34)
#define US_FPM_UG_DEFAULT_PERCENTAGE        (66)
#else
#define DS_FPM_UG_DEFAULT_PERCENTAGE        (60)
#define US_FPM_UG_DEFAULT_PERCENTAGE        (40)
#endif
#define WLAN_FPM_UG_DEFAULT_PERCENTAGE      (0)

#define DS_FPM_UG_NO_XEPON_PERCENTAGE       (13)
#define US_FPM_UG_NO_XEPON_PERCENTAGE       (39)
#define WLAN_FPM_UG_NO_XEPON_PERCENTAGE     (48)

#define DS_FPM_UG_XEPON_PERCENTAGE          (13)
#define US_FPM_UG_XEPON_PERCENTAGE          (61)
#define WLAN_FPM_UG_XEPON_PERCENTAGE        (26)


/*XGPON DEFS*/
/* size of each one of FIFOs 0-7 - value is size + 1*/
#define BBH_TX_XGPON_PD_FIFO_SIZE_0_7 11
#define BBH_TX_XGPON_PD_FIFO_SIZE_8_15 11
#define BBH_TX_XGPON_PD_FIFO_SIZE_16_23 11
#define BBH_TX_XGPON_PD_FIFO_SIZE_24_31 11
#define BBH_TX_XGPON_PD_FIFO_SIZE_32_39 11

#define BBH_TX_GPON_PD_FIFO_SIZE_0_7 5
#define BBH_TX_GPON_PD_FIFO_SIZE_8_15 5
#define BBH_TX_GPON_PD_FIFO_SIZE_16_23 5
#define BBH_TX_GPON_PD_FIFO_SIZE_24_31 5
#define BBH_TX_GPON_PD_FIFO_SIZE_32_39 5

/*EPON DEFS*/
#define BBH_TX_XEPON_PD_FIFO_SIZE_0_7 19
#define BBH_TX_XEPON_PD_FIFO_SIZE_8_15 19
#define BBH_TX_XEPON_PD_FIFO_SIZE_16_23 0
#define BBH_TX_XEPON_PD_FIFO_SIZE_24_31 0
#define BBH_TX_XEPON_PD_FIFO_SIZE_32_39 0

#define BBH_TX_EPON_PD_FIFO_SIZE_0_7 5
#define BBH_TX_EPON_PD_FIFO_SIZE_8_15 5
#define BBH_TX_EPON_PD_FIFO_SIZE_16_23 0
#define BBH_TX_EPON_PD_FIFO_SIZE_24_31 0
#define BBH_TX_EPON_PD_FIFO_SIZE_32_39 0

typedef enum dma_id_e
{
    DMA0_ID = 0,
    DMA1_ID,
    SDMA0_ID,
    SDMA1_ID,
    DMA_NUM,
    DMA_ID_FIRST = DMA0_ID,
} dma_id_e;

typedef enum bbh_id_e
{
    BBH_ID_0 = 0,  /* XLMAC1_0_RGMII */
    BBH_ID_1,      /* XLMAC0_1_2p5G  */
    BBH_ID_2,      /* XLMAC0_2_1G    */
    BBH_ID_3,      /* XLMAC0_3_1G    */
    BBH_ID_4,      /* XLMAC0_0_10G   */
    BBH_ID_5,      /* XLMAC1_1_RGMII */
    BBH_ID_6,      /* XLMAC1_2_RGMII */
    BBH_ID_7,      /* XLMAC1_3_RGMII */
    BBH_ID_PON,
    BBH_ID_NUM,
    BBH_ID_FIRST = BBH_ID_0,
    BBH_ID_LAST = BBH_ID_PON,
    BBH_ID_LAST_XLMAC = BBH_ID_7,
    BBH_ID_NULL = BBH_ID_NUM
} bbh_id_e;

typedef enum rnr_quad_id_e
{
    RNR_QUAD0_ID = 0,
    RNR_QUAD1_ID,
    RNR_QUAD2_ID,
    RNR_QUAD3_ID,
    RNR_QUAD_ID_LAST = RNR_QUAD3_ID,
    NUM_OF_RNR_QUADS = RNR_QUAD_ID_LAST + 1,
} rnr_quad_id_e;

#define DMA_MAX_READ_ON_THE_FLY ( 16 )
#define SDMA_MAX_READ_ON_THE_FLY ( 8 )

#define NUM_OF_CORES_IN_QUAD                        4
#define DRV_PARSER_MASKED_DA_FILTER_NUM             2
#define DRV_PARSER_DA_FILTER_NUM                    9
#define NUM_OF_RNR_WITH_PROFILING                   16

#define DISP_REOR_VIQ_BBH_WAN_EXCL DISP_REOR_VIQ_BBH_RX8_EXCL

#endif

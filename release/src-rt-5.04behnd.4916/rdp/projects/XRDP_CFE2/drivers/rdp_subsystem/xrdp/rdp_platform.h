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

/* SBPM */
#define SBPM_BASE_ADDRESS                   0
#define SBPM_INIT_OFFSET                    (SBPM_MAX_BUFFER_NUMBER)
#define SBPM_MAX_NUM_OF_BNS                 (SBPM_MAX_BUFFER_NUMBER + 1)

typedef enum rnr_quad_id_e
{
    RNR_QUAD0_ID = 0,
    RNR_QUAD_ID_LAST = RNR_QUAD0_ID,
    NUM_OF_RNR_QUADS = RNR_QUAD_ID_LAST + 1,
} rnr_quad_id_e;

#if  defined(BCM6858)

#define RNR_FREQ_IN_MHZ 1000
#define UBUS_SLV_FREQ_IN_MHZ 500
#define NUM_OF_CORES_IN_QUAD 4

/* DISP CONGESTION */
#define DIS_REOR_CONGESTION_THRESHOLD    450
#define DIS_REOR_LINKED_LIST_BUFFER_NUM  1024
#define NUM_OF_PROCESSING_TASKS          96

/* BBH_RX */
#define DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET 127

/* BBH_TX */
#define BBH_TX_DS_PD_FIFO_SIZE_0 8
#define BBH_TX_DS_PD_FIFO_SIZE_1 8

#define IS_WAN_TX_PORT(bbh_id) (bbh_id == BBH_ID_PON)
#define IS_SDMA(dma_id)     (dma_id > DMA1_ID)
#define IS_DMA(dma_id)      (dma_id <= DMA1_ID)

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


/* SBPM */

#define SBPM_MAX_BUFFER_NUMBER              0xFFF


/**********************************************************************************************************/
#elif  defined(BCM6846)

#define RNR_FREQ_IN_MHZ 1400
#define UBUS_SLV_FREQ_IN_MHZ 466
#define NUM_OF_CORES_IN_QUAD 3

#define IS_WAN_TX_PORT(bbh_id) (bbh_id == BBH_TX_WAN_ID)
#define IS_WAN_RX_PORT(bbh_id) (bbh_id == BBH_ID_PON)
#define IS_SDMA(dma_id)  (dma_id > DMA0_ID)
#define IS_DMA(dma_id)   (dma_id <= DMA0_ID)


/* DISP CONGESTION */
#define DIS_REOR_CONGESTION_THRESHOLD    200
#define DIS_REOR_LINKED_LIST_BUFFER_NUM  512
#define NUM_OF_PROCESSING_TASKS          12

/* SBPM */

#define SBPM_MAX_BUFFER_NUMBER              0x5FF


/* BBH_RX */
#define DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET 63
#define BBH_FREQUENCY (466770000) /* 466.77 MHz */

/* BBH_TX */
#define BBH_TX_DS_PD_FIFO_SIZE_0 7
#define BBH_TX_DS_PD_FIFO_SIZE_1 7

typedef enum bbh_id_e
{
    BBH_ID_0 = 0,
    BBH_ID_1,
    BBH_ID_2,
    BBH_ID_3,
    BBH_ID_4,
    BBH_ID_PON,
    BBH_ID_NUM,
    BBH_ID_FIRST = BBH_ID_0,
    BBH_ID_LAST = BBH_ID_PON,
    BBH_ID_NULL = BBH_ID_NUM
} bbh_id_e;

typedef enum dma_id_e
{
    DMA0_ID = 0,
    SDMA0_ID,
    DMA_NUM,
    DMA_ID_FIRST = DMA0_ID,
} dma_id_e;

/**********************************************************************************************************/

#elif  defined(BCM6878)
#define RNR_FREQ_IN_MHZ 1000
#define UBUS_SLV_FREQ_IN_MHZ 466
#define NUM_OF_CORES_IN_QUAD 3

#define IS_WAN_TX_PORT(bbh_id) (bbh_id == BBH_TX_WAN_ID)
#define IS_WAN_RX_PORT(bbh_id) (bbh_id == BBH_ID_PON)
#define IS_SDMA(dma_id)  (dma_id > DMA0_ID)
#define IS_DMA(dma_id)   (dma_id <= DMA0_ID)


/* DISP CONGESTION */
#define DIS_REOR_CONGESTION_THRESHOLD    100
#define DIS_REOR_LINKED_LIST_BUFFER_NUM  256
#define NUM_OF_PROCESSING_TASKS          12

/* BBH_RX */
#define DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET 63
#define BBH_FREQUENCY (466770000) /* 466.77 MHz */

/* BBH_TX */
#define BBH_TX_DS_PD_FIFO_SIZE_0 7
#define BBH_TX_DS_PD_FIFO_SIZE_1 7

typedef enum bbh_id_e
{
    BBH_ID_0 = 0,
    BBH_ID_1,
    BBH_ID_2,
    BBH_ID_3,
    BBH_ID_4,
    BBH_ID_PON,
    BBH_ID_NUM,
    BBH_ID_FIRST = BBH_ID_0,
    BBH_ID_LAST = BBH_ID_PON,
    BBH_ID_NULL = BBH_ID_NUM
} bbh_id_e;

typedef enum dma_id_e
{
    DMA0_ID = 0,
    DMA_NUM,
    DMA_ID_FIRST = DMA0_ID,
} dma_id_e;

typedef enum bbh_tx_id_e
{
    BBH_TX_ID_LAN = 0,
    BBH_TX_ID_PON,
    BBH_TX_ID_NUM,
    BBH_TX_ID_FIRST = BBH_TX_ID_LAN,
    BBH_TX_ID_LAST = BBH_TX_ID_PON,
    BBH_TX_ID_NULL = BBH_TX_ID_NUM
} bbh_tx_id_e;

/* SBPM */
#define SBPM_MAX_BUFFER_NUMBER              0x3FF
#elif  defined(BCM6855)
#define RNR_FREQ_IN_MHZ 1000
#define UBUS_SLV_FREQ_IN_MHZ 500
#define NUM_OF_CORES_IN_QUAD 6
#define DRV_PARSER_MASKED_DA_FILTER_NUM 2
#define DRV_PARSER_DA_FILTER_NUM 9
#define NUM_OF_RNR_WITH_PROFILING 3

#define IS_WAN_TX_PORT(bbh_id) (bbh_id == BBH_TX_WAN_ID)
#define IS_WAN_RX_PORT(bbh_id) (bbh_id == BBH_ID_PON)
#define IS_SDMA(dma_id)  (dma_id > DMA0_ID)
#define IS_DMA(dma_id)   (dma_id <= DMA0_ID)

/* DISP CONGESTION */
#define DIS_REOR_CONGESTION_THRESHOLD    100
#define DIS_REOR_LINKED_LIST_BUFFER_NUM  256
#define NUM_OF_PROCESSING_TASKS          12

#define DSPTCHR_CONG_PARAM_INGRESS_NORMAL 1023
#define DSPTCHR_CONG_PARAM_EGRESS_NORMAL  1023
#define DSPTCHR_CONG_PARAM_GLOBAL         (DIS_REOR_LINKED_LIST_BUFFER_NUM - DSPTCHR_VIRTUAL_QUEUE_NUM - 2)
#define DSPTCHR_CONG_PARAM_EGRESS_GLOBAL  300
#define DSPTCHR_RESERVED_PRIORITY_BUFF_NUM 100
#define DSPTCHR_CONG_PARAM_HYST           8
#define DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ             (DIS_REOR_LINKED_LIST_BUFFER_NUM - 1)
#define DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ  8
#define DSPTCHR_GUARANTEED_MAX_LIMIT_PER_EXCLUS_VIQ  16

/* SBPM */
#define SBPM_UG0_BN_THRESHOLD               0x200
#define SBPM_UG0_EXCL_LOW_THRESHOLD         0x1C0

/* BBH_RX */
#define DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET 63
#define BBH_FREQUENCY (466770000) /* 466.77 MHz */

/* BBH_TX */
#define BBH_TX_DS_PD_FIFO_SIZE_0 7
#define BBH_TX_DS_PD_FIFO_SIZE_1 15

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
    BBH_TX_ID_LAN = 0,
    BBH_ID_QM_COPY,
    BBH_TX_ID_PON,

    BBH_TX_ID_NUM,
    BBH_TX_ID_FIRST = BBH_TX_ID_LAN,
    BBH_TX_ID_LAST = BBH_TX_ID_PON,
    BBH_TX_ID_NULL = BBH_ID_NULL
} bbh_id_e;

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

#define SDMA_STRICT_PRI_RX_BBH_5G_VALUE         ( 0x04 )
#define SDMA_STRICT_PRI_RX_BBH_10G_VALUE        ( 0x04 )
#define SDMA_STRICT_PRI_RX_BBH_2P5G_VALUE       ( 0x04 )

#define SDMA_STRICT_PRI_TX_BBH_LAN_VALUE        ( 0x04 )
#define SDMA_STRICT_PRI_TX_BBH_PON_VALUE        ( 0x04 )
#define SDMA_STRICT_PRI_TX_BBH_COPY_VALUE       ( 0x04 )
#define DMA_STRICT_PRI_TX_BBH_COPY_VALUE        ( 0x08 )

#define SDMA_RR_WEIGHT_RX_BBH_5G_VALUE          ( 0x00 )
#define SDMA_RR_WEIGHT_RX_BBH_2P5G_VALUE        ( 0x00 )
#define SDMA_RR_WEIGHT_RX_BBH_DSL_VALUE         ( 0x00 )
#define SDMA_RR_WEIGHT_RX_BBH_10G_VALUE         ( 0x00 )

#define SDMA_RR_WEIGHT_TX_BBH_COPY_VALUE        ( 0x00 )
#define DMA_RR_WEIGHT_TX_BBH_COPY_VALUE         ( 0x00 )

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

#define FPM_INTERRUPT_MASK_OFF              {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
#define FPM_INTERRUPT_TIMER_DELAY           1000  /* msec */
#define SBPM_MAX_BUFFER_NUMBER              0x7FF


#elif  defined(BCM6888) || defined(BCM68880)

#define RNR_FREQ_IN_MHZ 1250
#define UBUS_SLV_FREQ_IN_MHZ 880

#define IS_WAN_TX_PORT(bbh_id) (bbh_id == BBH_TX_WAN_ID)
#define IS_WAN_RX_PORT(bbh_id) (bbh_id == BBH_ID_PON)
#define IS_SDMA(dma_id)  (dma_id > DMA0_ID)
#define IS_DMA(dma_id)   (dma_id <= DMA0_ID)

/* DISP CONGESTION */
#define DIS_REOR_CONGESTION_THRESHOLD    100
#define DIS_REOR_LINKED_LIST_BUFFER_NUM  1024
#define NUM_OF_PROCESSING_TASKS          12

#define DISP_REOR_VIQ_BBH_RX10_NORMAL       	10
#define DISP_REOR_VIQ_BBH_RX10_EXCL         	11
#define DISP_REOR_VIQ_BBH_WAN_EXCL DISP_REOR_VIQ_BBH_RX10_EXCL
#define DISP_REOR_VIQ_BBH_RX_MAX_NUM 16
#define DSPTCHR_CONG_PARAM_INGRESS_NORMAL 1023
#define DSPTCHR_CONG_PARAM_EGRESS_NORMAL  1023
#define DSPTCHR_CONG_PARAM_GLOBAL         (DIS_REOR_LINKED_LIST_BUFFER_NUM - DSPTCHR_VIRTUAL_QUEUE_NUM - 2)
#define DSPTCHR_CONG_PARAM_EGRESS_GLOBAL  300
#define DSPTCHR_RESERVED_PRIORITY_BUFF_NUM 100
#define DSPTCHR_CONG_PARAM_HYST           8
#define DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ             (DIS_REOR_LINKED_LIST_BUFFER_NUM - 1)
#define DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ  8
#define DSPTCHR_GUARANTEED_MAX_LIMIT_PER_EXCLUS_VIQ  16

/* SBPM */
#define SBPM_UG0_BN_THRESHOLD               0x3FF
#define SBPM_UG0_EXCL_LOW_THRESHOLD         0x320
#define SBPM_UG1_EXCL_LOW_THRESHOLD         0x980
#define SBPM_UG0_EXCL_LOW_HIST              0x14
#define SBPM_UG1_BN_THRESHOLD               0xC00
#define MAX_OMCI_PKT_SIZE 1980
#define SBPM_UG1_EXCL_LOW_HIST              0x14
#define SBPM_BASE_ADDRESS                   0
#define SBPM_MAX_BUFFER_NUMBER              0xFFF
#define SBPM_UG1_EXCL_HIGH_THRESHOLD        0xb80


/* BBH_RX */
#define DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET 63
#define BBH_FREQUENCY (466770000) /* 466.77 MHz */

/* XLIF */
#define XLIF0_ACTIVE_CHANNELS 3
#define XLIF1_ACTIVE_CHANNELS 4
#define XLIF2_ACTIVE_CHANNELS 4

/* BBH_TX */
#define BBH_TX_DS_PD_FIFO_SIZE_0 7
#define BBH_TX_DS_PD_FIFO_SIZE_1 15


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

#define SBPM_MAX_BUFFER_NUMBER              0xFFF

#elif  defined(BCM6837)
#define RNR_FREQ_IN_MHZ 1250
#define UBUS_SLV_FREQ_IN_MHZ 750

#define IS_WAN_TX_PORT(bbh_id) (bbh_id == BBH_TX_WAN_ID)
#define IS_WAN_RX_PORT(bbh_id) (bbh_id == BBH_ID_PON)
#define IS_SDMA(dma_id)  (dma_id > DMA0_ID)
#define IS_DMA(dma_id)   (dma_id <= DMA0_ID)

/* DISP CONGESTION */
#define DIS_REOR_CONGESTION_THRESHOLD       100
#define DIS_REOR_LINKED_LIST_BUFFER_NUM     1024
#define NUM_OF_PROCESSING_TASKS             12

#define DISP_REOR_VIQ_BBH_RX10_NORMAL       10
#define DISP_REOR_VIQ_BBH_RX10_EXCL         11
#define DISP_REOR_VIQ_BBH_WAN_EXCL          DISP_REOR_VIQ_BBH_RX10_EXCL
#define DISP_REOR_VIQ_BBH_RX_MAX_NUM        DISP_REOR_VIQ_BBH_RX10_NORMAL
#define DSPTCHR_CONG_PARAM_INGRESS_NORMAL   1023
#define DSPTCHR_CONG_PARAM_EGRESS_NORMAL    1023
#define DSPTCHR_CONG_PARAM_GLOBAL           (DIS_REOR_LINKED_LIST_BUFFER_NUM - DSPTCHR_VIRTUAL_QUEUE_NUM - 2)
#define DSPTCHR_CONG_PARAM_EGRESS_GLOBAL    300
#define DSPTCHR_RESERVED_PRIORITY_BUFF_NUM  100
#define DSPTCHR_CONG_PARAM_HYST             8
#define DSPTCHR_COMMON_MAX_LIMIT_PER_VIQ    (DIS_REOR_LINKED_LIST_BUFFER_NUM - 1)
#define DSPTCHR_GUARANTEED_MAX_LIMIT_PER_NORMAL_VIQ  8
#define DSPTCHR_GUARANTEED_MAX_LIMIT_PER_EXCLUS_VIQ  16

/* SBPM */
#define SBPM_UG0_BN_THRESHOLD               0x3FF
#define SBPM_UG0_EXCL_LOW_THRESHOLD         0x320
#define SBPM_UG1_EXCL_LOW_THRESHOLD         0x180
#define SBPM_UG0_EXCL_LOW_HIST              0x14
#define SBPM_UG1_BN_THRESHOLD               0x400
#define MAX_OMCI_PKT_SIZE                   1980
#define SBPM_UG1_EXCL_LOW_HIST              0x14
#define SBPM_BASE_ADDRESS                   0
#define SBPM_MAX_BUFFER_NUMBER              0x7FF
#define SBPM_UG1_EXCL_HIGH_THRESHOLD        0x380

/* XLIF */
#define XLIF0_ACTIVE_CHANNELS 3
#define XLIF1_ACTIVE_CHANNELS 4
#define XLIF2_ACTIVE_CHANNELS 4

/* BBH_RX */
#define DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET 63
#define BBH_FREQUENCY (466770000) /* 466.77 MHz */

/* BBH_TX */
#define BBH_TX_DS_PD_FIFO_SIZE_0 7
#define BBH_TX_DS_PD_FIFO_SIZE_1 15


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
    BBH_ID_PON,
    BBH_ID_NUM,
    BBH_ID_FIRST = BBH_ID_0,
    BBH_ID_LAST = BBH_ID_PON,
    BBH_ID_NULL = BBH_ID_NUM,

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

/* size of each one of FIFOs 0-7 - value is size + 1*/
#define BBH_TX_GPON_PD_FIFO_SIZE_0_7 5
#define BBH_TX_GPON_PD_FIFO_SIZE_8_15 5
#define BBH_TX_GPON_PD_FIFO_SIZE_16_23 5
#define BBH_TX_GPON_PD_FIFO_SIZE_24_31 5
#define BBH_TX_GPON_PD_FIFO_SIZE_32_39 5

#elif defined(BCM6856) 

#define RNR_FREQ_IN_MHZ 1400
#define UBUS_SLV_FREQ_IN_MHZ 500
#define NUM_OF_CORES_IN_QUAD 4

#define IS_WAN_TX_PORT(bbh_id) (bbh_id == BBH_TX_WAN_ID)
#define IS_SDMA(dma_id)  (dma_id > DMA0_ID)
#define IS_DMA(dma_id)   (dma_id <= DMA0_ID)

/* DISP CONGESTION */
#define DIS_REOR_CONGESTION_THRESHOLD    200
#define DIS_REOR_LINKED_LIST_BUFFER_NUM  1024
#define NUM_OF_PROCESSING_TASKS          32

/* SBPM */
#define SBPM_UG0_BN_THRESHOLD               0x3FF
#define SBPM_MAX_BUFFER_NUMBER              0x7FF

/* BBH_RX */
#define DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET 63
#define BBH_FREQUENCY (500*1000000) /* 500 MHz */

/* BBH_TX */
#define BBH_TX_DS_PD_FIFO_SIZE_0 7
#define BBH_TX_DS_PD_FIFO_SIZE_1 7

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
    BBH_ID_NULL = BBH_ID_NUM
} bbh_id_e;

typedef enum bbh_tx_id_e
{
    BBH_TX_ID_LAN = 0,
    BBH_TX_ID_PON,
    BBH_TX_ID_NUM,
    BBH_TX_ID_FIRST = BBH_TX_ID_LAN,
    BBH_TX_ID_LAST = BBH_TX_ID_PON,
    BBH_TX_ID_NULL = BBH_TX_ID_NUM
} bbh_tx_id_e;

#elif defined(BCM63146) || defined(BCM4912) || defined(BCM6813)

#define RNR_FREQ_IN_MHZ 1250
#define UBUS_SLV_FREQ_IN_MHZ 560 /* TBD63146. Need for BCM63146 */
/* TBD below, not used */
#define NUM_OF_CORES_IN_QUAD 3

/* IS_WAN_TX_PORT and IS_WAN_RX_PORT identify TX WAN ports for DSL only (NOT Ethernet) */
#define IS_WAN_TX_PORT(bbh_id) (bbh_id == BBH_TX_ID_DSL)
#define IS_WAN_RX_PORT(bbh_id) (bbh_id == BBH_ID_DSL)
/* IS_DS_WAN_PORT and IS_US_WAN_PORT identify ALL (DS/US respectively) WAN ports */
/* TBD63146. Identify Ethernet WAN */
#define IS_DS_WAN_PORT(bbh_id)  (bbh_id == BBH_ID_DSL)
#define IS_US_WAN_PORT(bbh_id) (bbh_id == BBH_TX_ID_DSL)

/* DISP CONGESTION */
#define DIS_REOR_CONGESTION_THRESHOLD     200
#define DIS_REOR_LINKED_LIST_BUFFER_NUM   512
#define NUM_OF_PROCESSING_TASKS           32

/* SBPM */
#define SBPM_MAX_BUFFER_NUMBER              0x17F
#define SBPM_UG0_BN_THRESHOLD               0x15F

/* BBH_RX */
#define DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET 127
#define BBH_FREQUENCY (560*1000000) /* 560 MHz */ /* TBD63146. Need for BCM63146 */

/* BBH_TX */
#define BBH_TX_DS_PD_FIFO_SIZE_0 7
#define BBH_TX_DS_PD_FIFO_SIZE_1 7

#if defined(BCM63146)
typedef enum dma_id_e
{
    DMA0_ID = 0,
    DMA1_ID,
    DMA_NUM,
    DMA_ID_FIRST = DMA0_ID,
} dma_id_e;

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
} bbh_id_e;

typedef enum bbh_tx_id_e
{
    BBH_TX_ID_LAN = 0,
    BBH_ID_QM_COPY,
    BBH_TX_ID_DSL,
    BBH_TX_ID_NUM,
    BBH_TX_ID_FIRST = BBH_TX_ID_LAN,
    BBH_TX_ID_LAST = BBH_TX_ID_DSL,
    BBH_TX_ID_NULL = BBH_ID_NULL,
} bbh_tx_id_e;

#define SDMA_U_THRESH_IN_BBH_LAN_VALUE    ( 0x2 )
#define SDMA_U_THRESH_OUT_BBH_LAN_VALUE   ( 0x1 )

#define SDMA_U_THRESH_IN_BBH_5G_VALUE     ( 0x3 )
#define SDMA_U_THRESH_OUT_BBH_5G_VALUE    ( 0x2 )

#define SDMA_U_THRESH_IN_BBH_2P5G_VALUE   ( 0x3 )
#define SDMA_U_THRESH_OUT_BBH_2P5G_VALUE  ( 0x2 )

#define SDMA_U_THRESH_IN_BBH_DSL_VALUE    ( 0x6 )
#define SDMA_U_THRESH_OUT_BBH_DSL_VALUE   ( 0x4 )

#define SDMA_STRICT_PRI_RX_BBH_LAN_VALUE  ( 0x4 )
#define SDMA_STRICT_PRI_RX_BBH_5G_VALUE   ( 0x04 )
#define SDMA_STRICT_PRI_RX_BBH_2P5G_VALUE ( 0x04 )
#define SDMA_STRICT_PRI_RX_BBH_DSL_VALUE  ( 0x4 )

#define SDMA_STRICT_PRI_TX_BBH_LAN_VALUE  ( 0x04 )
#define DMA_STRICT_PRI_TX_BBH_LAN_VALUE   ( 0x08 )
#define SDMA_STRICT_PRI_TX_BBH_COPY_VALUE ( 0x04 )
#define DMA_STRICT_PRI_TX_BBH_COPY_VALUE  ( 0x08 )
#define SDMA_STRICT_PRI_TX_BBH_DSL_VALUE  ( 0x04 )
#define DMA_STRICT_PRI_TX_BBH_DSL_VALUE   ( 0x08 )

#define SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE   ( 0x00 )
#define SDMA_RR_WEIGHT_RX_BBH_5G_VALUE    ( 0x00 )
#define SDMA_RR_WEIGHT_RX_BBH_2P5G_VALUE  ( 0x00 )
#define SDMA_RR_WEIGHT_RX_BBH_DSL_VALUE   ( 0x00 )

#define SDMA_RR_WEIGHT_TX_BBH_LAN_VALUE   ( 0x00 )
#define DMA_RR_WEIGHT_TX_BBH_LAN_VALUE    ( 0x00 )
#define SDMA_RR_WEIGHT_TX_BBH_COPY_VALUE  ( 0x00 )
#define DMA_RR_WEIGHT_TX_BBH_COPY_VALUE   ( 0x00 )
#define SDMA_RR_WEIGHT_TX_BBH_DSL_VALUE   ( 0x00 )
#define DMA_RR_WEIGHT_TX_BBH_DSL_VALUE    ( 0x00 )
#else /* if defined(BCM4912) || defined(BCM6813) */
typedef enum dma_id_e
{
    DMA0_ID = 0,
    DMA1_ID,
    DMA_COPY_ID,
    DMA_NUM,
    DMA_ID_FIRST = DMA0_ID,
} dma_id_e;

typedef enum bbh_id_e
{
    /* RX */
    /* RX BBH_ID needs to be the same as DISP_REOR_VIQ defined in
     * project_data_structures.xml or else bbh_rx configuration will break */
    BBH_ID_0 = 0,
    BBH_ID_1,
    BBH_ID_2,
    BBH_ID_3,
    BBH_ID_4,    /* 4912: RGMII.  6813: AE_10G */
    BBH_ID_5_10G,
    BBH_ID_6_5G, /* for 4912B0 and 6813, it becomes 10G port */
    BBH_ID_7_10G,
    BBH_ID_8_5G_2P5G,
    BBH_ID_9_2P5G,
    BBH_ID_10_2P5G,
#if defined(BCM6813)
    BBH_ID_PON,
#endif
    BBH_ID_NUM,
    BBH_ID_FIRST = BBH_ID_0,
    BBH_ID_NUM_LAN = BBH_ID_10_2P5G,
    BBH_ID_LAST = BBH_ID_10_2P5G,
    BBH_ID_NULL = BBH_ID_NUM,
    BBH_ID_DSL = BBH_ID_NULL,
} bbh_id_e;

typedef enum bbh_tx_id_e
{
    BBH_TX_ID_LAN = 0,
    /* BBH Queue 0-3: QGPHY
     * BBH Queue 4: RGMII (4912) or AE_10G (6813)
     * BBH Queue 5: XLMAC 0, port 0, 10Gbps */
    BBH_ID_QM_COPY,
    BBH_TX_ID_LAN_1,
#if defined(BCM6813)
    BBH_TX_ID_PON,
#endif
    /* BBH Queue 0: XLMAC 0, port 2, 5Gbps
     * BBH Queue 1: XLMAC 2, port 0, 10Gbps */
    BBH_TX_ID_NUM,
    BBH_TX_ID_FIRST = BBH_TX_ID_LAN,
    BBH_TX_ID_LAST = BBH_TX_ID_NUM - 1,
    BBH_TX_ID_NULL = BBH_ID_NULL,
    BBH_TX_ID_DSL = BBH_ID_NULL,
} bbh_tx_id_e;

typedef enum xlif_id_e
{
    XLIF_ID_CHANNEL_FIRST = 0,
    XLIF_ID_CHANNEL_0 = XLIF_ID_CHANNEL_FIRST,
    XLIF_ID_CHANNEL_1,
    XLIF_ID_CHANNEL_2,
    XLIF_ID_CHANNEL_3,
    XLIF_ID_CHANNEL_NUM,
} xlif_id_e;

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
#endif

typedef enum dma_target_e
{
    DMA_TARGET_DMA = 0,
    DMA_TARGET_SDMA,
    DMA_TARGET_NUM,
    DMA_TARGET_FIRST = DMA_TARGET_DMA,
} dma_target_e;

/* some change of definition for easy to compilation */
#if !defined(BCM6813)
#define BBH_ID_PON  BBH_ID_DSL
#endif
#define BB_ID_RX_PON  BB_ID_RX_DSL
#define BB_ID_TX_PON_ETH_PD  BB_ID_TX_DSL
#define BB_ID_TX_PON_ETH_STAT  BB_ID_TX_DSL_STAT

#endif


#if !defined(BCM63146) && !defined(BCM4912) && !defined(BCM6888) &&!defined(BCM68880) && \
    !defined(BCM6813) && !defined(BCM6837)
#define DMA_U_THRESH_IN_BBH_LAN_VALUE     ( 0x3 )
#define DMA_U_THRESH_OUT_BBH_LAN_VALUE    ( 0x2 )
#define SDMA_U_THRESH_IN_BBH_LAN_VALUE    ( 0x3 )
#define SDMA_U_THRESH_OUT_BBH_LAN_VALUE   ( 0x2 )

#define DMA_U_THRESH_IN_BBH_PON_VALUE  ( 0x8 )
#define DMA_U_THRESH_OUT_BBH_PON_VALUE ( 0x6 )
#define SDMA_U_THRESH_IN_BBH_PON_VALUE    ( 0x8 )
#define SDMA_U_THRESH_OUT_BBH_PON_VALUE   ( 0x6 )

#define DMA_STRICT_PRI_RX_BBH_LAN_VALUE   ( 0x4 )
#define DMA_STRICT_PRI_TX_BBH_LAN_VALUE   ( 0x8 )
#define SDMA_STRICT_PRI_RX_BBH_LAN_VALUE  ( 0x4 )
#define SDMA_STRICT_PRI_TX_BBH_LAN_VALUE  ( 0x8 )

#define DMA_STRICT_PRI_RX_BBH_PON_VALUE   ( 0x4 )
#define DMA_STRICT_PRI_TX_BBH_PON_VALUE   ( 0x8 )
#define SDMA_STRICT_PRI_RX_BBH_PON_VALUE  ( 0x4 )
#define SDMA_STRICT_PRI_TX_BBH_PON_VALUE  ( 0x8 )

#define DMA_RR_WEIGHT_RX_BBH_LAN_VALUE    ( 0x0 )
#define DMA_RR_WEIGHT_TX_BBH_LAN_VALUE    ( 0x0 )

#define SDMA_RR_WEIGHT_RX_BBH_LAN_VALUE   ( 0x0 )
#define SDMA_RR_WEIGHT_TX_BBH_LAN_VALUE   ( 0x0 )

#define DMA_RR_WEIGHT_RX_BBH_PON_VALUE    ( 0x0 )
#define DMA_RR_WEIGHT_TX_BBH_PON_VALUE    ( 0x0 )
#define SDMA_RR_WEIGHT_RX_BBH_PON_VALUE   ( 0x0 )
#define SDMA_RR_WEIGHT_TX_BBH_PON_VALUE   ( 0x0 )
#endif

#endif

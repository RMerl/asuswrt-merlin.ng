// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013 Broadcom
 */
/*
    
*/
/******************************************************************************/
/*                                                                            */
/* File Description:                                                          */
/*                                                                            */
/* This file contains the implementation for Broadcom's BCM63138 Data path    */
/* initialization sequence                                                    */
/*                                                                            */
/******************************************************************************/

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/


#ifdef __UBOOT__
#include <common.h>
#endif
#include "data_path_init.h"
#include "rdpa_types.h"
#include "rdpa_config.h"
#include "rdp_drv_bpm.h"
#include "rdp_drv_sbpm.h"
#include "rdp_drv_ih.h"
#include "rdp_drv_bbh.h"
#include "rdp_dma.h"
#include "rdd_ih_defs.h"
#include "rdd.h"
#include "rdd_tm.h"

static  S_DPI_CFG   DpiBasicConfig = {WAN_TYPE_NONE,1536,0,0,0,0};

#if defined(BDMF_SYSTEM_SIM)
#define __print(fmt, arg...)
#elif defined(__UBOOT__)
#define __print(fmt, arg...) printf(fmt, ##arg)
#else
#define __print(fmt, arg...) printk(fmt, ##arg)
#endif

#define BCM63138_TM_DEF_DDR_SIZE                    0x0E00000
#define BCM63138_TM_MC_DEF_DDR_SIZE                 0x0400000
#define CS_RDD_ETH_TX_QUEUE_PACKET_THRESHOLD    256
#define CS_RDD_CPU_RX_QUEUE_SIZE                32


#define SHIFTL(_a) ( 1 << _a)
/****************************************************************************
 *
 * Defines
 *
 * *************************************************************************/

#define DEFAULT_RUNNER_FREQ             800
#define RDD_CPU_TX_ABS_FIFO_SIZE        LILAC_RDD_CPU_TX_SKB_LIMIT_MAX

/* multicast header size */
#define BBH_MULTICAST_HEADER_SIZE_FOR_LAN_PORT 32
#define BBH_MULTICAST_HEADER_SIZE_FOR_WAN_PORT 96

/* PD FIFO size of EMAC, when MDU mode is disabled */
#define BBH_TX_EMAC_PD_FIFO_SIZE_MDU_MODE_DISABLED 8
/* PD FIFO size of EMAC, when MDU mode is enabled */
#define BBH_TX_EMAC_PD_FIFO_SIZE_MDU_MODE_ENABLED 4
/* DMA */
#define BBH_RX_DMA_FIFOS_SIZE_WAN_DMA 19
#define BBH_RX_DMA_FIFOS_SIZE_WAN_DSL 19
#define BBH_RX_DMA_FIFOS_SIZE_LAN_DMA 9
#define BBH_RX_DMA_FIFOS_SIZE_LAN_BBH 11
#define BBH_RX_DMA_EXCLUSIVE_THRESHOLD_WAN_DMA 17
#define BBH_RX_DMA_EXCLUSIVE_THRESHOLD_WAN_BBH 18
#define BBH_RX_DMA_EXCLUSIVE_THRESHOLD_LAN_DMA 7
#define BBH_RX_DMA_EXCLUSIVE_THRESHOLD_LAN_BBH 11
#define BBH_RX_SDMA_FIFOS_SIZE_WAN 7
#define BBH_RX_SDMA_FIFOS_SIZE_LAN 5
#define BBH_RX_SDMA_EXCLUSIVE_THRESHOLD_WAN 6
#define BBH_RX_SDMA_EXCLUSIVE_THRESHOLD_LAN 4
#define BBH_RX_DMA_TOTAL_NUMBER_OF_CHUNK_DESCRIPTORS 64
#define BBH_RX_SDMA_TOTAL_NUMBER_OF_CHUNK_DESCRIPTORS 32

#define ATM_MIN_DATA_SIZE    4   /* Such as PPPoA or IPoA */
#define MIN_XTM_PKT_SIZE ATM_MIN_DATA_SIZE
#define MIN_ETH_PKT_SIZE 64
#define BBH_RX_FLOWS_32_255_GROUP_DIVIDER 255
#define BBH_RX_ETH_MIN_PKT_SIZE_SELECTION_INDEX 0

#define BBH_RX_MAX_PKT_SIZE_SELECTION_INDEX 0

//#define IH_HEADER_LENGTH_MIN 64
#define IH_HEADER_LENGTH_MIN ATM_MIN_DATA_SIZE

#define IH_PARSER_EXCEPTION_STATUS_BITS 0x47
#define IH_PARSER_AH_DETECTION 0x18000
/* PPP protocol code for IPv4 is configured at index 0 */
#define IH_PARSER_PPP_PROTOCOL_CODE_0_IPV4 0x21
/* PPP protocol code for IPv6 is configured at index 1 */
#define IH_PARSER_PPP_PROTOCOL_CODE_1_IPV6 0x57

#define IH_ETH0_ROUTE_ADDRESS 0x1C
#define IH_ETH1_ROUTE_ADDRESS 0xC
#define IH_ETH2_ROUTE_ADDRESS 0x14
#define IH_ETH3_ROUTE_ADDRESS 0x8
#define IH_ETH4_ROUTE_ADDRESS 0x10
#define IH_GPON_ROUTE_ADDRESS 0
#define IH_DSL_ROUTE_ADDRESS IH_GPON_ROUTE_ADDRESS
#define IH_RUNNER_A_ROUTE_ADDRESS 0x3
#define IH_RUNNER_B_ROUTE_ADDRESS 0x2
#define IH_DA_FILTER_IPTV_IPV4 0
#define IH_DA_FILTER_IPTV_IPV6 1
#define TCP_CTRL_FLAG_RST 0x04
#define TCP_CTRL_FLAG_SYN 0x02
#define TCP_CTRL_FLAG_FIN 0x01
/* size of ingress queue of each one of the EMACs which function as LAN */
#define IH_INGRESS_QUEUE_SIZE_LAN_EMACS 2
/* size of ingress queue of the WAN port */
#define IH_INGRESS_QUEUE_SIZE_WAN 4
/* size of ingress queue of each runner */
#define IH_INGRESS_QUEUE_SIZE_RUNNERS 1
/* priority of ingress queue of each one of the EMACs which function as LAN */
#define IH_INGRESS_QUEUE_PRIORITY_LAN_EMACS 1
/* priority of ingress queue of the WAN port */
#define IH_INGRESS_QUEUE_PRIORITY_WAN 2
/* priority of ingress queue of each runner */
#define IH_INGRESS_QUEUE_PRIORITY_RUNNERS 0
/* weight of ingress queue of each one of the EMACs which function as LAN */
#define IH_INGRESS_QUEUE_WEIGHT_LAN_EMACS 1
/* weight of ingress queue of the WAN port */
#define IH_INGRESS_QUEUE_WEIGHT_WAN 1
/* weight of ingress queue of each runner */
#define IH_INGRESS_QUEUE_WEIGHT_RUNNERS 1
/* congestion threshold of ingress queue of each one of the EMACs which function as LAN */
#define IH_INGRESS_QUEUE_CONGESTION_THRESHOLD_LAN_EMACS 65
/* congestion threshold of ingress queue of the WAN port */
#define IH_INGRESS_QUEUE_CONGESTION_THRESHOLD_WAN_ETH 65
#define IH_INGRESS_QUEUE_CONGESTION_THRESHOLD_WAN_DSL 0x1F /* 31 */
/* congestion threshold of ingress queue of each runner */
#define IH_INGRESS_QUEUE_CONGESTION_THRESHOLD_RUNNERS 65

#define IH_IP_L4_FILTER_USER_DEFINED_0          0
#define IH_IP_L4_FILTER_USER_DEFINED_1          1
#define IH_IP_L4_FILTER_USER_DEFINED_2          2
#define IH_IP_L4_FILTER_USER_DEFINED_3          3

#define IH_L4_FILTER_DEF                        0xff
/* we use one basic class */
#define IH_BASIC_CLASS_INDEX          0
/* IH classifier indices for broadcast and multicast traffic iptv destination */
#define IH_CLASSIFIER_BCAST_IPTV 0
#define IH_CLASSIFIER_IGMP_IPTV 1
#define IH_CLASSIFIER_ICMPV6 2
#define IH_CLASSIFIER_IPTV  (IH_CLASSIFIER_ICMPV6 + 1)

#define MASK_IH_CLASS_KEY_L4 0x3c0
/* IPTV DA filter mask in IH */
#define IPTV_FILTER_MASK_DA 0x3800
#define IPTV_FILTER_MASK_BCAST  0x8000

/* default value for SBPM */
#define SBPM_DEFAULT_THRESHOLD 800
#define SBPM_DEFAULT_HYSTERESIS 0
#define BPM_DEFAULT_HYSTERESIS 64
#define SBPM_BASE_ADDRESS 0
#define SBPM_LIST_SIZE 0x3FF
#define BPM_CPU_NUMBER_OF_BUFFERS 1536
#if defined(DSL_63138)
#if defined(CONFIG_BCM_JUMBO_FRAME)
#define DRV_BBH_DDR_BUFFER_SIZE  DRV_BBH_DDR_BUFFER_SIZE_2_5_KB
#else
#define DRV_BBH_DDR_BUFFER_SIZE  DRV_BBH_DDR_BUFFER_SIZE_2_KB
#endif
#define DRV_BBH_DDR_BPM_MESSAGE_FORMAT DRV_BBH_DDR_BPM_MESSAGE_FORMAT_15_BIT_BN_WIDTH
#define DRV_SPARE_BN_MESSAGE_FORMAT DRV_SPARE_BN_MESSAGE_FORMAT_15_bit_BN_WIDTH
#elif defined(DSL_63148) && defined(CONFIG_BCM_JUMBO_FRAME)
#define DRV_BBH_DDR_BUFFER_SIZE  DRV_BBH_DDR_BUFFER_SIZE_4_KB
#define DRV_BBH_DDR_BPM_MESSAGE_FORMAT  DRV_BBH_DDR_BPM_MESSAGE_FORMAT_14_BIT_BN_WIDTH
#define DRV_SPARE_BN_MESSAGE_FORMAT DRV_SPARE_BN_MESSAGE_FORMAT_14_bit_BN_WIDTH
#else
#define DRV_BBH_DDR_BUFFER_SIZE  DRV_BBH_DDR_BUFFER_SIZE_2_KB
#define DRV_BBH_DDR_BPM_MESSAGE_FORMAT DRV_BBH_DDR_BPM_MESSAGE_FORMAT_14_BIT_BN_WIDTH
#define DRV_SPARE_BN_MESSAGE_FORMAT DRV_SPARE_BN_MESSAGE_FORMAT_14_bit_BN_WIDTH
#endif

/*DSL DEFS*/
/* size of each one of FIFOs 0-7 */
#define BBH_TX_DSL_PD_FIFO_SIZE_0_7 4
/* size of each one of FIFOs 8-15 */
#define BBH_TX_DSL_PD_FIFO_SIZE_8_15 3
#define BBH_TX_DSL_PD_FIFO_SIZE_16_23 3
#define BBH_TX_DSL_PD_FIFO_SIZE_24_31 3
#define BBH_TX_DSL_PD_FIFO_SIZE_32_39 3
#define BBH_TX_DSL_NUMBER_OF_QUEUES_IN_PD_FIFO_GROUP 8
#define BBH_TX_DSL_TOTAL_NUMBER_OF_PDS 128
#define BBH_TX_DSL_PD_FIFO_BASE_0 0
#define BBH_TX_DSL_PD_FIFO_BASE_1 (BBH_TX_DSL_PD_FIFO_BASE_0 + BBH_TX_DSL_PD_FIFO_SIZE_0_7)
#define BBH_TX_DSL_PD_FIFO_BASE_2 (BBH_TX_DSL_PD_FIFO_BASE_1 + BBH_TX_DSL_PD_FIFO_SIZE_0_7)
#define BBH_TX_DSL_PD_FIFO_BASE_3 (BBH_TX_DSL_PD_FIFO_BASE_2 + BBH_TX_DSL_PD_FIFO_SIZE_0_7)
#define BBH_TX_DSL_PD_FIFO_BASE_4 (BBH_TX_DSL_PD_FIFO_BASE_3 + BBH_TX_DSL_PD_FIFO_SIZE_0_7)
#define BBH_TX_DSL_PD_FIFO_BASE_5 (BBH_TX_DSL_PD_FIFO_BASE_4 + BBH_TX_DSL_PD_FIFO_SIZE_0_7)
#define BBH_TX_DSL_PD_FIFO_BASE_6 (BBH_TX_DSL_PD_FIFO_BASE_5 + BBH_TX_DSL_PD_FIFO_SIZE_0_7)
#define BBH_TX_DSL_PD_FIFO_BASE_7 (BBH_TX_DSL_PD_FIFO_BASE_6 + BBH_TX_DSL_PD_FIFO_SIZE_0_7)
#define BBH_TX_DSL_PD_FIFO_BASE_8_15 (BBH_TX_DSL_PD_FIFO_BASE_7 + BBH_TX_DSL_PD_FIFO_SIZE_0_7)
#define BBH_TX_DSL_PD_FIFO_BASE_16_23 \
    (BBH_TX_DSL_PD_FIFO_BASE_8_15 + (BBH_TX_DSL_NUMBER_OF_QUEUES_IN_PD_FIFO_GROUP * BBH_TX_DSL_PD_FIFO_SIZE_8_15))
#define BBH_TX_DSL_PD_FIFO_BASE_24_31 (BBH_TX_DSL_PD_FIFO_BASE_16_23 + \
    (BBH_TX_DSL_NUMBER_OF_QUEUES_IN_PD_FIFO_GROUP * BBH_TX_DSL_PD_FIFO_SIZE_16_23))
#define BBH_TX_DSL_PD_FIFO_BASE_32_39 (BBH_TX_DSL_PD_FIFO_BASE_24_31 + \
    (BBH_TX_DSL_NUMBER_OF_QUEUES_IN_PD_FIFO_GROUP * BBH_TX_DSL_PD_FIFO_SIZE_24_31))

/******************************************************************************/
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    BB_MODULE_DMA,
    BB_MODULE_SDMA,

    BB_MODULE_NUM
}E_BB_MODULE;

typedef enum
{
    DMA_PERIPHERAL_EMAC_0 = 0,
    DMA_PERIPHERAL_EMAC_1 = 1,
    DMA_PERIPHERAL_DSL = 5,

    DMA_NUMBER_OF_PERIPHERALS
}
E_DMA_PERIPHERAL;
/******************************************************************************/
/*                                                                            */
/* Macros definitions                                                         */
/*                                                                            */
/******************************************************************************/
/* sets bit #i of a given number to a given value */
#define SET_BIT_I( number , i , bit_value )   ( ( number ) &= ( ~ ( 1 << i ) )  , ( number ) |= ( bit_value << i ) )
#define MS_BYTE_TO_8_BYTE_RESOLUTION(address)  ((address) >> 3)
#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))

#define BBH_PORT_IS_WAN(_portIndex) (_portIndex == DRV_BBH_EMAC_0 || _portIndex == DRV_BBH_DSL)

/*****************************************************************************/
/*                                                                           */
/* Local Defines                                                             */
/*                                                                           */
/*****************************************************************************/

static S_DPI_CFG *pDpiCfg = NULL;
static uint32_t initDone = 0;

/* route addresses (for both TX & RX) */
static const uint8_t bbh_route_address_dma[DRV_BBH_NUMBER_OF_PORTS]=
{
    0xC, 0xD, 0xE, 0xD, 0xE, 0xF, 0xF
};

static const uint8_t bbh_route_address_bpm[DRV_BBH_NUMBER_OF_PORTS]=
{
    0x14, 0x15, 0x16, 0x15, 0x16, 0x17, 0x17
};

static const uint8_t bbh_route_address_sdma[DRV_BBH_NUMBER_OF_PORTS]=
{
    0x1C, 0x1D, 0x1E, 0x1D, 0x1E, 0x1F, 0x1F
};

static const uint8_t bbh_route_address_sbpm[DRV_BBH_NUMBER_OF_PORTS]=
{
    0x34, 0x35, 0x36, 0x35, 0x36, 0x37, 0x37
};

static const uint8_t bbh_route_address_runner_0[DRV_BBH_NUMBER_OF_PORTS]=
{
    0x0, 0x1, 0x2, 0x1, 0x2, 0x3, 0x3
};

static const uint8_t bbh_route_address_runner_1[DRV_BBH_NUMBER_OF_PORTS]=
{
    0x8, 0x9, 0xA, 0x9, 0xA, 0xB, 0xB
};

static const uint8_t bbh_route_address_ih[DRV_BBH_NUMBER_OF_PORTS]=
{
    0x18, 0x19, 0x1A, 0x11, 0x12, 0x13, 0x13
};

/* same values for DMA & SDMA */
static const uint8_t bbh_dma_and_sdma_read_requests_fifo_base_address[DRV_BBH_NUMBER_OF_PORTS]=
{
    0x0, 0x8, 0x10, 0x18, 0x20, 0x28, 0x28
};

/* IH Classes indexes & configurations */
typedef struct
{
    uint8_t class_index;

    DRV_IH_CLASS_CONFIG class_config;
}
ih_class_cfg;
ih_class_cfg gs_ih_classes[] =
{
    {
        /* LAN bridged eth0 */
        DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH0_INDEX,
        {
            DRV_RDD_IH_LOOKUP_TABLE_MAC_DA_INDEX, /* da_lookup_required */
            DRV_RDD_IH_LOOKUP_TABLE_MAC_SA_INDEX, /* sa_lookup_required */
            DRV_RDD_IH_CLASS_10_CLASS_SEARCH_3,
            DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_4,
            DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_BASED_ON_SEARCH1, /* da_lookup_required */
            DRV_RDD_IH_CLASS_10_DROP_ON_MISS,
            DRV_RDD_IH_CLASS_10_DSCP_TO_PBITS_TABLE_INDEX,
            DRV_RDD_IH_CLASS_10_DIRECT_MODE_DEFAULT,
            DRV_RDD_IH_CLASS_10_DIRECT_MODE_OVERRIDE,
            DRV_RDD_IH_CLASS_10_TARGET_MEMORY_DEFAULT,
            DRV_RDD_IH_CLASS_10_TARGET_MEMORY_OVERRIDE,
            DRV_RDD_IH_CLASS_10_INGRESS_QOS_DEFAULT,
            DRV_RDD_IH_CLASS_10_INGRESS_QOS_OVERRIDE,
            DRV_RDD_IH_CLASS_10_TARGET_RUNNER_DEFAULT,
            DRV_RDD_IH_CLASS_10_TARGET_RUNNER_OVERRIDE_IN_DIRECT_MODE,
            DRV_RDD_IH_CLASS_10_TARGET_RUNNER_FOR_DIRECT_MODE,
            DRV_RDD_IH_CLASS_10_LOAD_BALANCING_ENABLE,
            DRV_RDD_IH_CLASS_10_PREFERENCE_LOAD_BALANCING_ENABLE
        }
    },
    {
        /* LAN bridged eth1 */
        DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH1_INDEX,
        {
            DRV_RDD_IH_LOOKUP_TABLE_MAC_DA_INDEX, /* da_lookup_required */
            DRV_RDD_IH_LOOKUP_TABLE_MAC_SA_INDEX, /* sa_lookup_required */
            DRV_RDD_IH_CLASS_11_CLASS_SEARCH_3,
            DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_4,
            DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_BASED_ON_SEARCH1, /* da_lookup_required */
            DRV_RDD_IH_CLASS_11_DROP_ON_MISS,
            DRV_RDD_IH_CLASS_11_DSCP_TO_PBITS_TABLE_INDEX,
            DRV_RDD_IH_CLASS_11_DIRECT_MODE_DEFAULT,
            DRV_RDD_IH_CLASS_11_DIRECT_MODE_OVERRIDE,
            DRV_RDD_IH_CLASS_11_TARGET_MEMORY_DEFAULT,
            DRV_RDD_IH_CLASS_11_TARGET_MEMORY_OVERRIDE,
            DRV_RDD_IH_CLASS_11_INGRESS_QOS_DEFAULT,
            DRV_RDD_IH_CLASS_11_INGRESS_QOS_OVERRIDE,
            DRV_RDD_IH_CLASS_11_TARGET_RUNNER_DEFAULT,
            DRV_RDD_IH_CLASS_11_TARGET_RUNNER_OVERRIDE_IN_DIRECT_MODE,
            DRV_RDD_IH_CLASS_11_TARGET_RUNNER_FOR_DIRECT_MODE,
            DRV_RDD_IH_CLASS_11_LOAD_BALANCING_ENABLE,
            DRV_RDD_IH_CLASS_11_PREFERENCE_LOAD_BALANCING_ENABLE
        }
    },
};

/* following arrays are initialized in run-time. */
/* DMA related */
static uint8_t bbh_rx_dma_data_fifo_base_address[DRV_BBH_NUMBER_OF_PORTS];
static uint8_t bbh_rx_dma_chunk_descriptor_fifo_base_address[DRV_BBH_NUMBER_OF_PORTS];
static uint8_t bbh_rx_dma_data_and_chunk_descriptor_fifos_size[DRV_BBH_NUMBER_OF_PORTS];
static uint8_t bbh_rx_dma_exclusive_threshold[DRV_BBH_NUMBER_OF_PORTS];
/* SDMA related */
static uint8_t bbh_rx_sdma_data_fifo_base_address[DRV_BBH_NUMBER_OF_PORTS];
static uint8_t bbh_rx_sdma_chunk_descriptor_fifo_base_address[DRV_BBH_NUMBER_OF_PORTS];
static uint8_t bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[DRV_BBH_NUMBER_OF_PORTS];
static uint8_t bbh_rx_sdma_exclusive_threshold[DRV_BBH_NUMBER_OF_PORTS];
static uint16_t bbh_ih_ingress_buffers_bitmask[DRV_BBH_NUMBER_OF_PORTS];

static const BL_LILAC_RDD_EMAC_ID_DTE bbh_to_rdd_emac_map[] =
{
        BL_LILAC_RDD_EMAC_ID_0 ,
        BL_LILAC_RDD_EMAC_ID_1 ,
        BL_LILAC_RDD_EMAC_ID_2 ,
        BL_LILAC_RDD_EMAC_ID_3 ,
        BL_LILAC_RDD_EMAC_ID_4 ,
};

/******************************************************************************/
/* There are 8 IH ingress queues. This enumeration defines, for each ingress  */
/* queue, which physical source port it belongs to.                           */
/******************************************************************************/

typedef enum
{
    IH_INGRESS_QUEUE_0_ETH0 = 0 ,
    IH_INGRESS_QUEUE_1_ETH1 = 1 ,
    IH_INGRESS_QUEUE_5_DSL = 5 ,
    IH_INGRESS_QUEUE_6_RUNNER_A = 6 ,
    IH_INGRESS_QUEUE_7_RUNNER_B = 7 ,

    NUMBER_OF_IH_INGRESS_QUEUES
}
IH_INGRESS_QUEUE_INDEX ;

/* FW binaries */
extern uint32_t firmware_binary_A[];
extern uint32_t firmware_binary_B[];
extern uint32_t firmware_binary_C[];
extern uint32_t firmware_binary_D[];
extern uint16_t firmware_predict_A[];
extern uint16_t firmware_predict_B[];
extern uint16_t firmware_predict_C[];
extern uint16_t firmware_predict_D[];

void f_basic_sbpm_sp_enable(void);
void f_basic_bpm_sp_enable(void);

typedef struct
{
    uint32_t bpm_def_thresh;    
    uint32_t bpm_gbl_thresh;    
} rdp_bpm_cfg_params;


static void f_update_bbh_ih_ingress_buffers_bitmask(DRV_BBH_PORT_INDEX bbh_port_index, uint8_t base_location, uint8_t queue_size)
{
    uint16_t bitmask = 0;
    uint8_t i;

    /* set '1's according to queue_size  */
    for (i= 0; i < queue_size; i++)
    {
        SET_BIT_I(bitmask, i, 1);
    }
    /* do shifting according to xi_base_location */
    bitmask <<= base_location;

    /* update in database */
    bbh_ih_ingress_buffers_bitmask[bbh_port_index] = bitmask;
}

static void fi_dma_configure_memory_allocation ( E_BB_MODULE module_id ,
                                                 E_DMA_PERIPHERAL peripheral_id ,
                                                 uint32_t data_memory_offset_address ,
                                                 uint32_t cd_memory_offset_address ,
                                                 uint32_t number_of_buffers )
{
    DMA_REGS_CONFIG_MALLOC config_malloc ;

    DMA_REGS_CONFIG_MALLOC_READ( module_id , peripheral_id , config_malloc ) ;
    config_malloc.datatoffset = data_memory_offset_address ;
    config_malloc.cdoffset = cd_memory_offset_address ;
    config_malloc.numofbuff = number_of_buffers ;
    DMA_REGS_CONFIG_MALLOC_WRITE( module_id , peripheral_id , config_malloc ) ;
}

static void f_initialize_dma_sdma(void)
{
    DMA_REGS_CONFIG_U_THRESH dma_thresh;

    /* DMA */
    fi_dma_configure_memory_allocation ( BB_MODULE_DMA ,
                                         DMA_PERIPHERAL_EMAC_0 ,
                                         BBH_RX_DMA_FIFOS_SIZE_LAN_BBH * DMA_PERIPHERAL_EMAC_0  ,
                                         BBH_RX_DMA_FIFOS_SIZE_LAN_BBH * DMA_PERIPHERAL_EMAC_0  ,
                                         BBH_RX_DMA_FIFOS_SIZE_LAN_BBH) ;
    /* SDMA */
    fi_dma_configure_memory_allocation ( BB_MODULE_SDMA ,
                                         DMA_PERIPHERAL_EMAC_0 ,
                                         bbh_rx_sdma_data_fifo_base_address [ DMA_PERIPHERAL_EMAC_0 ] ,
                                         bbh_rx_sdma_chunk_descriptor_fifo_base_address [ DMA_PERIPHERAL_EMAC_0 ] ,
                                         bbh_rx_sdma_data_and_chunk_descriptor_fifos_size [ DMA_PERIPHERAL_EMAC_0 ] ) ;

    /* DMA */
    fi_dma_configure_memory_allocation ( BB_MODULE_DMA ,
                                         DMA_PERIPHERAL_EMAC_1 ,
                                         BBH_RX_DMA_FIFOS_SIZE_LAN_BBH * DMA_PERIPHERAL_EMAC_1  ,
                                         BBH_RX_DMA_FIFOS_SIZE_LAN_BBH * DMA_PERIPHERAL_EMAC_1  ,
                                         BBH_RX_DMA_FIFOS_SIZE_LAN_BBH) ;
    /* SDMA */
    fi_dma_configure_memory_allocation ( BB_MODULE_SDMA ,
                                         DMA_PERIPHERAL_EMAC_1 ,
                                         bbh_rx_sdma_data_fifo_base_address [ DMA_PERIPHERAL_EMAC_1 ] ,
                                         bbh_rx_sdma_chunk_descriptor_fifo_base_address [ DMA_PERIPHERAL_EMAC_1 ] ,
                                         bbh_rx_sdma_data_and_chunk_descriptor_fifos_size [ DMA_PERIPHERAL_EMAC_1 ] ) ;

    /* DMA*/
    fi_dma_configure_memory_allocation ( BB_MODULE_DMA ,
                                         DMA_PERIPHERAL_DSL ,
                                         bbh_rx_dma_data_fifo_base_address [ DRV_BBH_DSL ] ,
                                         bbh_rx_dma_chunk_descriptor_fifo_base_address [ DRV_BBH_DSL ] ,
                                         bbh_rx_dma_data_and_chunk_descriptor_fifos_size [ DRV_BBH_DSL ] ) ;
    /* SDMA */
    fi_dma_configure_memory_allocation ( BB_MODULE_SDMA ,
                                         DMA_PERIPHERAL_DSL ,
                                         bbh_rx_sdma_data_fifo_base_address [ DRV_BBH_DSL ] ,
                                         bbh_rx_sdma_chunk_descriptor_fifo_base_address [ DRV_BBH_DSL ] ,
                                         bbh_rx_sdma_data_and_chunk_descriptor_fifos_size [ DRV_BBH_DSL ] ) ;

    dma_thresh.out_of_u = 1;
    dma_thresh.into_u = 5;
    DMA_REGS_CONFIG_U_THRESH_WRITE(BB_MODULE_DMA,DMA_PERIPHERAL_EMAC_0,dma_thresh);
    DMA_REGS_CONFIG_U_THRESH_WRITE(BB_MODULE_DMA,DMA_PERIPHERAL_EMAC_1,dma_thresh);
    DMA_REGS_CONFIG_U_THRESH_WRITE(BB_MODULE_DMA,DMA_PERIPHERAL_DSL,dma_thresh);
}

static E_DPI_RC f_initialize_bbh_dma_sdma_related_arrays(void)
{
    uint8_t dma_base_address = 0;
    uint8_t sdma_base_address = 0;

    /* Ethernet WAN */
    bbh_rx_dma_data_and_chunk_descriptor_fifos_size[DRV_BBH_EMAC_0] = BBH_RX_DMA_FIFOS_SIZE_WAN_DMA;
    bbh_rx_dma_exclusive_threshold[DRV_BBH_EMAC_0] = BBH_RX_DMA_EXCLUSIVE_THRESHOLD_WAN_DMA;
    bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[DRV_BBH_EMAC_0] = BBH_RX_SDMA_FIFOS_SIZE_WAN;
    bbh_rx_sdma_exclusive_threshold[DRV_BBH_EMAC_0] = BBH_RX_SDMA_EXCLUSIVE_THRESHOLD_WAN;

    bbh_rx_dma_data_fifo_base_address[DRV_BBH_EMAC_0] = dma_base_address;
    bbh_rx_dma_chunk_descriptor_fifo_base_address[DRV_BBH_EMAC_0] = dma_base_address;
    bbh_rx_sdma_data_fifo_base_address[DRV_BBH_EMAC_0] = sdma_base_address;
    bbh_rx_sdma_chunk_descriptor_fifo_base_address[DRV_BBH_EMAC_0] = sdma_base_address;

    dma_base_address += bbh_rx_dma_data_and_chunk_descriptor_fifos_size[DRV_BBH_EMAC_0];
    sdma_base_address += bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[DRV_BBH_EMAC_0];

    /* SF2 */
    bbh_rx_dma_data_and_chunk_descriptor_fifos_size[DRV_BBH_EMAC_1] = BBH_RX_DMA_FIFOS_SIZE_LAN_DMA;
    bbh_rx_dma_exclusive_threshold[DRV_BBH_EMAC_1] = BBH_RX_DMA_EXCLUSIVE_THRESHOLD_LAN_DMA;
    bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[DRV_BBH_EMAC_1] = BBH_RX_SDMA_FIFOS_SIZE_LAN;
    bbh_rx_sdma_exclusive_threshold[DRV_BBH_EMAC_1] = BBH_RX_SDMA_EXCLUSIVE_THRESHOLD_LAN;

    bbh_rx_dma_data_fifo_base_address[DRV_BBH_EMAC_1] = dma_base_address;
    bbh_rx_dma_chunk_descriptor_fifo_base_address[DRV_BBH_EMAC_1] = dma_base_address;
    bbh_rx_sdma_data_fifo_base_address[DRV_BBH_EMAC_1] = sdma_base_address;
    bbh_rx_sdma_chunk_descriptor_fifo_base_address[DRV_BBH_EMAC_1] = sdma_base_address;

    dma_base_address += bbh_rx_dma_data_and_chunk_descriptor_fifos_size[DRV_BBH_EMAC_1];
    sdma_base_address += bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[DRV_BBH_EMAC_1];

    /* DSL */
    bbh_rx_dma_data_and_chunk_descriptor_fifos_size[DRV_BBH_DSL] = BBH_RX_DMA_FIFOS_SIZE_WAN_DMA;
    bbh_rx_dma_exclusive_threshold[DRV_BBH_DSL] = BBH_RX_DMA_EXCLUSIVE_THRESHOLD_WAN_DMA;
    bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[DRV_BBH_DSL] = BBH_RX_SDMA_FIFOS_SIZE_WAN;
    bbh_rx_sdma_exclusive_threshold[DRV_BBH_DSL] = BBH_RX_SDMA_EXCLUSIVE_THRESHOLD_WAN;

    bbh_rx_dma_data_fifo_base_address[DRV_BBH_DSL] = dma_base_address;
    bbh_rx_dma_chunk_descriptor_fifo_base_address[DRV_BBH_DSL] = dma_base_address;
    bbh_rx_sdma_data_fifo_base_address[DRV_BBH_DSL] = sdma_base_address;
    bbh_rx_sdma_chunk_descriptor_fifo_base_address[DRV_BBH_DSL] = sdma_base_address;

    dma_base_address += bbh_rx_dma_data_and_chunk_descriptor_fifos_size[DRV_BBH_DSL];
    sdma_base_address += bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[DRV_BBH_DSL];

    /* check that we didn't overrun */
    if (dma_base_address > BBH_RX_DMA_TOTAL_NUMBER_OF_CHUNK_DESCRIPTORS ||
        sdma_base_address > BBH_RX_SDMA_TOTAL_NUMBER_OF_CHUNK_DESCRIPTORS)
    {
        __print("%s:(%d) error",__FUNCTION__,__LINE__);
        return DPI_RC_ERROR;
    }
    return DPI_RC_OK;
}

static uint16_t f_get_bbh_rx_pd_fifo_base_address_normal_queue_in_8_byte(DRV_BBH_PORT_INDEX port_index)
{
    switch (port_index)
    {
    case DRV_BBH_EMAC_0:
        return MS_BYTE_TO_8_BYTE_RESOLUTION(ETH0_RX_DESCRIPTORS_ADDRESS);

    case DRV_BBH_EMAC_1:
        return MS_BYTE_TO_8_BYTE_RESOLUTION(ETH1_RX_DESCRIPTORS_ADDRESS);

    case DRV_BBH_DSL:
        return MS_BYTE_TO_8_BYTE_RESOLUTION(GPON_RX_NORMAL_DESCRIPTORS_ADDRESS);

    default:
        return 0;
    }
}
static uint16_t f_get_bbh_rx_pd_fifo_base_address_direct_queue_in_8_byte(DRV_BBH_PORT_INDEX port_index)
{
    switch (port_index)
    {
    case DRV_BBH_EMAC_0:
        return MS_BYTE_TO_8_BYTE_RESOLUTION(ETH0_RX_DIRECT_DESCRIPTORS_ADDRESS);

    case DRV_BBH_EMAC_1:
        return MS_BYTE_TO_8_BYTE_RESOLUTION(ETH1_RX_DIRECT_DESCRIPTORS_ADDRESS);

    case DRV_BBH_DSL:
        return MS_BYTE_TO_8_BYTE_RESOLUTION(GPON_RX_DIRECT_DESCRIPTORS_ADDRESS);

    default:
        return 0;
    }
}
static uint8_t f_bbh_rx_runner_task_normal_queue(DRV_IH_RUNNER_CLUSTER runner_cluster, DRV_BBH_PORT_INDEX port_index)
{
    uint8_t isWan = BBH_PORT_IS_WAN(port_index);

    if (runner_cluster == DRV_IH_RUNNER_CLUSTER_A)
    {
        if (isWan)
        {
            return (port_index == DRV_BBH_DSL) ? WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER
                                               : WAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER;
        }
        /* the port is LAN */
        else
        {
            /* runner A doesn't handle the normal queue for LAN */
            return 0;
        }
    }
    /* runner B */
    else
    {
        if (isWan)
        {
            /* runner B doesn't handle WAN */
            return 0;
        }
        /* the port is LAN */
        else
        {
            switch (port_index)
            {
            case DRV_BBH_EMAC_0:
                /* EMAC 0 is not supported as a LAN */
                return 0;

            case DRV_BBH_EMAC_1:
#if defined(__UBOOT__)
                return LAN1_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER;
#else
                return LAN_DISPATCH_THREAD_NUMBER;
#endif

                /* we are not supposed to get here */
            default:
                return 0;
            }
        }
    }
}

static uint8_t f_bbh_rx_runner_task_direct_queue(DRV_IH_RUNNER_CLUSTER runner_cluster, DRV_BBH_PORT_INDEX portIndex)
{
    if ( BBH_PORT_IS_WAN( portIndex ) )
    {
        return WAN_DIRECT_THREAD_NUMBER;
    }

    /* the port is LAN */
    if (runner_cluster == DRV_IH_RUNNER_CLUSTER_A)
    {
        switch (portIndex)
        {
        case DRV_BBH_EMAC_0:
             return ETH0_RX_DIRECT_RUNNER_A_TASK_NUMBER;

        case DRV_BBH_EMAC_1:
             return ETH1_RX_DIRECT_RUNNER_A_TASK_NUMBER;

             /* we are not supposed to get here */
         default:
             return 0;
         }
    }/* runner B */
    else
    {
        switch (portIndex)
         {
        case DRV_BBH_EMAC_0:
             return ETH0_RX_DIRECT_RUNNER_B_TASK_NUMBER;

        case DRV_BBH_EMAC_1:
             return ETH1_RX_DIRECT_RUNNER_B_TASK_NUMBER;

             /* we are not supposed to get here */
         default:
             return 0;
         }
    }
}

static void f_initialize_bbh_of_emac_port(DRV_BBH_PORT_INDEX port_index)
{
	uint16_t mdu_mode_read_pointer_address_in_byte;
    uint32_t mdu_mode_read_pointer_address_in_byte_uint32;
    DRV_BBH_TX_CONFIGURATION bbh_tx_configuration ;
    DRV_BBH_RX_CONFIGURATION bbh_rx_configuration ;
    DRV_BBH_PER_FLOW_CONFIGURATION per_flow_configuration ;

    /*** BBH TX ***/
    bbh_tx_configuration.dma_route_address = bbh_route_address_dma[port_index];
    bbh_tx_configuration.bpm_route_address = bbh_route_address_bpm[port_index];
    bbh_tx_configuration.sdma_route_address = bbh_route_address_sdma[port_index];
    bbh_tx_configuration.sbpm_route_address = bbh_route_address_sbpm[port_index];
    /* runner 0 is the one which handles TX  except for wan mode emac4*/
    if ( port_index == DRV_BBH_EMAC_0 )
    {
        bbh_tx_configuration.runner_route_address = bbh_route_address_runner_1[port_index];
    }
    else
    {
        bbh_tx_configuration.runner_route_address = bbh_route_address_runner_0[port_index];
    }

    bbh_tx_configuration.ddr_buffer_size = DRV_BBH_DDR_BUFFER_SIZE;
    bbh_tx_configuration.ddr_bpm_message_format = DRV_BBH_DDR_BPM_MESSAGE_FORMAT;
    bbh_tx_configuration.payload_offset_resolution = DRV_BBH_PAYLOAD_OFFSET_RESOLUTION_2_B;
    bbh_tx_configuration.multicast_headers_base_address_in_byte = (uint32_t) pDpiCfg->runner_mc_base_addr_phys;

    if ( port_index == DRV_BBH_EMAC_0 )
    {
        bbh_tx_configuration.multicast_header_size = BBH_MULTICAST_HEADER_SIZE_FOR_LAN_PORT;
        /* this is the task also in Gbe case */
        bbh_tx_configuration.task_0 = WAN1_TX_THREAD_NUMBER;
        bbh_tx_configuration.skb_address = MS_BYTE_TO_8_BYTE_RESOLUTION(ETHWAN_ABSOLUTE_TX_BBH_COUNTER_ADDRESS);
    }
    else
        /*we will reach here in case the BBH port in lan ports 0 - 4*/
    {
        bbh_tx_configuration.multicast_header_size = BBH_MULTICAST_HEADER_SIZE_FOR_LAN_PORT;

        bbh_tx_configuration.skb_address = MS_BYTE_TO_8_BYTE_RESOLUTION(EMAC_ABSOLUTE_TX_BBH_COUNTER_ADDRESS) + bbh_to_rdd_emac_map[port_index];

        bbh_tx_configuration.task_0 = ETH_TX_THREAD_NUMBER;
    }

    /* other task numbers are irrelevant (relevant for DSL only) */
    bbh_tx_configuration.task_1 = 0;
    bbh_tx_configuration.task_2 = 0;
    bbh_tx_configuration.task_3 = 0;
    bbh_tx_configuration.task_4 = 0;
    bbh_tx_configuration.task_5 = 0;
    bbh_tx_configuration.task_6 = 0;
    bbh_tx_configuration.task_7 = 0;
    bbh_tx_configuration.task_8_39 = 0;


    if ( port_index == DRV_BBH_EMAC_0 )
    {
        bbh_tx_configuration.mdu_mode_enable = 0;
        /* irrelevant in this case */
        bbh_tx_configuration.mdu_mode_read_pointer_address_in_8_byte = 0;
    }
    else
    {
        bbh_tx_configuration.mdu_mode_enable = 1;

        rdd_mdu_mode_pointer_get(bbh_to_rdd_emac_map[port_index], &mdu_mode_read_pointer_address_in_byte);

        mdu_mode_read_pointer_address_in_byte_uint32 = mdu_mode_read_pointer_address_in_byte;

        /* after division, this will be back a 16 bit number */
        bbh_tx_configuration.mdu_mode_read_pointer_address_in_8_byte = (uint16_t)MS_BYTE_TO_8_BYTE_RESOLUTION(mdu_mode_read_pointer_address_in_byte_uint32);
    }

    /* For Ethernet port working in MDU mode, PD FIFO size should be configured to 4 (and not 8). */
    bbh_tx_configuration.pd_fifo_size_0 =  (bbh_tx_configuration.mdu_mode_enable == 1) ? BBH_TX_EMAC_PD_FIFO_SIZE_MDU_MODE_ENABLED : BBH_TX_EMAC_PD_FIFO_SIZE_MDU_MODE_DISABLED;

    /* other FIFOs are irrelevant (relevant for DSL only) */
    bbh_tx_configuration.pd_fifo_size_1 = DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE;
    bbh_tx_configuration.pd_fifo_size_2 = DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE;
    bbh_tx_configuration.pd_fifo_size_3 = DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE;
    bbh_tx_configuration.pd_fifo_size_4 = DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE;
    bbh_tx_configuration.pd_fifo_size_5 = DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE;
    bbh_tx_configuration.pd_fifo_size_6 = DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE;
    bbh_tx_configuration.pd_fifo_size_7 = DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE;
    bbh_tx_configuration.pd_fifo_size_8_15 = DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE;
    bbh_tx_configuration.pd_fifo_size_16_23 = DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE;
    bbh_tx_configuration.pd_fifo_size_24_31 = DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE;
    bbh_tx_configuration.pd_fifo_size_32_39 = DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE;

    bbh_tx_configuration.pd_fifo_base_0 = 0;

    /* other FIFOs are irrelevant (relevant for DSL only) */
    bbh_tx_configuration.pd_fifo_base_1 = 0;
    bbh_tx_configuration.pd_fifo_base_2 = 0;
    bbh_tx_configuration.pd_fifo_base_3 = 0;
    bbh_tx_configuration.pd_fifo_base_4 = 0;
    bbh_tx_configuration.pd_fifo_base_5 = 0;
    bbh_tx_configuration.pd_fifo_base_6 = 0;
    bbh_tx_configuration.pd_fifo_base_7 = 0;
    bbh_tx_configuration.pd_fifo_base_8_15 = 0;
    bbh_tx_configuration.pd_fifo_base_16_23 = 0;
    bbh_tx_configuration.pd_fifo_base_24_31 = 0;
    bbh_tx_configuration.pd_fifo_base_32_39 = 0;

    /* pd_prefetch_byte_threshold feature is irrelevant in EMAC (since there is only one FIFO) */
    bbh_tx_configuration.pd_prefetch_byte_threshold_enable = 0;
    bbh_tx_configuration.pd_prefetch_byte_threshold_0_in_32_byte = 0;
    bbh_tx_configuration.pd_prefetch_byte_threshold_1_in_32_byte = 0;
    bbh_tx_configuration.pd_prefetch_byte_threshold_2_in_32_byte = 0;
    bbh_tx_configuration.pd_prefetch_byte_threshold_3_in_32_byte = 0;
    bbh_tx_configuration.pd_prefetch_byte_threshold_4_in_32_byte = 0;
    bbh_tx_configuration.pd_prefetch_byte_threshold_5_in_32_byte = 0;
    bbh_tx_configuration.pd_prefetch_byte_threshold_6_in_32_byte = 0;
    bbh_tx_configuration.pd_prefetch_byte_threshold_7_in_32_byte = 0;
    bbh_tx_configuration.pd_prefetch_byte_threshold_8_39_in_32_byte = 0;

    bbh_tx_configuration.dma_read_requests_fifo_base_address = bbh_dma_and_sdma_read_requests_fifo_base_address[port_index];
    bbh_tx_configuration.dma_read_requests_maximal_number = BBH_TX_CONFIGURATIONS_DMACFG_TX_MAXREQ_MAX_VALUE_VALUE_RESET_VALUE;
    bbh_tx_configuration.sdma_read_requests_fifo_base_address = bbh_dma_and_sdma_read_requests_fifo_base_address[port_index];
    bbh_tx_configuration.sdma_read_requests_maximal_number = BBH_TX_CONFIGURATIONS_SDMACFG_TX_MAXREQ_MAX_VALUE_VALUE_RESET_VALUE;

    /* irrelevant in EMAC case */
    bbh_tx_configuration.tcont_address_in_8_byte = 0;

    bbh_tx_configuration.ddr_tm_base_address = (uint32_t)pDpiCfg->runner_tm_base_addr_phys;

    bbh_tx_configuration.emac_1588_enable = 0;

    fi_bl_drv_bbh_tx_set_configuration(port_index, &bbh_tx_configuration);

    /*** BBH RX ***/
    /* bbh_rx_set_configuration */
    bbh_rx_configuration.dma_route_address = bbh_route_address_dma[port_index];
    bbh_rx_configuration.bpm_route_address = bbh_route_address_bpm[port_index];
    bbh_rx_configuration.sdma_route_address = bbh_route_address_sdma[port_index];
    bbh_rx_configuration.sbpm_route_address = bbh_route_address_sbpm[port_index];
    bbh_rx_configuration.runner_0_route_address = bbh_route_address_runner_0[port_index];
    bbh_rx_configuration.runner_1_route_address = bbh_route_address_runner_1[port_index];
    bbh_rx_configuration.ih_route_address = bbh_route_address_ih[port_index];

    bbh_rx_configuration.ddr_buffer_size = DRV_BBH_DDR_BUFFER_SIZE;
    bbh_rx_configuration.ddr_bpm_message_format = DRV_BBH_DDR_BPM_MESSAGE_FORMAT;
    bbh_rx_configuration.ddr_tm_base_address = (uint32_t)pDpiCfg->runner_tm_base_addr_phys;
    bbh_rx_configuration.pd_fifo_base_address_normal_queue_in_8_byte = f_get_bbh_rx_pd_fifo_base_address_normal_queue_in_8_byte(port_index);
    bbh_rx_configuration.pd_fifo_base_address_direct_queue_in_8_byte = f_get_bbh_rx_pd_fifo_base_address_direct_queue_in_8_byte(port_index);
    bbh_rx_configuration.pd_fifo_size_normal_queue = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    bbh_rx_configuration.pd_fifo_size_direct_queue = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    bbh_rx_configuration.runner_0_task_normal_queue = f_bbh_rx_runner_task_normal_queue (DRV_IH_RUNNER_CLUSTER_A, port_index);
    bbh_rx_configuration.runner_0_task_direct_queue = f_bbh_rx_runner_task_direct_queue (DRV_IH_RUNNER_CLUSTER_A, port_index);
    bbh_rx_configuration.runner_1_task_normal_queue = f_bbh_rx_runner_task_normal_queue (DRV_IH_RUNNER_CLUSTER_B, port_index);
    bbh_rx_configuration.runner_1_task_direct_queue = f_bbh_rx_runner_task_direct_queue (DRV_IH_RUNNER_CLUSTER_B, port_index);
    bbh_rx_configuration.dma_data_fifo_base_address = BBH_RX_DMA_FIFOS_SIZE_LAN_BBH * port_index;
    bbh_rx_configuration.dma_chunk_descriptor_fifo_base_address = BBH_RX_DMA_FIFOS_SIZE_LAN_BBH * port_index;
    bbh_rx_configuration.sdma_data_fifo_base_address = bbh_rx_sdma_data_fifo_base_address[port_index];
    bbh_rx_configuration.sdma_chunk_descriptor_fifo_base_address = bbh_rx_sdma_chunk_descriptor_fifo_base_address[port_index ];
    bbh_rx_configuration.dma_data_and_chunk_descriptor_fifos_size = BBH_RX_DMA_FIFOS_SIZE_LAN_BBH;
    bbh_rx_configuration.dma_exclusive_threshold = BBH_RX_DMA_EXCLUSIVE_THRESHOLD_LAN_BBH;
    bbh_rx_configuration.sdma_data_and_chunk_descriptor_fifos_size = bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[port_index ];
    bbh_rx_configuration.sdma_exclusive_threshold = bbh_rx_sdma_exclusive_threshold[port_index];


    bbh_rx_configuration.minimum_packet_size_0 = MIN_ETH_PKT_SIZE;


    /* minimum_packet_size 1-3 are not in use */
    bbh_rx_configuration.minimum_packet_size_1 = MIN_ETH_PKT_SIZE;
    bbh_rx_configuration.minimum_packet_size_2 = MIN_ETH_PKT_SIZE;
    bbh_rx_configuration.minimum_packet_size_3 = MIN_ETH_PKT_SIZE;


    bbh_rx_configuration.maximum_packet_size_0 = pDpiCfg->mtu_size;


    /* maximum_packet_size 1-3 are not in use */
    bbh_rx_configuration.maximum_packet_size_1 = pDpiCfg->mtu_size;
    bbh_rx_configuration.maximum_packet_size_2 = pDpiCfg->mtu_size;
    bbh_rx_configuration.maximum_packet_size_3 = pDpiCfg->mtu_size;

    bbh_rx_configuration.ih_ingress_buffers_bitmask = bbh_ih_ingress_buffers_bitmask[port_index];
    bbh_rx_configuration.packet_header_offset = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    bbh_rx_configuration.reassembly_offset_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION(pDpiCfg->headroom_size);

    /* By default, the triggers for FC will be disabled and the triggers for drop enabled.
       If the user configures flow control for the port, the triggers for drop will be
       disabled and triggers for FC (including Runner request) will be enabled */
    bbh_rx_configuration.flow_control_triggers_bitmask = 0;
    bbh_rx_configuration.drop_triggers_bitmask = DRV_BBH_RX_DROP_TRIGGER_BPM_IS_IN_EXCLUSIVE_STATE | DRV_BBH_RX_DROP_TRIGGER_SBPM_IS_IN_EXCLUSIVE_STATE;

    /* following configuration is irrelevant in EMAC case */
    bbh_rx_configuration.flows_32_255_group_divider = BBH_RX_FLOWS_32_255_GROUP_DIVIDER;
    bbh_rx_configuration.ploam_default_ih_class = 0;
    bbh_rx_configuration.ploam_ih_class_override = 0;

    fi_bl_drv_bbh_rx_set_configuration(port_index, &bbh_rx_configuration);

    /* bbh_rx_set_per_flow_configuration */
    per_flow_configuration.minimum_packet_size_selection = BBH_RX_ETH_MIN_PKT_SIZE_SELECTION_INDEX;
    per_flow_configuration.maximum_packet_size_selection = BBH_RX_MAX_PKT_SIZE_SELECTION_INDEX;

    if ( port_index == DRV_BBH_EMAC_0 )
    {
        per_flow_configuration.default_ih_class = DRV_RDD_IH_CLASS_WAN_BRIDGED_LOW_INDEX;
        per_flow_configuration.ih_class_override = 1;
    }
    else
    {
        per_flow_configuration.ih_class_override = 0;

        switch (port_index)
        {
        case DRV_BBH_EMAC_0:
            per_flow_configuration.default_ih_class = DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH0_INDEX;
            break;
        case DRV_BBH_EMAC_1:
            per_flow_configuration.default_ih_class = DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH1_INDEX;
            break;
        default:
            return;
            break;
        }
    }

    /* in EMAC, only flow 0 is relevant */
    fi_bl_drv_bbh_rx_set_per_flow_configuration(port_index, 0, &per_flow_configuration);

    return;
}
/* this function configures all 8 ingress queues in IH */
static int fi_ih_configure_ingress_queues(void)
{
    int i;
    DRV_IH_INGRESS_QUEUE_CONFIG ingress_queue_config;
    uint8_t base_location = 0;

    /* queues of EMAC 0 */
    ingress_queue_config.base_location = base_location;
    ingress_queue_config.size = IH_INGRESS_QUEUE_SIZE_WAN;
    ingress_queue_config.priority = IH_INGRESS_QUEUE_PRIORITY_WAN;
    ingress_queue_config.weight = IH_INGRESS_QUEUE_WEIGHT_WAN;
    ingress_queue_config.congestion_threshold = IH_INGRESS_QUEUE_CONGESTION_THRESHOLD_WAN_ETH;

    fi_bl_drv_ih_configure_ingress_queue(IH_INGRESS_QUEUE_0_ETH0, &ingress_queue_config);

    /* update the correspoding bitmask in database, to be configured in BBH */
    f_update_bbh_ih_ingress_buffers_bitmask(DRV_BBH_EMAC_0, ingress_queue_config.base_location, ingress_queue_config.size);

    base_location += ingress_queue_config.size;

    /* queues of EMAC 1 */
    ingress_queue_config.base_location = base_location;
    ingress_queue_config.size = IH_INGRESS_QUEUE_SIZE_LAN_EMACS;
    ingress_queue_config.priority = IH_INGRESS_QUEUE_PRIORITY_LAN_EMACS;
    ingress_queue_config.weight = IH_INGRESS_QUEUE_WEIGHT_LAN_EMACS;
    ingress_queue_config.congestion_threshold = IH_INGRESS_QUEUE_CONGESTION_THRESHOLD_LAN_EMACS;

    fi_bl_drv_ih_configure_ingress_queue(IH_INGRESS_QUEUE_1_ETH1, &ingress_queue_config);

    /* update the correspoding bitmask in database, to be configured in BBH */
    f_update_bbh_ih_ingress_buffers_bitmask(DRV_BBH_EMAC_1, ingress_queue_config.base_location, ingress_queue_config.size);

    base_location += ingress_queue_config.size;

    /* queues of DSL */
    ingress_queue_config.base_location = base_location;
    ingress_queue_config.size = IH_INGRESS_QUEUE_SIZE_WAN;
    ingress_queue_config.priority = IH_INGRESS_QUEUE_PRIORITY_WAN;
    ingress_queue_config.weight = IH_INGRESS_QUEUE_WEIGHT_WAN;
    ingress_queue_config.congestion_threshold = IH_INGRESS_QUEUE_CONGESTION_THRESHOLD_WAN_DSL;

    fi_bl_drv_ih_configure_ingress_queue(IH_INGRESS_QUEUE_5_DSL, &ingress_queue_config);

    /* update the correspoding bitmask in database, to be configured in BBH */
    f_update_bbh_ih_ingress_buffers_bitmask(DRV_BBH_DSL, ingress_queue_config.base_location, ingress_queue_config.size);

    base_location += ingress_queue_config.size;

    /* queues of runners */
    for (i = IH_INGRESS_QUEUE_6_RUNNER_A; i <= IH_INGRESS_QUEUE_6_RUNNER_A; i++)
    {
        ingress_queue_config.base_location = base_location;
        ingress_queue_config.size = IH_INGRESS_QUEUE_SIZE_RUNNERS;
        ingress_queue_config.priority = IH_INGRESS_QUEUE_PRIORITY_RUNNERS;
        ingress_queue_config.weight = IH_INGRESS_QUEUE_WEIGHT_RUNNERS;
        ingress_queue_config.congestion_threshold = IH_INGRESS_QUEUE_CONGESTION_THRESHOLD_RUNNERS;

        fi_bl_drv_ih_configure_ingress_queue (i, &ingress_queue_config);

        base_location += ingress_queue_config.size;
    }

    /* check that we didn't overrun */
    if (base_location > DRV_IH_INGRESS_QUEUES_ARRAY_SIZE)
    {
        /* sum of sizes exceeded the total array size */
        return DPI_RC_ERROR;
    }
    return DPI_RC_OK;
}

/* this function configures the IH classes which are in use */
static void f_ih_configure_lookup_tables(void)
{
    DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG lookup_table_60_bit_key_config ;

    /*** table 1: MAC DA ***/
    lookup_table_60_bit_key_config.table_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_LOOKUP_TABLE_1_BASE_ADDRESS);
    lookup_table_60_bit_key_config.table_size = DRV_RDD_IH_LOOKUP_TABLE_1_SIZE;
    lookup_table_60_bit_key_config.maximal_search_depth = DRV_RDD_IH_LOOKUP_TABLE_1_SEARCH_DEPTH;
    lookup_table_60_bit_key_config.hash_type = DRV_RDD_IH_LOOKUP_TABLE_1_HASH_TYPE;
    lookup_table_60_bit_key_config.sa_search_enable = DRV_RDD_IH_LOOKUP_TABLE_1_SA_ENABLE;
    lookup_table_60_bit_key_config.aging_enable = DRV_RDD_IH_LOOKUP_TABLE_1_AGING_ENABLE;
    lookup_table_60_bit_key_config.cam_enable = DRV_RDD_IH_LOOKUP_TABLE_1_CAM_ENABLE;
    lookup_table_60_bit_key_config.cam_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_LOOKUP_TABLE_1_CAM_BASE_ADDRESS);
    lookup_table_60_bit_key_config.context_table_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_CONTEXT_TABLE_1_BASE_ADDRESS);
    lookup_table_60_bit_key_config.context_table_entry_size = DRV_RDD_IH_CONTEXT_TABLE_1_ENTRY_SIZE;
    lookup_table_60_bit_key_config.cam_context_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_CONTEXT_TABLE_1_CAM_BASE_ADDRESS);
    lookup_table_60_bit_key_config.part_0_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_1_DST_MAC_KEY_PART_0_START_OFFSET;
    lookup_table_60_bit_key_config.part_0_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_1_DST_MAC_KEY_PART_0_SHIFT;
    lookup_table_60_bit_key_config.part_1_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_1_DST_MAC_KEY_PART_1_START_OFFSET;
    lookup_table_60_bit_key_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_1_DST_MAC_KEY_PART_1_SHIFT;
    lookup_table_60_bit_key_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_1_DST_MAC_KEY_KEY_EXTENSION;
    lookup_table_60_bit_key_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_1_DST_MAC_KEY_PART_0_MASK_LOW;
    lookup_table_60_bit_key_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_1_DST_MAC_KEY_PART_0_MASK_HIGH;
    lookup_table_60_bit_key_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_1_DST_MAC_KEY_PART_1_MASK_LOW;
    lookup_table_60_bit_key_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_1_DST_MAC_KEY_PART_1_MASK_HIGH;
    lookup_table_60_bit_key_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_1_DST_MAC_KEY_GLOBAL_MASK;

    fi_bl_drv_ih_configure_lut_60_bit_key(DRV_RDD_IH_LOOKUP_TABLE_MAC_DA_INDEX, &lookup_table_60_bit_key_config);

    //TODO:move that function to RDPA_System
    /*** table 2: IPTV ***/
    //rdpa_ih_cfg_iptv_lookup_table(iptv_lookup_method_mac);

    /*** table 3: DS ingress classification ***/
    lookup_table_60_bit_key_config.table_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_LOOKUP_TABLE_3_BASE_ADDRESS);
    lookup_table_60_bit_key_config.table_size = DRV_RDD_IH_LOOKUP_TABLE_3_SIZE;
    lookup_table_60_bit_key_config.maximal_search_depth = DRV_RDD_IH_LOOKUP_TABLE_3_SEARCH_DEPTH;
    lookup_table_60_bit_key_config.hash_type = DRV_RDD_IH_LOOKUP_TABLE_3_HASH_TYPE;
    lookup_table_60_bit_key_config.sa_search_enable = DRV_RDD_IH_LOOKUP_TABLE_3_SA_ENABLE;
    lookup_table_60_bit_key_config.aging_enable = DRV_RDD_IH_LOOKUP_TABLE_3_AGING_ENABLE;
    lookup_table_60_bit_key_config.cam_enable = DRV_RDD_IH_LOOKUP_TABLE_3_CAM_ENABLE;
    lookup_table_60_bit_key_config.cam_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_LOOKUP_TABLE_3_CAM_BASE_ADDRESS);
    lookup_table_60_bit_key_config.context_table_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_CONTEXT_TABLE_3_BASE_ADDRESS);
    lookup_table_60_bit_key_config.context_table_entry_size = DRV_RDD_IH_CONTEXT_TABLE_3_ENTRY_SIZE;
    lookup_table_60_bit_key_config.cam_context_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_CONTEXT_TABLE_3_CAM_BASE_ADDRESS);
    lookup_table_60_bit_key_config.part_0_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_0_START_OFFSET;
    lookup_table_60_bit_key_config.part_0_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_0_SHIFT;
    lookup_table_60_bit_key_config.part_1_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_1_START_OFFSET;
    lookup_table_60_bit_key_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_1_SHIFT;
    lookup_table_60_bit_key_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_KEY_EXTENSION;
    lookup_table_60_bit_key_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_0_MASK_LOW;
    lookup_table_60_bit_key_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_0_MASK_HIGH;
    lookup_table_60_bit_key_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_1_MASK_LOW;
    lookup_table_60_bit_key_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_PART_1_MASK_HIGH;
    lookup_table_60_bit_key_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_3_VID_KEY_GLOBAL_MASK;

    fi_bl_drv_ih_configure_lut_60_bit_key(DRV_RDD_IH_LOOKUP_TABLE_DS_INGRESS_CLASSIFICATION_INDEX, &lookup_table_60_bit_key_config);

    /*** table 4: US ingress classification 1 per LAN port ***/
    lookup_table_60_bit_key_config.table_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_LOOKUP_TABLE_4_BASE_ADDRESS);
    lookup_table_60_bit_key_config.table_size = DRV_RDD_IH_LOOKUP_TABLE_4_SIZE;
    lookup_table_60_bit_key_config.maximal_search_depth = DRV_RDD_IH_LOOKUP_TABLE_4_SEARCH_DEPTH;
    lookup_table_60_bit_key_config.hash_type = DRV_RDD_IH_LOOKUP_TABLE_4_HASH_TYPE;
    lookup_table_60_bit_key_config.sa_search_enable = DRV_RDD_IH_LOOKUP_TABLE_4_SA_ENABLE;
    lookup_table_60_bit_key_config.aging_enable = DRV_RDD_IH_LOOKUP_TABLE_4_AGING_ENABLE;
    lookup_table_60_bit_key_config.cam_enable = DRV_RDD_IH_LOOKUP_TABLE_4_CAM_ENABLE;
    lookup_table_60_bit_key_config.cam_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_LOOKUP_TABLE_4_CAM_BASE_ADDRESS);
    lookup_table_60_bit_key_config.context_table_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_CONTEXT_TABLE_4_BASE_ADDRESS);
    lookup_table_60_bit_key_config.context_table_entry_size = DRV_RDD_IH_CONTEXT_TABLE_4_ENTRY_SIZE;
    lookup_table_60_bit_key_config.cam_context_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_CONTEXT_TABLE_4_CAM_BASE_ADDRESS);
    lookup_table_60_bit_key_config.part_0_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_4_VID_SRC_PORT_KEY_PART_0_START_OFFSET;
    lookup_table_60_bit_key_config.part_0_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_VID_SRC_PORT_KEY_PART_0_SHIFT;
    lookup_table_60_bit_key_config.part_1_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_4_VID_SRC_PORT_KEY_PART_1_START_OFFSET;
    lookup_table_60_bit_key_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_VID_SRC_PORT_KEY_PART_1_SHIFT;
    lookup_table_60_bit_key_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_4_VID_SRC_PORT_KEY_KEY_EXTENSION;
    lookup_table_60_bit_key_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_4_VID_SRC_PORT_KEY_PART_0_MASK_LOW;
    lookup_table_60_bit_key_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_4_VID_SRC_PORT_KEY_PART_0_MASK_HIGH;
    lookup_table_60_bit_key_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_4_VID_SRC_PORT_KEY_PART_1_MASK_LOW;
    lookup_table_60_bit_key_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_4_VID_SRC_PORT_KEY_PART_1_MASK_HIGH;
    lookup_table_60_bit_key_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_4_VID_SRC_PORT_KEY_GLOBAL_MASK;

    fi_bl_drv_ih_configure_lut_60_bit_key(DRV_RDD_IH_LOOKUP_TABLE_US_INGRESS_CLASSIFICATION_INDEX, &lookup_table_60_bit_key_config);

    /*** table 6: IPTV source IP ***/
    lookup_table_60_bit_key_config.table_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_LOOKUP_TABLE_6_BASE_ADDRESS);
    lookup_table_60_bit_key_config.table_size = DRV_RDD_IH_LOOKUP_TABLE_6_SIZE;
    lookup_table_60_bit_key_config.maximal_search_depth = DRV_RDD_IH_LOOKUP_TABLE_6_SEARCH_DEPTH;
    lookup_table_60_bit_key_config.hash_type = DRV_RDD_IH_LOOKUP_TABLE_6_HASH_TYPE;
    lookup_table_60_bit_key_config.sa_search_enable = DRV_RDD_IH_LOOKUP_TABLE_6_SA_ENABLE;
    lookup_table_60_bit_key_config.aging_enable = DRV_RDD_IH_LOOKUP_TABLE_6_AGING_ENABLE;
    lookup_table_60_bit_key_config.cam_enable = DRV_RDD_IH_LOOKUP_TABLE_6_CAM_ENABLE;
    lookup_table_60_bit_key_config.cam_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_LOOKUP_TABLE_6_CAM_BASE_ADDRESS);
    lookup_table_60_bit_key_config.context_table_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_CONTEXT_TABLE_6_BASE_ADDRESS);
    lookup_table_60_bit_key_config.context_table_entry_size = DRV_RDD_IH_CONTEXT_TABLE_6_ENTRY_SIZE;
    lookup_table_60_bit_key_config.cam_context_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_CONTEXT_TABLE_6_CAM_BASE_ADDRESS);
    lookup_table_60_bit_key_config.part_0_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_6_KEY_PART_0_START_OFFSET;
    lookup_table_60_bit_key_config.part_0_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_6_KEY_PART_0_SHIFT;
    lookup_table_60_bit_key_config.part_1_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_6_KEY_PART_1_START_OFFSET;
    lookup_table_60_bit_key_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_6_KEY_PART_1_SHIFT;
    lookup_table_60_bit_key_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_6_KEY_KEY_EXTENSION;
    lookup_table_60_bit_key_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_6_KEY_PART_0_MASK_LOW;
    lookup_table_60_bit_key_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_6_KEY_PART_0_MASK_HIGH;
    lookup_table_60_bit_key_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_6_KEY_PART_1_MASK_LOW;
    lookup_table_60_bit_key_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_6_KEY_PART_1_MASK_HIGH;
    lookup_table_60_bit_key_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_6_KEY_GLOBAL_MASK;

    fi_bl_drv_ih_configure_lut_60_bit_key(DRV_RDD_IH_LOOKUP_TABLE_IPTV_SRC_IP_INDEX, &lookup_table_60_bit_key_config);

    /*** table 9: MAC SA ***/
    lookup_table_60_bit_key_config.table_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_LOOKUP_TABLE_9_BASE_ADDRESS);
    lookup_table_60_bit_key_config.table_size = DRV_RDD_IH_LOOKUP_TABLE_9_SIZE;
    lookup_table_60_bit_key_config.maximal_search_depth = DRV_RDD_IH_LOOKUP_TABLE_9_SEARCH_DEPTH;
    lookup_table_60_bit_key_config.hash_type = DRV_RDD_IH_LOOKUP_TABLE_9_HASH_TYPE;
    lookup_table_60_bit_key_config.sa_search_enable = DRV_RDD_IH_LOOKUP_TABLE_9_SA_ENABLE;
    lookup_table_60_bit_key_config.aging_enable = DRV_RDD_IH_LOOKUP_TABLE_9_AGING_ENABLE;
    lookup_table_60_bit_key_config.cam_enable = DRV_RDD_IH_LOOKUP_TABLE_9_CAM_ENABLE;
    lookup_table_60_bit_key_config.cam_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_LOOKUP_TABLE_9_CAM_BASE_ADDRESS);
    lookup_table_60_bit_key_config.context_table_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_CONTEXT_TABLE_9_BASE_ADDRESS);
    lookup_table_60_bit_key_config.context_table_entry_size = DRV_RDD_IH_CONTEXT_TABLE_9_ENTRY_SIZE;
    lookup_table_60_bit_key_config.cam_context_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_CONTEXT_TABLE_9_CAM_BASE_ADDRESS);
    lookup_table_60_bit_key_config.part_0_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_9_SRC_MAC_KEY_PART_0_START_OFFSET;
    lookup_table_60_bit_key_config.part_0_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_9_SRC_MAC_KEY_PART_0_SHIFT;
    lookup_table_60_bit_key_config.part_1_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_9_SRC_MAC_KEY_PART_1_START_OFFSET;
    lookup_table_60_bit_key_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_9_SRC_MAC_KEY_PART_1_SHIFT;
    lookup_table_60_bit_key_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_9_SRC_MAC_KEY_KEY_EXTENSION;
    lookup_table_60_bit_key_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_9_SRC_MAC_KEY_PART_0_MASK_LOW;
    lookup_table_60_bit_key_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_9_SRC_MAC_KEY_PART_0_MASK_HIGH;
    lookup_table_60_bit_key_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_9_SRC_MAC_KEY_PART_1_MASK_LOW;
    lookup_table_60_bit_key_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_9_SRC_MAC_KEY_PART_1_MASK_HIGH;
    lookup_table_60_bit_key_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_9_SRC_MAC_KEY_GLOBAL_MASK;

    fi_bl_drv_ih_configure_lut_60_bit_key(DRV_RDD_IH_LOOKUP_TABLE_MAC_SA_INDEX,
        &lookup_table_60_bit_key_config);
}
static uint32_t is_matrix_source_lan(DRV_IH_TARGET_MATRIX_SOURCE_PORT source_port)
{

    if ( source_port != DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH0 && source_port != DRV_IH_TARGET_MATRIX_SOURCE_PORT_GPON )
    {
        return 1;
    }
    return 0;
}

static uint32_t is_matrix_dest_lan(DRV_IH_TARGET_MATRIX_SOURCE_PORT source_port)
{
    if ( source_port != DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH0 && source_port != DRV_IH_TARGET_MATRIX_SOURCE_PORT_GPON )
    {
        return 1;
    }
    return 0;
}

static uint32_t is_matrix_source_wan(DRV_IH_TARGET_MATRIX_SOURCE_PORT source_port)
{
    if ( source_port == DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH0 || source_port == DRV_IH_TARGET_MATRIX_SOURCE_PORT_GPON )
    {
        return 1;
    }
    return 0;
}
/* calculates the "direct_mode" value for target matrix */
static uint32_t f_ih_calculate_direct_mode(DRV_IH_TARGET_MATRIX_SOURCE_PORT source_port,
									DRV_IH_TARGET_MATRIX_DESTINATION_PORT destination_port)
{
    /* lan -> lan/pci/multicast: direct mode should be set */
    if (is_matrix_source_lan(source_port) &&
        (is_matrix_dest_lan(destination_port) ||
        destination_port == DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE0 ||
        destination_port == DRV_IH_TARGET_MATRIX_DESTINATION_PORT_MULTICAST) &&
        (DRV_IH_TARGET_MATRIX_DESTINATION_PORT)source_port != destination_port)
    {
        return 1;
    }
    /* pci -> lan/pci: direct mode should be set */
    else if (source_port == DRV_IH_TARGET_MATRIX_SOURCE_PORT_PCIE0 &&
        (is_matrix_dest_lan(destination_port) || destination_port == DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE0))
    {
        return 1;
    }
    return 0;
}
static uint32_t f_ih_calculate_forward(DRV_IH_TARGET_MATRIX_SOURCE_PORT source_port, DRV_IH_TARGET_MATRIX_DESTINATION_PORT destination_port)
{

    /* no self forwarding except PCI */
    if (source_port == DRV_IH_TARGET_MATRIX_SOURCE_PORT_PCIE0 &&
        destination_port == DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE0)
    {
        return 1;
    }
    else if ((DRV_IH_TARGET_MATRIX_DESTINATION_PORT)source_port == destination_port)
    {
        return 0;
    }
    /* no self forwarding (if emac 4 is wan, then its, sp will be emac 4 and its dest port will be gpon */
    else if (is_matrix_source_wan(source_port) && destination_port == DRV_IH_TARGET_MATRIX_DESTINATION_PORT_GPON)
    {
        return 0;
    }
    /* enable forwarding to LANs/WAN/PCI0 (PCI1 is not supported currently). */
    else if (destination_port <= DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE0)
    {
        return 1;
    }
    /* forward to "multicast" from WAN/LAN port */
    else if ((is_matrix_source_wan(source_port) || is_matrix_source_lan(source_port)) &&
        (destination_port == DRV_IH_TARGET_MATRIX_DESTINATION_PORT_MULTICAST))
    {
        return 1;
    }
    return 0;
}

static void sbpm_drv_init(void)
{
    DRV_SBPM_GLOBAL_CONFIGURATION sbpm_global_configuration;
    DRV_SBPM_USER_GROUPS_THRESHOLDS sbpm_ug_configuration;
    int i;

    sbpm_global_configuration.hysteresis = SBPM_DEFAULT_HYSTERESIS;
    sbpm_global_configuration.threshold = SBPM_DEFAULT_THRESHOLD;

    for (i = 0; i < ARRAY_LENGTH(sbpm_ug_configuration.ug_arr); i++)
    {
        sbpm_ug_configuration.ug_arr[i].hysteresis = SBPM_DEFAULT_HYSTERESIS;
        sbpm_ug_configuration.ug_arr[i].threshold = SBPM_DEFAULT_THRESHOLD;
        sbpm_ug_configuration.ug_arr[i].exclusive_hysteresis = SBPM_DEFAULT_HYSTERESIS;
        sbpm_ug_configuration.ug_arr[i].exclusive_threshold = SBPM_DEFAULT_THRESHOLD;
    }
    fi_bl_drv_sbpm_init(SBPM_BASE_ADDRESS, SBPM_LIST_SIZE, SBPM_REPLY_ADDRESS, &sbpm_global_configuration,
        &sbpm_ug_configuration);
}

#define VAL_2K      (2*1024)
#define VAL_2_5K    (2*1024+512)
#define VAL_4K      (4*1024)
#define VAL_5K      (5*1024)
#define VAL_7_5K    (7*1024+512)
#define VAL_15K     (15*1024)
#define VAL_16K     (16*1024)
#define VAL_30K     (30*1024)

static void rdp_bpm_threshod_get(const uint32_t max_bpm_bufs, uint32_t *p_bpm_gbl_thresh, uint32_t *p_bpm_def_thresh)
{
    if (max_bpm_bufs < VAL_2_5K)
    {
        *p_bpm_gbl_thresh = 0;
        *p_bpm_def_thresh = 0;
    }
    else if (max_bpm_bufs < VAL_5K)
    {
        *p_bpm_gbl_thresh = DRV_BPM_GLOBAL_THRESHOLD_2_5K;
        *p_bpm_def_thresh = VAL_2_5K;
    }
    else if (max_bpm_bufs < VAL_7_5K)
    {
        *p_bpm_gbl_thresh = DRV_BPM_GLOBAL_THRESHOLD_5K;
        *p_bpm_def_thresh = VAL_5K;
    }
    else if (max_bpm_bufs < VAL_15K)
    {
        *p_bpm_gbl_thresh = DRV_BPM_GLOBAL_THRESHOLD_7_5K;
        *p_bpm_def_thresh = VAL_7_5K;
    }
    else
    {
#if defined(DSL_63138)
        if (max_bpm_bufs < VAL_30K)
        {
            *p_bpm_gbl_thresh = DRV_BPM_GLOBAL_THRESHOLD_15K;
            *p_bpm_def_thresh = VAL_15K;
        }
        else
        {
            *p_bpm_gbl_thresh = DRV_BPM_GLOBAL_THRESHOLD_30K;
            *p_bpm_def_thresh = VAL_30K;
        }
#else
        *p_bpm_gbl_thresh = DRV_BPM_GLOBAL_THRESHOLD_15K;
        *p_bpm_def_thresh = VAL_15K;
#endif
    }
}
static uint32_t rdp_bpm_buf_size(void)
{
    if (DRV_BBH_DDR_BUFFER_SIZE == DRV_BBH_DDR_BUFFER_SIZE_2_KB)
    {
        return VAL_2K;
    }
    else if (DRV_BBH_DDR_BUFFER_SIZE == DRV_BBH_DDR_BUFFER_SIZE_2_5_KB)
    {
        return VAL_2_5K;
    }
    else if (DRV_BBH_DDR_BUFFER_SIZE == DRV_BBH_DDR_BUFFER_SIZE_4_KB)
    {
        return VAL_4K;
    }
    else if (DRV_BBH_DDR_BUFFER_SIZE == DRV_BBH_DDR_BUFFER_SIZE_16_KB)
    {
        return VAL_16K;
    }
    return 0; /* Error */
}

static int rdp_bpm_cfg_params_get(S_DPI_CFG *pCfg, rdp_bpm_cfg_params *p_bpm_cfg_params)
{
    int sz_for_bpm, max_bpm_bufs, max_mc_bufs, max_bufs_supported, tm_mc_can_merge = 0, wasted_size = 0;
    uint32_t bpm_buf_size, new_runner_mc_size, new_runner_tm_size;

    if (((pCfg->runner_tm_base_addr + (pCfg->runner_tm_size << 20)) == pCfg->runner_mc_base_addr) &&
        ((pCfg->runner_tm_base_addr_phys + (pCfg->runner_tm_size << 20)) == pCfg->runner_mc_base_addr_phys))
    {
        tm_mc_can_merge = 1;
    } 
    
    if (((pCfg->runner_mc_base_addr + (pCfg->runner_mc_size << 20)) == pCfg->runner_tm_base_addr) &&
        ((pCfg->runner_mc_base_addr_phys + (pCfg->runner_mc_size << 20)) == pCfg->runner_tm_base_addr_phys))
        tm_mc_can_merge = 2;

    bpm_buf_size = rdp_bpm_buf_size();
    if (tm_mc_can_merge != 0)
    {
        /* calculate the bpm buffer number supported based on the total of memory size reserved */
        sz_for_bpm = ((pCfg->runner_tm_size + pCfg->runner_mc_size) << 20) - RDP_DDR_DATA_STRUCTURES_SIZE;

        max_bufs_supported = sz_for_bpm / (bpm_buf_size + 512);
        if (max_bufs_supported < VAL_2_5K)
            goto bpm_cfg_params_error;

        rdp_bpm_threshod_get(max_bufs_supported, &p_bpm_cfg_params->bpm_gbl_thresh, &p_bpm_cfg_params->bpm_def_thresh);
        new_runner_tm_size = ROUND_UP_MB(p_bpm_cfg_params->bpm_def_thresh * bpm_buf_size);
        new_runner_mc_size = ROUND_UP_MB(p_bpm_cfg_params->bpm_def_thresh * 512);

        if ((new_runner_tm_size + new_runner_mc_size) > sz_for_bpm)
        {
            max_bufs_supported = p_bpm_cfg_params->bpm_def_thresh - 1;
            rdp_bpm_threshod_get(max_bufs_supported, &p_bpm_cfg_params->bpm_gbl_thresh, &p_bpm_cfg_params->bpm_def_thresh);
            if (p_bpm_cfg_params->bpm_def_thresh == 0)
                goto bpm_cfg_params_error;

            new_runner_tm_size = ROUND_UP_MB(p_bpm_cfg_params->bpm_def_thresh * bpm_buf_size);
            new_runner_mc_size = ROUND_UP_MB(p_bpm_cfg_params->bpm_def_thresh * 512);
        }

        new_runner_tm_size += RDP_DDR_DATA_STRUCTURES_SIZE;
        wasted_size = pCfg->runner_tm_size - (new_runner_tm_size >> 20) + pCfg->runner_mc_size - (new_runner_mc_size >> 20);
        /* we only need to adjust the reserved memory only when sizes for both tables have changed */
        if ((pCfg->runner_tm_size != (new_runner_tm_size >> 20)) && (pCfg->runner_mc_size != (new_runner_mc_size >> 20)))
        {
            pCfg->runner_tm_size = new_runner_tm_size >> 20;
            pCfg->runner_mc_size = new_runner_mc_size >> 20;
            if (tm_mc_can_merge == 1)
            {
                pCfg->runner_mc_base_addr = pCfg->runner_tm_base_addr + new_runner_tm_size;
                pCfg->runner_mc_base_addr_phys = pCfg->runner_tm_base_addr_phys + new_runner_tm_size;
            }
            else /* if (tm_mc_can_merge == 2) */
            {
                pCfg->runner_tm_base_addr = pCfg->runner_mc_base_addr + new_runner_mc_size;
                pCfg->runner_tm_base_addr_phys = pCfg->runner_mc_base_addr_phys + new_runner_mc_size;
            }
            __print("RDP reserved memory has been adjusted\n"
                    "\tTM: base_addr = 0x%p, phys_addr = 0x%p, size = %uMB\n"
                    "\tMC: base_addr = 0x%p, phys_addr = 0x%p, size = %uMB\n",
                    (void *)pCfg->runner_tm_base_addr, (void *)pCfg->runner_tm_base_addr_phys, pCfg->runner_tm_size,
                    (void *)pCfg->runner_mc_base_addr, (void *)pCfg->runner_mc_base_addr_phys, pCfg->runner_mc_size);
        }
    }
    else /* if (tm_mc_can_merge == 0) */
    {
        sz_for_bpm = (pCfg->runner_tm_size << 20) - RDP_DDR_DATA_STRUCTURES_SIZE;
    
        max_bpm_bufs = sz_for_bpm / bpm_buf_size;
        if (max_bpm_bufs < VAL_2_5K)
            goto bpm_cfg_params_error;

        max_mc_bufs = pCfg->runner_mc_size << 11;  /* runner_mc_size * 1MB / 512B */
        if (max_mc_bufs < VAL_2_5K)
            goto bpm_cfg_params_error;

        /* use whichever is less as the real buffer number supported */
        if (max_mc_bufs > max_bpm_bufs)
            max_bufs_supported = max_bpm_bufs;
        else
            max_bufs_supported = max_mc_bufs;

        rdp_bpm_threshod_get(max_bufs_supported, &p_bpm_cfg_params->bpm_gbl_thresh, &p_bpm_cfg_params->bpm_def_thresh);

        new_runner_tm_size = ROUND_UP_MB(p_bpm_cfg_params->bpm_def_thresh * bpm_buf_size) + RDP_DDR_DATA_STRUCTURES_SIZE;
        new_runner_mc_size = ROUND_UP_MB(p_bpm_cfg_params->bpm_def_thresh * 512);
        wasted_size = pCfg->runner_tm_size - (new_runner_tm_size >> 20) + pCfg->runner_mc_size - (new_runner_mc_size >> 20);
    }

    __print("\n RDP TM memory = %uMB, MC memory = %uMB : Max Possible Bufs <%u> of size <%u>; Allocating <%u> bufs; RDP enum <%u>\n",
            pCfg->runner_tm_size, pCfg->runner_mc_size, max_bufs_supported, bpm_buf_size, p_bpm_cfg_params->bpm_def_thresh, p_bpm_cfg_params->bpm_gbl_thresh );
    /* print some warrning messages when there are some significant amounts of wasted reserved memory */
    if (wasted_size >= 5)
        __print("WARNING!RDP reserved memories are wasting %dMB of memory, please adjust your reserved memory size\n", wasted_size);

    return 0;

bpm_cfg_params_error:

    __print("\nRDP memory reservation required at least %uMB TM memory (%uMB for table + %uMB for buffer) "
            "and %uMB MC memory\n",
            ((ROUND_UP_MB(RDP_DDR_DATA_STRUCTURES_SIZE) + ROUND_UP_MB(VAL_2_5K * bpm_buf_size)) >> 20),
            (ROUND_UP_MB(RDP_DDR_DATA_STRUCTURES_SIZE) >> 20),
            (ROUND_UP_MB(VAL_2_5K * bpm_buf_size) >> 20),
            (ROUND_UP_MB(VAL_2_5K << 9) >> 20));
    __print("\nHave only reserved RDP TM memory = %uMB, MC memory = %uMB\n",
            pCfg->runner_tm_size, pCfg->runner_mc_size);

    return -1;

}

static void bpm_drv_init(S_DPI_CFG  *pCfg, rdp_bpm_cfg_params *p_bpm_cfg_params)
{
    DRV_BPM_GLOBAL_CONFIGURATION bpm_global_configuration;
    DRV_BPM_USER_GROUPS_THRESHOLDS bpm_ug_configuration;
    DRV_BPM_RUNNER_MSG_CTRL_PARAMS runner_msg_ctrl_params;
    int i;

    bpm_global_configuration.hysteresis = BPM_DEFAULT_HYSTERESIS;
    bpm_global_configuration.threshold = p_bpm_cfg_params->bpm_gbl_thresh;
    for (i = 0; i < ARRAY_LENGTH(bpm_ug_configuration.ug_arr); i++)
    {
        bpm_ug_configuration.ug_arr[i].hysteresis = BPM_DEFAULT_HYSTERESIS;
        bpm_ug_configuration.ug_arr[i].threshold = (i == DRV_BPM_USER_GROUP_7) ?
            BPM_CPU_NUMBER_OF_BUFFERS : p_bpm_cfg_params->bpm_def_thresh;
        bpm_ug_configuration.ug_arr[i].exclusive_hysteresis = BPM_DEFAULT_HYSTERESIS;
        bpm_ug_configuration.ug_arr[i].exclusive_threshold = (i == DRV_BPM_USER_GROUP_7) ?
            BPM_CPU_NUMBER_OF_BUFFERS - 32 : p_bpm_cfg_params->bpm_def_thresh;
    }

    fi_bl_drv_bpm_init(&bpm_global_configuration,&bpm_ug_configuration, BPM_REPLY_RUNNER_A_ADDRESS, DRV_SPARE_BN_MESSAGE_FORMAT);

    fi_bl_drv_bpm_get_runner_msg_ctrl(&runner_msg_ctrl_params);

    runner_msg_ctrl_params.runner_a_reply_target_address = (BPM_REPLY_RUNNER_A_ADDRESS + 0x10000) >> 3;
    runner_msg_ctrl_params.runner_b_reply_target_address = (BPM_REPLY_RUNNER_B_ADDRESS + 0x10000) >> 3;

    fi_bl_drv_bpm_set_runner_msg_ctrl(&runner_msg_ctrl_params);

    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_GPON, DRV_BPM_USER_GROUP_4);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_EMAC0, DRV_BPM_USER_GROUP_4);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_EMAC1, DRV_BPM_USER_GROUP_4);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_EMAC2, DRV_BPM_USER_GROUP_4);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_EMAC3, DRV_BPM_USER_GROUP_4);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_EMAC4, DRV_BPM_USER_GROUP_4);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_MIPS_C, DRV_BPM_USER_GROUP_4);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_MIPS_D, DRV_BPM_USER_GROUP_4);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_PCI0, DRV_BPM_USER_GROUP_4);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_PCI1, DRV_BPM_USER_GROUP_4);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_USB0, DRV_BPM_USER_GROUP_4);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_USB1, DRV_BPM_USER_GROUP_4);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_SPARE_0, DRV_BPM_USER_GROUP_4);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_SPARE_1, DRV_BPM_USER_GROUP_4);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_RNR_A, DRV_BPM_USER_GROUP_4);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_RNR_B, DRV_BPM_USER_GROUP_4);
}

static void f_ih_configure_target_matrix(void)
{
    int i, j;
    DRV_IH_TARGET_MATRIX_PER_SP_CONFIG per_sp_config;

    for (i = DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH0; i < DRV_IH_TARGET_MATRIX_NUMBER_OF_SOURCE_PORTS; i++)
    {
        /* the destination ports after 'multicast' are not in use currently */
        for (j = DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH0; j < DRV_IH_TARGET_MATRIX_NUMBER_OF_DESTINATION_PORTS; j++)
        {
            per_sp_config.entry[j].target_memory = DRV_IH_TARGET_MEMORY_DDR;
            per_sp_config.entry[j].direct_mode = f_ih_calculate_direct_mode(i, j);

            fi_bl_drv_ih_set_forward(i, j, f_ih_calculate_forward(i, j));
        }
        fi_bl_drv_ih_set_target_matrix(i, &per_sp_config);
    }
}

static void f_ih_configure_parser(void)
{
    DRV_IH_PARSER_CONFIG parser_config ;

    /* parser configuration */
    parser_config.tcp_flags = TCP_CTRL_FLAG_FIN | TCP_CTRL_FLAG_RST | TCP_CTRL_FLAG_SYN;
    parser_config.exception_status_bits = IH_PARSER_EXCEPTION_STATUS_BITS;
    parser_config.ppp_code_1_ipv6 = 1;
    parser_config.ipv6_extension_header_bitmask = 0;
    parser_config.snap_user_defined_organization_code = IH_REGS_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_CODE_RESET_VALUE_RESET_VALUE;
    parser_config.snap_rfc1042_encapsulation_enable = IH_REGS_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_RFC1042_DISABLED_VALUE_RESET_VALUE;
    parser_config.snap_802_1q_encapsulation_enable = IH_REGS_PARSER_CORE_CONFIGURATION_SNAP_ORG_CODE_EN_8021Q_DISABLED_VALUE_RESET_VALUE;
    parser_config.gre_protocol = IH_REGS_PARSER_CORE_CONFIGURATION_GRE_PROTOCOL_CFG_GRE_PROTOCOL_PROTOCOL_VALUE_RESET_VALUE;

    /*  eng[15] = eng[16] = 1 for AH header detection (IPv4 and IPv6) */
    parser_config.eng_cfg = IH_PARSER_AH_DETECTION;

    fi_bl_drv_ih_configure_parser(&parser_config);
}



/* this function configures the IH classes which are in use */
static void f_ih_configure_classes(void)
{
    uint8_t i;

    for (i = 0; i < ARRAY_LENGTH(gs_ih_classes); i++)
    {
        fi_bl_drv_ih_configure_class(gs_ih_classes[i].class_index, &gs_ih_classes[i].class_config);
    }
}

static void f_ih_init(void)
{
    DRV_IH_GENERAL_CONFIG ih_general_config ;
    DRV_IH_PACKET_HEADER_OFFSETS packet_header_offsets ;
    DRV_IH_RUNNER_BUFFERS_CONFIG runner_buffers_config ;
    DRV_IH_RUNNERS_LOAD_THRESHOLDS runners_load_thresholds ;
    DRV_IH_ROUTE_ADDRESSES xi_route_addresses ;
    DRV_IH_LOGICAL_PORTS_CONFIG logical_ports_config ;
    DRV_IH_SOURCE_PORT_TO_INGRESS_QUEUE_MAPPING source_port_to_ingress_queue_mapping ;
    DRV_IH_WAN_PORTS_CONFIG wan_ports_config ;

    /* general configuration */
    ih_general_config.runner_a_ih_response_address                  = MS_BYTE_TO_8_BYTE_RESOLUTION(RUNNER_FLOW_IH_RESPONSE_ADDRESS);
    ih_general_config.runner_b_ih_response_address                  = MS_BYTE_TO_8_BYTE_RESOLUTION(RUNNER_FLOW_IH_RESPONSE_ADDRESS);
    ih_general_config.runner_a_ih_congestion_report_address         = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_0_IH_CONGESTION_REPORT_ADDRESS);
    ih_general_config.runner_b_ih_congestion_report_address         = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_1_IH_CONGESTION_REPORT_ADDRESS);
    ih_general_config.runner_a_ih_congestion_report_enable          = DRV_RDD_IH_RUNNER_0_IH_CONGESTION_REPORT_ENABLE;
    ih_general_config.runner_b_ih_congestion_report_enable          = DRV_RDD_IH_RUNNER_1_IH_CONGESTION_REPORT_ENABLE;
    ih_general_config.lut_searches_enable_in_direct_mode            = 1;
    ih_general_config.sn_stamping_enable_in_direct_mode             = 1;
    ih_general_config.header_length_minimum                         = IH_HEADER_LENGTH_MIN;
    ih_general_config.congestion_discard_disable                    = 0;
    ih_general_config.cam_search_enable_upon_invalid_lut_entry      = DRV_RDD_IH_CAM_SEARCH_ENABLE_UPON_INVALID_LUT_ENTRY;

    fi_bl_drv_ih_set_general_configuration(&ih_general_config);

    /* packet header offsets configuration */
    packet_header_offsets.eth0_packet_header_offset                 = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    packet_header_offsets.eth1_packet_header_offset                 = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    packet_header_offsets.gpon_packet_header_offset                 = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    packet_header_offsets.runner_a_packet_header_offset             = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    packet_header_offsets.runner_b_packet_header_offset             = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    packet_header_offsets.eth2_packet_header_offset                 = 0;
    packet_header_offsets.eth3_packet_header_offset                 = 0;
    packet_header_offsets.eth4_packet_header_offset                 = 0;

    fi_bl_drv_ih_set_packet_header_offsets(& packet_header_offsets);

    /* Runner Buffers configuration */
    /* same ih_managed_rb_base_address should be used for both runners */
    runner_buffers_config.runner_a_ih_managed_rb_base_address       = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_0_IH_MANAGED_RB_BASE_ADDRESS);
    runner_buffers_config.runner_b_ih_managed_rb_base_address       = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_1_IH_MANAGED_RB_BASE_ADDRESS);
    runner_buffers_config.runner_a_runner_managed_rb_base_address   = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_0_RUNNER_MANAGED_RB_BASE_ADDRESS);
    runner_buffers_config.runner_b_runner_managed_rb_base_address   = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_1_RUNNER_MANAGED_RB_BASE_ADDRESS);
    runner_buffers_config.runner_a_maximal_number_of_buffers        = DRV_IH_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS_32;
    runner_buffers_config.runner_b_maximal_number_of_buffers        = DRV_IH_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS_32;

    fi_bl_drv_ih_set_runner_buffers_configuration(&runner_buffers_config);

    /* runners load thresholds configuration */
    runners_load_thresholds.runner_a_high_congestion_threshold      = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    runners_load_thresholds.runner_b_high_congestion_threshold      = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    runners_load_thresholds.runner_a_exclusive_congestion_threshold = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    runners_load_thresholds.runner_b_exclusive_congestion_threshold = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    runners_load_thresholds.runner_a_load_balancing_threshold       = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    runners_load_thresholds.runner_b_load_balancing_threshold       = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    runners_load_thresholds.runner_a_load_balancing_hysteresis      = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    runners_load_thresholds.runner_b_load_balancing_hysteresis      = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;

    fi_bl_drv_ih_set_runners_load_thresholds (& runners_load_thresholds);

    /* route addresses configuration */
    xi_route_addresses.eth0_route_address     = IH_ETH0_ROUTE_ADDRESS;
    xi_route_addresses.eth1_route_address     = IH_ETH1_ROUTE_ADDRESS;
    xi_route_addresses.gpon_route_address     = IH_DSL_ROUTE_ADDRESS;
    xi_route_addresses.runner_a_route_address = IH_RUNNER_A_ROUTE_ADDRESS;
    xi_route_addresses.runner_b_route_address = IH_RUNNER_B_ROUTE_ADDRESS;

    fi_bl_drv_ih_set_route_addresses(&xi_route_addresses);

    /* logical ports configuration */
    logical_ports_config.eth0_config.parsing_layer_depth        = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_ETH0_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.eth0_config.proprietary_tag_size       = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    logical_ports_config.eth1_config.parsing_layer_depth        = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_ETH1_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
#if defined(__UBOOT__)
    logical_ports_config.eth1_config.proprietary_tag_size       = DRV_IH_PROPRIETARY_TAG_SIZE_0;
#else
    logical_ports_config.eth1_config.proprietary_tag_size       = DRV_IH_PROPRIETARY_TAG_SIZE_4;
#endif

    logical_ports_config.gpon_config.parsing_layer_depth        = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_GPON_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.gpon_config.proprietary_tag_size       = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    logical_ports_config.eth2_config.parsing_layer_depth 		= 0;
    logical_ports_config.eth2_config.proprietary_tag_size 		= 0;

    logical_ports_config.eth3_config.parsing_layer_depth 		= 0;
    logical_ports_config.eth3_config.proprietary_tag_size 		= 0;

    logical_ports_config.eth4_config.parsing_layer_depth 		= 0;
    logical_ports_config.eth4_config.proprietary_tag_size 		= 0;

    logical_ports_config.runner_a_config.parsing_layer_depth    = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_RNRA_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.runner_a_config.proprietary_tag_size   = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    logical_ports_config.runner_b_config.parsing_layer_depth    = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_RNRB_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.runner_b_config.proprietary_tag_size   = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    logical_ports_config.pcie0_config.parsing_layer_depth 		= IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_PCIE0_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.pcie0_config.proprietary_tag_size 		= DRV_IH_PROPRIETARY_TAG_SIZE_0;

    logical_ports_config.pcie1_config.parsing_layer_depth 		= IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_PCIE1_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.pcie1_config.proprietary_tag_size 		= DRV_IH_PROPRIETARY_TAG_SIZE_0;

    fi_bl_drv_ih_set_logical_ports_configuration(&logical_ports_config);

    /* source port to ingress queue mapping */
    source_port_to_ingress_queue_mapping.eth0_ingress_queue     = IH_INGRESS_QUEUE_0_ETH0;
    source_port_to_ingress_queue_mapping.eth1_ingress_queue     = IH_INGRESS_QUEUE_1_ETH1;
    source_port_to_ingress_queue_mapping.gpon_ingress_queue     = IH_INGRESS_QUEUE_5_DSL;
    source_port_to_ingress_queue_mapping.runner_a_ingress_queue = IH_INGRESS_QUEUE_6_RUNNER_A;
    source_port_to_ingress_queue_mapping.runner_b_ingress_queue = IH_INGRESS_QUEUE_7_RUNNER_B;

    fi_bl_drv_ih_set_source_port_to_ingress_queue_mapping(&source_port_to_ingress_queue_mapping);

    /* ingress queues configuration */
    fi_ih_configure_ingress_queues();

    /* configure wan ports */
    wan_ports_config.eth0 = 1;
    wan_ports_config.eth1 = 0;
    wan_ports_config.eth2 = 0;
    wan_ports_config.eth3 = 0;
    wan_ports_config.eth4 = 0;
    wan_ports_config.gpon = 1; /* DSL */
    wan_ports_config.runner_a = 0;
    wan_ports_config.runner_b = 0;
    wan_ports_config.pcie0 = 0;
    wan_ports_config.pcie1 = 0;

    fi_bl_drv_ih_configure_wan_ports(&wan_ports_config);

    f_ih_configure_lookup_tables();

    f_ih_configure_classes();

    f_ih_configure_target_matrix();

    f_ih_configure_parser();
}


static void f_initialize_basic_runner_parameters(void)
{
    BL_LILAC_RDD_ERROR_DTE rdd_error;
    uint32_t    i,j;

    /* Default configuration of all RDD reasons to CPU_RX_QUEUE_ID_0 */
    for (i = 0; i < rdpa_cpu_reason__num_of; i++)
    {
        rdd_error = rdd_cpu_reason_to_cpu_rx_queue(i, 0, rdpa_dir_ds, 0);

        rdd_error |= rdd_cpu_reason_to_cpu_rx_queue(i, 0, rdpa_dir_us, 0);
    }

    /* Initialize Ethernet priority queues */
    for (i = 0; i < 8; i++)
    {
        for (j = BL_LILAC_RDD_EMAC_ID_0; j < BL_LILAC_RDD_EMAC_ID_7; j++)
        {
            /* Configure queue size in RDD */
            rdd_error |= rdd_eth_tx_queue_config(j, i, CS_RDD_ETH_TX_QUEUE_PACKET_THRESHOLD,0,INVALID_COUNTER_ID);
        }
    }
}

static void f_configure_runner(S_DPI_CFG  *pCfg, rdp_bpm_cfg_params *p_bpm_cfg_params)
{
    /* Local Variables */
    RDD_INIT_PARAMS rdd_init_params ;

    /* zero Runner memories (data, program and context) */
    rdd_init();

    rdd_load_microcode((uint8_t*) NULL,
            (uint8_t*) firmware_binary_B, (uint8_t*) firmware_binary_C,
            (uint8_t*) firmware_binary_D);

    rdd_load_prediction((uint8_t*) NULL,
            (uint8_t*) firmware_predict_B, (uint8_t*) firmware_predict_C,
            (uint8_t*) firmware_predict_D);

    /* Add basic offset when pass the addresses to RDD */
    rdd_init_params.ddr_pool_ptr       = (uint8_t *) pCfg->runner_tm_base_addr;
    rdd_init_params.ddr_pool_ptr_phys  = (uint32_t) pCfg->runner_tm_base_addr_phys;
    rdd_init_params.extra_ddr_pool_ptr = (uint8_t *) pCfg->runner_mc_base_addr;
    rdd_init_params.extra_ddr_pool_ptr_phys = (uint32_t) pCfg->runner_mc_base_addr_phys;
    /* ddr_runner_table only needs to be 512KB aligned, but we make 1 MB anyway */
    rdd_init_params.ddr_runner_tables_ptr = (uint8_t *) ROUND_UP_MB(pCfg->runner_tm_base_addr +
        (p_bpm_cfg_params->bpm_def_thresh * rdp_bpm_buf_size()));
    rdd_init_params.ddr_runner_tables_ptr_phys = RDD_RSV_VIRT_TO_PHYS(pCfg->runner_tm_base_addr, 
        pCfg->runner_tm_base_addr_phys, rdd_init_params.ddr_runner_tables_ptr);
    rdd_init_params.mac_table_size     = BL_LILAC_RDD_MAC_TABLE_SIZE_64;
    /* we don't have iptv table. the following is just a dummy */
    rdd_init_params.iptv_table_size    = BL_LILAC_RDD_MAC_TABLE_SIZE_256;
    rdd_init_params.ddr_headroom_size  = pCfg->headroom_size;
    /* XXX: Temporary, take this from global_system */
#ifdef __UBOOT__
    rdd_init_params.broadcom_switch_mode = 0; /* unmanaged */
#else
    rdd_init_params.broadcom_switch_mode = 1; /* managed */
#endif
    rdd_init_params.broadcom_switch_physical_port = BL_LILAC_RDD_LAN1_BRIDGE_PORT;

    /* ip class operational mode*/
    rdd_init_params.bridge_flow_cache_mode = 0;

    /* The chip revision A0/B0 refers to BCM6838.  The BCM63138A0 handles Runner semaphores
     * like BCM6838A0. The BCM63148 and BCM63138B0 handle Runner semaphores like BCM6838B0.
     */
    rdd_init_params.chip_revision = RDD_CHIP_REVISION_B0;
    rdd_init_params.cpu_tx_abs_packet_limit = RDD_CPU_TX_ABS_FIFO_SIZE;
    rdd_init_params.lp_mode = pCfg->runner_lp;
    rdd_data_structures_init(&rdd_init_params);

}

void f_basic_bpm_sp_enable(void)
{
    BPM_MODULE_REGS_BPM_SP_EN bpm_sp_enable;

    BPM_MODULE_REGS_BPM_SP_EN_READ( bpm_sp_enable);

    bpm_sp_enable.rnra_en = 1;
    bpm_sp_enable.rnrb_en = 1;
    bpm_sp_enable.gpon_en = 0;
    bpm_sp_enable.emac0_en = 1;
    bpm_sp_enable.emac1_en = 1;


    BPM_MODULE_REGS_BPM_SP_EN_WRITE( bpm_sp_enable);

}
void f_basic_sbpm_sp_enable(void)
{
    DRV_SBPM_SP_ENABLE sbpm_sp_enable = { 0 };

    sbpm_sp_enable.rnra_sp_enable = 1;
    sbpm_sp_enable.rnrb_sp_enable = 1;
    sbpm_sp_enable.eth0_sp_enable = 1;
    sbpm_sp_enable.eth1_sp_enable = 1;
    sbpm_sp_enable.eth2_sp_enable = 0;
    sbpm_sp_enable.eth3_sp_enable = 0;
    sbpm_sp_enable.eth4_sp_enable = 0;

    fi_bl_drv_sbpm_sp_enable(&sbpm_sp_enable);
}

uint32_t data_path_init( S_DPI_CFG  *pCfg)
{
    uint32_t            error        = DPI_RC_OK;
    uint32_t            rnrFreq      = DEFAULT_RUNNER_FREQ;

    DRV_BBH_PORT_INDEX  macIter;
    rdp_bpm_cfg_params   bpm_cfg_params;


    /* Point to default configuration if none is passed to init */
    pDpiCfg = !pCfg ? &DpiBasicConfig : pCfg;

    /*init dma arrays*/
    f_initialize_bbh_dma_sdma_related_arrays();

    if (rdp_bpm_cfg_params_get(pCfg, &bpm_cfg_params)) {
        __print("FAILED TO INITIALIZE RDP DUE TO RESERVE MEMORY NOT ENOUGH\n");
        return DPI_RC_ERROR;
    }

    /*init runner,load microcode and structures*/
    f_configure_runner(pCfg, &bpm_cfg_params);

    /*set runner frequency*/
    if (pDpiCfg->runner_freq == 0)
    {
        rnrFreq = DEFAULT_RUNNER_FREQ;
    }
    else
    {
        rnrFreq = pDpiCfg->runner_freq;
    }

    rdd_runner_frequency_set(rnrFreq);
        
    /*configure IH*/
    f_ih_init();

    /*init SBPM*/
    sbpm_drv_init();

    /*init BPM*/
    bpm_drv_init(pCfg, &bpm_cfg_params);

    /*init BBH of emac ports*/
    for ( macIter =  DRV_BBH_EMAC_0; macIter <= DRV_BBH_EMAC_1; macIter++)
    {
        f_initialize_bbh_of_emac_port(macIter);
    }
    /*init DMA and SDMA*/
    f_initialize_dma_sdma();

    /*configure basic runner parameters*/
    f_initialize_basic_runner_parameters();

    if ( error == DPI_RC_OK )
    {
        initDone = 1;
    }
    return error;

}

uint32_t data_path_go(void)
{
    if ( initDone != 1 )
    {
        __print("Data Path init didn't finished \n");
        return DPI_RC_ERROR;
    }

    /*enable runner*/
    rdd_runner_enable();

    /*enable the source ports in bpm/sbpm in case of basic config*/
    f_basic_bpm_sp_enable();
    f_basic_sbpm_sp_enable();

    return DPI_RC_OK;
}
uint32_t data_path_shutdown(void)
{
    return DPI_RC_OK;
}

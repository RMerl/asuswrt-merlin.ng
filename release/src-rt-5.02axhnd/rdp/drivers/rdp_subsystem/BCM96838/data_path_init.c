/*
   <:copyright-BRCM:2013:DUAL/GPL:standard
   
      Copyright (c) 2013 Broadcom 
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
/******************************************************************************/
/*                                                                            */
/* File Description:                                                          */
/*                                                                            */
/* This file contains the implementation for Broadcom's 6838 Data path		  */
/* initialization sequence											          */
/*                                                                            */
/******************************************************************************/

#ifdef _CFE_
#include "lib_types.h"
#endif
#include "data_path_init.h"
#include "rdpa_types.h"
#include "rdp_drv_bpm.h"
#include "rdp_drv_sbpm.h"
#include "rdp_drv_ih.h"
#include "rdp_drv_bbh.h"
#include "rdp_dma.h"
#include "rdd_ih_defs.h"
#include "rdd.h"
#ifdef LEGACY_RDP
#include "rdd_legacy_conv.h"
#endif
#include "rdd_runner_defs.h"
#include "rdd_init.h"
#include "rdd_tm.h"

#ifdef RDP_SIM
#define UtilGetChipRev() 2
#define err bdmf_trace
#else
#include "shared_utils.h"
#define err printk
#endif

#ifndef RDD_BASIC
void f_initialize_bbh_of_gpon_port(void);
void f_initialize_bbh_of_epon_port(void);
#endif

static  S_DPI_CFG       DpiBasicConfig = {DRV_BBH_GPON,1536,0,0,0,0,0,0,0,0,0,0,RDPA_BPM_BUFFER_2K,DRV_BPM_GLOBAL_THRESHOLD_7_5K};

#define CS_RDD_ETH_TX_QUEUE_PACKET_THRESHOLD    256

#ifdef _CFE_
#define VIRT_TO_PHYS(_addr)                                     K0_TO_PHYS((uint32_t)_addr)
#else
#ifndef CPHYSADDR
#define CPHYSADDR(val1) ((unsigned long)(val1) & 0x1fffffff)
#endif
#define VIRT_TO_PHYS(_addr)                                     CPHYSADDR(_addr)
#endif
#define SHIFTL(_a) ( 1 << _a)

/****************************************************************************
 *
 * Defines
 *
 * *************************************************************************/

#define DEFAULT_RUNNER_FREQ                             800
#define RDD_CPU_TX_ABS_FIFO_SIZE                2048
/* multicast header size */
#define BBH_MULTICAST_HEADER_SIZE_FOR_LAN_PORT 32

/* PD FIFO size of EMAC, when MDU mode is disabled */
#define BBH_TX_EMAC_PD_FIFO_SIZE_MDU_MODE_DISABLED 8
/* PD FIFO size of EMAC, when MDU mode is enabled */
#define BBH_TX_EMAC_PD_FIFO_SIZE_MDU_MODE_ENABLED 4
/* DMA */
#define BBH_RX_DMA_FIFOS_SIZE_WAN_DMA 32
#define BBH_RX_DMA_FIFOS_SIZE_WAN_GPON 32
#define BBH_RX_DMA_FIFOS_SIZE_LAN_DMA 13
#define BBH_RX_DMA_FIFOS_SIZE_RGMII_LAN_DMA 12
#define BBH_RX_DMA_FIFOS_SIZE_LAN_BBH 13
#define BBH_RX_DMA_FIFOS_SIZE_RGMII_LAN_BBH 12
#define BBH_RX_DMA_EXCLUSIVE_THRESHOLD_WAN_DMA 32
#define BBH_RX_DMA_EXCLUSIVE_THRESHOLD_WAN_BBH 32
#define BBH_RX_DMA_EXCLUSIVE_THRESHOLD_LAN_DMA 13
#define BBH_RX_DMA_EXCLUSIVE_THRESHOLD_RGMII_LAN_DMA 13
#define BBH_RX_DMA_EXCLUSIVE_THRESHOLD_LAN_BBH 13
#define BBH_RX_DMA_EXCLUSIVE_THRESHOLD_RGMII_LAN_BBH 12

#define BBH_RX_SDMA_FIFOS_SIZE_WAN 7
#define BBH_RX_SDMA_FIFOS_SIZE_LAN 5
#define BBH_RX_SDMA_EXCLUSIVE_THRESHOLD_WAN 6
#define BBH_RX_SDMA_EXCLUSIVE_THRESHOLD_LAN 4
#define BBH_RX_DMA_TOTAL_NUMBER_OF_CHUNK_DESCRIPTORS 64
#define BBH_RX_SDMA_TOTAL_NUMBER_OF_CHUNK_DESCRIPTORS 32

#define MIN_ETH_PKT_SIZE 64
#define MIN_GPON_PKT_SIZE 60
#define BBH_RX_FLOWS_32_255_GROUP_DIVIDER 255
#define BBH_RX_ETH_MIN_PKT_SIZE_SELECTION_INDEX 0
#define BBH_RX_OMCI_MIN_PKT_SIZE_SELECTION_INDEX 1
#define BBH_RX_MAX_PKT_SIZE_SELECTION_INDEX 0
/* minimal OMCI packet size */
#define MIN_OMCI_PKT_SIZE 14

#define IH_HEADER_LENGTH_MIN 60
/* high congestion threshold of runner A (DS) */
#define IH_GPON_MODE_DS_RUNNER_A_HIGH_CONGESTION_THRESHOLD 28
/* exclusive congestion threshold of runner A (DS) */
#define IH_GPON_MODE_DS_RUNNER_A_EXCLUSIVE_CONGESTION_THRESHOLD 30
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
#define IH_INGRESS_QUEUE_CONGESTION_THRESHOLD_WAN 65
/* congestion threshold of ingress queue of each runner */
#define IH_INGRESS_QUEUE_CONGESTION_THRESHOLD_RUNNERS 65

#define IH_IP_L4_FILTER_USER_DEFINED_0                  0
#define IH_IP_L4_FILTER_USER_DEFINED_1                  1
#define IH_IP_L4_FILTER_USER_DEFINED_2                  2
#define IH_IP_L4_FILTER_USER_DEFINED_3                  3

#define IH_L4_FILTER_DEF                                                0xff
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
#define SBPM_G9991_DEFAULT_THRESHOLD 768
#define SBPM_DEFAULT_HYSTERESIS 0
#define BPM_DEFAULT_HYSTERESIS 64
#define SBPM_BASE_ADDRESS 0
#define SBPM_LIST_SIZE 0x3FF
#define BPM_CPU_NUMBER_OF_BUFFERS 1536
#define BPM_DEFAULT_DS_THRESHOLD 7680


/*GPON DEFS*/
/* size of each one of FIFOs 0-7 */
#define BBH_TX_GPON_PD_FIFO_SIZE_0_7 4
/* size of each one of FIFOs 8-15 */
#define BBH_TX_GPON_PD_FIFO_SIZE_8_15 3
#define BBH_TX_GPON_PD_FIFO_SIZE_16_23 3
#define BBH_TX_GPON_PD_FIFO_SIZE_24_31 3
#define BBH_TX_GPON_PD_FIFO_SIZE_32_39 3
#define BBH_TX_GPON_NUMBER_OF_TCONTS_IN_PD_FIFO_GROUP 8
#define BBH_TX_GPON_TOTAL_NUMBER_OF_PDS 128
#define BBH_TX_GPON_PD_FIFO_BASE_0 0
#define BBH_TX_GPON_PD_FIFO_BASE_1 (BBH_TX_GPON_PD_FIFO_BASE_0 + BBH_TX_GPON_PD_FIFO_SIZE_0_7)
#define BBH_TX_GPON_PD_FIFO_BASE_2 (BBH_TX_GPON_PD_FIFO_BASE_1 + BBH_TX_GPON_PD_FIFO_SIZE_0_7)
#define BBH_TX_GPON_PD_FIFO_BASE_3 (BBH_TX_GPON_PD_FIFO_BASE_2 + BBH_TX_GPON_PD_FIFO_SIZE_0_7)
#define BBH_TX_GPON_PD_FIFO_BASE_4 (BBH_TX_GPON_PD_FIFO_BASE_3 + BBH_TX_GPON_PD_FIFO_SIZE_0_7)
#define BBH_TX_GPON_PD_FIFO_BASE_5 (BBH_TX_GPON_PD_FIFO_BASE_4 + BBH_TX_GPON_PD_FIFO_SIZE_0_7)
#define BBH_TX_GPON_PD_FIFO_BASE_6 (BBH_TX_GPON_PD_FIFO_BASE_5 + BBH_TX_GPON_PD_FIFO_SIZE_0_7)
#define BBH_TX_GPON_PD_FIFO_BASE_7 (BBH_TX_GPON_PD_FIFO_BASE_6 + BBH_TX_GPON_PD_FIFO_SIZE_0_7)
#define BBH_TX_GPON_PD_FIFO_BASE_8_15 (BBH_TX_GPON_PD_FIFO_BASE_7 + BBH_TX_GPON_PD_FIFO_SIZE_0_7)
#define BBH_TX_GPON_PD_FIFO_BASE_16_23 \
    (BBH_TX_GPON_PD_FIFO_BASE_8_15 + (BBH_TX_GPON_NUMBER_OF_TCONTS_IN_PD_FIFO_GROUP * BBH_TX_GPON_PD_FIFO_SIZE_8_15))
#define BBH_TX_GPON_PD_FIFO_BASE_24_31 (BBH_TX_GPON_PD_FIFO_BASE_16_23 + \
    (BBH_TX_GPON_NUMBER_OF_TCONTS_IN_PD_FIFO_GROUP * BBH_TX_GPON_PD_FIFO_SIZE_16_23))
#define BBH_TX_GPON_PD_FIFO_BASE_32_39 (BBH_TX_GPON_PD_FIFO_BASE_24_31 + \
    (BBH_TX_GPON_NUMBER_OF_TCONTS_IN_PD_FIFO_GROUP * BBH_TX_GPON_PD_FIFO_SIZE_24_31))

/*EPON DEFS*/
#define BBH_TX_EPON_PD_FIFO_SIZE_0_7 8
/* size of each one of FIFOs 8-15 */
#define BBH_TX_EPON_PD_FIFO_SIZE_8_15 8
#define BBH_TX_EPON_PD_FIFO_SIZE_16_23 8
#define BBH_TX_EPON_PD_FIFO_SIZE_24_31 8
#define BBH_TX_EPON_PD_FIFO_SIZE_32_39 8
#define BBH_TX_EPON_NUMBER_OF_TCONTS_IN_PD_FIFO_GROUP 8
#define BBH_TX_EPON_TOTAL_NUMBER_OF_PDS 128
#define BBH_TX_EPON_PD_FIFO_BASE_0 0
#define BBH_TX_EPON_PD_FIFO_BASE_1 (BBH_TX_EPON_PD_FIFO_BASE_0 + BBH_TX_EPON_PD_FIFO_SIZE_0_7)
#define BBH_TX_EPON_PD_FIFO_BASE_2 (BBH_TX_EPON_PD_FIFO_BASE_1 + BBH_TX_EPON_PD_FIFO_SIZE_0_7)
#define BBH_TX_EPON_PD_FIFO_BASE_3 (BBH_TX_EPON_PD_FIFO_BASE_2 + BBH_TX_EPON_PD_FIFO_SIZE_0_7)
#define BBH_TX_EPON_PD_FIFO_BASE_4 (BBH_TX_EPON_PD_FIFO_BASE_3 + BBH_TX_EPON_PD_FIFO_SIZE_0_7)
#define BBH_TX_EPON_PD_FIFO_BASE_5 (BBH_TX_EPON_PD_FIFO_BASE_4 + BBH_TX_EPON_PD_FIFO_SIZE_0_7)
#define BBH_TX_EPON_PD_FIFO_BASE_6 (BBH_TX_EPON_PD_FIFO_BASE_5 + BBH_TX_EPON_PD_FIFO_SIZE_0_7)
#define BBH_TX_EPON_PD_FIFO_BASE_7 (BBH_TX_EPON_PD_FIFO_BASE_6 + BBH_TX_EPON_PD_FIFO_SIZE_0_7)
#define BBH_TX_EPON_PD_FIFO_BASE_8_15 (BBH_TX_EPON_PD_FIFO_BASE_7 + BBH_TX_EPON_PD_FIFO_SIZE_0_7)
#define BBH_TX_EPON_PD_FIFO_BASE_16_23 \
    (BBH_TX_EPON_PD_FIFO_BASE_8_15 + (BBH_TX_EPON_NUMBER_OF_TCONTS_IN_PD_FIFO_GROUP * BBH_TX_EPON_PD_FIFO_SIZE_8_15))
#define BBH_TX_EPON_PD_FIFO_BASE_24_31 (BBH_TX_EPON_PD_FIFO_BASE_16_23 + \
    (BBH_TX_EPON_NUMBER_OF_TCONTS_IN_PD_FIFO_GROUP * BBH_TX_EPON_PD_FIFO_SIZE_16_23))
#define BBH_TX_EPON_PD_FIFO_BASE_32_39 0
#define BBH_TX_EPON_RUNNER_STS_ROUTE 0xB
#define DRV_IH_INGRESS_QOS_NUMBER_OF_BUFFERS 28

/******************************************************************************/
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    BB_MODULE_DMA,
    BB_MODULE_SDMA,

    BB_MODULE_NUM
} E_BB_MODULE;
typedef enum
{
    DMA_PERIPHERAL_EMAC_0,
    DMA_PERIPHERAL_EMAC_1,
    DMA_PERIPHERAL_EMAC_2,
    DMA_PERIPHERAL_EMAC_3,
    DMA_PERIPHERAL_EMAC_4,
    DMA_PERIPHERAL_EMAC_AE_GPON_EPON,

    DMA_NUMBER_OF_PERIPHERALS
} E_DMA_PERIPHERAL;

/******************************************************************************/
/*                                                                            */
/* Macros definitions                                                         */
/*                                                                            */
/******************************************************************************/
/* sets bit #i of a given number to a given value */
#define SET_BIT_I( number, i, bit_value )   ( ( number ) &= ( ~ ( 1 << i ) ), ( number ) |= ( bit_value << i ) )
#define MS_BYTE_TO_8_BYTE_RESOLUTION(address)  ((address) >> 3)
#define ARRAY_LENGTH(array) (sizeof(array)/sizeof(array[0]))
#define BBH_PORT_IS_WAN(_portIndex)           ( pDpiCfg->wan_bbh == _portIndex )

/*****************************************************************************/
/*                                                                           */
/* Local Defines                                                             */
/*                                                                           */
/*****************************************************************************/

/*DDR address for RDP usage*/
static void * ddr_tm_base_address_phys;
static void * ddr_multicast_base_address_phys;

static S_DPI_CFG *pDpiCfg = NULL;
static uint32_t initDone = 0;

/* route addresses (for both TX & RX) */
static const uint8_t bbh_route_address_dma[DRV_BBH_NUMBER_OF_PORTS]=
{
    0xC, 0xD, 0xE, 0xD, 0xE, 0xF, 0xF,0xF
};

static const uint8_t bbh_route_address_bpm[DRV_BBH_NUMBER_OF_PORTS]=
{
    0x14, 0x15, 0x16, 0x15, 0x16, 0x17, 0x17,0x17
};

static const uint8_t bbh_route_address_sdma[DRV_BBH_NUMBER_OF_PORTS]=
{
    0x1C, 0x1D, 0x1E, 0x1D, 0x1E, 0x1F, 0x1F,0
};

static const uint8_t bbh_route_address_sbpm[DRV_BBH_NUMBER_OF_PORTS]=
{
    0x34, 0x35, 0x36, 0x35, 0x36, 0x37, 0x37,0
};

static const uint8_t bbh_route_address_runner_0[DRV_BBH_NUMBER_OF_PORTS]=
{
    0x0, 0x1, 0x2, 0x1, 0x2, 0x3, 0x3,0x3
};

static const uint8_t bbh_route_address_runner_1[DRV_BBH_NUMBER_OF_PORTS]=
{
    0x8, 0x9, 0xA, 0x9, 0xA, 0xB, 0xB,0xB
};

static const uint8_t bbh_route_address_ih[DRV_BBH_NUMBER_OF_PORTS]=
{
    0x18, 0x19, 0x1A, 0x11, 0x12, 0x13, 0x13,0x13
};

/* same values for DMA & SDMA */
static const uint8_t bbh_dma_and_sdma_read_requests_fifo_base_address[DRV_BBH_NUMBER_OF_PORTS]=
{
    0x0, 0x8, 0x10, 0x18, 0x20, 0x28, 0x28,0x28
};

/* IH Classes indexes & configurations */
typedef struct
{
    uint8_t class_index;

    DRV_IH_CLASS_CONFIG class_config;
} ih_class_cfg;

ih_class_cfg gs_ih_classes[] =
{
    {
        /* Wan Control */
        DRV_RDD_IH_CLASS_WAN_CONTROL_INDEX,
        {
            DRV_RDD_IH_CLASS_0_CLASS_SEARCH_1, /*class_search_1*/
            DRV_RDD_IH_CLASS_0_CLASS_SEARCH_2, /*class_search_2*/
            DRV_RDD_IH_CLASS_0_CLASS_SEARCH_3, /*class_search_3*/
            DRV_RDD_IH_CLASS_0_CLASS_SEARCH_4, /*class_search_4*/
            DRV_RDD_IH_CLASS_0_DESTINATION_PORT_EXTRACTION,  /*destination_port_extraction*/
            DRV_RDD_IH_CLASS_0_DROP_ON_MISS, /*drop_on_miss*/
            DRV_RDD_IH_CLASS_0_DSCP_TO_PBITS_TABLE_INDEX, /*dscp_to_tci_table_index*/
            DRV_RDD_IH_CLASS_0_DIRECT_MODE_DEFAULT, /*direct_mode_default*/
            DRV_RDD_IH_CLASS_0_DIRECT_MODE_OVERRIDE, /*direct_mode_override*/
            DRV_RDD_IH_CLASS_0_TARGET_MEMORY_DEFAULT, /*target_memory_default*/
            DRV_RDD_IH_CLASS_0_TARGET_MEMORY_OVERRIDE, /*target_memory_override*/
            DRV_RDD_IH_CLASS_0_INGRESS_QOS_DEFAULT, /*ingress_qos_default*/
            DRV_RDD_IH_CLASS_0_INGRESS_QOS_OVERRIDE, /*ingress_qos_override*/
            DRV_RDD_IH_CLASS_0_TARGET_RUNNER_DEFAULT, /*target_runner_default*/
            DRV_RDD_IH_CLASS_0_TARGET_RUNNER_OVERRIDE_IN_DIRECT_MODE, /*target_runner_override_in_direct_mode*/
            DRV_RDD_IH_CLASS_0_TARGET_RUNNER_FOR_DIRECT_MODE, /*target_runner_for_direct_mode*/
            DRV_RDD_IH_CLASS_0_LOAD_BALANCING_ENABLE, /*load_balancing_enable*/
            DRV_RDD_IH_CLASS_0_PREFERENCE_LOAD_BALANCING_ENABLE /*preference_load_balancing_enable*/
        }
    },
    {
        /* IPTV */
        DRV_RDD_IH_CLASS_IPTV_INDEX,
        {
            DRV_RDD_IH_CLASS_1_CLASS_SEARCH_1,
            DRV_IH_CLASS_SEARCH_DISABLED,   /* iptv_lookup_method_mac */
            DRV_RDD_IH_CLASS_1_CLASS_SEARCH_3,
            DRV_RDD_IH_CLASS_1_CLASS_SEARCH_4,
            DRV_RDD_IH_CLASS_1_DESTINATION_PORT_EXTRACTION,
            DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_BASED_ON_SEARCH1,
            DRV_RDD_IH_CLASS_1_DSCP_TO_PBITS_TABLE_INDEX,
            DRV_RDD_IH_CLASS_1_DIRECT_MODE_DEFAULT,
            DRV_RDD_IH_CLASS_1_DIRECT_MODE_OVERRIDE,
            DRV_RDD_IH_CLASS_1_TARGET_MEMORY_DEFAULT,
            DRV_RDD_IH_CLASS_1_TARGET_MEMORY_OVERRIDE,
            DRV_RDD_IH_CLASS_1_INGRESS_QOS_DEFAULT,
            DRV_RDD_IH_CLASS_1_INGRESS_QOS_OVERRIDE,
            DRV_RDD_IH_CLASS_1_TARGET_RUNNER_DEFAULT,
            DRV_RDD_IH_CLASS_1_TARGET_RUNNER_OVERRIDE_IN_DIRECT_MODE,
            DRV_RDD_IH_CLASS_1_TARGET_RUNNER_FOR_DIRECT_MODE,
            DRV_RDD_IH_CLASS_1_LOAD_BALANCING_ENABLE,
            DRV_RDD_IH_CLASS_1_PREFERENCE_LOAD_BALANCING_ENABLE
        }
    },
    {
        /* PCIE */
        DRV_RDD_IH_CLASS_PCI_INDEX,
        {
            DRV_RDD_IH_LOOKUP_TABLE_MAC_DA_INDEX, /* da_lookup_required */
            DRV_RDD_IH_LOOKUP_TABLE_MAC_SA_INDEX, /* sa_lookup_required */
            DRV_RDD_IH_CLASS_2_CLASS_SEARCH_3,
            DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_4,
            DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_BASED_ON_SEARCH1, /* da_lookup_required */
            DRV_RDD_IH_CLASS_2_DROP_ON_MISS,
            DRV_RDD_IH_CLASS_2_DSCP_TO_PBITS_TABLE_INDEX,
            DRV_RDD_IH_CLASS_2_DIRECT_MODE_DEFAULT,
            DRV_RDD_IH_CLASS_2_DIRECT_MODE_OVERRIDE,
            DRV_RDD_IH_CLASS_2_TARGET_MEMORY_DEFAULT,
            DRV_RDD_IH_CLASS_2_TARGET_MEMORY_OVERRIDE,
            DRV_RDD_IH_CLASS_2_INGRESS_QOS_DEFAULT,
            DRV_RDD_IH_CLASS_2_INGRESS_QOS_OVERRIDE,
            DRV_RDD_IH_CLASS_2_TARGET_RUNNER_DEFAULT,
            DRV_RDD_IH_CLASS_2_TARGET_RUNNER_OVERRIDE_IN_DIRECT_MODE,
            DRV_RDD_IH_CLASS_2_TARGET_RUNNER_FOR_DIRECT_MODE,
            DRV_RDD_IH_CLASS_2_LOAD_BALANCING_ENABLE,
            DRV_RDD_IH_CLASS_2_PREFERENCE_LOAD_BALANCING_ENABLE
        }
    },
    {
        /* WAN bridged high */
        DRV_RDD_IH_CLASS_WAN_BRIDGED_HIGH_INDEX,
        {
            DRV_RDD_IH_LOOKUP_TABLE_MAC_DA_INDEX, /* da_lookup_required */
            DRV_RDD_IH_LOOKUP_TABLE_MAC_SA_INDEX, /* sa_lookup_required */
            DRV_RDD_IH_CLASS_8_CLASS_SEARCH_3,
            DRV_RDD_IH_CLASS_8_CLASS_SEARCH_4,
            DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_BASED_ON_SEARCH1, /* da_lookup_required */
            DRV_RDD_IH_CLASS_8_DROP_ON_MISS,
            DRV_RDD_IH_CLASS_8_DSCP_TO_PBITS_TABLE_INDEX,
            DRV_RDD_IH_CLASS_8_DIRECT_MODE_DEFAULT,
            DRV_RDD_IH_CLASS_8_DIRECT_MODE_OVERRIDE,
            DRV_RDD_IH_CLASS_8_TARGET_MEMORY_DEFAULT,
            DRV_RDD_IH_CLASS_8_TARGET_MEMORY_OVERRIDE,
            DRV_RDD_IH_CLASS_8_INGRESS_QOS_DEFAULT,
            DRV_RDD_IH_CLASS_8_INGRESS_QOS_OVERRIDE,
            DRV_RDD_IH_CLASS_8_TARGET_RUNNER_DEFAULT,
            DRV_RDD_IH_CLASS_8_TARGET_RUNNER_OVERRIDE_IN_DIRECT_MODE,
            DRV_RDD_IH_CLASS_8_TARGET_RUNNER_FOR_DIRECT_MODE,
            DRV_RDD_IH_CLASS_8_LOAD_BALANCING_ENABLE,
            DRV_RDD_IH_CLASS_8_PREFERENCE_LOAD_BALANCING_ENABLE
        }
    },
    {
        /* WAN bridged low */
        DRV_RDD_IH_CLASS_WAN_BRIDGED_LOW_INDEX,
        {
            DRV_RDD_IH_LOOKUP_TABLE_MAC_DA_INDEX, /* da_lookup_required */
            DRV_RDD_IH_LOOKUP_TABLE_MAC_SA_INDEX, /* sa_lookup_required */
            DRV_RDD_IH_CLASS_9_CLASS_SEARCH_3,
            DRV_RDD_IH_CLASS_9_CLASS_SEARCH_4,
            DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_BASED_ON_SEARCH1, /* da_lookup_required */
            DRV_RDD_IH_CLASS_9_DROP_ON_MISS,
            DRV_RDD_IH_CLASS_9_DSCP_TO_PBITS_TABLE_INDEX,
            DRV_RDD_IH_CLASS_9_DIRECT_MODE_DEFAULT,
            DRV_RDD_IH_CLASS_9_DIRECT_MODE_OVERRIDE,
            DRV_RDD_IH_CLASS_9_TARGET_MEMORY_DEFAULT,
            DRV_RDD_IH_CLASS_9_TARGET_MEMORY_OVERRIDE,
            DRV_RDD_IH_CLASS_9_INGRESS_QOS_DEFAULT,
            DRV_RDD_IH_CLASS_9_INGRESS_QOS_OVERRIDE,
            DRV_RDD_IH_CLASS_9_TARGET_RUNNER_DEFAULT,
            DRV_RDD_IH_CLASS_9_TARGET_RUNNER_OVERRIDE_IN_DIRECT_MODE,
            DRV_RDD_IH_CLASS_9_TARGET_RUNNER_FOR_DIRECT_MODE,
            DRV_RDD_IH_CLASS_9_LOAD_BALANCING_ENABLE,
            DRV_RDD_IH_CLASS_9_PREFERENCE_LOAD_BALANCING_ENABLE
        }
    },
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
    {
        /* LAN bridged eth2 */
        DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH2_INDEX,
        {
            DRV_RDD_IH_LOOKUP_TABLE_MAC_DA_INDEX, /* da_lookup_required */
            DRV_RDD_IH_LOOKUP_TABLE_MAC_SA_INDEX, /* sa_lookup_required */
            DRV_RDD_IH_CLASS_12_CLASS_SEARCH_3,
            DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_4,
            DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_BASED_ON_SEARCH1, /* da_lookup_required */
            DRV_RDD_IH_CLASS_12_DROP_ON_MISS,
            DRV_RDD_IH_CLASS_12_DSCP_TO_PBITS_TABLE_INDEX,
            DRV_RDD_IH_CLASS_12_DIRECT_MODE_DEFAULT,
            DRV_RDD_IH_CLASS_12_DIRECT_MODE_OVERRIDE,
            DRV_RDD_IH_CLASS_12_TARGET_MEMORY_DEFAULT,
            DRV_RDD_IH_CLASS_12_TARGET_MEMORY_OVERRIDE,
            DRV_RDD_IH_CLASS_12_INGRESS_QOS_DEFAULT,
            DRV_RDD_IH_CLASS_12_INGRESS_QOS_OVERRIDE,
            DRV_RDD_IH_CLASS_12_TARGET_RUNNER_DEFAULT,
            DRV_RDD_IH_CLASS_12_TARGET_RUNNER_OVERRIDE_IN_DIRECT_MODE,
            DRV_RDD_IH_CLASS_12_TARGET_RUNNER_FOR_DIRECT_MODE,
            DRV_RDD_IH_CLASS_12_LOAD_BALANCING_ENABLE,
            DRV_RDD_IH_CLASS_12_PREFERENCE_LOAD_BALANCING_ENABLE
        }
    },
    {
        /* LAN bridged eth3 */
        DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH3_INDEX,
        {
            DRV_RDD_IH_LOOKUP_TABLE_MAC_DA_INDEX, /* da_lookup_required */
            DRV_RDD_IH_LOOKUP_TABLE_MAC_SA_INDEX, /*sa_lookup_required*/
            DRV_RDD_IH_CLASS_13_CLASS_SEARCH_3,
            DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_4,
            DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_BASED_ON_SEARCH1, /* da_lookup_required */
            DRV_RDD_IH_CLASS_13_DROP_ON_MISS,
            DRV_RDD_IH_CLASS_13_DSCP_TO_PBITS_TABLE_INDEX,
            DRV_RDD_IH_CLASS_13_DIRECT_MODE_DEFAULT,
            DRV_RDD_IH_CLASS_13_DIRECT_MODE_OVERRIDE,
            DRV_RDD_IH_CLASS_13_TARGET_MEMORY_DEFAULT,
            DRV_RDD_IH_CLASS_13_TARGET_MEMORY_OVERRIDE,
            DRV_RDD_IH_CLASS_13_INGRESS_QOS_DEFAULT,
            DRV_RDD_IH_CLASS_13_INGRESS_QOS_OVERRIDE,
            DRV_RDD_IH_CLASS_13_TARGET_RUNNER_DEFAULT,
            DRV_RDD_IH_CLASS_13_TARGET_RUNNER_OVERRIDE_IN_DIRECT_MODE,
            DRV_RDD_IH_CLASS_13_TARGET_RUNNER_FOR_DIRECT_MODE,
            DRV_RDD_IH_CLASS_13_LOAD_BALANCING_ENABLE,
            DRV_RDD_IH_CLASS_13_PREFERENCE_LOAD_BALANCING_ENABLE
        }
    },
    {
        /* LAN bridged eth4 */
        DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH4_INDEX,
        {
            DRV_RDD_IH_LOOKUP_TABLE_MAC_DA_INDEX, /* da_lookup_required */
            DRV_RDD_IH_LOOKUP_TABLE_MAC_SA_INDEX, /* sa_lookup_required */
            DRV_RDD_IH_CLASS_14_CLASS_SEARCH_3,
            DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_4,
            DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_BASED_ON_SEARCH1, /* da_lookup_required */
            DRV_RDD_IH_CLASS_14_DROP_ON_MISS,
            DRV_RDD_IH_CLASS_14_DSCP_TO_PBITS_TABLE_INDEX,
            DRV_RDD_IH_CLASS_14_DIRECT_MODE_DEFAULT,
            DRV_RDD_IH_CLASS_14_DIRECT_MODE_OVERRIDE,
            DRV_RDD_IH_CLASS_14_TARGET_MEMORY_DEFAULT,
            DRV_RDD_IH_CLASS_14_TARGET_MEMORY_OVERRIDE,
            DRV_RDD_IH_CLASS_14_INGRESS_QOS_DEFAULT,
            DRV_RDD_IH_CLASS_14_INGRESS_QOS_OVERRIDE,
            DRV_RDD_IH_CLASS_14_TARGET_RUNNER_DEFAULT,
            DRV_RDD_IH_CLASS_14_TARGET_RUNNER_OVERRIDE_IN_DIRECT_MODE,
            DRV_RDD_IH_CLASS_14_TARGET_RUNNER_FOR_DIRECT_MODE,
            DRV_RDD_IH_CLASS_14_LOAD_BALANCING_ENABLE,
            DRV_RDD_IH_CLASS_14_PREFERENCE_LOAD_BALANCING_ENABLE
        }
    }
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
static const rdd_emac_id_t bbh_to_rdd_emac_map[] =
{
    RDD_EMAC_ID_0,
    RDD_EMAC_ID_1,
    RDD_EMAC_ID_2,
    RDD_EMAC_ID_3,
    RDD_EMAC_ID_4,
};

static const uint32_t bbh_to_rdd_eth_thread[] =
{
    LAN0_TX_THREAD_NUMBER,
    LAN1_TX_THREAD_NUMBER,
    LAN2_TX_THREAD_NUMBER,
    LAN3_TX_THREAD_NUMBER,
    LAN4_TX_THREAD_NUMBER,

};
/******************************************************************************/
/* There are 8 IH ingress queues. This enumeration defines, for each ingress  */
/* queue, which physical source port it belongs to.                           */
/******************************************************************************/
typedef enum
{
    IH_INGRESS_QUEUE_0_ETH0,
    IH_INGRESS_QUEUE_1_ETH1,
    IH_INGRESS_QUEUE_2_ETH2,
    IH_INGRESS_QUEUE_3_ETH3,
    IH_INGRESS_QUEUE_4_ETH4,
    IH_INGRESS_QUEUE_5_GPON_OR_AE,
    IH_INGRESS_QUEUE_6_RUNNER_A,
    IH_INGRESS_QUEUE_7_RUNNER_B,

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

int f_validate_ddr_address(uint32_t address);

static void f_map_wan_port_from_wan_bbh(uint32_t bbh_port, uint32_t *dma_reg)
{
    uint32_t ret;

    if (bbh_port >= DRV_BBH_EMAC_5)
        ret = DRV_BBH_EMAC_5;
    else
        ret = bbh_port;
    
    *dma_reg = 1 << ret;
    
    return;
}

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

static void fi_dma_configure_memory_allocation ( E_BB_MODULE module_id,
    E_DMA_PERIPHERAL peripheral_id,
    uint32_t data_memory_offset_address,
    uint32_t cd_memory_offset_address,
    uint32_t number_of_buffers )
{
    DMA_REGS_CONFIG_MALLOC config_malloc ;

    DMA_REGS_CONFIG_MALLOC_READ( module_id, peripheral_id, config_malloc ) ;
    config_malloc.datatoffset = data_memory_offset_address ;
    config_malloc.cdoffset = cd_memory_offset_address ;
    config_malloc.numofbuff = number_of_buffers ;
    DMA_REGS_CONFIG_MALLOC_WRITE( module_id, peripheral_id, config_malloc ) ;
}

static void _f_initialize_dma_sdma(DRV_BBH_PORT_INDEX bbh_emac)
{
    E_DMA_PERIPHERAL peripheral_id = (bbh_emac == pDpiCfg->wan_bbh && pDpiCfg->wan_bbh > DRV_BBH_EMAC_4) ?
        DMA_PERIPHERAL_EMAC_AE_GPON_EPON : bbh_emac;
    DMA_REGS_CONFIG_U_THRESH dma_thresh;
    DMA_REGS_CONFIG_PRI dma_pri_cfg;

    dma_thresh.out_of_u = 1;
    dma_thresh.into_u = 5;

    fi_dma_configure_memory_allocation(BB_MODULE_DMA,
            peripheral_id,
            bbh_rx_dma_data_fifo_base_address[bbh_emac],
            bbh_rx_dma_chunk_descriptor_fifo_base_address[bbh_emac],
            bbh_rx_dma_data_and_chunk_descriptor_fifos_size[bbh_emac]);
    fi_dma_configure_memory_allocation(BB_MODULE_SDMA,
            peripheral_id,
            bbh_rx_sdma_data_fifo_base_address[bbh_emac],
            bbh_rx_sdma_chunk_descriptor_fifo_base_address[bbh_emac],
            bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[bbh_emac]);

    DMA_REGS_CONFIG_U_THRESH_WRITE(BB_MODULE_DMA, peripheral_id, dma_thresh);

    /* Make the EPON wan with highest priority */
    dma_pri_cfg.r1 = 0;
    dma_pri_cfg.rxpri = DMA_REGS_CONFIG_PRI_RXPRI_LOW_VALUE;
    dma_pri_cfg.txpri = (bbh_emac==DRV_BBH_EPON) ? 
    	DMA_REGS_CONFIG_PRI_TXPRI_HIGH_VALUE : 
    	DMA_REGS_CONFIG_PRI_TXPRI_LOW_VALUE;
	
    DMA_REGS_0_CONFIG_PRI_WRITE_I(dma_pri_cfg, peripheral_id);
}

static void f_initialize_dma_sdma(int fiber)
{
    uint32_t dma_mem_sel_reg, bbh_emac;

    if (!fiber)
    {
        for (bbh_emac = DRV_BBH_EMAC_0; bbh_emac < DRV_BBH_EMAC_5; bbh_emac++)
            _f_initialize_dma_sdma(bbh_emac);
    }
    else
        _f_initialize_dma_sdma(pDpiCfg->wan_bbh);

    if ((fiber && pDpiCfg->wan_bbh >= DRV_BBH_EMAC_5) || (!fiber && pDpiCfg->wan_bbh < DRV_BBH_EMAC_5))
    {
        /* Write the DMA configuration */
        f_map_wan_port_from_wan_bbh(pDpiCfg->wan_bbh, &dma_mem_sel_reg);
        WRITE_32(DMA_REGS_0_MEM_SET, dma_mem_sel_reg);
    }
	
}

static E_DPI_RC f_initialize_bbh_dma_sdma_related_arrays(int with_fiber)
{
    int i;
    uint8_t dma_base_address = 0;
    uint8_t sdma_base_address = 0;
    uint8_t dma_total_chunks = 0;
    uint8_t sdma_total_chunks = 0;
    uint32_t wan_emac = pDpiCfg->wan_bbh;

    /* handle emac ports */
    for (i = 0; i <= DRV_BBH_EMAC_4; i++)
    {
        if (i == wan_emac)
            continue;

        bbh_rx_dma_data_and_chunk_descriptor_fifos_size[i] = (i == DRV_BBH_EMAC_4) ? BBH_RX_DMA_FIFOS_SIZE_RGMII_LAN_DMA :
            BBH_RX_DMA_FIFOS_SIZE_LAN_DMA;
        bbh_rx_dma_exclusive_threshold[i] = (i == DRV_BBH_EMAC_4) ? BBH_RX_DMA_EXCLUSIVE_THRESHOLD_RGMII_LAN_DMA:
            BBH_RX_DMA_EXCLUSIVE_THRESHOLD_LAN_DMA;
        bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[i] = BBH_RX_SDMA_FIFOS_SIZE_LAN;
        bbh_rx_sdma_exclusive_threshold[i] = BBH_RX_SDMA_EXCLUSIVE_THRESHOLD_LAN;

        bbh_rx_dma_data_fifo_base_address[i] = dma_base_address;
        bbh_rx_dma_chunk_descriptor_fifo_base_address[i] = dma_base_address;
        bbh_rx_sdma_data_fifo_base_address[i] = sdma_base_address;
        bbh_rx_sdma_chunk_descriptor_fifo_base_address[i] = sdma_base_address;


        dma_base_address += bbh_rx_dma_data_and_chunk_descriptor_fifos_size[i];
        sdma_base_address += bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[i];
    }

    if (with_fiber || (pDpiCfg->wan_bbh < DRV_BBH_EMAC_5))
    {
        bbh_rx_dma_data_and_chunk_descriptor_fifos_size[wan_emac] = BBH_RX_DMA_FIFOS_SIZE_WAN_DMA;
        bbh_rx_dma_exclusive_threshold[wan_emac] = BBH_RX_DMA_EXCLUSIVE_THRESHOLD_WAN_DMA;
        bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[wan_emac] = BBH_RX_SDMA_FIFOS_SIZE_WAN;
        bbh_rx_sdma_exclusive_threshold[wan_emac] = BBH_RX_SDMA_EXCLUSIVE_THRESHOLD_WAN;

        bbh_rx_dma_data_fifo_base_address[wan_emac] = 0;
        bbh_rx_dma_chunk_descriptor_fifo_base_address[wan_emac] = 0;
        bbh_rx_sdma_data_fifo_base_address[wan_emac] = sdma_base_address;
        bbh_rx_sdma_chunk_descriptor_fifo_base_address[wan_emac] = sdma_base_address;

        dma_base_address += bbh_rx_dma_data_and_chunk_descriptor_fifos_size[wan_emac];
        sdma_base_address += bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[wan_emac];
    }

    dma_total_chunks = BBH_RX_DMA_TOTAL_NUMBER_OF_CHUNK_DESCRIPTORS + BBH_RX_DMA_FIFOS_SIZE_WAN_DMA;
    sdma_total_chunks = BBH_RX_SDMA_TOTAL_NUMBER_OF_CHUNK_DESCRIPTORS + BBH_RX_SDMA_FIFOS_SIZE_WAN;

    /* check that we didn't overrun */
    if (dma_base_address > dma_total_chunks ||
        sdma_base_address > sdma_total_chunks)
    {
        err("%s:(%d) error",__FUNCTION__,__LINE__);
        return DPI_RC_ERROR;
    }
    return DPI_RC_OK;
}

static uint16_t f_get_bbh_rx_pd_fifo_base_address_normal_queue_in_8_byte(DRV_BBH_PORT_INDEX port_index)
{
    if (BBH_PORT_IS_WAN(port_index))
        return MS_BYTE_TO_8_BYTE_RESOLUTION(WAN_RX_NORMAL_DESCRIPTORS_ADDRESS); 
    else
        switch (port_index)
        {
        case DRV_BBH_EMAC_0:
            return MS_BYTE_TO_8_BYTE_RESOLUTION(LAN0_RX_DESCRIPTORS_ADDRESS);

        case DRV_BBH_EMAC_1:
            return MS_BYTE_TO_8_BYTE_RESOLUTION(LAN1_RX_DESCRIPTORS_ADDRESS);

        case DRV_BBH_EMAC_2:
            return MS_BYTE_TO_8_BYTE_RESOLUTION(LAN2_RX_DESCRIPTORS_ADDRESS);

        case DRV_BBH_EMAC_3:
            return MS_BYTE_TO_8_BYTE_RESOLUTION(LAN3_RX_DESCRIPTORS_ADDRESS);

        case DRV_BBH_EMAC_4:
            return MS_BYTE_TO_8_BYTE_RESOLUTION(LAN4_RX_DESCRIPTORS_ADDRESS);
        default:
            break;
        }
    return 0;
}
static uint16_t f_get_bbh_rx_pd_fifo_base_address_direct_queue_in_8_byte(DRV_BBH_PORT_INDEX port_index)
{
    if (BBH_PORT_IS_WAN(port_index))
        return MS_BYTE_TO_8_BYTE_RESOLUTION(GPON_RX_DIRECT_DESCRIPTORS_ADDRESS);
    else
        switch (port_index)
        {
        case DRV_BBH_EMAC_0:
            return MS_BYTE_TO_8_BYTE_RESOLUTION(LAN0_RX_DIRECT_DESCRIPTORS_ADDRESS);

        case DRV_BBH_EMAC_1:
            return MS_BYTE_TO_8_BYTE_RESOLUTION(LAN1_RX_DIRECT_DESCRIPTORS_ADDRESS);

        case DRV_BBH_EMAC_2:
            return MS_BYTE_TO_8_BYTE_RESOLUTION(LAN2_RX_DIRECT_DESCRIPTORS_ADDRESS);

        case DRV_BBH_EMAC_3:
            return MS_BYTE_TO_8_BYTE_RESOLUTION(LAN3_RX_DIRECT_DESCRIPTORS_ADDRESS);

        case DRV_BBH_EMAC_4:
            return MS_BYTE_TO_8_BYTE_RESOLUTION(LAN4_RX_DIRECT_DESCRIPTORS_ADDRESS);
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
#ifdef LEGACY_RDP
            return WAN_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER;
#else
            return WAN_RX_DISPATCH_THREAD_NUMBER;
#endif
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
            if (port_index >= DRV_BBH_EMAC_0 && port_index <= DRV_BBH_EMAC_4)
            {
#ifdef G9991
                if (port_index == pDpiCfg->g9991_debug_port)
                    return LAN_DIRECT_TO_CPU_THREAD_NUMBER;
#endif
#ifdef LEGACY_RDP
                return LAN0_FILTERS_AND_CLASSIFICATION_THREAD_NUMBER + port_index;
#else
                return LAN0_RX_DISPATCH_THREAD_NUMBER + port_index;
#endif
            }
        }
    }
    return 0;
}

static uint8_t f_bbh_rx_runner_task_direct_queue(DRV_IH_RUNNER_CLUSTER runner_cluster, DRV_BBH_PORT_INDEX portIndex)
{
    if ( BBH_PORT_IS_WAN( portIndex ))
    {
        if (runner_cluster == DRV_IH_RUNNER_CLUSTER_A)
        {
            return WAN_DIRECT_THREAD_NUMBER;
        }
        else
        {
            return WAN_TO_WAN_THREAD_NUMBER;
        }
    }

    /* the port is LAN */
    if (runner_cluster == DRV_IH_RUNNER_CLUSTER_A)
    {
        switch (portIndex)
        {
        case DRV_BBH_EMAC_0:
            return LAN0_RX_DIRECT_RUNNER_A_TASK_NUMBER;

        case DRV_BBH_EMAC_1:
            return LAN1_RX_DIRECT_RUNNER_A_TASK_NUMBER;

        case DRV_BBH_EMAC_2:
            return LAN2_RX_DIRECT_RUNNER_A_TASK_NUMBER;

        case DRV_BBH_EMAC_3:
            return LAN3_RX_DIRECT_RUNNER_A_TASK_NUMBER;

        case DRV_BBH_EMAC_4:
            return LAN4_RX_DIRECT_RUNNER_A_TASK_NUMBER;

            /* we are not supposed to get here */
        default:
            break;
        }
    }/* runner B */
    else
    {
        switch (portIndex)
        {
        case DRV_BBH_EMAC_0:
            return LAN0_RX_DIRECT_RUNNER_B_TASK_NUMBER;

        case DRV_BBH_EMAC_1:
            return LAN1_RX_DIRECT_RUNNER_B_TASK_NUMBER;

        case DRV_BBH_EMAC_2:
            return LAN2_RX_DIRECT_RUNNER_B_TASK_NUMBER;

        case DRV_BBH_EMAC_3:
            return LAN3_RX_DIRECT_RUNNER_B_TASK_NUMBER;

        case DRV_BBH_EMAC_4:
            return LAN4_RX_DIRECT_RUNNER_B_TASK_NUMBER;

            /* we are not supposed to get here */
        default:
            break;
        }
    }
    return 0;
}

static DRV_BBH_DDR_BUFFER_SIZE f_rdpa_bbh_ddr_buffer_size(void)
{
    uint32_t  bpm_buffer_size =  pDpiCfg->bpm_buffer_size;

    switch (bpm_buffer_size)
    {
    case RDPA_BPM_BUFFER_2K:
        return  DRV_BBH_DDR_BUFFER_SIZE_2_KB;
    case RDPA_BPM_BUFFER_2_5K:
        return  DRV_BBH_DDR_BUFFER_SIZE_2_5_KB;
    case RDPA_BPM_BUFFER_4K:
        return  DRV_BBH_DDR_BUFFER_SIZE_4_KB;
    case RDPA_BPM_BUFFER_16K:
        return  DRV_BBH_DDR_BUFFER_SIZE_16_KB;
    }
    return  DRV_BBH_DDR_BUFFER_SIZE_2_KB;
}

static void f_initialize_bbh_of_emac_port(DRV_BBH_PORT_INDEX port_index)
{
    uint16_t mdu_mode_read_pointer_address_in_byte;
    uint32_t mdu_mode_read_pointer_address_in_byte_uint32;
    DRV_BBH_TX_CONFIGURATION bbh_tx_configuration ;
    DRV_BBH_RX_CONFIGURATION bbh_rx_configuration ;
    DRV_BBH_PER_FLOW_CONFIGURATION per_flow_configuration ;
    uint16_t    lan_fifo_size;
    uint16_t    lan_exc_thresh;

    /*** BBH TX ***/
    bbh_tx_configuration.dma_route_address = bbh_route_address_dma[port_index];
    bbh_tx_configuration.bpm_route_address = bbh_route_address_bpm[port_index];
    bbh_tx_configuration.sdma_route_address = bbh_route_address_sdma[port_index];
    bbh_tx_configuration.sbpm_route_address = bbh_route_address_sbpm[port_index];
    /* runner 0 is the one which handles TX  except for wan mode emac4*/
    if (BBH_PORT_IS_WAN(port_index))
    {
        bbh_tx_configuration.runner_route_address = bbh_route_address_runner_1[port_index];
    }
    else
    {
        bbh_tx_configuration.runner_route_address = bbh_route_address_runner_0[port_index];
    }

    bbh_tx_configuration.ddr_buffer_size = f_rdpa_bbh_ddr_buffer_size();
    bbh_tx_configuration.payload_offset_resolution = DRV_BBH_PAYLOAD_OFFSET_RESOLUTION_2_B;

    bbh_tx_configuration.multicast_headers_base_address_in_byte = (uint32_t)ddr_multicast_base_address_phys;//


    if (BBH_PORT_IS_WAN(port_index))
    {
        bbh_tx_configuration.multicast_header_size = BBH_MULTICAST_HEADER_SIZE_FOR_LAN_PORT;
        /* this is the task also in Gbe case */
        bbh_tx_configuration.task_0 = WAN_TX_THREAD_NUMBER;
        bbh_tx_configuration.skb_address = MS_BYTE_TO_8_BYTE_RESOLUTION(GPON_ABSOLUTE_TX_BBH_COUNTER_ADDRESS);
    }
    else
        /*we will reach here in case the BBH port in lan ports 0 - 4*/
    {
        bbh_tx_configuration.multicast_header_size = BBH_MULTICAST_HEADER_SIZE_FOR_LAN_PORT;

        bbh_tx_configuration.skb_address = MS_BYTE_TO_8_BYTE_RESOLUTION(EMAC_ABSOLUTE_TX_BBH_COUNTER_ADDRESS) + bbh_to_rdd_emac_map[port_index];

        bbh_tx_configuration.task_0 = bbh_to_rdd_eth_thread[port_index];
    }

    /* other task numbers are irrelevant (relevant for GPON only) */
    bbh_tx_configuration.task_1 = 0;
    bbh_tx_configuration.task_2 = 0;
    bbh_tx_configuration.task_3 = 0;
    bbh_tx_configuration.task_4 = 0;
    bbh_tx_configuration.task_5 = 0;
    bbh_tx_configuration.task_6 = 0;
    bbh_tx_configuration.task_7 = 0;
    bbh_tx_configuration.task_8_39 = 0;

    if (BBH_PORT_IS_WAN(port_index))
    {
        bbh_tx_configuration.mdu_mode_enable = 0;
        /* irrelevant in this case */
        bbh_tx_configuration.mdu_mode_read_pointer_address_in_8_byte = 0;
    }
    else
    {
#ifndef G9991
        bbh_tx_configuration.mdu_mode_enable = 1;
#else
        bbh_tx_configuration.mdu_mode_enable = 0;
#endif

        rdd_mdu_mode_pointer_get(bbh_to_rdd_emac_map[port_index], &mdu_mode_read_pointer_address_in_byte);

        mdu_mode_read_pointer_address_in_byte_uint32 = mdu_mode_read_pointer_address_in_byte;

        /* after division, this will be back a 16 bit number */
        bbh_tx_configuration.mdu_mode_read_pointer_address_in_8_byte = (uint16_t)MS_BYTE_TO_8_BYTE_RESOLUTION(mdu_mode_read_pointer_address_in_byte_uint32);
    }

    /* For Ethernet port working in MDU mode, PD FIFO size should be configured to 4 (and not 8). */
    bbh_tx_configuration.pd_fifo_size_0 =  (bbh_tx_configuration.mdu_mode_enable == 1) ? BBH_TX_EMAC_PD_FIFO_SIZE_MDU_MODE_ENABLED : BBH_TX_EMAC_PD_FIFO_SIZE_MDU_MODE_DISABLED;

    /* other FIFOs are irrelevant (relevant for GPON only) */
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

    /* other FIFOs are irrelevant (relevant for GPON only) */
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
    bbh_tx_configuration.epnurgnt = BBH_TX_CONFIGURATIONS_DMACFG_TX_EPNURGNT_NORMAL_VALUE_RESET_VALUE;
    bbh_tx_configuration.sdma_read_requests_fifo_base_address = bbh_dma_and_sdma_read_requests_fifo_base_address[port_index];
    bbh_tx_configuration.sdma_read_requests_maximal_number = BBH_TX_CONFIGURATIONS_SDMACFG_TX_MAXREQ_MAX_VALUE_VALUE_RESET_VALUE;

    /* irrelevant in EMAC case */
    bbh_tx_configuration.tcont_address_in_8_byte = 0;

    bbh_tx_configuration.ddr_tm_base_address = (uint32_t)ddr_tm_base_address_phys;

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

    bbh_rx_configuration.ddr_buffer_size = f_rdpa_bbh_ddr_buffer_size();
    bbh_rx_configuration.ddr_tm_base_address = (uint32_t)ddr_tm_base_address_phys;
    bbh_rx_configuration.pd_fifo_base_address_normal_queue_in_8_byte = f_get_bbh_rx_pd_fifo_base_address_normal_queue_in_8_byte(port_index);
    bbh_rx_configuration.pd_fifo_base_address_direct_queue_in_8_byte = f_get_bbh_rx_pd_fifo_base_address_direct_queue_in_8_byte(port_index);
    bbh_rx_configuration.pd_fifo_size_normal_queue = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    bbh_rx_configuration.pd_fifo_size_direct_queue = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    bbh_rx_configuration.runner_0_task_normal_queue = f_bbh_rx_runner_task_normal_queue (DRV_IH_RUNNER_CLUSTER_A, port_index);
    bbh_rx_configuration.runner_0_task_direct_queue = f_bbh_rx_runner_task_direct_queue (DRV_IH_RUNNER_CLUSTER_A, port_index);
    bbh_rx_configuration.runner_1_task_normal_queue = f_bbh_rx_runner_task_normal_queue (DRV_IH_RUNNER_CLUSTER_B, port_index);
    bbh_rx_configuration.runner_1_task_direct_queue = f_bbh_rx_runner_task_direct_queue (DRV_IH_RUNNER_CLUSTER_B, port_index);

    if(BBH_PORT_IS_WAN(port_index))
    {
        lan_fifo_size  = BBH_RX_DMA_FIFOS_SIZE_WAN_GPON;
        lan_exc_thresh = BBH_RX_DMA_EXCLUSIVE_THRESHOLD_WAN_BBH;
    }
    else if(port_index == DRV_BBH_EMAC_4)
    {
        lan_fifo_size  = BBH_RX_DMA_FIFOS_SIZE_RGMII_LAN_BBH;
        lan_exc_thresh = BBH_RX_DMA_EXCLUSIVE_THRESHOLD_RGMII_LAN_BBH;
    }
    else
    {
        lan_fifo_size  = BBH_RX_DMA_FIFOS_SIZE_LAN_BBH;
        lan_exc_thresh = BBH_RX_DMA_EXCLUSIVE_THRESHOLD_LAN_BBH;
    }

    bbh_rx_configuration.dma_data_fifo_base_address = bbh_rx_dma_data_fifo_base_address[port_index];
    bbh_rx_configuration.dma_chunk_descriptor_fifo_base_address = bbh_rx_configuration.dma_data_fifo_base_address;
    bbh_rx_configuration.sdma_data_fifo_base_address = bbh_rx_sdma_data_fifo_base_address[port_index];
    bbh_rx_configuration.sdma_chunk_descriptor_fifo_base_address = bbh_rx_sdma_chunk_descriptor_fifo_base_address[port_index ];
    bbh_rx_configuration.dma_data_and_chunk_descriptor_fifos_size = lan_fifo_size;
    bbh_rx_configuration.dma_exclusive_threshold =  lan_exc_thresh;
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
    bbh_rx_configuration.drop_triggers_bitmask = DRV_BBH_RX_DROP_TRIGGER_SBPM_IS_IN_EXCLUSIVE_STATE;

    /* following configuration is irrelevant in EMAC case */
    bbh_rx_configuration.flows_32_255_group_divider = BBH_RX_FLOWS_32_255_GROUP_DIVIDER;
    bbh_rx_configuration.ploam_default_ih_class = 0;
    bbh_rx_configuration.ploam_ih_class_override = 0;

    fi_bl_drv_bbh_rx_set_configuration(port_index, &bbh_rx_configuration);

    /* bbh_rx_set_per_flow_configuration */
    per_flow_configuration.minimum_packet_size_selection = BBH_RX_ETH_MIN_PKT_SIZE_SELECTION_INDEX;
    per_flow_configuration.maximum_packet_size_selection = BBH_RX_MAX_PKT_SIZE_SELECTION_INDEX;

    if (BBH_PORT_IS_WAN(port_index) )
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
        case DRV_BBH_EMAC_2:
            per_flow_configuration.default_ih_class = DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH2_INDEX;
            break;
        case DRV_BBH_EMAC_3:
            per_flow_configuration.default_ih_class = DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH3_INDEX;
            break;
        case DRV_BBH_EMAC_4:
            /* if we are here, emac 4 is not the WAN */
            per_flow_configuration.default_ih_class = DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH4_INDEX;
            break;
            /*yoni itah: the next situation is impossible in Oren, AE port is never lan port*/
            //        case DRV_BBH_EMAC_5:
            //            /* if we are here, we are in GBE mode and emac 5 is not the WAN, meaning emac 4 is the WAN.
            //               in this case emac 5 will have the class LAN_BRIDGED_ETH4 */
            //            per_flow_configuration.default_ih_class = DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH4_INDEX;
            //            break;
        default:
            return;
            break;
        }
    }

    /* in EMAC, only flow 0 is relevant */
    fi_bl_drv_bbh_rx_set_per_flow_configuration(port_index, 0, &per_flow_configuration);

    return;
}

/* this function uses deafault values of IH ingress queues and update bbh-IH queus bitmask. */
static int fi_ih_ingress_buffers_bitmask_set(void)
{
    uint32_t i;
    DRV_IH_INGRESS_QUEUE_CONFIG ingress_queue_config;
    
    for (i = IH_INGRESS_QUEUE_0_ETH0; i <= IH_INGRESS_QUEUE_4_ETH4; i++)
    {
        fi_bl_drv_ih_get_ingress_queue_configuration(i, &ingress_queue_config);
        f_update_bbh_ih_ingress_buffers_bitmask((DRV_BBH_PORT_INDEX)i, ingress_queue_config.base_location, ingress_queue_config.size);
    }

    /* configure WAN port*/
    fi_bl_drv_ih_get_ingress_queue_configuration(IH_INGRESS_QUEUE_5_GPON_OR_AE, &ingress_queue_config);
    f_update_bbh_ih_ingress_buffers_bitmask(DRV_BBH_GPON, ingress_queue_config.base_location, 
                                            ingress_queue_config.size);
    f_update_bbh_ih_ingress_buffers_bitmask(DRV_BBH_EPON, ingress_queue_config.base_location, 
                                            ingress_queue_config.size);
    f_update_bbh_ih_ingress_buffers_bitmask(DRV_BBH_EMAC_5, ingress_queue_config.base_location, 
                                            ingress_queue_config.size);

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

    if ( source_port < DRV_IH_TARGET_MATRIX_SOURCE_PORT_GPON  && !BBH_PORT_IS_WAN(source_port))
    {
        return 1;
    }
    return 0;
}

static uint32_t is_matrix_dest_lan(DRV_IH_TARGET_MATRIX_DESTINATION_PORT source_port)
{
    if (source_port < DRV_IH_TARGET_MATRIX_DESTINATION_PORT_GPON && !BBH_PORT_IS_WAN(source_port))
    {
        return 1;
    }
    return 0;
}

static uint32_t is_matrix_source_wan(DRV_IH_TARGET_MATRIX_SOURCE_PORT source_port)
{
	if (source_port == DRV_IH_TARGET_MATRIX_SOURCE_PORT_GPON &&
        (pDpiCfg->wan_bbh == DRV_BBH_EMAC_5 || pDpiCfg->wan_bbh == DRV_BBH_GPON
        || pDpiCfg->wan_bbh == DRV_BBH_EPON))
    {
        return 1;
    }
    else if (source_port < DRV_IH_TARGET_MATRIX_SOURCE_PORT_GPON && BBH_PORT_IS_WAN(source_port))
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
    else if (is_matrix_source_wan(source_port)&& is_matrix_source_wan(destination_port))
    {
        return 1;
    }
    return 0;
}

static void sbpm_drv_init(void)
{
#define SBPM_WAN_LAN_GROUP(emac) ((BBH_PORT_IS_WAN(emac)) ? \
DRV_SBPM_USER_GROUP_0 : DRV_SBPM_USER_GROUP_1)
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
#ifdef G9991
    sbpm_ug_configuration.ug_arr[0].threshold = SBPM_G9991_DEFAULT_THRESHOLD;
#endif
    fi_bl_drv_sbpm_init(SBPM_BASE_ADDRESS, SBPM_LIST_SIZE, SBPM_REPLY_ADDRESS, &sbpm_global_configuration,
        &sbpm_ug_configuration);

    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_GPON, DRV_SBPM_USER_GROUP_0);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_RNR_A, DRV_SBPM_USER_GROUP_0);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_EMAC0, SBPM_WAN_LAN_GROUP(DRV_BBH_EMAC_0));
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_EMAC1, SBPM_WAN_LAN_GROUP(DRV_BBH_EMAC_1));
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_EMAC2, SBPM_WAN_LAN_GROUP(DRV_BBH_EMAC_2));
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_EMAC3, SBPM_WAN_LAN_GROUP(DRV_BBH_EMAC_3));
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_EMAC4, SBPM_WAN_LAN_GROUP(DRV_BBH_EMAC_4));
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_PCI0, DRV_SBPM_USER_GROUP_1);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_RNR_B, DRV_SBPM_USER_GROUP_1);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_SPARE_0, DRV_SBPM_USER_GROUP_6);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_MIPS_C, DRV_SBPM_USER_GROUP_7);
}

/* Following array is based on index of DRV_BPM_GLOBAL_THRESHOLD;
 * uninitialized entries are place holders and should not be used */
static struct Bpm_defenition {
    int num_bpm;
    int ds_thresh;
    int us_thresh;
    int dongle_thresh;
    int common_thresh;
} bpm_defenitions[] =
{
    { 2.5 * 1024, 0, 0, 0, 0 },
    { 5 * 1024, 1.5 * 1024, 3 * 1024, 0, 384 },
    { 7.5 * 1024, 1.5 *1024, 2 * 1024, 1.5 *1024, 2432 },
    { 10 * 1024, 0, 0, 0, 0 },
    { 12.5 * 1024, 0, 0, 0, 0 },
#ifdef G9991
    { 15 * 1024, 11.5 * 1024, 2.5 * 1024, 0, 896 },
#else
    { 15 * 1024, 2 * 1024, 3 * 1024, 2.5 * 1024, 7552},
#endif
};

static void bpm_drv_init(DRV_BPM_GLOBAL_THRESHOLD global_thresh)
{
#define BPM_WAN_LAN_GROUP(emac) ((BBH_PORT_IS_WAN( emac)) ? \
DRV_BPM_USER_GROUP_0 : DRV_BPM_USER_GROUP_1)
    DRV_BPM_GLOBAL_CONFIGURATION bpm_global_configuration;
    DRV_BPM_USER_GROUPS_THRESHOLDS bpm_ug_configuration;
    DRV_BPM_RUNNER_MSG_CTRL_PARAMS runner_msg_ctrl_params;
    int i;

    bpm_global_configuration.hysteresis = BPM_DEFAULT_HYSTERESIS;
    bpm_global_configuration.threshold = global_thresh;
    for (i = 0; i < ARRAY_LENGTH(bpm_ug_configuration.ug_arr); i++)
    {
        bpm_ug_configuration.ug_arr[i].hysteresis = BPM_DEFAULT_HYSTERESIS;
        bpm_ug_configuration.ug_arr[i].threshold = 64;
        bpm_ug_configuration.ug_arr[i].exclusive_hysteresis = BPM_DEFAULT_HYSTERESIS;
        bpm_ug_configuration.ug_arr[i].exclusive_threshold = 64;
    }

    bpm_ug_configuration.ug_arr[DRV_BPM_USER_GROUP_0].threshold = bpm_defenitions[global_thresh].num_bpm;
    bpm_ug_configuration.ug_arr[DRV_BPM_USER_GROUP_0].exclusive_threshold = bpm_defenitions[global_thresh].ds_thresh;
    bpm_ug_configuration.ug_arr[DRV_BPM_USER_GROUP_1].hysteresis = 128;
    bpm_ug_configuration.ug_arr[DRV_BPM_USER_GROUP_1].exclusive_hysteresis= 128;
    bpm_ug_configuration.ug_arr[DRV_BPM_USER_GROUP_1].threshold = bpm_defenitions[global_thresh].num_bpm;
    bpm_ug_configuration.ug_arr[DRV_BPM_USER_GROUP_1].exclusive_threshold = bpm_defenitions[global_thresh].us_thresh;
    if (bpm_defenitions[global_thresh].dongle_thresh)
    {
        bpm_ug_configuration.ug_arr[DRV_BPM_USER_GROUP_2].threshold = bpm_defenitions[global_thresh].num_bpm;
        bpm_ug_configuration.ug_arr[DRV_BPM_USER_GROUP_2].exclusive_threshold = bpm_defenitions[global_thresh].num_bpm;

        bpm_ug_configuration.ug_arr[DRV_BPM_USER_GROUP_3].threshold = bpm_defenitions[global_thresh].num_bpm;
        bpm_ug_configuration.ug_arr[DRV_BPM_USER_GROUP_3].exclusive_threshold = bpm_defenitions[global_thresh].dongle_thresh;
    }
    bpm_ug_configuration.ug_arr[DRV_BPM_USER_GROUP_6].threshold = BPM_DEFAULT_DS_THRESHOLD;
    bpm_ug_configuration.ug_arr[DRV_BPM_USER_GROUP_6].exclusive_threshold= BPM_DEFAULT_DS_THRESHOLD;
    bpm_ug_configuration.ug_arr[DRV_BPM_USER_GROUP_7].threshold = BPM_CPU_NUMBER_OF_BUFFERS;
    bpm_ug_configuration.ug_arr[DRV_BPM_USER_GROUP_7].exclusive_threshold= BPM_CPU_NUMBER_OF_BUFFERS - 32;

    fi_bl_drv_bpm_init(&bpm_global_configuration, &bpm_ug_configuration, BPM_REPLY_RUNNER_A_ADDRESS, DRV_SPARE_BN_MESSAGE_FORMAT_14_bit_BN_WIDTH);

    fi_bl_drv_bpm_get_runner_msg_ctrl(&runner_msg_ctrl_params);

    runner_msg_ctrl_params.runner_a_reply_target_address = (BPM_REPLY_RUNNER_A_ADDRESS + 0x10000) >> 3;
    runner_msg_ctrl_params.runner_b_reply_target_address = (BPM_REPLY_RUNNER_B_ADDRESS + 0x10000) >> 3;

    fi_bl_drv_bpm_set_runner_msg_ctrl(&runner_msg_ctrl_params);

    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_GPON, DRV_BPM_USER_GROUP_0);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_RNR_A, DRV_BPM_USER_GROUP_0);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_EMAC0, BPM_WAN_LAN_GROUP(DRV_BBH_EMAC_0));
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_EMAC1, BPM_WAN_LAN_GROUP(DRV_BBH_EMAC_1));
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_EMAC2, BPM_WAN_LAN_GROUP(DRV_BBH_EMAC_2));
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_EMAC3, BPM_WAN_LAN_GROUP(DRV_BBH_EMAC_3));
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_EMAC4, BPM_WAN_LAN_GROUP(DRV_BBH_EMAC_4));
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_SPARE_1, DRV_BPM_USER_GROUP_2);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_PCI0, DRV_BPM_USER_GROUP_1);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_PCI1, DRV_BPM_USER_GROUP_3);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_RNR_B, DRV_BPM_USER_GROUP_1);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_SPARE_0, DRV_BPM_USER_GROUP_6);
    fi_bl_drv_bpm_set_user_group_mapping(DRV_BPM_SP_MIPS_C, DRV_BPM_USER_GROUP_7);
}

/* calculates the "forward" value for target matrix */
static uint32_t f_ih_calculate_forward(DRV_IH_TARGET_MATRIX_SOURCE_PORT source_port, DRV_IH_TARGET_MATRIX_DESTINATION_PORT destination_port)
{

    /* no self forwarding except PCI */
    if (source_port == DRV_IH_TARGET_MATRIX_SOURCE_PORT_PCIE0 &&
        destination_port == DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE0)
    {
        return 1;
    }
    /* no self forwarding (if emac 4 is wan, then its, sp will be emac 4 and its dest port will be gpon */
    else if (source_port == DRV_IH_TARGET_MATRIX_SOURCE_PORT_GPON  && destination_port == DRV_IH_TARGET_MATRIX_DESTINATION_PORT_GPON)
    {
        err("enable IH Wan-Wan forwarding...\n");
        return 1;
    }
    else if ((DRV_IH_TARGET_MATRIX_DESTINATION_PORT)source_port == destination_port)
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

/* calculates the "target_memory" value for target matrix */
static DRV_IH_TARGET_MEMORY f_ih_calculate_target_memory(DRV_IH_TARGET_MATRIX_SOURCE_PORT source_port,
    DRV_IH_TARGET_MATRIX_DESTINATION_PORT destination_port, int is_pci_target_ddr)
{
    if (is_matrix_source_lan(source_port) &&
        ((DRV_IH_TARGET_MATRIX_DESTINATION_PORT)source_port != destination_port &&
        (is_matrix_dest_lan(destination_port) ||
        destination_port == DRV_IH_TARGET_MATRIX_DESTINATION_PORT_MULTICAST)))
    {
        /* lan -> lan: target memory is SRAM */
        return DRV_IH_TARGET_MEMORY_SRAM;
    }
    else if (is_matrix_source_lan(source_port) && destination_port == DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE0 &&
        !is_pci_target_ddr)
    {
        /* lan -> PCI: target memory is SRAM unless specified otherwise */
        return DRV_IH_TARGET_MEMORY_SRAM;
    }
    else if (destination_port == DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ALWAYS_SRAM)
    {
        return DRV_IH_TARGET_MEMORY_SRAM;
    }

    return DRV_IH_TARGET_MEMORY_DDR;
}

static void f_ih_configure_target_matrix(int is_pci_target_ddr)
{
    int i, j;
    DRV_IH_TARGET_MATRIX_PER_SP_CONFIG per_sp_config;

    for (i = DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH0; i < DRV_IH_TARGET_MATRIX_NUMBER_OF_SOURCE_PORTS; i++)
    {
        /* the destination ports after 'multicast' are not in use currently */
        for (j = DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH0; j < DRV_IH_TARGET_MATRIX_NUMBER_OF_DESTINATION_PORTS; j++)
        {
            per_sp_config.entry[j].target_memory = f_ih_calculate_target_memory(i, j, is_pci_target_ddr);
            per_sp_config.entry[j].direct_mode = f_ih_calculate_direct_mode(i, j);

            fi_bl_drv_ih_set_forward(i, j, f_ih_calculate_forward(i, j));
        }
        fi_bl_drv_ih_set_target_matrix(i, &per_sp_config);
    }
}

static void f_ih_configure_parser(void)
{
    DRV_IH_PARSER_CONFIG parser_config ;
#ifndef _CFE_
    uint8_t i;
#endif

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

#ifdef G9991
    /* pre-da lookup */
    parser_config.eng_cfg |= 0x8;
#endif

    fi_bl_drv_ih_configure_parser(&parser_config);
#ifndef _CFE_
    /* set PPP protocol code for IPv4 */
    fi_bl_drv_ih_set_ppp_code(0,IH_PARSER_PPP_PROTOCOL_CODE_0_IPV4);

    /* set PPP protocol code for IPv6 */
    fi_bl_drv_ih_set_ppp_code (1, IH_PARSER_PPP_PROTOCOL_CODE_1_IPV6);

    for ( i = IH_IP_L4_FILTER_USER_DEFINED_0 ; i <= IH_IP_L4_FILTER_USER_DEFINED_3 ; i ++ )
    {
        /* set a user-defined L4 Protocol to 255 */
        fi_bl_drv_ih_set_user_ip_l4_protocol ( i - IH_IP_L4_FILTER_USER_DEFINED_0,
            IH_L4_FILTER_DEF ) ;
    }
#endif
}

/* this function configures the IH classes which are in use */
static void f_ih_configure_classes(void)
{
    uint8_t i;

    for (i = 0; i < ARRAY_LENGTH(gs_ih_classes); i++)
        fi_bl_drv_ih_configure_class(gs_ih_classes[i].class_index, &gs_ih_classes[i].class_config);
}

/* Enable IPTV prefix filter in IH. */
#ifndef RDD_BASIC
void f_ih_cfg_mcast_prefix_filter_enable(void)
{
    DRV_IH_CLASSIFIER_CONFIG ih_classifier_config;
    uint8_t ipv4_mac_da_prefix[] = {0x01, 0x00, 0x5E, 0x00, 0x00, 0x00};
    uint8_t ipv4_mac_da_prefix_mask[] = {0xFF, 0xFF, 0xFF, 0x80, 0x00, 0x00};
    uint8_t ipv6_mac_da_prefix[] = {0x33, 0x33, 0x00, 0x00, 0x00, 0x00};
    uint8_t ipv6_mac_da_prefix_mask[] = {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};

    /* Clear all attributes */
    memset(&ih_classifier_config, 0, sizeof(DRV_IH_CLASSIFIER_CONFIG));

    /* We're using 2 DA filters with mask, 1 for IPv4 and 1 for IPv6 */
    fi_bl_drv_ih_set_da_filter_with_mask(IH_DA_FILTER_IPTV_IPV4, ipv4_mac_da_prefix, ipv4_mac_da_prefix_mask);

    fi_bl_drv_ih_set_da_filter_with_mask (IH_DA_FILTER_IPTV_IPV6, ipv6_mac_da_prefix, ipv6_mac_da_prefix_mask);

    /* Enable the IH DA filter - IPv4: 0x01005E */
    fi_bl_drv_ih_enable_da_filter(IH_DA_FILTER_IPTV_IPV4, 1);

    /* Enable the IH DA filter - IPv6: 0x3333 */
    fi_bl_drv_ih_enable_da_filter (IH_DA_FILTER_IPTV_IPV6, 1);

    /* When IPTV prefix filter is enabled: We want to override the class to IPTV class  */
    ih_classifier_config.da_filter_any_hit = 1;
    /* da_filter_hit set to 1 and da_filter_number mask is[110] because we're after filters 0-1 */
    ih_classifier_config.mask = IPTV_FILTER_MASK_DA;
    ih_classifier_config.resulting_class = DRV_RDD_IH_CLASS_IPTV_INDEX;
    ih_classifier_config.matched_da_filter = 0;

    /* Configure a classifier for IPTV */
    fi_bl_drv_ih_configure_classifier(IH_CLASSIFIER_IPTV, &ih_classifier_config);
}
#endif


#ifndef RDD_BASIC
/* this function configures the IH IPTV classifier
 * For GPON mode: create a classifier to perform class override from IPTV to BRIDGE_LOW if and only if the bcast filter
 * for iptv is enabled. */
void f_ih_cfg_iptv_lookup_classifier(void)
{
    DRV_IH_CLASSIFIER_CONFIG ih_classifier_config;

    /* Configure the ih IGMP classifier */
    memset(&ih_classifier_config, 0, sizeof(DRV_IH_CLASSIFIER_CONFIG));

    ih_classifier_config.mask                    = MASK_IH_CLASS_KEY_L4;
    ih_classifier_config.resulting_class         = DRV_RDD_IH_CLASS_WAN_BRIDGED_LOW_INDEX;
    ih_classifier_config.l4_protocol             = DRV_IH_L4_PROTOCOL_IGMP;
    /* Configure a classifier for broadcast traffic arriving on IPTV flow */
    fi_bl_drv_ih_configure_classifier(IH_CLASSIFIER_IGMP_IPTV, &ih_classifier_config);

    /* Configure the ih ICMPV6 classifier */
    memset(&ih_classifier_config, 0, sizeof(DRV_IH_CLASSIFIER_CONFIG));

    ih_classifier_config.mask                    = MASK_IH_CLASS_KEY_L4;
    ih_classifier_config.resulting_class         = DRV_RDD_IH_CLASS_WAN_BRIDGED_LOW_INDEX;
    ih_classifier_config.l4_protocol             = DRV_IH_L4_PROTOCOL_ICMPV6;
    /* Configure a classifier for broadcast traffic arriving on IPTV flow */
    fi_bl_drv_ih_configure_classifier(IH_CLASSIFIER_ICMPV6, &ih_classifier_config);
}
#endif

static void f_if_configure_wan_ports(int fiber)
{
    DRV_IH_WAN_PORTS_CONFIG wan_ports_config = {};

    wan_ports_config.eth0 = pDpiCfg->wan_bbh == DRV_BBH_EMAC_0 ? 1 : 0;
    wan_ports_config.eth1 = pDpiCfg->wan_bbh == DRV_BBH_EMAC_1 ? 1 : 0;
    wan_ports_config.eth2 = pDpiCfg->wan_bbh == DRV_BBH_EMAC_2 ? 1 : 0;
    wan_ports_config.eth3 = pDpiCfg->wan_bbh == DRV_BBH_EMAC_3 ? 1 : 0;
    wan_ports_config.eth4 = pDpiCfg->wan_bbh == DRV_BBH_EMAC_4 ? 1 : 0;
    wan_ports_config.runner_a = 0;
    wan_ports_config.runner_b = 0;
    wan_ports_config.pcie0 = 0;
    wan_ports_config.pcie1 = 0;

    if (fiber)
    {
        wan_ports_config.gpon = (pDpiCfg->wan_bbh == DRV_BBH_GPON
            || pDpiCfg->wan_bbh == DRV_BBH_EPON
            || pDpiCfg->wan_bbh == DRV_BBH_EMAC_5) ? 1 : 0;
    }

    fi_bl_drv_ih_configure_wan_ports(&wan_ports_config);
}

static void f_ih_init(void)
{
    DRV_IH_GENERAL_CONFIG ih_general_config ;
    DRV_IH_PACKET_HEADER_OFFSETS packet_header_offsets ;
    DRV_IH_RUNNER_BUFFERS_CONFIG runner_buffers_config ;
    DRV_IH_RUNNERS_LOAD_THRESHOLDS runners_load_thresholds ;
    DRV_IH_ROUTE_ADDRESSES xi_route_addresses ;
    DRV_IH_LOGICAL_PORTS_CONFIG logical_ports_config ;
#ifndef _CFE_
    int i;
#endif

    /* general configuration */
    ih_general_config.runner_a_ih_response_address                                      = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_0_IH_RESPONSE_ADDRESS);
    ih_general_config.runner_b_ih_response_address                                      = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_1_IH_RESPONSE_ADDRESS);
    ih_general_config.runner_a_ih_congestion_report_address             = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_0_IH_CONGESTION_REPORT_ADDRESS);
    ih_general_config.runner_b_ih_congestion_report_address             = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_1_IH_CONGESTION_REPORT_ADDRESS);
    ih_general_config.runner_a_ih_congestion_report_enable                      = DRV_RDD_IH_RUNNER_0_IH_CONGESTION_REPORT_ENABLE;
    ih_general_config.runner_b_ih_congestion_report_enable                      = DRV_RDD_IH_RUNNER_1_IH_CONGESTION_REPORT_ENABLE;
    ih_general_config.lut_searches_enable_in_direct_mode                        = 1;
    ih_general_config.sn_stamping_enable_in_direct_mode                         = 1;
    ih_general_config.header_length_minimum                                             = IH_HEADER_LENGTH_MIN;
    ih_general_config.congestion_discard_disable                                        = 0;
    ih_general_config.cam_search_enable_upon_invalid_lut_entry          = DRV_RDD_IH_CAM_SEARCH_ENABLE_UPON_INVALID_LUT_ENTRY;

    fi_bl_drv_ih_set_general_configuration(&ih_general_config);

    /* packet header offsets configuration */
    packet_header_offsets.eth0_packet_header_offset                             = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    packet_header_offsets.eth1_packet_header_offset                             = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    packet_header_offsets.eth2_packet_header_offset                             = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    packet_header_offsets.eth3_packet_header_offset                             = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    packet_header_offsets.eth4_packet_header_offset                             = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    packet_header_offsets.gpon_packet_header_offset                             = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    packet_header_offsets.runner_a_packet_header_offset                         = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    packet_header_offsets.runner_b_packet_header_offset                         = DRV_RDD_IH_PACKET_HEADER_OFFSET;

    fi_bl_drv_ih_set_packet_header_offsets(& packet_header_offsets);

    /* Runner Buffers configuration */
    /* same ih_managed_rb_base_address should be used for both runners */
    runner_buffers_config.runner_a_ih_managed_rb_base_address           = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_0_IH_MANAGED_RB_BASE_ADDRESS);
    runner_buffers_config.runner_b_ih_managed_rb_base_address           = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_1_IH_MANAGED_RB_BASE_ADDRESS);
    runner_buffers_config.runner_a_runner_managed_rb_base_address       = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_0_RUNNER_MANAGED_RB_BASE_ADDRESS);
    runner_buffers_config.runner_b_runner_managed_rb_base_address       = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_1_RUNNER_MANAGED_RB_BASE_ADDRESS);
    runner_buffers_config.runner_a_maximal_number_of_buffers            = DRV_IH_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS_32;
    runner_buffers_config.runner_b_maximal_number_of_buffers            = DRV_IH_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS_32;

    fi_bl_drv_ih_set_runner_buffers_configuration(&runner_buffers_config);

    /* runners load thresholds configuration */
    runners_load_thresholds.runner_a_high_congestion_threshold          = IH_GPON_MODE_DS_RUNNER_A_HIGH_CONGESTION_THRESHOLD;
    runners_load_thresholds.runner_b_high_congestion_threshold          = DRV_IH_INGRESS_QOS_NUMBER_OF_BUFFERS;
    runners_load_thresholds.runner_a_exclusive_congestion_threshold     = IH_GPON_MODE_DS_RUNNER_A_EXCLUSIVE_CONGESTION_THRESHOLD;
    runners_load_thresholds.runner_b_exclusive_congestion_threshold     = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    runners_load_thresholds.runner_a_load_balancing_threshold           = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    runners_load_thresholds.runner_b_load_balancing_threshold           = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    runners_load_thresholds.runner_a_load_balancing_hysteresis          = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    runners_load_thresholds.runner_b_load_balancing_hysteresis          = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;

    fi_bl_drv_ih_set_runners_load_thresholds (& runners_load_thresholds);

    /* route addresses configuration */
    xi_route_addresses.eth0_route_address     = IH_ETH0_ROUTE_ADDRESS;
    xi_route_addresses.eth1_route_address     = IH_ETH1_ROUTE_ADDRESS;
    xi_route_addresses.eth2_route_address     = IH_ETH2_ROUTE_ADDRESS;
    xi_route_addresses.eth3_route_address     = IH_ETH3_ROUTE_ADDRESS;
    xi_route_addresses.eth4_route_address     = IH_ETH4_ROUTE_ADDRESS;
    xi_route_addresses.gpon_route_address         = IH_GPON_ROUTE_ADDRESS;
    xi_route_addresses.runner_a_route_address = IH_RUNNER_A_ROUTE_ADDRESS;
    xi_route_addresses.runner_b_route_address = IH_RUNNER_B_ROUTE_ADDRESS;

    fi_bl_drv_ih_set_route_addresses(&xi_route_addresses);

    /* logical ports configuration */
    logical_ports_config.eth0_config.parsing_layer_depth                = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_ETH0_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.eth0_config.proprietary_tag_size               = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    logical_ports_config.eth1_config.parsing_layer_depth                = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_ETH1_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.eth1_config.proprietary_tag_size               = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    logical_ports_config.eth2_config.parsing_layer_depth                = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_ETH2_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.eth2_config.proprietary_tag_size               = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    logical_ports_config.eth3_config.parsing_layer_depth                = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_ETH3_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.eth3_config.proprietary_tag_size               = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    logical_ports_config.eth4_config.parsing_layer_depth                = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_ETH4_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.eth4_config.proprietary_tag_size               = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    logical_ports_config.gpon_config.parsing_layer_depth                = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_GPON_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.gpon_config.proprietary_tag_size               = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    logical_ports_config.runner_a_config.parsing_layer_depth    = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_RNRA_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.runner_a_config.proprietary_tag_size   = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    logical_ports_config.runner_b_config.parsing_layer_depth    = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_RNRB_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.runner_b_config.proprietary_tag_size   = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    logical_ports_config.pcie0_config.parsing_layer_depth               = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_PCIE0_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.pcie0_config.proprietary_tag_size              = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    logical_ports_config.pcie1_config.parsing_layer_depth               = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_PCIE1_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.pcie1_config.proprietary_tag_size              = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    fi_bl_drv_ih_set_logical_ports_configuration(&logical_ports_config);

    /* source port to ingress queue mapping - use default values*/
    /* ingress queues configuration - use default values */

    /* update BBH-IH queues bitmask according to default values*/
    fi_ih_ingress_buffers_bitmask_set();
#ifndef _CFE_
    /* enabling the 2 dscp to pbit tables */
    for (i = 0; i < DRV_IH_NUMBER_OF_DSCP_TO_TCI_TABLES; i++)
        fi_bl_drv_ih_enable_dscp_to_tci_table(i, 1);
#endif
    f_if_configure_wan_ports(0);

    f_ih_configure_lookup_tables();

    f_ih_configure_classes();

    f_ih_configure_target_matrix(1);

    f_ih_configure_parser();
    /*do the following configuration only in wan mode*/
#ifndef RDD_BASIC
    f_ih_cfg_mcast_prefix_filter_enable();
    f_ih_cfg_iptv_lookup_classifier();
#endif
}

void bridge_port_sa_da_cfg(void)
{
    /* SA/DA Lookup Configuration */
    int i;

    for (i = BL_LILAC_RDD_WAN_BRIDGE_PORT;  i <= BL_LILAC_RDD_PCI_BRIDGE_PORT; i++)
    {
        if (i == BL_LILAC_RDD_WAN_IPTV_BRIDGE_PORT)
        {
            rdd_da_mac_lkp_cfg(i, 0);
            rdd_sa_mac_lkp_cfg(i, 0);
        }
        else
        {
            rdd_da_mac_lkp_cfg(i, 1);
            rdd_unknown_da_mac_cmd_cfg(i, rdpa_forward_action_host);

            rdd_sa_mac_lkp_cfg(i, 1);
            rdd_unknown_sa_mac_cmd_cfg(i, rdpa_forward_action_host);
        }
    }
}
#ifdef RDD_BASIC
static void f_initialize_basic_runner_parameters(void)
{
    int rdd_error;
    uint32_t    i,j;

    /* Default configuration of all RDD reasons to CPU_RX_QUEUE_ID_0 */
    for (i = 0; i < rdpa_cpu_reason__num_of; i++)
    {
        rdd_error = rdd_cpu_reason_to_cpu_rx_queue(i, 0, rdpa_dir_ds);

        rdd_error |= rdd_cpu_reason_to_cpu_rx_queue(i, 0, rdpa_dir_us);
    }

    /* Initialize Ethernet priority queues */
    for (i = 0; i < 8; i++)
    {
        /* (-1) TBD : when runner will support EMAC_5 */
        for (j = RDD_EMAC_ID_0; j <= RDD_EMAC_ID_4; j++)
        {
            /* Configure queue size in RDD */
            rdd_error |= rdd_eth_tx_queue_config(j, i, CS_RDD_ETH_TX_QUEUE_PACKET_THRESHOLD,0,INVALID_COUNTER_ID);
        }
    }

    bridge_port_sa_da_cfg();

}
#endif

static void f_configure_runner(uint32_t runner_tm_base_addr,
    uint32_t runner_mc_base_addr, int bpm_thresh)
{
    /* Local Variables */
    rdd_init_params_t rdd_init_params;
    uint8_t *ddr_runner_base_ptr;
    uint32_t high_priority_threshold;

    memset(&rdd_init_params,0,sizeof(rdd_init_params_t));

    /* zero Runner memories (data, program and context) */
    rdd_init();

    rdd_load_microcode((uint8_t *)firmware_binary_A,
        (uint8_t *)firmware_binary_B, (uint8_t*)firmware_binary_C, (uint8_t *)firmware_binary_D);

    rdd_load_prediction((uint8_t *)firmware_predict_A, (uint8_t *)firmware_predict_B,
        (uint8_t *)firmware_predict_C, (uint8_t *)firmware_predict_D);

    switch (pDpiCfg->wan_bbh)
    {
    case DRV_BBH_EMAC_0:
        rdd_init_params.wan_physical_port = RDD_WAN_PHYSICAL_PORT_ETH0;
        break;
    case DRV_BBH_EMAC_4:
        rdd_init_params.wan_physical_port = RDD_WAN_PHYSICAL_PORT_ETH4;
        break;
    case DRV_BBH_EMAC_1:
        rdd_init_params.wan_physical_port = RDD_WAN_PHYSICAL_PORT_ETH1;
        break;
    case DRV_BBH_EMAC_2:
        rdd_init_params.wan_physical_port = RDD_WAN_PHYSICAL_PORT_ETH2;
        break;
    case DRV_BBH_EMAC_3:
        rdd_init_params.wan_physical_port = RDD_WAN_PHYSICAL_PORT_ETH3;
        break;
    case DRV_BBH_EMAC_5:
    case DRV_BBH_GPON:
    case DRV_BBH_EPON:
    default:
        rdd_init_params.wan_physical_port = RDD_WAN_PHYSICAL_PORT_FIBER;
        break;
    }

    /* Add basic offset when pass the addresses to RDD */
    ddr_runner_base_ptr = (uint8_t *)DEVICE_ADDRESS((uint32_t)runner_tm_base_addr);
#ifndef LEGACY_RDP
    rdd_init_params.ddr_runner_base_ptr = ddr_runner_base_ptr;
    rdd_init_params.ddr_packet_headroom_size = pDpiCfg->headroom_size;
#else
    rdd_init_params.ddr_pool_ptr = ddr_runner_base_ptr;
    rdd_init_params.ddr_headroom_size = pDpiCfg->headroom_size;
#endif
    rdd_init_params.extra_ddr_pool_ptr = (uint8_t*)DEVICE_ADDRESS((uint32_t)runner_mc_base_addr);
    rdd_init_params.mac_table_size = RDD_MAC_TABLE_SIZE_1024;
    rdd_init_params.iptv_table_size = RDD_MAC_TABLE_SIZE_256;
    /* XXX: Temporary, take this from global_system */
    rdd_init_params.broadcom_switch_mode = 0;
    rdd_init_params.broadcom_switch_physical_port = 0;
    /* ip class operational mode*/
    rdd_init_params.bridge_flow_cache_mode = pDpiCfg->bridge_fc_mode;
    rdd_init_params.chip_revision = RDD_CHIP_REVISION_B0;
#ifdef G9991
    if (pDpiCfg->g9991_debug_port != -1)
    {
        rdd_init_params.lan_direct_to_cpu_port = BL_LILAC_RDD_LAN0_BRIDGE_PORT +
            pDpiCfg->g9991_debug_port;
    }
    else
        rdd_init_params.lan_direct_to_cpu_port = 0; /* disabled */
#endif

    rdd_init_params.cpu_tx_abs_packet_limit = RDD_CPU_TX_ABS_FIFO_SIZE;

/* Runner tables on offset 7.5K BPM buffers * 2K */
#ifndef LEGACY_RDP
    rdd_init_params.bpm_global_threshold = bpm_thresh;
#else
    rdd_init_params.ddr_runner_tables_ptr = ddr_runner_base_ptr + (uint32_t)(bpm_defenitions[bpm_thresh].num_bpm * pDpiCfg->bpm_buffer_size);
#endif
    rdd_init_params.us_ddr_queue_enable = pDpiCfg->us_ddr_queue_enable;
    rdd_init_params.bpm_buffer_size = pDpiCfg->bpm_buffer_size;
    rdd_data_structures_init(&rdd_init_params);

    if (pDpiCfg->wan_bbh < DRV_BBH_EMAC_5) // III also needed for GBE ?
        rdd_wan_mode_config(rdd_wan_gpon);
    high_priority_threshold = bpm_defenitions[bpm_thresh].num_bpm - bpm_defenitions[bpm_thresh].ds_thresh - bpm_defenitions[bpm_thresh].us_thresh -bpm_defenitions[bpm_thresh].dongle_thresh;
    rdd_common_bpm_threshold_config(bpm_defenitions[bpm_thresh].common_thresh, high_priority_threshold);
}

/* checks that the address is multipilcation of 2*(1024^2)*/
int f_validate_ddr_address(uint32_t address)
{
    if (address % (2 * 1024 * 1024) == 0)
        return 1;
    return 0;
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
    bpm_sp_enable.emac2_en = 1;
    bpm_sp_enable.emac3_en = 1;
    bpm_sp_enable.emac4_en = 1;

    BPM_MODULE_REGS_BPM_SP_EN_WRITE( bpm_sp_enable);
}

void f_basic_sbpm_sp_enable(void)
{
    DRV_SBPM_SP_ENABLE sbpm_sp_enable = { 0 };

    sbpm_sp_enable.rnra_sp_enable = 1;
    sbpm_sp_enable.rnrb_sp_enable = 1;
    sbpm_sp_enable.eth0_sp_enable = 1;
    sbpm_sp_enable.eth1_sp_enable = 1;
    sbpm_sp_enable.eth2_sp_enable = 1;
    sbpm_sp_enable.eth3_sp_enable = 1;
    sbpm_sp_enable.eth4_sp_enable = 1;

    fi_bl_drv_sbpm_sp_enable(&sbpm_sp_enable);
}

uint32_t data_path_init(S_DPI_CFG *pCfg)
{
    uint32_t rc, rnrFreq = DEFAULT_RUNNER_FREQ;
    DRV_BBH_PORT_INDEX macIter;

    /* Point to default configuration if none is passed to init */
    pDpiCfg = !pCfg ? &DpiBasicConfig : pCfg;

    /* Point to Runner allocated DDR memory */
#ifdef _CFE_
    pCfg->runner_tm_base_addr = K0_TO_K1((uint32_t)KMALLOC(0x1400000, 0x200000));
    pCfg->runner_mc_base_addr = K0_TO_K1((uint32_t)KMALLOC(0x400000, 0x200000));
#endif

    if ((rc = f_validate_ddr_address((uint32_t)pCfg->runner_tm_base_addr)) != 1)
    {
        err("Failed to get valid DDR TM address, rc = %d " \
            "validate_ddr_address: ddr_tm_base_address=%x\n", rc,
            pCfg->runner_tm_base_addr);
    }

    if ((rc = f_validate_ddr_address((uint32_t)pCfg->runner_mc_base_addr)) != 1)
        err("Failed to get valid DDR Multicast address, rc = %d\n", rc);

    rc = DPI_RC_OK;

    ddr_tm_base_address_phys = (void*)VIRT_TO_PHYS(pCfg->runner_tm_base_addr);
    ddr_multicast_base_address_phys =
        (void*)VIRT_TO_PHYS(pCfg->runner_mc_base_addr);

    /*init dma arrays*/
    f_initialize_bbh_dma_sdma_related_arrays(0);

    /*init runner,load microcode and structures*/
    f_configure_runner(pCfg->runner_tm_base_addr, pCfg->runner_mc_base_addr,
        pCfg->bpm_buffers_number);

    if (pDpiCfg->runner_freq)
        rnrFreq = pDpiCfg->runner_freq;
    rdd_runner_frequency_set(rnrFreq);

    f_ih_init();
    sbpm_drv_init();
    bpm_drv_init(pCfg->bpm_buffers_number);

    /*take out of reset emacs and reset BBH*/
    for (macIter =0; macIter <= DRV_BBH_EMAC_4; macIter++)
    {
        if (pDpiCfg->enabled_port_map & (1<<macIter))
            f_initialize_bbh_of_emac_port(macIter);
    }

    f_initialize_dma_sdma(0);

#ifdef RDD_BASIC
    f_initialize_basic_runner_parameters();
#endif

    if (rc == DPI_RC_OK)
        initDone = 1;

    return rc;
}

uint32_t data_path_go(void)
{

    if (initDone != 1)
    {
        err("Data Path init didn't finished \n");
        return DPI_RC_ERROR;
    }

    /*enable runner*/
    rdd_runner_enable();

    /*enable the source ports in bpm/sbpm in case of basic config*/
#ifdef  RDD_BASIC
    f_basic_bpm_sp_enable();
    f_basic_sbpm_sp_enable();
#endif

    err("data_path_go Done!!\n");
    return DPI_RC_OK;
}



#ifndef RDD_BASIC
uint32_t data_path_init_fiber(DRV_BBH_PORT_INDEX wan_bbh)
{
    if (wan_bbh < DRV_BBH_EMAC_5)
        return -1;

    pDpiCfg->wan_bbh = wan_bbh;

    f_initialize_bbh_dma_sdma_related_arrays(1);
    
    f_ih_configure_target_matrix(1);
    f_if_configure_wan_ports(1);

#ifndef RDD_BASIC
    if (wan_bbh == DRV_BBH_GPON)
        f_initialize_bbh_of_gpon_port();
    else if (wan_bbh == DRV_BBH_EPON)
        f_initialize_bbh_of_epon_port();
    else if (wan_bbh == DRV_BBH_EMAC_5)
        f_initialize_bbh_of_emac_port(DRV_BBH_EMAC_5);
#endif

    f_initialize_dma_sdma(1);

    switch (wan_bbh)
    {
    case DRV_BBH_EPON:
        rdd_wan_mode_config(rdd_wan_epon);
        break;
    case DRV_BBH_EMAC_5:
        rdd_wan_mode_config(rdd_wan_ae);
        break;
    default:
        rdd_wan_mode_config(rdd_wan_gpon);
        break;
    }

    return 0;
}

/* initilaization of BBH of GPON port */
void f_initialize_bbh_of_gpon_port(void)
{
    DRV_BBH_TX_CONFIGURATION bbh_tx_configuration ;
    DRV_BBH_RX_CONFIGURATION bbh_rx_configuration ;
    /* need 16 bit variable (and not 8) for the break condition */
    DRV_BBH_PER_FLOW_CONFIGURATION per_flow_configuration ;
    uint16_t number_of_packets_dropped_by_ih;

    uint32_t i;


    /*** BBH TX ***/
    bbh_tx_configuration.skb_address = MS_BYTE_TO_8_BYTE_RESOLUTION(GPON_ABSOLUTE_TX_BBH_COUNTER_ADDRESS);

    bbh_tx_configuration.dma_route_address = bbh_route_address_dma[DRV_BBH_GPON];
    bbh_tx_configuration.bpm_route_address = bbh_route_address_bpm[DRV_BBH_GPON];
    bbh_tx_configuration.sdma_route_address = bbh_route_address_sdma[DRV_BBH_GPON];
    bbh_tx_configuration.sbpm_route_address = bbh_route_address_sbpm[DRV_BBH_GPON];
    /* runner 1 is the one which handles TX */
    bbh_tx_configuration.runner_route_address = bbh_route_address_runner_1[DRV_BBH_GPON];

    bbh_tx_configuration.ddr_buffer_size = f_rdpa_bbh_ddr_buffer_size();
    bbh_tx_configuration.payload_offset_resolution = DRV_BBH_PAYLOAD_OFFSET_RESOLUTION_2_B;
    bbh_tx_configuration.multicast_header_size = BBH_MULTICAST_HEADER_SIZE_FOR_LAN_PORT;
    bbh_tx_configuration.multicast_headers_base_address_in_byte = (uint32_t)ddr_multicast_base_address_phys;

    /* one thread is used for all TCONTs */
    bbh_tx_configuration.task_0 = WAN_TX_THREAD_NUMBER;
    bbh_tx_configuration.task_1 = WAN_TX_THREAD_NUMBER;
    bbh_tx_configuration.task_2 = WAN_TX_THREAD_NUMBER;
    bbh_tx_configuration.task_3 = WAN_TX_THREAD_NUMBER;
    bbh_tx_configuration.task_4 = WAN_TX_THREAD_NUMBER;
    bbh_tx_configuration.task_5 = WAN_TX_THREAD_NUMBER;
    bbh_tx_configuration.task_6 = WAN_TX_THREAD_NUMBER;
    bbh_tx_configuration.task_7 = WAN_TX_THREAD_NUMBER;
    bbh_tx_configuration.task_8_39 = WAN_TX_THREAD_NUMBER;

    bbh_tx_configuration.pd_fifo_size_0 = BBH_TX_GPON_PD_FIFO_SIZE_0_7;
    bbh_tx_configuration.pd_fifo_size_1 = BBH_TX_GPON_PD_FIFO_SIZE_0_7;
    bbh_tx_configuration.pd_fifo_size_2 = BBH_TX_GPON_PD_FIFO_SIZE_0_7;
    bbh_tx_configuration.pd_fifo_size_3 = BBH_TX_GPON_PD_FIFO_SIZE_0_7;
    bbh_tx_configuration.pd_fifo_size_4 = BBH_TX_GPON_PD_FIFO_SIZE_0_7;
    bbh_tx_configuration.pd_fifo_size_5 = BBH_TX_GPON_PD_FIFO_SIZE_0_7;
    bbh_tx_configuration.pd_fifo_size_6 = BBH_TX_GPON_PD_FIFO_SIZE_0_7;
    bbh_tx_configuration.pd_fifo_size_7 = BBH_TX_GPON_PD_FIFO_SIZE_0_7;
    bbh_tx_configuration.pd_fifo_size_8_15 = BBH_TX_GPON_PD_FIFO_SIZE_8_15;
    bbh_tx_configuration.pd_fifo_size_16_23 = BBH_TX_GPON_PD_FIFO_SIZE_16_23;
    bbh_tx_configuration.pd_fifo_size_24_31 = BBH_TX_GPON_PD_FIFO_SIZE_24_31;
    bbh_tx_configuration.pd_fifo_size_32_39 = BBH_TX_GPON_PD_FIFO_SIZE_32_39;

    if (bbh_tx_configuration.pd_fifo_size_32_39 == 0)
    {
        /* in case we support 32 tconts only, we don't need PD FIFOs for tconts 32-39.
           however the bbh driver doesn't allow configuring their size to 0. therefore we set dummy value.
           anyway, the BBH will ingore this value, since FW will not activate these tconts. */
        bbh_tx_configuration.pd_fifo_size_32_39 = DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE;
    }

    bbh_tx_configuration.pd_fifo_base_0 = BBH_TX_GPON_PD_FIFO_BASE_0;
    bbh_tx_configuration.pd_fifo_base_1 = BBH_TX_GPON_PD_FIFO_BASE_1;
    bbh_tx_configuration.pd_fifo_base_2 = BBH_TX_GPON_PD_FIFO_BASE_2;
    bbh_tx_configuration.pd_fifo_base_3 = BBH_TX_GPON_PD_FIFO_BASE_3;
    bbh_tx_configuration.pd_fifo_base_4 = BBH_TX_GPON_PD_FIFO_BASE_4;
    bbh_tx_configuration.pd_fifo_base_5 = BBH_TX_GPON_PD_FIFO_BASE_5;
    bbh_tx_configuration.pd_fifo_base_6 = BBH_TX_GPON_PD_FIFO_BASE_6;
    bbh_tx_configuration.pd_fifo_base_7 = BBH_TX_GPON_PD_FIFO_BASE_7;
    bbh_tx_configuration.pd_fifo_base_8_15 = BBH_TX_GPON_PD_FIFO_BASE_8_15;
    bbh_tx_configuration.pd_fifo_base_16_23 = BBH_TX_GPON_PD_FIFO_BASE_16_23;
    bbh_tx_configuration.pd_fifo_base_24_31 = BBH_TX_GPON_PD_FIFO_BASE_24_31;
    bbh_tx_configuration.pd_fifo_base_32_39 = BBH_TX_GPON_PD_FIFO_BASE_32_39;

    if (bbh_tx_configuration.pd_fifo_base_32_39 == BBH_TX_GPON_TOTAL_NUMBER_OF_PDS)
    {
        /* see comment of pd_fifo_size_32_39 above */
        bbh_tx_configuration.pd_fifo_base_32_39 = 0;
    }

    /* In GPON case, PD prefetch byte threshold will be enabled and configured to the maximal value (4095 * 32 bytes = 131040 bytes) */
    bbh_tx_configuration.pd_prefetch_byte_threshold_enable = 1;
    bbh_tx_configuration.pd_prefetch_byte_threshold_0_in_32_byte = DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE;
    bbh_tx_configuration.pd_prefetch_byte_threshold_1_in_32_byte = DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE;
    bbh_tx_configuration.pd_prefetch_byte_threshold_2_in_32_byte = DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE;
    bbh_tx_configuration.pd_prefetch_byte_threshold_3_in_32_byte = DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE;
    bbh_tx_configuration.pd_prefetch_byte_threshold_4_in_32_byte = DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE;
    bbh_tx_configuration.pd_prefetch_byte_threshold_5_in_32_byte = DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE;
    bbh_tx_configuration.pd_prefetch_byte_threshold_6_in_32_byte = DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE;
    bbh_tx_configuration.pd_prefetch_byte_threshold_7_in_32_byte = DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE;
    bbh_tx_configuration.pd_prefetch_byte_threshold_8_39_in_32_byte = DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE;

    bbh_tx_configuration.dma_read_requests_fifo_base_address = bbh_dma_and_sdma_read_requests_fifo_base_address[DRV_BBH_GPON];
    bbh_tx_configuration.dma_read_requests_maximal_number = BBH_TX_CONFIGURATIONS_DMACFG_TX_MAXREQ_MAX_VALUE_VALUE_RESET_VALUE;
    bbh_tx_configuration.epnurgnt = BBH_TX_CONFIGURATIONS_DMACFG_TX_EPNURGNT_NORMAL_VALUE_RESET_VALUE;
    bbh_tx_configuration.sdma_read_requests_fifo_base_address = bbh_dma_and_sdma_read_requests_fifo_base_address[DRV_BBH_GPON];
    bbh_tx_configuration.sdma_read_requests_maximal_number = BBH_TX_CONFIGURATIONS_SDMACFG_TX_MAXREQ_MAX_VALUE_VALUE_RESET_VALUE;

    /* The address is in 8 bytes resolution */
    bbh_tx_configuration.tcont_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION(BBH_TX_WAN_CHANNEL_INDEX_ADDRESS);

    /* MDU Mode feature is not supported for GPON */
    bbh_tx_configuration.mdu_mode_enable = 0;
    bbh_tx_configuration.mdu_mode_read_pointer_address_in_8_byte = 0;
    bbh_tx_configuration.ddr_tm_base_address = (uint32_t)ddr_tm_base_address_phys;

    bbh_tx_configuration.emac_1588_enable = 0;

    fi_bl_drv_bbh_tx_set_configuration(DRV_BBH_GPON, &bbh_tx_configuration);

    /*** BBH RX ***/
    /* bbh_rx_set_configuration */
    bbh_rx_configuration.dma_route_address = bbh_route_address_dma[DRV_BBH_GPON];
    bbh_rx_configuration.bpm_route_address = bbh_route_address_bpm[DRV_BBH_GPON];
    bbh_rx_configuration.sdma_route_address = bbh_route_address_sdma[DRV_BBH_GPON];
    bbh_rx_configuration.sbpm_route_address = bbh_route_address_sbpm[DRV_BBH_GPON];
    bbh_rx_configuration.runner_0_route_address = bbh_route_address_runner_0[DRV_BBH_GPON];
    bbh_rx_configuration.runner_1_route_address = bbh_route_address_runner_1[DRV_BBH_GPON];
    bbh_rx_configuration.ih_route_address = bbh_route_address_ih[DRV_BBH_GPON];

    bbh_rx_configuration.ddr_buffer_size = f_rdpa_bbh_ddr_buffer_size();


    bbh_rx_configuration.ddr_tm_base_address = (uint32_t)ddr_tm_base_address_phys;
    bbh_rx_configuration.pd_fifo_base_address_normal_queue_in_8_byte = f_get_bbh_rx_pd_fifo_base_address_normal_queue_in_8_byte(DRV_BBH_GPON);
    bbh_rx_configuration.pd_fifo_base_address_direct_queue_in_8_byte = f_get_bbh_rx_pd_fifo_base_address_direct_queue_in_8_byte(DRV_BBH_GPON);
    bbh_rx_configuration.pd_fifo_size_normal_queue = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    bbh_rx_configuration.pd_fifo_size_direct_queue = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    bbh_rx_configuration.runner_0_task_normal_queue = f_bbh_rx_runner_task_normal_queue(DRV_IH_RUNNER_CLUSTER_A, DRV_BBH_GPON);
    bbh_rx_configuration.runner_0_task_direct_queue = f_bbh_rx_runner_task_direct_queue(DRV_IH_RUNNER_CLUSTER_A, DRV_BBH_GPON);
    bbh_rx_configuration.runner_1_task_normal_queue = f_bbh_rx_runner_task_normal_queue(DRV_IH_RUNNER_CLUSTER_B, DRV_BBH_GPON);
    bbh_rx_configuration.runner_1_task_direct_queue = f_bbh_rx_runner_task_direct_queue(DRV_IH_RUNNER_CLUSTER_B, DRV_BBH_GPON);
    bbh_rx_configuration.dma_data_fifo_base_address = bbh_rx_dma_data_fifo_base_address[DRV_BBH_GPON];
    bbh_rx_configuration.dma_chunk_descriptor_fifo_base_address = bbh_rx_dma_chunk_descriptor_fifo_base_address[DRV_BBH_GPON];
    bbh_rx_configuration.sdma_data_fifo_base_address = bbh_rx_sdma_data_fifo_base_address[DRV_BBH_GPON];
    bbh_rx_configuration.sdma_chunk_descriptor_fifo_base_address = bbh_rx_sdma_chunk_descriptor_fifo_base_address[DRV_BBH_GPON];
    bbh_rx_configuration.dma_data_and_chunk_descriptor_fifos_size = BBH_RX_DMA_FIFOS_SIZE_WAN_GPON;
    bbh_rx_configuration.dma_exclusive_threshold = BBH_RX_DMA_EXCLUSIVE_THRESHOLD_WAN_BBH;
    bbh_rx_configuration.sdma_data_and_chunk_descriptor_fifos_size = bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[DRV_BBH_GPON];
    bbh_rx_configuration.sdma_exclusive_threshold = bbh_rx_sdma_exclusive_threshold[DRV_BBH_GPON];

#if BBH_RX_ETH_MIN_PKT_SIZE_SELECTION_INDEX == 0
    bbh_rx_configuration.minimum_packet_size_0 = MIN_GPON_PKT_SIZE;
#else
#error problem with BBH_RX_ETH_MIN_PKT_SIZE_SELECTION_INDEX
#endif

#if BBH_RX_OMCI_MIN_PKT_SIZE_SELECTION_INDEX == 1
    bbh_rx_configuration.minimum_packet_size_1 = MIN_OMCI_PKT_SIZE;
#else
#error problem with BBH_RX_OMCI_MIN_PKT_SIZE_SELECTION_INDEX
#endif

    /* minimum_packet_size 2-3 are not in use */
    bbh_rx_configuration.minimum_packet_size_2 = MIN_GPON_PKT_SIZE;
    bbh_rx_configuration.minimum_packet_size_3 = MIN_GPON_PKT_SIZE;

#if BBH_RX_MAX_PKT_SIZE_SELECTION_INDEX == 0
    bbh_rx_configuration.maximum_packet_size_0 = pDpiCfg->mtu_size;
#else
#error problem with BBH_RX_MAX_PKT_SIZE_SELECTION_INDEX
#endif

    /* maximum_packet_size 1-3 are not in use */
    bbh_rx_configuration.maximum_packet_size_1 = pDpiCfg->mtu_size;
    bbh_rx_configuration.maximum_packet_size_2 = pDpiCfg->mtu_size;
    bbh_rx_configuration.maximum_packet_size_3 = pDpiCfg->mtu_size;

    bbh_rx_configuration.ih_ingress_buffers_bitmask = bbh_ih_ingress_buffers_bitmask[DRV_BBH_GPON];
    bbh_rx_configuration.packet_header_offset = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    bbh_rx_configuration.reassembly_offset_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION(pDpiCfg->headroom_size);

    /* By default, the triggers for FC will be disabled and the triggers for drop enabled.
       If the user configures flow control for the port, the triggers for drop will be
       disabled and triggers for FC (including Runner request) will be enabled */
    bbh_rx_configuration.flow_control_triggers_bitmask = 0;
    bbh_rx_configuration.drop_triggers_bitmask = DRV_BBH_RX_DROP_TRIGGER_SBPM_IS_IN_EXCLUSIVE_STATE;

    bbh_rx_configuration.flows_32_255_group_divider = BBH_RX_FLOWS_32_255_GROUP_DIVIDER;
    bbh_rx_configuration.ploam_default_ih_class = DRV_RDD_IH_CLASS_WAN_CONTROL_INDEX;
    bbh_rx_configuration.ploam_ih_class_override = 0;

    /* Enable runner task for inter-wan option */
    bbh_rx_configuration.runner_1_task_direct_queue = WAN_TO_WAN_THREAD_NUMBER;

    fi_bl_drv_bbh_rx_set_configuration(DRV_BBH_GPON, &bbh_rx_configuration);

    /* bbh_rx_set_per_flow_configuration */
    per_flow_configuration.minimum_packet_size_selection = 0;
    per_flow_configuration.maximum_packet_size_selection = 0;
    per_flow_configuration.default_ih_class = 0;
    per_flow_configuration.ih_class_override = 0;

    /* initialize all flows (including the 2 groups) */
    for (i = 0; i <= DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_1; i++)
        fi_bl_drv_bbh_rx_set_per_flow_configuration(DRV_BBH_GPON, i, &per_flow_configuration);

    /* configure group 0 (we use one group only, of flows 32-255) */
    per_flow_configuration.minimum_packet_size_selection = BBH_RX_ETH_MIN_PKT_SIZE_SELECTION_INDEX;
    per_flow_configuration.maximum_packet_size_selection = BBH_RX_MAX_PKT_SIZE_SELECTION_INDEX;
    per_flow_configuration.default_ih_class = DRV_RDD_IH_CLASS_WAN_BRIDGED_LOW_INDEX;
    per_flow_configuration.ih_class_override = 1;

    fi_bl_drv_bbh_rx_set_per_flow_configuration(DRV_BBH_GPON, DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_0, &per_flow_configuration);

    /* bbh_rx_get_per_flow_counters */
    /* the per-flow counters must be read at initialization stage, in order to clear them */
    for (i = 0; i < DRV_BBH_RX_NUMBER_OF_FLOWS; i++)
        fi_bl_drv_bbh_rx_get_per_flow_counters(DRV_BBH_GPON, i, &number_of_packets_dropped_by_ih);
}

void f_initialize_bbh_of_epon_port(void)
{
    DRV_BBH_TX_CONFIGURATION                                    bbh_tx_configuration ;
    DRV_BBH_RX_CONFIGURATION                                    bbh_rx_configuration ;
    /* need 16 bit variable (and not 8) for the break condition */
    DRV_BBH_PER_FLOW_CONFIGURATION                              per_flow_configuration ;
    BBH_TX_EPON_CFG                                                             epon_Cfg;
    BBH_TX_CONFIGURATIONS_PDWKUPH0_7                    bbhTxPdWkup0_7;
    BBH_TX_CONFIGURATIONS_PDWKUPH8_31                   bbhTxPdWkup8_31;
    uint32_t                                            eponIHclass0 = DRV_RDD_IH_CLASS_WAN_BRIDGED_LOW_INDEX;
    uint16_t number_of_packets_dropped_by_ih;

    uint32_t i;


    /*** BBH TX ***/
    bbh_tx_configuration.skb_address = MS_BYTE_TO_8_BYTE_RESOLUTION(GPON_ABSOLUTE_TX_BBH_COUNTER_ADDRESS);

    bbh_tx_configuration.dma_route_address = bbh_route_address_dma[DRV_BBH_EPON];
    bbh_tx_configuration.bpm_route_address = bbh_route_address_bpm[DRV_BBH_EPON];
    bbh_tx_configuration.sdma_route_address = bbh_route_address_sdma[DRV_BBH_EPON];
    bbh_tx_configuration.sbpm_route_address = bbh_route_address_sbpm[DRV_BBH_EPON];
    /* runner 1 is the one which handles TX */
    bbh_tx_configuration.runner_route_address = bbh_route_address_runner_1[DRV_BBH_EPON];
    bbh_tx_configuration.runner_sts_route         = BBH_TX_EPON_RUNNER_STS_ROUTE;
    bbh_tx_configuration.ddr_buffer_size = f_rdpa_bbh_ddr_buffer_size();
    bbh_tx_configuration.payload_offset_resolution = DRV_BBH_PAYLOAD_OFFSET_RESOLUTION_2_B;
    bbh_tx_configuration.multicast_header_size = BBH_MULTICAST_HEADER_SIZE_FOR_LAN_PORT;
    bbh_tx_configuration.multicast_headers_base_address_in_byte = (uint32_t)ddr_multicast_base_address_phys;

    /* one thread is used for all TCONTs */
    bbh_tx_configuration.task_0 = EPON_TX_REQUEST_THREAD_NUMBER;
    bbh_tx_configuration.task_1 = EPON_TX_REQUEST_THREAD_NUMBER;
    bbh_tx_configuration.task_2 = EPON_TX_REQUEST_THREAD_NUMBER;
    bbh_tx_configuration.task_3 = EPON_TX_REQUEST_THREAD_NUMBER;
    bbh_tx_configuration.task_4 = EPON_TX_REQUEST_THREAD_NUMBER;
    bbh_tx_configuration.task_5 = EPON_TX_REQUEST_THREAD_NUMBER;
    bbh_tx_configuration.task_6 = EPON_TX_REQUEST_THREAD_NUMBER;
    bbh_tx_configuration.task_7 = EPON_TX_REQUEST_THREAD_NUMBER;
    bbh_tx_configuration.task_8_39 = EPON_TX_REQUEST_THREAD_NUMBER;

    bbh_tx_configuration.pd_fifo_size_0 = BBH_TX_EPON_PD_FIFO_SIZE_0_7;
    bbh_tx_configuration.pd_fifo_size_1 = BBH_TX_EPON_PD_FIFO_SIZE_0_7;
    bbh_tx_configuration.pd_fifo_size_2 = BBH_TX_EPON_PD_FIFO_SIZE_0_7;
    bbh_tx_configuration.pd_fifo_size_3 = BBH_TX_EPON_PD_FIFO_SIZE_0_7;
    bbh_tx_configuration.pd_fifo_size_4 = BBH_TX_EPON_PD_FIFO_SIZE_0_7;
    bbh_tx_configuration.pd_fifo_size_5 = BBH_TX_EPON_PD_FIFO_SIZE_0_7;
    bbh_tx_configuration.pd_fifo_size_6 = BBH_TX_EPON_PD_FIFO_SIZE_0_7;
    bbh_tx_configuration.pd_fifo_size_7 = BBH_TX_EPON_PD_FIFO_SIZE_0_7;
    bbh_tx_configuration.pd_fifo_size_8_15 = BBH_TX_EPON_PD_FIFO_SIZE_8_15;
    bbh_tx_configuration.pd_fifo_size_16_23 = BBH_TX_EPON_PD_FIFO_SIZE_16_23;
    bbh_tx_configuration.pd_fifo_size_24_31 = BBH_TX_EPON_PD_FIFO_SIZE_24_31;
    bbh_tx_configuration.pd_fifo_size_32_39 = BBH_TX_EPON_PD_FIFO_SIZE_32_39;

    if (bbh_tx_configuration.pd_fifo_size_32_39 == 0)
    {
        /* in case we support 32 tconts only, we don't need PD FIFOs for tconts 32-39.
           however the bbh driver doesn't allow configuring their size to 0. therefore we set dummy value.
           anyway, the BBH will ingore this value, since FW will not activate these tconts. */
        bbh_tx_configuration.pd_fifo_size_32_39 = DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE;
    }

    bbh_tx_configuration.pd_fifo_base_0 = BBH_TX_EPON_PD_FIFO_BASE_0;
    bbh_tx_configuration.pd_fifo_base_1 = BBH_TX_EPON_PD_FIFO_BASE_1;
    bbh_tx_configuration.pd_fifo_base_2 = BBH_TX_EPON_PD_FIFO_BASE_2;
    bbh_tx_configuration.pd_fifo_base_3 = BBH_TX_EPON_PD_FIFO_BASE_3;
    bbh_tx_configuration.pd_fifo_base_4 = BBH_TX_EPON_PD_FIFO_BASE_4;
    bbh_tx_configuration.pd_fifo_base_5 = BBH_TX_EPON_PD_FIFO_BASE_5;
    bbh_tx_configuration.pd_fifo_base_6 = BBH_TX_EPON_PD_FIFO_BASE_6;
    bbh_tx_configuration.pd_fifo_base_7 = BBH_TX_EPON_PD_FIFO_BASE_7;
    bbh_tx_configuration.pd_fifo_base_8_15 = BBH_TX_EPON_PD_FIFO_BASE_8_15;
    bbh_tx_configuration.pd_fifo_base_16_23 = BBH_TX_EPON_PD_FIFO_BASE_16_23;
    bbh_tx_configuration.pd_fifo_base_24_31 = BBH_TX_EPON_PD_FIFO_BASE_24_31;
    bbh_tx_configuration.pd_fifo_base_32_39 = BBH_TX_EPON_PD_FIFO_BASE_32_39;

    if (bbh_tx_configuration.pd_fifo_base_32_39 == BBH_TX_EPON_TOTAL_NUMBER_OF_PDS)
    {
        /* see comment of pd_fifo_size_32_39 above */
        bbh_tx_configuration.pd_fifo_base_32_39 = 0;
    }


    /* In EPON case, PD prefetch byte threshold will be enabled and configured to the maximal value (4095 * 32 bytes = 131040 bytes) */
    bbh_tx_configuration.pd_prefetch_byte_threshold_enable = 1;
    bbh_tx_configuration.pd_prefetch_byte_threshold_0_in_32_byte = DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE;
    bbh_tx_configuration.pd_prefetch_byte_threshold_1_in_32_byte = DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE;
    bbh_tx_configuration.pd_prefetch_byte_threshold_2_in_32_byte = DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE;
    bbh_tx_configuration.pd_prefetch_byte_threshold_3_in_32_byte = DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE;
    bbh_tx_configuration.pd_prefetch_byte_threshold_4_in_32_byte = DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE;
    bbh_tx_configuration.pd_prefetch_byte_threshold_5_in_32_byte = DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE;
    bbh_tx_configuration.pd_prefetch_byte_threshold_6_in_32_byte = DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE;
    bbh_tx_configuration.pd_prefetch_byte_threshold_7_in_32_byte = DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE;
    bbh_tx_configuration.pd_prefetch_byte_threshold_8_39_in_32_byte = DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE;

    bbh_tx_configuration.dma_read_requests_fifo_base_address = bbh_dma_and_sdma_read_requests_fifo_base_address[DRV_BBH_EPON];
    bbh_tx_configuration.dma_read_requests_maximal_number = BBH_TX_CONFIGURATIONS_DMACFG_TX_MAXREQ_MAX_VALUE_VALUE_RESET_VALUE;
    bbh_tx_configuration.epnurgnt = BBH_TX_CONFIGURATIONS_DMACFG_TX_EPNURGNT_URGENT_VALUE;
    bbh_tx_configuration.sdma_read_requests_fifo_base_address = bbh_dma_and_sdma_read_requests_fifo_base_address[DRV_BBH_EPON];
    bbh_tx_configuration.sdma_read_requests_maximal_number = BBH_TX_CONFIGURATIONS_SDMACFG_TX_MAXREQ_MAX_VALUE_VALUE_RESET_VALUE;


    /* The address is in 8 bytes resolution */
    bbh_tx_configuration.tcont_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION(BBH_TX_EPON_REQUEST_WAN_CHANNEL_INDEX_ADDRESS);

    /* MDU Mode feature is not supported for GPON */
    bbh_tx_configuration.mdu_mode_enable = 0;
    bbh_tx_configuration.mdu_mode_read_pointer_address_in_8_byte = 0;
    bbh_tx_configuration.ddr_tm_base_address = (uint32_t)ddr_tm_base_address_phys;

    bbh_tx_configuration.emac_1588_enable = 0;

    fi_bl_drv_bbh_tx_set_configuration(DRV_BBH_EPON, &bbh_tx_configuration);

    /*in EPON we must configure the wakeup threshold*/
    bbhTxPdWkup0_7.wkupthresh0  =       BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    bbhTxPdWkup0_7.wkupthresh1  =       BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    bbhTxPdWkup0_7.wkupthresh2  =       BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    bbhTxPdWkup0_7.wkupthresh3  =       BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    bbhTxPdWkup0_7.wkupthresh4  =       BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    bbhTxPdWkup0_7.wkupthresh5  =       BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    bbhTxPdWkup0_7.wkupthresh6  =       BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    bbhTxPdWkup0_7.wkupthresh7  =       BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    BBH_TX_CONFIGURATIONS_PDWKUPH0_7_WRITE(DRV_BBH_EPON,bbhTxPdWkup0_7);

    bbhTxPdWkup8_31.wkupthresh8_15      = BBH_TX_EPON_PD_FIFO_SIZE_8_15 - 1;
    bbhTxPdWkup8_31.wkupthresh16_23     = BBH_TX_EPON_PD_FIFO_SIZE_16_23 - 1;
    bbhTxPdWkup8_31.wkupthresh24_31     = BBH_TX_EPON_PD_FIFO_SIZE_24_31 - 1;
    BBH_TX_CONFIGURATIONS_PDWKUPH8_31_WRITE(DRV_BBH_EPON,bbhTxPdWkup8_31);

    /*** BBH RX ***/
    /* bbh_rx_set_configuration */
    bbh_rx_configuration.dma_route_address = bbh_route_address_dma[DRV_BBH_EPON];
    bbh_rx_configuration.bpm_route_address = bbh_route_address_bpm[DRV_BBH_EPON];
    bbh_rx_configuration.sdma_route_address = bbh_route_address_sdma[DRV_BBH_EPON];
    bbh_rx_configuration.sbpm_route_address = bbh_route_address_sbpm[DRV_BBH_EPON];
    bbh_rx_configuration.runner_0_route_address = bbh_route_address_runner_0[DRV_BBH_EPON];
    bbh_rx_configuration.runner_1_route_address = bbh_route_address_runner_1[DRV_BBH_EPON];
    bbh_rx_configuration.ih_route_address = bbh_route_address_ih[DRV_BBH_EPON];

    bbh_rx_configuration.ddr_buffer_size = f_rdpa_bbh_ddr_buffer_size();


    bbh_rx_configuration.ddr_tm_base_address = (uint32_t)ddr_tm_base_address_phys;
    bbh_rx_configuration.pd_fifo_base_address_normal_queue_in_8_byte = f_get_bbh_rx_pd_fifo_base_address_normal_queue_in_8_byte(DRV_BBH_EPON);
    bbh_rx_configuration.pd_fifo_base_address_direct_queue_in_8_byte = f_get_bbh_rx_pd_fifo_base_address_direct_queue_in_8_byte(DRV_BBH_EPON);
    bbh_rx_configuration.pd_fifo_size_normal_queue = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    bbh_rx_configuration.pd_fifo_size_direct_queue = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    bbh_rx_configuration.runner_0_task_normal_queue = f_bbh_rx_runner_task_normal_queue(DRV_IH_RUNNER_CLUSTER_A, DRV_BBH_EPON);
    bbh_rx_configuration.runner_0_task_direct_queue = f_bbh_rx_runner_task_direct_queue(DRV_IH_RUNNER_CLUSTER_A, DRV_BBH_EPON);
    bbh_rx_configuration.runner_1_task_normal_queue = f_bbh_rx_runner_task_normal_queue(DRV_IH_RUNNER_CLUSTER_B, DRV_BBH_EPON);
    bbh_rx_configuration.runner_1_task_direct_queue = f_bbh_rx_runner_task_direct_queue(DRV_IH_RUNNER_CLUSTER_B, DRV_BBH_EPON);
    bbh_rx_configuration.dma_data_fifo_base_address = bbh_rx_dma_data_fifo_base_address[DRV_BBH_EPON];
    bbh_rx_configuration.dma_chunk_descriptor_fifo_base_address = bbh_rx_dma_chunk_descriptor_fifo_base_address[DRV_BBH_EPON];
    bbh_rx_configuration.dma_data_and_chunk_descriptor_fifos_size = BBH_RX_DMA_FIFOS_SIZE_WAN_GPON;
    bbh_rx_configuration.dma_exclusive_threshold = BBH_RX_DMA_EXCLUSIVE_THRESHOLD_WAN_BBH;
    bbh_rx_configuration.sdma_data_fifo_base_address = bbh_rx_sdma_data_fifo_base_address[DRV_BBH_EPON];
    bbh_rx_configuration.sdma_chunk_descriptor_fifo_base_address = bbh_rx_sdma_chunk_descriptor_fifo_base_address[DRV_BBH_EPON];
    bbh_rx_configuration.sdma_data_and_chunk_descriptor_fifos_size = bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[DRV_BBH_EPON];
    bbh_rx_configuration.sdma_exclusive_threshold = bbh_rx_sdma_exclusive_threshold[DRV_BBH_EPON];


    bbh_rx_configuration.minimum_packet_size_0 = MIN_ETH_PKT_SIZE;
    bbh_rx_configuration.minimum_packet_size_1 = MIN_ETH_PKT_SIZE;
    bbh_rx_configuration.minimum_packet_size_2 = MIN_ETH_PKT_SIZE;
    bbh_rx_configuration.minimum_packet_size_3 = MIN_ETH_PKT_SIZE;
    bbh_rx_configuration.maximum_packet_size_0 = pDpiCfg->mtu_size;
    bbh_rx_configuration.maximum_packet_size_1 = pDpiCfg->mtu_size;
    bbh_rx_configuration.maximum_packet_size_2 = pDpiCfg->mtu_size;
    bbh_rx_configuration.maximum_packet_size_3 = pDpiCfg->mtu_size;

    bbh_rx_configuration.ih_ingress_buffers_bitmask = bbh_ih_ingress_buffers_bitmask[DRV_BBH_EPON];

    bbh_rx_configuration.packet_header_offset = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    bbh_rx_configuration.reassembly_offset_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION(pDpiCfg->headroom_size);

    /* By default, the triggers for FC will be disabled and the triggers for drop enabled.
       If the user configures flow control for the port, the triggers for drop will be
       disabled and triggers for FC (including Runner request) will be enabled */
    bbh_rx_configuration.flow_control_triggers_bitmask = 0;
    bbh_rx_configuration.drop_triggers_bitmask = DRV_BBH_RX_DROP_TRIGGER_SBPM_IS_IN_EXCLUSIVE_STATE;

    bbh_rx_configuration.flows_32_255_group_divider = BBH_RX_FLOWS_32_255_GROUP_DIVIDER;
    bbh_rx_configuration.ploam_default_ih_class = DRV_RDD_IH_CLASS_WAN_CONTROL_INDEX;
    bbh_rx_configuration.ploam_ih_class_override = 0;

    /* Enable runner task for inter-wan option */
    bbh_rx_configuration.runner_1_task_direct_queue = WAN_TO_WAN_THREAD_NUMBER;
    fi_bl_drv_bbh_rx_set_configuration(DRV_BBH_EPON, &bbh_rx_configuration);

    /* bbh_rx_set_per_flow_configuration */
    per_flow_configuration.minimum_packet_size_selection = BBH_RX_ETH_MIN_PKT_SIZE_SELECTION_INDEX;
    per_flow_configuration.maximum_packet_size_selection = BBH_RX_MAX_PKT_SIZE_SELECTION_INDEX;
    per_flow_configuration.default_ih_class = DRV_RDD_IH_CLASS_WAN_BRIDGED_LOW_INDEX;
    per_flow_configuration.ih_class_override = 1;

    /* initialize all flows (including the 2 groups) */
    for (i = 0; i <= DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_1; i++)
        fi_bl_drv_bbh_rx_set_per_flow_configuration(DRV_BBH_EPON, i, &per_flow_configuration);

    /* configure group 0 (we use one group only, of flows 32-255) */
    per_flow_configuration.minimum_packet_size_selection = BBH_RX_ETH_MIN_PKT_SIZE_SELECTION_INDEX;
    per_flow_configuration.maximum_packet_size_selection = BBH_RX_MAX_PKT_SIZE_SELECTION_INDEX;
    per_flow_configuration.default_ih_class = DRV_RDD_IH_CLASS_WAN_BRIDGED_LOW_INDEX;
    per_flow_configuration.ih_class_override = 1;

    fi_bl_drv_bbh_rx_set_per_flow_configuration(DRV_BBH_EPON, DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_0, &per_flow_configuration);

    /* bbh_rx_get_per_flow_counters */
    /* the per-flow counters must be read at initialization stage, in order to clear them */
    for (i = 0; i < DRV_BBH_RX_NUMBER_OF_FLOWS; i++)
        fi_bl_drv_bbh_rx_get_per_flow_counters(DRV_BBH_EPON, i, &number_of_packets_dropped_by_ih);

    /*configure the extras for epon cfg*/
    BBH_TX_EPON_CFG_TASKLSB_READ(DRV_BBH_EPON,epon_Cfg.tasklsb);
    epon_Cfg.tasklsb.task0 = WAN_TX_THREAD_NUMBER;
    epon_Cfg.tasklsb.task1 = WAN_TX_THREAD_NUMBER;
    epon_Cfg.tasklsb.task2 = WAN_TX_THREAD_NUMBER;
    epon_Cfg.tasklsb.task3 = WAN_TX_THREAD_NUMBER;
    BBH_TX_EPON_CFG_TASKLSB_WRITE(DRV_BBH_EPON,epon_Cfg.tasklsb);

    BBH_TX_EPON_CFG_TASKMSB_READ(DRV_BBH_EPON,epon_Cfg.taskmsb);
    epon_Cfg.taskmsb.task4 = WAN_TX_THREAD_NUMBER;
    epon_Cfg.taskmsb.task5 = WAN_TX_THREAD_NUMBER;
    epon_Cfg.taskmsb.task6 = WAN_TX_THREAD_NUMBER;
    epon_Cfg.taskmsb.task7 = WAN_TX_THREAD_NUMBER;
    BBH_TX_EPON_CFG_TASKMSB_WRITE(DRV_BBH_EPON,epon_Cfg.taskmsb);


    BBH_TX_EPON_CFG_TASK8_39_READ(DRV_BBH_EPON, epon_Cfg.task8_39);
    epon_Cfg.task8_39.task8_39  = WAN_TX_THREAD_NUMBER;
    BBH_TX_EPON_CFG_TASK8_39_WRITE(DRV_BBH_EPON, epon_Cfg.task8_39);


    BBH_TX_EPON_CFG_PDSIZE0_3_READ(DRV_BBH_EPON, epon_Cfg.pdsize0_3);
    epon_Cfg.pdsize0_3.fifosize0 = BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    epon_Cfg.pdsize0_3.fifosize1 = BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    epon_Cfg.pdsize0_3.fifosize2 = BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    epon_Cfg.pdsize0_3.fifosize3 = BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    BBH_TX_EPON_CFG_PDSIZE0_3_WRITE(DRV_BBH_EPON, epon_Cfg.pdsize0_3);

    BBH_TX_EPON_CFG_PDSIZE4_7_READ(DRV_BBH_EPON, epon_Cfg.pdsize4_7);
    epon_Cfg.pdsize4_7.fifosize4 = BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    epon_Cfg.pdsize4_7.fifosize5 = BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    epon_Cfg.pdsize4_7.fifosize6 = BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    epon_Cfg.pdsize4_7.fifosize7 = BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    BBH_TX_EPON_CFG_PDSIZE4_7_WRITE(DRV_BBH_EPON, epon_Cfg.pdsize4_7);

    BBH_TX_EPON_CFG_PDSIZE8_31_READ(DRV_BBH_EPON, epon_Cfg.pdsize8_31);
    epon_Cfg.pdsize8_31.fifosize8_15 = BBH_TX_EPON_PD_FIFO_SIZE_8_15 - 1;
    epon_Cfg.pdsize8_31.fifosize16_23 = BBH_TX_EPON_PD_FIFO_SIZE_16_23 - 1;
    epon_Cfg.pdsize8_31.fifosize24_31 = BBH_TX_EPON_PD_FIFO_SIZE_24_31 - 1;
    BBH_TX_EPON_CFG_PDSIZE8_31_WRITE(DRV_BBH_EPON, epon_Cfg.pdsize8_31);

    BBH_TX_EPON_CFG_PDBASE0_3_READ(DRV_BBH_EPON, epon_Cfg.pdbase0_3);
    epon_Cfg.pdbase0_3.fifobase0 =  BBH_TX_EPON_PD_FIFO_BASE_0;
    epon_Cfg.pdbase0_3.fifobase1 =  BBH_TX_EPON_PD_FIFO_BASE_1;
    epon_Cfg.pdbase0_3.fifobase2 =  BBH_TX_EPON_PD_FIFO_BASE_2;
    epon_Cfg.pdbase0_3.fifobase3 =  BBH_TX_EPON_PD_FIFO_BASE_3;
    BBH_TX_EPON_CFG_PDBASE0_3_WRITE(DRV_BBH_EPON, epon_Cfg.pdbase0_3);

    BBH_TX_EPON_CFG_PDBASE4_7_READ(DRV_BBH_EPON, epon_Cfg.pdbase4_7);
    epon_Cfg.pdbase4_7.fifobase4 =  BBH_TX_EPON_PD_FIFO_BASE_4;
    epon_Cfg.pdbase4_7.fifobase5 =  BBH_TX_EPON_PD_FIFO_BASE_5;
    epon_Cfg.pdbase4_7.fifobase6 =  BBH_TX_EPON_PD_FIFO_BASE_6;
    epon_Cfg.pdbase4_7.fifobase7 =  BBH_TX_EPON_PD_FIFO_BASE_7;
    BBH_TX_EPON_CFG_PDBASE4_7_WRITE(DRV_BBH_EPON, epon_Cfg.pdbase4_7);

    BBH_TX_EPON_CFG_PDBASE8_39_WRITE(DRV_BBH_EPON, epon_Cfg.pdbase8_39);
    epon_Cfg.pdbase8_39.fifobase8_15 =  BBH_TX_EPON_PD_FIFO_BASE_8_15;
    epon_Cfg.pdbase8_39.fifobase16_23 =  BBH_TX_EPON_PD_FIFO_BASE_16_23;
    epon_Cfg.pdbase8_39.fifobase24_31 =  BBH_TX_EPON_PD_FIFO_BASE_24_31;
    epon_Cfg.pdbase8_39.fifobase32_39 =  BBH_TX_EPON_PD_FIFO_BASE_32_39;
    BBH_TX_EPON_CFG_PDBASE8_39_WRITE(DRV_BBH_EPON, epon_Cfg.pdbase8_39);

    BBH_TX_EPON_CFG_RUNNERCFG_READ(DRV_BBH_EPON, epon_Cfg.runnercfg);
    epon_Cfg.runnercfg.tcontaddr = MS_BYTE_TO_8_BYTE_RESOLUTION(BBH_TX_WAN_CHANNEL_INDEX_ADDRESS);
    BBH_TX_EPON_CFG_RUNNERCFG_WRITE(DRV_BBH_EPON, epon_Cfg.runnercfg);


    /*In EPON the IH_CLASS0 shall be 9*/
    BBH_RX_GENERAL_CONFIGURATION_IHCLASS0_WRITE(DRV_BBH_EPON,eponIHclass0);

    epon_Cfg.pdwkuph0_3.wkupthresh0 = BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    epon_Cfg.pdwkuph0_3.wkupthresh1 = BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    epon_Cfg.pdwkuph0_3.wkupthresh2 = BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    epon_Cfg.pdwkuph0_3.wkupthresh3 = BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    BBH_TX_EPON_CFG_PDWKUPH0_3_WRITE(DRV_BBH_EPON,epon_Cfg.pdwkuph0_3);

    epon_Cfg.pdwkuph4_7.wkupthresh4 =  BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    epon_Cfg.pdwkuph4_7.wkupthresh5 =  BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    epon_Cfg.pdwkuph4_7.wkupthresh6 =  BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    epon_Cfg.pdwkuph4_7.wkupthresh7 =  BBH_TX_EPON_PD_FIFO_SIZE_0_7 - 1;
    BBH_TX_EPON_CFG_PDWKUPH4_7_WRITE(DRV_BBH_EPON,epon_Cfg.pdwkuph4_7);

    epon_Cfg.pdwkuph8_31.wkupthresh8_15  =  BBH_TX_EPON_PD_FIFO_SIZE_8_15 - 1;
    epon_Cfg.pdwkuph8_31.wkupthresh16_23 =  BBH_TX_EPON_PD_FIFO_SIZE_16_23 - 1;
    epon_Cfg.pdwkuph8_31.wkupthresh24_31 =  BBH_TX_EPON_PD_FIFO_SIZE_24_31 - 1;

    BBH_TX_EPON_CFG_PDWKUPH8_31_WRITE(DRV_BBH_EPON,epon_Cfg.pdwkuph8_31);
}
#endif


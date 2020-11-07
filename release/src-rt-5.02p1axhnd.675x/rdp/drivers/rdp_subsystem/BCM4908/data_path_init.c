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
/* This file contains the implementation for Broadcom's 4908 Data path        */
/* initialization sequence                                                    */
/*                                                                            */
/******************************************************************************/

#include "data_path_init.h"
#include "rdpa_types.h"
#include "rdpa_config.h"
#ifdef CONFIG_DHD_RUNNER
#include "rdpa_dhd_helper_basic.h"
#endif
#include "rdd_ih_defs.h"
#include "bdmf_interface.h"
#include "rdp_drv_bbh.h"
#include "rdp_drv_ih.h"
#include "rdp_dma.h"
#include "rdp_drv_sbpm.h"
#include "rdp_drv_bpm.h"
#include "rdp_natcache.h"
#include "rdp_mm.h"
#ifndef FIRMWARE_INIT
#include "hwapi_mac.h"
#endif
#include "rdd.h"
#include "rdd_runner_defs.h"
#include "rdd_runner_defs_auto.h"
#include "rdd_init.h"
#include "rdd_tm.h"

#if defined(BDMF_SYSTEM_SIM)
#define __print(fmt, arg...)
#elif defined(_CFE_)
#define __print(fmt, arg...) printf(fmt, ##arg)
#else
#define __print(fmt, arg...) printk(fmt, ##arg)
#endif

static int bbh_ddr_buffer_size_kb = DRV_BBH_DDR_BUFFER_SIZE_2_KB;
static int fpm_pool_size_max = 16;

uint8_t * g_backup_queues_mem_virt_addr = 0;
uint32_t g_backup_queues_mem_phys_addr = 0;

#define DRV_BBH_DDR_BPM_MESSAGE_FORMAT DRV_BBH_DDR_BPM_MESSAGE_FORMAT_15_BIT_BN_WIDTH
#define DRV_SPARE_BN_MESSAGE_FORMAT DRV_SPARE_BN_MESSAGE_FORMAT_15_bit_BN_WIDTH

#ifdef __KERNEL__
extern void fpm_reset_bb(bool reset);
extern void fpm_set_pool_sel_both(void);
#endif

static data_path_init_params *p_dpi_cfg;

uint8_t *soc_base_address;
#if defined(__KERNEL__) && !defined(FIRMWARE_INIT)
struct device *rdp_dummy_dev = NULL;
#endif

uint16_t g_congestion_threshold_disable_mask = 0;

/* firmware externs */
extern uint32_t firmware_binary_A[];
extern uint32_t firmware_binary_B[];
extern uint32_t firmware_binary_C[];
extern uint32_t firmware_binary_D[];
extern uint16_t firmware_predict_A[];
extern uint16_t firmware_predict_B[];
extern uint16_t firmware_predict_C[];
extern uint16_t firmware_predict_D[];


/* MACROS */
#define MS_BYTE_TO_8_BYTE_RESOLUTION(address)  ((address) >> 3)
#define BBH_PORT_IS_WAN(port) (port == DRV_BBH_WAN)

/* route addresses (for both TX & RX) */
static const uint8_t bbh_route_address_dma[DRV_BBH_NUMBER_OF_PORTS]=
{
    BBH_ROUTE_ADDRESS_DMA_EMAC0,
    BBH_ROUTE_ADDRESS_DMA_EMAC1,
    BBH_ROUTE_ADDRESS_DMA_EMAC2,
    BBH_ROUTE_ADDRESS_DMA_WAN,
};

static const uint8_t bbh_route_address_bpm[DRV_BBH_NUMBER_OF_PORTS]=
{
    BBH_ROUTE_ADDRESS_BPM_EMAC0,
    BBH_ROUTE_ADDRESS_BPM_EMAC1,
    BBH_ROUTE_ADDRESS_BPM_EMAC2,
    BBH_ROUTE_ADDRESS_BPM_WAN,
};

static const uint8_t bbh_route_address_sdma[DRV_BBH_NUMBER_OF_PORTS]=
{
    BBH_ROUTE_ADDRESS_SDMA_EMAC0,
    BBH_ROUTE_ADDRESS_SDMA_EMAC1,
    BBH_ROUTE_ADDRESS_SDMA_EMAC2,
    BBH_ROUTE_ADDRESS_SDMA_WAN,
};

static const uint8_t bbh_route_address_sbpm[DRV_BBH_NUMBER_OF_PORTS]=
{
    BBH_ROUTE_ADDRESS_SBPM_EMAC0,
    BBH_ROUTE_ADDRESS_SBPM_EMAC1,
    BBH_ROUTE_ADDRESS_SBPM_EMAC2,
    BBH_ROUTE_ADDRESS_SBPM_WAN,
};

static const uint8_t bbh_route_address_runner_0[DRV_BBH_NUMBER_OF_PORTS]=
{
    BBH_ROUTE_ADDRESS_RUNNER0_EMAC0,
    BBH_ROUTE_ADDRESS_RUNNER0_EMAC1,
    BBH_ROUTE_ADDRESS_RUNNER0_EMAC2,
    BBH_ROUTE_ADDRESS_RUNNER0_WAN,
};

static const uint8_t bbh_route_address_runner_1[DRV_BBH_NUMBER_OF_PORTS]=
{
    BBH_ROUTE_ADDRESS_RUNNER1_EMAC0,
    BBH_ROUTE_ADDRESS_RUNNER1_EMAC1,
    BBH_ROUTE_ADDRESS_RUNNER1_EMAC2,
    BBH_ROUTE_ADDRESS_RUNNER1_WAN,
};

static const uint8_t bbh_route_address_ih[DRV_BBH_NUMBER_OF_PORTS] =
{
    BBH_ROUTE_ADDRESS_IH_EMAC0,
    BBH_ROUTE_ADDRESS_IH_EMAC1,
    BBH_ROUTE_ADDRESS_IH_EMAC2,
    BBH_ROUTE_ADDRESS_IH_WAN,
};

/* same values for DMA & SDMA */
static const uint8_t bbh_dma_and_sdma_read_requests_fifo_base_address[DRV_BBH_NUMBER_OF_PORTS]=
{
    BBH_DMA_FIFO_ADDRESS_BASE_EMAC0,
    BBH_DMA_FIFO_ADDRESS_BASE_EMAC1,
    BBH_DMA_FIFO_ADDRESS_BASE_EMAC2,
    BBH_DMA_FIFO_ADDRESS_BASE_WAN,
};

static uint16_t bbh_ih_ingress_buffers_bitmask[DRV_BBH_NUMBER_OF_PORTS];

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
        /* WAN */
        DRV_RDD_IH_CLASS_WAN_INDEX,
        {
            DRV_RDD_IH_LOOKUP_TABLE_MAC_DA_INDEX, /* da_lookup_required */
            DRV_RDD_IH_LOOKUP_TABLE_MAC_SA_INDEX, /* sa_lookup_required */
            DRV_RDD_IH_CLASS_13_CLASS_SEARCH_3,
            DRV_RDD_IH_LOOKUP_TABLE_DS_INGRESS_CLASSIFICATION_INDEX,
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
        /* LAN bridged eth0 */
        DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH0_INDEX,
        {
            DRV_RDD_IH_LOOKUP_TABLE_MAC_DA_INDEX, /* da_lookup_required */
            DRV_RDD_IH_LOOKUP_TABLE_MAC_SA_INDEX, /* sa_lookup_required */
            DRV_RDD_IH_CLASS_10_CLASS_SEARCH_3,
            DRV_RDD_IH_LOOKUP_TABLE_US_INGRESS_CLASSIFICATION_INDEX,
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
            DRV_RDD_IH_LOOKUP_TABLE_US_INGRESS_CLASSIFICATION_INDEX,
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
            DRV_RDD_IH_LOOKUP_TABLE_US_INGRESS_CLASSIFICATION_INDEX,
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
        /* WIFI */
        DRV_RDD_IH_CLASS_WIFI_INDEX,
        {
            DRV_RDD_IH_LOOKUP_TABLE_MAC_DA_INDEX, /* da_lookup_required */
            DRV_RDD_IH_LOOKUP_TABLE_MAC_SA_INDEX, /* sa_lookup_required */
            DRV_RDD_IH_CLASS_2_CLASS_SEARCH_3,
            DRV_RDD_IH_CLASS_2_CLASS_SEARCH_4,
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
};

static const rdd_rdd_emac bbh_to_rdd_emac_map[] =
{
        RDD_EMAC_ID_0 ,
        RDD_EMAC_ID_1 ,
        RDD_EMAC_ID_2 ,
        RDD_EMAC_ID_3 ,
};

static int fpm_ddr2_enabled = 0;

typedef struct
{
    uint32_t bpm_def_thresh;    
    uint32_t bpm_gbl_thresh;    
} rdp_bpm_cfg_params;

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

static void fi_dma_configure_memory_allocation (E_BB_MODULE module_id,E_DMA_PERIPHERAL peripheral_id,
                                                uint32_t data_memory_offset_address,uint32_t cd_memory_offset_address,
                                                uint32_t number_of_buffers)
{
    DMA_REGS_CONFIG_MALLOC config_malloc ;

    DMA_REGS_CONFIG_MALLOC_READ( module_id, peripheral_id, config_malloc ) ;
    config_malloc.datatoffset = data_memory_offset_address ;
    config_malloc.cdoffset = cd_memory_offset_address ;
    config_malloc.numofbuff = number_of_buffers ;
    DMA_REGS_CONFIG_MALLOC_WRITE( module_id, peripheral_id, config_malloc ) ;
}

static void update_bbh_ih_ingress_buffers_bitmask(DRV_BBH_PORT_INDEX bbh_port_index, uint8_t base_location, uint8_t queue_size)
{
    uint16_t bitmask = 0;
    uint8_t i;

    /* set '1's according to queue_size  */
    for (i=0; i < queue_size; i++)
        bitmask |= (1<<i);

    /* do shifting according to xi_base_location */
    bitmask <<= base_location;

    /* update in database */
    bbh_ih_ingress_buffers_bitmask[bbh_port_index] = bitmask;
}

static int ih_configure_ingress_queues(void)
{
    DRV_IH_INGRESS_QUEUE_CONFIG ingress_queue_config;
    uint8_t base_location = 0;
    uint32_t i;

    /* queues of EMACS 0-3 */
    for (i = IH_INGRESS_QUEUE_0_ETH0; i <= IH_INGRESS_QUEUE_2_ETH2; i++)
    {
        ingress_queue_config.base_location = base_location;
        ingress_queue_config.size = IH_INGRESS_QUEUE_SIZE_LAG_EMACS;
        ingress_queue_config.priority = IH_INGRESS_QUEUE_PRIORITY_LAG_EMACS;
        ingress_queue_config.weight = IH_INGRESS_QUEUE_WEIGHT_LAG_EMACS;
        ingress_queue_config.congestion_threshold = IH_INGRESS_QUEUE_CONGESTION_THRESHOLD_LAG_EMACS;
        g_congestion_threshold_disable_mask |= CONGESTION_THRESHOLD_DISABLE_MASK_SET(CTD_RUNNER_A, i);

        fi_bl_drv_ih_configure_ingress_queue(i, &ingress_queue_config);

        /* update the correspoding bitmask in database, to be configured in BBH */
        update_bbh_ih_ingress_buffers_bitmask((DRV_BBH_PORT_INDEX)i, ingress_queue_config.base_location, ingress_queue_config.size);

        base_location += ingress_queue_config.size;
    }

    /* queues of WAN */
    ingress_queue_config.base_location = base_location;
    ingress_queue_config.size = IH_INGRESS_QUEUE_SIZE_WAN;
    ingress_queue_config.priority = IH_INGRESS_QUEUE_PRIORITY_WAN;
    ingress_queue_config.weight = IH_INGRESS_QUEUE_WEIGHT_WAN;
    ingress_queue_config.congestion_threshold = IH_INGRESS_QUEUE_CONGESTION_THRESHOLD_WAN;
    g_congestion_threshold_disable_mask |= CONGESTION_THRESHOLD_DISABLE_MASK_SET(CTD_RUNNER_B, IH_INGRESS_QUEUE_5_WAN);

    fi_bl_drv_ih_configure_ingress_queue(IH_INGRESS_QUEUE_5_WAN, &ingress_queue_config);

    update_bbh_ih_ingress_buffers_bitmask(DRV_BBH_WAN, ingress_queue_config.base_location,
        ingress_queue_config.size);

    base_location += ingress_queue_config.size;

    /* queues of runner A */
    ingress_queue_config.base_location = base_location;
    ingress_queue_config.size = IH_INGRESS_QUEUE_SIZE_RUNNERS;
    ingress_queue_config.priority = IH_INGRESS_QUEUE_PRIORITY_RUNNERS;
    ingress_queue_config.weight = IH_INGRESS_QUEUE_WEIGHT_RUNNERS;
    ingress_queue_config.congestion_threshold = IH_INGRESS_QUEUE_CONGESTION_THRESHOLD_RUNNERS;

    fi_bl_drv_ih_configure_ingress_queue (IH_INGRESS_QUEUE_6_RUNNER_A, &ingress_queue_config);

    base_location += ingress_queue_config.size;

    /* queues of runner B */
    ingress_queue_config.base_location = base_location;
    ingress_queue_config.size = IH_INGRESS_QUEUE_SIZE_RUNNERS;
    ingress_queue_config.priority = IH_INGRESS_QUEUE_PRIORITY_RUNNERS;
    ingress_queue_config.weight = IH_INGRESS_QUEUE_WEIGHT_RUNNERS;
    ingress_queue_config.congestion_threshold = IH_INGRESS_QUEUE_CONGESTION_THRESHOLD_RUNNERS;

    fi_bl_drv_ih_configure_ingress_queue (IH_INGRESS_QUEUE_7_RUNNER_B, &ingress_queue_config);

    base_location += ingress_queue_config.size;

    /* check that we didn't overrun */
    if (base_location > DRV_IH_INGRESS_QUEUES_ARRAY_SIZE)
    {
        /* sum of sizes exceeded the total array size */
        return -1;
    }

    return 0;
}

static void ih_configure_lookup_tables(void)
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

    fi_bl_drv_ih_configure_lut_60_bit_key(DRV_RDD_IH_LOOKUP_TABLE_DS_INGRESS_CLASSIFICATION_INDEX,
        &lookup_table_60_bit_key_config);

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

    fi_bl_drv_ih_configure_lut_60_bit_key(DRV_RDD_IH_LOOKUP_TABLE_US_INGRESS_CLASSIFICATION_INDEX,
        &lookup_table_60_bit_key_config);
}

/* this function configures the IH classes which are in use */
static void ih_configure_classes(void)
{
    uint8_t i;

    for (i = 0; i < ARRAY_LENGTH(gs_ih_classes); i++)
    {
        fi_bl_drv_ih_configure_class(gs_ih_classes[i].class_index, &gs_ih_classes[i].class_config);
    }
}

/* if xi_ethertype is one of the constants of IH (8100, 88A8, 9100, 9200), this
   function configures qtag nesting accordingly. otherwise, a user-defined ethertype
   and qtag nesting are configured, according to xi_index (should be 0 or 1) */
static void fi_ih_parser_set_ingress_ether_type(uint16_t ethertype, uint8_t index, uint32_t status)
{
    uint16_t ethertype_0;
    uint16_t ethertype_1;
    DRV_IH_ETHERTYPE_FOR_QTAG_NESTING ih_ethertype_index;

    switch(ethertype)
    {
    case 0x8100:
        fi_bl_drv_ih_configure_qtag_nesting(DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_8100, status, status);
        break;

    case 0x88A8:
        fi_bl_drv_ih_configure_qtag_nesting(DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_88A8, status, status);
        break;

    case 0x9100:
        fi_bl_drv_ih_configure_qtag_nesting(DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_9100, status, status);
        break;

    case 0x9200:
        fi_bl_drv_ih_configure_qtag_nesting(DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_9200, status, status);
        break;

        /* if it is none of the constant ethertypes, we configure a user-defined ethertype */
    default:
        fi_bl_drv_ih_get_ethertypes_for_qtag_identification(&ethertype_0, &ethertype_1);

        if (index == 0)
        {
            ethertype_0 = ethertype;
            ih_ethertype_index = DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_USER_DEFIEND_0;
        }
        else
        {
            ethertype_1 = ethertype;
            ih_ethertype_index = DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_USER_DEFIEND_1;
        }

        fi_bl_drv_ih_set_ethertypes_for_qtag_identification (ethertype_0, ethertype_1);
        fi_bl_drv_ih_configure_qtag_nesting(ih_ethertype_index, status, status);
    }
}

static void ih_configure_parser(void)
{
    DRV_IH_PARSER_CONFIG parser_config;
    uint8_t i;

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

    /* Configure VLAN EtherType (Ingress) */
    for (i = DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_USER_DEFIEND_0; i < DRV_IH_NUMBER_OF_ETHERTYPES_FOR_QTAG_NESTING; i++)
    {
        fi_bl_drv_ih_configure_qtag_nesting(i, 0, 0);
    }

    /* Configure the pre - defined TPID*/
    fi_ih_parser_set_ingress_ether_type (RDPA_VLAN_ETH_TYPE_DETECT_1, 0, 1);

    fi_ih_parser_set_ingress_ether_type(RDPA_VLAN_ETH_TYPE_DETECT_2, 1,1);

    /* Configure the user - defined if they are valid TPID value*/
    fi_ih_parser_set_ingress_ether_type(RDPA_VLAN_ETH_TYPE_DETECT_3, 0, 0);

    fi_ih_parser_set_ingress_ether_type(RDPA_VLAN_ETH_TYPE_DETECT_4, 1, 0);


    /* set PPP protocol code for IPv4 */
    fi_bl_drv_ih_set_ppp_code(0,IH_PARSER_PPP_PROTOCOL_CODE_0_IPV4);

    /* set PPP protocol code for IPv6 */
    fi_bl_drv_ih_set_ppp_code(1, IH_PARSER_PPP_PROTOCOL_CODE_1_IPV6);

    for (i = IH_IP_L4_FILTER_USER_DEFINED_0; i <= IH_IP_L4_FILTER_USER_DEFINED_3; i ++)
    {
        /* set a user-defined L4 Protocol to 255 */
        fi_bl_drv_ih_set_user_ip_l4_protocol(i - IH_IP_L4_FILTER_USER_DEFINED_0, IH_L4_FILTER_DEF);
    }

}

static void ih_init(void)
{
    DRV_IH_GENERAL_CONFIG ih_general_config;
    DRV_IH_PACKET_HEADER_OFFSETS packet_header_offsets;
    DRV_IH_RUNNER_BUFFERS_CONFIG runner_buffers_config;
    DRV_IH_RUNNERS_LOAD_THRESHOLDS runners_load_thresholds;
    DRV_IH_ROUTE_ADDRESSES xi_route_addresses;
    DRV_IH_LOGICAL_PORTS_CONFIG logical_ports_config;
    DRV_IH_SOURCE_PORT_TO_INGRESS_QUEUE_MAPPING source_port_to_ingress_queue_mapping;
    DRV_IH_WAN_PORTS_CONFIG wan_ports_config;
    int i;

    /* ingress queues configuration */
    ih_configure_ingress_queues();

    /* general configuration */
    ih_general_config.runner_a_ih_response_address             = MS_BYTE_TO_8_BYTE_RESOLUTION(RUNNER_FLOW_IH_RESPONSE_ADDRESS);
    ih_general_config.runner_b_ih_response_address             = MS_BYTE_TO_8_BYTE_RESOLUTION(RUNNER_FLOW_IH_RESPONSE_ADDRESS);
    ih_general_config.runner_a_ih_congestion_report_address    = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_0_IH_CONGESTION_REPORT_ADDRESS);
    ih_general_config.runner_b_ih_congestion_report_address    = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_1_IH_CONGESTION_REPORT_ADDRESS);
    ih_general_config.runner_a_ih_congestion_report_enable     = DRV_RDD_IH_RUNNER_0_IH_CONGESTION_REPORT_ENABLE;
    ih_general_config.runner_b_ih_congestion_report_enable     = DRV_RDD_IH_RUNNER_1_IH_CONGESTION_REPORT_ENABLE;
    ih_general_config.lut_searches_enable_in_direct_mode       = 1;
    ih_general_config.sn_stamping_enable_in_direct_mode        = 1;
    ih_general_config.header_length_minimum                    = IH_HEADER_LENGTH_MIN;
    ih_general_config.congestion_discard_disable               = 0;
    ih_general_config.cam_search_enable_upon_invalid_lut_entry = DRV_RDD_IH_CAM_SEARCH_ENABLE_UPON_INVALID_LUT_ENTRY;
    ih_general_config.congestion_threshold_disable = g_congestion_threshold_disable_mask;

    fi_bl_drv_ih_set_general_configuration(&ih_general_config);

    /* packet header offsets configuration */
    packet_header_offsets.eth0_packet_header_offset = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    packet_header_offsets.eth1_packet_header_offset = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    packet_header_offsets.eth2_packet_header_offset = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    packet_header_offsets.eth3_packet_header_offset = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    packet_header_offsets.eth4_packet_header_offset = 0; /* source port not used*/
    packet_header_offsets.gpon_packet_header_offset = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    packet_header_offsets.runner_a_packet_header_offset = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    packet_header_offsets.runner_b_packet_header_offset = DRV_RDD_IH_PACKET_HEADER_OFFSET;

    fi_bl_drv_ih_set_packet_header_offsets(& packet_header_offsets);

    /* Runner Buffers configuration */
    /* same ih_managed_rb_base_address should be used for both runners */
    runner_buffers_config.runner_a_ih_managed_rb_base_address     = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_0_IH_MANAGED_RB_BASE_ADDRESS);
    runner_buffers_config.runner_b_ih_managed_rb_base_address     = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_1_IH_MANAGED_RB_BASE_ADDRESS);
    runner_buffers_config.runner_a_runner_managed_rb_base_address = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_0_RUNNER_MANAGED_RB_BASE_ADDRESS);
    runner_buffers_config.runner_b_runner_managed_rb_base_address = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_1_RUNNER_MANAGED_RB_BASE_ADDRESS);
    runner_buffers_config.runner_a_maximal_number_of_buffers      = DRV_IH_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS_32;
    runner_buffers_config.runner_b_maximal_number_of_buffers      = DRV_IH_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS_32;

    fi_bl_drv_ih_set_runner_buffers_configuration(&runner_buffers_config);

    /* runners load thresholds configuration */
    runners_load_thresholds.runner_a_high_congestion_threshold      = IH_DS_RUNNER_A_HIGH_CONGESTION_THRESHOLD;
    runners_load_thresholds.runner_b_high_congestion_threshold      = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    runners_load_thresholds.runner_a_exclusive_congestion_threshold = IH_DS_RUNNER_A_EXCLUSIVE_CONGESTION_THRESHOLD;
    runners_load_thresholds.runner_b_exclusive_congestion_threshold = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    runners_load_thresholds.runner_a_load_balancing_threshold       = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    runners_load_thresholds.runner_b_load_balancing_threshold       = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    runners_load_thresholds.runner_a_load_balancing_hysteresis      = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    runners_load_thresholds.runner_b_load_balancing_hysteresis      = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;

    fi_bl_drv_ih_set_runners_load_thresholds (& runners_load_thresholds);

    /* route addresses configuration */
    xi_route_addresses.eth0_route_address     = IH_ETH0_ROUTE_ADDRESS;
    xi_route_addresses.eth1_route_address     = IH_ETH1_ROUTE_ADDRESS;
    xi_route_addresses.eth2_route_address     = IH_ETH2_ROUTE_ADDRESS;
    xi_route_addresses.eth3_route_address     = 0;
    xi_route_addresses.eth4_route_address     = 0;
    xi_route_addresses.gpon_route_address     = IH_WAN_ROUTE_ADDRESS;
    xi_route_addresses.runner_a_route_address = IH_RUNNER_A_ROUTE_ADDRESS;
    xi_route_addresses.runner_b_route_address = IH_RUNNER_B_ROUTE_ADDRESS;

    fi_bl_drv_ih_set_route_addresses(&xi_route_addresses);

    /* logical ports configuration */
    logical_ports_config.eth0_config.parsing_layer_depth      = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_ETH0_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.eth0_config.proprietary_tag_size     =  DRV_IH_PROPRIETARY_TAG_SIZE_4 ;

    logical_ports_config.eth1_config.parsing_layer_depth      = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_ETH1_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.eth1_config.proprietary_tag_size     =  DRV_IH_PROPRIETARY_TAG_SIZE_4;

    logical_ports_config.eth2_config.parsing_layer_depth      = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_ETH2_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.eth2_config.proprietary_tag_size     =  DRV_IH_PROPRIETARY_TAG_SIZE_4;

    logical_ports_config.eth3_config.parsing_layer_depth      = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_ETH3_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.eth3_config.proprietary_tag_size     = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    logical_ports_config.eth4_config.parsing_layer_depth      = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_ETH4_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.eth4_config.proprietary_tag_size     = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    logical_ports_config.gpon_config.parsing_layer_depth      = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_GPON_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.gpon_config.proprietary_tag_size     = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    logical_ports_config.runner_a_config.parsing_layer_depth  = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_RNRA_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.runner_a_config.proprietary_tag_size = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    logical_ports_config.runner_b_config.parsing_layer_depth  = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_RNRB_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.runner_b_config.proprietary_tag_size = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    logical_ports_config.pcie0_config.parsing_layer_depth     = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_PCIE0_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.pcie0_config.proprietary_tag_size    = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    logical_ports_config.pcie1_config.parsing_layer_depth     = IH_REGS_GENERAL_CONFIGURATION_PARSE_LAYER_PER_PORT_CFG_PCIE1_PARSE_LAYER_STG_PARSE_LAYER_VALUE_RESET_VALUE;
    logical_ports_config.pcie1_config.proprietary_tag_size    = DRV_IH_PROPRIETARY_TAG_SIZE_0;

    fi_bl_drv_ih_set_logical_ports_configuration(&logical_ports_config);

    /* source port to ingress queue mapping */
    source_port_to_ingress_queue_mapping.eth0_ingress_queue     = IH_INGRESS_QUEUE_0_ETH0;
    source_port_to_ingress_queue_mapping.eth1_ingress_queue     = IH_INGRESS_QUEUE_1_ETH1;
    source_port_to_ingress_queue_mapping.eth2_ingress_queue     = IH_INGRESS_QUEUE_2_ETH2;
    source_port_to_ingress_queue_mapping.eth3_ingress_queue     = IH_INGRESS_QUEUE_3_NOT_USED;
    source_port_to_ingress_queue_mapping.eth4_ingress_queue     = IH_INGRESS_QUEUE_4_NOT_USED;
    source_port_to_ingress_queue_mapping.gpon_ingress_queue     = IH_INGRESS_QUEUE_5_WAN;
    source_port_to_ingress_queue_mapping.runner_a_ingress_queue = IH_INGRESS_QUEUE_6_RUNNER_A;
    source_port_to_ingress_queue_mapping.runner_b_ingress_queue = IH_INGRESS_QUEUE_7_RUNNER_B;

    fi_bl_drv_ih_set_source_port_to_ingress_queue_mapping(&source_port_to_ingress_queue_mapping);

    /* enabling the 2 dscp to pbit tables */
    for (i = 0; i < DRV_IH_NUMBER_OF_DSCP_TO_TCI_TABLES; i++)
        fi_bl_drv_ih_enable_dscp_to_tci_table(i, 1);

    /* configure wan ports */
    wan_ports_config.eth0 = 0;
    wan_ports_config.eth1 = 0;
    wan_ports_config.eth2 = 0;
    wan_ports_config.eth3 = 0;
    wan_ports_config.eth4 = 0;
    wan_ports_config.gpon = 1;
    wan_ports_config.runner_a = 0;
    wan_ports_config.runner_b = 0;
    wan_ports_config.pcie0 = 0;
    wan_ports_config.pcie1 = 0;

    fi_bl_drv_ih_configure_wan_ports(&wan_ports_config);

    ih_configure_lookup_tables();

    ih_configure_classes();

    ih_configure_parser();

#if 0 /* target matrix not in use for now */
    ih_configure_target_matrix();
#endif
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

    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_EMAC1, DRV_SBPM_USER_GROUP_0);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_EMAC2, DRV_SBPM_USER_GROUP_0);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_EMAC3, DRV_SBPM_USER_GROUP_0);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_EMAC4, DRV_SBPM_USER_GROUP_0);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_GPON, DRV_SBPM_USER_GROUP_0);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_RNR_A, DRV_SBPM_USER_GROUP_0);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_RNR_B, DRV_SBPM_USER_GROUP_0);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_MIPS_C, DRV_SBPM_USER_GROUP_0);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_MIPS_D, DRV_SBPM_USER_GROUP_0);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_SPARE_0, DRV_SBPM_USER_GROUP_0);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_SPARE_1, DRV_SBPM_USER_GROUP_0);
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
    if (max_bpm_bufs < VAL_5K)
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
        *p_bpm_gbl_thresh = DRV_BPM_GLOBAL_THRESHOLD_15K;
        *p_bpm_def_thresh = VAL_15K;
    }
}

static uint32_t rdp_bpm_buf_size(void)
{
    if (bbh_ddr_buffer_size_kb == DRV_BBH_DDR_BUFFER_SIZE_2_KB)
    {
        return VAL_2K;
    }
    else if (bbh_ddr_buffer_size_kb == DRV_BBH_DDR_BUFFER_SIZE_4_KB)
    {
        return VAL_4K;
    }
    else if (bbh_ddr_buffer_size_kb == DRV_BBH_DDR_BUFFER_SIZE_16_KB)
    {
        return VAL_16K;
    }
    return 0; /* Error */
}

static void rdp_bpm_cfg_params_get(uint32_t ddr_size, rdp_bpm_cfg_params *p_bpm_cfg_params)
{
    uint32_t max_bpm_bufs, bpm_buf_size;

    bpm_buf_size = rdp_bpm_buf_size();
    
    max_bpm_bufs = (ddr_size*1024*1024)/bpm_buf_size;
    rdp_bpm_threshod_get(max_bpm_bufs, &p_bpm_cfg_params->bpm_gbl_thresh, &p_bpm_cfg_params->bpm_def_thresh);
    __print("\n RDP PKT memory = %uMB : Max Possible Bufs <%u> of size <%u>; Allocating <%u> bufs; RDP enum <%u>\n",
            ddr_size, max_bpm_bufs, bpm_buf_size, p_bpm_cfg_params->bpm_def_thresh, p_bpm_cfg_params->bpm_gbl_thresh );
}

static void bpm_drv_init(void)
{
    DRV_BPM_RUNNER_MSG_CTRL_PARAMS runner_msg_ctrl_params;
    DRV_BPM_USER_GROUP_CONFIGURATION bpm_ug_configuration;
    rdp_bpm_cfg_params   bpm_cfg_params;

    init_bpm_virt_base();

    rdp_bpm_cfg_params_get(p_dpi_cfg->runner_ddr_bm_size, &bpm_cfg_params);

    fi_bl_drv_bpm_init(NULL, NULL, 0, DRV_SPARE_BN_MESSAGE_FORMAT);

    /*Set up user group 0. It's the only one that can be used for this chip*/
    bpm_ug_configuration.hysteresis = BPM_DEFAULT_HYSTERESIS;
    bpm_ug_configuration.threshold = bpm_cfg_params.bpm_def_thresh;
    bpm_ug_configuration.exclusive_hysteresis = BPM_DEFAULT_HYSTERESIS;
    bpm_ug_configuration.exclusive_threshold = bpm_cfg_params.bpm_def_thresh;

    fi_bl_drv_bpm_set_user_group_thresholds (DRV_BPM_USER_GROUP_0, 
                                             &bpm_ug_configuration);

    fi_bl_drv_bpm_get_runner_msg_ctrl(&runner_msg_ctrl_params);

    runner_msg_ctrl_params.runner_a_reply_target_address = (BPM_REPLY_RUNNER_A_ADDRESS + 0x10000) >> 3;
    runner_msg_ctrl_params.runner_b_reply_target_address = (BPM_REPLY_RUNNER_B_ADDRESS + 0x10000) >> 3;

    fi_bl_drv_bpm_set_runner_msg_ctrl(&runner_msg_ctrl_params);
}

static void init_bbh_dma_sdma_related_arrays(void)
{
    int bbh;
    uint8_t dma_base_address = 0;
    uint8_t sdma_base_address = 0;

    /* first allocate the DMA fifo for LAG ports towards the SF2 */
    for (bbh = DRV_BBH_EMAC_0; bbh <= DRV_BBH_EMAC_2; bbh++)
    {
        bbh_rx_dma_data_and_chunk_descriptor_fifos_size[bbh] = BBH_RX_DMA_FIFOS_SIZE_LAG;
        bbh_rx_dma_exclusive_threshold[bbh] = BBH_RX_DMA_EXCLUSIVE_THRESHOLD_LAG;
        bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[bbh] = BBH_RX_SDMA_FIFOS_SIZE_LAG;
        bbh_rx_sdma_exclusive_threshold[bbh] = BBH_RX_SDMA_EXCLUSIVE_THRESHOLD_LAG;

        bbh_rx_dma_data_fifo_base_address[bbh] = dma_base_address;
        bbh_rx_dma_chunk_descriptor_fifo_base_address[bbh] = dma_base_address;
        bbh_rx_sdma_data_fifo_base_address[bbh] = sdma_base_address;
        bbh_rx_sdma_chunk_descriptor_fifo_base_address[bbh] = sdma_base_address;

        dma_base_address += bbh_rx_dma_data_and_chunk_descriptor_fifos_size[bbh];
        sdma_base_address += bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[bbh];
    }

    /* now allcate the remaining for Ethernet WAN*/
    bbh_rx_dma_data_and_chunk_descriptor_fifos_size[DRV_BBH_WAN] = BBH_RX_DMA_FIFOS_SIZE_WAN;
    bbh_rx_dma_exclusive_threshold[DRV_BBH_WAN] = BBH_RX_DMA_EXCLUSIVE_THRESHOLD_WAN;
    bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[DRV_BBH_WAN] = BBH_RX_SDMA_FIFOS_SIZE_WAN;
    bbh_rx_sdma_exclusive_threshold[DRV_BBH_WAN] = BBH_RX_SDMA_EXCLUSIVE_THRESHOLD_WAN;

    bbh_rx_dma_data_fifo_base_address[DRV_BBH_WAN] = dma_base_address;
    bbh_rx_dma_chunk_descriptor_fifo_base_address[DRV_BBH_WAN] = dma_base_address;
    bbh_rx_sdma_data_fifo_base_address[DRV_BBH_WAN] = sdma_base_address;
    bbh_rx_sdma_chunk_descriptor_fifo_base_address[DRV_BBH_WAN] = sdma_base_address;

    dma_base_address += bbh_rx_dma_data_and_chunk_descriptor_fifos_size[DRV_BBH_WAN];
    sdma_base_address += bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[DRV_BBH_WAN];
}

static void init_dma_sdma(void)
{
    E_DMA_PERIPHERAL peripheral_id ;
    DMA_REGS_CONFIG_U_THRESH dma_thresh;
    DMA_REGS_CONFIG_DDR_SEL0 dma_mem_sel;

    /* emac 0-2 */
    for ( peripheral_id = DMA_PERIPHERAL_EMAC_0 ; peripheral_id <= DMA_PERIPHERAL_EMAC_2 ; ++peripheral_id )
    {
        fi_dma_configure_memory_allocation(BB_MODULE_DMA, peripheral_id,
            bbh_rx_dma_data_fifo_base_address[peripheral_id],
            bbh_rx_dma_chunk_descriptor_fifo_base_address[peripheral_id],
            bbh_rx_dma_data_and_chunk_descriptor_fifos_size[peripheral_id]);
        /* SDMA */
        fi_dma_configure_memory_allocation(BB_MODULE_SDMA,
            peripheral_id,
            bbh_rx_sdma_data_fifo_base_address[peripheral_id],
            bbh_rx_sdma_chunk_descriptor_fifo_base_address[peripheral_id],
            bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[peripheral_id]) ;
    }

    {
        //handle the RGMII port
        fi_dma_configure_memory_allocation(BB_MODULE_DMA, DMA_PERIPHERAL_WAN,
            bbh_rx_dma_data_fifo_base_address[DRV_BBH_WAN],
            bbh_rx_dma_chunk_descriptor_fifo_base_address[DRV_BBH_WAN],
            bbh_rx_dma_data_and_chunk_descriptor_fifos_size[DRV_BBH_WAN]);
            /* SDMA */
        fi_dma_configure_memory_allocation(BB_MODULE_SDMA, DMA_PERIPHERAL_WAN,
            bbh_rx_sdma_data_fifo_base_address[DRV_BBH_WAN],
            bbh_rx_sdma_chunk_descriptor_fifo_base_address[DRV_BBH_WAN],
            bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[DRV_BBH_WAN]) ;
    }

    dma_thresh.out_of_u = 3;
    dma_thresh.into_u = 5;
    DMA_REGS_CONFIG_U_THRESH_WRITE(BB_MODULE_DMA,DMA_PERIPHERAL_EMAC_0,dma_thresh);
    DMA_REGS_CONFIG_U_THRESH_WRITE(BB_MODULE_DMA,DMA_PERIPHERAL_EMAC_1,dma_thresh);
    DMA_REGS_CONFIG_U_THRESH_WRITE(BB_MODULE_DMA,DMA_PERIPHERAL_EMAC_2,dma_thresh);
    DMA_REGS_CONFIG_U_THRESH_WRITE(BB_MODULE_DMA,DMA_PERIPHERAL_WAN,dma_thresh);

    /*4908 only has one DDR*/

    dma_mem_sel.ddr_base = 0x0;
    dma_mem_sel.ddr_mask = 0x8000;
    DMA_REGS_CONFIG_DDR_SEL0_WRITE(BB_MODULE_DMA, dma_mem_sel);

    dma_mem_sel.ddr_base = 0x8000;
    dma_mem_sel.ddr_mask = 0x8000;
    DMA_REGS_CONFIG_DDR_SEL1_WRITE(BB_MODULE_DMA, dma_mem_sel);
}

static uint16_t f_get_bbh_rx_pd_fifo_base_address_normal_queue_in_8_byte(DRV_BBH_PORT_INDEX port_index)
{
    uint16_t desc_addr;

    switch (port_index)
    {
    case DRV_BBH_EMAC_0:
        desc_addr = MS_BYTE_TO_8_BYTE_RESOLUTION(ETH0_RX_DESCRIPTORS_ADDRESS);
        break;
    case DRV_BBH_EMAC_1:
        desc_addr = MS_BYTE_TO_8_BYTE_RESOLUTION(ETH1_RX_DESCRIPTORS_ADDRESS);
        break;
    case DRV_BBH_EMAC_2:
        desc_addr = MS_BYTE_TO_8_BYTE_RESOLUTION(ETH2_RX_DESCRIPTORS_ADDRESS);
        break;
    case DRV_BBH_WAN:
        desc_addr = MS_BYTE_TO_8_BYTE_RESOLUTION(GPON_RX_NORMAL_DESCRIPTORS_ADDRESS);
        break;

    default:
        desc_addr = 0;
    }

    return desc_addr;
}

static void init_single_bbh(DRV_BBH_PORT_INDEX bbh)
{
    uint16_t mdu_mode_read_pointer_address_in_byte;
    uint32_t mdu_mode_read_pointer_address_in_byte_uint32;
    DRV_BBH_TX_CONFIGURATION bbh_tx_configuration = {};
    DRV_BBH_RX_CONFIGURATION bbh_rx_configuration = {};
    DRV_BBH_PER_FLOW_CONFIGURATION per_flow_configuration = {};

    /*** BBH TX ***/
    bbh_tx_configuration.dma_route_address = bbh_route_address_dma[bbh];
    bbh_tx_configuration.bpm_route_address = bbh_route_address_bpm[bbh];
    bbh_tx_configuration.sdma_route_address = bbh_route_address_sdma[bbh];
    bbh_tx_configuration.sbpm_route_address = bbh_route_address_sbpm[bbh];

    /*Runner 1 handles WAN, Runner 0 handles LAN*/
    if (bbh == DRV_BBH_WAN)
        bbh_tx_configuration.runner_route_address = bbh_route_address_runner_1[bbh];
    else
        bbh_tx_configuration.runner_route_address = bbh_route_address_runner_0[bbh];

    bbh_tx_configuration.ddr_buffer_size = DRV_BBH_DDR_BUFFER_SIZE_RESERVED;
    bbh_tx_configuration.payload_offset_resolution = DRV_BBH_PAYLOAD_OFFSET_RESOLUTION_1_B;

    /* multicast memory will be after the runner table memory */
#if defined(CONFIG_BCM_RDPA_MCAST) || defined(FIRMWARE_INIT)
    bbh_tx_configuration.ddr1_multicast_headers_base_address_in_byte = (uint32_t)(uintptr_t)p_dpi_cfg->rdp_ddr_fm_phys + RDP_DDR_DATA_STRUCTURES_SIZE;
#else
    bbh_tx_configuration.ddr1_multicast_headers_base_address_in_byte = 0U;
#endif
    bbh_tx_configuration.ddr1_tm_base_address = (uint32_t)(uintptr_t)p_dpi_cfg->rdp_ddr_bm_phys;
    if (fpm_ddr2_enabled == 0)
    {
        bbh_tx_configuration.ddr2_multicast_headers_base_address_in_byte = 0U;
        bbh_tx_configuration.ddr2_tm_base_address = 0U;
    }
    else
    {
        /* in current 4908 design, we are not going to have multicast traffic
         * for buffer from 2nd pool, so we don't need to configure the
         * following value yet. */
        //bbh_tx_configuration.ddr2_multicast_headers_base_address_in_byte = (uint32_t)(uintptr_t)p_dpi_cfg->rdp_ddr_fm_phys + RDP_DDR_DATA_STRUCTURES_SIZE + RDP_DDR_MC_HEADER_SIZE;
        bbh_tx_configuration.ddr2_multicast_headers_base_address_in_byte = 0U;
        bbh_tx_configuration.ddr2_tm_base_address = (uint32_t)(uintptr_t)p_dpi_cfg->rdp_ddr_bm_phys + (fpm_pool_size_max << 20);
    }

    bbh_tx_configuration.fpm_buff_size_set =  p_dpi_cfg->fpm_buff_size_set;

    if (BBH_PORT_IS_WAN(bbh))
    {
        bbh_tx_configuration.multicast_header_size = BBH_MULTICAST_HEADER_SIZE_FOR_LAN_PORT;
        /* this is the task also in Gbe case */
        bbh_tx_configuration.task_0 = WAN1_TX_THREAD_NUMBER;
        bbh_tx_configuration.skb_address = MS_BYTE_TO_8_BYTE_RESOLUTION(ETHWAN_ABSOLUTE_TX_BBH_COUNTER_ADDRESS);
        bbh_tx_configuration.byoi_no_fpm_release = 0;
    }
    else
    /*we will reach here in case the BBH port in lan ports 0 - 2*/
    {
        bbh_tx_configuration.multicast_header_size = BBH_MULTICAST_HEADER_SIZE_FOR_LAN_PORT;

        bbh_tx_configuration.skb_address = MS_BYTE_TO_8_BYTE_RESOLUTION(EMAC_ABSOLUTE_TX_BBH_COUNTER_ADDRESS)
                        + bbh_to_rdd_emac_map[bbh];
        bbh_tx_configuration.task_0 = ETH_TX_THREAD_NUMBER;
    }
    /* other task numbers are irrelevant */
    bbh_tx_configuration.task_1 = 0;
    bbh_tx_configuration.task_2 = 0;
    bbh_tx_configuration.task_3 = 0;
    bbh_tx_configuration.task_4 = 0;
    bbh_tx_configuration.task_5 = 0;
    bbh_tx_configuration.task_6 = 0;
    bbh_tx_configuration.task_7 = 0;
    bbh_tx_configuration.task_8_39 = 0;

    if (BBH_PORT_IS_WAN(bbh))
    {
        bbh_tx_configuration.mdu_mode_enable = 0;
        /* irrelevant in this case */
        bbh_tx_configuration.mdu_mode_read_pointer_address_in_8_byte = 0;

        bbh_tx_configuration.pd_fifo_size_0 =  BBH_TX_EMAC_PD_FIFO_SIZE_MDU_MODE_DISABLED;
    }
    else
    {
        bbh_tx_configuration.mdu_mode_enable = 1;

        bbh_tx_configuration.pd_fifo_size_0 =  BBH_TX_EMAC_PD_FIFO_SIZE_MDU_MODE_ENABLED;

        rdd_mdu_mode_pointer_get((BL_LILAC_RDD_EMAC_ID_DTE)(bbh + 1), &mdu_mode_read_pointer_address_in_byte);

        mdu_mode_read_pointer_address_in_byte_uint32 = mdu_mode_read_pointer_address_in_byte;

        /* after division, this will be back a 16 bit number */
        bbh_tx_configuration.mdu_mode_read_pointer_address_in_8_byte = (uint16_t)MS_BYTE_TO_8_BYTE_RESOLUTION(mdu_mode_read_pointer_address_in_byte_uint32);
    }

    /* other FIFOs are irrelevant (relevant for GPON only) */
    bbh_tx_configuration.pd_fifo_size_1 = 0;
    bbh_tx_configuration.pd_fifo_size_2 = 0;
    bbh_tx_configuration.pd_fifo_size_3 = 0;
    bbh_tx_configuration.pd_fifo_size_4 = 0;
    bbh_tx_configuration.pd_fifo_size_5 = 0;
    bbh_tx_configuration.pd_fifo_size_6 = 0;
    bbh_tx_configuration.pd_fifo_size_7 = 0;
    bbh_tx_configuration.pd_fifo_size_8_15 = 0;
    bbh_tx_configuration.pd_fifo_size_16_23 = 0;
    bbh_tx_configuration.pd_fifo_size_24_31 = 0;
    bbh_tx_configuration.pd_fifo_size_32_39 = 0;

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

    bbh_tx_configuration.dma_read_requests_fifo_base_address = bbh_dma_and_sdma_read_requests_fifo_base_address[bbh];
    bbh_tx_configuration.dma_read_requests_maximal_number = BBH_TX_CONFIGURATIONS_DMACFG_TX_MAXREQ_MAX_VALUE_VALUE_RESET_VALUE;
    bbh_tx_configuration.sdma_read_requests_fifo_base_address = bbh_dma_and_sdma_read_requests_fifo_base_address[bbh];
    bbh_tx_configuration.sdma_read_requests_maximal_number = BBH_TX_CONFIGURATIONS_SDMACFG_TX_MAXREQ_MAX_VALUE_VALUE_RESET_VALUE;

    /* irrelevant in EMAC case */
    if (bbh == DRV_BBH_WAN)
        bbh_tx_configuration.tcont_address_in_8_byte =  MS_BYTE_TO_8_BYTE_RESOLUTION(BBH_TX_WAN_CHANNEL_INDEX_ADDRESS);
    else
        bbh_tx_configuration.tcont_address_in_8_byte =  0;

    bbh_tx_configuration.emac_1588_enable = 0;

    fi_bl_drv_bbh_tx_set_configuration(bbh, &bbh_tx_configuration);

    /*** BBH RX ***/
    /* bbh_rx_set_configuration */
    bbh_rx_configuration.dma_route_address = bbh_route_address_dma[bbh];
    bbh_rx_configuration.bpm_route_address = bbh_route_address_bpm[bbh];
    bbh_rx_configuration.sdma_route_address = bbh_route_address_sdma[bbh];
    bbh_rx_configuration.sbpm_route_address = bbh_route_address_sbpm[bbh];
    bbh_rx_configuration.runner_0_route_address = bbh_route_address_runner_0[bbh];
    bbh_rx_configuration.runner_1_route_address = bbh_route_address_runner_1[bbh];
    bbh_rx_configuration.ih_route_address = bbh_route_address_ih[bbh];

    bbh_rx_configuration.ddr_buffer_size = DRV_BBH_DDR_BUFFER_SIZE_RESERVED;
    bbh_rx_configuration.ddr1_tm_base_address = (uint32_t)(uintptr_t)p_dpi_cfg->rdp_ddr_bm_phys;
    if (fpm_ddr2_enabled == 0)
        bbh_rx_configuration.ddr2_tm_base_address = 0U;
    else
        bbh_rx_configuration.ddr2_tm_base_address = (uint32_t)(uintptr_t)p_dpi_cfg->rdp_ddr_bm_phys + (fpm_pool_size_max << 20);

    bbh_rx_configuration.pd_fifo_base_address_normal_queue_in_8_byte = f_get_bbh_rx_pd_fifo_base_address_normal_queue_in_8_byte(bbh);
    bbh_rx_configuration.pd_fifo_base_address_direct_queue_in_8_byte = 0;
    bbh_rx_configuration.pd_fifo_size_normal_queue = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    bbh_rx_configuration.pd_fifo_size_direct_queue = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    bbh_rx_configuration.runner_0_task_normal_queue = BBH_PORT_IS_WAN(bbh) ? DS_RX_BUFFER_COPY_THREAD_NUMBER : 0;
    bbh_rx_configuration.runner_0_task_direct_queue = 0;
    bbh_rx_configuration.runner_1_task_normal_queue = BBH_PORT_IS_WAN(bbh) ? 0 : US_RX_BUFFER_COPY_THREAD_NUMBER + (bbh - DRV_BBH_EMAC_0);
    bbh_rx_configuration.runner_1_task_direct_queue = 0;

    bbh_rx_configuration.dma_data_fifo_base_address = bbh_rx_dma_data_fifo_base_address[bbh];
    bbh_rx_configuration.dma_chunk_descriptor_fifo_base_address = bbh_rx_dma_chunk_descriptor_fifo_base_address[bbh];
    bbh_rx_configuration.sdma_data_fifo_base_address = bbh_rx_sdma_data_fifo_base_address[bbh];
    bbh_rx_configuration.sdma_chunk_descriptor_fifo_base_address = bbh_rx_sdma_chunk_descriptor_fifo_base_address[bbh ];
    bbh_rx_configuration.dma_data_and_chunk_descriptor_fifos_size = bbh_rx_dma_data_and_chunk_descriptor_fifos_size[bbh];
    bbh_rx_configuration.dma_exclusive_threshold =  bbh_rx_dma_exclusive_threshold[bbh];
    bbh_rx_configuration.sdma_data_and_chunk_descriptor_fifos_size = bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[bbh ];
    bbh_rx_configuration.sdma_exclusive_threshold = bbh_rx_sdma_exclusive_threshold[bbh];

    if (BBH_PORT_IS_WAN(bbh))
    {
        bbh_rx_configuration.minimum_packet_size_0 = MIN_ETH_PKT_SIZE_WAN;
        bbh_rx_configuration.packet_header_offset = DRV_RDD_IH_PACKET_HEADER_OFFSET;

#if 0 /* hardware dispatch is not currently being used */
        /* parallel processing */
        bbh_rx_configuration.pp_task_enable_bitmap = 0xf;
        bbh_rx_configuration.pp_task_nums[0] = DS_PROCESSING_1_THREAD_NUMBER;
        bbh_rx_configuration.pp_task_nums[1] = DS_PROCESSING_2_THREAD_NUMBER;
        bbh_rx_configuration.pp_task_nums[2] = DS_PROCESSING_3_THREAD_NUMBER;
#endif
    }
    else
    {
        bbh_rx_configuration.minimum_packet_size_0 = MIN_ETH_PKT_SIZE_LAN;
        bbh_rx_configuration.packet_header_offset = DRV_RDD_IH_PACKET_HEADER_OFFSET;
    }

    bbh_rx_configuration.minimum_packet_size_1 = 0;
    bbh_rx_configuration.minimum_packet_size_2 = 0;
    bbh_rx_configuration.minimum_packet_size_3 = 0;

    bbh_rx_configuration.maximum_packet_size_0 = p_dpi_cfg->mtu_size;

    /* maximum_packet_size 1-3 are not in use */
    bbh_rx_configuration.maximum_packet_size_1 = p_dpi_cfg->mtu_size;
    bbh_rx_configuration.maximum_packet_size_2 = p_dpi_cfg->mtu_size;
    bbh_rx_configuration.maximum_packet_size_3 = p_dpi_cfg->mtu_size;

    bbh_rx_configuration.ih_ingress_buffers_bitmask = bbh_ih_ingress_buffers_bitmask[bbh];

    bbh_rx_configuration.reassembly_offset_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION(p_dpi_cfg->headroom_size);

    /* By default, the triggers for FC will be disabled and the triggers for drop enabled.
       If the user configures flow control for the port, the triggers for drop will be
       disabled and triggers for FC (including Runner request) will be enabled */
    bbh_rx_configuration.flow_control_triggers_bitmask = 0;
    bbh_rx_configuration.drop_triggers_bitmask = DRV_BBH_RX_DROP_TRIGGER_BPM_IS_IN_EXCLUSIVE_STATE | DRV_BBH_RX_DROP_TRIGGER_SBPM_IS_IN_EXCLUSIVE_STATE;

    /* following configuration is irrelevant in EMAC case */
    bbh_rx_configuration.flows_32_255_group_divider = 0;
    bbh_rx_configuration.ploam_default_ih_class = 0;
    bbh_rx_configuration.ploam_ih_class_override = 0;
    bbh_rx_configuration.fpm_buff_size_set = p_dpi_cfg->fpm_buff_size_set;

    fi_bl_drv_bbh_rx_set_configuration(bbh, &bbh_rx_configuration);

    /* bbh_rx_set_per_flow_configuration */
    per_flow_configuration.minimum_packet_size_selection = 0;
    per_flow_configuration.maximum_packet_size_selection = 0;

    per_flow_configuration.ih_class_override = 0;

    switch (bbh)
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
    case DRV_BBH_WAN:
        per_flow_configuration.default_ih_class = DRV_RDD_IH_CLASS_WAN_INDEX;
        per_flow_configuration.ih_class_override = 1;
        break;
    default:
        return;
        break;
    }

    /* in EMAC, only flow 0 is relevant */
    fi_bl_drv_bbh_rx_set_per_flow_configuration(bbh, 0, &per_flow_configuration);
}

static void init_bbh_ports(void)
{
    DRV_BBH_PORT_INDEX bbhidx;

    for (bbhidx = DRV_BBH_EMAC_0; bbhidx <= DRV_BBH_EMAC_2; bbhidx++)
    {
        init_single_bbh(bbhidx);
    }

    init_single_bbh(DRV_BBH_WAN);
}

static void configure_runner(void)
{
#ifdef __KERNEL__
    uint32_t natc_key_table_size;
    uint32_t natc_key_context_size;
    uint32_t context_cont_size;
#endif
    /* Local Variables */
    RDD_INIT_PARAMS rdd_init_params = {0};

    /* zero Runner memories (data, program and context) */
    rdd_init();

    rdd_load_microcode((uint8_t *)firmware_binary_A, (uint8_t *)firmware_binary_B,
        (uint8_t*)firmware_binary_C, (uint8_t *)firmware_binary_D);

    rdd_load_prediction((uint8_t *)firmware_predict_A, (uint8_t *)firmware_predict_B,
        (uint8_t *)firmware_predict_C, (uint8_t *)firmware_predict_D);

    rdd_init_params.ddr_bm_ptr = p_dpi_cfg->rdp_ddr_bm_base;
    rdd_init_params.ddr_bm_phys = (uint32_t)(uintptr_t)p_dpi_cfg->rdp_ddr_bm_phys;
    if (fpm_ddr2_enabled != 0)
        rdd_init_params.ddr1_bm_phys = (uint32_t)(uintptr_t)p_dpi_cfg->rdp_ddr_bm_phys + (fpm_pool_size_max << 20);
    rdd_init_params.ddr_fm_ptr = p_dpi_cfg->rdp_ddr_fm_base;
    rdd_init_params.ddr_fm_phys = (uint32_t)(uintptr_t)p_dpi_cfg->rdp_ddr_fm_phys;
#ifdef __KERNEL__
    natc_key_table_size = RDP_NATC_KEY_TABLE_SIZE;
    natc_key_context_size = RDP_NATC_CONTEXT_TABLE_SIZE;
    context_cont_size = RDP_CONTEXT_CONTINUATION_TABLE_SIZE;
    rdd_init_params.runner_nat_cache_key_ptr = rdd_init_params.ddr_fm_ptr + RDP_NATC_KEY_TABLE_ADDR;
    rdd_init_params.runner_nat_cache_context_ptr = rdd_init_params.ddr_fm_ptr + RDP_NATC_CONTEXT_TABLE_ADDR;
    rdd_init_params.runner_context_cont_ptr = rdd_init_params.ddr_fm_ptr + RDP_CONTEXT_CONTINUATION_TABLE_ADDR;
    memset(rdd_init_params.runner_nat_cache_key_ptr, 0, natc_key_table_size);
    memset(rdd_init_params.runner_nat_cache_context_ptr, 0, natc_key_context_size);
    memset(rdd_init_params.runner_context_cont_ptr, 0, context_cont_size);

    printk("configure_runner: NATC keysize 0x%x key element size %ld contextsize 0x%x context element size %ld\n",
        natc_key_table_size, sizeof(RDD_NAT_CACHE_LKP_ENTRY_DTS),
        natc_key_context_size, sizeof(RDD_FC_NATC_UCAST_FLOW_CONTEXT_ENTRY_DTS));
#endif
    rdd_init_params.mac_table_size     = BL_LILAC_RDD_MAC_TABLE_SIZE_64;
    /* we don't have iptv table. the following is just a dummy */
    rdd_init_params.iptv_table_size    = BL_LILAC_RDD_MAC_TABLE_SIZE_256;
    rdd_init_params.ddr_headroom_size = p_dpi_cfg->headroom_size;
    rdd_init_params.bridge_flow_cache_mode = rdpa_method_none;

    rdd_init_params.broadcom_switch_mode = 1; /* managed */
    rdd_init_params.broadcom_switch_physical_port = RDD_EMAC_ID_0;
    rdd_init_params.cpu_tx_abs_packet_limit = RDD_CPU_TX_ABS_FIFO_SIZE;

    if (p_dpi_cfg->fpm_buff_size_set == DRV_BBH_FPM_BUFF_SIZE_512_1024_2048_4056)
        rdd_init_params.token_size = 512;
    else
        rdd_init_params.token_size = 256;
    rdd_init_params.lp_mode = 0;

    rdd_data_structures_init(&rdd_init_params);

#ifdef __KERNEL__
    printk("rdd_init_params.runner_nat_cache_key_ptr=0x%p\n", rdd_init_params.runner_nat_cache_key_ptr);
    printk("rdd_init_params.runner_nat_cache_context_ptr=0x%p\n", rdd_init_params.runner_nat_cache_context_ptr);
    printk("rdd_init_params.runner_context_cont_ptr=0x%p\n", rdd_init_params.runner_context_cont_ptr);
#endif
}

static void natcache_drv_init(void)
{
    uint32_t cntl_status_reg0 = 0;
    uint32_t cntl_status_reg1 = 0;
    uint32_t key_base = (uint32_t)(uintptr_t)p_dpi_cfg->rdp_ddr_fm_phys + RDP_NATC_KEY_TABLE_ADDR;
    uint32_t result_base = (uint32_t)(uintptr_t)p_dpi_cfg->rdp_ddr_fm_phys + RDP_NATC_CONTEXT_TABLE_ADDR;

    cntl_status_reg1 |= NATCACHE_RDP_CONTROL_STATUS2_AGE_TIMER_TICK;
    cntl_status_reg1 |= (DEFAULT_AGE_TIMER_32 << NATCACHE_RDP_CONTROL_STATUS2_AGE_TIMER_OFFSET);
    cntl_status_reg1 |= (DEFAULT_DDR_BINS_PER_BACKET << NATCACHE_RDP_CONTROL_STATUS2_DDR_BINS_PER_BUCKET_OFFSET);
    cntl_status_reg1 |= (NATCACHE_RDP_CONTROL_STATUS2_DDR_SIZE_VAL_64K << NATCACHE_RDP_CONTROL_STATUS2_DDR_SIZE_OFFSET);
    cntl_status_reg1 |=
        (NATCACHE_RDP_CONTROL_STATUS2_DDR_HASH_MODE_VAL_CRC32_16B_REDUCED << NATCACHE_RDP_CONTROL_STATUS2_DDR_HASH_MODE_OFFSET);

    cntl_status_reg0 = 0;
    cntl_status_reg0 |= NATCACHE_RDP_CONTROL_STATUS_DDR_ENABLE;
    cntl_status_reg0 |= NATCACHE_RDP_CONTROL_STATUS_DDR_DISABLE_ON_REG_LOOKUP;
    cntl_status_reg0 |= (NATCACHE_RDP_CONTROL_STATUS_TOTAL_LEN_VAL_80_BYTE << NATCACHE_RDP_CONTROL_STATUS_TOTAL_LEN_OFFSET);
    cntl_status_reg0 |= (0xF << NATCACHE_RDP_CONTROL_STATUS_MULTI_HASH_LIMIT_OFFSET);
    cntl_status_reg0 |= NATCACHE_RDP_CONTROL_STATUS_NATC_ENABLE;

    WRITE_32(NATCACHE_RDP_CONTROL_STATUS2,cntl_status_reg1);
    WRITE_32(NATCACHE_RDP_CONTROL_STATUS,cntl_status_reg0);

    WRITE_32(NATCACHE_RDP_DDR_KEY_BASE_ADDRESS_LOWER, key_base);
    WRITE_32(NATCACHE_RDP_DDR_RESULT_BASE_ADDRESS_LOWER, result_base);
}

static void init_ubus_masters(void)
{
    uint32_t reg;

    /*first Ubus Master*/
    reg = 0x00060601;
    WRITE_32(UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_EN + RDP_UBUS_MASTER_BRDG_REG_HP_OFFSET,reg);
    reg = 0x90010000;
    WRITE_32(UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_EN + 4,reg);

    /*second Ubus Master*/
    reg = 0x000f0d01;
    WRITE_32(UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_EN + RDP_UBUS_MASTER_BRDG_REG_HP_OFFSET,reg);
    reg = 0x90010000;
    WRITE_32(UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_EN + 4,reg);
    
    /*third Ubus Master*/
    reg = 0x000e0e01;
    WRITE_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_EN + RDP_UBUS_MASTER_BRDG_REG_HP_OFFSET,reg);
    reg = 0x90010000;
    WRITE_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_EN + 4,reg);

    reg = 0x00090901;
    WRITE_32(UBUS_MASTER_4_RDP_UBUS_MASTER_BRDG_REG_EN + RDP_UBUS_MASTER_BRDG_REG_HP_OFFSET,reg);
    reg = 0x90010000;
    WRITE_32(UBUS_MASTER_4_RDP_UBUS_MASTER_BRDG_REG_EN + 4,reg);

    /* enable all Ubus masters */
    reg = 0x1; // bit 0 is the enable bit
    WRITE_32(UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_EN,reg);
    WRITE_32(UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_EN,reg);
    WRITE_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_EN,reg);
    WRITE_32(UBUS_MASTER_4_RDP_UBUS_MASTER_BRDG_REG_EN,reg);
}

static void enable_source_ports(void)
{
    DRV_SBPM_SP_ENABLE sbpm_sp_enable = {0};

    fi_bl_drv_bpm_sp_enable(0, 1);

    sbpm_sp_enable.rnra_sp_enable = 1;
    sbpm_sp_enable.rnrb_sp_enable = 1;
    sbpm_sp_enable.gpon_or_eth5_sp_enable = 1;
    sbpm_sp_enable.eth0_sp_enable = 1;
    sbpm_sp_enable.eth1_sp_enable = 1;
    sbpm_sp_enable.eth2_sp_enable = 1;
    sbpm_sp_enable.eth3_sp_enable = 1;
    sbpm_sp_enable.eth4_sp_enable = 1;

    fi_bl_drv_sbpm_sp_enable(&sbpm_sp_enable);
}

#ifndef FIRMWARE_INIT
int init_rdp_virtual_mem(void)
{
#ifdef USE_SOC_BASE_ADDR
    soc_base_address = (uint8_t*)bdmf_ioremap(RDP_PHYS_BASE, RDP_SIZE);
    printk(_BBh_ "RDP Physical address=0x%x Virtual address = 0x%p"_BBnl_, RDP_PHYS_BASE, (void *)soc_base_address);
#endif
    return 0;
}

#ifdef __KERNEL__

#if !defined(CONFIG_BCM_FPM_POOL_NUM) || (CONFIG_BCM_FPM_POOL_NUM < 1)
#define FPM_POOL_NUM 1
#else
#define FPM_POOL_NUM CONFIG_BCM_FPM_POOL_NUM
#endif

static int check_reserved_memory(void)
{
    uint32_t sz_mb_required;

    /* We need to reserve 32MB MC memory per FPM Pool enabled.
     * If we have 2 FPM pools enabled, we usually need to reserve 64MB if
     * multicast can happen with buffer from 2nd pool.  We workaround this issue
     * to force multicast using only buffer from Pool#0, so we only need 32MB for MC
     * However, we do provide an option that we don't even enable multicast traffic
     * pushed to the runner to further save that 32MB */

#if FPM_POOL_NUM > 1
    fpm_ddr2_enabled = 1;
#endif

    /* TODO! implement this
     * based on build flag to see if RDP_DDR_MC_HEADER_SIZE needs to be included */
    sz_mb_required = (RDP_DDR_DATA_STRUCTURES_SIZE + RDP_DDR_MC_HEADER_SIZE + RDPA_DHD_BACKUP_QUEUE_RESERVED_SIZE) >> 20;
    
    if (p_dpi_cfg->runner_ddr_fm_size < sz_mb_required) {
        printk("You need %dMB flow memory, but only have reserved %dMB\n",
               sz_mb_required, p_dpi_cfg->runner_ddr_fm_size);
        return -1; 
    }

    return 0;
}

static void allocate_dummy_dev(void)
{
    if (rdp_dummy_dev == NULL) {
        rdp_dummy_dev = kzalloc(sizeof(struct device), GFP_ATOMIC);
        /* need to confirm how many bits we support in 4908 runner */
        dma_set_coherent_mask(rdp_dummy_dev, DMA_BIT_MASK(32));
    }
}
#endif

#endif

int data_path_init(data_path_init_params *dpi_params)
{
    p_dpi_cfg = dpi_params;

#ifndef FIRMWARE_INIT
#ifdef __KERNEL__
    if (check_reserved_memory())
    {
        printk(_BBh_ "FAILED TO INITIALIZE RDP DUE TO RESERVE MEMORY NOT ENOUGH\n" _BBnl_);
        return -1;
    }
#endif

#ifdef RDPA_DHD_HELPER_FEATURE_BACKUP_QUEUE_SUPPORT
    /* If reserved memory is okay, then set the global base address for backup queues */
    g_backup_queues_mem_virt_addr = (uint8_t*)p_dpi_cfg->rdp_ddr_fm_base + RDP_DDR_DATA_STRUCTURES_SIZE + RDP_DDR_MC_HEADER_SIZE;
    g_backup_queues_mem_phys_addr = RDD_RSV_VIRT_TO_PHYS(p_dpi_cfg->rdp_ddr_fm_base, p_dpi_cfg->rdp_ddr_fm_phys, g_backup_queues_mem_virt_addr);  
#else
    g_backup_queues_mem_virt_addr = 0;
    g_backup_queues_mem_phys_addr = 0;
#endif

    init_rdp_virtual_mem();

#ifdef __KERNEL__
    allocate_dummy_dev();
#endif
#endif

    if (p_dpi_cfg->fpm_buff_size_set == DRV_BBH_FPM_BUFF_SIZE_512_1024_2048_4056)
    {
        bbh_ddr_buffer_size_kb = DRV_BBH_DDR_BUFFER_SIZE_4_KB;
        fpm_pool_size_max = 32;
    }

    init_bbh_dma_sdma_related_arrays();

    /* init runner,load microcode and structures */
    configure_runner();

    rdd_runner_frequency_set(p_dpi_cfg->runner_freq);

    ih_init();

    sbpm_drv_init();

#ifdef __KERNEL__
    /* the FPM hold the Broad Bus in reset until all the Runner drivers are initilaized */
    fpm_reset_bb(false);

    /* enable fpm to select from both FPM pools */
    fpm_set_pool_sel_both();
#endif

    bpm_drv_init();

    natcache_drv_init();

    /* init DMA and SDMA */
    init_dma_sdma();

    init_bbh_ports();

    init_ubus_masters();

#ifndef FIRMWARE_INIT
    //rnr_wkup_cpuc_init();
#endif

    rdd_runner_enable();

    enable_source_ports();

    return 0;
}

int data_path_init_sim(data_path_init_params *dpi_params)
{
    p_dpi_cfg = dpi_params;

    init_bbh_dma_sdma_related_arrays();
    configure_runner();
    init_bbh_ports();
    return 0;
}


#if defined(__KERNEL__)
EXPORT_SYMBOL(soc_base_address);
EXPORT_SYMBOL(rdp_dummy_dev);
#endif


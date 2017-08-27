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
/* This file contains the implementation for Broadcom's 3390 Data path        */
/* initialization sequence                                                    */
/*                                                                            */
/******************************************************************************/

#include "data_path_init.h"
#include "rdd_ih_defs.h"
#include "bdmf_interface.h"
#include "rdp_drv_bbh.h"
#include "rdp_drv_ih.h"
#include "rdp_dma.h"
#include "rdp_drv_sbpm.h"
#include "rdp_drv_bpm.h"
#include "rdp_natcache.h"
#include "rdp_dqm_wkup.h"
#include "hwapi_mac.h"
#include "rdd.h"
#include "rdd_runner_defs.h"
#include "rdd_runner_defs_auto.h"
#include "rdd_init.h"
#include "rdd_tm.h"

static data_path_init_params *p_dpi_cfg;
uint8_t *soc_base_address;

#ifdef __KERNEL__
#ifdef CM3390
extern uint32_t g_runner_ddr0_iptv_lookup_ptr;
extern uint32_t g_runner_ddr0_iptv_context_ptr;
extern uint32_t g_runner_ddr0_iptv_ssm_context_ptr;
#endif
extern void fpm_reset_bb(int);
#endif

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
#define BBH_PORT_IS_WAN(port) (port == DRV_BBH_DOCSIS || port == DRV_BBH_BYOI)

/* route addresses (for both TX & RX) */
static const uint8_t bbh_route_address_dma[DRV_BBH_NUMBER_OF_PORTS] =
{
    BBH_ROUTE_ADDRESS_DMA_EMAC0,
    BBH_ROUTE_ADDRESS_DMA_EMAC1,
    BBH_ROUTE_ADDRESS_DMA_EMAC2,
    BBH_ROUTE_ADDRESS_DMA_DOCSIS,
    BBH_ROUTE_ADDRESS_DMA_BYOI
};

static const uint8_t bbh_route_address_bpm[DRV_BBH_NUMBER_OF_PORTS] =
{
    BBH_ROUTE_ADDRESS_BPM_EMAC0,
    BBH_ROUTE_ADDRESS_BPM_EMAC1,
    BBH_ROUTE_ADDRESS_BPM_EMAC2,
    BBH_ROUTE_ADDRESS_BPM_DOCSIS,
    BBH_ROUTE_ADDRESS_BPM_BYOI
};

static const uint8_t bbh_route_address_sdma[DRV_BBH_NUMBER_OF_PORTS] =
{
    BBH_ROUTE_ADDRESS_SDMA_EMAC0,
    BBH_ROUTE_ADDRESS_SDMA_EMAC1,
    BBH_ROUTE_ADDRESS_SDMA_EMAC2,
    BBH_ROUTE_ADDRESS_SDMA_DOCSIS,
    BBH_ROUTE_ADDRESS_SDMA_BYOI
};

static const uint8_t bbh_route_address_sbpm[DRV_BBH_NUMBER_OF_PORTS] =
{
    BBH_ROUTE_ADDRESS_SBPM_EMAC0,
    BBH_ROUTE_ADDRESS_SBPM_EMAC1,
    BBH_ROUTE_ADDRESS_SBPM_EMAC2,
    BBH_ROUTE_ADDRESS_SBPM_DOCSIS,
    BBH_ROUTE_ADDRESS_SBPM_BYOI
};

static const uint8_t bbh_route_address_runner_0[DRV_BBH_NUMBER_OF_PORTS] =
{
    BBH_ROUTE_ADDRESS_RUNNER0_EMAC0,
    BBH_ROUTE_ADDRESS_RUNNER0_EMAC1,
    BBH_ROUTE_ADDRESS_RUNNER0_EMAC2,
    BBH_ROUTE_ADDRESS_RUNNER0_DOCSIS,
    BBH_ROUTE_ADDRESS_RUNNER0_BYOI
};

static const uint8_t bbh_route_address_runner_1[DRV_BBH_NUMBER_OF_PORTS] =
{
    BBH_ROUTE_ADDRESS_RUNNER1_EMAC0,
    BBH_ROUTE_ADDRESS_RUNNER1_EMAC1,
    BBH_ROUTE_ADDRESS_RUNNER1_EMAC2,
    BBH_ROUTE_ADDRESS_RUNNER1_DOCSIS,
    BBH_ROUTE_ADDRESS_RUNNER1_BYOI
};

static const uint8_t bbh_route_address_ih[DRV_BBH_NUMBER_OF_PORTS] =
{
    BBH_ROUTE_ADDRESS_IH_EMAC0,
    BBH_ROUTE_ADDRESS_IH_EMAC1,
    BBH_ROUTE_ADDRESS_IH_EMAC2,
    BBH_ROUTE_ADDRESS_IH_DOCSIS,
    BBH_ROUTE_ADDRESS_IH_BYOI
};

/* same values for DMA & SDMA */
static const uint8_t bbh_dma_and_sdma_read_requests_fifo_base_address[DRV_BBH_NUMBER_OF_PORTS] =
{
    BBH_DMA_FIFO_ADDRESS_BASE_EMAC0,
    BBH_DMA_FIFO_ADDRESS_BASE_EMAC1,
    BBH_DMA_FIFO_ADDRESS_BASE_EMAC2,
    BBH_DMA_FIFO_ADDRESS_BASE_DOCSIS,
    BBH_DMA_FIFO_ADDRESS_BASE_BYOI,
};

static const uint16_t bbh_rx_runner_thread[DRV_BBH_NUMBER_OF_PORTS] =
{
    LAN0_RX_DISPATCH_THREAD_NUMBER,
    LAN1_RX_DISPATCH_THREAD_NUMBER,
    LAN2_RX_DISPATCH_THREAD_NUMBER,
    0,
    0
};

static const uint16_t bbh_wan_pp_task[DRV_BBH_NUMBER_OF_PORTS][4] =
{
    {0},
    {0},
    {0},
    {DOCSIS_PROCESSING_0_THREAD_NUMBER,
     DOCSIS_PROCESSING_1_THREAD_NUMBER,
     DOCSIS_PROCESSING_2_THREAD_NUMBER,
     DOCSIS_PROCESSING_3_THREAD_NUMBER},
    {BYOI_PROCESSING_0_THREAD_NUMBER,
     BYOI_PROCESSING_1_THREAD_NUMBER,
     BYOI_PROCESSING_2_THREAD_NUMBER,
     BYOI_PROCESSING_3_THREAD_NUMBER},
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
        /* WAN bridged high */
        DRV_RDD_IH_CLASS_WAN_DOCSIS_INDEX,
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
        DRV_RDD_IH_CLASS_WAN_BYOI_INDEX,
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
            DRV_RDD_IH_CLASS_10_CLASS_SEARCH_4,
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
            DRV_RDD_IH_CLASS_11_CLASS_SEARCH_4,
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
            DRV_RDD_IH_CLASS_12_CLASS_SEARCH_4,
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

/* IH FORWARDING matrix configuration
SP \ DEST | Eth 0  | Eth 1 |  Eth 2 | Eth 3 |  BYOI | DOCSIS | PCIE 0 | MC | CPU | DDR | SRAM | PCIE 1 | SPARE   |
----------|--------|-------|--------|-------|-------|--------|--------|--------|----|-----|-----|------|--------|---------|
Eth 0     | DIS    | EN    | EN     | DIS   | EN    | EN     |
----------|--------|-------|--------|-------|-------|------|--------|--------|----|-----|-----|------|--------|---------|
Eth 1
Eth 2
Eth 3
BYOI
DOCSIS
PCIE 0
PCIE 1                                                      0
*/
static const int ih_forwarding_matrix[DRV_IH_TARGET_MATRIX_NUMBER_OF_SOURCE_PORTS][DRV_IH_TARGET_MATRIX_NUMBER_OF_DESTINATION_PORTS] =
{
    {0,1,1,0,1,1,1,1,0,0,0,0,0},    /* ETH0 */
    {1,0,1,0,1,1,1,1,0,0,0,0,0},    /* ETH1 */
    {1,1,0,0,1,1,1,1,0,0,0,0,0},    /* ETH2 */
    {0,0,0,0,0,0,0,0,0,0,0,0,0},    /* ETH3 */
    {1,1,1,0,1,1,1,1,0,0,0,0,0},    /* BYOI */
    {1,1,1,0,1,1,1,1,0,0,0,0,0},    /* DOCSIS */
    {1,1,1,0,1,1,1,1,0,0,0,0,0}     /* PCIE0 */
};

static const int ih_target_mem_matrix_byoi_enabled[DRV_IH_TARGET_MATRIX_NUMBER_OF_SOURCE_PORTS][DRV_IH_TARGET_MATRIX_NUMBER_OF_DESTINATION_PORTS] =
{
    {1,1,1,0,0,0,0,0,0,0,0,0,0},    /* ETH0 */
    {1,1,1,0,0,0,0,0,0,0,0,0,0},    /* ETH1 */
    {1,1,1,0,0,0,0,0,0,0,0,0,0},    /* ETH2 */
    {0,0,0,0,0,0,0,0,0,0,0,0,0},    /* ETH3 */
    {0,0,0,0,0,0,0,0,0,0,0,0,0},    /* BYOI */
    {0,0,0,0,0,0,0,0,0,0,0,0,0},    /* DOCSIS */
    {0,0,0,0,0,0,0,0,0,0,0,0,0}     /* PCIE0 */
};

static const int ih_target_mem_matrix_byoi_disabled[DRV_IH_TARGET_MATRIX_NUMBER_OF_SOURCE_PORTS][DRV_IH_TARGET_MATRIX_NUMBER_OF_DESTINATION_PORTS] =
{
    {1,1,1,0,0,1,0,0,0,0,0,0,0},    /* ETH0 */
    {1,1,1,0,0,1,0,0,0,0,0,0,0},    /* ETH1 */
    {1,1,1,0,0,1,0,0,0,0,0,0,0},    /* ETH2 */
    {0,0,0,0,0,0,0,0,0,0,0,0,0},    /* ETH3 */
    {0,0,0,0,0,0,0,0,0,0,0,0,0},    /* BYOI */
    {0,0,0,0,0,0,0,0,0,0,0,0,0},    /* DOCSIS */
    {0,0,0,0,0,0,0,0,0,0,0,0,0}     /* PCIE0 */
};

static const int ih_direct_mode_matrix[DRV_IH_TARGET_MATRIX_NUMBER_OF_SOURCE_PORTS][DRV_IH_TARGET_MATRIX_NUMBER_OF_DESTINATION_PORTS] =
{
    {0,0,0,0,0,0,0,0,0,0,0,0,0},    /* ETH0 */
    {0,0,0,0,0,0,0,0,0,0,0,0,0},    /* ETH1 */
    {0,0,0,0,0,0,0,0,0,0,0,0,0},    /* ETH2 */
    {0,0,0,0,0,0,0,0,0,0,0,0,0},    /* ETH3 */
    {0,0,0,0,0,0,0,0,0,0,0,0,0},    /* BYOI */
    {0,0,0,0,0,1,0,0,0,0,0,0,0},    /* DOCSIS */
    {1,1,1,0,0,0,0,0,0,0,0,0,0}     /* PCIE0 */
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

#if 0
static DRV_IH_TARGET_MEMORY calculate_ih_target_memory(DRV_IH_TARGET_MATRIX_SOURCE_PORT src, DRV_IH_TARGET_MATRIX_DESTINATION_PORT dest)
{

    if(p_dpi_cfg->rdp_byoi_enabled)
    {
       return (DRV_IH_TARGET_MEMORY)ih_target_mem_matrix_byoi_enabled[src][dest];
    }
    else
    {
        return (DRV_IH_TARGET_MEMORY)ih_target_mem_matrix_byoi_disabled[src][dest];
    }

}


static void ih_configure_target_matrix(void)
{
    int i, j;
    DRV_IH_TARGET_MATRIX_PER_SP_CONFIG per_sp_config;

    for (i = DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH0; i < DRV_IH_TARGET_MATRIX_NUMBER_OF_SOURCE_PORTS; i++)
    {
        /* the destination ports after 'multicast' are not in use currently */
        for (j = DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH0; j < DRV_IH_TARGET_MATRIX_NUMBER_OF_DESTINATION_PORTS; j++)
        {
            per_sp_config.entry[j].target_memory = calculate_ih_target_memory(i, j);
            per_sp_config.entry[j].direct_mode = ih_direct_mode_matrix[i][j];

            fi_bl_drv_ih_set_forward(i, j, ih_forwarding_matrix[i][j]);
        }
        fi_bl_drv_ih_set_target_matrix(i, &per_sp_config);
    }
}
#endif

static void update_bbh_ih_ingress_buffers_bitmask(DRV_BBH_PORT_INDEX bbh_port_index, uint8_t base_location, uint8_t queue_size)
{
    uint16_t bitmask;
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

        fi_bl_drv_ih_configure_ingress_queue(i, &ingress_queue_config);

        /* update the correspoding bitmask in database, to be configured in BBH */
        update_bbh_ih_ingress_buffers_bitmask((DRV_BBH_PORT_INDEX)i, ingress_queue_config.base_location, ingress_queue_config.size);

        base_location += ingress_queue_config.size;
    }

    /* queues of BYOI */
    ingress_queue_config.base_location = base_location;
    ingress_queue_config.size = IH_INGRESS_QUEUE_SIZE_WAN;
    ingress_queue_config.priority = IH_INGRESS_QUEUE_PRIORITY_WAN;
    ingress_queue_config.weight = IH_INGRESS_QUEUE_WEIGHT_WAN;
    ingress_queue_config.congestion_threshold = IH_INGRESS_QUEUE_CONGESTION_THRESHOLD_WAN;

    fi_bl_drv_ih_configure_ingress_queue(IH_INGRESS_QUEUE_4_BYOI, &ingress_queue_config);

    update_bbh_ih_ingress_buffers_bitmask(DRV_BBH_BYOI, ingress_queue_config.base_location,
        ingress_queue_config.size);

    base_location += ingress_queue_config.size;

    /* queues of DOCSIS */
    ingress_queue_config.base_location = base_location;
    ingress_queue_config.size = IH_INGRESS_QUEUE_SIZE_WAN;
    ingress_queue_config.priority = IH_INGRESS_QUEUE_PRIORITY_WAN;
    ingress_queue_config.weight = IH_INGRESS_QUEUE_WEIGHT_WAN;
    ingress_queue_config.congestion_threshold = IH_INGRESS_QUEUE_CONGESTION_THRESHOLD_WAN;

    fi_bl_drv_ih_configure_ingress_queue(IH_INGRESS_QUEUE_5_DOCSIS, &ingress_queue_config);

    update_bbh_ih_ingress_buffers_bitmask(DRV_BBH_DOCSIS, ingress_queue_config.base_location,
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

    /* check that we didn't overrun */
    if (base_location > DRV_IH_INGRESS_QUEUES_ARRAY_SIZE)
    {
        /* sum of sizes exceeded the total array size */
        return -1;
    }

    return 0;
}

#if 0 /* IH Lookup tables are not in use for now */
/* this function configures the IH classes which are in use */
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

    /** Table 2, table 3 & table 6: IPTV src ip - configured in RDPA iptv object */

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

    /*** table 8: MAC_SA_BROADCOM_SWITCH_LAN ***/
    lookup_table_60_bit_key_config.table_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_LOOKUP_TABLE_8_BASE_ADDRESS);
    lookup_table_60_bit_key_config.table_size = DRV_RDD_IH_LOOKUP_TABLE_8_SIZE;
    lookup_table_60_bit_key_config.maximal_search_depth = DRV_RDD_IH_LOOKUP_TABLE_8_SEARCH_DEPTH;
    lookup_table_60_bit_key_config.hash_type = DRV_RDD_IH_LOOKUP_TABLE_8_HASH_TYPE;
    lookup_table_60_bit_key_config.sa_search_enable = DRV_RDD_IH_LOOKUP_TABLE_8_SA_ENABLE;
    lookup_table_60_bit_key_config.aging_enable = DRV_RDD_IH_LOOKUP_TABLE_8_AGING_ENABLE;
    lookup_table_60_bit_key_config.cam_enable = DRV_RDD_IH_LOOKUP_TABLE_8_CAM_ENABLE;
    lookup_table_60_bit_key_config.cam_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_LOOKUP_TABLE_8_CAM_BASE_ADDRESS);
    lookup_table_60_bit_key_config.context_table_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_CONTEXT_TABLE_8_BASE_ADDRESS);
    lookup_table_60_bit_key_config.context_table_entry_size = DRV_RDD_IH_CONTEXT_TABLE_8_ENTRY_SIZE;
    lookup_table_60_bit_key_config.cam_context_base_address_in_8_byte = MS_BYTE_TO_8_BYTE_RESOLUTION (DRV_RDD_IH_CONTEXT_TABLE_8_CAM_BASE_ADDRESS);
    lookup_table_60_bit_key_config.part_0_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_8_SRC_MAC_BROADCOM_SWITCH_LAN_KEY_PART_0_START_OFFSET;
    lookup_table_60_bit_key_config.part_0_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_8_SRC_MAC_BROADCOM_SWITCH_LAN_KEY_PART_0_SHIFT;
    lookup_table_60_bit_key_config.part_1_start_offset_in_4_byte = DRV_RDD_IH_LOOKUP_TABLE_8_SRC_MAC_BROADCOM_SWITCH_LAN_KEY_PART_1_START_OFFSET;
    lookup_table_60_bit_key_config.part_1_shift_offset_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_8_SRC_MAC_BROADCOM_SWITCH_LAN_KEY_PART_1_SHIFT;
    lookup_table_60_bit_key_config.key_extension = DRV_RDD_IH_LOOKUP_TABLE_8_SRC_MAC_BROADCOM_SWITCH_LAN_KEY_KEY_EXTENSION;
    lookup_table_60_bit_key_config.part_0_mask_low = DRV_RDD_IH_LOOKUP_TABLE_8_SRC_MAC_BROADCOM_SWITCH_LAN_KEY_PART_0_MASK_LOW;
    lookup_table_60_bit_key_config.part_0_mask_high = DRV_RDD_IH_LOOKUP_TABLE_8_SRC_MAC_BROADCOM_SWITCH_LAN_KEY_PART_0_MASK_HIGH;
    lookup_table_60_bit_key_config.part_1_mask_low = DRV_RDD_IH_LOOKUP_TABLE_8_SRC_MAC_BROADCOM_SWITCH_LAN_KEY_PART_1_MASK_LOW;
    lookup_table_60_bit_key_config.part_1_mask_high = DRV_RDD_IH_LOOKUP_TABLE_8_SRC_MAC_BROADCOM_SWITCH_LAN_KEY_PART_1_MASK_HIGH;
    lookup_table_60_bit_key_config.global_mask_in_4_bit = DRV_RDD_IH_LOOKUP_TABLE_8_SRC_MAC_BROADCOM_SWITCH_LAN_KEY_GLOBAL_MASK;

    fi_bl_drv_ih_configure_lut_60_bit_key(DRV_RDD_IH_LOOKUP_TABLE_MAC_SA_BROADCOM_SWITCH_LAN_INDEX, &lookup_table_60_bit_key_config);

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

    fi_bl_drv_ih_configure_lut_60_bit_key(DRV_RDD_IH_LOOKUP_TABLE_MAC_SA_INDEX, &lookup_table_60_bit_key_config);
}
#endif

/* this function configures the IH classes which are in use */
static void ih_configure_classes(void)
{
    DRV_IH_CLASS_CONFIG us_class_config;
    uint8_t i;

    for (i = 0; i < ARRAY_LENGTH(gs_ih_classes); i++)
    {
        fi_bl_drv_ih_configure_class(gs_ih_classes[i].class_index, &gs_ih_classes[i].class_config);
    }
    /* in case BYOI is disabled we have to send upstream packets to SRAM instead of DDR */
    if (!p_dpi_cfg->rdp_byoi_enabled)
    {
        fi_bl_drv_ih_get_class_configuration(DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH0_INDEX, &us_class_config);
        us_class_config.target_memory_default = DRV_IH_TARGET_MEMORY_SRAM;
        fi_bl_drv_ih_configure_class(DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH0_INDEX, &us_class_config);
        fi_bl_drv_ih_get_class_configuration(DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH1_INDEX, &us_class_config);
        us_class_config.target_memory_default = DRV_IH_TARGET_MEMORY_SRAM;
        fi_bl_drv_ih_configure_class(DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH1_INDEX, &us_class_config);
        fi_bl_drv_ih_get_class_configuration(DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH2_INDEX, &us_class_config);
        us_class_config.target_memory_default = DRV_IH_TARGET_MEMORY_SRAM;
        fi_bl_drv_ih_configure_class(DRV_RDD_IH_CLASS_LAN_BRIDGED_ETH2_INDEX, &us_class_config);
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

void ih_cfg_mcast_prefix_filter_enable(void)
{
    DRV_IH_CLASSIFIER_CONFIG ih_classifier_config = {};
    uint8_t ipv4_mac_da_prefix[] = {0x01, 0x00, 0x5E, 0x00, 0x00, 0x00};
    uint8_t ipv4_mac_da_prefix_mask[] = {0xFF, 0xFF, 0xFF, 0x80, 0x00, 0x00};
    uint8_t ipv6_mac_da_prefix[] = {0x33, 0x33, 0x00, 0x00, 0x00, 0x00};
    uint8_t ipv6_mac_da_prefix_mask[] = {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};

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

/* this function configures the IH IPTV classifier
 * For GPON mode: create a classifier to perform class override from IPTV to BRIDGE_LOW if and only if the bcast filter
 * for iptv is enabled. */
void ih_cfg_iptv_lookup_classifier(void)
{
    DRV_IH_CLASSIFIER_CONFIG ih_classifier_config = {};

    ih_classifier_config.mask            = MASK_IH_CLASS_KEY_L4;
    ih_classifier_config.resulting_class = DRV_RDD_IH_CLASS_WAN_BRIDGED_LOW_INDEX;
    ih_classifier_config.l4_protocol     = DRV_IH_L4_PROTOCOL_IGMP;
    /* Configure a classifier for broadcast traffic arriving on IPTV flow */
    fi_bl_drv_ih_configure_classifier(IH_CLASSIFIER_IGMP_IPTV, &ih_classifier_config);

    /* Configure the ih ICMPV6 classifier */
    memset(&ih_classifier_config, 0, sizeof(DRV_IH_CLASSIFIER_CONFIG));

    ih_classifier_config.mask            = MASK_IH_CLASS_KEY_L4;
    ih_classifier_config.resulting_class = DRV_RDD_IH_CLASS_WAN_BRIDGED_LOW_INDEX;
    ih_classifier_config.l4_protocol     = DRV_IH_L4_PROTOCOL_ICMPV6;
    /* Configure a classifier for broadcast traffic arriving on IPTV flow */
    fi_bl_drv_ih_configure_classifier(IH_CLASSIFIER_ICMPV6, &ih_classifier_config);
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

    /* general configuration */
    ih_general_config.runner_a_ih_response_address             = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_0_IH_RESPONSE_ADDRESS);
    ih_general_config.runner_b_ih_response_address             = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_1_IH_RESPONSE_ADDRESS);
    ih_general_config.runner_a_ih_congestion_report_address    = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_0_IH_CONGESTION_REPORT_ADDRESS);
    ih_general_config.runner_b_ih_congestion_report_address    = MS_BYTE_TO_8_BYTE_RESOLUTION(DRV_RDD_IH_RUNNER_1_IH_CONGESTION_REPORT_ADDRESS);
    ih_general_config.runner_a_ih_congestion_report_enable     = DRV_RDD_IH_RUNNER_0_IH_CONGESTION_REPORT_ENABLE;
    ih_general_config.runner_b_ih_congestion_report_enable     = DRV_RDD_IH_RUNNER_1_IH_CONGESTION_REPORT_ENABLE;
    ih_general_config.lut_searches_enable_in_direct_mode       = 1;
    ih_general_config.sn_stamping_enable_in_direct_mode        = 1;
    ih_general_config.header_length_minimum                    = IH_HEADER_LENGTH_MIN;
    ih_general_config.congestion_discard_disable               = 0;
    ih_general_config.cam_search_enable_upon_invalid_lut_entry = DRV_RDD_IH_CAM_SEARCH_ENABLE_UPON_INVALID_LUT_ENTRY;

    fi_bl_drv_ih_set_general_configuration(&ih_general_config);

    /* packet header offsets configuration */
    packet_header_offsets.eth0_packet_header_offset = IH_PACKET_HEADER_OFFSET_ETH0;
    packet_header_offsets.eth1_packet_header_offset = IH_PACKET_HEADER_OFFSET_ETH1;
    packet_header_offsets.eth2_packet_header_offset = IH_PACKET_HEADER_OFFSET_ETH2;
    packet_header_offsets.eth3_packet_header_offset = 0 /* source port not used */;
    packet_header_offsets.eth4_packet_header_offset = IH_PACKET_HEADER_OFFSET_BYOI;
    packet_header_offsets.gpon_packet_header_offset = IH_PACKET_HEADER_OFFSET_DOCSIS;
    packet_header_offsets.runner_a_packet_header_offset = IH_PACKET_HEADER_OFFSET_RUNNER_A;
    packet_header_offsets.runner_b_packet_header_offset = IH_PACKET_HEADER_OFFSET_RUNNER_B;

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
    xi_route_addresses.eth3_route_address     = IH_ETH3_ROUTE_ADDRESS;
    xi_route_addresses.eth4_route_address     = IH_BYOI_ROUTE_ADDRESS;
    xi_route_addresses.gpon_route_address     = IH_DOCSIS_ROUTE_ADDRESS;
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
    source_port_to_ingress_queue_mapping.eth3_ingress_queue     = IH_INGRESS_QUEUE_3_ETH3;
    source_port_to_ingress_queue_mapping.eth4_ingress_queue     = IH_INGRESS_QUEUE_4_BYOI;
    source_port_to_ingress_queue_mapping.gpon_ingress_queue     = IH_INGRESS_QUEUE_5_DOCSIS;
    source_port_to_ingress_queue_mapping.runner_a_ingress_queue = IH_INGRESS_QUEUE_6_RUNNER_A;
    source_port_to_ingress_queue_mapping.runner_b_ingress_queue = IH_INGRESS_QUEUE_7_RUNNER_B;

    fi_bl_drv_ih_set_source_port_to_ingress_queue_mapping(&source_port_to_ingress_queue_mapping);

    /* ingress queues configuration */
    ih_configure_ingress_queues();

    /* configure wan ports */
    wan_ports_config.eth0 = 0;
    wan_ports_config.eth1 = 0;
    wan_ports_config.eth2 = 0;
    wan_ports_config.eth3 = 0;
    wan_ports_config.eth4 = 1;
    wan_ports_config.gpon = 1;
    wan_ports_config.runner_a = 0;
    wan_ports_config.runner_b = 0;
    wan_ports_config.pcie0 = 0;
    wan_ports_config.pcie1 = 0;

    fi_bl_drv_ih_configure_wan_ports(&wan_ports_config);

#if 0 /* lookup  is disabled */
    ih_configure_lookup_tables();
#endif

    ih_configure_classes();

    ih_configure_parser();

#if 0 /* target matrix not in use for now */
    ih_configure_target_matrix();
#endif

    ih_cfg_mcast_prefix_filter_enable();

    ih_cfg_iptv_lookup_classifier();
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

    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_RNR_A, DRV_SBPM_USER_GROUP_0);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_GPON,  DRV_SBPM_USER_GROUP_0);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_EMAC4, DRV_SBPM_USER_GROUP_0);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_EMAC0, DRV_SBPM_USER_GROUP_1);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_EMAC1, DRV_SBPM_USER_GROUP_1);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_EMAC2, DRV_SBPM_USER_GROUP_1);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_PCI0,  DRV_SBPM_USER_GROUP_1);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_RNR_B, DRV_SBPM_USER_GROUP_1);
    fi_bl_drv_sbpm_set_user_group_mapping(DRV_SBPM_SP_SPARE_0, DRV_SBPM_USER_GROUP_6);
}

static void bpm_drv_init(void)
{
    DRV_BPM_RUNNER_MSG_CTRL_PARAMS runner_msg_ctrl_params;

    init_bpm_virt_base();

    fi_bl_drv_bpm_init(NULL, NULL, 0, DRV_SPARE_BN_MESSAGE_FORMAT_14_bit_BN_WIDTH);

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
    for (bbh = DRV_BBH_EMAC_0; bbh < DRV_BBH_BYOI; bbh++)
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

    /* now allcate the remaining for BYOI if exists*/
    bbh_rx_dma_data_and_chunk_descriptor_fifos_size[DRV_BBH_BYOI] = BBH_RX_DMA_FIFOS_SIZE_BYOI;
    bbh_rx_dma_exclusive_threshold[DRV_BBH_BYOI] = BBH_RX_DMA_EXCLUSIVE_THRESHOLD_BYOI;
    bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[DRV_BBH_BYOI] = BBH_RX_SDMA_FIFOS_SIZE_BYOI;
    bbh_rx_sdma_exclusive_threshold[DRV_BBH_BYOI] = BBH_RX_SDMA_EXCLUSIVE_THRESHOLD_BYOI;

    bbh_rx_dma_data_fifo_base_address[DRV_BBH_BYOI] = dma_base_address;
    bbh_rx_dma_chunk_descriptor_fifo_base_address[DRV_BBH_BYOI] = dma_base_address;
    bbh_rx_sdma_data_fifo_base_address[DRV_BBH_BYOI] = sdma_base_address;
    bbh_rx_sdma_chunk_descriptor_fifo_base_address[DRV_BBH_BYOI] = sdma_base_address;

    dma_base_address += bbh_rx_dma_data_and_chunk_descriptor_fifos_size[DRV_BBH_BYOI];
    sdma_base_address += bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[DRV_BBH_BYOI];

    /* now allcate the remaining for DOCSIS if exists*/
    bbh_rx_dma_data_and_chunk_descriptor_fifos_size[DRV_BBH_BYOI] = BBH_RX_DMA_FIFOS_SIZE_DOCSIS;
    bbh_rx_dma_exclusive_threshold[DRV_BBH_BYOI] = BBH_RX_DMA_EXCLUSIVE_THRESHOLD_DOCSIS;
    bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[DRV_BBH_BYOI] = BBH_RX_SDMA_FIFOS_SIZE_DOCSIS;
    bbh_rx_sdma_exclusive_threshold[DRV_BBH_BYOI] = BBH_RX_SDMA_EXCLUSIVE_THRESHOLD_DOCSIS;

    bbh_rx_dma_data_fifo_base_address[DRV_BBH_BYOI] = dma_base_address;
    bbh_rx_dma_chunk_descriptor_fifo_base_address[DRV_BBH_BYOI] = dma_base_address;
    bbh_rx_sdma_data_fifo_base_address[DRV_BBH_BYOI] = sdma_base_address;
    bbh_rx_sdma_chunk_descriptor_fifo_base_address[DRV_BBH_BYOI] = sdma_base_address;

    dma_base_address += bbh_rx_dma_data_and_chunk_descriptor_fifos_size[DRV_BBH_BYOI];
    sdma_base_address += bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[DRV_BBH_BYOI];
}

static void init_dma_sdma(void)
{
    E_DMA_PERIPHERAL peripheral_id ;
    DMA_REGS_CONFIG_U_THRESH dma_thresh;
    DMA_REGS_CONFIG_DDR_SEL0 dma_mem_sel;
    uint32_t dma_mem_sel_reg;

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
    if (1 /* is BYOI enabled */)
    {
        //handle the RGMII port
        fi_dma_configure_memory_allocation(BB_MODULE_DMA, DMA_PERIPHERAL_BYOI,
            bbh_rx_dma_data_fifo_base_address[DMA_PERIPHERAL_BYOI],
            bbh_rx_dma_chunk_descriptor_fifo_base_address[DMA_PERIPHERAL_BYOI],
            bbh_rx_dma_data_and_chunk_descriptor_fifos_size[DMA_PERIPHERAL_BYOI]);
            /* SDMA */
        fi_dma_configure_memory_allocation(BB_MODULE_SDMA, DMA_PERIPHERAL_BYOI,
            bbh_rx_sdma_data_fifo_base_address[DMA_PERIPHERAL_BYOI],
            bbh_rx_sdma_chunk_descriptor_fifo_base_address[DMA_PERIPHERAL_BYOI],
            bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[DMA_PERIPHERAL_BYOI]) ;
    }
    /* DOCSIS */
        /* DMA */
        fi_dma_configure_memory_allocation(BB_MODULE_DMA, DMA_PERIPHERAL_DOCSIS,
            bbh_rx_dma_data_fifo_base_address[DRV_BBH_DOCSIS],
            bbh_rx_dma_chunk_descriptor_fifo_base_address[DRV_BBH_DOCSIS],
            bbh_rx_dma_data_and_chunk_descriptor_fifos_size[DRV_BBH_DOCSIS]);
        /* SDMA */
        fi_dma_configure_memory_allocation(BB_MODULE_SDMA, DMA_PERIPHERAL_DOCSIS,
            bbh_rx_sdma_data_fifo_base_address[DRV_BBH_DOCSIS],
            bbh_rx_sdma_chunk_descriptor_fifo_base_address[DRV_BBH_DOCSIS],
            bbh_rx_sdma_data_and_chunk_descriptor_fifos_size[DRV_BBH_DOCSIS]) ;

    /* assign DOCSIS to the second buffer of DMA */
    dma_mem_sel_reg = (1 << DMA_PERIPHERAL_DOCSIS);
    DMA_REGS_CONFIG_MEM_SEL_WRITE(BB_MODULE_DMA,dma_mem_sel_reg);

    dma_thresh.out_of_u = 3;
    dma_thresh.into_u = 5;
    DMA_REGS_CONFIG_U_THRESH_WRITE(BB_MODULE_DMA,DMA_PERIPHERAL_EMAC_0,dma_thresh);
    DMA_REGS_CONFIG_U_THRESH_WRITE(BB_MODULE_DMA,DMA_PERIPHERAL_EMAC_1,dma_thresh);
    DMA_REGS_CONFIG_U_THRESH_WRITE(BB_MODULE_DMA,DMA_PERIPHERAL_EMAC_2,dma_thresh);
    DMA_REGS_CONFIG_U_THRESH_WRITE(BB_MODULE_DMA,DMA_PERIPHERAL_BYOI,dma_thresh);
    DMA_REGS_CONFIG_U_THRESH_WRITE(BB_MODULE_DMA,DMA_PERIPHERAL_DOCSIS,dma_thresh);

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
        desc_addr = MS_BYTE_TO_8_BYTE_RESOLUTION(LAN0_RX_DESCRIPTORS_ADDRESS);
        break;
    case DRV_BBH_EMAC_1:
        desc_addr = MS_BYTE_TO_8_BYTE_RESOLUTION(LAN1_RX_DESCRIPTORS_ADDRESS);
        break;
    case DRV_BBH_EMAC_2:
        desc_addr = MS_BYTE_TO_8_BYTE_RESOLUTION(LAN2_RX_DESCRIPTORS_ADDRESS);
        break;
    case DRV_BBH_BYOI:
        desc_addr = MS_BYTE_TO_8_BYTE_RESOLUTION(BYOI_RX_NORMAL_DESCRIPTORS_ADDRESS);
        break;
    case DRV_BBH_DOCSIS:
        desc_addr = MS_BYTE_TO_8_BYTE_RESOLUTION(WAN_RX_NORMAL_DESCRIPTORS_ADDRESS);
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

    bbh_tx_configuration.runner_route_address = bbh_route_address_runner_1[bbh];

    /* irelevant for 3390 */
    bbh_tx_configuration.ddr_buffer_size = DRV_BBH_DDR_BUFFER_SIZE_RESERVED;
    bbh_tx_configuration.payload_offset_resolution = DRV_BBH_PAYLOAD_OFFSET_RESOLUTION_1_B;

    /*TODO: add multicast configuration here */
    bbh_tx_configuration.ddr1_multicast_headers_base_address_in_byte = p_dpi_cfg->rdp_ddr0_mc_base;
    bbh_tx_configuration.ddr2_multicast_headers_base_address_in_byte = p_dpi_cfg->rdp_ddr1_mc_base;
    bbh_tx_configuration.ddr1_tm_base_address = p_dpi_cfg->rdp_ddr0_pkt_base;
    bbh_tx_configuration.ddr2_tm_base_address = p_dpi_cfg->rdp_ddr1_pkt_base;
    bbh_tx_configuration.fpm_buff_size_set =  p_dpi_cfg->fpm_buff_size_set;

    if (BBH_PORT_IS_WAN(bbh))
    {
        bbh_tx_configuration.multicast_header_size = BBH_MULTICAST_HEADER_SIZE_FOR_LAN_PORT;
        /* this is the task also in Gbe case */
        bbh_tx_configuration.task_0 = bbh == DRV_BBH_DOCSIS ? DOCSIS_WAN_TX_THREAD_NUMBER : BYOI_WAN_TX_THREAD_NUMBER;

        /* not relevant
        bbh_tx_configuration.skb_address = MS_BYTE_TO_8_BYTE_RESOLUTION(GPON_ABSOLUTE_TX_BBH_COUNTER_ADDRESS);
        */
        if (bbh == DRV_BBH_BYOI)
        {
            bbh_tx_configuration.byoi_no_fpm_release = 1;
        }

        if (bbh == DRV_BBH_DOCSIS && p_dpi_cfg->cmim_pass_through_enabled)
        {
            bbh_tx_configuration.byoi_no_fpm_release = 1;
        }
    }
    else
    /*we will reach here in case the BBH port in lan ports 0 - 4*/
    {
        bbh_tx_configuration.multicast_header_size = BBH_MULTICAST_HEADER_SIZE_FOR_LAN_PORT;

        /* not relevant
        bbh_tx_configuration.skb_address = MS_BYTE_TO_8_BYTE_RESOLUTION(EMAC_ABSOLUTE_TX_BBH_COUNTER_ADDRESS)
                        + bbh_to_rdd_emac_map[port_index];
        */
        bbh_tx_configuration.task_0 = LAN_TX_THREAD_NUMBER;
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

        rdd_mdu_mode_pointer_get((rdd_emac_id_t)(bbh + 1), &mdu_mode_read_pointer_address_in_byte);

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
    if (bbh == DRV_BBH_BYOI)
        bbh_tx_configuration.tcont_address_in_8_byte =  MS_BYTE_TO_8_BYTE_RESOLUTION(BYOI_BBH_TX_WAN_CHANNEL_INDEX_ADDRESS);
    else if (bbh == DRV_BBH_DOCSIS)
        bbh_tx_configuration.tcont_address_in_8_byte =  MS_BYTE_TO_8_BYTE_RESOLUTION(DOCSIS_BBH_TX_WAN_CHANNEL_INDEX_ADDRESS);
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
    bbh_rx_configuration.ddr1_tm_base_address = p_dpi_cfg->rdp_ddr0_pkt_base;
    bbh_rx_configuration.ddr2_tm_base_address = p_dpi_cfg->rdp_ddr1_pkt_base;

    bbh_rx_configuration.pd_fifo_base_address_normal_queue_in_8_byte = f_get_bbh_rx_pd_fifo_base_address_normal_queue_in_8_byte(bbh);
    bbh_rx_configuration.pd_fifo_base_address_direct_queue_in_8_byte = 0;
    bbh_rx_configuration.pd_fifo_size_normal_queue = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    bbh_rx_configuration.pd_fifo_size_direct_queue = DRV_RDD_IH_RUNNER_0_MAXIMAL_NUMBER_OF_BUFFERS;
    bbh_rx_configuration.runner_0_task_normal_queue = bbh_wan_pp_task[bbh][0];
    bbh_rx_configuration.runner_0_task_direct_queue = 0;
    bbh_rx_configuration.runner_1_task_normal_queue = bbh_rx_runner_thread[bbh];
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

        /* parallel processing */
        bbh_rx_configuration.pp_task_enable_bitmap = 0xf;
        bbh_rx_configuration.pp_task_nums[0] = bbh_wan_pp_task[bbh][1];
        bbh_rx_configuration.pp_task_nums[1] = bbh_wan_pp_task[bbh][2];
        bbh_rx_configuration.pp_task_nums[2] = bbh_wan_pp_task[bbh][3];
        bbh_rx_configuration.packet_header_offset = IH_PACKET_HEADER_OFFSET_DOCSIS;
    }
    else
    {
        bbh_rx_configuration.minimum_packet_size_0 = MIN_ETH_PKT_SIZE_LAN;
        bbh_rx_configuration.packet_header_offset = IH_PACKET_HEADER_OFFSET_ETH0;
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
    case DRV_BBH_DOCSIS:
        per_flow_configuration.default_ih_class = DRV_RDD_IH_CLASS_WAN_DOCSIS_INDEX;
        per_flow_configuration.ih_class_override = 1;
        break;
    case DRV_BBH_BYOI:
        per_flow_configuration.default_ih_class = DRV_RDD_IH_CLASS_WAN_BYOI_INDEX;
        per_flow_configuration.ih_class_override = 1;
        break;

    default:
        return;
        break;
    }

    /* in EMAC, only flow 0 is relevant */
    fi_bl_drv_bbh_rx_set_per_flow_configuration(bbh, 0, &per_flow_configuration);

    /* Docsis CMIM pass through is BBH flow 1 */
    if (bbh == DRV_BBH_DOCSIS && p_dpi_cfg->cmim_pass_through_enabled)
        fi_bl_drv_bbh_rx_set_per_flow_configuration(bbh, 1, &per_flow_configuration);
}

static void init_bbh_ports(void)
{
    DRV_BBH_PORT_INDEX bbhidx;

    for (bbhidx = DRV_BBH_EMAC_0; bbhidx <= DRV_BBH_EMAC_2; bbhidx++)
    {
        init_single_bbh(bbhidx);
    }

    if (p_dpi_cfg->rdp_byoi_enabled)
    {
        init_single_bbh(DRV_BBH_BYOI);
    }

    init_single_bbh(DRV_BBH_DOCSIS);
}

static void configure_runner(void)
{
#ifdef __KERNEL__
    uint32_t natc_key_table_size;
    uint32_t natc_key_context_size;
#endif
    /* Local Variables */
    rdd_init_params_t rdd_init_params = {0};

    /* zero Runner memories (data, program and context) */
    rdd_init();

    rdd_load_microcode((uint8_t *)firmware_binary_A, (uint8_t *)firmware_binary_B,
        (uint8_t*)firmware_binary_C, (uint8_t *)firmware_binary_D);

    rdd_load_prediction((uint8_t *)firmware_predict_A, (uint8_t *)firmware_predict_B,
        (uint8_t *)firmware_predict_C, (uint8_t *)firmware_predict_D);

    rdd_init_params.ddr0_runner_base_ptr = (uint8_t *)p_dpi_cfg->rdp_ddr0_pkt_base;
    rdd_init_params.extra_ddr0_pool_ptr = (uint8_t *)p_dpi_cfg->rdp_ddr0_mc_base;
    rdd_init_params.ddr1_runner_base_ptr = (uint8_t *)p_dpi_cfg->rdp_ddr1_pkt_base;
    rdd_init_params.extra_ddr1_pool_ptr =  (uint8_t *)p_dpi_cfg->rdp_ddr1_mc_base;
    rdd_init_params.iptv_table_size = RDD_MAC_TABLE_SIZE_256;
#ifdef __KERNEL__
    natc_key_table_size = sizeof(RDD_NAT_CACHE_LKP_ENTRY_DTS) * (RDD_NAT_CACHE_TABLE_SIZE + RDD_NAT_CACHE_EXTENSION_TABLE_SIZE);
    natc_key_context_size = sizeof(RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS) * RDD_CONTEXT_TABLE_SIZE;
    rdd_init_params.runner_nat_cache_key_ptr = (uint32_t)ioremap(p_dpi_cfg->rdp_runner_tables_ddr_ptr, natc_key_table_size);
    rdd_init_params.runner_nat_cache_context_ptr = (uint32_t)ioremap(p_dpi_cfg->rdp_runner_tables_ddr_ptr + natc_key_table_size,
        natc_key_context_size);
    rdd_init_params.runner_iptv_tables_base_ptr = p_dpi_cfg->rdp_runner_tables_ddr_ptr + 0x600000;
    printk("passing rdd_init_params.runner_iptv_tables_base_ptr=0x%x\n",rdd_init_params.runner_iptv_tables_base_ptr);
    memset(rdd_init_params.runner_nat_cache_key_ptr, 0, natc_key_table_size);
    memset(rdd_init_params.runner_nat_cache_context_ptr, 0, natc_key_context_size);
    printk("configure_runner: NATC keysize 0x%x key element size %d contextsize 0x%x context element size %d\n",
        natc_key_table_size, sizeof(RDD_NAT_CACHE_LKP_ENTRY_DTS),
        natc_key_context_size, sizeof(RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS));
#endif
    rdd_init_params.ddr_packet_headroom_size = p_dpi_cfg->headroom_size;
    rdd_init_params.psram_packet_headroom_size = PSRAM_HEADROOM_SIZE;
    rdd_init_params.bridge_flow_cache_mode = rdpa_method_fc;

    rdd_data_structures_init(&rdd_init_params);

    rdd_cmim_pass_through_cfg(p_dpi_cfg->cmim_pass_through_enabled);

#ifdef __KERNEL__
#ifdef CM3390
    printk("rdd_init_params.runner_nat_cache_key_ptr=0x%08x\n", rdd_init_params.runner_nat_cache_key_ptr);
    printk("g_runner_ddr0_iptv_lookup_ptr=0x%x\n", g_runner_ddr0_iptv_lookup_ptr);
    printk("g_runner_ddr0_iptv_context_ptr=0x%x\n", g_runner_ddr0_iptv_context_ptr);
    printk("g_runner_ddr0_iptv_ssm_context_ptr=0x%x\n", g_runner_ddr0_iptv_ssm_context_ptr);
#endif
#endif
}

static void natcach_drv_init(void)
{
    uint32_t cntl_status_reg0 = 0;
    uint32_t cntl_status_reg1 = 0;
    uint32_t key_base = p_dpi_cfg->rdp_runner_tables_ddr_ptr;
    uint32_t result_base = key_base + sizeof(RDD_NAT_CACHE_LKP_ENTRY_DTS) * (RDD_NAT_CACHE_TABLE_SIZE + RDD_NAT_CACHE_EXTENSION_TABLE_SIZE);

    cntl_status_reg1 |= NATCACHE_RDP_CONTROL_STATUS2_AGE_TIMER_TICK;
    cntl_status_reg1 |= (DEFAULT_AGE_TIMER_32 << NATCACHE_RDP_CONTROL_STATUS2_AGE_TIMER_OFFSET);
    cntl_status_reg1 |= (DEFAULT_DDR_BINS_PER_BACKET << NATCACHE_RDP_CONTROL_STATUS2_DDR_BINS_PER_BUCKET_OFFSET);
    cntl_status_reg1 |= (NATCACHE_RDP_CONTROL_STATUS2_DDR_SIZE_VAL_64K << NATCACHE_RDP_CONTROL_STATUS2_DDR_SIZE_OFFSET);
    cntl_status_reg1 |= NATCACHE_RDP_CONTROL_STATUS2_CACHE_UPDATE_ON_REG_DDR_LOOKUP;
    cntl_status_reg1 |= (NATCACHE_RDP_CONTROL_STATUS2_DDR_HASH_MODE_VAL_CRC32_16B_REDUCED << NATCACHE_RDP_CONTROL_STATUS2_DDR_HASH_MODE_OFFSET);

    cntl_status_reg0 = 0;
    cntl_status_reg0 |= NATCACHE_RDP_CONTROL_STATUS_DDR_ENABLE;
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
    WRITE_32(UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_EN + RDP_UBUS_MASTER_BRDG_REG_HP_OFFSET, reg);

    /*second Ubus Master*/
    reg = 0x000f0d01;
    WRITE_32(UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_EN + RDP_UBUS_MASTER_BRDG_REG_HP_OFFSET, reg);

    /*third Ubus Master*/
    reg = 0x000e0e01;
    WRITE_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_EN + RDP_UBUS_MASTER_BRDG_REG_HP_OFFSET, reg);

    reg = 0x00090901;
    WRITE_32(UBUS_MASTER_4_RDP_UBUS_MASTER_BRDG_REG_EN + RDP_UBUS_MASTER_BRDG_REG_HP_OFFSET, reg);

    /* enable all Ubus masters */
    reg = 0x1; // bit 0 is the enable bit
    WRITE_32(UBUS_MASTER_1_RDP_UBUS_MASTER_BRDG_REG_EN, reg);
    WRITE_32(UBUS_MASTER_2_RDP_UBUS_MASTER_BRDG_REG_EN, reg);
    WRITE_32(UBUS_MASTER_3_RDP_UBUS_MASTER_BRDG_REG_EN, reg);
    WRITE_32(UBUS_MASTER_4_RDP_UBUS_MASTER_BRDG_REG_EN, reg);
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
    sbpm_sp_enable.eth3_sp_enable = 0;

    if (p_dpi_cfg->rdp_byoi_enabled)
        sbpm_sp_enable.eth4_sp_enable = 1;

    fi_bl_drv_sbpm_sp_enable(&sbpm_sp_enable);
}

#ifndef FIRMWARE_INIT
#ifndef __KERNEL__
static uint32_t ioremap(uint32_t addr, uint32_t size)
{
    return 0;
}
#endif

static void rnr_wkup_cpuc_init(void)
{
    uint32_t runner_wakeup_reg;
    uint32_t runner_wakeup_addr;
    uint32_t rnr_wkup_reg_base = (uint32_t)ioremap(RNR_WKUP_CPUC_BASE,RNR_WKUP_CPUC_SIZE);
    rnr_wakeup_msg_cfg  l_rnr_wkup = {0};
    rnr_wakeup_msg_cfg* p_rnr_wkup;
    p_rnr_wkup = (rnr_wakeup_msg_cfg *)rnr_wkup_reg_base;

    /* write the Runner wakeup registers to RNR_WKUP_CPUC */
    /* core A */
    runner_wakeup_reg = 0xd5099004;
    runner_wakeup_addr = rnr_wkup_reg_base + RNR_WKUP_CPUC_RNR_ADDR_0_OFFSET;

    *(volatile unsigned int *)runner_wakeup_addr = runner_wakeup_reg;
    /* core B */
    runner_wakeup_reg = 0xd509a004;
    runner_wakeup_addr = rnr_wkup_reg_base + RNR_WKUP_CPUC_RNR_ADDR_1_OFFSET;
    *(volatile unsigned int *)runner_wakeup_addr = runner_wakeup_reg;

    /* move to the first runner dqm configuration no. 32 */
    p_rnr_wkup += 32;

    l_rnr_wkup.wake_on_not_empty = 1;

    /* Downstream forward from RG  - 32*/
    l_rnr_wkup.runner_sel = RUNNER_CORE_A;

    l_rnr_wkup.runner_task = CPU_TX_RGW_DS_FORWARD_THREAD_NUMBER;
    *p_rnr_wkup = l_rnr_wkup;

    /* Upstream forward from RG  - 33*/
    p_rnr_wkup++;
    l_rnr_wkup.runner_sel = RUNNER_CORE_B;
    l_rnr_wkup.runner_task = CPU_TX_RGW_US_FORWARD_THREAD_NUMBER;
    *p_rnr_wkup = l_rnr_wkup;

    /* Downstream egress from RG 34 */
    p_rnr_wkup++;
    l_rnr_wkup.runner_sel = RUNNER_CORE_B;
    l_rnr_wkup.runner_task = CPU_TX_RGW_DS_EGRESS_THREAD_NUMBER;
    *p_rnr_wkup = l_rnr_wkup;

    /* Upstream egress from RG 35 */
    p_rnr_wkup++;
    l_rnr_wkup.runner_sel = RUNNER_CORE_B;
    l_rnr_wkup.runner_task = CPU_TX_RGW_US_EGRESS_THREAD_NUMBER;
    *p_rnr_wkup = l_rnr_wkup;

    /* Downstream forward from STB - 36 */
    p_rnr_wkup++;
    l_rnr_wkup.runner_sel = RUNNER_CORE_A;
    l_rnr_wkup.runner_task = CPU_TX_STB_DS_FORWARD_THREAD_NUMBER;
    *p_rnr_wkup = l_rnr_wkup;

    /* Upstream forward from STB - 37*/
    p_rnr_wkup++;
    l_rnr_wkup.runner_sel = RUNNER_CORE_B;
    l_rnr_wkup.runner_task = CPU_TX_STB_US_FORWARD_THREAD_NUMBER;
    *p_rnr_wkup = l_rnr_wkup;

    /* Downstream egress from STB 38 */
    p_rnr_wkup++;
    l_rnr_wkup.runner_sel = RUNNER_CORE_B;
    l_rnr_wkup.runner_task = CPU_TX_STB_DS_EGRESS_THREAD_NUMBER;
    *p_rnr_wkup = l_rnr_wkup;

    /* Upstream egress from STB 39 */
    p_rnr_wkup++;
    l_rnr_wkup.runner_sel = RUNNER_CORE_B;
    l_rnr_wkup.runner_task = CPU_TX_STB_US_EGRESS_THREAD_NUMBER;
    *p_rnr_wkup = l_rnr_wkup;

    /* Downstream forward from DFAP  40*/
    p_rnr_wkup++;
    /* fw not supported yet */

    /* Upstream forward from DFAP  41*/
    p_rnr_wkup++;
    /* fw not supported yet */

    /* Downstream egress from DFAP * 42 */
    p_rnr_wkup++;
    l_rnr_wkup.runner_sel = RUNNER_CORE_B;
    l_rnr_wkup.runner_task = CPU_TX_DFAP_DS_EGRESS_THREAD_NUMBER;
    *p_rnr_wkup = l_rnr_wkup;

    /* Upstream egress from DFAP * 43 */
    p_rnr_wkup++;
    l_rnr_wkup.runner_sel = RUNNER_CORE_B;
    l_rnr_wkup.runner_task = CPU_TX_DFAP_US_EGRESS_THREAD_NUMBER;
    *p_rnr_wkup = l_rnr_wkup;

    /* Downstream forward from VIPER  44*/
    p_rnr_wkup++;
    /* fw not supported yet */

    /* Upstream forward from VIPER  45*/
    p_rnr_wkup++;
    /* fw not supported yet */

    /* Downstream egress from VIPER * 46 */
    p_rnr_wkup++;
    l_rnr_wkup.runner_sel = RUNNER_CORE_B;
    l_rnr_wkup.runner_task = CPU_TX_VIPER_DS_EGRESS_THREAD_NUMBER;
    *p_rnr_wkup = l_rnr_wkup;

    /* Upstream egress from DFAP * 47 */
    p_rnr_wkup++;
    l_rnr_wkup.runner_sel = RUNNER_CORE_B;
    l_rnr_wkup.runner_task = CPU_TX_VIPER_US_EGRESS_THREAD_NUMBER;
    *p_rnr_wkup = l_rnr_wkup;
}

int init_rdp_virtual_mem(void)
{
#if __KERNEL__
    soc_base_address = (uint8_t*)ioremap(RDP_3390_PHYS_BASE, RDP_3390_RDP_PHYS_SIZE);
    printk(_BBh_ "RDP Physical address=0x%x Virtual address = %p"_BBnl_, RDP_3390_PHYS_BASE, soc_base_address);
#else
    soc_base_address = 0;
#endif
    return 0;
}

static void init_unimacs(void)
{
    mac_hwapi_init_emac(rdpa_emac0);
    mac_hwapi_init_emac(rdpa_emac1);
    mac_hwapi_init_emac(rdpa_emac2);
    mac_hwapi_set_unimac_cfg(rdpa_emac0);
    mac_hwapi_set_unimac_cfg(rdpa_emac1);
    mac_hwapi_set_unimac_cfg(rdpa_emac2);
    mac_hwapi_set_tx_max_frame_len(rdpa_emac0, p_dpi_cfg->mtu_size);
    mac_hwapi_set_tx_max_frame_len(rdpa_emac1, p_dpi_cfg->mtu_size);
    mac_hwapi_set_tx_max_frame_len(rdpa_emac2, p_dpi_cfg->mtu_size);
    mac_hwapi_set_rxtx_enable(rdpa_emac0, 1, 1);
    mac_hwapi_set_rxtx_enable(rdpa_emac1, 1, 1);
    mac_hwapi_set_rxtx_enable(rdpa_emac2, 1, 1);
}
#endif

int data_path_init(data_path_init_params *dpi_params)
{
    p_dpi_cfg = dpi_params;
#ifndef FIRMWARE_INIT
    init_rdp_virtual_mem();
#endif

    init_bbh_dma_sdma_related_arrays();

    /* init runner,load microcode and structures */
    configure_runner();

    rdd_runner_frequency_set(p_dpi_cfg->runner_freq);

    ih_init();

    sbpm_drv_init();

#ifdef __KERNEL__
    /* the FPM hold the Broad Bus in reset until all the Runner drivers are initilaized */
    fpm_reset_bb(0);
#endif

    bpm_drv_init();

    natcach_drv_init();

    /* init DMA and SDMA */
    init_dma_sdma();

    init_bbh_ports();

    init_ubus_masters();

#ifndef FIRMWARE_INIT
    rnr_wkup_cpuc_init();

    init_unimacs();
#endif

    rdd_runner_enable();

    enable_source_ports();

    return 0;
}

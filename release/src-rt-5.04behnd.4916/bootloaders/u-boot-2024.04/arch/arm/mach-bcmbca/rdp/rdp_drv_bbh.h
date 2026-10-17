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
/* This header file defines all datatypes and functions exported for the      */
/* Lilac BBH driver.                                                          */
/*                                                                            */
/******************************************************************************/


#ifndef DRV_BBH_H_INCLUDED
#define DRV_BBH_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif


/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/

#include "rdp_bbh.h"


/******************************************************************************/
/*                                                                            */
/* Types and values definitions                                               */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* Error codes returned by BBH driver APIs                                    */
/******************************************************************************/
typedef enum
{
    DRV_BBH_NO_ERROR ,
    DRV_BBH_INVALID_PORT_INDEX ,
    DRV_BBH_INVALID_MULTICAST_HEADER_SIZE ,
    DRV_BBH_INVALID_TX_PD_FIFO_SIZE ,
    DRV_BBH_INVALID_PD_PREFETCH_BYTE_THRESHOLD ,
    DRV_BBH_INVALID_DMA_READ_REQUESTS_MAXIMAL_NUMBER ,
    DRV_BBH_INVALID_SDMA_READ_REQUESTS_MAXIMAL_NUMBER ,
    DRV_BBH_INVALID_RX_PD_FIFO_SIZE ,
    DRV_BBH_INVALID_DMA_DATA_AND_CHUNK_DESCRIPTOR_FIFOS_SIZE ,
    DRV_BBH_INVALID_DMA_EXCLUSIVE_THRESHOLD ,
    DRV_BBH_INVALID_SDMA_DATA_AND_CHUNK_DESCRIPTOR_FIFOS_SIZE ,
    DRV_BBH_INVALID_SDMA_EXCLUSIVE_THRESHOLD ,
    DRV_BBH_INVALID_MINIMUM_PACKET_SIZE ,
    DRV_BBH_INVALID_MAXIMUM_PACKET_SIZE ,
    DRV_BBH_INVALID_PACKET_HEADER_OFFSET ,
    DRV_BBH_INVALID_FLOWS_32_255_GROUP_DIVIDER ,
    DRV_BBH_INVALID_IH_CLASS ,
    DRV_BBH_INVALID_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION ,
    DRV_BBH_ILLEGAL_FLOW_INDEX_FOR_ETHERNET_PORT ,
    DRV_BBH_INVALID_MINIMUM_PACKET_SIZE_SELECTION ,
    DRV_BBH_INVALID_MAXIMUM_PACKET_SIZE_SELECTION ,
    DRV_BBH_INVALID_REASSEMBLY_OFFSET ,
    DRV_BBH_API_IS_FOR_GPON_PORT_ONLY,
    DRV_BBH_TCONT_ID_IS_OUT_OF_RANGE
}
DRV_BBH_ERROR ;

/* Minimal Multicast header size */
#define DRV_BBH_MINIMAL_MULTICAST_HEADER_SIZE                                  ( 4 )
/* Maximal Multicast header size */                                           
#define DRV_BBH_MAXIMAL_MULTICAST_HEADER_SIZE                                  ( 64 )
                                                                              
/* Minimal PD FIFO size in TX */                                              
#define DRV_BBH_TX_MINIMAL_PD_FIFO_SIZE                                        ( 1 )
/* Maximal PD FIFO size in TX */                                              
#define DRV_BBH_TX_MAXIMAL_PD_FIFO_SIZE                                        ( 8 )

/* Maximal PD prefetch byte threshold, in 32 byte resolution */
#define DRV_BBH_TX_MAXIMAL_PD_PREFETCH_BYTE_THRESHOLD_IN_32_BYTE               ( 4095 )

/* Minimal value for DMA read requests maximal number */
#define DRV_BBH_TX_MINIMAL_VALUE_FOR_DMA_READ_REQUESTS_MAXIMAL_NUMBER          ( 1 )
/* Maximal value for DMA read requests maximal number */
#define DRV_BBH_TX_MAXIMAL_VALUE_FOR_DMA_READ_REQUESTS_MAXIMAL_NUMBER          ( 8 )

/* Minimal value for SDMA read requests maximal number */
#define DRV_BBH_TX_MINIMAL_VALUE_FOR_SDMA_READ_REQUESTS_MAXIMAL_NUMBER         ( 1 )
/* Maximal value for SDMA read requests maximal number */
#define DRV_BBH_TX_MAXIMAL_VALUE_FOR_SDMA_READ_REQUESTS_MAXIMAL_NUMBER         ( 4 )


/* Minimal PD FIFO size in RX */
#define DRV_BBH_RX_MINIMAL_PD_FIFO_SIZE                                        ( 2 )
/* Maximal PD FIFO size in RX */
#define DRV_BBH_RX_MAXIMAL_PD_FIFO_SIZE                                        ( 64 )

/* Minimal DMA/SDMA data FIFO size */
#define DRV_BBH_RX_MINIMAL_DMA_OR_SDMA_DATA_FIFO_SIZE                          ( 2 )
/* Maximal DMA/SDMA data FIFO size */
#define DRV_BBH_RX_MAXIMAL_DMA_OR_SDMA_DATA_FIFO_SIZE                          ( 64 )

/* Maximal value for minimum packet size in RX */
#define DRV_BBH_RX_MAXIMAL_VALUE_FOR_MINIMUM_PACKET_SIZE                       ( 96 )

/* Maximal value for maximum packet size in RX */
#define DRV_BBH_RX_MAXIMAL_VALUE_FOR_MAXIMUM_PACKET_SIZE                       ( 16383 )

/* Maximal packet header offset */
#define DRV_BBH_RX_MAXIMAL_PACKET_HEADER_OFFSET                                ( 63 )

/* Number of Flows */
#define DRV_BBH_RX_NUMBER_OF_FLOWS                                             ( 256 )

/* Minimal Flows 32-255 group divider */
#define DRV_BBH_RX_MINIMAL_FLOWS_32_255_GROUP_DIVIDER                          ( 32 )

/* Maximal IH class */
#define DRV_BBH_RX_MAXIMAL_IH_CLASS                                            ( 15 )

/* Maximal selection for minimum or maximum packet size */
#define DRV_BBH_RX_MAXIMAL_SELECTION_FOR_MINIMUM_OR_MAXIMUM_PACKET_SIZE        ( 3 )

/* Maximal reassembly offset (in 8 byte resolution) */
#define DRV_BBH_RX_MAXIMAL_REASSEMBLY_OFFSET_IN_8_BYTE                         ( 50 )


/******************************************************************************/
/* BBH Port index: there are 7 instances of the BBH block. BBH 0-5 refer to   */
/* EMAC 0-5 and BBH 6 refers to GPON (EMAC 5 is the one MUXed with GPON).     */
/******************************************************************************/
typedef enum
{
    DRV_BBH_EMAC_0 ,
    DRV_BBH_EMAC_1 ,
    DRV_BBH_EMAC_2 ,
    DRV_BBH_EMAC_3 ,
    DRV_BBH_EMAC_4 ,
    DRV_BBH_EMAC_5 ,
    DRV_BBH_GPON ,
    DRV_BBH_DSL = DRV_BBH_GPON ,
    DRV_BBH_EPON,
    DRV_BBH_NUMBER_OF_PORTS
}
DRV_BBH_PORT_INDEX ;


/******************************************************************************/
typedef enum
{
    DRV_BBH_DDR_BPM_MESSAGE_FORMAT_14_BIT_BN_WIDTH ,
    DRV_BBH_DDR_BPM_MESSAGE_FORMAT_15_BIT_BN_WIDTH
}
DRV_BBH_DDR_BPM_MESSAGE_FORMAT ;
/* DDR buffer size                                                            */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_BBH_DDR_BUFFER_SIZE_2_KB ,
    DRV_BBH_DDR_BUFFER_SIZE_4_KB ,
    DRV_BBH_DDR_BUFFER_SIZE_16_KB ,
    DRV_BBH_DDR_BUFFER_SIZE_2_5_KB
}
DRV_BBH_DDR_BUFFER_SIZE ;


/******************************************************************************/
/* Payload offset resolution                                                  */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_BBH_PAYLOAD_OFFSET_RESOLUTION_1_B ,
    DRV_BBH_PAYLOAD_OFFSET_RESOLUTION_2_B
}
DRV_BBH_PAYLOAD_OFFSET_RESOLUTION ;


/******************************************************************************/
/* BBH TX internal units                                                      */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_BBH_TX_INTERNAL_UNIT_SEGMENTATION_CONTEXT_TABLE = 1 << 0 ,
    DRV_BBH_TX_INTERNAL_UNIT_ALL_40_PDS_FIFOS = 1 << 1 ,
    DRV_BBH_TX_INTERNAL_UNIT_WRITE_POINTER_IN_THE_DMA = 1 << 2 ,
    DRV_BBH_TX_INTERNAL_UNIT_WRITE_POINTER_IN_THE_SDMA = 1 << 3 ,
    DRV_BBH_TX_INTERNAL_UNIT_BPM_RELEASE_FIFO = 1 << 4 ,
    DRV_BBH_TX_INTERNAL_UNIT_SBPM_RELEASE_FIFO = 1 << 5 ,
    DRV_BBH_TX_INTERNAL_UNIT_ORDER_KEEPER_FIFO = 1 << 6 ,
    DRV_BBH_TX_INTERNAL_UNIT_DDR_DATA_FIFO = 1 << 7 ,
    DRV_BBH_TX_INTERNAL_UNIT_SRAM_DATA_FIFO = 1 << 8 ,
    DRV_BBH_TX_INTERNAL_UNIT_SKB_POINTERS = 1 << 9
}
DRV_BBH_TX_INTERNAL_UNIT ;


/******************************************************************************/
/* Flow control triggers                                                      */
/******************************************************************************/
typedef enum
{
    DRV_BBH_RX_FLOW_CONTROL_TRIGGER_BPM_IS_IN_EXCLUSIVE_STATE = 1 << 0 ,
    DRV_BBH_RX_FLOW_CONTROL_TRIGGER_SBPM_IS_IN_EXCLUSIVE_STATE = 1 << 1 ,
    DRV_BBH_RX_FLOW_CONTROL_TRIGGER_RUNNER_REQUEST = 1 << 2 ,
}
DRV_BBH_RX_FLOW_CONTROL_TRIGGER ;


/******************************************************************************/
/* Drop triggers                                                              */
/******************************************************************************/
typedef enum
{
    DRV_BBH_RX_DROP_TRIGGER_BPM_IS_IN_EXCLUSIVE_STATE = 1 << 0 ,
    DRV_BBH_RX_DROP_TRIGGER_SBPM_IS_IN_EXCLUSIVE_STATE = 1 << 1 ,
}
DRV_BBH_RX_DROP_TRIGGER ;


/******************************************************************************/
/* Flow index for per flow configuration:                                     */
/* Each one of flows 0-31 has its own configuration. Flows 32-255 are divided */
/* into 2 groups (32 to x, x+1 to 255). Each group has its own configuration. */
/* The groups-divider (x) is configured in "RX Set configuration" API.        */
/* The allowed values for this type are: 0-31,                                */
/* DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_0,      */
/* DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_1       */
/******************************************************************************/
typedef uint8_t DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION ;
#define DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_0    ( 32 )
#define DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION_GROUP_1    ( 33 )


/******************************************************************************/
/* BBH RX internal units                                                      */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_BBH_RX_INTERNAL_UNIT_INPUT_BUFFER = 1 << 0 ,
    DRV_BBH_RX_INTERNAL_UNIT_BURST_BUFFER = 1 << 1 ,
    DRV_BBH_RX_INTERNAL_UNIT_IH_CONTEXT = 1 << 2 ,
    DRV_BBH_RX_INTERNAL_UNIT_IH_BUFFER_ENABLE = 1 << 3 ,
    DRV_BBH_RX_INTERNAL_UNIT_REASSEMBLY_FIFO = 1 << 4 ,
    DRV_BBH_RX_INTERNAL_UNIT_BPM_FIFO = 1 << 5 ,
    DRV_BBH_RX_INTERNAL_UNIT_SBPM_FIFO = 1 << 6 ,
    DRV_BBH_RX_INTERNAL_UNIT_IH_RESPONSE_FIFO = 1 << 7 ,
    DRV_BBH_RX_INTERNAL_UNIT_PRE_WAKEUP_FIFO = 1 << 8 ,
    DRV_BBH_RX_INTERNAL_UNIT_REASSEMBLY_CONTEXT_TABLE = 1 << 9 ,
    DRV_BBH_RX_INTERNAL_UNIT_DMA_WRITE_POINTER = 1 << 10 ,
    DRV_BBH_RX_INTERNAL_UNIT_SDMA_WRITE_POINTER = 1 << 11 ,
    DRV_BBH_RX_INTERNAL_UNIT_RUNNER_WRITE_POINTER = 1 << 12
}
DRV_BBH_RX_INTERNAL_UNIT ;


/******************************************************************************/
/* TX configuration                                                           */
/******************************************************************************/
typedef struct
{
    /* DMA route address. */
    uint8_t dma_route_address ;

    /* BPM route address. */
    uint8_t bpm_route_address ;

    /* SDMA route address. */
    uint8_t sdma_route_address ;

    /* SBPM route address. */
    uint8_t sbpm_route_address ;

    /* Runner route address. */
    uint8_t runner_route_address ;

    /* Runner route address. */
	uint8_t runner_sts_route ;

    /* The data is arranged in the DDR in a fixed size buffers. */
    DRV_BBH_DDR_BUFFER_SIZE ddr_buffer_size ;

    DRV_BBH_DDR_BPM_MESSAGE_FORMAT ddr_bpm_message_format ;
    /* The payload offset itself is indicated in the PD sent from runner to
       BBH. */
    DRV_BBH_PAYLOAD_OFFSET_RESOLUTION payload_offset_resolution ;

    /* Multicast header size. */
    uint8_t multicast_header_size ;

    /* Multicast headers base address, in byte resolution. This base address
       should be aligned to 512 bytes, therefore the 9 LSB bits are always
       zero (and will be ignored). The address is relative to DDR TM base. */
    uint32_t multicast_headers_base_address_in_byte ;

    /* Runner task to be woken up, per TCONT. In Ethernet case, only Task 0
       is relevant. */
    uint8_t task_0 ;

    /* Runner task to be woken up, per TCONT. In Ethernet case, only Task 0
       is relevant. */
    uint8_t task_1 ;

    /* Runner task to be woken up, per TCONT. In Ethernet case, only Task 0
       is relevant. */
    uint8_t task_2 ;

    /* Runner task to be woken up, per TCONT. In Ethernet case, only Task 0
       is relevant. */
    uint8_t task_3 ;

    /* Runner task to be woken up, per TCONT. In Ethernet case, only Task 0
       is relevant. */
    uint8_t task_4 ;

    /* Runner task to be woken up, per TCONT. In Ethernet case, only Task 0
       is relevant. */
    uint8_t task_5 ;

    /* Runner task to be woken up, per TCONT. In Ethernet case, only Task 0
       is relevant. */
    uint8_t task_6 ;

    /* Runner task to be woken up, per TCONT. In Ethernet case, only Task 0
       is relevant. */
    uint8_t task_7 ;

    /* Runner task to be woken up, for TCONTs 8-39. In Ethernet case, only
       Task 0 is relevant. */
    uint8_t task_8_39 ;

    /* PD FIFO size, per TCONT. A total of 128 PDs is available for all
       queues. For Ethernet, queue 0 should be configured. */
    uint8_t pd_fifo_size_0 ;

    /* PD FIFO size, per TCONT. A total of 128 PDs is available for all
       queues. For Ethernet, queue 0 should be configured. */
    uint8_t pd_fifo_size_1 ;

    /* PD FIFO size, per TCONT. A total of 128 PDs is available for all
       queues. For Ethernet, queue 0 should be configured. */
    uint8_t pd_fifo_size_2 ;

    /* PD FIFO size, per TCONT. A total of 128 PDs is available for all
       queues. For Ethernet, queue 0 should be configured. */
    uint8_t pd_fifo_size_3 ;

    /* PD FIFO size, per TCONT. A total of 128 PDs is available for all
       queues. For Ethernet, queue 0 should be configured. */
    uint8_t pd_fifo_size_4 ;

    /* PD FIFO size, per TCONT. A total of 128 PDs is available for all
       queues. For Ethernet, queue 0 should be configured. */
    uint8_t pd_fifo_size_5 ;

    /* PD FIFO size, per TCONT. A total of 128 PDs is available for all
       queues. For Ethernet, queue 0 should be configured. */
    uint8_t pd_fifo_size_6 ;

    /* PD FIFO size, per TCONT. A total of 128 PDs is available for all
       queues. For Ethernet, queue 0 should be configured. */
    uint8_t pd_fifo_size_7 ;

    /* PD FIFO size, for TCONTs 8-15. A total of 128 PDs is available for all
       queues. For Ethernet, queue 0 should be configured. */
    uint8_t pd_fifo_size_8_15 ;

    /* PD FIFO size, for TCONTs 16-23. A total of 128 PDs is available for
       all queues. For Ethernet, queue 0 should be configured. */
    uint8_t pd_fifo_size_16_23 ;

    /* PD FIFO size, for TCONTs 24-31. A total of 128 PDs is available for
       all queues. For Ethernet, queue 0 should be configured. */
    uint8_t pd_fifo_size_24_31 ;

    /* PD FIFO size, for TCONTs 32-39. A total of 128 PDs is available for
       all queues. For Ethernet, queue 0 should be configured. */
    uint8_t pd_fifo_size_32_39 ;

    /* Base within the 128 PDs array. Should correspond to the FIFOs sizes
       configuration. */
    uint8_t pd_fifo_base_0 ;

    /* Base within the 128 PDs array. Should correspond to the FIFOs sizes
       configuration. */
    uint8_t pd_fifo_base_1 ;

    /* Base within the 128 PDs array. Should correspond to the FIFOs sizes
       configuration. */
    uint8_t pd_fifo_base_2 ;

    /* Base within the 128 PDs array. Should correspond to the FIFOs sizes
       configuration. */
    uint8_t pd_fifo_base_3 ;

    /* Base within the 128 PDs array. Should correspond to the FIFOs sizes
       configuration. */
    uint8_t pd_fifo_base_4 ;

    /* Base within the 128 PDs array. Should correspond to the FIFOs sizes
       configuration. */
    uint8_t pd_fifo_base_5 ;

    /* Base within the 128 PDs array. Should correspond to the FIFOs sizes
       configuration. */
    uint8_t pd_fifo_base_6 ;

    /* Base within the 128 PDs array. Should correspond to the FIFOs sizes
       configuration. */
    uint8_t pd_fifo_base_7 ;

    /* Base within the 128 PDs array. Should correspond to the FIFOs sizes
       configuration. */
    uint8_t pd_fifo_base_8_15 ;

    /* Base within the 128 PDs array. Should correspond to the FIFOs sizes
       configuration. */
    uint8_t pd_fifo_base_16_23 ;

    /* Base within the 128 PDs array. Should correspond to the FIFOs sizes
       configuration. */
    uint8_t pd_fifo_base_24_31 ;

    /* Base within the 128 PDs array. Should correspond to the FIFOs sizes
       configuration. */
    uint8_t pd_fifo_base_32_39 ;

    /* PD prefetch byte threshold enable (for preventing HOL blocking). */
    int32_t pd_prefetch_byte_threshold_enable ;

    /* PD prefetch byte threshold (in 32 byte resolution), per TCONT.
       Relevant if "PD prefetch byte threshold enable" is set. */
    uint16_t pd_prefetch_byte_threshold_0_in_32_byte ;

    /* PD prefetch byte threshold (in 32 byte resolution), per TCONT.
       Relevant if "PD prefetch byte threshold enable" is set. */
    uint16_t pd_prefetch_byte_threshold_1_in_32_byte ;

    /* PD prefetch byte threshold (in 32 byte resolution), per TCONT.
       Relevant if "PD prefetch byte threshold enable" is set. */
    uint16_t pd_prefetch_byte_threshold_2_in_32_byte ;

    /* PD prefetch byte threshold (in 32 byte resolution), per TCONT.
       Relevant if "PD prefetch byte threshold enable" is set. */
    uint16_t pd_prefetch_byte_threshold_3_in_32_byte ;

    /* PD prefetch byte threshold (in 32 byte resolution), per TCONT.
       Relevant if "PD prefetch byte threshold enable" is set. */
    uint16_t pd_prefetch_byte_threshold_4_in_32_byte ;

    /* PD prefetch byte threshold (in 32 byte resolution), per TCONT.
       Relevant if "PD prefetch byte threshold enable" is set. */
    uint16_t pd_prefetch_byte_threshold_5_in_32_byte ;

    /* PD prefetch byte threshold (in 32 byte resolution), per TCONT.
       Relevant if "PD prefetch byte threshold enable" is set. */
    uint16_t pd_prefetch_byte_threshold_6_in_32_byte ;

    /* PD prefetch byte threshold (in 32 byte resolution), per TCONT.
       Relevant if "PD prefetch byte threshold enable" is set. */
    uint16_t pd_prefetch_byte_threshold_7_in_32_byte ;

    /* PD prefetch byte threshold (in 32 byte resolution), for TCONTs 8-39.
       Relevant if "PD prefetch byte threshold enable" is set. */
    uint16_t pd_prefetch_byte_threshold_8_39_in_32_byte ;

    /* This value should be identical to the relevant configuration in the
       DMA. */
    uint8_t dma_read_requests_fifo_base_address ;

    /* DMA read requests maximal number. */
    uint8_t dma_read_requests_maximal_number ;

  	/* Epon_read_urgent */
    uint8_t epnurgnt ;

    /* The value should be identical to the relevant configuration in the
       SDMA. */
    uint8_t sdma_read_requests_fifo_base_address ;

    /* SDMA read requests maximal number */
    uint8_t sdma_read_requests_maximal_number ;

    /* Defines the TCONT address within the Runner address space. The address
       is in 8 bytes resolution. In GPON case, the BBH writes the relevant
       TCONT number into this address before sending a wake-up request to the
       Runner, for requesting a PD. */
    uint16_t tcont_address_in_8_byte ;

    /* When the packet is transmitted from absolute address, instead of
       releasing the BN, the BBH writes a 6 bits read counter into the Runner
       SRAM. It writes it into a pre-defined address + TCONT_NUM. This value
       defines the SKB free base address within the Runner address. */
    uint16_t skb_address ;

    /* In MDU mode, the runner pushes PDs to BBH without any wakeups from the
       BBH. Despite its name, this mode may be used in SFU case as well. */
    int32_t mdu_mode_enable ;

    /* In MDU mode, Each time that the BBH reads a PD from the PD FIFO, it'll
       write the read pointer into this address (in the Runner). The address
       is in 8-bytes resolution. */
    uint16_t mdu_mode_read_pointer_address_in_8_byte ;

    /* The address is in bytes resolution. Should be aligned to 128 bytes.
       Should match the relevant registers value in the BBH RX and in the
       Runner. */
    uint32_t ddr_tm_base_address ;

    /* When this bit is set, the BBH will wait till the EMAC FIFO is empty
       before issuing a DMA read command (of a 1588 packet only). This
       configuration is relevant for Ethernet only. */
    int32_t emac_1588_enable ;
}
DRV_BBH_TX_CONFIGURATION ;


/******************************************************************************/
/* TX Counters                                                                */
/******************************************************************************/
typedef struct
{
    /* This counter counts the number of packets which were transmitted from
       the SRAM. This counter is relevant for Ethernet only. */
    uint32_t tx_packets_from_sram ;

    /* This counter counts the number of packets which were transmitted from
       the DDR. It counts the packets for all TCONTs together. */
    uint32_t tx_packets_from_ddr ;

    /* This counter counts the number of PDs which were dropped due to PD
       FIFO full. This counter is relevant for Ethernet only. */
    uint16_t dropped_pd ;

    /* This counter counts the number of PDs with packet length equal zero.
       It counts the packets for all TCONTs together. */
    uint16_t pd_with_zero_packet_length ;

    /* This counter counts the number Get next responses with a null BN. This
       counter is relevant for Ethernet only. */
    uint16_t get_next_null ;
}
DRV_BBH_TX_COUNTERS ;


/******************************************************************************/
/* RX configuration                                                           */
/******************************************************************************/
typedef struct
{
    /* DMA route address. */
    uint8_t dma_route_address ;

    /* BPM route address. */
    uint8_t bpm_route_address ;

    /* SDMA route address. */
    uint8_t sdma_route_address ;

    /* SBPM route address. */
    uint8_t sbpm_route_address ;

    /* Runner 0 route address. */
    uint8_t runner_0_route_address ;

    /* Runner 1 route address. */
    uint8_t runner_1_route_address ;

    /* IH route address. */
    uint8_t ih_route_address ;

    /* The data is arranged in the DDR in a fixed size buffers. */
    DRV_BBH_DDR_BUFFER_SIZE ddr_buffer_size ;

    DRV_BBH_DDR_BPM_MESSAGE_FORMAT ddr_bpm_message_format ;
    /* The address is in bytes resolution. Should be aligned to 128 bytes.
       Should match the relevant registers value in the BBH TX and in the
       Runner. */
    uint32_t ddr_tm_base_address ;

    /* For every reassembled packet in the DDR the BBH writes a packet
       descriptor (PD) into the Runner. The PDs are arranged in a predefined
       address space in the Runner SRAM and managed in a cyclic FIFO style.
       The address is in 8-byte resolution. Same configuration for both
       Runner 0 and 1. */
    uint16_t pd_fifo_base_address_normal_queue_in_8_byte ;

    /* The address is in 8-byte resolution. Same configuration for both
       Runner 0 and 1. */
    uint16_t pd_fifo_base_address_direct_queue_in_8_byte ;

    /* Same configuration for both Runner 0 and 1. This value should be
       identical to the number of RIBs (runner ingress buffers) configuration
       in the IH block. */
    uint8_t pd_fifo_size_normal_queue ;

    /* Same configuration for both Runner 0 and 1. This value should be
       identical to the number of IH ingress buffers configuration in the IH
       block. */
    uint8_t pd_fifo_size_direct_queue ;

    /* For every PD written into the Runner, the BBH wakes the relevant
       task. */
    uint8_t runner_0_task_normal_queue ;

    /* For every PD written into the Runner, the BBH wakes the relevant
       task. */
    uint8_t runner_0_task_direct_queue ;

    /* For every PD written into the Runner, the BBH wakes the relevant
       task. */
    uint8_t runner_1_task_normal_queue ;

    /* For every PD written into the Runner, the BBH wakes the relevant
       task. */
    uint8_t runner_1_task_direct_queue ;

    /* The address is in chunk resolution (128 bytes). The value should be
       identical to the relevant configuration in the DMA. */
    uint8_t dma_data_fifo_base_address ;

    /* The address is in chunk descriptor resolution (8 bytes). The value
       should be identical to the relevant configuration in the DMA. */
    uint8_t dma_chunk_descriptor_fifo_base_address ;

    /* This value defines the size of both data FIFO and chunk descriptor
       FIFO. */
    uint8_t dma_data_and_chunk_descriptor_fifos_size ;

    /* This value defines the number of occupied DMA write chunks for
       dropping low or high priority packets. */
    uint8_t dma_exclusive_threshold ;

    /* The address is in chunk resolution (128 bytes). The value should be
       identical to the relevant configuration in the SDMA. */
    uint8_t sdma_data_fifo_base_address ;

    /* The address is in chunk descriptor resolution (8 bytes). The value
       should be identical to the relevant configuration in the SDMA. */
    uint8_t sdma_chunk_descriptor_fifo_base_address ;

    /* This value defines the size of both data FIFO and chunk descriptor
       FIFO. */
    uint8_t sdma_data_and_chunk_descriptor_fifos_size ;

    /* This value defines the number of occupied SDMA write chunks for
       dropping low or high priority packets. */
    uint8_t sdma_exclusive_threshold ;

    /* There are 4 global configurations for Minimum packet size. Each flow
       can get one out of these 4 global configurations. Packets shorter than
       this threshold will be discarded. */
    uint8_t minimum_packet_size_0 ;

    /* There are 4 global configurations for Minimum packet size. Each flow
       can get one out of these 4 global configurations. Packets shorter than
       this threshold will be discarded. */
    uint8_t minimum_packet_size_1 ;

    /* There are 4 global configurations for Minimum packet size. Each flow
       can get one out of these 4 global configurations. Packets shorter than
       this threshold will be discarded. */
    uint8_t minimum_packet_size_2 ;

    /* There are 4 global configurations for Minimum packet size. Each flow
       can get one out of these 4 global configurations. Packets shorter than
       this threshold will be discarded. */
    uint8_t minimum_packet_size_3 ;

    /* There are 4 global configurations for Maximum packet size. Each flow
       can get one out of these 4 global configurations. Packets longer than
       this threshold will be discarded. Should not exceed DDR buffer size. */
    uint16_t maximum_packet_size_0 ;

    /* There are 4 global configurations for Maximum packet size. Each flow
       can get one out of these 4 global configurations. Packets longer than
       this threshold will be discarded. Should not exceed DDR buffer size. */
    uint16_t maximum_packet_size_1 ;

    /* There are 4 global configurations for Maximum packet size. Each flow
       can get one out of these 4 global configurations. Packets longer than
       this threshold will be discarded. Should not exceed DDR buffer size. */
    uint16_t maximum_packet_size_2 ;

    /* There are 4 global configurations for Maximum packet size. Each flow
       can get one out of these 4 global configurations. Packets longer than
       this threshold will be discarded. Should not exceed DDR buffer size. */
    uint16_t maximum_packet_size_3 ;

    /* Each bit in the bitmask corresponds to 1 of the 16 IH ingress buffers.
       The bitmask specifies which ingress buffers are assigned to the
       relevant BBH. This configuration should correspond to the IH ingress
       queue configurations. */
    uint16_t ih_ingress_buffers_bitmask ;

    /* Packet header offset in the ingress buffer. This value should match
       the relevant configuration in the IH block and in the Runner. */
    uint8_t packet_header_offset ;

    /* Triggers for flow control to MAC. Values of the enumeration
       DRV_BBH_RX_FLOW_CONTROL_TRIGGER should be ORed, as a description
       of the desired triggers. */
    uint8_t flow_control_triggers_bitmask ;

    /* Triggers for drop. Values of the enumeration
       DRV_BBH_RX_DROP_TRIGGER should be ORed, as a description of the
       desired triggers. */
    uint8_t drop_triggers_bitmask ;

    /* DS Flows 0-31 have full configuration each. Flows 32-255 are divided
       into 2 groups: 32-x, (x+1)-255. Each group has its configuration. This
       value defines the divider (x). Relevant for GPON only. */
    uint8_t flows_32_255_group_divider ;

    /* Default IH class for PLOAM. Relevant for GPON only. */
    uint8_t ploam_default_ih_class ;

    /* IH class override for PLOAM. Relevant for GPON only. */
    int32_t ploam_ih_class_override ;

    /* The BBH writes the packets header into the IH. If the rest of the packet
       is written into the DDR, then the address is according to the reassembly
       offset configurations  */
    uint8_t reassembly_offset_in_8_byte ;
}
DRV_BBH_RX_CONFIGURATION ;


/******************************************************************************/
/* Per flow configuration                                                     */
/******************************************************************************/
typedef struct
{
    /* Selects one of the 4 global configurations configured in "RX Set per
       flow configuration" API. */
    uint8_t minimum_packet_size_selection ;

    /* Selects one of the 4 global configurations configured in "RX Set per
       flow configuration" API. */
    uint8_t maximum_packet_size_selection ;

    /* Default IH class. */
    uint8_t default_ih_class ;

    /* IH class override. */
    int32_t ih_class_override ;
}
DRV_BBH_PER_FLOW_CONFIGURATION ;


/******************************************************************************/
/* RX Counters                                                                */
/******************************************************************************/
typedef struct
{
    /* This counter counts the number of incoming good packets. It counts the
       packets from all flows together. */
    uint32_t incoming_packets ;

    /* This counter counts the packets drop due to Too short error. */
    uint32_t too_short_error ;

    /* This counter counts the packets drop due to Too long error. */
    uint32_t too_long_error ;

    /* This counter counts the packets drop due to CRC error. */
    uint32_t crc_error ;

    /* This counter counts the packets drop due to Runner Congestion
       indication (by IH). */
    uint32_t runner_congestion ;

    /* This counter counts the packets drop due to No BPM BN error. */
    uint32_t no_bpm_bn_error ;

    /* This counter counts the packets drop due to No SBPM SBN error. */
    uint32_t no_sbpm_sbn_error ;

    /* This counter counts the packets drop due to No DMA CD error. */
    uint32_t no_dma_cd_error ;

    /* This counter counts the packets drop due to No SDMA CD error. */
    uint32_t no_sdma_cd_error ;
}
DRV_BBH_RX_COUNTERS ;


/******************************************************************************/
/* RX Error Counters                                                          */
/******************************************************************************/
typedef struct
{
    /* This counter counts the packets drop due to SOP after SOP error. */
    uint32_t sop_after_sop_error ;

    /* This counter counts the packets drop due to Third flow error. */
    uint32_t third_flow_error ;

    /* This counter counts the PLOAMs drop due to Runner Congestion or IPTV
       Filter errors. */
    uint32_t ih_drop_error_for_ploam ;

    /* This counter counts the PLOAMs drop due to No BPM BN error. */
    uint32_t no_bpm_bn_error_for_ploam ;

    /* This counter counts the PLOAMs drop due to CRC error. */
    uint32_t crc_error_for_ploam ;
}
DRV_BBH_RX_ERROR_COUNTERS ;


/******************************************************************************/
/*                                                                            */
/* Functions prototypes                                                       */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_tx_set_configuration                                       */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BBH Driver - TX Set configuration                                        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets configuration of the TX part of BBH block.            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_port_index - Port index                                               */
/*                                                                            */
/*   xi_bbh_tx_configuration - BBH TX configuration.                          */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*     DRV_BBH_NO_ERROR - No error                                      */
/*     DRV_BBH_INVALID_PORT_INDEX - Invalid port index                  */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_tx_set_configuration ( DRV_BBH_PORT_INDEX xi_port_index ,
                                                          const DRV_BBH_TX_CONFIGURATION * xi_bbh_tx_configuration ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_rx_set_configuration                                       */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BBH Driver - RX Set configuration                                        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets configuration of the RX part of BBH block.            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_port_index - Port index                                               */
/*                                                                            */
/*   xi_rx_configuration - RX configuration                                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*     DRV_BBH_NO_ERROR - No error                                      */
/*     DRV_BBH_INVALID_PORT_INDEX - Invalid port index                  */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_rx_set_configuration ( DRV_BBH_PORT_INDEX xi_port_index ,
                                                          const DRV_BBH_RX_CONFIGURATION * xi_rx_configuration ) ;

/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_rx_set_triggers_of_flow_control_and_drop                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BBH Driver - RX Set triggers for flow control and drop                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets triggers for sending flow control to MAC, and         */
/*   triggers for dropping packets. For flow control, there are 3 possible    */
/*   triggers: BPM is in exclusive state, SBPM is in exclusive state, Runner  */
/*   request. For drop, there are 2 possible triggers: BPM is in exclusive    */
/*   state, SBPM is in exclusive state. The triggers are turned on/off        */
/*   according to the given bitmask. Values of the enumeration                */
/*   DRV_BBH_RX_FLOW_CONTROL_TRIGGER should be ORed, as a        */
/*   description of the desired triggers for flow control. Values of the      */
/*   enumeration DRV_BBH_RX_DROP_TRIGGER should be ORed, as a    */
/*   description of the desired triggers for drop.                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_port_index - Port index                                               */
/*                                                                            */
/*   xi_flow_control_triggers_bitmask - Flow control triggers bitmask         */
/*                                                                            */
/*   xi_drop_triggers_bitmask - Drop triggers bitmask                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*     DRV_BBH_NO_ERROR - No error                                      */
/*     DRV_BBH_INVALID_PORT_INDEX - Invalid port index                  */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_rx_set_triggers_of_flow_control_and_drop ( DRV_BBH_PORT_INDEX xi_port_index ,
                                                                              uint8_t xi_flow_control_triggers_bitmask ,
                                                                              uint8_t xi_drop_triggers_bitmask ) ;




/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_bbh_rx_set_per_flow_configuration                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   BBH Driver - RX Set per flow configuration                               */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets per-flow configuration in the RX part of BBH block.   */
/*   Each one of flows 0-31 has its own configuration. Flows 32-255 are       */
/*   divided into 2 groups (32 to x, x+1 to 255). Each group has its own      */
/*   configuration. The groups-divider (x) is configured in "RX Set           */
/*   configuration" API. In Ethernet case, only flow 0 is relevant.           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_port_index - Port index                                               */
/*                                                                            */
/*   xi_flow_index - Flow index                                               */
/*                                                                            */
/*   xi_per_flow_configuration - Per flow configuration                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_BBH_ERROR - Return code                                       */
/*     DRV_BBH_NO_ERROR - No error                                      */
/*     DRV_BBH_INVALID_PORT_INDEX - Invalid port index                  */
/*                                                                            */
/******************************************************************************/
DRV_BBH_ERROR fi_bl_drv_bbh_rx_set_per_flow_configuration ( DRV_BBH_PORT_INDEX xi_port_index ,
                                                                   DRV_BBH_RX_FLOW_INDEX_FOR_PER_FLOW_CONFIGURATION xi_flow_index ,
                                                                   const DRV_BBH_PER_FLOW_CONFIGURATION * xi_per_flow_configuration ) ;

#ifdef __cplusplus
}
#endif

#endif



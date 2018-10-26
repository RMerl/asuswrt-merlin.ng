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
/* This header file defines all datatypes and functions exported for the      */
/* Lilac IH driver                                                            */
/*                                                                            */
/******************************************************************************/


#ifndef DRV_IH_H_INCLUDED
#define DRV_IH_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif


/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/

#include "rdp_ih.h"


/******************************************************************************/
/*                                                                            */
/* Types and values definitions                                               */
/*                                                                            */
/******************************************************************************/

/******************************************************************************/
/* Error codes returned by IH driver APIs                                     */
/******************************************************************************/
typedef enum
{
    DRV_IH_NO_ERROR ,
    DRV_IH_ERROR_INVALID_INDEX ,
    DRV_IH_ERROR_INVALID_NUMBER_OF_RUNNER_BUFFERS ,
    DRV_IH_ERROR_INVALID_INGRESS_QUEUE ,
    DRV_IH_ERROR_INVALID_INGRESS_QUEUE_BASE_LOCATION ,
    DRV_IH_ERROR_INVALID_INGRESS_QUEUE_SIZE ,
    DRV_IH_ERROR_INVALID_INGRESS_QUEUE_PRIORITY ,
    DRV_IH_ERROR_INVALID_INGRESS_QUEUE_WEIGHT ,
    DRV_IH_ERROR_DESTINATION_PORT_AND_TARGET_MEMORY_MISMATCH ,
    DRV_IH_ERROR_INVALID_PORT ,
    DRV_IH_ERROR_INVALID_L3_OFFSET ,
    DRV_IH_ERROR_INVALID_VID ,
    DRV_IH_ERROR_INVALID_DSCP ,
    DRV_IH_ERROR_INVALID_TCI ,
    DRV_IH_ERROR_INVALID_TABLE_SIZE ,
    DRV_IH_ERROR_INVALID_MAXIMAL_SEARCH_DEPTH ,
    DRV_IH_ERROR_INVALID_START_OFFSET_SEATCH_KEY_PART ,
    DRV_IH_ERROR_INVALID_SHIFT_OFFSET_SEATCH_KEY_PART ,
    DRV_IH_ERROR_TABLE_IS_NOT_60_BIT_KEY ,
    DRV_IH_ERROR_TABLE_IS_NOT_120_BIT_KEY ,
    DRV_IH_ERROR_CLASS_SEARCH_AND_LUT_LOCATION_MISMATCH ,
    DRV_IH_ERROR_VALUE_IS_WRITE_ONLY
}
DRV_IH_ERROR ;

/* Maximal value for maximal number of runner buffers allocated to runner */
#define DRV_IH_MAXIMAL_VALUE_FOR_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS ( 64 )
/* Maximal ingress queue congestion threshold.
   Needed to allow threshold=65 for following scenario: maximal number of
   runner buffers is 64, and we want to never reach the threshold (i.e.
   IH will never stop serving the queue).  */
#define DRV_IH_MAXIMAL_INGRESS_QUEUE_CONGESTION_THRESHOLD         ( DRV_IH_MAXIMAL_VALUE_FOR_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS + 1 )
/* Number of lookup tables */
#define DRV_IH_NUMBER_OF_LOOKUP_TABLES                            ( 10 )
/* Number of classes */
#define DRV_IH_NUMBER_OF_CLASSES                                  ( 16 )
/* Number of classifiers */
#define DRV_IH_NUMBER_OF_CLASSIFIERS                              ( 16 )
/* maximal start offset for part (0 or 1) of a search key (in 4 byte resolution) */
#define DRV_IH_MAXIMAL_START_OFFSET_SEATCH_KEY_PART               ( 14 )
/* maximal shift offset for part (0 or 1) of a search key (in 4 bit resolution) */
#define DRV_IH_MAXIMAL_SHIFT_OFFSET_SEATCH_KEY_PART               ( 15 )
/* Number of DSCP-to-TCI tables. these tables are used for untagged IP packets */
#define DRV_IH_NUMBER_OF_DSCP_TO_TCI_TABLES                       ( 2 )
/* Number of ingress queues */
#define DRV_IH_NUMBER_OF_INGRESS_QUEUES                           ( 8 )
/* Ingress queues array size */
#define DRV_IH_INGRESS_QUEUES_ARRAY_SIZE                          ( 16 )
/* Minimal ingress queue size. */
#define DRV_IH_MINIMAL_INGRESS_QUEUE_SIZE                         ( 1 )
/* Maximal ingress queue priority. lower priority value means lower priority. */
#define DRV_IH_MAXIMAL_INGRESS_QUEUE_PRIORITY                      ( 7 )
/* Maximal ingress queue weight. lower weight value means lower weight.
   Weight is relevant only for two or more queues with the same priority */
#define DRV_IH_MAXIMAL_INGRESS_QUEUE_WEIGHT                        ( 15 )
/* number of bytes in MAC address */
#define DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS                      ( 6 )
/* number of DA filters with mask */
#define DRV_IH_NUMBER_OF_DA_FILTERS_WITH_MASK                      ( 2 )
/* number of DA filters (with and without mask) */
#define DRV_IH_NUMBER_OF_DA_FILTERS                                ( 8 )
/* Number of user-defined ethertypes */
#define DRV_IH_NUMBER_OF_USER_DEFINED_ETHERTYPES                   ( 4 )
/* Maximal L3 offset */
#define DRV_IH_MAXIMAL_L3_OFFSET                                   ( 15 )
/* Number of user-defined IP L4 protocols */
#define DRV_IH_NUMBER_OF_USER_DEFINED_IP_L4_PROTOCOLS              ( 4 )
/* Number of PPP Protocol Codes */
#define DRV_IH_NUMBER_OF_PPP_PROTOCOL_CODES                        ( 2 )
/* Number of VID filters */
#define DRV_IH_NUMBER_OF_VID_FILTERS                               ( 12 )
/* Maximal VID value */
#define DRV_IH_MAXIMAL_VID_VALUE                                   ( 4095 )
/* Number of IP filters */
#define DRV_IH_NUMBER_OF_IP_FILTERS                                ( 4 )
/* Maximal DSCP value */
#define DRV_IH_MAXIMAL_DSCP_VALUE                                  ( 63 )
/* Maximal TCI value */
#define DRV_IH_MAXIMAL_TCI_VALUE                                   ( 7 )

#define CTD_RUNNER_A                                               0
#define CTD_RUNNER_B                                               1
#define CONGESTION_THRESHOLD_DISABLE_MASK_SET(RUNNER, QUEUE)       (1 << ((QUEUE * 2) + RUNNER))
    

/******************************************************************************/
/* General configuration                                                      */
/******************************************************************************/
typedef struct
{
    /* In runner flow, IH response is sent to runner after completing writing
       the runner buffer. */
    uint16_t runner_a_ih_response_address ;

    /* In runner flow, IH response is sent to runner after completing writing
       the runner buffer. */
    uint16_t runner_b_ih_response_address ;

    /* In case of change of runner congestion state, IH sends Runner
       Congestion report to predefined buffer with number of allocated RBs
       for relevant runner. */
    uint16_t runner_a_ih_congestion_report_address ;

    /* In case of change of runner congestion state, IH sends Runner
       Congestion report to predefined buffer with number of allocated RBs
       for relevant runner. */
    uint16_t runner_b_ih_congestion_report_address ;

    /* Runner A IH congestion report enable */
    int32_t runner_a_ih_congestion_report_enable ;

    /* Runner B IH congestion report enable */
    int32_t runner_b_ih_congestion_report_enable ;

    /* Enable/disable LUT searches when the default direct mode of a class is
       enabled. */
    int32_t lut_searches_enable_in_direct_mode ;

    /* SN stamping enable in direct mode. */
    int32_t sn_stamping_enable_in_direct_mode ;

    /* If a packet's header length is below this threshold, IH will disable
       Parsing & Look-up processing, however the RIB will be assigned + and
       stamped with SN. */
    uint8_t header_length_minimum ;

    /* When this field is set to true, IH will ignore the runner congestion
       state, and discard packets only when there are no available RIBs. */
    int32_t congestion_discard_disable ;

    /* If true, IH will go to CAM (if CAM is enabled) upon encountering an
       invalid LUT entry. If false, IH will stop the search. */
    int32_t cam_search_enable_upon_invalid_lut_entry ;

    /* Disable congestion threshold for given Runner and queue. First bit is
       for Runner A queue 0 second bit is for Runner B queue 0 third bit is
       for Runner queue 1 and so on. */
    uint16_t congestion_threshold_disable;
}
DRV_IH_GENERAL_CONFIG ;


/******************************************************************************/
/* Packet header offsets                                                      */
/******************************************************************************/
typedef struct
{
    /* Eth0 packet header offset */
    uint8_t eth0_packet_header_offset ;

    /* Eth1 packet header offset */
    uint8_t eth1_packet_header_offset ;

    /* Eth2 packet header offset */
    uint8_t eth2_packet_header_offset ;

    /* Eth3 packet header offset */
    uint8_t eth3_packet_header_offset ;

    /* Eth4 packet header offset */
    uint8_t eth4_packet_header_offset ;

    /* GPON packet header offset */
    uint8_t gpon_packet_header_offset ;

    /* Runner A packet header offset */
    uint8_t runner_a_packet_header_offset ;

    /* Runner B packet header offset */
    uint8_t runner_b_packet_header_offset ;
}
DRV_IH_PACKET_HEADER_OFFSETS ;


/******************************************************************************/
/* Runner maximal number of buffers                                           */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_IH_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS_16 ,
    DRV_IH_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS_24 ,
    DRV_IH_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS_32 ,
    DRV_IH_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS_48 ,
    DRV_IH_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS_64
}
DRV_IH_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS ;


/******************************************************************************/
/* Runner Buffers Configuration                                               */
/******************************************************************************/
typedef struct
{
    /* Runner A IH managed RB base address */
    uint16_t runner_a_ih_managed_rb_base_address ;

    /* Runner B IH managed RB base address */
    uint16_t runner_b_ih_managed_rb_base_address ;

    /* Runner A runner managed RB base address */
    uint16_t runner_a_runner_managed_rb_base_address ;

    /* Runner B runner managed RB base address */
    uint16_t runner_b_runner_managed_rb_base_address ;

    /* Runner A maximal number of buffers */
    DRV_IH_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS runner_a_maximal_number_of_buffers ;

    /* Runner B maximal number of buffers */
    DRV_IH_RUNNER_MAXIMAL_NUMBER_OF_BUFFERS runner_b_maximal_number_of_buffers ;
}
DRV_IH_RUNNER_BUFFERS_CONFIG ;


/******************************************************************************/
/* Runners Load Thresholds                                                    */
/******************************************************************************/
typedef struct
{
    /* Threshold for dropping low priority packets */
    uint8_t runner_a_high_congestion_threshold ;

    /* Threshold for dropping low priority packets */
    uint8_t runner_b_high_congestion_threshold ;

    /* Threshold for dropping low & high priority packets */
    uint8_t runner_a_exclusive_congestion_threshold ;

    /* Threshold for dropping low & high priority packets */
    uint8_t runner_b_exclusive_congestion_threshold ;

    /* Threshold for turning-on load balancing */
    uint8_t runner_a_load_balancing_threshold ;

    /* Threshold for turning-on load balancing */
    uint8_t runner_b_load_balancing_threshold ;

    /* Threshold for turning-off load balancing */
    uint8_t runner_a_load_balancing_hysteresis ;

    /* Threshold for turning-off load balancing */
    uint8_t runner_b_load_balancing_hysteresis ;
}
DRV_IH_RUNNERS_LOAD_THRESHOLDS ;


/******************************************************************************/
/* Route Addresses                                                            */
/******************************************************************************/
typedef struct
{
    /* Eth0 route address */
    uint8_t eth0_route_address ;

    /* Eth1 route address */
    uint8_t eth1_route_address ;

    /* Eth2 route address */
    uint8_t eth2_route_address ;

    /* Eth3 route address */
    uint8_t eth3_route_address ;

    /* Eth4 route address */
    uint8_t eth4_route_address ;

    /* GPON route address */
    uint8_t gpon_route_address ;

    /* Runner A route address */
    uint8_t runner_a_route_address ;

    /* Runner B route address */
    uint8_t runner_b_route_address ;
}
DRV_IH_ROUTE_ADDRESSES ;


/******************************************************************************/
/* Parsing layer depth                                                        */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_IH_PARSING_LAYER_DEPTH_VLAN ,
    DRV_IH_PARSING_LAYER_DEPTH_LAYER2 ,
    DRV_IH_PARSING_LAYER_DEPTH_LAYER3 ,
    DRV_IH_PARSING_LAYER_DEPTH_LAYER4
}
DRV_IH_PARSING_LAYER_DEPTH ;


/******************************************************************************/
/* Proprietary tag size                                                       */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_IH_PROPRIETARY_TAG_SIZE_0 = 0 ,
	DRV_IH_PROPRIETARY_TAG_SIZE_4 = 4 ,
	DRV_IH_PROPRIETARY_TAG_SIZE_6 = 6 ,
	DRV_IH_PROPRIETARY_TAG_SIZE_8 = 8
}
DRV_IH_PROPRIETARY_TAG_SIZE ;


/******************************************************************************/
/* Logical Port Configuration                                                 */
/******************************************************************************/
typedef struct
{
    /* Parsing layer depth */
    DRV_IH_PARSING_LAYER_DEPTH parsing_layer_depth ;

    /* Proprietary tag size */
    DRV_IH_PROPRIETARY_TAG_SIZE proprietary_tag_size ;
}
DRV_IH_LOGICAL_PORT_CONFIG ;


/******************************************************************************/
/* Logical Ports Configuration                                                */
/******************************************************************************/
typedef struct
{
    /* Eth0 configuration */
    DRV_IH_LOGICAL_PORT_CONFIG eth0_config ;

    /* Eth1 configuration */
    DRV_IH_LOGICAL_PORT_CONFIG eth1_config ;

    /* Eth2 configuration */
    DRV_IH_LOGICAL_PORT_CONFIG eth2_config ;

    /* Eth3 configuration */
    DRV_IH_LOGICAL_PORT_CONFIG eth3_config ;

    /* Eth4 configuration */
    DRV_IH_LOGICAL_PORT_CONFIG eth4_config ;

    /* GPON configuration */
    DRV_IH_LOGICAL_PORT_CONFIG gpon_config ;

    /* Runner A configuration */
    DRV_IH_LOGICAL_PORT_CONFIG runner_a_config ;

    /* Runner B configuration */
    DRV_IH_LOGICAL_PORT_CONFIG runner_b_config ;

    /* PCIE0 configuration */
    DRV_IH_LOGICAL_PORT_CONFIG pcie0_config ;

    /* PCIE1 configuration */
    DRV_IH_LOGICAL_PORT_CONFIG pcie1_config ;
}
DRV_IH_LOGICAL_PORTS_CONFIG ;




/******************************************************************************/
/* Lookup table size                                                          */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_IH_LOOKUP_TABLE_SIZE_32_ENTRIES ,
    DRV_IH_LOOKUP_TABLE_SIZE_64_ENTRIES ,
    DRV_IH_LOOKUP_TABLE_SIZE_128_ENTRIES ,
    DRV_IH_LOOKUP_TABLE_SIZE_256_ENTRIES ,
    DRV_IH_LOOKUP_TABLE_SIZE_512_ENTRIES ,
    DRV_IH_LOOKUP_TABLE_SIZE_1024_ENTRIES ,
    DRV_IH_LOOKUP_TABLE_SIZE_2048_ENTRIES ,
    DRV_IH_LOOKUP_TABLE_SIZE_4096_ENTRIES
}
DRV_IH_LOOKUP_TABLE_SIZE ;


/******************************************************************************/
/* Lookup table hash type                                                     */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_IH_LOOKUP_TABLE_HASH_TYPE_HASH_FOR_INCREMENTAL_KEYS ,
    DRV_IH_LOOKUP_TABLE_HASH_TYPE_CRC16
}
DRV_IH_LOOKUP_TABLE_HASH_TYPE ;



/******************************************************************************/
/* Lookup table maximal search depth                                          */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_1_STEP ,
    DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_2_STEPS ,
    DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_4_STEPS ,
    DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_8_STEPS ,
    DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_16_STEPS ,
    DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_32_STEPS ,
    DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_64_STEPS ,
    DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_128_STEPS ,
    DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_256_STEPS ,
    DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_512_STEPS ,
    DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH_1024_STEPS
}
DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH ;


/******************************************************************************/
/* Lookup context table entry size                                            */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_IH_LOOKUP_CONTEXT_TABLE_ENTRY_SIZE_1_BYTE ,
    DRV_IH_LOOKUP_CONTEXT_TABLE_ENTRY_SIZE_2_BYTES ,
    DRV_IH_LOOKUP_CONTEXT_TABLE_ENTRY_SIZE_4_BYTES ,
    DRV_IH_LOOKUP_CONTEXT_TABLE_ENTRY_SIZE_INTERNAL_ENTRY
}
DRV_IH_LOOKUP_CONTEXT_TABLE_ENTRY_SIZE ;


/******************************************************************************/
/* Lookup key extension                                                       */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_IH_LOOKUP_KEY_EXTENSION_DISABLE ,
    DRV_IH_LOOKUP_KEY_EXTENSION_SOURCE_PORT ,
    DRV_IH_LOOKUP_KEY_EXTENSION_GEM_FLOW_ID ,
    DRV_IH_LOOKUP_KEY_EXTENSION_WAN
}
DRV_IH_LOOKUP_KEY_EXTENSION ;


/******************************************************************************/
/* Lookup table 60 bit key configuration                                      */
/******************************************************************************/
typedef struct
{
    /* Table base address in 8 byte resolution */
    uint16_t table_base_address_in_8_byte ;

    /* Table size */
    DRV_IH_LOOKUP_TABLE_SIZE table_size ;

    /* Maximal search depth */
    DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH maximal_search_depth ;

    /* Hash type */
    DRV_IH_LOOKUP_TABLE_HASH_TYPE hash_type ;

    /* This flag invokes additional comparison of source port for "move"
       indication */
    int32_t sa_search_enable ;

    /* When this flag is enabled, look-up engine will de-assert aging bit in
       case of search match */
    int32_t aging_enable ;

    /* CAM enable */
    int32_t cam_enable ;

    /* CAM base address in 8 byte resolution */
    uint16_t cam_base_address_in_8_byte ;

    /* Context table base address in 8 byte resolution */
    uint16_t context_table_base_address_in_8_byte ;

    /* Context table entry size */
    DRV_IH_LOOKUP_CONTEXT_TABLE_ENTRY_SIZE context_table_entry_size ;

    /* CAM context base address in 8 byte resolution */
    uint16_t cam_context_base_address_in_8_byte ;

    /* Part 0 start offset in 4 byte resolution */
    uint8_t part_0_start_offset_in_4_byte ;

    /* Part 0 shift offset in 4 bit resolution */
    uint8_t part_0_shift_offset_in_4_bit ;

    /* Part 1 start offset in 4 byte resolution */
    uint8_t part_1_start_offset_in_4_byte ;

    /* Part 1 shift offset in 4 bit resolution */
    uint8_t part_1_shift_offset_in_4_bit ;

    /* Extension option for search key (always concatenated with MS bits) */
    DRV_IH_LOOKUP_KEY_EXTENSION key_extension ;

    /* Lower 32 bits of part 0 mask */
    uint32_t part_0_mask_low ;

    /* higher 28 bits of part 0 mask */
    uint32_t part_0_mask_high ;

    /* Lower 32 bits of part 1 mask */
    uint32_t part_1_mask_low ;

    /* higher 28 bits of part 1 mask */
    uint32_t part_1_mask_high ;

    /* Global mask in 4 bit resolution (only 15 LS bits are used, 15*4=60
       bits mask) */
    uint16_t global_mask_in_4_bit ;
}
DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG ;


/******************************************************************************/
/* Lookup table 120 bit key configuration                                     */
/******************************************************************************/
typedef struct
{
    /* Table base address in 8 byte resolution */
    uint16_t table_base_address_in_8_byte ;

    /* Table size */
    DRV_IH_LOOKUP_TABLE_SIZE table_size ;

    /* Maximal search depth */
    DRV_IH_LOOKUP_TABLE_MAXIMAL_SEARCH_DEPTH maximal_search_depth ;

    /* Hash type */
    DRV_IH_LOOKUP_TABLE_HASH_TYPE hash_type ;

    /* When this flag is enabled, look-up engine will de-assert aging bit in
       case of search match */
    int32_t aging_enable ;

    /* CAM enable */
    int32_t cam_enable ;

    /* CAM base address in 8 byte resolution */
    uint16_t cam_base_address_in_8_byte ;

    /* Context table base address in 8 byte resolution */
    uint16_t context_table_base_address_in_8_byte ;

    /* Context table entry size */
    DRV_IH_LOOKUP_CONTEXT_TABLE_ENTRY_SIZE context_table_entry_size ;

    /* CAM context base address in 8 byte resolution */
    uint16_t cam_context_base_address_in_8_byte ;

    /* Primary key part 0 start offset in 4 byte resolution */
    uint8_t primary_key_part_0_start_offset_in_4_byte ;

    /* Primary key part 0 shift offset in 4 bit resolution */
    uint8_t primary_key_part_0_shift_offset_in_4_bit ;

    /* Primary key part 1 start offset in 4 byte resolution */
    uint8_t primary_key_part_1_start_offset_in_4_byte ;

    /* Primary key part 1 shift offset in 4 bit resolution */
    uint8_t primary_key_part_1_shift_offset_in_4_bit ;

    /* Extension option for search key (always concatenated with MS bits) */
    DRV_IH_LOOKUP_KEY_EXTENSION primary_key_extension ;

    /* Lower 32 bits of part 0 mask */
    uint32_t primary_key_part_0_mask_low ;

    /* higher 28 bits of part 0 mask */
    uint32_t primary_key_part_0_mask_high ;

    /* Lower 32 bits of part 1 mask */
    uint32_t primary_key_part_1_mask_low ;

    /* higher 28 bits of part 1 mask */
    uint32_t primary_key_part_1_mask_high ;

    /* Secondary key part 0 start offset in 4 byte resolution */
    uint8_t secondary_key_part_0_start_offset_in_4_byte ;

    /* Secondary key part 0 shift offset in 4 bit resolution */
    uint8_t secondary_key_part_0_shift_offset_in_4_bit ;

    /* Secondary key part 1 start offset in 4 byte resolution */
    uint8_t secondary_key_part_1_start_offset_in_4_byte ;

    /* Secondary key part 1 shift offset in 4 bit resolution */
    uint8_t secondary_key_part_1_shift_offset_in_4_bit ;

    /* Extension option for search key (always concatenated with MS bits) */
    DRV_IH_LOOKUP_KEY_EXTENSION secondary_key_extension ;

    /* Lower 32 bits of part 0 mask */
    uint32_t secondary_key_part_0_mask_low ;

    /* higher 28 bits of part 0 mask */
    uint32_t secondary_key_part_0_mask_high ;

    /* Lower 32 bits of part 1 mask */
    uint32_t secondary_key_part_1_mask_low ;

    /* higher 28 bits of part 1 mask */
    uint32_t secondary_key_part_1_mask_high ;

    /* Global mask in 4 bit resolution (only 15 LS bits are used, 15*4=60
       bits mask) */
    uint16_t global_mask_in_4_bit ;
}
DRV_IH_LOOKUP_TABLE_120_BIT_KEY_CONFIG ;


/******************************************************************************/
/* Class search                                                               */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_0 ,
    DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_1 ,
    DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_2 ,
    DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_3 ,
    DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_4 ,
    DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_5 ,
    DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_6 ,
    DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_7 ,
    DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_8 ,
    DRV_IH_CLASS_SEARCH_LOOKUP_TABLE_9 ,
    DRV_IH_CLASS_SEARCH_DISABLED       = 15
}
DRV_IH_CLASS_SEARCH ;


/******************************************************************************/
/* Operation based on class search                                            */
/* There are several operations which can be either diabled or based on       */
/* search 1 or based on search 3.                                             */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_OPERATION_DISABLED ,
    DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_BASED_ON_SEARCH1 ,
    DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH_BASED_ON_SEARCH3
}
DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH ;


/******************************************************************************/
/* Target memory                                                              */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_IH_TARGET_MEMORY_DDR ,
    DRV_IH_TARGET_MEMORY_SRAM
}
DRV_IH_TARGET_MEMORY ;


/******************************************************************************/
/* Ingress QOS                                                                */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_IH_INGRESS_QOS_LOW = 0 ,
    DRV_IH_INGRESS_QOS_HIGH = 1 ,
    DRV_IH_INGRESS_QOS_EXCLUSIVE = 3
}
DRV_IH_INGRESS_QOS ;


/******************************************************************************/
/* Runner cluster                                                             */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_IH_RUNNER_CLUSTER_A ,
    DRV_IH_RUNNER_CLUSTER_B
}
DRV_IH_RUNNER_CLUSTER ;


/******************************************************************************/
/* Class Configuration                                                        */
/******************************************************************************/
typedef struct
{
    /* Class search 1 */
    DRV_IH_CLASS_SEARCH class_search_1 ;

    /* Class search 2 */
    DRV_IH_CLASS_SEARCH class_search_2 ;

    /* Class search 3 */
    DRV_IH_CLASS_SEARCH class_search_3 ;

    /* Class search 4 */
    DRV_IH_CLASS_SEARCH class_search_4 ;

    /* Destination Port extraction. Destination Port is used when accessing target matrix. */
    DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH destination_port_extraction ;

    /* Drop on miss */
    DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH drop_on_miss ;

    /* DSCP to TCI table index. Used for untagged IP packets. */
    uint8_t dscp_to_tci_table_index ;

    /* Direct mode default */
    int32_t direct_mode_default ;

    /* Direct mode override. If true, direct-mode is taken from Target Matrix */
    int32_t direct_mode_override ;

    /* Target memory default */
    DRV_IH_TARGET_MEMORY target_memory_default ;

    /* Target memory override. If true, target memory is taken from Target
       Matrix */
    int32_t target_memory_override ;

    /* Ingress QoS default */
    DRV_IH_INGRESS_QOS ingress_qos_default ;

    /* Ingress QoS override. */
    DRV_IH_OPERATION_BASED_ON_CLASS_SEARCH ingress_qos_override ;

    /* Target Runner Default */
    DRV_IH_RUNNER_CLUSTER target_runner_default ;

    /* Target Runner override in direct mode. If enabled, no load balancing
       is performed. Direct mode decision might be done based on Target
       Matrix. */
    int32_t target_runner_override_in_direct_mode ;

    /* Target Runner for direct mode. If "Target Runner override in direct
       mode" is enabled, this target runner is chosen instead of the default
       target runner */
    DRV_IH_RUNNER_CLUSTER target_runner_for_direct_mode ;

    /* Load balancing enable. Enable: Behavior according to Preference Load
       Balancing. Disable: default target runner is used. */
    int32_t load_balancing_enable ;

    /* Preference Load Balancing enable. Enable: prefer default runner.
       Disable: prefer previous runner. Irrelevant if load balancing is
       disabled. */
    int32_t preference_load_balancing_enable ;
}
DRV_IH_CLASS_CONFIG ;


/******************************************************************************/
/* Logical port                                                               */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_IH_LOGICAL_PORT_ETH0      = 0x0 ,
    DRV_IH_LOGICAL_PORT_ETH1      = 0x1 ,
    DRV_IH_LOGICAL_PORT_ETH2      = 0x2 ,
    DRV_IH_LOGICAL_PORT_ETH3      = 0x3 ,
    DRV_IH_LOGICAL_PORT_ETH4      = 0x4,
    DRV_IH_LOGICAL_PORT_GPON      = 0x5 ,
    DRV_IH_LOGICAL_PORT_RUNNER_A  = 0x6 ,
    DRV_IH_LOGICAL_PORT_RUNNER_B  = 0xE ,
    DRV_IH_LOGICAL_PORT_PCIE0     = 0x7 ,
    DRV_IH_LOGICAL_PORT_PCIE1     = 0x8
}
DRV_IH_LOGICAL_PORT ;


/******************************************************************************/
/* L2 protocol                                                                */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_IH_L2_PROTOCOL_UNKNOWN = 0x0 ,
    DRV_IH_L2_PROTOCOL_PPPOE_DISCOVERY = 0x1 ,
    DRV_IH_L2_PROTOCOL_PPPOE_SESSION = 0x2 ,
    DRV_IH_L2_PROTOCOL_IPV4OE = 0x4 ,
    DRV_IH_L2_PROTOCOL_IPV6OE = 0x5 ,
    DRV_IH_L2_PROTOCOL_USER_DEFINED_0 = 0x8 ,
    DRV_IH_L2_PROTOCOL_USER_DEFINED_1 = 0x9 ,
    DRV_IH_L2_PROTOCOL_USER_DEFINED_2 = 0xA ,
    DRV_IH_L2_PROTOCOL_USER_DEFINED_3 = 0xB ,
    DRV_IH_L2_PROTOCOL_ARP = 0xC ,
    DRV_IH_L2_PROTOCOL_TYPE1588 = 0xD ,
    DRV_IH_L2_PROTOCOL_TYPE8021X = 0xE ,
    DRV_IH_L2_PROTOCOL_TYPE8011AGCFM = 0xF
}
DRV_IH_L2_PROTOCOL ;


/******************************************************************************/
/* L3 protocol                                                                */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_IH_L3_PROTOCOL_NONE ,
    DRV_IH_L3_PROTOCOL_IPV4 ,
    DRV_IH_L3_PROTOCOL_IPV6
}
DRV_IH_L3_PROTOCOL ;


/******************************************************************************/
/* L4 protocol                                                                */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_IH_L4_PROTOCOL_NONE = 0x0 ,
    DRV_IH_L4_PROTOCOL_TCP = 0x1 ,
    DRV_IH_L4_PROTOCOL_UDP = 0x2 ,
    DRV_IH_L4_PROTOCOL_IGMP = 0x3 ,
    DRV_IH_L4_PROTOCOL_ICMP = 0x4 ,
    DRV_IH_L4_PROTOCOL_ICMPV6 = 0x5 ,
    DRV_IH_L4_PROTOCOL_ESP = 0x6 ,
    DRV_IH_L4_PROTOCOL_GRE = 0x7 ,
    DRV_IH_L4_PROTOCOL_USER_DEFINED_0 = 0x8 ,
    DRV_IH_L4_PROTOCOL_USER_DEFINED_1 = 0x9 ,
    DRV_IH_L4_PROTOCOL_USER_DEFINED_2 = 0xA ,
    DRV_IH_L4_PROTOCOL_USER_DEFINED_3 = 0xB ,
    DRV_IH_L4_PROTOCOL_IPV6 = 0xD ,
    DRV_IH_L4_PROTOCOL_AH = 0xE ,
    DRV_IH_L4_PROTOCOL_NOT_PARSED = 0XF
}
DRV_IH_L4_PROTOCOL ;


/******************************************************************************/
/* Classifier configuration                                                   */
/******************************************************************************/
typedef struct
{
    /* L2 protocol */
    DRV_IH_L2_PROTOCOL l2_protocol ;

    /* L3 protocol */
    DRV_IH_L3_PROTOCOL l3_protocol ;

    /* L4 protocol */
    DRV_IH_L4_PROTOCOL l4_protocol ;

    /* DA filter any hit */
    int32_t da_filter_any_hit ;

    /* Matched DA filter (There are 6 DA filters) */
    uint8_t matched_da_filter ;

    /* Multicast DA indication */
    int32_t multicast_da_indication ;

    /* Broadcast DA indication */
    int32_t broadcast_da_indication ;

    /* VID filter any hit */
    int32_t vid_filter_any_hit ;

    /* Matched VID filter (There are 12 VID filters) */
    uint8_t matched_vid_filter ;

    /* IP filter any hit */
    int32_t ip_filter_any_hit ;

    /* Matched IP filter (There are 4 IP filters) */
    uint8_t matched_ip_filter ;

    /* WAN indication */
    int32_t wan_indication ;

    /* Five tuple valid */
    int32_t five_tuple_valid ;

    /* Logical source port */
    DRV_IH_LOGICAL_PORT logical_source_port ;

    /* Error */
    int32_t error ;

    /* 32 bit mask */
    uint32_t mask ;

    /* Resulting class */
    uint8_t resulting_class ;
}
DRV_IH_CLASSIFIER_CONFIG ;


/******************************************************************************/
/* Source port to ingress queue mapping                                       */
/******************************************************************************/
typedef struct
{
    /* Eth0 ingress queue */
    uint8_t eth0_ingress_queue ;

    /* Eth1 ingress queue */
    uint8_t eth1_ingress_queue ;

    /* Eth2 ingress queue */
    uint8_t eth2_ingress_queue ;

    /* Eth3 ingress queue */
    uint8_t eth3_ingress_queue ;

    /* Eth4 ingress queue */
    uint8_t eth4_ingress_queue ;

    /* GPON ingress queue */
    uint8_t gpon_ingress_queue ;

    /* Runner A ingress queue */
    uint8_t runner_a_ingress_queue ;

    /* Runner B ingress queue */
    uint8_t runner_b_ingress_queue ;
}
DRV_IH_SOURCE_PORT_TO_INGRESS_QUEUE_MAPPING ;


/******************************************************************************/
/* Ingress queue configuration                                                */
/******************************************************************************/
typedef struct
{
    /* Base location in the 16-entries ingress queue array */
    uint8_t base_location ;

    /* Size. Sum of sizes of all queues is up to 16 */
    uint8_t size ;

    /* Priority. lower value means lower priority. allowed values: 0-7 */
    uint8_t priority ;

    /* Weight. lower weight value means lower weight. Weight is relevant only
       for two or more queues with the same priority. */
    uint8_t weight ;

    /* Congestion threshold. When the number of total Runner Buffers (either
       Runner A or Runner B) reaches the defined threshold per queue - IH
       stops serving this queue. */
    uint8_t congestion_threshold ;
}
DRV_IH_INGRESS_QUEUE_CONFIG ;


/******************************************************************************/
/* Target Matrix source port                                                  */
/******************************************************************************/
typedef enum
{
    DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH0 ,
    DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH1 ,
    DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH2 ,
    DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH3 ,
    DRV_IH_TARGET_MATRIX_SOURCE_PORT_ETH4 ,
    DRV_IH_TARGET_MATRIX_SOURCE_PORT_GPON ,
    DRV_IH_TARGET_MATRIX_SOURCE_PORT_PCIE0 ,
    DRV_IH_TARGET_MATRIX_SOURCE_PORT_PCIE1 ,

    DRV_IH_TARGET_MATRIX_NUMBER_OF_SOURCE_PORTS
}
DRV_IH_TARGET_MATRIX_SOURCE_PORT ;


/******************************************************************************/
/* Target Matrix destination port                                             */
/******************************************************************************/
typedef enum
{
    DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH0 ,
    DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH1 ,
    DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH2 ,
    DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH3 ,
    DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ETH4 ,
    DRV_IH_TARGET_MATRIX_DESTINATION_PORT_GPON ,
    DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE0 ,
    DRV_IH_TARGET_MATRIX_DESTINATION_PORT_PCIE1 ,
    DRV_IH_TARGET_MATRIX_DESTINATION_PORT_CPU ,
    DRV_IH_TARGET_MATRIX_DESTINATION_PORT_MULTICAST ,
    DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ALWAYS_DDR ,
    DRV_IH_TARGET_MATRIX_DESTINATION_PORT_ALWAYS_SRAM ,
    DRV_IH_TARGET_MATRIX_DESTINATION_PORT_SPARE ,

    DRV_IH_TARGET_MATRIX_NUMBER_OF_DESTINATION_PORTS
}
DRV_IH_TARGET_MATRIX_DESTINATION_PORT ;


/******************************************************************************/
/* Target matrix entry                                                        */
/******************************************************************************/
typedef struct
{
    /* target memory */
    DRV_IH_TARGET_MEMORY target_memory ;

    /* direct mode */
    int32_t direct_mode ;
}
DRV_IH_TARGET_MATRIX_ENTRY ;



/******************************************************************************/
/* Target matrix per SP configuration                                         */
/******************************************************************************/
typedef struct
{
    /* an array of all entries of a certain source port. */
    DRV_IH_TARGET_MATRIX_ENTRY entry [ DRV_IH_TARGET_MATRIX_NUMBER_OF_DESTINATION_PORTS ] ;
}
DRV_IH_TARGET_MATRIX_PER_SP_CONFIG ;


/******************************************************************************/
/* WAN ports configuration                                                    */
/* value of true indicates that the corresponding port belongs to WAN         */
/******************************************************************************/
typedef struct
{
    /* Eth0 */
    int32_t eth0 ;

    /* Eth1 */
    int32_t eth1 ;

    /* Eth2 */
    int32_t eth2 ;

    /* Eth3 */
    int32_t eth3 ;

    /* Eth4 */
    int32_t eth4 ;

    /* GPON */
    int32_t gpon ;

    /* Runner A */
    int32_t runner_a ;

    /* Runner B */
    int32_t runner_b ;

    /* PCIE0 */
    int32_t pcie0 ;

    /* PCIE1 */
    int32_t pcie1 ;
}
DRV_IH_WAN_PORTS_CONFIG ;


/******************************************************************************/
/* Ethertypes for QTAG nesting                                                */
/* The first 2 Ethertypes indices are for the user defined Ethertypes         */
/* configured by Set Ethertypes for QTAG Identification API.                  */
/******************************************************************************/
typedef enum
{
    DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_USER_DEFIEND_0 ,
    DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_USER_DEFIEND_1 ,
    DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_8100 ,
    DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_88A8 ,
    DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_9100 ,
    DRV_IH_ETHERTYPE_FOR_QTAG_NESTING_9200 ,

    DRV_IH_NUMBER_OF_ETHERTYPES_FOR_QTAG_NESTING
}
DRV_IH_ETHERTYPE_FOR_QTAG_NESTING ;


/******************************************************************************/
/* Critical Bits                                                              */
/******************************************************************************/
typedef struct
{
    int32_t iq_fifo_full ;

    int32_t iq0_fifo_full ;

    int32_t iq1_fifo_full ;

    int32_t iq2_fifo_full ;

    int32_t iq3_fifo_full ;

    int32_t iq4_fifo_full ;

    int32_t iq5_fifo_full ;

    int32_t iq6_fifo_full ;

    int32_t iq7_fifo_full ;

    int32_t lookup_1_stuck ;

    int32_t lookup_2_stuck ;

    int32_t lookup_3_stuck ;

    int32_t lookup_4_stuck ;

    int32_t look_up_packet_command_fifo_full ;

    int32_t egress_tx_data_fifo_full ;

    int32_t egress_tx_message_fifo_full ;

    int32_t egress_queue_packet_command_fifo_full ;
}
DRV_IH_CRITICAL_BITS ;


/******************************************************************************/
/* Parser Configuration                                                       */
/******************************************************************************/
typedef struct
{
    /* Defines which TCP flags set will cause TCP_FLAG bit in summary word to
       be set */
    uint8_t tcp_flags ;

    /* 4-bits mask which defines which status bits will cause exception bit
       in summary word to be set. The cause vector is {IP fragment, IP
       version error, IP checksum error, IP header length error}. */
    uint8_t exception_status_bits ;

    /* when this flag is set, PPP with code 1 is identified as IPV6 (instead of IPV4) */
    int32_t ppp_code_1_ipv6 ;

    /* 3-bits mask which defines which IPV6 extension headers types will cause
       'IPV6 extension header' indication in the summary word. the bits order is:
       {Destination Options (MSB), Routing Header, Hop-by-Hop Options (LSB)} */
    uint8_t ipv6_extension_header_bitmask ;

    /* SNAP user defined organization Code. 24 bit value. This is a
       user-defined code which is used in addition to RFC1042 and 802.1Q (in
       case they are enalbed). */
    uint32_t snap_user_defined_organization_code ;

    /* SNAP RFC1042 encapsulation enable. */
    int32_t snap_rfc1042_encapsulation_enable ;

    /* SNAP 802.1Q encapsulation enable. */
    int32_t snap_802_1q_encapsulation_enable ;

    /* GRE protocol. Used for VPN. 16 bit value which is compared to "Protocol
       type" field of the GRE header. */
    uint16_t gre_protocol ;

	uint32_t eng_cfg;
}
DRV_IH_PARSER_CONFIG ;


/******************************************************************************/
/* IP Filter Selection                                                        */
/* The enumeration values are compatible with the values used by the HW.      */
/******************************************************************************/
typedef enum
{
    DRV_IH_IP_FILTER_SELECTION_SOURCE_IP ,
    DRV_IH_IP_FILTER_SELECTION_DESTINATION_IP
}
DRV_IH_IP_FILTER_SELECTION ;



/******************************************************************************/
/*                                                                            */
/* Functions prototypes                                                       */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_general_configuration                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set general configuration                                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets general configuration of the IH block.                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ih_general_config - IH general configuration.                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_general_configuration ( const DRV_IH_GENERAL_CONFIG * xi_ih_general_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_general_configuration                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get general configuration                                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets general configuration of the IH block.                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_ih_general_config - IH general configuration.                         */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_general_configuration ( DRV_IH_GENERAL_CONFIG * const xo_ih_general_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_packet_header_offsets                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set packet header offsets                                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets packet header offset for each physical port.          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_packet_header_offsets - Packet header offsets                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_packet_header_offsets ( const DRV_IH_PACKET_HEADER_OFFSETS * xi_packet_header_offsets ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_packet_header_offsets                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get packet header offsets                                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets packet header offset for each physical port.          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_packet_header_offsets - Packet header offsets                         */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_packet_header_offsets ( DRV_IH_PACKET_HEADER_OFFSETS * const xo_packet_header_offsets ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_runner_buffers_configuration                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set Runner Buffers configuration                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets runner-buffers related configuration, for each        */
/*   runner.                                                                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_runner_buffers_config - Runner Buffers Configuration                  */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_runner_buffers_configuration ( const DRV_IH_RUNNER_BUFFERS_CONFIG * xi_runner_buffers_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_runner_buffers_configuration                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Runner Buffers configuration                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets runner-buffers related configuration, for each        */
/*   runner.                                                                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_runner_buffers_config - Runner Buffers Configuration                  */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_runner_buffers_configuration ( DRV_IH_RUNNER_BUFFERS_CONFIG * const xo_runner_buffers_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_runners_load_thresholds                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set Runners Load Thresholds                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets thresholds related to runner load, for each runner.   */
/*   The thresholds are in number of occupied Runner Buffers.                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_runners_load_thresholds - Runners Load Thresholds (in number of       */
/*     occupied Runner Buffers)                                               */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_runners_load_thresholds ( const DRV_IH_RUNNERS_LOAD_THRESHOLDS * xi_runners_load_thresholds ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_runners_load_thresholds                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Runners Load Thresholds                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets thresholds related to runner load, for each runner.   */
/*   The thresholds are in number of occupied Runner Buffers.                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_runners_load_thresholds - Runners Load Thresholds (in number of       */
/*     occupied Runner Buffers)                                               */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_runners_load_thresholds ( DRV_IH_RUNNERS_LOAD_THRESHOLDS * const xo_runners_load_thresholds ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_route_addresses                                         */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set Route Addresses                                          */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets route address for each physical port. The route       */
/*   address is used for broad-bus access for sending responses, message and  */
/*   data.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_route_addresses - Route Addresses                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_route_addresses ( const DRV_IH_ROUTE_ADDRESSES * xi_route_addresses ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_route_addresses                                         */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Route Addresses                                          */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets route address for each physical port. The route       */
/*   address is used for broad-bus access for sending responses, message and  */
/*   data.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_route_addresses - Route Addresses                                     */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_route_addresses ( DRV_IH_ROUTE_ADDRESSES * const xo_route_addresses ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_logical_ports_configuration                             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set Logical Ports Configuration                              */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets configuration of the following logical ports:         */
/*   Ethernet 0-4, GPON, Runner A, Runner B and PCIE 0-1. The following       */
/*   parameters are configured per port: Parsing layer depth, Proprietary tag */
/*   size.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_logical_ports_config - Logical Ports Configuration                    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_logical_ports_configuration ( const DRV_IH_LOGICAL_PORTS_CONFIG * xi_logical_ports_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_logical_ports_configuration                             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Logical Ports Configuration                              */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets configuration of the following logical ports:         */
/*   Ethernet 0-4, GPON, Runner A, Runner B and PCIE 0-1. The following       */
/*   parameters are configured per port: Parsing layer depth, Proprietary tag */
/*   size.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_logical_ports_config - Logical Ports Configuration                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_logical_ports_configuration ( DRV_IH_LOGICAL_PORTS_CONFIG * const xo_logical_ports_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_configure_lut_60_bit_key                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Configure Lookup Table 60 bit key                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures a lookup table with 60-bit key. There is a      */
/*   total of 10 tables. Note that when configuring a lookup table with       */
/*   120-bit key (with dedicated API), it occupies 2 tables. The lookup key   */
/*   is obtained by ORing two 60-bit parts taken from the parser result. Each */
/*   part has a configurable offset, and optionally a shift & rotate          */
/*   operation. Initially 64 bits are taken from the configured offset, then  */
/*   shift & rotate is optionally done, then the 4 MS-bits are omitted. Then  */
/*   each part is masked with its own mask. Then ORing the left 60 bits of    */
/*   the 2 parts yields the key. Then a key-extension can optionally be done, */
/*   i.e. ORing the MS-bits of the key with one of the following values: (1)  */
/*   5-bit Source Port from Header Descriptor. (2) 8-bit GEM Flow ID from     */
/*   Header Descriptor. (3) 1-bit WAN/LAN indication extracted from           */
/*   configuration of the source port. The global mask is applied on both key */
/*   & LUT entry when comparing between them. Move indication: When "Source   */
/*   port search enable" parameter is enabled, additional comparison will be  */
/*   done, between source-port (from Header descriptor) and bits 56:52 of LUT */
/*   entry, where source-port value should reside. In this case, the global   */
/*   mask must mask these bits for the regular comparison between the key and */
/*   LUT entry, which would be a MAC address comparison. If both comparisons  */
/*   match, the result would be "hit". If only MAC address comparison         */
/*   matches, the result would be "move".                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_table_index - Table index                                             */
/*                                                                            */
/*   xi_lookup_table_60_bit_key_config - Lookup table 60 bit key              */
/*     configuration                                                          */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_configure_lut_60_bit_key ( uint8_t xi_table_index ,
                                                            const DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG * xi_lookup_table_60_bit_key_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_lut_60_bit_key_configuration                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Lookup Table 60 bit key configuration                    */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the configuration of Lookup Table 60 bit key.         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_table_index - Table index                                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_lookup_table_60_bit_key_config - Lookup table 60 bit key              */
/*     configuration                                                          */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_lut_60_bit_key_configuration ( uint8_t xi_table_index ,
                                                                    DRV_IH_LOOKUP_TABLE_60_BIT_KEY_CONFIG * const xo_lookup_table_60_bit_key_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_configure_lut_120_bit_key                                   */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Configure Lookup Table 120 bit key                           */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures a lookup table with 120-bit key. Configuring a  */
/*   lookup table with 120-bit key occupies 2 tables (out of total of 10):    */
/*   primary table and secondary table. The secondary table is used only for  */
/*   generation of the secondary key (see below). The primary table must be   */
/*   defined as search 1 or 3 of a class and the secondary must be 2 of 4     */
/*   respectively. Two 60 bit keys are defined per table: primary key and     */
/*   secondary key. Each one of these keys is generated the same way as       */
/*   explained in Configure Lookup Table 60 bit key API description. The 120  */
/*   bit key is composed of these two 60 bit keys. The 60 bit global mask is  */
/*   applied on secondary key only. SA search (for Move indication) is not    */
/*   supported for 120 bit key.                                               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_primary_table_index - Primary table index                             */
/*                                                                            */
/*   xi_secondary_table_index - Secondary table index                         */
/*                                                                            */
/*   xi_lookup_table_120_bit_key_config - Lookup table 120 bit key            */
/*     configuration                                                          */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_configure_lut_120_bit_key ( uint8_t xi_primary_table_index ,
                                                             uint8_t xi_secondary_table_index ,
                                                             const DRV_IH_LOOKUP_TABLE_120_BIT_KEY_CONFIG * xi_lookup_table_120_bit_key_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_lut_120_bit_key_configuration                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Lookup Table 120 bit key configuration                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the configuration of Lookup Table 120 bit key.        */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_primary_table_index - Primary table index                             */
/*                                                                            */
/*   xi_secondary_table_index - Secondary table index                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_lookup_table_120_bit_key_config - Lookup table 120 bit key            */
/*     configuration                                                          */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_lut_120_bit_key_configuration ( uint8_t xi_primary_table_index ,
                                                                     uint8_t xi_secondary_table_index ,
                                                                     DRV_IH_LOOKUP_TABLE_120_BIT_KEY_CONFIG * const xo_lookup_table_120_bit_key_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_configure_class                                             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Configure Class                                              */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures an IH class. Ingress handler class is type of   */
/*   ingress traffic, e.g. IPTV, bridged, routed. Each class includes         */
/*   predefined set of settings, such as: target runner, destination memory,  */
/*   QoS, definition of look-up searches. There are up to 16 classes. Each    */
/*   physical port has a default class (For GPON port, default class is per   */
/*   GEM flow). IH may override the default class according to                */
/*   enable-override configuration and to classification based on reduced     */
/*   Parser results, called "Classifier Key Word" (Parser Summary Word plus   */
/*   source port). Default classes and override-enable configurations are in  */
/*   BBH. In runner flow, runner assigns an initial class (which can be       */
/*   overridden by IH). Class override is done using classifiers, which are   */
/*   configured using Configure Classifier API.                               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_class_index - Class index                                             */
/*                                                                            */
/*   xi_class_config - Class Configuration                                    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_configure_class ( uint8_t xi_class_index ,
                                                   const DRV_IH_CLASS_CONFIG * xi_class_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_class_configuration                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Class configuration                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the configuration of a Class.                         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_class_index - Class index                                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_class_config - Class Configuration                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_class_configuration ( uint8_t xi_class_index ,
                                                           DRV_IH_CLASS_CONFIG * const xo_class_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_configure_classifier                                        */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Configure Classifier                                         */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures a classifier. There are up to 16 classifiers. A */
/*   classifier is a pair of key and mask, and a resulting class. When class  */
/*   override is enabled, the key is compared to the masked Classifier Key    */
/*   Word (the mask is NOT applied on the key, so user is responsible to set  */
/*   0 at the masked fields/bits in the key). In case of match, the default   */
/*   class is overridden by the classifier's resulting class. If there is a   */
/*   match in more than one classifier, the one with the lower index is       */
/*   chosen.                                                                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_classifier_index - Classifier index                                   */
/*                                                                            */
/*   xi_classifier_config - Classifier configuration                          */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_configure_classifier ( uint8_t xi_classifier_index ,
                                                        const DRV_IH_CLASSIFIER_CONFIG * xi_classifier_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_classifier_configuration                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Classifier configuration                                 */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the configuration of a Classifier.                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_classifier_index - Classifier index                                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_classifier_config - Classifier configuration                          */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_classifier_configuration ( uint8_t xi_classifier_index ,
                                                                DRV_IH_CLASSIFIER_CONFIG * const xo_classifier_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_remove_classifier                                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Remove Classifier                                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function removes a classifier which was configured by Configure     */
/*   Classifier API. There is no "enable" bit - this function actually sets   */
/*   the mask to 0 and the key to 11...1 (bitwise) so there will never be a   */
/*   match.                                                                   */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_classifier_index - Classifier index                                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_remove_classifier ( uint8_t xi_classifier_index ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_source_port_to_ingress_queue_mapping                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set source port to ingress queue mapping                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets the mapping of physical source ports (eth0-4, GPON,   */
/*   runner A, runner B) to ingress queues. There are 8 ingress queues. BBH   */
/*   or runner (in case of runner flow) writes the Header Descriptor to one   */
/*   of these queues, according to the configuration of the source port.      */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_source_port_to_ingress_queue_mapping - Source port to ingress queue   */
/*     mapping                                                                */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_source_port_to_ingress_queue_mapping ( const DRV_IH_SOURCE_PORT_TO_INGRESS_QUEUE_MAPPING * xi_source_port_to_ingress_queue_mapping ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_source_port_to_ingress_queue_mapping                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get source port to ingress queue mapping                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the source port to ingress queue mapping.             */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_source_port_to_ingress_queue_mapping - Source port to ingress queue   */
/*     mapping                                                                */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_source_port_to_ingress_queue_mapping ( DRV_IH_SOURCE_PORT_TO_INGRESS_QUEUE_MAPPING * const xo_source_port_to_ingress_queue_mapping ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_configure_ingress_queue                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Configure ingress queue                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures an ingress queue. There are 8 queues. All of    */
/*   them reside in the same Ingress-queue (IQ) array of 16 entries (ingress  */
/*   buffers). E.g. queue 0 occupies entries 0-1, queue 1 occupies entries    */
/*   2-3, etc.                                                                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ingress_queue_index - Ingress queue index                             */
/*                                                                            */
/*   xi_ingress_queue_config - Ingress queue configuration                    */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_configure_ingress_queue ( uint8_t xi_ingress_queue_index ,
                                                           const DRV_IH_INGRESS_QUEUE_CONFIG * xi_ingress_queue_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_ingress_queue_configuration                             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get ingress queue configuration                              */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the configuration of an ingress queue.                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ingress_queue_index - Ingress queue index                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_ingress_queue_config - Ingress queue configuration                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_ingress_queue_configuration ( uint8_t xi_ingress_queue_index ,
                                                                   DRV_IH_INGRESS_QUEUE_CONFIG * const xo_ingress_queue_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_target_matrix                                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set Target Matrix                                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets the per-source-port configuration in the target       */
/*   matrix, i.e. all entries which belong to the given source port.          */
/*   Each entry consists of the following parameters: target memory           */
/*   (DDR/SRAM), direct mode (true/false).                                    */
/*   The function will fail when trying to configure an "Always DDR" entry    */
/*   with Target memory = SRAM, or "Always SRAM" entry with                   */
/*   Target memory = DDR.                                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_source_port - Source port                                             */
/*                                                                            */
/*   xi_per_sp_config - Per-source-port configuration                         */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_target_matrix ( DRV_IH_TARGET_MATRIX_SOURCE_PORT xi_source_port ,
                                                     const DRV_IH_TARGET_MATRIX_PER_SP_CONFIG * xi_per_sp_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_target_matrix_entry                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Target Matrix entry                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets an entry in the target matrix.                        */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_source_port - Source port                                             */
/*                                                                            */
/*   xi_destination_port - Destination port                                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_entry - Entry                                                         */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_target_matrix_entry ( DRV_IH_TARGET_MATRIX_SOURCE_PORT xi_source_port ,
                                                           DRV_IH_TARGET_MATRIX_DESTINATION_PORT xi_destination_port ,
                                                           DRV_IH_TARGET_MATRIX_ENTRY * const xo_entry ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_forward                                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set Target Forward                                           */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets the "forward-enable" bit for the given source         */
/*   port and destination port.                                               */
/*   The "forward-enable" is only indication to FW (IH doesn't drop if        */
/*   forwarding is disabled).                                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_source_port - Source port                                             */
/*                                                                            */
/*   xi_destination_port - Destination port                                   */
/*                                                                            */
/*   xi_forward - Forward                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_forward ( DRV_IH_TARGET_MATRIX_SOURCE_PORT xi_source_port ,
                                               DRV_IH_TARGET_MATRIX_DESTINATION_PORT xi_destination_port ,
                                               int32_t xi_forward ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_forward                                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Target Forward                                           */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the "forward-enable" bit for the given source         */
/*   port and destination port.                                               */
/*   The "forward-enable" is only indication to FW (IH doesn't drop if        */
/*   forwarding is disabled).                                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_source_port - Source port                                             */
/*                                                                            */
/*   xi_destination_port - Destination port                                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_forward - Forward                                                     */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_forward ( DRV_IH_TARGET_MATRIX_SOURCE_PORT xi_source_port ,
                                               DRV_IH_TARGET_MATRIX_DESTINATION_PORT xi_destination_port ,
                                               int32_t * const xo_forward ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_configure_wan_ports                                         */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Configure WAN ports                                          */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures, for each logical port, whether it belongs to   */
/*   WAN traffic. IH uses this configuration for WAN indication in the parser */
/*   result (and Classifier Key Word).                                        */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_wan_ports_config - WAN ports configuration                            */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_configure_wan_ports ( const DRV_IH_WAN_PORTS_CONFIG * xi_wan_ports_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_wan_ports_configuration                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get WAN ports configuration                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the WAN ports configuration                           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_wan_ports_config - WAN ports configuration                            */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_wan_ports_configuration ( DRV_IH_WAN_PORTS_CONFIG * const xo_wan_ports_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_allocated_runner_buffers_counters                       */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get allocated runner buffers counters                        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function returns the current number of allocated Runner Buffers of  */
/*   each runner.                                                             */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_runner_a_counter - Runner A counter                                   */
/*                                                                            */
/*   xo_runner_b_counter - Runner B counter                                   */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_allocated_runner_buffers_counters ( uint32_t * const xo_runner_a_counter ,
                                                                         uint32_t * const xo_runner_b_counter ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_critical_bits                                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Critical Bits                                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function returns the status of the critical bits (debug             */
/*   indications).                                                            */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_critical_bits - Critical Bits                                         */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_critical_bits ( DRV_IH_CRITICAL_BITS * const xo_critical_bits ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_configure_parser                                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Configure Parser                                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures general parameters in the parser accelerator in */
/*   IH.                                                                      */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_parser_config - Parser Configuration                                  */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_configure_parser ( const DRV_IH_PARSER_CONFIG * xi_parser_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_parser_configuration                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Parser configuration                                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the general parameters configuration in the parser    */
/*   accelerator in IH.                                                       */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_parser_config - Parser Configuration                                  */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_parser_configuration ( DRV_IH_PARSER_CONFIG * const xo_parser_config ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_da_filter_with_mask                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set DA Filter with Mask                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets a DA filter with mask. Allowed filter index: 0-1      */
/*   (only these filters has mask). The filter should be enabled using Enable */
/*   DA Filter API in order to take effect.                                   */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/*   xi_mac_address - MAC address                                             */
/*                                                                            */
/*   xi_mask - Mask                                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_da_filter_with_mask ( uint8_t xi_filter_index ,
                                                           uint8_t xi_mac_address [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] ,
                                                           uint8_t xi_mask [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_da_filter_with_mask                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get DA Filter with Mask                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets a DA filter with mask.                                */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_mac_address - MAC address                                             */
/*                                                                            */
/*   xo_mask - Mask                                                           */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_da_filter_with_mask ( uint8_t xi_filter_index ,
                                                           uint8_t xo_mac_address [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] ,
                                                           uint8_t xo_mask [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_da_filter_without_mask                                  */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set DA Filter without mask                                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets DA filter without mask. Allowed filter index: 2-7     */
/*   (only these filters don't have mask). The filter should be enabled using */
/*   Enable Filter API in order to take effect.                               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/*   xi_mac_address - MAC address                                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_da_filter_without_mask ( uint8_t xi_filter_index ,
                                                              uint8_t xi_mac_address [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_da_filter_without_mask                                  */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get DA Filter without mask                                   */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets DA filter without mask.                               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_mac_address - MAC address                                             */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_da_filter_without_mask ( uint8_t xi_filter_index ,
                                                              uint8_t xo_mac_address [ DRV_IH_NUMBER_OF_BYTES_IN_MAC_ADDRESS ] ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_enable_da_filter                                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Enable DA Filter                                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function enables/disables a DA filter. Before enabling a DA filter, */
/*   it has to be configured by Configure DA Filter with Mask API or          */
/*   Configure DA Filter without Mask API.                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/*   xi_enable - Enable                                                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_enable_da_filter ( uint8_t xi_filter_index ,
                                                    int32_t xi_enable ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_da_filter_enable_status                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get DA filter enable status                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the enable status of a DA filter.                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_enable - Enable                                                       */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_da_filter_enable_status ( uint8_t xi_filter_index ,
                                                               int32_t * const xo_enable ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_ethertypes_for_qtag_identification                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set Ethertypes for QTAG identification                       */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets two user-defined Ethertypes for QTAG (VLAN tag)       */
/*   identification.                                                          */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ethertype_0 - Ethertype 0                                             */
/*                                                                            */
/*   xi_ethertype_1 - Ethertype 1                                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_ethertypes_for_qtag_identification ( uint16_t xi_ethertype_0 ,
                                                                          uint16_t xi_ethertype_1 ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_ethertypes_for_qtag_identification                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get Ethertypes for QTAG identification                       */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the two user-defined Ethertypes for QTAG (VLAN tag)   */
/*   identification                                                           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_ethertype_0 - Ethertype 0                                             */
/*                                                                            */
/*   xo_ethertype_1 - Ethertype 1                                             */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_ethertypes_for_qtag_identification ( uint16_t * const xo_ethertype_0 ,
                                                                          uint16_t * const xo_ethertype_1 ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_configure_qtag_nesting                                      */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Configure QTAG Nesting                                       */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures, for 6 possible Ethertypes, whether each        */
/*   Ethertype can be used for QTAG identification, as inner and as outer     */
/*   tag. Note that when packet has a single tag, parser treats it as outer   */
/*   tag.                                                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ethertype_index - Ethertype index                                     */
/*                                                                            */
/*   xi_use_as_outer - Use as outer                                           */
/*                                                                            */
/*   xi_use_as_inner - Use as inner                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_configure_qtag_nesting ( DRV_IH_ETHERTYPE_FOR_QTAG_NESTING xi_ethertype_index ,
                                                          int32_t xi_use_as_outer ,
                                                          int32_t xi_use_as_inner ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_qtag_nesting_configuration                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get QTAG Nesting configuration                               */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the QTAG Nesting configuration of an Ethertype.       */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ethertype_index - Ethertype index                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_use_as_outer - Use as outer                                           */
/*                                                                            */
/*   xo_use_as_inner - Use as inner                                           */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_qtag_nesting_configuration ( DRV_IH_ETHERTYPE_FOR_QTAG_NESTING xi_ethertype_index ,
                                                                  int32_t * const xo_use_as_outer ,
                                                                  int32_t * const xo_use_as_inner ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_configure_user_ethertype                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Configure User Ethertype                                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures user defined Ethertype, to be indicated in "L2  */
/*   Protocol" field in parser result. There are up to 4 user defined         */
/*   Ethertypes. For such an Ethertype, the API configures which L3 protocol  */
/*   comes after it, and its offset (for L3 parsing). In order to take        */
/*   effect, the user Ethertype should be enabled using Enable User Ethertype */
/*   API.                                                                     */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ethertype_index - Ethertype index                                     */
/*                                                                            */
/*   xi_ethertype_value - Ethertype value                                     */
/*                                                                            */
/*   xi_l3_protocol - L3 protocol                                             */
/*                                                                            */
/*   xi_l3_offset - L3 offset                                                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_configure_user_ethertype ( uint8_t xi_ethertype_index ,
                                                            uint16_t xi_ethertype_value ,
                                                            DRV_IH_L3_PROTOCOL xi_l3_protocol ,
                                                            uint8_t xi_l3_offset ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_user_ethertype_configuration                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get User Ethertype configuration                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets user defined Ethertype configuration.                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ethertype_index - Ethertype index                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_ethertype_value - Ethertype value                                     */
/*                                                                            */
/*   xo_l3_protocol - L3 protocol                                             */
/*                                                                            */
/*   xo_l3_offset - L3 offset                                                 */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_user_ethertype_configuration ( uint8_t xi_ethertype_index ,
                                                                    uint16_t * const xo_ethertype_value ,
                                                                    DRV_IH_L3_PROTOCOL * const xo_l3_protocol ,
                                                                    uint8_t * const xo_l3_offset ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_enable_user_ethertype                                       */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Enable User Ethertype                                        */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function enables/disables a user-defined Ethertype which was        */
/*   configured by Configure User Ethertype API.                              */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ethertype_index - Ethertype index                                     */
/*                                                                            */
/*   xi_enable - Enable                                                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_enable_user_ethertype ( uint8_t xi_ethertype_index ,
                                                         int32_t xi_enable ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_user_ethertype_enable_status                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get User Ethertype enable status                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the enable status of a user-defined Ethertype.        */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ethertype_index - Ethertype index                                     */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_enable - Enable                                                       */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_user_ethertype_enable_status ( uint8_t xi_ethertype_index ,
                                                                    int32_t * const xo_enable ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_user_ip_l4_protocol                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set user IP L4 protocol                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets a user-defined L4 Protocol ID to be matched to        */
/*   Protocol field in IP header and to be indicated in the output summary    */
/*   word. There are up to 4 user-defined L4 protocols.                       */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_l4_protocol_index - L4 protocol index                                 */
/*                                                                            */
/*   xi_l4_protocol_value - L4 protocol value                                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_user_ip_l4_protocol ( uint8_t xi_l4_protocol_index ,
                                                           uint8_t xi_l4_protocol_value ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_user_ip_l4_protocol                                     */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get user IP L4 protocol                                      */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets a user-defined L4 Protocol.                           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_l4_protocol_index - L4 protocol index                                 */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_l4_protocol_value - L4 protocol value                                 */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_user_ip_l4_protocol ( uint8_t xi_l4_protocol_index ,
                                                           uint8_t * const xo_l4_protocol_value ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_ppp_code                                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set PPP code                                                 */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets PPP Protocol Code to indicate L3 is IP.               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ppp_code_index - PPP code index                                       */
/*                                                                            */
/*   xi_ppp_code - PPP code                                                   */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_ppp_code ( uint8_t xi_ppp_code_index ,
                                                uint16_t xi_ppp_code ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_ppp_code                                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get PPP code                                                 */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets PPP Protocol Code.                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_ppp_code_index - PPP code index                                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_ppp_code - PPP code                                                   */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_ppp_code ( uint8_t xi_ppp_code_index ,
                                                uint16_t * const xo_ppp_code ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_vid_filter                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set VID filter                                               */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets a VID filter. There are up to 12 VID filters. The     */
/*   filter has to be enabled by Enable VID filter API in order to take       */
/*   effect.                                                                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/*   xi_vid - VID                                                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_vid_filter ( uint8_t xi_filter_index ,
                                                  uint16_t xi_vid ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_vid_filter                                              */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get VID filter                                               */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets a VID filter. There are up to 12 VID filters.         */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_vid - VID                                                             */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_vid_filter ( uint8_t xi_filter_index ,
                                                  uint16_t * const xo_vid ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_enable_vid_filter                                           */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Enable VID filter                                            */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function enables/disables a VID filter. There are up to 12 VID      */
/*   filters. Before enabling a filter, it should be configured by Set VID    */
/*   filter API.                                                              */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/*   xi_enable - Enable                                                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_enable_vid_filter ( uint8_t xi_filter_index ,
                                                     int32_t xi_enable ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_vid_filter_enable_status                                */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get VID filter enable status                                 */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the enable status of a VID filter. There are up to 12 */
/*   VID filters.                                                             */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_enable - Enable                                                       */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_vid_filter_enable_status ( uint8_t xi_filter_index ,
                                                                int32_t * const xo_enable ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_ip_filter                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set IP filter                                                */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets an IP filter. There are up to 4 IP filters. The       */
/*   filter has to be enabled by Enable IP filter API in order to take        */
/*   effect.                                                                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/*   xi_ip_address - IP address                                               */
/*                                                                            */
/*   xi_ip_address_mask - IP address mask                                     */
/*                                                                            */
/*   xi_selection - selection (SIP/DIP)                                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_ip_filter ( uint8_t xi_filter_index ,
                                                 uint32_t xi_ip_address ,
                                                 uint32_t xi_ip_address_mask ,
                                                 DRV_IH_IP_FILTER_SELECTION xi_selection ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_ip_filter                                               */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get IP filter                                                */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets an IP filter. There are up to 4 IP filters.           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_ip_address - IP address                                               */
/*                                                                            */
/*   xo_ip_address_mask - IP address mask                                     */
/*                                                                            */
/*   xo_selection - selection (SIP/DIP)                                       */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_ip_filter ( uint8_t xi_filter_index ,
                                                 uint32_t * const xo_ip_address ,
                                                 uint32_t * const xo_ip_address_mask ,
                                                 DRV_IH_IP_FILTER_SELECTION * const xo_selection ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_enable_ip_filter                                            */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Enable IP filter                                             */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function enables/disables an IP filter. There are up to 4 IP        */
/*   filters. Before enabling a filter, it should be configured by Set IP     */
/*   filter API.                                                              */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/*   xi_enable - Enable                                                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_enable_ip_filter ( uint8_t xi_filter_index ,
                                                    int32_t xi_enable ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_ip_filter_enable_status                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get IP filter enable status                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the enable status of an IP filter. There are up to 4  */
/*   IP filters.                                                              */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_filter_index - Filter index                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_enable - Enable                                                       */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_ip_filter_enable_status ( uint8_t xi_filter_index ,
                                                               int32_t * const xo_enable ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_dscp_to_tci_table_entry                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set DSCP to TCI table entry                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets an entry in a DSCP to TCI table. There are 2 such     */
/*   tables. Each class is configured with one of these tables. The table has */
/*   to be enabled by Enable DSCP to TCI table API in order to take effect.   */
/*   If a class is configured with a table which is not enabled, the TCI will */
/*   be 0.                                                                    */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_table_index - Table index                                             */
/*                                                                            */
/*   xi_dscp - DSCP                                                           */
/*                                                                            */
/*   xi_tci - TCI                                                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_dscp_to_tci_table_entry ( uint8_t xi_table_index ,
                                                               uint8_t xi_dscp ,
                                                               uint8_t xi_tci ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_dscp_to_tci_table_entry                                 */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get DSCP to TCI table entry                                  */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets an entry in a DSCP to TCI table. There are 2 such     */
/*   tables.                                                                  */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_table_index - Table index                                             */
/*                                                                            */
/*   xi_dscp - DSCP                                                           */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_tci - TCI                                                             */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_dscp_to_tci_table_entry ( uint8_t xi_table_index ,
                                                               uint8_t xi_dscp ,
                                                               uint8_t * const xo_tci ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_set_default_tci                                             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Set default TCI                                              */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function sets default TCI, per DSCP to TCI table. The default is    */
/*   used in case of non-IP untagged packet. The default TCI will take effect */
/*   only after enabling the DSCP to TCI table, by Enable DSCP to TCI table   */
/*   API. If a class is configured with a table which is not enabled, the TCI */
/*   will be 0.                                                               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_table_index - Table index                                             */
/*                                                                            */
/*   xi_default_tci - Default TCI                                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_set_default_tci ( uint8_t xi_table_index ,
                                                   uint8_t xi_default_tci ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_default_tci                                             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get default TCI                                              */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the default TCI, per DSCP to TCI table.               */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_table_index - Table index                                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_default_tci - Default TCI                                             */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_default_tci ( uint8_t xi_table_index ,
                                                   uint8_t * const xo_default_tci ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_enable_dscp_to_tci_table                                    */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Enable DSCP to TCI table                                     */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function enables/disables a DSCP to TCI table. There are 2 such     */
/*   tables. Each class is configured with one of these tables. Before        */
/*   enabling a table, it should be configured by Set DSCP to TCI table API   */
/*   and Set default TCI API.                                                 */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_table_index - Table index                                             */
/*                                                                            */
/*   xi_enable - Enable                                                       */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_enable_dscp_to_tci_table ( uint8_t xi_table_index ,
                                                            int32_t xi_enable ) ;


/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_get_dscp_to_tci_table_enable_status                         */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Get DSCP to TCI table enable status                          */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function gets the enable status of a DSCP to TCI table. There are 2 */
/*   such tables.                                                             */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_table_index - Table index                                             */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   xo_enable - Enable                                                       */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                        */
/*     DRV_IH_NO_ERROR - No error                                       */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                       */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_get_dscp_to_tci_table_enable_status ( uint8_t xi_table_index ,
                                                                       int32_t * const xo_enable ) ;
/******************************************************************************/
/*                                                                            */
/* Name:                                                                      */
/*                                                                            */
/*   fi_bl_drv_ih_configure_parser_core_cfg_eng_3rd_tag_detection             */
/*                                                                            */
/* Title:                                                                     */
/*                                                                            */
/*   IH Driver - Configure Parser ENG register                                */
/*                                                                            */
/* Abstract:                                                                  */
/*                                                                            */
/*   This function configures the ENG register (bits 8-13 are for triple      */
/*   tag detection)                                                           */
/*                                                                            */
/* Input:                                                                     */
/*                                                                            */
/*   xi_enable - ENG register bit value                                       */
/*                                                                            */
/*   xi_tpid_index - ENG register internal bit index                          */
/*                                                                            */
/* Output:                                                                    */
/*                                                                            */
/*   DRV_IH_ERROR - Return code                                               */
/*     DRV_IH_NO_ERROR - No error                                             */
/*     DRV_IH_ERROR_INVALID_INDEX - Invalid index                             */
/*                                                                            */
/******************************************************************************/
DRV_IH_ERROR fi_bl_drv_ih_configure_parser_core_cfg_eng_3rd_tag_detection ( uint32_t xi_enable,
		uint8_t xi_tpid_index );


int32_t fi_is_class_configured ( uint8_t xi_class_index );
int32_t fi_is_classifier_configured ( uint8_t xi_classifier_index );

#ifdef __cplusplus
}
#endif

#endif

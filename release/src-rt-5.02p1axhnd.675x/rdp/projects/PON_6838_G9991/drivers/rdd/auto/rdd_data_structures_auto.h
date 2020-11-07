/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard

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



/* This is an automated file. Do not edit its contents. */


#ifndef _RDD_DATA_STRUCTURES_AUTO_H
#define _RDD_DATA_STRUCTURES_AUTO_H

/* PRIVATE_A */
#define RDD_IH_BUFFER_RESERVED_FW_ONLY_NUMBER	64

typedef struct
{
	uint32_t	reserved_fw_only[RDD_IH_BUFFER_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IH_BUFFER_DTS;

#if defined OREN

#define RDD_INGRESS_HANDLER_BUFFER_SIZE     32
typedef struct
{
	RDD_IH_BUFFER_DTS	entry[ RDD_INGRESS_HANDLER_BUFFER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INGRESS_HANDLER_BUFFER_DTS;

#define RDD_INGRESS_HANDLER_BUFFER_PTR()	( RDD_INGRESS_HANDLER_BUFFER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + INGRESS_HANDLER_BUFFER_ADDRESS )

#endif

typedef struct
{
	uint32_t	reserved0                     	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1                     	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	next_packet_descriptor_pointer	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PACKET_DESCRIPTOR_DTS;

#define RDD_PACKET_DESCRIPTOR_NEXT_PACKET_DESCRIPTOR_POINTER_READ(r, p)                 MREAD_16((uint8_t *)p + 6, r)
#define RDD_PACKET_DESCRIPTOR_NEXT_PACKET_DESCRIPTOR_POINTER_WRITE(v, p)                MWRITE_16((uint8_t *)p + 6, v)

typedef struct
{
	uint32_t	crc_calc          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	gem_port          	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	source_bridge_port	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	packet_length     	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	absolute_normal   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	last_indication   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pti               	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	_1588             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	add_indication    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	header_number     	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	payload_offset    	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	target_memory     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	packet_location   	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = packet_location, size = 14 bits
	uint32_t	buffer_number     	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs_address_index 	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BBH_TX_DESCRIPTOR_DTS;

#define RDD_BBH_TX_DESCRIPTOR_CRC_CALC_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_BBH_TX_DESCRIPTOR_CRC_CALC_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_BBH_TX_DESCRIPTOR_GEM_PORT_READ(r, p)                           FIELD_MREAD_16((uint8_t *)p, 3, 12, r)
#define RDD_BBH_TX_DESCRIPTOR_GEM_PORT_WRITE(v, p)                          FIELD_MWRITE_16((uint8_t *)p, 3, 12, v)
#define RDD_BBH_TX_DESCRIPTOR_SOURCE_BRIDGE_PORT_READ(r, p)                 FIELD_MREAD_32((uint8_t *)p + 0, 14, 5, r)
#define RDD_BBH_TX_DESCRIPTOR_SOURCE_BRIDGE_PORT_WRITE(v, p)                FIELD_MWRITE_32((uint8_t *)p + 0, 14, 5, v)
#define RDD_BBH_TX_DESCRIPTOR_PACKET_LENGTH_READ(r, p)                      FIELD_MREAD_16((uint8_t *)p + 2, 0, 14, r)
#define RDD_BBH_TX_DESCRIPTOR_PACKET_LENGTH_WRITE(v, p)                     FIELD_MWRITE_16((uint8_t *)p + 2, 0, 14, v)
#define RDD_BBH_TX_DESCRIPTOR_ABSOLUTE_NORMAL_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r)
#define RDD_BBH_TX_DESCRIPTOR_ABSOLUTE_NORMAL_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 4, 7, 1, v)
#define RDD_BBH_TX_DESCRIPTOR_LAST_INDICATION_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r)
#define RDD_BBH_TX_DESCRIPTOR_LAST_INDICATION_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 4, 6, 1, v)
#define RDD_BBH_TX_DESCRIPTOR_PTI_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 4, 4, 2, r)
#define RDD_BBH_TX_DESCRIPTOR_PTI_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 4, 4, 2, v)
#define RDD_BBH_TX_DESCRIPTOR__1588_READ(r, p)                              FIELD_MREAD_8((uint8_t *)p + 4, 3, 1, r)
#define RDD_BBH_TX_DESCRIPTOR__1588_WRITE(v, p)                             FIELD_MWRITE_8((uint8_t *)p + 4, 3, 1, v)
#define RDD_BBH_TX_DESCRIPTOR_ADD_INDICATION_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 4, 2, 1, r)
#define RDD_BBH_TX_DESCRIPTOR_ADD_INDICATION_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 4, 2, 1, v)
#define RDD_BBH_TX_DESCRIPTOR_HEADER_NUMBER_READ(r, p)                      FIELD_MREAD_16((uint8_t *)p + 4, 7, 3, r)
#define RDD_BBH_TX_DESCRIPTOR_HEADER_NUMBER_WRITE(v, p)                     FIELD_MWRITE_16((uint8_t *)p + 4, 7, 3, v)
#define RDD_BBH_TX_DESCRIPTOR_PAYLOAD_OFFSET_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 5, 0, 7, r)
#define RDD_BBH_TX_DESCRIPTOR_PAYLOAD_OFFSET_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 5, 0, 7, v)
#define RDD_BBH_TX_DESCRIPTOR_TARGET_MEMORY_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 6, 7, 1, r)
#define RDD_BBH_TX_DESCRIPTOR_TARGET_MEMORY_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 6, 7, 1, v)
#define RDD_BBH_TX_DESCRIPTOR_PACKET_LOCATION_READ(r, p)                    FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r)
#define RDD_BBH_TX_DESCRIPTOR_PACKET_LOCATION_WRITE(v, p)                   FIELD_MWRITE_16((uint8_t *)p + 6, 0, 14, v)
#define RDD_BBH_TX_DESCRIPTOR_BUFFER_NUMBER_READ(r, p)                      FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r)
#define RDD_BBH_TX_DESCRIPTOR_BUFFER_NUMBER_WRITE(v, p)                     FIELD_MWRITE_16((uint8_t *)p + 6, 0, 14, v)
#define RDD_BBH_TX_DESCRIPTOR_ABS_ADDRESS_INDEX_READ(r, p)                  FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r)
#define RDD_BBH_TX_DESCRIPTOR_ABS_ADDRESS_INDEX_WRITE(v, p)                 FIELD_MWRITE_16((uint8_t *)p + 6, 0, 14, v)
#define RDD_SIXTEEN_BYTES_RESERVED_FW_ONLY_NUMBER	4

typedef struct
{
	uint32_t	reserved_fw_only[RDD_SIXTEEN_BYTES_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SIXTEEN_BYTES_DTS;

#if defined OREN

#define RDD_SERVICE_QUEUES_DDR_CACHE_FIFO_SIZE     384
typedef struct
{
	RDD_SIXTEEN_BYTES_DTS	entry[ RDD_SERVICE_QUEUES_DDR_CACHE_FIFO_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SERVICE_QUEUES_DDR_CACHE_FIFO_DTS;

#define RDD_SERVICE_QUEUES_DDR_CACHE_FIFO_PTR()	( RDD_SERVICE_QUEUES_DDR_CACHE_FIFO_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + SERVICE_QUEUES_DDR_CACHE_FIFO_ADDRESS )

#endif
#if defined OREN

typedef struct
{
	uint32_t	vlan_index_table_ptr	:13	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	opbit_remark_mode   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ipbit_remark_mode   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wifi_ssid           	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dei_remark_enable   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dei_value           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	service_queue_mode  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	forward_mode        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_port         	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	qos_mapping_mode    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	traffic_class       	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	drop                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cpu                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	policer_mode        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	policer_id          	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_shaping_mode   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cpu_mirroring       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ic_ip_flow          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	service_queue       	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	subnet_id           	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	outer_pbit          	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	inner_pbit          	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dscp_remarking_mode 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dscp                	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ecn                 	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS;

#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_VLAN_INDEX_TABLE_PTR_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p, 3, 13, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_VLAN_INDEX_TABLE_PTR_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p, 3, 13, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OPBIT_REMARK_MODE_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 1, 2, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OPBIT_REMARK_MODE_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 1, 2, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IPBIT_REMARK_MODE_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 1, 1, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IPBIT_REMARK_MODE_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 1, 1, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WIFI_SSID_READ(r, p)                            FIELD_MREAD_32((uint8_t *)p + 0, 13, 4, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WIFI_SSID_WRITE(v, p)                           FIELD_MWRITE_32((uint8_t *)p + 0, 13, 4, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_REMARK_ENABLE_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 2, 4, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_REMARK_ENABLE_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 2, 4, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_VALUE_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 2, 3, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_VALUE_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 2, 3, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SERVICE_QUEUE_MODE_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p + 2, 2, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SERVICE_QUEUE_MODE_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p + 2, 2, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_FORWARD_MODE_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 2, 0, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_FORWARD_MODE_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 2, 0, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_EGRESS_PORT_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 3, 4, 4, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_EGRESS_PORT_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 3, 4, 4, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_MAPPING_MODE_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 3, 3, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_MAPPING_MODE_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 3, 3, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_TRAFFIC_CLASS_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 3, 0, 3, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_TRAFFIC_CLASS_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 3, 0, 3, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DROP_READ(r, p)                                 FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DROP_WRITE(v, p)                                FIELD_MWRITE_8((uint8_t *)p + 4, 7, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CPU_READ(r, p)                                  FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CPU_WRITE(v, p)                                 FIELD_MWRITE_8((uint8_t *)p + 4, 6, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_MODE_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 4, 5, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_MODE_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 4, 5, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_ID_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 4, 1, 4, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_ID_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 4, 1, 4, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_RATE_SHAPING_MODE_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 4, 0, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_RATE_SHAPING_MODE_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 4, 0, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CPU_MIRRORING_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 5, 7, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CPU_MIRRORING_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 5, 7, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IC_IP_FLOW_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 5, 6, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IC_IP_FLOW_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 5, 6, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SERVICE_QUEUE_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 5, 1, 5, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SERVICE_QUEUE_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 5, 1, 5, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SUBNET_ID_READ(r, p)                            FIELD_MREAD_32((uint8_t *)p + 4, 15, 2, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SUBNET_ID_WRITE(v, p)                           FIELD_MWRITE_32((uint8_t *)p + 4, 15, 2, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OUTER_PBIT_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 6, 4, 3, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OUTER_PBIT_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 6, 4, 3, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_INNER_PBIT_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 6, 1, 3, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_INNER_PBIT_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 6, 1, 3, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_REMARKING_MODE_READ(r, p)                  FIELD_MREAD_8((uint8_t *)p + 6, 0, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_REMARKING_MODE_WRITE(v, p)                 FIELD_MWRITE_8((uint8_t *)p + 6, 0, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_READ(r, p)                                 FIELD_MREAD_8((uint8_t *)p + 7, 2, 6, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_WRITE(v, p)                                FIELD_MWRITE_8((uint8_t *)p + 7, 2, 6, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_ECN_READ(r, p)                                  FIELD_MREAD_8((uint8_t *)p + 7, 0, 2, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_ECN_WRITE(v, p)                                 FIELD_MWRITE_8((uint8_t *)p + 7, 0, 2, v)
#endif
#if defined OREN

#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE     256
typedef struct
{
	RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS	entry[ RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS;

#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_PTR()	( RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	l4_protocol         	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	error               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ptag                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	number_of_vlans     	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	broadcast           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	multicast           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	l3_protocol         	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	l2_protocol         	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	l4_protocol_mask    	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	error_mask          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ptag_mask           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	number_of_vlans_mask	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	broadcast_mask      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	multicast_mask      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	l3_protocol_mask    	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	l2_protocol_mask    	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS;

#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p, 4, 4, r)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p, 4, 4, v)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_ERROR_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p, 3, 1, r)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_ERROR_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p, 3, 1, v)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_PTAG_READ(r, p)                                 FIELD_MREAD_8((uint8_t *)p, 2, 1, r)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_PTAG_WRITE(v, p)                                FIELD_MWRITE_8((uint8_t *)p, 2, 1, v)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_NUMBER_OF_VLANS_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p, 0, 2, r)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_NUMBER_OF_VLANS_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p, 0, 2, v)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_BROADCAST_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 1, 7, 1, r)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_BROADCAST_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 1, 7, 1, v)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_MULTICAST_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 1, 6, 1, r)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_MULTICAST_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 1, 6, 1, v)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L3_PROTOCOL_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 1, 4, 2, r)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L3_PROTOCOL_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 1, 4, 2, v)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 1, 0, 4, r)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 1, 0, 4, v)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 2, 4, 4, r)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 2, 4, 4, v)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_ERROR_MASK_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 2, 3, 1, r)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_ERROR_MASK_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 2, 3, 1, v)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_PTAG_MASK_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 2, 2, 1, r)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_PTAG_MASK_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 2, 2, 1, v)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_NUMBER_OF_VLANS_MASK_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 2, 0, 2, r)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_NUMBER_OF_VLANS_MASK_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 2, 0, 2, v)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_BROADCAST_MASK_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 3, 7, 1, r)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_BROADCAST_MASK_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 3, 7, 1, v)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_MULTICAST_MASK_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 3, 6, 1, r)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_MULTICAST_MASK_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 3, 6, 1, v)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L3_PROTOCOL_MASK_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 3, 4, 2, r)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L3_PROTOCOL_MASK_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 3, 4, 2, v)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 3, 0, 4, r)
#define RDD_INGRESS_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 3, 0, 4, v)
#if defined OREN

#define RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_SIZE     10
#define RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_SIZE2    32
typedef struct
{
	RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS	entry[ RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_SIZE ][ RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_DTS;

#define RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_PTR()	( RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_FILTERS_LOOKUP_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	head_ptr              	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tail_ptr              	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ingress_packet_counter	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_packet_counter 	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	packet_threshold      	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	profile_ptr           	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rs_status_offset      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rs_group_status_offset	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	status_offset         	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0             	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_DTS;

#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_HEAD_PTR_READ(r, p)                               MREAD_16((uint8_t *)p, r)
#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_HEAD_PTR_WRITE(v, p)                              MWRITE_16((uint8_t *)p, v)
#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_TAIL_PTR_READ(r, p)                               MREAD_16((uint8_t *)p + 2, r)
#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_TAIL_PTR_WRITE(v, p)                              MWRITE_16((uint8_t *)p + 2, v)
#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_INGRESS_PACKET_COUNTER_READ(r, p)                 MREAD_16((uint8_t *)p + 4, r)
#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_INGRESS_PACKET_COUNTER_WRITE(v, p)                MWRITE_16((uint8_t *)p + 4, v)
#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_EGRESS_PACKET_COUNTER_READ(r, p)                  MREAD_16((uint8_t *)p + 6, r)
#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_EGRESS_PACKET_COUNTER_WRITE(v, p)                 MWRITE_16((uint8_t *)p + 6, v)
#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_PACKET_THRESHOLD_READ(r, p)                       MREAD_16((uint8_t *)p + 8, r)
#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_PACKET_THRESHOLD_WRITE(v, p)                      MWRITE_16((uint8_t *)p + 8, v)
#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_PROFILE_PTR_READ(r, p)                            MREAD_16((uint8_t *)p + 10, r)
#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_PROFILE_PTR_WRITE(v, p)                           MWRITE_16((uint8_t *)p + 10, v)
#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_RS_STATUS_OFFSET_READ(r, p)                       MREAD_8((uint8_t *)p + 12, r)
#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_RS_STATUS_OFFSET_WRITE(v, p)                      MWRITE_8((uint8_t *)p + 12, v)
#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_RS_GROUP_STATUS_OFFSET_READ(r, p)                 MREAD_8((uint8_t *)p + 13, r)
#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_RS_GROUP_STATUS_OFFSET_WRITE(v, p)                MWRITE_8((uint8_t *)p + 13, v)
#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_STATUS_OFFSET_READ(r, p)                          MREAD_8((uint8_t *)p + 14, r)
#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_STATUS_OFFSET_WRITE(v, p)                         MWRITE_8((uint8_t *)p + 14, v)
#if defined OREN

#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_TABLE_SIZE     128
typedef struct
{
	RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_DTS	entry[ RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_TABLE_DTS;

#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_TABLE_PTR()	( RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + ETH_TX_RS_QUEUE_DESCRIPTOR_TABLE_ADDRESS )

#endif

typedef struct
{
	uint8_t	pbits     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PBITS_TO_PBITS_ENTRY_DTS;

#define RDD_PBITS_TO_PBITS_ENTRY_PBITS_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_PBITS_TO_PBITS_ENTRY_PBITS_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#if defined OREN

#define RDD_DS_PBITS_TO_PBITS_TABLE_SIZE     32
#define RDD_DS_PBITS_TO_PBITS_TABLE_SIZE2    8
typedef struct
{
	RDD_PBITS_TO_PBITS_ENTRY_DTS	entry[ RDD_DS_PBITS_TO_PBITS_TABLE_SIZE ][ RDD_DS_PBITS_TO_PBITS_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_PBITS_TO_PBITS_TABLE_DTS;

#define RDD_DS_PBITS_TO_PBITS_TABLE_PTR()	( RDD_DS_PBITS_TO_PBITS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PBITS_TO_PBITS_TABLE_ADDRESS )

#endif

typedef struct
{
	uint16_t	reserved             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	cpu_reason           	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	ingress_classify_mode	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	ingress_flow         	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_WAN_FLOW_ENTRY_DTS;

#define RDD_DS_WAN_FLOW_ENTRY_CPU_REASON_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p, 1, 6, r)
#define RDD_DS_WAN_FLOW_ENTRY_CPU_REASON_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p, 1, 6, v)
#define RDD_DS_WAN_FLOW_ENTRY_INGRESS_CLASSIFY_MODE_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 0, 1, r)
#define RDD_DS_WAN_FLOW_ENTRY_INGRESS_CLASSIFY_MODE_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 0, 1, v)
#define RDD_DS_WAN_FLOW_ENTRY_INGRESS_FLOW_READ(r, p)                          MREAD_8((uint8_t *)p + 1, r)
#define RDD_DS_WAN_FLOW_ENTRY_INGRESS_FLOW_WRITE(v, p)                         MWRITE_8((uint8_t *)p + 1, v)
#if defined OREN

#define RDD_DS_WAN_FLOW_TABLE_SIZE     256
typedef struct
{
	RDD_DS_WAN_FLOW_ENTRY_DTS	entry[ RDD_DS_WAN_FLOW_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_WAN_FLOW_TABLE_DTS;

#define RDD_DS_WAN_FLOW_TABLE_PTR()	( RDD_DS_WAN_FLOW_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_WAN_FLOW_TABLE_ADDRESS )

#endif

typedef struct
{
	uint16_t	counter   	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_COUNTER_ENTRY_DTS;

#define RDD_IPTV_COUNTER_ENTRY_COUNTER_READ(r, p)                 MREAD_16((uint8_t *)p, r)
#define RDD_IPTV_COUNTER_ENTRY_COUNTER_WRITE(v, p)                MWRITE_16((uint8_t *)p, v)
#if defined OREN

#define RDD_IPTV_COUNTERS_TABLE_SIZE     288
typedef struct
{
	RDD_IPTV_COUNTER_ENTRY_DTS	entry[ RDD_IPTV_COUNTERS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_COUNTERS_TABLE_DTS;

#define RDD_IPTV_COUNTERS_TABLE_PTR()	( RDD_IPTV_COUNTERS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_COUNTERS_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	union_field1                    	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = union_field1, size = 32 bits
	uint32_t	register_r9                     	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	emac_descriptor_ptr             	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0                       	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	union_field2                    	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = union_field2, size = 32 bits
	uint32_t	register_r11                    	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth_tx_queues_pointers_table_ptr	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbh_destination                 	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_DTS;

#define RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_UNION_FIELD1_READ(r, p)                                     MREAD_32((uint8_t *)p, r)
#define RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_UNION_FIELD1_WRITE(v, p)                                    MWRITE_32((uint8_t *)p, v)
#define RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_REGISTER_R9_READ(r, p)                                      MREAD_32((uint8_t *)p, r)
#define RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_REGISTER_R9_WRITE(v, p)                                     MWRITE_32((uint8_t *)p, v)
#define RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_EMAC_DESCRIPTOR_PTR_READ(r, p)                              MREAD_16((uint8_t *)p, r)
#define RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_EMAC_DESCRIPTOR_PTR_WRITE(v, p)                             MWRITE_16((uint8_t *)p, v)
#define RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_UNION_FIELD2_READ(r, p)                                     MREAD_32((uint8_t *)p + 4, r)
#define RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_UNION_FIELD2_WRITE(v, p)                                    MWRITE_32((uint8_t *)p + 4, v)
#define RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_REGISTER_R11_READ(r, p)                                     MREAD_32((uint8_t *)p + 4, r)
#define RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_REGISTER_R11_WRITE(v, p)                                    MWRITE_32((uint8_t *)p + 4, v)
#define RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_ETH_TX_QUEUES_POINTERS_TABLE_PTR_READ(r, p)                 MREAD_16((uint8_t *)p + 4, r)
#define RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_ETH_TX_QUEUES_POINTERS_TABLE_PTR_WRITE(v, p)                MWRITE_16((uint8_t *)p + 4, v)
#define RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_BBH_DESTINATION_READ(r, p)                                  MREAD_16((uint8_t *)p + 6, r)
#define RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_BBH_DESTINATION_WRITE(v, p)                                 MWRITE_16((uint8_t *)p + 6, v)
#if defined OREN

#define RDD_ETH_TX_LOCAL_REGISTERS_SIZE     8
typedef struct
{
	RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_DTS	entry[ RDD_ETH_TX_LOCAL_REGISTERS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_LOCAL_REGISTERS_DTS;

#define RDD_ETH_TX_LOCAL_REGISTERS_PTR()	( RDD_ETH_TX_LOCAL_REGISTERS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + ETH_TX_LOCAL_REGISTERS_ADDRESS )

#endif

typedef struct
{
	uint32_t	us_flow_control_mode    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0               	:15	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	max_low_threshold       	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	min_high_threshold      	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	max_high_threshold      	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	low_large_interval_flag 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1               	:15	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	low_drop_constant       	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	high_large_interval_flag	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2               	:15	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	high_drop_constant      	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_QUEUE_PROFILE_DTS;

#define RDD_QUEUE_PROFILE_US_FLOW_CONTROL_MODE_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_QUEUE_PROFILE_US_FLOW_CONTROL_MODE_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_QUEUE_PROFILE_MAX_LOW_THRESHOLD_READ(r, p)                        MREAD_16((uint8_t *)p + 2, r)
#define RDD_QUEUE_PROFILE_MAX_LOW_THRESHOLD_WRITE(v, p)                       MWRITE_16((uint8_t *)p + 2, v)
#define RDD_QUEUE_PROFILE_MIN_HIGH_THRESHOLD_READ(r, p)                       MREAD_16((uint8_t *)p + 4, r)
#define RDD_QUEUE_PROFILE_MIN_HIGH_THRESHOLD_WRITE(v, p)                      MWRITE_16((uint8_t *)p + 4, v)
#define RDD_QUEUE_PROFILE_MAX_HIGH_THRESHOLD_READ(r, p)                       MREAD_16((uint8_t *)p + 6, r)
#define RDD_QUEUE_PROFILE_MAX_HIGH_THRESHOLD_WRITE(v, p)                      MWRITE_16((uint8_t *)p + 6, v)
#define RDD_QUEUE_PROFILE_LOW_LARGE_INTERVAL_FLAG_READ(r, p)                  FIELD_MREAD_8((uint8_t *)p + 8, 7, 1, r)
#define RDD_QUEUE_PROFILE_LOW_LARGE_INTERVAL_FLAG_WRITE(v, p)                 FIELD_MWRITE_8((uint8_t *)p + 8, 7, 1, v)
#define RDD_QUEUE_PROFILE_LOW_DROP_CONSTANT_READ(r, p)                        MREAD_16((uint8_t *)p + 10, r)
#define RDD_QUEUE_PROFILE_LOW_DROP_CONSTANT_WRITE(v, p)                       MWRITE_16((uint8_t *)p + 10, v)
#define RDD_QUEUE_PROFILE_HIGH_LARGE_INTERVAL_FLAG_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 12, 7, 1, r)
#define RDD_QUEUE_PROFILE_HIGH_LARGE_INTERVAL_FLAG_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 12, 7, 1, v)
#define RDD_QUEUE_PROFILE_HIGH_DROP_CONSTANT_READ(r, p)                       MREAD_16((uint8_t *)p + 14, r)
#define RDD_QUEUE_PROFILE_HIGH_DROP_CONSTANT_WRITE(v, p)                      MWRITE_16((uint8_t *)p + 14, v)
#if defined OREN

#define RDD_DS_QUEUE_PROFILE_TABLE_SIZE     8
typedef struct
{
	RDD_QUEUE_PROFILE_DTS	entry[ RDD_DS_QUEUE_PROFILE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_QUEUE_PROFILE_TABLE_DTS;

#define RDD_DS_QUEUE_PROFILE_TABLE_PTR()	( RDD_DS_QUEUE_PROFILE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_QUEUE_PROFILE_TABLE_ADDRESS )

#endif

typedef struct
{
	uint16_t	outer_pbits	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	inner_pbits	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PBITS_PARAMETER_ENTRY_DTS;

#define RDD_PBITS_PARAMETER_ENTRY_OUTER_PBITS_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_PBITS_PARAMETER_ENTRY_OUTER_PBITS_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#define RDD_PBITS_PARAMETER_ENTRY_INNER_PBITS_READ(r, p)                 MREAD_8((uint8_t *)p + 1, r)
#define RDD_PBITS_PARAMETER_ENTRY_INNER_PBITS_WRITE(v, p)                MWRITE_8((uint8_t *)p + 1, v)
#if defined OREN

#define RDD_DS_PBITS_PARAMETER_TABLE_SIZE     128
typedef struct
{
	RDD_PBITS_PARAMETER_ENTRY_DTS	entry[ RDD_DS_PBITS_PARAMETER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_PBITS_PARAMETER_TABLE_DTS;

#define RDD_DS_PBITS_PARAMETER_TABLE_PTR()	( RDD_DS_PBITS_PARAMETER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PBITS_PARAMETER_TABLE_ADDRESS )

#endif

typedef struct
{
	uint16_t	wlan_mcast_index	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_SSID_EXTENSION_ENTRY_DTS;

#define RDD_IPTV_SSID_EXTENSION_ENTRY_WLAN_MCAST_INDEX_READ(r, p)                 MREAD_16((uint8_t *)p, r)
#define RDD_IPTV_SSID_EXTENSION_ENTRY_WLAN_MCAST_INDEX_WRITE(v, p)                MWRITE_16((uint8_t *)p, v)
#if defined OREN

#define RDD_IPTV_SSID_EXTENSION_TABLE_SIZE     256
typedef struct
{
	RDD_IPTV_SSID_EXTENSION_ENTRY_DTS	entry[ RDD_IPTV_SSID_EXTENSION_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_SSID_EXTENSION_TABLE_DTS;

#define RDD_IPTV_SSID_EXTENSION_TABLE_PTR()	( RDD_IPTV_SSID_EXTENSION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_SSID_EXTENSION_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	outer_tpid_overwrite_enable	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	inner_tpid_overwrite_enable	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	outer_tpid_id              	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	inner_tpid_id              	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	outer_vid                  	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	inner_vid                  	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_VLAN_PARAMETER_ENTRY_DTS;

#define RDD_VLAN_PARAMETER_ENTRY_OUTER_TPID_OVERWRITE_ENABLE_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_VLAN_PARAMETER_ENTRY_OUTER_TPID_OVERWRITE_ENABLE_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_VLAN_PARAMETER_ENTRY_INNER_TPID_OVERWRITE_ENABLE_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 6, 1, r)
#define RDD_VLAN_PARAMETER_ENTRY_INNER_TPID_OVERWRITE_ENABLE_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 6, 1, v)
#define RDD_VLAN_PARAMETER_ENTRY_OUTER_TPID_ID_READ(r, p)                               FIELD_MREAD_8((uint8_t *)p, 3, 3, r)
#define RDD_VLAN_PARAMETER_ENTRY_OUTER_TPID_ID_WRITE(v, p)                              FIELD_MWRITE_8((uint8_t *)p, 3, 3, v)
#define RDD_VLAN_PARAMETER_ENTRY_INNER_TPID_ID_READ(r, p)                               FIELD_MREAD_8((uint8_t *)p, 0, 3, r)
#define RDD_VLAN_PARAMETER_ENTRY_INNER_TPID_ID_WRITE(v, p)                              FIELD_MWRITE_8((uint8_t *)p, 0, 3, v)
#define RDD_VLAN_PARAMETER_ENTRY_OUTER_VID_READ(r, p)                                   FIELD_MREAD_32((uint8_t *)p + 0, 12, 12, r)
#define RDD_VLAN_PARAMETER_ENTRY_OUTER_VID_WRITE(v, p)                                  FIELD_MWRITE_32((uint8_t *)p + 0, 12, 12, v)
#define RDD_VLAN_PARAMETER_ENTRY_INNER_VID_READ(r, p)                                   FIELD_MREAD_16((uint8_t *)p + 2, 0, 12, r)
#define RDD_VLAN_PARAMETER_ENTRY_INNER_VID_WRITE(v, p)                                  FIELD_MWRITE_16((uint8_t *)p + 2, 0, 12, v)
#if defined OREN

#define RDD_DS_VLAN_PARAMETER_TABLE_SIZE     128
typedef struct
{
	RDD_VLAN_PARAMETER_ENTRY_DTS	entry[ RDD_DS_VLAN_PARAMETER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_VLAN_PARAMETER_TABLE_DTS;

#define RDD_DS_VLAN_PARAMETER_TABLE_PTR()	( RDD_DS_VLAN_PARAMETER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_VLAN_PARAMETER_TABLE_ADDRESS )

#endif

typedef struct
{
	uint8_t	cpu_meter 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_REASON_TO_METER_ENTRY_DTS;

#define RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_CPU_REASON_TO_METER_ENTRY_CPU_METER_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#if defined OREN

#define RDD_DS_CPU_REASON_TO_METER_TABLE_SIZE     64
typedef struct
{
	RDD_CPU_REASON_TO_METER_ENTRY_DTS	entry[ RDD_DS_CPU_REASON_TO_METER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_CPU_REASON_TO_METER_TABLE_DTS;

#define RDD_DS_CPU_REASON_TO_METER_TABLE_PTR()	( RDD_DS_CPU_REASON_TO_METER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_CPU_REASON_TO_METER_TABLE_ADDRESS )

#endif

typedef struct
{
	uint8_t	context   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS;

#define RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CONTEXT_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CONTEXT_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#if defined OREN

#define RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE     288
typedef struct
{
	RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS	entry[ RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS;

#define RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_PTR()	( RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	period        	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	counter_reload	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	firmware_ptr  	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved      	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_TIMER_TASK_DESCRIPTOR_ENTRY_DTS;

#define RDD_TIMER_TASK_DESCRIPTOR_ENTRY_PERIOD_READ(r, p)                         MREAD_8((uint8_t *)p, r)
#define RDD_TIMER_TASK_DESCRIPTOR_ENTRY_PERIOD_WRITE(v, p)                        MWRITE_8((uint8_t *)p, v)
#define RDD_TIMER_TASK_DESCRIPTOR_ENTRY_COUNTER_RELOAD_READ(r, p)                 MREAD_8((uint8_t *)p + 1, r)
#define RDD_TIMER_TASK_DESCRIPTOR_ENTRY_COUNTER_RELOAD_WRITE(v, p)                MWRITE_8((uint8_t *)p + 1, v)
#define RDD_TIMER_TASK_DESCRIPTOR_ENTRY_FIRMWARE_PTR_READ(r, p)                   MREAD_16((uint8_t *)p + 2, r)
#define RDD_TIMER_TASK_DESCRIPTOR_ENTRY_FIRMWARE_PTR_WRITE(v, p)                  MWRITE_16((uint8_t *)p + 2, v)

#define RDD_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_SIZE     4
typedef struct
{
	RDD_TIMER_TASK_DESCRIPTOR_ENTRY_DTS	entry[ RDD_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_DTS;

#define RDD_DS_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_PTR()	( RDD_DS_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_ADDRESS )

#define RDD_SBPM_REPLY_ENTRY_RESERVED_FW_ONLY_NUMBER	32

typedef struct
{
	uint32_t	reserved_fw_only[RDD_SBPM_REPLY_ENTRY_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SBPM_REPLY_ENTRY_DTS;

#define RDD_CONNECTION_CONTEXT_BUFFER_ENTRY_RESERVED_FW_ONLY_NUMBER	16

typedef struct
{
	uint32_t	reserved_fw_only[RDD_CONNECTION_CONTEXT_BUFFER_ENTRY_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CONNECTION_CONTEXT_BUFFER_ENTRY_DTS;

#if defined OREN

#define RDD_DS_CONNECTION_CONTEXT_BUFFER_SIZE     8
typedef struct
{
	RDD_CONNECTION_CONTEXT_BUFFER_ENTRY_DTS	entry[ RDD_DS_CONNECTION_CONTEXT_BUFFER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_CONNECTION_CONTEXT_BUFFER_DTS;

#define RDD_DS_CONNECTION_CONTEXT_BUFFER_PTR()	( RDD_DS_CONNECTION_CONTEXT_BUFFER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_CONNECTION_CONTEXT_BUFFER_ADDRESS )

#endif

typedef struct
{
	uint32_t	head_ptr              	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tail_ptr              	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ingress_packet_counter	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_packet_counter 	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	packet_threshold      	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	profile_ptr           	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0             	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	counter_number        	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	queue_mask            	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_limit_override   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS;

#define RDD_ETH_TX_QUEUE_DESCRIPTOR_HEAD_PTR_READ(r, p)                               MREAD_16((uint8_t *)p, r)
#define RDD_ETH_TX_QUEUE_DESCRIPTOR_HEAD_PTR_WRITE(v, p)                              MWRITE_16((uint8_t *)p, v)
#define RDD_ETH_TX_QUEUE_DESCRIPTOR_TAIL_PTR_READ(r, p)                               MREAD_16((uint8_t *)p + 2, r)
#define RDD_ETH_TX_QUEUE_DESCRIPTOR_TAIL_PTR_WRITE(v, p)                              MWRITE_16((uint8_t *)p + 2, v)
#define RDD_ETH_TX_QUEUE_DESCRIPTOR_INGRESS_PACKET_COUNTER_READ(r, p)                 MREAD_16((uint8_t *)p + 4, r)
#define RDD_ETH_TX_QUEUE_DESCRIPTOR_INGRESS_PACKET_COUNTER_WRITE(v, p)                MWRITE_16((uint8_t *)p + 4, v)
#define RDD_ETH_TX_QUEUE_DESCRIPTOR_EGRESS_PACKET_COUNTER_READ(r, p)                  MREAD_16((uint8_t *)p + 6, r)
#define RDD_ETH_TX_QUEUE_DESCRIPTOR_EGRESS_PACKET_COUNTER_WRITE(v, p)                 MWRITE_16((uint8_t *)p + 6, v)
#define RDD_ETH_TX_QUEUE_DESCRIPTOR_PACKET_THRESHOLD_READ(r, p)                       MREAD_16((uint8_t *)p + 8, r)
#define RDD_ETH_TX_QUEUE_DESCRIPTOR_PACKET_THRESHOLD_WRITE(v, p)                      MWRITE_16((uint8_t *)p + 8, v)
#define RDD_ETH_TX_QUEUE_DESCRIPTOR_PROFILE_PTR_READ(r, p)                            MREAD_16((uint8_t *)p + 10, r)
#define RDD_ETH_TX_QUEUE_DESCRIPTOR_PROFILE_PTR_WRITE(v, p)                           MWRITE_16((uint8_t *)p + 10, v)
#define RDD_ETH_TX_QUEUE_DESCRIPTOR_COUNTER_NUMBER_READ(r, p)                         MREAD_8((uint8_t *)p + 13, r)
#define RDD_ETH_TX_QUEUE_DESCRIPTOR_COUNTER_NUMBER_WRITE(v, p)                        MWRITE_8((uint8_t *)p + 13, v)
#define RDD_ETH_TX_QUEUE_DESCRIPTOR_QUEUE_MASK_READ(r, p)                             MREAD_8((uint8_t *)p + 14, r)
#define RDD_ETH_TX_QUEUE_DESCRIPTOR_QUEUE_MASK_WRITE(v, p)                            MWRITE_8((uint8_t *)p + 14, v)
#define RDD_ETH_TX_QUEUE_DESCRIPTOR_RATE_LIMIT_OVERRIDE_READ(r, p)                    MREAD_8((uint8_t *)p + 15, r)
#define RDD_ETH_TX_QUEUE_DESCRIPTOR_RATE_LIMIT_OVERRIDE_WRITE(v, p)                   MWRITE_8((uint8_t *)p + 15, v)
#if defined OREN

#define RDD_ETH_TX_QUEUES_TABLE_SIZE     48
typedef struct
{
	RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS	entry[ RDD_ETH_TX_QUEUES_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_QUEUES_TABLE_DTS;

#define RDD_ETH_TX_QUEUES_TABLE_PTR()	( RDD_ETH_TX_QUEUES_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + ETH_TX_QUEUES_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	current_peak_budget           	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	allocated_peak_budget_exponent	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	allocated_peak_budget         	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	peak_budget_limit_exponent    	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	peak_budget_limit             	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	current_sustain_budget        	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	allocated_sustain_budget      	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_limiter_mask             	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	peak_burst_counter            	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	peak_weight                   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0                     	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	peak_burst_flag               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_DTS;

#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_CURRENT_PEAK_BUDGET_READ(r, p)                            MREAD_32((uint8_t *)p, r)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_CURRENT_PEAK_BUDGET_WRITE(v, p)                           MWRITE_32((uint8_t *)p, v)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_ALLOCATED_PEAK_BUDGET_EXPONENT_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 4, 6, 2, r)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_ALLOCATED_PEAK_BUDGET_EXPONENT_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 4, 6, 2, v)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_ALLOCATED_PEAK_BUDGET_READ(r, p)                          FIELD_MREAD_16((uint8_t *)p + 4, 0, 14, r)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_ALLOCATED_PEAK_BUDGET_WRITE(v, p)                         FIELD_MWRITE_16((uint8_t *)p + 4, 0, 14, v)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_PEAK_BUDGET_LIMIT_EXPONENT_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 6, 6, 2, r)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_PEAK_BUDGET_LIMIT_EXPONENT_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 6, 6, 2, v)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_PEAK_BUDGET_LIMIT_READ(r, p)                              FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_PEAK_BUDGET_LIMIT_WRITE(v, p)                             FIELD_MWRITE_16((uint8_t *)p + 6, 0, 14, v)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_CURRENT_SUSTAIN_BUDGET_READ(r, p)                         MREAD_32((uint8_t *)p + 8, r)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_CURRENT_SUSTAIN_BUDGET_WRITE(v, p)                        MWRITE_32((uint8_t *)p + 8, v)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_ALLOCATED_SUSTAIN_BUDGET_READ(r, p)                       MREAD_32((uint8_t *)p + 12, r)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_ALLOCATED_SUSTAIN_BUDGET_WRITE(v, p)                      MWRITE_32((uint8_t *)p + 12, v)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_RATE_LIMITER_MASK_READ(r, p)                              MREAD_32((uint8_t *)p + 16, r)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_RATE_LIMITER_MASK_WRITE(v, p)                             MWRITE_32((uint8_t *)p + 16, v)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_PEAK_BURST_COUNTER_READ(r, p)                             MREAD_16((uint8_t *)p + 20, r)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_PEAK_BURST_COUNTER_WRITE(v, p)                            MWRITE_16((uint8_t *)p + 20, v)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_PEAK_WEIGHT_READ(r, p)                                    MREAD_8((uint8_t *)p + 22, r)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_PEAK_WEIGHT_WRITE(v, p)                                   MWRITE_8((uint8_t *)p + 22, v)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_PEAK_BURST_FLAG_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 23, 0, 1, r)
#define RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_PEAK_BURST_FLAG_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 23, 0, 1, v)
#if defined OREN

#define RDD_SERVICE_QUEUES_RATE_LIMITER_TABLE_SIZE     32
typedef struct
{
	RDD_SERVICE_QUEUES_RATE_LIMITER_DESCRIPTOR_DTS	entry[ RDD_SERVICE_QUEUES_RATE_LIMITER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SERVICE_QUEUES_RATE_LIMITER_TABLE_DTS;

#define RDD_SERVICE_QUEUES_RATE_LIMITER_TABLE_PTR()	( RDD_SERVICE_QUEUES_RATE_LIMITER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + SERVICE_QUEUES_RATE_LIMITER_TABLE_ADDRESS )

#endif

typedef struct
{
	uint16_t	primitive_address	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PBITS_PRIMITIVE_ENTRY_DTS;

#define RDD_PBITS_PRIMITIVE_ENTRY_PRIMITIVE_ADDRESS_READ(r, p)                 MREAD_16((uint8_t *)p, r)
#define RDD_PBITS_PRIMITIVE_ENTRY_PRIMITIVE_ADDRESS_WRITE(v, p)                MWRITE_16((uint8_t *)p, v)
#if defined OREN

#define RDD_DS_PBITS_PRIMITIVE_TABLE_SIZE     32
typedef struct
{
	RDD_PBITS_PRIMITIVE_ENTRY_DTS	entry[ RDD_DS_PBITS_PRIMITIVE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_PBITS_PRIMITIVE_TABLE_DTS;

#define RDD_DS_PBITS_PRIMITIVE_TABLE_PTR()	( RDD_DS_PBITS_PRIMITIVE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PBITS_PRIMITIVE_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	reserved_fw_only	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FOUR_BYTES_DTS;

#if defined OREN

#define RDD_DS_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_SIZE     16
typedef struct
{
	RDD_FOUR_BYTES_DTS	entry[ RDD_DS_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_DTS;

#define RDD_DS_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_PTR()	( RDD_DS_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_ADDRESS )

#endif
#if defined OREN

#define RDD_DS_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_SIZE     16
typedef struct
{
	RDD_FOUR_BYTES_DTS	entry[ RDD_DS_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_DTS;

#define RDD_DS_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_PTR()	( RDD_DS_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_ADDRESS )

#endif

typedef struct
{
	uint16_t	allocated_budget	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	current_budget  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RATE_LIMITER_REMAINDER_ENTRY_DTS;

#define RDD_RATE_LIMITER_REMAINDER_ENTRY_ALLOCATED_BUDGET_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_RATE_LIMITER_REMAINDER_ENTRY_ALLOCATED_BUDGET_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#define RDD_RATE_LIMITER_REMAINDER_ENTRY_CURRENT_BUDGET_READ(r, p)                   MREAD_8((uint8_t *)p + 1, r)
#define RDD_RATE_LIMITER_REMAINDER_ENTRY_CURRENT_BUDGET_WRITE(v, p)                  MWRITE_8((uint8_t *)p + 1, v)
#if defined OREN

#define RDD_RATE_LIMITER_REMAINDER_TABLE_SIZE     32
typedef struct
{
	RDD_RATE_LIMITER_REMAINDER_ENTRY_DTS	entry[ RDD_RATE_LIMITER_REMAINDER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RATE_LIMITER_REMAINDER_TABLE_DTS;

#define RDD_RATE_LIMITER_REMAINDER_TABLE_PTR()	( RDD_RATE_LIMITER_REMAINDER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + RATE_LIMITER_REMAINDER_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	current_budget      	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	budget_limit_exp    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	budget_limit        	:15	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	allocated_budget_exp	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	allocated_budget    	:15	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RATE_LIMITER_ENTRY_DTS;

#define RDD_RATE_LIMITER_ENTRY_CURRENT_BUDGET_READ(r, p)                       MREAD_32((uint8_t *)p, r)
#define RDD_RATE_LIMITER_ENTRY_CURRENT_BUDGET_WRITE(v, p)                      MWRITE_32((uint8_t *)p, v)
#define RDD_RATE_LIMITER_ENTRY_BUDGET_LIMIT_EXP_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r)
#define RDD_RATE_LIMITER_ENTRY_BUDGET_LIMIT_EXP_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 4, 7, 1, v)
#define RDD_RATE_LIMITER_ENTRY_BUDGET_LIMIT_READ(r, p)                         FIELD_MREAD_16((uint8_t *)p + 4, 0, 15, r)
#define RDD_RATE_LIMITER_ENTRY_BUDGET_LIMIT_WRITE(v, p)                        FIELD_MWRITE_16((uint8_t *)p + 4, 0, 15, v)
#define RDD_RATE_LIMITER_ENTRY_ALLOCATED_BUDGET_EXP_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 6, 7, 1, r)
#define RDD_RATE_LIMITER_ENTRY_ALLOCATED_BUDGET_EXP_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 6, 7, 1, v)
#define RDD_RATE_LIMITER_ENTRY_ALLOCATED_BUDGET_READ(r, p)                     FIELD_MREAD_16((uint8_t *)p + 6, 0, 15, r)
#define RDD_RATE_LIMITER_ENTRY_ALLOCATED_BUDGET_WRITE(v, p)                    FIELD_MWRITE_16((uint8_t *)p + 6, 0, 15, v)
#if defined OREN

#define RDD_DS_RATE_LIMITER_TABLE_SIZE     32
typedef struct
{
	RDD_RATE_LIMITER_ENTRY_DTS	entry[ RDD_DS_RATE_LIMITER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_RATE_LIMITER_TABLE_DTS;

#define RDD_DS_RATE_LIMITER_TABLE_PTR()	( RDD_DS_RATE_LIMITER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_RATE_LIMITER_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	vlan_untagged_command        	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pbits_untagged_command       	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vlan_single_tagged_command   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pbits_single_tagged_command  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vlan_double_tagged_command   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pbits_double_tagged_command  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vlan_priority_tagged_command 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pbits_priority_tagged_command	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_VLAN_COMMAND_ENRTY_DTS;

#define RDD_VLAN_COMMAND_ENRTY_VLAN_UNTAGGED_COMMAND_READ(r, p)                         MREAD_8((uint8_t *)p, r)
#define RDD_VLAN_COMMAND_ENRTY_VLAN_UNTAGGED_COMMAND_WRITE(v, p)                        MWRITE_8((uint8_t *)p, v)
#define RDD_VLAN_COMMAND_ENRTY_PBITS_UNTAGGED_COMMAND_READ(r, p)                        MREAD_8((uint8_t *)p + 1, r)
#define RDD_VLAN_COMMAND_ENRTY_PBITS_UNTAGGED_COMMAND_WRITE(v, p)                       MWRITE_8((uint8_t *)p + 1, v)
#define RDD_VLAN_COMMAND_ENRTY_VLAN_SINGLE_TAGGED_COMMAND_READ(r, p)                    MREAD_8((uint8_t *)p + 2, r)
#define RDD_VLAN_COMMAND_ENRTY_VLAN_SINGLE_TAGGED_COMMAND_WRITE(v, p)                   MWRITE_8((uint8_t *)p + 2, v)
#define RDD_VLAN_COMMAND_ENRTY_PBITS_SINGLE_TAGGED_COMMAND_READ(r, p)                   MREAD_8((uint8_t *)p + 3, r)
#define RDD_VLAN_COMMAND_ENRTY_PBITS_SINGLE_TAGGED_COMMAND_WRITE(v, p)                  MWRITE_8((uint8_t *)p + 3, v)
#define RDD_VLAN_COMMAND_ENRTY_VLAN_DOUBLE_TAGGED_COMMAND_READ(r, p)                    MREAD_8((uint8_t *)p + 4, r)
#define RDD_VLAN_COMMAND_ENRTY_VLAN_DOUBLE_TAGGED_COMMAND_WRITE(v, p)                   MWRITE_8((uint8_t *)p + 4, v)
#define RDD_VLAN_COMMAND_ENRTY_PBITS_DOUBLE_TAGGED_COMMAND_READ(r, p)                   MREAD_8((uint8_t *)p + 5, r)
#define RDD_VLAN_COMMAND_ENRTY_PBITS_DOUBLE_TAGGED_COMMAND_WRITE(v, p)                  MWRITE_8((uint8_t *)p + 5, v)
#define RDD_VLAN_COMMAND_ENRTY_VLAN_PRIORITY_TAGGED_COMMAND_READ(r, p)                  MREAD_8((uint8_t *)p + 6, r)
#define RDD_VLAN_COMMAND_ENRTY_VLAN_PRIORITY_TAGGED_COMMAND_WRITE(v, p)                 MWRITE_8((uint8_t *)p + 6, v)
#define RDD_VLAN_COMMAND_ENRTY_PBITS_PRIORITY_TAGGED_COMMAND_READ(r, p)                 MREAD_8((uint8_t *)p + 7, r)
#define RDD_VLAN_COMMAND_ENRTY_PBITS_PRIORITY_TAGGED_COMMAND_WRITE(v, p)                MWRITE_8((uint8_t *)p + 7, v)
#if defined OREN

#define RDD_DS_VLAN_COMMANDS_TABLE_SIZE     64
typedef struct
{
	RDD_VLAN_COMMAND_ENRTY_DTS	entry[ RDD_DS_VLAN_COMMANDS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_VLAN_COMMANDS_TABLE_DTS;

#define RDD_DS_VLAN_COMMANDS_TABLE_PTR()	( RDD_DS_VLAN_COMMANDS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_VLAN_COMMANDS_TABLE_ADDRESS )

#endif

typedef struct
{
	uint16_t	vid       	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_VID_ENTRY_DTS;

#define RDD_VID_ENTRY_VID_READ(r, p)                 MREAD_16((uint8_t *)p, r)
#define RDD_VID_ENTRY_VID_WRITE(v, p)                MWRITE_16((uint8_t *)p, v)
#if defined OREN

#define RDD_DS_LAN_VID_TABLE_SIZE     128
typedef struct
{
	RDD_VID_ENTRY_DTS	entry[ RDD_DS_LAN_VID_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_LAN_VID_TABLE_DTS;

#define RDD_DS_LAN_VID_TABLE_PTR()	( RDD_DS_LAN_VID_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_LAN_VID_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	current_budget	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	exponent      	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	commited_rate 	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved      	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bucket_size   	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	commited_burst	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	drop_counter  	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_POLICER_ENTRY_DTS;

#define RDD_POLICER_ENTRY_CURRENT_BUDGET_READ(r, p)                 MREAD_32((uint8_t *)p, r)
#define RDD_POLICER_ENTRY_CURRENT_BUDGET_WRITE(v, p)                MWRITE_32((uint8_t *)p, v)
#define RDD_POLICER_ENTRY_EXPONENT_READ(r, p)                       MREAD_16((uint8_t *)p + 4, r)
#define RDD_POLICER_ENTRY_EXPONENT_WRITE(v, p)                      MWRITE_16((uint8_t *)p + 4, v)
#define RDD_POLICER_ENTRY_COMMITED_RATE_READ(r, p)                  MREAD_16((uint8_t *)p + 6, r)
#define RDD_POLICER_ENTRY_COMMITED_RATE_WRITE(v, p)                 MWRITE_16((uint8_t *)p + 6, v)
#define RDD_POLICER_ENTRY_BUCKET_SIZE_READ(r, p)                    MREAD_16((uint8_t *)p + 10, r)
#define RDD_POLICER_ENTRY_BUCKET_SIZE_WRITE(v, p)                   MWRITE_16((uint8_t *)p + 10, v)
#define RDD_POLICER_ENTRY_COMMITED_BURST_READ(r, p)                 MREAD_16((uint8_t *)p + 12, r)
#define RDD_POLICER_ENTRY_COMMITED_BURST_WRITE(v, p)                MWRITE_16((uint8_t *)p + 12, v)
#define RDD_POLICER_ENTRY_DROP_COUNTER_READ(r, p)                   MREAD_16((uint8_t *)p + 14, r)
#define RDD_POLICER_ENTRY_DROP_COUNTER_WRITE(v, p)                  MWRITE_16((uint8_t *)p + 14, v)

#define RDD_POLICER_TABLE_SIZE     16
typedef struct
{
	RDD_POLICER_ENTRY_DTS	entry[ RDD_POLICER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_POLICER_TABLE_DTS;

#define RDD_DS_POLICER_TABLE_PTR()	( RDD_DS_POLICER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_POLICER_TABLE_ADDRESS )

#define RDD_CPU_TX_BBH_DESCRIPTORS_ENTRY_RESERVED_FW_ONLY_NUMBER	2

typedef struct
{
	uint32_t	reserved_fw_only[RDD_CPU_TX_BBH_DESCRIPTORS_ENTRY_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_BBH_DESCRIPTORS_ENTRY_DTS;

#if defined OREN

#define RDD_DS_CPU_TX_BBH_DESCRIPTORS_SIZE     32
typedef struct
{
	RDD_CPU_TX_BBH_DESCRIPTORS_ENTRY_DTS	entry[ RDD_DS_CPU_TX_BBH_DESCRIPTORS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_CPU_TX_BBH_DESCRIPTORS_DTS;

#define RDD_DS_CPU_TX_BBH_DESCRIPTORS_PTR()	( RDD_DS_CPU_TX_BBH_DESCRIPTORS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_CPU_TX_BBH_DESCRIPTORS_ADDRESS )

#endif

typedef struct
{
	uint32_t	eth_mac_pointer 	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tx_queue_pointer	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_QUEUE_POINTERS_ENTRY_DTS;

#define RDD_ETH_TX_QUEUE_POINTERS_ENTRY_ETH_MAC_POINTER_READ(r, p)                  MREAD_16((uint8_t *)p, r)
#define RDD_ETH_TX_QUEUE_POINTERS_ENTRY_ETH_MAC_POINTER_WRITE(v, p)                 MWRITE_16((uint8_t *)p, v)
#define RDD_ETH_TX_QUEUE_POINTERS_ENTRY_TX_QUEUE_POINTER_READ(r, p)                 MREAD_16((uint8_t *)p + 2, r)
#define RDD_ETH_TX_QUEUE_POINTERS_ENTRY_TX_QUEUE_POINTER_WRITE(v, p)                MWRITE_16((uint8_t *)p + 2, v)
#if defined OREN

#define RDD_ETH_TX_QUEUES_POINTERS_TABLE_SIZE     48
typedef struct
{
	RDD_ETH_TX_QUEUE_POINTERS_ENTRY_DTS	entry[ RDD_ETH_TX_QUEUES_POINTERS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_QUEUES_POINTERS_TABLE_DTS;

#define RDD_ETH_TX_QUEUES_POINTERS_TABLE_PTR()	( RDD_ETH_TX_QUEUES_POINTERS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + ETH_TX_QUEUES_POINTERS_TABLE_ADDRESS )

#endif

typedef struct
{
	uint16_t	reserved_fw_only	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_TWO_BYTES_DTS;

#if defined OREN

#define RDD_DS_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_SIZE     32
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_DS_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_DTS;

#define RDD_DS_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_PTR()	( RDD_DS_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_ADDRESS )

#endif
#define RDD_EIGHT_BYTES_RESERVED_FW_ONLY_NUMBER	2

typedef struct
{
	uint32_t	reserved_fw_only[RDD_EIGHT_BYTES_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EIGHT_BYTES_DTS;

#if defined OREN

#define RDD_CPU_RX_PD_INGRESS_QUEUE_SIZE     32
typedef struct
{
	RDD_EIGHT_BYTES_DTS	entry[ RDD_CPU_RX_PD_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_PD_INGRESS_QUEUE_DTS;

#define RDD_CPU_RX_PD_INGRESS_QUEUE_PTR()	( RDD_CPU_RX_PD_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + CPU_RX_PD_INGRESS_QUEUE_ADDRESS )

#endif

typedef struct
{
	uint8_t	cpu_trap  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	drop      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	parameter 	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DTS;

#define RDD_INGRESS_FILTERS_PARAMETER_ENTRY_CPU_TRAP_READ(r, p)                  FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_INGRESS_FILTERS_PARAMETER_ENTRY_CPU_TRAP_WRITE(v, p)                 FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DROP_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p, 6, 1, r)
#define RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DROP_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p, 6, 1, v)
#define RDD_INGRESS_FILTERS_PARAMETER_ENTRY_PARAMETER_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 0, 6, r)
#define RDD_INGRESS_FILTERS_PARAMETER_ENTRY_PARAMETER_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 0, 6, v)
#if defined OREN

#define RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_SIZE     10
#define RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_SIZE2    16
typedef struct
{
	RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DTS	entry[ RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_SIZE ][ RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_DTS;

#define RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_PTR()	( RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_FILTERS_PARAMETER_TABLE_ADDRESS )

#endif

#define RDD_PICO_TIMER_TASK_DESCRIPTOR_TABLE_SIZE     4
typedef struct
{
	RDD_TIMER_TASK_DESCRIPTOR_ENTRY_DTS	entry[ RDD_PICO_TIMER_TASK_DESCRIPTOR_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PICO_TIMER_TASK_DESCRIPTOR_TABLE_DTS;

#define RDD_DS_PICO_TIMER_TASK_DESCRIPTOR_TABLE_PTR()	( RDD_DS_PICO_TIMER_TASK_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PICO_TIMER_TASK_DESCRIPTOR_TABLE_ADDRESS )

#define RDD_IPTV_SSID_EXTENSION_TABLE_CAM_RESERVED_FW_ONLY_NUMBER	16

typedef struct
{
	uint32_t	reserved_fw_only[RDD_IPTV_SSID_EXTENSION_TABLE_CAM_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_SSID_EXTENSION_TABLE_CAM_DTS;

#if defined OREN

#define RDD_CPU_RX_FAST_PD_INGRESS_QUEUE_SIZE     32
typedef struct
{
	RDD_EIGHT_BYTES_DTS	entry[ RDD_CPU_RX_FAST_PD_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_FAST_PD_INGRESS_QUEUE_DTS;

#define RDD_CPU_RX_FAST_PD_INGRESS_QUEUE_PTR()	( RDD_CPU_RX_FAST_PD_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + CPU_RX_FAST_PD_INGRESS_QUEUE_ADDRESS )

#endif

typedef struct
{
	uint8_t	enable    	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FORWARDING_MATRIX_ENTRY_DTS;

#define RDD_FORWARDING_MATRIX_ENTRY_ENABLE_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_FORWARDING_MATRIX_ENTRY_ENABLE_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#if defined OREN

#define RDD_DS_FORWARDING_MATRIX_TABLE_SIZE     9
#define RDD_DS_FORWARDING_MATRIX_TABLE_SIZE2    16
typedef struct
{
	RDD_FORWARDING_MATRIX_ENTRY_DTS	entry[ RDD_DS_FORWARDING_MATRIX_TABLE_SIZE ][ RDD_DS_FORWARDING_MATRIX_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_FORWARDING_MATRIX_TABLE_DTS;

#define RDD_DS_FORWARDING_MATRIX_TABLE_PTR()	( RDD_DS_FORWARDING_MATRIX_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_FORWARDING_MATRIX_TABLE_ADDRESS )

#endif

typedef struct
{
	uint16_t	tpid      	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_TPID_OVERWRITE_ENTRY_DTS;

#define RDD_TPID_OVERWRITE_ENTRY_TPID_READ(r, p)                 MREAD_16((uint8_t *)p, r)
#define RDD_TPID_OVERWRITE_ENTRY_TPID_WRITE(v, p)                MWRITE_16((uint8_t *)p, v)
#if defined OREN

#define RDD_DS_TPID_OVERWRITE_TABLE_SIZE     8
typedef struct
{
	RDD_TPID_OVERWRITE_ENTRY_DTS	entry[ RDD_DS_TPID_OVERWRITE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_TPID_OVERWRITE_TABLE_DTS;

#define RDD_DS_TPID_OVERWRITE_TABLE_PTR()	( RDD_DS_TPID_OVERWRITE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_TPID_OVERWRITE_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	generic_rule_type  	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1          	:23	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	generic_rule_offset	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	generic_rule_mask  	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_ENTRY_DTS;

#define RDD_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_ENTRY_GENERIC_RULE_TYPE_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p, 6, 2, r)
#define RDD_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_ENTRY_GENERIC_RULE_TYPE_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p, 6, 2, v)
#define RDD_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_ENTRY_GENERIC_RULE_OFFSET_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 3, 0, 7, r)
#define RDD_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_ENTRY_GENERIC_RULE_OFFSET_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 3, 0, 7, v)
#define RDD_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_ENTRY_GENERIC_RULE_MASK_READ(r, p)                   MREAD_32((uint8_t *)p + 4, r)
#define RDD_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_ENTRY_GENERIC_RULE_MASK_WRITE(v, p)                  MWRITE_32((uint8_t *)p + 4, v)
#if defined OREN

#define RDD_DS_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_SIZE     4
typedef struct
{
	RDD_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_ENTRY_DTS	entry[ RDD_DS_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_DTS;

#define RDD_DS_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_PTR()	( RDD_DS_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_DS_FLOW_BASED_ACTION_PTR_TABLE_SIZE     32
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_DS_FLOW_BASED_ACTION_PTR_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_FLOW_BASED_ACTION_PTR_TABLE_DTS;

#define RDD_DS_FLOW_BASED_ACTION_PTR_TABLE_PTR()	( RDD_DS_FLOW_BASED_ACTION_PTR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_FLOW_BASED_ACTION_PTR_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	_5_tupple_valid            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_first_fragment          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_fragment                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_filter_match            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_filter_match_num        	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tcp_udp                    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ipv6_ext_header_filter     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tcp_flag                   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wan                        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vid_fit                    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	exception                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	da_filter_number           	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	da_filter                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	l4_protocol                	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	error                      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ptag                       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	number_of_vlans            	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	broadcast                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	multicast                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	l3_protocol                	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	l2_protocol                	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	_5_tupple_valid_mask       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_first_fragment_mask     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_fragment_mask           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_filter_match_mask       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_filter_match_num_mask   	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tcp_udp_mask               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ipv6_ext_header_filter_mask	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tcp_flag_mask              	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wan_mask                   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vid_fit_mask               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	exception_mask             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	da_filter_number_mask      	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	da_filter_mask             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	l4_protocol_mask           	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	error_mask                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ptag_mask                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	number_of_vlans_mask       	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	broadcast_mask             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	multicast_mask             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	l3_protocol_mask           	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	l2_protocol_mask           	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LAYER4_FILTERS_LOOKUP_ENTRY_DTS;

#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY__5_TUPPLE_VALID_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY__5_TUPPLE_VALID_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FIRST_FRAGMENT_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p, 6, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FIRST_FRAGMENT_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p, 6, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FRAGMENT_READ(r, p)                                 FIELD_MREAD_8((uint8_t *)p, 5, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FRAGMENT_WRITE(v, p)                                FIELD_MWRITE_8((uint8_t *)p, 5, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FILTER_MATCH_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p, 4, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FILTER_MATCH_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p, 4, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FILTER_MATCH_NUM_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p, 2, 2, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FILTER_MATCH_NUM_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p, 2, 2, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_TCP_UDP_READ(r, p)                                     FIELD_MREAD_8((uint8_t *)p, 1, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_TCP_UDP_WRITE(v, p)                                    FIELD_MWRITE_8((uint8_t *)p, 1, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IPV6_EXT_HEADER_FILTER_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p, 0, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IPV6_EXT_HEADER_FILTER_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p, 0, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_TCP_FLAG_READ(r, p)                                    FIELD_MREAD_8((uint8_t *)p + 1, 7, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_TCP_FLAG_WRITE(v, p)                                   FIELD_MWRITE_8((uint8_t *)p + 1, 7, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_WAN_READ(r, p)                                         FIELD_MREAD_8((uint8_t *)p + 1, 6, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_WAN_WRITE(v, p)                                        FIELD_MWRITE_8((uint8_t *)p + 1, 6, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_VID_FIT_READ(r, p)                                     FIELD_MREAD_8((uint8_t *)p + 1, 5, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_VID_FIT_WRITE(v, p)                                    FIELD_MWRITE_8((uint8_t *)p + 1, 5, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_EXCEPTION_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p + 1, 4, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_EXCEPTION_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p + 1, 4, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_DA_FILTER_NUMBER_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 1, 1, 3, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_DA_FILTER_NUMBER_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 1, 1, 3, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_DA_FILTER_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p + 1, 0, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_DA_FILTER_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p + 1, 0, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_READ(r, p)                                 FIELD_MREAD_8((uint8_t *)p + 2, 4, 4, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_WRITE(v, p)                                FIELD_MWRITE_8((uint8_t *)p + 2, 4, 4, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_ERROR_READ(r, p)                                       FIELD_MREAD_8((uint8_t *)p + 2, 3, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_ERROR_WRITE(v, p)                                      FIELD_MWRITE_8((uint8_t *)p + 2, 3, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_PTAG_READ(r, p)                                        FIELD_MREAD_8((uint8_t *)p + 2, 2, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_PTAG_WRITE(v, p)                                       FIELD_MWRITE_8((uint8_t *)p + 2, 2, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_NUMBER_OF_VLANS_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 2, 0, 2, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_NUMBER_OF_VLANS_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 2, 0, 2, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_BROADCAST_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p + 3, 7, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_BROADCAST_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p + 3, 7, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_MULTICAST_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p + 3, 6, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_MULTICAST_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p + 3, 6, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L3_PROTOCOL_READ(r, p)                                 FIELD_MREAD_8((uint8_t *)p + 3, 4, 2, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L3_PROTOCOL_WRITE(v, p)                                FIELD_MWRITE_8((uint8_t *)p + 3, 4, 2, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_READ(r, p)                                 FIELD_MREAD_8((uint8_t *)p + 3, 0, 4, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_WRITE(v, p)                                FIELD_MWRITE_8((uint8_t *)p + 3, 0, 4, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY__5_TUPPLE_VALID_MASK_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY__5_TUPPLE_VALID_MASK_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 4, 7, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FIRST_FRAGMENT_MASK_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FIRST_FRAGMENT_MASK_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 4, 6, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FRAGMENT_MASK_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 4, 5, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FRAGMENT_MASK_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 4, 5, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FILTER_MATCH_MASK_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 4, 4, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FILTER_MATCH_MASK_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 4, 4, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FILTER_MATCH_NUM_MASK_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 4, 2, 2, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IP_FILTER_MATCH_NUM_MASK_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 4, 2, 2, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_TCP_UDP_MASK_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 4, 1, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_TCP_UDP_MASK_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 4, 1, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IPV6_EXT_HEADER_FILTER_MASK_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 4, 0, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_IPV6_EXT_HEADER_FILTER_MASK_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 4, 0, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_TCP_FLAG_MASK_READ(r, p)                               FIELD_MREAD_8((uint8_t *)p + 5, 7, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_TCP_FLAG_MASK_WRITE(v, p)                              FIELD_MWRITE_8((uint8_t *)p + 5, 7, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_WAN_MASK_READ(r, p)                                    FIELD_MREAD_8((uint8_t *)p + 5, 6, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_WAN_MASK_WRITE(v, p)                                   FIELD_MWRITE_8((uint8_t *)p + 5, 6, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_VID_FIT_MASK_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 5, 5, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_VID_FIT_MASK_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 5, 5, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_EXCEPTION_MASK_READ(r, p)                              FIELD_MREAD_8((uint8_t *)p + 5, 4, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_EXCEPTION_MASK_WRITE(v, p)                             FIELD_MWRITE_8((uint8_t *)p + 5, 4, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_DA_FILTER_NUMBER_MASK_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 5, 1, 3, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_DA_FILTER_NUMBER_MASK_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 5, 1, 3, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_DA_FILTER_MASK_READ(r, p)                              FIELD_MREAD_8((uint8_t *)p + 5, 0, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_DA_FILTER_MASK_WRITE(v, p)                             FIELD_MWRITE_8((uint8_t *)p + 5, 0, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 6, 4, 4, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L4_PROTOCOL_MASK_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 6, 4, 4, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_ERROR_MASK_READ(r, p)                                  FIELD_MREAD_8((uint8_t *)p + 6, 3, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_ERROR_MASK_WRITE(v, p)                                 FIELD_MWRITE_8((uint8_t *)p + 6, 3, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_PTAG_MASK_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p + 6, 2, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_PTAG_MASK_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p + 6, 2, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_NUMBER_OF_VLANS_MASK_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 6, 0, 2, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_NUMBER_OF_VLANS_MASK_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 6, 0, 2, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_BROADCAST_MASK_READ(r, p)                              FIELD_MREAD_8((uint8_t *)p + 7, 7, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_BROADCAST_MASK_WRITE(v, p)                             FIELD_MWRITE_8((uint8_t *)p + 7, 7, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_MULTICAST_MASK_READ(r, p)                              FIELD_MREAD_8((uint8_t *)p + 7, 6, 1, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_MULTICAST_MASK_WRITE(v, p)                             FIELD_MWRITE_8((uint8_t *)p + 7, 6, 1, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L3_PROTOCOL_MASK_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 7, 4, 2, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L3_PROTOCOL_MASK_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 7, 4, 2, v)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 7, 0, 4, r)
#define RDD_LAYER4_FILTERS_LOOKUP_ENTRY_L2_PROTOCOL_MASK_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 7, 0, 4, v)
#if defined OREN

#define RDD_DS_LAYER4_FILTERS_LOOKUP_TABLE_SIZE     16
typedef struct
{
	RDD_LAYER4_FILTERS_LOOKUP_ENTRY_DTS	entry[ RDD_DS_LAYER4_FILTERS_LOOKUP_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_LAYER4_FILTERS_LOOKUP_TABLE_DTS;

#define RDD_DS_LAYER4_FILTERS_LOOKUP_TABLE_PTR()	( RDD_DS_LAYER4_FILTERS_LOOKUP_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_LAYER4_FILTERS_LOOKUP_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	current_budget  	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	budget_limit    	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	allocated_budget	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved        	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_METER_ENTRY_DTS;

#define RDD_CPU_RX_METER_ENTRY_CURRENT_BUDGET_READ(r, p)                   MREAD_16((uint8_t *)p, r)
#define RDD_CPU_RX_METER_ENTRY_CURRENT_BUDGET_WRITE(v, p)                  MWRITE_16((uint8_t *)p, v)
#define RDD_CPU_RX_METER_ENTRY_BUDGET_LIMIT_READ(r, p)                     MREAD_16((uint8_t *)p + 2, r)
#define RDD_CPU_RX_METER_ENTRY_BUDGET_LIMIT_WRITE(v, p)                    MWRITE_16((uint8_t *)p + 2, v)
#define RDD_CPU_RX_METER_ENTRY_ALLOCATED_BUDGET_READ(r, p)                 MREAD_16((uint8_t *)p + 4, r)
#define RDD_CPU_RX_METER_ENTRY_ALLOCATED_BUDGET_WRITE(v, p)                MWRITE_16((uint8_t *)p + 4, v)

#define RDD_CPU_RX_METER_TABLE_SIZE     16
typedef struct
{
	RDD_CPU_RX_METER_ENTRY_DTS	entry[ RDD_CPU_RX_METER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_METER_TABLE_DTS;

#define RDD_DS_CPU_RX_METER_TABLE_PTR()	( RDD_DS_CPU_RX_METER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_CPU_RX_METER_TABLE_ADDRESS )

#define RDD_BRIDGE_CONFIGURATION_REGISTER_RESERVED_BYTES_NUMBER	179

typedef struct
{
	uint32_t	ds_miss_eth_flow                                                            	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ds_untagged_eth_flow                                                        	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wan_to_wan_ingress_flow                                                     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_limit_overhead                                                         	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	subnet_classification_mode                                                  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	global_ingress_config                                                       	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	drop_precedence_eligibility_vector                                          	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wan_unknown_sa_command                                                      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth0_unknown_sa_command                                                     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth1_unknown_sa_command                                                     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth2_unknown_sa_command                                                     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth3_unknown_sa_command                                                     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth4_unknown_sa_command                                                     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wan_router_unknown_sa_command                                               	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wan_iptv_unknown_sa_command                                                 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pci_unknown_sa_command                                                      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	us_unknown_da_flooding_bridge_port                                          	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wan_unknown_da_command                                                      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth0_unknown_da_command                                                     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth1_unknown_da_command                                                     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth2_unknown_da_command                                                     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth3_unknown_da_command                                                     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth4_unknown_da_command                                                     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wan_router_unknown_da_command                                               	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	broadcom_switch_port                                                        	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pci_unknown_da_command                                                      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	flooding_bridge_ports_vector                                                	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_ether_type_1                                                         	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_ether_type_2                                                         	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_ether_type_3                                                         	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	us_rate_controller_timer                                                    	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	us_g9991_mtu_max_fragments                                                  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	us_g9991_mtu_max_eof_length                                                 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	us_tx_queue_flow_control_enable                                             	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	packet_buffer_size_asr_8                                                    	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mtu                                                                         	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1                                                                   	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pci_ls_dp_eligibility_vector                                                	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_sync_1588_mode                                                           	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	iptv_classification_method                                                  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ds_connection_miss_action                                                   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dscp_to_wan_flow_control                                                    	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ipv6_enable                                                                 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	hash_based_forwarding_port_count                                            	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	us_padding_max_size                                                         	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	us_padding_cpu_max_size                                                     	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	active_policers_vector                                                      	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mirroring_port                                                              	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vlan_binding_mode                                                           	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	policers_period                                                             	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	timer_scheduler_period                                                      	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tpid_detect_value                                                           	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	flooding_wifi_ssid_vector                                                   	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	inter_lan_scheduling_mode                                                   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ipv6_ecn_remark                                                             	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ds_ingress_policers_mode                                                    	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	debug_mode                                                                  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	us_rate_limit_sustain_budget_limit                                          	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	force_dscp_to_pbit                                                          	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wan_channel_mapping                                                         	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ack_prioritization                                                          	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ack_packet_size_threshold                                                   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	reserved_bytes[RDD_BRIDGE_CONFIGURATION_REGISTER_RESERVED_BYTES_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BRIDGE_CONFIGURATION_REGISTER_DTS;

#define RDD_BRIDGE_CONFIGURATION_REGISTER_DS_MISS_ETH_FLOW_READ(r, p)                                   MREAD_8((uint8_t *)p, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_DS_MISS_ETH_FLOW_WRITE(v, p)                                  MWRITE_8((uint8_t *)p, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_DS_UNTAGGED_ETH_FLOW_READ(r, p)                               MREAD_8((uint8_t *)p + 1, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_DS_UNTAGGED_ETH_FLOW_WRITE(v, p)                              MWRITE_8((uint8_t *)p + 1, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_WAN_TO_WAN_INGRESS_FLOW_READ(r, p)                            MREAD_8((uint8_t *)p + 2, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_WAN_TO_WAN_INGRESS_FLOW_WRITE(v, p)                           MWRITE_8((uint8_t *)p + 2, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_RATE_LIMIT_OVERHEAD_READ(r, p)                                MREAD_8((uint8_t *)p + 3, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_RATE_LIMIT_OVERHEAD_WRITE(v, p)                               MWRITE_8((uint8_t *)p + 3, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_SUBNET_CLASSIFICATION_MODE_READ(r, p)                         MREAD_8((uint8_t *)p + 4, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_SUBNET_CLASSIFICATION_MODE_WRITE(v, p)                        MWRITE_8((uint8_t *)p + 4, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_GLOBAL_INGRESS_CONFIG_READ(r, p)                              MREAD_8((uint8_t *)p + 5, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_GLOBAL_INGRESS_CONFIG_WRITE(v, p)                             MWRITE_8((uint8_t *)p + 5, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_DROP_PRECEDENCE_ELIGIBILITY_VECTOR_READ(r, p)                 MREAD_16((uint8_t *)p + 6, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_DROP_PRECEDENCE_ELIGIBILITY_VECTOR_WRITE(v, p)                MWRITE_16((uint8_t *)p + 6, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_WAN_UNKNOWN_SA_COMMAND_READ(r, p)                             MREAD_8((uint8_t *)p + 8, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_WAN_UNKNOWN_SA_COMMAND_WRITE(v, p)                            MWRITE_8((uint8_t *)p + 8, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ETH0_UNKNOWN_SA_COMMAND_READ(r, p)                            MREAD_8((uint8_t *)p + 9, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ETH0_UNKNOWN_SA_COMMAND_WRITE(v, p)                           MWRITE_8((uint8_t *)p + 9, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ETH1_UNKNOWN_SA_COMMAND_READ(r, p)                            MREAD_8((uint8_t *)p + 10, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ETH1_UNKNOWN_SA_COMMAND_WRITE(v, p)                           MWRITE_8((uint8_t *)p + 10, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ETH2_UNKNOWN_SA_COMMAND_READ(r, p)                            MREAD_8((uint8_t *)p + 11, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ETH2_UNKNOWN_SA_COMMAND_WRITE(v, p)                           MWRITE_8((uint8_t *)p + 11, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ETH3_UNKNOWN_SA_COMMAND_READ(r, p)                            MREAD_8((uint8_t *)p + 12, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ETH3_UNKNOWN_SA_COMMAND_WRITE(v, p)                           MWRITE_8((uint8_t *)p + 12, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ETH4_UNKNOWN_SA_COMMAND_READ(r, p)                            MREAD_8((uint8_t *)p + 13, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ETH4_UNKNOWN_SA_COMMAND_WRITE(v, p)                           MWRITE_8((uint8_t *)p + 13, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_WAN_ROUTER_UNKNOWN_SA_COMMAND_READ(r, p)                      MREAD_8((uint8_t *)p + 14, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_WAN_ROUTER_UNKNOWN_SA_COMMAND_WRITE(v, p)                     MWRITE_8((uint8_t *)p + 14, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_WAN_IPTV_UNKNOWN_SA_COMMAND_READ(r, p)                        MREAD_8((uint8_t *)p + 15, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_WAN_IPTV_UNKNOWN_SA_COMMAND_WRITE(v, p)                       MWRITE_8((uint8_t *)p + 15, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_PCI_UNKNOWN_SA_COMMAND_READ(r, p)                             MREAD_8((uint8_t *)p + 16, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_PCI_UNKNOWN_SA_COMMAND_WRITE(v, p)                            MWRITE_8((uint8_t *)p + 16, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_US_UNKNOWN_DA_FLOODING_BRIDGE_PORT_READ(r, p)                 MREAD_8((uint8_t *)p + 17, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_US_UNKNOWN_DA_FLOODING_BRIDGE_PORT_WRITE(v, p)                MWRITE_8((uint8_t *)p + 17, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_WAN_UNKNOWN_DA_COMMAND_READ(r, p)                             MREAD_8((uint8_t *)p + 18, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_WAN_UNKNOWN_DA_COMMAND_WRITE(v, p)                            MWRITE_8((uint8_t *)p + 18, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ETH0_UNKNOWN_DA_COMMAND_READ(r, p)                            MREAD_8((uint8_t *)p + 19, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ETH0_UNKNOWN_DA_COMMAND_WRITE(v, p)                           MWRITE_8((uint8_t *)p + 19, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ETH1_UNKNOWN_DA_COMMAND_READ(r, p)                            MREAD_8((uint8_t *)p + 20, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ETH1_UNKNOWN_DA_COMMAND_WRITE(v, p)                           MWRITE_8((uint8_t *)p + 20, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ETH2_UNKNOWN_DA_COMMAND_READ(r, p)                            MREAD_8((uint8_t *)p + 21, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ETH2_UNKNOWN_DA_COMMAND_WRITE(v, p)                           MWRITE_8((uint8_t *)p + 21, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ETH3_UNKNOWN_DA_COMMAND_READ(r, p)                            MREAD_8((uint8_t *)p + 22, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ETH3_UNKNOWN_DA_COMMAND_WRITE(v, p)                           MWRITE_8((uint8_t *)p + 22, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ETH4_UNKNOWN_DA_COMMAND_READ(r, p)                            MREAD_8((uint8_t *)p + 23, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ETH4_UNKNOWN_DA_COMMAND_WRITE(v, p)                           MWRITE_8((uint8_t *)p + 23, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_WAN_ROUTER_UNKNOWN_DA_COMMAND_READ(r, p)                      MREAD_8((uint8_t *)p + 24, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_WAN_ROUTER_UNKNOWN_DA_COMMAND_WRITE(v, p)                     MWRITE_8((uint8_t *)p + 24, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_BROADCOM_SWITCH_PORT_READ(r, p)                               MREAD_8((uint8_t *)p + 25, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_BROADCOM_SWITCH_PORT_WRITE(v, p)                              MWRITE_8((uint8_t *)p + 25, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_PCI_UNKNOWN_DA_COMMAND_READ(r, p)                             MREAD_8((uint8_t *)p + 26, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_PCI_UNKNOWN_DA_COMMAND_WRITE(v, p)                            MWRITE_8((uint8_t *)p + 26, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_FLOODING_BRIDGE_PORTS_VECTOR_READ(r, p)                       MREAD_8((uint8_t *)p + 27, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_FLOODING_BRIDGE_PORTS_VECTOR_WRITE(v, p)                      MWRITE_8((uint8_t *)p + 27, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_EGRESS_ETHER_TYPE_1_READ(r, p)                                MREAD_16((uint8_t *)p + 28, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_EGRESS_ETHER_TYPE_1_WRITE(v, p)                               MWRITE_16((uint8_t *)p + 28, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_EGRESS_ETHER_TYPE_2_READ(r, p)                                MREAD_16((uint8_t *)p + 30, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_EGRESS_ETHER_TYPE_2_WRITE(v, p)                               MWRITE_16((uint8_t *)p + 30, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_EGRESS_ETHER_TYPE_3_READ(r, p)                                MREAD_16((uint8_t *)p + 32, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_EGRESS_ETHER_TYPE_3_WRITE(v, p)                               MWRITE_16((uint8_t *)p + 32, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_US_RATE_CONTROLLER_TIMER_READ(r, p)                           MREAD_16((uint8_t *)p + 34, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_US_RATE_CONTROLLER_TIMER_WRITE(v, p)                          MWRITE_16((uint8_t *)p + 34, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_US_G9991_MTU_MAX_FRAGMENTS_READ(r, p)                         MREAD_8((uint8_t *)p + 36, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_US_G9991_MTU_MAX_FRAGMENTS_WRITE(v, p)                        MWRITE_8((uint8_t *)p + 36, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_US_G9991_MTU_MAX_EOF_LENGTH_READ(r, p)                        MREAD_8((uint8_t *)p + 37, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_US_G9991_MTU_MAX_EOF_LENGTH_WRITE(v, p)                       MWRITE_8((uint8_t *)p + 37, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_US_TX_QUEUE_FLOW_CONTROL_ENABLE_READ(r, p)                    MREAD_8((uint8_t *)p + 38, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_US_TX_QUEUE_FLOW_CONTROL_ENABLE_WRITE(v, p)                   MWRITE_8((uint8_t *)p + 38, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_PACKET_BUFFER_SIZE_ASR_8_READ(r, p)                           MREAD_8((uint8_t *)p + 39, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_PACKET_BUFFER_SIZE_ASR_8_WRITE(v, p)                          MWRITE_8((uint8_t *)p + 39, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_MTU_READ(r, p)                                                MREAD_16((uint8_t *)p + 40, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_MTU_WRITE(v, p)                                               MWRITE_16((uint8_t *)p + 40, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_PCI_LS_DP_ELIGIBILITY_VECTOR_READ(r, p)                       MREAD_16((uint8_t *)p + 44, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_PCI_LS_DP_ELIGIBILITY_VECTOR_WRITE(v, p)                      MWRITE_16((uint8_t *)p + 44, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_IP_SYNC_1588_MODE_READ(r, p)                                  MREAD_8((uint8_t *)p + 46, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_IP_SYNC_1588_MODE_WRITE(v, p)                                 MWRITE_8((uint8_t *)p + 46, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_IPTV_CLASSIFICATION_METHOD_READ(r, p)                         MREAD_8((uint8_t *)p + 47, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_IPTV_CLASSIFICATION_METHOD_WRITE(v, p)                        MWRITE_8((uint8_t *)p + 47, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_DS_CONNECTION_MISS_ACTION_READ(r, p)                          MREAD_8((uint8_t *)p + 48, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_DS_CONNECTION_MISS_ACTION_WRITE(v, p)                         MWRITE_8((uint8_t *)p + 48, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_DSCP_TO_WAN_FLOW_CONTROL_READ(r, p)                           MREAD_8((uint8_t *)p + 49, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_DSCP_TO_WAN_FLOW_CONTROL_WRITE(v, p)                          MWRITE_8((uint8_t *)p + 49, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_IPV6_ENABLE_READ(r, p)                                        MREAD_8((uint8_t *)p + 50, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_IPV6_ENABLE_WRITE(v, p)                                       MWRITE_8((uint8_t *)p + 50, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_HASH_BASED_FORWARDING_PORT_COUNT_READ(r, p)                   MREAD_8((uint8_t *)p + 51, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_HASH_BASED_FORWARDING_PORT_COUNT_WRITE(v, p)                  MWRITE_8((uint8_t *)p + 51, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_US_PADDING_MAX_SIZE_READ(r, p)                                MREAD_16((uint8_t *)p + 52, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_US_PADDING_MAX_SIZE_WRITE(v, p)                               MWRITE_16((uint8_t *)p + 52, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_US_PADDING_CPU_MAX_SIZE_READ(r, p)                            MREAD_16((uint8_t *)p + 54, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_US_PADDING_CPU_MAX_SIZE_WRITE(v, p)                           MWRITE_16((uint8_t *)p + 54, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ACTIVE_POLICERS_VECTOR_READ(r, p)                             MREAD_16((uint8_t *)p + 56, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ACTIVE_POLICERS_VECTOR_WRITE(v, p)                            MWRITE_16((uint8_t *)p + 56, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_MIRRORING_PORT_READ(r, p)                                     MREAD_8((uint8_t *)p + 58, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_MIRRORING_PORT_WRITE(v, p)                                    MWRITE_8((uint8_t *)p + 58, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_VLAN_BINDING_MODE_READ(r, p)                                  MREAD_8((uint8_t *)p + 59, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_VLAN_BINDING_MODE_WRITE(v, p)                                 MWRITE_8((uint8_t *)p + 59, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_POLICERS_PERIOD_READ(r, p)                                    MREAD_16((uint8_t *)p + 60, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_POLICERS_PERIOD_WRITE(v, p)                                   MWRITE_16((uint8_t *)p + 60, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_TIMER_SCHEDULER_PERIOD_READ(r, p)                             MREAD_16((uint8_t *)p + 62, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_TIMER_SCHEDULER_PERIOD_WRITE(v, p)                            MWRITE_16((uint8_t *)p + 62, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_TPID_DETECT_VALUE_READ(r, p)                                  MREAD_16((uint8_t *)p + 64, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_TPID_DETECT_VALUE_WRITE(v, p)                                 MWRITE_16((uint8_t *)p + 64, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_FLOODING_WIFI_SSID_VECTOR_READ(r, p)                          MREAD_16((uint8_t *)p + 66, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_FLOODING_WIFI_SSID_VECTOR_WRITE(v, p)                         MWRITE_16((uint8_t *)p + 66, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_INTER_LAN_SCHEDULING_MODE_READ(r, p)                          MREAD_8((uint8_t *)p + 68, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_INTER_LAN_SCHEDULING_MODE_WRITE(v, p)                         MWRITE_8((uint8_t *)p + 68, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_IPV6_ECN_REMARK_READ(r, p)                                    MREAD_8((uint8_t *)p + 69, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_IPV6_ECN_REMARK_WRITE(v, p)                                   MWRITE_8((uint8_t *)p + 69, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_DS_INGRESS_POLICERS_MODE_READ(r, p)                           MREAD_8((uint8_t *)p + 70, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_DS_INGRESS_POLICERS_MODE_WRITE(v, p)                          MWRITE_8((uint8_t *)p + 70, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_DEBUG_MODE_READ(r, p)                                         MREAD_8((uint8_t *)p + 71, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_DEBUG_MODE_WRITE(v, p)                                        MWRITE_8((uint8_t *)p + 71, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_US_RATE_LIMIT_SUSTAIN_BUDGET_LIMIT_READ(r, p)                 MREAD_8((uint8_t *)p + 72, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_US_RATE_LIMIT_SUSTAIN_BUDGET_LIMIT_WRITE(v, p)                MWRITE_8((uint8_t *)p + 72, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_FORCE_DSCP_TO_PBIT_READ(r, p)                                 MREAD_8((uint8_t *)p + 73, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_FORCE_DSCP_TO_PBIT_WRITE(v, p)                                MWRITE_8((uint8_t *)p + 73, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_WAN_CHANNEL_MAPPING_READ(r, p)                                MREAD_8((uint8_t *)p + 74, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_WAN_CHANNEL_MAPPING_WRITE(v, p)                               MWRITE_8((uint8_t *)p + 74, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ACK_PRIORITIZATION_READ(r, p)                                 MREAD_8((uint8_t *)p + 75, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ACK_PRIORITIZATION_WRITE(v, p)                                MWRITE_8((uint8_t *)p + 75, v)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ACK_PACKET_SIZE_THRESHOLD_READ(r, p)                          MREAD_8((uint8_t *)p + 76, r)
#define RDD_BRIDGE_CONFIGURATION_REGISTER_ACK_PACKET_SIZE_THRESHOLD_WRITE(v, p)                         MWRITE_8((uint8_t *)p + 76, v)
#if defined OREN

#define RDD_INGRESS_HANDLER_SKB_DATA_POINTER_SIZE     32
typedef struct
{
	RDD_FOUR_BYTES_DTS	entry[ RDD_INGRESS_HANDLER_SKB_DATA_POINTER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INGRESS_HANDLER_SKB_DATA_POINTER_DTS;

#define RDD_INGRESS_HANDLER_SKB_DATA_POINTER_PTR()	( RDD_INGRESS_HANDLER_SKB_DATA_POINTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + INGRESS_HANDLER_SKB_DATA_POINTER_ADDRESS )

#endif

typedef struct
{
	uint32_t	next_rule_cfg_id    	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	next_group_id       	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rule_type           	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	lookup_mode         	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	hit_action          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	miss_action         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1           	:15	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2           	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	generic_rule_index_1	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	generic_rule_index_2	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	key_mask            	:24	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_DTS;

#define RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p, 3, 5, r)
#define RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_RULE_CFG_ID_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p, 3, 5, v)
#define RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_READ(r, p)                        FIELD_MREAD_16((uint8_t *)p, 6, 5, r)
#define RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_NEXT_GROUP_ID_WRITE(v, p)                       FIELD_MWRITE_16((uint8_t *)p, 6, 5, v)
#define RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_RULE_TYPE_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 1, 3, 3, r)
#define RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_RULE_TYPE_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 1, 3, 3, v)
#define RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_LOOKUP_MODE_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 1, 1, 2, r)
#define RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_LOOKUP_MODE_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 1, 1, 2, v)
#define RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_HIT_ACTION_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 1, 0, 1, r)
#define RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_HIT_ACTION_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 1, 0, 1, v)
#define RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_MISS_ACTION_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 2, 7, 1, r)
#define RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_MISS_ACTION_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 2, 7, 1, v)
#define RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_GENERIC_RULE_INDEX_1_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 4, 2, 2, r)
#define RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_GENERIC_RULE_INDEX_1_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 4, 2, 2, v)
#define RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_GENERIC_RULE_INDEX_2_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 4, 0, 2, r)
#define RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_GENERIC_RULE_INDEX_2_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 4, 0, 2, v)
#define RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_KEY_MASK_READ(r, p)                             FIELD_MREAD_32((uint8_t *)p + 4, 0, 24, r)
#define RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_KEY_MASK_WRITE(v, p)                            FIELD_MWRITE_32((uint8_t *)p + 4, 0, 24, v)
#if defined OREN

#define RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE     16
typedef struct
{
	RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_DTS	entry[ RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS;

#define RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_PTR()	( RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_ADDRESS )

#endif
#define RDD_PROFILING_BUFFER_PICO_RUNNER_RESERVED_FW_ONLY_NUMBER	64

typedef struct
{
	uint32_t	reserved_fw_only[RDD_PROFILING_BUFFER_PICO_RUNNER_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PROFILING_BUFFER_PICO_RUNNER_DTS;

#if defined OREN

#define RDD_DOWNSTREAM_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_SIZE     16
typedef struct
{
	RDD_EIGHT_BYTES_DTS	entry[ RDD_DOWNSTREAM_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DOWNSTREAM_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_DTS;

#define RDD_DOWNSTREAM_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_PTR()	( RDD_DOWNSTREAM_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DOWNSTREAM_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS )

#endif

typedef struct
{
	uint8_t	optimize_enable	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_VLAN_OPTIMIZATION_ENTRY_DTS;

#define RDD_VLAN_OPTIMIZATION_ENTRY_OPTIMIZE_ENABLE_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_VLAN_OPTIMIZATION_ENTRY_OPTIMIZE_ENABLE_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#if defined OREN

#define RDD_DS_VLAN_OPTIMIZATION_TABLE_SIZE     128
typedef struct
{
	RDD_VLAN_OPTIMIZATION_ENTRY_DTS	entry[ RDD_DS_VLAN_OPTIMIZATION_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_VLAN_OPTIMIZATION_TABLE_DTS;

#define RDD_DS_VLAN_OPTIMIZATION_TABLE_PTR()	( RDD_DS_VLAN_OPTIMIZATION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_VLAN_OPTIMIZATION_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_DOWNSTREAM_MULTICAST_LAN_ENQUEUE_SERVICE_QUEUE_SIZE     16
typedef struct
{
	RDD_EIGHT_BYTES_DTS	entry[ RDD_DOWNSTREAM_MULTICAST_LAN_ENQUEUE_SERVICE_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DOWNSTREAM_MULTICAST_LAN_ENQUEUE_SERVICE_QUEUE_DTS;

#define RDD_DOWNSTREAM_MULTICAST_LAN_ENQUEUE_SERVICE_QUEUE_PTR()	( RDD_DOWNSTREAM_MULTICAST_LAN_ENQUEUE_SERVICE_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DOWNSTREAM_MULTICAST_LAN_ENQUEUE_SERVICE_QUEUE_ADDRESS )

#endif
#define RDD_PCI_MULTICAST_SCRATCHPAD_RESERVED_FW_ONLY_NUMBER	32

typedef struct
{
	uint32_t	reserved_fw_only[RDD_PCI_MULTICAST_SCRATCHPAD_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PCI_MULTICAST_SCRATCHPAD_DTS;

#if defined OREN

#define RDD_CPU_RX_MIRRORING_PD_INGRESS_QUEUE_SIZE     16
typedef struct
{
	RDD_EIGHT_BYTES_DTS	entry[ RDD_CPU_RX_MIRRORING_PD_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_MIRRORING_PD_INGRESS_QUEUE_DTS;

#define RDD_CPU_RX_MIRRORING_PD_INGRESS_QUEUE_PTR()	( RDD_CPU_RX_MIRRORING_PD_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + CPU_RX_MIRRORING_PD_INGRESS_QUEUE_ADDRESS )

#endif
#define RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_ENTRY_RESERVED_FW_ONLY_NUMBER	16

typedef struct
{
	uint16_t	reserved_fw_only[RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_ENTRY_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_ENTRY_DTS;

#if defined OREN

#define RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_SIZE     5
typedef struct
{
	RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_ENTRY_DTS	entry[ RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_DTS;

#define RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_PTR()	( RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + EMAC_SKB_ENQUEUED_INDEXES_FIFO_ADDRESS )

#endif

typedef struct
{
	uint32_t	reserved_fw_only	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FAST_RUNNER_GLOBAL_REGISTERS_INIT_ENTRY_DTS;

#if defined OREN

#define RDD_DS_FAST_RUNNER_GLOBAL_REGISTERS_INIT_SIZE     8
typedef struct
{
	RDD_FAST_RUNNER_GLOBAL_REGISTERS_INIT_ENTRY_DTS	entry[ RDD_DS_FAST_RUNNER_GLOBAL_REGISTERS_INIT_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_FAST_RUNNER_GLOBAL_REGISTERS_INIT_DTS;

#define RDD_DS_FAST_RUNNER_GLOBAL_REGISTERS_INIT_PTR()	( RDD_DS_FAST_RUNNER_GLOBAL_REGISTERS_INIT_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_FAST_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS )

#endif

typedef struct
{
	uint8_t	queue     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_QUEUE_ENTRY_DTS;

#define RDD_QUEUE_ENTRY_QUEUE_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_QUEUE_ENTRY_QUEUE_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#if defined OREN

#define RDD_DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_SIZE     6
#define RDD_DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_SIZE2    8
typedef struct
{
	RDD_QUEUE_ENTRY_DTS	entry[ RDD_DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_SIZE ][ RDD_DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_DTS;

#define RDD_DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_PTR()	( RDD_DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_ADDRESS )

#endif

typedef struct
{
	uint16_t	primitive_address	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_TIMER_SCHEDULER_PRIMITIVE_ENTRY_DTS;

#define RDD_TIMER_SCHEDULER_PRIMITIVE_ENTRY_PRIMITIVE_ADDRESS_READ(r, p)                 MREAD_16((uint8_t *)p, r)
#define RDD_TIMER_SCHEDULER_PRIMITIVE_ENTRY_PRIMITIVE_ADDRESS_WRITE(v, p)                MWRITE_16((uint8_t *)p, v)
#if defined OREN

#define RDD_DS_TIMER_SCHEDULER_PRIMITIVE_TABLE_SIZE     8
typedef struct
{
	RDD_TIMER_SCHEDULER_PRIMITIVE_ENTRY_DTS	entry[ RDD_DS_TIMER_SCHEDULER_PRIMITIVE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_TIMER_SCHEDULER_PRIMITIVE_TABLE_DTS;

#define RDD_DS_TIMER_SCHEDULER_PRIMITIVE_TABLE_PTR()	( RDD_DS_TIMER_SCHEDULER_PRIMITIVE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_TIMER_SCHEDULER_PRIMITIVE_TABLE_ADDRESS )

#endif
#define RDD_RUNNER_FLOW_HEADER_BUFFER_RESERVED_FW_ONLY_NUMBER	32

typedef struct
{
	uint32_t	reserved_fw_only[RDD_RUNNER_FLOW_HEADER_BUFFER_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RUNNER_FLOW_HEADER_BUFFER_DTS;


typedef struct
{
	uint8_t	reserved_fw_only	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ONE_BYTE_DTS;

#if defined OREN

#define RDD_DOWNSTREAM_LAN_ENQUEUE_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_DOWNSTREAM_LAN_ENQUEUE_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DOWNSTREAM_LAN_ENQUEUE_INGRESS_QUEUE_DTS;

#define RDD_DOWNSTREAM_LAN_ENQUEUE_INGRESS_QUEUE_PTR()	( RDD_DOWNSTREAM_LAN_ENQUEUE_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DOWNSTREAM_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS )

#endif

typedef struct
{
	uint8_t	pbits     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DSCP_TO_PBITS_ENTRY_DTS;

#define RDD_DSCP_TO_PBITS_ENTRY_PBITS_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_DSCP_TO_PBITS_ENTRY_PBITS_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#if defined OREN

#define RDD_DS_DSCP_TO_PBITS_TABLE_SIZE2    64
typedef struct
{
	RDD_DSCP_TO_PBITS_ENTRY_DTS	entry[ RDD_DS_DSCP_TO_PBITS_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_DSCP_TO_PBITS_TABLE_DTS;

#define RDD_DS_DSCP_TO_PBITS_TABLE_PTR()	( RDD_DS_DSCP_TO_PBITS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_DSCP_TO_PBITS_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_DOWNSTREAM_LAN_ENQUEUE_SERVICE_QUEUE_SIZE     64
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_DOWNSTREAM_LAN_ENQUEUE_SERVICE_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DOWNSTREAM_LAN_ENQUEUE_SERVICE_QUEUE_DTS;

#define RDD_DOWNSTREAM_LAN_ENQUEUE_SERVICE_QUEUE_PTR()	( RDD_DOWNSTREAM_LAN_ENQUEUE_SERVICE_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DOWNSTREAM_LAN_ENQUEUE_SERVICE_QUEUE_ADDRESS )

#endif

typedef struct
{
	uint16_t	primitive_address	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_VLAN_PRIMITIVE_ENTRY_DTS;

#define RDD_VLAN_PRIMITIVE_ENTRY_PRIMITIVE_ADDRESS_READ(r, p)                 MREAD_16((uint8_t *)p, r)
#define RDD_VLAN_PRIMITIVE_ENTRY_PRIMITIVE_ADDRESS_WRITE(v, p)                MWRITE_16((uint8_t *)p, v)
#if defined OREN

#define RDD_DS_VLAN_PRIMITIVE_TABLE_SIZE     32
typedef struct
{
	RDD_VLAN_PRIMITIVE_ENTRY_DTS	entry[ RDD_DS_VLAN_PRIMITIVE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_VLAN_PRIMITIVE_TABLE_DTS;

#define RDD_DS_VLAN_PRIMITIVE_TABLE_PTR()	( RDD_DS_VLAN_PRIMITIVE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_VLAN_PRIMITIVE_TABLE_ADDRESS )

#endif
#define RDD_PACKET_SRAM_TO_DDR_COPY_BUFFER_RESERVED_FW_ONLY_NUMBER	32

typedef struct
{
	uint32_t	reserved_fw_only[RDD_PACKET_SRAM_TO_DDR_COPY_BUFFER_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PACKET_SRAM_TO_DDR_COPY_BUFFER_DTS;


typedef struct
{
	uint8_t	reserved_fw_only	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INGRESS_QUEUE_ENTRY_DTS;

#if defined OREN

#define RDD_DS_CPU_RX_PICO_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_DS_CPU_RX_PICO_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_CPU_RX_PICO_INGRESS_QUEUE_DTS;

#define RDD_DS_CPU_RX_PICO_INGRESS_QUEUE_PTR()	( RDD_DS_CPU_RX_PICO_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_CPU_RX_PICO_INGRESS_QUEUE_ADDRESS )

#endif
#define RDD_CPU_TX_MESSAGE_DATA_BUFFER_ENTRY_RESERVED_FW_ONLY_NUMBER	16

typedef struct
{
	uint32_t	reserved_fw_only[RDD_CPU_TX_MESSAGE_DATA_BUFFER_ENTRY_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_MESSAGE_DATA_BUFFER_ENTRY_DTS;

#if defined OREN

#define RDD_DOWNSTREAM_MULTICAST_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_DOWNSTREAM_MULTICAST_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DOWNSTREAM_MULTICAST_INGRESS_QUEUE_DTS;

#define RDD_DOWNSTREAM_MULTICAST_INGRESS_QUEUE_PTR()	( RDD_DOWNSTREAM_MULTICAST_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DOWNSTREAM_MULTICAST_INGRESS_QUEUE_ADDRESS )

#endif
#define RDD_VLAN_ACTION_BUFFER_RESERVED_FW_ONLY_NUMBER	16

typedef struct
{
	uint32_t	reserved_fw_only[RDD_VLAN_ACTION_BUFFER_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_VLAN_ACTION_BUFFER_DTS;

#if defined OREN

#define RDD_DS_ROUTER_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_DS_ROUTER_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_ROUTER_INGRESS_QUEUE_DTS;

#define RDD_DS_ROUTER_INGRESS_QUEUE_PTR()	( RDD_DS_ROUTER_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_ROUTER_INGRESS_QUEUE_ADDRESS )

#endif

typedef struct
{
	uint32_t	reserved_fw_only	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PICO_RUNNER_GLOBAL_REGISTERS_INIT_ENTRY_DTS;

#if defined OREN

#define RDD_DS_PICO_RUNNER_GLOBAL_REGISTERS_INIT_SIZE     8
typedef struct
{
	RDD_PICO_RUNNER_GLOBAL_REGISTERS_INIT_ENTRY_DTS	entry[ RDD_DS_PICO_RUNNER_GLOBAL_REGISTERS_INIT_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_PICO_RUNNER_GLOBAL_REGISTERS_INIT_DTS;

#define RDD_DS_PICO_RUNNER_GLOBAL_REGISTERS_INIT_PTR()	( RDD_DS_PICO_RUNNER_GLOBAL_REGISTERS_INIT_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PICO_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS )

#endif
#if defined OREN

#define RDD_DS_DATA_POINTER_DUMMY_TARGET_SIZE     5
typedef struct
{
	RDD_FOUR_BYTES_DTS	entry[ RDD_DS_DATA_POINTER_DUMMY_TARGET_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_DATA_POINTER_DUMMY_TARGET_DTS;

#define RDD_DS_DATA_POINTER_DUMMY_TARGET_PTR()	( RDD_DS_DATA_POINTER_DUMMY_TARGET_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_DATA_POINTER_DUMMY_TARGET_ADDRESS )

#endif

typedef struct
{
	uint32_t	reserved_fw_only	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BPM_DDR_OPTIMIZED_BUFFERS_WITHOUT_HEADROOM_BASE_DTS;


typedef struct
{
	uint32_t	ssid_vector    	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wlan_info      	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = wlan_info, size = 16 bits
	uint32_t	ring_id        	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wl_tx_prio     	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	chain_id       	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wl_metadata    	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	headroom_size  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	src_bridge_port	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dma_sync       	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	type           	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_PARAMETERS_BLOCK_ENTRY_DTS;

#define RDD_CPU_PARAMETERS_BLOCK_ENTRY_SSID_VECTOR_READ(r, p)                     MREAD_16((uint8_t *)p, r)
#define RDD_CPU_PARAMETERS_BLOCK_ENTRY_SSID_VECTOR_WRITE(v, p)                    MWRITE_16((uint8_t *)p, v)
#define RDD_CPU_PARAMETERS_BLOCK_ENTRY_WLAN_INFO_READ(r, p)                       MREAD_16((uint8_t *)p + 2, r)
#define RDD_CPU_PARAMETERS_BLOCK_ENTRY_WLAN_INFO_WRITE(v, p)                      MWRITE_16((uint8_t *)p + 2, v)
#define RDD_CPU_PARAMETERS_BLOCK_ENTRY_RING_ID_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 2, 6, 2, r)
#define RDD_CPU_PARAMETERS_BLOCK_ENTRY_RING_ID_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 2, 6, 2, v)
#define RDD_CPU_PARAMETERS_BLOCK_ENTRY_WL_TX_PRIO_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 2, 0, 6, r)
#define RDD_CPU_PARAMETERS_BLOCK_ENTRY_WL_TX_PRIO_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 2, 0, 6, v)
#define RDD_CPU_PARAMETERS_BLOCK_ENTRY_CHAIN_ID_READ(r, p)                        MREAD_8((uint8_t *)p + 3, r)
#define RDD_CPU_PARAMETERS_BLOCK_ENTRY_CHAIN_ID_WRITE(v, p)                       MWRITE_8((uint8_t *)p + 3, v)
#define RDD_CPU_PARAMETERS_BLOCK_ENTRY_WL_METADATA_READ(r, p)                     MREAD_16((uint8_t *)p + 2, r)
#define RDD_CPU_PARAMETERS_BLOCK_ENTRY_WL_METADATA_WRITE(v, p)                    MWRITE_16((uint8_t *)p + 2, v)
#define RDD_CPU_PARAMETERS_BLOCK_ENTRY_HEADROOM_SIZE_READ(r, p)                   MREAD_8((uint8_t *)p + 4, r)
#define RDD_CPU_PARAMETERS_BLOCK_ENTRY_HEADROOM_SIZE_WRITE(v, p)                  MWRITE_8((uint8_t *)p + 4, v)
#define RDD_CPU_PARAMETERS_BLOCK_ENTRY_SRC_BRIDGE_PORT_READ(r, p)                 MREAD_8((uint8_t *)p + 5, r)
#define RDD_CPU_PARAMETERS_BLOCK_ENTRY_SRC_BRIDGE_PORT_WRITE(v, p)                MWRITE_8((uint8_t *)p + 5, v)
#define RDD_CPU_PARAMETERS_BLOCK_ENTRY_DMA_SYNC_READ(r, p)                        MREAD_8((uint8_t *)p + 6, r)
#define RDD_CPU_PARAMETERS_BLOCK_ENTRY_DMA_SYNC_WRITE(v, p)                       MWRITE_8((uint8_t *)p + 6, v)
#define RDD_CPU_PARAMETERS_BLOCK_ENTRY_TYPE_READ(r, p)                            MREAD_8((uint8_t *)p + 7, r)
#define RDD_CPU_PARAMETERS_BLOCK_ENTRY_TYPE_WRITE(v, p)                           MWRITE_8((uint8_t *)p + 7, v)
#if defined OREN

#define RDD_DS_CPU_RX_FAST_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_DS_CPU_RX_FAST_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_CPU_RX_FAST_INGRESS_QUEUE_DTS;

#define RDD_DS_CPU_RX_FAST_INGRESS_QUEUE_PTR()	( RDD_DS_CPU_RX_FAST_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_CPU_RX_FAST_INGRESS_QUEUE_ADDRESS )

#endif

typedef struct
{
	uint16_t	ownership 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	reserved  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	skb_index 	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FREE_SKB_INDEXES_FIFO_ENTRY_DTS;

#define RDD_FREE_SKB_INDEXES_FIFO_ENTRY_OWNERSHIP_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_FREE_SKB_INDEXES_FIFO_ENTRY_OWNERSHIP_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_FREE_SKB_INDEXES_FIFO_ENTRY_SKB_INDEX_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p, 0, 14, r)
#define RDD_FREE_SKB_INDEXES_FIFO_ENTRY_SKB_INDEX_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p, 0, 14, v)

#define RDD_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_SIZE     8
typedef struct
{
	RDD_FREE_SKB_INDEXES_FIFO_ENTRY_DTS	entry[ RDD_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_DTS;

#define RDD_DS_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_PTR()	( RDD_DS_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_ADDRESS )

#if defined OREN

#define RDD_EMAC_SKB_ENQUEUED_INDEXES_PUT_PTR_SIZE     8
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_EMAC_SKB_ENQUEUED_INDEXES_PUT_PTR_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EMAC_SKB_ENQUEUED_INDEXES_PUT_PTR_DTS;

#define RDD_EMAC_SKB_ENQUEUED_INDEXES_PUT_PTR_PTR()	( RDD_EMAC_SKB_ENQUEUED_INDEXES_PUT_PTR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + EMAC_SKB_ENQUEUED_INDEXES_PUT_PTR_ADDRESS )

#endif

#define RDD_PICO_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_SIZE     8
typedef struct
{
	RDD_FREE_SKB_INDEXES_FIFO_ENTRY_DTS	entry[ RDD_PICO_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PICO_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_DTS;

#define RDD_DS_PICO_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_PTR()	( RDD_DS_PICO_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PICO_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_ADDRESS )

#if defined OREN

#define RDD_EMAC_SKB_ENQUEUED_INDEXES_FREE_PTR_SIZE     8
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_EMAC_SKB_ENQUEUED_INDEXES_FREE_PTR_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EMAC_SKB_ENQUEUED_INDEXES_FREE_PTR_DTS;

#define RDD_EMAC_SKB_ENQUEUED_INDEXES_FREE_PTR_PTR()	( RDD_EMAC_SKB_ENQUEUED_INDEXES_FREE_PTR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + EMAC_SKB_ENQUEUED_INDEXES_FREE_PTR_ADDRESS )

#endif
#if defined OREN

#define RDD_CPU_FLOW_CACHE_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_CPU_FLOW_CACHE_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_FLOW_CACHE_INGRESS_QUEUE_DTS;

#define RDD_CPU_FLOW_CACHE_INGRESS_QUEUE_PTR()	( RDD_CPU_FLOW_CACHE_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + CPU_FLOW_CACHE_INGRESS_QUEUE_ADDRESS )

#endif

typedef struct
{
	uint32_t	reserved_fw_only	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DEBUG_BUFFER_ENTRY_DTS;

#if defined OREN

#define RDD_DS_DEBUG_BUFFER_SIZE     32
typedef struct
{
	RDD_DEBUG_BUFFER_ENTRY_DTS	entry[ RDD_DS_DEBUG_BUFFER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_DEBUG_BUFFER_DTS;

#define RDD_DS_DEBUG_BUFFER_PTR()	( RDD_DS_DEBUG_BUFFER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_DEBUG_BUFFER_ADDRESS )

#endif
#if defined OREN

#define RDD_DS_IPTV_SBPM_REPLICATION_BN_SIZE     16
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_DS_IPTV_SBPM_REPLICATION_BN_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_IPTV_SBPM_REPLICATION_BN_DTS;

#define RDD_DS_IPTV_SBPM_REPLICATION_BN_PTR()	( RDD_DS_IPTV_SBPM_REPLICATION_BN_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_IPTV_SBPM_REPLICATION_BN_ADDRESS )

#endif
#define RDD_HASH_BUFFER_RESERVED_FW_ONLY_NUMBER	4

typedef struct
{
	uint32_t	reserved_fw_only[RDD_HASH_BUFFER_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_HASH_BUFFER_DTS;

#define RDD_IPTV_DMA_LKP_KEY_RESERVED_FW_ONLY_NUMBER	4

typedef struct
{
	uint32_t	reserved_fw_only[RDD_IPTV_DMA_LKP_KEY_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_DMA_LKP_KEY_DTS;

#define RDD_MULTICAST_HEADER_BUFFER_RESERVED_FW_ONLY_NUMBER	8

typedef struct
{
	uint32_t	reserved_fw_only[RDD_MULTICAST_HEADER_BUFFER_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MULTICAST_HEADER_BUFFER_DTS;

#if defined OREN

#define RDD_DS_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_CAM_SIZE     8
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_DS_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_CAM_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_CAM_DTS;

#define RDD_DS_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_CAM_PTR()	( RDD_DS_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_CAM_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_CAM_ADDRESS )

#endif
#define RDD_IPV6_ENTRY_IPV6_NUMBER	16

typedef struct
{
	uint8_t	ipv6[RDD_IPV6_ENTRY_IPV6_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPV6_ENTRY_DTS;

#define RDD_IPV6_ENTRY_IPV6_READ(r, p, i)              MREAD_I_8((uint8_t *)p, i, r)
#define RDD_IPV6_ENTRY_IPV6_WRITE(v, p, i)             MWRITE_I_8((uint8_t *)p, i, v)

typedef struct
{
	uint32_t	ingress_filters                 	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = ingress_filters, size = 32 bits
	uint32_t	reserved1                       	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	acl_layer3_filter               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	acl_layer2_filter               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	src_mac_anti_spoofing_lookup    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tpid_detect_filter              	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_fragment_ingress_filter_trap 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_fragment_ingress_filter      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	header_error_ingress_filter_trap	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	header_error_ingress_filter     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_validation_filter            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ingress_filters_bypass          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0                       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vlan_switching_filter           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vlan_us_aggregation_filter      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	timing_1588_ingress_filter      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2                       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_mac_lookup                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	src_mac_lookup                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	local_switching_ingress_filters 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	multicast_filter                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	broadcast_filter                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ethertype_filter                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	icmpv6_filter                   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	igmp_filter                     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mld_ingress_filter              	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dhcp_ingress_filter             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS;

#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_READ(r, p)                                  MREAD_32((uint8_t *)p, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_WRITE(v, p)                                 MWRITE_32((uint8_t *)p, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_L_READ( wv )                                FIELD_GET( wv, 0, 32 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_L_WRITE( v, wv )                            FIELD_SET( v, 0, 32, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ACL_LAYER3_FILTER_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p, 0, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ACL_LAYER3_FILTER_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p, 0, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ACL_LAYER3_FILTER_L_READ( wv )                              FIELD_GET( wv, 24, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ACL_LAYER3_FILTER_L_WRITE( v, wv )                          FIELD_SET( v, 24, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ACL_LAYER2_FILTER_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 1, 7, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ACL_LAYER2_FILTER_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 1, 7, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ACL_LAYER2_FILTER_L_READ( wv )                              FIELD_GET( wv, 23, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ACL_LAYER2_FILTER_L_WRITE( v, wv )                          FIELD_SET( v, 23, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_SRC_MAC_ANTI_SPOOFING_LOOKUP_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 1, 6, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_SRC_MAC_ANTI_SPOOFING_LOOKUP_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 1, 6, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_SRC_MAC_ANTI_SPOOFING_LOOKUP_L_READ( wv )                   FIELD_GET( wv, 22, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_SRC_MAC_ANTI_SPOOFING_LOOKUP_L_WRITE( v, wv )               FIELD_SET( v, 22, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_TPID_DETECT_FILTER_READ(r, p)                               FIELD_MREAD_8((uint8_t *)p + 1, 5, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_TPID_DETECT_FILTER_WRITE(v, p)                              FIELD_MWRITE_8((uint8_t *)p + 1, 5, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_TPID_DETECT_FILTER_L_READ( wv )                             FIELD_GET( wv, 21, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_TPID_DETECT_FILTER_L_WRITE( v, wv )                         FIELD_SET( v, 21, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_TRAP_READ(r, p)                  FIELD_MREAD_8((uint8_t *)p + 1, 4, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_TRAP_WRITE(v, p)                 FIELD_MWRITE_8((uint8_t *)p + 1, 4, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_TRAP_L_READ( wv )                FIELD_GET( wv, 20, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_TRAP_L_WRITE( v, wv )            FIELD_SET( v, 20, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 1, 3, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 1, 3, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_L_READ( wv )                     FIELD_GET( wv, 19, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_FRAGMENT_INGRESS_FILTER_L_WRITE( v, wv )                 FIELD_SET( v, 19, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_TRAP_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 1, 2, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_TRAP_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 1, 2, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_TRAP_L_READ( wv )               FIELD_GET( wv, 18, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_TRAP_L_WRITE( v, wv )           FIELD_SET( v, 18, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 1, 1, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 1, 1, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_L_READ( wv )                    FIELD_GET( wv, 17, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_HEADER_ERROR_INGRESS_FILTER_L_WRITE( v, wv )                FIELD_SET( v, 17, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_VALIDATION_FILTER_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 1, 0, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_VALIDATION_FILTER_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 1, 0, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_VALIDATION_FILTER_L_READ( wv )                           FIELD_GET( wv, 16, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IP_VALIDATION_FILTER_L_WRITE( v, wv )                       FIELD_SET( v, 16, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 2, 7, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 2, 7, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_READ( wv )                         FIELD_GET( wv, 15, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_INGRESS_FILTERS_BYPASS_L_WRITE( v, wv )                     FIELD_SET( v, 15, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_VLAN_SWITCHING_FILTER_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 2, 5, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_VLAN_SWITCHING_FILTER_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 2, 5, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_VLAN_SWITCHING_FILTER_L_READ( wv )                          FIELD_GET( wv, 13, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_VLAN_SWITCHING_FILTER_L_WRITE( v, wv )                      FIELD_SET( v, 13, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_VLAN_US_AGGREGATION_FILTER_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 2, 4, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_VLAN_US_AGGREGATION_FILTER_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 2, 4, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_VLAN_US_AGGREGATION_FILTER_L_READ( wv )                     FIELD_GET( wv, 12, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_VLAN_US_AGGREGATION_FILTER_L_WRITE( v, wv )                 FIELD_SET( v, 12, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_TIMING_1588_INGRESS_FILTER_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 2, 3, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_TIMING_1588_INGRESS_FILTER_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 2, 3, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_TIMING_1588_INGRESS_FILTER_L_READ( wv )                     FIELD_GET( wv, 11, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_TIMING_1588_INGRESS_FILTER_L_WRITE( v, wv )                 FIELD_SET( v, 11, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DST_MAC_LOOKUP_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p + 2, 1, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DST_MAC_LOOKUP_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p + 2, 1, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DST_MAC_LOOKUP_L_READ( wv )                                 FIELD_GET( wv, 9, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DST_MAC_LOOKUP_L_WRITE( v, wv )                             FIELD_SET( v, 9, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_SRC_MAC_LOOKUP_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p + 2, 0, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_SRC_MAC_LOOKUP_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p + 2, 0, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_SRC_MAC_LOOKUP_L_READ( wv )                                 FIELD_GET( wv, 8, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_SRC_MAC_LOOKUP_L_WRITE( v, wv )                             FIELD_SET( v, 8, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_READ(r, p)                  FIELD_MREAD_8((uint8_t *)p + 3, 7, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_WRITE(v, p)                 FIELD_MWRITE_8((uint8_t *)p + 3, 7, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_L_READ( wv )                FIELD_GET( wv, 7, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_LOCAL_SWITCHING_INGRESS_FILTERS_L_WRITE( v, wv )            FIELD_SET( v, 7, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MULTICAST_FILTER_READ(r, p)                                 FIELD_MREAD_8((uint8_t *)p + 3, 6, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MULTICAST_FILTER_WRITE(v, p)                                FIELD_MWRITE_8((uint8_t *)p + 3, 6, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MULTICAST_FILTER_L_READ( wv )                               FIELD_GET( wv, 6, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MULTICAST_FILTER_L_WRITE( v, wv )                           FIELD_SET( v, 6, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_READ(r, p)                                 FIELD_MREAD_8((uint8_t *)p + 3, 5, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_WRITE(v, p)                                FIELD_MWRITE_8((uint8_t *)p + 3, 5, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_L_READ( wv )                               FIELD_GET( wv, 5, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_BROADCAST_FILTER_L_WRITE( v, wv )                           FIELD_SET( v, 5, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_READ(r, p)                                 FIELD_MREAD_8((uint8_t *)p + 3, 4, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_WRITE(v, p)                                FIELD_MWRITE_8((uint8_t *)p + 3, 4, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_L_READ( wv )                               FIELD_GET( wv, 4, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ETHERTYPE_FILTER_L_WRITE( v, wv )                           FIELD_SET( v, 4, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_READ(r, p)                                    FIELD_MREAD_8((uint8_t *)p + 3, 3, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_WRITE(v, p)                                   FIELD_MWRITE_8((uint8_t *)p + 3, 3, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_L_READ( wv )                                  FIELD_GET( wv, 3, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_ICMPV6_FILTER_L_WRITE( v, wv )                              FIELD_SET( v, 3, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_READ(r, p)                                      FIELD_MREAD_8((uint8_t *)p + 3, 2, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_WRITE(v, p)                                     FIELD_MWRITE_8((uint8_t *)p + 3, 2, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_L_READ( wv )                                    FIELD_GET( wv, 2, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_IGMP_FILTER_L_WRITE( v, wv )                                FIELD_SET( v, 2, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_READ(r, p)                               FIELD_MREAD_8((uint8_t *)p + 3, 1, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_WRITE(v, p)                              FIELD_MWRITE_8((uint8_t *)p + 3, 1, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_READ( wv )                             FIELD_GET( wv, 1, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_MLD_INGRESS_FILTER_L_WRITE( v, wv )                         FIELD_SET( v, 1, 1, wv )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DHCP_INGRESS_FILTER_READ(r, p)                              FIELD_MREAD_8((uint8_t *)p + 3, 0, 1, r)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DHCP_INGRESS_FILTER_WRITE(v, p)                             FIELD_MWRITE_8((uint8_t *)p + 3, 0, 1, v)
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DHCP_INGRESS_FILTER_L_READ( wv )                            FIELD_GET( wv, 0, 1 )
#define RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DHCP_INGRESS_FILTER_L_WRITE( v, wv )                        FIELD_SET( v, 0, 1, wv )
#if defined OREN

#define RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_SIZE     3
typedef struct
{
	RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS	entry[ RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS;

#define RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR()	( RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_FILTERS_CONFIGURATION_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	reserved_fw_only	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BPM_EXTRA_DDR_BUFFERS_BASE_DTS;


typedef struct
{
	uint8_t	cpu_trap  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	drop      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	parameter 	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DTS;

#define RDD_LAYER4_FILTERS_CONTEXT_ENTRY_CPU_TRAP_READ(r, p)                  FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_LAYER4_FILTERS_CONTEXT_ENTRY_CPU_TRAP_WRITE(v, p)                 FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p, 6, 1, r)
#define RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DROP_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p, 6, 1, v)
#define RDD_LAYER4_FILTERS_CONTEXT_ENTRY_PARAMETER_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 0, 6, r)
#define RDD_LAYER4_FILTERS_CONTEXT_ENTRY_PARAMETER_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 0, 6, v)
#if defined OREN

#define RDD_DS_LAYER4_FILTERS_CONTEXT_TABLE_SIZE     16
typedef struct
{
	RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DTS	entry[ RDD_DS_LAYER4_FILTERS_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_LAYER4_FILTERS_CONTEXT_TABLE_DTS;

#define RDD_DS_LAYER4_FILTERS_CONTEXT_TABLE_PTR()	( RDD_DS_LAYER4_FILTERS_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_LAYER4_FILTERS_CONTEXT_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	rules_map_table_address	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rules_table_address    	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FIREWALL_CONFIGURATION_REGISTER_ENTRY_DTS;

#define RDD_FIREWALL_CONFIGURATION_REGISTER_ENTRY_RULES_MAP_TABLE_ADDRESS_READ(r, p)                 MREAD_32((uint8_t *)p, r)
#define RDD_FIREWALL_CONFIGURATION_REGISTER_ENTRY_RULES_MAP_TABLE_ADDRESS_WRITE(v, p)                MWRITE_32((uint8_t *)p, v)
#define RDD_FIREWALL_CONFIGURATION_REGISTER_ENTRY_RULES_TABLE_ADDRESS_READ(r, p)                     MREAD_32((uint8_t *)p + 4, r)
#define RDD_FIREWALL_CONFIGURATION_REGISTER_ENTRY_RULES_TABLE_ADDRESS_WRITE(v, p)                    MWRITE_32((uint8_t *)p + 4, v)
#define RDD_IPTV_DMA_RW_BUFFER_RESERVED_FW_ONLY_NUMBER	4

typedef struct
{
	uint32_t	reserved_fw_only[RDD_IPTV_DMA_RW_BUFFER_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_DMA_RW_BUFFER_DTS;

#define RDD_ETH_TX_QUEUE_DUMMY_DESCRIPTOR_RESERVED_FW_ONLY_NUMBER	4

typedef struct
{
	uint32_t	reserved_fw_only[RDD_ETH_TX_QUEUE_DUMMY_DESCRIPTOR_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_QUEUE_DUMMY_DESCRIPTOR_DTS;

#define RDD_RUNNER_FLOW_HEADER_DESCRIPTOR_RESERVED_FW_ONLY_NUMBER	2

typedef struct
{
	uint32_t	reserved_fw_only[RDD_RUNNER_FLOW_HEADER_DESCRIPTOR_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RUNNER_FLOW_HEADER_DESCRIPTOR_DTS;


typedef struct
{
	uint8_t	bridge_port	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MULTICAST_VECTOR_TO_PORT_ENTRY_DTS;

#define RDD_MULTICAST_VECTOR_TO_PORT_ENTRY_BRIDGE_PORT_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_MULTICAST_VECTOR_TO_PORT_ENTRY_BRIDGE_PORT_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#if defined OREN

#define RDD_DS_MULTICAST_VECTOR_TO_PORT_TABLE_SIZE     8
typedef struct
{
	RDD_MULTICAST_VECTOR_TO_PORT_ENTRY_DTS	entry[ RDD_DS_MULTICAST_VECTOR_TO_PORT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_MULTICAST_VECTOR_TO_PORT_TABLE_DTS;

#define RDD_DS_MULTICAST_VECTOR_TO_PORT_TABLE_PTR()	( RDD_DS_MULTICAST_VECTOR_TO_PORT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_MULTICAST_VECTOR_TO_PORT_TABLE_ADDRESS )

#endif
#define RDD_SC_BUFFER_RESERVED_FW_ONLY_NUMBER	3

typedef struct
{
	uint32_t	reserved_fw_only[RDD_SC_BUFFER_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SC_BUFFER_DTS;


typedef struct
{
	uint32_t	reserved_fw_only	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_SSM_CONTEXT_TABLE_PTR_DTS;

#define RDD_MULTICAST_DUMMY_VLAN_INDEXES_TABLE_RESERVED_FW_ONLY_NUMBER	2

typedef struct
{
	uint32_t	reserved_fw_only[RDD_MULTICAST_DUMMY_VLAN_INDEXES_TABLE_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MULTICAST_DUMMY_VLAN_INDEXES_TABLE_DTS;


typedef struct
{
	uint32_t	reserved_fw_only	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_PTR_ENTRY_DTS;


typedef struct
{
	uint32_t	reserved_fw_only	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_CONTEXT_PTR_ENTRY_DTS;


typedef struct
{
	uint32_t	reserved_fw_only	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BPM_DDR_OPTIMIZED_BUFFERS_BASE_DTS;


typedef struct
{
	uint32_t	reserved_fw_only	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BPM_DDR_BUFFERS_BASE_DTS;


typedef struct
{
	uint32_t	reserved_fw_only	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CONNECTION_TABLE_CONFIG_DTS;


typedef struct
{
	uint32_t	reserved_fw_only	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CONTEXT_TABLE_CONFIG_DTS;

#if defined OREN

typedef struct
{
	uint8_t	ih_buffer 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PARALLEL_PROCESSING_ENTRY_DTS;

#define RDD_PARALLEL_PROCESSING_ENTRY_IH_BUFFER_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_PARALLEL_PROCESSING_ENTRY_IH_BUFFER_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#endif
#if defined OREN

#define RDD_DS_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_SIZE     4
typedef struct
{
	RDD_PARALLEL_PROCESSING_ENTRY_DTS	entry[ RDD_DS_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_DTS;

#define RDD_DS_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_PTR()	( RDD_DS_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS )

#endif
#if defined OREN

typedef struct
{
	uint8_t	reserved_fw_only	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PARALLEL_PROCESSING_TASK_REORDER_ENTRY_DTS;

#endif
#if defined OREN

#define RDD_DS_PARALLEL_PROCESSING_TASK_REORDER_FIFO_SIZE     4
typedef struct
{
	RDD_PARALLEL_PROCESSING_TASK_REORDER_ENTRY_DTS	entry[ RDD_DS_PARALLEL_PROCESSING_TASK_REORDER_FIFO_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_PARALLEL_PROCESSING_TASK_REORDER_FIFO_DTS;

#define RDD_DS_PARALLEL_PROCESSING_TASK_REORDER_FIFO_PTR()	( RDD_DS_PARALLEL_PROCESSING_TASK_REORDER_FIFO_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PARALLEL_PROCESSING_TASK_REORDER_FIFO_ADDRESS )

#endif
#if defined OREN

#define RDD_DS_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_TABLE_SIZE     8
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_DS_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_TABLE_DTS;

#define RDD_DS_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_TABLE_PTR()	( RDD_DS_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_COUNTERS_SIZE     5
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_COUNTERS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_COUNTERS_DTS;

#define RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_COUNTERS_PTR()	( RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_COUNTERS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + EMAC_SKB_ENQUEUED_INDEXES_FIFO_COUNTERS_ADDRESS )

#endif
#if defined OREN

typedef struct
{
	uint8_t	status_vector	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_EMACS_STATUS_ENTRY_DTS;

#define RDD_ETH_TX_EMACS_STATUS_ENTRY_STATUS_VECTOR_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_ETH_TX_EMACS_STATUS_ENTRY_STATUS_VECTOR_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#endif

typedef struct
{
	uint16_t	number_of_active_tasks	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_TIMER_CONTROL_DESCRIPTOR_DTS;

#define RDD_TIMER_CONTROL_DESCRIPTOR_NUMBER_OF_ACTIVE_TASKS_READ(r, p)                 MREAD_16((uint8_t *)p, r)
#define RDD_TIMER_CONTROL_DESCRIPTOR_NUMBER_OF_ACTIVE_TASKS_WRITE(v, p)                MWRITE_16((uint8_t *)p, v)

typedef struct
{
	uint8_t	egress_port	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_HASH_BASED_FORWARDING_PORT_ENTRY_DTS;

#define RDD_HASH_BASED_FORWARDING_PORT_ENTRY_EGRESS_PORT_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_HASH_BASED_FORWARDING_PORT_ENTRY_EGRESS_PORT_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#if defined OREN

#define RDD_HASH_BASED_FORWARDING_PORT_TABLE_SIZE     4
typedef struct
{
	RDD_HASH_BASED_FORWARDING_PORT_ENTRY_DTS	entry[ RDD_HASH_BASED_FORWARDING_PORT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_HASH_BASED_FORWARDING_PORT_TABLE_DTS;

#define RDD_HASH_BASED_FORWARDING_PORT_TABLE_PTR()	( RDD_HASH_BASED_FORWARDING_PORT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + HASH_BASED_FORWARDING_PORT_TABLE_ADDRESS )

#endif

typedef struct
{
	uint16_t	reserved_fw_only	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_COUNTERS_BUFFER_DTS;


typedef struct
{
	uint32_t	reserved_fw_only	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FIREWALL_IPV6_R16_BUFFER_ENTRY_DTS;


typedef struct
{
	uint16_t	reserved_fw_only	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_PICO_INGRESS_QUEUE_PTR_DTS;


typedef struct
{
	uint16_t	reserved_fw_only	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FREE_PACKET_DESCRIPTORS_POOL_THRESHOLD_DTS;


typedef struct
{
	uint16_t	reserved_fw_only	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_PICO_INGRESS_QUEUE_PTR_DTS;


typedef struct
{
	uint16_t	reserved_fw_only	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BPM_DDR_BUFFER_HEADROOM_SIZE_DTS;


typedef struct
{
	uint16_t	reserved_fw_only	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BPM_DDR_BUFFER_HEADROOM_SIZE_2_BYTE_RESOLUTION_DTS;


typedef struct
{
	uint16_t	reserved_fw_only	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_FAST_INGRESS_QUEUE_PTR_DTS;


typedef struct
{
	uint32_t	reserved_fw_only	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MICROCODE_VERSION_ENTRY_DTS;


typedef struct
{
	uint32_t	reserved_fw_only	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RATE_LIMITER_COUNTER_BUFFER_ENTRY_DTS;

#if defined OREN

typedef struct
{
	uint16_t	reserved_fw_only	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PARALLEL_PROCESSING_IH_BUFFER_PTR_DTS;

#endif

typedef struct
{
	uint16_t	reserved_fw_only	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RUNNER_CONGESTION_STATE_ENTRY_DTS;


typedef struct
{
	uint16_t	reserved_fw_only	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FIREWALL_RULE_MAP_ENTRY_BUFFER_DTS;


typedef struct
{
	uint8_t	dummy_store	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DUMMY_STORE_ENTRY_DTS;

#define RDD_DUMMY_STORE_ENTRY_DUMMY_STORE_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_DUMMY_STORE_ENTRY_DUMMY_STORE_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#if defined OREN

typedef struct
{
	uint8_t	emac_offset	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_INTER_LAN_SCHEDULING_OFFSET_ENTRY_DTS;

#define RDD_ETH_TX_INTER_LAN_SCHEDULING_OFFSET_ENTRY_EMAC_OFFSET_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_ETH_TX_INTER_LAN_SCHEDULING_OFFSET_ENTRY_EMAC_OFFSET_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#endif
#if defined OREN

typedef struct
{
	uint8_t	reserved0       	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	available_slave3	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	available_slave2	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	available_slave1	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	available_slave0	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_DTS;

#define RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_AVAILABLE_SLAVE3_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 3, 1, r)
#define RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_AVAILABLE_SLAVE3_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 3, 1, v)
#define RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_AVAILABLE_SLAVE2_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 2, 1, r)
#define RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_AVAILABLE_SLAVE2_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 2, 1, v)
#define RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_AVAILABLE_SLAVE1_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 1, 1, r)
#define RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_AVAILABLE_SLAVE1_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 1, 1, v)
#define RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_AVAILABLE_SLAVE0_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 0, 1, r)
#define RDD_PARALLEL_PROCESSING_SLAVE_VECTOR_AVAILABLE_SLAVE0_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 0, 1, v)
#endif

typedef struct
{
	uint32_t	reserved0 	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1 	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_DESCRIPTOR_DTS;


typedef struct
{
	uint32_t	valid             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	command           	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ds_drop_precedence	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	emac              	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tx_queue          	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	src_bridge_port   	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	packet_length     	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs_flag          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ssid              	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ih_class          	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	payload_offset    	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	gso               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2         	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_DESCRIPTOR_CORE_DTS;

#define RDD_CPU_TX_DESCRIPTOR_CORE_VALID_READ(r, p)                              FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_CORE_VALID_WRITE(v, p)                             FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_CORE_VALID_L_READ( wv )                            FIELD_GET( wv, 31, 1 )
#define RDD_CPU_TX_DESCRIPTOR_CORE_VALID_L_WRITE( v, wv )                        FIELD_SET( v, 31, 1, wv )
#define RDD_CPU_TX_DESCRIPTOR_CORE_COMMAND_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p, 4, 3, r)
#define RDD_CPU_TX_DESCRIPTOR_CORE_COMMAND_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p, 4, 3, v)
#define RDD_CPU_TX_DESCRIPTOR_CORE_COMMAND_L_READ( wv )                          FIELD_GET( wv, 28, 3 )
#define RDD_CPU_TX_DESCRIPTOR_CORE_COMMAND_L_WRITE( v, wv )                      FIELD_SET( v, 28, 3, wv )
#define RDD_CPU_TX_DESCRIPTOR_CORE_DS_DROP_PRECEDENCE_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 3, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_CORE_DS_DROP_PRECEDENCE_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 3, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_CORE_DS_DROP_PRECEDENCE_L_READ( wv )               FIELD_GET( wv, 27, 1 )
#define RDD_CPU_TX_DESCRIPTOR_CORE_DS_DROP_PRECEDENCE_L_WRITE( v, wv )           FIELD_SET( v, 27, 1, wv )
#define RDD_CPU_TX_DESCRIPTOR_CORE_EMAC_READ(r, p)                               FIELD_MREAD_16((uint8_t *)p, 6, 5, r)
#define RDD_CPU_TX_DESCRIPTOR_CORE_EMAC_WRITE(v, p)                              FIELD_MWRITE_16((uint8_t *)p, 6, 5, v)
#define RDD_CPU_TX_DESCRIPTOR_CORE_EMAC_L_READ( wv )                             FIELD_GET( wv, 22, 5 )
#define RDD_CPU_TX_DESCRIPTOR_CORE_EMAC_L_WRITE( v, wv )                         FIELD_SET( v, 22, 5, wv )
#define RDD_CPU_TX_DESCRIPTOR_CORE_TX_QUEUE_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 1, 3, 3, r)
#define RDD_CPU_TX_DESCRIPTOR_CORE_TX_QUEUE_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 1, 3, 3, v)
#define RDD_CPU_TX_DESCRIPTOR_CORE_TX_QUEUE_L_READ( wv )                         FIELD_GET( wv, 19, 3 )
#define RDD_CPU_TX_DESCRIPTOR_CORE_TX_QUEUE_L_WRITE( v, wv )                     FIELD_SET( v, 19, 3, wv )
#define RDD_CPU_TX_DESCRIPTOR_CORE_SRC_BRIDGE_PORT_READ(r, p)                    FIELD_MREAD_32((uint8_t *)p + 0, 14, 5, r)
#define RDD_CPU_TX_DESCRIPTOR_CORE_SRC_BRIDGE_PORT_WRITE(v, p)                   FIELD_MWRITE_32((uint8_t *)p + 0, 14, 5, v)
#define RDD_CPU_TX_DESCRIPTOR_CORE_SRC_BRIDGE_PORT_L_READ( wv )                  FIELD_GET( wv, 14, 5 )
#define RDD_CPU_TX_DESCRIPTOR_CORE_SRC_BRIDGE_PORT_L_WRITE( v, wv )              FIELD_SET( v, 14, 5, wv )
#define RDD_CPU_TX_DESCRIPTOR_CORE_PACKET_LENGTH_READ(r, p)                      FIELD_MREAD_16((uint8_t *)p + 2, 0, 14, r)
#define RDD_CPU_TX_DESCRIPTOR_CORE_PACKET_LENGTH_WRITE(v, p)                     FIELD_MWRITE_16((uint8_t *)p + 2, 0, 14, v)
#define RDD_CPU_TX_DESCRIPTOR_CORE_PACKET_LENGTH_L_READ( wv )                    FIELD_GET( wv, 0, 14 )
#define RDD_CPU_TX_DESCRIPTOR_CORE_PACKET_LENGTH_L_WRITE( v, wv )                FIELD_SET( v, 0, 14, wv )
#define RDD_CPU_TX_DESCRIPTOR_CORE_ABS_FLAG_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_CORE_ABS_FLAG_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 4, 7, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_CORE_ABS_FLAG_L_READ( wv )                         FIELD_GET( wv, 31, 1 )
#define RDD_CPU_TX_DESCRIPTOR_CORE_ABS_FLAG_L_WRITE( v, wv )                     FIELD_SET( v, 31, 1, wv )
#define RDD_CPU_TX_DESCRIPTOR_CORE_SSID_READ(r, p)                               FIELD_MREAD_8((uint8_t *)p + 4, 3, 4, r)
#define RDD_CPU_TX_DESCRIPTOR_CORE_SSID_WRITE(v, p)                              FIELD_MWRITE_8((uint8_t *)p + 4, 3, 4, v)
#define RDD_CPU_TX_DESCRIPTOR_CORE_SSID_L_READ( wv )                             FIELD_GET( wv, 27, 4 )
#define RDD_CPU_TX_DESCRIPTOR_CORE_SSID_L_WRITE( v, wv )                         FIELD_SET( v, 27, 4, wv )
#define RDD_CPU_TX_DESCRIPTOR_CORE_IH_CLASS_READ(r, p)                           FIELD_MREAD_16((uint8_t *)p + 4, 7, 4, r)
#define RDD_CPU_TX_DESCRIPTOR_CORE_IH_CLASS_WRITE(v, p)                          FIELD_MWRITE_16((uint8_t *)p + 4, 7, 4, v)
#define RDD_CPU_TX_DESCRIPTOR_CORE_IH_CLASS_L_READ( wv )                         FIELD_GET( wv, 23, 4 )
#define RDD_CPU_TX_DESCRIPTOR_CORE_IH_CLASS_L_WRITE( v, wv )                     FIELD_SET( v, 23, 4, wv )
#define RDD_CPU_TX_DESCRIPTOR_CORE_PAYLOAD_OFFSET_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 5, 0, 7, r)
#define RDD_CPU_TX_DESCRIPTOR_CORE_PAYLOAD_OFFSET_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 5, 0, 7, v)
#define RDD_CPU_TX_DESCRIPTOR_CORE_PAYLOAD_OFFSET_L_READ( wv )                   FIELD_GET( wv, 16, 7 )
#define RDD_CPU_TX_DESCRIPTOR_CORE_PAYLOAD_OFFSET_L_WRITE( v, wv )               FIELD_SET( v, 16, 7, wv )
#define RDD_CPU_TX_DESCRIPTOR_CORE_GSO_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 6, 6, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_CORE_GSO_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 6, 6, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_CORE_GSO_L_READ( wv )                              FIELD_GET( wv, 14, 1 )
#define RDD_CPU_TX_DESCRIPTOR_CORE_GSO_L_WRITE( v, wv )                          FIELD_SET( v, 14, 1, wv )

typedef struct
{
	uint32_t	reserved0    	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1    	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	buffer_number	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_DESCRIPTOR_BPM_DTS;

#define RDD_CPU_TX_DESCRIPTOR_BPM_BUFFER_NUMBER_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r)
#define RDD_CPU_TX_DESCRIPTOR_BPM_BUFFER_NUMBER_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p + 6, 0, 14, v)
#define RDD_CPU_TX_DESCRIPTOR_BPM_BUFFER_NUMBER_L_READ( wv )               FIELD_GET( wv, 0, 14 )
#define RDD_CPU_TX_DESCRIPTOR_BPM_BUFFER_NUMBER_L_WRITE( v, wv )           FIELD_SET( v, 0, 14, wv )

typedef struct
{
	uint32_t	reserved0 	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1 	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	skb_index 	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_DESCRIPTOR_ABS_DTS;

#define RDD_CPU_TX_DESCRIPTOR_ABS_SKB_INDEX_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r)
#define RDD_CPU_TX_DESCRIPTOR_ABS_SKB_INDEX_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p + 6, 0, 14, v)
#define RDD_CPU_TX_DESCRIPTOR_ABS_SKB_INDEX_L_READ( wv )               FIELD_GET( wv, 0, 14 )
#define RDD_CPU_TX_DESCRIPTOR_ABS_SKB_INDEX_L_WRITE( v, wv )           FIELD_SET( v, 0, 14, wv )

typedef struct
{
	uint32_t	reserved0          	:10	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	downstream_wan_flow	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1          	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2          	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_DESCRIPTOR_DS_FAST_DTS;

#define RDD_CPU_TX_DESCRIPTOR_DS_FAST_DOWNSTREAM_WAN_FLOW_READ(r, p)                 FIELD_MREAD_32((uint8_t *)p + 0, 14, 8, r)
#define RDD_CPU_TX_DESCRIPTOR_DS_FAST_DOWNSTREAM_WAN_FLOW_WRITE(v, p)                FIELD_MWRITE_32((uint8_t *)p + 0, 14, 8, v)
#define RDD_CPU_TX_DESCRIPTOR_DS_FAST_DOWNSTREAM_WAN_FLOW_L_READ( wv )               FIELD_GET( wv, 14, 8 )
#define RDD_CPU_TX_DESCRIPTOR_DS_FAST_DOWNSTREAM_WAN_FLOW_L_WRITE( v, wv )           FIELD_SET( v, 14, 8, wv )

typedef struct
{
	uint32_t	reserved0         	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	upstream_gem_flow 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	us_drop_precedence	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1         	:19	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tx_queue          	:9	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2         	:23	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_DESCRIPTOR_US_FAST_DTS;

#define RDD_CPU_TX_DESCRIPTOR_US_FAST_UPSTREAM_GEM_FLOW_READ(r, p)                  FIELD_MREAD_16((uint8_t *)p, 4, 8, r)
#define RDD_CPU_TX_DESCRIPTOR_US_FAST_UPSTREAM_GEM_FLOW_WRITE(v, p)                 FIELD_MWRITE_16((uint8_t *)p, 4, 8, v)
#define RDD_CPU_TX_DESCRIPTOR_US_FAST_UPSTREAM_GEM_FLOW_L_READ( wv )                FIELD_GET( wv, 20, 8 )
#define RDD_CPU_TX_DESCRIPTOR_US_FAST_UPSTREAM_GEM_FLOW_L_WRITE( v, wv )            FIELD_SET( v, 20, 8, wv )
#define RDD_CPU_TX_DESCRIPTOR_US_FAST_US_DROP_PRECEDENCE_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 1, 3, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_US_FAST_US_DROP_PRECEDENCE_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 1, 3, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_US_FAST_US_DROP_PRECEDENCE_L_READ( wv )               FIELD_GET( wv, 19, 1 )
#define RDD_CPU_TX_DESCRIPTOR_US_FAST_US_DROP_PRECEDENCE_L_WRITE( v, wv )           FIELD_SET( v, 19, 1, wv )
#define RDD_CPU_TX_DESCRIPTOR_US_FAST_TX_QUEUE_READ(r, p)                           FIELD_MREAD_16((uint8_t *)p + 4, 7, 9, r)
#define RDD_CPU_TX_DESCRIPTOR_US_FAST_TX_QUEUE_WRITE(v, p)                          FIELD_MWRITE_16((uint8_t *)p + 4, 7, 9, v)
#define RDD_CPU_TX_DESCRIPTOR_US_FAST_TX_QUEUE_L_READ( wv )                         FIELD_GET( wv, 23, 9 )
#define RDD_CPU_TX_DESCRIPTOR_US_FAST_TX_QUEUE_L_WRITE( v, wv )                     FIELD_SET( v, 23, 9, wv )

typedef struct
{
	uint32_t	reserved0 	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1 	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	en_1588   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2 	:27	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_DESCRIPTOR_DS_PICO_DTS;

#define RDD_CPU_TX_DESCRIPTOR_DS_PICO_EN_1588_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p + 4, 3, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_DS_PICO_EN_1588_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p + 4, 3, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_DS_PICO_EN_1588_L_READ( wv )                 FIELD_GET( wv, 27, 1 )
#define RDD_CPU_TX_DESCRIPTOR_DS_PICO_EN_1588_L_WRITE( v, wv )             FIELD_SET( v, 27, 1, wv )

typedef struct
{
	uint32_t	reserved0     	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ssid_multicast	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ssid          	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2     	:26	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_DESCRIPTOR_DS_PICO_WIFI_DTS;

#define RDD_CPU_TX_DESCRIPTOR_DS_PICO_WIFI_SSID_MULTICAST_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_DS_PICO_WIFI_SSID_MULTICAST_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 4, 6, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_DS_PICO_WIFI_SSID_MULTICAST_L_READ( wv )               FIELD_GET( wv, 30, 1 )
#define RDD_CPU_TX_DESCRIPTOR_DS_PICO_WIFI_SSID_MULTICAST_L_WRITE( v, wv )           FIELD_SET( v, 30, 1, wv )
#define RDD_CPU_TX_DESCRIPTOR_DS_PICO_WIFI_SSID_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 4, 2, 4, r)
#define RDD_CPU_TX_DESCRIPTOR_DS_PICO_WIFI_SSID_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 4, 2, 4, v)
#define RDD_CPU_TX_DESCRIPTOR_DS_PICO_WIFI_SSID_L_READ( wv )                         FIELD_GET( wv, 26, 4 )
#define RDD_CPU_TX_DESCRIPTOR_DS_PICO_WIFI_SSID_L_WRITE( v, wv )                     FIELD_SET( v, 26, 4, wv )

typedef struct
{
	uint32_t	valid       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	command     	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0   	:28	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1   	:28	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	message_type	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_MESSAGE_DESCRIPTOR_DTS;

#define RDD_CPU_TX_MESSAGE_DESCRIPTOR_VALID_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_CPU_TX_MESSAGE_DESCRIPTOR_VALID_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_CPU_TX_MESSAGE_DESCRIPTOR_COMMAND_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p, 4, 3, r)
#define RDD_CPU_TX_MESSAGE_DESCRIPTOR_COMMAND_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p, 4, 3, v)
#define RDD_CPU_TX_MESSAGE_DESCRIPTOR_MESSAGE_TYPE_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 7, 0, 4, r)
#define RDD_CPU_TX_MESSAGE_DESCRIPTOR_MESSAGE_TYPE_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 7, 0, 4, v)

typedef struct
{
	uint32_t	reserved0     	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ssid          	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ssid_multicast	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	radio_idx     	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	flow_ring_id  	:10	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2     	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_DHD_DESCRIPTOR_DTS;

#define RDD_CPU_TX_DHD_DESCRIPTOR_SSID_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 4, 3, 4, r)
#define RDD_CPU_TX_DHD_DESCRIPTOR_SSID_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 4, 3, 4, v)
#define RDD_CPU_TX_DHD_DESCRIPTOR_SSID_L_READ( wv )                         FIELD_GET( wv, 27, 4 )
#define RDD_CPU_TX_DHD_DESCRIPTOR_SSID_L_WRITE( v, wv )                     FIELD_SET( v, 27, 4, wv )
#define RDD_CPU_TX_DHD_DESCRIPTOR_SSID_MULTICAST_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 4, 2, 1, r)
#define RDD_CPU_TX_DHD_DESCRIPTOR_SSID_MULTICAST_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 4, 2, 1, v)
#define RDD_CPU_TX_DHD_DESCRIPTOR_SSID_MULTICAST_L_READ( wv )               FIELD_GET( wv, 26, 1 )
#define RDD_CPU_TX_DHD_DESCRIPTOR_SSID_MULTICAST_L_WRITE( v, wv )           FIELD_SET( v, 26, 1, wv )
#define RDD_CPU_TX_DHD_DESCRIPTOR_RADIO_IDX_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 4, 0, 2, r)
#define RDD_CPU_TX_DHD_DESCRIPTOR_RADIO_IDX_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 4, 0, 2, v)
#define RDD_CPU_TX_DHD_DESCRIPTOR_RADIO_IDX_L_READ( wv )                    FIELD_GET( wv, 24, 2 )
#define RDD_CPU_TX_DHD_DESCRIPTOR_RADIO_IDX_L_WRITE( v, wv )                FIELD_SET( v, 24, 2, wv )
#define RDD_CPU_TX_DHD_DESCRIPTOR_FLOW_RING_ID_READ(r, p)                   FIELD_MREAD_32((uint8_t *)p + 4, 14, 10, r)
#define RDD_CPU_TX_DHD_DESCRIPTOR_FLOW_RING_ID_WRITE(v, p)                  FIELD_MWRITE_32((uint8_t *)p + 4, 14, 10, v)
#define RDD_CPU_TX_DHD_DESCRIPTOR_FLOW_RING_ID_L_READ( wv )                 FIELD_GET( wv, 14, 10 )
#define RDD_CPU_TX_DHD_DESCRIPTOR_FLOW_RING_ID_L_WRITE( v, wv )             FIELD_SET( v, 14, 10, wv )

typedef struct
{
	uint32_t	reserved0     	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dhd_msg_type  	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	radio_idx     	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	flow_ring_id  	:10	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	disabled      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	read_idx_valid	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	read_idx      	:10	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2     	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_DTS;

#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_DHD_MSG_TYPE_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p + 4, 6, 2, r)
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_DHD_MSG_TYPE_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p + 4, 6, 2, v)
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_DHD_MSG_TYPE_L_READ( wv )                 FIELD_GET( wv, 30, 2 )
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_DHD_MSG_TYPE_L_WRITE( v, wv )             FIELD_SET( v, 30, 2, wv )
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_RADIO_IDX_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 4, 4, 2, r)
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_RADIO_IDX_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 4, 4, 2, v)
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_RADIO_IDX_L_READ( wv )                    FIELD_GET( wv, 28, 2 )
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_RADIO_IDX_L_WRITE( v, wv )                FIELD_SET( v, 28, 2, wv )
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_FLOW_RING_ID_READ(r, p)                   FIELD_MREAD_16((uint8_t *)p + 4, 2, 10, r)
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_FLOW_RING_ID_WRITE(v, p)                  FIELD_MWRITE_16((uint8_t *)p + 4, 2, 10, v)
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_FLOW_RING_ID_L_READ( wv )                 FIELD_GET( wv, 18, 10 )
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_FLOW_RING_ID_L_WRITE( v, wv )             FIELD_SET( v, 18, 10, wv )
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_DISABLED_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 5, 1, 1, r)
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_DISABLED_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 5, 1, 1, v)
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_DISABLED_L_READ( wv )                     FIELD_GET( wv, 17, 1 )
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_DISABLED_L_WRITE( v, wv )                 FIELD_SET( v, 17, 1, wv )
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_READ_IDX_VALID_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 5, 0, 1, r)
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_READ_IDX_VALID_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 5, 0, 1, v)
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_READ_IDX_VALID_L_READ( wv )               FIELD_GET( wv, 16, 1 )
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_READ_IDX_VALID_L_WRITE( v, wv )           FIELD_SET( v, 16, 1, wv )
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_READ_IDX_READ(r, p)                       FIELD_MREAD_16((uint8_t *)p + 6, 6, 10, r)
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_READ_IDX_WRITE(v, p)                      FIELD_MWRITE_16((uint8_t *)p + 6, 6, 10, v)
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_READ_IDX_L_READ( wv )                     FIELD_GET( wv, 6, 10 )
#define RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_READ_IDX_L_WRITE( v, wv )                 FIELD_SET( v, 6, 10, wv )
#if defined OREN

typedef struct
{
	uint32_t	egress_counter              	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	loopback_mode               	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved7                   	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ingress_counter             	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_limiter_id             	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tx_task_number              	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved8                   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved9                   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved10                  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved11                  	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved12                  	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved13                  	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved14                  	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	emac_mask                   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tx_queues_status            	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	packet_counters_ptr         	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	gpio_flow_control_vector_ptr	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0                   	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1                   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2                   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved3                   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved4                   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved5                   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved6                   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_MAC_DESCRIPTOR_DTS;

#define RDD_ETH_TX_MAC_DESCRIPTOR_EGRESS_COUNTER_READ(r, p)                               MREAD_8((uint8_t *)p, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_EGRESS_COUNTER_WRITE(v, p)                              MWRITE_8((uint8_t *)p, v)
#define RDD_ETH_TX_MAC_DESCRIPTOR_LOOPBACK_MODE_READ(r, p)                                MREAD_8((uint8_t *)p + 1, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_LOOPBACK_MODE_WRITE(v, p)                               MWRITE_8((uint8_t *)p + 1, v)
#define RDD_ETH_TX_MAC_DESCRIPTOR_INGRESS_COUNTER_READ(r, p)                              MREAD_8((uint8_t *)p + 4, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_INGRESS_COUNTER_WRITE(v, p)                             MWRITE_8((uint8_t *)p + 4, v)
#define RDD_ETH_TX_MAC_DESCRIPTOR_RATE_LIMITER_ID_READ(r, p)                              MREAD_8((uint8_t *)p + 5, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_RATE_LIMITER_ID_WRITE(v, p)                             MWRITE_8((uint8_t *)p + 5, v)
#define RDD_ETH_TX_MAC_DESCRIPTOR_TX_TASK_NUMBER_READ(r, p)                               MREAD_8((uint8_t *)p + 6, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_TX_TASK_NUMBER_WRITE(v, p)                              MWRITE_8((uint8_t *)p + 6, v)
#define RDD_ETH_TX_MAC_DESCRIPTOR_EMAC_MASK_READ(r, p)                                    MREAD_8((uint8_t *)p + 32, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_EMAC_MASK_WRITE(v, p)                                   MWRITE_8((uint8_t *)p + 32, v)
#define RDD_ETH_TX_MAC_DESCRIPTOR_TX_QUEUES_STATUS_READ(r, p)                             MREAD_8((uint8_t *)p + 33, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_TX_QUEUES_STATUS_WRITE(v, p)                            MWRITE_8((uint8_t *)p + 33, v)
#define RDD_ETH_TX_MAC_DESCRIPTOR_PACKET_COUNTERS_PTR_READ(r, p)                          MREAD_16((uint8_t *)p + 34, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_PACKET_COUNTERS_PTR_WRITE(v, p)                         MWRITE_16((uint8_t *)p + 34, v)
#define RDD_ETH_TX_MAC_DESCRIPTOR_GPIO_FLOW_CONTROL_VECTOR_PTR_READ(r, p)                 MREAD_16((uint8_t *)p + 36, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_GPIO_FLOW_CONTROL_VECTOR_PTR_WRITE(v, p)                MWRITE_16((uint8_t *)p + 36, v)
#endif
#if defined OREN

#define RDD_ETH_TX_MAC_TABLE_SIZE     6
typedef struct
{
	RDD_ETH_TX_MAC_DESCRIPTOR_DTS	entry[ RDD_ETH_TX_MAC_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_MAC_TABLE_DTS;

#define RDD_ETH_TX_MAC_TABLE_PTR()	( RDD_ETH_TX_MAC_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + ETH_TX_MAC_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_EMAC_ABSOLUTE_TX_BBH_COUNTER_SIZE     6
typedef struct
{
	RDD_EIGHT_BYTES_DTS	entry[ RDD_EMAC_ABSOLUTE_TX_BBH_COUNTER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EMAC_ABSOLUTE_TX_BBH_COUNTER_DTS;

#define RDD_EMAC_ABSOLUTE_TX_BBH_COUNTER_PTR()	( RDD_EMAC_ABSOLUTE_TX_BBH_COUNTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + EMAC_ABSOLUTE_TX_BBH_COUNTER_ADDRESS )

#endif

typedef struct
{
	uint32_t	head_pointer   	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tail_pointer   	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_counter 	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ingress_counter	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_DTS;

#define RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_HEAD_POINTER_READ(r, p)                    MREAD_16((uint8_t *)p, r)
#define RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_HEAD_POINTER_WRITE(v, p)                   MWRITE_16((uint8_t *)p, v)
#define RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_TAIL_POINTER_READ(r, p)                    MREAD_16((uint8_t *)p + 2, r)
#define RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_TAIL_POINTER_WRITE(v, p)                   MWRITE_16((uint8_t *)p + 2, v)
#define RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_EGRESS_COUNTER_READ(r, p)                  MREAD_16((uint8_t *)p + 4, r)
#define RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_EGRESS_COUNTER_WRITE(v, p)                 MWRITE_16((uint8_t *)p + 4, v)
#define RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_INGRESS_COUNTER_READ(r, p)                 MREAD_16((uint8_t *)p + 6, r)
#define RDD_FREE_PACKET_DESCRIPTORS_POOL_DESCRIPTOR_ENTRY_INGRESS_COUNTER_WRITE(v, p)                MWRITE_16((uint8_t *)p + 6, v)

typedef struct
{
	uint32_t	last_sbn        	:10	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	flow_id         	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	packet_length   	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	error           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ploam           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	error_type      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ih_buffer_number	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	target_memory   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	buffer_number   	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BBH_RX_DESCRIPTOR_DTS;

#define RDD_BBH_RX_DESCRIPTOR_LAST_SBN_READ(r, p)                         FIELD_MREAD_16((uint8_t *)p, 6, 10, r)
#define RDD_BBH_RX_DESCRIPTOR_LAST_SBN_WRITE(v, p)                        FIELD_MWRITE_16((uint8_t *)p, 6, 10, v)
#define RDD_BBH_RX_DESCRIPTOR_FLOW_ID_READ(r, p)                          FIELD_MREAD_32((uint8_t *)p + 0, 14, 8, r)
#define RDD_BBH_RX_DESCRIPTOR_FLOW_ID_WRITE(v, p)                         FIELD_MWRITE_32((uint8_t *)p + 0, 14, 8, v)
#define RDD_BBH_RX_DESCRIPTOR_PACKET_LENGTH_READ(r, p)                    FIELD_MREAD_16((uint8_t *)p + 2, 0, 14, r)
#define RDD_BBH_RX_DESCRIPTOR_PACKET_LENGTH_WRITE(v, p)                   FIELD_MWRITE_16((uint8_t *)p + 2, 0, 14, v)
#define RDD_BBH_RX_DESCRIPTOR_ERROR_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r)
#define RDD_BBH_RX_DESCRIPTOR_ERROR_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 4, 7, 1, v)
#define RDD_BBH_RX_DESCRIPTOR_PLOAM_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r)
#define RDD_BBH_RX_DESCRIPTOR_PLOAM_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 4, 6, 1, v)
#define RDD_BBH_RX_DESCRIPTOR_ERROR_TYPE_READ(r, p)                       FIELD_MREAD_16((uint8_t *)p + 4, 6, 8, r)
#define RDD_BBH_RX_DESCRIPTOR_ERROR_TYPE_WRITE(v, p)                      FIELD_MWRITE_16((uint8_t *)p + 4, 6, 8, v)
#define RDD_BBH_RX_DESCRIPTOR_IH_BUFFER_NUMBER_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 5, 0, 6, r)
#define RDD_BBH_RX_DESCRIPTOR_IH_BUFFER_NUMBER_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 5, 0, 6, v)
#define RDD_BBH_RX_DESCRIPTOR_TARGET_MEMORY_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 6, 7, 1, r)
#define RDD_BBH_RX_DESCRIPTOR_TARGET_MEMORY_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 6, 7, 1, v)
#define RDD_BBH_RX_DESCRIPTOR_BUFFER_NUMBER_READ(r, p)                    FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r)
#define RDD_BBH_RX_DESCRIPTOR_BUFFER_NUMBER_WRITE(v, p)                   FIELD_MWRITE_16((uint8_t *)p + 6, 0, 14, v)
#if defined OREN

#define RDD_GPON_RX_DIRECT_DESCRIPTORS_SIZE     32
typedef struct
{
	RDD_BBH_RX_DESCRIPTOR_DTS	entry[ RDD_GPON_RX_DIRECT_DESCRIPTORS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GPON_RX_DIRECT_DESCRIPTORS_DTS;

#define RDD_GPON_RX_DIRECT_DESCRIPTORS_PTR()	( RDD_GPON_RX_DIRECT_DESCRIPTORS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + GPON_RX_DIRECT_DESCRIPTORS_ADDRESS )

#endif

typedef struct
{
	uint32_t	head_ptr       	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_counter 	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tail_ptr       	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ingress_counter	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PCI_TX_FIFO_DESCRIPTOR_ENTRY_DTS;

#define RDD_PCI_TX_FIFO_DESCRIPTOR_ENTRY_HEAD_PTR_READ(r, p)                        MREAD_16((uint8_t *)p, r)
#define RDD_PCI_TX_FIFO_DESCRIPTOR_ENTRY_HEAD_PTR_WRITE(v, p)                       MWRITE_16((uint8_t *)p, v)
#define RDD_PCI_TX_FIFO_DESCRIPTOR_ENTRY_EGRESS_COUNTER_READ(r, p)                  MREAD_16((uint8_t *)p + 2, r)
#define RDD_PCI_TX_FIFO_DESCRIPTOR_ENTRY_EGRESS_COUNTER_WRITE(v, p)                 MWRITE_16((uint8_t *)p + 2, v)
#define RDD_PCI_TX_FIFO_DESCRIPTOR_ENTRY_TAIL_PTR_READ(r, p)                        MREAD_16((uint8_t *)p + 4, r)
#define RDD_PCI_TX_FIFO_DESCRIPTOR_ENTRY_TAIL_PTR_WRITE(v, p)                       MWRITE_16((uint8_t *)p + 4, v)
#define RDD_PCI_TX_FIFO_DESCRIPTOR_ENTRY_INGRESS_COUNTER_READ(r, p)                 MREAD_16((uint8_t *)p + 6, r)
#define RDD_PCI_TX_FIFO_DESCRIPTOR_ENTRY_INGRESS_COUNTER_WRITE(v, p)                MWRITE_16((uint8_t *)p + 6, v)
#if defined OREN

#define RDD_PCI_TX_FIFO_DESCRIPTORS_TABLE_SIZE     4
typedef struct
{
	RDD_PCI_TX_FIFO_DESCRIPTOR_ENTRY_DTS	entry[ RDD_PCI_TX_FIFO_DESCRIPTORS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PCI_TX_FIFO_DESCRIPTORS_TABLE_DTS;

#define RDD_PCI_TX_FIFO_DESCRIPTORS_TABLE_PTR()	( RDD_PCI_TX_FIFO_DESCRIPTORS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + PCI_TX_FIFO_DESCRIPTORS_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	reserved_fw_only	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PCI_TX_QUEUES_VECTOR_DTS;


typedef struct
{
	uint8_t	reserved0          	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	fifo_3_not_full_bit	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	fifo_2_not_full_bit	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	fifo_1_not_full_bit	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	fifo_0_not_full_bit	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PCI_TX_FIFO_FULL_VECTOR_ENTRY_DTS;

#define RDD_PCI_TX_FIFO_FULL_VECTOR_ENTRY_FIFO_3_NOT_FULL_BIT_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 3, 1, r)
#define RDD_PCI_TX_FIFO_FULL_VECTOR_ENTRY_FIFO_3_NOT_FULL_BIT_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 3, 1, v)
#define RDD_PCI_TX_FIFO_FULL_VECTOR_ENTRY_FIFO_2_NOT_FULL_BIT_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 2, 1, r)
#define RDD_PCI_TX_FIFO_FULL_VECTOR_ENTRY_FIFO_2_NOT_FULL_BIT_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 2, 1, v)
#define RDD_PCI_TX_FIFO_FULL_VECTOR_ENTRY_FIFO_1_NOT_FULL_BIT_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 1, 1, r)
#define RDD_PCI_TX_FIFO_FULL_VECTOR_ENTRY_FIFO_1_NOT_FULL_BIT_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 1, 1, v)
#define RDD_PCI_TX_FIFO_FULL_VECTOR_ENTRY_FIFO_0_NOT_FULL_BIT_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 0, 1, r)
#define RDD_PCI_TX_FIFO_FULL_VECTOR_ENTRY_FIFO_0_NOT_FULL_BIT_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 0, 1, v)
#define RDD_RUNNER_FLOW_IH_RESPONSE_RESERVED_FW_ONLY_NUMBER	2

typedef struct
{
	uint32_t	reserved_fw_only[RDD_RUNNER_FLOW_IH_RESPONSE_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RUNNER_FLOW_IH_RESPONSE_DTS;

#if defined OREN

#define RDD_GPON_RX_NORMAL_DESCRIPTORS_SIZE     32
typedef struct
{
	RDD_BBH_RX_DESCRIPTOR_DTS	entry[ RDD_GPON_RX_NORMAL_DESCRIPTORS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GPON_RX_NORMAL_DESCRIPTORS_DTS;

#define RDD_GPON_RX_NORMAL_DESCRIPTORS_PTR()	( RDD_GPON_RX_NORMAL_DESCRIPTORS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + GPON_RX_NORMAL_DESCRIPTORS_ADDRESS )

#endif

typedef struct
{
	uint32_t	reserved_fw_only	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DOWNSTREAM_DMA_PIPE_BUFFER_DTS;

/* PRIVATE_B */
#if defined OREN

#define RDD_US_INGRESS_HANDLER_BUFFER_SIZE     32
typedef struct
{
	RDD_IH_BUFFER_DTS	entry[ RDD_US_INGRESS_HANDLER_BUFFER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_HANDLER_BUFFER_DTS;

#define RDD_US_INGRESS_HANDLER_BUFFER_PTR()	( RDD_US_INGRESS_HANDLER_BUFFER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_HANDLER_BUFFER_ADDRESS )

#endif

typedef struct
{
	uint32_t	vlan_cmd_index                	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	qos_rule_match                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	qos_rule_overrun_wan_flow_mode	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wan_flow_mapping_mode         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wan_flow_mapping_table        	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	opbit_remark_mode             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ipbit_remark_mode             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wan_flow                      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	qos_mapping_mode              	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	traffic_class                 	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_controller               	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	drop                          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cpu                           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ic_ip_flow                    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1                     	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dei_remark_enable             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dei_value                     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	policer_mode                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	policer_id                    	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2                     	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	outer_pbit                    	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	inner_pbit                    	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dscp_remarking_mode           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dscp                          	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ecn                           	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_VLAN_CMD_INDEX_READ(r, p)                                 FIELD_MREAD_8((uint8_t *)p, 1, 7, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_VLAN_CMD_INDEX_WRITE(v, p)                                FIELD_MWRITE_8((uint8_t *)p, 1, 7, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_RULE_MATCH_READ(r, p)                                 FIELD_MREAD_8((uint8_t *)p, 0, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_RULE_MATCH_WRITE(v, p)                                FIELD_MWRITE_8((uint8_t *)p, 0, 1, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_RULE_OVERRUN_WAN_FLOW_MODE_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 1, 7, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_RULE_OVERRUN_WAN_FLOW_MODE_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 1, 7, 1, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WAN_FLOW_MAPPING_MODE_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 1, 6, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WAN_FLOW_MAPPING_MODE_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 1, 6, 1, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WAN_FLOW_MAPPING_TABLE_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 1, 3, 3, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WAN_FLOW_MAPPING_TABLE_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 1, 3, 3, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OPBIT_REMARK_MODE_READ(r, p)                              FIELD_MREAD_8((uint8_t *)p + 1, 2, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OPBIT_REMARK_MODE_WRITE(v, p)                             FIELD_MWRITE_8((uint8_t *)p + 1, 2, 1, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IPBIT_REMARK_MODE_READ(r, p)                              FIELD_MREAD_8((uint8_t *)p + 1, 1, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IPBIT_REMARK_MODE_WRITE(v, p)                             FIELD_MWRITE_8((uint8_t *)p + 1, 1, 1, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WAN_FLOW_READ(r, p)                                       FIELD_MREAD_32((uint8_t *)p + 0, 9, 8, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WAN_FLOW_WRITE(v, p)                                      FIELD_MWRITE_32((uint8_t *)p + 0, 9, 8, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_MAPPING_MODE_READ(r, p)                               FIELD_MREAD_8((uint8_t *)p + 2, 0, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_MAPPING_MODE_WRITE(v, p)                              FIELD_MWRITE_8((uint8_t *)p + 2, 0, 1, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_TRAFFIC_CLASS_READ(r, p)                                  FIELD_MREAD_8((uint8_t *)p + 3, 5, 3, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_TRAFFIC_CLASS_WRITE(v, p)                                 FIELD_MWRITE_8((uint8_t *)p + 3, 5, 3, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_RATE_CONTROLLER_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 3, 0, 5, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_RATE_CONTROLLER_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 3, 0, 5, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DROP_READ(r, p)                                           FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DROP_WRITE(v, p)                                          FIELD_MWRITE_8((uint8_t *)p + 4, 7, 1, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CPU_READ(r, p)                                            FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CPU_WRITE(v, p)                                           FIELD_MWRITE_8((uint8_t *)p + 4, 6, 1, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IC_IP_FLOW_READ(r, p)                                     FIELD_MREAD_8((uint8_t *)p + 4, 5, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IC_IP_FLOW_WRITE(v, p)                                    FIELD_MWRITE_8((uint8_t *)p + 4, 5, 1, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_REMARK_ENABLE_READ(r, p)                              FIELD_MREAD_8((uint8_t *)p + 5, 7, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_REMARK_ENABLE_WRITE(v, p)                             FIELD_MWRITE_8((uint8_t *)p + 5, 7, 1, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_VALUE_READ(r, p)                                      FIELD_MREAD_8((uint8_t *)p + 5, 6, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_VALUE_WRITE(v, p)                                     FIELD_MWRITE_8((uint8_t *)p + 5, 6, 1, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_MODE_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p + 5, 5, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_MODE_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p + 5, 5, 1, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_ID_READ(r, p)                                     FIELD_MREAD_8((uint8_t *)p + 5, 1, 4, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_ID_WRITE(v, p)                                    FIELD_MWRITE_8((uint8_t *)p + 5, 1, 4, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OUTER_PBIT_READ(r, p)                                     FIELD_MREAD_8((uint8_t *)p + 6, 4, 3, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OUTER_PBIT_WRITE(v, p)                                    FIELD_MWRITE_8((uint8_t *)p + 6, 4, 3, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_INNER_PBIT_READ(r, p)                                     FIELD_MREAD_8((uint8_t *)p + 6, 1, 3, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_INNER_PBIT_WRITE(v, p)                                    FIELD_MWRITE_8((uint8_t *)p + 6, 1, 3, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_REMARKING_MODE_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 6, 0, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_REMARKING_MODE_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 6, 0, 1, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_READ(r, p)                                           FIELD_MREAD_8((uint8_t *)p + 7, 2, 6, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_WRITE(v, p)                                          FIELD_MWRITE_8((uint8_t *)p + 7, 2, 6, v)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_ECN_READ(r, p)                                            FIELD_MREAD_8((uint8_t *)p + 7, 0, 2, r)
#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_ECN_WRITE(v, p)                                           FIELD_MWRITE_8((uint8_t *)p + 7, 0, 2, v)
#if defined OREN

#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE     256
typedef struct
{
	RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS	entry[ RDD_US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_PTR()	( RDD_US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_DSCP_TO_PBITS_TABLE_SIZE     6
#define RDD_US_DSCP_TO_PBITS_TABLE_SIZE2    64
typedef struct
{
	RDD_DSCP_TO_PBITS_ENTRY_DTS	entry[ RDD_US_DSCP_TO_PBITS_TABLE_SIZE ][ RDD_US_DSCP_TO_PBITS_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_DSCP_TO_PBITS_TABLE_DTS;

#define RDD_US_DSCP_TO_PBITS_TABLE_PTR()	( RDD_US_DSCP_TO_PBITS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_DSCP_TO_PBITS_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_VLAN_PARAMETER_TABLE_SIZE     128
typedef struct
{
	RDD_VLAN_PARAMETER_ENTRY_DTS	entry[ RDD_US_VLAN_PARAMETER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_VLAN_PARAMETER_TABLE_DTS;

#define RDD_US_VLAN_PARAMETER_TABLE_PTR()	( RDD_US_VLAN_PARAMETER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_VLAN_PARAMETER_TABLE_ADDRESS )

#endif
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLER_ADDR_NUMBER	4

typedef struct
{
	uint32_t	reserved0                                                                           	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_limiter_priority                                                               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_limiter_mode                                                                   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	schedule                                                                            	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbh_destination                                                                     	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_controllers_status                                                             	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_controllers_sustain_vector                                                     	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_controllers_peak_vector                                                        	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	peak_burst_counter                                                                  	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	peak_offset                                                                         	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	peak_scheduling_mode                                                                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1                                                                           	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ack_pending_epon                                                                    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ack_pending                                                                         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	byte_counter                                                                        	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	rate_controller_addr[RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLER_ADDR_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WAN_CHANNEL_8_39_DESCRIPTOR_DTS;

#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_LIMITER_PRIORITY_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p, 1, 1, r)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_LIMITER_PRIORITY_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p, 1, 1, v)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_LIMITER_MODE_READ(r, p)                               FIELD_MREAD_8((uint8_t *)p, 0, 1, r)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_LIMITER_MODE_WRITE(v, p)                              FIELD_MWRITE_8((uint8_t *)p, 0, 1, v)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_SCHEDULE_READ(r, p)                                        MREAD_8((uint8_t *)p + 1, r)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_SCHEDULE_WRITE(v, p)                                       MWRITE_8((uint8_t *)p + 1, v)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_BBH_DESTINATION_READ(r, p)                                 MREAD_16((uint8_t *)p + 2, r)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_BBH_DESTINATION_WRITE(v, p)                                MWRITE_16((uint8_t *)p + 2, v)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLERS_STATUS_READ(r, p)                         MREAD_32((uint8_t *)p + 4, r)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLERS_STATUS_WRITE(v, p)                        MWRITE_32((uint8_t *)p + 4, v)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLERS_SUSTAIN_VECTOR_READ(r, p)                 MREAD_32((uint8_t *)p + 8, r)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLERS_SUSTAIN_VECTOR_WRITE(v, p)                MWRITE_32((uint8_t *)p + 8, v)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLERS_PEAK_VECTOR_READ(r, p)                    MREAD_32((uint8_t *)p + 12, r)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLERS_PEAK_VECTOR_WRITE(v, p)                   MWRITE_32((uint8_t *)p + 12, v)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_PEAK_BURST_COUNTER_READ(r, p)                              MREAD_16((uint8_t *)p + 16, r)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_PEAK_BURST_COUNTER_WRITE(v, p)                             MWRITE_16((uint8_t *)p + 16, v)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_PEAK_OFFSET_READ(r, p)                                     MREAD_8((uint8_t *)p + 18, r)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_PEAK_OFFSET_WRITE(v, p)                                    MWRITE_8((uint8_t *)p + 18, v)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_PEAK_SCHEDULING_MODE_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 19, 7, 1, r)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_PEAK_SCHEDULING_MODE_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 19, 7, 1, v)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_ACK_PENDING_EPON_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 19, 1, 1, r)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_ACK_PENDING_EPON_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 19, 1, 1, v)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_ACK_PENDING_READ(r, p)                                     FIELD_MREAD_8((uint8_t *)p + 19, 0, 1, r)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_ACK_PENDING_WRITE(v, p)                                    FIELD_MWRITE_8((uint8_t *)p + 19, 0, 1, v)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_BYTE_COUNTER_READ(r, p)                                    MREAD_32((uint8_t *)p + 20, r)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_BYTE_COUNTER_WRITE(v, p)                                   MWRITE_32((uint8_t *)p + 20, v)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLER_ADDR_READ(r, p, i)                         MREAD_I_16((uint8_t *)p + 24, i, r)
#define RDD_WAN_CHANNEL_8_39_DESCRIPTOR_RATE_CONTROLLER_ADDR_WRITE(v, p, i)                        MWRITE_I_16((uint8_t *)p + 24, i, v)
#if defined OREN

#define RDD_WAN_CHANNELS_8_39_TABLE_SIZE     32
typedef struct
{
	RDD_WAN_CHANNEL_8_39_DESCRIPTOR_DTS	entry[ RDD_WAN_CHANNELS_8_39_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WAN_CHANNELS_8_39_TABLE_DTS;

#define RDD_WAN_CHANNELS_8_39_TABLE_PTR()	( RDD_WAN_CHANNELS_8_39_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + WAN_CHANNELS_8_39_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_RUNNER_FLOW_HEADER_BUFFER_SIZE     3
typedef struct
{
	RDD_RUNNER_FLOW_HEADER_BUFFER_DTS	entry[ RDD_US_RUNNER_FLOW_HEADER_BUFFER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_RUNNER_FLOW_HEADER_BUFFER_DTS;

#define RDD_US_RUNNER_FLOW_HEADER_BUFFER_PTR()	( RDD_US_RUNNER_FLOW_HEADER_BUFFER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_RUNNER_FLOW_HEADER_BUFFER_ADDRESS )

#endif
#if defined OREN

#define RDD_US_QUEUE_PROFILE_TABLE_SIZE     8
typedef struct
{
	RDD_QUEUE_PROFILE_DTS	entry[ RDD_US_QUEUE_PROFILE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_QUEUE_PROFILE_TABLE_DTS;

#define RDD_US_QUEUE_PROFILE_TABLE_PTR()	( RDD_US_QUEUE_PROFILE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_QUEUE_PROFILE_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_CPU_REASON_TO_METER_TABLE_SIZE     64
typedef struct
{
	RDD_CPU_REASON_TO_METER_ENTRY_DTS	entry[ RDD_US_CPU_REASON_TO_METER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_CPU_REASON_TO_METER_TABLE_DTS;

#define RDD_US_CPU_REASON_TO_METER_TABLE_PTR()	( RDD_US_CPU_REASON_TO_METER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_REASON_TO_METER_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_SIZE     16
typedef struct
{
	RDD_FOUR_BYTES_DTS	entry[ RDD_US_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_DTS;

#define RDD_US_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_PTR()	( RDD_US_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_SIZE     16
typedef struct
{
	RDD_FOUR_BYTES_DTS	entry[ RDD_US_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_DTS;

#define RDD_US_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_PTR()	( RDD_US_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_ADDRESS )

#endif

typedef struct
{
	uint8_t	queue          	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	rate_controller	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_QUEUE_ENTRY_DTS;

#define RDD_US_QUEUE_ENTRY_QUEUE_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p, 5, 3, r)
#define RDD_US_QUEUE_ENTRY_QUEUE_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p, 5, 3, v)
#define RDD_US_QUEUE_ENTRY_RATE_CONTROLLER_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 0, 5, r)
#define RDD_US_QUEUE_ENTRY_RATE_CONTROLLER_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 0, 5, v)
#if defined OREN

#define RDD_US_PBITS_TO_QOS_TABLE_SIZE     8
#define RDD_US_PBITS_TO_QOS_TABLE_SIZE2    8
typedef struct
{
	RDD_US_QUEUE_ENTRY_DTS	entry[ RDD_US_PBITS_TO_QOS_TABLE_SIZE ][ RDD_US_PBITS_TO_QOS_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_PBITS_TO_QOS_TABLE_DTS;

#define RDD_US_PBITS_TO_QOS_TABLE_PTR()	( RDD_US_PBITS_TO_QOS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PBITS_TO_QOS_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_RATE_LIMITER_TABLE_SIZE     16
typedef struct
{
	RDD_RATE_LIMITER_ENTRY_DTS	entry[ RDD_US_RATE_LIMITER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_RATE_LIMITER_TABLE_DTS;

#define RDD_US_RATE_LIMITER_TABLE_PTR()	( RDD_US_RATE_LIMITER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_RATE_LIMITER_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_LAYER4_FILTERS_LOOKUP_TABLE_SIZE     16
typedef struct
{
	RDD_LAYER4_FILTERS_LOOKUP_ENTRY_DTS	entry[ RDD_US_LAYER4_FILTERS_LOOKUP_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_LAYER4_FILTERS_LOOKUP_TABLE_DTS;

#define RDD_US_LAYER4_FILTERS_LOOKUP_TABLE_PTR()	( RDD_US_LAYER4_FILTERS_LOOKUP_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_LAYER4_FILTERS_LOOKUP_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	reserved0                         	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	hdr_type                          	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	traffic_class_to_queue_table_index	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wan_channel_id                    	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pbits_to_queue_table_index        	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	crc_calc                          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wan_port_id                       	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_WAN_FLOW_ENTRY_DTS;

#define RDD_US_WAN_FLOW_ENTRY_HDR_TYPE_READ(r, p)                                           FIELD_MREAD_8((uint8_t *)p, 1, 3, r)
#define RDD_US_WAN_FLOW_ENTRY_HDR_TYPE_WRITE(v, p)                                          FIELD_MWRITE_8((uint8_t *)p, 1, 3, v)
#define RDD_US_WAN_FLOW_ENTRY_TRAFFIC_CLASS_TO_QUEUE_TABLE_INDEX_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p, 6, 3, r)
#define RDD_US_WAN_FLOW_ENTRY_TRAFFIC_CLASS_TO_QUEUE_TABLE_INDEX_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p, 6, 3, v)
#define RDD_US_WAN_FLOW_ENTRY_WAN_CHANNEL_ID_READ(r, p)                                     FIELD_MREAD_8((uint8_t *)p + 1, 0, 6, r)
#define RDD_US_WAN_FLOW_ENTRY_WAN_CHANNEL_ID_WRITE(v, p)                                    FIELD_MWRITE_8((uint8_t *)p + 1, 0, 6, v)
#define RDD_US_WAN_FLOW_ENTRY_PBITS_TO_QUEUE_TABLE_INDEX_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 2, 5, 3, r)
#define RDD_US_WAN_FLOW_ENTRY_PBITS_TO_QUEUE_TABLE_INDEX_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 2, 5, 3, v)
#define RDD_US_WAN_FLOW_ENTRY_CRC_CALC_READ(r, p)                                           FIELD_MREAD_8((uint8_t *)p + 2, 4, 1, r)
#define RDD_US_WAN_FLOW_ENTRY_CRC_CALC_WRITE(v, p)                                          FIELD_MWRITE_8((uint8_t *)p + 2, 4, 1, v)
#define RDD_US_WAN_FLOW_ENTRY_WAN_PORT_ID_READ(r, p)                                        FIELD_MREAD_16((uint8_t *)p + 2, 0, 12, r)
#define RDD_US_WAN_FLOW_ENTRY_WAN_PORT_ID_WRITE(v, p)                                       FIELD_MWRITE_16((uint8_t *)p + 2, 0, 12, v)
#if defined OREN

#define RDD_US_WAN_FLOW_TABLE_SIZE     256
typedef struct
{
	RDD_US_WAN_FLOW_ENTRY_DTS	entry[ RDD_US_WAN_FLOW_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_WAN_FLOW_TABLE_DTS;

#define RDD_US_WAN_FLOW_TABLE_PTR()	( RDD_US_WAN_FLOW_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_WAN_FLOW_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_SIZE     6
#define RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_SIZE2    32
typedef struct
{
	RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS	entry[ RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_SIZE ][ RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_DTS;

#define RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_PTR()	( RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_FILTERS_LOOKUP_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_PBITS_TO_PBITS_TABLE_SIZE     32
#define RDD_US_PBITS_TO_PBITS_TABLE_SIZE2    8
typedef struct
{
	RDD_PBITS_TO_PBITS_ENTRY_DTS	entry[ RDD_US_PBITS_TO_PBITS_TABLE_SIZE ][ RDD_US_PBITS_TO_PBITS_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_PBITS_TO_PBITS_TABLE_DTS;

#define RDD_US_PBITS_TO_PBITS_TABLE_PTR()	( RDD_US_PBITS_TO_PBITS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PBITS_TO_PBITS_TABLE_ADDRESS )

#endif
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLER_ADDR_NUMBER	32

typedef struct
{
	uint32_t	reserved0                                                                           	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_limiter_priority                                                               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_limiter_mode                                                                   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	schedule                                                                            	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbh_destination                                                                     	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_controllers_status                                                             	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_controllers_sustain_vector                                                     	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_controllers_peak_vector                                                        	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	peak_burst_counter                                                                  	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	peak_offset                                                                         	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	peak_scheduling_mode                                                                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1                                                                           	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ack_pending_epon                                                                    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ack_pending                                                                         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	byte_counter                                                                        	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	rate_controller_addr[RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLER_ADDR_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WAN_CHANNEL_0_7_DESCRIPTOR_DTS;

#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_LIMITER_PRIORITY_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p, 1, 1, r)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_LIMITER_PRIORITY_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p, 1, 1, v)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_LIMITER_MODE_READ(r, p)                               FIELD_MREAD_8((uint8_t *)p, 0, 1, r)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_LIMITER_MODE_WRITE(v, p)                              FIELD_MWRITE_8((uint8_t *)p, 0, 1, v)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_SCHEDULE_READ(r, p)                                        MREAD_8((uint8_t *)p + 1, r)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_SCHEDULE_WRITE(v, p)                                       MWRITE_8((uint8_t *)p + 1, v)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_BBH_DESTINATION_READ(r, p)                                 MREAD_16((uint8_t *)p + 2, r)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_BBH_DESTINATION_WRITE(v, p)                                MWRITE_16((uint8_t *)p + 2, v)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLERS_STATUS_READ(r, p)                         MREAD_32((uint8_t *)p + 4, r)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLERS_STATUS_WRITE(v, p)                        MWRITE_32((uint8_t *)p + 4, v)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLERS_SUSTAIN_VECTOR_READ(r, p)                 MREAD_32((uint8_t *)p + 8, r)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLERS_SUSTAIN_VECTOR_WRITE(v, p)                MWRITE_32((uint8_t *)p + 8, v)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLERS_PEAK_VECTOR_READ(r, p)                    MREAD_32((uint8_t *)p + 12, r)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLERS_PEAK_VECTOR_WRITE(v, p)                   MWRITE_32((uint8_t *)p + 12, v)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_PEAK_BURST_COUNTER_READ(r, p)                              MREAD_16((uint8_t *)p + 16, r)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_PEAK_BURST_COUNTER_WRITE(v, p)                             MWRITE_16((uint8_t *)p + 16, v)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_PEAK_OFFSET_READ(r, p)                                     MREAD_8((uint8_t *)p + 18, r)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_PEAK_OFFSET_WRITE(v, p)                                    MWRITE_8((uint8_t *)p + 18, v)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_PEAK_SCHEDULING_MODE_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 19, 7, 1, r)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_PEAK_SCHEDULING_MODE_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 19, 7, 1, v)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_ACK_PENDING_EPON_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 19, 1, 1, r)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_ACK_PENDING_EPON_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 19, 1, 1, v)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_ACK_PENDING_READ(r, p)                                     FIELD_MREAD_8((uint8_t *)p + 19, 0, 1, r)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_ACK_PENDING_WRITE(v, p)                                    FIELD_MWRITE_8((uint8_t *)p + 19, 0, 1, v)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_BYTE_COUNTER_READ(r, p)                                    MREAD_32((uint8_t *)p + 20, r)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_BYTE_COUNTER_WRITE(v, p)                                   MWRITE_32((uint8_t *)p + 20, v)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLER_ADDR_READ(r, p, i)                         MREAD_I_16((uint8_t *)p + 24, i, r)
#define RDD_WAN_CHANNEL_0_7_DESCRIPTOR_RATE_CONTROLLER_ADDR_WRITE(v, p, i)                        MWRITE_I_16((uint8_t *)p + 24, i, v)
#if defined OREN

#define RDD_WAN_CHANNELS_0_7_TABLE_SIZE     8
typedef struct
{
	RDD_WAN_CHANNEL_0_7_DESCRIPTOR_DTS	entry[ RDD_WAN_CHANNELS_0_7_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WAN_CHANNELS_0_7_TABLE_DTS;

#define RDD_WAN_CHANNELS_0_7_TABLE_PTR()	( RDD_WAN_CHANNELS_0_7_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + WAN_CHANNELS_0_7_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_TRAFFIC_CLASS_TO_QUEUE_TABLE_SIZE     8
#define RDD_US_TRAFFIC_CLASS_TO_QUEUE_TABLE_SIZE2    8
typedef struct
{
	RDD_US_QUEUE_ENTRY_DTS	entry[ RDD_US_TRAFFIC_CLASS_TO_QUEUE_TABLE_SIZE ][ RDD_US_TRAFFIC_CLASS_TO_QUEUE_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_TRAFFIC_CLASS_TO_QUEUE_TABLE_DTS;

#define RDD_US_TRAFFIC_CLASS_TO_QUEUE_TABLE_PTR()	( RDD_US_TRAFFIC_CLASS_TO_QUEUE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_TRAFFIC_CLASS_TO_QUEUE_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_PBITS_PARAMETER_TABLE_SIZE     128
typedef struct
{
	RDD_PBITS_PARAMETER_ENTRY_DTS	entry[ RDD_US_PBITS_PARAMETER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_PBITS_PARAMETER_TABLE_DTS;

#define RDD_US_PBITS_PARAMETER_TABLE_PTR()	( RDD_US_PBITS_PARAMETER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PBITS_PARAMETER_TABLE_ADDRESS )

#endif
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_DATA_BYTES_NUMBER	256

typedef struct
{
	uint32_t	waiting_time                                                      	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	guard_time                                                        	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	etu                                                               	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sampling_time                                                     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	max_retransmit                                                    	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	task_type                                                         	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	status_byte                                                       	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved                                                          	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	header_bytes                                                      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	send_length                                                       	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	receive_length                                                    	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	data_bytes[RDD_SMART_CARD_DESCRIPTOR_ENTRY_DATA_BYTES_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SMART_CARD_DESCRIPTOR_ENTRY_DTS;

#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_WAITING_TIME_READ(r, p)                   MREAD_32((uint8_t *)p, r)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_WAITING_TIME_WRITE(v, p)                  MWRITE_32((uint8_t *)p, v)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_GUARD_TIME_READ(r, p)                     MREAD_8((uint8_t *)p + 4, r)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_GUARD_TIME_WRITE(v, p)                    MWRITE_8((uint8_t *)p + 4, v)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_ETU_READ(r, p)                            MREAD_8((uint8_t *)p + 5, r)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_ETU_WRITE(v, p)                           MWRITE_8((uint8_t *)p + 5, v)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_SAMPLING_TIME_READ(r, p)                  MREAD_8((uint8_t *)p + 6, r)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_SAMPLING_TIME_WRITE(v, p)                 MWRITE_8((uint8_t *)p + 6, v)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_MAX_RETRANSMIT_READ(r, p)                 MREAD_8((uint8_t *)p + 7, r)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_MAX_RETRANSMIT_WRITE(v, p)                MWRITE_8((uint8_t *)p + 7, v)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_TASK_TYPE_READ(r, p)                      MREAD_8((uint8_t *)p + 8, r)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_TASK_TYPE_WRITE(v, p)                     MWRITE_8((uint8_t *)p + 8, v)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_STATUS_BYTE_READ(r, p)                    MREAD_8((uint8_t *)p + 9, r)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_STATUS_BYTE_WRITE(v, p)                   MWRITE_8((uint8_t *)p + 9, v)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_HEADER_BYTES_READ(r, p)                   MREAD_8((uint8_t *)p + 11, r)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_HEADER_BYTES_WRITE(v, p)                  MWRITE_8((uint8_t *)p + 11, v)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_SEND_LENGTH_READ(r, p)                    MREAD_16((uint8_t *)p + 12, r)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_SEND_LENGTH_WRITE(v, p)                   MWRITE_16((uint8_t *)p + 12, v)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_RECEIVE_LENGTH_READ(r, p)                 MREAD_16((uint8_t *)p + 14, r)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_RECEIVE_LENGTH_WRITE(v, p)                MWRITE_16((uint8_t *)p + 14, v)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_DATA_BYTES_READ(r, p, i)                  MREAD_I_8((uint8_t *)p + 16, i, r)
#define RDD_SMART_CARD_DESCRIPTOR_ENTRY_DATA_BYTES_WRITE(v, p, i)                 MWRITE_I_8((uint8_t *)p + 16, i, v)
#if defined OREN

#define RDD_US_VLAN_COMMANDS_TABLE_SIZE     64
typedef struct
{
	RDD_VLAN_COMMAND_ENRTY_DTS	entry[ RDD_US_VLAN_COMMANDS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_VLAN_COMMANDS_TABLE_DTS;

#define RDD_US_VLAN_COMMANDS_TABLE_PTR()	( RDD_US_VLAN_COMMANDS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_VLAN_COMMANDS_TABLE_ADDRESS )

#endif

typedef struct
{
	uint16_t	active_rate_controllers	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BUDGET_ALLOCATOR_ENTRY_DTS;

#define RDD_BUDGET_ALLOCATOR_ENTRY_ACTIVE_RATE_CONTROLLERS_READ(r, p)                 MREAD_16((uint8_t *)p, r)
#define RDD_BUDGET_ALLOCATOR_ENTRY_ACTIVE_RATE_CONTROLLERS_WRITE(v, p)                MWRITE_16((uint8_t *)p, v)

#define RDD_BUDGET_ALLOCATOR_TABLE_SIZE     8
typedef struct
{
	RDD_BUDGET_ALLOCATOR_ENTRY_DTS	entry[ RDD_BUDGET_ALLOCATOR_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BUDGET_ALLOCATOR_TABLE_DTS;

#define RDD_US_RATE_CONTROL_BUDGET_ALLOCATOR_TABLE_PTR()	( RDD_US_RATE_CONTROL_BUDGET_ALLOCATOR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_RATE_CONTROL_BUDGET_ALLOCATOR_TABLE_ADDRESS )

#define RDD_US_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_PTR()	( RDD_US_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_ADDRESS )

#if defined OREN

#define RDD_US_PBITS_TO_WAN_FLOW_TABLE_SIZE     8
#define RDD_US_PBITS_TO_WAN_FLOW_TABLE_SIZE2    8
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_US_PBITS_TO_WAN_FLOW_TABLE_SIZE ][ RDD_US_PBITS_TO_WAN_FLOW_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_PBITS_TO_WAN_FLOW_TABLE_DTS;

#define RDD_US_PBITS_TO_WAN_FLOW_TABLE_PTR()	( RDD_US_PBITS_TO_WAN_FLOW_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PBITS_TO_WAN_FLOW_TABLE_ADDRESS )

#endif
#define RDD_US_CPU_RX_METER_TABLE_PTR()	( RDD_US_CPU_RX_METER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_RX_METER_TABLE_ADDRESS )

#if defined OREN

#define RDD_US_LAN_VID_TABLE_SIZE     128
typedef struct
{
	RDD_VID_ENTRY_DTS	entry[ RDD_US_LAN_VID_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_LAN_VID_TABLE_DTS;

#define RDD_US_LAN_VID_TABLE_PTR()	( RDD_US_LAN_VID_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_LAN_VID_TABLE_ADDRESS )

#endif
#define RDD_US_POLICER_TABLE_PTR()	( RDD_US_POLICER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_POLICER_TABLE_ADDRESS )

#if defined OREN

#define RDD_US_PACKET_BUFFER_TABLE_SIZE     256
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_US_PACKET_BUFFER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_PACKET_BUFFER_TABLE_DTS;

#define RDD_US_PACKET_BUFFER_TABLE_PTR()	( RDD_US_PACKET_BUFFER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PACKET_BUFFER_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_FORWARDING_MATRIX_TABLE_SIZE     9
#define RDD_US_FORWARDING_MATRIX_TABLE_SIZE2    16
typedef struct
{
	RDD_FORWARDING_MATRIX_ENTRY_DTS	entry[ RDD_US_FORWARDING_MATRIX_TABLE_SIZE ][ RDD_US_FORWARDING_MATRIX_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_FORWARDING_MATRIX_TABLE_DTS;

#define RDD_US_FORWARDING_MATRIX_TABLE_PTR()	( RDD_US_FORWARDING_MATRIX_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FORWARDING_MATRIX_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_TPID_OVERWRITE_TABLE_SIZE     8
typedef struct
{
	RDD_TPID_OVERWRITE_ENTRY_DTS	entry[ RDD_US_TPID_OVERWRITE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_TPID_OVERWRITE_TABLE_DTS;

#define RDD_US_TPID_OVERWRITE_TABLE_PTR()	( RDD_US_TPID_OVERWRITE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_TPID_OVERWRITE_TABLE_ADDRESS )

#endif
#define RDD_US_PICO_TIMER_TASK_DESCRIPTOR_TABLE_PTR()	( RDD_US_PICO_TIMER_TASK_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PICO_TIMER_TASK_DESCRIPTOR_TABLE_ADDRESS )

#if defined OREN

#define RDD_US_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_SIZE     32
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_US_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_PTR()	( RDD_US_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_CPU_TX_BBH_DESCRIPTORS_SIZE     32
typedef struct
{
	RDD_CPU_TX_BBH_DESCRIPTORS_ENTRY_DTS	entry[ RDD_US_CPU_TX_BBH_DESCRIPTORS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_CPU_TX_BBH_DESCRIPTORS_DTS;

#define RDD_US_CPU_TX_BBH_DESCRIPTORS_PTR()	( RDD_US_CPU_TX_BBH_DESCRIPTORS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_TX_BBH_DESCRIPTORS_ADDRESS )

#endif
#if defined OREN

#define RDD_US_INGRESS_HANDLER_SKB_DATA_POINTER_SIZE     32
typedef struct
{
	RDD_FOUR_BYTES_DTS	entry[ RDD_US_INGRESS_HANDLER_SKB_DATA_POINTER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_HANDLER_SKB_DATA_POINTER_DTS;

#define RDD_US_INGRESS_HANDLER_SKB_DATA_POINTER_PTR()	( RDD_US_INGRESS_HANDLER_SKB_DATA_POINTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_HANDLER_SKB_DATA_POINTER_ADDRESS )

#endif
#if defined OREN

#define RDD_US_TIMER_SCHEDULER_PRIMITIVE_TABLE_SIZE     8
typedef struct
{
	RDD_TIMER_SCHEDULER_PRIMITIVE_ENTRY_DTS	entry[ RDD_US_TIMER_SCHEDULER_PRIMITIVE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_TIMER_SCHEDULER_PRIMITIVE_TABLE_DTS;

#define RDD_US_TIMER_SCHEDULER_PRIMITIVE_TABLE_PTR()	( RDD_US_TIMER_SCHEDULER_PRIMITIVE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_TIMER_SCHEDULER_PRIMITIVE_TABLE_ADDRESS )

#endif

typedef struct
{
	uint8_t	reserved_fw_only	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GPON_ABSOLUTE_TX_COUNTER_DTS;

#if defined OREN

#define RDD_GPON_ABSOLUTE_TX_BBH_COUNTER_SIZE     40
typedef struct
{
	RDD_GPON_ABSOLUTE_TX_COUNTER_DTS	entry[ RDD_GPON_ABSOLUTE_TX_BBH_COUNTER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GPON_ABSOLUTE_TX_BBH_COUNTER_DTS;

#define RDD_GPON_ABSOLUTE_TX_BBH_COUNTER_PTR()	( RDD_GPON_ABSOLUTE_TX_BBH_COUNTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + GPON_ABSOLUTE_TX_BBH_COUNTER_ADDRESS )

#endif
#if defined OREN

#define RDD_US_FLOW_BASED_ACTION_PTR_TABLE_SIZE     32
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_US_FLOW_BASED_ACTION_PTR_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_FLOW_BASED_ACTION_PTR_TABLE_DTS;

#define RDD_US_FLOW_BASED_ACTION_PTR_TABLE_PTR()	( RDD_US_FLOW_BASED_ACTION_PTR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FLOW_BASED_ACTION_PTR_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_CONNECTION_CONTEXT_BUFFER_SIZE     4
typedef struct
{
	RDD_CONNECTION_CONTEXT_BUFFER_ENTRY_DTS	entry[ RDD_US_CONNECTION_CONTEXT_BUFFER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_CONNECTION_CONTEXT_BUFFER_DTS;

#define RDD_US_CONNECTION_CONTEXT_BUFFER_PTR()	( RDD_US_CONNECTION_CONTEXT_BUFFER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CONNECTION_CONTEXT_BUFFER_ADDRESS )

#endif
#if defined OREN

#define RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE     16
typedef struct
{
	RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_DTS	entry[ RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_PTR()	( RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_SIZE     6
#define RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_SIZE2    16
typedef struct
{
	RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DTS	entry[ RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_SIZE ][ RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_DTS;

#define RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_PTR()	( RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_FILTERS_PARAMETER_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_SIZE     4
typedef struct
{
	RDD_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_ENTRY_DTS	entry[ RDD_US_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_PTR()	( RDD_US_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	current_budget           	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	drop_threshold           	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	xoff_threshold           	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	allocated_budget_mantisa 	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	allocated_budget_exponent	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	active_pause_flag        	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbh_rx_address           	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INGRESS_RATE_LIMITER_ENTRY_DTS;

#define RDD_INGRESS_RATE_LIMITER_ENTRY_CURRENT_BUDGET_READ(r, p)                            MREAD_32((uint8_t *)p, r)
#define RDD_INGRESS_RATE_LIMITER_ENTRY_CURRENT_BUDGET_WRITE(v, p)                           MWRITE_32((uint8_t *)p, v)
#define RDD_INGRESS_RATE_LIMITER_ENTRY_DROP_THRESHOLD_READ(r, p)                            MREAD_32((uint8_t *)p + 4, r)
#define RDD_INGRESS_RATE_LIMITER_ENTRY_DROP_THRESHOLD_WRITE(v, p)                           MWRITE_32((uint8_t *)p + 4, v)
#define RDD_INGRESS_RATE_LIMITER_ENTRY_XOFF_THRESHOLD_READ(r, p)                            MREAD_32((uint8_t *)p + 8, r)
#define RDD_INGRESS_RATE_LIMITER_ENTRY_XOFF_THRESHOLD_WRITE(v, p)                           MWRITE_32((uint8_t *)p + 8, v)
#define RDD_INGRESS_RATE_LIMITER_ENTRY_ALLOCATED_BUDGET_MANTISA_READ(r, p)                  FIELD_MREAD_16((uint8_t *)p + 12, 2, 14, r)
#define RDD_INGRESS_RATE_LIMITER_ENTRY_ALLOCATED_BUDGET_MANTISA_WRITE(v, p)                 FIELD_MWRITE_16((uint8_t *)p + 12, 2, 14, v)
#define RDD_INGRESS_RATE_LIMITER_ENTRY_ALLOCATED_BUDGET_EXPONENT_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 13, 0, 2, r)
#define RDD_INGRESS_RATE_LIMITER_ENTRY_ALLOCATED_BUDGET_EXPONENT_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 13, 0, 2, v)
#define RDD_INGRESS_RATE_LIMITER_ENTRY_ACTIVE_PAUSE_FLAG_READ(r, p)                         MREAD_8((uint8_t *)p + 14, r)
#define RDD_INGRESS_RATE_LIMITER_ENTRY_ACTIVE_PAUSE_FLAG_WRITE(v, p)                        MWRITE_8((uint8_t *)p + 14, v)
#define RDD_INGRESS_RATE_LIMITER_ENTRY_BBH_RX_ADDRESS_READ(r, p)                            MREAD_8((uint8_t *)p + 15, r)
#define RDD_INGRESS_RATE_LIMITER_ENTRY_BBH_RX_ADDRESS_WRITE(v, p)                           MWRITE_8((uint8_t *)p + 15, v)
#if defined OREN

#define RDD_US_INGRESS_RATE_LIMITER_TABLE_SIZE     5
typedef struct
{
	RDD_INGRESS_RATE_LIMITER_ENTRY_DTS	entry[ RDD_US_INGRESS_RATE_LIMITER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_RATE_LIMITER_TABLE_DTS;

#define RDD_US_INGRESS_RATE_LIMITER_TABLE_PTR()	( RDD_US_INGRESS_RATE_LIMITER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_RATE_LIMITER_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_VLAN_OPTIMIZATION_TABLE_SIZE     128
typedef struct
{
	RDD_VLAN_OPTIMIZATION_ENTRY_DTS	entry[ RDD_US_VLAN_OPTIMIZATION_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_VLAN_OPTIMIZATION_TABLE_DTS;

#define RDD_US_VLAN_OPTIMIZATION_TABLE_PTR()	( RDD_US_VLAN_OPTIMIZATION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_VLAN_OPTIMIZATION_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_INGRESS_CLASSIFICATION_COUNTERS_BUFFER_SIZE     6
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_US_INGRESS_CLASSIFICATION_COUNTERS_BUFFER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_COUNTERS_BUFFER_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_COUNTERS_BUFFER_PTR()	( RDD_US_INGRESS_CLASSIFICATION_COUNTERS_BUFFER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_COUNTERS_BUFFER_ADDRESS )

#endif
#if defined OREN

#define RDD_US_FAST_RUNNER_GLOBAL_REGISTERS_INIT_SIZE     8
typedef struct
{
	RDD_FAST_RUNNER_GLOBAL_REGISTERS_INIT_ENTRY_DTS	entry[ RDD_US_FAST_RUNNER_GLOBAL_REGISTERS_INIT_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_FAST_RUNNER_GLOBAL_REGISTERS_INIT_DTS;

#define RDD_US_FAST_RUNNER_GLOBAL_REGISTERS_INIT_PTR()	( RDD_US_FAST_RUNNER_GLOBAL_REGISTERS_INIT_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FAST_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS )

#endif
#define RDD_ETH_RX_DESCRIPTORS_RESERVED_FW_ONLY_NUMBER	2

typedef struct
{
	uint32_t	reserved_fw_only[RDD_ETH_RX_DESCRIPTORS_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_RX_DESCRIPTORS_DTS;

#if defined OREN

#define RDD_ETH0_RX_DESCRIPTORS_SIZE     32
typedef struct
{
	RDD_ETH_RX_DESCRIPTORS_DTS	entry[ RDD_ETH0_RX_DESCRIPTORS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH0_RX_DESCRIPTORS_DTS;

#define RDD_ETH0_RX_DESCRIPTORS_PTR()	( RDD_ETH0_RX_DESCRIPTORS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + ETH0_RX_DESCRIPTORS_ADDRESS )

#endif
#if defined OREN

#define RDD_US_PBITS_PRIMITIVE_TABLE_SIZE     32
typedef struct
{
	RDD_PBITS_PRIMITIVE_ENTRY_DTS	entry[ RDD_US_PBITS_PRIMITIVE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_PBITS_PRIMITIVE_TABLE_DTS;

#define RDD_US_PBITS_PRIMITIVE_TABLE_PTR()	( RDD_US_PBITS_PRIMITIVE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PBITS_PRIMITIVE_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	bbh_descriptor_0 	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbh_descriptor_1 	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	timer_period     	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	skb_free_index   	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	copies_in_transit	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	total_copies     	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tx_queue_discards	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tx_queue_writes  	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tx_queue_reads   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bucket           	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bucket_size      	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	total_length     	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tokens           	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SPEED_SERVICE_PARAMETERS_DTS;

#define RDD_SPEED_SERVICE_PARAMETERS_BBH_DESCRIPTOR_0_READ(r, p)                  MREAD_32((uint8_t *)p, r)
#define RDD_SPEED_SERVICE_PARAMETERS_BBH_DESCRIPTOR_0_WRITE(v, p)                 MWRITE_32((uint8_t *)p, v)
#define RDD_SPEED_SERVICE_PARAMETERS_BBH_DESCRIPTOR_1_READ(r, p)                  MREAD_32((uint8_t *)p + 4, r)
#define RDD_SPEED_SERVICE_PARAMETERS_BBH_DESCRIPTOR_1_WRITE(v, p)                 MWRITE_32((uint8_t *)p + 4, v)
#define RDD_SPEED_SERVICE_PARAMETERS_TIMER_PERIOD_READ(r, p)                      MREAD_16((uint8_t *)p + 8, r)
#define RDD_SPEED_SERVICE_PARAMETERS_TIMER_PERIOD_WRITE(v, p)                     MWRITE_16((uint8_t *)p + 8, v)
#define RDD_SPEED_SERVICE_PARAMETERS_SKB_FREE_INDEX_READ(r, p)                    MREAD_16((uint8_t *)p + 10, r)
#define RDD_SPEED_SERVICE_PARAMETERS_SKB_FREE_INDEX_WRITE(v, p)                   MWRITE_16((uint8_t *)p + 10, v)
#define RDD_SPEED_SERVICE_PARAMETERS_COPIES_IN_TRANSIT_READ(r, p)                 MREAD_32((uint8_t *)p + 12, r)
#define RDD_SPEED_SERVICE_PARAMETERS_COPIES_IN_TRANSIT_WRITE(v, p)                MWRITE_32((uint8_t *)p + 12, v)
#define RDD_SPEED_SERVICE_PARAMETERS_TOTAL_COPIES_READ(r, p)                      MREAD_32((uint8_t *)p + 16, r)
#define RDD_SPEED_SERVICE_PARAMETERS_TOTAL_COPIES_WRITE(v, p)                     MWRITE_32((uint8_t *)p + 16, v)
#define RDD_SPEED_SERVICE_PARAMETERS_TX_QUEUE_DISCARDS_READ(r, p)                 MREAD_32((uint8_t *)p + 20, r)
#define RDD_SPEED_SERVICE_PARAMETERS_TX_QUEUE_DISCARDS_WRITE(v, p)                MWRITE_32((uint8_t *)p + 20, v)
#define RDD_SPEED_SERVICE_PARAMETERS_TX_QUEUE_WRITES_READ(r, p)                   MREAD_32((uint8_t *)p + 24, r)
#define RDD_SPEED_SERVICE_PARAMETERS_TX_QUEUE_WRITES_WRITE(v, p)                  MWRITE_32((uint8_t *)p + 24, v)
#define RDD_SPEED_SERVICE_PARAMETERS_TX_QUEUE_READS_READ(r, p)                    MREAD_32((uint8_t *)p + 28, r)
#define RDD_SPEED_SERVICE_PARAMETERS_TX_QUEUE_READS_WRITE(v, p)                   MWRITE_32((uint8_t *)p + 28, v)
#define RDD_SPEED_SERVICE_PARAMETERS_BUCKET_READ(r, p)                            MREAD_32((uint8_t *)p + 32, r)
#define RDD_SPEED_SERVICE_PARAMETERS_BUCKET_WRITE(v, p)                           MWRITE_32((uint8_t *)p + 32, v)
#define RDD_SPEED_SERVICE_PARAMETERS_BUCKET_SIZE_READ(r, p)                       MREAD_16((uint8_t *)p + 36, r)
#define RDD_SPEED_SERVICE_PARAMETERS_BUCKET_SIZE_WRITE(v, p)                      MWRITE_16((uint8_t *)p + 36, v)
#define RDD_SPEED_SERVICE_PARAMETERS_TOTAL_LENGTH_READ(r, p)                      MREAD_16((uint8_t *)p + 38, r)
#define RDD_SPEED_SERVICE_PARAMETERS_TOTAL_LENGTH_WRITE(v, p)                     MWRITE_16((uint8_t *)p + 38, v)
#define RDD_SPEED_SERVICE_PARAMETERS_TOKENS_READ(r, p)                            MREAD_16((uint8_t *)p + 40, r)
#define RDD_SPEED_SERVICE_PARAMETERS_TOKENS_WRITE(v, p)                           MWRITE_16((uint8_t *)p + 40, v)
#if defined OREN

#define RDD_US_LAYER4_FILTERS_CONTEXT_TABLE_SIZE     16
typedef struct
{
	RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DTS	entry[ RDD_US_LAYER4_FILTERS_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_LAYER4_FILTERS_CONTEXT_TABLE_DTS;

#define RDD_US_LAYER4_FILTERS_CONTEXT_TABLE_PTR()	( RDD_US_LAYER4_FILTERS_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_LAYER4_FILTERS_CONTEXT_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_CPU_RX_PICO_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_US_CPU_RX_PICO_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_CPU_RX_PICO_INGRESS_QUEUE_DTS;

#define RDD_US_CPU_RX_PICO_INGRESS_QUEUE_PTR()	( RDD_US_CPU_RX_PICO_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_RX_PICO_INGRESS_QUEUE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_ACK_PACKETS_QUEUE_INDEX_TABLE_SIZE     40
typedef struct
{
	RDD_US_QUEUE_ENTRY_DTS	entry[ RDD_US_ACK_PACKETS_QUEUE_INDEX_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_ACK_PACKETS_QUEUE_INDEX_TABLE_DTS;

#define RDD_US_ACK_PACKETS_QUEUE_INDEX_TABLE_PTR()	( RDD_US_ACK_PACKETS_QUEUE_INDEX_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_ACK_PACKETS_QUEUE_INDEX_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_SIZE     3
#define RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_SIZE2    6
typedef struct
{
	RDD_CPU_REASON_TO_METER_ENTRY_DTS	entry[ RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_SIZE ][ RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_DTS;

#define RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_PTR()	( RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_ETH1_RX_DESCRIPTORS_SIZE     32
typedef struct
{
	RDD_ETH_RX_DESCRIPTORS_DTS	entry[ RDD_ETH1_RX_DESCRIPTORS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH1_RX_DESCRIPTORS_DTS;

#define RDD_ETH1_RX_DESCRIPTORS_PTR()	( RDD_ETH1_RX_DESCRIPTORS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + ETH1_RX_DESCRIPTORS_ADDRESS )

#endif
#if defined OREN

#define RDD_US_ROUTER_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_US_ROUTER_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_ROUTER_INGRESS_QUEUE_DTS;

#define RDD_US_ROUTER_INGRESS_QUEUE_PTR()	( RDD_US_ROUTER_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_ROUTER_INGRESS_QUEUE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_VLAN_PRIMITIVE_TABLE_SIZE     32
typedef struct
{
	RDD_VLAN_PRIMITIVE_ENTRY_DTS	entry[ RDD_US_VLAN_PRIMITIVE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_VLAN_PRIMITIVE_TABLE_DTS;

#define RDD_US_VLAN_PRIMITIVE_TABLE_PTR()	( RDD_US_VLAN_PRIMITIVE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_VLAN_PRIMITIVE_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_CPU_RX_FAST_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_US_CPU_RX_FAST_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_CPU_RX_FAST_INGRESS_QUEUE_DTS;

#define RDD_US_CPU_RX_FAST_INGRESS_QUEUE_PTR()	( RDD_US_CPU_RX_FAST_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_RX_FAST_INGRESS_QUEUE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_GPON_RX_DIRECT_DESCRIPTORS_SIZE     32
typedef struct
{
	RDD_BBH_RX_DESCRIPTOR_DTS	entry[ RDD_US_GPON_RX_DIRECT_DESCRIPTORS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_GPON_RX_DIRECT_DESCRIPTORS_DTS;

#define RDD_US_GPON_RX_DIRECT_DESCRIPTORS_PTR()	( RDD_US_GPON_RX_DIRECT_DESCRIPTORS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_GPON_RX_DIRECT_DESCRIPTORS_ADDRESS )

#endif
#if defined OREN

#define RDD_US_DEBUG_BUFFER_SIZE     32
typedef struct
{
	RDD_DEBUG_BUFFER_ENTRY_DTS	entry[ RDD_US_DEBUG_BUFFER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_DEBUG_BUFFER_DTS;

#define RDD_US_DEBUG_BUFFER_PTR()	( RDD_US_DEBUG_BUFFER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_DEBUG_BUFFER_ADDRESS )

#endif
#if defined OREN

#define RDD_LOCAL_SWITCHING_MULTICAST_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_LOCAL_SWITCHING_MULTICAST_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LOCAL_SWITCHING_MULTICAST_INGRESS_QUEUE_DTS;

#define RDD_LOCAL_SWITCHING_MULTICAST_INGRESS_QUEUE_PTR()	( RDD_LOCAL_SWITCHING_MULTICAST_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + LOCAL_SWITCHING_MULTICAST_INGRESS_QUEUE_ADDRESS )

#endif
#if defined OREN

#define RDD_VLAN_ACTION_BRIDGE_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_VLAN_ACTION_BRIDGE_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_VLAN_ACTION_BRIDGE_INGRESS_QUEUE_DTS;

#define RDD_VLAN_ACTION_BRIDGE_INGRESS_QUEUE_PTR()	( RDD_VLAN_ACTION_BRIDGE_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + VLAN_ACTION_BRIDGE_INGRESS_QUEUE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_PICO_RUNNER_GLOBAL_REGISTERS_INIT_SIZE     8
typedef struct
{
	RDD_PICO_RUNNER_GLOBAL_REGISTERS_INIT_ENTRY_DTS	entry[ RDD_US_PICO_RUNNER_GLOBAL_REGISTERS_INIT_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_PICO_RUNNER_GLOBAL_REGISTERS_INIT_DTS;

#define RDD_US_PICO_RUNNER_GLOBAL_REGISTERS_INIT_PTR()	( RDD_US_PICO_RUNNER_GLOBAL_REGISTERS_INIT_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PICO_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS )

#endif
#if defined OREN

#define RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_SIZE     6
typedef struct
{
	RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS	entry[ RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS;

#define RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR()	( RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_FILTERS_CONFIGURATION_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	receive_errors 	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	transmit_errors	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SMART_CARD_ERROR_COUNTERS_ENTRY_DTS;

#define RDD_SMART_CARD_ERROR_COUNTERS_ENTRY_RECEIVE_ERRORS_READ(r, p)                  MREAD_32((uint8_t *)p, r)
#define RDD_SMART_CARD_ERROR_COUNTERS_ENTRY_RECEIVE_ERRORS_WRITE(v, p)                 MWRITE_32((uint8_t *)p, v)
#define RDD_SMART_CARD_ERROR_COUNTERS_ENTRY_TRANSMIT_ERRORS_READ(r, p)                 MREAD_32((uint8_t *)p + 4, r)
#define RDD_SMART_CARD_ERROR_COUNTERS_ENTRY_TRANSMIT_ERRORS_WRITE(v, p)                MWRITE_32((uint8_t *)p + 4, v)
#if defined OREN

#define RDD_UPSTREAM_FLOODING_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_UPSTREAM_FLOODING_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_UPSTREAM_FLOODING_INGRESS_QUEUE_DTS;

#define RDD_UPSTREAM_FLOODING_INGRESS_QUEUE_PTR()	( RDD_UPSTREAM_FLOODING_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + UPSTREAM_FLOODING_INGRESS_QUEUE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_RUNNER_FLOW_HEADER_DESCRIPTOR_SIZE     3
typedef struct
{
	RDD_RUNNER_FLOW_HEADER_DESCRIPTOR_DTS	entry[ RDD_US_RUNNER_FLOW_HEADER_DESCRIPTOR_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_RUNNER_FLOW_HEADER_DESCRIPTOR_DTS;

#define RDD_US_RUNNER_FLOW_HEADER_DESCRIPTOR_PTR()	( RDD_US_RUNNER_FLOW_HEADER_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_RUNNER_FLOW_HEADER_DESCRIPTOR_ADDRESS )

#endif
#if defined OREN

#define RDD_US_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_CAM_SIZE     4
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_US_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_CAM_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_CAM_DTS;

#define RDD_US_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_CAM_PTR()	( RDD_US_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_CAM_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_CAM_ADDRESS )

#endif
#if defined OREN

#define RDD_US_MULTICAST_VECTOR_TO_PORT_TABLE_SIZE     8
typedef struct
{
	RDD_MULTICAST_VECTOR_TO_PORT_ENTRY_DTS	entry[ RDD_US_MULTICAST_VECTOR_TO_PORT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_MULTICAST_VECTOR_TO_PORT_TABLE_DTS;

#define RDD_US_MULTICAST_VECTOR_TO_PORT_TABLE_PTR()	( RDD_US_MULTICAST_VECTOR_TO_PORT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_MULTICAST_VECTOR_TO_PORT_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_WAN_INTERWORKING_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_WAN_INTERWORKING_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WAN_INTERWORKING_INGRESS_QUEUE_DTS;

#define RDD_WAN_INTERWORKING_INGRESS_QUEUE_PTR()	( RDD_WAN_INTERWORKING_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + WAN_INTERWORKING_INGRESS_QUEUE_ADDRESS )

#endif

typedef struct
{
	uint16_t	wan_channel_ptr	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_ENTRY_DTS;

#define RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_ENTRY_WAN_CHANNEL_PTR_READ(r, p)                 MREAD_16((uint8_t *)p, r)
#define RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_ENTRY_WAN_CHANNEL_PTR_WRITE(v, p)                MWRITE_16((uint8_t *)p, v)
#if defined OREN

#define RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_TABLE_SIZE     48
typedef struct
{
	RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_ENTRY_DTS	entry[ RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_TABLE_DTS;

#define RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_TABLE_PTR()	( RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	ingress_fifo_ptr                 	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	broadcom_switch_task_wakeup_value	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY_DTS;

#define RDD_LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY_INGRESS_FIFO_PTR_READ(r, p)                                  MREAD_16((uint8_t *)p, r)
#define RDD_LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY_INGRESS_FIFO_PTR_WRITE(v, p)                                 MWRITE_16((uint8_t *)p, v)
#define RDD_LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY_BROADCOM_SWITCH_TASK_WAKEUP_VALUE_READ(r, p)                 MREAD_16((uint8_t *)p + 2, r)
#define RDD_LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY_BROADCOM_SWITCH_TASK_WAKEUP_VALUE_WRITE(v, p)                MWRITE_16((uint8_t *)p + 2, v)
#if defined OREN

#define RDD_LAN_INGRESS_FIFO_DESCRIPTOR_TABLE_SIZE     5
typedef struct
{
	RDD_LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY_DTS	entry[ RDD_LAN_INGRESS_FIFO_DESCRIPTOR_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LAN_INGRESS_FIFO_DESCRIPTOR_TABLE_DTS;

#define RDD_LAN_INGRESS_FIFO_DESCRIPTOR_TABLE_PTR()	( RDD_LAN_INGRESS_FIFO_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + LAN_INGRESS_FIFO_DESCRIPTOR_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_GPON_ABSOLUTE_TX_FIRMWARE_COUNTER_SIZE     40
typedef struct
{
	RDD_GPON_ABSOLUTE_TX_COUNTER_DTS	entry[ RDD_GPON_ABSOLUTE_TX_FIRMWARE_COUNTER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GPON_ABSOLUTE_TX_FIRMWARE_COUNTER_DTS;

#define RDD_GPON_ABSOLUTE_TX_FIRMWARE_COUNTER_PTR()	( RDD_GPON_ABSOLUTE_TX_FIRMWARE_COUNTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + GPON_ABSOLUTE_TX_FIRMWARE_COUNTER_ADDRESS )

#endif
#define RDD_US_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_PTR()	( RDD_US_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_ADDRESS )

#if defined OREN

#define RDD_ETH2_RX_DESCRIPTORS_SIZE     32
typedef struct
{
	RDD_ETH_RX_DESCRIPTORS_DTS	entry[ RDD_ETH2_RX_DESCRIPTORS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH2_RX_DESCRIPTORS_DTS;

#define RDD_ETH2_RX_DESCRIPTORS_PTR()	( RDD_ETH2_RX_DESCRIPTORS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + ETH2_RX_DESCRIPTORS_ADDRESS )

#endif
#define RDD_US_PICO_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_PTR()	( RDD_US_PICO_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PICO_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_ADDRESS )


typedef struct
{
	uint8_t	flow_id   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_ENTRY_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_ENTRY_FLOW_ID_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_ENTRY_FLOW_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#if defined OREN

#define RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_TABLE_SIZE     32
typedef struct
{
	RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_ENTRY_DTS	entry[ RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_TABLE_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_TABLE_PTR()	( RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_TABLE_ADDRESS )

#endif
#define RDD_IPV6_LOCAL_IP_RESERVED_FW_ONLY_NUMBER	14

typedef struct
{
	uint8_t	reserved_fw_only[RDD_IPV6_LOCAL_IP_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPV6_LOCAL_IP_DTS;

#if defined OREN

#define RDD_LOCAL_SWITCHING_MODE_TABLE_SIZE     6
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_LOCAL_SWITCHING_MODE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LOCAL_SWITCHING_MODE_TABLE_DTS;

#define RDD_LOCAL_SWITCHING_MODE_TABLE_PTR()	( RDD_LOCAL_SWITCHING_MODE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + LOCAL_SWITCHING_MODE_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_SIZE     4
typedef struct
{
	RDD_PARALLEL_PROCESSING_ENTRY_DTS	entry[ RDD_US_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_DTS;

#define RDD_US_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_PTR()	( RDD_US_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PARALLEL_PROCESSING_IH_BUFFER_VECTOR_ADDRESS )

#endif
#if defined OREN

#define RDD_US_PARALLEL_PROCESSING_TASK_REORDER_FIFO_SIZE     4
typedef struct
{
	RDD_PARALLEL_PROCESSING_TASK_REORDER_ENTRY_DTS	entry[ RDD_US_PARALLEL_PROCESSING_TASK_REORDER_FIFO_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_PARALLEL_PROCESSING_TASK_REORDER_FIFO_DTS;

#define RDD_US_PARALLEL_PROCESSING_TASK_REORDER_FIFO_PTR()	( RDD_US_PARALLEL_PROCESSING_TASK_REORDER_FIFO_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PARALLEL_PROCESSING_TASK_REORDER_FIFO_ADDRESS )

#endif

typedef struct
{
	uint8_t	physical_port	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BROADCOM_SWITCH_PORT_MAPPING_DTS;

#define RDD_BROADCOM_SWITCH_PORT_MAPPING_PHYSICAL_PORT_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_BROADCOM_SWITCH_PORT_MAPPING_PHYSICAL_PORT_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#if defined OREN

#define RDD_BROADCOM_SWITCH_PORT_TO_BRIDGE_PORT_MAPPING_TABLE_SIZE     8
typedef struct
{
	RDD_BROADCOM_SWITCH_PORT_MAPPING_DTS	entry[ RDD_BROADCOM_SWITCH_PORT_TO_BRIDGE_PORT_MAPPING_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BROADCOM_SWITCH_PORT_TO_BRIDGE_PORT_MAPPING_TABLE_DTS;

#define RDD_BROADCOM_SWITCH_PORT_TO_BRIDGE_PORT_MAPPING_TABLE_PTR()	( RDD_BROADCOM_SWITCH_PORT_TO_BRIDGE_PORT_MAPPING_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + BROADCOM_SWITCH_PORT_TO_BRIDGE_PORT_MAPPING_TABLE_ADDRESS )

#endif

typedef struct
{
	uint8_t	reserved   	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	egress_cfg 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	ingress_cfg	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_VLAN_SWITCHING_ISOLATION_CONFIG_ENTRY_DTS;

#define RDD_VLAN_SWITCHING_ISOLATION_CONFIG_ENTRY_EGRESS_CFG_READ(r, p)                  FIELD_MREAD_8((uint8_t *)p, 1, 1, r)
#define RDD_VLAN_SWITCHING_ISOLATION_CONFIG_ENTRY_EGRESS_CFG_WRITE(v, p)                 FIELD_MWRITE_8((uint8_t *)p, 1, 1, v)
#define RDD_VLAN_SWITCHING_ISOLATION_CONFIG_ENTRY_INGRESS_CFG_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 0, 1, r)
#define RDD_VLAN_SWITCHING_ISOLATION_CONFIG_ENTRY_INGRESS_CFG_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 0, 1, v)
#if defined OREN

#define RDD_INGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_SIZE     5
typedef struct
{
	RDD_VLAN_SWITCHING_ISOLATION_CONFIG_ENTRY_DTS	entry[ RDD_INGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_DTS;

#define RDD_INGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_PTR()	( RDD_INGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + INGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_ADDRESS )

#endif

typedef struct
{
	uint16_t	entry     	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LOCAL_SWITCHING_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_PTR_DTS;

#define RDD_LOCAL_SWITCHING_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_PTR_ENTRY_READ(r, p)                 MREAD_16((uint8_t *)p, r)
#define RDD_LOCAL_SWITCHING_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_PTR_ENTRY_WRITE(v, p)                MWRITE_16((uint8_t *)p, v)

typedef struct
{
	uint16_t	reserved_fw_only	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_PTR_DTS;

#if defined OREN

#define RDD_US_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_TABLE_SIZE     4
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_US_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_TABLE_DTS;

#define RDD_US_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_TABLE_PTR()	( RDD_US_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PARALLEL_PROCESSING_CONTEXT_INDEX_CACHE_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_ETH3_RX_DESCRIPTORS_SIZE     32
typedef struct
{
	RDD_ETH_RX_DESCRIPTORS_DTS	entry[ RDD_ETH3_RX_DESCRIPTORS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH3_RX_DESCRIPTORS_DTS;

#define RDD_ETH3_RX_DESCRIPTORS_PTR()	( RDD_ETH3_RX_DESCRIPTORS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + ETH3_RX_DESCRIPTORS_ADDRESS )

#endif
#define RDD_BBH_TX_WAN_CHANNEL_INDEX_RESERVED_FW_ONLY_NUMBER	2

typedef struct
{
	uint32_t	reserved_fw_only[RDD_BBH_TX_WAN_CHANNEL_INDEX_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BBH_TX_WAN_CHANNEL_INDEX_DTS;

#if defined OREN

#define RDD_ETH4_RX_DESCRIPTORS_SIZE     32
typedef struct
{
	RDD_ETH_RX_DESCRIPTORS_DTS	entry[ RDD_ETH4_RX_DESCRIPTORS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH4_RX_DESCRIPTORS_DTS;

#define RDD_ETH4_RX_DESCRIPTORS_PTR()	( RDD_ETH4_RX_DESCRIPTORS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + ETH4_RX_DESCRIPTORS_ADDRESS )

#endif
/* COMMON_A */

typedef struct
{
	uint32_t	user_defined	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_addr0   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_addr1   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_addr2   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_addr3   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_addr4   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_addr5   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	aging       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	skip        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	valid       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MAC_ENTRY_DTS;

#define RDD_MAC_ENTRY_USER_DEFINED_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p, 4, 12, r)
#define RDD_MAC_ENTRY_USER_DEFINED_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p, 4, 12, v)
#define RDD_MAC_ENTRY_MAC_ADDR0_READ(r, p)                    FIELD_MREAD_32((uint8_t *)p + 0, 12, 8, r)
#define RDD_MAC_ENTRY_MAC_ADDR0_WRITE(v, p)                   FIELD_MWRITE_32((uint8_t *)p + 0, 12, 8, v)
#define RDD_MAC_ENTRY_MAC_ADDR1_READ(r, p)                    FIELD_MREAD_16((uint8_t *)p + 2, 4, 8, r)
#define RDD_MAC_ENTRY_MAC_ADDR1_WRITE(v, p)                   FIELD_MWRITE_16((uint8_t *)p + 2, 4, 8, v)
#define RDD_MAC_ENTRY_MAC_ADDR2_READ(r, p)                    { uint32_t temp; FIELD_MREAD_32((uint8_t *)p, 0, 4, temp); r = temp << 4; FIELD_MREAD_32(((uint8_t *)p + 4), 28, 4, temp); r = r | temp; }
#define RDD_MAC_ENTRY_MAC_ADDR2_WRITE(v, p)                   { FIELD_MWRITE_32((uint8_t *)p, 0, 4, ((v & 0xfffffff0) >> 4)); FIELD_MWRITE_32(((uint8_t *)p + 4), 28, 4, (v & 0x0000000f)); }
#define RDD_MAC_ENTRY_MAC_ADDR3_READ(r, p)                    FIELD_MREAD_16((uint8_t *)p + 4, 4, 8, r)
#define RDD_MAC_ENTRY_MAC_ADDR3_WRITE(v, p)                   FIELD_MWRITE_16((uint8_t *)p + 4, 4, 8, v)
#define RDD_MAC_ENTRY_MAC_ADDR4_READ(r, p)                    FIELD_MREAD_32((uint8_t *)p + 4, 12, 8, r)
#define RDD_MAC_ENTRY_MAC_ADDR4_WRITE(v, p)                   FIELD_MWRITE_32((uint8_t *)p + 4, 12, 8, v)
#define RDD_MAC_ENTRY_MAC_ADDR5_READ(r, p)                    FIELD_MREAD_16((uint8_t *)p + 6, 4, 8, r)
#define RDD_MAC_ENTRY_MAC_ADDR5_WRITE(v, p)                   FIELD_MWRITE_16((uint8_t *)p + 6, 4, 8, v)
#define RDD_MAC_ENTRY_AGING_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 7, 2, 1, r)
#define RDD_MAC_ENTRY_AGING_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 7, 2, 1, v)
#define RDD_MAC_ENTRY_SKIP_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r)
#define RDD_MAC_ENTRY_SKIP_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 7, 1, 1, v)
#define RDD_MAC_ENTRY_VALID_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 7, 0, 1, r)
#define RDD_MAC_ENTRY_VALID_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 7, 0, 1, v)
#if defined OREN

#define RDD_MAC_TABLE_SIZE     1024
typedef struct
{
	RDD_MAC_ENTRY_DTS	entry[ RDD_MAC_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MAC_TABLE_DTS;

#define RDD_MAC_TABLE_PTR()	( RDD_MAC_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	vid       	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_addr0 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_addr1 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_addr2 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_addr3 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_addr4 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_addr5 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	excl      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	aging     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	skip      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	valid     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_L2_LOOKUP_ENTRY_DTS;

#define RDD_IPTV_L2_LOOKUP_ENTRY_VID_READ(r, p)                       FIELD_MREAD_16((uint8_t *)p, 4, 12, r)
#define RDD_IPTV_L2_LOOKUP_ENTRY_VID_WRITE(v, p)                      FIELD_MWRITE_16((uint8_t *)p, 4, 12, v)
#define RDD_IPTV_L2_LOOKUP_ENTRY_MAC_ADDR0_READ(r, p)                 FIELD_MREAD_32((uint8_t *)p + 0, 12, 8, r)
#define RDD_IPTV_L2_LOOKUP_ENTRY_MAC_ADDR0_WRITE(v, p)                FIELD_MWRITE_32((uint8_t *)p + 0, 12, 8, v)
#define RDD_IPTV_L2_LOOKUP_ENTRY_MAC_ADDR1_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p + 2, 4, 8, r)
#define RDD_IPTV_L2_LOOKUP_ENTRY_MAC_ADDR1_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p + 2, 4, 8, v)
#define RDD_IPTV_L2_LOOKUP_ENTRY_MAC_ADDR2_READ(r, p)                 { uint32_t temp; FIELD_MREAD_32((uint8_t *)p, 0, 4, temp); r = temp << 4; FIELD_MREAD_32(((uint8_t *)p + 4), 28, 4, temp); r = r | temp; }
#define RDD_IPTV_L2_LOOKUP_ENTRY_MAC_ADDR2_WRITE(v, p)                { FIELD_MWRITE_32((uint8_t *)p, 0, 4, ((v & 0xfffffff0) >> 4)); FIELD_MWRITE_32(((uint8_t *)p + 4), 28, 4, (v & 0x0000000f)); }
#define RDD_IPTV_L2_LOOKUP_ENTRY_MAC_ADDR3_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p + 4, 4, 8, r)
#define RDD_IPTV_L2_LOOKUP_ENTRY_MAC_ADDR3_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p + 4, 4, 8, v)
#define RDD_IPTV_L2_LOOKUP_ENTRY_MAC_ADDR4_READ(r, p)                 FIELD_MREAD_32((uint8_t *)p + 4, 12, 8, r)
#define RDD_IPTV_L2_LOOKUP_ENTRY_MAC_ADDR4_WRITE(v, p)                FIELD_MWRITE_32((uint8_t *)p + 4, 12, 8, v)
#define RDD_IPTV_L2_LOOKUP_ENTRY_MAC_ADDR5_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p + 6, 4, 8, r)
#define RDD_IPTV_L2_LOOKUP_ENTRY_MAC_ADDR5_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p + 6, 4, 8, v)
#define RDD_IPTV_L2_LOOKUP_ENTRY_EXCL_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 7, 3, 1, r)
#define RDD_IPTV_L2_LOOKUP_ENTRY_EXCL_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 7, 3, 1, v)
#define RDD_IPTV_L2_LOOKUP_ENTRY_AGING_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 7, 2, 1, r)
#define RDD_IPTV_L2_LOOKUP_ENTRY_AGING_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 7, 2, 1, v)
#define RDD_IPTV_L2_LOOKUP_ENTRY_SKIP_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r)
#define RDD_IPTV_L2_LOOKUP_ENTRY_SKIP_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 7, 1, 1, v)
#define RDD_IPTV_L2_LOOKUP_ENTRY_VALID_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 7, 0, 1, r)
#define RDD_IPTV_L2_LOOKUP_ENTRY_VALID_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 7, 0, 1, v)

typedef struct
{
	uint32_t	reserved     	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	context_table	:10	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	context_valid	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	any          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vid          	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dest_ip0     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dest_ip1     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dest_ip2     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dest_ip3     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	excl         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	aging        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	skip         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	valid        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_L3_LOOKUP_ENTRY_DTS;

#define RDD_IPTV_L3_LOOKUP_ENTRY_CONTEXT_TABLE_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p, 2, 10, r)
#define RDD_IPTV_L3_LOOKUP_ENTRY_CONTEXT_TABLE_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p, 2, 10, v)
#define RDD_IPTV_L3_LOOKUP_ENTRY_CONTEXT_VALID_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 1, 1, 1, r)
#define RDD_IPTV_L3_LOOKUP_ENTRY_CONTEXT_VALID_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 1, 1, 1, v)
#define RDD_IPTV_L3_LOOKUP_ENTRY_ANY_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 1, 0, 1, r)
#define RDD_IPTV_L3_LOOKUP_ENTRY_ANY_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 1, 0, 1, v)
#define RDD_IPTV_L3_LOOKUP_ENTRY_VID_READ(r, p)                           FIELD_MREAD_16((uint8_t *)p + 2, 4, 12, r)
#define RDD_IPTV_L3_LOOKUP_ENTRY_VID_WRITE(v, p)                          FIELD_MWRITE_16((uint8_t *)p + 2, 4, 12, v)
#define RDD_IPTV_L3_LOOKUP_ENTRY_DEST_IP0_READ(r, p)                      { uint32_t temp; FIELD_MREAD_32((uint8_t *)p, 0, 4, temp); r = temp << 4; FIELD_MREAD_32(((uint8_t *)p + 4), 28, 4, temp); r = r | temp; }
#define RDD_IPTV_L3_LOOKUP_ENTRY_DEST_IP0_WRITE(v, p)                     { FIELD_MWRITE_32((uint8_t *)p, 0, 4, ((v & 0xfffffff0) >> 4)); FIELD_MWRITE_32(((uint8_t *)p + 4), 28, 4, (v & 0x0000000f)); }
#define RDD_IPTV_L3_LOOKUP_ENTRY_DEST_IP1_READ(r, p)                      FIELD_MREAD_16((uint8_t *)p + 4, 4, 8, r)
#define RDD_IPTV_L3_LOOKUP_ENTRY_DEST_IP1_WRITE(v, p)                     FIELD_MWRITE_16((uint8_t *)p + 4, 4, 8, v)
#define RDD_IPTV_L3_LOOKUP_ENTRY_DEST_IP2_READ(r, p)                      FIELD_MREAD_32((uint8_t *)p + 4, 12, 8, r)
#define RDD_IPTV_L3_LOOKUP_ENTRY_DEST_IP2_WRITE(v, p)                     FIELD_MWRITE_32((uint8_t *)p + 4, 12, 8, v)
#define RDD_IPTV_L3_LOOKUP_ENTRY_DEST_IP3_READ(r, p)                      FIELD_MREAD_16((uint8_t *)p + 6, 4, 8, r)
#define RDD_IPTV_L3_LOOKUP_ENTRY_DEST_IP3_WRITE(v, p)                     FIELD_MWRITE_16((uint8_t *)p + 6, 4, 8, v)
#define RDD_IPTV_L3_LOOKUP_ENTRY_EXCL_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 7, 3, 1, r)
#define RDD_IPTV_L3_LOOKUP_ENTRY_EXCL_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 7, 3, 1, v)
#define RDD_IPTV_L3_LOOKUP_ENTRY_AGING_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 7, 2, 1, r)
#define RDD_IPTV_L3_LOOKUP_ENTRY_AGING_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 7, 2, 1, v)
#define RDD_IPTV_L3_LOOKUP_ENTRY_SKIP_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r)
#define RDD_IPTV_L3_LOOKUP_ENTRY_SKIP_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 7, 1, 1, v)
#define RDD_IPTV_L3_LOOKUP_ENTRY_VALID_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 7, 0, 1, r)
#define RDD_IPTV_L3_LOOKUP_ENTRY_VALID_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 7, 0, 1, v)

typedef struct
{
	uint16_t	multicast       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	multicast_vector	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	egress_port     	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	move_indication 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	mac_type        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	da_action       	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	sa_action       	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MAC_CONTEXT_ENTRY_DTS;

#define RDD_MAC_CONTEXT_ENTRY_MULTICAST_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_MAC_CONTEXT_ENTRY_MULTICAST_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_MAC_CONTEXT_ENTRY_MULTICAST_VECTOR_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 5, 2, r)
#define RDD_MAC_CONTEXT_ENTRY_MULTICAST_VECTOR_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 5, 2, v)
#define RDD_MAC_CONTEXT_ENTRY_EGRESS_PORT_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p, 0, 5, r)
#define RDD_MAC_CONTEXT_ENTRY_EGRESS_PORT_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p, 0, 5, v)
#define RDD_MAC_CONTEXT_ENTRY_MOVE_INDICATION_READ(r, p)                  FIELD_MREAD_8((uint8_t *)p + 1, 7, 1, r)
#define RDD_MAC_CONTEXT_ENTRY_MOVE_INDICATION_WRITE(v, p)                 FIELD_MWRITE_8((uint8_t *)p + 1, 7, 1, v)
#define RDD_MAC_CONTEXT_ENTRY_MAC_TYPE_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 1, 6, 1, r)
#define RDD_MAC_CONTEXT_ENTRY_MAC_TYPE_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 1, 6, 1, v)
#define RDD_MAC_CONTEXT_ENTRY_DA_ACTION_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 1, 3, 3, r)
#define RDD_MAC_CONTEXT_ENTRY_DA_ACTION_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 1, 3, 3, v)
#define RDD_MAC_CONTEXT_ENTRY_SA_ACTION_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 1, 0, 3, r)
#define RDD_MAC_CONTEXT_ENTRY_SA_ACTION_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 1, 0, 3, v)
#if defined OREN

#define RDD_MAC_CONTEXT_TABLE_SIZE     1024
typedef struct
{
	RDD_MAC_CONTEXT_ENTRY_DTS	entry[ RDD_MAC_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MAC_CONTEXT_TABLE_DTS;

#define RDD_MAC_CONTEXT_TABLE_PTR()	( RDD_MAC_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_CONTEXT_TABLE_ADDRESS )

#endif
#if defined OREN

typedef struct
{
	uint16_t	port_vector_info  	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = port_vector_info, size = 16 bits
	uint16_t	replication_number	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	egress_port_vector	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_CONTEXT_ENTRY_DTS;

#define RDD_IPTV_CONTEXT_ENTRY_PORT_VECTOR_INFO_READ(r, p)                   MREAD_16((uint8_t *)p, r)
#define RDD_IPTV_CONTEXT_ENTRY_PORT_VECTOR_INFO_WRITE(v, p)                  MWRITE_16((uint8_t *)p, v)
#define RDD_IPTV_CONTEXT_ENTRY_PORT_VECTOR_INFO_L_READ( wv )                 FIELD_GET( wv, 0, 16 )
#define RDD_IPTV_CONTEXT_ENTRY_PORT_VECTOR_INFO_L_WRITE( v, wv )             FIELD_SET( v, 0, 16, wv )
#define RDD_IPTV_CONTEXT_ENTRY_REPLICATION_NUMBER_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_IPTV_CONTEXT_ENTRY_REPLICATION_NUMBER_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#define RDD_IPTV_CONTEXT_ENTRY_REPLICATION_NUMBER_L_READ( wv )               FIELD_GET( wv, 8, 8 )
#define RDD_IPTV_CONTEXT_ENTRY_REPLICATION_NUMBER_L_WRITE( v, wv )           FIELD_SET( v, 8, 8, wv )
#define RDD_IPTV_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_READ(r, p)                 MREAD_8((uint8_t *)p + 1, r)
#define RDD_IPTV_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_WRITE(v, p)                MWRITE_8((uint8_t *)p + 1, v)
#define RDD_IPTV_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_L_READ( wv )               FIELD_GET( wv, 0, 8 )
#define RDD_IPTV_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_L_WRITE( v, wv )           FIELD_SET( v, 0, 8, wv )
#endif
#if defined OREN

#define RDD_IPTV_CONTEXT_TABLE_SIZE     256
typedef struct
{
	RDD_IPTV_CONTEXT_ENTRY_DTS	entry[ RDD_IPTV_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_CONTEXT_TABLE_DTS;

#define RDD_IPTV_CONTEXT_TABLE_PTR()	( RDD_IPTV_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + IPTV_CONTEXT_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	head_idx            	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tail_idx            	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	packet_counter      	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	valid_entries_number	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tail_entry          	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	head_entry          	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	packet_threshold    	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	queue_state         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	profile_id          	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tail_base_entry     	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	head_base_entry     	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	profile_en          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	union_field1        	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = union_field1, size = 7 bits
	uint32_t	reserved0           	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	drop_bit            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_controller_id  	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	queue_mask          	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cache_ptr           	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DDR_QUEUE_DESCRIPTOR_DTS;

#define RDD_DDR_QUEUE_DESCRIPTOR_HEAD_IDX_READ(r, p)                             MREAD_16((uint8_t *)p, r)
#define RDD_DDR_QUEUE_DESCRIPTOR_HEAD_IDX_WRITE(v, p)                            MWRITE_16((uint8_t *)p, v)
#define RDD_DDR_QUEUE_DESCRIPTOR_TAIL_IDX_READ(r, p)                             MREAD_16((uint8_t *)p + 2, r)
#define RDD_DDR_QUEUE_DESCRIPTOR_TAIL_IDX_WRITE(v, p)                            MWRITE_16((uint8_t *)p + 2, v)
#define RDD_DDR_QUEUE_DESCRIPTOR_PACKET_COUNTER_READ(r, p)                       MREAD_16((uint8_t *)p + 4, r)
#define RDD_DDR_QUEUE_DESCRIPTOR_PACKET_COUNTER_WRITE(v, p)                      MWRITE_16((uint8_t *)p + 4, v)
#define RDD_DDR_QUEUE_DESCRIPTOR_VALID_ENTRIES_NUMBER_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 6, 4, 4, r)
#define RDD_DDR_QUEUE_DESCRIPTOR_VALID_ENTRIES_NUMBER_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 6, 4, 4, v)
#define RDD_DDR_QUEUE_DESCRIPTOR_TAIL_ENTRY_READ(r, p)                           FIELD_MREAD_16((uint8_t *)p + 6, 6, 6, r)
#define RDD_DDR_QUEUE_DESCRIPTOR_TAIL_ENTRY_WRITE(v, p)                          FIELD_MWRITE_16((uint8_t *)p + 6, 6, 6, v)
#define RDD_DDR_QUEUE_DESCRIPTOR_HEAD_ENTRY_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 7, 0, 6, r)
#define RDD_DDR_QUEUE_DESCRIPTOR_HEAD_ENTRY_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 7, 0, 6, v)
#define RDD_DDR_QUEUE_DESCRIPTOR_PACKET_THRESHOLD_READ(r, p)                     MREAD_16((uint8_t *)p + 8, r)
#define RDD_DDR_QUEUE_DESCRIPTOR_PACKET_THRESHOLD_WRITE(v, p)                    MWRITE_16((uint8_t *)p + 8, v)
#define RDD_DDR_QUEUE_DESCRIPTOR_QUEUE_STATE_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 10, 7, 1, r)
#define RDD_DDR_QUEUE_DESCRIPTOR_QUEUE_STATE_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 10, 7, 1, v)
#define RDD_DDR_QUEUE_DESCRIPTOR_PROFILE_ID_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 10, 4, 3, r)
#define RDD_DDR_QUEUE_DESCRIPTOR_PROFILE_ID_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 10, 4, 3, v)
#define RDD_DDR_QUEUE_DESCRIPTOR_TAIL_BASE_ENTRY_READ(r, p)                      FIELD_MREAD_16((uint8_t *)p + 10, 6, 6, r)
#define RDD_DDR_QUEUE_DESCRIPTOR_TAIL_BASE_ENTRY_WRITE(v, p)                     FIELD_MWRITE_16((uint8_t *)p + 10, 6, 6, v)
#define RDD_DDR_QUEUE_DESCRIPTOR_HEAD_BASE_ENTRY_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 11, 0, 6, r)
#define RDD_DDR_QUEUE_DESCRIPTOR_HEAD_BASE_ENTRY_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 11, 0, 6, v)
#define RDD_DDR_QUEUE_DESCRIPTOR_PROFILE_EN_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 12, 7, 1, r)
#define RDD_DDR_QUEUE_DESCRIPTOR_PROFILE_EN_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 12, 7, 1, v)
#define RDD_DDR_QUEUE_DESCRIPTOR_UNION_FIELD1_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 12, 0, 7, r)
#define RDD_DDR_QUEUE_DESCRIPTOR_UNION_FIELD1_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 12, 0, 7, v)
#define RDD_DDR_QUEUE_DESCRIPTOR_DROP_BIT_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 12, 0, 1, r)
#define RDD_DDR_QUEUE_DESCRIPTOR_DROP_BIT_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 12, 0, 1, v)
#define RDD_DDR_QUEUE_DESCRIPTOR_RATE_CONTROLLER_ID_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p + 12, 0, 7, r)
#define RDD_DDR_QUEUE_DESCRIPTOR_RATE_CONTROLLER_ID_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p + 12, 0, 7, v)
#define RDD_DDR_QUEUE_DESCRIPTOR_QUEUE_MASK_READ(r, p)                           MREAD_8((uint8_t *)p + 13, r)
#define RDD_DDR_QUEUE_DESCRIPTOR_QUEUE_MASK_WRITE(v, p)                          MWRITE_8((uint8_t *)p + 13, v)
#define RDD_DDR_QUEUE_DESCRIPTOR_CACHE_PTR_READ(r, p)                            MREAD_16((uint8_t *)p + 14, r)
#define RDD_DDR_QUEUE_DESCRIPTOR_CACHE_PTR_WRITE(v, p)                           MWRITE_16((uint8_t *)p + 14, v)
#if defined OREN

#define RDD_SERVICE_QUEUES_DESCRIPTOR_TABLE_SIZE     32
typedef struct
{
	RDD_DDR_QUEUE_DESCRIPTOR_DTS	entry[ RDD_SERVICE_QUEUES_DESCRIPTOR_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SERVICE_QUEUES_DESCRIPTOR_TABLE_DTS;

#define RDD_SERVICE_QUEUES_DESCRIPTOR_TABLE_PTR()	( RDD_SERVICE_QUEUES_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + SERVICE_QUEUES_DESCRIPTOR_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_IPTV_LOOKUP_TABLE_CAM_SIZE     32
typedef struct
{
	RDD_MAC_ENTRY_DTS	entry[ RDD_IPTV_LOOKUP_TABLE_CAM_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_LOOKUP_TABLE_CAM_DTS;

#define RDD_IPTV_LOOKUP_TABLE_CAM_PTR()	( RDD_IPTV_LOOKUP_TABLE_CAM_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + IPTV_LOOKUP_TABLE_CAM_ADDRESS )

#endif
#if defined OREN

#define RDD_MAC_TABLE_CAM_SIZE     32
typedef struct
{
	RDD_MAC_ENTRY_DTS	entry[ RDD_MAC_TABLE_CAM_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MAC_TABLE_CAM_DTS;

#define RDD_MAC_TABLE_CAM_PTR()	( RDD_MAC_TABLE_CAM_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_TABLE_CAM_ADDRESS )

#endif
#if defined OREN

#define RDD_IPTV_CONTEXT_TABLE_CAM_SIZE     32
typedef struct
{
	RDD_IPTV_CONTEXT_ENTRY_DTS	entry[ RDD_IPTV_CONTEXT_TABLE_CAM_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_CONTEXT_TABLE_CAM_DTS;

#define RDD_IPTV_CONTEXT_TABLE_CAM_PTR()	( RDD_IPTV_CONTEXT_TABLE_CAM_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + IPTV_CONTEXT_TABLE_CAM_ADDRESS )

#endif

typedef struct
{
	uint32_t	current_timeout            	:10	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	current_packet_count       	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	configured_timeout         	:10	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	configured_max_packet_count	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INTERRUPT_COALESCING_CONFIG_DTS;

#define RDD_INTERRUPT_COALESCING_CONFIG_CURRENT_TIMEOUT_READ(r, p)                             FIELD_MREAD_16((uint8_t *)p, 6, 10, r)
#define RDD_INTERRUPT_COALESCING_CONFIG_CURRENT_TIMEOUT_WRITE(v, p)                            FIELD_MWRITE_16((uint8_t *)p, 6, 10, v)
#define RDD_INTERRUPT_COALESCING_CONFIG_CURRENT_PACKET_COUNT_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 1, 0, 6, r)
#define RDD_INTERRUPT_COALESCING_CONFIG_CURRENT_PACKET_COUNT_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 1, 0, 6, v)
#define RDD_INTERRUPT_COALESCING_CONFIG_CONFIGURED_TIMEOUT_READ(r, p)                          FIELD_MREAD_16((uint8_t *)p + 2, 6, 10, r)
#define RDD_INTERRUPT_COALESCING_CONFIG_CONFIGURED_TIMEOUT_WRITE(v, p)                         FIELD_MWRITE_16((uint8_t *)p + 2, 6, 10, v)
#define RDD_INTERRUPT_COALESCING_CONFIG_CONFIGURED_MAX_PACKET_COUNT_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 3, 0, 6, r)
#define RDD_INTERRUPT_COALESCING_CONFIG_CONFIGURED_MAX_PACKET_COUNT_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 3, 0, 6, v)
#if defined OREN

#define RDD_INTERRUPT_COALESCING_CONFIG_TABLE_SIZE     12
typedef struct
{
	RDD_INTERRUPT_COALESCING_CONFIG_DTS	entry[ RDD_INTERRUPT_COALESCING_CONFIG_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INTERRUPT_COALESCING_CONFIG_TABLE_DTS;

#define RDD_INTERRUPT_COALESCING_CONFIG_TABLE_PTR()	( RDD_INTERRUPT_COALESCING_CONFIG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + INTERRUPT_COALESCING_CONFIG_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_WAN_VID_TABLE_SIZE     4
typedef struct
{
	RDD_VID_ENTRY_DTS	entry[ RDD_WAN_VID_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WAN_VID_TABLE_DTS;

#define RDD_WAN_VID_TABLE_PTR()	( RDD_WAN_VID_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + WAN_VID_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	timer_period	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	timer_armed 	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INTERRUPT_COALESCING_TIMER_CONFIG_DTS;

#define RDD_INTERRUPT_COALESCING_TIMER_CONFIG_TIMER_PERIOD_READ(r, p)                 MREAD_16((uint8_t *)p, r)
#define RDD_INTERRUPT_COALESCING_TIMER_CONFIG_TIMER_PERIOD_WRITE(v, p)                MWRITE_16((uint8_t *)p, v)
#define RDD_INTERRUPT_COALESCING_TIMER_CONFIG_TIMER_ARMED_READ(r, p)                  MREAD_16((uint8_t *)p + 2, r)
#define RDD_INTERRUPT_COALESCING_TIMER_CONFIG_TIMER_ARMED_WRITE(v, p)                 MWRITE_16((uint8_t *)p + 2, v)

typedef struct
{
	uint32_t	reserved_fw_only	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FREE_SKB_INDEXES_FIFO_TAIL_DTS;


typedef struct
{
	uint8_t	qos       	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PBITS_TO_QOS_ENTRY_DTS;

#define RDD_PBITS_TO_QOS_ENTRY_QOS_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_PBITS_TO_QOS_ENTRY_QOS_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#if defined OREN

#define RDD_DS_PBITS_TO_QOS_TABLE_SIZE     6
#define RDD_DS_PBITS_TO_QOS_TABLE_SIZE2    8
typedef struct
{
	RDD_PBITS_TO_QOS_ENTRY_DTS	entry[ RDD_DS_PBITS_TO_QOS_TABLE_SIZE ][ RDD_DS_PBITS_TO_QOS_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_PBITS_TO_QOS_TABLE_DTS;

#define RDD_DS_PBITS_TO_QOS_TABLE_PTR()	( RDD_DS_PBITS_TO_QOS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DS_PBITS_TO_QOS_TABLE_ADDRESS )

#endif

typedef struct
{
	uint8_t	cpu_rx_queue	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_REASON_TO_CPU_RX_QUEUE_ENTRY_DTS;

#define RDD_CPU_REASON_TO_CPU_RX_QUEUE_ENTRY_CPU_RX_QUEUE_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_CPU_REASON_TO_CPU_RX_QUEUE_ENTRY_CPU_RX_QUEUE_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)

#define RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_SIZE     64
typedef struct
{
	RDD_CPU_REASON_TO_CPU_RX_QUEUE_ENTRY_DTS	entry[ RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_DTS;

#define RDD_DS_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_PTR()	( RDD_DS_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DS_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_ADDRESS )


typedef struct
{
	uint32_t	bpm_counter                	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bpm_threshold              	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bpm_high_priority_threshold	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BPM_CONGESTION_CONTROL_ENTRY_DTS;

#define RDD_BPM_CONGESTION_CONTROL_ENTRY_BPM_COUNTER_READ(r, p)                                 MREAD_32((uint8_t *)p, r)
#define RDD_BPM_CONGESTION_CONTROL_ENTRY_BPM_COUNTER_WRITE(v, p)                                MWRITE_32((uint8_t *)p, v)
#define RDD_BPM_CONGESTION_CONTROL_ENTRY_BPM_THRESHOLD_READ(r, p)                               MREAD_32((uint8_t *)p + 4, r)
#define RDD_BPM_CONGESTION_CONTROL_ENTRY_BPM_THRESHOLD_WRITE(v, p)                              MWRITE_32((uint8_t *)p + 4, v)
#define RDD_BPM_CONGESTION_CONTROL_ENTRY_BPM_HIGH_PRIORITY_THRESHOLD_READ(r, p)                 MREAD_32((uint8_t *)p + 8, r)
#define RDD_BPM_CONGESTION_CONTROL_ENTRY_BPM_HIGH_PRIORITY_THRESHOLD_WRITE(v, p)                MWRITE_32((uint8_t *)p + 8, v)

typedef struct
{
	uint32_t	src_ip_12 	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	src_ip_13 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	src_ip_14 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	src_ip_15 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	src_ip_0  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	src_ip_1  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	src_ip_2  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	src_ip_3  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	excl      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	aging     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	skip      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	valid     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS;

#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_12_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 4, 4, r)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_12_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 4, 4, v)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_13_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p, 4, 8, r)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_13_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p, 4, 8, v)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_14_READ(r, p)                 FIELD_MREAD_32((uint8_t *)p + 0, 12, 8, r)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_14_WRITE(v, p)                FIELD_MWRITE_32((uint8_t *)p + 0, 12, 8, v)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_15_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p + 2, 4, 8, r)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_15_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p + 2, 4, 8, v)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_0_READ(r, p)                  { uint32_t temp; FIELD_MREAD_32((uint8_t *)p, 0, 4, temp); r = temp << 4; FIELD_MREAD_32(((uint8_t *)p + 4), 28, 4, temp); r = r | temp; }
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_0_WRITE(v, p)                 { FIELD_MWRITE_32((uint8_t *)p, 0, 4, ((v & 0xfffffff0) >> 4)); FIELD_MWRITE_32(((uint8_t *)p + 4), 28, 4, (v & 0x0000000f)); }
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_1_READ(r, p)                  FIELD_MREAD_16((uint8_t *)p + 4, 4, 8, r)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_1_WRITE(v, p)                 FIELD_MWRITE_16((uint8_t *)p + 4, 4, 8, v)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_2_READ(r, p)                  FIELD_MREAD_32((uint8_t *)p + 4, 12, 8, r)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_2_WRITE(v, p)                 FIELD_MWRITE_32((uint8_t *)p + 4, 12, 8, v)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_3_READ(r, p)                  FIELD_MREAD_16((uint8_t *)p + 6, 4, 8, r)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SRC_IP_3_WRITE(v, p)                 FIELD_MWRITE_16((uint8_t *)p + 6, 4, 8, v)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_EXCL_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 7, 3, 1, r)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_EXCL_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 7, 3, 1, v)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_AGING_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 7, 2, 1, r)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_AGING_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 7, 2, 1, v)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SKIP_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_SKIP_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 7, 1, 1, v)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_VALID_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 7, 0, 1, r)
#define RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_VALID_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 7, 0, 1, v)
#if defined OREN

#define RDD_IPTV_L3_SRC_IP_LOOKUP_TABLE_SIZE     32
typedef struct
{
	RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS	entry[ RDD_IPTV_L3_SRC_IP_LOOKUP_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_L3_SRC_IP_LOOKUP_TABLE_DTS;

#define RDD_IPTV_L3_SRC_IP_LOOKUP_TABLE_PTR()	( RDD_IPTV_L3_SRC_IP_LOOKUP_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + IPTV_L3_SRC_IP_LOOKUP_TABLE_ADDRESS )

#endif

typedef struct
{
	uint16_t	reserved0                   	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	isolation_mode_port_vector  	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	aggregated_vid_idx          	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	aggregation_mode_port_vector	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LAN_VID_CONTEXT_ENTRY_DTS;

#define RDD_LAN_VID_CONTEXT_ENTRY_ISOLATION_MODE_PORT_VECTOR_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p, 0, 6, r)
#define RDD_LAN_VID_CONTEXT_ENTRY_ISOLATION_MODE_PORT_VECTOR_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p, 0, 6, v)
#define RDD_LAN_VID_CONTEXT_ENTRY_AGGREGATED_VID_IDX_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 1, 6, 2, r)
#define RDD_LAN_VID_CONTEXT_ENTRY_AGGREGATED_VID_IDX_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 1, 6, 2, v)
#define RDD_LAN_VID_CONTEXT_ENTRY_AGGREGATION_MODE_PORT_VECTOR_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 1, 0, 6, r)
#define RDD_LAN_VID_CONTEXT_ENTRY_AGGREGATION_MODE_PORT_VECTOR_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 1, 0, 6, v)
#if defined OREN

#define RDD_LAN_VID_CONTEXT_TABLE_SIZE     128
typedef struct
{
	RDD_LAN_VID_CONTEXT_ENTRY_DTS	entry[ RDD_LAN_VID_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LAN_VID_CONTEXT_TABLE_DTS;

#define RDD_LAN_VID_CONTEXT_TABLE_PTR()	( RDD_LAN_VID_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + LAN_VID_CONTEXT_TABLE_ADDRESS )

#endif
#define RDD_RUNNER_SCRATCHPAD_RESERVED_FW_ONLY_NUMBER	64

typedef struct
{
	uint32_t	reserved_fw_only[RDD_RUNNER_SCRATCHPAD_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RUNNER_SCRATCHPAD_DTS;


typedef struct
{
	uint32_t	flow_id                 	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	source_bridge_port      	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	packet_length           	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	payload_offset_flag     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reason                  	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_ssid                	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2               	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	descriptor_type         	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ownership               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved3               	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	host_data_buffer_pointer	:29	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved4               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	is_ucast                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wl_tx_prio              	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved5               	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_sync_1588_entry_index	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wl_info                 	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = wl_info, size = 16 bits
	uint32_t	ssid_vector             	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wl_metadata             	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_DESCRIPTOR_DTS;

#define RDD_CPU_RX_DESCRIPTOR_FLOW_ID_READ(r, p)                                  FIELD_MREAD_16((uint8_t *)p, 4, 12, r)
#define RDD_CPU_RX_DESCRIPTOR_FLOW_ID_WRITE(v, p)                                 FIELD_MWRITE_16((uint8_t *)p, 4, 12, v)
#define RDD_CPU_RX_DESCRIPTOR_SOURCE_BRIDGE_PORT_READ(r, p)                       FIELD_MREAD_32((uint8_t *)p + 0, 14, 5, r)
#define RDD_CPU_RX_DESCRIPTOR_SOURCE_BRIDGE_PORT_WRITE(v, p)                      FIELD_MWRITE_32((uint8_t *)p + 0, 14, 5, v)
#define RDD_CPU_RX_DESCRIPTOR_PACKET_LENGTH_READ(r, p)                            FIELD_MREAD_16((uint8_t *)p + 2, 0, 14, r)
#define RDD_CPU_RX_DESCRIPTOR_PACKET_LENGTH_WRITE(v, p)                           FIELD_MWRITE_16((uint8_t *)p + 2, 0, 14, v)
#define RDD_CPU_RX_DESCRIPTOR_PAYLOAD_OFFSET_FLAG_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r)
#define RDD_CPU_RX_DESCRIPTOR_PAYLOAD_OFFSET_FLAG_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 4, 7, 1, v)
#define RDD_CPU_RX_DESCRIPTOR_REASON_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p + 4, 1, 6, r)
#define RDD_CPU_RX_DESCRIPTOR_REASON_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p + 4, 1, 6, v)
#define RDD_CPU_RX_DESCRIPTOR_DST_SSID_READ(r, p)                                 FIELD_MREAD_32((uint8_t *)p + 4, 9, 16, r)
#define RDD_CPU_RX_DESCRIPTOR_DST_SSID_WRITE(v, p)                                FIELD_MWRITE_32((uint8_t *)p + 4, 9, 16, v)
#define RDD_CPU_RX_DESCRIPTOR_DESCRIPTOR_TYPE_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 7, 0, 4, r)
#define RDD_CPU_RX_DESCRIPTOR_DESCRIPTOR_TYPE_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 7, 0, 4, v)
#define RDD_CPU_RX_DESCRIPTOR_OWNERSHIP_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 8, 7, 1, r)
#define RDD_CPU_RX_DESCRIPTOR_OWNERSHIP_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 8, 7, 1, v)
#define RDD_CPU_RX_DESCRIPTOR_HOST_DATA_BUFFER_POINTER_READ(r, p)                 FIELD_MREAD_32((uint8_t *)p + 8, 0, 29, r)
#define RDD_CPU_RX_DESCRIPTOR_HOST_DATA_BUFFER_POINTER_WRITE(v, p)                FIELD_MWRITE_32((uint8_t *)p + 8, 0, 29, v)
#define RDD_CPU_RX_DESCRIPTOR_IS_UCAST_READ(r, p)                                 FIELD_MREAD_8((uint8_t *)p + 12, 6, 1, r)
#define RDD_CPU_RX_DESCRIPTOR_IS_UCAST_WRITE(v, p)                                FIELD_MWRITE_8((uint8_t *)p + 12, 6, 1, v)
#define RDD_CPU_RX_DESCRIPTOR_WL_TX_PRIO_READ(r, p)                               FIELD_MREAD_8((uint8_t *)p + 12, 2, 4, r)
#define RDD_CPU_RX_DESCRIPTOR_WL_TX_PRIO_WRITE(v, p)                              FIELD_MWRITE_8((uint8_t *)p + 12, 2, 4, v)
#define RDD_CPU_RX_DESCRIPTOR_IP_SYNC_1588_ENTRY_INDEX_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 13, 0, 4, r)
#define RDD_CPU_RX_DESCRIPTOR_IP_SYNC_1588_ENTRY_INDEX_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 13, 0, 4, v)
#define RDD_CPU_RX_DESCRIPTOR_WL_INFO_READ(r, p)                                  MREAD_16((uint8_t *)p + 14, r)
#define RDD_CPU_RX_DESCRIPTOR_WL_INFO_WRITE(v, p)                                 MWRITE_16((uint8_t *)p + 14, v)
#define RDD_CPU_RX_DESCRIPTOR_SSID_VECTOR_READ(r, p)                              MREAD_16((uint8_t *)p + 14, r)
#define RDD_CPU_RX_DESCRIPTOR_SSID_VECTOR_WRITE(v, p)                             MWRITE_16((uint8_t *)p + 14, v)
#define RDD_CPU_RX_DESCRIPTOR_WL_METADATA_READ(r, p)                              MREAD_16((uint8_t *)p + 14, r)
#define RDD_CPU_RX_DESCRIPTOR_WL_METADATA_WRITE(v, p)                             MWRITE_16((uint8_t *)p + 14, v)
#if defined OREN

#define RDD_DS_RING_PACKET_DESCRIPTORS_CACHE_SIZE     12
typedef struct
{
	RDD_CPU_RX_DESCRIPTOR_DTS	entry[ RDD_DS_RING_PACKET_DESCRIPTORS_CACHE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_RING_PACKET_DESCRIPTORS_CACHE_DTS;

#define RDD_DS_RING_PACKET_DESCRIPTORS_CACHE_PTR()	( RDD_DS_RING_PACKET_DESCRIPTORS_CACHE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DS_RING_PACKET_DESCRIPTORS_CACHE_ADDRESS )

#endif
#if defined OREN

#define RDD_MAC_CONTEXT_TABLE_CAM_SIZE     32
typedef struct
{
	RDD_MAC_CONTEXT_ENTRY_DTS	entry[ RDD_MAC_CONTEXT_TABLE_CAM_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MAC_CONTEXT_TABLE_CAM_DTS;

#define RDD_MAC_CONTEXT_TABLE_CAM_PTR()	( RDD_MAC_CONTEXT_TABLE_CAM_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_CONTEXT_TABLE_CAM_ADDRESS )

#endif

typedef struct
{
	uint8_t	extension_entry	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MAC_EXTENSION_ENTRY_DTS;

#define RDD_MAC_EXTENSION_ENTRY_EXTENSION_ENTRY_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_MAC_EXTENSION_ENTRY_EXTENSION_ENTRY_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#if defined OREN

#define RDD_MAC_EXTENSION_TABLE_SIZE     1024
typedef struct
{
	RDD_MAC_EXTENSION_ENTRY_DTS	entry[ RDD_MAC_EXTENSION_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MAC_EXTENSION_TABLE_DTS;

#define RDD_MAC_EXTENSION_TABLE_PTR()	( RDD_MAC_EXTENSION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_EXTENSION_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	sustain_scheduling_mode  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	peak_scheduling_mode     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	overall_rate_limiter_mode	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved                 	:13	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	peak_ffi_offset          	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sustain_ffi_offset       	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	service_queues_status    	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_limiter_status      	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sustain_vector           	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	peak_vector              	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SERVICE_QUEUES_CFG_ENTRY_DTS;

#define RDD_SERVICE_QUEUES_CFG_ENTRY_SUSTAIN_SCHEDULING_MODE_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_SERVICE_QUEUES_CFG_ENTRY_SUSTAIN_SCHEDULING_MODE_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_SERVICE_QUEUES_CFG_ENTRY_PEAK_SCHEDULING_MODE_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p, 6, 1, r)
#define RDD_SERVICE_QUEUES_CFG_ENTRY_PEAK_SCHEDULING_MODE_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p, 6, 1, v)
#define RDD_SERVICE_QUEUES_CFG_ENTRY_OVERALL_RATE_LIMITER_MODE_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 5, 1, r)
#define RDD_SERVICE_QUEUES_CFG_ENTRY_OVERALL_RATE_LIMITER_MODE_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 5, 1, v)
#define RDD_SERVICE_QUEUES_CFG_ENTRY_PEAK_FFI_OFFSET_READ(r, p)                           MREAD_8((uint8_t *)p + 2, r)
#define RDD_SERVICE_QUEUES_CFG_ENTRY_PEAK_FFI_OFFSET_WRITE(v, p)                          MWRITE_8((uint8_t *)p + 2, v)
#define RDD_SERVICE_QUEUES_CFG_ENTRY_SUSTAIN_FFI_OFFSET_READ(r, p)                        MREAD_8((uint8_t *)p + 3, r)
#define RDD_SERVICE_QUEUES_CFG_ENTRY_SUSTAIN_FFI_OFFSET_WRITE(v, p)                       MWRITE_8((uint8_t *)p + 3, v)
#define RDD_SERVICE_QUEUES_CFG_ENTRY_SERVICE_QUEUES_STATUS_READ(r, p)                     MREAD_32((uint8_t *)p + 4, r)
#define RDD_SERVICE_QUEUES_CFG_ENTRY_SERVICE_QUEUES_STATUS_WRITE(v, p)                    MWRITE_32((uint8_t *)p + 4, v)
#define RDD_SERVICE_QUEUES_CFG_ENTRY_RATE_LIMITER_STATUS_READ(r, p)                       MREAD_32((uint8_t *)p + 8, r)
#define RDD_SERVICE_QUEUES_CFG_ENTRY_RATE_LIMITER_STATUS_WRITE(v, p)                      MWRITE_32((uint8_t *)p + 8, v)
#define RDD_SERVICE_QUEUES_CFG_ENTRY_SUSTAIN_VECTOR_READ(r, p)                            MREAD_32((uint8_t *)p + 12, r)
#define RDD_SERVICE_QUEUES_CFG_ENTRY_SUSTAIN_VECTOR_WRITE(v, p)                           MWRITE_32((uint8_t *)p + 12, v)
#define RDD_SERVICE_QUEUES_CFG_ENTRY_PEAK_VECTOR_READ(r, p)                               MREAD_32((uint8_t *)p + 16, r)
#define RDD_SERVICE_QUEUES_CFG_ENTRY_PEAK_VECTOR_WRITE(v, p)                              MWRITE_32((uint8_t *)p + 16, v)

typedef struct
{
	uint32_t	flood_bridge_port	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bridge_port      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ssid_vector      	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1        	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	policer_enable   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	policer_id       	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	filter_enable    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_prefix       	:24	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_DTS;

#define RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_FLOOD_BRIDGE_PORT_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_FLOOD_BRIDGE_PORT_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#define RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_BRIDGE_PORT_READ(r, p)                       MREAD_8((uint8_t *)p + 1, r)
#define RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_BRIDGE_PORT_WRITE(v, p)                      MWRITE_8((uint8_t *)p + 1, v)
#define RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_SSID_VECTOR_READ(r, p)                       MREAD_16((uint8_t *)p + 2, r)
#define RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_SSID_VECTOR_WRITE(v, p)                      MWRITE_16((uint8_t *)p + 2, v)
#define RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_POLICER_ENABLE_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 4, 5, 1, r)
#define RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_POLICER_ENABLE_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 4, 5, 1, v)
#define RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_POLICER_ID_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 4, 1, 4, r)
#define RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_POLICER_ID_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 4, 1, 4, v)
#define RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_FILTER_ENABLE_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 4, 0, 1, r)
#define RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_FILTER_ENABLE_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 4, 0, 1, v)
#define RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_MAC_PREFIX_READ(r, p)                        FIELD_MREAD_32((uint8_t *)p + 4, 0, 24, r)
#define RDD_MAC_UNKNOWN_DA_FORWARDING_ENTRY_MAC_PREFIX_WRITE(v, p)                       FIELD_MWRITE_32((uint8_t *)p + 4, 0, 24, v)
#define RDD_BPM_REPLY_RESERVED_FW_ONLY_NUMBER	12

typedef struct
{
	uint32_t	reserved_fw_only[RDD_BPM_REPLY_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BPM_REPLY_DTS;

#if defined OREN

#define RDD_GLOBAL_DSCP_TO_PBITS_TABLE_SIZE2    64
typedef struct
{
	RDD_DSCP_TO_PBITS_ENTRY_DTS	entry[ RDD_GLOBAL_DSCP_TO_PBITS_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GLOBAL_DSCP_TO_PBITS_TABLE_DTS;

#define RDD_GLOBAL_DSCP_TO_PBITS_TABLE_PTR()	( RDD_GLOBAL_DSCP_TO_PBITS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + GLOBAL_DSCP_TO_PBITS_TABLE_ADDRESS )

#endif

typedef struct
{
	uint8_t	reserved1 	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	pbits     	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	dei       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DSCP_TO_PBITS_DEI_ENTRY_DTS;

#define RDD_DSCP_TO_PBITS_DEI_ENTRY_PBITS_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p, 1, 3, r)
#define RDD_DSCP_TO_PBITS_DEI_ENTRY_PBITS_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p, 1, 3, v)
#define RDD_DSCP_TO_PBITS_DEI_ENTRY_DEI_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p, 0, 1, r)
#define RDD_DSCP_TO_PBITS_DEI_ENTRY_DEI_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p, 0, 1, v)
#if defined OREN

#define RDD_GLOBAL_DSCP_TO_PBITS_DEI_TABLE_SIZE2    64
typedef struct
{
	RDD_DSCP_TO_PBITS_DEI_ENTRY_DTS	entry[ RDD_GLOBAL_DSCP_TO_PBITS_DEI_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GLOBAL_DSCP_TO_PBITS_DEI_TABLE_DTS;

#define RDD_GLOBAL_DSCP_TO_PBITS_DEI_TABLE_PTR()	( RDD_GLOBAL_DSCP_TO_PBITS_DEI_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + GLOBAL_DSCP_TO_PBITS_DEI_TABLE_ADDRESS )

#endif
#define RDD_GPON_SKB_ENQUEUED_INDEXES_FIFO_ENTRY_RESERVED_FW_ONLY_NUMBER	24

typedef struct
{
	uint16_t	reserved_fw_only[RDD_GPON_SKB_ENQUEUED_INDEXES_FIFO_ENTRY_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GPON_SKB_ENQUEUED_INDEXES_FIFO_ENTRY_DTS;

#if defined OREN

#define RDD_GPON_SKB_ENQUEUED_INDEXES_FIFO_SIZE     32
typedef struct
{
	RDD_GPON_SKB_ENQUEUED_INDEXES_FIFO_ENTRY_DTS	entry[ RDD_GPON_SKB_ENQUEUED_INDEXES_FIFO_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GPON_SKB_ENQUEUED_INDEXES_FIFO_DTS;

#define RDD_GPON_SKB_ENQUEUED_INDEXES_FIFO_PTR()	( RDD_GPON_SKB_ENQUEUED_INDEXES_FIFO_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + GPON_SKB_ENQUEUED_INDEXES_FIFO_ADDRESS )

#endif

typedef struct
{
	uint32_t	size          	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	counter_number	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	addr          	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DDR_QUEUE_ADDRESS_ENTRY_DTS;

#define RDD_DDR_QUEUE_ADDRESS_ENTRY_SIZE_READ(r, p)                           MREAD_16((uint8_t *)p, r)
#define RDD_DDR_QUEUE_ADDRESS_ENTRY_SIZE_WRITE(v, p)                          MWRITE_16((uint8_t *)p, v)
#define RDD_DDR_QUEUE_ADDRESS_ENTRY_COUNTER_NUMBER_READ(r, p)                 MREAD_8((uint8_t *)p + 3, r)
#define RDD_DDR_QUEUE_ADDRESS_ENTRY_COUNTER_NUMBER_WRITE(v, p)                MWRITE_8((uint8_t *)p + 3, v)
#define RDD_DDR_QUEUE_ADDRESS_ENTRY_ADDR_READ(r, p)                           MREAD_32((uint8_t *)p + 4, r)
#define RDD_DDR_QUEUE_ADDRESS_ENTRY_ADDR_WRITE(v, p)                          MWRITE_32((uint8_t *)p + 4, v)
#if defined OREN

#define RDD_SERVICE_QUEUES_DDR_QUEUE_ADDRESS_TABLE_SIZE     32
typedef struct
{
	RDD_DDR_QUEUE_ADDRESS_ENTRY_DTS	entry[ RDD_SERVICE_QUEUES_DDR_QUEUE_ADDRESS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SERVICE_QUEUES_DDR_QUEUE_ADDRESS_TABLE_DTS;

#define RDD_SERVICE_QUEUES_DDR_QUEUE_ADDRESS_TABLE_PTR()	( RDD_SERVICE_QUEUES_DDR_QUEUE_ADDRESS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + SERVICE_QUEUES_DDR_QUEUE_ADDRESS_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_LOCAL_SWITCHING_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_SIZE     16
typedef struct
{
	RDD_BBH_RX_DESCRIPTOR_DTS	entry[ RDD_LOCAL_SWITCHING_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LOCAL_SWITCHING_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_DTS;

#define RDD_LOCAL_SWITCHING_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_PTR()	( RDD_LOCAL_SWITCHING_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + LOCAL_SWITCHING_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS )

#endif
#if defined OREN

#define RDD_MAC_EXTENSION_TABLE_CAM_SIZE     32
typedef struct
{
	RDD_MAC_EXTENSION_ENTRY_DTS	entry[ RDD_MAC_EXTENSION_TABLE_CAM_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MAC_EXTENSION_TABLE_CAM_DTS;

#define RDD_MAC_EXTENSION_TABLE_CAM_PTR()	( RDD_MAC_EXTENSION_TABLE_CAM_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_EXTENSION_TABLE_CAM_ADDRESS )

#endif
#define RDD_PM_COUNTERS_BUFFER_RESERVED_FW_ONLY_NUMBER	8

typedef struct
{
	uint32_t	reserved_fw_only[RDD_PM_COUNTERS_BUFFER_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PM_COUNTERS_BUFFER_DTS;

#if defined OREN

#define RDD_CPU_RX_RUNNER_A_SCRATCHPAD_SIZE     8
typedef struct
{
	RDD_RUNNER_SCRATCHPAD_DTS	entry[ RDD_CPU_RX_RUNNER_A_SCRATCHPAD_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_RUNNER_A_SCRATCHPAD_DTS;

#define RDD_CPU_RX_RUNNER_A_SCRATCHPAD_PTR()	( RDD_CPU_RX_RUNNER_A_SCRATCHPAD_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + CPU_RX_RUNNER_A_SCRATCHPAD_ADDRESS )

#endif

typedef struct
{
	uint32_t	key_index 	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	key_0     	:24	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	key_1     	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	aging     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	skip      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	valid     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	context   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	key_2     	:24	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	key_3     	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_DTS;

#define RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_INDEX_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 4, 4, r)
#define RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_INDEX_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 4, 4, v)
#define RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_0_READ(r, p)                     FIELD_MREAD_32((uint8_t *)p, 4, 24, r)
#define RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_0_WRITE(v, p)                    FIELD_MWRITE_32((uint8_t *)p, 4, 24, v)
#define RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_1_READ(r, p)                     { uint32_t temp; FIELD_MREAD_32((uint8_t *)p, 0, 4, temp); r = temp << 28; FIELD_MREAD_32(((uint8_t *)p + 4), 4, 28, temp); r = r | temp; }
#define RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_1_WRITE(v, p)                    { FIELD_MWRITE_32((uint8_t *)p, 0, 4, ((v & 0xf0000000) >> 28)); FIELD_MWRITE_32(((uint8_t *)p + 4), 4, 28, (v & 0x0fffffff)); }
#define RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_AGING_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 7, 2, 1, r)
#define RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_AGING_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 7, 2, 1, v)
#define RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_SKIP_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r)
#define RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_SKIP_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 7, 1, 1, v)
#define RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_VALID_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 7, 0, 1, r)
#define RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_VALID_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 7, 0, 1, v)
#define RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_CONTEXT_READ(r, p)                   MREAD_8((uint8_t *)p + 8, r)
#define RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_CONTEXT_WRITE(v, p)                  MWRITE_8((uint8_t *)p + 8, v)
#define RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_2_READ(r, p)                     FIELD_MREAD_32((uint8_t *)p + 8, 0, 24, r)
#define RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_2_WRITE(v, p)                    FIELD_MWRITE_32((uint8_t *)p + 8, 0, 24, v)
#define RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_3_READ(r, p)                     MREAD_32((uint8_t *)p + 12, r)
#define RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_KEY_3_WRITE(v, p)                    MWRITE_32((uint8_t *)p + 12, v)
#if defined OREN

#define RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_SIZE     128
typedef struct
{
	RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_DTS	entry[ RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_DTS;

#define RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_PTR()	( RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_ADDRESS )

#endif
#define RDD_PM_COUNTERS_RESERVED_FW_ONLY_NUMBER	1536

typedef struct
{
	uint32_t	reserved_fw_only[RDD_PM_COUNTERS_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PM_COUNTERS_DTS;

#if defined OREN

#define RDD_WAN_DDR_QUEUE_ADDRESS_TABLE_SIZE     128
typedef struct
{
	RDD_DDR_QUEUE_ADDRESS_ENTRY_DTS	entry[ RDD_WAN_DDR_QUEUE_ADDRESS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WAN_DDR_QUEUE_ADDRESS_TABLE_DTS;

#define RDD_WAN_DDR_QUEUE_ADDRESS_TABLE_PTR()	( RDD_WAN_DDR_QUEUE_ADDRESS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + WAN_DDR_QUEUE_ADDRESS_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	reserved0        	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	entries_counter  	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1        	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	number_of_entries	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ring_pointer     	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	interrupt_id     	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	drop_counter     	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2        	:27	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	size_of_entry    	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RING_DESCRIPTOR_DTS;

#define RDD_RING_DESCRIPTOR_ENTRIES_COUNTER_READ(r, p)                   FIELD_MREAD_16((uint8_t *)p, 0, 12, r)
#define RDD_RING_DESCRIPTOR_ENTRIES_COUNTER_WRITE(v, p)                  FIELD_MWRITE_16((uint8_t *)p, 0, 12, v)
#define RDD_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p + 2, 0, 12, r)
#define RDD_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p + 2, 0, 12, v)
#define RDD_RING_DESCRIPTOR_RING_POINTER_READ(r, p)                      MREAD_32((uint8_t *)p + 4, r)
#define RDD_RING_DESCRIPTOR_RING_POINTER_WRITE(v, p)                     MWRITE_32((uint8_t *)p + 4, v)
#define RDD_RING_DESCRIPTOR_INTERRUPT_ID_READ(r, p)                      MREAD_16((uint8_t *)p + 8, r)
#define RDD_RING_DESCRIPTOR_INTERRUPT_ID_WRITE(v, p)                     MWRITE_16((uint8_t *)p + 8, v)
#define RDD_RING_DESCRIPTOR_DROP_COUNTER_READ(r, p)                      MREAD_16((uint8_t *)p + 10, r)
#define RDD_RING_DESCRIPTOR_DROP_COUNTER_WRITE(v, p)                     MWRITE_16((uint8_t *)p + 10, v)
#define RDD_RING_DESCRIPTOR_SIZE_OF_ENTRY_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 15, 0, 5, r)
#define RDD_RING_DESCRIPTOR_SIZE_OF_ENTRY_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 15, 0, 5, v)
#if defined OREN

#define RDD_RING_DESCRIPTORS_TABLE_SIZE     12
typedef struct
{
	RDD_RING_DESCRIPTOR_DTS	entry[ RDD_RING_DESCRIPTORS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RING_DESCRIPTORS_TABLE_DTS;

#define RDD_RING_DESCRIPTORS_TABLE_PTR()	( RDD_RING_DESCRIPTORS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + RING_DESCRIPTORS_TABLE_ADDRESS )

#endif

typedef struct
{
	uint32_t	valid                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	context_index          	:15	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bucket_overflow_counter	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	key_extend             	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	protocol               	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	src_port               	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_port               	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	src_ip                 	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_ip                 	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CONNECTION_ENTRY_DTS;

#define RDD_CONNECTION_ENTRY_VALID_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_CONNECTION_ENTRY_VALID_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_CONNECTION_ENTRY_CONTEXT_INDEX_READ(r, p)                           FIELD_MREAD_16((uint8_t *)p, 0, 15, r)
#define RDD_CONNECTION_ENTRY_CONTEXT_INDEX_WRITE(v, p)                          FIELD_MWRITE_16((uint8_t *)p, 0, 15, v)
#define RDD_CONNECTION_ENTRY_BUCKET_OVERFLOW_COUNTER_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 2, 5, 3, r)
#define RDD_CONNECTION_ENTRY_BUCKET_OVERFLOW_COUNTER_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 2, 5, 3, v)
#define RDD_CONNECTION_ENTRY_KEY_EXTEND_READ(r, p)                              FIELD_MREAD_8((uint8_t *)p + 2, 0, 5, r)
#define RDD_CONNECTION_ENTRY_KEY_EXTEND_WRITE(v, p)                             FIELD_MWRITE_8((uint8_t *)p + 2, 0, 5, v)
#define RDD_CONNECTION_ENTRY_PROTOCOL_READ(r, p)                                MREAD_8((uint8_t *)p + 3, r)
#define RDD_CONNECTION_ENTRY_PROTOCOL_WRITE(v, p)                               MWRITE_8((uint8_t *)p + 3, v)
#define RDD_CONNECTION_ENTRY_SRC_PORT_READ(r, p)                                MREAD_16((uint8_t *)p + 4, r)
#define RDD_CONNECTION_ENTRY_SRC_PORT_WRITE(v, p)                               MWRITE_16((uint8_t *)p + 4, v)
#define RDD_CONNECTION_ENTRY_DST_PORT_READ(r, p)                                MREAD_16((uint8_t *)p + 6, r)
#define RDD_CONNECTION_ENTRY_DST_PORT_WRITE(v, p)                               MWRITE_16((uint8_t *)p + 6, v)
#define RDD_CONNECTION_ENTRY_SRC_IP_READ(r, p)                                  MREAD_32((uint8_t *)p + 8, r)
#define RDD_CONNECTION_ENTRY_SRC_IP_WRITE(v, p)                                 MWRITE_32((uint8_t *)p + 8, v)
#define RDD_CONNECTION_ENTRY_DST_IP_READ(r, p)                                  MREAD_32((uint8_t *)p + 12, r)
#define RDD_CONNECTION_ENTRY_DST_IP_WRITE(v, p)                                 MWRITE_32((uint8_t *)p + 12, v)
#if defined OREN

#define RDD_CONNECTION_BUFFER_TABLE_SIZE     5
#define RDD_CONNECTION_BUFFER_TABLE_SIZE2    4
typedef struct
{
	RDD_CONNECTION_ENTRY_DTS	entry[ RDD_CONNECTION_BUFFER_TABLE_SIZE ][ RDD_CONNECTION_BUFFER_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CONNECTION_BUFFER_TABLE_DTS;

#define RDD_CONNECTION_BUFFER_TABLE_PTR()	( RDD_CONNECTION_BUFFER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + CONNECTION_BUFFER_TABLE_ADDRESS )

#endif
/* COMMON_B */
#if defined OREN

#define RDD_US_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_SIZE     128
typedef struct
{
	RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_DTS	entry[ RDD_US_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_PTR()	( RDD_US_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + US_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_ADDRESS - 0x8000 )

#endif
#if defined OREN

#define RDD_CPU_RX_RUNNER_B_SCRATCHPAD_SIZE     8
typedef struct
{
	RDD_RUNNER_SCRATCHPAD_DTS	entry[ RDD_CPU_RX_RUNNER_B_SCRATCHPAD_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_RUNNER_B_SCRATCHPAD_DTS;

#define RDD_CPU_RX_RUNNER_B_SCRATCHPAD_PTR()	( RDD_CPU_RX_RUNNER_B_SCRATCHPAD_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + CPU_RX_RUNNER_B_SCRATCHPAD_ADDRESS - 0x8000 )

#endif
#define RDD_TUNNEL_ENTRY_TUNNEL_HEADER_NUMBER	80

typedef struct
{
	uint32_t	local_ip                                                	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tunnel_type                                             	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tunnel_header_length                                    	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_family                                               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0                                               	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	layer3_offset                                           	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	tunnel_header[RDD_TUNNEL_ENTRY_TUNNEL_HEADER_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_TUNNEL_ENTRY_DTS;

#define RDD_TUNNEL_ENTRY_LOCAL_IP_READ(r, p)                             MREAD_32((uint8_t *)p, r)
#define RDD_TUNNEL_ENTRY_LOCAL_IP_WRITE(v, p)                            MWRITE_32((uint8_t *)p, v)
#define RDD_TUNNEL_ENTRY_TUNNEL_TYPE_READ(r, p)                          MREAD_8((uint8_t *)p + 4, r)
#define RDD_TUNNEL_ENTRY_TUNNEL_TYPE_WRITE(v, p)                         MWRITE_8((uint8_t *)p + 4, v)
#define RDD_TUNNEL_ENTRY_TUNNEL_HEADER_LENGTH_READ(r, p)                 MREAD_8((uint8_t *)p + 5, r)
#define RDD_TUNNEL_ENTRY_TUNNEL_HEADER_LENGTH_WRITE(v, p)                MWRITE_8((uint8_t *)p + 5, v)
#define RDD_TUNNEL_ENTRY_IP_FAMILY_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 6, 7, 1, r)
#define RDD_TUNNEL_ENTRY_IP_FAMILY_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 6, 7, 1, v)
#define RDD_TUNNEL_ENTRY_LAYER3_OFFSET_READ(r, p)                        MREAD_8((uint8_t *)p + 7, r)
#define RDD_TUNNEL_ENTRY_LAYER3_OFFSET_WRITE(v, p)                       MWRITE_8((uint8_t *)p + 7, v)
#define RDD_TUNNEL_ENTRY_TUNNEL_HEADER_READ(r, p, i)                     MREAD_I_8((uint8_t *)p + 8, i, r)
#define RDD_TUNNEL_ENTRY_TUNNEL_HEADER_WRITE(v, p, i)                    MWRITE_I_8((uint8_t *)p + 8, i, v)
#if defined OREN

#define RDD_TUNNEL_TABLE_SIZE     4
typedef struct
{
	RDD_TUNNEL_ENTRY_DTS	entry[ RDD_TUNNEL_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_TUNNEL_TABLE_DTS;

#define RDD_TUNNEL_TABLE_PTR()	( RDD_TUNNEL_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + TUNNEL_TABLE_ADDRESS - 0x8000 )

#endif

typedef struct
{
	uint16_t	wifi_ssid_vector	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WIFI_SSID_FORWARDING_MATRIX_ENTRY_DTS;

#define RDD_WIFI_SSID_FORWARDING_MATRIX_ENTRY_WIFI_SSID_VECTOR_READ(r, p)                 MREAD_16((uint8_t *)p, r)
#define RDD_WIFI_SSID_FORWARDING_MATRIX_ENTRY_WIFI_SSID_VECTOR_WRITE(v, p)                MWRITE_16((uint8_t *)p, v)
#if defined OREN

#define RDD_WIFI_SSID_FORWARDING_MATRIX_TABLE_SIZE     16
typedef struct
{
	RDD_WIFI_SSID_FORWARDING_MATRIX_ENTRY_DTS	entry[ RDD_WIFI_SSID_FORWARDING_MATRIX_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WIFI_SSID_FORWARDING_MATRIX_TABLE_DTS;

#define RDD_WIFI_SSID_FORWARDING_MATRIX_TABLE_PTR()	( RDD_WIFI_SSID_FORWARDING_MATRIX_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + WIFI_SSID_FORWARDING_MATRIX_TABLE_ADDRESS - 0x8000 )

#endif
#if defined OREN

#define RDD_EPON_DDR_QUEUE_ADDRESS_TABLE_SIZE     16
typedef struct
{
	RDD_DDR_QUEUE_ADDRESS_ENTRY_DTS	entry[ RDD_EPON_DDR_QUEUE_ADDRESS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EPON_DDR_QUEUE_ADDRESS_TABLE_DTS;

#define RDD_EPON_DDR_QUEUE_ADDRESS_TABLE_PTR()	( RDD_EPON_DDR_QUEUE_ADDRESS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + EPON_DDR_QUEUE_ADDRESS_TABLE_ADDRESS - 0x8000 )

#endif
#define RDD_DUAL_STACK_LITE_ENTRY_SRC_IP_NUMBER	16
#define RDD_DUAL_STACK_LITE_ENTRY_DST_IP_NUMBER	16
#define RDD_DUAL_STACK_LITE_ENTRY_RESERVED_NUMBER	24

typedef struct
{
	uint32_t	vesion_class_flow_label                                	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	payload_length_next_header_hop_limit                   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	src_ip[RDD_DUAL_STACK_LITE_ENTRY_SRC_IP_NUMBER];
	uint8_t	dst_ip[RDD_DUAL_STACK_LITE_ENTRY_DST_IP_NUMBER];
	uint8_t	reserved[RDD_DUAL_STACK_LITE_ENTRY_RESERVED_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DUAL_STACK_LITE_ENTRY_DTS;

#define RDD_DUAL_STACK_LITE_ENTRY_VESION_CLASS_FLOW_LABEL_READ(r, p)                              MREAD_32((uint8_t *)p, r)
#define RDD_DUAL_STACK_LITE_ENTRY_VESION_CLASS_FLOW_LABEL_WRITE(v, p)                             MWRITE_32((uint8_t *)p, v)
#define RDD_DUAL_STACK_LITE_ENTRY_PAYLOAD_LENGTH_NEXT_HEADER_HOP_LIMIT_READ(r, p)                 MREAD_32((uint8_t *)p + 4, r)
#define RDD_DUAL_STACK_LITE_ENTRY_PAYLOAD_LENGTH_NEXT_HEADER_HOP_LIMIT_WRITE(v, p)                MWRITE_32((uint8_t *)p + 4, v)
#define RDD_DUAL_STACK_LITE_ENTRY_SRC_IP_READ(r, p, i)                                            MREAD_I_8((uint8_t *)p + 8, i, r)
#define RDD_DUAL_STACK_LITE_ENTRY_SRC_IP_WRITE(v, p, i)                                           MWRITE_I_8((uint8_t *)p + 8, i, v)
#define RDD_DUAL_STACK_LITE_ENTRY_DST_IP_READ(r, p, i)                                            MREAD_I_8((uint8_t *)p + 24, i, r)
#define RDD_DUAL_STACK_LITE_ENTRY_DST_IP_WRITE(v, p, i)                                           MWRITE_I_8((uint8_t *)p + 24, i, v)
#if defined OREN

#define RDD_DUAL_STACK_LITE_TABLE_SIZE     4
typedef struct
{
	RDD_DUAL_STACK_LITE_ENTRY_DTS	entry[ RDD_DUAL_STACK_LITE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DUAL_STACK_LITE_TABLE_DTS;

#define RDD_DUAL_STACK_LITE_TABLE_PTR()	( RDD_DUAL_STACK_LITE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + DUAL_STACK_LITE_TABLE_ADDRESS - 0x8000 )

#endif

typedef struct
{
	uint32_t	context        	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0      	:24	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1      	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pbits          	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	no_outer       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vlan           	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	gem_flow_extend	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	aging          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	skip           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	valid          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_DTS;

#define RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_CONTEXT_READ(r, p)                         MREAD_8((uint8_t *)p, r)
#define RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_CONTEXT_WRITE(v, p)                        MWRITE_8((uint8_t *)p, v)
#define RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_PBITS_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 4, 1, 3, r)
#define RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_PBITS_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 4, 1, 3, v)
#define RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_NO_OUTER_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 4, 0, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_NO_OUTER_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 4, 0, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_VLAN_READ(r, p)                            FIELD_MREAD_32((uint8_t *)p + 4, 12, 12, r)
#define RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_VLAN_WRITE(v, p)                           FIELD_MWRITE_32((uint8_t *)p + 4, 12, 12, v)
#define RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_GEM_FLOW_EXTEND_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p + 6, 4, 8, r)
#define RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_GEM_FLOW_EXTEND_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p + 6, 4, 8, v)
#define RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_AGING_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 7, 2, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_AGING_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 7, 2, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_SKIP_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_SKIP_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 7, 1, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_VALID_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 7, 0, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_VALID_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 7, 0, 1, v)

typedef struct
{
	uint32_t	context        	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0      	:24	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1      	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pbits          	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	no_outer       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vlan           	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2      	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	src_port_extend	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved3      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	aging          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	skip           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	valid          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_CONTEXT_READ(r, p)                         MREAD_8((uint8_t *)p, r)
#define RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_CONTEXT_WRITE(v, p)                        MWRITE_8((uint8_t *)p, v)
#define RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_PBITS_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 4, 1, 3, r)
#define RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_PBITS_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 4, 1, 3, v)
#define RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_NO_OUTER_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 4, 0, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_NO_OUTER_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 4, 0, 1, v)
#define RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_VLAN_READ(r, p)                            FIELD_MREAD_32((uint8_t *)p + 4, 12, 12, r)
#define RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_VLAN_WRITE(v, p)                           FIELD_MWRITE_32((uint8_t *)p + 4, 12, 12, v)
#define RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_SRC_PORT_EXTEND_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p + 6, 4, 5, r)
#define RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_SRC_PORT_EXTEND_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p + 6, 4, 5, v)
#define RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_AGING_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 7, 2, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_AGING_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 7, 2, 1, v)
#define RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_SKIP_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_SKIP_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 7, 1, 1, v)
#define RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_VALID_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 7, 0, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_VALID_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 7, 0, 1, v)

typedef struct
{
	uint32_t	context    	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	key_index  	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	gem_flow   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	inner_pbits	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	no_inner   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	inner_vlan 	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	outer_pbits	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	no_outer   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	outer_vlan 	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	aging      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	skip       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	valid      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_DTS;

#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_CONTEXT_READ(r, p)                     MREAD_8((uint8_t *)p, r)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_CONTEXT_WRITE(v, p)                    MWRITE_8((uint8_t *)p, v)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_KEY_INDEX_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p + 1, 4, 4, r)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_KEY_INDEX_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p + 1, 4, 4, v)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_GEM_FLOW_READ(r, p)                    FIELD_MREAD_16((uint8_t *)p + 2, 4, 8, r)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_GEM_FLOW_WRITE(v, p)                   FIELD_MWRITE_16((uint8_t *)p + 2, 4, 8, v)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_INNER_PBITS_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 3, 1, 3, r)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_INNER_PBITS_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 3, 1, 3, v)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_NO_INNER_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 3, 0, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_NO_INNER_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 3, 0, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_INNER_VLAN_READ(r, p)                  FIELD_MREAD_16((uint8_t *)p + 4, 4, 12, r)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_INNER_VLAN_WRITE(v, p)                 FIELD_MWRITE_16((uint8_t *)p + 4, 4, 12, v)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_OUTER_PBITS_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 5, 1, 3, r)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_OUTER_PBITS_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 5, 1, 3, v)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_NO_OUTER_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 5, 0, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_NO_OUTER_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 5, 0, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_OUTER_VLAN_READ(r, p)                  FIELD_MREAD_16((uint8_t *)p + 6, 4, 12, r)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_OUTER_VLAN_WRITE(v, p)                 FIELD_MWRITE_16((uint8_t *)p + 6, 4, 12, v)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_AGING_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 7, 2, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_AGING_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 7, 2, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_SKIP_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_SKIP_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 7, 1, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_VALID_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 7, 0, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_VALID_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 7, 0, 1, v)

typedef struct
{
	uint32_t	context        	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	key_index      	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0      	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ssid           	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	src_bridge_port	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	inner_pbits    	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	no_inner       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	inner_vlan     	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	outer_pbits    	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	no_outer       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	outer_vlan     	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	aging          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	skip           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	valid          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_CONTEXT_READ(r, p)                         MREAD_8((uint8_t *)p, r)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_CONTEXT_WRITE(v, p)                        MWRITE_8((uint8_t *)p, v)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_KEY_INDEX_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 1, 4, 4, r)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_KEY_INDEX_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 1, 4, 4, v)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_SSID_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 2, 4, 4, r)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_SSID_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 2, 4, 4, v)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_SRC_BRIDGE_PORT_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p + 2, 4, 8, r)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_SRC_BRIDGE_PORT_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p + 2, 4, 8, v)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_INNER_PBITS_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 3, 1, 3, r)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_INNER_PBITS_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 3, 1, 3, v)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_NO_INNER_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 3, 0, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_NO_INNER_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 3, 0, 1, v)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_INNER_VLAN_READ(r, p)                      FIELD_MREAD_16((uint8_t *)p + 4, 4, 12, r)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_INNER_VLAN_WRITE(v, p)                     FIELD_MWRITE_16((uint8_t *)p + 4, 4, 12, v)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_OUTER_PBITS_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 5, 1, 3, r)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_OUTER_PBITS_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 5, 1, 3, v)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_NO_OUTER_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 5, 0, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_NO_OUTER_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 5, 0, 1, v)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_OUTER_VLAN_READ(r, p)                      FIELD_MREAD_16((uint8_t *)p + 6, 4, 12, r)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_OUTER_VLAN_WRITE(v, p)                     FIELD_MWRITE_16((uint8_t *)p + 6, 4, 12, v)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_AGING_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 7, 2, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_AGING_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 7, 2, 1, v)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_SKIP_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_SKIP_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 7, 1, 1, v)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_VALID_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 7, 0, 1, r)
#define RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_VALID_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 7, 0, 1, v)

typedef struct
{
	uint32_t	context   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	key_index 	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	key_0     	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	key_1     	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	aging     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	skip      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	valid     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INGRESS_CLASSIFICATION_SHORT_LOOKUP_ENTRY_DTS;

#define RDD_INGRESS_CLASSIFICATION_SHORT_LOOKUP_ENTRY_CONTEXT_READ(r, p)                   MREAD_8((uint8_t *)p, r)
#define RDD_INGRESS_CLASSIFICATION_SHORT_LOOKUP_ENTRY_CONTEXT_WRITE(v, p)                  MWRITE_8((uint8_t *)p, v)
#define RDD_INGRESS_CLASSIFICATION_SHORT_LOOKUP_ENTRY_KEY_INDEX_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 1, 4, 4, r)
#define RDD_INGRESS_CLASSIFICATION_SHORT_LOOKUP_ENTRY_KEY_INDEX_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 1, 4, 4, v)
#define RDD_INGRESS_CLASSIFICATION_SHORT_LOOKUP_ENTRY_KEY_0_READ(r, p)                     FIELD_MREAD_32((uint8_t *)p + 0, 4, 16, r)
#define RDD_INGRESS_CLASSIFICATION_SHORT_LOOKUP_ENTRY_KEY_0_WRITE(v, p)                    FIELD_MWRITE_32((uint8_t *)p + 0, 4, 16, v)
#define RDD_INGRESS_CLASSIFICATION_SHORT_LOOKUP_ENTRY_KEY_1_READ(r, p)                     { uint32_t temp; FIELD_MREAD_32((uint8_t *)p, 0, 4, temp); r = temp << 28; FIELD_MREAD_32(((uint8_t *)p + 4), 4, 28, temp); r = r | temp; }
#define RDD_INGRESS_CLASSIFICATION_SHORT_LOOKUP_ENTRY_KEY_1_WRITE(v, p)                    { FIELD_MWRITE_32((uint8_t *)p, 0, 4, ((v & 0xf0000000) >> 28)); FIELD_MWRITE_32(((uint8_t *)p + 4), 4, 28, (v & 0x0fffffff)); }
#define RDD_INGRESS_CLASSIFICATION_SHORT_LOOKUP_ENTRY_AGING_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 7, 2, 1, r)
#define RDD_INGRESS_CLASSIFICATION_SHORT_LOOKUP_ENTRY_AGING_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 7, 2, 1, v)
#define RDD_INGRESS_CLASSIFICATION_SHORT_LOOKUP_ENTRY_SKIP_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r)
#define RDD_INGRESS_CLASSIFICATION_SHORT_LOOKUP_ENTRY_SKIP_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 7, 1, 1, v)
#define RDD_INGRESS_CLASSIFICATION_SHORT_LOOKUP_ENTRY_VALID_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 7, 0, 1, r)
#define RDD_INGRESS_CLASSIFICATION_SHORT_LOOKUP_ENTRY_VALID_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 7, 0, 1, v)
#if defined OREN

typedef struct
{
	uint32_t	pci0_vlan_command_id	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth0_vlan_command_id	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth1_vlan_command_id	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth2_vlan_command_id	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth3_vlan_command_id	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth4_vlan_command_id	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0           	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_VLAN_COMMAND_INDEX_ENTRY_DTS;

#define RDD_VLAN_COMMAND_INDEX_ENTRY_PCI0_VLAN_COMMAND_ID_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_PCI0_VLAN_COMMAND_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH0_VLAN_COMMAND_ID_READ(r, p)                 MREAD_8((uint8_t *)p + 1, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH0_VLAN_COMMAND_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p + 1, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH1_VLAN_COMMAND_ID_READ(r, p)                 MREAD_8((uint8_t *)p + 2, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH1_VLAN_COMMAND_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p + 2, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH2_VLAN_COMMAND_ID_READ(r, p)                 MREAD_8((uint8_t *)p + 3, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH2_VLAN_COMMAND_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p + 3, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH3_VLAN_COMMAND_ID_READ(r, p)                 MREAD_8((uint8_t *)p + 4, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH3_VLAN_COMMAND_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p + 4, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH4_VLAN_COMMAND_ID_READ(r, p)                 MREAD_8((uint8_t *)p + 5, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH4_VLAN_COMMAND_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p + 5, v)
#endif
#if defined OREN

#define RDD_VLAN_COMMAND_INDEX_TABLE_SIZE     256
typedef struct
{
	RDD_VLAN_COMMAND_INDEX_ENTRY_DTS	entry[ RDD_VLAN_COMMAND_INDEX_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_VLAN_COMMAND_INDEX_TABLE_DTS;

#define RDD_VLAN_COMMAND_INDEX_TABLE_PTR()	( RDD_VLAN_COMMAND_INDEX_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + VLAN_COMMAND_INDEX_TABLE_ADDRESS - 0x8000 )

#endif

typedef struct
{
	uint32_t	tod_high        	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tod_low         	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	local_time_delta	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0       	:20	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1       	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IP_SYNC_1588_DESCRIPTOR_ENTRY_DTS;

#define RDD_IP_SYNC_1588_DESCRIPTOR_ENTRY_TOD_HIGH_READ(r, p)                         MREAD_32((uint8_t *)p, r)
#define RDD_IP_SYNC_1588_DESCRIPTOR_ENTRY_TOD_HIGH_WRITE(v, p)                        MWRITE_32((uint8_t *)p, v)
#define RDD_IP_SYNC_1588_DESCRIPTOR_ENTRY_TOD_LOW_READ(r, p)                          MREAD_32((uint8_t *)p + 4, r)
#define RDD_IP_SYNC_1588_DESCRIPTOR_ENTRY_TOD_LOW_WRITE(v, p)                         MWRITE_32((uint8_t *)p + 4, v)
#define RDD_IP_SYNC_1588_DESCRIPTOR_ENTRY_LOCAL_TIME_DELTA_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p + 8, 4, 12, r)
#define RDD_IP_SYNC_1588_DESCRIPTOR_ENTRY_LOCAL_TIME_DELTA_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p + 8, 4, 12, v)
#if defined OREN

#define RDD_IP_SYNC_1588_DESCRIPTOR_QUEUE_SIZE     16
typedef struct
{
	RDD_IP_SYNC_1588_DESCRIPTOR_ENTRY_DTS	entry[ RDD_IP_SYNC_1588_DESCRIPTOR_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IP_SYNC_1588_DESCRIPTOR_QUEUE_DTS;

#define RDD_IP_SYNC_1588_DESCRIPTOR_QUEUE_PTR()	( RDD_IP_SYNC_1588_DESCRIPTOR_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + IP_SYNC_1588_DESCRIPTOR_QUEUE_ADDRESS - 0x8000 )

#endif
#if defined OREN

#define RDD_US_RING_PACKET_DESCRIPTORS_CACHE_SIZE     12
typedef struct
{
	RDD_CPU_RX_DESCRIPTOR_DTS	entry[ RDD_US_RING_PACKET_DESCRIPTORS_CACHE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_RING_PACKET_DESCRIPTORS_CACHE_DTS;

#define RDD_US_RING_PACKET_DESCRIPTORS_CACHE_PTR()	( RDD_US_RING_PACKET_DESCRIPTORS_CACHE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + US_RING_PACKET_DESCRIPTORS_CACHE_ADDRESS - 0x8000 )

#endif
#define RDD_DUMMY_RATE_CONTROLLER_DESCRIPTOR_RESERVED_FW_ONLY_NUMBER	16

typedef struct
{
	uint32_t	reserved_fw_only[RDD_DUMMY_RATE_CONTROLLER_DESCRIPTOR_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DUMMY_RATE_CONTROLLER_DESCRIPTOR_DTS;


typedef struct
{
	uint32_t	head_ptr           	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tail_ptr           	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	packet_counter     	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved           	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	counter_number     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	packet_threshold   	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	profile_ptr        	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_controller_ptr	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	queue_mask         	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS;

#define RDD_WAN_TX_QUEUE_DESCRIPTOR_HEAD_PTR_READ(r, p)                            MREAD_16((uint8_t *)p, r)
#define RDD_WAN_TX_QUEUE_DESCRIPTOR_HEAD_PTR_WRITE(v, p)                           MWRITE_16((uint8_t *)p, v)
#define RDD_WAN_TX_QUEUE_DESCRIPTOR_TAIL_PTR_READ(r, p)                            MREAD_16((uint8_t *)p + 2, r)
#define RDD_WAN_TX_QUEUE_DESCRIPTOR_TAIL_PTR_WRITE(v, p)                           MWRITE_16((uint8_t *)p + 2, v)
#define RDD_WAN_TX_QUEUE_DESCRIPTOR_PACKET_COUNTER_READ(r, p)                      MREAD_16((uint8_t *)p + 4, r)
#define RDD_WAN_TX_QUEUE_DESCRIPTOR_PACKET_COUNTER_WRITE(v, p)                     MWRITE_16((uint8_t *)p + 4, v)
#define RDD_WAN_TX_QUEUE_DESCRIPTOR_COUNTER_NUMBER_READ(r, p)                      MREAD_8((uint8_t *)p + 7, r)
#define RDD_WAN_TX_QUEUE_DESCRIPTOR_COUNTER_NUMBER_WRITE(v, p)                     MWRITE_8((uint8_t *)p + 7, v)
#define RDD_WAN_TX_QUEUE_DESCRIPTOR_PACKET_THRESHOLD_READ(r, p)                    MREAD_16((uint8_t *)p + 8, r)
#define RDD_WAN_TX_QUEUE_DESCRIPTOR_PACKET_THRESHOLD_WRITE(v, p)                   MWRITE_16((uint8_t *)p + 8, v)
#define RDD_WAN_TX_QUEUE_DESCRIPTOR_PROFILE_PTR_READ(r, p)                         MREAD_16((uint8_t *)p + 10, r)
#define RDD_WAN_TX_QUEUE_DESCRIPTOR_PROFILE_PTR_WRITE(v, p)                        MWRITE_16((uint8_t *)p + 10, v)
#define RDD_WAN_TX_QUEUE_DESCRIPTOR_RATE_CONTROLLER_PTR_READ(r, p)                 MREAD_16((uint8_t *)p + 12, r)
#define RDD_WAN_TX_QUEUE_DESCRIPTOR_RATE_CONTROLLER_PTR_WRITE(v, p)                MWRITE_16((uint8_t *)p + 12, v)
#define RDD_WAN_TX_QUEUE_DESCRIPTOR_QUEUE_MASK_READ(r, p)                          MREAD_16((uint8_t *)p + 14, r)
#define RDD_WAN_TX_QUEUE_DESCRIPTOR_QUEUE_MASK_WRITE(v, p)                         MWRITE_16((uint8_t *)p + 14, v)
#if defined OREN

#define RDD_WAN_TX_QUEUES_TABLE_SIZE     256
typedef struct
{
	RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS	entry[ RDD_WAN_TX_QUEUES_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WAN_TX_QUEUES_TABLE_DTS;

#define RDD_WAN_TX_QUEUES_TABLE_PTR()	( RDD_WAN_TX_QUEUES_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + WAN_TX_QUEUES_TABLE_ADDRESS - 0x8000 )

#endif
#if defined OREN

#define RDD_EPON_DDR_CACHE_FIFO_SIZE     192
typedef struct
{
	RDD_BBH_TX_DESCRIPTOR_DTS	entry[ RDD_EPON_DDR_CACHE_FIFO_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EPON_DDR_CACHE_FIFO_DTS;

#define RDD_EPON_DDR_CACHE_FIFO_PTR()	( RDD_EPON_DDR_CACHE_FIFO_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + EPON_DDR_CACHE_FIFO_ADDRESS - 0x8000 )

#endif
#if defined OREN

#define RDD_EPON_DDR_QUEUE_DESCRIPTORS_TABLE_SIZE     16
typedef struct
{
	RDD_DDR_QUEUE_DESCRIPTOR_DTS	entry[ RDD_EPON_DDR_QUEUE_DESCRIPTORS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EPON_DDR_QUEUE_DESCRIPTORS_TABLE_DTS;

#define RDD_EPON_DDR_QUEUE_DESCRIPTORS_TABLE_PTR()	( RDD_EPON_DDR_QUEUE_DESCRIPTORS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + EPON_DDR_QUEUE_DESCRIPTORS_TABLE_ADDRESS - 0x8000 )

#endif
#if defined OREN

#define RDD_LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_SIZE     16
typedef struct
{
	RDD_BBH_RX_DESCRIPTOR_DTS	entry[ RDD_LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_DTS;

#define RDD_LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_PTR()	( RDD_LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS - 0x8000 )

#endif
#if defined OREN

#define RDD_GPON_SKB_ENQUEUED_INDEXES_FREE_PTR_SIZE     40
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_GPON_SKB_ENQUEUED_INDEXES_FREE_PTR_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GPON_SKB_ENQUEUED_INDEXES_FREE_PTR_DTS;

#define RDD_GPON_SKB_ENQUEUED_INDEXES_FREE_PTR_PTR()	( RDD_GPON_SKB_ENQUEUED_INDEXES_FREE_PTR_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + GPON_SKB_ENQUEUED_INDEXES_FREE_PTR_ADDRESS - 0x8000 )

#endif
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_NUMBER	8

typedef struct
{
	uint32_t	current_peak_budget                                                     	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	allocated_peak_budget_exponent                                          	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	allocated_peak_budget                                                   	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	peak_budget_limit_exponent                                              	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	peak_budget_limit                                                       	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	current_sustain_budget                                                  	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	allocated_sustain_budget                                                	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_controller_mask                                                    	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wan_channel_ptr                                                         	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	priority_queues_status                                                  	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	peak_burst_counter                                                      	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	peak_weight                                                             	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0                                                               	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	peak_burst_flag                                                         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	tx_queue_addr[RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_NUMBER];
	uint32_t	reserved1                                                               	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS;

#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_CURRENT_PEAK_BUDGET_READ(r, p)                            MREAD_32((uint8_t *)p, r)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_CURRENT_PEAK_BUDGET_WRITE(v, p)                           MWRITE_32((uint8_t *)p, v)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_ALLOCATED_PEAK_BUDGET_EXPONENT_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 4, 6, 2, r)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_ALLOCATED_PEAK_BUDGET_EXPONENT_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 4, 6, 2, v)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_ALLOCATED_PEAK_BUDGET_READ(r, p)                          FIELD_MREAD_16((uint8_t *)p + 4, 0, 14, r)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_ALLOCATED_PEAK_BUDGET_WRITE(v, p)                         FIELD_MWRITE_16((uint8_t *)p + 4, 0, 14, v)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_PEAK_BUDGET_LIMIT_EXPONENT_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 6, 6, 2, r)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_PEAK_BUDGET_LIMIT_EXPONENT_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 6, 6, 2, v)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_PEAK_BUDGET_LIMIT_READ(r, p)                              FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_PEAK_BUDGET_LIMIT_WRITE(v, p)                             FIELD_MWRITE_16((uint8_t *)p + 6, 0, 14, v)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_CURRENT_SUSTAIN_BUDGET_READ(r, p)                         MREAD_32((uint8_t *)p + 8, r)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_CURRENT_SUSTAIN_BUDGET_WRITE(v, p)                        MWRITE_32((uint8_t *)p + 8, v)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_ALLOCATED_SUSTAIN_BUDGET_READ(r, p)                       MREAD_32((uint8_t *)p + 12, r)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_ALLOCATED_SUSTAIN_BUDGET_WRITE(v, p)                      MWRITE_32((uint8_t *)p + 12, v)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_RATE_CONTROLLER_MASK_READ(r, p)                           MREAD_32((uint8_t *)p + 16, r)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_RATE_CONTROLLER_MASK_WRITE(v, p)                          MWRITE_32((uint8_t *)p + 16, v)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_WAN_CHANNEL_PTR_READ(r, p)                                MREAD_16((uint8_t *)p + 20, r)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_WAN_CHANNEL_PTR_WRITE(v, p)                               MWRITE_16((uint8_t *)p + 20, v)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_PRIORITY_QUEUES_STATUS_READ(r, p)                         MREAD_16((uint8_t *)p + 22, r)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_PRIORITY_QUEUES_STATUS_WRITE(v, p)                        MWRITE_16((uint8_t *)p + 22, v)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_PEAK_BURST_COUNTER_READ(r, p)                             MREAD_16((uint8_t *)p + 24, r)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_PEAK_BURST_COUNTER_WRITE(v, p)                            MWRITE_16((uint8_t *)p + 24, v)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_PEAK_WEIGHT_READ(r, p)                                    MREAD_8((uint8_t *)p + 26, r)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_PEAK_WEIGHT_WRITE(v, p)                                   MWRITE_8((uint8_t *)p + 26, v)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_PEAK_BURST_FLAG_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 27, 0, 1, r)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_PEAK_BURST_FLAG_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 27, 0, 1, v)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_READ(r, p, i)                               MREAD_I_16((uint8_t *)p + 28, i, r)
#define RDD_US_RATE_CONTROLLER_DESCRIPTOR_TX_QUEUE_ADDR_WRITE(v, p, i)                              MWRITE_I_16((uint8_t *)p + 28, i, v)
#if defined OREN

#define RDD_US_RATE_CONTROLLERS_TABLE_SIZE     128
typedef struct
{
	RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS	entry[ RDD_US_RATE_CONTROLLERS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_RATE_CONTROLLERS_TABLE_DTS;

#define RDD_US_RATE_CONTROLLERS_TABLE_PTR()	( RDD_US_RATE_CONTROLLERS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + US_RATE_CONTROLLERS_TABLE_ADDRESS - 0x8000 )

#endif
#if defined OREN

#define RDD_WAN_DDR_CACHE_FIFO_SIZE     512
typedef struct
{
	RDD_BBH_TX_DESCRIPTOR_DTS	entry[ RDD_WAN_DDR_CACHE_FIFO_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WAN_DDR_CACHE_FIFO_DTS;

#define RDD_WAN_DDR_CACHE_FIFO_PTR()	( RDD_WAN_DDR_CACHE_FIFO_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + WAN_DDR_CACHE_FIFO_ADDRESS - 0x8000 )

#endif
#if defined OREN

#define RDD_WAN_EXT_DDR_CACHE_FIFO_SIZE     256
typedef struct
{
	RDD_BBH_TX_DESCRIPTOR_DTS	entry[ RDD_WAN_EXT_DDR_CACHE_FIFO_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WAN_EXT_DDR_CACHE_FIFO_DTS;

#define RDD_WAN_EXT_DDR_CACHE_FIFO_PTR()	( RDD_WAN_EXT_DDR_CACHE_FIFO_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + WAN_EXT_DDR_CACHE_FIFO_ADDRESS - 0x8000 )

#endif

typedef struct
{
	uint32_t	mac_prefix	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_DTS;

#define RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_MAC_PREFIX_READ(r, p)                 MREAD_32((uint8_t *)p, r)
#define RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_MAC_PREFIX_WRITE(v, p)                MWRITE_32((uint8_t *)p, v)
#if defined OREN

#define RDD_SRC_MAC_ANTI_SPOOFING_TABLE_SIZE     6
#define RDD_SRC_MAC_ANTI_SPOOFING_TABLE_SIZE2    4
typedef struct
{
	RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_DTS	entry[ RDD_SRC_MAC_ANTI_SPOOFING_TABLE_SIZE ][ RDD_SRC_MAC_ANTI_SPOOFING_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SRC_MAC_ANTI_SPOOFING_TABLE_DTS;

#define RDD_SRC_MAC_ANTI_SPOOFING_TABLE_PTR()	( RDD_SRC_MAC_ANTI_SPOOFING_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + SRC_MAC_ANTI_SPOOFING_TABLE_ADDRESS - 0x8000 )

#endif

typedef struct
{
	uint32_t	l3_total_len	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	l3_chsum    	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_TUNNEL_DYNAMIC_FIELDS_ENTRY_DTS;

#define RDD_TUNNEL_DYNAMIC_FIELDS_ENTRY_L3_TOTAL_LEN_READ(r, p)                 MREAD_16((uint8_t *)p, r)
#define RDD_TUNNEL_DYNAMIC_FIELDS_ENTRY_L3_TOTAL_LEN_WRITE(v, p)                MWRITE_16((uint8_t *)p, v)
#define RDD_TUNNEL_DYNAMIC_FIELDS_ENTRY_L3_CHSUM_READ(r, p)                     MREAD_16((uint8_t *)p + 2, r)
#define RDD_TUNNEL_DYNAMIC_FIELDS_ENTRY_L3_CHSUM_WRITE(v, p)                    MWRITE_16((uint8_t *)p + 2, v)
#if defined OREN

#define RDD_TUNNEL_DYNAMIC_FIELDS_TABLE_SIZE     4
typedef struct
{
	RDD_TUNNEL_DYNAMIC_FIELDS_ENTRY_DTS	entry[ RDD_TUNNEL_DYNAMIC_FIELDS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_TUNNEL_DYNAMIC_FIELDS_TABLE_DTS;

#define RDD_TUNNEL_DYNAMIC_FIELDS_TABLE_PTR()	( RDD_TUNNEL_DYNAMIC_FIELDS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + TUNNEL_DYNAMIC_FIELDS_TABLE_ADDRESS - 0x8000 )

#endif
#define RDD_DUMMY_WAN_TX_QUEUE_DESCRIPTOR_RESERVED_FW_ONLY_NUMBER	4

typedef struct
{
	uint32_t	reserved_fw_only[RDD_DUMMY_WAN_TX_QUEUE_DESCRIPTOR_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DUMMY_WAN_TX_QUEUE_DESCRIPTOR_DTS;

#if defined OREN

#define RDD_GPON_SKB_ENQUEUED_INDEXES_PUT_PTR_SIZE     40
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_GPON_SKB_ENQUEUED_INDEXES_PUT_PTR_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GPON_SKB_ENQUEUED_INDEXES_PUT_PTR_DTS;

#define RDD_GPON_SKB_ENQUEUED_INDEXES_PUT_PTR_PTR()	( RDD_GPON_SKB_ENQUEUED_INDEXES_PUT_PTR_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + GPON_SKB_ENQUEUED_INDEXES_PUT_PTR_ADDRESS - 0x8000 )

#endif
#if defined OREN

#define RDD_BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_SIZE     8
typedef struct
{
	RDD_BROADCOM_SWITCH_PORT_MAPPING_DTS	entry[ RDD_BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_DTS;

#define RDD_BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_PTR()	( RDD_BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_ADDRESS - 0x8000 )

#endif

typedef struct
{
	uint32_t	start_ts  	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	last_ts   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SPEED_SERVICE_RX_TIMESTAMPS_DTS;

#define RDD_SPEED_SERVICE_RX_TIMESTAMPS_START_TS_READ(r, p)                 MREAD_32((uint8_t *)p, r)
#define RDD_SPEED_SERVICE_RX_TIMESTAMPS_START_TS_WRITE(v, p)                MWRITE_32((uint8_t *)p, v)
#define RDD_SPEED_SERVICE_RX_TIMESTAMPS_LAST_TS_READ(r, p)                  MREAD_32((uint8_t *)p + 4, r)
#define RDD_SPEED_SERVICE_RX_TIMESTAMPS_LAST_TS_WRITE(v, p)                 MWRITE_32((uint8_t *)p + 4, v)
#if defined OREN

#define RDD_BRIDGE_PORT_TO_BBH_PERIPHERAL_RX_TABLE_SIZE     7
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_BRIDGE_PORT_TO_BBH_PERIPHERAL_RX_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BRIDGE_PORT_TO_BBH_PERIPHERAL_RX_TABLE_DTS;

#define RDD_BRIDGE_PORT_TO_BBH_PERIPHERAL_RX_TABLE_PTR()	( RDD_BRIDGE_PORT_TO_BBH_PERIPHERAL_RX_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + BRIDGE_PORT_TO_BBH_PERIPHERAL_RX_TABLE_ADDRESS - 0x8000 )

#endif
#if defined OREN

#define RDD_EGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_SIZE     5
typedef struct
{
	RDD_VLAN_SWITCHING_ISOLATION_CONFIG_ENTRY_DTS	entry[ RDD_EGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_DTS;

#define RDD_EGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_PTR()	( RDD_EGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + EGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_ADDRESS - 0x8000 )

#endif

typedef struct
{
	uint16_t	reserved_fw_only	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_PTR_DTS;

#define RDD_LAN_INGRESS_FIFO_ENTRY_RESERVED_FW_ONLY_NUMBER	32

typedef struct
{
	uint16_t	reserved_fw_only[RDD_LAN_INGRESS_FIFO_ENTRY_RESERVED_FW_ONLY_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LAN_INGRESS_FIFO_ENTRY_DTS;

#if defined OREN

#define RDD_LOCAL_SWITCHING_PBITS_TO_QOS_TABLE_SIZE     6
#define RDD_LOCAL_SWITCHING_PBITS_TO_QOS_TABLE_SIZE2    8
typedef struct
{
	RDD_PBITS_TO_QOS_ENTRY_DTS	entry[ RDD_LOCAL_SWITCHING_PBITS_TO_QOS_TABLE_SIZE ][ RDD_LOCAL_SWITCHING_PBITS_TO_QOS_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LOCAL_SWITCHING_PBITS_TO_QOS_TABLE_DTS;

#define RDD_LOCAL_SWITCHING_PBITS_TO_QOS_TABLE_PTR()	( RDD_LOCAL_SWITCHING_PBITS_TO_QOS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + LOCAL_SWITCHING_PBITS_TO_QOS_TABLE_ADDRESS - 0x8000 )

#endif
#if defined OREN

#define RDD_CPU_TX_DESCRIPTOR_QUEUE_TAIL_TABLE_SIZE     4
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_CPU_TX_DESCRIPTOR_QUEUE_TAIL_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_DESCRIPTOR_QUEUE_TAIL_TABLE_DTS;

#define RDD_CPU_TX_DESCRIPTOR_QUEUE_TAIL_TABLE_PTR()	( RDD_CPU_TX_DESCRIPTOR_QUEUE_TAIL_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + CPU_TX_DESCRIPTOR_QUEUE_TAIL_TABLE_ADDRESS - 0x8000 )

#endif

typedef struct
{
	uint16_t	reserved_fw_only	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WAN_PHYSICAL_PORT_DTS;


typedef struct
{
	uint8_t	exponent  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RATE_CONTROLLER_EXPONENT_ENTRY_DTS;

#define RDD_RATE_CONTROLLER_EXPONENT_ENTRY_EXPONENT_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_RATE_CONTROLLER_EXPONENT_ENTRY_EXPONENT_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)

#define RDD_RATE_CONTROLLER_EXPONENT_TABLE_SIZE     4
typedef struct
{
	RDD_RATE_CONTROLLER_EXPONENT_ENTRY_DTS	entry[ RDD_RATE_CONTROLLER_EXPONENT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RATE_CONTROLLER_EXPONENT_TABLE_DTS;

#define RDD_RATE_CONTROLLER_EXPONENT_TABLE_PTR()	( RDD_RATE_CONTROLLER_EXPONENT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + RATE_CONTROLLER_EXPONENT_TABLE_ADDRESS - 0x8000 )

#define RDD_US_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_PTR()	( RDD_US_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + US_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_ADDRESS - 0x8000 )


typedef struct
{
	uint8_t	active_ports_number	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MULTICAST_ACTIVE_PORTS_ENTRY_DTS;

#define RDD_MULTICAST_ACTIVE_PORTS_ENTRY_ACTIVE_PORTS_NUMBER_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_MULTICAST_ACTIVE_PORTS_ENTRY_ACTIVE_PORTS_NUMBER_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#if defined OREN

#define RDD_MULTICAST_ACTIVE_PORTS_TABLE_SIZE     64
typedef struct
{
	RDD_MULTICAST_ACTIVE_PORTS_ENTRY_DTS	entry[ RDD_MULTICAST_ACTIVE_PORTS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MULTICAST_ACTIVE_PORTS_TABLE_DTS;

#define RDD_MULTICAST_ACTIVE_PORTS_TABLE_PTR()	( RDD_MULTICAST_ACTIVE_PORTS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + MULTICAST_ACTIVE_PORTS_TABLE_ADDRESS - 0x8000 )

#endif
#if defined OREN

#define RDD_CPU_TX_EMAC_LOOPBACK_QUEUE_SIZE     4
typedef struct
{
	RDD_BBH_RX_DESCRIPTOR_DTS	entry[ RDD_CPU_TX_EMAC_LOOPBACK_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_EMAC_LOOPBACK_QUEUE_DTS;

#define RDD_CPU_TX_EMAC_LOOPBACK_QUEUE_PTR()	( RDD_CPU_TX_EMAC_LOOPBACK_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + CPU_TX_EMAC_LOOPBACK_QUEUE_ADDRESS - 0x8000 )

#endif
#if defined OREN

#define RDD_CPU_TX_US_FLOODING_QUEUE_SIZE     4
typedef struct
{
	RDD_BBH_RX_DESCRIPTOR_DTS	entry[ RDD_CPU_TX_US_FLOODING_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_US_FLOODING_QUEUE_DTS;

#define RDD_CPU_TX_US_FLOODING_QUEUE_PTR()	( RDD_CPU_TX_US_FLOODING_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + CPU_TX_US_FLOODING_QUEUE_ADDRESS - 0x8000 )

#endif
#if defined OREN

#define RDD_WAN_ENQUEUE_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_WAN_ENQUEUE_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WAN_ENQUEUE_INGRESS_QUEUE_DTS;

#define RDD_WAN_ENQUEUE_INGRESS_QUEUE_PTR()	( RDD_WAN_ENQUEUE_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + WAN_ENQUEUE_INGRESS_QUEUE_ADDRESS - 0x8000 )

#endif
/* DDR */

#define RDD_CONNECTION_TABLE_SIZE     32768
typedef struct
{
	RDD_CONNECTION_ENTRY_DTS	entry[ RDD_CONNECTION_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CONNECTION_TABLE_DTS;

#define RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_HEADER_NUMBER	32

typedef struct
{
	uint32_t	reserved                                                    	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	valid_packets_counter                                       	:24	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	valid_bytes_counter                                         	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	fwd_action                                                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	service_queue_mode_miss_union                               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = service_queue_mode_miss_union, size = 1 bits
	uint32_t	service_queues_mode                                         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	trap_type                                                   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	action_union                                                	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = action_union, size = 6 bits
	uint32_t	cpu_reason                                                  	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0                                                   	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1                                                   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	service_queue                                               	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	dscp_value                                                  	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ecn_value                                                   	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	nat_port                                                    	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_checksum_delta                                           	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	layer4_checksum_delta                                       	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	nat_ip                                                      	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_port                                                 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_version                                                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wifi_ssid                                                   	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	drop_eligibility                                            	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	is_wfd                                                      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_params_union                                         	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = egress_params_union, size = 16 bits
	uint32_t	traffic_class                                               	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_controller                                             	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wan_flow_index                                              	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wfd_prio                                                    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wfd_idx                                                     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wl_metadata                                                 	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	priority                                                    	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1                                                   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	radio_idx                                                   	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	flow_ring_id                                                	:10	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	outer_vid_offset                                            	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	policer_id                                                  	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	overflow                                                    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	outer_pbit_remap_action                                     	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	inner_pbit_remap_action                                     	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	qos_mapping_mode                                            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	layer2_header_number_of_tags                                	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	l2_offset                                                   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	l2_size                                                     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	actions_vector                                              	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	connection_direction                                        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	connection_table_index                                      	:15	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	l2_header[RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_HEADER_NUMBER];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS;

#define RDD_FLOW_CACHE_CONTEXT_ENTRY_VALID_PACKETS_COUNTER_READ(r, p)                        FIELD_MREAD_32((uint8_t *)p + 0, 0, 24, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_VALID_PACKETS_COUNTER_WRITE(v, p)                       FIELD_MWRITE_32((uint8_t *)p + 0, 0, 24, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_VALID_BYTES_COUNTER_READ(r, p)                          MREAD_32((uint8_t *)p + 4, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_VALID_BYTES_COUNTER_WRITE(v, p)                         MWRITE_32((uint8_t *)p + 4, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_FWD_ACTION_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p + 8, 7, 1, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_FWD_ACTION_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p + 8, 7, 1, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_SERVICE_QUEUE_MODE_MISS_UNION_READ(r, p)                FIELD_MREAD_8((uint8_t *)p + 8, 6, 1, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_SERVICE_QUEUE_MODE_MISS_UNION_WRITE(v, p)               FIELD_MWRITE_8((uint8_t *)p + 8, 6, 1, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_SERVICE_QUEUES_MODE_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 8, 6, 1, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_SERVICE_QUEUES_MODE_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 8, 6, 1, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_TRAP_TYPE_READ(r, p)                                    FIELD_MREAD_8((uint8_t *)p + 8, 6, 1, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_TRAP_TYPE_WRITE(v, p)                                   FIELD_MWRITE_8((uint8_t *)p + 8, 6, 1, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_ACTION_UNION_READ(r, p)                                 FIELD_MREAD_8((uint8_t *)p + 8, 0, 6, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_ACTION_UNION_WRITE(v, p)                                FIELD_MWRITE_8((uint8_t *)p + 8, 0, 6, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_CPU_REASON_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p + 8, 3, 3, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_CPU_REASON_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p + 8, 3, 3, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_SERVICE_QUEUE_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 8, 0, 5, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_SERVICE_QUEUE_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 8, 0, 5, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_DSCP_VALUE_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p + 9, 2, 6, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_DSCP_VALUE_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p + 9, 2, 6, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_ECN_VALUE_READ(r, p)                                    FIELD_MREAD_8((uint8_t *)p + 9, 0, 2, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_ECN_VALUE_WRITE(v, p)                                   FIELD_MWRITE_8((uint8_t *)p + 9, 0, 2, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_NAT_PORT_READ(r, p)                                     MREAD_16((uint8_t *)p + 10, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_NAT_PORT_WRITE(v, p)                                    MWRITE_16((uint8_t *)p + 10, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_IP_CHECKSUM_DELTA_READ(r, p)                            MREAD_16((uint8_t *)p + 12, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_IP_CHECKSUM_DELTA_WRITE(v, p)                           MWRITE_16((uint8_t *)p + 12, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_LAYER4_CHECKSUM_DELTA_READ(r, p)                        MREAD_16((uint8_t *)p + 14, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_LAYER4_CHECKSUM_DELTA_WRITE(v, p)                       MWRITE_16((uint8_t *)p + 14, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_NAT_IP_READ(r, p)                                       MREAD_32((uint8_t *)p + 16, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_NAT_IP_WRITE(v, p)                                      MWRITE_32((uint8_t *)p + 16, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_EGRESS_PORT_READ(r, p)                                  MREAD_8((uint8_t *)p + 20, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_EGRESS_PORT_WRITE(v, p)                                 MWRITE_8((uint8_t *)p + 20, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_IP_VERSION_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p + 21, 7, 1, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_IP_VERSION_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p + 21, 7, 1, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_WIFI_SSID_READ(r, p)                                    FIELD_MREAD_8((uint8_t *)p + 21, 3, 4, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_WIFI_SSID_WRITE(v, p)                                   FIELD_MWRITE_8((uint8_t *)p + 21, 3, 4, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_DROP_ELIGIBILITY_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 21, 1, 2, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_DROP_ELIGIBILITY_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 21, 1, 2, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_IS_WFD_READ(r, p)                                       FIELD_MREAD_8((uint8_t *)p + 21, 0, 1, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_IS_WFD_WRITE(v, p)                                      FIELD_MWRITE_8((uint8_t *)p + 21, 0, 1, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_EGRESS_PARAMS_UNION_READ(r, p)                          MREAD_16((uint8_t *)p + 22, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_EGRESS_PARAMS_UNION_WRITE(v, p)                         MWRITE_16((uint8_t *)p + 22, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_TRAFFIC_CLASS_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 22, 5, 3, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_TRAFFIC_CLASS_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 22, 5, 3, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_RATE_CONTROLLER_READ(r, p)                              FIELD_MREAD_8((uint8_t *)p + 22, 0, 5, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_RATE_CONTROLLER_WRITE(v, p)                             FIELD_MWRITE_8((uint8_t *)p + 22, 0, 5, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_WAN_FLOW_INDEX_READ(r, p)                               MREAD_8((uint8_t *)p + 23, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_WAN_FLOW_INDEX_WRITE(v, p)                              MWRITE_8((uint8_t *)p + 23, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_WFD_PRIO_READ(r, p)                                     FIELD_MREAD_8((uint8_t *)p + 22, 7, 1, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_WFD_PRIO_WRITE(v, p)                                    FIELD_MWRITE_8((uint8_t *)p + 22, 7, 1, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_WFD_IDX_READ(r, p)                                      FIELD_MREAD_8((uint8_t *)p + 22, 6, 1, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_WFD_IDX_WRITE(v, p)                                     FIELD_MWRITE_8((uint8_t *)p + 22, 6, 1, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_WL_METADATA_READ(r, p)                                  FIELD_MREAD_16((uint8_t *)p + 22, 0, 14, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_WL_METADATA_WRITE(v, p)                                 FIELD_MWRITE_16((uint8_t *)p + 22, 0, 14, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_PRIORITY_READ(r, p)                                     FIELD_MREAD_8((uint8_t *)p + 22, 5, 3, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_PRIORITY_WRITE(v, p)                                    FIELD_MWRITE_8((uint8_t *)p + 22, 5, 3, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_RADIO_IDX_READ(r, p)                                    FIELD_MREAD_8((uint8_t *)p + 22, 2, 2, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_RADIO_IDX_WRITE(v, p)                                   FIELD_MWRITE_8((uint8_t *)p + 22, 2, 2, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_FLOW_RING_ID_READ(r, p)                                 FIELD_MREAD_16((uint8_t *)p + 22, 0, 10, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_FLOW_RING_ID_WRITE(v, p)                                FIELD_MWRITE_16((uint8_t *)p + 22, 0, 10, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_OUTER_VID_OFFSET_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 24, 5, 3, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_OUTER_VID_OFFSET_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 24, 5, 3, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_POLICER_ID_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p + 24, 0, 5, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_POLICER_ID_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p + 24, 0, 5, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_OVERFLOW_READ(r, p)                                     FIELD_MREAD_8((uint8_t *)p + 25, 7, 1, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_OVERFLOW_WRITE(v, p)                                    FIELD_MWRITE_8((uint8_t *)p + 25, 7, 1, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_OUTER_PBIT_REMAP_ACTION_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 25, 5, 2, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_OUTER_PBIT_REMAP_ACTION_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 25, 5, 2, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_INNER_PBIT_REMAP_ACTION_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 25, 3, 2, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_INNER_PBIT_REMAP_ACTION_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 25, 3, 2, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_QOS_MAPPING_MODE_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 25, 2, 1, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_QOS_MAPPING_MODE_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 25, 2, 1, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_LAYER2_HEADER_NUMBER_OF_TAGS_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 25, 0, 2, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_LAYER2_HEADER_NUMBER_OF_TAGS_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 25, 0, 2, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_OFFSET_READ(r, p)                                    MREAD_8((uint8_t *)p + 26, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_OFFSET_WRITE(v, p)                                   MWRITE_8((uint8_t *)p + 26, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_SIZE_READ(r, p)                                      MREAD_8((uint8_t *)p + 27, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_SIZE_WRITE(v, p)                                     MWRITE_8((uint8_t *)p + 27, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_ACTIONS_VECTOR_READ(r, p)                               MREAD_16((uint8_t *)p + 28, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_ACTIONS_VECTOR_WRITE(v, p)                              MWRITE_16((uint8_t *)p + 28, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_CONNECTION_DIRECTION_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 30, 7, 1, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_CONNECTION_DIRECTION_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 30, 7, 1, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_CONNECTION_TABLE_INDEX_READ(r, p)                       FIELD_MREAD_16((uint8_t *)p + 30, 0, 15, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_CONNECTION_TABLE_INDEX_WRITE(v, p)                      FIELD_MWRITE_16((uint8_t *)p + 30, 0, 15, v)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_HEADER_READ(r, p, i)                                 MREAD_I_8((uint8_t *)p + 32, i, r)
#define RDD_FLOW_CACHE_CONTEXT_ENTRY_L2_HEADER_WRITE(v, p, i)                                MWRITE_I_8((uint8_t *)p + 32, i, v)
#if defined OREN

#define RDD_CONTEXT_TABLE_SIZE     16512
typedef struct
{
	RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS	entry[ RDD_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CONTEXT_TABLE_DTS;

#endif

typedef struct
{
	uint32_t	reserved0        	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	check_mask_src_ip	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	check_src_ip     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	check_dst_ip     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	next_rule        	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	src_ip_mask      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2        	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_ip           	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	src_ip           	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1        	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FIREWALL_RULE_ENTRY_DTS;

#define RDD_FIREWALL_RULE_ENTRY_CHECK_MASK_SRC_IP_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 2, 1, r)
#define RDD_FIREWALL_RULE_ENTRY_CHECK_MASK_SRC_IP_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 2, 1, v)
#define RDD_FIREWALL_RULE_ENTRY_CHECK_SRC_IP_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p, 1, 1, r)
#define RDD_FIREWALL_RULE_ENTRY_CHECK_SRC_IP_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p, 1, 1, v)
#define RDD_FIREWALL_RULE_ENTRY_CHECK_DST_IP_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p, 0, 1, r)
#define RDD_FIREWALL_RULE_ENTRY_CHECK_DST_IP_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p, 0, 1, v)
#define RDD_FIREWALL_RULE_ENTRY_NEXT_RULE_READ(r, p)                         MREAD_8((uint8_t *)p + 1, r)
#define RDD_FIREWALL_RULE_ENTRY_NEXT_RULE_WRITE(v, p)                        MWRITE_8((uint8_t *)p + 1, v)
#define RDD_FIREWALL_RULE_ENTRY_SRC_IP_MASK_READ(r, p)                       MREAD_8((uint8_t *)p + 2, r)
#define RDD_FIREWALL_RULE_ENTRY_SRC_IP_MASK_WRITE(v, p)                      MWRITE_8((uint8_t *)p + 2, v)
#define RDD_FIREWALL_RULE_ENTRY_DST_IP_READ(r, p)                            MREAD_32((uint8_t *)p + 4, r)
#define RDD_FIREWALL_RULE_ENTRY_DST_IP_WRITE(v, p)                           MWRITE_32((uint8_t *)p + 4, v)
#define RDD_FIREWALL_RULE_ENTRY_SRC_IP_READ(r, p)                            MREAD_32((uint8_t *)p + 8, r)
#define RDD_FIREWALL_RULE_ENTRY_SRC_IP_WRITE(v, p)                           MWRITE_32((uint8_t *)p + 8, v)
#if defined OREN

#define RDD_FIREWALL_RULES_TABLE_SIZE     256
typedef struct
{
	RDD_FIREWALL_RULE_ENTRY_DTS	entry[ RDD_FIREWALL_RULES_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FIREWALL_RULES_TABLE_DTS;

#endif
#if defined OREN

#define RDD_EPON_TX_POST_SCHEDULING_DDR_QUEUES_SIZE     65536
typedef struct
{
	RDD_BBH_TX_DESCRIPTOR_DTS	entry[ RDD_EPON_TX_POST_SCHEDULING_DDR_QUEUES_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EPON_TX_POST_SCHEDULING_DDR_QUEUES_DTS;

#endif
#if defined OREN

typedef struct
{
	uint32_t	wlan_mcast_index              	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	counter                       	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0                     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ingress_classification_context	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1                     	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	valid                         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cache_valid                   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cache_index                   	:9	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2                     	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	port_vector_info              	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = port_vector_info, size = 32 bits
	uint32_t	replication_number            	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved3                     	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_port_vector            	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_DDR_CONTEXT_ENTRY_DTS;

#define RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_READ(r, p)                               MREAD_16((uint8_t *)p, r)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_WRITE(v, p)                              MWRITE_16((uint8_t *)p, v)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_L_READ( wv )                             FIELD_GET( wv, 16, 16 )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_L_WRITE( v, wv )                         FIELD_SET( v, 16, 16, wv )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_READ(r, p)                                        MREAD_16((uint8_t *)p + 2, r)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_WRITE(v, p)                                       MWRITE_16((uint8_t *)p + 2, v)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_L_READ( wv )                                      FIELD_GET( wv, 0, 16 )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_L_WRITE( v, wv )                                  FIELD_SET( v, 0, 16, wv )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_READ(r, p)                 MREAD_8((uint8_t *)p + 5, r)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_WRITE(v, p)                MWRITE_8((uint8_t *)p + 5, v)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_L_READ( wv )               FIELD_GET( wv, 16, 8 )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_L_WRITE( v, wv )           FIELD_SET( v, 16, 8, wv )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_READ(r, p)                                          FIELD_MREAD_8((uint8_t *)p + 6, 2, 1, r)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_WRITE(v, p)                                         FIELD_MWRITE_8((uint8_t *)p + 6, 2, 1, v)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_L_READ( wv )                                        FIELD_GET( wv, 10, 1 )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_L_WRITE( v, wv )                                    FIELD_SET( v, 10, 1, wv )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_READ(r, p)                                    FIELD_MREAD_8((uint8_t *)p + 6, 1, 1, r)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_WRITE(v, p)                                   FIELD_MWRITE_8((uint8_t *)p + 6, 1, 1, v)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_L_READ( wv )                                  FIELD_GET( wv, 9, 1 )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_L_WRITE( v, wv )                              FIELD_SET( v, 9, 1, wv )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_READ(r, p)                                    FIELD_MREAD_16((uint8_t *)p + 6, 0, 9, r)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_WRITE(v, p)                                   FIELD_MWRITE_16((uint8_t *)p + 6, 0, 9, v)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_L_READ( wv )                                  FIELD_GET( wv, 0, 9 )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_L_WRITE( v, wv )                              FIELD_SET( v, 0, 9, wv )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_PORT_VECTOR_INFO_READ(r, p)                               MREAD_32((uint8_t *)p + 12, r)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_PORT_VECTOR_INFO_WRITE(v, p)                              MWRITE_32((uint8_t *)p + 12, v)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_PORT_VECTOR_INFO_L_READ( wv )                             FIELD_GET( wv, 0, 32 )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_PORT_VECTOR_INFO_L_WRITE( v, wv )                         FIELD_SET( v, 0, 32, wv )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_READ(r, p)                             MREAD_8((uint8_t *)p + 12, r)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_WRITE(v, p)                            MWRITE_8((uint8_t *)p + 12, v)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_L_READ( wv )                           FIELD_GET( wv, 24, 8 )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_L_WRITE( v, wv )                       FIELD_SET( v, 24, 8, wv )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_READ(r, p)                             MREAD_8((uint8_t *)p + 15, r)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_WRITE(v, p)                            MWRITE_8((uint8_t *)p + 15, v)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_L_READ( wv )                           FIELD_GET( wv, 0, 8 )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_L_WRITE( v, wv )                       FIELD_SET( v, 0, 8, wv )
#endif
#if defined OREN

#define RDD_IPTV_DDR_CONTEXT_TABLE_SIZE     8192
typedef struct
{
	RDD_IPTV_DDR_CONTEXT_ENTRY_DTS	entry[ RDD_IPTV_DDR_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_DDR_CONTEXT_TABLE_DTS;

#endif

typedef struct
{
	uint32_t	valid     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_0	:31	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_1	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_2	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vid       	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_addr0 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_addr1 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_addr2 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_addr3 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_addr4 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_addr5 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_L2_DDR_LOOKUP_ENTRY_DTS;

#define RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VALID_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VALID_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VID_READ(r, p)                        FIELD_MREAD_16((uint8_t *)p + 8, 0, 12, r)
#define RDD_IPTV_L2_DDR_LOOKUP_ENTRY_VID_WRITE(v, p)                       FIELD_MWRITE_16((uint8_t *)p + 8, 0, 12, v)
#define RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR0_READ(r, p)                  MREAD_8((uint8_t *)p + 10, r)
#define RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR0_WRITE(v, p)                 MWRITE_8((uint8_t *)p + 10, v)
#define RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR1_READ(r, p)                  MREAD_8((uint8_t *)p + 11, r)
#define RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR1_WRITE(v, p)                 MWRITE_8((uint8_t *)p + 11, v)
#define RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR2_READ(r, p)                  MREAD_8((uint8_t *)p + 12, r)
#define RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR2_WRITE(v, p)                 MWRITE_8((uint8_t *)p + 12, v)
#define RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR3_READ(r, p)                  MREAD_8((uint8_t *)p + 13, r)
#define RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR3_WRITE(v, p)                 MWRITE_8((uint8_t *)p + 13, v)
#define RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR4_READ(r, p)                  MREAD_8((uint8_t *)p + 14, r)
#define RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR4_WRITE(v, p)                 MWRITE_8((uint8_t *)p + 14, v)
#define RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR5_READ(r, p)                  MREAD_8((uint8_t *)p + 15, r)
#define RDD_IPTV_L2_DDR_LOOKUP_ENTRY_MAC_ADDR5_WRITE(v, p)                 MWRITE_8((uint8_t *)p + 15, v)

typedef struct
{
	uint32_t	valid     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_0	:31	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_ip12  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_ip13  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_ip14  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_ip15  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_1	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_ip0   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_ip1   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_ip2   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_ip3   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DTS;

#define RDD_IPTV_L3_DDR_LOOKUP_ENTRY_VALID_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_IPTV_L3_DDR_LOOKUP_ENTRY_VALID_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_READ(r, p)                   MREAD_8((uint8_t *)p + 4, r)
#define RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP12_WRITE(v, p)                  MWRITE_8((uint8_t *)p + 4, v)
#define RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_READ(r, p)                   MREAD_8((uint8_t *)p + 5, r)
#define RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP13_WRITE(v, p)                  MWRITE_8((uint8_t *)p + 5, v)
#define RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_READ(r, p)                   MREAD_8((uint8_t *)p + 6, r)
#define RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP14_WRITE(v, p)                  MWRITE_8((uint8_t *)p + 6, v)
#define RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_READ(r, p)                   MREAD_8((uint8_t *)p + 7, r)
#define RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP15_WRITE(v, p)                  MWRITE_8((uint8_t *)p + 7, v)
#define RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_READ(r, p)                    MREAD_8((uint8_t *)p + 12, r)
#define RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP0_WRITE(v, p)                   MWRITE_8((uint8_t *)p + 12, v)
#define RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_READ(r, p)                    MREAD_8((uint8_t *)p + 13, r)
#define RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP1_WRITE(v, p)                   MWRITE_8((uint8_t *)p + 13, v)
#define RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_READ(r, p)                    MREAD_8((uint8_t *)p + 14, r)
#define RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP2_WRITE(v, p)                   MWRITE_8((uint8_t *)p + 14, v)
#define RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_READ(r, p)                    MREAD_8((uint8_t *)p + 15, r)
#define RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DST_IP3_WRITE(v, p)                   MWRITE_8((uint8_t *)p + 15, v)

typedef struct
{
	uint32_t	valid        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_0   	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	context_table	:10	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	context_valid	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	any          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_1   	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_ip12     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_ip13     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_ip14     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_ip15     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_2   	:20	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vid          	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_ip0      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_ip1      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_ip2      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dst_ip3      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DTS;

#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_VALID_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_VALID_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_TABLE_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p, 2, 10, r)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_TABLE_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p, 2, 10, v)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_VALID_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 1, 1, 1, r)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_CONTEXT_VALID_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 1, 1, 1, v)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_ANY_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 1, 0, 1, r)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_ANY_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 1, 0, 1, v)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP12_READ(r, p)                      MREAD_8((uint8_t *)p + 4, r)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP12_WRITE(v, p)                     MWRITE_8((uint8_t *)p + 4, v)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP13_READ(r, p)                      MREAD_8((uint8_t *)p + 5, r)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP13_WRITE(v, p)                     MWRITE_8((uint8_t *)p + 5, v)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP14_READ(r, p)                      MREAD_8((uint8_t *)p + 6, r)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP14_WRITE(v, p)                     MWRITE_8((uint8_t *)p + 6, v)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP15_READ(r, p)                      MREAD_8((uint8_t *)p + 7, r)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP15_WRITE(v, p)                     MWRITE_8((uint8_t *)p + 7, v)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_VID_READ(r, p)                           FIELD_MREAD_16((uint8_t *)p + 10, 0, 12, r)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_VID_WRITE(v, p)                          FIELD_MWRITE_16((uint8_t *)p + 10, 0, 12, v)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP0_READ(r, p)                       MREAD_8((uint8_t *)p + 12, r)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP0_WRITE(v, p)                      MWRITE_8((uint8_t *)p + 12, v)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP1_READ(r, p)                       MREAD_8((uint8_t *)p + 13, r)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP1_WRITE(v, p)                      MWRITE_8((uint8_t *)p + 13, v)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP2_READ(r, p)                       MREAD_8((uint8_t *)p + 14, r)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP2_WRITE(v, p)                      MWRITE_8((uint8_t *)p + 14, v)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP3_READ(r, p)                       MREAD_8((uint8_t *)p + 15, r)
#define RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DST_IP3_WRITE(v, p)                      MWRITE_8((uint8_t *)p + 15, v)
#if defined OREN

#define RDD_IPTV_SSM_DDR_CONTEXT_TABLE_SIZE     32768
typedef struct
{
	RDD_IPTV_DDR_CONTEXT_ENTRY_DTS	entry[ RDD_IPTV_SSM_DDR_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS;

#endif

typedef struct
{
	uint8_t	rule_index	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FIREWALL_RULES_MAP_ENTRY_DTS;

#define RDD_FIREWALL_RULES_MAP_ENTRY_RULE_INDEX_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_FIREWALL_RULES_MAP_ENTRY_RULE_INDEX_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#if defined OREN

#define RDD_FIREWALL_RULES_MAP_TABLE_SIZE     8
#define RDD_FIREWALL_RULES_MAP_TABLE_SIZE2    2
#define RDD_FIREWALL_RULES_MAP_TABLE_SIZE3    65536
typedef struct
{
	RDD_FIREWALL_RULES_MAP_ENTRY_DTS	entry[ RDD_FIREWALL_RULES_MAP_TABLE_SIZE ][ RDD_FIREWALL_RULES_MAP_TABLE_SIZE2 ][ RDD_FIREWALL_RULES_MAP_TABLE_SIZE3 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FIREWALL_RULES_MAP_TABLE_DTS;

#endif
/* PSRAM */
#if defined OREN

#define RDD_DS_INGRESS_CLASSIFICATION_COUNTERS_TABLE_SIZE     256
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_DS_INGRESS_CLASSIFICATION_COUNTERS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_CLASSIFICATION_COUNTERS_TABLE_DTS;

#define RDD_DS_INGRESS_CLASSIFICATION_COUNTERS_TABLE_PTR()	( RDD_DS_INGRESS_CLASSIFICATION_COUNTERS_TABLE_DTS * )(DEVICE_ADDRESS( PSRAM_BLOCK_OFFSET ) + DS_INGRESS_CLASSIFICATION_COUNTERS_TABLE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_INGRESS_CLASSIFICATION_COUNTERS_TABLE_SIZE     256
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_US_INGRESS_CLASSIFICATION_COUNTERS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_COUNTERS_TABLE_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_COUNTERS_TABLE_PTR()	( RDD_US_INGRESS_CLASSIFICATION_COUNTERS_TABLE_DTS * )(DEVICE_ADDRESS( PSRAM_BLOCK_OFFSET ) + US_INGRESS_CLASSIFICATION_COUNTERS_TABLE_ADDRESS )

#endif
/* PRIVATE_A */
#if defined G9991

#define RDD_INGRESS_HANDLER_BUFFER_SIZE     32
typedef struct
{
	RDD_IH_BUFFER_DTS	entry[ RDD_INGRESS_HANDLER_BUFFER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INGRESS_HANDLER_BUFFER_DTS;

#define RDD_INGRESS_HANDLER_BUFFER_PTR()	( RDD_INGRESS_HANDLER_BUFFER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + INGRESS_HANDLER_BUFFER_ADDRESS )

#endif
#if defined G9991

#define RDD_G9991_DDR_CACHE_FIFO_SIZE     1440
typedef struct
{
	RDD_BBH_TX_DESCRIPTOR_DTS	entry[ RDD_G9991_DDR_CACHE_FIFO_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_G9991_DDR_CACHE_FIFO_DTS;

#define RDD_G9991_DDR_CACHE_FIFO_PTR()	( RDD_G9991_DDR_CACHE_FIFO_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + G9991_DDR_CACHE_FIFO_ADDRESS )

#endif
#if defined G9991

typedef struct
{
	uint32_t	eth0_vlan_command_id 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth1_vlan_command_id 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth2_vlan_command_id 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth3_vlan_command_id 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth4_vlan_command_id 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth5_vlan_command_id 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth6_vlan_command_id 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth7_vlan_command_id 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth8_vlan_command_id 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth9_vlan_command_id 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth10_vlan_command_id	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth11_vlan_command_id	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth12_vlan_command_id	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth13_vlan_command_id	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth14_vlan_command_id	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth15_vlan_command_id	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth16_vlan_command_id	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth17_vlan_command_id	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth18_vlan_command_id	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth19_vlan_command_id	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth20_vlan_command_id	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth21_vlan_command_id	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth22_vlan_command_id	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth23_vlan_command_id	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_VLAN_COMMAND_INDEX_ENTRY_DTS;

#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH0_VLAN_COMMAND_ID_READ(r, p)                  MREAD_8((uint8_t *)p, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH0_VLAN_COMMAND_ID_WRITE(v, p)                 MWRITE_8((uint8_t *)p, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH1_VLAN_COMMAND_ID_READ(r, p)                  MREAD_8((uint8_t *)p + 1, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH1_VLAN_COMMAND_ID_WRITE(v, p)                 MWRITE_8((uint8_t *)p + 1, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH2_VLAN_COMMAND_ID_READ(r, p)                  MREAD_8((uint8_t *)p + 2, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH2_VLAN_COMMAND_ID_WRITE(v, p)                 MWRITE_8((uint8_t *)p + 2, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH3_VLAN_COMMAND_ID_READ(r, p)                  MREAD_8((uint8_t *)p + 3, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH3_VLAN_COMMAND_ID_WRITE(v, p)                 MWRITE_8((uint8_t *)p + 3, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH4_VLAN_COMMAND_ID_READ(r, p)                  MREAD_8((uint8_t *)p + 4, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH4_VLAN_COMMAND_ID_WRITE(v, p)                 MWRITE_8((uint8_t *)p + 4, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH5_VLAN_COMMAND_ID_READ(r, p)                  MREAD_8((uint8_t *)p + 5, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH5_VLAN_COMMAND_ID_WRITE(v, p)                 MWRITE_8((uint8_t *)p + 5, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH6_VLAN_COMMAND_ID_READ(r, p)                  MREAD_8((uint8_t *)p + 6, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH6_VLAN_COMMAND_ID_WRITE(v, p)                 MWRITE_8((uint8_t *)p + 6, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH7_VLAN_COMMAND_ID_READ(r, p)                  MREAD_8((uint8_t *)p + 7, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH7_VLAN_COMMAND_ID_WRITE(v, p)                 MWRITE_8((uint8_t *)p + 7, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH8_VLAN_COMMAND_ID_READ(r, p)                  MREAD_8((uint8_t *)p + 8, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH8_VLAN_COMMAND_ID_WRITE(v, p)                 MWRITE_8((uint8_t *)p + 8, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH9_VLAN_COMMAND_ID_READ(r, p)                  MREAD_8((uint8_t *)p + 9, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH9_VLAN_COMMAND_ID_WRITE(v, p)                 MWRITE_8((uint8_t *)p + 9, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH10_VLAN_COMMAND_ID_READ(r, p)                 MREAD_8((uint8_t *)p + 10, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH10_VLAN_COMMAND_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p + 10, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH11_VLAN_COMMAND_ID_READ(r, p)                 MREAD_8((uint8_t *)p + 11, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH11_VLAN_COMMAND_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p + 11, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH12_VLAN_COMMAND_ID_READ(r, p)                 MREAD_8((uint8_t *)p + 12, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH12_VLAN_COMMAND_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p + 12, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH13_VLAN_COMMAND_ID_READ(r, p)                 MREAD_8((uint8_t *)p + 13, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH13_VLAN_COMMAND_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p + 13, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH14_VLAN_COMMAND_ID_READ(r, p)                 MREAD_8((uint8_t *)p + 14, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH14_VLAN_COMMAND_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p + 14, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH15_VLAN_COMMAND_ID_READ(r, p)                 MREAD_8((uint8_t *)p + 15, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH15_VLAN_COMMAND_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p + 15, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH16_VLAN_COMMAND_ID_READ(r, p)                 MREAD_8((uint8_t *)p + 16, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH16_VLAN_COMMAND_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p + 16, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH17_VLAN_COMMAND_ID_READ(r, p)                 MREAD_8((uint8_t *)p + 17, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH17_VLAN_COMMAND_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p + 17, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH18_VLAN_COMMAND_ID_READ(r, p)                 MREAD_8((uint8_t *)p + 18, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH18_VLAN_COMMAND_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p + 18, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH19_VLAN_COMMAND_ID_READ(r, p)                 MREAD_8((uint8_t *)p + 19, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH19_VLAN_COMMAND_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p + 19, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH20_VLAN_COMMAND_ID_READ(r, p)                 MREAD_8((uint8_t *)p + 20, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH20_VLAN_COMMAND_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p + 20, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH21_VLAN_COMMAND_ID_READ(r, p)                 MREAD_8((uint8_t *)p + 21, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH21_VLAN_COMMAND_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p + 21, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH22_VLAN_COMMAND_ID_READ(r, p)                 MREAD_8((uint8_t *)p + 22, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH22_VLAN_COMMAND_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p + 22, v)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH23_VLAN_COMMAND_ID_READ(r, p)                 MREAD_8((uint8_t *)p + 23, r)
#define RDD_VLAN_COMMAND_INDEX_ENTRY_ETH23_VLAN_COMMAND_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p + 23, v)
#endif
#if defined G9991

#define RDD_VLAN_COMMAND_INDEX_TABLE_SIZE     256
typedef struct
{
	RDD_VLAN_COMMAND_INDEX_ENTRY_DTS	entry[ RDD_VLAN_COMMAND_INDEX_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_VLAN_COMMAND_INDEX_TABLE_DTS;

#define RDD_VLAN_COMMAND_INDEX_TABLE_PTR()	( RDD_VLAN_COMMAND_INDEX_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + VLAN_COMMAND_INDEX_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_PBITS_TO_PBITS_TABLE_SIZE     32
#define RDD_DS_PBITS_TO_PBITS_TABLE_SIZE2    8
typedef struct
{
	RDD_PBITS_TO_PBITS_ENTRY_DTS	entry[ RDD_DS_PBITS_TO_PBITS_TABLE_SIZE ][ RDD_DS_PBITS_TO_PBITS_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_PBITS_TO_PBITS_TABLE_DTS;

#define RDD_DS_PBITS_TO_PBITS_TABLE_PTR()	( RDD_DS_PBITS_TO_PBITS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PBITS_TO_PBITS_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_WAN_FLOW_TABLE_SIZE     256
typedef struct
{
	RDD_DS_WAN_FLOW_ENTRY_DTS	entry[ RDD_DS_WAN_FLOW_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_WAN_FLOW_TABLE_DTS;

#define RDD_DS_WAN_FLOW_TABLE_PTR()	( RDD_DS_WAN_FLOW_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_WAN_FLOW_TABLE_ADDRESS )

#endif
#if defined G9991

typedef struct
{
	uint32_t	vlan_index_table_ptr	:13	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	opbit_remark_mode   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ipbit_remark_mode   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wifi_ssid           	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dei_remark_enable   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dei_value           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	service_queue_mode  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	forward_mode        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_port         	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	qos_mapping_mode    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	traffic_class       	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	drop                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cpu                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	policer_mode        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	policer_id          	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_shaping_mode   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cpu_mirroring       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ic_ip_flow          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	service_queue       	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	subnet_id           	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	outer_pbit          	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	inner_pbit          	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dscp_remarking_mode 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dscp                	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ecn                 	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS;

#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_VLAN_INDEX_TABLE_PTR_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p, 3, 13, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_VLAN_INDEX_TABLE_PTR_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p, 3, 13, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OPBIT_REMARK_MODE_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 1, 2, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OPBIT_REMARK_MODE_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 1, 2, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IPBIT_REMARK_MODE_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 1, 1, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IPBIT_REMARK_MODE_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 1, 1, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WIFI_SSID_READ(r, p)                            FIELD_MREAD_32((uint8_t *)p + 0, 13, 4, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_WIFI_SSID_WRITE(v, p)                           FIELD_MWRITE_32((uint8_t *)p + 0, 13, 4, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_REMARK_ENABLE_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 2, 4, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_REMARK_ENABLE_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 2, 4, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_VALUE_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 2, 3, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DEI_VALUE_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 2, 3, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SERVICE_QUEUE_MODE_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p + 2, 2, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SERVICE_QUEUE_MODE_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p + 2, 2, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_FORWARD_MODE_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 2, 1, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_FORWARD_MODE_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 2, 1, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_EGRESS_PORT_READ(r, p)                          FIELD_MREAD_16((uint8_t *)p + 2, 4, 5, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_EGRESS_PORT_WRITE(v, p)                         FIELD_MWRITE_16((uint8_t *)p + 2, 4, 5, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_MAPPING_MODE_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 3, 3, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_QOS_MAPPING_MODE_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 3, 3, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_TRAFFIC_CLASS_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 3, 0, 3, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_TRAFFIC_CLASS_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 3, 0, 3, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DROP_READ(r, p)                                 FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DROP_WRITE(v, p)                                FIELD_MWRITE_8((uint8_t *)p + 4, 7, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CPU_READ(r, p)                                  FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CPU_WRITE(v, p)                                 FIELD_MWRITE_8((uint8_t *)p + 4, 6, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_MODE_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 4, 5, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_MODE_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 4, 5, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_ID_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 4, 1, 4, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_POLICER_ID_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 4, 1, 4, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_RATE_SHAPING_MODE_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 4, 0, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_RATE_SHAPING_MODE_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 4, 0, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CPU_MIRRORING_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 5, 7, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_CPU_MIRRORING_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 5, 7, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IC_IP_FLOW_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 5, 6, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_IC_IP_FLOW_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 5, 6, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SERVICE_QUEUE_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 5, 1, 5, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SERVICE_QUEUE_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 5, 1, 5, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SUBNET_ID_READ(r, p)                            FIELD_MREAD_32((uint8_t *)p + 4, 15, 2, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_SUBNET_ID_WRITE(v, p)                           FIELD_MWRITE_32((uint8_t *)p + 4, 15, 2, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OUTER_PBIT_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 6, 4, 3, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_OUTER_PBIT_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 6, 4, 3, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_INNER_PBIT_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 6, 1, 3, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_INNER_PBIT_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 6, 1, 3, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_REMARKING_MODE_READ(r, p)                  FIELD_MREAD_8((uint8_t *)p + 6, 0, 1, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_REMARKING_MODE_WRITE(v, p)                 FIELD_MWRITE_8((uint8_t *)p + 6, 0, 1, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_READ(r, p)                                 FIELD_MREAD_8((uint8_t *)p + 7, 2, 6, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DSCP_WRITE(v, p)                                FIELD_MWRITE_8((uint8_t *)p + 7, 2, 6, v)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_ECN_READ(r, p)                                  FIELD_MREAD_8((uint8_t *)p + 7, 0, 2, r)
#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_ECN_WRITE(v, p)                                 FIELD_MWRITE_8((uint8_t *)p + 7, 0, 2, v)
#endif
#if defined G9991

#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE     256
typedef struct
{
	RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS	entry[ RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS;

#define RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_PTR()	( RDD_DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_CONTEXT_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_SIZE     10
#define RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_SIZE2    32
typedef struct
{
	RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS	entry[ RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_SIZE ][ RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_DTS;

#define RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_PTR()	( RDD_DS_INGRESS_FILTERS_LOOKUP_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_FILTERS_LOOKUP_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_TABLE_SIZE     128
typedef struct
{
	RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_DTS	entry[ RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_TABLE_DTS;

#define RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_TABLE_PTR()	( RDD_ETH_TX_RS_QUEUE_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + ETH_TX_RS_QUEUE_DESCRIPTOR_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_ETH_TX_QUEUES_TABLE_SIZE     120
typedef struct
{
	RDD_ETH_TX_QUEUE_DESCRIPTOR_DTS	entry[ RDD_ETH_TX_QUEUES_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_QUEUES_TABLE_DTS;

#define RDD_ETH_TX_QUEUES_TABLE_PTR()	( RDD_ETH_TX_QUEUES_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + ETH_TX_QUEUES_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_QUEUE_PROFILE_TABLE_SIZE     8
typedef struct
{
	RDD_QUEUE_PROFILE_DTS	entry[ RDD_DS_QUEUE_PROFILE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_QUEUE_PROFILE_TABLE_DTS;

#define RDD_DS_QUEUE_PROFILE_TABLE_PTR()	( RDD_DS_QUEUE_PROFILE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_QUEUE_PROFILE_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_PBITS_PARAMETER_TABLE_SIZE     128
typedef struct
{
	RDD_PBITS_PARAMETER_ENTRY_DTS	entry[ RDD_DS_PBITS_PARAMETER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_PBITS_PARAMETER_TABLE_DTS;

#define RDD_DS_PBITS_PARAMETER_TABLE_PTR()	( RDD_DS_PBITS_PARAMETER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PBITS_PARAMETER_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_IPTV_SSID_EXTENSION_TABLE_SIZE     256
typedef struct
{
	RDD_IPTV_SSID_EXTENSION_ENTRY_DTS	entry[ RDD_IPTV_SSID_EXTENSION_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_SSID_EXTENSION_TABLE_DTS;

#define RDD_IPTV_SSID_EXTENSION_TABLE_PTR()	( RDD_IPTV_SSID_EXTENSION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_SSID_EXTENSION_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_CPU_REASON_TO_METER_TABLE_SIZE     64
typedef struct
{
	RDD_CPU_REASON_TO_METER_ENTRY_DTS	entry[ RDD_DS_CPU_REASON_TO_METER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_CPU_REASON_TO_METER_TABLE_DTS;

#define RDD_DS_CPU_REASON_TO_METER_TABLE_PTR()	( RDD_DS_CPU_REASON_TO_METER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_CPU_REASON_TO_METER_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE     288
typedef struct
{
	RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS	entry[ RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS;

#define RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_PTR()	( RDD_IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_INGRESS_CLASSIFICATION_CONTEXT_TABLE_ADDRESS )

#endif
#define RDD_DS_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_PTR()	( RDD_DS_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_ADDRESS )

#if defined G9991

#define RDD_DS_VLAN_PARAMETER_TABLE_SIZE     128
typedef struct
{
	RDD_VLAN_PARAMETER_ENTRY_DTS	entry[ RDD_DS_VLAN_PARAMETER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_VLAN_PARAMETER_TABLE_DTS;

#define RDD_DS_VLAN_PARAMETER_TABLE_PTR()	( RDD_DS_VLAN_PARAMETER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_VLAN_PARAMETER_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_IPTV_COUNTERS_TABLE_SIZE     288
typedef struct
{
	RDD_IPTV_COUNTER_ENTRY_DTS	entry[ RDD_IPTV_COUNTERS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_COUNTERS_TABLE_DTS;

#define RDD_IPTV_COUNTERS_TABLE_PTR()	( RDD_IPTV_COUNTERS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + IPTV_COUNTERS_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_RATE_LIMITER_REMAINDER_TABLE_SIZE     32
typedef struct
{
	RDD_RATE_LIMITER_REMAINDER_ENTRY_DTS	entry[ RDD_RATE_LIMITER_REMAINDER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RATE_LIMITER_REMAINDER_TABLE_DTS;

#define RDD_RATE_LIMITER_REMAINDER_TABLE_PTR()	( RDD_RATE_LIMITER_REMAINDER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + RATE_LIMITER_REMAINDER_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_LAYER4_FILTERS_LOOKUP_TABLE_SIZE     16
typedef struct
{
	RDD_LAYER4_FILTERS_LOOKUP_ENTRY_DTS	entry[ RDD_DS_LAYER4_FILTERS_LOOKUP_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_LAYER4_FILTERS_LOOKUP_TABLE_DTS;

#define RDD_DS_LAYER4_FILTERS_LOOKUP_TABLE_PTR()	( RDD_DS_LAYER4_FILTERS_LOOKUP_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_LAYER4_FILTERS_LOOKUP_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_LAN_VID_TABLE_SIZE     128
typedef struct
{
	RDD_VID_ENTRY_DTS	entry[ RDD_DS_LAN_VID_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_LAN_VID_TABLE_DTS;

#define RDD_DS_LAN_VID_TABLE_PTR()	( RDD_DS_LAN_VID_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_LAN_VID_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_ETH_TX_QUEUES_POINTERS_TABLE_SIZE     120
typedef struct
{
	RDD_ETH_TX_QUEUE_POINTERS_ENTRY_DTS	entry[ RDD_ETH_TX_QUEUES_POINTERS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_QUEUES_POINTERS_TABLE_DTS;

#define RDD_ETH_TX_QUEUES_POINTERS_TABLE_PTR()	( RDD_ETH_TX_QUEUES_POINTERS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + ETH_TX_QUEUES_POINTERS_TABLE_ADDRESS )

#endif
#define RDD_DS_PICO_TIMER_TASK_DESCRIPTOR_TABLE_PTR()	( RDD_DS_PICO_TIMER_TASK_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PICO_TIMER_TASK_DESCRIPTOR_TABLE_ADDRESS )

#if defined G9991

#define RDD_DS_PBITS_PRIMITIVE_TABLE_SIZE     32
typedef struct
{
	RDD_PBITS_PRIMITIVE_ENTRY_DTS	entry[ RDD_DS_PBITS_PRIMITIVE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_PBITS_PRIMITIVE_TABLE_DTS;

#define RDD_DS_PBITS_PRIMITIVE_TABLE_PTR()	( RDD_DS_PBITS_PRIMITIVE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PBITS_PRIMITIVE_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_SIZE     16
typedef struct
{
	RDD_FOUR_BYTES_DTS	entry[ RDD_DS_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_DTS;

#define RDD_DS_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_PTR()	( RDD_DS_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_SIZE     16
typedef struct
{
	RDD_FOUR_BYTES_DTS	entry[ RDD_DS_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_DTS;

#define RDD_DS_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_PTR()	( RDD_DS_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_SIZE     32
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_DS_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_DTS;

#define RDD_DS_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_PTR()	( RDD_DS_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_RATE_LIMITER_TABLE_SIZE     32
typedef struct
{
	RDD_RATE_LIMITER_ENTRY_DTS	entry[ RDD_DS_RATE_LIMITER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_RATE_LIMITER_TABLE_DTS;

#define RDD_DS_RATE_LIMITER_TABLE_PTR()	( RDD_DS_RATE_LIMITER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_RATE_LIMITER_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_VLAN_COMMANDS_TABLE_SIZE     64
typedef struct
{
	RDD_VLAN_COMMAND_ENRTY_DTS	entry[ RDD_DS_VLAN_COMMANDS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_VLAN_COMMANDS_TABLE_DTS;

#define RDD_DS_VLAN_COMMANDS_TABLE_PTR()	( RDD_DS_VLAN_COMMANDS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_VLAN_COMMANDS_TABLE_ADDRESS )

#endif
#define RDD_DS_POLICER_TABLE_PTR()	( RDD_DS_POLICER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_POLICER_TABLE_ADDRESS )

#if defined G9991

#define RDD_ETH_TX_LOCAL_REGISTERS_SIZE     30
typedef struct
{
	RDD_ETH_TX_LOCAL_REGISTERS_ENTRY_DTS	entry[ RDD_ETH_TX_LOCAL_REGISTERS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_LOCAL_REGISTERS_DTS;

#define RDD_ETH_TX_LOCAL_REGISTERS_PTR()	( RDD_ETH_TX_LOCAL_REGISTERS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + ETH_TX_LOCAL_REGISTERS_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_TPID_OVERWRITE_TABLE_SIZE     8
typedef struct
{
	RDD_TPID_OVERWRITE_ENTRY_DTS	entry[ RDD_DS_TPID_OVERWRITE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_TPID_OVERWRITE_TABLE_DTS;

#define RDD_DS_TPID_OVERWRITE_TABLE_PTR()	( RDD_DS_TPID_OVERWRITE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_TPID_OVERWRITE_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_CPU_TX_BBH_DESCRIPTORS_SIZE     32
typedef struct
{
	RDD_CPU_TX_BBH_DESCRIPTORS_ENTRY_DTS	entry[ RDD_DS_CPU_TX_BBH_DESCRIPTORS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_CPU_TX_BBH_DESCRIPTORS_DTS;

#define RDD_DS_CPU_TX_BBH_DESCRIPTORS_PTR()	( RDD_DS_CPU_TX_BBH_DESCRIPTORS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_CPU_TX_BBH_DESCRIPTORS_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_SIZE     10
#define RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_SIZE2    16
typedef struct
{
	RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DTS	entry[ RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_SIZE ][ RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_DTS;

#define RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_PTR()	( RDD_DS_INGRESS_FILTERS_PARAMETER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_FILTERS_PARAMETER_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_SIZE     4
typedef struct
{
	RDD_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_ENTRY_DTS	entry[ RDD_DS_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_DTS;

#define RDD_DS_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_PTR()	( RDD_DS_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_CPU_RX_PD_INGRESS_QUEUE_SIZE     32
typedef struct
{
	RDD_EIGHT_BYTES_DTS	entry[ RDD_CPU_RX_PD_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_PD_INGRESS_QUEUE_DTS;

#define RDD_CPU_RX_PD_INGRESS_QUEUE_PTR()	( RDD_CPU_RX_PD_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + CPU_RX_PD_INGRESS_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_FORWARDING_MATRIX_TABLE_SIZE     9
#define RDD_DS_FORWARDING_MATRIX_TABLE_SIZE2    16
typedef struct
{
	RDD_FORWARDING_MATRIX_ENTRY_DTS	entry[ RDD_DS_FORWARDING_MATRIX_TABLE_SIZE ][ RDD_DS_FORWARDING_MATRIX_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_FORWARDING_MATRIX_TABLE_DTS;

#define RDD_DS_FORWARDING_MATRIX_TABLE_PTR()	( RDD_DS_FORWARDING_MATRIX_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_FORWARDING_MATRIX_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_TIMER_SCHEDULER_PRIMITIVE_TABLE_SIZE     8
typedef struct
{
	RDD_TIMER_SCHEDULER_PRIMITIVE_ENTRY_DTS	entry[ RDD_DS_TIMER_SCHEDULER_PRIMITIVE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_TIMER_SCHEDULER_PRIMITIVE_TABLE_DTS;

#define RDD_DS_TIMER_SCHEDULER_PRIMITIVE_TABLE_PTR()	( RDD_DS_TIMER_SCHEDULER_PRIMITIVE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_TIMER_SCHEDULER_PRIMITIVE_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_FAST_RUNNER_GLOBAL_REGISTERS_INIT_SIZE     8
typedef struct
{
	RDD_FAST_RUNNER_GLOBAL_REGISTERS_INIT_ENTRY_DTS	entry[ RDD_DS_FAST_RUNNER_GLOBAL_REGISTERS_INIT_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_FAST_RUNNER_GLOBAL_REGISTERS_INIT_DTS;

#define RDD_DS_FAST_RUNNER_GLOBAL_REGISTERS_INIT_PTR()	( RDD_DS_FAST_RUNNER_GLOBAL_REGISTERS_INIT_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_FAST_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_FLOW_BASED_ACTION_PTR_TABLE_SIZE     32
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_DS_FLOW_BASED_ACTION_PTR_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_FLOW_BASED_ACTION_PTR_TABLE_DTS;

#define RDD_DS_FLOW_BASED_ACTION_PTR_TABLE_PTR()	( RDD_DS_FLOW_BASED_ACTION_PTR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_FLOW_BASED_ACTION_PTR_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_CPU_RX_FAST_PD_INGRESS_QUEUE_SIZE     32
typedef struct
{
	RDD_EIGHT_BYTES_DTS	entry[ RDD_CPU_RX_FAST_PD_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_FAST_PD_INGRESS_QUEUE_DTS;

#define RDD_CPU_RX_FAST_PD_INGRESS_QUEUE_PTR()	( RDD_CPU_RX_FAST_PD_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + CPU_RX_FAST_PD_INGRESS_QUEUE_ADDRESS )

#endif
#define RDD_DS_CPU_RX_METER_TABLE_PTR()	( RDD_DS_CPU_RX_METER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_CPU_RX_METER_TABLE_ADDRESS )

#if defined G9991

#define RDD_INGRESS_HANDLER_SKB_DATA_POINTER_SIZE     32
typedef struct
{
	RDD_FOUR_BYTES_DTS	entry[ RDD_INGRESS_HANDLER_SKB_DATA_POINTER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INGRESS_HANDLER_SKB_DATA_POINTER_DTS;

#define RDD_INGRESS_HANDLER_SKB_DATA_POINTER_PTR()	( RDD_INGRESS_HANDLER_SKB_DATA_POINTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + INGRESS_HANDLER_SKB_DATA_POINTER_ADDRESS )

#endif
#if defined G9991

typedef struct
{
	uint32_t	egress_counter              	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	loopback_mode               	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved6                   	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ingress_counter             	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_limiter_id             	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tx_task_number              	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved7                   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved8                   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved9                   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved10                  	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved11                  	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved12                  	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved13                  	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eof_flag                    	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tx_queues_status            	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	packet_counters_ptr         	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	gpio_flow_control_vector_ptr	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	last_queue_served           	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	phyiscal_port               	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	emac_mask                   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1                   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2                   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved3                   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved4                   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved5                   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_MAC_DESCRIPTOR_DTS;

#define RDD_ETH_TX_MAC_DESCRIPTOR_EGRESS_COUNTER_READ(r, p)                               MREAD_8((uint8_t *)p, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_EGRESS_COUNTER_WRITE(v, p)                              MWRITE_8((uint8_t *)p, v)
#define RDD_ETH_TX_MAC_DESCRIPTOR_LOOPBACK_MODE_READ(r, p)                                MREAD_8((uint8_t *)p + 1, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_LOOPBACK_MODE_WRITE(v, p)                               MWRITE_8((uint8_t *)p + 1, v)
#define RDD_ETH_TX_MAC_DESCRIPTOR_INGRESS_COUNTER_READ(r, p)                              MREAD_8((uint8_t *)p + 4, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_INGRESS_COUNTER_WRITE(v, p)                             MWRITE_8((uint8_t *)p + 4, v)
#define RDD_ETH_TX_MAC_DESCRIPTOR_RATE_LIMITER_ID_READ(r, p)                              MREAD_8((uint8_t *)p + 5, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_RATE_LIMITER_ID_WRITE(v, p)                             MWRITE_8((uint8_t *)p + 5, v)
#define RDD_ETH_TX_MAC_DESCRIPTOR_TX_TASK_NUMBER_READ(r, p)                               MREAD_8((uint8_t *)p + 6, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_TX_TASK_NUMBER_WRITE(v, p)                              MWRITE_8((uint8_t *)p + 6, v)
#define RDD_ETH_TX_MAC_DESCRIPTOR_EOF_FLAG_READ(r, p)                                     MREAD_8((uint8_t *)p + 32, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_EOF_FLAG_WRITE(v, p)                                    MWRITE_8((uint8_t *)p + 32, v)
#define RDD_ETH_TX_MAC_DESCRIPTOR_TX_QUEUES_STATUS_READ(r, p)                             MREAD_8((uint8_t *)p + 33, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_TX_QUEUES_STATUS_WRITE(v, p)                            MWRITE_8((uint8_t *)p + 33, v)
#define RDD_ETH_TX_MAC_DESCRIPTOR_PACKET_COUNTERS_PTR_READ(r, p)                          MREAD_16((uint8_t *)p + 34, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_PACKET_COUNTERS_PTR_WRITE(v, p)                         MWRITE_16((uint8_t *)p + 34, v)
#define RDD_ETH_TX_MAC_DESCRIPTOR_GPIO_FLOW_CONTROL_VECTOR_PTR_READ(r, p)                 MREAD_16((uint8_t *)p + 36, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_GPIO_FLOW_CONTROL_VECTOR_PTR_WRITE(v, p)                MWRITE_16((uint8_t *)p + 36, v)
#define RDD_ETH_TX_MAC_DESCRIPTOR_LAST_QUEUE_SERVED_READ(r, p)                            MREAD_8((uint8_t *)p + 38, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_LAST_QUEUE_SERVED_WRITE(v, p)                           MWRITE_8((uint8_t *)p + 38, v)
#define RDD_ETH_TX_MAC_DESCRIPTOR_PHYISCAL_PORT_READ(r, p)                                MREAD_8((uint8_t *)p + 39, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_PHYISCAL_PORT_WRITE(v, p)                               MWRITE_8((uint8_t *)p + 39, v)
#define RDD_ETH_TX_MAC_DESCRIPTOR_EMAC_MASK_READ(r, p)                                    MREAD_32((uint8_t *)p + 40, r)
#define RDD_ETH_TX_MAC_DESCRIPTOR_EMAC_MASK_WRITE(v, p)                                   MWRITE_32((uint8_t *)p + 40, v)
#endif
#if defined G9991

#define RDD_ETH_TX_MAC_TABLE_SIZE     30
typedef struct
{
	RDD_ETH_TX_MAC_DESCRIPTOR_DTS	entry[ RDD_ETH_TX_MAC_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_MAC_TABLE_DTS;

#define RDD_ETH_TX_MAC_TABLE_PTR()	( RDD_ETH_TX_MAC_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + ETH_TX_MAC_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE     16
typedef struct
{
	RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_DTS	entry[ RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS;

#define RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_PTR()	( RDD_DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DOWNSTREAM_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_SIZE     16
typedef struct
{
	RDD_EIGHT_BYTES_DTS	entry[ RDD_DOWNSTREAM_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DOWNSTREAM_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_DTS;

#define RDD_DOWNSTREAM_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_PTR()	( RDD_DOWNSTREAM_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DOWNSTREAM_MULTICAST_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_VLAN_OPTIMIZATION_TABLE_SIZE     128
typedef struct
{
	RDD_VLAN_OPTIMIZATION_ENTRY_DTS	entry[ RDD_DS_VLAN_OPTIMIZATION_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_VLAN_OPTIMIZATION_TABLE_DTS;

#define RDD_DS_VLAN_OPTIMIZATION_TABLE_PTR()	( RDD_DS_VLAN_OPTIMIZATION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_VLAN_OPTIMIZATION_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DOWNSTREAM_MULTICAST_LAN_ENQUEUE_SERVICE_QUEUE_SIZE     16
typedef struct
{
	RDD_EIGHT_BYTES_DTS	entry[ RDD_DOWNSTREAM_MULTICAST_LAN_ENQUEUE_SERVICE_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DOWNSTREAM_MULTICAST_LAN_ENQUEUE_SERVICE_QUEUE_DTS;

#define RDD_DOWNSTREAM_MULTICAST_LAN_ENQUEUE_SERVICE_QUEUE_PTR()	( RDD_DOWNSTREAM_MULTICAST_LAN_ENQUEUE_SERVICE_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DOWNSTREAM_MULTICAST_LAN_ENQUEUE_SERVICE_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_CPU_RX_MIRRORING_PD_INGRESS_QUEUE_SIZE     16
typedef struct
{
	RDD_EIGHT_BYTES_DTS	entry[ RDD_CPU_RX_MIRRORING_PD_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_MIRRORING_PD_INGRESS_QUEUE_DTS;

#define RDD_CPU_RX_MIRRORING_PD_INGRESS_QUEUE_PTR()	( RDD_CPU_RX_MIRRORING_PD_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + CPU_RX_MIRRORING_PD_INGRESS_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_SIZE     5
typedef struct
{
	RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_ENTRY_DTS	entry[ RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_DTS;

#define RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_PTR()	( RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + EMAC_SKB_ENQUEUED_INDEXES_FIFO_ADDRESS )

#endif
#if defined G9991

#define RDD_G9991_FRAGMENT_BUFFER_SIZE     132
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_G9991_FRAGMENT_BUFFER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_G9991_FRAGMENT_BUFFER_DTS;

#define RDD_G9991_FRAGMENT_BUFFER_PTR()	( RDD_G9991_FRAGMENT_BUFFER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + G9991_FRAGMENT_BUFFER_ADDRESS )

#endif
#if defined G9991

typedef struct
{
	uint32_t	status_vector	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_EMACS_STATUS_ENTRY_DTS;

#define RDD_ETH_TX_EMACS_STATUS_ENTRY_STATUS_VECTOR_READ(r, p)                 MREAD_32((uint8_t *)p, r)
#define RDD_ETH_TX_EMACS_STATUS_ENTRY_STATUS_VECTOR_WRITE(v, p)                MWRITE_32((uint8_t *)p, v)
#endif
#if defined G9991

#define RDD_EMAC_SKB_ENQUEUED_INDEXES_PUT_PTR_SIZE     8
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_EMAC_SKB_ENQUEUED_INDEXES_PUT_PTR_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EMAC_SKB_ENQUEUED_INDEXES_PUT_PTR_DTS;

#define RDD_EMAC_SKB_ENQUEUED_INDEXES_PUT_PTR_PTR()	( RDD_EMAC_SKB_ENQUEUED_INDEXES_PUT_PTR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + EMAC_SKB_ENQUEUED_INDEXES_PUT_PTR_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_SIZE     6
#define RDD_DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_SIZE2    8
typedef struct
{
	RDD_QUEUE_ENTRY_DTS	entry[ RDD_DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_SIZE ][ RDD_DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_DTS;

#define RDD_DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_PTR()	( RDD_DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_TRAFFIC_CLASS_TO_QUEUE_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_EMAC_SKB_ENQUEUED_INDEXES_FREE_PTR_SIZE     8
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_EMAC_SKB_ENQUEUED_INDEXES_FREE_PTR_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EMAC_SKB_ENQUEUED_INDEXES_FREE_PTR_DTS;

#define RDD_EMAC_SKB_ENQUEUED_INDEXES_FREE_PTR_PTR()	( RDD_EMAC_SKB_ENQUEUED_INDEXES_FREE_PTR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + EMAC_SKB_ENQUEUED_INDEXES_FREE_PTR_ADDRESS )

#endif
#if defined G9991

#define RDD_DOWNSTREAM_LAN_ENQUEUE_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_DOWNSTREAM_LAN_ENQUEUE_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DOWNSTREAM_LAN_ENQUEUE_INGRESS_QUEUE_DTS;

#define RDD_DOWNSTREAM_LAN_ENQUEUE_INGRESS_QUEUE_PTR()	( RDD_DOWNSTREAM_LAN_ENQUEUE_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DOWNSTREAM_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_DSCP_TO_PBITS_TABLE_SIZE2    64
typedef struct
{
	RDD_DSCP_TO_PBITS_ENTRY_DTS	entry[ RDD_DS_DSCP_TO_PBITS_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_DSCP_TO_PBITS_TABLE_DTS;

#define RDD_DS_DSCP_TO_PBITS_TABLE_PTR()	( RDD_DS_DSCP_TO_PBITS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_DSCP_TO_PBITS_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_G9991_FRAGMENT_ENQUEUE_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_G9991_FRAGMENT_ENQUEUE_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_G9991_FRAGMENT_ENQUEUE_INGRESS_QUEUE_DTS;

#define RDD_G9991_FRAGMENT_ENQUEUE_INGRESS_QUEUE_PTR()	( RDD_G9991_FRAGMENT_ENQUEUE_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + G9991_FRAGMENT_ENQUEUE_INGRESS_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_VLAN_PRIMITIVE_TABLE_SIZE     32
typedef struct
{
	RDD_VLAN_PRIMITIVE_ENTRY_DTS	entry[ RDD_DS_VLAN_PRIMITIVE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_VLAN_PRIMITIVE_TABLE_DTS;

#define RDD_DS_VLAN_PRIMITIVE_TABLE_PTR()	( RDD_DS_VLAN_PRIMITIVE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_VLAN_PRIMITIVE_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DOWNSTREAM_LAN_ENQUEUE_SERVICE_QUEUE_SIZE     64
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_DOWNSTREAM_LAN_ENQUEUE_SERVICE_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DOWNSTREAM_LAN_ENQUEUE_SERVICE_QUEUE_DTS;

#define RDD_DOWNSTREAM_LAN_ENQUEUE_SERVICE_QUEUE_PTR()	( RDD_DOWNSTREAM_LAN_ENQUEUE_SERVICE_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DOWNSTREAM_LAN_ENQUEUE_SERVICE_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_CPU_RX_PICO_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_DS_CPU_RX_PICO_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_CPU_RX_PICO_INGRESS_QUEUE_DTS;

#define RDD_DS_CPU_RX_PICO_INGRESS_QUEUE_PTR()	( RDD_DS_CPU_RX_PICO_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_CPU_RX_PICO_INGRESS_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_DOWNSTREAM_MULTICAST_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_DOWNSTREAM_MULTICAST_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DOWNSTREAM_MULTICAST_INGRESS_QUEUE_DTS;

#define RDD_DOWNSTREAM_MULTICAST_INGRESS_QUEUE_PTR()	( RDD_DOWNSTREAM_MULTICAST_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DOWNSTREAM_MULTICAST_INGRESS_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_PICO_RUNNER_GLOBAL_REGISTERS_INIT_SIZE     8
typedef struct
{
	RDD_PICO_RUNNER_GLOBAL_REGISTERS_INIT_ENTRY_DTS	entry[ RDD_DS_PICO_RUNNER_GLOBAL_REGISTERS_INIT_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_PICO_RUNNER_GLOBAL_REGISTERS_INIT_DTS;

#define RDD_DS_PICO_RUNNER_GLOBAL_REGISTERS_INIT_PTR()	( RDD_DS_PICO_RUNNER_GLOBAL_REGISTERS_INIT_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PICO_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_DATA_POINTER_DUMMY_TARGET_SIZE     5
typedef struct
{
	RDD_FOUR_BYTES_DTS	entry[ RDD_DS_DATA_POINTER_DUMMY_TARGET_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_DATA_POINTER_DUMMY_TARGET_DTS;

#define RDD_DS_DATA_POINTER_DUMMY_TARGET_PTR()	( RDD_DS_DATA_POINTER_DUMMY_TARGET_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_DATA_POINTER_DUMMY_TARGET_ADDRESS )

#endif
#if defined G9991

typedef struct
{
	uint32_t	emac_offset	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_TX_INTER_LAN_SCHEDULING_OFFSET_ENTRY_DTS;

#define RDD_ETH_TX_INTER_LAN_SCHEDULING_OFFSET_ENTRY_EMAC_OFFSET_READ(r, p)                 MREAD_32((uint8_t *)p, r)
#define RDD_ETH_TX_INTER_LAN_SCHEDULING_OFFSET_ENTRY_EMAC_OFFSET_WRITE(v, p)                MWRITE_32((uint8_t *)p, v)
#endif
#if defined G9991

#define RDD_DS_ROUTER_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_DS_ROUTER_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_ROUTER_INGRESS_QUEUE_DTS;

#define RDD_DS_ROUTER_INGRESS_QUEUE_PTR()	( RDD_DS_ROUTER_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_ROUTER_INGRESS_QUEUE_ADDRESS )

#endif
#define RDD_DS_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_PTR()	( RDD_DS_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_ADDRESS )

#define RDD_DS_PICO_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_PTR()	( RDD_DS_PICO_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_PICO_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_ADDRESS )

#if defined G9991

#define RDD_DS_CPU_RX_FAST_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_DS_CPU_RX_FAST_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_CPU_RX_FAST_INGRESS_QUEUE_DTS;

#define RDD_DS_CPU_RX_FAST_INGRESS_QUEUE_PTR()	( RDD_DS_CPU_RX_FAST_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_CPU_RX_FAST_INGRESS_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_IPTV_SBPM_REPLICATION_BN_SIZE     16
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_DS_IPTV_SBPM_REPLICATION_BN_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_IPTV_SBPM_REPLICATION_BN_DTS;

#define RDD_DS_IPTV_SBPM_REPLICATION_BN_PTR()	( RDD_DS_IPTV_SBPM_REPLICATION_BN_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_IPTV_SBPM_REPLICATION_BN_ADDRESS )

#endif
#if defined G9991

#define RDD_CPU_FLOW_CACHE_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_CPU_FLOW_CACHE_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_FLOW_CACHE_INGRESS_QUEUE_DTS;

#define RDD_CPU_FLOW_CACHE_INGRESS_QUEUE_PTR()	( RDD_CPU_FLOW_CACHE_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + CPU_FLOW_CACHE_INGRESS_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_DEBUG_BUFFER_SIZE     32
typedef struct
{
	RDD_DEBUG_BUFFER_ENTRY_DTS	entry[ RDD_DS_DEBUG_BUFFER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_DEBUG_BUFFER_DTS;

#define RDD_DS_DEBUG_BUFFER_PTR()	( RDD_DS_DEBUG_BUFFER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_DEBUG_BUFFER_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_SIZE     3
typedef struct
{
	RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS	entry[ RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS;

#define RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR()	( RDD_DS_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_INGRESS_FILTERS_CONFIGURATION_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_LAYER4_FILTERS_CONTEXT_TABLE_SIZE     16
typedef struct
{
	RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DTS	entry[ RDD_DS_LAYER4_FILTERS_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_LAYER4_FILTERS_CONTEXT_TABLE_DTS;

#define RDD_DS_LAYER4_FILTERS_CONTEXT_TABLE_PTR()	( RDD_DS_LAYER4_FILTERS_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_LAYER4_FILTERS_CONTEXT_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_EMAC_ABSOLUTE_TX_BBH_COUNTER_SIZE     6
typedef struct
{
	RDD_EIGHT_BYTES_DTS	entry[ RDD_EMAC_ABSOLUTE_TX_BBH_COUNTER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EMAC_ABSOLUTE_TX_BBH_COUNTER_DTS;

#define RDD_EMAC_ABSOLUTE_TX_BBH_COUNTER_PTR()	( RDD_EMAC_ABSOLUTE_TX_BBH_COUNTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + EMAC_ABSOLUTE_TX_BBH_COUNTER_ADDRESS )

#endif
#if defined G9991

#define RDD_ETH_PHYSICAL_PORT_ACK_PENDING_SIZE     5
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_ETH_PHYSICAL_PORT_ACK_PENDING_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH_PHYSICAL_PORT_ACK_PENDING_DTS;

#define RDD_ETH_PHYSICAL_PORT_ACK_PENDING_PTR()	( RDD_ETH_PHYSICAL_PORT_ACK_PENDING_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + ETH_PHYSICAL_PORT_ACK_PENDING_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_MULTICAST_VECTOR_TO_PORT_TABLE_SIZE     8
typedef struct
{
	RDD_MULTICAST_VECTOR_TO_PORT_ENTRY_DTS	entry[ RDD_DS_MULTICAST_VECTOR_TO_PORT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_MULTICAST_VECTOR_TO_PORT_TABLE_DTS;

#define RDD_DS_MULTICAST_VECTOR_TO_PORT_TABLE_PTR()	( RDD_DS_MULTICAST_VECTOR_TO_PORT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_MULTICAST_VECTOR_TO_PORT_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_COUNTERS_SIZE     5
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_COUNTERS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_COUNTERS_DTS;

#define RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_COUNTERS_PTR()	( RDD_EMAC_SKB_ENQUEUED_INDEXES_FIFO_COUNTERS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + EMAC_SKB_ENQUEUED_INDEXES_FIFO_COUNTERS_ADDRESS )

#endif
#if defined G9991

#define RDD_HASH_BASED_FORWARDING_PORT_TABLE_SIZE     4
typedef struct
{
	RDD_HASH_BASED_FORWARDING_PORT_ENTRY_DTS	entry[ RDD_HASH_BASED_FORWARDING_PORT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_HASH_BASED_FORWARDING_PORT_TABLE_DTS;

#define RDD_HASH_BASED_FORWARDING_PORT_TABLE_PTR()	( RDD_HASH_BASED_FORWARDING_PORT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + HASH_BASED_FORWARDING_PORT_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_GPON_RX_DIRECT_DESCRIPTORS_SIZE     32
typedef struct
{
	RDD_BBH_RX_DESCRIPTOR_DTS	entry[ RDD_GPON_RX_DIRECT_DESCRIPTORS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GPON_RX_DIRECT_DESCRIPTORS_DTS;

#define RDD_GPON_RX_DIRECT_DESCRIPTORS_PTR()	( RDD_GPON_RX_DIRECT_DESCRIPTORS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + GPON_RX_DIRECT_DESCRIPTORS_ADDRESS )

#endif
#if defined G9991

#define RDD_PCI_TX_FIFO_DESCRIPTORS_TABLE_SIZE     4
typedef struct
{
	RDD_PCI_TX_FIFO_DESCRIPTOR_ENTRY_DTS	entry[ RDD_PCI_TX_FIFO_DESCRIPTORS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PCI_TX_FIFO_DESCRIPTORS_TABLE_DTS;

#define RDD_PCI_TX_FIFO_DESCRIPTORS_TABLE_PTR()	( RDD_PCI_TX_FIFO_DESCRIPTORS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + PCI_TX_FIFO_DESCRIPTORS_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_GPON_RX_NORMAL_DESCRIPTORS_SIZE     32
typedef struct
{
	RDD_BBH_RX_DESCRIPTOR_DTS	entry[ RDD_GPON_RX_NORMAL_DESCRIPTORS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GPON_RX_NORMAL_DESCRIPTORS_DTS;

#define RDD_GPON_RX_NORMAL_DESCRIPTORS_PTR()	( RDD_GPON_RX_NORMAL_DESCRIPTORS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + GPON_RX_NORMAL_DESCRIPTORS_ADDRESS )

#endif
/* PRIVATE_B */
#if defined G9991

#define RDD_US_INGRESS_HANDLER_BUFFER_SIZE     32
typedef struct
{
	RDD_IH_BUFFER_DTS	entry[ RDD_US_INGRESS_HANDLER_BUFFER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_HANDLER_BUFFER_DTS;

#define RDD_US_INGRESS_HANDLER_BUFFER_PTR()	( RDD_US_INGRESS_HANDLER_BUFFER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_HANDLER_BUFFER_ADDRESS )

#endif
#if defined G9991

#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE     256
typedef struct
{
	RDD_US_INGRESS_CLASSIFICATION_CONTEXT_ENTRY_DTS	entry[ RDD_US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_PTR()	( RDD_US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_CONTEXT_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_DSCP_TO_PBITS_TABLE_SIZE     6
#define RDD_US_DSCP_TO_PBITS_TABLE_SIZE2    64
typedef struct
{
	RDD_DSCP_TO_PBITS_ENTRY_DTS	entry[ RDD_US_DSCP_TO_PBITS_TABLE_SIZE ][ RDD_US_DSCP_TO_PBITS_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_DSCP_TO_PBITS_TABLE_DTS;

#define RDD_US_DSCP_TO_PBITS_TABLE_PTR()	( RDD_US_DSCP_TO_PBITS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_DSCP_TO_PBITS_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_VLAN_PARAMETER_TABLE_SIZE     128
typedef struct
{
	RDD_VLAN_PARAMETER_ENTRY_DTS	entry[ RDD_US_VLAN_PARAMETER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_VLAN_PARAMETER_TABLE_DTS;

#define RDD_US_VLAN_PARAMETER_TABLE_PTR()	( RDD_US_VLAN_PARAMETER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_VLAN_PARAMETER_TABLE_ADDRESS )

#endif
#if defined G9991

typedef struct
{
	uint32_t	reserved0                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	state_machine              	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__; /*defined by rdd_us_sid_state_machine enumeration*/
	uint32_t	fragment_count             	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1                  	:24	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2                  	:24	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cpu_rx_cpu_reason          	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cpu_rx_pd_0                	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cpu_rx_length              	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cpu_rx_pd_1                	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cpu_rx_bpm_number          	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved3                  	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	upstream_vlan_vlan_tags    	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	upstream_vlan_l3_protocol  	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	upstream_vlan_dscp         	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	upstream_outer_vlan        	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	upstream_ingress_flow_entry	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	upstream_vlan_pd_0         	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	upstream_vlan_length       	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	upstream_vlan_pd_1         	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	upstream_vlan_bpm_number   	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_SID_CONTEXT_ENTRY_DTS;

#define RDD_US_SID_CONTEXT_ENTRY_STATE_MACHINE_READ(r, p)                               FIELD_MREAD_8((uint8_t *)p, 5, 2, r) /*defined by rdd_us_sid_state_machine enumeration*/
#define RDD_US_SID_CONTEXT_ENTRY_STATE_MACHINE_WRITE(v, p)                              FIELD_MWRITE_8((uint8_t *)p, 5, 2, v) /*defined by rdd_us_sid_state_machine enumeration*/
#define RDD_US_SID_CONTEXT_ENTRY_FRAGMENT_COUNT_READ(r, p)                              FIELD_MREAD_8((uint8_t *)p, 0, 5, r)
#define RDD_US_SID_CONTEXT_ENTRY_FRAGMENT_COUNT_WRITE(v, p)                             FIELD_MWRITE_8((uint8_t *)p, 0, 5, v)
#define RDD_US_SID_CONTEXT_ENTRY_CPU_RX_CPU_REASON_READ(r, p)                           MREAD_8((uint8_t *)p + 7, r)
#define RDD_US_SID_CONTEXT_ENTRY_CPU_RX_CPU_REASON_WRITE(v, p)                          MWRITE_8((uint8_t *)p + 7, v)
#define RDD_US_SID_CONTEXT_ENTRY_CPU_RX_PD_0_READ(r, p)                                 FIELD_MREAD_32((uint8_t *)p + 8, 14, 18, r)
#define RDD_US_SID_CONTEXT_ENTRY_CPU_RX_PD_0_WRITE(v, p)                                FIELD_MWRITE_32((uint8_t *)p + 8, 14, 18, v)
#define RDD_US_SID_CONTEXT_ENTRY_CPU_RX_LENGTH_READ(r, p)                               FIELD_MREAD_16((uint8_t *)p + 10, 0, 14, r)
#define RDD_US_SID_CONTEXT_ENTRY_CPU_RX_LENGTH_WRITE(v, p)                              FIELD_MWRITE_16((uint8_t *)p + 10, 0, 14, v)
#define RDD_US_SID_CONTEXT_ENTRY_CPU_RX_PD_1_READ(r, p)                                 FIELD_MREAD_32((uint8_t *)p + 12, 14, 18, r)
#define RDD_US_SID_CONTEXT_ENTRY_CPU_RX_PD_1_WRITE(v, p)                                FIELD_MWRITE_32((uint8_t *)p + 12, 14, 18, v)
#define RDD_US_SID_CONTEXT_ENTRY_CPU_RX_BPM_NUMBER_READ(r, p)                           FIELD_MREAD_16((uint8_t *)p + 14, 0, 14, r)
#define RDD_US_SID_CONTEXT_ENTRY_CPU_RX_BPM_NUMBER_WRITE(v, p)                          FIELD_MWRITE_16((uint8_t *)p + 14, 0, 14, v)
#define RDD_US_SID_CONTEXT_ENTRY_UPSTREAM_VLAN_VLAN_TAGS_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 16, 2, 3, r)
#define RDD_US_SID_CONTEXT_ENTRY_UPSTREAM_VLAN_VLAN_TAGS_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 16, 2, 3, v)
#define RDD_US_SID_CONTEXT_ENTRY_UPSTREAM_VLAN_L3_PROTOCOL_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p + 16, 0, 2, r)
#define RDD_US_SID_CONTEXT_ENTRY_UPSTREAM_VLAN_L3_PROTOCOL_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p + 16, 0, 2, v)
#define RDD_US_SID_CONTEXT_ENTRY_UPSTREAM_VLAN_DSCP_READ(r, p)                          MREAD_8((uint8_t *)p + 17, r)
#define RDD_US_SID_CONTEXT_ENTRY_UPSTREAM_VLAN_DSCP_WRITE(v, p)                         MWRITE_8((uint8_t *)p + 17, v)
#define RDD_US_SID_CONTEXT_ENTRY_UPSTREAM_OUTER_VLAN_READ(r, p)                         MREAD_16((uint8_t *)p + 18, r)
#define RDD_US_SID_CONTEXT_ENTRY_UPSTREAM_OUTER_VLAN_WRITE(v, p)                        MWRITE_16((uint8_t *)p + 18, v)
#define RDD_US_SID_CONTEXT_ENTRY_UPSTREAM_INGRESS_FLOW_ENTRY_READ(r, p)                 MREAD_32((uint8_t *)p + 20, r)
#define RDD_US_SID_CONTEXT_ENTRY_UPSTREAM_INGRESS_FLOW_ENTRY_WRITE(v, p)                MWRITE_32((uint8_t *)p + 20, v)
#define RDD_US_SID_CONTEXT_ENTRY_UPSTREAM_VLAN_PD_0_READ(r, p)                          FIELD_MREAD_32((uint8_t *)p + 24, 14, 18, r)
#define RDD_US_SID_CONTEXT_ENTRY_UPSTREAM_VLAN_PD_0_WRITE(v, p)                         FIELD_MWRITE_32((uint8_t *)p + 24, 14, 18, v)
#define RDD_US_SID_CONTEXT_ENTRY_UPSTREAM_VLAN_LENGTH_READ(r, p)                        FIELD_MREAD_16((uint8_t *)p + 26, 0, 14, r)
#define RDD_US_SID_CONTEXT_ENTRY_UPSTREAM_VLAN_LENGTH_WRITE(v, p)                       FIELD_MWRITE_16((uint8_t *)p + 26, 0, 14, v)
#define RDD_US_SID_CONTEXT_ENTRY_UPSTREAM_VLAN_PD_1_READ(r, p)                          FIELD_MREAD_32((uint8_t *)p + 28, 14, 18, r)
#define RDD_US_SID_CONTEXT_ENTRY_UPSTREAM_VLAN_PD_1_WRITE(v, p)                         FIELD_MWRITE_32((uint8_t *)p + 28, 14, 18, v)
#define RDD_US_SID_CONTEXT_ENTRY_UPSTREAM_VLAN_BPM_NUMBER_READ(r, p)                    FIELD_MREAD_16((uint8_t *)p + 30, 0, 14, r)
#define RDD_US_SID_CONTEXT_ENTRY_UPSTREAM_VLAN_BPM_NUMBER_WRITE(v, p)                   FIELD_MWRITE_16((uint8_t *)p + 30, 0, 14, v)
#endif
#if defined G9991

#define RDD_US_SID_CONTEXT_TABLE_SIZE     32
typedef struct
{
	RDD_US_SID_CONTEXT_ENTRY_DTS	entry[ RDD_US_SID_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_SID_CONTEXT_TABLE_DTS;

#define RDD_US_SID_CONTEXT_TABLE_PTR()	( RDD_US_SID_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_SID_CONTEXT_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_INGRESS_RATE_LIMITER_TABLE_SIZE     24
typedef struct
{
	RDD_INGRESS_RATE_LIMITER_ENTRY_DTS	entry[ RDD_US_INGRESS_RATE_LIMITER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_RATE_LIMITER_TABLE_DTS;

#define RDD_US_INGRESS_RATE_LIMITER_TABLE_PTR()	( RDD_US_INGRESS_RATE_LIMITER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_RATE_LIMITER_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_QUEUE_PROFILE_TABLE_SIZE     8
typedef struct
{
	RDD_QUEUE_PROFILE_DTS	entry[ RDD_US_QUEUE_PROFILE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_QUEUE_PROFILE_TABLE_DTS;

#define RDD_US_QUEUE_PROFILE_TABLE_PTR()	( RDD_US_QUEUE_PROFILE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_QUEUE_PROFILE_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_CPU_REASON_TO_METER_TABLE_SIZE     64
typedef struct
{
	RDD_CPU_REASON_TO_METER_ENTRY_DTS	entry[ RDD_US_CPU_REASON_TO_METER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_CPU_REASON_TO_METER_TABLE_DTS;

#define RDD_US_CPU_REASON_TO_METER_TABLE_PTR()	( RDD_US_CPU_REASON_TO_METER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_REASON_TO_METER_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_SIZE     16
typedef struct
{
	RDD_FOUR_BYTES_DTS	entry[ RDD_US_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_DTS;

#define RDD_US_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_PTR()	( RDD_US_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FAST_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_SIZE     16
typedef struct
{
	RDD_FOUR_BYTES_DTS	entry[ RDD_US_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_DTS;

#define RDD_US_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_PTR()	( RDD_US_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PICO_CPU_TX_DESCRIPTOR_ABS_DATA_PTR_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_PBITS_TO_QOS_TABLE_SIZE     8
#define RDD_US_PBITS_TO_QOS_TABLE_SIZE2    8
typedef struct
{
	RDD_US_QUEUE_ENTRY_DTS	entry[ RDD_US_PBITS_TO_QOS_TABLE_SIZE ][ RDD_US_PBITS_TO_QOS_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_PBITS_TO_QOS_TABLE_DTS;

#define RDD_US_PBITS_TO_QOS_TABLE_PTR()	( RDD_US_PBITS_TO_QOS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PBITS_TO_QOS_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_RATE_LIMITER_TABLE_SIZE     16
typedef struct
{
	RDD_RATE_LIMITER_ENTRY_DTS	entry[ RDD_US_RATE_LIMITER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_RATE_LIMITER_TABLE_DTS;

#define RDD_US_RATE_LIMITER_TABLE_PTR()	( RDD_US_RATE_LIMITER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_RATE_LIMITER_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_LAYER4_FILTERS_LOOKUP_TABLE_SIZE     16
typedef struct
{
	RDD_LAYER4_FILTERS_LOOKUP_ENTRY_DTS	entry[ RDD_US_LAYER4_FILTERS_LOOKUP_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_LAYER4_FILTERS_LOOKUP_TABLE_DTS;

#define RDD_US_LAYER4_FILTERS_LOOKUP_TABLE_PTR()	( RDD_US_LAYER4_FILTERS_LOOKUP_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_LAYER4_FILTERS_LOOKUP_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_WAN_CHANNELS_8_39_TABLE_SIZE     32
typedef struct
{
	RDD_WAN_CHANNEL_8_39_DESCRIPTOR_DTS	entry[ RDD_WAN_CHANNELS_8_39_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WAN_CHANNELS_8_39_TABLE_DTS;

#define RDD_WAN_CHANNELS_8_39_TABLE_PTR()	( RDD_WAN_CHANNELS_8_39_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + WAN_CHANNELS_8_39_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_WAN_FLOW_TABLE_SIZE     256
typedef struct
{
	RDD_US_WAN_FLOW_ENTRY_DTS	entry[ RDD_US_WAN_FLOW_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_WAN_FLOW_TABLE_DTS;

#define RDD_US_WAN_FLOW_TABLE_PTR()	( RDD_US_WAN_FLOW_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_WAN_FLOW_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_SIZE     6
#define RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_SIZE2    32
typedef struct
{
	RDD_INGRESS_FILTERS_LOOKUP_ENTRY_DTS	entry[ RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_SIZE ][ RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_DTS;

#define RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_PTR()	( RDD_US_INGRESS_FILTERS_LOOKUP_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_FILTERS_LOOKUP_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_PBITS_TO_PBITS_TABLE_SIZE     32
#define RDD_US_PBITS_TO_PBITS_TABLE_SIZE2    8
typedef struct
{
	RDD_PBITS_TO_PBITS_ENTRY_DTS	entry[ RDD_US_PBITS_TO_PBITS_TABLE_SIZE ][ RDD_US_PBITS_TO_PBITS_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_PBITS_TO_PBITS_TABLE_DTS;

#define RDD_US_PBITS_TO_PBITS_TABLE_PTR()	( RDD_US_PBITS_TO_PBITS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PBITS_TO_PBITS_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_WAN_CHANNELS_0_7_TABLE_SIZE     8
typedef struct
{
	RDD_WAN_CHANNEL_0_7_DESCRIPTOR_DTS	entry[ RDD_WAN_CHANNELS_0_7_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WAN_CHANNELS_0_7_TABLE_DTS;

#define RDD_WAN_CHANNELS_0_7_TABLE_PTR()	( RDD_WAN_CHANNELS_0_7_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + WAN_CHANNELS_0_7_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_TRAFFIC_CLASS_TO_QUEUE_TABLE_SIZE     8
#define RDD_US_TRAFFIC_CLASS_TO_QUEUE_TABLE_SIZE2    8
typedef struct
{
	RDD_US_QUEUE_ENTRY_DTS	entry[ RDD_US_TRAFFIC_CLASS_TO_QUEUE_TABLE_SIZE ][ RDD_US_TRAFFIC_CLASS_TO_QUEUE_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_TRAFFIC_CLASS_TO_QUEUE_TABLE_DTS;

#define RDD_US_TRAFFIC_CLASS_TO_QUEUE_TABLE_PTR()	( RDD_US_TRAFFIC_CLASS_TO_QUEUE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_TRAFFIC_CLASS_TO_QUEUE_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_PBITS_PARAMETER_TABLE_SIZE     128
typedef struct
{
	RDD_PBITS_PARAMETER_ENTRY_DTS	entry[ RDD_US_PBITS_PARAMETER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_PBITS_PARAMETER_TABLE_DTS;

#define RDD_US_PBITS_PARAMETER_TABLE_PTR()	( RDD_US_PBITS_PARAMETER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PBITS_PARAMETER_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_RUNNER_FLOW_HEADER_BUFFER_SIZE     3
typedef struct
{
	RDD_RUNNER_FLOW_HEADER_BUFFER_DTS	entry[ RDD_US_RUNNER_FLOW_HEADER_BUFFER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_RUNNER_FLOW_HEADER_BUFFER_DTS;

#define RDD_US_RUNNER_FLOW_HEADER_BUFFER_PTR()	( RDD_US_RUNNER_FLOW_HEADER_BUFFER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_RUNNER_FLOW_HEADER_BUFFER_ADDRESS )

#endif
#define RDD_US_CPU_RX_METER_TABLE_PTR()	( RDD_US_CPU_RX_METER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_RX_METER_TABLE_ADDRESS )

#if defined G9991

#define RDD_US_VLAN_COMMANDS_TABLE_SIZE     64
typedef struct
{
	RDD_VLAN_COMMAND_ENRTY_DTS	entry[ RDD_US_VLAN_COMMANDS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_VLAN_COMMANDS_TABLE_DTS;

#define RDD_US_VLAN_COMMANDS_TABLE_PTR()	( RDD_US_VLAN_COMMANDS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_VLAN_COMMANDS_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_VLAN_OPTIMIZATION_TABLE_SIZE     128
typedef struct
{
	RDD_VLAN_OPTIMIZATION_ENTRY_DTS	entry[ RDD_US_VLAN_OPTIMIZATION_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_VLAN_OPTIMIZATION_TABLE_DTS;

#define RDD_US_VLAN_OPTIMIZATION_TABLE_PTR()	( RDD_US_VLAN_OPTIMIZATION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_VLAN_OPTIMIZATION_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_GPON_ABSOLUTE_TX_BBH_COUNTER_SIZE     40
typedef struct
{
	RDD_GPON_ABSOLUTE_TX_COUNTER_DTS	entry[ RDD_GPON_ABSOLUTE_TX_BBH_COUNTER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GPON_ABSOLUTE_TX_BBH_COUNTER_DTS;

#define RDD_GPON_ABSOLUTE_TX_BBH_COUNTER_PTR()	( RDD_GPON_ABSOLUTE_TX_BBH_COUNTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + GPON_ABSOLUTE_TX_BBH_COUNTER_ADDRESS )

#endif
#if defined G9991

#define RDD_US_PBITS_TO_WAN_FLOW_TABLE_SIZE     8
#define RDD_US_PBITS_TO_WAN_FLOW_TABLE_SIZE2    8
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_US_PBITS_TO_WAN_FLOW_TABLE_SIZE ][ RDD_US_PBITS_TO_WAN_FLOW_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_PBITS_TO_WAN_FLOW_TABLE_DTS;

#define RDD_US_PBITS_TO_WAN_FLOW_TABLE_PTR()	( RDD_US_PBITS_TO_WAN_FLOW_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PBITS_TO_WAN_FLOW_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_LAN_VID_TABLE_SIZE     128
typedef struct
{
	RDD_VID_ENTRY_DTS	entry[ RDD_US_LAN_VID_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_LAN_VID_TABLE_DTS;

#define RDD_US_LAN_VID_TABLE_PTR()	( RDD_US_LAN_VID_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_LAN_VID_TABLE_ADDRESS )

#endif
#define RDD_US_POLICER_TABLE_PTR()	( RDD_US_POLICER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_POLICER_TABLE_ADDRESS )

#if defined G9991

#define RDD_US_INGRESS_HANDLER_SKB_DATA_POINTER_SIZE     32
typedef struct
{
	RDD_FOUR_BYTES_DTS	entry[ RDD_US_INGRESS_HANDLER_SKB_DATA_POINTER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_HANDLER_SKB_DATA_POINTER_DTS;

#define RDD_US_INGRESS_HANDLER_SKB_DATA_POINTER_PTR()	( RDD_US_INGRESS_HANDLER_SKB_DATA_POINTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_HANDLER_SKB_DATA_POINTER_ADDRESS )

#endif
#if defined G9991

#define RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE     16
typedef struct
{
	RDD_INGRESS_CLASSIFICATION_RULE_CFG_ENTRY_DTS	entry[ RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_PTR()	( RDD_US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_RULE_CFG_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_CPU_TX_BBH_DESCRIPTORS_SIZE     32
typedef struct
{
	RDD_CPU_TX_BBH_DESCRIPTORS_ENTRY_DTS	entry[ RDD_US_CPU_TX_BBH_DESCRIPTORS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_CPU_TX_BBH_DESCRIPTORS_DTS;

#define RDD_US_CPU_TX_BBH_DESCRIPTORS_PTR()	( RDD_US_CPU_TX_BBH_DESCRIPTORS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_TX_BBH_DESCRIPTORS_ADDRESS )

#endif
#if defined G9991

#define RDD_US_FORWARDING_MATRIX_TABLE_SIZE     9
#define RDD_US_FORWARDING_MATRIX_TABLE_SIZE2    16
typedef struct
{
	RDD_FORWARDING_MATRIX_ENTRY_DTS	entry[ RDD_US_FORWARDING_MATRIX_TABLE_SIZE ][ RDD_US_FORWARDING_MATRIX_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_FORWARDING_MATRIX_TABLE_DTS;

#define RDD_US_FORWARDING_MATRIX_TABLE_PTR()	( RDD_US_FORWARDING_MATRIX_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FORWARDING_MATRIX_TABLE_ADDRESS )

#endif
#define RDD_US_RATE_CONTROL_BUDGET_ALLOCATOR_TABLE_PTR()	( RDD_US_RATE_CONTROL_BUDGET_ALLOCATOR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_RATE_CONTROL_BUDGET_ALLOCATOR_TABLE_ADDRESS )

#define RDD_US_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_PTR()	( RDD_US_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_MAIN_TIMER_TASK_DESCRIPTOR_TABLE_ADDRESS )

#if defined G9991

#define RDD_US_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_SIZE     32
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_US_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_PTR()	( RDD_US_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_KEY_PRIMITIVE_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_ETH0_RX_DESCRIPTORS_SIZE     32
typedef struct
{
	RDD_ETH_RX_DESCRIPTORS_DTS	entry[ RDD_ETH0_RX_DESCRIPTORS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH0_RX_DESCRIPTORS_DTS;

#define RDD_ETH0_RX_DESCRIPTORS_PTR()	( RDD_ETH0_RX_DESCRIPTORS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + ETH0_RX_DESCRIPTORS_ADDRESS )

#endif
#if defined G9991

#define RDD_ETH1_RX_DESCRIPTORS_SIZE     32
typedef struct
{
	RDD_ETH_RX_DESCRIPTORS_DTS	entry[ RDD_ETH1_RX_DESCRIPTORS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH1_RX_DESCRIPTORS_DTS;

#define RDD_ETH1_RX_DESCRIPTORS_PTR()	( RDD_ETH1_RX_DESCRIPTORS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + ETH1_RX_DESCRIPTORS_ADDRESS )

#endif
#if defined G9991

#define RDD_US_GPON_RX_DIRECT_DESCRIPTORS_SIZE     32
typedef struct
{
	RDD_BBH_RX_DESCRIPTOR_DTS	entry[ RDD_US_GPON_RX_DIRECT_DESCRIPTORS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_GPON_RX_DIRECT_DESCRIPTORS_DTS;

#define RDD_US_GPON_RX_DIRECT_DESCRIPTORS_PTR()	( RDD_US_GPON_RX_DIRECT_DESCRIPTORS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_GPON_RX_DIRECT_DESCRIPTORS_ADDRESS )

#endif
#if defined G9991

#define RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_SIZE     6
#define RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_SIZE2    16
typedef struct
{
	RDD_INGRESS_FILTERS_PARAMETER_ENTRY_DTS	entry[ RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_SIZE ][ RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_DTS;

#define RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_PTR()	( RDD_US_INGRESS_FILTERS_PARAMETER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_FILTERS_PARAMETER_TABLE_ADDRESS )

#endif
#define RDD_US_PICO_TIMER_TASK_DESCRIPTOR_TABLE_PTR()	( RDD_US_PICO_TIMER_TASK_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PICO_TIMER_TASK_DESCRIPTOR_TABLE_ADDRESS )

#if defined G9991

#define RDD_US_PBITS_PRIMITIVE_TABLE_SIZE     32
typedef struct
{
	RDD_PBITS_PRIMITIVE_ENTRY_DTS	entry[ RDD_US_PBITS_PRIMITIVE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_PBITS_PRIMITIVE_TABLE_DTS;

#define RDD_US_PBITS_PRIMITIVE_TABLE_PTR()	( RDD_US_PBITS_PRIMITIVE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PBITS_PRIMITIVE_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_FLOW_BASED_ACTION_PTR_TABLE_SIZE     32
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_US_FLOW_BASED_ACTION_PTR_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_FLOW_BASED_ACTION_PTR_TABLE_DTS;

#define RDD_US_FLOW_BASED_ACTION_PTR_TABLE_PTR()	( RDD_US_FLOW_BASED_ACTION_PTR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FLOW_BASED_ACTION_PTR_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_CPU_RX_PICO_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_US_CPU_RX_PICO_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_CPU_RX_PICO_INGRESS_QUEUE_DTS;

#define RDD_US_CPU_RX_PICO_INGRESS_QUEUE_PTR()	( RDD_US_CPU_RX_PICO_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_RX_PICO_INGRESS_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_INGRESS_CLASSIFICATION_COUNTERS_BUFFER_SIZE     30
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_US_INGRESS_CLASSIFICATION_COUNTERS_BUFFER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_COUNTERS_BUFFER_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_COUNTERS_BUFFER_PTR()	( RDD_US_INGRESS_CLASSIFICATION_COUNTERS_BUFFER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_COUNTERS_BUFFER_ADDRESS )

#endif
#if defined G9991

#define RDD_US_ROUTER_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_US_ROUTER_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_ROUTER_INGRESS_QUEUE_DTS;

#define RDD_US_ROUTER_INGRESS_QUEUE_PTR()	( RDD_US_ROUTER_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_ROUTER_INGRESS_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_ACK_PACKETS_QUEUE_INDEX_TABLE_SIZE     40
typedef struct
{
	RDD_US_QUEUE_ENTRY_DTS	entry[ RDD_US_ACK_PACKETS_QUEUE_INDEX_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_ACK_PACKETS_QUEUE_INDEX_TABLE_DTS;

#define RDD_US_ACK_PACKETS_QUEUE_INDEX_TABLE_PTR()	( RDD_US_ACK_PACKETS_QUEUE_INDEX_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_ACK_PACKETS_QUEUE_INDEX_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_CPU_RX_FAST_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_US_CPU_RX_FAST_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_CPU_RX_FAST_INGRESS_QUEUE_DTS;

#define RDD_US_CPU_RX_FAST_INGRESS_QUEUE_PTR()	( RDD_US_CPU_RX_FAST_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_RX_FAST_INGRESS_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_DEBUG_BUFFER_SIZE     32
typedef struct
{
	RDD_DEBUG_BUFFER_ENTRY_DTS	entry[ RDD_US_DEBUG_BUFFER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_DEBUG_BUFFER_DTS;

#define RDD_US_DEBUG_BUFFER_PTR()	( RDD_US_DEBUG_BUFFER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_DEBUG_BUFFER_ADDRESS )

#endif
#if defined G9991

#define RDD_US_TPID_OVERWRITE_TABLE_SIZE     8
typedef struct
{
	RDD_TPID_OVERWRITE_ENTRY_DTS	entry[ RDD_US_TPID_OVERWRITE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_TPID_OVERWRITE_TABLE_DTS;

#define RDD_US_TPID_OVERWRITE_TABLE_PTR()	( RDD_US_TPID_OVERWRITE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_TPID_OVERWRITE_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_ETH2_RX_DESCRIPTORS_SIZE     32
typedef struct
{
	RDD_ETH_RX_DESCRIPTORS_DTS	entry[ RDD_ETH2_RX_DESCRIPTORS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH2_RX_DESCRIPTORS_DTS;

#define RDD_ETH2_RX_DESCRIPTORS_PTR()	( RDD_ETH2_RX_DESCRIPTORS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + ETH2_RX_DESCRIPTORS_ADDRESS )

#endif
#if defined G9991

#define RDD_LOCAL_SWITCHING_MULTICAST_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_LOCAL_SWITCHING_MULTICAST_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LOCAL_SWITCHING_MULTICAST_INGRESS_QUEUE_DTS;

#define RDD_LOCAL_SWITCHING_MULTICAST_INGRESS_QUEUE_PTR()	( RDD_LOCAL_SWITCHING_MULTICAST_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + LOCAL_SWITCHING_MULTICAST_INGRESS_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_VLAN_PRIMITIVE_TABLE_SIZE     32
typedef struct
{
	RDD_VLAN_PRIMITIVE_ENTRY_DTS	entry[ RDD_US_VLAN_PRIMITIVE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_VLAN_PRIMITIVE_TABLE_DTS;

#define RDD_US_VLAN_PRIMITIVE_TABLE_PTR()	( RDD_US_VLAN_PRIMITIVE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_VLAN_PRIMITIVE_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_VLAN_ACTION_BRIDGE_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_VLAN_ACTION_BRIDGE_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_VLAN_ACTION_BRIDGE_INGRESS_QUEUE_DTS;

#define RDD_VLAN_ACTION_BRIDGE_INGRESS_QUEUE_PTR()	( RDD_VLAN_ACTION_BRIDGE_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + VLAN_ACTION_BRIDGE_INGRESS_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_SIZE     4
typedef struct
{
	RDD_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_ENTRY_DTS	entry[ RDD_US_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_PTR()	( RDD_US_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_GENERIC_RULE_CFG_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_FAST_RUNNER_GLOBAL_REGISTERS_INIT_SIZE     8
typedef struct
{
	RDD_FAST_RUNNER_GLOBAL_REGISTERS_INIT_ENTRY_DTS	entry[ RDD_US_FAST_RUNNER_GLOBAL_REGISTERS_INIT_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_FAST_RUNNER_GLOBAL_REGISTERS_INIT_DTS;

#define RDD_US_FAST_RUNNER_GLOBAL_REGISTERS_INIT_PTR()	( RDD_US_FAST_RUNNER_GLOBAL_REGISTERS_INIT_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FAST_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS )

#endif
#if defined G9991

#define RDD_ETH3_RX_DESCRIPTORS_SIZE     32
typedef struct
{
	RDD_ETH_RX_DESCRIPTORS_DTS	entry[ RDD_ETH3_RX_DESCRIPTORS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH3_RX_DESCRIPTORS_DTS;

#define RDD_ETH3_RX_DESCRIPTORS_PTR()	( RDD_ETH3_RX_DESCRIPTORS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + ETH3_RX_DESCRIPTORS_ADDRESS )

#endif
#if defined G9991

#define RDD_UPSTREAM_FLOODING_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_UPSTREAM_FLOODING_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_UPSTREAM_FLOODING_INGRESS_QUEUE_DTS;

#define RDD_UPSTREAM_FLOODING_INGRESS_QUEUE_PTR()	( RDD_UPSTREAM_FLOODING_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + UPSTREAM_FLOODING_INGRESS_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_WAN_INTERWORKING_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_WAN_INTERWORKING_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WAN_INTERWORKING_INGRESS_QUEUE_DTS;

#define RDD_WAN_INTERWORKING_INGRESS_QUEUE_PTR()	( RDD_WAN_INTERWORKING_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + WAN_INTERWORKING_INGRESS_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_TABLE_SIZE     48
typedef struct
{
	RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_ENTRY_DTS	entry[ RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_TABLE_DTS;

#define RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_TABLE_PTR()	( RDD_US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_OVERALL_RATE_LIMITER_WAN_CHANNEL_PTR_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_SIZE     3
#define RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_SIZE2    30
typedef struct
{
	RDD_CPU_REASON_TO_METER_ENTRY_DTS	entry[ RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_SIZE ][ RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_DTS;

#define RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_PTR()	( RDD_CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + CPU_REASON_AND_SRC_BRIDGE_PORT_TO_METER_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_PICO_RUNNER_GLOBAL_REGISTERS_INIT_SIZE     8
typedef struct
{
	RDD_PICO_RUNNER_GLOBAL_REGISTERS_INIT_ENTRY_DTS	entry[ RDD_US_PICO_RUNNER_GLOBAL_REGISTERS_INIT_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_PICO_RUNNER_GLOBAL_REGISTERS_INIT_DTS;

#define RDD_US_PICO_RUNNER_GLOBAL_REGISTERS_INIT_PTR()	( RDD_US_PICO_RUNNER_GLOBAL_REGISTERS_INIT_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PICO_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS )

#endif
#if defined G9991

#define RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_SIZE     6
typedef struct
{
	RDD_INGRESS_FILTERS_CONFIGURATION_ENTRY_DTS	entry[ RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS;

#define RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_PTR()	( RDD_US_INGRESS_FILTERS_CONFIGURATION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_FILTERS_CONFIGURATION_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_INGRESS_FILTERS_PROFILE_TABLE_SIZE     32
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_INGRESS_FILTERS_PROFILE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INGRESS_FILTERS_PROFILE_TABLE_DTS;

#define RDD_INGRESS_FILTERS_PROFILE_TABLE_PTR()	( RDD_INGRESS_FILTERS_PROFILE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + INGRESS_FILTERS_PROFILE_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_RUNNER_FLOW_HEADER_DESCRIPTOR_SIZE     3
typedef struct
{
	RDD_RUNNER_FLOW_HEADER_DESCRIPTOR_DTS	entry[ RDD_US_RUNNER_FLOW_HEADER_DESCRIPTOR_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_RUNNER_FLOW_HEADER_DESCRIPTOR_DTS;

#define RDD_US_RUNNER_FLOW_HEADER_DESCRIPTOR_PTR()	( RDD_US_RUNNER_FLOW_HEADER_DESCRIPTOR_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_RUNNER_FLOW_HEADER_DESCRIPTOR_ADDRESS )

#endif
#if defined G9991

#define RDD_LAN_INGRESS_FIFO_DESCRIPTOR_TABLE_SIZE     5
typedef struct
{
	RDD_LAN_INGRESS_FIFO_DESCRIPTOR_ENTRY_DTS	entry[ RDD_LAN_INGRESS_FIFO_DESCRIPTOR_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LAN_INGRESS_FIFO_DESCRIPTOR_TABLE_DTS;

#define RDD_LAN_INGRESS_FIFO_DESCRIPTOR_TABLE_PTR()	( RDD_LAN_INGRESS_FIFO_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + LAN_INGRESS_FIFO_DESCRIPTOR_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_GPON_ABSOLUTE_TX_FIRMWARE_COUNTER_SIZE     40
typedef struct
{
	RDD_GPON_ABSOLUTE_TX_COUNTER_DTS	entry[ RDD_GPON_ABSOLUTE_TX_FIRMWARE_COUNTER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GPON_ABSOLUTE_TX_FIRMWARE_COUNTER_DTS;

#define RDD_GPON_ABSOLUTE_TX_FIRMWARE_COUNTER_PTR()	( RDD_GPON_ABSOLUTE_TX_FIRMWARE_COUNTER_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + GPON_ABSOLUTE_TX_FIRMWARE_COUNTER_ADDRESS )

#endif
#if defined G9991

#define RDD_US_TIMER_SCHEDULER_PRIMITIVE_TABLE_SIZE     8
typedef struct
{
	RDD_TIMER_SCHEDULER_PRIMITIVE_ENTRY_DTS	entry[ RDD_US_TIMER_SCHEDULER_PRIMITIVE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_TIMER_SCHEDULER_PRIMITIVE_TABLE_DTS;

#define RDD_US_TIMER_SCHEDULER_PRIMITIVE_TABLE_PTR()	( RDD_US_TIMER_SCHEDULER_PRIMITIVE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_TIMER_SCHEDULER_PRIMITIVE_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_LAYER4_FILTERS_CONTEXT_TABLE_SIZE     16
typedef struct
{
	RDD_LAYER4_FILTERS_CONTEXT_ENTRY_DTS	entry[ RDD_US_LAYER4_FILTERS_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_LAYER4_FILTERS_CONTEXT_TABLE_DTS;

#define RDD_US_LAYER4_FILTERS_CONTEXT_TABLE_PTR()	( RDD_US_LAYER4_FILTERS_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_LAYER4_FILTERS_CONTEXT_TABLE_ADDRESS )

#endif
#define RDD_US_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_PTR()	( RDD_US_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FAST_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_ADDRESS )

#define RDD_US_PICO_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_PTR()	( RDD_US_PICO_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_PICO_FREE_SKB_INDEXES_FIFO_LOCAL_TABLE_ADDRESS )

#if defined G9991

#define RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_TABLE_SIZE     32
typedef struct
{
	RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_ENTRY_DTS	entry[ RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_TABLE_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_TABLE_PTR()	( RDD_US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_INGRESS_CLASSIFICATION_DEFAULT_FLOWS_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_ETH4_RX_DESCRIPTORS_SIZE     32
typedef struct
{
	RDD_ETH_RX_DESCRIPTORS_DTS	entry[ RDD_ETH4_RX_DESCRIPTORS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ETH4_RX_DESCRIPTORS_DTS;

#define RDD_ETH4_RX_DESCRIPTORS_PTR()	( RDD_ETH4_RX_DESCRIPTORS_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + ETH4_RX_DESCRIPTORS_ADDRESS )

#endif
#if defined G9991

#define RDD_LOCAL_SWITCHING_MODE_TABLE_SIZE     6
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_LOCAL_SWITCHING_MODE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LOCAL_SWITCHING_MODE_TABLE_DTS;

#define RDD_LOCAL_SWITCHING_MODE_TABLE_PTR()	( RDD_LOCAL_SWITCHING_MODE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + LOCAL_SWITCHING_MODE_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_MULTICAST_VECTOR_TO_PORT_TABLE_SIZE     8
typedef struct
{
	RDD_MULTICAST_VECTOR_TO_PORT_ENTRY_DTS	entry[ RDD_US_MULTICAST_VECTOR_TO_PORT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_MULTICAST_VECTOR_TO_PORT_TABLE_DTS;

#define RDD_US_MULTICAST_VECTOR_TO_PORT_TABLE_PTR()	( RDD_US_MULTICAST_VECTOR_TO_PORT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_MULTICAST_VECTOR_TO_PORT_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_BROADCOM_SWITCH_PORT_TO_BRIDGE_PORT_MAPPING_TABLE_SIZE     8
typedef struct
{
	RDD_BROADCOM_SWITCH_PORT_MAPPING_DTS	entry[ RDD_BROADCOM_SWITCH_PORT_TO_BRIDGE_PORT_MAPPING_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BROADCOM_SWITCH_PORT_TO_BRIDGE_PORT_MAPPING_TABLE_DTS;

#define RDD_BROADCOM_SWITCH_PORT_TO_BRIDGE_PORT_MAPPING_TABLE_PTR()	( RDD_BROADCOM_SWITCH_PORT_TO_BRIDGE_PORT_MAPPING_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + BROADCOM_SWITCH_PORT_TO_BRIDGE_PORT_MAPPING_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_INGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_SIZE     5
typedef struct
{
	RDD_VLAN_SWITCHING_ISOLATION_CONFIG_ENTRY_DTS	entry[ RDD_INGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_DTS;

#define RDD_INGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_PTR()	( RDD_INGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + INGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_ADDRESS )

#endif
/* COMMON_A */
#if defined G9991

#define RDD_MAC_TABLE_SIZE     1024
typedef struct
{
	RDD_MAC_ENTRY_DTS	entry[ RDD_MAC_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MAC_TABLE_DTS;

#define RDD_MAC_TABLE_PTR()	( RDD_MAC_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_MAC_CONTEXT_TABLE_SIZE     1024
typedef struct
{
	RDD_MAC_CONTEXT_ENTRY_DTS	entry[ RDD_MAC_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MAC_CONTEXT_TABLE_DTS;

#define RDD_MAC_CONTEXT_TABLE_PTR()	( RDD_MAC_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_CONTEXT_TABLE_ADDRESS )

#endif
#if defined G9991

typedef struct
{
	uint32_t	port_vector_info  	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = port_vector_info, size = 32 bits
	uint32_t	replication_number	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_port_vector	:24	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_CONTEXT_ENTRY_DTS;

#define RDD_IPTV_CONTEXT_ENTRY_PORT_VECTOR_INFO_READ(r, p)                   MREAD_32((uint8_t *)p, r)
#define RDD_IPTV_CONTEXT_ENTRY_PORT_VECTOR_INFO_WRITE(v, p)                  MWRITE_32((uint8_t *)p, v)
#define RDD_IPTV_CONTEXT_ENTRY_PORT_VECTOR_INFO_L_READ( wv )                 FIELD_GET( wv, 0, 32 )
#define RDD_IPTV_CONTEXT_ENTRY_PORT_VECTOR_INFO_L_WRITE( v, wv )             FIELD_SET( v, 0, 32, wv )
#define RDD_IPTV_CONTEXT_ENTRY_REPLICATION_NUMBER_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_IPTV_CONTEXT_ENTRY_REPLICATION_NUMBER_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
#define RDD_IPTV_CONTEXT_ENTRY_REPLICATION_NUMBER_L_READ( wv )               FIELD_GET( wv, 24, 8 )
#define RDD_IPTV_CONTEXT_ENTRY_REPLICATION_NUMBER_L_WRITE( v, wv )           FIELD_SET( v, 24, 8, wv )
#define RDD_IPTV_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_READ(r, p)                 FIELD_MREAD_32((uint8_t *)p + 0, 0, 24, r)
#define RDD_IPTV_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_WRITE(v, p)                FIELD_MWRITE_32((uint8_t *)p + 0, 0, 24, v)
#define RDD_IPTV_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_L_READ( wv )               FIELD_GET( wv, 0, 24 )
#define RDD_IPTV_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_L_WRITE( v, wv )           FIELD_SET( v, 0, 24, wv )
#endif
#if defined G9991

#define RDD_IPTV_CONTEXT_TABLE_SIZE     256
typedef struct
{
	RDD_IPTV_CONTEXT_ENTRY_DTS	entry[ RDD_IPTV_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_CONTEXT_TABLE_DTS;

#define RDD_IPTV_CONTEXT_TABLE_PTR()	( RDD_IPTV_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + IPTV_CONTEXT_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_IPTV_LOOKUP_TABLE_CAM_SIZE     32
typedef struct
{
	RDD_MAC_ENTRY_DTS	entry[ RDD_IPTV_LOOKUP_TABLE_CAM_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_LOOKUP_TABLE_CAM_DTS;

#define RDD_IPTV_LOOKUP_TABLE_CAM_PTR()	( RDD_IPTV_LOOKUP_TABLE_CAM_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + IPTV_LOOKUP_TABLE_CAM_ADDRESS )

#endif
#if defined G9991

#define RDD_MAC_TABLE_CAM_SIZE     32
typedef struct
{
	RDD_MAC_ENTRY_DTS	entry[ RDD_MAC_TABLE_CAM_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MAC_TABLE_CAM_DTS;

#define RDD_MAC_TABLE_CAM_PTR()	( RDD_MAC_TABLE_CAM_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_TABLE_CAM_ADDRESS )

#endif
#if defined G9991

#define RDD_IPTV_CONTEXT_TABLE_CAM_SIZE     32
typedef struct
{
	RDD_IPTV_CONTEXT_ENTRY_DTS	entry[ RDD_IPTV_CONTEXT_TABLE_CAM_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_CONTEXT_TABLE_CAM_DTS;

#define RDD_IPTV_CONTEXT_TABLE_CAM_PTR()	( RDD_IPTV_CONTEXT_TABLE_CAM_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + IPTV_CONTEXT_TABLE_CAM_ADDRESS )

#endif
#if defined G9991

#define RDD_IPTV_L3_SRC_IP_LOOKUP_TABLE_SIZE     32
typedef struct
{
	RDD_IPTV_L3_SRC_IP_LOOKUP_ENTRY_DTS	entry[ RDD_IPTV_L3_SRC_IP_LOOKUP_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_L3_SRC_IP_LOOKUP_TABLE_DTS;

#define RDD_IPTV_L3_SRC_IP_LOOKUP_TABLE_PTR()	( RDD_IPTV_L3_SRC_IP_LOOKUP_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + IPTV_L3_SRC_IP_LOOKUP_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_LAN_VID_CONTEXT_TABLE_SIZE     128
typedef struct
{
	RDD_LAN_VID_CONTEXT_ENTRY_DTS	entry[ RDD_LAN_VID_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LAN_VID_CONTEXT_TABLE_DTS;

#define RDD_LAN_VID_CONTEXT_TABLE_PTR()	( RDD_LAN_VID_CONTEXT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + LAN_VID_CONTEXT_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_RING_PACKET_DESCRIPTORS_CACHE_SIZE     12
typedef struct
{
	RDD_CPU_RX_DESCRIPTOR_DTS	entry[ RDD_DS_RING_PACKET_DESCRIPTORS_CACHE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_RING_PACKET_DESCRIPTORS_CACHE_DTS;

#define RDD_DS_RING_PACKET_DESCRIPTORS_CACHE_PTR()	( RDD_DS_RING_PACKET_DESCRIPTORS_CACHE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DS_RING_PACKET_DESCRIPTORS_CACHE_ADDRESS )

#endif
#if defined G9991

#define RDD_MAC_CONTEXT_TABLE_CAM_SIZE     32
typedef struct
{
	RDD_MAC_CONTEXT_ENTRY_DTS	entry[ RDD_MAC_CONTEXT_TABLE_CAM_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MAC_CONTEXT_TABLE_CAM_DTS;

#define RDD_MAC_CONTEXT_TABLE_CAM_PTR()	( RDD_MAC_CONTEXT_TABLE_CAM_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_CONTEXT_TABLE_CAM_ADDRESS )

#endif
#if defined G9991

#define RDD_MAC_EXTENSION_TABLE_SIZE     1024
typedef struct
{
	RDD_MAC_EXTENSION_ENTRY_DTS	entry[ RDD_MAC_EXTENSION_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MAC_EXTENSION_TABLE_DTS;

#define RDD_MAC_EXTENSION_TABLE_PTR()	( RDD_MAC_EXTENSION_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_EXTENSION_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_INTERRUPT_COALESCING_CONFIG_TABLE_SIZE     12
typedef struct
{
	RDD_INTERRUPT_COALESCING_CONFIG_DTS	entry[ RDD_INTERRUPT_COALESCING_CONFIG_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INTERRUPT_COALESCING_CONFIG_TABLE_DTS;

#define RDD_INTERRUPT_COALESCING_CONFIG_TABLE_PTR()	( RDD_INTERRUPT_COALESCING_CONFIG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + INTERRUPT_COALESCING_CONFIG_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_WAN_VID_TABLE_SIZE     4
typedef struct
{
	RDD_VID_ENTRY_DTS	entry[ RDD_WAN_VID_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WAN_VID_TABLE_DTS;

#define RDD_WAN_VID_TABLE_PTR()	( RDD_WAN_VID_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + WAN_VID_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_GPON_SKB_ENQUEUED_INDEXES_FIFO_SIZE     32
typedef struct
{
	RDD_GPON_SKB_ENQUEUED_INDEXES_FIFO_ENTRY_DTS	entry[ RDD_GPON_SKB_ENQUEUED_INDEXES_FIFO_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GPON_SKB_ENQUEUED_INDEXES_FIFO_DTS;

#define RDD_GPON_SKB_ENQUEUED_INDEXES_FIFO_PTR()	( RDD_GPON_SKB_ENQUEUED_INDEXES_FIFO_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + GPON_SKB_ENQUEUED_INDEXES_FIFO_ADDRESS )

#endif
#if defined G9991

#define RDD_G9991_DOWNSTREAM_MULTICAST_FRAGMENTATION_INGRESS_QUEUE_SIZE     32
typedef struct
{
	RDD_EIGHT_BYTES_DTS	entry[ RDD_G9991_DOWNSTREAM_MULTICAST_FRAGMENTATION_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_G9991_DOWNSTREAM_MULTICAST_FRAGMENTATION_INGRESS_QUEUE_DTS;

#define RDD_G9991_DOWNSTREAM_MULTICAST_FRAGMENTATION_INGRESS_QUEUE_PTR()	( RDD_G9991_DOWNSTREAM_MULTICAST_FRAGMENTATION_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + G9991_DOWNSTREAM_MULTICAST_FRAGMENTATION_INGRESS_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_PBITS_TO_QOS_TABLE_SIZE     6
#define RDD_DS_PBITS_TO_QOS_TABLE_SIZE2    8
typedef struct
{
	RDD_PBITS_TO_QOS_ENTRY_DTS	entry[ RDD_DS_PBITS_TO_QOS_TABLE_SIZE ][ RDD_DS_PBITS_TO_QOS_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_PBITS_TO_QOS_TABLE_DTS;

#define RDD_DS_PBITS_TO_QOS_TABLE_PTR()	( RDD_DS_PBITS_TO_QOS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DS_PBITS_TO_QOS_TABLE_ADDRESS )

#endif
#define RDD_DS_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_PTR()	( RDD_DS_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DS_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_ADDRESS )

#if defined G9991

#define RDD_GLOBAL_DSCP_TO_PBITS_TABLE_SIZE2    64
typedef struct
{
	RDD_DSCP_TO_PBITS_ENTRY_DTS	entry[ RDD_GLOBAL_DSCP_TO_PBITS_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GLOBAL_DSCP_TO_PBITS_TABLE_DTS;

#define RDD_GLOBAL_DSCP_TO_PBITS_TABLE_PTR()	( RDD_GLOBAL_DSCP_TO_PBITS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + GLOBAL_DSCP_TO_PBITS_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_GLOBAL_DSCP_TO_PBITS_DEI_TABLE_SIZE2    64
typedef struct
{
	RDD_DSCP_TO_PBITS_DEI_ENTRY_DTS	entry[ RDD_GLOBAL_DSCP_TO_PBITS_DEI_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GLOBAL_DSCP_TO_PBITS_DEI_TABLE_DTS;

#define RDD_GLOBAL_DSCP_TO_PBITS_DEI_TABLE_PTR()	( RDD_GLOBAL_DSCP_TO_PBITS_DEI_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + GLOBAL_DSCP_TO_PBITS_DEI_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_CPU_RX_RUNNER_A_SCRATCHPAD_SIZE     8
typedef struct
{
	RDD_RUNNER_SCRATCHPAD_DTS	entry[ RDD_CPU_RX_RUNNER_A_SCRATCHPAD_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_RUNNER_A_SCRATCHPAD_DTS;

#define RDD_CPU_RX_RUNNER_A_SCRATCHPAD_PTR()	( RDD_CPU_RX_RUNNER_A_SCRATCHPAD_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + CPU_RX_RUNNER_A_SCRATCHPAD_ADDRESS )

#endif
#if defined G9991

#define RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_SIZE     128
typedef struct
{
	RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_DTS	entry[ RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_DTS;

#define RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_PTR()	( RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + DS_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_G9991_DDR_QUEUE_ADDRESS_TABLE_SIZE     120
typedef struct
{
	RDD_DDR_QUEUE_ADDRESS_ENTRY_DTS	entry[ RDD_G9991_DDR_QUEUE_ADDRESS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_G9991_DDR_QUEUE_ADDRESS_TABLE_DTS;

#define RDD_G9991_DDR_QUEUE_ADDRESS_TABLE_PTR()	( RDD_G9991_DDR_QUEUE_ADDRESS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + G9991_DDR_QUEUE_ADDRESS_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_CONNECTION_BUFFER_TABLE_SIZE     5
#define RDD_CONNECTION_BUFFER_TABLE_SIZE2    4
typedef struct
{
	RDD_CONNECTION_ENTRY_DTS	entry[ RDD_CONNECTION_BUFFER_TABLE_SIZE ][ RDD_CONNECTION_BUFFER_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CONNECTION_BUFFER_TABLE_DTS;

#define RDD_CONNECTION_BUFFER_TABLE_PTR()	( RDD_CONNECTION_BUFFER_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + CONNECTION_BUFFER_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_RING_DESCRIPTORS_TABLE_SIZE     12
typedef struct
{
	RDD_RING_DESCRIPTOR_DTS	entry[ RDD_RING_DESCRIPTORS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RING_DESCRIPTORS_TABLE_DTS;

#define RDD_RING_DESCRIPTORS_TABLE_PTR()	( RDD_RING_DESCRIPTORS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + RING_DESCRIPTORS_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_G9991_DDR_QUEUE_DESCRIPTORS_TABLE_SIZE     120
typedef struct
{
	RDD_DDR_QUEUE_DESCRIPTOR_DTS	entry[ RDD_G9991_DDR_QUEUE_DESCRIPTORS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_G9991_DDR_QUEUE_DESCRIPTORS_TABLE_DTS;

#define RDD_G9991_DDR_QUEUE_DESCRIPTORS_TABLE_PTR()	( RDD_G9991_DDR_QUEUE_DESCRIPTORS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + G9991_DDR_QUEUE_DESCRIPTORS_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_G9991_VIRTUAL_PORT_STATUS_PER_EMAC_SIZE     5
typedef struct
{
	RDD_FOUR_BYTES_DTS	entry[ RDD_G9991_VIRTUAL_PORT_STATUS_PER_EMAC_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_G9991_VIRTUAL_PORT_STATUS_PER_EMAC_DTS;

#define RDD_G9991_VIRTUAL_PORT_STATUS_PER_EMAC_PTR()	( RDD_G9991_VIRTUAL_PORT_STATUS_PER_EMAC_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + G9991_VIRTUAL_PORT_STATUS_PER_EMAC_ADDRESS )

#endif
#if defined G9991

#define RDD_MAC_EXTENSION_TABLE_CAM_SIZE     32
typedef struct
{
	RDD_MAC_EXTENSION_ENTRY_DTS	entry[ RDD_MAC_EXTENSION_TABLE_CAM_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MAC_EXTENSION_TABLE_CAM_DTS;

#define RDD_MAC_EXTENSION_TABLE_CAM_PTR()	( RDD_MAC_EXTENSION_TABLE_CAM_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + MAC_EXTENSION_TABLE_CAM_ADDRESS )

#endif
/* COMMON_B */
#if defined G9991

#define RDD_US_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_SIZE     128
typedef struct
{
	RDD_INGRESS_CLASSIFICATION_LONG_LOOKUP_ENTRY_DTS	entry[ RDD_US_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_PTR()	( RDD_US_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + US_INGRESS_CLASSIFICATION_LOOKUP_LONG_TABLE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_CPU_RX_RUNNER_B_SCRATCHPAD_SIZE     8
typedef struct
{
	RDD_RUNNER_SCRATCHPAD_DTS	entry[ RDD_CPU_RX_RUNNER_B_SCRATCHPAD_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_RUNNER_B_SCRATCHPAD_DTS;

#define RDD_CPU_RX_RUNNER_B_SCRATCHPAD_PTR()	( RDD_CPU_RX_RUNNER_B_SCRATCHPAD_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + CPU_RX_RUNNER_B_SCRATCHPAD_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_TUNNEL_TABLE_SIZE     4
typedef struct
{
	RDD_TUNNEL_ENTRY_DTS	entry[ RDD_TUNNEL_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_TUNNEL_TABLE_DTS;

#define RDD_TUNNEL_TABLE_PTR()	( RDD_TUNNEL_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + TUNNEL_TABLE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_WIFI_SSID_FORWARDING_MATRIX_TABLE_SIZE     16
typedef struct
{
	RDD_WIFI_SSID_FORWARDING_MATRIX_ENTRY_DTS	entry[ RDD_WIFI_SSID_FORWARDING_MATRIX_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WIFI_SSID_FORWARDING_MATRIX_TABLE_DTS;

#define RDD_WIFI_SSID_FORWARDING_MATRIX_TABLE_PTR()	( RDD_WIFI_SSID_FORWARDING_MATRIX_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + WIFI_SSID_FORWARDING_MATRIX_TABLE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_EPON_DDR_QUEUE_ADDRESS_TABLE_SIZE     16
typedef struct
{
	RDD_DDR_QUEUE_ADDRESS_ENTRY_DTS	entry[ RDD_EPON_DDR_QUEUE_ADDRESS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EPON_DDR_QUEUE_ADDRESS_TABLE_DTS;

#define RDD_EPON_DDR_QUEUE_ADDRESS_TABLE_PTR()	( RDD_EPON_DDR_QUEUE_ADDRESS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + EPON_DDR_QUEUE_ADDRESS_TABLE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_DUAL_STACK_LITE_TABLE_SIZE     4
typedef struct
{
	RDD_DUAL_STACK_LITE_ENTRY_DTS	entry[ RDD_DUAL_STACK_LITE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DUAL_STACK_LITE_TABLE_DTS;

#define RDD_DUAL_STACK_LITE_TABLE_PTR()	( RDD_DUAL_STACK_LITE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + DUAL_STACK_LITE_TABLE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_EPON_DDR_CACHE_FIFO_SIZE     192
typedef struct
{
	RDD_BBH_TX_DESCRIPTOR_DTS	entry[ RDD_EPON_DDR_CACHE_FIFO_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EPON_DDR_CACHE_FIFO_DTS;

#define RDD_EPON_DDR_CACHE_FIFO_PTR()	( RDD_EPON_DDR_CACHE_FIFO_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + EPON_DDR_CACHE_FIFO_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_IP_SYNC_1588_DESCRIPTOR_QUEUE_SIZE     16
typedef struct
{
	RDD_IP_SYNC_1588_DESCRIPTOR_ENTRY_DTS	entry[ RDD_IP_SYNC_1588_DESCRIPTOR_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IP_SYNC_1588_DESCRIPTOR_QUEUE_DTS;

#define RDD_IP_SYNC_1588_DESCRIPTOR_QUEUE_PTR()	( RDD_IP_SYNC_1588_DESCRIPTOR_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + IP_SYNC_1588_DESCRIPTOR_QUEUE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_US_RING_PACKET_DESCRIPTORS_CACHE_SIZE     12
typedef struct
{
	RDD_CPU_RX_DESCRIPTOR_DTS	entry[ RDD_US_RING_PACKET_DESCRIPTORS_CACHE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_RING_PACKET_DESCRIPTORS_CACHE_DTS;

#define RDD_US_RING_PACKET_DESCRIPTORS_CACHE_PTR()	( RDD_US_RING_PACKET_DESCRIPTORS_CACHE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + US_RING_PACKET_DESCRIPTORS_CACHE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_EPON_DDR_QUEUE_DESCRIPTORS_TABLE_SIZE     16
typedef struct
{
	RDD_DDR_QUEUE_DESCRIPTOR_DTS	entry[ RDD_EPON_DDR_QUEUE_DESCRIPTORS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EPON_DDR_QUEUE_DESCRIPTORS_TABLE_DTS;

#define RDD_EPON_DDR_QUEUE_DESCRIPTORS_TABLE_PTR()	( RDD_EPON_DDR_QUEUE_DESCRIPTORS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + EPON_DDR_QUEUE_DESCRIPTORS_TABLE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_SIZE     16
typedef struct
{
	RDD_BBH_RX_DESCRIPTOR_DTS	entry[ RDD_LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_DTS;

#define RDD_LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_PTR()	( RDD_LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + LOCAL_SWITCHING_LAN_ENQUEUE_INGRESS_QUEUE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_SRC_MAC_ANTI_SPOOFING_TABLE_SIZE     6
#define RDD_SRC_MAC_ANTI_SPOOFING_TABLE_SIZE2    4
typedef struct
{
	RDD_SRC_MAC_ANTI_SPOOFING_ENTRY_DTS	entry[ RDD_SRC_MAC_ANTI_SPOOFING_TABLE_SIZE ][ RDD_SRC_MAC_ANTI_SPOOFING_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SRC_MAC_ANTI_SPOOFING_TABLE_DTS;

#define RDD_SRC_MAC_ANTI_SPOOFING_TABLE_PTR()	( RDD_SRC_MAC_ANTI_SPOOFING_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + SRC_MAC_ANTI_SPOOFING_TABLE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_TUNNEL_DYNAMIC_FIELDS_TABLE_SIZE     4
typedef struct
{
	RDD_TUNNEL_DYNAMIC_FIELDS_ENTRY_DTS	entry[ RDD_TUNNEL_DYNAMIC_FIELDS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_TUNNEL_DYNAMIC_FIELDS_TABLE_DTS;

#define RDD_TUNNEL_DYNAMIC_FIELDS_TABLE_PTR()	( RDD_TUNNEL_DYNAMIC_FIELDS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + TUNNEL_DYNAMIC_FIELDS_TABLE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_WAN_TX_QUEUES_TABLE_SIZE     256
typedef struct
{
	RDD_WAN_TX_QUEUE_DESCRIPTOR_DTS	entry[ RDD_WAN_TX_QUEUES_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WAN_TX_QUEUES_TABLE_DTS;

#define RDD_WAN_TX_QUEUES_TABLE_PTR()	( RDD_WAN_TX_QUEUES_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + WAN_TX_QUEUES_TABLE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_GPON_SKB_ENQUEUED_INDEXES_FREE_PTR_SIZE     40
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_GPON_SKB_ENQUEUED_INDEXES_FREE_PTR_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GPON_SKB_ENQUEUED_INDEXES_FREE_PTR_DTS;

#define RDD_GPON_SKB_ENQUEUED_INDEXES_FREE_PTR_PTR()	( RDD_GPON_SKB_ENQUEUED_INDEXES_FREE_PTR_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + GPON_SKB_ENQUEUED_INDEXES_FREE_PTR_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_SIZE     8
typedef struct
{
	RDD_BROADCOM_SWITCH_PORT_MAPPING_DTS	entry[ RDD_BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_DTS;

#define RDD_BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_PTR()	( RDD_BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + BRIDGE_PORT_TO_BROADCOM_SWITCH_PORT_MAPPING_TABLE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_BRIDGE_PORT_TO_BBH_PERIPHERAL_RX_TABLE_SIZE     7
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_BRIDGE_PORT_TO_BBH_PERIPHERAL_RX_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BRIDGE_PORT_TO_BBH_PERIPHERAL_RX_TABLE_DTS;

#define RDD_BRIDGE_PORT_TO_BBH_PERIPHERAL_RX_TABLE_PTR()	( RDD_BRIDGE_PORT_TO_BBH_PERIPHERAL_RX_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + BRIDGE_PORT_TO_BBH_PERIPHERAL_RX_TABLE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_EGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_SIZE     5
typedef struct
{
	RDD_VLAN_SWITCHING_ISOLATION_CONFIG_ENTRY_DTS	entry[ RDD_EGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_DTS;

#define RDD_EGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_PTR()	( RDD_EGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + EGRESS_VLAN_SWITCHING_ISOLATION_CONFIG_TABLE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_GPON_SKB_ENQUEUED_INDEXES_PUT_PTR_SIZE     40
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_GPON_SKB_ENQUEUED_INDEXES_PUT_PTR_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_GPON_SKB_ENQUEUED_INDEXES_PUT_PTR_DTS;

#define RDD_GPON_SKB_ENQUEUED_INDEXES_PUT_PTR_PTR()	( RDD_GPON_SKB_ENQUEUED_INDEXES_PUT_PTR_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + GPON_SKB_ENQUEUED_INDEXES_PUT_PTR_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_CPU_TX_DESCRIPTOR_QUEUE_TAIL_TABLE_SIZE     4
typedef struct
{
	RDD_ONE_BYTE_DTS	entry[ RDD_CPU_TX_DESCRIPTOR_QUEUE_TAIL_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_DESCRIPTOR_QUEUE_TAIL_TABLE_DTS;

#define RDD_CPU_TX_DESCRIPTOR_QUEUE_TAIL_TABLE_PTR()	( RDD_CPU_TX_DESCRIPTOR_QUEUE_TAIL_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + CPU_TX_DESCRIPTOR_QUEUE_TAIL_TABLE_ADDRESS - 0x8000 )

#endif
#define RDD_RATE_CONTROLLER_EXPONENT_TABLE_PTR()	( RDD_RATE_CONTROLLER_EXPONENT_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + RATE_CONTROLLER_EXPONENT_TABLE_ADDRESS - 0x8000 )

#if defined G9991

#define RDD_LOCAL_SWITCHING_PBITS_TO_QOS_TABLE_SIZE     6
#define RDD_LOCAL_SWITCHING_PBITS_TO_QOS_TABLE_SIZE2    8
typedef struct
{
	RDD_PBITS_TO_QOS_ENTRY_DTS	entry[ RDD_LOCAL_SWITCHING_PBITS_TO_QOS_TABLE_SIZE ][ RDD_LOCAL_SWITCHING_PBITS_TO_QOS_TABLE_SIZE2 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_LOCAL_SWITCHING_PBITS_TO_QOS_TABLE_DTS;

#define RDD_LOCAL_SWITCHING_PBITS_TO_QOS_TABLE_PTR()	( RDD_LOCAL_SWITCHING_PBITS_TO_QOS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + LOCAL_SWITCHING_PBITS_TO_QOS_TABLE_ADDRESS - 0x8000 )

#endif
#define RDD_US_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_PTR()	( RDD_US_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + US_CPU_REASON_TO_CPU_RX_QUEUE_TABLE_ADDRESS - 0x8000 )

#if defined G9991

#define RDD_MULTICAST_ACTIVE_PORTS_TABLE_SIZE     64
typedef struct
{
	RDD_MULTICAST_ACTIVE_PORTS_ENTRY_DTS	entry[ RDD_MULTICAST_ACTIVE_PORTS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MULTICAST_ACTIVE_PORTS_TABLE_DTS;

#define RDD_MULTICAST_ACTIVE_PORTS_TABLE_PTR()	( RDD_MULTICAST_ACTIVE_PORTS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + MULTICAST_ACTIVE_PORTS_TABLE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_CPU_TX_EMAC_LOOPBACK_QUEUE_SIZE     4
typedef struct
{
	RDD_BBH_RX_DESCRIPTOR_DTS	entry[ RDD_CPU_TX_EMAC_LOOPBACK_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_EMAC_LOOPBACK_QUEUE_DTS;

#define RDD_CPU_TX_EMAC_LOOPBACK_QUEUE_PTR()	( RDD_CPU_TX_EMAC_LOOPBACK_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + CPU_TX_EMAC_LOOPBACK_QUEUE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_CPU_TX_US_FLOODING_QUEUE_SIZE     4
typedef struct
{
	RDD_BBH_RX_DESCRIPTOR_DTS	entry[ RDD_CPU_TX_US_FLOODING_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_US_FLOODING_QUEUE_DTS;

#define RDD_CPU_TX_US_FLOODING_QUEUE_PTR()	( RDD_CPU_TX_US_FLOODING_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + CPU_TX_US_FLOODING_QUEUE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_WAN_ENQUEUE_INGRESS_QUEUE_SIZE     64
typedef struct
{
	RDD_INGRESS_QUEUE_ENTRY_DTS	entry[ RDD_WAN_ENQUEUE_INGRESS_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_WAN_ENQUEUE_INGRESS_QUEUE_DTS;

#define RDD_WAN_ENQUEUE_INGRESS_QUEUE_PTR()	( RDD_WAN_ENQUEUE_INGRESS_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + WAN_ENQUEUE_INGRESS_QUEUE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_US_RATE_CONTROLLERS_TABLE_SIZE     128
typedef struct
{
	RDD_US_RATE_CONTROLLER_DESCRIPTOR_DTS	entry[ RDD_US_RATE_CONTROLLERS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_RATE_CONTROLLERS_TABLE_DTS;

#define RDD_US_RATE_CONTROLLERS_TABLE_PTR()	( RDD_US_RATE_CONTROLLERS_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + US_RATE_CONTROLLERS_TABLE_ADDRESS - 0x8000 )

#endif
/* DDR */
#if defined G9991

#define RDD_CONTEXT_TABLE_SIZE     16512
typedef struct
{
	RDD_FLOW_CACHE_CONTEXT_ENTRY_DTS	entry[ RDD_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CONTEXT_TABLE_DTS;

#endif
#if defined G9991

#define RDD_FIREWALL_RULES_TABLE_SIZE     256
typedef struct
{
	RDD_FIREWALL_RULE_ENTRY_DTS	entry[ RDD_FIREWALL_RULES_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FIREWALL_RULES_TABLE_DTS;

#endif
#if defined G9991

#define RDD_EPON_TX_POST_SCHEDULING_DDR_QUEUES_SIZE     65536
typedef struct
{
	RDD_BBH_TX_DESCRIPTOR_DTS	entry[ RDD_EPON_TX_POST_SCHEDULING_DDR_QUEUES_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EPON_TX_POST_SCHEDULING_DDR_QUEUES_DTS;

#endif
#if defined G9991

typedef struct
{
	uint32_t	wlan_mcast_index              	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	counter                       	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bridge_port                   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ingress_classification_context	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0                     	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	valid                         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cache_valid                   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cache_index                   	:9	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1                     	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	port_vector_info              	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = port_vector_info, size = 32 bits
	uint32_t	replication_number            	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_port_vector            	:24	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_DDR_CONTEXT_ENTRY_DTS;

#define RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_READ(r, p)                               MREAD_16((uint8_t *)p, r)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_WRITE(v, p)                              MWRITE_16((uint8_t *)p, v)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_L_READ( wv )                             FIELD_GET( wv, 16, 16 )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_WLAN_MCAST_INDEX_L_WRITE( v, wv )                         FIELD_SET( v, 16, 16, wv )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_READ(r, p)                                        MREAD_16((uint8_t *)p + 2, r)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_WRITE(v, p)                                       MWRITE_16((uint8_t *)p + 2, v)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_L_READ( wv )                                      FIELD_GET( wv, 0, 16 )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_COUNTER_L_WRITE( v, wv )                                  FIELD_SET( v, 0, 16, wv )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_BRIDGE_PORT_READ(r, p)                                    MREAD_8((uint8_t *)p + 4, r)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_BRIDGE_PORT_WRITE(v, p)                                   MWRITE_8((uint8_t *)p + 4, v)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_BRIDGE_PORT_L_READ( wv )                                  FIELD_GET( wv, 24, 8 )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_BRIDGE_PORT_L_WRITE( v, wv )                              FIELD_SET( v, 24, 8, wv )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_READ(r, p)                 MREAD_8((uint8_t *)p + 5, r)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_WRITE(v, p)                MWRITE_8((uint8_t *)p + 5, v)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_L_READ( wv )               FIELD_GET( wv, 16, 8 )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_INGRESS_CLASSIFICATION_CONTEXT_L_WRITE( v, wv )           FIELD_SET( v, 16, 8, wv )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_READ(r, p)                                          FIELD_MREAD_8((uint8_t *)p + 6, 2, 1, r)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_WRITE(v, p)                                         FIELD_MWRITE_8((uint8_t *)p + 6, 2, 1, v)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_L_READ( wv )                                        FIELD_GET( wv, 10, 1 )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_VALID_L_WRITE( v, wv )                                    FIELD_SET( v, 10, 1, wv )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_READ(r, p)                                    FIELD_MREAD_8((uint8_t *)p + 6, 1, 1, r)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_WRITE(v, p)                                   FIELD_MWRITE_8((uint8_t *)p + 6, 1, 1, v)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_L_READ( wv )                                  FIELD_GET( wv, 9, 1 )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_VALID_L_WRITE( v, wv )                              FIELD_SET( v, 9, 1, wv )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_READ(r, p)                                    FIELD_MREAD_16((uint8_t *)p + 6, 0, 9, r)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_WRITE(v, p)                                   FIELD_MWRITE_16((uint8_t *)p + 6, 0, 9, v)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_L_READ( wv )                                  FIELD_GET( wv, 0, 9 )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_CACHE_INDEX_L_WRITE( v, wv )                              FIELD_SET( v, 0, 9, wv )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_PORT_VECTOR_INFO_READ(r, p)                               MREAD_32((uint8_t *)p + 12, r)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_PORT_VECTOR_INFO_WRITE(v, p)                              MWRITE_32((uint8_t *)p + 12, v)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_PORT_VECTOR_INFO_L_READ( wv )                             FIELD_GET( wv, 0, 32 )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_PORT_VECTOR_INFO_L_WRITE( v, wv )                         FIELD_SET( v, 0, 32, wv )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_READ(r, p)                             MREAD_8((uint8_t *)p + 12, r)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_WRITE(v, p)                            MWRITE_8((uint8_t *)p + 12, v)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_L_READ( wv )                           FIELD_GET( wv, 24, 8 )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_REPLICATION_NUMBER_L_WRITE( v, wv )                       FIELD_SET( v, 24, 8, wv )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_READ(r, p)                             FIELD_MREAD_32((uint8_t *)p + 12, 0, 24, r)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_WRITE(v, p)                            FIELD_MWRITE_32((uint8_t *)p + 12, 0, 24, v)
#define RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_L_READ( wv )                           FIELD_GET( wv, 0, 24 )
#define RDD_IPTV_DDR_CONTEXT_ENTRY_EGRESS_PORT_VECTOR_L_WRITE( v, wv )                       FIELD_SET( v, 0, 24, wv )
#endif
#if defined G9991

#define RDD_IPTV_DDR_CONTEXT_TABLE_SIZE     8192
typedef struct
{
	RDD_IPTV_DDR_CONTEXT_ENTRY_DTS	entry[ RDD_IPTV_DDR_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_DDR_CONTEXT_TABLE_DTS;

#endif
#if defined G9991

#define RDD_IPTV_SSM_DDR_CONTEXT_TABLE_SIZE     32768
typedef struct
{
	RDD_IPTV_DDR_CONTEXT_ENTRY_DTS	entry[ RDD_IPTV_SSM_DDR_CONTEXT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_SSM_DDR_CONTEXT_TABLE_DTS;

#endif
#if defined G9991

#define RDD_FIREWALL_RULES_MAP_TABLE_SIZE     8
#define RDD_FIREWALL_RULES_MAP_TABLE_SIZE2    2
#define RDD_FIREWALL_RULES_MAP_TABLE_SIZE3    65536
typedef struct
{
	RDD_FIREWALL_RULES_MAP_ENTRY_DTS	entry[ RDD_FIREWALL_RULES_MAP_TABLE_SIZE ][ RDD_FIREWALL_RULES_MAP_TABLE_SIZE2 ][ RDD_FIREWALL_RULES_MAP_TABLE_SIZE3 ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FIREWALL_RULES_MAP_TABLE_DTS;

#endif
/* PSRAM */
#if defined G9991

#define RDD_DS_INGRESS_CLASSIFICATION_COUNTERS_TABLE_SIZE     256
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_DS_INGRESS_CLASSIFICATION_COUNTERS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_CLASSIFICATION_COUNTERS_TABLE_DTS;

#define RDD_DS_INGRESS_CLASSIFICATION_COUNTERS_TABLE_PTR()	( RDD_DS_INGRESS_CLASSIFICATION_COUNTERS_TABLE_DTS * )(DEVICE_ADDRESS( PSRAM_BLOCK_OFFSET ) + DS_INGRESS_CLASSIFICATION_COUNTERS_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_INGRESS_CLASSIFICATION_COUNTERS_TABLE_SIZE     256
typedef struct
{
	RDD_TWO_BYTES_DTS	entry[ RDD_US_INGRESS_CLASSIFICATION_COUNTERS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_COUNTERS_TABLE_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_COUNTERS_TABLE_PTR()	( RDD_US_INGRESS_CLASSIFICATION_COUNTERS_TABLE_DTS * )(DEVICE_ADDRESS( PSRAM_BLOCK_OFFSET ) + US_INGRESS_CLASSIFICATION_COUNTERS_TABLE_ADDRESS )

#endif
#if defined OREN

typedef union
{
	RDD_BBH_TX_DESCRIPTOR_DTS bbh_tx_descriptor;
	RDD_PACKET_DESCRIPTOR_DTS packet_descriptor;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_PACKET_DESCRIPTOR_UNION_DTS;

#endif
#if defined OREN

#define RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE     1536
typedef struct
{
	RDD_DS_PACKET_DESCRIPTOR_UNION_DTS	entry[ RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_DTS;

#define RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_PTR()	( RDD_DS_FREE_PACKET_DESCRIPTORS_POOL_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + DS_FREE_PACKET_DESCRIPTORS_POOL_ADDRESS )

#endif
#if defined OREN || defined G9991

typedef union
{
	RDD_CPU_TX_DESCRIPTOR_DS_FAST_DTS      cpu_tx_descriptor_ds_fast;
	RDD_CPU_TX_MESSAGE_DESCRIPTOR_DTS      cpu_tx_message_descriptor;
	RDD_CPU_TX_DESCRIPTOR_DS_PICO_WIFI_DTS cpu_tx_descriptor_ds_pico_wifi;
	RDD_CPU_TX_DESCRIPTOR_ABS_DTS          cpu_tx_descriptor_abs;
	RDD_CPU_TX_DESCRIPTOR_CORE_DTS         cpu_tx_descriptor_core;
	RDD_CPU_TX_DESCRIPTOR_US_FAST_DTS      cpu_tx_descriptor_us_fast;
	RDD_CPU_TX_DHD_DESCRIPTOR_DTS          cpu_tx_dhd_descriptor;
	RDD_CPU_TX_DESCRIPTOR_DS_PICO_DTS      cpu_tx_descriptor_ds_pico;
	RDD_CPU_TX_DESCRIPTOR_BPM_DTS          cpu_tx_descriptor_bpm;
	RDD_CPU_TX_DESCRIPTOR_DTS              cpu_tx_descriptor;
	RDD_CPU_TX_DHD_MESSAGE_DESCRIPTOR_DTS  cpu_tx_dhd_message_descriptor;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_PICO_DESCRIPTOR_UNION_DTS;

#endif
#if defined OREN

#define RDD_CPU_TX_PICO_QUEUE_SIZE     16
typedef struct
{
	RDD_CPU_TX_PICO_DESCRIPTOR_UNION_DTS	entry[ RDD_CPU_TX_PICO_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_PICO_QUEUE_DTS;

#define RDD_CPU_TX_PICO_QUEUE_PTR()	( RDD_CPU_TX_PICO_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + CPU_TX_PICO_QUEUE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_CPU_TX_PICO_QUEUE_SIZE     16
typedef struct
{
	RDD_CPU_TX_PICO_DESCRIPTOR_UNION_DTS	entry[ RDD_US_CPU_TX_PICO_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_CPU_TX_PICO_QUEUE_DTS;

#define RDD_US_CPU_TX_PICO_QUEUE_PTR()	( RDD_US_CPU_TX_PICO_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_TX_PICO_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_CPU_TX_PICO_QUEUE_SIZE     16
typedef struct
{
	RDD_CPU_TX_PICO_DESCRIPTOR_UNION_DTS	entry[ RDD_CPU_TX_PICO_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_PICO_QUEUE_DTS;

#define RDD_CPU_TX_PICO_QUEUE_PTR()	( RDD_CPU_TX_PICO_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + CPU_TX_PICO_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_CPU_TX_PICO_QUEUE_SIZE     16
typedef struct
{
	RDD_CPU_TX_PICO_DESCRIPTOR_UNION_DTS	entry[ RDD_US_CPU_TX_PICO_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_CPU_TX_PICO_QUEUE_DTS;

#define RDD_US_CPU_TX_PICO_QUEUE_PTR()	( RDD_US_CPU_TX_PICO_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_TX_PICO_QUEUE_ADDRESS )

#endif
#if defined OREN || defined G9991

typedef union
{
	RDD_INGRESS_CLASSIFICATION_SHORT_LOOKUP_ENTRY_DTS        ingress_classification_short_lookup_entry;
	RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_DTS        ds_ingress_classification_ih_lookup_entry;
	RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_DTS ds_ingress_classification_optimized_lookup_entry;
	RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_DTS us_ingress_classification_optimized_lookup_entry;
	RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_DTS        us_ingress_classification_ih_lookup_entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INGRESS_CLASSIFICATION_LOOKUP_UNION_DTS;

#endif
#if defined OREN

#define RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_TABLE_SIZE     256
typedef struct
{
	RDD_INGRESS_CLASSIFICATION_LOOKUP_UNION_DTS	entry[ RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_TABLE_DTS;

#define RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_TABLE_PTR()	( RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + DS_INGRESS_CLASSIFICATION_LOOKUP_TABLE_ADDRESS - 0x8000 )

#endif
#if defined OREN

#define RDD_US_INGRESS_CLASSIFICATION_LOOKUP_TABLE_SIZE     256
typedef struct
{
	RDD_INGRESS_CLASSIFICATION_LOOKUP_UNION_DTS	entry[ RDD_US_INGRESS_CLASSIFICATION_LOOKUP_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_LOOKUP_TABLE_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_LOOKUP_TABLE_PTR()	( RDD_US_INGRESS_CLASSIFICATION_LOOKUP_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + US_INGRESS_CLASSIFICATION_LOOKUP_TABLE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_TABLE_SIZE     256
typedef struct
{
	RDD_INGRESS_CLASSIFICATION_LOOKUP_UNION_DTS	entry[ RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_TABLE_DTS;

#define RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_TABLE_PTR()	( RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + DS_INGRESS_CLASSIFICATION_LOOKUP_TABLE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_US_INGRESS_CLASSIFICATION_LOOKUP_TABLE_SIZE     256
typedef struct
{
	RDD_INGRESS_CLASSIFICATION_LOOKUP_UNION_DTS	entry[ RDD_US_INGRESS_CLASSIFICATION_LOOKUP_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_LOOKUP_TABLE_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_LOOKUP_TABLE_PTR()	( RDD_US_INGRESS_CLASSIFICATION_LOOKUP_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + US_INGRESS_CLASSIFICATION_LOOKUP_TABLE_ADDRESS - 0x8000 )

#endif
#if defined OREN || defined G9991

typedef union
{
	RDD_INGRESS_CLASSIFICATION_SHORT_LOOKUP_ENTRY_DTS        ingress_classification_short_lookup_entry;
	RDD_DS_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_DTS        ds_ingress_classification_ih_lookup_entry;
	RDD_DS_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_DTS ds_ingress_classification_optimized_lookup_entry;
	RDD_US_INGRESS_CLASSIFICATION_OPTIMIZED_LOOKUP_ENTRY_DTS us_ingress_classification_optimized_lookup_entry;
	RDD_US_INGRESS_CLASSIFICATION_IH_LOOKUP_ENTRY_DTS        us_ingress_classification_ih_lookup_entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_INGRESS_CLASSIFICATION_LOOKUP_CAM_UNION_DTS;

#endif
#if defined OREN

#define RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_SIZE     32
typedef struct
{
	RDD_INGRESS_CLASSIFICATION_LOOKUP_CAM_UNION_DTS	entry[ RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_DTS;

#define RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_PTR()	( RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + DS_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_ADDRESS - 0x8000 )

#endif
#if defined OREN

#define RDD_US_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_SIZE     32
typedef struct
{
	RDD_INGRESS_CLASSIFICATION_LOOKUP_CAM_UNION_DTS	entry[ RDD_US_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_PTR()	( RDD_US_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + US_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_SIZE     32
typedef struct
{
	RDD_INGRESS_CLASSIFICATION_LOOKUP_CAM_UNION_DTS	entry[ RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_DTS;

#define RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_PTR()	( RDD_DS_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + DS_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_ADDRESS - 0x8000 )

#endif
#if defined G9991

#define RDD_US_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_SIZE     32
typedef struct
{
	RDD_INGRESS_CLASSIFICATION_LOOKUP_CAM_UNION_DTS	entry[ RDD_US_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_DTS;

#define RDD_US_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_PTR()	( RDD_US_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_1_OFFSET ) + US_INGRESS_CLASSIFICATION_LOOKUP_CAM_TABLE_ADDRESS - 0x8000 )

#endif
#if defined OREN || defined G9991

typedef union
{
	RDD_BBH_TX_DESCRIPTOR_DTS bbh_tx_descriptor;
	RDD_PACKET_DESCRIPTOR_DTS packet_descriptor;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_PACKET_DESCRIPTOR_UNION_DTS;

#endif
#if defined OREN

#define RDD_US_FREE_PACKET_DESCRIPTORS_POOL_SIZE     3072
typedef struct
{
	RDD_US_PACKET_DESCRIPTOR_UNION_DTS	entry[ RDD_US_FREE_PACKET_DESCRIPTORS_POOL_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_FREE_PACKET_DESCRIPTORS_POOL_DTS;

#define RDD_US_FREE_PACKET_DESCRIPTORS_POOL_PTR()	( RDD_US_FREE_PACKET_DESCRIPTORS_POOL_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FREE_PACKET_DESCRIPTORS_POOL_ADDRESS )

#endif
#if defined G9991

#define RDD_US_FREE_PACKET_DESCRIPTORS_POOL_SIZE     3072
typedef struct
{
	RDD_US_PACKET_DESCRIPTOR_UNION_DTS	entry[ RDD_US_FREE_PACKET_DESCRIPTORS_POOL_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_FREE_PACKET_DESCRIPTORS_POOL_DTS;

#define RDD_US_FREE_PACKET_DESCRIPTORS_POOL_PTR()	( RDD_US_FREE_PACKET_DESCRIPTORS_POOL_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_FREE_PACKET_DESCRIPTORS_POOL_ADDRESS )

#endif
#if defined OREN || defined G9991

typedef union
{
	RDD_IPTV_L3_DDR_LOOKUP_ENTRY_DTS     iptv_l3_ddr_lookup_entry;
	RDD_IPTV_L2_DDR_LOOKUP_ENTRY_DTS     iptv_l2_ddr_lookup_entry;
	RDD_IPTV_L3_SSM_DDR_LOOKUP_ENTRY_DTS iptv_l3_ssm_ddr_lookup_entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_LOOKUP_DDR_UNION_DTS;

#endif
#if defined OREN

#define RDD_IPTV_DDR_LOOKUP_TABLE_SIZE     8192
typedef struct
{
	RDD_IPTV_LOOKUP_DDR_UNION_DTS	entry[ RDD_IPTV_DDR_LOOKUP_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_DDR_LOOKUP_TABLE_DTS;

#endif
#if defined G9991

#define RDD_IPTV_DDR_LOOKUP_TABLE_SIZE     8192
typedef struct
{
	RDD_IPTV_LOOKUP_DDR_UNION_DTS	entry[ RDD_IPTV_DDR_LOOKUP_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_DDR_LOOKUP_TABLE_DTS;

#endif
#if defined OREN || defined G9991

typedef union
{
	RDD_CPU_TX_DESCRIPTOR_DS_FAST_DTS      cpu_tx_descriptor_ds_fast;
	RDD_CPU_TX_MESSAGE_DESCRIPTOR_DTS      cpu_tx_message_descriptor;
	RDD_CPU_TX_DESCRIPTOR_DS_PICO_WIFI_DTS cpu_tx_descriptor_ds_pico_wifi;
	RDD_CPU_TX_DESCRIPTOR_ABS_DTS          cpu_tx_descriptor_abs;
	RDD_CPU_TX_DESCRIPTOR_CORE_DTS         cpu_tx_descriptor_core;
	RDD_CPU_TX_DESCRIPTOR_US_FAST_DTS      cpu_tx_descriptor_us_fast;
	RDD_CPU_TX_DESCRIPTOR_DS_PICO_DTS      cpu_tx_descriptor_ds_pico;
	RDD_CPU_TX_DESCRIPTOR_BPM_DTS          cpu_tx_descriptor_bpm;
	RDD_CPU_TX_DESCRIPTOR_DTS              cpu_tx_descriptor;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_FAST_DESCRIPTOR_UNION_DTS;

#endif
#if defined OREN

#define RDD_CPU_TX_FAST_QUEUE_SIZE     16
typedef struct
{
	RDD_CPU_TX_FAST_DESCRIPTOR_UNION_DTS	entry[ RDD_CPU_TX_FAST_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_FAST_QUEUE_DTS;

#define RDD_CPU_TX_FAST_QUEUE_PTR()	( RDD_CPU_TX_FAST_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + CPU_TX_FAST_QUEUE_ADDRESS )

#endif
#if defined OREN

#define RDD_US_CPU_TX_FAST_QUEUE_SIZE     16
typedef struct
{
	RDD_CPU_TX_FAST_DESCRIPTOR_UNION_DTS	entry[ RDD_US_CPU_TX_FAST_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_CPU_TX_FAST_QUEUE_DTS;

#define RDD_US_CPU_TX_FAST_QUEUE_PTR()	( RDD_US_CPU_TX_FAST_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_TX_FAST_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_CPU_TX_FAST_QUEUE_SIZE     16
typedef struct
{
	RDD_CPU_TX_FAST_DESCRIPTOR_UNION_DTS	entry[ RDD_CPU_TX_FAST_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_FAST_QUEUE_DTS;

#define RDD_CPU_TX_FAST_QUEUE_PTR()	( RDD_CPU_TX_FAST_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_0_OFFSET ) + CPU_TX_FAST_QUEUE_ADDRESS )

#endif
#if defined G9991

#define RDD_US_CPU_TX_FAST_QUEUE_SIZE     16
typedef struct
{
	RDD_CPU_TX_FAST_DESCRIPTOR_UNION_DTS	entry[ RDD_US_CPU_TX_FAST_QUEUE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_CPU_TX_FAST_QUEUE_DTS;

#define RDD_US_CPU_TX_FAST_QUEUE_PTR()	( RDD_US_CPU_TX_FAST_QUEUE_DTS * )(DEVICE_ADDRESS( RUNNER_PRIVATE_1_OFFSET ) + US_CPU_TX_FAST_QUEUE_ADDRESS )

#endif
#if defined OREN || defined G9991

typedef union
{
	RDD_IPTV_L3_LOOKUP_ENTRY_DTS iptv_l3_lookup_entry;
	RDD_IPTV_L2_LOOKUP_ENTRY_DTS iptv_l2_lookup_entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_LOOKUP_UNION_DTS;

#endif
#if defined OREN

#define RDD_IPTV_LOOKUP_TABLE_SIZE     256
typedef struct
{
	RDD_IPTV_LOOKUP_UNION_DTS	entry[ RDD_IPTV_LOOKUP_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_LOOKUP_TABLE_DTS;

#define RDD_IPTV_LOOKUP_TABLE_PTR()	( RDD_IPTV_LOOKUP_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + IPTV_LOOKUP_TABLE_ADDRESS )

#endif
#if defined G9991

#define RDD_IPTV_LOOKUP_TABLE_SIZE     256
typedef struct
{
	RDD_IPTV_LOOKUP_UNION_DTS	entry[ RDD_IPTV_LOOKUP_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_IPTV_LOOKUP_TABLE_DTS;

#define RDD_IPTV_LOOKUP_TABLE_PTR()	( RDD_IPTV_LOOKUP_TABLE_DTS * )(DEVICE_ADDRESS( RUNNER_COMMON_0_OFFSET ) + IPTV_LOOKUP_TABLE_ADDRESS )

#endif

#if defined OREN
typedef enum
{
	DS_ACTION_ID_FIRST =     0,
	DS_ACTION_ID_TRAP         = 0,
	DS_ACTION_ID_TTL          = 2,
	DS_ACTION_ID_DSCP         = 4,
	DS_ACTION_ID_NAT          = 5,
	DS_ACTION_ID_GRE          = 6,
	DS_ACTION_ID_OPBITS       = 7,
	DS_ACTION_ID_IPBITS       = 8,
	DS_ACTION_ID_DS_LITE      = 9,
	DS_ACTION_ID_PPPOE        = 10,
	DS_ACTION_ID_SPDSVC       = 13,
	DS_ACTION_ID_TOTAL_NUM    = 17,
	DS_ACTION_ID_LAST =      17
} rdd_ds_action_id;
typedef enum
{
	US_ACTION_ID_FIRST =      0,
	US_ACTION_ID_TRAP          = 0,
	US_ACTION_ID_TTL           = 2,
	US_ACTION_ID_DSCP          = 4,
	US_ACTION_ID_NAT           = 5,
	US_ACTION_ID_GRE           = 6,
	US_ACTION_ID_OPBITS        = 7,
	US_ACTION_ID_IPBITS        = 8,
	US_ACTION_ID_DS_LITE       = 9,
	US_ACTION_ID_GRE_TUNNEL    = 10,
	US_ACTION_ID_PPPOE         = 11,
	US_ACTION_ID_SPDSVC        = 13,
	US_ACTION_ID_TOTAL_NUM     = 17,
	US_ACTION_ID_LAST =       17
} rdd_us_action_id;
#endif

#if defined G9991
typedef enum
{
	US_SID_STATE_MACHINE_FIRST =         0,
	US_SID_STATE_MACHINE_WAITING_SOF      = 0,
	US_SID_STATE_MACHINE_SOF_FORWARDED    = 1,
	US_SID_STATE_MACHINE_SOF_TRAPPED      = 2,
	US_SID_STATE_MACHINE_SOF_DROPPED      = 3,
	US_SID_STATE_MACHINE_LAST =          3
} rdd_us_sid_state_machine;
typedef enum
{
	DS_ACTION_ID_FIRST =     0,
	DS_ACTION_ID_TRAP         = 0,
	DS_ACTION_ID_TTL          = 2,
	DS_ACTION_ID_DSCP         = 4,
	DS_ACTION_ID_NAT          = 5,
	DS_ACTION_ID_GRE          = 6,
	DS_ACTION_ID_OPBITS       = 7,
	DS_ACTION_ID_IPBITS       = 8,
	DS_ACTION_ID_DS_LITE      = 9,
	DS_ACTION_ID_PPPOE        = 10,
	DS_ACTION_ID_SPDSVC       = 13,
	DS_ACTION_ID_TOTAL_NUM    = 17,
	DS_ACTION_ID_LAST =      17
} rdd_ds_action_id;
typedef enum
{
	US_ACTION_ID_FIRST =      0,
	US_ACTION_ID_TRAP          = 0,
	US_ACTION_ID_TTL           = 2,
	US_ACTION_ID_DSCP          = 4,
	US_ACTION_ID_NAT           = 5,
	US_ACTION_ID_GRE           = 6,
	US_ACTION_ID_OPBITS        = 7,
	US_ACTION_ID_IPBITS        = 8,
	US_ACTION_ID_DS_LITE       = 9,
	US_ACTION_ID_GRE_TUNNEL    = 10,
	US_ACTION_ID_PPPOE         = 11,
	US_ACTION_ID_SPDSVC        = 13,
	US_ACTION_ID_TOTAL_NUM     = 17,
	US_ACTION_ID_LAST =       17
} rdd_us_action_id;
#endif
#endif /* _RDD_DATA_STRUCTURES_AUTO_H */

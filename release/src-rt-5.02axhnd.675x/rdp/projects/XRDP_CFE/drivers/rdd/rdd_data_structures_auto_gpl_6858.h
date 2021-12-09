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
#ifndef _RDD_DATA_STRUCTURES_AUTO_H_
#define _RDD_DATA_STRUCTURES_AUTO_H_
#define GROUPED_EN_SEGMENTS_NUM       3
#define INVALID_TABLE_ADDRESS         0xFFFFFF
/* >>>RDD_BYTES_2_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint16_t	bits      	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint16_t	bits      	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BYTES_2_DTS;

#define RDD_BYTES_2_BITS_READ_G(r, g, idx)          GROUP_MREAD_16(g, idx*sizeof(RDD_BYTES_2_DTS), r)
#define RDD_BYTES_2_BITS_WRITE_G(v, g, idx)         GROUP_MWRITE_16(g, idx*sizeof(RDD_BYTES_2_DTS), v)
#define RDD_BYTES_2_BITS_READ(r, p)                 MREAD_16((uint8_t *)p, r)
#define RDD_BYTES_2_BITS_WRITE(v, p)                MWRITE_16((uint8_t *)p, v)
/* <<<RDD_BYTES_2_DTS */
/* >>>RDD_BYTE_1_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint8_t	bits      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint8_t	bits      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BYTE_1_DTS;

#define RDD_BYTE_1_BITS_READ_G(r, g, idx)          GROUP_MREAD_8(g, idx*sizeof(RDD_BYTE_1_DTS), r)
#define RDD_BYTE_1_BITS_WRITE_G(v, g, idx)         GROUP_MWRITE_8(g, idx*sizeof(RDD_BYTE_1_DTS), v)
#define RDD_BYTE_1_BITS_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_BYTE_1_BITS_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
/* <<<RDD_BYTE_1_DTS */
/* >>>CPU_RING_INTERRUPT_COUNTER_TABLE */

/* >>>RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	counter   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	max_size  	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	counter   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	max_size  	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_DTS;

#define RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_COUNTER_READ_G(r, g, idx)           GROUP_MREAD_32(g, idx*sizeof(RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_DTS), r)
#define RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_COUNTER_WRITE_G(v, g, idx)          GROUP_MWRITE_32(g, idx*sizeof(RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_DTS), v)
#define RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_COUNTER_READ(r, p)                  MREAD_32((uint8_t *)p, r)
#define RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_COUNTER_WRITE(v, p)                 MWRITE_32((uint8_t *)p, v)
#define RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_MAX_SIZE_READ_G(r, g, idx)          GROUP_MREAD_32(g, idx*sizeof(RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_DTS) + 4, r)
#define RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_MAX_SIZE_WRITE_G(v, g, idx)         GROUP_MWRITE_32(g, idx*sizeof(RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_DTS) + 4, v)
#define RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_MAX_SIZE_READ(r, p)                 MREAD_32((uint8_t *)p + 4, r)
#define RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_MAX_SIZE_WRITE(v, p)                MWRITE_32((uint8_t *)p + 4, v)
/* <<<RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_DTS */


#define RDD_CPU_RING_INTERRUPT_COUNTER_TABLE_SIZE     18
typedef struct
{
	RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_DTS	entry[ RDD_CPU_RING_INTERRUPT_COUNTER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RING_INTERRUPT_COUNTER_TABLE_DTS;

extern uint32_t RDD_CPU_RING_INTERRUPT_COUNTER_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_RING_INTERRUPT_COUNTER_TABLE_PTR(core_id)	( RDD_CPU_RING_INTERRUPT_COUNTER_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_RING_INTERRUPT_COUNTER_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_RING_INTERRUPT_COUNTER_TABLE */
/* >>>CPU_RECYCLE_INTERRUPT_COALESCING_TABLE */

/* >>>RDD_CPU_INTERRUPT_COALESCING_ENTRY_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	timer_period	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	timer_armed 	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	scratch     	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	timer_period	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	timer_armed 	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	scratch     	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_INTERRUPT_COALESCING_ENTRY_DTS;

#define RDD_CPU_INTERRUPT_COALESCING_ENTRY_TIMER_PERIOD_READ_G(r, g, idx)          GROUP_MREAD_32(g, idx*sizeof(RDD_CPU_INTERRUPT_COALESCING_ENTRY_DTS), r)
#define RDD_CPU_INTERRUPT_COALESCING_ENTRY_TIMER_PERIOD_WRITE_G(v, g, idx)         GROUP_MWRITE_32(g, idx*sizeof(RDD_CPU_INTERRUPT_COALESCING_ENTRY_DTS), v)
#define RDD_CPU_INTERRUPT_COALESCING_ENTRY_TIMER_PERIOD_READ(r, p)                 MREAD_32((uint8_t *)p, r)
#define RDD_CPU_INTERRUPT_COALESCING_ENTRY_TIMER_PERIOD_WRITE(v, p)                MWRITE_32((uint8_t *)p, v)
#define RDD_CPU_INTERRUPT_COALESCING_ENTRY_TIMER_ARMED_READ_G(r, g, idx)           GROUP_MREAD_32(g, idx*sizeof(RDD_CPU_INTERRUPT_COALESCING_ENTRY_DTS) + 4, r)
#define RDD_CPU_INTERRUPT_COALESCING_ENTRY_TIMER_ARMED_WRITE_G(v, g, idx)          GROUP_MWRITE_32(g, idx*sizeof(RDD_CPU_INTERRUPT_COALESCING_ENTRY_DTS) + 4, v)
#define RDD_CPU_INTERRUPT_COALESCING_ENTRY_TIMER_ARMED_READ(r, p)                  MREAD_32((uint8_t *)p + 4, r)
#define RDD_CPU_INTERRUPT_COALESCING_ENTRY_TIMER_ARMED_WRITE(v, p)                 MWRITE_32((uint8_t *)p + 4, v)
#define RDD_CPU_INTERRUPT_COALESCING_ENTRY_SCRATCH_READ_G(r, g, idx)               GROUP_MREAD_32(g, idx*sizeof(RDD_CPU_INTERRUPT_COALESCING_ENTRY_DTS) + 8, r)
#define RDD_CPU_INTERRUPT_COALESCING_ENTRY_SCRATCH_WRITE_G(v, g, idx)              GROUP_MWRITE_32(g, idx*sizeof(RDD_CPU_INTERRUPT_COALESCING_ENTRY_DTS) + 8, v)
#define RDD_CPU_INTERRUPT_COALESCING_ENTRY_SCRATCH_READ(r, p)                      MREAD_32((uint8_t *)p + 8, r)
#define RDD_CPU_INTERRUPT_COALESCING_ENTRY_SCRATCH_WRITE(v, p)                     MWRITE_32((uint8_t *)p + 8, v)
/* <<<RDD_CPU_INTERRUPT_COALESCING_ENTRY_DTS */

typedef struct
{
	RDD_CPU_INTERRUPT_COALESCING_ENTRY_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_DTS;

extern uint32_t RDD_CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_PTR(core_id)	( RDD_CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_RECYCLE_INTERRUPT_COALESCING_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_RECYCLE_INTERRUPT_COALESCING_TABLE */
/* >>>DS_CPU_REASON_TO_METER_TABLE */

#define RDD_DS_CPU_REASON_TO_METER_TABLE_SIZE     64
typedef struct
{
	RDD_BYTE_1_DTS	entry[ RDD_DS_CPU_REASON_TO_METER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_CPU_REASON_TO_METER_TABLE_DTS;

extern uint32_t RDD_DS_CPU_REASON_TO_METER_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_DS_CPU_REASON_TO_METER_TABLE_PTR(core_id)	( RDD_DS_CPU_REASON_TO_METER_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_DS_CPU_REASON_TO_METER_TABLE_ADDRESS_ARR[core_id] ))

/* <<<DS_CPU_REASON_TO_METER_TABLE */
/* >>>CPU_REASON_AND_VPORT_TO_METER_TABLE */

#define RDD_CPU_REASON_AND_VPORT_TO_METER_TABLE_SIZE     120
typedef struct
{
	RDD_BYTE_1_DTS	entry[ RDD_CPU_REASON_AND_VPORT_TO_METER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_REASON_AND_VPORT_TO_METER_TABLE_DTS;

extern uint32_t RDD_CPU_REASON_AND_VPORT_TO_METER_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_REASON_AND_VPORT_TO_METER_TABLE_PTR(core_id)	( RDD_CPU_REASON_AND_VPORT_TO_METER_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_REASON_AND_VPORT_TO_METER_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_REASON_AND_VPORT_TO_METER_TABLE */
/* >>>RDD_DDR_ADDRESS_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	low       	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	high      	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	low       	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	high      	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DDR_ADDRESS_DTS;

#define RDD_DDR_ADDRESS_LOW_READ_G(r, g, idx)           GROUP_MREAD_32(g, idx*sizeof(RDD_DDR_ADDRESS_DTS), r)
#define RDD_DDR_ADDRESS_LOW_WRITE_G(v, g, idx)          GROUP_MWRITE_32(g, idx*sizeof(RDD_DDR_ADDRESS_DTS), v)
#define RDD_DDR_ADDRESS_LOW_READ(r, p)                  MREAD_32((uint8_t *)p, r)
#define RDD_DDR_ADDRESS_LOW_WRITE(v, p)                 MWRITE_32((uint8_t *)p, v)
#define RDD_DDR_ADDRESS_HIGH_READ_G(r, g, idx)          GROUP_MREAD_32(g, idx*sizeof(RDD_DDR_ADDRESS_DTS) + 4, r)
#define RDD_DDR_ADDRESS_HIGH_WRITE_G(v, g, idx)         GROUP_MWRITE_32(g, idx*sizeof(RDD_DDR_ADDRESS_DTS) + 4, v)
#define RDD_DDR_ADDRESS_HIGH_READ(r, p)                 MREAD_32((uint8_t *)p + 4, r)
#define RDD_DDR_ADDRESS_HIGH_WRITE(v, p)                MWRITE_32((uint8_t *)p + 4, v)
/* <<<RDD_DDR_ADDRESS_DTS */
/* >>>US_CPU_REASON_TO_METER_TABLE */

#define RDD_US_CPU_REASON_TO_METER_TABLE_SIZE     64
typedef struct
{
	RDD_BYTE_1_DTS	entry[ RDD_US_CPU_REASON_TO_METER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_CPU_REASON_TO_METER_TABLE_DTS;

extern uint32_t RDD_US_CPU_REASON_TO_METER_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_US_CPU_REASON_TO_METER_TABLE_PTR(core_id)	( RDD_US_CPU_REASON_TO_METER_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_US_CPU_REASON_TO_METER_TABLE_ADDRESS_ARR[core_id] ))

/* <<<US_CPU_REASON_TO_METER_TABLE */
/* >>>CPU_VPORT_TO_METER_TABLE */

#define RDD_CPU_VPORT_TO_METER_TABLE_SIZE     40
typedef struct
{
	RDD_BYTE_1_DTS	entry[ RDD_CPU_VPORT_TO_METER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_VPORT_TO_METER_TABLE_DTS;

extern uint32_t RDD_CPU_VPORT_TO_METER_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_VPORT_TO_METER_TABLE_PTR(core_id)	( RDD_CPU_VPORT_TO_METER_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_VPORT_TO_METER_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_VPORT_TO_METER_TABLE */
/* >>>CPU_RECYCLE_RING_DESCRIPTOR_TABLE */

/* >>>RDD_CPU_RING_DESCRIPTOR_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	size_of_entry         	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	number_of_entries     	:11	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	interrupt_id          	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	drop_counter          	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	write_idx             	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	base_addr_low         	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	read_idx              	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	low_priority_threshold	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	base_addr_high        	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	interrupt_id          	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	number_of_entries     	:11	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	size_of_entry         	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	write_idx             	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	drop_counter          	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	base_addr_low         	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	base_addr_high        	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	low_priority_threshold	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	read_idx              	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RING_DESCRIPTOR_DTS;

#define RDD_CPU_RING_DESCRIPTOR_SIZE_OF_ENTRY_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_RING_DESCRIPTOR_DTS), 3, 5, r)
#define RDD_CPU_RING_DESCRIPTOR_SIZE_OF_ENTRY_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_RING_DESCRIPTOR_DTS), 3, 5, v)
#define RDD_CPU_RING_DESCRIPTOR_SIZE_OF_ENTRY_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p, 3, 5, r)
#define RDD_CPU_RING_DESCRIPTOR_SIZE_OF_ENTRY_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p, 3, 5, v)
#define RDD_CPU_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_READ_G(r, g, idx)               GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_CPU_RING_DESCRIPTOR_DTS), 0, 11, r)
#define RDD_CPU_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_CPU_RING_DESCRIPTOR_DTS), 0, 11, v)
#define RDD_CPU_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_READ(r, p)                      FIELD_MREAD_16((uint8_t *)p, 0, 11, r)
#define RDD_CPU_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_WRITE(v, p)                     FIELD_MWRITE_16((uint8_t *)p, 0, 11, v)
#define RDD_CPU_RING_DESCRIPTOR_INTERRUPT_ID_READ_G(r, g, idx)                    GROUP_MREAD_16(g, idx*sizeof(RDD_CPU_RING_DESCRIPTOR_DTS) + 2, r)
#define RDD_CPU_RING_DESCRIPTOR_INTERRUPT_ID_WRITE_G(v, g, idx)                   GROUP_MWRITE_16(g, idx*sizeof(RDD_CPU_RING_DESCRIPTOR_DTS) + 2, v)
#define RDD_CPU_RING_DESCRIPTOR_INTERRUPT_ID_READ(r, p)                           MREAD_16((uint8_t *)p + 2, r)
#define RDD_CPU_RING_DESCRIPTOR_INTERRUPT_ID_WRITE(v, p)                          MWRITE_16((uint8_t *)p + 2, v)
#define RDD_CPU_RING_DESCRIPTOR_DROP_COUNTER_READ_G(r, g, idx)                    GROUP_MREAD_16(g, idx*sizeof(RDD_CPU_RING_DESCRIPTOR_DTS) + 4, r)
#define RDD_CPU_RING_DESCRIPTOR_DROP_COUNTER_WRITE_G(v, g, idx)                   GROUP_MWRITE_16(g, idx*sizeof(RDD_CPU_RING_DESCRIPTOR_DTS) + 4, v)
#define RDD_CPU_RING_DESCRIPTOR_DROP_COUNTER_READ(r, p)                           MREAD_16((uint8_t *)p + 4, r)
#define RDD_CPU_RING_DESCRIPTOR_DROP_COUNTER_WRITE(v, p)                          MWRITE_16((uint8_t *)p + 4, v)
#define RDD_CPU_RING_DESCRIPTOR_WRITE_IDX_READ_G(r, g, idx)                       GROUP_MREAD_16(g, idx*sizeof(RDD_CPU_RING_DESCRIPTOR_DTS) + 6, r)
#define RDD_CPU_RING_DESCRIPTOR_WRITE_IDX_WRITE_G(v, g, idx)                      GROUP_MWRITE_16(g, idx*sizeof(RDD_CPU_RING_DESCRIPTOR_DTS) + 6, v)
#define RDD_CPU_RING_DESCRIPTOR_WRITE_IDX_READ(r, p)                              MREAD_16((uint8_t *)p + 6, r)
#define RDD_CPU_RING_DESCRIPTOR_WRITE_IDX_WRITE(v, p)                             MWRITE_16((uint8_t *)p + 6, v)
#define RDD_CPU_RING_DESCRIPTOR_BASE_ADDR_LOW_READ_G(r, g, idx)                   GROUP_MREAD_32(g, idx*sizeof(RDD_CPU_RING_DESCRIPTOR_DTS) + 8, r)
#define RDD_CPU_RING_DESCRIPTOR_BASE_ADDR_LOW_WRITE_G(v, g, idx)                  GROUP_MWRITE_32(g, idx*sizeof(RDD_CPU_RING_DESCRIPTOR_DTS) + 8, v)
#define RDD_CPU_RING_DESCRIPTOR_BASE_ADDR_LOW_READ(r, p)                          MREAD_32((uint8_t *)p + 8, r)
#define RDD_CPU_RING_DESCRIPTOR_BASE_ADDR_LOW_WRITE(v, p)                         MWRITE_32((uint8_t *)p + 8, v)
#define RDD_CPU_RING_DESCRIPTOR_READ_IDX_READ_G(r, g, idx)                        GROUP_MREAD_16(g, idx*sizeof(RDD_CPU_RING_DESCRIPTOR_DTS) + 12, r)
#define RDD_CPU_RING_DESCRIPTOR_READ_IDX_WRITE_G(v, g, idx)                       GROUP_MWRITE_16(g, idx*sizeof(RDD_CPU_RING_DESCRIPTOR_DTS) + 12, v)
#define RDD_CPU_RING_DESCRIPTOR_READ_IDX_READ(r, p)                               MREAD_16((uint8_t *)p + 12, r)
#define RDD_CPU_RING_DESCRIPTOR_READ_IDX_WRITE(v, p)                              MWRITE_16((uint8_t *)p + 12, v)
#define RDD_CPU_RING_DESCRIPTOR_LOW_PRIORITY_THRESHOLD_READ_G(r, g, idx)          GROUP_MREAD_8(g, idx*sizeof(RDD_CPU_RING_DESCRIPTOR_DTS) + 14, r)
#define RDD_CPU_RING_DESCRIPTOR_LOW_PRIORITY_THRESHOLD_WRITE_G(v, g, idx)         GROUP_MWRITE_8(g, idx*sizeof(RDD_CPU_RING_DESCRIPTOR_DTS) + 14, v)
#define RDD_CPU_RING_DESCRIPTOR_LOW_PRIORITY_THRESHOLD_READ(r, p)                 MREAD_8((uint8_t *)p + 14, r)
#define RDD_CPU_RING_DESCRIPTOR_LOW_PRIORITY_THRESHOLD_WRITE(v, p)                MWRITE_8((uint8_t *)p + 14, v)
#define RDD_CPU_RING_DESCRIPTOR_BASE_ADDR_HIGH_READ_G(r, g, idx)                  GROUP_MREAD_8(g, idx*sizeof(RDD_CPU_RING_DESCRIPTOR_DTS) + 15, r)
#define RDD_CPU_RING_DESCRIPTOR_BASE_ADDR_HIGH_WRITE_G(v, g, idx)                 GROUP_MWRITE_8(g, idx*sizeof(RDD_CPU_RING_DESCRIPTOR_DTS) + 15, v)
#define RDD_CPU_RING_DESCRIPTOR_BASE_ADDR_HIGH_READ(r, p)                         MREAD_8((uint8_t *)p + 15, r)
#define RDD_CPU_RING_DESCRIPTOR_BASE_ADDR_HIGH_WRITE(v, p)                        MWRITE_8((uint8_t *)p + 15, v)
/* <<<RDD_CPU_RING_DESCRIPTOR_DTS */

typedef struct
{
	RDD_CPU_RING_DESCRIPTOR_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_DTS;

extern uint32_t RDD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_PTR(core_id)	( RDD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_RECYCLE_RING_DESCRIPTOR_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_RECYCLE_RING_DESCRIPTOR_TABLE */
/* >>>CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE */

#define RDD_CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_SIZE     2
typedef struct
{
	RDD_DDR_ADDRESS_DTS	entry[ RDD_CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_DTS;

extern uint32_t RDD_CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_PTR(core_id)	( RDD_CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_RECYCLE_RING_INDEX_DDR_ADDR_TABLE */
/* >>>CPU_INTERRUPT_COALESCING_TABLE */
typedef struct
{
	RDD_CPU_INTERRUPT_COALESCING_ENTRY_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_INTERRUPT_COALESCING_TABLE_DTS;

extern uint32_t RDD_CPU_INTERRUPT_COALESCING_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_INTERRUPT_COALESCING_TABLE_PTR(core_id)	( RDD_CPU_INTERRUPT_COALESCING_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_INTERRUPT_COALESCING_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_INTERRUPT_COALESCING_TABLE */
/* >>>CPU_REASON_TO_TC */

#define RDD_CPU_REASON_TO_TC_SIZE     64
typedef struct
{
	RDD_BYTE_1_DTS	entry[ RDD_CPU_REASON_TO_TC_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_REASON_TO_TC_DTS;

extern uint32_t RDD_CPU_REASON_TO_TC_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_REASON_TO_TC_PTR(core_id)	( RDD_CPU_REASON_TO_TC_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_REASON_TO_TC_ADDRESS_ARR[core_id] ))

/* <<<CPU_REASON_TO_TC */
/* >>>TC_TO_CPU_RXQ */

#define RDD_TC_TO_CPU_RXQ_SIZE     64
typedef struct
{
	RDD_BYTE_1_DTS	entry[ RDD_TC_TO_CPU_RXQ_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_TC_TO_CPU_RXQ_DTS;

extern uint32_t RDD_TC_TO_CPU_RXQ_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_TC_TO_CPU_RXQ_PTR(core_id)	( RDD_TC_TO_CPU_RXQ_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_TC_TO_CPU_RXQ_ADDRESS_ARR[core_id] ))

/* <<<TC_TO_CPU_RXQ */
/* >>>EXC_TC_TO_CPU_RXQ */

#define RDD_EXC_TC_TO_CPU_RXQ_SIZE     64
typedef struct
{
	RDD_BYTE_1_DTS	entry[ RDD_EXC_TC_TO_CPU_RXQ_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_EXC_TC_TO_CPU_RXQ_DTS;

extern uint32_t RDD_EXC_TC_TO_CPU_RXQ_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_EXC_TC_TO_CPU_RXQ_PTR(core_id)	( RDD_EXC_TC_TO_CPU_RXQ_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_EXC_TC_TO_CPU_RXQ_ADDRESS_ARR[core_id] ))

/* <<<EXC_TC_TO_CPU_RXQ */
/* >>>CPU_FEED_RING_DESCRIPTOR_TABLE */
typedef struct
{
	RDD_CPU_RING_DESCRIPTOR_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_DTS;

extern uint32_t RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_PTR(core_id)	( RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_FEED_RING_DESCRIPTOR_TABLE */
/* >>>VPORT_TO_CPU_OBJ */

#define RDD_VPORT_TO_CPU_OBJ_SIZE     40
typedef struct
{
	RDD_BYTE_1_DTS	entry[ RDD_VPORT_TO_CPU_OBJ_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_VPORT_TO_CPU_OBJ_DTS;

extern uint32_t RDD_VPORT_TO_CPU_OBJ_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_VPORT_TO_CPU_OBJ_PTR(core_id)	( RDD_VPORT_TO_CPU_OBJ_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_VPORT_TO_CPU_OBJ_ADDRESS_ARR[core_id] ))

/* <<<VPORT_TO_CPU_OBJ */
/* >>>CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE */
typedef struct
{
	RDD_CPU_RING_INTERRUPT_COUNTER_ENTRY_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_DTS;

extern uint32_t RDD_CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_PTR(core_id)	( RDD_CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_RECYCLE_RING_INTERRUPT_COUNTER_TABLE */
/* >>>CPU_FEED_RING_INTERRUPT_THRESHOLD */
typedef struct
{
	RDD_BYTES_2_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_FEED_RING_INTERRUPT_THRESHOLD_DTS;

extern uint32_t RDD_CPU_FEED_RING_INTERRUPT_THRESHOLD_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_FEED_RING_INTERRUPT_THRESHOLD_PTR(core_id)	( RDD_CPU_FEED_RING_INTERRUPT_THRESHOLD_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_FEED_RING_INTERRUPT_THRESHOLD_ADDRESS_ARR[core_id] ))

/* <<<CPU_FEED_RING_INTERRUPT_THRESHOLD */
/* >>>CPU_RING_DESCRIPTORS_TABLE */

#define RDD_CPU_RING_DESCRIPTORS_TABLE_SIZE     16
typedef struct
{
	RDD_CPU_RING_DESCRIPTOR_DTS	entry[ RDD_CPU_RING_DESCRIPTORS_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RING_DESCRIPTORS_TABLE_DTS;

extern uint32_t RDD_CPU_RING_DESCRIPTORS_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_RING_DESCRIPTORS_TABLE_PTR(core_id)	( RDD_CPU_RING_DESCRIPTORS_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_RING_DESCRIPTORS_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_RING_DESCRIPTORS_TABLE */
/* >>>CPU_TX_RING_TABLE */

/* >>>RDD_CPU_TX_DESCRIPTOR_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	valid               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	first_level_q       	:9	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs_data1           	:22	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs_data0           	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	packet_length       	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	drop                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	flag_1588           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	color               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	do_not_recycle      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	lan                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wan_flow_source_port	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bn1_or_abs2_or_1588 	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = bn1_or_abs2_or_1588, size = 18 bits
	uint32_t	ssid                	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	fpm_fallback        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sbpm_copy           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bn1_first           	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	_ssid               	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	_fpm_fallback       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	_sbpm_copy          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs2                	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	data_1588           	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	agg_pd              	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	target_mem_0        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	payload_offset_or_abs_1	:11	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = payload_offset_or_abs_1, size = 11 bits
	uint32_t	sop                 	:11	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs1                	:11	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	buffer_number_0_or_abs_0	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = buffer_number_0_or_abs_0, size = 18 bits
	uint32_t	bn0_first           	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs0                	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
#else
	uint32_t	abs_data1           	:22	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	first_level_q       	:9	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	valid               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	packet_length       	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs_data0           	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bn1_or_abs2_or_1588 	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = bn1_or_abs2_or_1588, size = 18 bits
	uint32_t	ssid                	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	fpm_fallback        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sbpm_copy           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bn1_first           	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	_ssid               	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	_fpm_fallback       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	_sbpm_copy          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs2                	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	data_1588           	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	wan_flow_source_port	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	lan                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	do_not_recycle      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	color               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	flag_1588           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	drop                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	buffer_number_0_or_abs_0	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = buffer_number_0_or_abs_0, size = 18 bits
	uint32_t	bn0_first           	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs0                	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	payload_offset_or_abs_1	:11	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = payload_offset_or_abs_1, size = 11 bits
	uint32_t	sop                 	:11	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs1                	:11	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	abs                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	target_mem_0        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	agg_pd              	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_DESCRIPTOR_DTS;

#define RDD_CPU_TX_DESCRIPTOR_VALID_READ_G(r, g, idx)                         GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS), 7, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_VALID_WRITE_G(v, g, idx)                        GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS), 7, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_VALID_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_VALID_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_FIRST_LEVEL_Q_READ_G(r, g, idx)                 GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS), 6, 9, r)
#define RDD_CPU_TX_DESCRIPTOR_FIRST_LEVEL_Q_WRITE_G(v, g, idx)                GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS), 6, 9, v)
#define RDD_CPU_TX_DESCRIPTOR_FIRST_LEVEL_Q_READ(r, p)                        FIELD_MREAD_16((uint8_t *)p, 6, 9, r)
#define RDD_CPU_TX_DESCRIPTOR_FIRST_LEVEL_Q_WRITE(v, p)                       FIELD_MWRITE_16((uint8_t *)p, 6, 9, v)
#define RDD_CPU_TX_DESCRIPTOR_ABS_DATA1_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 0, 0, 22, r)
#define RDD_CPU_TX_DESCRIPTOR_ABS_DATA1_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 0, 0, 22, v)
#define RDD_CPU_TX_DESCRIPTOR_ABS_DATA1_READ(r, p)                            FIELD_MREAD_32((uint8_t *)p + 0, 0, 22, r)
#define RDD_CPU_TX_DESCRIPTOR_ABS_DATA1_WRITE(v, p)                           FIELD_MWRITE_32((uint8_t *)p + 0, 0, 22, v)
#define RDD_CPU_TX_DESCRIPTOR_ABS_DATA0_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 4, 14, 18, r)
#define RDD_CPU_TX_DESCRIPTOR_ABS_DATA0_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 4, 14, 18, v)
#define RDD_CPU_TX_DESCRIPTOR_ABS_DATA0_READ(r, p)                            FIELD_MREAD_32((uint8_t *)p + 4, 14, 18, r)
#define RDD_CPU_TX_DESCRIPTOR_ABS_DATA0_WRITE(v, p)                           FIELD_MWRITE_32((uint8_t *)p + 4, 14, 18, v)
#define RDD_CPU_TX_DESCRIPTOR_PACKET_LENGTH_READ_G(r, g, idx)                 GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 6, 0, 14, r)
#define RDD_CPU_TX_DESCRIPTOR_PACKET_LENGTH_WRITE_G(v, g, idx)                GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 6, 0, 14, v)
#define RDD_CPU_TX_DESCRIPTOR_PACKET_LENGTH_READ(r, p)                        FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r)
#define RDD_CPU_TX_DESCRIPTOR_PACKET_LENGTH_WRITE(v, p)                       FIELD_MWRITE_16((uint8_t *)p + 6, 0, 14, v)
#define RDD_CPU_TX_DESCRIPTOR_DROP_READ_G(r, g, idx)                          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 8, 7, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_DROP_WRITE_G(v, g, idx)                         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 8, 7, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_DROP_READ(r, p)                                 FIELD_MREAD_8((uint8_t *)p + 8, 7, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_DROP_WRITE(v, p)                                FIELD_MWRITE_8((uint8_t *)p + 8, 7, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_FLAG_1588_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 8, 6, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_FLAG_1588_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 8, 6, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_FLAG_1588_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 8, 6, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_FLAG_1588_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 8, 6, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_COLOR_READ_G(r, g, idx)                         GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 8, 5, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_COLOR_WRITE_G(v, g, idx)                        GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 8, 5, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_COLOR_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 8, 5, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_COLOR_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 8, 5, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_DO_NOT_RECYCLE_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 8, 4, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_DO_NOT_RECYCLE_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 8, 4, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_DO_NOT_RECYCLE_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 8, 4, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_DO_NOT_RECYCLE_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 8, 4, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_LAN_READ_G(r, g, idx)                           GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 8, 2, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_LAN_WRITE_G(v, g, idx)                          GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 8, 2, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_LAN_READ(r, p)                                  FIELD_MREAD_8((uint8_t *)p + 8, 2, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_LAN_WRITE(v, p)                                 FIELD_MWRITE_8((uint8_t *)p + 8, 2, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_WAN_FLOW_SOURCE_PORT_READ_G(r, g, idx)          GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 8, 2, 8, r)
#define RDD_CPU_TX_DESCRIPTOR_WAN_FLOW_SOURCE_PORT_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 8, 2, 8, v)
#define RDD_CPU_TX_DESCRIPTOR_WAN_FLOW_SOURCE_PORT_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p + 8, 2, 8, r)
#define RDD_CPU_TX_DESCRIPTOR_WAN_FLOW_SOURCE_PORT_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p + 8, 2, 8, v)
#define RDD_CPU_TX_DESCRIPTOR_BN1_OR_ABS2_OR_1588_READ_G(r, g, idx)           GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 8, 0, 18, r)
#define RDD_CPU_TX_DESCRIPTOR_BN1_OR_ABS2_OR_1588_WRITE_G(v, g, idx)          GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 8, 0, 18, v)
#define RDD_CPU_TX_DESCRIPTOR_BN1_OR_ABS2_OR_1588_READ(r, p)                  FIELD_MREAD_32((uint8_t *)p + 8, 0, 18, r)
#define RDD_CPU_TX_DESCRIPTOR_BN1_OR_ABS2_OR_1588_WRITE(v, p)                 FIELD_MWRITE_32((uint8_t *)p + 8, 0, 18, v)
#define RDD_CPU_TX_DESCRIPTOR_SSID_READ_G(r, g, idx)                          GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 8, 14, 4, r)
#define RDD_CPU_TX_DESCRIPTOR_SSID_WRITE_G(v, g, idx)                         GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 8, 14, 4, v)
#define RDD_CPU_TX_DESCRIPTOR_SSID_READ(r, p)                                 FIELD_MREAD_32((uint8_t *)p + 8, 14, 4, r)
#define RDD_CPU_TX_DESCRIPTOR_SSID_WRITE(v, p)                                FIELD_MWRITE_32((uint8_t *)p + 8, 14, 4, v)
#define RDD_CPU_TX_DESCRIPTOR_FPM_FALLBACK_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 10, 5, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_FPM_FALLBACK_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 10, 5, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_FPM_FALLBACK_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 10, 5, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_FPM_FALLBACK_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 10, 5, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_SBPM_COPY_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 10, 4, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_SBPM_COPY_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 10, 4, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_SBPM_COPY_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 10, 4, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_SBPM_COPY_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 10, 4, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_BN1_FIRST_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 10, 0, 12, r)
#define RDD_CPU_TX_DESCRIPTOR_BN1_FIRST_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 10, 0, 12, v)
#define RDD_CPU_TX_DESCRIPTOR_BN1_FIRST_READ(r, p)                            FIELD_MREAD_16((uint8_t *)p + 10, 0, 12, r)
#define RDD_CPU_TX_DESCRIPTOR_BN1_FIRST_WRITE(v, p)                           FIELD_MWRITE_16((uint8_t *)p + 10, 0, 12, v)
#define RDD_CPU_TX_DESCRIPTOR__SSID_READ_G(r, g, idx)                         GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 8, 14, 4, r)
#define RDD_CPU_TX_DESCRIPTOR__SSID_WRITE_G(v, g, idx)                        GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 8, 14, 4, v)
#define RDD_CPU_TX_DESCRIPTOR__SSID_READ(r, p)                                FIELD_MREAD_32((uint8_t *)p + 8, 14, 4, r)
#define RDD_CPU_TX_DESCRIPTOR__SSID_WRITE(v, p)                               FIELD_MWRITE_32((uint8_t *)p + 8, 14, 4, v)
#define RDD_CPU_TX_DESCRIPTOR__FPM_FALLBACK_READ_G(r, g, idx)                 GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 10, 5, 1, r)
#define RDD_CPU_TX_DESCRIPTOR__FPM_FALLBACK_WRITE_G(v, g, idx)                GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 10, 5, 1, v)
#define RDD_CPU_TX_DESCRIPTOR__FPM_FALLBACK_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 10, 5, 1, r)
#define RDD_CPU_TX_DESCRIPTOR__FPM_FALLBACK_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 10, 5, 1, v)
#define RDD_CPU_TX_DESCRIPTOR__SBPM_COPY_READ_G(r, g, idx)                    GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 10, 4, 1, r)
#define RDD_CPU_TX_DESCRIPTOR__SBPM_COPY_WRITE_G(v, g, idx)                   GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 10, 4, 1, v)
#define RDD_CPU_TX_DESCRIPTOR__SBPM_COPY_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 10, 4, 1, r)
#define RDD_CPU_TX_DESCRIPTOR__SBPM_COPY_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 10, 4, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_ABS2_READ_G(r, g, idx)                          GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 10, 0, 12, r)
#define RDD_CPU_TX_DESCRIPTOR_ABS2_WRITE_G(v, g, idx)                         GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 10, 0, 12, v)
#define RDD_CPU_TX_DESCRIPTOR_ABS2_READ(r, p)                                 FIELD_MREAD_16((uint8_t *)p + 10, 0, 12, r)
#define RDD_CPU_TX_DESCRIPTOR_ABS2_WRITE(v, p)                                FIELD_MWRITE_16((uint8_t *)p + 10, 0, 12, v)
#define RDD_CPU_TX_DESCRIPTOR_DATA_1588_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 8, 0, 18, r)
#define RDD_CPU_TX_DESCRIPTOR_DATA_1588_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 8, 0, 18, v)
#define RDD_CPU_TX_DESCRIPTOR_DATA_1588_READ(r, p)                            FIELD_MREAD_32((uint8_t *)p + 8, 0, 18, r)
#define RDD_CPU_TX_DESCRIPTOR_DATA_1588_WRITE(v, p)                           FIELD_MWRITE_32((uint8_t *)p + 8, 0, 18, v)
#define RDD_CPU_TX_DESCRIPTOR_AGG_PD_READ_G(r, g, idx)                        GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 12, 7, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_AGG_PD_WRITE_G(v, g, idx)                       GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 12, 7, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_AGG_PD_READ(r, p)                               FIELD_MREAD_8((uint8_t *)p + 12, 7, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_AGG_PD_WRITE(v, p)                              FIELD_MWRITE_8((uint8_t *)p + 12, 7, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_TARGET_MEM_0_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 12, 6, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_TARGET_MEM_0_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 12, 6, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_TARGET_MEM_0_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 12, 6, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_TARGET_MEM_0_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 12, 6, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_ABS_READ_G(r, g, idx)                           GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 12, 5, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_ABS_WRITE_G(v, g, idx)                          GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 12, 5, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_ABS_READ(r, p)                                  FIELD_MREAD_8((uint8_t *)p + 12, 5, 1, r)
#define RDD_CPU_TX_DESCRIPTOR_ABS_WRITE(v, p)                                 FIELD_MWRITE_8((uint8_t *)p + 12, 5, 1, v)
#define RDD_CPU_TX_DESCRIPTOR_PAYLOAD_OFFSET_OR_ABS_1_READ_G(r, g, idx)       GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 12, 2, 11, r)
#define RDD_CPU_TX_DESCRIPTOR_PAYLOAD_OFFSET_OR_ABS_1_WRITE_G(v, g, idx)      GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 12, 2, 11, v)
#define RDD_CPU_TX_DESCRIPTOR_PAYLOAD_OFFSET_OR_ABS_1_READ(r, p)              FIELD_MREAD_16((uint8_t *)p + 12, 2, 11, r)
#define RDD_CPU_TX_DESCRIPTOR_PAYLOAD_OFFSET_OR_ABS_1_WRITE(v, p)             FIELD_MWRITE_16((uint8_t *)p + 12, 2, 11, v)
#define RDD_CPU_TX_DESCRIPTOR_SOP_READ_G(r, g, idx)                           GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 12, 2, 11, r)
#define RDD_CPU_TX_DESCRIPTOR_SOP_WRITE_G(v, g, idx)                          GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 12, 2, 11, v)
#define RDD_CPU_TX_DESCRIPTOR_SOP_READ(r, p)                                  FIELD_MREAD_16((uint8_t *)p + 12, 2, 11, r)
#define RDD_CPU_TX_DESCRIPTOR_SOP_WRITE(v, p)                                 FIELD_MWRITE_16((uint8_t *)p + 12, 2, 11, v)
#define RDD_CPU_TX_DESCRIPTOR_ABS1_READ_G(r, g, idx)                          GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 12, 2, 11, r)
#define RDD_CPU_TX_DESCRIPTOR_ABS1_WRITE_G(v, g, idx)                         GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 12, 2, 11, v)
#define RDD_CPU_TX_DESCRIPTOR_ABS1_READ(r, p)                                 FIELD_MREAD_16((uint8_t *)p + 12, 2, 11, r)
#define RDD_CPU_TX_DESCRIPTOR_ABS1_WRITE(v, p)                                FIELD_MWRITE_16((uint8_t *)p + 12, 2, 11, v)
#define RDD_CPU_TX_DESCRIPTOR_BUFFER_NUMBER_0_OR_ABS_0_READ_G(r, g, idx)      GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 12, 0, 18, r)
#define RDD_CPU_TX_DESCRIPTOR_BUFFER_NUMBER_0_OR_ABS_0_WRITE_G(v, g, idx)     GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 12, 0, 18, v)
#define RDD_CPU_TX_DESCRIPTOR_BUFFER_NUMBER_0_OR_ABS_0_READ(r, p)             FIELD_MREAD_32((uint8_t *)p + 12, 0, 18, r)
#define RDD_CPU_TX_DESCRIPTOR_BUFFER_NUMBER_0_OR_ABS_0_WRITE(v, p)            FIELD_MWRITE_32((uint8_t *)p + 12, 0, 18, v)
#define RDD_CPU_TX_DESCRIPTOR_BN0_FIRST_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 12, 0, 18, r)
#define RDD_CPU_TX_DESCRIPTOR_BN0_FIRST_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 12, 0, 18, v)
#define RDD_CPU_TX_DESCRIPTOR_BN0_FIRST_READ(r, p)                            FIELD_MREAD_32((uint8_t *)p + 12, 0, 18, r)
#define RDD_CPU_TX_DESCRIPTOR_BN0_FIRST_WRITE(v, p)                           FIELD_MWRITE_32((uint8_t *)p + 12, 0, 18, v)
#define RDD_CPU_TX_DESCRIPTOR_ABS0_READ_G(r, g, idx)                          GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 12, 0, 18, r)
#define RDD_CPU_TX_DESCRIPTOR_ABS0_WRITE_G(v, g, idx)                         GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_CPU_TX_DESCRIPTOR_DTS) + 12, 0, 18, v)
#define RDD_CPU_TX_DESCRIPTOR_ABS0_READ(r, p)                                 FIELD_MREAD_32((uint8_t *)p + 12, 0, 18, r)
#define RDD_CPU_TX_DESCRIPTOR_ABS0_WRITE(v, p)                                FIELD_MWRITE_32((uint8_t *)p + 12, 0, 18, v)
/* <<<RDD_CPU_TX_DESCRIPTOR_DTS */


#define RDD_CPU_TX_RING_TABLE_SIZE     8
typedef struct
{
	RDD_CPU_TX_DESCRIPTOR_DTS	entry[ RDD_CPU_TX_RING_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_RING_TABLE_DTS;

extern uint32_t RDD_CPU_TX_RING_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_TX_RING_TABLE_PTR(core_id)	( RDD_CPU_TX_RING_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_TX_RING_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_TX_RING_TABLE */
/* >>>RDD_QM_QUEUE */
typedef enum
{
	QM_QUEUE_FIRST                    = 0,
	QM_QUEUE_US_START                 = 0,
	QM_QUEUE_US_EPON_START            = 128,
	QM_QUEUE_US_END                   = 159,
	QM_QUEUE_DS_START                 = 160,
	QM_QUEUE_DS_END                   = 255,
	QM_QUEUE_CPU_RX                   = 256,
	QM_QUEUE_CPU_TX_EGRESS            = 257,
	QM_QUEUE_CPU_TX_INGRESS           = 258,
	QM_QUEUE_DROP                     = 259,
	QM_QUEUE_CPU_RX_COPY_EXCLUSIVE    = 267,
	QM_QUEUE_CPU_RX_COPY_NORMAL       = 268,
	QM_QUEUE_TX_ABS_RECYCLE           = 269,
	QM_QUEUE_LAST                     = 269
} rdd_qm_queue;
/* <<<RDD_QM_QUEUE */
/* >>>RDD_CPU_IF_RDD */
typedef enum
{
	CPU_IF_RDD_FIRST      = 0,
	CPU_IF_RDD_DATA       = 0,
	CPU_IF_RDD_RECYCLE    = 1,
	CPU_IF_RDD_FEED       = 2,
	CPU_IF_RDD_LAST       = 2
} rdd_cpu_if_rdd;
/* <<<RDD_CPU_IF_RDD */
/* >>>RDD_COUNTER_CPU_GROUP */
typedef enum
{
	COUNTER_CPU_GROUP_FIRST            = 0,
	COUNTER_CPU_GROUP_SPINLOCK_FEED    = 0,
	COUNTER_CPU_GROUP_READ_PTR_FEED    = 1,
	COUNTER_CPU_GROUP_LAST             = 1
} rdd_counter_cpu_group;
/* <<<RDD_COUNTER_CPU_GROUP */
/* >>>RDD_COUNTER */
typedef enum
{
	COUNTER_FIRST                                   = 0,
	COUNTER_TM_PD_NOT_VALID_ID                      = 0,
	COUNTER_TM_ACTION_NOT_VALID_ID                  = 1,
	COUNTER_EPON_TM_PD_NOT_VALID_ID                 = 2,
	COUNTER_G9991_TM_PD_NOT_VALID_ID                = 3,
	COUNTER_PROCESSING_ACTION_NOT_VALID_ID          = 4,
	COUNTER_CPU_RECYCLE_RING_CONGESTION             = 5,
	COUNTER_CPU_RX_FEED_RING_CONGESTION             = 6,
	COUNTER_CPU_RX_PSRAM_DROP                       = 7,
	COUNTER_IPTV_HASH_LKP_MISS_DROP                 = 8,
	COUNTER_IPTV_SIP_VID_LKP_MISS_DROP              = 9,
	COUNTER_IPTV_INVALID_CTX_ENTRY_DROP             = 10,
	COUNTER_IPTV_FPM_ALLOC_NACK_DROP                = 11,
	COUNTER_IPTV_FIRST_REPL_DISP_ALLOC_NACK_DROP    = 12,
	COUNTER_IPTV_EXCEPTION_DROP                     = 13,
	COUNTER_IPTV_OTHER_REPL_DISP_ALLOC_NACK_DROP    = 14,
	COUNTER_CPU_TX_COPY_NO_FPM                      = 15,
	COUNTER_CPU_TX_COPY_NO_SBPM                     = 16,
	COUNTER_CPU_RX_TC_TO_RXQ_MAP_DROP               = 17,
	COUNTER_CPU_RX_VPORT_TO_CPU_OBJ_MAP_DROP        = 18,
	COUNTER_ETHERNET_FLOW_DROP_ACTION               = 19,
	COUNTER_SBPM_ALLOC_EXCEPTION_DROP               = 20,
	COUNTER_DROP_CONNECTION_ACTION_DROP_ID          = 21,
	COUNTER_DROP_RESOURCE_CONGESTION                = 22,
	COUNTER_INGRESS_FILTER_DROP_FIRST_DS            = 23,
	COUNTER_INGRESS_FILTER_DROP_FIRST_US            = 44,
	COUNTER_INGRESS_FILTER_DROP_LAST                = 64,
	COUNTER_CPU_RX_METER_DROP                       = 65,
	COUNTER_INGRESS_ISOLATION_DROP                  = 66,
	COUNTER_EGRESS_ISOLATION_DROP                   = 67,
	COUNTER_DA_LKP_MISS_DROP                        = 68,
	COUNTER_SA_LKP_MISS_DROP                        = 69,
	COUNTER_BRIDGE_FW_ELIGABILITY_DROP              = 70,
	COUNTER_DA_LKP_MATCH_DROP                       = 71,
	COUNTER_SA_LKP_MATCH_DROP                       = 72,
	COUNTER_DROP_DISABLED_QUEUE                     = 73,
	COUNTER_DROP_DISABLED_TX_FLOW                   = 74,
	COUNTER_CPU_FEED_RING_DISP_CONGESTION           = 75,
	COUNTER_SBPM_LIB_DISP_CONG                      = 76,
	COUNTER_BRIDGE_FLOODING                         = 77,
	COUNTER_LAST_NON_DHD                            = 77,
	COUNTER_LAST                                    = 77
} rdd_counter;
/* <<<RDD_COUNTER */
/* >>>RDD_FLOW_DEST */
typedef enum
{
	FLOW_DEST_FIRST      = 0,
	FLOW_DEST_ETH_ID     = 0,
	FLOW_DEST_IPTV_ID    = 1,
	FLOW_DEST_LAST       = 1
} rdd_flow_dest;
/* <<<RDD_FLOW_DEST */
/* >>>RDD_RDD_VPORT */
typedef enum
{
	RDD_VPORT_FIRST     = 0,
	RDD_VPORT_ID_0      = 0,
	RDD_VPORT_ID_1      = 1,
	RDD_VPORT_ID_2      = 2,
	RDD_VPORT_ID_3      = 3,
	RDD_VPORT_ID_4      = 4,
	RDD_VPORT_ID_5      = 5,
	RDD_VPORT_ID_6      = 6,
	RDD_VPORT_ID_7      = 7,
	RDD_VPORT_ID_8      = 8,
	RDD_VPORT_ID_9      = 9,
	RDD_VPORT_ID_10     = 10,
	RDD_VPORT_ID_11     = 11,
	RDD_VPORT_ID_12     = 12,
	RDD_VPORT_ID_13     = 13,
	RDD_VPORT_ID_14     = 14,
	RDD_VPORT_ID_15     = 15,
	RDD_VPORT_ID_16     = 16,
	RDD_VPORT_ID_17     = 17,
	RDD_VPORT_ID_18     = 18,
	RDD_VPORT_ID_19     = 19,
	RDD_VPORT_ID_20     = 20,
	RDD_VPORT_ID_21     = 21,
	RDD_VPORT_ID_22     = 22,
	RDD_VPORT_ID_23     = 23,
	RDD_VPORT_ID_24     = 24,
	RDD_VPORT_ID_25     = 25,
	RDD_VPORT_ID_26     = 26,
	RDD_VPORT_ID_27     = 27,
	RDD_VPORT_ID_28     = 28,
	RDD_VPORT_ID_29     = 29,
	RDD_VPORT_ID_30     = 30,
	RDD_VPORT_ID_31     = 31,
	RDD_VPORT_ID_32     = 32,
	RDD_VPORT_ID_33     = 33,
	RDD_VPORT_ID_34     = 34,
	RDD_VPORT_ID_35     = 35,
	RDD_VPORT_ID_36     = 36,
	RDD_VPORT_ID_37     = 37,
	RDD_VPORT_ID_38     = 38,
	RDD_VPORT_ID_39     = 39,
	RDD_VPORT_ID_ANY    = 40,
	RDD_VPORT_LAST      = 40
} rdd_rdd_vport;
/* <<<RDD_RDD_VPORT */
/* >>>RDD_CPU_RX_METER */
typedef enum
{
	CPU_RX_METER_FIRST      = 0,
	CPU_RX_METER_ID_0       = 0,
	CPU_RX_METER_ID_1       = 1,
	CPU_RX_METER_ID_2       = 2,
	CPU_RX_METER_ID_3       = 3,
	CPU_RX_METER_ID_4       = 4,
	CPU_RX_METER_ID_5       = 5,
	CPU_RX_METER_ID_6       = 6,
	CPU_RX_METER_ID_7       = 7,
	CPU_RX_METER_ID_8       = 8,
	CPU_RX_METER_ID_9       = 9,
	CPU_RX_METER_ID_10      = 10,
	CPU_RX_METER_ID_11      = 11,
	CPU_RX_METER_ID_12      = 12,
	CPU_RX_METER_ID_13      = 13,
	CPU_RX_METER_ID_14      = 14,
	CPU_RX_METER_ID_15      = 15,
	CPU_RX_METER_DISABLE    = 255,
	CPU_RX_METER_LAST       = 255
} rdd_cpu_rx_meter;
/* <<<RDD_CPU_RX_METER */
/* >>>RDD_MAC_TYPE */
typedef enum
{
	MAC_TYPE_FIRST    = 0,
	MAC_TYPE_EMAC     = 0,
	MAC_TYPE_GPON     = 1,
	MAC_TYPE_XGPON    = 2,
	MAC_TYPE_EPON     = 3,
	MAC_TYPE_XEPON    = 4,
	MAC_TYPE_DSL      = 5,
	MAC_TYPE_AE10G    = 6,
	MAC_TYPE_AE2P5    = 7,
	MAC_TYPE_LAST     = 7
} rdd_mac_type;
/* <<<RDD_MAC_TYPE */
/* >>>RDD_CPU_RX_DESCRIPTOR */
#define CPU_RX_DESCRIPTOR_CPU_RX_DATA_PTR0_UNION_F_OFFSET              0
#define CPU_RX_DESCRIPTOR_CPU_RX_DATA_PTR0_UNION_F_WIDTH               32
#define CPU_RX_DESCRIPTOR_CPU_RX_DATA_PTR0_UNION_OFFSET                0
#define CPU_RX_DESCRIPTOR_HOST_BUFFER_DATA_PTR_HI_F_OFFSET             24
#define CPU_RX_DESCRIPTOR_HOST_BUFFER_DATA_PTR_HI_F_WIDTH              8
#define CPU_RX_DESCRIPTOR_HOST_BUFFER_DATA_PTR_HI_OFFSET               4
#define CPU_RX_DESCRIPTOR_HOST_BUFFER_DATA_PTR_HI_F_OFFSET_MOD16       8
#define CPU_RX_DESCRIPTOR_ABS_F_OFFSET                                 16
#define CPU_RX_DESCRIPTOR_ABS_F_WIDTH                                  1
#define CPU_RX_DESCRIPTOR_ABS_OFFSET                                   5
#define CPU_RX_DESCRIPTOR_ABS_F_OFFSET_MOD8                            0
#define CPU_RX_DESCRIPTOR_ABS_F_OFFSET_MOD16                           0
#define CPU_RX_DESCRIPTOR_PLEN_F_OFFSET                                2
#define CPU_RX_DESCRIPTOR_PLEN_F_WIDTH                                 14
#define CPU_RX_DESCRIPTOR_PLEN_OFFSET                                  6
#define CPU_RX_DESCRIPTOR_IS_CHKSUM_VERIFIED_F_OFFSET                  1
#define CPU_RX_DESCRIPTOR_IS_CHKSUM_VERIFIED_F_WIDTH                   1
#define CPU_RX_DESCRIPTOR_IS_CHKSUM_VERIFIED_OFFSET                    7
#define CPU_RX_DESCRIPTOR_IS_SRC_LAN_F_OFFSET                          31
#define CPU_RX_DESCRIPTOR_IS_SRC_LAN_F_WIDTH                           1
#define CPU_RX_DESCRIPTOR_IS_SRC_LAN_OFFSET                            8
#define CPU_RX_DESCRIPTOR_IS_SRC_LAN_F_OFFSET_MOD8                     7
#define CPU_RX_DESCRIPTOR_IS_SRC_LAN_F_OFFSET_MOD16                    15
#define CPU_RX_DESCRIPTOR_VPORT_F_OFFSET                               25
#define CPU_RX_DESCRIPTOR_VPORT_F_WIDTH                                5
#define CPU_RX_DESCRIPTOR_VPORT_OFFSET                                 8
#define CPU_RX_DESCRIPTOR_VPORT_F_OFFSET_MOD8                          1
#define CPU_RX_DESCRIPTOR_VPORT_F_OFFSET_MOD16                         9
#define CPU_RX_DESCRIPTOR_CPU_RX_SRC_UNION_F_OFFSET                    13
#define CPU_RX_DESCRIPTOR_CPU_RX_SRC_UNION_F_WIDTH                     12
#define CPU_RX_DESCRIPTOR_CPU_RX_SRC_UNION_OFFSET                      8
#define CPU_RX_DESCRIPTOR_DATA_OFFSET_F_OFFSET                         6
#define CPU_RX_DESCRIPTOR_DATA_OFFSET_F_WIDTH                          7
#define CPU_RX_DESCRIPTOR_DATA_OFFSET_OFFSET                           10
#define CPU_RX_DESCRIPTOR_REASON_F_OFFSET                              0
#define CPU_RX_DESCRIPTOR_REASON_F_WIDTH                               6
#define CPU_RX_DESCRIPTOR_REASON_OFFSET                                11
#define CPU_RX_DESCRIPTOR_IS_EXCEPTION_F_OFFSET                        31
#define CPU_RX_DESCRIPTOR_IS_EXCEPTION_F_WIDTH                         1
#define CPU_RX_DESCRIPTOR_IS_EXCEPTION_OFFSET                          12
#define CPU_RX_DESCRIPTOR_IS_EXCEPTION_F_OFFSET_MOD8                   7
#define CPU_RX_DESCRIPTOR_IS_EXCEPTION_F_OFFSET_MOD16                  15
#define CPU_RX_DESCRIPTOR_IS_RX_OFFLOAD_F_OFFSET                       30
#define CPU_RX_DESCRIPTOR_IS_RX_OFFLOAD_F_WIDTH                        1
#define CPU_RX_DESCRIPTOR_IS_RX_OFFLOAD_OFFSET                         12
#define CPU_RX_DESCRIPTOR_IS_RX_OFFLOAD_F_OFFSET_MOD8                  6
#define CPU_RX_DESCRIPTOR_IS_RX_OFFLOAD_F_OFFSET_MOD16                 14
#define CPU_RX_DESCRIPTOR_IS_UCAST_F_OFFSET                            29
#define CPU_RX_DESCRIPTOR_IS_UCAST_F_WIDTH                             1
#define CPU_RX_DESCRIPTOR_IS_UCAST_OFFSET                              12
#define CPU_RX_DESCRIPTOR_IS_UCAST_F_OFFSET_MOD8                       5
#define CPU_RX_DESCRIPTOR_IS_UCAST_F_OFFSET_MOD16                      13
#define CPU_RX_DESCRIPTOR_MCAST_TX_PRIO_UNION_F_OFFSET                 16
#define CPU_RX_DESCRIPTOR_MCAST_TX_PRIO_UNION_F_WIDTH                  13
#define CPU_RX_DESCRIPTOR_MCAST_TX_PRIO_UNION_OFFSET                   12
#define CPU_RX_DESCRIPTOR_MCAST_TX_PRIO_UNION_F_OFFSET_MOD16           0
#define CPU_RX_DESCRIPTOR_CPU_RX_METADATA_UNION_F_OFFSET               0
#define CPU_RX_DESCRIPTOR_CPU_RX_METADATA_UNION_F_WIDTH                16
#define CPU_RX_DESCRIPTOR_CPU_RX_METADATA_UNION_OFFSET                 14

/* >>>RDD_CPU_RX_DESCRIPTOR_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	cpu_rx_data_ptr0_union  	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = cpu_rx_data_ptr0_union, size = 32 bits
	uint32_t	host_buffer_data_ptr_low	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	fpm_idx                 	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0               	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	host_buffer_data_ptr_hi 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1               	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs                     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	plen                    	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	is_chksum_verified      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	is_src_lan              	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved7               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vport                   	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cpu_rx_src_union        	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = cpu_rx_src_union, size = 12 bits
	uint32_t	wan_flow_id             	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved3               	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ssid                    	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved4               	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	data_offset             	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reason                  	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	is_exception            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	is_rx_offload           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	is_ucast                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mcast_tx_prio_union     	:13	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = mcast_tx_prio_union, size = 13 bits
	uint32_t	mcast_tx_prio           	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved5               	:10	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved6               	:13	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	cpu_rx_metadata_union   	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = cpu_rx_metadata_union, size = 16 bits
	uint32_t	dst_ssid_vector         	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved6               	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	metadata_0              	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	metadata_1              	:10	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_queue            	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wan_flow                	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
#else
	uint32_t	cpu_rx_data_ptr0_union  	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = cpu_rx_data_ptr0_union, size = 32 bits
	uint32_t	host_buffer_data_ptr_low	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	fpm_idx                 	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0               	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	reserved2               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	is_chksum_verified      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	plen                    	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs                     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1               	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	host_buffer_data_ptr_hi 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reason                  	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	data_offset             	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cpu_rx_src_union        	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = cpu_rx_src_union, size = 12 bits
	uint32_t	wan_flow_id             	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved3               	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ssid                    	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved4               	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	vport                   	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved7               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	is_src_lan              	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cpu_rx_metadata_union   	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = cpu_rx_metadata_union, size = 16 bits
	uint32_t	dst_ssid_vector         	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved6               	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	metadata_0              	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	metadata_1              	:10	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_queue            	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	wan_flow                	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	mcast_tx_prio_union     	:13	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = mcast_tx_prio_union, size = 13 bits
	uint32_t	mcast_tx_prio           	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved5               	:10	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved6               	:13	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	is_ucast                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	is_rx_offload           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	is_exception            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_DESCRIPTOR_DTS;

#define RDD_CPU_RX_DESCRIPTOR_CPU_RX_DATA_PTR0_UNION_READ_G(r, g, idx)            GROUP_MREAD_32(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS), r)
#define RDD_CPU_RX_DESCRIPTOR_CPU_RX_DATA_PTR0_UNION_WRITE_G(v, g, idx)           GROUP_MWRITE_32(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS), v)
#define RDD_CPU_RX_DESCRIPTOR_CPU_RX_DATA_PTR0_UNION_READ(r, p)                   MREAD_32((uint8_t *)p, r)
#define RDD_CPU_RX_DESCRIPTOR_CPU_RX_DATA_PTR0_UNION_WRITE(v, p)                  MWRITE_32((uint8_t *)p, v)
#define RDD_CPU_RX_DESCRIPTOR_HOST_BUFFER_DATA_PTR_LOW_READ_G(r, g, idx)          GROUP_MREAD_32(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS), r)
#define RDD_CPU_RX_DESCRIPTOR_HOST_BUFFER_DATA_PTR_LOW_WRITE_G(v, g, idx)         GROUP_MWRITE_32(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS), v)
#define RDD_CPU_RX_DESCRIPTOR_HOST_BUFFER_DATA_PTR_LOW_READ(r, p)                 MREAD_32((uint8_t *)p, r)
#define RDD_CPU_RX_DESCRIPTOR_HOST_BUFFER_DATA_PTR_LOW_WRITE(v, p)                MWRITE_32((uint8_t *)p, v)
#define RDD_CPU_RX_DESCRIPTOR_FPM_IDX_READ_G(r, g, idx)                           GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS), 14, 18, r)
#define RDD_CPU_RX_DESCRIPTOR_FPM_IDX_WRITE_G(v, g, idx)                          GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS), 14, 18, v)
#define RDD_CPU_RX_DESCRIPTOR_FPM_IDX_READ(r, p)                                  FIELD_MREAD_32((uint8_t *)p, 14, 18, r)
#define RDD_CPU_RX_DESCRIPTOR_FPM_IDX_WRITE(v, p)                                 FIELD_MWRITE_32((uint8_t *)p, 14, 18, v)
#define RDD_CPU_RX_DESCRIPTOR_HOST_BUFFER_DATA_PTR_HI_READ_G(r, g, idx)           GROUP_MREAD_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 4, r)
#define RDD_CPU_RX_DESCRIPTOR_HOST_BUFFER_DATA_PTR_HI_WRITE_G(v, g, idx)          GROUP_MWRITE_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 4, v)
#define RDD_CPU_RX_DESCRIPTOR_HOST_BUFFER_DATA_PTR_HI_READ(r, p)                  MREAD_8((uint8_t *)p + 4, r)
#define RDD_CPU_RX_DESCRIPTOR_HOST_BUFFER_DATA_PTR_HI_WRITE(v, p)                 MWRITE_8((uint8_t *)p + 4, v)
#define RDD_CPU_RX_DESCRIPTOR_ABS_READ_G(r, g, idx)                               GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 5, 0, 1, r)
#define RDD_CPU_RX_DESCRIPTOR_ABS_WRITE_G(v, g, idx)                              GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 5, 0, 1, v)
#define RDD_CPU_RX_DESCRIPTOR_ABS_READ(r, p)                                      FIELD_MREAD_8((uint8_t *)p + 5, 0, 1, r)
#define RDD_CPU_RX_DESCRIPTOR_ABS_WRITE(v, p)                                     FIELD_MWRITE_8((uint8_t *)p + 5, 0, 1, v)
#define RDD_CPU_RX_DESCRIPTOR_PLEN_READ_G(r, g, idx)                              GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 6, 2, 14, r)
#define RDD_CPU_RX_DESCRIPTOR_PLEN_WRITE_G(v, g, idx)                             GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 6, 2, 14, v)
#define RDD_CPU_RX_DESCRIPTOR_PLEN_READ(r, p)                                     FIELD_MREAD_16((uint8_t *)p + 6, 2, 14, r)
#define RDD_CPU_RX_DESCRIPTOR_PLEN_WRITE(v, p)                                    FIELD_MWRITE_16((uint8_t *)p + 6, 2, 14, v)
#define RDD_CPU_RX_DESCRIPTOR_IS_CHKSUM_VERIFIED_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 7, 1, 1, r)
#define RDD_CPU_RX_DESCRIPTOR_IS_CHKSUM_VERIFIED_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 7, 1, 1, v)
#define RDD_CPU_RX_DESCRIPTOR_IS_CHKSUM_VERIFIED_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 7, 1, 1, r)
#define RDD_CPU_RX_DESCRIPTOR_IS_CHKSUM_VERIFIED_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 7, 1, 1, v)
#define RDD_CPU_RX_DESCRIPTOR_IS_SRC_LAN_READ_G(r, g, idx)                        GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 8, 7, 1, r)
#define RDD_CPU_RX_DESCRIPTOR_IS_SRC_LAN_WRITE_G(v, g, idx)                       GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 8, 7, 1, v)
#define RDD_CPU_RX_DESCRIPTOR_IS_SRC_LAN_READ(r, p)                               FIELD_MREAD_8((uint8_t *)p + 8, 7, 1, r)
#define RDD_CPU_RX_DESCRIPTOR_IS_SRC_LAN_WRITE(v, p)                              FIELD_MWRITE_8((uint8_t *)p + 8, 7, 1, v)
#define RDD_CPU_RX_DESCRIPTOR_VPORT_READ_G(r, g, idx)                             GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 8, 1, 5, r)
#define RDD_CPU_RX_DESCRIPTOR_VPORT_WRITE_G(v, g, idx)                            GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 8, 1, 5, v)
#define RDD_CPU_RX_DESCRIPTOR_VPORT_READ(r, p)                                    FIELD_MREAD_8((uint8_t *)p + 8, 1, 5, r)
#define RDD_CPU_RX_DESCRIPTOR_VPORT_WRITE(v, p)                                   FIELD_MWRITE_8((uint8_t *)p + 8, 1, 5, v)
#define RDD_CPU_RX_DESCRIPTOR_CPU_RX_SRC_UNION_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 8, 13, 12, r)
#define RDD_CPU_RX_DESCRIPTOR_CPU_RX_SRC_UNION_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 8, 13, 12, v)
#define RDD_CPU_RX_DESCRIPTOR_CPU_RX_SRC_UNION_READ(r, p)                         FIELD_MREAD_32((uint8_t *)p + 8, 13, 12, r)
#define RDD_CPU_RX_DESCRIPTOR_CPU_RX_SRC_UNION_WRITE(v, p)                        FIELD_MWRITE_32((uint8_t *)p + 8, 13, 12, v)
#define RDD_CPU_RX_DESCRIPTOR_WAN_FLOW_ID_READ_G(r, g, idx)                       GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 8, 13, 12, r)
#define RDD_CPU_RX_DESCRIPTOR_WAN_FLOW_ID_WRITE_G(v, g, idx)                      GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 8, 13, 12, v)
#define RDD_CPU_RX_DESCRIPTOR_WAN_FLOW_ID_READ(r, p)                              FIELD_MREAD_32((uint8_t *)p + 8, 13, 12, r)
#define RDD_CPU_RX_DESCRIPTOR_WAN_FLOW_ID_WRITE(v, p)                             FIELD_MWRITE_32((uint8_t *)p + 8, 13, 12, v)
#define RDD_CPU_RX_DESCRIPTOR_SSID_READ_G(r, g, idx)                              GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 8, 5, 4, r)
#define RDD_CPU_RX_DESCRIPTOR_SSID_WRITE_G(v, g, idx)                             GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 8, 5, 4, v)
#define RDD_CPU_RX_DESCRIPTOR_SSID_READ(r, p)                                     FIELD_MREAD_16((uint8_t *)p + 8, 5, 4, r)
#define RDD_CPU_RX_DESCRIPTOR_SSID_WRITE(v, p)                                    FIELD_MWRITE_16((uint8_t *)p + 8, 5, 4, v)
#define RDD_CPU_RX_DESCRIPTOR_DATA_OFFSET_READ_G(r, g, idx)                       GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 10, 6, 7, r)
#define RDD_CPU_RX_DESCRIPTOR_DATA_OFFSET_WRITE_G(v, g, idx)                      GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 10, 6, 7, v)
#define RDD_CPU_RX_DESCRIPTOR_DATA_OFFSET_READ(r, p)                              FIELD_MREAD_16((uint8_t *)p + 10, 6, 7, r)
#define RDD_CPU_RX_DESCRIPTOR_DATA_OFFSET_WRITE(v, p)                             FIELD_MWRITE_16((uint8_t *)p + 10, 6, 7, v)
#define RDD_CPU_RX_DESCRIPTOR_REASON_READ_G(r, g, idx)                            GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 11, 0, 6, r)
#define RDD_CPU_RX_DESCRIPTOR_REASON_WRITE_G(v, g, idx)                           GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 11, 0, 6, v)
#define RDD_CPU_RX_DESCRIPTOR_REASON_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p + 11, 0, 6, r)
#define RDD_CPU_RX_DESCRIPTOR_REASON_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p + 11, 0, 6, v)
#define RDD_CPU_RX_DESCRIPTOR_IS_EXCEPTION_READ_G(r, g, idx)                      GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 12, 7, 1, r)
#define RDD_CPU_RX_DESCRIPTOR_IS_EXCEPTION_WRITE_G(v, g, idx)                     GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 12, 7, 1, v)
#define RDD_CPU_RX_DESCRIPTOR_IS_EXCEPTION_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 12, 7, 1, r)
#define RDD_CPU_RX_DESCRIPTOR_IS_EXCEPTION_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 12, 7, 1, v)
#define RDD_CPU_RX_DESCRIPTOR_IS_RX_OFFLOAD_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 12, 6, 1, r)
#define RDD_CPU_RX_DESCRIPTOR_IS_RX_OFFLOAD_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 12, 6, 1, v)
#define RDD_CPU_RX_DESCRIPTOR_IS_RX_OFFLOAD_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 12, 6, 1, r)
#define RDD_CPU_RX_DESCRIPTOR_IS_RX_OFFLOAD_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 12, 6, 1, v)
#define RDD_CPU_RX_DESCRIPTOR_IS_UCAST_READ_G(r, g, idx)                          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 12, 5, 1, r)
#define RDD_CPU_RX_DESCRIPTOR_IS_UCAST_WRITE_G(v, g, idx)                         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 12, 5, 1, v)
#define RDD_CPU_RX_DESCRIPTOR_IS_UCAST_READ(r, p)                                 FIELD_MREAD_8((uint8_t *)p + 12, 5, 1, r)
#define RDD_CPU_RX_DESCRIPTOR_IS_UCAST_WRITE(v, p)                                FIELD_MWRITE_8((uint8_t *)p + 12, 5, 1, v)
#define RDD_CPU_RX_DESCRIPTOR_MCAST_TX_PRIO_UNION_READ_G(r, g, idx)               GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 12, 0, 13, r)
#define RDD_CPU_RX_DESCRIPTOR_MCAST_TX_PRIO_UNION_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 12, 0, 13, v)
#define RDD_CPU_RX_DESCRIPTOR_MCAST_TX_PRIO_UNION_READ(r, p)                      FIELD_MREAD_16((uint8_t *)p + 12, 0, 13, r)
#define RDD_CPU_RX_DESCRIPTOR_MCAST_TX_PRIO_UNION_WRITE(v, p)                     FIELD_MWRITE_16((uint8_t *)p + 12, 0, 13, v)
#define RDD_CPU_RX_DESCRIPTOR_MCAST_TX_PRIO_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 12, 2, 3, r)
#define RDD_CPU_RX_DESCRIPTOR_MCAST_TX_PRIO_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 12, 2, 3, v)
#define RDD_CPU_RX_DESCRIPTOR_MCAST_TX_PRIO_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 12, 2, 3, r)
#define RDD_CPU_RX_DESCRIPTOR_MCAST_TX_PRIO_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 12, 2, 3, v)
#define RDD_CPU_RX_DESCRIPTOR_CPU_RX_METADATA_UNION_READ_G(r, g, idx)             GROUP_MREAD_16(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 14, r)
#define RDD_CPU_RX_DESCRIPTOR_CPU_RX_METADATA_UNION_WRITE_G(v, g, idx)            GROUP_MWRITE_16(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 14, v)
#define RDD_CPU_RX_DESCRIPTOR_CPU_RX_METADATA_UNION_READ(r, p)                    MREAD_16((uint8_t *)p + 14, r)
#define RDD_CPU_RX_DESCRIPTOR_CPU_RX_METADATA_UNION_WRITE(v, p)                   MWRITE_16((uint8_t *)p + 14, v)
#define RDD_CPU_RX_DESCRIPTOR_DST_SSID_VECTOR_READ_G(r, g, idx)                   GROUP_MREAD_16(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 14, r)
#define RDD_CPU_RX_DESCRIPTOR_DST_SSID_VECTOR_WRITE_G(v, g, idx)                  GROUP_MWRITE_16(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 14, v)
#define RDD_CPU_RX_DESCRIPTOR_DST_SSID_VECTOR_READ(r, p)                          MREAD_16((uint8_t *)p + 14, r)
#define RDD_CPU_RX_DESCRIPTOR_DST_SSID_VECTOR_WRITE(v, p)                         MWRITE_16((uint8_t *)p + 14, v)
#define RDD_CPU_RX_DESCRIPTOR_METADATA_0_READ_G(r, g, idx)                        GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 14, 2, 4, r)
#define RDD_CPU_RX_DESCRIPTOR_METADATA_0_WRITE_G(v, g, idx)                       GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 14, 2, 4, v)
#define RDD_CPU_RX_DESCRIPTOR_METADATA_0_READ(r, p)                               FIELD_MREAD_8((uint8_t *)p + 14, 2, 4, r)
#define RDD_CPU_RX_DESCRIPTOR_METADATA_0_WRITE(v, p)                              FIELD_MWRITE_8((uint8_t *)p + 14, 2, 4, v)
#define RDD_CPU_RX_DESCRIPTOR_METADATA_1_READ_G(r, g, idx)                        GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 14, 0, 10, r)
#define RDD_CPU_RX_DESCRIPTOR_METADATA_1_WRITE_G(v, g, idx)                       GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 14, 0, 10, v)
#define RDD_CPU_RX_DESCRIPTOR_METADATA_1_READ(r, p)                               FIELD_MREAD_16((uint8_t *)p + 14, 0, 10, r)
#define RDD_CPU_RX_DESCRIPTOR_METADATA_1_WRITE(v, p)                              FIELD_MWRITE_16((uint8_t *)p + 14, 0, 10, v)
#define RDD_CPU_RX_DESCRIPTOR_EGRESS_QUEUE_READ_G(r, g, idx)                      GROUP_MREAD_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 14, r)
#define RDD_CPU_RX_DESCRIPTOR_EGRESS_QUEUE_WRITE_G(v, g, idx)                     GROUP_MWRITE_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 14, v)
#define RDD_CPU_RX_DESCRIPTOR_EGRESS_QUEUE_READ(r, p)                             MREAD_8((uint8_t *)p + 14, r)
#define RDD_CPU_RX_DESCRIPTOR_EGRESS_QUEUE_WRITE(v, p)                            MWRITE_8((uint8_t *)p + 14, v)
#define RDD_CPU_RX_DESCRIPTOR_WAN_FLOW_READ_G(r, g, idx)                          GROUP_MREAD_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 15, r)
#define RDD_CPU_RX_DESCRIPTOR_WAN_FLOW_WRITE_G(v, g, idx)                         GROUP_MWRITE_8(g, idx*sizeof(RDD_CPU_RX_DESCRIPTOR_DTS) + 15, v)
#define RDD_CPU_RX_DESCRIPTOR_WAN_FLOW_READ(r, p)                                 MREAD_8((uint8_t *)p + 15, r)
#define RDD_CPU_RX_DESCRIPTOR_WAN_FLOW_WRITE(v, p)                                MWRITE_8((uint8_t *)p + 15, v)
/* <<<RDD_CPU_RX_DESCRIPTOR_DTS */

/* <<<RDD_CPU_RX_DESCRIPTOR */
/* >>>RDD_CPU_FEED_DESCRIPTOR */
#define CPU_FEED_DESCRIPTOR_PTR_LOW_F_OFFSET                             0
#define CPU_FEED_DESCRIPTOR_PTR_LOW_F_WIDTH                              32
#define CPU_FEED_DESCRIPTOR_PTR_LOW_OFFSET                               0
#define CPU_FEED_DESCRIPTOR_TYPE_F_OFFSET                                8
#define CPU_FEED_DESCRIPTOR_TYPE_F_WIDTH                                 1
#define CPU_FEED_DESCRIPTOR_TYPE_OFFSET                                  6
#define CPU_FEED_DESCRIPTOR_TYPE_F_OFFSET_MOD8                           0
#define CPU_FEED_DESCRIPTOR_PTR_HI_F_OFFSET                              0
#define CPU_FEED_DESCRIPTOR_PTR_HI_F_WIDTH                               8
#define CPU_FEED_DESCRIPTOR_PTR_HI_OFFSET                                7

/* >>>RDD_CPU_FEED_DESCRIPTOR_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	ptr_low   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0 	:23	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	type      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ptr_hi    	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	ptr_low   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ptr_hi    	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	type      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0 	:23	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_FEED_DESCRIPTOR_DTS;

#define RDD_CPU_FEED_DESCRIPTOR_PTR_LOW_READ_G(r, g, idx)            GROUP_MREAD_32(g, idx*sizeof(RDD_CPU_FEED_DESCRIPTOR_DTS), r)
#define RDD_CPU_FEED_DESCRIPTOR_PTR_LOW_WRITE_G(v, g, idx)           GROUP_MWRITE_32(g, idx*sizeof(RDD_CPU_FEED_DESCRIPTOR_DTS), v)
#define RDD_CPU_FEED_DESCRIPTOR_PTR_LOW_READ(r, p)                   MREAD_32((uint8_t *)p, r)
#define RDD_CPU_FEED_DESCRIPTOR_PTR_LOW_WRITE(v, p)                  MWRITE_32((uint8_t *)p, v)
#define RDD_CPU_FEED_DESCRIPTOR_TYPE_READ_G(r, g, idx)               GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_CPU_FEED_DESCRIPTOR_DTS) + 6, 0, 1, r)
#define RDD_CPU_FEED_DESCRIPTOR_TYPE_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_CPU_FEED_DESCRIPTOR_DTS) + 6, 0, 1, v)
#define RDD_CPU_FEED_DESCRIPTOR_TYPE_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 6, 0, 1, r)
#define RDD_CPU_FEED_DESCRIPTOR_TYPE_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 6, 0, 1, v)
#define RDD_CPU_FEED_DESCRIPTOR_PTR_HI_READ_G(r, g, idx)             GROUP_MREAD_8(g, idx*sizeof(RDD_CPU_FEED_DESCRIPTOR_DTS) + 7, r)
#define RDD_CPU_FEED_DESCRIPTOR_PTR_HI_WRITE_G(v, g, idx)            GROUP_MWRITE_8(g, idx*sizeof(RDD_CPU_FEED_DESCRIPTOR_DTS) + 7, v)
#define RDD_CPU_FEED_DESCRIPTOR_PTR_HI_READ(r, p)                    MREAD_8((uint8_t *)p + 7, r)
#define RDD_CPU_FEED_DESCRIPTOR_PTR_HI_WRITE(v, p)                   MWRITE_8((uint8_t *)p + 7, v)
/* <<<RDD_CPU_FEED_DESCRIPTOR_DTS */

/* <<<RDD_CPU_FEED_DESCRIPTOR */
#endif /* _RDD_DATA_STRUCTURES_AUTO_H_ */

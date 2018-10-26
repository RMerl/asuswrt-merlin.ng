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


#ifndef _RDD_DATA_STRUCTURES_AUTO_H_
#define _RDD_DATA_STRUCTURES_AUTO_H_

#define GROUPED_EN_SEGMENTS_NUM       3
#define INVALID_TABLE_ADDRESS         0xFFFFFF

/* DDR */
/* PSRAM */
/* CORE_0 */

/* >>>BASIC_RATE_LIMITER_TABLE_DS */

/* >>>RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	rl_type       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2     	:21	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	block_type    	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	block_index   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	current_budget	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	alloc_mantissa	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	alloc_exponent	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	limit_mantissa	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	limit_exponent	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved      	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	block_index   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	block_type    	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2     	:21	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rl_type       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	current_budget	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	limit_exponent	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	limit_mantissa	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	alloc_exponent	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	alloc_mantissa	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved      	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS;

#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_RL_TYPE_READ_G(r, g, idx)                 GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS), 7, 1, r)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_RL_TYPE_WRITE_G(v, g, idx)                GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS), 7, 1, v)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_RL_TYPE_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_RL_TYPE_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_BLOCK_TYPE_READ_G(r, g, idx)              GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS) + 2, 0, 2, r)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_BLOCK_TYPE_WRITE_G(v, g, idx)             GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS) + 2, 0, 2, v)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_BLOCK_TYPE_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 2, 0, 2, r)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_BLOCK_TYPE_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 2, 0, 2, v)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_BLOCK_INDEX_READ_G(r, g, idx)             GROUP_MREAD_8(g, idx*sizeof(RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS) + 3, r)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_BLOCK_INDEX_WRITE_G(v, g, idx)            GROUP_MWRITE_8(g, idx*sizeof(RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS) + 3, v)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_BLOCK_INDEX_READ(r, p)                    MREAD_8((uint8_t *)p + 3, r)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_BLOCK_INDEX_WRITE(v, p)                   MWRITE_8((uint8_t *)p + 3, v)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_CURRENT_BUDGET_READ_G(r, g, idx)          GROUP_MREAD_32(g, idx*sizeof(RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS) + 4, r)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_CURRENT_BUDGET_WRITE_G(v, g, idx)         GROUP_MWRITE_32(g, idx*sizeof(RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS) + 4, v)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_CURRENT_BUDGET_READ(r, p)                 MREAD_32((uint8_t *)p + 4, r)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_CURRENT_BUDGET_WRITE(v, p)                MWRITE_32((uint8_t *)p + 4, v)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_ALLOC_MANTISSA_READ_G(r, g, idx)          GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS) + 8, 2, 14, r)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_ALLOC_MANTISSA_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS) + 8, 2, 14, v)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_ALLOC_MANTISSA_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p + 8, 2, 14, r)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_ALLOC_MANTISSA_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p + 8, 2, 14, v)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_ALLOC_EXPONENT_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS) + 9, 0, 2, r)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_ALLOC_EXPONENT_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS) + 9, 0, 2, v)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_ALLOC_EXPONENT_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 9, 0, 2, r)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_ALLOC_EXPONENT_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 9, 0, 2, v)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_LIMIT_MANTISSA_READ_G(r, g, idx)          GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS) + 10, 2, 14, r)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_LIMIT_MANTISSA_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS) + 10, 2, 14, v)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_LIMIT_MANTISSA_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p + 10, 2, 14, r)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_LIMIT_MANTISSA_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p + 10, 2, 14, v)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_LIMIT_EXPONENT_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS) + 11, 0, 2, r)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_LIMIT_EXPONENT_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS) + 11, 0, 2, v)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_LIMIT_EXPONENT_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 11, 0, 2, r)
#define RDD_BASIC_RATE_LIMITER_DESCRIPTOR_LIMIT_EXPONENT_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 11, 0, 2, v)
/* <<<RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS */


#define RDD_BASIC_RATE_LIMITER_TABLE_DS_SIZE     128
typedef struct
{
	RDD_BASIC_RATE_LIMITER_DESCRIPTOR_DTS	entry[ RDD_BASIC_RATE_LIMITER_TABLE_DS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BASIC_RATE_LIMITER_TABLE_DS_DTS;

extern uint32_t RDD_BASIC_RATE_LIMITER_TABLE_DS_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_BASIC_RATE_LIMITER_TABLE_DS_PTR(core_id)	( RDD_BASIC_RATE_LIMITER_TABLE_DS_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_BASIC_RATE_LIMITER_TABLE_DS_ADDRESS_ARR[core_id] ))

/* <<<BASIC_RATE_LIMITER_TABLE_DS */


/* >>>DS_TM_SCHEDULING_QUEUE_TABLE */

/* >>>RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	rate_limit_enable 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	block_type        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	prefetch_pd       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	enable            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved          	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbh_queue_index   	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	scheduler_index   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	queue_bit_mask    	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_limiter_index	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	quantum_number    	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	deficit_counter   	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	queue_bit_mask    	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	scheduler_index   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbh_queue_index   	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved          	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	enable            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	prefetch_pd       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	block_type        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_limit_enable 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	deficit_counter   	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	quantum_number    	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_limiter_index	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS;

#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMIT_ENABLE_READ_G(r, g, idx)           GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS), 7, 1, r)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_G(v, g, idx)          GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS), 7, 1, v)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMIT_ENABLE_READ(r, p)                  FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE(v, p)                 FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_BLOCK_TYPE_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS), 6, 1, r)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_BLOCK_TYPE_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS), 6, 1, v)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_BLOCK_TYPE_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p, 6, 1, r)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_BLOCK_TYPE_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p, 6, 1, v)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_PREFETCH_PD_READ_G(r, g, idx)                 GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS), 5, 1, r)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_PREFETCH_PD_WRITE_G(v, g, idx)                GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS), 5, 1, v)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_PREFETCH_PD_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p, 5, 1, r)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_PREFETCH_PD_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p, 5, 1, v)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_ENABLE_READ_G(r, g, idx)                      GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS), 4, 1, r)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_ENABLE_WRITE_G(v, g, idx)                     GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS), 4, 1, v)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_ENABLE_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p, 4, 1, r)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_ENABLE_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p, 4, 1, v)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_BBH_QUEUE_INDEX_READ_G(r, g, idx)             GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS) + 1, 0, 6, r)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_BBH_QUEUE_INDEX_WRITE_G(v, g, idx)            GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS) + 1, 0, 6, v)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_BBH_QUEUE_INDEX_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 1, 0, 6, r)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_BBH_QUEUE_INDEX_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 1, 0, 6, v)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_READ_G(r, g, idx)             GROUP_MREAD_8(g, idx*sizeof(RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS) + 2, r)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_WRITE_G(v, g, idx)            GROUP_MWRITE_8(g, idx*sizeof(RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS) + 2, v)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_READ(r, p)                    MREAD_8((uint8_t *)p + 2, r)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_WRITE(v, p)                   MWRITE_8((uint8_t *)p + 2, v)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUEUE_BIT_MASK_READ_G(r, g, idx)              GROUP_MREAD_8(g, idx*sizeof(RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS) + 3, r)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUEUE_BIT_MASK_WRITE_G(v, g, idx)             GROUP_MWRITE_8(g, idx*sizeof(RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS) + 3, v)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUEUE_BIT_MASK_READ(r, p)                     MREAD_8((uint8_t *)p + 3, r)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUEUE_BIT_MASK_WRITE(v, p)                    MWRITE_8((uint8_t *)p + 3, v)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMITER_INDEX_READ_G(r, g, idx)          GROUP_MREAD_8(g, idx*sizeof(RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS) + 4, r)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMITER_INDEX_WRITE_G(v, g, idx)         GROUP_MWRITE_8(g, idx*sizeof(RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS) + 4, v)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMITER_INDEX_READ(r, p)                 MREAD_8((uint8_t *)p + 4, r)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_RATE_LIMITER_INDEX_WRITE(v, p)                MWRITE_8((uint8_t *)p + 4, v)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUANTUM_NUMBER_READ_G(r, g, idx)              GROUP_MREAD_8(g, idx*sizeof(RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS) + 5, r)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUANTUM_NUMBER_WRITE_G(v, g, idx)             GROUP_MWRITE_8(g, idx*sizeof(RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS) + 5, v)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUANTUM_NUMBER_READ(r, p)                     MREAD_8((uint8_t *)p + 5, r)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_QUANTUM_NUMBER_WRITE(v, p)                    MWRITE_8((uint8_t *)p + 5, v)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_DEFICIT_COUNTER_READ_G(r, g, idx)             GROUP_MREAD_16(g, idx*sizeof(RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS) + 6, r)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_DEFICIT_COUNTER_WRITE_G(v, g, idx)            GROUP_MWRITE_16(g, idx*sizeof(RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS) + 6, v)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_DEFICIT_COUNTER_READ(r, p)                    MREAD_16((uint8_t *)p + 6, r)
#define RDD_SCHEDULING_QUEUE_DESCRIPTOR_DEFICIT_COUNTER_WRITE(v, p)                   MWRITE_16((uint8_t *)p + 6, v)
/* <<<RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS */


#define RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE     160
typedef struct
{
	RDD_SCHEDULING_QUEUE_DESCRIPTOR_DTS	entry[ RDD_DS_TM_SCHEDULING_QUEUE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_TM_SCHEDULING_QUEUE_TABLE_DTS;

extern uint32_t RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_DS_TM_SCHEDULING_QUEUE_TABLE_PTR(core_id)	( RDD_DS_TM_SCHEDULING_QUEUE_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_DS_TM_SCHEDULING_QUEUE_TABLE_ADDRESS_ARR[core_id] ))

/* <<<DS_TM_SCHEDULING_QUEUE_TABLE */


/* >>>DS_TM_BBH_TX_EGRESS_COUNTER_TABLE */

/* >>>RDD_BBH_TX_EGRESS_COUNTER_ENTRY_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	counter   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0 	:24	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1 	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	reserved0 	:24	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	counter   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1 	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BBH_TX_EGRESS_COUNTER_ENTRY_DTS;

#define RDD_BBH_TX_EGRESS_COUNTER_ENTRY_COUNTER_READ_G(r, g, idx)            GROUP_MREAD_8(g, idx*sizeof(RDD_BBH_TX_EGRESS_COUNTER_ENTRY_DTS), r)
#define RDD_BBH_TX_EGRESS_COUNTER_ENTRY_COUNTER_WRITE_G(v, g, idx)           GROUP_MWRITE_8(g, idx*sizeof(RDD_BBH_TX_EGRESS_COUNTER_ENTRY_DTS), v)
#define RDD_BBH_TX_EGRESS_COUNTER_ENTRY_COUNTER_READ(r, p)                   MREAD_8((uint8_t *)p, r)
#define RDD_BBH_TX_EGRESS_COUNTER_ENTRY_COUNTER_WRITE(v, p)                  MWRITE_8((uint8_t *)p, v)
/* <<<RDD_BBH_TX_EGRESS_COUNTER_ENTRY_DTS */


#define RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_SIZE     8
typedef struct
{
	RDD_BBH_TX_EGRESS_COUNTER_ENTRY_DTS	entry[ RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_DTS;

extern uint32_t RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_PTR(core_id)	( RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_ADDRESS_ARR[core_id] ))

/* <<<DS_TM_BBH_TX_EGRESS_COUNTER_TABLE */


/* >>>REGISTERS_BUFFER */

/* >>>RDD_BYTES_4_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	bits      	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	bits      	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BYTES_4_DTS;

#define RDD_BYTES_4_BITS_READ_G(r, g, idx)          GROUP_MREAD_32(g, idx*sizeof(RDD_BYTES_4_DTS), r)
#define RDD_BYTES_4_BITS_WRITE_G(v, g, idx)         GROUP_MWRITE_32(g, idx*sizeof(RDD_BYTES_4_DTS), v)
#define RDD_BYTES_4_BITS_READ(r, p)                 MREAD_32((uint8_t *)p, r)
#define RDD_BYTES_4_BITS_WRITE(v, p)                MWRITE_32((uint8_t *)p, v)
/* <<<RDD_BYTES_4_DTS */


#define RDD_REGISTERS_BUFFER_SIZE     32
typedef struct
{
	RDD_BYTES_4_DTS	entry[ RDD_REGISTERS_BUFFER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_REGISTERS_BUFFER_DTS;

extern uint32_t RDD_REGISTERS_BUFFER_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_REGISTERS_BUFFER_PTR(core_id)	( RDD_REGISTERS_BUFFER_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_REGISTERS_BUFFER_ADDRESS_ARR[core_id] ))

/* <<<REGISTERS_BUFFER */


/* >>>DS_TM_TM_ACTION_PTR_TABLE */

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


#define RDD_DS_TM_TM_ACTION_PTR_TABLE_SIZE     17
typedef struct
{
	RDD_BYTES_2_DTS	entry[ RDD_DS_TM_TM_ACTION_PTR_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_TM_TM_ACTION_PTR_TABLE_DTS;

extern uint32_t RDD_DS_TM_TM_ACTION_PTR_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_DS_TM_TM_ACTION_PTR_TABLE_PTR(core_id)	( RDD_DS_TM_TM_ACTION_PTR_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_DS_TM_TM_ACTION_PTR_TABLE_ADDRESS_ARR[core_id] ))

/* <<<DS_TM_TM_ACTION_PTR_TABLE */


/* >>>BUDGET_ALLOCATION_TIMER_VALUE */
typedef struct
{
	RDD_BYTES_2_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BUDGET_ALLOCATION_TIMER_VALUE_DTS;

extern uint32_t RDD_BUDGET_ALLOCATION_TIMER_VALUE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_BUDGET_ALLOCATION_TIMER_VALUE_PTR(core_id)	( RDD_BUDGET_ALLOCATION_TIMER_VALUE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_BUDGET_ALLOCATION_TIMER_VALUE_ADDRESS_ARR[core_id] ))

/* <<<BUDGET_ALLOCATION_TIMER_VALUE */


/* >>>DS_TM_BB_DESTINATION_TABLE */
typedef struct
{
	RDD_BYTES_2_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_TM_BB_DESTINATION_TABLE_DTS;

extern uint32_t RDD_DS_TM_BB_DESTINATION_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_DS_TM_BB_DESTINATION_TABLE_PTR(core_id)	( RDD_DS_TM_BB_DESTINATION_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_DS_TM_BB_DESTINATION_TABLE_ADDRESS_ARR[core_id] ))

/* <<<DS_TM_BB_DESTINATION_TABLE */


/* >>>SCHEDULING_FLUSH_GLOBAL_CFG */

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

typedef struct
{
	RDD_BYTE_1_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SCHEDULING_FLUSH_GLOBAL_CFG_DTS;

extern uint32_t RDD_SCHEDULING_FLUSH_GLOBAL_CFG_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_SCHEDULING_FLUSH_GLOBAL_CFG_PTR(core_id)	( RDD_SCHEDULING_FLUSH_GLOBAL_CFG_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_SCHEDULING_FLUSH_GLOBAL_CFG_ADDRESS_ARR[core_id] ))

/* <<<SCHEDULING_FLUSH_GLOBAL_CFG */


/* >>>DS_TM_SCHEDULING_GLOBAL_FLUSH_CFG */
typedef struct
{
	RDD_BYTE_1_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_TM_SCHEDULING_GLOBAL_FLUSH_CFG_DTS;

extern uint32_t RDD_DS_TM_SCHEDULING_GLOBAL_FLUSH_CFG_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_DS_TM_SCHEDULING_GLOBAL_FLUSH_CFG_PTR(core_id)	( RDD_DS_TM_SCHEDULING_GLOBAL_FLUSH_CFG_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_DS_TM_SCHEDULING_GLOBAL_FLUSH_CFG_ADDRESS_ARR[core_id] ))

/* <<<DS_TM_SCHEDULING_GLOBAL_FLUSH_CFG */


/* >>>DS_TM_BBH_TX_QUEUE_ID_TABLE */

/* >>>RDD_BBH_TX_QUEUE_ID_ENTRY_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	bytes_in_bbh	:24	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	queue       	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	queue       	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bytes_in_bbh	:24	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BBH_TX_QUEUE_ID_ENTRY_DTS;

#define RDD_BBH_TX_QUEUE_ID_ENTRY_BYTES_IN_BBH_READ_G(r, g, idx)          GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_BBH_TX_QUEUE_ID_ENTRY_DTS), 8, 24, r)
#define RDD_BBH_TX_QUEUE_ID_ENTRY_BYTES_IN_BBH_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_BBH_TX_QUEUE_ID_ENTRY_DTS), 8, 24, v)
#define RDD_BBH_TX_QUEUE_ID_ENTRY_BYTES_IN_BBH_READ(r, p)                 FIELD_MREAD_32((uint8_t *)p, 8, 24, r)
#define RDD_BBH_TX_QUEUE_ID_ENTRY_BYTES_IN_BBH_WRITE(v, p)                FIELD_MWRITE_32((uint8_t *)p, 8, 24, v)
#define RDD_BBH_TX_QUEUE_ID_ENTRY_QUEUE_READ_G(r, g, idx)                 GROUP_MREAD_8(g, idx*sizeof(RDD_BBH_TX_QUEUE_ID_ENTRY_DTS) + 3, r)
#define RDD_BBH_TX_QUEUE_ID_ENTRY_QUEUE_WRITE_G(v, g, idx)                GROUP_MWRITE_8(g, idx*sizeof(RDD_BBH_TX_QUEUE_ID_ENTRY_DTS) + 3, v)
#define RDD_BBH_TX_QUEUE_ID_ENTRY_QUEUE_READ(r, p)                        MREAD_8((uint8_t *)p + 3, r)
#define RDD_BBH_TX_QUEUE_ID_ENTRY_QUEUE_WRITE(v, p)                       MWRITE_8((uint8_t *)p + 3, v)
/* <<<RDD_BBH_TX_QUEUE_ID_ENTRY_DTS */


#define RDD_DS_TM_BBH_TX_QUEUE_ID_TABLE_SIZE     2
typedef struct
{
	RDD_BBH_TX_QUEUE_ID_ENTRY_DTS	entry[ RDD_DS_TM_BBH_TX_QUEUE_ID_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_TM_BBH_TX_QUEUE_ID_TABLE_DTS;

extern uint32_t RDD_DS_TM_BBH_TX_QUEUE_ID_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_DS_TM_BBH_TX_QUEUE_ID_TABLE_PTR(core_id)	( RDD_DS_TM_BBH_TX_QUEUE_ID_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_DS_TM_BBH_TX_QUEUE_ID_TABLE_ADDRESS_ARR[core_id] ))

/* <<<DS_TM_BBH_TX_QUEUE_ID_TABLE */


/* >>>DS_TM_TM_FLOW_CNTR_TABLE */

/* >>>RDD_TM_FLOW_CNTR_ENTRY_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint8_t	cntr_id   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint8_t	cntr_id   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_TM_FLOW_CNTR_ENTRY_DTS;

#define RDD_TM_FLOW_CNTR_ENTRY_CNTR_ID_READ_G(r, g, idx)          GROUP_MREAD_8(g, idx*sizeof(RDD_TM_FLOW_CNTR_ENTRY_DTS), r)
#define RDD_TM_FLOW_CNTR_ENTRY_CNTR_ID_WRITE_G(v, g, idx)         GROUP_MWRITE_8(g, idx*sizeof(RDD_TM_FLOW_CNTR_ENTRY_DTS), v)
#define RDD_TM_FLOW_CNTR_ENTRY_CNTR_ID_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_TM_FLOW_CNTR_ENTRY_CNTR_ID_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
/* <<<RDD_TM_FLOW_CNTR_ENTRY_DTS */


#define RDD_DS_TM_TM_FLOW_CNTR_TABLE_SIZE     6
typedef struct
{
	RDD_TM_FLOW_CNTR_ENTRY_DTS	entry[ RDD_DS_TM_TM_FLOW_CNTR_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_TM_TM_FLOW_CNTR_TABLE_DTS;

extern uint32_t RDD_DS_TM_TM_FLOW_CNTR_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_DS_TM_TM_FLOW_CNTR_TABLE_PTR(core_id)	( RDD_DS_TM_TM_FLOW_CNTR_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_DS_TM_TM_FLOW_CNTR_TABLE_ADDRESS_ARR[core_id] ))

/* <<<DS_TM_TM_FLOW_CNTR_TABLE */


/* >>>SRAM_DUMMY_STORE */
typedef struct
{
	RDD_BYTE_1_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SRAM_DUMMY_STORE_DTS;

extern uint32_t RDD_SRAM_DUMMY_STORE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_SRAM_DUMMY_STORE_PTR(core_id)	( RDD_SRAM_DUMMY_STORE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_SRAM_DUMMY_STORE_ADDRESS_ARR[core_id] ))

/* <<<SRAM_DUMMY_STORE */


/* >>>DS_TM_FIRST_QUEUE_MAPPING */
typedef struct
{
	RDD_BYTE_1_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_TM_FIRST_QUEUE_MAPPING_DTS;

extern uint32_t RDD_DS_TM_FIRST_QUEUE_MAPPING_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_DS_TM_FIRST_QUEUE_MAPPING_PTR(core_id)	( RDD_DS_TM_FIRST_QUEUE_MAPPING_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_DS_TM_FIRST_QUEUE_MAPPING_ADDRESS_ARR[core_id] ))

/* <<<DS_TM_FIRST_QUEUE_MAPPING */


/* >>>TASK_IDX */
typedef struct
{
	RDD_BYTES_4_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_TASK_IDX_DTS;

extern uint32_t RDD_TASK_IDX_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_TASK_IDX_PTR(core_id)	( RDD_TASK_IDX_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_TASK_IDX_ADDRESS_ARR[core_id] ))

/* <<<TASK_IDX */


/* >>>BASIC_SCHEDULER_TABLE_DS */

/* >>>RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS */
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_QUEUE_INDEX_NUMBER	8

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	rate_limit_enable                                                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	complex_scheduler_exists                                         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dwrr_offset                                                      	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	last_served_queue                                                	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	parent_index                                                     	:9	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = parent_index, size = 9 bits
	uint32_t	complex_scheduler_slot_index                                     	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	complex_scheduler_index                                          	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbh_queue                                                        	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ovl_rl_en                                                        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	is_positive_budget                                               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1                                                        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	slot_budget_bit_vector                                           	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	status_bit_vector                                                	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_limiter_index                                               	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	quantum_number                                                   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	deficit_counter                                                  	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	queue_index[RDD_BASIC_SCHEDULER_DESCRIPTOR_QUEUE_INDEX_NUMBER];
#else
	uint32_t	status_bit_vector                                                	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	slot_budget_bit_vector                                           	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	parent_index                                                     	:9	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = parent_index, size = 9 bits
	uint32_t	complex_scheduler_slot_index                                     	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	complex_scheduler_index                                          	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbh_queue                                                        	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ovl_rl_en                                                        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	is_positive_budget                                               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1                                                        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	last_served_queue                                                	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dwrr_offset                                                      	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	complex_scheduler_exists                                         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_limit_enable                                                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	deficit_counter                                                  	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	quantum_number                                                   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_limiter_index                                               	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	queue_index[RDD_BASIC_SCHEDULER_DESCRIPTOR_QUEUE_INDEX_NUMBER];
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS;

#define RDD_BASIC_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS), 7, 1, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS), 7, 1, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_EXISTS_READ_G(r, g, idx)              GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS), 6, 1, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_EXISTS_WRITE_G(v, g, idx)             GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS), 6, 1, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_EXISTS_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p, 6, 1, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_EXISTS_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p, 6, 1, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_READ_G(r, g, idx)                           GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS), 4, 2, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_WRITE_G(v, g, idx)                          GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS), 4, 2, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_READ(r, p)                                  FIELD_MREAD_8((uint8_t *)p, 4, 2, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_WRITE(v, p)                                 FIELD_MWRITE_8((uint8_t *)p, 4, 2, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_LAST_SERVED_QUEUE_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS), 1, 3, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_LAST_SERVED_QUEUE_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS), 1, 3, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_LAST_SERVED_QUEUE_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p, 1, 3, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_LAST_SERVED_QUEUE_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p, 1, 3, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_PARENT_INDEX_READ_G(r, g, idx)                          GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS), 0, 9, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_PARENT_INDEX_WRITE_G(v, g, idx)                         GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS), 0, 9, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_PARENT_INDEX_READ(r, p)                                 FIELD_MREAD_16((uint8_t *)p, 0, 9, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_PARENT_INDEX_WRITE(v, p)                                FIELD_MWRITE_16((uint8_t *)p, 0, 9, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_SLOT_INDEX_READ_G(r, g, idx)          GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS), 4, 5, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_SLOT_INDEX_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS), 4, 5, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_SLOT_INDEX_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p, 4, 5, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_SLOT_INDEX_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p, 4, 5, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_INDEX_READ_G(r, g, idx)               GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS) + 1, 0, 4, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_INDEX_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS) + 1, 0, 4, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_INDEX_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 1, 0, 4, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_COMPLEX_SCHEDULER_INDEX_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 1, 0, 4, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_BBH_QUEUE_READ_G(r, g, idx)                             GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS), 3, 6, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_BBH_QUEUE_WRITE_G(v, g, idx)                            GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS), 3, 6, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_BBH_QUEUE_READ(r, p)                                    FIELD_MREAD_16((uint8_t *)p, 3, 6, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_BBH_QUEUE_WRITE(v, p)                                   FIELD_MWRITE_16((uint8_t *)p, 3, 6, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_OVL_RL_EN_READ_G(r, g, idx)                             GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS) + 1, 2, 1, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_OVL_RL_EN_WRITE_G(v, g, idx)                            GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS) + 1, 2, 1, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_OVL_RL_EN_READ(r, p)                                    FIELD_MREAD_8((uint8_t *)p + 1, 2, 1, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_OVL_RL_EN_WRITE(v, p)                                   FIELD_MWRITE_8((uint8_t *)p + 1, 2, 1, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_READ_G(r, g, idx)                    GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS) + 1, 1, 1, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE_G(v, g, idx)                   GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS) + 1, 1, 1, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 1, 1, 1, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 1, 1, 1, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_READ_G(r, g, idx)                GROUP_MREAD_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS) + 2, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_WRITE_G(v, g, idx)               GROUP_MWRITE_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS) + 2, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_READ(r, p)                       MREAD_8((uint8_t *)p + 2, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_WRITE(v, p)                      MWRITE_8((uint8_t *)p + 2, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_READ_G(r, g, idx)                     GROUP_MREAD_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS) + 3, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_WRITE_G(v, g, idx)                    GROUP_MWRITE_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS) + 3, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_READ(r, p)                            MREAD_8((uint8_t *)p + 3, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_WRITE(v, p)                           MWRITE_8((uint8_t *)p + 3, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_READ_G(r, g, idx)                    GROUP_MREAD_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS) + 4, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_WRITE_G(v, g, idx)                   GROUP_MWRITE_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS) + 4, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_READ(r, p)                           MREAD_8((uint8_t *)p + 4, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_WRITE(v, p)                          MWRITE_8((uint8_t *)p + 4, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_READ_G(r, g, idx)                        GROUP_MREAD_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS) + 5, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_WRITE_G(v, g, idx)                       GROUP_MWRITE_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS) + 5, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_READ(r, p)                               MREAD_8((uint8_t *)p + 5, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_QUANTUM_NUMBER_WRITE(v, p)                              MWRITE_8((uint8_t *)p + 5, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_DEFICIT_COUNTER_READ_G(r, g, idx)                       GROUP_MREAD_16(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS) + 6, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_DEFICIT_COUNTER_WRITE_G(v, g, idx)                      GROUP_MWRITE_16(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS) + 6, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_DEFICIT_COUNTER_READ(r, p)                              MREAD_16((uint8_t *)p + 6, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_DEFICIT_COUNTER_WRITE(v, p)                             MWRITE_16((uint8_t *)p + 6, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_QUEUE_INDEX_READ_G(r, g, idx, i)                        GROUP_MREAD_I_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS) + 8, i, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_QUEUE_INDEX_WRITE_G(v, g, idx, i)                       GROUP_MWRITE_I_8(g, idx*sizeof(RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS) + 8, i, v)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_QUEUE_INDEX_READ(r, p, i)                               MREAD_I_8((uint8_t *)p + 8, i, r)
#define RDD_BASIC_SCHEDULER_DESCRIPTOR_QUEUE_INDEX_WRITE(v, p, i)                              MWRITE_I_8((uint8_t *)p + 8, i, v)
/* <<<RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS */


#define RDD_BASIC_SCHEDULER_TABLE_DS_SIZE     32
typedef struct
{
	RDD_BASIC_SCHEDULER_DESCRIPTOR_DTS	entry[ RDD_BASIC_SCHEDULER_TABLE_DS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BASIC_SCHEDULER_TABLE_DS_DTS;

extern uint32_t RDD_BASIC_SCHEDULER_TABLE_DS_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_BASIC_SCHEDULER_TABLE_DS_PTR(core_id)	( RDD_BASIC_SCHEDULER_TABLE_DS_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_BASIC_SCHEDULER_TABLE_DS_ADDRESS_ARR[core_id] ))

/* <<<BASIC_SCHEDULER_TABLE_DS */


/* >>>DS_TM_PD_FIFO_TABLE */

/* >>>RDD_PROCESSING_TX_DESCRIPTOR_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	valid         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	headroom      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dont_agg      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mc_copy       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reprocess     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	color         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	force_copy    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	second_level_q	:9	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	first_level_q 	:9	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	flag_1588     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	coherent      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	hn            	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	serial_num    	:10	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	priority      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bn_num        	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	packet_length 	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	drop          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	target_mem_1  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mcst_bcst_union	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = mcst_bcst_union, size = 2 bits
	uint32_t	cong_state    	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mcst_packet   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bcst_packet   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	ingress_cong  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	lan           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ingress_port  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = ingress_port, size = 8 bits
	uint32_t	flow          	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	source_port   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	lag_port      	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	lan_vport     	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	union3        	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = union3, size = 18 bits
	uint32_t	bn1_first     	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs3          	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	agg_pd        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	target_mem_0  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	payload_offset_or_abs_1	:11	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = payload_offset_or_abs_1, size = 11 bits
	uint32_t	sop           	:11	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs1          	:11	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	buffer_number_0_or_abs_0	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = buffer_number_0_or_abs_0, size = 18 bits
	uint32_t	bn0_first     	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs0          	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
#else
	uint32_t	hn            	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	coherent      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	flag_1588     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	first_level_q 	:9	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	second_level_q	:9	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	force_copy    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	color         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reprocess     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mc_copy       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dont_agg      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	headroom      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	valid         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	packet_length 	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bn_num        	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	priority      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	serial_num    	:10	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	union3        	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = union3, size = 18 bits
	uint32_t	bn1_first     	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs3          	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	ingress_port  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = ingress_port, size = 8 bits
	uint32_t	flow          	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	source_port   	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	lag_port      	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	lan_vport     	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	lan           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ingress_cong  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mcst_bcst_union	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = mcst_bcst_union, size = 2 bits
	uint32_t	cong_state    	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mcst_packet   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bcst_packet   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	target_mem_1  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	drop          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	buffer_number_0_or_abs_0	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = buffer_number_0_or_abs_0, size = 18 bits
	uint32_t	bn0_first     	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs0          	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	payload_offset_or_abs_1	:11	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = payload_offset_or_abs_1, size = 11 bits
	uint32_t	sop           	:11	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs1          	:11	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	abs           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	target_mem_0  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	agg_pd        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PROCESSING_TX_DESCRIPTOR_DTS;

#define RDD_PROCESSING_TX_DESCRIPTOR_VALID_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS), 7, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_VALID_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS), 7, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_VALID_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_VALID_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_HEADROOM_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS), 6, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_HEADROOM_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS), 6, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_HEADROOM_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p, 6, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_HEADROOM_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p, 6, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_DONT_AGG_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS), 5, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_DONT_AGG_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS), 5, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_DONT_AGG_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p, 5, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_DONT_AGG_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p, 5, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_MC_COPY_READ_G(r, g, idx)                 GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS), 4, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_MC_COPY_WRITE_G(v, g, idx)                GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS), 4, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_MC_COPY_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p, 4, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_MC_COPY_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p, 4, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_REPROCESS_READ_G(r, g, idx)               GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS), 3, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_REPROCESS_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS), 3, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_REPROCESS_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p, 3, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_REPROCESS_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p, 3, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_COLOR_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS), 2, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_COLOR_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS), 2, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_COLOR_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p, 2, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_COLOR_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p, 2, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_FORCE_COPY_READ_G(r, g, idx)              GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS), 1, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_FORCE_COPY_WRITE_G(v, g, idx)             GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS), 1, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_FORCE_COPY_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p, 1, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_FORCE_COPY_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p, 1, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_SECOND_LEVEL_Q_READ_G(r, g, idx)          GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS), 0, 9, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_SECOND_LEVEL_Q_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS), 0, 9, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_SECOND_LEVEL_Q_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p, 0, 9, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_SECOND_LEVEL_Q_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p, 0, 9, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_FIRST_LEVEL_Q_READ_G(r, g, idx)           GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 2, 7, 9, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_FIRST_LEVEL_Q_WRITE_G(v, g, idx)          GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 2, 7, 9, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_FIRST_LEVEL_Q_READ(r, p)                  FIELD_MREAD_16((uint8_t *)p + 2, 7, 9, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_FIRST_LEVEL_Q_WRITE(v, p)                 FIELD_MWRITE_16((uint8_t *)p + 2, 7, 9, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_FLAG_1588_READ_G(r, g, idx)               GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 3, 6, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_FLAG_1588_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 3, 6, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_FLAG_1588_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 3, 6, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_FLAG_1588_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 3, 6, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_COHERENT_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 3, 5, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_COHERENT_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 3, 5, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_COHERENT_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 3, 5, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_COHERENT_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 3, 5, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_HN_READ_G(r, g, idx)                      GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 3, 0, 5, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_HN_WRITE_G(v, g, idx)                     GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 3, 0, 5, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_HN_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 3, 0, 5, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_HN_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 3, 0, 5, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_SERIAL_NUM_READ_G(r, g, idx)              GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 4, 6, 10, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_SERIAL_NUM_WRITE_G(v, g, idx)             GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 4, 6, 10, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_SERIAL_NUM_READ(r, p)                     FIELD_MREAD_16((uint8_t *)p + 4, 6, 10, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_SERIAL_NUM_WRITE(v, p)                    FIELD_MWRITE_16((uint8_t *)p + 4, 6, 10, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_PRIORITY_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 5, 5, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_PRIORITY_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 5, 5, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_PRIORITY_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 5, 5, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_PRIORITY_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 5, 5, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_BN_NUM_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 4, 14, 7, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_BN_NUM_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 4, 14, 7, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_BN_NUM_READ(r, p)                         FIELD_MREAD_32((uint8_t *)p + 4, 14, 7, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_BN_NUM_WRITE(v, p)                        FIELD_MWRITE_32((uint8_t *)p + 4, 14, 7, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_PACKET_LENGTH_READ_G(r, g, idx)           GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 6, 0, 14, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_PACKET_LENGTH_WRITE_G(v, g, idx)          GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 6, 0, 14, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_PACKET_LENGTH_READ(r, p)                  FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_PACKET_LENGTH_WRITE(v, p)                 FIELD_MWRITE_16((uint8_t *)p + 6, 0, 14, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_DROP_READ_G(r, g, idx)                    GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 7, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_DROP_WRITE_G(v, g, idx)                   GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 7, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_DROP_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p + 8, 7, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_DROP_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 8, 7, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_1_READ_G(r, g, idx)            GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 6, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_1_WRITE_G(v, g, idx)           GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 6, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_1_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p + 8, 6, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_1_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p + 8, 6, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_MCST_BCST_UNION_READ_G(r, g, idx)         GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 4, 2, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_MCST_BCST_UNION_WRITE_G(v, g, idx)        GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 4, 2, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_MCST_BCST_UNION_READ(r, p)                FIELD_MREAD_8((uint8_t *)p + 8, 4, 2, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_MCST_BCST_UNION_WRITE(v, p)               FIELD_MWRITE_8((uint8_t *)p + 8, 4, 2, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_CONG_STATE_READ_G(r, g, idx)              GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 4, 2, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_CONG_STATE_WRITE_G(v, g, idx)             GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 4, 2, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_CONG_STATE_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 8, 4, 2, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_CONG_STATE_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 8, 4, 2, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_MCST_PACKET_READ_G(r, g, idx)             GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 5, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_MCST_PACKET_WRITE_G(v, g, idx)            GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 5, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_MCST_PACKET_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 8, 5, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_MCST_PACKET_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 8, 5, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_BCST_PACKET_READ_G(r, g, idx)             GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 4, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_BCST_PACKET_WRITE_G(v, g, idx)            GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 4, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_BCST_PACKET_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 8, 4, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_BCST_PACKET_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 8, 4, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_CONG_READ_G(r, g, idx)            GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 3, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_CONG_WRITE_G(v, g, idx)           GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 3, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_CONG_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p + 8, 3, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_CONG_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p + 8, 3, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_LAN_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 2, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_LAN_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 2, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_LAN_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 8, 2, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_LAN_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 8, 2, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_PORT_READ_G(r, g, idx)            GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 2, 8, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_PORT_WRITE_G(v, g, idx)           GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 2, 8, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_PORT_READ(r, p)                   FIELD_MREAD_16((uint8_t *)p + 8, 2, 8, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_INGRESS_PORT_WRITE(v, p)                  FIELD_MWRITE_16((uint8_t *)p + 8, 2, 8, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_FLOW_READ_G(r, g, idx)                    GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 2, 8, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_FLOW_WRITE_G(v, g, idx)                   GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 2, 8, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_FLOW_READ(r, p)                           FIELD_MREAD_16((uint8_t *)p + 8, 2, 8, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_FLOW_WRITE(v, p)                          FIELD_MWRITE_16((uint8_t *)p + 8, 2, 8, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_SOURCE_PORT_READ_G(r, g, idx)             GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 2, 8, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_SOURCE_PORT_WRITE_G(v, g, idx)            GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 2, 8, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_SOURCE_PORT_READ(r, p)                    FIELD_MREAD_16((uint8_t *)p + 8, 2, 8, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_SOURCE_PORT_WRITE(v, p)                   FIELD_MWRITE_16((uint8_t *)p + 8, 2, 8, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_LAG_PORT_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 0, 2, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_LAG_PORT_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 0, 2, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_LAG_PORT_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 8, 0, 2, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_LAG_PORT_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 8, 0, 2, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_LAN_VPORT_READ_G(r, g, idx)               GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 9, 2, 6, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_LAN_VPORT_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 9, 2, 6, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_LAN_VPORT_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 9, 2, 6, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_LAN_VPORT_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 9, 2, 6, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_UNION3_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 0, 18, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_UNION3_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 0, 18, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_UNION3_READ(r, p)                         FIELD_MREAD_32((uint8_t *)p + 8, 0, 18, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_UNION3_WRITE(v, p)                        FIELD_MWRITE_32((uint8_t *)p + 8, 0, 18, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_BN1_FIRST_READ_G(r, g, idx)               GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 0, 18, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_BN1_FIRST_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 0, 18, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_BN1_FIRST_READ(r, p)                      FIELD_MREAD_32((uint8_t *)p + 8, 0, 18, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_BN1_FIRST_WRITE(v, p)                     FIELD_MWRITE_32((uint8_t *)p + 8, 0, 18, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_ABS3_READ_G(r, g, idx)                    GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 0, 18, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_ABS3_WRITE_G(v, g, idx)                   GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 8, 0, 18, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_ABS3_READ(r, p)                           FIELD_MREAD_32((uint8_t *)p + 8, 0, 18, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_ABS3_WRITE(v, p)                          FIELD_MWRITE_32((uint8_t *)p + 8, 0, 18, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_AGG_PD_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 12, 7, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_AGG_PD_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 12, 7, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_AGG_PD_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 12, 7, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_AGG_PD_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 12, 7, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_0_READ_G(r, g, idx)            GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 12, 6, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_0_WRITE_G(v, g, idx)           GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 12, 6, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_0_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p + 12, 6, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_TARGET_MEM_0_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p + 12, 6, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_ABS_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 12, 5, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_ABS_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 12, 5, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_ABS_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 12, 5, 1, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_ABS_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 12, 5, 1, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_PAYLOAD_OFFSET_OR_ABS_1_READ_G(r, g, idx) GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 12, 2, 11, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_PAYLOAD_OFFSET_OR_ABS_1_WRITE_G(v, g, idx)GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 12, 2, 11, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_PAYLOAD_OFFSET_OR_ABS_1_READ(r, p)        FIELD_MREAD_16((uint8_t *)p + 12, 2, 11, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_PAYLOAD_OFFSET_OR_ABS_1_WRITE(v, p)       FIELD_MWRITE_16((uint8_t *)p + 12, 2, 11, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_SOP_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 12, 2, 11, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_SOP_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 12, 2, 11, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_SOP_READ(r, p)                            FIELD_MREAD_16((uint8_t *)p + 12, 2, 11, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_SOP_WRITE(v, p)                           FIELD_MWRITE_16((uint8_t *)p + 12, 2, 11, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_ABS1_READ_G(r, g, idx)                    GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 12, 2, 11, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_ABS1_WRITE_G(v, g, idx)                   GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 12, 2, 11, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_ABS1_READ(r, p)                           FIELD_MREAD_16((uint8_t *)p + 12, 2, 11, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_ABS1_WRITE(v, p)                          FIELD_MWRITE_16((uint8_t *)p + 12, 2, 11, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_BUFFER_NUMBER_0_OR_ABS_0_READ_G(r, g, idx)GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 12, 0, 18, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_BUFFER_NUMBER_0_OR_ABS_0_WRITE_G(v, g, idx)GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 12, 0, 18, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_BUFFER_NUMBER_0_OR_ABS_0_READ(r, p)       FIELD_MREAD_32((uint8_t *)p + 12, 0, 18, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_BUFFER_NUMBER_0_OR_ABS_0_WRITE(v, p)      FIELD_MWRITE_32((uint8_t *)p + 12, 0, 18, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_BN0_FIRST_READ_G(r, g, idx)               GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 12, 0, 18, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_BN0_FIRST_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 12, 0, 18, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_BN0_FIRST_READ(r, p)                      FIELD_MREAD_32((uint8_t *)p + 12, 0, 18, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_BN0_FIRST_WRITE(v, p)                     FIELD_MWRITE_32((uint8_t *)p + 12, 0, 18, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_ABS0_READ_G(r, g, idx)                    GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 12, 0, 18, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_ABS0_WRITE_G(v, g, idx)                   GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_PROCESSING_TX_DESCRIPTOR_DTS) + 12, 0, 18, v)
#define RDD_PROCESSING_TX_DESCRIPTOR_ABS0_READ(r, p)                           FIELD_MREAD_32((uint8_t *)p + 12, 0, 18, r)
#define RDD_PROCESSING_TX_DESCRIPTOR_ABS0_WRITE(v, p)                          FIELD_MWRITE_32((uint8_t *)p + 12, 0, 18, v)
/* <<<RDD_PROCESSING_TX_DESCRIPTOR_DTS */


#define RDD_DS_TM_PD_FIFO_TABLE_SIZE     80
typedef struct
{
	RDD_PROCESSING_TX_DESCRIPTOR_DTS	entry[ RDD_DS_TM_PD_FIFO_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_TM_PD_FIFO_TABLE_DTS;

extern uint32_t RDD_DS_TM_PD_FIFO_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_DS_TM_PD_FIFO_TABLE_PTR(core_id)	( RDD_DS_TM_PD_FIFO_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_DS_TM_PD_FIFO_TABLE_ADDRESS_ARR[core_id] ))

/* <<<DS_TM_PD_FIFO_TABLE */


/* >>>UPDATE_FIFO_TABLE */

/* >>>RDD_UPDATE_FIFO_ENTRY_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	valid            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pd_fifo_write_ptr	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0        	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	queue_number     	:9	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	queue_number     	:9	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0        	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pd_fifo_write_ptr	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	valid            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_UPDATE_FIFO_ENTRY_DTS;

#define RDD_UPDATE_FIFO_ENTRY_VALID_READ_G(r, g, idx)                      GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_UPDATE_FIFO_ENTRY_DTS), 7, 1, r)
#define RDD_UPDATE_FIFO_ENTRY_VALID_WRITE_G(v, g, idx)                     GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_UPDATE_FIFO_ENTRY_DTS), 7, 1, v)
#define RDD_UPDATE_FIFO_ENTRY_VALID_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_UPDATE_FIFO_ENTRY_VALID_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_UPDATE_FIFO_ENTRY_PD_FIFO_WRITE_PTR_READ_G(r, g, idx)          GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_UPDATE_FIFO_ENTRY_DTS), 0, 14, r)
#define RDD_UPDATE_FIFO_ENTRY_PD_FIFO_WRITE_PTR_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_UPDATE_FIFO_ENTRY_DTS), 0, 14, v)
#define RDD_UPDATE_FIFO_ENTRY_PD_FIFO_WRITE_PTR_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p, 0, 14, r)
#define RDD_UPDATE_FIFO_ENTRY_PD_FIFO_WRITE_PTR_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p, 0, 14, v)
#define RDD_UPDATE_FIFO_ENTRY_QUEUE_NUMBER_READ_G(r, g, idx)               GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_UPDATE_FIFO_ENTRY_DTS) + 2, 0, 9, r)
#define RDD_UPDATE_FIFO_ENTRY_QUEUE_NUMBER_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_UPDATE_FIFO_ENTRY_DTS) + 2, 0, 9, v)
#define RDD_UPDATE_FIFO_ENTRY_QUEUE_NUMBER_READ(r, p)                      FIELD_MREAD_16((uint8_t *)p + 2, 0, 9, r)
#define RDD_UPDATE_FIFO_ENTRY_QUEUE_NUMBER_WRITE(v, p)                     FIELD_MWRITE_16((uint8_t *)p + 2, 0, 9, v)
/* <<<RDD_UPDATE_FIFO_ENTRY_DTS */


#define RDD_UPDATE_FIFO_TABLE_SIZE     8
typedef struct
{
	RDD_UPDATE_FIFO_ENTRY_DTS	entry[ RDD_UPDATE_FIFO_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_UPDATE_FIFO_TABLE_DTS;

extern uint32_t RDD_UPDATE_FIFO_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_UPDATE_FIFO_TABLE_PTR(core_id)	( RDD_UPDATE_FIFO_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_UPDATE_FIFO_TABLE_ADDRESS_ARR[core_id] ))

/* <<<UPDATE_FIFO_TABLE */


/* >>>RUNNER_GLOBAL_REGISTERS_INIT */

#define RDD_RUNNER_GLOBAL_REGISTERS_INIT_SIZE     8
typedef struct
{
	RDD_BYTES_4_DTS	entry[ RDD_RUNNER_GLOBAL_REGISTERS_INIT_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RUNNER_GLOBAL_REGISTERS_INIT_DTS;

extern uint32_t RDD_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_RUNNER_GLOBAL_REGISTERS_INIT_PTR(core_id)	( RDD_RUNNER_GLOBAL_REGISTERS_INIT_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_RUNNER_GLOBAL_REGISTERS_INIT_ADDRESS_ARR[core_id] ))

/* <<<RUNNER_GLOBAL_REGISTERS_INIT */


/* >>>DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR */

#define RDD_DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_SIZE     5
typedef struct
{
	RDD_BYTES_4_DTS	entry[ RDD_DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_DTS;

extern uint32_t RDD_DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_PTR(core_id)	( RDD_DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR_ADDRESS_ARR[core_id] ))

/* <<<DS_TM_SCHEDULING_AGGREGATION_CONTEXT_VECTOR */


/* >>>DS_TM_SCHEDULING_FLUSH_VECTOR */

#define RDD_DS_TM_SCHEDULING_FLUSH_VECTOR_SIZE     5
typedef struct
{
	RDD_BYTES_4_DTS	entry[ RDD_DS_TM_SCHEDULING_FLUSH_VECTOR_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_TM_SCHEDULING_FLUSH_VECTOR_DTS;

extern uint32_t RDD_DS_TM_SCHEDULING_FLUSH_VECTOR_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_DS_TM_SCHEDULING_FLUSH_VECTOR_PTR(core_id)	( RDD_DS_TM_SCHEDULING_FLUSH_VECTOR_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_DS_TM_SCHEDULING_FLUSH_VECTOR_ADDRESS_ARR[core_id] ))

/* <<<DS_TM_SCHEDULING_FLUSH_VECTOR */


/* >>>DS_TM_SCHEDULING_QUEUE_AGING_VECTOR */

#define RDD_DS_TM_SCHEDULING_QUEUE_AGING_VECTOR_SIZE     5
typedef struct
{
	RDD_BYTES_4_DTS	entry[ RDD_DS_TM_SCHEDULING_QUEUE_AGING_VECTOR_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_TM_SCHEDULING_QUEUE_AGING_VECTOR_DTS;

extern uint32_t RDD_DS_TM_SCHEDULING_QUEUE_AGING_VECTOR_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_DS_TM_SCHEDULING_QUEUE_AGING_VECTOR_PTR(core_id)	( RDD_DS_TM_SCHEDULING_QUEUE_AGING_VECTOR_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_DS_TM_SCHEDULING_QUEUE_AGING_VECTOR_ADDRESS_ARR[core_id] ))

/* <<<DS_TM_SCHEDULING_QUEUE_AGING_VECTOR */


/* >>>RATE_LIMITER_VALID_TABLE_DS */

#define RDD_RATE_LIMITER_VALID_TABLE_DS_SIZE     4
typedef struct
{
	RDD_BYTES_4_DTS	entry[ RDD_RATE_LIMITER_VALID_TABLE_DS_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RATE_LIMITER_VALID_TABLE_DS_DTS;

extern uint32_t RDD_RATE_LIMITER_VALID_TABLE_DS_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_RATE_LIMITER_VALID_TABLE_DS_PTR(core_id)	( RDD_RATE_LIMITER_VALID_TABLE_DS_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_RATE_LIMITER_VALID_TABLE_DS_ADDRESS_ARR[core_id] ))

/* <<<RATE_LIMITER_VALID_TABLE_DS */


/* >>>FPM_GLOBAL_CFG */

/* >>>RDD_FPM_GLOBAL_CFG_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	fpm_base_low        	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	fpm_base_high       	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	fpm_token_size_asr_8	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0           	:24	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	fpm_base_low        	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	fpm_base_high       	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0           	:24	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	fpm_token_size_asr_8	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FPM_GLOBAL_CFG_DTS;

#define RDD_FPM_GLOBAL_CFG_FPM_BASE_LOW_READ_G(r, g, idx)                  GROUP_MREAD_32(g, idx*sizeof(RDD_FPM_GLOBAL_CFG_DTS), r)
#define RDD_FPM_GLOBAL_CFG_FPM_BASE_LOW_WRITE_G(v, g, idx)                 GROUP_MWRITE_32(g, idx*sizeof(RDD_FPM_GLOBAL_CFG_DTS), v)
#define RDD_FPM_GLOBAL_CFG_FPM_BASE_LOW_READ(r, p)                         MREAD_32((uint8_t *)p, r)
#define RDD_FPM_GLOBAL_CFG_FPM_BASE_LOW_WRITE(v, p)                        MWRITE_32((uint8_t *)p, v)
#define RDD_FPM_GLOBAL_CFG_FPM_BASE_HIGH_READ_G(r, g, idx)                 GROUP_MREAD_32(g, idx*sizeof(RDD_FPM_GLOBAL_CFG_DTS) + 4, r)
#define RDD_FPM_GLOBAL_CFG_FPM_BASE_HIGH_WRITE_G(v, g, idx)                GROUP_MWRITE_32(g, idx*sizeof(RDD_FPM_GLOBAL_CFG_DTS) + 4, v)
#define RDD_FPM_GLOBAL_CFG_FPM_BASE_HIGH_READ(r, p)                        MREAD_32((uint8_t *)p + 4, r)
#define RDD_FPM_GLOBAL_CFG_FPM_BASE_HIGH_WRITE(v, p)                       MWRITE_32((uint8_t *)p + 4, v)
#define RDD_FPM_GLOBAL_CFG_FPM_TOKEN_SIZE_ASR_8_READ_G(r, g, idx)          GROUP_MREAD_8(g, idx*sizeof(RDD_FPM_GLOBAL_CFG_DTS) + 8, r)
#define RDD_FPM_GLOBAL_CFG_FPM_TOKEN_SIZE_ASR_8_WRITE_G(v, g, idx)         GROUP_MWRITE_8(g, idx*sizeof(RDD_FPM_GLOBAL_CFG_DTS) + 8, v)
#define RDD_FPM_GLOBAL_CFG_FPM_TOKEN_SIZE_ASR_8_READ(r, p)                 MREAD_8((uint8_t *)p + 8, r)
#define RDD_FPM_GLOBAL_CFG_FPM_TOKEN_SIZE_ASR_8_WRITE(v, p)                MWRITE_8((uint8_t *)p + 8, v)
/* <<<RDD_FPM_GLOBAL_CFG_DTS */

typedef struct
{
	RDD_FPM_GLOBAL_CFG_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FPM_GLOBAL_CFG_TABLE_DTS;

extern uint32_t RDD_FPM_GLOBAL_CFG_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_FPM_GLOBAL_CFG_PTR(core_id)	( RDD_FPM_GLOBAL_CFG_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_FPM_GLOBAL_CFG_ADDRESS_ARR[core_id] ))

/* <<<FPM_GLOBAL_CFG */


/* >>>DS_TM_BBH_QUEUE_TABLE */

/* >>>RDD_BBH_QUEUE_DESCRIPTOR_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	reserved       	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mirroring_en   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	priority       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	scheduler_index	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	scheduler_type 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bb_destination 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ingress_counter	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	ingress_counter	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bb_destination 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	scheduler_type 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	scheduler_index	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	priority       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mirroring_en   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved       	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BBH_QUEUE_DESCRIPTOR_DTS;

#define RDD_BBH_QUEUE_DESCRIPTOR_MIRRORING_EN_READ_G(r, g, idx)             GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BBH_QUEUE_DESCRIPTOR_DTS), 1, 1, r)
#define RDD_BBH_QUEUE_DESCRIPTOR_MIRRORING_EN_WRITE_G(v, g, idx)            GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BBH_QUEUE_DESCRIPTOR_DTS), 1, 1, v)
#define RDD_BBH_QUEUE_DESCRIPTOR_MIRRORING_EN_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p, 1, 1, r)
#define RDD_BBH_QUEUE_DESCRIPTOR_MIRRORING_EN_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p, 1, 1, v)
#define RDD_BBH_QUEUE_DESCRIPTOR_PRIORITY_READ_G(r, g, idx)                 GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BBH_QUEUE_DESCRIPTOR_DTS), 0, 1, r)
#define RDD_BBH_QUEUE_DESCRIPTOR_PRIORITY_WRITE_G(v, g, idx)                GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BBH_QUEUE_DESCRIPTOR_DTS), 0, 1, v)
#define RDD_BBH_QUEUE_DESCRIPTOR_PRIORITY_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p, 0, 1, r)
#define RDD_BBH_QUEUE_DESCRIPTOR_PRIORITY_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p, 0, 1, v)
#define RDD_BBH_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BBH_QUEUE_DESCRIPTOR_DTS) + 1, 1, 7, r)
#define RDD_BBH_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BBH_QUEUE_DESCRIPTOR_DTS) + 1, 1, 7, v)
#define RDD_BBH_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 1, 1, 7, r)
#define RDD_BBH_QUEUE_DESCRIPTOR_SCHEDULER_INDEX_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 1, 1, 7, v)
#define RDD_BBH_QUEUE_DESCRIPTOR_SCHEDULER_TYPE_READ_G(r, g, idx)           GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BBH_QUEUE_DESCRIPTOR_DTS) + 1, 0, 1, r)
#define RDD_BBH_QUEUE_DESCRIPTOR_SCHEDULER_TYPE_WRITE_G(v, g, idx)          GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BBH_QUEUE_DESCRIPTOR_DTS) + 1, 0, 1, v)
#define RDD_BBH_QUEUE_DESCRIPTOR_SCHEDULER_TYPE_READ(r, p)                  FIELD_MREAD_8((uint8_t *)p + 1, 0, 1, r)
#define RDD_BBH_QUEUE_DESCRIPTOR_SCHEDULER_TYPE_WRITE(v, p)                 FIELD_MWRITE_8((uint8_t *)p + 1, 0, 1, v)
#define RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_READ_G(r, g, idx)           GROUP_MREAD_8(g, idx*sizeof(RDD_BBH_QUEUE_DESCRIPTOR_DTS) + 2, r)
#define RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_WRITE_G(v, g, idx)          GROUP_MWRITE_8(g, idx*sizeof(RDD_BBH_QUEUE_DESCRIPTOR_DTS) + 2, v)
#define RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_READ(r, p)                  MREAD_8((uint8_t *)p + 2, r)
#define RDD_BBH_QUEUE_DESCRIPTOR_BB_DESTINATION_WRITE(v, p)                 MWRITE_8((uint8_t *)p + 2, v)
#define RDD_BBH_QUEUE_DESCRIPTOR_INGRESS_COUNTER_READ_G(r, g, idx)          GROUP_MREAD_8(g, idx*sizeof(RDD_BBH_QUEUE_DESCRIPTOR_DTS) + 3, r)
#define RDD_BBH_QUEUE_DESCRIPTOR_INGRESS_COUNTER_WRITE_G(v, g, idx)         GROUP_MWRITE_8(g, idx*sizeof(RDD_BBH_QUEUE_DESCRIPTOR_DTS) + 3, v)
#define RDD_BBH_QUEUE_DESCRIPTOR_INGRESS_COUNTER_READ(r, p)                 MREAD_8((uint8_t *)p + 3, r)
#define RDD_BBH_QUEUE_DESCRIPTOR_INGRESS_COUNTER_WRITE(v, p)                MWRITE_8((uint8_t *)p + 3, v)
/* <<<RDD_BBH_QUEUE_DESCRIPTOR_DTS */


#define RDD_DS_TM_BBH_QUEUE_TABLE_SIZE     8
typedef struct
{
	RDD_BBH_QUEUE_DESCRIPTOR_DTS	entry[ RDD_DS_TM_BBH_QUEUE_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_TM_BBH_QUEUE_TABLE_DTS;

extern uint32_t RDD_DS_TM_BBH_QUEUE_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_DS_TM_BBH_QUEUE_TABLE_PTR(core_id)	( RDD_DS_TM_BBH_QUEUE_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_DS_TM_BBH_QUEUE_TABLE_ADDRESS_ARR[core_id] ))

/* <<<DS_TM_BBH_QUEUE_TABLE */


/* >>>COMPLEX_SCHEDULER_TABLE */

/* >>>RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS */
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RESERVED1_NUMBER	12
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BLOCK_INDEX_NUMBER	32

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	rate_limit_enable                                                   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	is_positive_budget                                                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	basic_scheduler_exists                                              	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbh_queue                                                           	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_limiter_index                                                  	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dwrr_offset_pir                                                     	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dwrr_offset_sir                                                     	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	last_served_block_pir                                               	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	last_served_block_sir                                               	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	status_bit_vector                                                   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	slot_budget_bit_vector_0                                            	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	slot_budget_bit_vector_1                                            	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ovl_rl_en                                                           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved3                                                           	:31	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	reserved1[RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RESERVED1_NUMBER];
	uint8_t	block_index[RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BLOCK_INDEX_NUMBER];
#else
	uint32_t	last_served_block_sir                                               	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	last_served_block_pir                                               	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dwrr_offset_sir                                                     	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dwrr_offset_pir                                                     	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_limiter_index                                                  	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbh_queue                                                           	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	basic_scheduler_exists                                              	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	is_positive_budget                                                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rate_limit_enable                                                   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	status_bit_vector                                                   	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	slot_budget_bit_vector_0                                            	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	slot_budget_bit_vector_1                                            	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved3                                                           	:31	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ovl_rl_en                                                           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	reserved1[RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RESERVED1_NUMBER];
	uint8_t	block_index[RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BLOCK_INDEX_NUMBER];
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS;

#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_READ_G(r, g, idx)                 GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS), 7, 1, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE_G(v, g, idx)                GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS), 7, 1, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RATE_LIMIT_ENABLE_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS), 6, 1, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS), 6, 1, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p, 6, 1, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_IS_POSITIVE_BUDGET_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p, 6, 1, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BASIC_SCHEDULER_EXISTS_READ_G(r, g, idx)            GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS), 5, 1, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BASIC_SCHEDULER_EXISTS_WRITE_G(v, g, idx)           GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS), 5, 1, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BASIC_SCHEDULER_EXISTS_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p, 5, 1, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BASIC_SCHEDULER_EXISTS_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p, 5, 1, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BBH_QUEUE_READ_G(r, g, idx)                         GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS), 7, 6, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BBH_QUEUE_WRITE_G(v, g, idx)                        GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS), 7, 6, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BBH_QUEUE_READ(r, p)                                FIELD_MREAD_16((uint8_t *)p, 7, 6, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BBH_QUEUE_WRITE(v, p)                               FIELD_MWRITE_16((uint8_t *)p, 7, 6, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS) + 1, 0, 7, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS) + 1, 0, 7, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 1, 0, 7, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_RATE_LIMITER_INDEX_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 1, 0, 7, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_PIR_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS) + 2, 5, 3, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_PIR_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS) + 2, 5, 3, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_PIR_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 2, 5, 3, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_PIR_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 2, 5, 3, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_SIR_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS) + 2, 2, 3, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_SIR_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS) + 2, 2, 3, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_SIR_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 2, 2, 3, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DWRR_OFFSET_SIR_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 2, 2, 3, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_PIR_READ_G(r, g, idx)             GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS) + 2, 5, 5, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_PIR_WRITE_G(v, g, idx)            GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS) + 2, 5, 5, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_PIR_READ(r, p)                    FIELD_MREAD_16((uint8_t *)p + 2, 5, 5, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_PIR_WRITE(v, p)                   FIELD_MWRITE_16((uint8_t *)p + 2, 5, 5, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_SIR_READ_G(r, g, idx)             GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS) + 3, 0, 5, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_SIR_WRITE_G(v, g, idx)            GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS) + 3, 0, 5, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_SIR_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 3, 0, 5, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_LAST_SERVED_BLOCK_SIR_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 3, 0, 5, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_READ_G(r, g, idx)                 GROUP_MREAD_32(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS) + 4, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_WRITE_G(v, g, idx)                GROUP_MWRITE_32(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS) + 4, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_READ(r, p)                        MREAD_32((uint8_t *)p + 4, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_STATUS_BIT_VECTOR_WRITE(v, p)                       MWRITE_32((uint8_t *)p + 4, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_READ_G(r, g, idx)          GROUP_MREAD_32(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS) + 8, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_WRITE_G(v, g, idx)         GROUP_MWRITE_32(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS) + 8, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_READ(r, p)                 MREAD_32((uint8_t *)p + 8, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_0_WRITE(v, p)                MWRITE_32((uint8_t *)p + 8, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_1_READ_G(r, g, idx)          GROUP_MREAD_32(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS) + 12, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_1_WRITE_G(v, g, idx)         GROUP_MWRITE_32(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS) + 12, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_1_READ(r, p)                 MREAD_32((uint8_t *)p + 12, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_SLOT_BUDGET_BIT_VECTOR_1_WRITE(v, p)                MWRITE_32((uint8_t *)p + 12, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_OVL_RL_EN_READ_G(r, g, idx)                         GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS) + 16, 7, 1, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_OVL_RL_EN_WRITE_G(v, g, idx)                        GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS) + 16, 7, 1, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_OVL_RL_EN_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 16, 7, 1, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_OVL_RL_EN_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 16, 7, 1, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BLOCK_INDEX_READ_G(r, g, idx, i)                    GROUP_MREAD_I_8(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS) + 32, i, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BLOCK_INDEX_WRITE_G(v, g, idx, i)                   GROUP_MWRITE_I_8(g, idx*sizeof(RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS) + 32, i, v)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BLOCK_INDEX_READ(r, p, i)                           MREAD_I_8((uint8_t *)p + 32, i, r)
#define RDD_COMPLEX_SCHEDULER_DESCRIPTOR_BLOCK_INDEX_WRITE(v, p, i)                          MWRITE_I_8((uint8_t *)p + 32, i, v)
/* <<<RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS */


#define RDD_COMPLEX_SCHEDULER_TABLE_SIZE     16
typedef struct
{
	RDD_COMPLEX_SCHEDULER_DESCRIPTOR_DTS	entry[ RDD_COMPLEX_SCHEDULER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_COMPLEX_SCHEDULER_TABLE_DTS;

extern uint32_t RDD_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_COMPLEX_SCHEDULER_TABLE_PTR(core_id)	( RDD_COMPLEX_SCHEDULER_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_COMPLEX_SCHEDULER_TABLE_ADDRESS_ARR[core_id] ))

/* <<<COMPLEX_SCHEDULER_TABLE */


/* >>>RUNNER_PROFILING_TRACE_BUFFER */

/* >>>RDD_TRACE_EVENT_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	timestamp              	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	event_id               	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	trace_event_info       	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = trace_event_info, size = 18 bits
	uint32_t	incoming_task_num      	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	task_pc                	:13	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	incoming_bbhrx_src_addr	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dma_wr                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dma_rd                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dma_wr_reply           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ramrd                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	parser                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbhtx                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbhrx_async            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbhrx_sync             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cpu                    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	fw_self                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	fw                     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	timer                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2              	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	acc_type               	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
#else
	uint32_t	trace_event_info       	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = trace_event_info, size = 18 bits
	uint32_t	incoming_task_num      	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	task_pc                	:13	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	incoming_bbhrx_src_addr	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dma_wr                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dma_rd                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dma_wr_reply           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ramrd                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	parser                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbhtx                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbhrx_async            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbhrx_sync             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cpu                    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	fw_self                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	fw                     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	timer                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2              	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	acc_type               	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	event_id               	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	timestamp              	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_TRACE_EVENT_DTS;

#define RDD_TRACE_EVENT_TIMESTAMP_READ_G(r, g, idx)                        GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_TRACE_EVENT_DTS), 4, 12, r)
#define RDD_TRACE_EVENT_TIMESTAMP_WRITE_G(v, g, idx)                       GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_TRACE_EVENT_DTS), 4, 12, v)
#define RDD_TRACE_EVENT_TIMESTAMP_READ(r, p)                               FIELD_MREAD_16((uint8_t *)p, 4, 12, r)
#define RDD_TRACE_EVENT_TIMESTAMP_WRITE(v, p)                              FIELD_MWRITE_16((uint8_t *)p, 4, 12, v)
#define RDD_TRACE_EVENT_EVENT_ID_READ_G(r, g, idx)                         GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 1, 2, 2, r)
#define RDD_TRACE_EVENT_EVENT_ID_WRITE_G(v, g, idx)                        GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 1, 2, 2, v)
#define RDD_TRACE_EVENT_EVENT_ID_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 1, 2, 2, r)
#define RDD_TRACE_EVENT_EVENT_ID_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 1, 2, 2, v)
#define RDD_TRACE_EVENT_TRACE_EVENT_INFO_READ_G(r, g, idx)                 GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 0, 0, 18, r)
#define RDD_TRACE_EVENT_TRACE_EVENT_INFO_WRITE_G(v, g, idx)                GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 0, 0, 18, v)
#define RDD_TRACE_EVENT_TRACE_EVENT_INFO_READ(r, p)                        FIELD_MREAD_32((uint8_t *)p + 0, 0, 18, r)
#define RDD_TRACE_EVENT_TRACE_EVENT_INFO_WRITE(v, p)                       FIELD_MWRITE_32((uint8_t *)p + 0, 0, 18, v)
#define RDD_TRACE_EVENT_INCOMING_TASK_NUM_READ_G(r, g, idx)                GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 0, 14, 4, r)
#define RDD_TRACE_EVENT_INCOMING_TASK_NUM_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 0, 14, 4, v)
#define RDD_TRACE_EVENT_INCOMING_TASK_NUM_READ(r, p)                       FIELD_MREAD_32((uint8_t *)p + 0, 14, 4, r)
#define RDD_TRACE_EVENT_INCOMING_TASK_NUM_WRITE(v, p)                      FIELD_MWRITE_32((uint8_t *)p + 0, 14, 4, v)
#define RDD_TRACE_EVENT_TASK_PC_READ_G(r, g, idx)                          GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 2, 0, 13, r)
#define RDD_TRACE_EVENT_TASK_PC_WRITE_G(v, g, idx)                         GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 2, 0, 13, v)
#define RDD_TRACE_EVENT_TASK_PC_READ(r, p)                                 FIELD_MREAD_16((uint8_t *)p + 2, 0, 13, r)
#define RDD_TRACE_EVENT_TASK_PC_WRITE(v, p)                                FIELD_MWRITE_16((uint8_t *)p + 2, 0, 13, v)
#define RDD_TRACE_EVENT_INCOMING_BBHRX_SRC_ADDR_READ_G(r, g, idx)          GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 0, 12, 6, r)
#define RDD_TRACE_EVENT_INCOMING_BBHRX_SRC_ADDR_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 0, 12, 6, v)
#define RDD_TRACE_EVENT_INCOMING_BBHRX_SRC_ADDR_READ(r, p)                 FIELD_MREAD_32((uint8_t *)p + 0, 12, 6, r)
#define RDD_TRACE_EVENT_INCOMING_BBHRX_SRC_ADDR_WRITE(v, p)                FIELD_MWRITE_32((uint8_t *)p + 0, 12, 6, v)
#define RDD_TRACE_EVENT_DMA_WR_READ_G(r, g, idx)                           GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 2, 3, 1, r)
#define RDD_TRACE_EVENT_DMA_WR_WRITE_G(v, g, idx)                          GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 2, 3, 1, v)
#define RDD_TRACE_EVENT_DMA_WR_READ(r, p)                                  FIELD_MREAD_8((uint8_t *)p + 2, 3, 1, r)
#define RDD_TRACE_EVENT_DMA_WR_WRITE(v, p)                                 FIELD_MWRITE_8((uint8_t *)p + 2, 3, 1, v)
#define RDD_TRACE_EVENT_DMA_RD_READ_G(r, g, idx)                           GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 2, 2, 1, r)
#define RDD_TRACE_EVENT_DMA_RD_WRITE_G(v, g, idx)                          GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 2, 2, 1, v)
#define RDD_TRACE_EVENT_DMA_RD_READ(r, p)                                  FIELD_MREAD_8((uint8_t *)p + 2, 2, 1, r)
#define RDD_TRACE_EVENT_DMA_RD_WRITE(v, p)                                 FIELD_MWRITE_8((uint8_t *)p + 2, 2, 1, v)
#define RDD_TRACE_EVENT_DMA_WR_REPLY_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 2, 1, 1, r)
#define RDD_TRACE_EVENT_DMA_WR_REPLY_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 2, 1, 1, v)
#define RDD_TRACE_EVENT_DMA_WR_REPLY_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 2, 1, 1, r)
#define RDD_TRACE_EVENT_DMA_WR_REPLY_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 2, 1, 1, v)
#define RDD_TRACE_EVENT_RAMRD_READ_G(r, g, idx)                            GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 2, 0, 1, r)
#define RDD_TRACE_EVENT_RAMRD_WRITE_G(v, g, idx)                           GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 2, 0, 1, v)
#define RDD_TRACE_EVENT_RAMRD_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p + 2, 0, 1, r)
#define RDD_TRACE_EVENT_RAMRD_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p + 2, 0, 1, v)
#define RDD_TRACE_EVENT_PARSER_READ_G(r, g, idx)                           GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 3, 7, 1, r)
#define RDD_TRACE_EVENT_PARSER_WRITE_G(v, g, idx)                          GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 3, 7, 1, v)
#define RDD_TRACE_EVENT_PARSER_READ(r, p)                                  FIELD_MREAD_8((uint8_t *)p + 3, 7, 1, r)
#define RDD_TRACE_EVENT_PARSER_WRITE(v, p)                                 FIELD_MWRITE_8((uint8_t *)p + 3, 7, 1, v)
#define RDD_TRACE_EVENT_BBHTX_READ_G(r, g, idx)                            GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 3, 6, 1, r)
#define RDD_TRACE_EVENT_BBHTX_WRITE_G(v, g, idx)                           GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 3, 6, 1, v)
#define RDD_TRACE_EVENT_BBHTX_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p + 3, 6, 1, r)
#define RDD_TRACE_EVENT_BBHTX_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p + 3, 6, 1, v)
#define RDD_TRACE_EVENT_BBHRX_ASYNC_READ_G(r, g, idx)                      GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 3, 5, 1, r)
#define RDD_TRACE_EVENT_BBHRX_ASYNC_WRITE_G(v, g, idx)                     GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 3, 5, 1, v)
#define RDD_TRACE_EVENT_BBHRX_ASYNC_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 3, 5, 1, r)
#define RDD_TRACE_EVENT_BBHRX_ASYNC_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 3, 5, 1, v)
#define RDD_TRACE_EVENT_BBHRX_SYNC_READ_G(r, g, idx)                       GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 3, 4, 1, r)
#define RDD_TRACE_EVENT_BBHRX_SYNC_WRITE_G(v, g, idx)                      GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 3, 4, 1, v)
#define RDD_TRACE_EVENT_BBHRX_SYNC_READ(r, p)                              FIELD_MREAD_8((uint8_t *)p + 3, 4, 1, r)
#define RDD_TRACE_EVENT_BBHRX_SYNC_WRITE(v, p)                             FIELD_MWRITE_8((uint8_t *)p + 3, 4, 1, v)
#define RDD_TRACE_EVENT_CPU_READ_G(r, g, idx)                              GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 3, 3, 1, r)
#define RDD_TRACE_EVENT_CPU_WRITE_G(v, g, idx)                             GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 3, 3, 1, v)
#define RDD_TRACE_EVENT_CPU_READ(r, p)                                     FIELD_MREAD_8((uint8_t *)p + 3, 3, 1, r)
#define RDD_TRACE_EVENT_CPU_WRITE(v, p)                                    FIELD_MWRITE_8((uint8_t *)p + 3, 3, 1, v)
#define RDD_TRACE_EVENT_FW_SELF_READ_G(r, g, idx)                          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 3, 2, 1, r)
#define RDD_TRACE_EVENT_FW_SELF_WRITE_G(v, g, idx)                         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 3, 2, 1, v)
#define RDD_TRACE_EVENT_FW_SELF_READ(r, p)                                 FIELD_MREAD_8((uint8_t *)p + 3, 2, 1, r)
#define RDD_TRACE_EVENT_FW_SELF_WRITE(v, p)                                FIELD_MWRITE_8((uint8_t *)p + 3, 2, 1, v)
#define RDD_TRACE_EVENT_FW_READ_G(r, g, idx)                               GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 3, 1, 1, r)
#define RDD_TRACE_EVENT_FW_WRITE_G(v, g, idx)                              GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 3, 1, 1, v)
#define RDD_TRACE_EVENT_FW_READ(r, p)                                      FIELD_MREAD_8((uint8_t *)p + 3, 1, 1, r)
#define RDD_TRACE_EVENT_FW_WRITE(v, p)                                     FIELD_MWRITE_8((uint8_t *)p + 3, 1, 1, v)
#define RDD_TRACE_EVENT_TIMER_READ_G(r, g, idx)                            GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 3, 0, 1, r)
#define RDD_TRACE_EVENT_TIMER_WRITE_G(v, g, idx)                           GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 3, 0, 1, v)
#define RDD_TRACE_EVENT_TIMER_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p + 3, 0, 1, r)
#define RDD_TRACE_EVENT_TIMER_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p + 3, 0, 1, v)
#define RDD_TRACE_EVENT_ACC_TYPE_READ_G(r, g, idx)                         GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 2, 0, 12, r)
#define RDD_TRACE_EVENT_ACC_TYPE_WRITE_G(v, g, idx)                        GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_TRACE_EVENT_DTS) + 2, 0, 12, v)
#define RDD_TRACE_EVENT_ACC_TYPE_READ(r, p)                                FIELD_MREAD_16((uint8_t *)p + 2, 0, 12, r)
#define RDD_TRACE_EVENT_ACC_TYPE_WRITE(v, p)                               FIELD_MWRITE_16((uint8_t *)p + 2, 0, 12, v)
/* <<<RDD_TRACE_EVENT_DTS */


#define RDD_RUNNER_PROFILING_TRACE_BUFFER_SIZE     128
typedef struct
{
	RDD_TRACE_EVENT_DTS	entry[ RDD_RUNNER_PROFILING_TRACE_BUFFER_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RUNNER_PROFILING_TRACE_BUFFER_DTS;

extern uint32_t RDD_RUNNER_PROFILING_TRACE_BUFFER_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_RUNNER_PROFILING_TRACE_BUFFER_PTR(core_id)	( RDD_RUNNER_PROFILING_TRACE_BUFFER_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_RUNNER_PROFILING_TRACE_BUFFER_ADDRESS_ARR[core_id] ))

/* <<<RUNNER_PROFILING_TRACE_BUFFER */

/* CORE_1 */

/* >>>CPU_RECYCLE_SRAM_PD_FIFO */

#define RDD_CPU_RECYCLE_SRAM_PD_FIFO_SIZE     16
typedef struct
{
	RDD_PROCESSING_TX_DESCRIPTOR_DTS	entry[ RDD_CPU_RECYCLE_SRAM_PD_FIFO_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RECYCLE_SRAM_PD_FIFO_DTS;

extern uint32_t RDD_CPU_RECYCLE_SRAM_PD_FIFO_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_RECYCLE_SRAM_PD_FIFO_PTR(core_id)	( RDD_CPU_RECYCLE_SRAM_PD_FIFO_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_RECYCLE_SRAM_PD_FIFO_ADDRESS_ARR[core_id] ))

/* <<<CPU_RECYCLE_SRAM_PD_FIFO */


/* >>>CPU_RX_COPY_PD_FIFO_TABLE */

#define RDD_CPU_RX_COPY_PD_FIFO_TABLE_SIZE     4
typedef struct
{
	RDD_PROCESSING_TX_DESCRIPTOR_DTS	entry[ RDD_CPU_RX_COPY_PD_FIFO_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_COPY_PD_FIFO_TABLE_DTS;

extern uint32_t RDD_CPU_RX_COPY_PD_FIFO_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_RX_COPY_PD_FIFO_TABLE_PTR(core_id)	( RDD_CPU_RX_COPY_PD_FIFO_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_RX_COPY_PD_FIFO_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_RX_COPY_PD_FIFO_TABLE */


/* >>>DIRECT_PROCESSING_PD_TABLE */

/* >>>RDD_PROCESSING_RX_DESCRIPTOR_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	pd_info           	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = pd_info, size = 32 bits
	uint32_t	key_index         	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	fragment_type     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	options           	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0         	:11	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ctrl_key_index    	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ctrl_fragment_type	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sfc               	:10	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1         	:19	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	g9991_tci_sof     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	g9991_tci_eof     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	g9991_tci_const   	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	g9991_tci_sid_1_0 	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	g9991_tci_sid_9_2 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	g9991_length_time 	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	timestamp         	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	serial_num        	:10	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ploam             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bn_num            	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	packet_length     	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	error             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	target_mem_1      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cong_state        	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ingress_cong      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	lan               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ingress_port      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = ingress_port, size = 8 bits
	uint32_t	flow              	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	source_port       	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	bn1_last_or_abs1  	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = bn1_last_or_abs1, size = 18 bits
	uint32_t	bn1_last          	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs1              	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	agg_pd            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	target_mem_0      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	error_type_or_cpu_tx	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = error_type_or_cpu_tx, size = 4 bits
	uint32_t	error_type        	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cpu_tx            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2         	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	sop               	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bn0_first         	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	pd_info           	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = pd_info, size = 32 bits
	uint32_t	key_index         	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	fragment_type     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	options           	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0         	:11	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ctrl_key_index    	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ctrl_fragment_type	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sfc               	:10	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1         	:19	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	g9991_tci_sof     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	g9991_tci_eof     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	g9991_tci_const   	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	g9991_tci_sid_1_0 	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	g9991_tci_sid_9_2 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	g9991_length_time 	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	timestamp         	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	packet_length     	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bn_num            	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ploam             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	serial_num        	:10	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bn1_last_or_abs1  	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = bn1_last_or_abs1, size = 18 bits
	uint32_t	bn1_last          	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs1              	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	ingress_port      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = ingress_port, size = 8 bits
	uint32_t	flow              	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	source_port       	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	lan               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ingress_cong      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cong_state        	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	target_mem_1      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	error             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bn0_first         	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sop               	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	error_type_or_cpu_tx	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = error_type_or_cpu_tx, size = 4 bits
	uint32_t	error_type        	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cpu_tx            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2         	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint32_t	abs               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	target_mem_0      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	agg_pd            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PROCESSING_RX_DESCRIPTOR_DTS;

#define RDD_PROCESSING_RX_DESCRIPTOR_PD_INFO_READ_G(r, g, idx)                     GROUP_MREAD_32(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), r)
#define RDD_PROCESSING_RX_DESCRIPTOR_PD_INFO_WRITE_G(v, g, idx)                    GROUP_MWRITE_32(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), v)
#define RDD_PROCESSING_RX_DESCRIPTOR_PD_INFO_READ(r, p)                            MREAD_32((uint8_t *)p, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_PD_INFO_WRITE(v, p)                           MWRITE_32((uint8_t *)p, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_KEY_INDEX_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), 6, 2, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_KEY_INDEX_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), 6, 2, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_KEY_INDEX_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p, 6, 2, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_KEY_INDEX_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p, 6, 2, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_FRAGMENT_TYPE_READ_G(r, g, idx)               GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), 5, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_FRAGMENT_TYPE_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), 5, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_FRAGMENT_TYPE_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p, 5, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_FRAGMENT_TYPE_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p, 5, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_OPTIONS_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), 11, 18, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_OPTIONS_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), 11, 18, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_OPTIONS_READ(r, p)                            FIELD_MREAD_32((uint8_t *)p, 11, 18, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_OPTIONS_WRITE(v, p)                           FIELD_MWRITE_32((uint8_t *)p, 11, 18, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_CTRL_KEY_INDEX_READ_G(r, g, idx)              GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), 6, 2, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_CTRL_KEY_INDEX_WRITE_G(v, g, idx)             GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), 6, 2, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_CTRL_KEY_INDEX_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p, 6, 2, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_CTRL_KEY_INDEX_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p, 6, 2, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_CTRL_FRAGMENT_TYPE_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), 5, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_CTRL_FRAGMENT_TYPE_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), 5, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_CTRL_FRAGMENT_TYPE_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 5, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_CTRL_FRAGMENT_TYPE_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 5, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_SFC_READ_G(r, g, idx)                         GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), 3, 10, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_SFC_WRITE_G(v, g, idx)                        GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), 3, 10, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_SFC_READ(r, p)                                FIELD_MREAD_16((uint8_t *)p, 3, 10, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_SFC_WRITE(v, p)                               FIELD_MWRITE_16((uint8_t *)p, 3, 10, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_TCI_SOF_READ_G(r, g, idx)               GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), 7, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_TCI_SOF_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), 7, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_TCI_SOF_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_TCI_SOF_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_TCI_EOF_READ_G(r, g, idx)               GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), 6, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_TCI_EOF_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), 6, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_TCI_EOF_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p, 6, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_TCI_EOF_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p, 6, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_TCI_CONST_READ_G(r, g, idx)             GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), 2, 4, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_TCI_CONST_WRITE_G(v, g, idx)            GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), 2, 4, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_TCI_CONST_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p, 2, 4, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_TCI_CONST_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p, 2, 4, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_TCI_SID_1_0_READ_G(r, g, idx)           GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), 0, 2, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_TCI_SID_1_0_WRITE_G(v, g, idx)          GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), 0, 2, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_TCI_SID_1_0_READ(r, p)                  FIELD_MREAD_8((uint8_t *)p, 0, 2, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_TCI_SID_1_0_WRITE(v, p)                 FIELD_MWRITE_8((uint8_t *)p, 0, 2, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_TCI_SID_9_2_READ_G(r, g, idx)           GROUP_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_TCI_SID_9_2_WRITE_G(v, g, idx)          GROUP_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_TCI_SID_9_2_READ(r, p)                  MREAD_8((uint8_t *)p + 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_TCI_SID_9_2_WRITE(v, p)                 MWRITE_8((uint8_t *)p + 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_LENGTH_TIME_READ_G(r, g, idx)           GROUP_MREAD_16(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 2, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_LENGTH_TIME_WRITE_G(v, g, idx)          GROUP_MWRITE_16(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 2, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_LENGTH_TIME_READ(r, p)                  MREAD_16((uint8_t *)p + 2, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_G9991_LENGTH_TIME_WRITE(v, p)                 MWRITE_16((uint8_t *)p + 2, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_TIMESTAMP_READ_G(r, g, idx)                   GROUP_MREAD_32(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), r)
#define RDD_PROCESSING_RX_DESCRIPTOR_TIMESTAMP_WRITE_G(v, g, idx)                  GROUP_MWRITE_32(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS), v)
#define RDD_PROCESSING_RX_DESCRIPTOR_TIMESTAMP_READ(r, p)                          MREAD_32((uint8_t *)p, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_TIMESTAMP_WRITE(v, p)                         MWRITE_32((uint8_t *)p, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_SERIAL_NUM_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 4, 6, 10, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_SERIAL_NUM_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 4, 6, 10, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_SERIAL_NUM_READ(r, p)                         FIELD_MREAD_16((uint8_t *)p + 4, 6, 10, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_SERIAL_NUM_WRITE(v, p)                        FIELD_MWRITE_16((uint8_t *)p + 4, 6, 10, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_PLOAM_READ_G(r, g, idx)                       GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 5, 5, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_PLOAM_WRITE_G(v, g, idx)                      GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 5, 5, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_PLOAM_READ(r, p)                              FIELD_MREAD_8((uint8_t *)p + 5, 5, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_PLOAM_WRITE(v, p)                             FIELD_MWRITE_8((uint8_t *)p + 5, 5, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_BN_NUM_READ_G(r, g, idx)                      GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 4, 14, 7, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_BN_NUM_WRITE_G(v, g, idx)                     GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 4, 14, 7, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_BN_NUM_READ(r, p)                             FIELD_MREAD_32((uint8_t *)p + 4, 14, 7, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_BN_NUM_WRITE(v, p)                            FIELD_MWRITE_32((uint8_t *)p + 4, 14, 7, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_PACKET_LENGTH_READ_G(r, g, idx)               GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 6, 0, 14, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_PACKET_LENGTH_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 6, 0, 14, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_PACKET_LENGTH_READ(r, p)                      FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_PACKET_LENGTH_WRITE(v, p)                     FIELD_MWRITE_16((uint8_t *)p + 6, 0, 14, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_ERROR_READ_G(r, g, idx)                       GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 7, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_ERROR_WRITE_G(v, g, idx)                      GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 7, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_ERROR_READ(r, p)                              FIELD_MREAD_8((uint8_t *)p + 8, 7, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_ERROR_WRITE(v, p)                             FIELD_MWRITE_8((uint8_t *)p + 8, 7, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_TARGET_MEM_1_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 6, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_TARGET_MEM_1_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 6, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_TARGET_MEM_1_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 8, 6, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_TARGET_MEM_1_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 8, 6, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_CONG_STATE_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 4, 2, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_CONG_STATE_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 4, 2, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_CONG_STATE_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 8, 4, 2, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_CONG_STATE_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 8, 4, 2, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_INGRESS_CONG_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 3, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_INGRESS_CONG_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 3, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_INGRESS_CONG_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 8, 3, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_INGRESS_CONG_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 8, 3, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_LAN_READ_G(r, g, idx)                         GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 2, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_LAN_WRITE_G(v, g, idx)                        GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 2, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_LAN_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 8, 2, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_LAN_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 8, 2, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_INGRESS_PORT_READ_G(r, g, idx)                GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 2, 8, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_INGRESS_PORT_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 2, 8, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_INGRESS_PORT_READ(r, p)                       FIELD_MREAD_16((uint8_t *)p + 8, 2, 8, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_INGRESS_PORT_WRITE(v, p)                      FIELD_MWRITE_16((uint8_t *)p + 8, 2, 8, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_FLOW_READ_G(r, g, idx)                        GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 2, 8, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_FLOW_WRITE_G(v, g, idx)                       GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 2, 8, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_FLOW_READ(r, p)                               FIELD_MREAD_16((uint8_t *)p + 8, 2, 8, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_FLOW_WRITE(v, p)                              FIELD_MWRITE_16((uint8_t *)p + 8, 2, 8, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_SOURCE_PORT_READ_G(r, g, idx)                 GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 2, 8, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_SOURCE_PORT_WRITE_G(v, g, idx)                GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 2, 8, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_SOURCE_PORT_READ(r, p)                        FIELD_MREAD_16((uint8_t *)p + 8, 2, 8, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_SOURCE_PORT_WRITE(v, p)                       FIELD_MWRITE_16((uint8_t *)p + 8, 2, 8, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_BN1_LAST_OR_ABS1_READ_G(r, g, idx)            GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 0, 18, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_BN1_LAST_OR_ABS1_WRITE_G(v, g, idx)           GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 0, 18, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_BN1_LAST_OR_ABS1_READ(r, p)                   FIELD_MREAD_32((uint8_t *)p + 8, 0, 18, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_BN1_LAST_OR_ABS1_WRITE(v, p)                  FIELD_MWRITE_32((uint8_t *)p + 8, 0, 18, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_BN1_LAST_READ_G(r, g, idx)                    GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 0, 18, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_BN1_LAST_WRITE_G(v, g, idx)                   GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 0, 18, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_BN1_LAST_READ(r, p)                           FIELD_MREAD_32((uint8_t *)p + 8, 0, 18, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_BN1_LAST_WRITE(v, p)                          FIELD_MWRITE_32((uint8_t *)p + 8, 0, 18, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_ABS1_READ_G(r, g, idx)                        GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 0, 18, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_ABS1_WRITE_G(v, g, idx)                       GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 8, 0, 18, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_ABS1_READ(r, p)                               FIELD_MREAD_32((uint8_t *)p + 8, 0, 18, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_ABS1_WRITE(v, p)                              FIELD_MWRITE_32((uint8_t *)p + 8, 0, 18, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_AGG_PD_READ_G(r, g, idx)                      GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 12, 7, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_AGG_PD_WRITE_G(v, g, idx)                     GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 12, 7, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_AGG_PD_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 12, 7, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_AGG_PD_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 12, 7, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_TARGET_MEM_0_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 12, 6, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_TARGET_MEM_0_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 12, 6, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_TARGET_MEM_0_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 12, 6, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_TARGET_MEM_0_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 12, 6, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_ABS_READ_G(r, g, idx)                         GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 12, 5, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_ABS_WRITE_G(v, g, idx)                        GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 12, 5, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_ABS_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 12, 5, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_ABS_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 12, 5, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_ERROR_TYPE_OR_CPU_TX_READ_G(r, g, idx)        GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 12, 1, 4, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_ERROR_TYPE_OR_CPU_TX_WRITE_G(v, g, idx)       GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 12, 1, 4, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_ERROR_TYPE_OR_CPU_TX_READ(r, p)               FIELD_MREAD_8((uint8_t *)p + 12, 1, 4, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_ERROR_TYPE_OR_CPU_TX_WRITE(v, p)              FIELD_MWRITE_8((uint8_t *)p + 12, 1, 4, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_ERROR_TYPE_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 12, 1, 4, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_ERROR_TYPE_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 12, 1, 4, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_ERROR_TYPE_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 12, 1, 4, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_ERROR_TYPE_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 12, 1, 4, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_CPU_TX_READ_G(r, g, idx)                      GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 12, 4, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_CPU_TX_WRITE_G(v, g, idx)                     GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 12, 4, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_CPU_TX_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 12, 4, 1, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_CPU_TX_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 12, 4, 1, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_SOP_READ_G(r, g, idx)                         GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 12, 2, 7, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_SOP_WRITE_G(v, g, idx)                        GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 12, 2, 7, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_SOP_READ(r, p)                                FIELD_MREAD_16((uint8_t *)p + 12, 2, 7, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_SOP_WRITE(v, p)                               FIELD_MWRITE_16((uint8_t *)p + 12, 2, 7, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_BN0_FIRST_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 12, 0, 18, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_BN0_FIRST_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_PROCESSING_RX_DESCRIPTOR_DTS) + 12, 0, 18, v)
#define RDD_PROCESSING_RX_DESCRIPTOR_BN0_FIRST_READ(r, p)                          FIELD_MREAD_32((uint8_t *)p + 12, 0, 18, r)
#define RDD_PROCESSING_RX_DESCRIPTOR_BN0_FIRST_WRITE(v, p)                         FIELD_MWRITE_32((uint8_t *)p + 12, 0, 18, v)
/* <<<RDD_PROCESSING_RX_DESCRIPTOR_DTS */


#define RDD_DIRECT_PROCESSING_PD_TABLE_SIZE     2
typedef struct
{
	RDD_PROCESSING_RX_DESCRIPTOR_DTS	entry[ RDD_DIRECT_PROCESSING_PD_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DIRECT_PROCESSING_PD_TABLE_DTS;

extern uint32_t RDD_DIRECT_PROCESSING_PD_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_DIRECT_PROCESSING_PD_TABLE_PTR(core_id)	( RDD_DIRECT_PROCESSING_PD_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_DIRECT_PROCESSING_PD_TABLE_ADDRESS_ARR[core_id] ))

/* <<<DIRECT_PROCESSING_PD_TABLE */


/* >>>DS_CPU_RX_METER_TABLE */

/* >>>RDD_CPU_RX_METER_ENTRY_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	current_budget  	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	budget_limit    	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	allocated_budget	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved        	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	budget_limit    	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	current_budget  	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved        	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	allocated_budget	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_METER_ENTRY_DTS;

#define RDD_CPU_RX_METER_ENTRY_CURRENT_BUDGET_READ_G(r, g, idx)            GROUP_MREAD_16(g, idx*sizeof(RDD_CPU_RX_METER_ENTRY_DTS), r)
#define RDD_CPU_RX_METER_ENTRY_CURRENT_BUDGET_WRITE_G(v, g, idx)           GROUP_MWRITE_16(g, idx*sizeof(RDD_CPU_RX_METER_ENTRY_DTS), v)
#define RDD_CPU_RX_METER_ENTRY_CURRENT_BUDGET_READ(r, p)                   MREAD_16((uint8_t *)p, r)
#define RDD_CPU_RX_METER_ENTRY_CURRENT_BUDGET_WRITE(v, p)                  MWRITE_16((uint8_t *)p, v)
#define RDD_CPU_RX_METER_ENTRY_BUDGET_LIMIT_READ_G(r, g, idx)              GROUP_MREAD_16(g, idx*sizeof(RDD_CPU_RX_METER_ENTRY_DTS) + 2, r)
#define RDD_CPU_RX_METER_ENTRY_BUDGET_LIMIT_WRITE_G(v, g, idx)             GROUP_MWRITE_16(g, idx*sizeof(RDD_CPU_RX_METER_ENTRY_DTS) + 2, v)
#define RDD_CPU_RX_METER_ENTRY_BUDGET_LIMIT_READ(r, p)                     MREAD_16((uint8_t *)p + 2, r)
#define RDD_CPU_RX_METER_ENTRY_BUDGET_LIMIT_WRITE(v, p)                    MWRITE_16((uint8_t *)p + 2, v)
#define RDD_CPU_RX_METER_ENTRY_ALLOCATED_BUDGET_READ_G(r, g, idx)          GROUP_MREAD_16(g, idx*sizeof(RDD_CPU_RX_METER_ENTRY_DTS) + 4, r)
#define RDD_CPU_RX_METER_ENTRY_ALLOCATED_BUDGET_WRITE_G(v, g, idx)         GROUP_MWRITE_16(g, idx*sizeof(RDD_CPU_RX_METER_ENTRY_DTS) + 4, v)
#define RDD_CPU_RX_METER_ENTRY_ALLOCATED_BUDGET_READ(r, p)                 MREAD_16((uint8_t *)p + 4, r)
#define RDD_CPU_RX_METER_ENTRY_ALLOCATED_BUDGET_WRITE(v, p)                MWRITE_16((uint8_t *)p + 4, v)
/* <<<RDD_CPU_RX_METER_ENTRY_DTS */


#define RDD_DS_CPU_RX_METER_TABLE_SIZE     16
typedef struct
{
	RDD_CPU_RX_METER_ENTRY_DTS	entry[ RDD_DS_CPU_RX_METER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DS_CPU_RX_METER_TABLE_DTS;

extern uint32_t RDD_DS_CPU_RX_METER_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_DS_CPU_RX_METER_TABLE_PTR(core_id)	( RDD_DS_CPU_RX_METER_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_DS_CPU_RX_METER_TABLE_ADDRESS_ARR[core_id] ))

/* <<<DS_CPU_RX_METER_TABLE */


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


/* >>>SCRATCH */

#define RDD_SCRATCH_SIZE     8
typedef struct
{
	RDD_BYTES_4_DTS	entry[ RDD_SCRATCH_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_SCRATCH_DTS;

extern uint32_t RDD_SCRATCH_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_SCRATCH_PTR(core_id)	( RDD_SCRATCH_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_SCRATCH_ADDRESS_ARR[core_id] ))

/* <<<SCRATCH */


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


/* >>>US_CPU_RX_METER_TABLE */

#define RDD_US_CPU_RX_METER_TABLE_SIZE     16
typedef struct
{
	RDD_CPU_RX_METER_ENTRY_DTS	entry[ RDD_US_CPU_RX_METER_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_US_CPU_RX_METER_TABLE_DTS;

extern uint32_t RDD_US_CPU_RX_METER_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_US_CPU_RX_METER_TABLE_PTR(core_id)	( RDD_US_CPU_RX_METER_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_US_CPU_RX_METER_TABLE_ADDRESS_ARR[core_id] ))

/* <<<US_CPU_RX_METER_TABLE */


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


/* >>>CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR */

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

typedef struct
{
	RDD_DDR_ADDRESS_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_DTS;

extern uint32_t RDD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_PTR(core_id)	( RDD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR_ADDRESS_ARR[core_id] ))

/* <<<CPU_RECYCLE_INTERRUPT_ID_DDR_ADDR */


/* >>>CPU_RX_SCRATCHPAD */

/* >>>RDD_BYTES_8_DTS */
#define RDD_BYTES_8_BITS_NUMBER	2

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	bits[RDD_BYTES_8_BITS_NUMBER];
#else
	uint32_t	bits[RDD_BYTES_8_BITS_NUMBER];
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BYTES_8_DTS;

#define RDD_BYTES_8_BITS_READ_G(r, g, idx, i)       GROUP_MREAD_I_32(g, idx*sizeof(RDD_BYTES_8_DTS), i, r)
#define RDD_BYTES_8_BITS_WRITE_G(v, g, idx, i)      GROUP_MWRITE_I_32(g, idx*sizeof(RDD_BYTES_8_DTS), i, v)
#define RDD_BYTES_8_BITS_READ(r, p, i)              MREAD_I_32((uint8_t *)p, i, r)
#define RDD_BYTES_8_BITS_WRITE(v, p, i)             MWRITE_I_32((uint8_t *)p, i, v)
/* <<<RDD_BYTES_8_DTS */


#define RDD_CPU_RX_SCRATCHPAD_SIZE     64
typedef struct
{
	RDD_BYTES_8_DTS	entry[ RDD_CPU_RX_SCRATCHPAD_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_SCRATCHPAD_DTS;

extern uint32_t RDD_CPU_RX_SCRATCHPAD_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_RX_SCRATCHPAD_PTR(core_id)	( RDD_CPU_RX_SCRATCHPAD_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_RX_SCRATCHPAD_ADDRESS_ARR[core_id] ))

/* <<<CPU_RX_SCRATCHPAD */


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


/* >>>CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD */

#define RDD_CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD_SIZE     4
typedef struct
{
	RDD_BYTES_4_DTS	entry[ RDD_CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD_DTS;

extern uint32_t RDD_CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD_PTR(core_id)	( RDD_CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD_ADDRESS_ARR[core_id] ))

/* <<<CPU_RX_PSRAM_GET_NEXT_SCRATCHPAD */


/* >>>CPU_RX_COPY_DISPATCHER_CREDIT_TABLE */

#define RDD_CPU_RX_COPY_DISPATCHER_CREDIT_TABLE_SIZE     3
typedef struct
{
	RDD_BYTES_4_DTS	entry[ RDD_CPU_RX_COPY_DISPATCHER_CREDIT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_COPY_DISPATCHER_CREDIT_TABLE_DTS;

extern uint32_t RDD_CPU_RX_COPY_DISPATCHER_CREDIT_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_RX_COPY_DISPATCHER_CREDIT_TABLE_PTR(core_id)	( RDD_CPU_RX_COPY_DISPATCHER_CREDIT_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_RX_COPY_DISPATCHER_CREDIT_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_RX_COPY_DISPATCHER_CREDIT_TABLE */


/* >>>CPU_RX_INTERRUPT_SCRATCH */
typedef struct
{
	RDD_BYTES_4_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_INTERRUPT_SCRATCH_DTS;

extern uint32_t RDD_CPU_RX_INTERRUPT_SCRATCH_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_RX_INTERRUPT_SCRATCH_PTR(core_id)	( RDD_CPU_RX_INTERRUPT_SCRATCH_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_RX_INTERRUPT_SCRATCH_ADDRESS_ARR[core_id] ))

/* <<<CPU_RX_INTERRUPT_SCRATCH */


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


/* >>>CPU_RX_INTERRUPT_ID_DDR_ADDR */
typedef struct
{
	RDD_DDR_ADDRESS_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_INTERRUPT_ID_DDR_ADDR_DTS;

extern uint32_t RDD_CPU_RX_INTERRUPT_ID_DDR_ADDR_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_RX_INTERRUPT_ID_DDR_ADDR_PTR(core_id)	( RDD_CPU_RX_INTERRUPT_ID_DDR_ADDR_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_RX_INTERRUPT_ID_DDR_ADDR_ADDRESS_ARR[core_id] ))

/* <<<CPU_RX_INTERRUPT_ID_DDR_ADDR */


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


/* >>>CPU_RX_COPY_UPDATE_FIFO_TABLE */

#define RDD_CPU_RX_COPY_UPDATE_FIFO_TABLE_SIZE     8
typedef struct
{
	RDD_UPDATE_FIFO_ENTRY_DTS	entry[ RDD_CPU_RX_COPY_UPDATE_FIFO_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_COPY_UPDATE_FIFO_TABLE_DTS;

extern uint32_t RDD_CPU_RX_COPY_UPDATE_FIFO_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_RX_COPY_UPDATE_FIFO_TABLE_PTR(core_id)	( RDD_CPU_RX_COPY_UPDATE_FIFO_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_RX_COPY_UPDATE_FIFO_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_RX_COPY_UPDATE_FIFO_TABLE */


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


/* >>>PD_FIFO_TABLE */

#define RDD_PD_FIFO_TABLE_SIZE     2
typedef struct
{
	RDD_PROCESSING_TX_DESCRIPTOR_DTS	entry[ RDD_PD_FIFO_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PD_FIFO_TABLE_DTS;

extern uint32_t RDD_PD_FIFO_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_PD_FIFO_TABLE_PTR(core_id)	( RDD_PD_FIFO_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_PD_FIFO_TABLE_ADDRESS_ARR[core_id] ))

/* <<<PD_FIFO_TABLE */


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


/* >>>CPU_FEED_RING_DESCRIPTOR_TABLE */
typedef struct
{
	RDD_CPU_RING_DESCRIPTOR_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_DTS;

extern uint32_t RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_PTR(core_id)	( RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_FEED_RING_DESCRIPTOR_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_FEED_RING_DESCRIPTOR_TABLE */


/* >>>CPU_RX_LOCAL_SCRATCH */

#define RDD_CPU_RX_LOCAL_SCRATCH_SIZE     2
typedef struct
{
	RDD_BYTES_8_DTS	entry[ RDD_CPU_RX_LOCAL_SCRATCH_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_LOCAL_SCRATCH_DTS;

extern uint32_t RDD_CPU_RX_LOCAL_SCRATCH_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_RX_LOCAL_SCRATCH_PTR(core_id)	( RDD_CPU_RX_LOCAL_SCRATCH_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_RX_LOCAL_SCRATCH_ADDRESS_ARR[core_id] ))

/* <<<CPU_RX_LOCAL_SCRATCH */


/* >>>RX_FLOW_TABLE */

/* >>>RDD_RX_FLOW_ENTRY_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint16_t	virtual_port	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	flow_dest   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	exception   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	reserved    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	cntr_id     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint16_t	cntr_id     	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	reserved    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	exception   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	flow_dest   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	virtual_port	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RX_FLOW_ENTRY_DTS;

#define RDD_RX_FLOW_ENTRY_VIRTUAL_PORT_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_RX_FLOW_ENTRY_DTS), 3, 5, r)
#define RDD_RX_FLOW_ENTRY_VIRTUAL_PORT_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_RX_FLOW_ENTRY_DTS), 3, 5, v)
#define RDD_RX_FLOW_ENTRY_VIRTUAL_PORT_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 3, 5, r)
#define RDD_RX_FLOW_ENTRY_VIRTUAL_PORT_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 3, 5, v)
#define RDD_RX_FLOW_ENTRY_FLOW_DEST_READ_G(r, g, idx)             GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_RX_FLOW_ENTRY_DTS), 2, 1, r)
#define RDD_RX_FLOW_ENTRY_FLOW_DEST_WRITE_G(v, g, idx)            GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_RX_FLOW_ENTRY_DTS), 2, 1, v)
#define RDD_RX_FLOW_ENTRY_FLOW_DEST_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p, 2, 1, r)
#define RDD_RX_FLOW_ENTRY_FLOW_DEST_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p, 2, 1, v)
#define RDD_RX_FLOW_ENTRY_EXCEPTION_READ_G(r, g, idx)             GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_RX_FLOW_ENTRY_DTS), 1, 1, r)
#define RDD_RX_FLOW_ENTRY_EXCEPTION_WRITE_G(v, g, idx)            GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_RX_FLOW_ENTRY_DTS), 1, 1, v)
#define RDD_RX_FLOW_ENTRY_EXCEPTION_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p, 1, 1, r)
#define RDD_RX_FLOW_ENTRY_EXCEPTION_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p, 1, 1, v)
#define RDD_RX_FLOW_ENTRY_CNTR_ID_READ_G(r, g, idx)               GROUP_MREAD_8(g, idx*sizeof(RDD_RX_FLOW_ENTRY_DTS) + 1, r)
#define RDD_RX_FLOW_ENTRY_CNTR_ID_WRITE_G(v, g, idx)              GROUP_MWRITE_8(g, idx*sizeof(RDD_RX_FLOW_ENTRY_DTS) + 1, v)
#define RDD_RX_FLOW_ENTRY_CNTR_ID_READ(r, p)                      MREAD_8((uint8_t *)p + 1, r)
#define RDD_RX_FLOW_ENTRY_CNTR_ID_WRITE(v, p)                     MWRITE_8((uint8_t *)p + 1, v)
/* <<<RDD_RX_FLOW_ENTRY_DTS */


#define RDD_RX_FLOW_TABLE_SIZE     320
typedef struct
{
	RDD_RX_FLOW_ENTRY_DTS	entry[ RDD_RX_FLOW_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_RX_FLOW_TABLE_DTS;

extern uint32_t RDD_RX_FLOW_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_RX_FLOW_TABLE_PTR(core_id)	( RDD_RX_FLOW_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_RX_FLOW_TABLE_ADDRESS_ARR[core_id] ))

/* <<<RX_FLOW_TABLE */


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


/* >>>CPU_RX_COPY_LOCAL_SCRATCH */
typedef struct
{
	RDD_BYTES_8_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RX_COPY_LOCAL_SCRATCH_DTS;

extern uint32_t RDD_CPU_RX_COPY_LOCAL_SCRATCH_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_RX_COPY_LOCAL_SCRATCH_PTR(core_id)	( RDD_CPU_RX_COPY_LOCAL_SCRATCH_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_RX_COPY_LOCAL_SCRATCH_ADDRESS_ARR[core_id] ))

/* <<<CPU_RX_COPY_LOCAL_SCRATCH */


/* >>>DIRECT_PROCESSING_EPON_CONTROL_SCRATCH */
typedef struct
{
	RDD_BYTE_1_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_DIRECT_PROCESSING_EPON_CONTROL_SCRATCH_DTS;

extern uint32_t RDD_DIRECT_PROCESSING_EPON_CONTROL_SCRATCH_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_DIRECT_PROCESSING_EPON_CONTROL_SCRATCH_PTR(core_id)	( RDD_DIRECT_PROCESSING_EPON_CONTROL_SCRATCH_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_DIRECT_PROCESSING_EPON_CONTROL_SCRATCH_ADDRESS_ARR[core_id] ))

/* <<<DIRECT_PROCESSING_EPON_CONTROL_SCRATCH */


/* >>>CPU_FEED_RING_INTERRUPT_COUNTER_MAX */
typedef struct
{
	RDD_BYTES_2_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_FEED_RING_INTERRUPT_COUNTER_MAX_DTS;

extern uint32_t RDD_CPU_FEED_RING_INTERRUPT_COUNTER_MAX_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_FEED_RING_INTERRUPT_COUNTER_MAX_PTR(core_id)	( RDD_CPU_FEED_RING_INTERRUPT_COUNTER_MAX_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_FEED_RING_INTERRUPT_COUNTER_MAX_ADDRESS_ARR[core_id] ))

/* <<<CPU_FEED_RING_INTERRUPT_COUNTER_MAX */


/* >>>CPU_FEED_RING_INTERRUPT_THRESHOLD */
typedef struct
{
	RDD_BYTES_2_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_FEED_RING_INTERRUPT_THRESHOLD_DTS;

extern uint32_t RDD_CPU_FEED_RING_INTERRUPT_THRESHOLD_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_FEED_RING_INTERRUPT_THRESHOLD_PTR(core_id)	( RDD_CPU_FEED_RING_INTERRUPT_THRESHOLD_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_FEED_RING_INTERRUPT_THRESHOLD_ADDRESS_ARR[core_id] ))

/* <<<CPU_FEED_RING_INTERRUPT_THRESHOLD */


/* >>>CPU_FEED_RING_INTERRUPT_COUNTER */
typedef struct
{
	RDD_BYTES_2_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_FEED_RING_INTERRUPT_COUNTER_DTS;

extern uint32_t RDD_CPU_FEED_RING_INTERRUPT_COUNTER_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_FEED_RING_INTERRUPT_COUNTER_PTR(core_id)	( RDD_CPU_FEED_RING_INTERRUPT_COUNTER_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_FEED_RING_INTERRUPT_COUNTER_ADDRESS_ARR[core_id] ))

/* <<<CPU_FEED_RING_INTERRUPT_COUNTER */


/* >>>MAC_TYPE */

/* >>>RDD_MAC_TYPE_ENTRY_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint8_t	type      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint8_t	type      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MAC_TYPE_ENTRY_DTS;

#define RDD_MAC_TYPE_ENTRY_TYPE_READ_G(r, g, idx)          GROUP_MREAD_8(g, idx*sizeof(RDD_MAC_TYPE_ENTRY_DTS), r)
#define RDD_MAC_TYPE_ENTRY_TYPE_WRITE_G(v, g, idx)         GROUP_MWRITE_8(g, idx*sizeof(RDD_MAC_TYPE_ENTRY_DTS), v)
#define RDD_MAC_TYPE_ENTRY_TYPE_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_MAC_TYPE_ENTRY_TYPE_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
/* <<<RDD_MAC_TYPE_ENTRY_DTS */

typedef struct
{
	RDD_MAC_TYPE_ENTRY_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_MAC_TYPE_DTS;

extern uint32_t RDD_MAC_TYPE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_MAC_TYPE_PTR(core_id)	( RDD_MAC_TYPE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_MAC_TYPE_ADDRESS_ARR[core_id] ))

/* <<<MAC_TYPE */


/* >>>CPU_RECYCLE_INTERRUPT_SCRATCH */

#define RDD_CPU_RECYCLE_INTERRUPT_SCRATCH_SIZE     2
typedef struct
{
	RDD_BYTES_4_DTS	entry[ RDD_CPU_RECYCLE_INTERRUPT_SCRATCH_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RECYCLE_INTERRUPT_SCRATCH_DTS;

extern uint32_t RDD_CPU_RECYCLE_INTERRUPT_SCRATCH_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_RECYCLE_INTERRUPT_SCRATCH_PTR(core_id)	( RDD_CPU_RECYCLE_INTERRUPT_SCRATCH_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_RECYCLE_INTERRUPT_SCRATCH_ADDRESS_ARR[core_id] ))

/* <<<CPU_RECYCLE_INTERRUPT_SCRATCH */


/* >>>CPU_RECYCLE_SHADOW_RD_IDX */
typedef struct
{
	RDD_BYTES_2_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RECYCLE_SHADOW_RD_IDX_DTS;

extern uint32_t RDD_CPU_RECYCLE_SHADOW_RD_IDX_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_RECYCLE_SHADOW_RD_IDX_PTR(core_id)	( RDD_CPU_RECYCLE_SHADOW_RD_IDX_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_RECYCLE_SHADOW_RD_IDX_ADDRESS_ARR[core_id] ))

/* <<<CPU_RECYCLE_SHADOW_RD_IDX */


/* >>>CPU_RECYCLE_SHADOW_WR_IDX */
typedef struct
{
	RDD_BYTES_2_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_RECYCLE_SHADOW_WR_IDX_DTS;

extern uint32_t RDD_CPU_RECYCLE_SHADOW_WR_IDX_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_RECYCLE_SHADOW_WR_IDX_PTR(core_id)	( RDD_CPU_RECYCLE_SHADOW_WR_IDX_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_RECYCLE_SHADOW_WR_IDX_ADDRESS_ARR[core_id] ))

/* <<<CPU_RECYCLE_SHADOW_WR_IDX */


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

/* CORE_2 */

/* >>>CPU_TX_SCRATCHPAD */

#define RDD_CPU_TX_SCRATCHPAD_SIZE     128
typedef struct
{
	RDD_BYTES_8_DTS	entry[ RDD_CPU_TX_SCRATCHPAD_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_SCRATCHPAD_DTS;

extern uint32_t RDD_CPU_TX_SCRATCHPAD_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_TX_SCRATCHPAD_PTR(core_id)	( RDD_CPU_TX_SCRATCHPAD_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_TX_SCRATCHPAD_ADDRESS_ARR[core_id] ))

/* <<<CPU_TX_SCRATCHPAD */


/* >>>TX_FLOW_TABLE */

/* >>>RDD_TX_FLOW_ENTRY_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint8_t	valid        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	qos_table_ptr	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint8_t	qos_table_ptr	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	valid        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_TX_FLOW_ENTRY_DTS;

#define RDD_TX_FLOW_ENTRY_VALID_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_TX_FLOW_ENTRY_DTS), 7, 1, r)
#define RDD_TX_FLOW_ENTRY_VALID_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_TX_FLOW_ENTRY_DTS), 7, 1, v)
#define RDD_TX_FLOW_ENTRY_VALID_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_TX_FLOW_ENTRY_VALID_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_TX_FLOW_ENTRY_QOS_TABLE_PTR_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_TX_FLOW_ENTRY_DTS), 0, 7, r)
#define RDD_TX_FLOW_ENTRY_QOS_TABLE_PTR_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_TX_FLOW_ENTRY_DTS), 0, 7, v)
#define RDD_TX_FLOW_ENTRY_QOS_TABLE_PTR_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 0, 7, r)
#define RDD_TX_FLOW_ENTRY_QOS_TABLE_PTR_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 0, 7, v)
/* <<<RDD_TX_FLOW_ENTRY_DTS */


#define RDD_TX_FLOW_TABLE_SIZE     320
typedef struct
{
	RDD_TX_FLOW_ENTRY_DTS	entry[ RDD_TX_FLOW_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_TX_FLOW_TABLE_DTS;

extern uint32_t RDD_TX_FLOW_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_TX_FLOW_TABLE_PTR(core_id)	( RDD_TX_FLOW_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_TX_FLOW_TABLE_ADDRESS_ARR[core_id] ))

/* <<<TX_FLOW_TABLE */


/* >>>QUEUE_THRESHOLD_VECTOR */

#define RDD_QUEUE_THRESHOLD_VECTOR_SIZE     9
typedef struct
{
	RDD_BYTES_4_DTS	entry[ RDD_QUEUE_THRESHOLD_VECTOR_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_QUEUE_THRESHOLD_VECTOR_DTS;

extern uint32_t RDD_QUEUE_THRESHOLD_VECTOR_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_QUEUE_THRESHOLD_VECTOR_PTR(core_id)	( RDD_QUEUE_THRESHOLD_VECTOR_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_QUEUE_THRESHOLD_VECTOR_ADDRESS_ARR[core_id] ))

/* <<<QUEUE_THRESHOLD_VECTOR */


/* >>>CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE */

/* >>>RDD_FPM_POOL_NUMBER_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint8_t	reserved   	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	pool_number	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint8_t	pool_number	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	reserved   	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FPM_POOL_NUMBER_DTS;

#define RDD_FPM_POOL_NUMBER_POOL_NUMBER_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_FPM_POOL_NUMBER_DTS), 0, 2, r)
#define RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_FPM_POOL_NUMBER_DTS), 0, 2, v)
#define RDD_FPM_POOL_NUMBER_POOL_NUMBER_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 0, 2, r)
#define RDD_FPM_POOL_NUMBER_POOL_NUMBER_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 0, 2, v)
/* <<<RDD_FPM_POOL_NUMBER_DTS */


#define RDD_CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_SIZE     16
typedef struct
{
	RDD_FPM_POOL_NUMBER_DTS	entry[ RDD_CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_DTS;

extern uint32_t RDD_CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_PTR(core_id)	( RDD_CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE */


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


/* >>>FPM_REPLY */

#define RDD_FPM_REPLY_SIZE     2
typedef struct
{
	RDD_BYTES_8_DTS	entry[ RDD_FPM_REPLY_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_FPM_REPLY_DTS;

extern uint32_t RDD_FPM_REPLY_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_FPM_REPLY_PTR(core_id)	( RDD_FPM_REPLY_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_FPM_REPLY_ADDRESS_ARR[core_id] ))

/* <<<FPM_REPLY */


/* >>>CPU_TX_EGRESS_UPDATE_FIFO_TABLE */

#define RDD_CPU_TX_EGRESS_UPDATE_FIFO_TABLE_SIZE     8
typedef struct
{
	RDD_UPDATE_FIFO_ENTRY_DTS	entry[ RDD_CPU_TX_EGRESS_UPDATE_FIFO_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_EGRESS_UPDATE_FIFO_TABLE_DTS;

extern uint32_t RDD_CPU_TX_EGRESS_UPDATE_FIFO_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_TX_EGRESS_UPDATE_FIFO_TABLE_PTR(core_id)	( RDD_CPU_TX_EGRESS_UPDATE_FIFO_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_TX_EGRESS_UPDATE_FIFO_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_TX_EGRESS_UPDATE_FIFO_TABLE */


/* >>>CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE */

#define RDD_CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_SIZE     3
typedef struct
{
	RDD_BYTES_4_DTS	entry[ RDD_CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_DTS;

extern uint32_t RDD_CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_PTR(core_id)	( RDD_CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE */


/* >>>CORE_ID_TABLE */
typedef struct
{
	RDD_BYTE_1_DTS	entry;
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CORE_ID_TABLE_DTS;

extern uint32_t RDD_CORE_ID_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CORE_ID_TABLE_PTR(core_id)	( RDD_CORE_ID_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CORE_ID_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CORE_ID_TABLE */


/* >>>CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE */

#define RDD_CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_SIZE     3
typedef struct
{
	RDD_BYTES_4_DTS	entry[ RDD_CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_DTS;

extern uint32_t RDD_CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_PTR(core_id)	( RDD_CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE */


/* >>>CPU_TX_INGRESS_UPDATE_FIFO_TABLE */

#define RDD_CPU_TX_INGRESS_UPDATE_FIFO_TABLE_SIZE     8
typedef struct
{
	RDD_UPDATE_FIFO_ENTRY_DTS	entry[ RDD_CPU_TX_INGRESS_UPDATE_FIFO_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_INGRESS_UPDATE_FIFO_TABLE_DTS;

extern uint32_t RDD_CPU_TX_INGRESS_UPDATE_FIFO_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_TX_INGRESS_UPDATE_FIFO_TABLE_PTR(core_id)	( RDD_CPU_TX_INGRESS_UPDATE_FIFO_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_TX_INGRESS_UPDATE_FIFO_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_TX_INGRESS_UPDATE_FIFO_TABLE */


/* >>>CPU_TX_EGRESS_PD_FIFO_TABLE */

#define RDD_CPU_TX_EGRESS_PD_FIFO_TABLE_SIZE     2
typedef struct
{
	RDD_PROCESSING_TX_DESCRIPTOR_DTS	entry[ RDD_CPU_TX_EGRESS_PD_FIFO_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_EGRESS_PD_FIFO_TABLE_DTS;

extern uint32_t RDD_CPU_TX_EGRESS_PD_FIFO_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_TX_EGRESS_PD_FIFO_TABLE_PTR(core_id)	( RDD_CPU_TX_EGRESS_PD_FIFO_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_TX_EGRESS_PD_FIFO_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_TX_EGRESS_PD_FIFO_TABLE */


/* >>>CPU_TX_INGRESS_PD_FIFO_TABLE */

#define RDD_CPU_TX_INGRESS_PD_FIFO_TABLE_SIZE     2
typedef struct
{
	RDD_PROCESSING_TX_DESCRIPTOR_DTS	entry[ RDD_CPU_TX_INGRESS_PD_FIFO_TABLE_SIZE ];
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_TX_INGRESS_PD_FIFO_TABLE_DTS;

extern uint32_t RDD_CPU_TX_INGRESS_PD_FIFO_TABLE_ADDRESS_ARR[GROUPED_EN_SEGMENTS_NUM];

#define RDD_CPU_TX_INGRESS_PD_FIFO_TABLE_PTR(core_id)	( RDD_CPU_TX_INGRESS_PD_FIFO_TABLE_DTS * )(DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + RDD_CPU_TX_INGRESS_PD_FIFO_TABLE_ADDRESS_ARR[core_id] ))

/* <<<CPU_TX_INGRESS_PD_FIFO_TABLE */


#ifdef XRDP_CFE

/* >>>RDD_DISP_REOR_VIQ */
typedef enum
{
	DISP_REOR_VIQ_FIRST             = 0,
	DISP_REOR_VIQ_BBH_RX0_NORMAL    = 0,
	DISP_REOR_VIQ_BBH_RX1_NORMAL    = 1,
	DISP_REOR_VIQ_BBH_RX2_NORMAL    = 2,
	DISP_REOR_VIQ_BBH_RX3_NORMAL    = 3,
	DISP_REOR_VIQ_BBH_RX4_NORMAL    = 4,
	DISP_REOR_VIQ_BBH_RX5_NORMAL    = 5,
	DISP_REOR_VIQ_BBH_RX6_NORMAL    = 6,
	DISP_REOR_VIQ_BBH_RX0_EXCL      = 7,
	DISP_REOR_VIQ_BBH_RX1_EXCL      = 8,
	DISP_REOR_VIQ_BBH_RX2_EXCL      = 9,
	DISP_REOR_VIQ_BBH_RX3_EXCL      = 10,
	DISP_REOR_VIQ_BBH_RX4_EXCL      = 11,
	DISP_REOR_VIQ_BBH_RX5_EXCL      = 12,
	DISP_REOR_VIQ_BBH_RX6_EXCL      = 13,
	DISP_REOR_VIQ_CPU_TX_EGRESS     = 14,
	DISP_REOR_VIQ_CPU_RX_COPY       = 15,
	DISP_REOR_VIQ_LAST              = 15
} rdd_disp_reor_viq;
/* <<<RDD_DISP_REOR_VIQ */


/* >>>RDD_QM_QUEUE */
typedef enum
{
	QM_QUEUE_FIRST                    = 0,
	QM_QUEUE_US_START                 = 0,
	QM_QUEUE_US_EPON_START            = 40,
	QM_QUEUE_US_END                   = 47,
	QM_QUEUE_DS_START                 = 48,
	QM_QUEUE_DS_END                   = 86,
	QM_QUEUE_DHD_CPU_TX_POST_0        = 87,
	QM_QUEUE_DHD_TX_POST_0            = 88,
	QM_QUEUE_DHD_MCAST                = 89,
	QM_QUEUE_CPU_RX                   = 90,
	QM_QUEUE_CPU_TX_EGRESS            = 91,
	QM_QUEUE_CPU_TX_INGRESS           = 92,
	QM_QUEUE_DROP                     = 93,
	QM_QUEUE_CPU_RX_COPY_EXCLUSIVE    = 94,
	QM_QUEUE_CPU_RX_COPY_NORMAL       = 95,
	QM_QUEUE_LAST                     = 95
} rdd_qm_queue;
/* <<<RDD_QM_QUEUE */


/* >>>RDD_IMAGE_0_DS_TM */
typedef enum
{
	IMAGE_0_DS_TM_FIRST                      = 0,
	IMAGE_0_DS_TM_BUDGET_ALLOCATION_THREAD_NUMBER = 0,
	IMAGE_0_DS_TM_UPDATE_FIFO_THREAD_NUMBER  = 1,
	IMAGE_0_DS_TM_FLUSH_THREAD_NUMBER        = 2,
	IMAGE_0_DS_TM_TX_TASK_THREAD_NUMBER      = 3,
	IMAGE_0_DS_TM_LAST                       = 3
} rdd_ds_tm;
/* <<<RDD_IMAGE_0_DS_TM */


/* >>>RDD_IMAGE_1_CPU_IF_1 */
typedef enum
{
	IMAGE_1_CPU_IF_1_FIRST                                  = 0,
	IMAGE_1_CPU_IF_1_WAN_DIRECT_THREAD_NUMBER               = 0,
	IMAGE_1_CPU_IF_1_CPU_RX_THREAD_NUMBER                   = 2,
	IMAGE_1_CPU_IF_1_CPU_RX_UPDATE_FIFO_THREAD_NUMBER       = 2,
	IMAGE_1_CPU_IF_1_INTERRUPT_COALESCING_THREAD_NUMBER     = 3,
	IMAGE_1_CPU_IF_1_CPU_RX_METER_BUDGET_ALLOCATOR_THREAD_NUMBER = 4,
	IMAGE_1_CPU_IF_1_CPU_RECYCLE_THREAD_NUMBER              = 12,
	IMAGE_1_CPU_IF_1_CPU_RX_COPY_THREAD_NUMBER              = 13,
	IMAGE_1_CPU_IF_1_LAST                                   = 13
} rdd_cpu_if_1;
/* <<<RDD_IMAGE_1_CPU_IF_1 */


/* >>>RDD_IMAGE_2_CPU_IF_2 */
typedef enum
{
	IMAGE_2_CPU_IF_2_FIRST                               = 0,
	IMAGE_2_CPU_IF_2_CPU_TX_UPDATE_FIFO_EGRESS_THREAD_NUMBER = 0,
	IMAGE_2_CPU_IF_2_CPU_TX_UPDATE_FIFO_INGRESS_THREAD_NUMBER = 1,
	IMAGE_2_CPU_IF_2_CPU_RECYCLE_THREAD_NUMBER           = 2,
	IMAGE_2_CPU_IF_2_CPU_TX_EGRESS_THREAD_NUMBER         = 3,
	IMAGE_2_CPU_IF_2_CPU_TX_INGRESS_THREAD_NUMBER        = 4,
	IMAGE_2_CPU_IF_2_REPORTING_THREAD_NUMBER             = 5,
	IMAGE_2_CPU_IF_2_INTERRUPT_COALESCING_THREAD_NUMBER  = 6,
	IMAGE_2_CPU_IF_2_TX_ABS_RECYCLE_THREAD_NUMBER        = 14,
	IMAGE_2_CPU_IF_2_LAST                                = 14
} rdd_cpu_if_2;
/* <<<RDD_IMAGE_2_CPU_IF_2 */


/* >>>RDD_GLOBAL_CFG_REG */
typedef enum
{
	GLOBAL_CFG_REG_FIRST        = 0,
	GLOBAL_CFG_REG_IS_6858A0    = 0,
	GLOBAL_CFG_REG_LAST         = 0
} rdd_global_cfg_reg;
/* <<<RDD_GLOBAL_CFG_REG */


/* >>>RDD_OAM_ETHERTYPE */
typedef enum
{
	OAM_ETHERTYPE_FIRST   = 34824,
	OAM_ETHERTYPE_8808    = 34824,
	OAM_ETHERTYPE_8809    = 34825,
	OAM_ETHERTYPE_LAST    = 34825
} rdd_oam_ethertype;
/* <<<RDD_OAM_ETHERTYPE */


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


/* >>>RDD_TM_UPDATE_VECTOR */
typedef enum
{
	TM_UPDATE_VECTOR_FIRST              = 0,
	TM_UPDATE_VECTOR_BS_DWRR_Q          = 0,
	TM_UPDATE_VECTOR_CS_DWRR_Q          = 1,
	TM_UPDATE_VECTOR_CS_DWRR_BS         = 2,
	TM_UPDATE_VECTOR_BS_Q_BRL           = 3,
	TM_UPDATE_VECTOR_CS_BS_Q_BRL        = 4,
	TM_UPDATE_VECTOR_CS_Q_CRL_SIR       = 5,
	TM_UPDATE_VECTOR_CS_Q_CRL_PIR       = 6,
	TM_UPDATE_VECTOR_BS_BRL_Q           = 7,
	TM_UPDATE_VECTOR_CS_BS_CRL_SIR_Q    = 8,
	TM_UPDATE_VECTOR_CS_BS_CRL_PIR_Q    = 9,
	TM_UPDATE_VECTOR_CS_BRL             = 10,
	TM_UPDATE_VECTOR_OVL_RL             = 11,
	TM_UPDATE_VECTOR_LAST               = 11
} rdd_tm_update_vector;
/* <<<RDD_TM_UPDATE_VECTOR */


/* >>>RDD_CNPL_GROUP */
typedef enum
{
	CNPL_GROUP_FIRST                   = 0,
	CNPL_GROUP_RX_FLOW                 = 0,
	CNPL_GROUP_RX_FLOW_DROP            = 1,
	CNPL_GROUP_TX_FLOW                 = 2,
	CNPL_GROUP_IPTV_TCAM_DEF           = 3,
	CNPL_GROUP_VARIOUS                 = 4,
	CNPL_GROUP_GENERAL                 = 5,
	CNPL_GROUP_US_TX_QUEUE             = 6,
	CNPL_GROUP_DS_TX_QUEUE             = 7,
	CNPL_GROUP_CPU_IF                  = 8,
	CNPL_GROUP_CPU_RX_METER_DROP       = 9,
	CNPL_GROUP_POLICER_GREEN_COLOR     = 10,
	CNPL_GROUP_POLICER_YELLOW_COLOR    = 11,
	CNPL_GROUP_POLICER_RED_COLOR       = 12,
	CNPL_GROUP_LAST                    = 12
} rdd_cnpl_group;
/* <<<RDD_CNPL_GROUP */


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


/* >>>RDD_IMAGE_0_DS_TM_TIMER_INDEX */
typedef enum
{
	IMAGE_0_DS_TM_TIMER_INDEX_FIRST       = 0,
	IMAGE_0_DS_TM_TIMER_INDEX_FLUSH       = 0,
	IMAGE_0_DS_TM_TIMER_INDEX_BUDGET_ALLOCATOR = 2,
	IMAGE_0_DS_TM_TIMER_INDEX_LAST        = 2
} rdd_ds_tm_timer_index;
/* <<<RDD_IMAGE_0_DS_TM_TIMER_INDEX */


/* >>>RDD_IMAGE_1_CPU_IF_1_TIMER_INDEX */
typedef enum
{
	IMAGE_1_CPU_IF_1_TIMER_INDEX_FIRST                    = 0,
	IMAGE_1_CPU_IF_1_TIMER_INDEX_INTERRUPT_COALESCING     = 0,
	IMAGE_1_CPU_IF_1_TIMER_INDEX_CPU_RX_METER_BUDGET_ALLOCATOR = 1,
	IMAGE_1_CPU_IF_1_TIMER_INDEX_LAST                     = 1
} rdd_cpu_if_1_timer_index;
/* <<<RDD_IMAGE_1_CPU_IF_1_TIMER_INDEX */


/* >>>RDD_IMAGE_2_CPU_IF_2_TIMER_INDEX */
typedef enum
{
	IMAGE_2_CPU_IF_2_TIMER_INDEX_FIRST           = 0,
	IMAGE_2_CPU_IF_2_TIMER_INDEX_REPORTING       = 0,
	IMAGE_2_CPU_IF_2_TIMER_INDEX_TX_RING_POLL    = 1,
	IMAGE_2_CPU_IF_2_TIMER_INDEX_INTERRUPT_COALESCING = 2,
	IMAGE_2_CPU_IF_2_TIMER_INDEX_LAST            = 2
} rdd_cpu_if_2_timer_index;
/* <<<RDD_IMAGE_2_CPU_IF_2_TIMER_INDEX */


/* >>>RDD_ACTION */
typedef enum
{
	ACTION_FIRST        = 0,
	ACTION_FORWARD      = 0,
	ACTION_TRAP         = 1,
	ACTION_DROP         = 2,
	ACTION_MULTICAST    = 3,
	ACTION_LAST         = 3
} rdd_action;
/* <<<RDD_ACTION */


/* >>>RDD_LAYER3_IPV6_HEADER */
typedef enum
{
	LAYER3_IPV6_HEADER_FIRST            = 8,
	LAYER3_IPV6_HEADER_SRC_IP_OFFSET    = 8,
	LAYER3_IPV6_HEADER_DST_IP_OFFSET    = 24,
	LAYER3_IPV6_HEADER_LAST             = 24
} rdd_layer3_ipv6_header;
/* <<<RDD_LAYER3_IPV6_HEADER */


/* >>>RDD_LAYER3_HEADER */
typedef enum
{
	LAYER3_HEADER_FIRST                  = 0,
	LAYER3_HEADER_TRAFIC_CLASS_OFFSET    = 0,
	LAYER3_HEADER_TOS_OFFSET             = 1,
	LAYER3_HEADER_FLAGS_OFFSET           = 6,
	LAYER3_HEADER_HOP_LIMIT_OFFSET       = 7,
	LAYER3_HEADER_TTL_OFFSET             = 8,
	LAYER3_HEADER_PROTOCOL_OFFSET        = 9,
	LAYER3_HEADER_IP_CHECKSUM_OFFSET     = 10,
	LAYER3_HEADER_SRC_IP_OFFSET          = 12,
	LAYER3_HEADER_DST_IP_OFFSET          = 16,
	LAYER3_HEADER_LAST                   = 16
} rdd_layer3_header;
/* <<<RDD_LAYER3_HEADER */


/* >>>RDD_LAYER4_HEADER */
typedef enum
{
	LAYER4_HEADER_FIRST                  = 0,
	LAYER4_HEADER_SRC_PORT_OFFSET        = 0,
	LAYER4_HEADER_ESP_SPI_OFFSET         = 0,
	LAYER4_HEADER_DST_PORT_OFFSET        = 2,
	LAYER4_HEADER_UDP_CHECKSUM_OFFSET    = 6,
	LAYER4_HEADER_GRE_CALL_ID_OFFSET     = 6,
	LAYER4_HEADER_TCP_FLAGS_OFFSET       = 13,
	LAYER4_HEADER_TCP_CHECKSUM_OFFSET    = 16,
	LAYER4_HEADER_LAST                   = 16
} rdd_layer4_header;
/* <<<RDD_LAYER4_HEADER */


/* >>>RDD_PARSER_L2_PROTOCOL */
typedef enum
{
	PARSER_L2_PROTOCOL_FIRST             = 1,
	PARSER_L2_PROTOCOL_PPPOE_D           = 1,
	PARSER_L2_PROTOCOL_PPPOE_S           = 2,
	PARSER_L2_PROTOCOL_USER_DEFINED_0    = 8,
	PARSER_L2_PROTOCOL_USER_DEFINED_1    = 9,
	PARSER_L2_PROTOCOL_USER_DEFINED_2    = 10,
	PARSER_L2_PROTOCOL_USER_DEFINED_3    = 11,
	PARSER_L2_PROTOCOL_ARP               = 12,
	PARSER_L2_PROTOCOL__1588             = 13,
	PARSER_L2_PROTOCOL__802_1X           = 14,
	PARSER_L2_PROTOCOL_MASK              = 15,
	PARSER_L2_PROTOCOL__802_1AG_CFM      = 15,
	PARSER_L2_PROTOCOL_LAST              = 15
} rdd_parser_l2_protocol;
/* <<<RDD_PARSER_L2_PROTOCOL */


/* >>>RDD_PARSER_L3_PROTOCOL */
typedef enum
{
	PARSER_L3_PROTOCOL_FIRST    = 0,
	PARSER_L3_PROTOCOL_OTHER    = 0,
	PARSER_L3_PROTOCOL_IPV4     = 1,
	PARSER_L3_PROTOCOL_IPV6     = 2,
	PARSER_L3_PROTOCOL_MASK     = 3,
	PARSER_L3_PROTOCOL_LAST     = 3
} rdd_parser_l3_protocol;
/* <<<RDD_PARSER_L3_PROTOCOL */


/* >>>RDD_PARSER_L4_PROTOCOL */
typedef enum
{
	PARSER_L4_PROTOCOL_FIRST             = 0,
	PARSER_L4_PROTOCOL_OTHER             = 0,
	PARSER_L4_PROTOCOL_TCP               = 1,
	PARSER_L4_PROTOCOL_UDP               = 2,
	PARSER_L4_PROTOCOL_IGMP              = 3,
	PARSER_L4_PROTOCOL_ICMP              = 4,
	PARSER_L4_PROTOCOL_ICMPV6            = 5,
	PARSER_L4_PROTOCOL_ESP               = 6,
	PARSER_L4_PROTOCOL_GRE               = 7,
	PARSER_L4_PROTOCOL_USER_DEFINED_0    = 8,
	PARSER_L4_PROTOCOL_USER_DEFINED_1    = 9,
	PARSER_L4_PROTOCOL_USER_DEFINED_2    = 10,
	PARSER_L4_PROTOCOL_USER_DEFINED_3    = 11,
	PARSER_L4_PROTOCOL_RESERVED          = 12,
	PARSER_L4_PROTOCOL_IPV6              = 13,
	PARSER_L4_PROTOCOL_AH                = 14,
	PARSER_L4_PROTOCOL_NOT_PARSED        = 15,
	PARSER_L4_PROTOCOL_MASK              = 15,
	PARSER_L4_PROTOCOL_LAST              = 15
} rdd_parser_l4_protocol;
/* <<<RDD_PARSER_L4_PROTOCOL */


/* >>>RDD_ACTION_ECN */
typedef enum
{
	ACTION_ECN_FIRST               = 0,
	ACTION_ECN_REMARKING_OFFSET    = 0,
	ACTION_ECN_REMARKING_WIDTH     = 2,
	ACTION_ECN_LAST                = 2
} rdd_action_ecn;
/* <<<RDD_ACTION_ECN */


/* >>>RDD_ACTION_DSCP */
typedef enum
{
	ACTION_DSCP_FIRST                             = 2,
	ACTION_DSCP_REMARKING_OFFSET                  = 2,
	ACTION_DSCP_REMARKING_TRAFFIC_CLASS_OFFSET    = 4,
	ACTION_DSCP_REMARKING_WIDTH                   = 6,
	ACTION_DSCP_REMARKING_TRAFFIC_CLASS_WIDTH     = 8,
	ACTION_DSCP_LAST                              = 8
} rdd_action_dscp;
/* <<<RDD_ACTION_DSCP */


/* >>>RDD_ACTION_OUTER */
typedef enum
{
	ACTION_OUTER_FIRST                         = 14,
	ACTION_OUTER_PBITS_REMARKING_VID_OFFSET    = 14,
	ACTION_OUTER_LAST                          = 14
} rdd_action_outer;
/* <<<RDD_ACTION_OUTER */


/* >>>RDD_ACTION_INNER */
typedef enum
{
	ACTION_INNER_FIRST                         = 18,
	ACTION_INNER_PBITS_REMARKING_VID_OFFSET    = 18,
	ACTION_INNER_LAST                          = 18
} rdd_action_inner;
/* <<<RDD_ACTION_INNER */


/* >>>RDD_ACTION_PBITS */
typedef enum
{
	ACTION_PBITS_FIRST                      = 2,
	ACTION_PBITS_REMARKING_DSCP_OFFSET      = 2,
	ACTION_PBITS_REMARKING_PACKET_WIDTH     = 3,
	ACTION_PBITS_REMARKING_DSCP_WIDTH       = 6,
	ACTION_PBITS_REMARKING_PACKET_OFFSET    = 13,
	ACTION_PBITS_LAST                       = 13
} rdd_action_pbits;
/* <<<RDD_ACTION_PBITS */


/* >>>RDD_ACTION_DS_LITE */
typedef enum
{
	ACTION_DS_LITE_FIRST   = 40,
	ACTION_DS_LITE_SIZE    = 40,
	ACTION_DS_LITE_LAST    = 40
} rdd_action_ds_lite;
/* <<<RDD_ACTION_DS_LITE */


/* >>>RDD_DS_ACTION_ID */
typedef enum
{
	DS_ACTION_ID_FIRST        = 0,
	DS_ACTION_ID_TRAP         = 0,
	DS_ACTION_ID_TTL          = 2,
	DS_ACTION_ID_DSCP         = 4,
	DS_ACTION_ID_NAT          = 5,
	DS_ACTION_ID_GRE          = 6,
	DS_ACTION_ID_OPBITS       = 7,
	DS_ACTION_ID_IPBITS       = 8,
	DS_ACTION_ID_DS_LITE      = 9,
	DS_ACTION_ID_PPPOE        = 10,
	DS_ACTION_ID_TOTAL_NUM    = 17,
	DS_ACTION_ID_LAST         = 17
} rdd_ds_action_id;
/* <<<RDD_DS_ACTION_ID */


/* >>>RDD_US_ACTION_ID */
typedef enum
{
	US_ACTION_ID_FIRST        = 0,
	US_ACTION_ID_TRAP         = 0,
	US_ACTION_ID_TTL          = 2,
	US_ACTION_ID_DSCP         = 4,
	US_ACTION_ID_NAT          = 5,
	US_ACTION_ID_GRE          = 6,
	US_ACTION_ID_OPBITS       = 7,
	US_ACTION_ID_IPBITS       = 8,
	US_ACTION_ID_DS_LITE      = 9,
	US_ACTION_ID_PPPOE        = 10,
	US_ACTION_ID_TOTAL_NUM    = 17,
	US_ACTION_ID_LAST         = 17
} rdd_us_action_id;
/* <<<RDD_US_ACTION_ID */


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


/* >>>RDD_DSCP_TO */
typedef enum
{
	DSCP_TO_FIRST                 = 6,
	DSCP_TO_PBITS_SHIFT_OFFSET    = 6,
	DSCP_TO_LAST                  = 6
} rdd_dscp_to;
/* <<<RDD_DSCP_TO */


/* >>>RDD_RESOLUTION_CONTEXT */
typedef enum
{
	RESOLUTION_CONTEXT_FIRST                          = 0,
	RESOLUTION_CONTEXT_VIRTUAL_SRC_PORT_OFFSET        = 0,
	RESOLUTION_CONTEXT_GPE_DMA_OFFSET_WIDTH           = 1,
	RESOLUTION_CONTEXT_GPE_HEADER_LENGTH_WIDTH        = 1,
	RESOLUTION_CONTEXT_GPE_HEADER_IH_PTR_WIDTH        = 2,
	RESOLUTION_CONTEXT_GPE_COMMAND_LIST_PTR_WIDTH     = 2,
	RESOLUTION_CONTEXT_GPE_PACKET_DRR_PTR_WIDTH       = 4,
	RESOLUTION_CONTEXT_GPE_HEADER_LENGTH_OFFSET       = 6,
	RESOLUTION_CONTEXT_GPE_DMA_OFFSET_OFFSET          = 7,
	RESOLUTION_CONTEXT_GPE_COMMAND_LIST_PTR_OFFSET    = 8,
	RESOLUTION_CONTEXT_GPE_HEADER_IH_PTR_OFFSET       = 10,
	RESOLUTION_CONTEXT_GPE_PACKET_DRR_PTR_OFFSET      = 12,
	RESOLUTION_CONTEXT_LAST                           = 12
} rdd_resolution_context;
/* <<<RDD_RESOLUTION_CONTEXT */


/* >>>RDD_CPU_RX_REASON */
typedef enum
{
	CPU_RX_REASON_FIRST                 = 0,
	CPU_RX_REASON_OAM                   = 0,
	CPU_RX_REASON_PLOAM                 = 0,
	CPU_RX_REASON_OMCI                  = 1,
	CPU_RX_REASON_MPCP                  = 1,
	CPU_RX_REASON_FLOW                  = 2,
	CPU_RX_REASON_MULTICAST             = 3,
	CPU_RX_REASON_BROADCAST             = 4,
	CPU_RX_REASON_IGMP                  = 5,
	CPU_RX_REASON_ICMPV6                = 6,
	CPU_RX_REASON_MAC_TRAP_0            = 7,
	CPU_RX_REASON_MAC_TRAP_1            = 8,
	CPU_RX_REASON_MAC_TRAP_2            = 9,
	CPU_RX_REASON_MAC_TRAP_3            = 10,
	CPU_RX_REASON_DHCP                  = 11,
	CPU_RX_REASON_NON_TCP_UDP           = 12,
	CPU_RX_REASON_LOCAL_IP              = 13,
	CPU_RX_REASON_IP_HEADER_ERROR       = 14,
	CPU_RX_REASON_SA_MOVE               = 15,
	CPU_RX_REASON_UNKNOWN_SA            = 16,
	CPU_RX_REASON_UNKNOWN_DA            = 17,
	CPU_RX_REASON_IP_FRAGMENT           = 18,
	CPU_RX_REASON_MAC_SPOOFING          = 19,
	CPU_RX_REASON_DIRECT_QUEUE_0        = 20,
	CPU_RX_REASON_DIRECT_QUEUE_1        = 21,
	CPU_RX_REASON_DIRECT_QUEUE_2        = 22,
	CPU_RX_REASON_DIRECT_QUEUE_3        = 23,
	CPU_RX_REASON_DIRECT_QUEUE_4        = 24,
	CPU_RX_REASON_DIRECT_QUEUE_5        = 25,
	CPU_RX_REASON_DIRECT_QUEUE_6        = 26,
	CPU_RX_REASON_DIRECT_QUEUE_7        = 27,
	CPU_RX_REASON_ETYPE_UDEF_0          = 28,
	CPU_RX_REASON_ETYPE_UDEF_1          = 29,
	CPU_RX_REASON_ETYPE_UDEF_2          = 30,
	CPU_RX_REASON_ETYPE_UDEF_3          = 31,
	CPU_RX_REASON_ETYPE_PPPOE_D         = 32,
	CPU_RX_REASON_ETYPE_PPPOE_S         = 33,
	CPU_RX_REASON_ETYPE_ARP             = 34,
	CPU_RX_REASON__1588                 = 35,
	CPU_RX_REASON_ETYPE_802_1X          = 36,
	CPU_RX_REASON_ETYPE_801_1AG_CFM     = 37,
	CPU_RX_REASON_PCI_IP_FLOW_MISS_1    = 38,
	CPU_RX_REASON_PCI_IP_FLOW_MISS_2    = 39,
	CPU_RX_REASON_PCI_IP_FLOW_MISS_3    = 40,
	CPU_RX_REASON_IP_FLOW_MISS          = 41,
	CPU_RX_REASON_TCP_FLAGS             = 42,
	CPU_RX_REASON_TTL_EXPIRED           = 43,
	CPU_RX_REASON_MTU_EXCEEDED          = 44,
	CPU_RX_REASON_L4_ICMP               = 45,
	CPU_RX_REASON_L4_ESP                = 46,
	CPU_RX_REASON_L4_GRE                = 47,
	CPU_RX_REASON_L4_AH                 = 48,
	CPU_RX_REASON_PARSER_ERROR          = 49,
	CPU_RX_REASON_L4_IPV6               = 50,
	CPU_RX_REASON_L4_UDEF_0             = 51,
	CPU_RX_REASON_L4_UDEF_1             = 52,
	CPU_RX_REASON_L4_UDEF_2             = 53,
	CPU_RX_REASON_L4_UDEF_3             = 54,
	CPU_RX_REASON_CPU_REDIRECT          = 55,
	CPU_RX_REASON_UDEF_0                = 56,
	CPU_RX_REASON_UDEF_1                = 57,
	CPU_RX_REASON_UDEF_2                = 58,
	CPU_RX_REASON_UDEF_3                = 59,
	CPU_RX_REASON_UDEF_4                = 60,
	CPU_RX_REASON_UDEF_5                = 61,
	CPU_RX_REASON_UDEF_6                = 62,
	CPU_RX_REASON_UDEF_7                = 63,
	CPU_RX_REASON_LAST                  = 63
} rdd_cpu_rx_reason;
/* <<<RDD_CPU_RX_REASON */


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


/* >>>RDD_CPU_REDIRECT_TYPE */
typedef enum
{
	CPU_REDIRECT_TYPE_FIRST   = 0,
	CPU_REDIRECT_TYPE_NONE    = 0,
	CPU_REDIRECT_TYPE_ALL     = 1,
	CPU_REDIRECT_TYPE_FLOW    = 2,
	CPU_REDIRECT_TYPE_LAST    = 2
} rdd_cpu_redirect_type;
/* <<<RDD_CPU_REDIRECT_TYPE */


/* >>>RDD_BBMSG_TYPE */
typedef enum
{
	BBMSG_TYPE_FIRST                                              = 0,
	BBMSG_TYPE_RUNNER_BBH_RX_FLOW_CONTROL                         = 0,
	BBMSG_TYPE_QM_PD_FIFO_CREDIT_FREE                             = 0,
	BBMSG_TYPE_BBH_TX_PACKET_DESCRIPTOR                           = 1,
	BBMSG_TYPE_RUNNER_SBPM_BUFFER_ALLOC                           = 1,
	BBMSG_TYPE_QM_UPDATE_FIFO_CREDIT_FREE                         = 1,
	BBMSG_TYPE_FPM_BUFFER_ALLOC                                   = 1,
	BBMSG_TYPE_FPM_BUFFER_MCAST_INCREMENT                         = 2,
	BBMSG_TYPE_RUNNER_SBPM_MCAST_INC_REQUEST                      = 2,
	BBMSG_TYPE_BBH_TX_NACK                                        = 2,
	BBMSG_TYPE_BBH_TX_ACK                                         = 3,
	BBMSG_TYPE_FPM_BUFFER_FREE                                    = 3,
	BBMSG_TYPE_BBH_SBPM_BUFFER_FREE                               = 3,
	BBMSG_TYPE_BBH_GHOST_DBR                                      = 4,
	BBMSG_TYPE_RUNNER_SBPM_CONNECT                                = 4,
	BBMSG_TYPE_RUNNER_SBPM_GET_NEXT                               = 5,
	BBMSG_TYPE_BBH_TX_REPORT_ACK                                  = 5,
	BBMSG_TYPE_BBH_SBPM_BUFFER_FREE_WITHOUT_CONTEXT               = 6,
	BBMSG_TYPE_RUNNER_SBPM_BUFFER_FREE_WITHOUT_CONTEXT            = 6,
	BBMSG_TYPE_BBH_TX_REPORT_NACK                                 = 7,
	BBMSG_TYPE_RUNNER_SBPM_INGRESS_TO_EGRESS                      = 7,
	BBMSG_TYPE_RUNNER_REORDER_PD_WRITE                            = 8,
	BBMSG_TYPE_RUNNER_DISPATCHER_PD_ACK                           = 9,
	BBMSG_TYPE_RUNNER_DISPATCHER_PD_WRITE                         = 10,
	BBMSG_TYPE_RUNNER_REORDER_TOKEN_REQUEST                       = 11,
	BBMSG_TYPE_RUNNER_REORDER_BUFFER_CONNECT                      = 12,
	BBMSG_TYPE_RUNNER_DISPATCHER_WAKEUP_PENDING                   = 13,
	BBMSG_TYPE_RUNNER_REORDER_CONNECT_REQUEST_BUFFER_CONNECTED    = 14,
	BBMSG_TYPE_BBH_RX_DISPATCHER_PD_WRITE                         = 24,
	BBMSG_TYPE_LAST                                               = 24
} rdd_bbmsg_type;
/* <<<RDD_BBMSG_TYPE */


/* >>>RDD_ERR_RX_PD */
typedef enum
{
	ERR_RX_PD_FIRST                 = 1,
	ERR_RX_PD_NO_SBPM               = 1,
	ERR_RX_PD_PACKET_TOO_SHORT      = 2,
	ERR_RX_PD_PACKET_TOO_LONG       = 3,
	ERR_RX_PD_CRC                   = 4,
	ERR_RX_PD_ENCRYPT               = 5,
	ERR_RX_PD_NO_SDMA_CHUNK         = 6,
	ERR_RX_PD_SOP_AFTER_SOP         = 7,
	ERR_RX_PD_THIRD_FLOW_ARRIVAL    = 8,
	ERR_RX_PD_LAST                  = 8
} rdd_err_rx_pd;
/* <<<RDD_ERR_RX_PD */


/* >>>RDD_SBPM_OPCODE */
typedef enum
{
	SBPM_OPCODE_FIRST                   = 0,
	SBPM_OPCODE_MULTI_GET_NEXT          = 0,
	SBPM_OPCODE_BN_ALLOC                = 1,
	SBPM_OPCODE_MCST_INC                = 2,
	SBPM_OPCODE_BN_FREE_WITH_CONTEXT    = 3,
	SBPM_OPCODE_BN_CONNECT              = 4,
	SBPM_OPCODE_GET_NEXT                = 5,
	SBPM_OPCODE_BN_FREE_WO_CONTEXT      = 6,
	SBPM_OPCODE_INGRESS_TO_EGRESS       = 7,
	SBPM_OPCODE_LAST                    = 7
} rdd_sbpm_opcode;
/* <<<RDD_SBPM_OPCODE */


/* >>>RDD_RDD_LAN */
typedef enum
{
	RDD_LAN_FIRST           = 1,
	RDD_LAN_BRIDGE_PORT0    = 1,
	RDD_LAN_BRIDGE_PORT1    = 2,
	RDD_LAN_BRIDGE_PORT2    = 3,
	RDD_LAN_BRIDGE_PORT3    = 4,
	RDD_LAN_BRIDGE_PORT4    = 5,
	RDD_LAN_BRIDGE_PORT5    = 6,
	RDD_LAN_LAST            = 6
} rdd_rdd_lan;
/* <<<RDD_RDD_LAN */


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


/* >>>RDD_SBPM */
typedef enum
{
	SBPM_FIRST                    = 16383,
	SBPM_INVALID_BUFFER_NUMBER    = 16383,
	SBPM_LAST                     = 16383
} rdd_sbpm;
/* <<<RDD_SBPM */


/* >>>RDD_FPM_POOL_ID */
typedef enum
{
	FPM_POOL_ID_FIRST            = 0,
	FPM_POOL_ID_EIGHT_BUFFERS    = 0,
	FPM_POOL_ID_FOUR_BUFFERS     = 1,
	FPM_POOL_ID_TWO_BUFFERS      = 2,
	FPM_POOL_ID_ONE_BUFFER       = 3,
	FPM_POOL_ID_LAST             = 3
} rdd_fpm_pool_id;
/* <<<RDD_FPM_POOL_ID */


/* >>>RDD_RNR */
typedef enum
{
	RNR_FIRST       = 0,
	RNR_CORE0_ID    = 0,
	RNR_CORE1_ID    = 1,
	RNR_CORE2_ID    = 2,
	RNR_LAST        = 2
} rdd_rnr;
/* <<<RDD_RNR */


/* >>>RDD_BB_ID */
typedef enum
{
	BB_ID_FIRST                 = 0,
	BB_ID_RNR0                  = 0,
	BB_ID_RNR1                  = 1,
	BB_ID_RNR2                  = 2,
	BB_ID_RNR3                  = 3,
	BB_ID_RNR4                  = 4,
	BB_ID_RNR5                  = 5,
	BB_ID_RNR6                  = 6,
	BB_ID_RNR7                  = 7,
	BB_ID_RNR8                  = 8,
	BB_ID_RNR9                  = 9,
	BB_ID_RNR10                 = 10,
	BB_ID_RNR11                 = 11,
	BB_ID_RNR12                 = 12,
	BB_ID_RNR13                 = 13,
	BB_ID_RNR14                 = 14,
	BB_ID_RNR15                 = 15,
	BB_ID_BBHLB                 = 16,
	BB_ID_CNPL                  = 17,
	BB_ID_DISPATCHER_REORDER    = 18,
	BB_ID_DMA0                  = 19,
	BB_ID_DMA1                  = 20,
	BB_ID_SDMA0                 = 21,
	BB_ID_SDMA1                 = 22,
	BB_ID_FPM                   = 23,
	BB_ID_HASH                  = 24,
	BB_ID_NATC                  = 25,
	BB_ID_QM_CP_SDMA            = 26,
	BB_ID_QM_RNR_GRID           = 27,
	BB_ID_QM_BBHTX              = 28,
	BB_ID_QM_TOP                = 29,
	BB_ID_QM_CP_MACHINE         = 30,
	BB_ID_RX_BBH_0              = 31,
	BB_ID_TX_LAN                = 32,
	BB_ID_RX_BBH_1              = 33,
	BB_ID_RX_BBH_2              = 35,
	BB_ID_RX_BBH_3              = 37,
	BB_ID_RX_BBH_4              = 39,
	BB_ID_RX_BBH_5              = 41,
	BB_ID_RX_PON                = 45,
	BB_ID_TX_PON_ETH_PD         = 46,
	BB_ID_TX_PON_ETH_STAT       = 47,
	BB_ID_SBPM                  = 48,
	BB_ID_TCAM_0                = 49,
	BB_ID_LAST                  = 49
} rdd_bb_id;
/* <<<RDD_BB_ID */

#endif


/* Hardware defines */

/* >>>RDD_PACKET_BUFFER_SCRATCH_ENTRY */
#define PACKET_BUFFER_SCRATCH_ENTRY_NATC_CFG_OFFSET                              0

/* >>>RDD_PACKET_BUFFER_SCRATCH_ENTRY_DTS */
#define RDD_PACKET_BUFFER_SCRATCH_ENTRY_NATC_CFG_NUMBER	8
#define RDD_PACKET_BUFFER_SCRATCH_ENTRY_RESERVED_NUMBER	128

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint8_t	natc_cfg[RDD_PACKET_BUFFER_SCRATCH_ENTRY_NATC_CFG_NUMBER];
	uint8_t	reserved[RDD_PACKET_BUFFER_SCRATCH_ENTRY_RESERVED_NUMBER];
#else
	uint8_t	natc_cfg[RDD_PACKET_BUFFER_SCRATCH_ENTRY_NATC_CFG_NUMBER];
	uint8_t	reserved[RDD_PACKET_BUFFER_SCRATCH_ENTRY_RESERVED_NUMBER];
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PACKET_BUFFER_SCRATCH_ENTRY_DTS;

#define RDD_PACKET_BUFFER_SCRATCH_ENTRY_NATC_CFG_READ_G(r, g, idx, i)       GROUP_MREAD_I_8(g, idx*sizeof(RDD_PACKET_BUFFER_SCRATCH_ENTRY_DTS), i, r)
#define RDD_PACKET_BUFFER_SCRATCH_ENTRY_NATC_CFG_WRITE_G(v, g, idx, i)      GROUP_MWRITE_I_8(g, idx*sizeof(RDD_PACKET_BUFFER_SCRATCH_ENTRY_DTS), i, v)
#define RDD_PACKET_BUFFER_SCRATCH_ENTRY_NATC_CFG_READ(r, p, i)              MREAD_I_8((uint8_t *)p, i, r)
#define RDD_PACKET_BUFFER_SCRATCH_ENTRY_NATC_CFG_WRITE(v, p, i)             MWRITE_I_8((uint8_t *)p, i, v)
/* <<<RDD_PACKET_BUFFER_SCRATCH_ENTRY_DTS */

/* <<<RDD_PACKET_BUFFER_SCRATCH_ENTRY */


/* >>>RDD_PACKET_BUFFER_RESOLUTION_ENTRY */
#define PACKET_BUFFER_RESOLUTION_ENTRY_RX_MIRRORING_FLAG_F_OFFSET                   31
#define PACKET_BUFFER_RESOLUTION_ENTRY_RX_MIRRORING_FLAG_F_WIDTH                    1
#define PACKET_BUFFER_RESOLUTION_ENTRY_RX_MIRRORING_FLAG_OFFSET                     0
#define PACKET_BUFFER_RESOLUTION_ENTRY_RX_MIRRORING_FLAG_F_OFFSET_MOD8              7
#define PACKET_BUFFER_RESOLUTION_ENTRY_RX_MIRRORING_FLAG_F_OFFSET_MOD16             15
#define PACKET_BUFFER_RESOLUTION_ENTRY_CPU_REDIRECT_FLAG_F_OFFSET                   30
#define PACKET_BUFFER_RESOLUTION_ENTRY_CPU_REDIRECT_FLAG_F_WIDTH                    1
#define PACKET_BUFFER_RESOLUTION_ENTRY_CPU_REDIRECT_FLAG_OFFSET                     0
#define PACKET_BUFFER_RESOLUTION_ENTRY_CPU_REDIRECT_FLAG_F_OFFSET_MOD8              6
#define PACKET_BUFFER_RESOLUTION_ENTRY_CPU_REDIRECT_FLAG_F_OFFSET_MOD16             14
#define PACKET_BUFFER_RESOLUTION_ENTRY_IS_LAN_F_OFFSET                              24
#define PACKET_BUFFER_RESOLUTION_ENTRY_IS_LAN_F_WIDTH                               1
#define PACKET_BUFFER_RESOLUTION_ENTRY_IS_LAN_OFFSET                                0
#define PACKET_BUFFER_RESOLUTION_ENTRY_IS_LAN_F_OFFSET_MOD8                         0
#define PACKET_BUFFER_RESOLUTION_ENTRY_IS_LAN_F_OFFSET_MOD16                        8
#define PACKET_BUFFER_RESOLUTION_ENTRY_RX_FLOW_F_OFFSET                             16
#define PACKET_BUFFER_RESOLUTION_ENTRY_RX_FLOW_F_WIDTH                              8
#define PACKET_BUFFER_RESOLUTION_ENTRY_RX_FLOW_OFFSET                               1
#define PACKET_BUFFER_RESOLUTION_ENTRY_RX_FLOW_F_OFFSET_MOD16                       0
#define PACKET_BUFFER_RESOLUTION_ENTRY_G9991_BUFFER_F_OFFSET                        16
#define PACKET_BUFFER_RESOLUTION_ENTRY_G9991_BUFFER_F_WIDTH                         16
#define PACKET_BUFFER_RESOLUTION_ENTRY_G9991_BUFFER_OFFSET                          4
#define PACKET_BUFFER_RESOLUTION_ENTRY_TX_FLOW_F_OFFSET                             0
#define PACKET_BUFFER_RESOLUTION_ENTRY_TX_FLOW_F_WIDTH                              16
#define PACKET_BUFFER_RESOLUTION_ENTRY_TX_FLOW_OFFSET                               6
#define PACKET_BUFFER_RESOLUTION_ENTRY_PER_FLOW_RESOLUTION_OFFSET                   8
#define PACKET_BUFFER_RESOLUTION_ENTRY_PER_FLOW_RESOLUTION_F_OFFSET_MOD16           8

/* >>>RDD_PACKET_BUFFER_RESOLUTION_ENTRY_DTS */
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_PER_FLOW_RESOLUTION_NUMBER	8

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	rx_mirroring_flag                                                                    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cpu_redirect_flag                                                                    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_0                                                                           	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	is_lan                                                                               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rx_flow                                                                              	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_1                                                                           	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_2                                                                           	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	g9991_buffer                                                                         	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tx_flow                                                                              	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	per_flow_resolution[RDD_PACKET_BUFFER_RESOLUTION_ENTRY_PER_FLOW_RESOLUTION_NUMBER];
#else
	uint32_t	reserved_2                                                                           	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_1                                                                           	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rx_flow                                                                              	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	is_lan                                                                               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_0                                                                           	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cpu_redirect_flag                                                                    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rx_mirroring_flag                                                                    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tx_flow                                                                              	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	g9991_buffer                                                                         	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	per_flow_resolution[RDD_PACKET_BUFFER_RESOLUTION_ENTRY_PER_FLOW_RESOLUTION_NUMBER];
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PACKET_BUFFER_RESOLUTION_ENTRY_DTS;

#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_RX_MIRRORING_FLAG_READ_G(r, g, idx)            GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_ENTRY_DTS), 7, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_RX_MIRRORING_FLAG_WRITE_G(v, g, idx)           GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_ENTRY_DTS), 7, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_RX_MIRRORING_FLAG_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_RX_MIRRORING_FLAG_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_CPU_REDIRECT_FLAG_READ_G(r, g, idx)            GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_ENTRY_DTS), 6, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_CPU_REDIRECT_FLAG_WRITE_G(v, g, idx)           GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_ENTRY_DTS), 6, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_CPU_REDIRECT_FLAG_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p, 6, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_CPU_REDIRECT_FLAG_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p, 6, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_IS_LAN_READ_G(r, g, idx)                       GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_ENTRY_DTS), 0, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_IS_LAN_WRITE_G(v, g, idx)                      GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_ENTRY_DTS), 0, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_IS_LAN_READ(r, p)                              FIELD_MREAD_8((uint8_t *)p, 0, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_IS_LAN_WRITE(v, p)                             FIELD_MWRITE_8((uint8_t *)p, 0, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_RX_FLOW_READ_G(r, g, idx)                      GROUP_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_ENTRY_DTS) + 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_RX_FLOW_WRITE_G(v, g, idx)                     GROUP_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_ENTRY_DTS) + 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_RX_FLOW_READ(r, p)                             MREAD_8((uint8_t *)p + 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_RX_FLOW_WRITE(v, p)                            MWRITE_8((uint8_t *)p + 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_G9991_BUFFER_READ_G(r, g, idx)                 GROUP_MREAD_16(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_ENTRY_DTS) + 4, r)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_G9991_BUFFER_WRITE_G(v, g, idx)                GROUP_MWRITE_16(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_ENTRY_DTS) + 4, v)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_G9991_BUFFER_READ(r, p)                        MREAD_16((uint8_t *)p + 4, r)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_G9991_BUFFER_WRITE(v, p)                       MWRITE_16((uint8_t *)p + 4, v)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_TX_FLOW_READ_G(r, g, idx)                      GROUP_MREAD_16(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_ENTRY_DTS) + 6, r)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_TX_FLOW_WRITE_G(v, g, idx)                     GROUP_MWRITE_16(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_ENTRY_DTS) + 6, v)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_TX_FLOW_READ(r, p)                             MREAD_16((uint8_t *)p + 6, r)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_TX_FLOW_WRITE(v, p)                            MWRITE_16((uint8_t *)p + 6, v)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_PER_FLOW_RESOLUTION_READ_G(r, g, idx, i)       GROUP_MREAD_I_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_ENTRY_DTS) + 8, i, r)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_PER_FLOW_RESOLUTION_WRITE_G(v, g, idx, i)      GROUP_MWRITE_I_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_ENTRY_DTS) + 8, i, v)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_PER_FLOW_RESOLUTION_READ(r, p, i)              MREAD_I_8((uint8_t *)p + 8, i, r)
#define RDD_PACKET_BUFFER_RESOLUTION_ENTRY_PER_FLOW_RESOLUTION_WRITE(v, p, i)             MWRITE_I_8((uint8_t *)p + 8, i, v)
/* <<<RDD_PACKET_BUFFER_RESOLUTION_ENTRY_DTS */

/* <<<RDD_PACKET_BUFFER_RESOLUTION_ENTRY */


/* >>>RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY */
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_IS_CPU_F_OFFSET                       31
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_IS_CPU_F_WIDTH                        1
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_IS_CPU_OFFSET                         0
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_IS_CPU_F_OFFSET_MOD8                  7
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_IS_CPU_F_OFFSET_MOD16                 15
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_CPU_TC_F_OFFSET                       28
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_CPU_TC_F_WIDTH                        3
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_CPU_TC_OFFSET                         0
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_CPU_TC_F_OFFSET_MOD8                  4
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_CPU_TC_F_OFFSET_MOD16                 12
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_CPU_RESERVED_F_OFFSET                 16
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_CPU_RESERVED_F_WIDTH                  12
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_CPU_RESERVED_OFFSET                   0
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_CPU_RESERVED_F_OFFSET_MOD16           0
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_POLICER_DEI_F_OFFSET                         8
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_POLICER_DEI_F_WIDTH                          1
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_POLICER_DEI_OFFSET                           2
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_POLICER_DEI_F_OFFSET_MOD8                    0
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_RULE2BRIDGE_RESOLUTION_F_OFFSET              16
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_RULE2BRIDGE_RESOLUTION_F_WIDTH               16
#define PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_RULE2BRIDGE_RESOLUTION_OFFSET                4

/* >>>RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS */
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_RESERVED_4_NUMBER	2

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	egress_is_cpu                                                              	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_cpu_tc                                                              	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_cpu_reserved                                                        	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1                                                                  	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	policer_dei                                                                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2                                                                  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rule2bridge_resolution                                                     	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = rule2bridge_resolution, size = 16 bits
	uint32_t	egress_port_resolved                                                       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved5                                                                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bridge_egress_vport                                                        	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved3                                                                  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved7                                                                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tc_to_queue                                                                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pbit_to_queue                                                              	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vlan_action                                                                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	forw_mode                                                                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tc                                                                         	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	drop_precedence                                                            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pbit_remark                                                                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved6                                                                  	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
	uint8_t	reserved_4[RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_RESERVED_4_NUMBER];
#else
	uint32_t	reserved2                                                                  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	policer_dei                                                                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1                                                                  	:7	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_cpu_reserved                                                        	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_cpu_tc                                                              	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_is_cpu                                                              	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	reserved_4[RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_RESERVED_4_NUMBER];
	uint32_t	rule2bridge_resolution                                                     	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
/* fields union = rule2bridge_resolution, size = 16 bits
	uint32_t	egress_port_resolved                                                       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved5                                                                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bridge_egress_vport                                                        	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved3                                                                  	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved7                                                                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tc_to_queue                                                                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pbit_to_queue                                                              	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vlan_action                                                                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	forw_mode                                                                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tc                                                                         	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	drop_precedence                                                            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pbit_remark                                                                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved6                                                                  	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
 end fields union*/
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS;

#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_IS_CPU_READ_G(r, g, idx)                 GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS), 7, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_IS_CPU_WRITE_G(v, g, idx)                GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS), 7, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_IS_CPU_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_IS_CPU_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_CPU_TC_READ_G(r, g, idx)                 GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS), 4, 3, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_CPU_TC_WRITE_G(v, g, idx)                GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS), 4, 3, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_CPU_TC_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p, 4, 3, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_CPU_TC_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p, 4, 3, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_CPU_RESERVED_READ_G(r, g, idx)           GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS), 0, 12, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_CPU_RESERVED_WRITE_G(v, g, idx)          GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS), 0, 12, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_CPU_RESERVED_READ(r, p)                  FIELD_MREAD_16((uint8_t *)p, 0, 12, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_CPU_RESERVED_WRITE(v, p)                 FIELD_MWRITE_16((uint8_t *)p, 0, 12, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_POLICER_DEI_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 2, 0, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_POLICER_DEI_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 2, 0, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_POLICER_DEI_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 2, 0, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_POLICER_DEI_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 2, 0, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_RULE2BRIDGE_RESOLUTION_READ_G(r, g, idx)        GROUP_MREAD_16(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 4, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_RULE2BRIDGE_RESOLUTION_WRITE_G(v, g, idx)       GROUP_MWRITE_16(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 4, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_RULE2BRIDGE_RESOLUTION_READ(r, p)               MREAD_16((uint8_t *)p + 4, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_RULE2BRIDGE_RESOLUTION_WRITE(v, p)              MWRITE_16((uint8_t *)p + 4, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_PORT_RESOLVED_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 4, 7, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_PORT_RESOLVED_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 4, 7, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_PORT_RESOLVED_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 4, 7, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_EGRESS_PORT_RESOLVED_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 4, 7, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_BRIDGE_EGRESS_VPORT_READ_G(r, g, idx)           GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 4, 0, 6, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_BRIDGE_EGRESS_VPORT_WRITE_G(v, g, idx)          GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 4, 0, 6, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_BRIDGE_EGRESS_VPORT_READ(r, p)                  FIELD_MREAD_8((uint8_t *)p + 4, 0, 6, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_BRIDGE_EGRESS_VPORT_WRITE(v, p)                 FIELD_MWRITE_8((uint8_t *)p + 4, 0, 6, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_TC_TO_QUEUE_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 4, 6, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_TC_TO_QUEUE_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 4, 6, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_TC_TO_QUEUE_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 4, 6, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_TC_TO_QUEUE_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 4, 6, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_PBIT_TO_QUEUE_READ_G(r, g, idx)                 GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 4, 5, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_PBIT_TO_QUEUE_WRITE_G(v, g, idx)                GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 4, 5, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_PBIT_TO_QUEUE_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 4, 5, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_PBIT_TO_QUEUE_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 4, 5, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_VLAN_ACTION_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 4, 4, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_VLAN_ACTION_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 4, 4, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_VLAN_ACTION_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 4, 4, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_VLAN_ACTION_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 4, 4, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_FORW_MODE_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 4, 3, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_FORW_MODE_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 4, 3, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_FORW_MODE_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 4, 3, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_FORW_MODE_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 4, 3, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_TC_READ_G(r, g, idx)                            GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 4, 0, 3, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_TC_WRITE_G(v, g, idx)                           GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 4, 0, 3, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_TC_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p + 4, 0, 3, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_TC_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p + 4, 0, 3, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DROP_PRECEDENCE_READ_G(r, g, idx)               GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 5, 7, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DROP_PRECEDENCE_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 5, 7, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DROP_PRECEDENCE_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 5, 7, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DROP_PRECEDENCE_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 5, 7, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_PBIT_REMARK_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 5, 6, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_PBIT_REMARK_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS) + 5, 6, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_PBIT_REMARK_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 5, 6, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_PBIT_REMARK_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 5, 6, 1, v)
/* <<<RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY_DTS */

/* <<<RDD_PACKET_BUFFER_RESOLUTION_UNICAST_ENTRY */


/* >>>RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY */
#define PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_IS_CPU_F_OFFSET                       31
#define PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_IS_CPU_F_WIDTH                        1
#define PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_IS_CPU_OFFSET                         0
#define PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_IS_CPU_F_OFFSET_MOD8                  7
#define PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_IS_CPU_F_OFFSET_MOD16                 15
#define PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_CPU_TC_F_OFFSET                       28
#define PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_CPU_TC_F_WIDTH                        3
#define PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_CPU_TC_OFFSET                         0
#define PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_CPU_TC_F_OFFSET_MOD8                  4
#define PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_CPU_TC_F_OFFSET_MOD16                 12
#define PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_CPU_RESERVED_F_OFFSET                 16
#define PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_CPU_RESERVED_F_WIDTH                  12
#define PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_CPU_RESERVED_OFFSET                   0
#define PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_CPU_RESERVED_F_OFFSET_MOD16           0
#define PACKET_BUFFER_RESOLUTION_1588_ENTRY_TIMESTAMP_OFFSET                             4
#define PACKET_BUFFER_RESOLUTION_1588_ENTRY_TIMESTAMP_F_OFFSET_MOD16                     8

/* >>>RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_DTS */
#define RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_TIMESTAMP_NUMBER	4

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	egress_is_cpu                                                         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_cpu_tc                                                         	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_cpu_reserved                                                   	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved                                                              	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	timestamp[RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_TIMESTAMP_NUMBER];
#else
	uint32_t	reserved                                                              	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_cpu_reserved                                                   	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_cpu_tc                                                         	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	egress_is_cpu                                                         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	timestamp[RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_TIMESTAMP_NUMBER];
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_DTS;

#define RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_IS_CPU_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_DTS), 7, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_IS_CPU_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_DTS), 7, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_IS_CPU_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_IS_CPU_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_CPU_TC_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_DTS), 4, 3, r)
#define RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_CPU_TC_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_DTS), 4, 3, v)
#define RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_CPU_TC_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p, 4, 3, r)
#define RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_CPU_TC_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p, 4, 3, v)
#define RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_CPU_RESERVED_READ_G(r, g, idx)          GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_DTS), 0, 12, r)
#define RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_CPU_RESERVED_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_DTS), 0, 12, v)
#define RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_CPU_RESERVED_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p, 0, 12, r)
#define RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_EGRESS_CPU_RESERVED_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p, 0, 12, v)
#define RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_TIMESTAMP_READ_G(r, g, idx, i)                 GROUP_MREAD_I_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_DTS) + 4, i, r)
#define RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_TIMESTAMP_WRITE_G(v, g, idx, i)                GROUP_MWRITE_I_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_DTS) + 4, i, v)
#define RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_TIMESTAMP_READ(r, p, i)                        MREAD_I_8((uint8_t *)p + 4, i, r)
#define RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_TIMESTAMP_WRITE(v, p, i)                       MWRITE_I_8((uint8_t *)p + 4, i, v)
/* <<<RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY_DTS */

/* <<<RDD_PACKET_BUFFER_RESOLUTION_1588_ENTRY */


/* >>>RDD_CPU_TX_DESCRIPTOR */
#define CPU_TX_DESCRIPTOR_VALID_F_OFFSET                               31
#define CPU_TX_DESCRIPTOR_VALID_F_WIDTH                                1
#define CPU_TX_DESCRIPTOR_VALID_OFFSET                                 0
#define CPU_TX_DESCRIPTOR_VALID_F_OFFSET_MOD8                          7
#define CPU_TX_DESCRIPTOR_VALID_F_OFFSET_MOD16                         15
#define CPU_TX_DESCRIPTOR_FIRST_LEVEL_Q_F_OFFSET                       22
#define CPU_TX_DESCRIPTOR_FIRST_LEVEL_Q_F_WIDTH                        9
#define CPU_TX_DESCRIPTOR_FIRST_LEVEL_Q_OFFSET                         0
#define CPU_TX_DESCRIPTOR_FIRST_LEVEL_Q_F_OFFSET_MOD16                 6
#define CPU_TX_DESCRIPTOR_ABS_DATA1_F_OFFSET                           0
#define CPU_TX_DESCRIPTOR_ABS_DATA1_F_WIDTH                            22
#define CPU_TX_DESCRIPTOR_ABS_DATA1_OFFSET                             0
#define CPU_TX_DESCRIPTOR_ABS_DATA0_F_OFFSET                           14
#define CPU_TX_DESCRIPTOR_ABS_DATA0_F_WIDTH                            18
#define CPU_TX_DESCRIPTOR_ABS_DATA0_OFFSET                             4
#define CPU_TX_DESCRIPTOR_PACKET_LENGTH_F_OFFSET                       0
#define CPU_TX_DESCRIPTOR_PACKET_LENGTH_F_WIDTH                        14
#define CPU_TX_DESCRIPTOR_PACKET_LENGTH_OFFSET                         6
#define CPU_TX_DESCRIPTOR_DROP_F_OFFSET                                31
#define CPU_TX_DESCRIPTOR_DROP_F_WIDTH                                 1
#define CPU_TX_DESCRIPTOR_DROP_OFFSET                                  8
#define CPU_TX_DESCRIPTOR_DROP_F_OFFSET_MOD8                           7
#define CPU_TX_DESCRIPTOR_DROP_F_OFFSET_MOD16                          15
#define CPU_TX_DESCRIPTOR_FLAG_1588_F_OFFSET                           30
#define CPU_TX_DESCRIPTOR_FLAG_1588_F_WIDTH                            1
#define CPU_TX_DESCRIPTOR_FLAG_1588_OFFSET                             8
#define CPU_TX_DESCRIPTOR_FLAG_1588_F_OFFSET_MOD8                      6
#define CPU_TX_DESCRIPTOR_FLAG_1588_F_OFFSET_MOD16                     14
#define CPU_TX_DESCRIPTOR_COLOR_F_OFFSET                               29
#define CPU_TX_DESCRIPTOR_COLOR_F_WIDTH                                1
#define CPU_TX_DESCRIPTOR_COLOR_OFFSET                                 8
#define CPU_TX_DESCRIPTOR_COLOR_F_OFFSET_MOD8                          5
#define CPU_TX_DESCRIPTOR_COLOR_F_OFFSET_MOD16                         13
#define CPU_TX_DESCRIPTOR_DO_NOT_RECYCLE_F_OFFSET                      28
#define CPU_TX_DESCRIPTOR_DO_NOT_RECYCLE_F_WIDTH                       1
#define CPU_TX_DESCRIPTOR_DO_NOT_RECYCLE_OFFSET                        8
#define CPU_TX_DESCRIPTOR_DO_NOT_RECYCLE_F_OFFSET_MOD8                 4
#define CPU_TX_DESCRIPTOR_DO_NOT_RECYCLE_F_OFFSET_MOD16                12
#define CPU_TX_DESCRIPTOR_LAN_F_OFFSET                                 26
#define CPU_TX_DESCRIPTOR_LAN_F_WIDTH                                  1
#define CPU_TX_DESCRIPTOR_LAN_OFFSET                                   8
#define CPU_TX_DESCRIPTOR_LAN_F_OFFSET_MOD8                            2
#define CPU_TX_DESCRIPTOR_LAN_F_OFFSET_MOD16                           10
#define CPU_TX_DESCRIPTOR_WAN_FLOW_SOURCE_PORT_F_OFFSET                18
#define CPU_TX_DESCRIPTOR_WAN_FLOW_SOURCE_PORT_F_WIDTH                 8
#define CPU_TX_DESCRIPTOR_WAN_FLOW_SOURCE_PORT_OFFSET                  8
#define CPU_TX_DESCRIPTOR_WAN_FLOW_SOURCE_PORT_F_OFFSET_MOD16          2
#define CPU_TX_DESCRIPTOR_BN1_OR_ABS2_OR_1588_F_OFFSET                 0
#define CPU_TX_DESCRIPTOR_BN1_OR_ABS2_OR_1588_F_WIDTH                  18
#define CPU_TX_DESCRIPTOR_BN1_OR_ABS2_OR_1588_OFFSET                   8
#define CPU_TX_DESCRIPTOR_AGG_PD_F_OFFSET                              31
#define CPU_TX_DESCRIPTOR_AGG_PD_F_WIDTH                               1
#define CPU_TX_DESCRIPTOR_AGG_PD_OFFSET                                12
#define CPU_TX_DESCRIPTOR_AGG_PD_F_OFFSET_MOD8                         7
#define CPU_TX_DESCRIPTOR_AGG_PD_F_OFFSET_MOD16                        15
#define CPU_TX_DESCRIPTOR_TARGET_MEM_0_F_OFFSET                        30
#define CPU_TX_DESCRIPTOR_TARGET_MEM_0_F_WIDTH                         1
#define CPU_TX_DESCRIPTOR_TARGET_MEM_0_OFFSET                          12
#define CPU_TX_DESCRIPTOR_TARGET_MEM_0_F_OFFSET_MOD8                   6
#define CPU_TX_DESCRIPTOR_TARGET_MEM_0_F_OFFSET_MOD16                  14
#define CPU_TX_DESCRIPTOR_ABS_F_OFFSET                                 29
#define CPU_TX_DESCRIPTOR_ABS_F_WIDTH                                  1
#define CPU_TX_DESCRIPTOR_ABS_OFFSET                                   12
#define CPU_TX_DESCRIPTOR_ABS_F_OFFSET_MOD8                            5
#define CPU_TX_DESCRIPTOR_ABS_F_OFFSET_MOD16                           13
#define CPU_TX_DESCRIPTOR_PAYLOAD_OFFSET_OR_ABS_1_F_OFFSET             18
#define CPU_TX_DESCRIPTOR_PAYLOAD_OFFSET_OR_ABS_1_F_WIDTH              11
#define CPU_TX_DESCRIPTOR_PAYLOAD_OFFSET_OR_ABS_1_OFFSET               12
#define CPU_TX_DESCRIPTOR_PAYLOAD_OFFSET_OR_ABS_1_F_OFFSET_MOD16       2
#define CPU_TX_DESCRIPTOR_BUFFER_NUMBER_0_OR_ABS_0_F_OFFSET            0
#define CPU_TX_DESCRIPTOR_BUFFER_NUMBER_0_OR_ABS_0_F_WIDTH             18
#define CPU_TX_DESCRIPTOR_BUFFER_NUMBER_0_OR_ABS_0_OFFSET              12
/* <<<RDD_CPU_TX_DESCRIPTOR */


/* >>>RDD_TC_TO_QUEUE_8 */
#define TC_TO_QUEUE_8_QUEUE_OFFSET_OFFSET                          0

/* >>>RDD_TC_TO_QUEUE_8_DTS */
#define RDD_TC_TO_QUEUE_8_QUEUE_OFFSET_NUMBER	8

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint8_t	queue_offset[RDD_TC_TO_QUEUE_8_QUEUE_OFFSET_NUMBER];
#else
	uint8_t	queue_offset[RDD_TC_TO_QUEUE_8_QUEUE_OFFSET_NUMBER];
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_TC_TO_QUEUE_8_DTS;

#define RDD_TC_TO_QUEUE_8_QUEUE_OFFSET_READ_G(r, g, idx, i)       GROUP_MREAD_I_8(g, idx*sizeof(RDD_TC_TO_QUEUE_8_DTS), i, r)
#define RDD_TC_TO_QUEUE_8_QUEUE_OFFSET_WRITE_G(v, g, idx, i)      GROUP_MWRITE_I_8(g, idx*sizeof(RDD_TC_TO_QUEUE_8_DTS), i, v)
#define RDD_TC_TO_QUEUE_8_QUEUE_OFFSET_READ(r, p, i)              MREAD_I_8((uint8_t *)p, i, r)
#define RDD_TC_TO_QUEUE_8_QUEUE_OFFSET_WRITE(v, p, i)             MWRITE_I_8((uint8_t *)p, i, v)
/* <<<RDD_TC_TO_QUEUE_8_DTS */

/* <<<RDD_TC_TO_QUEUE_8 */


/* >>>RDD_TC_TO_QUEUE_32 */
#define TC_TO_QUEUE_32_QUEUE_OFFSET_OFFSET                          0

/* >>>RDD_TC_TO_QUEUE_32_DTS */
#define RDD_TC_TO_QUEUE_32_QUEUE_OFFSET_NUMBER	32

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint8_t	queue_offset[RDD_TC_TO_QUEUE_32_QUEUE_OFFSET_NUMBER];
#else
	uint8_t	queue_offset[RDD_TC_TO_QUEUE_32_QUEUE_OFFSET_NUMBER];
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_TC_TO_QUEUE_32_DTS;

#define RDD_TC_TO_QUEUE_32_QUEUE_OFFSET_READ_G(r, g, idx, i)       GROUP_MREAD_I_8(g, idx*sizeof(RDD_TC_TO_QUEUE_32_DTS), i, r)
#define RDD_TC_TO_QUEUE_32_QUEUE_OFFSET_WRITE_G(v, g, idx, i)      GROUP_MWRITE_I_8(g, idx*sizeof(RDD_TC_TO_QUEUE_32_DTS), i, v)
#define RDD_TC_TO_QUEUE_32_QUEUE_OFFSET_READ(r, p, i)              MREAD_I_8((uint8_t *)p, i, r)
#define RDD_TC_TO_QUEUE_32_QUEUE_OFFSET_WRITE(v, p, i)             MWRITE_I_8((uint8_t *)p, i, v)
/* <<<RDD_TC_TO_QUEUE_32_DTS */

/* <<<RDD_TC_TO_QUEUE_32 */


/* >>>RDD_PBIT_TO_QUEUE_8 */
#define PBIT_TO_QUEUE_8_QUEUE_OFFSET_OFFSET                          0

/* >>>RDD_PBIT_TO_QUEUE_8_DTS */
#define RDD_PBIT_TO_QUEUE_8_QUEUE_OFFSET_NUMBER	8

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint8_t	queue_offset[RDD_PBIT_TO_QUEUE_8_QUEUE_OFFSET_NUMBER];
#else
	uint8_t	queue_offset[RDD_PBIT_TO_QUEUE_8_QUEUE_OFFSET_NUMBER];
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PBIT_TO_QUEUE_8_DTS;

#define RDD_PBIT_TO_QUEUE_8_QUEUE_OFFSET_READ_G(r, g, idx, i)       GROUP_MREAD_I_8(g, idx*sizeof(RDD_PBIT_TO_QUEUE_8_DTS), i, r)
#define RDD_PBIT_TO_QUEUE_8_QUEUE_OFFSET_WRITE_G(v, g, idx, i)      GROUP_MWRITE_I_8(g, idx*sizeof(RDD_PBIT_TO_QUEUE_8_DTS), i, v)
#define RDD_PBIT_TO_QUEUE_8_QUEUE_OFFSET_READ(r, p, i)              MREAD_I_8((uint8_t *)p, i, r)
#define RDD_PBIT_TO_QUEUE_8_QUEUE_OFFSET_WRITE(v, p, i)             MWRITE_I_8((uint8_t *)p, i, v)
/* <<<RDD_PBIT_TO_QUEUE_8_DTS */

/* <<<RDD_PBIT_TO_QUEUE_8 */


/* >>>RDD_VPORT_CFG_ENTRY */
#define VPORT_CFG_ENTRY_LOOPBACK_EN_F_OFFSET                         15
#define VPORT_CFG_ENTRY_LOOPBACK_EN_F_WIDTH                          1
#define VPORT_CFG_ENTRY_LOOPBACK_EN_OFFSET                           0
#define VPORT_CFG_ENTRY_LOOPBACK_EN_F_OFFSET_MOD8                    7
#define VPORT_CFG_ENTRY_MIRRORING_EN_F_OFFSET                        14
#define VPORT_CFG_ENTRY_MIRRORING_EN_F_WIDTH                         1
#define VPORT_CFG_ENTRY_MIRRORING_EN_OFFSET                          0
#define VPORT_CFG_ENTRY_MIRRORING_EN_F_OFFSET_MOD8                   6
#define VPORT_CFG_ENTRY_SA_LOOKUP_EN_F_OFFSET                        13
#define VPORT_CFG_ENTRY_SA_LOOKUP_EN_F_WIDTH                         1
#define VPORT_CFG_ENTRY_SA_LOOKUP_EN_OFFSET                          0
#define VPORT_CFG_ENTRY_SA_LOOKUP_EN_F_OFFSET_MOD8                   5
#define VPORT_CFG_ENTRY_DA_LOOKUP_EN_F_OFFSET                        12
#define VPORT_CFG_ENTRY_DA_LOOKUP_EN_F_WIDTH                         1
#define VPORT_CFG_ENTRY_DA_LOOKUP_EN_OFFSET                          0
#define VPORT_CFG_ENTRY_DA_LOOKUP_EN_F_OFFSET_MOD8                   4
#define VPORT_CFG_ENTRY_EGRESS_ISOLATION_EN_F_OFFSET                 11
#define VPORT_CFG_ENTRY_EGRESS_ISOLATION_EN_F_WIDTH                  1
#define VPORT_CFG_ENTRY_EGRESS_ISOLATION_EN_OFFSET                   0
#define VPORT_CFG_ENTRY_EGRESS_ISOLATION_EN_F_OFFSET_MOD8            3
#define VPORT_CFG_ENTRY_INGRESS_ISOLATION_EN_F_OFFSET                10
#define VPORT_CFG_ENTRY_INGRESS_ISOLATION_EN_F_WIDTH                 1
#define VPORT_CFG_ENTRY_INGRESS_ISOLATION_EN_OFFSET                  0
#define VPORT_CFG_ENTRY_INGRESS_ISOLATION_EN_F_OFFSET_MOD8           2
#define VPORT_CFG_ENTRY_BRIDGE_AND_VLAN_LOOKUP_METHOD_F_OFFSET       9
#define VPORT_CFG_ENTRY_BRIDGE_AND_VLAN_LOOKUP_METHOD_F_WIDTH        1
#define VPORT_CFG_ENTRY_BRIDGE_AND_VLAN_LOOKUP_METHOD_OFFSET         0
#define VPORT_CFG_ENTRY_BRIDGE_AND_VLAN_LOOKUP_METHOD_F_OFFSET_MOD8  1
#define VPORT_CFG_ENTRY_DISCARD_PRTY_F_OFFSET                        8
#define VPORT_CFG_ENTRY_DISCARD_PRTY_F_WIDTH                         1
#define VPORT_CFG_ENTRY_DISCARD_PRTY_OFFSET                          0
#define VPORT_CFG_ENTRY_DISCARD_PRTY_F_OFFSET_MOD8                   0
#define VPORT_CFG_ENTRY_SA_LOOKUP_MISS_ACTION_F_OFFSET               3
#define VPORT_CFG_ENTRY_SA_LOOKUP_MISS_ACTION_F_WIDTH                2
#define VPORT_CFG_ENTRY_SA_LOOKUP_MISS_ACTION_OFFSET                 1
#define VPORT_CFG_ENTRY_DA_LOOKUP_MISS_ACTION_F_OFFSET               1
#define VPORT_CFG_ENTRY_DA_LOOKUP_MISS_ACTION_F_WIDTH                2
#define VPORT_CFG_ENTRY_DA_LOOKUP_MISS_ACTION_OFFSET                 1
#define VPORT_CFG_ENTRY_LS_FC_CFG_F_OFFSET                           0
#define VPORT_CFG_ENTRY_LS_FC_CFG_F_WIDTH                            1
#define VPORT_CFG_ENTRY_LS_FC_CFG_OFFSET                             1

/* >>>RDD_VPORT_CFG_ENTRY_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint16_t	loopback_en                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	mirroring_en                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	sa_lookup_en                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	da_lookup_en                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	egress_isolation_en          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	ingress_isolation_en         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	bridge_and_vlan_lookup_method	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__; /*defined by rdd_bridge_and_vlan_ctx_lookup_method enumeration*/
	uint16_t	discard_prty                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	reserved2                    	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	sa_lookup_miss_action        	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__; /*defined by rdd_action enumeration*/
	uint16_t	da_lookup_miss_action        	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__; /*defined by rdd_action enumeration*/
	uint16_t	ls_fc_cfg                    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint16_t	ls_fc_cfg                    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	da_lookup_miss_action        	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__; /*defined by rdd_action enumeration*/
	uint16_t	sa_lookup_miss_action        	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__; /*defined by rdd_action enumeration*/
	uint16_t	reserved2                    	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	discard_prty                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	bridge_and_vlan_lookup_method	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__; /*defined by rdd_bridge_and_vlan_ctx_lookup_method enumeration*/
	uint16_t	ingress_isolation_en         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	egress_isolation_en          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	da_lookup_en                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	sa_lookup_en                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	mirroring_en                 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	loopback_en                  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_VPORT_CFG_ENTRY_DTS;

#define RDD_VPORT_CFG_ENTRY_LOOPBACK_EN_READ_G(r, g, idx)                            GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS), 7, 1, r)
#define RDD_VPORT_CFG_ENTRY_LOOPBACK_EN_WRITE_G(v, g, idx)                           GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS), 7, 1, v)
#define RDD_VPORT_CFG_ENTRY_LOOPBACK_EN_READ(r, p)                                   FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_VPORT_CFG_ENTRY_LOOPBACK_EN_WRITE(v, p)                                  FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_VPORT_CFG_ENTRY_MIRRORING_EN_READ_G(r, g, idx)                           GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS), 6, 1, r)
#define RDD_VPORT_CFG_ENTRY_MIRRORING_EN_WRITE_G(v, g, idx)                          GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS), 6, 1, v)
#define RDD_VPORT_CFG_ENTRY_MIRRORING_EN_READ(r, p)                                  FIELD_MREAD_8((uint8_t *)p, 6, 1, r)
#define RDD_VPORT_CFG_ENTRY_MIRRORING_EN_WRITE(v, p)                                 FIELD_MWRITE_8((uint8_t *)p, 6, 1, v)
#define RDD_VPORT_CFG_ENTRY_SA_LOOKUP_EN_READ_G(r, g, idx)                           GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS), 5, 1, r)
#define RDD_VPORT_CFG_ENTRY_SA_LOOKUP_EN_WRITE_G(v, g, idx)                          GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS), 5, 1, v)
#define RDD_VPORT_CFG_ENTRY_SA_LOOKUP_EN_READ(r, p)                                  FIELD_MREAD_8((uint8_t *)p, 5, 1, r)
#define RDD_VPORT_CFG_ENTRY_SA_LOOKUP_EN_WRITE(v, p)                                 FIELD_MWRITE_8((uint8_t *)p, 5, 1, v)
#define RDD_VPORT_CFG_ENTRY_DA_LOOKUP_EN_READ_G(r, g, idx)                           GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS), 4, 1, r)
#define RDD_VPORT_CFG_ENTRY_DA_LOOKUP_EN_WRITE_G(v, g, idx)                          GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS), 4, 1, v)
#define RDD_VPORT_CFG_ENTRY_DA_LOOKUP_EN_READ(r, p)                                  FIELD_MREAD_8((uint8_t *)p, 4, 1, r)
#define RDD_VPORT_CFG_ENTRY_DA_LOOKUP_EN_WRITE(v, p)                                 FIELD_MWRITE_8((uint8_t *)p, 4, 1, v)
#define RDD_VPORT_CFG_ENTRY_EGRESS_ISOLATION_EN_READ_G(r, g, idx)                    GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS), 3, 1, r)
#define RDD_VPORT_CFG_ENTRY_EGRESS_ISOLATION_EN_WRITE_G(v, g, idx)                   GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS), 3, 1, v)
#define RDD_VPORT_CFG_ENTRY_EGRESS_ISOLATION_EN_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p, 3, 1, r)
#define RDD_VPORT_CFG_ENTRY_EGRESS_ISOLATION_EN_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p, 3, 1, v)
#define RDD_VPORT_CFG_ENTRY_INGRESS_ISOLATION_EN_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS), 2, 1, r)
#define RDD_VPORT_CFG_ENTRY_INGRESS_ISOLATION_EN_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS), 2, 1, v)
#define RDD_VPORT_CFG_ENTRY_INGRESS_ISOLATION_EN_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p, 2, 1, r)
#define RDD_VPORT_CFG_ENTRY_INGRESS_ISOLATION_EN_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p, 2, 1, v)
#define RDD_VPORT_CFG_ENTRY_BRIDGE_AND_VLAN_LOOKUP_METHOD_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS), 1, 1, r) /*defined by rdd_bridge_and_vlan_ctx_lookup_method enumeration*/
#define RDD_VPORT_CFG_ENTRY_BRIDGE_AND_VLAN_LOOKUP_METHOD_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS), 1, 1, v) /*defined by rdd_bridge_and_vlan_ctx_lookup_method enumeration*/
#define RDD_VPORT_CFG_ENTRY_BRIDGE_AND_VLAN_LOOKUP_METHOD_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 1, 1, r) /*defined by rdd_bridge_and_vlan_ctx_lookup_method enumeration*/
#define RDD_VPORT_CFG_ENTRY_BRIDGE_AND_VLAN_LOOKUP_METHOD_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 1, 1, v) /*defined by rdd_bridge_and_vlan_ctx_lookup_method enumeration*/
#define RDD_VPORT_CFG_ENTRY_DISCARD_PRTY_READ_G(r, g, idx)                           GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS), 0, 1, r)
#define RDD_VPORT_CFG_ENTRY_DISCARD_PRTY_WRITE_G(v, g, idx)                          GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS), 0, 1, v)
#define RDD_VPORT_CFG_ENTRY_DISCARD_PRTY_READ(r, p)                                  FIELD_MREAD_8((uint8_t *)p, 0, 1, r)
#define RDD_VPORT_CFG_ENTRY_DISCARD_PRTY_WRITE(v, p)                                 FIELD_MWRITE_8((uint8_t *)p, 0, 1, v)
#define RDD_VPORT_CFG_ENTRY_SA_LOOKUP_MISS_ACTION_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS) + 1, 3, 2, r) /*defined by rdd_action enumeration*/
#define RDD_VPORT_CFG_ENTRY_SA_LOOKUP_MISS_ACTION_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS) + 1, 3, 2, v) /*defined by rdd_action enumeration*/
#define RDD_VPORT_CFG_ENTRY_SA_LOOKUP_MISS_ACTION_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 1, 3, 2, r) /*defined by rdd_action enumeration*/
#define RDD_VPORT_CFG_ENTRY_SA_LOOKUP_MISS_ACTION_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 1, 3, 2, v) /*defined by rdd_action enumeration*/
#define RDD_VPORT_CFG_ENTRY_DA_LOOKUP_MISS_ACTION_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS) + 1, 1, 2, r) /*defined by rdd_action enumeration*/
#define RDD_VPORT_CFG_ENTRY_DA_LOOKUP_MISS_ACTION_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS) + 1, 1, 2, v) /*defined by rdd_action enumeration*/
#define RDD_VPORT_CFG_ENTRY_DA_LOOKUP_MISS_ACTION_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 1, 1, 2, r) /*defined by rdd_action enumeration*/
#define RDD_VPORT_CFG_ENTRY_DA_LOOKUP_MISS_ACTION_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 1, 1, 2, v) /*defined by rdd_action enumeration*/
#define RDD_VPORT_CFG_ENTRY_LS_FC_CFG_READ_G(r, g, idx)                              GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS) + 1, 0, 1, r)
#define RDD_VPORT_CFG_ENTRY_LS_FC_CFG_WRITE_G(v, g, idx)                             GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_VPORT_CFG_ENTRY_DTS) + 1, 0, 1, v)
#define RDD_VPORT_CFG_ENTRY_LS_FC_CFG_READ(r, p)                                     FIELD_MREAD_8((uint8_t *)p + 1, 0, 1, r)
#define RDD_VPORT_CFG_ENTRY_LS_FC_CFG_WRITE(v, p)                                    FIELD_MWRITE_8((uint8_t *)p + 1, 0, 1, v)
/* <<<RDD_VPORT_CFG_ENTRY_DTS */

/* <<<RDD_VPORT_CFG_ENTRY */


/* >>>RDD_FPM_GLOBAL_CFG */
#define FPM_GLOBAL_CFG_FPM_BASE_LOW_F_OFFSET                        0
#define FPM_GLOBAL_CFG_FPM_BASE_LOW_F_WIDTH                         32
#define FPM_GLOBAL_CFG_FPM_BASE_LOW_OFFSET                          0
#define FPM_GLOBAL_CFG_FPM_BASE_HIGH_F_OFFSET                       0
#define FPM_GLOBAL_CFG_FPM_BASE_HIGH_F_WIDTH                        32
#define FPM_GLOBAL_CFG_FPM_BASE_HIGH_OFFSET                         4
#define FPM_GLOBAL_CFG_FPM_TOKEN_SIZE_ASR_8_F_OFFSET                24
#define FPM_GLOBAL_CFG_FPM_TOKEN_SIZE_ASR_8_F_WIDTH                 8
#define FPM_GLOBAL_CFG_FPM_TOKEN_SIZE_ASR_8_OFFSET                  8
#define FPM_GLOBAL_CFG_FPM_TOKEN_SIZE_ASR_8_F_OFFSET_MOD16          8
/* <<<RDD_FPM_GLOBAL_CFG */


/* >>>RDD_VLAN_TAG */
#define VLAN_TAG_PBITS_F_OFFSET                               13
#define VLAN_TAG_PBITS_F_WIDTH                                3
#define VLAN_TAG_PBITS_OFFSET                                 0
#define VLAN_TAG_PBITS_F_OFFSET_MOD8                          5
#define VLAN_TAG_CFI_F_OFFSET                                 12
#define VLAN_TAG_CFI_F_WIDTH                                  1
#define VLAN_TAG_CFI_OFFSET                                   0
#define VLAN_TAG_CFI_F_OFFSET_MOD8                            4
#define VLAN_TAG_VID_F_OFFSET                                 0
#define VLAN_TAG_VID_F_WIDTH                                  12
#define VLAN_TAG_VID_OFFSET                                   0

/* >>>RDD_VLAN_TAG_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint16_t	pbits     	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	cfi       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	vid       	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint16_t	vid       	:12	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	cfi       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint16_t	pbits     	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_VLAN_TAG_DTS;

#define RDD_VLAN_TAG_PBITS_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_VLAN_TAG_DTS), 5, 3, r)
#define RDD_VLAN_TAG_PBITS_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_VLAN_TAG_DTS), 5, 3, v)
#define RDD_VLAN_TAG_PBITS_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 5, 3, r)
#define RDD_VLAN_TAG_PBITS_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 5, 3, v)
#define RDD_VLAN_TAG_CFI_READ_G(r, g, idx)            GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_VLAN_TAG_DTS), 4, 1, r)
#define RDD_VLAN_TAG_CFI_WRITE_G(v, g, idx)           GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_VLAN_TAG_DTS), 4, 1, v)
#define RDD_VLAN_TAG_CFI_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p, 4, 1, r)
#define RDD_VLAN_TAG_CFI_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p, 4, 1, v)
#define RDD_VLAN_TAG_VID_READ_G(r, g, idx)            GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_VLAN_TAG_DTS), 0, 12, r)
#define RDD_VLAN_TAG_VID_WRITE_G(v, g, idx)           GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_VLAN_TAG_DTS), 0, 12, v)
#define RDD_VLAN_TAG_VID_READ(r, p)                   FIELD_MREAD_16((uint8_t *)p, 0, 12, r)
#define RDD_VLAN_TAG_VID_WRITE(v, p)                  FIELD_MWRITE_16((uint8_t *)p, 0, 12, v)
/* <<<RDD_VLAN_TAG_DTS */

/* <<<RDD_VLAN_TAG */


/* >>>RDD_CPU_RX_METER_ENTRY */
#define CPU_RX_METER_ENTRY_CURRENT_BUDGET_F_OFFSET                      16
#define CPU_RX_METER_ENTRY_CURRENT_BUDGET_F_WIDTH                       16
#define CPU_RX_METER_ENTRY_CURRENT_BUDGET_OFFSET                        0
#define CPU_RX_METER_ENTRY_BUDGET_LIMIT_F_OFFSET                        0
#define CPU_RX_METER_ENTRY_BUDGET_LIMIT_F_WIDTH                         16
#define CPU_RX_METER_ENTRY_BUDGET_LIMIT_OFFSET                          2
#define CPU_RX_METER_ENTRY_ALLOCATED_BUDGET_F_OFFSET                    16
#define CPU_RX_METER_ENTRY_ALLOCATED_BUDGET_F_WIDTH                     16
#define CPU_RX_METER_ENTRY_ALLOCATED_BUDGET_OFFSET                      4
/* <<<RDD_CPU_RX_METER_ENTRY */


/* >>>RDD_CPU_REDIRECT_MODE_ENTRY */
#define CPU_REDIRECT_MODE_ENTRY_MODE_F_OFFSET                                0
#define CPU_REDIRECT_MODE_ENTRY_MODE_F_WIDTH                                 8
#define CPU_REDIRECT_MODE_ENTRY_MODE_OFFSET                                  0

/* >>>RDD_CPU_REDIRECT_MODE_ENTRY_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint8_t	mode      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint8_t	mode      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_CPU_REDIRECT_MODE_ENTRY_DTS;

#define RDD_CPU_REDIRECT_MODE_ENTRY_MODE_READ_G(r, g, idx)          GROUP_MREAD_8(g, idx*sizeof(RDD_CPU_REDIRECT_MODE_ENTRY_DTS), r)
#define RDD_CPU_REDIRECT_MODE_ENTRY_MODE_WRITE_G(v, g, idx)         GROUP_MWRITE_8(g, idx*sizeof(RDD_CPU_REDIRECT_MODE_ENTRY_DTS), v)
#define RDD_CPU_REDIRECT_MODE_ENTRY_MODE_READ(r, p)                 MREAD_8((uint8_t *)p, r)
#define RDD_CPU_REDIRECT_MODE_ENTRY_MODE_WRITE(v, p)                MWRITE_8((uint8_t *)p, v)
/* <<<RDD_CPU_REDIRECT_MODE_ENTRY_DTS */

/* <<<RDD_CPU_REDIRECT_MODE_ENTRY */


/* >>>RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY */
#define PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_EGRESS_PORTS_VECTOR_F_OFFSET                 0
#define PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_EGRESS_PORTS_VECTOR_F_WIDTH                  32
#define PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_EGRESS_PORTS_VECTOR_OFFSET                   0
#define PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_IPTV_REPL_0_SOP_F_OFFSET                     24
#define PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_IPTV_REPL_0_SOP_F_WIDTH                      8
#define PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_IPTV_REPL_0_SOP_OFFSET                       4
#define PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_IPTV_REPL_0_SOP_F_OFFSET_MOD16               8
#define PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_POOL_NUM_F_OFFSET                            22
#define PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_POOL_NUM_F_WIDTH                             2
#define PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_POOL_NUM_OFFSET                              5
#define PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_POOL_NUM_F_OFFSET_MOD8                       6
#define PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_POOL_NUM_F_OFFSET_MOD16                      6
#define PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_REPLICATIONS_F_OFFSET                        16
#define PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_REPLICATIONS_F_WIDTH                         6
#define PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_REPLICATIONS_OFFSET                          5
#define PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_REPLICATIONS_F_OFFSET_MOD8                   0
#define PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_REPLICATIONS_F_OFFSET_MOD16                  0
#define PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_TC_F_OFFSET                                  0
#define PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_TC_F_WIDTH                                   3
#define PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_TC_OFFSET                                    7

/* >>>RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	egress_ports_vector	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	iptv_repl_0_sop    	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pool_num           	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	replications       	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1          	:13	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tc                 	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	egress_ports_vector	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tc                 	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1          	:13	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	replications       	:6	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pool_num           	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	iptv_repl_0_sop    	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_DTS;

#define RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_EGRESS_PORTS_VECTOR_READ_G(r, g, idx)          GROUP_MREAD_32(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_DTS), r)
#define RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_EGRESS_PORTS_VECTOR_WRITE_G(v, g, idx)         GROUP_MWRITE_32(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_DTS), v)
#define RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_EGRESS_PORTS_VECTOR_READ(r, p)                 MREAD_32((uint8_t *)p, r)
#define RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_EGRESS_PORTS_VECTOR_WRITE(v, p)                MWRITE_32((uint8_t *)p, v)
#define RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_IPTV_REPL_0_SOP_READ_G(r, g, idx)              GROUP_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_DTS) + 4, r)
#define RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_IPTV_REPL_0_SOP_WRITE_G(v, g, idx)             GROUP_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_DTS) + 4, v)
#define RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_IPTV_REPL_0_SOP_READ(r, p)                     MREAD_8((uint8_t *)p + 4, r)
#define RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_IPTV_REPL_0_SOP_WRITE(v, p)                    MWRITE_8((uint8_t *)p + 4, v)
#define RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_POOL_NUM_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_DTS) + 5, 6, 2, r)
#define RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_POOL_NUM_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_DTS) + 5, 6, 2, v)
#define RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_POOL_NUM_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 5, 6, 2, r)
#define RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_POOL_NUM_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 5, 6, 2, v)
#define RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_REPLICATIONS_READ_G(r, g, idx)                 GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_DTS) + 5, 0, 6, r)
#define RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_REPLICATIONS_WRITE_G(v, g, idx)                GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_DTS) + 5, 0, 6, v)
#define RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_REPLICATIONS_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p + 5, 0, 6, r)
#define RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_REPLICATIONS_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p + 5, 0, 6, v)
#define RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_TC_READ_G(r, g, idx)                           GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_DTS) + 7, 0, 3, r)
#define RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_TC_WRITE_G(v, g, idx)                          GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_DTS) + 7, 0, 3, v)
#define RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_TC_READ(r, p)                                  FIELD_MREAD_8((uint8_t *)p + 7, 0, 3, r)
#define RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_TC_WRITE(v, p)                                 FIELD_MWRITE_8((uint8_t *)p + 7, 0, 3, v)
/* <<<RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY_DTS */

/* <<<RDD_PACKET_BUFFER_RESOLUTION_MCAST_ENTRY */


/* >>>RDD_PARSER_SUMMARY */
#define PARSER_SUMMARY_DA_FILTER_MATCH_F_OFFSET                     31
#define PARSER_SUMMARY_DA_FILTER_MATCH_F_WIDTH                      1
#define PARSER_SUMMARY_DA_FILTER_MATCH_OFFSET                       0
#define PARSER_SUMMARY_DA_FILTER_MATCH_F_OFFSET_MOD8                7
#define PARSER_SUMMARY_DA_FILTER_MATCH_F_OFFSET_MOD16               15
#define PARSER_SUMMARY_FIRST_IP_FRAGMENT_F_OFFSET                   30
#define PARSER_SUMMARY_FIRST_IP_FRAGMENT_F_WIDTH                    1
#define PARSER_SUMMARY_FIRST_IP_FRAGMENT_OFFSET                     0
#define PARSER_SUMMARY_FIRST_IP_FRAGMENT_F_OFFSET_MOD8              6
#define PARSER_SUMMARY_FIRST_IP_FRAGMENT_F_OFFSET_MOD16             14
#define PARSER_SUMMARY_IP_FRAGMENT_F_OFFSET                         29
#define PARSER_SUMMARY_IP_FRAGMENT_F_WIDTH                          1
#define PARSER_SUMMARY_IP_FRAGMENT_OFFSET                           0
#define PARSER_SUMMARY_IP_FRAGMENT_F_OFFSET_MOD8                    5
#define PARSER_SUMMARY_IP_FRAGMENT_F_OFFSET_MOD16                   13
#define PARSER_SUMMARY_IP_FILTER_MATCH_F_OFFSET                     28
#define PARSER_SUMMARY_IP_FILTER_MATCH_F_WIDTH                      1
#define PARSER_SUMMARY_IP_FILTER_MATCH_OFFSET                       0
#define PARSER_SUMMARY_IP_FILTER_MATCH_F_OFFSET_MOD8                4
#define PARSER_SUMMARY_IP_FILTER_MATCH_F_OFFSET_MOD16               12
#define PARSER_SUMMARY_IP_FILTER_NUM_F_OFFSET                       26
#define PARSER_SUMMARY_IP_FILTER_NUM_F_WIDTH                        2
#define PARSER_SUMMARY_IP_FILTER_NUM_OFFSET                         0
#define PARSER_SUMMARY_IP_FILTER_NUM_F_OFFSET_MOD8                  2
#define PARSER_SUMMARY_IP_FILTER_NUM_F_OFFSET_MOD16                 10
#define PARSER_SUMMARY_TCP_UDP_F_OFFSET                             25
#define PARSER_SUMMARY_TCP_UDP_F_WIDTH                              1
#define PARSER_SUMMARY_TCP_UDP_OFFSET                               0
#define PARSER_SUMMARY_TCP_UDP_F_OFFSET_MOD8                        1
#define PARSER_SUMMARY_TCP_UDP_F_OFFSET_MOD16                       9
#define PARSER_SUMMARY_IPV6_EXT_HEADER_F_OFFSET                     24
#define PARSER_SUMMARY_IPV6_EXT_HEADER_F_WIDTH                      1
#define PARSER_SUMMARY_IPV6_EXT_HEADER_OFFSET                       0
#define PARSER_SUMMARY_IPV6_EXT_HEADER_F_OFFSET_MOD8                0
#define PARSER_SUMMARY_IPV6_EXT_HEADER_F_OFFSET_MOD16               8
#define PARSER_SUMMARY_TCP_FLAG_F_OFFSET                            23
#define PARSER_SUMMARY_TCP_FLAG_F_WIDTH                             1
#define PARSER_SUMMARY_TCP_FLAG_OFFSET                              1
#define PARSER_SUMMARY_TCP_FLAG_F_OFFSET_MOD8                       7
#define PARSER_SUMMARY_TCP_FLAG_F_OFFSET_MOD16                      7
#define PARSER_SUMMARY_P_TAG_F_OFFSET                               22
#define PARSER_SUMMARY_P_TAG_F_WIDTH                                1
#define PARSER_SUMMARY_P_TAG_OFFSET                                 1
#define PARSER_SUMMARY_P_TAG_F_OFFSET_MOD8                          6
#define PARSER_SUMMARY_P_TAG_F_OFFSET_MOD16                         6
#define PARSER_SUMMARY_VID_FILTER_HIT_F_OFFSET                      21
#define PARSER_SUMMARY_VID_FILTER_HIT_F_WIDTH                       1
#define PARSER_SUMMARY_VID_FILTER_HIT_OFFSET                        1
#define PARSER_SUMMARY_VID_FILTER_HIT_F_OFFSET_MOD8                 5
#define PARSER_SUMMARY_VID_FILTER_HIT_F_OFFSET_MOD16                5
#define PARSER_SUMMARY_EXCEPTION_F_OFFSET                           20
#define PARSER_SUMMARY_EXCEPTION_F_WIDTH                            1
#define PARSER_SUMMARY_EXCEPTION_OFFSET                             1
#define PARSER_SUMMARY_EXCEPTION_F_OFFSET_MOD8                      4
#define PARSER_SUMMARY_EXCEPTION_F_OFFSET_MOD16                     4
#define PARSER_SUMMARY_DA_FILTER_NUMBER_F_OFFSET                    16
#define PARSER_SUMMARY_DA_FILTER_NUMBER_F_WIDTH                     4
#define PARSER_SUMMARY_DA_FILTER_NUMBER_OFFSET                      1
#define PARSER_SUMMARY_DA_FILTER_NUMBER_F_OFFSET_MOD8               0
#define PARSER_SUMMARY_DA_FILTER_NUMBER_F_OFFSET_MOD16              0
#define PARSER_SUMMARY_L4_PROTOCOL_F_OFFSET                         12
#define PARSER_SUMMARY_L4_PROTOCOL_F_WIDTH                          4
#define PARSER_SUMMARY_L4_PROTOCOL_OFFSET                           2
#define PARSER_SUMMARY_L4_PROTOCOL_F_OFFSET_MOD8                    4
#define PARSER_SUMMARY__5_TUP_VALID_F_OFFSET                        11
#define PARSER_SUMMARY__5_TUP_VALID_F_WIDTH                         1
#define PARSER_SUMMARY__5_TUP_VALID_OFFSET                          2
#define PARSER_SUMMARY__5_TUP_VALID_F_OFFSET_MOD8                   3
#define PARSER_SUMMARY_DHCP_F_OFFSET                                10
#define PARSER_SUMMARY_DHCP_F_WIDTH                                 1
#define PARSER_SUMMARY_DHCP_OFFSET                                  2
#define PARSER_SUMMARY_DHCP_F_OFFSET_MOD8                           2
#define PARSER_SUMMARY_VLANS_NUM_F_OFFSET                           8
#define PARSER_SUMMARY_VLANS_NUM_F_WIDTH                            2
#define PARSER_SUMMARY_VLANS_NUM_OFFSET                             2
#define PARSER_SUMMARY_VLANS_NUM_F_OFFSET_MOD8                      0
#define PARSER_SUMMARY_BROADCAST_F_OFFSET                           7
#define PARSER_SUMMARY_BROADCAST_F_WIDTH                            1
#define PARSER_SUMMARY_BROADCAST_OFFSET                             3
#define PARSER_SUMMARY_MULTICAST_F_OFFSET                           6
#define PARSER_SUMMARY_MULTICAST_F_WIDTH                            1
#define PARSER_SUMMARY_MULTICAST_OFFSET                             3
#define PARSER_SUMMARY_L3_PROTOCOL_F_OFFSET                         4
#define PARSER_SUMMARY_L3_PROTOCOL_F_WIDTH                          2
#define PARSER_SUMMARY_L3_PROTOCOL_OFFSET                           3
#define PARSER_SUMMARY_L2_PROTOCOL_F_OFFSET                         0
#define PARSER_SUMMARY_L2_PROTOCOL_F_WIDTH                          4
#define PARSER_SUMMARY_L2_PROTOCOL_OFFSET                           3

/* >>>RDD_PARSER_SUMMARY_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	da_filter_match  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	first_ip_fragment	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_fragment      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_filter_match  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_filter_num    	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tcp_udp          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ipv6_ext_header  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tcp_flag         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	p_tag            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vid_filter_hit   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	exception        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	da_filter_number 	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	l4_protocol      	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__; /*defined by rdd_parser_l4_protocol enumeration*/
	uint32_t	_5_tup_valid     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dhcp             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vlans_num        	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	broadcast        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	multicast        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	l3_protocol      	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__; /*defined by rdd_parser_l3_protocol enumeration*/
	uint32_t	l2_protocol      	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	l2_protocol      	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	l3_protocol      	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__; /*defined by rdd_parser_l3_protocol enumeration*/
	uint32_t	multicast        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	broadcast        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vlans_num        	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dhcp             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	_5_tup_valid     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	l4_protocol      	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__; /*defined by rdd_parser_l4_protocol enumeration*/
	uint32_t	da_filter_number 	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	exception        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vid_filter_hit   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	p_tag            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tcp_flag         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ipv6_ext_header  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tcp_udp          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_filter_num    	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_filter_match  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_fragment      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	first_ip_fragment	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	da_filter_match  	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PARSER_SUMMARY_DTS;

#define RDD_PARSER_SUMMARY_DA_FILTER_MATCH_READ_G(r, g, idx)            GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS), 7, 1, r)
#define RDD_PARSER_SUMMARY_DA_FILTER_MATCH_WRITE_G(v, g, idx)           GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS), 7, 1, v)
#define RDD_PARSER_SUMMARY_DA_FILTER_MATCH_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_PARSER_SUMMARY_DA_FILTER_MATCH_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_PARSER_SUMMARY_FIRST_IP_FRAGMENT_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS), 6, 1, r)
#define RDD_PARSER_SUMMARY_FIRST_IP_FRAGMENT_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS), 6, 1, v)
#define RDD_PARSER_SUMMARY_FIRST_IP_FRAGMENT_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 6, 1, r)
#define RDD_PARSER_SUMMARY_FIRST_IP_FRAGMENT_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 6, 1, v)
#define RDD_PARSER_SUMMARY_IP_FRAGMENT_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS), 5, 1, r)
#define RDD_PARSER_SUMMARY_IP_FRAGMENT_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS), 5, 1, v)
#define RDD_PARSER_SUMMARY_IP_FRAGMENT_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p, 5, 1, r)
#define RDD_PARSER_SUMMARY_IP_FRAGMENT_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p, 5, 1, v)
#define RDD_PARSER_SUMMARY_IP_FILTER_MATCH_READ_G(r, g, idx)            GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS), 4, 1, r)
#define RDD_PARSER_SUMMARY_IP_FILTER_MATCH_WRITE_G(v, g, idx)           GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS), 4, 1, v)
#define RDD_PARSER_SUMMARY_IP_FILTER_MATCH_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p, 4, 1, r)
#define RDD_PARSER_SUMMARY_IP_FILTER_MATCH_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p, 4, 1, v)
#define RDD_PARSER_SUMMARY_IP_FILTER_NUM_READ_G(r, g, idx)              GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS), 2, 2, r)
#define RDD_PARSER_SUMMARY_IP_FILTER_NUM_WRITE_G(v, g, idx)             GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS), 2, 2, v)
#define RDD_PARSER_SUMMARY_IP_FILTER_NUM_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p, 2, 2, r)
#define RDD_PARSER_SUMMARY_IP_FILTER_NUM_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p, 2, 2, v)
#define RDD_PARSER_SUMMARY_TCP_UDP_READ_G(r, g, idx)                    GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS), 1, 1, r)
#define RDD_PARSER_SUMMARY_TCP_UDP_WRITE_G(v, g, idx)                   GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS), 1, 1, v)
#define RDD_PARSER_SUMMARY_TCP_UDP_READ(r, p)                           FIELD_MREAD_8((uint8_t *)p, 1, 1, r)
#define RDD_PARSER_SUMMARY_TCP_UDP_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p, 1, 1, v)
#define RDD_PARSER_SUMMARY_IPV6_EXT_HEADER_READ_G(r, g, idx)            GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS), 0, 1, r)
#define RDD_PARSER_SUMMARY_IPV6_EXT_HEADER_WRITE_G(v, g, idx)           GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS), 0, 1, v)
#define RDD_PARSER_SUMMARY_IPV6_EXT_HEADER_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p, 0, 1, r)
#define RDD_PARSER_SUMMARY_IPV6_EXT_HEADER_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p, 0, 1, v)
#define RDD_PARSER_SUMMARY_TCP_FLAG_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 1, 7, 1, r)
#define RDD_PARSER_SUMMARY_TCP_FLAG_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 1, 7, 1, v)
#define RDD_PARSER_SUMMARY_TCP_FLAG_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 1, 7, 1, r)
#define RDD_PARSER_SUMMARY_TCP_FLAG_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 1, 7, 1, v)
#define RDD_PARSER_SUMMARY_P_TAG_READ_G(r, g, idx)                      GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 1, 6, 1, r)
#define RDD_PARSER_SUMMARY_P_TAG_WRITE_G(v, g, idx)                     GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 1, 6, 1, v)
#define RDD_PARSER_SUMMARY_P_TAG_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 1, 6, 1, r)
#define RDD_PARSER_SUMMARY_P_TAG_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 1, 6, 1, v)
#define RDD_PARSER_SUMMARY_VID_FILTER_HIT_READ_G(r, g, idx)             GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 1, 5, 1, r)
#define RDD_PARSER_SUMMARY_VID_FILTER_HIT_WRITE_G(v, g, idx)            GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 1, 5, 1, v)
#define RDD_PARSER_SUMMARY_VID_FILTER_HIT_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 1, 5, 1, r)
#define RDD_PARSER_SUMMARY_VID_FILTER_HIT_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 1, 5, 1, v)
#define RDD_PARSER_SUMMARY_EXCEPTION_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 1, 4, 1, r)
#define RDD_PARSER_SUMMARY_EXCEPTION_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 1, 4, 1, v)
#define RDD_PARSER_SUMMARY_EXCEPTION_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 1, 4, 1, r)
#define RDD_PARSER_SUMMARY_EXCEPTION_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 1, 4, 1, v)
#define RDD_PARSER_SUMMARY_DA_FILTER_NUMBER_READ_G(r, g, idx)           GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 1, 0, 4, r)
#define RDD_PARSER_SUMMARY_DA_FILTER_NUMBER_WRITE_G(v, g, idx)          GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 1, 0, 4, v)
#define RDD_PARSER_SUMMARY_DA_FILTER_NUMBER_READ(r, p)                  FIELD_MREAD_8((uint8_t *)p + 1, 0, 4, r)
#define RDD_PARSER_SUMMARY_DA_FILTER_NUMBER_WRITE(v, p)                 FIELD_MWRITE_8((uint8_t *)p + 1, 0, 4, v)
#define RDD_PARSER_SUMMARY_L4_PROTOCOL_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 2, 4, 4, r) /*defined by rdd_parser_l4_protocol enumeration*/
#define RDD_PARSER_SUMMARY_L4_PROTOCOL_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 2, 4, 4, v) /*defined by rdd_parser_l4_protocol enumeration*/
#define RDD_PARSER_SUMMARY_L4_PROTOCOL_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 2, 4, 4, r) /*defined by rdd_parser_l4_protocol enumeration*/
#define RDD_PARSER_SUMMARY_L4_PROTOCOL_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 2, 4, 4, v) /*defined by rdd_parser_l4_protocol enumeration*/
#define RDD_PARSER_SUMMARY__5_TUP_VALID_READ_G(r, g, idx)               GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 2, 3, 1, r)
#define RDD_PARSER_SUMMARY__5_TUP_VALID_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 2, 3, 1, v)
#define RDD_PARSER_SUMMARY__5_TUP_VALID_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 2, 3, 1, r)
#define RDD_PARSER_SUMMARY__5_TUP_VALID_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 2, 3, 1, v)
#define RDD_PARSER_SUMMARY_DHCP_READ_G(r, g, idx)                       GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 2, 2, 1, r)
#define RDD_PARSER_SUMMARY_DHCP_WRITE_G(v, g, idx)                      GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 2, 2, 1, v)
#define RDD_PARSER_SUMMARY_DHCP_READ(r, p)                              FIELD_MREAD_8((uint8_t *)p + 2, 2, 1, r)
#define RDD_PARSER_SUMMARY_DHCP_WRITE(v, p)                             FIELD_MWRITE_8((uint8_t *)p + 2, 2, 1, v)
#define RDD_PARSER_SUMMARY_VLANS_NUM_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 2, 0, 2, r)
#define RDD_PARSER_SUMMARY_VLANS_NUM_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 2, 0, 2, v)
#define RDD_PARSER_SUMMARY_VLANS_NUM_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 2, 0, 2, r)
#define RDD_PARSER_SUMMARY_VLANS_NUM_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 2, 0, 2, v)
#define RDD_PARSER_SUMMARY_BROADCAST_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 3, 7, 1, r)
#define RDD_PARSER_SUMMARY_BROADCAST_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 3, 7, 1, v)
#define RDD_PARSER_SUMMARY_BROADCAST_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 3, 7, 1, r)
#define RDD_PARSER_SUMMARY_BROADCAST_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 3, 7, 1, v)
#define RDD_PARSER_SUMMARY_MULTICAST_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 3, 6, 1, r)
#define RDD_PARSER_SUMMARY_MULTICAST_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 3, 6, 1, v)
#define RDD_PARSER_SUMMARY_MULTICAST_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 3, 6, 1, r)
#define RDD_PARSER_SUMMARY_MULTICAST_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 3, 6, 1, v)
#define RDD_PARSER_SUMMARY_L3_PROTOCOL_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 3, 4, 2, r) /*defined by rdd_parser_l3_protocol enumeration*/
#define RDD_PARSER_SUMMARY_L3_PROTOCOL_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 3, 4, 2, v) /*defined by rdd_parser_l3_protocol enumeration*/
#define RDD_PARSER_SUMMARY_L3_PROTOCOL_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 3, 4, 2, r) /*defined by rdd_parser_l3_protocol enumeration*/
#define RDD_PARSER_SUMMARY_L3_PROTOCOL_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 3, 4, 2, v) /*defined by rdd_parser_l3_protocol enumeration*/
#define RDD_PARSER_SUMMARY_L2_PROTOCOL_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 3, 0, 4, r)
#define RDD_PARSER_SUMMARY_L2_PROTOCOL_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_SUMMARY_DTS) + 3, 0, 4, v)
#define RDD_PARSER_SUMMARY_L2_PROTOCOL_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 3, 0, 4, r)
#define RDD_PARSER_SUMMARY_L2_PROTOCOL_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 3, 0, 4, v)
/* <<<RDD_PARSER_SUMMARY_DTS */

/* <<<RDD_PARSER_SUMMARY */


/* >>>RDD_PARSER_RESULT */
#define PARSER_RESULT_DA_FILTER_MATCH_F_OFFSET                     31
#define PARSER_RESULT_DA_FILTER_MATCH_F_WIDTH                      1
#define PARSER_RESULT_DA_FILTER_MATCH_OFFSET                       0
#define PARSER_RESULT_DA_FILTER_MATCH_F_OFFSET_MOD8                7
#define PARSER_RESULT_DA_FILTER_MATCH_F_OFFSET_MOD16               15
#define PARSER_RESULT_FIRST_IP_FRAGMENT_F_OFFSET                   30
#define PARSER_RESULT_FIRST_IP_FRAGMENT_F_WIDTH                    1
#define PARSER_RESULT_FIRST_IP_FRAGMENT_OFFSET                     0
#define PARSER_RESULT_FIRST_IP_FRAGMENT_F_OFFSET_MOD8              6
#define PARSER_RESULT_FIRST_IP_FRAGMENT_F_OFFSET_MOD16             14
#define PARSER_RESULT_IP_FRAGMENT_F_OFFSET                         29
#define PARSER_RESULT_IP_FRAGMENT_F_WIDTH                          1
#define PARSER_RESULT_IP_FRAGMENT_OFFSET                           0
#define PARSER_RESULT_IP_FRAGMENT_F_OFFSET_MOD8                    5
#define PARSER_RESULT_IP_FRAGMENT_F_OFFSET_MOD16                   13
#define PARSER_RESULT_IP_FILTER_MATCH_F_OFFSET                     28
#define PARSER_RESULT_IP_FILTER_MATCH_F_WIDTH                      1
#define PARSER_RESULT_IP_FILTER_MATCH_OFFSET                       0
#define PARSER_RESULT_IP_FILTER_MATCH_F_OFFSET_MOD8                4
#define PARSER_RESULT_IP_FILTER_MATCH_F_OFFSET_MOD16               12
#define PARSER_RESULT_IP_FILTER_NUM_F_OFFSET                       26
#define PARSER_RESULT_IP_FILTER_NUM_F_WIDTH                        2
#define PARSER_RESULT_IP_FILTER_NUM_OFFSET                         0
#define PARSER_RESULT_IP_FILTER_NUM_F_OFFSET_MOD8                  2
#define PARSER_RESULT_IP_FILTER_NUM_F_OFFSET_MOD16                 10
#define PARSER_RESULT_TCP_UDP_F_OFFSET                             25
#define PARSER_RESULT_TCP_UDP_F_WIDTH                              1
#define PARSER_RESULT_TCP_UDP_OFFSET                               0
#define PARSER_RESULT_TCP_UDP_F_OFFSET_MOD8                        1
#define PARSER_RESULT_TCP_UDP_F_OFFSET_MOD16                       9
#define PARSER_RESULT_IPV6_EXT_HEADER_F_OFFSET                     24
#define PARSER_RESULT_IPV6_EXT_HEADER_F_WIDTH                      1
#define PARSER_RESULT_IPV6_EXT_HEADER_OFFSET                       0
#define PARSER_RESULT_IPV6_EXT_HEADER_F_OFFSET_MOD8                0
#define PARSER_RESULT_IPV6_EXT_HEADER_F_OFFSET_MOD16               8
#define PARSER_RESULT_TCP_FLAG_F_OFFSET                            23
#define PARSER_RESULT_TCP_FLAG_F_WIDTH                             1
#define PARSER_RESULT_TCP_FLAG_OFFSET                              1
#define PARSER_RESULT_TCP_FLAG_F_OFFSET_MOD8                       7
#define PARSER_RESULT_TCP_FLAG_F_OFFSET_MOD16                      7
#define PARSER_RESULT_P_TAG_F_OFFSET                               22
#define PARSER_RESULT_P_TAG_F_WIDTH                                1
#define PARSER_RESULT_P_TAG_OFFSET                                 1
#define PARSER_RESULT_P_TAG_F_OFFSET_MOD8                          6
#define PARSER_RESULT_P_TAG_F_OFFSET_MOD16                         6
#define PARSER_RESULT_VID_FILTER_HIT_F_OFFSET                      21
#define PARSER_RESULT_VID_FILTER_HIT_F_WIDTH                       1
#define PARSER_RESULT_VID_FILTER_HIT_OFFSET                        1
#define PARSER_RESULT_VID_FILTER_HIT_F_OFFSET_MOD8                 5
#define PARSER_RESULT_VID_FILTER_HIT_F_OFFSET_MOD16                5
#define PARSER_RESULT_EXCEPTION_F_OFFSET                           20
#define PARSER_RESULT_EXCEPTION_F_WIDTH                            1
#define PARSER_RESULT_EXCEPTION_OFFSET                             1
#define PARSER_RESULT_EXCEPTION_F_OFFSET_MOD8                      4
#define PARSER_RESULT_EXCEPTION_F_OFFSET_MOD16                     4
#define PARSER_RESULT_DA_FILTER_NUMBER_F_OFFSET                    16
#define PARSER_RESULT_DA_FILTER_NUMBER_F_WIDTH                     4
#define PARSER_RESULT_DA_FILTER_NUMBER_OFFSET                      1
#define PARSER_RESULT_DA_FILTER_NUMBER_F_OFFSET_MOD8               0
#define PARSER_RESULT_DA_FILTER_NUMBER_F_OFFSET_MOD16              0
#define PARSER_RESULT_LAYER4_PROTOCOL_F_OFFSET                     12
#define PARSER_RESULT_LAYER4_PROTOCOL_F_WIDTH                      4
#define PARSER_RESULT_LAYER4_PROTOCOL_OFFSET                       2
#define PARSER_RESULT_LAYER4_PROTOCOL_F_OFFSET_MOD8                4
#define PARSER_RESULT__5_TUP_VALID_F_OFFSET                        11
#define PARSER_RESULT__5_TUP_VALID_F_WIDTH                         1
#define PARSER_RESULT__5_TUP_VALID_OFFSET                          2
#define PARSER_RESULT__5_TUP_VALID_F_OFFSET_MOD8                   3
#define PARSER_RESULT_DHCP_F_OFFSET                                10
#define PARSER_RESULT_DHCP_F_WIDTH                                 1
#define PARSER_RESULT_DHCP_OFFSET                                  2
#define PARSER_RESULT_DHCP_F_OFFSET_MOD8                           2
#define PARSER_RESULT_VLANS_NUM_F_OFFSET                           8
#define PARSER_RESULT_VLANS_NUM_F_WIDTH                            2
#define PARSER_RESULT_VLANS_NUM_OFFSET                             2
#define PARSER_RESULT_VLANS_NUM_F_OFFSET_MOD8                      0
#define PARSER_RESULT_BROADCAST_F_OFFSET                           7
#define PARSER_RESULT_BROADCAST_F_WIDTH                            1
#define PARSER_RESULT_BROADCAST_OFFSET                             3
#define PARSER_RESULT_MULTICAST_F_OFFSET                           6
#define PARSER_RESULT_MULTICAST_F_WIDTH                            1
#define PARSER_RESULT_MULTICAST_OFFSET                             3
#define PARSER_RESULT_LAYER3_PROTOCOL_F_OFFSET                     4
#define PARSER_RESULT_LAYER3_PROTOCOL_F_WIDTH                      2
#define PARSER_RESULT_LAYER3_PROTOCOL_OFFSET                       3
#define PARSER_RESULT_LAYER2_PROTOCOL_F_OFFSET                     0
#define PARSER_RESULT_LAYER2_PROTOCOL_F_WIDTH                      4
#define PARSER_RESULT_LAYER2_PROTOCOL_OFFSET                       3
#define PARSER_RESULT_TAG_TYPE_F_OFFSET                            30
#define PARSER_RESULT_TAG_TYPE_F_WIDTH                             2
#define PARSER_RESULT_TAG_TYPE_OFFSET                              4
#define PARSER_RESULT_TAG_TYPE_F_OFFSET_MOD8                       6
#define PARSER_RESULT_TAG_TYPE_F_OFFSET_MOD16                      14
#define PARSER_RESULT_MAC_SPOOF_F_OFFSET                           29
#define PARSER_RESULT_MAC_SPOOF_F_WIDTH                            1
#define PARSER_RESULT_MAC_SPOOF_OFFSET                             4
#define PARSER_RESULT_MAC_SPOOF_F_OFFSET_MOD8                      5
#define PARSER_RESULT_MAC_SPOOF_F_OFFSET_MOD16                     13
#define PARSER_RESULT_TPID_VLAN_0_F_OFFSET                         26
#define PARSER_RESULT_TPID_VLAN_0_F_WIDTH                          3
#define PARSER_RESULT_TPID_VLAN_0_OFFSET                           4
#define PARSER_RESULT_TPID_VLAN_0_F_OFFSET_MOD8                    2
#define PARSER_RESULT_TPID_VLAN_0_F_OFFSET_MOD16                   10
#define PARSER_RESULT_TPID_VLAN_1_F_OFFSET                         23
#define PARSER_RESULT_TPID_VLAN_1_F_WIDTH                          3
#define PARSER_RESULT_TPID_VLAN_1_OFFSET                           4
#define PARSER_RESULT_TPID_VLAN_1_F_OFFSET_MOD8                    7
#define PARSER_RESULT_TPID_VLAN_1_F_OFFSET_MOD16                   7
#define PARSER_RESULT_TPID_VLAN_2_F_OFFSET                         20
#define PARSER_RESULT_TPID_VLAN_2_F_WIDTH                          3
#define PARSER_RESULT_TPID_VLAN_2_OFFSET                           5
#define PARSER_RESULT_TPID_VLAN_2_F_OFFSET_MOD8                    4
#define PARSER_RESULT_TPID_VLAN_2_F_OFFSET_MOD16                   4
#define PARSER_RESULT_IN_CFI_F_OFFSET                              19
#define PARSER_RESULT_IN_CFI_F_WIDTH                               1
#define PARSER_RESULT_IN_CFI_OFFSET                                5
#define PARSER_RESULT_IN_CFI_F_OFFSET_MOD8                         3
#define PARSER_RESULT_IN_CFI_F_OFFSET_MOD16                        3
#define PARSER_RESULT_OUT_CFI_F_OFFSET                             18
#define PARSER_RESULT_OUT_CFI_F_WIDTH                              1
#define PARSER_RESULT_OUT_CFI_OFFSET                               5
#define PARSER_RESULT_OUT_CFI_F_OFFSET_MOD8                        2
#define PARSER_RESULT_OUT_CFI_F_OFFSET_MOD16                       2
#define PARSER_RESULT_NO_OUTER_F_OFFSET                            17
#define PARSER_RESULT_NO_OUTER_F_WIDTH                             1
#define PARSER_RESULT_NO_OUTER_OFFSET                              5
#define PARSER_RESULT_NO_OUTER_F_OFFSET_MOD8                       1
#define PARSER_RESULT_NO_OUTER_F_OFFSET_MOD16                      1
#define PARSER_RESULT_NO_INNER_F_OFFSET                            16
#define PARSER_RESULT_NO_INNER_F_WIDTH                             1
#define PARSER_RESULT_NO_INNER_OFFSET                              5
#define PARSER_RESULT_NO_INNER_F_OFFSET_MOD8                       0
#define PARSER_RESULT_NO_INNER_F_OFFSET_MOD16                      0
#define PARSER_RESULT_ETH_VERSION_F_OFFSET                         14
#define PARSER_RESULT_ETH_VERSION_F_WIDTH                          2
#define PARSER_RESULT_ETH_VERSION_OFFSET                           6
#define PARSER_RESULT_ETH_VERSION_F_OFFSET_MOD8                    6
#define PARSER_RESULT_IP_MC_L2_F_OFFSET                            12
#define PARSER_RESULT_IP_MC_L2_F_WIDTH                             1
#define PARSER_RESULT_IP_MC_L2_OFFSET                              6
#define PARSER_RESULT_IP_MC_L2_F_OFFSET_MOD8                       4
#define PARSER_RESULT_VID_FILTER_MATCH_F_OFFSET                    8
#define PARSER_RESULT_VID_FILTER_MATCH_F_WIDTH                     4
#define PARSER_RESULT_VID_FILTER_MATCH_OFFSET                      6
#define PARSER_RESULT_VID_FILTER_MATCH_F_OFFSET_MOD8               0
#define PARSER_RESULT_LAYER2_ADDRESS_F_OFFSET                      0
#define PARSER_RESULT_LAYER2_ADDRESS_F_WIDTH                       8
#define PARSER_RESULT_LAYER2_ADDRESS_OFFSET                        7
#define PARSER_RESULT_OUTER_VLAN_F_OFFSET                          16
#define PARSER_RESULT_OUTER_VLAN_F_WIDTH                           16
#define PARSER_RESULT_OUTER_VLAN_OFFSET                            8
#define PARSER_RESULT_DA1_2_F_OFFSET                               0
#define PARSER_RESULT_DA1_2_F_WIDTH                                16
#define PARSER_RESULT_DA1_2_OFFSET                                 10
#define PARSER_RESULT_DA3_6_F_OFFSET                               0
#define PARSER_RESULT_DA3_6_F_WIDTH                                32
#define PARSER_RESULT_DA3_6_OFFSET                                 12
#define PARSER_RESULT_INNER_VLAN_F_OFFSET                          16
#define PARSER_RESULT_INNER_VLAN_F_WIDTH                           16
#define PARSER_RESULT_INNER_VLAN_OFFSET                            16
#define PARSER_RESULT_SA1_2_F_OFFSET                               0
#define PARSER_RESULT_SA1_2_F_WIDTH                                16
#define PARSER_RESULT_SA1_2_OFFSET                                 18
#define PARSER_RESULT_SA3_6_F_OFFSET                               0
#define PARSER_RESULT_SA3_6_F_WIDTH                                32
#define PARSER_RESULT_SA3_6_OFFSET                                 20
#define PARSER_RESULT__3RD_VLAN_F_OFFSET                           16
#define PARSER_RESULT__3RD_VLAN_F_WIDTH                            16
#define PARSER_RESULT__3RD_VLAN_OFFSET                             24
#define PARSER_RESULT_ETHERTYPE_F_OFFSET                           0
#define PARSER_RESULT_ETHERTYPE_F_WIDTH                            16
#define PARSER_RESULT_ETHERTYPE_OFFSET                             26
#define PARSER_RESULT_IP_LENGTH_F_OFFSET                           16
#define PARSER_RESULT_IP_LENGTH_F_WIDTH                            16
#define PARSER_RESULT_IP_LENGTH_OFFSET                             28
#define PARSER_RESULT_IP_TTL_F_OFFSET                              8
#define PARSER_RESULT_IP_TTL_F_WIDTH                               8
#define PARSER_RESULT_IP_TTL_OFFSET                                30
#define PARSER_RESULT_TOS_F_OFFSET                                 0
#define PARSER_RESULT_TOS_F_WIDTH                                  8
#define PARSER_RESULT_TOS_OFFSET                                   31
#define PARSER_RESULT_LAYER4_OFFSET_F_OFFSET                       24
#define PARSER_RESULT_LAYER4_OFFSET_F_WIDTH                        8
#define PARSER_RESULT_LAYER4_OFFSET_OFFSET                         32
#define PARSER_RESULT_LAYER4_OFFSET_F_OFFSET_MOD16                 8
#define PARSER_RESULT_LAYER3_OFFSET_F_OFFSET                       16
#define PARSER_RESULT_LAYER3_OFFSET_F_WIDTH                        8
#define PARSER_RESULT_LAYER3_OFFSET_OFFSET                         33
#define PARSER_RESULT_LAYER3_OFFSET_F_OFFSET_MOD16                 0
#define PARSER_RESULT_MC_L3_CTL_F_OFFSET                           12
#define PARSER_RESULT_MC_L3_CTL_F_WIDTH                            1
#define PARSER_RESULT_MC_L3_CTL_OFFSET                             34
#define PARSER_RESULT_MC_L3_CTL_F_OFFSET_MOD8                      4
#define PARSER_RESULT_MC_L3_F_OFFSET                               11
#define PARSER_RESULT_MC_L3_F_WIDTH                                1
#define PARSER_RESULT_MC_L3_OFFSET                                 34
#define PARSER_RESULT_MC_L3_F_OFFSET_MOD8                          3
#define PARSER_RESULT_ERROR_F_OFFSET                               10
#define PARSER_RESULT_ERROR_F_WIDTH                                1
#define PARSER_RESULT_ERROR_OFFSET                                 34
#define PARSER_RESULT_ERROR_F_OFFSET_MOD8                          2
#define PARSER_RESULT_NON_FIVE_TUPLE_F_OFFSET                      9
#define PARSER_RESULT_NON_FIVE_TUPLE_F_WIDTH                       1
#define PARSER_RESULT_NON_FIVE_TUPLE_OFFSET                        34
#define PARSER_RESULT_NON_FIVE_TUPLE_F_OFFSET_MOD8                 1
#define PARSER_RESULT_IP_LEN_ERROR_F_OFFSET                        8
#define PARSER_RESULT_IP_LEN_ERROR_F_WIDTH                         1
#define PARSER_RESULT_IP_LEN_ERROR_OFFSET                          34
#define PARSER_RESULT_IP_LEN_ERROR_F_OFFSET_MOD8                   0
#define PARSER_RESULT_PROTOCOL_F_OFFSET                            0
#define PARSER_RESULT_PROTOCOL_F_WIDTH                             8
#define PARSER_RESULT_PROTOCOL_OFFSET                              35
#define PARSER_RESULT_SOURCE_PORT_F_OFFSET                         16
#define PARSER_RESULT_SOURCE_PORT_F_WIDTH                          16
#define PARSER_RESULT_SOURCE_PORT_OFFSET                           36
#define PARSER_RESULT_DESTINATION_PORT_F_OFFSET                    0
#define PARSER_RESULT_DESTINATION_PORT_F_WIDTH                     16
#define PARSER_RESULT_DESTINATION_PORT_OFFSET                      38
#define PARSER_RESULT_SOURCE_IP_F_OFFSET                           0
#define PARSER_RESULT_SOURCE_IP_F_WIDTH                            32
#define PARSER_RESULT_SOURCE_IP_OFFSET                             40
#define PARSER_RESULT_DESTINATION_IP_F_OFFSET                      0
#define PARSER_RESULT_DESTINATION_IP_F_WIDTH                       32
#define PARSER_RESULT_DESTINATION_IP_OFFSET                        44
#define PARSER_RESULT_FLAGS_F_OFFSET                               24
#define PARSER_RESULT_FLAGS_F_WIDTH                                8
#define PARSER_RESULT_FLAGS_OFFSET                                 48
#define PARSER_RESULT_FLAGS_F_OFFSET_MOD16                         8
#define PARSER_RESULT_ICMPV6_F_OFFSET                              23
#define PARSER_RESULT_ICMPV6_F_WIDTH                               1
#define PARSER_RESULT_ICMPV6_OFFSET                                49
#define PARSER_RESULT_ICMPV6_F_OFFSET_MOD8                         7
#define PARSER_RESULT_ICMPV6_F_OFFSET_MOD16                        7
#define PARSER_RESULT_V6_AH_F_OFFSET                               22
#define PARSER_RESULT_V6_AH_F_WIDTH                                1
#define PARSER_RESULT_V6_AH_OFFSET                                 49
#define PARSER_RESULT_V6_AH_F_OFFSET_MOD8                          6
#define PARSER_RESULT_V6_AH_F_OFFSET_MOD16                         6
#define PARSER_RESULT_V6_DEST_OPT_F_OFFSET                         21
#define PARSER_RESULT_V6_DEST_OPT_F_WIDTH                          1
#define PARSER_RESULT_V6_DEST_OPT_OFFSET                           49
#define PARSER_RESULT_V6_DEST_OPT_F_OFFSET_MOD8                    5
#define PARSER_RESULT_V6_DEST_OPT_F_OFFSET_MOD16                   5
#define PARSER_RESULT_V6_ROUTE_F_OFFSET                            20
#define PARSER_RESULT_V6_ROUTE_F_WIDTH                             1
#define PARSER_RESULT_V6_ROUTE_OFFSET                              49
#define PARSER_RESULT_V6_ROUTE_F_OFFSET_MOD8                       4
#define PARSER_RESULT_V6_ROUTE_F_OFFSET_MOD16                      4
#define PARSER_RESULT_V6_HOP_F_OFFSET                              19
#define PARSER_RESULT_V6_HOP_F_WIDTH                               1
#define PARSER_RESULT_V6_HOP_OFFSET                                49
#define PARSER_RESULT_V6_HOP_F_OFFSET_MOD8                         3
#define PARSER_RESULT_V6_HOP_F_OFFSET_MOD16                        3
#define PARSER_RESULT_HDR_LEN_ERR_F_OFFSET                         18
#define PARSER_RESULT_HDR_LEN_ERR_F_WIDTH                          1
#define PARSER_RESULT_HDR_LEN_ERR_OFFSET                           49
#define PARSER_RESULT_HDR_LEN_ERR_F_OFFSET_MOD8                    2
#define PARSER_RESULT_HDR_LEN_ERR_F_OFFSET_MOD16                   2
#define PARSER_RESULT_CHKSM_ERR_F_OFFSET                           17
#define PARSER_RESULT_CHKSM_ERR_F_WIDTH                            1
#define PARSER_RESULT_CHKSM_ERR_OFFSET                             49
#define PARSER_RESULT_CHKSM_ERR_F_OFFSET_MOD8                      1
#define PARSER_RESULT_CHKSM_ERR_F_OFFSET_MOD16                     1
#define PARSER_RESULT_VER_MISS_F_OFFSET                            16
#define PARSER_RESULT_VER_MISS_F_WIDTH                             1
#define PARSER_RESULT_VER_MISS_OFFSET                              49
#define PARSER_RESULT_VER_MISS_F_OFFSET_MOD8                       0
#define PARSER_RESULT_VER_MISS_F_OFFSET_MOD16                      0
#define PARSER_RESULT_LAYER3_CHECKSUM_F_OFFSET                     0
#define PARSER_RESULT_LAYER3_CHECKSUM_F_WIDTH                      16
#define PARSER_RESULT_LAYER3_CHECKSUM_OFFSET                       50
#define PARSER_RESULT_LAYER4_CALC_CHECKSUM_F_OFFSET                16
#define PARSER_RESULT_LAYER4_CALC_CHECKSUM_F_WIDTH                 16
#define PARSER_RESULT_LAYER4_CALC_CHECKSUM_OFFSET                  52
#define PARSER_RESULT_LAYER4_CHECKSUM_F_OFFSET                     0
#define PARSER_RESULT_LAYER4_CHECKSUM_F_WIDTH                      16
#define PARSER_RESULT_LAYER4_CHECKSUM_OFFSET                       54

/* >>>RDD_PARSER_RESULT_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	da_filter_match     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	first_ip_fragment   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_fragment         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_filter_match     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_filter_num       	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tcp_udp             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ipv6_ext_header     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tcp_flag            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	p_tag               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vid_filter_hit      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	exception           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	da_filter_number    	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	layer4_protocol     	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__; /*defined by rdd_parser_l4_protocol enumeration*/
	uint32_t	_5_tup_valid        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dhcp                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vlans_num           	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	broadcast           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	multicast           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	layer3_protocol     	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__; /*defined by rdd_parser_l3_protocol enumeration*/
	uint32_t	layer2_protocol     	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tag_type            	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_spoof           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tpid_vlan_0         	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tpid_vlan_1         	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tpid_vlan_2         	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	in_cfi              	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	out_cfi             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	no_outer            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	no_inner            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth_version         	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_mc_l2            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vid_filter_match    	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	layer2_address      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	outer_vlan          	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	da1_2               	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	da3_6               	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	inner_vlan          	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sa1_2               	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sa3_6               	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	_3rd_vlan           	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ethertype           	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_length           	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_ttl              	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tos                 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	layer4_offset       	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	layer3_offset       	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved3           	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mc_l3_ctl           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mc_l3               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	error               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	non_five_tuple      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_len_error        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	protocol            	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	source_port         	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	destination_port    	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	source_ip           	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	destination_ip      	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	flags               	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	icmpv6              	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	v6_ah               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	v6_dest_opt         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	v6_route            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	v6_hop              	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	hdr_len_err         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	chksm_err           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ver_miss            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	layer3_checksum     	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	layer4_calc_checksum	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	layer4_checksum     	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	layer2_protocol     	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	layer3_protocol     	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__; /*defined by rdd_parser_l3_protocol enumeration*/
	uint32_t	multicast           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	broadcast           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vlans_num           	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dhcp                	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	_5_tup_valid        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	layer4_protocol     	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__; /*defined by rdd_parser_l4_protocol enumeration*/
	uint32_t	da_filter_number    	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	exception           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vid_filter_hit      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	p_tag               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tcp_flag            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ipv6_ext_header     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tcp_udp             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_filter_num       	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_filter_match     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_fragment         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	first_ip_fragment   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	da_filter_match     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	layer2_address      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	vid_filter_match    	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_mc_l2            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	eth_version         	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	no_inner            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	no_outer            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	out_cfi             	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	in_cfi              	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tpid_vlan_2         	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tpid_vlan_1         	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tpid_vlan_0         	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mac_spoof           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tag_type            	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	da1_2               	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	outer_vlan          	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	da3_6               	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sa1_2               	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	inner_vlan          	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sa3_6               	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ethertype           	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	_3rd_vlan           	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	tos                 	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_ttl              	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_length           	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	protocol            	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ip_len_error        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	non_five_tuple      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	error               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mc_l3               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	mc_l3_ctl           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved3           	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	layer3_offset       	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	layer4_offset       	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	destination_port    	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	source_port         	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	source_ip           	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	destination_ip      	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	layer3_checksum     	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ver_miss            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	chksm_err           	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	hdr_len_err         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	v6_hop              	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	v6_route            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	v6_dest_opt         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	v6_ah               	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	icmpv6              	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	flags               	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	layer4_checksum     	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	layer4_calc_checksum	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PARSER_RESULT_DTS;

#define RDD_PARSER_RESULT_DA_FILTER_MATCH_READ_G(r, g, idx)               GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS), 7, 1, r)
#define RDD_PARSER_RESULT_DA_FILTER_MATCH_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS), 7, 1, v)
#define RDD_PARSER_RESULT_DA_FILTER_MATCH_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_PARSER_RESULT_DA_FILTER_MATCH_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_PARSER_RESULT_FIRST_IP_FRAGMENT_READ_G(r, g, idx)             GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS), 6, 1, r)
#define RDD_PARSER_RESULT_FIRST_IP_FRAGMENT_WRITE_G(v, g, idx)            GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS), 6, 1, v)
#define RDD_PARSER_RESULT_FIRST_IP_FRAGMENT_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p, 6, 1, r)
#define RDD_PARSER_RESULT_FIRST_IP_FRAGMENT_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p, 6, 1, v)
#define RDD_PARSER_RESULT_IP_FRAGMENT_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS), 5, 1, r)
#define RDD_PARSER_RESULT_IP_FRAGMENT_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS), 5, 1, v)
#define RDD_PARSER_RESULT_IP_FRAGMENT_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p, 5, 1, r)
#define RDD_PARSER_RESULT_IP_FRAGMENT_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p, 5, 1, v)
#define RDD_PARSER_RESULT_IP_FILTER_MATCH_READ_G(r, g, idx)               GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS), 4, 1, r)
#define RDD_PARSER_RESULT_IP_FILTER_MATCH_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS), 4, 1, v)
#define RDD_PARSER_RESULT_IP_FILTER_MATCH_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p, 4, 1, r)
#define RDD_PARSER_RESULT_IP_FILTER_MATCH_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p, 4, 1, v)
#define RDD_PARSER_RESULT_IP_FILTER_NUM_READ_G(r, g, idx)                 GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS), 2, 2, r)
#define RDD_PARSER_RESULT_IP_FILTER_NUM_WRITE_G(v, g, idx)                GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS), 2, 2, v)
#define RDD_PARSER_RESULT_IP_FILTER_NUM_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p, 2, 2, r)
#define RDD_PARSER_RESULT_IP_FILTER_NUM_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p, 2, 2, v)
#define RDD_PARSER_RESULT_TCP_UDP_READ_G(r, g, idx)                       GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS), 1, 1, r)
#define RDD_PARSER_RESULT_TCP_UDP_WRITE_G(v, g, idx)                      GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS), 1, 1, v)
#define RDD_PARSER_RESULT_TCP_UDP_READ(r, p)                              FIELD_MREAD_8((uint8_t *)p, 1, 1, r)
#define RDD_PARSER_RESULT_TCP_UDP_WRITE(v, p)                             FIELD_MWRITE_8((uint8_t *)p, 1, 1, v)
#define RDD_PARSER_RESULT_IPV6_EXT_HEADER_READ_G(r, g, idx)               GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS), 0, 1, r)
#define RDD_PARSER_RESULT_IPV6_EXT_HEADER_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS), 0, 1, v)
#define RDD_PARSER_RESULT_IPV6_EXT_HEADER_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p, 0, 1, r)
#define RDD_PARSER_RESULT_IPV6_EXT_HEADER_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p, 0, 1, v)
#define RDD_PARSER_RESULT_TCP_FLAG_READ_G(r, g, idx)                      GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 1, 7, 1, r)
#define RDD_PARSER_RESULT_TCP_FLAG_WRITE_G(v, g, idx)                     GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 1, 7, 1, v)
#define RDD_PARSER_RESULT_TCP_FLAG_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 1, 7, 1, r)
#define RDD_PARSER_RESULT_TCP_FLAG_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 1, 7, 1, v)
#define RDD_PARSER_RESULT_P_TAG_READ_G(r, g, idx)                         GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 1, 6, 1, r)
#define RDD_PARSER_RESULT_P_TAG_WRITE_G(v, g, idx)                        GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 1, 6, 1, v)
#define RDD_PARSER_RESULT_P_TAG_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 1, 6, 1, r)
#define RDD_PARSER_RESULT_P_TAG_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 1, 6, 1, v)
#define RDD_PARSER_RESULT_VID_FILTER_HIT_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 1, 5, 1, r)
#define RDD_PARSER_RESULT_VID_FILTER_HIT_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 1, 5, 1, v)
#define RDD_PARSER_RESULT_VID_FILTER_HIT_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 1, 5, 1, r)
#define RDD_PARSER_RESULT_VID_FILTER_HIT_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 1, 5, 1, v)
#define RDD_PARSER_RESULT_EXCEPTION_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 1, 4, 1, r)
#define RDD_PARSER_RESULT_EXCEPTION_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 1, 4, 1, v)
#define RDD_PARSER_RESULT_EXCEPTION_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 1, 4, 1, r)
#define RDD_PARSER_RESULT_EXCEPTION_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 1, 4, 1, v)
#define RDD_PARSER_RESULT_DA_FILTER_NUMBER_READ_G(r, g, idx)              GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 1, 0, 4, r)
#define RDD_PARSER_RESULT_DA_FILTER_NUMBER_WRITE_G(v, g, idx)             GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 1, 0, 4, v)
#define RDD_PARSER_RESULT_DA_FILTER_NUMBER_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 1, 0, 4, r)
#define RDD_PARSER_RESULT_DA_FILTER_NUMBER_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 1, 0, 4, v)
#define RDD_PARSER_RESULT_LAYER4_PROTOCOL_READ_G(r, g, idx)               GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 2, 4, 4, r) /*defined by rdd_parser_l4_protocol enumeration*/
#define RDD_PARSER_RESULT_LAYER4_PROTOCOL_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 2, 4, 4, v) /*defined by rdd_parser_l4_protocol enumeration*/
#define RDD_PARSER_RESULT_LAYER4_PROTOCOL_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 2, 4, 4, r) /*defined by rdd_parser_l4_protocol enumeration*/
#define RDD_PARSER_RESULT_LAYER4_PROTOCOL_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 2, 4, 4, v) /*defined by rdd_parser_l4_protocol enumeration*/
#define RDD_PARSER_RESULT__5_TUP_VALID_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 2, 3, 1, r)
#define RDD_PARSER_RESULT__5_TUP_VALID_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 2, 3, 1, v)
#define RDD_PARSER_RESULT__5_TUP_VALID_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 2, 3, 1, r)
#define RDD_PARSER_RESULT__5_TUP_VALID_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 2, 3, 1, v)
#define RDD_PARSER_RESULT_DHCP_READ_G(r, g, idx)                          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 2, 2, 1, r)
#define RDD_PARSER_RESULT_DHCP_WRITE_G(v, g, idx)                         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 2, 2, 1, v)
#define RDD_PARSER_RESULT_DHCP_READ(r, p)                                 FIELD_MREAD_8((uint8_t *)p + 2, 2, 1, r)
#define RDD_PARSER_RESULT_DHCP_WRITE(v, p)                                FIELD_MWRITE_8((uint8_t *)p + 2, 2, 1, v)
#define RDD_PARSER_RESULT_VLANS_NUM_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 2, 0, 2, r)
#define RDD_PARSER_RESULT_VLANS_NUM_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 2, 0, 2, v)
#define RDD_PARSER_RESULT_VLANS_NUM_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 2, 0, 2, r)
#define RDD_PARSER_RESULT_VLANS_NUM_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 2, 0, 2, v)
#define RDD_PARSER_RESULT_BROADCAST_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 3, 7, 1, r)
#define RDD_PARSER_RESULT_BROADCAST_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 3, 7, 1, v)
#define RDD_PARSER_RESULT_BROADCAST_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 3, 7, 1, r)
#define RDD_PARSER_RESULT_BROADCAST_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 3, 7, 1, v)
#define RDD_PARSER_RESULT_MULTICAST_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 3, 6, 1, r)
#define RDD_PARSER_RESULT_MULTICAST_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 3, 6, 1, v)
#define RDD_PARSER_RESULT_MULTICAST_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 3, 6, 1, r)
#define RDD_PARSER_RESULT_MULTICAST_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 3, 6, 1, v)
#define RDD_PARSER_RESULT_LAYER3_PROTOCOL_READ_G(r, g, idx)               GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 3, 4, 2, r) /*defined by rdd_parser_l3_protocol enumeration*/
#define RDD_PARSER_RESULT_LAYER3_PROTOCOL_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 3, 4, 2, v) /*defined by rdd_parser_l3_protocol enumeration*/
#define RDD_PARSER_RESULT_LAYER3_PROTOCOL_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 3, 4, 2, r) /*defined by rdd_parser_l3_protocol enumeration*/
#define RDD_PARSER_RESULT_LAYER3_PROTOCOL_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 3, 4, 2, v) /*defined by rdd_parser_l3_protocol enumeration*/
#define RDD_PARSER_RESULT_LAYER2_PROTOCOL_READ_G(r, g, idx)               GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 3, 0, 4, r)
#define RDD_PARSER_RESULT_LAYER2_PROTOCOL_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 3, 0, 4, v)
#define RDD_PARSER_RESULT_LAYER2_PROTOCOL_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 3, 0, 4, r)
#define RDD_PARSER_RESULT_LAYER2_PROTOCOL_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 3, 0, 4, v)
#define RDD_PARSER_RESULT_TAG_TYPE_READ_G(r, g, idx)                      GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 4, 6, 2, r)
#define RDD_PARSER_RESULT_TAG_TYPE_WRITE_G(v, g, idx)                     GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 4, 6, 2, v)
#define RDD_PARSER_RESULT_TAG_TYPE_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 4, 6, 2, r)
#define RDD_PARSER_RESULT_TAG_TYPE_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 4, 6, 2, v)
#define RDD_PARSER_RESULT_MAC_SPOOF_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 4, 5, 1, r)
#define RDD_PARSER_RESULT_MAC_SPOOF_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 4, 5, 1, v)
#define RDD_PARSER_RESULT_MAC_SPOOF_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 4, 5, 1, r)
#define RDD_PARSER_RESULT_MAC_SPOOF_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 4, 5, 1, v)
#define RDD_PARSER_RESULT_TPID_VLAN_0_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 4, 2, 3, r)
#define RDD_PARSER_RESULT_TPID_VLAN_0_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 4, 2, 3, v)
#define RDD_PARSER_RESULT_TPID_VLAN_0_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 4, 2, 3, r)
#define RDD_PARSER_RESULT_TPID_VLAN_0_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 4, 2, 3, v)
#define RDD_PARSER_RESULT_TPID_VLAN_1_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 4, 7, 3, r)
#define RDD_PARSER_RESULT_TPID_VLAN_1_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 4, 7, 3, v)
#define RDD_PARSER_RESULT_TPID_VLAN_1_READ(r, p)                          FIELD_MREAD_16((uint8_t *)p + 4, 7, 3, r)
#define RDD_PARSER_RESULT_TPID_VLAN_1_WRITE(v, p)                         FIELD_MWRITE_16((uint8_t *)p + 4, 7, 3, v)
#define RDD_PARSER_RESULT_TPID_VLAN_2_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 5, 4, 3, r)
#define RDD_PARSER_RESULT_TPID_VLAN_2_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 5, 4, 3, v)
#define RDD_PARSER_RESULT_TPID_VLAN_2_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 5, 4, 3, r)
#define RDD_PARSER_RESULT_TPID_VLAN_2_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 5, 4, 3, v)
#define RDD_PARSER_RESULT_IN_CFI_READ_G(r, g, idx)                        GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 5, 3, 1, r)
#define RDD_PARSER_RESULT_IN_CFI_WRITE_G(v, g, idx)                       GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 5, 3, 1, v)
#define RDD_PARSER_RESULT_IN_CFI_READ(r, p)                               FIELD_MREAD_8((uint8_t *)p + 5, 3, 1, r)
#define RDD_PARSER_RESULT_IN_CFI_WRITE(v, p)                              FIELD_MWRITE_8((uint8_t *)p + 5, 3, 1, v)
#define RDD_PARSER_RESULT_OUT_CFI_READ_G(r, g, idx)                       GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 5, 2, 1, r)
#define RDD_PARSER_RESULT_OUT_CFI_WRITE_G(v, g, idx)                      GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 5, 2, 1, v)
#define RDD_PARSER_RESULT_OUT_CFI_READ(r, p)                              FIELD_MREAD_8((uint8_t *)p + 5, 2, 1, r)
#define RDD_PARSER_RESULT_OUT_CFI_WRITE(v, p)                             FIELD_MWRITE_8((uint8_t *)p + 5, 2, 1, v)
#define RDD_PARSER_RESULT_NO_OUTER_READ_G(r, g, idx)                      GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 5, 1, 1, r)
#define RDD_PARSER_RESULT_NO_OUTER_WRITE_G(v, g, idx)                     GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 5, 1, 1, v)
#define RDD_PARSER_RESULT_NO_OUTER_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 5, 1, 1, r)
#define RDD_PARSER_RESULT_NO_OUTER_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 5, 1, 1, v)
#define RDD_PARSER_RESULT_NO_INNER_READ_G(r, g, idx)                      GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 5, 0, 1, r)
#define RDD_PARSER_RESULT_NO_INNER_WRITE_G(v, g, idx)                     GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 5, 0, 1, v)
#define RDD_PARSER_RESULT_NO_INNER_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 5, 0, 1, r)
#define RDD_PARSER_RESULT_NO_INNER_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 5, 0, 1, v)
#define RDD_PARSER_RESULT_ETH_VERSION_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 6, 6, 2, r)
#define RDD_PARSER_RESULT_ETH_VERSION_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 6, 6, 2, v)
#define RDD_PARSER_RESULT_ETH_VERSION_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 6, 6, 2, r)
#define RDD_PARSER_RESULT_ETH_VERSION_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 6, 6, 2, v)
#define RDD_PARSER_RESULT_IP_MC_L2_READ_G(r, g, idx)                      GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 6, 4, 1, r)
#define RDD_PARSER_RESULT_IP_MC_L2_WRITE_G(v, g, idx)                     GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 6, 4, 1, v)
#define RDD_PARSER_RESULT_IP_MC_L2_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 6, 4, 1, r)
#define RDD_PARSER_RESULT_IP_MC_L2_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 6, 4, 1, v)
#define RDD_PARSER_RESULT_VID_FILTER_MATCH_READ_G(r, g, idx)              GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 6, 0, 4, r)
#define RDD_PARSER_RESULT_VID_FILTER_MATCH_WRITE_G(v, g, idx)             GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 6, 0, 4, v)
#define RDD_PARSER_RESULT_VID_FILTER_MATCH_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 6, 0, 4, r)
#define RDD_PARSER_RESULT_VID_FILTER_MATCH_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 6, 0, 4, v)
#define RDD_PARSER_RESULT_LAYER2_ADDRESS_READ_G(r, g, idx)                GROUP_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 7, r)
#define RDD_PARSER_RESULT_LAYER2_ADDRESS_WRITE_G(v, g, idx)               GROUP_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 7, v)
#define RDD_PARSER_RESULT_LAYER2_ADDRESS_READ(r, p)                       MREAD_8((uint8_t *)p + 7, r)
#define RDD_PARSER_RESULT_LAYER2_ADDRESS_WRITE(v, p)                      MWRITE_8((uint8_t *)p + 7, v)
#define RDD_PARSER_RESULT_OUTER_VLAN_READ_G(r, g, idx)                    GROUP_MREAD_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 8, r)
#define RDD_PARSER_RESULT_OUTER_VLAN_WRITE_G(v, g, idx)                   GROUP_MWRITE_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 8, v)
#define RDD_PARSER_RESULT_OUTER_VLAN_READ(r, p)                           MREAD_16((uint8_t *)p + 8, r)
#define RDD_PARSER_RESULT_OUTER_VLAN_WRITE(v, p)                          MWRITE_16((uint8_t *)p + 8, v)
#define RDD_PARSER_RESULT_DA1_2_READ_G(r, g, idx)                         GROUP_MREAD_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 10, r)
#define RDD_PARSER_RESULT_DA1_2_WRITE_G(v, g, idx)                        GROUP_MWRITE_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 10, v)
#define RDD_PARSER_RESULT_DA1_2_READ(r, p)                                MREAD_16((uint8_t *)p + 10, r)
#define RDD_PARSER_RESULT_DA1_2_WRITE(v, p)                               MWRITE_16((uint8_t *)p + 10, v)
#define RDD_PARSER_RESULT_DA3_6_READ_G(r, g, idx)                         GROUP_MREAD_32(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 12, r)
#define RDD_PARSER_RESULT_DA3_6_WRITE_G(v, g, idx)                        GROUP_MWRITE_32(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 12, v)
#define RDD_PARSER_RESULT_DA3_6_READ(r, p)                                MREAD_32((uint8_t *)p + 12, r)
#define RDD_PARSER_RESULT_DA3_6_WRITE(v, p)                               MWRITE_32((uint8_t *)p + 12, v)
#define RDD_PARSER_RESULT_INNER_VLAN_READ_G(r, g, idx)                    GROUP_MREAD_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 16, r)
#define RDD_PARSER_RESULT_INNER_VLAN_WRITE_G(v, g, idx)                   GROUP_MWRITE_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 16, v)
#define RDD_PARSER_RESULT_INNER_VLAN_READ(r, p)                           MREAD_16((uint8_t *)p + 16, r)
#define RDD_PARSER_RESULT_INNER_VLAN_WRITE(v, p)                          MWRITE_16((uint8_t *)p + 16, v)
#define RDD_PARSER_RESULT_SA1_2_READ_G(r, g, idx)                         GROUP_MREAD_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 18, r)
#define RDD_PARSER_RESULT_SA1_2_WRITE_G(v, g, idx)                        GROUP_MWRITE_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 18, v)
#define RDD_PARSER_RESULT_SA1_2_READ(r, p)                                MREAD_16((uint8_t *)p + 18, r)
#define RDD_PARSER_RESULT_SA1_2_WRITE(v, p)                               MWRITE_16((uint8_t *)p + 18, v)
#define RDD_PARSER_RESULT_SA3_6_READ_G(r, g, idx)                         GROUP_MREAD_32(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 20, r)
#define RDD_PARSER_RESULT_SA3_6_WRITE_G(v, g, idx)                        GROUP_MWRITE_32(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 20, v)
#define RDD_PARSER_RESULT_SA3_6_READ(r, p)                                MREAD_32((uint8_t *)p + 20, r)
#define RDD_PARSER_RESULT_SA3_6_WRITE(v, p)                               MWRITE_32((uint8_t *)p + 20, v)
#define RDD_PARSER_RESULT__3RD_VLAN_READ_G(r, g, idx)                     GROUP_MREAD_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 24, r)
#define RDD_PARSER_RESULT__3RD_VLAN_WRITE_G(v, g, idx)                    GROUP_MWRITE_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 24, v)
#define RDD_PARSER_RESULT__3RD_VLAN_READ(r, p)                            MREAD_16((uint8_t *)p + 24, r)
#define RDD_PARSER_RESULT__3RD_VLAN_WRITE(v, p)                           MWRITE_16((uint8_t *)p + 24, v)
#define RDD_PARSER_RESULT_ETHERTYPE_READ_G(r, g, idx)                     GROUP_MREAD_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 26, r)
#define RDD_PARSER_RESULT_ETHERTYPE_WRITE_G(v, g, idx)                    GROUP_MWRITE_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 26, v)
#define RDD_PARSER_RESULT_ETHERTYPE_READ(r, p)                            MREAD_16((uint8_t *)p + 26, r)
#define RDD_PARSER_RESULT_ETHERTYPE_WRITE(v, p)                           MWRITE_16((uint8_t *)p + 26, v)
#define RDD_PARSER_RESULT_IP_LENGTH_READ_G(r, g, idx)                     GROUP_MREAD_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 28, r)
#define RDD_PARSER_RESULT_IP_LENGTH_WRITE_G(v, g, idx)                    GROUP_MWRITE_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 28, v)
#define RDD_PARSER_RESULT_IP_LENGTH_READ(r, p)                            MREAD_16((uint8_t *)p + 28, r)
#define RDD_PARSER_RESULT_IP_LENGTH_WRITE(v, p)                           MWRITE_16((uint8_t *)p + 28, v)
#define RDD_PARSER_RESULT_IP_TTL_READ_G(r, g, idx)                        GROUP_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 30, r)
#define RDD_PARSER_RESULT_IP_TTL_WRITE_G(v, g, idx)                       GROUP_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 30, v)
#define RDD_PARSER_RESULT_IP_TTL_READ(r, p)                               MREAD_8((uint8_t *)p + 30, r)
#define RDD_PARSER_RESULT_IP_TTL_WRITE(v, p)                              MWRITE_8((uint8_t *)p + 30, v)
#define RDD_PARSER_RESULT_TOS_READ_G(r, g, idx)                           GROUP_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 31, r)
#define RDD_PARSER_RESULT_TOS_WRITE_G(v, g, idx)                          GROUP_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 31, v)
#define RDD_PARSER_RESULT_TOS_READ(r, p)                                  MREAD_8((uint8_t *)p + 31, r)
#define RDD_PARSER_RESULT_TOS_WRITE(v, p)                                 MWRITE_8((uint8_t *)p + 31, v)
#define RDD_PARSER_RESULT_LAYER4_OFFSET_READ_G(r, g, idx)                 GROUP_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 32, r)
#define RDD_PARSER_RESULT_LAYER4_OFFSET_WRITE_G(v, g, idx)                GROUP_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 32, v)
#define RDD_PARSER_RESULT_LAYER4_OFFSET_READ(r, p)                        MREAD_8((uint8_t *)p + 32, r)
#define RDD_PARSER_RESULT_LAYER4_OFFSET_WRITE(v, p)                       MWRITE_8((uint8_t *)p + 32, v)
#define RDD_PARSER_RESULT_LAYER3_OFFSET_READ_G(r, g, idx)                 GROUP_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 33, r)
#define RDD_PARSER_RESULT_LAYER3_OFFSET_WRITE_G(v, g, idx)                GROUP_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 33, v)
#define RDD_PARSER_RESULT_LAYER3_OFFSET_READ(r, p)                        MREAD_8((uint8_t *)p + 33, r)
#define RDD_PARSER_RESULT_LAYER3_OFFSET_WRITE(v, p)                       MWRITE_8((uint8_t *)p + 33, v)
#define RDD_PARSER_RESULT_MC_L3_CTL_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 34, 4, 1, r)
#define RDD_PARSER_RESULT_MC_L3_CTL_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 34, 4, 1, v)
#define RDD_PARSER_RESULT_MC_L3_CTL_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 34, 4, 1, r)
#define RDD_PARSER_RESULT_MC_L3_CTL_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 34, 4, 1, v)
#define RDD_PARSER_RESULT_MC_L3_READ_G(r, g, idx)                         GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 34, 3, 1, r)
#define RDD_PARSER_RESULT_MC_L3_WRITE_G(v, g, idx)                        GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 34, 3, 1, v)
#define RDD_PARSER_RESULT_MC_L3_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 34, 3, 1, r)
#define RDD_PARSER_RESULT_MC_L3_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 34, 3, 1, v)
#define RDD_PARSER_RESULT_ERROR_READ_G(r, g, idx)                         GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 34, 2, 1, r)
#define RDD_PARSER_RESULT_ERROR_WRITE_G(v, g, idx)                        GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 34, 2, 1, v)
#define RDD_PARSER_RESULT_ERROR_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 34, 2, 1, r)
#define RDD_PARSER_RESULT_ERROR_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 34, 2, 1, v)
#define RDD_PARSER_RESULT_NON_FIVE_TUPLE_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 34, 1, 1, r)
#define RDD_PARSER_RESULT_NON_FIVE_TUPLE_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 34, 1, 1, v)
#define RDD_PARSER_RESULT_NON_FIVE_TUPLE_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p + 34, 1, 1, r)
#define RDD_PARSER_RESULT_NON_FIVE_TUPLE_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p + 34, 1, 1, v)
#define RDD_PARSER_RESULT_IP_LEN_ERROR_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 34, 0, 1, r)
#define RDD_PARSER_RESULT_IP_LEN_ERROR_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 34, 0, 1, v)
#define RDD_PARSER_RESULT_IP_LEN_ERROR_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 34, 0, 1, r)
#define RDD_PARSER_RESULT_IP_LEN_ERROR_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 34, 0, 1, v)
#define RDD_PARSER_RESULT_PROTOCOL_READ_G(r, g, idx)                      GROUP_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 35, r)
#define RDD_PARSER_RESULT_PROTOCOL_WRITE_G(v, g, idx)                     GROUP_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 35, v)
#define RDD_PARSER_RESULT_PROTOCOL_READ(r, p)                             MREAD_8((uint8_t *)p + 35, r)
#define RDD_PARSER_RESULT_PROTOCOL_WRITE(v, p)                            MWRITE_8((uint8_t *)p + 35, v)
#define RDD_PARSER_RESULT_SOURCE_PORT_READ_G(r, g, idx)                   GROUP_MREAD_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 36, r)
#define RDD_PARSER_RESULT_SOURCE_PORT_WRITE_G(v, g, idx)                  GROUP_MWRITE_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 36, v)
#define RDD_PARSER_RESULT_SOURCE_PORT_READ(r, p)                          MREAD_16((uint8_t *)p + 36, r)
#define RDD_PARSER_RESULT_SOURCE_PORT_WRITE(v, p)                         MWRITE_16((uint8_t *)p + 36, v)
#define RDD_PARSER_RESULT_DESTINATION_PORT_READ_G(r, g, idx)              GROUP_MREAD_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 38, r)
#define RDD_PARSER_RESULT_DESTINATION_PORT_WRITE_G(v, g, idx)             GROUP_MWRITE_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 38, v)
#define RDD_PARSER_RESULT_DESTINATION_PORT_READ(r, p)                     MREAD_16((uint8_t *)p + 38, r)
#define RDD_PARSER_RESULT_DESTINATION_PORT_WRITE(v, p)                    MWRITE_16((uint8_t *)p + 38, v)
#define RDD_PARSER_RESULT_SOURCE_IP_READ_G(r, g, idx)                     GROUP_MREAD_32(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 40, r)
#define RDD_PARSER_RESULT_SOURCE_IP_WRITE_G(v, g, idx)                    GROUP_MWRITE_32(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 40, v)
#define RDD_PARSER_RESULT_SOURCE_IP_READ(r, p)                            MREAD_32((uint8_t *)p + 40, r)
#define RDD_PARSER_RESULT_SOURCE_IP_WRITE(v, p)                           MWRITE_32((uint8_t *)p + 40, v)
#define RDD_PARSER_RESULT_DESTINATION_IP_READ_G(r, g, idx)                GROUP_MREAD_32(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 44, r)
#define RDD_PARSER_RESULT_DESTINATION_IP_WRITE_G(v, g, idx)               GROUP_MWRITE_32(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 44, v)
#define RDD_PARSER_RESULT_DESTINATION_IP_READ(r, p)                       MREAD_32((uint8_t *)p + 44, r)
#define RDD_PARSER_RESULT_DESTINATION_IP_WRITE(v, p)                      MWRITE_32((uint8_t *)p + 44, v)
#define RDD_PARSER_RESULT_FLAGS_READ_G(r, g, idx)                         GROUP_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 48, r)
#define RDD_PARSER_RESULT_FLAGS_WRITE_G(v, g, idx)                        GROUP_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 48, v)
#define RDD_PARSER_RESULT_FLAGS_READ(r, p)                                MREAD_8((uint8_t *)p + 48, r)
#define RDD_PARSER_RESULT_FLAGS_WRITE(v, p)                               MWRITE_8((uint8_t *)p + 48, v)
#define RDD_PARSER_RESULT_ICMPV6_READ_G(r, g, idx)                        GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 49, 7, 1, r)
#define RDD_PARSER_RESULT_ICMPV6_WRITE_G(v, g, idx)                       GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 49, 7, 1, v)
#define RDD_PARSER_RESULT_ICMPV6_READ(r, p)                               FIELD_MREAD_8((uint8_t *)p + 49, 7, 1, r)
#define RDD_PARSER_RESULT_ICMPV6_WRITE(v, p)                              FIELD_MWRITE_8((uint8_t *)p + 49, 7, 1, v)
#define RDD_PARSER_RESULT_V6_AH_READ_G(r, g, idx)                         GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 49, 6, 1, r)
#define RDD_PARSER_RESULT_V6_AH_WRITE_G(v, g, idx)                        GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 49, 6, 1, v)
#define RDD_PARSER_RESULT_V6_AH_READ(r, p)                                FIELD_MREAD_8((uint8_t *)p + 49, 6, 1, r)
#define RDD_PARSER_RESULT_V6_AH_WRITE(v, p)                               FIELD_MWRITE_8((uint8_t *)p + 49, 6, 1, v)
#define RDD_PARSER_RESULT_V6_DEST_OPT_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 49, 5, 1, r)
#define RDD_PARSER_RESULT_V6_DEST_OPT_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 49, 5, 1, v)
#define RDD_PARSER_RESULT_V6_DEST_OPT_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 49, 5, 1, r)
#define RDD_PARSER_RESULT_V6_DEST_OPT_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 49, 5, 1, v)
#define RDD_PARSER_RESULT_V6_ROUTE_READ_G(r, g, idx)                      GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 49, 4, 1, r)
#define RDD_PARSER_RESULT_V6_ROUTE_WRITE_G(v, g, idx)                     GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 49, 4, 1, v)
#define RDD_PARSER_RESULT_V6_ROUTE_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 49, 4, 1, r)
#define RDD_PARSER_RESULT_V6_ROUTE_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 49, 4, 1, v)
#define RDD_PARSER_RESULT_V6_HOP_READ_G(r, g, idx)                        GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 49, 3, 1, r)
#define RDD_PARSER_RESULT_V6_HOP_WRITE_G(v, g, idx)                       GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 49, 3, 1, v)
#define RDD_PARSER_RESULT_V6_HOP_READ(r, p)                               FIELD_MREAD_8((uint8_t *)p + 49, 3, 1, r)
#define RDD_PARSER_RESULT_V6_HOP_WRITE(v, p)                              FIELD_MWRITE_8((uint8_t *)p + 49, 3, 1, v)
#define RDD_PARSER_RESULT_HDR_LEN_ERR_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 49, 2, 1, r)
#define RDD_PARSER_RESULT_HDR_LEN_ERR_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 49, 2, 1, v)
#define RDD_PARSER_RESULT_HDR_LEN_ERR_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 49, 2, 1, r)
#define RDD_PARSER_RESULT_HDR_LEN_ERR_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 49, 2, 1, v)
#define RDD_PARSER_RESULT_CHKSM_ERR_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 49, 1, 1, r)
#define RDD_PARSER_RESULT_CHKSM_ERR_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 49, 1, 1, v)
#define RDD_PARSER_RESULT_CHKSM_ERR_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p + 49, 1, 1, r)
#define RDD_PARSER_RESULT_CHKSM_ERR_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 49, 1, 1, v)
#define RDD_PARSER_RESULT_VER_MISS_READ_G(r, g, idx)                      GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 49, 0, 1, r)
#define RDD_PARSER_RESULT_VER_MISS_WRITE_G(v, g, idx)                     GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 49, 0, 1, v)
#define RDD_PARSER_RESULT_VER_MISS_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 49, 0, 1, r)
#define RDD_PARSER_RESULT_VER_MISS_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 49, 0, 1, v)
#define RDD_PARSER_RESULT_LAYER3_CHECKSUM_READ_G(r, g, idx)               GROUP_MREAD_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 50, r)
#define RDD_PARSER_RESULT_LAYER3_CHECKSUM_WRITE_G(v, g, idx)              GROUP_MWRITE_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 50, v)
#define RDD_PARSER_RESULT_LAYER3_CHECKSUM_READ(r, p)                      MREAD_16((uint8_t *)p + 50, r)
#define RDD_PARSER_RESULT_LAYER3_CHECKSUM_WRITE(v, p)                     MWRITE_16((uint8_t *)p + 50, v)
#define RDD_PARSER_RESULT_LAYER4_CALC_CHECKSUM_READ_G(r, g, idx)          GROUP_MREAD_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 52, r)
#define RDD_PARSER_RESULT_LAYER4_CALC_CHECKSUM_WRITE_G(v, g, idx)         GROUP_MWRITE_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 52, v)
#define RDD_PARSER_RESULT_LAYER4_CALC_CHECKSUM_READ(r, p)                 MREAD_16((uint8_t *)p + 52, r)
#define RDD_PARSER_RESULT_LAYER4_CALC_CHECKSUM_WRITE(v, p)                MWRITE_16((uint8_t *)p + 52, v)
#define RDD_PARSER_RESULT_LAYER4_CHECKSUM_READ_G(r, g, idx)               GROUP_MREAD_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 54, r)
#define RDD_PARSER_RESULT_LAYER4_CHECKSUM_WRITE_G(v, g, idx)              GROUP_MWRITE_16(g, idx*sizeof(RDD_PARSER_RESULT_DTS) + 54, v)
#define RDD_PARSER_RESULT_LAYER4_CHECKSUM_READ(r, p)                      MREAD_16((uint8_t *)p + 54, r)
#define RDD_PARSER_RESULT_LAYER4_CHECKSUM_WRITE(v, p)                     MWRITE_16((uint8_t *)p + 54, v)
/* <<<RDD_PARSER_RESULT_DTS */

/* <<<RDD_PARSER_RESULT */


/* >>>RDD_NATC_CONTROL_ENTRY */
#define NATC_CONTROL_ENTRY_DONE_F_OFFSET                                31
#define NATC_CONTROL_ENTRY_DONE_F_WIDTH                                 1
#define NATC_CONTROL_ENTRY_DONE_OFFSET                                  0
#define NATC_CONTROL_ENTRY_DONE_F_OFFSET_MOD8                           7
#define NATC_CONTROL_ENTRY_DONE_F_OFFSET_MOD16                          15
#define NATC_CONTROL_ENTRY_NATC_HIT_F_OFFSET                            30
#define NATC_CONTROL_ENTRY_NATC_HIT_F_WIDTH                             1
#define NATC_CONTROL_ENTRY_NATC_HIT_OFFSET                              0
#define NATC_CONTROL_ENTRY_NATC_HIT_F_OFFSET_MOD8                       6
#define NATC_CONTROL_ENTRY_NATC_HIT_F_OFFSET_MOD16                      14
#define NATC_CONTROL_ENTRY_CACHE_HIT_F_OFFSET                           29
#define NATC_CONTROL_ENTRY_CACHE_HIT_F_WIDTH                            1
#define NATC_CONTROL_ENTRY_CACHE_HIT_OFFSET                             0
#define NATC_CONTROL_ENTRY_CACHE_HIT_F_OFFSET_MOD8                      5
#define NATC_CONTROL_ENTRY_CACHE_HIT_F_OFFSET_MOD16                     13
#define NATC_CONTROL_ENTRY_HW_RESERVED0_F_OFFSET                        24
#define NATC_CONTROL_ENTRY_HW_RESERVED0_F_WIDTH                         5
#define NATC_CONTROL_ENTRY_HW_RESERVED0_OFFSET                          0
#define NATC_CONTROL_ENTRY_HW_RESERVED0_F_OFFSET_MOD8                   0
#define NATC_CONTROL_ENTRY_HW_RESERVED0_F_OFFSET_MOD16                  8
#define NATC_CONTROL_ENTRY_HAS_ITER_F_OFFSET                            20
#define NATC_CONTROL_ENTRY_HAS_ITER_F_WIDTH                             4
#define NATC_CONTROL_ENTRY_HAS_ITER_OFFSET                              1
#define NATC_CONTROL_ENTRY_HAS_ITER_F_OFFSET_MOD8                       4
#define NATC_CONTROL_ENTRY_HAS_ITER_F_OFFSET_MOD16                      4
#define NATC_CONTROL_ENTRY_HW_RESERVED1_F_OFFSET                        18
#define NATC_CONTROL_ENTRY_HW_RESERVED1_F_WIDTH                         2
#define NATC_CONTROL_ENTRY_HW_RESERVED1_OFFSET                          1
#define NATC_CONTROL_ENTRY_HW_RESERVED1_F_OFFSET_MOD8                   2
#define NATC_CONTROL_ENTRY_HW_RESERVED1_F_OFFSET_MOD16                  2
#define NATC_CONTROL_ENTRY_HASH_VAL_F_OFFSET                            0
#define NATC_CONTROL_ENTRY_HASH_VAL_F_WIDTH                             18
#define NATC_CONTROL_ENTRY_HASH_VAL_OFFSET                              0
#define NATC_CONTROL_ENTRY_HW_RESERVED2_F_OFFSET                        0
#define NATC_CONTROL_ENTRY_HW_RESERVED2_F_WIDTH                         32
#define NATC_CONTROL_ENTRY_HW_RESERVED2_OFFSET                          4

/* >>>RDD_NATC_CONTROL_ENTRY_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	done        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	natc_hit    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cache_hit   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	hw_reserved0	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	has_iter    	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	hw_reserved1	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	hash_val    	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	hw_reserved2	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	hash_val    	:18	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	hw_reserved1	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	has_iter    	:4	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	hw_reserved0	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	cache_hit   	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	natc_hit    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	done        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	hw_reserved2	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_NATC_CONTROL_ENTRY_DTS;

#define RDD_NATC_CONTROL_ENTRY_DONE_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_NATC_CONTROL_ENTRY_DTS), 7, 1, r)
#define RDD_NATC_CONTROL_ENTRY_DONE_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_NATC_CONTROL_ENTRY_DTS), 7, 1, v)
#define RDD_NATC_CONTROL_ENTRY_DONE_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_NATC_CONTROL_ENTRY_DONE_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_NATC_CONTROL_ENTRY_NATC_HIT_READ_G(r, g, idx)              GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_NATC_CONTROL_ENTRY_DTS), 6, 1, r)
#define RDD_NATC_CONTROL_ENTRY_NATC_HIT_WRITE_G(v, g, idx)             GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_NATC_CONTROL_ENTRY_DTS), 6, 1, v)
#define RDD_NATC_CONTROL_ENTRY_NATC_HIT_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p, 6, 1, r)
#define RDD_NATC_CONTROL_ENTRY_NATC_HIT_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p, 6, 1, v)
#define RDD_NATC_CONTROL_ENTRY_CACHE_HIT_READ_G(r, g, idx)             GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_NATC_CONTROL_ENTRY_DTS), 5, 1, r)
#define RDD_NATC_CONTROL_ENTRY_CACHE_HIT_WRITE_G(v, g, idx)            GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_NATC_CONTROL_ENTRY_DTS), 5, 1, v)
#define RDD_NATC_CONTROL_ENTRY_CACHE_HIT_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p, 5, 1, r)
#define RDD_NATC_CONTROL_ENTRY_CACHE_HIT_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p, 5, 1, v)
#define RDD_NATC_CONTROL_ENTRY_HW_RESERVED0_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_NATC_CONTROL_ENTRY_DTS), 0, 5, r)
#define RDD_NATC_CONTROL_ENTRY_HW_RESERVED0_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_NATC_CONTROL_ENTRY_DTS), 0, 5, v)
#define RDD_NATC_CONTROL_ENTRY_HW_RESERVED0_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 0, 5, r)
#define RDD_NATC_CONTROL_ENTRY_HW_RESERVED0_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 0, 5, v)
#define RDD_NATC_CONTROL_ENTRY_HAS_ITER_READ_G(r, g, idx)              GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_NATC_CONTROL_ENTRY_DTS) + 1, 4, 4, r)
#define RDD_NATC_CONTROL_ENTRY_HAS_ITER_WRITE_G(v, g, idx)             GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_NATC_CONTROL_ENTRY_DTS) + 1, 4, 4, v)
#define RDD_NATC_CONTROL_ENTRY_HAS_ITER_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 1, 4, 4, r)
#define RDD_NATC_CONTROL_ENTRY_HAS_ITER_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 1, 4, 4, v)
#define RDD_NATC_CONTROL_ENTRY_HW_RESERVED1_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_NATC_CONTROL_ENTRY_DTS) + 1, 2, 2, r)
#define RDD_NATC_CONTROL_ENTRY_HW_RESERVED1_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_NATC_CONTROL_ENTRY_DTS) + 1, 2, 2, v)
#define RDD_NATC_CONTROL_ENTRY_HW_RESERVED1_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 1, 2, 2, r)
#define RDD_NATC_CONTROL_ENTRY_HW_RESERVED1_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 1, 2, 2, v)
#define RDD_NATC_CONTROL_ENTRY_HASH_VAL_READ_G(r, g, idx)              GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_NATC_CONTROL_ENTRY_DTS) + 0, 0, 18, r)
#define RDD_NATC_CONTROL_ENTRY_HASH_VAL_WRITE_G(v, g, idx)             GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_NATC_CONTROL_ENTRY_DTS) + 0, 0, 18, v)
#define RDD_NATC_CONTROL_ENTRY_HASH_VAL_READ(r, p)                     FIELD_MREAD_32((uint8_t *)p + 0, 0, 18, r)
#define RDD_NATC_CONTROL_ENTRY_HASH_VAL_WRITE(v, p)                    FIELD_MWRITE_32((uint8_t *)p + 0, 0, 18, v)
#define RDD_NATC_CONTROL_ENTRY_HW_RESERVED2_READ_G(r, g, idx)          GROUP_MREAD_32(g, idx*sizeof(RDD_NATC_CONTROL_ENTRY_DTS) + 4, r)
#define RDD_NATC_CONTROL_ENTRY_HW_RESERVED2_WRITE_G(v, g, idx)         GROUP_MWRITE_32(g, idx*sizeof(RDD_NATC_CONTROL_ENTRY_DTS) + 4, v)
#define RDD_NATC_CONTROL_ENTRY_HW_RESERVED2_READ(r, p)                 MREAD_32((uint8_t *)p + 4, r)
#define RDD_NATC_CONTROL_ENTRY_HW_RESERVED2_WRITE(v, p)                MWRITE_32((uint8_t *)p + 4, v)
/* <<<RDD_NATC_CONTROL_ENTRY_DTS */

/* <<<RDD_NATC_CONTROL_ENTRY */


/* >>>RDD_NATC_COUNTERS_ENTRY */
#define NATC_COUNTERS_ENTRY_HIT_COUNTER_F_OFFSET                         0
#define NATC_COUNTERS_ENTRY_HIT_COUNTER_F_WIDTH                          32
#define NATC_COUNTERS_ENTRY_HIT_COUNTER_OFFSET                           0
#define NATC_COUNTERS_ENTRY_BYTES_COUNTER_F_OFFSET                       0
#define NATC_COUNTERS_ENTRY_BYTES_COUNTER_F_WIDTH                        32
#define NATC_COUNTERS_ENTRY_BYTES_COUNTER_OFFSET                         4

/* >>>RDD_NATC_COUNTERS_ENTRY_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	hit_counter  	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bytes_counter	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	hit_counter  	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bytes_counter	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_NATC_COUNTERS_ENTRY_DTS;

#define RDD_NATC_COUNTERS_ENTRY_HIT_COUNTER_READ_G(r, g, idx)            GROUP_MREAD_32(g, idx*sizeof(RDD_NATC_COUNTERS_ENTRY_DTS), r)
#define RDD_NATC_COUNTERS_ENTRY_HIT_COUNTER_WRITE_G(v, g, idx)           GROUP_MWRITE_32(g, idx*sizeof(RDD_NATC_COUNTERS_ENTRY_DTS), v)
#define RDD_NATC_COUNTERS_ENTRY_HIT_COUNTER_READ(r, p)                   MREAD_32((uint8_t *)p, r)
#define RDD_NATC_COUNTERS_ENTRY_HIT_COUNTER_WRITE(v, p)                  MWRITE_32((uint8_t *)p, v)
#define RDD_NATC_COUNTERS_ENTRY_BYTES_COUNTER_READ_G(r, g, idx)          GROUP_MREAD_32(g, idx*sizeof(RDD_NATC_COUNTERS_ENTRY_DTS) + 4, r)
#define RDD_NATC_COUNTERS_ENTRY_BYTES_COUNTER_WRITE_G(v, g, idx)         GROUP_MWRITE_32(g, idx*sizeof(RDD_NATC_COUNTERS_ENTRY_DTS) + 4, v)
#define RDD_NATC_COUNTERS_ENTRY_BYTES_COUNTER_READ(r, p)                 MREAD_32((uint8_t *)p + 4, r)
#define RDD_NATC_COUNTERS_ENTRY_BYTES_COUNTER_WRITE(v, p)                MWRITE_32((uint8_t *)p + 4, v)
/* <<<RDD_NATC_COUNTERS_ENTRY_DTS */

/* <<<RDD_NATC_COUNTERS_ENTRY */


/* >>>RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE */
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_FPM_POOL0_F_OFFSET                           0
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_FPM_POOL0_F_WIDTH                            2
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_FPM_POOL0_OFFSET                             3
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_FPM_POOL1_F_OFFSET                           30
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_FPM_POOL1_F_WIDTH                            2
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_FPM_POOL1_OFFSET                             4
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_FPM_POOL1_F_OFFSET_MOD8                      6
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_FPM_POOL1_F_OFFSET_MOD16                     14
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_BYTES_POPPED_F_OFFSET                        16
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_BYTES_POPPED_F_WIDTH                         14
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_BYTES_POPPED_OFFSET                          4
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_BYTES_POPPED_F_OFFSET_MOD16                  0
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_AGG_F_OFFSET                                 15
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_AGG_F_WIDTH                                  1
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_AGG_OFFSET                                   6
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_AGG_F_OFFSET_MOD8                            7
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_TARGET_MEM0_F_OFFSET                         14
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_TARGET_MEM0_F_WIDTH                          1
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_TARGET_MEM0_OFFSET                           6
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_TARGET_MEM0_F_OFFSET_MOD8                    6
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_ABS_F_OFFSET                                 13
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_ABS_F_WIDTH                                  1
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_ABS_OFFSET                                   6
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_ABS_F_OFFSET_MOD8                            5
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_TARGET_MEM1_F_OFFSET                         12
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_TARGET_MEM1_F_WIDTH                          1
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_TARGET_MEM1_OFFSET                           6
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_TARGET_MEM1_F_OFFSET_MOD8                    4
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_QUEUE_NUMBER_F_OFFSET                        0
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_QUEUE_NUMBER_F_WIDTH                         9
#define BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_QUEUE_NUMBER_OFFSET                          6

/* >>>RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	reserved0   	:30	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	fpm_pool0   	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	fpm_pool1   	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bytes_popped	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	agg         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	target_mem0 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	target_mem1 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1   	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	queue_number	:9	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	fpm_pool0   	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved0   	:30	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	queue_number	:9	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved1   	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	target_mem1 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	abs         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	target_mem0 	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	agg         	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bytes_popped	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	fpm_pool1   	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_DTS;

#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_FPM_POOL0_READ_G(r, g, idx)             GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_DTS) + 3, 0, 2, r)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_FPM_POOL0_WRITE_G(v, g, idx)            GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_DTS) + 3, 0, 2, v)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_FPM_POOL0_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 3, 0, 2, r)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_FPM_POOL0_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 3, 0, 2, v)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_FPM_POOL1_READ_G(r, g, idx)             GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_DTS) + 4, 6, 2, r)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_FPM_POOL1_WRITE_G(v, g, idx)            GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_DTS) + 4, 6, 2, v)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_FPM_POOL1_READ(r, p)                    FIELD_MREAD_8((uint8_t *)p + 4, 6, 2, r)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_FPM_POOL1_WRITE(v, p)                   FIELD_MWRITE_8((uint8_t *)p + 4, 6, 2, v)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_BYTES_POPPED_READ_G(r, g, idx)          GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_DTS) + 4, 0, 14, r)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_BYTES_POPPED_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_DTS) + 4, 0, 14, v)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_BYTES_POPPED_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p + 4, 0, 14, r)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_BYTES_POPPED_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p + 4, 0, 14, v)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_AGG_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_DTS) + 6, 7, 1, r)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_AGG_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_DTS) + 6, 7, 1, v)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_AGG_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 6, 7, 1, r)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_AGG_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 6, 7, 1, v)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_TARGET_MEM0_READ_G(r, g, idx)           GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_DTS) + 6, 6, 1, r)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_TARGET_MEM0_WRITE_G(v, g, idx)          GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_DTS) + 6, 6, 1, v)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_TARGET_MEM0_READ(r, p)                  FIELD_MREAD_8((uint8_t *)p + 6, 6, 1, r)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_TARGET_MEM0_WRITE(v, p)                 FIELD_MWRITE_8((uint8_t *)p + 6, 6, 1, v)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_ABS_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_DTS) + 6, 5, 1, r)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_ABS_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_DTS) + 6, 5, 1, v)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_ABS_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p + 6, 5, 1, r)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_ABS_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p + 6, 5, 1, v)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_TARGET_MEM1_READ_G(r, g, idx)           GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_DTS) + 6, 4, 1, r)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_TARGET_MEM1_WRITE_G(v, g, idx)          GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_DTS) + 6, 4, 1, v)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_TARGET_MEM1_READ(r, p)                  FIELD_MREAD_8((uint8_t *)p + 6, 4, 1, r)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_TARGET_MEM1_WRITE(v, p)                 FIELD_MWRITE_8((uint8_t *)p + 6, 4, 1, v)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_QUEUE_NUMBER_READ_G(r, g, idx)          GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_DTS) + 6, 0, 9, r)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_QUEUE_NUMBER_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_DTS) + 6, 0, 9, v)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_QUEUE_NUMBER_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p + 6, 0, 9, r)
#define RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_QUEUE_NUMBER_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p + 6, 0, 9, v)
/* <<<RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE_DTS */

/* <<<RDD_BBMSG_RNR_TO_QM_PD_FIFO_CREDIT_FREE */


/* >>>RDD_HASH_RESULT */
#define HASH_RESULT_MATCH_F_OFFSET                               30
#define HASH_RESULT_MATCH_F_WIDTH                                2
#define HASH_RESULT_MATCH_OFFSET                                 0
#define HASH_RESULT_MATCH_F_OFFSET_MOD8                          6
#define HASH_RESULT_MATCH_F_OFFSET_MOD16                         14
#define HASH_RESULT_MATCH_INDEX_F_OFFSET                         19
#define HASH_RESULT_MATCH_INDEX_F_WIDTH                          11
#define HASH_RESULT_MATCH_INDEX_OFFSET                           0
#define HASH_RESULT_MATCH_INDEX_F_OFFSET_MOD16                   3
#define HASH_RESULT_MATCH_ENGINE_F_OFFSET                        17
#define HASH_RESULT_MATCH_ENGINE_F_WIDTH                         2
#define HASH_RESULT_MATCH_ENGINE_OFFSET                          1
#define HASH_RESULT_MATCH_ENGINE_F_OFFSET_MOD8                   1
#define HASH_RESULT_MATCH_ENGINE_F_OFFSET_MOD16                  1
#define HASH_RESULT_CONTEXT0_32_47_F_OFFSET                      0
#define HASH_RESULT_CONTEXT0_32_47_F_WIDTH                       16
#define HASH_RESULT_CONTEXT0_32_47_OFFSET                        2
#define HASH_RESULT_CONTEXT0_0_31_F_OFFSET                       0
#define HASH_RESULT_CONTEXT0_0_31_F_WIDTH                        32
#define HASH_RESULT_CONTEXT0_0_31_OFFSET                         4
#define HASH_RESULT_CONTEXT1_16_47_F_OFFSET                      0
#define HASH_RESULT_CONTEXT1_16_47_F_WIDTH                       32
#define HASH_RESULT_CONTEXT1_16_47_OFFSET                        8
#define HASH_RESULT_CONTEXT1_0_15_F_OFFSET                       16
#define HASH_RESULT_CONTEXT1_0_15_F_WIDTH                        16
#define HASH_RESULT_CONTEXT1_0_15_OFFSET                         12
#define HASH_RESULT_CONTEXT2_32_47_F_OFFSET                      0
#define HASH_RESULT_CONTEXT2_32_47_F_WIDTH                       16
#define HASH_RESULT_CONTEXT2_32_47_OFFSET                        14
#define HASH_RESULT_CONTEXT2_0_31_F_OFFSET                       0
#define HASH_RESULT_CONTEXT2_0_31_F_WIDTH                        32
#define HASH_RESULT_CONTEXT2_0_31_OFFSET                         16

/* >>>RDD_HASH_RESULT_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	match         	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	match_index   	:11	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	match_engine  	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_0    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	context0_32_47	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	context0_0_31 	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	context1_16_47	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	context1_0_15 	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	context2_32_47	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	context2_0_31 	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_1    	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	context0_32_47	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_0    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	match_engine  	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	match_index   	:11	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	match         	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	context0_0_31 	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	context1_16_47	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	context2_32_47	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	context1_0_15 	:16	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	context2_0_31 	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_1    	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_HASH_RESULT_DTS;

#define RDD_HASH_RESULT_MATCH_READ_G(r, g, idx)                   GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_HASH_RESULT_DTS), 6, 2, r)
#define RDD_HASH_RESULT_MATCH_WRITE_G(v, g, idx)                  GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_HASH_RESULT_DTS), 6, 2, v)
#define RDD_HASH_RESULT_MATCH_READ(r, p)                          FIELD_MREAD_8((uint8_t *)p, 6, 2, r)
#define RDD_HASH_RESULT_MATCH_WRITE(v, p)                         FIELD_MWRITE_8((uint8_t *)p, 6, 2, v)
#define RDD_HASH_RESULT_MATCH_INDEX_READ_G(r, g, idx)             GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_HASH_RESULT_DTS), 3, 11, r)
#define RDD_HASH_RESULT_MATCH_INDEX_WRITE_G(v, g, idx)            GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_HASH_RESULT_DTS), 3, 11, v)
#define RDD_HASH_RESULT_MATCH_INDEX_READ(r, p)                    FIELD_MREAD_16((uint8_t *)p, 3, 11, r)
#define RDD_HASH_RESULT_MATCH_INDEX_WRITE(v, p)                   FIELD_MWRITE_16((uint8_t *)p, 3, 11, v)
#define RDD_HASH_RESULT_MATCH_ENGINE_READ_G(r, g, idx)            GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_HASH_RESULT_DTS) + 1, 1, 2, r)
#define RDD_HASH_RESULT_MATCH_ENGINE_WRITE_G(v, g, idx)           GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_HASH_RESULT_DTS) + 1, 1, 2, v)
#define RDD_HASH_RESULT_MATCH_ENGINE_READ(r, p)                   FIELD_MREAD_8((uint8_t *)p + 1, 1, 2, r)
#define RDD_HASH_RESULT_MATCH_ENGINE_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p + 1, 1, 2, v)
#define RDD_HASH_RESULT_CONTEXT0_32_47_READ_G(r, g, idx)          GROUP_MREAD_16(g, idx*sizeof(RDD_HASH_RESULT_DTS) + 2, r)
#define RDD_HASH_RESULT_CONTEXT0_32_47_WRITE_G(v, g, idx)         GROUP_MWRITE_16(g, idx*sizeof(RDD_HASH_RESULT_DTS) + 2, v)
#define RDD_HASH_RESULT_CONTEXT0_32_47_READ(r, p)                 MREAD_16((uint8_t *)p + 2, r)
#define RDD_HASH_RESULT_CONTEXT0_32_47_WRITE(v, p)                MWRITE_16((uint8_t *)p + 2, v)
#define RDD_HASH_RESULT_CONTEXT0_0_31_READ_G(r, g, idx)           GROUP_MREAD_32(g, idx*sizeof(RDD_HASH_RESULT_DTS) + 4, r)
#define RDD_HASH_RESULT_CONTEXT0_0_31_WRITE_G(v, g, idx)          GROUP_MWRITE_32(g, idx*sizeof(RDD_HASH_RESULT_DTS) + 4, v)
#define RDD_HASH_RESULT_CONTEXT0_0_31_READ(r, p)                  MREAD_32((uint8_t *)p + 4, r)
#define RDD_HASH_RESULT_CONTEXT0_0_31_WRITE(v, p)                 MWRITE_32((uint8_t *)p + 4, v)
#define RDD_HASH_RESULT_CONTEXT1_16_47_READ_G(r, g, idx)          GROUP_MREAD_32(g, idx*sizeof(RDD_HASH_RESULT_DTS) + 8, r)
#define RDD_HASH_RESULT_CONTEXT1_16_47_WRITE_G(v, g, idx)         GROUP_MWRITE_32(g, idx*sizeof(RDD_HASH_RESULT_DTS) + 8, v)
#define RDD_HASH_RESULT_CONTEXT1_16_47_READ(r, p)                 MREAD_32((uint8_t *)p + 8, r)
#define RDD_HASH_RESULT_CONTEXT1_16_47_WRITE(v, p)                MWRITE_32((uint8_t *)p + 8, v)
#define RDD_HASH_RESULT_CONTEXT1_0_15_READ_G(r, g, idx)           GROUP_MREAD_16(g, idx*sizeof(RDD_HASH_RESULT_DTS) + 12, r)
#define RDD_HASH_RESULT_CONTEXT1_0_15_WRITE_G(v, g, idx)          GROUP_MWRITE_16(g, idx*sizeof(RDD_HASH_RESULT_DTS) + 12, v)
#define RDD_HASH_RESULT_CONTEXT1_0_15_READ(r, p)                  MREAD_16((uint8_t *)p + 12, r)
#define RDD_HASH_RESULT_CONTEXT1_0_15_WRITE(v, p)                 MWRITE_16((uint8_t *)p + 12, v)
#define RDD_HASH_RESULT_CONTEXT2_32_47_READ_G(r, g, idx)          GROUP_MREAD_16(g, idx*sizeof(RDD_HASH_RESULT_DTS) + 14, r)
#define RDD_HASH_RESULT_CONTEXT2_32_47_WRITE_G(v, g, idx)         GROUP_MWRITE_16(g, idx*sizeof(RDD_HASH_RESULT_DTS) + 14, v)
#define RDD_HASH_RESULT_CONTEXT2_32_47_READ(r, p)                 MREAD_16((uint8_t *)p + 14, r)
#define RDD_HASH_RESULT_CONTEXT2_32_47_WRITE(v, p)                MWRITE_16((uint8_t *)p + 14, v)
#define RDD_HASH_RESULT_CONTEXT2_0_31_READ_G(r, g, idx)           GROUP_MREAD_32(g, idx*sizeof(RDD_HASH_RESULT_DTS) + 16, r)
#define RDD_HASH_RESULT_CONTEXT2_0_31_WRITE_G(v, g, idx)          GROUP_MWRITE_32(g, idx*sizeof(RDD_HASH_RESULT_DTS) + 16, v)
#define RDD_HASH_RESULT_CONTEXT2_0_31_READ(r, p)                  MREAD_32((uint8_t *)p + 16, r)
#define RDD_HASH_RESULT_CONTEXT2_0_31_WRITE(v, p)                 MWRITE_32((uint8_t *)p + 16, v)
/* <<<RDD_HASH_RESULT_DTS */

/* <<<RDD_HASH_RESULT */


/* >>>RDD_TRACE_EVENT */
#define TRACE_EVENT_TIMESTAMP_F_OFFSET                           20
#define TRACE_EVENT_TIMESTAMP_F_WIDTH                            12
#define TRACE_EVENT_TIMESTAMP_OFFSET                             0
#define TRACE_EVENT_TIMESTAMP_F_OFFSET_MOD16                     4
#define TRACE_EVENT_EVENT_ID_F_OFFSET                            18
#define TRACE_EVENT_EVENT_ID_F_WIDTH                             2
#define TRACE_EVENT_EVENT_ID_OFFSET                              1
#define TRACE_EVENT_EVENT_ID_F_OFFSET_MOD8                       2
#define TRACE_EVENT_EVENT_ID_F_OFFSET_MOD16                      2
#define TRACE_EVENT_TRACE_EVENT_INFO_F_OFFSET                    0
#define TRACE_EVENT_TRACE_EVENT_INFO_F_WIDTH                     18
#define TRACE_EVENT_TRACE_EVENT_INFO_OFFSET                      0
/* <<<RDD_TRACE_EVENT */


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


/* >>>RDD_CPU_RING_DESCRIPTOR */
#define CPU_RING_DESCRIPTOR_SIZE_OF_ENTRY_F_OFFSET                       27
#define CPU_RING_DESCRIPTOR_SIZE_OF_ENTRY_F_WIDTH                        5
#define CPU_RING_DESCRIPTOR_SIZE_OF_ENTRY_OFFSET                         0
#define CPU_RING_DESCRIPTOR_SIZE_OF_ENTRY_F_OFFSET_MOD8                  3
#define CPU_RING_DESCRIPTOR_SIZE_OF_ENTRY_F_OFFSET_MOD16                 11
#define CPU_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_F_OFFSET                   16
#define CPU_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_F_WIDTH                    11
#define CPU_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_OFFSET                     0
#define CPU_RING_DESCRIPTOR_NUMBER_OF_ENTRIES_F_OFFSET_MOD16             0
#define CPU_RING_DESCRIPTOR_INTERRUPT_ID_F_OFFSET                        0
#define CPU_RING_DESCRIPTOR_INTERRUPT_ID_F_WIDTH                         16
#define CPU_RING_DESCRIPTOR_INTERRUPT_ID_OFFSET                          2
#define CPU_RING_DESCRIPTOR_DROP_COUNTER_F_OFFSET                        16
#define CPU_RING_DESCRIPTOR_DROP_COUNTER_F_WIDTH                         16
#define CPU_RING_DESCRIPTOR_DROP_COUNTER_OFFSET                          4
#define CPU_RING_DESCRIPTOR_WRITE_IDX_F_OFFSET                           0
#define CPU_RING_DESCRIPTOR_WRITE_IDX_F_WIDTH                            16
#define CPU_RING_DESCRIPTOR_WRITE_IDX_OFFSET                             6
#define CPU_RING_DESCRIPTOR_BASE_ADDR_LOW_F_OFFSET                       0
#define CPU_RING_DESCRIPTOR_BASE_ADDR_LOW_F_WIDTH                        32
#define CPU_RING_DESCRIPTOR_BASE_ADDR_LOW_OFFSET                         8
#define CPU_RING_DESCRIPTOR_READ_IDX_F_OFFSET                            16
#define CPU_RING_DESCRIPTOR_READ_IDX_F_WIDTH                             16
#define CPU_RING_DESCRIPTOR_READ_IDX_OFFSET                              12
#define CPU_RING_DESCRIPTOR_LOW_PRIORITY_THRESHOLD_F_OFFSET              8
#define CPU_RING_DESCRIPTOR_LOW_PRIORITY_THRESHOLD_F_WIDTH               8
#define CPU_RING_DESCRIPTOR_LOW_PRIORITY_THRESHOLD_OFFSET                14
#define CPU_RING_DESCRIPTOR_BASE_ADDR_HIGH_F_OFFSET                      0
#define CPU_RING_DESCRIPTOR_BASE_ADDR_HIGH_F_WIDTH                       8
#define CPU_RING_DESCRIPTOR_BASE_ADDR_HIGH_OFFSET                        15
/* <<<RDD_CPU_RING_DESCRIPTOR */


/* >>>RDD_PERIPHERALS_STS */
#define PERIPHERALS_STS_SCHEDULER_DEBUG_F_OFFSET                     27
#define PERIPHERALS_STS_SCHEDULER_DEBUG_F_WIDTH                      5
#define PERIPHERALS_STS_SCHEDULER_DEBUG_OFFSET                       0
#define PERIPHERALS_STS_SCHEDULER_DEBUG_F_OFFSET_MOD8                3
#define PERIPHERALS_STS_SCHEDULER_DEBUG_F_OFFSET_MOD16               11
#define PERIPHERALS_STS_NEXT_THREAD_IS_READY_F_OFFSET                26
#define PERIPHERALS_STS_NEXT_THREAD_IS_READY_F_WIDTH                 1
#define PERIPHERALS_STS_NEXT_THREAD_IS_READY_OFFSET                  0
#define PERIPHERALS_STS_NEXT_THREAD_IS_READY_F_OFFSET_MOD8           2
#define PERIPHERALS_STS_NEXT_THREAD_IS_READY_F_OFFSET_MOD16          10
#define PERIPHERALS_STS_NEXT_THREAD_NUMBER_F_OFFSET                  21
#define PERIPHERALS_STS_NEXT_THREAD_NUMBER_F_WIDTH                   5
#define PERIPHERALS_STS_NEXT_THREAD_NUMBER_OFFSET                    0
#define PERIPHERALS_STS_NEXT_THREAD_NUMBER_F_OFFSET_MOD8             5
#define PERIPHERALS_STS_NEXT_THREAD_NUMBER_F_OFFSET_MOD16            5
#define PERIPHERALS_STS_CURRENT_THREAD_NUMBER_F_OFFSET               16
#define PERIPHERALS_STS_CURRENT_THREAD_NUMBER_F_WIDTH                5
#define PERIPHERALS_STS_CURRENT_THREAD_NUMBER_OFFSET                 1
#define PERIPHERALS_STS_CURRENT_THREAD_NUMBER_F_OFFSET_MOD8          0
#define PERIPHERALS_STS_CURRENT_THREAD_NUMBER_F_OFFSET_MOD16         0
#define PERIPHERALS_STS_TIMER_1_STATUS_F_OFFSET                      12
#define PERIPHERALS_STS_TIMER_1_STATUS_F_WIDTH                       1
#define PERIPHERALS_STS_TIMER_1_STATUS_OFFSET                        2
#define PERIPHERALS_STS_TIMER_1_STATUS_F_OFFSET_MOD8                 4
#define PERIPHERALS_STS_TIMER_0_STATUS_F_OFFSET                      11
#define PERIPHERALS_STS_TIMER_0_STATUS_F_WIDTH                       1
#define PERIPHERALS_STS_TIMER_0_STATUS_OFFSET                        2
#define PERIPHERALS_STS_TIMER_0_STATUS_F_OFFSET_MOD8                 3
#define PERIPHERALS_STS_RAM_READ_COMMAND_FIFO_FULL_F_OFFSET          8
#define PERIPHERALS_STS_RAM_READ_COMMAND_FIFO_FULL_F_WIDTH           1
#define PERIPHERALS_STS_RAM_READ_COMMAND_FIFO_FULL_OFFSET            2
#define PERIPHERALS_STS_RAM_READ_COMMAND_FIFO_FULL_F_OFFSET_MOD8     0
#define PERIPHERALS_STS_QUAD_ID_F_OFFSET                             6
#define PERIPHERALS_STS_QUAD_ID_F_WIDTH                              2
#define PERIPHERALS_STS_QUAD_ID_OFFSET                               3
#define PERIPHERALS_STS_CORE_ID_F_OFFSET                             4
#define PERIPHERALS_STS_CORE_ID_F_WIDTH                              2
#define PERIPHERALS_STS_CORE_ID_OFFSET                               3
#define PERIPHERALS_STS_BB_MESSAGE_PENDING_F_OFFSET                  3
#define PERIPHERALS_STS_BB_MESSAGE_PENDING_F_WIDTH                   1
#define PERIPHERALS_STS_BB_MESSAGE_PENDING_OFFSET                    3
#define PERIPHERALS_STS_BBTX_COMMAND_FIFO_IS_EMPTY_F_OFFSET          2
#define PERIPHERALS_STS_BBTX_COMMAND_FIFO_IS_EMPTY_F_WIDTH           1
#define PERIPHERALS_STS_BBTX_COMMAND_FIFO_IS_EMPTY_OFFSET            3
#define PERIPHERALS_STS_BBTX_COMMAND_FIFO_FULL_F_OFFSET              1
#define PERIPHERALS_STS_BBTX_COMMAND_FIFO_FULL_F_WIDTH               1
#define PERIPHERALS_STS_BBTX_COMMAND_FIFO_FULL_OFFSET                3
#define PERIPHERALS_STS_DMA_COMMAND_FIFO_FULL_F_OFFSET               0
#define PERIPHERALS_STS_DMA_COMMAND_FIFO_FULL_F_WIDTH                1
#define PERIPHERALS_STS_DMA_COMMAND_FIFO_FULL_OFFSET                 3

/* >>>RDD_PERIPHERALS_STS_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	scheduler_debug           	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	next_thread_is_ready      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	next_thread_number        	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	current_thread_number     	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_1                	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	timer_1_status            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	timer_0_status            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_2                	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ram_read_command_fifo_full	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	quad_id                   	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	core_id                   	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bb_message_pending        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbtx_command_fifo_is_empty	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbtx_command_fifo_full    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	dma_command_fifo_full     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	dma_command_fifo_full     	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbtx_command_fifo_full    	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bbtx_command_fifo_is_empty	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	bb_message_pending        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	core_id                   	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	quad_id                   	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	ram_read_command_fifo_full	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_2                	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	timer_0_status            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	timer_1_status            	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved_1                	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	current_thread_number     	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	next_thread_number        	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	next_thread_is_ready      	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	scheduler_debug           	:5	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_PERIPHERALS_STS_DTS;

#define RDD_PERIPHERALS_STS_SCHEDULER_DEBUG_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS), 3, 5, r)
#define RDD_PERIPHERALS_STS_SCHEDULER_DEBUG_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS), 3, 5, v)
#define RDD_PERIPHERALS_STS_SCHEDULER_DEBUG_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p, 3, 5, r)
#define RDD_PERIPHERALS_STS_SCHEDULER_DEBUG_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p, 3, 5, v)
#define RDD_PERIPHERALS_STS_NEXT_THREAD_IS_READY_READ_G(r, g, idx)                GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS), 2, 1, r)
#define RDD_PERIPHERALS_STS_NEXT_THREAD_IS_READY_WRITE_G(v, g, idx)               GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS), 2, 1, v)
#define RDD_PERIPHERALS_STS_NEXT_THREAD_IS_READY_READ(r, p)                       FIELD_MREAD_8((uint8_t *)p, 2, 1, r)
#define RDD_PERIPHERALS_STS_NEXT_THREAD_IS_READY_WRITE(v, p)                      FIELD_MWRITE_8((uint8_t *)p, 2, 1, v)
#define RDD_PERIPHERALS_STS_NEXT_THREAD_NUMBER_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS), 5, 5, r)
#define RDD_PERIPHERALS_STS_NEXT_THREAD_NUMBER_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS), 5, 5, v)
#define RDD_PERIPHERALS_STS_NEXT_THREAD_NUMBER_READ(r, p)                         FIELD_MREAD_16((uint8_t *)p, 5, 5, r)
#define RDD_PERIPHERALS_STS_NEXT_THREAD_NUMBER_WRITE(v, p)                        FIELD_MWRITE_16((uint8_t *)p, 5, 5, v)
#define RDD_PERIPHERALS_STS_CURRENT_THREAD_NUMBER_READ_G(r, g, idx)               GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS) + 1, 0, 5, r)
#define RDD_PERIPHERALS_STS_CURRENT_THREAD_NUMBER_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS) + 1, 0, 5, v)
#define RDD_PERIPHERALS_STS_CURRENT_THREAD_NUMBER_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 1, 0, 5, r)
#define RDD_PERIPHERALS_STS_CURRENT_THREAD_NUMBER_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 1, 0, 5, v)
#define RDD_PERIPHERALS_STS_TIMER_1_STATUS_READ_G(r, g, idx)                      GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS) + 2, 4, 1, r)
#define RDD_PERIPHERALS_STS_TIMER_1_STATUS_WRITE_G(v, g, idx)                     GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS) + 2, 4, 1, v)
#define RDD_PERIPHERALS_STS_TIMER_1_STATUS_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 2, 4, 1, r)
#define RDD_PERIPHERALS_STS_TIMER_1_STATUS_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 2, 4, 1, v)
#define RDD_PERIPHERALS_STS_TIMER_0_STATUS_READ_G(r, g, idx)                      GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS) + 2, 3, 1, r)
#define RDD_PERIPHERALS_STS_TIMER_0_STATUS_WRITE_G(v, g, idx)                     GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS) + 2, 3, 1, v)
#define RDD_PERIPHERALS_STS_TIMER_0_STATUS_READ(r, p)                             FIELD_MREAD_8((uint8_t *)p + 2, 3, 1, r)
#define RDD_PERIPHERALS_STS_TIMER_0_STATUS_WRITE(v, p)                            FIELD_MWRITE_8((uint8_t *)p + 2, 3, 1, v)
#define RDD_PERIPHERALS_STS_RAM_READ_COMMAND_FIFO_FULL_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS) + 2, 0, 1, r)
#define RDD_PERIPHERALS_STS_RAM_READ_COMMAND_FIFO_FULL_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS) + 2, 0, 1, v)
#define RDD_PERIPHERALS_STS_RAM_READ_COMMAND_FIFO_FULL_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 2, 0, 1, r)
#define RDD_PERIPHERALS_STS_RAM_READ_COMMAND_FIFO_FULL_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 2, 0, 1, v)
#define RDD_PERIPHERALS_STS_QUAD_ID_READ_G(r, g, idx)                             GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS) + 3, 6, 2, r)
#define RDD_PERIPHERALS_STS_QUAD_ID_WRITE_G(v, g, idx)                            GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS) + 3, 6, 2, v)
#define RDD_PERIPHERALS_STS_QUAD_ID_READ(r, p)                                    FIELD_MREAD_8((uint8_t *)p + 3, 6, 2, r)
#define RDD_PERIPHERALS_STS_QUAD_ID_WRITE(v, p)                                   FIELD_MWRITE_8((uint8_t *)p + 3, 6, 2, v)
#define RDD_PERIPHERALS_STS_CORE_ID_READ_G(r, g, idx)                             GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS) + 3, 4, 2, r)
#define RDD_PERIPHERALS_STS_CORE_ID_WRITE_G(v, g, idx)                            GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS) + 3, 4, 2, v)
#define RDD_PERIPHERALS_STS_CORE_ID_READ(r, p)                                    FIELD_MREAD_8((uint8_t *)p + 3, 4, 2, r)
#define RDD_PERIPHERALS_STS_CORE_ID_WRITE(v, p)                                   FIELD_MWRITE_8((uint8_t *)p + 3, 4, 2, v)
#define RDD_PERIPHERALS_STS_BB_MESSAGE_PENDING_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS) + 3, 3, 1, r)
#define RDD_PERIPHERALS_STS_BB_MESSAGE_PENDING_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS) + 3, 3, 1, v)
#define RDD_PERIPHERALS_STS_BB_MESSAGE_PENDING_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 3, 3, 1, r)
#define RDD_PERIPHERALS_STS_BB_MESSAGE_PENDING_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 3, 3, 1, v)
#define RDD_PERIPHERALS_STS_BBTX_COMMAND_FIFO_IS_EMPTY_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS) + 3, 2, 1, r)
#define RDD_PERIPHERALS_STS_BBTX_COMMAND_FIFO_IS_EMPTY_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS) + 3, 2, 1, v)
#define RDD_PERIPHERALS_STS_BBTX_COMMAND_FIFO_IS_EMPTY_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 3, 2, 1, r)
#define RDD_PERIPHERALS_STS_BBTX_COMMAND_FIFO_IS_EMPTY_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 3, 2, 1, v)
#define RDD_PERIPHERALS_STS_BBTX_COMMAND_FIFO_FULL_READ_G(r, g, idx)              GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS) + 3, 1, 1, r)
#define RDD_PERIPHERALS_STS_BBTX_COMMAND_FIFO_FULL_WRITE_G(v, g, idx)             GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS) + 3, 1, 1, v)
#define RDD_PERIPHERALS_STS_BBTX_COMMAND_FIFO_FULL_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p + 3, 1, 1, r)
#define RDD_PERIPHERALS_STS_BBTX_COMMAND_FIFO_FULL_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p + 3, 1, 1, v)
#define RDD_PERIPHERALS_STS_DMA_COMMAND_FIFO_FULL_READ_G(r, g, idx)               GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS) + 3, 0, 1, r)
#define RDD_PERIPHERALS_STS_DMA_COMMAND_FIFO_FULL_WRITE_G(v, g, idx)              GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_PERIPHERALS_STS_DTS) + 3, 0, 1, v)
#define RDD_PERIPHERALS_STS_DMA_COMMAND_FIFO_FULL_READ(r, p)                      FIELD_MREAD_8((uint8_t *)p + 3, 0, 1, r)
#define RDD_PERIPHERALS_STS_DMA_COMMAND_FIFO_FULL_WRITE(v, p)                     FIELD_MWRITE_8((uint8_t *)p + 3, 0, 1, v)
/* <<<RDD_PERIPHERALS_STS_DTS */

/* <<<RDD_PERIPHERALS_STS */


/* >>>RDD_ACB_CONTROL */
#define ACB_CONTROL_IMP_PORT_F_OFFSET                            6
#define ACB_CONTROL_IMP_PORT_F_WIDTH                             2
#define ACB_CONTROL_IMP_PORT_OFFSET                              0
#define ACB_CONTROL_EGRESS_PORT_F_OFFSET                         3
#define ACB_CONTROL_EGRESS_PORT_F_WIDTH                          3
#define ACB_CONTROL_EGRESS_PORT_OFFSET                           0
#define ACB_CONTROL_EGRESS_QUEUE_ID_F_OFFSET                     0
#define ACB_CONTROL_EGRESS_QUEUE_ID_F_WIDTH                      3
#define ACB_CONTROL_EGRESS_QUEUE_ID_OFFSET                       0

/* >>>RDD_ACB_CONTROL_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint8_t	imp_port       	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	egress_port    	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	egress_queue_id	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint8_t	egress_queue_id	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	egress_port    	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	imp_port       	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_ACB_CONTROL_DTS;

#define RDD_ACB_CONTROL_IMP_PORT_READ_G(r, g, idx)                 GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_ACB_CONTROL_DTS), 6, 2, r)
#define RDD_ACB_CONTROL_IMP_PORT_WRITE_G(v, g, idx)                GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_ACB_CONTROL_DTS), 6, 2, v)
#define RDD_ACB_CONTROL_IMP_PORT_READ(r, p)                        FIELD_MREAD_8((uint8_t *)p, 6, 2, r)
#define RDD_ACB_CONTROL_IMP_PORT_WRITE(v, p)                       FIELD_MWRITE_8((uint8_t *)p, 6, 2, v)
#define RDD_ACB_CONTROL_EGRESS_PORT_READ_G(r, g, idx)              GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_ACB_CONTROL_DTS), 3, 3, r)
#define RDD_ACB_CONTROL_EGRESS_PORT_WRITE_G(v, g, idx)             GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_ACB_CONTROL_DTS), 3, 3, v)
#define RDD_ACB_CONTROL_EGRESS_PORT_READ(r, p)                     FIELD_MREAD_8((uint8_t *)p, 3, 3, r)
#define RDD_ACB_CONTROL_EGRESS_PORT_WRITE(v, p)                    FIELD_MWRITE_8((uint8_t *)p, 3, 3, v)
#define RDD_ACB_CONTROL_EGRESS_QUEUE_ID_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_ACB_CONTROL_DTS), 0, 3, r)
#define RDD_ACB_CONTROL_EGRESS_QUEUE_ID_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_ACB_CONTROL_DTS), 0, 3, v)
#define RDD_ACB_CONTROL_EGRESS_QUEUE_ID_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 0, 3, r)
#define RDD_ACB_CONTROL_EGRESS_QUEUE_ID_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 0, 3, v)
/* <<<RDD_ACB_CONTROL_DTS */

/* <<<RDD_ACB_CONTROL */


/* >>>RDD_HASH_COMMAND */
#define HASH_COMMAND_CONFIGURATION_F_OFFSET                       29
#define HASH_COMMAND_CONFIGURATION_F_WIDTH                        3
#define HASH_COMMAND_CONFIGURATION_OFFSET                         0
#define HASH_COMMAND_CONFIGURATION_F_OFFSET_MOD8                  5
#define HASH_COMMAND_CONFIGURATION_F_OFFSET_MOD16                 13
#define HASH_COMMAND_AGING_F_OFFSET                               28
#define HASH_COMMAND_AGING_F_WIDTH                                1
#define HASH_COMMAND_AGING_OFFSET                                 0
#define HASH_COMMAND_AGING_F_OFFSET_MOD8                          4
#define HASH_COMMAND_AGING_F_OFFSET_MOD16                         12
#define HASH_COMMAND_KEY_1_F_OFFSET                               0
#define HASH_COMMAND_KEY_1_F_WIDTH                                28
#define HASH_COMMAND_KEY_1_OFFSET                                 0
#define HASH_COMMAND_KEY_0_F_OFFSET                               0
#define HASH_COMMAND_KEY_0_F_WIDTH                                32
#define HASH_COMMAND_KEY_0_OFFSET                                 4

/* >>>RDD_HASH_COMMAND_DTS */

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	configuration	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	aging        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	key_1        	:28	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	key_0        	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#else
	uint32_t	key_1        	:28	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	aging        	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	configuration	:3	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	key_0        	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_HASH_COMMAND_DTS;

#define RDD_HASH_COMMAND_CONFIGURATION_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_HASH_COMMAND_DTS), 5, 3, r)
#define RDD_HASH_COMMAND_CONFIGURATION_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_HASH_COMMAND_DTS), 5, 3, v)
#define RDD_HASH_COMMAND_CONFIGURATION_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p, 5, 3, r)
#define RDD_HASH_COMMAND_CONFIGURATION_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p, 5, 3, v)
#define RDD_HASH_COMMAND_AGING_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_HASH_COMMAND_DTS), 4, 1, r)
#define RDD_HASH_COMMAND_AGING_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_HASH_COMMAND_DTS), 4, 1, v)
#define RDD_HASH_COMMAND_AGING_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p, 4, 1, r)
#define RDD_HASH_COMMAND_AGING_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p, 4, 1, v)
#define RDD_HASH_COMMAND_KEY_1_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_32(g, idx*sizeof(RDD_HASH_COMMAND_DTS), 0, 28, r)
#define RDD_HASH_COMMAND_KEY_1_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_32(g, idx*sizeof(RDD_HASH_COMMAND_DTS), 0, 28, v)
#define RDD_HASH_COMMAND_KEY_1_READ(r, p)                         FIELD_MREAD_32((uint8_t *)p, 0, 28, r)
#define RDD_HASH_COMMAND_KEY_1_WRITE(v, p)                        FIELD_MWRITE_32((uint8_t *)p, 0, 28, v)
#define RDD_HASH_COMMAND_KEY_0_READ_G(r, g, idx)                  GROUP_MREAD_32(g, idx*sizeof(RDD_HASH_COMMAND_DTS) + 4, r)
#define RDD_HASH_COMMAND_KEY_0_WRITE_G(v, g, idx)                 GROUP_MWRITE_32(g, idx*sizeof(RDD_HASH_COMMAND_DTS) + 4, v)
#define RDD_HASH_COMMAND_KEY_0_READ(r, p)                         MREAD_32((uint8_t *)p + 4, r)
#define RDD_HASH_COMMAND_KEY_0_WRITE(v, p)                        MWRITE_32((uint8_t *)p + 4, v)
/* <<<RDD_HASH_COMMAND_DTS */

/* <<<RDD_HASH_COMMAND */


/* >>>RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR */
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_RL_TYPE_F_OFFSET                             31
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_RL_TYPE_F_WIDTH                              1
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_RL_TYPE_OFFSET                               0
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_RL_TYPE_F_OFFSET_MOD8                        7
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_RL_TYPE_F_OFFSET_MOD16                       15
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_BLOCK_TYPE_F_OFFSET                          8
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_BLOCK_TYPE_F_WIDTH                           1
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_BLOCK_TYPE_OFFSET                            2
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_BLOCK_TYPE_F_OFFSET_MOD8                     0
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_BLOCK_INDEX_F_OFFSET                         0
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_BLOCK_INDEX_F_WIDTH                          8
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_BLOCK_INDEX_OFFSET                           3
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_CURRENT_BUDGET_F_OFFSET                  0
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_CURRENT_BUDGET_F_WIDTH                   32
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_CURRENT_BUDGET_OFFSET                    4
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_ALLOC_MANTISSA_F_OFFSET                  18
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_ALLOC_MANTISSA_F_WIDTH                   14
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_ALLOC_MANTISSA_OFFSET                    8
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_ALLOC_MANTISSA_F_OFFSET_MOD16            2
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_ALLOC_EXPONENT_F_OFFSET                  16
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_ALLOC_EXPONENT_F_WIDTH                   2
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_ALLOC_EXPONENT_OFFSET                    9
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_ALLOC_EXPONENT_F_OFFSET_MOD8             0
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_ALLOC_EXPONENT_F_OFFSET_MOD16            0
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_LIMIT_MANTISSA_F_OFFSET                  2
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_LIMIT_MANTISSA_F_WIDTH                   14
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_LIMIT_MANTISSA_OFFSET                    10
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_LIMIT_EXPONENT_F_OFFSET                  0
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_LIMIT_EXPONENT_F_WIDTH                   2
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_LIMIT_EXPONENT_OFFSET                    11
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_CURRENT_BUDGET_F_OFFSET                  0
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_CURRENT_BUDGET_F_WIDTH                   32
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_CURRENT_BUDGET_OFFSET                    12
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_ALLOC_MANTISSA_F_OFFSET                  18
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_ALLOC_MANTISSA_F_WIDTH                   14
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_ALLOC_MANTISSA_OFFSET                    16
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_ALLOC_MANTISSA_F_OFFSET_MOD16            2
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_ALLOC_EXPONENT_F_OFFSET                  16
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_ALLOC_EXPONENT_F_WIDTH                   2
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_ALLOC_EXPONENT_OFFSET                    17
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_ALLOC_EXPONENT_F_OFFSET_MOD8             0
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_ALLOC_EXPONENT_F_OFFSET_MOD16            0
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_LIMIT_MANTISSA_F_OFFSET                  2
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_LIMIT_MANTISSA_F_WIDTH                   14
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_LIMIT_MANTISSA_OFFSET                    18
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_LIMIT_EXPONENT_F_OFFSET                  0
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_LIMIT_EXPONENT_F_WIDTH                   2
#define COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_LIMIT_EXPONENT_OFFSET                    19

/* >>>RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS */
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_RESERVED_NUMBER	12

typedef struct
{
#ifndef FIRMWARE_LITTLE_ENDIAN
	uint32_t	rl_type                                                          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2                                                        	:22	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	block_type                                                       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	block_index                                                      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sir_current_budget                                               	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sir_alloc_mantissa                                               	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sir_alloc_exponent                                               	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sir_limit_mantissa                                               	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sir_limit_exponent                                               	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pir_current_budget                                               	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pir_alloc_mantissa                                               	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pir_alloc_exponent                                               	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pir_limit_mantissa                                               	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pir_limit_exponent                                               	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	reserved[RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_RESERVED_NUMBER];
#else
	uint32_t	block_index                                                      	:8	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	block_type                                                       	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	reserved2                                                        	:22	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	rl_type                                                          	:1	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sir_current_budget                                               	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sir_limit_exponent                                               	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sir_limit_mantissa                                               	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sir_alloc_exponent                                               	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	sir_alloc_mantissa                                               	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pir_current_budget                                               	:32	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pir_limit_exponent                                               	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pir_limit_mantissa                                               	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pir_alloc_exponent                                               	:2	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint32_t	pir_alloc_mantissa                                               	:14	__PACKING_ATTRIBUTE_FIELD_LEVEL__;
	uint8_t	reserved[RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_RESERVED_NUMBER];
#endif
}
__PACKING_ATTRIBUTE_STRUCT_END__ RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS;

#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_RL_TYPE_READ_G(r, g, idx)                     GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS), 7, 1, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_RL_TYPE_WRITE_G(v, g, idx)                    GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS), 7, 1, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_RL_TYPE_READ(r, p)                            FIELD_MREAD_8((uint8_t *)p, 7, 1, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_RL_TYPE_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_BLOCK_TYPE_READ_G(r, g, idx)                  GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 2, 0, 1, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_BLOCK_TYPE_WRITE_G(v, g, idx)                 GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 2, 0, 1, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_BLOCK_TYPE_READ(r, p)                         FIELD_MREAD_8((uint8_t *)p + 2, 0, 1, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_BLOCK_TYPE_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 2, 0, 1, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_BLOCK_INDEX_READ_G(r, g, idx)                 GROUP_MREAD_8(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 3, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_BLOCK_INDEX_WRITE_G(v, g, idx)                GROUP_MWRITE_8(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 3, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_BLOCK_INDEX_READ(r, p)                        MREAD_8((uint8_t *)p + 3, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_BLOCK_INDEX_WRITE(v, p)                       MWRITE_8((uint8_t *)p + 3, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_CURRENT_BUDGET_READ_G(r, g, idx)          GROUP_MREAD_32(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 4, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_CURRENT_BUDGET_WRITE_G(v, g, idx)         GROUP_MWRITE_32(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 4, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_CURRENT_BUDGET_READ(r, p)                 MREAD_32((uint8_t *)p + 4, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_CURRENT_BUDGET_WRITE(v, p)                MWRITE_32((uint8_t *)p + 4, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_ALLOC_MANTISSA_READ_G(r, g, idx)          GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 8, 2, 14, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_ALLOC_MANTISSA_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 8, 2, 14, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_ALLOC_MANTISSA_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p + 8, 2, 14, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_ALLOC_MANTISSA_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p + 8, 2, 14, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_ALLOC_EXPONENT_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 9, 0, 2, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_ALLOC_EXPONENT_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 9, 0, 2, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_ALLOC_EXPONENT_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 9, 0, 2, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_ALLOC_EXPONENT_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 9, 0, 2, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_LIMIT_MANTISSA_READ_G(r, g, idx)          GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 10, 2, 14, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_LIMIT_MANTISSA_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 10, 2, 14, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_LIMIT_MANTISSA_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p + 10, 2, 14, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_LIMIT_MANTISSA_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p + 10, 2, 14, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_LIMIT_EXPONENT_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 11, 0, 2, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_LIMIT_EXPONENT_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 11, 0, 2, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_LIMIT_EXPONENT_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 11, 0, 2, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_SIR_LIMIT_EXPONENT_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 11, 0, 2, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_CURRENT_BUDGET_READ_G(r, g, idx)          GROUP_MREAD_32(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 12, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_CURRENT_BUDGET_WRITE_G(v, g, idx)         GROUP_MWRITE_32(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 12, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_CURRENT_BUDGET_READ(r, p)                 MREAD_32((uint8_t *)p + 12, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_CURRENT_BUDGET_WRITE(v, p)                MWRITE_32((uint8_t *)p + 12, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_ALLOC_MANTISSA_READ_G(r, g, idx)          GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 16, 2, 14, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_ALLOC_MANTISSA_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 16, 2, 14, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_ALLOC_MANTISSA_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p + 16, 2, 14, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_ALLOC_MANTISSA_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p + 16, 2, 14, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_ALLOC_EXPONENT_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 17, 0, 2, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_ALLOC_EXPONENT_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 17, 0, 2, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_ALLOC_EXPONENT_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 17, 0, 2, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_ALLOC_EXPONENT_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 17, 0, 2, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_LIMIT_MANTISSA_READ_G(r, g, idx)          GROUP_FIELD_MREAD_16(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 18, 2, 14, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_LIMIT_MANTISSA_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_16(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 18, 2, 14, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_LIMIT_MANTISSA_READ(r, p)                 FIELD_MREAD_16((uint8_t *)p + 18, 2, 14, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_LIMIT_MANTISSA_WRITE(v, p)                FIELD_MWRITE_16((uint8_t *)p + 18, 2, 14, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_LIMIT_EXPONENT_READ_G(r, g, idx)          GROUP_FIELD_MREAD_8(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 19, 0, 2, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_LIMIT_EXPONENT_WRITE_G(v, g, idx)         GROUP_FIELD_MWRITE_8(g, idx*sizeof(RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS) + 19, 0, 2, v)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_LIMIT_EXPONENT_READ(r, p)                 FIELD_MREAD_8((uint8_t *)p + 19, 0, 2, r)
#define RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_PIR_LIMIT_EXPONENT_WRITE(v, p)                FIELD_MWRITE_8((uint8_t *)p + 19, 0, 2, v)
/* <<<RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR_DTS */

/* <<<RDD_COMPLEX_RATE_LIMITER_DESCRIPTOR */

#endif /* _RDD_DATA_STRUCTURES_AUTO_H_ */

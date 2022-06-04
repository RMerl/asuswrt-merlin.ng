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


#include "bdmf_shell.h"
#include "rdd_map_auto.h"
#include "rdd_runner_reg_dump.h"
#include "rdd_runner_reg_dump_addrs.h"
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT CPU_RX_CFE_SRAM_COUNTERS =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x0 },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT CPU_RX_FPM_POOL_NUMBER_MAPPING_TABLE =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0x50 },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT RUNNER_GLOBAL_REGISTERS_INIT =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x60 },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT CPU_TX_CFE_SRAM_COUNTERS =
{
	8,
	{
		{ dump_RDD_PACKETS_AND_BYTES, 0x80 },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE =
{
	1,
	{
		{ dump_RDD_FPM_POOL_NUMBER, 0xd0 },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT BBH_TX_RING_TABLE =
{
	16,
	{
		{ dump_RDD_BBH_TX_DESCRIPTOR, 0xe0 },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0xf0 },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT BBH_TX_BB_DESTINATION_TABLE =
{
	4,
	{
		{ dump_RDD_BB_DESTINATION_ENTRY, 0xfc },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT US_TM_BBH_TX_EGRESS_COUNTER_TABLE =
{
	8,
	{
		{ dump_RDD_BBH_TX_EGRESS_COUNTER_ENTRY, 0x100 },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT DIRECT_PROCESSING_PD_TABLE =
{
	16,
	{
		{ dump_RDD_PROCESSING_RX_DESCRIPTOR, 0x140 },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x160 },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT TASK_IDX =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x16c },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_SCRATCH =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x170 },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT CPU_RX_INTERRUPT_ID_DDR_ADDR =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x178 },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT DS_TM_BBH_TX_EGRESS_COUNTER_TABLE =
{
	8,
	{
		{ dump_RDD_BBH_TX_EGRESS_COUNTER_ENTRY, 0x180 },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT REGISTERS_BUFFER =
{
	4,
	{
		{ dump_RDD_BYTES_4, 0x1c0 },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT CPU_RECYCLE_RING_DESCRIPTOR_TABLE =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x240 },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT CPU_FEED_RING_DESCRIPTOR_TABLE =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x250 },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT FPM_GLOBAL_CFG =
{
	12,
	{
		{ dump_RDD_FPM_GLOBAL_CFG, 0x260 },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT SRAM_DUMMY_STORE =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x26c },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT RX_FLOW_TABLE =
{
	2,
	{
		{ dump_RDD_RX_FLOW_ENTRY, 0x400 },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT SRAM_SCRATCH =
{
	1,
	{
		{ dump_RDD_BYTE_1, 0x800 },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT SRAM_PD_FIFO =
{
	16,
	{
		{ dump_RDD_PROCESSING_TX_DESCRIPTOR, 0x1000 },
		{ 0, 0 },
	}
};
#endif
#if defined XRDP_CFE
static DUMP_RUNNERREG_STRUCT CPU_RING_DESCRIPTORS_TABLE =
{
	16,
	{
		{ dump_RDD_CPU_RING_DESCRIPTOR, 0x3000 },
		{ 0, 0 },
	}
};
#endif

TABLE_STRUCT RUNNER_TABLES[NUMBER_OF_TABLES] =
{
#if defined XRDP_CFE
	{ "CPU_RX_CFE_SRAM_COUNTERS", 1, CORE_0_INDEX, &CPU_RX_CFE_SRAM_COUNTERS, 10, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "CPU_RX_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_0_INDEX, &CPU_RX_FPM_POOL_NUMBER_MAPPING_TABLE, 16, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "RUNNER_GLOBAL_REGISTERS_INIT", 1, CORE_0_INDEX, &RUNNER_GLOBAL_REGISTERS_INIT, 8, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "CPU_TX_CFE_SRAM_COUNTERS", 1, CORE_0_INDEX, &CPU_TX_CFE_SRAM_COUNTERS, 10, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE", 1, CORE_0_INDEX, &CPU_TX_FPM_POOL_NUMBER_MAPPING_TABLE, 16, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "BBH_TX_RING_TABLE", 1, CORE_0_INDEX, &BBH_TX_RING_TABLE, 1, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &CPU_TX_EGRESS_DISPATCHER_CREDIT_TABLE, 3, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "BBH_TX_BB_DESTINATION_TABLE", 1, CORE_0_INDEX, &BBH_TX_BB_DESTINATION_TABLE, 1, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "US_TM_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_0_INDEX, &US_TM_BBH_TX_EGRESS_COUNTER_TABLE, 8, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "DIRECT_PROCESSING_PD_TABLE", 1, CORE_0_INDEX, &DIRECT_PROCESSING_PD_TABLE, 2, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE", 1, CORE_0_INDEX, &CPU_TX_INGRESS_DISPATCHER_CREDIT_TABLE, 3, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "TASK_IDX", 1, CORE_0_INDEX, &TASK_IDX, 1, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "CPU_RX_INTERRUPT_SCRATCH", 1, CORE_0_INDEX, &CPU_RX_INTERRUPT_SCRATCH, 8, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "CPU_RX_INTERRUPT_ID_DDR_ADDR", 1, CORE_0_INDEX, &CPU_RX_INTERRUPT_ID_DDR_ADDR, 8, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "DS_TM_BBH_TX_EGRESS_COUNTER_TABLE", 1, CORE_0_INDEX, &DS_TM_BBH_TX_EGRESS_COUNTER_TABLE, 8, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "REGISTERS_BUFFER", 1, CORE_0_INDEX, &REGISTERS_BUFFER, 32, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "CPU_RECYCLE_RING_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &CPU_RECYCLE_RING_DESCRIPTOR_TABLE, 1, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "CPU_FEED_RING_DESCRIPTOR_TABLE", 1, CORE_0_INDEX, &CPU_FEED_RING_DESCRIPTOR_TABLE, 1, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "FPM_GLOBAL_CFG", 1, CORE_0_INDEX, &FPM_GLOBAL_CFG, 1, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "SRAM_DUMMY_STORE", 1, CORE_0_INDEX, &SRAM_DUMMY_STORE, 1, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "RX_FLOW_TABLE", 1, CORE_0_INDEX, &RX_FLOW_TABLE, 320, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "SRAM_SCRATCH", 1, CORE_0_INDEX, &SRAM_SCRATCH, 2048, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "SRAM_PD_FIFO", 1, CORE_0_INDEX, &SRAM_PD_FIFO, 64, 1, 1 },
#endif
#if defined XRDP_CFE
	{ "CPU_RING_DESCRIPTORS_TABLE", 1, CORE_0_INDEX, &CPU_RING_DESCRIPTORS_TABLE, 16, 1, 1 },
#endif
};

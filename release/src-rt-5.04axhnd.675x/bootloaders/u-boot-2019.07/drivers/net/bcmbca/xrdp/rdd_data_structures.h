// SPDX-License-Identifier: GPL-2.0+
/*
   Copyright (c) 2015 Broadcom
   All Rights Reserved
*/

#ifndef _RDD_DATA_STRUCTURES_AUTO__
#define _RDD_DATA_STRUCTURES_AUTO__

#ifndef NO_ACCESS_LOGGING
#include "access_macros.h"
#if defined(CONFIG_BCM63146) || defined(CONFIG_BCM4912) || defined(CONFIG_BCM6813)
#include <asm/arch/misc.h>
#endif

#if defined(CONFIG_BCM6858)
#define XRDP_BBH_PER_LAN_PORT
#endif

extern uintptr_t rdp_runner_core_addr[];

/* RNR_REGS_ADDRS */
#if defined(CONFIG_BCM6878) || defined(CONFIG_BCM6846) || defined(CONFIG_BCM6856)
#define RNR_REGS_ADDR_CORE0 0x82d00000
#elif defined(CONFIG_BCM6858)
#define RNR_REGS_ADDR_CORE0 0x82280000
#elif defined(CONFIG_BCM63146) || defined(CONFIG_BCM4912) || defined(CONFIG_BCM6813) || defined(CONFIG_BCM6855)
#define RNR_REGS_ADDR_CORE0 0x82800000
#endif

#ifdef XRDP_SBPM
#ifdef CONFIG_BCM6878
#define SBPM_ADDRS 0x82d99000
#define PSRAM_MEM_ADDRS 0x82600000
#define SBPM_MAX_BUFFER_NUMBER 0x3FF
#elif defined(CONFIG_BCM6858)
#define SBPM_ADDRS 0x82d2c000
#define PSRAM_MEM_ADDRS 0x82600000
#define SBPM_MAX_BUFFER_NUMBER 0xFFF
#elif defined(CONFIG_BCM6846)
#define SBPM_ADDRS 0x82d98000
#define PSRAM_MEM_ADDRS 0x82600000
#define SBPM_MAX_BUFFER_NUMBER 0x5FF
#elif defined(CONFIG_BCM6856)
#define SBPM_ADDRS 0x82d99000
#define PSRAM_MEM_ADDRS 0x82600000
#define SBPM_MAX_BUFFER_NUMBER 0x7FF
#elif defined(CONFIG_BCM6855)
#define SBPM_ADDRS 0x828a1000
#define PSRAM_MEM_ADDRS 0x82000000
#define SBPM_MAX_BUFFER_NUMBER 0x7FF
#elif defined(CONFIG_BCM63146) || defined(CONFIG_BCM4912) || defined(CONFIG_BCM6813)
#define SBPM_MAX_BUFFER_NUMBER 0x17F
#endif
#endif

#if defined(CONFIG_BCMBCA_XRDP_DSL)
#if defined(CONFIG_BCM63146) || defined(CONFIG_BCM4912) || defined(CONFIG_BCM6813)
#define PACKET_BUFFER_POOL_TABLE_ADDR_TX 2181099520
#endif
#endif

#ifdef GEN45
#define RDD_BBH_TX_DESCRIPTOR_SOF_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p, 7, 1, v)
#define RDD_BBH_TX_DESCRIPTOR_LAST_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 4, 7, 1, v)
#define RDD_BBH_TX_DESCRIPTOR_BN_NUM_WRITE(v, p)                        FIELD_MWRITE_32((uint8_t *)p + 4, 14, 7, v)
#define RDD_BBH_TX_DESCRIPTOR_PACKET_LENGTH_READ(r, p)                  FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r)
#define RDD_BBH_TX_DESCRIPTOR_PACKET_LENGTH_WRITE(v, p)                 FIELD_MWRITE_16((uint8_t *)p + 6, 0, 14, v)
#define RDD_BBH_TX_DESCRIPTOR_TARGET_MEM_1_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p + 8, 6, 1, v)
#define RDD_BBH_TX_DESCRIPTOR_BN1_FIRST_WRITE(v, p)                     FIELD_MWRITE_32((uint8_t *)p + 8, 0, 18, v)
#define RDD_BBH_TX_DESCRIPTOR_AGG_PD_WRITE(v, p)                        FIELD_MWRITE_8((uint8_t *)p + 12, 7, 1, v)
#define RDD_BBH_TX_DESCRIPTOR_TARGET_MEM_0_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p + 12, 6, 1, v)
#define RDD_BBH_TX_DESCRIPTOR_ABS_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 12, 5, 1, v)
#define RDD_BBH_TX_DESCRIPTOR_SOP_WRITE(v, p)                           FIELD_MWRITE_16((uint8_t *)p + 12, 2, 11, v)
#define RDD_BBH_TX_DESCRIPTOR_BN0_FIRST_WRITE(v, p)                     FIELD_MWRITE_32((uint8_t *)p + 12, 0, 18, v)

#define RDD_BBH_TX_RING_TABLE_PTR(core_id) (DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + 0x50))

#define RDD_BB_DESTINATION_ENTRY_BB_DESTINATION_WRITE(v, p)                MWRITE_32((uint8_t *)p, v)
#define RDD_BBH_TX_BB_DESTINATION_TABLE_PTR(core_id) (DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + 0xdc))

#define RDD_BBH_TX_EGRESS_COUNTER_ENTRY_COUNTER_READ(r, p)                   MREAD_8((uint8_t *)p, r)
#define RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_PTR(core_id) (DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + 0x180 ))

#define RDD_SRAM_PD_FIFO_SIZE     64
#define RDD_SRAM_PD_FIFO_PTR(core_id) (DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + 0x800))
#endif

#ifdef CONFIG_BCMBCA_XRDP_DSL
#define RDD_BBH_TX_DESCRIPTOR_LAST_WRITE(v, p)                          FIELD_MWRITE_8((uint8_t *)p + 4, 7, 1, v)
#define RDD_BBH_TX_DESCRIPTOR_ABS_WRITE(v, p)                           FIELD_MWRITE_8((uint8_t *)p + 5, 3, 1, v)
#define RDD_BBH_TX_DESCRIPTOR_PACKET_LENGTH_READ(r, p)                  FIELD_MREAD_16((uint8_t *)p + 6, 0, 14, r)
#define RDD_BBH_TX_DESCRIPTOR_PACKET_LENGTH_WRITE(v, p)                 FIELD_MWRITE_16((uint8_t *)p + 6, 0, 14, v)
#define RDD_BBH_TX_DESCRIPTOR_TARGET_MEM_1_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p + 8, 6, 1, v)
#define RDD_BBH_TX_DESCRIPTOR_TARGET_MEM_0_WRITE(v, p)                  FIELD_MWRITE_8((uint8_t *)p + 12, 6, 1, v)
#define RDD_BBH_TX_RING_TABLE_PTR(core_id) (DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + 0xd60))

#define RDD_BB_DESTINATION_ENTRY_BB_DESTINATION_WRITE(v, p)                MWRITE_32((uint8_t *)p, v)
#define RDD_BBH_TX_EGRESS_COUNTER_ENTRY_COUNTER_READ(r, p)                   MREAD_8((uint8_t *)p, r)

#define RDD_CPU_RX_LAST_READ_INDEX_PTR(core_id) DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + 0xdfc)

#define RDD_BBH_TX_BB_DESTINATION_TABLE_PTR(core_id) (DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + 0xd7c))

#define RDD_US_TM_BBH_TX_EGRESS_COUNTER_TABLE_PTR(core_id) (DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + 0x100 ))
#define RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_PTR(core_id) (DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + 0xe80 ))

#define RDD_SRAM_PD_FIFO_SIZE     16
#define RDD_SRAM_PD_FIFO_PTR(core_id) (DEVICE_ADDRESS( rdp_runner_core_addr[core_id] + 0x0))
#endif

typedef enum
{
#if defined(CONFIG_BCM6878) || defined(CONFIG_BCM6846) || defined(CONFIG_BCM6856) || defined(CONFIG_BCM6855)
	BB_ID_RX_BBH_0              = 31,
	BB_ID_TX_LAN                = 32,
	BB_ID_RX_BBH_1              = 33,
	BB_ID_RX_BBH_2              = 35,
	BB_ID_RX_BBH_3              = 37,
	BB_ID_RX_BBH_4              = 39,
	BB_ID_RX_BBH_5              = 41,
#elif defined(CONFIG_BCM6858)
	BB_ID_RX_BBH_0              = 31,
	BB_ID_TX_BBH_0              = 32,
	BB_ID_RX_BBH_1              = 33,
	BB_ID_TX_BBH_1              = 34,
	BB_ID_RX_BBH_2              = 35,
	BB_ID_TX_BBH_2              = 36,
	BB_ID_RX_BBH_3              = 37,
	BB_ID_TX_BBH_3              = 38,
	BB_ID_RX_BBH_4              = 39,
	BB_ID_TX_BBH_4              = 40,
	BB_ID_RX_BBH_5              = 41,
	BB_ID_TX_BBH_5              = 42,
	BB_ID_RX_BBH_6              = 43,
	BB_ID_TX_BBH_6              = 44,
	BB_ID_RX_BBH_7              = 51,
	BB_ID_TX_BBH_7              = 52,
#elif defined(CONFIG_BCM4912) || defined(CONFIG_BCM6813)
	BB_ID_RX_BBH_0              = 31,
	BB_ID_TX_LAN                = 32,
	BB_ID_RX_BBH_1              = 33,
	BB_ID_TX_LAN_1              = 34,
	BB_ID_RX_BBH_2              = 35,
	BB_ID_RX_BBH_3              = 37,
	BB_ID_RX_BBH_4              = 39,
	BB_ID_RX_BBH_5              = 41,
	BB_ID_RX_BBH_6              = 43,
	BB_ID_RX_BBH_7              = 45,
	BB_ID_RX_BBH_8              = 47,
#elif defined(CONFIG_BCM63146)
	BB_ID_RX_BBH_0              = 31,
	BB_ID_TX_LAN                = 32,
	BB_ID_RX_BBH_1              = 33,
	BB_ID_RX_BBH_2              = 35,
	BB_ID_RX_BBH_3              = 37,
	BB_ID_RX_BBH_4              = 39,
	BB_ID_RX_BBH_5              = 41,
	BB_ID_RX_BBH_6              = 43,
	BB_ID_RX_BBH_7              = 45,
#endif
} rdd_bb_id;

static inline void rnr_regs_cfg_cpu_wakeup_set(uint8_t rnr_id,
	uint8_t thread_num)
{
	writel(thread_num, RNR_REGS_ADDR_CORE0+0x4);
}

#include "access_logging.h"

#if defined(CONFIG_BCM63146) || defined(CONFIG_BCM4912)
extern const access_log_tuple_t init_data_a0[];
#endif
extern const access_log_tuple_t init_data[];
	
static inline int xrdp_data_path_init(void)
{
#if defined(CONFIG_BCM63146) || defined(CONFIG_BCM4912)
	unsigned int rev_id = PERF->RevID & REV_ID_MASK & 0xff;

	if (rev_id == 0xa0)
		return access_log_restore(init_data_a0);
#endif
	return access_log_restore(init_data);
}
#else

#include <rdd.h>
#include <rdp_drv_rnr.h>

extern int xrdp_data_path_init(void);

#define rnr_regs_cfg_cpu_wakeup_set(rnr_id, thread_num) \
	ag_drv_rnr_regs_cfg_cpu_wakeup_set(rnr_id, thread_num)
#endif

static inline uint32_t __rdd_cpu_tx_get_bb_id(uint8_t tx_port)
{
#if defined(CONFIG_BCM6858)
	uint32_t tx_map[] = {BB_ID_TX_BBH_4, BB_ID_TX_BBH_1, BB_ID_TX_BBH_2,
		BB_ID_TX_BBH_3, BB_ID_TX_BBH_0, BB_ID_TX_BBH_5, BB_ID_TX_BBH_6};

	if (tx_port < sizeof(tx_map)/sizeof(tx_map[0]))
		return tx_map[tx_port];

	return tx_map[0];
#elif defined(CONFIG_BCM4912) || defined(CONFIG_BCM6813)
	if (tx_port <= 5)
		return (BB_ID_TX_LAN + (tx_port << 6));
	else
		return (BB_ID_TX_LAN_1 + ((tx_port - 6) << 6));
#else
	return (BB_ID_TX_LAN + (tx_port << 6));
#endif
}

#endif

// SPDX-License-Identifier: GPL-2.0+
/*
 *	Copyright 2020 Broadcom Ltd.
 *	Ido Brezel <ido.brezel@broadcom.com>
 */

#include <common.h>
#include <asm/io.h>
#include "rdd_data_structures.h"

#ifdef XRDP_SBPM
extern int drv_sbpm_copy_list(uint16_t bn, uint8_t *dest_buffer);
extern int drv_sbpm_free_list(uint16_t head_bn);
extern int drv_sbpm_alloc_list(uint32_t size, uint32_t headroom, uint8_t *data,
	uint16_t *bn0, uint16_t *bn1, uint8_t *bns_num);
#endif

#define cfe_core_runner_image 0
#define IMAGE_0_CFE_CORE_CPU_TX_THREAD_NUMBER 1

// #define DEBUG_DUMP_PACKET

#define INV_RANGE(_addr, _size) invalidate_dcache_range((unsigned long)_addr,\
	(unsigned long)(_addr + _size))
#define FLUSH_RANGE(_addr, _size) flush_dcache_range((unsigned long)_addr,\
	(unsigned long)(_addr + _size))

typedef struct
{
	union {
		struct {
			uint32_t word0;
			uint32_t word1;
		};
		struct {
			uint32_t reserved0:14;
			uint32_t fpm_idx:18;
			uint32_t reserved2:1;
			uint32_t is_chksum_verified:1;
			uint32_t packet_length:14;
			uint32_t abs:1;
			uint32_t reserved1:15;
		} fpm;
		struct {
			uint32_t reserved0:14;
			uint32_t bn0:18;
			uint32_t reserved2:1;
			uint32_t is_chksum_verified:1;
			uint32_t packet_length:14;
			uint32_t abs:1;
			uint32_t reserved1:15;
		} sbpm;
	};
	union {
		uint32_t word2;
		struct {
			uint32_t reason:6;
			uint32_t data_offset:7;
			uint32_t reserved3:12;
			uint32_t source_port:5; /* LAN */
			uint32_t reserved7:1;
			uint32_t is_src_lan:1;
		} lan;
	};
	uint32_t word3;
}
CPU_RX_DESCRIPTOR;

struct cpu_rx_params {
	uint8_t *data_ptr;
	uint8_t data_offset;
	uint16_t packet_size;
	uint16_t reason;
	uint16_t src_bridge_port;
};

#ifdef XRDP_DIRECT
static int cpu_ring_read_packet(uint32_t ring_id,
	struct cpu_rx_params *rx_params)
{
	CPU_RX_DESCRIPTOR rx_desc;
	uint32_t *desc_addr = (uint32_t *)RDD_SRAM_PD_FIFO_PTR(0);
	static int idx;
	volatile void *last_read_idx_ptr = RDD_CPU_RX_LAST_READ_INDEX_PTR(0);
	uint8_t *rx_buffer;

	rx_params->reason = 0;
	rx_params->packet_size = 0;

	desc_addr += idx;
	rx_desc.word2 = swap4bytes(readl(desc_addr + 2));
	rx_desc.word3 = swap4bytes(readl(desc_addr + 3));
	rx_desc.word1 = swap4bytes(readl(desc_addr + 1));
	rx_desc.word0 = swap4bytes(readl(desc_addr));

	if ((rx_desc.word2 == 0) || (rx_desc.word1 == 0))
		return -ENOSR;

	/* The place of data_ofset is the same in all structures in this union
	 * we could use any
	 */
	rx_params->data_offset = rx_desc.lan.data_offset;
	rx_params->packet_size = rx_desc.fpm.packet_length;
	rx_params->src_bridge_port = rx_desc.lan.source_port;
	rx_params->reason = (uint16_t)rx_desc.lan.reason;

	rx_buffer = (uint8_t *)((unsigned long)rx_desc.word0);
	INV_RANGE(rx_buffer, rx_params->packet_size + rx_params->data_offset);
	memcpy(rx_params->data_ptr, rx_buffer, rx_params->packet_size + rx_params->data_offset);

	MWRITE_16(last_read_idx_ptr, ((uintptr_t)desc_addr & 0xffff));
	idx = (idx + 4) % (RDD_SRAM_PD_FIFO_SIZE * 4);

	return 0;
}
#else /* if defined(XRDP_SBPM) */
static int GetPdFromRamFifo(uint32_t *word0, uint32_t *word1, uint32_t *word2,
	uint32_t *word3)
{
	uint32_t *desc_addr = (uint32_t *)RDD_SRAM_PD_FIFO_PTR(0);
	static int idx;

	INV_RANGE((uintptr_t)desc_addr, (64 * 16));

	desc_addr += idx;
	*word1 = swap4bytes(readl(desc_addr + 1));
	*word0 = swap4bytes(readl(desc_addr));
	*word2 = swap4bytes(readl(desc_addr + 2));
	*word3 = swap4bytes(readl(desc_addr + 3));

	if (*word1 == 0)
		return -1;

	writel(0, desc_addr + 3);
	writel(0, desc_addr + 2);
	writel(0, desc_addr);
	writel(0, desc_addr + 1);
	idx = (idx + 4) % (RDD_SRAM_PD_FIFO_SIZE * 4);

	return 0;
}

static int cpu_ring_read_packet(uint32_t ring_id,
	struct cpu_rx_params *rx_params)
{
	CPU_RX_DESCRIPTOR rx_desc;

	rx_params->reason = 0;
	rx_params->packet_size = 0;
	if (GetPdFromRamFifo(&rx_desc.word0, &rx_desc.word1, &rx_desc.word2,
		&rx_desc.word3)) {
		return -ENOSR;
	}

	/* The place of data_ofset is the same in all structures in this union
	 * we could use any
	 */
	rx_params->data_offset = rx_desc.lan.data_offset;
	rx_params->packet_size = rx_desc.fpm.packet_length;
	rx_params->src_bridge_port = rx_desc.lan.source_port;
	rx_params->reason = (uint16_t)rx_desc.lan.reason;

	if (drv_sbpm_copy_list(rx_desc.sbpm.bn0, rx_params->data_ptr)) {
		printf("copy sbpm failed\n");
		return -ENOSR;
	}

	if (drv_sbpm_free_list(rx_desc.sbpm.bn0)) {
		printf("free sbpm failed\n");
		return -ENOSR;
	}

	return 0;
}
#endif

#ifdef XRDP_BBH_PER_LAN_PORT
#define EGRESS_COUNTER_SIZE 8
#else
#define EGRESS_COUNTER_SIZE 1
#endif

static volatile void *bbh_pd_table;
static void *counters_table;
static volatile void *bb_dest_table;
static uint8_t bbh_ingress_counter[8];


#define RDD_CPU_TX_MAX_ITERS	2
#define RDD_CPU_TX_ITER_DELAY	1000
#define RDD_CPU_TX_MAX_BUF_SIZE 2048

static int __rdd_cpu_tx_poll(uint8_t tx_port)
{
	volatile void *tx_pd;
	void *counter_id;
	int old_packet_size;
	int iter;
	int egress_counter_val;

	/* Init when first called */
	if (!bbh_pd_table) {
		bbh_pd_table = RDD_BBH_TX_RING_TABLE_PTR(cfe_core_runner_image);
		bb_dest_table = RDD_BBH_TX_BB_DESTINATION_TABLE_PTR(
			cfe_core_runner_image);
	}

	tx_pd = bbh_pd_table;

#if defined(CONFIG_BCM4912) || defined(CONFIG_BCM6813)
	if (tx_port > 5) {
		counters_table =
			(uint32_t *)RDD_US_TM_BBH_TX_EGRESS_COUNTER_TABLE_PTR(
			cfe_core_runner_image);
		/* First counter index */
		counter_id = counters_table;
		/* egress counters are either 8 or 64 bit alignment */
		counter_id += (tx_port - 6) * EGRESS_COUNTER_SIZE;
	} else
#endif
	{
		counters_table =
			(uint32_t *)RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_PTR(
			cfe_core_runner_image);
		/* First counter index */
		counter_id = counters_table;
		/* egress counters are either 8 or 64 bit alignment */
		counter_id += tx_port * EGRESS_COUNTER_SIZE;
	}

	/* Wait until TX_PD becomes available */
	RDD_BBH_TX_DESCRIPTOR_PACKET_LENGTH_READ(old_packet_size, tx_pd);

	RDD_BBH_TX_EGRESS_COUNTER_ENTRY_COUNTER_READ(egress_counter_val,
		counter_id);

	for (iter = 0; ((old_packet_size != 0) ||
		((bbh_ingress_counter[tx_port] - egress_counter_val) >= 8)
		/*BBH_TX_FIFO_SIZE*/) && (iter < RDD_CPU_TX_MAX_ITERS);
		iter++) {
		udelay(RDD_CPU_TX_ITER_DELAY);
		RDD_BBH_TX_DESCRIPTOR_PACKET_LENGTH_READ(old_packet_size,
			tx_pd);
		RDD_BBH_TX_EGRESS_COUNTER_ENTRY_COUNTER_READ(egress_counter_val,
			counter_id);
	}
	if (iter == RDD_CPU_TX_MAX_ITERS) {
		printf("non empty at bbh full(ingress=%d,egress=%d,diff=%d)\n",
			bbh_ingress_counter[tx_port],
			egress_counter_val,
			bbh_ingress_counter[tx_port] - egress_counter_val);
		return -1;
	}

	return 0;
}

#ifdef XRDP_DIRECT
static int rdd_cpu_tx(uint8_t *buffer, uint32_t length, uint8_t tx_port)
{
	volatile void *tx_pd;
	volatile void *bb_dest;
	uint32_t bb_id;
	uint32_t *tx_buffer_ptr = (uint32_t *)PACKET_BUFFER_POOL_TABLE_ADDR_TX;
	int rc;

	if (length >= RDD_CPU_TX_MAX_BUF_SIZE) {
		printf("ERR: can't transmit buffer with length %u longer "
			"than %d\n", length, RDD_CPU_TX_MAX_BUF_SIZE);
		return -1;
	}

	rc = __rdd_cpu_tx_poll(tx_port);
	if (rc)
		return rc;

	tx_pd = bbh_pd_table;
	bb_dest = bb_dest_table;

	/* copy buffer to the fix location */
	memcpy(tx_buffer_ptr, buffer, length);
	FLUSH_RANGE(tx_buffer_ptr, length);

	/* complete TX descriptor */
	RDD_BBH_TX_DESCRIPTOR_LAST_WRITE(1, tx_pd);
	RDD_BBH_TX_DESCRIPTOR_ABS_WRITE(1, tx_pd);
	RDD_BBH_TX_DESCRIPTOR_TARGET_MEM_0_WRITE(1, tx_pd);
	RDD_BBH_TX_DESCRIPTOR_TARGET_MEM_1_WRITE(1, tx_pd);

	bb_id = __rdd_cpu_tx_get_bb_id(tx_port);
	RDD_BB_DESTINATION_ENTRY_BB_DESTINATION_WRITE(bb_id, bb_dest);

	/* must be latest write */
	if (length < ETH_ZLEN)
		length = ETH_ZLEN;
	RDD_BBH_TX_DESCRIPTOR_PACKET_LENGTH_WRITE(length, tx_pd);

	WMB();

	/* update ingress counter */
	bbh_ingress_counter[tx_port] += 1;
	rnr_regs_cfg_cpu_wakeup_set(cfe_core_runner_image,
		IMAGE_0_CFE_CORE_CPU_TX_THREAD_NUMBER);

	return 0;
}
#endif

#ifdef XRDP_SBPM
static int rdd_cpu_tx_sbpm(uint32_t length, uint16_t bn0, uint16_t bn1,
	uint8_t bns_num, uint8_t tx_port)
{
	volatile void *tx_pd;
	volatile void *bb_dest;
	uint32_t bb_id;
	int rc;

	if (length >= RDD_CPU_TX_MAX_BUF_SIZE) {
		printf("ERR: can't transmit buffer with length %u longer than"\
			"%d\n", length, RDD_CPU_TX_MAX_BUF_SIZE);
		return -1;
	}

	rc = __rdd_cpu_tx_poll(tx_port);
	if (rc)
		return rc;

	tx_pd = bbh_pd_table;
	bb_dest = bb_dest_table;

	/* transmit */
#define CPU_TX_DESCRIPTOR_STRUCT_SIZE 32
	memset((void *)tx_pd, 0, CPU_TX_DESCRIPTOR_STRUCT_SIZE);
	RDD_BBH_TX_DESCRIPTOR_SOF_WRITE(0, tx_pd);
	RDD_BBH_TX_DESCRIPTOR_LAST_WRITE(1, tx_pd);
	RDD_BBH_TX_DESCRIPTOR_AGG_PD_WRITE(0, tx_pd);
	RDD_BBH_TX_DESCRIPTOR_ABS_WRITE(0, tx_pd);
	RDD_BBH_TX_DESCRIPTOR_SOP_WRITE(0, tx_pd);
	RDD_BBH_TX_DESCRIPTOR_BN0_FIRST_WRITE(bn0, tx_pd);
	RDD_BBH_TX_DESCRIPTOR_BN1_FIRST_WRITE(bn1, tx_pd);
	RDD_BBH_TX_DESCRIPTOR_TARGET_MEM_0_WRITE(1, tx_pd);
	RDD_BBH_TX_DESCRIPTOR_TARGET_MEM_1_WRITE(1, tx_pd);
	RDD_BBH_TX_DESCRIPTOR_BN_NUM_WRITE(bns_num, tx_pd);

	bb_id = __rdd_cpu_tx_get_bb_id(tx_port);
	RDD_BB_DESTINATION_ENTRY_BB_DESTINATION_WRITE(bb_id, bb_dest);

	/* must be latest write */
	RDD_BBH_TX_DESCRIPTOR_PACKET_LENGTH_WRITE(length, tx_pd);

	bbh_ingress_counter[tx_port] += 1;

	rnr_regs_cfg_cpu_wakeup_set(cfe_core_runner_image,
		IMAGE_0_CFE_CORE_CPU_TX_THREAD_NUMBER);

	return 0;
}

static int rdd_cpu_tx(uint8_t *buffer, uint32_t length, uint8_t tx_port)
{
	uint16_t bn0, bn1;
	uint8_t bns_num;
	int rc;

	rc = drv_sbpm_alloc_list(length, 0, buffer, &bn0, &bn1, &bns_num);
	if (rc) {
		printf("%s: Failed to allocate sbpm (%d)\n", __func__, rc);
		return rc;
	}

	return rdd_cpu_tx_sbpm(length, bn0, bn1, bns_num, tx_port);
}
#endif

int bcmbca_xrdp_init(void)
{
	int rc = 0;

	printf("%s: Restore HW configuration\n", __func__);
	rc = xrdp_data_path_init();
	printf("%s: Restore HW configuration done. rc=%d\n", __func__, rc);

	return rc;
}

#ifdef DEBUG_DUMP_PACKET
static void dump_packet(uint8_t *data_ptr, uint16_t size)
{
	int i;

	for (i = 0; i < size; i++) {
		if ((i & 0xf) == 0)
			printf("0x%x:", i);
		if ((i & 0x3) == 0)
			printf(" ");
		printf("%02x ", data_ptr[i]);
		if ((i & 0xf) == 0xf)
			printf("\n");
	}
	if ((i & 0xf) != 0)
		printf("\n");
}
#endif

int bcmbca_xrdp_send(void *buffer, uint16_t length, uint8_t tx_port)
{
#ifdef DEBUG_DUMP_PACKET
	printf("%s:%d:send to port %d, buffer is @0x%p, size = %d\n", __func__,
		__LINE__, tx_port, buffer, length);
	dump_packet((uint8_t *)buffer, length);
#endif

	return rdd_cpu_tx(buffer, length, tx_port);
}

int bcmbca_xrdp_recv(uint8_t **buffer, uint16_t *length, uint8_t *rx_port)
{
	int rc;
	struct cpu_rx_params rx_params;

	rx_params.data_ptr = *buffer;
	rc = cpu_ring_read_packet(0, &rx_params);
	if (rc) {
#ifdef DEBUG_DUMP_PACKET // XXX check what should be printed here
		printf("NET: RDD error receive: %d len=%d reason=%d\n",
			rc, (int)rx_params.packet_size, rx_params.reason);
#endif
		return -EAGAIN;
	}

#ifdef DEBUG_DUMP_PACKET
	printf("Dumping received packet: port %d, size %d, "\
		"data_ptr %p, data_offset %d, "\
		"reason %d\n",
		rx_params.src_bridge_port, rx_params.packet_size,
		rx_params.data_ptr, rx_params.data_offset,
		rx_params.reason);
	dump_packet(rx_params.data_ptr, rx_params.packet_size);
#endif

	if (!rx_params.packet_size) {
		printf("NET: Get zero length packet\n");
		return -EAGAIN;
	}

	*buffer = rx_params.data_ptr;
	*rx_port = rx_params.src_bridge_port;
	*length = rx_params.packet_size;

	return 0;
}

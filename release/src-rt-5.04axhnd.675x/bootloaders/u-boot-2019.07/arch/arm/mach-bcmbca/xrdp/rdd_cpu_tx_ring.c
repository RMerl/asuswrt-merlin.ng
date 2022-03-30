// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2015 Broadcom
 */
/*
	
*/

/*
 * cpu_tx_ring.c
 */

#include "rdd.h"
#include "rdd_cpu_tx_ring.h"
#include "bdmf_errno.h"
#include "bdmf_session.h"
#include "rdp_mm.h"
#if defined(CONFIG_BCMBCA_XRDP_GPL)
#include "xrdp_drv_rnr_regs.h"
#else
#include "xrdp_drv_rnr_regs_ag.h"
#endif

#define XRDP_USLEEP(_a)		udelay(_a)
#define XRDP_ERR_MSG(args...)	printf(args)

static RDD_BBH_TX_RING_TABLE_DTS *bbh_pd_table = NULL;
static RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_DTS *counters_table = NULL;
static RDD_BBH_TX_BB_DESTINATION_TABLE_DTS *bb_dest_table = NULL;
static int tx_pd_idx;
static uint8_t bbh_ingress_counter[8];

#define RDD_CPU_TX_MAX_ITERS	2
#define RDD_CPU_TX_ITER_DELAY	1000
#define QM_QUEUE_INDEX_DS_FIRST QM_QUEUE_DS_START

#define RDD_CPU_TX_MAX_BUF_SIZE 2048

static int __rdd_cpu_tx_poll(uint8_t tx_port)
{
	volatile RDD_CPU_TX_DESCRIPTOR_DTS *tx_pd;
	volatile RDD_BBH_TX_EGRESS_COUNTER_ENTRY_DTS *counters_id;
	int old_packet_size, iter, egress_counter_val;
	uint8_t *p;

	if (!bbh_pd_table) { /* First call ? */
		bbh_pd_table = RDD_BBH_TX_RING_TABLE_PTR(
					get_runner_idx(cfe_core_runner_image));
		counters_table = RDD_DS_TM_BBH_TX_EGRESS_COUNTER_TABLE_PTR(
					get_runner_idx(cfe_core_runner_image));
		bb_dest_table = RDD_BBH_TX_BB_DESTINATION_TABLE_PTR(
					get_runner_idx(cfe_core_runner_image));
	}

	tx_pd = (volatile RDD_CPU_TX_DESCRIPTOR_DTS *)&bbh_pd_table->entry;

	counters_id = (volatile RDD_BBH_TX_EGRESS_COUNTER_ENTRY_DTS *)&counters_table->entry[0];

	/* Wait until TX_PD becomes available */
	RDD_BBH_TX_DESCRIPTOR_PACKET_LENGTH_READ(old_packet_size, tx_pd);

	/* Wait until BBH is available*/
	p = (uint8_t *)counters_id;

	/* in 6858 (XRDP_BBH_PER_LAN_PORT)egress counters are in 64 bit alignment),
	 * in other 1 byte */
#if defined(XRDP_BBH_PER_LAN_PORT)
	p = p + (tx_port * EGRESS_COUNTER_SIZE);
#else
	p = p + tx_port;
#endif

	RDD_BBH_TX_EGRESS_COUNTER_ENTRY_COUNTER_READ(egress_counter_val, p);

	for (iter = 0; ((old_packet_size!=0) ||
			((bbh_ingress_counter[tx_port] - egress_counter_val) >= 8)) &&
		       (iter < RDD_CPU_TX_MAX_ITERS); iter++) {
		XRDP_USLEEP(RDD_CPU_TX_ITER_DELAY);
		RDD_BBH_TX_DESCRIPTOR_PACKET_LENGTH_READ(old_packet_size, tx_pd);
		RDD_BBH_TX_EGRESS_COUNTER_ENTRY_COUNTER_READ(egress_counter_val, p);
	}
	if (iter == RDD_CPU_TX_MAX_ITERS) {
		printf("non empty at index %d or bbh full(ingress=%d, "
		       "egress=%d,diff=%d)\n", tx_pd_idx,
		       bbh_ingress_counter[tx_port], egress_counter_val,
                       bbh_ingress_counter[tx_port] - egress_counter_val);
		return BDMF_ERR_INTERNAL;
	}

	return BDMF_ERR_OK;
}

static uint32_t __rdd_cpu_tx_get_bb_id(uint8_t tx_port)
{
#ifdef CONFIG_BCM96858
	switch (tx_port)
	{
	case 0:
		return BB_ID_TX_BBH_4;
	case 1:
		return BB_ID_TX_BBH_1;
	case 2:
		return BB_ID_TX_BBH_2;
	case 3:
		return BB_ID_TX_BBH_3;
	case 4:
		return BB_ID_TX_BBH_0;
	case 5:
		return BB_ID_TX_BBH_5;
	case 6:
		return BB_ID_TX_BBH_6;
	case 7:
		return BB_ID_TX_BBH_7;
	default:
		return BB_ID_TX_BBH_0;
	}
#else
	return (BB_ID_TX_LAN + (tx_port << 6));
#endif
}

#define MIN_PACKET_LENGTH_WITHOUT_CRC 60

#if defined(CONFIG_BCM63146)
int rdd_cpu_tx_new(uint8_t *buffer, uint32_t length, uint8_t tx_port)
{
	volatile RDD_CPU_TX_DESCRIPTOR_DTS *tx_pd;
	volatile RDD_BB_DESTINATION_ENTRY_DTS *bb_dest;
	uint32_t bb_id;
	uint32_t *tx_buffer_ptr = (uint32_t *)PACKET_BUFFER_POOL_TABLE_ADDR_TX;
	int rc;

	if (length >= RDD_CPU_TX_MAX_BUF_SIZE) {
		printf("ERR: can't transmit buffer with length %u longer "
		       "than %d\n", length, RDD_CPU_TX_MAX_BUF_SIZE);
		return BDMF_ERR_PARM;
	}

	rc = __rdd_cpu_tx_poll(tx_port);
	if (rc)
		return rc;

	tx_pd = (volatile RDD_CPU_TX_DESCRIPTOR_DTS *)&bbh_pd_table->entry;
	bb_dest = (volatile RDD_BB_DESTINATION_ENTRY_DTS *)&bb_dest_table->entry;

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

	/*  must be latest write */
	if (unlikely(length < MIN_PACKET_LENGTH_WITHOUT_CRC))
		length = MIN_PACKET_LENGTH_WITHOUT_CRC;
	RDD_BBH_TX_DESCRIPTOR_PACKET_LENGTH_WRITE(length, tx_pd);

	WMB();

	/* update ingress counter */
	bbh_ingress_counter[tx_port] += 1;
	ag_drv_rnr_regs_cfg_cpu_wakeup_set(
			get_runner_idx(cfe_core_runner_image),
			IMAGE_0_CFE_CORE_CPU_TX_THREAD_NUMBER);

	return BDMF_ERR_OK;
}
#endif

int rdd_cpu_tx(uint32_t length, uint16_t bn0, uint16_t bn1, uint8_t bns_num,
	       uint8_t tx_port)
{
	volatile RDD_CPU_TX_DESCRIPTOR_DTS *tx_pd;
	volatile RDD_BB_DESTINATION_ENTRY_DTS *bb_dest;
	uint32_t bb_id;
	int rc;

	if (length >= RDD_CPU_TX_MAX_BUF_SIZE) {
		printf("ERR: can't transmit buffer with length %u longer "
		       "than %d\n", length, RDD_CPU_TX_MAX_BUF_SIZE);
		return BDMF_ERR_PARM;
	}

	rc = __rdd_cpu_tx_poll(tx_port);
	if (rc)
		return rc;

	tx_pd = (volatile RDD_CPU_TX_DESCRIPTOR_DTS *)&bbh_pd_table->entry;
	bb_dest = (volatile RDD_BB_DESTINATION_ENTRY_DTS *)&bb_dest_table->entry;

	/* transmit */
	memset((void *)tx_pd, 0, sizeof(*tx_pd));
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

	/*  must be latest write */
	if (unlikely(length < MIN_PACKET_LENGTH_WITHOUT_CRC))
		length = MIN_PACKET_LENGTH_WITHOUT_CRC;
	RDD_BBH_TX_DESCRIPTOR_PACKET_LENGTH_WRITE(length, tx_pd);

	/* update ingress counter */
	bbh_ingress_counter[tx_port] += 1;
	ag_drv_rnr_regs_cfg_cpu_wakeup_set(
			get_runner_idx(cfe_core_runner_image),
			IMAGE_0_CFE_CORE_CPU_TX_THREAD_NUMBER);

	return BDMF_ERR_OK;
}


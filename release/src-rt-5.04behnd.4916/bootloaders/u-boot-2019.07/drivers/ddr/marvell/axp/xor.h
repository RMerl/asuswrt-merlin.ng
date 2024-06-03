/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#ifndef __XOR_H
#define __XOR_H

#include "ddr3_hw_training.h"

#define MV_XOR_MAX_CHAN         4 /* total channels for all units together */

/*
 * This enumerator describes the type of functionality the XOR channel
 * can have while using the same data structures.
 */
enum xor_type {
	MV_XOR,		/* XOR channel functions as XOR accelerator     */
	MV_DMA,		/* XOR channel functions as IDMA channel        */
	MV_CRC32	/* XOR channel functions as CRC 32 calculator   */
};

/*
 * This enumerator describes the set of commands that can be applied on
 * an engine (e.g. IDMA, XOR). Appling a comman depends on the current
 * status (see MV_STATE enumerator)
 * Start can be applied only when status is IDLE
 * Stop can be applied only when status is IDLE, ACTIVE or PAUSED
 * Pause can be applied only when status is ACTIVE
 * Restart can be applied only when status is PAUSED
 */
enum mv_command {
	MV_START,		/* Start     */
	MV_STOP,		/* Stop     */
	MV_PAUSE,		/* Pause    */
	MV_RESTART		/* Restart  */
};

/*
 * This enumerator describes the set of state conditions.
 * Moving from one state to other is stricted.
 */
enum mv_state {
	MV_IDLE,
	MV_ACTIVE,
	MV_PAUSED,
	MV_UNDEFINED_STATE
};

/* XOR descriptor structure for CRC and DMA descriptor */
struct crc_dma_desc {
	u32 status;		/* Successful descriptor execution indication */
	u32 crc32_result;	/* Result of CRC-32 calculation */
	u32 desc_cmd;		/* type of operation to be carried out on the data */
	u32 next_desc_ptr;	/* Next descriptor address pointer */
	u32 byte_cnt;		/* Size of source block part represented by the descriptor */
	u32 dst_addr;		/* Destination Block address pointer (not used in CRC32 */
	u32 src_addr0;		/* Mode: Source Block address pointer */
	u32 src_addr1;		/* Mode: Source Block address pointer */
} __packed;

void mv_xor_hal_init(u32 chan_num);
int mv_xor_state_get(u32 chan);
void mv_sys_xor_init(MV_DRAM_INFO *dram_info);
void mv_sys_xor_finish(void);
int mv_xor_transfer(u32 chan, int xor_type, u32 xor_chain_ptr);
int mv_xor_mem_init(u32 chan, u32 start_ptr, u32 block_size, u32 init_val_high,
		    u32 init_val_low);

#endif /* __XOR_H */

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Enhanced Direct Memory Access (EDMA3) Controller
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef _EDMA3_H_
#define _EDMA3_H_

#include <linux/stddef.h>

#define EDMA3_PARSET_NULL_LINK			0xffff

/*
 * All parameter RAM set options
 * opt field in edma3_param_set_config structure
 */
#define EDMA3_SLOPT_PRIV_LEVEL			BIT(31)
#define EDMA3_SLOPT_PRIV_ID(id)			((0xf & (id)) << 24)
#define EDMA3_SLOPT_INTERM_COMP_CHAIN_ENB	BIT(23)
#define EDMA3_SLOPT_TRANS_COMP_CHAIN_ENB	BIT(22)
#define EDMA3_SLOPT_INTERM_COMP_INT_ENB		BIT(21)
#define EDMA3_SLOPT_TRANS_COMP_INT_ENB		BIT(20)
#define EDMA3_SLOPT_COMP_CODE(code)		((0x3f & (code)) << 12)
#define EDMA3_SLOPT_FIFO_WIDTH_8		0
#define EDMA3_SLOPT_FIFO_WIDTH_16		(1 << 8)
#define EDMA3_SLOPT_FIFO_WIDTH_32		(2 << 8)
#define EDMA3_SLOPT_FIFO_WIDTH_64		(3 << 8)
#define EDMA3_SLOPT_FIFO_WIDTH_128		(4 << 8)
#define EDMA3_SLOPT_FIFO_WIDTH_256		(5 << 8)
#define EDMA3_SLOPT_FIFO_WIDTH_SET(w)		((w & 0x7) << 8)
#define EDMA3_SLOPT_STATIC			BIT(3)
#define EDMA3_SLOPT_AB_SYNC			BIT(2)
#define EDMA3_SLOPT_DST_ADDR_CONST_MODE		BIT(1)
#define EDMA3_SLOPT_SRC_ADDR_CONST_MODE		BIT(0)

enum edma3_address_mode {
	INCR = 0,
	FIFO = 1
};

enum edma3_fifo_width {
	W8BIT = 0,
	W16BIT = 1,
	W32BIT = 2,
	W64BIT = 3,
	W128BIT = 4,
	W256BIT = 5
};

enum edma3_sync_dimension {
	ASYNC = 0,
	ABSYNC = 1
};

/* PaRAM slots are laid out like this */
struct edma3_slot_layout {
	u32 opt;
	u32 src;
	u32 a_b_cnt;
	u32 dst;
	u32 src_dst_bidx;
	u32 link_bcntrld;
	u32 src_dst_cidx;
	u32 ccnt;
} __packed;

/*
 * Use this to assign trigger word number of edma3_slot_layout struct.
 * trigger_word_name - is the exact name from edma3_slot_layout.
 */
#define EDMA3_TWORD(trigger_word_name)\
		(offsetof(struct edma3_slot_layout, trigger_word_name) / 4)

struct edma3_slot_config {
	u32 opt;
	u32 src;
	u32 dst;
	int bcnt;
	int acnt;
	int ccnt;
	int src_bidx;
	int dst_bidx;
	int src_cidx;
	int dst_cidx;
	int bcntrld;
	int link;
};

struct edma3_channel_config {
	int slot;
	int chnum;
	int complete_code;	/* indicate pending complete interrupt */
	int trigger_slot_word;	/* only used for qedma */
};

void qedma3_start(u32 base, struct edma3_channel_config *cfg);
void qedma3_stop(u32 base, struct edma3_channel_config *cfg);
void edma3_slot_configure(u32 base, int slot, struct edma3_slot_config *cfg);
int edma3_check_for_transfer(u32 base, struct edma3_channel_config *cfg);
void edma3_write_slot(u32 base, int slot, struct edma3_slot_layout *param);
void edma3_read_slot(u32 base, int slot, struct edma3_slot_layout *param);

void edma3_set_dest(u32 base, int slot, u32 dst, enum edma3_address_mode mode,
		    enum edma3_fifo_width width);
void edma3_set_dest_index(u32 base, unsigned slot, int bidx, int cidx);
void edma3_set_dest_addr(u32 base, int slot, u32 dst);

void edma3_set_src(u32 base, int slot, u32 src, enum edma3_address_mode mode,
		   enum edma3_fifo_width width);
void edma3_set_src_index(u32 base, unsigned slot, int bidx, int cidx);
void edma3_set_src_addr(u32 base, int slot, u32 src);

void edma3_set_transfer_params(u32 base, int slot, int acnt,
			       int bcnt, int ccnt, u16 bcnt_rld,
			       enum edma3_sync_dimension sync_mode);
void edma3_transfer(unsigned long edma3_base_addr, unsigned int
		edma_slot_num, void *dst, void *src, size_t len);
void edma3_fill(unsigned long edma3_base_addr, unsigned int edma_slot_num,
		void *dst, u8 val, size_t len);

#endif

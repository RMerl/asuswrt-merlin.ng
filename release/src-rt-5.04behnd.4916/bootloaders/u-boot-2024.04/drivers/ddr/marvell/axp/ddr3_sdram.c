// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) Marvell International Ltd. and its affiliates
 */

#include <common.h>
#include <i2c.h>
#include <spl.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>
#include <asm/arch/soc.h>

#include "ddr3_hw_training.h"
#include "xor.h"
#include "xor_regs.h"

static void ddr3_flush_l1_line(u32 line);

extern u32 pbs_pattern[2][LEN_16BIT_PBS_PATTERN];
extern u32 pbs_pattern_32b[2][LEN_PBS_PATTERN];
#if defined(MV88F78X60)
extern u32 pbs_pattern_64b[2][LEN_PBS_PATTERN];
#endif
extern u32 pbs_dq_mapping[PUP_NUM_64BIT + 1][DQ_NUM];

#if defined(MV88F78X60) || defined(MV88F672X)
/* PBS locked dq (per pup) */
u32 pbs_locked_dq[MAX_PUP_NUM][DQ_NUM] = { { 0 } };
u32 pbs_locked_dm[MAX_PUP_NUM] = { 0 };
u32 pbs_locked_value[MAX_PUP_NUM][DQ_NUM] = { { 0 } };

int per_bit_data[MAX_PUP_NUM][DQ_NUM];
#endif

static u32 sdram_data[LEN_KILLER_PATTERN] __aligned(32) = { 0 };

static struct crc_dma_desc dma_desc __aligned(32) = { 0 };

#define XOR_TIMEOUT 0x8000000

struct xor_channel_t {
	struct crc_dma_desc *desc;
	unsigned long desc_phys_addr;
};

#define XOR_CAUSE_DONE_MASK(chan)	((0x1 | 0x2) << (chan * 16))

void xor_waiton_eng(int chan)
{
	int timeout;

	timeout = 0;
	while (!(reg_read(XOR_CAUSE_REG(XOR_UNIT(chan))) &
		 XOR_CAUSE_DONE_MASK(XOR_CHAN(chan)))) {
		if (timeout > XOR_TIMEOUT)
			goto timeout;

		timeout++;
	}

	timeout = 0;
	while (mv_xor_state_get(chan) != MV_IDLE) {
		if (timeout > XOR_TIMEOUT)
			goto timeout;

		timeout++;
	}

	/* Clear int */
	reg_write(XOR_CAUSE_REG(XOR_UNIT(chan)),
		  ~(XOR_CAUSE_DONE_MASK(XOR_CHAN(chan))));

timeout:
	return;
}

static int special_compare_pattern(u32 uj)
{
	if ((uj == 30) || (uj == 31) || (uj == 61) || (uj == 62) ||
	    (uj == 93) || (uj == 94) || (uj == 126) || (uj == 127))
		return 1;

	return 0;
}

/*
 * Compare code extracted as its used by multiple functions. This
 * reduces code-size and makes it easier to maintain it. Additionally
 * the code is not indented that much and therefore easier to read.
 */
static void compare_pattern_v1(u32 uj, u32 *pup, u32 *pattern,
			       u32 pup_groups, int debug_dqs)
{
	u32 val;
	u32 uk;
	u32 var1;
	u32 var2;
	__maybe_unused u32 dq;

	if (((sdram_data[uj]) != (pattern[uj])) && (*pup != 0xFF)) {
		for (uk = 0; uk < PUP_NUM_32BIT; uk++) {
			val = CMP_BYTE_SHIFT * uk;
			var1 = ((sdram_data[uj] >> val) & CMP_BYTE_MASK);
			var2 = ((pattern[uj] >> val) & CMP_BYTE_MASK);

			if (var1 != var2) {
				*pup |= (1 << (uk + (PUP_NUM_32BIT *
						     (uj % pup_groups))));

#ifdef MV_DEBUG_DQS
				if (!debug_dqs)
					continue;

				for (dq = 0; dq < DQ_NUM; dq++) {
					val = uk + (PUP_NUM_32BIT *
						    (uj % pup_groups));
					if (((var1 >> dq) & 0x1) !=
					    ((var2 >> dq) & 0x1))
						per_bit_data[val][dq] = 1;
					else
						per_bit_data[val][dq] = 0;
				}
#endif
			}
		}
	}
}

static void compare_pattern_v2(u32 uj, u32 *pup, u32 *pattern)
{
	u32 val;
	u32 uk;
	u32 var1;
	u32 var2;

	if (((sdram_data[uj]) != (pattern[uj])) && (*pup != 0x3)) {
		/* Found error */
		for (uk = 0; uk < PUP_NUM_32BIT; uk++) {
			val = CMP_BYTE_SHIFT * uk;
			var1 = (sdram_data[uj] >> val) & CMP_BYTE_MASK;
			var2 = (pattern[uj] >> val) & CMP_BYTE_MASK;
			if (var1 != var2)
				*pup |= (1 << (uk % PUP_NUM_16BIT));
		}
	}
}

/*
 * Name:     ddr3_sdram_compare
 * Desc:     Execute compare per PUP
 * Args:     unlock_pup      Bit array of the unlock pups
 *           new_locked_pup  Output  bit array of the pups with failed compare
 *           pattern         Pattern to compare
 *           pattern_len     Length of pattern (in bytes)
 *           sdram_offset    offset address to the SDRAM
 *           write           write to the SDRAM before read
 *           mask            compare pattern with mask;
 *           mask_pattern    Mask to compare pattern
 *
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
int ddr3_sdram_compare(MV_DRAM_INFO *dram_info, u32 unlock_pup,
		       u32 *new_locked_pup, u32 *pattern,
		       u32 pattern_len, u32 sdram_offset, int write,
		       int mask, u32 *mask_pattern,
		       int special_compare)
{
	u32 uj;
	__maybe_unused u32 pup_groups;
	__maybe_unused u32 dq;

#if !defined(MV88F67XX)
	if (dram_info->num_of_std_pups == PUP_NUM_64BIT)
		pup_groups = 2;
	else
		pup_groups = 1;
#endif

	ddr3_reset_phy_read_fifo();

	/* Check if need to write to sdram before read */
	if (write == 1)
		ddr3_dram_sram_burst((u32)pattern, sdram_offset, pattern_len);

	ddr3_dram_sram_burst(sdram_offset, (u32)sdram_data, pattern_len);

	/* Compare read result to write */
	for (uj = 0; uj < pattern_len; uj++) {
		if (special_compare && special_compare_pattern(uj))
			continue;

#if defined(MV88F78X60) || defined(MV88F672X)
		compare_pattern_v1(uj, new_locked_pup, pattern, pup_groups, 1);
#elif defined(MV88F67XX)
		compare_pattern_v2(uj, new_locked_pup, pattern);
#endif
	}

	return MV_OK;
}

#if defined(MV88F78X60) || defined(MV88F672X)
/*
 * Name:     ddr3_sdram_dm_compare
 * Desc:     Execute compare per PUP
 * Args:     unlock_pup      Bit array of the unlock pups
 *           new_locked_pup  Output  bit array of the pups with failed compare
 *           pattern         Pattern to compare
 *           pattern_len     Length of pattern (in bytes)
 *           sdram_offset    offset address to the SDRAM
 *           write           write to the SDRAM before read
 *           mask            compare pattern with mask;
 *           mask_pattern    Mask to compare pattern
 *
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
int ddr3_sdram_dm_compare(MV_DRAM_INFO *dram_info, u32 unlock_pup,
			  u32 *new_locked_pup, u32 *pattern,
			  u32 sdram_offset)
{
	u32 uj, uk, var1, var2, pup_groups;
	u32 val;
	u32 pup = 0;

	if (dram_info->num_of_std_pups == PUP_NUM_64BIT)
		pup_groups = 2;
	else
		pup_groups = 1;

	ddr3_dram_sram_burst((u32)pattern, SDRAM_PBS_TX_OFFS,
			     LEN_PBS_PATTERN);
	ddr3_dram_sram_burst(SDRAM_PBS_TX_OFFS, (u32)sdram_data,
			     LEN_PBS_PATTERN);

	/* Validate the correctness of the results */
	for (uj = 0; uj < LEN_PBS_PATTERN; uj++)
		compare_pattern_v1(uj, &pup, pattern, pup_groups, 0);

	/* Test the DM Signals */
	*(u32 *)(SDRAM_PBS_TX_OFFS + 0x10) = 0x12345678;
	*(u32 *)(SDRAM_PBS_TX_OFFS + 0x14) = 0x12345678;

	sdram_data[0] = *(u32 *)(SDRAM_PBS_TX_OFFS + 0x10);
	sdram_data[1] = *(u32 *)(SDRAM_PBS_TX_OFFS + 0x14);

	for (uj = 0; uj < 2; uj++) {
		if (((sdram_data[uj]) != (pattern[uj])) &&
		    (*new_locked_pup != 0xFF)) {
			for (uk = 0; uk < PUP_NUM_32BIT; uk++) {
				val = CMP_BYTE_SHIFT * uk;
				var1 = ((sdram_data[uj] >> val) & CMP_BYTE_MASK);
				var2 = ((pattern[uj] >> val) & CMP_BYTE_MASK);
				if (var1 != var2) {
					*new_locked_pup |= (1 << (uk +
						(PUP_NUM_32BIT * (uj % pup_groups))));
					*new_locked_pup |= pup;
				}
			}
		}
	}

	return MV_OK;
}

/*
 * Name:     ddr3_sdram_pbs_compare
 * Desc:     Execute SRAM compare per PUP and DQ.
 * Args:     pup_locked             bit array of locked pups
 *           is_tx                  Indicate whether Rx or Tx
 *           pbs_pattern_idx        Index of PBS pattern
 *           pbs_curr_val           The PBS value
 *           pbs_lock_val           The value to set to locked PBS
 *           skew_array             Global array to update with the compare results
 *           ai_unlock_pup_dq_array bit array of the locked / unlocked pups per dq.
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
int ddr3_sdram_pbs_compare(MV_DRAM_INFO *dram_info, u32 pup_locked,
			   int is_tx, u32 pbs_pattern_idx,
			   u32 pbs_curr_val, u32 pbs_lock_val,
			   u32 *skew_array, u8 *unlock_pup_dq_array,
			   u32 ecc)
{
	/* bit array failed dq per pup for current compare */
	u32 pbs_write_pup[DQ_NUM] = { 0 };
	u32 update_pup;	/* pup as HW convention */
	u32 max_pup;	/* maximal pup index */
	u32 pup_addr;
	u32 ui, dq, pup;
	int var1, var2;
	u32 sdram_offset, pup_groups, tmp_pup;
	u32 *pattern_ptr;
	u32 val;

	/* Choose pattern */
	switch (dram_info->ddr_width) {
#if defined(MV88F672X)
	case 16:
		pattern_ptr = (u32 *)&pbs_pattern[pbs_pattern_idx];
		break;
#endif
	case 32:
		pattern_ptr = (u32 *)&pbs_pattern_32b[pbs_pattern_idx];
		break;
#if defined(MV88F78X60)
	case 64:
		pattern_ptr = (u32 *)&pbs_pattern_64b[pbs_pattern_idx];
		break;
#endif
	default:
		return MV_FAIL;
	}

	max_pup = dram_info->num_of_std_pups;

	sdram_offset = SDRAM_PBS_I_OFFS + pbs_pattern_idx * SDRAM_PBS_NEXT_OFFS;

	if (dram_info->num_of_std_pups == PUP_NUM_64BIT)
		pup_groups = 2;
	else
		pup_groups = 1;

	ddr3_reset_phy_read_fifo();

	/* Check if need to write to sdram before read */
	if (is_tx == 1) {
		ddr3_dram_sram_burst((u32)pattern_ptr, sdram_offset,
				     LEN_PBS_PATTERN);
	}

	ddr3_dram_sram_read(sdram_offset, (u32)sdram_data, LEN_PBS_PATTERN);

	/* Compare read result to write */
	for (ui = 0; ui < LEN_PBS_PATTERN; ui++) {
		if ((sdram_data[ui]) != (pattern_ptr[ui])) {
			/* found error */
			/* error in low pup group */
			for (pup = 0; pup < PUP_NUM_32BIT; pup++) {
				val = CMP_BYTE_SHIFT * pup;
				var1 = ((sdram_data[ui] >> val) &
					CMP_BYTE_MASK);
				var2 = ((pattern_ptr[ui] >> val) &
					CMP_BYTE_MASK);

				if (var1 != var2) {
					if (dram_info->ddr_width > 16) {
						tmp_pup = (pup + PUP_NUM_32BIT *
							   (ui % pup_groups));
					} else {
						tmp_pup = (pup % PUP_NUM_16BIT);
					}

					update_pup = (1 << tmp_pup);
					if (ecc && (update_pup != 0x1))
						continue;

					/*
					 * Pup is failed - Go over all DQs and
					 * look for failures
					 */
					for (dq = 0; dq < DQ_NUM; dq++) {
						val = tmp_pup * (1 - ecc) +
							ecc * ECC_PUP;
						if (((var1 >> dq) & 0x1) !=
						    ((var2 >> dq) & 0x1)) {
							if (pbs_locked_dq[val][dq] == 1 &&
							    pbs_locked_value[val][dq] != pbs_curr_val)
								continue;

							/*
							 * Activate write to
							 * update PBS to
							 * pbs_lock_val
							 */
							pbs_write_pup[dq] |=
								update_pup;

							/*
							 * Update the
							 * unlock_pup_dq_array
							 */
							unlock_pup_dq_array[dq] &=
								~update_pup;

							/*
							 * Lock PBS value for
							 * failed bits in
							 * compare operation
							 */
							skew_array[tmp_pup * DQ_NUM + dq] =
								pbs_curr_val;
						}
					}
				}
			}
		}
	}

	pup_addr = (is_tx == 1) ? PUP_PBS_TX : PUP_PBS_RX;

	/* Set last failed bits PBS to min / max pbs value */
	for (dq = 0; dq < DQ_NUM; dq++) {
		for (pup = 0; pup < max_pup; pup++) {
			if (pbs_write_pup[dq] & (1 << pup)) {
				val = pup * (1 - ecc) + ecc * ECC_PUP;
				if (pbs_locked_dq[val][dq] == 1 &&
				    pbs_locked_value[val][dq] != pbs_curr_val)
					continue;

				/* Mark the dq as locked */
				pbs_locked_dq[val][dq] = 1;
				pbs_locked_value[val][dq] = pbs_curr_val;
				ddr3_write_pup_reg(pup_addr +
						   pbs_dq_mapping[val][dq],
						   CS0, val, 0, pbs_lock_val);
			}
		}
	}

	return MV_OK;
}
#endif

/*
 * Name:     ddr3_sdram_direct_compare
 * Desc:     Execute compare  per PUP without DMA (no burst mode)
 * Args:     unlock_pup       Bit array of the unlock pups
 *           new_locked_pup   Output  bit array of the pups with failed compare
 *           pattern          Pattern to compare
 *           pattern_len      Length of pattern (in bytes)
 *           sdram_offset     offset address to the SDRAM
 *           write            write to the SDRAM before read
 *           mask             compare pattern with mask;
 *           auiMaskPatter    Mask to compare pattern
 *
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
int ddr3_sdram_direct_compare(MV_DRAM_INFO *dram_info, u32 unlock_pup,
			      u32 *new_locked_pup, u32 *pattern,
			      u32 pattern_len, u32 sdram_offset,
			      int write, int mask, u32 *mask_pattern)
{
	u32 uj, uk, pup_groups;
	u32 *sdram_addr;	/* used to read from SDRAM */

	sdram_addr = (u32 *)sdram_offset;

	if (dram_info->num_of_std_pups == PUP_NUM_64BIT)
		pup_groups = 2;
	else
		pup_groups = 1;

	/* Check if need to write before read */
	if (write == 1) {
		for (uk = 0; uk < pattern_len; uk++) {
			*sdram_addr = pattern[uk];
			sdram_addr++;
		}
	}

	sdram_addr = (u32 *)sdram_offset;

	for (uk = 0; uk < pattern_len; uk++) {
		sdram_data[uk] = *sdram_addr;
		sdram_addr++;
	}

	/* Compare read result to write */
	for (uj = 0; uj < pattern_len; uj++) {
		if (dram_info->ddr_width > 16) {
			compare_pattern_v1(uj, new_locked_pup, pattern,
					   pup_groups, 0);
		} else {
			compare_pattern_v2(uj, new_locked_pup, pattern);
		}
	}

	return MV_OK;
}

/*
 * Name:     ddr3_dram_sram_burst
 * Desc:     Read from the SDRAM in burst of 64 bytes
 * Args:     src
 *           dst
 * Notes:    Using the XOR mechanism
 * Returns:  MV_OK if success, other error code if fail.
 */
int ddr3_dram_sram_burst(u32 src, u32 dst, u32 len)
{
	u32 chan, byte_count, cs_num, byte;
	struct xor_channel_t channel;

	chan = 0;
	byte_count = len * 4;

	/* Wait for previous transfer completion */
	while (mv_xor_state_get(chan) != MV_IDLE)
		;

	/* Build the channel descriptor */
	channel.desc = &dma_desc;

	/* Enable Address Override and set correct src and dst */
	if (src < SRAM_BASE) {
		/* src is DRAM CS, dst is SRAM */
		cs_num = (src / (1 + SDRAM_CS_SIZE));
		reg_write(XOR_ADDR_OVRD_REG(0, 0),
			  ((cs_num << 1) | (1 << 0)));
		channel.desc->src_addr0 = (src % (1 + SDRAM_CS_SIZE));
		channel.desc->dst_addr = dst;
	} else {
		/* src is SRAM, dst is DRAM CS */
		cs_num = (dst / (1 + SDRAM_CS_SIZE));
		reg_write(XOR_ADDR_OVRD_REG(0, 0),
			  ((cs_num << 25) | (1 << 24)));
		channel.desc->src_addr0 = (src);
		channel.desc->dst_addr = (dst % (1 + SDRAM_CS_SIZE));
		channel.desc->src_addr0 = src;
		channel.desc->dst_addr = (dst % (1 + SDRAM_CS_SIZE));
	}

	channel.desc->src_addr1 = 0;
	channel.desc->byte_cnt = byte_count;
	channel.desc->next_desc_ptr = 0;
	channel.desc->status = 1 << 31;
	channel.desc->desc_cmd = 0x0;
	channel.desc_phys_addr = (unsigned long)&dma_desc;

	ddr3_flush_l1_line((u32)&dma_desc);

	/* Issue the transfer */
	if (mv_xor_transfer(chan, MV_DMA, channel.desc_phys_addr) != MV_OK)
		return MV_FAIL;

	/* Wait for completion */
	xor_waiton_eng(chan);

	if (dst > SRAM_BASE) {
		for (byte = 0; byte < byte_count; byte += 0x20)
			cache_inv(dst + byte);
	}

	return MV_OK;
}

/*
 * Name:     ddr3_flush_l1_line
 * Desc:
 * Args:
 * Notes:
 * Returns:  MV_OK if success, other error code if fail.
 */
static void ddr3_flush_l1_line(u32 line)
{
	u32 reg;

#if defined(MV88F672X)
	reg = 1;
#else
	reg = reg_read(REG_SAMPLE_RESET_LOW_ADDR) &
		(1 << REG_SAMPLE_RESET_CPU_ARCH_OFFS);
#ifdef MV88F67XX
	reg = ~reg & (1 << REG_SAMPLE_RESET_CPU_ARCH_OFFS);
#endif
#endif

	if (reg) {
		/* V7 Arch mode */
		flush_l1_v7(line);
		flush_l1_v7(line + CACHE_LINE_SIZE);
	} else {
		/* V6 Arch mode */
		flush_l1_v6(line);
		flush_l1_v6(line + CACHE_LINE_SIZE);
	}
}

int ddr3_dram_sram_read(u32 src, u32 dst, u32 len)
{
	u32 ui;
	u32 *dst_ptr, *src_ptr;

	dst_ptr = (u32 *)dst;
	src_ptr = (u32 *)src;

	for (ui = 0; ui < len; ui++) {
		*dst_ptr = *src_ptr;
		dst_ptr++;
		src_ptr++;
	}

	return MV_OK;
}

int ddr3_sdram_dqs_compare(MV_DRAM_INFO *dram_info, u32 unlock_pup,
			   u32 *new_locked_pup, u32 *pattern,
			   u32 pattern_len, u32 sdram_offset, int write,
			   int mask, u32 *mask_pattern,
			   int special_compare)
{
	u32 uj, pup_groups;

	if (dram_info->num_of_std_pups == PUP_NUM_64BIT)
		pup_groups = 2;
	else
		pup_groups = 1;

	ddr3_reset_phy_read_fifo();

	/* Check if need to write to sdram before read */
	if (write == 1)
		ddr3_dram_sram_burst((u32)pattern, sdram_offset, pattern_len);

	ddr3_dram_sram_burst(sdram_offset, (u32)sdram_data, pattern_len);

	/* Compare read result to write */
	for (uj = 0; uj < pattern_len; uj++) {
		if (special_compare && special_compare_pattern(uj))
			continue;

		if (dram_info->ddr_width > 16) {
			compare_pattern_v1(uj, new_locked_pup, pattern,
					   pup_groups, 1);
		} else {
			compare_pattern_v2(uj, new_locked_pup, pattern);
		}
	}

	return MV_OK;
}

void ddr3_reset_phy_read_fifo(void)
{
	u32 reg;

	/* reset read FIFO */
	reg = reg_read(REG_DRAM_TRAINING_ADDR);
	/* Start Auto Read Leveling procedure */
	reg |= (1 << REG_DRAM_TRAINING_RL_OFFS);

	/* 0x15B0 - Training Register */
	reg_write(REG_DRAM_TRAINING_ADDR, reg);

	reg = reg_read(REG_DRAM_TRAINING_2_ADDR);
	reg |= ((1 << REG_DRAM_TRAINING_2_FIFO_RST_OFFS) +
		(1 << REG_DRAM_TRAINING_2_SW_OVRD_OFFS));

	/* [0] = 1 - Enable SW override, [4] = 1 - FIFO reset  */
	/* 0x15B8 - Training SW 2 Register */
	reg_write(REG_DRAM_TRAINING_2_ADDR, reg);

	do {
		reg = reg_read(REG_DRAM_TRAINING_2_ADDR) &
			(1 << REG_DRAM_TRAINING_2_FIFO_RST_OFFS);
	} while (reg);	/* Wait for '0' */

	reg = reg_read(REG_DRAM_TRAINING_ADDR);

	/* Clear Auto Read Leveling procedure */
	reg &= ~(1 << REG_DRAM_TRAINING_RL_OFFS);

	/* 0x15B0 - Training Register */
	reg_write(REG_DRAM_TRAINING_ADDR, reg);
}

/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * MSMC controller
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 */

#ifndef _MSMC_H_
#define _MSMC_H_

#include <asm/arch/hardware.h>

enum mpax_seg_size {
	MPAX_SEG_4K = 0x0b,
	MPAX_SEG_8K,
	MPAX_SEG_16K,
	MPAX_SEG_32K,
	MPAX_SEG_64K,
	MPAX_SEG_128K,
	MPAX_SEG_256K,
	MPAX_SEG_512K,
	MPAX_SEG_1M,
	MPAX_SEG_2M,
	MPAX_SEG_4M,
	MPAX_SEG_8M,
	MPAX_SEG_16M,
	MPAX_SEG_32M,
	MPAX_SEG_64M,
	MPAX_SEG_128M,
	MPAX_SEG_256M,
	MPAX_SEG_512M,
	MPAX_SEG_1G,
	MPAX_SEG_2G,
	MPAX_SEG_4G
};

void msmc_share_all_segments(int priv_id);
void msmc_get_ses_mpax(int priv_id, int ses_pair, u32 *mpax);
void msmc_set_ses_mpax(int priv_id, int ses_pair, u32 *mpax);
void msmc_map_ses_segment(int priv_id, int ses_pair,
			  u32 src_pfn, u32 dst_pfn, enum mpax_seg_size size);

#endif

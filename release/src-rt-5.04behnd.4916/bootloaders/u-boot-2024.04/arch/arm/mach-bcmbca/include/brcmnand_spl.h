/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _BRCM_NAND_SPL_H
#define _BRCM_NAND_SPL_H

void brcmnand_init(void);
uint32_t brcmnand_get_block_size(void);
uint32_t brcmnand_get_page_size(void);
uint64_t brcmnand_get_total_size(void);
int brcmnand_read_buf(int block, int offset, u8 *dst, u32 len);
int brcmnand_is_bad_block(int block);

#endif

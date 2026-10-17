/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 Broadcom Ltd.
 */

#ifndef _BCMBCA_NAND_SPL_H
#define _BCMBCA_NAND_SPL_H

u32 nand_spl_get_blk_size(void);
u32 nand_spl_get_page_size(void);
u64 nand_spl_get_total_size(void);

#endif

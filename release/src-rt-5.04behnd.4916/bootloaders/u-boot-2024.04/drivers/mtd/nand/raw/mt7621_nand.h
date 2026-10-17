/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _MT7621_NAND_H_
#define _MT7621_NAND_H_

#include <linux/types.h>
#include <linux/mtd/mtd.h>
#include <linux/compiler.h>
#include <linux/mtd/rawnand.h>

struct mt7621_nfc {
	struct nand_chip nand;

	void __iomem *nfi_regs;
	void __iomem *ecc_regs;

	u32 spare_per_sector;
};

/* for SPL */
void mt7621_nfc_spl_init(struct mt7621_nfc *nfc);
int mt7621_nfc_spl_post_init(struct mt7621_nfc *nfc);

#endif /* _MT7621_NAND_H_ */

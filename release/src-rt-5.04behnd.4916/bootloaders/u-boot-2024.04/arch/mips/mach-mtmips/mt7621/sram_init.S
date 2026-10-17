/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2022 MediaTek Inc. All rights reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include <asm/addrspace.h>
#include <asm/asm.h>
#include <asm/regdef.h>
#include "mt7621.h"

LEAF(mips_sram_init)
	li	t0, KSEG1ADDR(FE_BASE)
	li	t1, FE_PSE_RESET
	sw	t1, FE_RST_GLO_REG(t0)

	li	t1, (FE_PSE_RAM | FE_PSE_MEM_EN)
	sw	t1, FE_RST_GLO_REG(t0)

	jr	ra
	END(mips_sram_init)

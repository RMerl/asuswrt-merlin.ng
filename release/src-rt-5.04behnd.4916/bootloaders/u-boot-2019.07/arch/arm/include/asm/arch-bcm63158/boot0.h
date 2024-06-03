/* SPDX-License-Identifier: GPL-2.0+
 *
 *  Copyright 2019 Broadcom Ltd.
 */

#ifndef _63158_BOOT0_H
#define _63158_BOOT0_H

#include <asm-offsets.h>
#include <linux/linkage.h>
#include <asm/system.h>

_bcm_boot:
	b	reset
#if defined(CONFIG_SPL_BUILD) && !defined(CONFIG_TPL_BUILD)
.ascii 	CONFIG_BCMBCA_SPL_SALT
#endif

#endif
